uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;

uniform vec2		u_Dimensions;

varying vec2		var_ScreenTex;

// Enable output Debugging...
//#define DEBUG

// Shall we blur the result?
//#define BLUR_WIDTH 1.0

// Shall we pixelize randomly the output? -- Sucks!
//#define RANDOMIZE_PIXELS

#ifdef RANDOMIZE_PIXELS
//noise producing function to eliminate banding (got it from someone else´s shader):
float rand(vec2 co){
	return 0.5+(fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453))*0.5;
}
#endif

void main()
{
	vec4 color = texture2D(u_DiffuseMap, var_ScreenTex);
#ifdef RANDOMIZE_PIXELS
	float random = rand(var_ScreenTex);
#endif

#if defined(BLUR_WIDTH)
	vec2 offset = vec2(1.0) / u_Dimensions;

	vec3 hbao = texture2D(u_NormalMap, var_ScreenTex).rgb;
	
	for (float i = 1.0; i < BLUR_WIDTH+1.0; i+=1.0)
	{
		hbao += texture2D(u_NormalMap, var_ScreenTex + (offset * vec2(i, 0.0))).rgb;
		hbao += texture2D(u_NormalMap, var_ScreenTex + (offset * vec2(0.0, i))).rgb;
		hbao += texture2D(u_NormalMap, var_ScreenTex + (offset * vec2(i, i))).rgb;
		hbao += texture2D(u_NormalMap, var_ScreenTex + (offset * vec2(-i, 0.0))).rgb;
		hbao += texture2D(u_NormalMap, var_ScreenTex + (offset * vec2(0.0, -i))).rgb;
		hbao += texture2D(u_NormalMap, var_ScreenTex + (offset * vec2(-i, -i))).rgb;
	}

	hbao /= ((6.0 * (BLUR_WIDTH+1.0)) + 1.0);
#else
	vec3 hbao = texture2D(u_NormalMap, var_ScreenTex).rgb;
#endif

#ifdef DEBUG
	gl_FragColor.rgb = hbao;
	gl_FragColor.a = 1.0;
	return;
#endif

	gl_FragColor = color;

//#define const_1 ( 12.0 / 255.0)
//#define const_2 (255.0 / 219.0)
//	hbao.rgb = ((clamp(hbao.rgb - const_1, 0.0, 1.0)) * const_2);

#ifdef RANDOMIZE_PIXELS
	gl_FragColor.rgb *= vec3(1.0)-((vec3(1.0)-hbao)*random);
#else
	//gl_FragColor.rgb *= hbao;
	gl_FragColor.rgb = (gl_FragColor.rgb + (hbao * gl_FragColor.rgb)) / 2.0; // UQ1: Blending to reduce pixelation...
#endif	
}
