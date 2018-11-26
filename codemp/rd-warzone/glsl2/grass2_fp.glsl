uniform sampler2D					u_DiffuseMap;	// Land grass atlas
uniform sampler2D					u_WaterEdgeMap; // Sea grass atlas

uniform vec3						u_ViewOrigin;

uniform vec4						u_Settings1; // IS_DEPTH_PASS, 0.0, 0.0, 0.0
uniform vec4						u_Settings5; // MAP_COLOR_SWITCH_RG, MAP_COLOR_SWITCH_RB, MAP_COLOR_SWITCH_GB, 0.0

#define IS_DEPTH_PASS				u_Settings1.r

#define MAP_COLOR_SWITCH_RG			u_Settings5.r
#define MAP_COLOR_SWITCH_RB			u_Settings5.g
#define MAP_COLOR_SWITCH_GB			u_Settings5.b

uniform vec4						u_Local1; // MAP_SIZE, sway, overlaySway, materialType
uniform vec4						u_Local2; // hasSteepMap, hasWaterEdgeMap, haveNormalMap, SHADER_WATER_LEVEL
uniform vec4						u_Local3; // hasSplatMap1, hasSplatMap2, hasSplatMap3, hasSplatMap4
uniform vec4						u_Local8; // passnum, GRASS_DISTANCE_FROM_ROADS, GRASS_HEIGHT, 0.0
uniform vec4						u_Local9; // testvalue0, 1, 2, 3
uniform vec4						u_Local10; // foliageLODdistance, TERRAIN_TESS_OFFSET, 0.0, GRASS_TYPE_UNIFORMALITY
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
#define TERRAIN_TESS_OFFSET			u_Local10.g
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
	if (IS_DEPTH_PASS > 0.0)
	{
		diffuse = vec4(1.0, 1.0, 1.0, texture(u_WaterEdgeMap, tc).a);
	}
	else
	{
		diffuse = texture(u_WaterEdgeMap, tc);
	}
#else //!defined(__USE_UNDERWATER_ONLY__)
	if (IS_DEPTH_PASS > 0.0)
	{
		if (iGrassType >= 1)
		{
			diffuse = vec4(1.0, 1.0, 1.0, texture(u_WaterEdgeMap, tc).a);
		}
		else
		{
			diffuse = vec4(1.0, 1.0, 1.0, texture(u_DiffuseMap, tc).a);
		}
	}
	else if (iGrassType >= 1)
	{
		diffuse = texture(u_WaterEdgeMap, tc);
	}
	else
	{
		diffuse = texture(u_DiffuseMap, tc);
	}
#endif //defined(__USE_UNDERWATER_ONLY__)

	if (MAP_COLOR_SWITCH_RG > 0.0)
	{
		diffuse.rg = diffuse.gr;
	}

	if (MAP_COLOR_SWITCH_RB > 0.0)
	{
		diffuse.rb = diffuse.br;
	}

	if (MAP_COLOR_SWITCH_GB > 0.0)
	{
		diffuse.gb = diffuse.bg;
	}

	if (diffuse.a > 0.5)
	{
#if 0
		float alpha = 1.0;
		float dist = distance(vVertPosition, u_ViewOrigin);
		float fadeStart = MAX_RANGE * 0.75;
		if (dist > fadeStart)
		{
			float fadeDiv = MAX_RANGE * 0.25;
			float fd = dist - fadeStart;
			alpha = 1.0 - clamp(fd / fadeDiv, 0.0, 1.0);
		}
		else if (dist <= 64.0)
		{
			alpha = clamp(dist / 64.0, 0.0, 1.0);
		}

		diffuse.a = alpha;
#else
		diffuse.a = 1.0;
#endif
	}
	else
	{
		diffuse.a = 0.0;
	}

	if (diffuse.a > 0.05/*0.5*/)
	{
		gl_FragColor = vec4(diffuse.rgb, diffuse.a/*1.0*/);
		out_Glow = vec4(0.0);
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
