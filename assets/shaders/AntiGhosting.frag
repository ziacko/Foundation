#version 450

in defaultBlock
{
	vec4 position;
	vec2 uv;
	vec2 fullUV;
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

layout(std140, binding = 2) uniform taaSettings
{
	//velocity
	float		velocityScale;
	float 		feedbackFactor;
	float 		maxDepthFalloff;
};

layout(std140, binding = 3) uniform upcsaleSettings
{
    vec4 metrics; //Z and W are output resolution, xy = rcp(zw). calculate rcp on c++ side
    vec2 resolutionScale;
    float blendingFactor;
    float reproSharpness;
    float spatialFlickerTime;
    float timeMax;
    float timeMin;
    float edgeThreshold;
};

out vec4 outColor;

layout(binding = 0) uniform sampler2D topLeft;
layout(binding = 1) uniform sampler2D topRight;
layout(binding = 2) uniform sampler2D bottomRight;
layout(binding = 3) uniform sampler2D bottomLeft;
layout(binding = 4) uniform sampler2D upsampled;
layout(binding = 5) uniform sampler2D depth;
layout(binding = 6) uniform sampler2D velocity;


const vec2 kOffsets3x3[9] =
{
	vec2(-1, -1), //upper left
	vec2( 0, -1), //up
	vec2( 1, -1), //upper right
	vec2(-1,  0), //left
	vec2( 0,  0), // K
	vec2( 1,  0), //right
	vec2(-1,  1), //lower left
	vec2( 0,  1), //down
	vec2( 1,  1), //lower right
}; //k is index 4


// Number of neighbors.
const uint kNeighborsCount = 9;

//we can cut this down to 4
const vec2 kOffsets2x2[5] =
{
	vec2(-1, 0), //left
	vec2(0, -1), //up
	vec2( 0,  0), // K
	vec2(1, 0), //right
	vec2(0, 1) //down
}; //k is index 3

const uint neighborCount = 5;

vec4 FreiChenEdge(sampler2D current)
{
		vec2 texel = vec2(1.0 / resolution.x, 1.0 / resolution.y) / 2;
		//if I were to put all these into imGUI it would be about 81 (9x9) uniforms!
		mat3 G[9] = 
		{
			mat3( 0.3535533845424652, 0, -0.3535533845424652, 0.5, 0, -0.5, 0.3535533845424652, 0, -0.3535533845424652 ),
			mat3( 0.3535533845424652, 0.5, 0.3535533845424652, 0, 0, 0, -0.3535533845424652, -0.5, -0.3535533845424652 ),
			mat3( 0, 0.3535533845424652, -0.5, -0.3535533845424652, 0, 0.3535533845424652, 0.5, -0.3535533845424652, 0 ),
			mat3( 0.5, -0.3535533845424652, 0, -0.3535533845424652, 0, 0.3535533845424652, 0, 0.3535533845424652, -0.5 ),
			mat3( 0, -0.5, 0, 0.5, 0, 0.5, 0, -0.5, 0 ),
			mat3( -0.5, 0, 0.5, 0, 0, 0, 0.5, 0, -0.5 ),
			mat3( 0.1666666716337204, -0.3333333432674408, 0.1666666716337204, -0.3333333432674408, 0.6666666865348816, -0.3333333432674408, 0.1666666716337204, -0.3333333432674408, 0.1666666716337204 ),
			mat3( -0.3333333432674408, 0.1666666716337204, -0.3333333432674408, 0.1666666716337204, 0.6666666865348816, 0.1666666716337204, -0.3333333432674408, 0.1666666716337204, -0.3333333432674408 ),
			mat3( 0.3333333432674408, 0.3333333432674408, 0.3333333432674408, 0.3333333432674408, 0.3333333432674408, 0.3333333432674408, 0.3333333432674408, 0.3333333432674408, 0.3333333432674408 ),
		};

		mat3 I;
		float cnv[9];
		vec3 tex;

		for(float i = 0.0; i < 3.0; i++)
		{
			for(float j = 0.0; j < 3.0; j++)
			{
				tex = texture(current, inBlock.uv + texel * vec2(i - 1.0, j - 1.0)).rgb;
				I[int(i)][int(j)] = length(tex);
			}
		}

		for(int i = 0; i < 9; i++)
		{
			float dp3 = dot(G[i][0], I[0]) + dot(G[i][1], I[1]) + dot(G[i][2], I[2]) * edgeThreshold;
			cnv[i] = dp3 * dp3;
		}

		float M = (cnv[0] + cnv[1]) + (cnv[2] + cnv[3]);
		float S = (cnv[4] + cnv[5]) + (cnv[6] + cnv[7]) + (cnv[8] + M);

		return vec4(vec3(sqrt(M/S)), 1);
}



void main()
{
    vec2 deltaRes = vec2(1.0f / resolution.x, 1.0f / resolution.y);
    vec4 col = vec4(1, 0, 0, 1);

    float blarg = length(FreiChenEdge(upsampled).xyz);
	col = texture(upsampled, inBlock.uv);   
    if((totalFrames % 4) == 0) //top left
    {
       // blarg = length(FreiChenEdge(topLeft).xyz);
		col = texture(topLeft, inBlock.uv);          
    }

    else if((totalFrames % 4) == 1) //top right
    {
       // blarg = length(FreiChenEdge(topRight).xyz);
		col = texture(topRight, inBlock.uv);
    }

    else if((totalFrames % 4) == 2) //bottom right
    {
       // blarg = length(FreiChenEdge(bottomRight).xyz);
		col = texture(bottomRight, inBlock.uv);
    }

    else if((totalFrames % 4) == 3) //bottom left
    {
        //blarg = length(FreiChenEdge(bottomLeft).xyz);
		col = texture(bottomLeft, inBlock.uv);
    }

	if(blarg < maxDepthFalloff)
	{
		outColor = texture(upsampled, inBlock.uv);
	}

	else
	{
		outColor = col;//texture2D(currentColor, inBlock.uv);
	}
}