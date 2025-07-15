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
	noperspective vec4 position;
	noperspective vec2 uv;
} outBlock;

out edgeBlock
{
    noperspective vec4 offset[3];
} outEdge;

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


float snapToZeroOrOne(float value, float threshold) {
    if (value < threshold) return 0.0;
    if (value > 1.0 - threshold) return 1.0;
    return value;
}

vec4 snapToZeroOrOne(vec4 value, float threshold) {
    return vec4(
        snapToZeroOrOne(value.x, threshold),
        snapToZeroOrOne(value.y, threshold),
        snapToZeroOrOne(value.z, threshold),
		snapToZeroOrOne(value.w, threshold)
    );
}

/**
 * Edge Detection Vertex Shader
 */
void SMAAEdgeDetectionVS(float2 texcoord, out float4 offset[3])
{
    offset[0] = mad(rtMetrics.xyxy, float4(-1.0, 0.0, 0.0, -1.0), texcoord.xyxy);
    offset[1] = mad(rtMetrics.xyxy, float4( 1.0, 0.0, 0.0,  1.0), texcoord.xyxy);
    offset[2] = mad(rtMetrics.xyxy, float4(-2.0, 0.0, 0.0, -2.0), texcoord.xyxy);

	//offset[0] = snapToZeroOrOne(offset[0], 0.5);
	//offset[1] = snapToZeroOrOne(offset[1], 0.5);
	//offset[2] = snapToZeroOrOne(offset[2], 0.5);
	
}

void main()
{
	outBlock.position = position;
	outBlock.uv = outBlock.position.xy * 0.5 + 0.5;

	//outBlock.uv.y = 1.0 - outBlock.uv.y; // Flip Y coordinate for correct texture sampling
	SMAAEdgeDetectionVS(outBlock.uv, outEdge.offset);

	gl_Position = outBlock.position;
}