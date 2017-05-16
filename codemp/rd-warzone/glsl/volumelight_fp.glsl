uniform sampler2D			u_DiffuseMap;
uniform sampler2D			u_GlowMap;
uniform sampler2D			u_SpecularMap;
uniform sampler2D			u_ScreenDepthMap;

#define						MAX_VOLUMETRIC_LIGHTS 16

uniform int					u_lightCount;
uniform vec2				u_vlightPositions[MAX_VOLUMETRIC_LIGHTS];
uniform vec3				u_vlightColors[MAX_VOLUMETRIC_LIGHTS];

uniform vec4				u_Local0;
uniform vec4				u_ViewInfo; // zmin, zmax, zmax / zmin, SUN_ID

flat varying vec4			var_LightColor[MAX_VOLUMETRIC_LIGHTS];
varying vec2				var_TexCoords;

// General options...
#define VOLUMETRIC_STRENGTH		u_Local0.b


#if defined(HQ_VOLUMETRIC)
const int	iBloomraySamples = 32;
const float	fBloomrayDecay = 0.96875;
#elif defined (MQ_VOLUMETRIC)
const int	iBloomraySamples = 16;
const float	fBloomrayDecay = 0.9375;
#else //!defined(HQ_VOLUMETRIC) && !defined(MQ_VOLUMETRIC)
const int	iBloomraySamples = 8;
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
	int  SUN_ID = int(u_ViewInfo.a);

	if (u_lightCount <= 0)
	{
		gl_FragColor = vec4(0.0);
		return;
	}

	vec2			inRangePositions[MAX_VOLUMETRIC_LIGHTS];
	vec3			lightColors[MAX_VOLUMETRIC_LIGHTS];
	float			fallOffRanges[MAX_VOLUMETRIC_LIGHTS];
	float			lightDepths[MAX_VOLUMETRIC_LIGHTS];
	bool			isSun[MAX_VOLUMETRIC_LIGHTS];
	int				inRangeSunID = -1;
	int				numInRange = 0;

	for (int i = 0; i < MAX_VOLUMETRIC_LIGHTS; i++)
	{
		if (i >= u_lightCount) break;

		if (var_LightColor[i].a <= 0.0)
		{
			continue;
		}

		float dist = length(var_TexCoords - u_vlightPositions[i]);
		float invDepth = 1.0 - var_LightColor[i].a;
		float fall = clamp((fBloomrayFalloffRange * invDepth) - dist, 0.0, 1.0) * invDepth;

		if (i == SUN_ID) 
		{
			fall = pow(clamp((2.0 - dist) / 2.0, 0.0, 1.0), 2.0);
		}

		if (fall > 0.0)
		{
			vec3 spotColor = u_vlightColors[i];

			inRangePositions[numInRange] = u_vlightPositions[i];
			lightDepths[numInRange] = 1.0 - var_LightColor[i].a;

			if (i == SUN_ID) 
			{
				fallOffRanges[numInRange] = fall;
				spotColor = var_LightColor[i].rgb;
			}
			else
			{
				fallOffRanges[numInRange] = (fall + (fall*fall)) / 2.0;
			}

			float lightColorLength = length(spotColor);

			// Try to maximize light strengths...
			spotColor /= lightColorLength;

			lightColors[numInRange] = spotColor;

			if (i == SUN_ID) 
			{
				isSun[numInRange] = true;
				lightColors[numInRange] *= 1.5;
			}
			else
			{
				isSun[numInRange] = false;
				lightColors[numInRange] *= 1.5;
			}

			numInRange++;
		}
	}

	if (numInRange <= 0)
	{// Nothing in range...
		gl_FragColor = vec4(0.0);
		return;
	}

	vec3 totalColor = vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < MAX_VOLUMETRIC_LIGHTS; i++) // MAX_VOLUMETRIC_LIGHTS to use constant loop size
	{
		if (i >= numInRange) break;

		float	lens = 0.0;
		vec2	ScreenLightPos = inRangePositions[i];
		vec2	texCoord = var_TexCoords;
		float	lightDepth = lightDepths[i];
		vec2	deltaTexCoord = (texCoord.xy - ScreenLightPos.xy);

		deltaTexCoord *= 1.0 / float(float(iBloomraySamples) * fBloomrayDensity);

		float illuminationDecay = 1.0;

		for(int g = 0; g < iBloomraySamples; g++)
		{
			texCoord -= deltaTexCoord;

			float linDepth = linearize(textureLod(u_ScreenDepthMap, texCoord.xy, 0.0).r);

			lens += linDepth * illuminationDecay * fBloomrayWeight;

			illuminationDecay *= fBloomrayDecay;

			if (illuminationDecay <= 0.0)
				break;
		}

		if (isSun[i])
		{
			totalColor += clamp(lightColors[i].rgb * (lens * 0.05) * fallOffRanges[i], 0.0, 1.0);
		}
		else
		{
			totalColor += clamp((lightColors[i].rgb * (lens * 0.5/*0.3*/) * (1.0 - lightDepth)) * fallOffRanges[i], 0.0, 1.0);
		}
	}

	totalColor.rgb *= VOLUMETRIC_STRENGTH;

	gl_FragColor = vec4(totalColor, 1.0);
}
