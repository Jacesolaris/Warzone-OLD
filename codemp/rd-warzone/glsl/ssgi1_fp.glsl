uniform sampler2D u_TextureMap;
uniform sampler2D u_ScreenDepthMap;
uniform sampler2D u_NormalMap; // actually saturation map image

varying vec2		var_TexCoords;
varying vec2		var_Dimensions;
varying vec4		var_ViewInfo; // zmin, zmax, zmax / zmin
varying vec4		var_Local0; // MODE, NUM_SAMPLES, 0, 0

vec3 CalculateFlare ( vec3 flare_color, vec3 final_color )
{
	float bt = (flare_color.r + flare_color.g + flare_color.b) / 3.0;

	if (bt > 0.666) 
		bt *= 0.666; // Bright lights get dulled... (eg: white)
	else if (bt < 0.333) 
		bt *= 1.8; // Dull lights get amplified... (eg: blue)
	else 
		bt *= 1.1; // Mid range lights get amplified slightly... (eg: yellow)

	vec3 flare_color2 = clamp(flare_color * bt * 8.0, 0.0, 1.0);
	vec3 add_flare = clamp(final_color * (flare_color2 * (1.25 - final_color) * 2.5), 0.0, 1.0);

#define const_1 ( 12.0 / 255.0)
#define const_2 (255.0 / 219.0)
	add_flare = ((clamp(add_flare - const_1, 0.0, 1.0)) * const_2);

	return add_flare * 0.7;
}

void main()
{   
	// Fast (just color bleed) mode...
	vec3 final_color = texture2D(u_TextureMap, var_TexCoords.st).xyz;// * 1.25;
		
	// UQ1: Let's add some of the flare color as well... Just to boost colors/glows...
	vec3 flare_color = clamp(texture2D(u_NormalMap, var_TexCoords.st).rgb, 0.0, 1.0);
	vec3 add_flare = CalculateFlare(flare_color, final_color);
	final_color = clamp((final_color + final_color + final_color + max(add_flare, final_color)) / 4.0, 0.0, 1.0);

	gl_FragColor = vec4(final_color,1.0);
}
