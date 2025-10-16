#ifndef FXAA_H
#define FXAA_H

#include "scene3D.h"

struct FXAASettings_t
{
	glm::float1		pixelShift;
	glm::float1		vxOffset;
	glm::float1		maxSpan;
	glm::float1		reduceMul;
	glm::float1		reduceMin;

	explicit FXAASettings_t(
		const glm::float1 pixelShift = 0.25f, const glm::float1 vxOffset = 0.0f, const glm::float1 maxSpan = 8.0f,
		const glm::float1 reduceMul = 0.125f, const glm::float1 reduceMin = 0.0078125f)
	{
		this->pixelShift = pixelShift;
		this->vxOffset = vxOffset;
		this->maxSpan = maxSpan;
		this->reduceMul = reduceMul;
		this->reduceMin = reduceMin;
	}

	~FXAASettings_t() { };
};

class FXAA_Scene final : public scene3D
{
public:

	explicit FXAA_Scene(
		const char* windowName = "Ziyad Barakat's portfolio (FXAA)",
		const camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		const model_t model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

		geometryBuffer = frameBuffer();
		FXAABuffer = frameBuffer();

		this->camera.position.y -= 100.0f;
	}

	~FXAA_Scene() override = default;

	virtual void Initialize() override
	{
		scene3D::Initialize();

		FBODescriptor depthDesc;
		depthDesc.target = GL_TEXTURE_2D;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		const auto res = window->GetSettings().resolution;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(res.width, res.height, 1);

		geometryBuffer.Initialize();
		geometryBuffer.Bind();

		geometryBuffer.AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer.AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		FXAABuffer.Initialize();
		FXAABuffer.Bind();
		FXAABuffer.AddAttachment(frameBuffer::attachment_t("FXAA", colorDesc));

		frameBuffer::Unbind();

		defProgram = shaderProgramsMap["geometryProgram"];
		FXAAProgram = shaderProgramsMap["FXAAProgram"];
		compareProgram = shaderProgramsMap["compareProgram"];
		finalProgram = shaderProgramsMap["finalProgram"];
	}

protected:

	frameBuffer geometryBuffer;
	frameBuffer FXAABuffer;

	shaderProgram_t FXAAProgram;
	shaderProgram_t compareProgram;
	shaderProgram_t finalProgram;

	bool enableCompare = true;

	FXAASettings_t*		FXAA = nullptr;

	void Draw() override
	{
		GL_PUSH_DEBUG_GROUP();

		GeometryPass(); //render current scene with jitter
		FXAAPass(); //use the positions, colors, depth and velocity to smooth the final image
		FinalPass(&FXAABuffer.attachments["FXAA"], &geometryBuffer.attachments["color"]);

		glPopDebugGroup();
	}

	virtual void GeometryPass()
	{
		GL_PUSH_DEBUG_GROUP();
		geometryBuffer.Bind();
		geometryBuffer.attachments["color"].Draw(); //color

		//we just need the first LOd so only do the first 3 meshes
		for (size_t iter = 0; iter < testModel.meshes.size(); iter++)
		{
			if (testModel.meshes[iter].isCollision)
			{
				continue;
			}

			for (uint8_t textureIter = 0; textureIter < testModel.meshes[iter].textures.size(); textureIter++)
			{
				testModel.meshes[iter].textures[textureIter].SetActive(textureIter);
			}

			glBindVertexArray(testModel.meshes[iter].vertexArrayHandle);
			defProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel.meshes[iter].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer.Unbind();

		glPopDebugGroup();
	}

	virtual void FXAAPass()
	{
		GL_PUSH_DEBUG_GROUP();
		FXAABuffer.Bind();
		FXAABuffer.attachments["FXAA"].Draw();

		//current frame
		geometryBuffer.attachments["color"].SetActive(0); // color
		
		defaultVertexBuffer.Bind();
		FXAAProgram.Use();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		FXAABuffer.Unbind();
		glPopDebugGroup();
	}

	void FinalPass(texture* tex1, texture* tex2)
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

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);

		DrawBufferAttachments();
		DrawFXAASettings();
	}

	void DrawFXAASettings()
	{
		if (ImGui::BeginTabItem("FXAA Settings"))
		{
			ImGui::Checkbox("enable Compare", &enableCompare);
			ImGui::SliderFloat("Sub pixel drift", &FXAA->pixelShift, 0.0f, 1.0f, "%.1f");
			ImGui::SliderFloat("vertex Offset", &FXAA->vxOffset, 0.0f, 1.0f, "%.3f");
			ImGui::SliderFloat("max span", &FXAA->maxSpan, 0.0f, 10.0f, "%.1f");
			ImGui::SliderFloat("reduce multiplier", &FXAA->reduceMul, 0.0f, 1.0f, "%.5f");
			ImGui::SliderFloat("reduce minimizer", &FXAA->reduceMin, 0.0f, 1.0f, "%.8f");

			ImGui::EndTabItem();
		}
	}

	virtual void DrawBufferAttachments()
	{
		if (ImGui::BeginTabItem("framebuffers"))
		{
			for (const auto& iter : geometryBuffer.attachments | std::views::values)
			{
				ImGui::Image((ImTextureID)iter.GetHandle(), ImVec2(512, 288),
					ImVec2(0, 1), ImVec2(1, 0));
				ImGui::SameLine();
				ImGui::Text("%s\n", iter.GetUniformName().c_str());
			}

			for (const auto& iter : FXAABuffer.attachments | std::views::values)
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
		geometryBuffer.Bind();
		geometryBuffer.ClearTexture(geometryBuffer.attachments["color"], clearColor);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer.Unbind();

		FXAABuffer.Bind();
		FXAABuffer.ClearTexture(FXAABuffer.attachments["FXAA"], clearColor);
		FXAABuffer.Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(const glm::ivec2& resolution)
	{
		for (auto iter : geometryBuffer.attachments | std::views::values)
		{
			iter.Resize(glm::ivec3(resolution, 1));
		}

		FXAABuffer.attachments["FXAA"].Resize(glm::ivec3(resolution, 1));
	}

	virtual void HandleWindowResize(const tWindow* window, const TinyWindow::vec2_t<uint16_t>& dimensions) override
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
		auto fxaaBlock = &bufferHandler.uniformBlocks["fxaaSettings"];
		fxaaBlock->SetPayload(FXAASettings_t());
		FXAA = fxaaBlock->GetPayload<FXAASettings_t>();
	}
};
#endif