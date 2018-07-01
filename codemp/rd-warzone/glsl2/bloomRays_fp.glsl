uniform sampler2D				u_DiffuseMap;
uniform sampler2D				u_GlowMap;
uniform sampler2D				u_ScreenDepthMap;

uniform vec4					u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2					u_Dimensions;

uniform vec4					u_Local1; // r_bloomRaysDecay, r_bloomRaysWeight, r_bloomRaysDensity, r_bloomRaysStrength
uniform vec4					u_Local2; // nightScale, r_bloomRaysSamples, testvalue0, testvalue1

varying vec2					var_TexCoords;

#define BLOOMRAYS_STEPS			u_Local2.g
#define	BLOOMRAYS_DECAY			u_Local1.r
#define	BLOOMRAYS_WEIGHT		u_Local1.g
#define	BLOOMRAYS_DENSITY		u_Local1.b
#define	BLOOMRAYS_STRENGTH		u_Local1.a
#define	BLOOMRAYS_FALLOFF		1.0

// 9 quads on screen for positions...
const vec2    lightPositions[9] = vec2[] ( vec2(0.25, 0.25), vec2(0.25, 0.5), vec2(0.25, 0.75), vec2(0.5, 0.25), vec2(0.5, 0.5), vec2(0.5, 0.75), vec2(0.75, 0.25), vec2(0.75, 0.5), vec2(0.75, 0.75) );

void AddContrast(inout float value, float contrast, float brightness)
{
	// Apply contrast.
	value = ((value - 0.5f) * max(contrast, 0)) + 0.5f;
	// Apply brightness.
	value += brightness;
}

vec3 ProcessBloomRays(vec2 inTC)
{
	vec3 totalColor = vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < 9; i++)
	{
		float dist = length(inTC.xy - lightPositions[i]);
		float fall = clamp(BLOOMRAYS_FALLOFF - dist, 0.0, 1.0);

		if (fall > 0.0)
		{// Within range...
			vec3	lens = vec3(0.0, 0.0, 0.0);
			vec2	ScreenLightPos = lightPositions[i];
			vec2	texCoord = inTC.xy;
			vec2	deltaTexCoord = (texCoord.xy - ScreenLightPos.xy);

			deltaTexCoord *= 1.0 / float(float(BLOOMRAYS_STEPS) * BLOOMRAYS_DENSITY);

			float illuminationDecay = 1.0;

			for (int g = 0; g < BLOOMRAYS_STEPS; g++)
			{
				texCoord -= deltaTexCoord;

				if (texCoord.x >= 0.0 && texCoord.x <= 1.0 && texCoord.y >= 0.0 && texCoord.y <= 1.0)
				{// Don't bother with lookups outside screen area...
					vec4 sample2 = texture(u_GlowMap, vec2(texCoord.x, 1.0 - texCoord.y));

					float grey = (length(sample2.rgb * sample2.a) / 3.0) * 0.01;
					AddContrast(grey, 1.175, 0.1);

					lens.xyz += sample2.xyz * grey * illuminationDecay * BLOOMRAYS_WEIGHT;
				}

				illuminationDecay *= BLOOMRAYS_DECAY;

				if (illuminationDecay <= 0.0)
					break;
			}

			totalColor += clamp(lens * 0.25 * fall * fall, 0.0, 1.0);
		}

		totalColor = clamp(totalColor, 0.0, 1.0);
	}

	if (u_Local2.r > 0.5)
	{// Sunset/Sunrise/Night... Scale up the glow strengths at night...
		float nightNess = (u_Local2.r - 0.5) / 0.5;
		float nightMult = nightNess * 4.0;
		totalColor *= BLOOMRAYS_STRENGTH * nightMult;
	}
	else
	{
		totalColor *= BLOOMRAYS_STRENGTH;
	}

	// Amplify contrast...
#define lightLower ( 0.0 / 255.0 )
#define lightUpper ( 255.0 / 24.0 )
	totalColor.rgb = clamp((totalColor - lightLower) * lightUpper, 0.0, 1.0);

	if (u_Local2.r > 0.0)
	{// Sunset/Sunrise/Night... Scale down the glows to reduce flicker...
		totalColor.rgb = mix(totalColor, totalColor / 3.0, u_Local2.r);
	}

	return totalColor;
}

void main()
{
	gl_FragColor = vec4(ProcessBloomRays(var_TexCoords), 1.0);
}
