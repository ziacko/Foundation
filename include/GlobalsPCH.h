#pragma once

// Precompiled header for the project.
// NOTE: Do NOT place single-header library IMPLEMENTATION defines here.
// Those belong in a dedicated .cpp (see src/thirdparty_impl.cpp) to avoid ODR issues.

#define GLM_ENABLE_EXPERIMENTAL
#define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
#define IMGUI_IMPL_OPENGL_USE_BUFFER_BINDING

#ifndef DEBUG
#define TW_NO_CONSOLE
#endif

// When compiled as C (for .c third-party sources) we must not include C++ headers.
#ifdef __cplusplus

// C++ std libs
#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include <numeric>
#include <algorithm>
#include <cstddef>
#include <chrono>
#include <thread>
#include <array>
#include <list>
#include <filesystem>
#include <utility>

// External libs (headers only in PCH)
#include <TinyExtender.h>
namespace te = TinyExtender; using namespace te;
#include <TinyShaders.h>
namespace ts = TinyShaders; using namespace ts;
#include <TinyWindow.h>
namespace tw = TinyWindow; using namespace tw;
#include <TinyClock.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <gli/gli.hpp>
#include <stb_image.h>
#include <stb_image_write.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <yyjson.h>
#include <tsl/robin_map.h>
#include <ufbx.h>
#include <glext.h>

// Debug group helper macro
#ifndef GL_PUSH_DEBUG_GROUP
#define GL_PUSH_DEBUG_GROUP() glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, __FUNCTION__)
#endif

// Global constants / defaults (safe in PCH)
#define PI 3.14159265
constexpr glm::vec4 clearColor  = { 0.33f, 0.33f, 0.33f, 1.0f };
constexpr glm::vec4 clearColor2 = { 0.0f,  0.0f,  0.0f,  1.0f };
constexpr glm::vec4 clearDepth  = { 0.0f,  0.0f,  0.0f,  0.0f };
constexpr glm::vec4 clearColor3 = { 1.0f,  0.0f,  0.0f,  0.0f }; // debugging only

constexpr glm::ivec2 defaultWindowSize    = glm::ivec2(1280, 720);
constexpr float      defaultNearPlane     = 0.01f;
constexpr float      defaultFarPlane      = 10000.0f;
constexpr float      defaultFieldOfView   = 90.0f;
constexpr float      defaultCameraSpeed   = PI;
constexpr glm::ivec2 defaultViewportOrigin = glm::ivec2(0);

using namespace std::placeholders;

// Local / internal headers (keep last to leverage external definitions)
#include "Camera.h"
#include "DefaultUniformBuffer.h"
#include "GPUQuery.h"
#include "VertexBuffer.h"
#include "shaderLoader_t.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Model.h"

#endif // __cplusplus
