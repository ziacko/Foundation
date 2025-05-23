#ifndef BINDLESS_H
#define BINDLESS_H
#include <textured.h>

class Bindless : public texturedScene
{
public:

	Bindless(
		const char* windowName = "Ziyad Barakat's portfolio (Bindless texture)",
		texture* defaultTexture = new texture(),
		camera_t* cam = new camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: texturedScene(defaultTexture, windowName, cam, shaderConfigPath)
	{
		bindless = bufferHandler_t<GLuint>(0);
	}

	~Bindless(){};

	virtual void Initialize() override
	{
		texturedScene::Initialize();		
		defaultTexture->ToggleResident();
		programGLID = shaderProgramsMap["bindlessProgram"].handle;
	}

	virtual void Draw() override
	{
		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glUseProgram(this->programGLID);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		DrawGUI(window);
		manager->SwapDrawBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

protected:

	bufferHandler_t<GLuint>		bindless;

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		bindless.Initialize(1);
	}

	void Update() override
	{
		scene::Update();
		bindless.data = defaultTexture->GetResidentHandle();
		bindless.Update(gl_uniform_buffer, gl_dynamic_draw);
	}
};

#endif