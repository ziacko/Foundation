#ifndef SCENE3D_H
#define SCENE3D_H

#include "scene.h"

struct baseMaterialSettings_t
{
	glm::vec4	diffuse;
	glm::vec4	specular;
	glm::vec4	ambient;
	glm::vec4	emissive;
	glm::vec4	reflective;

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
	explicit scene3D(const char* windowName = "Ziyad Barakat's Portfolio(3D scene)",
			const camera_t& camera3D = camera_t(defaultWindowSize, defaultCameraSpeed, camera_t::projection_e::perspective),
	        const char* shaderConfigPath = SHADER_CONFIG_DIR,
	        const model_t& model = model_t("models/SoulSpear/SoulSpear.fbx")) :
		scene(windowName, camera3D, shaderConfigPath)
	{
		testModel = model;
		wireframe = false;
		glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	}

	//override input code. use this to mess with camera
	void SetupCallbacks() override
	{
		manager->resizeEvent = std::bind(&scene3D::HandleWindowResize, this, _1, _2);
		manager->maximizedEvent = std::bind(&scene3D::HandleMaximize, this, _1);
		//manager->destroyedEvent = std::bind(&scene::ShutDown, this, _1);

		manager->mouseWheelEvent = std::bind(&scene3D::HandleMouseWheel, this, _1, _2);
		manager->mouseButtonEvent = std::bind(&scene3D::HandleMouseClick, this, _1, _2, _3);
		manager->mouseMoveEvent = std::bind(&scene3D::HandleMouseMotion, this, _1, _2, _3);
		manager->keyEvent = std::bind(&scene3D::HandleKey, this, _1, _2, _3);
	}

	void Initialize() override
	{
		scene::Initialize();
		testModel.loadModel();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		accum = 0.0f;
		accumReturn = 0.0f;
		accumMult = 0.0f;
	}

protected:

	model_t testModel;
	bufferHandler_t<baseMaterialSettings_t>	materialBuffer;

	unsigned int OGLProgram{};

	float accum{};
	float accumReturn{};
	float accumMult{};

	void Draw() override
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

			glDrawElements(GL_TRIANGLES, iter.indices.size(), GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		DrawGUI(window);

		manager->SwapDrawBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Update() override
	{
		//this keeps resetting the values
		manager->PollForEvents();
		camera.Update();
		clock.UpdateClockAdaptive();

		defaultPayload.data.deltaTime = (float)clock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)clock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / clock.GetDeltaTime());
		defaultPayload.data.totalFrames++;

		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.view = camera.view;
		if (camera.currentProjectionType == camera_t::projection_e::perspective)
		{
			defaultPayload.data.translation = glm::identity<glm::mat4>();
		}

		else
		{
			defaultPayload.data.translation = camera.translation;
		}

		defaultPayload.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	}

	void BuildGUI(tWindow* window, const ImGuiIO& io) override
	{
		scene::BuildGUI(window, io);
		DrawCameraStats();
	}

	void DrawCameraStats() override
	{
		//set up the view matrix
		if (ImGui::BeginTabItem("Camera"))
		{
			ImGui::SliderFloat("near plane", &camera.nearPlane, defaultNearPlane, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
			ImGui::SliderFloat("far plane", &camera.farPlane, 0, defaultFarPlane, "%.3f", ImGuiSliderFlags_NoInput);
			ImGui::SliderFloat("Field of view", &camera.fieldOfView, 0, defaultFieldOfView, "%.3f", ImGuiSliderFlags_NoInput);

			ImGui::InputFloat("camera speed", &camera.speed, 0.01f);
			ImGui::InputFloat("x sensitivity", &camera.xSensitivity, 0.f);
			ImGui::InputFloat("y sensitivity", &camera.ySensitivity, 0.f);

			ImGui::Text("local up %f %f %f %f", camera.up.x, camera.up.y, camera.up.z, camera.up.w);
			ImGui::Text("local right %f %f %f %f", camera.right.x, camera.right.y, camera.right.z, camera.right.w);
			ImGui::Text("local forward %f %f %f %f", camera.forward.x, camera.forward.y, camera.forward.z, camera.forward.w);

			ImGui::EndTabItem();
		}
	}

	void InitializeUniforms() override
	{
		defaultPayload.data = defaultUniformBuffer(camera);
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);

		defaultPayload.data.resolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.translation = camera.translation;
		defaultPayload.data.view = camera.view;

		materialBuffer.data = baseMaterialSettings_t();

		defaultPayload.Initialize(0);
		materialBuffer.Initialize(1);
	}

	void HandleMouseClick(const tWindow* window, mouseButton_e button, buttonState_e state) override
	{
		scene::HandleMouseClick(window, button, state);
	}

	void HandleMouseMotion(const tWindow* window, const vec2_t<int16_t>& windowPosition, const vec2_t<int16_t>& screenPosition) override
	{
		scene3D* thisScene = (scene3D*)window->GetSettings().userData;
		scene::HandleMouseMotion(window, windowPosition, screenPosition);

		const glm::vec2 mouseDelta = glm::vec2(window->GetMousePosition().x - window->GetPreviousMousePosition().x, window->GetMousePosition().y - window->GetPreviousMousePosition().y);
		const float deltaTime = (float)thisScene->clock.GetDeltaTime();

		if (window->GetMouseButtonState()[(int)mouseButton_e::right] == buttonState_e::down)
		{
			if (mouseDelta.x != 0)
			{
				camera.Roll((-mouseDelta.x * camera.xSensitivity) * (1 - deltaTime));
			}

			if (mouseDelta.y != 0)
			{
				camera.Pitch((mouseDelta.y * camera.ySensitivity) * (1 - deltaTime));
			}
		}
	}

	void HandleMaximize(const tWindow* window) override
	{
		glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		camera.resolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.resolution = camera.resolution;
		camera.UpdateProjection();
		defaultPayload.data.projection = camera.projection;

		//bind the uniform buffer and refill it
		defaultPayload.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	}

	void HandleWindowResize(const tWindow* window, const vec2_t<uint16_t>& dimensions) override
	{
		glViewport(0, 0, dimensions.width, dimensions.height);
		camera.resolution = glm::vec2(dimensions.width, dimensions.height);
		defaultPayload.data.resolution = camera.resolution;
		camera.UpdateProjection();
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.deltaTime = (float)clock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)clock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / clock.GetDeltaTime());

		defaultPayload.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	}

	void HandleKey(const tWindow* window, const int16_t& key, const keyState_e& state)	override
	{
		auto it = windowContextMap.find(const_cast<tWindow*>(window));
		if (it != windowContextMap.end())
		{
			ImGui::SetCurrentContext(it->second);
		}
		ImGuiIO& io = ImGui::GetIO();


		if (state == keyState_e::down)
		{
			io.KeysData[key].Down = true;
			io.AddInputCharacter(key);
		}

		else

		{
			io.KeysData[key].Down = false;
		}
		float camSpeed = 0.0f;
		if (key == TinyWindow::key_e::leftShift && state == keyState_e::down)
		{
			camSpeed = camera.speed * 2;
		}

		else
		{
			camSpeed = camera.speed;
		}

		const float deltaTime = (float)clock.GetDeltaTime();

		if (state == keyState_e::down) //instead of one key could we check multiple keys?
		{
			if(window->GetKeyState()['w'] == keyState_e::down)
			{
				camera.MoveForward(camSpeed, deltaTime);
			}

			if (window->GetKeyState()['s'] == keyState_e::down)
			{
				camera.MoveForward(-camSpeed, deltaTime);
			}

			if (window->GetKeyState()['a'] == keyState_e::down)
			{
				camera.MoveRight(-camSpeed, deltaTime);
			}

			if (window->GetKeyState()['d'] == keyState_e::down)
			{
				camera.MoveRight(camSpeed, deltaTime);
			}

			if (window->GetKeyState()['e'] == keyState_e::down)
			{
				camera.MoveUp(camSpeed, deltaTime);
			}

			if (window->GetKeyState()['q'] == keyState_e::down)
			{
				camera.MoveUp(-camSpeed, deltaTime);
			}

			if (window->GetKeyState()['z'] == keyState_e::down)
			{
				camera.Roll(glm::radians(camera.zSensitivity * deltaTime));
			}

			if (window->GetKeyState()['x'] == keyState_e::down)
			{
				camera.Roll(glm::radians(camera.zSensitivity * deltaTime));
			}
		}
	}
};

#endif

