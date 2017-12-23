uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_SplatMap1;
uniform sampler2D	u_SplatMap2;
uniform sampler2D	u_OverlayMap;

uniform vec4		u_Local9;

flat in int			iGrassType;
smooth in vec2		vTexCoord;
smooth in vec3		vVertPosition;
in vec3				vVertNormal;

out vec4			out_Glow;
out vec4			out_Normal;
out vec4			out_NormalDetail;
out vec4			out_Position;

vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
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

	if (diffuse.a > 0.5)
	{
		gl_FragColor = vec4(diffuse.rgb, 1.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(EncodeNormal(vVertNormal.xyz), 0.0, 1.0);
		out_NormalDetail = vec4(0.0);
		//out_Position = vec4(vVertPosition, MATERIAL_GREENLEAVES+1.0);
		out_Position = vec4(0.0);
	}
	else
	{
		gl_FragColor = vec4(0.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(0.0);
		out_NormalDetail = vec4(0.0);
		out_Position = vec4(0.0);
	}
}
