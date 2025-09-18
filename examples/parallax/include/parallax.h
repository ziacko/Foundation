#ifndef PARALLAX_H
#define PARALLAX_H

#include <textured.h>

struct parallax_t
{
	float			scale;
	float			rayHeight;
	int				numSamples;

	parallax_t(GLfloat scale = 0.1f, GLfloat rayHeight = 0.25f, GLuint numSamples = 100)
	{
		this->scale = scale;
		this->rayHeight = rayHeight;
		this->numSamples = numSamples;
	}

	~parallax_t(){};
};

class parallaxScene : public texturedScene
{
public:

	parallaxScene(
		bufferHandler_t<parallax_t> parallaxSettings = bufferHandler_t<parallax_t>(),
		texture defaultTexture = texture("textures\rocks.jpg"),
		texture heightMap = texture("textures\rocks_NM_height.tga"),
		const char* windowName = "Ziyad Barakat's portfolio (parallax mapping)",
		camera_t parallaxCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		texturedScene(defaultTexture, windowName, parallaxCamera, shaderConfigPath)
	{
		this->parallax = parallaxSettings;
		this->heightMap = heightMap;
	}

	void Initialize() override
	{
		texturedScene::Initialize();
		heightMap.LoadTexture();
	}

	~parallaxScene(){};

protected:

	bufferHandler_t<parallax_t>		parallax;
	texture							heightMap;
	int								heightMapIndex = 0;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		//this one's gonna be trickier
		scene::BuildGUI(window, io);

		static int currentTextureIndex = 0;

		std::vector<const char*> tempTextureDirs;
		for (auto & textureDir : textureDirs)
		{
			tempTextureDirs.push_back(textureDir.c_str());
		}

		if (ImGui::ListBox("textures", &currentTextureIndex, tempTextureDirs.data(), tempTextureDirs.size()))
		{
			//delete defaultTexture; //remove the old one from memory
			defaultTexture = texture(tempTextureDirs[currentTextureIndex], texture::textureType_t::diffuse, "diffuseMap", textureDescriptor());
			defaultTexture.LoadTexture();
		}

		if (ImGui::ListBox("heightmap", &heightMapIndex, tempTextureDirs.data(), tempTextureDirs.size()))
		{
			//delete heightMap; //remove the old one from memory
			heightMap = texture(tempTextureDirs[heightMapIndex], texture::textureType_t::diffuse, "heightMap", textureDescriptor());
			heightMap.LoadTexture();
		}

		ImGui::SliderFloat("parallax scale", &parallax.data.scale, 0.f, 10.0f);
		ImGui::SliderFloat("ray height", &parallax.data.rayHeight, 0.0f, 10.0f);
		ImGui::SliderInt("num samples", &parallax.data.numSamples, 0, 1000);

	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		parallax.Initialize(1);
	}

	void SetupParallaxUniforms()
	{
		parallax.Initialize(1);
		glUniformBlockBinding(defProgram.handle, parallax.uniformHandle, 1);
	}

	void bindTextures()
	{
		defaultTexture.GetUniformLocation(defProgram.handle); //ok so heightmap is fine. just diffuse map is screwed
		heightMap.GetUniformLocation(defProgram.handle);
	}

	void Update() override
	{
		scene::Update();
		parallax.Update();
	}

	void Draw() override
	{
		
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glUseProgram(defProgram.handle);
		defaultTexture.GetUniformLocation(defProgram.handle);
		heightMap.GetUniformLocation(defProgram.handle);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		DrawGUI(window);
		manager->SwapDrawBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
};
#endif