#pragma once

#include <scene.h>
#include <cstdlib>
#include <gameOfLife.h>

class GOLCompute : public golScene
{
public:

	GOLCompute(const char* windowName = "Ziyad Barakat's portfolio (game of life (compute)",
		camera_t* golCamera = new camera_t(), const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: golScene(100.0f, 0.3, 666, 90, windowName, golCamera, shaderConfigPath)
	{

	}

	~GOLCompute(void) {};

	void Initialize() override
	{
		golScene::Initialize();
		programGLID =		shaderProgramsMap["GOL"].handle;
		computeProgram =	shaderProgramsMap["GOLCompute"].handle;
	}

protected:

	unsigned int computeProgram = 0;

	void Update() override
	{
		scene::Update();

		if (currentTickDelay < tickDelay)
		{
			currentTickDelay += clock->GetDeltaTime();
		}

		else
		{
			glUseProgram(computeProgram);
			glDispatchCompute(25, 25, 1);
			currentTickDelay = 0;
		}
	}

	void InitializeUniforms() override
	{
		golScene::InitializeUniforms();
		cellBuffer.Update(gl_shader_storage_buffer, gl_dynamic_draw, sizeof(int) * cellBuffer.data.cells.size(), cellBuffer.data.cells.data());
	}
};
