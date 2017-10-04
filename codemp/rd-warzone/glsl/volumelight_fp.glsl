uniform sampler2D			u_DiffuseMap;
uniform sampler2D			u_GlowMap;
uniform sampler2D			u_ScreenDepthMap;

uniform vec2				u_vlightPositions;
uniform vec3				u_vlightColors;

uniform vec4				u_Local0;
uniform vec4				u_Local1; // nightScale, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value
uniform vec4				u_ViewInfo; // zmin, zmax, zmax / zmin, SUN_ID

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

void main ( void )
{
	if (u_Local1.r >= 1.0)
	{// Night...
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec3	totalColor = vec3(0.0, 0.0, 0.0);
	vec3	lightColor = u_vlightColors * 1.5;

	float dist = length(var_TexCoords - u_vlightPositions);
	float fall = pow(clamp((2.0 - dist) / 2.0, 0.0, 1.0), 2.0);

	float	lens = 0.0;
	vec2	texCoord = var_TexCoords;
	vec2	deltaTexCoord = (texCoord.xy - u_vlightPositions);

	deltaTexCoord *= 1.0 / float(float(iBloomraySamples) * fBloomrayDensity);

	float illuminationDecay = 1.0;

//#pragma unroll iBloomraySamples
	for(int g = 0; g < iBloomraySamples; g++)
	{
		texCoord -= deltaTexCoord;
		float linDepth = textureLod(u_ScreenDepthMap, texCoord.xy, 0.0).r;
		lens += linDepth * illuminationDecay * fBloomrayWeight;
		illuminationDecay *= fBloomrayDecay;

		if (illuminationDecay <= 0.0)
			break;
	}

	totalColor += clamp(lightColor * (lens * 0.05) * fall, 0.0, 1.0);

	totalColor.rgb += u_vlightColors * 0.05;

	totalColor.rgb *= VOLUMETRIC_STRENGTH * 2.75;

	// Amplify contrast...
#define lightLower ( 64.0 / 255.0 )
#define lightUpper ( 255.0 / 64.0 )
	totalColor.rgb = clamp((totalColor.rgb - lightLower) * lightUpper, 0.0, 1.0);

	if (u_Local1.r > 0.0)
	{// Sunset, Sunrise, and Night times... Scale down screen color, before adding lighting...
		vec3 nightColor = vec3(0.0);
		totalColor.rgb = mix(totalColor.rgb, nightColor, u_Local1.r);
	}

	gl_FragColor = vec4(totalColor, 1.0);
}
