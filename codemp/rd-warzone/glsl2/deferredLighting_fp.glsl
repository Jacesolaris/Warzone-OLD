#ifndef __LQ_MODE__

#define __FAST_NORMAL_DETAIL__
#define __AMBIENT_OCCLUSION__
#define __ENHANCED_AO__
#define __RANDOMIZE_LIGHT_PIXELS__
#define __SCREEN_SPACE_REFLECTIONS__
//#define __FAST_LIGHTING__ // Game code now defines this when enabled...
//#define __BUMP_ENHANCE__

#ifdef USE_CUBEMAPS
	#define __CUBEMAPS__
#endif //USE_CUBEMAPS

#endif //__LQ_MODE__

uniform sampler2D							u_DiffuseMap;		// Screen image
uniform sampler2D							u_ScreenDepthMap;	// Depth map
uniform sampler2D							u_NormalMap;		// Flat normals
uniform sampler2D							u_PositionMap;		// positionMap
uniform sampler2D							u_WaterPositionMap;	// water positions
uniform sampler2D							u_OverlayMap;		// Real normals. Alpha channel 1.0 means enabled...
uniform sampler2D							u_SteepMap;			// ssao image
uniform sampler2D							u_HeightMap;		// ssdoImage
uniform sampler2D							u_GlowMap;			// anamorphic
uniform sampler2D							u_ShadowMap;		// Screen Shadow Map
uniform sampler2D							u_WaterEdgeMap;		// tr.shinyImage
uniform sampler2D							u_RoadsControlMap;	// map heightmap
uniform samplerCube							u_SkyCubeMap;		// Day sky cubemap
uniform samplerCube							u_SkyCubeMapNight;	// Night sky cubemap
uniform samplerCube							u_CubeMap;			// Closest cubemap
uniform samplerCube							u_EmissiveCubeMap;	// Closest emissive cubemap

uniform mat4								u_ModelViewProjectionMatrix;

uniform vec2								u_Dimensions;

uniform vec4								u_Local1; // r_blinnPhong, SUN_PHONG_SCALE, r_ao, r_env
uniform vec4								u_Local2; // SSDO, SHADOWS_ENABLED, SHADOW_MINBRIGHT, SHADOW_MAXBRIGHT
uniform vec4								u_Local3; // r_testShaderValue1, r_testShaderValue2, r_testShaderValue3, r_testShaderValue4
uniform vec4								u_Local4; // MAP_INFO_MAXSIZE, MAP_WATER_LEVEL, floatTime, MAP_EMISSIVE_COLOR_SCALE
uniform vec4								u_Local5; // CONTRAST, SATURATION, BRIGHTNESS, TRUEHDR_ENABLED
uniform vec4								u_Local6; // AO_MINBRIGHT, AO_MULTBRIGHT, VIBRANCY, NightScale
uniform vec4								u_Local7; // cubemapEnabled, r_cubemapCullRange, r_cubeMapSize, r_skyLightContribution
uniform vec4								u_Local8; // enableReflections, MAP_HDR_MIN, MAP_HDR_MAX, MAP_INFO_PLAYABLE_MAXS[2]
uniform vec4								u_Local9; // haveEmissiveCube, MAP_USE_PALETTE_ON_SKY, SNOW_ENABLED, PROCEDURAL_SNOW_LOWEST_ELEVATION

uniform vec4								u_ViewInfo; // znear, zfar, zfar / znear, fov
uniform vec3								u_ViewOrigin;
uniform vec4								u_PrimaryLightOrigin;
uniform vec3								u_PrimaryLightColor;

uniform vec4								u_CubeMapInfo;
uniform float								u_CubeMapStrength;

uniform int									u_lightCount;
uniform vec3								u_lightPositions2[MAX_DEFERRED_LIGHTS];
uniform float								u_lightDistances[MAX_DEFERRED_LIGHTS];
uniform float								u_lightHeightScales[MAX_DEFERRED_LIGHTS];
uniform vec3								u_lightColors[MAX_DEFERRED_LIGHTS];
//uniform int									u_lightMax;
uniform float								u_lightMaxDistance;

uniform vec4								u_Mins; // mins, mins, mins, WATER_ENABLED
uniform vec4								u_Maxs;

varying vec2								var_TexCoords;


#define BLINN_PHONG_STRENGTH				u_Local1.r
#define SUN_PHONG_SCALE						u_Local1.g
#define AO_TYPE								u_Local1.b
#define USE_ENVMAPPING						u_Local1.a

#define USE_SSDO							u_Local2.r
#define SHADOWS_ENABLED						u_Local2.g
#define SHADOW_MINBRIGHT					u_Local2.b
#define SHADOW_MAXBRIGHT					u_Local2.a

#define MAP_INFO_MAXSIZE					u_Local4.r // UNUSED
#define MAP_WATER_LEVEL						u_Local4.g // UNUSED
#define FLOAT_TIME							u_Local4.b // UNUSED
#define MAP_EMISSIVE_COLOR_SCALE			u_Local4.a

#define CONTRAST_STRENGTH					u_Local5.r
#define SATURATION_STRENGTH					u_Local5.g
#define BRIGHTNESS_STRENGTH					u_Local5.b
#define TRUEHDR_ENABLED						u_Local5.a

#define AO_MINBRIGHT						u_Local6.r
#define AO_MULTBRIGHT						u_Local6.g
#define VIBRANCY							u_Local6.b
#define NIGHT_SCALE							u_Local6.a

#define CUBEMAP_ENABLED						u_Local7.r
#define CUBEMAP_CULLRANGE					u_Local7.g
#define CUBEMAP_SIZE						u_Local7.b // UNUSED
#define SKY_LIGHT_CONTRIBUTION				u_Local7.a

#define REFLECTIONS_ENABLED					u_Local8.r
#define MAP_HDR_MIN							u_Local8.g
#define MAP_HDR_MAX							u_Local8.b
#define MAP_INFO_PLAYABLE_HEIGHT			u_Local8.a

#define HAVE_EMISSIVE_CUBE					u_Local9.r // UNUSED
#define MAP_USE_PALETTE_ON_SKY				u_Local9.g
#define SNOW_ENABLED						u_Local9.b
#define PROCEDURAL_SNOW_LOWEST_ELEVATION	u_Local9.a

#define WATER_ENABLED						u_Mins.a

vec2 pixel = vec2(1.0) / u_Dimensions;

#define hdr_const_1 (MAP_HDR_MIN / 255.0)
#define hdr_const_2 (255.0 / MAP_HDR_MAX)

//#define __ENCODE_NORMALS_RECONSTRUCT_Z__
#define __ENCODE_NORMALS_STEREOGRAPHIC_PROJECTION__
//#define __ENCODE_NORMALS_CRY_ENGINE__
//#define __ENCODE_NORMALS_EQUAL_AREA_PROJECTION__

#ifdef __ENCODE_NORMALS_STEREOGRAPHIC_PROJECTION__
vec2 EncodeNormal(vec3 n)
{
	float scale = 1.7777;
	vec2 enc = n.xy / (n.z + 1.0);
	enc /= scale;
	enc = enc * 0.5 + 0.5;
	return enc;
}
vec3 DecodeNormal(vec2 enc)
{
	vec3 enc2 = vec3(enc.xy, 0.0);
	float scale = 1.7777;
	vec3 nn =
		enc2.xyz*vec3(2.0 * scale, 2.0 * scale, 0.0) +
		vec3(-scale, -scale, 1.0);
	float g = 2.0 / dot(nn.xyz, nn.xyz);
	return vec3(g * nn.xy, g - 1.0);
}
#elif defined(__ENCODE_NORMALS_CRY_ENGINE__)
vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N * 4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);
	return vec3(encoded * g, 1.0 - f * 0.5);
}
vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}
#elif defined(__ENCODE_NORMALS_EQUAL_AREA_PROJECTION__)
vec2 EncodeNormal(vec3 n)
{
	float f = sqrt(8.0 * n.z + 8.0);
	return n.xy / f + 0.5;
}
vec3 DecodeNormal(vec2 enc)
{
	vec2 fenc = enc * 4.0 - 2.0;
	float f = dot(fenc, fenc);
	float g = sqrt(1.0 - f / 4.0);
	vec3 n;
	n.xy = fenc*g;
	n.z = 1.0 - f / 2.0;
	return n;
}
#else //__ENCODE_NORMALS_RECONSTRUCT_Z__
vec3 DecodeNormal(in vec2 N)
{
	vec3 norm;
	norm.xy = N * 2.0 - 1.0;
	norm.z = sqrt(1.0 - dot(norm.xy, norm.xy));
	return norm;
}
vec2 EncodeNormal(vec3 n)
{
	return vec2(n.xy * 0.5 + 0.5);
}
#endif //__ENCODE_NORMALS_RECONSTRUCT_Z__


vec4 positionMapAtCoord ( vec2 coord, out bool changedToWater )
{
	changedToWater = false;

	vec4 pos = textureLod(u_PositionMap, coord, 0.0);

	if (WATER_ENABLED > 0.0)
	{
		bool isSky = (pos.a - 1.0 >= MATERIAL_SKY) ? true : false;

		float isWater = textureLod(u_WaterPositionMap, coord, 0.0).a;

		if (isWater > 0.0 || (isWater > 0.0 && isSky))
		{
			vec3 wMap = textureLod(u_WaterPositionMap, coord, 0.0).xyz;
		
			if (wMap.z > pos.z || isSky)
			{
				pos.xyz = wMap.xyz;

				if (!isSky)
				{// So we know if this is a shoreline or water in skybox...
					changedToWater = true;
				}
				else
				{// Also change the material...
					pos.a = MATERIAL_WATER + 1.0;
				}
			}
		}
	}

	return pos;
}


vec2 RB_PBR_DefaultsForMaterial(float MATERIAL_TYPE)
{// I probably should use an array of const's instead, but this will do for now...
	vec2 settings = vec2(0.0);

	float specularReflectionScale = 0.0;
	float cubeReflectionScale = 0.0;

	switch (int(MATERIAL_TYPE))
	{
	case MATERIAL_WATER:			// 13			// light covering of water on a surface
		specularReflectionScale = 0.1;
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
	case MATERIAL_POLISHEDWOOD:		// 3			// shiny polished wood
		specularReflectionScale = 0.026;
		cubeReflectionScale = 0.35;
		break;
	case MATERIAL_SOLIDMETAL:		// 3			// solid girders
		specularReflectionScale = 0.098;
		cubeReflectionScale = 0.98;
		break;
	case MATERIAL_HOLLOWMETAL:		// 4			// hollow metal machines -- UQ1: Used for weapons to force lower parallax and high reflection...
		specularReflectionScale = 0.098;
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
		specularReflectionScale = 0.1;
		cubeReflectionScale = 0.7;
		break;
	case MATERIAL_LAVA:
		specularReflectionScale = 0.002;
		cubeReflectionScale = 0.0;
		break;
	case MATERIAL_EFX:
	case MATERIAL_BLASTERBOLT:
	case MATERIAL_FIRE:
	case MATERIAL_SMOKE:
	case MATERIAL_MAGIC_PARTICLES:
	case MATERIAL_MAGIC_PARTICLES_TREE:
	case MATERIAL_FIREFLIES:
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
	//specularReflectionScale = specularReflectionScale * u_Local3.b + u_Local3.a;
	
	cubeReflectionScale = cubeReflectionScale * 0.75 + 0.25;
	//cubeReflectionScale = cubeReflectionScale * u_Local3.r + u_Local3.g;

	settings.x = specularReflectionScale;
	settings.y = cubeReflectionScale;

	return settings;
}

float lhash( const in float n ) {
	return fract(sin(n)*4378.5453);
}

float lnoise(in vec3 o) 
{
	vec3 p = floor(o);
	vec3 fr = fract(o);
		
	float n = p.x + p.y*57.0 + p.z * 1009.0;

	float a = lhash(n+  0.0);
	float b = lhash(n+  1.0);
	float c = lhash(n+ 57.0);
	float d = lhash(n+ 58.0);
	
	float e = lhash(n+  0.0 + 1009.0);
	float f = lhash(n+  1.0 + 1009.0);
	float g = lhash(n+ 57.0 + 1009.0);
	float h = lhash(n+ 58.0 + 1009.0);
	
	
	vec3 fr2 = fr * fr;
	vec3 fr3 = fr2 * fr;
	
	vec3 t = 3.0 * fr2 - 2.0 * fr3;
	
	float u = t.x;
	float v = t.y;
	float w = t.z;

	// this last bit should be refactored to the same form as the rest :)
	float res1 = a + (b-a)*u +(c-a)*v + (a-b+d-c)*u*v;
	float res2 = e + (f-e)*u +(g-e)*v + (e-f+h-g)*u*v;
	
	float res = res1 * (1.0- w) + res2 * (w);
	
	return res;
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

#if defined(__SCREEN_SPACE_REFLECTIONS__)
#define pw pixel.x
#define ph pixel.y
vec3 AddReflection(vec2 coord, vec4 positionMap, vec3 flatNorm, vec3 inColor, float reflectiveness, float ssReflection)
{
	if (reflectiveness <= 0.5)
	{// Top of screen pixel is water, don't check...
		return inColor;
	}

	bool changedToWater = false;
	float pixelDistance = distance(positionMap.xyz, u_ViewOrigin.xyz);

	//const float scanSpeed = 48.0;// 16.0;// 5.0; // How many pixels to scan by on the 1st rough pass...
	const float scanSpeed = 16.0;

	// Quick scan for pixel that is not water...
	float QLAND_Y = 0.0;

	for (float y = coord.y; y <= 1.0; y += ph * scanSpeed)
	{
		vec3 norm = DecodeNormal(textureLod(u_NormalMap, vec2(coord.x, y), 0.0).xy);
		vec4 pMap = positionMapAtCoord(vec2(coord.x, y), changedToWater);

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
		vec4 pMap = positionMapAtCoord(vec2(coord.x, y), changedToWater);
		
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

	float d = 1.0 / ((1.0 - (LAND_Y - coord.y)) * 1.75);

	upPos = clamp(coord.y + ((LAND_Y - coord.y) * 2.0 * d), 0.0, 1.0);

	if (upPos > 1.0 || upPos < 0.0)
	{// Not on screen...
		return inColor;
	}

	vec4 pMap = positionMapAtCoord(vec2(coord.x, upPos), changedToWater);

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

#if defined(__AMBIENT_OCCLUSION__)
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

float aomap(vec3 p)
{
    float n = (.5-cellTile(p))*1.5;
    return p.y + dot(sin(p/2. + cos(p.yzx/2. + 3.14159/2.)), vec3(.5)) + n;
}
#endif //defined(__AMBIENT_OCCLUSION__)


#if defined(__AMBIENT_OCCLUSION__)
float calculateAO(in vec3 pos, in vec3 nor)
{
	float sca = 0.00013/*2.0*/, occ = 0.0;
	for( int i=0; i<5; i++ ){
		float hr = 0.01 + float(i)*0.5/4.0;        
		float dd = aomap(nor * hr + pos);
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
float getspecularLight(vec3 surfaceNormal, vec3 lightDirection, vec3 viewDirection, float shininess)
{
	//Calculate Blinn-Phong power
	vec3 H = normalize(viewDirection + lightDirection);
	return clamp(pow(max(0.0, dot(surfaceNormal, H)), shininess), 0.1, 1.0);
}

float getdiffuse(vec3 n, vec3 l, float p) {
	float ndotl = clamp(dot(n, l), 0.5, 0.9);
	return pow(ndotl, p);
	//return pow(dot(n, l) * 0.4 + 0.6, p);
}

vec3 blinn_phong(vec3 pos, vec3 color, vec3 normal, vec3 view, vec3 light, vec3 diffuseColor, vec3 specularColor, float specPower, vec3 lightPos) {
	// Ambient light.
	float ambience = 4.0;// 0.25;

	// Diffuse lighting.
	float diff = getdiffuse(normal, light, 2.0) * 16.0;

	// Specular lighting.
	float fre = clamp(pow(clamp(dot(normal, -view) + 1.0, 0.0, 1.0), -2.0), 0.0, 1.0);
	float spec = pow(max(dot(reflect(-light, normal), view), 0.0), 1.2);

	return (clamp(diffuseColor, 0.0, 1.0) * ambience) + (clamp(diffuseColor, 0.0, 1.0) * diff) + (clamp(specularColor, 0.0, 1.0) * spec * fre);
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
	float noise = lnoise(pos.xyx) * 0.5;
	noise += lnoise(pos.yzx * 0.5);
	noise += lnoise(pos.zxy * 0.25) * 2.0;
	noise += lnoise(pos.yxz * 0.125) * 4.0;

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

	// Ambient light.
	float ambience = 4.0;// 0.25;

	// Diffuse light.
	float diff = getdiffuse(normal, light, 2.0) * 16.0;

	// Specular light.
	vec3 v = view;
	float NoV = clamp(dot(normal, v), 0.0, 1.0);
	vec3 r = reflect(-v, normal);

#if 1
	float NdotLSphere;
	float specSph = clamp(sphereLight(pos, normal, v, r, f0, roughness, NoV, NdotLSphere, lightPos), 0.0, 1.0/*0.2*/);
	float spec = clamp(/*0.3183 **/ NdotLSphere+specSph, 0.0, 1.0/*0.2*/);
#else
	float NdotLTube;
	float specTube = clamp(tubeLight(pos, normal, v, r, f0, roughness, NoV, NdotLTube, lightPosStart, lightPosEnd), 0.0, 1.0/*0.2*/);
	float spec = clamp(/*0.3183 **/ NdotLTube+specTube, 0.0, 1.0/*0.2*/);
#endif
	
	spec = pow(spec, 1.0 / 2.2) * specPower * 256.0;

	return (clamp(diffuseColor, 0.0, 1.0) * ambience) + (clamp(diffuseColor, 0.0, 1.0) * diff) + (clamp(specularColor, 0.0, 1.0) * clamp(albedo, 0.0, 1.0) * spec);
}
#endif //defined(__LQ_MODE__) || defined(__FAST_LIGHTING__)

//
// Normal variation...
//
const float pi = 3.14159;
const vec4 cHashA4 = vec4 (0., 1., 57., 58.);
const vec3 cHashA3 = vec3 (1., 57., 113.);
const float cHashM = 43758.54;

vec4 Hashv4f (float p)
{
  return fract (sin (p + cHashA4) * cHashM);
}

float Noisefv2 (vec2 p)
{
  vec2 i = floor (p);
  vec2 f = fract (p);
  f = f * f * (3. - 2. * f);
  vec4 t = Hashv4f (dot (i, cHashA3.xy));
  return mix (mix (t.x, t.y, f.x), mix (t.z, t.w, f.x), f.y);
}

vec3 Noisev3v2 (vec2 p)
{
  vec2 i = floor (p);
  vec2 f = fract (p);
  vec2 ff = f * f;
  vec2 u = ff * (3. - 2. * f);
  vec2 uu = 30. * ff * (ff - 2. * f + 1.);
  vec4 h = Hashv4f (dot (i, cHashA3.xy));
  return vec3 (h.x + (h.y - h.x) * u.x + (h.z - h.x) * u.y +
     (h.x - h.y - h.z + h.w) * u.x * u.y, uu * (vec2 (h.y - h.x, h.z - h.x) +
     (h.x - h.y - h.z + h.w) * u.yx));
}

float Fbmn (vec3 p, vec3 n)
{
  vec3 s;
  float a;
  s = vec3 (0.);
  a = 1.;
  for (int i = 0; i < 5; i ++) {
    s += a * vec3 (Noisefv2 (p.yz), Noisefv2 (p.zx), Noisefv2 (p.xy));
    a *= 0.5;
    p *= 2.;
  }
  return dot (s, abs (n));
}

vec3 VaryNf (vec3 p, vec3 n, float f)
{
  vec3 g;
  float s;
  vec3 e = vec3 (0.1, 0., 0.);
  s = Fbmn (p, n);
  g = vec3 (Fbmn (p + e.xyy, n) - s,
     Fbmn (p + e.yxy, n) - s, Fbmn (p + e.yyx, n) - s);
  return normalize (n + f * (g - n * dot (n, g)));
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

vec3 splatblend(vec3 color1, float a1, vec3 color2, float a2)
{
    float depth = 0.2;
	float ma = max(a1, a2) - depth;

    float b1 = max(a1 - ma, 0);
    float b2 = max(a2 - ma, 0);

    return ((color1.rgb * b1) + (color2.rgb * b2)) / (b1 + b2);
}

// Grey scale.
float getGrey(vec3 p){ return p.x*0.299 + p.y*0.587 + p.z*0.114; }

// Texture bump mapping. Four tri-planar lookups, or 12 texture lookups in total.
vec3 doBumpMap( sampler2D tex, in vec2 tc, in vec3 nor, float bumpfactor)
{
    const float eps = 0.001;
    vec3 grad = vec3( getGrey(texture(tex, vec2(tc.x-eps, tc.y)).rgb),
                      getGrey(texture(tex, vec2(tc.x, tc.y-eps)).rgb),
                      getGrey(texture(tex, vec2(tc.x, tc.y)).rgb));
    
    grad = (grad - getGrey(texture(tex, tc.xy).rgb))/eps; 
            
    grad -= nor*dot(nor, grad);          

    return normalize( nor + grad*bumpfactor );
}

void main(void)
{
	bool changedToWater = false;
	vec4 color = texture(u_DiffuseMap, var_TexCoords);
	vec4 outColor = vec4(color.rgb, 1.0);
	vec4 position = positionMapAtCoord(var_TexCoords, changedToWater);

	if (position.a - 1.0 == MATERIAL_SKY
		|| position.a - 1.0 == MATERIAL_SUN
		|| position.a - 1.0 == MATERIAL_GLASS
		|| position.a - 1.0 == MATERIAL_EFX
		|| position.a - 1.0 == MATERIAL_BLASTERBOLT
		|| position.a - 1.0 == MATERIAL_FIRE
		|| position.a - 1.0 == MATERIAL_SMOKE
		|| position.a - 1.0 == MATERIAL_MAGIC_PARTICLES
		|| position.a - 1.0 == MATERIAL_MAGIC_PARTICLES_TREE
		|| position.a - 1.0 == MATERIAL_FIREFLIES)
	{// Skybox... Skip...
		if (MAP_USE_PALETTE_ON_SKY > 0.0)
		{
			if (!(CONTRAST_STRENGTH == 1.0 && SATURATION_STRENGTH == 1.0 && BRIGHTNESS_STRENGTH == 1.0))
			{// C/S/B enabled...
				outColor.rgb = ContrastSaturationBrightness(outColor.rgb, CONTRAST_STRENGTH, SATURATION_STRENGTH, BRIGHTNESS_STRENGTH);
			}
		}

		if (TRUEHDR_ENABLED > 0.0)
		{// TrueHDR enabled...
			outColor.rgb = TrueHDR(outColor.rgb);
		}

		if (VIBRANCY > 0.0)
		{// Vibrancy enabled...
			outColor.rgb = Vibrancy(outColor.rgb, VIBRANCY);
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


#ifdef __BUMP_ENHANCE__
	norm.xyz = doBumpMap( u_DiffuseMap, texCoords, norm.xyz, 0.005);
#endif //__BUMP_ENHANCE__


	vec3 flatNorm = norm.xyz = normalize(norm.xyz);

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

#ifdef __USE_REAL_NORMALMAPS__

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

#else //!__USE_REAL_NORMALMAPS__

#ifdef __FAST_NORMAL_DETAIL__
	vec4 normalDetail = normalVector(outColor.rgb);
#else //!__FAST_NORMAL_DETAIL__
	vec4 normalDetail = normalVector(texCoords);
#endif //__FAST_NORMAL_DETAIL__

#endif //__USE_REAL_NORMALMAPS__

	// Simply offset the normal value based on the detail value... It looks good enough, but true PBR would probably want to use the tangent/bitangent below instead...
	normalDetail.rgb = normalize(clamp(normalDetail.xyz, 0.0, 1.0) * 2.0 - 1.0);

	//vec3 bump = normalize(mix(norm.xyz, normalDetail.xyz, u_Local3.a * (length(norm.xyz - normalDetail.xyz) / 3.0)));
	vec3 bump = normalize(mix(norm.xyz, normalDetail.xyz, 0.5 * (length((norm.xyz * 0.5 + 0.5) - (normalDetail.xyz * 0.5 + 0.5)) / 3.0)));
	norm.rgb = normalize(mix(norm.xyz, normalDetail.xyz, 0.25 * (length((norm.xyz * 0.5 + 0.5) - (normalDetail.xyz * 0.5 + 0.5)) / 3.0)));

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

	float snow = 0.0;
	float wetness = 0.0;

	if (SNOW_ENABLED > 0.0)
	{// snow testing
		if (position.z >= PROCEDURAL_SNOW_LOWEST_ELEVATION)
		{
			float snowMult = 1.0;

			if (PROCEDURAL_SNOW_LOWEST_ELEVATION > -999999.0)
			{// Elevation is enabled...
				float elevationRange = MAP_INFO_PLAYABLE_HEIGHT - PROCEDURAL_SNOW_LOWEST_ELEVATION;
				float pixelElevation = position.z - PROCEDURAL_SNOW_LOWEST_ELEVATION;
			
				snowMult = clamp(pow(clamp(pixelElevation / elevationRange, 0.0, 1.0) * 4.0, 2.0), 0.0, 1.0);
			}
			
			vec3 sBump = VaryNf(normalize(position.xyz), flatNorm.xyz, 2.0);

			snow = clamp(dot(normalize(sBump.rgb), vec3(0., 0., 1.)), 0., 1.);

			if (position.a - 1.0 == MATERIAL_GREENLEAVES)
				snow = pow(snow, 1.333);
			else
				snow = pow(snow, 0.4);

			snow *= snowMult;

			materialSettings.x += 0.05 * clamp(1.0 - materialSettings.x, 0.05, 0.95) * snow;
			materialSettings.y += 3.0 * clamp(1.0 - materialSettings.y, 0.05, 0.95) * snow;
		}
	}

	/*if (u_Local3.g != 0.0)
	{// wet testing
		wetness += 1.0;
	}*/


	//
	// This is the basics of creating a fake PBR look to the lighting. It could be replaced, or overridden by actual PBR pixel buffer inputs.
	//

	// This should give me roughly how close to grey this color is... For light colorization.. Highly colored stuff should get less color added to it...
	float greynessFactor = 1.0 - clamp((length(outColor.r - outColor.g) + length(outColor.r - outColor.b) + length(outColor.g - outColor.b)) / 3.0, 0.0, 1.0);
	greynessFactor = pow(greynessFactor, 64.0);

	// Also check how bright it is, so we can scale the lighting up/down...
	float brightnessFactor = 1.0 - clamp(max(outColor.r, max(outColor.g, outColor.b)), 0.0, 1.0);
	brightnessFactor = 1.0 - pow(brightnessFactor, 16.0);

	// It looks better to use slightly different cube and light reflection multipliers... Lights should always add some light, cubes should allow none on some pixels..
	float cubeReflectionFactor = clamp(greynessFactor * brightnessFactor, 0.5, 1.0) * reflectionPower;
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

	float origColorStrength = clamp(max(color.r, max(color.g, color.b)), 0.0, 1.0) * 0.75 + 0.25;
	float snowColorStrength = clamp(max(color.r, max(color.g, color.b)), 0.0, 1.0);

	if (snow > 0.0)
	{// snow testing
		//outColor.rgb = mix(outColor.rgb, vec3(0.4)*snow, snow);
		//float snowMix = 1.0 - clamp(snowColorStrength * 3.15, 0.0, 1.0);
		float snowMix = 1.0 - clamp(pow(snowColorStrength * 0.575 + 0.05, 0.34), 0.0, 1.0);
		snowMix *= snow;
		outColor.rgb = splatblend(outColor.rgb, 1.0 - snowMix, vec3(clamp(pow(snow, 0.001), 0.0, 1.0)), snowMix);
	}


	//
	// Now all the hard work...
	//

	vec3 specularColor = vec3(0.0);
	vec3 skyColor = vec3(0.0);
	vec3 emissiveCubeLightColor = vec3(0.0);
	vec3 emissiveCubeLightDirection = vec3(0.0);
	vec3 irradiance = vec3(1.0);

#ifndef __LQ_MODE__
	if (SKY_LIGHT_CONTRIBUTION > 0.0)
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

		if (NIGHT_SCALE > 0.0 && NIGHT_SCALE < 1.0)
		{// Mix between night and day colors...
			vec3 skyColorDay = texture(u_SkyCubeMap, reflected).rgb;
			vec3 skyColorNight = texture(u_SkyCubeMapNight, reflected).rgb;
			skyColor = mix(skyColorDay, skyColorNight, clamp(NIGHT_SCALE, 0.0, 1.0));
		}
		else if (NIGHT_SCALE >= 1.0)
		{// Night only colors...
			skyColor = texture(u_SkyCubeMapNight, reflected).rgb;
		}
		else
		{// Day only colors...
			skyColor = texture(u_SkyCubeMap, reflected).rgb;
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
#if defined(__CUBEMAPS__)
		if (CUBEMAP_ENABLED > 0.0)
		{// Cubemaps enabled...
			vec3 cubeLightColor = vec3(0.0);
			
			// This used to be done in rend2 code, now done here because I need u_CubeMapInfo.xyz to be cube origin for distance checks above... u_CubeMapInfo.w is now radius.
			vec4 cubeInfo = u_CubeMapInfo;
			cubeInfo.xyz -= u_ViewOrigin.xyz;

			cubeInfo.w = pow(distance(u_ViewOrigin.xyz, u_CubeMapInfo.xyz), 3.0);

			cubeInfo.xyz *= 1.0 / cubeInfo.w;
			cubeInfo.w = 1.0 / cubeInfo.w;

			vec3 parallax = cubeInfo.xyz + cubeInfo.w * E;
			parallax.z *= -1.0;

			if (cubeReflectionFactor > 0.0 && NE > 0.0 && u_CubeMapStrength > 0.0)
			{
				float curDist = distance(u_ViewOrigin.xyz, position.xyz);
				float cubeDist = distance(u_CubeMapInfo.xyz, position.xyz);
				float cubeFade = (1.0 - clamp(curDist / CUBEMAP_CULLRANGE, 0.0, 1.0)) * (1.0 - clamp(cubeDist / CUBEMAP_CULLRANGE, 0.0, 1.0));

				if (cubeFade > 0.0)
				{
					cubeLightColor = textureLod(u_CubeMap, cubeRayDir + parallax, 7.0 - (cubeReflectionFactor * 7.0)).rgb;
					cubeLightColor += texture(u_CubeMap, cubeRayDir + parallax).rgb;
					//cubeLightColor += texture(u_CubeMap, rayDir + parallax).rgb;
					cubeLightColor /= 2.0;

					//cubeLightColor = clamp(ContrastSaturationBrightness(cubeLightColor, 3.0, 4.0, 0.3), 0.0, 1.0);
					//cubeLightColor = clamp(Vibrancy(cubeLightColor, 0.4), 0.0, 1.0);

					outColor.rgb = mix(outColor.rgb, outColor.rgb + cubeLightColor.rgb, clamp(NE * cubeFade * (u_CubeMapStrength * 20.0) * cubeReflectionFactor, 0.0, 1.0));
				}
			}
		}
		else
		{
			vec3 shiny = textureLod(u_WaterEdgeMap, ((cubeRayDir.xy + cubeRayDir.z) / 2.0) * 0.5 + 0.5, 5.5 - (cubeReflectionFactor * 5.5)).rgb;
			shiny = clamp(ContrastSaturationBrightness(shiny, 1.75, 1.0, 0.333), 0.0, 1.0);
			outColor.rgb = mix(outColor.rgb, outColor.rgb + shiny.rgb, clamp(NE * cubeReflectionFactor * (origColorStrength * 0.75 + 0.25), 0.0, 1.0));
		}
#else //!defined(__CUBEMAPS__)
		vec3 shiny = textureLod(u_WaterEdgeMap, ((cubeRayDir.xy + cubeRayDir.z) / 2.0) * 0.5 + 0.5, 5.5 - (cubeReflectionFactor * 5.5)).rgb;
		shiny = clamp(ContrastSaturationBrightness(shiny, 1.75, 1.0, 0.333), 0.0, 1.0);
		outColor.rgb = mix(outColor.rgb, outColor.rgb + shiny.rgb, clamp(NE * cubeReflectionFactor * (origColorStrength * 0.75 + 0.25), 0.0, 1.0));
#endif //defined(__CUBEMAPS__)
#endif //!__LQ_MODE__
	}

	//
	// SSDO input, if enabled...
	//
	vec4 occlusion = vec4(0.0);
	bool useOcclusion = false;

#ifndef __LQ_MODE__
	if (USE_SSDO == 1.0)
	{
		useOcclusion = true;
		occlusion = texture(u_HeightMap, texCoords);
	}
#endif //!__LQ_MODE__

	if (SKY_LIGHT_CONTRIBUTION > 0.0)
	{// Sky light contributions...
#ifndef __LQ_MODE__
		outColor.rgb = mix(outColor.rgb, outColor.rgb + skyColor, clamp(NE * SKY_LIGHT_CONTRIBUTION * cubeReflectionFactor * (origColorStrength * 0.75 + 0.25), 0.0, 1.0));
#endif //__LQ_MODE__
		outColor.rgb = mix(outColor.rgb, outColor.rgb + specularColor, clamp(pow(reflectVectorPower, 2.0) * cubeReflectionFactor * (origColorStrength * 0.75 + 0.25), 0.0, 1.0));
		//outColor.rgb = skyColor;
	}


	if (BLINN_PHONG_STRENGTH > 0.0)
	{// If r_blinnPhong is <= 0.0 then this is pointless...
		float phongFactor = (BLINN_PHONG_STRENGTH * 12.0) * SUN_PHONG_SCALE;

#define LIGHT_COLOR_POWER			4.0

		if (phongFactor > 0.0 && NIGHT_SCALE < 1.0)
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
				lightColor *= max(outColor.r, max(outColor.g, outColor.b)) * 0.9 + 0.1;
				lightColor *= clamp(1.0 - NIGHT_SCALE, 0.0, 1.0); // Day->Night scaling of sunlight...
				lightColor = clamp(lightColor, 0.0, 0.7);

				// Add vibrancy to light color at sunset/sunrise???
				if (NIGHT_SCALE > 0.0 && NIGHT_SCALE < 1.0)
				{// Vibrancy gets greater the closer we get to night time...
					float vib = NIGHT_SCALE;
					if (vib > 0.8)
					{// Scale back vibrancy to 0.0 just before nightfall...
						float downScale = 1.0 - vib;
						downScale *= 4.0;
						vib = mix(vib, 0.0, clamp(downScale, 0.0, 1.0));
					}
					lightColor = Vibrancy(lightColor, clamp(vib * 4.0, 0.0, 1.0));
				}

				lightColor.rgb *= lightsReflectionFactor * phongFactor * irradiance * origColorStrength * 8.0;

				vec3 blinn = blinn_phong(position.xyz, outColor.rgb, N, E, normalize(-sunDir), lightColor * 0.06, lightColor, 1.0, u_PrimaryLightOrigin.xyz);
				lightColor.rgb += blinn + (blinn * wetness);

				outColor.rgb = outColor.rgb + max(lightColor, vec3(0.0));
			}
		}

		if (u_lightCount > 0.0 && specularReflectivePower > 0.0)
		{
			phongFactor = BLINN_PHONG_STRENGTH * 12.0;

			if (phongFactor <= 0.0)
			{// Never allow no phong...
				phongFactor = 1.0;
			}

			vec3 addedLight = vec3(0.0);
			float maxBright = clamp(max(outColor.r, max(outColor.g, outColor.b)), 0.0, 1.0);
			float power = maxBright * 0.85;
			power = clamp(pow(power, LIGHT_COLOR_POWER) + 0.333, 0.0, 1.0);

			for (int li = 0; li < u_lightCount; li++)
			{
				vec3 lightPos = u_lightPositions2[li].xyz;

				float lightDist = distance(lightPos, position.xyz);

				if (u_lightHeightScales[li] > 0.0)
				{// ignore height differences, check later...
					lightDist -= length(lightPos.z - position.z);
				}

				float lightPlayerDist = distance(lightPos.xyz, u_ViewOrigin.xyz);

				float lightDistMult = 1.0 - clamp((lightPlayerDist / MAX_DEFERRED_LIGHT_RANGE), 0.0, 1.0);
				lightDistMult = pow(lightDistMult, 2.0);

				if (lightDistMult <= 0.0) continue;

				// Attenuation...
				float lightFade = 1.0 - clamp((lightDist * lightDist) / (u_lightDistances[li] * u_lightDistances[li]), 0.0, 1.0);
				
				if (lightFade <= 0.0) continue;

				lightFade = pow(lightFade, 2.0);

				float lightStrength = lightDistMult * lightFade * specularReflectivePower * 0.5;

				//float maxLightsScale = mix(0.0, 1.0, clamp(pow(1.0 - (float(li) / float(u_lightMax)), u_Local3.r), 0.0, 1.0));
				float maxLightsScale = mix(0.01, 1.0, clamp(pow(1.0 - (lightPlayerDist / u_lightMaxDistance), 0.5), 0.0, 1.0));
				lightStrength *= maxLightsScale;

				if (lightStrength > 0.0)
				{
					vec3 lightColor = (u_lightColors[li].rgb / length(u_lightColors[li].rgb)) * MAP_EMISSIVE_COLOR_SCALE * maxLightsScale; // Normalize.
					vec3 lightDir = normalize(lightPos - position.xyz);
					float light_occlusion = 1.0;
					float selfShadow = clamp(pow(clamp(dot(-lightDir.rgb, bump.rgb/*norm.rgb*/), 0.0, 1.0), 8.0) * 0.6 + 0.6, 0.0, 1.0);
				
					lightColor = lightColor * power * irradiance * origColorStrength * 2.0;// * maxStr;

					addedLight.rgb += lightColor * lightStrength * 0.333 * selfShadow;

#ifndef __LQ_MODE__
					if (useOcclusion)
					{
						light_occlusion = (1.0 - clamp(dot(vec4(-lightDir*E, 1.0), occlusion), 0.0, 1.0));
					}
#endif //__LQ_MODE__

					vec3 blinn = blinn_phong(position.xyz, outColor.rgb, N, E, lightDir, lightColor * 0.06, lightColor, mix(0.1, 0.5, clamp(lightsReflectionFactor, 0.0, 1.0)) * clamp(lightStrength * light_occlusion * phongFactor, 0.0, 1.0), lightPos) * lightFade * selfShadow;
					addedLight.rgb += blinn + (blinn * wetness);
				}
			}

			addedLight.rgb *= lightsReflectionFactor; // More grey colors get more colorization from lights...
			outColor.rgb = outColor.rgb + max(addedLight, vec3(0.0));
		}
	}

#if defined(__SCREEN_SPACE_REFLECTIONS__)
	if (REFLECTIONS_ENABLED > 0.0 && (ssrReflectivePower > 0.0 || wetness > 0.0) && position.a - 1.0 != MATERIAL_WATER && !changedToWater)
	{
#if 1
		if (wetness > 0.0)
			outColor.rgb = AddReflection(texCoords, position, flatNorm, outColor.rgb, ssrReflectivePower + 0.37504, ssReflection);
		else
			outColor.rgb = AddReflection(texCoords, position, flatNorm, outColor.rgb, ssrReflectivePower, ssReflection);
#else
		/*
		vec3 rf = reflect(E.xyz, flatNorm.xyz);
		float maxDistance = u_Local3.r;
		float lenz = length(rf.z);
		float offset = (maxDistance + ((1.0-lenz) * maxDistance));
		float tcoff = (1.0 - texCoords.y) - (offset / u_Local3.g);
		tcoff -= distance(0.5, maxDistance);
		//float tcoff = (offset - texCoords.y) * offset;
		*/
		float tcoff = reflect(-texCoords.y, flatNorm.z);
		//vec3 reflection = texture(u_DiffuseMap, vec2(texCoords.x, 1.0 - tcoff)).rgb;
		vec3 reflection = texture(u_DiffuseMap, vec2(texCoords.x, 1.0 - (tcoff + (distance(texCoords.y, 1.0 - (E.z * 0.5 + 0.5)) * u_Local3.a)))).rgb;
		outColor.rgb += reflection;
#endif
	}
#endif //defined(__SCREEN_SPACE_REFLECTIONS__)

#if defined(__AMBIENT_OCCLUSION__)
	if (AO_TYPE == 1.0)
	{// Fast AO enabled...
		float ao = calculateAO(sunDir, N * 10000.0);
		//float selfShadow = bad_ao(bump.xyz);
		//float selfShadow = clamp(pow(clamp(dot(-sunDir.rgb, bump.rgb), 0.0, 1.0), 8.0) * 0.6 + 0.6, 0.0, 1.0);
		float selfShadow = clamp(pow(clamp(dot(-sunDir.rgb, bump.rgb), 0.0, 1.0), 8.0), 0.0, 1.0);
		//ao = clamp(ao * AO_MULTBRIGHT + AO_MINBRIGHT, AO_MINBRIGHT, 1.0);
		ao = clamp(((ao + selfShadow) / 2.0) * AO_MULTBRIGHT + AO_MINBRIGHT, AO_MINBRIGHT, 1.0);
		//ao = clamp((ao * selfShadow) * AO_MULTBRIGHT + AO_MINBRIGHT, AO_MINBRIGHT, 1.0);
		outColor.rgb *= ao;
	}
#endif //defined(__AMBIENT_OCCLUSION__)

#if defined(__ENHANCED_AO__)
	if (AO_TYPE >= 2.0)
	{// Better, HQ AO enabled...
		float msao = 0.0;

		if (AO_TYPE >= 3.0)
		{
			int width = int(AO_TYPE-2.0);
			float numSamples = 0.0;
		
			for (int x = -width; x <= width; x+=2)
			{
				for (int y = -width; y <= width; y+=2)
				{
					vec2 coord = texCoords + (vec2(float(x), float(y)) * pixel);
					msao += textureLod(u_SteepMap, coord, 0.0).x;
					numSamples += 1.0;
				}
			}

			msao /= numSamples;
		}
		else
		{
			msao = textureLod(u_SteepMap, texCoords, 0.0).x;
		}

		float sao = clamp(msao, 0.0, 1.0);
		sao = clamp(sao * AO_MULTBRIGHT + AO_MINBRIGHT, AO_MINBRIGHT, 1.0);
		outColor.rgb *= sao;
	}
#endif //defined(__ENHANCED_AO__)

#if defined(USE_SHADOWMAP) && !defined(__LQ_MODE__)
	if (SHADOWS_ENABLED > 0.0 && NIGHT_SCALE < 1.0)
	{
		float shadowValue = texture(u_ShadowMap, texCoords).r;

		shadowValue = pow(shadowValue, 1.5);

#define sm_cont_1 ( 64.0 / 255.0)
#define sm_cont_2 (255.0 / 200.0)
		shadowValue = clamp((clamp(shadowValue - sm_cont_1, 0.0, 1.0)) * sm_cont_2, 0.0, 1.0);
		float finalShadow = clamp(shadowValue + SHADOW_MINBRIGHT, SHADOW_MINBRIGHT, SHADOW_MAXBRIGHT);
		finalShadow = mix(finalShadow, 1.0, clamp(NIGHT_SCALE, 0.0, 1.0)); // Dampen out shadows at sunrise/sunset...
		outColor.rgb *= finalShadow;
	}
#endif //defined(USE_SHADOWMAP) && !defined(__LQ_MODE__)

	// De-emphasize (darken) the distant map a bit...
	//float depth = clamp(pow(1.0 - clamp(distance(position.xyz, u_ViewOrigin.xyz) / 65536.0, 0.0, 1.0), 4.5), 0.35, 1.0); // darken distant
	//float depth = clamp(pow(clamp(distance(position.xyz, u_ViewOrigin.xyz) / 65536.0, 0.0, 1.0), 4.5) * 100000.0 + 1.0, 1.0, 2.0); // brighten distant
	//outColor.rgb *= depth;


	if (!(CONTRAST_STRENGTH == 1.0 && SATURATION_STRENGTH == 1.0 && BRIGHTNESS_STRENGTH == 1.0))
	{// C/S/B enabled...
		outColor.rgb = ContrastSaturationBrightness(outColor.rgb, CONTRAST_STRENGTH, SATURATION_STRENGTH, BRIGHTNESS_STRENGTH);
	}

	if (TRUEHDR_ENABLED > 0.0)
	{// TrueHDR enabled...
		outColor.rgb = TrueHDR( outColor.rgb );
	}

	if (VIBRANCY > 0.0)
	{// Vibrancy enabled...
		outColor.rgb = Vibrancy( outColor.rgb, VIBRANCY );
	}

	outColor.rgb = clamp(outColor.rgb, 0.0, 1.0);
	gl_FragColor = outColor;
}

