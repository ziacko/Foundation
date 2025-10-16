#ifndef CONTRAST_H
#define CONTRAST_H

#include <textured.h>

class contrastScene : public texturedScene
{
public:

	explicit contrastScene(
		texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (contrast)",
		camera_t contrastCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: texturedScene(defaultTexture, windowName, contrastCamera, shaderConfigPath)
	{
	}

	~contrastScene() override {};

protected:

	float*		contrastSettings;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);
		ImGui::SliderFloat("contrast level", contrastSettings, 0.0f, 10.0f);
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		auto contrastBlock = &bufferHandler.uniformBlocks["contrastSettings"];
		contrastBlock->SetPayload<float>(float(1.2f));
		contrastSettings = contrastBlock->GetPayload<float>();
	}

};

#endif
