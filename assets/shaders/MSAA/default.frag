#version 450

in defaultBlock
{
	vec4 position;
	vec2 uv;
} inBlock;

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

out vec4 outColor;

// We are sampling from a multisampled texture produced by the geometry pass
layout(binding = 0) uniform sampler2DMS defaultTexture;

void main()
{
    ivec2 texel = ivec2(gl_FragCoord.xy);
    int samples = textureSamples(defaultTexture);
    vec4 accum = vec4(0.0);
    for (int i = 0; i < samples; ++i)
    {
        accum += texelFetch(defaultTexture, texel, i);
    }
    outColor = accum / float(max(samples, 1));
}
