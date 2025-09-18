#pragma once
#include <scene.h>
#include <Texture.h>

class texturedScene : public scene
{
public:

	texturedScene(texture defaultTexture = texture("textures/earth_diffuse.tga"),
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

	virtual void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene::BuildGUI(window, io);
	}

	/*virtual void HandleFileDrop(const tWindow* window, const std::vector<std::string>& files, const vec2_t<int>& windowMousePosition)
	{
		//for each file that is dropped in
		//make sure it's a texture
		//and load up a new window for each one

		//first let's have it change the texture on display
		glFinish();
		defaultTexture->ReloadTexture(files[0].c_str());
	}*/

	virtual void Draw() override
	{
		PreDraw();

		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glUseProgram(this->programGLID);
		defaultTexture.SetActive(0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		PostDraw();

	}

	virtual void SetupCallbacks() override
	{
		scene::SetupCallbacks();
		manager->fileDropEvent = std::bind(&texturedScene::HandleFileDrop, this, _1, _2, _3);
	}

protected:

	texture								defaultTexture;
	std::vector<std::string>			textureDirs;
	bool								isGUIActive;
};
