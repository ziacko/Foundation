#ifndef TRANSFORMS_H
#define TRANSFORMS_H
#include "Scene3D.h"
#include "ImGuizmo.h"
#include "Transform.h"


struct transformSettings_t
{
	glm::mat4 position;
	glm::mat4 rotation;
	glm::mat4 scale;
	glm::vec3 eulerAngles; //could be useful

	transformSettings_t()
	{
		position = glm::mat4(1.0f);
		rotation = glm::mat4(1.0f);
		scale = glm::mat4(1.0f);
		eulerAngles = glm::vec3(0.0f);
	}

	transformSettings_t(transform_t* newTransform)
	{
		position = glm::translate(glm::mat4(1.0f), glm::vec3(newTransform->position));
		rotation = glm::toMat4(newTransform->rotation);
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(newTransform->scale));
		eulerAngles = glm::eulerAngles(newTransform->rotation);
	}

};

class transformScene : public scene3D
{
public:

	transformScene(
		const char* windowName = "Ziyad Barakat's portfolio (transforms)",
		camera_t* texModelCamera = new camera_t(glm::vec2(1280, 720), 1.0f, camera_t::projection_e::perspective, 0.1f, 1000000.f),
		const char* shaderConfigPath = "../../resources/shaders/Transforms.txt",
		model_t* model = new model_t("../../resources/models/SoulSpear/SoulSpear.FBX"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{

		transformUniforms = bufferHandler_t<transformSettings_t>();
		transContainer = transformContainer();

		first = transContainer.AddTransform();
		first->name = "first";
		second = transContainer.AddTransform();
		second->name = "second";
		third = transContainer.AddTransform();
		third->name = "third";

		




		second->SetParent(first);
	}

	virtual void Initialize() override
	{
		scene3D::Initialize();

		second->SetPosition(glm::vec4(10, 0, 0, 1));
		second->SetLocalPosition(second->position);
		third->SetPosition(glm::vec4(-10, 0, 0, 1));
		//second = new model_t();
		//third = new model_t();

		//maybe just draw the same geometry three times?


	}

	~transformScene(){};

	bufferHandler_t<transformSettings_t> transformUniforms;

protected:

	int currentTexture = 0;
	std::vector<const char*> textureNames;

	transformContainer transContainer;
	transform_t* first;
	transform_t* second;
	transform_t* third;

	model_t secondModel;
	model_t thirdModel;

	virtual void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		ImGui::Begin(window->settings.name, &isGUIActive);
		scene::BuildGUI(window, io);

		scene3D::DrawCameraStats();
	}

	virtual void BuildTransformStuff(tWindow* window)
	{
		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(true);
		ImGuizmo::Enable(true);
		ImGuizmo::SetRect(0, 0, window->settings.resolution.x, window->settings.resolution.y);

		//are Z and Y swapped?
		//glm::quat gridRotation(glm::radians(glm::vec3(90, 0, 0)));
		//glm::mat4 gridMat = glm::mat4(1.0f) * glm::toMat4(gridRotation);

		glm::mat4 view = sceneCamera->view;
		glm::mat4 projection = sceneCamera->projection;
		
		//why does rotating break this
		ImGuizmo::DrawGrid((const float*)&view, (const float*)&projection, glm::value_ptr(glm::mat4(1.0f)), 10.0f);

		ImGuizmo::SetID(0);
		TransformRedundancy(first, sceneCamera);
		//ImGuizmo::SetID(1);
		//TransformRedundancy(second, sceneCamera);
		//ImGuizmo::SetID(2);
		//TransformRedundancy(third, sceneCamera);

		DrawTransformData(first);
		DrawTransformData(second);
	}

	void DrawTransformData(transform_t* trans)
	{
		ImGui::Begin(trans->name.c_str());
		ImGui::DragFloat4("world position:", (float*)&trans->position, 0.1f, -100.0f, 100.0f);
		ImGui::DragFloat4("local position:", (float*)&trans->localPosition, 0.1f, -100.0f, 100.0f);

		if (ImGui::CollapsingHeader("local to world", NULL))
		{
			ImGui::DragFloat4("row 0", (float*)&trans->localToWorldMatrix[0], 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat4("row 1", (float*)&trans->localToWorldMatrix[1], 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat4("row 2", (float*)&trans->localToWorldMatrix[2], 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat4("row 3", (float*)&trans->localToWorldMatrix[3], 0.1f, -100.0f, 100.0f);
		}

		if (ImGui::CollapsingHeader("world to local", NULL))
		{
			ImGui::DragFloat4("row 0", (float*)&trans->transform[0], 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat4("row 1", (float*)&trans->transform[1], 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat4("row 2", (float*)&trans->transform[2], 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat4("row 3", (float*)&trans->transform[3], 0.1f, -100.0f, 100.0f);
		}

		ImGui::End();

	}

	virtual void TransformRedundancy(transform_t* toDraw, camera_t* cam)
	{
		ImGuizmo::BeginFrame();
		toDraw->mouseOver = ImGuizmo::IsOver();
		
		glm::mat4 objectMatrix = glm::mat4(1.0f);
		glm::mat4 position = glm::translate(glm::mat4(1.0f), glm::vec3(toDraw->position));
		glm::mat4 rotation = glm::mat4_cast(glm::quat(glm::radians(toDraw->eulerAngles)));
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(toDraw->scale));

		objectMatrix = objectMatrix * position * rotation * scale;
		ImGuizmo::Manipulate((const float*)&cam->view, (const float*)&cam->projection, toDraw->gizmoOp, toDraw->gizmoMode, (float*)&objectMatrix);

		if (ImGuizmo::IsUsing())
		{
			float matrixTranslation[3], matrixRotation[3], matrixScale[3];
			ImGuizmo::DecomposeMatrixToComponents((float*)&objectMatrix, matrixTranslation, matrixRotation, matrixScale);

			toDraw->SetPosition(glm::vec4(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2], 1));
			toDraw->eulerAngles = glm::vec4(matrixRotation[0], matrixRotation[1], matrixRotation[2], 1);
			toDraw->SetRotation(glm::quat(glm::radians(toDraw->eulerAngles)));
			toDraw->SetScale(glm::vec4(matrixScale[0], matrixScale[1], matrixScale[2], 1));
		}
	}

	/*virtual void Update() override
	{
		scene3D::Update();

		
	}*/

	virtual void BeginGUI(tWindow* window) override
	{
		ImGUINewFrame(window);
		BuildTransformStuff(window); //this needs to happen before rendering begins at least
	}

	virtual void Draw() override
	{
		BeginGUI(windows[0]);
		ImGuiIO io = ImGui::GetIO();


		//replace with instanced rendering? no

		transformUniforms.data = transformSettings_t(first);
		transformUniforms.Update();
		DrawMeshes();
	
		transformUniforms.data = transformSettings_t(second);
		transformUniforms.Update();
		DrawMeshes();

		/*transformUniforms.data = transformSettings_t(third);
		transformUniforms.Update();
		DrawMeshes();*/


		BuildGUI(windows[0], io);
		EndGUI(windows[0]);

		windows[0]->SwapDrawBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void DrawMeshes()
	{
		for (auto iter : testModel->meshes)
		{
			if (iter.isCollision)
			{
				continue;
			}
			//set the materials per mesh
			materialBuffer.data.diffuse = iter.diffuse;
			materialBuffer.data.ambient = iter.ambient;
			materialBuffer.data.specular = iter.specular;
			materialBuffer.data.reflective = iter.reflective;
			materialBuffer.Update(gl_uniform_buffer, gl_dynamic_draw);

			//glBindBuffer(gl_element_array_buffer, iter.indexBufferHandle);
			glBindVertexArray(iter.vertexArrayHandle);
			glUseProgram(this->programGLID);

			glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);

			if (!iter.textures.empty())
			{
				iter.textures[currentTexture].SetActive();
			}

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			glDrawElements(GL_TRIANGLES, iter.indices.size(), GL_UNSIGNED_INT, nullptr);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	virtual void InitializeUniforms() override
	{
		scene3D::InitializeUniforms();
		transformUniforms.Initialize(1);
	}

	void KeyPerTransform(transform_t* trans, int key)
	{

			switch (key)
			{
			case 't':
			{
				trans->gizmoOp = ImGuizmo::TRANSLATE;
				break;
			}

			case 'r':
			{
				trans->gizmoOp = ImGuizmo::ROTATE;
				break;
			}

			case 'y':
			{
				trans->gizmoOp = ImGuizmo::SCALE;
				break;
			}

			case 'b':
			{
				trans->gizmoOp = ImGuizmo::BOUNDS;
				break;
			}

			case 'l':
			{
				trans->gizmoMode = ImGuizmo::LOCAL;
				break;
			}

			case 'g':
			{
				trans->gizmoMode = ImGuizmo::WORLD;
				break;
			}

			default:
				{
					break;
				}
			}
	}

	virtual void HandleKey(tWindow* window, int key, keyState_t state) override
	{
		scene3D::HandleKey(window, key, state);

		if (first->mouseOver)
		{
			KeyPerTransform(first, key);
		}

		if (second->mouseOver)
		{
			KeyPerTransform(second, key);
		}
		if (third->mouseOver)
		{
			KeyPerTransform(third, key);
		}

		if (key == ' ')
		{
			first->Translate(glm::vec3(0, 1, 0));
		}
	}
};

#endif