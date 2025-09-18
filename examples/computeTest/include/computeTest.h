#ifndef COMPUTE_TEST_H
#define COMPUTE_TEST_H
#include "scene.h"

struct dispatchStruct
{
	std::vector<float> dispatchArray;

	dispatchStruct(int size = 10)
	{
		for (size_t iter = 0; iter < size; iter++)
		{
			dispatchArray.push_back(0.0f);
		}
	}
};

class computeTestScene : public scene
{
public:

	computeTestScene(
		const char* windowName = "Ziyad Barakat's portfolio (compute shader test)",
		camera_t* cam = new camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: scene(windowName, cam, shaderConfigPath)
	{
		disp = bufferHandler_t<dispatchStruct>();
		loadFromBuffer = false;
	}

	~computeTestScene() {};

	unsigned int computeProgram = 0;

	bufferHandler_t<dispatchStruct> disp;

	bool loadFromBuffer;

	virtual void Initialize() override
	{
		scene::Initialize();
		programGLID = shaderProgramsMap["scene"].handle;
		computeProgram = shaderProgramsMap[PROJECT_NAME].handle;
		printf("%i \n", computeProgram);
	}

	virtual void Draw() override
	{
		scene::Draw();
	}

protected:

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		disp.Initialize(0, gl_shader_storage_buffer);
	}

	void Update() override
	{
		scene::Update();

		if(loadFromBuffer)
		{
			glBindBuffer(gl_shader_storage_buffer, disp.bufferHandle);
			glGetBufferSubData(gl_shader_storage_buffer, 0, disp.data.dispatchArray.size(), disp.data.dispatchArray.data());
			for(size_t iter = 0; iter < 10; iter++)
			{
				printf("%f \t", disp.data.dispatchArray[0]);
			}
			printf("\n");
			loadFromBuffer = false;
		}
	}

	void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene::BuildGUI(window, io);

		ImGui::Begin("compute dispatcher");
		if(ImGui::Button("dispatch"))
		{
			glUseProgram(computeProgram);
			glDispatchCompute(10, 1, 1);
			loadFromBuffer = true;
		}
		ImGui::End();
	}
};

#endif