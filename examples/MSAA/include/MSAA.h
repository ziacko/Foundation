#ifndef TEMPORALAA_H
#define TEMPORALAA_H

#include "scene3D.h"
#include "FrameBuffer.h"

class MSAA final : public scene3D
{
public:

	explicit MSAA(
		const char* windowName = "Ziyad Barakat's portfolio (MSAA)",
		const camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 5.0f, camera_t::projection_e::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		const model_t model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

		geometryBuffer = frameBuffer();
		this->camera.position.y -= 100.0f;

		//soulspear is loaded at an awkward angle so let's hack this
		//this->camera.Roll(glm::radians(270.0f));
		//this->camera.Pitch(glm::radians(180.0f));
	}

	~MSAA() override {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		geometryBuffer.Initialize();
		geometryBuffer.Bind();

		FBODescriptor colorDesc;
		colorDesc.target = GL_TEXTURE_2D_MULTISAMPLE;
		colorDesc.dataType = GL_FLOAT;
		colorDesc.sampleCount = 8;
		colorDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		FBODescriptor depthDesc;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.target = GL_TEXTURE_2D_MULTISAMPLE;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.internalFormat = GL_DEPTH_COMPONENT32F;
		depthDesc.sampleCount = 8;
		depthDesc.attachmentType = FBODescriptor::attachmentType_e::depth;
		depthDesc.dimensions = glm::ivec3(window->GetSettings().resolution.width, window->GetSettings().resolution.height, 1);

		geometryBuffer.AddAttachment(frameBuffer::attachment_t("color", colorDesc));
		geometryBuffer.AddAttachment(frameBuffer::attachment_t("depth", depthDesc));

		frameBuffer::Unbind();

		defProgram = shaderProgramsMap["geometryProgram"];
		finalProgram = shaderProgramsMap["MSAA"];
	}

protected:

	frameBuffer geometryBuffer;
	shaderProgram_t finalProgram;

	bool enableCompare = true;

	virtual void Draw() override
	{
		camera.ChangeProjection(camera_t::projection_e::perspective);
		camera.Update();

		UpdateDefaultUniforms(camera, clock);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GeometryPass();
		glDisable(GL_BLEND);

		camera.ChangeProjection(camera_t::projection_e::orthographic);
		UpdateDefaultUniforms(camera, clock);

		auto finalTexture = geometryBuffer.attachments["color"];

		FinalPass(&finalTexture);
	}

	virtual void GeometryPass()
	{
		glEnable(GL_SAMPLE_SHADING);
		glMinSampleShading(1.0f);
		geometryBuffer.Bind();

		geometryBuffer.attachments["color"].Draw();

		for (size_t iter = 0; iter < testModel.meshes.size(); iter++)
		{
			if (testModel.meshes[iter].isCollision)
			{
				continue;
			}

			testModel.meshes[iter].textures[0].SetActive(0);

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

		glDisable(GL_SAMPLE_SHADING);
		geometryBuffer.Unbind();
	}

	void FinalPass(texture* tex1) const
	{
		//draw directly to backbuffer
		tex1->SetActive(0);
		
		defaultVertexBuffer.Bind();
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		finalProgram.Use();

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	virtual void ClearBuffers() override
	{
		//ok copy the current frame into the previous frame and clear the rest of the buffers
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

	virtual void InitializeUniforms() override
	{
		scene3D::InitializeUniforms();
		defaultVertexBuffer.SetupDefault();
	}
};
#endif