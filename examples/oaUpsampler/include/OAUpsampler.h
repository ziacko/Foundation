#ifndef OA_UPSAMPLER_H
#define OA_UPSAMPLER_H

#include "DynamicRes.h"

#include "ImGuizmo.h"

#include "FrameBuffer.h"
#include "Transform.h"

struct SMAA_Settings_t
{
	float	threshold;
	float	contrastAdaptationFactor;

	int		maxSearchSteps;
	int		maxSearchStepsDiag;
	int		cornerRounding;

	SMAA_Settings_t(float threshold = 0.05, float CAFactor = 2.0f, unsigned int maxSearchSteps = 32, unsigned int maxSearchStepsDiag = 16, unsigned int cornerRounding = 25)
	{
		this->threshold = threshold;
		this->contrastAdaptationFactor = CAFactor;
		this->maxSearchSteps = maxSearchSteps;
		this->maxSearchStepsDiag = maxSearchStepsDiag;
		this->cornerRounding = cornerRounding;
	}
};

typedef enum { BILINEAR = 0, BICUBIC, CATMULL, NONE } BiFilters_t;

struct BiFilter_Settings_t
{
	int biFilterIndex;

	BiFilter_Settings_t(int biFilterIndex = BiFilters_t::BILINEAR)
	{
		this->biFilterIndex = biFilterIndex;
	}
};


class OAUpsampler : public dynamicRes
{
public:

	OAUpsampler(
		const char* windowName = "One Axis Upsampler",
		camera_t* texModelCamera = new camera_t(glm::vec2(1280, 720), 0.2f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = "../../resources/shaders/OAUpsampler.txt",
		model_t* model = new model_t("../../resources/models/SoulSpear/SoulSpear.FBX"))
		: dynamicRes(windowName, texModelCamera, shaderConfigPath, model)
	{
		edgesBuffer = new frameBuffer();
		weightsBuffer = new frameBuffer();
		SMAABuffer = new frameBuffer();
		upsampledBuffer = new frameBuffer();

		SMAAArea = new texture("../../resources/textures/SMAA/AreaTexDX10.dds");
		SMAASearch = new texture("../../resources/textures/SMAA/SearchTex.dds");
	}

	virtual ~OAUpsampler() { dynamicRes::~dynamicRes(); };

	virtual void Initialize() override
	{
		scene3D::Initialize();

		SMAAArea->LoadTexture();
		SMAASearch->LoadTexture();

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height, 1);

		FBODescriptor depthDesc;
		depthDesc.target = GL_TEXTURE_2D;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = gl_depth_component24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_t::depth;
		depthDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height, 1);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("color"));
		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("depth", depthDesc));

		edgesBuffer->Initialize();
		edgesBuffer->Bind();

		FBODescriptor edgeDesc;
		edgeDesc.format = gl_rg;
		edgeDesc.internalFormat = gl_rg8;
		edgeDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height, 1);

		edgesBuffer->AddAttachment(new frameBuffer::attachment_t("edge", edgeDesc));

		weightsBuffer->Initialize();
		weightsBuffer->Bind();

		weightsBuffer->AddAttachment(new frameBuffer::attachment_t("blend", colorDesc));

		SMAABuffer->Initialize();
		SMAABuffer->Bind();
		SMAABuffer->AddAttachment(new frameBuffer::attachment_t("SMAA", colorDesc));

		upsampledBuffer->Initialize();
		upsampledBuffer->Bind();
		upsampledBuffer->AddAttachment(new frameBuffer::attachment_t("upsampled", colorDesc));

		edgeDetectionProgram = shaderPrograms[1]->handle;
		blendingWeightProgram = shaderPrograms[2]->handle;
		SMAAProgram = shaderPrograms[3]->handle;
		compareProgram = shaderPrograms[4]->handle;
		finalProgram = shaderPrograms[5]->handle;
		biFilterProgram = shaderPrograms[6]->handle;

		frameBuffer::Unbind();
	}

protected:

	bufferHandler_t<resolutionSettings_t>	resolution;
	bufferHandler_t<SMAA_Settings_t>		SMAAsettings;
	bufferHandler_t<BiFilter_Settings_t>	BiFilterSettings;
	frameBuffer* edgesBuffer;
	frameBuffer* weightsBuffer;
	frameBuffer* SMAABuffer;
	frameBuffer* upsampledBuffer;

	texture* SMAAArea;
	texture* SMAASearch;

	unsigned int edgeDetectionProgram = 0;
	unsigned int blendingWeightProgram = 0;
	unsigned int SMAAProgram = 0;
	unsigned int upsampleProgram = 0;
	unsigned int biFilterProgram = 0;

	glm::vec2 resPercent = glm::vec2(1.0f);

	std::vector<const char*>	BiFilters = { "Bilinear", "BiCubic", "Catmull-Rom", "None" };

	virtual void Update() override
	{
		dynamicRes::Update();
		resolution.Update(gl_uniform_buffer, gl_dynamic_draw);
		SMAAsettings.Update(gl_uniform_buffer, gl_dynamic_draw);
		BiFilterSettings.Update(gl_uniform_buffer, gl_dynamic_draw);
	}

	void UpdateDefaultBuffer()
	{
		dynamicRes::UpdateDefaultBuffer();
	}

	virtual void EdgeDetectionPass()
	{
		edgesBuffer->Bind();

		glDrawBuffers(1, &edgesBuffer->attachments[0]->FBODesc.attachmentFormat);

		geometryBuffer->attachments[0]->SetActive(0);//color
		geometryBuffer->attachments[1]->SetActive(1);//depth

		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glUseProgram(edgeDetectionProgram);
		glViewport(0, 0, sceneCamera->resolution.x, sceneCamera->resolution.y);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		edgesBuffer->Unbind();
	}

	virtual void BlendingWeightsPass()
	{
		weightsBuffer->Bind();

		glDrawBuffers(1, &weightsBuffer->attachments[0]->FBODesc.attachmentFormat);

		edgesBuffer->attachments[0]->SetActive(0);
		SMAAArea->SetActive(1);
		SMAASearch->SetActive(2);

		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glUseProgram(blendingWeightProgram);
		glViewport(0, 0, sceneCamera->resolution.x, sceneCamera->resolution.y);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		weightsBuffer->Unbind();
	}

	virtual void SMAAPass()
	{
		SMAABuffer->Bind();
		glDrawBuffers(1, &SMAABuffer->attachments[0]->FBODesc.attachmentFormat);

		//current frame
		geometryBuffer->attachments[0]->SetActive(0); // color
		weightsBuffer->attachments[0]->SetActive(1); //blending weights

		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glUseProgram(SMAAProgram);
		glViewport(0, 0, sceneCamera->resolution.x, sceneCamera->resolution.y);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		SMAABuffer->Unbind();
	}

	virtual void FinalPass(texture* tex1, texture* tex2)
	{
		//draw directly to backbuffer		
		tex1->SetActive(0);

		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		if (enableCompare)
		{
			tex2->SetActive(1);
			glUseProgram(compareProgram);
		}

		else
		{
			glUseProgram(finalProgram);
		}

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	virtual void Draw()
	{
		sceneCamera->resolution = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);// *resPercent;
		defaultPayload.data.resolution = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);// *resPercent;
		//resolution.data.resPercent = glm::vec2(windows[0]->settings.resolution.width * resPercent.x, windows[0]->settings.resolution.height * resPercent.y);
		sceneCamera->ChangeProjection(camera_t::projection_e::perspective);
		sceneCamera->Update();
		UpdateDefaultBuffer();

		GeometryPass();

		sceneCamera->resolution = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) *resPercent;
		defaultPayload.data.resolution = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) * resPercent;
		//resolution.data.resPercent = glm::vec2(windows[0]->settings.resolution.width * resPercent.x, windows[0]->settings.resolution.height * resPercent.y);
		sceneCamera->ChangeProjection(camera_t::projection_e::orthographic); //should probably use 2 different cameras...
		sceneCamera->Update();
		UpdateDefaultBuffer();		

		EdgeDetectionPass();
		BlendingWeightsPass();
		SMAAPass();

		sceneCamera->resolution = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);// *resPercent;
		defaultPayload.data.resolution = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);// *resPercent;
		//resolution.data.resPercent = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		sceneCamera->ChangeProjection(camera_t::projection_e::orthographic); //should probably use 2 different cameras...
		sceneCamera->Update();		
		UpdateDefaultBuffer();
		
		UpsamplePass(SMAABuffer->attachments[0]);
		FinalPass(upsampledBuffer->attachments[0], geometryBuffer->attachments[0]);
		
		DrawGUI(windows[0]);
		
		windows[0]->SwapDrawBuffers();
		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void UpsamplePass(texture* tex1)
	{
		upsampledBuffer->Bind();
		glDrawBuffers(1, &upsampledBuffer->attachments[0]->FBODesc.attachmentFormat);
		tex1->SetActive(0);

		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		glUseProgram(biFilterProgram);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		upsampledBuffer->Unbind();
	}

	virtual void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene3D::BuildGUI(windows[0], io);
		DrawSMAASettings();
		DrawBufferAttachments();
		DrawResolutionSettings();
		DrawBiFilterSettings();
	}

	virtual void DrawResolutionSettings()
	{
		ImGui::Begin("resolution Settings");
		
		if (ImGui::SliderFloat("horizontal \%", &resPercent.x, 0.25f, 2.0f, "%.3f") ||
			ImGui::SliderFloat("vertical \%", &resPercent.y, 0.25f, 2.0f, "%.3f"))
		{
			res = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) * resPercent;

			ResizeBuffers(glm::ivec2(res));

			resolution.data.resPercent = glm::vec2(windows[0]->settings.resolution.width * resPercent.x, windows[0]->settings.resolution.height * resPercent.y);
			sceneCamera->resolution = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) * resPercent;
			defaultPayload.data.resolution = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) * resPercent;
		}

		ImGui::End();
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		auto windowRes = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);// *this->resolution.data.resPercent;
		for (auto iter : geometryBuffer->attachments)
		{
			iter->Resize(glm::ivec3(resolution, 1));
		}

		edgesBuffer->attachments[0]->Resize(glm::ivec3(resolution, 1));
		weightsBuffer->attachments[0]->Resize(glm::ivec3(resolution, 1));
		SMAABuffer->attachments[0]->Resize(glm::ivec3(resolution, 1));
		upsampledBuffer->attachments[0]->Resize(glm::ivec3(windowRes, 1));

		//glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
	}

	virtual void DrawBufferAttachments()
	{
		ImGui::Begin("framebuffers");
		for (auto iter : geometryBuffer->attachments)
		{
			ImGui::Image((ImTextureID*)iter->GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", iter->GetUniformName().c_str());
		}

		ImGui::Image((ImTextureID*)edgesBuffer->attachments[0]->GetHandle(), ImVec2(512, 288),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Text("%s\n", edgesBuffer->attachments[0]->GetUniformName().c_str());

		ImGui::Image((ImTextureID*)weightsBuffer->attachments[0]->GetHandle(), ImVec2(512, 288),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Text("%s\n", weightsBuffer->attachments[0]->GetUniformName().c_str());

		ImGui::Image((ImTextureID*)SMAABuffer->attachments[0]->GetHandle(), ImVec2(512, 288),
			ImVec2(0, 1), ImVec2(1, 0));
		ImGui::SameLine();
		ImGui::Text("%s\n", SMAABuffer->attachments[0]->GetUniformName().c_str());
		ImGui::End();
	}

	virtual void ClearBuffers()
	{
		//ok copy the current frame into the previous frame and clear the rest of the buffers	
		float clearColor1[4] = { 0.25f, 0.25f, 0.25f, 1.0f };
		float clearColor2[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

		geometryBuffer->Bind();
		geometryBuffer->ClearTexture(geometryBuffer->attachments[0], clearColor1);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();

		SMAABuffer->Bind();
		SMAABuffer->ClearTexture(SMAABuffer->attachments[0], clearColor1);
		SMAABuffer->Unbind();

		edgesBuffer->Bind();
		edgesBuffer->ClearTexture(edgesBuffer->attachments[0], clearColor2);
		edgesBuffer->Unbind();

		weightsBuffer->Bind();
		weightsBuffer->ClearTexture(weightsBuffer->attachments[0], clearColor2);
		weightsBuffer->Unbind();

		upsampledBuffer->Bind();
		upsampledBuffer->ClearTexture(upsampledBuffer->attachments[0], clearColor1);
		upsampledBuffer->Unbind();

		sceneCamera->ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void HandleWindowResize(tWindow* window, TinyWindow::vec2_t<unsigned int> dimensions) override
	{
		resolution.data.resPercent = glm::vec2(windows[0]->settings.resolution.width * resPercent.x, windows[0]->settings.resolution.height * resPercent.y);
		defaultPayload.data.resolution = glm::ivec2(window->settings.resolution.width, window->settings.resolution.height);
		ResizeBuffers(defaultPayload.data.resolution);

		res = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) * resPercent;
	}

	virtual void HandleMaximize(tWindow* window) override
	{
		resolution.data.resPercent = glm::vec2(windows[0]->settings.resolution.width * resPercent.x, windows[0]->settings.resolution.height * resPercent.y);
		defaultPayload.data.resolution = glm::ivec2(window->settings.resolution.width, window->settings.resolution.height);
		ResizeBuffers(defaultPayload.data.resolution);

		res = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) * resPercent;

	}

	virtual void InitializeUniforms() override
	{
		defaultPayload = bufferHandler_t<defaultUniformBuffer>(sceneCamera);
		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);

		defaultPayload.data.resolution = glm::ivec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		defaultPayload.data.projection = sceneCamera->projection;
		defaultPayload.data.translation = sceneCamera->translation;
		defaultPayload.data.view = sceneCamera->view;

		defaultPayload.Initialize(0);
		resolution.Initialize(2);
		SMAAsettings.Initialize(1);
		BiFilterSettings.Initialize(3);

		SetupVertexBuffer();
	}

	void DrawSMAASettings()
	{
		ImGui::Begin("SMAA Settings");
		ImGui::Checkbox("enable Compare", &enableCompare);
		ImGui::InputFloat("threshold", &SMAAsettings.data.threshold, 0.001f, 0.1f);
		ImGui::InputFloat("contrast adaption factor", &SMAAsettings.data.contrastAdaptationFactor, 0.001f, 0.1f);
		ImGui::SliderInt("max search steps", &SMAAsettings.data.maxSearchSteps, 0, 255);
		ImGui::SliderInt("max search steps diagonal", &SMAAsettings.data.maxSearchStepsDiag, 0, 255);
		ImGui::SliderInt("corner rounding", &SMAAsettings.data.cornerRounding, 0, 255);

		ImGui::End();
	}

	void DrawBiFilterSettings()
	{
		ImGui::Begin("Bi-Whatever setting");
		static int filterPick = 0;
		ImGui::ListBox("Texture Filter Index", &filterPick, BiFilters.data(), (int)BiFilters.size());
		BiFilterSettings.data.biFilterIndex = filterPick;
		ImGui::End();
	}
};

#endif