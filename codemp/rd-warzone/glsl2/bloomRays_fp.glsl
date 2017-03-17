uniform sampler2D				u_DiffuseMap;
uniform sampler2D				u_GlowMap;

uniform vec4					u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2					u_Dimensions;

uniform vec4					u_Local1;

varying vec2					var_TexCoords;

#define BLOOMRAYS_STEPS			4//u_Local1.a//32
#define	BLOOMRAYS_DECAY			u_Local1.r//0.975
#define	BLOOMRAYS_WEIGHT		u_Local1.g//0.1
#define	BLOOMRAYS_DENSITY		u_Local1.b//2.0
#define	BLOOMRAYS_FALLOFF		1.0
#define	BLOOMRAYS_THRESHOLD		0.0

// 9 quads on screen for positions...
const vec2    lightPositions[9] = vec2[] ( vec2(0.25, 0.25), vec2(0.25, 0.5), vec2(0.25, 0.75), vec2(0.5, 0.25), vec2(0.5, 0.5), vec2(0.5, 0.75), vec2(0.75, 0.25), vec2(0.75, 0.5), vec2(0.75, 0.75) );

vec4 ProcessBloomRays(vec2 inTC)
{
	vec4 totalColor = vec4(0.0, 0.0, 0.0, 0.0);

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
          
			deltaTexCoord *= 1.0 / float(float(BLOOMRAYS_STEPS) * BLOOMRAYS_DENSITY);
          
			float illuminationDecay = 1.0;
          
			for(int g = 0; g < BLOOMRAYS_STEPS && illuminationDecay > 0.0; g++) 
			{
				texCoord -= deltaTexCoord;
				vec4 sample2 = texture(u_GlowMap, texCoord.xy);
				sample2.w = 1.0;
				sample2 *= illuminationDecay * BLOOMRAYS_WEIGHT;
          
				lens.xyz += sample2.xyz*sample2.w;
				illuminationDecay *= BLOOMRAYS_DECAY;
			}
          
			totalColor += clamp(lens * fallOffRange, 0.0, 1.0);
		}
	}

	totalColor=max(totalColor, 0.0);
	totalColor=min(totalColor, 1.0);
	totalColor.w=1.0;

	return totalColor;
}

void main()
{
	vec4 color = texture2D(u_DiffuseMap, var_TexCoords);
	gl_FragColor = vec4(color.rgb + ProcessBloomRays(var_TexCoords).rgb, 1.0);
}
