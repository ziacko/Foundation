#ifndef PERLINNOISE3D_H
#define PERLINNOISE3D_H
#include <perlin.h>
#include "FrameBuffer.h"

struct perlinSettings3D_t
{
	glm::vec3	uvwScale;
	int			layer;

	explicit perlinSettings3D_t(const glm::vec3 uvwScale = glm::vec3(1, 1, 0.01))
	{
		this->uvwScale = uvwScale;
		layer = 0;
	}

	~perlinSettings3D_t() = default;
};

class perlinScene3D final : public perlinScene
{
public:
	explicit perlinScene3D(const char* windowName = "Ziyad Barakat's Portfolio ( Perlin3D noise )",
	                       const camera_t perlinCamera = camera_t(),
	                       const GLchar* shaderConfigPath = SHADER_CONFIG_DIR)
		: perlinScene(windowName, perlinCamera, shaderConfigPath), perlinTex(nullptr)
	{
		perlin.data = perlinSettings_t();
		perlin3D.data = perlinSettings3D_t();
	}

	void Initialize() override
	{
		scene::Initialize();

		FBODescriptor perlinDesc;
		perlinDesc.target = GL_TEXTURE_3D;
		perlinDesc.dataType = GL_FLOAT;
		perlinDesc.format = GL_RED;
		perlinDesc.internalFormat = GL_R16;
		perlinDesc.dimensions = glm::ivec3(50);

		//don't make this a rendertarget. just regular texture
		perlinTex = new frameBuffer::attachment_t("perlin", perlinDesc);

		perlinProgram = shaderProgramsMap["Perlin3D"];
		defProgram = shaderProgramsMap["final"];

		scene::InitializeUniforms();
		perlin3D.Initialize(1);
	}

protected:

	shaderProgram_t							perlinProgram;
	frameBuffer::attachment_t*				perlinTex;
	bufferHandler_t<perlinSettings3D_t>		perlin3D;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene::BuildGUI(window, io); 
		ImGui::SliderFloat3("uvw Scale", &perlin3D.data.uvwScale[0], 0.01f, 100);
		ImGui::SliderInt("layer", &perlin3D.data.layer, 0, 50);
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		perlin3D.Initialize(1);
		perlin.Initialize(2);
	}

	void SetPerlinUniforms() override
	{
		perlin3D.SetupUniforms(defProgram.handle, "perlin3DSettings", 1);
		perlin3D.SetupUniforms(defProgram.handle, "perlinSettings", 2);
	}

	void Update() override
	{
		scene::Update();
		perlin3D.Update();
		perlin.Update();
	}

	void PerlinCalc() const
	{
		perlinTex->BindAsImage(0);
		perlinProgram.Use();
		glDispatchCompute(2, 2, 2);
	}

	void FinalPass() override
	{
		perlinTex->SetActive(0);
		defProgram.Use();
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void Draw()	override
	{
		PerlinCalc();
		FinalPass();
	}
};
#endif
