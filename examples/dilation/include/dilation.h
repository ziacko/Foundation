#ifndef DILATION_H
#define DILATION_H
#include <textured.h>

struct dilationSettings_t
{
	float			strengthX;
	float			strengthY;

	explicit dilationSettings_t(
		GLfloat const strengthX = 1.0f, const GLfloat strengthY = 1.0f)
	{
		this->strengthX = strengthX;
		this->strengthY = strengthY;
	}

	~dilationSettings_t() = default;
};

class dilationScene final : public texturedScene
{
public:
	explicit dilationScene(
		bufferHandler_t<dilationSettings_t> dilationSettings = bufferHandler_t<dilationSettings_t>(),
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (dilation)",
		const camera_t dilationCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: texturedScene(defaultTexture, windowName, dilationCamera, shaderConfigPath)
	{
		this->dilation = dilationSettings;
	}

	~dilationScene() override = default;

protected:

	bufferHandler_t<dilationSettings_t>		dilation;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		if (ImGui::BeginTabItem("dilation"))
		{
			ImGui::SliderFloat("dilation strength X", &dilation.data.strengthX, 0.0f, 10.0f);
			ImGui::SliderFloat("dilation strength Y", &dilation.data.strengthY, 0.0f, 10.0f);
			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		dilation.Initialize(1);
	}

	void Update() override
	{
		scene::Update();
		dilation.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	}
};

#endif
