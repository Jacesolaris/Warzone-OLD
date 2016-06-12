uniform sampler2D	u_TextureMap;

varying vec2		var_TexCoords;
varying vec2		var_Dimensions;

vec2 recipres = vec2(1.0f / var_Dimensions.x, 1.0f / var_Dimensions.y);

const float Vibrance = 0.15;

#define Vibrance_RGB_balance vec3(1.00, 1.00, 1.00)
#define Vibrance_coeff vec3(Vibrance_RGB_balance * Vibrance)

const vec3 lumCoeff = vec3(0.212656, 0.715158, 0.072186);  //Values to calculate luma with

vec4 VibrancePass( vec4 colorInput )
{
	vec4 color = colorInput; //original input color
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
	float USE_FILTER = 1.0f;
	vec3 color = col2.rgb;

	if (USE_FILTER > 0.0)
	{
		vec2 tc = var_TexCoords.st, add = recipres;

		color = 0.25*(
			texture2D(u_TextureMap, tc+add.xy)+
			texture2D(u_TextureMap, tc-add.xy)+
			texture2D(u_TextureMap, tc+add.yx)+
			texture2D(u_TextureMap, tc-add.yx)
		).rgb;
	}

	float bright = log(dot(color, lumCoeff) + 0.25);
	color += 0.2*sin(-10.0*color/(color+2.0));
	return vec4(color, bright);
}

struct Fragment {
    vec3 color;
    float gamma;
};

vec3 tone_map(in Fragment fragment, float luma)
{
    fragment.color = exp(-1.0 / (2.72 * fragment.color + 0.15));
    fragment.color = pow(fragment.color, vec3(1.0 / (fragment.gamma * luma)));
    return fragment.color;
}

vec3 newHDR(in Fragment fragment) {
    float luma = dot(fragment.color, vec3(0.2126, 0.7152, 0.0722));
    return mix(fragment.color, tone_map(fragment, luma), 1.0 - luma);
}

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;

vec3 SexyTonemapMath(vec3 x)
{
     return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec4 SexyTonemap(vec3 inColor)
{
     vec3 texColor = inColor;
     texColor *= 16.0;  // Hardcoded Exposure Adjustment
     float ExposureBias = 2.0;
     vec3 curr = SexyTonemapMath(texColor * ExposureBias);
     vec3 whiteScale = vec3(1.0) / SexyTonemapMath( vec3(W) );
     vec3 color = curr * whiteScale;
     vec3 retColor = pow(color, vec3(1.0 / 2.2));
     return vec4(retColor, 1.0);
}

void main()
{
	vec4 color = texture2D(u_TextureMap, var_TexCoords);
	//vec4 HDR = GetHDR( color.rgb );

//#define const_1 ( 12.0 / 255.0)
//#define const_2 (255.0 / 219.0)
	//HDR = ((clamp(HDR - const_1, 0.0, 1.0)) * const_2);

	// Use HDR+VIBRANCE mix...
	//gl_FragColor = ( (HDR + color + color) * 0.33333 );
	//gl_FragColor = VibrancePass( HDR + color + color * 0.33333 );
	//gl_FragColor.a = 1.0;

	// New method...
	//Fragment frag;
	//frag.color = color.rgb;
	//frag.gamma = 6.4;
	//gl_FragColor = vec4((color.rgb + (newHDR(frag) * 2.0)) / 3.0, 1.0);
	//gl_FragColor = SexyTonemap(newHDR(frag));

#define const_1 ( 24.0 / 255.0)
#define const_2 (255.0 / 219.0)
	gl_FragColor.rgb = clamp((clamp(color.rgb - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);
	gl_FragColor.a = 1.0;
}
