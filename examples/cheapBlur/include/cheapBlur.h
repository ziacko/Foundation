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

	~jitterSettings_t() {};
};

class cheapBlurScene : public texturedScene
{
public:

	cheapBlurScene(bufferHandler_t<jitterSettings_t> jitterSet = bufferHandler_t<jitterSettings_t>(),
		texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's Portfolio (Cheap blur)",
		camera_t textureCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		texturedScene(defaultTexture, windowName, textureCamera, shaderConfigPath)
	{
		this->jitterSettings = jitterSet;

		for (int iter = 0; iter < 128; iter++)
		{
			jitterSettings.data.haltonSequence[iter] = glm::vec2(CreateHaltonSequence(iter + 1, 2), CreateHaltonSequence(iter + 1, 3));
		}

		accum = 0.0f;
		accumReturn = 0.0f;
		accumMult = 0.0f;
	}

	~cheapBlurScene(){};

	void Initialize() override
	{
		texturedScene::Initialize();
	}

protected:

	bufferHandler_t<jitterSettings_t>		jitterSettings;

	float accum;
	float accumReturn;
	float accumMult;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);
		if (ImGui::BeginTabItem("cheap blur"))
		{
			ImGui::SliderFloat("blur strength", &jitterSettings.data.haltonScale, 0.0f, 50.0f);
			ImGui::SliderFloat("accum strength", &accum, -1.0f, 1.0f);
			ImGui::SliderFloat("accum return strength", &accumReturn, -1.0f, 1.0f);
			ImGui::SliderFloat("accum mult strength", &accumMult, -1.0f, 1.0f);
			ImGui::SliderFloat("dither scale", &jitterSettings.data.ditheringScale, 0.0f, 1.0f);
			ImGui::EndTabItem();
		}

	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		jitterSettings.Initialize(1);
	}

	void Draw() override
	{
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glUseProgram(defProgram.handle);

		defaultTexture.SetActive(0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glAccum(GL_ACCUM, accum); //adding the current frame to the buffer
		glAccum(GL_RETURN, accumReturn); //Drawing last frame, saved in buffer
		glAccum(GL_MULT, accumMult); //make current frame in buffer dim

		DrawGUI(window);
		manager->SwapDrawBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Update() override
	{
		scene::Update();
		jitterSettings.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	}
};

#endif
