#version 420

in defaultBlock
{
	vec4 position;
	vec2 uv;
	vec2 fullUV;
} inBlock;

out vec4 outColor;

layout(std140, binding = 1) uniform chromaticSettings
{
	float redOffset;
	float greenOffset;
	float blueOffset;
};

uniform sampler2D defaultTexture;

void main()
{
	
	vec4 redValue = texture(defaultTexture, inBlock.uv - redOffset);
	vec4 greenValue = texture(defaultTexture, inBlock.uv - greenOffset);
	vec4 blueValue = texture(defaultTexture, inBlock.uv - blueOffset);

	outColor = vec4( redValue.r, greenValue.g, blueValue.b, 1.0f);
}