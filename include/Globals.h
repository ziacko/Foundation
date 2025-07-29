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
namespace te = TinyExtender;
using namespace te;
#include <TinyShaders.h>
namespace ts = TinyShaders;
using namespace ts;
#include <TinyWindow.h>
namespace tw = TinyWindow;
using namespace tw;
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
//#include <absl/container/inlined_vector.h>
//#include <absl/container/fixed_array.h>
//#include <absl/strings/string_view.h>
#include <tsl/robin_map.h>
#include <ufbx.h>


#include <glext.h>

//global defines
#define PI 3.14159265
constexpr glm::vec4 clearColor = {0.33f, 0.33f, 0.33f, 1.0f};
constexpr glm::vec4 clearColor2 = { 0.0f, 0.0f, 0.0f, 1.0f };

constexpr glm::ivec2 defaultWindowSize = glm::ivec2(1280, 720);

constexpr float defaultNearPlane = 0.01f;
constexpr float defaultFarPlane = 100.0f;
constexpr float defaultFieldOfView = 90.0f;
constexpr float defaultCameraSpeed = PI * 0.1;

constexpr glm::ivec2 defaultViewportOrigin = glm::ivec2(0);

//could put all opf these into a namespace but what to call it? globdefaults? gDefs?

//local headers
using namespace std::placeholders;
//internal libs
#include "Camera.h"
#include "DefaultUniformBuffer.h"
#include "GPUQuery.h"
#include "VertexBuffer.h"
#include "shaderLoader_t.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Model.h"


