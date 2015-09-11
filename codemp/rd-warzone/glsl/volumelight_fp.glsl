uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_GlowMap;
uniform sampler2D	u_ScreenDepthMap;

uniform int			u_lightCount;
uniform vec2		u_lightPositions[16];

varying vec2		var_TexCoords;
varying vec2		var_Dimensions;

uniform vec2		u_Dimensions;
uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin

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
const float fBloomrayFalloffRange = 0.2;

float linearize(float depth)
{
	return (1.0 / mix(u_ViewInfo.z, 1.0, depth));
}

void main ( void )
{
	vec4 diffuseColor = texture2D(u_DiffuseMap, var_TexCoords.xy);

	if (u_lightCount <= 0)
	{
#ifdef DUAL_PASS
		gl_FragColor = vec4(0.0);
#else //!DUAL_PASS
		gl_FragColor = diffuseColor;
#endif //DUAL_PASS
		return;
	}

	vec2		inRangePositions[16];
	float		fallOffRanges[16];
	float		lightDepths[16];
	int			numInRange = 0;

	for (int i = 0; i < u_lightCount; i++)
	{
		float dist = length(var_TexCoords - u_lightPositions[i]);
		float depth = 1.0 - linearize(texture2D(u_ScreenDepthMap, u_lightPositions[i]).r);
		float fall = clamp((fBloomrayFalloffRange * depth * 2.0) - dist, 0.0, 1.0);

		if (fall > 0.0)
		{
			inRangePositions[numInRange] = u_lightPositions[i];
			lightDepths[numInRange] = depth;
			fallOffRanges[numInRange] = (fall + (fall*fall)) / 2.0;
			numInRange++;
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

		deltaTexCoord *= 1.0 / float(iBloomraySamples * fBloomrayDensity);

		float illuminationDecay = 1.0;

		for(int g = 0; g < iBloomraySamples; g++) {
			texCoord -= deltaTexCoord;
			vec4 sample2 = texture2D(u_DiffuseMap, texCoord.xy);
			//sample2.rgb = vec3(linearize(sample2.x));
			sample2.w = 1.0;
			sample2 *= illuminationDecay * fBloomrayWeight;

			lens.xyz += sample2.xyz*sample2.w;
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
