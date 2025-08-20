#ifndef TINYWINDOW_H
#define TINYWINDOW_H

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
#pragma ide diagnostic ignored "modernize-use-nodiscard"
// created by Ziyad Barakat 2014 - 2025

#include <cstdlib>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#define TW_WINDOWS
#if defined(_MSC_VER)
// this automatically loads the OpenGL library
// feel free to comment out
#pragma comment(lib, "opengl32.lib")
// for gamepad support
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "xinput.lib")

// this makes sure that the entry point of your program is main() not Winmain().
// feel free to comment out
#if defined(TW_NO_CONSOLE)
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#else
#pragma comment(linker, "/subsystem:console /ENTRY:mainCRTStartup")
#endif
#endif//_MSC_VER
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif// WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX 1
#endif// NOMINMAX
#ifndef WGL_WGLEXT_PROTOTYPES
#define WGL_WGLEXT_PROTOTYPES 1
#endif// WGL_WGLEXT_PROTOTYPES

#ifndef _WINDOWS_
#include <Windows.h>
#endif
#if !defined(TW_USE_VULKAN)
#include "wglext.h"
#include <gl/GL.h>
#ifdef USE_DINPUT
#include <dinput.h>
#endif// USE_DINPUT

#include <Dbt.h>
#include <Xinput.h>

#else
#include <vulkan.h>
#endif
#include <codecvt>
#include <fcntl.h>
#include <io.h>
#include <mmsystem.h>
#include <shellapi.h>
#endif//_WIN32 || _WIN64

#if defined(__linux__)
#define TW_LINUX

// Motif window hints definitions
#define MWM_HINTS_FUNCTIONS (1L << 0)
#define MWM_HINTS_DECORATIONS (1L << 1)
#define MWM_HINTS_INPUT_MODE (1L << 2)
#define MWM_HINTS_STATUS (1L << 3)

// MWM decorations
#define MWM_DECOR_ALL (1L << 0)
#define MWM_DECOR_BORDER (1L << 1)
#define MWM_DECOR_RESIZE (1L << 2)
#define MWM_DECOR_TITLE (1L << 3)
#define MWM_DECOR_MENU (1L << 4)
#define MWM_DECOR_MINIMIZE (1L << 5)
#define MWM_DECOR_MAXIMIZE (1L << 6)

#define MWM_FUNC_ALL (1L << 0)
#define MWM_FUNC_RESIZE (1L << 1)
#define MWM_FUNC_MOVE (1L << 2)
#define MWM_FUNC_MINIMIZE (1L << 3)
#define MWM_FUNC_MAXIMIZE (1L << 4)
#define MWM_FUNC_CLOSE (1L << 5)

#if !defined(TW_USE_VULKAN)

#include <GL/glx.h>
#include <GL/glxext.h>

#else
#include <vulkan.h>
#endif

#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>

#endif//__linux__

#include <algorithm>
#include <cstring>
#include <functional>
#include <memory>
#include <vector>

namespace TinyWindow
{
	class tWindow;
	class WindowManager;

	constexpr uint16_t defaultWindowWidth	 = 1280;
	constexpr uint16_t defaultWindowHeight = 720;

	template<typename type> struct vec2_t
	{
		vec2_t()
		{
			x = 0;
			y = 0;
		}

		vec2_t(type x, type y)
		{
			this->x = x;
			this->y = y;
		}

		union
		{
			type x;
			type width;
		};

		union
		{
			type y;
			type height;
		};

		static vec2_t Zero() { return vec2_t<type>(0, 0); }
	};

	template<typename type> struct vec4_t
	{
		vec4_t()
		{
			x = 0;
			y = 0;
			z = 0;
			w = 0;
		}

		vec4_t(type x, type y, type z, type w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		union
		{
			type x;
			type width;
			type left;
		};

		union
		{
			type y;
			type height;
			type top;
		};

		union
		{
			type z;
			type depth;
			type right;
		};

		union
		{
			type w;
			type homogenous;
			type bottom;
		};

		static vec4_t Zero() { return vec4_t<type>(0, 0, 0, 0); }
	};

	struct monitorSetting_t
	{
		// make all this private?
		vec2_t<uint16_t> resolution;
		// uint16_t bitsPerPixel;
		uint16_t displayFrequency;

#if defined(TW_WINDOWS)
		uint16_t displayFlags;
		uint16_t fixedOutput;
#elif defined(TW_LINUX)
		RROutput output;
		RRMode mode;
		RRCrtc crtc;
#endif

		explicit monitorSetting_t(const vec2_t<uint16_t>& inResolution = vec2_t<uint16_t>::Zero(), const uint16_t& inDisplayFrequency = 0) : resolution(inResolution), displayFrequency(inDisplayFrequency)
		{
#if defined(TW_WINDOWS)
			displayFlags = 0;
			fixedOutput	 = 0;
#endif

#if defined(TW_LINUX)
			output = 0;
			mode   = 0;
			crtc   = 0;
#endif
		}
	};

	class monitor_t
	{
		friend class tWindow;
		friend class windowManager;

	private:
		bool isPrimary;
		std::string deviceName;
		std::string monitorName;
		std::string displayName;
		vec4_t<uint16_t> extents;
		monitorSetting_t currentSetting;
		monitorSetting_t previousSetting;
		vec2_t<uint16_t> resolution;
		std::vector<monitorSetting_t> settings;// store all display settings

#if defined(TW_WINDOWS)
		HMONITOR monitorHandle;
#elif defined(TW_LINUX)
		Rotation rotation;
#endif

	public:
		monitor_t()
		{
			isPrimary  = false;
			resolution = vec2_t<uint16_t>::Zero();
			extents	   = vec4_t<uint16_t>::Zero();
#if defined(TW_WINDOWS)
			monitorHandle = nullptr;
#endif

#if defined(TW_LINUX)
			rotation = 0;
#endif
		}

		explicit monitor_t(const std::string& displayName, const std::string& deviceName, const std::string& monitorName, const bool& isPrimary = false)
		{
#if defined(TW_WINDOWS)
			this->monitorHandle = nullptr;
			this->displayName	= displayName;
#endif
#if defined(TW_LINUX)
			rotation = 0;
#endif
			this->deviceName  = deviceName;
			this->monitorName = monitorName;
			this->displayName = displayName;
			this->isPrimary	  = isPrimary;
		}

		const bool* GetIsPrimary() const { return &isPrimary; }
		const vec4_t<uint16_t>* GetExtents() const { return &extents; }
		const std::string* GetDeviceName() const { return &deviceName; }
		const std::string* GetMonitorName() const { return &monitorName; }
		const std::string* GetDisplayName() const { return &displayName; }
		const vec2_t<uint16_t>* GetResolution() const { return &resolution; }
		const monitorSetting_t* GetCurrentSetting() const { return &currentSetting; }
		const monitorSetting_t* GetPreviousSetting() const { return &previousSetting; }
		const std::vector<monitorSetting_t>* GetMonitorSettings() const { return &settings; }
	};

	struct formatSetting_t
	{
		friend class windowManager;

		uint8_t redBits;
		uint8_t greenBits;
		uint8_t blueBits;
		uint8_t alphaBits;
		uint8_t depthBits;
		uint8_t stencilBits;

		uint8_t accumRedBits;
		uint8_t accumGreenBits;
		uint8_t accumBlueBits;
		uint8_t accumAlphaBits;

		uint8_t auxBuffers;
		uint8_t numSamples;

		bool stereo;
		bool doubleBuffer;
		bool pixelRGB;

	private:
#if defined(TW_WINDOWS)
		int handle;
#endif

	public:
		explicit formatSetting_t(const uint8_t& redBits = 8, const uint8_t& greenBits = 8, const uint8_t& blueBits = 8, const uint8_t& alphaBits = 8,
			const uint8_t& depthBits = 32, const uint8_t& stencilBits = 8, const uint8_t& accumRedBits = 8, const uint8_t& accumGreenBits = 8, const uint8_t& accumBlueBits = 8, const uint8_t& accumAlphaBits = 8,
			const uint8_t& auxBuffers = 0, const uint8_t& numSamples = 0, const bool& stereo = false, const bool& doubleBuffer = true)
		{
			this->redBits	  = redBits;
			this->greenBits	  = greenBits;
			this->blueBits	  = blueBits;
			this->alphaBits	  = alphaBits;
			this->depthBits	  = depthBits;
			this->stencilBits = stencilBits;

			this->accumRedBits	 = accumRedBits;
			this->accumGreenBits = accumGreenBits;
			this->accumBlueBits	 = accumBlueBits;
			this->accumAlphaBits = accumAlphaBits;

			this->auxBuffers = auxBuffers;
			this->numSamples = numSamples;

			this->stereo	   = stereo;
			this->doubleBuffer = doubleBuffer;
			pixelRGB		   = true;
#if defined(TW_WINDOWS)
			this->handle = 0;
#endif
		}
	};

	enum class profile_e
	{
		core,
		compatibility,
	};

	enum class state_e
	{
		normal,		/**< The window is in its default state */
		maximized,	/**< The window is currently maximized */
		minimized,	/**< The window is currently minimized */
		fullscreen, /**< The window is currently full screen */
	};

	struct windowSetting_t
	{
		friend class windowManager;

		//should i move this to a window descriptor system?
		explicit windowSetting_t(const std::string& name = "window", void* userData = nullptr, const vec2_t<uint16_t>& resolution = vec2_t(defaultWindowWidth, defaultWindowHeight),
			const uint8_t& versionMajor = 4, const uint8_t& versionMinor = 5, const uint8_t& colorBits = 8, const uint8_t& depthBits = 24, const uint8_t& stencilBits = 8, const uint8_t& accumBits = 8,
			const state_e& currentState = state_e::normal, const profile_e& profile = profile_e::core)
		{
			this->name		   = name;
			this->resolution   = resolution;
			this->colorBits	   = colorBits;
			this->depthBits	   = depthBits;
			this->stencilBits  = stencilBits;
			this->accumBits	   = accumBits;
			this->currentState = currentState;
			this->userData	   = userData;
#if !defined(TW_USE_VULKAN)
			this->versionMajor = versionMajor;
			this->versionMinor = versionMinor;
#endif
			this->enableSRGB = false;
			SetProfile(profile);

#if defined(TW_LINUX)
			this->bestFBConfig = nullptr;
#endif
		}

		void SetProfile(profile_e inProfile)
		{
#if defined(TW_WINDOWS) && !defined(TW_USE_VULKAN)
			this->profile = (inProfile == profile_e::compatibility) ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
#elif defined(TW_LINUX) && !defined(TW_USE_VULKAN)
			this->profile = (inProfile == profile_e::compatibility) ? GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
#endif
		}

		void* userData;
		std::string name; /**< Name of the window */
		bool enableSRGB;  /**< whether the window will support an sRGB colorspace backbuffer */
		state_e currentState;
		/**< The current state of the window. these states include Normal, Minimized, Maximized and Full screen */
		unsigned char colorBits;		   /**< Color format of the window. (defaults to 32 bit color) */
		unsigned char depthBits;		   /**< Size of the Depth buffer. (defaults to 8 bit depth) */
		unsigned char accumBits;		   /**< Size of the Accum buffer */
		unsigned char stencilBits;		   /**< Size of the stencil buffer, (defaults to 8 bit) */
		vec2_t<uint16_t> resolution; /**< Resolution/Size of the window stored in an array */

#if !defined(TW_USE_VULKAN)
		GLint versionMajor;				   /**< Major OpenGL version*/
		GLint versionMinor;				   /**< Minor OpenGL version*/
	private:
		GLint profile;					   /**< Compatibility or core OpenGL profiles*/
#if defined(TW_LINUX)
		GLXFBConfig bestFBConfig;
#endif
#endif
	};

	enum class keyState_e
	{
		bad,  /**< If get key state fails (could not name it ERROR) */
		up,	  /**< The key is currently up */
		down, /**< The key is currently down */
	};

	enum key_e
	{
		bad	  = -1,		 /**< The key pressed is considered invalid */
		first = 256 + 1, /**< The first key that is not a char */
		F1,				 /**< The F1 key */
		F2,				 /**< The F2 key */
		F3,				 /**< The F3 key */
		F4,				 /**< The F4 key */
		F5,				 /**< The F5 key */
		F6,				 /**< The F6 key */
		F7,				 /**< The F7 key */
		F8,				 /**< The F8 key */
		F9,				 /**< The F9 key */
		F10,			 /**< The F10 key */
		F11,			 /**< The F11 key */
		F12,			 /**< The F12 key */
		capsLock,		 /**< The CapsLock key */
		leftShift,		 /**< The left Shift key */
		rightShift,		 /**< The right Shift key */
		leftControl,	 /**< The left Control key */
		rightControl,	 /**< The right Control key */
		leftWindow,		 /**< The left Window key */
		rightWindow,	 /**< The right Window key */
		leftAlt,		 /**< The left Alternate key */
		rightAlt,		 /**< The right Alternate key */
		enter,			 /**< The Enter/Return key */
		printScreen,	 /**< The PrintScreen key */
		scrollLock,		 /**< The ScrollLock key */
		numLock,		 /**< The NumLock key */
		pause,			 /**< The pause/break key */
		insert,			 /**< The insert key */
		home,			 /**< The Home key */
		end,			 /**< The End key */
		pageUp,			 /**< The PageUp key */
		pageDown,		 /**< The PageDown key */
		arrowDown,		 /**< The ArrowDown key */
		arrowUp,		 /**< The ArrowUp key */
		arrowLeft,		 /**< The ArrowLeft key */
		arrowRight,		 /**< The ArrowRight key */
		keypadDivide,	 /**< The KeyPad Divide key */
		keypadMultiply,	 /**< The Keypad Multiply key */
		keypadSubtract,	 /**< The Keypad Subtract key */
		keypadAdd,		 /**< The Keypad Add key */
		keypadEnter,	 /**< The Keypad Enter key */
		keypadPeriod,	 /**< The Keypad Period/Decimal key */
		keypad0,		 /**< The Keypad 0 key */
		keypad1,		 /**< The Keypad 1 key */
		keypad2,		 /**< The Keypad 2 key */
		keypad3,		 /**< The Keypad 3 key */
		keypad4,		 /**< The Keypad 4 key */
		keypad5,		 /**< The Keypad 5 key */
		keypad6,		 /**< The Keypad 6 key */
		keypad7,		 /**< The Keypad 7 key */
		keypad8,		 /**< The keypad 8 key */
		keypad9,		 /**< The Keypad 9 key */
		backspace,		 /**< The Backspace key */
		tab,			 /**< The Tab key */
		del,			 /**< The Delete key */
		spacebar,		 /**< The Spacebar key */
		escape,			 /**< The Escape key */
		apps,			 /**< The Applications key*/
		last = apps,	 /**< The last key to be supported */
	};

	enum class buttonState_e
	{
		up,		/**< The mouse button is currently up */
		down	/**< The mouse button is currently down */
	};

	enum class mouseButton_e
	{
		left,	 /**< The left mouse button */
		right,	 /**< The right mouse button */
		middle,	 /**< The middle mouse button / ScrollWheel */
		XFirst,	 /**< The first mouse X button */
		XSecond, /**< The second mouse X button */
		last,	 /**< The last mouse button to be supported */
	};

	enum class mouseScroll_e
	{
		down,	/**< The mouse wheel up */
		up		/**< The mouse wheel down */
	};

	enum decorator_e
	{
		titleBar	   = 1L << 1, /**< The title bar decoration of the window */
		icon		   = 1L << 2, /**< The icon decoration of the window */
		border		   = 1L << 3, /**< The border decoration of the window */
		minimizeButton = 1L << 4, /**< The minimize button decoration of the window */
		maximizeButton = 1L << 5, /**< The maximize button decoration pf the window */
		closeButton	   = 1L << 6, /**< The close button decoration of the window */
		sizeableBorder = 1L << 7, /**< The sizable border decoration of the window */
	};

	enum class clipboard_e
	{
		text, /**< The clipboard contains text */
		files, /**< The clipboard contains file paths */
		invalid /**< The clipboard content is invalid */
	};

	namespace style_n
	{
		constexpr uint32_t none = 0; /**< The window has no decorators */
		constexpr uint32_t bare = titleBar | border; /**< The window has no decorators but the window border and title bar */
		constexpr uint32_t normal = titleBar | border | closeButton | minimizeButton | maximizeButton | sizeableBorder; /**< The default window style for the respective platform */
	}

	enum class error_e
	{
		success,						/**< If a function call was successful*/
		invalidWindowName,				/**< If an invalid window name was given */
		invalidIconPath,				/**< If an invalid icon path was given */
		invalidWindowIndex,				/**< If an invalid window index was given */
		invalidWindowState,				/**< If an invalid window state was given */
		invalidResolution,				/**< If an invalid window resolution was given */
		invalidContext,					/**< If the OpenGL context for the window is invalid */
		existingContext,				/**< If the window already has an OpenGL context */
		notInitialized,					/**< If the window is being used without being initialized */
		alreadyInitialized,				/**< If the window was already initialized */
		invalidTitlebar,				/**< If the Title-bar text given was invalid */
		invalidCallback,				/**< If the given event callback was invalid */
		windowInvalid,					/**< If the window given was invalid */
		invalidWindowStyle,				/**< If the window style gives is invalid */
		invalidVersion,					/**< If an invalid OpenGL version is being used */
		invalidProfile,					/**< If an invalid OpenGL profile is being used */
		invalidInterval,				/**< If a window swap interval setting is invalid */
		fullscreenFailed,				/**< If setting the window to fullscreen has failed */
		noExtensions,					/**< If platform-specific window extensions have not been properly loaded */
		invalidExtension,				/**< If a platform-specific window extension is not supported */
		invalidDummyWindow,				/**< If the dummy window creation has failed */
		invalidDummyPixelFormat,		/**< If the pixel format for the dummy context id invalid */
		dummyCreationFailed,			/**< If the dummy context has failed to be created */
		invalidDummyContext,			/**< If the dummy context in invalid */
		dummyCannotMakeCurrent,			/**< If the dummy cannot be made the current context */
		cannotCreateCurrent, 			/**< cannot make context current */
		invalidMonitorSettingIndex,		/**< If the provided monitor setting index is invalid */
		functionNotImplemented,			/**< If the function has not yet been implemented in the current version of the API */
		linuxCannotConnectXServer,		/**< Linux: If cannot connect to an X11 server */
		linuxInvalidVisualinfo,			/**< Linux: If visual information given was invalid */
		linuxCannotCreateWindow,		/**< Linux: When X11 fails to create a new window */
		linuxFunctionNotImplemented,	/**< Linux: When the function has not yet been implemented on the Linux in the current version of the API */
		linuxCannotCreateDummyContext,	/**< Linux: if a dummy OpenGL context cannot be created */
		linuxCannotCreateAdvancedContext, /**< Linux: cannot create advanced context */
		linuxNoValidFBConfig,				/**< Linux: cannot find a suitable Framebuffer config */
		linuxNoHDRConfig, 					/**< Linux: cannot find HDR compatible FBConfig */
		windowsCannotCreateWindows,		/**< Windows: When Win32 cannot create a window */
		windowsCannotInitialize,		/**< Windows: When Win32 cannot initialize */
		windowsFullscreenBadDualView,	/**< Windows: The system is not DualView capable. whatever that means */
		windowsFullscreenBadFlags,		/**< Windows: Bad display change flags */
		windowsFullscreenBadMode,		/**< Windows: Bad display change mode */
		WindowsFullscreenBadParam,		/**< Windows: Bad display change Parameter */
		WindowsFullscreenChangeFailed,	/**< Windows: The display driver failed to implement the specified graphics mode */
		WindowsFullscreenNotUpdated,	/**< Windows: Unable to write settings to the registry */
		WindowsFullscreenNeedRestart,	/**< Windows: The computer must be restarted for the graphics mode to work */
		windowsFunctionNotImplemented,	/**< Windows: When a function has yet to be implemented on the Windows platform in the current version of the API */
	};

	typedef std::pair<error_e, std::string> errorEntry;
	const std::unordered_map errorLUT =
	{
		errorEntry(error_e::invalidWindowName, "Error: invalid window name"),
		errorEntry(error_e::invalidIconPath, "Error: invalid icon path"),
		errorEntry(error_e::invalidWindowIndex, "Error: invalid window index"),
		errorEntry(error_e::invalidWindowState, "Error: invalid window state"),
		errorEntry(error_e::invalidResolution, "Error: invalid resolution"),
		errorEntry(error_e::invalidContext, "Error: Failed to create OpenGL context"),
		errorEntry(error_e::existingContext, "Error: context already created"),
		errorEntry(error_e::notInitialized, "Error: Window manager not initialized"),
		errorEntry(error_e::alreadyInitialized, "Error: window has already been initialized"),
		errorEntry(error_e::invalidTitlebar, "Error: invalid title bar name (cannot be null or nullptr)"),
		errorEntry(error_e::invalidCallback, "Error: invalid event callback given"),
		errorEntry(error_e::windowInvalid, "Error: window was not found"),
		errorEntry(error_e::invalidWindowStyle, "Error: invalid window style given"),
		errorEntry(error_e::invalidVersion, "Error: invalid OpenGL version"),
		errorEntry(error_e::invalidProfile, "Error: invalid OpenGL profile"),
		errorEntry(error_e::fullscreenFailed, "Error: failed to enter fullscreen mode"),
		errorEntry(error_e::functionNotImplemented, "Error: I'm sorry but this function has not been implemented yet"),
		errorEntry(error_e::noExtensions, "Error: Platform extensions have not been loaded correctly"),
		errorEntry(error_e::invalidExtension, "Error: Platform specific extension is not valid"),
		errorEntry(error_e::invalidDummyWindow, "Error: the dummy window failed to be created"),
		errorEntry(error_e::invalidDummyPixelFormat, "Error: the pixel format for the dummy context is invalid"),
		errorEntry(error_e::dummyCreationFailed, "Error: the dummy context has failed to be created"),
		errorEntry(error_e::invalidDummyContext, "Error: the dummy context in invalid"),
		errorEntry(error_e::dummyCannotMakeCurrent, "Error: the dummy cannot be made the current context"),
		errorEntry(error_e::cannotCreateCurrent, "Error: the context cannot be made current"),
		errorEntry(error_e::invalidMonitorSettingIndex, "Error: the provided monitor setting index is invalid"),
		errorEntry(error_e::linuxCannotConnectXServer, "Linux Error: cannot connect to X server"),
		errorEntry(error_e::linuxInvalidVisualinfo, "Linux Error: Invalid visual information given"),
		errorEntry(error_e::linuxCannotCreateWindow, "Linux Error: failed to create window"),
		errorEntry(error_e::linuxFunctionNotImplemented, "Linux Error: function not implemented on Linux platform yet"),
		errorEntry(error_e::linuxCannotCreateDummyContext, "Linux Error: failed to create dummy context"),
		errorEntry(error_e::linuxNoValidFBConfig, "Linux Error: failed to get valid FBConfig"),
		errorEntry(error_e::linuxNoHDRConfig, "Linux Error: failed to get HDR config"),
		errorEntry(error_e::windowsCannotCreateWindows, "Windows Error: failed to create window"),
		//errorEntry(error_e::windowsFullscreenBadDualView, "Windows Error: bad dual view value for fullscreen"),
		errorEntry(error_e::windowsFullscreenBadFlags, "Windows Error: Bad display change flags"),
		errorEntry(error_e::windowsFullscreenBadMode, "Windows Error: Bad display change mode"),
		errorEntry(error_e::WindowsFullscreenBadParam, "Windows Error: Bad display change Parameter"),
		errorEntry(error_e::WindowsFullscreenChangeFailed, "Windows Error: The display driver failed to implement the specified graphics mode"),
		errorEntry(error_e::WindowsFullscreenNotUpdated, "Windows Error: Unable to write settings to the registry"),
		errorEntry(error_e::WindowsFullscreenNeedRestart, "Windows Error: The computer must be restarted for the graphics mode to work"),
		errorEntry(error_e::windowsFunctionNotImplemented, "Windows Error: function not implemented on Windows platform yet"),
		errorEntry(error_e::success, "function call was successful"),
	};

	using key_c			 = std::function<void(const tWindow* window, const uint16_t& key, const keyState_e& keyState)>;
	using focus_c		 = std::function<void(const tWindow* window, const bool& isFocused)>;
	using moved_c		 = std::function<void(const tWindow* window, const vec2_t<int16_t>& windowPosition)>;
	using resize_c		 = std::function<void(const tWindow* window, const vec2_t<uint16_t>& windowResolution)>;
	using fileDrop_c	 = std::function<void(const tWindow* window, const std::vector<std::string>& files, const vec2_t<int16_t>& windowMousePosition)>;
	using destroyed_c	 = std::function<void(const tWindow* window)>;
	using maximized_c	 = std::function<void(const tWindow* window)>;
	using minimized_c	 = std::function<void(const tWindow* window)>;
	using mouseMove_c	 = std::function<void(const tWindow* window, const vec2_t<int16_t>& windowMousePosition, const vec2_t<int16_t>& screenMousePosition)>;
	using mouseWheel_c	 = std::function<void(const tWindow* window, const mouseScroll_e& mouseScrollDirection)>;
	using mouseButton_c  = std::function<void(const tWindow* window, const mouseButton_e& mouseButton, const buttonState_e& buttonState)>;
	using windowError_c  = std::function<void(const tWindow* window, const std::string& entry)>;
	using managerError_c = std::function<void(const std::string& entry)>;

	class tWindow
	{
		friend class windowManager;

		using key_c			 = std::function<void(const tWindow* window, uint16_t key, keyState_e keyState)>;
		using focus_c		 = std::function<void(const tWindow* window, bool isFocused)>;
		using moved_c		 = std::function<void(const tWindow* window, vec2_t<int16_t> windowPosition)>;
		using resize_c		 = std::function<void(const tWindow* window, vec2_t<uint16_t> windowResolution)>;
		using mouseMove_c	 = std::function<void(const tWindow* window, vec2_t<int16_t> windowMousePosition, vec2_t<int16_t> screenMousePosition)>;
		using destroyed_c	 = std::function<void(const tWindow* window)>;
		using maximized_c	 = std::function<void(const tWindow* window)>;
		using minimized_c	 = std::function<void(const tWindow* window)>;
		using mouseWheel_c	 = std::function<void(const tWindow* window, const mouseScroll_e& mouseScrollDirection)>;
		using mouseButton_c	 = std::function<void(const tWindow* window, const mouseButton_e& mouseButton, const buttonState_e& buttonState)>;

	public:
		const bool& GetIsFocused() const { return inFocus; }
		const keyState_e* GetKeyState() const { return keys; }
		vec2_t<int16_t> GetPosition() const { return position; }
		const bool& GetShouldClose() const { return shouldClose; }
		const bool& GetIsFullscreen() const { return isFullscreen; }
		const bool& GetIsInitialized() const { return initialized; }
		const windowSetting_t& GetSettings() const { return settings; }
		const uint32_t& GetCurrentStyle() const { return currentStyle; }
		const bool& GetContextCreated() const { return contextCreated; }
		const bool& GetIsCurrentContext() const { return isCurrentContext; }
		const monitor_t* GetCurrentMonitor() const { return currentMonitor; }
		const buttonState_e* GetMouseButtonState() const { return mouseButton; }
		const vec2_t<int16_t>& GetMousePosition() const { return mousePosition; }
		const uint16_t& GetCurrentScreenIndex() const { return currentScreenIndex; }
		const vec2_t<int16_t>& GetPreviousPosition() const { return previousPosition; }
		const vec2_t<uint16_t>& GetPreviousDimensions() const { return previousDimensions; }
		const vec2_t<int16_t>& GetPreviousMousePosition() const { return previousMousePosition; }

		void SetShouldClose(const bool& inShouldClose) { shouldClose = inShouldClose; }

	private:
		bool inFocus;						/**< Whether the Window is currently in focus(if it is the current window be used) */
		bool shouldClose;					/**< Whether the Window should be closing */
		bool initialized;					/**< Whether the window has been successfully initialized */
		bool isFullscreen;					/**< Whether the window is currently in fullscreen mode */
		bool contextCreated;				/**< Whether the OpenGL context has been successfully created */
		bool isCurrentContext;				/**< Whether the window is the current window being drawn to */
		uint32_t currentStyle;				/**< The current style of the window */
		uint32_t previousStyle;				/**< The previous style of the window (for restoration) */
		keyState_e keys[last];				/**< Record of keys that are either pressed or released in the respective window */
		vec2_t<int16_t> position;			/**< Position of the Window relative to the screen co-ordinates */
		windowSetting_t settings;			/**< List of User-defined settings for this windowS */
		monitor_t* currentMonitor;			/**< The monitor that the window is currently rendering to */
		uint16_t currentScreenIndex;		/**< The Index of the screen currently being rendered to (fullscreen) */
		vec2_t<int16_t> mousePosition;		/**< Position of the Mouse cursor relative to the window co-ordinates */
		vec2_t<int16_t> previousPosition;	/**< Previous position of the window before being set as Fullscreen */
		vec2_t<uint16_t> previousDimensions;
		vec2_t<int16_t> previousMousePosition;
		buttonState_e mouseButton[(uint16_t)mouseButton_e::last] {};
		/**< Record of mouse buttons that are either presses or released */

#if defined(TW_USE_VULKAN)
		VkInstance vulkanInstanceHandle;
		VkSurfaceKHR vulkanSurfaceHandle;
#endif

#if defined(TW_WINDOWS)

		HDC deviceContextHandle;					 /**< A handle to a device context */
		HGLRC glRenderingContextHandle;				 /**< A handle to an OpenGL rendering context*/
		HPALETTE paletteHandle;						 /**< A handle to a Win32 palette*/
		PIXELFORMATDESCRIPTOR pixelFormatDescriptor; /**< Describes the pixel format of a drawing surface*/
		WNDCLASS windowClass;						 /**< Contains the window class attributes */
		HWND windowHandle;							 /**< A handle to A window */
		HINSTANCE instanceHandle;					 /**< A handle to the window class instance */
		int accumWheelDelta;						 /**< holds the accumulated mouse wheel delta for this window */
		vec2_t<uint16_t> clientArea;				/**< the width and height of the client window */

#elif defined(TW_LINUX)

		int16_t* attributes;				/**< Attributes of the window. RGB, depth, stencil, etc */
		GLXContext context;					/**< The handle to the GLX rendering context */
		Window windowHandle;				/**< The X11 handle to the window. I wish they didn't name the type 'Window' */
		XVisualInfo* visualInfo;			/**< The handle to the Visual Information. similar purpose to PixelformatDesriptor */
		Display* currentDisplay;			/**< Handle to the X11 window */
		uint16_t linuxDecorators;			/**< Enabled window decorators */
		XSetWindowAttributes setAttributes;	/**< The attributes to be set for the window */

		/* these atoms are needed to change window states via the extended window manager */
		Atom AtomIcon;			   /**< Atom for the icon of the window */
		Atom AtomHints;			   /**< Atom for the window decorations */
		Atom AtomClose;			   /**< Atom for closing the window */
		Atom AtomActive;		   /**< Atom for the active window */
		Atom AtomCardinal;		   /**< Atom for cardinal coordinates */
		Atom AtomFullScreen;	   /**< Atom for the full screen state of the window */
		Atom AtomDesktopGeometry;  /**< Atom for Desktop Geometry */
		Atom AtomDemandsAttention; /**< Atom for when the window demands attention */

		Atom AtomState;
		Atom AtomStateMaximizedVert;
		Atom AtomStateMaximizedHorz;
		Atom AtomStateHidden;

		//DND Atoms
		Atom AtomXDNDAware;		  /**< Atom for making the window Drag and Drop aware */
		Atom AtomXDNDEnter;		  /**< Atom for when a Drag and Drop selection enters the client area */
		Atom AtomXDNDPosition;	  /**< Atom the position of the mouse when a Drag and Drop Event occurs */
		Atom AtomXDNDStatus;	  /**< Atom for the DND status when a drag and drop event occurs */
		Atom AtomXDNDDrop;		  /**< Atom for when a Drag and Drop selection occurs */
		Atom AtomXDNDFinished;	  /**< Atom for when a DND event finishes */
		Atom AtomXDNDLeave;		  /**< Atom for when the mouse leaves the window client area during a DND event */
		Atom AtomXDNDSelection;	  /**< Atom for the DND selection of files when a DND event occurs */
		Atom AtomXDNDTextUriList; /**< Atom for the list of file strings when a DND event occurs */

		//Action Atoms
		Atom AtomXDNDActionCopy;
		Atom AtomXDNDActionMove;
		Atom AtomXDNDActionLink;
		Atom AtomXDNDActionAsk;
		Atom AtomXDNDActionPrivate;

		enum decorator_e
		{
			linuxBorder	  = 1L << 1,
			linuxMove	  = 1L << 2,
			linuxMinimize = 1L << 3,
			linuxMaximize = 1L << 4,
			linuxClose	  = 1L << 5,
		};

		enum hint_e
		{
			function = 1,
			decorator,
		};

		void InitializeAtoms()
		{
			AtomIcon			 = XInternAtom(currentDisplay, "_NET_WM_ICON", false);
			AtomHints			 = XInternAtom(currentDisplay, "_MOTIF_WM_HINTS", true);
			AtomClose			 = XInternAtom(currentDisplay, "WM_DELETE_WINDOW", false);
			AtomActive			 = XInternAtom(currentDisplay, "_NET_ACTIVE_WINDOW", false);
			AtomCardinal		 = XInternAtom(currentDisplay, "CARDINAL", false);
			AtomFullScreen		 = XInternAtom(currentDisplay, "_NET_WM_STATE_FULLSCREEN", false);
			AtomDesktopGeometry	 = XInternAtom(currentDisplay, "_NET_DESKTOP_GEOMETRY", false);
			AtomDemandsAttention = XInternAtom(currentDisplay, "_NET_WM_STATE_DEMANDS_ATTENTION", false);

			AtomState				  = XInternAtom(currentDisplay, "_NET_WM_STATE", false);
			AtomStateMaximizedVert	  = XInternAtom(currentDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", false);
			AtomStateMaximizedHorz	  = XInternAtom(currentDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
			AtomStateHidden			  = XInternAtom(currentDisplay, "_NET_WM_STATE_HIDDEN", false);

			AtomXDNDAware		= XInternAtom(currentDisplay, "XdndAware", false);
			AtomXDNDEnter		= XInternAtom(currentDisplay, "XdndEnter", false);
			AtomXDNDPosition	= XInternAtom(currentDisplay, "xdndPosition", false);
			AtomXDNDStatus		= XInternAtom(currentDisplay, "xdndStatus", false);
			AtomXDNDDrop		= XInternAtom(currentDisplay, "xdndDrop", false);
			AtomXDNDFinished	= XInternAtom(currentDisplay, "xdndFinished", false);
			AtomXDNDLeave		= XInternAtom(currentDisplay, "xdndLeave", false);
			AtomXDNDSelection	= XInternAtom(currentDisplay, "xdndSelection", false);
			AtomXDNDTextUriList = XInternAtom(currentDisplay, "text/uri-list", false);

			AtomXDNDActionCopy	  = XInternAtom(currentDisplay, "XdndActionCopy", false);
			AtomXDNDActionMove	  = XInternAtom(currentDisplay, "XdndActionMove", false);
			AtomXDNDActionLink	  = XInternAtom(currentDisplay, "XdndActionLink", false);
			AtomXDNDActionAsk	  = XInternAtom(currentDisplay, "XdndActionAsk", false);
			AtomXDNDActionPrivate = XInternAtom(currentDisplay, "XdndActionPrivate", false);
		}

#endif

	public:
		explicit tWindow(const windowSetting_t& windowSetting)
		{
			this->settings = windowSetting;

			shouldClose		   = false;
			initialized		   = false;
			contextCreated	   = false;
			currentStyle	   = titleBar | icon | border | minimizeButton | maximizeButton | closeButton | sizeableBorder;
			inFocus			   = false;
			isCurrentContext   = false;
			currentScreenIndex = 0;
			isFullscreen	   = false;
			currentMonitor	   = nullptr;

			std::fill(keys, keys + last, keyState_e::up);// = { keyState_e.bad };
			std::fill_n(mouseButton, static_cast<uint16_t>(mouseButton_e::last), buttonState_e::up);

#if defined(TW_WINDOWS)
			deviceContextHandle		 = nullptr;
			glRenderingContextHandle = nullptr;
			paletteHandle			 = nullptr;
			pixelFormatDescriptor	 = PIXELFORMATDESCRIPTOR();
			windowClass				 = WNDCLASS();
			windowHandle			 = nullptr;
			instanceHandle			 = nullptr;
			accumWheelDelta			 = 0;
			clientArea				 = vec2_t<uint16_t>::Zero();
#endif

#if defined(__linux__)
			context = nullptr;
#endif
		}

	private:
#pragma region Windows
#if defined(TW_WINDOWS)

		// if windows is defined then allow the user to only GET the necessary info
		inline HDC GetDeviceContextDeviceHandle() { return deviceContextHandle; }

		inline HGLRC GetGLRenderingContextHandle() { return glRenderingContextHandle; }

		inline HWND GetWindowHandle() { return windowHandle; }

		inline HINSTANCE GetWindowClassInstance() { return instanceHandle; }

#endif
#pragma endregion

#pragma region Linux
#if defined(TW_LINUX)

		Window GetWindowHandle() const { return windowHandle; }

		GLXContext GetGLXContext() const { return context; }

		Display* GetCurrentDisplay() const { return currentDisplay; }

#pragma endregion
#endif
	};

	class windowManager
	{
	public:
		key_c keyEvent; /**< This is the callback to be used when a key has been pressed */
		focus_c focusEvent; /**< This is the callback to be used when the window has been given focus in a non-programmatic fashion */
		moved_c movedEvent; /**< This is the callback to be used the window has been moved in a non-programmatic fashion */
		resize_c resizeEvent; /**< This is a callback to be used when the window has been resized in a non-programmatic fashion */
		fileDrop_c fileDropEvent; /**< This is a callback to be used when files have been dragged onto a window */
		destroyed_c destroyedEvent; /**< This is the callback to be used when the window has been closed in a non-programmatic fashion */
		maximized_c maximizedEvent; /**< This is the callback to be used when the window has been maximized in a non-programmatic fashion */
		minimized_c minimizedEvent; /**< This is the callback to be used when the window has been minimized in a non-programmatic fashion */
		mouseMove_c mouseMoveEvent; /**< This is a callback to be used when the mouse has been moved */
		mouseWheel_c mouseWheelEvent; /**< This is the callback to be used when the mouse wheel has been scrolled. */
		mouseButton_c mouseButtonEvent; /**< This is the callback to be used when a mouse button has been pressed */
		windowError_c windowErrorEvent; /**< This is the callback to be used when an error has occurred */
		managerError_c managerErrorEvent; /**< This is the callback to be used when a manager specific error has occurred */

		windowManager() = default;

		/**
		* Shutdown and delete all windows in the manager
		*/
		~windowManager()
		{
			ShutDown();
		}

		void Initialize()
		{
#if defined(TW_WINDOWS)
			HWND desktopHandle = GetDesktopWindow();

			if (desktopHandle)
			{
				bestPixelFormat = nullptr;
				GetScreenInfo();
				CreateDummyContext();
				InitExtensions();
				{
					// delete the dummy context and make the current context null
					wglMakeCurrent(dummyDeviceContextHandle, nullptr);
					wglMakeCurrent(dummyDeviceContextHandle, nullptr);
					wglDeleteContext(dummyGLContextHandle);
					ShutdownDummy();
				}

				//gamepadList.resize(4, nullptr);
				//Windows_InitGamepad();
			}
#elif defined(TW_LINUX)
			glxSwapIntervalMESA = nullptr;
			glxSwapIntervalEXT	= nullptr;
			currentEvent		= XEvent();
			currentDisplay		= XOpenDisplay(nullptr);

			if (!currentDisplay)
			{
				return;
			}

			GetScreenInfo();
			InitExtensions();
#endif
		}

		/**
		* Use this to shut down the window manager when your program is finished
		*/
		void ShutDown()
		{
#if defined(TW_WINDOWS)
			ResetMonitors();
#elif defined(TW_LINUX)
			Linux_Shutdown();
#endif

			for (auto& windowIndex: windowList)
			{
				ShutdownWindow(windowIndex.get());
			}
			windowList.clear();
		}

		/**
		* Use this to add a window to the manager
		*/
		tWindow* AddWindow(const windowSetting_t& windowSetting)
		{
			if (windowSetting.name.empty() == false)
			{
				std::unique_ptr<tWindow> newWindow(new tWindow(windowSetting));
				windowList.push_back(std::move(newWindow));
				InitializeWindow(windowList.back().get());

				return windowList.back().get();
			}
			return nullptr;
		}

		tWindow* AddSharedWindow(tWindow* sourceWindow, const windowSetting_t& windowSetting)
		{
			if (windowSetting.name.empty() == false)
			{
				std::unique_ptr<tWindow> newWindow(new tWindow(windowSetting));
				windowList.push_back(std::move(newWindow));
				InitializeWindow(windowList.back().get());
				ShareContexts(sourceWindow, windowList.back().get());

				return windowList.back().get();
			}
			return nullptr;
		}

		/**
		* Return the total amount of windows the manager has
		*/
		int8_t GetNumWindows() const { return static_cast<int8_t>(windowList.size()); }

		/**
		* Return the mouse position in screen co-ordinates
		*/
		vec2_t<int16_t> GetMousePositionInScreen() const { return screenMousePosition; }

		/**
		* Set the position of the mouse cursor relative to screen co-ordinates
		*/
		void SetMousePositionInScreen(const vec2_t<int16_t> mousePosition)
		{
			screenMousePosition.x = mousePosition.x;
			screenMousePosition.y = mousePosition.y;

#if defined(TW_WINDOWS)
			SetCursorPos(screenMousePosition.x, screenMousePosition.y);
#elif defined(TW_LINUX)
			/*XWarpPointer(currentDisplay, None,
			XDefaultRootWindow(currentDisplay), 0, 0,
			screenResolution.x,
			screenResolution.y,
			screenMousePosition.x, screenMousePosition.y);*/
			AddErrorLog(error_e::linuxFunctionNotImplemented);
#endif
		}

		/**
		* Ask the window manager to poll for events
		*/
		void PollForEvents()
		{
#if defined(TW_WINDOWS)
			// only process events if there are any to process
			while (PeekMessage(&winMessage, nullptr, 0, 0, PM_REMOVE))
			{
				// the only place I can see this being needed if someone called
				// PostQuitMessage manually
				TranslateMessage(&winMessage);
				DispatchMessage(&winMessage);
				if (winMessage.message == WM_QUIT)
				{
					ShutDown();
				}
			}

#elif defined(TW_LINUX)
			// if there are any events to process
			while (XEventsQueued(currentDisplay, QueuedAfterReading) > 0)
			{
				XEvent currentEvent;
				XNextEvent(currentDisplay, &currentEvent);
				Linux_ProcessEvents(currentEvent);
			}
#endif

#if !defined(TW_NO_GAMEPAD_POLL)
			//PollGamepads();
#endif
		}

		/**
		* Ask the window manager to wait for events
		*/
		void WaitForEvents()
		{
#if defined(TW_WINDOWS)
			// process even if there aren't any to process
			GetMessage(&winMessage, nullptr, 0, 0);
			TranslateMessage(&winMessage);
			DispatchMessage(&winMessage);
			if (winMessage.message == WM_QUIT)
			{
				ShutDown();
				return;
			}
#elif defined(TW_LINUX)
			// even if there aren't any events to process
			XNextEvent(currentDisplay, &currentEvent);
			Linux_ProcessEvents(currentEvent);
#endif
		}

		/**
		* Remove window from the manager by name
		*/
		void RemoveWindow(tWindow* window)
		{
			if (window != nullptr)
			{
				ShutdownWindow(window);
			}
			AddErrorLog(error_e::windowInvalid, window, __LINE__, __func__);
		}

		/**
		* Set window swap interval
		*/
		void SetWindowSwapInterval(tWindow* window, int interval)
		{
#if defined(TW_WINDOWS)
			if (swapControlEXT && wglSwapIntervalEXT != nullptr)
			{
				HGLRC previousGLContext	  = wglGetCurrentContext();
				HDC previousDeviceContext = wglGetCurrentDC();
				wglMakeCurrent(window->deviceContextHandle, window->glRenderingContextHandle);
				wglSwapIntervalEXT(interval);
				wglMakeCurrent(previousDeviceContext, previousGLContext);
			}
#elif defined(TW_LINUX)
			if (glxSwapIntervalMESA != nullptr)
			{
				// set previous context
				GLXContext previousGLContext = glXGetCurrentContext();
				// get previous Display
				Display* previousDisplay = glXGetCurrentDisplay();

				glXMakeCurrent(window->currentDisplay, window->windowHandle, window->context);
				int result = 0;
				if (glxSwapIntervalMESA != nullptr) result = glxSwapIntervalMESA(interval);
				else if (glxSwapIntervalEXT != nullptr) glxSwapIntervalEXT(window->currentDisplay, window->windowHandle, interval);

				if (result != 0)
				{
					AddErrorLog(error_e::invalidInterval, window, __LINE__, __func__);
					return;
				}
				glXMakeCurrent(previousDisplay, window->windowHandle, previousGLContext);
			}
#endif
		}

		/**
		* Get the swap interval (V-Sync) of the given window
		*/
		int GetWindowSwapInterval(tWindow* window) const
		{
#if defined(TW_WINDOWS)

			if (wglGetSwapIntervalEXT && swapControlEXT)
			{
				HGLRC previousGLContext	  = wglGetCurrentContext();
				HDC previousDeviceContext = wglGetCurrentDC();
				wglMakeCurrent(window->deviceContextHandle, window->glRenderingContextHandle);
				int interval = wglGetSwapIntervalEXT();
				wglMakeCurrent(previousDeviceContext, previousGLContext);
				return interval;
			}
#elif defined(TW_LINUX)

			// set previous context
			GLXContext previousGLContext = glXGetCurrentContext();
			// get previous Display
			Display* previousDisplay = glXGetCurrentDisplay();

			glXMakeCurrent(window->currentDisplay, window->windowHandle, window->context);
			int interval = 0;

			if (glXGetSwapIntervalMESA != nullptr) interval = glXGetSwapIntervalMESA();

			glXMakeCurrent(previousDisplay, window->windowHandle, previousGLContext);
			return interval;
#endif
		}

		/**
		* Get the list of monitors connected to the system
		*/
		std::vector<monitor_t> GetMonitors() const { return monitorList; }

		std::vector<std::string> GetClipboardLatest(tWindow* window, clipboard_e& clipType)
		{
#if defined(TW_WINDOWS)

#endif
#if defined(TW_LINUX)
			return Linux_GetClipboardLatest(window, clipType);
#endif
			//AddErrorLog(error_e::functionNotImplemented);
		}

		/**
		* Set the Size/Resolution of the given window
		*/
		void SetWindowSize(tWindow* window, vec2_t<uint16_t> newResolution) const
		{
			window->previousDimensions	= window->settings.resolution;
			window->settings.resolution = newResolution;
#if defined(TW_WINDOWS)
			SetWindowPos(window->windowHandle, HWND_TOP, window->position.x, window->position.y, newResolution.x, newResolution.y, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
#elif defined(TW_LINUX)
			XResizeWindow(currentDisplay, window->windowHandle, newResolution.x, newResolution.y);
#endif
			//TODO: add error handling
		}

		/**
		* Set the Position of the given window relative to screen co-ordinates
		*/
		void SetPosition(tWindow* window, vec2_t<int16_t> newPosition) const //lol, sure
		{
			window->previousPosition = window->position;
			window->position		 = newPosition;

#if defined(TW_WINDOWS)
			SetWindowPos(window->windowHandle, HWND_TOP, newPosition.x, newPosition.y, window->settings.resolution.x, window->settings.resolution.y, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
#elif defined(TW_LINUX)
			XWindowChanges windowChanges;

			windowChanges.x = newPosition.x;
			windowChanges.y = newPosition.y;

			int result = XConfigureWindow(currentDisplay, window->windowHandle, CWX | CWY, &windowChanges);
#endif

			//TODO: add code for error handling
		}

		/**
		* Set the mouse Position of the given window's co-ordinates
		*/
		void SetMousePosition(tWindow* window, const vec2_t<int16_t>& newMousePosition) const
		{
			window->mousePosition.x = newMousePosition.x;
			window->mousePosition.y = newMousePosition.y;
#if defined(TW_WINDOWS)
			POINT mousePoint;
			mousePoint.x = newMousePosition.x;
			mousePoint.y = newMousePosition.y;
			ClientToScreen(window->windowHandle, &mousePoint);
			SetCursorPos(mousePoint.x, mousePoint.y);
#elif defined(TW_LINUX)
			XWarpPointer(currentDisplay, window->windowHandle, window->windowHandle, window->position.x, window->position.y, window->settings.resolution.width, window->settings.resolution.height, newMousePosition.x, newMousePosition.y);
#endif
		}

#if defined(TW_USE_VULKAN)// these are hidden if TW_USE_VULKAN is not defined
		// get reference to instance
		inline VkInstance& GetVulkanInstance() const { return vulkanInstanceHandle; }

		// get reference to surface
		inline VkSurfaceKHR& GetVulkanSurface() const { return vulkanSurfaceHandle; }
#else

		/**
		* Swap the draw buffers of the given window.
		*/
		void SwapDrawBuffers(const tWindow* window) const
		{
#if defined(TW_WINDOWS)
			SwapBuffers(window->deviceContextHandle);
#elif defined(TW_LINUX)
			glXSwapBuffers(currentDisplay, window->windowHandle);
#endif
		}

		/**
		* Make the given window be the current OpenGL Context to be drawn to
		*/
		void MakeCurrentContext(const tWindow* window) const
		{
#if defined(TW_WINDOWS)
			wglMakeCurrent(window->deviceContextHandle, window->glRenderingContextHandle);
#elif defined(TW_LINUX)
			glXMakeCurrent(currentDisplay, window->windowHandle, window->context);
#endif
		}

#endif

		/**
		* Toggle the minimization state of the given window
		*/
		void Minimize(tWindow* window, const bool& newState) const
		{
			if (newState)
			{
				window->settings.currentState = state_e::minimized;

#if defined(TW_WINDOWS)
				ShowWindow(window->windowHandle, SW_MINIMIZE);
#elif defined(TW_LINUX)
				XIconifyWindow(currentDisplay, window->windowHandle, 0);
#endif
			}
			else
			{
				window->settings.currentState = state_e::normal;
#if defined(TW_WINDOWS)
				ShowWindow(window->windowHandle, SW_RESTORE);
#elif defined(TW_LINUX)
				XMapWindow(currentDisplay, window->windowHandle);
				//TODO: impl error branch
#endif
			}
		}

		/**
		* Toggle the maximization state of the current window
		*/
		void Maximize(tWindow* window, const bool& newState) const
		{
			if (newState)
			{
				window->settings.currentState = state_e::maximized;
#if defined(TW_WINDOWS)
				ShowWindow(window->windowHandle, SW_MAXIMIZE);
#elif defined(TW_LINUX)
				XEvent currentEvent = {};

				currentEvent.xany.type			  = ClientMessage;
				currentEvent.xclient.message_type = window->AtomState;
				currentEvent.xclient.format		  = 32;
				currentEvent.xclient.window		  = window->windowHandle;
				currentEvent.xclient.data.l[0]	  = true;
				currentEvent.xclient.data.l[1]	  = (long)window->AtomStateMaximizedVert;
				currentEvent.xclient.data.l[2]	  = (long)window->AtomStateMaximizedVert;

				XSendEvent(currentDisplay, window->windowHandle, 0, SubstructureNotifyMask, &currentEvent);
#endif
			}
			else
			{
				window->settings.currentState = state_e::normal;
#if defined(TW_WINDOWS)
				ShowWindow(window->windowHandle, SW_RESTORE);
#elif defined(TW_LINUX)
				XEvent currentEvent = {};

				currentEvent.xany.type			  = ClientMessage;
				currentEvent.xclient.message_type = window->AtomState;
				currentEvent.xclient.format		  = 32;
				currentEvent.xclient.window		  = window->windowHandle;
				currentEvent.xclient.data.l[0]	  = false;
				currentEvent.xclient.data.l[1]	  = (long)window->AtomStateMaximizedVert;
				currentEvent.xclient.data.l[2]	  = (long)window->AtomStateMaximizedVert;

				XSendEvent(currentDisplay, window->windowHandle, 0, SubstructureNotifyMask, &currentEvent);
#endif
			}
		}

		/**
		* Toggle the given window's full screen mode
		*/
		void SetFullScreen(tWindow* window, const bool& newState /* need to add definition for which screen*/) const
		{
			window->settings.currentState = newState ? state_e::fullscreen : state_e::normal;

#if defined(TW_WINDOWS)

			SetWindowLongPtr(window->windowHandle, GWL_STYLE, static_cast<LONG_PTR>(WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE));

			RECT desktop;
			GetWindowRect(window->windowHandle, &desktop);
			MoveWindow(window->windowHandle, 0, 0, desktop.right, desktop.bottom, true);

#elif defined(TW_LINUX)

			XEvent currentEvent;
			memset(&currentEvent, 0, sizeof(currentEvent));

			currentEvent.xany.type			  = ClientMessage;
			currentEvent.xclient.message_type = window->AtomState;
			currentEvent.xclient.format		  = 32;
			currentEvent.xclient.window		  = window->windowHandle;
			currentEvent.xclient.data.l[0]	  = window->settings.currentState == state_e::fullscreen;
			currentEvent.xclient.data.l[1]	  = (long)window->AtomFullScreen;

			XSendEvent(currentDisplay, window->windowHandle, 0, SubstructureNotifyMask, &currentEvent);

			//put a error handling path here
#endif
		}

		/**
		* Toggles full-screen mode for a window by parsing in a monitor and a monitor setting index
		*/
		void ToggleFullscreen(tWindow* window, monitor_t* monitor, const uint16_t& monitorSettingIndex)
		{

			monitor->previousSetting = monitor->currentSetting;
			monitor->currentSetting = monitor->settings[monitorSettingIndex];
#if defined(TW_WINDOWS)
			Windows_ToggleFullscreen(window, monitor, monitorSettingIndex);
#elif defined(TW_LINUX)
			Linux_ToggleFullscreen(window, monitor, monitorSettingIndex);
#endif
			window->isFullscreen = !window->isFullscreen;
		}

		/**
		* Set the window title bar
		*/
		void SetTitleBar(tWindow* window, const char* newTitle)
		{
			if (newTitle != nullptr)
			{
#if defined(TW_WINDOWS)
				SetWindowText(window->windowHandle, (wchar_t*)newTitle);
#elif defined(TW_LINUX)
				XStoreName(currentDisplay, window->windowHandle, newTitle);
#endif
			}
			AddErrorLog(TinyWindow::error_e::invalidTitlebar, window, __LINE__, __func__);
		}

		/**
		* Set the window icon (currently not functional)
		*/
		void SetIcon(tWindow* window)
		{
			// const char* windowName, const char* icon, uint16_t width, uint16_t height
			AddErrorLog(error_e::functionNotImplemented, window);
		}

		/**
		* Set the window to be in focus
		*/
		void Focus(tWindow* window, const bool& newState) const
		{
			if (newState)
			{
#if defined(TW_WINDOWS)
				SetFocus(window->windowHandle);
#elif defined(TW_LINUX)
				XMapWindow(currentDisplay, window->windowHandle);
#endif
			}
			else
			{
#if defined(_WIN32) || defined(_WIN64)
				SetFocus(nullptr);
#elif defined(TW_LINUX)
				XUnmapWindow(currentDisplay, window->windowHandle);
#endif
			}
		}

		/**
		* Restore the window
		*/
		void Restore(tWindow* window) const
		{
#if defined(TW_WINDOWS)
			ShowWindow(window->windowHandle, SW_RESTORE);
#elif defined(TW_LINUX)
			XMapWindow(currentDisplay, window->windowHandle);
#endif
		}

		/**
		* Set window decorators
		*/
		void SetDecorators(tWindow* window, const uint16_t& decorators) const
		{
			window->previousStyle = window->currentStyle;

#if defined(TW_WINDOWS)
			if (decorators & border)
			{
				window->currentStyle |= WS_BORDER;
			}
			if (decorators & titleBar)
			{
				window->currentStyle |= WS_CAPTION;
			}
			if (decorators & icon)
			{
				window->currentStyle |= WS_ICONIC;
			}
			if (decorators & closeButton)
			{
				window->currentStyle |= WS_SYSMENU;
			}
			if (decorators & minimizeButton)
			{
				window->currentStyle |= WS_MINIMIZEBOX;
			}
			if (decorators & maximizeButton)
			{
				window->currentStyle |= WS_MAXIMIZEBOX;
			}
			if (decorators & sizeableBorder)
			{
				window->currentStyle |= WS_THICKFRAME;
			}

			SetWindowLongPtr(window->windowHandle, GWL_STYLE, static_cast<LONG_PTR>(window->currentStyle | WS_VISIBLE));
			SetWindowPos(window->windowHandle, HWND_TOPMOST, window->position.x, window->position.y, window->settings.resolution.width, window->settings.resolution.height, SWP_FRAMECHANGED);
#elif defined(TW_LINUX)

			MWMHints_t hints;
			hints.flags		  = MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS;
			hints.decorations = 0;
			hints.functions	  = window->linuxDecorators;

			if (decorators & titleBar)
			{
				hints.decorations |= MWM_DECOR_TITLE;
				hints.functions |= MWM_FUNC_MOVE;
			}
			if (decorators & border)
			{
				hints.decorations |= MWM_DECOR_BORDER;
			}
			if (decorators & minimizeButton)
			{
				hints.decorations |= MWM_DECOR_MINIMIZE;
				hints.functions |= MWM_FUNC_MINIMIZE;
			}
			if (decorators & maximizeButton)
			{
				hints.decorations |= MWM_DECOR_MAXIMIZE;
				hints.functions |= MWM_FUNC_MAXIMIZE;
			}
			if (decorators & closeButton)
			{
				hints.functions |= MWM_FUNC_CLOSE;
			}
			if (decorators & sizeableBorder)
			{
				hints.decorations |= MWM_DECOR_RESIZE;
				hints.functions |= MWM_FUNC_RESIZE;
			}

			XChangeProperty(currentDisplay, window->windowHandle, window->AtomHints, window->AtomHints, 32, PropModeReplace, (unsigned char*)&hints, sizeof(MWMHints_t) / sizeof(long));
			XMapWindow(currentDisplay, window->windowHandle);
#endif

			window->currentStyle = decorators;
		}

		const std::vector<errorEntry>& GetErrorLog() { return errorLog; }

		const std::vector<formatSetting_t>* GetFormatList() const
		{
			return &formatList;
		}

	private:
		vec2_t<int16_t> screenMousePosition;
		std::vector<monitor_t> monitorList;
		std::vector<formatSetting_t> formatList;
		std::vector<std::unique_ptr<tWindow>> windowList;// replace with unordered map <handle, window?>?
		std::vector<errorEntry> errorLog;

		template<typename T = void>
		void AddErrorLog(const error_e& newError, T* obj = nullptr, const uint16_t& fileLine = __LINE__, const std::string& functionName = __FUNCTION__)
		{
			auto newString = errorLUT.at(newError);

			//add to string then send it along
			newString.append(" | in function: %s ");
			newString.append(functionName);
			newString.append("at line %i \n");
			newString.append(std::to_string(fileLine));

			const auto newEntry = errorEntry(newError, newString);

			errorLog.push_back(newEntry);
			if constexpr (std::is_same_v<T, tWindow>)
			{
				if (windowErrorEvent != nullptr)
				{
					windowErrorEvent(obj, newEntry.second);
				}
			}

			else
			{
				if (managerErrorEvent != nullptr)
				{
					managerErrorEvent(newEntry.second);
				}
			}
		}

		void CreateDummyContext()
		{
#if defined(TW_WINDOWS)
			Windows_CreateDummyContext();
#elif defined(TW_LINUX)
			//return error_e::success;// TODO: flesh this out?
			AddErrorLog(error_e::linuxFunctionNotImplemented);
#endif
		}

		void InitExtensions()
		{
#if defined(TW_WINDOWS)
			Windows_InitExtensions();
#elif defined(TW_LINUX)
			Linux_InitExtensions();
#endif
		}

		void InitializeWindow(tWindow* window)
		{
#if defined(TW_WINDOWS)
			Windows_InitializeWindow(window);
#elif defined(TW_LINUX)
			Linux_InitializeWindow(window);
#endif
		}

		void InitializeGL(tWindow* window)
		{
#if defined(TW_WINDOWS)
			Windows_InitGL(window);
#elif defined(TW_LINUX)
			Linux_InitGL(window);
#endif
		}

		void GetScreenInfo()
		{
#if defined(TW_WINDOWS)
			Windows_GetScreenInfo();
#elif defined(TW_LINUX)
			InitXRandR();
			InitializeAtoms();
#endif
		}

		void CheckWindowScreen(tWindow* window)
		{
#if defined(TW_WINDOWS)
			// for each monitor
			for (auto& monitorIndex: monitorList)
			{
				if (monitorIndex.monitorHandle == MonitorFromWindow(window->windowHandle, MONITOR_DEFAULTTONEAREST))
				{
					window->currentMonitor = &monitorIndex;
				}
			}
#endif

#if defined(TW_LINUX)
			AddErrorLog(error_e::linuxFunctionNotImplemented);
#endif
		}

		void ShutdownWindow(tWindow* window)
		{
			if (destroyedEvent != nullptr)
			{
				destroyedEvent(window);
			}

#if defined(TW_WINDOWS)
			window->shouldClose = true;
			if (window->glRenderingContextHandle)
			{
				wglMakeCurrent(nullptr, nullptr);
				wglDeleteContext(window->glRenderingContextHandle);
			}

			if (window->paletteHandle)
			{
				DeleteObject(window->paletteHandle);
			}

			ReleaseDC(window->windowHandle, window->deviceContextHandle);
			UnregisterClass((wchar_t*)window->settings.name.c_str(), window->instanceHandle);

			FreeModule(window->instanceHandle);

			window->deviceContextHandle		 = nullptr;
			window->windowHandle			 = nullptr;
			window->glRenderingContextHandle = nullptr;

#elif defined(TW_LINUX)
			if (window->settings.currentState == state_e::fullscreen)
				Restore(window);

			glXDestroyContext(currentDisplay, window->context);
			XUnmapWindow(currentDisplay, window->windowHandle);
			XDestroyWindow(currentDisplay, window->windowHandle);
			window->windowHandle = 0;
			window->context		 = nullptr;
#endif
		}

		void ShareContexts(tWindow* sourceWindow, tWindow* newWindow)
		{
#if defined(TW_WINDOWS)
			Windows_ShareContexts(sourceWindow, newWindow);
#elif defined(TW_LINUX)
			// TODO: need to implement shared context functionality
			AddErrorLog(error_e::linuxFunctionNotImplemented);
#endif
		}

		void ResetMonitors()
		{
#if defined(TW_WINDOWS)
			Windows_ResetMonitors();
#elif defined(TW_LINUX)
			AddErrorLog(error_e::linuxFunctionNotImplemented);
#endif
		}

#pragma region Windows_Internal
#if defined(TW_WINDOWS)

		MSG winMessage;
		HWND dummyWindowHandle;
		HGLRC dummyGLContextHandle; /**< A handle to the dummy OpenGL rendering
												   context*/
		HDC dummyDeviceContextHandle;

		HINSTANCE dummyWindowInstance;
		// wgl extensions
		PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
		PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT;
		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
		PFNWGLCHOOSEPIXELFORMATEXTPROC wglChoosePixelFormatEXT;
		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
		PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
		PFNWGLGETPIXELFORMATATTRIBFVARBPROC wglGetPixelFormatAttribfvARB;
		PFNWGLGETPIXELFORMATATTRIBFVEXTPROC wglGetPixelFormatAttribfvEXT;
		PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB;
		PFNWGLGETPIXELFORMATATTRIBIVEXTPROC wglGetPixelFormatAttribivEXT;

		bool swapControlEXT;
		bool wglFramebufferSRGBCapableARB;

		formatSetting_t* bestPixelFormat;

		// the window procedure for all windows. This is used mainly to handle window events
		static LRESULT CALLBACK WindowProcedure(HWND windowHandle, UINT winMessage, WPARAM wordParam, LPARAM longParam)
		{
			windowManager* manager = (windowManager*)GetWindowLongPtr(windowHandle, GWLP_USERDATA);
			tWindow* window		   = nullptr;
			if (manager != nullptr)
			{
				window = manager->GetWindowByHandle(windowHandle);
			}

			uint16_t translatedKey = 0;
			static bool wasLowerCase   = false;

			switch (winMessage)
			{
				case WM_DESTROY:
					{
						if (manager != nullptr)
						{
							window->shouldClose = true;

							if (manager->destroyedEvent != nullptr)
							{
								manager->destroyedEvent(window);
							}

							// don't shutdown automatically, let people choose when to unload
							// manager->ShutdownWindow(window);
						}
						break;
					}

				case WM_MOVE:
					{
						window->position.x = LOWORD(longParam);
						window->position.y = HIWORD(longParam);
						manager->CheckWindowScreen(window);

						if (manager->movedEvent != nullptr)
						{
							manager->movedEvent(window, window->position);
						}

						break;
					}

				case WM_MOVING:
					{
						window->position.x = LOWORD(longParam);
						window->position.y = HIWORD(longParam);

						if (manager->movedEvent != nullptr)
						{
							manager->movedEvent(window, window->position);
						}
						break;
					}

				case WM_SIZE:
					{
						// high and low word are the client resolution. will need to change this
						window->settings.resolution.width  = (uint16_t)LOWORD(longParam);
						window->settings.resolution.height = (uint16_t)HIWORD(longParam);

						RECT tempRect;
						GetClientRect(window->windowHandle, &tempRect);
						window->clientArea.width  = tempRect.right;
						window->clientArea.height = tempRect.bottom;

						GetWindowRect(window->windowHandle, &tempRect);

						switch (wordParam)
						{
							case SIZE_MAXIMIZED:
								{
									if (manager->maximizedEvent != nullptr)
									{
										manager->maximizedEvent(window);
									}
									break;
								}

							case SIZE_MINIMIZED:
								{
									if (manager->minimizedEvent != nullptr)
									{
										manager->minimizedEvent(window);
									}
									break;
								}

							default:
								{
									if (manager->resizeEvent != nullptr)
									{
										manager->resizeEvent(window, window->settings.resolution);
									}
									break;
								}
						}
						break;
					}
				// only occurs when the window size is being dragged
				case WM_SIZING:
					{
						RECT tempRect;
						GetWindowRect(window->windowHandle, &tempRect);
						window->settings.resolution.width  = tempRect.right;
						window->settings.resolution.height = tempRect.bottom;

						GetClientRect(window->windowHandle, &tempRect);
						window->clientArea.width  = tempRect.right;
						window->clientArea.height = tempRect.bottom;

						if (manager->resizeEvent != nullptr)
						{
							manager->resizeEvent(window, window->settings.resolution);
						}

						UpdateWindow(window->windowHandle);// , NULL, true);
						break;
					}

				case WM_INPUT:
					{
						char buffer[sizeof(RAWINPUT)] = {};
						UINT size					  = sizeof(RAWINPUT);
						GetRawInputData(reinterpret_cast<HRAWINPUT>(longParam), RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER));

						RAWINPUT* rawInput = reinterpret_cast<RAWINPUT*>(buffer);
						switch (rawInput->header.dwType)
						{
							// grab raw keyboard info
							case RIM_TYPEKEYBOARD:
								{
									const RAWKEYBOARD& rawKB = rawInput->data.keyboard;
									uint16_t virtualKey	 = rawKB.VKey;
									uint16_t scanCode	 = rawKB.MakeCode;
									uint16_t flags		 = rawKB.Flags;
									bool isE0				 = false;
									bool isE1				 = false;

									if (virtualKey == 255)
										break;

									keyState_e keyState;
									if ((flags & RI_KEY_BREAK) != 0)
									{
										keyState = keyState_e::up;
									}

									else
									{
										keyState = keyState_e::down;
									}

									if (flags & RI_KEY_E0)
									{
										isE0 = true;
									}

									if (flags & RI_KEY_E1)
									{
										isE1 = true;
									}

									if (virtualKey == VK_SHIFT)
									{
										virtualKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);

										if (virtualKey == VK_LSHIFT)
										{
											window->keys[leftShift] = keyState;
										}

										else if (virtualKey == VK_RSHIFT)
										{
											window->keys[rightShift] = keyState;
										}
									}

									else if (virtualKey == VK_NUMLOCK)
									{
										// in raw input there is a big problem with PAUSE/break and numlock
										// the scancode needs to be remapped and have the extended bit set
										scanCode = (MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC) | 0x100);

										if (scanCode == VK_PAUSE) {}

										// std::bitset<64> bits(scanCode);
										// bits.set(24);
									}

									if (isE1)
									{
										if (virtualKey == VK_PAUSE)
										{
											scanCode = 0x45;// the E key???
										}

										else
										{
											scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
										}
									}

									translatedKey = 0;

									switch (virtualKey)
									{
										case VK_CONTROL:
											{
												translatedKey = (isE0) ? rightControl : leftControl;
												break;
											}
									}
								}

							// grab mouse info
							case RIM_TYPEMOUSE:
							// grab joystick info
							case RIM_TYPEHID:
								{
									break;
								}
						}
					}

				case WM_CHAR:
					{
						// WM_KEYUP/DOWN cannot tell between uppercase and lowercase since it
						// takes directly from the keyboard so WM_CHAR is needed to determine
						// casing. still a pain though to see whether the key was pressed or
						// released.
						wasLowerCase			= islower(static_cast<int16_t>(wordParam)) != 0;
						window->keys[wordParam] = keyState_e::down;
						if (manager->keyEvent != nullptr)
						{
							manager->keyEvent(window, static_cast<int16_t>(wordParam), keyState_e::down);
						}
						break;
					}

				case WM_KEYDOWN:
					{
						switch (DetermineLeftOrRight(wordParam, longParam))
						{
							case VK_LCONTROL:
								{
									window->keys[leftControl] = keyState_e::down;
									translatedKey			  = leftControl;
									break;
								}

							case VK_RCONTROL:
								{
									window->keys[rightControl] = keyState_e::down;
									translatedKey			   = rightControl;
									break;
								}

							case VK_LSHIFT:
								{
									window->keys[leftShift] = keyState_e::down;
									translatedKey			= leftShift;
									break;
								}

							case VK_RSHIFT:
								{
									window->keys[rightShift] = keyState_e::down;
									translatedKey			 = rightShift;
									break;
								}

							default:
								{
									translatedKey = Windows_TranslateKey(wordParam);
									if (translatedKey != 0)
									{
										window->keys[translatedKey] = keyState_e::down;
									}
									break;
								}
						}

						if (manager->keyEvent != nullptr && translatedKey != 0)
						{
							manager->keyEvent(window, translatedKey, keyState_e::down);
						}
						break;
					}

				case WM_KEYUP:
					{
						switch (DetermineLeftOrRight(wordParam, longParam))
						{
							case VK_LCONTROL:
								{
									window->keys[leftControl] = keyState_e::up;
									translatedKey			  = leftControl;
									break;
								}

							case VK_RCONTROL:
								{
									window->keys[rightControl] = keyState_e::up;
									translatedKey			   = rightControl;
									break;
								}

							case VK_LSHIFT:
								{
									window->keys[leftShift] = keyState_e::up;
									translatedKey			= leftShift;
									break;
								}

							case VK_RSHIFT:
								{
									window->keys[rightShift] = keyState_e::up;
									translatedKey			 = rightShift;
									break;
								}

							default:
								{
									translatedKey = Windows_TranslateKey(wordParam);
									if (translatedKey != 0)
									{
										window->keys[translatedKey] = keyState_e::up;
									}

									else
									{
										// if it was lowercase
										if (wasLowerCase)
										{
											// change the wordParam to lowercase
											translatedKey = tolower(static_cast<uint16_t>(wordParam));
										}
										else
										{
											// keep it as is if it isn't
											translatedKey = static_cast<uint16_t>(wordParam);
										}

										window->keys[translatedKey] = keyState_e::up;
									}
									break;
								}
						}

						if (manager->keyEvent != nullptr)
						{
							manager->keyEvent(window, translatedKey, keyState_e::up);
						}
						break;
					}

				case WM_SYSKEYDOWN:
					{
						translatedKey = 0;

						switch (DetermineLeftOrRight(wordParam, longParam))
						{
							case VK_LMENU:
								{
									window->keys[leftAlt] = keyState_e::down;
									translatedKey		  = leftAlt;
									break;
								}

							case VK_RMENU:
								{
									window->keys[rightAlt] = keyState_e::down;
									translatedKey		   = rightAlt;
									break;
								}
						}

						if (manager->keyEvent != nullptr)
						{
							manager->keyEvent(window, translatedKey, keyState_e::down);
						}
						break;
					}

				case WM_SYSKEYUP:
					{
						translatedKey = 0;
						switch (DetermineLeftOrRight(wordParam, longParam))
						{
							case VK_LMENU:
								{
									window->keys[leftAlt] = keyState_e::up;
									translatedKey		  = leftAlt;
									break;
								}

							case VK_RMENU:
								{
									window->keys[rightAlt] = keyState_e::up;
									translatedKey		   = rightAlt;
									break;
								}

							default:
								{
									break;
								}
						}

						if (manager->keyEvent != nullptr)
						{
							manager->keyEvent(window, translatedKey, keyState_e::up);
						}
						break;
					}

				case WM_MOUSEMOVE:
					{
						window->previousMousePosition = window->mousePosition;
						window->mousePosition.x		  = (int)LOWORD(longParam);
						window->mousePosition.y		  = (int)HIWORD(longParam);

						POINT point;
						point.x = (LONG)window->mousePosition.x;
						point.y = (LONG)window->mousePosition.y;

						ClientToScreen(windowHandle, &point);

						if (manager->mouseMoveEvent != nullptr)
						{
							manager->mouseMoveEvent(window, window->mousePosition, vec2_t<int16_t>(point.x, point.y));
						}
						break;
					}

				case WM_XBUTTONDOWN:
					{
						int XButton = (int)HIWORD(wordParam);

						if (XButton == XBUTTON1)
						{
							window->mouseButton[(uint16_t)mouseButton_e::XFirst] = buttonState_e::down;

							if (manager->mouseButtonEvent != nullptr)
							{
								manager->mouseButtonEvent(window, mouseButton_e::XFirst, buttonState_e::down);
							}
						}

						if (XButton == XBUTTON2)
						{
							window->mouseButton[(uint16_t)mouseButton_e::XSecond] = buttonState_e::down;

							if (manager->mouseButtonEvent != nullptr)
							{
								manager->mouseButtonEvent(window, mouseButton_e::XSecond, buttonState_e::down);
							}
						}
						break;
					}

				case WM_XBUTTONUP:
					{
						int XButton = (int)HIWORD(wordParam);

						if (XButton == XBUTTON1)
						{
							window->mouseButton[(short)mouseButton_e::XFirst] = buttonState_e::up;

							if (manager->mouseButtonEvent != nullptr)
							{
								manager->mouseButtonEvent(window, mouseButton_e::XFirst, buttonState_e::up);
							}
						}

						if (XButton == XBUTTON2)
						{
							window->mouseButton[(short)mouseButton_e::XSecond] = buttonState_e::up;

							if (manager->mouseButtonEvent != nullptr)
							{
								manager->mouseButtonEvent(window, mouseButton_e::XSecond, buttonState_e::up);
							}
						}
						break;
					}

				case WM_LBUTTONDOWN:
					{
						window->mouseButton[(short)mouseButton_e::left] = buttonState_e::down;

						if (manager->mouseButtonEvent != nullptr)
						{
							manager->mouseButtonEvent(window, mouseButton_e::left, buttonState_e::down);
						}

						break;
					}

				case WM_LBUTTONUP:
					{
						window->mouseButton[(short)mouseButton_e::left] = buttonState_e::up;

						if (manager->mouseButtonEvent != nullptr)
						{
							manager->mouseButtonEvent(window, mouseButton_e::left, buttonState_e::up);
						}
						break;
					}

				case WM_RBUTTONDOWN:
					{
						window->mouseButton[(short)mouseButton_e::right] = buttonState_e::down;

						if (manager->mouseButtonEvent != nullptr)
						{
							manager->mouseButtonEvent(window, mouseButton_e::right, buttonState_e::down);
						}
						break;
					}

				case WM_RBUTTONUP:
					{
						window->mouseButton[(short)mouseButton_e::right] = buttonState_e::up;

						if (manager->mouseButtonEvent != nullptr)
						{
							manager->mouseButtonEvent(window, mouseButton_e::right, buttonState_e::up);
						}
						break;
					}

				case WM_MBUTTONDOWN:
					{
						window->mouseButton[(uint16_t)mouseButton_e::middle] = buttonState_e::down;

						if (manager->mouseButtonEvent != nullptr)
						{
							manager->mouseButtonEvent(window, mouseButton_e::middle, buttonState_e::down);
						}
						break;
					}

				case WM_MBUTTONUP:
					{
						window->mouseButton[(uint16_t)mouseButton_e::middle] = buttonState_e::up;

						if (manager->mouseButtonEvent != nullptr)
						{
							manager->mouseButtonEvent(window, mouseButton_e::middle, buttonState_e::up);
						}
						break;
					}

				case WM_MOUSEWHEEL:
					{
						int delta = GET_WHEEL_DELTA_WPARAM(wordParam);
						if (delta > 0)
						{
							// if was previously negative, revert to zero
							if (window->accumWheelDelta < 0)
								window->accumWheelDelta = 0;

							else
								window->accumWheelDelta += delta;

							if (window->accumWheelDelta >= WHEEL_DELTA)
							{
								if (manager->mouseWheelEvent != nullptr)
								{
									manager->mouseWheelEvent(window, mouseScroll_e::up);
								}

								// reset accum
								window->accumWheelDelta = 0;
							}
						}

						else
						{
							// if was previously positive, revert to zero
							if (window->accumWheelDelta > 0)
								window->accumWheelDelta = 0;

							else
								window->accumWheelDelta += delta;

							// if the delta is equal to or greater than delta
							if (window->accumWheelDelta <= -WHEEL_DELTA)
							{
								if (manager->mouseWheelEvent != nullptr)
								{
									manager->mouseWheelEvent(window, mouseScroll_e::down);
								}

								// reset accum
								window->accumWheelDelta = 0;
							}
						}
						break;
					}

				case WM_SETFOCUS:
					{
						window->inFocus = true;
						if (manager->focusEvent != nullptr)
						{
							manager->focusEvent(window, true);
						}

						break;
					}

				case WM_KILLFOCUS:
					{
						window->inFocus = false;
						if (manager->focusEvent != nullptr)
						{
							manager->focusEvent(window, false);
						}

						break;
					}

				case WM_DROPFILES:
					{
						// get the number of files that were dropped
						uint16_t numfilesDropped = DragQueryFile((HDROP)wordParam, 0xFFFFFFFF, nullptr, 0);
						std::vector<std::string> files;

						// for each file dropped store the path
						for (size_t fileIter = 0; fileIter < numfilesDropped; fileIter++)
						{
							char file[255]			= {0};
							uint16_t stringSize = DragQueryFile((HDROP)wordParam, (UINT)fileIter, nullptr, 0);// get the size of the string
							DragQueryFile((HDROP)wordParam, (UINT)fileIter, (wchar_t*)file, stringSize + 1);	  // get the string itself
							files.emplace_back(file);
						}
						POINT mousePoint;
						vec2_t<int16_t> mousePosition;
						if (DragQueryPoint((HDROP)wordParam, &mousePoint))// get the mouse position where the file was dropped
						{
							mousePosition = vec2_t<int16_t>(mousePoint.x, mousePoint.y);
						}

						// release the memory
						DragFinish((HDROP)wordParam);

						if (manager->fileDropEvent != nullptr)
						{
							manager->fileDropEvent(window, std::move(files), mousePosition);
						}
						break;
					}

				default:
					{
						return DefWindowProc(windowHandle, winMessage, wordParam, longParam);
					}
			}

			return 0;
		}

		// user data should be a pointer to a window manager
		static BOOL CALLBACK MonitorEnumProcedure(HMONITOR monitorHandle, HDC monitorDeviceContextHandle, LPRECT monitorSize, LPARAM userData)
		{
			windowManager* manager = (windowManager*)userData;
			MONITORINFOEX info	   = {};
			info.cbSize			   = sizeof(info);
			GetMonitorInfo(monitorHandle, &info);

			std::wstring wstr(info.szDevice);

			monitor_t* monitor		   = manager->GetMonitorByHandle(std::string(wstr.begin(), wstr.end()));
			monitor->monitorHandle	   = monitorHandle;
			monitor->extents		   = vec4_t<uint16_t>(monitorSize->left, monitorSize->top, monitorSize->right, monitorSize->bottom);
			monitor->resolution.width  = monitor->extents.right - monitor->extents.left;
			monitor->resolution.height = monitor->extents.bottom - monitor->extents.top;
			return true;
		}

		// get the window that is associated with this Win32 window handle
		tWindow* GetWindowByHandle(HWND windowHandle)
		{
			for (auto& windowIndex: windowList)
			{
				if (windowIndex->windowHandle == windowHandle)
				{
					return windowIndex.get();
				}
				return nullptr;
			}
		}

		monitor_t* GetMonitorByHandle(std::string const& displayName)
		{
			for (auto& iter: monitorList)
			{
				if (displayName.compare(iter.displayName) == 0)
				{
					return &iter;
				}
			}
			return nullptr;
		}

		// initialize the given window using Win32
		void Windows_InitializeWindow(tWindow* window, UINT style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW, int clearScreenExtra = 0, int windowExtra = 0, HINSTANCE winInstance = GetModuleHandle(nullptr), HICON icon = LoadIcon(nullptr, IDI_APPLICATION), HCURSOR cursor = LoadCursor(nullptr, IDC_ARROW), HBRUSH brush = (HBRUSH)GetStockObject(WHITE_BRUSH))
		{
			std::string menuName   = window->settings.name;
			std::wstring wMenuName = std::wstring(menuName.begin(), menuName.end());;

			std::string className	= window->settings.name;
			std::wstring wClassName = std::wstring(className.begin(), className.end());;

			window->instanceHandle			  = winInstance;
			window->windowClass.style		  = style;
			window->windowClass.lpfnWndProc	  = windowManager::WindowProcedure;
			window->windowClass.cbClsExtra	  = clearScreenExtra;
			window->windowClass.cbWndExtra	  = windowExtra;
			window->windowClass.hInstance	  = window->instanceHandle;
			window->windowClass.hIcon		  = icon;
			window->windowClass.hCursor		  = cursor;
			window->windowClass.hbrBackground = brush;
			window->windowClass.lpszMenuName  = wMenuName.c_str();
			window->windowClass.lpszClassName = wClassName.c_str();
			RegisterClass(&window->windowClass);

			window->windowHandle = CreateWindow(window->windowClass.lpszClassName, window->windowClass.lpszMenuName, WS_OVERLAPPEDWINDOW, 0, 0, window->settings.resolution.width, window->settings.resolution.height, nullptr, nullptr, nullptr, nullptr);

			SetWindowLongPtr(window->windowHandle, GWLP_USERDATA, (LONG_PTR)this);

			// if TW_USE_VULKAN is defined then stop TinyWindow from creating an OpenGL
			// context since it will conflict with a vulkan context
#if !defined(TW_USE_VULKAN)
			InitializeGL(window);
#endif
			ShowWindow(window->windowHandle, 1);
			UpdateWindow(window->windowHandle);

			CheckWindowScreen(window);

			// get screen by window Handle

			//SetStyle(window, style_e::normal);

			DragAcceptFiles(window->windowHandle, true);
		}

		void Windows_CreateDummyWindow()
		{
			dummyWindowInstance = GetModuleHandle(nullptr);
			WNDCLASS dummyClass;
			dummyClass.style		 = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
			dummyClass.lpfnWndProc	 = windowManager::WindowProcedure;
			dummyClass.cbClsExtra	 = 0;
			dummyClass.cbWndExtra	 = 0;
			dummyClass.hInstance	 = dummyWindowInstance;
			dummyClass.hIcon		 = LoadIcon(nullptr, IDI_APPLICATION);
			dummyClass.hCursor		 = LoadCursor(nullptr, IDC_ARROW);
			dummyClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
			dummyClass.lpszMenuName	 = L"dummy";
			dummyClass.lpszClassName = L"dummy";
			RegisterClass(&dummyClass);

			dummyWindowHandle = CreateWindow(dummyClass.lpszMenuName, dummyClass.lpszClassName, WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, nullptr, nullptr, nullptr, nullptr);
			if (dummyWindowHandle == nullptr)
			{
				AddErrorLog(error_e::invalidDummyWindow);
			}

			ShowWindow(dummyWindowHandle, SW_HIDE);
		}

		// initialize the pixel format for the selected window
		void InitializePixelFormat(tWindow* window)
		{
			UINT count = WGL_NUMBER_PIXEL_FORMATS_ARB;
			int format		   = 0;
			int attribs[]	   = {WGL_SUPPORT_OPENGL_ARB, 1, WGL_DRAW_TO_WINDOW_ARB, 1, WGL_DOUBLE_BUFFER_ARB, 1, WGL_RED_BITS_ARB, window->settings.colorBits, WGL_GREEN_BITS_ARB, window->settings.colorBits, WGL_BLUE_BITS_ARB, window->settings.colorBits, WGL_ALPHA_BITS_ARB, window->settings.colorBits, WGL_DEPTH_BITS_ARB, window->settings.depthBits, WGL_STENCIL_BITS_ARB, window->settings.stencilBits, WGL_ACCUM_RED_BITS_ARB, window->settings.accumBits, WGL_ACCUM_GREEN_BITS_ARB, window->settings.accumBits, WGL_ACCUM_BLUE_BITS_ARB, window->settings.accumBits, WGL_ACCUM_ALPHA_BITS_ARB, window->settings.accumBits, WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB, WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB};

			std::vector<int> attribList;
			attribList.assign(attribs, attribs + std::size(attribs));

			if (wglChoosePixelFormatARB != nullptr)
			{
				if (window->settings.enableSRGB)
				{
					attribList.push_back(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB);
				}

				attribList.push_back(0);// needs a 0 to notify as the end of the list.
				wglChoosePixelFormatARB(window->deviceContextHandle, &attribList[0], nullptr, 1, &format, &count);
				SetPixelFormat(window->deviceContextHandle, format, &window->pixelFormatDescriptor);
			}

			else if (wglChoosePixelFormatEXT != nullptr)
			{
				if (window->settings.enableSRGB)
				{
					attribList.push_back(WGL_FRAMEBUFFER_SRGB_CAPABLE_EXT);
				}

				attribList.push_back(0);
				wglChoosePixelFormatEXT(window->deviceContextHandle, &attribList[0], nullptr, 1, &format, &count);
				SetPixelFormat(window->deviceContextHandle, format, &window->pixelFormatDescriptor);
			}

			else
			{
				PIXELFORMATDESCRIPTOR pfd		= {};
				formatSetting_t* desiredSetting = new formatSetting_t(window->settings.colorBits, window->settings.colorBits, window->settings.colorBits, window->settings.colorBits, window->settings.depthBits, window->settings.stencilBits, window->settings.accumBits, window->settings.accumBits, window->settings.accumBits, window->settings.accumBits);
				int bestPFDHandle				= GetLegacyPFD(desiredSetting, window->deviceContextHandle)->handle;
				if (!DescribePixelFormat(window->deviceContextHandle, bestPFDHandle, sizeof(pfd), &pfd))
					return;
				SetPixelFormat(window->deviceContextHandle, bestPFDHandle, &pfd);
			}
		}

		formatSetting_t* GetLegacyPFD(formatSetting_t* desiredSetting, HDC deviceContextHandle)
		{
			// use the old PFD system on the window if none of the extensions will load
			int nativeCount	  = 0;
			int numCompatible = 0;
			// pass nullptr to get the total number of PFDs that are available
			nativeCount = DescribePixelFormat(deviceContextHandle, 1, sizeof(PIXELFORMATDESCRIPTOR), nullptr);

			for (int nativeIter = 0; nativeIter < nativeCount; nativeIter++)
			{
				const int num = nativeIter + 1;
				PIXELFORMATDESCRIPTOR pfd;
				if (!DescribePixelFormat(deviceContextHandle, num, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
					continue;

				// skip if the PFD does not have PFD_DRAW_TO_WINDOW and PFD_SUPPORT_OPENGL
				if (!(pfd.dwFlags & PFD_DRAW_TO_WINDOW) || !(pfd.dwFlags & PFD_SUPPORT_OPENGL))
					continue;

				// skip if the PFD does not have PFD_GENERIC_ACCELERATION and PFD_GENERIC
				// FORMAT
				if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) && (pfd.dwFlags & PFD_GENERIC_FORMAT))
					continue;

				// if the pixel type is not RGBA
				if (pfd.iPixelType != PFD_TYPE_RGBA)
					continue;

				formatSetting_t* setting = new formatSetting_t(pfd.cRedBits, pfd.cGreenBits, pfd.cBlueBits, pfd.cAlphaBits, pfd.cDepthBits, pfd.cStencilBits, pfd.cAccumRedBits, pfd.cAccumGreenBits, pfd.cAccumBlueBits, pfd.cAccumAlphaBits, pfd.cAuxBuffers, (pfd.dwFlags & PFD_STEREO) ? true : false, (pfd.dwFlags & PFD_DOUBLEBUFFER) ? true : false);
				setting->handle			 = num;

				formatList.push_back(std::move(setting));
				numCompatible++;
			}

			if (numCompatible == 0)
			{
				// need to add an error message pipeline to this.
				// or a list of messages with a function to get last error
				// your driver has no compatible PFDs for OpenGL. at all. the fuck?
				return nullptr;
			}

			// the best PFD would probably be the most basic by far
			formatSetting_t defaultSetting = formatSetting_t();
			defaultSetting.redBits		   = 8;
			defaultSetting.greenBits	   = 8;
			defaultSetting.blueBits		   = 8;
			defaultSetting.alphaBits	   = 8;
			defaultSetting.depthBits	   = 24;
			defaultSetting.stencilBits	   = 8;
			defaultSetting.doubleBuffer	   = true;

			// if the best format hasn't already been found then find them manually
			formatSetting_t* bestFormat = GetClosestFormat(desiredSetting);
			if (!bestFormat)
				return nullptr;
			return bestFormat;
		}

		formatSetting_t* GetClosestFormat(const formatSetting_t* desiredFormat)
		{
			// go through all the compatible format settings
			uint16_t absent, lowestAbsent		= UINT_MAX;
			uint16_t colorDiff, lowestColorDiff = UINT_MAX;
			uint16_t extraDiff, lowestExtraDiff = UINT_MAX;
			formatSetting_t* currentFormat;
			formatSetting_t* closestFormat = nullptr;

			for (auto formatIter: formatList)
			{
				currentFormat = formatIter;

				// must have the same stereoscopic setting
				if (desiredFormat->stereo && !currentFormat->stereo)
					continue;

				// must have the same double buffer setting
				if (desiredFormat->doubleBuffer != currentFormat->doubleBuffer)
					continue;

				// get the missing buffers
				{
					absent = 0;

					// if the current format doesn't have any alpha bits	and the desired
					// has over 0
					if (desiredFormat->alphaBits && !currentFormat->alphaBits)
					{
						absent++;
					}

					// if the current format doesn't have any depth bits	and the desired
					// has over 0
					if (desiredFormat->depthBits && !currentFormat->depthBits)
					{
						absent++;
					}

					// if the current format doesn't have any stencil bits and the desired
					// has over 0
					if (desiredFormat->stencilBits && !currentFormat->stencilBits)
					{
						absent++;
					}

					// if the desired has aux buffers and the desired has more aux buffers
					// than the current
					if (desiredFormat->auxBuffers && currentFormat->auxBuffers < desiredFormat->auxBuffers)
					{
						// add up the missing buffers as the difference in buffers between
						// desired and current in aux buffer count
						absent += desiredFormat->auxBuffers - currentFormat->auxBuffers;
					}

					// for modern framebuffers.if the desired needs samples and the current
					// has not samples
					if (desiredFormat->numSamples > 0 && !currentFormat->numSamples)
					{
						absent++;
					}
				}

				// gather the value differences in color channels
				{
					colorDiff = 0;

					if (desiredFormat->redBits != -1)
					{
						colorDiff += (uint16_t)pow((desiredFormat->redBits - currentFormat->redBits), 2);
					}

					if (desiredFormat->greenBits != -1)
					{
						colorDiff += (uint16_t)pow((desiredFormat->greenBits - currentFormat->greenBits), 2);
					}

					if (desiredFormat->blueBits != -1)
					{
						colorDiff += (uint16_t)pow((desiredFormat->blueBits - currentFormat->blueBits), 2);
					}
				}

				// calculates the difference in values for extras
				{
					extraDiff = 0;

					if (desiredFormat->alphaBits != -1)
					{
						extraDiff += (uint16_t)pow((desiredFormat->alphaBits - currentFormat->alphaBits), 2);
					}

					if (desiredFormat->depthBits != -1)
					{
						extraDiff += (uint16_t)pow((desiredFormat->depthBits - currentFormat->depthBits), 2);
					}

					if (desiredFormat->stencilBits != -1)
					{
						extraDiff += (uint16_t)pow((desiredFormat->stencilBits - currentFormat->stencilBits), 2);
					}

					if (desiredFormat->accumRedBits != -1)
					{
						extraDiff += (uint16_t)pow((desiredFormat->accumRedBits - currentFormat->accumRedBits), 2);
					}

					if (desiredFormat->accumGreenBits != -1)
					{
						extraDiff += (uint16_t)pow((desiredFormat->accumGreenBits - currentFormat->accumGreenBits), 2);
					}

					if (desiredFormat->accumBlueBits != -1)
					{
						extraDiff += (uint16_t)pow((desiredFormat->accumBlueBits - currentFormat->accumBlueBits), 2);
					}

					if (desiredFormat->numSamples != -1)
					{
						extraDiff += (uint16_t)pow((desiredFormat->numSamples - currentFormat->numSamples), 2);
					}

					if (desiredFormat->alphaBits != -1)
					{
						extraDiff += (uint16_t)pow((desiredFormat->alphaBits - currentFormat->alphaBits), 2);
					}

					if (desiredFormat->pixelRGB && !currentFormat->pixelRGB)
					{
						extraDiff++;
					}
				}

				// determine if the current one is better than the best one so far
				if (absent < lowestAbsent)
				{
					closestFormat = currentFormat;
				}

				else if (absent == lowestAbsent)
				{
					if ((colorDiff < lowestColorDiff) || (colorDiff == lowestColorDiff && extraDiff < lowestExtraDiff))
					{
						closestFormat = currentFormat;
					}
				}

				if (currentFormat == closestFormat)
				{
					lowestAbsent	= absent;
					lowestColorDiff = colorDiff;
					lowestExtraDiff = extraDiff;
				}
			}
			return closestFormat;
		}

		void Windows_Shutown() {}

		void Windows_CreateDummyContext()
		{
			Windows_CreateDummyWindow();
			dummyDeviceContextHandle		= GetDC(dummyWindowHandle);
			PIXELFORMATDESCRIPTOR pfd		= {};
			formatSetting_t* desiredSetting = new formatSetting_t();
			int bestPFDHandle				= GetLegacyPFD(desiredSetting, dummyDeviceContextHandle)->handle;

			if (!DescribePixelFormat(dummyDeviceContextHandle, bestPFDHandle, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
			{
				AddErrorLog(error_e::invalidDummyPixelFormat);
			}

			if (!SetPixelFormat(dummyDeviceContextHandle, bestPFDHandle, &pfd))
			{
				AddErrorLog(error_e::invalidDummyPixelFormat);
			}

			dummyGLContextHandle = wglCreateContext(dummyDeviceContextHandle);
			if (!dummyGLContextHandle)
			{
				AddErrorLog(error_e::dummyCreationFailed);
			}

			if (!wglMakeCurrent(dummyDeviceContextHandle, dummyGLContextHandle))
			{
				AddErrorLog(error_e::dummyCannotMakeCurrent);
			}
		}

		void Windows_ToggleFullscreen(tWindow* window, monitor_t* monitor, uint16_t monitorSettingIndex)
		{
			window->currentMonitor = monitor;

			DEVMODE devMode;
			ZeroMemory(&devMode, sizeof(DEVMODE));
			devMode.dmSize = sizeof(DEVMODE);
			int err		   = 0;
			if (window->isFullscreen)
			{
				err = ChangeDisplaySettingsEx((wchar_t*)window->currentMonitor->displayName.c_str(), nullptr, nullptr, CDS_FULLSCREEN, nullptr);
			}

			else if (monitorSettingIndex < (monitor->settings.size() - 1))
			{
				monitorSetting_t selectedSetting = monitor->settings[monitorSettingIndex];
				devMode.dmPelsWidth				 = selectedSetting.resolution.width;
				devMode.dmPelsHeight			 = selectedSetting.resolution.height;
				devMode.dmBitsPerPel			 = window->settings.colorBits * 4;
				devMode.dmDisplayFrequency		 = selectedSetting.displayFrequency;
				devMode.dmFields				 = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
				err								 = ChangeDisplaySettingsEx((wchar_t*)window->currentMonitor->displayName.c_str(), &devMode, nullptr, CDS_FULLSCREEN, nullptr);
			}
			else
			{
				AddErrorLog(window, error_e::invalidMonitorSettingIndex, __LINE__, __FUNCTION__);
			}

			switch (err)
			{
				case DISP_CHANGE_SUCCESSFUL:
					{
						window->isFullscreen = !window->isFullscreen;
						if (window->isFullscreen)
						{
							SetStyle(window, style_e::popup);
						}
						else
						{
							SetStyle(window, style_e::normal);
						}
						break;
					}

				case DISP_CHANGE_BADDUALVIEW:
					{
						AddErrorLog(window, error_e::windowsFullscreenBadDualView, __LINE__, __FUNCTION__);
						break;
					}
				case DISP_CHANGE_BADFLAGS:
					{
						AddErrorLog(window, error_e::windowsFullscreenBadFlags, __LINE__, __FUNCTION__);
						break;
					}
				case DISP_CHANGE_BADMODE:
					{
						AddErrorLog(window, error_e::windowsFullscreenBadMode, __LINE__, __FUNCTION__);
						break;
					}
				case DISP_CHANGE_BADPARAM:
					{
						AddErrorLog(window, error_e::WindowsFullscreenBadParam, __LINE__, __FUNCTION__);
						break;
					}
				case DISP_CHANGE_FAILED:
					{
						AddErrorLog(window, error_e::WindowsFullscreenChangeFailed, __LINE__, __FUNCTION__);
						break;
					}
				case DISP_CHANGE_NOTUPDATED:
					{
						AddErrorLog(window, error_e::WindowsFullscreenNotUpdated, __LINE__, __FUNCTION__);
						break;
					}

				default:
					{
						break;
					}
			}

			SetPosition(window, vec2_t<int16_t>((int)monitor->extents.left, (int)monitor->extents.top));
		}

		static int RetrieveDataFromWin32Pointer(LPARAM longParam, uint16_t depth) { return (longParam >> depth) & ((1L << sizeof(longParam)) - 1); }

		static WPARAM DetermineLeftOrRight(WPARAM key, LPARAM longParam)
		{
			std::bitset<32> bits(longParam);
			WPARAM newKey = key;
			// extract data at the 16th bit point to retrieve the scancode
			UINT scancode = RetrieveDataFromWin32Pointer(longParam, 16);
			// extract at the 24th bit point to determine if it is an extended key
			int extended = bits.test(24) != 0;

			switch (key)
			{
				case VK_SHIFT:
					{
						newKey = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
						break;
					}

				case VK_CONTROL:
					{
						newKey = extended ? VK_RCONTROL : VK_LCONTROL;
						break;
					}

				case VK_MENU:
					{
						newKey = extended ? VK_RMENU : VK_LMENU;
						break;
					}

				default:
					{
						// if it cannot determine left from right then just return the original key
						newKey = key;
						break;
					}
			}

			return newKey;
		}

		static uint16_t Windows_TranslateKey(WPARAM wordParam)
		{
			switch (wordParam)
			{
				case VK_ESCAPE:
					{
						return escape;
					}
				case VK_SPACE:
					{
						return spacebar;
					}
				case VK_F1:
					{
						return F1;
					}
				case VK_F2:
					{
						return F2;
					}
				case VK_F3:
					{
						return F3;
					}
				case VK_F4:
					{
						return F4;
					}
				case VK_F5:
					{
						return F5;
					}
				case VK_F6:
					{
						return F6;
					}
				case VK_F7:
					{
						return F7;
					}
				case VK_F8:
					{
						return F8;
					}
				case VK_F9:
					{
						return F9;
					}
				case VK_F10:
					{
						return F10;
					}
				case VK_F11:
					{
						return F11;
					}
				case VK_F12:
					{
						return F12;
					}
				case VK_BACK:
					{
						return backspace;
					}
				case VK_TAB:
					{
						return tab;
					}
				case VK_CAPITAL:
					{
						return capsLock;
					}
				case VK_RETURN:
					{
						return enter;
					}
				case VK_PRINT:
					{
						return printScreen;
					}
				case VK_SCROLL:
					{
						return scrollLock;
					}
				case VK_PAUSE:
					{
						return pause;
					}
				case VK_INSERT:
					{
						return insert;
					}
				case VK_HOME:
					{
						return home;
					}
				case VK_DELETE:
					{
						return del;
					}
				case VK_END:
					{
						return end;
					}
				case VK_PRIOR:
					{
						return pageUp;
					}
				case VK_NEXT:
					{
						return pageDown;
					}
				case VK_DOWN:
					{
						return arrowDown;
					}
				case VK_UP:
					{
						return arrowUp;
					}
				case VK_LEFT:
					{
						return arrowLeft;
					}
				case VK_RIGHT:
					{
						return arrowRight;
					}
				case VK_DIVIDE:
					{
						return keypadDivide;
					}
				case VK_MULTIPLY:
					{
						return keypadMultiply;
					}
				case VK_SUBTRACT:
					{
						return keypadDivide;
					}
				case VK_ADD:
					{
						return keypadAdd;
					}
				case VK_DECIMAL:
					{
						return keypadPeriod;
					}
				case VK_NUMPAD0:
					{
						return keypad0;
					}
				case VK_NUMPAD1:
					{
						return keypad1;
					}
				case VK_NUMPAD2:
					{
						return keypad2;
					}
				case VK_NUMPAD3:
					{
						return keypad3;
					}
				case VK_NUMPAD4:
					{
						return keypad4;
					}
				case VK_NUMPAD5:
					{
						return keypad5;
					}
				case VK_NUMPAD6:
					{
						return keypad6;
					}
				case VK_NUMPAD7:
					{
						return keypad7;
					}
				case VK_NUMPAD8:
					{
						return keypad8;
					}
				case VK_NUMPAD9:
					{
						return keypad9;
					}
				case VK_LWIN:
					{
						return leftWindow;
					}
				case VK_RWIN:
					{
						return rightWindow;
					}

				default:
					{
						return 0;
					}
			}
		}

		static void Windows_SetWindowIcon(tWindow* window, const char* icon, uint16_t width, uint16_t height) { SendMessage(window->windowHandle, (UINT)WM_SETICON, ICON_BIG, (LPARAM)LoadImage(window->instanceHandle, (wchar_t*)icon, IMAGE_ICON, (int)width, (int)height, LR_LOADFROMFILE)); }

		void Windows_GetScreenInfo()
		{
			// get display device info
			DISPLAY_DEVICE graphicsDevice;
			graphicsDevice.cb		  = sizeof(DISPLAY_DEVICE);
			graphicsDevice.StateFlags = DISPLAY_DEVICE_ATTACHED_TO_DESKTOP;
			DWORD deviceNum			  = 0;
			DWORD monitorNum		  = 0;
			while (EnumDisplayDevices(nullptr, deviceNum, &graphicsDevice, EDD_GET_DEVICE_INTERFACE_NAME))
			{
				// get monitor info for the current display device
				DISPLAY_DEVICE monitorDevice = {0};
				monitorDevice.cb			 = sizeof(DISPLAY_DEVICE);
				monitorDevice.StateFlags	 = DISPLAY_DEVICE_ATTACHED_TO_DESKTOP;
				monitor_t* monitor			 = nullptr;

				// if it has children add them to the list, else, ignore them since those
				// are only POTENTIAL monitors/devices
				while (EnumDisplayDevices(graphicsDevice.DeviceName, monitorNum, &monitorDevice, EDD_GET_DEVICE_INTERFACE_NAME))
				{
					std::wstring wDeviceName(graphicsDevice.DeviceName);
					std::string deviceName(wDeviceName.begin(), wDeviceName.end());

					std::wstring wDeviceString(graphicsDevice.DeviceString);
					std::string deviceString(wDeviceString.begin(), wDeviceString.end());

					std::wstring wMonitorString(monitorDevice.DeviceString);
					std::string monitorString(wMonitorString.begin(), wMonitorString.end());


					monitor = new monitor_t(deviceName, deviceString, monitorString, (graphicsDevice.StateFlags | DISPLAY_DEVICE_PRIMARY_DEVICE) ? true : false);
					// get current display mode
					DEVMODE devmode;
					// get all display modes
					uint16_t modeIndex = UINT_MAX;
					while (EnumDisplaySettings(graphicsDevice.DeviceName, modeIndex, &devmode))
					{
						// get the current settings of the display
						if (modeIndex == ENUM_CURRENT_SETTINGS)
						{
							monitor->currentSetting				 = monitorSetting_t(vec2_t<uint16_t>(devmode.dmPelsWidth, devmode.dmPelsHeight), devmode.dmDisplayFrequency);
							monitor->currentSetting.displayFlags = devmode.dmDisplayFlags;
							monitor->currentSetting.fixedOutput	 = devmode.dmDisplayFixedOutput;
						}
						// get the settings that are stored in the registry
						else
						{
							monitorSetting_t* newSetting = new monitorSetting_t(vec2_t<uint16_t>(devmode.dmPelsWidth, devmode.dmPelsHeight), devmode.dmDisplayFrequency);
							newSetting->displayFlags	 = devmode.dmDisplayFlags;
							newSetting->fixedOutput		 = devmode.dmDisplayFixedOutput;
							monitor->settings.insert(monitor->settings.begin(), *std::move(newSetting));
						}
						modeIndex++;
					}
					monitorList.push_back(*std::move(monitor));
					monitorNum++;
					monitorDevice.StateFlags = DISPLAY_DEVICE_ATTACHED_TO_DESKTOP;
				}
				deviceNum++;
				monitorNum = 0;
			}
			// this is just to grab the monitor extents
			EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProcedure, (LPARAM)this);
		}

		bool Windows_ExtensionSupported(const char* extensionName)
		{
			const char* wglExtensions;

			if (wglGetExtensionsStringARB != nullptr)
			{
				wglExtensions = wglGetExtensionsStringARB(dummyDeviceContextHandle);
				if (wglExtensions != nullptr)
				{
					if (std::strstr(wglExtensions, extensionName) != nullptr)
					{
						return true;
					}
				}
			}

			if (wglGetExtensionsStringEXT != nullptr)
			{
				wglExtensions = wglGetExtensionsStringEXT();
				if (wglExtensions != nullptr)
				{
					if (std::strstr(wglExtensions, extensionName) != nullptr)
					{
						return true;
					}
				}
			}
			return false;
		}

		void Windows_ResetMonitors()
		{
			for (auto iter: monitorList)
			{
				ChangeDisplaySettingsEx((wchar_t*)iter.displayName.c_str(), nullptr, nullptr, CDS_FULLSCREEN, nullptr);
			}
		}

		void Windows_InitExtensions()
		{
			wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
			wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
			if (wglGetExtensionsStringARB == nullptr && wglGetExtensionsStringEXT == nullptr)
			{
				AddErrorLog(error_e::noExtensions);
			}
			wglChoosePixelFormatARB	   = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
			wglChoosePixelFormatEXT	   = (PFNWGLCHOOSEPIXELFORMATEXTPROC)wglGetProcAddress("wglChoosePixelFormatEXT");
			wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
			wglSwapIntervalEXT		   = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
			wglGetSwapIntervalEXT	   = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");

			swapControlEXT				 = Windows_ExtensionSupported("WGL_EXT_swap_control");
			wglFramebufferSRGBCapableARB = Windows_ExtensionSupported("WGL_ARB_framebuffer_sRGB");

			wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
			wglGetPixelFormatAttribfvEXT = (PFNWGLGETPIXELFORMATATTRIBFVEXTPROC)wglGetProcAddress("wglGetPixelFormatAttribfvEXT");
			wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
			wglGetPixelFormatAttribivEXT = (PFNWGLGETPIXELFORMATATTRIBIVEXTPROC)wglGetProcAddress("wglGetPixelFormatAttribivEXT");
		}

		void Windows_InitGL(tWindow* window)
		{
			window->deviceContextHandle = GetDC(window->windowHandle);
			InitializePixelFormat(window);
			if (wglCreateContextAttribsARB)
			{
				int attribs[] {WGL_CONTEXT_MAJOR_VERSION_ARB,
							   window->settings.versionMajor,
							   WGL_CONTEXT_MINOR_VERSION_ARB,
							   window->settings.versionMinor,
							   WGL_CONTEXT_PROFILE_MASK_ARB,
							   window->settings.profile,
#if defined(_DEBUG)
							   WGL_CONTEXT_FLAGS_ARB,
							   WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
							   0};

				window->glRenderingContextHandle = wglCreateContextAttribsARB(window->deviceContextHandle, nullptr, attribs);

				if (window->glRenderingContextHandle == nullptr)
				{
					switch (GetLastError())
					{
						case ERROR_INVALID_VERSION_ARB:
							{
								AddErrorLog(window, error_e::invalidVersion, __LINE__, __FUNCTION__);
							}

						case ERROR_INVALID_PROFILE_ARB:
							{
								AddErrorLog(window, error_e::invalidProfile, __LINE__, __FUNCTION__);
							}
					}
				}
			}

			else
			{
				// use the old fashion method if the extensions aren't loading
				window->glRenderingContextHandle = wglCreateContext(window->deviceContextHandle);
			}

			wglMakeCurrent(window->deviceContextHandle, window->glRenderingContextHandle);

			window->contextCreated = (window->glRenderingContextHandle != nullptr);

			if (!window->contextCreated)
			{
				AddErrorLog(window, error_e::invalidContext, __LINE__, __FUNCTION__);
			}
		}

		void Windows_ShareContexts(tWindow* sourceWindow, tWindow* newWindow) { wglShareLists(sourceWindow->glRenderingContextHandle, newWindow->glRenderingContextHandle); }

		void ShutdownDummy()
		{
			if (dummyGLContextHandle)
			{
				wglMakeCurrent(nullptr, nullptr);
				wglDeleteContext(dummyGLContextHandle);
			}

			ReleaseDC(dummyWindowHandle, dummyDeviceContextHandle);
			UnregisterClass((wchar_t*)"dummy", dummyWindowInstance);

			FreeModule(dummyWindowInstance);

			dummyDeviceContextHandle = nullptr;
			dummyWindowHandle		 = nullptr;
			dummyGLContextHandle	 = nullptr;
		}

#endif
#pragma endregion
#pragma region Linux_Internal
#if defined(TW_LINUX)

		XEvent currentEvent;
		Display* currentDisplay;

		// PFNGLXGETCONTEXTIDEXTPROC glXGetContextIDEXT;
		PFNGLXSWAPINTERVALEXTPROC glxSwapIntervalEXT;
		PFNGLXSWAPINTERVALMESAPROC glxSwapIntervalMESA;
		PFNGLXGETSWAPINTERVALMESAPROC glXGetSwapIntervalMESA;
		PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;

		// clipboard Atoms
		Atom uriList;		/**< Atom for grabbing clipboard files paths */
		Atom clipboard;		/**< Atom for grabbing data from the clipboard */
		Atom utf8String;	/**< Atom for storing the clipboard data */
		Atom clipProperty;	/**< Atom for using the clipboard property */

		Atom clipMultiple;	/**< Atom for storing multiple files in the clipboard */
		Atom propUtf8;
		Atom propUriList;
		Atom propMultiple;

		struct MWMHints_t
		{
			unsigned long flags		  = 0;
			unsigned long functions	  = 0;
			unsigned long decorations = 0;
			long input_mode			  = 0;
			unsigned long status	  = 0;
		};

		typedef std::pair<Window, tWindow*> windowEntry;
		std::unordered_map<Window, tWindow*> windowLUT;

		typedef std::pair<uint32_t, key_e> keyEntry;
		const std::unordered_map<uint32_t, key_e> keyLUT =
		{
			keyEntry(XK_Escape, escape),
			keyEntry(XK_space, spacebar),
			keyEntry(XK_Home, home),
			keyEntry(XK_Left, arrowLeft),
			keyEntry(XK_Right, arrowRight),
			keyEntry(XK_Up, arrowUp),
			keyEntry(XK_Down, arrowDown),
			keyEntry(XK_Page_Up, pageUp),
			keyEntry(XK_Page_Down, pageDown),
			keyEntry(XK_End, end),
			keyEntry(XK_Print, printScreen),
			keyEntry(XK_Insert, insert),
			keyEntry(XK_Num_Lock, numLock),
			keyEntry(XK_KP_Multiply, keypadMultiply),
			keyEntry(XK_KP_Add, keypadAdd),
			keyEntry(XK_KP_Subtract, keypadSubtract),
			keyEntry(XK_KP_Decimal, keypadPeriod),
			keyEntry(XK_KP_Divide, keypadDivide),
			keyEntry(XK_KP_0, keypad0),
			keyEntry(XK_KP_1, keypad1),
			keyEntry(XK_KP_2, keypad2),
			keyEntry(XK_KP_3, keypad3),
			keyEntry(XK_KP_4, keypad4),
			keyEntry(XK_KP_5, keypad5),
			keyEntry(XK_KP_6, keypad6),
			keyEntry(XK_KP_7, keypad7),
			keyEntry(XK_KP_8, keypad8),
			keyEntry(XK_KP_9, keypad9),
			keyEntry(XK_F1, F1),
			keyEntry(XK_F2, F2),
			keyEntry(XK_F3, F3),
			keyEntry(XK_F4, F4),
			keyEntry(XK_F5, F5),
			keyEntry(XK_F6, F6),
			keyEntry(XK_F7, F7),
			keyEntry(XK_F8, F8),
			keyEntry(XK_F9, F9),
			keyEntry(XK_F10, F10),
			keyEntry(XK_F11, F11),
			keyEntry(XK_F12, F12),
			keyEntry(XK_Shift_L, leftShift),
			keyEntry(XK_Shift_R, rightShift),
			keyEntry(XK_Control_R, rightControl),
			keyEntry(XK_Control_L, leftControl),
			keyEntry(XK_Caps_Lock, capsLock),
			keyEntry(XK_Alt_L, leftAlt),
			keyEntry(XK_Alt_R, rightAlt),
		};

		tWindow* GetWindowByEvent(const XEvent& inEvent) const
		{
			switch (inEvent.type)
			{
				//use case fallthrough. not the biggest fan of this
				case Expose: return windowLUT.at(inEvent.xexpose.window);
				case DestroyNotify: return windowLUT.at(inEvent.xdestroywindow.window);
				case CreateNotify: return windowLUT.at(inEvent.xcreatewindow.window);
				case KeyPress:
				case KeyRelease: return windowLUT.at(inEvent.xkey.window);
				case ButtonPress:
				case ButtonRelease: return windowLUT.at(inEvent.xbutton.window);
				case MotionNotify: return windowLUT.at(inEvent.xmotion.window);
				case FocusIn:
				case FocusOut: return windowLUT.at(inEvent.xfocus.window);
				case ResizeRequest: return windowLUT.at(inEvent.xresizerequest.window);
				case ConfigureNotify: return windowLUT.at(inEvent.xconfigure.window);
				case PropertyNotify: return windowLUT.at(inEvent.xproperty.window);
				case GravityNotify: return windowLUT.at(inEvent.xgravity.window);
				case ClientMessage: return windowLUT.at(inEvent.xclient.window);
				case VisibilityNotify: return windowLUT.at(inEvent.xvisibility.window);
				default: return nullptr;
			}
		}

		void Linux_InitializeWindow(tWindow* window)
		{
			window->currentDisplay = currentDisplay;
			window->attributes = new int16_t[5] {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, window->settings.depthBits, None};

			window->linuxDecorators = 1;
			window->currentStyle |= tWindow::linuxClose | tWindow::linuxMaximize | tWindow::linuxMinimize | tWindow::linuxMove;

			if (!currentDisplay)
			{
				AddErrorLog(error_e::linuxCannotConnectXServer, window, __LINE__, __func__);
				return;
			}

			GetBestFrameBufferConfig(window);
			window->visualInfo = glXGetVisualFromFBConfig(window->currentDisplay, window->settings.bestFBConfig);

#if defined(DEBUG)

			//debug best Config
			int redBits, greenBits, blueBits = 0;
			int alphaBits, depthBits = 0;

			int totalBits = 0;
			glXGetFBConfigAttrib(window->currentDisplay, window->settings.bestFBConfig, GLX_RED_SIZE, &redBits);
			glXGetFBConfigAttrib(window->currentDisplay, window->settings.bestFBConfig, GLX_GREEN_SIZE, &greenBits);
			glXGetFBConfigAttrib(window->currentDisplay, window->settings.bestFBConfig, GLX_BLUE_SIZE, &blueBits);

			glXGetFBConfigAttrib(window->currentDisplay, window->settings.bestFBConfig, GLX_ALPHA_SIZE, &alphaBits);
			glXGetFBConfigAttrib(window->currentDisplay, window->settings.bestFBConfig, GLX_DEPTH_SIZE, &depthBits);
			glXGetFBConfigAttrib(window->currentDisplay, window->settings.bestFBConfig, GLX_BUFFER_SIZE, &totalBits);

			printf("red: %i green: %i blue: %i \n", redBits, greenBits, blueBits);
			printf("alpha: %i depth: %i totalBits %i \n", alphaBits, depthBits, totalBits);
#endif

			if (!window->visualInfo)
			{
				AddErrorLog(error_e::linuxInvalidVisualinfo, window, __LINE__, __func__);
				return;
			}

			window->setAttributes.colormap = XCreateColormap(window->currentDisplay, DefaultRootWindow(window->currentDisplay), window->visualInfo->visual, AllocNone);
			window->setAttributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | MotionNotify | ButtonPressMask | ButtonReleaseMask | FocusIn | FocusOut | Button1MotionMask | Button2MotionMask | Button3MotionMask | Button4MotionMask | Button5MotionMask | PointerMotionMask | FocusChangeMask | VisibilityChangeMask | PropertyChangeMask | SubstructureNotifyMask;
			window->windowHandle = XCreateWindow(window->currentDisplay, RootWindow(currentDisplay, window->visualInfo->screen), 0, 0, window->settings.resolution.width, window->settings.resolution.height, 0, window->visualInfo->depth, InputOutput, window->visualInfo->visual, CWColormap | CWEventMask, &window->setAttributes);

			if (!window->windowHandle)
			{
				AddErrorLog(error_e::linuxCannotCreateWindow, window, __LINE__, __func__);
				return;
			}

			XMapWindow(window->currentDisplay, window->windowHandle);
			XStoreName(window->currentDisplay, window->windowHandle, window->settings.name.c_str());
			XSetWMProtocols(window->currentDisplay, window->windowHandle, &window->AtomClose, true);

			XWindowAttributes attributes;
			Status status	 = XGetWindowAttributes(window->currentDisplay, window->windowHandle, &attributes);
			window->position = vec2_t<int16_t>((int16_t)attributes.x, (int16_t)attributes.y);

			windowLUT.emplace(windowEntry(window->windowHandle, window));

			InitializeGL(window);
		}

		void Linux_ShutdownWindow(tWindow* window) const { XDestroyWindow(currentDisplay, window->windowHandle); }

		void Linux_Shutdown() const
		{
			for (const auto& windowIndex: windowList)
				Linux_ShutdownWindow(windowIndex.get());

			XCloseDisplay(currentDisplay);
		}

		void Linux_ProcessEvents(XEvent inEvent)
		{
			tWindow* window = GetWindowByEvent(inEvent);

			switch (inEvent.type)
			{
				case Expose:
					{
						break;
					}

				case DestroyNotify:
					{
						if (destroyedEvent != nullptr)
							destroyedEvent(window);

						ShutdownWindow(window);
						break;
					}

				case CreateNotify:
					{
						// TODO: move some stuff here?
						break;
					}

				case KeyPress:
					{
						const uint16_t functionKeysym = XkbKeycodeToKeysym(currentDisplay, inEvent.xkey.keycode, 0, inEvent.xkey.state & ShiftMask ? 1 : 0);
						const uint16_t translatedKey	= Linux_TranslateKey(functionKeysym);
						window->keys[translatedKey] = keyState_e::down;
						if (keyEvent != nullptr)
							keyEvent(window, translatedKey, keyState_e::down);

						break;
					}

				case KeyRelease:
					{
						bool triggered = false;
						if (XEventsQueued(currentDisplay, QueuedAfterReading))
						{
							XEvent nextEvent;
							XPeekEvent(currentDisplay, &nextEvent);

							if (nextEvent.type == KeyPress && nextEvent.xkey.time == inEvent.xkey.time && nextEvent.xkey.keycode == inEvent.xkey.keycode)
							{
								const uint16_t functionKeysym = XkbKeycodeToKeysym(currentDisplay, inEvent.xkey.keycode, 0, inEvent.xkey.state & ShiftMask ? 1 : 0);
								const uint16_t translatedKey	= Linux_TranslateKey(functionKeysym);
								window->keys[translatedKey] = keyState_e::down;
								XNextEvent(currentDisplay, &inEvent);
								triggered = true;
								if (keyEvent != nullptr)
									keyEvent(window, Linux_TranslateKey(functionKeysym), keyState_e::down);
							}
						}

						if (triggered == false)
						{
							const uint16_t functionKeysym = XkbKeycodeToKeysym(currentDisplay, inEvent.xkey.keycode, 0, inEvent.xkey.state & ShiftMask ? 1 : 0);
							const uint16_t translatedKey	= Linux_TranslateKey(functionKeysym);
							window->keys[translatedKey] = keyState_e::up;

							if (keyEvent != nullptr)
								keyEvent(window, translatedKey, keyState_e::up);
						}

						break;
					}

				case ButtonPress:
					{
						switch (inEvent.xbutton.button)
						{
							case 1:
								{
									window->mouseButton[(uint16_t)mouseButton_e::left] = buttonState_e::down;

									if (mouseButtonEvent != nullptr)
										mouseButtonEvent(window, mouseButton_e::left, buttonState_e::down);
									break;
								}

							case 2:
								{
									window->mouseButton[(uint16_t)mouseButton_e::middle] = buttonState_e::down;

									if (mouseButtonEvent != nullptr)
										mouseButtonEvent(window, mouseButton_e::middle, buttonState_e::down);
									break;
								}

							case 3:
								{
									window->mouseButton[(uint16_t)mouseButton_e::right] = buttonState_e::down;

									if (mouseButtonEvent != nullptr)
										mouseButtonEvent(window, mouseButton_e::right, buttonState_e::down);
									break;
								}

							case 4:
								{
									//window->mouseButton[(uint16_t)mouseScroll_e::up] = buttonState_e::down;

									if (mouseWheelEvent != nullptr)
										mouseWheelEvent(window, mouseScroll_e::up);
									break;
								}

							case 5:
								{
									//window->mouseButton[(uint16_t)mouseScroll_e::down] = buttonState_e::down;

									if (mouseWheelEvent != nullptr)
										mouseWheelEvent(window, mouseScroll_e::down);
									break;
								}

							default:
								{
									// need to add more mouse buttons
									break;
								}
						}

						break;
					}

				case ButtonRelease:
					{
						switch (inEvent.xbutton.button)
						{
							case 1:
								{
									// the left mouse button was released
									window->mouseButton[(uint16_t)mouseButton_e::left] = buttonState_e::up;

									if (mouseButtonEvent != nullptr)
										mouseButtonEvent(window, mouseButton_e::left, buttonState_e::up);
									break;
								}

							case 2:
								{
									// the middle mouse button was released
									window->mouseButton[(uint16_t)mouseButton_e::middle] = buttonState_e::up;

									if (mouseButtonEvent != nullptr)
										mouseButtonEvent(window, mouseButton_e::middle, buttonState_e::up);
									break;
								}

							case 3:
								{
									// the right mouse button was released
									window->mouseButton[(uint16_t)mouseButton_e::right] = buttonState_e::up;

									if (mouseButtonEvent != nullptr)
										mouseButtonEvent(window, mouseButton_e::right, buttonState_e::up);
									break;
								}

							default:
								{
									// need to add more mouse buttons
									break;
								}
						}
						break;
					}

				// when the mouse/pointer device is moved
				case MotionNotify:
					{
						//setup previous mouse positons
						window->previousMousePosition = window->mousePosition;

						// set the windows mouse position to match the event
						window->mousePosition.x = (int16_t)inEvent.xmotion.x;
						window->mousePosition.y = (int16_t)inEvent.xmotion.y;

						/// set the screen mouse position to match the event
						screenMousePosition.x = (int16_t)inEvent.xmotion.x_root;
						screenMousePosition.y = (int16_t)inEvent.xmotion.y_root;

						if (mouseMoveEvent != nullptr)
							mouseMoveEvent(window, vec2_t<int16_t>((int16_t)inEvent.xmotion.x, (int16_t)inEvent.xmotion.y), vec2_t<int16_t>((int16_t)inEvent.xmotion.x_root, (int16_t)inEvent.xmotion.y_root));
						break;
					}

				// when the window goes out of focus
				case FocusOut:
					{
						window->inFocus = false;
						if (focusEvent != nullptr)
							focusEvent(window, window->inFocus);
						break;
					}

				// when the window is back in focus (use to call restore callback?)
				case FocusIn:
					{
						window->inFocus = true;

						if (focusEvent != nullptr)
							focusEvent(window, window->inFocus);
						break;
					}

				// when a request to resize the window is made either by
				// dragging out the window or programmatically
				case ResizeRequest:
					{
						window->settings.resolution.width  = inEvent.xresizerequest.width;
						window->settings.resolution.height = inEvent.xresizerequest.height;

						// kinda redundant for this to be here...
						// glViewport(0, 0, window->settings.resolution.width,
						// window->settings.resolution.height);

						if (resizeEvent != nullptr)
						{
							resizeEvent(window, vec2_t<uint16_t>(inEvent.xresizerequest.width, inEvent.xresizerequest.height));
						}
						break;
					}

				// when a request to configure the window is made
				case ConfigureNotify:
					{
						// check if window was resized
						if ((uint16_t)inEvent.xconfigure.width != window->settings.resolution.width || (uint16_t)inEvent.xconfigure.height != window->settings.resolution.height)
						{
							if (resizeEvent != nullptr)
							{
								resizeEvent(window, vec2_t<uint16_t>(inEvent.xconfigure.width, inEvent.xconfigure.height));
							}


							window->settings.resolution.width  = inEvent.xconfigure.width;
							window->settings.resolution.height = inEvent.xconfigure.height;
						}

						// check if window was moved
						if (inEvent.xconfigure.x != window->position.x || inEvent.xconfigure.y != window->position.y)
						{
							if (movedEvent != nullptr)
								movedEvent(window, vec2_t<int16_t>((int16_t)inEvent.xconfigure.x, (int16_t)inEvent.xconfigure.y));

							window->position.x = (int16_t)inEvent.xconfigure.x;
							window->position.y = (int16_t)inEvent.xconfigure.y;
						}
						break;
					}

				case PropertyNotify:
					{
						// this is needed in order to read from the windows WM_STATE Atomic
						// to determine if the property notify event was caused by a client
						// iconify Event(window, minimizing the window), a maximise event, a focus
						// event and an attention demand event. NOTE these should only be
						// for events that are not triggered programatically

						Atom type;
						int format;
						ulong numItems, bytesAfter;
						unsigned char* properties = nullptr;

						XGetWindowProperty(currentDisplay, inEvent.xproperty.window, window->AtomState, 0, LONG_MAX, false, AnyPropertyType, &type, &format, &numItems, &bytesAfter, &properties);

						if (properties && (format == 32))
						{
							XWindowAttributes attributes;
							XGetWindowAttributes(window->currentDisplay, window->windowHandle, &attributes);
							window->settings.resolution.width = attributes.width;
							window->settings.resolution.height = attributes.height;
						
							// go through each property and match it to an existing Atomic state
							for (ulong itemIndex = 0; itemIndex < numItems; itemIndex++)
							{
								Atom currentProperty = ((long*)(properties))[itemIndex];

								if (currentProperty == window->AtomStateHidden)
								{
									// window was minimized
									if (minimizedEvent != nullptr)
									{
										// if the minimized callback for the window was set
										minimizedEvent(window);
									}
								}

								if (currentProperty == window->AtomStateMaximizedVert || currentProperty == window->AtomStateMaximizedHorz)
								{
									// window was maximized
									if (maximizedEvent != nullptr)
									{
										// if the maximized callback for the window was set
										maximizedEvent(window);
									}
								}

								if (currentProperty == window->AtomDemandsAttention)
								{
									// the window demands user attention
								}
							}
						}

						break;
					}

				case GravityNotify:
					{
						// this is only supposed to pop up when the parent of this window(if any)
						// has something happen to it so that this window can react to said event
						// as well.
						break;
					}

				// check for events that were created by the TinyWindow manager
				case ClientMessage:
					{
						const char* atomName = XGetAtomName(currentDisplay, inEvent.xclient.message_type);
						if (atomName != nullptr) {}

						if ((Atom)inEvent.xclient.data.l[0] == window->AtomClose)
						{
							window->shouldClose = true;
							if (destroyedEvent != nullptr)
								destroyedEvent(window);
							break;
						}

						// check if full screen
						if ((Atom)inEvent.xclient.data.l[1] == window->AtomFullScreen)
							break;
						break;
					}
			default: {};
			}

			if (inEvent.type == window->AtomXDNDPosition)
			{
				// Accept the drop and specify position
				XClientMessageEvent response;
				response.type		  = ClientMessage;
				response.display	  = currentDisplay;
				response.window		  = inEvent.xclient.data.l[0];
				response.message_type = window->AtomXDNDStatus;
				response.format		  = 32;
				response.data.l[0]	  = window->windowHandle;
				response.data.l[1]	  = 1;// Accept drop
				response.data.l[2]	  = 0;// x,y coordinates for rectangle
				response.data.l[3]	  = 0;// w,h coordinates for rectangle
				response.data.l[4]	  = window->AtomXDNDActionCopy;

				XSendEvent(currentDisplay, inEvent.xclient.data.l[0], false, NoEventMask, (XEvent*)&response);
				XFlush(currentDisplay);
			}
			else if (inEvent.type == window->AtomXDNDDrop)
			{
				Atom selection = window->AtomXDNDSelection;

				XConvertSelection(currentDisplay, selection, window->AtomXDNDTextUriList, selection, window->windowHandle, inEvent.xclient.data.l[2]);

				// Send finished message
				XClientMessageEvent finished;
				finished.type		  = ClientMessage;
				finished.display	  = currentDisplay;
				finished.window		  = window->windowHandle;
				finished.message_type = window->AtomXDNDFinished;
				finished.format		  = 32;
				finished.data.l[0]	  = window->windowHandle;
				finished.data.l[1]	  = 1;
				finished.data.l[2]	  = window->AtomXDNDActionCopy;// Indicate we performed a copy

				XSendEvent(currentDisplay, window->windowHandle, false, NoEventMask, (XEvent*)&finished);
				XFlush(currentDisplay);
			}
		}

		// debugging. used to determine what type of event was generated
		static const char* Linux_GetEventType(const XEvent& currentEvent)
		{
			switch (currentEvent.type)
			{
				case MotionNotify: return "Motion Notify Event\n";
				case ButtonPress: return "Button Press Event\n";
				case ButtonRelease: return "Button Release Event\n";
				case ColormapNotify: return "Color Map Notify event \n";
				case EnterNotify: return "Enter Notify Event\n";
				case LeaveNotify: return "Leave Notify Event\n";
				case Expose: return "Expose Event\n";
				case GraphicsExpose: return "Graphics expose event\n";
				case NoExpose: return "No Expose Event\n";
				case FocusIn: return "Focus In Event\n";
				case FocusOut: return "Focus Out Event\n";
				case KeymapNotify: return "Key Map Notify Event\n";
				case KeyPress: return "Key Press Event\n";
				case KeyRelease: return "Key Release Event\n";
				case PropertyNotify: return "Property Notify Event\n";
				case ResizeRequest: return "Resize Property Event\n";
				case CirculateNotify: return "Circulate Notify Event\n";
				case ConfigureNotify: return "configure Notify Event\n";
				case DestroyNotify: return "Destroy Notify Request\n";
				case GravityNotify: return "Gravity Notify Event \n";
				case MapNotify: return "Map Notify Event\n";
				case ReparentNotify: return "Reparent Notify Event\n";
				case UnmapNotify: return "Unmap notify event\n";
				case MapRequest: return "Map request event\n";
				case ClientMessage: return "Client Message Event\n";
				case MappingNotify: return "Mapping notify event\n";
				case SelectionClear: return "Selection Clear event\n";
				case SelectionNotify: return "Selection Notify Event\n";
				case SelectionRequest: return "Selection Request event\n";
				case VisibilityNotify: return "Visibility Notify Event\n";
				default: return nullptr;
			}
		}

		// translate keys from X keys to TinyWindow Keys
		uint16_t Linux_TranslateKey(const uint16_t& keySymbol) const
		{
			if (keyLUT.contains(keySymbol))
			{
				return keyLUT.at(keySymbol);
			}
			return keySymbol;
		}

		void Linux_SetWindowIcon(tWindow* window)
		{
			/*std::unique_ptr<window_t> window, const char*
				icon, uint16_t width, uint16_t height */
			// sorry :(
			AddErrorLog(error_e::linuxFunctionNotImplemented, window, __LINE__, __func__);
		}

		void Linux_InitExtensions()
		{
			glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB((const unsigned char*)"glXCreateContextAttribsARB");
			glXGetSwapIntervalMESA	   = (PFNGLXGETSWAPINTERVALMESAPROC)glXGetProcAddressARB((const unsigned char*)"glXGetSwapIntervalMESA");
			glxSwapIntervalEXT		   = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const unsigned char*)"glXSwapIntervalEXT");
			glxSwapIntervalMESA		   = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddressARB((const unsigned char*)"glXSwapIntervalMESA");
		}

		void GetBestFrameBufferConfig(tWindow* window)
		{
			const int visualAttributes[] =
				{
					GLX_X_RENDERABLE, true,
					GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
					GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
					GLX_RENDER_TYPE, GLX_RGBA_BIT,
					GLX_RED_SIZE, window->settings.colorBits,
					GLX_GREEN_SIZE, window->settings.colorBits,
					GLX_BLUE_SIZE, window->settings.colorBits,
					GLX_ALPHA_SIZE, window->settings.colorBits,
					GLX_DEPTH_SIZE, window->settings.depthBits,
					GLX_STENCIL_SIZE, window->settings.stencilBits,
					GLX_BUFFER_SIZE, 32,
					GLX_DOUBLEBUFFER, true,
					None
				};

			int frameBufferCount = 0;
			uint16_t bestBufferConfig = 0;//, bestNumSamples = 0;
			GLXFBConfig* configs = glXChooseFBConfig(window->currentDisplay, 0, visualAttributes, &frameBufferCount);

			if (configs == nullptr || frameBufferCount == 0)
			{
				AddErrorLog(error_e::linuxNoValidFBConfig, window, __LINE__, __func__);
				return;
			}


			int currentConfigHighScore = 0;
			for (int configIndex = 0; configIndex < frameBufferCount; configIndex++) //print out all the info for these for debugging
			{
				XVisualInfo* visualInfo = glXGetVisualFromFBConfig(window->currentDisplay, configs[configIndex]);

				int r, g, b, a = 0;
				int stencilSize, numSamples, numSampleBuffers = 0;

				glXGetFBConfigAttrib(window->currentDisplay, configs[configIndex], GLX_RED_SIZE, &r);
				glXGetFBConfigAttrib(window->currentDisplay, configs[configIndex], GLX_GREEN_SIZE, &g);
				glXGetFBConfigAttrib(window->currentDisplay, configs[configIndex], GLX_BLUE_SIZE, &b);
				glXGetFBConfigAttrib(window->currentDisplay, configs[configIndex], GLX_ALPHA_SIZE, &a);

				glXGetFBConfigAttrib(window->currentDisplay, configs[configIndex], GLX_STENCIL_SIZE, &stencilSize);
				glXGetFBConfigAttrib(window->currentDisplay, configs[configIndex], GLX_SAMPLES, &numSamples);
				glXGetFBConfigAttrib(window->currentDisplay, configs[configIndex], GLX_SAMPLE_BUFFERS, &numSampleBuffers);

#if defined(DEBUG)
				printf("Config %d: R:%d G:%d B:%d A:%d \n", configIndex, r, g, b, a);
				printf("stencil size: %i \n", stencilSize);

				printf("Bits per pixel: %i \n", visualInfo->bits_per_rgb);
				printf("depth: %i \n", visualInfo->depth);
				printf("color depth %i \n", visualInfo->colormap_size);
#endif

				//now from this point how we get the best configs to use?
				//or do we just use the first in the list?

				//we could use a point system tol determine how close the current config is to the selected config
				int currentPoints = 0;
				currentPoints += (window->settings.colorBits == r);
				currentPoints += (window->settings.colorBits == g);
				currentPoints += (window->settings.colorBits == b);
				currentPoints += (window->settings.colorBits == a);
				currentPoints += (window->settings.depthBits == visualInfo->depth);
				currentPoints += (window->settings.stencilBits == stencilSize);
				currentPoints += (window->settings.colorBits == visualInfo->bits_per_rgb);

				//printf("config score %i \n\n", currentPoints);

				if (currentPoints > currentConfigHighScore)
				{
					currentConfigHighScore = currentPoints;
					bestBufferConfig = configIndex;
				}
				XFree(visualInfo);
			}

			window->settings.bestFBConfig = configs[bestBufferConfig];
			//GLXFBConfig BestConfig = configs[bestBufferConfig];
			XFree(configs);
		}

		void Linux_InitGL(tWindow* window)
		{
			if (glXCreateContextAttribsARB != nullptr)
			{
				//GetBestFrameBufferConfig(window);
				GLXContext dummyContext = glXCreateNewContext(window->currentDisplay, window->settings.bestFBConfig, GLX_RGBA_TYPE, nullptr, true);

				if (dummyContext == nullptr)
				{
					AddErrorLog(error_e::linuxFunctionNotImplemented, window, __LINE__, __func__);
					return;
				}

				Linux_InitExtensions();

				int attribs[] {GLX_CONTEXT_MAJOR_VERSION_ARB, window->settings.versionMajor,
							   GLX_CONTEXT_MINOR_VERSION_ARB, window->settings.versionMinor,
							   GLX_CONTEXT_PROFILE_MASK_ARB, window->settings.profile,
#if defined(_DEBUG)
							   GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
							   0};

				window->context = glXCreateContextAttribsARB(window->currentDisplay, window->settings.bestFBConfig, nullptr, true, attribs);
				glXDestroyContext(window->currentDisplay, dummyContext);

				if (window->context == nullptr)
				{
					AddErrorLog(error_e::invalidContext, window, __LINE__, __func__);
					return;
				}
			}
			else
			{
				window->context = glXCreateContext(window->currentDisplay, window->visualInfo, nullptr, true);
			}

			if (window->context)
			{
				if (glXMakeCurrent(window->currentDisplay, window->windowHandle, window->context) == false)
				{
					AddErrorLog(error_e::dummyCannotMakeCurrent, window, __LINE__, __func__);
					return;
				}

				XWindowAttributes attributes;
				XGetWindowAttributes(window->currentDisplay, window->windowHandle, &attributes);
				window->position.x = (int16_t)attributes.x;
				window->position.y = (int16_t)attributes.y;

				window->contextCreated = true;
				window->InitializeAtoms();
			}
			else
			{
				AddErrorLog(error_e::linuxCannotConnectXServer, window, __LINE__, __func__);
			}
		}

		static void GetExtraMonitorData(const XRROutputInfo* inInfo, const RROutput& inOutput, monitor_t* inMonitor)
		{
			// throw in a vector of monitors and the output
			std::string GPUName;
			std::string monitorName;
			auto rootDisplay						= XOpenDisplay(nullptr);
			const auto rootWindow			= XDefaultRootWindow(rootDisplay);
			XRRScreenResources* screenResources		= XRRGetScreenResources(rootDisplay, rootWindow);

			Atom edid_atom = XInternAtom(rootDisplay, RR_PROPERTY_RANDR_EDID, False);
			unsigned char* edid_data;
			int actual_format;
			unsigned long numItems, bytes_after;
			Atom actual_type;

			if (XRRGetOutputProperty(rootDisplay, inOutput, edid_atom, 0, 100, False, False, AnyPropertyType, &actual_type, &actual_format, &numItems, &bytes_after, &edid_data) == Success)
			{
				std::vector<unsigned char> edid(edid_data, edid_data + numItems);

				if (edid.size() > 128)
				{
					// Manufacturer ID (3 characters)
					char mfg_id[4] = {0};
					mfg_id[0]	   = ((edid[8] & 0x7C) >> 2) + '@';
					mfg_id[1]	   = ((edid[8] & 0x03) << 3) + ((edid[9] & 0xE0) >> 5) + '@';
					mfg_id[2]	   = (edid[9] & 0x1F) + '@';

					monitorName = std::string(mfg_id, 4);
				}
			}

			inMonitor->displayName = inInfo->name;
			inMonitor->monitorName = monitorName;

			// printf("output: %s \n", inMonitor->GetDisplayName()->c_str());

			auto primaryOutput = XRRGetOutputPrimary(rootDisplay, rootWindow);
			bool isPrimary	   = (inOutput == primaryOutput);
			auto crtcInfo	   = XRRGetCrtcInfo(rootDisplay, screenResources, inInfo->crtc);

			inMonitor->extents	  = vec4_t<uint16_t>(crtcInfo->x, crtcInfo->y, crtcInfo->width, crtcInfo->height);
			inMonitor->resolution = vec2_t<uint16_t>(crtcInfo->width, crtcInfo->height);
			inMonitor->isPrimary  = isPrimary;
		}

		void InitXRandR()
		{
			auto rootDisplay = XOpenDisplay(nullptr);

			// Get the number of screens
			int screenCount = 0;

			// I NEED A WHOLE OTHER EXTENSION JUST TO SEE HOW MANY SCREENS ARE ACTUALLY
			// ATTACHED!
			if (XineramaIsActive(rootDisplay) == true)
			{
				XineramaScreenInfo* xineramaInfo = nullptr;
				xineramaInfo					 = XineramaQueryScreens(rootDisplay, &screenCount);
#if defined(_DEBUG)
				printf("num screens: %i\n", screenCount);
#endif
			}

			for (int screenIndex = 0; screenIndex < screenCount; ++screenIndex)
			{
				// one monitor per screen
				monitor_t newMonitor;
				Window root = RootWindow(rootDisplay, 0);
				std::vector<monitorSetting_t> settings;

				// Get screen resources
				XRRScreenResources* screenResources		= XRRGetScreenResources(rootDisplay, root);
				XRRProviderResources* providerResources = XRRGetProviderResources(rootDisplay, root);

				Rotation currentRotation;
				XRRRotations(rootDisplay, screenIndex, &currentRotation);

				newMonitor.rotation = currentRotation;

				// get GPU name
				if (providerResources->nproviders > 0)
				{
					for (size_t providerIter = 0; providerIter < providerResources->nproviders; providerIter++)
					{
						XRRProviderInfo* providerInfo = XRRGetProviderInfo(rootDisplay, screenResources, providerResources->providers[providerIter]);

						if (providerInfo != nullptr)
						{
							// one monitor_t per valid output
							for (size_t iter = 0; iter < providerInfo->noutputs; iter++)
							{
								auto outputInfo = XRRGetOutputInfo(rootDisplay, screenResources, providerInfo->outputs[iter]);
								// if there are valid modes
								if (outputInfo->nmode > 0)
								{
									for (size_t crtcIter = 0; crtcIter < outputInfo->ncrtc; crtcIter++)
									{
										RRCrtc crtc			  = outputInfo->crtcs[crtcIter];
										XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(rootDisplay, screenResources, crtc);

										// now go through modes
										for (size_t modeIter = 0; modeIter < screenResources->nmode; modeIter++)
										{
											XRRModeInfo currentMode = screenResources->modes[modeIter];
											monitorSetting_t newSetting;
											newSetting.resolution		= vec2_t<uint16_t>(currentMode.width, currentMode.height);
											newSetting.displayFrequency = (uint16_t)((double)currentMode.dotClock / ((double)currentMode.hTotal * (double)currentMode.vTotal));
											newSetting.output			= screenResources->outputs[iter];
											newSetting.crtc				= crtc;
											newSetting.mode				= currentMode.id;
											settings.push_back(newSetting);
											if (currentMode.id == crtcInfo->mode)
												newMonitor.currentSetting = newSetting;
										}
									}
									newMonitor.deviceName = std::string(providerInfo->name);
									newMonitor.settings	  = settings;
									GetExtraMonitorData(outputInfo, providerInfo->outputs[iter], &newMonitor);
									monitorList.push_back(newMonitor);
									break;
								}
							}
						}
					}
				}
				else
				{
					for (size_t iter = 0; iter < screenResources->noutput; iter++)
					{
						XRROutputInfo* outputInfo = XRRGetOutputInfo(rootDisplay, screenResources, screenResources->outputs[iter]);
						// if there are valid modes
						if (outputInfo->nmode > 0)
						{
							for (size_t crtcIter = 0; crtcIter < outputInfo->ncrtc; crtcIter++)
							{
								RRCrtc crtc			  = outputInfo->crtcs[crtcIter];
								XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(rootDisplay, screenResources, crtc);

								// now go through modes
								for (size_t modeIter = 0; modeIter < screenResources->nmode; modeIter++)
								{
									XRRModeInfo currentMode = screenResources->modes[modeIter];
									monitorSetting_t newSetting;
									newSetting.resolution		= vec2_t<uint16_t>(currentMode.width, currentMode.height);
									newSetting.displayFrequency = (uint16_t)((double)currentMode.dotClock / ((double)currentMode.hTotal * (double)currentMode.vTotal));
									newSetting.output			= screenResources->outputs[iter];
									newSetting.crtc				= crtc;
									newSetting.mode				= currentMode.id;
									settings.push_back(newSetting);
									// printf("name %s \n", currentMode.name);
									if (currentMode.id == crtcInfo->mode)
										newMonitor.currentSetting = newSetting;
								}
							}
							newMonitor.deviceName = std::string(outputInfo->name);
							newMonitor.settings	  = settings;

							GetExtraMonitorData(outputInfo, screenResources->outputs[iter], &newMonitor);
							monitorList.push_back(newMonitor);
							break;
						}
					}
				}
			}
		}



		void Linux_ToggleFullscreen(tWindow* window, monitor_t* monitor, const uint16_t& monitorSettingIndex) const
		{
			// set window position and change style to popup
			window->currentMonitor = monitor;
			const auto rootDisplay	   = XOpenDisplay(nullptr);

			monitorSetting_t* monitorSetting = const_cast<monitorSetting_t*>(&monitor->GetMonitorSettings()->at(monitorSettingIndex));
			const Window root						 = RootWindow(rootDisplay, 0);

			// Get screen resources
			XRRScreenResources* screenResources = XRRGetScreenResources(rootDisplay, root);

			// Get the current screen configuration
			XRRScreenConfiguration* conf = XRRGetScreenInfo(rootDisplay, root);
			int result					 = 0;

			if (window->isFullscreen == false)
			{
				// Save the current window state
				window->previousDimensions = window->settings.resolution; // Assuming currentDimensions exists
				window->previousPosition = window->previousPosition;     // Assuming currentPosition exists
				result = XRRSetCrtcConfig(rootDisplay, screenResources, monitorSetting->crtc, CurrentTime,
										  (int)monitor->extents.left, (int)monitor->extents.top,
										  monitorSetting->mode, monitor->rotation, &monitorSetting->output, 1);
				XSync(rootDisplay, True);
				if (result == Success)
				{
					SetWindowSize(window, vec2_t<uint16_t>(monitor->resolution.width, monitor->resolution.height));
					SetPosition(window, vec2_t<int16_t>(0, 0)); // Use (0,0) as origin after mode change
					SetDecorators(window, style_n::none);
					window->isFullscreen = true;
				}
			}

			else if (window->isFullscreen == true)
			{
				result = XRRSetCrtcConfig(rootDisplay, screenResources, monitor->currentSetting.crtc, CurrentTime,
				(int)monitor->extents.left, (int)monitor->extents.top,
				monitor->currentSetting.mode, monitor->rotation, &monitor->currentSetting.output, 1);

				XSync(rootDisplay, True);
				if (result == Success)
				{
					window->isFullscreen = false;
					SetWindowSize(window, vec2_t<uint16_t>(window->previousDimensions.width, window->previousDimensions.height));
					SetPosition(window, vec2_t<int16_t>(window->previousPosition.x, window->previousPosition.y));
					SetDecorators(window, style_n::normal); // Restore original style if different
				}
			}
		}

		void InitializeAtoms()
		{
			clipboard  = XInternAtom(currentDisplay, "CLIPBOARD", False);
			utf8String = XInternAtom(currentDisplay, "UTF8_STRING", False);
			uriList	   = XInternAtom(currentDisplay, "text/uri-list", False);
			clipProperty = XInternAtom(currentDisplay, "CLIPBOARD_PROPERTY", False);

			clipMultiple = XInternAtom(currentDisplay, "MULTIPLE", False);
			propMultiple = XInternAtom(currentDisplay, "MY_CLIP_MULTI", False);
			propUtf8 = XInternAtom(currentDisplay, "MY_CLIP_UTF8", False);
			propUriList = XInternAtom(currentDisplay, "MY_CLIP_URI", False);
		}

	std::vector<std::string> Linux_GetClipboardLatest(tWindow* window, clipboard_e& clipType) const
	{
		// Check if there is a selection owner
		Atom sel_owner = XGetSelectionOwner(currentDisplay, clipboard);
		if (sel_owner == None)
		{
			clipType = clipboard_e::invalid;
			return std::vector<std::string>();
		}

		Atom pairs[] =
		{
			uriList, propUriList,  // Prioritize URI list (files) first
			utf8String, propUtf8
		};

		// Put the pairs into propMultiple (type = XA_ATOM, format = 32)
		XChangeProperty(currentDisplay, window->windowHandle, propMultiple, XA_ATOM, 32, PropModeReplace,
						(unsigned char*)pairs, sizeof(pairs) / sizeof(pairs[0]));

		// Request the clipboard selection with MULTIPLE
		XConvertSelection(currentDisplay, clipboard, clipMultiple, propMultiple, window->windowHandle, CurrentTime);

		XFlush(currentDisplay); // Ensure the request is sent immediately

		// Temporary event loop to wait for SelectionNotify
		XEvent event;
		bool selectionReceived = false;
		Atom received_property = None;
		for (int i = 0; i < 200 && !selectionReceived; i++) // Increased to 200 for 2s timeout
		{
			if (XPending(currentDisplay))
			{
				XNextEvent(currentDisplay, &event);
				if (event.type == SelectionNotify && event.xselection.selection == clipboard)
				{
					selectionReceived = true;
					received_property = event.xselection.property;
				}
			}
			else
			{
				usleep(10000); // Sleep 10ms to avoid busy-waiting
			}
		}

		if (!selectionReceived)
		{
			// Timeout or no response
			clipType = clipboard_e::invalid;
			return std::vector<std::string>();
		}

		bool multiple_success = (received_property != None);

		Atom actualType;
		int actualFormat;
		unsigned long nitems, bytesAfter;
		unsigned char* data = nullptr;
		std::vector<std::string> result;

		if (multiple_success)
		{
			// MULTIPLE succeeded, process as before
			int result_get = XGetWindowProperty(currentDisplay, window->windowHandle, propMultiple, 0, LONG_MAX / 4, False,
											AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesAfter, &data);

			if (result_get != Success)
			{
				clipType = clipboard_e::invalid;
				return std::vector<std::string>();
			}

			std::vector<std::pair<Atom, Atom>> propPairs;
			Atom* atoms = reinterpret_cast<Atom*>(data); // nitems atoms
			for (unsigned long i = 0; i < nitems; i += 2)
			{
				propPairs.emplace_back(atoms[i + 0], atoms[i + 1]); // (target, property)
			}
			if (data) XFree(data); // Free after extracting pairs

			for (uint8_t i = 0; i < propPairs.size(); i++)
			{
				int localResult = XGetWindowProperty(currentDisplay, window->windowHandle, propPairs[i].second, 0, LONG_MAX / 4, False,
													 AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesAfter, &data);

				if (localResult != Success || !data || actualType == None)
				{
					if (data) XFree(data);
					continue; // Skip to next pair
				}

				if (actualType == utf8String)
				{
					// Successfully retrieved data
					clipType = clipboard_e::text;
					result.push_back(std::string((char*)data, nitems));
					XFree(data); // Free data before returning
					return result;
				}
				else if (actualType == uriList)
				{
					clipType = clipboard_e::files;
					// Parse the text/uri-list data
					std::string uriData((char*)data, nitems);
					XFree(data); // Free data

					std::string line;
					for (size_t i = 0, start = 0; i <= uriData.size(); ++i)
					{
						if (i == uriData.size() || uriData[i] == '\n' || uriData[i] == '\r')
						{
							if (i > start)
							{
								line = uriData.substr(start, i - start);
								// Skip comments and empty lines
								if (!line.empty() && line[0] != '#')
								{
									// Check for file:// URI
									if (line.find("file://") == 0)
									{
										// Remove file:// prefix and decode basic URL encoding
										std::string path = line.substr(7); // Skip "file://"
										// Simple URL decoding for common cases (e.g., %20 -> space)
										std::string decoded;
										for (size_t j = 0; j < path.size(); ++j)
										{
											if (path[j] == '%' && j + 2 < path.size() &&
												isxdigit(path[j + 1]) && isxdigit(path[j + 2]))
											{
												std::string hex = path.substr(j + 1, 2);
												char decodedChar = static_cast<char>(std::stoi(hex, nullptr, 16));
												decoded += decodedChar;
												j += 2;
											}
											else
											{
												decoded += path[j];
											}
										}
										result.push_back(decoded);
									}
								}
							}
							start = i + 1;
						}
					}
					return result;
				}
				else
				{
					XFree(data);
				}
			}
		}
		else
		{
			// MULTIPLE failed (property == None), fallback to single requests
			// Try uriList first
			XConvertSelection(currentDisplay, clipboard, uriList, propUriList, window->windowHandle, CurrentTime);
			XFlush(currentDisplay);

			selectionReceived = false;
			received_property = None;
			for (int i = 0; i < 200 && !selectionReceived; i++)
			{
				if (XPending(currentDisplay))
				{
					XNextEvent(currentDisplay, &event);
					if (event.type == SelectionNotify && event.xselection.selection == clipboard)
					{
						selectionReceived = true;
						received_property = event.xselection.property;
					}
				}
				else
				{
					usleep(10000);
				}
			}

			if (selectionReceived && received_property != None)
			{
				int localResult = XGetWindowProperty(currentDisplay, window->windowHandle, propUriList, 0, LONG_MAX / 4, False,
													 AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesAfter, &data);

				if (localResult == Success && data && actualType != None)
				{
					if (actualType == uriList)
					{
						clipType = clipboard_e::files;
						std::string uriData((char*)data, nitems);
						XFree(data);

						std::string line;
						for (size_t i = 0, start = 0; i <= uriData.size(); ++i)
						{
							if (i == uriData.size() || uriData[i] == '\n' || uriData[i] == '\r')
							{
								if (i > start)
								{
									line = uriData.substr(start, i - start);
									if (!line.empty() && line[0] != '#')
									{
										if (line.find("file://") == 0)
										{
											std::string path = line.substr(7);
											std::string decoded;
											for (size_t j = 0; j < path.size(); ++j)
											{
												if (path[j] == '%' && j + 2 < path.size() &&
													isxdigit(path[j + 1]) && isxdigit(path[j + 2]))
												{
													std::string hex = path.substr(j + 1, 2);
													char decodedChar = static_cast<char>(std::stoi(hex, nullptr, 16));
													decoded += decodedChar;
													j += 2;
												}
												else
												{
													decoded += path[j];
												}
											}
											result.push_back(decoded);
										}
									}
								}
								start = i + 1;
							}
						}
						return result;
					}
				}
				if (data) XFree(data);
			}

			// If uriList failed, try utf8String
			XConvertSelection(currentDisplay, clipboard, utf8String, propUtf8, window->windowHandle, CurrentTime);
			XFlush(currentDisplay);

			selectionReceived = false;
			received_property = None;
			for (int i = 0; i < 200 && !selectionReceived; i++)
			{
				if (XPending(currentDisplay))
				{
					XNextEvent(currentDisplay, &event);
					if (event.type == SelectionNotify && event.xselection.selection == clipboard)
					{
						selectionReceived = true;
						received_property = event.xselection.property;
					}
				}
				else
				{
					usleep(10000);
				}
			}

			if (selectionReceived && received_property != None)
			{
				int localResult = XGetWindowProperty(currentDisplay, window->windowHandle, propUtf8, 0, LONG_MAX / 4, False,
													 AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesAfter, &data);

				if (localResult == Success && data && actualType != None)
				{
					if (actualType == utf8String)
					{
						clipType = clipboard_e::text;
						result.push_back(std::string((char*)data, nitems));
						XFree(data);
						return result;
					}
				}
				if (data) XFree(data);
			}
		}

		// If nothing succeeded
		clipType = clipboard_e::invalid;
		return std::vector<std::string>();
	}

#endif
#pragma endregion
	};
}// namespace TinyWindow

#endif

#pragma clang diagnostic pop
