#ifndef TEMPORALAA_H
#define TEMPORALAA_H

#include "scene3D.h"
#include "FrameBuffer.h"

class MSAA : public scene3D
{
public:

	MSAA(
		const char* windowName = "Ziyad Barakat's portfolio (MSAA)",
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		model_t model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);


		geometryBuffer = frameBuffer();
	}

	~MSAA() {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		geometryBuffer.Initialize();
		geometryBuffer.Bind();

		FBODescriptor colorDesc;
		colorDesc.target = GL_TEXTURE_2D_MULTISAMPLE;
		colorDesc.dataType = GL_FLOAT;
		colorDesc.sampleCount = 8;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		FBODescriptor depthDesc;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.target = GL_TEXTURE_2D_MULTISAMPLE;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT32F;
		depthDesc.sampleCount = 8;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer.AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer.AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		frameBuffer::Unbind();

		defProgram = shaderProgramsMap["geometryProgram"];
		finalProgram = shaderProgramsMap["MSAA"];
	}

protected:

	frameBuffer geometryBuffer;
	shaderProgram_t finalProgram;

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

		float dt = (float)clock.GetDeltaTime();
		defaultPayload.data.deltaTime = dt;
		defaultPayload.data.totalTime = (float)clock.GetTotalTime();
		defaultPayload.data.framesPerSec = (dt > 0.0f) ? (1.0f / dt) : 0.0f;
		defaultPayload.data.totalFrames++;
		defaultPayload.Update();
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
		float dt2 = (float)clock.GetDeltaTime();
		defaultPayload.data.deltaTime = dt2;
		defaultPayload.data.totalTime = (float)clock.GetTotalTime();
		defaultPayload.data.framesPerSec = (dt2 > 0.0f) ? (1.0f / dt2) : 0.0f;

		defaultPayload.Update();
		//defaultVertexBuffer.UpdateBuffer(defaultPayload.data.resolution);
	}

	virtual void Draw()
	{
		camera.ChangeProjection(camera_t::projection_e::perspective);
		camera.Update();

		UpdateDefaultBuffer();

		GeometryPass();

		camera.ChangeProjection(camera_t::projection_e::orthographic);
		UpdateDefaultBuffer();

		auto finalTexture = geometryBuffer.attachments["color"];

		FinalPass(&finalTexture);
		
		DrawGUI(window);

		manager->SwapDrawBuffers(window);
		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void GeometryPass()
	{
		glEnable(GL_SAMPLE_SHADING);
		glMinSampleShading(1.0f);
		geometryBuffer.Bind();

		GLenum drawbuffers[1] = {
			geometryBuffer.attachments["color"].FBODesc.attachmentFormat, //color
		};

		glDrawBuffers(1, drawbuffers);

		for (size_t iter = 0; iter < 1; iter++)
		{
			if (testModel.meshes[iter].isCollision)
			{
				continue;
			}

			testModel.meshes[iter].textures[0].SetActive(0);

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

		glDisable(GL_SAMPLE_SHADING);
		geometryBuffer.Unbind();
	}

	void FinalPass(texture* tex1)
	{
		//draw directly to backbuffer
		tex1->SetActive(0);
		
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glUseProgram(finalProgram.handle);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
	}

	virtual void DrawBufferAttachments()
	{
		/*ImGui::Begin("framebuffers");
		for (auto iter : geometryBuffer.attachments)
		{
			ImGui::Image((ImTextureID*)iter.second.GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", iter.second.GetUniformName().c_str());
		}

		ImGui::End();*/
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
		float clearColor1[4] = { 0.5f, 0.5f, 0.5f, 0.5f };

		geometryBuffer.Bind();
		geometryBuffer.ClearTexture(geometryBuffer.attachments["color"], clearColor1);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer.Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		geometryBuffer.Resize(glm::ivec3(resolution, 1));
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
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		defaultPayload.data.resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.translation = camera.translation;
		defaultPayload.data.view = camera.view;

		defaultPayload.Initialize(0);
		defaultVertexBuffer.SetupDefault();
	}
};
#endif