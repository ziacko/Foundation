if os.host() == "linux" then
    local cmake = require "cmake"
    cmake.workspace_directory = _SCRIPT_DIR
    cmake.write_settings = {
        CMAKE_CURRENT_SOURCE_DIR = _SCRIPT_DIR
    }
end

dofile (_SCRIPT_DIR .. "/premakeExtras/templates.lua")
dofile (_SCRIPT_DIR .. "/premakeExtras/helperFunctions.lua")

function scene_project(name, parents)
    project(name)
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"

        toolset "clang"
        debugdir(_SCRIPT_DIR) -- Changed to use workspace location
        local extradir = "./examples/" .. name .. "/"
        local shaderPath = _SCRIPT_DIR .. "/assets/shaders/" .. name .. "/" .. name .. ".json"

        --use the last inheritance as the parent
        if inheritances and #inheritances > 0 then
            checkHeaderFile(name, inheritances[#inheritances])
            --print("parents found for " .. name .. ": " .. table.concat(inheritances, ", "))
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
            "lib/imgui-docking/*.cpp",
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
            "lib/gl/",
            "lib/EGL-Registry/api/",
            "lib/glm/",
            "lib/gli/",
            "lib/stb/",
            "lib/imgui-docking/",
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
        if parents then
            for _, file in ipairs(parents) do
                --surround the project name with the include path
                local inheritPath = "examples/" .. file .. "/include/"
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

        --communal settings for all projects
        filter { "configurations:Debug" }
            defines { "DEBUG" }
            symbols "on"
            optimize "Off"
            targetdir (_SCRIPT_DIR .. "/bin/Debug")

        filter { "configurations:Release" }
            optimize "on"
            symbols "off"
            targetdir (_SCRIPT_DIR .. "/bin/Release")

        filter { "toolset:clang"}
            configurations { "Debug", "Release" }
            buildoptions { "-Wno-missing-template-arg-list-after-template-kw",
                    "-Wno-deprecated-enum-enum-conversion", "-Wno-macro-redefined"}
end

if os.host() == "linux" then
    location "proj/cmake"
    else if os.host() == "windows" then
    location "proj/vs"
    end
end

workspace "Portfolio"
    configurations { "Debug", "Release" }
    filter "system:linux"
        platforms { "Linux" }
    filter "system:windows"
        platforms { "Windows" }
    architecture "x64"

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


--ok now make a new command for "punlishing" a project
-- this will create the project files, shaders, source and header files
--then move the necessary library 

--[[function publish_project(name, parents)
    --first make a new folder for the project under ./pub/<name>
    local projectDir = _SCRIPT_DIR .. "/pub/" .. name
    os.mkdir(projectDir)
    print("Publishing project: " .. name)
    --create the source file directory and copy over the original source file
    printf("Creating source directory for project: %s", name)
    local sourceDir = projectDir .. "/source"
    os.mkdir(sourceDir)
    local sourceFilePath = _SCRIPT_DIR .. "/examples/" .. name .. "/source/" .. name .. ".cpp"
    local sourceFile = io.open(sourceFilePath, "r")
    if not sourceFile then
        print("Source file not found: " .. sourceFilePath)
        return
    end
    local newSourceFilePath = sourceDir .. "/" .. name .. ".cpp"
    local newSourceFile = io.open(newSourceFilePath, "w")
    if not newSourceFile then
        print("Failed to create source file: " .. newSourceFilePath)
        return
    end
    newSourceFile:write(sourceFile:read("*a"))
    newSourceFile:close()
    sourceFile:close()
    printf("Source directory created at: %s", sourceDir)

    --create the header file directory, go through each parent and copy the header files over + add scene.h by default
    --create a sub-function that goes though each parent and copies the header files over
    printf("Creating header directory for project: %s", name)
    local headerDir = projectDir .. "/include"
    os.mkdir(headerDir)
    local sceneHeaderPath = _SCRIPT_DIR .. "/examples/scene/include/scene.h"
    local sceneHeaderFile = io.open(sceneHeaderPath, "r")
    if not sceneHeaderFile then
        print("Scene header file not found: " .. sceneHeaderPath)
        return
    end
    local newSceneHeaderPath = headerDir .. "/scene.h"
    local newSceneHeaderFile = io.open(newSceneHeaderPath, "w")
    
    
    --create the shader directory

    --make a copy of the library folder into ./pub/<name>/lib

    --copy over the shader files
    --how do i copy oveer the model files? just copy them over by hand or copy all model and texture assets?
    --maybe make a new folder for packaged assets under assets?

    --create a new lua premake file from templates 

    --need to account for assets like models and textures

    --use a custom manifest file to store the project information?
      --

end


newaction {
    trigger = "publish",
    description = "Publish a new project",
    --first argument is the project name and anything following it are the parent projects
    --grab every word after project name but no the final word

    execute = function ()
        local args = _OPTIONS["parents"] or "default"
        local inputs = {}
        for i, arg in ipairs(_ARGS) do
            if not arg:match("^%-%-") then -- Ignore flags like --verbose
                table.insert(inputs, arg)
            end
        end

        print("parents: " .. inputs)

        local name = _ACTION
        if not name or name == "" then
            print("Please provide a project name.")
            return
        end
        publish_project(name, inputs)
        print("Project " .. name .. " published successfully.")
    end
}]]--

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
scene_project("textureSettings", {"textured"})
scene_project("mipMapping", {"textured"})
scene_project("parallax", { "textured"})
scene_project("perlin3D", {"perlin"})
scene_project("pixelize", {"textured"})
scene_project("radialBlur", {"textured"})
scene_project("sepia",  {"textured"})
scene_project("sharpen", {"textured"})
scene_project("dotProduct")

--3d projects
scene_project("scene3D")
scene_project("texturedScene3D", {"scene3D"})
scene_project("depthPrePass", {"scene3D", "texturedScene3D"})

--anti aliasing projects
scene_project("FXAA", {"scene3D", "texturedScene3D"})
scene_project("SMAA", {"scene3D", "texturedScene3D"})
scene_project("OAUpsampler", {"scene3D", "texturedScene3D", "SMAA"})
scene_project("MSAA", {"scene3D", "texturedScene3D"})