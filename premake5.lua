if os.host() == "linux" then
    local cmake = require "cmake"
    --cmake.builddir = "build/cmake"
   -- location "build/cmake"
end

function scene_project(name, extra_files, extra_includes)
    project(name)
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"
        
        targetdir (_SCRIPT_DIR .. "/examples/" .. name .. "/bin/")
        toolset "clang"
        debugdir "%{wks.location}/" -- Changed to use workspace location
        local extradir = "./examples/" .. name .. "/"
        -- common settings
        files {
            "examples/scene/include/**.h",
            --"examples/scene/source/**.cpp",
            "examples/" .. name .. "/include/**.h",
            "examples/" .. name .. "/source/**.cpp",
            "include/Globals.h",
            "lib/imgui/*.cpp",
            "lib/yyjson/src/yyjson.c",
            "lib/ufbx/ufbx.c",
        }

        --add extra files
        if extra_files then
            for _, file in ipairs(extra_files) do
                files { file }
            end
        end

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
            "lib/abseil-cpp/",
            "lib/cereal/",
            "lib/eigen/",
            "lib/eve/",
            "lib/fast_float/",
            "lib/fast_obj/",
            "lib/Remotery/",
            "lib/yyjson/src/",
            "lib/ufbx/"
        }

        if extra_files and #extra_files > 0 then
            defines {
                "SHADER_CONFIG_DIR=\"" .. extra_files[1] .. "\"",
                "ASSET_DIR=\"" .. _SCRIPT_DIR .. "/assets/\"",
            }
        end

        --add extra files
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
            links { "GL", "X11", "Xrandr", "Xinerama" }

            --add Cmake working directory
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

--base scene project

--2d projects
scene_project("scene", {"%{wks.location}/assets/shaders/scene/Default.json"}, {})
scene_project("textured", {"%{wks.location}/assets/shaders/textured/Textured.json"}, {})
scene_project("perlinNoise", {"%{wks.location}/assets/shaders/perlin/PerlinNoise.json"}, {})
scene_project("bindless", {
    "%{wks.location}/assets/shaders/bindless/Bindless.json"}, { 
    "%{wks.location}/examples/textured/include/"
})
scene_project("bubble", { 
    "%{wks.location}/assets/shaders/bubble/Bubble.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("cellShading", { 
    "%{wks.location}/assets/shaders/cellShading/CellShading.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("cheapBlur", { 
    "%{wks.location}/assets/shaders/cheapBlur/CheapBlur.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("chromaticAbberation", { 
    "%{wks.location}/assets/shaders/chromaticAbberation/ChromaticAbberation.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("computeTest", { 
    "%{wks.location}/assets/shaders/computeTest/ComputeTest.json"} )

scene_project("contrast", { 
    "%{wks.location}/assets/shaders/contrast/Contrast.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("dilation", { 
    "%{wks.location}/assets/shaders/dilation/Dilation.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("edgeDetection", { 
    "%{wks.location}/assets/shaders/edgeDetection/EdgeDetection.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("erosion", { 
    "%{wks.location}/assets/shaders/erosion/Erosion.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("heatHaze", { 
    "%{wks.location}/assets/shaders/heatHaze/HeatHaze.json"}, { 
    "%{wks.location}/examples/textured/include/", "%{wks.location}/examples/bubble/include/"
})

scene_project("frost", { 
    "%{wks.location}/assets/shaders/heatHaze/HeatHaze.json"}, { 
    "%{wks.location}/examples/textured/include/", "%{wks.location}/examples/heatHaze/include/"
})

scene_project("gameOfLife", { 
    "%{wks.location}/assets/shaders/gameOfLife/GameOfLife.json"}, {}) 

scene_project("gamma", { 
    "%{wks.location}/assets/shaders/gamma/Gamma.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("gaussian", { 
    "%{wks.location}/assets/shaders/gaussian/Gaussian.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("gaussianMulti" , { 
    "%{wks.location}/assets/shaders/gaussianMulti/GaussianMulti.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("golCompute", { 
    "%{wks.location}/assets/shaders/golCompute/GOLCompute.json"}, {"%{wks.location}/examples/gameOfLife/include/"}) 
    
scene_project("textureSettings", { 
    "%{wks.location}/assets/shaders/textureSettings/TextureSettings.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("mipMapping", { 
    "%{wks.location}/assets/shaders/mipMapping/MipMapping.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("parallax", { 
    "%{wks.location}/assets/shaders/parallax/Parallax.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("perlin3D", { 
    "%{wks.location}/assets/shaders/perlin3D/Perlin3D.json"}, {})

scene_project("pixelize", { 
    "%{wks.location}/assets/shaders/pixelize/Pixelize.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("radialBlur", { 
    "%{wks.location}/assets/shaders/radialBlur/RadialBlur.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("sepia", { 
    "%{wks.location}/assets/shaders/sepia/Sepia.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

scene_project("sharpen", { 
    "%{wks.location}/assets/shaders/sharpen/Sharpen.json"}, { 
    "%{wks.location}/examples/textured/include/"
})

--scene_project("dotProduct", { "%{wks.location}/assets/shaders/dotProduct/DotProduct.json"}, {})

--3d projects

scene_project("scene3D", {"%{wks.location}/assets/shaders/scene3D/scene3D.json"}, {})
scene_project("texturedScene3D", {"%{wks.location}/assets/shaders/texturedScene3D/texturedScene3D.json"}, {"%{wks.location}/examples/scene3D/include/"})

scene_project("depthPrePass", 
{"%{wks.location}/assets/shaders/depthPrePass/DepthPrePass.json"}, 
{"%{wks.location}/examples/scene3D/include/", "%{wks.location}/examples/texturedScene3D/include/"})

scene_project("FXAA", 
{"%{wks.location}/assets/shaders/FXAA/FXAA.json"}, 
{"%{wks.location}/examples/scene3D/include/", "%{wks.location}/examples/texturedScene3D/include/"})