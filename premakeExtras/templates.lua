headerTemplate = [[

#pragma once

#include <<parent>.h>

class <name>Scene : public <parent>Scene
{


};]]


sourceTemplate = [[
#include "<name>.h"

int main()
{
	<name>Scene exampleScene = <name>Scene();
	exampleScene.Initialize();
	exampleScene.Run();

	return 0;
}
]]

shaderConfigTemplate = [[
[
    {
        "name": "<name>",
        "outputs": [
            "outColor"
        ],
        "shaders": [
            {
                "name": "defaultVertex",
                "path": "default.vert",
                "type": "vertex"
            },
            {
                "name": "defaultFragment",
                "path": "default.frag",
                "type": "fragment"
            }
        ],
        "vertex attributes": [
            "position",
            "UV"
        ]
    }
]
]]

vertexShaderTemplate = [[
#version 440

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;

out defaultBlock
{
	vec4 position;
	vec2 uv;
	vec2 fullUV;
} outBlock;

layout(std140, binding = 0) uniform defaultSettings
{
	mat4		projection;
	mat4		view;
	mat4		translation;
	vec2		resolution;
	vec2		mousePosition;
	float		deltaTime;
	float		totalTime;
	float 		framesPerSecond;
	uint		totalFrames;
};

void main()
{
	outBlock.position = projection * view * translation * position;
	outBlock.uv = outBlock.position.xy * 0.5f + 0.5f;
	gl_Position = outBlock.position;
}
]]

fragmentShaderTemplate = [[
#version 440

in defaultBlock
{
	vec4 position;
	vec2 uv;
	vec2 fullUV;
} inBlock;

layout(std140, binding = 0) uniform defaultSettings
{
	mat4		projection;
	mat4		view;
	mat4		translation;
	vec2		resolution;
	vec2		mousePosition;
	float		deltaTime;
	float		totalTime;
	float 		framesPerSecond;
	uint		totalFrames;
};

out vec4 outColor;

void main()
{
	outColor = vec4(0.25f, 0.25f, 0.0f, 1.0f);
}
]]