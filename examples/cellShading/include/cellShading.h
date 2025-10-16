#ifndef CELL_SHADING_H
#define CELL_SHADING_H
#include <textured.h>

struct cellShadeSettings_t
{
	float		redModifier;
	float		greenModifier;
	float		blueModifier;
	float		cellDistance;

	explicit cellShadeSettings_t(const float cellDistance = 1.0f, const float redModifier = 1.0f,
	                             const float greenModifier = 1.0f, const float blueModifier = 1.0f)
	{
		this->redModifier = redModifier;
		this->greenModifier = greenModifier;
		this->blueModifier = blueModifier;
		this->cellDistance = cellDistance;
	}

	~cellShadeSettings_t() = default;
};

class cellShadingScene final : public texturedScene
{
public:
	explicit cellShadingScene(
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's Portfolio (Cell Shading)",
		const camera_t textureCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		texturedScene(defaultTexture, windowName, textureCamera, shaderConfigPath) {}

	void Initialize() override
	{
		texturedScene::Initialize();
	}

	~cellShadingScene() override = default;

protected:

	cellShadeSettings_t*		cellBuffer = nullptr;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);
		if (ImGui::BeginTabItem("cell shading"))
		{
			ImGui::SliderFloat("red modifier", &cellBuffer->redModifier, 0.0f, 1.0f);
			ImGui::SliderFloat("green modifier", &cellBuffer->greenModifier, 0.0f, 1.0f);
			ImGui::SliderFloat("blue modifier", &cellBuffer->blueModifier, 0.0f, 1.0f);
			ImGui::SliderFloat("cell distance", &cellBuffer->cellDistance, 0.0f, 1.0f);
			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		const auto cellBlock = &bufferHandler.uniformBlocks["cellSettings"];
		cellBlock->SetPayload<cellShadeSettings_t>(cellShadeSettings_t());
		cellBuffer = cellBlock->GetPayload<cellShadeSettings_t>();
	}
};

#endif
