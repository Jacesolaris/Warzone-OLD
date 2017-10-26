#define __AMBIENT_OCCLUSION__
#define __EXPERIMENTAL_AO__
#define __ENVMAP__
#define __RANDOMIZE_LIGHT_PIXELS__

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_OverlayMap; // Real normals. Alpha channel 1.0 means enabled...
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_HeightMap;
uniform sampler2D	u_ShadowMap;
uniform sampler2D	u_DeluxeMap;  // Random2K image...
uniform sampler2D	u_GlowMap;
uniform samplerCube	u_CubeMap;

uniform mat4		u_ModelViewProjectionMatrix;

uniform vec2		u_Dimensions;

uniform vec4		u_Local1; // r_blinnPhong, SUN_PHONG_SCALE, r_ao, r_env
uniform vec4		u_Local2; // SSDO, SHADOWS_ENABLED, SHADOW_MINBRIGHT, SHADOW_MAXBRIGHT
uniform vec4		u_Local3; // r_testShaderValue1, r_testShaderValue2, r_testShaderValue3, r_testShaderValue4
uniform vec4		u_Local4; // MAP_INFO_MAXSIZE, MAP_WATER_LEVEL, floatTime, MAP_EMISSIVE_COLOR_SCALE
uniform vec4		u_Local5; // CONTRAST, SATURATION, BRIGHTNESS, TRUEHDR_ENABLED
uniform vec4		u_Local6; // AO_MINBRIGHT, AO_MULTBRIGHT, VIBRANCY, NightScale
uniform vec4		u_Local7; // cubemapEnabled, r_cubemapCullRange, r_cubeMapSize, 0.0

uniform vec4		u_ViewInfo; // znear, zfar, zfar / znear, fov
uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

uniform vec4		u_CubeMapInfo;
uniform float		u_CubeMapStrength;

#define MAX_DEFERRED_LIGHTS 128//64//16//24

uniform int			u_lightCount;
//uniform vec2		u_lightPositions[MAX_DEFERRED_LIGHTS];
uniform vec3		u_lightPositions2[MAX_DEFERRED_LIGHTS];
uniform float		u_lightDistances[MAX_DEFERRED_LIGHTS];
uniform float		u_lightHeightScales[MAX_DEFERRED_LIGHTS];
uniform vec3		u_lightColors[MAX_DEFERRED_LIGHTS];

varying vec2		var_TexCoords;


vec2 pixel = vec2(1.0) / u_Dimensions;


vec3 RB_PBR_DefaultsForMaterial(float MATERIAL_TYPE)
{
	vec3 settings = vec3(0.0);

	float specularScale = 0.0;
	float cubemapScale = 0.9;
	float parallaxScale = 0.0;

	switch (int(MATERIAL_TYPE))
	{
	case MATERIAL_WATER:			// 13			// light covering of water on a surface
		specularScale = 1.0;
		cubemapScale = 0.7;
		parallaxScale = 1.5;
		break;
	case MATERIAL_SHORTGRASS:		// 5			// manicured lawn
		specularScale = 0.75;
		cubemapScale = 0.15;
		parallaxScale = 1.5;
		break;
	case MATERIAL_LONGGRASS:		// 6			// long jungle grass
		specularScale = 0.75;
		cubemapScale = 0.15;
		parallaxScale = 1.5;
		break;
	case MATERIAL_SAND:				// 8			// sandy beach
		specularScale = 0.65;
		cubemapScale = 0.2;
		parallaxScale = 1.5;
		break;
	case MATERIAL_CARPET:			// 27			// lush carpet
		specularScale = 0.25;
		cubemapScale = 0.05;
		parallaxScale = 1.5;
		break;
	case MATERIAL_GRAVEL:			// 9			// lots of small stones
		specularScale = 0.30;
		cubemapScale = 0.05;
		parallaxScale = 1.5;
		break;
	case MATERIAL_ROCK:				// 23			//
		specularScale = 0.22;
		cubemapScale = 0.07;
		parallaxScale = 1.5;
		break;
	case MATERIAL_TILES:			// 26			// tiled floor
		specularScale = 0.56;
		cubemapScale = 0.15;
		parallaxScale = 1.5;
		break;
	case MATERIAL_SOLIDWOOD:		// 1			// freshly cut timber
		specularScale = 0.05;
		cubemapScale = 0.07;
		parallaxScale = 1.5;
		break;
	case MATERIAL_HOLLOWWOOD:		// 2			// termite infested creaky wood
		specularScale = 0.025;
		cubemapScale = 0.07;
		parallaxScale = 1.5;
		break;
	case MATERIAL_SOLIDMETAL:		// 3			// solid girders
		specularScale = 0.98;
		cubemapScale = 0.98;
		parallaxScale = 1.5;
		break;
	case MATERIAL_HOLLOWMETAL:		// 4			// hollow metal machines -- UQ1: Used for weapons to force lower parallax and high reflection...
		specularScale = 1.0;
		cubemapScale = 1.0;
		parallaxScale = 1.5;
		break;
	case MATERIAL_DRYLEAVES:		// 19			// dried up leaves on the floor
		specularScale = 0.35;
		cubemapScale = 0.1;
		parallaxScale = 0.0;
		break;
	case MATERIAL_GREENLEAVES:		// 20			// fresh leaves still on a tree
		specularScale = 0.95;
		cubemapScale = 0.2;
		parallaxScale = 0.0; // GreenLeaves should NEVER be parallaxed.. It's used for surfaces with an alpha channel and parallax screws it up...
		break;
	case MATERIAL_FABRIC:			// 21			// Cotton sheets
		specularScale = 0.45;
		cubemapScale = 0.1;
		parallaxScale = 1.5;
		break;
	case MATERIAL_CANVAS:			// 22			// tent material
		specularScale = 0.35;
		cubemapScale = 0.1;
		parallaxScale = 1.5;
		break;
	case MATERIAL_MARBLE:			// 12			// marble floors
		specularScale = 0.65;
		cubemapScale = 0.46;
		parallaxScale = 1.5;
		break;
	case MATERIAL_SNOW:				// 14			// freshly laid snow
		specularScale = 0.75;
		cubemapScale = 0.25;
		parallaxScale = 1.5;
		break;
	case MATERIAL_MUD:				// 17			// wet soil
		specularScale = 0.25;
		cubemapScale = 0.15;
		parallaxScale = 1.5;
		break;
	case MATERIAL_DIRT:				// 7			// hard mud
		specularScale = 0.15;
		cubemapScale = 0.07;
		parallaxScale = 1.5;
		break;
	case MATERIAL_CONCRETE:			// 11			// hardened concrete pavement
		specularScale = 0.375;
		cubemapScale = 0.1;
		parallaxScale = 1.5;
		break;
	case MATERIAL_FLESH:			// 16			// hung meat, corpses in the world
		specularScale = 0.25;
		cubemapScale = 0.1;
		parallaxScale = 1.5;
		break;
	case MATERIAL_RUBBER:			// 24			// hard tire like rubber
		specularScale = 0.25;
		cubemapScale = 0.07;
		parallaxScale = 1.5;
		break;
	case MATERIAL_PLASTIC:			// 25			//
		specularScale = 0.58;
		cubemapScale = 0.28;
		parallaxScale = 1.5;
		break;
	case MATERIAL_PLASTER:			// 28			// drywall style plaster
		specularScale = 0.3;
		cubemapScale = 0.1;
		parallaxScale = 1.5;
		break;
	case MATERIAL_SHATTERGLASS:		// 29			// glass with the Crisis Zone style shattering
		specularScale = 0.93;
		cubemapScale = 0.37;
		parallaxScale = 1.0;
		break;
	case MATERIAL_ARMOR:			// 30			// body armor
		specularScale = 0.5;
		cubemapScale = 0.56;
		parallaxScale = 1.5;
		break;
	case MATERIAL_ICE:				// 15			// packed snow/solid ice
		specularScale = 0.75;
		cubemapScale = 0.47;
		parallaxScale = 2.0;
		break;
	case MATERIAL_GLASS:			// 10			//
		specularScale = 0.95;
		cubemapScale = 0.39;
		parallaxScale = 1.0;
		break;
	case MATERIAL_BPGLASS:			// 18			// bulletproof glass
		specularScale = 0.93;
		cubemapScale = 0.33;
		parallaxScale = 1.0;
		break;
	case MATERIAL_COMPUTER:			// 31			// computers/electronic equipment
		specularScale = 0.92;
		cubemapScale = 0.48;
		parallaxScale = 1.5;
		break;
	default:
		specularScale = 0.15;
		cubemapScale = 0.15;
		parallaxScale = 1.0;
		break;
	}

	settings.x = specularScale;
	settings.y = cubemapScale;
	settings.z = parallaxScale;

	return settings;
}


#ifdef __RANDOMIZE_LIGHT_PIXELS__
float lrand(vec2 co) {
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453) * 0.25 + 0.75;
}
#endif //__RANDOMIZE_LIGHT_PIXELS__

float rand(vec2 co) {
        return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float hash( float n ) {
    return fract(sin(n)*687.3123);
}

float noise( in vec2 x ) {
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*157.0;
    return mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
               mix( hash(n+157.0), hash(n+158.0),f.x),f.y);
}

vec3 TangentFromNormal ( vec3 normal )
{
	vec3 tangent;
	vec3 c1 = cross(normal, vec3(0.0, 0.0, 1.0)); 
	vec3 c2 = cross(normal, vec3(0.0, 1.0, 0.0)); 

	if( length(c1) > length(c2) )
	{
		tangent = c1;
	}
	else
	{
		tangent = c2;
	}

	return normalize(tangent);
}

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


#if defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__)
float drawObject(in vec3 p){
    p = abs(fract(p)-.5);
    return dot(p, vec3(.5));
}

float cellTile(in vec3 p)
{
    p /= 5.5;
    // Draw four overlapping objects at various positions throughout the tile.
    vec4 v, d; 
    d.x = drawObject(p - vec3(.81, .62, .53));
    p.xy = vec2(p.y-p.x, p.y + p.x)*.7071;
    d.y = drawObject(p - vec3(.39, .2, .11));
    p.yz = vec2(p.z-p.y, p.z + p.y)*.7071;
    d.z = drawObject(p - vec3(.62, .24, .06));
    p.xz = vec2(p.z-p.x, p.z + p.x)*.7071;
    d.w = drawObject(p - vec3(.2, .82, .64));

    v.xy = min(d.xz, d.yw), v.z = min(max(d.x, d.y), max(d.z, d.w)), v.w = max(v.x, v.y); 
   
    d.x =  min(v.z, v.w) - min(v.x, v.y); // Maximum minus second order, for that beveled Voronoi look. Range [0, 1].
    //d.x =  min(v.x, v.y); // First order.
        
    return d.x*2.66; // Normalize... roughly.
}

float map(vec3 p)
{
    float n = (.5-cellTile(p))*1.5;
    return p.y + dot(sin(p/2. + cos(p.yzx/2. + 3.14159/2.)), vec3(.5)) + n;
}
#endif //defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__)


#ifdef __AMBIENT_OCCLUSION__
float calculateAO(in vec3 pos, in vec3 nor)
{
	float sca = 0.00013/*2.0*/, occ = 0.0;
    for( int i=0; i<5; i++ ){
    
        float hr = 0.01 + float(i)*0.5/4.0;        
        float dd = map(nor * hr + pos);
        occ += (hr - dd)*sca;
        sca *= 0.7;
    }
    return clamp( 1.0 - occ, 0.0, 1.0 );    
}

// that is really shitty AO but at least unlit fragments do not look so plain... :)
float bad_ao(vec3 n) {
    return abs(dot(n, vec3(0.0, 1.0, 0.0))); 
}
#endif //__AMBIENT_OCCLUSION__

#ifdef __ENVMAP__
vec3 envMap(vec3 p, float warmth)
{
    float c = cellTile(p*6.);
    c = smoothstep(0.2, 1., c); // Contract gives it more of a lit look... kind of.
    
	// Icy glow... for whatever reason.
    vec3 coolMap = vec3(pow(c, 8.), c*c, c);
    
	// Alternate firey glow.
    vec3 heatMap = vec3(min(c*1.5, 1.), pow(c, 2.5), pow(c, 12.));

	// Mix Ice and Heat based on warmth setting...
	return mix(coolMap, heatMap, warmth);
}
#endif //__ENVMAP__



vec3 TrueHDR ( vec3 color )
{
//#define const_1 ( 12.0 / 255.0)
//#define const_2 (255.0 / 229.0)
#define const_1 ( 26.0 / 255.0)
#define const_2 (255.0 / 209.0)
	return clamp((clamp(color.rgb - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);
}

vec3 Vibrancy ( vec3 origcolor, float vibrancyStrength )
{
	vec3	lumCoeff = vec3(0.212656, 0.715158, 0.072186);  				//Calculate luma with these values
	float	max_color = max(origcolor.r, max(origcolor.g,origcolor.b)); 	//Find the strongest color
	float	min_color = min(origcolor.r, min(origcolor.g,origcolor.b)); 	//Find the weakest color
	float	color_saturation = max_color - min_color; 						//Saturation is the difference between min and max
	float	luma = dot(lumCoeff, origcolor.rgb); 							//Calculate luma (grey)
	return mix(vec3(luma), origcolor.rgb, (1.0 + (vibrancyStrength * (1.0 - (sign(vibrancyStrength) * color_saturation))))); 	//Extrapolate between luma and original by 1 + (1-saturation) - current
}


//
// Full lighting... Blinn phong and basic lighting as well...
//

// Blinn-Phong shading model with rim lighting (diffuse light bleeding to the other side).
// `normal`, `view` and `light` should be normalized.
vec3 blinn_phong(vec3 normal, vec3 view, vec3 light, vec3 diffuseColor, vec3 specularColor) {
	vec3 halfLV = normalize(light + view);
	float spe = pow(max(dot(normal, halfLV), 0.0), 32.0);
	float dif = dot(normal, light) * 0.5 + 0.75;
	return dif*diffuseColor + spe*specularColor;
}

/*
** Contrast, saturation, brightness
** Code of this function is from TGM's shader pack
** http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?t=21057
*/

// For all settings: 1.0 = 100% 0.5=50% 1.5 = 150%
vec3 ContrastSaturationBrightness(vec3 color, float con, float sat, float brt)
{
	// Increase or decrease theese values to adjust r, g and b color channels seperately
	const float AvgLumR = 0.5;
	const float AvgLumG = 0.5;
	const float AvgLumB = 0.5;
	
	const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
	
	vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
	vec3 brtColor = color * brt;
	vec3 intensity = vec3(dot(brtColor, LumCoeff));
	vec3 satColor = mix(intensity, brtColor, sat);
	vec3 conColor = mix(AvgLumin, satColor, con);
	return conColor;
}

#ifdef __EXPERIMENTAL_AO__
float linearize(float depth)
{
#if 0
	float d = depth;
	d /= u_ViewInfo.z - depth * u_ViewInfo.z + depth;
	return clamp(d * u_ViewInfo.r, 0.0, 1.0);
#else
	return depth;
#endif
}

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

float ssao( in vec3 position, in vec2 pixel, in vec3 normal, in vec3 light, in int numOcclusionChecks, in float resolution, in float strength, in float minDistance, in float maxDisance )
{
    vec2  uv  = pixel;
    float z   = linearize(texture2D( u_ScreenDepthMap, uv ).x);		// read eye linear z
	vec2  res = vec2(resolution) / u_Dimensions.xy;

	//vec3  ref = texture2D( u_DeluxeMap, uv ).xyz;					// read dithering vector
	
	vLocalSeed = position;
	vec3 ref = vec3(randZeroOne(), randZeroOne(), randZeroOne());
	//float rnd = randZeroOne();

	if (z >= 1.0) return 1.0;

    // accumulate occlusion
    float bl = 0.0;
    for( int i=0; i<numOcclusionChecks; i++ )
    {
		vec3  of = faceforward( reflect( unKernel[i], ref ), light, normal );
		//vec3  of = faceforward( reflect( unKernel[i], light ), light, normal );
		//vec3  of = reflect( unKernel[i]*-light, normal+ref );
        float sz = linearize(texture2D( u_ScreenDepthMap, uv + (res * of.xy)).x);
        float zd = (sz-z)*strength;

		if (length(sz - z) < minDistance || length(sz - z) > maxDisance)
			bl += 1.0;
		else
			bl += clamp(zd*10.0,0.1,1.0)*(1.0-clamp((zd-1.0)/5.0,0.0,1.0));
    }

	float ao = clamp(1.0*bl/float(numOcclusionChecks), 0.0, 1.0);
	ao = mix(ao, 1.0, z);
    return ao;
}
#endif //__EXPERIMENTAL_AO__

vec3 EnvironmentBRDF(float gloss, float NE, vec3 specular)
{
	vec4 t = vec4( 1/0.96, 0.475, (0.0275 - 0.25 * 0.04)/0.96,0.25 ) * gloss;
	t += vec4( 0.0, 0.0, (0.015 - 0.75 * 0.04)/0.96,0.75 );
	float a0 = t.x * min( t.y, exp2( -9.28 * NE ) ) + t.z;
	float a1 = t.w;
	return clamp( a0 + specular * ( a1 - a0 ), 0.0, 1.0 );
}

void main(void)
{
	vec4 color = textureLod(u_DiffuseMap, var_TexCoords, 0.0);
	vec4 position = textureLod(u_PositionMap, var_TexCoords, 0.0);
	vec4 outColor = vec4(color.rgb, 1.0);

	if (position.a-1.0 == MATERIAL_SKY || position.a-1.0 == MATERIAL_SUN || position.a-1.0 == MATERIAL_NONE)
	{// Skybox... Skip...
		if (!(u_Local5.r == 1.0 && u_Local5.g == 1.0 && u_Local5.b == 1.0))
		{// C/S/B enabled...
			outColor.rgb = ContrastSaturationBrightness(outColor.rgb, u_Local5.r, u_Local5.g, u_Local5.b);
		}

		if (u_Local5.a > 0.0)
		{// TrueHDR enabled...
			outColor.rgb = TrueHDR( outColor.rgb );
		}

		if (u_Local6.b > 0.0)
		{// Vibrancy enabled...
			outColor.rgb = Vibrancy( outColor.rgb, u_Local6.b );
		}

		//outColor.rgb = LevelsControlOutputRange(outColor.rgb, 0.0, 1.0);
		gl_FragColor = outColor;
		return;
	}


	vec2 texCoords = var_TexCoords;
	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);
	vec3 materialSettings = RB_PBR_DefaultsForMaterial(position.a-1.0);


	if (u_Local6.a > 0.0)
	{// Sunset, Sunrise, and Night times... Scale down screen color, before adding lighting...
		vec3 nightColor = vec3(outColor.rgb * 0.35);
		outColor.rgb = mix(outColor.rgb, nightColor, u_Local6.a);
	}



#if 0
	if (u_Local3.r > 0.0)
	{// Debug position map by showing pixel distance from view...
		float dist = distance(u_ViewOrigin.xyz, position.xyz);
		float d = clamp(dist / u_Local3.r, 0.0, 1.0);
		outColor.rgb = vec3(d);
		outColor.a = 1.0;
		gl_FragColor = outColor;
		return;
	}
#endif

	vec4 norm = textureLod(u_NormalMap, texCoords, 0.0);
	vec4 normalDetail = textureLod(u_OverlayMap, texCoords, 0.0);

	norm.rgb = normalize(norm.rgb * 2.0 - 1.0);
	norm.z = sqrt(1.0-dot(norm.xy, norm.xy)); // reconstruct Z from X and Y

	vec3 cubeNorm = norm.rgb;
	
	if (normalDetail.a < 1.0)
	{// Don't have real normalmap, make normals for this pixel...
		normalDetail = normalVector(texCoords);
	}

	normalDetail.rgb = normalize(normalDetail.rgb * 2.0 - 1.0);
	normalDetail.rgb *= 0.25;
	//normalDetail.rgb *= u_Local3.r;
	//normalDetail.z = sqrt(clamp((0.25 - normalDetail.x * normalDetail.x) - normalDetail.y * normalDetail.y, 0.0, 1.0));
	norm.rgb = normalize(norm.rgb + normalDetail.rgb);
	//norm.rgb = normalize(norm.rgb * ((normalDetail.rgb * 0.5 + 0.5) * 0.25 + 0.75));

	//vec3 tangent = TangentFromNormal( norm.xyz );
	//vec3 bitangent = normalize( cross(norm.xyz, tangent) );
	//mat3 tangentToWorld = mat3(tangent.xyz, bitangent.xyz, norm.xyz);
	//norm.xyz = tangentToWorld * normalDetail.xyz;

	vec3 N = norm.xyz;


	vec3 to_light = position.xyz - u_PrimaryLightOrigin.xyz;
	float to_light_dist = length(to_light);
	vec3 to_light_norm = (to_light / to_light_dist);

	vec3 to_pos = u_ViewOrigin.xyz - position.xyz;
	float to_pos_dist = length(to_pos);
	vec3 to_pos_norm = (to_pos / to_pos_dist);

	#define rd reflect(surfaceToCamera, N)


	vec3 specular = vec3(0.0);
	vec3 reflectance = vec3(0.0);
	float specularPower = clamp(materialSettings.x * 0.04, 0.0, 1.0);
	float gloss = clamp(clamp((materialSettings.x + materialSettings.y) * 0.5, 0.0, 1.0) * 1.6, 0.0, 1.0);

	if (specularPower > 0.0)
	{
#define specLower ( 48.0 / 255.0)
#define specUpper (255.0 / 192.0)
		specular = clamp((clamp(outColor.rgb - specLower, 0.0, 1.0)) * specUpper, 0.0, 1.0);

		float NE = clamp(length(dot(N, E)), 0.0, 1.0);
		vec3 reflectance = EnvironmentBRDF(gloss, NE, specular.rgb);

		outColor.rgb = mix(outColor.rgb, reflectance.rgb, specularPower);

		if (u_Local7.r > 0.0)
		{// Cubemaps enabled...
			if (materialSettings.x > 0.0 && materialSettings.y > 0.0)
			{
				float curDist = distance(u_ViewOrigin.xyz, position.xyz);
				float cubeDist = 0.0;//distance(u_CubeMapInfo.xyz, position.xyz);
				float cubeFade = 0.0;
		
				if (curDist < u_Local7.g && cubeDist < u_Local7.g)
				{
					cubeFade = clamp((1.0 - clamp(curDist / u_Local7.g, 0.0, 1.0)) * (1.0 - clamp(cubeDist / u_Local7.g, 0.0, 1.0)) * materialSettings.y * u_CubeMapStrength, 0.0, 1.0);
				}

				if (cubeFade > 0.0)
				{
					vec3 m_ViewDir = to_pos_norm;
					vec3 R = reflect(E, cubeNorm);
					
					// This used to be done in rend2 code, now done here because I need u_CubeMapInfo.xyz to be cube origin for distance checks above... u_CubeMapInfo.w is now radius.
					vec4 cubeInfo = u_CubeMapInfo;
					cubeInfo.xyz -= u_ViewOrigin.xyz;

					cubeInfo.w = pow(distance(u_ViewOrigin.xyz, u_CubeMapInfo.xyz), 3.0/*u_Local3.r*/);

					cubeInfo.xyz *= 1.0 / cubeInfo.w;
					cubeInfo.w = 1.0 / cubeInfo.w;
					
					vec3 parallax = cubeInfo.xyz + cubeInfo.w * m_ViewDir;
					parallax.z *= -1.0;
					
					//vec3 cubeLightColor = textureLod(u_CubeMap, R + parallax, 7.0 - gloss * 7.0).rgb;
					vec3 cubeLightColor = texture(u_CubeMap, R + parallax).rgb;

					float cPx = 1.0 / u_Local7.b;
					float blurCount = 1.0;
					for (float x = -1.0; x <= 1.0; x += 1.0)
					{
						for (float y = -1.0; y <= 1.0; y += 1.0)
						{
							vec3 blurColor = texture(u_CubeMap, (R + parallax) + (vec3(x,y,0.0) * cPx)).rgb;
#define iblLower ( 128.0 / 255.0)
#define iblUpper (255.0 / 156.0)
							blurColor = clamp((clamp(blurColor.rgb - iblLower, 0.0, 1.0)) * iblUpper, 0.0, 1.0); // Darken dark, lighten light...
							cubeLightColor += blurColor;
							blurCount += 1.0;
						}
					}
					cubeLightColor.rgb /= blurCount;

					// Maybe if not metal, here, we should add contrast to only show the brights as reflection...
					outColor.rgb = mix(outColor.rgb, outColor.rgb + (cubeLightColor * reflectance), clamp(cubeFade * gloss, 0.0, 1.0));
				}
			}
		}
	}

	float shadowMult = 1.0;

#if defined(USE_SHADOWMAP)
	if (u_Local2.g > 0.0 && u_Local6.a < 1.0)
	{
		float shadowValue = texture(u_ShadowMap, texCoords).r;

		shadowValue = pow(shadowValue, 1.5);

#define sm_cont_1 ( 64.0 / 255.0)
#define sm_cont_2 (255.0 / 200.0)
		shadowValue = clamp((clamp(shadowValue - sm_cont_1, 0.0, 1.0)) * sm_cont_2, 0.0, 1.0);
		float finalShadow = clamp(shadowValue + u_Local2.b, u_Local2.b, u_Local2.a);
		finalShadow = mix(finalShadow, 1.0, u_Local6.a); // Dampen out shadows at sunrise/sunset...
		outColor.rgb *= finalShadow;
		shadowMult = clamp(shadowValue, 0.2, 1.0) * 0.75 + 0.25;
		shadowValue = mix(shadowValue, 1.0, u_Local6.a); // Dampen out shadows at sunrise/sunset...
	}
#endif //defined(USE_SHADOWMAP)


	vec3 surfaceToCamera = E;

	vec3 PrimaryLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);

	vec4 occlusion = vec4(0.0);
	bool useOcclusion = false;

	if (u_Local2.r == 1.0)
	{
		useOcclusion = true;
		occlusion = texture(u_HeightMap, texCoords);
	}

	float reflectivePower = materialSettings.x;


	if (u_Local1.r > 0.0)
	{// If r_blinnPhong is <= 0.0 then this is pointless...
		float phongFactor = u_Local1.r * u_Local1.g;

#define LIGHT_COLOR_POWER			4.0

		if (phongFactor > 0.0 && u_Local6.a < 1.0)
		{// this is blinn phong
			float light_occlusion = 1.0;

			if (useOcclusion)
			{
				light_occlusion = 1.0 - clamp(dot(vec4(-to_light_norm*E, 1.0), occlusion), 0.0, 1.0);
			}

			float power = clamp(length(outColor.rgb) / 3.0, 0.0, 1.0) * 0.5 + 0.5;
			power = pow(power, LIGHT_COLOR_POWER);
			power = power * 0.5 + 0.5;
		
			vec3 lightColor = u_PrimaryLightColor.rgb;

			float lightMult = clamp(reflectivePower * power * light_occlusion * phongFactor * shadowMult, 0.0, 1.0);

			if (lightMult > 0.0)
			{
				lightColor *= lightMult;
				lightColor = blinn_phong(N, E, -to_light_norm, lightColor, lightColor);
				float maxStr = max(outColor.r, max(outColor.g, outColor.b)) * 0.9 + 0.1;
				lightColor *= maxStr;
				lightColor *= clamp(1.0 - u_Local6.a, 0.0, 1.0); // Day->Night scaling of sunlight...

				// Add vibrancy to light color at sunset/sunrise???
				if (u_Local6.a > 0.0 && u_Local6.a < 1.0)
				{// Vibrancy gets greater the closer we get to night time...
					float vib = u_Local6.a;
					if (vib > 0.8)
					{// Scale back vibrancy to 0.0 just before nightfall...
						float downScale = 1.0 - vib;
						downScale *= 4.0;
						vib = mix(vib, 0.0, downScale);
					}
					lightColor = Vibrancy( lightColor, vib * 4.0 );
				}

				outColor.rgb = outColor.rgb + max(lightColor, vec3(0.0));
			}
		}

		if (u_lightCount > 0.0 && reflectivePower > 0.0)
		{
			phongFactor = u_Local1.r;

			if (phongFactor <= 0.0)
			{// Never allow no phong...
				phongFactor = 1.0;
			}

			vec3 addedLight = vec3(0.0);
			float power = clamp(length(outColor.rgb) / 3.0, 0.0, 1.0) * 0.5 + 0.5;
			power = pow(power, LIGHT_COLOR_POWER);
			power = power * 0.5 + 0.5;

			float maxStr = max(outColor.r, max(outColor.g, outColor.b)) * 0.9 + 0.1;

			for (int li = 0; li < u_lightCount; li++)
			{
				vec3 lightPos = u_lightPositions2[li].xyz;

				float lightDist = distance(lightPos, position.xyz);

				if (u_lightHeightScales[li] > 0.0)
				{// ignore height differences, check later...
					lightDist -= length(lightPos.z - position.z);
				}

				float lightDistMult = 1.0 - clamp((distance(lightPos.xyz, u_ViewOrigin.xyz) / 4096.0), 0.0, 1.0);
				//float lightStrength = pow(1.0 - clamp(lightDist / u_lightDistances[li], 0.0, 1.0), 2.0);

				// Attenuation...
				float lightStrength = 1.0 - clamp((lightDist * lightDist) / (u_lightDistances[li] * u_lightDistances[li]), 0.0, 1.0);
				lightStrength = pow(lightStrength, 2.0);
				lightStrength *= lightDistMult * reflectivePower;

				if (lightStrength > 0.0)
				{
					vec3 lightColor = (u_lightColors[li].rgb / length(u_lightColors[li].rgb)) * u_Local4.a; // Normalize.
					vec3 lightDir = normalize(lightPos - position.xyz);
					float light_occlusion = 1.0;
				
					lightColor = lightColor * lightStrength * power;

					//addedLight.rgb += lightColor;
					addedLight.rgb += lightColor * maxStr * 0.333;

					if (useOcclusion)
					{
						light_occlusion = (1.0 - clamp(dot(vec4(-lightDir*E, 1.0), occlusion), 0.0, 1.0));
					}

					float lightMult = clamp(light_occlusion * phongFactor, 0.0, 1.0);

					if (lightMult > 0.0)
					{
						lightColor *= lightMult;
						lightColor *= maxStr;
						addedLight.rgb += blinn_phong(N, E, lightDir, lightColor, lightColor);
					}
				}
			}

			outColor.rgb = outColor.rgb + max(addedLight, vec3(0.0));
		}
	}

	// Has a sort of lightmap-ish type coloring/lighting effect on result...
	//outColor.rgb *= ((outColor.rgb / length(outColor.rgb)) * 3.0) * 0.25 + 0.75;

#ifdef __AMBIENT_OCCLUSION__
	if (u_Local1.b == 1.0)
	{// AO enabled...
		float ao = calculateAO(to_light_norm, N * 10000.0);
		ao = clamp(ao * u_Local6.g + u_Local6.r, u_Local6.r, 1.0);
		outColor.rgb *= ao;
	}
#endif //__AMBIENT_OCCLUSION__

#ifdef __EXPERIMENTAL_AO__
	if (u_Local1.b >= 2.0)
	{// HQ AO enabled...
		float hsao = 1.0; // High frequency occlusion...
		float msao = 1.0; // Mid frequency occlusion...
		float lsao = 1.0; // Low frequency occlusion...

		if (u_Local1.b == 2.0 || u_Local1.b >= 5.0)
			msao = ssao( position.xyz, texCoords, N.xyz, to_light_norm, 16, 32.0, 64.0, 0.001, 0.01 );

		if (u_Local1.b == 3.0 || u_Local1.b >= 5.0)
			lsao = ssao( position.xyz, texCoords, N.xyz, to_light_norm, 8, 64.0, 4.0, 0.0001, 1.0 ) * 0.25 + 0.75;
		
		//if (u_Local1.b == 4.0 || u_Local1.b >= 5.0)
		//	hsao = ssao( position.xyz, texCoords, N.xyz, to_light_norm, 4, 16.0, 64.0, 0.0008, 0.007 ) * 0.5 + 0.5;

		float sao = clamp(msao * hsao * lsao, 0.0, 1.0);
		sao = clamp(sao * u_Local6.g + u_Local6.r, u_Local6.r, 1.0);

		//if (u_Local3.a > 0.0)
		//	outColor.rgb = vec3(sao);
		//else
			outColor.rgb *= sao;
	}
#endif //__EXPERIMENTAL_AO__

#ifdef __ENVMAP__
	if (u_Local1.a > 0.0)
	{// Envmap enabled...
		float lightScale = clamp(1.0 - clamp(max(max(outColor.r, outColor.g), outColor.b), 0.0, 1.0), 0.0, 1.0);
		float invLightScale = clamp((1.0 - lightScale) * 1.2, 0.2, 1.0);
		vec3 env = envMap(rd, 0.6 /* warmth */);
		outColor.rgb = mix(outColor.rgb, outColor.rgb + ((env * (reflectivePower * 0.5) * invLightScale) * lightScale), (reflectivePower * 0.5) * lightScale * gloss);
	}
#endif //__ENVMAP__

	if (!(u_Local5.r == 1.0 && u_Local5.g == 1.0 && u_Local5.b == 1.0))
	{// C/S/B enabled...
		outColor.rgb = ContrastSaturationBrightness(outColor.rgb, u_Local5.r, u_Local5.g, u_Local5.b);
	}

	if (u_Local5.a > 0.0)
	{// TrueHDR enabled...
		outColor.rgb = TrueHDR( outColor.rgb );
	}

	if (u_Local6.b > 0.0)
	{// Vibrancy enabled...
		outColor.rgb = Vibrancy( outColor.rgb, u_Local6.b );
	}

	//outColor.rgb = LevelsControlOutputRange(outColor.rgb, 0.0, 1.0);
	gl_FragColor = outColor;
}

