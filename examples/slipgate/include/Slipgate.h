#ifndef SLIPGATE_H
#define SLIPGATE_H

#include "Scene3D.h"

#include "ImGuizmo.h"

#include "FrameBuffer.h"
#include "Transform.h"

struct divideTest_t final
{
	GLfloat testFloat;

	divideTest_t()
	{
		testFloat = 1.0f;
	}
};

class slipgateTest : public scene3D
{
public:

	slipgateTest(
		const char* windowName = "Slipgate test",
		camera* texModelCamera = new camera(glm::vec2(1280, 720), 0.2f, camera::projection_t::perspective, 0.1f, 2000.f),
		const char* shaderConfigPath = "../../resources/shaders/Slipgate.txt",
		model_t* model = new model_t("../../resources/models/SoulSpear/SoulSpear.FBX"))
		: scene3D(windowName, texModelCamera, shaderConfigPath, model)
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glHint(gl_generate_mipmap_hint, GL_NICEST);

		geometryBuffer = new frameBuffer();
	}

	virtual ~slipgateTest() {};

	virtual void Initialize() override
	{
		scene3D::Initialize();

		FBODescriptor colorDesc;
		colorDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height, 1);

		geometryBuffer->Initialize();
		geometryBuffer->Bind();

		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("color", colorDesc));

		FBODescriptor depthDesc;
		depthDesc.dataType = GL_FLOAT;
		depthDesc.format = GL_DEPTH_COMPONENT;
		depthDesc.internalFormat = gl_depth_component24;
		depthDesc.attachmentType = FBODescriptor::attachmentType_t::depth;
		depthDesc.dimensions = glm::ivec3(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height, 1);

		geometryBuffer->AddAttachment(new frameBuffer::attachment_t("depth", depthDesc));

		frameBuffer::Unbind();

		//geometry automatically gets assigned to 0
		finalProgram = shaderPrograms[1]->handle;
		ComputeProgram = shaderPrograms[2]->handle;

		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
	}

protected:

	frameBuffer* geometryBuffer;

	bufferHandler_t<divideTest_t> testBuffer;

	unsigned int finalProgram = 0;
	unsigned int ComputeProgram = 0;

	virtual void Update() override
	{
		manager->PollForEvents();
		if (lockedFrameRate > 0)
		{
			sceneClock->UpdateClockFixed(lockedFrameRate);
		}
		else
		{
			sceneClock->UpdateClockAdaptive();
		}

		defaultPayload.data.deltaTime = (float)sceneClock->GetDeltaTime();
		defaultPayload.data.totalTime = (float)sceneClock->GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / sceneClock->GetDeltaTime());
		defaultPayload.data.totalFrames++;

		testBuffer.Update();
	}

	void UpdateDefaultBuffer()
	{
		sceneCamera->UpdateProjection();
		defaultPayload.data.projection = sceneCamera->projection;
		defaultPayload.data.view = sceneCamera->view;
		if (sceneCamera->currentProjectionType == camera::projection_t::perspective)
		{
			defaultPayload.data.translation = testModel->makeTransform();
		}

		else
		{
			defaultPayload.data.translation = sceneCamera->translation;
		}
		defaultPayload.data.deltaTime = (float)sceneClock->GetDeltaTime();
		defaultPayload.data.totalTime = (float)sceneClock->GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / sceneClock->GetDeltaTime());

		defaultPayload.Update();
		defaultVertexBuffer->UpdateBuffer(defaultPayload.data.resolution);
	}

	virtual void Draw()
	{
		sceneCamera->ChangeProjection(camera::projection_t::perspective);
		sceneCamera->Update();

		UpdateDefaultBuffer();

		defaultTimer->Begin();
		GeometryPass(); //render current scene with jitter
		defaultTimer->End();

		sceneCamera->ChangeProjection(camera::projection_t::orthographic);
		UpdateDefaultBuffer();

		FinalPass(geometryBuffer->attachments[0]);
		
		DrawGUI(windows[0]);
		
		windows[0]->SwapDrawBuffers();
		ClearBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	virtual void GeometryPass()
	{
		geometryBuffer->Bind();
		geometryBuffer->DrawAll();

		testModel->meshes[0].textures[0].SetActive(0);

		glBindVertexArray(testModel->meshes[0].vertexArrayHandle);
		glUseProgram(programGLID);
		glCullFace(GL_BACK);

		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		glDrawElements(GL_TRIANGLES, testModel->meshes[0].indices.size(), GL_UNSIGNED_INT, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		geometryBuffer->Unbind();
	}

	virtual void FinalPass(texture* tex1)
	{
		//draw directly to backbuffer		
		tex1->SetActive(0);
		
		glBindVertexArray(defaultVertexBuffer->vertexArrayHandle);
		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		glUseProgram(finalProgram);
	
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	virtual void BuildGUI(tWindow* window, ImGuiIO io) override
	{
		scene3D::BuildGUI(windows[0], io);

		DrawBufferAttachments();
		DrawUserSettings();
	}

	virtual void DrawBufferAttachments()
	{
		ImGui::Begin("framebuffers");
		for (auto iter : geometryBuffer->attachments)
		{
			ImGui::Image((ImTextureID*)iter->GetHandle(), ImVec2(512, 288),
				ImVec2(0, 1), ImVec2(1, 0));
			ImGui::SameLine();
			ImGui::Text("%s\n", iter->GetUniformName().c_str());
		}

		ImGui::End();
	}

	virtual void DrawUserSettings()
	{
		ImGui::Begin("User Settings");
		ImGui::DragFloat("test float", &testBuffer.data.testFloat, 0.01f, 0.001f, 10.0f, "%.3f");

		ImGui::End();
	}

	virtual void ClearBuffers()
	{
		//ok copy the current frame into the previous frame and clear the rest of the buffers	
		float clearColor1[4] = { 0.05f, 0.05f, 0.05f, 0.0f };
		float clearColor2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float clearColor3[4] = { 1.0f, 0.0f, 0.0f, 0.0f }; //this is for debugging only!

		glClear(GL_DEPTH_BUFFER_BIT);

		geometryBuffer->Bind();
		geometryBuffer->ClearTexture(geometryBuffer->attachments[0], clearColor1);
		geometryBuffer->ClearTexture(geometryBuffer->attachments[1], clearColor2);
		glClear(GL_DEPTH_BUFFER_BIT);
		geometryBuffer->Unbind();

		sceneCamera->ChangeProjection(camera::projection_t::perspective);
	}

	virtual void ResizeBuffers(glm::ivec2 resolution)
	{
		for (auto iter : geometryBuffer->attachments)
		{
			iter->Resize(glm::ivec3(resolution, 1));
		}

		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
	}

	virtual void InitializeUniforms() override
	{
		defaultPayload = bufferHandler_t<defaultUniformBuffer>(sceneCamera);
		glViewport(0, 0, windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);

		defaultPayload.data.resolution = glm::vec2(windows[0]->settings.resolution.width, windows[0]->settings.resolution.height);
		defaultPayload.data.projection = sceneCamera->projection;
		defaultPayload.data.translation = sceneCamera->translation;
		defaultPayload.data.view = sceneCamera->view;

		defaultPayload.Initialize(0);
		testBuffer.Initialize(1);

		SetupVertexBuffer();
	}

	virtual void HandleWindowResize(tWindow* window, TinyWindow::vec2_t<unsigned int> dimensions) override
	{
		defaultPayload.data.resolution = glm::ivec2(dimensions.width, dimensions.height);	
		ResizeBuffers(glm::ivec2(dimensions.x, dimensions.y));
	}

	virtual void HandleMaximize(tWindow* window) override
	{
		defaultPayload.data.resolution = glm::ivec2(window->settings.resolution.width, window->settings.resolution.height);
		ResizeBuffers(defaultPayload.data.resolution);
	}

	virtual void HandleMouseClick(tWindow* window, mouseButton_t button, buttonState_t state) override
	{
		scene3D::HandleMouseClick(window, button, state);
	}

	virtual void HandleMouseMotion(tWindow* window, vec2_t<int> windowPosition, vec2_t<int> screenPosition) override
	{
		scene3D::HandleMouseMotion(window, windowPosition, screenPosition);
	}

	virtual void HandleKey(tWindow* window, int key, keyState_t state)	override
	{
		ImGuiIO& io = ImGui::GetIO();
		if (state == keyState_t::down)
		{
			io.KeysDown[key] = true;
			io.AddInputCharacter(key);
		}

		else
		{
			io.KeysDown[key] = false;
		}

		float camSpeed = 0.0f;
		if (key == key_t::leftShift && state == keyState_t::down)
		{
			camSpeed = sceneCamera->speed * 2;
		}

		else
		{
			camSpeed = sceneCamera->speed;
		}

		float deltaTime = (float)sceneClock->GetDeltaTime();

		if (state == keyState_t::down) //instead of one key could we check multiple keys?
		{
			if (window->keys['w'] == keyState_t::down)
			{
				sceneCamera->MoveForward(camSpeed, deltaTime);
			}

			if (window->keys['s'] == keyState_t::down)
			{
				sceneCamera->MoveForward(-camSpeed, deltaTime);
			}

			if (window->keys['a'] == keyState_t::down)
			{
				sceneCamera->MoveRight(-camSpeed, deltaTime);
			}

			if (window->keys['d'] == keyState_t::down)
			{
				sceneCamera->MoveRight(camSpeed, deltaTime);
			}

			if (window->keys['e'] == keyState_t::down)
			{
				sceneCamera->MoveUp(camSpeed, deltaTime);
			}

			if (window->keys['q'] == keyState_t::down)
			{
				sceneCamera->MoveUp(-camSpeed, deltaTime);
			}

			if (window->keys['z'] == keyState_t::down)
			{
				sceneCamera->Roll(glm::radians((float)sceneCamera->zSensitivity * deltaTime));
			}

			if (window->keys['x'] == keyState_t::down)
			{
				sceneCamera->Roll(glm::radians((float)-sceneCamera->zSensitivity * deltaTime));
			}
		}
	}

};

#endif