#ifndef GAMMA_H
#define GAMMA_H
#include <textured.h>

class gammaScene final : public texturedScene
{
public:
	explicit gammaScene(
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (gamma)",
		const camera_t gammaCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: texturedScene(defaultTexture, windowName, gammaCamera, shaderConfigPath)
	{
	}

	~gammaScene() override = default;

protected:

	glm::vec3*		gamma = nullptr;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);
		if (ImGui::BeginTabItem("gamma"))
		{
			ImGui::SliderFloat("gamma red", &gamma->r, 0.f, 10.0f);
			ImGui::SliderFloat("gamma green", &gamma->g, 0.0f, 10.0f);
			ImGui::SliderFloat("gamma blue", &gamma->b, 0.0f, 10.0f);
			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		const auto gammaBlock = &bufferHandler.uniformBlocks["gammaSettings"];
		gammaBlock->SetPayload<glm::vec3>(glm::vec3(0.33f, 0.33f, 0.33f));
		gamma = gammaBlock->GetPayload<glm::vec3>();
	}
};

#endif