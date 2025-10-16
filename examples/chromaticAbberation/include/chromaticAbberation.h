#ifndef CHROMATICABERRATION_H
#define CHROMATICABERRATION_H
#include <textured.h>

struct chromaticSettings_t
{
	float			redOffset;
	float			blueOffset;
	float			greenOffset;

	explicit chromaticSettings_t(const GLfloat redOffset = -0.01f, const GLfloat greenOffset = 0.003f, const GLfloat blueOffset = 0.0012f)
	{
		this->redOffset = redOffset;
		this->greenOffset = greenOffset;
		this->blueOffset = blueOffset;
	}

	~chromaticSettings_t() = default;
};

class chromaticScene final : public texturedScene
{
public:

	explicit chromaticScene(
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (chromatic aberration)",
		const camera_t chromaticCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) : texturedScene(defaultTexture, windowName, chromaticCamera, shaderConfigPath) {}

	~chromaticScene() override = default;

protected:

	chromaticSettings_t*	chromaticSettings = nullptr;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		if (ImGui::BeginTabItem("chromatic Abberration"))
		{
			ImGui::SliderFloat("red offset", &chromaticSettings->redOffset, -1.0f, 1.0f, "%0.10f");
			ImGui::SliderFloat("green offset", &chromaticSettings->greenOffset, -1.0f, 1.0f, "%0.10f");
			ImGui::SliderFloat("blue offset", &chromaticSettings->blueOffset, -1.0f, 1.0f, "%0.10f");

			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		const auto chromaticBlock = &bufferHandler.uniformBlocks["chromaticSettings"];
		chromaticBlock->SetPayload<chromaticSettings_t>(chromaticSettings_t());
		chromaticSettings = chromaticBlock->GetPayload<chromaticSettings_t>();
	}
};
#endif