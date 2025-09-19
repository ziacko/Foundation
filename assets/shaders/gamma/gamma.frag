#version 420

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
	mat4		view;
	mat4		translation;
	vec2		resolution;
	vec2		mousePosition;
	float		deltaTime;
	float		totalTime;
	float 		framesPerSecond;
	uint		totalFrames;
};

layout(std140, binding = 1) uniform gammaSettings
{
	vec4		gamma;
};

uniform sampler2D defaultTexture;

void main()
{
	if(gl_FragCoord.x > mousePosition.x)
	{
		outColor = texture(defaultTexture, inBlock.uv);
	}

	else
	{
		outColor = pow( texture(defaultTexture, inBlock.uv), 1.0 / gamma);
	}
}