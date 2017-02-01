uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;

varying vec2		var_TexCoords;

vec4 ConvertToNormals ( vec4 color )
{
	// This makes silly assumptions, but it adds variation to the output. Hopefully this will look ok without doing a crapload of texture lookups or
	// wasting vram on real normals.
	//
	// UPDATE: In my testing, this method looks just as good as real normal maps. I am now using this as default method unless r_normalmapping >= 2
	// for the very noticable FPS boost over texture lookups.

	vec3 color2 = color.rgb;

	vec3 N = vec3(clamp(color2.r + color2.b, 0.0, 1.0), clamp(color2.g + color2.b, 0.0, 1.0), clamp(color2.r + color2.g, 0.0, 1.0));

	vec3 brightness = color2.rgb; //adaptation luminance
	brightness = (brightness/(brightness+1.0));
	brightness = vec3(max(brightness.x, max(brightness.y, brightness.z)));
	vec3 brightnessMult = (vec3(1.0) - brightness) * 0.5;

	color2 = pow(clamp(color2 + brightnessMult, 0.0, 1.0), vec3(2.0));

	N.xy = 1.0 - N.xy;
	N.xyz = N.xyz * 0.5 + 0.5;
	N.xyz = pow(N.xyz, vec3(2.0));
	N.xyz *= 0.8;

	//float displacement = brightness.r;
	float displacement = clamp(length(color.rgb), 0.0, 1.0);
	//float displacement = clamp(length(color2.rgb), 0.0, 1.0);
#define const_1 ( 32.0 / 255.0)
#define const_2 (255.0 / 219.0)
	displacement = clamp((clamp(displacement - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);

	vec4 norm = vec4(N, displacement);

	if (norm.g > norm.b)
	{// Switch them for screen space fakes...
		norm.gb = norm.bg;
	}

	return norm;
}

void main(void)
{
	vec4 norm = texture2D(u_NormalMap, var_TexCoords);

	if (norm.a < 0.05 || length(norm.xyz) <= 0.05)
	{
		norm = ConvertToNormals(texture(u_DiffuseMap, var_TexCoords));
	}

	gl_FragColor = vec4(norm.rgb, 1.0);
}