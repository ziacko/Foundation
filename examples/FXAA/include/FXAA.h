#ifndef FXAA_H
#define FXAA_H

#include "Scene3D.h"

struct FXAASettings_t
{
	GLfloat			pixelShift;
	GLfloat			vxOffset;
	GLfloat			maxSpan;
	GLfloat			reduceMul;
	GLfloat			reduceMin;

	FXAASettings_t(
		GLfloat pixelShift = 0.25f, GLfloat vxOffset = 0.0f, GLfloat maxSpan = 8.0f, 
		GLfloat reduceMul = 0.125f, GLfloat reduceMin = 0.0078125f)
	{
		this->pixelShift = pixelShift;
		this->vxOffset = vxOffset;
		this->maxSpan = maxSpan;
		this->reduceMul = reduceMul;
		this->reduceMin = reduceMin;
	}

	~FXAASettings_t() { };
};

class FXAA_Scene : public scene3D
{
public:

	FXAA_Scene(
		const char* windowName = "Ziyad Barakat's portfolio (FXAA)",
		camera_t* texModelCamera = new camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		model_t* model = new model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(gl_generate_mipmap_hint, GL_NICEST);

		geometryBuffer = new frameBuffer();
		FXAABuffer = new frameBuffer();

		FXAA = bufferHandler_t<FXAASettings_t>();
	}

	~FXAA_Scene() {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		FBODescriptor depthDesc;
		depthDesc.target = GL_TEXTURE_2D;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = gl_depth_component24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetWindowSettings().resolution.width, window->GetWindowSettings().resolution.height, 1);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("color"));
		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("depth", depthDesc));

		FXAABuffer->Initialize();
		FXAABuffer->Bind();
		FXAABuffer->AddAttachment(new frameBuffer::attachment_t("FXAA"));

		frameBuffer::Unbind();

		programGLID = shaderProgramsMap["geometryProgram"].handle;
		FXAAProgram = shaderProgramsMap["FXAAProgram"].handle;
		compareProgram = shaderProgramsMap["compareProgram"].handle;
		finalProgram = shaderProgramsMap["finalProgram"].handle;
	}

protected:

	frameBuffer* geometryBuffer;
	frameBuffer* FXAABuffer;

	unsigned int FXAAProgram = 0;
	unsigned int compareProgram = 0;
	unsigned int finalProgram = 0;

	bool enableCompare = true;

	bufferHandler_t<FXAASettings_t>		FXAA;

	virtual void Update() override
	{
		manager->PollForEvents();
		if (lockedFrameRate > 0)
		{
			sceneClock->UpdateClockFixed(lockedFrameRate);
		}
		else
		{
			sceneClock->UpdateClockAdaptive();
		}

		defaultPayload.data.deltaTime = (float)sceneClock->GetDeltaTime();
		defaultPayload.data.totalTime = (float)sceneClock->GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / sceneClock->GetDeltaTime());
		defaultPayload.data.totalFrames++;

		FXAA.Update();
		defaultVertexBuffer->UpdateBuffer(defaultPayload.data.resolution);
	}

	void UpdateDefaultBuffer()
	{
		sceneCamera->UpdateProjection();
		defaultPayload.data.projection = sceneCamera->projection;
		defaultPayload.data.view = sceneCamera->view;
		if (sceneCamera->currentProjectionType == camera_t::projection_e::perspective)
		{
			defaultPayload.data.translation = testModel->makeTransform();
		}

		else
		{
			defaultPayload.data.translation = sceneCamera->translation;
		}
		defaultPayload.data.deltaTime = (float)sceneClock->GetDeltaTime();
		defaultPayload.data.totalTime = (float)sceneClock->GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / sceneClock->GetDeltaTime());

		defaultPayload.Update();
		defaultVertexBuffer->UpdateBuffer(defaultPayload.data.resolution);
	}

	void Draw() override
	{
		sceneCamera->ChangeProjection(camera_t::projection_e::perspective);
		sceneCamera->Update();

		UpdateDefaultBuffer();

		GeometryPass(); //render current scene with jitter

		sceneCamera->ChangeProjection(camera_t::projection_e::orthographic);
		UpdateDefaultBuffer();
		
		FXAAPass(); //use the positions, colors, depth and velocity to smooth the final image

		FinalPass(FXAABuffer->attachments[0], geometryBuffer->attachments[0]);
		
		DrawGUI(window);

		manager->SwapDrawBuffers(window);

		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void GeometryPass()
	{
		geometryBuffer->Bind();

		GLenum drawbuffers[1] = {
			geometryBuffer->attachments[0]->FBODesc.attachmentFormat, //color
		};

		glDrawBuffers(1, drawbuffers);

		//we just need the first LOd so only do the first 3 meshes
		for (size_t iter = 0; iter < 1; iter++)
		{
			if (testModel->meshes[iter].isCollision)
			{
				continue;
			}

			testModel->meshes[iter].textures[0].SetActive(0);
			//add the previous depth?

			glBindVertexArray(testModel->meshes[iter].vertexArrayHandle);
			glUseProgram(programGLID);
			glViewport(0, 0, window->GetWindowSettings().resolution.width, window->GetWindowSettings().resolution.height);
			
			//glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel->meshes[iter].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();
	}

	virtual void FXAAPass()
	{
		FXAABuffer->Bind();
		GLenum drawBuffers[1] = {
			FXAABuffer->attachments[0]->FBODesc.attachmentFormat
		};
		glDrawBuffers(1, drawBuffers);

		//current frame
		geometryBuffer->attachments[0]->SetActive(0); // color
		
		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glUseProgram(FXAAProgram);
		glViewport(0, 0, window->GetWindowSettings().resolution.width, window->GetWindowSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		FXAABuffer->Unbind();
	}

	void FinalPass(texture* tex1, texture* tex2)
	{
		//draw directly to backbuffer		
		tex1->SetActive(0);
		
		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glViewport(0, 0, window->GetWindowSettings().resolution.width, window->GetWindowSettings().resolution.height);
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

	virtual void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawFXAASettings();
	}

	void DrawFXAASettings()
	{
		ImGui::Begin("FXAA Settings");
		ImGui::Checkbox("enable Compare", &enableCompare);
		ImGui::SliderFloat("Sub pixel drift", &FXAA.data.pixelShift, 0.0f, 1.0f, "%.1f");
		ImGui::SliderFloat("vertex Offset", &FXAA.data.vxOffset, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("max span", &FXAA.data.maxSpan, 0.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("reduce multiplier", &FXAA.data.reduceMul, 0.0f, 1.0f, "%.5f");
		ImGui::SliderFloat("reduce minimizer", &FXAA.data.reduceMin, 0.0f, 1.0f, "%.8f");

		ImGui::End();
	}

	virtual void DrawBufferAttachments()
	{
		ImGui::Begin("framebuffers");
		for (auto iter : geometryBuffer->attachments)
		{
			ImGui::Image((ImTextureID)iter->GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", iter->GetUniformName().c_str());
		}

		for (auto iter : FXAABuffer->attachments)
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

		ImGui::DragFloat("near plane", &sceneCamera->nearPlane);
		ImGui::DragFloat("far plane", &sceneCamera->farPlane);
		ImGui::SliderFloat("Field of view", &sceneCamera->fieldOfView, 0, 90, "%.0f");

		ImGui::InputFloat("camera speed", &sceneCamera->speed, 0.f);
		ImGui::InputFloat("x sensitivity", &sceneCamera->xSensitivity, 0.f);
		ImGui::InputFloat("y sensitivity", &sceneCamera->ySensitivity, 0.f);
		ImGui::End();
	}

	virtual void ClearBuffers()
	{
		//ok copy the current frame into the previous frame and clear the rest of the buffers	
		float clearColor1[4] = { 0.025f, 0.025f, 0.025f, 0.025f };

		geometryBuffer->Bind();
		geometryBuffer->ClearTexture(geometryBuffer->attachments[0], clearColor1);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();

		FXAABuffer->Bind();
		FXAABuffer->ClearTexture(FXAABuffer->attachments[0], clearColor1);
		FXAABuffer->Unbind();

		sceneCamera->ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		for (auto iter : geometryBuffer->attachments)
		{
			iter->Resize(glm::ivec3(resolution, 1));
		}

		FXAABuffer->attachments[0]->Resize(glm::ivec3(resolution, 1));
	}

	virtual void HandleWindowResize(const tWindow* window, TinyWindow::vec2_t<unsigned int> dimensions) override
	{
		defaultPayload.data.resolution = glm::ivec2(dimensions.width, dimensions.height);	
		ResizeBuffers(glm::ivec2(dimensions.x, dimensions.y));
	}

	virtual void HandleMaximize(const tWindow* window) override
	{
		defaultPayload.data.resolution = glm::ivec2(window->GetWindowSettings().resolution.width, window->GetWindowSettings().resolution.height);
		ResizeBuffers(defaultPayload.data.resolution);
	}

	virtual void InitializeUniforms() override
	{
		defaultPayload = bufferHandler_t<defaultUniformBuffer>(sceneCamera);
		glViewport(0, 0, window->GetWindowSettings().resolution.width, window->GetWindowSettings().resolution.height);

		defaultPayload.data.resolution = glm::ivec2(window->GetWindowSettings().resolution.width, window->GetWindowSettings().resolution.height);
		defaultPayload.data.projection = sceneCamera->projection;
		defaultPayload.data.translation = sceneCamera->translation;
		defaultPayload.data.view = sceneCamera->view;

		defaultPayload.Initialize(0);
		FXAA.Initialize(5);

		SetupVertexBuffer();
	}
};
#endif