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

/*layout(std140, binding = 1) uniform dynResSettings
{
	vec2		resScale;
};*/

layout(std140, binding = 5) uniform dynResSettings
{
	vec2		resScale;
};

void main()
{
	outBlock.position = projection * view * translation * position;
	outBlock.uv = outBlock.position.xy * 0.5f + 0.5f;
	outBlock.fullUV = uv;
	outBlock.uv *= resScale;

	gl_Position = outBlock.position;
}