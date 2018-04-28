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

vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

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
