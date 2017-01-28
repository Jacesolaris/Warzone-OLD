uniform sampler2D		u_DiffuseMap;
uniform sampler2D		u_GlowMap;
uniform sampler2D		u_ScreenDepthMap;

#define					MAX_VLIGHTS 16

uniform int				u_lightCount;
uniform vec2			u_vlightPositions[MAX_VLIGHTS];

uniform vec4			u_Local0;
uniform vec4			u_ViewInfo; // zmin, zmax, zmax / zmin, SUN_ID

varying vec2			var_TexCoords;

// Debugging...
//#define DEBUG_POSITIONS
//#define DEBUG_POSITIONS_NOLIGHTCOLOR

// General options...
//#define ADJUST_DEPTH_PIXELS // Gives more shadow effect to the lighting, but also can make it too intense...

#define VOLUMETRIC_THRESHOLD  0.001


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

	vec2		inRangePositions[MAX_VLIGHTS];
	vec3		lightColors[MAX_VLIGHTS];
	float		fallOffRanges[MAX_VLIGHTS];
	float		lightDepths[MAX_VLIGHTS];
	int			inRangeSunID = -1;
	int			numInRange = 0;

#ifdef DEBUG_POSITIONS
	vec3 debugColor = vec3(0.0);
#endif //DEBUG_POSITIONS

	for (int i = 0; i < u_lightCount; i++)
	{
		float dist = length(var_TexCoords - u_vlightPositions[i]);
		float invDepth = clamp(1.0 - linearize(texture( u_ScreenDepthMap, u_vlightPositions[i] ).r), 0.0, 1.0);
		float fall = clamp((fBloomrayFalloffRange * invDepth) - dist, 0.0, 1.0) * invDepth;

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

			vec3 spotColor = texture( u_GlowMap, u_vlightPositions[i] ).rgb;

			if (length(spotColor) > VOLUMETRIC_THRESHOLD)
			{
				lightColors[numInRange] = spotColor;
				lightColors[numInRange] *= 0.22;

				if (i == SUN_ID) 
				{
					lightColors[numInRange] *= u_Local0.z;
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

	vec4 totalColor = vec4(0.0, 0.0, 0.0, 0.0);

	for (int i = 0; i < MAX_VLIGHTS/*numInRange*/; i++) // MAX_VLIGHTS to use constant loop size
	{
		if (i >= numInRange) continue;

		vec4	lens = vec4(0.0, 0.0, 0.0, 0.0);
		vec2	ScreenLightPos = inRangePositions[i];
		vec2	texCoord = var_TexCoords;
		float	lightDepth = lightDepths[i];
		vec2	deltaTexCoord = (texCoord.xy - ScreenLightPos.xy);

		deltaTexCoord *= 1.0 / float(iBloomraySamples * fBloomrayDensity);

		float illuminationDecay = 1.0;

		for(int g = 0; g < int(iBloomraySamples); g++)
		{
			texCoord -= deltaTexCoord;

			float linDepth = linearize(texture(u_ScreenDepthMap, texCoord.xy).r);

#ifdef ADJUST_DEPTH_PIXELS
			if (linDepth > lightDepths[i]) linDepth = 1.0;
			//if (linDepth <= lightDepths[i]) linDepth = 0.0;
#endif //ADJUST_DEPTH_PIXELS

			float sample2 = linDepth * illuminationDecay * fBloomrayWeight;

			lens.xyz += (sample2 * 0.5) * lightColors[i];

			illuminationDecay *= fBloomrayDecay;

			if (illuminationDecay <= 0.0)
				break;
		}

		totalColor += clamp((lens * lightDepth) * fallOffRanges[i], 0.0, 1.0);
	}

	totalColor.rgb *= 100.0;

#ifdef DUAL_PASS
	totalColor.a = 1.0;
	gl_FragColor = totalColor;
#else //!DUAL_PASS
	totalColor.a = 0.0;
	gl_FragColor = diffuseColor + totalColor;
#endif //DUAL_PASS
}
