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
		const bufferHandler_t<bubbleSettings_t> bubbleSettings = bufferHandler_t<bubbleSettings_t>(),
		const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's Portfolio ( bubble displacement )",		
		const camera_t bubbleCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR, GLfloat attenuation = 1.0f,
		GLfloat offset = 1.0f) : texturedScene(defaultTexture, windowName, bubbleCamera, shaderConfigPath)
	{
		this->bubble = bubbleSettings;
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

			ImGui::SliderFloat("Attenuation", &bubble.data.attenuation, 0.0f, 1.0f);
			ImGui::SliderFloat("grid dimensions", &bubble.data.gridDimensions, 0.0f, 1000.0f, "%.0f");
			ImGui::SliderFloat("offset", &bubble.data.offset, 0.0f, 1.0f);
			ImGui::EndTabItem();
		}
	}

	bufferHandler_t<bubbleSettings_t>			bubble;
	std::vector<glm::vec4>						gridVerts = {};
	int											gridDimensions = 0;
	bool										enableWireframe = false;

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		bubble.Initialize(1);
	}

	virtual void SetupVertexBuffer()
	{ 
		GLfloat cellWidth = defaultPayload.data.resolution.x / bubble.data.gridDimensions;
		GLfloat cellHeight = defaultPayload.data.resolution.y / bubble.data.gridDimensions;

		defaultVertexBuffer.SetupCustom(glm::vec2(cellWidth, cellHeight));
	}

	void Update() override
	{
		scene::Update();
		bubble.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
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

		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)(bubble.data.gridDimensions * bubble.data.gridDimensions));
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
