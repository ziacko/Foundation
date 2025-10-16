#ifndef GAUSSIAN_BLURRING_H
#define GAUSSIAN_BLURRING_H
#include <textured.h>

struct gaussianSettings_t
{
	//damn 5 points is so fkn awkward
	int		offset1;
	int		offset2;
	int		offset3;
	int		offset4;
	int		offset5;

	float		weight1;
	float		weight2;
	float		weight3;
	float		weight4;
	float		weight5;

	explicit gaussianSettings_t(const int offset1 = 0, const int offset2 = 1, const int offset3 = 2, const int offset4 = 3, const int offset5 = 4,
	                            const float weight1 = 0.2270270270f, const float weight2 = 0.1945945946, const float weight3 = 0.1216216216, const float weight4 = 0.0540540541, const float weight5 = 0.0162162162)
	{
		this->offset1 = offset1;
		this->offset2 = offset2;
		this->offset3 = offset3;
		this->offset4 = offset4;
		this->offset5 = offset5;

		this->weight1 = weight1;
		this->weight2 = weight2;
		this->weight3 = weight3;
		this->weight4 = weight4;
		this->weight5 = weight5;
	}

	~gaussianSettings_t() = default;
};

class gaussianScene final : public texturedScene
{
public:
	explicit gaussianScene(const texture& defaultTexture = texture(),
	                       const char* windowName = "Ziyad Barakat's Portfolio (gaussian blurring)",
	                       const camera_t& textureCamera = camera_t(),
	                       const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		texturedScene(defaultTexture, windowName, textureCamera, shaderConfigPath)
	{
	}

	~gaussianScene() override {};

protected:

	gaussianSettings_t*		gaussian = nullptr;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		if (ImGui::BeginTabItem("gaussian settings"))
		{
			ImGui::PushItemWidth(100.0f);
			ImGui::SliderFloat("weight1", &gaussian->weight1, 0.0f, 0.25f, "%.10f");
			ImGui::SameLine();
			ImGui::SliderFloat("weight2", &gaussian->weight2, 0.0f, 0.25f, "%.10f");

			ImGui::SliderFloat("weight3", &gaussian->weight3, 0.0f, 0.25f, "%.10f");
			ImGui::SameLine();
			ImGui::SliderFloat("weight4", &gaussian->weight4, 0.0f, 0.25f, "%.10f");
			ImGui::SliderFloat("weight5", &gaussian->weight5, 0.0f, 0.25f, "%.10f");
			ImGui::PopItemWidth();

			ImGui::SliderInt("offset1", &gaussian->offset1, 0, 100);
			ImGui::SliderInt("offset2", &gaussian->offset2, 0, 100);
			ImGui::SliderInt("offset3", &gaussian->offset3, 0, 100);
			ImGui::SliderInt("offset4", &gaussian->offset4, 0, 100);
			ImGui::SliderInt("offset5", &gaussian->offset5, 0, 100);
			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		auto gaussianBlock = &bufferHandler.uniformBlocks["gaussianSettings"];
		gaussianBlock->SetPayload<gaussianSettings_t>(gaussianSettings_t());
		gaussian = gaussianBlock->GetPayload<gaussianSettings_t>();
	}
};

#endif