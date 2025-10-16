#ifndef CHEAP_BLUR_H
#define CHEAP_BLUR_H

#include <textured.h>
#include <HaltonSequence.h>

struct jitterSettings_t
{
	glm::vec2			haltonSequence[128];
	float				haltonScale;
	float				ditheringScale;
	int					haltonIndex;

	jitterSettings_t()
	{
		haltonIndex = 128;
		haltonScale = 100.0f;
		ditheringScale = 0.66f;
	}

	~jitterSettings_t() = default;
};

class cheapBlurScene : public texturedScene
{
public:

	cheapBlurScene(
		texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's Portfolio (Cheap blur)",
		camera_t textureCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		texturedScene(defaultTexture, windowName, textureCamera, shaderConfigPath)
	{
		jitterSettings = new jitterSettings_t();
		for (int iter = 0; iter < 128; iter++)
		{
			jitterSettings->haltonSequence[iter] = glm::vec2(CreateHaltonSequence(iter + 1, 2), CreateHaltonSequence(iter + 1, 3));
		}

		accum = 0.0f;
		accumReturn = 0.0f;
		accumMult = 0.0f;
	}

	~cheapBlurScene() = default;

	void Initialize() override
	{
		texturedScene::Initialize();
	}

protected:

	jitterSettings_t*		jitterSettings = nullptr;

	float accum;
	float accumReturn;
	float accumMult;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);
		if (ImGui::BeginTabItem("cheap blur"))
		{
			ImGui::SliderFloat("blur strength", &jitterSettings->haltonScale, 0.0f, 50.0f);
			ImGui::SliderFloat("accum strength", &accum, -1.0f, 1.0f);
			ImGui::SliderFloat("accum return strength", &accumReturn, -1.0f, 1.0f);
			ImGui::SliderFloat("accum mult strength", &accumMult, -1.0f, 1.0f);
			ImGui::SliderFloat("dither scale", &jitterSettings->ditheringScale, 0.0f, 1.0f);
			ImGui::EndTabItem();
		}

	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		auto jitterBlock = &bufferHandler.uniformBlocks["jitterSettings"];
		jitterBlock->SetPayload<jitterSettings_t>(*jitterSettings);
		jitterSettings = jitterBlock->GetPayload<jitterSettings_t>();
	}

	void Draw() override
	{
		defaultVertexBuffer.Bind();
		defProgram.Use();

		defaultTexture.SetActive(0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glAccum(GL_ACCUM, accum); //adding the current frame to the buffer
		glAccum(GL_RETURN, accumReturn); //Drawing last frame, saved in buffer
		glAccum(GL_MULT, accumMult); //make current frame in buffer dim
	}
};

#endif
