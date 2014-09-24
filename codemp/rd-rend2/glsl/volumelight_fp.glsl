uniform sampler2D	u_DiffuseMap; // screen
uniform sampler2D	u_NormalMap; // glowmap
uniform sampler2D	u_ScreenDepthMap; // depthmap

//float exposure =	0.0034;
float exposure =	0.01;
//float exposure =	0.1;
//float decay =		1.0;
float decay =		0.95;
float density =		0.84;
//float weight =		5.65;
float weight =		1.65;

varying vec2		var_Dimensions;
varying vec2		var_LightScreenPos;
varying vec2		var_TexCoords;
varying vec4		var_Local0; // lightOrg
varying vec4		var_Local1; // lightColor
varying vec4		var_LightOrg;

const int NUM_SAMPLES = 100;

/*
// UQ1: This is gonna be really slow!!! :( If only modelMatrix worked...

vec2 FindLight()
{
	bool found = false;
	vec2 offset = vec2(1.0 / var_Dimensions.x, 1.0 / var_Dimensions.y);
	offset *= 20.0; // 10 px at a time...
	
	vec2 pos = vec2(0.0);

	while (pos.y < 1.0)
	{
		vec4 glow = texture2D(u_NormalMap, pos);

		if (glow.r > 0.0 || glow.g > 0.0 || glow.b > 0.0)
		{
			pos.y = 1.0 - pos.y; // wtf??? y axis inverted?
			return pos;
		}
		else
		{
			pos.x += offset.x;

			if (pos.x >= 1.0)
			{
				pos.y += offset.y;
				
				if (pos.y < 1.0)
				{
					pos.x = 0.0;
				}
			}
		}
	}

	return vec2(1.0);
}

void main()
{
	// 1. get light positions from glowmap and store in array...
	// 2. for each light position run below...
	//gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);

	//gl_FragColor = texture2D(u_NormalMap, var_TexCoords);
	//	return;
	
	vec2 lightPos = FindLight();
	
	if (lightPos.x >= 1.0 && lightPos.y >= 1.0)
	{
		// No lights...
		gl_FragColor = texture2D(u_DiffuseMap, var_TexCoords);
		return;
	}
	else
	{
		vec2 texCoord = var_TexCoords;
		vec2 deltaTextCoord = vec2( texCoord - lightPos.xy );
		deltaTextCoord *= 1.0 / float(NUM_SAMPLES) * density;
		float illuminationDecay = 1.0;

		
		if (length(texCoord - lightPos.xy) < 0.03)
		{
			gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
			return;
		}
		

		gl_FragColor = texture2D(u_DiffuseMap, texCoord);
    
		for (int i = 0; i < NUM_SAMPLES; i++) {
			texCoord -= deltaTextCoord;
			vec4 sample = texture2D(u_DiffuseMap, texCoord);
			sample *= illuminationDecay * weight;
			gl_FragColor += (sample);// * var_Local1.rgba);
			illuminationDecay *= decay;
		}
		gl_FragColor *= exposure;
	}
}
*/


void main()
{
	//vec4 lightOrg = var_LightOrg;
	//lightOrg.xyz = lightOrg.xyz / lightOrg.w * 0.5 + 0.5;
	vec4 lightOrg = var_LightOrg;
	lightOrg.xy = vec2(var_LightScreenPos);

    vec2 texCoord = var_TexCoords;
    vec2 deltaTextCoord = vec2( texCoord - lightOrg.xy );
    deltaTextCoord *= 1.0 / float(NUM_SAMPLES) * density;
    float illuminationDecay = 1.0;

	if (length(texCoord - lightOrg.xy) < 0.1)
	{
		gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
		return;
	}

	gl_FragColor = texture2D(u_DiffuseMap, texCoord);
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        texCoord -= deltaTextCoord;
        vec4 sample = texture2D(u_DiffuseMap, texCoord);
        sample *= illuminationDecay * weight;
        gl_FragColor += (sample); //* var_Local1.rgba);
        illuminationDecay *= decay;
    }
    gl_FragColor *= exposure;
}

