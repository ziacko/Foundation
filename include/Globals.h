#pragma once
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#define QOI_IMPLEMENTATION
#define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
#define IMGUI_IMPL_OPENGL_USE_BUFFER_BINDING
//C++ libs
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
//external libs
#include <TinyExtender.h>
using namespace TinyExtender;
#include <TinyShaders.h>
using namespace TinyShaders;
#include <TinyWindow.h>
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
#include <imgui.h>
#include <yyjson.h>
#include <absl/container/inlined_vector.h>
#include <absl/container/fixed_array.h>
#include <absl/strings/string_view.h>
#include <tsl/robin_map.h>
#include <ufbx.h>
using namespace TinyWindow;
using namespace std::placeholders;
//internal libs
#include "Camera.h"
#include "DefaultUniformBuffer.h"
#include "GPUQuery.h"
#include "Utilities.h"
#include "VertexBuffer.h"
#include "shaderLoader_t.h"
#include "Texture.h"
#include "FrameBuffer.h"


//global defines
#define PI 3.14159265
constexpr float clearColor[4] = {0.33f, 0.33f, 0.33f, 1.0f};
constexpr float clearColor2[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

constexpr glm::vec2 defaultWindowSize = glm::vec2(1280, 720);