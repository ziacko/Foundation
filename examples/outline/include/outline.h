#ifndef OUTLINE_H
#define OUTLINE_H
#include "stencil.h"
#include "FrameBuffer.h"

struct outlineSettings_t
{
	float uvScale;

	outlineSettings_t(float uvScale = 1.005f)
	{
		this->uvScale = uvScale;
	}

	~outlineSettings_t() {};
};

class outline : public stencil
{
public:

	explicit outline(const char* windowName = "Ziyad Barakat's Portfolio(outline)",
		camera_t camera3D = camera_t(glm::vec2(1280, 720), 10.0f, camera_t::projection_e::perspective, 0.001f, 1000.0f),
		model_t model = model_t("models/fbx_foliage/broadleaf_field/Broadleaf_Desktop_Field.FBX"),
		const char* shaderConfigPath = SHADER_CONFIG_DIR) :
		stencil(windowName, camera3D, model, shaderConfigPath)
	{

	}

	~outline() override {};

protected:

	//bufferHandler_t<outlineSettings_t> outlineBuffer;

	virtual void InitializeUniforms() override
	{
		stencil::InitializeUniforms();
	}

	virtual void Update() override
	{
		stencil::Update();
	}

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		stencil::BuildGUI(window, io);
	}

	virtual void StencilPass()
	{
		GL_PUSH_DEBUG_GROUP();
		geometryBuffer->Bind();
		//enable stencils, 
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0x01);
		glStencilFunc(GL_ALWAYS, 5, 0x01);

		geometryBuffer->attachments["depth"].Draw();

		//we just need the first LOd so only do the first 3 meshes
		for (size_t iter = 0; iter < 1; iter++)
		{
			if (testModel.meshes[iter].isCollision)
			{
				continue;
			}

			testModel.meshes[iter].textures[0].SetActive(0);
			//add the previous depth?

			glBindVertexArray(testModel.meshes[iter].vertexArrayHandle);
			DepthStencilProgram.Use();
			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, testModel.meshes[iter].indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		geometryBuffer->Unbind();
		glPopDebugGroup();
	}
};

#endif

