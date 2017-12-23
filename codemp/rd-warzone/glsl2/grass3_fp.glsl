uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_SplatMap1;
uniform sampler2D	u_SplatMap2;
uniform sampler2D	u_OverlayMap;

uniform mat4		u_ModelViewProjectionMatrix;

uniform vec4		u_Local9;

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

flat in int			iGrassType;
smooth in vec2		vTexCoord;
in vec3				vVertPosition;
in vec3				m_Normal;

out vec4			out_Glow;
out vec4			out_Normal;
out vec4			out_Position;
out vec4			out_NormalDetail;

vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

void AddContrast ( inout vec3 color )
{
	const float contrast = 3.0;
	const float brightness = 0.03;
	//float contrast = u_Local9.r;
	//float brightness = u_Local9.g;
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
	vec4 diffuse;

	if (iGrassType >= 3)
		diffuse = texture(u_OverlayMap, vTexCoord);
	else if (iGrassType >= 2)
		diffuse = texture(u_SplatMap2, vTexCoord);
	else if (iGrassType >= 1)
		diffuse = texture(u_SplatMap1, vTexCoord);
	else
		diffuse = texture(u_DiffuseMap, vTexCoord);

	diffuse.rgb *= clamp((1.0-vTexCoord.y) * 1.5, 0.3, 1.0);

	float alpha = (diffuse.a > 0.5) ? 1.0 : 0.0;

	if (alpha == 0.0) discard;

	gl_FragColor = vec4(diffuse.rgb, 1.0);

	//vec4 m_Normal = ConvertToNormals(diffuse);

	out_Glow = vec4(0.0);
	out_Normal = vec4(EncodeNormal(m_Normal.xyz), 0.0, 1.0);
	out_Position = vec4(vVertPosition.xyz, MATERIAL_GREENLEAVES + 1.0);
	out_NormalDetail = vec4(0.0);
}
