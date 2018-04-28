#define __HIGH_PASS_SHARPEN__

#define SCREEN_MAPS_ALPHA_THRESHOLD 0.666
#define SCREEN_MAPS_LEAFS_THRESHOLD 0.001
//#define SCREEN_MAPS_LEAFS_THRESHOLD 0.9


uniform sampler2D					u_DiffuseMap;

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
#define MAP_LIGHTMAP_ENHANCEMENT		u_Settings4.g


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


const float							fBranchHardiness = 0.001;
const float							fBranchSize = 128.0;
const float							fWindStrength = 12.0;
const vec3							vWindDirection = normalize(vec3(1.0, 1.0, 0.0));

vec2 pxSize = vec2(1.0) / u_Dimensions;

vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

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

#if defined(__HIGH_PASS_SHARPEN__)
vec3 Enhance(in sampler2D tex, in vec2 uv, vec3 color, float level)
{
	vec3 blur = textureLod(tex, uv, level).xyz;
	vec3 col = ((color - blur)*0.5 + 0.5) * 1.0;
	col *= ((color - blur)*0.25 + 0.25) * 8.0;
	col = mix(color, col * color, 1.0);
	return col;
}
#endif //defined(__HIGH_PASS_SHARPEN__)

void main()
{
	bool LIGHTMAP_ENABLED = (USE_LIGHTMAP > 0.0 && USE_GLOW_BUFFER != 1.0 && USE_IS2D <= 0.0) ? true : false;

	vec2 texCoords = m_TexCoords.xy;

	if (SHADER_SWAY > 0.0)
	{// Sway...
		texCoords += vec2(GetSway());
	}


	vec4 diffuse = texture(u_DiffuseMap, texCoords);

#if defined(__HIGH_PASS_SHARPEN__)
	if (USE_IS2D > 0.0 || USE_TEXTURECLAMP > 0.0)
	{
		diffuse.rgb = Enhance(u_DiffuseMap, texCoords, diffuse.rgb, 16.0);
	}
	else
	{
		diffuse.rgb = Enhance(u_DiffuseMap, texCoords, diffuse.rgb, 8.0);
	}
#endif //defined(__HIGH_PASS_SHARPEN__)

	// Set alpha early so that we can cull early...
	gl_FragColor.a = clamp(diffuse.a * var_Color.a, 0.0, 1.0);


	vec3 N = normalize(m_Normal.xyz);

#ifdef __USE_REAL_NORMALMAPS__
	vec4 norm = vec4(0.0);

	if (USE_GLOW_BUFFER != 1.0 && USE_IS2D <= 0.0 && USE_ISDETAIL <= 0.0 && SHADER_HAS_NORMALMAP > 0.0)
	{
		norm = texture(u_NormalMap, texCoords);
		norm.a = 1.0;
	}
#endif //__USE_REAL_NORMALMAPS__


	vec3 ambientColor = vec3(0.0);
	vec3 lightColor = clamp(var_Color.rgb, 0.0, 1.0);


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

			//float spec = clamp(getspecularLight(bNorm, -var_PrimaryLightDir.xyz, E, 16.0), 0.05, 0.125);
			//lightColor2 = clamp(spec * lightColor2, 0.0, 1.0);

			float fre = pow(clamp(dot(bNorm, -E) + 1.0, 0.0, 1.0), 0.3);
			//float spec = clamp(getspecularLight(bNorm, -var_PrimaryLightDir.xyz, E, 16.0), 0.05, 0.125);
			float spec = clamp(getspecularLight(bNorm, -var_PrimaryLightDir.xyz, E, 16.0) * fre, 0.05, 1.0);
			spec = clamp(spec, 0.25, 0.325);
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

			//float spec = clamp(getspecularLight(bNorm, -var_PrimaryLightDir.xyz, E, 16.0), 0.05, 0.125);
			//lightColor = clamp(spec * lightColor, 0.0, 1.0);

			float fre = pow(clamp(dot(bNorm, -E) + 1.0, 0.0, 1.0), 0.3);
			//float spec = clamp(getspecularLight(bNorm, -var_PrimaryLightDir.xyz, E, 16.0), 0.05, 0.125);
			float spec = clamp(getspecularLight(bNorm, -var_PrimaryLightDir.xyz, E, 16.0) * fre, 0.05, 1.0);
			spec = clamp(spec, 0.25, 0.325);
			lightColor = clamp(spec * lightColor, 0.0, 1.0);
		}
	}


	gl_FragColor.rgb = diffuse.rgb + ambientColor;


	if (USE_GLOW_BUFFER != 1.0 && USE_IS2D <= 0.0 && SHADER_MATERIAL_TYPE != MATERIAL_SKY && SHADER_MATERIAL_TYPE != MATERIAL_SUN && SHADER_MATERIAL_TYPE != MATERIAL_GLASS)
	{
		gl_FragColor.rgb = gl_FragColor.rgb * u_MapAmbient.rgb;
	}
	
	gl_FragColor.rgb *= clamp(lightColor, 0.0, 1.0);


	float alphaThreshold = (SHADER_MATERIAL_TYPE == MATERIAL_GREENLEAVES) ? SCREEN_MAPS_LEAFS_THRESHOLD : SCREEN_MAPS_ALPHA_THRESHOLD;

	if (gl_FragColor.a >= alphaThreshold || SHADER_MATERIAL_TYPE == 1024.0 || SHADER_MATERIAL_TYPE == 1025.0 || SHADER_MATERIAL_TYPE == MATERIAL_PUDDLE || USE_IS2D > 0.0)
	{

	}
	else if (SHADER_MATERIAL_TYPE == MATERIAL_EFX)
	{

	}
	else
	{
		gl_FragColor.a = 0.0;
	}
	
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

	gl_FragColor.a = clamp(gl_FragColor.a, 0.0, 1.0);

	if (gl_FragColor.a >= 0.99) gl_FragColor.a = 1.0; // Allow for rounding errors... Don't let them stop pixel culling...

	if (USE_EMISSIVE_BLACK > 0.0 && USE_GLOW_BUFFER <= 0.0)
	{
		gl_FragColor.rgb = vec3(0.0);
	}

	float useDisplacementMapping = 0.0;

	if ( SHADER_MATERIAL_TYPE == MATERIAL_SOLIDWOOD 
		|| SHADER_MATERIAL_TYPE == MATERIAL_HOLLOWWOOD 
		|| SHADER_MATERIAL_TYPE == MATERIAL_ROCK 
		|| SHADER_MATERIAL_TYPE == MATERIAL_SAND 
		|| SHADER_MATERIAL_TYPE == MATERIAL_GRAVEL
		|| SHADER_MATERIAL_TYPE == MATERIAL_SNOW 
		|| SHADER_MATERIAL_TYPE == MATERIAL_MUD 
		|| SHADER_MATERIAL_TYPE == MATERIAL_DIRT )
	{
		useDisplacementMapping = 1.0;
	}

#define glow_const_1 ( 23.0 / 255.0)
#define glow_const_2 (255.0 / 229.0)

	if (SHADER_MATERIAL_TYPE == 1024.0 || SHADER_MATERIAL_TYPE == 1025.0)
	{
		out_Glow = vec4(0.0);
		out_Position = vec4(0.0);
		out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
		out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
	}
	else if (USE_GLOW_BUFFER >= 2.0)
	{// Merged diffuse+glow stage...
		vec4 glowColor = max(texture(u_GlowMap, texCoords), 0.0);

		if (SHADER_MATERIAL_TYPE != MATERIAL_GLASS && length(glowColor.rgb) <= 0.0)
			glowColor.a = 0.0;

#if 1 // Conversion of old stages to merged glows is disabled atm. Not happy with the results.
#define GLSL_BLEND_ALPHA			0
#define GLSL_BLEND_INVALPHA			1
#define GLSL_BLEND_DST_ALPHA		2
#define GLSL_BLEND_INV_DST_ALPHA	3
#define GLSL_BLEND_GLOWCOLOR		4
#define GLSL_BLEND_INV_GLOWCOLOR	5
#define GLSL_BLEND_DSTCOLOR			6
#define GLSL_BLEND_INV_DSTCOLOR		7

		if (USE_GLOW_BLEND_MODE == GLSL_BLEND_INV_DSTCOLOR)
		{
			glowColor.rgb = (glowColor.rgb * glowColor.a) * (vec3(1.0) - clamp(gl_FragColor.rgb, 0.0, 1.0));
		}
		else if (USE_GLOW_BLEND_MODE == GLSL_BLEND_DSTCOLOR)
		{
			glowColor.rgb = (glowColor.rgb * glowColor.a) * clamp(gl_FragColor.rgb, 0.0, 1.0);
		}
		else if (USE_GLOW_BLEND_MODE == GLSL_BLEND_INV_GLOWCOLOR)
		{
			glowColor.rgb = (glowColor.rgb * glowColor.a) * (vec3(1.0) - glowColor.rgb);
		}
		else if (USE_GLOW_BLEND_MODE == GLSL_BLEND_GLOWCOLOR)
		{
			glowColor.rgb = (glowColor.rgb * glowColor.a) * glowColor.rgb;
		}
		else if (USE_GLOW_BLEND_MODE == GLSL_BLEND_INV_DST_ALPHA)
		{
			glowColor.rgb = (glowColor.rgb * glowColor.a) * (1.0 - clamp(gl_FragColor.a, 0.0, 1.0));
		}
		else if (USE_GLOW_BLEND_MODE == GLSL_BLEND_DST_ALPHA)
		{
			glowColor.rgb = (glowColor.rgb * glowColor.a) * clamp(gl_FragColor.a, 0.0, 1.0);
		}
		else if (USE_GLOW_BLEND_MODE == GLSL_BLEND_INVALPHA)
		{
			glowColor.rgb = (glowColor.rgb * (1.0 - clamp(glowColor.a, 0.0, 1.0)));
		}
		else
		{
			glowColor.rgb = (glowColor.rgb * glowColor.a);
		}
#else
		glowColor.rgb = (glowColor.rgb * glowColor.a);
#endif

		if (SHADER_GLOW_VIBRANCY != 0.0)
		{
			glowColor.rgb = Vibrancy( glowColor.rgb, SHADER_GLOW_VIBRANCY );
		}

		glowColor.rgb = clamp((clamp(glowColor.rgb - glow_const_1, 0.0, 1.0)) * glow_const_2, 0.0, 1.0);
		glowColor.rgb *= SHADER_GLOW_STRENGTH;

		if (SHADER_MATERIAL_TYPE != MATERIAL_GLASS && SHADER_MATERIAL_TYPE != MATERIAL_BLASTERBOLT && length(glowColor.rgb) <= 0.0)
			glowColor.a = 0.0;

		float glowMax = clamp(length(glowColor.rgb) / 3.0, 0.0, 1.0);//clamp(max(glowColor.r, max(glowColor.g, glowColor.b)), 0.0, 1.0);
		glowColor.a *= glowMax;
		glowColor.rgb *= glowColor.a;

		//float gMax = max(glowColor.r, max(glowColor.g, glowColor.b));
		//if (gMax > 1.0)
		//	glowColor.rgb = glowColor.rgb / gMax;

		glowColor.a = clamp(glowColor.a, 0.0, 1.0);
		out_Glow = glowColor;

		gl_FragColor.rgb = mix(gl_FragColor.rgb, glowColor.rgb, glowColor.a);
		gl_FragColor.a = max(gl_FragColor.a, glowColor.a);

		if (USE_ISDETAIL >= 1.0)
		{
			out_Position = vec4(0.0);
			out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		}
		else if (SHADER_MATERIAL_TYPE == MATERIAL_EFX)
		{
			out_Position = vec4(0.0);
			out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		}
		else if (gl_FragColor.a >= alphaThreshold || SHADER_MATERIAL_TYPE == 1024.0 || SHADER_MATERIAL_TYPE == 1025.0 || SHADER_MATERIAL_TYPE == MATERIAL_PUDDLE)
		{
			out_Position = vec4(m_vertPos.xyz, SHADER_MATERIAL_TYPE+1.0);
			out_Normal = vec4(vec3(EncodeNormal(N.xyz), 0.0), 1.0 );
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = norm;
#endif //__USE_REAL_NORMALMAPS__
		}
		else
		{
			out_Position = vec4(0.0);
			out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		}
	}
	else if (USE_GLOW_BUFFER > 0.0)
	{
		vec4 glowColor = max(gl_FragColor, 0.0);

		if (SHADER_MATERIAL_TYPE != MATERIAL_GLASS && length(glowColor.rgb) <= 0.0)
			glowColor.a = 0.0;

		if (SHADER_GLOW_VIBRANCY != 0.0)
		{
			glowColor.rgb = Vibrancy( glowColor.rgb, SHADER_GLOW_VIBRANCY );
		}

		glowColor.rgb = clamp((clamp(glowColor.rgb - glow_const_1, 0.0, 1.0)) * glow_const_2, 0.0, 1.0);
		glowColor.rgb *= SHADER_GLOW_STRENGTH;
		
		if (SHADER_MATERIAL_TYPE != MATERIAL_GLASS && SHADER_MATERIAL_TYPE != MATERIAL_BLASTERBOLT && length(glowColor.rgb) <= 0.0)
			glowColor.a = 0.0;

		//float gMax = max(glowColor.r, max(glowColor.g, glowColor.b));
		//if (gMax > 1.0)
		//	glowColor.rgb = glowColor.rgb / gMax;

		glowColor.a = clamp(glowColor.a, 0.0, 1.0);
		out_Glow = glowColor;

		gl_FragColor.rgb = glowColor.rgb;
		gl_FragColor.a = max(gl_FragColor.a, glowColor.a);

		if (USE_ISDETAIL >= 1.0)
		{
			out_Position = vec4(0.0);
			out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		}
		else if (SHADER_MATERIAL_TYPE == MATERIAL_EFX)
		{
			out_Position = vec4(0.0);
			out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		}
		else if (gl_FragColor.a >= alphaThreshold || SHADER_MATERIAL_TYPE == 1024.0 || SHADER_MATERIAL_TYPE == 1025.0 || SHADER_MATERIAL_TYPE == MATERIAL_PUDDLE)
		{
			out_Position = vec4(m_vertPos.xyz, SHADER_MATERIAL_TYPE+1.0);
			out_Normal = vec4(vec3(EncodeNormal(N.xyz), 0.0), 1.0 );
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		}
		else
		{
			out_Position = vec4(0.0);
			out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		}
	}
	else
	{
		out_Glow = vec4(0.0);

		if (USE_ISDETAIL >= 1.0)
		{
			out_Position = vec4(0.0);
			out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		}
		else if (SHADER_MATERIAL_TYPE == MATERIAL_EFX)
		{
			out_Position = vec4(0.0);
			out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		}
		else if (gl_FragColor.a >= alphaThreshold || SHADER_MATERIAL_TYPE == 1024.0 || SHADER_MATERIAL_TYPE == 1025.0 || SHADER_MATERIAL_TYPE == MATERIAL_PUDDLE)
		{
			out_Position = vec4(m_vertPos.xyz, SHADER_MATERIAL_TYPE+1.0);
			out_Normal = vec4(vec3(EncodeNormal(N.xyz), useDisplacementMapping), 1.0 );
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = norm;
#endif //__USE_REAL_NORMALMAPS__
		}
		else
		{
			out_Position = vec4(0.0);
			out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
			out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
		}
	}
}
