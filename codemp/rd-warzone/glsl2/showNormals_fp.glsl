uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;

varying vec2		var_TexCoords;

vec2 encode (vec3 n)
{
    float p = sqrt(n.z*8+8);
    return vec2(n.xy/p + 0.5);
}

vec3 decode (vec2 enc)
{
    vec2 fenc = enc*4-2;
    float f = dot(fenc,fenc);
    float g = sqrt(1-f/4);
    vec3 n;
    n.xy = fenc*g;
    n.z = 1-f/2;
    return n;
}

vec4 ConvertToNormals ( vec4 color )
{
	// This makes silly assumptions, but it adds variation to the output. Hopefully this will look ok without doing a crapload of texture lookups or
	// wasting vram on real normals.
	//
	// UPDATE: In my testing, this method looks just as good as real normal maps. I am now using this as default method unless r_normalmapping >= 2
	// for the very noticable FPS boost over texture lookups.

	vec3 N = vec3(clamp(color.r + color.b, 0.0, 1.0), clamp(color.g + color.b, 0.0, 1.0), clamp(color.r + color.g, 0.0, 1.0));

	N.xy = 1.0 - N.xy;
	N.xyz = N.xyz * 0.5 + 0.5;
	N.xyz = pow(N.xyz, vec3(2.0));
	N.xyz *= 0.8;

	float displacement = clamp(length(color.rgb), 0.0, 1.0);
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
	vec4 norm = texture(u_NormalMap, var_TexCoords);
	norm.xyz = decode(norm.xy);

	vec4 color = texture(u_DiffuseMap, var_TexCoords);

	/*
	vec2 encode (vec3 n)
	vec3 decode (vec2 enc)
	*/

	if (/*norm.a < 0.05 ||*/ length(norm.xyz) <= 0.05)
	{
		norm = ConvertToNormals(color);
	}
	else
	{
		float displacement = clamp(length(color.rgb), 0.0, 1.0);
#define const_1 ( 32.0 / 255.0)
#define const_2 (255.0 / 219.0)
		norm.a = clamp((clamp(norm.a - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);
	}

	gl_FragColor = vec4(norm.rgb, 1.0);
}