#ifndef DYNAMIC_RES_H
#define DYNAMIC_RES_H

#include "scene3D.h"
#include "FrameBuffer.h"

constexpr glm::vec2 defaultResScale = glm::vec2(1, 1);

struct resolutionSettings_t
{
	glm::vec2 resolutionScale{defaultResScale};

	explicit resolutionSettings_t(const glm::vec2& res = defaultResScale)
	{
		resolutionScale = res;
	}
};

class dynamicRes final : public scene3D
{
public:

	explicit dynamicRes(
		const char* windowName = "Ziyad Barakat's portfolio (dynamic resolution)",
		const camera_t& texModelCamera = camera_t(glm::vec2(1280, 720), PI * 0.1f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		const model_t& model = model_t("models/SoulSpear/SoulSpear.fbx"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model), scaledResolution()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

		geometryBuffer = new frameBuffer();
		dynamicBuffer = new frameBuffer();

		resolution = bufferHandler_t<resolutionSettings_t>();
	}

	virtual void Initialize() override
	{
		scene3D::Initialize();

		FBODescriptor depthDesc;
		depthDesc.target = GL_TEXTURE_2D;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);
		
		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		dynamicBuffer->Initialize();
		dynamicBuffer->Bind();
		dynamicBuffer->AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		dynamicBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		frameBuffer::Unbind();

		defProgram = shaderProgramsMap["geometry"];
		compareProgram = shaderProgramsMap["compare"];
		finalProgram = shaderProgramsMap["final"];

		defaultVertexBuffer.SetupDefault();
	}

protected:

	frameBuffer* geometryBuffer;
	frameBuffer* dynamicBuffer;

	shaderProgram_t compareProgram;
	shaderProgram_t finalProgram;

	glm::ivec2 scaledResolution;

	bool enableCompare = true;

	bufferHandler_t<resolutionSettings_t>		resolution;

	virtual void Update() override
	{
		scene3D::Update();
		resolution.Update();
	}


	void Draw() override
	{
		GL_PUSH_DEBUG_GROUP();
		camera.ChangeProjection(camera_t::projection_e::perspective);
		camera.Update();
		UpdateDefaultUniforms(camera, clock);

		GeometryPass();
		DynamicPass();

		camera.ChangeProjection(camera_t::projection_e::orthographic);
		camera.Update();
		UpdateDefaultUniforms(camera, clock);

		FinalPass(&geometryBuffer->attachments["color"], &dynamicBuffer->attachments["color"]);
		glPopDebugGroup();
	}

	virtual void GeometryPass() const
	{
		GL_PUSH_DEBUG_GROUP();
		geometryBuffer->Bind();

		geometryBuffer->attachments["color"].Draw();

		for (size_t i = 0; i < testModel.meshes.size(); ++i)
		{
			if (testModel.meshes[i].isCollision)
				continue;

			for (uint8_t tex = 0; tex < testModel.meshes[i].textures.size(); ++tex)
			{
				testModel.meshes[i].textures[tex].SetActive(tex);
			}

			glBindVertexArray(testModel.meshes[i].vertexArrayHandle);
			defProgram.Use();
			
			auto res = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
			glViewport(0, 0, (GLint)res.x, (GLint)res.y);
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();
		glPopDebugGroup();
	}

	virtual void DynamicPass()
	{
		GL_PUSH_DEBUG_GROUP();
		dynamicBuffer->Bind();

		dynamicBuffer->attachments["color"].Draw();

		for (size_t i = 0; i < testModel.meshes.size(); ++i)
		{
			if (testModel.meshes[i].isCollision)
				continue;

			for (uint8_t tex = 0; tex < testModel.meshes[i].textures.size(); ++tex)
			{
				testModel.meshes[i].textures[tex].SetActive(tex);
			}

			glBindVertexArray(testModel.meshes[i].vertexArrayHandle);
			defProgram.Use();

			auto res = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height) * this->resolution.data.resolutionScale;
			glViewport(0, 0, (GLint)res.x, (GLint)res.y);
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		dynamicBuffer->Unbind();
		glPopDebugGroup();
	}

	void FinalPass(texture* tex1, texture* tex2) const
	{
		GL_PUSH_DEBUG_GROUP();
		//draw directly to backbuffer
		tex1->SetActive(0);
		tex2->SetActive(1);

		defaultVertexBuffer.Bind();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		if (enableCompare == true)
		{
			compareProgram.Use();
		}
		else
		{
			finalProgram.Use();
		}
		
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glPopDebugGroup();
	}

	void InitializeUniforms() override
	{
		scene3D::InitializeUniforms();
		defaultVertexBuffer.SetupDefault();
		resolution.Initialize(1);
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawResolutionSettings();
	}

	virtual void DrawResolutionSettings()
	{
		if (ImGui::BeginTabItem("resolution Settings"))
		{
			if (ImGui::SliderFloat("horizontal %", &resolution.data.resolutionScale.x, 0.25f, 2.0f, "%.3f") ||
				ImGui::SliderFloat("vertical %", &resolution.data.resolutionScale.y, 0.25f, 2.0f, "%.3f"))
			{
				//camera.resolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height) * resolution.data.resolutionScale;
				ResizeBuffers(glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height) * resolution.data.resolutionScale);
			}
			ImGui::EndTabItem();
		}
	}

	virtual void DrawBufferAttachments()
	{
		if (ImGui::BeginTabItem("framebuffers"))
		{
			for (auto& iter : geometryBuffer->attachments | std::views::values)
			{
				ImGui::Image((ImTextureID)iter.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter.GetUniformName().c_str());
			}

			for (auto& iter : dynamicBuffer->attachments | std::views::values)
			{
				ImGui::Image((ImTextureID)iter.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter.GetUniformName().c_str());
			}

			ImGui::EndTabItem();
		}
	}

	virtual void ClearBuffers() override
	{
		// Clear the FBO color and depth
		geometryBuffer->Bind();
		frameBuffer::ClearTexture(geometryBuffer->attachments["color"], clearColor);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();

		dynamicBuffer->Bind();
		frameBuffer::ClearTexture(dynamicBuffer->attachments["color"], clearColor);
		glClear(GL_DEPTH_BUFFER_BIT);
		dynamicBuffer->Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(const glm::ivec2& newSize)
	{
		auto res = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height) * this->resolution.data.resolutionScale;
		geometryBuffer->Resize(glm::ivec3((int)window->GetSettings().resolution.width, (int)window->GetSettings().resolution.height, 1));
		dynamicBuffer->Resize(glm::ivec3((int)res.x, (int)res.y, 1));
	}

	virtual void HandleWindowResize(const tWindow* window, const tw::vec2_t<uint16_t>& dimensions) override
	{
		defaultPayload.data.resolution = glm::ivec2(dimensions.width, dimensions.height);
		ResizeBuffers(glm::ivec2(dimensions.x, dimensions.y));
	}

	virtual void HandleMaximize(const tWindow* window) override
	{
		defaultPayload.data.resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		ResizeBuffers(defaultPayload.data.resolution);
	}
};
#endif