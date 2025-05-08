#version 440

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 UV;

out defaultBlock
{
	vec4 position;
	vec2 uv;
	vec4 color;
} outBlock;

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
	uint		totalFrames;
};

layout(std140, binding = 1) uniform DotNodes 
{
	vec2 position1;
	vec2 position2;
	float scaler;
	bool current;
};

void main()
{
	vec4 temp = position;
	if(current == false) //yes i know branches in shaders are bad. leave me alone
	{
		temp.xy += position1.xy;
	}

	else if(current == true)
	{
		temp.xy += position2.xy;
	}
	
	outBlock.position = projection * view * translation * temp;
	outBlock.uv = outBlock.position.xy * 0.5f + 0.5f;
	gl_Position = outBlock.position;
}