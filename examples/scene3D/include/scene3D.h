#ifndef SCENE3D_H
#define SCENE3D_H

#include "scene.h"
#include "Texture.h"
#include "Model.h"
#include "OBJLoader.h"

struct baseMaterialSettings_t
{
	glm::vec4								diffuse{};
	glm::vec4								specular{};
	glm::vec4								ambient{};
	glm::vec4								emissive{};
	glm::vec4								reflective{};

	baseMaterialSettings_t()
	{
		diffuse = glm::vec4(0.0f);
		specular = glm::vec4(0.0f);
		ambient = glm::vec4(0.0f);
		emissive = glm::vec4(0.0f);
		reflective = glm::vec4(0.0f);
	}
};

class scene3D : public scene
{
public:

	scene3D(const char* windowName = "Ziyad Barakat's Portfolio(3D scene)",
		camera_t camera3D = camera_t(glm::vec2(1280, 720), PI, camera_t::projection_e::perspective, 0.1f, 1000000.f),
		const char* shaderConfigPath = SHADER_CONFIG_DIR,
		model_t model = model_t("models/SoulSpear/SoulSpear.fbx")) :
		scene(windowName, camera3D, shaderConfigPath)
	{
		testModel = model;
		wireframe = false;
		glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	}

	virtual ~scene3D() {};

	//override input code. use this to mess with camera
	virtual void SetupCallbacks() override 
	{
		manager->resizeEvent = std::bind(&scene3D::HandleWindowResize, this, _1, _2);
		manager->maximizedEvent = std::bind(&scene3D::HandleMaximize, this, _1);
		//manager->destroyedEvent = std::bind(&scene::ShutDown, this, _1);

		manager->mouseButtonEvent = std::bind(&scene3D::HandleMouseClick, this, _1, _2, _3);
		manager->mouseMoveEvent = std::bind(&scene3D::HandleMouseMotion, this, _1, _2, _3);
		manager->keyEvent = std::bind(&scene3D::HandleKey, this, _1, _2, _3);
	}

	virtual void Initialize() override
	{
		scene::Initialize();
		testModel.loadModel();
		//model.Load();
		//for(size_t iter = 0; iter < testModel->meshes.size(); iter++)
		{
		/*	testModel->boneBuffer.Initialize(0, gl_shader_storage_buffer, gl_dynamic_draw);
			testModel->boneBuffer.Update(gl_shader_storage_buffer, gl_dynamic_draw,
				sizeof(glm::mat4) * testModel->boneBuffer.data.finalTransforms.size(),
				testModel->boneBuffer.data.finalTransforms.data());*/
		}
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		accum = 0.0f;
		accumReturn = 0.0f;
		accumMult = 0.0f;
	}

protected:

	model_t testModel;
	OBJModel model;
	bufferHandler_t<baseMaterialSettings_t>	materialBuffer;
	bool wireframe;

	unsigned int OGLProgram{};

	float accum{};
	float accumReturn{};
	float accumMult{};

	virtual void Draw() override 
	{
		for (const auto& iter : testModel.meshes)
		{
			glBindVertexArray(iter.vertexArrayHandle);
			glUseProgram(this->programGLID);

			glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}


			glDrawElements(GL_TRIANGLES, iter.indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		DrawGUI(window);

		manager->SwapDrawBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void Update() override
	{
		//this keeps resetting the values
		manager->PollForEvents();
		sceneCamera.Update();
		sceneClock.UpdateClockAdaptive();

		defaultPayload.data.deltaTime = (float)sceneClock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)sceneClock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / sceneClock.GetDeltaTime());
		defaultPayload.data.totalFrames++;

		defaultPayload.data.projection = sceneCamera.projection;
		defaultPayload.data.view = sceneCamera.view;
		if (sceneCamera.currentProjectionType == camera_t::projection_e::perspective)
		{
			defaultPayload.data.translation = glm::identity<glm::mat4>();
		}

		else
		{
			defaultPayload.data.translation = sceneCamera.translation;
		}
		
		defaultPayload.Update(gl_uniform_buffer, gl_dynamic_draw);
	}

	virtual void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene::BuildGUI(window, io);
		ImGui::Checkbox("wireframe", &wireframe);
		DrawCameraStats();
	}

	virtual void DrawCameraStats() override
	{
		//set up the view matrix
		ImGui::Begin("camera", &isGUIActive);

		ImGui::DragFloat("near plane", &sceneCamera.nearPlane, 0.1f, 0.00001f, 3.0f);
		ImGui::DragFloat("far plane", &sceneCamera.farPlane);
		ImGui::SliderFloat("Field of view", &sceneCamera.fieldOfView, 0, 90, "%.0f");

		ImGui::InputFloat("camera speed", &sceneCamera.speed);
		ImGui::InputFloat("x sensitivity", &sceneCamera.xSensitivity, 0.f);
		ImGui::InputFloat("y sensitivity", &sceneCamera.ySensitivity, 0.f);

		ImGui::Text("local up %f %f %f %f", sceneCamera.up.x, sceneCamera.up.y, sceneCamera.up.z, sceneCamera.up.w);
		ImGui::Text("local right %f %f %f %f", sceneCamera.right.x, sceneCamera.right.y, sceneCamera.right.z, sceneCamera.right.w);
		ImGui::Text("local forward %f %f %f %f", sceneCamera.forward.x, sceneCamera.forward.y, sceneCamera.forward.z, sceneCamera.forward.w);
		ImGui::End();
	}

	virtual void InitializeUniforms() override
	{
		defaultPayload.data = defaultUniformBuffer(sceneCamera);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		defaultPayload.data.resolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.projection = sceneCamera.projection;
		defaultPayload.data.translation = sceneCamera.translation;
		defaultPayload.data.view = sceneCamera.view;

		materialBuffer.data = baseMaterialSettings_t();

		defaultPayload.Initialize(0);
		materialBuffer.Initialize(1);
	}

	virtual void HandleMouseClick(const tWindow* window, mouseButton_t button, buttonState_t state) override
	{
		scene::HandleMouseClick(window, button, state);
	}

	virtual void HandleMouseMotion(const tWindow* window, vec2_t<int16_t> windowPosition, vec2_t<int16_t> screenPosition) override
	{
		scene3D* thisScene = (scene3D*)window->GetSettings().userData;
		scene::HandleMouseMotion(window, windowPosition, screenPosition);

		glm::vec2 mouseDelta = glm::vec2(window->GetMousePosition().x - window->GetPreviousMousePosition().x, window->GetMousePosition().y - window->GetPreviousMousePosition().y);
		float deltaTime = (float)thisScene->sceneClock.GetDeltaTime();

		if (window->GetMouseButtonState()[(int)mouseButton_t::right] == buttonState_t::down)
		{
			if (mouseDelta.x != 0)
			{
				sceneCamera.Yaw((float)((mouseDelta.x * sceneCamera.xSensitivity) * (1 - deltaTime)));
			}

			if (mouseDelta.y != 0)
			{
				sceneCamera.Pitch((float)((mouseDelta.y * sceneCamera.ySensitivity) * (1 - deltaTime)));
			}
		}
	}

	virtual void HandleMaximize(const tWindow* window) override
	{
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		sceneCamera.resolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.resolution = sceneCamera.resolution;
		sceneCamera.UpdateProjection();
		defaultPayload.data.projection = sceneCamera.projection;

		//bind the uniform buffer and refill it
		defaultPayload.Update(gl_uniform_buffer, gl_dynamic_draw);
	}

	virtual void HandleWindowResize(const tWindow* window, TinyWindow::vec2_t<uint16_t> dimensions) override
	{
		scene3D* thisScene = (scene3D*)window->GetSettings().userData;
		glViewport(0, 0, dimensions.width, dimensions.height);
		sceneCamera.resolution = glm::vec2(dimensions.width, dimensions.height);
		defaultPayload.data.resolution = sceneCamera.resolution;
		sceneCamera.UpdateProjection();
		defaultPayload.data.projection = sceneCamera.projection;
		defaultPayload.data.deltaTime = (float)sceneClock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)sceneClock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / sceneClock.GetDeltaTime());

		defaultPayload.Update(gl_uniform_buffer, gl_dynamic_draw);
	}

	virtual void HandleKey(const tWindow* window, const uint16_t key, const keyState_t state)	override
	{
		auto it = windowContextMap.find(const_cast<tWindow*>(window));
		if (it != windowContextMap.end())
		{
			ImGui::SetCurrentContext(it->second);
		}
		ImGuiIO& io = ImGui::GetIO();


		if (state == keyState_t::down)
		{
			io.KeysData[key].Down = true;
			io.AddInputCharacter(key);
		}

		else

		{
			io.KeysData[key].Down = false;
		}
		float camSpeed = 0.0f;
		if (key == TinyWindow::key_t::leftShift && state == keyState_t::down)
		{
			camSpeed = sceneCamera.speed * 2;
		}
		
		else
		{
			camSpeed = sceneCamera.speed;
		}

		float deltaTime = (float)sceneClock.GetDeltaTime();

		if (state == keyState_t::down) //instead of one key could we check multiple keys?
		{
			if(window->GetKeyState()['w'] == keyState_t::down)
			{
				sceneCamera.MoveForward(camSpeed, deltaTime);
			}

			if (window->GetKeyState()['s'] == keyState_t::down)
			{
				sceneCamera.MoveForward(-camSpeed, deltaTime);
			}

			if (window->GetKeyState()['a'] == keyState_t::down)
			{
				sceneCamera.MoveRight(-camSpeed, deltaTime);
			}

			if (window->GetKeyState()['d'] == keyState_t::down)
			{
				sceneCamera.MoveRight(camSpeed, deltaTime);
			}

			if (window->GetKeyState()['e'] == keyState_t::down)
			{
				sceneCamera.MoveUp(camSpeed, deltaTime);
			}

			if (window->GetKeyState()['q'] == keyState_t::down)
			{
				sceneCamera.MoveUp(-camSpeed, deltaTime);
			}

			if (window->GetKeyState()['z'] == keyState_t::down)
			{
				sceneCamera.Roll(glm::radians((float)sceneCamera.zSensitivity * deltaTime));
			}

			if (window->GetKeyState()['x'] == keyState_t::down)
			{
				sceneCamera.Roll(glm::radians((float)-sceneCamera.zSensitivity * deltaTime));
			}
		}
	}
	
};

#endif

