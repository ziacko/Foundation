#ifndef DISPLACEMENT_H
#define DISPLACEMENT_H
#include "scene3D.h"
#include "Grid.h"

struct displacementSettings_t
{
	float outerTessLevel;
	float innerTessLevel;
	float offsetStrength;

	explicit displacementSettings_t(const float outerTessLevel = 10.0f, const float innerTessLevel = 10.0f, const float offsetStrength = 1.0f)
	{
		this->outerTessLevel = outerTessLevel;
		this->innerTessLevel = innerTessLevel;
		this->offsetStrength = offsetStrength;
	}

	~displacementSettings_t() = default;
};

class displacement final : public scene3D
{
public:
	explicit displacement(
		const char* windowName = "Ziyad Barakat's portfolio (displacement)",
		const camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 0.1f, camera_t::projection_e::perspective, 0.1f, 1000000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: scene3D(windowName, texModelCamera, shaderConfigPath), renderGrid(nullptr)
	{
		diffuseMap = new texture("textures/rock_diffuse.tga");
		displacementMap = new texture("textures/rock_offset.tga");


		this->camera.position.y = 4.2f;
		this->camera.position.z = 3.0f;

		this->camera.Roll(glm::radians(-90.0f));
		//this->camera.Pitch(glm::radians(-45.0f));
	}

	~displacement() override = default;

	virtual void Initialize() override
	{
		scene3D::Initialize();

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
	displacementSettings_t* displacementBuffer = nullptr;

	void InitializeUniforms() override
	{
		scene3D::InitializeUniforms();
		auto displacementBlock = &bufferHandler.uniformBlocks["displacementSettings"];
		displacementBlock->SetPayload(displacementSettings_t());
		displacementBuffer = displacementBlock->GetPayload<displacementSettings_t>();
	}

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);
		if (ImGui::BeginTabItem("textures"))
		{
			//ImGui::ListBox("loaded textures", &currentTexture, textureNames.data(), textureNames.size());
			ImGui::SliderFloat("outer tessellation", &displacementBuffer->outerTessLevel, 1.0f, 100.0f);
			ImGui::SliderFloat("inner tessellation", &displacementBuffer->innerTessLevel, 1.0f, 100.0f);
			ImGui::SliderFloat("offset strength", &displacementBuffer->offsetStrength, 1.0f, 10.0f);
			ImGui::Image((ImTextureID*)diffuseMap->GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::Image((ImTextureID*)displacementMap->GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::EndTabItem();
		}
	}

	virtual void Draw() override
	{
		GL_PUSH_DEBUG_GROUP();
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
		glDrawElements(GL_PATCHES, renderGrid->indices.size(), GL_UNSIGNED_INT, nullptr);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glPopDebugGroup();
	}
};

#endif