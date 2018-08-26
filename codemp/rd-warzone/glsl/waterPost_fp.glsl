#define REAL_WAVES					// You probably always want this turned on.
#define USE_UNDERWATER				// TODO: Convert from HLSL when I can be bothered.
#define USE_REFLECTION				// Enable reflections on water.
#define FIX_WATER_DEPTH_ISSUES		// Use basic depth value for sky hits...
//#define EXPERIMENTAL_WATERFALL	// Experimental waterfalls...
//#define __DEBUG__
//#define USE_LIGHTING				// Use lighting in this shader? trying to handle the lighting in deferredlight now instead.

//#define TEST_WATER

/*
heightMap – height-map used for waves generation as described in the section “Modifying existing geometry”
backBufferMap – current contents of the back buffer
positionMap – texture storing scene position vectors (material type is alpha)
waterPositionMap – texture storing scene water position vectors (alpha is 1.0 when water is at this pixel. 0.0 if not).
normalMap – texture storing normal vectors for normal mapping as described in the section “The computation of normal vectors”
foamMap – texture containing foam – in my case it is a photo of foam converted to greyscale
*/

uniform mat4		u_ModelViewProjectionMatrix;

uniform sampler2D	u_DiffuseMap;			// backBufferMap
uniform sampler2D	u_PositionMap;

uniform sampler2D	u_OverlayMap;			// foamMap 1
uniform sampler2D	u_SplatMap1;			// foamMap 2
uniform sampler2D	u_SplatMap2;			// foamMap 3
uniform sampler2D	u_SplatMap3;			// foamMap 4

uniform sampler2D	u_DetailMap;			// causics map

uniform sampler2D	u_WaterPositionMap;

uniform samplerCube	u_SkyCubeMap;
uniform samplerCube	u_SkyCubeMapNight;

uniform vec4		u_Mins;					// MAP_MINS[0], MAP_MINS[1], MAP_MINS[2], 0.0
uniform vec4		u_Maxs;					// MAP_MAXS[0], MAP_MAXS[1], MAP_MAXS[2], 0.0
uniform vec4		u_MapInfo;				// MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], SUN_VISIBLE

uniform vec4		u_Local0;				// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_Local1;				// MAP_WATER_LEVEL, USE_GLSL_REFLECTION, IS_UNDERWATER, WATER_REFLECTIVENESS
uniform vec4		u_Local2;				// WATER_COLOR_SHALLOW_R, WATER_COLOR_SHALLOW_G, WATER_COLOR_SHALLOW_B, WATER_CLARITY
uniform vec4		u_Local3;				// WATER_COLOR_DEEP_R, WATER_COLOR_DEEP_G, WATER_COLOR_DEEP_B
uniform vec4		u_Local4;				// DayNightFactor, WATER_EXTINCTION1, WATER_EXTINCTION2, WATER_EXTINCTION3
uniform vec4		u_Local7;				// testshadervalue1, etc
uniform vec4		u_Local8;				// testshadervalue5, etc
uniform vec4		u_Local10;				// waveHeight, waveDensity, USE_OCEAN, viewUpDown

uniform vec2		u_Dimensions;
uniform vec4		u_ViewInfo;				// zmin, zmax, zmax / zmin

varying vec2		var_TexCoords;

uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

#define				MAP_WATER_LEVEL u_Local1.r

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
//const float waterClarity = 0.03;
#define waterClarity u_Local2.a

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

//const vec3 extinction = vec3(35.0, 480.0, 8192.0);
vec3 extinction = u_Local4.gba;

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

float hash( const in float n ) {
	return fract(sin(n)*4378.5453);
}

float noise(in vec3 o) 
{
	vec3 p = floor(o);
	vec3 fr = fract(o);
		
	float n = p.x + p.y*57.0 + p.z * 1009.0;

	float a = hash(n+  0.0);
	float b = hash(n+  1.0);
	float c = hash(n+ 57.0);
	float d = hash(n+ 58.0);
	
	float e = hash(n+  0.0 + 1009.0);
	float f = hash(n+  1.0 + 1009.0);
	float g = hash(n+ 57.0 + 1009.0);
	float h = hash(n+ 58.0 + 1009.0);
	
	
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

const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

float SmoothNoise( vec3 p )
{
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); 
	
    return f * (1.0 / (0.5000 + 0.2500));
}

vec2 GetMapTC(vec3 pos)
{
	vec2 mapSize = u_Maxs.xy - u_Mins.xy;
	return (pos.xy - u_Mins.xy) / mapSize;
}

vec3 GetCausicMap(vec3 m_vertPos)
{
	vec2 coord = (m_vertPos.xy * m_vertPos.z * 0.000025) + ((u_Time * 0.1) * normalize(m_vertPos).xy * 0.5);
	return texture(u_DetailMap, coord).rgb;
}

vec4 GetFoamMap(vec3 m_vertPos)
{
	vec2 coord = (m_vertPos.xy * 0.005) + ((u_Time * 0.1) * normalize(m_vertPos).xy * 0.5);

	vec2 xy = GetMapTC(m_vertPos);
	float x = xy.x;
	float y = xy.y;

	vec4 tex0 = texture(u_OverlayMap, coord);
	vec4 tex1 = texture(u_SplatMap1, coord);
	vec4 tex2 = texture(u_SplatMap2, coord);
	vec4 tex3 = texture(u_SplatMap3, coord);

	vec4 splatColor1 = mix(tex0, tex1, x);
	vec4 splatColor2 = mix(tex2, tex3, y);

	vec4 splatColor = mix(splatColor1, splatColor2, (x+y)*0.5);
	return splatColor;
}


float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

#ifdef TEST_WATER

#define iResolution u_Dimensions.xy
#define wTime (u_Time * u_Local0.r)

float wave(vec2 uv, vec2 emitter, float speed, float phase){
	float dst = distance(uv, emitter);
	return pow((0.5 + 0.5 * sin(dst * phase - wTime * speed)), 5.0);
}

#define GOLDEN_ANGLE_RADIAN 2.39996
float getwavesLO(vec2 uv){
	float w = 0.0;
	float sw = 0.0;
	float iter = 0.0;
	float ww = 1.0;
    uv += wTime * 0.5;
	// it seems its absolutely fastest way for water height function that looks real
	for(int i=0;i<6;i++){
		w += ww * wave(uv * 0.06 , vec2(sin(iter), cos(iter)) * 10.0, 2.0 + iter * 0.08, 2.0 + iter * 3.0);
		sw += ww;
		ww = mix(ww, 0.0115, 0.4);
		iter += GOLDEN_ANGLE_RADIAN;
	}
	
	return w / sw;
}
float getwavesHI(vec2 uv){
	float w = 0.0;
	float sw = 0.0;
	float iter = 0.0;
	float ww = 1.0;
    uv += wTime * 0.5;
	// it seems its absolutely fastest way for water height function that looks real
	for(int i=0;i<24;i++){
		w += ww * wave(uv * 0.06 , vec2(sin(iter), cos(iter)) * 10.0, 2.0 + iter * 0.08, 2.0 + iter * 3.0);
		sw += ww;
		ww = mix(ww, 0.0115, 0.4);
		iter += GOLDEN_ANGLE_RADIAN;
	}
	
	return w / sw;
}

vec3 normal(vec2 pos, float e, float depth)
{
	float H = 0.0;
    vec2 ex = vec2(e, 0);
    H = getwavesHI(pos.xy) * depth;
    vec3 a = vec3(pos.x, H, pos.y);
    return normalize(cross(normalize(a-vec3(pos.x - e, getwavesHI(pos.xy - ex.xy) * depth, pos.y)), 
                           normalize(a-vec3(pos.x, getwavesHI(pos.xy + ex.yx) * depth, pos.y + e))));
}

float rand2sTimex(vec2 co){
    return fract(sin(dot(co.xy * wTime,vec2(12.9898,78.233))) * 43758.5453);
}

float raymarchwater(vec3 camera, vec3 start, vec3 end, float depth){
    vec3 pos = start;
    float h = 0.0;
    float hupper = depth;
    float hlower = 0.0;
    vec2 zer = vec2(0.0);
    vec3 dir = normalize(end - start);
    for(int i=0; i < int(318.0 * u_Local0.g); i++){
        h = getwavesLO(pos.xz) * depth - depth;
        if(h + u_Local0.a/*0.5*/ > pos.y) {
            return distance(pos, camera);
        }
        pos += dir * (pos.y - h);
    }
    return -1.0;
}

float intersectPlane(vec3 origin, vec3 direction, vec3 point, vec3 normal)
{ 
    return clamp(dot(point - origin, normal) / dot(direction, normal), -1.0, 9991999.0); 
}
vec3 extra_cheap_atmosphere(vec3 raydir, vec3 sundir){
	sundir.y = max(sundir.y, -0.07);
	float special_trick = 1.0 / (raydir.y * 1.0 + 0.1);
	float special_trick2 = 1.0 / (sundir.y * 11.0 + 1.0);
	float raysundt = pow(abs(dot(sundir, raydir)), 2.0);
	float sundt = pow(max(0.0, dot(sundir, raydir)), 8.0);
	float mymie = sundt * special_trick * 0.2;
	vec3 suncolor = mix(vec3(1.0), max(vec3(0.0), vec3(1.0) - vec3(5.5, 13.0, 22.4) / 22.4), special_trick2);
	vec3 bluesky= vec3(5.5, 13.0, 22.4) / 22.4 * suncolor;
	vec3 bluesky2 = max(vec3(0.0), bluesky - vec3(5.5, 13.0, 22.4) * 0.004 * (special_trick + -6.0 * sundir.y * sundir.y));
	bluesky2 *= special_trick * (0.24 + raysundt * 0.24);
	return bluesky2 + mymie * suncolor;
} 
vec3 getatm(vec3 ray){
 	return extra_cheap_atmosphere(ray, normalize(vec3(1.0))) * 0.5;
    
}

float sun(vec3 ray){
 	vec3 sd = normalize(vec3(1.0));   
    return pow(max(0.0, dot(ray, sd)), 528.0) * 110.0;
}

void TestWater( out vec4 fragColor, in vec2 fragCoord, in vec4 pMap )
{
	vec2 uv = fragCoord.xy;
 	
	float waterdepth = 2.1;
	vec3 wfloor = vec3(0.0, -waterdepth, 0.0);
	vec3 wceil = vec3(0.0, 0.0, 0.0);
	vec3 orig = vec3(0.0, 2.0, 0.0);
	vec3 ray = normalize(pMap.xyz) * u_Local0.b;//getRay(uv);
	if (ray.y > 0.0) 
    {
        ray.y *= -1.0;
    }
	float hihit = intersectPlane(orig, ray, wceil, vec3(0.0, 1.0, 0.0));
	float lohit = intersectPlane(orig, ray, wfloor, vec3(0.0, 1.0, 0.0));
    vec3 hipos = orig + ray * hihit;
    vec3 lopos = orig + ray * lohit;
	float dist = raymarchwater(orig, hipos, lopos, waterdepth);
    vec3 pos = orig + ray * dist;

	vec3 N = normal(pos.xz, 0.001, waterdepth);
    vec2 velocity = N.xz * (1.0 - N.y);
    N = mix(vec3(0.0, 1.0, 0.0), N, 1.0 / clamp(dist * dist * 0.04 + 1.0, 0.0, 1.0));
    vec3 R = reflect(ray, N);
    float fresnel = (0.04 + (1.0-0.04)*(pow(1.0 - max(0.0, dot(-N, ray)), 5.0)));
	
    vec3 C = fresnel * getatm(R) * 2.0 + fresnel * sun(R);
    //tonemapping
    C = normalize(C) * sqrt(length(C));
    
	fragColor = vec4(clamp(C, 0.0, 1.0), 1.0);
}
#endif //TEST_WATER


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

	if (u_Local1.b <= 0.0) 
		wmap.y = max(wmap.y, MAP_WATER_LEVEL);

	wmap.y -= waveHeight;
	return wmap;
}

vec4 waterMapUpperAtCoord ( vec2 coord )
{
	vec4 wmap = texture(u_WaterPositionMap, coord).xzyw;

	if (u_Local1.b <= 0.0) 
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

	const float scanSpeed = 16.0;// 48.0;// 16.0;// 5.0; // How many pixels to scan by on the 1st rough pass...
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
#endif

	return mix(inColor.rgb, landColor.rgb, vec3(1.0 - pow(upPos, 4.0)) * u_Local1.a);
}
#endif //defined(USE_REFLECTION) && !defined(__LQ_MODE__)


float getdiffuse(vec3 n, vec3 l, float p) {
	return pow(dot(n, l) * 0.4 + 0.6, p);
}

float dosun(vec3 ray, vec3 lightDir) {
	return pow(max(0.0, dot(ray, lightDir)), 528.0);
}

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
#define waveDetailFactor 0.333 //0.5

	if (USE_OCEAN > 0.0)
	{
		height = getwaves(pos.xy) * depth;

		// Create a basic "big waves" normal vector...
		vec2 ex = vec2(e, 0.0) * 16.0;

		float height2 = getwaves(pos.xy + ex.xy) * depth;
		float height3 = getwaves(pos.xy + ex.yx) * depth;

		vec3 a = vec3(depth, height * 0.001, depth);
		vec3 b = vec3(depth - e, height2 * 0.001, depth);
		vec3 c = vec3(depth, height3 * 0.001, depth + e);
		waveNormal = cross(normalize(a - b), normalize(a - c));

		// Now make a lighting normal vector, with some "small waves" bumpiness...
		waveNormal = normalize(waveNormal);

		ex = vec2(e, 0.0) * 64.0;

		float dheight = getwavesDetail(pos.xy * waveDetailFactor) * depth;
		float dheight2 = getwavesDetail((pos.xy * waveDetailFactor) + ex.xy) * depth;
		float dheight3 = getwavesDetail((pos.xy * waveDetailFactor) + ex.yx) * depth;

		vec3 da = vec3(depth, dheight * 0.05, depth);
		vec3 db = vec3(depth - e, dheight2 * 0.05, depth);
		vec3 dc = vec3(depth, dheight2 * 0.05, depth + e);
		vec3 dnormal = cross(normalize(da - db), normalize(da - dc));
		dnormal = normalize(dnormal);

		lightingNormal = normalize(mix(waveNormal, dnormal, vec3(0.75)));
	}
	else
#endif //!defined(__LQ_MODE__) && defined(REAL_WAVES)
	{
		vec2 ex = vec2(e, 0) * 64.0;

		height = getwavesDetail(pos.xy * waveDetailFactor) * depth;
		float height2 = getwavesDetail((pos.xy * waveDetailFactor) + ex.xy) * depth;
		float height3 = getwavesDetail((pos.xy * waveDetailFactor) + ex.yx) * depth;

		/*vec3 da = vec3(pos.x, height, pos.y);
		waveNormal = normalize(cross(normalize(da - vec3(pos.x - e, height2, pos.y)), normalize(da - vec3(pos.x, height3, pos.y + e))));
		waveNormal.xz *= 256.0;
		waveNormal = normalize(waveNormal);
		lightingNormal = waveNormal;*/

		vec3 da = vec3(depth, height * 0.05, depth);
		vec3 db = vec3(depth - e, height2 * 0.05, depth);
		vec3 dc = vec3(depth, height2 * 0.05, depth + e);
		waveNormal = cross(normalize(da - db), normalize(da - dc));
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

float WaterFallNearby ( vec2 uv, out float hit )
{
	hit = 0.0;
	float numTests = 0.0;

	for (float x = 1.0; x <= 16.0; x += 1.0)
	{
		numTests += 1.0;

		vec2 coord = uv + vec2(x * pw, 0.0);
		if (uv.x >= 0.0 && uv.x <= 1.0)
		{
			float isWater = texture(u_WaterPositionMap, coord).a;

			if (isWater >= 2.0)
			{
				hit = isWater;
				break;
			}
		}

		coord = uv - vec2(x * pw, 0.0);
		if (uv.x >= 0.0 && uv.x <= 1.0)
		{
			float isWater = texture(u_WaterPositionMap, coord).a;

			if (isWater >= 2.0)
			{
				hit = isWater;
				break;
			}
		}
	}

	return numTests / 16.0;
}

vec4 WaterFall(vec3 color, vec3 color2, vec3 waterMapUpper, vec3 position, float timer, float slope, float wfEdgeFactor, float wfHitType)
{
#ifdef EXPERIMENTAL_WATERFALL
	//return vec4(0.5 + (0.5 * (1.0 - wfEdgeFactor)), 0.0, 0.0, 1.0);

	vec3 n = normalize(position);
	n.y += systemtimer * 0.00008;
	float rand = SmoothNoise( n * u_Local7.r );

	for (float x = 2.0; x <= u_Local7.g; x *= 2.0)
	{
		rand *= SmoothNoise( n * (u_Local7.r * x) );
	}

	rand = clamp(pow(rand, u_Local7.b), 0.0, 1.0);

	rand -= 0.05;

	rand = clamp(pow(rand, u_Local7.a), 0.0, 1.0);

	vec2 texCoord = var_TexCoords.xy;

	//float hitType = 0.0;
	//float edgeFactor = WaterFallScanEdge( texCoord, hitType );
	float edgeFactor = smoothstep(0.0, 1.0, 1.0 - wfEdgeFactor);

	texCoord.x += sin(timer * 0.002 + 3.0 * abs(position.y)) * refractionScale;

	vec3 refraction = textureLod(u_DiffuseMap, texCoord, 0.0).rgb;
	refraction = mix(waterColorShallow.rgb, refraction.rgb, clamp(slope, 0.0, 1.0)/*0.3*/);

	vec3 wfColor = mix(refraction.rgb, vec3(1.0), vec3(rand));
	
	return vec4(mix(color, wfColor, vec3(edgeFactor)), 1.0);
#else //!EXPERIMENTAL_WATERFALL
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

	vec3 foam = (GetFoamMap(vec3(waterMapUpper.x * 0.03, (waterMapUpper.y * 0.03) + fTime, 0.5)).rgb + GetFoamMap(vec3(waterMapUpper.z * 0.03, (waterMapUpper.y * 0.03) + fTime, 0.5)).rgb) * 0.5;

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

	//color += blinn_phong(waterMapUpper.xyz, color.rgb, normal, eyeVecNorm, lightDir, u_PrimaryLightColor.rgb, u_PrimaryLightColor.rgb, 1.0, u_PrimaryLightOrigin.xyz);

#if defined(USE_REFLECTION) && !defined(__LQ_MODE__)
	color = AddReflection(texCoord, position.xyz, waterMapUpper.xyz, color, 0.0);
#endif //defined(USE_REFLECTION) && !defined(__LQ_MODE__)

	return vec4(color, 1.0);
#endif //EXPERIMENTAL_WATERFALL
}

void main ( void )
{
	vec3 color2 = textureLod(u_DiffuseMap, var_TexCoords, 0.0).rgb;
	vec4 waterMapUpper = waterMapUpperAtCoord(var_TexCoords);
	vec4 waterMapLower = waterMapUpper;
	vec4 origWaterMapUpper = waterMapUpper;
	waterMapLower.y -= waveHeight;
	bool IS_UNDERWATER = false;

	if (u_Local1.b > 0.0) 
	{
		IS_UNDERWATER = true;
	}

	bool pixelIsInWaterRange = false;
	bool pixelIsUnderWater = false;
	vec3 color = color2;

	vec4 positionMap = positionMapAtCoord(var_TexCoords);
	vec3 position = positionMap.xyz;

	float timer = systemtimer * (waveHeight / 16.0);

	position.xz = waterMapLower.xz; // test

#if defined(FIX_WATER_DEPTH_ISSUES)
	if (positionMap.a-1.0 == 1024.0)
	{
		position.xyz = waterMapLower.xyz;
		if (pixelIsUnderWater)
			position.y += 1024.0;
		else
			position.y -= 1024.0;
	}
#endif //defined(FIX_WATER_DEPTH_ISSUES)

#if 0
	if (IS_UNDERWATER /*|| waterMapLower.y > ViewOrigin.y*/)
	{
		/*vec3 p = normalize(ViewOrigin.xyz - position.xyz);
		float pp = position.y * 0.5 + 0.5;*/
		float pp = clamp((waterMapLower.y - ViewOrigin.y) / u_Local7.g, 0.0, 1.0);

		vec3 waterCol = clamp(length(sunColor) / vec3(sunScale), 0.0, 1.0);
		vec3 shallow = waterColorShallow * vec3(waterCol);
		vec3 deep = waterColorDeep * vec3(waterCol);
		vec3 wcol = mix(deep, shallow, pp);
		color.rgb = mix(color.rgb, wcol, u_Local7.r);
		color2.rgb = color.rgb;
	}
#endif

#ifdef EXPERIMENTAL_WATERFALL
	float hitType = 0.0;
	float edgeFactor = 0.0;
	
	if (waterMapLower.a < 2.0)
	{
		edgeFactor = WaterFallNearby( var_TexCoords, hitType );
	}

	if ((waterMapLower.a >= 2.0 || hitType >= 2.0) /*&& positionMap.a-1.0 < 1024.0*/)
	{// Low horizontal normal, this is a waterfall...
		if (waterMapLower.a >= 2.0 /*&& positionMap.a-1.0 < 1024.0*/)
		{// Actual waterfall pixel...
			gl_FragColor = WaterFall(color.rgb, color2.rgb, waterMapUpper.xyz, position.xyz, timer, waterMapLower.a - 2.0, 0.0, waterMapLower.a);
			return;
		}
		else if (waterMapUpper.a <= 0.0)
		{// near a waterfall, but this is not water.
			gl_FragColor = WaterFall(color.rgb, color2.rgb, waterMapUpper.xyz, position.xyz, timer, hitType - 2.0, edgeFactor, hitType);
			return;
		}
		else
		{// Blend with water...
			color = WaterFall(color.rgb, color2.rgb, waterMapUpper.xyz, position.xyz, timer, waterMapLower.a - 2.0, edgeFactor, hitType).rgb;
			color2 = color;
		}
	}
#else //!EXPERIMENTAL_WATERFALL
	if (waterMapLower.a >= 2.0 /*&& positionMap.a-1.0 < 1024.0*/)
	{// Actual waterfall pixel...
		gl_FragColor = WaterFall(color.rgb, color2.rgb, waterMapUpper.xyz, position.xyz, timer, waterMapLower.a - 2.0, 0.0, waterMapLower.a);
		return;
	}
#endif //EXPERIMENTAL_WATERFALL

	if (waterMapUpper.a <= 0.0)
	{// Should be safe to skip everything else.
		gl_FragColor = vec4(color2, 1.0);
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

#ifdef TEST_WATER
	if (pixelIsInWaterRange || pixelIsUnderWater || position.y <= level + waveHeight)
	{
		TestWater( gl_FragColor, var_TexCoords, waterMapUpper/*positionMap*/ );
	}
	else
	{
		gl_FragColor = vec4(color2, 1.0);
	}
	return;
#endif //TEST_WATER

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

		if (pixelIsUnderWater)
			lightingNormal.y *= -1.0;

		float depth2;
		float depthN;

		if (pixelIsUnderWater)
		{
			depth = length(surfacePoint - position);
			depth2 = position.y - surfacePoint.y;
			depthN = depth * fadeSpeed;
#if 0 // i think it looks better with the extra transparancy...
			if (position.y <= level)
			{// This pixel is below the water line... Fast path...
				vec3 waterCol = clamp(length(sunColor) / vec3(sunScale), 0.0, 1.0);
				waterCol = waterCol * mix(waterColorShallow, waterColorDeep, clamp(depth2 / extinction, 0.0, 1.0));

				color2 = color2 - color2 * clamp(depth2 / extinction, 0.0, 1.0);
				color = mix(color2, waterCol, clamp(depthN / visibility, 0.0, 1.0));

				gl_FragColor = vec4(color, 1.0);
				return;
			}
#endif
		}
		else
		{
			depth = length(position - surfacePoint);
			depth2 = surfacePoint.y - position.y;
			depthN = depth * fadeSpeed;
		}

#if defined(REAL_WAVES) && !defined(__LQ_MODE__)
		if (USE_OCEAN > 0.0)
		{
			float vDist = distance(vec3(surfacePoint.x, ViewOrigin.y + waveHeight, surfacePoint.z), ViewOrigin);
			vec3 vDir = normalize(ViewOrigin - vec3(surfacePoint.x, ViewOrigin.y + waveHeight, surfacePoint.z));
			surfacePoint += vDir * vDist * height * (pixelIsUnderWater ? -1.0 : 1.0);
			//if (u_Local0.r == 0.0)
			//	surfacePoint += lightingNormal * height * u_Local0.g; // 0.875
			//else if (u_Local0.r == 1.0)
			//	surfacePoint.y += lightingNormal.y * height * u_Local0.g;
		}
#endif //defined(REAL_WAVES) && !defined(__LQ_MODE__)

		eyeVecNorm = normalize(ViewOrigin - surfacePoint);

		if (!pixelIsUnderWater && depth2 < 0.0 && waterMapLower.y < surfacePoint.y)
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
			if (pixelIsUnderWater)
				position2.y += 1024.0;
			else
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
		float causicStrength = 1.0; // Scale back causics where there's foam, and over distance...

		float pixDist = distance(surfacePoint.xyz, ViewOrigin.xyz);
		causicStrength *= 1.0 - clamp(pixDist / 1024.0, 0.0, 1.0);

#if !defined(__LQ_MODE__)
		if (USE_OCEAN > 0.0)
		{
			if (depth2 < foamExistence.x)
			{
				foam = GetFoamMap(origWaterMapUpper.xzy);
			}
			else if (depth2 < foamExistence.y)
			{
				foam = mix(GetFoamMap(origWaterMapUpper.xzy), vec4(0.0), (depth2 - foamExistence.x) / (foamExistence.y - foamExistence.x));
			}

			if (waveHeight - foamExistence.z > 0.0001)
			{
				foam += GetFoamMap(origWaterMapUpper.xzy + foamExistence.z) * clamp((level - (waterLevel + foamExistence.z)) / (waveHeight - foamExistence.z), 0.0, 1.0);
			}
		}
#endif //!defined(__LQ_MODE__)
		
		causicStrength *= 0.15 - clamp(max(foam.r, max(foam.g, foam.b)) * foam.a * 32.0, 0.0, 0.15);

		vec3 specular = vec3(0.0);

		vec3 lightDir = normalize(ViewOrigin.xyz - u_PrimaryLightOrigin.xzy);


		float fresnel = fresnelTerm(lightingNormal, eyeVecNorm);
		fresnel = pow(fresnel, 0.5);


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

		// CryTek's way
		vec3 mirrorEye = (2.0f * dot(eyeVecNorm, lightingNormal) * lightingNormal - eyeVecNorm);
		float dotSpec = saturate(dot(mirrorEye.xyz, -lightDir) * 0.5f + 0.5f);
		specular = (1.0f - fresnel) * saturate(-lightDir.y) * ((pow(dotSpec, 8.0f/*512.0f*/)) * (shininess * 1.8f + 0.2f)) * sunColor;
		specular += specular * 1.5 * (pixelIsUnderWater ? 8.0 : 1.0) * saturate(shininess - 0.05f) * sunColor;



		vec3 caustic = color * GetCausicMap(surfacePoint.xzy/*origWaterMapUpper.xzy*/);
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


		vec3 R = reflect(eyeVecNorm, lightingNormal);


#ifdef USE_LIGHTING
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

				float lightDistMult = 1.0 - clamp((distance(lightPos.xyz, u_ViewOrigin.xyz) / MAX_DEFERRED_LIGHT_RANGE), 0.0, 1.0);
				float lightStrength = pow(1.0 - clamp(lightDist / (u_lightDistances[li] * 2.0), 0.0, 1.0), 2.0);
				lightStrength *= lightDistMult;

				if (lightStrength > 0.0)
				{
					vec3 lightColor = (u_lightColors[li].rgb / length(u_lightColors[li].rgb)) * 4.0; // Normalize.
					vec3 lightDir2 = normalize(lightPos - waterMapLower3.xzy);
				
					float lightReflect = fresnel * dosun(R, lightDir2);
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
		if (u_Local0.r == 1.0)
		{
			gl_FragColor = vec4(normal.rbg * 0.5 + 0.5, 1.0);
			return;
		}
		else if (u_Local0.r == 2.0)
		{
			gl_FragColor = vec4(normal.rbg, 1.0);
			return;
		}
		else if (u_Local0.r == 3.0)
		{
			gl_FragColor = vec4(lightingNormal.rbg * 0.5 + 0.5, 1.0);
			return;
		}
		else if (u_Local0.r == 4.0)
		{
			gl_FragColor = vec4(lightingNormal.rbg, 1.0);
			return;
		}
		else if (u_Local0.r == 5.0)
		{
			gl_FragColor = vec4(fresnel, fresnel, fresnel, 1.0);
			return;
		}
		/*else if (u_Local0.r == 6.0)
		{
			gl_FragColor = vec4(fresnel2.xyz, 1.0);
			return;
		}*/
		else if (u_Local0.r == 6.0)
		{
			gl_FragColor = vec4(height, height, height, 1.0);
			return;
		}
#endif //__DEBUG__
	}

	gl_FragColor = vec4(color, 1.0);
}
