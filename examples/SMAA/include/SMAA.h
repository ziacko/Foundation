#pragma once

#include "scene3D.h"
#include "FrameBuffer.h"

enum class EdgeDetectionMode_e
{
	luma = 0,
	color = 1,
	depth = 2
};

struct SMAASettings_t
{
	glm::vec4	rtMetrics = glm::vec4(1.0 / defaultWindowSize.x, 1.0 / defaultWindowSize.y, defaultWindowSize.x, defaultWindowSize.y);
	float		threshold;
	float		contrastAdaptationFactor;

	int32_t		maxSearchSteps;
	int32_t		maxSearchStepsDiag;
	int32_t		cornerRounding;
	int32_t		edgeDetectionMode;

	explicit SMAASettings_t(const glm::ivec2& resolution = defaultWindowSize, const float threshold = 0.05, const float CAFactor = 2.0f,
		const uint8_t maxSearchSteps = 32, const uint8_t maxSearchStepsDiag = 16, const uint8_t cornerRounding = 25,
		const EdgeDetectionMode_e& edgeDetectionMode = EdgeDetectionMode_e::color)
	{
		this->rtMetrics = glm::vec4(1.0 / resolution.x, 1.0 / resolution.y, resolution.x, resolution.y);
		this->threshold = threshold;
		this->contrastAdaptationFactor = CAFactor;
		this->maxSearchSteps = maxSearchSteps;
		this->maxSearchStepsDiag = maxSearchStepsDiag;
		this->cornerRounding = cornerRounding;
		this->edgeDetectionMode = (int32_t)edgeDetectionMode;
	}
};

class SMAAScene : public scene3D
{
public:

	explicit SMAAScene(
		const char* windowName = "Ziyad Barakat's portfolio (SMAA)",
		const camera_t& camera = camera_t(defaultWindowSize, defaultCameraSpeed, camera_t::projection_e::perspective),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		const model_t& model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: scene3D(windowName, camera, shaderConfigPath, model)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		this->camera.position.y -= 100.0f;

		//soulspear is loaded at an awkward angle so let's hack this
		//this->camera.Roll(glm::radians(270.0f));
		//this->camera.Pitch(glm::radians(180.0f));

		geometryBuffer = frameBuffer();
		edgesBuffer = frameBuffer();
		weightsBuffer = frameBuffer();
		SMAABuffer = frameBuffer();

		SMAAArea = texture("textures/SMAA/AreaTexDX_Flipped.png");
		SMAASearch = texture("textures/SMAA/SearchTex_Flipped.png");
	}

	~SMAAScene() override = default;

	void Initialize() override
	{
		scene3D::Initialize();

		SMAAArea.LoadTexture();
		SMAASearch.LoadTexture();

		SMAASearch.SetMagFilter(GL_NEAREST);
		SMAASearch.SetMinFilter(GL_NEAREST);

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);
		colorDesc.dataType = GL_FLOAT;
		colorDesc.format = GL_RGBA;
		colorDesc.internalFormat = GL_RGBA32F;
		colorDesc.wrapRSetting = GL_CLAMP_TO_EDGE;
		colorDesc.wrapTSetting = GL_CLAMP_TO_EDGE;
		colorDesc.wrapSSetting = GL_CLAMP_TO_EDGE;

		FBODescriptor depthDesc;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.wrapRSetting = GL_CLAMP_TO_EDGE;
		depthDesc.wrapTSetting = GL_CLAMP_TO_EDGE;
		depthDesc.wrapSSetting = GL_CLAMP_TO_EDGE;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT32F;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer.Initialize();
		geometryBuffer.Bind();

		geometryBuffer.AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer.AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		edgesBuffer.Initialize();
		edgesBuffer.Bind();

		FBODescriptor edgeDesc;
		edgeDesc.format = GL_RG;
		edgeDesc.dataType = GL_FLOAT;
		edgeDesc.internalFormat = GL_RG32F;
		edgeDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);
		edgeDesc.wrapRSetting = GL_CLAMP_TO_EDGE;
		edgeDesc.wrapTSetting = GL_CLAMP_TO_EDGE;
		edgeDesc.wrapSSetting = GL_CLAMP_TO_EDGE;

		edgesBuffer.AddAttachment(frameBuffer::attachment_t("edge", edgeDesc));

		weightsBuffer.Initialize();
		weightsBuffer.Bind();

		FBODescriptor weightsDesc;
		weightsDesc = colorDesc;
		weightsDesc.dataType = GL_FLOAT;
		weightsDesc.internalFormat = GL_RGBA32F;
		weightsDesc.wrapRSetting = GL_CLAMP_TO_EDGE;
		weightsDesc.wrapTSetting = GL_CLAMP_TO_EDGE;
		weightsDesc.wrapSSetting = GL_CLAMP_TO_EDGE;

		weightsBuffer.AddAttachment(frameBuffer::attachment_t("blend", weightsDesc));

		SMAABuffer.Initialize();
		SMAABuffer.Bind();
		SMAABuffer.AddAttachment(frameBuffer::attachment_t("SMAA", colorDesc));

		geometryProgram = shaderProgramsMap["geometry"];
		edgeDetectionProgram = shaderProgramsMap["edgeDetection"];
		blendingWeightProgram = shaderProgramsMap["blendingWeight"];
		SMAAProgram = shaderProgramsMap["SMAA"];
		compareProgram = shaderProgramsMap["compare"];
		finalProgram = shaderProgramsMap["final"];

		frameBuffer::Unbind();
	}

protected:

	frameBuffer					geometryBuffer;
	frameBuffer					edgesBuffer;
	frameBuffer					weightsBuffer;
	frameBuffer					SMAABuffer;

	texture						SMAAArea;
	texture						SMAASearch;

	SMAASettings_t*	SMAASettings = nullptr;

	shaderProgram_t geometryProgram;
	shaderProgram_t edgeDetectionProgram;
	shaderProgram_t blendingWeightProgram;
	shaderProgram_t SMAAProgram;
	shaderProgram_t compareProgram;
	shaderProgram_t finalProgram;

	int currentTexture = 0;
	bool enableCompare = true;

	void Draw() override
	{
		GL_PUSH_DEBUG_GROUP();
		//enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GeometryPass(); //render current scene with jitter
		glDisable(GL_BLEND);
		EdgeDetectionPass();
		BlendingWeightsPass();
		SMAAPass();

		FinalPass(&SMAABuffer.attachments["SMAA"], &geometryBuffer.attachments["color"]);
		glPopDebugGroup();
	}

	virtual void GeometryPass()
	{
		GL_PUSH_DEBUG_GROUP();
		geometryBuffer.Bind();

		geometryBuffer.attachments["color"].Draw();

		//we just need the first LOd so only do the first 3 meshes
		for (auto iter : testModel.meshes)
		{
			if (iter.isCollision)
			{
				continue;
			}

			iter.textures[0].SetActive(0); //we just want diffuse
			//add the previous depth?

			iter.Bind();
			geometryProgram.Use();

			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, iter.indices.size(), GL_UNSIGNED_INT, nullptr);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		frameBuffer::Unbind();
		glPopDebugGroup();
	}

	virtual void EdgeDetectionPass()
	{
		GL_PUSH_DEBUG_GROUP();
		edgesBuffer.Bind();

		edgesBuffer.attachments["edge"].Draw();

		geometryBuffer.attachments["color"].SetActive(0);//color
		geometryBuffer.attachments["depth"].SetActive(1);//depth

		defaultVertexBuffer.Bind();
		edgeDetectionProgram.Use();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		frameBuffer::Unbind();
		glPopDebugGroup();
	}

	virtual void BlendingWeightsPass()
	{
		GL_PUSH_DEBUG_GROUP();
		weightsBuffer.Bind();

		weightsBuffer.attachments["blend"].Draw();

		edgesBuffer.attachments["edge"].SetActive(0);
		SMAAArea.SetActive(1);
		SMAASearch.SetActive(2);

		defaultVertexBuffer.Bind();
		blendingWeightProgram.Use();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		frameBuffer::Unbind();
		glPopDebugGroup();
	}

	virtual void SMAAPass()
	{
		GL_PUSH_DEBUG_GROUP();
		SMAABuffer.Bind();
		SMAABuffer.attachments["SMAA"].Draw();

		//current frame
		geometryBuffer.attachments["color"].SetActive(0); // color
		weightsBuffer.attachments["blend"].SetActive(1); //blending weights

		defaultVertexBuffer.Bind();
		SMAAProgram.Use();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		frameBuffer::Unbind();
		glPopDebugGroup();
	}

	void FinalPass(const texture* tex1, const texture* tex2) const
	{
		GL_PUSH_DEBUG_GROUP();
		//draw directly to backbuffer
		tex1->SetActive(0);
		
		defaultVertexBuffer.Bind();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		if (enableCompare)
		{
			tex2->SetActive(1);
			compareProgram.Use();
		}

		else
		{
			finalProgram.Use();
		}
	
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glPopDebugGroup();
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawSMAASettings();
	}

	virtual void DrawBufferAttachments()
	{
		if (ImGui::BeginTabItem("framebuffers"))
		{
			for (const auto& val : geometryBuffer.attachments | std::views::values)
			{
				ImGui::Image((ImTextureID)val.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", val.GetUniformName().c_str());
			}

			ImGui::Image((ImTextureID)edgesBuffer.attachments["edge"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", edgesBuffer.attachments["edge"].GetUniformName().c_str());

			ImGui::Image((ImTextureID)weightsBuffer.attachments["blend"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", weightsBuffer.attachments["blend"].GetUniformName().c_str());

			ImGui::Image((ImTextureID)SMAABuffer.attachments["SMAA"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", SMAABuffer.attachments["SMAA"].GetUniformName().c_str());
			ImGui::EndTabItem();
		}
	}

	virtual void ClearBuffers() override
	{
		//move clearColor into a float array
		geometryBuffer.Bind();
		frameBuffer::ClearTexture(geometryBuffer.attachments["color"], clearColor);
		glClear(GL_DEPTH_BUFFER_BIT);
		frameBuffer::Unbind();

		SMAABuffer.Bind();
		frameBuffer::ClearTexture(SMAABuffer.attachments["SMAA"], clearColor2);
		frameBuffer::Unbind();

		edgesBuffer.Bind();
		frameBuffer::ClearTexture(edgesBuffer.attachments["edge"], clearColor2);
		frameBuffer::Unbind();

		weightsBuffer.Bind();
		frameBuffer::ClearTexture(weightsBuffer.attachments["blend"], clearColor2);
		frameBuffer::Unbind();
	}

	virtual void ResizeBuffers(const glm::ivec2 resolution)
	{
		for (auto val : geometryBuffer.attachments | std::views::values)
		{
			geometryBuffer.GetAttachmentRef(val.uniformName).Resize(glm::ivec3(resolution, 1));
		}

		edgesBuffer.GetAttachmentRef("edge").Resize(glm::ivec3(resolution, 1));
		weightsBuffer.GetAttachmentRef("blend").Resize(glm::ivec3(resolution, 1));
		SMAABuffer.GetAttachmentRef("SMAA").Resize(glm::ivec3(resolution, 1));

		SMAASettings->rtMetrics = glm::vec4(1.0f / window->GetSettings().resolution.width, 1.0f /  window->GetSettings().resolution.height, window->GetSettings().resolution.width, window->GetSettings().resolution.height );
	}

	void HandleWindowResize(const tWindow* window, const vec2_t<uint16_t>& dimensions) override
	{
		defaultPayload->resolution = glm::ivec2(dimensions.width, dimensions.height);
		ResizeBuffers(glm::ivec2(dimensions.x, dimensions.y));
	}

	void HandleMaximize(const tWindow* window) override
	{
		defaultPayload->resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		ResizeBuffers(defaultPayload->resolution);
	}

	void InitializeUniforms() override
	{
		scene::InitializeUniforms();
		auto smaaBlock = &bufferHandler.uniformBlocks["SMAASettings"];
		smaaBlock->SetPayload(SMAASettings_t());
		SMAASettings = smaaBlock->GetPayload<SMAASettings_t>();
	}

	void DrawSMAASettings()
	{
		if (ImGui::BeginTabItem("SMAA Settings"))
		{
			ImGui::Checkbox("enable Compare", &enableCompare);
			ImGui::SliderFloat("threshold", &SMAASettings->threshold, 0.001f, 1.0f, "%0.5f");
			ImGui::SliderFloat("contrast adaption factor", &SMAASettings->contrastAdaptationFactor, 0.1f, 5.0f, "0.5f");
			ImGui::SliderInt("max search steps", &SMAASettings->maxSearchSteps, 0, 255);
			ImGui::SliderInt("max search steps diagonal", &SMAASettings->maxSearchStepsDiag, 0, 255);
			ImGui::SliderInt("corner rounding", &SMAASettings->cornerRounding, 0, 255);

			//set a list box for edge detection modes
			static int edgeDetectionPick = (int32_t)SMAASettings->edgeDetectionMode;
			const std::vector edgeDetectionSettings = { "luma", "color", "depth" };
			ImGui::ListBox("Edge Detection Mode", &edgeDetectionPick, edgeDetectionSettings.data(), edgeDetectionSettings.size());
			switch (edgeDetectionPick)
			{
				case 0: SMAASettings->edgeDetectionMode = (int32_t)EdgeDetectionMode_e::luma; break;
				case 1: SMAASettings->edgeDetectionMode = (int32_t)EdgeDetectionMode_e::color; break;
				case 2: SMAASettings->edgeDetectionMode = (int32_t)EdgeDetectionMode_e::depth; break;
				default: break;
			}
			ImGui::EndTabItem();
		}
	}
};