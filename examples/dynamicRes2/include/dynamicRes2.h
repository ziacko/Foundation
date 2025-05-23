#ifndef DYNAMIC_RES2_H
#define DYNAMIC_RES2_H

#include "DynamicRes.h"

class dynamicRes2 : public dynamicRes
{
public:

	dynamicRes2(
		const char* windowName = "Ziyad Barakat's portfolio (dynamic resolution2)",
		camera* texModelCamera = new camera(glm::vec2(1280, 720), 5.0f, camera::projection_t::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = "../../resources/shaders/DynamicRes2.txt",
		model_t* model = new model_t("../../resources/models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: dynamicRes(windowName, texModelCamera, shaderConfigPath, model)
	{

	}

	~dynamicRes2() {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		FBODescriptor depthDesc;
		depthDesc.target = GL_TEXTURE_2D;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = gl_depth_component24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_t::depth;
		depthDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height, 1);

		FBODescriptor colorDesc;
		colorDesc.magFilterSetting = GL_NEAREST;
		colorDesc.minFilterSetting = GL_LINEAR;
		colorDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height, 1);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("depth", depthDesc));

		frameBuffer::Unbind();

		//DynamicResProgram = shaderPrograms[1]->handle;
		compareProgram = shaderPrograms[1]->handle;
		finalProgram = shaderPrograms[2]->handle;
		res = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) * resolution.data.resPercent;
	}

protected:

	void DrawResolutionSettings() override
	{
		ImGui::Begin("resolution Settings");
		if (ImGui::SliderFloat("horizontal \%", &resolution.data.resPercent.x, 0.3f, 1.0f, "%.2f") ||
			ImGui::SliderFloat("vertical \%", &resolution.data.resPercent.y, 0.3f, 1.0f, "%.2f"))
		{
			res = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) * resolution.data.resPercent;
			defaultVertexBuffer->UpdateBuffer(res);
		}

		ImGui::End();
	}

	virtual void GeometryPass() override
	{
		geometryBuffer->Bind();

		GLenum drawbuffers[1] = {
			geometryBuffer->attachments[0]->FBODesc.attachmentFormat, //color
		};

		glDrawBuffers(1, drawbuffers);

		//we just need the first LOd so only do the first 3 meshes
		for (size_t iter = 0; iter < 3; iter++)
		{
			if (testModel->meshes[iter].isCollision)
			{
				continue;
			}

			testModel->meshes[iter].textures[0].SetActive(0);
			//add the previous depth?

			glBindVertexArray(testModel->meshes[iter].vertexArrayHandle);
			glUseProgram(programGLID);
			
			glViewport(0, 0, res.x, res.y);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel->meshes[iter].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		geometryBuffer->Unbind();
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		//res = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height) * this->resolution.data.resPercent;
		for (auto iter : geometryBuffer->attachments)
		{
			iter->Resize(glm::ivec3(resolution, 1));
		}
	}
};
#endif