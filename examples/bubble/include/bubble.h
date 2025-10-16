#pragma once
#include "textured.h"
//#include <DefaultUniformBuffer.h>

struct bubbleSettings_t
{
	float			attenuation;
	float			offset;
	float			gridDimensions;

	explicit bubbleSettings_t(GLfloat attenuation = 0.25f, GLfloat offset = 0.1f, GLfloat gridDimensions = 100)
	{
		this->attenuation = attenuation;
		this->offset = offset;
		this->gridDimensions = gridDimensions;
	}

	~bubbleSettings_t() = default;
};

class bubbleScene : public texturedScene
{
public:
	explicit bubbleScene(
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's Portfolio ( bubble displacement )",		
		const camera_t bubbleCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR, GLfloat attenuation = 1.0f,
		GLfloat offset = 1.0f) : texturedScene(defaultTexture, windowName, bubbleCamera, shaderConfigPath)
	{
		enableWireframe = true;
		texturedScene::Initialize();
	}

	void Initialize() override
	{
		texturedScene::Initialize();
		defProgram = shaderProgramsMap["bubble"];
	}

	~bubbleScene( void ) override {}

protected:

	void BuildGUI(tWindow* window, const ImGuiIO &io) override
	{
		texturedScene::BuildGUI(window, io);
		if (ImGui::BeginTabItem("bubble"))
		{
			ImGui::Checkbox("enable wireframe", &enableWireframe);

			ImGui::SliderFloat("Attenuation", &bubble->attenuation, 0.0f, 1.0f);
			ImGui::SliderFloat("grid dimensions", &bubble->gridDimensions, 0.0f, 1000.0f, "%.0f");
			ImGui::SliderFloat("offset", &bubble->offset, 0.0f, 1.0f);
			ImGui::EndTabItem();
		}
	}

	bubbleSettings_t*			bubble;
	std::vector<glm::vec4>		gridVerts = {};
	int							gridDimensions = 0;
	bool						enableWireframe = false;

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		const auto bubbleBlock = &bufferHandler.uniformBlocks["bubbleSettings"];
		bubbleBlock->SetPayload<bubbleSettings_t>(bubbleSettings_t());
		bubble = bubbleBlock->GetPayload<bubbleSettings_t>();
	}

	virtual void SetupVertexBuffer()
	{ 
		GLfloat cellWidth = defaultPayload->resolution.x / bubble->gridDimensions;
		GLfloat cellHeight = defaultPayload->resolution.y / bubble->gridDimensions;

		defaultVertexBuffer.SetupCustom(glm::vec2(cellWidth, cellHeight));
	}

	void Draw()	override
	{
		GL_PUSH_DEBUG_GROUP();

		defaultTexture.GetUniformLocation(defProgram.handle);
		glUseProgram(defProgram.handle);
		if (enableWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)(bubble->gridDimensions * bubble->gridDimensions));
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glPopDebugGroup();
	}

	void HandleWindowResize(const tWindow* inWindow, const vec2_t<uint16_t>& dimensions) override
	{
		texturedScene::HandleWindowResize(inWindow, dimensions);
		camera.resolution = glm::ivec2(dimensions.x, dimensions.y);
		camera.Update();
	}

	void HandleMaximize(const tWindow* window) override
	{
		texturedScene::HandleMaximize(window);
		auto newRes = window->GetSettings().resolution;
		camera.resolution = glm::ivec2(newRes.width, newRes.height);
		camera.Update();
	}
};
