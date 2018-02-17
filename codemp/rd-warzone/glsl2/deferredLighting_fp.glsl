﻿#define __AMBIENT_OCCLUSION__
#define __ENHANCED_AO__
#define __ENVMAP__
#define __RANDOMIZE_LIGHT_PIXELS__
#define __SCREEN_SPACE_REFLECTIONS__
//#define __HEIGHTMAP_SHADOWS__

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
uniform vec4		u_Local8; // enableReflections, MAP_HDR_MIN, MAP_HDR_MAX, MAP_INFO_SIZE[2]

uniform vec4		u_ViewInfo; // znear, zfar, zfar / znear, fov
uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

uniform vec4		u_CubeMapInfo;
uniform float		u_CubeMapStrength;

#define MAX_DEFERRED_LIGHTS 64//128

uniform int			u_lightCount;
//uniform vec2		u_lightPositions[MAX_DEFERRED_LIGHTS];
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


vec3 RB_PBR_DefaultsForMaterial(float MATERIAL_TYPE)
{
	vec3 settings = vec3(0.0);

	float specularScale = 0.0;
	float cubemapScale = 0.0;
	float parallaxScale = 0.0;

	switch (int(MATERIAL_TYPE))
	{
	case MATERIAL_WATER:			// 13			// light covering of water on a surface
		specularScale = 1.0;
		cubemapScale = 0.7;
		break;
	case MATERIAL_SHORTGRASS:		// 5			// manicured lawn
		specularScale = 0.75;
		cubemapScale = 0.35;
		break;
	case MATERIAL_LONGGRASS:		// 6			// long jungle grass
		specularScale = 0.75;
		cubemapScale = 0.35;
		break;
	case MATERIAL_SAND:				// 8			// sandy beach
		specularScale = 0.35;
		cubemapScale = 0.0;
		break;
	case MATERIAL_CARPET:			// 27			// lush carpet
		specularScale = 0.25;
		cubemapScale = 0.0;
		break;
	case MATERIAL_GRAVEL:			// 9			// lots of small stones
		specularScale = 0.30;
		cubemapScale = 0.0;
		break;
	case MATERIAL_ROCK:				// 23			//
		specularScale = 0.22;
		cubemapScale = 0.0;
		break;
	case MATERIAL_TILES:			// 26			// tiled floor
		specularScale = 0.56;
		cubemapScale = 0.15;
		break;
	case MATERIAL_SOLIDWOOD:		// 1			// freshly cut timber
		specularScale = 0.05;
		cubemapScale = 0.0;
		break;
	case MATERIAL_HOLLOWWOOD:		// 2			// termite infested creaky wood
		specularScale = 0.025;
		cubemapScale = 0.0;
		break;
	case MATERIAL_SOLIDMETAL:		// 3			// solid girders
		specularScale = 0.98;
		cubemapScale = 0.98;
		break;
	case MATERIAL_HOLLOWMETAL:		// 4			// hollow metal machines -- UQ1: Used for weapons to force lower parallax and high reflection...
		specularScale = 1.0;
		cubemapScale = 0.98;
		break;
	case MATERIAL_DRYLEAVES:		// 19			// dried up leaves on the floor
		specularScale = 0.35;
		cubemapScale = 0.0;
		break;
	case MATERIAL_GREENLEAVES:		// 20			// fresh leaves still on a tree
		specularScale = 0.95;
		cubemapScale = 0.35;
		break;
	case MATERIAL_FABRIC:			// 21			// Cotton sheets
		specularScale = 0.45;
		cubemapScale = 0.35;
		break;
	case MATERIAL_CANVAS:			// 22			// tent material
		specularScale = 0.35;
		cubemapScale = 0.0;
		break;
	case MATERIAL_MARBLE:			// 12			// marble floors
		specularScale = 0.65;
		cubemapScale = 0.46;
		break;
	case MATERIAL_SNOW:				// 14			// freshly laid snow
		specularScale = 0.35;
		cubemapScale = 0.65;
		break;
	case MATERIAL_MUD:				// 17			// wet soil
		specularScale = 0.25;
		cubemapScale = 0.0;
		break;
	case MATERIAL_DIRT:				// 7			// hard mud
		specularScale = 0.15;
		cubemapScale = 0.0;
		break;
	case MATERIAL_CONCRETE:			// 11			// hardened concrete pavement
		specularScale = 0.375;
		cubemapScale = 0.0;
		break;
	case MATERIAL_FLESH:			// 16			// hung meat, corpses in the world
		specularScale = 0.25;
		cubemapScale = 0.0;
		break;
	case MATERIAL_RUBBER:			// 24			// hard tire like rubber
		specularScale = 0.25;
		cubemapScale = 0.0;
		break;
	case MATERIAL_PLASTIC:			// 25			//
		specularScale = 0.58;
		cubemapScale = 0.48;
		break;
	case MATERIAL_PLASTER:			// 28			// drywall style plaster
		specularScale = 0.3;
		cubemapScale = 0.0;
		break;
	case MATERIAL_SHATTERGLASS:		// 29			// glass with the Crisis Zone style shattering
		specularScale = 0.35;
		cubemapScale = 0.67;
		break;
	case MATERIAL_ARMOR:			// 30			// body armor
		specularScale = 0.65;
		cubemapScale = 0.66;
		break;
	case MATERIAL_ICE:				// 15			// packed snow/solid ice
		specularScale = 0.35;
		cubemapScale = 0.78;
		break;
	case MATERIAL_GLASS:			// 10			//
		specularScale = 0.35;
		cubemapScale = 0.70;
		break;
	case MATERIAL_BPGLASS:			// 18			// bulletproof glass
		specularScale = 0.33;
		cubemapScale = 0.70;
		break;
	case MATERIAL_COMPUTER:			// 31			// computers/electronic equipment
		specularScale = 0.52;
		cubemapScale = 0.68;
		break;
	default:
		specularScale = 0.4;
		cubemapScale = 0.2;
		break;
	}

	specularScale = specularScale * 0.5 + 0.5;
	cubemapScale = cubemapScale * 0.75 + 0.25;

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

#ifdef __SCREEN_SPACE_REFLECTIONS__
#define pw pixel.x
#define ph pixel.y
vec3 AddReflection(vec2 coord, vec4 positionMap, vec3 origNorm, vec3 inColor, float reflectiveness)
{
	if (reflectiveness <= 0.5)
	{// Top of screen pixel is water, don't check...
		return inColor;
	}

	float pixelDistance = distance(positionMap.xyz, u_ViewOrigin.xyz);

	// Quick scan for pixel that is not water...
	float QLAND_Y = 0.0;

	for (float y = coord.y; y <= 1.0; y += ph * 5.0)
	{
		vec3 norm = DecodeNormal(textureLod(u_NormalMap, vec2(coord.x, y), 0.0).xy);
		vec4 pMap = textureLod(u_PositionMap, vec2(coord.x, y), 0.0);

		float pMapDistance = distance(pMap.xyz, u_ViewOrigin.xyz);

		if (pMap.a > 1.0 && pMap.xyz != vec3(0.0) && distance(pMap.xyz, u_ViewOrigin.xyz) <= pixelDistance)
		{
			continue;
		}

		if (norm.z * 0.5 + 0.5 < 0.75 && distance(norm.xyz, origNorm.xyz) > 0.0)
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
	
	QLAND_Y -= ph * 5.0;
	
	// Full scan from within 5 px for the real 1st pixel...
	float upPos = coord.y;
	float LAND_Y = 0.0;

	for (float y = QLAND_Y; y <= 1.0; y += ph)
	{
		vec3 norm = DecodeNormal(textureLod(u_NormalMap, vec2(coord.x, y), 0.0).xy);
		vec4 pMap = textureLod(u_PositionMap, vec2(coord.x, y), 0.0);
		
		float pMapDistance = distance(pMap.xyz, u_ViewOrigin.xyz);

		if (pMap.a > 1.0 && pMap.xyz != vec3(0.0) && distance(pMap.xyz, u_ViewOrigin.xyz) <= pixelDistance)
		{
			continue;
		}

		if (norm.z * 0.5 + 0.5 < 0.75 && distance(norm.xyz, origNorm.xyz) > 0.0)
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

	/*if (pMap.a != finalMaterial && pMap.a > 1.0)
	{// After moving upwards, we ended up hitting a second mateiral type (surface), this may not be valid, so skip this pixel...
		return inColor;
	}*/

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

	vec4 glowColor = textureLod(u_GlowMap, vec2(coord.x, upPos), 0.0);
	vec4 landColor = textureLod(u_DiffuseMap, vec2(coord.x, upPos), 0.0);
	return mix(inColor.rgb, inColor.rgb + landColor.rgb + (glowColor.rgb * 3.5), clamp(strength * reflectiveness * 4.0, 0.0, 1.0));
}
#endif //__SCREEN_SPACE_REFLECTIONS__

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
	return mix(coolMap, heatMap, clamp(warmth, 0.0, 1.0));
}
#endif //__ENVMAP__


#ifdef __HEIGHTMAP_SHADOWS__
vec3 getTexture(vec2 uv) {
	return texture(u_PositionMap, uv).rgb;
}

float getHeightmap(vec2 uv) {
	//const float inv3 = 1.0 / 3.0;
	//return dot(getTexture(uv), vec3(inv3));
	return (getTexture(uv).z / u_Local8.a) * 2.0 - 1.0;
}

float getShadow(vec2 uv, vec3 lp) {
	const int steps = 64;
	const float invSteps = 1.0 / float(steps);

	vec3 inc = lp.xzy * invSteps;
	float heightmap = getHeightmap(uv);
	vec3 position = vec3(uv, heightmap);

	float shadow = 1.0;

	for (int i = 0; i < steps && position.z < 1.0; i++) {
		position += inc;
		float offsetHightmap = getHeightmap(position.xy);

		if (offsetHightmap > position.z)
		{
			return 0.0;
		}

	}
	return 1.0;
}

float GetShadow(vec2 texCoords, vec3 lightPos)
{
	//vec3 lightPos = normalize(vec3(u_Local3.rg, 1.0));
	return getShadow(texCoords, lightPos);
}
#endif //__HEIGHTMAP_SHADOWS__


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
//#define __LIGHTING_TYPE_1__
//#define __LIGHTING_TYPE_2__
#define __LIGHTING_TYPE_3__

#if defined(__LIGHTING_TYPE_1__)
// lighting
float getdiffuse(vec3 n, vec3 l, float p) {
	float ndotl = clamp(dot(n, l), 0.5, 0.9);
	return pow(ndotl, p);
}
float getspecular(vec3 n, vec3 l, vec3 e, float s) {
	//float nrm = (s + 8.0) / (3.1415 * 8.0);
	float ndotl = clamp(max(dot(reflect(e, n), l), 0.0), 0.1, 1.0);
	return clamp(pow(ndotl, s), 0.1, 1.0);// * nrm;
}
vec3 blinn_phong(vec3 normal, vec3 view, vec3 light, vec3 diffuseColor, vec3 specularColor, float specPower) {
	vec3 diffuse = diffuseColor * getdiffuse(normal, light, 2.0) * 6.0;
	vec3 specular = specularColor * getspecular(-normal, light, -view, 0.6) * 8.0 * specPower;
	
	vec3 lightBounce = reflect(-view, -normal) * -light;
	vec3 specularBounce = specularColor * getspecular(-normal, lightBounce, -view, 0.6) * 16.0 * specPower;

	return diffuse + specular + specularBounce;
}
#elif defined(__LIGHTING_TYPE_2__)
// lighting
float getdiffuse(vec3 n, vec3 l, float p) {
	return pow(dot(n, l) * 0.4 + 0.6, p);
}
vec3 blinn_phong(vec3 normal, vec3 view, vec3 light, vec3 diffuseColor, vec3 specularColor, float specPower) {
	vec3 diffuse = diffuseColor * getdiffuse(normal, light, 2.0) * 6.0;
	vec3 specular = vec3(0.0);

	float lambertian2 = dot(light, normal);

	if(lambertian2 > 0.0)
	{// this is blinn phong
		float fresnel = clamp(1.0 - dot(normal, -view), 0.0, 1.0);
		fresnel = pow(fresnel, 3.0) * 0.65;

		vec3 mirrorEye = (2.0 * dot(view, normal) * normal - view);
		vec3 halfDir2 = normalize(light + mirrorEye);
		float specAngle = max(dot(halfDir2, normal), 0.0);
		float spec = pow(specAngle, 16.0);
		specular = specularColor * clamp(1.0 - fresnel, 0.4, 1.0) * spec * 8.0 * specPower;
	}

	return diffuse + specular;
}
#elif defined(__LIGHTING_TYPE_3__)
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

	//vec3 L = lightPos - pos;
	//vec3 l = normalize(L);
	//vec3 h = normalize(V + l);

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
#else //__LIGHTING_TYPE_0__
// Blinn-Phong shading model with rim lighting (diffuse light bleeding to the other side).
// `normal`, `view` and `light` should be normalized.
vec3 blinn_phong(vec3 normal, vec3 view, vec3 light, vec3 diffuseColor, vec3 specularColor, float specPower) {
	vec3 halfLV = normalize(-light + view);
	float ndl = max(dot(normal, halfLV), 0.0);
	float spe = pow(ndl, 0.3);
	float spe2 = pow(ndl, 32.0);
	float dif = dot(normal, -light) * 0.5 + 0.75;
	return (dif*diffuseColor) + (spe*specularColor*0.75*specPower) + (spe2*specularColor*0.25*specPower);
}
#endif //__LIGHTING_TYPE_0__

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
	vec4 position = textureLod(u_PositionMap, var_TexCoords, 0.0);
	vec4 outColor = vec4(color.rgb, 1.0);

	if (position.a-1.0 == MATERIAL_SKY || position.a-1.0 == MATERIAL_SUN || position.a-1.0 == MATERIAL_GLASS /*|| position.a-1.0 == MATERIAL_NONE*/)
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

		outColor.rgb = clamp(outColor.rgb, 0.0, 1.0);
		gl_FragColor = outColor;
		return;
	}


	vec2 texCoords = var_TexCoords;
	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);
	vec3 materialSettings = RB_PBR_DefaultsForMaterial(position.a-1.0);


	/*if (u_Local6.a > 0.0)
	{// Sunset, Sunrise, and Night times... Scale down screen color, before adding lighting...
		vec3 nightColor = vec3(outColor.rgb * 0.35);
		outColor.rgb = mix(outColor.rgb, nightColor, clamp(u_Local6.a, 0.0, 1.0));
	}*/



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
	norm.xyz = DecodeNormal(norm.xy);

#ifdef __SCREEN_SPACE_REFLECTIONS__
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

	vec3 origNorm = norm.xyz;
#endif //__SCREEN_SPACE_REFLECTIONS__

	vec4 normalDetail = textureLod(u_OverlayMap, texCoords, 0.0);

	vec3 cubeNorm = norm.rgb;
	
	if (normalDetail.a < 1.0)
	{// Don't have real normalmap, make normals for this pixel...
		normalDetail = normalVector(texCoords);
	}

	normalDetail.rgb = normalize(clamp(normalDetail.rgb, 0.0, 1.0) * 2.0 - 1.0);
	//normalDetail.rgb *= 0.25;
	//norm.rgb = normalize(norm.rgb + normalDetail.rgb);
	norm.rgb = normalize(mix(norm.rgb, normalDetail.rgb, 0.5 * (length(norm.rgb - normalDetail.rgb) / 3.0)));
	//norm.rgb = normalize(mix(norm.rgb, normalDetail.rgb, 0.25 * (1.0 - (length(norm.rgb - normalDetail.rgb) / 3.0)) ));

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

	vec3 surfaceToCamera = E;
#define rd reflect(surfaceToCamera, N)

	float specularPower = clamp(materialSettings.x * 0.04, 0.0, 1.0);
	float gloss = clamp(clamp((materialSettings.x + materialSettings.y) * 0.5, 0.0, 1.0) * 1.6, 0.0, 1.0);

	vec3 skyColor = vec3(0.0);

	if (u_Local7.a > 0.0)
	{// Sky light contributions...
		vec3 m_ViewDir = to_pos_norm;
		vec3 R = reflect(E, cubeNorm);
					
		// This used to be done in rend2 code, now done here because I need u_CubeMapInfo.xyz to be cube origin for distance checks above... u_CubeMapInfo.w is now radius.
		vec4 cubeInfo = vec4(0.0, 0.0, 0.0, 1.0);//u_CubeMapInfo;
		cubeInfo.xyz -= u_ViewOrigin.xyz;

		cubeInfo.w = pow(distance(u_ViewOrigin.xyz, vec3(0.0, 0.0, 0.0)), 3.0);

		cubeInfo.xyz *= 1.0 / cubeInfo.w;
		cubeInfo.w = 1.0 / cubeInfo.w;
					
		vec3 parallax = cubeInfo.xyz + cubeInfo.w * m_ViewDir;
		parallax.z *= -1.0;
		
		vec3 reflected = R + parallax;

		reflected = vec3(-reflected.y, -reflected.z, -reflected.x);

		if (u_Local6.a > 0.0 && u_Local6.a < 1.0)
		{// Mix between night and day colors...
			vec3 skyColorDay = texture(u_SkyCubeMap, reflected).rgb;
			vec3 skyColorNight = texture(u_SkyCubeMapNight, reflected).rgb;
			skyColor = mix(skyColorDay, skyColorNight, clamp(u_Local6.a, 0.0, 1.0));
		}
		else if (u_Local6.a >= 1.0)
		{// Night only colors...
			skyColor = texture(u_SkyCubeMapNight, reflected).rgb;
		}
		else
		{// Day only colors...
			skyColor = texture(u_SkyCubeMap, reflected).rgb;
		}

		skyColor = ContrastSaturationBrightness(skyColor, 1.0, 2.0, 0.7);
		skyColor = Vibrancy( skyColor, 0.4 );

#if 0
		if (u_Local3.a > 0.0)
		{
			gl_FragColor = vec4(skyColor.rgb, 1.0);
			return;
		}
#endif
	}

	vec3 specular;
	float NE = clamp(length(dot(N, E)), 0.0, 1.0);
	float reflectPower = pow(clamp(materialSettings.x*NE, 0.0, 1.0), 16.0) * materialSettings.g;
	
	// This should give me roughly how close to grey this color is... We can use this for colorization factors.. Highly colored stuff should get less color added to it...
	//float greynessFactor = 1.0 - clamp(distance(vec3(length(outColor.rgb) / 3.0), outColor.rgb) / 3.0, 0.0, 1.0);
	float greynessFactor = 1.0 - clamp((length(outColor.r - outColor.g) + length(outColor.r - outColor.b)) / 2.0, 0.0, 1.0);
	float brightnessFactor = 1.0 - clamp(max(outColor.r, max(outColor.g, outColor.b)), 0.0, 1.0);
	greynessFactor = clamp(pow(greynessFactor, 64.0/*u_Local3.r*/), 0.0, 1.0);
	brightnessFactor = 1.0 - clamp(pow(brightnessFactor, 16.0/*u_Local3.g*/), 0.0, 1.0);

	float glossinessFactor = greynessFactor * brightnessFactor;
	float lightGlossinessFactor = (greynessFactor * 0.5 + 0.5) * (brightnessFactor * 0.5 + 0.5);

#if 0
	if (u_Local3.b == 3.0)
	{
		outColor.rgb = vec3(glossinessFactor);
		gl_FragColor = outColor;
		return;
	}
	else if (u_Local3.b == 2.0)
	{
		outColor.rgb = vec3(brightnessFactor);
		gl_FragColor = outColor;
		return;
	}
	else if (u_Local3.b == 1.0)
	{
		outColor.rgb = vec3(greynessFactor);
		gl_FragColor = outColor;
		return;
	}
#endif

//#define __DEBUG_LIGHT__

#ifdef __DEBUG_LIGHT__
	if (u_Local3.r == 1.0)
	{// Debug position map by showing pixel distance from view...
		outColor.rgb = vec3(clamp(materialSettings.x, 0.0, 1.0));
		outColor.a = 1.0;
		gl_FragColor = outColor;
		return;
	}

	if (u_Local3.r == 2.0)
	{// Debug position map by showing pixel distance from view...
		outColor.rgb = vec3(clamp(materialSettings.y, 0.0, 1.0));
		outColor.a = 1.0;
		gl_FragColor = outColor;
		return;
	}

	if (u_Local3.r == 3.0)
	{// Debug position map by showing pixel distance from view...
		outColor.rgb = vec3(clamp(lightGlossinessFactor, 0.0, 1.0));
		outColor.a = 1.0;
		gl_FragColor = outColor;
		return;
	}
#endif

	if (specularPower > 0.0)
	{
		specular = ContrastSaturationBrightness(outColor.rgb, 1.25, 0.05, 1.0);
		specular.rgb = clamp(vec3(length(specular.rgb) / 3.0), 0.0, 1.0);
		specular.rgb *= clamp(materialSettings.r, 0.0, 1.0);

		if (position.a-1.0 == MATERIAL_SOLIDMETAL || position.a-1.0 == MATERIAL_HOLLOWMETAL)
		{// Re-add color to shiny reflections...
			specular.rgb = skyColor * specular.rgb;
		}
		else
		{
			specular.rgb = mix(specular.rgb, skyColor * specular.rgb, clamp(materialSettings.g, 0.0, 1.0));
		}

		//vec3 reflectance = EnvironmentBRDF(gloss, NE, specular.rgb);
		vec3 cubeLightColor = vec3(0.0);
		float curDist = distance(u_ViewOrigin.xyz, position.xyz);
		//float cubeDist = distance(u_CubeMapInfo.xyz, position.xyz);

		if (u_Local7.r > 0.0)
		{// Cubemaps enabled...
			if (materialSettings.x > 0.0 && materialSettings.y > 0.0)
			{
				float cubeFade = 0.0;
		
				if (curDist < u_Local7.g)
				{
					cubeFade = clamp((1.0 - clamp(curDist / u_Local7.g, 0.0, 1.0)) * materialSettings.y * (u_CubeMapStrength * 20.0), 0.0, 1.0);
					//cubeFade = 1.0 - clamp(curDist / u_Local7.g, 0.0, 1.0);
				}

				if (cubeFade > 0.0)
				{
					vec3 m_ViewDir = to_pos_norm;
					vec3 R = reflect(E, cubeNorm);
					
					// This used to be done in rend2 code, now done here because I need u_CubeMapInfo.xyz to be cube origin for distance checks above... u_CubeMapInfo.w is now radius.
					vec4 cubeInfo = u_CubeMapInfo;
					cubeInfo.xyz -= u_ViewOrigin.xyz;

					cubeInfo.w = pow(distance(u_ViewOrigin.xyz, u_CubeMapInfo.xyz), 3.0);

					cubeInfo.xyz *= 1.0 / cubeInfo.w;
					cubeInfo.w = 1.0 / cubeInfo.w;
					
					vec3 parallax = cubeInfo.xyz + cubeInfo.w * m_ViewDir;
					parallax.z *= -1.0;
					
					//cubeLightColor = textureLod(u_CubeMap, R + parallax, 7.0 - gloss * 7.0).rgb;
					cubeLightColor = texture(u_CubeMap, R + parallax).rgb;

					float cPx = 1.0 / u_Local7.b;
					float blurCount = 1.0;
					for (float x = -4.0; x <= 4.0; x += 2.0)
					{
						for (float y = -4.0; y <= 4.0; y += 2.0)
						{
							vec3 blurColor = texture(u_CubeMap, (R + parallax) + (vec3(x,y,0.0) * cPx)).rgb;
							cubeLightColor += blurColor;
							blurCount += 1.0;
						}
					}
					cubeLightColor.rgb /= blurCount;

					// Maybe if not metal, here, we should add contrast to only show the brights as reflection...
					//outColor.rgb = mix(outColor.rgb, outColor.rgb + (cubeLightColor * reflectance), clamp(cubeFade * gloss, 0.0, 1.0));

					//cubeLightColor *= reflectance;

					//float cubeFade = 1.0 - clamp(curDist / u_Local7.g, 0.0, 1.0);
					outColor.rgb = mix(outColor.rgb, outColor.rgb + cubeLightColor.rgb, reflectPower * cubeFade * (u_CubeMapStrength * 20.0) * glossinessFactor/*greynessFactor*/);
				}
			}
		}
	}

	vec3 PrimaryLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);

	vec4 occlusion = vec4(0.0);
	bool useOcclusion = false;

	if (u_Local2.r == 1.0)
	{
		useOcclusion = true;
		occlusion = texture(u_HeightMap, texCoords);
	}

	float reflectivePower = materialSettings.x;


	if (u_Local7.a > 0.0)
	{// Sky light contributions...
		outColor.rgb = mix(outColor.rgb, outColor.rgb + skyColor, clamp(pow(materialSettings.y, 2.0) * u_Local7.a * glossinessFactor, 0.0, 1.0));
		outColor.rgb = mix(outColor.rgb, outColor.rgb + specular, clamp(pow(reflectPower, 2.0) * glossinessFactor, 0.0, 1.0));
	}


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

			float maxBright = clamp(max(outColor.r, max(outColor.g, outColor.b)), 0.0, 1.0);
			float power = clamp(pow(maxBright * 0.75, LIGHT_COLOR_POWER) + 0.333, 0.0, 1.0);
		
			vec3 lightColor = u_PrimaryLightColor.rgb;

			float lightMult = clamp(reflectivePower * power * light_occlusion, 0.0, 1.0);

			if (lightMult > 0.0)
			{
				lightColor *= lightMult;
#ifdef __LIGHTING_TYPE_3__
				lightColor = blinn_phong(position.xyz, outColor.rgb, N, E, normalize(-to_light_norm), outColor.rgb * lightColor, outColor.rgb * lightColor, 1.0, u_PrimaryLightOrigin.xyz);
#else //!__LIGHTING_TYPE_3__
				lightColor = blinn_phong(N, E, normalize(-to_light_norm), outColor.rgb * lightColor, (outColor.rgb * lightColor), 1.0);
#endif //__LIGHTING_TYPE_3__
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

				lightColor.rgb *= lightGlossinessFactor * phongFactor;
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

				float lightDistMult = 1.0 - clamp((distance(lightPos.xyz, u_ViewOrigin.xyz) / 4096.0), 0.0, 1.0);

				// Attenuation...
				float lightFade = 1.0 - clamp((lightDist * lightDist) / (u_lightDistances[li] * u_lightDistances[li]), 0.0, 1.0);
				lightFade = pow(lightFade, 2.0);
				float lightStrength = lightDistMult * lightFade * reflectivePower * 0.5;

				if (lightStrength > 0.0)
				{
					vec3 lightColor = (u_lightColors[li].rgb / length(u_lightColors[li].rgb)) * u_Local4.a; // Normalize.
					vec3 lightDir = normalize(lightPos - position.xyz);
					float light_occlusion = 1.0;
				
					lightColor = lightColor * power;// * maxStr;

					addedLight.rgb += lightColor * lightStrength * 0.333;

					if (useOcclusion)
					{
						light_occlusion = (1.0 - clamp(dot(vec4(-lightDir*E, 1.0), occlusion), 0.0, 1.0));
					}
					
#ifdef __LIGHTING_TYPE_3__
					addedLight.rgb += blinn_phong(position.xyz, outColor.rgb, N, E, lightDir, outColor.rgb * lightColor, outColor.rgb * lightColor, mix(0.1, 0.5, clamp(lightGlossinessFactor, 0.0, 1.0)) * clamp(lightStrength * light_occlusion * phongFactor, 0.0, 1.0), lightPos) * lightFade;
#else //!__LIGHTING_TYPE_3__
					addedLight.rgb += blinn_phong(N, E, lightDir, outColor.rgb * lightColor, outColor.rgb * lightColor, mix(0.1, 0.5, lightGlossinessFactor)) * lightStrength * light_occlusion * phongFactor;
#endif //__LIGHTING_TYPE_3__
				}
			}

			addedLight.rgb *= lightGlossinessFactor; // More grey colors get more colorization from lights...
			outColor.rgb = outColor.rgb + max(addedLight, vec3(0.0));
		}
	}

	// Has a sort of lightmap-ish type coloring/lighting effect on result...
	//outColor.rgb *= ((outColor.rgb / length(outColor.rgb)) * 3.0) * 0.25 + 0.75;

#ifdef __SCREEN_SPACE_REFLECTIONS__
	if (u_Local8.r > 0.0)
	{
		outColor.rgb = AddReflection(texCoords, position, origNorm, outColor.rgb, reflectivePower * ssReflection);
	}
#endif //__SCREEN_SPACE_REFLECTIONS__

#ifdef __AMBIENT_OCCLUSION__
	if (u_Local1.b == 1.0)
	{// AO enabled...
		float ao = calculateAO(to_light_norm, N * 10000.0);
		ao = clamp(ao * u_Local6.g + u_Local6.r, u_Local6.r, 1.0);
		outColor.rgb *= ao;
	}
#endif //__AMBIENT_OCCLUSION__

#ifdef __ENHANCED_AO__
	if (u_Local1.b >= 2.0)
	{// HQ AO enabled...
		float msao = 0.0;

		if (u_Local1.b >= 4.0)
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
		else
		{
			msao = textureLod(u_SteepMap, texCoords, 0.0).x;
		}

		float sao = clamp(msao, 0.0, 1.0);
		sao = clamp(sao * u_Local6.g + u_Local6.r, u_Local6.r, 1.0);
		outColor.rgb *= sao;
	}
#endif //__ENHANCED_AO__

#ifdef __ENVMAP__
	if (u_Local1.a > 0.0)
	{// Envmap enabled...
		float lightScale = clamp(1.0 - clamp(max(max(outColor.r, outColor.g), outColor.b), 0.0, 1.0), 0.0, 1.0);
		float invLightScale = clamp((1.0 - lightScale) * 1.2, 0.2, 1.0);
		vec3 env = envMap(rd, 0.6 /* warmth */);
		outColor.rgb = mix(outColor.rgb, outColor.rgb + ((env * (reflectivePower * 0.5) * invLightScale) * lightScale), clamp((reflectivePower * 0.5) * lightScale * gloss, 0.0, 1.0));
	}
#endif //__ENVMAP__

#ifdef __HEIGHTMAP_SHADOWS__
	if (u_Local2.g > 0.0 && u_Local6.a < 1.0)
	{
		float hshadow = GetShadow(texCoords, -to_light_norm);
		float finalShadow = clamp(hshadow + u_Local3.b, u_Local3.b, u_Local3.a);
		finalShadow = mix(finalShadow, 1.0, clamp(u_Local6.a, 0.0, 1.0)); // Dampen out shadows at sunrise/sunset...
		outColor.rgb *= finalShadow;
	}
#endif //__HEIGHTMAP_SHADOWS__

#if defined(USE_SHADOWMAP)
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
#endif //defined(USE_SHADOWMAP)

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

