#ifndef SSAA_H
#define SSAA_H

#include "scene3D.h"
#include "FrameBuffer.h"
#include "HaltonSequence.h"

using downsampleType_t = enum { none = 0, lanczos };

struct jitterSettings_t
{
	glm::vec2			haltonSequence[128];
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

struct downscaleSettings_t
{
	float texelWidthOffset;
	float texelHeightOffset;
	int downsampleMode;

	downscaleSettings_t()
	{
		texelWidthOffset = 0.00025f;
		texelHeightOffset = 0.00025f;
		downsampleMode = none;
	}
};

struct lanzcosSettings_t
{
	float magicValue1;
	float magicValue2;
	float magicValue3;
	float magicValue4;
	float magicValue5;

	lanzcosSettings_t()
	{
		magicValue1 = 0.38026f;
		magicValue2 = 0.27667f;
		magicValue3 = 0.08074f;
		magicValue4 = -0.02612f;
		magicValue5 = -0.02143f;
	}
};

class SSAA : public scene3D
{
public:

	SSAA(
		const char* windowName = "Ziyad Barakat's portfolio (SSAA)",
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		model_t model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

		downscaledBuffer = new frameBuffer();
		geometryBuffer = new frameBuffer();
		unJitteredBuffer = new frameBuffer();

		jitterUniforms = bufferHandler_t<jitterSettings_t>();
		downscaleUniforms = bufferHandler_t<downscaleSettings_t>();
		lanzcosUniforms = bufferHandler_t<lanzcosSettings_t>();

		for (int iter = 0; iter < 128; iter++)
		{
			jitterUniforms.data.haltonSequence[iter] = glm::vec2(CreateHaltonSequence(iter + 1, 2), CreateHaltonSequence(iter + 1, 3));
		}

		this->camera.position.y -= 100.0f;
	}

	~SSAA() {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		FBODescriptor ssDesc;
		ssDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width * 2, window->GetSettings().resolution.height * 2, 1);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();
		geometryBuffer->AddAttachment(frameBuffer::attachment_t("color", ssDesc));

		FBODescriptor ssDepthDesc;
		ssDepthDesc.dataType = GL_FLOAT;
		ssDepthDesc.format = GL_DEPTH_COMPONENT;
		ssDepthDesc.internalFormat = GL_DEPTH_COMPONENT24;
		ssDepthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		ssDepthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width * 2, window->GetSettings().resolution.height * 2, 1);

		geometryBuffer->AddAttachment(frameBuffer::attachment_t("depth", ssDepthDesc));

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		downscaledBuffer->Initialize();
		downscaledBuffer->Bind();
		downscaledBuffer->AddAttachment(frameBuffer::attachment_t("downscaled", colorDesc));

		//change back to regular dimensions here
		ssDepthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		unJitteredBuffer->Initialize();
		unJitteredBuffer->Bind();
		unJitteredBuffer->AddAttachment(frameBuffer::attachment_t("unJittered", colorDesc));
		unJitteredBuffer->AddAttachment(frameBuffer::attachment_t("depth", ssDepthDesc));

		//geometry automatically gets assigned to 0
		defProgram = shaderProgramsMap["geometryProgram"];
		downscaleProgram = shaderProgramsMap["downscaleProgram"];
		unjitteredProgram = shaderProgramsMap["treeProgram"];
		compareProgram = shaderProgramsMap["compareProgram"];
		finalProgram = shaderProgramsMap["finalProgram"];

		frameBuffer::Unbind();
	}

protected:

	frameBuffer* geometryBuffer;
	frameBuffer* downscaledBuffer;
	frameBuffer* unJitteredBuffer;

	shaderProgram_t downscaleProgram;
	shaderProgram_t unjitteredProgram;
	shaderProgram_t compareProgram;
	shaderProgram_t finalProgram;

	bool enableCompare = true;
	bufferHandler_t<jitterSettings_t>		jitterUniforms;
	bufferHandler_t<downscaleSettings_t>	downscaleUniforms;
	bufferHandler_t<lanzcosSettings_t>		lanzcosUniforms;

	std::vector<const char*>		downsampleSettings = { "none", "lanczos" };

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

		downscaleUniforms.Update();
		jitterUniforms.Update();
		lanzcosUniforms.Update();
	}

	void UpdateDefaultBuffer(glm::vec2 resolution)
	{
		camera.UpdateProjection();
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.view = camera.view;
		defaultPayload.data.resolution = resolution;
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

	virtual void Draw() override
	{
		camera.ChangeProjection(camera_t::projection_e::perspective);
		glm::vec2 defaultResolution = camera.resolution;
		camera.resolution = glm::vec2(camera.resolution.x * 2, camera.resolution.y * 2);
		camera.Update();

		UpdateDefaultBuffer(camera.resolution);

		UpsamplePass(); //render current scene with jitter
		camera.resolution = defaultResolution;
		if (enableCompare)
		{
			UnJitteredPass();
		}

		camera.ChangeProjection(camera_t::projection_e::orthographic);
		UpdateDefaultBuffer(defaultResolution);
		
		DownscalePass(); //use the positions, colors, depth and velocity to smooth the final image

		FinalPass(&downscaledBuffer->attachments["downscaled"], &unJitteredBuffer->attachments["unJittered"]);

		DrawGUI(window);

		manager->SwapDrawBuffers(window);
		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//camera.resolution = defaultResolution;
	}

	virtual void UpsamplePass()
	{
		GL_PUSH_DEBUG_GROUP();
		geometryBuffer->Bind();
		geometryBuffer->attachments["color"].Draw();

		//we just need the first LOD so only do the first 3 meshes
		for (size_t iter = 0; iter < testModel.meshes.size(); iter++)
		{
			if (testModel.meshes[iter].isCollision)
			{
				continue;
			}

			testModel.meshes[iter].textures[0].SetActive(0);

			glBindVertexArray(testModel.meshes[iter].vertexArrayHandle);
			defProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width * 2, window->GetSettings().resolution.height * 2);
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, (GLsizei)testModel.meshes[iter].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();

		glPopDebugGroup();
	}

	virtual void UnJitteredPass()
	{
		GL_PUSH_DEBUG_GROUP();
		unJitteredBuffer->Bind();
		unJitteredBuffer->attachments["unJittered"].Draw();

		//we just need the first LOd so only do the first 3 meshes
		for (size_t iter = 0; iter < testModel.meshes.size(); iter++)
		{
			if (testModel.meshes[iter].isCollision)
			{
				continue;
			}

			testModel.meshes[iter].textures[0].SetActive(0);
			//add the previous depth?

			glBindVertexArray(testModel.meshes[iter].vertexArrayHandle);
			unjitteredProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, (GLsizei)testModel.meshes[iter].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		unJitteredBuffer->Unbind();
		glPopDebugGroup();
	}

	virtual void DownscalePass()
	{
		GL_PUSH_DEBUG_GROUP();
		downscaledBuffer->Bind();
		downscaledBuffer->attachments["downscaled"].Draw();

		//current frame
		geometryBuffer->attachments["color"].SetActive(0); //current color
		
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		downscaleProgram.Use();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		downscaledBuffer->Unbind();
		glPopDebugGroup();
	}

	void FinalPass(texture* tex1, texture* tex2) const
	{
		GL_PUSH_DEBUG_GROUP();
		//draw directly to backbuffer		
		tex1->SetActive(0);
		
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
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

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawTAASettings();
		DrawDownsampleSettings();
	}

	virtual void DrawTAASettings()
	{
		if (ImGui::BeginTabItem("TAA Settings"))
		{
			ImGui::Text("performance | %f", defaultTimer->GetTimeMilliseconds());
			ImGui::Checkbox("enable Compare", &enableCompare);

			//jitter settings
			ImGui::Separator();
			ImGui::DragFloat("halton scale", &jitterUniforms.data.haltonScale, 0.1f, 0.0f, 1000.0f, "%.3f");
			ImGui::DragInt("halton index",  &jitterUniforms.data.haltonIndex, 1.0f, 0, 128);
			ImGui::DragInt("enable dithering", &jitterUniforms.data.enableDithering, 1.0f, 0, 1);
			ImGui::DragFloat("dithering scale", &jitterUniforms.data.ditheringScale, 1.0f, 0.0f, 1000.0f, "%.3f");

			ImGui::EndTabItem();
		}
	}

	virtual void DrawDownsampleSettings()
	{
		if (ImGui::BeginTabItem("Lanczos settings"))
		{
			ImGui::ListBox("downsample types", &downscaleUniforms.data.downsampleMode, downsampleSettings.data(), downsampleSettings.size());
			ImGui::DragFloat("texel width", &downscaleUniforms.data.texelWidthOffset, 0.1f, 0.0f, 10.0f, "%.5f");
			ImGui::DragFloat("texel height", &downscaleUniforms.data.texelHeightOffset, 0.1f, 0.0f, 10.0f, "%.5f");
			switch(downscaleUniforms.data.downsampleMode)
			{
			case none:
				{
					break;
				}

			case lanczos:
				{

					ImGui::DragFloat("value 1", &lanzcosUniforms.data.magicValue1, 0.01f, 0.0f, 1.0f, "%.3f");
					ImGui::DragFloat("value 2", &lanzcosUniforms.data.magicValue2, 0.01f, 0.0f, 1.0f, "%.3f");
					ImGui::DragFloat("value 3", &lanzcosUniforms.data.magicValue3, 0.01f, 0.0f, 1.0f, "%.3f");
					ImGui::DragFloat("value 4", &lanzcosUniforms.data.magicValue4, 0.01f, 0.0f, 1.0f, "%.3f");
					ImGui::DragFloat("value 5", &lanzcosUniforms.data.magicValue5, 0.01f, 0.0f, 1.0f, "%.3f");

					break;
				}

			default:
				{
					break;
				}

			}

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

			ImGui::Image((ImTextureID*)downscaledBuffer->attachments["downscaled"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", downscaledBuffer->attachments["downscaled"].GetUniformName().c_str());

			ImGui::Image((ImTextureID*)unJitteredBuffer->attachments["unJittered"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", unJitteredBuffer->attachments["unJittered"].GetUniformName().c_str());

			ImGui::EndTabItem();
		}
	}

	virtual void DrawCameraStats() override
	{
		//set up the view matrix
		if (ImGui::BeginTabItem("camera"))
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
		float clearColor1[4] = { 0.3f, 0.3f, 0.3f, 1.0f };

		downscaledBuffer->Bind(); //clear the previous, the next frame current becomes previous
		downscaledBuffer->ClearTexture(downscaledBuffer->attachments["downscaled"], clearColor1);
		downscaledBuffer->Unbind();

		geometryBuffer->Bind();
		geometryBuffer->ClearTexture(geometryBuffer->attachments["color"], clearColor1);
		geometryBuffer->ClearTexture(geometryBuffer->attachments["depth"], clearColor1);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();

		unJitteredBuffer->Bind();
		unJitteredBuffer->ClearTexture(unJitteredBuffer->attachments["color"], clearColor1);
		glClear(GL_DEPTH_BUFFER_BIT);
		unJitteredBuffer->Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		for (auto iter : geometryBuffer->attachments | std::views::values)
		{
			iter.Resize(glm::ivec3(resolution.x * 2, resolution.y * 2, 1));
		}

		downscaledBuffer->attachments["downscaled"].Resize(glm::ivec3(resolution.x, resolution.y, 1));
		unJitteredBuffer->attachments["unJittered"].Resize(glm::ivec3(resolution.x, resolution.y, 1));
		unJitteredBuffer->attachments["depth"].Resize(glm::ivec3(resolution.x, resolution.y, 1));
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
		defaultPayload = bufferHandler_t<defaultUniformBuffer>(camera);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		defaultPayload.data.resolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.translation = camera.translation;
		defaultPayload.data.view = camera.view;

		defaultPayload.Initialize(0);
		jitterUniforms.Initialize(1);
		downscaleUniforms.Initialize(2);
		lanzcosUniforms.Initialize(3);
		defaultVertexBuffer.SetupDefault();
	}

};

#endif