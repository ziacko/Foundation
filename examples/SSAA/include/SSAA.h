#ifndef SSAA_H
#define SSAA_H

#include "scene3D.h"
#include "FrameBuffer.h"
#include "HaltonSequence.h"

using downsampleType_t = enum { none = 0, lanczos };

struct downscaleSettings_t
{
	float texelWidthOffset;
	float texelHeightOffset;
	int downsampleMode;

	downscaleSettings_t()
	{
		texelWidthOffset = 0.00025f;
		texelHeightOffset = 0.00025f;
		downsampleMode = none;
	}
};

struct lanzcosSettings_t
{
	float magicValue1;
	float magicValue2;
	float magicValue3;
	float magicValue4;
	float magicValue5;

	lanzcosSettings_t()
	{
		magicValue1 = 0.38026f;
		magicValue2 = 0.27667f;
		magicValue3 = 0.08074f;
		magicValue4 = -0.02612f;
		magicValue5 = -0.02143f;
	}
};

class SSAA : public scene3D
{
public:

	SSAA(
		const char* windowName = "Ziyad Barakat's portfolio (SSAA)",
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		model_t model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model), downscaleUniforms(nullptr),
		  lanzcosUniforms(nullptr)
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

		downscaledBuffer = new frameBuffer();
		geometryBuffer = new frameBuffer();
		unJitteredBuffer = new frameBuffer();

		this->camera.position.y -= 100.0f;
	}

	~SSAA() {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		FBODescriptor ssDesc;
		ssDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width * 2, window->GetSettings().resolution.height * 2, 1);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();
		geometryBuffer->AddAttachment(frameBuffer::attachment_t("color", ssDesc));

		FBODescriptor ssDepthDesc;
		ssDepthDesc.dataType = GL_FLOAT;
		ssDepthDesc.format = GL_DEPTH_COMPONENT;
		ssDepthDesc.internalFormat = GL_DEPTH_COMPONENT24;
		ssDepthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		ssDepthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width * 2, window->GetSettings().resolution.height * 2, 1);

		geometryBuffer->AddAttachment(frameBuffer::attachment_t("depth", ssDepthDesc));

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		downscaledBuffer->Initialize();
		downscaledBuffer->Bind();
		downscaledBuffer->AddAttachment(frameBuffer::attachment_t("downscaled", colorDesc));

		//change back to regular dimensions here
		ssDepthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		unJitteredBuffer->Initialize();
		unJitteredBuffer->Bind();
		unJitteredBuffer->AddAttachment(frameBuffer::attachment_t("unJittered", colorDesc));
		unJitteredBuffer->AddAttachment(frameBuffer::attachment_t("depth", ssDepthDesc));

		//geometry automatically gets assigned to 0
		defProgram = shaderProgramsMap["geometryProgram"];
		downscaleProgram = shaderProgramsMap["downscaleProgram"];
		unjitteredProgram = shaderProgramsMap["treeProgram"];
		compareProgram = shaderProgramsMap["compareProgram"];
		finalProgram = shaderProgramsMap["finalProgram"];

		frameBuffer::Unbind();
	}

protected:

	frameBuffer* geometryBuffer;
	frameBuffer* downscaledBuffer;
	frameBuffer* unJitteredBuffer;

	shaderProgram_t downscaleProgram;
	shaderProgram_t unjitteredProgram;
	shaderProgram_t compareProgram;
	shaderProgram_t finalProgram;

	bool enableCompare = true;
	downscaleSettings_t*	downscaleUniforms = nullptr;
	lanzcosSettings_t*		lanzcosUniforms = nullptr;

	std::vector<const char*>		downsampleSettings = { "none", "lanczos" };

	virtual void Draw() override
	{
		camera.ChangeProjection(camera_t::projection_e::perspective);
		glm::vec2 defaultResolution = camera.resolution;
		camera.resolution = glm::vec2(camera.resolution.x * 2, camera.resolution.y * 2);
		camera.Update();

		UpdateDefaultUniforms(camera, clock, &testModel);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		UpsamplePass(); //render current scene with jitter
		camera.resolution = defaultResolution;
		if (enableCompare)
		{
			UnJitteredPass();
		}
		glDisable(GL_BLEND);

		camera.ChangeProjection(camera_t::projection_e::orthographic);
		UpdateDefaultUniforms(camera, clock, &testModel);
		
		DownscalePass(); //use the positions, colors, depth and velocity to smooth the final image

		FinalPass(&downscaledBuffer->attachments["downscaled"], &unJitteredBuffer->attachments["unJittered"]);
	}

	virtual void UpsamplePass()
	{
		GL_PUSH_DEBUG_GROUP();
		geometryBuffer->Bind();
		geometryBuffer->attachments["color"].Draw();

		//we just need the first LOD so only do the first 3 meshes
		for ( auto mesh : testModel.meshes)
		{
			if (mesh.isCollision)
			{
				continue;
			}

			mesh.textures[0].SetActive(0);

			mesh.Bind();
			defProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width * 2, window->GetSettings().resolution.height * 2);
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();

		glPopDebugGroup();
	}

	virtual void UnJitteredPass()
	{
		GL_PUSH_DEBUG_GROUP();
		unJitteredBuffer->Bind();
		unJitteredBuffer->attachments["unJittered"].Draw();

		//we just need the first LOd so only do the first 3 meshes
		for ( auto mesh : testModel.meshes)
		{
			if (mesh.isCollision)
			{
				continue;
			}

			mesh.textures[0].SetActive(0);
			//add the previous depth?

			mesh.Bind();
			unjitteredProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		unJitteredBuffer->Unbind();
		glPopDebugGroup();
	}

	virtual void DownscalePass()
	{
		GL_PUSH_DEBUG_GROUP();
		downscaledBuffer->Bind();
		downscaledBuffer->attachments["downscaled"].Draw();

		//current frame
		geometryBuffer->attachments["color"].SetActive(0); //current color
		
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		downscaleProgram.Use();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		downscaledBuffer->Unbind();
		glPopDebugGroup();
	}

	void FinalPass(texture* tex1, texture* tex2) const
	{
		GL_PUSH_DEBUG_GROUP();
		//draw directly to backbuffer		
		tex1->SetActive(0);
		
		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
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

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		//DrawTAASettings();
		DrawDownsampleSettings();
	}

	virtual void DrawDownsampleSettings()
	{
		if (ImGui::BeginTabItem("Lanczos settings"))
		{
			ImGui::ListBox("downsample types", &downscaleUniforms->downsampleMode, downsampleSettings.data(), downsampleSettings.size());
			ImGui::DragFloat("texel width", &downscaleUniforms->texelWidthOffset, 0.1f, 0.0f, 10.0f, "%.5f");
			ImGui::DragFloat("texel height", &downscaleUniforms->texelHeightOffset, 0.1f, 0.0f, 10.0f, "%.5f");
			switch(downscaleUniforms->downsampleMode)
			{
			case none:
				{
					break;
				}

			case lanczos:
				{
					ImGui::DragFloat("value 1", &lanzcosUniforms->magicValue1, 0.01f, 0.0f, 1.0f, "%.3f");
					ImGui::DragFloat("value 2", &lanzcosUniforms->magicValue2, 0.01f, 0.0f, 1.0f, "%.3f");
					ImGui::DragFloat("value 3", &lanzcosUniforms->magicValue3, 0.01f, 0.0f, 1.0f, "%.3f");
					ImGui::DragFloat("value 4", &lanzcosUniforms->magicValue4, 0.01f, 0.0f, 1.0f, "%.3f");
					ImGui::DragFloat("value 5", &lanzcosUniforms->magicValue5, 0.01f, 0.0f, 1.0f, "%.3f");
					break;
				}

			default:
				{
					break;
				}

			}

			ImGui::EndTabItem();
		}
	}

	virtual void DrawBufferAttachments()
	{
		if (ImGui::BeginTabItem("framebuffers"))
		{
			for (auto iter : geometryBuffer->attachments | std::views::values)
			{
				ImGui::Image((ImTextureID*)iter.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter.GetUniformName().c_str());
			}

			ImGui::Image((ImTextureID*)downscaledBuffer->attachments["downscaled"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", downscaledBuffer->attachments["downscaled"].GetUniformName().c_str());

			ImGui::Image((ImTextureID*)unJitteredBuffer->attachments["unJittered"].GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", unJitteredBuffer->attachments["unJittered"].GetUniformName().c_str());

			ImGui::EndTabItem();
		}
	}

	virtual void DrawCameraStats() override
	{
		//set up the view matrix
		if (ImGui::BeginTabItem("camera"))
		{
			ImGui::DragFloat("near plane", &camera.nearPlane);
			ImGui::DragFloat("far plane", &camera.farPlane);
			ImGui::SliderFloat("Field of view", &camera.fieldOfView, 0, 90, "%.0f");

			ImGui::InputFloat("camera speed", &camera.speed, 0.f);
			ImGui::InputFloat("x sensitivity", &camera.xSensitivity, 0.f);
			ImGui::InputFloat("y sensitivity", &camera.ySensitivity, 0.f);
			ImGui::EndTabItem();
		}
	}

	virtual void ClearBuffers() override
	{
		downscaledBuffer->Bind(); //clear the previous, the next frame current becomes previous
		downscaledBuffer->ClearTexture(downscaledBuffer->attachments["downscaled"], clearColor);
		downscaledBuffer->Unbind();

		geometryBuffer->Bind();
		geometryBuffer->ClearTexture(geometryBuffer->attachments["color"], clearColor);
		geometryBuffer->ClearTexture(geometryBuffer->attachments["depth"], clearColor);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();

		unJitteredBuffer->Bind();
		unJitteredBuffer->ClearTexture(unJitteredBuffer->attachments["color"], clearColor);
		glClear(GL_DEPTH_BUFFER_BIT);
		unJitteredBuffer->Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		for (auto iter : geometryBuffer->attachments | std::views::values)
		{
			iter.Resize(glm::ivec3(resolution.x * 2, resolution.y * 2, 1));
		}

		downscaledBuffer->attachments["downscaled"].Resize(glm::ivec3(resolution.x, resolution.y, 1));
		unJitteredBuffer->attachments["unJittered"].Resize(glm::ivec3(resolution.x, resolution.y, 1));
		unJitteredBuffer->attachments["depth"].Resize(glm::ivec3(resolution.x, resolution.y, 1));
	}

	virtual void HandleWindowResize(const tWindow* window, const tw::vec2_t<uint16_t>& dimensions) override
	{
		defaultPayload->resolution = glm::ivec2(dimensions.width, dimensions.height);
		ResizeBuffers(glm::ivec2(dimensions.x, dimensions.y));
	}

	virtual void HandleMaximize(const tWindow* window) override
	{
		defaultPayload->resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		ResizeBuffers(defaultPayload->resolution);
	}

	virtual void InitializeUniforms() override
	{
		scene3D::InitializeUniforms();

		auto downscaleBlock = &bufferHandler.uniformBlocks["downscaleSettings"];
		downscaleBlock->SetPayload<downscaleSettings_t>(downscaleSettings_t());
		downscaleUniforms = downscaleBlock->GetPayload<downscaleSettings_t>();

		auto lanzcosBlock = &bufferHandler.uniformBlocks["lanczosSettings"];
		lanzcosBlock->SetPayload<lanzcosSettings_t>(lanzcosSettings_t());
		lanzcosUniforms = lanzcosBlock->GetPayload<lanzcosSettings_t>();
	}

};

#endif