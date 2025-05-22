if os.host() == "linux" then
    local cmake = require "cmake"
    --cmake.builddir = "build/cmake"
   -- location "build/cmake"
end

function scene_project(name, extra_includes)
    project(name)
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"
        
        targetdir (_SCRIPT_DIR .. "/examples/" .. name .. "/bin/")
        toolset "clang"
        debugdir "%{wks.location}/" -- Changed to use workspace location
        local extradir = "./examples/" .. name .. "/"
        local shaderPath = "%{wks.location}/assets/shaders/" .. name .. "/" .. name .. ".json"
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
        --end

        -- Add extra includes
        if extra_includes then
            for _, file in ipairs(extra_includes) do
                includedirs { file }
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
--scene_project("bindless", {"%{wks.location}/examples/textured/include/"})
scene_project("bubble", {"%{wks.location}/examples/textured/include/"})
scene_project("cellShading", {"%{wks.location}/examples/textured/include/"})
scene_project("cheapBlur", {"%{wks.location}/examples/textured/include/"})
scene_project("chromaticAbberation", {"%{wks.location}/examples/textured/include/"})
scene_project("computeTest")
scene_project("contrast", {"%{wks.location}/examples/textured/include/"})
scene_project("dilation", {"%{wks.location}/examples/textured/include/"})
scene_project("edgeDetection", {"%{wks.location}/examples/textured/include/"})
scene_project("erosion", {"%{wks.location}/examples/textured/include/"})
scene_project("heatHaze", {"%{wks.location}/examples/textured/include/", "%{wks.location}/examples/bubble/include/"})
--scene_project("frost", {"%{wks.location}/examples/textured/include/", "%{wks.location}/examples/heatHaze/include/"})
scene_project("gameOfLife") 
scene_project("gamma", {"%{wks.location}/examples/textured/include/"})
scene_project("gaussian", {"%{wks.location}/examples/textured/include/"})
scene_project("gaussianMulti" , {"%{wks.location}/examples/textured/include/"})
scene_project("GOLCompute", {"%{wks.location}/examples/gameOfLife/include/"})
scene_project("textureSettings", { "%{wks.location}/examples/textured/include/"})
scene_project("mipMapping", { "%{wks.location}/examples/textured/include/"})
scene_project("parallax", { "%{wks.location}/examples/textured/include/"})
scene_project("perlin3D", {"%{wks.location}/examples/perlin/include/"})
scene_project("pixelize", {"%{wks.location}/examples/textured/include/"})
scene_project("radialBlur", {"%{wks.location}/examples/textured/include/"})
scene_project("sepia",  {"%{wks.location}/examples/textured/include/"})
scene_project("sharpen", {"%{wks.location}/examples/textured/include/"})
--scene_project("dotProduct")

--3d projects
scene_project("scene3D")
scene_project("texturedScene3D", {"%{wks.location}/examples/scene3D/include/"})
scene_project("depthPrePass", {"%{wks.location}/examples/scene3D/include/", "%{wks.location}/examples/texturedScene3D/include/"})
--scene_project("FXAA", {"%{wks.location}/examples/scene3D/include/", "%{wks.location}/examples/texturedScene3D/include/"})