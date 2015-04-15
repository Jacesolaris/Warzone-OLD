uniform sampler2D u_DiffuseMap;

#if defined(USE_LIGHTMAP)
uniform sampler2D u_LightMap;

uniform int       u_Texture1Env;
#endif

varying vec2      var_DiffuseTex;

#if defined(USE_LIGHTMAP)
varying vec2      var_LightTex;
#endif

varying vec4      var_Color;

varying vec2	var_Dimensions;

out vec4 out_Glow;

float SampleDepth(sampler2D normalMap, vec2 t)
{
	vec3 color = texture2D(u_DiffuseMap, t).rgb;

#define const_1 ( 16.0 / 255.0)
#define const_2 (255.0 / 219.0)
	vec3 color2 = ((color - const_1) * const_2);
#define const_3 ( 125.0 / 255.0)
#define const_4 (255.0 / 115.0)
		
	color = ((color - const_3) * const_4);

	color = clamp(color * color * (color * 5.0), 0.0, 1.0); // testing

	//vec3 orig_color = color * 2.0;
	vec3 orig_color = color + color2;

	orig_color = clamp(orig_color * 2.0, 0.0, 1.0); // testing

	orig_color = clamp(orig_color, 0.0, 1.0);
	float combined_color2 = orig_color.r + orig_color.g + orig_color.b;
	combined_color2 /= 4.0;

	return clamp(1.0 - combined_color2, 0.0, 1.0);
}


float RayIntersectDisplaceMap(vec2 dp, vec2 ds, sampler2D normalMap)
{
	const float MAX_SIZE = 1.0;//1.5;
	const int linearSearchSteps = 16;
	const int binarySearchSteps = 6;

	// current size of search window
	float size = MAX_SIZE / float(linearSearchSteps);

	// current depth position
	float depth = 0.0;

	// best match found (starts with last position 1.0)
	float bestDepth = MAX_SIZE;

	// search front to back for first point inside object
	for(int i = 0; i < linearSearchSteps - 1; ++i)
	{
		depth += size;
		
		float t = SampleDepth(normalMap, dp + ds * depth) * MAX_SIZE;
		
		//if(bestDepth > 0.996)		// if no depth found yet
		if(bestDepth > MAX_SIZE - (MAX_SIZE / linearSearchSteps))		// if no depth found yet
			if(depth >= t)
				bestDepth = depth;	// store best depth
	}

	depth = bestDepth;
	
	// recurse around first point (depth) for closest match
	for(int i = 0; i < binarySearchSteps; ++i)
	{
		size *= 0.5;

		float t = SampleDepth(normalMap, dp + ds * depth) * MAX_SIZE;
		
		if(depth >= t)
		{
			bestDepth = depth;
			depth -= 2.0 * size;
		}

		depth += size;
	}

	//return ((bestDepth * var_Local1.x) + (SampleDepth(normalMap, dp) - 1.0)) * 0.5;
	return bestDepth;
}

void main()
{
	vec2 texCoords = var_DiffuseTex;
	
	vec2 offsetDir = vec2(1.0 / var_Dimensions.x, 1.0 / var_Dimensions.y);

	texCoords += offsetDir.xy * RayIntersectDisplaceMap(texCoords, offsetDir.xy, u_DiffuseMap);

	vec4 color = texture2D(u_DiffuseMap, texCoords);

	//vec4 color  = texture2D(u_DiffuseMap, var_DiffuseTex);

#if defined(USE_LIGHTMAP)
	vec4 color2 = texture2D(u_LightMap, var_LightTex);

  #if defined(RGBM_LIGHTMAP)
	color2.rgb *= color2.a;
	color2.a = 1.0;
  #endif

	if (u_Texture1Env == TEXENV_MODULATE)
	{
		color *= color2;
	}
	else if (u_Texture1Env == TEXENV_ADD)
	{
		color += color2;
	}
	else if (u_Texture1Env == TEXENV_REPLACE)
	{
		color = color2;
	}
	
	//color = color * (u_Texture1Env.xxxx + color2 * u_Texture1Env.z) + color2 * u_Texture1Env.y;
#endif

	gl_FragColor = color * var_Color;

#if defined(USE_GLOW_BUFFER)
	out_Glow = gl_FragColor;
#else
	out_Glow = vec4(0.0);
#endif
}
