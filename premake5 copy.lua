if os.host() == "linux" then
    require "cmake"
end

workspace "Portfolio"
    architecture "x64"
    configurations { "Debug", "Release" }
    location (_SCRIPT_DIR) -- Set the workspace location to the script directory

BaseSceneProject = {}

function BaseSceneProject:new(o, name)
    o = o or {}
    o.name = name or "scene"
    setmetatable(o, self)
    self.__index = self
    o:setup()
    return o
end

function BaseSceneProject:GetExtraFiles()
    return {}
end

function BaseSceneProject:configure()
    -- Default implementation for configuring the project
    -- Override this method in derived classes if needed
end

function BaseSceneProject:setup()
    -- Build the project
    project(self.name)
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"
        targetdir(_SCRIPT_DIR .. "/examples/" .. self.name .. "/bin/")
        toolset "clang"
        
        local extradir = "./examples/" .. self.name .. "/"
        files {
            "examples/scene/include/**.h",
            --"examples/scene/source/**.cpp",
            "examples/" .. self.name .. "/include/**.h",
            "examples/" .. self.name .. "/source/**.cpp",
            "include/Globals.h",
            "lib/imgui/*.cpp",
            "lib/yyjson/src/yyjson.c"
        }

        for _, file in ipairs(self:GetExtraFiles()) do
            files { file }
        end

        includedirs {
            "include/",
            "examples/scene/include/",
            "examples/" .. self.name .. "/include/",
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
            "lib/yyjson/src/"
        }

        self:configure()
        filter { "system:windows" }
            toolset "clang"
            systemversion "latest"
            links { "opengl32.lib" }
        filter { "system:linux" }
            toolset "clang"
            links { "GL", "X11", "Xrandr", "Xinerama" }
end

SceneProject = {}
setmetatable(SceneProject, {__index = BaseSceneProject})

function SceneProject:new(o, name)
    o = o or {}
    o.name = name
    setmetatable(o, self)  -- Set metatable to SceneProject, not BaseSceneProject
    self.__index = self
    o:setup()
    return o
end

function SceneProject:GetExtraFiles()
    return {
        "examples/scene/source/**.cpp"
    }
end

TexturedProject = {}
setmetatable(TexturedProject, {__index = SceneProject})

function TexturedProject:new(o, name)
    o = o or {}
    o.name = name
    setmetatable(o, self)  -- Set metatable to TexturedProject
    self.__index = self
    o:setup()
    return o
end

function TexturedProject:GetExtraFiles()
    return {
        "%{wks.location}/assets/shaders/textured/Textured.json"
    }
end


PerlinNoiseProject = {}
setmetatable(PerlinNoiseProject, {__index = SceneProject})

function PerlinNoiseProject:new(o, name)
    o = o or {}
    o.name = name
    setmetatable(o, self)  -- Set metatable to PerlinNoiseProject
    self.__index = self
    o:setup()
    return o
end

function PerlinNoiseProject:GetExtraFiles()
    return {
        "%{wks.location}/assets/shaders/perlin/PerlinNoise.json"
    }
end

BindlessProject = {}
setmetatable(BindlessProject, {__index = TexturedProject})  -- Inherit from TexturedProject

function BindlessProject:new(o, name)
    o = o or {}
    o.name = name
    setmetatable(o, self)  -- Set metatable to BindlessProject
    self.__index = self
    o:setup()
    return o
end

function BindlessProject:GetExtraFiles()
    local texturedFiles = TexturedProject.GetExtraFiles(self)  -- Get parent's files
    local bindlessFiles = {
        "%{wks.location}/assets/shaders/bindless/Bindless.json"
    }
    for _, file in ipairs(texturedFiles) do
        table.insert(bindlessFiles, file)
    end
    return bindlessFiles
end

--2d projects
sceneProj = SceneProject:new(nil, "scene")
texturedProj = TexturedProject:new(nil, "textured")
perlinProj = PerlinNoiseProject:new(nil, "perlinNoise")
bindlessProj = BindlessProject:new(nil, "bindless")


--3d projects

