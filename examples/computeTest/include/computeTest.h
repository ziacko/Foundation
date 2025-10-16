#ifndef COMPUTE_TEST_H
#define COMPUTE_TEST_H
#include "scene.h"

struct dispatchStruct
{
	glm::vec4 dispatchArray;

	explicit dispatchStruct(const float defValue = 10.0f)
	{
		dispatchArray = glm::vec4(defValue);
	}
};

class computeTestScene final : public scene
{
public:

	explicit  computeTestScene(
		const char* windowName = "Ziyad Barakat's portfolio (compute shader test)",
		camera_t camera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: scene(windowName, camera, shaderConfigPath)
	{
		loadFromBuffer = false;
	}
	

	shaderProgram_t computeProgram;

	dispatchStruct* disp = nullptr;

	bool loadFromBuffer;
	std::string inputBuffer;

protected:

	void InitializeUniforms() override
	{
		defProgram = shaderProgramsMap["scene"];
		computeProgram = shaderProgramsMap[PROJECT_NAME];
		//scene::InitializeUniforms();
		auto dispBlock = &bufferHandler.shaderStorageBlocks["testBuffer"];
		// Set initial payload and ensure the SSBO is created and bound to binding=0 as in the shader
		dispBlock->SetPayload<dispatchStruct>(dispatchStruct());
		dispBlock->Initialize(GL_DYNAMIC_DRAW);
		//dispBlock->BindToSlot(0);
		disp = dispBlock->GetPayload<dispatchStruct>();
	}

	void Draw() override
	{
		//keep this empty
	}

	void Update() override
	{
		scene::Update();
		bufferHandler.shaderStorageBlocks["testBuffer"].Read(&disp->dispatchArray, sizeof(dispatchStruct));
		if(loadFromBuffer)
		{
			for(size_t iter = 0; iter < 4; iter++)
			{
				inputBuffer.append(std::to_string(disp->dispatchArray[iter]));
				inputBuffer.append(" ");
			}			
			loadFromBuffer = false;
			inputBuffer.append("\n");
		}
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene::BuildGUI(window, io);

		if (ImGui::BeginTabItem("compute dispatcher"))
		{
			if (ImGui::Button("dispatch"))
			{
				computeProgram.Use();
				// Ensure SSBO is bound to the expected slot in case other passes changed it
				bufferHandler.shaderStorageBlocks["testBuffer"].BindToSlot(0);
				glDispatchCompute(1, 1, 1);
				loadFromBuffer = true;
			}

			ImGui::Text("Array Input \n %s", inputBuffer.c_str());
			ImGui::EndTabItem();
		}
	}
};

#endif