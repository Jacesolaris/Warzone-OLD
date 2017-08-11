uniform sampler2D			u_DiffuseMap;
uniform sampler2D			u_GlowMap;
uniform sampler2D			u_SpecularMap;
uniform sampler2D			u_ScreenDepthMap;

uniform vec2				u_vlightPositions;
uniform vec3				u_vlightColors;

uniform vec4				u_Local0;
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

float linearize(float depth)
{
	return (1.0 / mix(u_ViewInfo.z, 1.0, depth));
}

void main ( void )
{
	vec3	totalColor = vec3(0.0, 0.0, 0.0);

#if 0
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
		float linDepth = 1.0 - linearize(textureLod(u_ScreenDepthMap, texCoord.xy, 0.0).r);
		lens += linDepth * illuminationDecay * fBloomrayWeight;
		illuminationDecay *= fBloomrayDecay;

		if (illuminationDecay <= 0.0)
			break;
	}

	totalColor += clamp(lightColor * (lens * 0.05) * fall, 0.0, 1.0);
#endif

	totalColor.rgb += u_vlightColors * 0.05;
	totalColor.rgb *= VOLUMETRIC_STRENGTH * 2.75;

	gl_FragColor = vec4(totalColor, 1.0);
}
