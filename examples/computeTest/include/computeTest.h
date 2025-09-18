#ifndef COMPUTE_TEST_H
#define COMPUTE_TEST_H
#include "scene.h"

struct dispatchStruct
{
	std::vector<float> dispatchArray;

	dispatchStruct(uint16_t size = 10)
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
		camera_t cam = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: scene(windowName, cam, shaderConfigPath)
	{
		disp = bufferHandler_t<dispatchStruct>();
		loadFromBuffer = false;
	}

	~computeTestScene() {};

	shaderProgram_t computeProgram;

	bufferHandler_t<dispatchStruct> disp;

	bool loadFromBuffer;

	virtual void Initialize() override
	{
		scene::Initialize();
		defProgram = shaderProgramsMap["scene"];
		computeProgram = shaderProgramsMap[PROJECT_NAME];
	}

	virtual void Draw() override
	{
		scene::Draw();
	}

protected:

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		disp.Initialize(0, GL_SHADER_STORAGE_BUFFER);
	}

	void Update() override
	{
		scene::Update();

		if(loadFromBuffer)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, disp.bufferHandle);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, disp.data.dispatchArray.size(), disp.data.dispatchArray.data());
			for(size_t iter = 0; iter < 10; iter++)
			{
				printf("%f \t", disp.data.dispatchArray[0]);
			}
			printf("\n");
			loadFromBuffer = false;
		}
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene::BuildGUI(window, io);

		if (ImGui::BeginTabItem("compute dispatcher"))
		{
			if (ImGui::Button("dispatch"))
			{
				glUseProgram(computeProgram.handle);
				glDispatchCompute(10, 1, 1);
				loadFromBuffer = true;
			}

			ImGui::EndTabItem();
		}

	}
};

#endif