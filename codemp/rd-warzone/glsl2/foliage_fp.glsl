uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_SplatControlMap;
uniform sampler2D	u_SplatMap1;
uniform sampler2D	u_SplatMap2;
uniform sampler2D	u_SplatMap3;

uniform vec4		u_Local4;
uniform vec4		u_Local5;
uniform vec4		u_Local6;
uniform vec4		u_Local10;

in vec4				v_position;
in vec3				v_normal;
in vec2				v_texCoord;
in vec3				v_PrimaryLightDir;
flat in int			v_foliageLayer;

out vec4 out_Glow;
out vec4 out_Position;
out vec4 out_Normal;
#ifdef __USE_REAL_NORMALMAPS__
out vec4 out_NormalDetail;
#endif //__USE_REAL_NORMALMAPS__

//#define __ENCODE_NORMALS_RECONSTRUCT_Z__
#define __ENCODE_NORMALS_STEREOGRAPHIC_PROJECTION__
//#define __ENCODE_NORMALS_CRY_ENGINE__
//#define __ENCODE_NORMALS_EQUAL_AREA_PROJECTION__

#ifdef __ENCODE_NORMALS_STEREOGRAPHIC_PROJECTION__
vec2 EncodeNormal(vec3 n)
{
	float scale = 1.7777;
	vec2 enc = n.xy / (n.z + 1.0);
	enc /= scale;
	enc = enc * 0.5 + 0.5;
	return enc;
}
vec3 DecodeNormal(vec2 enc)
{
	vec3 enc2 = vec3(enc.xy, 0.0);
	float scale = 1.7777;
	vec3 nn =
		enc2.xyz*vec3(2.0 * scale, 2.0 * scale, 0.0) +
		vec3(-scale, -scale, 1.0);
	float g = 2.0 / dot(nn.xyz, nn.xyz);
	return vec3(g * nn.xy, g - 1.0);
}
#elif defined(__ENCODE_NORMALS_CRY_ENGINE__)
vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N * 4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);
	return vec3(encoded * g, 1.0 - f * 0.5);
}
vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}
#elif defined(__ENCODE_NORMALS_EQUAL_AREA_PROJECTION__)
vec2 EncodeNormal(vec3 n)
{
	float f = sqrt(8.0 * n.z + 8.0);
	return n.xy / f + 0.5;
}
vec3 DecodeNormal(vec2 enc)
{
	vec2 fenc = enc * 4.0 - 2.0;
	float f = dot(fenc, fenc);
	float g = sqrt(1.0 - f / 4.0);
	vec3 n;
	n.xy = fenc*g;
	n.z = 1.0 - f / 2.0;
	return n;
}
#else //__ENCODE_NORMALS_RECONSTRUCT_Z__
vec3 DecodeNormal(in vec2 N)
{
	vec3 norm;
	norm.xy = N * 2.0 - 1.0;
	norm.z = sqrt(1.0 - dot(norm.xy, norm.xy));
	return norm;
}
vec2 EncodeNormal(vec3 n)
{
	return vec2(n.xy * 0.5 + 0.5);
}
#endif //__ENCODE_NORMALS_RECONSTRUCT_Z__

vec4 GetControlMap(vec3 m_vertPos)
{
	float scale = 1.0 / u_Local6.b; /* control scale */
	float offset = (u_Local6.b / 2.0) * scale;
	vec4 xaxis = texture( u_SplatControlMap, (m_vertPos.yz * scale) + offset);
	vec4 yaxis = texture( u_SplatControlMap, (m_vertPos.xz * scale) + offset);
	vec4 zaxis = texture( u_SplatControlMap, (m_vertPos.xy * scale) + offset);
	vec4 control = xaxis * 0.333 + yaxis * 0.333 + zaxis * 0.333;
	control = clamp(control * u_Local10.b, 0.0, 1.0);
	return control;
}

vec4 GetGrassMap(vec3 m_vertPos)
{
	vec4 control = GetControlMap(m_vertPos);
	return control;
}

void main()
{
	vec2 texCoords = v_texCoord * u_Local10.g;
	texCoords += vec2(u_Local5.y * u_Local4.a * ((1.0 - v_texCoord.y) + 1.0), 0.0);

	vec4 grassMap = GetGrassMap(v_position.xyz);

	int grassMap2[4];
	//grassMap2[0] = (grassMap.r >= 0.2 || grassMap.g >= 0.2 || grassMap.b >= 0.2) ? 1 : 0;
	grassMap2[0] = 0;
	grassMap2[1] = grassMap.r >= 0.3 ? 1 : 0;
	grassMap2[2] = grassMap.g >= 0.3 ? 1 : 0;
	grassMap2[3] = grassMap.b >= 0.3 ? 1 : 0;

	gl_FragColor = vec4(0.0);

	if (v_foliageLayer == 3 && grassMap2[3] == 1)
	{
		gl_FragColor = texture(u_SplatMap3, texCoords);
	}
	else if (v_foliageLayer == 2 && grassMap2[2] == 1)
	{
		gl_FragColor = texture(u_SplatMap2, texCoords);
	}
	else if (v_foliageLayer == 1 && grassMap2[1] == 1)
	{
		gl_FragColor = texture(u_SplatMap1, texCoords);
	}
	else if (grassMap2[0] == 1)
	{
		gl_FragColor = texture(u_DiffuseMap, texCoords);
	}

	if (gl_FragColor.a > 0.5) 
	{
		gl_FragColor.a = 1.0;
	}
	else
	{
		gl_FragColor.a = 0.0;
	}

	out_Position = vec4(v_position.xyz, gl_FragColor.a > 0.0 ? MATERIAL_GREENLEAVES + 1.0 : 0.0);
	out_Normal = vec4(EncodeNormal(v_normal.xyz), 0.0, gl_FragColor.a > 0.0 ? 1.0 : 0.0);
	out_Glow = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
	out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
}
