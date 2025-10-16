#ifndef PERLINNOISE_H
#define PERLINNOISE_H
#include "scene.h"
#include "Texture.h"
#include "FrameBuffer.h"

struct perlinSettings_t//	: public uniformBuffer_t
{
	glm::vec2		uvOffset;
	glm::vec2		uvScale;
	float			modValue;
	float			permuteValue;
	float			taylorInverse;
	float			fadeValue1;
	float			fadeValue2;
	float			fadeValue3;
	int				numOctaves;

	float			noiseValue;
	float			noiseValue2;
	int				colorBias;//20

	/*
	GLfloat			pattern1Value1;//2.5
	GLfloat			pattern1Value2;//0.4
	GLfloat			pattern1Value3;//5.2
	GLfloat			pattern1Value4;//1.3
	GLfloat			pattern1Value5;//4.0
	GLfloat			pattern1Value6;//1.7
	GLfloat			pattern1Value7;//9.2
	GLfloat			pattern1Value8;//4.0
	GLfloat			pattern1Value9;//8.3
	GLfloat			pattern1Value10;//2.8
	GLfloat			pattern1Value11;//4.0*/

	float			pattern2Value1;//0.3
	float			pattern2Value2;//0.8
	float			pattern2Value3;//5.2
	float			pattern2Value4;//1.3
	float			pattern2Value5;//4.0
	float			pattern2Value6;//1.7
	float			pattern2Value7;//9.2
	float			pattern2Value8;//4.0
	float			pattern2Value9;//8.3
	float			pattern2Value10;//2.8
	float			pattern2Value11;//4.0


	perlinSettings_t(GLfloat modValue = 289.0f, GLfloat permuteValue = 29.33f,
		GLfloat taylorInverse = 1.79284291400159f, GLfloat fadeValue1 = 6.0f, GLfloat fadeValue2 = 15.0f,
		GLfloat fadeValue3 = 10.0f, GLuint numOctaves = 20,
		GLuint colorBias = 2,
		GLfloat noiseValue = 79.66f, GLfloat noiseValue2 = 5.6f,

		GLfloat pattern2Value1 = 2.5f, GLfloat pattern2Value2 = 0.4f, GLfloat pattern2Value3 = 5.2f,
		GLfloat pattern2Value4 = 1.3f, GLfloat pattern2Value5 = 4.0f, GLfloat pattern2Value6 = 1.7f,
		GLfloat pattern2Value7 = 9.2f, GLfloat pattern2Value8 = 4.0f, GLfloat pattern2Value9 = 8.3f,
		GLfloat pattern2Value10 = 2.8f, GLfloat	pattern2Value11 = 2.02f,

		GLfloat pattern1Value1 = 0.3f, GLfloat pattern1Value2 = 0.8f, GLfloat pattern1Value3 = 5.2f,
		GLfloat pattern1Value4 = 1.3f, GLfloat pattern1Value5 = 4.0f, GLfloat pattern1Value6 = 1.7f,
		GLfloat pattern1Value7 = 9.2f, GLfloat pattern1Value8 = 4.0f, GLfloat pattern1Value9 = 8.3f,
		GLfloat pattern1Value10 = 2.8f, GLfloat	pattern1Value11 = 4.0f)
	{
		this->modValue = modValue;
		this->permuteValue = permuteValue;
		this->taylorInverse = taylorInverse;
		this->fadeValue1 = fadeValue1;
		this->fadeValue2 = fadeValue2;
		this->fadeValue3 = fadeValue3;
		this->numOctaves = numOctaves;

		this->colorBias = colorBias;

		this->noiseValue = noiseValue;
		this->noiseValue2 = noiseValue2;

		/*this->pattern1Value1 = pattern1Value1;
		this->pattern1Value2 = pattern1Value2;
		this->pattern1Value3 = pattern1Value3;
		this->pattern1Value4 = pattern1Value4;
		this->pattern1Value5 = pattern1Value5;
		this->pattern1Value6 = pattern1Value6;
		this->pattern1Value7 = pattern1Value7;
		this->pattern1Value8 = pattern1Value8;
		this->pattern1Value9 = pattern1Value9;
		this->pattern1Value10 = pattern1Value10;
		this->pattern1Value11 = pattern1Value11;*/

		this->pattern2Value1 = pattern2Value1;
		this->pattern2Value2 = pattern2Value2;
		this->pattern2Value3 = pattern2Value3;
		this->pattern2Value4 = pattern2Value4;
		this->pattern2Value5 = pattern2Value5;
		this->pattern2Value6 = pattern2Value6;
		this->pattern2Value7 = pattern2Value7;
		this->pattern2Value8 = pattern2Value8;
		this->pattern2Value9 = pattern2Value9;
		this->pattern2Value10 = pattern2Value10;
		this->pattern2Value11 = pattern2Value11;

		uvOffset = glm::vec2(0);
		uvScale = glm::vec2(1);
	};

	~perlinSettings_t() = default;
};

class perlinScene : public scene
{
public:
	explicit perlinScene(const char* windowName = "Ziyad Barakat's Portfolio ( Perlin noise )",
	                     const camera_t perlinCamera = camera_t(), const GLchar* shaderConfigPath = SHADER_CONFIG_DIR)
		: scene(windowName, perlinCamera, shaderConfigPath)
	{
		perlinBuffer = new frameBuffer();
	}

	void Initialize() override
	{
		scene::Initialize();
		perlinBuffer->Initialize();
		perlinBuffer->Bind();

		FBODescriptor perlinDesc;
		perlinDesc.dataType = GL_FLOAT;
		perlinDesc.format = GL_RGBA;
		perlinDesc.internalFormat = GL_RGBA16_SNORM;
		perlinDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		perlinBuffer->AddAttachment(frameBuffer::attachment_t("perlin", perlinDesc));

		defProgram = shaderProgramsMap["perlin"];
		finalProgram = shaderProgramsMap["textured"];

		frameBuffer::Unbind();
	}

protected:

	perlinSettings_t*	perlin = nullptr;
	frameBuffer*		perlinBuffer;
	shaderProgram_t		finalProgram;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene::BuildGUI(window, io); 

		ImGui::SliderFloat2("offset", &perlin->uvOffset[0], 0, 10);
		ImGui::SliderFloat2("scale", &perlin->uvScale[0], 0.1, 100);

		ImGui::SliderFloat("modifier value", &perlin->modValue, 0.0f, 1000.0f);
		ImGui::SliderFloat("permutation value", &perlin->permuteValue, 0.0f, 100.0f);
		ImGui::SliderFloat("taylor inverse", &perlin->taylorInverse, 0.0f, 10.0f);

		AddGUISpacer();

		ImGui::SliderInt("num octaves", &perlin->numOctaves, 0, 100);
		
		AddGUISpacer();

		ImGui::SliderFloat("noise value", &perlin->noiseValue, 0, 100);
		ImGui::SliderFloat("noise value 2", &perlin->noiseValue2, 0, 100);
		ImGui::SliderInt("color bias", &perlin->colorBias, 0, 100);

		AddGUISpacer();
		ImGui::SliderFloat("pattern value 1", &perlin->pattern2Value1, 0.0f, 10.0f);
		ImGui::SliderFloat("pattern value 2", &perlin->pattern2Value2, 0.0f, 10.0f);
		ImGui::SliderFloat("pattern value 3", &perlin->pattern2Value3, 0.0f, 10.0f);
		ImGui::SliderFloat("pattern value 4", &perlin->pattern2Value4, 0.0f, 10.0f);
		ImGui::SliderFloat("pattern value 5", &perlin->pattern2Value5, 0.0f, 10.0f);
		ImGui::SliderFloat("pattern value 6", &perlin->pattern2Value6, 0.0f, 10.0f);
		ImGui::SliderFloat("pattern value 7", &perlin->pattern2Value7, 0.0f, 10.0f);
		ImGui::SliderFloat("pattern value 8", &perlin->pattern2Value8, 0.0f, 10.0f);
		ImGui::SliderFloat("pattern value 9", &perlin->pattern2Value9, 0.0f, 10.0f);
		ImGui::SliderFloat("pattern value 10", &perlin->pattern2Value10, 0.0f, 10.0f);
		ImGui::SliderFloat("pattern value 11", &perlin->pattern2Value11, 0.0f, 10.0f);
	}

	virtual void AddGUISpacer()
	{
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		auto perlinBlock = &bufferHandler.uniformBlocks["perlinSettings"];
		perlinBlock->SetPayload(perlinSettings_t());
		perlin = perlinBlock->GetPayload<perlinSettings_t>();
	}

	virtual void PerlinPass()
	{
		perlinBuffer->Bind();
		perlinBuffer->attachments["perlin"].Draw();

		defaultVertexBuffer.Bind();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		defProgram.Use();
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	virtual void FinalPass()
	{
		frameBuffer::Unbind();
		defaultVertexBuffer.Bind();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		perlinBuffer->attachments["perlin"].SetActive(0);
		finalProgram.Use();
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void Draw() override
	{
		PerlinPass();
		FinalPass();
	}
};
#endif
