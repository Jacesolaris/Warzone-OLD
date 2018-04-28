uniform sampler2D   u_DiffuseMap;		// screen diffuse map
uniform sampler2D   u_ScreenDepthMap;	// depth
uniform sampler2D   u_PositionMap;		// position map
uniform sampler2D   u_NormalMap;		// normal map
uniform sampler2D   u_OverlayMap;		// detailed normal map

uniform vec2        u_Dimensions;
uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec4		u_PrimaryLightOrigin;

uniform vec4		u_Local0; // r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value
uniform vec4		u_Local1;

varying vec2	var_ScreenTex;
varying vec3	var_Position;


#ifdef __USE_REAL_NORMALMAPS__
//#define __USE_DETAIL_NORMALS__ // Not needed. Waste of time...
#endif //__USE_REAL_NORMALMAPS__

#ifdef __USE_DETAIL_NORMALS__
#define __FAST_NORMAL_DETAIL__
#endif //__USE_DETAIL_NORMALS__

#define NUM_OCCLUSION_CHECKS 8//16


vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N*4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);

	return vec3(encoded * g, 1.0 - f * 0.5);
}

#ifdef __FAST_NORMAL_DETAIL__
vec4 normalVector(vec3 color) {
	vec4 normals = vec4(color.rgb, length(color.rgb) / 3.0);
	normals.rgb = vec3(length(normals.r - normals.a), length(normals.g - normals.a), length(normals.b - normals.a));

	// Contrast...
	//#define normLower ( 128.0 / 255.0 )
	//#define normUpper (255.0 / 192.0 )
#define normLower ( 32.0 / 255.0 )
#define normUpper (255.0 / 212.0 )
	vec3 N = clamp((clamp(normals.rgb - normLower, 0.0, 1.0)) * normUpper, 0.0, 1.0);

	return vec4(vec3(1.0) - (normalize(pow(N, vec3(4.0))) * 0.5 + 0.5), 1.0 - normals.a);
}
#else //!__FAST_NORMAL_DETAIL__
float getHeight(vec2 uv) {
  return length(texture(u_DiffuseMap, uv).rgb) / 3.0;
}

vec4 bumpFromDepth(vec2 uv, vec2 resolution, float scale) {
  vec2 step = 1. / resolution;
    
  float height = getHeight(uv);
    
  vec2 dxy = height - vec2(
      getHeight(uv + vec2(step.x, 0.)), 
      getHeight(uv + vec2(0., step.y))
  );

  vec3 N = vec3(dxy * scale / step, 1.);

// Contrast...
#define normLower ( 128.0 / 255.0 )
#define normUpper (255.0 / 192.0 )
  N = clamp((clamp(N - normLower, 0.0, 1.0)) * normUpper, 0.0, 1.0);

  return vec4(normalize(N) * 0.5 + 0.5, height);
}

vec4 normalVector(vec2 coord) {
	return bumpFromDepth(coord, u_Dimensions, 0.1 /*scale*/);
}
#endif //__FAST_NORMAL_DETAIL__

const vec3 unKernel[32] = vec3[]
(
	vec3(-0.134, 0.044, -0.825),
	vec3(0.045, -0.431, -0.529),
	vec3(-0.537, 0.195, -0.371),
	vec3(0.525, -0.397, 0.713),
	vec3(0.895, 0.302, 0.139),
	vec3(-0.613, -0.408, -0.141),
	vec3(0.307, 0.822, 0.169),
	vec3(-0.819, 0.037, -0.388),
	vec3(0.376, 0.009, 0.193),
	vec3(-0.006, -0.103, -0.035),
	vec3(0.098, 0.393, 0.019),
	vec3(0.542, -0.218, -0.593),
	vec3(0.526, -0.183, 0.424),
	vec3(-0.529, -0.178, 0.684),
	vec3(0.066, -0.657, -0.570),
	vec3(-0.214, 0.288, 0.188),
	vec3(-0.689, -0.222, -0.192),
	vec3(-0.008, -0.212, -0.721),
	vec3(0.053, -0.863, 0.054),
	vec3(0.639, -0.558, 0.289),
	vec3(-0.255, 0.958, 0.099),
	vec3(-0.488, 0.473, -0.381),
	vec3(-0.592, -0.332, 0.137),
	vec3(0.080, 0.756, -0.494),
	vec3(-0.638, 0.319, 0.686),
	vec3(-0.663, 0.230, -0.634),
	vec3(0.235, -0.547, 0.664),
	vec3(0.164, -0.710, 0.086),
	vec3(-0.009, 0.493, -0.038),
	vec3(-0.322, 0.147, -0.105),
	vec3(-0.554, -0.725, 0.289),
	vec3(0.534, 0.157, -0.250)
);

vec3 vLocalSeed;

// This function returns random number from zero to one
float randZeroOne()
{
	uint n = floatBitsToUint(vLocalSeed.y * 214013.0 + vLocalSeed.x * 2531011.0 + vLocalSeed.z * 141251.0);
	n = n * (n * n * 15731u + 789221u);
	n = (n >> 9u) | 0x3F800000u;

	float fRes = 2.0 - uintBitsToFloat(n);
	vLocalSeed = vec3(vLocalSeed.x + 147158.0 * fRes, vLocalSeed.y*fRes + 415161.0 * fRes, vLocalSeed.z + 324154.0*fRes);
	return fRes;
}

float ssao( in vec3 position, in vec2 pixel, in vec3 normal, in vec3 light, in float resolution, in float strength, in float minDistance, in float maxDisance )
{
    vec2  uv  = pixel;
    float z   = texture2D( u_ScreenDepthMap, uv ).x;		// read eye linear z

	if (z >= 1.0)
	{// Sky...
		return 1.0;
	}

	vec2  res = vec2(resolution) / u_Dimensions.xy;
	float numOcclusions = 0.0;

	vLocalSeed = position;
	vec3 ref = unKernel[int(randZeroOne() * 32.0)];

    // accumulate occlusion
    float bl = 0.0;
    for( int i=0; i<NUM_OCCLUSION_CHECKS; i++ )
    {
		vec3 of = faceforward( reflect( unKernel[i], ref ), light, normal );
		vec2 thisUV = uv + (res * of.xy);

		if (thisUV.x > 1.0 || thisUV.y > 1.0 || thisUV.x < 0.0 || thisUV.y < 0.0)
		{// Don't sample outside of screen bounds...
			continue;
		}

        float sz = texture2D( u_ScreenDepthMap, thisUV).x;
        float zd = (sz-z)*strength;

		if (length(sz - z) < minDistance)
		{
			zd = 0.5;
			bl += clamp(zd*10.0,0.1,1.0)*(1.0-clamp((zd-1.0)/5.0,0.0,1.0));
			numOcclusions += 1.0;
			continue;
		}
		else if (length(sz - z) > maxDisance)
		{
			zd = 0.0;
			bl += clamp(zd*10.0,0.1,1.0)*(1.0-clamp((zd-1.0)/5.0,0.0,1.0));
			numOcclusions += 1.0;
			continue;
		}
		else
		{
			bl += clamp(zd*10.0,0.1,1.0)*(1.0-clamp((zd-1.0)/5.0,0.0,1.0));
			numOcclusions += 1.0;
		}
    }

	//float ao = clamp(bl/float(NUM_OCCLUSION_CHECKS), 0.0, 1.0);
	float ao = clamp(bl/float(numOcclusions), 0.0, 1.0);
	ao = mix(ao, 1.0, z);
    return ao;
}

void main( void ) 
{
	vec4 position = textureLod(u_PositionMap, var_ScreenTex, 0.0);

	if (position.a-1.0 == MATERIAL_SKY || position.a-1.0 == MATERIAL_SUN || position.a-1.0 == MATERIAL_GLASS || position.a-1.0 == MATERIAL_NONE)
	{// Skybox... Skip...
		gl_FragColor=vec4(1.0, 0.0, 0.0, 1.0);
		return;
	}

	vec4 norm = textureLod(u_NormalMap, var_ScreenTex, 0.0);
	//norm.rgb = normalize(norm.rgb * 2.0 - 1.0);
	//norm.z = sqrt(1.0-dot(norm.xy, norm.xy)); // reconstruct Z from X and Y
	//norm.z = sqrt(clamp((0.25 - norm.x * norm.x) - norm.y * norm.y, 0.0, 1.0));
	norm.xyz = DecodeNormal(norm.xy);

#ifdef __USE_DETAIL_NORMALS__
	if (u_Local0.r > 0.0)
	{// Use detail normals...
		vec4 normalDetail = textureLod(u_OverlayMap, var_ScreenTex, 0.0);

		if (normalDetail.a < 1.0)
		{// Don't have real normalmap, make normals for this pixel...
#ifdef __FAST_NORMAL_DETAIL__
			normalDetail = normalVector(texture(u_DiffuseMap, var_ScreenTex).rgb);
#else //!__FAST_NORMAL_DETAIL__
			normalDetail = normalVector(var_ScreenTex);
#endif //__FAST_NORMAL_DETAIL__
		}

		//normalDetail.rgb = normalize(normalDetail.rgb * 2.0 - 1.0);
		//normalDetail.rgb *= 0.25;
		//norm.rgb = norm.rgb + normalDetail.rgb;

		normalDetail.rgb = normalize(clamp(normalDetail.xyz, 0.0, 1.0) * 2.0 - 1.0);
		norm.rgb = normalize(mix(norm.xyz, normalDetail.xyz, 0.25 * (length(norm.xyz - normalDetail.xyz) / 3.0)));
	}
#endif //__USE_DETAIL_NORMALS__

	vec3 N = normalize(norm.xyz);

	vec3 to_light = position.xyz - u_PrimaryLightOrigin.xyz;
	float to_light_dist = length(to_light);
	vec3 to_light_norm = (to_light / to_light_dist);

	float msao = ssao( position.xyz, var_ScreenTex, N.xyz, to_light_norm, 32.0, 64.0, 0.001, 0.01 );
	float sao = clamp(msao, 0.0, 1.0);
	gl_FragColor=vec4(sao, 0.0, 0.0, 1.0);
}
