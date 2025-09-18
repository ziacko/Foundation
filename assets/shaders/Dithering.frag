	#version 440

in defaultBlock
{
	vec4 position;
	vec2 uv;
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
	float		framesPerSecond;
	uint		totalFrames;
};

layout(binding = 1) uniform jitterSettings
{
	vec2 haltonSequence[128];
	float haltonScale;
	float ditheringScale;
    uint numSamples;
};

float OffsetX[4]; //use quincux set here. couldn't get halton to work here
float OffsetY[4];


layout(binding = 0) uniform sampler2D defaultTexture;

void main()
{

    float deltaWidth = 1.0 / resolution.x;
	float deltaHeight = 1.0 / resolution.y;
    OffsetX = float[4](0.25f, 0.75f, 0.5f, 0.0f);
	OffsetY = float[4](0.0f, 0.25f, 0.75f, 0.5f);

	//dithering?
	uint index = totalFrames % 4;
    float randomX = fract(dot(gl_FragCoord.xy, vec2(0.375, -0.125)) + (OffsetY[index] * deltaWidth));
    float randomY = fract(dot(vec2(-0.125,  0.375), gl_FragCoord.xy)) - (OffsetX[index] / deltaHeight);
	float noise = fract(fract(randomX + randomY) + (totalTime * ditheringScale));

    vec4 col = texture(defaultTexture, inBlock.uv);

	if(length(col.rgb) < noise)
	{
		// (xchen) disable ToD
		discard;
	}

    outColor = col;
}