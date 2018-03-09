#define REAL_WAVES					// You probably always want this turned on.
#define USE_UNDERWATER				// TODO: Convert from HLSL when I can be bothered.
#define USE_REFLECTION				// Enable reflections on water.
#define FIX_WATER_DEPTH_ISSUES		// Use basic depth value for sky hits...
//#define __OLD_LIGHTING__
#define __DEBUG__

/*
heightMap – height-map used for waves generation as described in the section “Modifying existing geometry”
backBufferMap – current contents of the back buffer
positionMap – texture storing scene position vectors (material type is alpha)
waterPositionMap – texture storing scene water position vectors (alpha is 1.0 when water is at this pixel. 0.0 if not).
normalMap – texture storing normal vectors for normal mapping as described in the section “The computation of normal vectors”
foamMap – texture containing foam – in my case it is a photo of foam converted to greyscale
*/

uniform mat4		u_ModelViewProjectionMatrix;

uniform sampler2D	u_WaterHeightMap;
uniform sampler2D	u_DiffuseMap;			// backBufferMap
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;

uniform sampler2D	u_SplatControlMap;		// foamSplatControl
uniform sampler2D	u_OverlayMap;			// foamMap 1
uniform sampler2D	u_SplatMap1;			// foamMap 2
uniform sampler2D	u_SplatMap2;			// foamMap 3
uniform sampler2D	u_SplatMap3;			// foamMap 4

uniform sampler2D	u_DetailMap;			// causics map

//uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_DeluxeMap;			// noise

uniform sampler2D	u_WaterPositionMap;

uniform samplerCube	u_SkyCubeMap;
uniform samplerCube	u_SkyCubeMapNight;

uniform vec4		u_Mins;					// MAP_MINS[0], MAP_MINS[1], MAP_MINS[2], 0.0
uniform vec4		u_Maxs;					// MAP_MAXS[0], MAP_MAXS[1], MAP_MAXS[2], 0.0
uniform vec4		u_MapInfo;				// MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], SUN_VISIBLE

uniform vec4		u_Local0;				// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_Local1;				// MAP_WATER_LEVEL, USE_GLSL_REFLECTION, IS_UNDERWATER, WATER_REFLECTIVENESS
uniform vec4		u_Local2;				// WATER_COLOR_SHALLOW_R, WATER_COLOR_SHALLOW_G, WATER_COLOR_SHALLOW_B
uniform vec4		u_Local3;				// WATER_COLOR_DEEP_R, WATER_COLOR_DEEP_G, WATER_COLOR_DEEP_B
uniform vec4		u_Local4;				// DayNightFactor, 0, 0, 0
uniform vec4		u_Local7;				// testshadervalue1, etc
uniform vec4		u_Local8;				// testshadervalue5, etc
uniform vec4		u_Local10;				// waveHeight, waveDensity, USE_OCEAN

uniform vec2		u_Dimensions;
uniform vec4		u_ViewInfo;				// zmin, zmax, zmax / zmin

varying vec2		var_TexCoords;

uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

#define				MAP_WATER_LEVEL u_Local1.r

#define MAX_DEFERRED_LIGHTS 16//64//128

uniform int			u_lightCount;
uniform vec3		u_lightPositions2[MAX_DEFERRED_LIGHTS];
uniform float		u_lightDistances[MAX_DEFERRED_LIGHTS];
uniform float		u_lightHeightScales[MAX_DEFERRED_LIGHTS];
uniform vec3		u_lightColors[MAX_DEFERRED_LIGHTS];

// Position of the camera
uniform vec3		u_ViewOrigin;
#define ViewOrigin	u_ViewOrigin.xzy

// Timer
uniform float		u_Time;
#define systemtimer		(u_Time * 5000.0)


// Over-all water clearness...
//const float waterClarity = 0.001;
const float waterClarity = 0.03;

// How fast will colours fade out. You can also think about this
// values as how clear water is. Therefore use smaller values (eg. 0.05f)
// to have crystal clear water and bigger to achieve "muddy" water.
const float fadeSpeed = 0.15;

// Normals scaling factor
const float normalScale = 1.0;

// R0 is a constant related to the index of refraction (IOR).
// It should be computed on the CPU and passed to the shader.
const float R0 = 0.5;

// Maximum waves amplitude
//const float waveHeight = 6.0;//4.0;
#define waveHeight u_Local10.r
#define waveDensity u_Local10.g

// Colour of the sun
//vec3 sunColor = (u_PrimaryLightColor.rgb + vec3(1.0) + vec3(1.0) + vec3(1.0)) / 4.0; // 1/4 real sun color, 3/4 white...
vec3 sunColor = vec3(1.0);

// The smaller this value is, the more soft the transition between
// shore and water. If you want hard edges use very big value.
// Default is 1.0f.
const float shoreHardness = 0.2;//1.0;

// This value modifies current fresnel term. If you want to weaken
// reflections use bigger value. If you want to empasize them use
// value smaller then 0. Default is 0.0.
const float refractionStrength = 0.0;
//float refractionStrength = -0.3;

// Modifies 4 sampled normals. Increase first values to have more
// smaller "waves" or last to have more bigger "waves"
const vec4 normalModifier = vec4(1.0, 2.0, 4.0, 8.0);

// Describes at what depth foam starts to fade out and
// at what it is completely invisible. The third value is at
// what height foam for waves appear (+ waterLevel).
#define foamExistence vec3(8.0, 50.0, waveHeight)

const float sunScale = 3.0;

const float shininess = 0.7;
const float specularScale = 0.07;


// Colour of the water surface
//const vec3 waterColorShallow = vec3(0.0078, 0.5176, 0.7);
vec3 waterColorShallow = u_Local2.rgb;

// Colour of the water depth
//const vec3 waterColorDeep = vec3(0.0059, 0.1276, 0.18);
vec3 waterColorDeep = u_Local3.rgb;

const vec3 extinction = vec3(35.0, 480.0, 8192.0);

// Water transparency along eye vector.
//const float visibility = 320.0;
const float visibility = 32.0;

// Increase this value to have more smaller waves.
const vec2 scale = vec2(0.002, 0.002);
const float refractionScale = 0.005;

#define USE_OCEAN u_Local10.b


float saturate(in float val) {
	return clamp(val, 0.0, 1.0);
}

vec3 saturate(in vec3 val) {
	return clamp(val, vec3(0.0), vec3(1.0));
}

vec4 GetControlMap(vec3 m_vertPos)
{
	vec3 controlScale = vec3(1.0) / u_MapInfo.xyz;
	vec4 xaxis = texture(u_SplatControlMap, m_vertPos.yz * controlScale.yz);
	vec4 yaxis = texture(u_SplatControlMap, m_vertPos.xz * controlScale.xz);
	vec4 zaxis = texture(u_SplatControlMap, m_vertPos.xy * controlScale.xy);
	return clamp((xaxis * 0.333 + yaxis * 0.333 + zaxis * 0.333) * 10.0, 0.0, 1.0);
}

vec4 GetMap( in sampler2D tex, vec2 coord)
{
	return texture( tex, coord );
}

vec4 GetFoamMap(vec3 m_vertPos, vec2 coord)
{
	// Use splat mapping to variate foam textures used across the map...
	const float textureScale = 0.3;
	vec4 splatColor = GetMap(u_OverlayMap, coord * textureScale);
	vec4 control = GetControlMap(m_vertPos);
	
	if (control.r > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap1, coord * textureScale);
		splatColor = mix(splatColor, tex, control.r * tex.a);
	}

	if (control.g > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap2, coord * textureScale);
		splatColor = mix(splatColor, tex, control.g * tex.a);
	}

	if (control.b > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap3, coord * textureScale);
		splatColor = mix(splatColor, tex, control.b * tex.a);
	}

	return splatColor;
}


float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

mat3 compute_tangent_frame(in vec3 N, in vec3 P, in vec2 UV) {
    vec3 dp1 = dFdx(P);
    vec3 dp2 = dFdy(P);
    vec2 duv1 = dFdx(UV);
    vec2 duv2 = dFdy(UV);
    // solve the linear system
    vec3 dp1xdp2 = cross(dp1, dp2);
    mat2x3 inverseM = mat2x3(cross(dp2, dp1xdp2), cross(dp1xdp2, dp1));
    vec3 T = inverseM * vec2(duv1.x, duv2.x);
    vec3 B = inverseM * vec2(duv1.y, duv2.y);
    // construct tangent frame
    float maxLength = max(length(T), length(B));
    T = T / maxLength;
    B = B / maxLength;
    return mat3(T, B, N);
}

// Function calculating fresnel term.
// - normal - normalized normal vector
// - eyeVec - normalized eye vector
float fresnelTerm(vec3 normal, vec3 eyeVec)
{
		float angle = 1.0f - clamp(dot(normal, eyeVec), 0.0, 1.0);
		float fresnel = angle * angle;
		fresnel = fresnel * fresnel;
		fresnel = fresnel * angle;
		return clamp(fresnel * (1.0 - clamp(R0, 0.0, 1.0)) + R0 - refractionStrength, 0.0, 1.0);
}

// Indices of refraction
#define Air 1.0
#define Bubble 1.06

// Air to glass ratio of the indices of refraction (Eta)
#define Eta (Air / Bubble)

// see http://en.wikipedia.org/wiki/Refractive_index Reflectivity
#define R0 (((Air - Bubble) * (Air - Bubble)) / ((Air + Bubble) * (Air + Bubble)))

vec3 GetFresnel( vec3 vView, vec3 vNormal, vec3 vR0, float fGloss )
{
	float NdotV = max( 0.0, dot( vView, vNormal ) );
	return vR0 + (vec3(1.0) - vR0) * pow( 1.0 - NdotV, 5.0 ) * pow( fGloss, 20.0 );
}

vec4 positionMapAtCoord ( vec2 coord )
{
	return texture(u_PositionMap, coord).xzyw;
}

vec4 waterMapLowerAtCoord ( vec2 coord )
{
	vec4 wmap = texture(u_WaterPositionMap, coord).xzyw;
	wmap.y = max(wmap.y, MAP_WATER_LEVEL);
	wmap.y -= waveHeight;
	return wmap;
}

vec4 waterMapUpperAtCoord ( vec2 coord )
{
	vec4 wmap = texture(u_WaterPositionMap, coord).xzyw;
	wmap.y = max(wmap.y, MAP_WATER_LEVEL);
	//wmap.y += waveHeight;
	return wmap;
}

float pw = (1.0/u_Dimensions.x);
float ph = (1.0/u_Dimensions.y);

#if defined(USE_REFLECTION) && !defined(__LQ_MODE__)
vec3 AddReflection(vec2 coord, vec3 positionMap, vec3 waterMapLower, vec3 inColor, float height)
{
	vec3 skyColor = vec3(0.0);
	float skyContribution = 0.06;

	//if (u_Local7.a > 0.0)
	{// Sky cube light contributions... If enabled...
		vec4 cubeInfo = vec4(0.0, 0.0, 0.0, 1.0);
		cubeInfo.xyz -= u_ViewOrigin.xyz;

		cubeInfo.w = pow(distance(u_ViewOrigin.xyz, vec3(0.0, 0.0, 0.0)), 3.0);

		cubeInfo.xyz *= 1.0 / cubeInfo.w;
		cubeInfo.w = 1.0 / cubeInfo.w;

		vec3 E = normalize(ViewOrigin.xzy - positionMap.xzy);
		vec3 cubeRayDir = reflect(E, vec3(0.0, 0.0, 1.0));

		vec3 parallax = cubeInfo.xyz + cubeInfo.w * E;
		parallax.z *= -1.0;

		vec3 reflected = cubeRayDir + parallax;

		reflected = vec3(-reflected.y, -reflected.z, -reflected.x);

		if (u_Local4.r > 0.0 && u_Local4.r < 1.0)
		{// Mix between night and day colors...
			vec3 skyColorDay = texture(u_SkyCubeMap, reflected).rgb;
			vec3 skyColorNight = texture(u_SkyCubeMapNight, reflected).rgb;
			skyColor = mix(skyColorDay, skyColorNight, clamp(u_Local4.r, 0.0, 1.0));
		}
		else if (u_Local4.r >= 1.0)
		{// Night only colors...
			skyColor = texture(u_SkyCubeMapNight, reflected).rgb;
		}
		else
		{// Day only colors...
			skyColor = texture(u_SkyCubeMap, reflected).rgb;
		}
	}

	if (positionMap.y > waterMapLower.y)
	{
		return mix(inColor, skyColor, skyContribution);
	}

	float wMapCheck = texture(u_WaterPositionMap, vec2(coord.x, 1.0)).a;
	if (wMapCheck > 0.0)
	{// Top of screen pixel is water, don't check...
		return mix(inColor, skyColor, skyContribution);
	}

	float pixelDistance = distance(waterMapLower.xyz, ViewOrigin.xyz);

	// Quick scan for pixel that is not water...
	float QLAND_Y = 0.0;

	const float scanSpeed = 48.0;// 16.0;// 5.0; // How many pixels to scan by on the 1st rough pass...
	//float scanSpeed = u_Local0.r;
	
	for (float y = coord.y; y <= 1.0; y += ph * scanSpeed)
	{
		float isWater = texture(u_WaterPositionMap, vec2(coord.x, y)).a;
		vec4 pMap = positionMapAtCoord(vec2(coord.x, y));

		if (isWater <= 0.0 && distance(pMap.xyz, ViewOrigin.xyz) > pixelDistance && (pMap.a - 1.0 == 1024.0 || pMap.a - 1.0 == 1025.0 || pMap.y >= waterMapLower.y || length(pMap.xyz) == 0.0))
		{
			QLAND_Y = y;
			break;
		}
	}

	if (QLAND_Y <= 0.0 || QLAND_Y >= 1.0)
	{// Found no non-water surfaces...
		return mix(inColor, skyColor, skyContribution);
	}
	
	QLAND_Y -= ph * scanSpeed;
	
	// Full scan from within scanSpeed px for the real 1st pixel...
	float upPos = coord.y;
	float LAND_Y = 0.0;

	for (float y = QLAND_Y; y <= QLAND_Y + (ph * scanSpeed); y += ph)
	{
		float isWater = texture(u_WaterPositionMap, vec2(coord.x, y)).a;
		vec4 pMap = positionMapAtCoord(vec2(coord.x, y));

		if (isWater <= 0.0 && distance(pMap.xyz, ViewOrigin.xyz) > pixelDistance && (pMap.a - 1.0 == 1024.0 || pMap.a - 1.0 == 1025.0 || pMap.y >= waterMapLower.y || length(pMap.xyz) == 0.0))
		{
			LAND_Y = y;
			break;
		}
	}

	if (LAND_Y <= 0.0 || LAND_Y >= 1.0)
	{// Found no non-water surfaces...
		//return mix(inColor, skyColor, skyContribution);
		LAND_Y = QLAND_Y;
	}

	upPos = clamp(coord.y + ((LAND_Y - coord.y) * 2.0), 0.0, 1.0);

	if (upPos > 1.0 || upPos < 0.0)
	{// Not on screen...
		return mix(inColor, skyColor, skyContribution);
	}

	// Offset the final pixel based on the height of the wave at that point, to create randomization...
	float hoff = height * 2.0 - 1.0;
	float offset = hoff * distance(coord.y, upPos) * (-8.0 * waveHeight * pw);

	vec2 finalPosition = clamp(vec2(coord.x + offset, upPos), 0.0, 1.0);

	float isWater = texture(u_WaterPositionMap, finalPosition).a;

	if (isWater > 0.0)
	{// This position is water...
		return mix(inColor, skyColor, skyContribution);
	}
	else
	{
		vec4 pMap = positionMapAtCoord(finalPosition);
		
		if (distance(pMap.xyz, ViewOrigin) <= pixelDistance)
		{// This position is sky or closer than the original pixel...
			return mix(inColor, skyColor, skyContribution);
		}

		inColor = mix(inColor, skyColor, skyContribution);
	}

	vec4 landColor = textureLod(u_DiffuseMap, finalPosition, 0.0);
#if 0
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + offset + pw, upPos), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + offset - pw, upPos), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + offset, upPos + ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + offset, upPos - ph), 0.0);
	//landColor /= 5.0;
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + offset + pw, upPos + ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + offset - pw, upPos - ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + offset + pw, upPos - ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + offset - pw, upPos + ph), 0.0);
	landColor /= 9.0;
#else
	/*float weight = 1.0;
	vec2 origOffset = vec2(coord.x + offset, upPos);

	for (float x = -3.0; x <= 3.0; x += 2.0)
	{
		for (float y = -3.0; y <= 3.0; y += 2.0)
		{
			vec2 tc = vec2(origOffset.x + (x * pw), origOffset.y + (y * ph));
			float w = 1.0 - clamp(distance(tc, origOffset) / 20.0, 0.0, 1.0);
			landColor += textureLod(u_DiffuseMap, tc, 0.0) * w;
			weight += w;
		}
	}

	landColor /= weight;*/
#endif

	return mix(inColor.rgb, landColor.rgb, vec3(1.0 - pow(upPos, 4.0)) * u_Local1.a);
}
#endif //defined(USE_REFLECTION) && !defined(__LQ_MODE__)


float getdiffuse(vec3 n, vec3 l, float p) {
	return pow(dot(n, l) * 0.4 + 0.6, p);
}

float sun(vec3 ray, vec3 lightDir) {
	return pow(max(0.0, dot(ray, lightDir)), 528.0);
}

#ifdef __OLD_LIGHTING__
float getspecular(vec3 n, vec3 l, vec3 e, float s) {
	float nrm = (s + 8.0) / (3.1415 * 8.0);
	return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm;
}
#else //!__OLD_LIGHTING__
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
#define sphereRad 0.2

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

	float NdotLSphere;
	float specSph = clamp(sphereLight(pos, normal, v, r, f0, roughness, NoV, NdotLSphere, lightPos), 0.0, 0.2);
	vec3 spec = albedo * clamp(0.3183 * NdotLSphere + specSph, 0.0, 0.2);

	spec = specularColor * (pow(spec, vec3(1.0 / 2.2))) * specPower * 64.0;

	vec3 diffuse = diffuseColor * getdiffuse(normal, light, 2.0);

	return diffuse + spec;
}
#endif //__OLD_LIGHTING__

#define iTime (1.0-u_Time)
#define EULER 2.7182818284590452353602874

// its from here https://github.com/achlubek/venginenative/blob/master/shaders/include/WaterHeight.glsl 
float wave(vec2 uv, vec2 emitter, float speed, float phase, float timeshift) {
	float dst = distance(uv, emitter);
	return pow(EULER, sin(dst * phase - (iTime + timeshift) * speed)) / EULER;
}
vec2 wavedrag(vec2 uv, vec2 emitter) {
	return normalize(uv - emitter);
}

#define DRAG_MULT 4.0

float getwaves(vec2 position) {
	position *= 0.1;
	float iter = 0.0;
	float phase = 1.5;// 6.0;
	float speed = 1.0;// 1.5;// 2.0;
	float weight = 1.0;// 1.0;
	float w = 0.0;
	float ws = 0.0;
	for (int i = 0; i<3; i++)
	{
		vec2 p = vec2(sin(iter), cos(iter)) * 30.0;
		float res = wave(position, p, speed, phase, 0.0);
		float res2 = wave(position, p, speed, phase, 0.006);
		position -= wavedrag(position, p) * (res - res2) * weight * DRAG_MULT;
		w += res * weight;
		iter += 12.0;
		ws += weight;
		weight = mix(weight, 0.0, float(i + 1) / 4.0/*0.2*/);
		phase *= 1.2;
		speed *= 1.02;
	}
	//return pow(w / ws, 0.2);
	return clamp(w / ws, 0.0, 1.0);
}

float getwavesDetail(vec2 position) {
	position *= 0.1;
	float iter = 0.0;
	float phase = 1.5;
	float speed = 1.5;
	float weight = 1.0;
	float w = 0.0;
	float ws = 0.0;
	for (int i = 0; i<12; i++)
	{
		vec2 p = vec2(sin(iter), cos(iter)) * 8.0;
		float res = wave(position, p, speed, phase, 0.0);
		float res2 = wave(position, p, speed, phase, 0.003/*0.006*/);
		position -= wavedrag(position, p) * (res - res2) * weight * DRAG_MULT * 8.0/*u_Local0.b*/;// 1.3;
		w += res * weight;
		iter += 36.0;// 12.0;
		ws += weight;
		weight = mix(weight, 0.0, float(i+1) / 14.0/*0.1*/);
		phase *= 1.67;//1.2;
		speed *= 1.07;//1.02;
	}
	//return pow(w / ws, u_Local0.r/*-0.333*/);
	return clamp(w / ws, 0.0, 1.0);
}

void GetHeightAndNormal(in vec2 pos, in float e, in float depth, inout float height, inout vec3 waveNormal, inout vec3 lightingNormal, in vec3 eyeVecNorm, in float timer, in float level) {
#if !defined(__LQ_MODE__) && defined(REAL_WAVES)
	if (USE_OCEAN > 0.0)
	{
		height = getwaves(pos.xy) * depth;

		// Create a basic "big waves" normal vector...
		vec2 ex = vec2(e, 0) * 16.0;

		float height2 = getwaves(pos.xy + ex.xy) * depth;
		float height3 = getwaves(pos.xy + ex.yx) * depth;

		vec3 a = vec3(pos.x, height, pos.y);
		waveNormal = normalize(cross(normalize(a - vec3(pos.x - e, height2, pos.y)), normalize(a - vec3(pos.x, height3, pos.y + e))));

		// Now make a lighting normal vector, with some "small waves" bumpiness...
		waveNormal.xz *= 256.0;
		waveNormal = normalize(waveNormal);

		ex = vec2(e, 0) * 64.0;

		float dheight = getwavesDetail(pos.xy * 0.5) * depth;
		float dheight2 = getwavesDetail((pos.xy * 0.5) + ex.xy) * depth;
		float dheight3 = getwavesDetail((pos.xy * 0.5) + ex.yx) * depth;

		vec3 da = vec3(pos.x, dheight, pos.y);
		vec3 dnormal = normalize(cross(normalize(da - vec3(pos.x - e, dheight2, pos.y)), normalize(da - vec3(pos.x, dheight3, pos.y + e))));
		dnormal.xz *= 256.0;
		dnormal = normalize(dnormal);

		//lightingNormal = normalize(waveNormal + (dnormal * 0.8));
		lightingNormal = normalize(mix(-waveNormal, dnormal, vec3(0.75)));
	}
	else
#endif //!defined(__LQ_MODE__) && defined(REAL_WAVES)
	{
		vec2 ex = vec2(e, 0) * 64.0;

		height = getwavesDetail(pos.xy * 0.5) * depth;
		float height2 = getwavesDetail((pos.xy * 0.5) + ex.xy) * depth;
		float height3 = getwavesDetail((pos.xy * 0.5) + ex.yx) * depth;

		vec3 da = vec3(pos.x, height, pos.y);
		waveNormal = normalize(cross(normalize(da - vec3(pos.x - e, height2, pos.y)), normalize(da - vec3(pos.x, height3, pos.y + e))));
		waveNormal.xz *= 256.0;
		waveNormal = normalize(waveNormal);
		lightingNormal = waveNormal;
	}
}

void ScanHeightMap(inout vec3 surfacePoint, inout vec3 eyeVecNorm, inout float level, inout vec2 texCoord, inout float t, out vec3 waveNormal, inout vec3 lightingNormal, in float timer, inout float height)
{
	GetHeightAndNormal(surfacePoint.xz*0.03, 0.004, 1.0, height, waveNormal, lightingNormal, eyeVecNorm, timer, level);
	level = level + (height * waveHeight);
	//surfacePoint.y = level;

	t = (level - ViewOrigin.y) / eyeVecNorm.y;
	surfacePoint = ViewOrigin + eyeVecNorm * t;
}

vec3 WaterFall(vec3 color, vec3 color2, vec3 waterMapUpper, vec3 position, float timer)
{
	vec3 eyeVecNorm = normalize(ViewOrigin - waterMapUpper.xyz);
	vec3 pixelDir = eyeVecNorm;
	vec3 normal = normalize(pixelDir + (color.rgb * 0.5 - 0.25));
	vec3 specular = vec3(0.0);
	vec3 lightDir = normalize(ViewOrigin.xyz - u_PrimaryLightOrigin.xzy);
	float lambertian2 = dot(lightDir.xyz, normal);
	float spec2 = 0.0;
	float fresnel = clamp(1.0 - dot(normal, -eyeVecNorm), 0.0, 1.0);
	fresnel = pow(fresnel, 3.0) * 0.65;

	vec2 texCoord = var_TexCoords.xy;
	texCoord.x += sin(timer * 0.002 + 3.0 * abs(position.y)) * refractionScale;

	vec3 refraction = textureLod(u_DiffuseMap, texCoord, 0.0).rgb;
	refraction = mix(waterColorShallow.rgb, refraction.rgb, 0.3);

	float fTime = u_Time * 2.0;

	vec3 foam = (GetFoamMap(vec3(waterMapUpper.x * 0.03, (waterMapUpper.y * 0.03) + fTime, 0.5), vec2(waterMapUpper.x * 0.03, (waterMapUpper.y * 0.03) + fTime)).rgb + GetFoamMap(vec3(waterMapUpper.z * 0.03, (waterMapUpper.y * 0.03) + fTime, 0.5), vec2(waterMapUpper.z * 0.03, (waterMapUpper.y * 0.03) + fTime)).rgb) * 0.5;

	color = mix(color + (foam * sunColor), refraction, fresnel * 0.8);

	if (lambertian2 > 0.0)
	{// this is blinn phong
		vec3 mirrorEye = (2.0 * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
		vec3 halfDir2 = normalize(lightDir.xyz + mirrorEye);
		float specAngle = max(dot(halfDir2, normal), 0.0);
		spec2 = pow(specAngle, 16.0);
		specular = vec3(clamp(1.0 - fresnel, 0.4, 1.0)) * (vec3(spec2 * shininess)) * sunColor * specularScale * 25.0;
	}

	color = clamp(color + specular, 0.0, 1.0);

#ifdef __OLD_LIGHTING__
	color += vec3(getspecular(normal, lightDir, eyeVecNorm, 60.0));
#else //!__OLD_LIGHTING__
	color += blinn_phong(waterMapUpper.xyz, color.rgb, normal, eyeVecNorm, lightDir, u_PrimaryLightColor.rgb, u_PrimaryLightColor.rgb, 1.0, u_PrimaryLightOrigin.xyz);
#endif //__OLD_LIGHTING__

#if defined(USE_REFLECTION) && !defined(__LQ_MODE__)
	color = AddReflection(texCoord, position.xyz, waterMapUpper.xyz, color, 0.0);
#endif //defined(USE_REFLECTION) && !defined(__LQ_MODE__)

	return color;
}

void main ( void )
{
	vec3 color2 = textureLod(u_DiffuseMap, var_TexCoords, 0.0).rgb;
	vec4 waterMapUpper = waterMapUpperAtCoord(var_TexCoords);
	vec4 waterMapLower = waterMapUpper;
	waterMapLower.y -= waveHeight;
	bool IS_UNDERWATER = false;

	if (u_Local1.b > 0.0) 
	{
		IS_UNDERWATER = true;
	}

	if (waterMapUpper.a <= 0.0)
	{// Should be safe to skip everything.
		gl_FragColor = vec4(color2, 1.0);
		return;
	}

	bool pixelIsInWaterRange = false;
	bool pixelIsUnderWater = false;
	vec3 color = color2;

	vec4 positionMap = positionMapAtCoord(var_TexCoords);
	vec3 position = positionMap.xyz;

	position.xz = waterMapLower.xz; // test

#if defined(FIX_WATER_DEPTH_ISSUES)
	if (positionMap.a-1.0 == 1024.0)
	{
		position.xyz = waterMapLower.xyz;
		position.y -= 1024.0;
	}
#endif //defined(FIX_WATER_DEPTH_ISSUES)

	float timer = systemtimer * (waveHeight / 16.0);

	if (waterMapLower.a >= 2.0 && positionMap.a-1.0 < 1024.0)
	{// Low horizontal normal, this is a waterfall...
		vec3 color = WaterFall(color.rgb, color2.rgb, waterMapUpper.xyz, position.xyz, timer);
		gl_FragColor = vec4(color, 1.0);
		return;
	}
	else if (IS_UNDERWATER || waterMapLower.y > ViewOrigin.y)
	{
		pixelIsUnderWater = true;
	}
	else if (waterMapUpper.y >= position.y)
	{
		pixelIsInWaterRange = true;
	}

	float waterLevel = waterMapLower.y;
	float level = waterMapLower.y;
	float depth = 0.0;

	if (pixelIsInWaterRange || pixelIsUnderWater || position.y <= level + waveHeight)
	{
		vec2 wind = normalize(waterMapLower.xzy).xy; // Waves head toward center of map. Should suit current WZ maps...

		// Find intersection with water surface
		vec3 eyeVecNorm = normalize(waterMapLower.xyz - ViewOrigin);
		float t = 0.0;
		vec3 surfacePoint = waterMapLower.xyz;

		vec2 texCoord = vec2(0.0);

		float height = 0.0;
		vec3 normal;
		vec3 lightingNormal;
		ScanHeightMap(surfacePoint, eyeVecNorm, level, texCoord, t, normal, lightingNormal, timer, height);

		depth = length(position - surfacePoint);
		float depth2 = surfacePoint.y - position.y;
		float depthN = depth * fadeSpeed;

#if defined(REAL_WAVES) && !defined(__LQ_MODE__)
		if (USE_OCEAN > 0.0)
		{
#if 1
			//float vDist = distance(surfacePoint, ViewOrigin);
			//vec3 vDir = normalize(ViewOrigin - surfacePoint);
			float vDist = distance(vec3(surfacePoint.x, ViewOrigin.y + waveHeight, surfacePoint.z), ViewOrigin);
			vec3 vDir = normalize(ViewOrigin - vec3(surfacePoint.x, ViewOrigin.y + waveHeight, surfacePoint.z));
			surfacePoint += vDir * vDist * height;
			//surfacePoint += u_Local0.rgb * height;
#else

			vec3 fakeView = vec3(ViewOrigin.x, waterMapLower.y + 1024.0, ViewOrigin.z);
			vec3 vDist = (fakeView - surfacePoint) * u_Local0.rgb;
			vec3 vDir = normalize(fakeView/*ViewOrigin*/ - surfacePoint);
			surfacePoint += vDir * (vec3(0.75, 0.0, 0.75)) * vDist * height;
			//level = surfacePoint.y;
#endif
		}
#endif //defined(REAL_WAVES) && !defined(__LQ_MODE__)


		eyeVecNorm = normalize(ViewOrigin - surfacePoint);

		if (pixelIsUnderWater)
		{
			depth = length(surfacePoint - position);
			float depth2 = position.y - surfacePoint.y;
			float depthN = depth * fadeSpeed;

			if (position.y <= level)
			{// This pixel is below the water line... Fast path...
				vec3 waterCol = clamp(length(sunColor) / vec3(sunScale), 0.0, 1.0);
				waterCol = waterCol * mix(waterColorShallow, waterColorDeep, clamp(depth2 / extinction, 0.0, 1.0));

				color2 = color2 - color2 * clamp(depth2 / extinction, 0.0, 1.0);
				color = mix(color2, waterCol, clamp(depthN / visibility, 0.0, 1.0));

				gl_FragColor = vec4(color, 1.0);
				return;
			}
		}
		else if (depth2 < 0.0 && waterMapLower.y < surfacePoint.y)
		{// Waves against shoreline. Pixel is above waterLevel + waveHeight... (but ignore anything marked as actual water - eg: not a shoreline)
			gl_FragColor = vec4(color2, 1.0);
			return;
		}

		//depth = length(position - surfacePoint);
		//depth2 = surfacePoint.y - position.y;
		//depthN = depth * fadeSpeed;


		texCoord = var_TexCoords.xy;

		texCoord.x += sin(timer * 0.002 + 3.0 * abs(position.y)) * (refractionScale * min(depth2, 1.0));

		vec3 refraction = textureLod(u_DiffuseMap, texCoord, 0.0).rgb;

		//texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00001 * wind + sin(timer * 0.001 + position.x) * 0.005;
		vec4 position2 = positionMapAtCoord(texCoord);
		vec4 waterMapLower3 = waterMapLowerAtCoord(texCoord);

#if defined(FIX_WATER_DEPTH_ISSUES)
		if (position2.a-1.0 == 1024.0)
		{
			position2.xyz = waterMapLower3.xyz;
			position2.y -= 1024.0;
		}
#endif //defined(FIX_WATER_DEPTH_ISSUES)

		if (!pixelIsUnderWater && position2.y > level)
		{
			refraction = color2;
		}

		float waterCol = clamp(length(sunColor) / sunScale, 0.0, 1.0);
		
		vec3 refraction1 = mix(refraction, waterColorShallow * vec3(waterCol), clamp(vec3(depthN) / vec3(visibility), 0.0, 1.0));
		refraction = mix(refraction1, waterColorDeep * vec3(waterCol), clamp((vec3(depth2) / vec3(extinction)), 0.0, 1.0));

		vec4 foam = vec4(0.0);

		texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00001 * wind + sin(timer * 0.001 + position.x) * 0.005;
		vec2 texCoord2 = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00002 * wind + sin(timer * 0.001 + position.z) * 0.005;
		
		float causicStrength = 1.0; // Scale back causics where there's foam, and over distance...

		float pixDist = distance(surfacePoint.xyz, ViewOrigin.xyz);
		causicStrength *= 1.0 - clamp(pixDist / 1024.0, 0.0, 1.0);

#if !defined(__LQ_MODE__)
		if (USE_OCEAN > 0.0)
		{
			if (depth2 < foamExistence.x)
			{
				foam = (GetFoamMap(surfacePoint.xzy, texCoord) + GetFoamMap(surfacePoint.xzy, texCoord2)) * 0.5;
			}
			else if (depth2 < foamExistence.y)
			{
				foam = mix((GetFoamMap(surfacePoint.xzy, texCoord) + GetFoamMap(surfacePoint.xzy, texCoord2)) * 0.5,
					vec4(0.0), vec4((depth2 - foamExistence.x) / (foamExistence.y - foamExistence.x)));
			}

			if (waveHeight - foamExistence.z > 0.0001)
			{
				foam += (GetFoamMap(surfacePoint.xzy * 0.5, texCoord) + GetFoamMap(surfacePoint.xzy * 0.5, texCoord2)) * 0.5 *
					clamp((level - (waterLevel + foamExistence.z)) / (waveHeight - foamExistence.z), 0.0, 1.0);
			}
		}
#endif //!defined(__LQ_MODE__)
		
		causicStrength *= 0.15 - clamp(max(foam.r, max(foam.g, foam.b)) * foam.a * 32.0, 0.0, 0.15);

		vec3 specular = vec3(0.0);

		//vec3 lightDir = normalize(surfacePoint.xyz - u_PrimaryLightOrigin.xzy);
		vec3 lightDir = normalize(ViewOrigin.xyz - u_PrimaryLightOrigin.xzy);


#if 1
		float fresnel = fresnelTerm(lightingNormal, eyeVecNorm);
		fresnel = pow(fresnel, 0.3);
#else
		vec3 fresnel2 = GetFresnel(eyeVecNorm, lightingNormal, vec3(R0), 1.0);
		float fresnel = length(fresnel2.rgb) / 3.0;
#endif


		vec3 dist = -eyeVecNorm;

		color = mix(refraction, waterColorDeep, fresnel);


		float atten = max(1.0 - dot(dist, dist) * 0.001, 0.0);
		color += waterColorShallow.rgb * height * 0.18 * atten;

		//color.rgb *= height;


#if defined(USE_REFLECTION) && !defined(__LQ_MODE__)
		if (!pixelIsUnderWater && u_Local1.g >= 2.0)
		{
			color = AddReflection(var_TexCoords, position, vec3(waterMapLower3.x, level, waterMapLower3.z), color.rgb, height);
		}
#endif //defined(USE_REFLECTION) && !defined(__LQ_MODE__)

		//
		// Crytek style lighting... Done before final wave colors...
		//

#if 1
		// CryTek's way
		vec3 mirrorEye = (2.0f * dot(eyeVecNorm, lightingNormal) * lightingNormal - eyeVecNorm);
		float dotSpec = saturate(dot(mirrorEye.xyz, -lightDir) * 0.5f + 0.5f);
		specular = (1.0f - fresnel) * saturate(-lightDir.y) * ((pow(dotSpec, 8.0f/*512.0f*/)) * (shininess * 1.8f + 0.2f)) * sunColor;
		specular += specular * 25.0 * saturate(shininess - 0.05f) * sunColor;
#endif

		vec3 caustic = color * (texture(u_DetailMap, vec2((texCoord.x + (texCoord2.x*2.2)) * 0.25, (texCoord.y + (texCoord2.y*1.2)) * 0.25)).rgb * 1.1);
		color = clamp(color + (caustic * causicStrength), 0.0, 1.0);

#if !defined(__LQ_MODE__)
		if (USE_OCEAN > 0.0)
			color = clamp(color + max(specular, foam.rgb * sunColor), 0.0, 1.0);
		else
#endif //!defined(__LQ_MODE__)
			color = clamp(color + specular, 0.0, 1.0);

		color = mix(refraction, color, clamp(depth * shoreHardness, 0.0, 1.0));
		color = mix(color, color2, 1.0 - clamp(waterClarity * depth, 0.8, 1.0));


		//
		// Final Lighting...
		//


#if 0
		float diffuse = getdiffuse(lightingNormal, -lightDir, u_Local0.r/*16.0*/);
		color += diffuse * u_Local0.g;// 0.01;
#endif

#if 0
		// Special sauce...
		vec3 N = lightingNormal;
		vec2 velocity = N.xz * (1.0 - N.y);
		float dist2 = distance(surfacePoint, ViewOrigin) * 0.0001;
		N = mix(vec3(0.0, 1.0, 0.0), N, 1.0 / (dist2 * dist2 * 0.01 + 1.0));
		vec3 R = reflect(eyeVecNorm, N);
		float windSpray = smoothstep(.01, .12, length(velocity)) * u_Local0.b;// 0.1;
		float sunReflect = (fresnel * 0.75 + 0.25) * sun(R, lightDir) * u_Local0.a;
		color = (color + (sunReflect * sunColor) + windSpray);
#else
		vec3 R = reflect(eyeVecNorm, lightingNormal);
#endif


#if 1
		// Also do dlights. Includes map glows and sabers and all that good stuff...
		if (u_lightCount > 0.0)
		{
#define LIGHT_COLOR_POWER			4.0

			vec3 addedLight = vec3(0.0);
			float power = clamp(length(color.rgb) / 3.0, 0.0, 1.0) * 0.5 + 0.5;
			power = pow(power, LIGHT_COLOR_POWER);
			power = power * 0.5 + 0.5;

			float maxStr = max(color.r, max(color.g, color.b)) * 0.9 + 0.1;

#ifdef __LQ_MODE__
			for (int li = 0; li < min(u_lightCount, 4); li++)
#else //!__LQ_MODE__
			for (int li = 0; li < u_lightCount; li++)
#endif //__LQ_MODE__
			{
				vec3 lightPos = u_lightPositions2[li].xyz;

				float lightDist = distance(lightPos, waterMapLower3.xzy);

				if (u_lightHeightScales[li] > 0.0)
				{// ignore height differences, check later...
					lightDist -= length(lightPos.z - waterMapLower3.y);
				}

				float lightDistMult = 1.0 - clamp((distance(lightPos.xyz, u_ViewOrigin.xyz) / 4096.0), 0.0, 1.0);
				float lightStrength = pow(1.0 - clamp(lightDist / (u_lightDistances[li] * 2.0), 0.0, 1.0), 2.0);
				lightStrength *= lightDistMult;

				if (lightStrength > 0.0)
				{
					vec3 lightColor = (u_lightColors[li].rgb / length(u_lightColors[li].rgb)) * 4.0; // Normalize.
					vec3 lightDir2 = normalize(lightPos - waterMapLower3.xzy);
				
					float lightReflect = fresnel * sun(R, lightDir2);
					float lightDiffuse = getdiffuse(lightingNormal, lightDir, 6.0);
					addedLight.rgb += ((lightReflect * lightColor) + (lightDiffuse * lightColor)) * lightStrength * lightStrength;
				}
			}

			color.rgb = color.rgb + addedLight;
		}
#endif


		if (pixelIsUnderWater)
		{
			if (position.y < level)
				color = color2;
		}
#if 1
		else
		{
			if (position.y > level)
				color = color2;
		}
#endif

#ifdef __DEBUG__
		if (u_Local0.a == 1.0)
		{
			color.rgb = normal.rgb * 0.5 + 0.5;
		}
		else if (u_Local0.a == 2.0)
		{
			gl_FragColor = vec4(normal.rgb, 1.0);
			return;
		}
		else if (u_Local0.a == 3.0)
		{
			color.rgb = lightingNormal.rgb * 0.5 + 0.5;
		}
		else if (u_Local0.a == 4.0)
		{
			gl_FragColor = vec4(lightingNormal.rgb, 1.0);
			return;
		}
		else if (u_Local0.a == 5.0)
		{
			gl_FragColor = vec4(fresnel, fresnel, fresnel, 1.0);
			return;
		}
		/*else if (u_Local0.a == 6.0)
		{
			gl_FragColor = vec4(fresnel2.xyz, 1.0);
			return;
		}*/
		else if (u_Local0.a == 6.0)
		{
			gl_FragColor = vec4(height, height, height, 1.0);
			return;
		}
#endif //__DEBUG__
	}

	gl_FragColor = vec4(color, 1.0);
}
