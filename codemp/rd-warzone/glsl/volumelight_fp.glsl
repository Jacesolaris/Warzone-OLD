uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_DeluxeMap;
uniform sampler2D	u_ScreenDepthMap;

uniform int			u_lightCount;
uniform vec2		u_lightPositions[16];
uniform float		u_lightDistances[16];
uniform vec3		u_lightColors[16];

varying vec2		var_TexCoords;
varying vec2		var_Dimensions;

uniform vec2		u_Dimensions;
uniform vec4		u_Local0;
uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin, SUN_ID

#define VOLUMETRIC_THRESHOLD 0.15

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
const float fBloomrayFalloffRange = 0.4;//0.5;//0.4;

float linearize(float depth)
{
	return (1.0 / mix(u_ViewInfo.z, 1.0, depth));
}

void main ( void )
{
	vec4 diffuseColor = texture2D(u_DiffuseMap, var_TexCoords.xy);
	int  SUN_ID = int(u_ViewInfo.a);

	//gl_FragColor = vec4(texture2D(u_DeluxeMap, vec2(var_TexCoords.x, 1.0 - var_TexCoords.y)).rgb, 1.0);
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

	//float depth = 1.0 - linearize(texture2D(u_ScreenDepthMap, var_TexCoords).r);
	//gl_FragColor = vec4(depth, depth, depth, 1.0);
	//return;

	vec2		inRangePositions[16];
	vec3		lightColors[16];
	float		fallOffRanges[16];
	float		lightDepths[16];
	int			numInRange = 0;

	for (int i = 0; i < u_lightCount; i++)
	{
		float dist = length(var_TexCoords - u_lightPositions[i]);
		//float depth = 1.0 - linearize(texture2D(u_ScreenDepthMap, u_lightPositions[i]).r);
		float depth = 1.0 - u_lightDistances[i];
		float fall = clamp((fBloomrayFalloffRange * depth) - dist, 0.0, 1.0) * depth;

		if (i == SUN_ID) fall *= 4.0;

		if (fall > 0.0)
		{
			inRangePositions[numInRange] = u_lightPositions[i];
			lightDepths[numInRange] = depth;
			fallOffRanges[numInRange] = (fall + (fall*fall)) / 2.0;
			lightColors[numInRange] = u_lightColors[i];

			if (lightColors[numInRange].r < 0.0 && lightColors[numInRange].g < 0.0 && lightColors[numInRange].b < 0.0)
			{
				vec3 spotColor = texture2D(u_DeluxeMap, vec2(inRangePositions[numInRange].x, 1.0 - inRangePositions[numInRange].y)).rgb;

				if (length(spotColor) > VOLUMETRIC_THRESHOLD)
				{
					if (length(spotColor) <= 1.0)
					{
						spotColor *= 4.0;
					}
					else if (length(spotColor) <= 2.0)
					{
						spotColor *= 2.5;
					}

					lightColors[numInRange] = -(spotColor + 0.5); // + 0.5 to make sure all .rgb are negative...
				}
				else
				{// Not bright enough for a volumetric light...
					continue;
				}
			}

			if (length(lightColors[numInRange]) > VOLUMETRIC_THRESHOLD)
			{// Only use it if it is not a dark pixel...
				numInRange++;
			}
		}
	}

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

	for (int i = 0; i < numInRange; i++)
	{
		vec4	lens = vec4(0.0, 0.0, 0.0, 0.0);
		vec2	ScreenLightPos = inRangePositions[i];
		vec2	texCoord = var_TexCoords;
		float	lightDepth = lightDepths[i];
		vec2	deltaTexCoord = (texCoord.xy - ScreenLightPos.xy);
		//int		samples = int(float(iBloomraySamples) * lightDepth);

		//deltaTexCoord *= 1.0 / float(samples * fBloomrayDensity);
		deltaTexCoord *= 1.0 / float(iBloomraySamples * fBloomrayDensity);

		float illuminationDecay = 1.0;

		//for(int g = 0; g < int(samples); g++)
		for(int g = 0; g < int(iBloomraySamples); g++)
		{
			texCoord -= deltaTexCoord;

			float linDepth = linearize(texture2D(u_ScreenDepthMap, texCoord.xy).r);
			float sample2 = linDepth * illuminationDecay * fBloomrayWeight;

			if (lightColors[i].r < 0.0 && lightColors[i].g < 0.0 && lightColors[i].b < 0.0)
			{// This is a map glow. Since we have no color value, look at the pixel...
				lens.xyz += (sample2 * 0.5) * -(lightColors[i] + 0.5) * 1.5;//u_Local0.r; // + 0.5 to undo + 0.5 above...
			}
			else
			{
				lens.xyz += (sample2 * 0.5) * lightColors[i];
			}

			illuminationDecay *= fBloomrayDecay;

			if (illuminationDecay <= 0.0)
				break;
		}

		totalColor += clamp((lens * lightDepth) * fallOffRanges[i], 0.0, 1.0);
	}

	totalColor.a = 0.0;

#ifdef DUAL_PASS
	gl_FragColor = totalColor;
#else //!DUAL_PASS
	gl_FragColor = diffuseColor + totalColor;
#endif //DUAL_PASS
}
