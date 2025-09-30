#ifndef PARALLAX_H
#define PARALLAX_H

#include <textured.h>

struct parallax_t
{
	float			scale;
	float			rayHeight;
	int				numSamples;

	explicit parallax_t(const GLfloat scale = 0.1f, const GLfloat rayHeight = 0.25f, const GLuint numSamples = 100)
	{
		this->scale = scale;
		this->rayHeight = rayHeight;
		this->numSamples = numSamples;
	}

	~parallax_t() = default;
};

class parallaxScene final: public texturedScene
{
public:

	explicit parallaxScene(
		bufferHandler_t<parallax_t> parallaxSettings = bufferHandler_t<parallax_t>(),
		texture defaultTexture = texture("textures/rocks.jpg", texture::textureType_t::image, "diffuseMap"),
		texture heightMap = texture("textures/rocks_NM_height.tga", texture::textureType_t::image, "heightMap"),
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

	~parallaxScene() override = default;

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

	void Update() override
	{
		scene::Update();
		parallax.Update();
	}

	void Draw() override
	{
		defaultVertexBuffer.Bind();
		defProgram.Use();
		defaultTexture.GetUniformLocation(defProgram.handle);
		heightMap.GetUniformLocation(defProgram.handle);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
};
#endif