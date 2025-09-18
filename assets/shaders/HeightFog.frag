#version 440

in defaultBlock
{
	vec4 position;
	vec2 uv;
	vec2 fullUV;
} inBlock;

out vec4 outColor;

layout(std140, binding = 0) uniform defaultSettings
{
	mat4		projection;
	mat4 		view;
	mat4 		translation;
	vec2		resolution;
	vec2		mousePosition;
	float		deltaTime;
	float		totalTime;
	float 		framesPerSecond;
	uint 		totalFrames;
};

uniform sampler2D diffuse;

//for distanceToPoint we can read from the depth buffer (convert it back to linear depth if using perspective projection)
//cameraToPoint being the (normalized?) direction from view[3] to point
//make any arbitrary sun direction we want

/*
vec3 fog5(in vec3 fogColor, in float distanceToPoint, in vec3 cameraToPoint, in vec3 sunDirection)
{

	//float fogAmount = 1.0 - exp(-distanceToPoint * )

	return vec3(0);

}*/

void main()
{
	outColor = texture2D(diffuse, inBlock.uv);
}