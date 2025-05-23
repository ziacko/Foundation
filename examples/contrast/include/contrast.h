#ifndef CONTRAST_H
#define CONTRAST_H

#include <textured.h>

class contrastScene : public texturedScene
{
public:

	contrastScene(
		bufferHandler_t<float> contrastSettings = bufferHandler_t<float>(1.2f),
		texture* defaultTexture = new texture(),
		const char* windowName = "Ziyad Barakat's portfolio (contrast)",
		camera_t* contrastCamera = new camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: texturedScene(defaultTexture, windowName, contrastCamera, shaderConfigPath)
	{
		this->contrastSettings = contrastSettings;
	}

	~contrastScene(){};

protected:

	bufferHandler_t<float>		contrastSettings;

	void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		texturedScene::BuildGUI(window, io);
		ImGui::SliderFloat("contrast level", &contrastSettings.data, 0.0f, 10.0f);
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		contrastSettings.Initialize(1);
	}

	void Update() override
	{
		scene::Update();
		contrastSettings.Update(gl_uniform_buffer, gl_dynamic_draw);
	}
};

#endif
