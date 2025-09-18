#ifndef SCENE_H
#define SCENE_H

//#include <utility>

#include "Globals.h"

using frameRates_t = enum {UNCAPPED = 0, THIRTY = 30, SIXTY = 60, NINETY = 90, ONETWENTY = 120, ONEFOURTYFOUR = 144};

class scene
{
public:
	explicit scene(const char* windowName = "Ziyad Barakat's Portfolio ( Example Scene )",
	               const camera_t& camera = camera_t(),
	               const char* shaderConfigPath = SHADER_CONFIG_DIR)
	{
		this->windowName = windowName;
		this->camera = camera;
		this->shaderConfigPath = shaderConfigPath;
		//defaultVertexBuffer = vertexBuffer_t();
		//defaultUniform = nullptr;
		imGUIFontTexture = 0;

		isFrameRateLocked = false;
		lockedFrameRate = 60;

		manager = new windowManager();
		
		windowSetting_t setting;
		setting.name = windowName;
		setting.userData = this;
		setting.resolution = vec2_t<uint16_t>(defaultWindowSize.x, defaultWindowSize.y);
		setting.SetProfile(profile_e::core);
		//setting.enableSRGB = true;

		window = manager->AddWindow(setting);
		assert(window != nullptr);
		scene::InitImGUI(window);

		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		
		//sceneClock = tinyClock_t();
	}

	virtual ~scene()
	{
		scene::ImGUIInvalidateDeviceObject(); // Missing cleanup for ImGui context
		ImGui::DestroyContext(); // Should destroy the context
		windowContextMap.clear(); // Clear the map
		//delete this->sceneCamera;		this->sceneCamera = nullptr;
		delete manager;					manager = nullptr;
		//delete sceneClock;				sceneClock = nullptr;
		delete defaultTimer;			defaultTimer = nullptr;
	}

	virtual void Run()
	{
		while (!window->GetShouldClose())
		{
			Update();
			Draw();
		}
	}

	virtual void Initialize()
	{
		te::InitializeExtensions();

		if (glDebugMessageCallback == nullptr)
		{
			printf("blarg \n");
		}

		// Enable debug output
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(&OpenGLDebugCallback, nullptr);

		LoadShaderProgramsFromConfigFile(&shaderProgramsMap);

		defProgram = shaderProgramsMap[PROJECT_NAME]; //need a better way to automate this

		glUseProgram(defProgram.handle);

		InitializeUniforms();
		SetupCallbacks();
		defaultTimer = new GPUTimer();
	}

	virtual void SetupCallbacks()
	{
		manager->resizeEvent = std::bind(&scene::HandleWindowResize, this, _1, _2);
		manager->mouseButtonEvent = std::bind(&scene::HandleMouseClick, this, _1, _2, _3);
		manager->mouseMoveEvent = std::bind(&scene::HandleMouseMotion, this, _1, _2, _3);
		manager->mouseWheelEvent = std::bind(&scene::HandleMouseWheel, this, _1, _2);
		manager->maximizedEvent = std::bind(&scene::HandleMaximize, this, _1);
		manager->keyEvent = std::bind(&scene::HandleKey, this, _1, _2, _3);
		manager->fileDropEvent = std::bind(&scene::HandleFileDrop, this, _1, _2);

		//tinywindow
		manager->managerErrorEvent = std::bind(&scene::HandleTWErrors, this, _1);
		manager->windowErrorEvent = std::bind(&scene::HandleTWWindowErrors, this, _1, _2);
		//tinyextender
		TinyExtender::errorEvent = std::bind(&scene::HandleTEError, this, _1);
		//tinyshaders
		TinyShaders::managerErrorEvent = std::bind(&scene::HandleShaderManagerError, this, _1);
		TinyShaders::shaderErrorEvent = std::bind(&scene::HandleShaderError, this, _1, _2);
		TinyShaders::shaderProgramErrorEvent = std::bind(&scene::HandleShaderProgramError, this, _1, _2);

	}

	void ShutDown(tWindow* window)
	{
		for (auto val : shaderProgramsMap | std::views::values)
		{
			ShutdownShaderProgram(val);
		}

		ImGUIInvalidateDeviceObject();
		manager->ShutDown();
	}
	
protected:

	windowManager*									manager;
	std::map<tWindow*, ImGuiContext*>				windowContextMap;
	tWindow*										window;

	tsl::robin_map<std::string, shaderProgram_t>	shaderProgramsMap;

	tinyClock_t										clock;
	vertexBuffer_t									defaultVertexBuffer;

	bufferHandler_t<defaultUniformBuffer>			defaultPayload;

	camera_t					camera;
	const char*					windowName;
	shaderProgram_t				defProgram;
	const char*					shaderConfigPath;

	ImGuiContext*				imGUIContext;
	GLuint						imGUIFontTexture;
	GLuint						imGUIShaderhandle;
	GLuint						imGUIVertexHandle;
	GLuint						imGUIFragmentHandle;
	GLint						imGUITexAttribLocation;
	GLint						imGUIProjMatrixAttribLocation;
	GLint						imGUIPositionAttribLocation;
	GLint						imGUIUVAttribLocation;
	GLint						imGUIColorAttribLocation;
	GLuint						imGUIVBOHandle;
	GLuint						imGUIVAOHandle;
	GLuint						imGUIIBOHandle;
	int interval = 1;

	bool						isGUIActive;
	bool						wireframe;

	bool						isFrameRateLocked;
	int							lockedFrameRate = UNCAPPED;
	std::vector<const char*>	frameRateSettings = { "none", "30", "60", "90", "120", "144" };

	int							currentResolution = 0;

	GPUTimer*					defaultTimer;

	std::string					defaultDockName = "Default";
	ImGuiID left_node, central_node;

	typedef std::pair<int32_t, std::string> debugTypeEntry;
	static tsl::robin_map<int32_t, std::string> debugTypeLUT;

	typedef std::pair<int32_t, std::string> severityEntry;
	static tsl::robin_map<int32_t, std::string> severityLUT;

	typedef std::pair<int32_t, std::string> sourceTypeEntry;
	static tsl::robin_map<int32_t, std::string> sourceTypeLUT;

	typedef std::pair<int32_t, ImGuiKey> keyMapEntry;
	static tsl::robin_map<int32_t, ImGuiKey> keyMapLUT;

	virtual void Update()
	{
		manager->PollForEvents();
		camera.Update();
		if (lockedFrameRate > 0)
		{
			clock.UpdateClockFixed(lockedFrameRate);
		}
		else
		{
			clock.UpdateClockAdaptive();
		}		

		defaultPayload.data.totalFrames++;
		defaultPayload.data.deltaTime = (float)clock.GetDeltaTime();
		defaultPayload.data.totalTime = (float)clock.GetTotalTime();
		defaultPayload.data.framesPerSec = (float)(1.0 / clock.GetDeltaTime());
		defaultPayload.data.projection = camera.projection;
		defaultPayload.data.view = camera.view;
		defaultPayload.data.translation = camera.translation;

		defaultPayload.Update(GL_UNIFORM_BUFFER, GL_STATIC_DRAW);
	}

	virtual void Draw()
	{
		PreDraw();

		glBindVertexArray(defaultVertexBuffer.vertexArrayHandle);
		glUseProgram(defProgram.handle);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		PostDraw();
	}

	virtual void PreDraw()
	{
		manager->MakeCurrentContext(window);
	}

	virtual void PostDraw()
	{
		DrawGUI(window);

		manager->SwapDrawBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	virtual void BuildGUI(tWindow* window, const ImGuiIO& io)
	{
		if (ImGui::BeginTabItem("Default"))
		{
			ImGui::SetCurrentContext(windowContextMap[window]);

			ImGui::Text("FPS %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, 1.0f / clock.GetDeltaTime());
			ImGui::Text("Total running time %.5f", clock.GetTotalTime());
			ImGui::Text("Mouse coordinates: \t X: %.0f \t Y: %.0f", io.MousePos.x, io.MousePos.y);
			ImGui::Text("Window size: \t Width: %i \t Height: %i", window->GetSettings().resolution.width, window->GetSettings().resolution.height);

			if(ImGui::Button("Toggle Fullscreen"))
			{
				//manager->ToggleFullscreen(window, &manager->GetMonitors()[0], 0);
				glViewport(0, 0, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
			}

			if (ImGui::InputInt("Swap Interval", &interval, 1))
			{
				manager->SetWindowSwapInterval(window, interval);
			}

			static int frameRatePick = 0;
			ImGui::ListBox("Frame rate cap", &frameRatePick, frameRateSettings.data(), (uint16_t)frameRateSettings.size());
			switch (frameRatePick)
			{
			case 0: //none
			case 1: //30
			case 2: //60
			case 3: //90
			case 4: //120
				{
					lockedFrameRate = frameRatePick * 30;
					break;
				}
			case 5: //144
				{
					lockedFrameRate = 144;
					break;
				}
			default: {};
			}

			//camera.resolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
			ImGui::Checkbox("wireframe", &wireframe);
			ImGui::EndTabItem();
		}
		//ImGui::End();
	}

	virtual void DrawCameraStats()
	{
		//set up the view matrix
		ImGui::Begin("camera", &isGUIActive);// , ImVec2(0, 0));

		ImGui::Combo("projection type", (int*)&camera.currentProjectionType, "perspective\0orthographic");  // NOLINT(performance-no-int-to-ptr)

		if (camera.currentProjectionType == camera_t::projection_e::orthographic)
		{
			ImGui::DragFloat("near plane", &camera.nearPlane);
			ImGui::DragFloat("far plane", &camera.farPlane);
			ImGui::SliderFloat("Field of view", &camera.fieldOfView, 0, 90, "%.10f");
		}

		else
		{
			ImGui::InputFloat("camera speed", &camera.speed, 0.f);
			ImGui::InputFloat("x sensitivity", &camera.xSensitivity, 0.f);
			ImGui::InputFloat("y sensitivity", &camera.ySensitivity, 0.f);
		}

		if (ImGui::TreeNode("view matrix"))
		{
			if (ImGui::TreeNode("right"))
			{
				//ImGui::Columns(2);
				//ImGui::ListBox("blarg", 0, )
				//ImGui::Text("blarg");
				//ImGui::SameLine();
				//ImGui::NextColumn();
				ImGui::DragFloat4("##", (float*)&camera.view[0], 0.1f, -100.0f, 100.0f);
				//ImGui::Columns(1);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("up"))
			{
				ImGui::DragFloat4("##", (float*)&camera.view[1], 0.1f, -100.0f, 100.0f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("forward"))
			{
				ImGui::DragFloat4("##", (float*)&camera.view[2], 0.1f, -100.0f, 100.0f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("position"))
			{
				ImGui::DragFloat4("##", (float*)&camera.view[3], 0.1f, -100.0f, 100.0f);
				ImGui::TreePop();
			}
			ImGui::TreePop();

		}
		ImGui::End();

		defaultPayload.Update(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
		//UpdateBuffer(d, defaultUniform->bufferHandle, sizeof(defaultUniform), gl_uniform_buffer, gl_dynamic_draw);
	}

	virtual void BeginGUI(tWindow* window)
	{
		ImGUINewFrame(window);

		ImGui::DockSpace(ImGui::GetID(defaultDockName.c_str()), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_NoResize);
		ImGuiID dockspace_id = ImGui::GetID(defaultDockName.c_str());
		//ImGui::DockBuilderRemoveNode(dockspace_id, dockspace_id | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, &left_node, &central_node);
		ImGui::DockBuilderFinish(dockspace_id);

		ImGui::SetNextWindowDockID(left_node, ImGuiCond_Once);
		ImGui::Begin(window->GetSettings().name.c_str(), &isGUIActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::SetWindowSize(ImVec2(window->GetSettings().resolution.width / 4, window->GetSettings().resolution.height));
		ImGui::SetWindowPos(ImVec2(0, 0));

		ImGui::BeginTabBar("SidePanelTabs");
	}

	virtual void EndGUI(tWindow* window)
	{
		ImGui::EndTabBar();
		ImGui::End();
		ImGui::Render();
		HandleImGUIRender(window);
	}

	virtual void DrawGUI(tWindow* window)
	{
		BeginGUI(window);
		const ImGuiIO io = ImGui::GetIO();
		BuildGUI(window, io);
		EndGUI(window);
	}

	void SetupBuffer(const GLenum target, const GLenum usage)
	{
		defaultPayload.Initialize(0, target, usage);
	}

	void UpdateBuffer(const GLenum target, const GLenum usage)
	{
		defaultPayload.Update(target, usage);
	}

	virtual void InitializeUniforms()
	{
		defaultPayload.data = defaultUniformBuffer(this->camera);
		glViewport(defaultViewportOrigin.x, defaultViewportOrigin.y, window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.resolution = glm::vec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		defaultPayload.data.projection = glm::ortho(0.0f, (GLfloat)window->GetSettings().resolution.width, (GLfloat)window->GetSettings().resolution.height, 0.0f, 0.01f, 10.0f);

		defaultVertexBuffer.SetupDefault();
		SetupBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);

		SetupDefaultUniforms();
	}

	void SetupDefaultUniforms()
	{
		defaultPayload.SetupUniforms(defProgram.handle, "defaultSettings", 0);
	}

	virtual void Resize(const tWindow* window, glm::ivec2 dimensions)
	{
		if (dimensions == glm::ivec2(0))
		{
			dimensions = glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height);
		}
		glViewport(defaultViewportOrigin.x, defaultViewportOrigin.y, dimensions.x, dimensions.y);
		
		defaultPayload.data.resolution = glm::ivec2(dimensions.x, dimensions.y);
		defaultPayload.data.projection = glm::ortho(0.0f, (GLfloat)dimensions.x, (GLfloat)dimensions.y, 0.0f, defaultNearPlane, defaultFarPlane);

		UpdateBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	}

	virtual void HandleMouseClick(const tWindow* window, const mouseButton_e button, const buttonState_e state)
	{
		ImGuiIO& io = ImGui::GetIO();

		switch (button)
		{
			case mouseButton_e::left: state == buttonState_e::down ? io.MouseDown[0] = true : io.MouseDown[0] = false; break;
			case mouseButton_e::right: state == buttonState_e::down ? io.MouseDown[1] = true : io.MouseDown[1] = false; break;
			case mouseButton_e::middle: state == buttonState_e::down ? io.MouseDown[2] = true : io.MouseDown[2] = false; break;
			default: break;;
		}
	}

	virtual void HandleWindowResize(const tWindow* window, const vec2_t<uint16_t>& dimensions)
	{
		Resize(window, glm::vec2(dimensions.x, dimensions.y));
	}

	virtual void HandleMaximize(const tWindow* window)
	{
		//thrown in new window size
		Resize(window, glm::ivec2(window->GetSettings().resolution.width, window->GetSettings().resolution.height));
	}

	virtual void HandleMouseMotion(const tWindow* window, const vec2_t<int16_t>& windowPosition, const vec2_t<int16_t>& screenPosition)
	{
		defaultPayload.data.mousePosition = glm::vec2(windowPosition.x, windowPosition.y);
		UpdateBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2((float)windowPosition.x, (float)windowPosition.y); //why screen co-ordinates?
	}

	virtual void HandleMouseWheel(const tWindow* window, const mouseScroll_e scroll)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheel += (float)((scroll == mouseScroll_e::down) ? -1 : 1);
	}

	static ImGuiKey MapToImGuiKey(const int16_t& key)
	{
		// Map letters A-Z
		if (key >= 'A' && key <= 'Z')
		{
			return static_cast<ImGuiKey>(ImGuiKey_A + (key - 'A'));
		}
		// Map numbers 0-9
		if (key >= '0' && key <= '9')
		{
			return static_cast<ImGuiKey>(ImGuiKey_0 + (key - '0'));
		}
		// Map special keys using Windows virtual key codes

		if (keyMapLUT.contains(key))
		{
			return keyMapLUT.at(key);
		}
		else
		{
			return ImGuiKey_None;
		}
	}

	virtual void HandleKey(const tWindow* window, const int16_t& key, const keyState_e& keyState)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiKey imguiKey = MapToImGuiKey(key);
		if (imguiKey != ImGuiKey_None) {
			io.AddKeyEvent(imguiKey, keyState == keyState_e::down);
		}
	}

	virtual void HandleFileDrop(const tWindow* window, const std::vector<std::string>& files)
	{
		//for each file dropped in
		//make sure it's a texture
		//and load up a new window for each one?
	}

	virtual void HandleTEError(const std::string& message)
	{
		printf("%s\n", message.c_str());
	}

	virtual void HandleTWErrors(const std::string message)
	{
		printf("%s\n", message.c_str());
	}

	virtual void HandleTWWindowErrors(const tWindow* window,  const std::string message)
	{
		printf("window: %s error %s\n", window->GetSettings().name.c_str(), message.c_str());
	}

	virtual void HandleShaderManagerError(const std::string& message)
	{
		printf(message.c_str());
	}

	virtual void HandleShaderError(const shader_t* shader, const std::string& message)
	{
		printf("shader: %s error %s\n", shader->name.c_str(), message.c_str());
	}

	virtual void HandleShaderProgramError(const shaderProgram_t* program, const std::string& message)
	{
		printf("shader program: %s error %s\n", program->name.c_str(), message.c_str());
	}


	virtual void InitImGUI(tWindow* window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		
		if (windowContextMap.contains(window)) {
			ImGui::DestroyContext(windowContextMap[window]);
		}
		windowContextMap[window] = ImGui::GetCurrentContext();
		
		ImGuiIO& io = ImGui::GetIO();
		io.BackendRendererName = "imgui_impl_opengl3";
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

#if defined(TW_WINDOWS)
		//io.ImeWindowHandle = window->GetWindowHandle();
#endif

		imGUIShaderhandle = 0;
		imGUIVertexHandle = 0;
		imGUIFragmentHandle = 0;
		imGUITexAttribLocation = 0;
		imGUIProjMatrixAttribLocation = 0;
		imGUIPositionAttribLocation = 0;
		imGUIUVAttribLocation = 0;
		imGUIColorAttribLocation = 0;
		imGUIVBOHandle = 0;
		imGUIVAOHandle = 0;
		imGUIIBOHandle = 0;
		imGUIFontTexture = 0;

		ImGui::SetCurrentContext(windowContextMap[window]);
	}

	virtual void HandleImGUIRender(tWindow* window)
	{
		ImDrawData* drawData = ImGui::GetDrawData();

		ImGuiIO& io = ImGui::GetIO();

		drawData->ScaleClipRects(io.DisplayFramebufferScale);

		GLint lastProgram;
		GLint lastTexture;
		GLint lastArrayBuffer;
		GLint lastElementArrayBuffer;
		GLint lastVertexArray;
		GLint lastBlendSrc;
		GLint lastBlendDst;
		GLint lastBlendEquationRGB;
		GLint lastBlendEquationAlpha;
		GLint lastViewport[4];

		glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastArrayBuffer);
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &lastElementArrayBuffer);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArray);
		glGetIntegerv(GL_BLEND_SRC, &lastBlendSrc);
		glGetIntegerv(GL_BLEND_DST, &lastBlendDst);
		glGetIntegerv(GL_BLEND_EQUATION_RGB, &lastBlendEquationRGB);
		glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &lastBlendEquationAlpha);
		glGetIntegerv(GL_VIEWPORT, lastViewport);

		GLboolean lastEnableBlend = glIsEnabled(GL_BLEND);
		GLboolean lastEnableCullFace = glIsEnabled(GL_CULL_FACE);
		GLboolean lastEnableDepthTest = glIsEnabled(GL_DEPTH_TEST);
		GLboolean lastEnableScissorTest = glIsEnabled(GL_SCISSOR_TEST);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_SCISSOR_TEST);
		glActiveTexture(GL_TEXTURE0);

		const glm::vec2 resolution = glm::vec2(window->GetSettings().resolution.x, window->GetSettings().resolution.y);
		glViewport(0, 0, (GLsizei)(io.DisplaySize.x * io.DisplayFramebufferScale.x),
                 (GLsizei)(io.DisplaySize.y * io.DisplayFramebufferScale.y));

		glm::mat4 proj = glm::ortho(-(resolution.x / 2), resolution.x / 2, resolution.y / 2, -(resolution.y / 2), -1.0f, 10.f);
		const float orthoProjection[4][4] =
		{
			{ 2.0f / (float)window->GetSettings().resolution.width, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 2.0f / -(float)window->GetSettings().resolution.height, 0.0f, 0.0f },
			{ 0.0f, 0.0f, -1.0f, 0.0f },
			{ -1.0f, 1.0f, 0.0f, 1.0f }
		};
		//glm::mat4 testOrtho = glm::perspective(45.0f, )
		glUseProgram(imGUIShaderhandle);
		glUniform1i(imGUITexAttribLocation, 0);
		glUniformMatrix4fv(imGUIProjMatrixAttribLocation, 1, GL_FALSE, &orthoProjection[0][0]);
		glBindVertexArray(imGUIVAOHandle);

		for (int numCommandLists = 0; numCommandLists < drawData->CmdListsCount; numCommandLists++)
		{
			const ImDrawList* commandList = drawData->CmdLists[numCommandLists];
			const ImDrawIdx* indexBufferOffset = nullptr;

			glBindBuffer(GL_ARRAY_BUFFER, imGUIVBOHandle);
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)commandList->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&commandList->VtxBuffer.front(), GL_STREAM_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, imGUIIBOHandle);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)commandList->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid*)&commandList->IdxBuffer.front(), GL_STREAM_DRAW);

			for (const ImDrawCmd* drawCommand = commandList->CmdBuffer.begin(); drawCommand != commandList->CmdBuffer.end(); drawCommand++)
			{
				if (drawCommand->UserCallback)
				{
					drawCommand->UserCallback(commandList, drawCommand);
				}

				else
				{
					glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)drawCommand->GetTexID());
					glScissor(drawCommand->ClipRect.x, window->GetSettings().resolution.height - drawCommand->ClipRect.w, drawCommand->ClipRect.z - drawCommand->ClipRect.x, drawCommand->ClipRect.w - drawCommand->ClipRect.y);
					glDrawElements(GL_TRIANGLES, (GLsizei)drawCommand->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, indexBufferOffset);
				}

				indexBufferOffset += drawCommand->ElemCount;
			}
		}

		glUseProgram(lastProgram);
		//glActiveTexture(lastActiveTexture);
		glBindTexture(GL_TEXTURE_2D, lastTexture);
		glBindVertexArray(lastVertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, lastArrayBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lastElementArrayBuffer);
		glBlendEquationSeparate(lastBlendEquationRGB, lastBlendEquationAlpha);
		glBlendFunc(lastBlendSrc, lastBlendDst);
		lastEnableBlend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		lastEnableCullFace ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		lastEnableDepthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		lastEnableScissorTest ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
		glViewport(lastViewport[0], lastViewport[1], (GLsizei)lastViewport[2], (GLsizei)lastViewport[3]);
	}



	virtual void ImGUICreateFontsTexture()
	{
		const ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		GLint lastTexture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
		glGenTextures(1, &imGUIFontTexture);
		glBindTexture(GL_TEXTURE_2D, imGUIFontTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		//store the texture handle
		io.Fonts->TexID = static_cast<ImTextureID>(imGUIFontTexture);

		glBindTexture(GL_TEXTURE_2D, lastTexture);
	}

	virtual void ImGUINewFrame(const tWindow* drawWindow)
	{
		if (!imGUIFontTexture)
		{
			ImGUICreateDeviceObjects();
		}

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)drawWindow->GetSettings().resolution.width, (float)drawWindow->GetSettings().resolution.height);
		io.DisplayFramebufferScale = ImVec2(1, 1);
		io.DeltaTime = (float)clock.GetDeltaTime();

		auto it = windowContextMap.find(window);
		if (it != windowContextMap.end())
		{
			ImGui::SetCurrentContext(it->second);
		}

		ImGui::NewFrame();
	}

	virtual void ImGUICreateDeviceObjects()
	{
		GLint lastTexture, lastArrayBuffer, LastVertexArray;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastArrayBuffer);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &LastVertexArray);

		const char *vertex_shader =
			"#version 330\n"
			"uniform mat4 ProjMtx;\n"
			"in vec2 Position;\n"
			"in vec2 UV;\n"
			"in vec4 Color;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"	Frag_UV = UV;\n"
			"	Frag_Color = Color;\n"
			"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const char* fragment_shader =
			"#version 330\n"
			"uniform sampler2D Texture;\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
			"}\n";

		imGUIShaderhandle = glCreateProgram();
		imGUIVertexHandle = glCreateShader(GL_VERTEX_SHADER);
		imGUIFragmentHandle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(imGUIVertexHandle, 1, &vertex_shader, nullptr);
		glShaderSource(imGUIFragmentHandle, 1, &fragment_shader, nullptr);
		glCompileShader(imGUIVertexHandle);
		glCompileShader(imGUIFragmentHandle);
		glAttachShader(imGUIShaderhandle, imGUIVertexHandle);
		glAttachShader(imGUIShaderhandle, imGUIFragmentHandle);
		glLinkProgram(imGUIShaderhandle);

		imGUITexAttribLocation = glGetUniformLocation(imGUIShaderhandle, "Texture");
		imGUIProjMatrixAttribLocation = glGetUniformLocation(imGUIShaderhandle, "ProjMtx");
		imGUIPositionAttribLocation = glGetAttribLocation(imGUIShaderhandle, "Position");
		imGUIUVAttribLocation = glGetAttribLocation(imGUIShaderhandle, "UV");
		imGUIColorAttribLocation = glGetAttribLocation(imGUIShaderhandle, "Color");

		glGenBuffers(1, &imGUIVBOHandle);
		glGenBuffers(1, &imGUIIBOHandle);

		glGenVertexArrays(1, &imGUIVAOHandle);
		glBindVertexArray(imGUIVAOHandle);
		glBindBuffer(GL_ARRAY_BUFFER, imGUIVBOHandle);
		glEnableVertexAttribArray(imGUIPositionAttribLocation);
		glEnableVertexAttribArray(imGUIUVAttribLocation);
		glEnableVertexAttribArray(imGUIColorAttribLocation);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
		glVertexAttribPointer(imGUIPositionAttribLocation, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos));
		glVertexAttribPointer(imGUIUVAttribLocation, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv));
		glVertexAttribPointer(imGUIColorAttribLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col));
#undef OFFSETOF

		ImGUICreateFontsTexture();

		//restore GL state
		glBindTexture(GL_TEXTURE_2D, lastTexture); //why do the values change?
		glBindBuffer(GL_ARRAY_BUFFER, lastArrayBuffer);
		glBindVertexArray(LastVertexArray);
	}

	virtual void ImGUIInvalidateDeviceObject()
	{
		if (imGUIVAOHandle)
		{
			glDeleteVertexArrays(1, &imGUIVAOHandle);
			imGUIVAOHandle = 0;
		}

		if (imGUIVBOHandle)
		{
			glDeleteBuffers(1, &imGUIVBOHandle);
			imGUIVBOHandle = 0;
		}

		if (imGUIIBOHandle)
		{
			glDeleteBuffers(1, &imGUIIBOHandle);
			imGUIIBOHandle = 0;
		}

		glDetachShader(imGUIShaderhandle, imGUIVertexHandle);
		glDeleteShader(imGUIVertexHandle);
		imGUIVertexHandle = 0;

		glDetachShader(imGUIShaderhandle, imGUIFragmentHandle);
		glDeleteShader(imGUIFragmentHandle);
		imGUIFragmentHandle = 0;

		glDeleteProgram(imGUIShaderhandle);
		imGUIShaderhandle = 0;

		if (imGUIFontTexture)
		{
			glDeleteTextures(1, &imGUIFontTexture);
			ImGui::GetIO().Fonts->TexID = nullptr;
			imGUIFontTexture = 0;
		}
	}

	static void APIENTRY OpenGLDebugCallback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const char* message,
		const void* userParam)
	{
		if (severity != GL_DEBUG_SEVERITY_LOW &&
			severity != GL_DEBUG_SEVERITY_MEDIUM &&
			severity != GL_DEBUG_SEVERITY_HIGH)
		{
			//we can skip these for the time being
			return;
		}

		printf("---------------------opengl-callback-start------------\n");
		//print debug type
		printf("type: ");
		printf("%s\n", debugTypeLUT.at((int32_t)type).c_str());

		printf("ID: %i\n", id);

		//severity
		printf("severity: ");
		printf("%s\n", severityLUT.at((int32_t)severity).c_str());

		//source
		printf("Source: ");
		printf("%s\n", sourceTypeLUT.at((int32_t)source).c_str());

		printf("Message: \n");
		printf("%s \n", message);

		printf("---------------------opengl-callback-end--------------\n");
	}
};

tsl::robin_map<int32_t, std::string> scene::debugTypeLUT =
{
	debugTypeEntry(GL_DEBUG_TYPE_ERROR, "error"),
	debugTypeEntry(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "deprecated behavior"),
	debugTypeEntry(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "undefined behavior"),
	debugTypeEntry(GL_DEBUG_TYPE_PERFORMANCE, "performance"),
	debugTypeEntry(GL_DEBUG_TYPE_PORTABILITY, "portability"),
	debugTypeEntry(GL_DEBUG_TYPE_MARKER, "marker"),
	debugTypeEntry(GL_DEBUG_TYPE_PUSH_GROUP, "push group"),
	debugTypeEntry(GL_DEBUG_TYPE_POP_GROUP, "pop group"),
	debugTypeEntry(GL_DEBUG_TYPE_OTHER, "other"),
};

tsl::robin_map<int32_t, std::string> scene::severityLUT =
{
	severityEntry(GL_DEBUG_SEVERITY_LOW, "low"),
	severityEntry(GL_DEBUG_SEVERITY_MEDIUM, "medium"),
	severityEntry(GL_DEBUG_SEVERITY_HIGH, "high"),
};

tsl::robin_map<int32_t, std::string> scene::sourceTypeLUT =
{
	sourceTypeEntry(GL_DEBUG_SOURCE_API, "API"),
	sourceTypeEntry(GL_DEBUG_SOURCE_SHADER_COMPILER, "shader compiler"),
	sourceTypeEntry(GL_DEBUG_SOURCE_WINDOW_SYSTEM, "window system"),
	sourceTypeEntry(GL_DEBUG_SOURCE_THIRD_PARTY, "third party"),
	sourceTypeEntry(GL_DEBUG_SOURCE_APPLICATION, "application"),
	sourceTypeEntry(GL_DEBUG_SOURCE_OTHER, "other"),
};

tsl::robin_map<int32_t, ImGuiKey> scene::keyMapLUT =
{
	keyMapEntry(tab, ImGuiKey_Tab),					// VK_TAB
	keyMapEntry(arrowLeft, ImGuiKey_LeftArrow),		// VK_LEFT
	keyMapEntry(arrowRight, ImGuiKey_RightArrow),	// VK_RIGHT
	keyMapEntry(arrowUp , ImGuiKey_UpArrow),		// VK_UP
	keyMapEntry(arrowDown, ImGuiKey_DownArrow),		// VK_DOWN
	keyMapEntry(pageUp, ImGuiKey_PageUp),			// VK_PRIOR
	keyMapEntry(pageDown, ImGuiKey_PageDown),		// VK_NEXT
	keyMapEntry(home, ImGuiKey_Home),				// VK_HOME
	keyMapEntry(end, ImGuiKey_End),					// VK_END
	keyMapEntry(insert, ImGuiKey_Insert),			// VK_INSERT
	keyMapEntry(del, ImGuiKey_Delete),				// VK_DELETE
	keyMapEntry(backspace, ImGuiKey_Backspace),		// VK_BACK
	keyMapEntry(spacebar, ImGuiKey_Space),			// VK_SPACE
	keyMapEntry(enter, ImGuiKey_Enter),				// VK_RETURN
	keyMapEntry(escape, ImGuiKey_Escape),			// VK_ESCAPE
};

#endif
