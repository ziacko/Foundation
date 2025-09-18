#ifndef GAMMA_H
#define GAMMA_H
#include <textured.h>

class gammaScene : public texturedScene
{
public:

	gammaScene(
		glm::vec3 gammaSettings = glm::vec3(1.2, 3.7, 2.0),
		texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's portfolio (gamma)",
		camera_t gammaCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: texturedScene(defaultTexture, windowName, gammaCamera, shaderConfigPath)
	{
		this->gamma = bufferHandler_t<glm::vec3>(gammaSettings);
	}

	~gammaScene(){};

protected:

	bufferHandler_t<glm::vec3>		gamma;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);
		if (ImGui::BeginTabItem("gamma"))
		{
			ImGui::SliderFloat("gamma red", &gamma.data.r, 0.f, 10.0f);
			ImGui::SliderFloat("gamma green", &gamma.data.g, 0.0f, 10.0f);
			ImGui::SliderFloat("gamma blue", &gamma.data.b, 0.0f, 10.0f);
			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		gamma.Initialize(1);
	}

	void Update() override
	{
		scene::Update();
		gamma.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	}
};

#endif