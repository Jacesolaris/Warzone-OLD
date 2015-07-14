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
	lightOutColor *= var_Local0.a; // distance mult - new
	//lightOutColor /= var_Local0.a; // num_lights - old
	//lightOutColor *= pow(1.0 - screenDistance, 3);
	vec3 add_color = origLightColor.rgb * lightOutColor;
#define const_1 ( 12.0 / 255.0)
#define const_2 (255.0 / 219.0)
	add_color = ((clamp(add_color - const_1, 0.0, 1.0)) * const_2);

	gl_FragColor = clamp(vec4(vec3(origColor.rgb + add_color), 1.0), 0.0, 1.0);
}
