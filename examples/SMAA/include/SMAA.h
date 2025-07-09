#pragma once

#include "scene3D.h"
#include "FrameBuffer.h"

struct SMAASettings_t
{
	//float				weightScale;
	float				threshold;
	float				contrastAdaptationFactor;

	int		maxSearchSteps;
	int		maxSearchStepsDiag;
	int		cornerRounding;

	explicit SMAASettings_t(const float threshold = 0.05, const float CAFactor = 2.0f, const unsigned int maxSearchSteps = 32, const unsigned int maxSearchStepsDiag = 16, const unsigned int cornerRounding = 25)
	{
		this->threshold = threshold;
		this->contrastAdaptationFactor = CAFactor;
		this->maxSearchSteps = maxSearchSteps;
		this->maxSearchStepsDiag = maxSearchStepsDiag;
		this->cornerRounding = cornerRounding;
	}
};

class SMAAScene : public scene3D
{
public:

	explicit SMAAScene(
		const char* windowName = "Ziyad Barakat's portfolio (SMAA)",
		const camera_t& texModelCamera = camera_t(glm::vec2(1280, 720), 0.31415f, camera_t::projection_e::perspective, 0.01f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		model_t model = model_t("models/SoulSpear/SoulSpear.fbx"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, std::move(model))
	{
		//glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		//glHint(gl_generate_mipmap_hint, GL_NICEST);

		geometryBuffer = frameBuffer();
		edgesBuffer = frameBuffer();
		weightsBuffer = frameBuffer();
		SMAABuffer = frameBuffer();

		SMAAArea = texture("assets/textures/SMAA/AreaTexDX10.dds");
		SMAASearch = texture("assets/textures/SMAA/SearchTex.dds");
	}

	~SMAAScene() override = default;

	virtual void Initialize() override
	{
		scene3D::Initialize();

		SMAAArea.LoadTexture();
		SMAASearch.LoadTexture();

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);
		colorDesc.wrapRSetting = GL_CLAMP;
		colorDesc.wrapTSetting = GL_CLAMP;
		colorDesc.wrapSSetting = GL_CLAMP;

		FBODescriptor depthDesc;
		depthDesc.target = GL_TEXTURE_2D;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.wrapRSetting = GL_CLAMP;
		depthDesc.wrapTSetting = GL_CLAMP;
		depthDesc.wrapSSetting = GL_CLAMP;
		depthDesc.internalFormat = gl_depth_component32f;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer.Initialize();
		geometryBuffer.Bind();

		geometryBuffer.AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer.AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		edgesBuffer.Initialize();
		edgesBuffer.Bind();

		FBODescriptor edgeDesc;
		edgeDesc.format = gl_rg;
		edgeDesc.dataType = GL_FLOAT;
		edgeDesc.internalFormat = gl_rg32f;
		edgeDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);
		edgeDesc.wrapRSetting = GL_CLAMP;
		edgeDesc.wrapTSetting = GL_CLAMP;
		edgeDesc.wrapSSetting = GL_CLAMP;
		//edgeDesc.minFilterSetting = GL_NEAREST;
		//edgeDesc.magFilterSetting = GL_NEAREST;

		edgesBuffer.AddAttachment(frameBuffer::attachment_t("edge", edgeDesc));

		weightsBuffer.Initialize();
		weightsBuffer.Bind();

		FBODescriptor weightsDesc;
		weightsDesc = colorDesc;
		weightsDesc.dataType = GL_FLOAT;
		weightsDesc.internalFormat = gl_rgba32f;
		weightsDesc.wrapRSetting = GL_CLAMP;
		weightsDesc.wrapTSetting = GL_CLAMP;
		weightsDesc.wrapSSetting = GL_CLAMP;
		//weightsDesc.minFilterSetting = GL_NEAREST;
		//weightsDesc.magFilterSetting = GL_NEAREST;

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
	}

protected:

	frameBuffer					geometryBuffer;
	frameBuffer					edgesBuffer;
	frameBuffer					weightsBuffer;
	frameBuffer					SMAABuffer;

	texture						SMAAArea;
	texture						SMAASearch;

	bufferHandler_t<SMAASettings_t>		SMAAsettings;

	tShaderProgram* geometryProgram = nullptr;
	tShaderProgram* edgeDetectionProgram = nullptr;
	tShaderProgram* blendingWeightProgram = nullptr;
	tShaderProgram* SMAAProgram = nullptr;
	tShaderProgram* compareProgram = nullptr;
	tShaderProgram* finalProgram = nullptr;

	int currentTexture = 0;
	bool enableCompare = true;

	virtual void Update() override
	{
		manager->PollForEvents();
		if (lockedFrameRate > 0)
		{
			sceneClock.UpdateClockFixed(lockedFrameRate);
		}
		else
		{
			sceneClock.UpdateClockAdaptive();
		}

		defaultPayload.data.deltaTime = (float)sceneClock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)sceneClock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / sceneClock.GetDeltaTime());
		defaultPayload.data.totalFrames++;

		SMAAsettings.Update(gl_uniform_buffer, gl_dynamic_draw);
	}

	void UpdateDefaultBuffer()
	{
		sceneCamera.UpdateProjection();
		defaultPayload.data.projection = sceneCamera.projection;
		defaultPayload.data.view = sceneCamera.view;
		defaultPayload.data.resolution = sceneCamera.resolution;
		if (sceneCamera.currentProjectionType == camera_t::projection_e::perspective)
		{
			defaultPayload.data.translation = testModel.makeTransform();
		}

		else
		{
			defaultPayload.data.translation = sceneCamera.translation;
		}
		defaultPayload.data.deltaTime = (float)sceneClock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)sceneClock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / sceneClock.GetDeltaTime());

		defaultPayload.Update();
		defaultVertexBuffer.UpdateBuffer(defaultPayload.data.resolution);
	}

	virtual void Draw() override
	{
		sceneCamera.ChangeProjection(camera_t::projection_e::perspective);
		sceneCamera.Update();

		UpdateDefaultBuffer();

		GeometryPass(); //render current scene with jitter

		sceneCamera.ChangeProjection(camera_t::projection_e::orthographic);
		UpdateDefaultBuffer();
		
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

	void FinalPass(texture* tex1, texture* tex2) const
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

		/*
		//draw directly to backbuffer
		tex1->SetActive(0);

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glUseProgram(finalProgram->handle);

		glDrawArrays(GL_TRIANGLES, 0, 6);*/
	}

	virtual void BuildGUI(tWindow* window, const ImGuiIO io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawSMAASettings();
	}

	virtual void DrawBufferAttachments()
	{
		ImGui::Begin("framebuffers");
		for (auto val : geometryBuffer.attachments | std::views::values)
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
		ImGui::End();
	}

	virtual void DrawCameraStats() override
	{
		//set up the view matrix
		ImGui::Begin("camera", &isGUIActive);

		ImGui::DragFloat("near plane", &sceneCamera.nearPlane);
		ImGui::DragFloat("far plane", &sceneCamera.farPlane);
		ImGui::SliderFloat("Field of view", &sceneCamera.fieldOfView, 0, 90, "%.0f");

		ImGui::InputFloat("camera speed", &sceneCamera.speed, 0.f);
		ImGui::InputFloat("x sensitivity", &sceneCamera.xSensitivity, 0.f);
		ImGui::InputFloat("y sensitivity", &sceneCamera.ySensitivity, 0.f);
		ImGui::End();
	}

	virtual void ClearBuffers()
	{
		geometryBuffer.Bind();
		frameBuffer::ClearTexture(geometryBuffer.attachments["color"], clearColor);
		glClear(GL_DEPTH_BUFFER_BIT);
		frameBuffer::Unbind();

		SMAABuffer.Bind();
		frameBuffer::ClearTexture(SMAABuffer.attachments["SMAA"], clearColor2);
		frameBuffer::Unbind();

		edgesBuffer.Bind();
		frameBuffer::ClearTexture(edgesBuffer.attachments["edge"], clearColor2);
		frameBuffer::Unbind();

		weightsBuffer.Bind();
		frameBuffer::ClearTexture(weightsBuffer.attachments["blend"], clearColor2);
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

	void HandleWindowResize(const tWindow* window, const TinyWindow::vec2_t<uint16_t> dimensions) override
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
		defaultPayload = bufferHandler_t<defaultUniformBuffer>(sceneCamera);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		defaultPayload.data.resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.projection = sceneCamera.projection;
		defaultPayload.data.translation = sceneCamera.translation;
		defaultPayload.data.view = sceneCamera.view;

		defaultPayload.Initialize(0);
		SMAAsettings.Initialize(1);

		SetupVertexBuffer();
	}

	void DrawSMAASettings()
	{
		ImGui::Begin("SMAA Settings");
		ImGui::Checkbox("enable Compare", &enableCompare);
		ImGui::InputFloat("threshold", &SMAAsettings.data.threshold, 0.001f, 0.1f);
		ImGui::InputFloat("contrast adaption factor", &SMAAsettings.data.contrastAdaptationFactor, 0.001f, 0.1f);
		ImGui::SliderInt("max search steps", &SMAAsettings.data.maxSearchSteps, 0, 255);
		ImGui::SliderInt("max search steps diagonal", &SMAAsettings.data.maxSearchStepsDiag, 0, 255);
		ImGui::SliderInt("corner rounding", &SMAAsettings.data.cornerRounding, 0, 255);

		ImGui::End();
	}
};