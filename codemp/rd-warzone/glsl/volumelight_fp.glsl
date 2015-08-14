uniform sampler2D	u_DiffuseMap; // screen
uniform sampler2D	u_NormalMap; // glowmap
uniform sampler2D	u_ScreenDepthMap; // depthmap

varying vec2		var_Dimensions;
varying vec2		var_LightScreenPos;
varying vec2		var_TexCoords;
varying vec4		var_Local0; // lightOrg, num_lights_on_screen
varying vec4		var_Local1; // lightColorR, lightColorG, lightColorB, volumelightShadowEnhancement
varying vec4		var_Local2; // lightScreenPos (x,y), testvar, volumeSamples
varying vec4		var_Local3; // exposure, decay, density, weight
varying vec4		var_LightOrg;

// UQ1: Sigh... This would run much faster if I made these all constants instead...
float exposure	=	var_Local3.r;
float decay		=	var_Local3.g;
float density	=	var_Local3.b;
float weight	=	var_Local3.a;

float NUM_SAMPLES = var_Local2.a;
//const int NUM_SAMPLES = 100;
//const int NUM_SAMPLES = 50;
//const int NUM_SAMPLES = 20;

float OcclusionFromDepth(vec2 pos)
{
	int closer = 0;
	float depthSample = texture2D(u_ScreenDepthMap, pos).x;
	depthSample = pow(depthSample, 256/*2048*/);
	if (depthSample > 0.98) depthSample = 0.0;
	depthSample *= 256.0;
	depthSample = clamp(depthSample, 0.0, 1.0);
	return depthSample;
}

void main()
{
	vec2 texCoord = var_TexCoords; 
	vec4 origColor = texture2D(u_DiffuseMap, var_TexCoords);

	vec4 origLightColor = var_Local1;

    vec2 deltaTextCoord = vec2( texCoord - var_LightScreenPos.xy );  
    deltaTextCoord *= 1.0 / float(NUM_SAMPLES) * density;  

    float illuminationDecay = 1.0;

	float tmpShadow = 0.0;

    for (float i = 0.0; i < NUM_SAMPLES; i+=1.0) {  
        texCoord -= deltaTextCoord;  
		float shadowSample = OcclusionFromDepth(texCoord);
		shadowSample *= illuminationDecay * weight;
        tmpShadow += shadowSample;
        illuminationDecay *= decay;  
    }

	tmpShadow *= exposure;
	
	float lightOutColor = tmpShadow;// * (3.0 - brightness);

	float bt = lightOutColor;

	if (bt > 0.666) 
		bt *= 0.444; // Bright lights get dulled... (eg: white)
	else if (bt < 0.333) 
		bt *= 2.55; // Dull lights get amplified... (eg: blue)
	else 
		bt *= 1.35; // Mid range lights get amplified slightly... (eg: yellow)

	lightOutColor = clamp(lightOutColor * bt, 0.0, 1.0);

	lightOutColor *= var_Local0.a; // distance mult - new

	vec3 add_color = origLightColor.rgb * lightOutColor;
#define const_1 ( 12.0 / 255.0)
#define const_2 (255.0 / 119.0)
	add_color = ((clamp(add_color - const_1, 0.0, 1.0)) * const_2);

	gl_FragColor = clamp(vec4(vec3(origColor.rgb + add_color), 1.0), 0.0, 1.0);

	float origStrength = length(origColor.rgb) / 3.0;
	float newStrength = length(gl_FragColor.rgb) / 3.0;

	if (origStrength >= newStrength * var_Local1.a)
	{// Enhance shadows...
		gl_FragColor.rgb *= var_Local1.a;
	}
}
