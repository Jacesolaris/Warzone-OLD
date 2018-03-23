#ifndef __LQ_MODE__

#define __FAST_NORMAL_DETAIL__
#define __AMBIENT_OCCLUSION__
#define __ENHANCED_AO__
//#define __ENVMAP__
#define __RANDOMIZE_LIGHT_PIXELS__
#define __SCREEN_SPACE_REFLECTIONS__
//#define __HEIGHTMAP_SHADOWS__
#define __FAST_LIGHTING__
//#define __IRRADIANCE__

#ifdef USE_CUBEMAPS
	#define __CUBEMAPS__

	#ifdef USE_EMISSIVECUBES
		#define __EMISSIVE_IBL__
	#endif //USE_EMISSIVECUBES
#endif //USE_CUBEMAPS

#endif //__LQ_MODE__

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
uniform samplerCube	u_SkyCubeMap;
uniform samplerCube	u_SkyCubeMapNight;
uniform samplerCube	u_EmissiveCubeMap;
uniform sampler2D	u_SteepMap;	  // ssao image
uniform sampler2D	u_WaterEdgeMap; // sky up image (for sky lighting contribution)
uniform sampler2D	u_RoadsControlMap; // sky night up image (for sky lighting contribution)

uniform mat4		u_ModelViewProjectionMatrix;

uniform vec2		u_Dimensions;

uniform vec4		u_Local1; // r_blinnPhong, SUN_PHONG_SCALE, r_ao, r_env
uniform vec4		u_Local2; // SSDO, SHADOWS_ENABLED, SHADOW_MINBRIGHT, SHADOW_MAXBRIGHT
uniform vec4		u_Local3; // r_testShaderValue1, r_testShaderValue2, r_testShaderValue3, r_testShaderValue4
uniform vec4		u_Local4; // MAP_INFO_MAXSIZE, MAP_WATER_LEVEL, floatTime, MAP_EMISSIVE_COLOR_SCALE
uniform vec4		u_Local5; // CONTRAST, SATURATION, BRIGHTNESS, TRUEHDR_ENABLED
uniform vec4		u_Local6; // AO_MINBRIGHT, AO_MULTBRIGHT, VIBRANCY, NightScale
uniform vec4		u_Local7; // cubemapEnabled, r_cubemapCullRange, r_cubeMapSize, r_skyLightContribution
uniform vec4		u_Local8; // enableReflections, MAP_HDR_MIN, MAP_HDR_MAX, MAP_INFO_PLAYABLE_SIZE[2]
uniform vec4		u_Local9; // haveEmissiveCube, MAP_USE_PALETTE_ON_SKY, 0.0, 0.0

uniform vec4		u_ViewInfo; // znear, zfar, zfar / znear, fov
uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

uniform vec4		u_CubeMapInfo;
uniform float		u_CubeMapStrength;

uniform int			u_lightCount;
#if !defined(__LQ_MODE__) && defined(__LIGHT_OCCLUSION__)
uniform vec2		u_lightPositions[MAX_DEFERRED_LIGHTS];
#endif
uniform vec3		u_lightPositions2[MAX_DEFERRED_LIGHTS];
uniform float		u_lightDistances[MAX_DEFERRED_LIGHTS];
uniform float		u_lightHeightScales[MAX_DEFERRED_LIGHTS];
uniform vec3		u_lightColors[MAX_DEFERRED_LIGHTS];

varying vec2		var_TexCoords;


vec2 pixel = vec2(1.0) / u_Dimensions;

#define hdr_const_1 (u_Local8.g / 255.0)
#define hdr_const_2 (255.0 / u_Local8.b)

vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N*4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);

	return vec3(encoded * g, 1.0 - f * 0.5);
}


vec2 RB_PBR_DefaultsForMaterial(float MATERIAL_TYPE)
{// I probably should use an array of const's instead, but this will do for now...
	vec2 settings = vec2(0.0);

	float specularReflectionScale = 0.0;
	float cubeReflectionScale = 0.0;

	switch (int(MATERIAL_TYPE))
	{
	case MATERIAL_WATER:			// 13			// light covering of water on a surface
		specularReflectionScale = 1.0;
		cubeReflectionScale = 0.7;
		break;
	case MATERIAL_SHORTGRASS:		// 5			// manicured lawn
		specularReflectionScale = 0.0055;
		cubeReflectionScale = 0.35;
		break;
	case MATERIAL_LONGGRASS:		// 6			// long jungle grass
		specularReflectionScale = 0.0065;
		cubeReflectionScale = 0.35;
		break;
	case MATERIAL_SAND:				// 8			// sandy beach
		specularReflectionScale = 0.0055;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_CARPET:			// 27			// lush carpet
		specularReflectionScale = 0.0015;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_GRAVEL:			// 9			// lots of small stones
		specularReflectionScale = 0.0015;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_ROCK:				// 23			//
		specularReflectionScale = 0.002;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_TILES:			// 26			// tiled floor
		specularReflectionScale = 0.026;
		cubeReflectionScale = 0.15;
		break;
	case MATERIAL_SOLIDWOOD:		// 1			// freshly cut timber
		specularReflectionScale = 0.0015;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_HOLLOWWOOD:		// 2			// termite infested creaky wood
		specularReflectionScale = 0.00075;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_SOLIDMETAL:		// 3			// solid girders
		specularReflectionScale = 0.98;
		cubeReflectionScale = 0.98;
		break;
	case MATERIAL_HOLLOWMETAL:		// 4			// hollow metal machines -- UQ1: Used for weapons to force lower parallax and high reflection...
		specularReflectionScale = 1.0;
		cubeReflectionScale = 0.98;
		break;
	case MATERIAL_DRYLEAVES:		// 19			// dried up leaves on the floor
		specularReflectionScale = 0.0026;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_GREENLEAVES:		// 20			// fresh leaves still on a tree
		specularReflectionScale = 0.0055;
		cubeReflectionScale = 0.35;
		break;
	case MATERIAL_FABRIC:			// 21			// Cotton sheets
		specularReflectionScale = 0.0055;
		cubeReflectionScale = 0.35;
		break;
	case MATERIAL_CANVAS:			// 22			// tent material
		specularReflectionScale = 0.0045;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_MARBLE:			// 12			// marble floors
		specularReflectionScale = 0.025;
		cubeReflectionScale = 0.46;
		break;
	case MATERIAL_SNOW:				// 14			// freshly laid snow
		specularReflectionScale = 0.025;
		cubeReflectionScale = 0.65;
		break;
	case MATERIAL_MUD:				// 17			// wet soil
		specularReflectionScale = 0.003;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_DIRT:				// 7			// hard mud
		specularReflectionScale = 0.002;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_CONCRETE:			// 11			// hardened concrete pavement
		specularReflectionScale = 0.00175;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_FLESH:			// 16			// hung meat, corpses in the world
		specularReflectionScale = 0.0045;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_RUBBER:			// 24			// hard tire like rubber
		specularReflectionScale = 0.0015;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_PLASTIC:			// 25			//
		specularReflectionScale = 0.028;
		cubeReflectionScale = 0.48;
		break;
	case MATERIAL_PLASTER:			// 28			// drywall style plaster
		specularReflectionScale = 0.0025;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_SHATTERGLASS:		// 29			// glass with the Crisis Zone style shattering
		specularReflectionScale = 0.025;
		cubeReflectionScale = 0.67;
		break;
	case MATERIAL_ARMOR:			// 30			// body armor
		specularReflectionScale = 0.055;
		cubeReflectionScale = 0.66;
		break;
	case MATERIAL_ICE:				// 15			// packed snow/solid ice
		specularReflectionScale = 0.045;
		cubeReflectionScale = 0.78;
		break;
	case MATERIAL_GLASS:			// 10			//
		specularReflectionScale = 0.035;
		cubeReflectionScale = 0.70;
		break;
	case MATERIAL_BPGLASS:			// 18			// bulletproof glass
		specularReflectionScale = 0.033;
		cubeReflectionScale = 0.70;
		break;
	case MATERIAL_COMPUTER:			// 31			// computers/electronic equipment
		specularReflectionScale = 0.042;
		cubeReflectionScale = 0.68;
		break;
	case MATERIAL_PUDDLE:
		specularReflectionScale = 1.0;
		cubeReflectionScale = 0.7;
		break;
	case MATERIAL_EFX:
	case MATERIAL_BLASTERBOLT:
	case MATERIAL_FIRE:
	case MATERIAL_SMOKE:
		specularReflectionScale = 0.0;
		cubeReflectionScale = 0.0;
		break;
	default:
		specularReflectionScale = 0.0075;
		cubeReflectionScale = 0.2;
		break;
	}

	// TODO: Update original values with these modifications that I added after... Save time on the math, even though it's minor...
	//specularReflectionScale = specularReflectionScale * 0.5 + 0.5;
	cubeReflectionScale = cubeReflectionScale * 0.75 + 0.25;

	settings.x = specularReflectionScale;
	settings.y = cubeReflectionScale;

	return settings;
}

#ifdef __EMISSIVE_IBL__
float saturate(in float val) {
	return clamp(val, 0.0, 1.0);
}

vec3 saturate(in vec3 val) {
	return clamp(val, vec3(0.0), vec3(1.0));
}
#endif //__EMISSIVE_IBL__

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

#if defined(__SCREEN_SPACE_REFLECTIONS__)
#define pw pixel.x
#define ph pixel.y
vec3 AddReflection(vec2 coord, vec4 positionMap, vec3 flatNorm, vec3 inColor, float reflectiveness)
{
	if (reflectiveness <= 0.5)
	{// Top of screen pixel is water, don't check...
		return inColor;
	}

	float pixelDistance = distance(positionMap.xyz, u_ViewOrigin.xyz);

	//const float scanSpeed = 48.0;// 16.0;// 5.0; // How many pixels to scan by on the 1st rough pass...
	//float scanSpeed = u_Local3.r;
	const float scanSpeed = 16.0;

	// Quick scan for pixel that is not water...
	float QLAND_Y = 0.0;

	for (float y = coord.y; y <= 1.0; y += ph * scanSpeed)
	{
		vec3 norm = DecodeNormal(textureLod(u_NormalMap, vec2(coord.x, y), 0.0).xy);
		vec4 pMap = textureLod(u_PositionMap, vec2(coord.x, y), 0.0);

		float pMapDistance = distance(pMap.xyz, u_ViewOrigin.xyz);

		if (pMap.a > 1.0 && pMap.xyz != vec3(0.0) && distance(pMap.xyz, u_ViewOrigin.xyz) <= pixelDistance)
		{
			continue;
		}

		if (norm.z * 0.5 + 0.5 < 0.75 && distance(norm.xyz, flatNorm.xyz) > 0.0)
		{
			QLAND_Y = y;
			break;
		}
		else if (positionMap.a != pMap.a && pMap.a == 0.0)
		{
			QLAND_Y = y;
			break;
		}
	}

	if (QLAND_Y <= 0.0 || QLAND_Y >= 1.0)
	{// Found no non-water surfaces...
		return inColor;
	}
	
	QLAND_Y -= ph * scanSpeed;
	
	// Full scan from within 5 px for the real 1st pixel...
	float upPos = coord.y;
	float LAND_Y = 0.0;

	for (float y = QLAND_Y; y <= QLAND_Y + (ph * scanSpeed); y += ph)
	{
		vec3 norm = DecodeNormal(textureLod(u_NormalMap, vec2(coord.x, y), 0.0).xy);
		vec4 pMap = textureLod(u_PositionMap, vec2(coord.x, y), 0.0);
		
		float pMapDistance = distance(pMap.xyz, u_ViewOrigin.xyz);

		if (pMap.a > 1.0 && pMap.xyz != vec3(0.0) && distance(pMap.xyz, u_ViewOrigin.xyz) <= pixelDistance)
		{
			continue;
		}

		if (norm.z * 0.5 + 0.5 < 0.75 && distance(norm.xyz, flatNorm.xyz) > 0.0)
		{
			LAND_Y = y;
			break;
		}
		else if (positionMap.a != pMap.a && pMap.a == 0.0)
		{
			LAND_Y = y;
			break;
		}
	}

	if (LAND_Y <= 0.0 || LAND_Y >= 1.0)
	{// Found no non-water surfaces...
		return inColor;
	}

	//float finalMaterial = textureLod(u_PositionMap, vec2(coord.x, LAND_Y), 0.0).a;
	//float finalNormal = textureLod(u_NormalMap, vec2(coord.x, LAND_Y), 0.0).a;

	float d = 1.0 / ((1.0 - (LAND_Y - coord.y)) * 1.75);

	upPos = clamp(coord.y + ((LAND_Y - coord.y) * 2.0 * d), 0.0, 1.0);

	if (upPos > 1.0 || upPos < 0.0)
	{// Not on screen...
		return inColor;
	}

	vec4 pMap = textureLod(u_PositionMap, vec2(coord.x, upPos), 0.0);

	if (pMap.a > 1.0 && pMap.xyz != vec3(0.0) && distance(pMap.xyz, u_ViewOrigin.xyz) <= pixelDistance)
	{// The reflected pixel is closer then the original, this would be a bad reflection.
		return inColor;
	}

	float heightDiff = clamp(distance(upPos, coord.y) * 3.0, 0.0, 1.0);
	float heightStrength = 1.0 - clamp(pow(heightDiff, 0.025), 0.0, 1.0);

	float strength = 1.0 - clamp(pow(upPos, 4.0), 0.0, 1.0);
	strength *= heightStrength;
	strength = clamp(strength, 0.0, 1.0);

	if (strength <= 0.0)
	{
		return inColor;
	}

	vec3 glowColor = textureLod(u_GlowMap, vec2(coord.x, upPos), 0.0).rgb;
	vec3 landColor = textureLod(u_DiffuseMap, vec2(coord.x, upPos), 0.0).rgb;
	return mix(inColor.rgb, inColor.rgb + landColor.rgb + (glowColor.rgb * 3.5), clamp(strength * reflectiveness * 4.0, 0.0, 1.0));
}
#endif //defined(__SCREEN_SPACE_REFLECTIONS__)

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
	vec4 normals = bumpFromDepth(coord, u_Dimensions, 0.1 /*scale*/);
	normals.r = 1.0 - normals.r;
	normals.g = 1.0 - normals.g;
	normals.b = 1.0 - normals.b;
	return normals;
}
#endif //__FAST_NORMAL_DETAIL__

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


#if defined(__AMBIENT_OCCLUSION__)
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
#endif //defined(__AMBIENT_OCCLUSION__)

#if defined(__ENVMAP__)
vec3 envMap(vec3 p, float warmth)
{
    float c = cellTile(p*6.);
    c = smoothstep(0.2, 1., c); // Contract gives it more of a lit look... kind of.
    
	// Icy glow... for whatever reason.
    vec3 coolMap = vec3(pow(c, 8.), c*c, c);
    
	// Alternate firey glow.
    vec3 heatMap = vec3(min(c*1.5, 1.), pow(c, 2.5), pow(c, 12.));

	// Mix Ice and Heat based on warmth setting...
	return mix(coolMap, heatMap, clamp(warmth, 0.0, 1.0));
}
#endif //defined(__ENVMAP__)


#if defined(__HEIGHTMAP_SHADOWS__)
float adjustHeightZeroOne(float height)
{
	return (height / u_Local8.a) * 0.5 + 0.5;
}

float getHeightmap(vec2 uv)
{
	vec2 p = texture(u_PositionMap, uv).zw;

	float material = p.y - 1.0;

	if (material == MATERIAL_SKY || material == MATERIAL_SUN)
	{
		return -1.0;
	}

	float height = p.x;
	return adjustHeightZeroOne(height);
}

float GetShadow(vec2 uv, vec3 lp, vec3 lDir) {
	const int steps = 8;// int(u_Local3.g);// 64;
	const float invSteps = 1.0 / float(steps);
	//float bias = 0.01;

	//vec3 lightOrg = u_ViewOrigin.xzy - lp.xzy;
	//lightOrg.x = adjustHeightZeroOne(lightOrg.x);
	//lightOrg.y = adjustHeightZeroOne(lightOrg.y);
	//lightOrg.z = adjustHeightZeroOne(lightOrg.z);
	//lightOrg = normalize(lightOrg);
	
	//vec3 lightOrg = normalize(vec3(0.0, 1.0, 0.0));

	vec3 lightOrg = vec3(0.0, -lDir.b, 0.0);
	if (lDir.g > 0.0)
		lightOrg.r = lDir.g;
	else if (lDir.b > 0.0)
		lightOrg.r = -lDir.r;

	//if (u_Local3.r == 6.0)
	//	return lightOrg.r;

	vec3 inc = lightOrg * invSteps;

	float heightmap = getHeightmap(uv);
	
	vec3 position = vec3(uv, heightmap);

	float shadow = 1.0;
	float k = 0.0;// u_Local3.b;
	float tmax = 8.0;// u_Local3.a;

	for (int i = 0; i < steps && position.z < 1.0 && position.y > 0.0 && position.y < 1.0 && position.x > 0.0 && position.x < 1.0; i++) {
		position += inc;

		float offsetHightmap = getHeightmap(position.xy);// -bias;

		//if (offsetHightmap > position.z)
		//{
		//	return 0.0;
		//}

		if (offsetHightmap != -1.0)
		{
			shadow = min(shadow, offsetHightmap*1.5);

			k += offsetHightmap;

			if (k > tmax)
			{
				break;
			}
		}
	}
	return shadow;
}
#endif //defined(__HEIGHTMAP_SHADOWS__)


vec3 TrueHDR ( vec3 color )
{
	return clamp((clamp(color.rgb - hdr_const_1, 0.0, 1.0)) * hdr_const_2, 0.0, 1.0);
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
#if defined(__LQ_MODE__) || defined(__FAST_LIGHTING__)
float getspecularLight(vec3 n, vec3 l, vec3 e, float s) {
	//float nrm = (s + 8.0) / (3.1415 * 8.0);
	float ndotl = clamp(max(dot(reflect(e, n), l), 0.0), 0.1, 1.0);
	return clamp(pow(ndotl, s), 0.1, 1.0);// * nrm;
}

float getdiffuse(vec3 n, vec3 l, float p) {
	float ndotl = clamp(dot(n, l), 0.5, 0.9);
	return pow(ndotl, p);
}

vec3 blinn_phong(vec3 pos, vec3 color, vec3 normal, vec3 view, vec3 light, vec3 diffuseColor, vec3 specularColor, float specPower, vec3 lightPos) {
	/*float fre = clamp(dot(normal, -view) + 1.0, 0.0, 1.0);
	vec3 diffuse = diffuseColor * getdiffuse(normal, light, 2.0);
	vec3 specular = specularColor * clamp(getspecularLight(normal, -light, view, 0.2) * fre * u_Local3.r, u_Local3.g, u_Local3.b);
	return diffuse + specular;*/

	// Ambient light.
	float ambience = 0.25;

	// Diffuse lighting.
	//float diff = max(dot(normal, light), 0.0);
	float diff = getdiffuse(normal, light, 2.0) * 16.0;

	// Specular lighting.
	float fre = clamp(pow(clamp(dot(normal, -view) + 1.0, 0.0, 1.0), -2.0), 2.0, 48.0);
	float spec = pow(max(dot(reflect(-light, normal), view), 0.0), 1.2);

	return (ambience * diffuseColor) + (diffuseColor * diff) + (specularColor * spec * fre);
}
#else //!defined(__LQ_MODE__) || defined(__FAST_LIGHTING__)
float specTrowbridgeReitz(float HoN, float a, float aP)
{
	float a2 = a * a;
	float aP2 = aP * aP;
	return (a2 * aP2) / pow(HoN * HoN * (a2 - 1.0) + 1.0, 2.0);
}

float visSchlickSmithMod(float NoL, float NoV, float r)
{
	float k = pow(r * 0.5 + 0.5, 2.0) * 0.5;
	float l = NoL * (1.0 - k) + k;
	float v = NoV * (1.0 - k) + k;
	return 1.0 / (4.0 * l * v);
}

float fresSchlickSmith(float HoV, float f0)
{
	return f0 + (1.0 - f0) * pow(1.0 - HoV, 5.0);
}

float sphereLight(vec3 pos, vec3 N, vec3 V, vec3 r, float f0, float roughness, float NoV, out float NoL, vec3 lightPos)
{
#define sphereRad 0.2//0.3

	vec3 L = normalize(lightPos - pos);
	vec3 centerToRay = dot(L, r) * r - L;
	vec3 closestPoint = L + centerToRay * clamp(sphereRad / length(centerToRay), 0.0, 1.0);
	vec3 l = normalize(closestPoint);
	vec3 h = normalize(V + l);


	NoL = clamp(dot(N, l), 0.0, 1.0);
	float HoN = clamp(dot(h, N), 0.0, 1.0);
	float HoV = dot(h, V);

	float distL = length(L);
	float alpha = roughness * roughness;
	float alphaPrime = clamp(sphereRad / (distL * 2.0) + alpha, 0.0, 1.0);

	float specD = specTrowbridgeReitz(HoN, alpha, alphaPrime);
	float specF = fresSchlickSmith(HoV, f0);
	float specV = visSchlickSmithMod(NoL, NoV, roughness);

	return specD * specF * specV * NoL;
}

float tubeLight(vec3 pos, vec3 N, vec3 V, vec3 r, float f0, float roughness, float NoV, out float NoL, vec3 tubeStart, vec3 tubeEnd)
{
	vec3 L0 = tubeStart - pos;
	vec3 L1 = tubeEnd - pos;
	float distL0 = length(L0);
	float distL1 = length(L1);

	float NoL0 = dot(L0, N) / (2.0 * distL0);
	float NoL1 = dot(L1, N) / (2.0 * distL1);
	NoL = (2.0 * clamp(NoL0 + NoL1, 0.0, 1.0))
		/ (distL0 * distL1 + dot(L0, L1) + 2.0);

	vec3 Ld = L1 - L0;
	float RoL0 = dot(r, L0);
	float RoLd = dot(r, Ld);
	float L0oLd = dot(L0, Ld);
	float distLd = length(Ld);
	float t = (RoL0 * RoLd - L0oLd)
		/ (distLd * distLd - RoLd * RoLd);

	float tubeRad = distLd;// 0.2

	vec3 closestPoint = L0 + Ld * clamp(t, 0.0, 1.0);
	vec3 centerToRay = dot(closestPoint, r) * r - closestPoint;
	closestPoint = closestPoint + centerToRay * clamp(tubeRad / length(centerToRay), 0.0, 1.0);
	vec3 l = normalize(closestPoint);
	vec3 h = normalize(V + l);

	float HoN = clamp(dot(h, N), 0.0, 1.0);
	float HoV = dot(h, V);

	float distLight = length(closestPoint);
	float alpha = roughness * roughness;
	float alphaPrime = clamp(tubeRad / (distLight * 2.0) + alpha, 0.0, 1.0);

	float specD = specTrowbridgeReitz(HoN, alpha, alphaPrime);
	float specF = fresSchlickSmith(HoV, f0);
	float specV = visSchlickSmithMod(NoL, NoV, roughness);

	return specD * specF * specV * NoL;
}

float getdiffuse(vec3 n, vec3 l, float p) {
	float ndotl = clamp(dot(n, l), 0.5, 0.9);
	return pow(ndotl, p);
}

vec3 blinn_phong(vec3 pos, vec3 color, vec3 normal, vec3 view, vec3 light, vec3 diffuseColor, vec3 specularColor, float specPower, vec3 lightPos) {
	float noise = texture(u_DeluxeMap, pos.xy).x * 0.5;
	noise += texture(u_DeluxeMap, pos.xy * 0.5).y;
	noise += texture(u_DeluxeMap, pos.xy * 0.25).z * 2.0;
	noise += texture(u_DeluxeMap, pos.xy * 0.125).w * 4.0;

	vec3 albedo = pow(color, vec3(2.2));
	albedo = mix(albedo, albedo * 1.3, noise * 0.35 - 1.0);
	float roughness = 0.7 - clamp(0.5 - dot(albedo, albedo), 0.05, 0.95);
	float f0 = 0.3;

#ifdef DISABLE_ALBEDO
	albedo = vec3(0.1);
#endif

#ifdef DISABLE_ROUGHNESS
	roughness = 0.05;
#endif

	vec3 v = view;
	float NoV = clamp(dot(normal, v), 0.0, 1.0);
	vec3 r = reflect(-v, normal);

#if 1
	float NdotLSphere;
	float specSph = clamp(sphereLight(pos, normal, v, r, f0, roughness, NoV, NdotLSphere, lightPos), 0.0, 0.2);
	vec3 spec = albedo * clamp(0.3183 * NdotLSphere+specSph, 0.0, 0.2);
#else
	float NdotLTube;
	float specTube = clamp(tubeLight(pos, normal, v, r, f0, roughness, NoV, NdotLTube, lightPosStart, lightPosEnd), 0.0, 0.2);
	vec3 spec = albedo * clamp(0.3183 * NdotLTube+specTube, 0.0, 0.2);
#endif

	spec = specularColor * (pow(spec, vec3(1.0 / 2.2))) * specPower * 64.0;

	vec3 diffuse = diffuseColor * getdiffuse(normal, light, 2.0);

	return diffuse + spec;
}
#endif //defined(__LQ_MODE__) || defined(__FAST_LIGHTING__)

#if !defined(__LQ_MODE__) && defined(__IRRADIANCE__)
vec3 computeIrradiance(vec3 n)
{
	// Coefficients for SH 03
	vec3 l_0_p0 = vec3(0.379727, 0.427857, 0.452654);
	vec3 l_1_n1 = vec3(0.288207, 0.358230, 0.414330);
	vec3 l_1_p0 = vec3(0.039812, 0.031627, 0.012003);
	vec3 l_1_p1 = vec3(-0.103013, -0.102729, -0.087898);
	vec3 l_2_n2 = vec3(-0.060510, -0.053534, -0.037656);
	vec3 l_2_n1 = vec3(0.008683, -0.013685, -0.045723);
	vec3 l_2_p0 = vec3(-0.092757, -0.124872, -0.152495);
	vec3 l_2_p1 = vec3(-0.059096, -0.052316, -0.038539);
	vec3 l_2_p2 = vec3(0.022220, -0.002188, -0.042826);

	vec3 irr = vec3(0.0);

	float c1 = 0.429043;
	float c2 = 0.511664;
	float c3 = 0.743125;
	float c4 = 0.886227;
	float c5 = 0.247708;

	irr += c1 * l_2_p2 * (n.x*n.x - n.y*n.y);
	irr += c3 * l_2_p0 * (n.z*n.z);
	irr += c4 * l_0_p0;
	irr -= c5 * l_2_p0;
	irr += 2.0 * c1 * (l_2_n2*n.x*n.y + l_2_p1*n.x*n.z + l_2_n1*n.y*n.z);
	irr += 2.0 * c2 * (l_1_p1*n.x + l_1_n1*n.y + l_1_p0*n.z);

	return vec3(irr);
}
#endif //!defined(__LQ_MODE__) && defined(__IRRADIANCE__)

#if !defined(__LQ_MODE__) && defined(__LIGHT_OCCLUSION__)
float checkVisibility(vec2 p1, vec2 p2) {
	float percent = 1.0;
	float iter = 0.1;
	float d1 = texture(u_ScreenDepthMap, p1).r;
	float d2 = texture(u_ScreenDepthMap, p2).r;
	for (int i = 1; i < 9; i++) {
		vec2 mx = mix(p1, p2, iter);
		float d = texture(u_ScreenDepthMap, mx).r;
		percent -= smoothstep(0.0, 0.9, max(0.0, mix(d1, d2, iter) - d));
		iter += 0.1;
	}
	return max(0.0, percent);
}
#endif //!defined(__LQ_MODE__) && defined(__LIGHT_OCCLUSION__)

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
	vec4 outColor = vec4(color.rgb, 1.0);
	vec4 position = textureLod(u_PositionMap, var_TexCoords, 0.0);

	if (position.a-1.0 == MATERIAL_SKY 
		|| position.a-1.0 == MATERIAL_SUN 
		|| position.a-1.0 == MATERIAL_GLASS
		|| position.a-1.0 == MATERIAL_EFX
		|| position.a-1.0 == MATERIAL_BLASTERBOLT
		|| position.a - 1.0 == MATERIAL_FIRE
		|| position.a - 1.0 == MATERIAL_SMOKE)
	{// Skybox... Skip...
		if (u_Local9.g > 0.0)
		{
			if (!(u_Local5.r == 1.0 && u_Local5.g == 1.0 && u_Local5.b == 1.0))
			{// C/S/B enabled...
				outColor.rgb = ContrastSaturationBrightness(outColor.rgb, u_Local5.r, u_Local5.g, u_Local5.b);
			}
		}

		if (u_Local5.a > 0.0)
		{// TrueHDR enabled...
			outColor.rgb = TrueHDR(outColor.rgb);
		}

		if (u_Local6.b > 0.0)
		{// Vibrancy enabled...
			outColor.rgb = Vibrancy(outColor.rgb, u_Local6.b);
		}

		outColor.rgb = clamp(outColor.rgb, 0.0, 1.0);
		gl_FragColor = outColor;
		return;
	}


	vec2 texCoords = var_TexCoords;
	vec2 materialSettings = RB_PBR_DefaultsForMaterial(position.a-1.0);
	bool isMetalic = (position.a - 1.0 == MATERIAL_SOLIDMETAL || position.a - 1.0 == MATERIAL_HOLLOWMETAL) ? true : false;

	//
	// Grab and set up our normal value, from lightall buffers, or fallback to offseting the flat normal buffer by pixel luminances, etc...
	//

	vec4 norm = textureLod(u_NormalMap, texCoords, 0.0);
	norm.xyz = DecodeNormal(norm.xy);

	vec3 flatNorm = normalize(norm.xyz);

#if defined(__SCREEN_SPACE_REFLECTIONS__)
	// If doing screen space reflections on floors, we need to know what are floor pixels...
	float ssReflection = norm.z * 0.5 + 0.5;

	// Allow only fairly flat surfaces for reflections (floors), and not roofs (for now)...
	if (ssReflection < 0.8)
	{
		ssReflection = 0.0;
	}
	else
	{
		ssReflection = clamp(ssReflection - 0.8, 0.0, 0.2) * 5.0;
	}
#endif //defined(__SCREEN_SPACE_REFLECTIONS__)

	// Now add detail offsets to the normal value...
	vec4 normalDetail = textureLod(u_OverlayMap, texCoords, 0.0);

	if (normalDetail.a < 1.0)
	{// If we don't have real normalmap, generate fallback normal offsets for this pixel from luminances...
#ifdef __FAST_NORMAL_DETAIL__
		normalDetail = normalVector(outColor.rgb);
#else //!__FAST_NORMAL_DETAIL__
		normalDetail = normalVector(texCoords);
#endif //__FAST_NORMAL_DETAIL__
	}

	// Simply offset the normal value based on the detail value... It looks good enough, but true PBR would probably want to use the tangent/bitangent below instead...
	normalDetail.rgb = normalize(clamp(normalDetail.xyz, 0.0, 1.0) * 2.0 - 1.0);
	//vec3 bump = normalize(mix(norm.xyz, normalDetail.xyz, u_Local3.a * (length(norm.xyz - normalDetail.xyz) / 3.0)));
	vec3 bump = normalize(mix(norm.xyz, normalDetail.xyz, 0.4));
	norm.rgb = normalize(mix(norm.xyz, normalDetail.xyz, 0.25 * (length(norm.xyz - normalDetail.xyz) / 3.0)));

	// If we ever need a tangent/bitangent, we can get one like this... But I'm just working in world directions, so it's not required...
	//vec3 tangent = TangentFromNormal( norm.xyz );
	//vec3 bitangent = normalize( cross(norm.xyz, tangent) );
	//mat3 tangentToWorld = mat3(tangent.xyz, bitangent.xyz, norm.xyz);
	//norm.xyz = tangentToWorld * normalDetail.xyz;


	//
	// Default material settings... PBR inputs could go here instead...
	//

#define specularReflectivePower		materialSettings.x
#define reflectionPower				materialSettings.y


	//
	// This is the basics of creating a fake PBR look to the lighting. It could be replaced, or overridden by actual PBR pixel buffer inputs.
	//

	// This should give me roughly how close to grey this color is... For light colorization.. Highly colored stuff should get less color added to it...
	float greynessFactor = 1.0 - clamp((length(outColor.r - outColor.g) + length(outColor.r - outColor.b)) / 2.0, 0.0, 1.0);
	greynessFactor = pow(greynessFactor, 64.0);

	// Also check how bright it is, so we can scale the lighting up/down...
	float brightnessFactor = 1.0 - clamp(max(outColor.r, max(outColor.g, outColor.b)), 0.0, 1.0);
	brightnessFactor = 1.0 - pow(brightnessFactor, 16.0);

	// It looks better to use slightly different cube and light reflection multipliers... Lights should always add some light, cubes should allow none on some pixels..
	float cubeReflectionFactor = clamp(greynessFactor * brightnessFactor, 0.25, 1.0) * reflectionPower;
	float lightsReflectionFactor = (greynessFactor * brightnessFactor * specularReflectivePower) * 0.5 + 0.5;
#if defined(__SCREEN_SPACE_REFLECTIONS__)
	float ssrReflectivePower = lightsReflectionFactor * reflectionPower * ssReflection;
#endif //defined(__SCREEN_SPACE_REFLECTIONS__)


	//
	// Set up the general directional stuff, to use throughout the shader...
	//

	vec3 N = norm.xyz;
	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);
	vec3 rayDir = reflect(E, N);
	vec3 cubeRayDir = reflect(E, flatNorm);
	vec3 sunDir = normalize(position.xyz - u_PrimaryLightOrigin.xyz);
	float NE = clamp(length(dot(N, E)), 0.0, 1.0);
	float reflectVectorPower = pow(specularReflectivePower*NE, 16.0) * reflectionPower;


	float diffuse = clamp(pow(clamp(dot(-sunDir.rgb, bump.rgb/*norm.rgb*/), 0.0, 1.0), 8.0) * 0.6 + 0.6, 0.0, 1.0);
	color.rgb = outColor.rgb = outColor.rgb * diffuse;

//#define __DEBUG_LIGHT__
#ifdef __DEBUG_LIGHT__
	if (u_Local3.r == 1.0)
	{
		outColor.rgb = vec3(specularReflectivePower);
		outColor.a = 1.0;
		gl_FragColor = outColor;
		return;
	}

	if (u_Local3.r == 2.0)
	{
		outColor.rgb = vec3(reflectionPower);
		outColor.a = 1.0;
		gl_FragColor = outColor;
		return;
	}

	if (u_Local3.r == 3.0)
	{
		outColor.rgb = vec3(lightsReflectionFactor);
		outColor.a = 1.0;
		gl_FragColor = outColor;
		return;
	}
#endif

	//
	// Now all the hard work...
	//

	vec3 specularColor = vec3(0.0);
	vec3 skyColor = vec3(0.0);
	vec3 emissiveCubeLightColor = vec3(0.0);
	vec3 emissiveCubeLightDirection = vec3(0.0);
	vec3 irradiance = vec3(1.0);

#if !defined(__LQ_MODE__) && defined(__IRRADIANCE__)
	if (u_Local3.r > 0.0)
	{
		irradiance = computeIrradiance(N);

		if (u_Local3.r >= 2.0)
		{
			outColor.rgb = vec3(irradiance);
			outColor.a = 1.0;
			gl_FragColor = outColor;
			return;
		}
	}
#endif //!defined(__LQ_MODE__) && defined(__IRRADIANCE__)

#ifndef __LQ_MODE__
	if (u_Local7.a > 0.0)
	{// Sky cube light contributions... If enabled...
		// This used to be done in rend2 code, now done here because I need u_CubeMapInfo.xyz to be cube origin for distance checks above... u_CubeMapInfo.w is now radius.
		vec4 cubeInfo = vec4(0.0, 0.0, 0.0, 1.0);//u_CubeMapInfo;
		cubeInfo.xyz -= u_ViewOrigin.xyz;

		cubeInfo.w = pow(distance(u_ViewOrigin.xyz, vec3(0.0, 0.0, 0.0)), 3.0);

		cubeInfo.xyz *= 1.0 / cubeInfo.w;
		cubeInfo.w = 1.0 / cubeInfo.w;
					
		vec3 parallax = cubeInfo.xyz + cubeInfo.w * E;
		parallax.z *= -1.0;
		
		vec3 reflected = cubeRayDir + parallax;
		reflected = vec3(-reflected.y, -reflected.z, -reflected.x);

		vec3 reflected2 = rayDir + parallax;
		reflected2 = vec3(-reflected2.y, -reflected2.z, -reflected2.x);

		if (u_Local6.a > 0.0 && u_Local6.a < 1.0)
		{// Mix between night and day colors...
			vec3 skyColorDay = texture(u_SkyCubeMap, reflected).rgb;
			skyColorDay += texture(u_SkyCubeMap, reflected2).rgb;
			skyColorDay /= 2.0;
			vec3 skyColorNight = texture(u_SkyCubeMapNight, reflected).rgb;
			skyColorNight += texture(u_SkyCubeMapNight, reflected2).rgb;
			skyColorNight /= 2.0;
			skyColor = mix(skyColorDay, skyColorNight, clamp(u_Local6.a, 0.0, 1.0));
		}
		else if (u_Local6.a >= 1.0)
		{// Night only colors...
			skyColor = texture(u_SkyCubeMapNight, reflected).rgb;
			skyColor += texture(u_SkyCubeMapNight, reflected2).rgb;
			skyColor /= 2.0;
		}
		else
		{// Day only colors...
			skyColor = texture(u_SkyCubeMap, reflected).rgb;
			skyColor += texture(u_SkyCubeMap, reflected2).rgb;
			skyColor /= 2.0;
		}

		skyColor = clamp(ContrastSaturationBrightness(skyColor, 1.0, 2.0, 0.333), 0.0, 1.0);
		skyColor = clamp(Vibrancy( skyColor, 0.4 ), 0.0, 1.0);
	}
#endif //__LQ_MODE__

	if (specularReflectivePower > 0.0)
	{// If this pixel is ging to get any specular reflection, generate (PBR would instead look up image buffer) specular color, and grab any cubeMap lighting as well...
		// Construct generic specular map by creating a greyscale, contrasted, saturation removed, color from the screen color... Then multiply by the material's default specular modifier...
		if (isMetalic)
		{
			specularColor = ContrastSaturationBrightness(outColor.rgb, 1.25, 1.25, 0.7);
		}
		else
		{
			specularColor = ContrastSaturationBrightness(outColor.rgb, 1.25, 0.05, 1.0);
			specularColor.rgb = clamp(vec3(length(specularColor.rgb) / 3.0), 0.0, 1.0);
		}

		specularColor.rgb *= specularReflectivePower;

		specularColor.rgb = mix(specularColor.rgb, skyColor * specularColor.rgb, reflectionPower);

#ifndef __LQ_MODE__
		if (u_Local7.r > 0.0)
		{// Cubemaps enabled...
#if defined(__CUBEMAPS__) || defined(__EMISSIVE_IBL__)
			//vec3 reflectance = EnvironmentBRDF(cubeReflectionFactor, NE, specularColor.rgb);
			vec3 cubeLightColor = vec3(0.0);
			float cubeFade = 0.0;
			
			// This used to be done in rend2 code, now done here because I need u_CubeMapInfo.xyz to be cube origin for distance checks above... u_CubeMapInfo.w is now radius.
			vec4 cubeInfo = u_CubeMapInfo;
			cubeInfo.xyz -= u_ViewOrigin.xyz;

			cubeInfo.w = pow(distance(u_ViewOrigin.xyz, u_CubeMapInfo.xyz), 3.0);

			cubeInfo.xyz *= 1.0 / cubeInfo.w;
			cubeInfo.w = 1.0 / cubeInfo.w;

			vec3 parallax = cubeInfo.xyz + cubeInfo.w * E;
			parallax.z *= -1.0;

#ifdef __EMISSIVE_IBL__
			if (u_Local9.r > 0.0)
			{// Also grab emissive cube lighting color...
				emissiveCubeLightColor = texture(u_EmissiveCubeMap, cubeRayDir/*rayDir*/ + parallax).rgb;
				emissiveCubeLightColor += texture(u_EmissiveCubeMap, rayDir + parallax).rgb;
				emissiveCubeLightColor /= 2.0;
				emissiveCubeLightDirection = rayDir + parallax;
			}
#endif //__EMISSIVE_IBL__
		
#ifdef __CUBEMAPS__
			if (reflectionPower > 0.0)
			{
				float curDist = distance(u_ViewOrigin.xyz, position.xyz);

				if (curDist < u_Local7.g)
				{
					cubeFade = clamp((1.0 - clamp(curDist / u_Local7.g, 0.0, 1.0)) * reflectionPower * (u_CubeMapStrength * 20.0), 0.0, 1.0);
				}

				if (cubeFade > 0.0)
				{
					//cubeLightColor = textureLod(u_CubeMap, cubeRayDir + parallax, 7.0 - (cubeReflectionFactor * 7.0)).rgb;
					cubeLightColor = texture(u_CubeMap, cubeRayDir/*rayDir*/ + parallax).rgb;
					cubeLightColor += texture(u_CubeMap, rayDir + parallax).rgb;
					cubeLightColor /= 2.0;

					cubeLightColor = clamp(ContrastSaturationBrightness(cubeLightColor, 3.0, 4.0, 0.3), 0.0, 1.0);
					cubeLightColor = clamp(Vibrancy(cubeLightColor, 0.4), 0.0, 1.0);

					outColor.rgb = mix(outColor.rgb, outColor.rgb + cubeLightColor.rgb, reflectVectorPower * cubeFade * (u_CubeMapStrength * 20.0) * cubeReflectionFactor);
				}
			}
#endif //__CUBEMAPS__
#endif //defined(__CUBEMAPS__) || defined(__EMISSIVE_IBL__)
		}
#endif //!__LQ_MODE__
	}

	//
	// SSDO input, if enabled...
	//
	vec4 occlusion = vec4(0.0);
	bool useOcclusion = false;

#ifndef __LQ_MODE__
	if (u_Local2.r == 1.0)
	{
		useOcclusion = true;
		occlusion = texture(u_HeightMap, texCoords);
	}
#endif //!__LQ_MODE__

	if (u_Local7.a > 0.0)
	{// Sky light contributions...
#ifndef __LQ_MODE__
		outColor.rgb = mix(outColor.rgb, outColor.rgb + skyColor, clamp(pow(reflectionPower, 2.0) * u_Local7.a * cubeReflectionFactor, 0.0, 1.0));
#endif //__LQ_MODE__
		outColor.rgb = mix(outColor.rgb, outColor.rgb + specularColor, clamp(pow(reflectVectorPower, 2.0) * cubeReflectionFactor, 0.0, 1.0));
		//outColor.rgb = skyColor;
	}


	if (u_Local1.r > 0.0)
	{// If r_blinnPhong is <= 0.0 then this is pointless...
		float phongFactor = (u_Local1.r * 12.0) * u_Local1.g;

#define LIGHT_COLOR_POWER			4.0

		if (phongFactor > 0.0 && u_Local6.a < 1.0)
		{// this is blinn phong
			float light_occlusion = 1.0;

#ifndef __LQ_MODE__
			if (useOcclusion)
			{
				light_occlusion = 1.0 - clamp(dot(vec4(-sunDir*E, 1.0), occlusion), 0.0, 1.0);
			}
#endif //__LQ_MODE__

			float maxBright = clamp(max(outColor.r, max(outColor.g, outColor.b)), 0.0, 1.0);
			float power = clamp(pow(maxBright * 0.75, LIGHT_COLOR_POWER) + 0.333, 0.0, 1.0);
		
			vec3 lightColor = u_PrimaryLightColor.rgb;

			float lightMult = clamp(specularReflectivePower * power * light_occlusion, 0.0, 1.0);

			if (lightMult > 0.0)
			{
				lightColor *= lightMult;
				lightColor = blinn_phong(position.xyz, outColor.rgb, N, E, normalize(-sunDir), outColor.rgb * lightColor, outColor.rgb * lightColor, 1.0, u_PrimaryLightOrigin.xyz);

#ifndef __LQ_MODE__
				if (isMetalic)
				{// Metals do an extra randomish light query to make them really shine... Mixed in at a lower level then the real direction...
					vec3 query = normalize(reflect(normalize(-sunDir), E));
					lightColor = mix(lightColor, blinn_phong(position.xyz, outColor.rgb, N, E, query, outColor.rgb * lightColor, outColor.rgb * lightColor * 2.0, 1.0, u_PrimaryLightOrigin.xyz), 0.25);
				}
#endif //__LQ_MODE__

				lightColor *= max(outColor.r, max(outColor.g, outColor.b)) * 0.9 + 0.1;
				lightColor *= clamp(1.0 - u_Local6.a, 0.0, 1.0); // Day->Night scaling of sunlight...
				lightColor = clamp(lightColor, 0.0, 0.7);

				// Add vibrancy to light color at sunset/sunrise???
				if (u_Local6.a > 0.0 && u_Local6.a < 1.0)
				{// Vibrancy gets greater the closer we get to night time...
					float vib = u_Local6.a;
					if (vib > 0.8)
					{// Scale back vibrancy to 0.0 just before nightfall...
						float downScale = 1.0 - vib;
						downScale *= 4.0;
						vib = mix(vib, 0.0, clamp(downScale, 0.0, 1.0));
					}
					lightColor = Vibrancy( lightColor, clamp(vib * 4.0, 0.0, 1.0) );
				}

				lightColor.rgb *= lightsReflectionFactor * phongFactor * irradiance;
				outColor.rgb = outColor.rgb + max(lightColor, vec3(0.0));
			}
		}

		if (u_lightCount > 0.0 && specularReflectivePower > 0.0)
		{
			phongFactor = u_Local1.r * 12.0;

			if (phongFactor <= 0.0)
			{// Never allow no phong...
				phongFactor = 1.0;
			}

			vec3 addedLight = vec3(0.0);
			float maxBright = clamp(max(outColor.r, max(outColor.g, outColor.b)), 0.0, 1.0);
			float power = maxBright * 0.85;
			power = clamp(pow(power, LIGHT_COLOR_POWER) + 0.333, 0.0, 1.0);

#ifdef __LQ_MODE__
			for (int li = 0; li < min(u_lightCount, 16); li++)
#else //!__LQ_MODE__
			for (int li = 0; li < u_lightCount; li++)
#endif //__LQ_MODE__
			{
				vec3 lightPos = u_lightPositions2[li].xyz;

				float lightDist = distance(lightPos, position.xyz);

				if (u_lightHeightScales[li] > 0.0)
				{// ignore height differences, check later...
					lightDist -= length(lightPos.z - position.z);
				}

				float lightDistMult = 1.0 - clamp((distance(lightPos.xyz, u_ViewOrigin.xyz) / MAX_DEFERRED_LIGHT_RANGE), 0.0, 1.0);
				lightDistMult = pow(lightDistMult, 2.0);

				// Attenuation...
				float lightFade = 1.0 - clamp((lightDist * lightDist) / (u_lightDistances[li] * u_lightDistances[li]), 0.0, 1.0);
				lightFade = pow(lightFade, 2.0);
				float lightStrength = lightDistMult * lightFade * specularReflectivePower * 0.5;

				if (lightStrength > 0.0)
				{
					vec3 lightColor = (u_lightColors[li].rgb / length(u_lightColors[li].rgb)) * u_Local4.a; // Normalize.
					vec3 lightDir = normalize(lightPos - position.xyz);
					float light_occlusion = 1.0;
					float selfShadow = clamp(pow(clamp(dot(-lightDir.rgb, bump.rgb/*norm.rgb*/), 0.0, 1.0), 8.0) * 0.6 + 0.6, 0.0, 1.0);
				
					lightColor = lightColor * power * irradiance;// * maxStr;

					addedLight.rgb += lightColor * lightStrength * 0.333 * selfShadow;

#ifndef __LQ_MODE__
					if (useOcclusion)
					{
						light_occlusion = (1.0 - clamp(dot(vec4(-lightDir*E, 1.0), occlusion), 0.0, 1.0));
					}
#endif //__LQ_MODE__

#if !defined(__LQ_MODE__) && defined(__LIGHT_OCCLUSION__)
					if (u_lightPositions[li].x >= 0.0 && u_lightPositions[li].x <= 1.0 && u_lightPositions[li].y >= 0.0 && u_lightPositions[li].y <= 1.0)
					{
						light_occlusion *= checkVisibility(texCoords, u_lightPositions[li]);

						if (u_Local3.r >= 0.0 && li == int(u_Local3.r))
						{
							outColor.rgb = vec3(light_occlusion);
							gl_FragColor = outColor;
							return;
						}
					}
#endif //!defined(__LQ_MODE__) && defined(__LIGHT_OCCLUSION__)
					
					addedLight.rgb += blinn_phong(position.xyz, outColor.rgb, N, E, lightDir, outColor.rgb * lightColor, outColor.rgb * lightColor * 0.1, mix(0.1, 0.5, clamp(lightsReflectionFactor, 0.0, 1.0)) * clamp(lightStrength * light_occlusion * phongFactor, 0.0, 1.0), lightPos) * lightFade * selfShadow;
				}
			}

			addedLight.rgb *= lightsReflectionFactor; // More grey colors get more colorization from lights...
			outColor.rgb = outColor.rgb + max(addedLight, vec3(0.0));
		}

#ifdef __EMISSIVE_IBL__
		if (u_Local9.r > 0.0 && u_Local3.a == 1.0)
		{// Debugging...
			outColor.rgb = emissiveCubeLightColor;
		}
		else if (u_Local9.r > 0.0 && specularReflectivePower > 0.0 && length(emissiveCubeLightColor.rgb) > 0.0)
		{// Also do emissive cube lighting (Emissive IBL)... Experimental crap...
			//emissiveCubeLightColor = texture(u_EmissiveCubeMap, R + parallax).rgb;
			//emissiveCubeLightDirection = R + parallax;

			phongFactor = u_Local1.r * 12.0;

			if (phongFactor <= 0.0)
			{// Never allow no phong...
				phongFactor = 1.0;
			}

			vec3 addedLight = vec3(0.0);

// Indices of refraction
#define Air 1.0
#define Bubble 1.06

// Air to glass ratio of the indices of refraction (Eta)
#define Eta (Air / Bubble)

// see http://en.wikipedia.org/wiki/Refractive_index Reflectivity
#define R0 (((Air - Bubble) * (Air - Bubble)) / ((Air + Bubble) * (Air + Bubble)))

#define shininess 1.0

			float NdotV = max(0.0, dot(E, N));
			float f = R0 + (1.0 - R0) * pow(1.0 - NdotV, 5.0) * pow(shininess, 20.0);
			//f = length(f.rgb) / 3.0;

			// Diffuse...
			float diffuse = getdiffuse(N, emissiveCubeLightDirection, u_Local3.b);
			addedLight += diffuse * emissiveCubeLightColor * specularReflectivePower * u_Local3.r;// 0.01;

			// CryTek's way
			vec3 mirrorEye = (2.0f * dot(E, N) * N - E);
			float dotSpec = saturate(dot(mirrorEye.xyz, -emissiveCubeLightDirection) * 0.5f + 0.5f);
			vec3 spec = (1.0f - f) * saturate(-emissiveCubeLightDirection.y) * ((pow(dotSpec, 512.0f)) * (shininess * 1.8f + 0.2f)) * emissiveCubeLightColor;
			addedLight += spec * 25.0 * saturate(shininess - 0.05f) * emissiveCubeLightColor * specularReflectivePower * u_Local3.g;// 0.1;
			outColor.rgb = max(outColor.rgb, addedLight.rgb);
		}
#endif //__EMISSIVE_IBL__
	}

#if defined(__SCREEN_SPACE_REFLECTIONS__)
	if (u_Local8.r > 0.0 && ssrReflectivePower > 0.0)
	{
		outColor.rgb = AddReflection(texCoords, position, flatNorm, outColor.rgb, ssrReflectivePower);
	}
#endif //defined(__SCREEN_SPACE_REFLECTIONS__)

#if defined(__AMBIENT_OCCLUSION__)
	if (u_Local1.b == 1.0)
	{// Fast AO enabled...
		float ao = calculateAO(sunDir, N * 10000.0);
		ao = clamp(ao * u_Local6.g + u_Local6.r, u_Local6.r, 1.0);
		outColor.rgb *= ao;
	}
#endif //defined(__AMBIENT_OCCLUSION__)

#if defined(__ENHANCED_AO__)
	if (u_Local1.b >= 2.0)
	{// Better, HQ AO enabled...
		float msao = 0.0;

		/*if (u_Local1.b >= 4.0)
		{
			const float width = 2.0;
			float numSamples = 0.0;
		
			for (float x = -width; x <= width; x += 1.0)
			{
				for (float y = -width; y <= width; y += 1.0)
				{
					vec2 coord = texCoords + (vec2(x, y) * pixel);
					msao += textureLod(u_SteepMap, coord, 0.0).x;
					numSamples += 1.0;
				}
			}

			msao /= numSamples;
		}
		else if (u_Local1.b >= 3.0)
		{
			const float width = 1.0;
			float numSamples = 0.0;
		
			for (float x = -width; x <= width; x += 1.0)
			{
				for (float y = -width; y <= width; y += 1.0)
				{
					vec2 coord = texCoords + (vec2(x, y) * pixel);
					msao += textureLod(u_SteepMap, coord, 0.0).x;
					numSamples += 1.0;
				}
			}

			msao /= numSamples;
		}
		else*/
		{
			msao = textureLod(u_SteepMap, texCoords, 0.0).x;
		}

		float sao = clamp(msao, 0.0, 1.0);
		sao = clamp(sao * u_Local6.g + u_Local6.r, u_Local6.r, 1.0);
		outColor.rgb *= sao;
	}
#endif //defined(__ENHANCED_AO__)

#if defined(__ENVMAP__)
	if (u_Local1.a > 0.0)
	{// Envmap enabled...
		float lightScale = clamp(1.0 - clamp(max(max(outColor.r, outColor.g), outColor.b), 0.0, 1.0), 0.0, 1.0);
		float invLightScale = clamp((1.0 - lightScale) * 1.2, 0.2, 1.0);
		vec3 env = envMap(rayDir, 0.6 /* warmth */);
		outColor.rgb = mix(outColor.rgb, outColor.rgb + ((env * (specularReflectivePower * 0.5) * invLightScale) * lightScale), clamp((specularReflectivePower * 0.5) * lightScale * cubeReflectionFactor, 0.0, 1.0));
	}
#endif //defined(__ENVMAP__)

#if defined(__HEIGHTMAP_SHADOWS__)
	float hshadow = GetShadow(texCoords, u_PrimaryLightOrigin.xyz, sunDir.xyz);
	outColor.rgb *= hshadow;
#endif //defined(__HEIGHTMAP_SHADOWS__)

#if defined(USE_SHADOWMAP) && !defined(__LQ_MODE__)
	if (u_Local2.g > 0.0 && u_Local6.a < 1.0)
	{
		float shadowValue = texture(u_ShadowMap, texCoords).r;

		shadowValue = pow(shadowValue, 1.5);

#define sm_cont_1 ( 64.0 / 255.0)
#define sm_cont_2 (255.0 / 200.0)
		shadowValue = clamp((clamp(shadowValue - sm_cont_1, 0.0, 1.0)) * sm_cont_2, 0.0, 1.0);
		float finalShadow = clamp(shadowValue + u_Local2.b, u_Local2.b, u_Local2.a);
		finalShadow = mix(finalShadow, 1.0, clamp(u_Local6.a, 0.0, 1.0)); // Dampen out shadows at sunrise/sunset...
		outColor.rgb *= finalShadow;
	}
#endif //defined(USE_SHADOWMAP) && !defined(__LQ_MODE__)

	// De-emphasize (darken) the distant map a bit...
	//float depth = clamp(pow(1.0 - clamp(distance(position.xyz, u_ViewOrigin.xyz) / 65536.0, 0.0, 1.0), 4.5), 0.35, 1.0); // darken distant
	//float depth = clamp(pow(clamp(distance(position.xyz, u_ViewOrigin.xyz) / 65536.0, 0.0, 1.0), 4.5) * 100000.0 + 1.0, 1.0, 2.0); // brighten distant
	//outColor.rgb *= depth;

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

	outColor.rgb = clamp(outColor.rgb, 0.0, 1.0);
	gl_FragColor = outColor;
}

