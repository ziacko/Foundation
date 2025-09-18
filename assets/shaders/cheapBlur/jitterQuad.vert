#version 450

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;

out defaultBlock
{
	vec4 position;
	vec2 uv;
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
	float		framesPerSecond;
	uint		totalFrames;
};

layout(binding = 1) uniform jitterSettings
{
	vec2 haltonSequence[128];
	float haltonScale;
	float ditheringScale;
	uint numSamples;
};

void main()
{
	float deltaWidth = 1.0 / resolution.x;
	float deltaHeight = 1.0 / resolution.y;

	uint index = totalFrames % numSamples;
	vec2 jitter = vec2(haltonSequence[index].x * deltaWidth, haltonSequence[index].y * deltaHeight) * haltonScale;

	mat4 jitterm = mat4( 1.0 );

	jitterm[2][0] += jitter.x;
	jitterm[2][1] += jitter.y;
	gl_Position = jitterm * position;


	outBlock.position = position;
	outBlock.uv = outBlock.position.xy * 0.5f + 0.5f;
	//outBlock.position.xy += jitter;
	//gl_Position = outBlock.position;//.xy + jitter;
}
