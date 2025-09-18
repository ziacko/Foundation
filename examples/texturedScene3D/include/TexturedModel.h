#ifndef TEXTUREDMODEL_H
#define TEXTUREDMODEL_H
#include "scene3D.h"

class texturedModel : public scene3D
{
public:

	texturedModel(
		const char* windowName = "Ziyad Barakat's portfolio (textured Model)",
		camera_t texModelCamera = camera_t(glm::vec2(1280, 720), 1.0f, camera_t::projection_e::perspective, 0.1f, 1000000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR)
		: scene3D(windowName, texModelCamera, shaderConfigPath)
	{

	}

	~texturedModel(){};


protected:

	int currentTexture = 0;
	std::vector<const char*> textureNames;

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene3D::BuildGUI(window, io);
		ImGui::Begin("textures");
		if (ImGui::SliderInt("active index:", &currentTexture, 0, testModel.meshes[0].textures.size() - 1))
		{
			testModel.meshes[0].textures[currentTexture].UnbindTexture();
			testModel.meshes[0].textures[currentTexture].BindTexture();
			testModel.meshes[0].textures[currentTexture].SetActive(0);
		}
		ImGui::End();
	}

	virtual void Draw()
	{
		DrawMeshes();
	
		DrawGUI(window);

		manager->SwapDrawBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void DrawMeshes()
	{
		for (const auto& iter : testModel.meshes)
		{
			glBindVertexArray(iter.vertexArrayHandle);
			glUseProgram(defProgram.handle);

			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}

			if (!iter.textures.empty())
			{
				iter.textures[0].SetActive();
			}

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}

			//glLineWidth(10.0f);
			//glPointSize(10.0f);
			//glDrawArrays(GL_TRIANGLES, 0, iter.vertices.size());
			glDrawElements(GL_TRIANGLES, iter.indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
};

#endif