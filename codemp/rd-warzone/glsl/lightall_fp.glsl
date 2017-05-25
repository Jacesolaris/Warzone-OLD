#if defined(USE_PARALLAXMAP) && !defined(USE_GLOW_BUFFER)
#define __PARALLAX_ENABLED__
#endif

#if defined(USE_CUBEMAP) && !defined(USE_GLOW_BUFFER)
#define __CUBEMAPS_ENABLED__
#endif



uniform sampler2D			u_DiffuseMap;
uniform sampler2D			u_SteepMap;
uniform sampler2D			u_SteepMap2;
uniform sampler2D			u_SplatControlMap;
uniform sampler2D			u_SplatMap1;
uniform sampler2D			u_SplatMap2;
uniform sampler2D			u_SplatMap3;
//uniform sampler2D			u_SplatMap4;
uniform sampler2D			u_SplatNormalMap1;
uniform sampler2D			u_SplatNormalMap2;
uniform sampler2D			u_SplatNormalMap3;
//uniform sampler2D			u_SplatNormalMap4;
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



uniform vec4				u_Settings0; // useTC, useDeform, useRGBA, USE_TEXTURECLAMP
uniform vec4				u_Settings1; // useVertexAnim, useSkeletalAnim, useFog, noNormOutputs

#define USE_TC				u_Settings0.r
#define USE_DEFORM			u_Settings0.g
#define USE_RGBA			u_Settings0.b
#define USE_TEXTURECLAMP	u_Settings0.a

#define USE_VERTEX_ANIM		u_Settings1.r
#define USE_SKELETAL_ANIM	u_Settings1.g
#define USE_FOG				u_Settings1.b
#define USE_NO_NORMALS		u_Settings1.a


uniform vec2				u_Dimensions;
uniform vec4				u_Local1; // parallaxScale, haveSpecular, specularScale, materialType
uniform vec4				u_Local2; // ExtinctionCoefficient
uniform vec4				u_Local3; // RimScalar, MaterialThickness, subSpecPower, cubemapScale
uniform vec4				u_Local4; // haveNormalMap, isMetalic, hasRealSubsurfaceMap, sway
uniform vec4				u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap
uniform vec4				u_Local6; // useSunLightSpecular, hasSteepMap2, MAP_SIZE, WATER_LEVEL
uniform vec4				u_Local7; // hasSplatMap1, hasSplatMap2, hasSplatMap3, hasSplatMap4
uniform vec4				u_Local8; // stageNum, glowStrength, MAP_INFO_MAXS[2], r_showsplat
uniform vec4				u_Local9; // testvalue0, 1, 2, 3

#define WATER_LEVEL			u_Local6.a

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

varying vec3				var_fogDir;


#define m_Normal			var_Normal
#define m_TexCoords			var_TexCoords
#define m_vertPos			var_vertPos
#define m_ViewDir			var_ViewDir


#endif //defined(USE_TESSELLATION) || defined(USE_ICR_CULLING)



out vec4 out_Glow;
out vec4 out_Position;
out vec4 out_Normal;

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


// 'threshold ' is constant , 'value ' is smoothly varying
/*float aastep ( float threshold , float value )
{
	float afwidth = 0.7 * length ( vec2 ( dFdx ( value ) , dFdy ( value ))) ;
	// GLSL 's fwidth ( value ) is abs ( dFdx ( value )) + abs ( dFdy ( value ))
	return smoothstep ( threshold - afwidth , threshold + afwidth , value ) ;
}*/

void AddContrast ( inout vec3 color )
{
	const float contrast = 3.0;
	const float brightness = 0.03;
	//float contrast = u_Local9.r;
	//float brightness = u_Local9.g;
	// Apply contrast.
	color.rgb = ((color.rgb - 0.5f) * max(contrast, 0)) + 0.5f;
	// Apply brightness.
	color.rgb += brightness;
	//color.rgb = clamp(color.rgb, 0.0, 1.0);
}

vec4 ConvertToNormals ( vec4 colorIn )
{
	// This makes silly assumptions, but it adds variation to the output. Hopefully this will look ok without doing a crapload of texture lookups or
	// wasting vram on real normals.
	//
	// UPDATE: In my testing, this method looks just as good as real normal maps. I am now using this as default method unless r_normalmapping >= 2
	// for the very noticable FPS boost over texture lookups.

	vec4 color = colorIn;

	vec3 N = vec3(clamp(color.r + color.b, 0.0, 1.0), clamp(color.g + color.b, 0.0, 1.0), clamp(color.r + color.g, 0.0, 1.0));

	N.xy = 1.0 - N.xy;
	N.xyz = N.xyz * 0.5 + 0.5;
	N.xyz = pow(N.xyz, vec3(2.0));
	N.xyz *= 0.8;

	vec3 N2 = N;

	// Centralize the color, then stretch, generating lots of contrast...
	N.rgb = N.rgb * 0.5 + 0.5;
	AddContrast(N.rgb);

	float displacement = clamp(length(color.rgb), 0.0, 1.0);
#define const_1 ( 32.0 / 255.0)
#define const_2 (255.0 / 219.0)
	displacement = clamp((clamp(displacement - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);

	//vec4 norm = vec4((N + N2 + (1.0 - N.brg)) / 3.0, displacement);
	vec4 norm = vec4((N + N2) / 2.0, displacement);
	//norm.z = dot(N.xyz, 1.0 - N2.xyz);
	//norm.z = sqrt(clamp((1.0 - norm.x * norm.x) - norm.y * norm.y, 0.0, 1.0));
	//norm.z = (N.x + N.y) / 2.0;
	norm.rgb = norm.rbg;
	if (length(norm.xyz) < 0.1) norm.xyz = norm.xyz * 0.5 + 0.5;
	//return norm;// * colorIn.a;
	return vec4(vec3(1.0)-norm.rgb * 0.5, norm.a);
}


// Used on everything...
const float detailRepeatFine = 64.0;

#if defined(USE_TRI_PLANAR) || defined(USE_REGIONS)
const float detailRepeatTerrain1 = 2.5;
const float detailRepeatTerrain2 = 7.5;
#endif //defined(USE_TRI_PLANAR) || defined(USE_REGIONS)
   
void AddDetail(inout vec4 color, in vec2 tc)
{
	if (USE_TEXTURECLAMP > 0.0 /*|| USE_DEFORM > 0.0 || USE_RGBA > 0.0*/) return;

	vec4 origColor = color;

	// Add fine detail to everything...
    vec3 detail = texture(u_DetailMap, tc * detailRepeatFine).rgb;

	if (length(detail.rgb) == 0.0) return;

	color.rgb = color.rgb * detail.rgb * 2.0;

#if defined(USE_TRI_PLANAR) || defined(USE_REGIONS)
	// Add a much less fine detail over terrains to help hide texture repetition...
	detail = texture(u_DetailMap, tc * detailRepeatTerrain1).rgb;
	color.rgb = color.rgb * detail.rgb * 2.0;

	// And a second, even less fine pass...
	detail = texture(u_DetailMap, tc * detailRepeatTerrain2).rgb;
	color.rgb = color.rgb * detail.rgb * 2.0;
#endif //defined(USE_TRI_PLANAR) || defined(USE_REGIONS)
}

#if defined(USE_TRI_PLANAR) || defined(USE_REGIONS)
vec4 GetControlMap( sampler2D tex)
{
	float scale = 1.0 / u_Local6.b; /* control scale */
	vec4 control;
	
#if defined(USE_REGIONS)
	// Try to verticalize the control map, so hopefully we can paint it in a more vertical way to get snowtop mountains, etc...
#define SNOW_HEIGHT_STRENGTH 0.25
	float maxHeightOverWater = u_Local8.b - u_Local6.a;
	float currentheightOverWater = u_Local8.b - m_vertPos.z;
	float y = pow(currentheightOverWater / maxHeightOverWater, SNOW_HEIGHT_STRENGTH/*u_Local9.r*/);
	float xyoffset = (u_Local6.b - (u_Local6.b / 2.0)) / (u_Local6.b * 2.0);
	vec4 xaxis = textureLod( tex, vec2((m_vertPos.x / (u_Local6.b / 2.0)) * xyoffset, y), 0.0);
	vec4 yaxis = textureLod( tex, vec2((m_vertPos.y / (u_Local6.b / 2.0)) * xyoffset, y), 0.0);
	vec4 zaxis = textureLod( tex, vec2((m_vertPos.z / (u_Local6.b / 2.0)) * xyoffset, y), 0.0);
	control = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;
#else //!defined(USE_REGIONS)
	/*vec4 xaxis = textureLod( tex, (m_vertPos.yz * scale) * 0.5 + 0.5, 0.0);
	vec4 yaxis = textureLod( tex, (m_vertPos.xz * scale) * 0.5 + 0.5, 0.0);
	vec4 zaxis = textureLod( tex, (m_vertPos.xy * scale) * 0.5 + 0.5, 0.0);*/

	/*vec4 xaxis = textureLod( tex, (m_vertPos.yze * scale), 0.0);
	vec4 yaxis = textureLod( tex, (m_vertPos.xz * scale), 0.0);
	vec4 zaxis = textureLod( tex, (m_vertPos.xy * scale), 0.0);*/

	float offset = (u_Local6.b / 2.0) * scale;
	vec4 xaxis = textureLod( tex, (m_vertPos.yz * scale) + offset, 0.0);
	vec4 yaxis = textureLod( tex, (m_vertPos.xz * scale) + offset, 0.0);
	vec4 zaxis = textureLod( tex, (m_vertPos.xy * scale) + offset, 0.0);
	control = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;
#endif //defined(USE_REGIONS)

	//control = clamp(pow(control, vec4(2.5)) * 10.0, 0.0, 1.0);
	//control = clamp(pow(control, vec4(2.5)), 0.0, 1.0);
	//control = control * 0.7 + 0.3;
	//control *= 10.0;
	control = clamp(control * 10.0, 0.0, 1.0);
	
	return control;
}

// For fake normal map lookups.
#define FAKE_MAP_NONE 0
#define FAKE_MAP_NORMALMAP 1
#define FAKE_MAP_NORMALMAP2 2
#define FAKE_MAP_NORMALMAP3 3

vec3 splatblend(vec4 texture1, float a1, vec4 texture2, float a2)
{
    float depth = 0.2;
    float ma = max(texture1.a + a1, texture2.a + a2) - depth;

    float b1 = max(texture1.a + a1 - ma, 0);
    float b2 = max(texture2.a + a2 - ma, 0);

    return (texture1.rgb * b1 + texture2.rgb * b2) / (b1 + b2);
}

vec4 GetMap( in sampler2D tex, float scale, vec2 ParallaxOffset, int fakeMapType)
{
	bool fakeNormal = false;
	vec4 xaxis;
	vec4 yaxis;
	vec4 zaxis;

	vec2 tScale = vec2(1.0);
	//if (u_textureScale.x > 0.0) tScale.x = u_textureScale.x;
	//if (u_textureScale.y > 0.0) tScale.y = u_textureScale.y;

	if (!(u_textureScale.x <= 0.0 && u_textureScale.y <= 0.0) && !(u_textureScale.x == 1.0 && u_textureScale.y == 1.0))
	{
		tScale *= u_textureScale;
	}

	if (fakeMapType == FAKE_MAP_NORMALMAP)
	{
		fakeNormal = true;

		xaxis = texture( u_DiffuseMap, (m_vertPos.yz * tScale * scale) + ParallaxOffset.xy);
		yaxis = texture( u_DiffuseMap, (m_vertPos.xz * tScale * scale) + ParallaxOffset.xy);
		zaxis = texture( u_DiffuseMap, (m_vertPos.xy * tScale * scale) + ParallaxOffset.xy);
	}
	else if (fakeMapType == FAKE_MAP_NORMALMAP2)
	{
		fakeNormal = true;

		xaxis = texture( u_SteepMap, (m_vertPos.yz * tScale * scale) + ParallaxOffset.xy);
		yaxis = texture( u_SteepMap, (m_vertPos.xz * tScale * scale) + ParallaxOffset.xy);
		zaxis = texture( u_SteepMap, (m_vertPos.xy * tScale * scale) + ParallaxOffset.xy);
	}
	else if (fakeMapType == FAKE_MAP_NORMALMAP3)
	{
		fakeNormal = true;

		xaxis = texture( u_SteepMap2, (m_vertPos.yz * tScale * scale) + ParallaxOffset.xy);
		yaxis = texture( u_SteepMap2, (m_vertPos.xz * tScale * scale) + ParallaxOffset.xy);
		zaxis = texture( u_SteepMap2, (m_vertPos.xy * tScale * scale) + ParallaxOffset.xy);
	}
	else
	{
		xaxis = texture( tex, (m_vertPos.yz * tScale * scale) + ParallaxOffset.xy);
		yaxis = texture( tex, (m_vertPos.xz * tScale * scale) + ParallaxOffset.xy);
		zaxis = texture( tex, (m_vertPos.xy * tScale * scale) + ParallaxOffset.xy);
	}

	if (fakeNormal)
	{
		return ConvertToNormals(xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z);
	}

	return xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;
}

vec4 GetSplatMap(vec2 texCoords, vec2 ParallaxOffset, float pixRandom, vec4 inColor, float inA1)
{
	if (u_Local7.r <= 0.0 && u_Local7.g <= 0.0 && u_Local7.b <= 0.0 && u_Local7.a <= 0.0)
	{
		return inColor;
	}

	if (u_Local6.g > 0.0 && m_vertPos.z <= WATER_LEVEL)// + 128.0) + (64.0 * pixRandom))
	{// Steep maps (water edges)... Underwater doesn't use splats for now...
		return inColor;
	}

	// Splat blend in all the textues using the control strengths...

	vec4 splatColor = inColor;

	vec4 control = GetControlMap(u_SplatControlMap);

	if (length(control.rgb/*a*/) <= 0.0)
	{
		return inColor;
	}

	if (u_Local7.r > 0.0 && control.r > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap1, 0.01, ParallaxOffset, FAKE_MAP_NONE);
		splatColor = mix(splatColor, tex, control.r * tex.a);
	}

	if (u_Local7.g > 0.0 && control.g > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap2, 0.01, ParallaxOffset, FAKE_MAP_NONE);
		splatColor = mix(splatColor, tex, control.g * tex.a);
	}

	if (u_Local7.b > 0.0 && control.b > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap3, 0.01, ParallaxOffset, FAKE_MAP_NONE);
		splatColor = mix(splatColor, tex, control.b * tex.a);
	}
	/*
	if (u_Local7.a > 0.0 && control.a > 0.0)
	{
		vec4 tex = GetMap(u_SplatMap4, 0.01, ParallaxOffset, FAKE_MAP_NONE);
		splatColor = mix(splatColor, tex, control.a * tex.a);
	}
	*/

	return splatColor;
}
#endif //defined(USE_TRI_PLANAR) || defined(USE_REGIONS)

#if defined(USE_REGIONS)
vec4 GenerateTerrainMap(vec2 coord)
{
	if (u_Local8.a > 0.0)
	{
		vec4 control = GetControlMap(u_SplatControlMap);
		return vec4(control.rgb, 1.0);
	}

	// Splat mapping...
#define SNOW_HEIGHT 0.001
	vec4 tex = GetMap(u_DiffuseMap, SNOW_HEIGHT/*u_Local9.g*/, vec2(0.0), FAKE_MAP_NONE);
	float a1 = 0.0;
	a1 = ConvertToNormals(tex).a;
	return GetSplatMap(coord, vec2(0.0), 0.0, tex, a1);
}
#endif //defined(USE_REGIONS)

#if defined(USE_TRI_PLANAR)
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

vec4 GetNonSplatMap( in sampler2D tex, vec2 coord )
{
	return texture( tex, coord );
}

vec4 GetDiffuse(vec2 texCoords, vec2 ParallaxOffset, float pixRandom)
{
	if (u_Local8.a > 0.0)
	{
		vec4 control = GetControlMap(u_SplatControlMap);
		return vec4(control.rgb, 1.0);
	}

	if (u_Local6.g > 0.0 && m_vertPos.z <= WATER_LEVEL + 128.0 + (64.0 * pixRandom))
	{// Steep maps (water edges)...
		float mixVal = ((WATER_LEVEL + 128.0) - m_vertPos.z) / 128.0;

		vec4 tex1 = GetMap(u_SteepMap2, 0.0075, ParallaxOffset, FAKE_MAP_NONE);
		float a1 = 0.0;

		if (u_Local4.r <= 0.0) // save texture lookup
			a1 = ConvertToNormals(tex1).a;
		else
			a1 = GetMap(u_NormalMap3, 0.0075, ParallaxOffset, FAKE_MAP_NORMALMAP3).a;

		vec4 tex2 = GetMap(u_DiffuseMap, 0.0075, ParallaxOffset, FAKE_MAP_NONE);
		float a2 = 0.0;

		if (u_Local4.r <= 0.0) // save texture lookup
			a2 = ConvertToNormals(tex2).a;
		else
			a2 = GetMap(u_NormalMap, 0.0075, ParallaxOffset, FAKE_MAP_NORMALMAP).a;

		if (u_Local7.r <= 0.0 && u_Local7.g <= 0.0 && u_Local7.b <= 0.0 && u_Local7.a <= 0.0)
		{// No splat maps...
			return vec4(splatblend(tex1, a1 * (a1 * mixVal), tex2, a2 * (1.0 - (a2 * mixVal))), 1.0);
		}

		// Splat mapping...
		tex2 = GetSplatMap(texCoords, ParallaxOffset, pixRandom, tex2, a2);

		return vec4(splatblend(tex1, a1 * (a1 * mixVal), tex2, a2 * (1.0 - (a2 * mixVal))), 1.0);
	}
	else if (u_Local5.a > 0.0 && var_Slope > 0)
	{// Steep maps (high angles)...
		return GetMap(u_SteepMap, 0.0025, ParallaxOffset, FAKE_MAP_NONE);
	}
	else if (u_Local5.a > 0.0)
	{// Steep maps (low angles)...
		if (u_Local7.r <= 0.0 && u_Local7.g <= 0.0 && u_Local7.b <= 0.0 && u_Local7.a <= 0.0)
		{// No splat maps...
			return GetMap(u_DiffuseMap, 0.0075, ParallaxOffset, FAKE_MAP_NONE);
		}

		// Splat mapping...
		vec4 tex = GetMap(u_DiffuseMap, 0.0075, ParallaxOffset, FAKE_MAP_NONE);
		float a1 = 0.0;

		if (u_Local4.r <= 0.0) // save texture lookup
			a1 = ConvertToNormals(tex).a;
		else
			a1 = GetMap(u_NormalMap, 0.0075, ParallaxOffset, FAKE_MAP_NORMALMAP).a;

		return GetSplatMap(texCoords, ParallaxOffset, pixRandom, tex, a1);
	}
	else
	{
		return GetNonSplatMap(u_DiffuseMap, texCoords);
	}
}

vec4 GetNormal(vec2 texCoords, vec2 ParallaxOffset, float pixRandom)
{
	if (u_Local6.g > 0.0 && m_vertPos.z <= WATER_LEVEL + 128.0 + (64.0 * pixRandom))
	{// Steep maps (water edges)...
		if (u_Local4.r <= 0.0)
		{
			return ConvertToNormals(GetDiffuse(texCoords, ParallaxOffset, pixRandom));
		}

		float mixVal = ((WATER_LEVEL + 128.0) - m_vertPos.z) / 128.0;

		vec4 tex1 = GetMap(u_NormalMap3, 0.0075, ParallaxOffset, FAKE_MAP_NORMALMAP3);
		float a1 = tex1.a;

		vec4 tex2 = GetMap(u_NormalMap, 0.0075, ParallaxOffset, FAKE_MAP_NORMALMAP);
		float a2 = tex2.a;

		return vec4(splatblend(tex1, a1 * (a1 * mixVal), tex2, a2 * (1.0 - (a2 * mixVal))), 1.0);
	}
	else if (u_Local5.a > 0.0 && var_Slope > 0)
	{// Steep maps (high angles)...
		if (u_Local4.r <= 0.0)
		{
			return ConvertToNormals(GetDiffuse(texCoords, ParallaxOffset, pixRandom));
		}

		return GetMap(u_NormalMap2, 0.0025, ParallaxOffset, FAKE_MAP_NORMALMAP2);
	}
	else if (u_Local5.a > 0.0)
	{// Steep maps (normal angles)...
		if (u_Local4.r <= 0.0)
		{
			return ConvertToNormals(GetDiffuse(texCoords, ParallaxOffset, pixRandom));
		}

		return GetMap(u_NormalMap, 0.0075, ParallaxOffset, FAKE_MAP_NORMALMAP);
	}
	else
	{
		if (u_Local4.r <= 0.0)
		{
			return ConvertToNormals(GetNonSplatMap(u_DiffuseMap, texCoords));
		}

		return textureLod(u_NormalMap, texCoords, 0.0);
	}
}

#else //!defined(USE_TRI_PLANAR)

vec4 GetDiffuse(vec2 texCoords, vec2 ParallaxOffset, float pixRandom)
{
#if defined(USE_REGIONS)
	return GenerateTerrainMap(texCoords + ParallaxOffset);
#else
	return texture(u_DiffuseMap, texCoords + ParallaxOffset);
#endif
}

vec4 GetNormal(vec2 texCoords, vec2 ParallaxOffset, float pixRandom)
{
#if defined(USE_REGIONS)
	return ConvertToNormals(GenerateTerrainMap(texCoords + ParallaxOffset));
#else
	if (u_Local4.r <= 0.0)
	{
		return ConvertToNormals(texture(u_DiffuseMap, texCoords + ParallaxOffset));
	}

	return textureLod(u_NormalMap, texCoords, 0.0);
#endif
}
#endif //!defined(USE_TRI_PLANAR)

float GetDepth(vec2 t)
{
	return 1.0 - GetNormal(t, vec2(0.0), 0.0).a;
}

#if defined(USE_PARALLAXMAP)

float RayIntersectDisplaceMap(vec2 dp, inout float displacement)
{
	if (u_Local1.x == 0.0)
		return 0.0;

	displacement = GetDepth(dp);
	return (1.0 - displacement) * 0.5 + 0.5;
}

float ReliefMapping(vec2 dp, vec2 ds)
{
	const int linear_steps = 10;
	const int binary_steps = 5;
	float depth_step = 1.0 / linear_steps;
	float size = depth_step;
	float depth = 1.0;
	float best_depth = 1.0;
	for (int i = 0 ; i < linear_steps - 1 ; ++i) {
		depth -= size;
		float t = GetDepth(dp + ds * depth);
		if (depth >= 1.0 - t)
			best_depth = depth;
	}
	depth = best_depth - size;
	for (int i = 0 ; i < binary_steps ; ++i) {
		size *= 0.5;
		float t = GetDepth(dp + ds * depth);
		if (depth >= 1.0 - t) {
			best_depth = depth;
			depth -= 2 * size;
		}
		depth += size;
	}
	return clamp(best_depth, 0.0, 1.0);
}
#endif //defined(USE_PARALLAXMAP)

vec3 EnvironmentBRDF(float gloss, float NE, vec3 specular)
{
	vec4 t = vec4( 1/0.96, 0.475, (0.0275 - 0.25 * 0.04)/0.96,0.25 ) * gloss;
	t += vec4( 0.0, 0.0, (0.015 - 0.75 * 0.04)/0.96,0.75 );
	float a0 = t.x * min( t.y, exp2( -9.28 * NE ) ) + t.z;
	float a1 = t.w;
	return clamp( a0 + specular * ( a1 - a0 ), 0.0, 1.0 );
}

void main()
{
	vec4 specular = vec4(0.0);
	vec2 texCoords = m_TexCoords.xy;

	float dist = distance(m_vertPos.xyz, u_ViewOrigin.xyz);
	bool isDistant = false;

	if (dist > 4096.0)
	{
		isDistant = true;
	}


#if defined(USE_TRI_PLANAR)
	vLocalSeed = m_vertPos.xyz;
	float pixRandom = randZeroOne();
#else //!defined(USE_TRI_PLANAR)
	float pixRandom = 0.0; // Don't use it anyway...
#endif //defined(USE_TRI_PLANAR)

#if 0
	vec3 debugColor;

	
	
	if (USE_VERTEX_ANIM == 1.0)
	{
		debugColor = vec3(1.0, 0.0, 0.0);
	}
	else if (USE_SKELETAL_ANIM == 1.0)
	{
		debugColor = vec3(0.0, 1.0, 0.0);
	}
	else
	{
		debugColor = vec3(0.0, 0.0, 1.0);
	}
	
	
	/*if (USE_TC == 1.0)
	{
		debugColor = vec3(1.0, 0.0, 0.0);
	}
	else if (USE_DEFORM == 1.0)
	{
		debugColor = vec3(0.0, 1.0, 0.0);
	}
	else if (USE_RGBA == 1.0)
	{
		debugColor = vec3(1.0, 1.0, 1.0);
	}
	else
	{
		debugColor = vec3(0.0, 0.0, 1.0);
	}*/

	//debugColor = texture(u_SplatControlMap, m_TexCoords.xy).rgb;
	//debugColor = m_Normal.xyz;

	gl_FragColor = vec4(debugColor, 1.0);

	#if defined(USE_GLOW_BUFFER)
		out_Glow = gl_FragColor;
	#else
		out_Glow = vec4(0.0);
		vec2 normData = encode(vec3(1.0));
		vec2 cubeData = vec2(0.0, 1.0);
		out_Normal = vec4( normData.x, normData.y, cubeData.x, cubeData.y );
		out_Position = vec4(m_vertPos.xyz, u_Local1.a);
	#endif
	return;
#endif

	#if !defined(USE_GLOW_BUFFER)
		if (u_Local4.a > 0.0 && !(u_Local5.a > 0.0 && var_Slope > 0) && !(u_Local6.g > 0.0 && m_vertPos.z <= WATER_LEVEL + 128.0 + (64.0 * pixRandom)))
		{// Sway...
			texCoords += vec2(u_Local5.y * u_Local4.a * ((1.0 - m_TexCoords.y) + 1.0), 0.0);
		}
	#endif //!defined(USE_GLOW_BUFFER)



#if defined(__PARALLAX_ENABLED__) || defined(__CUBEMAPS_ENABLED__)
	vec3 viewDir;
	vec3 E;

	if ((!isDistant && u_Local3.a > 0.0 && u_EnableTextures.w > 0.0 && u_CubeMapStrength > 0.0) || (u_Local1.x > 0.0 && !isDistant))
	{
		viewDir = m_ViewDir;
		E = normalize(viewDir);
	}

#endif //defined(__PARALLAX_ENABLED__) || defined(__CUBEMAPS_ENABLED__)



	vec2 ParallaxOffset = vec2(0.0);
	float displacement = 0.0;


#if defined(__PARALLAX_ENABLED__)
	if (u_Local1.x > 0.0 && !isDistant && USE_TEXTURECLAMP <= 0.0)
	{
		vec2 tex_offset = vec2(1.0 / u_Dimensions);
		vec3 offsetDir = normalize((normalize(var_Tangent.xyz) * E.x) + (normalize(var_Bitangent.xyz) * E.y) + (normalize(m_Normal.xyz) * E.z));
		vec2 ParallaxXY = offsetDir.xy * tex_offset * u_Local1.x;

		#if defined(FAST_PARALLAX)

			ParallaxOffset = ParallaxXY * RayIntersectDisplaceMap(texCoords, displacement);
			texCoords += ParallaxOffset;

		#else //!defined(FAST_PARALLAX)

			// Steep Parallax
			float Step = 0.01;
			vec2 dt = ParallaxXY * Step;
			float Height = 0.5;
			float oldHeight = 0.5;
			vec2 Coord = texCoords;
			vec2 oldCoord = Coord;
			float HeightMap = GetDepth( Coord );
			float oldHeightMap = HeightMap;

			while( Height >= 0.0 && HeightMap < Height )
			{
				oldHeightMap = HeightMap;
				oldHeight = Height;
				oldCoord = Coord;

				Height -= Step;
				Coord += dt;
				HeightMap = GetDepth( Coord );
			}

			displacement = HeightMap;

			if( Height < 0.0 )
			{
				Coord = oldCoord;
				Height = 0.0;
				displacement = oldHeightMap;
			}

			ParallaxOffset = texCoords - Coord;
			texCoords = Coord;

		#endif //defined(FAST_PARALLAX)
	}
	#endif //defined(__PARALLAX_ENABLED__)




	vec4 diffuse = GetDiffuse(texCoords, ParallaxOffset, pixRandom);


	AddDetail(diffuse, texCoords);



#if !defined(USE_GLOW_BUFFER)
	vec4 norm;
	vec3 N;

	//if (u_Local4.r <= 0.0)
	//{
		norm = ConvertToNormals(diffuse /** var_Color.rgba*/);
	//}
	//else
	//{
	//	norm = GetNormal(texCoords, ParallaxOffset, pixRandom);
	//}

	N.xy = norm.xy * 2.0 - 1.0;
	N.xy *= 0.25;
	N.z = sqrt(clamp((0.25 - N.x * N.x) - N.y * N.y, 0.0, 1.0));
	N = normalize((normalize(var_Tangent.xyz) * N.x) + (normalize(var_Bitangent.xyz) * N.y) + (normalize(m_Normal.xyz) * N.z));
	N.rgb *= diffuse.a * var_Color.a;
#endif //!defined(USE_GLOW_BUFFER)


	vec3 ambientColor = vec3(0.0);
	vec3 lightColor = var_Color.rgb;


	#if defined(USE_LIGHTMAP) && !defined(USE_GLOW_BUFFER)

		vec4 lightmapColor = textureLod(u_LightMap, var_TexCoords2.st, 0.0);

		#if defined(RGBM_LIGHTMAP)
			lightmapColor.rgb *= lightmapColor.a;
		#endif //defined(RGBM_LIGHTMAP)

		float lmBrightMult = clamp(1.0 - (length(lightmapColor.rgb) / 3.0), 0.0, 0.9);
		
		//lmBrightMult *= lmBrightMult * 0.7;
#define lm_const_1 ( 56.0 / 255.0)
#define lm_const_2 (255.0 / 200.0)
		lmBrightMult = clamp((clamp(lmBrightMult - lm_const_1, 0.0, 1.0)) * lm_const_2, 0.0, 1.0);
		lmBrightMult = lmBrightMult * 0.7;

		//lightColor	= mix(lightmapColor.rgb * lmBrightMult, lightmapColor.rgb * lmBrightMult * var_Color.rgb, var_Color.a);
		lightColor	= lightmapColor.rgb * lmBrightMult;

		ambientColor = lightColor;
		float surfNL = clamp(-dot(var_PrimaryLightDir.xyz, N.xyz), 0.0, 1.0);
		lightColor /= max(surfNL, 0.35/*0.25*/);
		ambientColor = clamp(ambientColor - lightColor * surfNL, 0.0, 1.0);
		lightColor *= lightmapColor.rgb;

	#endif //defined(USE_LIGHTMAP) && !defined(USE_GLOW_BUFFER)


		gl_FragColor = vec4(diffuse.rgb + (diffuse.rgb * ambientColor), diffuse.a * var_Color.a);



#if !defined(USE_GLOW_BUFFER)
		bool outputNormals = false;

		if (USE_NO_NORMALS == 0.0)
		{
			if (gl_FragColor.a >= 0.3/*u_Local9.r*//*0.996*/ || ((u_Local1.a == 5.0 || u_Local1.a == 6.0 || u_Local1.a == 19.0 || u_Local1.a == 20.0) && gl_FragColor.a >= 0.5))
			{// Hmm how to handle transparancies with deferred... Maybe I should add a second alpha normals map...
				outputNormals = true;
			}
		}
#endif //!defined(USE_GLOW_BUFFER)


#if !defined(USE_GLOW_BUFFER)
#if !defined(DEFERRED_REFLECTIONS)
	#if defined(__CUBEMAPS_ENABLED__)
		if (!isDistant && u_Local3.a > 0.0 && u_EnableTextures.w > 0.0 && u_CubeMapStrength > 0.0 && outputNormals)
		{
			#if defined(USE_SPECULARMAP) && !defined(USE_GLOW_BUFFER)
			if (u_Local1.g != 0.0)
			{// Real specMap...
				specular = textureLod(u_SpecularMap, texCoords, 0.0);
			}
			else
			#endif //defined(USE_SPECULARMAP)
			{// Fake it...
				specular.rgb = gl_FragColor.rgb;
#define specLower ( 64.0 / 255.0)
#define specUpper (255.0 / 192.0)
				specular.rgb = clamp((clamp(specular.rgb - specLower, 0.0, 1.0)) * specUpper, 0.0, 1.0);
				specular.a = ((clamp(u_Local1.g, 0.0, 1.0) + clamp(u_Local3.a, 0.0, 1.0)) / 2.0) * 1.6;
			}

			specular.rgb *= u_SpecularScale.rgb;

			vec3  H  = normalize(var_PrimaryLightDir.xyz + E);
			float NE = clamp(dot(m_Normal.xyz/*N*/, E), 0.0, 1.0);

			vec3 R = reflect(E, m_Normal.xyz);
			vec3 reflectance = EnvironmentBRDF(clamp(specular.a, 0.5, 1.0) * 100.0, NE, specular.rgb);
			vec3 parallax = u_CubeMapInfo.xyz + u_CubeMapInfo.w * viewDir;
			vec3 cubeLightColor = textureCubeLod(u_CubeMap, R + parallax, 7.0 - specular.a * 7.0).rgb * u_EnableTextures.w * 0.25;
			gl_FragColor.rgb += (cubeLightColor * reflectance * (u_Local3.a * specular.a)) * u_CubeMapStrength * 0.5;
		}
	#endif //__CUBEMAPS_ENABLED__
#else //defined(DEFERRED_REFLECTIONS)
	float enableCubemap = 0.0; // For deferred cubemaps...

	if (!isDistant && u_Local3.a > 0.0 && u_EnableTextures.w > 0.0 && u_CubeMapStrength > 0.0)
	{
		enableCubemap = 1.0;
	}
#endif //defined(DEFERRED_REFLECTIONS)
#endif //!defined(USE_GLOW_BUFFER)


	gl_FragColor.rgb *= lightColor;
	

	/*
	vec2 encode (vec3 n)
	vec3 decode (vec2 enc)
	*/

	#if defined(USE_GLOW_BUFFER)
#define glow_const_1 ( 23.0 / 255.0)
#define glow_const_2 (255.0 / 229.0)
		gl_FragColor.rgb = clamp((clamp(gl_FragColor.rgb - glow_const_1, 0.0, 1.0)) * glow_const_2, 0.0, 1.0);
		gl_FragColor.rgb *= u_Local8.g;

		out_Glow = gl_FragColor;
	#else
		if (/*u_Local8.r == 0.0 &&*/ outputNormals)
		{
			out_Glow = vec4(0.0);
			vec2 normData = encode(N.xyz * 0.5 + 0.5);
#if defined(DEFERRED_REFLECTIONS)
			vec2 cubeData = encode(vec3(enableCubemap, u_Local3.a / 10.0, u_Local1.g / 10.0));
#else //!defined(DEFERRED_REFLECTIONS)
			float enabled = 0.0;
			if (u_Local1.a != 1024.0 && u_Local1.a != 1025.0)
				enabled = 1.0;
			vec2 cubeData = vec2(0.0, enabled);
#endif //defined(DEFERRED_REFLECTIONS)
			out_Normal = vec4( normData.x, normData.y, cubeData.x, cubeData.y );
			out_Position = vec4(m_vertPos.xyz, u_Local1.a);
		}
	#endif
}
