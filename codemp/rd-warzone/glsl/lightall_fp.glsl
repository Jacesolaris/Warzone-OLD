//#define UI_ENHANCEMENT
//#define USE_ALPHA_TEST
#define USE_GLOW_DETAIL_BUFFERS
//#define USE_ALPHA_MULT
//#define USE_DETAIL


#define SNOW_HEIGHT_STRENGTH 0.25 // Distance above water to start snow...


uniform sampler2D			u_DiffuseMap;
uniform sampler2D			u_SteepMap;
uniform sampler2D			u_WaterEdgeMap;
uniform sampler2D			u_SplatControlMap;
uniform sampler2D			u_SplatMap1;
uniform sampler2D			u_SplatMap2;
uniform sampler2D			u_SplatMap3;
//uniform sampler2D			u_SplatMap4;
uniform sampler2D			u_DetailMap;

uniform sampler2D			u_LightMap;

uniform sampler2D			u_NormalMap;
uniform sampler2D			u_NormalMap2;
uniform sampler2D			u_NormalMap3;

uniform sampler2D			u_DeluxeMap;

uniform sampler2D			u_SpecularMap;

uniform sampler2D			u_ShadowMap;

#define textureCubeLod textureLod // UQ1: > ver 140 support
uniform samplerCube			u_CubeMap;

uniform sampler2D			u_OverlayMap;

uniform vec4				u_MapAmbient; // a basic light/color addition across the whole map...

uniform vec4				u_Settings0; // useTC, useDeform, useRGBA, isTextureClamped
uniform vec4				u_Settings1; // useVertexAnim, useSkeletalAnim, useFog, is2D
uniform vec4				u_Settings2; // LIGHTDEF_USE_LIGHTMAP, LIGHTDEF_USE_GLOW_BUFFER, LIGHTDEF_USE_CUBEMAP, LIGHTDEF_USE_TRIPLANAR
uniform vec4				u_Settings3; // LIGHTDEF_USE_REGIONS, LIGHTDEF_IS_DETAIL, 0=DetailMapNormal 1=detailMapFromTC 2=detailMapFromWorld, PARALLAX_MODE

#define USE_TC				u_Settings0.r
#define USE_DEFORM			u_Settings0.g
#define USE_RGBA			u_Settings0.b
#define USE_TEXTURECLAMP	u_Settings0.a

#define USE_VERTEX_ANIM		u_Settings1.r
#define USE_SKELETAL_ANIM	u_Settings1.g
#define USE_FOG				u_Settings1.b
#define USE_IS2D			u_Settings1.a

#define USE_LIGHTMAP		u_Settings2.r
#define USE_GLOW_BUFFER		u_Settings2.g
#define USE_CUBEMAP			u_Settings2.b
#define USE_TRIPLANAR		u_Settings2.a

#define USE_REGIONS			u_Settings3.r
#define USE_ISDETAIL		u_Settings3.g
#define USE_DETAIL_COORD	u_Settings3.b
#define PARALLAX_MODE		u_Settings3.a


uniform vec2				u_Dimensions;
uniform vec4				u_Local1; // parallaxScale, haveSpecular, specularScale, materialType
uniform vec4				u_Local2; // ExtinctionCoefficient
uniform vec4				u_Local3; // 0, 0, r_cubemapCullRange->value, cubemapScale
uniform vec4				u_Local4; // haveNormalMap, isMetalic, hasRealSubsurfaceMap, sway
uniform vec4				u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap
uniform vec4				u_Local6; // useSunLightSpecular, hasWaterEdgeMap, MAP_SIZE, WATER_LEVEL
uniform vec4				u_Local7; // hasSplatMap1, hasSplatMap2, hasSplatMap3, hasSplatMap4
uniform vec4				u_Local8; // stageNum, glowStrength, MAP_INFO_MAXS[2], r_showsplat
uniform vec4				u_Local9; // testvalue0, 1, 2, 3

#define UI_VIBRANCY			0.4
#define MAP_MAX_HEIGHT		u_Local8.b
#define WATER_LEVEL			u_Local6.a
#define cubeStrength		u_Local3.a
#define cubeMaxDist			u_Local3.b

uniform mat4				u_ModelViewProjectionMatrix;
uniform mat4				u_ModelMatrix;
uniform mat4				u_invEyeProjectionMatrix;
uniform mat4				u_ModelViewMatrix;

uniform vec4				u_EnableTextures;

uniform vec4				u_PrimaryLightOrigin;
uniform vec3				u_PrimaryLightColor;
uniform vec3				u_PrimaryLightAmbient;

uniform vec4				u_NormalScale;
uniform vec4				u_SpecularScale;

uniform vec4				u_CubeMapInfo;
uniform float				u_CubeMapStrength;

uniform vec3				u_ViewOrigin;

uniform vec2				u_textureScale;

uniform int					u_ColorGen;
uniform int					u_AlphaGen;

uniform vec2				u_AlphaTestValues;

#define ATEST_NONE	0
#define ATEST_LT	1
#define ATEST_GT	2
#define ATEST_GE	3


#if defined(USE_TESSELLATION) || defined(USE_ICR_CULLING)

in precise vec3				Normal_FS_in;
in precise vec2				TexCoord_FS_in;
in precise vec3				WorldPos_FS_in;
in precise vec3				ViewDir_FS_in;
in precise vec4				Tangent_FS_in;
in precise vec4				Bitangent_FS_in;

in precise vec4				Color_FS_in;
in precise vec4				PrimaryLightDir_FS_in;
in precise vec2				TexCoord2_FS_in;

in precise vec3				Blending_FS_in;
flat in float				Slope_FS_in;
flat in float				usingSteepMap_FS_in;


#define m_Normal 			normalize(Normal_FS_in.xyz)

#define m_TexCoords			TexCoord_FS_in
#define m_vertPos			WorldPos_FS_in
#define m_ViewDir			ViewDir_FS_in

#define var_Normal2			ViewDir_FS_in.x

#define var_Tangent			Tangent_FS_in
#define var_Bitangent		Bitangent_FS_in

#define var_nonTCtexCoords	TexCoord_FS_in
#define var_Color			Color_FS_in
#define	var_PrimaryLightDir PrimaryLightDir_FS_in
#define var_TexCoords2		TexCoord2_FS_in

#define var_Blending		Blending_FS_in
#define var_Slope			Slope_FS_in
#define var_usingSteepMap	usingSteepMap_FS_in


#else //!defined(USE_TESSELLATION) && !defined(USE_ICR_CULLING)

varying vec2				var_TexCoords;
varying vec2				var_TexCoords2;
varying vec4				var_Normal;

#define var_Normal2			var_Normal.w

varying vec4				var_Tangent;
varying vec4				var_Bitangent;
varying vec4				var_Color;

varying vec4				var_PrimaryLightDir;

varying vec3				var_vertPos;

varying vec3				var_ViewDir;
varying vec2				var_nonTCtexCoords; // for steep maps


varying vec3				var_Blending;
varying float				var_Slope;
varying float				var_usingSteepMap;


#define m_Normal			var_Normal
#define m_TexCoords			var_TexCoords
#define m_vertPos			var_vertPos
#define m_ViewDir			var_ViewDir


#endif //defined(USE_TESSELLATION) || defined(USE_ICR_CULLING)



out vec4 out_Glow;
out vec4 out_Position;
out vec4 out_Normal;
out vec4 out_NormalDetail;

vec2 encode (vec3 n)
{
    float p = sqrt(n.z*8+8);
    return vec2(n.xy/p + 0.5);
}

vec3 decode (vec2 enc)
{
    vec2 fenc = enc*4-2;
    float f = dot(fenc,fenc);
    float g = sqrt(1-f/4);
    vec3 n;
    n.xy = fenc*g;
    n.z = 1-f/2;
    return n;
}

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
	if (color.a * var_Color.a <= 0.0)
	{
		return 0.0;
	}

	float displacement = clamp(max(max(color.r, color.g), color.b), 0.0, 1.0);
	DepthContrast(displacement); // Enhance the dark/lights...
	return 1.0 - clamp(displacement, 0.0, 1.0);
}

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
		float xyoffset = (u_Local6.b - (u_Local6.b / 2.0)) / (u_Local6.b * 2.0);
		coord = vec2(m_vertPos.xy / (u_Local6.b / 2.0)) * xyoffset;
		coord *= (vec2(m_vertPos.z / (u_Local6.b / 2.0)) * xyoffset) * 2.0 - 1.0;
	}
	else
	{// Standard... -1.0 -> +1.0 (good all-round option when matching specific coordinates is not needed)
		coord = (tc * 2.0 - 1.0);
	}

    vec3 detail = texture(u_DetailMap, coord).rgb;

	if (length(detail.rgb) <= 0.0) return;

	color.rgb = color.rgb * detail.rgb * 2.0;
}

vec4 GetControlMap( void )
{
	float scale = 1.0 / u_Local6.b; /* control scale */
	vec4 control;
	
	if (USE_REGIONS > 0.0)
	{
		// Try to verticalize the control map, so hopefully we can paint it in a more vertical way to get snowtop mountains, etc...
		float maxHeightOverWater = MAP_MAX_HEIGHT - WATER_LEVEL;
		float currentheightOverWater = MAP_MAX_HEIGHT - m_vertPos.z;
		float y = pow(currentheightOverWater / maxHeightOverWater, SNOW_HEIGHT_STRENGTH);
		float xyoffset = (u_Local6.b - (u_Local6.b / 2.0)) / (u_Local6.b * 2.0);
		vec4 xaxis = texture( u_SplatControlMap, vec2((m_vertPos.x / (u_Local6.b / 2.0)) * xyoffset, y));
		vec4 yaxis = texture( u_SplatControlMap, vec2((m_vertPos.y / (u_Local6.b / 2.0)) * xyoffset, y));
		vec4 zaxis = texture( u_SplatControlMap, vec2((m_vertPos.z / (u_Local6.b / 2.0)) * xyoffset, y));
		control = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;
	}
	else
	{
		float offset = (u_Local6.b / 2.0) * scale;
		vec4 xaxis = texture( u_SplatControlMap, (m_vertPos.yz * scale) + offset);
		vec4 yaxis = texture( u_SplatControlMap, (m_vertPos.xz * scale) + offset);
		vec4 zaxis = texture( u_SplatControlMap, (m_vertPos.xy * scale) + offset);
		control = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;
	}

	control = clamp(control * 10.0, 0.0, 1.0);
	
	return control;
}

vec3 splatblend(vec4 texture1, float a1, vec4 texture2, float a2)
{
    float depth = 0.2;
    float ma = max(texture1.a + a1, texture2.a + a2) - depth;

    float b1 = max(texture1.a + a1 - ma, 0);
    float b2 = max(texture2.a + a2 - ma, 0);

    return (texture1.rgb * b1 + texture2.rgb * b2) / (b1 + b2);
}

vec4 GetMap( in sampler2D tex, float scale, inout float depth)
{
	vec4 xaxis;
	vec4 yaxis;
	vec4 zaxis;

	vec2 tScale = vec2(1.0);

	if (!(u_textureScale.x <= 0.0 && u_textureScale.y <= 0.0) && !(u_textureScale.x == 1.0 && u_textureScale.y == 1.0))
	{
		tScale *= u_textureScale;
	}

	xaxis = texture( tex, (m_vertPos.yz * tScale * scale));
	yaxis = texture( tex, (m_vertPos.xz * tScale * scale));
	zaxis = texture( tex, (m_vertPos.xy * tScale * scale));

	vec4 color = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;

	if (depth != -1.0)
	{// Only bother calculating if requested...
		depth = GetDepthForPixel(color);
	}

	return color;
}

vec4 GetSplatMap(vec2 texCoords, vec4 inColor, inout float depth)
{
	if (u_Local7.r <= 0.0 && u_Local7.g <= 0.0 && u_Local7.b <= 0.0 && u_Local7.a <= 0.0)
	{
		return inColor;
	}

	if (u_Local6.g > 0.0 && m_vertPos.z <= WATER_LEVEL)
	{// Steep maps (water edges)... Underwater doesn't use splats for now...
		return inColor;
	}

	// Splat blend in all the textues using the control strengths...

	vec4 splatColor = inColor;

	vec4 control = GetControlMap();

	float scale = 0.01;

	if (USE_REGIONS > 0.0)
	{
		scale = 0.001;
	}

	if (u_Local7.r > 0.0 && control.r > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap1, scale, depth);
		splatColor = mix(splatColor, tex, control.r * tex.a);
	}

	if (u_Local7.g > 0.0 && control.g > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap2, scale, depth);
		splatColor = mix(splatColor, tex, control.g * tex.a);
	}

	if (u_Local7.b > 0.0 && control.b > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap3, scale, depth);
		splatColor = mix(splatColor, tex, control.b * tex.a);
	}

	/*if (u_Local7.a > 0.0 && control.a > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap4, scale, depth);
		splatColor = mix(splatColor, tex, control.a * tex.a);
	}*/

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

	float maxHeightOverWater = MAP_MAX_HEIGHT - WATER_LEVEL;
	float currentheightOverWater = MAP_MAX_HEIGHT - m_vertPos.z;
	float mixVal = 1.0 - pow(currentheightOverWater / maxHeightOverWater, SNOW_HEIGHT_STRENGTH);

	mixVal *= -32.0;//u_Local9.r;

	return vec4(splatblend(tex1, a1 * (a1 * mixVal), tex2, a2 * (1.0 - (a2 * mixVal))), 1.0);
}

vec4 GetDiffuse(vec2 texCoords, float pixRandom)
{
	if (USE_REGIONS > 0.0 || USE_TRIPLANAR > 0.0)
	{
		if (u_Local8.a > 0.0)
		{
			vec4 control = vec4(0.0);

			if (u_Local8.a > 5.0)
			{
				control = texture(u_WaterEdgeMap, texCoords);
			}
			else if (u_Local8.a > 4.0)
			{
				control = texture(u_SteepMap, texCoords);
			}
			else if (u_Local8.a > 3.0)
			{
				control = texture(u_SplatMap3, texCoords);
			}
			else if (u_Local8.a > 2.0)
			{
				control = texture(u_SplatMap2, texCoords);
			}
			else if (u_Local8.a > 1.0)
			{
				control = texture(u_SplatMap1, texCoords);
			}
			else
			{
				control = GetControlMap();
			}
		
			return vec4(control.rgb, 1.0);
		}

		if (USE_REGIONS > 0.0)
		{// Regions...
			return GenerateTerrainMap(texCoords);
		}
		else
		{// Tri-Planar...
			if (u_Local6.g > 0.0 && m_vertPos.z <= WATER_LEVEL + 128.0 + (64.0 * pixRandom))
			{// Steep maps (water edges)...
				float mixVal = ((WATER_LEVEL + 128.0) - m_vertPos.z) / 128.0;

				float a1 = 0.0;
				float a2 = 0.0;
				vec4 tex1 = GetMap(u_WaterEdgeMap, 0.0075, a1);
				vec4 tex2 = GetMap(u_DiffuseMap, 0.0075, a2);

				if (u_Local7.r <= 0.0 && u_Local7.g <= 0.0 && u_Local7.b <= 0.0 && u_Local7.a <= 0.0)
				{// No splat maps...
					return vec4(splatblend(tex1, a1 * (a1 * mixVal), tex2, a2 * (1.0 - (a2 * mixVal))), 1.0);
				}

				// Splat mapping...
				tex2 = GetSplatMap(texCoords, tex2, a2);

				a1 = 1.0 - a1;
				a2 = 1.0 - a2;

				return vec4(splatblend(tex1, a1 * (a1 * mixVal), tex2, a2 * (1.0 - (a2 * mixVal))), 1.0);
			}
			else if (u_Local5.a > 0.0 && var_Slope > 0)
			{// Steep maps (high angles)...
				float a1 = -1.0;
				return GetMap(u_SteepMap, 0.0025, a1);
			}
			else if (u_Local7.r > 0.0 || u_Local7.g > 0.0 || u_Local7.b > 0.0 || u_Local7.a > 0.0)
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

vec3 EnvironmentBRDF(float gloss, float NE, vec3 specular)
{
	vec4 t = vec4( 1/0.96, 0.475, (0.0275 - 0.25 * 0.04)/0.96,0.25 ) * gloss;
	t += vec4( 0.0, 0.0, (0.015 - 0.75 * 0.04)/0.96,0.75 );
	float a0 = t.x * min( t.y, exp2( -9.28 * NE ) ) + t.z;
	float a1 = t.w;
	return clamp( a0 + specular * ( a1 - a0 ), 0.0, 1.0 );
}

#if 0
mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );

	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame 
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}
#endif


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


void main()
{
#if 0
	gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	out_Glow = vec4(0.0);

#ifdef USE_GLOW_DETAIL_BUFFERS
	if (USE_ISDETAIL <= 0.0)
#else //!USE_GLOW_DETAIL_BUFFERS
	if (gl_FragColor.a > 0.99)
#endif //USE_GLOW_DETAIL_BUFFERS
	{
		out_Position = vec4(m_vertPos.xyz, u_Local1.a);
		out_Normal = vec4(m_Normal.xyz, u_Local1.b /*specularScale*/ );
	}
	return;
#endif

	vec4 specular = vec4(0.0);
	vec2 texCoords = m_TexCoords.xy;
	float pixRandom = 0.0;

	if (USE_TRIPLANAR > 0.0)
	{
		vLocalSeed = m_vertPos.xyz;
		pixRandom = randZeroOne();
	}

	if (USE_GLOW_BUFFER <= 0.0 && u_Local4.a > 0.0 && !(u_Local5.a > 0.0 && var_Slope > 0) && !(u_Local6.g > 0.0 && m_vertPos.z <= WATER_LEVEL + 128.0 + (64.0 * pixRandom)))
	{// Sway...
		texCoords += vec2(u_Local5.y * u_Local4.a * ((1.0 - m_TexCoords.y) + 1.0), 0.0);
	}


	bool LIGHTMAP_ENABLED = (USE_LIGHTMAP > 0.0 && USE_GLOW_BUFFER <= 0.0 && USE_IS2D <= 0.0) ? true : false;
	bool CUBEMAP_ENABLED = (USE_CUBEMAP > 0.0 && USE_GLOW_BUFFER <= 0.0 && u_EnableTextures.w > 0.0 && u_CubeMapStrength > 0.0 && cubeStrength > 0.0 && USE_IS2D <= 0.0) ? true : false;


	mat3 tangentToWorld;
	vec3 E;

	//if (USE_GLOW_BUFFER <= 0.0 /*&& USE_ISDETAIL <= 0.0*/)
	if (CUBEMAP_ENABLED)
	{
#if 0
		tangentToWorld = mat3(var_Tangent.xyz, var_Bitangent.xyz, m_Normal.xyz);
#endif
		E = normalize(m_ViewDir);
	}

	vec4 diffuse = GetDiffuse(texCoords, pixRandom);

	// Set alpha early so that we can cull early...
	gl_FragColor.a = clamp(diffuse.a * var_Color.a, 0.0, 1.0);


#ifdef USE_ALPHA_TEST
	float alphaMult = 1.0;

	if (u_AlphaTestValues.r > 0.0)
	{
		bool discardFrag = false;

		if (u_AlphaTestValues.r == ATEST_LT)
			if (gl_FragColor.a >= u_AlphaTestValues.g)
				discardFrag = true;
		if (u_AlphaTestValues.r == ATEST_GT)
			if (gl_FragColor.a <= u_AlphaTestValues.g)
				discardFrag = true;
		if (u_AlphaTestValues.r == ATEST_GE)
			if (gl_FragColor.a < u_AlphaTestValues.g)
				discardFrag = true;

		if (discardFrag)
		{
			gl_FragColor.rgba = vec4(0.0);
			out_Glow = vec4(0.0);
			out_Position = vec4(0.0);
			out_Normal = vec4(0.0);
			return;
		}
	}

#elif defined(USE_ALPHA_MULT)
	/*
	float alphaMult = 1.0;

	if (u_AlphaTestValues.r > 0.0)
	{
		if (u_AlphaTestValues.r == ATEST_LT && gl_FragColor.a >= u_AlphaTestValues.g)
			alphaMult = 0.0;
		if (u_AlphaTestValues.r == ATEST_GT && u_AlphaTestValues.g > 0.0 && gl_FragColor.a <= u_AlphaTestValues.g)
			alphaMult = 0.0;
		if (u_AlphaTestValues.r == ATEST_GE && gl_FragColor.a < u_AlphaTestValues.g)
			alphaMult = 0.0;
	}*/
#endif //USE_ALPHA_TEST



	//float lightScale = clamp((1.0 - max(max(diffuse.r, diffuse.g), diffuse.b)) - 0.5, 0.0, 1.0);


	vec3 N = normalize(m_Normal.xyz);
	vec4 norm = vec4(0.0);

	if (USE_GLOW_BUFFER <= 0.0 && USE_IS2D <= 0.0 && USE_ISDETAIL <= 0.0)
	{
		if (!(u_Local4.r <= 0.0 || USE_TRIPLANAR >= 0.0 || USE_REGIONS >= 0.0))
		{
			norm = texture(u_NormalMap, texCoords);
			norm.a = 1.0;
		}
	
		//N.xy = norm.xy * 2.0 - 1.0;
		//N.xy *= 0.25;
		//N.z = sqrt(clamp((0.25 - N.x * N.x) - N.y * N.y, 0.0, 1.0));
		//N = tangentToWorld * N;
	}


#ifdef USE_DETAIL
	if (USE_TRIPLANAR >= 0.0 || USE_REGIONS >= 0.0)// || USE_GLOW_BUFFER >= 0.0)
	{
		AddDetail(diffuse, texCoords);
	}
#endif //USE_DETAIL


	vec3 ambientColor = vec3(0.0);
	vec3 lightColor = clamp(var_Color.rgb, 0.0, 1.0);


	if (LIGHTMAP_ENABLED)
	{// TODO: Move to screen space?
		vec4 lightmapColor = textureLod(u_LightMap, var_TexCoords2.st, 0.0);

		#if defined(RGBM_LIGHTMAP)
			lightmapColor.rgb *= lightmapColor.a;
		#endif //defined(RGBM_LIGHTMAP)

		float lmBrightMult = clamp(1.0 - (length(lightmapColor.rgb) / 3.0), 0.0, 0.9);
		
#define lm_const_1 ( 56.0 / 255.0)
#define lm_const_2 (255.0 / 200.0)
		lmBrightMult = clamp((clamp(lmBrightMult - lm_const_1, 0.0, 1.0)) * lm_const_2, 0.0, 1.0);
		lmBrightMult = lmBrightMult * 0.7;

		lightColor	= lightmapColor.rgb * lmBrightMult;

		ambientColor = lightColor;
		float surfNL = clamp(dot(var_PrimaryLightDir.xyz, N.xyz), 0.0, 1.0);
		lightColor /= clamp(max(surfNL, /*0.35*/0.25), 0.0, 1.0);
		ambientColor = clamp(ambientColor - lightColor * surfNL, 0.0, 1.0);
		lightColor *= lightmapColor.rgb;
	}


	if (USE_GLOW_BUFFER > 0.0 || USE_IS2D > 0.0)
		gl_FragColor.rgb = diffuse.rgb + ambientColor;
	else
		gl_FragColor.rgb = diffuse.rgb + (ambientColor * 0.6);


	if (USE_GLOW_BUFFER <= 0.0 && USE_IS2D <= 0.0 && u_Local1.a != MATERIAL_SKY && u_Local1.a != MATERIAL_SUN /*&& u_Local1.a != MATERIAL_NONE*/)
	{
		gl_FragColor.rgb = gl_FragColor.rgb * u_MapAmbient.rgb;
	}
	

	if (CUBEMAP_ENABLED)
	{// TODO: Move to screen space...
		float curDist = distance(u_ViewOrigin.xyz, m_vertPos.xyz);
		float cubeFade = 0.0;
		
		if (curDist < cubeMaxDist)
		{
			cubeFade = clamp(1.0 - (curDist / cubeMaxDist), 0.0, 1.0);
		}

		if (cubeFade > 0.0)
		{
			if (u_Local1.g > 0.0)
			{// Real specMap...
				specular = texture(u_SpecularMap, texCoords);
			}
			else
			{// Fake it...
				specular.rgb = gl_FragColor.rgb;
#define specLower ( 48.0 / 255.0)
#define specUpper (255.0 / 192.0)
				specular.rgb = clamp((clamp(specular.rgb - specLower, 0.0, 1.0)) * specUpper, 0.0, 1.0);
				specular.a = clamp(((clamp(u_Local1.g, 0.0, 1.0) + clamp(u_Local3.a, 0.0, 1.0)) / 2.0) * 1.6, 0.0, 1.0);
			}

			specular.rgb *= u_SpecularScale.rgb;

#define gloss specular.a

			float NE = clamp(dot(N, E), 0.0, 1.0);
			vec3 reflectance = EnvironmentBRDF(gloss, NE, specular.rgb);

			vec3 R = reflect(E, N);
			vec3 parallax = u_CubeMapInfo.xyz + u_CubeMapInfo.w * m_ViewDir;
			vec3 cubeLightColor = textureCubeLod(u_CubeMap, R + parallax, 7.0 - specular.a * 7.0).rgb * u_EnableTextures.w;

			// Maybe if not metal, here, we should add contrast to only show the brights as reflection...
			gl_FragColor.rgb = mix(gl_FragColor.rgb, cubeLightColor * reflectance, clamp(cubeFade * cubeStrength * u_CubeMapStrength * u_EnableTextures.w * 0.2, 0.0, 1.0));
		}
	}


	gl_FragColor.rgb *= clamp(lightColor, 0.0, 1.0);

#if defined(USE_ALPHA_MULT)
	gl_FragColor.a *= alphaMult;

	if (u_Local1.a == 19.0 || u_Local1.a == 20.0)
	{// Leaves/grass-blades are either solid, or not, never partially solid.
		if (gl_FragColor.a >= 0.5)
		{
			gl_FragColor.a = 1.0;
			alphaMult = 1.0;
		}
		else
		{
			gl_FragColor.a = 0.0;
			alphaMult = 0.0;
		}
	}
	else if (gl_FragColor.a <= 0.0)
	{
		alphaMult = 0.0;
	}
#endif

	if (USE_GLOW_BUFFER > 0.0)
	{
#define glow_const_1 ( 23.0 / 255.0)
#define glow_const_2 (255.0 / 229.0)
		gl_FragColor.rgb = clamp((clamp(gl_FragColor.rgb - glow_const_1, 0.0, 1.0)) * glow_const_2, 0.0, 1.0);
		gl_FragColor.rgb *= u_Local8.g;

		out_Glow = gl_FragColor;

#ifdef USE_GLOW_DETAIL_BUFFERS
		if (USE_ISDETAIL <= 0.0)
#else //!USE_GLOW_DETAIL_BUFFERS
		if (gl_FragColor.a > 0.99)
#endif //USE_GLOW_DETAIL_BUFFERS
		{
#if defined(USE_ALPHA_MULT)
			out_Position = vec4(m_vertPos.xyz, u_Local1.a * alphaMult);
			out_Normal = vec4( N.xyz * 0.5 + 0.5, u_Local1.b * alphaMult /*specularScale*/ );
#else
			out_Position = vec4(m_vertPos.xyz, u_Local1.a);
			out_Normal = vec4( N.xyz * 0.5 + 0.5, u_Local1.b /*specularScale*/ );
#endif
		}
	}
	else
	{
		out_Glow = vec4(0.0);

#ifdef USE_GLOW_DETAIL_BUFFERS
		if (USE_ISDETAIL <= 0.0)
#else //!USE_GLOW_DETAIL_BUFFERS
		if (gl_FragColor.a > 0.99)
#endif //USE_GLOW_DETAIL_BUFFERS
		{
#if defined(USE_ALPHA_MULT)
			out_Position = vec4(m_vertPos.xyz, u_Local1.a * alphaMult);
			out_Normal = vec4( N.xyz * 0.5 + 0.5, u_Local1.b * alphaMult /*specularScale*/ );
			out_NormalDetail = norm;
#else
			out_Position = vec4(m_vertPos.xyz, u_Local1.a);
			out_Normal = vec4( N.xyz * 0.5 + 0.5, u_Local1.b /*specularScale*/ );
			out_NormalDetail = norm;
#endif
		}
	}
}
