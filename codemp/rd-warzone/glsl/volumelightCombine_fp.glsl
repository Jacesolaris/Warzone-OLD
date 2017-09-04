uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;

uniform vec2		u_Dimensions;

uniform vec4		u_Local1;

varying vec2		var_ScreenTex;

// Enable output Debugging...
//#define DEBUG

// Shall we blur the result?
////#define BLUR_WIDTH 3.0
//#define BLUR_WIDTH 2.0
#define BLUR_WIDTH 1.0

// Shall we pixelize randomly the output? -- Sucks!
#define RANDOMIZE_PIXELS

//#define APPLY_CONTRAST

#ifdef RANDOMIZE_PIXELS
float rand(vec2 co) {
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453) * 0.25 + 0.75;
}
#endif

void main()
{
	vec4 diffuseColor = textureLod(u_DiffuseMap, var_ScreenTex, 0.0);
	
#if defined(BLUR_WIDTH)
	vec2 offset = vec2(1.0) / u_Dimensions;

	vec4 pixelLight = textureLod(u_NormalMap, var_ScreenTex, 0.0);
	vec3 volumeLight = pixelLight.rgb;
	
	float numSamples = 1.0;

	for (float x = -BLUR_WIDTH; x <= BLUR_WIDTH; x += 1.0)
	{
		for (float y = -BLUR_WIDTH; y <= BLUR_WIDTH; y += 1.0)
		{
			vec2 coord = var_ScreenTex + (offset * vec2(x*length(x), y*length(y)));
			coord.y = 1.0 - coord.y;
			volumeLight += textureLod(u_NormalMap, coord, 0.0).rgb;
			numSamples += 1.0;
		}
	}

	volumeLight /= numSamples;

#else
	vec3 volumeLight = textureLod(u_NormalMap, var_ScreenTex, 0.0).rgb;
#endif

#ifdef APPLY_CONTRAST
#define const_1 ( 64.0 / 255.0)
#define const_2 (255.0 / 255.0)
	volumeLight.rgb = clamp((clamp(volumeLight.rgb - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);
#endif

#ifdef DEBUG
	gl_FragColor.rgb = volumeLight;
	gl_FragColor.a = 1.0;
	return;
#endif

#ifdef RANDOMIZE_PIXELS
	volumeLight *= rand(var_ScreenTex * length(volumeLight.rgb) * u_Local1.r);
#endif

	gl_FragColor = vec4(diffuseColor.rgb + volumeLight, 1.0);
}
