uniform sampler2D		u_DiffuseMap;
uniform sampler2D		u_GlowMap;
uniform sampler2D		u_ScreenDepthMap;

#define					MAX_VOLUMETRIC_LIGHTS 16//64

uniform int				u_lightCount;
uniform vec2			u_vlightPositions[MAX_VOLUMETRIC_LIGHTS];
uniform vec3			u_vlightColors[MAX_VOLUMETRIC_LIGHTS];

uniform vec4			u_Local0;
uniform vec4			u_ViewInfo; // zmin, zmax, zmax / zmin, SUN_ID

varying vec2			var_TexCoords;

// Debugging...
//#define DEBUG_POSITIONS
//#define DEBUG_POSITIONS_NOLIGHTCOLOR
//#define DEBUG_POSITIONS2

// General options...
#define VOLUMETRIC_THRESHOLD  0.001 //u_Local0.b


#if defined(HQ_VOLUMETRIC)
const float	iBloomraySamples = 32.0;
const float	fBloomrayDecay = 0.96875;
#elif defined (MQ_VOLUMETRIC)
const float	iBloomraySamples = 16.0;
const float	fBloomrayDecay = 0.9375;
#else //!defined(HQ_VOLUMETRIC) && !defined(MQ_VOLUMETRIC)
const float	iBloomraySamples = 8.0;
const float	fBloomrayDecay = 0.875;
#endif //defined(HQ_VOLUMETRIC) && defined(MQ_VOLUMETRIC)

const float	fBloomrayWeight = 0.5;
const float	fBloomrayDensity = 1.0;
const float fBloomrayFalloffRange = 0.4;

float linearize(float depth)
{
	return (1.0 / mix(u_ViewInfo.z, 1.0, depth));
}

void main ( void )
{
#ifndef DUAL_PASS
	vec4 diffuseColor = texture(u_DiffuseMap, var_TexCoords.xy);
#endif //DUAL_PASS
	int  SUN_ID = int(u_ViewInfo.a);

	//gl_FragColor.rgb = texture( u_GlowMap, var_TexCoords ).rgb;
	//gl_FragColor.a = 1.0;
	//return;

	//gl_FragColor.rgb = vec3(linearize(texture( u_ScreenDepthMap, var_TexCoords ).r));
	//gl_FragColor.a = 1.0;
	//return;

	if (u_lightCount <= 0)
	{
#ifdef DUAL_PASS
		gl_FragColor = vec4(0.0);
#else //!DUAL_PASS
		gl_FragColor = diffuseColor;
#endif //DUAL_PASS
		return;
	}

	highp vec2		inRangePositions[MAX_VOLUMETRIC_LIGHTS];
	highp vec3		lightColors[MAX_VOLUMETRIC_LIGHTS];
	highp float		fallOffRanges[MAX_VOLUMETRIC_LIGHTS];
	highp float		lightDepths[MAX_VOLUMETRIC_LIGHTS];
	int			inRangeSunID = -1;
	int			numInRange = 0;

#ifdef DEBUG_POSITIONS
	vec3 debugColor = vec3(0.0);
#endif //DEBUG_POSITIONS

#ifdef DEBUG_POSITIONS2
	vec3 debugColor = vec3(0.0);

	for (int i = 0; i < MAX_VOLUMETRIC_LIGHTS; i++)
	{
		if (i >= u_lightCount) break;

		float dist = length(var_TexCoords - u_vlightPositions[i]);

		if (dist < 0.025)
		{
			debugColor += vec3(1.0, 0.0, 0.0);

			float invDepth = clamp(1.0 - linearize(texture( u_ScreenDepthMap, u_vlightPositions[i] ).r), 0.0, 1.0);
			float fall = clamp((fBloomrayFalloffRange * invDepth) - dist, 0.0, 1.0) * invDepth;

			if (i == SUN_ID) 
			{
				fall = 1.0;
			}

			if (fall > 0.0)
			{
				debugColor += vec3(0.0, 1.0, 0.0);

				vec3 spotColor = u_vlightColors[i];
				
				//if (i == SUN_ID) 
				{
					if (length(texture( u_GlowMap, u_vlightPositions[i] ).rgb) <= VOLUMETRIC_THRESHOLD)
					{// If it doesnt appear on the glow map, then it's not on the screen...
						continue;
					}
				}

				if (length(spotColor) > VOLUMETRIC_THRESHOLD)
				{
					debugColor += vec3(0.0, 0.0, 1.0);
				}
			}
		}
	}

	gl_FragColor.rgb = debugColor.rgb;
	gl_FragColor.a = 1.0;
	return;
#endif //DEBUG_POSITIONS2

	for (int i = 0; i < MAX_VOLUMETRIC_LIGHTS; i++)
	{
		if (i >= u_lightCount) break;

		highp float dist = length(var_TexCoords - u_vlightPositions[i]);
		highp float invDepth = clamp(1.0 - linearize(texture( u_ScreenDepthMap, u_vlightPositions[i] ).r), 0.0, 1.0);
		highp float fall = clamp((fBloomrayFalloffRange * invDepth) - dist, 0.0, 1.0) * invDepth;

		if (i == SUN_ID) 
		{
			//fall *= 4.0;
			fall = 1.0;
		}

		if (fall > 0.0)
		{
			inRangePositions[numInRange] = u_vlightPositions[i];
			lightDepths[numInRange] = 1.0 - invDepth;

			if (i == SUN_ID) 
			{
				fallOffRanges[numInRange] = fall;
			}
			else
			{
				fallOffRanges[numInRange] = (fall + (fall*fall)) / 2.0;
			}

			//vec3 spotColor = texture( u_GlowMap, u_vlightPositions[i] ).rgb;
			highp vec3 spotColor = u_vlightColors[i];

			//if (i == SUN_ID) 
			{
				if (length(texture( u_GlowMap, u_vlightPositions[i] ).rgb) <= VOLUMETRIC_THRESHOLD)
				{// If it doesnt appear on the glow map, then it's not on the screen...
					continue;
				}
			}

			highp float lightColorLength = length(spotColor);

			if (lightColorLength > VOLUMETRIC_THRESHOLD)
			{
				/*
				if (lightColorLength < 3.0)
				{// Try to maximize light strengths...
					float mult = (3.0 / lightColorLength);
					spotColor += spotColor * mult;
				}*/

				spotColor /= lightColorLength;

				lightColors[numInRange] = spotColor;
				lightColors[numInRange] *= 1.5;//u_Local0.a;//0.22;

				if (i == SUN_ID) 
				{
					//lightColors[numInRange] *= 0.005;
					lightColors[numInRange] *= 0.125;
				}

#ifdef DEBUG_POSITIONS
				if (fallOffRanges[numInRange] > 0)
				{// For debugging positions...
#ifdef DEBUG_POSITIONS_NOLIGHTCOLOR
					if (i == SUN_ID)
					{
						debugColor += vec3(1.0, 1.0, 0.0) * fallOffRanges[numInRange];
					}
					else
					{
						debugColor += vec3(0.0, 0.0, 1.0) * fallOffRanges[numInRange];
					}
#else //!DEBUG_POSITIONS_NOLIGHTCOLOR
					debugColor += lightColors[numInRange] * fallOffRanges[numInRange];
#endif //DEBUG_POSITIONS_NOLIGHTCOLOR
				}
#endif //DEBUG_POSITIONS

				numInRange++;
			}
			else
			{// Not bright enough for a volumetric light...
				continue;
			}
		}
	}

#ifdef DEBUG_POSITIONS
	gl_FragColor.rgb = debugColor.rgb;
	gl_FragColor.a = 1.0;
	return;
#endif //DEBUG_POSITIONS

	if (numInRange <= 0)
	{// Nothing in range...
#ifdef DUAL_PASS
		gl_FragColor = vec4(0.0);
#else //!DUAL_PASS
		gl_FragColor = diffuseColor;
#endif //DUAL_PASS
		return;
	}

	highp vec4 totalColor = vec4(0.0, 0.0, 0.0, 0.0);

	for (int i = 0; i < MAX_VOLUMETRIC_LIGHTS; i++) // MAX_VOLUMETRIC_LIGHTS to use constant loop size
	{
		if (i >= numInRange) break;

		highp vec4	lens = vec4(0.0, 0.0, 0.0, 0.0);
		highp vec2	ScreenLightPos = inRangePositions[i];
		highp vec2	texCoord = var_TexCoords;
		highp float	lightDepth = lightDepths[i];
		highp vec2	deltaTexCoord = (texCoord.xy - ScreenLightPos.xy);

		deltaTexCoord *= 1.0 / float(iBloomraySamples * fBloomrayDensity);

		highp float illuminationDecay = 1.0;

		for(int g = 0; g < int(iBloomraySamples); g++)
		{
			texCoord -= deltaTexCoord;

			highp float linDepth = linearize(texture(u_ScreenDepthMap, texCoord.xy).r);

			highp float sample2 = linDepth * illuminationDecay * fBloomrayWeight;

			lens.xyz += (sample2 * 0.5) * lightColors[i];

			illuminationDecay *= fBloomrayDecay;

			if (illuminationDecay <= 0.0)
				break;
		}

		totalColor += clamp((lens * lightDepth) * fallOffRanges[i], 0.0, 1.0);
	}

	//totalColor.rgb *= 100.0;
	totalColor.rgb *= 0.5;//1.5;//u_Local0.b;

#ifdef DUAL_PASS
	totalColor.a = 1.0;
	gl_FragColor = totalColor;
#else //!DUAL_PASS
	totalColor.a = 0.0;
	gl_FragColor = diffuseColor + totalColor;
#endif //DUAL_PASS
}
