#version 440

in defaultBlock
{
	vec4 position;
	vec2 uv;
	vec2 fullUV;
} inBlock;

out vec4 outColor;

layout(binding = 0) uniform sampler2D defaultTexture;

layout(std140) uniform mipSettings
{
    float		mipLevel;
};

void main()
{
    outColor = textureLod(defaultTexture, inBlock.uv, mipLevel);
}