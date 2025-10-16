#pragma once

#include <textured.h>

struct pixellize_t
{
	float pixelWidth;
	float pixelHeight;

	explicit pixellize_t(const float& inPixelWidth = 3.33f, const float& inPixelHeight = 3.33f)
	: pixelWidth(inPixelWidth), pixelHeight(inPixelHeight)	{}
};

class pixelizeScene final : public texturedScene
{
public:

	explicit pixelizeScene(
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (pixellize)",
		const camera_t parallaxCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		texturedScene(defaultTexture, windowName, parallaxCamera, shaderConfigPath) {}

	~pixelizeScene() override = default;

protected:

	pixellize_t* pixelSettings = nullptr;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);
		if (ImGui::BeginTabItem("pixellize settings"))
		{
			ImGui::SliderFloat("pixel width", &pixelSettings->pixelWidth, 0.0f, 100.0f);
			ImGui::SliderFloat("pixel height", &pixelSettings->pixelHeight, 0.0f, 100.0f);
			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		auto pixelBlock = &bufferHandler.uniformBlocks["pixelize"];
		pixelBlock->SetPayload<pixellize_t>(pixellize_t());
		pixelSettings = pixelBlock->GetPayload<pixellize_t>();
	}
};