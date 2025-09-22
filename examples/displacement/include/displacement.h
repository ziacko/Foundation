#ifndef DISPLACEMENT_H
#define DISPLACEMENT_H
#include "scene3D.h"
#include "Grid.h"

struct displacementSettings_t
{
	float outerTessLevel;
	float innerTessLevel;
	float offsetStrength;

	displacementSettings_t(float outerTessLevel = 10.0f, float innerTessLevel = 10.0f, float offsetStrength = 1.0f)
	{
		this->outerTessLevel = outerTessLevel;
		this->innerTessLevel = innerTessLevel;
		this->offsetStrength = offsetStrength;
	}

	~displacementSettings_t() {};
};

class displacement : public scene3D
{
public:

	displacement(
		const char* windowName = "Ziyad Barakat's portfolio (displacement)",
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 0.1f, camera_t::projection_e::perspective, 0.1f, 1000000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: scene3D(windowName, texModelCamera, shaderConfigPath)
	{
		diffuseMap = new texture("textures/rock_diffuse.tga");
		displacementMap = new texture("textures/rock_offset.tga");



		this->camera.position.y = 4.2f;
		this->camera.position.z = 3.0f;

		this->camera.Roll(glm::radians(-90.0f));
		this->camera.Pitch(glm::radians(-90.0f));
	}

	~displacement(){};

	virtual void Initialize() override
	{
		scene3D::Initialize();
		displacementBuffer.Initialize(1);

		renderGrid = new grid(glm::ivec2(10));

		diffuseMap->LoadTexture();
		displacementMap->LoadTexture();
	}

protected:

	//make a grid and dump it into the engine

	int currentTexture = 0;
	std::vector<const char*> textureNames;
	grid* renderGrid;
	texture* displacementMap;
	texture* diffuseMap;
	bufferHandler_t<displacementSettings_t> displacementBuffer;

	void InitializeUniforms() override
	{
		scene3D::InitializeUniforms();
		displacementBuffer.Initialize(1);
	}

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);
		if (ImGui::BeginTabItem("textures"))
		{
			//ImGui::ListBox("loaded textures", &currentTexture, textureNames.data(), textureNames.size());
			ImGui::SliderFloat("outer tessellation", &displacementBuffer.data.outerTessLevel, 1.0f, 100.0f);
			ImGui::SliderFloat("inner tessellation", &displacementBuffer.data.innerTessLevel, 1.0f, 100.0f);
			ImGui::SliderFloat("offset strength", &displacementBuffer.data.offsetStrength, 1.0f, 10.0f);
			ImGui::Image((ImTextureID*)diffuseMap->GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::Image((ImTextureID*)displacementMap->GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::EndTabItem();
		}
	}

	virtual void Update() override
	{
		scene3D::Update();
		displacementBuffer.Update();
	}

	virtual void Draw() override
	{
		DrawMeshes();
		DrawGUI(window);
		manager->SwapDrawBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void DrawMeshes()
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, __FUNCTION__);
		renderGrid->BindVA();
		defProgram.Use();

		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		diffuseMap->SetActive(0);
		displacementMap->SetActive(1);

		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		//gotta change this to patches for tesselation
		glDrawElements(GL_PATCHES, renderGrid->indices.size(), GL_UNSIGNED_INT, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glPopDebugGroup();
	}
};

#endif