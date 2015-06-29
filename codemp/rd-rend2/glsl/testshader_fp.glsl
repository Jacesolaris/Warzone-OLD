/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define tex2D(tex, coord) texture2D(tex, coord)
#define tex2Dlod(tex, coord) texture2D(tex, coord)
#define lerp(a, b, t) mix(a, b, t)
#define saturate(a) clamp(a, 0.0, 1.0)
#define mad(a, b, c) (a * b + c)
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define bool2 bvec2
#define bool3 bvec3
#define bool4 bvec4
#define frac fract

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

uniform sampler2D u_DiffuseMap;
//uniform sampler2D u_ScreenDepthMap;

varying vec2		var_TexCoords;
varying vec2		var_Dimensions;

varying vec4		var_Local0; // CURRENT_PASS_NUMBER, 0, 0, 0

float				CURRENT_PASS_NUMBER = var_Local0.x;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

vec2 texCoord = var_TexCoords;
vec4 ScreenSize = vec4(var_Dimensions.x, 1.0 / var_Dimensions.x, var_Dimensions.y, 1.0 / var_Dimensions.y);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

vec4	Timer;
float	EInteriorFactor = 1.0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

vec4 PS_EdgePreservingSmooth() 
{
	float	px 			= 1.0 / var_Dimensions.x;
	float	py 			= 1.0f / var_Dimensions.y;
	vec2	OFFSET		= vec2(px, py);
  
	vec4	ColorInput = texture2D(u_DiffuseMap, var_TexCoords.xy);
	float	colCombined = ColorInput.r + ColorInput.g + ColorInput.b;
	
	float	USE_RADIUS = var_Local0.y;//1.0;
	float	E_SMOOTH_THRESHOLD = 9.0;
	
	vec2 USE_OFFSET = OFFSET.xy;
	vec4  col1 = texture2D(u_DiffuseMap, var_TexCoords.xy + vec2(USE_OFFSET.x, 0.0));
	if (colCombined - (col1.r + col1.g + col1.b) > E_SMOOTH_THRESHOLD) return ColorInput;
	vec4  col2 = texture2D(u_DiffuseMap, var_TexCoords.xy - vec2(USE_OFFSET.x, 0.0));
	if (colCombined - (col2.r + col2.g + col2.b) > E_SMOOTH_THRESHOLD) return ColorInput;
	vec4  col3 = texture2D(u_DiffuseMap, var_TexCoords.xy + vec2(0.0, USE_OFFSET.y));
	if (colCombined - (col3.r + col3.g + col3.b) > E_SMOOTH_THRESHOLD) return ColorInput;
	vec4  col4 = texture2D(u_DiffuseMap, var_TexCoords.xy - vec2(0.0, USE_OFFSET.y));
	if (colCombined - (col4.r + col4.g + col4.b) > E_SMOOTH_THRESHOLD) return ColorInput;
	vec4  col5 = texture2D(u_DiffuseMap, var_TexCoords.xy + vec2(USE_OFFSET.x, 0.0-USE_OFFSET.y) * 0.666);
	if (colCombined - (col5.r + col5.g + col5.b) > E_SMOOTH_THRESHOLD) return ColorInput;
	vec4  col6 = texture2D(u_DiffuseMap, var_TexCoords.xy + vec2(0.0-USE_OFFSET.x, USE_OFFSET.y) * 0.666);
	if (colCombined - (col6.r + col6.g + col6.b) > E_SMOOTH_THRESHOLD) return ColorInput;
  
	// Looks like we are ok to smooth!
	vec4	color = ((ColorInput + col1 + col2 + col3 + col4 + col5 + col6) / 7.0);
  
	float   num_passes = 1.0;
	
	if (USE_RADIUS >= 2.0)
	{
		for (float offset_current = 2.0; offset_current < USE_RADIUS; offset_current += 1.0)
		{
			USE_OFFSET = OFFSET.xy * vec2(offset_current, offset_current);
    
			col1 = texture2D(u_DiffuseMap, var_TexCoords.xy + vec2(USE_OFFSET.x, 0.0));
			if (colCombined - (col1.r + col1.g + col1.b) > E_SMOOTH_THRESHOLD) { color /= num_passes; return color; }
			col2 = texture2D(u_DiffuseMap, var_TexCoords.xy - vec2(USE_OFFSET.x, 0.0));
			if (colCombined - (col2.r + col2.g + col2.b) > E_SMOOTH_THRESHOLD) { color /= num_passes; return color; }
			col3 = texture2D(u_DiffuseMap, var_TexCoords.xy + vec2(0.0, USE_OFFSET.y));
			if (colCombined - (col3.r + col3.g + col3.b) > E_SMOOTH_THRESHOLD) { color /= num_passes; return color; }
			col4 = texture2D(u_DiffuseMap, var_TexCoords.xy - vec2(0.0, USE_OFFSET.y));
			if (colCombined - (col4.r + col4.g + col4.b) > E_SMOOTH_THRESHOLD) { color /= num_passes; return color; }
			col5 = texture2D(u_DiffuseMap, var_TexCoords.xy + vec2(USE_OFFSET.x, 0.0-USE_OFFSET.y) * 0.666);
			if (colCombined - (col5.r + col5.g + col5.b) > E_SMOOTH_THRESHOLD) { color /= num_passes; return color; }
			col6 = texture2D(u_DiffuseMap, var_TexCoords.xy + vec2(0.0-USE_OFFSET.x, USE_OFFSET.y) * 0.666);
			if (colCombined - (col6.r + col6.g + col6.b) > E_SMOOTH_THRESHOLD) { color /= num_passes; return color; }
    
			// Looks like we are ok to smooth!
			color += ((col1 + col2 + col3 + col4 + col5 + col6) / 6.0);
			num_passes += 1.0;
		}
	}
  
	color /= num_passes;
	return color;
}

float	AdaptationMin = 0.0;
float	AdaptationMax = 0.1;//0.04;
float	PaletteIntensity = 1.05;
float	PaletteBrightness = 1.25;
float	Gamma = 1.38;
float	RedFilter = 1.0;
float	GreenFilter = 1.0;
float	BlueFilter = 1.0;
float	DesatR = 1.0;
float	DesatG = 0.10;
float	DesatB = 0.60;
float	IntensityContrast = 0.90;//1.15;//1.35;
float	Saturation = 1.68;//1.38;//1.68;
float	ToneMappingCurve = 2.0;
float	ToneMappingOversaturation = 30.0;
float	Brightness = 0.3;//0.4;
float	BrightnessCurve = 1.3;//1.8;
float	BrightnessMultiplier = 0.90;//0.6;
float	BrightnessToneMappingCurve = 0.50;


float	ECCGamma = 1.0;
float	ECCInBlack = 0.0;
float	ECCInWhite = 1.0;
float	ECCOutBlack = 0.0;
float	ECCOutWhite = 1.0;
float	ECCBrightness = 1.0;
float	ECCContrastGrayLevel = 0.5;
float	ECCContrast = 1.0;
float	ECCSaturation = 1.0;
float	ECCDesaturateShadows = 0.0;
vec3	ECCColorBalanceShadows = vec3(0.5, 0.5, 0.5);
vec3	ECCColorBalanceHighlights = vec3(0.5, 0.5, 0.5);
vec3	ECCChannelMixerR = vec3(1.0, 0.0, 0.0);
vec3	ECCChannelMixerG = vec3(0.0, 1.0, 0.0);
vec3	ECCChannelMixerB = vec3(0.0, 0.0, 1.0);

//#define E_CC_PROCEDURAL

vec4 PS_Adaptation() 
{
	vec4	color = texture2D(u_DiffuseMap, var_TexCoords.xy);

	//adaptation in time
	//vec4	Adaptation=texture2D(_s4, 0.5);
	//vec4	Adaptation=vec4(1.0) - texture2D(u_DiffuseMap, vec2(0.5,0.5));
	//float	grayadaptation=max(max(Adaptation.x, Adaptation.y), Adaptation.z);
	float	grayadaptation=1.0;

	float greyscale = dot(color.xyz, vec3(0.3, 0.59, 0.11));
    color.r = lerp(greyscale, color.r, DesatR);
    color.g = lerp(greyscale, color.g, DesatG);
    color.b = lerp(greyscale, color.b, DesatB);	
    	
	color = pow(color, vec4(Gamma));
	
	color.r = pow(color.r, RedFilter);
	color.g = pow(color.g, GreenFilter);
	color.b = pow(color.b, BlueFilter);
   
	grayadaptation=max(grayadaptation, 0.0); //0.0
	grayadaptation=min(grayadaptation, 50.0); //50.0
	color.xyz=color.xyz/(grayadaptation*AdaptationMax+AdaptationMin);//*tempF1.x

	color.xyz*=Brightness;
	color.xyz+=0.000001;
	vec3 xncol=normalize(color.xyz);
	vec3 scl=color.xyz/xncol.xyz;
	scl=pow(scl, float3(IntensityContrast));
	xncol.xyz=pow(xncol.xyz, float3(Saturation));
	color.xyz=scl*xncol.xyz;

	float	lumamax=ToneMappingOversaturation;
	color.xyz=(color.xyz * (1.0 + color.xyz/lumamax))/(color.xyz + ToneMappingCurve);
	
    float Y = dot(color.xyz, vec3(0.299, 0.587, 0.114)); //0.299 * R + 0.587 * G + 0.114 * B;
	float U = dot(color.xyz, vec3(-0.14713, -0.28886, 0.436)); //-0.14713 * R - 0.28886 * G + 0.436 * B;
	float V = dot(color.xyz, vec3(0.615, -0.51499, -0.10001)); //0.615 * R - 0.51499 * G - 0.10001 * B;	
	
	Y=pow(Y, BrightnessCurve);
	Y=Y*BrightnessMultiplier;
	Y=Y/(Y+BrightnessToneMappingCurve);
	float	desaturatefact=saturate(Y*Y*Y*1.7);
	U=lerp(U, 0.0, desaturatefact);
	V=lerp(V, 0.0, desaturatefact);
	color.xyz=V * vec3(1.13983, -0.58060, 0.0) + U * vec3(0.0, -0.39465, 2.03211) + Y;

#ifdef E_CC_PROCEDURAL
	float	tempgray;
	vec4	tempvar;
	vec3	tempcolor;

	//+++ levels like in photoshop, including gamma, lightness, additive brightness
	color=max(color-ECCInBlack, 0.0) / max(ECCInWhite-ECCInBlack, 0.0001);
	if (ECCGamma!=1.0) color=pow(color, ECCGamma);
	color=color*(ECCOutWhite-ECCOutBlack) + ECCOutBlack;

	//+++ brightness
	color=color*ECCBrightness;

	//+++ contrast
	color=(color-ECCContrastGrayLevel) * ECCContrast + ECCContrastGrayLevel;

	//+++ saturation
	tempgray=dot(color, 0.3333);
	color=lerp(tempgray, color, ECCSaturation);

	//+++ desaturate shadows
	tempgray=dot(color, 0.3333);
	tempvar.x=saturate(1.0-tempgray);
	tempvar.x*=tempvar.x;
	tempvar.x*=tempvar.x;
	color=lerp(color, tempgray, ECCDesaturateShadows*tempvar.x);

	//+++ color balance
	color=saturate(color);
	tempgray=dot(color, 0.3333);
	float2	shadow_highlight=float2(1.0-tempgray, tempgray);
	shadow_highlight*=shadow_highlight;
	color.rgb+=(ECCColorBalanceHighlights*2.0-1.0)*color * shadow_highlight.x;
	color.rgb+=(ECCColorBalanceShadows*2.0-1.0)*(1.0-color) * shadow_highlight.y;

	//+++ channel mixer
	tempcolor=color;
	color.r=dot(tempcolor, ECCChannelMixerR);
	color.g=dot(tempcolor, ECCChannelMixerG);
	color.b=dot(tempcolor, ECCChannelMixerB);
#endif //E_CC_PROCEDURAL

	return color;
}

vec4 PS_Reflection()
{
	float	px 			= 1.0 / var_Dimensions.x;
	float	py 			= 1.0f / var_Dimensions.y;
	vec2	OFFSET		= vec2(px, py);
  
	vec4	ColorInput = texture2D(u_DiffuseMap, var_TexCoords.xy);
	vec4	OrigColor = ColorInput;//vec4(0.6, 0.6, 1.0, 1.0);

	if (ColorInput.a <= 0.0)
	{// Reflective...
		/*
		float orig_alpha = 0.0-ColorInput.a;
		vec2 reflectCoords = var_TexCoords.xy;
		
		float pixelHeight = var_TexCoords.y;
		float heightPercent = 1.0 - (pixelHeight / 1.0);
		//float heightDiff = heightPercent;
		float heightDiff = pow(heightPercent, pixelHeight * 2.0);
		reflectCoords.y = heightDiff;

		ColorInput = (texture2D(u_DiffuseMap, reflectCoords.xy) + OrigColor + OrigColor) / 3.0;
		ColorInput.a = 1.0;//orig_alpha;
		*/

		vec2 reflectCoords = var_TexCoords.xy;

		for (float y = var_TexCoords.y; y < 1.0; y += (py*1.0))
		{
			float thisAlpha = texture2D(u_DiffuseMap, vec2(var_TexCoords.x,y)).a;
			
			if (thisAlpha >= 1.0)
			{// Found top of reflection area...
				float edgeLength = var_TexCoords.y - y;
				float reflectSpot = var_TexCoords.y - (edgeLength * 2.0);
				if (reflectSpot > 0.0 && reflectSpot < 1.0)
				{
					reflectCoords.y = reflectSpot;
				}

				break;
			}
		}

		//float distFromTop = 1.0 - reflectCoords.y;

		// Blur???
		//ColorInput.rgb = (texture2D(u_DiffuseMap, reflectCoords.xy).rgb + texture2D(u_DiffuseMap, reflectCoords.xy + OFFSET).rgb + texture2D(u_DiffuseMap, reflectCoords.xy - OFFSET).rgb + texture2D(u_DiffuseMap, reflectCoords.xy + vec2(px, 0.0)).rgb + texture2D(u_DiffuseMap, reflectCoords.xy + vec2(0.0, py)).rgb) / 5.0;
		ColorInput.rgb = texture2D(u_DiffuseMap, reflectCoords.xy).rgb;// * distFromTop;
		ColorInput.rgb += OrigColor.rgb + OrigColor.rgb;
		ColorInput.rgb /= 3.0;
		ColorInput.a = 1.0;
	}

	return ColorInput;
}

void main()
{
	vec4 color;
	/*
	if (CURRENT_PASS_NUMBER == 0) {
		//color = PS_ProcessGaussianH();
		color = texture2D(u_DiffuseMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 1) {
		//color = PS_ProcessGaussianV();
		color = texture2D(u_DiffuseMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 2) {
		//color = PS_ProcessEdges();
		color = texture2D(u_DiffuseMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 3) {
		//color = PS_ProcessSharpen1();
		color = texture2D(u_DiffuseMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 4) {
		//color = PS_ProcessSharpen2();
		color = texture2D(u_DiffuseMap, texCoord.xy);
	} else if (CURRENT_PASS_NUMBER == 5) {
		//color = PS_ProcessAfterFX();
		//color = texture2D(u_DiffuseMap, texCoord.xy);
		//color = PS_SuperEagle();
		//color = PS_EdgePreservingSmooth();
		//color = PS_Adaptation();
	}
	*/

	color = PS_Reflection();

	gl_FragColor.rgba = color.rgba;
	//gl_FragColor.a = 1.0;
}
