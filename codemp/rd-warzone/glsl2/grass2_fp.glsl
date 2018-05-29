uniform sampler2D	u_DiffuseMap;

#if !defined(__USE_FAST_GRASS__)
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

	#ifdef __HIGH_MTU_AVAILABLE__
	uniform sampler2D	u_WaterEdgeMap; // Sea grass 0
	uniform sampler2D	u_WaterPositionMap; // Sea grass 1
	uniform sampler2D	u_WaterHeightMap; // Sea grass 2
	uniform sampler2D	u_GlowMap; // Sea grass 3
	#endif //__HIGH_MTU_AVAILABLE__
#else //defined(__USE_FAST_GRASS__)
uniform sampler2D	u_WaterEdgeMap; // Sea grass 0
#endif //!defined(__USE_FAST_GRASS__)

uniform vec3		u_ViewOrigin;

uniform vec4						u_Local1; // MAP_SIZE, sway, overlaySway, materialType
uniform vec4						u_Local2; // hasSteepMap, hasWaterEdgeMap, haveNormalMap, SHADER_WATER_LEVEL
uniform vec4						u_Local3; // hasSplatMap1, hasSplatMap2, hasSplatMap3, hasSplatMap4
uniform vec4						u_Local8; // passnum, GRASS_DISTANCE_FROM_ROADS, GRASS_HEIGHT, 0.0
uniform vec4						u_Local9; // testvalue0, 1, 2, 3
uniform vec4						u_Local10; // foliageLODdistance, GRASS_UNDERWATER_ONLY, 0.0, GRASS_TYPE_UNIFORMALITY
uniform vec4						u_Local11; // GRASS_WIDTH_REPEATS, 0.0, 0.0, 0.0

#define SHADER_MAP_SIZE				u_Local1.r
#define SHADER_SWAY					u_Local1.g
#define SHADER_OVERLAY_SWAY			u_Local1.b
#define SHADER_MATERIAL_TYPE		u_Local1.a

#define SHADER_HAS_STEEPMAP			u_Local2.r
#define SHADER_HAS_WATEREDGEMAP		u_Local2.g
#define SHADER_HAS_NORMALMAP		u_Local2.b
#define SHADER_WATER_LEVEL			u_Local2.a

#define SHADER_HAS_SPLATMAP1		u_Local3.r
#define SHADER_HAS_SPLATMAP2		u_Local3.g
#define SHADER_HAS_SPLATMAP3		u_Local3.b
#define SHADER_HAS_SPLATMAP4		u_Local3.a

#define PASS_NUMBER					u_Local8.r
#define GRASS_DISTANCE_FROM_ROADS	u_Local8.g
#define GRASS_HEIGHT				u_Local8.b

#define MAX_RANGE					u_Local10.r
#define GRASS_UNDERWATER_ONLY		u_Local10.g
#define GRASS_TYPE_UNIFORMALITY		u_Local10.a

#define GRASS_WIDTH_REPEATS			u_Local11.r

smooth in vec2		vTexCoord;
smooth in vec3		vVertPosition;
//flat in float		vVertNormal;
smooth in vec2		vVertNormal;
flat in int			iGrassType;

out vec4			out_Glow;
out vec4			out_Normal;
#ifdef __USE_REAL_NORMALMAPS__
out vec4			out_NormalDetail;
#endif //__USE_REAL_NORMALMAPS__
out vec4			out_Position;

const float xdec = 1.0/255.0;
const float ydec = 1.0/65025.0;
const float zdec = 1.0/16581375.0;

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

vec4 DecodeFloatRGBA( float v ) {
  vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
  enc = fract(enc);
  enc -= enc.yzww * vec4(xdec,xdec,xdec,0.0);
  return enc;
}

void main() 
{
	vec4 diffuse;

	vec2 tc = vTexCoord;

	if (GRASS_WIDTH_REPEATS > 0.0) tc.x *= (GRASS_WIDTH_REPEATS * 2.0);

#if defined(__USE_UNDERWATER_ONLY__)
	if (iGrassType >= 19)
		diffuse = texture(u_GlowMap, tc);
	else if (iGrassType >= 18)
		diffuse = texture(u_WaterHeightMap, tc);
	else if (iGrassType >= 17)
		diffuse = texture(u_WaterPositionMap, tc);
	else
		diffuse = texture(u_WaterEdgeMap, tc);
#elif defined(__USE_FAST_GRASS__)
	/*if (iGrassType >= 19)
		diffuse = texture(u_GlowMap, tc);
	else if (iGrassType >= 18)
		diffuse = texture(u_WaterHeightMap, tc);
	else if (iGrassType >= 17)
		diffuse = texture(u_WaterPositionMap, tc);
	else if (iGrassType >= 16)
		diffuse = texture(u_WaterEdgeMap, tc);
	else if (iGrassType >= 3)
		diffuse = texture(u_SplatMap3, tc);
	else if (iGrassType >= 2)
		diffuse = texture(u_SplatMap2, tc);
	else if (iGrassType >= 1)
		diffuse = texture(u_SplatMap1, tc);
	else
		diffuse = texture(u_DiffuseMap, tc);*/
	if (iGrassType >= 1)
		diffuse = texture(u_WaterEdgeMap, tc);
	else
		diffuse = texture(u_DiffuseMap, tc);
#else
	#if defined(__HIGH_MTU_AVAILABLE__)
	if (iGrassType >= 19)
		diffuse = texture(u_GlowMap, tc);
	else if (iGrassType >= 18)
		diffuse = texture(u_WaterHeightMap, tc);
	else if (iGrassType >= 17)
		diffuse = texture(u_WaterPositionMap, tc);
	else if (iGrassType >= 16)
		diffuse = texture(u_WaterEdgeMap, tc);
	else if (iGrassType >= 15)
	#else //!defined(__HIGH_MTU_AVAILABLE__)
	if (iGrassType >= 15)
	#endif //defined(__HIGH_MTU_AVAILABLE__)
		diffuse = texture(u_HeightMap, tc);
	else if (iGrassType >= 14)
		diffuse = texture(u_PositionMap, tc);
	else if (iGrassType >= 13)
		diffuse = texture(u_CubeMap, tc);
	else if (iGrassType >= 12)
		diffuse = texture(u_LightMap, tc);
	else if (iGrassType >= 11)
		diffuse = texture(u_ShadowMap, tc);
	else if (iGrassType >= 10)
		diffuse = texture(u_OverlayMap, tc);
	else if (iGrassType >= 9)
		diffuse = texture(u_NormalMap, tc);
	else if (iGrassType >= 8)
		diffuse = texture(u_DeluxeMap, tc);
	else if (iGrassType >= 7)
		diffuse = texture(u_SpecularMap, tc);
	else if (iGrassType >= 6)
		diffuse = texture(u_DetailMap, tc);
	else if (iGrassType >= 5)
		diffuse = texture(u_RoadMap, tc);
	else if (iGrassType >= 4)
		diffuse = texture(u_SteepMap, tc);
	else if (iGrassType >= 3)
		diffuse = texture(u_SplatMap3, tc);
	else if (iGrassType >= 2)
		diffuse = texture(u_SplatMap2, tc);
	else if (iGrassType >= 1)
		diffuse = texture(u_SplatMap1, tc);
	else
		diffuse = texture(u_DiffuseMap, tc);
#endif //defined(__USE_UNDERWATER_ONLY__)

	//diffuse.rgb *= clamp((1.0 - vTexCoord.y) * 1.5, 0.8, 1.0);

	if (diffuse.a > 0.5)
	{
		gl_FragColor = vec4(diffuse.rgb, 1.0);
		out_Glow = vec4(0.0);
		
		//vec3 dir = normalize(u_ViewOrigin - vVertPosition);
		//out_Normal = vec4(EncodeNormal(DecodeNormal(vVertNormal.xy) * dir), 0.0, 1.0);
		out_Normal = vec4(EncodeNormal(DecodeNormal(vVertNormal.xy)), 0.0, 1.0);

#ifdef __USE_REAL_NORMALMAPS__
		out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		out_Position = vec4(vVertPosition, MATERIAL_GREENLEAVES+1.0);
	}
	else
	{
		gl_FragColor = vec4(0.0);
		out_Glow = vec4(0.0);
		out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
		out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		out_Position = vec4(0.0);
	}
}
