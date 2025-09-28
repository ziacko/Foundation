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

const float SRGB_GAMMA = 1.0 / 2.2;
const float SRGB_INVERSE_GAMMA = 2.2;
const float SRGB_ALPHA = 0.055;

// (xchen) gamma to linear sRGB transformation
// Converts a srgb color to a rgb color (approximated, but fast)
vec3 srgb_to_rgb_approx(vec3 srgb) {
    return pow(srgb, vec3(SRGB_INVERSE_GAMMA));
}

// (xchen) rgb to gamma sRGB transformation (approximated, but fast)
vec3 rgb_to_srgb_approx(vec3 rgb) {
	return pow(clamp(rgb, 0.0, 1.0), vec3(SRGB_GAMMA));
}


void main()
{
	if(gl_FragCoord.x > mousePosition.x)
	{
		outColor = texture(defaultTexture, inBlock.uv);
	}

	else
	{
		outColor = vec4(srgb_to_rgb_approx(texture(defaultTexture, inBlock.uv).rgb), 1);

		outColor = outColor * (1.0 / gamma);
		outColor.w = 1;
	}
}