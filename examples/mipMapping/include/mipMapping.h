#ifndef MIPMAPPING_H
#define MIPMAPPING_H
#include "textureSettings.h"

struct mipSettings_t
{
	float level = 1.0f;
};

class mipMappingScene final : public textureSettingsScene
{
public:
	//this was never finished so I'm going to leave it for last
	explicit mipMappingScene(const texture defaultTexture = texture("textures/earth_diffuse.tga"),
	                         const char* windowName = "Ziyad Barakat's Portfolio (mip mapping)",
	                         const camera_t edgeCamera = camera_t(),
	                         const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: textureSettingsScene(defaultTexture, windowName, edgeCamera, shaderConfigPath)
	{
	}

	void Initialize() override
	{
		defaultTexture.texDesc.hasMips = true;
		defaultTexture.texDesc.levels = 10;
		texturedScene::Initialize();
		defProgram = shaderProgramsMap["mipMapping"];
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		//mip.data.level = 10.0f;

		ImGui::SliderFloat("mip level", &mip->level, 0, 10);
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		auto mipBlock = &bufferHandler.uniformBlocks["mipSettings"];
		mipBlock->SetPayload<mipSettings_t>(mipSettings_t());
		mip = mipBlock->GetPayload<mipSettings_t>();
	}

protected:
	mipSettings_t* mip = nullptr;
};
#endif
