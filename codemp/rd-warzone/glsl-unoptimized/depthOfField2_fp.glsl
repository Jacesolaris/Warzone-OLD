//#version 120

uniform sampler2D u_TextureMap;
uniform sampler2D u_ScreenDepthMap;

varying vec2		var_TexCoords;
varying vec2		var_Dimensions;
//varying vec4		var_ViewInfo; // znear, zfar, zfar / znear, 0

varying vec4		var_Local0; // dofValue, 0, 0, 0

//float DOF_MANUALFOCUSDEPTH = var_Local0.y;

vec2 sampleOffset = vec2(1.0/var_Dimensions);

#define PIOVER180 0.017453292

//MATSO DOF
#define bMatsoDOFChromaEnable			// Enables Chromatic Abberation.
#define bMatsoDOFBokehEnable			// Enables Bokeh weighting do define bright light spots and increase bokeh shape definiton.	
#define fMatsoDOFChromaPow		2.4//	//[0.2 to 3.0] Amount of chromatic abberation color shifting.
#define fMatsoDOFBokehCurve		6.0//7//2.0	//[0.5 to 20.0] Bokeh curve.
#define fMatsoDOFBokehLight		1.0//0.512 	//[0.0 to 2.0] Bokeh brightening factor.
#define iMatsoDOFBokehQuality		5	//[1 to 10] Blur quality as control value over tap count.
#define fMatsoDOFBokehAngle		10	//[0 to 360] Rotation angle of bokeh shape.

#define DOF_FOCUSPOINT	 		vec2(0.5,0.5)	//[0.0 to 1.0] Screen coordinates of focus point. First value is horizontal, second value is vertical position.
//#define DOF_BLURRADIUS 			10.0	//[5.0 to 50.0] Blur radius approximately in pixels. Radius, not diameter.
#define DOF_BLURRADIUS 			5
//#define DOF_MANUALFOCUSDEPTH 		var_Local0.y//0.2	//[0.0 to 1.0] Manual focus depth. 0.0 means camera is focus plane, 1.0 means sky is focus plane.
#define DOF_MANUALFOCUSDEPTH 		253.0	//[0.0 to 1.0] Manual focus depth. 0.0 means camera is focus plane, 1.0 means sky is focus plane.

float GetLinearDepth(float depth)
{
	//return  1 / ((depth * ((var_ViewInfo.g - var_ViewInfo.r) / (-var_ViewInfo.g * var_ViewInfo.r)) + var_ViewInfo.g / (var_ViewInfo.g * var_ViewInfo.r)));
	return depth * 255.0;
}

float GetFocalDepth(vec2 focalpoint)
{ 
	if (var_Local0.r < 4.0)
		return DOF_MANUALFOCUSDEPTH;

#if 1
 	float depthsum = 0.0;
 	//float fcRadius = var_Local0.y;//300.00;
	
#if 0
	for(int r = 0; r < 6; r++)
	{ 
		highp float t = float(r);
 		t *= 3.1415*2/6; 
 		vec2 coord = vec2(cos(t),sin(t)); 
 		coord.y *= sampleOffset.y * fcRadius; 
 		//coord *= fcRadius; 
 		float depth = GetLinearDepth(texture2D(u_ScreenDepthMap,coord+focalpoint).x) * 0.999; 
 		depthsum+=depth; 
 	}

	depthsum = depthsum/6;
#else
	depthsum+=GetLinearDepth(texture2D(u_ScreenDepthMap,focalpoint).x) * 0.999;
#ifdef BLUR_FOCUS
	depthsum+=GetLinearDepth(texture2D(u_ScreenDepthMap,focalpoint+vec2(0.0, 0.05)).x) * 0.999;
	depthsum+=GetLinearDepth(texture2D(u_ScreenDepthMap,focalpoint+vec2(0.0, 0.1)).x) * 0.999;
	depthsum+=GetLinearDepth(texture2D(u_ScreenDepthMap,focalpoint+vec2(0.0, 0.15)).x) * 0.999;
	depthsum+=GetLinearDepth(texture2D(u_ScreenDepthMap,focalpoint+vec2(0.0, 0.2)).x) * 0.999;
	depthsum+=GetLinearDepth(texture2D(u_ScreenDepthMap,focalpoint+vec2(0.0, 0.25)).x) * 0.999;
	depthsum+=GetLinearDepth(texture2D(u_ScreenDepthMap,focalpoint+vec2(0.0, 0.3)).x) * 0.999;
	depthsum+=GetLinearDepth(texture2D(u_ScreenDepthMap,focalpoint+vec2(0.0, 0.35)).x) * 0.999;
	depthsum+=GetLinearDepth(texture2D(u_ScreenDepthMap,focalpoint+vec2(0.0, 0.4)).x) * 0.999;
	depthsum+=GetLinearDepth(texture2D(u_ScreenDepthMap,focalpoint+vec2(0.0, 0.45)).x) * 0.999;

	depthsum = depthsum/10;
#endif //BLUR_FOCUS
#endif

	return depthsum; 
#else
	return GetLinearDepth(texture2D(u_ScreenDepthMap,DOF_FOCUSPOINT).x) * 0.999;
#endif
}

float CircleOfConfusion(float t)
{
	return max(t * .04, (2.0 / var_Dimensions.y) * (1.0+t));
}

vec4 GetMatsoDOFCA(sampler2D col, vec2 tex, float CoC)
{
	vec3 chroma;
	chroma.r = pow(0.5, fMatsoDOFChromaPow * CoC);
	chroma.g = pow(1.0, fMatsoDOFChromaPow * CoC);
	chroma.b = pow(1.5, fMatsoDOFChromaPow * CoC);

	vec2 tr = ((2.0 * tex - 1.0) * chroma.r) * 0.5 + 0.5;
	vec2 tg = ((2.0 * tex - 1.0) * chroma.g) * 0.5 + 0.5;
	vec2 tb = ((2.0 * tex - 1.0) * chroma.b) * 0.5 + 0.5;
	
	vec3 color = vec3(texture2D(col, tr).r, texture2D(col, tg).g, texture2D(col, tb).b) * (1.0 - CoC);
	
	return vec4(color, 1.0);
}

vec4 GetMatsoDOFBlur(int axis, vec2 coord, sampler2D SamplerHDRX)
{
	vec4 res;
	vec4 tcol = texture2D(SamplerHDRX, coord.xy);
	
	//float origCoC = CircleOfConfusion((tcol.r + tcol.g + tcol.b) / 3.0);
	//vec2 discRadius = origCoC.x*((GetLinearDepth(texture2D(u_ScreenDepthMap, coord.xy).x) - GetFocalDepth(coord.xy))*DOF_BLURRADIUS) * sampleOffset.xy*0.5/iMatsoDOFBokehQuality;
	float coordDepth = GetLinearDepth(texture2D(u_ScreenDepthMap, coord.xy).x);
	float focalDepth = GetFocalDepth(DOF_FOCUSPOINT/*coord.xy*/);
	float depthDiff = (coordDepth - focalDepth);
	vec2 discRadius = /*origCoC **/ (depthDiff * float(DOF_BLURRADIUS)) * sampleOffset.xy * 0.5 / float(iMatsoDOFBokehQuality);
	
	if (depthDiff < 0.0) // Close to camera pixels, blur much less so player model is not blury...
		discRadius *= 0.06;//var_Local0.z;

	//if (var_Local0.r < 4.0) // Manual mode. Blur less...
		discRadius *= 0.5;//0.333;

	int passnumber=1;

	float sf = 0.0;

	const vec2 tdirs[4] = vec2[4]( vec2(-0.306, 0.739), vec2(0.306, 0.739), vec2(-0.739, 0.306), vec2(-0.739, -0.306) );

#ifdef bMatsoDOFBokehEnable
	float wValue = (1.0 + pow(length(tcol.rgb) + 0.1, fMatsoDOFBokehCurve)) * (1.0 - fMatsoDOFBokehLight);	// special recipe from papa Matso ;)
#else
	float wValue = 1.0;
#endif

	for (int i = -iMatsoDOFBokehQuality; i < iMatsoDOFBokehQuality; i++)
	{
		vec2 taxis =  tdirs[axis];

		taxis.x = cos(fMatsoDOFBokehAngle*PIOVER180)*taxis.x-sin(fMatsoDOFBokehAngle*PIOVER180)*taxis.y;
		taxis.y = sin(fMatsoDOFBokehAngle*PIOVER180)*taxis.x+cos(fMatsoDOFBokehAngle*PIOVER180)*taxis.y;
		
		float fi = float(i);
		vec2 tdir = taxis * fi * discRadius;
		vec2 tcoord = coord.xy + tdir.xy;

#ifdef bMatsoDOFChromaEnable
		vec4 ct = GetMatsoDOFCA(SamplerHDRX, tcoord.xy, discRadius.x);
#else
		vec4 ct = texture2D(SamplerHDRX, tcoord.xy);
#endif

#ifndef bMatsoDOFBokehEnable
		float w = 1.0 + abs(offset[i]);	// weight blur for better effect
#else	
		// my own pseudo-bokeh weighting
		float b = dot(length(ct.rgb),0.333) + length(ct.rgb) + 0.1;
		float w = pow(b, fMatsoDOFBokehCurve) + abs(fi);
#endif
		tcol += ct * w;
		wValue += w;
	}

	tcol /= wValue;

	res.xyz = tcol.xyz;

	res.w = 1.0;
	return res;
}

void main ()
{
	int axis = int(var_Local0.a);
	gl_FragColor = GetMatsoDOFBlur(axis, var_TexCoords, u_TextureMap);
}