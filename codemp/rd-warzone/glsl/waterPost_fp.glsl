//#define SIMPLIFIED_FRESNEL		// Seems no faster... not as good?
#define REAL_WAVES					// You probably always want this turned on.
#define USE_UNDERWATER				// TODO: Convert from HLSL when I can be bothered.
#define USE_REFLECTION				// Enable reflections on water.
#define FIX_WATER_DEPTH_ISSUES		// Use basic depth value for sky hits...
#define FOAM_SPLAT_MAPS				// Use foam splat maps to vary foam looks across the map...
//#define EXPERIMENTAL

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

uniform vec4		u_Mins;					// MAP_MINS[0], MAP_MINS[1], MAP_MINS[2], 0.0
uniform vec4		u_Maxs;					// MAP_MAXS[0], MAP_MAXS[1], MAP_MAXS[2], 0.0
uniform vec4		u_MapInfo;				// MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], SUN_VISIBLE

uniform vec4		u_Local0;				// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_Local1;				// MAP_WATER_LEVEL, USE_GLSL_REFLECTION, IS_UNDERWATER, WATER_REFLECTIVENESS
uniform vec4		u_Local2;				// WATER_COLOR_SHALLOW_R, WATER_COLOR_SHALLOW_G, WATER_COLOR_SHALLOW_B
uniform vec4		u_Local3;				// WATER_COLOR_DEEP_R, WATER_COLOR_DEEP_G, WATER_COLOR_DEEP_B
uniform vec4		u_Local4;				// FOG_COLOR
uniform vec4		u_Local5;				// FOG_COLOR_SUN
uniform vec4		u_Local6;				// FOG_DENSITY
uniform vec4		u_Local10;				// waveHeight, waveDensity

uniform vec2		u_Dimensions;
uniform vec4		u_ViewInfo;				// zmin, zmax, zmax / zmin

varying vec2		var_TexCoords;

uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

#define				MAP_WATER_LEVEL u_Local1.r

#define MAX_DEFERRED_LIGHTS 128

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
//float waterClarity = u_Local0.r;

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
//const vec3 foamExistence = vec3(1.5, 5.35, 2.3); //vec3(0.65, 1.35, 0.5);
//const vec3 foamExistence = vec3(1.5, 50.0, waveHeight * 0.85/*5.0*/);
#define foamExistence vec3(1.5, 50.0, waveHeight * 0.85/*5.0*/)
//vec3 foamExistence = vec3(u_Local0.r, u_Local0.g, u_Local0.b);

const float sunScale = 3.0;

const float shininess = 0.7;
const float specularScale = 0.07;


// Colour of the water surface
//const vec3 waterColorShallow = vec3(0.0078, 0.5176, 0.7);
vec3 waterColorShallow = u_Local2.rgb;

// Colour of the water depth
//const vec3 waterColorDeep = vec3(0.0059, 0.1276, 0.18);
vec3 waterColorDeep = u_Local3.rgb;

//const vec3 extinction = vec3(7.0, 30.0, 40.0);			// Horizontal
//const vec3 extinction = vec3(7.0, 96.0, 128.0);			// Horizontal
//const vec3 extinction = vec3(35.0, 480.0, 640.0);
const vec3 extinction = vec3(35.0, 480.0, 8192.0);
//vec3 extinction = vec3(u_Local0.r, u_Local0.g, u_Local0.b);

// Water transparency along eye vector.
const float visibility = 32.0;
//const float visibility = 320.0;
//float visibility = u_Local0.r;//320.0;

// Increase this value to have more smaller waves.
const vec2 scale = vec2(0.002, 0.002);
const float refractionScale = 0.005;

// Wind force in x and z axes.
//const vec2 wind = vec2(-0.3, 0.7);


#ifdef FOAM_SPLAT_MAPS
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
#else //!FOAM_SPLAT_MAPS
vec4 GetFoamMap(vec2 coord)
{
	// Just use single foam texture...
	return texture(u_OverlayMap, coord);
}
#endif //FOAM_SPLAT_MAPS


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
#ifdef SIMPLIFIED_FRESNEL
		// Simplified
		return R0 + (1.0f - R0) * pow(1.0f - dot(eyeVec, normal), 5.0f);
#else //!SIMPLIFIED_FRESNEL
		float angle = 1.0f - clamp(dot(normal, eyeVec), 0.0, 1.0);
		float fresnel = angle * angle;
		fresnel = fresnel * fresnel;
		fresnel = fresnel * angle;
		return clamp(fresnel * (1.0 - clamp(R0, 0.0, 1.0)) + R0 - refractionStrength, 0.0, 1.0);
#endif //SIMPLIFIED_FRESNEL
}

vec4 positionMapAtCoord ( vec2 coord )
{
	return textureLod(u_PositionMap, coord, 0.0).xzyw;
}

vec4 waterMapLowerAtCoord ( vec2 coord )
{
	vec4 wmap = textureLod(u_WaterPositionMap, coord, 0.0).xzyw;
	wmap.y -= waveHeight;
	return wmap;
}

vec4 waterMapUpperAtCoord ( vec2 coord )
{
	vec4 wmap = textureLod(u_WaterPositionMap, coord, 0.0).xzyw;
	return wmap;
}

float pw = (1.0/u_Dimensions.x);
float ph = (1.0/u_Dimensions.y);

vec3 AddReflection(vec2 coord, vec3 positionMap, vec3 waterMapLower, vec3 inColor)
{
	if (positionMap.y > waterMapLower.y)
	{
		return inColor;
	}

	vec4 wMapCheck = waterMapLowerAtCoord(vec2(coord.x, 1.0));
	if (wMapCheck.a > 0.0)
	{// Top of screen pixel is water, don't check...
		return inColor;
	}

	// Quick scan for pixel that is not water...
	float QLAND_Y = 0.0;

	for (float y = coord.y; y <= 1.0; y += ph * 5.0)
	{
		vec4 wMap = waterMapLowerAtCoord(vec2(coord.x, y));
		vec4 pMap = positionMapAtCoord(vec2(coord.x, y));
		float isWater = wMap.a;

		if (isWater <= 0.0 && (pMap.y >= waterMapLower.y || length(pMap.xyz) == 0.0))
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
		vec4 wMap = waterMapLowerAtCoord(vec2(coord.x, y));
		vec4 pMap = positionMapAtCoord(vec2(coord.x, y));
		float isWater = wMap.a;

		if (isWater <= 0.0 && (pMap.y >= waterMapLower.y || length(pMap.xyz) == 0.0))
		{
			LAND_Y = y;
			break;
		}
	}

	if (QLAND_Y <= 0.0 || QLAND_Y >= 1.0)
	{// Found no non-water surfaces...
		return inColor;
	}

	upPos = clamp(coord.y + ((LAND_Y - coord.y) * 2.0), 0.0, 1.0);

	if (upPos > 1.0 || upPos < 0.0)
	{// Not on screen...
		return inColor;
	}


	vec4 wMap = waterMapLowerAtCoord(vec2(coord.x, upPos));

	if (wMap.a > 0.0)
	{// This position is water, or it is closer then the reflection pixel...
		return inColor;
	}

	vec4 landColor = textureLod(u_DiffuseMap, vec2(coord.x, upPos), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + pw, upPos), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x - pw, upPos), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x, upPos + ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x, upPos - ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + pw, upPos + ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x - pw, upPos - ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + pw, upPos - ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x - pw, upPos + ph), 0.0);
	landColor /= 9.0;

	return mix(inColor.rgb, landColor.rgb, vec3(1.0 - pow(upPos, 4.0)) * /*0.28*/u_Local1.a);
}

// lighting
float getdiffuse(vec3 n, vec3 l, float p) {
	return pow(dot(n, l) * 0.4 + 0.6, p);
}
float getspecular(vec3 n, vec3 l, vec3 e, float s) {
	float nrm = (s + 8.0) / (3.1415 * 8.0);
	return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm;
}

// Blinn-Phong shading model with rim lighting (diffuse light bleeding to the other side).
// `normal`, `view` and `light` should be normalized.
vec3 blinn_phong(vec3 normal, vec3 view, vec3 light, vec3 diffuseColor, vec3 specularColor) {
	vec3 halfLV = normalize(light + view);
	float spe = pow(max(dot(normal, halfLV), 0.0), 32.0);
	float dif = dot(normal, light) * 0.5 + 0.75;
	return dif*diffuseColor + spe*specularColor;
}

#ifdef EXPERIMENTAL
const float PI	 	= 3.14159265358;

// Can you explain these epsilons to a wide graphics audience?  YOUR comment could go here.
const float EPSILON	= 1e-3;
#define EPSILON_NRM	(0.1 / u_Dimensions.x)

// Constant indicaing the number of steps taken while marching the light ray.  
const int NUM_STEPS = 4;

//Constants relating to the iteration of the heightmap for the wave, another part of the rendering
//process.
const int ITER_GEOMETRY = 2;
const int ITER_FRAGMENT =5;

// Constants that represent physical characteristics of the sea, can and should be changed and 
//  played with
const float SEA_HEIGHT = 1.1;
const float SEA_CHOPPY = 5.0;
const float SEA_SPEED = 0.9;
const float SEA_FREQ = 0.14;
const vec3 SEA_BASE = vec3(0.11,0.19,0.22);
const vec3 SEA_WATER_COLOR = vec3(0.55,0.9,0.7);
#define SEA_TIME (u_Time * SEA_SPEED)

//Matrix to permute the water surface into a complex, realistic form
mat2 octave_m = mat2(1.7,1.1,-1.1,1.4);

//Space bar key constant
const float KEY_SP    = 32.5/256.0;

//CaliCoastReplay :  These HSV/RGB translation functions are
//from http://gamedev.stackexchange.com/questions/59797/glsl-shader-change-hue-saturation-brightness
//This one converts red-green-blue color to hue-saturation-value color
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

//CaliCoastReplay :  These HSV/RGB translation functions are
//from http://gamedev.stackexchange.com/questions/59797/glsl-shader-change-hue-saturation-brightness
//This one converts hue-saturation-value color to red-green-blue color
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// math
// bteitler: Turn a vector of Euler angles into a rotation matrix
mat3 fromEuler(vec3 ang) {
	vec2 a1 = vec2(sin(ang.x),cos(ang.x));
    vec2 a2 = vec2(sin(ang.y),cos(ang.y));
    vec2 a3 = vec2(sin(ang.z),cos(ang.z));
    mat3 m;
    m[0] = vec3(a1.y*a3.y+a1.x*a2.x*a3.x,a1.y*a2.x*a3.x+a3.y*a1.x,-a2.y*a3.x);
	m[1] = vec3(-a2.y*a1.x,a1.y*a2.y,a2.x);
	m[2] = vec3(a3.y*a1.x*a2.x+a1.y*a3.x,a1.x*a3.x-a1.y*a3.y*a2.x,a2.y*a3.y);
	return m;
}

// bteitler: A 2D hash function for use in noise generation that returns range [0 .. 1].  You could
// use any hash function of choice, just needs to deterministic and return
// between 0 and 1, and also behave randomly.  Googling "GLSL hash function" returns almost exactly 
// this function: http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
// Performance is a real consideration of hash functions since ray-marching is already so heavy.
float hash( vec2 p ) {
    float h = dot(p,vec2(127.1,311.7));	
    return fract(sin(h)*43758.5453123);
}

// bteitler: A 2D psuedo-random wave / terrain function.  This is actually a poor name in my opinion,
// since its the "hash" function that is really the noise, and this function is smoothly interpolating
// between noisy points to create a continuous surface.
float noise( in vec2 p ) {
    vec2 i = floor( p );
    vec2 f = fract( p );	

    // bteitler: This is equivalent to the "smoothstep" interpolation function.
    // This is a smooth wave function with input between 0 and 1
    // (since it is taking the fractional part of <p>) and gives an output
    // between 0 and 1 that behaves and looks like a wave.  This is far from obvious, but we can graph it to see
    // Wolfram link: http://www.wolframalpha.com/input/?i=plot+x*x*%283.0-2.0*x%29+from+x%3D0+to+1
    // This is used to interpolate between random points.  Any smooth wave function that ramps up from 0 and
    // and hit 1.0 over the domain 0 to 1 would work.  For instance, sin(f * PI / 2.0) gives similar visuals.
    // This function is nice however because it does not require an expensive sine calculation.
    vec2 u = f*f*(3.0-2.0*f);

    // bteitler: This very confusing looking mish-mash is simply pulling deterministic random values (between 0 and 1)
    // for 4 corners of the grid square that <p> is inside, and doing 2D interpolation using the <u> function
    // (remember it looks like a nice wave!) 
    // The grid square has points defined at integer boundaries.  For example, if <p> is (4.3, 2.1), we will 
    // evaluate at points (4, 2), (5, 2), (4, 3), (5, 3), and then interpolate x using u(.3) and y using u(.1).
    return -1.0+2.0*mix( 
                mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), 
                        u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), 
                        u.x), 
                u.y);
}

// bteitler: diffuse lighting calculation - could be tweaked to taste
// lighting
float diffuse(vec3 n,vec3 l,float p) {
    return pow(dot(n,l) * 0.4 + 0.6,p);
}

// bteitler: specular lighting calculation - could be tweaked taste
float specular(vec3 n,vec3 l,vec3 e,float s) {    
    float nrm = (s + 8.0) / (3.1415 * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

// bteitler: Generate a smooth sky gradient color based on ray direction's Y value
// sky
vec3 getSkyColor(vec3 e) {
    e.y = max(e.y,0.0);
    vec3 ret;
    ret.x = pow(1.0-e.y,2.0);
    ret.y = 1.0-e.y;
    ret.z = 0.6+(1.0-e.y)*0.4;
    return ret;
}

// sea
// bteitler: TLDR is that this passes a low frequency random terrain through a 2D symmetric wave function that looks like this:
// http://www.wolframalpha.com/input/?i=%7B1-%7B%7B%7BAbs%5BCos%5B0.16x%5D%5D+%2B+Abs%5BCos%5B0.16x%5D%5D+%28%281.+-+Abs%5BSin%5B0.16x%5D%5D%29+-+Abs%5BCos%5B0.16x%5D%5D%29%7D+*+%7BAbs%5BCos%5B0.16y%5D%5D+%2B+Abs%5BCos%5B0.16y%5D%5D+%28%281.+-+Abs%5BSin%5B0.16y%5D%5D%29+-+Abs%5BCos%5B0.16y%5D%5D%29%7D%7D%5E0.65%7D%7D%5E4+from+-20+to+20
// The <choppy> parameter affects the wave shape.
float sea_octave(vec2 uv, float choppy) {
    // bteitler: Add the smoothed 2D terrain / wave function to the input coordinates
    // which are going to be our X and Z world coordinates.  It may be unclear why we are doing this.
    // This value is about to be passed through a wave function.  So we have a smoothed psuedo random height
    // field being added to our (X, Z) coordinates, and then fed through yet another wav function below.
    uv += noise(uv);
    // Note that you could simply return noise(uv) here and it would take on the characteristics of our 
    // noise interpolation function u and would be a reasonable heightmap for terrain.  
    // However, that isn't the shape we want in the end for an ocean with waves, so it will be fed through
    // a more wave like function.  Note that although both x and y channels of <uv> have the same value added, there is a 
    // symmetry break because <uv>.x and <uv>.y will typically be different values.

    // bteitler: This is a wave function with pointy peaks and curved troughs:
    // http://www.wolframalpha.com/input/?i=1-abs%28cos%28x%29%29%3B
    vec2 wv = 1.0-abs(sin(uv)); 

    // bteitler: This is a wave function with curved peaks and pointy troughs:
    // http://www.wolframalpha.com/input/?i=abs%28cos%28x%29%29%3B
    vec2 swv = abs(cos(uv));  
  
    // bteitler: Blending both wave functions gets us a new, cooler wave function (output between 0 and 1):
    // http://www.wolframalpha.com/input/?i=abs%28cos%28x%29%29+%2B+abs%28cos%28x%29%29+*+%28%281.0-abs%28sin%28x%29%29%29+-+abs%28cos%28x%29%29%29
    wv = mix(wv,swv,wv);

    // bteitler: Finally, compose both of the wave functions for X and Y channels into a final 
    // 1D height value, shaping it a bit along the way.  First, there is the composition (multiplication) of
    // the wave functions: wv.x * wv.y.  Wolfram will give us a cute 2D height graph for this!:
    // http://www.wolframalpha.com/input/?i=%7BAbs%5BCos%5Bx%5D%5D+%2B+Abs%5BCos%5Bx%5D%5D+%28%281.+-+Abs%5BSin%5Bx%5D%5D%29+-+Abs%5BCos%5Bx%5D%5D%29%7D+*+%7BAbs%5BCos%5By%5D%5D+%2B+Abs%5BCos%5By%5D%5D+%28%281.+-+Abs%5BSin%5By%5D%5D%29+-+Abs%5BCos%5By%5D%5D%29%7D
    // Next, we reshape the 2D wave function by exponentiation: (wv.x * wv.y)^0.65.  This slightly rounds the base of the wave:
    // http://www.wolframalpha.com/input/?i=%7B%7BAbs%5BCos%5Bx%5D%5D+%2B+Abs%5BCos%5Bx%5D%5D+%28%281.+-+Abs%5BSin%5Bx%5D%5D%29+-+Abs%5BCos%5Bx%5D%5D%29%7D+*+%7BAbs%5BCos%5By%5D%5D+%2B+Abs%5BCos%5By%5D%5D+%28%281.+-+Abs%5BSin%5By%5D%5D%29+-+Abs%5BCos%5By%5D%5D%29%7D%7D%5E0.65
    // one last final transform (with choppy = 4) results in this which resembles a recognizable ocean wave shape in 2D:
    // http://www.wolframalpha.com/input/?i=%7B1-%7B%7B%7BAbs%5BCos%5Bx%5D%5D+%2B+Abs%5BCos%5Bx%5D%5D+%28%281.+-+Abs%5BSin%5Bx%5D%5D%29+-+Abs%5BCos%5Bx%5D%5D%29%7D+*+%7BAbs%5BCos%5By%5D%5D+%2B+Abs%5BCos%5By%5D%5D+%28%281.+-+Abs%5BSin%5By%5D%5D%29+-+Abs%5BCos%5By%5D%5D%29%7D%7D%5E0.65%7D%7D%5E4
    // Note that this function is called with a specific frequency multiplier which will stretch out the wave.  Here is the graph
    // with the base frequency used by map and map_detailed (0.16):
    // http://www.wolframalpha.com/input/?i=%7B1-%7B%7B%7BAbs%5BCos%5B0.16x%5D%5D+%2B+Abs%5BCos%5B0.16x%5D%5D+%28%281.+-+Abs%5BSin%5B0.16x%5D%5D%29+-+Abs%5BCos%5B0.16x%5D%5D%29%7D+*+%7BAbs%5BCos%5B0.16y%5D%5D+%2B+Abs%5BCos%5B0.16y%5D%5D+%28%281.+-+Abs%5BSin%5B0.16y%5D%5D%29+-+Abs%5BCos%5B0.16y%5D%5D%29%7D%7D%5E0.65%7D%7D%5E4+from+-20+to+20
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

// bteitler: Compute the distance along Y axis of a point to the surface of the ocean
// using a low(er) resolution ocean height composition function (less iterations).
float map(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    // bteitler: Compose our wave noise generation ("sea_octave") with different frequencies
    // and offsets to achieve a final height map that looks like an ocean.  Likely lots
    // of black magic / trial and error here to get it to look right.  Each sea_octave has this shape:
    // http://www.wolframalpha.com/input/?i=%7B1-%7B%7B%7BAbs%5BCos%5B0.16x%5D%5D+%2B+Abs%5BCos%5B0.16x%5D%5D+%28%281.+-+Abs%5BSin%5B0.16x%5D%5D%29+-+Abs%5BCos%5B0.16x%5D%5D%29%7D+*+%7BAbs%5BCos%5B0.16y%5D%5D+%2B+Abs%5BCos%5B0.16y%5D%5D+%28%281.+-+Abs%5BSin%5B0.16y%5D%5D%29+-+Abs%5BCos%5B0.16y%5D%5D%29%7D%7D%5E0.65%7D%7D%5E4+from+-20+to+20
    // which should give you an idea of what is going.  You don't need to graph this function because it
    // appears to your left :)
    float d, h = 0.0;    
    for(int i = 0; i < ITER_GEOMETRY; i++) {
        // bteitler: start out with our 2D symmetric wave at the current frequency
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
        // bteitler: stack wave ontop of itself at an offset that varies over time for more height and wave pattern variance
    	//d += sea_octave((uv-SEA_TIME)*freq,choppy);

        h += d * amp; // bteitler: Bump our height by the current wave function
        
        // bteitler: "Twist" our domain input into a different space based on a permutation matrix
        // The scales of the matrix values affect the frequency of the wave at this iteration, but more importantly
        // it is responsible for the realistic assymetry since the domain is shiftly differently.
        // This is likely the most important parameter for wave topology.
    	uv *=  octave_m;
        
        freq *= 1.9; // bteitler: Exponentially increase frequency every iteration (on top of our permutation)
        amp *= 0.22; // bteitler: Lower the amplitude every frequency, since we are adding finer and finer detail
        // bteitler: finally, adjust the choppy parameter which will effect our base 2D sea_octave shape a bit.  This makes
        // the "waves within waves" have different looking shapes, not just frequency and offset
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

// bteitler: Compute the distance along Y axis of a point to the surface of the ocean
// using a high(er) resolution ocean height composition function (more iterations).
float map_detailed(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    // bteitler: Compose our wave noise generation ("sea_octave") with different frequencies
    // and offsets to achieve a final height map that looks like an ocean.  Likely lots
    // of black magic / trial and error here to get it to look right.  Each sea_octave has this shape:
    // http://www.wolframalpha.com/input/?i=%7B1-%7B%7B%7BAbs%5BCos%5B0.16x%5D%5D+%2B+Abs%5BCos%5B0.16x%5D%5D+%28%281.+-+Abs%5BSin%5B0.16x%5D%5D%29+-+Abs%5BCos%5B0.16x%5D%5D%29%7D+*+%7BAbs%5BCos%5B0.16y%5D%5D+%2B+Abs%5BCos%5B0.16y%5D%5D+%28%281.+-+Abs%5BSin%5B0.16y%5D%5D%29+-+Abs%5BCos%5B0.16y%5D%5D%29%7D%7D%5E0.65%7D%7D%5E4+from+-20+to+20
    // which should give you an idea of what is going.  You don't need to graph this function because it
    // appears to your left :)
    float d, h = 0.0;    
    for(int i = 0; i < ITER_FRAGMENT; i++) {
        // bteitler: start out with our 2D symmetric wave at the current frequency
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
        // bteitler: stack wave ontop of itself at an offset that varies over time for more height and wave pattern variance
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        
        h += d * amp; // bteitler: Bump our height by the current wave function
        
        // bteitler: "Twist" our domain input into a different space based on a permutation matrix
        // The scales of the matrix values affect the frequency of the wave at this iteration, but more importantly
        // it is responsible for the realistic assymetry since the domain is shiftly differently.
        // This is likely the most important parameter for wave topology.
    	uv *= octave_m/1.2;
        
        freq *= 1.9; // bteitler: Exponentially increase frequency every iteration (on top of our permutation)
        amp *= 0.22; // bteitler: Lower the amplitude every frequency, since we are adding finer and finer detail
        // bteitler: finally, adjust the choppy parameter which will effect our base 2D sea_octave shape a bit.  This makes
        // the "waves within waves" have different looking shapes, not just frequency and offset
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

// bteitler:
// p: point on ocean surface to get color for
// n: normal on ocean surface at <p>
// l: light (sun) direction
// eye: ray direction from camera position for this pixel
// dist: distance from camera to point <p> on ocean surface
vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist) {  
    // bteitler: Fresnel is an exponential that gets bigger when the angle between ocean
    // surface normal and eye ray is smaller
    float fresnel = 1.0 - max(dot(n,-eye),0.0);
    fresnel = pow(fresnel,3.0) * 0.45;
        
    // bteitler: Bounce eye ray off ocean towards sky, and get the color of the sky
    vec3 reflected = getSkyColor(reflect(eye,n))*0.89;    
    
    // bteitler: refraction effect based on angle between light surface normal
    vec3 refracted = SEA_BASE + diffuse(n,l,80.0) * SEA_WATER_COLOR * 0.27; 
    
    // bteitler: blend the refracted color with the reflected color based on our fresnel term
    vec3 color = mix(refracted,reflected,fresnel);
    
    // bteitler: Apply a distance based attenuation factor which is stronger
    // at peaks
    float atten = max(1.0 - dot(dist,dist) * 0.001, 0.0);
    color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;
    
    // bteitler: Apply specular highlight
    color += vec3(specular(n,l,eye,60.0));
    
    return color;
}

// bteitler: Estimate the normal at a point <p> on the ocean surface using a slight more detailed
// ocean mapping function (using more noise octaves).
// Takes an argument <eps> (stands for epsilon) which is the resolution to use
// for the gradient.  See here for more info on gradients: https://en.wikipedia.org/wiki/Gradient
// tracing
vec3 getNormal(vec3 p, float eps) {
    // bteitler: Approximate gradient.  An exact gradient would need the "map" / "map_detailed" functions
    // to return x, y, and z, but it only computes height relative to surface along Y axis.  I'm assuming
    // for simplicity and / or optimization reasons we approximate the gradient by the change in ocean
    // height for all axis.
    vec3 n;
    n.y = map_detailed(p); // bteitler: Detailed height relative to surface, temporarily here to save a variable?
    n.x = map_detailed(vec3(p.x+eps,p.y,p.z)) - n.y; // bteitler approximate X gradient as change in height along X axis delta
    n.z = map_detailed(vec3(p.x,p.y,p.z+eps)) - n.y; // bteitler approximate Z gradient as change in height along Z axis delta
    // bteitler: Taking advantage of the fact that we know we won't have really steep waves, we expect
    // the Y normal component to be fairly large always.  Sacrifices yet more accurately to avoid some calculation.
    n.y = eps; 
    return normalize(n);

    // bteitler: A more naive and easy to understand version could look like this and
    // produces almost the same visuals and is a little more expensive.
    // vec3 n;
    // float h = map_detailed(p);
    // n.y = map_detailed(vec3(p.x,p.y+eps,p.z)) - h;
    // n.x = map_detailed(vec3(p.x+eps,p.y,p.z)) - h;
    // n.z = map_detailed(vec3(p.x,p.y,p.z+eps)) - h;
    // return normalize(n);
}

// bteitler: Find out where a ray intersects the current ocean
float heightMapTracing(vec3 ori, vec3 dir, out vec3 p) {  
    float tm = 0.0;
    float tx = 500.0; // bteitler: a really far distance, this could likely be tweaked a bit as desired

    // bteitler: At a really far away distance along the ray, what is it's height relative
    // to the ocean in ONLY the Y direction?
    float hx = map(ori + dir * tx);
    
    // bteitler: A positive height relative to the ocean surface (in Y direction) at a really far distance means
    // this pixel is pure sky.  Quit early and return the far distance constant.
    if(hx > 0.0) return tx;   

    // bteitler: hm starts out as the height of the camera position relative to ocean.
    float hm = map(ori + dir * tm); 
   
    // bteitler: This is the main ray marching logic.  This is probably the single most confusing part of the shader
    // since height mapping is not an exact distance field (tells you distance to surface if you drop a line down to ocean
    // surface in the Y direction, but there could have been a peak at a very close point along the x and z 
    // directions that is closer).  Therefore, it would be possible/easy to overshoot the surface using the raw height field
    // as the march distance.  The author uses a trick to compensate for this.
    float tmid = 0.0;
    for(int i = 0; i < NUM_STEPS; i++) { // bteitler: Constant number of ray marches per ray that hits the water
        // bteitler: Move forward along ray in such a way that has the following properties:
        // 1. If our current height relative to ocean is higher, move forward more
        // 2. If the height relative to ocean floor very far along the ray is much lower
        //    below the ocean surface, move forward less
        // Idea behind 1. is that if we are far above the ocean floor we can risk jumping
        // forward more without shooting under ocean, because the ocean is mostly level.
        // The idea behind 2. is that if extruding the ray goes farther under the ocean, then 
        // you are looking more orthgonal to ocean surface (as opposed to looking towards horizon), and therefore
        // movement along the ray gets closer to ocean faster, so we need to move forward less to reduce risk
        // of overshooting.
        tmid = mix(tm,tx, hm/(hm-hx));
        p = ori + dir * tmid; 
                  
    	float hmid = map(p); // bteitler: Re-evaluate height relative to ocean surface in Y axis

        if(hmid < 0.0) { // bteitler: We went through the ocean surface if we are negative relative to surface now
            // bteitler: So instead of actually marching forward to cross the surface, we instead
            // assign our really far distance and height to be where we just evaluated that crossed the surface.
            // Next iteration will attempt to go forward more and is less likely to cross the boundary.
            // A naive implementation might have returned <tmid> immediately here, which
            // results in a much poorer / somewhat indeterministic quality rendering.
            tx = tmid;
            hx = hmid;
        } else {
            // Haven't hit surface yet, easy case, just march forward
            tm = tmid;
            hm = hmid;
        }
    }

    // bteitler: Return the distance, which should be really close to the height map without going under the ocean
    return tmid;
}

vec3 ExperimentalWater(vec2 coord, vec3 waterMapLower)
{
	 // bteitler: 2D Pixel location passed in as raw pixel, let's divide by resolution
    // to convert to coordinates between 0 and 1
    vec2 uv = coord;

    uv = uv * 2.0 - 1.0; //  bteitler: Shift pixel coordinates from 0 to 1 to between -1 and 1
    //uv -= 0.75;
    uv.x *= u_Dimensions.x / u_Dimensions.y; // bteitler: Aspect ratio correction - if you don't do this your rays will be distorted
    float time = u_Time*0.3;
        
    // ray

	vec3 ori = (ViewOrigin.xyz / u_MapInfo.xyz) * 0.5 + 0.5;//(ViewOrigin.xyz + u_MapInfo.xyz) / (u_MapInfo.xyz * 2.0); // Make everything positive numbers...
	ori.y *= u_Local0.g;
	ori.x *= u_Local0.b;
	ori.z *= u_Local0.a;

	vec3 wori = (waterMapLower.xyz / u_MapInfo.xyz) * 0.5 + 0.5;//(waterMapLower.xyz + u_MapInfo.xyz) / (u_MapInfo.xyz * 2.0); // Make everything positive numbers...

    // bteitler: Calculated a vector that smoothly changes over time in a sinusoidal (wave) pattern.  
    // This will be used to drive where the user is looking in world space.
    //vec3 ang = vec3(0.0, 1.25, 0.0);
    vec3 ang = (normalize(ViewOrigin.xyz - waterMapLower.xyz) * 0.5 + 0.5) * u_Local0.r;// * 1.25;//vec3(0.0, 0.0, 0.0); // 0.0 - flat, 3.14 upside down flat // 1.57??
    
    // bteitler: Calculate the "origin" of the camera in world space based on time.  Camera is located
    // at height 3.5, at x 0 (zero), and flies over the ocean in the z axis over time.
    //vec3 ori = vec3(length(ViewOrigin.x - waterMapLower.x), length(ViewOrigin.y - waterMapLower.y), length(ViewOrigin.z - waterMapLower.z));//vec3(0.0,5.5,0.0);
   
    // bteitler: This is the ray direction we are shooting from the camera location ("ori") that we need to light
    // for this pixel.  The -2.0 indicates we are using a focal length of 2.0 - this is just an artistic choice and
    // results in about a 90 degree field of view.
    vec3 dir = normalize(vec3(uv.xy,-2.0)); 

    // bteitler: Distort the ray a bit for a fish eye effect (if you remove this line, it will remove
    // the fish eye effect and look like a realistic perspective).
    // dir.z += length(uv) * 0.15;

    // bteitler: Renormalize the ray direction, and then rotate it based on the previously calculated
    // animation angle "ang".  "fromEuler" just calculates a rotation matrix from a vector of angles.
    // if you remove the " * fromEuler(ang)" part, you will disable the camera rotation animation.
    dir = normalize(dir) * fromEuler(ang);
    
    // tracing

    // bteitler: ray-march to the ocean surface (which can be thought of as a randomly generated height map)
    // and store in p
    vec3 p;
    heightMapTracing(ori,dir,p);

    vec3 dist = p - ori; // bteitler: distance vector to ocean surface for this pixel's ray

    // bteitler: Calculate the normal on the ocean surface where we intersected (p), using
    // different "resolution" (in a sense) based on how far away the ray traveled.  Normals close to
    // the camera should be calculated with high resolution, and normals far from the camera should be calculated with low resolution
    // The reason to do this is that specular effects (or non linear normal based lighting effects) become fairly random at
    // far distances and low resolutions and can cause unpleasant shimmering during motion.
    vec3 n = getNormal(p, 
             dot(dist,dist)   // bteitler: Think of this as inverse resolution, so far distances get bigger at an expnential rate
                * EPSILON_NRM // bteitler: Just a resolution constant.. could easily be tweaked to artistic content
           );

    // bteitler: direction of the infinitely far away directional light.  Changing this will change
    // the sunlight direction.
    vec3 light = normalize(vec3(0.0,1.0,0.8)); 
             
    // CaliCoastReplay:  Get the sky and sea colors
    vec3 seaColor = getSeaColor(p,n,light,dir,dist);
    
    //Sea/sky preprocessing
    
    //CaliCoastReplay:  A distance falloff for the sea color.   Drastically darkens the sea, 
    //this will be reversed later based on day/night.
    seaColor /= sqrt(sqrt(length(dist))) * 1.2;
    
    
    //CaliCoastReplay:  Day/night mode
    bool night; 	 
    /*if( isKeyPressed(KEY_SP) > 0.0 )    //night mode!
    {
        //Brighten the sea up again
    	seaColor *= seaColor * 8.5;
        night = true;
    }
    else*/  //day mode!
    {
        //Brighten the sea up again
    	seaColor *= sqrt(sqrt(seaColor)) * 5.0;
        night = false;
    }
    
    //CaliCoastReplay:  A simple but pretty beautiful hack that actually darkens
    //the highest-value spots somewhat to add even more contrast
    vec3 seaHsv = rgb2hsv(seaColor);
    if (seaHsv.z > .78 && length(dist) < 50.0)
        seaHsv.z -= (0.85 - seaHsv.z) * 1.8;
    seaColor = hsv2rgb(seaHsv);
    
	//    
    // Postprocessing
    //

    // bteitler: Apply an overall image brightness factor as the final color for this pixel.  Can be
    // tweaked artistically.
    vec3 color = pow(seaColor,vec3(0.75));
    
    // CaliCoastReplay:  Adjust hue, saturation, and value adjustment for an even more processed look
    // hsv.x is hue, hsv.y is saturation, and hsv.z is value
    vec3 hsv = rgb2hsv(color.xyz);    
    //CaliCoastReplay: Increase saturation slightly
    hsv.y += 0.091;
    //CaliCoastReplay:
    //A pseudo-multiplicative adjustment of value, increasing intensity near 1 and decreasing it near
    //0 to achieve a more contrasted, real-world look
    hsv.z *= sqrt(hsv.z) * 1.2; 
    
    if (night)    
    {
    ///CaliCoastReplay:
    //Slight value adjustment at daynight to turn down global intensity
        hsv.z -= 0.045;
        hsv*=0.7;
    }
    else
    {
      //CaliCoastReplay:
        //Add green tinge to the high range
      //Turn down intensity in day in a different way     
        
        hsv.z *= 0.9;
        
        //CaliCoastReplay:  Hue alteration 
        hsv.x -= hsv.z/10.0;
        //Final brightening
        hsv.z *= 1.2;
    }
    
    //CaliCoastReplay:    
    //Replace the final color with the adjusted, translated HSV values
    color.xyz = hsv2rgb(hsv);

	return color.rgb;
}
#endif //EXPERIMENTAL

void main ( void )
{
	vec3 color2 = textureLod(u_DiffuseMap, var_TexCoords, 0.0).rgb;
	vec4 waterMapUpper = waterMapUpperAtCoord(var_TexCoords);
	waterMapUpper.y += waveHeight;
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
	bool pixelIsWaterfall = false;
	vec3 color = color2;

	vec4 waterMapLower = waterMapUpper;//waterMapLowerAtCoord(var_TexCoords);
	waterMapLower.y -= waveHeight;

	vec4 positionMap = positionMapAtCoord(var_TexCoords);
	vec3 position = positionMap.xyz;

#if defined(FIX_WATER_DEPTH_ISSUES)
	if (positionMap.a-1.0 == 1024.0)
	{
		position.xyz = waterMapLower.xyz;
		position.y -= 1024.0;
	}
#endif //defined(FIX_WATER_DEPTH_ISSUES)

	if (waterMapLower.a >= 2.0 || waterMapUpper.a >= 2.0)
	{// Low horizontal normal, this is a waterfall...
		pixelIsWaterfall = true;
	}
	else if (IS_UNDERWATER || waterMapLower.y > ViewOrigin.y)
	{
		pixelIsUnderWater = true;
	}
	else if (waterMapUpper.y >= position.y)
	{
		if (waterMapLower.y < position.y)
		{
			waterMapLower.y = position.y;
			waterMapLower.a = 0.0;
		}

		if (waterMapUpper.y >= position.y)
		{
			pixelIsInWaterRange = true;
		}
	}

	if (!pixelIsInWaterRange && !pixelIsWaterfall && !pixelIsUnderWater)
	{// No water here. Skip calculations.
		gl_FragColor = vec4(color2, 1.0);
		return;
	}

	float timer = systemtimer * (waveHeight / 16.0);

	if (pixelIsWaterfall)
	{// How???
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
		
#ifdef FOAM_SPLAT_MAPS
		vec3 foam = (GetFoamMap(vec3(waterMapUpper.x * 0.03, (waterMapUpper.y * 0.03) + fTime, 0.5), vec2(waterMapUpper.x * 0.03, (waterMapUpper.y * 0.03) + fTime)).rgb + GetFoamMap(vec3(waterMapUpper.z * 0.03, (waterMapUpper.y * 0.03) + fTime, 0.5), vec2(waterMapUpper.z * 0.03, (waterMapUpper.y * 0.03) + fTime)).rgb) * 0.5;
#else //!FOAM_SPLAT_MAPS
		vec3 foam = (GetFoamMap(vec2(waterMapUpper.x * 0.03, (waterMapUpper.y * 0.03) + fTime)).rgb + GetFoamMap(vec2(waterMapUpper.z * 0.03, (waterMapUpper.y * 0.03) + fTime)).rgb) * 0.5;
#endif //FOAM_SPLAT_MAPS

		color = mix(color + (foam * sunColor), refraction, fresnel * 0.8);

		if(lambertian2 > 0.0)
		{// this is blinn phong
			vec3 mirrorEye = (2.0 * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
			vec3 halfDir2 = normalize(lightDir.xyz + mirrorEye);
			float specAngle = max(dot(halfDir2, normal), 0.0);
			spec2 = pow(specAngle, 16.0);
			specular = vec3(clamp(1.0 - fresnel, 0.4, 1.0)) * (vec3(spec2 * shininess)) * sunColor * specularScale * 25.0;
		}

		color = clamp(color + specular, 0.0, 1.0);
		color += vec3(getspecular(normal, lightDir, eyeVecNorm, 60.0));
		color = AddReflection(texCoord, position.xyz, waterMapUpper.xyz, color);

		gl_FragColor = vec4(color, 1.0);
		return;
	}

	float waterLevel = waterMapLower.y;
	float level = waterMapLower.y;
	float depth = 0.0;

	if (pixelIsInWaterRange || pixelIsUnderWater)
	{
#ifdef EXPERIMENTAL
		gl_FragColor = vec4(ExperimentalWater(var_TexCoords, waterMapLower.xyz), 1.0);
		return;
#endif //EXPERIMENTAL
		vec2 wind = normalize(waterMapLower.xz); // Waves head toward center of map. Should suit current WZ maps...

		// Find intersection with water surface
		vec3 eyeVecNorm = normalize(ViewOrigin - waterMapLower.xyz/*position*/);
		float t = ((level - ViewOrigin.y) / eyeVecNorm.y);
		vec3 surfacePoint = ViewOrigin + eyeVecNorm * t;

		vec2 texCoord;

#ifdef REAL_WAVES
		for(int i = 0; i < 10; i++)
#endif //REAL_WAVES
		{
			texCoord = ((surfacePoint.xz + eyeVecNorm.xz * 0.1) * scale + timer * 0.000005 * wind) * waveDensity;
			
			float bias = textureLod(u_WaterHeightMap, texCoord, 0.0).r;
	
			bias *= 0.1;
			level += bias * waveHeight;

			t = ((level - ViewOrigin.y) / eyeVecNorm.y);
			surfacePoint = ViewOrigin + eyeVecNorm * t;
		}

		// level, surfacePoint
		{// Adjust level for low view angles...
			//float heightDiff = (waveHeight - clamp(length(ViewOrigin.y - waterMapLower.y) / waveHeight, 0.0, 1.0));
			//float heightDiff = level - waterMapLower.y;
			//float diff = length(heightDiff / waveHeight);
			//level = waterMapLower.y + (heightDiff * diff * waveHeight);
			//surfacePoint.y = level;

			//t = (((level + heightDiff) - ViewOrigin.y) / eyeVecNorm.y);
			//surfacePoint = ViewOrigin + eyeVecNorm * t;
		}

		depth = length(position - surfacePoint);
		float depth2 = surfacePoint.y - position.y;
		float depthN = depth * fadeSpeed;

		float heightAboveGround = clamp(depth2 / waveHeight, 0.0, 1.0);

		if (pixelIsUnderWater)
		{
			//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			//return;

			depth = length(surfacePoint - position);
			depth2 = position.y - surfacePoint.y;
			depthN = depth * fadeSpeed;

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
		else if (depth2 < 0.0)
		{// Waves against shoreline. Pixel is above waterLevel + waveHeight... (but ignore anything marked as actual water - eg: not a shoreline)
			gl_FragColor = vec4(color2, 1.0);
			return;
		}


		eyeVecNorm = normalize(ViewOrigin - surfacePoint);

		float normal1 = textureLod(u_WaterHeightMap, (texCoord + (vec2(-1.0, 0.0) / 256.0)), 0.0).r;
		float normal2 = textureLod(u_WaterHeightMap, (texCoord + (vec2(1.0, 0.0) / 256.0)), 0.0).r;
		float normal3 = textureLod(u_WaterHeightMap, (texCoord + (vec2(0.0, -1.0) / 256.0)), 0.0).r;
		float normal4 = textureLod(u_WaterHeightMap, (texCoord + (vec2(0.0, 1.0) / 256.0)), 0.0).r;
		
		vec3 myNormal = normalize(vec3((normal1 - normal2) * waveHeight,
										   normalScale,
										   (normal3 - normal4) * waveHeight));

		texCoord = surfacePoint.xz * 1.6 + wind * timer * 0.00016;
		mat3 tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal0a = normalize(tangentFrame * (2.0 * textureLod(u_NormalMap, texCoord, 0.0).xyz - 1.0));

		texCoord = surfacePoint.xz * 0.8 + wind * timer * 0.00008;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal1a = normalize(tangentFrame * (2.0 * textureLod(u_NormalMap, texCoord, 0.0).xyz - 1.0));
		
		texCoord = surfacePoint.xz * 0.4 + wind * timer * 0.00004;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal2a = normalize(tangentFrame * (2.0 * textureLod(u_NormalMap, texCoord, 0.0).xyz - 1.0));
		
		texCoord = surfacePoint.xz * 0.1 + wind * timer * 0.00002;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal3a = normalize(tangentFrame * (2.0 * textureLod(u_NormalMap, texCoord, 0.0).xyz - 1.0));
		
		vec3 normal = normalize(((normal0a * normalModifier.x) + (normal1a * normalModifier.y) +
						(normal2a * normalModifier.z) + (normal3a * normalModifier.w)));

		texCoord = var_TexCoords.xy;

		texCoord.x += sin(timer * 0.002 + 3.0 * abs(position.y)) * (refractionScale * min(depth2, 1.0));

		vec3 refraction = textureLod(u_DiffuseMap, texCoord, 0.0).rgb;

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

#ifdef FOAM_SPLAT_MAPS
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
#else //!FOAM_SPLAT_MAPS
		if (depth2 < foamExistence.x)
		{
			foam = (GetFoamMap(texCoord) + GetFoamMap(texCoord2)) * 0.5;
		}
		else if (depth2 < foamExistence.y)
		{
			foam = mix((GetFoamMap(texCoord) + GetFoamMap(texCoord2)) * 0.5, vec4(0.0),
						vec4((depth2 - foamExistence.x) / (foamExistence.y - foamExistence.x)));
		}
		
		if (waveHeight - foamExistence.z > 0.0001)
		{
			foam += (GetFoamMap(texCoord) + GetFoamMap(texCoord2)) * 0.5 * 
				clamp((level - (waterLevel + foamExistence.z)) / (waveHeight - foamExistence.z), 0.0, 1.0);
		}
#endif //FOAM_SPLAT_MAPS
		
		causicStrength *= 0.15 - clamp(max(foam.r, max(foam.g, foam.b)) * foam.a * 32.0, 0.0, 0.15);

		vec3 specular = vec3(0.0);

		vec3 lightDir = normalize(ViewOrigin.xyz - u_PrimaryLightOrigin.xzy);
		//normal *= -myNormal.xyz;

		float lambertian2 = dot(lightDir.xyz, normal);
		float spec2 = 0.0;
		
		float fresnel = clamp(1.0 - dot(normal, -eyeVecNorm), 0.0, 1.0);
		fresnel = pow(fresnel, 3.0) * 0.65;

		if(lambertian2 > 0.0)
		{// this is blinn phong
			vec3 mirrorEye = (2.0 * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
			vec3 halfDir2 = normalize(lightDir.xyz + mirrorEye);
			float specAngle = max(dot(halfDir2, normal), 0.0);
			spec2 = pow(specAngle, 16.0);
			specular = vec3(clamp(1.0 - fresnel, 0.4, 1.0)) * (vec3(spec2 * shininess)) * sunColor * specularScale * 25.0;
		}


		vec3 dist = -eyeVecNorm;

		color = mix(refraction, waterColorDeep, fresnel);

		float atten = max(1.0 - dot(dist, dist) * 0.001, 0.0);
		color += waterColorShallow.rgb * (clamp(waveHeight - waterMapLower.y, 0.0, 1.0))* 0.18 * atten;

		vec3 caustic = color * (texture(u_DetailMap, vec2((texCoord.x + (texCoord2.x*2.2)) * 0.25, (texCoord.y + (texCoord2.y*1.2)) * 0.25)).rgb * 1.1);

		color += vec3(getspecular(normal, lightDir, eyeVecNorm, 60.0));
		/* END - TESTING */

		// Also do dlights. Includes map glows and sabers and all that good stuff...
		if (u_lightCount > 0.0)
		{
#define LIGHT_COLOR_POWER			4.0

			vec3 addedLight = vec3(0.0);
			float power = clamp(length(color.rgb) / 3.0, 0.0, 1.0) * 0.5 + 0.5;
			power = pow(power, LIGHT_COLOR_POWER);
			power = power * 0.5 + 0.5;

			float maxStr = max(color.r, max(color.g, color.b)) * 0.9 + 0.1;

			for (int li = 0; li < u_lightCount; li++)
			{
				vec3 lightPos = u_lightPositions2[li].xyz;

				float lightDist = distance(lightPos, waterMapLower3.xzy/*position.xyz*/);

				if (u_lightHeightScales[li] > 0.0)
				{// ignore height differences, check later...
					lightDist -= length(lightPos.z - waterMapLower3.y/*position.z*/);
				}

				float lightDistMult = 1.0 - clamp((distance(lightPos.xyz, u_ViewOrigin.xyz) / 4096.0), 0.0, 1.0);
				float lightStrength = pow(1.0 - clamp(lightDist / u_lightDistances[li], 0.0, 1.0), 2.0);
				lightStrength *= lightDistMult;

				if (lightStrength > 0.0)
				{
					vec3 lightColor = (u_lightColors[li].rgb / length(u_lightColors[li].rgb)) * 4.0;//1.5; // Normalize.
					vec3 lightDir2 = normalize(lightPos - waterMapLower3.xzy/*position.xyz*/);
				
					lightColor = lightColor * lightStrength * power;

					lightColor *= maxStr;
					addedLight.rgb += (lightColor * vec3(getspecular(-normal, -lightDir2, eyeVecNorm, 60.0)));
				}
			}

			color.rgb = color.rgb + addedLight;
		}


#if defined(USE_REFLECTION)
		if (!pixelIsUnderWater && u_Local1.g >= 2.0)
		{
			color = AddReflection(var_TexCoords, position, vec3(waterMapLower3.x, level, waterMapLower3.z), color.rgb);
		}
#endif //defined(USE_REFLECTION)

		color = clamp(color + (caustic * causicStrength), 0.0, 1.0);

		color = clamp(color + max(specular, foam.rgb * sunColor), 0.0, 1.0);

		color = mix(refraction, color, clamp(depth * shoreHardness, 0.0, 1.0));

		color = mix(color, color2, 1.0 - clamp(waterClarity * depth, 0.8, 1.0));

		if (!pixelIsUnderWater)
		{
			color.rgb = mix(color2.rgb, color.rgb, heightAboveGround);
			
			if (clamp(1.0 - (heightAboveGround * 0.65), 0.0, 1.0) > 0.5)
				color.rgb = color2.rgb;
		}
	}

	gl_FragColor = vec4(color, 1.0);
}
