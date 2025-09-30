#ifndef EROSION_H
#define EROSION_H
#include <textured.h>

struct erosionSettings_t
{
	float			strengthX;
	float			strengthY;

	explicit erosionSettings_t(const float strengthX = 1.0f, const float strengthY = 1.0f)
	{
		this->strengthX = strengthX;
		this->strengthY = strengthY;
	}

	~erosionSettings_t() = default;
};

class erosionScene final : public texturedScene
{
public:
	explicit erosionScene(
		const erosionSettings_t erosionSettings = erosionSettings_t(),
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (erosion)",
		const camera_t erosionCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: texturedScene(defaultTexture, windowName, erosionCamera, shaderConfigPath)
	{
		this->erosion = erosionSettings;
	}

	~erosionScene() override = default;

protected:

	bufferHandler_t<erosionSettings_t>	erosion;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		if (ImGui::BeginTabItem("erosion"))
		{
			ImGui::SliderFloat("erosion strength X", &erosion.data.strengthX, 0.0f, 10.0f);
			ImGui::SliderFloat("erosion strength Y", &erosion.data.strengthY, 0.0f, 10.0f);
			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		erosion.Initialize(1);
	}

	void Update() override
	{
		scene::Update();
		erosion.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	}
};

#endif