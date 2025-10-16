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
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (dilation)",
		const camera_t dilationCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: texturedScene(defaultTexture, windowName, dilationCamera, shaderConfigPath) {}

	~dilationScene() override = default;

protected:

	dilationSettings_t*		dilation;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		if (ImGui::BeginTabItem("dilation"))
		{
			ImGui::SliderFloat("dilation strength X", &dilation->strengthX, 0.0f, 10.0f);
			ImGui::SliderFloat("dilation strength Y", &dilation->strengthY, 0.0f, 10.0f);
			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		auto dilationBlock = &bufferHandler.uniformBlocks["dilationSettings"];
		dilationBlock->SetPayload<dilationSettings_t>(dilationSettings_t());
		dilation = dilationBlock->GetPayload<dilationSettings_t>();
	}
};

#endif
