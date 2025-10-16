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
		: texturedScene(defaultTexture, windowName, erosionCamera, shaderConfigPath) {}

	~erosionScene() override = default;

protected:

	erosionSettings_t*	erosion = nullptr;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		if (ImGui::BeginTabItem("erosion"))
		{
			ImGui::SliderFloat("erosion strength X", &erosion->strengthX, 0.0f, 10.0f);
			ImGui::SliderFloat("erosion strength Y", &erosion->strengthY, 0.0f, 10.0f);
			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();

		auto erosionBlock = &bufferHandler.uniformBlocks["erosionSettings"];
		erosionBlock->SetPayload<erosionSettings_t>(erosionSettings_t());
		erosion = erosionBlock->GetPayload<erosionSettings_t>();
	}
};

#endif