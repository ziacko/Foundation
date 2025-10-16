#ifndef STENCIL_H
#define STENCIL_H
#include "scene3D.h"
#include "FrameBuffer.h"

class stencil : public scene3D
{
public:

	stencil(const char* windowName = "Ziyad Barakat's Portfolio(stencil test)",
		camera_t camera3D = camera_t(defaultWindowSize, defaultCameraSpeed * 0.1f, camera_t::projection_e::perspective ),
		model_t model = model_t("models/SoulSpear/SoulSpear.fbx"),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) : scene3D(windowName, camera3D, shaderConfigPath)
	{
		testModel = model;
		this->camera.position.y -= 2.0f;
		this->camera.position.z -= 3.0f;
		geometryBuffer = new frameBuffer();
	}

	~stencil() {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		FBODescriptor depthDesc;
		depthDesc.dataType = GL_UNSIGNED_INT_24_8;
		depthDesc.format = GL_DEPTH_STENCIL;
		depthDesc.internalFormat = GL_DEPTH24_STENCIL8;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depthAndStencil;
		depthDesc.wrapRSetting = GL_CLAMP;
		depthDesc.wrapSSetting = GL_CLAMP;
		depthDesc.wrapTSetting = GL_CLAMP;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		FBODescriptor stencilDesc;
		stencilDesc.dataType = GL_UNSIGNED_INT;
		stencilDesc.format = GL_STENCIL_INDEX;
		stencilDesc.internalFormat = GL_STENCIL_INDEX8;
		stencilDesc.attachmentType = FBODescriptor::attachmentType_e::stencil;
		stencilDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer->AddAttachment(frameBuffer::attachment_t("depth", depthDesc));
	
		frameBuffer::Unbind();

		defProgram = shaderProgramsMap["geometry"];
		DepthStencilProgram = shaderProgramsMap["earlyDepth"];
		compareProgram = shaderProgramsMap["compare"];
		finalProgram = shaderProgramsMap["final"];
	}

protected:

	frameBuffer* geometryBuffer;

	shaderProgram_t DepthStencilProgram;
	shaderProgram_t finalProgram;
	shaderProgram_t compareProgram;

	bool enableCompare = true;

	void Draw() override
	{
		camera.ChangeProjection(camera_t::projection_e::perspective);
		camera.Update();

		UpdateDefaultUniforms(camera, clock, &testModel);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		EarlyDepthPass();

		GeometryPass(); //render current scene with jitter
		glDisable(GL_BLEND);

		camera.ChangeProjection(camera_t::projection_e::orthographic);
		UpdateDefaultUniforms(camera, clock, &testModel);

		FinalPass(&geometryBuffer->attachments["color"], &geometryBuffer->attachments["depth"]);
	}

	virtual void EarlyDepthPass()
	{
		GL_PUSH_DEBUG_GROUP();
		geometryBuffer->Bind();
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 5, 0xFF);
		//glStencilFuncSeparate(GL_FRONT, GL_ALWAYS, 1, 0xff);
		
		geometryBuffer->attachments["depth"].Draw();

		//we just need the first LOd so only do the first 3 meshes
		for (auto mesh : testModel.meshes)
		{
			if (mesh.isCollision)
			{
				continue;
			}

			mesh.textures[0].SetActive(0);
			mesh.Bind();
			DepthStencilProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, (GLsizei)mesh.indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();

		glPopDebugGroup();
	}

	virtual void GeometryPass()
	{
		GL_PUSH_DEBUG_GROUP();
		geometryBuffer->Bind();
		geometryBuffer->attachments["color"].Draw();

		//we just need the first LOd so only do the first 3 meshes
		for (auto mesh : testModel.meshes)
		{
			if (mesh.isCollision)
			{
				continue;
			}

			mesh.textures[0].SetActive(0);
			//add the previous depth?

			mesh.Bind();
			defProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			//glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, (GLsizei)mesh.indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();
		glPopDebugGroup();
	}

	virtual void FinalPass(texture* tex1, frameBuffer::attachment_t* tex2)
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
	}

	virtual void DrawBufferAttachments()
	{
		if (ImGui::BeginTabItem("framebuffers"))
		{
			ImGui::Checkbox("enable Compare", &enableCompare);
			for (auto iter : geometryBuffer->attachments | std::views::values)
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
		geometryBuffer->Bind();
		geometryBuffer->ClearTexture(geometryBuffer->attachments["color"], clearColor);
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		geometryBuffer->Unbind();

		camera.ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		geometryBuffer->Resize(glm::ivec3(resolution, 1));
	}

	virtual void HandleWindowResize(const tWindow* window, const tw::vec2_t<uint16_t>& dimensions) override
	{
		defaultPayload->resolution = glm::vec2(dimensions.width, dimensions.height);
		ResizeBuffers(glm::ivec2(dimensions.x, dimensions.y));
	}

	virtual void HandleMaximize(const tWindow* window) override
	{
		defaultPayload->resolution = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		ResizeBuffers(defaultPayload->resolution);
	}
};

#endif

