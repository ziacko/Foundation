#pragma once

#include <scene.h>
#include <cstdlib>
#include <gameOfLife.h>

class GOLCompute : public golScene
{
public:

	GOLCompute(const char* windowName = "Ziyad Barakat's portfolio (game of life (compute)",
		camera_t golCamera = camera_t(), const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: golScene(100.0f, 0.3, 666, 90, windowName, golCamera, shaderConfigPath)
	{

	}

	~GOLCompute(void) {};

	void Initialize() override
	{
		golScene::Initialize();
		defProgram =		shaderProgramsMap["GOL"];
		computeProgram =	shaderProgramsMap["GOLCompute"];
	}

protected:

	shaderProgram_t computeProgram;

	void Update() override
	{
		scene::Update();

		if (currentTickDelay < tickDelay)
		{
			currentTickDelay += clock.GetDeltaTime();
		}

		else
		{
			glUseProgram(computeProgram.handle);
			glDispatchCompute(25, 25, 1);
			currentTickDelay = 0;
		}
	}

	void InitializeUniforms() override
	{
		golScene::InitializeUniforms();
		cellBuffer.Update(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, sizeof(int) * cellBuffer.data.cells.size(), cellBuffer.data.cells.data());
	}
};
