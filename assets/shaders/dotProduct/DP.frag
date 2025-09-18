#version 440

in defaultBlock
{
	vec4 position;
	vec2 uv;
	vec4 color;
	
} inBlock;

out vec4 outColor;

void main()
{
	outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}