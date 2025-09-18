#version 450

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;
layout (location = 6) in vec2 uv;

out defaultBlock
{
	vec4 		position;
	vec4 		normal;
	vec2		uv;
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
	uint 		totalFrames;
};

layout(std140, binding = 2) uniform homogenousSettings
{
    float test;
};

void main()
{
	//move from world space to screen space
	mat4 mvp = projection * view * translation;
    vec4 pos = vec4(position.x, position.y, position.z, test);

	gl_Position = mvp * pos;

	outBlock.uv = uv;
	outBlock.normal = normal;
}