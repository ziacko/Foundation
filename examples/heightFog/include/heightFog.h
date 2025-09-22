#ifndef HEIGHT_FOG_H
#define HEIGHT_FOG_H
#include "displacement.h"
#include "Grid.h"
#include "FrameBuffer.h"

struct fogSettings_t
{
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec4 color;
	glm::vec4 sunDirection;
	glm::vec4 extinction;
	glm::vec4 inscattering;
	float strength;

	~fogSettings_t() {};
};

class heightFog : public displacement
{
public:

	heightFog(
		const char* windowName = "Ziyad Barakat's portfolio (height fog)",
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 0.1f, camera_t::projection_e::perspective, 0.1f, 1000000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: displacement(windowName, texModelCamera, shaderConfigPath)
	{
		diffuseMap = new texture("textures/rock_diffuse.tga");
		displacementMap = new texture("textures/rock_offset.tga");

		geometryBuffer = new frameBuffer();
		postCamera = new camera_t();


		postCamera->ChangeProjection(camera_t::projection_e::orthographic);
		postCamera->Update();
	}

	~heightFog(){};

	virtual void Initialize() override
	{
		scene3D::Initialize();
		displacementBuffer.Initialize(1);

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		FBODescriptor worldPosDesc;
		worldPosDesc.dataType = GL_FLOAT;
		worldPosDesc.format = GL_RGB;
		worldPosDesc.internalFormat = GL_RGB32F;
		worldPosDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);
		
		FBODescriptor depthDesc;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer->AddAttachment(frameBuffer::attachment_t("position", worldPosDesc));
		geometryBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		frameBuffer::Unbind();

		renderGrid = new grid(glm::ivec2(100));

		diffuseMap->LoadTexture();
		displacementMap->LoadTexture();

		defProgram = shaderProgramsMap["displacement"];
		heightFogProgram = shaderProgramsMap["heightFog"];

		defaultVertexBuffer.SetupDefault();
	}

protected:

	//make a grid and dump it into the engine
	bufferHandler_t<fogSettings_t>		fog;
	shaderProgram_t						heightFogProgram;
	frameBuffer*						geometryBuffer;
	camera_t*							postCamera;

	void InitializeUniforms() override
	{
		displacement::InitializeUniforms();
		fog.Initialize(2);
	}

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		displacement::BuildGUI(window, io);
		if (ImGui::BeginTabItem("fog setttings"))
		{
			ImGui::DragFloat("strength", &fog.data.strength, 0.001f, 0.0f, 1.0f);
			ImGui::DragFloat3("sun direction", &fog.data.sunDirection[0], 1.0f, 0.0f, 360.0f);
			ImGui::DragFloat3("extinction", &fog.data.extinction[0], 0.001f, 0.0f, 1.0f);
			ImGui::DragFloat3("inscattering", &fog.data.inscattering[0], 0.001f, 0.0f, 1.0f);
			ImGui::ColorPicker3("color", &fog.data.color[0]);
			ImGui::EndTabItem();
		}

		DrawBufferAttachments();
	}

	virtual void Update() override
	{
		displacement::Update();
		
		fog.data.projection = camera.projection;
		fog.data.view = camera.view;
		fog.Update();
	}

	virtual void Draw() override
	{
		//put camera in perspective mode and make a copy for the fog pass
		camera.Update();
		camera.UpdateProjection();
		UpdateDefaultBuffer(&camera);
		GeomPass();

		//put camera in orthographic mode
		postCamera->Update();
		postCamera->UpdateProjection();
		UpdateDefaultBuffer(postCamera);
		FogPass();
	
		DrawGUI(window);

		manager->SwapDrawBuffers(window);
		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void GeomPass()
	{
		geometryBuffer->Bind();
		geometryBuffer->DrawAll();

		glBindVertexArray(renderGrid->vertexArrayHandle);
		defProgram.Use();

		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		diffuseMap->SetActive(0);
		displacementMap->SetActive(1);

		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		//gotta change this to patches for tessellation
		glDrawElements(GL_PATCHES, renderGrid->indices.size(), GL_UNSIGNED_INT, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		geometryBuffer->Unbind();
	}

	virtual void FogPass()
	{
		//just draw to back buffer for now
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		
		geometryBuffer->attachments["color"].SetActive(0); //color
		geometryBuffer->attachments["position"].SetActive(1); //position
		geometryBuffer->attachments["depth"].SetActive(2); //depth

		heightFogProgram.Use();
		glDrawArrays(GL_TRIANGLES, 0, 6);
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
			ImGui::EndTabItem();
		}
	}

	void UpdateDefaultBuffer(camera_t* newCamera)
	{
		defaultPayload.data.projection = newCamera->projection;
		defaultPayload.data.view = newCamera->view;
		defaultPayload.data.resolution = newCamera->resolution;
		if (newCamera->currentProjectionType == camera_t::projection_e::perspective)
		{
			defaultPayload.data.translation = testModel.makeTransform();
		}

		else
		{
			defaultPayload.data.translation = newCamera->translation;
		}
		defaultPayload.data.deltaTime = (float)clock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)clock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / clock.GetDeltaTime());

		defaultPayload.Update();
	}

	void ClearBuffers()
	{
		float clearColor1[4] = { 0.33f, 0.33f, 0.33f, 1.0f };
		geometryBuffer->Bind();
		for (auto iter : geometryBuffer->attachments | std::views::values)
		{
			geometryBuffer->ClearTexture(iter, clearColor1);
		}
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		for (auto iter : geometryBuffer->attachments | std::views::values)
		{
			iter.Resize(glm::ivec3(resolution, 1));
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
};

#endif