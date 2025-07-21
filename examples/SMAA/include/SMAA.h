#pragma once

#include "scene3D.h"
#include "FrameBuffer.h"

enum class EdgeDetectionMode_e
{
	luma = 0,
	color = 1,
	depth = 2
};

struct SMAASettings_t
{
	glm::vec4	rtMetrics = glm::vec4(1.0 / defaultWindowSize.x, 1.0 / defaultWindowSize.y, defaultWindowSize.x, defaultWindowSize.y);
	float		threshold;
	float		contrastAdaptationFactor;

	int32_t		maxSearchSteps;
	int32_t		maxSearchStepsDiag;
	int32_t		cornerRounding;
	int32_t		edgeDetectionMode;

	explicit SMAASettings_t(const glm::ivec2& resolution = defaultWindowSize, const float threshold = 0.05, const float CAFactor = 2.0f,
		const uint8_t maxSearchSteps = 32, const uint8_t maxSearchStepsDiag = 16, const uint8_t cornerRounding = 25,
		const EdgeDetectionMode_e& edgeDetectionMode = EdgeDetectionMode_e::color)
	{
		this->rtMetrics = glm::vec4(1.0 / resolution.x, 1.0 / resolution.y, resolution.x, resolution.y);
		this->threshold = threshold;
		this->contrastAdaptationFactor = CAFactor;
		this->maxSearchSteps = maxSearchSteps;
		this->maxSearchStepsDiag = maxSearchStepsDiag;
		this->cornerRounding = cornerRounding;
		this->edgeDetectionMode = (int32_t)edgeDetectionMode;
	}
};

class SMAAScene : public scene3D
{
public:

	explicit SMAAScene(
		const char* windowName = "Ziyad Barakat's portfolio (SMAA)",
		const camera_t& camera = camera_t(defaultWindowSize, defaultCameraSpeed, camera_t::projection_e::perspective),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		const model_t& model = model_t("models/SoulSpear/SoulSpear.fbx"))
		: scene3D(windowName, camera, shaderConfigPath, model)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		//soulspear is loaded at an awkward angle so let's hack this
		this->camera.Roll(glm::radians(180.0f));
		this->camera.Pitch(glm::radians(270.0f));

		this->camera.position.z -= 1.0f;

		geometryBuffer = frameBuffer();
		edgesBuffer = frameBuffer();
		weightsBuffer = frameBuffer();
		SMAABuffer = frameBuffer();

		SMAAArea = texture("textures/SMAA/AreaTexDX_Flipped.png");
		SMAASearch = texture("assets/textures/SMAA/SearchTex_Flipped.dds");
	}

	~SMAAScene() override = default;

	void Initialize() override
	{
		scene3D::Initialize();

		SMAAArea.LoadTexture();
		SMAASearch.LoadTexture();

		SMAASearch.SetMagFilter(GL_NEAREST);
		SMAASearch.SetMinFilter(GL_NEAREST);

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);
		colorDesc.dataType = GL_FLOAT;
		colorDesc.format = GL_RGBA;
		colorDesc.internalFormat = GL_RGBA32F;
		colorDesc.wrapRSetting = GL_CLAMP_TO_EDGE;
		colorDesc.wrapTSetting = GL_CLAMP_TO_EDGE;
		colorDesc.wrapSSetting = GL_CLAMP_TO_EDGE;

		FBODescriptor depthDesc;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.wrapRSetting = GL_CLAMP_TO_EDGE;
		depthDesc.wrapTSetting = GL_CLAMP_TO_EDGE;
		depthDesc.wrapSSetting = GL_CLAMP_TO_EDGE;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT32F;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer.Initialize();
		geometryBuffer.Bind();

		geometryBuffer.AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer.AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		edgesBuffer.Initialize();
		edgesBuffer.Bind();

		FBODescriptor edgeDesc;
		edgeDesc.format = GL_RG;
		edgeDesc.dataType = GL_FLOAT;
		edgeDesc.internalFormat = GL_RG32F;
		edgeDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);
		edgeDesc.wrapRSetting = GL_CLAMP_TO_EDGE;
		edgeDesc.wrapTSetting = GL_CLAMP_TO_EDGE;
		edgeDesc.wrapSSetting = GL_CLAMP_TO_EDGE;

		edgesBuffer.AddAttachment(frameBuffer::attachment_t("edge", edgeDesc));

		weightsBuffer.Initialize();
		weightsBuffer.Bind();

		FBODescriptor weightsDesc;
		weightsDesc = colorDesc;
		weightsDesc.dataType = GL_FLOAT;
		weightsDesc.internalFormat = GL_RGBA32F;
		weightsDesc.wrapRSetting = GL_CLAMP_TO_EDGE;
		weightsDesc.wrapTSetting = GL_CLAMP_TO_EDGE;
		weightsDesc.wrapSSetting = GL_CLAMP_TO_EDGE;

		weightsBuffer.AddAttachment(frameBuffer::attachment_t("blend", weightsDesc));

		SMAABuffer.Initialize();
		SMAABuffer.Bind();
		SMAABuffer.AddAttachment(frameBuffer::attachment_t("SMAA", colorDesc));

		geometryProgram = &shaderProgramsMap["geometry"];
		edgeDetectionProgram = &shaderProgramsMap["edgeDetection"];
		blendingWeightProgram = &shaderProgramsMap["blendingWeight"];
		SMAAProgram = &shaderProgramsMap["SMAA"];
		compareProgram = &shaderProgramsMap["compare"];
		finalProgram = &shaderProgramsMap["final"];

		frameBuffer::Unbind();

		glDisable(GL_MULTISAMPLE);
	}

protected:

	frameBuffer					geometryBuffer;
	frameBuffer					edgesBuffer;
	frameBuffer					weightsBuffer;
	frameBuffer					SMAABuffer;

	texture						SMAAArea;
	texture						SMAASearch;

	bufferHandler_t<SMAASettings_t>		SMAASettings;

	ShaderProgram_t* geometryProgram = nullptr;
	ShaderProgram_t* edgeDetectionProgram = nullptr;
	ShaderProgram_t* blendingWeightProgram = nullptr;
	ShaderProgram_t* SMAAProgram = nullptr;
	ShaderProgram_t* compareProgram = nullptr;
	ShaderProgram_t* finalProgram = nullptr;

	int currentTexture = 0;
	bool enableCompare = true;

	void Update() override
	{
		manager->PollForEvents();
		if (lockedFrameRate > 0)
		{
			clock.UpdateClockFixed(lockedFrameRate);
		}
		else
		{
			clock.UpdateClockAdaptive();
		}

		defaultPayload.data.deltaTime = (float)clock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)clock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / clock.GetDeltaTime());
		defaultPayload.data.totalFrames++;
		defaultPayload.data.resolution = camera.resolution;

		SMAASettings.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	}

	void UpdateDefaultBuffer()
	{
		camera.UpdateProjection();
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.view = camera.view;
		defaultPayload.data.resolution = camera.resolution;
		if (camera.currentProjectionType == camera_t::projection_e::perspective)
		{
			defaultPayload.data.translation = testModel.makeTransform();
		}

		else
		{
			defaultPayload.data.translation = camera.translation;
		}
		defaultPayload.data.deltaTime = (float)clock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)clock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / clock.GetDeltaTime());

		defaultPayload.Update();
		//defaultVertexBuffer.UpdateBuffer(defaultPayload.data.resolution);
	}

	void Draw() override
	{
		camera.ChangeProjection(camera_t::projection_e::perspective);
		camera.Update();
		UpdateDefaultBuffer();

		GeometryPass(); //render current scene with jitter
		
		EdgeDetectionPass();
		BlendingWeightsPass();
		SMAAPass();

		FinalPass(&SMAABuffer.attachments["SMAA"], &geometryBuffer.attachments["color"]);
		
		DrawGUI(window);
		
		manager->SwapDrawBuffers(window);
		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	}

	virtual void GeometryPass()
	{
		geometryBuffer.Bind();

		glDrawBuffers(1, &geometryBuffer.attachments["color"].FBODesc.attachmentFormat);

		//we just need the first LOd so only do the first 3 meshes
		for (size_t iter = 0; iter < 1; iter++)
		{
			if (testModel.meshes[iter].isCollision)
			{
				continue;
			}

			testModel.meshes[iter].textures[0].SetActive(0);
			//add the previous depth?

			glBindVertexArray(testModel.meshes[iter].vertexArrayHandle);
			glUseProgram(geometryProgram->handle);

			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel.meshes[iter].indices.size(), GL_UNSIGNED_INT, nullptr);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		frameBuffer::Unbind();
	}

	virtual void EdgeDetectionPass()
	{
		edgesBuffer.Bind();

		glDrawBuffers(1, &edgesBuffer.attachments["edge"].FBODesc.attachmentFormat);

		geometryBuffer.attachments["color"].SetActive(0);//color
		geometryBuffer.attachments["depth"].SetActive(1);//depth

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glUseProgram(edgeDetectionProgram->handle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		frameBuffer::Unbind();
	}

	virtual void BlendingWeightsPass()
	{
		weightsBuffer.Bind();

		glDrawBuffers(1, &weightsBuffer.attachments["blend"].FBODesc.attachmentFormat);

		edgesBuffer.attachments["edge"].SetActive(0);
		SMAAArea.SetActive(1);
		SMAASearch.SetActive(2);

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glUseProgram(blendingWeightProgram->handle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		frameBuffer::Unbind();
	}

	virtual void SMAAPass()
	{
		SMAABuffer.Bind();
		glDrawBuffers(1, &SMAABuffer.attachments["SMAA"].FBODesc.attachmentFormat);

		//current frame
		geometryBuffer.attachments["color"].SetActive(0); // color
		weightsBuffer.attachments["blend"].SetActive(1); //blending weights

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glUseProgram(SMAAProgram->handle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		frameBuffer::Unbind();
	}

	void FinalPass(const texture* tex1, const texture* tex2) const
	{
		//draw directly to backbuffer
		tex1->SetActive(0);
		
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		if (enableCompare)
		{
			tex2->SetActive(1);
			glUseProgram(compareProgram->handle);
		}

		else
		{
			glUseProgram(finalProgram->handle);
		}
	
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawSMAASettings();
	}

	virtual void DrawBufferAttachments()
	{
		if (ImGui::BeginTabItem("framebuffers"))
		{
			for (const auto& val : geometryBuffer.attachments | std::views::values)
			{
				ImGui::Image((ImTextureID)val.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", val.GetUniformName().c_str());
			}

			ImGui::Image((ImTextureID)edgesBuffer.attachments["edge"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", edgesBuffer.attachments["edge"].GetUniformName().c_str());

			ImGui::Image((ImTextureID)weightsBuffer.attachments["blend"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", weightsBuffer.attachments["blend"].GetUniformName().c_str());

			ImGui::Image((ImTextureID)SMAABuffer.attachments["SMAA"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", SMAABuffer.attachments["SMAA"].GetUniformName().c_str());
			ImGui::EndTabItem();
		}
	}

	virtual void ClearBuffers()
	{
		//move clearColor into a float array
		geometryBuffer.Bind();
		frameBuffer::ClearTexture(geometryBuffer.attachments["color"], value_ptr(clearColor));
		glClear(GL_DEPTH_BUFFER_BIT);
		frameBuffer::Unbind();

		SMAABuffer.Bind();
		frameBuffer::ClearTexture(SMAABuffer.attachments["SMAA"], value_ptr(clearColor2));
		frameBuffer::Unbind();

		edgesBuffer.Bind();
		frameBuffer::ClearTexture(edgesBuffer.attachments["edge"], value_ptr(clearColor2));
		frameBuffer::Unbind();

		weightsBuffer.Bind();
		frameBuffer::ClearTexture(weightsBuffer.attachments["blend"], value_ptr(clearColor2));
		frameBuffer::Unbind();
	}

	virtual void ResizeBuffers(const glm::ivec2 resolution)
	{
		for (auto val : geometryBuffer.attachments | std::views::values)
		{
			val.Resize(glm::ivec3(resolution, 1));
		}

		edgesBuffer.attachments["edge"].Resize(glm::ivec3(resolution, 1));
		weightsBuffer.attachments["blend"].Resize(glm::ivec3(resolution, 1));
		SMAABuffer.attachments["SMAA"].Resize(glm::ivec3(resolution, 1));
	}

	void HandleWindowResize(const tWindow* window, const vec2_t<uint16_t>& dimensions) override
	{
		defaultPayload.data.resolution = glm::ivec2(dimensions.width, dimensions.height);
		ResizeBuffers(glm::ivec2(dimensions.x, dimensions.y));
	}

	void HandleMaximize(const tWindow* window) override
	{
		defaultPayload.data.resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		ResizeBuffers(defaultPayload.data.resolution);
	}

	void InitializeUniforms() override
	{
		defaultPayload = bufferHandler_t<defaultUniformBuffer>(camera);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		defaultPayload.data.resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.translation = camera.translation;
		defaultPayload.data.view = camera.view;

		defaultPayload.Initialize(0);
		SMAASettings.Initialize(1);

		defaultVertexBuffer.SetupDefault();
	}

	void DrawSMAASettings()
	{
		if (ImGui::BeginTabItem("SMAA Settings"))
		{
			ImGui::Checkbox("enable Compare", &enableCompare);
			ImGui::SliderFloat("threshold", &SMAASettings.data.threshold, 0.001f, 1.0f, "%0.5f");
			ImGui::SliderFloat("contrast adaption factor", &SMAASettings.data.contrastAdaptationFactor, 0.1f, 5.0f, "0.5f");
			ImGui::SliderInt("max search steps", &SMAASettings.data.maxSearchSteps, 0, 255);
			ImGui::SliderInt("max search steps diagonal", &SMAASettings.data.maxSearchStepsDiag, 0, 255);
			ImGui::SliderInt("corner rounding", &SMAASettings.data.cornerRounding, 0, 255);

			//set a list box for edge detection modes
			static int edgeDetectionPick = (int32_t)SMAASettings.data.edgeDetectionMode;
			const std::vector edgeDetectionSettings = { "luma", "color", "depth" };
			ImGui::ListBox("Edge Detection Mode", &edgeDetectionPick, edgeDetectionSettings.data(), edgeDetectionSettings.size());
			switch (edgeDetectionPick)
			{
				case 0: SMAASettings.data.edgeDetectionMode = (int32_t)EdgeDetectionMode_e::luma; break;
				case 1: SMAASettings.data.edgeDetectionMode = (int32_t)EdgeDetectionMode_e::color; break;
				case 2: SMAASettings.data.edgeDetectionMode = (int32_t)EdgeDetectionMode_e::depth; break;
				default: break;
			}
			ImGui::EndTabItem();
		}
	}
};