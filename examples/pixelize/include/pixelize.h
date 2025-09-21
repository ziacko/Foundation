
#pragma once

#include <textured.h>

struct pixellize_t
{
	float pixelWidth;
	float pixelHeight;

	explicit pixellize_t(const float& inPixelWidth = 1, const float& inPixelHeight = 1)
	: pixelWidth(inPixelWidth), pixelHeight(inPixelHeight)	{}
};

class pixelizeScene : public texturedScene
{
public:

	explicit pixelizeScene(bufferHandler_t<pixellize_t> pixelSettings = bufferHandler_t<pixellize_t>(),
		texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (pixellize)",
		camera_t parallaxCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		texturedScene(defaultTexture, windowName, parallaxCamera, shaderConfigPath)
	{
		this->pixelSettings = pixelSettings;
	}

	~pixelizeScene() override {}

protected:

	bufferHandler_t<pixellize_t> pixelSettings;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);
		if (ImGui::BeginTabItem("pixellize settings"))
		{
			ImGui::SliderFloat("pixel width", &pixelSettings.data.pixelWidth, 0.0f, 100.0f);
			ImGui::SliderFloat("pixel height", &pixelSettings.data.pixelHeight, 0.0f, 100.0f);
			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		pixelSettings.Initialize(1);
	}

	void Update() override
	{
		scene::Update();
		pixelSettings.Update();
	}
};