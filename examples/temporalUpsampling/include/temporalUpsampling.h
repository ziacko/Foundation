#ifndef TEMPORALUPSAMPLE_H
#define TEMPORALUPSAMPLE_H

#include "../../TemporalAA/include/TemporalAA.h"
#include "glm/gtc/reciprocal.hpp"

struct resolutionSettings_t
{
	glm::vec2 resPercent;

	resolutionSettings_t(glm::vec2 resPercent = glm::vec2(1.0f))
	{
		this->resPercent = resPercent;
	}

	~resolutionSettings_t() { };
};

struct upsampleJitter_t
{
	glm::ivec2 points[4] = { glm::ivec2(0), glm::ivec2(0), glm::ivec2(0), glm::ivec2(0) };

	upsampleJitter_t(glm::ivec2 topLeft = glm::ivec2(0, 0), glm::ivec2 topRight = glm::ivec2(1, 0), glm::ivec2 bottomRight = glm::ivec2(1, 1), glm::ivec2 bottomLeft = glm::ivec2(0, 1))
	{
		points[0] = topLeft;
		points[1] = topRight;
		points[2] = bottomRight;
		points[3] = bottomLeft;
	}

	~upsampleJitter_t()	{};
};

struct upsampleSettings_t
{
	glm::vec4 metrics; //Z and W are output resolution, xy = rcp(zw). calculate rcp on c++ side
	glm::vec2 resolutionScaler;
	GLfloat blendingFactor;
	GLfloat reproSharpness;	
	GLfloat spatialFlickerTime;
	GLfloat timeMax;
	GLfloat timeMin;
	GLfloat edgeThreshold;
	GLfloat maxDepthFalloff;

	upsampleSettings_t(GLfloat maxDepthFalloff = 1.0f, GLfloat blendingFactor = 0.8f, GLfloat reproSharpness = 0.66f)
	{
		this->blendingFactor = blendingFactor;
		this->reproSharpness = reproSharpness;
		spatialFlickerTime = 0.0f;
		timeMax = 0.0f;
		timeMin = 1.0f;
		edgeThreshold = 1.0f;
		resolutionScaler = glm::vec2(1.0f);
		this->maxDepthFalloff = maxDepthFalloff;
		//this->metrics = metrics;
	}
};

class temporalUpsampling : public temporalAA
{
public:

	temporalUpsampling(
		const char* windowName = "Ziyad Barakat's portfolio (temporal Upsampling)",
		camera* texModelCamera = new camera(glm::vec2(1280, 720), 5.0f, camera::projection_t::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = "../../resources/shaders/TemporalUpsampling.txt",
		model_t* model = new model_t("../../resources/models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: temporalAA(windowName, texModelCamera, shaderConfigPath, model)
	{
		upsampleBuffer = new frameBuffer();
		stupid = new frameBuffer();
		upsampleUniforms = bufferHandler_t<upsampleSettings_t>();
		resolutionUniforms = bufferHandler_t<resolutionSettings_t>();
		upJitterUniforms	= bufferHandler_t<upsampleJitter_t>();
		interimRes = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);

		upsampleUniforms.data.metrics.z = windows[0]->settings.resolution.width * 2;
		upsampleUniforms.data.metrics.w = windows[0]->settings.resolution.height * 2;
		upsampleUniforms.data.metrics.x = 1.0f / upsampleUniforms.data.metrics.z;
		upsampleUniforms.data.metrics.y = 1.0f / upsampleUniforms.data.metrics.w;

		numPrevFrames = 3;
	}

	virtual ~temporalUpsampling() {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width / 2, windows[0]->settings.resolution.height / 2, 1);
		colorDesc.minFilterSetting = GL_LINEAR;
		colorDesc.magFilterSetting = GL_LINEAR;
		
		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("color", colorDesc));

		FBODescriptor velDesc;
		velDesc.format = gl_rg;
		velDesc.internalFormat = gl_rg16_snorm;
		velDesc.dataType = GL_FLOAT;
		velDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width / 2, windows[0]->settings.resolution.height / 2, 1);

		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("velocity", velDesc));

		FBODescriptor depthDesc;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.minFilterSetting = GL_LINEAR;
		depthDesc.magFilterSetting = GL_LINEAR;
		depthDesc.internalFormat = gl_depth_component32;
		depthDesc.attachmentType = FBODescriptor::attachmentType_t::depth;
		depthDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width / 2, windows[0]->settings.resolution.height / 2, 1);

		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("depth", depthDesc));

		for (unsigned int iter = 0; iter < numPrevFrames; iter++)
		{
			frameBuffer* newBuffer = new frameBuffer();

			newBuffer->Initialize();
			newBuffer->Bind();
			newBuffer->AddAttachment(new frameBuffer::attachment_t("color" + std::to_string(iter), colorDesc));

			historyFrames.push_back(newBuffer);
		}

		unJitteredBuffer->Initialize();
		unJitteredBuffer->Bind();
		unJitteredBuffer->AddAttachment(new frameBuffer::attachment_t("unJittered", colorDesc));
		unJitteredBuffer->AddAttachment(new frameBuffer::attachment_t("depth", depthDesc));

		colorDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height, 1);

		upsampleBuffer->Initialize();
		upsampleBuffer->Bind();
		upsampleBuffer->AddAttachment(new frameBuffer::attachment_t("upsampled", colorDesc));

		stupid->Initialize();
		stupid->Bind();
		stupid->AddAttachment(new frameBuffer::attachment_t("antiGhost", colorDesc));

		//geometry automatically gets assigned to 0
		antiGhostProgram = shaderPrograms[1]->handle;
		unjitteredProgram = shaderPrograms[2]->handle;
		upsampleProgram = shaderPrograms[3]->handle;
		compareProgram = shaderPrograms[4]->handle;
		finalProgram = shaderPrograms[5]->handle;

		frameBuffer::Unbind();

		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		interimRes = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) * upsampleUniforms.data.resolutionScaler;
	}

protected:

	frameBuffer* upsampleBuffer;
	frameBuffer* stupid;
	unsigned int upsampleProgram = 0;
	unsigned int antiGhostProgram = 0;
	bufferHandler_t<upsampleSettings_t>		upsampleUniforms;
	bufferHandler_t<resolutionSettings_t>	resolutionUniforms;
	bufferHandler_t<upsampleJitter_t>		upJitterUniforms;

	glm::vec2 interimRes;

	virtual void Update() override
	{
		temporalAA::Update();
		resolutionUniforms.Update();
		upJitterUniforms.Update();
		upsampleUniforms.Update();
	}

	virtual void Draw()
	{
		velocityUniforms.data.currentView = sceneCamera->view; //need to update the current view matrix
		sceneCamera->ChangeProjection(camera::projection_t::perspective);
		sceneCamera->resolution = glm::ivec2(windows[0]->settings.resolution.width / 2, windows[0]->settings.resolution.height / 2);
		sceneCamera->Update();

		UpdateDefaultBuffer();

		//defaultTimer->Begin();
		JitterPass(); //render current scene with jitter
		//defaultTimer->End();

		if (enableCompare)
		{
			sceneCamera->Update();
			UpdateDefaultBuffer();
			UnJitteredPass();
		}
		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		sceneCamera->resolution = glm::ivec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		sceneCamera->ChangeProjection(camera::projection_t::orthographic);
		UpdateDefaultBuffer();		
		
		UpsamplePass();

		AntiGhostPass();

		FinalPass(stupid->attachments[0], unJitteredBuffer->attachments[0]);
		
		DrawGUI(windows[0]);
		
		windows[0]->SwapDrawBuffers();
		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void JitterPass()
	{
		geometryBuffer->Bind();
		geometryBuffer->DrawAll();

		//we just need the first LOd so only do the first 3 meshes
		for (size_t iter = 0; iter < 3; iter++)
		{
			if (testModel->meshes[iter].isCollision)
			{
				continue;
			}

			testModel->meshes[iter].textures[0].SetActive(0);
			//add the previous depth?

			glBindVertexArray(testModel->meshes[iter].vertexArrayHandle);
			glUseProgram(this->programGLID);
			
			glCullFace(GL_BACK);

			glViewport(0, 0, windows[0]->settings.resolution.width / 2, windows[0]->settings.resolution.height / 2);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel->meshes[iter].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();
	}

	void UpsamplePass()
	{
		upsampleBuffer->attachments[0]->BindAsImage(0);

		//current frame
		geometryBuffer->attachments[0]->BindAsImage(1, gl_read_only); //current color
		historyFrames[0]->attachments[0]->BindAsImage(2, gl_read_only); //previous color
		historyFrames[1]->attachments[0]->BindAsImage(3, gl_read_only); //previous color
		historyFrames[2]->attachments[0]->BindAsImage(4, gl_read_only); //previous color

		//current depth and velocity info
		geometryBuffer->attachments[1]->BindAsImage(5, gl_read_only);
		geometryBuffer->attachments[2]->BindAsImage(6, gl_read_only);
		
		glUseProgram(upsampleProgram);
		glDispatchCompute(upsampleBuffer->attachments[0]->texDesc.dimensions.x / 16, upsampleBuffer->attachments[0]->texDesc.dimensions.y / 16, 1);

		upsampleBuffer->Unbind();
	}

	virtual void AntiGhostPass()
	{
		stupid->Bind();
		stupid->DrawAll();

		//current frame
		geometryBuffer->attachments[0]->SetActive(0); //current color
		historyFrames[0]->attachments[0]->SetActive(1); //previous color
		historyFrames[1]->attachments[0]->SetActive(2); //previous color
		historyFrames[2]->attachments[0]->SetActive(3); //previous color


		upsampleBuffer->attachments[0]->SetActive(4); // upsampled color
		//current depth and velocity info
		geometryBuffer->attachments[1]->SetActive(5); //current depth
		geometryBuffer->attachments[2]->SetActive(6); //current velocity
		
		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		glUseProgram(antiGhostProgram);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		stupid->Unbind();
	}

	virtual void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene3D::BuildGUI(windows[0], io);

		DrawBufferAttachments();
		DrawTAASettings();
		DrawResolutionSettings();
		DrawUpsampleSettings();
		//DrawSharpenSettings();
		//DrawJitterSettings();
	}

	void DrawResolutionSettings()
	{
		ImGui::Begin("resolution Settings");
		if (ImGui::SliderFloat("horizontal \%", &upsampleUniforms.data.resolutionScaler.x, 0.3f, 2.0f, "%.2f") ||
			ImGui::SliderFloat("vertical \%", &upsampleUniforms.data.resolutionScaler.y, 0.3f, 2.0f, "%.2f"))
		{
			interimRes = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) * upsampleUniforms.data.resolutionScaler;
			//defaultVertexBuffer->UpdateBuffer(interimRes);
			upsampleUniforms.Update();
		}
		ImGui::End();
	}

	virtual void DrawSharpenSettings()
	{
		ImGui::Begin("Sharpen settings");
		ImGui::Checkbox("enable sharpen", &enableSharpen);

		ImGui::SliderFloat("kernel 1", &sharpenSettings.data.kernel1, -1.0f, 1.0f);
		ImGui::SliderFloat("kernel 5", &sharpenSettings.data.kernel2, 0.0f, 10.0f, "%.5f", 1.0f);		

		ImGui::End();
	}

	virtual void DrawTAASettings()
	{
		ImGui::Begin("TAA Settings");
		ImGui::Text("performance | %f", defaultTimer->GetTimeMilliseconds());
		ImGui::Checkbox("enable Compare", &enableCompare);
		ImGui::SliderFloat("feedback factor", &taaUniforms.data.feedbackFactor, 0.0f, 1.0f);
		ImGui::InputFloat("max depth falloff", &taaUniforms.data.maxDepthFalloff, 0.01f);

		//velocity settings
		ImGui::Separator();
		ImGui::SameLine();
		ImGui::Text("Velocity settings");
		ImGui::SliderFloat("Velocity scale", &taaUniforms.data.velocityScale, 0.0f, 10.0f);

		//jitter settings
		ImGui::Separator();
		//ImGui::SameLine();
		ImGui::DragFloat("halton scale", &jitterUniforms.data.haltonScale, 0.1f, 0.0f, 15.0f, "%.3f");
		ImGui::DragInt("halton index",  &jitterUniforms.data.haltonIndex, 1.0f, 0, 128);
		ImGui::DragInt("enable dithering", &jitterUniforms.data.enableDithering, 1.0f, 0, 1);
		ImGui::DragFloat("dithering scale", &jitterUniforms.data.ditheringScale, 1.0f, 0.0f, 1000.0f, "%.3f");

		ImGui::End();
	}

	virtual void DrawBufferAttachments() override
	{
		ImGui::Begin("framebuffers");
		ImGui::Image((ImTextureID*)upsampleBuffer->attachments[0]->GetHandle(), ImVec2(512, 288),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Text("%s\n", upsampleBuffer->attachments[0]->GetUniformName().c_str());

		for (auto iter : geometryBuffer->attachments)
		{
			ImGui::Image((ImTextureID*)iter->GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", iter->GetUniformName().c_str());
		}

		for (auto iter : historyFrames)
		{
			for (auto iter2 : iter->attachments)
			{
				ImGui::Image((ImTextureID*)iter2->GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter2->GetUniformName().c_str());
			}
		}

		ImGui::Image((ImTextureID*)unJitteredBuffer->attachments[0]->GetHandle(), ImVec2(512, 288),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Text("%s\n", unJitteredBuffer->attachments[0]->GetUniformName().c_str());

		ImGui::Image((ImTextureID*)stupid->attachments[0]->GetHandle(), ImVec2(512, 288),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Text("%s\n", stupid->attachments[0]->GetUniformName().c_str());

		ImGui::End();
	}

	virtual void DrawUpsampleSettings()
	{
		ImGui::Begin("Upsample Settings");
		ImGui::DragFloat("blending factor", &upsampleUniforms.data.blendingFactor, 0.001f, 0.0f, 1.0f);
		ImGui::DragFloat("reprojection sharpness", &upsampleUniforms.data.reproSharpness, 0.001f, 0.0f, 1.0f);
		ImGui::DragFloat("spatial flicker time", &upsampleUniforms.data.spatialFlickerTime, 0.001f);
		ImGui::DragFloat("timeMax", &upsampleUniforms.data.timeMax, 0.001f);
		ImGui::DragFloat("timeMin", &upsampleUniforms.data.timeMin, 0.001f);
		ImGui::DragFloat("edge threshold", &upsampleUniforms.data.edgeThreshold, 0.001f);
		ImGui::DragFloat("max depth falloff", &upsampleUniforms.data.maxDepthFalloff, 1.0f, 0.0f, 2.0f);
		upsampleUniforms.Update();
		/*if (ImGui::DragFloat4("metrics", &upsampleUniforms.data.metrics[0]))
		{
			
		}*/
		ImGui::End();
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		for (auto frame : historyFrames)
		{
			for (auto iter : frame->attachments)
			{
				iter->Resize(glm::ivec3(resolution / 2, 1));
			}
		}

		geometryBuffer->attachments[0]->Resize(glm::ivec3(resolution / 2, 1));
		geometryBuffer->attachments[1]->Resize(glm::ivec3(resolution / 2, 1));
		geometryBuffer->attachments[2]->Resize(glm::ivec3(resolution / 2, 1));

		//sharpenBuffer->attachments[0]->Resize(resolution);
		unJitteredBuffer->attachments[0]->Resize(glm::ivec3(resolution / 2, 1));
		unJitteredBuffer->attachments[1]->Resize(glm::ivec3(resolution / 2, 1));

		upsampleBuffer->attachments[0]->Resize(glm::ivec3(resolution, 1));

		stupid->attachments[0]->Resize(glm::ivec3(resolution, 1));


		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);

		upsampleUniforms.data.metrics.z = windows[0]->settings.resolution.width * 2;
		upsampleUniforms.data.metrics.w = windows[0]->settings.resolution.height * 2;
		upsampleUniforms.data.metrics.x = 1.0f / upsampleUniforms.data.metrics.z;
		upsampleUniforms.data.metrics.y = 1.0f / upsampleUniforms.data.metrics.w;
		upsampleUniforms.Update();
	}

	virtual void ClearBuffers()
	{
		//ok copy the current frame into the previous frame and clear the rest of the buffers	
		float clearColor1[4] = { 0.05f, 0.05f, 0.05f, 0.0f };
		float clearColor2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float clearColor3[4] = { 1.0f, 0.0f, 0.0f, 0.0f }; //this is for debugging only!

		//clear the previous, the next frame current becomes previous
		/*historyFrames[3]->ClearTexture(historyFrames[2]->attachments[0], clearColor1);
		historyFrames[3]->attachments[0]->Copy(historyFrames[2]->attachments[0]);*/
		//copy current depth to previous or vice versa?

		historyFrames[2]->Bind();
		historyFrames[2]->attachments[0]->Copy(historyFrames[1]->attachments[0]); //copy 3 to 2
		historyFrames[1]->Bind();
		historyFrames[1]->attachments[0]->Copy(historyFrames[0]->attachments[0]); //copy 2 to 1
		historyFrames[0]->Bind();
		historyFrames[0]->attachments[0]->Copy(geometryBuffer->attachments[0]); //copy current to 0

		glClear(GL_DEPTH_BUFFER_BIT);
		historyFrames[currentFrame]->Unbind();

		geometryBuffer->Bind();
		geometryBuffer->ClearTexture(geometryBuffer->attachments[0], clearColor1);
		geometryBuffer->ClearTexture(geometryBuffer->attachments[1], clearColor2);
		geometryBuffer->ClearTexture(geometryBuffer->attachments[2], clearColor2);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();

		unJitteredBuffer->Bind();
		unJitteredBuffer->ClearTexture(unJitteredBuffer->attachments[0], clearColor1);
		glClear(GL_DEPTH_BUFFER_BIT);
		unJitteredBuffer->Unbind();

		/*upsampleBuffer->Bind();
		upsampleBuffer->ClearTexture(upsampleBuffer->attachments[0], clearColor1);
		upsampleBuffer->Unbind();*/

		sceneCamera->ChangeProjection(camera::projection_t::perspective);
		velocityUniforms.data.previousProjection = sceneCamera->projection;
		velocityUniforms.data.previousView = sceneCamera->view;
		velocityUniforms.data.prevTranslation = testModel->makeTransform(); //could be jittering the camera instead of the geometry?
		velocityUniforms.Update();
	}

	virtual void InitializeUniforms() override
	{
		defaultPayload = bufferHandler_t<defaultUniformBuffer>(sceneCamera);
		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);

		defaultPayload.data.resolution = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		defaultPayload.data.projection = sceneCamera->projection;
		defaultPayload.data.translation = sceneCamera->translation;
		defaultPayload.data.view = sceneCamera->view;

		defaultPayload.Initialize(0);
		velocityUniforms.Initialize(1);
		taaUniforms.Initialize(2);
		upsampleUniforms.Initialize(3);
		jitterUniforms.Initialize(4);
		resolutionUniforms.Initialize(5);
		upJitterUniforms.Initialize(6);

		SetupVertexBuffer();
	}
};

#endif