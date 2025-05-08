#ifndef WDIVIDE_H
#define WDIVIDE_H
#include "Scene3D.h"

struct divideTest_t final
{
	GLfloat tester;

	divideTest_t()
	{
		tester = 1.0f;
	}
};

//the 4th dimension. a W of 1 means the vector is a position, a W of 0 means the vector is a direction


class WDivide : public scene3D
{
public:
	WDivide(
		const char* windowName = "Ziyad Barakat's portfolio (early depth test)",
		camera* cam = new camera(glm::vec2(1280, 720), 1.0f, camera::projection_t::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = "../../resources/shaders/WDivide.txt",
		model_t* model = new model_t("../../resources/models/SoulSpear/SoulSpear.fbx"))
		: scene3D(windowName, cam, shaderConfigPath, model)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(gl_generate_mipmap_hint, GL_NICEST);
	}

	~WDivide() {};

	virtual void Initialize() override
	{
		scene3D::Initialize();
	}

protected:

	bufferHandler_t<divideTest_t> testBuffer;

	void InitializeUniforms() override
	{
		scene3D::InitializeUniforms();
		testBuffer.Initialize(2);
	}

	void Update() override
	{
		scene3D::Update();
		testBuffer.Update();
	}

	virtual void DrawDivideStats()
	{
		ImGui::Begin("W Divide test");
		ImGui::DragFloat("W mod", &testBuffer.data.tester, 0.01f, 0.001f, 10.0f, "%.3f");
		ImGui::End();
	}

	virtual void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene3D::BuildGUI(window, io);
		DrawDivideStats();
	}
};

#endif // WDIVIDE_H_
