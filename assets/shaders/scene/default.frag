#version 440

in defaultBlock
{
	vec4 position;
	vec2 uv;
	vec2 fullUV;
} inBlock;

layout(std140) uniform defaultSettings
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

uniform testSettings
{
    int blah;
    float blah2;
    double blah3;
};

buffer GOLStatus
{
    int Status[100];
};

out vec4 outColor;

uniform vec3 peepeepoopoo;

void main()
{
	outColor = vec4(deltaTime * 50, 0.0f, 0.0f, 1.0f);
}