#ifndef SMAA1XT_H
#define SMAA1XT_H

#include "SMAA.h"

struct jitterSettings_t
{
	glm::vec2			haltonSequence[128];
	float				haltonScale;
	int					haltonIndex;
	int					enableDithering;
	float				ditheringScale;

	jitterSettings_t()
	{
		haltonIndex = 16;
		enableDithering = 1;
		haltonScale = 10.0f;
		ditheringScale = 0.0f;
	}

	~jitterSettings_t() {};
};

struct jitter2Settings_t
{
	glm::vec4		indices[2];
	glm::vec2		samples[2];
	float			scale;
	unsigned int	numSamples;

	jitter2Settings_t()
	{
		scale = 100.0f;
		numSamples = 2;

		samples[0] = glm::vec2(0.25f, 0.25f);
		samples[1] = glm::vec2(-0.25f, -0.25f);
		indices[0] = glm::vec4(1, 1, 1, 0);
		indices[1] = glm::vec4(2, 2, 2, 0);
	}

	~jitter2Settings_t() {};
};

struct reprojectSettings_t
{
	glm::mat4		previousProjection;
	glm::mat4		previousView;
	glm::mat4		prevTranslation;

	glm::mat4		currentView;

	reprojectSettings_t()
	{
		this->previousProjection = glm::mat4(1.0f);
		this->previousView = glm::mat4(1.0f);
		this->prevTranslation = glm::mat4(1.0f);

		this->currentView = glm::mat4(1.0f);
	}

	~reprojectSettings_t() {};
};

struct TAASettings_t
{
	//velocity
	float velocityScale;
	//Inside
	float feedbackFactor;
	//Custom
	float maxDepthFalloff;

	TAASettings_t()
	{
		this->feedbackFactor = 0.9f;
		this->maxDepthFalloff = 1.0f;
		this->velocityScale = 1.0f;
	}

	~TAASettings_t() { };
};

class TSMAA : public SMAAScene
{
public:

	TSMAA(
		const char* windowName = "Ziyad Barakat's portfolio (SMAA 1xt)",
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_e::perspective, 50.0f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		model_t model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: SMAAScene(windowName, texModelCamera, shaderConfigPath, model)
	{
		jitterUniforms = new jitterSettings_t();
		for (int iter = 0; iter < 128; iter++)
		{
			jitterUniforms->haltonSequence[iter] = glm::vec2(CreateHaltonSequence(iter + 1, 2), CreateHaltonSequence(iter + 1, 3));
		}

	}

	~TSMAA() override { delete jitterUniforms; };

	virtual void Initialize() override
	{
		scene3D::Initialize();
		SMAAArea.LoadTexture();
		SMAASearch.LoadTexture();

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer.Initialize();
		geometryBuffer.Bind();
		geometryBuffer.AddAttachment(frameBuffer::attachment_t("color", colorDesc));

		FBODescriptor velDesc;
		velDesc.format = GL_RG;
		velDesc.internalFormat = GL_RG16_SNORM;
		velDesc.dataType = GL_FLOAT;
		velDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer.AddAttachment(frameBuffer::attachment_t("velocity", velDesc));

		FBODescriptor depthDesc;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer.AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		edgesBuffer.Initialize();
		edgesBuffer.Bind();

		FBODescriptor edgeDesc;
		edgeDesc.format = GL_RG;
		edgeDesc.internalFormat = GL_RG8;
		edgeDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		edgesBuffer.AddAttachment(frameBuffer::attachment_t("edge", edgeDesc));

		weightsBuffer.Initialize();
		weightsBuffer.Bind();

		weightsBuffer.AddAttachment(frameBuffer::attachment_t("blend", colorDesc));

		SMAABuffer.Initialize();
		SMAABuffer.Bind();
		SMAABuffer.AddAttachment(frameBuffer::attachment_t("SMAA", colorDesc));

		for(unsigned int iter = 0; iter < numPreviousFrames; iter++)
		{
			frameBuffer* newBuffer = new frameBuffer();

			newBuffer->Initialize();
			newBuffer->Bind();
			newBuffer->AddAttachment(frameBuffer::attachment_t("color", colorDesc));
			newBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

			historyFrames.push_back(newBuffer);
		}

		defProgram = shaderProgramsMap["geometry"];
		edgeDetectionProgram = shaderProgramsMap["edgeDetection"];
		blendingWeightProgram = shaderProgramsMap["blendingWeight"];
		SMAAProgram = shaderProgramsMap["neighborhoodBlend"];
		resolveProgram = shaderProgramsMap["resolve"];
		compareProgram = shaderProgramsMap["compare"];
		finalProgram = shaderProgramsMap["final"];
		
		frameBuffer::Unbind();
	}

protected:

	std::vector<frameBuffer*>		historyFrames;

	shaderProgram_t resolveProgram;
	uint8_t numPreviousFrames = 2;

	bool enableCompare = true;
	bool currentFrame = false;

	reprojectSettings_t* reprojectUniforms = nullptr;
	TAASettings_t* taaUniforms = nullptr;
	jitterSettings_t* jitterUniforms = nullptr;
	jitter2Settings_t* jitter2Uniforms = nullptr;

	virtual void Update() override
	{
		SMAAScene::Update();
		//if even frame then write to 1 and read from 0 and vice versa
		currentFrame = ((defaultPayload->totalFrames % 2) == 0) ? false : true;
	}

	virtual void Draw() override
	{
		reprojectUniforms->currentView =camera.view;
		camera.ChangeProjection(camera_t::projection_e::perspective);
		camera.Update();

		UpdateDefaultUniforms(camera, clock, &testModel);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		JitterPass(); //i think jitter fucks with SMAA
		glDisable(GL_BLEND);

		camera.ChangeProjection(camera_t::projection_e::orthographic);
		UpdateDefaultUniforms(camera, clock, &testModel);
		
		EdgeDetectionPass();
		BlendingWeightsPass();
		SMAAPass();

		SMAAResolvePass();

		FinalPass(&historyFrames[currentFrame]->attachments["color"], &geometryBuffer.attachments["color"]);
	}

	virtual void PostDraw() override
	{
		SMAAScene::PostDraw();
		ClearBuffers();
	}

	virtual void JitterPass()
	{
		GL_PUSH_DEBUG_GROUP();
		geometryBuffer.Bind();

		GLenum drawbuffers[2] = {
			geometryBuffer.attachments["color"].FBODesc.attachmentFormat, //color
			geometryBuffer.attachments["velocity"].FBODesc.attachmentFormat, //velocity
		};

		glDrawBuffers(2, drawbuffers);

		//we just need the first LOd so only do the first 3 meshes
		for (size_t iter = 0; iter < testModel.meshes.size(); iter++)
		{
			if (testModel.meshes[iter].isCollision)
			{
				continue;
			}

			for (uint8_t i = 0; i < testModel.meshes[iter].textures.size(); i++)
			{
				testModel.meshes[iter].textures[i].SetActive(i);
			}


			glBindVertexArray(testModel.meshes[iter].vertexArrayHandle);
			defProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, (GLsizei)testModel.meshes[iter].indices.size(), GL_UNSIGNED_INT, nullptr);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer.Unbind();
		glPopDebugGroup();
	}

	virtual void EdgeDetectionPass() override
	{
		GL_PUSH_DEBUG_GROUP();
		edgesBuffer.Bind();

		glDrawBuffers(1, &edgesBuffer.attachments["edge"].FBODesc.attachmentFormat);

		geometryBuffer.attachments["color"].SetActive(0);//color
		geometryBuffer.attachments["depth"].SetActive(1);//depth

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		edgeDetectionProgram.Use();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		edgesBuffer.Unbind();
		glPopDebugGroup();
	}

	virtual void BlendingWeightsPass() override
	{
		GL_PUSH_DEBUG_GROUP();
		weightsBuffer.Bind();

		glDrawBuffers(1, &weightsBuffer.attachments["blend"].FBODesc.attachmentFormat);

		edgesBuffer.attachments["edge"].SetActive(0);
		SMAAArea.SetActive(1);
		SMAASearch.SetActive(2);

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		blendingWeightProgram.Use();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		weightsBuffer.Unbind();
		glPopDebugGroup();
	}

	virtual void SMAAPass() override
	{
		GL_PUSH_DEBUG_GROUP();
		SMAABuffer.Bind();
		glDrawBuffers(1, &SMAABuffer.attachments["SMAA"].FBODesc.attachmentFormat);

		geometryBuffer.attachments["color"].SetActive(0); // color
		weightsBuffer.attachments["blend"].SetActive(1); //blending weights

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		SMAAProgram.Use();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		SMAABuffer.Unbind();
		glPopDebugGroup();
	}

	virtual void SMAAResolvePass()
	{
		GL_PUSH_DEBUG_GROUP();
		historyFrames[currentFrame]->Bind();
		GLenum drawBuffers[1] = {
			historyFrames[currentFrame]->attachments["color"].FBODesc.attachmentFormat
		};
		glDrawBuffers(1, drawBuffers);

		SMAABuffer.attachments["SMAA"].SetActive(0); //current color
		geometryBuffer.attachments["depth"].SetActive(1);//current depth

		historyFrames[!currentFrame]->attachments["color"].SetActive(2); //previous color
		historyFrames[!currentFrame]->attachments["depth"].SetActive(3);//previous depth

		geometryBuffer.attachments["velocity"].SetActive(4); //velocity

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		resolveProgram.Use();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		historyFrames[currentFrame]->Unbind();
		glPopDebugGroup();
	}

	void FinalPass(texture* tex1, texture* tex2)
	{
		GL_PUSH_DEBUG_GROUP();
		//draw directly to backbuffer
		tex1->SetActive(0);
		
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		if (enableCompare)
		{
			tex2->SetActive(1);
			compareProgram.Use();
		}

		else
		{
			finalProgram.Use();
		}
	
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glPopDebugGroup();
	}

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawSMAASettings();
		DrawTAASettings();
		//DrawCameraStats();
	}

	virtual void DrawTAASettings()
	{
		if(ImGui::BeginTabItem("TAA Settings"))
		{
			ImGui::Checkbox("enable Compare", &enableCompare);
			ImGui::SliderFloat("feedback factor", &taaUniforms->feedbackFactor, 0.0f, 1.0f);
			ImGui::InputFloat("max depth falloff", &taaUniforms->maxDepthFalloff, 0.01f);

			//velocity settings
			ImGui::Separator();
			//ImGui::SameLine();
			ImGui::Text("Velocity settings");
			ImGui::SliderFloat("Velocity scale", &taaUniforms->velocityScale, 0.0f, 10.0f);

			//jitter settings
			ImGui::Separator();
			//ImGui::SameLine();
			ImGui::DragFloat("halton scale", &jitterUniforms->haltonScale, 0.1f, 0.0f, 15.0f, "%.3f");
			ImGui::DragInt("halton index", &jitterUniforms->haltonIndex, 1.0f, 0, 128);
			ImGui::DragInt("enable dithering", &jitterUniforms->enableDithering, 1.0f, 0, 1);
			ImGui::DragFloat("dithering scale", &jitterUniforms->ditheringScale, 1.0f, 0.0f, 1000.0f, "%.3f");

			ImGui::EndTabItem();
		}
	}

	virtual void DrawBufferAttachments()
	{
		if (ImGui::BeginTabItem("framebuffers"))
		{
			for (auto iter : geometryBuffer.attachments | std::views::values)
			{
				ImGui::Image((ImTextureID*)iter.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter.GetUniformName().c_str());
			}

			ImGui::Image((ImTextureID*)edgesBuffer.attachments["edge"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", edgesBuffer.attachments["edge"].GetUniformName().c_str());

			ImGui::Image((ImTextureID*)weightsBuffer.attachments["blend"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", weightsBuffer.attachments["blend"].GetUniformName().c_str());

			ImGui::Image((ImTextureID*)SMAABuffer.attachments["SMAA"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", SMAABuffer.attachments["SMAA"].GetUniformName().c_str());

			for (auto iter : historyFrames)
			{
				for (auto iter2 : iter->attachments | std::views::values)
				{
					ImGui::Image((ImTextureID*)iter2.GetHandle(), ImVec2(512, 288),
						ImVec2(0, 1), ImVec2(1, 0));
					ImGui::SameLine();
					ImGui::Text("%s\n", iter2.GetUniformName().c_str());
				}
			}

			ImGui::EndTabItem();
		}
	}

	virtual void DrawCameraStats() override
	{
		//set up the view matrix
		if(ImGui::BeginTabItem("camera", &isGUIActive))
		{
			ImGui::DragFloat("near plane", &camera.nearPlane);
			ImGui::DragFloat("far plane", &camera.farPlane);
			ImGui::SliderFloat("Field of view", &camera.fieldOfView, 0, 90, "%.0f");

			ImGui::InputFloat("camera speed", &camera.speed, 0.f);
			ImGui::InputFloat("x sensitivity", &camera.xSensitivity, 0.f);
			ImGui::InputFloat("y sensitivity", &camera.ySensitivity, 0.f);
			ImGui::EndTabItem();
		}
	}

	virtual void ClearBuffers()
	{
		GL_PUSH_DEBUG_GROUP();
		//ok copy the current frame into the previous frame and clear the rest of the buffers

		historyFrames[!currentFrame]->Bind(); //clear the previous, the next frame current becomes previous
		historyFrames[!currentFrame]->ClearTexture(historyFrames[!currentFrame]->GetAttachmentRef("color"), clearColor);
		historyFrames[!currentFrame]->ClearTexture(historyFrames[!currentFrame]->GetAttachmentRef("depth"), clearColor2);
		//copy current depth to previous or vice versa?
		historyFrames[currentFrame]->GetAttachmentRef("depth").Copy(&geometryBuffer.GetAttachmentRef("depth")); //copy depth over

		glClear(GL_DEPTH_BUFFER_BIT);
		historyFrames[!currentFrame]->Unbind();

		geometryBuffer.Bind();
		geometryBuffer.ClearTexture(geometryBuffer.GetAttachmentRef("color"), clearColor);
		geometryBuffer.ClearTexture(geometryBuffer.GetAttachmentRef("velocity"), clearColor2);
		geometryBuffer.ClearTexture(geometryBuffer.GetAttachmentRef("depth"), clearColor2);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer.Unbind();

		SMAABuffer.Bind();
		SMAABuffer.ClearTexture(SMAABuffer.GetAttachmentRef("SMAA"), clearColor);
		SMAABuffer.Unbind();

		edgesBuffer.Bind();
		edgesBuffer.ClearTexture(edgesBuffer.GetAttachmentRef("edge"), clearColor2);
		edgesBuffer.Unbind();

		weightsBuffer.Bind();
		weightsBuffer.ClearTexture(weightsBuffer.GetAttachmentRef("blend"), clearColor2);
		weightsBuffer.Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
		reprojectUniforms->previousProjection = camera.projection;
		reprojectUniforms->previousView = camera.view;
		reprojectUniforms->prevTranslation = testModel.makeTransform(); //could be jittering the camera instead of the geometry?
		glPopDebugGroup();
	}

	virtual void ResizeBuffers(glm::ivec2 resolution) override
	{
		SMAAScene::ResizeBuffers(resolution);
		for (auto frame : historyFrames)
		{
			for (auto iter : frame->attachments | std::views::values)
			{
				frame->GetAttachmentRef(iter.uniformName).Resize(glm::ivec3(resolution, 1));
			}
		}
	}

	virtual void InitializeUniforms() override
	{
		SMAAScene::InitializeUniforms();

		auto taaBlock = &bufferHandler.uniformBlocks["taaSettings"];
		taaBlock->SetPayload(TAASettings_t());
		taaUniforms = taaBlock->GetPayload<TAASettings_t>();

		//auto sharpenBlock = &bufferHandler.uniformBlocks["sharpenSettings"];
		//sharpenBlock->SetPayload<sharpenSettings_t>(sharpenSettings_t());
		//sharpenSettings = sharpenBlock->GetPayload<sharpenSettings_t>();

		auto jitterBlock = &bufferHandler.uniformBlocks["jitterSettings"];
		jitterBlock->SetPayload(*jitterUniforms);
		jitterUniforms = jitterBlock->GetPayload<jitterSettings_t>();

		auto reprojectBlock = &bufferHandler.uniformBlocks["reprojectSettings"];
		reprojectBlock->SetPayload(reprojectSettings_t());
		reprojectUniforms = reprojectBlock->GetPayload<reprojectSettings_t>();
	}

	float CreateHaltonSequence(unsigned int index, int base)
	{
		float f = 1;
		float r = 0;
		int current = index;
		do
		{
			f = f / base;
			r = r + f * (current % base);
			current = (int)glm::floor(current / base);
		} while (current > 0);

		return r;
	}
};
#endif