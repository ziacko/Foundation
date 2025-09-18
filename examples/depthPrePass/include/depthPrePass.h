#ifndef DEPTH_PRE_PASS_H
#define DEPTH_PRE_PASS_H

#include "scene3D.h"
#include "FrameBuffer.h"

class depthPrePassScene : public scene3D
{
public:

	depthPrePassScene(
		const char* windowName = "Ziyad Barakat's portfolio (early depth test)",
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		model_t model = model_t("models/SoulSpear/SoulSpear.fbx"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(gl_generate_mipmap_hint, GL_NICEST);

		geometryBuffer = new frameBuffer();
	}

	~depthPrePassScene() {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		FBODescriptor depthDesc;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.mipmapLevels = 8;
		depthDesc.internalFormat = gl_depth_component24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("color"));
		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("depth", depthDesc));

		frameBuffer::Unbind();

		programGLID = shaderProgramsMap["geometry"].handle;
		earlyDepthProgram = shaderProgramsMap["earlyDepth"].handle;
		compareProgram = shaderProgramsMap["compare"].handle;
		finalProgram = shaderProgramsMap["final"].handle;
	}

protected:

	frameBuffer* geometryBuffer;

	unsigned int earlyDepthProgram = 0;
	unsigned int finalProgram = 0;
	unsigned int compareProgram = 0;

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

		defaultPayload.data.deltaTime = (float)clock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)clock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / clock.GetDeltaTime());
		defaultPayload.data.totalFrames++;

		defaultVertexBuffer.UpdateBuffer(defaultPayload.data.resolution);
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

		defaultVertexBuffer.UpdateBuffer(defaultPayload.data.resolution);
	}

	void Draw() override
	{
		camera.ChangeProjection(camera_t::projection_e::perspective);
		camera.Update();

		UpdateDefaultBuffer();

		EarlyDepthPass();
		GeometryPass(); //render current scene with jitter

		camera.ChangeProjection(camera_t::projection_e::orthographic);
		UpdateDefaultBuffer();

		FinalPass(geometryBuffer->attachments[0], geometryBuffer->attachments[1]);

		DrawGUI(window);

		manager->SwapDrawBuffers(window);
		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void EarlyDepthPass()
	{
		geometryBuffer->Bind();

		geometryBuffer->attachments[1]->Draw();

		//we just need the first LOd so only do the first 3 meshes
		for(auto iter : testModel.meshes)
		{
			if (iter.isCollision)
			{
				continue;
			}
			if (iter.textures.size() > 0)
			{
				iter.textures[0].SetActive(0);
			}
			//add the previous depth?

			glBindVertexArray(iter.vertexArrayHandle);
			glUseProgram(earlyDepthProgram);
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, iter.indices.size(), GL_UNSIGNED_INT, 0);
			/*glDrawElementsBaseVertex(GL_TRIANGLES,
				iter.numIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * iter.indexOffset),
				iter.vertexOffset);*/
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->attachments[1]->SetActive(0);
		geometryBuffer->attachments[1]->BindTexture();
		glGenerateMipmap(geometryBuffer->attachments[1]->FBODesc.target);
		geometryBuffer->attachments[1]->UnbindTexture();
		geometryBuffer->Unbind();
	}

	virtual void GeometryPass()
	{
		geometryBuffer->Bind();

		//move this to std::array?
		GLenum drawbuffers[1] = {
			geometryBuffer->attachments[0]->FBODesc.attachmentFormat, //color
		};

		glDrawBuffers(1, drawbuffers);

		//we just need the first LOd so only do the first 3 meshes
		for (auto iter : testModel.meshes)
		{
			if (iter.isCollision)
			{
				continue;
			}

			if (iter.textures.size() > 0)
			{
				iter.textures[0].SetActive(0);
			}

			//add the previous depth?
			glBindVertexArray(iter.vertexArrayHandle);
			glUseProgram(programGLID);
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			//glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, iter.indices.size(), GL_UNSIGNED_INT, 0);
			//glDrawElements(GL_TRIANGLES, testModel->meshes[iter].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();
	}

	void FinalPass(texture* tex1, texture* tex2)
	{
		//draw directly to backbuffer		
		tex1->SetActive(0);

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		if (enableCompare)
		{
			tex2->SetActive(1);
			glUseProgram(compareProgram);
		}

		else
		{
			glUseProgram(finalProgram);
		}

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
	}

	virtual void DrawBufferAttachments()
	{
		ImGui::Begin("framebuffers");
		ImGui::Checkbox("enable Compare", &enableCompare);
		for (auto iter : geometryBuffer->attachments)
		{
			ImGui::Image((ImTextureID)iter->GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", iter->GetUniformName().c_str());
		}

		ImGui::End();
	}

	virtual void DrawCameraStats() override
	{
		//set up the view matrix
		ImGui::Begin("camera", &isGUIActive);

		ImGui::DragFloat("near plane", &camera.nearPlane);
		ImGui::DragFloat("far plane", &camera.farPlane);
		ImGui::SliderFloat("Field of view", &camera.fieldOfView, 0, 90, "%.0f");

		ImGui::InputFloat("camera speed", &camera.speed, 0.f);
		ImGui::InputFloat("x sensitivity", &camera.xSensitivity, 0.f);
		ImGui::InputFloat("y sensitivity", &camera.ySensitivity, 0.f);
		ImGui::End();
	}

	virtual void ClearBuffers()
	{
		//ok copy the current frame into the previous frame and clear the rest of the buffers	
		float clearColor1[4] = { 0.25f, 0.25f, 0.25f, 0.25f };

		geometryBuffer->Bind();
		geometryBuffer->ClearTexture(geometryBuffer->attachments[0], clearColor1);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		geometryBuffer->Resize(glm::ivec3(resolution, 1));
	}

	virtual void HandleWindowResize(const tWindow* window, TinyWindow::vec2_t<unsigned int> dimensions) override
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
		auto resolution = window->GetSettings().resolution;
		glViewport(0, 0, resolution.width, resolution.height);

		defaultPayload.data.resolution = glm::ivec2(resolution.width, resolution.height);
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.translation = camera.translation;
		defaultPayload.data.view = camera.view;

		SetupVertexBuffer();
		defaultPayload.Initialize(0);
	}
};

#endif