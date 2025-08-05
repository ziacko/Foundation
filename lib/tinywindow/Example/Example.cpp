//#define TW_NO_CONSOLE
#include "TinyWindow.h"

using namespace TinyWindow;

bool spacePressed = false;
bool shouldQuit = false;
void HandleKeyPresses(const tWindow* window, const unsigned int key, const keyState_e keyState)
{
	const auto& windowSetings = window->GetSettings();
	if(keyState == keyState_e::down)
	{
		switch (key)
		{
			case spacebar:
			{
				printf("Window: %s | spacebar has been pressed \n", windowSetings.name.c_str());
				spacePressed = true;
				break;
			}

		case leftShift:
				{
					printf("Window: %s | left shift has been pressed \n", windowSetings.name.c_str());
					break;
				}

			case rightShift:
				{
					printf("Window: %s | right shift has been pressed \n", windowSetings.name.c_str());
					break;
				}

			default:
			{
				if (key < 255)
				{
					printf("Window: %s | %c | down\n", windowSetings.name.c_str(), key);
				}
				break;
			}
		}
	}

	else if (keyState == keyState_e::up)
	{
		switch (key)
		{
			case escape:
			{
				printf("Window: %s | escape has been pressed \n", windowSetings.name.c_str());
				shouldQuit = true;
				break;
			}
			default:
			{
				if (key < 255)
				{
					printf("Window: %s | %c | up\n", windowSetings.name.c_str(), key);
				}
				break;
			}
		}
	}
}

void HandleMouseClick(const tWindow* window, const mouseButton_e button, const buttonState_e state)
{
	auto windowSetings = window->GetSettings();
	switch (button)
	{
	case mouseButton_e::left:
	{
		if (state == buttonState_e::down)
		{
			printf("Window: %s | left button down \n", windowSetings.name.c_str());
		}
		break;
	}

	case mouseButton_e::middle:
	{
		if (state == buttonState_e::down)
		{
			printf("Window: %s | middle button down \n", windowSetings.name.c_str());
		}
		break;
	}

	case mouseButton_e::right:
	{
		if (state == buttonState_e::down)
		{
			printf("Window: %s | right button down \n", windowSetings.name.c_str());
		}
		break;
	}
	
	case mouseButton_e::XFirst:
	{
		if (state == buttonState_e::down)
		{
			printf("poo poo \n");
		}
		break;
	}

	case mouseButton_e::XSecond:
	{
		if (state == buttonState_e::down)
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

void HandleMouseWheel(const tWindow* window, const mouseScroll_e mouseScrollDirection)
{
	auto windowSetings = window->GetSettings();
	switch (mouseScrollDirection)
	{
		case mouseScroll_e::down:
		{
			printf("Window: %s | mouse wheel down \n", windowSetings.name.c_str());
			break;
		}

		case mouseScroll_e::up:
		{
			printf("Window: %s | mouse wheel up \n", windowSetings.name.c_str());
			break;
		}
	}
}

void HandleShutdown(const tWindow* window)
{
	printf("window: %s has closed \n", window->GetSettings().name.c_str());
}

void HandleMaximized(const tWindow* window)
{
	printf("Window: %s | has been maximized \n", window->GetSettings().name.c_str());
}

void HandleMinimized(const tWindow* window)
{
	printf("Window: %s | has been minimized \n", window->GetSettings().name.c_str());
}

void HandleFocus(const tWindow* window, const bool isFocused)
{
	isFocused ? printf("Window: %s | is now in focus\n", window->GetSettings().name.c_str()) : printf("Window: %s | is out of focus\n", window->GetSettings().name.c_str());
}

void HandleMovement(const tWindow* window, const vec2_t<int>& windowPosition)
{
	printf("Window: %s | new position X: %i Y:%i\n", window->GetSettings().name.c_str(), windowPosition.x, windowPosition.y);
}

void HandleResize(const tWindow* window, const vec2_t<unsigned int>& windowSize)
{
	printf("Window: %s | new position X: %i Y:%i\n", window->GetSettings().name.c_str(), windowSize.width, windowSize.height);
}

void HandleMouseMovement(const tWindow* window, const vec2_t<int16_t>& windowMousePosition, const vec2_t<int16_t>& screenMousePosition)
{
	printf("Window: %s | window position X: %i Y: %i | screen position X: %i Y: %i \n", window->GetSettings().name.c_str(),
		windowMousePosition.x, windowMousePosition.y, screenMousePosition.x, screenMousePosition.y);
}

void HandleFileDrop(const tWindow* window, const std::vector<std::string>& files, const vec2_t<int>& windowMousePosition)
{
	printf("Window: %s | files dropped | \n", window->GetSettings().name.c_str());
	for (const auto& iter : files)
	{
		printf("\t %s \n", iter.c_str());
	}
}

void HandleWindowErrors(const tWindow* window, const std::string& newError)
{
	printf("%s \n", newError.c_str());
}

void HandleManagerErrors(const std::string& newError)
{
	printf("%s \n", newError.c_str());
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
	defaultSetting.SetProfile(profile_e::core);
	defaultSetting.currentState = state_e::maximized;
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
			auto files = manager->GetClipboardFiles(window.get());
			for (const auto& iter : files)
			{
				printf("%s \n", iter.c_str());
			}
			//manager->SetDecorators(window.get(), style_n::none);
			//manager->DisableDecorators(window.get(), decorator_e::titleBar | decorator_e::icon);
			//manager->SetTitleBar(window.get(), "poopoo");
			//manager->SetWindowSwapInterval(window.get(), 0);
			spacePressed = false;
		}
		
		//manager->SwapDrawBuffers(window.get());
		glClear(GL_COLOR_BUFFER_BIT);
	}

	manager->ShutDown();
	const tWindow* tempWindow = window.release();
	delete tempWindow;
	
	return 0;
}
