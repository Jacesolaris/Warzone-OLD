uniform sampler2D	u_DiffuseMap; // screen
uniform sampler2D	u_NormalMap; // glowmap
uniform sampler2D	u_ScreenDepthMap; // depthmap

varying vec2		var_Dimensions;
varying vec4		var_ViewInfo; // znear, zfar, zfar / znear, 0
varying vec2		var_LightScreenPos;
varying vec2		var_TexCoords;
varying vec4		var_Local0; // lightOrg
varying vec4		var_Local1; // lightColor
varying vec4		var_Local2; // lightScreenPos (x,y), testvar
varying vec4		var_Local3; // exposure, decay, density, weight
varying vec4		var_LightOrg;

/*
//float exposure =	0.0034;
//float exposure =	0.01;
float exposure =	0.1;
//float decay =		1.0;
float decay =		0.95;
float density =		0.84;
//float weight =		5.65;
float weight =		1.65;
*/

float exposure	=	var_Local3.r;
float decay		=	var_Local3.g;
float density	=	var_Local3.b;
float weight	=	var_Local3.a;

float znear = var_ViewInfo.x; //camera clipping start
float zfar = var_ViewInfo.y; //camera clipping end

const int NUM_SAMPLES = 100;

void main()
{
	vec2 texCoord = var_TexCoords; 
	vec4 origColor = texture2D(u_DiffuseMap, var_TexCoords);
	vec4 origLightColor = texture2D(u_DiffuseMap, var_LightScreenPos.xy);
	float origMult = clamp(1.0 - clamp(length(origColor.rgb) / 3.0, 0.0, 1.0), 0.0, 1.0) * 0.5; // work out a multiplier to even out brightnesses of darker and lighter colors
	//origLightColor.rgb *= origMult;

    vec2 deltaTextCoord = vec2( texCoord - var_LightScreenPos.xy );  
    deltaTextCoord *= 1.0 / float(NUM_SAMPLES) * density;  

    float illuminationDecay = 1.0;

    //vec4 tmpColor = vec4(0.0,0.0,0.0,0.0); //gl_FragColor;  
	vec4 tmpShadow = vec4(0.0,0.0,0.0,0.0); //gl_FragColor;
	//vec4 tmpLight = vec4(0.0,0.0,0.0,0.0); //gl_FragColor;

    for (int i = 0; i < NUM_SAMPLES; i++) {  
        texCoord -= deltaTextCoord;  

		if (length(texCoord - var_LightScreenPos.xy) > 0.3)
		{// UQ1: This increases FPS a little and only samples the color near the light source...
			illuminationDecay *= decay;
			continue;
		}

		//vec4 sample = vec4(texture2D(u_ScreenDepthMap,texCoord).rgb * 0.1397,1.0);
		//sample *= illuminationDecay * weight;  
        //tmpColor += sample * vec4(var_Local1.rgb, 1.0);  

		vec4 shadowSample = texture2D(u_DiffuseMap, texCoord);
		shadowSample *= illuminationDecay * weight;
        tmpShadow += shadowSample * origLightColor;
		//tmpLight += shadowSample * ((NUM_SAMPLES-i)/NUM_SAMPLES);

        illuminationDecay *= decay;  
    }

	//tmpColor *= exposure;
	tmpShadow *= exposure;
	//tmpLight *= exposure;

	//vec4 lightOutColor = vec4(origColor.rgb + clamp(tmpColor.rgb * var_Local0.a * 0.5, 0.0, 1.0), 1.0);
	vec4 shadowOutColor = vec4(origColor.rgb + clamp(tmpShadow.rgb * var_Local0.a * origMult, 0.0, 1.0), 1.0);
	//vec4 lightOutColor = vec4(origColor.rgb + clamp(tmpLight.rgb * var_Local0.a, 0.0, 1.0), 1.0);

    //gl_FragColor = clamp((origColor + lightOutColor + shadowOutColor) / 3.0, 0.0, 1.0);
	gl_FragColor = clamp((origColor + shadowOutColor) / 2.0, 0.0, 1.0);
	//gl_FragColor = clamp((origColor + shadowOutColor + lightOutColor) / 3.0, 0.0, 1.0);
}

