#version 440

in defaultBlock
{
	vec4 position;
	vec2 uv;
    vec2 fullUV;
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
	float 		framesPerSecond;
	uint		totalFrames;
};

layout(std140, binding = 3) uniform upsaleSettings
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

layout(binding = 4) uniform jitterSettings
{
	vec2 haltonSequence[128];
	float haltonScale;
	uint numSamples;
	uint enableDithering;
	float ditheringScale;
};

layout(binding = 0) uniform sampler2D currentColorTex;
layout(binding = 1) uniform sampler2D previousColorTex;

#define W 0
#define E 1
#define N 2
#define S 3
#define NW 0
#define NE 1
#define SW 2
#define SE 3

#define TEMPORALEDGE_TIME_MIN 0.0000001

float saturate( float x )
{
    return clamp( x, 0.0, 1.0 );
}

vec4 pow3( vec4 x, float y )
{
    return vec4( pow( x.x, y ), pow( x.y, y ), pow( x.z, y ), x.w );
}

float Luma( vec3 c )
{
    return dot( c, vec3( 0.2126, 0.7152, 0.0722 ) );
}

float LumaDiff( vec3 x, vec3 y )
{
    float l1 = dot( x, vec3( 0.2126, 0.7152, 0.0722 ) );
    float l2 = dot( y, vec3( 0.2126, 0.7152, 0.0722 ) );
    return abs( l1 - l2 );
}

float MorphStrengthShaper( float x )
{
    return 1.3 * x - 0.3 * x * x;
}

float SpatialContrast( vec2 spatialLumaMinMax )
{
    float spatialContrast = spatialLumaMinMax.y - spatialLumaMinMax.x;
    return mix( 0.0f, 1.0f, spatialContrast );
}

float TemporalFilterAlpha( float fpsRcp, float convergenceTime )
{
    return exp( -fpsRcp / convergenceTime );
}

vec3 ClampHistory( vec3 history, vec4 currN[4] ) // replace with variance clipping
{
    vec3 cmin = min( min( currN[0].rgb, currN[1].rgb ), min( currN[2].rgb, currN[3].rgb ) );
    vec3 cmax = max( min( currN[0].rgb, currN[1].rgb ), max( currN[2].rgb, currN[3].rgb ) );
    return vec3(
        clamp( history.r, cmin.r, cmax.r ),
        clamp( history.g, cmin.g, cmax.g ),
        clamp( history.b, cmin.b, cmax.b )
    );
}

vec3 BicubicFilter(sampler2D colorTex)
{
    vec2 position = metrics.zw * inBlock.uv;
    vec2 centerPos = floor(position - 0.5) + 0.5;

    vec2 f = position - centerPos; //kernel distance
    vec2 f2 =  f * f;
    vec2 f3 = f2 * f;

    float c = reproSharpness; //assuming 0-1 number scale?
    vec2 w0 = -c * f3 + 2.0 * c * f2 - c * f;
    vec2 w1 = (2.0f - c) * f3 - (3.0 - c) * f2 + 1.0f;
    vec2 w2 = -(2.0f - c) * f3 + (3.0 - 2.0 * c) * f2 + c * f;
    vec2 w3 = c * f3 - c * f2;

    vec2 w12 = w1 + w2;
    vec2 tc12 = metrics.xy * (centerPos + w2 / w12);
    vec3 centerColor = texture(colorTex, tc12).rgb;

    uint index = totalFrames % numSamples;

    vec2 deltaRes = 1.0 / resolution;


    vec2 tc0 = metrics.xy * (centerPos - 1.0); //are these the jitter positions?
    vec2 tc3 = metrics.xy * (centerPos + 2.0); //I'm guess I can't just plugin in current and previous halton index for this
    vec4 color = 
    vec4(texture(colorTex, vec2(tc12.x, tc0.y)).rgb, 1.0f) * (w12.x * w0.y) + 
    vec4(texture(colorTex, vec2(tc0.x, tc12.y)).rgb, 1.0f) * (w0.x * w12.y) + 
    vec4(centerColor, 1.0f) * (w12.x * w12.y) + 
    vec4(texture(colorTex, vec2(tc3.x, tc12.y)).rgb, 1.0f) * (w3.x * w12.y) + 
    vec4(texture(colorTex, vec2(tc12.x, tc3.y)).rgb, 1.0f) * (w12.x * w3.y);

    return color.rgb * (1 / color.a);
}

float TemporalContrast( float currentLuma, float historyLuma )
{
    float x = saturate( abs( historyLuma - currentLuma ) - edgeThreshold );
    float x2 = x * x, x3 = x2 * x;
    return saturate( 3.082671957671837 * x - 3.9384920634917364 * x2 + 1.8518518518516354 * x3 );
}

vec4 FilmicUpsample( vec4 current, vec4 history, vec4 currN[4], vec4 histN[4] )
{
    // Temporal contrast weight.
    float temporalContrastWeight = TemporalContrast( current.a, history.a );

    // Spatial contrast weight.
    vec2 spatialLumaMinMaxC = vec2(
        min( min( currN[0].a, currN[1].a ), min( currN[2].a, currN[3].a ) ),
        max( max( currN[0].a, currN[1].a ), max( currN[2].a, currN[3].a ) )
    );
    vec2 spatialLumaMinMaxH = vec2(
        min( min( histN[0].a, histN[1].a ), min( histN[2].a, histN[3].a ) ),
        max( max( histN[0].a, histN[1].a ), max( histN[2].a, histN[3].a ) )
    );
    float spatialContrastWeightC = SpatialContrast( spatialLumaMinMaxC );
    float spatialContrastWeightH = SpatialContrast( spatialLumaMinMaxH );
    float spatialContrastWeight = abs( spatialContrastWeightC - spatialContrastWeightH );
    
    // Evaluate convergence time from weights.
    float convergenceTime = mix( TEMPORALEDGE_TIME_MIN, timeMax, temporalContrastWeight );
    convergenceTime = mix( convergenceTime, spatialFlickerTime, spatialContrastWeight );
    //float alpha = TemporalFilterAlpha( deltaTime, convergenceTime );
    float alpha = TemporalFilterAlpha( 1.0f / 60.0f, convergenceTime );
    
    // Clamp history to neighbourhood, and apply filmic blend.
    history.rgb = ClampHistory( history.rgb, currN );
    current.rgb = mix( BicubicFilter(currentColorTex), history.rgb, alpha );
    return current;
}


//just call this in main
/*vec4 TAAUpsample(sampler2D current, sampler2D previous)
{
    float m03 = blendingFactor * (0.8 * blendingFactor - 0.8);
    vec3 color = mix(BicubicFilter(current), texture(previous, inBlock.fullUV).rgb, blendingFactor);

    return vec4(color, 1.0f); 
}*/

void main()
{
    vec2 uv =  gl_FragCoord.xy / resolution;
    vec2 scaledUV = uv * resolutionScale;
    // Sample scene and neighbourhood.
    
    vec4 current = clamp( vec4( texture( currentColorTex, inBlock.uv ).rgb, 1.0 ), vec4( 0.0f ), vec4( 1.0f ) );
    vec4 history = clamp( vec4( texture( previousColorTex, inBlock.fullUV ).rgb, 1.0 ), vec4( 0.0f ), vec4( 1.0f ) );
    current.a = Luma( current.rgb ); history.a = Luma( history.rgb );
    
    vec4 currN[4];
    currN[NW] = clamp( texture( currentColorTex, inBlock.uv + 0.6f * vec2( -1.0f,  1.0f ) / resolution.xy ), vec4( 0.0f ), vec4( 1.0f ) );
    currN[NE] = clamp( texture( currentColorTex, inBlock.uv + 0.6f * vec2(  1.0f,  1.0f ) / resolution.xy ), vec4( 0.0f ), vec4( 1.0f ) );
    currN[SW] = clamp( texture( currentColorTex, inBlock.uv + 0.6f * vec2( -1.0f, -1.0f ) / resolution.xy ), vec4( 0.0f ), vec4( 1.0f ) );
    currN[SE] = clamp( texture( currentColorTex, inBlock.uv + 0.6f * vec2(  1.0f, -1.0f ) / resolution.xy ), vec4( 0.0f ), vec4( 1.0f ) );
    currN[NW].a = Luma( currN[NW].rgb );
    currN[NE].a = Luma( currN[NE].rgb );
    currN[SW].a = Luma( currN[SW].rgb );
    currN[SE].a = Luma( currN[SE].rgb );
    
    vec4 histN[4];
    histN[NW] = clamp( texture( previousColorTex, inBlock.fullUV + 0.6f * vec2( -1.0f,  1.0f ) / resolution.xy ), vec4( 0.0f ), vec4( 1.0f ) );
    histN[NE] = clamp( texture( previousColorTex, inBlock.fullUV + 0.6f * vec2(  1.0f,  1.0f ) / resolution.xy ), vec4( 0.0f ), vec4( 1.0f ) );
    histN[SW] = clamp( texture( previousColorTex, inBlock.fullUV + 0.6f * vec2( -1.0f, -1.0f ) / resolution.xy ), vec4( 0.0f ), vec4( 1.0f ) );
    histN[SE] = clamp( texture( previousColorTex, inBlock.fullUV + 0.6f * vec2(  1.0f, -1.0f ) / resolution.xy ), vec4( 0.0f ), vec4( 1.0f ) );
    histN[NW].a = Luma( histN[NW].rgb );
    histN[NE].a = Luma( histN[NE].rgb );
    histN[SW].a = Luma( histN[SW].rgb );
    histN[SE].a = Luma( histN[SE].rgb );

    outColor = FilmicUpsample(current, history, currN, histN );
}