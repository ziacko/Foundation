#ifndef HEAT_HAZE_H
#define HEAT_HAZE_H
#include "bubble.h"
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

	float			patternValue1;//0.3
	float			patternValue2;//0.8
	float			patternValue3;//5.2
	float			patternValue4;//1.3
	float			patternValue5;//4.0
	float			patternValue6;//1.7
	float			patternValue7;//9.2
	float			patternValue8;//4.0
	float			patternValue9;//8.3
	float			patternValue10;//2.8
	float			patternValue11;//4.0


	explicit perlinSettings_t(GLfloat modValue = 289.0f, GLfloat permuteValue = 29.33f,
	                          GLfloat taylorInverse = 1.79284291400159f, GLfloat fadeValue1 = 6.0f, GLfloat fadeValue2 = 15.0f,
	                          GLfloat fadeValue3 = 10.0f, GLuint numOctaves = 4,
	                          GLuint colorBias = 2,
	                          GLfloat noiseValue = 79.66f, GLfloat noiseValue2 = 5.6f,

	                          GLfloat patternValue1 = 2.5f, GLfloat patternValue2 = 0.4f, GLfloat patternValue3 = 5.2f,
	                          GLfloat patternValue4 = 1.3f, GLfloat patternValue5 = 4.0f, GLfloat patternValue6 = 1.7f,
	                          GLfloat patternValue7 = 9.2f, GLfloat patternValue8 = 4.0f, GLfloat patternValue9 = 8.3f,
	                          GLfloat patternValue10 = 2.8f, GLfloat	patternValue11 = 2.02f)
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

		this->patternValue1 = patternValue1;
		this->patternValue2 = patternValue2;
		this->patternValue3 = patternValue3;
		this->patternValue4 = patternValue4;
		this->patternValue5 = patternValue5;
		this->patternValue6 = patternValue6;
		this->patternValue7 = patternValue7;
		this->patternValue8 = patternValue8;
		this->patternValue9 = patternValue9;
		this->patternValue10 = patternValue10;
		this->patternValue11 = patternValue11;

		uvOffset = glm::vec2(0);
		uvScale = glm::vec2(1);
	};

	~perlinSettings_t() {};
};

class heatHazeScene : public bubbleScene
{
public:
	explicit heatHazeScene(
		texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's Portfolio ( heat haze )",		
		camera_t bubbleCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR, GLfloat attenuation = 1.0f,
		GLfloat offset = 1.0f) : bubbleScene(defaultTexture, windowName, bubbleCamera, shaderConfigPath)
	{

		//this->bubble = bubbleSettings;
		//enableWireframe = true;
		perlinBuffer = new frameBuffer();
		enableWireframe = false;
	}

	~heatHazeScene( void ) override {}

	virtual void Initialize() override
	{
		perlinBuffer->Initialize();
		perlinBuffer->Bind();

		FBODescriptor perlinDesc;
		perlinDesc.dataType = GL_FLOAT;
		perlinDesc.format = GL_RG;
		perlinDesc.internalFormat = GL_RG16F;
		perlinDesc.internalFormat = GL_RG16F;
		perlinDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		perlinBuffer->AddAttachment(frameBuffer::attachment_t("perlin", perlinDesc));

		frameBuffer::Unbind();

		defProgram = shaderProgramsMap["perlin"];
		heatHazeProgram = shaderProgramsMap["heat"];

		InitializeUniforms();
		SetupVertexBuffer();
	}

protected:

	void AddGUISpacer()
	{
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		if (ImGui::BeginTabItem("perlin noise"))
		{
			ImGui::Checkbox("enable wireframe", &enableWireframe);

			ImGui::SliderFloat("Attenuation", &bubble->attenuation, 0.0f, 1.0f);
			ImGui::SliderFloat("grid dimensions", &bubble->gridDimensions, 0.0f, 1000.0f, "%.0f");
			ImGui::SliderFloat("offset", &bubble->offset, 0.0f, 1.0f);

			AddGUISpacer();

			ImGui::SliderFloat2("UV offset", &perlin->uvOffset[0], 0.0f, 10.0f);
			ImGui::SliderFloat2("UV scale", &perlin->uvScale[0], 100.0f, 1.0f);
			ImGui::SliderFloat("modifier value", &perlin->modValue, 0.0f, 1000.0f);
			ImGui::SliderFloat("permutation value", &perlin->permuteValue, 0.0f, 100.0f);
			ImGui::SliderFloat("taylor inverse", &perlin->taylorInverse, 0.0f, 10.0f);

			AddGUISpacer();

			ImGui::SliderFloat("fade value 1", &perlin->fadeValue1, 0.0f, 100.0f);
			ImGui::SliderFloat("fade value 2", &perlin->fadeValue2, 0.0f, 100.0f);
			ImGui::SliderFloat("fade value 3", &perlin->fadeValue3, 0.0f, 100.0f);

			AddGUISpacer();

			ImGui::SliderInt("num octaves", &perlin->numOctaves, 0, 100);

			AddGUISpacer();

			ImGui::SliderInt("color bias", &perlin->colorBias, 0, 100);

			AddGUISpacer();
			ImGui::SliderFloat("pattern value 1", &perlin->patternValue1, 0.0f, 10.0f);
			ImGui::SliderFloat("pattern value 2", &perlin->patternValue2, 0.0f, 10.0f);
			ImGui::SliderFloat("pattern value 3", &perlin->patternValue3, 0.0f, 10.0f);
			ImGui::SliderFloat("pattern value 4", &perlin->patternValue4, 0.0f, 10.0f);
			ImGui::SliderFloat("pattern value 5", &perlin->patternValue5, 0.0f, 10.0f);
			ImGui::SliderFloat("pattern value 6", &perlin->patternValue6, 0.0f, 10.0f);
			ImGui::SliderFloat("pattern value 7", &perlin->patternValue7, 0.0f, 10.0f);
			ImGui::SliderFloat("pattern value 8", &perlin->patternValue8, 0.0f, 10.0f);
			ImGui::SliderFloat("pattern value 9", &perlin->patternValue9, 0.0f, 10.0f);
			ImGui::SliderFloat("pattern value 10", &perlin->patternValue10, 0.0f, 10.0f);
			ImGui::SliderFloat("pattern value 11", &perlin->patternValue11, 0.0f, 10.0f);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("framebuffers"))
		{
			ImGui::Image((ImTextureID)perlinBuffer->attachments["perlin"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::EndTabItem();
		}

	}

	frameBuffer*		perlinBuffer;
	perlinSettings_t*	perlin = nullptr;
	shaderProgram_t		heatHazeProgram;
	vertexBuffer_t		perlinVBuffer;

	void InitializeUniforms() override
	{
		bubbleScene::InitializeUniforms();

		auto perlinBlock = &bufferHandler.uniformBlocks["perlinSettings"];
		perlinBlock->SetPayload<perlinSettings_t>(perlinSettings_t());
		perlin = perlinBlock->GetPayload<perlinSettings_t>();
	}

	void SetupVertexBuffer() override
	{
		defaultVertexBuffer.SetupDefault();
		perlinVBuffer.SetupDefault();
	}

	void PerlinPass() const
	{
		perlinBuffer->Bind();
		perlinBuffer->attachments["perlin"].Draw();
		defProgram.Use();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		perlinBuffer->Unbind();
	}

	void HeatHazePass()
	{
		defaultTexture.GetUniformLocation(defProgram.handle);
		defaultTexture.SetActive(0);
		perlinBuffer->attachments["perlin"].SetActive(1);
		heatHazeProgram.Use();
		if (enableWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		glDrawArrays(GL_TRIANGLES, 0, 6);
		//glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)(bubble.data.gridDimensions * bubble.data.gridDimensions));
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	void Draw()	override
	{
		PerlinPass();
		HeatHazePass();
	}

	virtual void ClearBuffers() override
	{
		perlinBuffer->Bind();
		perlinBuffer->ClearTexture(perlinBuffer->attachments["perlin"], clearColor);
		glClear(GL_DEPTH_BUFFER_BIT);
		perlinBuffer->Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(const glm::ivec2& resolution)
	{
		perlinBuffer->Resize(glm::ivec3(resolution, 1));
	}

	virtual void HandleWindowResize(const tWindow* window, const TinyWindow::vec2_t<uint16_t>& dimensions) override
	{
		defaultPayload->resolution = glm::ivec2(dimensions.width, dimensions.height);
		ResizeBuffers(glm::ivec2(dimensions.x, dimensions.y));
		Resize(window, glm::ivec2(dimensions.x, dimensions.y));
	}

	virtual void HandleMaximize(const tWindow* window) override
	{
		defaultPayload->resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		ResizeBuffers(defaultPayload->resolution);
		Resize(window, defaultPayload->resolution);
	}

	virtual void Resize(const tWindow* window, glm::ivec2 dimensions = glm::ivec2(0)) override
	{
		scene::Resize(window, dimensions);

		//perlinVBuffer.UpdateBuffer(dimensions);
		ResizeBuffers(dimensions);
	}
};

#endif
