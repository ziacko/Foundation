#ifndef RADIAL_BLUR_H
#define RADIAL_BLUR_H
#include <textured.h>

struct radialBlur_t
{
	GLfloat		exposure;
	GLfloat		decay;
	GLfloat		density;
	GLfloat		weight;
	int			samples;

	explicit radialBlur_t(const GLfloat exposure = 0.01f, const GLfloat decay = 0.975f, const GLfloat density = 0.0005f,
	                      const GLfloat weight = 1.0f, const GLuint samples = 100)
	{
		this->exposure = exposure;
		this->decay = decay;
		this->density = density;
		this->weight = weight;
		this->samples = samples;
	}

	~radialBlur_t() = default;
};

class radialScene final : public texturedScene
{
public:
	explicit radialScene(const texture defaultTexture = texture(),
						 const char* windowName = "Ziyad Barakat's Portfolio (radial blur)",
						 const camera_t radialCamera = camera_t(),
						 const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		texturedScene(defaultTexture, windowName, radialCamera, shaderConfigPath)
	{
	}

	~radialScene() override {};

protected:

	radialBlur_t*		radialBlur = nullptr;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);
		if (ImGui::BeginTabItem("radial blur settings"))
		{
			ImGui::SliderFloat("exposure", &radialBlur->exposure, 0.0f, 1.0f);
			ImGui::SliderFloat("decay", &radialBlur->decay, 0.0f, 1.0f);
			//ImGui::SliderFloat("density", &radialBlur->density, 0.0f, 1.0f, "%.5f", 100.0f);
			ImGui::SliderFloat("weight", &radialBlur->weight, 0.0f, 10.0f);
			ImGui::SliderInt("samples", &radialBlur->samples, 0, 1000);

			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		const auto radialBlock = &bufferHandler.uniformBlocks["radialSettings"];
		radialBlock->SetPayload<radialBlur_t>(radialBlur_t());
		radialBlur = radialBlock->GetPayload<radialBlur_t>();
	}

};
#endif