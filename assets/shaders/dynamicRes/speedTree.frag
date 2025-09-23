#version 450

in defaultBlock
{
	vec4 		position;
	vec4 		normal;
	vec2		uv;
} inBlock;

layout(location = 0) out vec4 outColor;

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

layout(binding = 0) uniform sampler2D diffuse;

// (xchen) gamma to linear sRGB transformation
const float SRGB_GAMMA = 1.0 / 2.2;
const float SRGB_INVERSE_GAMMA = 2.2;
const float SRGB_ALPHA = 0.055;

// (xchen) gamma to linear sRGB transformation
// Converts a srgb color to a rgb color (approximated, but fast)
vec3 srgb_to_rgb_approx(vec3 srgb) {
    return pow(srgb, vec3(SRGB_INVERSE_GAMMA));
}

vec3 linearToSRGB(vec3 linear) {
    return pow(clamp(linear, 0.0, 1.0), vec3(0.454545)); // 1.0 / 2.2 ≈ 0.454545
}

vec3 srgbToLinear(vec3 srgb) {
    return pow(srgb, vec3(2.2));
}

void main()
{
	//clamp the alpha down hard. if alpha is less than 0.1, clamp it to 0
	vec4 col =  texture(diffuse, inBlock.uv);

	// (xchen) gamma to linear sRGB transformation
	outColor.xyz = col.xyz;// srgbToLinear(col.xyz);
	outColor.a = col.a;
}