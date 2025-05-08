local RD = path.getabsolute("./") .. "/"

includedirs(RD .. "Include/")
includedirs(RD .. "Dependencies")
includedirs(RD .. "Examples/dependencies")

intermediateDir = RD .. "Intermediate"

local sources = {
    RD .. "Example/Example.cpp"
}
filter {"system:linux"}
    links { "GL", "X11", "Xrandr", "Xinerama" }

solution "TinyWindow"
    configurations { "Debug", "Release" }
    filter "system:linux"
        platforms { "Linux" }
    filter "system:windows"
        platforms { "Windows" }
    architecture "x64"

    project "Example"
        kind "ConsoleApp"
        language "C++"
        debugdir "./Example"
        files { sources }
        cppdialect "C++20"
        toolset "clang"
        
    filter {"configurations:Debug"}
    defines {"DEBUG"}
    symbols "On"
    targetname "Example_Debug"
    optimize "Off"
    targetdir "bin/Debug"

    filter {"configurations:Release"}
    defines {}
    symbols "off"
    optimize "on"
    targetname "Example"
    targetdir "bin/Release"


    filter {"platforms:Win64"}
    system "Windows"
    filter {"platforms:Linux"}
    system "Linux"