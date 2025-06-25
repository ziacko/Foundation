#version 420
#define SMAA_DECODE_VELOCITY(sample) sample.rg
#define SMAA_REPROJECTION_WEIGHT_SCALE 80.0

in defaultBlock
{
	vec4 position;
	vec2 uv;
} inBlock;


in blendBlock
{
	vec4 offset;
} inBlend;

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

/*
layout(std140, binding = 1) uniform sobelSettings
{
	float		redModifier;
	float		greenModifier;
	float		blueModifier;
	float		cellDistance;
};*/

layout(binding = 0) uniform sampler2D colorTex;
layout(binding = 1) uniform sampler2D blendTex;

/**
 * Conditional move:
 */
void SMAAMovc(bvec2 cond, inout vec2 variable, vec2 value) {
	if (cond.x) variable.x = value.x;
	if (cond.y) variable.y = value.y;
}

void SMAAMovc(bvec4 cond, inout vec4 variable, vec4 value) {
	SMAAMovc(cond.xy, variable.xy, value.xy);
	SMAAMovc(cond.zw, variable.zw, value.zw);
}

vec2 deltaResolution = vec2(1.0 / resolution.x, 1.0 / resolution.y );

//-----------------------------------------------------------------------------
// Neighborhood Blending Pixel Shader (Third Pass)

vec4 SMAANeighborhoodBlendingPS(vec2 texcoord, vec4 offset,
    sampler2D colorTex, sampler2D blendTex) 
{
    // Fetch the blending weights for current pixel:
    vec4 a;
    a.x = texture(blendTex, offset.xy).a; // Right
    a.y = texture(blendTex, offset.zw).g; // Top
    a.wz = texture(blendTex, texcoord).xz; // Bottom / Left

    // Is there any blending weight with a value greater than 0.0?
    if (dot(a, vec4(1.0, 1.0, 1.0, 1.0)) < 1e-5) 
    {
        vec4 color = textureLod(colorTex, texcoord, 0.0);
        return color;
    } 
    else 
    {
        bool h = max(a.x, a.z) > max(a.y, a.w); // max(horizontal) > max(vertical)

        // Calculate the blending offsets:
        vec4 blendingOffset = vec4(0.0, a.y, 0.0, a.w);
        vec2 blendingWeight = a.yw;
        SMAAMovc(bvec4(h, h, h, h), blendingOffset, vec4(a.x, 0.0, a.z, 0.0));
        SMAAMovc(bvec2(h, h), blendingWeight, a.xz);
        blendingWeight /= dot(blendingWeight, vec2(1.0, 1.0));

        // Calculate the texture coordinates:
        vec4 blendingCoord = fma(blendingOffset, vec4(deltaResolution.xy, -deltaResolution.xy), texcoord.xyxy);

        // We exploit bilinear filtering to mix current pixel with the chosen
        // neighbor:
        vec4 color = blendingWeight.x * textureLod(colorTex, blendingCoord.xy, 0.0);
        color += blendingWeight.y * textureLod(colorTex, blendingCoord.zw, 0.0);

        return color;
    }
}

void main()
{
    outColor = SMAANeighborhoodBlendingPS(inBlock.uv, inBlend.offset,
        colorTex, blendTex);
}