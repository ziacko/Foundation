#ifndef SHARPEN_H
#define SHARPEN_H
#include <textured.h>

struct sharpenSettings_t
{
	GLfloat			kernel1;
	GLfloat			kernel2;

	explicit sharpenSettings_t(
		const GLfloat kernel1 = 2.25f, const GLfloat kernel2 = 10.0f)
	{
		this->kernel1 = kernel1;
		this->kernel2 = kernel2;
	}

	~sharpenSettings_t() = default;
};

class sharpenScene final : public texturedScene
{
public:
	explicit sharpenScene(
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (sharpen)",
		const camera_t sharpencamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: texturedScene(defaultTexture, windowName, sharpencamera, shaderConfigPath) {}

	~sharpenScene() override = default;

protected:

	sharpenSettings_t* sharpen = nullptr;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);
		if (ImGui::BeginTabItem("sharpen"))
		{
			ImGui::SliderFloat("kernel 1", &sharpen->kernel1, -10.0f, 10.0f);
			ImGui::SliderFloat("kernel 2", &sharpen->kernel2, -10.0f, 10.0f);
			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		auto sharpenBlock = &bufferHandler.uniformBlocks["sharpenSettings"];
		sharpenBlock->SetPayload<sharpenSettings_t>(sharpenSettings_t());
		sharpen = sharpenBlock->GetPayload<sharpenSettings_t>();
	}
};

#endif
