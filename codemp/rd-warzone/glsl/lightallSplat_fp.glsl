//#define USE_DETAIL_TEXTURES
//#define __SPLATS_LOOKUP_ALPHA__			// Meh, waste of lookups... If we do need them at some point, then we can still enable them or add a mapinfo i guess...
//#define __USE_FULL_SPLAT_BLENDFUNC__		// Meh... fast should be nearly as good...
#define __HIGH_PASS_SHARPEN__
//#define __USE_REGIONS__


#define SNOW_HEIGHT_STRENGTH		0.25 // Distance above water to start snow...
#define SCREEN_MAPS_ALPHA_THRESHOLD 0.666
#define SCREEN_MAPS_LEAFS_THRESHOLD 0.0


uniform sampler2D					u_DiffuseMap;
uniform sampler2D					u_SteepMap;
uniform sampler2D					u_WaterEdgeMap;
uniform sampler2D					u_SplatControlMap;
uniform sampler2D					u_SplatMap1;
uniform sampler2D					u_SplatMap2;
uniform sampler2D					u_SplatMap3;

uniform sampler2D					u_RoadsControlMap;
uniform sampler2D					u_RoadMap;

#ifdef USE_DETAIL_TEXTURES
uniform sampler2D					u_DetailMap;
#endif //USE_DETAIL_TEXTURES

uniform sampler2D					u_GlowMap;

uniform sampler2D					u_LightMap;

uniform sampler2D					u_NormalMap;

uniform vec4						u_MapAmbient; // a basic light/color addition across the whole map...

uniform vec4						u_Settings0; // useTC, useDeform, useRGBA, isTextureClamped
uniform vec4						u_Settings1; // useVertexAnim, useSkeletalAnim, blendMethod, is2D
uniform vec4						u_Settings2; // LIGHTDEF_USE_LIGHTMAP, LIGHTDEF_USE_GLOW_BUFFER, LIGHTDEF_USE_CUBEMAP, LIGHTDEF_USE_TRIPLANAR
uniform vec4						u_Settings3; // LIGHTDEF_USE_REGIONS, LIGHTDEF_IS_DETAIL, 0=DetailMapNormal 1=detailMapFromTC 2=detailMapFromWorld, USE_GLOW_BLEND_MODE
uniform vec4						u_Settings4; // MAP_LIGHTMAP_MULTIPLIER, MAP_LIGHTMAP_ENHANCEMENT, 0.0, 0.0

#define USE_TC						u_Settings0.r
#define USE_DEFORM					u_Settings0.g
#define USE_RGBA					u_Settings0.b
#define USE_TEXTURECLAMP			u_Settings0.a

#define USE_VERTEX_ANIM				u_Settings1.r
#define USE_SKELETAL_ANIM			u_Settings1.g
#define USE_BLEND					u_Settings1.b
#define USE_IS2D					u_Settings1.a

#define USE_LIGHTMAP				u_Settings2.r
#define USE_GLOW_BUFFER				u_Settings2.g
#define USE_CUBEMAP					u_Settings2.b
#define USE_TRIPLANAR				u_Settings2.a

#define USE_REGIONS					u_Settings3.r
#define USE_ISDETAIL				u_Settings3.g
#define USE_EMISSIVE_BLACK			u_Settings3.b
#define USE_GLOW_BLEND_MODE			u_Settings3.a

#define MAP_LIGHTMAP_MULTIPLIER		u_Settings4.r
#define MAP_LIGHTMAP_ENHANCEMENT	u_Settings4.g


uniform vec4						u_Local1; // MAP_SIZE, sway, overlaySway, materialType
uniform vec4						u_Local2; // hasSteepMap, hasWaterEdgeMap, haveNormalMap, SHADER_WATER_LEVEL
uniform vec4						u_Local3; // hasSplatMap1, hasSplatMap2, hasSplatMap3, hasSplatMap4
uniform vec4						u_Local4; // stageNum, glowStrength, r_showsplat, glowVibrancy
uniform vec4						u_Local9; // testvalue0, 1, 2, 3

uniform vec2						u_Dimensions;
uniform vec2						u_textureScale;

uniform vec4						u_MapInfo; // MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], 0.0
uniform vec4						u_Mins;
uniform vec4						u_Maxs;

uniform float						u_Time;

#define MAP_MAX_HEIGHT				u_Maxs.b

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

#define SHADER_STAGE_NUM			u_Local4.r
#define SHADER_GLOW_STRENGTH		u_Local4.g
#define SHADER_SHOW_SPLAT			u_Local4.b
#define SHADER_GLOW_VIBRANCY		u_Local4.a


#if defined(USE_TESSELLATION) || defined(USE_ICR_CULLING)

in precise vec3				Normal_FS_in;
in precise vec2				TexCoord_FS_in;
in precise vec3				WorldPos_FS_in;
in precise vec3				ViewDir_FS_in;

in precise vec4				Color_FS_in;
in precise vec4				PrimaryLightDir_FS_in;
in precise vec2				TexCoord2_FS_in;

in precise vec3				Blending_FS_in;
flat in float				Slope_FS_in;


#define m_Normal 			normalize(Normal_FS_in.xyz)

#define m_TexCoords			TexCoord_FS_in
#define m_vertPos			WorldPos_FS_in
#define m_ViewDir			ViewDir_FS_in

#define var_Color			Color_FS_in
#define	var_PrimaryLightDir PrimaryLightDir_FS_in
#define var_TexCoords2		TexCoord2_FS_in

#define var_Blending		Blending_FS_in
#define var_Slope			Slope_FS_in


#else //!defined(USE_TESSELLATION) && !defined(USE_ICR_CULLING)

varying vec2				var_TexCoords;
varying vec2				var_TexCoords2;
varying vec3				var_Normal;

varying vec4				var_Color;

varying vec4				var_PrimaryLightDir;

varying vec3				var_vertPos;

varying vec3				var_ViewDir;


varying vec3				var_Blending;
varying float				var_Slope;


#define m_Normal			var_Normal
#define m_TexCoords			var_TexCoords
#define m_vertPos			var_vertPos
#define m_ViewDir			var_ViewDir


#endif //defined(USE_TESSELLATION) || defined(USE_ICR_CULLING)



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

void DepthContrast ( inout float depth )
{
	const float contrast = 3.0;
	const float brightness = 0.03;
	// Apply contrast.
	depth = ((depth - 0.5f) * max(contrast, 0)) + 0.5f;
	// Apply brightness.
	depth += brightness;
	depth = clamp(depth, 0.0, 1.0);
}

float GetDepthForPixel(vec4 color)
{
	if (color.a /** var_Color.a*/ <= 0.0)
	{
		return 0.0;
	}

	float displacement = clamp(max(max(color.r, color.g), color.b), 0.0, 1.0);
	DepthContrast(displacement); // Enhance the dark/lights...
	return (1.0 - displacement) * 0.9 + 0.1;
}

#ifdef USE_DETAIL_TEXTURES
void AddDetail(inout vec4 color, in vec2 tc)
{
	// Add fine detail to everything...
	vec2 coord = vec2(0.0);

	if (USE_DETAIL_COORD == 1.0 || USE_TEXTURECLAMP > 0.0 || USE_IS2D > 0.0)
	{// From TC... 1:1 match to diffuse coordinates... (good for guns/models/etc for adding detail)
		coord = tc;
	}
	else if (USE_DETAIL_COORD == 2.0 || USE_TRIPLANAR > 0.0 || USE_REGIONS > 0.0)
	{// From world... Using map coords like splatmaps... (good for splat mapping, etc for varying terrain shading)
		float xyoffset = (SHADER_MAP_SIZE - (SHADER_MAP_SIZE / 2.0)) / (SHADER_MAP_SIZE * 2.0);
		coord = vec2(m_vertPos.xy / (SHADER_MAP_SIZE / 2.0)) * xyoffset;
		coord *= (vec2(m_vertPos.z / (SHADER_MAP_SIZE / 2.0)) * xyoffset) * 2.0 - 1.0;
	}
	else
	{// Standard... -1.0 -> +1.0 (good all-round option when matching specific coordinates is not needed)
		coord = (tc * 2.0 - 1.0);
	}

    vec3 detail = texture(u_DetailMap, coord).rgb;

	if (length(detail.rgb) <= 0.0) return;

	color.rgb = color.rgb * detail.rgb * 2.0;
}
#endif //USE_DETAIL_TEXTURES

bool IsRoadmapMaterial ( void )
{
	if (SHADER_MATERIAL_TYPE == MATERIAL_SHORTGRASS || SHADER_MATERIAL_TYPE == MATERIAL_LONGGRASS || SHADER_MATERIAL_TYPE == MATERIAL_SAND || SHADER_MATERIAL_TYPE == MATERIAL_SNOW || SHADER_MATERIAL_TYPE == MATERIAL_MUD || SHADER_MATERIAL_TYPE == MATERIAL_CONCRETE)
	{
		return true;
	}

	return false;
}

vec4 GetControlMap( void )
{
	float scale = 1.0 / SHADER_MAP_SIZE; /* control scale */
	vec4 control;
	
#ifdef __USE_REGIONS__
	if (USE_REGIONS > 0.0)
	{
		// Try to verticalize the control map, so hopefully we can paint it in a more vertical way to get snowtop mountains, etc...
		float maxHeightOverWater = MAP_MAX_HEIGHT - SHADER_WATER_LEVEL;
		float currentheightOverWater = MAP_MAX_HEIGHT - m_vertPos.z;
		float y = pow(currentheightOverWater / maxHeightOverWater, SNOW_HEIGHT_STRENGTH);
		float xyoffset = (SHADER_MAP_SIZE - (SHADER_MAP_SIZE / 2.0)) / (SHADER_MAP_SIZE * 2.0);
		vec4 xaxis = texture( u_SplatControlMap, vec2((m_vertPos.x / (SHADER_MAP_SIZE / 2.0)) * xyoffset, y));
		vec4 yaxis = texture( u_SplatControlMap, vec2((m_vertPos.y / (SHADER_MAP_SIZE / 2.0)) * xyoffset, y));
		vec4 zaxis = texture( u_SplatControlMap, vec2((m_vertPos.z / (SHADER_MAP_SIZE / 2.0)) * xyoffset, y));
		control = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;
		control.rgb = clamp(control.rgb * 10.0, 0.0, 1.0);
	}
	else 
#endif //__USE_REGIONS__
	if (USE_TRIPLANAR >= 2.0)
	{
		control = vec4(var_Color.rgb, 0.0);
	}
	else
	{
		float offset = (SHADER_MAP_SIZE / 2.0) * scale;
		vec4 xaxis = texture( u_SplatControlMap, (m_vertPos.yz * scale) + offset);
		vec4 yaxis = texture( u_SplatControlMap, (m_vertPos.xz * scale) + offset);
		vec4 zaxis = texture( u_SplatControlMap, (m_vertPos.xy * scale) + offset);
		control = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;
		control.rgb = clamp(control.rgb * 10.0, 0.0, 1.0);
	}

	if (SHADER_HAS_SPLATMAP4 > 0.0 && IsRoadmapMaterial())
	{// Also grab the roads map, if we have one...
		vec2 mapSize = u_Maxs.xy - u_Mins.xy;
		vec2 pixel = (m_vertPos.xy - u_Mins.xy) / mapSize;
		float road = texture(u_RoadsControlMap, pixel).r;
		road = clamp(pow(road * 1.5, 2.0), 0.0, 1.0);
		control.a = road;
	}
	
	return control;
}

#ifdef __USE_FULL_SPLAT_BLENDFUNC__
vec3 splatblend(vec4 texture1, float a1, vec4 texture2, float a2)
{
    float depth = 0.2;
	float ma = max(texture1.a + a1, texture2.a + a2) - depth;

    float b1 = max(texture1.a + a1 - ma, 0);
    float b2 = max(texture2.a + a2 - ma, 0);

    return ((texture1.rgb * b1) + (texture2.rgb * b2)) / (b1 + b2);
}
#endif //__USE_FULL_SPLAT_BLENDFUNC__

#if defined(__HIGH_PASS_SHARPEN__)
vec3 Enhance(in sampler2D tex, in vec2 uv, vec3 color, float level)
{
	vec3 blur = textureLod(tex, uv, level).rgb;
	vec3 col = ((color - blur)*0.5 + 0.5) * 1.0;
	col *= ((color - blur)*0.25 + 0.25) * 8.0;
	col = col * color;
	return col;
}
#endif //defined(__HIGH_PASS_SHARPEN__)

vec4 GetMap( in sampler2D tex, float scale, inout float depth)
{
	vec4 xaxis;
	vec4 yaxis;
	vec4 zaxis;
	xaxis.a = 1.0;
	yaxis.a = 1.0;
	zaxis.a = 1.0;

	vec2 tScale = vec2(1.0) * scale;

	if (!(u_textureScale.x <= 0.0 && u_textureScale.y <= 0.0) && !(u_textureScale.x == 1.0 && u_textureScale.y == 1.0))
	{
		tScale *= u_textureScale;
	}

#ifdef __SPLATS_LOOKUP_ALPHA__
	xaxis = texture(tex, (m_vertPos.yz * tScale));
	yaxis = texture(tex, (m_vertPos.xz * tScale));
	zaxis = texture(tex, (m_vertPos.xy * tScale));
#else //!__SPLATS_LOOKUP_ALPHA__
	xaxis.rgb = texture(tex, (m_vertPos.yz * tScale)).rgb;
	yaxis.rgb = texture(tex, (m_vertPos.xz * tScale)).rgb;
	zaxis.rgb = texture(tex, (m_vertPos.xy * tScale)).rgb;
#endif //__SPLATS_LOOKUP_ALPHA__

#if defined(__HIGH_PASS_SHARPEN__)
	xaxis.rgb = Enhance(tex, (m_vertPos.yz * tScale), xaxis.rgb, 8.0 + (gl_FragCoord.z * 8.0));
	yaxis.rgb = Enhance(tex, (m_vertPos.xz * tScale), yaxis.rgb, 8.0 + (gl_FragCoord.z * 8.0));
	zaxis.rgb = Enhance(tex, (m_vertPos.xy * tScale), zaxis.rgb, 8.0 + (gl_FragCoord.z * 8.0));
#endif //defined(__HIGH_PASS_SHARPEN__)

	vec4 color = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;

	if (depth != -1.0)
	{// Only bother calculating if requested...
		depth = GetDepthForPixel(color);
	}

	return color;
}

vec4 QuickMix(vec3 color1, vec3 color2, float mix)
{
	float mixVal = clamp(mix, 0.0, 1.0);
	return vec4((color1 * (1.0 - mixVal)) + (color2 * mixVal), 1.0);
}

vec4 GetSplatMap(vec2 texCoords, vec4 inColor, inout float depth)
{
	if (SHADER_HAS_SPLATMAP1 <= 0.0 && SHADER_HAS_SPLATMAP2 <= 0.0 && SHADER_HAS_SPLATMAP3 <= 0.0 && SHADER_HAS_SPLATMAP4 <= 0.0)
	{
		return inColor;
	}

	if (SHADER_HAS_WATEREDGEMAP > 0.0 && m_vertPos.z <= SHADER_WATER_LEVEL)
	{// Steep maps (water edges)... Underwater doesn't use splats for now...
		return inColor;
	}

	// Splat blend in all the textues using the control strengths...

	vec4 splatColor = inColor;

	vec4 control = GetControlMap();

	float scale = 0.01;

#ifdef __USE_REGIONS__
	if (USE_REGIONS > 0.0)
	{
		scale = 0.001;
	}
#endif //__USE_REGIONS__

	if (SHADER_HAS_SPLATMAP1 > 0.0 && control.r > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap1, scale, depth);
		//splatColor = mix(splatColor, tex, control.r * tex.a);
		splatColor = QuickMix(splatColor.rgb, tex.rgb, control.r * tex.a);
	}

	if (SHADER_HAS_SPLATMAP2 > 0.0 && control.g > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap2, scale, depth);
		//splatColor = mix(splatColor, tex, control.g * tex.a);
		splatColor = QuickMix(splatColor.rgb, tex.rgb, control.g * tex.a);
	}

	if (SHADER_HAS_SPLATMAP3 > 0.0 && control.b > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap3, scale, depth);
		//splatColor = mix(splatColor, tex, control.b * tex.a);
		splatColor = QuickMix(splatColor.rgb, tex.rgb, control.b * tex.a);
	}

	if (SHADER_HAS_SPLATMAP4 > 0.0 && control.a > 0.0 && IsRoadmapMaterial())
	{
		vec4 tex = GetMap(u_RoadMap, scale, depth);
		splatColor = QuickMix(splatColor.rgb, tex.rgb, pow(control.a * 3.0, 0.5) * tex.a);
	}

	if (depth != -1.0)
	{// Only bother calculating if requested...
		depth = GetDepthForPixel(splatColor);
	}

	return splatColor;
}

vec4 GenerateTerrainMap(vec2 coord)
{
	// Splat mapping...
	float a1 = 0.0;
	float a2 = 0.0;
	vec4 tex1 = GetMap(u_DiffuseMap, 0.0075, a1);
	vec4 tex2 = GetSplatMap(coord, tex1, a2);

	float maxHeightOverWater = MAP_MAX_HEIGHT - SHADER_WATER_LEVEL;
	float currentheightOverWater = MAP_MAX_HEIGHT - m_vertPos.z;
	float mixVal = 1.0 - clamp(pow(currentheightOverWater / maxHeightOverWater, SNOW_HEIGHT_STRENGTH), 0.0, 1.0);

	mixVal *= -32.0;//u_Local9.r;

#ifdef __USE_FULL_SPLAT_BLENDFUNC__
	return vec4(splatblend(tex1, a1 * (a1 * mixVal), tex2, a2 * (1.0 - (a2 * mixVal))), 1.0);
#else //!__USE_FULL_SPLAT_BLENDFUNC__
	return QuickMix(tex1.rgb, tex2.rgb, 1.0 - (mixVal * a1));
#endif //__USE_FULL_SPLAT_BLENDFUNC__
}

vec4 GetDiffuse(vec2 texCoords, float pixRandom)
{
	if (USE_REGIONS > 0.0 || USE_TRIPLANAR > 0.0)
	{
		if (SHADER_SHOW_SPLAT > 0.0)
		{
			vec4 control = vec4(0.0);
			
			if (USE_TRIPLANAR >= 2.0)
			{
				return vec4(var_Color.rgb, 1.0);
			}

			if (SHADER_SHOW_SPLAT > 7.0)
			{
				control = texture(u_RoadMap, texCoords);
			}
			else if (SHADER_SHOW_SPLAT > 6.0)
			{
				control = texture(u_WaterEdgeMap, texCoords);
			}
			else if (SHADER_SHOW_SPLAT > 5.0)
			{
				control = texture(u_SteepMap, texCoords);
			}
			else if (SHADER_SHOW_SPLAT > 4.0)
			{
				control = texture(u_SplatMap3, texCoords);
			}
			else if (SHADER_SHOW_SPLAT > 3.0)
			{
				control = texture(u_SplatMap2, texCoords);
			}
			else if (SHADER_SHOW_SPLAT > 2.0)
			{
				control = texture(u_SplatMap1, texCoords);
			}
			else if (SHADER_SHOW_SPLAT > 1.0)
			{
				control = GetControlMap();
				control.rgb = control.aaa;
			}
			else
			{
				control = GetControlMap();
			}
		
			return vec4(control.rgb, 1.0);
		}

#ifdef __USE_REGIONS__
		if (USE_REGIONS > 0.0 
			&& (SHADER_HAS_SPLATMAP1 > 0 || SHADER_HAS_SPLATMAP2 > 0 || SHADER_HAS_SPLATMAP3 > 0 || SHADER_HAS_SPLATMAP4 > 0))
		{// Regions...
			return GenerateTerrainMap(texCoords);
		}
		else
#endif //__USE_REGIONS__
		{// Tri-Planar...
			if (SHADER_HAS_WATEREDGEMAP > 0.0 && m_vertPos.z <= SHADER_WATER_LEVEL + 128.0 + (64.0 * pixRandom))
			{// Steep maps (water edges)...
				float maxAbove = 128.0 + (64.0 * pixRandom);
				float mixVal = clamp(((SHADER_WATER_LEVEL + maxAbove) - m_vertPos.z) / maxAbove, 0.0, 1.0);

				float a1 = 0.0;
				float a2 = 0.0;
				vec4 tex1 = GetMap(u_WaterEdgeMap, 0.0075, a1);
				vec4 tex2 = GetMap(u_DiffuseMap, 0.0075, a2);

				if (SHADER_HAS_SPLATMAP1 <= 0.0 && SHADER_HAS_SPLATMAP2 <= 0.0 && SHADER_HAS_SPLATMAP3 <= 0.0 && SHADER_HAS_SPLATMAP4 <= 0.0)
				{// No splat maps...
#ifdef __USE_FULL_SPLAT_BLENDFUNC__
					return vec4(splatblend(tex1, a1 * (a1 * mixVal), tex2, a2 * (1.0 - (a2 * mixVal))), 1.0);
#else //!__USE_FULL_SPLAT_BLENDFUNC__
					return QuickMix(tex1.rgb, tex2.rgb, 1.0 - (mixVal * a1));
#endif //__USE_FULL_SPLAT_BLENDFUNC__
				}

				// Splat mapping...
				float a3 = 0.0;
				vec4 tex3 = GetSplatMap(texCoords, tex2, a3);

				a1 = 1.0 - a1;
				a3 = 1.0 - a3;

#ifdef __USE_FULL_SPLAT_BLENDFUNC__
				return vec4(splatblend(tex1, a1 * (a1 * mixVal), tex3, a3 * (1.0 - (a3 * mixVal))), 1.0);
#else //!__USE_FULL_SPLAT_BLENDFUNC__
				return QuickMix(tex1.rgb, tex3.rgb, 1.0 - (mixVal * a1));
#endif //__USE_FULL_SPLAT_BLENDFUNC__
			}
			else if (USE_TRIPLANAR >= 2.0 && (SHADER_HAS_SPLATMAP1 > 0.0 || SHADER_HAS_SPLATMAP2 > 0.0 || SHADER_HAS_SPLATMAP3 > 0.0 || (SHADER_HAS_SPLATMAP4 > 0.0 && IsRoadmapMaterial())))
			{// Steep maps (using vertex colors)...
				// Splat mapping...
				float a1 = 0.0;
				vec4 tex = GetMap(u_DiffuseMap, 0.0075, a1);
				return GetSplatMap(texCoords, tex, a1);
			}
			else if (SHADER_HAS_STEEPMAP > 0.0 && var_Slope > 0)
			{// Steep maps (high angles)...
				float a1 = -1.0;
				return GetMap(u_SteepMap, 0.0025, a1);
			}
			else if (SHADER_HAS_SPLATMAP1 > 0.0 || SHADER_HAS_SPLATMAP2 > 0.0 || SHADER_HAS_SPLATMAP3 > 0.0 || (SHADER_HAS_SPLATMAP4 > 0.0 && IsRoadmapMaterial()))
			{// Steep maps (low angles)...
				// Splat mapping...
				float a1 = 0.0;
				vec4 tex = GetMap(u_DiffuseMap, 0.0075, a1);
				return GetSplatMap(texCoords, tex, a1);
			}
			else
			{
				float a1 = -1.0;
				return GetMap(u_DiffuseMap, 0.0075, a1);
			}
		}
	}
	else
	{
		return texture(u_DiffuseMap, texCoords);
	}
}

vec3 vLocalSeed;

// This function returns random number from zero to one
float randZeroOne()
{
    uint n = floatBitsToUint(vLocalSeed.y * 214013.0 + vLocalSeed.x * 2531011.0 + vLocalSeed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;

    float fRes =  2.0 - uintBitsToFloat(n);
    vLocalSeed = vec3(vLocalSeed.x + 147158.0 * fRes, vLocalSeed.y*fRes  + 415161.0 * fRes, vLocalSeed.z + 324154.0*fRes);
    return fRes;
}

const float							fBranchHardiness = 0.001;
const float							fBranchSize = 128.0;
const float							fWindStrength = 12.0;
const vec3							vWindDirection = normalize(vec3(1.0, 1.0, 0.0));

vec2 GetSway ()
{
	// Wind calculation stuff...
	float fWindPower = 0.5f + sin(m_vertPos.x / fBranchSize + m_vertPos.z / fBranchSize + u_Time*(1.2f + fWindStrength / fBranchSize/*20.0f*/));

	if (fWindPower < 0.0f)
		fWindPower = fWindPower*0.2f;
	else
		fWindPower = fWindPower*0.3f;

	fWindPower *= fWindStrength;

	return vWindDirection.xy*fWindPower*fBranchHardiness;
}

vec3 Vibrancy ( vec3 origcolor, float vibrancyStrength )
{
	vec3	lumCoeff = vec3(0.212656, 0.715158, 0.072186);  				//Calculate luma with these values
	float	max_color = max(origcolor.r, max(origcolor.g,origcolor.b)); 	//Find the strongest color
	float	min_color = min(origcolor.r, min(origcolor.g,origcolor.b)); 	//Find the weakest color
	float	color_saturation = max_color - min_color; 						//Saturation is the difference between min and max
	float	luma = dot(lumCoeff, origcolor.rgb); 							//Calculate luma (grey)
	return mix(vec3(luma), origcolor.rgb, (1.0 + (vibrancyStrength * (1.0 - (sign(vibrancyStrength) * color_saturation))))); 	//Extrapolate between luma and original by 1 + (1-saturation) - current
}

float getspecularLight(vec3 n, vec3 l, vec3 e, float s) {
	//float nrm = (s + 8.0) / (3.1415 * 8.0);
	float ndotl = clamp(max(dot(reflect(e, n), l), 0.0), 0.1, 1.0);
	return clamp(pow(ndotl, s), 0.1, 1.0);// * nrm;
}

float getdiffuseLight(vec3 n, vec3 l, float p) {
	float ndotl = clamp(dot(n, l), 0.5, 0.9);
	return pow(ndotl, p);
}

void main()
{
	bool LIGHTMAP_ENABLED = (USE_LIGHTMAP > 0.0 && USE_GLOW_BUFFER != 1.0 && USE_IS2D <= 0.0) ? true : false;

	vec2 texCoords = m_TexCoords.xy;
	float pixRandom = 0.0;

	if (USE_TRIPLANAR > 0.0 && SHADER_HAS_WATEREDGEMAP > 0.0 && m_vertPos.z <= SHADER_WATER_LEVEL + 192.0)
	{// Only water edge map surfaces use this atm... Other stuff can skip calculation...
		vLocalSeed = m_vertPos.xyz;
		pixRandom = randZeroOne();
	}

#if 0 // Shouldn't need this on splatmap draws...
	if (SHADER_SWAY > 0.0)
	{// Sway...
		texCoords += vec2(GetSway());
	}
#endif

	vec4 colorMap = var_Color;

	if (USE_TRIPLANAR >= 2.0)
	{
		colorMap = vec4(1.0);
	}

	vec4 diffuse = GetDiffuse(texCoords, pixRandom);

	// Set alpha early so that we can cull early...
	gl_FragColor.a = clamp(diffuse.a * colorMap.a, 0.0, 1.0);


	vec3 N = normalize(m_Normal.xyz);

#ifdef __USE_REAL_NORMALMAPS__
	vec4 norm = vec4(0.0);

#if 0 // Shouldn't need this on splatmap draws...
	if (USE_GLOW_BUFFER != 1.0 && USE_IS2D <= 0.0 && USE_ISDETAIL <= 0.0 && USE_TRIPLANAR <= 0.0 && USE_REGIONS <= 0.0 && SHADER_HAS_NORMALMAP > 0.0)
	{
		norm = texture(u_NormalMap, texCoords);
		norm.a = 1.0;
	}
#endif
#endif //__USE_REAL_NORMALMAPS__


#ifdef USE_DETAIL_TEXTURES
	if (USE_TRIPLANAR >= 0.0 || USE_REGIONS >= 0.0)
	{
		AddDetail(diffuse, texCoords);
	}
#endif //USE_DETAIL_TEXTURES


	vec3 ambientColor = vec3(0.0);
	vec3 lightColor = clamp(colorMap.rgb, 0.0, 1.0);


	if (LIGHTMAP_ENABLED)
	{// TODO: Move to screen space?
		vec4 lightmapColor = textureLod(u_LightMap, var_TexCoords2.st, 0.0);

#if defined(RGBM_LIGHTMAP)
		lightmapColor.rgb *= lightmapColor.a;
#endif //defined(RGBM_LIGHTMAP)

		if (MAP_LIGHTMAP_ENHANCEMENT <= 0.0)
		{
			float lmBrightMult = clamp(1.0 - (length(lightmapColor.rgb) / 3.0), 0.0, 0.9);

#define lm_const_1 ( 56.0 / 255.0)
#define lm_const_2 (255.0 / 200.0)
			lmBrightMult = clamp((clamp(lmBrightMult - lm_const_1, 0.0, 1.0)) * lm_const_2, 0.0, 1.0);
			lmBrightMult = lmBrightMult * 0.7;

			lmBrightMult *= MAP_LIGHTMAP_MULTIPLIER;

			lightColor = lightmapColor.rgb * lmBrightMult;

			ambientColor = lightColor;
			float surfNL = clamp(dot(var_PrimaryLightDir.xyz, N.xyz), 0.0, 1.0);
			lightColor /= clamp(max(surfNL, 0.25), 0.0, 1.0);
			ambientColor = clamp(ambientColor - lightColor * surfNL, 0.0, 1.0);
			lightColor *= lightmapColor.rgb;
		}
		else if (MAP_LIGHTMAP_ENHANCEMENT == 1.0)
		{
			// Old style...
			float lmBrightMult = clamp(1.0 - (length(lightmapColor.rgb) / 3.0), 0.0, 0.9);

#define lm_const_1 ( 56.0 / 255.0)
#define lm_const_2 (255.0 / 200.0)
			lmBrightMult = clamp((clamp(lmBrightMult - lm_const_1, 0.0, 1.0)) * lm_const_2, 0.0, 1.0);
			lmBrightMult = lmBrightMult * 0.7;

			lmBrightMult *= MAP_LIGHTMAP_MULTIPLIER;

			lightColor = lightmapColor.rgb * lmBrightMult;

			ambientColor = lightColor;
			float surfNL = clamp(dot(var_PrimaryLightDir.xyz, N.xyz), 0.0, 1.0);
			lightColor /= clamp(max(surfNL, 0.25), 0.0, 1.0);
			ambientColor = clamp(ambientColor - lightColor * surfNL, 0.0, 1.0);
			lightColor *= lightmapColor.rgb;


			// New style...
			vec3 lightColor2 = lightmapColor.rgb * MAP_LIGHTMAP_MULTIPLIER;

			vec3 E = normalize(m_ViewDir);

			vec3 bNorm = normalize(N.xyz + ((diffuse.rgb * 2.0 - 1.0) * -0.25)); // just add some fake bumpiness to it, fast as possible...

			float dif = min(getdiffuseLight(bNorm, var_PrimaryLightDir.xyz, 0.3), 0.15);
			vec3 ambientColor2 = clamp(dif * lightColor2, 0.0, 1.0);

			float spec = clamp(getspecularLight(bNorm, -var_PrimaryLightDir.xyz, E, 16.0), 0.05, 0.125);
			lightColor2 = clamp(spec * lightColor2, 0.0, 1.0);


			// Mix them 50/50
			lightColor = (lightColor + lightColor2) * 0.5;
			ambientColor = (ambientColor + ambientColor2) * 0.5;
		}
		else
		{
			lightColor = lightmapColor.rgb * MAP_LIGHTMAP_MULTIPLIER;

			vec3 E = normalize(m_ViewDir);

			vec3 bNorm = normalize(N.xyz + ((diffuse.rgb * 2.0 - 1.0) * -0.25)); // just add some fake bumpiness to it, fast as possible...

			float dif = min(getdiffuseLight(bNorm, var_PrimaryLightDir.xyz, 0.3), 0.15);
			ambientColor = clamp(dif * lightColor, 0.0, 1.0);

			float spec = clamp(getspecularLight(bNorm, -var_PrimaryLightDir.xyz, E, 16.0), 0.05, 0.125);
			lightColor = clamp(spec * lightColor, 0.0, 1.0);
		}
	}


	gl_FragColor.rgb = diffuse.rgb + ambientColor;


	if (USE_GLOW_BUFFER != 1.0 && USE_IS2D <= 0.0 && SHADER_MATERIAL_TYPE != MATERIAL_SKY && SHADER_MATERIAL_TYPE != MATERIAL_SUN && SHADER_MATERIAL_TYPE != MATERIAL_GLASS)
	{
		gl_FragColor.rgb = gl_FragColor.rgb * u_MapAmbient.rgb;
	}
	

	gl_FragColor.rgb *= clamp(lightColor, 0.0, 1.0);


#if 0 // Shouldn't need this on splatmap draws...
	if (USE_BLEND > 0.0)
	{// Emulate RGB blending... Fuck I hate this crap...
		float colStr = clamp(max(gl_FragColor.r, max(gl_FragColor.g, gl_FragColor.b)), 0.0, 1.0);

		if (USE_BLEND == 3.0)
		{
			gl_FragColor.a *= colStr * 2.0;
			gl_FragColor.rgb *= 0.5;
		}
		else if (USE_BLEND == 2.0)
		{
			colStr = clamp(colStr + 0.1, 0.0, 1.0);
			gl_FragColor.a = 1.0 - colStr;
		}
		else
		{
			colStr = clamp(colStr - 0.1, 0.0, 1.0);
			gl_FragColor.a = colStr;
		}
	}
#endif

	gl_FragColor.a = clamp(gl_FragColor.a, 0.0, 1.0);

	if (USE_EMISSIVE_BLACK > 0.0 && USE_GLOW_BUFFER <= 0.0)
	{
		gl_FragColor.rgb = vec3(0.0);
	}

	float maxColor = max(gl_FragColor.r, max(gl_FragColor.g, gl_FragColor.b));

	if (/*USE_GLOW_BUFFER > 1.0 
		&&*/ SHADER_MATERIAL_TYPE == MATERIAL_ROCK 
		&& var_Slope > 0 
		&& SHADER_HAS_SPLATMAP1 <= 0 
		&& SHADER_HAS_SPLATMAP2 <= 0 
		&& SHADER_HAS_SPLATMAP3 <= 0 
		&& SHADER_HAS_SPLATMAP4 <= 0 
		&& maxColor > 0.1)
	{// Hmm using for city buildings...
		vec4 glowColor = gl_FragColor;

		if (SHADER_GLOW_VIBRANCY != 0.0)
		{
			glowColor.rgb = Vibrancy( glowColor.rgb, SHADER_GLOW_VIBRANCY );
		}

#define glow_const_1 ( 23.0 / 255.0)
#define glow_const_2 (255.0 / 229.0)
		glowColor.rgb = clamp((clamp(glowColor.rgb - glow_const_1, 0.0, 1.0)) * glow_const_2, 0.0, 1.0);
		glowColor.rgb *= SHADER_GLOW_STRENGTH;

		//float glowMax = clamp(length(glowColor.rgb) / 3.0, 0.0, 1.0);//clamp(max(glowColor.r, max(glowColor.g, glowColor.b)), 0.0, 1.0);
		//glowColor.a *= glowMax;
		//glowColor.rgb *= glowColor.a;

		//gl_FragColor.rgb = mix(gl_FragColor.rgb, glowColor.rgb, glowColor.a);
		//gl_FragColor.a = max(gl_FragColor.a, glowColor.a);

		//float gMax = max(glowColor.r, max(glowColor.g, glowColor.b));
		//if (gMax > 1.0)
		//	glowColor.rgb = glowColor.rgb / gMax;

		glowColor.a = clamp(glowColor.a, 0.0, 1.0);
		out_Glow = glowColor;

		gl_FragColor.rgb = mix(gl_FragColor.rgb, glowColor.rgb, glowColor.a);
		gl_FragColor.a = max(gl_FragColor.a, glowColor.a);

		out_Position = vec4(m_vertPos.xyz, SHADER_MATERIAL_TYPE+1.0);
		out_Normal = vec4(vec3(EncodeNormal(N.xyz), 1.0), 1.0 );
#ifdef __USE_REAL_NORMALMAPS__
		out_NormalDetail = norm;
#endif //__USE_REAL_NORMALMAPS__
	}
	else
	{
		out_Glow = vec4(0.0);

		out_Position = vec4(m_vertPos.xyz, SHADER_MATERIAL_TYPE+1.0);
		out_Normal = vec4( vec3(EncodeNormal(N.xyz), 1.0), 1.0 );
#ifdef __USE_REAL_NORMALMAPS__
		out_NormalDetail = norm;
#endif //__USE_REAL_NORMALMAPS__
	}
}
