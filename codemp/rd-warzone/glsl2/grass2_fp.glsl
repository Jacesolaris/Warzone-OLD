uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_SplatMap1;
uniform sampler2D	u_SplatMap2;
uniform sampler2D	u_SplatMap3;
uniform sampler2D	u_SteepMap;
uniform sampler2D	u_RoadMap;
uniform sampler2D	u_DetailMap;
uniform sampler2D	u_SpecularMap;
uniform sampler2D	u_DeluxeMap;
uniform sampler2D	u_NormalMap;

uniform sampler2D	u_OverlayMap; // Sea grass


uniform vec4		u_Local9;

flat in int			iGrassType;
smooth in vec2		vTexCoord;
smooth in vec3		vVertPosition;
in vec3				vVertNormal;

out vec4			out_Glow;
out vec4			out_Normal;
out vec4			out_NormalDetail;
out vec4			out_Position;

//#define _DEBUG_

void main() 
{
#if defined(_DEBUG_)
	if (u_Local9.g > 0.0)
	{
		gl_FragColor = vec4(1.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(vVertNormal.xyz * 0.5 + 0.5, 1.0);
		out_NormalDetail = vec4(0.0);
		out_Position = vec4(vVertPosition, MATERIAL_GREENLEAVES+1.0);
		//out_Position = vec4(0.0);
		return;
	}
#endif

	vec4 diffuse;

	if (iGrassType >= 10)
		diffuse = texture(u_OverlayMap, vTexCoord);
	else if (iGrassType >= 9)
		diffuse = texture(u_NormalMap, vTexCoord);
	else if (iGrassType >= 8)
		diffuse = texture(u_DeluxeMap, vTexCoord);
	else if (iGrassType >= 7)
		diffuse = texture(u_SpecularMap, vTexCoord);
	else if (iGrassType >= 6)
		diffuse = texture(u_DetailMap, vTexCoord);
	else if (iGrassType >= 5)
		diffuse = texture(u_RoadMap, vTexCoord);
	else if (iGrassType >= 4)
		diffuse = texture(u_SteepMap, vTexCoord);
	else if (iGrassType >= 3)
		diffuse = texture(u_SplatMap3, vTexCoord);
	else if (iGrassType >= 2)
		diffuse = texture(u_SplatMap2, vTexCoord);
	else if (iGrassType >= 1)
		diffuse = texture(u_SplatMap1, vTexCoord);
	else
		diffuse = texture(u_DiffuseMap, vTexCoord);

	diffuse.rgb *= clamp((1.0-vTexCoord.y) * 1.5, 0.6, 1.0);

	if (diffuse.a > 0.5)
	{
		gl_FragColor = vec4(diffuse.rgb, 1.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(vVertNormal.x * 0.5 + 0.5, vVertNormal.y * 0.5 + 0.5, 0.0, 1.0);
		out_NormalDetail = vec4(0.0);
		out_Position = vec4(vVertPosition, MATERIAL_GREENLEAVES+1.0);
		//out_Position = vec4(0.0);
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
