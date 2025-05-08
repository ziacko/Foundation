#include "TexturedScene.h"
#include <GL/gl.h>
#include <vector>
#include "CommandBuffer.h"

class commandScene : public texturedScene
{
public:

	commandScene(
		texture* defaultTexture = new texture(),
		const char* windowName = "Ziyad Barakat's Portfolio ( command buffers )",		
		camera_t* camera = new camera_t(), 
		const char* shaderConfigPath = "../../resources/shaders/CommandBuffers/CommandBuffers.txt") 
		: texturedScene(defaultTexture, windowName, camera, shaderConfigPath)
	{
		enableWireframe = true;
		Initialize();
		testStream.Init(5);

	}

	~commandScene(){}

protected:

    command::bufferStream testStream;

	void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		texturedScene::BuildGUI(window, io);

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
		defaultTexture->SetActive(0);
		testStream.BindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		testStream.UseProgram(this->programGLID);
		testStream.DrawArrays(GL_TRIANGLES, 0, 6);
		testStream.Execute();

		DrawGUI(windows[0]);
		windows[0]->SwapDrawBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //keep seperate from stream?
	}
};
