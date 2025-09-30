#ifndef TEMPORALAA_H
#define TEMPORALAA_H

#include "scene3D.h"
#include "FrameBuffer.h"
#include "HaltonSequence.h"

typedef enum { SMAA = 0, INSIDE, INSIDE2, CUSTOM, EXPERIMENTAL } TAAResolves_t;

struct temporalAAFrame
{
	temporalAAFrame(frameBuffer::attachment_t* color, frameBuffer::attachment_t* depth, frameBuffer::attachment_t* position)
	{
		if (color != nullptr && depth != nullptr && position != nullptr)
		{
			attachments.push_back(color);
			attachments.push_back(depth);
			attachments.push_back(position);
		}
	}

	std::vector<frameBuffer::attachment_t*> attachments;

	//also grab the view, translation and projection
};

struct jitterSettings_t
{
	glm::vec2			haltonSequence[128]{};
	float				haltonScale;
	int					haltonIndex;
	int					enableDithering;
	float				ditheringScale;

	jitterSettings_t()
	{
		haltonIndex = 16;
		enableDithering = 1;
		haltonScale = 100.0f;
		ditheringScale = 0.0f;
	}

	~jitterSettings_t() {};
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

struct sharpenSettings_t
{
	GLfloat			kernel1;
	GLfloat			kernel2;

	sharpenSettings_t(
		GLfloat kernel1 = -0.125f, GLfloat kernel2 = 1.75f)
	{
		this->kernel1 = kernel1;
		this->kernel2 = kernel2;
	}

	~sharpenSettings_t() { };
};

class temporalAA : public scene3D
{
public:

	temporalAA(
		const char* windowName = "Ziyad Barakat's portfolio (temporal AA)",
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		model_t model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		//glEnable(gl_clip_distance0);
		glDepthFunc(GL_LESS);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

		this->camera.position.y -= 100.0f;

		//soulspear is loaded at an awkward angle so let's hack this
		//this->camera.Roll(glm::radians(270.0f));
		//this->camera.Pitch(glm::radians(180.0f));

		geometryBuffer = new frameBuffer();
		unJitteredBuffer = new frameBuffer();
		//sharpenBuffer = new frameBuffer();

		velocityUniforms = bufferHandler_t<reprojectSettings_t>();
		taaUniforms = bufferHandler_t<TAASettings_t>();
		sharpenSettings = bufferHandler_t<sharpenSettings_t>();
		jitterUniforms = bufferHandler_t<jitterSettings_t>();

		for (int iter = 0; iter < 128; iter++)
		{
			jitterUniforms.data.haltonSequence[iter] = glm::vec2(CreateHaltonSequence(iter + 1, 2), CreateHaltonSequence(iter + 1, 3));
		}
		//glGenQueries(1, &defaultQuery);
		//glGenQueries(1, &TAAQuery);
	}

	virtual ~temporalAA() {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		glGenQueries(1, &jitterQuery.handle);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(frameBuffer::attachment_t("color", colorDesc));

		FBODescriptor velDesc;
		velDesc.format = GL_RG;
		velDesc.internalFormat = GL_RG16_SNORM;
		velDesc.dataType = GL_FLOAT;
		velDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer->AddAttachment(frameBuffer::attachment_t("velocity", velDesc));

		FBODescriptor depthDesc;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		for (unsigned int iter = 0; iter < numPrevFrames; iter++)
		{
			frameBuffer* newBuffer = new frameBuffer();

			newBuffer->Initialize();
			newBuffer->Bind();
			newBuffer->AddAttachment(frameBuffer::attachment_t("color", colorDesc));
			newBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

			historyFrames.push_back(newBuffer);
		}

		/*sharpenBuffer->Bind();
		sharpenBuffer->AddAttachment(new frameBuffer::attachment_t(frameBuffer::attachment_t::attachmentType_t::color,
			"sharp", glm::vec2(windows[0]->resolution.width, windows[0]->resolution.height)));*/

		unJitteredBuffer->Initialize();
		unJitteredBuffer->Bind();
		unJitteredBuffer->AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		unJitteredBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		//geometry automatically gets assigned to 0
		defProgram = shaderProgramsMap["geometry"];
		smoothProgram = shaderProgramsMap["smooth"];
		unjitteredProgram = shaderProgramsMap["unjittered"];
		//sharpenProgram = shaderPrograms[2]->handle;
		compareProgram = shaderProgramsMap["compare"];
		finalProgram = shaderProgramsMap["final"];

		averageGPUTimes.reserve(10);

		frameBuffer::Unbind();

		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
	}

protected:

	std::vector<frameBuffer*> historyFrames;
	frameBuffer* geometryBuffer;
	frameBuffer* unJitteredBuffer;
	//frameBuffer* sharpenBuffer;

	shaderProgram_t smoothProgram;
	shaderProgram_t unjitteredProgram;
	//unsigned int sharpenProgram = 0;
	shaderProgram_t compareProgram;
	shaderProgram_t finalProgram;

	shaderProgram_t jitterQuery;
	shaderProgram_t defaultQuery;
	shaderProgram_t TAAQuery;

	GLuint numPrevFrames = 2; //don't need this right now

	bufferHandler_t<reprojectSettings_t>	velocityUniforms;

	bufferHandler_t<TAASettings_t>			taaUniforms;

	std::vector<const char*>				TAAResolveSettings = { "SMAA", "Inside", "Inside2", "Custom", "Experimental" };
	bool enableCompare = true;

	bufferHandler_t<sharpenSettings_t>		sharpenSettings;
	bool enableSharpen = false;

	bufferHandler_t<jitterSettings_t>		jitterUniforms;

	bool currentFrame = 0;

	std::vector<uint64_t> averageGPUTimes;

	virtual void Update() override
	{
		scene::Update();

		taaUniforms.Update();
		sharpenSettings.Update();
		jitterUniforms.Update();

		currentFrame = ((defaultPayload.data.totalFrames % 2) == 0) ? 0 : 1;//if even frame then write to 1 and read from 0 and vice versa
	}

	virtual void Draw() override
	{
		velocityUniforms.data.currentView = camera.view; //need to update the current view matrix
		camera.ChangeProjection(camera_t::projection_e::perspective);
		camera.Update();

		UpdateDefaultUniforms(camera, clock, &testModel);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		JitterPass(); //render current scene with jitter

		if (enableCompare)
		{
			UnJitteredPass();
		}

		glDisable(GL_BLEND);

		camera.ChangeProjection(camera_t::projection_e::orthographic);
		UpdateDefaultUniforms(camera, clock, &testModel);
		//UpdateDefaultBuffer();
		
		TAAPass(); //use the positions, colors, depth and velocity to smooth the final image

		//SharpenPass();

		FinalPass(&historyFrames[currentFrame]->attachments["color"], &unJitteredBuffer->attachments["color"]);
	}

	virtual void JitterPass()
	{
		GL_PUSH_DEBUG_GROUP();
		geometryBuffer->Bind();
		geometryBuffer->DrawAll();

		//we just need the first LOd so only do the first 3 meshes
		for (auto& meshIter : testModel.meshes)
		{
			if (meshIter.isCollision)
			{
				continue;
			}

			meshIter.textures[0].SetActive(0);
			//add the previous depth?

			meshIter.Bind();
			defProgram.Use();
			
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, meshIter.indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();
		glPopDebugGroup();
	}

	virtual void UnJitteredPass()
	{
		GL_PUSH_DEBUG_GROUP();
		unJitteredBuffer->Bind();
		unJitteredBuffer->DrawAll();

		//we just need the first LOd so only do the first 3 meshes
		for (auto& meshIter : testModel.meshes)
		{
			if (meshIter.isCollision)
			{
				continue;
			}

			meshIter.textures[0].SetActive(0);
			//add the previous depth?

			meshIter.Bind();
			unjitteredProgram.Use();
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, meshIter.indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		unJitteredBuffer->Unbind();
		glPopDebugGroup();
	}

	virtual void TAAPass()
	{
		GL_PUSH_DEBUG_GROUP();
		historyFrames[currentFrame]->Bind();
		historyFrames[currentFrame]->attachments["color"].Draw();

		//current frame
		geometryBuffer->attachments["color"].SetActive(0); //current color
		geometryBuffer->attachments["depth"].SetActive(1); //current depth

		//previous frames
		historyFrames[!currentFrame]->attachments["color"].SetActive(2); //previous color
		historyFrames[!currentFrame]->attachments["depth"].SetActive(3); //previous depth

		geometryBuffer->attachments["velocity"].SetActive(4); //velocity
		
		defaultVertexBuffer.Bind();
		smoothProgram.Use();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		historyFrames[currentFrame]->Unbind();

		glPopDebugGroup();
	}

	virtual void FinalPass(texture* tex1, texture* tex2)
	{
		GL_PUSH_DEBUG_GROUP();
		//draw directly to backbuffer		
		tex1->SetActive(0);
		
		defaultVertexBuffer.Bind();
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

	virtual void PostDraw() override
	{
		DrawGUI(window);

		manager->SwapDrawBuffers(window);
		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawTAASettings();
		//DrawSharpenSettings();
		//DrawJitterSettings();
	}

	virtual void DrawSharpenSettings()
	{
		if (ImGui::BeginTabItem("Sharpen settings"))
		{
			ImGui::Checkbox("enable sharpen", &enableSharpen);

			ImGui::SliderFloat("kernel 1", &sharpenSettings.data.kernel1, -1.0f, 1.0f);
			ImGui::SliderFloat("kernel 5", &sharpenSettings.data.kernel2, 0.0f, 10.0f, "%.5f", 1.0f);

			ImGui::EndTabItem();
		}
	}

	virtual void DrawTAASettings()
	{
		if (ImGui::BeginTabItem("TAA Settings"))
		{
			//ImGui::Text("performance | %f", defaultTimer->GetTimeMilliseconds());
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

			ImGui::EndTabItem();
		}
	}

	virtual void DrawBufferAttachments()
	{
		if (ImGui::BeginTabItem("framebuffers"))
		{
			for (auto iter : geometryBuffer->attachments | std::views::values)
			{
				ImGui::Image((ImTextureID*)iter.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter.GetUniformName().c_str());
			}

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

			ImGui::Image((ImTextureID*)unJitteredBuffer->attachments["color"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", unJitteredBuffer->attachments["color"].GetUniformName().c_str());

			ImGui::EndTabItem();
		}
	}

	virtual void ClearBuffers() override
	{
		GL_PUSH_DEBUG_GROUP();
		//ok copy the current frame into the previous frame and clear the rest of the buffers
		historyFrames[!currentFrame]->Bind(); //clear the previous, the next frame current becomes previous
		historyFrames[!currentFrame]->ClearTexture(historyFrames[!currentFrame]->GetAttachmentRef("color"), clearColor);
		historyFrames[!currentFrame]->ClearTexture(historyFrames[!currentFrame]->GetAttachmentRef("depth"), clearDepth);
		//copy current depth to previous or vice versa?
		historyFrames[currentFrame]->GetAttachmentRef("depth").Copy(&geometryBuffer->GetAttachmentRef("depth")); //copy depth over

		glClear(GL_DEPTH_BUFFER_BIT);
		historyFrames[!currentFrame]->Unbind();

		geometryBuffer->Bind();
		geometryBuffer->ClearTexture(geometryBuffer->GetAttachmentRef("color"), clearColor);
		geometryBuffer->ClearTexture(geometryBuffer->GetAttachmentRef("velocity"), clearColor2);
		geometryBuffer->ClearTexture(geometryBuffer->GetAttachmentRef("depth"), clearDepth);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();

		unJitteredBuffer->Bind();
		unJitteredBuffer->ClearTexture(unJitteredBuffer->GetAttachmentRef("color"), clearColor);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		unJitteredBuffer->Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
		velocityUniforms.data.previousProjection = camera.projection;
		velocityUniforms.data.previousView = camera.view;
		velocityUniforms.data.prevTranslation = testModel.makeTransform(); //could be jittering the camera instead of the geometry?
		velocityUniforms.Update();

		glPopDebugGroup();
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		for (auto frame : historyFrames)
		{
			for (auto& iter : frame->attachments | std::views::values)
			{
				frame->GetAttachmentRef(iter.uniformName).Resize(glm::ivec3(resolution, 1));
			}
		}

		for (auto& iter : geometryBuffer->attachments | std::views::values)
		{
			geometryBuffer->GetAttachmentRef(iter.uniformName).Resize(glm::ivec3(resolution, 1));
		}

		for (auto& iter : unJitteredBuffer->attachments | std::views::values)
		{
			unJitteredBuffer->GetAttachmentRef(iter.uniformName).Resize(glm::ivec3(resolution, 1));
		}

		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
	}

	virtual void HandleWindowResize(const tWindow* window, const tw::vec2_t<uint16_t>& dimensions) override
	{
		defaultPayload.data.resolution = glm::ivec2(dimensions.width, dimensions.height);	
		ResizeBuffers(glm::ivec2(dimensions.x, dimensions.y));
	}

	virtual void HandleMaximize(const tWindow* window) override
	{
		defaultPayload.data.resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		ResizeBuffers(defaultPayload.data.resolution);
	}

	virtual void InitializeUniforms() override
	{
		scene3D::InitializeUniforms();
		velocityUniforms.Initialize(1);
		taaUniforms.Initialize(2);
		sharpenSettings.Initialize(3);
		jitterUniforms.Initialize(4);
	}
};

#endif