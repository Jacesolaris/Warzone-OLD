uniform sampler2D u_DiffuseMap;

#define FPS_BOOST

#if defined(USE_LIGHTMAP)
uniform sampler2D u_LightMap;

uniform int       u_Texture1Env;
#endif

uniform vec4		u_Local1; // parallaxScale, haveSpecular, specularScale, 0
varying vec3		var_ViewDir;
varying vec3		var_Normal;
varying vec4		var_LightDir;

varying vec2      var_DiffuseTex;

#if defined(USE_LIGHTMAP)
varying vec2      var_LightTex;
#endif

varying vec4      var_Color;

uniform vec2	u_Dimensions;

out vec4 out_Glow;
//out vec4 out_Normal;

#ifndef FPS_BOOST
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

	//return ((bestDepth * u_Local1.x) + (SampleDepth(normalMap, dp) - 1.0)) * 0.5;
	//return bestDepth;
	return bestDepth * u_Local1.x;
}

float CalcGGX(float NH, float gloss)
{
	float a_sq = exp2(gloss * -13.0 + 1.0);
	float d = ((NH * NH) * (a_sq - 1.0) + 1.0);
	return a_sq / (d * d);
}

float CalcFresnel(float EH)
{
#if 1
	return exp2(-10.0 * EH);
#elif 0
	return exp2((-5.55473 * EH - 6.98316) * EH);
#elif 0
	float blend = 1.0 - EH;
	float blend2 = blend * blend;
	blend *= blend2 * blend2;
	
	return blend;
#else
	return pow(1.0 - EH, 5.0);
#endif
}

float CalcVisibility(float NH, float NL, float NE, float EH, float gloss)
{
	float roughness = exp2(gloss * -6.5);

	float k = roughness + 1.0;
	k *= k * 0.125;

	float k2 = 1.0 - k;
	
	float invGeo1 = NL * k2 + k;
	float invGeo2 = NE * k2 + k;

	return 1.0 / (invGeo1 * invGeo2);
}

vec3 CalcSpecular(vec3 specular, float NH, float NL, float NE, float EH, float gloss, float shininess)
{
	float distrib = CalcGGX(NH, gloss);

	vec3 fSpecular = mix(specular, vec3(1.0), CalcFresnel(EH));

	float vis = CalcVisibility(NH, NL, NE, EH, gloss);

	return fSpecular * (distrib * vis);
}


float CalcLightAttenuation(float point, float normDist)
{
	// zero light at 1.0, approximating q3 style
	// also don't attenuate directional light
	float attenuation = (0.5 * normDist - 1.5) * point + 1.0;

	// clamp attenuation
	#if defined(NO_LIGHT_CLAMP)
	attenuation = max(attenuation, 0.0);
	#else
	attenuation = clamp(attenuation, 0.0, 1.0);
	#endif

	return attenuation;
}

mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );

	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame 
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}
#endif //FPS_BOOST

void main()
{
#ifndef FPS_BOOST
	vec3 viewDir;
	vec3 L, N, E, H;
	float NL, NH, NE, EH;

	vec2 texCoords = var_DiffuseTex;

	mat3 tangentToWorld = cotangent_frame(var_Normal.xyz, -var_ViewDir, texCoords.xy);
	viewDir = var_ViewDir;
	E = normalize(viewDir);
	L = var_LightDir.xyz;
	N = var_Normal.xyz;
	N = normalize(N);
	float sqrLightDist = dot(L, L);
	L /= sqrt(sqrLightDist);
	NL = clamp(dot(N, L), 0.0, 1.0);
	NE = clamp(dot(N, E), 0.0, 1.0);
	
	//vec2 offsetDir = vec2(1.0 / u_Dimensions.x, 1.0 / u_Dimensions.y);
	vec3 offsetDir = normalize(E * tangentToWorld) * (1.0 / u_Dimensions.x);

	texCoords += offsetDir.xy * RayIntersectDisplaceMap(texCoords, offsetDir.xy, u_DiffuseMap);

	vec4 color = texture2D(u_DiffuseMap, texCoords);

	//vec4 color  = texture2D(u_DiffuseMap, var_DiffuseTex);
#else //FPS_BOOST
	vec2 texCoords = var_DiffuseTex;
	vec4 color = texture2D(u_DiffuseMap, texCoords);
#endif //FPS_BOOST

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

#ifndef FPS_BOOST
	if (u_Local1.b > 0.0)
	{// Specular mapping on this stuff, pls...
		vec3 reflectance;
		float fakedepth = SampleDepth(u_DiffuseMap, texCoords);
		vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
		specular.a = ((clamp((1.0 - fakedepth), 0.0, 1.0) * 0.5) + 0.5);
		specular.a = clamp((specular.a * 2.0) * specular.a, 0.2, 0.9);
		specular *= u_Local1.b;

		float gloss = specular.a;
		float shininess = exp2(gloss * 13.0);
		color.rgb *= vec3(1.0) - specular.rgb;
		reflectance = color.rgb;
		float adjGloss = gloss;
		float adjShininess = shininess;

		H = normalize(L + E);

		EH = clamp(dot(E, H), 0.0, 1.0);
		NH = clamp(dot(N, H), 0.0, 1.0);

		float attenuation = CalcLightAttenuation(float(var_LightDir.w > 0.0), var_LightDir.w / sqrLightDist);

		reflectance += CalcSpecular(specular.rgb, NH, NL, NE, EH, adjGloss, adjShininess);

		vec3 reflection = (reflectance * specular.a) * (attenuation * NL);
		float reduce = ((reflection.r + reflection.g + reflection.b) / 3.0) * specular.a;
		float bright = ((color.r + color.g + color.b) / 3.0);
		
		if (bright > 0.0)
			color.rgb  = clamp((color.rgb - vec3(reduce)) + (reflection * specular.a), 0.0, 1.0);
	}
#endif //FPS_BOOST

	gl_FragColor = color * var_Color;

#if defined(USE_GLOW_BUFFER)
	out_Glow = gl_FragColor;
#else
	out_Glow = vec4(0.0);
#endif

#ifndef FPS_BOOST
	//out_Normal = vec4(N.xyz * 0.5 + 0.5, 0.0);
#endif //FPS_BOOST
}
