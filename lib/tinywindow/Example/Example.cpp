//#define TW_NO_CONSOLE
#include "TinyWindow.h"

using namespace TinyWindow;

bool spacePressed = false;
bool shouldQuit = false;
void HandleKeyPresses(const tWindow* window, const unsigned int key, const keyState_t keyState)
{
	auto windowSetings = window->GetWindowSettings();
	if(keyState == keyState_t::down && key == spacebar)
	{
		printf("Window: %s | spacebar has been pressed \n", windowSetings.name);
		spacePressed = true;
	}

	else if (keyState == keyState_t::up && key == escape)
	{
		printf("Window: %s | escape has been pressed \n", windowSetings.name);
		shouldQuit = true;
	}

	else if(keyState == keyState_t::down)
	{
		printf("Window: %s | %c | down\n", windowSetings.name, key);
	}

	else if (keyState == keyState_t::up)
	{
		printf("Window: %s | %c | up\n", windowSetings.name, key);
	}
}

void HandleMouseClick(const tWindow* window, const mouseButton_t button, const buttonState_t state)
{
	auto windowSetings = window->GetWindowSettings();
	switch (button)
	{
	case mouseButton_t::left:
	{
		if (state == buttonState_t::down)
		{
			printf("Window: %s | left button down \n", windowSetings.name);
		}
		break;
	}

	case mouseButton_t::middle:
	{
		if (state == buttonState_t::down)
		{
			printf("Window: %s | middle button down \n", windowSetings.name);
		}
		break;
	}

	case mouseButton_t::right:
	{
		if (state == buttonState_t::down)
		{
			printf("Window: %s | right button down \n", windowSetings.name);
		}
		break;
	}
	
	case mouseButton_t::XFirst:
	{
		if (state == buttonState_t::down)
		{
			printf("poo poo \n");
		}
		break;
	}

	case mouseButton_t::XSecond:
	{
		if (state == buttonState_t::down)
		{
			printf("pee pee \n");
		}
		break;
	}

	default:
	{
		break;
	}
	}
}

void HandleMouseWheel(const tWindow* window, const mouseScroll_t mouseScrollDirection)
{
	auto windowSetings = window->GetWindowSettings();
	switch (mouseScrollDirection)
	{
		case mouseScroll_t::down:
		{
			printf("Window: %s | mouse wheel down \n", windowSetings.name);
			break;
		}

		case mouseScroll_t::up:
		{
			printf("Window: %s | mouse wheel up \n", windowSetings.name);
			break;
		}
	}
}

void HandleShutdown(const tWindow* window)
{
	printf("window: %s has closed \n", window->GetWindowSettings().name);
}

void HandleMaximized(const tWindow* window)
{
	printf("Window: %s | has been maximized \n", window->GetWindowSettings().name);
}

void HandleMinimized(const tWindow* window)
{
	printf("Window: %s | has been minimized \n", window->GetWindowSettings().name);
}

void HandleFocus(const tWindow* window, const bool isFocused)
{
	isFocused ? printf("Window: %s | is now in focus\n", window->GetWindowSettings().name) : printf("Window: %s | is out of focus\n", window->GetWindowSettings().name);
}

void HandleMovement(const tWindow* window, const vec2_t<int>& windowPosition)
{
	printf("Window: %s | new position X: %i Y:%i\n", window->GetWindowSettings().name, windowPosition.x, windowPosition.y);
}

void HandleResize(const tWindow* window, const vec2_t<unsigned int>& windowSize)
{
	printf("Window: %s | new position X: %i Y:%i\n", window->GetWindowSettings().name, windowSize.width, windowSize.height);
}

void HandleMouseMovement(const tWindow* window, const vec2_t<int>& windowMousePosition, const vec2_t<int>& screenMousePosition)
{
	printf("Window: %s | window position X: %i Y: %i | screen position X: %i Y: %i \n", window->GetWindowSettings().name,
		windowMousePosition.x, windowMousePosition.y, screenMousePosition.x, screenMousePosition.y);
}

void HandleFileDrop(const tWindow* window, const std::vector<std::string>& files, const vec2_t<int>& windowMousePosition)
{
	printf("Window: %s | files dropped | \n", window->GetWindowSettings().name);
	for (const auto& iter : files)
	{
		printf("\t %s \n", iter.c_str());
	}
}

void HandleWindowErrors(const tWindow* window, const TinyWindow::errorEntry& newError)
{
	printf("%s \n", newError.second.c_str());
}

void HandleManagerErrors(const TinyWindow::errorEntry& newError)
{
	printf("%s \n", newError.second.c_str());
}

void PrintMonitorInfo(windowManager* manager)
{
	for (const auto& monitorIter : manager->GetMonitors())
	{
		printf("%s \n", monitorIter.GetDeviceName()->c_str());
		printf("%s \n", monitorIter.GetMonitorName()->c_str());
		printf("%s \n", monitorIter.GetDisplayName()->c_str());
		auto currentSetting = monitorIter.GetCurrentSetting();
		printf("resolution:\t current width: %i | current height: %i \n", currentSetting->resolution.width, currentSetting->resolution.height);
		printf("extents:\t top: %i | left: %i | bottom: %i | right: %i \n", monitorIter.GetExtents()->top, monitorIter.GetExtents()->left, monitorIter.GetExtents()->bottom, monitorIter.GetExtents()->right);
		int settingIndex = 0;
		for (const auto& settingIter : *monitorIter.GetMonitorSettings())
		{
			printf("index %i:\t width %i | height %i | frequency %i", settingIndex, settingIter.resolution.width, settingIter.resolution.height, settingIter.displayFrequency);
			settingIndex++;
#if defined(TW_WINDOWS)
			printf(" | flags %i", settingIter.displayFlags);
			switch (settingIter.fixedOutput)
			{
				case DMDFO_DEFAULT:
				{
					printf(" | output: %s", "default");
					break;
				}

				case DMDFO_CENTER:
				{
					printf(" | output: %s", "center");
					break;
				}

				case DMDFO_STRETCH:
				{
					printf(" | output: %s", "stretch");
					break;
				}
			}
#endif
			printf("\n");
		}
		printf("\n");
	}
}

int main()
{
	windowSetting_t defaultSetting;
	defaultSetting.name = "example window";
	defaultSetting.resolution = vec2_t<unsigned short>(1280, 720);
	defaultSetting.SetProfile(profile_t::core);
	defaultSetting.currentState = state_t::maximized;
	defaultSetting.enableSRGB = false;

	std::unique_ptr<windowManager> manager(new windowManager());
	manager->keyEvent = HandleKeyPresses;
	manager->mouseButtonEvent = HandleMouseClick;
	manager->windowErrorEvent = HandleWindowErrors;
	manager->managerErrorEvent = HandleManagerErrors;
	//manager->mouseWheelEvent = HandleMouseWheel;
	//manager->destroyedEvent = HandleShutdown;
	//manager->maximizedEvent = HandleMaximized;
	//manager->minimizedEvent = HandleMinimized;
	//manager->focusEvent = HandleFocus;
	//manager->movedEvent = HandleMovement;
	//manager->resizeEvent = HandleResize;
	//manager->fileDropEvent = HandleFileDrop;
	manager->mouseMoveEvent = HandleMouseMovement;
	manager->Initialize();
	std::unique_ptr<tWindow> window(manager->AddWindow(defaultSetting));

	PrintMonitorInfo(manager.get());

	//printf("%s \n", manager->GetClipboardInfo().c_str());
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

	bool toggleState = false;
	while (!window->GetShouldClose())
	{
		manager->PollForEvents();
		//HandleGamepadState(manager->GetGamepads()[0]);
		if (spacePressed)
		{
			//window->SetWindowSize(vec2_t<unsigned int>(manager->GetMonitors().back()->resolution.width, manager->GetMonitors().back()->resolution.height));
			//manager->ToggleFullscreen(window.get(), &manager->GetMonitors().at(0), 0);
			//window->SetFullScreen(true);
			manager->SetStyle(window.get(), style_t::bare);
			//manager->DisableDecorators(window.get(), decorator_t::border);
			//manager->SetTitleBar(window.get(), "poopoo");
			//manager->SetFullscreen(window.get(), true);
			//manager->SetWindowSwapInterval(window.get(), 0);
			spacePressed = false;
		}
		
		manager->SwapDrawBuffers(window.get());
		glClear(GL_COLOR_BUFFER_BIT);
	}

	manager->ShutDown();
	const tWindow* tempWindow = window.release();
	delete tempWindow;
	
	return 0;
}
