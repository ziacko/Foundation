if os.host() == "linux" then
    local cmake = require "cmake"
    cmake.workspace_directory = _SCRIPT_DIR
    cmake.write_settings = {
        CMAKE_CURRENT_SOURCE_DIR = _SCRIPT_DIR
    }
end

dofile (_SCRIPT_DIR .. "/premakeExtras/templates.lua")
dofile (_SCRIPT_DIR .. "/premakeExtras/helperFunctions.lua")

function scene_project(name, inheritances)
    project(name)
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"
        
        targetdir (_SCRIPT_DIR .. "/examples/" .. name .. "/bin/")
        toolset "clang"
        debugdir(_SCRIPT_DIR) -- Changed to use workspace location
        local extradir = "./examples/" .. name .. "/"
        local shaderPath = _SCRIPT_DIR .. "/assets/shaders/" .. name .. "/" .. name .. ".json"

        --use the last inheritance as the parent
        if inheritances and #inheritances > 0 then
            checkHeaderFile(name, inheritances[#inheritances])
        else
            checkHeaderFile(name)
        end
        checkSourceFile(name)

        --and now the same for shaders and the shader config
        checkShaderFileConfig(name)
        checkShaderFiles(name)

        -- common settings
        files {
            "examples/scene/include/**.h",
            "examples/" .. name .. "/include/**.h",
            "examples/" .. name .. "/source/**.cpp",
            "include/Globals.h",
            "lib/imgui/*.cpp",
            "lib/yyjson/src/yyjson.c",
            "lib/ufbx/ufbx.c",
            shaderPath,
        } 

        includedirs {
            "include/",
            "examples/scene/include/",
            "examples/" .. name .. "/include/",
            "lib/tinywindow/",
            "lib/tinywindow/Include",
            "lib/tinywindow/Dependencies",
            "lib/tinyextender/Include",
            "lib/tinyshaders/Include",
            "lib/tinyclock/Include",
            "lib/glm/",
            "lib/gli/",
            "lib/stb/",
            "lib/imgui/",
            "lib/robin-map/include/",
            "lib/abseil-cpp/absl/",
            "lib/cereal/",
            "lib/eigen/",
            "lib/eve/",
            "lib/fast_float/",
            "lib/fast_obj/",
            "lib/Remotery/",
            "lib/yyjson/src/",
            "lib/ufbx/"
        }

       --if extra_files and #extra_files > 0 then
        defines {
            "SHADER_CONFIG_DIR=\"" .. name .. "\"",
            "ASSET_DIR=\"" .. _SCRIPT_DIR .. "/assets/\"",
            "PROJECT_NAME=\"" .. name .. "\"",
        }

        -- Add extra includes
        if inheritances then
            for _, file in ipairs(inheritances) do
                --surround the project name with the include path
                local inheritPath = "%{wks.location}/examples/" .. file .. "/include/"
                --print("Adding extra include: " .. inheritPath)
                includedirs { inheritPath }
            end
        end

        filter { "system:windows" }
            toolset "clang"
            systemversion "latest"
            links { "opengl32.lib" }

        filter { "system:linux" }
            toolset "clang"
            links { "GL", "X11", "Xrandr", "Xinerama", "pthread" } -- Added pthread for Abseil

            -- Add CMake working directory
            debugdir(_SCRIPT_DIR)
end

location "build/cmake"

workspace "Portfolio"
    configurations { "Debug", "Release" }
    filter "system:linux"
        platforms { "Linux" }
    filter "system:windows"
        platforms { "Windows" }
    architecture "x64"

    --communal settings for all projects
    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "on"
        optimize "Off"
        targetdir "bin/Debug"

    filter { "configurations:Release" }
        optimize "on"
        symbols "off"

    filter {"platforms:Win64"}
    system "Windows"
    filter {"platforms:Linux"}
    system "Linux"

    filter "configurations:Debug"
        defines { "ABSL_DEBUG_SYNCHRONIZATION_VIOLATION" }
            
    filter "configurations:Release"
        defines { "ABSL_HARDENED" }
            
    filter {}  -- Reset filter

--base scene project

--2d projects
scene_project("scene")
scene_project("textured")
scene_project("perlin")
--scene_project("bindless", {"textured"})
scene_project("bubble", {"textured"})
scene_project("cellShading", {"textured"})
scene_project("cheapBlur", {"textured"})
scene_project("chromaticAbberation", {"textured"})
scene_project("computeTest")
scene_project("contrast", {"textured"})
scene_project("dilation", {"textured"})
scene_project("edgeDetection", {"textured"})
scene_project("erosion", {"textured"})
scene_project("heatHaze", {"textured", "bubble"})
--scene_project("frost", {"textured", "heatHaze"})
scene_project("gameOfLife") 
scene_project("gamma", {"textured"})
scene_project("gaussian", {"textured"})
scene_project("gaussianMulti" , {"textured"})
scene_project("GOLCompute", {"gameOfLife"})
scene_project("textureSettings", { "textured"})
scene_project("mipMapping", { "textured"})
scene_project("parallax", { "textured"})
scene_project("perlin3D", {"perlin"})
scene_project("pixelize", {"textured"})
scene_project("radialBlur", {"textured"})
scene_project("sepia",  {"textured"})
scene_project("sharpen", {"textured"})

--3d projects
scene_project("scene3D")
scene_project("texturedScene3D", {"scene3D"})
scene_project("depthPrePass", {"scene3D", "texturedScene3D"})
--scene_project("FXAA", {"scene3D", "texturedScene3D"})
