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

uniform sampler2D	u_OverlayMap;
uniform sampler2D	u_LightMap;
uniform sampler2D	u_ShadowMap;
uniform sampler2D	u_CubeMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_HeightMap;

uniform sampler2D	u_WaterEdgeMap; // Sea grass



uniform vec4		u_Local9;

//flat in int			iGrassType;
smooth in vec2		vTexCoord;
smooth in vec3		vVertPosition;
//in vec2				vVertNormal;
flat in float			vVertNormal;

out vec4			out_Glow;
out vec4			out_Normal;
out vec4			out_NormalDetail;
out vec4			out_Position;

//#define _DEBUG_

const float xdec = 1.0/255.0;
const float ydec = 1.0/65025.0;
const float zdec = 1.0/16581375.0;
  
vec4 DecodeFloatRGBA( float v ) {
  vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
  enc = fract(enc);
  enc -= enc.yzww * vec4(xdec,xdec,xdec,0.0);
  return enc;
}

void main() 
{
	//vec3 normal = vec3(vVertNormal.xy, 0.0);
	//normal.z = sqrt(1.0-dot(normal.xy, normal.xy)) * 2.0 - 1.0; // reconstruct Z from X and Y
	vec4 normal = DecodeFloatRGBA(vVertNormal);
	int iGrassType = int(normal.a * 16.0);

#if defined(_DEBUG_)
	if (u_Local9.g > 0.0)
	{
		gl_FragColor = vec4(1.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(normal.xyz * 0.5 + 0.5, 1.0);
		out_NormalDetail = vec4(0.0);
		out_Position = vec4(vVertPosition, MATERIAL_GREENLEAVES+1.0);
		//out_Position = vec4(0.0);
		return;
	}
#endif

	vec4 diffuse;

	if (iGrassType >= 16)
		diffuse = texture(u_WaterEdgeMap, vTexCoord);
	else if (iGrassType >= 15)
		diffuse = texture(u_HeightMap, vTexCoord);
	else if (iGrassType >= 14)
		diffuse = texture(u_PositionMap, vTexCoord);
	else if (iGrassType >= 13)
		diffuse = texture(u_CubeMap, vTexCoord);
	else if (iGrassType >= 12)
		diffuse = texture(u_LightMap, vTexCoord);
	else if (iGrassType >= 11)
		diffuse = texture(u_ShadowMap, vTexCoord);
	else if (iGrassType >= 10)
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

	diffuse.rgb *= clamp((1.0-vTexCoord.y) * 1.5, 0.8, 1.0);

	if (diffuse.a > 0.5)
	{
		gl_FragColor = vec4(diffuse.rgb, 1.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(normal.xy * 0.5 + 0.5, 0.0, 1.0);
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
