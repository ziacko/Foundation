#ifndef DYNAMIC_RES_H
#define DYNAMIC_RES_H

#include "scene3D.h"
#include "FrameBuffer.h"

constexpr glm::vec2 defaultResScale = glm::vec2(1, 1);

struct resolutionSettings_t
{
	glm::vec2 resolutionScale{defaultResScale};

	explicit resolutionSettings_t(const glm::vec2& res = defaultResScale)
	{
		resolutionScale = res;
	}
};

class dynamicRes : public scene3D
{
public:

	explicit dynamicRes(
		const char* windowName = "Ziyad Barakat's portfolio (dynamic resolution)",
		const camera_t& texModelCamera = camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		const model_t& model = model_t("models/SoulSpear/SoulSpear.fbx"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

		geometryBuffer = new frameBuffer();
		dynamicBuffer = new frameBuffer();

		resolution = bufferHandler_t<resolutionSettings_t>();
	}

	~dynamicRes() override {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		FBODescriptor depthDesc;
		depthDesc.target = GL_TEXTURE_2D;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(frameBuffer::attachment_t("color"));
		geometryBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		dynamicBuffer->Initialize();
		dynamicBuffer->Bind();
		dynamicBuffer->AddAttachment(frameBuffer::attachment_t("color"));
		dynamicBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		frameBuffer::Unbind();

		defProgram = shaderProgramsMap["geometry"];
		compareProgram = shaderProgramsMap["compare"];
		finalProgram = shaderProgramsMap["final"];

		defaultVertexBuffer.SetupDefault();
	}

protected:

	frameBuffer* geometryBuffer;
	frameBuffer* dynamicBuffer;

	shaderProgram_t compareProgram;
	shaderProgram_t finalProgram;

	glm::ivec2 scaledResolution;

	bool enableCompare = true;

	bufferHandler_t<resolutionSettings_t>		resolution;

	virtual void Update() override
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

		resolution.Update();
	}

	void UpdateDefaultBuffer()
	{
		camera.UpdateProjection();
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.view = camera.view;
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

		GeometryPass();
		DynamicPass();

		camera.ChangeProjection(camera_t::projection_e::orthographic);
		camera.Update();
		UpdateDefaultBuffer();

		FinalPass(&geometryBuffer->attachments["color"], &dynamicBuffer->attachments["color"]);
		
		DrawGUI(window);
		
		manager->SwapDrawBuffers(window);
		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void GeometryPass()
	{
		GL_PUSH_DEBUG_GROUP();
		geometryBuffer->Bind();

		GLenum drawbuffers[1] = {
			geometryBuffer->attachments["color"].FBODesc.attachmentFormat, // color
		};

		glDrawBuffers(1, drawbuffers);

		for (size_t i = 0; i < testModel.meshes.size(); ++i)
		{
			if (testModel.meshes[i].isCollision)
				continue;

			for (uint8_t tex = 0; tex < testModel.meshes[i].textures.size(); ++tex)
			{
				testModel.meshes[i].textures[tex].SetActive(tex);
			}

			glBindVertexArray(testModel.meshes[i].vertexArrayHandle);
			defProgram.Use();
			
			auto res = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
			glViewport(0, 0, (GLint)res.x, (GLint)res.y);
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();
		glPopDebugGroup();
	}

	virtual void DynamicPass()
	{
		GL_PUSH_DEBUG_GROUP();
		dynamicBuffer->Bind();

		GLenum drawbuffers[1] = {
			dynamicBuffer->attachments["color"].FBODesc.attachmentFormat, // color
		};

		glDrawBuffers(1, drawbuffers);

		for (size_t i = 0; i < testModel.meshes.size(); ++i)
		{
			if (testModel.meshes[i].isCollision)
				continue;

			for (uint8_t tex = 0; tex < testModel.meshes[i].textures.size(); ++tex)
			{
				testModel.meshes[i].textures[tex].SetActive(tex);
			}

			glBindVertexArray(testModel.meshes[i].vertexArrayHandle);
			defProgram.Use();

			auto res = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height) * this->resolution.data.resolutionScale;
			glViewport(0, 0, (GLint)res.x, (GLint)res.y);
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		dynamicBuffer->Unbind();
		glPopDebugGroup();
	}

	void FinalPass(texture* tex1, texture* tex2)
	{
		GL_PUSH_DEBUG_GROUP();
		//draw directly to backbuffer
		tex1->SetActive(0);
		tex2->SetActive(1);
		
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		if (enableCompare == true)
		{
			compareProgram.Use();
		}
		else
		{
			finalProgram.Use();
		}
		
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glPopDebugGroup();
	}

	void InitializeUniforms() override
	{
		scene3D::InitializeUniforms();
		defaultVertexBuffer.SetupDefault();
		resolution.Initialize(1);
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawResolutionSettings();
	}

	virtual void DrawResolutionSettings()
	{
		if (ImGui::BeginTabItem("resolution Settings"))
		{
			if (ImGui::SliderFloat("horizontal %", &resolution.data.resolutionScale.x, 0.25f, 2.0f, "%.3f") ||
				ImGui::SliderFloat("vertical %", &resolution.data.resolutionScale.y, 0.25f, 2.0f, "%.3f"))
			{
				//camera.resolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height) * resolution.data.resolutionScale;
				ResizeBuffers(glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height) * resolution.data.resolutionScale);
			}
			ImGui::EndTabItem();
		}
	}

	virtual void DrawBufferAttachments()
	{
		if (ImGui::BeginTabItem("framebuffers"))
		{
			for (auto& iter : geometryBuffer->attachments | std::views::values)
			{
				ImGui::Image((ImTextureID)iter.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter.GetUniformName().c_str());
			}

			for (auto& iter : dynamicBuffer->attachments | std::views::values)
			{
				ImGui::Image((ImTextureID)iter.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter.GetUniformName().c_str());
			}

			ImGui::EndTabItem();
		}
	}

	virtual void DrawCameraStats() override
	{
		//set up the view matrix
		if (ImGui::BeginTabItem("camera", &isGUIActive))
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
		// Clear the FBO color and depth
		float clearColor1[4] = { 0.25f, 0.25f, 0.25f, 1.0f };

		geometryBuffer->Bind();
		frameBuffer::ClearTexture(geometryBuffer->attachments["color"], clearColor1);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();

		dynamicBuffer->Bind();
		frameBuffer::ClearTexture(dynamicBuffer->attachments["color"], clearColor1);
		glClear(GL_DEPTH_BUFFER_BIT);
		dynamicBuffer->Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(glm::ivec2 newSize)
	{
		auto res = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height) * this->resolution.data.resolutionScale;
		geometryBuffer->Resize(glm::ivec3((int)window->GetSettings().resolution.width, (int)window->GetSettings().resolution.height, 1));
		dynamicBuffer->Resize(glm::ivec3((int)res.x, (int)res.y, 1));
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
};
#endif