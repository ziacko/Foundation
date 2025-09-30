#ifndef SEPIA_H
#define SEPIA_H
#include "textured.h"

struct sepiaSettings_t
{
	GLfloat			factor;
	
	GLfloat			redModifier1;
	GLfloat			redModifier2;
	GLfloat			redModifier3;

	GLfloat			greenModifier1;
	GLfloat			greenModifier2;
	GLfloat			greenModifier3;

	GLfloat			blueModifier1;
	GLfloat			blueModifier2;
	GLfloat			blueModifier3;

	explicit sepiaSettings_t(const GLfloat factor = 0.75f,
	                         const GLfloat redModifier1 = 0.393f, const GLfloat redModifier2 = 0.349f, const GLfloat redModifier3 = 0.272f,
	                         const GLfloat greenModifier1 = 0.769f, const GLfloat greenModifier2 = 0.686f, const GLfloat greenModifier3 = 0.534f,
	                         const GLfloat blueModifier1 = 0.189f, const GLfloat blueModifier2 = 0.168f, const GLfloat blueModifier3 = 0.131f)
	{
		this->factor = factor;
		this->redModifier1 = redModifier1;
		this->redModifier2 = redModifier2;
		this->redModifier3 = redModifier3;

		this->greenModifier1 = greenModifier1;
		this->greenModifier2 = greenModifier2;
		this->greenModifier3 = greenModifier3;

		this->blueModifier1 = blueModifier1;
		this->blueModifier2 = blueModifier2;
		this->blueModifier3 = blueModifier3;
	}

	~sepiaSettings_t(){};
};

class sepiaScene final : public texturedScene
{
public:
	explicit sepiaScene(
		bufferHandler_t<sepiaSettings_t> sepiaSettings = bufferHandler_t<sepiaSettings_t>(),
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (sepia)",
		const camera_t sepiaCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: texturedScene(defaultTexture, windowName, sepiaCamera, shaderConfigPath)
	{
		this->sepiaSettings = sepiaSettings;
	}

	~sepiaScene() override = default;

protected:

	bufferHandler_t<sepiaSettings_t>  sepiaSettings;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		if (ImGui::BeginTabItem("sepia"))
		{
			ImGui::SliderFloat("factor", &sepiaSettings.data.factor, 0.0f, 1.0f);

			ImGui::SliderFloat("red modifier 1", &sepiaSettings.data.redModifier1, 0.0f, 1.0f);
			ImGui::SliderFloat("red modifier 2", &sepiaSettings.data.redModifier2, 0.0f, 1.0f);
			ImGui::SliderFloat("red modifier 3", &sepiaSettings.data.redModifier3, 0.0f, 1.0f);

			ImGui::SliderFloat("green modifier 1", &sepiaSettings.data.greenModifier1, 0.0f, 1.0f);
			ImGui::SliderFloat("green modifier 2", &sepiaSettings.data.greenModifier2, 0.0f, 1.0f);
			ImGui::SliderFloat("green modifier 3", &sepiaSettings.data.greenModifier3, 0.0f, 1.0f);

			ImGui::SliderFloat("blue modifier 1", &sepiaSettings.data.blueModifier1, 0.0f, 1.0f);
			ImGui::SliderFloat("blue modifier 2", &sepiaSettings.data.blueModifier2, 0.0f, 1.0f);
			ImGui::SliderFloat("blue modifier 3", &sepiaSettings.data.blueModifier3, 0.0f, 1.0f);

			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		sepiaSettings.Initialize(1);
	}

	void Update() override
	{
		scene::Update();
		sepiaSettings.Update();
	}
};
#endif