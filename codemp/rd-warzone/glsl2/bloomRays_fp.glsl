uniform sampler2D				u_DiffuseMap;
uniform sampler2D				u_GlowMap;
uniform sampler2D				u_ScreenDepthMap;

uniform vec4					u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2					u_Dimensions;

uniform vec4					u_Local1; // r_bloomRaysDecay, r_bloomRaysWeight, r_bloomRaysDensity, r_bloomRaysStrength
uniform vec4					u_Local2; // nightScale, r_bloomRaysSamples, testvalue0, testvalue1

varying vec2					var_TexCoords;

#define BLOOMRAYS_STEPS			u_Local2.g//32
#define	BLOOMRAYS_DECAY			u_Local1.r
#define	BLOOMRAYS_WEIGHT		u_Local1.g
#define	BLOOMRAYS_DENSITY		u_Local1.b
#define	BLOOMRAYS_STRENGTH		u_Local1.a
#define	BLOOMRAYS_FALLOFF		1.0

void AddContrast ( inout vec3 color )
{
	const float contrast = 1.25;
	const float brightness = 0.1;
	// Apply contrast.
	color.rgb = ((color.rgb - 0.5f) * max(contrast, 0)) + 0.5f;
	// Apply brightness.
	color.rgb += brightness;
}

// 9 quads on screen for positions...
const vec2    lightPositions[9] = vec2[] ( vec2(0.25, 0.25), vec2(0.25, 0.5), vec2(0.25, 0.75), vec2(0.5, 0.25), vec2(0.5, 0.5), vec2(0.5, 0.75), vec2(0.75, 0.25), vec2(0.75, 0.5), vec2(0.75, 0.75) );

vec4 ProcessBloomRays(vec2 inTC)
{
	vec4 totalColor = vec4(0.0, 0.0, 0.0, 0.0);

//#pragma unroll 9
	for (int i = 0; i < 9; i++)
	{
		float dist = length(inTC.xy - lightPositions[i]);
		float fall = clamp(BLOOMRAYS_FALLOFF - dist, 0.0, 1.0);
	
		if (fall > 0.0)
		{// Within range...
       		float   fallOffRange = (fall + (fall*fall)) / 2.0;
			vec4	lens = vec4(0.0, 0.0, 0.0, 0.0);
			vec2	ScreenLightPos = lightPositions[i];
			vec2	texCoord = inTC.xy;
			vec2	deltaTexCoord = (texCoord.xy - ScreenLightPos.xy);
			float	lightLinDepth = textureLod(u_ScreenDepthMap, lightPositions[i], 0.0).r;
          
			deltaTexCoord *= 1.0 / float(float(BLOOMRAYS_STEPS) * BLOOMRAYS_DENSITY);
          
			float illuminationDecay = 1.0;
          
//#pragma unroll BLOOMRAYS_STEPS
			for(int g = 0; g < BLOOMRAYS_STEPS; g++) 
			{
				texCoord -= deltaTexCoord;

				float linDepth = textureLod(u_ScreenDepthMap, texCoord.xy, 0.0).r;
				linDepth = clamp(linDepth / lightLinDepth, 0.0, 1.0) * 0.75;

				vec4 sample2 = texture(u_GlowMap, texCoord.xy);
				sample2.w = 1.0;
				sample2 *= linDepth * illuminationDecay * BLOOMRAYS_WEIGHT;
          
				lens.xyz += sample2.xyz*sample2.w;
				illuminationDecay *= BLOOMRAYS_DECAY;

				if (illuminationDecay <= 0.0)
					break;
			}
          
			totalColor += clamp(lens * fallOffRange * (1.0 - lightLinDepth), 0.0, 1.0);
		}
	}

	totalColor=clamp(totalColor, 0.0, 1.0);
	totalColor.w=1.0;

	AddContrast(totalColor.rgb);

	totalColor.rgb *= BLOOMRAYS_STRENGTH;

// Amplify contrast...
#define lightLower ( 0.0 / 255.0 )
#define lightUpper ( 255.0 / 24.0 )
	totalColor.rgb = clamp((totalColor.rgb - lightLower) * lightUpper, 0.0, 1.0);

	if (u_Local2.r > 0.0)
	{// Sunset/Sunrise/Night... Scale down the glows to reduce flicker...
		totalColor.rgb = mix(totalColor.rgb, totalColor.rgb / 3.0, u_Local2.r);
	}

	return totalColor;
}

void main()
{
	gl_FragColor = vec4(ProcessBloomRays(var_TexCoords).rgb, 1.0);
}
