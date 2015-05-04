//#version 330

uniform sampler2D u_TextureMap;
uniform sampler2D u_ScreenDepthMap;

varying vec2		var_TexCoords;
varying vec2		var_Dimensions;
varying vec4		var_ViewInfo; // znear, zfar, zfar / znear, 0

varying vec4		var_Local0; // dofValue, 0, 0, 0

//smooth vec2 texcoord;
vec2 texcoord = var_TexCoords;

#define PI			3.1415926535897932384626433832795

float width = var_Dimensions.x; //texture width
float height = var_Dimensions.y; //texture height

vec2 texel = vec2(1.0/width,1.0/height);

//uniform variables from external script
float focalDepth = -0.01581;
float focalLength = 12.0;
float fstop = 2.0;
bool showFocus = false;
//bool showFocus = true;

/* 
make sure that these two values are the same for your camera, otherwise distances will be wrong.
*/

//float znear = 0.1; //camera clipping start
//float zfar = 100.0; //camera clipping end
float znear = var_ViewInfo.x; //camera clipping start
float zfar = var_ViewInfo.y; //camera clipping end

//------------------------------------------
//user variables

int samples = 3; //samples on the first ring
int rings = 3; //ring count

bool manualdof = false; //manual dof calculation
//bool manualdof = true; //manual dof calculation

float ndofstart = 1.0; //near dof blur start
float ndofdist = 2.0; //near dof blur falloff distance
float fdofstart = 1.0; //far dof blur start
float fdofdist = 3.0; //far dof blur falloff distance

float CoC = 0.03;//circle of confusion size in mm (35mm film = 0.03mm)

vec2 focus = vec2(0.5,0.5); // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)
float maxblur = 1.0; //clamp value of max blur (0.0 = no blur,1.0 default)

bool blur_distant_only = false; // only blur distant objects, not the objects closer to the camera then the focus point.
bool blur_less_close = true; // blur objects close to the camera less.

bool constant_distant_blur = true; // Blur all distant objects.
float constant_distant_blur_depth = 0.01578; // UQ1: JKA Optimized value.
float constant_distant_blur_strength = -0.011; // UQ1: JKA Optimized value.

//float threshold = 0.7; //highlight threshold;
float threshold = 5.7; //highlight threshold;
float gain = 100.0; //highlight gain;

float bias = 0.5; //bokeh edge bias
float fringe = 0.7; //bokeh chromatic aberration/fringing

bool noise = false; //use noise instead of pattern for sample dithering
float namount = 0.0001; //dither amount

bool depthblur = false; //blur the depth buffer?
float dbsize = 1.25; //depthblursize

//------------------------------------------

float linearize(float depth)
{
	return -zfar * znear / (depth * (zfar - znear) - zfar);
}

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

/*==============================================================================================================
	DOF MODES
==============================================================================================================*/

#define USE_DYNAMIC_DOF				0	//0 for STATIC
#define USE_NOT_BLURRING_SKY			0
#define USE_TILT_SHIFT				0
#define USE_POLYGONAL_BOKEH			1

vec4 Timer = vec4(1.0, 1.0, 1.0, 1.0);

/*==============================================================================================================
	Polygonal Bokeh Shape - Global
	Works if USE_ENB_ADAPTIVE_QUALITY is disabled	
==============================================================================================================*/

#define POLYGON_NUM 6

/*==============================================================================================================
	Polygonal Bokeh Shape - DNI separated
	Works if USE_ENB_ADAPTIVE_QUALITY is enabled
==============================================================================================================*/

#define fPolygons_Day				5
#define fPolygons_Night				7
#define fPolygons_Interior			6

/*==============================================================================================================
	DOF QUALITY LEVEL
==============================================================================================================*/

#define	DEPTH_OF_FIELD_QUALITY 			1 	//From 1 to 7 			//4, 5, 7	

#define	USE_ENB_ADAPTIVE_QUALITY		1	//Works dynamically if enabled in enblocal.ini, otherwise enables alternative code

/*==============================================================================================================
	DYNAMIC DOF 
==============================================================================================================*/

#define fFocusPoint_Day				vec2(0.5, 0.5) 
#define fFocusSampleRange_Day			1.0
#define fNearBlurCurve_Day			14.55
#define fFarBlurCurve_Day			1.675					//1.25, 1.55	
#define fDepthClip_Day				150000.0 

#define fFocusPoint_Night			vec2(0.5, 0.5)
#define fFocusSampleRange_Night			1.0
#define fNearBlurCurve_Night			14.45 
#define fFarBlurCurve_Night			1.625			
#define fDepthClip_Night			150000.0 

#define fFocusPoint_Interior			vec2(0.5, 0.5)
#define fFocusSampleRange_Interior		1.0
#define fNearBlurCurve_Interior			10.00
#define fFarBlurCurve_Interior			0.85					//0.825, 0.725, 0.5
#define fDepthClip_Interior			150000.0  

/*==============================================================================================================
	STATIC DOF 
==============================================================================================================*/

#define	fFocalPlaneDepth_Day			0.00
#define	fFarBlurDepth_Day			20.00	

#define	fFocalPlaneDepth_Night			0.00
#define	fFarBlurDepth_Night			20.00	

#define	fFocalPlaneDepth_Interior		0.00
#define	fFarBlurDepth_Interior			20.00	

/*==============================================================================================================
	BOKEH Effect Parameters
==============================================================================================================*/

#define	fBokehBias_Day				-0.00125
#define	fBokehBiasCurve_Day			0.75
#define	fBokehBrightnessThreshold_Day		0.975 
#define	fBokehBrightnessMultiplier_Day		1.00
#define	fRadiusScaleMultiplier_Day		2.23

#define	fBokehBias_Night			0.00
#define	fBokehBiasCurve_Night			0.75
#define	fBokehBrightnessThreshold_Night		1.00
#define	fBokehBrightnessMultiplier_Night	1.00
#define	fRadiusScaleMultiplier_Night		2.23

#define	fBokehBias_Interior			0.00
#define	fBokehBiasCurve_Interior		0.75
#define	fBokehBrightnessThreshold_Interior	0.975
#define	fBokehBrightnessMultiplier_Interior	1.00
#define	fRadiusScaleMultiplier_Interior		2.23

/**
 *	If DYNAMIC VARS enabled, otherwise ignored
 */

#define fIntensityFactor_BokehBright_Day		0.0		
#define fTimeFactor_BokehBright_Day			300.0
#define fFrequency_BokehBright_Day			0.45	
#define fAmplitude_BokehBright_Day			0.45

#define fIntensityFactor_BokehBright_Night		5.0
#define fTimeFactor_BokehBright_Night			300.0
#define fFrequency_BokehBright_Night			0.475
#define fAmplitude_BokehBright_Night			0.475

#define fIntensityFactor_BokehBright_Interior		7.0
#define fTimeFactor_BokehBright_Interior		350.0
#define fFrequency_BokehBright_Interior			0.45
#define fAmplitude_BokehBright_Interior			0.45

#define fIntensityFactor_BokehThreshold_Day		0.0		
#define fTimeFactor_BokehThreshold_Day			100.0
#define fFrequency_BokehThreshold_Day			0.45	
#define fAmplitude_BokehThreshold_Day			0.45

#define fIntensityFactor_BokehThreshold_Night		5.0
#define fTimeFactor_BokehThreshold_Night		150.0
#define fFrequency_BokehThreshold_Night			0.5
#define fAmplitude_BokehThreshold_Night			0.5

#define fIntensityFactor_BokehThreshold_Interior	8.0
#define fTimeFactor_BokehThreshold_Interior		350.0
#define fFrequency_BokehThreshold_Interior		0.5
#define fAmplitude_BokehThreshold_Interior		0.5

#define fIntensityFactor_RadiusScale_Day		0.0				
#define fTimeFactor_RadiusScale_Day			50.0
#define fFrequency_RadiusScale_Day			0.35		
#define fAmplitude_RadiusScale_Day			0.35		

#define fIntensityFactor_RadiusScale_Night		4.0
#define fTimeFactor_RadiusScale_Night			100.0
#define fFrequency_RadiusScale_Night			0.425
#define fAmplitude_RadiusScale_Night			0.425

#define fIntensityFactor_RadiusScale_Interior		8.0
#define fTimeFactor_RadiusScale_Interior		250.0
#define fFrequency_RadiusScale_Interior			0.5
#define fAmplitude_RadiusScale_Interior			0.5

#define ENightDayFactor 0.5
#define EInteriorFactor 0.0
#define SEED			1				//Set ' 1 ' instead for no time-dependancy

void sincos(float x, float s, float c)
{
    s = sin(x);
    c = cos(x);
}

void main()
{
	vec4 res;
	vec2 coord=texcoord.xy;
	vec4 origcolor=texture2D(u_TextureMap, coord.xy);
	float centerDepth=linearize(texture2D(u_ScreenDepthMap,texcoord.xy).x * 255);//origcolor.w;
	vec2 pixelSize=texel;//ScreenSize.y;
	//pixelSize.y*=ScreenSize.z;
	
	float blurAmount=abs(centerDepth * 2.0 - 1.0);
	float discRadius=blurAmount * float(DEPTH_OF_FIELD_QUALITY);

	float fRadiusScaleMultiplier =lerp( lerp( fRadiusScaleMultiplier_Day, fRadiusScaleMultiplier_Night, 1 - ENightDayFactor ), fRadiusScaleMultiplier_Interior, EInteriorFactor);	
	float fNearBlurCurve =lerp( lerp( fNearBlurCurve_Day, fNearBlurCurve_Night, 1 - ENightDayFactor ), fNearBlurCurve_Interior, EInteriorFactor);
	float fBokehBias =lerp( lerp( fBokehBias_Day, fBokehBias_Night, 1 - ENightDayFactor ), fBokehBias_Interior, EInteriorFactor);
	float fBokehBrightnessThreshold =lerp( lerp( fBokehBrightnessThreshold_Day, fBokehBrightnessThreshold_Night, 1 - ENightDayFactor ), fBokehBrightnessThreshold_Interior, EInteriorFactor);
	float fBokehBrightnessMultiplier =lerp( lerp( fBokehBrightnessMultiplier_Day, fBokehBrightnessMultiplier_Night, 1 - ENightDayFactor ), fBokehBrightnessMultiplier_Interior, EInteriorFactor);
	float fBokehBiasCurve =lerp( lerp( fBokehBiasCurve_Day, fBokehBiasCurve_Night, 1 - ENightDayFactor ), fBokehBiasCurve_Interior, EInteriorFactor);

	float fIntensityFactor_BokehBright =lerp( lerp( fIntensityFactor_BokehBright_Day, fIntensityFactor_BokehBright_Night, 1 - ENightDayFactor ), fIntensityFactor_BokehBright_Interior, EInteriorFactor);
	float fFrequency_BokehBright =lerp( lerp( fFrequency_BokehBright_Day, fFrequency_BokehBright_Night, 1 - ENightDayFactor ), fFrequency_BokehBright_Interior, EInteriorFactor);
	float fAmplitude_BokehBright =lerp( lerp( fAmplitude_BokehBright_Day, fAmplitude_BokehBright_Night, 1 - ENightDayFactor ), fAmplitude_BokehBright_Interior, EInteriorFactor);
	float fTimeFactor_BokehBright =lerp( lerp( fTimeFactor_BokehBright_Day, fTimeFactor_BokehBright_Night, 1 - ENightDayFactor ), fTimeFactor_BokehBright_Interior, EInteriorFactor);
	float TimeFactorBokehBright =(0.5 * sin(2 * PI * fFrequency_BokehBright * Timer.x * (fIntensityFactor_BokehBright * fTimeFactor_BokehBright)) + 0.75) * fAmplitude_BokehBright;	//+ 0.5 Non-Limiter

	float fIntensityFactor_BokehThreshold =lerp( lerp( fIntensityFactor_BokehThreshold_Day, fIntensityFactor_BokehThreshold_Night, 1 - ENightDayFactor ), fIntensityFactor_BokehThreshold_Interior, EInteriorFactor);
	float fFrequency_BokehThreshold =lerp( lerp( fFrequency_BokehThreshold_Day, fFrequency_BokehThreshold_Night, 1 - ENightDayFactor ), fFrequency_BokehThreshold_Interior, EInteriorFactor);
	float fAmplitude_BokehThreshold =lerp( lerp( fAmplitude_BokehThreshold_Day, fAmplitude_BokehThreshold_Night, 1 - ENightDayFactor ), fAmplitude_BokehThreshold_Interior, EInteriorFactor);
	float fTimeFactor_BokehThreshold =lerp( lerp( fTimeFactor_BokehThreshold_Day, fTimeFactor_BokehThreshold_Night, 1 - ENightDayFactor ), fTimeFactor_BokehThreshold_Interior, EInteriorFactor);
	float TimeFactorBokehThreshold =(0.5 * sin(2 * PI * fFrequency_BokehThreshold * Timer.x * (fIntensityFactor_BokehThreshold * fTimeFactor_BokehThreshold)) + 0.75) * fAmplitude_BokehThreshold;	//+ 0.5 Non-Limiter

	float fIntensityFactor_RadiusScale =lerp( lerp( fIntensityFactor_RadiusScale_Day, fIntensityFactor_RadiusScale_Night, 1 - ENightDayFactor ), fIntensityFactor_RadiusScale_Interior, EInteriorFactor);
	float fFrequency_RadiusScale =lerp( lerp( fFrequency_RadiusScale_Day, fFrequency_RadiusScale_Night, 1 - ENightDayFactor ), fFrequency_RadiusScale_Interior, EInteriorFactor);
	float fAmplitude_RadiusScale =lerp( lerp( fAmplitude_RadiusScale_Day, fAmplitude_RadiusScale_Night, 1 - ENightDayFactor ), fAmplitude_RadiusScale_Interior, EInteriorFactor);
	float fTimeFactor_RadiusScale =lerp( lerp( fTimeFactor_RadiusScale_Day, fTimeFactor_RadiusScale_Night, 1 - ENightDayFactor ), fTimeFactor_RadiusScale_Interior, EInteriorFactor);
	float TimeFactorRadiusScale =(0.5 * sin(2 * PI * fFrequency_RadiusScale * Timer.x * (fIntensityFactor_RadiusScale * fTimeFactor_RadiusScale)) + 0.75) * fAmplitude_RadiusScale;	//+ 0.5 Non-Limiter

#if (USE_DYNAMIC_BOKEH_BRIGHT_PUSH == 1)
	fBokehBrightnessMultiplier=fBokehBrightnessMultiplier + TimeFactorBokehBright;
#endif
#if (USE_DYNAMIC_BOKEH_BRIGHT_PULL == 1)
	fBokehBrightnessMultiplier=fBokehBrightnessMultiplier - TimeFactorBokehBright;
#endif	
#if (USE_DYNAMIC_BOKEH_THRESH_PUSH == 1)
	fBokehBrightnessThreshold=fBokehBrightnessThreshold - TimeFactorBokehThreshold;
#endif
#if (USE_DYNAMIC_BOKEH_THRESH_PULL == 1)
	fBokehBrightnessThreshold=fBokehBrightnessThreshold + TimeFactorBokehThreshold;
#endif
#if (USE_DYNAMIC_RADIUS_SCALE_PUSH == 1)
	fRadiusScaleMultiplier=fRadiusScaleMultiplier + TimeFactorRadiusScale;
#endif
#if (USE_DYNAMIC_RADIUS_SCALE_PULL == 1)
	fRadiusScaleMultiplier=fRadiusScaleMultiplier - TimeFactorRadiusScale;
#endif

	discRadius*=fRadiusScaleMultiplier;
	
	#if (USE_DYNAMIC_DOF == 1)
	discRadius*=(centerDepth < 0.5) ? (1.0 / max(fNearBlurCurve, 1.0)) : 1.0;
	#endif
		
	res.xyz=origcolor.xyz;
	res.w=dot(res.xyz, vec3(0.3333));
	res.w=max((res.w - fBokehBrightnessThreshold) * fBokehBrightnessMultiplier, 0.0);
	res.xyz*=1.0 + res.w*blurAmount;
	
	res.w=1.0;
	
	int sampleCycle=0;
	int sampleCycleCounter=0;
	int sampleCounterInCycle=0;
	
	#if (USE_POLYGONAL_BOKEH == 1)
		float basedAngle=360.0 / POLYGON_NUM;
		vec2 currentVertex;
		vec2 nextVertex;
	
		float	dofTaps=DEPTH_OF_FIELD_QUALITY * (DEPTH_OF_FIELD_QUALITY + 1) * POLYGON_NUM / 2.0;
	#else
		float	dofTaps=DEPTH_OF_FIELD_QUALITY * (DEPTH_OF_FIELD_QUALITY + 1) * 4;
	#endif
		
	
	for(float i=0; i < dofTaps; i++)
	{
		if(sampleCounterInCycle % sampleCycle == 0) 
		{
			sampleCounterInCycle=0;
			sampleCycleCounter++;
		
			#if (USE_POLYGONAL_BOKEH == 1)
				sampleCycle+=POLYGON_NUM;
				currentVertex.xy=vec2(1.0, 0.0);
				sincos(basedAngle* 0.017453292, nextVertex.y, nextVertex.x);	
			#else	
				sampleCycle+=8;
			#endif
		}
		sampleCounterInCycle++;
		
		#if (USE_POLYGONAL_BOKEH == 1)
			float sampleAngle=basedAngle / float(sampleCycleCounter) * sampleCounterInCycle;
			float remainAngle=frac(sampleAngle / basedAngle) * basedAngle;
		
			if(remainAngle == 0)
			{
				currentVertex=nextVertex;
				sincos((sampleAngle +  basedAngle) * 0.017453292, nextVertex.y, nextVertex.x);
			}

			vec2 sampleOffset=lerp(currentVertex.xy, nextVertex.xy, remainAngle / basedAngle);
		#else
			float sampleAngle=0.78539816 / float(sampleCycleCounter) * sampleCounterInCycle;
			vec2 sampleOffset;
			sincos(sampleAngle, sampleOffset.y, sampleOffset.x);
		#endif
		
		sampleOffset*=sampleCycleCounter / float(DEPTH_OF_FIELD_QUALITY);
		vec2  coordLow=coord.xy + (pixelSize.xy * sampleOffset.xy * discRadius);
		vec4 tap=texture2D(u_TextureMap, coordLow.xy);
		
		float weight=(tap.w >= centerDepth) ? 1.0 : abs(tap.w * 2.0 - 1.0);
		
		float luma=dot(tap.xyz, vec3(0.3333));
		float brightMultiplier=max((luma - fBokehBrightnessThreshold) * fBokehBrightnessMultiplier, 0.0);
		tap.xyz*=1.0 + brightMultiplier*abs(tap.w*2.0 - 1.0);
		
		tap.xyz*=1.0 + fBokehBias * pow(float(sampleCycleCounter)/float(DEPTH_OF_FIELD_QUALITY), fBokehBiasCurve);
		
	    res.xyz+=tap.xyz * weight;
	    res.w+=weight;
	}

	res.xyz /= res.w;
		
	res.w=centerDepth;

	gl_FragColor.rgb = res.rgb;
	gl_FragColor.a = 1.0;
}

