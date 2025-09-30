#pragma once
#include <scene.h>
#include <Texture.h>

class texturedScene : public scene
{
public:

	explicit texturedScene(texture defaultTexture = texture("textures/earth_diffuse.tga"),
		const char* windowName = "Ziyad Barakat's Portfolio (textured scene)",
		camera_t textureCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		scene(windowName, textureCamera, shaderConfigPath)
	{
		this->defaultTexture = defaultTexture;
		isGUIActive = false;
	}

	virtual void Initialize() override
	{
		scene::Initialize();
		defaultTexture.LoadTexture();
	}

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene::BuildGUI(window, io);
	}

	virtual void Draw() override
	{
		GL_PUSH_DEBUG_GROUP();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultVertexBuffer.Bind();
		defProgram.Use();
		defaultTexture.SetActive(0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glPopDebugGroup();
	}

	virtual void SetupCallbacks() override
	{
		scene::SetupCallbacks();
		manager->fileDropEvent = std::bind(&texturedScene::HandleFileDrop, this, _1, _2);
	}

protected:

	texture								defaultTexture;
	std::vector<std::string>			textureDirs;
	bool								isGUIActive;
};
