uniform sampler2D	u_DiffuseMap; // screen
uniform sampler2D	u_NormalMap; // glowmap
uniform sampler2D	u_ScreenDepthMap; // depthmap

varying vec2		var_Dimensions;
varying vec4		var_ViewInfo; // znear, zfar, zfar / znear, 0
varying vec2		var_LightScreenPos;
varying vec2		var_TexCoords;
varying vec4		var_Local0; // lightOrg, num_lights_on_screen
varying vec4		var_Local1; // lightColor
varying vec4		var_Local2; // lightScreenPos (x,y), testvar, volumeSamples
varying vec4		var_Local3; // exposure, decay, density, weight
varying vec4		var_LightOrg;

#define __OLD__

#ifdef __OLD__
float exposure	=	var_Local3.r;
float decay		=	var_Local3.g;
float density	=	var_Local3.b;
float weight	=	var_Local3.a;

float znear = var_ViewInfo.x; //camera clipping start
float zfar = var_ViewInfo.y; //camera clipping end

float NUM_SAMPLES = var_Local2.a;
//const int NUM_SAMPLES = 100;
//const int NUM_SAMPLES = 50;
//const int NUM_SAMPLES = 20;

float OcclusionFromDepth(vec2 pos)
{
	int closer = 0;
	//float lightDepthSample = texture2D(u_ScreenDepthMap, var_LightScreenPos).x;
	float depthSample = texture2D(u_ScreenDepthMap, pos).x;
	//if (lightDepthSample*0.7 > depthSample) closer = 1;
	depthSample = pow(depthSample, 256/*2048*/);
	if (depthSample > 0.98) depthSample = 0.0;
	//if (closer != 1)
	//{
		//float distance = length(var_TexCoords - var_LightScreenPos);
		//if (distance < 0.1) depthSample += (((0.1-distance) / 0.1) * 0.25);
	//}
	depthSample *= 256.0;
	depthSample = clamp(depthSample, 0.0, 1.0);
	return depthSample;
}

void main()
{
	//float screenDistance = length(var_TexCoords - var_LightScreenPos.xy);
	vec2 texCoord = var_TexCoords; 
	vec4 origColor = texture2D(u_DiffuseMap, var_TexCoords);

	/*if (screenDistance > 0.5)
	{// Pixel is too far away from light... Skip...
		gl_FragColor = origColor;
		return;
	}*/

	vec4 origLightColor = var_Local1;
	//vec4 origLightColor = texture2D(u_DiffuseMap, var_LightScreenPos.xy);
	//float origMult = clamp(1.0 - clamp(length(origColor.rgb) / 3.0, 0.0, 1.0), 0.0, 1.0) * 0.5; // work out a multiplier to even out brightnesses of darker and lighter colors

	float brightness = clamp(length(origColor.rgb) / 3.0, 0.0, 1.0);
	//float origMult = clamp(1.0 - brightness, 0.0, 1.0);

	// Equal out glow brightnesses...
	/*if (brightness > 2.0) {
		//origMult *= 1.0;
	} else if (brightness > 1.0) {
		origMult *= 2.0;
	} else {
		origMult *= 3.0;
	}*/

    vec2 deltaTextCoord = vec2( texCoord - var_LightScreenPos.xy );  
    deltaTextCoord *= 1.0 / float(NUM_SAMPLES) * density;  

    float illuminationDecay = 1.0;

	float tmpShadow = 0.0;

    for (float i = 0.0; i < NUM_SAMPLES; i+=1.0) {  
        texCoord -= deltaTextCoord;  

		/*
		if (length(texCoord - var_LightScreenPos.xy) > 0.5)
		{// UQ1: This increases FPS a little and only samples the color near the light source...
			illuminationDecay *= decay;
			continue;
		}
		*/
		

		float shadowSample = OcclusionFromDepth(texCoord);
		shadowSample *= illuminationDecay * weight;
        tmpShadow += shadowSample;
        illuminationDecay *= decay;  
    }

	tmpShadow *= exposure;
	
	float lightOutColor = tmpShadow * (3.0 - brightness);
	//lightOutColor /= (var_Local0.a);
	//lightOutColor *= pow(1.0 - screenDistance, 3);
	vec3 add_color = origLightColor.rgb * lightOutColor;
#define const_1 ( 12.0 / 255.0)
#define const_2 (255.0 / 219.0)
	add_color = ((clamp(add_color - const_1, 0.0, 1.0)) * const_2);

	gl_FragColor = clamp(vec4(vec3(origColor.rgb + add_color), 1.0), 0.0, 1.0);
}

#else //!__OLD__

//GODRAYS
#define bGodrayDepthCheck			1		//[0 or 1] if 1, only pixels with depth = 1 get godrays, this prevents white objects from getting godray source which would normally happen in LDR
#define iGodraySamples 				128		//[2^x format] How many samples the godrays get
#define fGodrayDecay   				0.99  	//[0.5 to 0.9999] How fast they decay. It's logarithmic, 1.0 means infinite long rays which will cover whole screen
#define fGodrayExposure				1.0		//[0.7 to 1.5] Upscales the godray's brightness
#define fGodrayWeight				1.25	//[0.8 to 1.7] weighting
#define fGodrayDensity				1.0		//[0.2 to 2.0] Density of rays, higher means more and brighter rays
//#define fGodrayThreshold			0.9    	//[0.6 to 1.0] Minimum brightness an object must have to cast godrays
#define fGodrayThreshold			0.0    	//[0.6 to 1.0] Minimum brightness an object must have to cast godrays

float depthMult = 255.0;

void main()
{
	vec4 origColor = texture2D(u_DiffuseMap, var_TexCoords.xy);
	vec4 lens = vec4(0.0, 0.0, 0.0, 0.0);
	vec2 ScreenLightPos = vec2(var_LightScreenPos.xy);
	vec2 texCoord = vec2(var_TexCoords.xy);
	vec2 deltaTexCoord = vec2(texCoord.xy - ScreenLightPos.xy);
	deltaTexCoord *= 1.0 / iGodraySamples * fGodrayDensity;

	float illuminationDecay = 1.0;

	for(int g = 0; g < iGodraySamples; g++) {
		texCoord -= deltaTexCoord;
		vec4 sample2 = texture2D(u_DiffuseMap, texCoord.xy);
		vec3 dt = vec3(dot(sample2.xyz, vec3(0.3333)));
		vec3 thresh = vec3(fGodrayThreshold);
		sample2.w = clamp(dt - thresh, 0.0, 1.0);
		sample2.r *= 1.0;
		sample2.g *= 0.95;
		sample2.b *= 0.85;
		sample2 *= illuminationDecay * fGodrayWeight * origColor;
	//#if (bGodrayDepthCheck == 1)
		float sampledepth = texture2D(u_ScreenDepthMap, texCoord.xy).x * depthMult;
		if(sampledepth>0.99999) lens.xyz += sample2.xyz*sample2.w;
	//#else
	//	lens.xyz += sample2;
	//#endif
		illuminationDecay *= fGodrayDecay;
	}

	//gl_FragColor = lens;
	gl_FragColor = clamp((origColor + origColor + lens) / 3.0, 0.0, 1.0);
}

#endif //!__OLD__

