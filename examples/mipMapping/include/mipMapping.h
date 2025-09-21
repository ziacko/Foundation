#ifndef MIPMAPPING_H
#define MIPMAPPING_H
#include "textureSettings.h"

struct mipSettings_t
{
	float level = 1.0f;
};

class mipMappingScene : public textureSettingsScene
{
public:
	//this was never finished so I'm going to leave it for last
	explicit mipMappingScene(texture defaultTexture = texture("textures/earth_diffuse.tga",
	                                                          texture::textureType_t::image, "defaultTexture",
	                                                          textureDescriptor()),
	                         const char* windowName = "Ziyad Barakat's Portfolio (mip mapping)",
	                         camera_t edgeCamera = camera_t(),
	                         const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: textureSettingsScene(defaultTexture, windowName, edgeCamera, shaderConfigPath)
	{
		mip = bufferHandler_t<mipSettings_t>();
	}

	void Initialize() override
	{
		defaultTexture.texDesc.mipmapLevels = 10;
		texturedScene::Initialize();
		defProgram = shaderProgramsMap["mipMapping"];
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		//mip.data.level = 10.0f;

		ImGui::SliderFloat("mip level", &mip.data.level, 0, 10);
		DrawTextureSettings();
	}

	void DrawTextureSettings() override
	{
		//min
		/*if (ImGui::ListBox("min filter setting", &minFilterIndex, filterSettings.data(), filterSettings.size()))
		{
			glFinish();
			defaultTexture->SetMinFilter(minFilterIndex);
		}*/
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		mip.Initialize(1);
	}

	void Update() override
	{
		scene::Update();
		mip.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	}

protected:
	bufferHandler_t<mipSettings_t> mip;
};
#endif
