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
		disp = bufferHandler_t<dispatchStruct>();
		loadFromBuffer = false;
	}
	

	shaderProgram_t computeProgram;

	bufferHandler_t<dispatchStruct> disp;

	bool loadFromBuffer;
	std::string inputBuffer;

	virtual void Initialize() override
	{
		scene::Initialize();
	}

protected:

	void InitializeUniforms() override
	{
		defProgram = shaderProgramsMap["scene"];
		computeProgram = shaderProgramsMap[PROJECT_NAME];
		scene::InitializeUniforms();
		disp.Initialize(0, GL_SHADER_STORAGE_BUFFER);
	}

	void Update() override
	{
		scene::Update();

		if(loadFromBuffer)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, disp.bufferHandle);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,  sizeof(disp.data.dispatchArray), &disp.data.dispatchArray);
			for(size_t iter = 0; iter < 4; iter++)
			{
				inputBuffer.append(std::to_string(disp.data.dispatchArray[iter]));
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
				glDispatchCompute(4, 1, 1);
				loadFromBuffer = true;
			}

			ImGui::Text("Array Input \n %s", inputBuffer.c_str());
			ImGui::EndTabItem();
		}
	}
};

#endif