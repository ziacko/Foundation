#ifndef CHROMATICABERRATION_H
#define CHROMATICABERRATION_H
#include <textured.h>

struct chromaticSettings_t
{
	float			redOffset;
	float			blueOffset;
	float			greenOffset;

	chromaticSettings_t(GLfloat redOffset = 0.0f, GLfloat greenOffset = 0.0f, GLfloat blueOffset = 0.0f)
	{
		this->redOffset = redOffset;
		this->greenOffset = greenOffset;
		this->blueOffset = blueOffset;
	}

	~chromaticSettings_t(){};
};

class chromaticScene : public texturedScene
{
public:

	chromaticScene(
		chromaticSettings_t chromaticSettings = chromaticSettings_t(),
		texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (chromatic aberration)",
		camera_t chromaticCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) : texturedScene(defaultTexture, windowName, chromaticCamera, shaderConfigPath)
	{
		this->chromaticSettings = chromaticSettings;
	}

	~chromaticScene( void ){}

protected:

	bufferHandler_t<chromaticSettings_t>	chromaticSettings;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		if (ImGui::BeginTabItem("chromatic Abberration"))
		{
			ImGui::SliderFloat("red offset", &chromaticSettings.data.redOffset, -1.0f, 1.0f, "%0.10f");
			ImGui::SliderFloat("green offset", &chromaticSettings.data.greenOffset, -1.0f, 1.0f, "%0.10f");
			ImGui::SliderFloat("blue offset", &chromaticSettings.data.blueOffset, -1.0f, 1.0f, "%0.10f");

			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		chromaticSettings.Initialize(1);
	}

	void Update() override
	{
		scene::Update();
		chromaticSettings.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	}
};
#endif