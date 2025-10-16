#ifndef GAUSSIAN_BLURRING_H
#define GAUSSIAN_BLURRING_H
#include <textured.h>
#include "FrameBuffer.h"

struct gaussianSettings_t
{
	int		offsets[5];
	float	weights[5];


	gaussianSettings_t()
	{
		for (size_t iter = 0; iter < 5; iter++)
		{
			offsets[iter] = iter;
		}

		weights[0] = 0.2270270270;
		weights[1] = 0.1945945946;
		weights[2] = 0.1216216216;
		weights[3] = 0.0540540541;
		weights[4] = 0.0162162162;
	}
	;

	~gaussianSettings_t() = default;
};

class gaussianMultiScene final : public texturedScene
{
public:

	explicit gaussianMultiScene(const texture defaultTexture = texture(),
		const char* windowName = "Ziyad Barakat's Portfolio (gaussian blurring)",
		const camera_t textureCamera = camera_t(),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		texturedScene(defaultTexture, windowName, textureCamera, shaderConfigPath)
	{
		gaussBuffer = new frameBuffer();
		compareBuffer = new frameBuffer();
	}

	void Initialize() override
	{
		texturedScene::Initialize();
		
		verticalProgram = shaderProgramsMap["gaussianVert"];
		horizontalProgram = shaderProgramsMap["GaussianHorz"];
		finalProgam = shaderProgramsMap["blend"];
		compareProgram = shaderProgramsMap["compare"];
		
		gaussBuffer->Initialize();
		gaussBuffer->Bind();

		FBODescriptor gaussDesc;
		auto localRes = window->GetSettings().resolution;
		gaussDesc.dimensions = glm::ivec3(localRes.width, localRes.height, 1);

		//add 2 render textures, one for the first pass and one for the second?
		//or just one and keep passing it through
		gaussBuffer->AddAttachment(frameBuffer::attachment_t("vertical", gaussDesc));
		gaussBuffer->AddAttachment(frameBuffer::attachment_t("horizontal", gaussDesc));

		compareBuffer->Initialize();
		compareBuffer->Bind();

		compareBuffer->AddAttachment(frameBuffer::attachment_t("compare", gaussDesc));

		frameBuffer::Unbind();
	}

	void VerticalPass() const
	{
		gaussBuffer->Bind();
		gaussBuffer->attachments["vertical"].Draw();

		defaultTexture.SetActive(0);

		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultVertexBuffer.Bind();
		verticalProgram.Use();
		
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void HorizontalPass() const
	{
		gaussBuffer->Bind();
		gaussBuffer->attachments["horizontal"].Draw();

		defaultTexture.SetActive(0);

		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultVertexBuffer.Bind();
		horizontalProgram.Use();
		
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void BlendPass() const
	{
		//draw to backbuffer
		compareBuffer->Bind();
		compareBuffer->attachments["compare"].Draw();

		defaultVertexBuffer.Bind();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		gaussBuffer->attachments["vertical"].SetActive(0);
		gaussBuffer->attachments["horizontal"].SetActive(1);

		finalProgam.Use();

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	virtual void FinalPass(texture* tex1, frameBuffer::attachment_t* tex2) const
	{
		//draw directly to backbuffer
		frameBuffer::Unbind();
		defaultVertexBuffer.Bind();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		tex1->SetActive(0);
		tex2->SetActive(1);

		compareProgram.Use();

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	virtual void Draw() override
	{
		//ok first do the vertical pass
		//then the horizontal pass
		//then draw the final image?
		VerticalPass();

		HorizontalPass();

		BlendPass();

		FinalPass(&defaultTexture, &compareBuffer->attachments["compare"]);
	}

	~gaussianMultiScene() override {};

protected:

	gaussianSettings_t*		gaussianHorz = nullptr;
	gaussianSettings_t*		gaussianVert = nullptr;
	
	frameBuffer* gaussBuffer;
	frameBuffer* compareBuffer;

	shaderProgram_t						verticalProgram;
	shaderProgram_t						horizontalProgram;
	shaderProgram_t						finalProgam;
	shaderProgram_t						compareProgram;

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		texturedScene::BuildGUI(window, io);

		//if (ImGui::BeginTabItem("gaussian multi pass"))
		{
			if (ImGui::BeginTabItem("horizontal"))
			{
				for (size_t iter = 0; iter < 5; iter++)
				{
					std::string num = std::to_string(iter);
					ImGui::DragInt(std::string("offset# " + num).c_str(),
						&gaussianHorz->offsets[iter], 1.0f, 0.0f, 10.0f);
				}
				ImGui::Separator();
				for (size_t iter = 0; iter < 5; iter++)
				{
					std::string num = std::to_string(iter);
					ImGui::DragFloat(std::string("weight# " + num).c_str(),
						&gaussianHorz->weights[iter], 0.001f, 0.0f, 1.0f);
				}
				//ImGui::EndTabBar();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("vertical"))
			{
				for (size_t iter = 0; iter < 5; iter++)
				{
					std::string num = std::to_string(iter);
					ImGui::DragInt(std::string("offset# " + num).c_str(),
						&gaussianVert->offsets[iter], 1.0f, 0.0f, 10.0f);
				}
				ImGui::Separator();
				for (size_t iter = 0; iter < 5; iter++)
				{
					std::string num = std::to_string(iter);
					ImGui::DragFloat(std::string("weight# " + num).c_str(),
						&gaussianVert->weights[iter], 0.001f, 0.0f, 1.0f);
				}

				ImGui::EndTabItem();
			}
		}

	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();

		auto gaussianHorzBlock = &bufferHandler.uniformBlocks["horzSettings"];
		gaussianHorzBlock->SetPayload<gaussianSettings_t>(gaussianSettings_t());
		gaussianHorz = gaussianHorzBlock->GetPayload<gaussianSettings_t>();

		auto gaussianVertBlock = &bufferHandler.uniformBlocks["vertSettings"];
		gaussianVertBlock->SetPayload<gaussianSettings_t>(gaussianSettings_t());
		gaussianVert = gaussianVertBlock->GetPayload<gaussianSettings_t>();
	}

	void ClearBuffers() const
	{
		gaussBuffer->Bind();
		gaussBuffer->ClearTexture(gaussBuffer->attachments["vertical"], clearColor);
		gaussBuffer->ClearTexture(gaussBuffer->attachments["horizontal"], clearColor);
		glClear(GL_DEPTH_BUFFER_BIT);
		frameBuffer::Unbind();
	}

	virtual void HandleWindowResize(const tWindow* window, const tw::vec2_t<uint16_t>& dimensions) override
	{
		defaultPayload->resolution = glm::ivec2(dimensions.width, dimensions.height);
		ResizeBuffers(glm::ivec2(dimensions.x, dimensions.y));
		Resize(window, glm::ivec2(dimensions.x, dimensions.y));
	}

	virtual void HandleMaximize(const tWindow* window) override
	{
		defaultPayload->resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		ResizeBuffers(defaultPayload->resolution);
		Resize(window, defaultPayload->resolution);
	}

	virtual void ResizeBuffers(const glm::ivec2 resolution) const
	{
		gaussBuffer->Resize(glm::ivec3(resolution, 1));
		compareBuffer->Resize(glm::ivec3(resolution, 1));
	}

	virtual void Resize(const tWindow* window, glm::ivec2 dimensions  = glm::ivec2(0)) override
	{
		scene::Resize(window, dimensions);
		ResizeBuffers(dimensions);
	}
};

#endif