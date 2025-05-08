#pragma once
#include "Scene3D.h"
#include "FeedbackBuffer.h"

class feedbackScene : public scene3D
{
public:

	feedbackScene(
		const char* windowName = "Ziyad Barakat's portfolio (feedback buffers)",
		camera_t* texModelCamera = new camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_t::perspective),
		const char* shaderConfigPath = "./shaders/FeedbackBuffers.txt",
		model_t* model = new model_t("../../resources/models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.fbx"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{

	}

	~feedbackScene(){}

protected:

	void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene3D::BuildGUI(window, io);

	}

	bool enableWireframe = false;

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
	}

	void Update() override
	{
		scene::Update();
	}

	void Draw()	override
	{
		//defaultTexture->SetActive(0);
		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glUseProgram(this->programGLID);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		DrawGUI(windows[0]);
		windows[0]->SwapDrawBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //keep seperate from stream?
	}
};
