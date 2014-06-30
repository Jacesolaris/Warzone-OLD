uniform sampler2D	u_DiffuseMap;

float exposure =	0.0034;
//float exposure =	0.1;
//float decay =		1.0;
float decay =		0.95;
float density =		0.84;
float weight =		5.65;

varying vec2		var_Dimensions;
varying vec2		var_LightScreenPos;
varying vec2		var_TexCoords;
varying vec4		var_Local0; // lightOrg
varying vec4		var_Local1; // lightColor
varying vec4		var_LightOrg;

const int NUM_SAMPLES = 100;

void main()
{
	vec4 lightOrg = var_LightOrg;
	lightOrg.xyz = lightOrg.xyz / lightOrg.w * 0.5 + 0.5;

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
        gl_FragColor += (sample /** var_Local1.rgba*/);
        illuminationDecay *= decay;
    }
    gl_FragColor *= exposure;
}
