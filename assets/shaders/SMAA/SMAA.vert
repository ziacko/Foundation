#version 450
#define SMAATexture2D(tex) sampler2D tex
#define SMAATexturePass2D(tex) tex
#define SMAASampleLevelZero(tex, coord) textureLod(tex, coord, 0.0)
#define SMAASampleLevelZeroPoint(tex, coord) textureLod(tex, coord, 0.0)
#define SMAASampleLevelZeroOffset(tex, coord, offset) textureLodOffset(tex, coord, 0.0, offset)
#define SMAASample(tex, coord) texture(tex, coord)
#define SMAASamplePoint(tex, coord) texture(tex, coord)
#define SMAASampleOffset(tex, coord, offset) texture(tex, coord, offset)
#define SMAA_FLATTEN
#define SMAA_BRANCH
#define lerp(a, b, t) mix(a, b, t)
#define saturate(a) clamp(a, 0.0, 1.0)
#define mad(a, b, c) fma(a, b, c)
#define SMAAGather(tex, coord) textureGather(tex, coord)
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define bool2 bvec2
#define bool3 bvec3
#define bool4 bvec4

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;

out defaultBlock
{
	vec4 position;
	vec2 uv;
	//vec2 flippedUV;
} outBlock;

out blendBlock
{
	vec4 offset;
} outBlend;

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

layout(std140, binding = 1) uniform SMAASettings
{
	vec4 		rtMetrics;
	float		inThreshold;
	float		contrastAdaptationFactor;
	uint		maxSearchSteps;
	uint		maxSearchStepsDiag;
	uint		cornerRounding;
};

/**
 * Neighborhood Blending Vertex Shader
 */
void SMAANeighborhoodBlendingVS(float2 texcoord, out float4 offset)
{
    offset = mad(rtMetrics.xyxy, float4( 1.0, 0.0, 0.0,  1.0), texcoord.xyxy);
}

void main()
{
	outBlock.position = position;
	outBlock.uv = outBlock.position.xy * 0.5 + 0.5;
	//outBlock.flippedUV = outBlock.uv; // Flip Y coordinate for correct texture sampling
	//outBlock.flippedUV.y = 1.0 - outBlock.flippedUV.y;
	SMAANeighborhoodBlendingVS(outBlock.uv, outBlend.offset);
	gl_Position = outBlock.position;
}