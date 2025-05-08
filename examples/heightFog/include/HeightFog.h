#ifndef HEIGHT_FOG_H
#define HEIGHT_FOG_H
#include "../../Displacement/include/Displacement.h"
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
		camera_t* texModelCamera = new camera_t(glm::vec2(1280, 720), 0.1f, camera_t::projection_e::perspective, 0.1f, 1000000.f),
		const char* shaderConfigPath = "shaders/HeightFog.txt")
		: displacement(windowName, texModelCamera, shaderConfigPath)
	{
		diffuseMap = new texture("../../resources/textures/rock_diffuse.tga");
		displacementMap = new texture("../../resources/textures/rock_offset.tga");

		geometryBuffer = new frameBuffer();
		postCamera = new camera_t();


		postCamera->ChangeProjection(camera_t::projection_e::orthographic);
		postCamera->Update();
	}

	~heightFog(){};

	virtual void Initialize() override
	{
		texturedModel::Initialize();
		displacementBuffer.Initialize(1);

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height, 1);

		FBODescriptor worldPosDesc;
		worldPosDesc.dataType = GL_FLOAT;
		worldPosDesc.format = GL_RGB;
		worldPosDesc.internalFormat = gl_rgb32f;
		worldPosDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height, 1);
		
		FBODescriptor depthDesc;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = gl_depth_component24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_t::depth;
		depthDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height, 1);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("position", worldPosDesc));
		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("depth", depthDesc));

		frameBuffer::Unbind();

		renderGrid = new grid(glm::ivec2(100));

		diffuseMap->LoadTexture();
		displacementMap->LoadTexture();

		heightFogProgram = shaderPrograms[1]->handle;

		SetupVertexBuffer();
	}

protected:

	//make a grid and dump it into the engine
	bufferHandler_t<fogSettings_t>		fog;
	unsigned int						heightFogProgram = 0;
	frameBuffer*						geometryBuffer;
	camera_t*							postCamera;

	void InitializeUniforms() override
	{
		displacement::InitializeUniforms();
		fog.Initialize(2);
	}

	virtual void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		displacement::BuildGUI(window, io);
		ImGui::Begin("fog setttings");
		ImGui::DragFloat("strength", &fog.data.strength, 0.001f, 0.0f, 1.0f);
		ImGui::DragFloat3("sun direction", &fog.data.sunDirection[0], 1.0f, 0.0f, 360.0f);
		ImGui::DragFloat3("extinction", &fog.data.extinction[0], 0.001f, 0.0f, 1.0f);
		ImGui::DragFloat3("inscattering", &fog.data.inscattering[0], 0.001f, 0.0f, 1.0f);
		ImGui::ColorPicker3("color", &fog.data.color[0]);
		ImGui::End();

		DrawBufferAttachments();
	}

	virtual void Update() override
	{
		displacement::Update();
		
		fog.data.projection = sceneCamera->projection;
		fog.data.view = sceneCamera->view;
		fog.Update();
	}

	virtual void Draw() override
	{
		//put camera in perspective mode and make a copy for the fog pass
		sceneCamera->Update();
		sceneCamera->UpdateProjection();
		UpdateDefaultBuffer(sceneCamera);
		GeomPass();

		//put camera in orthographic mode
		postCamera->Update();
		postCamera->UpdateProjection();
		UpdateDefaultBuffer(postCamera);
		FogPass();
	
		DrawGUI(windows[0]);

		windows[0]->SwapDrawBuffers();
		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void GeomPass()
	{
		geometryBuffer->Bind();
		geometryBuffer->DrawAll();

		glBindVertexArray(renderGrid->vertexArrayHandle);
		glUseProgram(this->programGLID);

		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);

		diffuseMap->SetActive(0);
		displacementMap->SetActive(1);

		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		//gotta change this to patches for tessellation
		glDrawElements(gl_patches, renderGrid->indices.size(), GL_UNSIGNED_INT, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		geometryBuffer->Unbind();
	}

	virtual void FogPass()
	{
		//just draw to back buffer for now
		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		
		geometryBuffer->attachments[0]->SetActive(0); //color
		geometryBuffer->attachments[1]->SetActive(1); //position
		geometryBuffer->attachments[2]->SetActive(2); //depth

		glUseProgram(heightFogProgram);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	virtual void DrawBufferAttachments()
	{
		ImGui::Begin("framebuffers");
		for (auto iter : geometryBuffer->attachments)
		{
			ImGui::Image((ImTextureID*)iter->GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", iter->GetUniformName().c_str());
		}
		ImGui::End();
	}

	void UpdateDefaultBuffer(camera_t* newCamera)
	{
		defaultPayload.data.projection = newCamera->projection;
		defaultPayload.data.view = newCamera->view;
		defaultPayload.data.resolution = newCamera->resolution;
		if (newCamera->currentProjectionType == camera_t::projection_e::perspective)
		{
			defaultPayload.data.translation = testModel->makeTransform();
		}

		else
		{
			defaultPayload.data.translation = newCamera->translation;
		}
		defaultPayload.data.deltaTime = (float)sceneClock->GetDeltaTime();
		defaultPayload.data.totalTime = (float)sceneClock->GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / sceneClock->GetDeltaTime());

		defaultPayload.Update();
		defaultVertexBuffer->UpdateBuffer(defaultPayload.data.resolution);
	}

	void ClearBuffers()
	{
		float clearColor1[4] = { 0.05f, 0.05f, 0.05f, 0.0f };
		geometryBuffer->Bind();
		for (auto iter : geometryBuffer->attachments)
		{
			geometryBuffer->ClearTexture(iter, clearColor1);
		}
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		for (auto iter : geometryBuffer->attachments)
		{
			iter->Resize(glm::ivec3(resolution, 1));
		}

		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
	}

	virtual void HandleWindowResize(tWindow* window, TinyWindow::vec2_t<unsigned int> dimensions) override
	{
		defaultPayload.data.resolution = glm::ivec2(dimensions.width, dimensions.height);
		ResizeBuffers(glm::ivec2(dimensions.x, dimensions.y));
	}

	virtual void HandleMaximize(tWindow* window) override
	{
		defaultPayload.data.resolution = glm::ivec2(window->settings.resolution.width, window->settings.resolution.height);
		ResizeBuffers(defaultPayload.data.resolution);
	}
};

#endif