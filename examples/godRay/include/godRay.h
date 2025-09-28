#pragma once

#include "scene3D.h"
#include "FrameBuffer.h"


struct godRaySettings_t
{
	glm::vec4 lightPosition;
	float decay;
	float density;
	float weight;
	float exposure;
	int samples;	

	godRaySettings_t()
	{
		exposure = 1.0f;
		density = 0.0001f;
		weight = 0.05f;
		decay = 0.75f;
		lightPosition = glm::vec4(0);
		samples = 20;
	}

	~godRaySettings_t() {};

};


class GodRayScene : public scene3D
{
public:

	GodRayScene(
		const char* windowName = "Ziyad Barakat's portfolio (god ray test)",
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), PI * 0.1f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		model_t model = model_t("models/SoulSpear/SoulSpear.fbx"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		geometryBuffer = new frameBuffer();
		occlusionBuffer = new frameBuffer();

		//orthoCamera = new camera(glm::vec2(1280, 720), 5.0f, camera::projection_t::orthographic, 0.1f);
		godRay = bufferHandler_t<godRaySettings_t>();

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

		this->camera.position.y -= 2.0f;
		this->camera.position.z = -1.0f;
	}

	virtual ~GodRayScene() {};

	void Initialize() override
	{
		scene3D::Initialize();

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		FBODescriptor geomDesc;
		geomDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		FBODescriptor depthDesc;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.wrapRSetting = GL_CLAMP;
		depthDesc.wrapSSetting = GL_CLAMP;
		depthDesc.wrapTSetting = GL_CLAMP;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer->AddAttachment(frameBuffer::attachment_t("color", geomDesc));
		geometryBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		occlusionBuffer->Initialize();
		occlusionBuffer->Bind();

		FBODescriptor occlusionDesc;
		occlusionDesc.format = GL_RED;
		occlusionDesc.internalFormat = GL_R8;
		occlusionDesc.dataType = GL_UNSIGNED_BYTE;
		occlusionDesc.wrapRSetting = GL_CLAMP;
		occlusionDesc.wrapSSetting = GL_CLAMP;
		occlusionDesc.wrapTSetting = GL_CLAMP;
		occlusionDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		occlusionBuffer->AddAttachment(frameBuffer::attachment_t("occlusion", occlusionDesc));

		frameBuffer::Unbind();

		defProgram = shaderProgramsMap["geometry"];
		occlusionProgram = shaderProgramsMap["occlusion"];
		GodRayPostProgram = shaderProgramsMap["godRay"];
		finalProgram = shaderProgramsMap["final"];

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glDisable(GL_BLEND);
		//glEnable(GL_DEPTH_TEST);
		//glEnable(gl_clip_distance0);
		//glDepthFunc(GL_LESS);
		//glHint(gl_generate_mipmap_hint, GL_NICEST);;
	}

protected:

	bufferHandler_t<godRaySettings_t> godRay;

	frameBuffer* geometryBuffer;
	frameBuffer* occlusionBuffer;

	//camera* orthoCamera;

	shaderProgram_t GodRayPostProgram;
	shaderProgram_t finalProgram;
	shaderProgram_t occlusionProgram;

	bool enableCompare = true;

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

		defaultPayload.data.deltaTime = static_cast<float>(clock.GetDeltaTime());
		defaultPayload.data.totalTime = static_cast<float>(clock.GetTotalTime());
		defaultPayload.data.framesPerSec = static_cast<float>(1.0 / clock.GetDeltaTime());
		defaultPayload.data.totalFrames++;

		godRay.Update();
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

		//occlusion pass
		OcclusionPass();

		//render scene normally
		GeometryPass();

		//swap to orthographic
		camera.ChangeProjection(camera_t::projection_e::orthographic);
		UpdateDefaultBuffer();	

		//final pass
		GodRayPass();

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
			geometryBuffer->attachments["color"].FBODesc.attachmentFormat, //color
		};

		glDrawBuffers(1, drawbuffers);

		//we just need the first LOd so only do the first 3 meshes
		for (size_t iter = 0; iter < 1; iter++)
		{
			if (testModel.meshes[iter].isCollision)
			{
				continue;
			}

			testModel.meshes[iter].textures[0].SetActive(0);

			glBindVertexArray(testModel.meshes[iter].vertexArrayHandle);
			defProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel.meshes[iter].indices.size(), GL_UNSIGNED_INT, nullptr);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		geometryBuffer->Unbind();
		glPopDebugGroup();
	}

	virtual void OcclusionPass()
	{
		GL_PUSH_DEBUG_GROUP();
		glDisable(GL_BLEND);
		occlusionBuffer->Bind();

		GLenum drawbuffers[1] = {
			occlusionBuffer->attachments["occlusion"].FBODesc.attachmentFormat, //occlusion
		};

		glDrawBuffers(1, drawbuffers);

		//we just need the first LOd so only do the first 3 meshes
		for (size_t iter = 0; iter < 1; iter++)
		{
			if (testModel.meshes[iter].isCollision)
			{
				continue;
			}

			testModel.meshes[iter].textures[0].SetActive(0);

			glBindVertexArray(testModel.meshes[iter].vertexArrayHandle);
			occlusionProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel.meshes[iter].indices.size(), GL_UNSIGNED_INT, nullptr);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		occlusionBuffer->Unbind();
		glPopDebugGroup();
	}

	virtual void GodRayPass()
	{
		GL_PUSH_DEBUG_GROUP();
		
		//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		//just draw to backbuffer. we don't need to compare anything
		geometryBuffer->attachments["color"].SetActive(0);
		occlusionBuffer->attachments["occlusion"].SetActive(1);

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		GodRayPostProgram.Use();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glPopDebugGroup();
	}

	void FinalPass(texture* tex1, texture* tex2)
	{
		GL_PUSH_DEBUG_GROUP();
		//draw directly to backbuffer		
		tex1->SetActive(0);

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		finalProgram.Use();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glPopDebugGroup();
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawGodRaySettings();
	}

	virtual void DrawBufferAttachments()
	{
		if (ImGui::BeginTabItem("framebuffers"))
		{
			ImGui::Checkbox("enable Compare", &enableCompare);
			for (auto iter : geometryBuffer->attachments | std::views::values)
			{
				ImGui::Image((ImTextureID)iter.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter.GetUniformName().c_str());
			}

			ImGui::Image((ImTextureID)occlusionBuffer->attachments["occlusion"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", occlusionBuffer->attachments["occlusion"].GetUniformName().c_str());

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

	virtual void DrawGodRaySettings()
	{
		if (ImGui::BeginTabItem("God rays"))
		{
			ImGui::DragFloat("decay", &godRay.data.decay, 0.001f);
			//ImGui::DragFloat("density", &godRay.data.density, 0.0001f);
			ImGui::DragFloat("weight", &godRay.data.weight, 0.001f);
			ImGui::DragFloat("exposure", &godRay.data.exposure, 0.001f);
			ImGui::InputInt("samples", &godRay.data.samples);

			ImGui::EndTabItem();
		}
	}

	virtual void ClearBuffers()
	{
		//ok copy the current frame into the previous frame and clear the rest of the buffers
		constexpr float clearColor1[4] = { 0.25f, 0.25f, 0.25f, 0.25f };
		constexpr float clearColor2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		geometryBuffer->Bind();
		geometryBuffer->ClearTexture(geometryBuffer->attachments["color"], clearColor1);
		glClear(GL_DEPTH_BUFFER_BIT);
		//frameBuffer::Unbind();

		occlusionBuffer->Bind();
		occlusionBuffer->ClearTexture(occlusionBuffer->attachments["occlusion"], clearColor2);
		frameBuffer::Unbind();
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		geometryBuffer->Resize(glm::ivec3(resolution, 1));
		occlusionBuffer->Resize(glm::ivec3(resolution, 1));
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
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		defaultPayload.data.resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.translation = camera.translation;
		defaultPayload.data.view = camera.view;
		
		defaultVertexBuffer.SetupDefault();

		defaultPayload.Initialize(0);
		godRay.Initialize(1);
	}
};