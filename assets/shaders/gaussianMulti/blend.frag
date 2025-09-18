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

layout(binding = 0) uniform sampler2D vertical;
layout(binding = 1) uniform sampler2D horizontal;

void main()
{
	outColor = mix(texture(vertical, inBlock.uv), texture(horizontal, inBlock.uv), 0.5);
}