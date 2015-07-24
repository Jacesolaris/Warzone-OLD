uniform sampler2D u_TextureMap;
uniform sampler2D u_LevelsMap;
uniform vec4      u_Color;
uniform vec2      u_AutoExposureMinMax;
uniform vec3      u_ToneMinAvgMaxLinear;

varying vec2		var_TexCoords;
varying vec2		var_Dimensions;
varying vec2		var_ScreenTex;

uniform vec4		u_ViewInfo; // zfar / znear, zfar
uniform vec2		u_Dimensions;

float near = u_ViewInfo.x;
float far = u_ViewInfo.y;
float viewWidth = var_Dimensions.x;
float viewHeight = var_Dimensions.y;

vec2 texCoord = var_TexCoords;

//float Vibrance = 0.35;
float Vibrance = 0.15;

vec4 VibrancePass( vec4 colorInput )
{
	#define Vibrance_RGB_balance vec3(1.00, 1.00, 1.00)
	#define Vibrance_coeff vec3(Vibrance_RGB_balance * Vibrance)

	vec4 color = colorInput; //original input color
	vec3 lumCoeff = vec3(0.212656, 0.715158, 0.072186);  //Values to calculate luma with

	float luma = dot(lumCoeff, color.rgb); //calculate luma (grey)

	float max_color = max(colorInput.r, max(colorInput.g, colorInput.b)); //Find the strongest color
	float min_color = min(colorInput.r, min(colorInput.g, colorInput.b)); //Find the weakest color

	float color_saturation = max_color - min_color; //The difference between the two is the saturation

	//color.rgb = mix(luma, color.rgb, (1.0 + (Vibrance * (1.0 - color_saturation)))); //extrapolate between luma and original by 1 + (1-saturation) - simple

	//color.rgb = mix(luma, color.rgb, (1.0 + (Vibrance * (1.0 - (sign(Vibrance) * color_saturation))))); //extrapolate between luma and original by 1 + (1-saturation) - current
	color.r = mix(luma, color.r, (1.0 + (Vibrance_coeff.r * (1.0 - (sign(Vibrance_coeff.r) * color_saturation))))); //extrapolate between luma and original by 1 + (1-saturation) - current
	color.g = mix(luma, color.g, (1.0 + (Vibrance_coeff.g * (1.0 - (sign(Vibrance_coeff.g) * color_saturation))))); //extrapolate between luma and original by 1 + (1-saturation) - current
	color.b = mix(luma, color.b, (1.0 + (Vibrance_coeff.b * (1.0 - (sign(Vibrance_coeff.b) * color_saturation))))); //extrapolate between luma and original by 1 + (1-saturation) - current

	//color.rgb = mix(luma, color.rgb, 1.0 + (1.0-pow(color_saturation, 1.0 - (1.0-Vibrance))) ); //pow version

	return color; //return the result
	//return color_saturation.xxxx; //Visualize the saturation
}

vec4 GetHDR( vec3 col2 )
{
	vec3 lumask = vec3(0.2125, 0.7154, 0.0721);
	float USE_FILTER = 1.0f;
	vec3 color = col2.rgb;

	if (USE_FILTER > 0.0)
	{
		vec2 recipres = vec2(1.0f / viewWidth, 1.0f / viewHeight);
		vec2 tc = texCoord.st, add = recipres;

		color = 0.25*(
			texture2D(u_TextureMap, tc+add.xy)+
			texture2D(u_TextureMap, tc-add.xy)+
			texture2D(u_TextureMap, tc+add.yx)+
			texture2D(u_TextureMap, tc-add.yx)
		).rgb;
	}

	float bright = log(dot(color, lumask) + 0.25);
	color += 0.2*sin(-10.0*color/(color+2.0));
	return vec4(color, bright);
}

void main()
{
	vec4 color = texture2D(u_TextureMap, texCoord);

	// Original - Only HDR...
//	gl_FragColor = GetHDR( color.rgb );
//	gl_FragColor.a = 1.0;

	// Mix HDR/Normal 50/50...
//	gl_FragColor = (color + GetHDR( color.rgb )) * 0.5;
//	gl_FragColor.a = 1.0;

	// Use HDR+VIBRANCE mix...
//	float luminance = dot( color.rgb, vec3( 0.6, 0.2, 0.2 ) );
//	gl_FragColor.rgb = mix( color.rgb, GetHDR( color.rgb ).rgb, luminance * luminance ).rgb;
	gl_FragColor.rgb = GetHDR( color.rgb ).rgb;
	gl_FragColor.rgba = VibrancePass( (gl_FragColor.rgba + color.rgba + color.rgba) * 0.33333 ).rgba;
	gl_FragColor.a = 1.0;
}
