uniform sampler2D u_DiffuseMap;

uniform vec2	u_Dimensions;
uniform vec4	u_Local4; // haveNormalMap, isMetalic, hasRealSubsurfaceMap, sway
uniform vec4	u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap


varying vec2	var_TexCoords;
varying vec3	var_Position;
varying vec3	var_Normal;
varying vec4	var_Color;

out vec4 out_Glow;
out vec4 out_Position;
out vec4 out_Normal;

void AddContrast ( inout vec3 color )
{
	const float contrast = 3.0;
	const float brightness = 0.03;
	// Apply contrast.
	color.rgb = ((color.rgb - 0.5f) * max(contrast, 0)) + 0.5f;
	// Apply brightness.
	color.rgb += brightness;
	//color.rgb = clamp(color.rgb, 0.0, 1.0);
}

vec4 ConvertToNormals ( vec4 colorIn )
{
	// This makes silly assumptions, but it adds variation to the output. Hopefully this will look ok without doing a crapload of texture lookups or
	// wasting vram on real normals.
	//
	// UPDATE: In my testing, this method looks just as good as real normal maps. I am now using this as default method unless r_normalmapping >= 2
	// for the very noticable FPS boost over texture lookups.

	vec4 color = colorIn;

	vec3 N = vec3(clamp(color.r + color.b, 0.0, 1.0), clamp(color.g + color.b, 0.0, 1.0), clamp(color.r + color.g, 0.0, 1.0));

	N.xy = 1.0 - N.xy;
	N.xyz = N.xyz * 0.5 + 0.5;
	N.xyz = pow(N.xyz, vec3(2.0));
	N.xyz *= 0.8;

	vec3 N2 = N;

	// Centralize the color, then stretch, generating lots of contrast...
	N.rgb = N.rgb * 0.5 + 0.5;
	AddContrast(N.rgb);

	float displacement = clamp(length(color.rgb), 0.0, 1.0);
#define const_1 ( 32.0 / 255.0)
#define const_2 (255.0 / 219.0)
	displacement = clamp((clamp(displacement - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);

	//vec4 norm = vec4((N + N2 + (1.0 - N.brg)) / 3.0, displacement);
	vec4 norm = vec4((N + N2) / 2.0, displacement);
	//norm.z = dot(N.xyz, 1.0 - N2.xyz);
	//norm.z = sqrt(clamp((1.0 - norm.x * norm.x) - norm.y * norm.y, 0.0, 1.0));
	//norm.z = (N.x + N.y) / 2.0;
	norm.rgb = norm.rbg;
	if (length(norm.xyz) < 0.1) norm.xyz = norm.xyz * 0.5 + 0.5;
	//return norm;// * colorIn.a;
	return vec4(vec3(1.0)-norm.rgb * 0.5, norm.a);
}

void main()
{
	vec2 texCoords = var_TexCoords;

	if (u_Local4.a > 0.0)
	{// Sway...
		texCoords += vec2(u_Local5.y * u_Local4.a * ((1.0 - texCoords.y) + 1.0), 0.0);
	}

	gl_FragColor = texture(u_DiffuseMap, texCoords);
	gl_FragColor.a *= var_Color.a;

	out_Glow = vec4(0.0);
	out_Position = vec4(var_Position.rgb, 1024.0);
	out_Normal = ConvertToNormals ( gl_FragColor );
}
