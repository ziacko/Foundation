#ifndef FXAA_H
#define FXAA_H

#include "scene3D.h"

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
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		model_t model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

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
		depthDesc.internalFormat = GL_DEPTH_COMPONENT24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(frameBuffer::attachment_t("color"));
		geometryBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		FXAABuffer->Initialize();
		FXAABuffer->Bind();
		FXAABuffer->AddAttachment(frameBuffer::attachment_t("FXAA"));

		frameBuffer::Unbind();

		defProgram = shaderProgramsMap["geometryProgram"];
		FXAAProgram = shaderProgramsMap["FXAAProgram"].handle;
		compareProgram = shaderProgramsMap["compareProgram"].handle;
		finalProgram = shaderProgramsMap["finalProgram"].handle;
		auto testShader = shaderProgramsMap["finalProgram"];
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

		FXAA.Update();
		//defaultVertexBuffer.UpdateBuffer(defaultPayload.data.resolution);
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

		GeometryPass(); //render current scene with jitter

		camera.ChangeProjection(camera_t::projection_e::orthographic);
		camera.Update();
		UpdateDefaultBuffer();
		
		FXAAPass(); //use the positions, colors, depth and velocity to smooth the final image

		FinalPass(&FXAABuffer->attachments["FXAA"], &geometryBuffer->attachments["color"]);
		
		DrawGUI(window);

		manager->SwapDrawBuffers(window);

		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void GeometryPass()
	{
		geometryBuffer->Bind();

		GLenum drawbuffers[1] = {
			geometryBuffer->attachments["color"].FBODesc.attachmentFormat, //color
		};

		glDrawBuffers(1, drawbuffers);

		//we just need the first LOd so only do the first 3 meshes
		for (size_t iter = 0; iter < testModel.meshes.size(); iter++)
		{
			if (testModel.meshes[iter].isCollision)
			{
				continue;
			}

			for (uint8_t textureIter = 0; textureIter < testModel.meshes[iter].textures.size(); textureIter++)
			{
				testModel.meshes[iter].textures[textureIter].SetActive(textureIter);
			}

			//add the previous depth?

			glBindVertexArray(testModel.meshes[iter].vertexArrayHandle);
			glUseProgram(defProgram.handle);
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel.meshes[iter].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();
	}

	virtual void FXAAPass()
	{
		FXAABuffer->Bind();
		GLenum drawBuffers[1] = {
			FXAABuffer->attachments["FXAA"].FBODesc.attachmentFormat
		};
		glDrawBuffers(1, drawBuffers);

		//current frame
		geometryBuffer->attachments["color"].SetActive(0); // color
		
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glUseProgram(FXAAProgram);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		FXAABuffer->Unbind();
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

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawFXAASettings();
	}

	void DrawFXAASettings()
	{
		if (ImGui::BeginTabItem("FXAA Settings"))
		{
			ImGui::Checkbox("enable Compare", &enableCompare);
			ImGui::SliderFloat("Sub pixel drift", &FXAA.data.pixelShift, 0.0f, 1.0f, "%.1f");
			ImGui::SliderFloat("vertex Offset", &FXAA.data.vxOffset, 0.0f, 1.0f, "%.3f");
			ImGui::SliderFloat("max span", &FXAA.data.maxSpan, 0.0f, 10.0f, "%.1f");
			ImGui::SliderFloat("reduce multiplier", &FXAA.data.reduceMul, 0.0f, 1.0f, "%.5f");
			ImGui::SliderFloat("reduce minimizer", &FXAA.data.reduceMin, 0.0f, 1.0f, "%.8f");

			ImGui::EndTabItem();
		}
	}

	virtual void DrawBufferAttachments()
	{
		if (ImGui::BeginTabItem("framebuffers"))
		{
			for (auto iter : geometryBuffer->attachments)
			{
				ImGui::Image((ImTextureID)iter.second.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter.second.GetUniformName().c_str());
			}

			for (auto iter : FXAABuffer->attachments)
			{
				ImGui::Image((ImTextureID)iter.second.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter.second.GetUniformName().c_str());
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
		//ok copy the current frame into the previous frame and clear the rest of the buffers	
		float clearColor1[4] = { 0.25f, 0.25f, 0.25f, 1.0f };

		geometryBuffer->Bind();
		geometryBuffer->ClearTexture(geometryBuffer->attachments["color"], clearColor1);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();

		FXAABuffer->Bind();
		FXAABuffer->ClearTexture(FXAABuffer->attachments["FXAA"], clearColor1);
		FXAABuffer->Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		for (auto iter : geometryBuffer->attachments)
		{
			iter.second.Resize(glm::ivec3(resolution, 1));
		}

		FXAABuffer->attachments["FXAA"].Resize(glm::ivec3(resolution, 1));
	}

	virtual void HandleWindowResize(const tWindow* window, const TinyWindow::vec2_t<uint16_t>& dimensions) override
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
		defaultPayload = bufferHandler_t<defaultUniformBuffer>(camera);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		defaultPayload.data.resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.translation = camera.translation;
		defaultPayload.data.view = camera.view;

		defaultPayload.Initialize(0);
		FXAA.Initialize(5);

		defaultVertexBuffer.SetupDefault();
	}
};
#endif