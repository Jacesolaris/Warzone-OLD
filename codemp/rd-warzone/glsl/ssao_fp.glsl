uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_SpecularMap;
uniform sampler2D	u_NormalMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec4		u_Local0;

varying vec2		var_ScreenTex;
varying vec2		var_Dimensions;
varying vec3		var_Position;

#if 0
vec3 vLocalSeed;
// This function returns random number from zero to one
float randZeroOne()
{
    uint n = floatBitsToUint(vLocalSeed.y * 214013.0 + vLocalSeed.x * 2531011.0 + vLocalSeed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
    float fRes =  2.0 - uintBitsToFloat(n);
    vLocalSeed = vec3(vLocalSeed.x + 147158.0 * fRes, vLocalSeed.y*fRes  + 415161.0 * fRes, vLocalSeed.z + 324154.0*fRes);
    return fRes;
}



float random( const vec2 p )
{
    return randZeroOne();
}



mat2 randomRotation( const vec2 p )
{
    float r = random(p);
    float sinr = sin(r);
    float cosr = cos(r);
    return mat2(cosr, sinr, -sinr, cosr);
}



float getLinearDepth(sampler2D depthMap, const vec2 tex)
{
    float depth = texture(depthMap, tex).r;
    return clamp(1.0 / mix(u_ViewInfo.z, 1.0, depth), 0.0, 1.0);
}



vec2 poissonDisc[9] = vec2[9](
vec2(-0.7055767, 0.196515),    vec2(0.3524343, -0.7791386),
vec2(0.2391056, 0.9189604),    vec2(-0.07580382, -0.09224417),
vec2(0.5784913, -0.002528916), vec2(0.192888, 0.4064181),
vec2(-0.6335801, -0.5247476),  vec2(-0.5579782, 0.7491854),
vec2(0.7320465, 0.6317794)
);
float ambientOcclusion(const vec2 tex)
{
    float result = 0;
    float zFar = u_ViewInfo.g;
    float sampleZ = zFar * getLinearDepth(u_ScreenDepthMap, tex);
    vec2 expectedSlope = vec2(dFdx(sampleZ), dFdy(sampleZ)) / vec2(dFdx(tex.x), dFdy(tex.y));
    //if (length(expectedSlope) > 5000.0)
	//	return 1.0;
	
	vec2 offsetScale = vec2(3.0 / sampleZ);
    mat2 rmat = randomRotation(tex);
    int i;
    for (i = 0; i < 3; i++)
	{
        vec2 offset = rmat * poissonDisc[i] * offsetScale;
        float sampleZ2 = zFar * getLinearDepth(u_ScreenDepthMap, tex + offset);
        //if (abs(sampleZ - sampleZ2) > 20.0)
		//	result += 1.0;
		//else
		{
            float expectedZ = sampleZ + dot(expectedSlope, offset);
            result += step(expectedZ - 1.0, sampleZ2);
        }


	}
	
	result *= 0.33333;
    //result *= 0.5;
	
	return result;
}


#else
#define USE_NORMALMAP

const float		total_strength = 1.0;
const float		base = 0.2;
const float		area = 0.0075;
/*const*/ float		falloff = u_Local0.r;//0.000001;
/*const*/ float		radius = u_Local0.g;//0.0002;
const int		samples = 16;

const vec3 sample_sphere[samples] = const vec3[samples](
      const vec3( 0.5381, 0.1856,-0.4319), 
	  const vec3( 0.1379, 0.2486, 0.4430),
      const vec3( 0.3371, 0.5679,-0.0057), 
	  const vec3(-0.6999,-0.0451,-0.0019),
      const vec3( 0.0689,-0.1598,-0.8547), 
	  const vec3( 0.0560, 0.0069,-0.1843),
      const vec3(-0.0146, 0.1402, 0.0762), 
	  const vec3( 0.0100,-0.1924,-0.0344),
      const vec3(-0.3577,-0.5301,-0.4358), 
	  const vec3(-0.3169, 0.1063, 0.0158),
      const vec3( 0.0103,-0.5869, 0.0046), 
	  const vec3(-0.0897,-0.4940, 0.3287),
      const vec3( 0.7119,-0.0154,-0.0918), 
	  const vec3(-0.0533, 0.0596,-0.5411),
      const vec3( 0.0352,-0.0631, 0.5460), 
	  const vec3(-0.4776, 0.2847,-0.0271)
);

//vec2 projAB = vec2( u_ViewInfo.g / (u_ViewInfo.g - u_ViewInfo.r), u_ViewInfo.g * u_ViewInfo.r / (u_ViewInfo.g - u_ViewInfo.r) );

float getLinearDepth(sampler2D depthMap, vec2 tex)
{
    float depth = texture(depthMap, tex).r;
    return clamp(1.0 / mix(u_ViewInfo.z, 1.0, depth), 0.0, 1.0);
	//float linearDepth = projAB.y / (depth - projAB.x);
	//return linearDepth;
	//return (u_ViewInfo.r * u_ViewInfo.g / (u_ViewInfo.g - depth * (u_ViewInfo.g - u_ViewInfo.r)) - u_ViewInfo.r);
}

vec2 offset1 = vec2(0.0, 1.0 / var_Dimensions.y);
vec2 offset2 = vec2(1.0 / var_Dimensions.x, 0.0);

vec3 normal_from_depth(float depth, vec2 texcoords)
{
#if defined(USE_NORMALMAP)
	/*
    float depth1 = getLinearDepth(u_ScreenDepthMap, texcoords + offset1);
    float depth2 = getLinearDepth(u_ScreenDepthMap, texcoords + offset2);
    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);
    vec3 normal = cross(p1, p2);
    normal.z = -normal.z;
	return normalize((texture(u_NormalMap, texcoords).rgb + normal) / 2.0);
	*/
	return texture(u_NormalMap, texcoords).rgb * 2.0 - 1.0;
#else //!defined(USE_NORMALMAP)
    float depth1 = getLinearDepth(u_ScreenDepthMap, texcoords + offset1);
    float depth2 = getLinearDepth(u_ScreenDepthMap, texcoords + offset2);
    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);
    vec3 normal = cross(p1, p2);
    normal.z = -normal.z;
    return normalize(normal);
#endif //defined(USE_NORMALMAP)
}

float ambientOcclusion(vec2 tex)
{
    //vec3 random = normalize( texture2D(u_SpecularMap, tex * 4.0).rgb );
    float depth = getLinearDepth(u_ScreenDepthMap, tex);
    vec3 position = vec3(tex, depth);
    vec3 normal = normal_from_depth(depth, tex);
    float radius_depth = radius/depth;
    float occlusion = 0.0;

    for(int i=0; i < samples; i++) {
        vec3 ray = radius_depth * reflect(sample_sphere[i], normal);//random);
		vec3 hemi_ray = position + (sign(dot(ray,normal)) * ray);
        float occ_depth = getLinearDepth(u_ScreenDepthMap, clamp(hemi_ray.xy, 0.0, 1.0));
        float difference = depth - occ_depth;
        occlusion += step(falloff, difference) * (1.0-smoothstep(falloff, area, difference));
    }

	float ao = 1.0 - clamp(total_strength * occlusion * (1.0 / samples), 0.0, 1.0);
    return clamp(ao + base, 0.0, 1.0);
}


#endif

void main()
{
    //vLocalSeed = var_Position.xyz;
    float ao = clamp(ambientOcclusion(var_ScreenTex), 0.0, 1.0);
    gl_FragColor = vec4(vec3(ao), 1.0);
}
