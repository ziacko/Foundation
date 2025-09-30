#ifndef DEPTH_PRE_PASS_H
#define DEPTH_PRE_PASS_H

#include "scene3D.h"
#include "FrameBuffer.h"

class depthPrePassScene final : public scene3D
{
public:
	explicit depthPrePassScene(
		const char* windowName = "Ziyad Barakat's portfolio (early depth test)",
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), defaultCameraSpeed * 0.1f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		const model_t model = model_t("models/SoulSpear/SoulSpear.fbx"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

		//this->camera.Roll(glm::radians(270.0f));

		geometryBuffer = frameBuffer();

	//	this->camera.position.y -= 2.0f;
		//this->camera.position.z = -1.0f;
	}

	~depthPrePassScene() override  = default;

	virtual void Initialize() override
	{
		scene3D::Initialize();

		geometryBuffer.Initialize();
		geometryBuffer.Bind();

		FBODescriptor depthDesc;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.mipmapLevels = 8;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer.AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer.AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		frameBuffer::Unbind();

		defProgram = shaderProgramsMap["geometry"];
		earlyDepthProgram = shaderProgramsMap["earlyDepth"];
		compareProgram = shaderProgramsMap["compare"];
		finalProgram = shaderProgramsMap["final"];
	}

protected:

	frameBuffer geometryBuffer;

	shaderProgram_t earlyDepthProgram;
	shaderProgram_t finalProgram;
	shaderProgram_t compareProgram;

	bool enableCompare = true;

	void Draw() override
	{
		camera.ChangeProjection(camera_t::projection_e::perspective);
		camera.Update();
		UpdateDefaultUniforms(camera, clock);

		EarlyDepthPass();
		GeometryPass(); //render current scene with jitter

		camera.resolution = glm::vec2(window->GetSettings().resolution.x, window->GetSettings().resolution.y);
		camera.ChangeProjection(camera_t::projection_e::orthographic);
		camera.Update();
		UpdateDefaultUniforms(camera, clock);

		FinalPass(geometryBuffer.attachments["color"], geometryBuffer.attachments["depth"]);
	}

	virtual void EarlyDepthPass()
	{
		geometryBuffer.Bind();

		geometryBuffer.attachments["depth"].Draw();

		//we just need the first LOd so only do the first 3 meshes
		for(auto iter : testModel.meshes)
		{
			if (iter.isCollision)
			{
				continue;
			}
			if (iter.textures.size() > 0)
			{
				iter.textures[0].SetActive(0);
			}
			//add the previous depth?

			glBindVertexArray(iter.vertexArrayHandle);
			earlyDepthProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, iter.indices.size(), GL_UNSIGNED_INT, 0);
			/*glDrawElementsBaseVertex(GL_TRIANGLES,
				iter.numIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * iter.indexOffset),
				iter.vertexOffset);*/
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer.attachments["depth"].SetActive(0);
		geometryBuffer.attachments["depth"].BindTexture();
		glGenerateMipmap(geometryBuffer.attachments["depth"].FBODesc.target);
		geometryBuffer.attachments["depth"].UnbindTexture();
		geometryBuffer.Unbind();
	}

	virtual void GeometryPass()
	{
		geometryBuffer.Bind();

		geometryBuffer.attachments["color"].Draw();

		//we just need the first LOd so only do the first 3 meshes
		for (auto iter : testModel.meshes)
		{
			if (iter.isCollision)
			{
				continue;
			}

			if (iter.textures.size() > 0)
			{
				iter.textures[0].SetActive(0);
			}

			//add the previous depth?
			glBindVertexArray(iter.vertexArrayHandle);
			defProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			//glCullFace(GL_BACK);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, iter.indices.size(), GL_UNSIGNED_INT, 0);
			//glDrawElements(GL_TRIANGLES, testModel->meshes[iter].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer.Unbind();
	}

	void FinalPass(texture& tex1, texture& tex2)
	{
		//draw directly to backbuffer		
		tex1.SetActive(0);

		defaultVertexBuffer.Bind();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		if (enableCompare)
		{
			tex2.SetActive(1);
			compareProgram.Use();
		}

		else
		{
			finalProgram.Use();
		}

		glDrawArrays(GL_TRIANGLES, 0, 6);
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
			for (auto iter : geometryBuffer.attachments | std::views::values)
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

		camera.ChangeProjection(camera_t::projection_e::perspective);
	}

	virtual void ResizeBuffers(const glm::ivec2& resolution)
	{
		geometryBuffer.Resize(glm::ivec3(resolution, 1));
	}

	virtual void HandleWindowResize(const tWindow* window, const TinyWindow::vec2_t<uint16_t>& dimensions) override
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