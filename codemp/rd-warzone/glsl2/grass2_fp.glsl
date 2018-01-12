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

uniform sampler2D	u_WaterEdgeMap; // Sea grass 0
uniform sampler2D	u_WaterPositionMap; // Sea grass 1
uniform sampler2D	u_WaterHeightMap; // Sea grass 2
uniform sampler2D	u_GlowMap; // Sea grass 3

uniform vec3		u_ViewOrigin;

uniform vec4		u_Local9;

smooth in vec2		vTexCoord;
smooth in vec3		vVertPosition;
//flat in float		vVertNormal;
smooth in vec2		vVertNormal;
flat in int			iGrassType;

out vec4			out_Glow;
out vec4			out_Normal;
out vec4			out_NormalDetail;
out vec4			out_Position;

const float xdec = 1.0/255.0;
const float ydec = 1.0/65025.0;
const float zdec = 1.0/16581375.0;

vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N*4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);

	return vec3(encoded * g, 1.0 - f * 0.5);
}

vec4 DecodeFloatRGBA( float v ) {
  vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
  enc = fract(enc);
  enc -= enc.yzww * vec4(xdec,xdec,xdec,0.0);
  return enc;
}

void main() 
{
	//vec4 normal = DecodeFloatRGBA(vVertNormal);
	//int iGrassType = int(normal.a * 20.0);

	vec3 dir = normalize(u_ViewOrigin - vVertPosition);

	vec4 diffuse;

	if (iGrassType >= 19)
		diffuse = texture(u_GlowMap, vTexCoord);
	else if (iGrassType >= 18)
		diffuse = texture(u_WaterHeightMap, vTexCoord);
	else if (iGrassType >= 17)
		diffuse = texture(u_WaterPositionMap, vTexCoord);
	else if (iGrassType >= 16)
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
		out_Normal = vec4(EncodeNormal(DecodeNormal(vVertNormal.xy) * dir), 0.0, 1.0);
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
