#define __AMBIENT_OCCLUSION__
#define __ENVMAP__
//#define __NORMAL_METHOD_1__
#define __NORMAL_METHOD_2__
#define __RANDOMIZE_LIGHT_PIXELS__

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_OverlayMap; // Real normals. Alpha channel 1.0 means enabled...
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_HeightMap;
uniform sampler2D	u_ShadowMap;
uniform sampler2D	u_DeluxeMap;
uniform sampler2D	u_GlowMap;
uniform samplerCube	u_CubeMap;

uniform vec2		u_Dimensions;

uniform vec4		u_Local1; // r_blinnPhong, SUN_PHONG_SCALE, r_ao, r_env
uniform vec4		u_Local2; // SSDO, SHADOWS_ENABLED, SHADOW_MINBRIGHT, SHADOW_MAXBRIGHT
uniform vec4		u_Local3; // r_testShaderValue1, r_testShaderValue2, r_testShaderValue3, r_testShaderValue4
uniform vec4		u_Local4; // MAP_INFO_MAXSIZE, MAP_WATER_LEVEL, floatTime, MAP_EMISSIVE_COLOR_SCALE
uniform vec4		u_Local5; // CONTRAST, SATURATION, BRIGHTNESS, 0.0
uniform vec4		u_Local6; // AO_MINBRIGHT, AO_MULTBRIGHT, 0.0, 0.0

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

#ifdef __NORMAL_METHOD_1__
const vec3 LUMA_COEFFICIENT = vec3(0.2126, 0.7152, 0.0722);

float lumaAtCoord(vec2 coord) {
  vec3 pixel = texture(u_DiffuseMap, coord).rgb;
  float luma = dot(pixel, LUMA_COEFFICIENT);
  return luma;
}

vec4 normalVector(vec2 coord) {
  float lumaU0 = lumaAtCoord(coord + vec2(-1.0,  0.0) / u_Dimensions);
  float lumaU1 = lumaAtCoord(coord + vec2( 1.0,  0.0) / u_Dimensions);
  float lumaV0 = lumaAtCoord(coord + vec2( 0.0, -1.0) / u_Dimensions);
  float lumaV1 = lumaAtCoord(coord + vec2( 0.0,  1.0) / u_Dimensions);

  vec2 slope = vec2(lumaU0 - lumaU1, lumaV0 - lumaV1) * 0.5 + 0.5;

// Contrast...
#define normLower ( 128.0/*48.0*/ / 255.0 )
#define normUpper (255.0 / 192.0/*128.0*/ )
  slope = clamp((clamp(slope - normLower, 0.0, 1.0)) * normUpper, 0.0, 1.0);

  return vec4(slope, 1.0, length(slope.rg / 2.0));
}
#endif //__NORMAL_METHOD_1__

#ifdef __NORMAL_METHOD_2__
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
#endif //__NORMAL_METHOD_2__


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

vec3 QuickMix(vec3 color1, vec3 color2, float mix)
{
	float mixVal = clamp(mix, 0.0, 1.0);
	return vec3((color1 * (1.0 - mixVal)) + (color2 * mixVal));
}

/*
** Desaturation
*/

vec4 Desaturate(vec3 color, float Desaturation)
{
	vec3 grayXfer = vec3(0.3, 0.59, 0.11);
	vec3 gray = vec3(dot(grayXfer, color));
	return vec4(mix(color, gray, Desaturation), 1.0);
}


/*
** Hue, saturation, luminance
*/

vec3 RGBToHSL(vec3 color)
{
	vec3 hsl; // init to 0 to avoid warnings ? (and reverse if + remove first part)
	
	float fmin = min(min(color.r, color.g), color.b);    //Min. value of RGB
	float fmax = max(max(color.r, color.g), color.b);    //Max. value of RGB
	float delta = fmax - fmin;             //Delta RGB value

	hsl.z = (fmax + fmin) / 2.0; // Luminance

	if (delta == 0.0)		//This is a gray, no chroma...
	{
		hsl.x = 0.0;	// Hue
		hsl.y = 0.0;	// Saturation
	}
	else                                    //Chromatic data...
	{
		if (hsl.z < 0.5)
			hsl.y = delta / (fmax + fmin); // Saturation
		else
			hsl.y = delta / (2.0 - fmax - fmin); // Saturation
		
		float deltaR = (((fmax - color.r) / 6.0) + (delta / 2.0)) / delta;
		float deltaG = (((fmax - color.g) / 6.0) + (delta / 2.0)) / delta;
		float deltaB = (((fmax - color.b) / 6.0) + (delta / 2.0)) / delta;

		if (color.r == fmax )
			hsl.x = deltaB - deltaG; // Hue
		else if (color.g == fmax)
			hsl.x = (1.0 / 3.0) + deltaR - deltaB; // Hue
		else if (color.b == fmax)
			hsl.x = (2.0 / 3.0) + deltaG - deltaR; // Hue

		if (hsl.x < 0.0)
			hsl.x += 1.0; // Hue
		else if (hsl.x > 1.0)
			hsl.x -= 1.0; // Hue
	}

	return hsl;
}

float HueToRGB(float f1, float f2, float hue)
{
	if (hue < 0.0)
		hue += 1.0;
	else if (hue > 1.0)
		hue -= 1.0;
	float res;
	if ((6.0 * hue) < 1.0)
		res = f1 + (f2 - f1) * 6.0 * hue;
	else if ((2.0 * hue) < 1.0)
		res = f2;
	else if ((3.0 * hue) < 2.0)
		res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
	else
		res = f1;
	return res;
}

vec3 HSLToRGB(vec3 hsl)
{
	vec3 rgb;
	
	if (hsl.y == 0.0)
		rgb = vec3(hsl.z); // Luminance
	else
	{
		float f2;
		
		if (hsl.z < 0.5)
			f2 = hsl.z * (1.0 + hsl.y);
		else
			f2 = (hsl.z + hsl.y) - (hsl.y * hsl.z);
			
		float f1 = 2.0 * hsl.z - f2;
		
		rgb.r = HueToRGB(f1, f2, hsl.x + (1.0/3.0));
		rgb.g = HueToRGB(f1, f2, hsl.x);
		rgb.b = HueToRGB(f1, f2, hsl.x - (1.0/3.0));
	}
	
	return rgb;
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


/*
** Float blending modes
** Adapted from here: http://www.nathanm.com/photoshop-blending-math/
** But I modified the HardMix (wrong condition), Overlay, SoftLight, ColorDodge, ColorBurn, VividLight, PinLight (inverted layers) ones to have correct results
*/

#define BlendLinearDodgef 			BlendAddf
#define BlendLinearBurnf 			BlendSubstractf
#define BlendAddf(base, blend) 		min(base + blend, 1.0)
#define BlendSubstractf(base, blend) 	max(base + blend - 1.0, 0.0)
#define BlendLightenf(base, blend) 		max(blend, base)
#define BlendDarkenf(base, blend) 		min(blend, base)
#define BlendLinearLightf(base, blend) 	(blend < 0.5 ? BlendLinearBurnf(base, (2.0 * blend)) : BlendLinearDodgef(base, (2.0 * (blend - 0.5))))
#define BlendScreenf(base, blend) 		(1.0 - ((1.0 - base) * (1.0 - blend)))
#define BlendOverlayf(base, blend) 	(base < 0.5 ? (2.0 * base * blend) : (1.0 - 2.0 * (1.0 - base) * (1.0 - blend)))
#define BlendSoftLightf(base, blend) 	((blend < 0.5) ? (2.0 * base * blend + base * base * (1.0 - 2.0 * blend)) : (sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend)))
#define BlendColorDodgef(base, blend) 	((blend == 1.0) ? blend : min(base / (1.0 - blend), 1.0))
#define BlendColorBurnf(base, blend) 	((blend == 0.0) ? blend : max((1.0 - ((1.0 - base) / blend)), 0.0))
#define BlendVividLightf(base, blend) 	((blend < 0.5) ? BlendColorBurnf(base, (2.0 * blend)) : BlendColorDodgef(base, (2.0 * (blend - 0.5))))
#define BlendPinLightf(base, blend) 	((blend < 0.5) ? BlendDarkenf(base, (2.0 * blend)) : BlendLightenf(base, (2.0 *(blend - 0.5))))
#define BlendHardMixf(base, blend) 	((BlendVividLightf(base, blend) < 0.5) ? 0.0 : 1.0)
#define BlendReflectf(base, blend) 		((blend == 1.0) ? blend : min(base * base / (1.0 - blend), 1.0))


/*
** Vector3 blending modes
*/

// Component wise blending
#define Blend(base, blend, funcf) 		vec3(funcf(base.r, blend.r), funcf(base.g, blend.g), funcf(base.b, blend.b))

#define BlendNormal(base, blend) 		(blend)
#define BlendLighten				BlendLightenf
#define BlendDarken				BlendDarkenf
#define BlendMultiply(base, blend) 		(base * blend)
#define BlendAverage(base, blend) 		((base + blend) / 2.0)
#define BlendAdd(base, blend) 		min(base + blend, vec3(1.0))
#define BlendSubstract(base, blend) 	max(base + blend - vec3(1.0), vec3(0.0))
#define BlendDifference(base, blend) 	abs(base - blend)
#define BlendNegation(base, blend) 	(vec3(1.0) - abs(vec3(1.0) - base - blend))
#define BlendExclusion(base, blend) 	(base + blend - 2.0 * base * blend)
#define BlendScreen(base, blend) 		Blend(base, blend, BlendScreenf)
#define BlendOverlay(base, blend) 		Blend(base, blend, BlendOverlayf)
#define BlendSoftLight(base, blend) 	Blend(base, blend, BlendSoftLightf)
#define BlendHardLight(base, blend) 	BlendOverlay(blend, base)
#define BlendColorDodge(base, blend) 	Blend(base, blend, BlendColorDodgef)
#define BlendColorBurn(base, blend) 	Blend(base, blend, BlendColorBurnf)
#define BlendLinearDodge			BlendAdd
#define BlendLinearBurn			BlendSubstract
// Linear Light is another contrast-increasing mode
// If the blend color is darker than midgray, Linear Light darkens the image by decreasing the brightness. If the blend color is lighter than midgray, the result is a brighter image due to increased brightness.
#define BlendLinearLight(base, blend) 	Blend(base, blend, BlendLinearLightf)
#define BlendVividLight(base, blend) 	Blend(base, blend, BlendVividLightf)
#define BlendPinLight(base, blend) 		Blend(base, blend, BlendPinLightf)
#define BlendHardMix(base, blend) 		Blend(base, blend, BlendHardMixf)
#define BlendReflect(base, blend) 		Blend(base, blend, BlendReflectf)
#define BlendGlow(base, blend) 		BlendReflect(blend, base)
#define BlendPhoenix(base, blend) 		(min(base, blend) - max(base, blend) + vec3(1.0))
#define BlendOpacity(base, blend, F, O) 	(F(base, blend) * O + blend * (1.0 - O))


// Hue Blend mode creates the result color by combining the luminance and saturation of the base color with the hue of the blend color.
vec3 BlendHue(vec3 base, vec3 blend)
{
	vec3 baseHSL = RGBToHSL(base);
	return HSLToRGB(vec3(RGBToHSL(blend).r, baseHSL.g, baseHSL.b));
}

// Saturation Blend mode creates the result color by combining the luminance and hue of the base color with the saturation of the blend color.
vec3 BlendSaturation(vec3 base, vec3 blend)
{
	vec3 baseHSL = RGBToHSL(base);
	return HSLToRGB(vec3(baseHSL.r, RGBToHSL(blend).g, baseHSL.b));
}

// Color Mode keeps the brightness of the base color and applies both the hue and saturation of the blend color.
vec3 BlendColor(vec3 base, vec3 blend)
{
	vec3 blendHSL = RGBToHSL(blend);
	return HSLToRGB(vec3(blendHSL.r, blendHSL.g, RGBToHSL(base).b));
}

// Luminosity Blend mode creates the result color by combining the hue and saturation of the base color with the luminance of the blend color.
vec3 BlendLuminosity(vec3 base, vec3 blend)
{
	vec3 baseHSL = RGBToHSL(base);
	return HSLToRGB(vec3(baseHSL.r, baseHSL.g, RGBToHSL(blend).b));
}

float blendSoftLight2(float base, float blend) {
	return (blend<0.5)?(2.0*base*blend+base*base*(1.0-2.0*blend)):(sqrt(base)*(2.0*blend-1.0)+2.0*base*(1.0-blend));
}

vec3 blendSoftLight2(vec3 base, vec3 blend) {
	return vec3(blendSoftLight2(base.r,blend.r),blendSoftLight2(base.g,blend.g),blendSoftLight2(base.b,blend.b));
}

vec3 blendSoftLight2(vec3 base, vec3 blend, float opacity) {
	return (blendSoftLight2(base, blend) * opacity + base * (1.0 - opacity));
}

//#define DefaultBlend BlendHue
//#define DefaultBlend BlendSaturation
#define DefaultBlend BlendColor
//#define DefaultBlend BlendLuminosity

/*
** Gamma correction
** Details: http://blog.mouaif.org/2009/01/22/photoshop-gamma-correction-shader/
*/

#define GammaCorrection(color, gamma)								pow(color, 1.0 / gamma)

/*
** Levels control (input (+gamma), output)
** Details: http://blog.mouaif.org/2009/01/28/levels-control-shader/
*/

#define LevelsControlInputRange(color, minInput, maxInput)				min(max(color - vec3(minInput), vec3(0.0)) / (vec3(maxInput) - vec3(minInput)), vec3(1.0))
#define LevelsControlInput(color, minInput, gamma, maxInput)				GammaCorrection(LevelsControlInputRange(color, minInput, maxInput), gamma)
#define LevelsControlOutputRange(color, minOutput, maxOutput) 			mix(vec3(minOutput), vec3(maxOutput), color)
#define LevelsControl(color, minInput, gamma, maxInput, minOutput, maxOutput) 	LevelsControlOutputRange(LevelsControlInput(color, minInput, gamma, maxInput), minOutput, maxOutput)

vec3 blendSoftLight3(vec3 base, vec3 blend) {
    return mix(
        sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend), 
        2.0 * base * blend + base * base * (1.0 - 2.0 * blend), 
        step(base, vec3(0.5))
    );
}

void main(void)
{
	vec4 color = textureLod(u_DiffuseMap, var_TexCoords, 0.0);
	vec4 outColor = vec4(color.rgb, 1.0);

	vec2 texCoords = var_TexCoords;


	vec3 viewOrg = u_ViewOrigin.xyz;
	vec4 position = textureLod(u_PositionMap, texCoords, 0.0);

	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);

	if (position.a == MATERIAL_SKY || position.a == MATERIAL_SUN || position.a == MATERIAL_NONE)
	{// Skybox... Skip...
		if (!(u_Local5.r == 1.0 && u_Local5.g == 1.0 && u_Local5.b == 1.0))
		{
			outColor.rgb = ContrastSaturationBrightness(outColor.rgb, u_Local5.r, u_Local5.g, u_Local5.b);
		}

		outColor.rgb = LevelsControlOutputRange(outColor.rgb, 0.0, 1.0);
		gl_FragColor = outColor;
		return;
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
	
	if (normalDetail.a < 1.0)
	{// Don't have real normalmap, make normals for this pixel...
		normalDetail = normalVector(texCoords);
	}

	normalDetail.rgb = normalize(normalDetail.rgb * 2.0 - 1.0);
	normalDetail.rgb *= 0.25;
	//normalDetail.z = sqrt(clamp((0.25 - normalDetail.x * normalDetail.x) - normalDetail.y * normalDetail.y, 0.0, 1.0));
	norm.rgb = normalize(norm.rgb + normalDetail.rgb);

	//vec3 tangent = TangentFromNormal( norm.xyz );
	//vec3 bitangent = normalize( cross(norm.xyz, tangent) );
	//mat3 tangentToWorld = mat3(tangent.xyz, bitangent.xyz, norm.xyz);
	//norm.xyz = tangentToWorld * normalDetail.xyz;

	vec3 N = norm.xyz;


	float shadowMult = 1.0;

#if defined(USE_SHADOWMAP)
	if (u_Local2.g > 0.0)
	{
		float shadowValue = texture(u_ShadowMap, texCoords).r;

		shadowValue = pow(shadowValue, 1.5);

#define sm_cont_1 ( 64.0 / 255.0)
#define sm_cont_2 (255.0 / 200.0)
		shadowValue = clamp((clamp(shadowValue - sm_cont_1, 0.0, 1.0)) * sm_cont_2, 0.0, 1.0);

		outColor.rgb *= clamp(shadowValue + u_Local2.b, u_Local2.b, u_Local2.a);
		shadowMult = clamp(shadowValue, 0.2, 1.0) * 0.75 + 0.25;
	}
#endif //defined(USE_SHADOWMAP)


	vec3 surfaceToCamera = normalize(u_ViewOrigin.xyz - position.xyz);

	vec3 PrimaryLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);

	vec4 occlusion = vec4(0.0);
	bool useOcclusion = false;

	if (u_Local2.r == 1.0)
	{
		useOcclusion = true;
		occlusion = texture(u_HeightMap, texCoords);
	}

	float reflectivePower = max(norm.a, 0.5);


#if defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__)
	vec3 to_light = position.xyz - u_PrimaryLightOrigin.xyz;
	float to_light_dist = length(to_light);
	vec3 to_light_norm = (to_light / to_light_dist);

	vec3 to_pos = u_ViewOrigin.xyz - position.xyz;
	float to_pos_dist = length(to_pos);
	vec3 to_pos_norm = (to_pos / to_pos_dist);

	#define rd reflect(surfaceToCamera, N)
#endif //defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__)


	float phongFactor = u_Local1.r * u_Local1.g;

#define LIGHT_COLOR_POWER			4.0

	if (phongFactor > 0.0)
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
			outColor.rgb = outColor.rgb + lightColor;
		}
	}

	if (u_lightCount > 0.0)
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
			float lightStrength = pow(1.0 - clamp(lightDist / u_lightDistances[li], 0.0, 1.0), 2.0);
			lightStrength *= lightDistMult * reflectivePower;

			if (lightStrength > 0.0)
			{
				vec3 lightColor = (u_lightColors[li].rgb / length(u_lightColors[li].rgb)) * u_Local4.a; // Normalize.
				vec3 lightDir = normalize(lightPos - position.xyz);
				float light_occlusion = 1.0;
				
				lightColor = lightColor * lightStrength * power;

				//addedLight.rgb += lightColor;

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

		outColor.rgb = outColor.rgb + addedLight;
	}

	//if (u_Local3.a >= 1.0) // Realtime lightmap-ish type coloring/lighting...
		outColor.rgb *= ((outColor.rgb / length(outColor.rgb)) * 3.0) * 0.25 + 0.75;

#ifdef __AMBIENT_OCCLUSION__
	if (u_Local1.b > 0.0)
	{
		float ao = calculateAO(to_light_norm, N * 10000.0);
		ao = clamp(ao * u_Local6.g + u_Local6.r, u_Local6.r, 1.0);
		outColor.rgb *= ao;
	}
#endif //__AMBIENT_OCCLUSION__

#ifdef __ENVMAP__
	if (u_Local1.a > 0.0)
	{
		float lightScale = clamp(1.0 - max(max(outColor.r, outColor.g), outColor.b), 0.0, 1.0);
		float invLightScale = clamp((1.0 - lightScale), 0.2, 1.0);
		vec3 env = envMap(rd, 0.6 /* warmth */);
		outColor.rgb = QuickMix(outColor.rgb, outColor.rgb + ((env * (norm.a * 0.5) * invLightScale) * lightScale), (norm.a * 0.5) * lightScale);
	}
#endif //__ENVMAP__

	if (!(u_Local5.r == 1.0 && u_Local5.g == 1.0 && u_Local5.b == 1.0))
	{
		outColor.rgb = ContrastSaturationBrightness(outColor.rgb, u_Local5.r, u_Local5.g, u_Local5.b);
	}

	outColor.rgb = LevelsControlOutputRange(outColor.rgb, 0.0, 1.0);
	gl_FragColor = outColor;
}

