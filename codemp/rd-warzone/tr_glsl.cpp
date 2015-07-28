/*
===========================================================================
Copyright (C) 2006-2009 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_glsl.c
#include "tr_local.h"

void GLSL_BindNullProgram(void);

extern const char *fallbackShader_bokeh_vp;
extern const char *fallbackShader_bokeh_fp;
extern const char *fallbackShader_calclevels4x_vp;
extern const char *fallbackShader_calclevels4x_fp;
extern const char *fallbackShader_depthblur_vp;
extern const char *fallbackShader_depthblur_fp;
extern const char *fallbackShader_dlight_vp;
extern const char *fallbackShader_dlight_fp;
extern const char *fallbackShader_down4x_vp;
extern const char *fallbackShader_down4x_fp;
extern const char *fallbackShader_fogpass_vp;
extern const char *fallbackShader_fogpass_fp;
extern const char *fallbackShader_generic_vp;
extern const char *fallbackShader_generic_fp;
extern const char *fallbackShader_lightall_vp;
extern const char *fallbackShader_lightall_fp;
extern const char *fallbackShader_pshadow_vp;
extern const char *fallbackShader_pshadow_fp;
extern const char *fallbackShader_shadowfill_vp;
extern const char *fallbackShader_shadowfill_fp;
extern const char *fallbackShader_shadowmask_vp;
extern const char *fallbackShader_shadowmask_fp;
extern const char *fallbackShader_ssao_vp;
extern const char *fallbackShader_ssao_fp;
extern const char *fallbackShader_texturecolor_vp;
extern const char *fallbackShader_texturecolor_fp;
extern const char *fallbackShader_tonemap_vp;
extern const char *fallbackShader_tonemap_fp;
extern const char *fallbackShader_gaussian_blur_vp;
extern const char *fallbackShader_gaussian_blur_fp;

// UQ1: Added...
extern const char *fallbackShader_generateNormalMap_vp;
extern const char *fallbackShader_generateNormalMap_fp;
extern const char *fallbackShader_truehdr_vp;
extern const char *fallbackShader_truehdr_fp;
extern const char *fallbackShader_volumelight_vp;
extern const char *fallbackShader_volumelight_fp;
extern const char *fallbackShader_fakeDepth_vp;
extern const char *fallbackShader_fakeDepth_fp;
extern const char *fallbackShader_fakeDepthSteepParallax_vp;
extern const char *fallbackShader_fakeDepthSteepParallax_fp;
extern const char *fallbackShader_anaglyph_vp;
extern const char *fallbackShader_anaglyph_fp;
extern const char *fallbackShader_lightVolume_omni_vp;
extern const char *fallbackShader_lightVolume_omni_fp;
extern const char *fallbackShader_uniquesky_fp;
extern const char *fallbackShader_uniquesky_vp;
extern const char *fallbackShader_uniquewater_fp;
extern const char *fallbackShader_uniquewater_vp;
extern const char *fallbackShader_ssao2_vp;
extern const char *fallbackShader_ssao2_fp;
extern const char *fallbackShader_esharpening_vp;
extern const char *fallbackShader_esharpening_fp;
extern const char *fallbackShader_esharpening2_vp;
extern const char *fallbackShader_esharpening2_fp;
extern const char *fallbackShader_textureclean_vp;
extern const char *fallbackShader_textureclean_fp;
extern const char *fallbackShader_depthOfField_vp;
extern const char *fallbackShader_depthOfField_fp;
extern const char *fallbackShader_bloom_darken_vp;
extern const char *fallbackShader_bloom_darken_fp;
extern const char *fallbackShader_bloom_blur_vp;
extern const char *fallbackShader_bloom_blur_fp;
extern const char *fallbackShader_bloom_combine_vp;
extern const char *fallbackShader_bloom_combine_fp;
extern const char *fallbackShader_anamorphic_darken_vp;
extern const char *fallbackShader_anamorphic_darken_fp;
extern const char *fallbackShader_anamorphic_blur_vp;
extern const char *fallbackShader_anamorphic_blur_fp;
extern const char *fallbackShader_anamorphic_combine_vp;
extern const char *fallbackShader_anamorphic_combine_fp;
extern const char *fallbackShader_ssgi_vp;
extern const char *fallbackShader_ssgi_fp;
extern const char *fallbackShader_darkexpand_vp;
extern const char *fallbackShader_darkexpand_fp;
extern const char *fallbackShader_lensflare_vp;
extern const char *fallbackShader_lensflare_fp;
extern const char *fallbackShader_multipost_vp;
extern const char *fallbackShader_multipost_fp;
extern const char *fallbackShader_vibrancy_vp;
extern const char *fallbackShader_vibrancy_fp;
extern const char *fallbackShader_underwater_vp;
extern const char *fallbackShader_underwater_fp;
extern const char *fallbackShader_fxaa_vp;
extern const char *fallbackShader_fxaa_fp;

extern const char *fallbackShader_testshader_vp;
extern const char *fallbackShader_testshader_fp;


typedef struct uniformInfo_s
{
	char *name;
	int type;
	int size;
}
uniformInfo_t;

// These must be in the same order as in uniform_t in tr_local.h.
static uniformInfo_t uniformsInfo[] =
{
	{ "u_DiffuseMap",  GLSL_INT, 1 },
	{ "u_LightMap",    GLSL_INT, 1 },
	{ "u_NormalMap",   GLSL_INT, 1 },
	{ "u_DeluxeMap",   GLSL_INT, 1 },
	{ "u_SpecularMap", GLSL_INT, 1 },

	{ "u_TextureMap", GLSL_INT, 1 },
	{ "u_LevelsMap",  GLSL_INT, 1 },
	{ "u_CubeMap",    GLSL_INT, 1 },
	{ "u_SubsurfaceMap",    GLSL_INT, 1 },

	{ "u_ScreenImageMap", GLSL_INT, 1 },
	{ "u_ScreenDepthMap", GLSL_INT, 1 },

	{ "u_ShadowMap",  GLSL_INT, 1 },
	{ "u_ShadowMap2", GLSL_INT, 1 },
	{ "u_ShadowMap3", GLSL_INT, 1 },

	{ "u_ShadowMvp",  GLSL_MAT16, 1 },
	{ "u_ShadowMvp2", GLSL_MAT16, 1 },
	{ "u_ShadowMvp3", GLSL_MAT16, 1 },

	{ "u_EnableTextures", GLSL_VEC4, 1 },
	{ "u_DiffuseTexMatrix",  GLSL_VEC4, 1 },
	{ "u_DiffuseTexOffTurb", GLSL_VEC4, 1 },
	{ "u_Texture1Env",       GLSL_INT, 1 },

	{ "u_TCGen0",        GLSL_INT, 1 },
	{ "u_TCGen0Vector0", GLSL_VEC3, 1 },
	{ "u_TCGen0Vector1", GLSL_VEC3, 1 },

	{ "u_DeformGen",    GLSL_INT, 1 },
	{ "u_DeformParams", GLSL_FLOAT5, 1 },

	{ "u_ColorGen",  GLSL_INT, 1 },
	{ "u_AlphaGen",  GLSL_INT, 1 },
	{ "u_Color",     GLSL_VEC4, 1 },
	{ "u_BaseColor", GLSL_VEC4, 1 },
	{ "u_VertColor", GLSL_VEC4, 1 },

	{ "u_DlightInfo",    GLSL_VEC4, 1 },
	{ "u_LightForward",  GLSL_VEC3, 1 },
	{ "u_LightUp",       GLSL_VEC3, 1 },
	{ "u_LightRight",    GLSL_VEC3, 1 },
	{ "u_LightOrigin",   GLSL_VEC4, 1 },
	{ "u_LightOrigin1",   GLSL_VEC4, 1 },
	{ "u_LightOrigin2",   GLSL_VEC4, 1 },
	{ "u_LightOrigin3",   GLSL_VEC4, 1 },
	{ "u_LightOrigin4",   GLSL_VEC4, 1 },
	{ "u_LightOrigin5",   GLSL_VEC4, 1 },
	{ "u_LightOrigin6",   GLSL_VEC4, 1 },
	{ "u_LightOrigin7",   GLSL_VEC4, 1 },
	{ "u_LightOrigin8",   GLSL_VEC4, 1 },
	{ "u_LightOrigin9",   GLSL_VEC4, 1 },
	{ "u_LightOrigin10",   GLSL_VEC4, 1 },
	{ "u_LightOrigin11",   GLSL_VEC4, 1 },
	{ "u_LightOrigin12",   GLSL_VEC4, 1 },
	{ "u_LightOrigin13",   GLSL_VEC4, 1 },
	{ "u_LightOrigin14",   GLSL_VEC4, 1 },
	{ "u_LightOrigin15",   GLSL_VEC4, 1 },
	{ "u_LightOrigin16",   GLSL_VEC4, 1 },
	{ "u_LightOrigin17",   GLSL_VEC4, 1 },
	{ "u_LightOrigin18",   GLSL_VEC4, 1 },
	{ "u_LightOrigin19",   GLSL_VEC4, 1 },
	{ "u_LightOrigin20",   GLSL_VEC4, 1 },
	{ "u_LightOrigin21",   GLSL_VEC4, 1 },
	{ "u_LightOrigin22",   GLSL_VEC4, 1 },
	{ "u_LightOrigin23",   GLSL_VEC4, 1 },
	{ "u_LightOrigin24",   GLSL_VEC4, 1 },
	{ "u_LightOrigin25",   GLSL_VEC4, 1 },
	{ "u_LightOrigin26",   GLSL_VEC4, 1 },
	{ "u_LightOrigin27",   GLSL_VEC4, 1 },
	{ "u_LightOrigin28",   GLSL_VEC4, 1 },
	{ "u_LightOrigin29",   GLSL_VEC4, 1 },
	{ "u_LightOrigin30",   GLSL_VEC4, 1 },
	{ "u_LightOrigin31",   GLSL_VEC4, 1 },
	{ "u_LightColor",   GLSL_VEC4, 1 },
	{ "u_LightColor1",   GLSL_VEC4, 1 },
	{ "u_LightColor2",   GLSL_VEC4, 1 },
	{ "u_LightColor3",   GLSL_VEC4, 1 },
	{ "u_LightColor4",   GLSL_VEC4, 1 },
	{ "u_LightColor5",   GLSL_VEC4, 1 },
	{ "u_LightColor6",   GLSL_VEC4, 1 },
	{ "u_LightColor7",   GLSL_VEC4, 1 },
	{ "u_LightColor8",   GLSL_VEC4, 1 },
	{ "u_LightColor9",   GLSL_VEC4, 1 },
	{ "u_LightColor10",   GLSL_VEC4, 1 },
	{ "u_LightColor11",   GLSL_VEC4, 1 },
	{ "u_LightColor12",   GLSL_VEC4, 1 },
	{ "u_LightColor13",   GLSL_VEC4, 1 },
	{ "u_LightColor14",   GLSL_VEC4, 1 },
	{ "u_LightColor15",   GLSL_VEC4, 1 },
	{ "u_LightColor16",   GLSL_VEC4, 1 },
	{ "u_LightColor17",   GLSL_VEC4, 1 },
	{ "u_LightColor18",   GLSL_VEC4, 1 },
	{ "u_LightColor19",   GLSL_VEC4, 1 },
	{ "u_LightColor20",   GLSL_VEC4, 1 },
	{ "u_LightColor21",   GLSL_VEC4, 1 },
	{ "u_LightColor22",   GLSL_VEC4, 1 },
	{ "u_LightColor23",   GLSL_VEC4, 1 },
	{ "u_LightColor24",   GLSL_VEC4, 1 },
	{ "u_LightColor25",   GLSL_VEC4, 1 },
	{ "u_LightColor26",   GLSL_VEC4, 1 },
	{ "u_LightColor27",   GLSL_VEC4, 1 },
	{ "u_LightColor28",   GLSL_VEC4, 1 },
	{ "u_LightColor29",   GLSL_VEC4, 1 },
	{ "u_LightColor30",   GLSL_VEC4, 1 },
	{ "u_LightColor31",   GLSL_VEC4, 1 },
	{ "u_ModelLightDir", GLSL_VEC3, 1 },
	{ "u_LightRadius",   GLSL_FLOAT, 1 },
	{ "u_LightRadius1",   GLSL_FLOAT, 1 },
	{ "u_LightRadius2",   GLSL_FLOAT, 1 },
	{ "u_LightRadius3",   GLSL_FLOAT, 1 },
	{ "u_LightRadius4",   GLSL_FLOAT, 1 },
	{ "u_LightRadius5",   GLSL_FLOAT, 1 },
	{ "u_LightRadius6",   GLSL_FLOAT, 1 },
	{ "u_LightRadius7",   GLSL_FLOAT, 1 },
	{ "u_LightRadius8",   GLSL_FLOAT, 1 },
	{ "u_LightRadius9",   GLSL_FLOAT, 1 },
	{ "u_LightRadius10",   GLSL_FLOAT, 1 },
	{ "u_LightRadius11",   GLSL_FLOAT, 1 },
	{ "u_LightRadius12",   GLSL_FLOAT, 1 },
	{ "u_LightRadius13",   GLSL_FLOAT, 1 },
	{ "u_LightRadius14",   GLSL_FLOAT, 1 },
	{ "u_LightRadius15",   GLSL_FLOAT, 1 },
	{ "u_LightRadius16",   GLSL_FLOAT, 1 },
	{ "u_LightRadius17",   GLSL_FLOAT, 1 },
	{ "u_LightRadius18",   GLSL_FLOAT, 1 },
	{ "u_LightRadius19",   GLSL_FLOAT, 1 },
	{ "u_LightRadius20",   GLSL_FLOAT, 1 },
	{ "u_LightRadius21",   GLSL_FLOAT, 1 },
	{ "u_LightRadius22",   GLSL_FLOAT, 1 },
	{ "u_LightRadius23",   GLSL_FLOAT, 1 },
	{ "u_LightRadius24",   GLSL_FLOAT, 1 },
	{ "u_LightRadius25",   GLSL_FLOAT, 1 },
	{ "u_LightRadius26",   GLSL_FLOAT, 1 },
	{ "u_LightRadius27",   GLSL_FLOAT, 1 },
	{ "u_LightRadius28",   GLSL_FLOAT, 1 },
	{ "u_LightRadius29",   GLSL_FLOAT, 1 },
	{ "u_LightRadius30",   GLSL_FLOAT, 1 },
	{ "u_LightRadius31",   GLSL_FLOAT, 1 },
	{ "u_AmbientLight",  GLSL_VEC3, 1 },
	{ "u_DirectedLight", GLSL_VEC3, 1 },

	{ "u_PortalRange", GLSL_FLOAT, 1 },

	{ "u_FogDistance",  GLSL_VEC4, 1 },
	{ "u_FogDepth",     GLSL_VEC4, 1 },
	{ "u_FogEyeT",      GLSL_FLOAT, 1 },
	{ "u_FogColorMask", GLSL_VEC4, 1 },

	{ "u_ModelMatrix",               GLSL_MAT16, 1 },
	{ "u_ModelViewProjectionMatrix", GLSL_MAT16, 1 },

	{ "u_Time",          GLSL_FLOAT, 1 },
	{ "u_VertexLerp" ,   GLSL_FLOAT, 1 },
	{ "u_NormalScale",   GLSL_VEC4, 1 },
	{ "u_SpecularScale", GLSL_VEC4, 1 },

	{ "u_ViewInfo",				GLSL_VEC4, 1 },
	{ "u_ViewOrigin",			GLSL_VEC3, 1 },
	{ "u_LocalViewOrigin",		GLSL_VEC3, 1 },
	{ "u_ViewForward",			GLSL_VEC3, 1 },
	{ "u_ViewLeft",				GLSL_VEC3, 1 },
	{ "u_ViewUp",				GLSL_VEC3, 1 },

	{ "u_InvTexRes",           GLSL_VEC2, 1 },
	{ "u_AutoExposureMinMax",  GLSL_VEC2, 1 },
	{ "u_ToneMinAvgMaxLinear", GLSL_VEC3, 1 },

	{ "u_PrimaryLightOrigin",  GLSL_VEC4, 1  },
	{ "u_PrimaryLightColor",   GLSL_VEC3, 1  },
	{ "u_PrimaryLightAmbient", GLSL_VEC3, 1  },
	{ "u_PrimaryLightRadius",  GLSL_FLOAT, 1 },

	{ "u_CubeMapInfo", GLSL_VEC4, 1 },

	{ "u_BoneMatrices",			GLSL_MAT16, 80 },

	// UQ1: Added...
	{ "u_Dimensions",           GLSL_VEC2, 1 },
	{ "u_HeightMap",			GLSL_INT, 1 },
	{ "u_Local0",				GLSL_VEC4, 1  },
	{ "u_Local1",				GLSL_VEC4, 1  },
	{ "u_Local2",				GLSL_VEC4, 1  },
	{ "u_Local3",				GLSL_VEC4, 1  },
	{ "u_Local4",				GLSL_VEC4, 1  },
	{ "u_Local5",				GLSL_VEC4, 1  },
	{ "u_Texture0",				GLSL_INT, 1 },
	{ "u_Texture1",				GLSL_INT, 1 },
	{ "u_Texture2",				GLSL_INT, 1 },
	{ "u_Texture3",				GLSL_INT, 1 },
};

static void GLSL_PrintProgramInfoLog(GLuint object, qboolean developerOnly)
{
	char           *msg;
	static char     msgPart[1024];
	int             maxLength = 0;
	int             i;
	int             printLevel = developerOnly ? PRINT_DEVELOPER : PRINT_ALL;

	qglGetProgramiv(object, GL_INFO_LOG_LENGTH, &maxLength);

	if (maxLength <= 0)
	{
		ri->Printf(printLevel, "No compile log.\n");
		return;
	}

	ri->Printf(printLevel, "compile log:\n");

	if (maxLength < 1023)
	{
		qglGetProgramInfoLog(object, maxLength, &maxLength, msgPart);

		msgPart[maxLength + 1] = '\0';

		ri->Printf(printLevel, "%s\n", msgPart);
	}
	else
	{
		msg = (char *)Z_Malloc(maxLength, TAG_SHADERTEXT);

		qglGetProgramInfoLog(object, maxLength, &maxLength, msg);

		for(i = 0; i < maxLength; i += 1024)
		{
			Q_strncpyz(msgPart, msg + i, sizeof(msgPart));

			ri->Printf(printLevel, "%s\n", msgPart);
		}

		Z_Free(msg);
	}
}

static void GLSL_PrintShaderInfoLog(GLuint object, qboolean developerOnly)
{
	char           *msg;
	static char     msgPart[1024];
	int             maxLength = 0;
	int             i;
	int             printLevel = developerOnly ? PRINT_DEVELOPER : PRINT_ALL;

	qglGetShaderiv(object, GL_INFO_LOG_LENGTH, &maxLength);

	if (maxLength <= 0)
	{
		ri->Printf(printLevel, "No compile log.\n");
		return;
	}

	ri->Printf(printLevel, "compile log:\n");

	if (maxLength < 1023)
	{
		qglGetShaderInfoLog(object, maxLength, &maxLength, msgPart);

		msgPart[maxLength + 1] = '\0';

		ri->Printf(printLevel, "%s\n", msgPart);
	}
	else
	{
		msg = (char *)Z_Malloc(maxLength, TAG_SHADERTEXT);

		qglGetShaderInfoLog(object, maxLength, &maxLength, msg);

		for(i = 0; i < maxLength; i += 1024)
		{
			Q_strncpyz(msgPart, msg + i, sizeof(msgPart));

			ri->Printf(printLevel, "%s\n", msgPart);
		}

		Z_Free(msg);
	}
}

static void GLSL_PrintShaderSource(GLuint shader)
{
	char           *msg;
	static char     msgPart[1024];
	int             maxLength = 0;
	int             i;

	qglGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &maxLength);

	msg = (char *)Z_Malloc(maxLength, TAG_SHADERTEXT);

	qglGetShaderSource(shader, maxLength, &maxLength, msg);

	for(i = 0; i < maxLength; i += 1024)
	{
		Q_strncpyz(msgPart, msg + i, sizeof(msgPart));
		ri->Printf(PRINT_ALL, "%s\n", msgPart);
	}

	Z_Free(msg);
}

static void GLSL_GetShaderHeader( GLenum shaderType, const GLcharARB *extra, char *dest, int size )
{
	float fbufWidthScale, fbufHeightScale;

	dest[0] = '\0';

	Q_strcat(dest, size, "#version 150 core\n");

	if(shaderType == GL_VERTEX_SHADER)
	{
		Q_strcat(dest, size, "#define attribute in\n");
		Q_strcat(dest, size, "#define varying out\n");
	}
	else
	{
		Q_strcat(dest, size, "#define varying in\n");

		Q_strcat(dest, size, "out vec4 out_Color;\n");
		Q_strcat(dest, size, "#define gl_FragColor out_Color\n");
	}

	// HACK: add some macros to avoid extra uniforms and save speed and code maintenance
	//Q_strcat(dest, size,
	//		 va("#ifndef r_SpecularExponent\n#define r_SpecularExponent %f\n#endif\n", r_specularExponent->value));
	//Q_strcat(dest, size,
	//		 va("#ifndef r_SpecularScale\n#define r_SpecularScale %f\n#endif\n", r_specularScale->value));
	//Q_strcat(dest, size,
	//       va("#ifndef r_NormalScale\n#define r_NormalScale %f\n#endif\n", r_normalScale->value));


	Q_strcat(dest, size, "#ifndef M_PI\n#define M_PI 3.14159265358979323846\n#endif\n");

	//Q_strcat(dest, size, va("#ifndef MAX_SHADOWMAPS\n#define MAX_SHADOWMAPS %i\n#endif\n", MAX_SHADOWMAPS));

	Q_strcat(dest, size,
					 va("#ifndef deformGen_t\n"
						"#define deformGen_t\n"
						"#define DGEN_WAVE_SIN %i\n"
						"#define DGEN_WAVE_SQUARE %i\n"
						"#define DGEN_WAVE_TRIANGLE %i\n"
						"#define DGEN_WAVE_SAWTOOTH %i\n"
						"#define DGEN_WAVE_INVERSE_SAWTOOTH %i\n"
						"#define DGEN_BULGE %i\n"
						"#define DGEN_MOVE %i\n"
						"#endif\n",
						DGEN_WAVE_SIN,
						DGEN_WAVE_SQUARE,
						DGEN_WAVE_TRIANGLE,
						DGEN_WAVE_SAWTOOTH,
						DGEN_WAVE_INVERSE_SAWTOOTH,
						DGEN_BULGE,
						DGEN_MOVE));

	Q_strcat(dest, size,
					 va("#ifndef tcGen_t\n"
						"#define tcGen_t\n"
						"#define TCGEN_LIGHTMAP %i\n"
						"#define TCGEN_LIGHTMAP1 %i\n"
						"#define TCGEN_LIGHTMAP2 %i\n"
						"#define TCGEN_LIGHTMAP3 %i\n"
						"#define TCGEN_TEXTURE %i\n"
						"#define TCGEN_ENVIRONMENT_MAPPED %i\n"
						"#define TCGEN_FOG %i\n"
						"#define TCGEN_VECTOR %i\n"
						"#endif\n",
						TCGEN_LIGHTMAP,
						TCGEN_LIGHTMAP1,
						TCGEN_LIGHTMAP2,
						TCGEN_LIGHTMAP3,
						TCGEN_TEXTURE,
						TCGEN_ENVIRONMENT_MAPPED,
						TCGEN_FOG,
						TCGEN_VECTOR));

	Q_strcat(dest, size,
					 va("#ifndef colorGen_t\n"
						"#define colorGen_t\n"
						"#define CGEN_LIGHTING_DIFFUSE %i\n"
						"#endif\n",
						CGEN_LIGHTING_DIFFUSE));

	Q_strcat(dest, size,
							 va("#ifndef alphaGen_t\n"
								"#define alphaGen_t\n"
								"#define AGEN_LIGHTING_SPECULAR %i\n"
								"#define AGEN_PORTAL %i\n"
								"#endif\n",
								AGEN_LIGHTING_SPECULAR,
								AGEN_PORTAL));

	Q_strcat(dest, size,
							 va("#ifndef texenv_t\n"
								"#define texenv_t\n"
								"#define TEXENV_MODULATE %i\n"
								"#define TEXENV_ADD %i\n"
								"#define TEXENV_REPLACE %i\n"
								"#endif\n",
								GL_MODULATE,
								GL_ADD,
								GL_REPLACE));

	fbufWidthScale = 1.0f / ((float)glConfig.vidWidth);
	fbufHeightScale = 1.0f / ((float)glConfig.vidHeight);
	Q_strcat(dest, size,
			 va("#ifndef r_FBufScale\n#define r_FBufScale vec2(%f, %f)\n#endif\n", fbufWidthScale, fbufHeightScale));

	if (extra)
	{
		Q_strcat(dest, size, extra);
	}

	// OK we added a lot of stuff but if we do something bad in the GLSL shaders then we want the proper line
	// so we have to reset the line counting
	Q_strcat(dest, size, "#line 0\n");
}

static int GLSL_EnqueueCompileGPUShader(GLuint program, GLuint *prevShader, const GLchar *buffer, int size, GLenum shaderType)
{
	GLuint     shader;

	shader = qglCreateShader(shaderType);

	qglShaderSource(shader, 1, &buffer, &size);

	// compile shader
	qglCompileShader(shader);

	*prevShader = shader;

	return 1;
}

static int GLSL_LoadGPUShaderText(const char *name, const char *fallback,
	GLenum shaderType, char *dest, int destSize)
{
	char            filename[MAX_QPATH];
	GLcharARB      *buffer = NULL;
	const GLcharARB *shaderText = NULL;
	int             size;
	int             result;

	if(shaderType == GL_VERTEX_SHADER)
	{
		Com_sprintf(filename, sizeof(filename), "glsl/%s_vp.glsl", name);
	}
	else
	{
		Com_sprintf(filename, sizeof(filename), "glsl/%s_fp.glsl", name);
	}

	ri->Printf(PRINT_DEVELOPER, "...loading '%s'\n", filename);
	size = ri->FS_ReadFile(filename, (void **)&buffer);
	if(!buffer)
	{
		if (fallback)
		{
			ri->Printf(PRINT_DEVELOPER, "couldn't load, using fallback\n");
			shaderText = fallback;
			size = strlen(shaderText);
		}
		else
		{
			ri->Printf(PRINT_DEVELOPER, "couldn't load!\n");
			return 0;
		}
	}
	else
	{
		shaderText = buffer;
	}

	if (size > destSize)
	{
		result = 0;
	}
	else
	{
		Q_strncpyz(dest, shaderText, size + 1);
		result = 1;
	}

	if (buffer)
	{
		ri->FS_FreeFile(buffer);
	}
	
	return result;
}

static void GLSL_LinkProgram(GLuint program)
{
	GLint           linked;

	qglLinkProgram(program);

	qglGetProgramiv(program, GL_LINK_STATUS, &linked);
	if(!linked)
	{
		GLSL_PrintProgramInfoLog(program, qfalse);
		ri->Printf(PRINT_ALL, "\n");
		ri->Error(ERR_DROP, "shaders failed to link");
	}
}

static void GLSL_ValidateProgram(GLuint program)
{
	GLint           validated;

	qglValidateProgram(program);

	qglGetProgramiv(program, GL_VALIDATE_STATUS, &validated);
	if(!validated)
	{
		GLSL_PrintProgramInfoLog(program, qfalse);
		ri->Printf(PRINT_ALL, "\n");
		ri->Error(ERR_DROP, "shaders failed to validate");
	}
}

static void GLSL_ShowProgramUniforms(GLuint program)
{
	int             i, count, size;
	GLenum			type;
	char            uniformName[1000];

	// install the executables in the program object as part of current state.
	qglUseProgram(program);

	// check for GL Errors

	// query the number of active uniforms
	qglGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);

	// Loop over each of the active uniforms, and set their value
	for(i = 0; i < count; i++)
	{
		qglGetActiveUniform(program, i, sizeof(uniformName), NULL, &size, &type, uniformName);

		ri->Printf(PRINT_DEVELOPER, "active uniform: '%s'\n", uniformName);
	}

	qglUseProgram(0);
}

static int GLSL_BeginLoadGPUShader2(shaderProgram_t * program, const char *name, int attribs, const char *vpCode, const char *fpCode)
{
	size_t nameBufSize = strlen (name) + 1;

	ri->Printf(PRINT_DEVELOPER, "------- GPU shader -------\n");

	program->name = (char *)Z_Malloc (nameBufSize, TAG_GENERAL);
	Q_strncpyz(program->name, name, nameBufSize);

	program->program = qglCreateProgram();
	program->attribs = attribs;

	if (!(GLSL_EnqueueCompileGPUShader(program->program, &program->vertexShader, vpCode, strlen(vpCode), GL_VERTEX_SHADER)))
	{
		ri->Printf(PRINT_ALL, "GLSL_BeginLoadGPUShader2: Unable to load \"%s\" as GL_VERTEX_SHADER\n", name);
		qglDeleteProgram(program->program);
		return 0;
	}

	if(fpCode)
	{
		if(!(GLSL_EnqueueCompileGPUShader(program->program, &program->fragmentShader, fpCode, strlen(fpCode), GL_FRAGMENT_SHADER)))
		{
			ri->Printf(PRINT_ALL, "GLSL_BeginLoadGPUShader2: Unable to load \"%s\" as GL_FRAGMENT_SHADER\n", name);
			qglDeleteProgram(program->program);
			return 0;
		}
	}
	
	return 1;
}

static bool GLSL_IsGPUShaderCompiled (GLuint shader)
{
	GLint compiled;

	qglGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if(!compiled)
	{
		GLSL_PrintShaderSource(shader);
		GLSL_PrintShaderInfoLog(shader, qfalse);
		ri->Error(ERR_DROP, "Couldn't compile shader");
		return qfalse;
	}

	return qtrue;
}

static bool GLSL_EndLoadGPUShader (shaderProgram_t *program)
{
	uint32_t attribs = program->attribs;

	//ri->Printf(PRINT_WARNING, "Compiling glsl: %s.\n", program->name);

	if (!GLSL_IsGPUShaderCompiled (program->vertexShader))
	{
		return false;
	}

	if (!GLSL_IsGPUShaderCompiled (program->fragmentShader))
	{
		return false;
	}

	qglAttachShader(program->program, program->vertexShader);
	qglAttachShader(program->program, program->fragmentShader);

	qglBindFragDataLocation (program->program, 0, "out_Color");
	qglBindFragDataLocation (program->program, 1, "out_Glow");

	if(attribs & ATTR_POSITION)
		qglBindAttribLocation(program->program, ATTR_INDEX_POSITION, "attr_Position");

	if(attribs & ATTR_TEXCOORD0)
		qglBindAttribLocation(program->program, ATTR_INDEX_TEXCOORD0, "attr_TexCoord0");

	if(attribs & ATTR_TEXCOORD1)
		qglBindAttribLocation(program->program, ATTR_INDEX_TEXCOORD1, "attr_TexCoord1");

#ifdef USE_VERT_TANGENT_SPACE
	if(attribs & ATTR_TANGENT)
		qglBindAttribLocation(program->program, ATTR_INDEX_TANGENT, "attr_Tangent");
#endif

	if(attribs & ATTR_NORMAL)
		qglBindAttribLocation(program->program, ATTR_INDEX_NORMAL, "attr_Normal");

	if(attribs & ATTR_COLOR)
		qglBindAttribLocation(program->program, ATTR_INDEX_COLOR, "attr_Color");

	if(attribs & ATTR_PAINTCOLOR)
		qglBindAttribLocation(program->program, ATTR_INDEX_PAINTCOLOR, "attr_PaintColor");

	if(attribs & ATTR_LIGHTDIRECTION)
		qglBindAttribLocation(program->program, ATTR_INDEX_LIGHTDIRECTION, "attr_LightDirection");

	if(attribs & ATTR_POSITION2)
		qglBindAttribLocation(program->program, ATTR_INDEX_POSITION2, "attr_Position2");

	if(attribs & ATTR_NORMAL2)
		qglBindAttribLocation(program->program, ATTR_INDEX_NORMAL2, "attr_Normal2");

#ifdef USE_VERT_TANGENT_SPACE
	if(attribs & ATTR_TANGENT2)
		qglBindAttribLocation(program->program, ATTR_INDEX_TANGENT2, "attr_Tangent2");
#endif

	if(attribs & ATTR_BONE_INDEXES)
		qglBindAttribLocation(program->program, ATTR_INDEX_BONE_INDEXES, "attr_BoneIndexes");

	if(attribs & ATTR_BONE_WEIGHTS)
		qglBindAttribLocation(program->program, ATTR_INDEX_BONE_WEIGHTS, "attr_BoneWeights");

	GLSL_LinkProgram(program->program);

	// Won't be needing these anymore...
	qglDetachShader (program->program, program->vertexShader);
	qglDetachShader (program->program, program->fragmentShader);

	qglDeleteShader (program->vertexShader);
	qglDeleteShader (program->fragmentShader);

	program->vertexShader = program->fragmentShader = 0;

	return true;
}

static int GLSL_BeginLoadGPUShader(shaderProgram_t * program, const char *name,
	int attribs, qboolean fragmentShader, const GLcharARB *extra, qboolean addHeader,
	const char *fallback_vp, const char *fallback_fp)
{
	char vpCode[32000];
	char fpCode[32000];
	char *postHeader;
	int size;

	size = sizeof(vpCode);
	if (addHeader)
	{
		GLSL_GetShaderHeader(GL_VERTEX_SHADER, extra, vpCode, size);
		postHeader = &vpCode[strlen(vpCode)];
		size -= strlen(vpCode);
	}
	else
	{
		postHeader = &vpCode[0];
	}

	if (!GLSL_LoadGPUShaderText(name, fallback_vp, GL_VERTEX_SHADER, postHeader, size))
	{
		return 0;
	}

	if (fragmentShader)
	{
		size = sizeof(fpCode);
		if (addHeader)
		{
			GLSL_GetShaderHeader(GL_FRAGMENT_SHADER, extra, fpCode, size);
			postHeader = &fpCode[strlen(fpCode)];
			size -= strlen(fpCode);
		}
		else
		{
			postHeader = &fpCode[0];
		}

		if (!GLSL_LoadGPUShaderText(name, fallback_fp, GL_FRAGMENT_SHADER, postHeader, size))
		{
			return 0;
		}
	}

	return GLSL_BeginLoadGPUShader2(program, name, attribs, vpCode, fragmentShader ? fpCode : NULL);
}

void GLSL_InitUniforms(shaderProgram_t *program)
{
	int i, size;

	GLint *uniforms;
	
	program->uniforms = (GLint *)Z_Malloc (UNIFORM_COUNT * sizeof (*program->uniforms), TAG_GENERAL);
	program->uniformBufferOffsets = (short *)Z_Malloc (UNIFORM_COUNT * sizeof (*program->uniformBufferOffsets), TAG_GENERAL);

	uniforms = program->uniforms;

	size = 0;
	for (i = 0; i < UNIFORM_COUNT; i++)
	{
		uniforms[i] = qglGetUniformLocation(program->program, uniformsInfo[i].name);

		if (uniforms[i] == -1)
			continue;
		 
		program->uniformBufferOffsets[i] = size;

		switch(uniformsInfo[i].type)
		{
			case GLSL_INT:
				size += sizeof(GLint) * uniformsInfo[i].size;
				break;
			case GLSL_FLOAT:
				size += sizeof(GLfloat) * uniformsInfo[i].size;
				break;
			case GLSL_FLOAT5:
				size += sizeof(float) * 5 * uniformsInfo[i].size;
				break;
			case GLSL_VEC2:
				size += sizeof(float) * 2 * uniformsInfo[i].size;
				break;
			case GLSL_VEC3:
				size += sizeof(float) * 3 * uniformsInfo[i].size;
				break;
			case GLSL_VEC4:
				size += sizeof(float) * 4 * uniformsInfo[i].size;
				break;
			case GLSL_MAT16:
				size += sizeof(float) * 16 * uniformsInfo[i].size;
				break;
			default:
				break;
		}
	}

	program->uniformBuffer = (char *)Z_Malloc(size, TAG_SHADERTEXT, qtrue);
}

void GLSL_FinishGPUShader(shaderProgram_t *program)
{
	GLSL_ValidateProgram(program->program);
	GLSL_ShowProgramUniforms(program->program);
	GL_CheckErrors();
}

void GLSL_SetUniformInt(shaderProgram_t *program, int uniformNum, GLint value)
{
	GLint *uniforms = program->uniforms;
	GLint *compare = (GLint *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_INT)
	{
		ri->Printf( PRINT_WARNING, "GLSL_SetUniformInt: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (value == *compare)
	{
		return;
	}

	*compare = value;

	qglUniform1i(uniforms[uniformNum], value);
}

void GLSL_SetUniformFloat(shaderProgram_t *program, int uniformNum, GLfloat value)
{
	GLint *uniforms = program->uniforms;
	GLfloat *compare = (GLfloat *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_FLOAT)
	{
		ri->Printf( PRINT_WARNING, "GLSL_SetUniformFloat: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (value == *compare)
	{
		return;
	}

	*compare = value;
	
	qglUniform1f(uniforms[uniformNum], value);
}

void GLSL_SetUniformVec2(shaderProgram_t *program, int uniformNum, const vec2_t v)
{
	GLint *uniforms = program->uniforms;
	float *compare = (float *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_VEC2)
	{
		ri->Printf( PRINT_WARNING, "GLSL_SetUniformVec2: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (v[0] == compare[0] && v[1] == compare[1])
	{
		return;
	}

	compare[0] = v[0];
	compare[1] = v[1];

	qglUniform2f(uniforms[uniformNum], v[0], v[1]);
}

void GLSL_SetUniformVec3(shaderProgram_t *program, int uniformNum, const vec3_t v)
{
	GLint *uniforms = program->uniforms;
	float *compare = (float *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_VEC3)
	{
		ri->Printf( PRINT_WARNING, "GLSL_SetUniformVec3: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (VectorCompare(v, compare))
	{
		return;
	}

	VectorCopy(v, compare);

	qglUniform3f(uniforms[uniformNum], v[0], v[1], v[2]);
}

void GLSL_SetUniformVec4(shaderProgram_t *program, int uniformNum, const vec4_t v)
{
	GLint *uniforms = program->uniforms;
	float *compare = (float *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_VEC4)
	{
		ri->Printf( PRINT_WARNING, "GLSL_SetUniformVec4: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (VectorCompare4(v, compare))
	{
		return;
	}

	VectorCopy4(v, compare);

	qglUniform4f(uniforms[uniformNum], v[0], v[1], v[2], v[3]);
}

void GLSL_SetUniformFloat5(shaderProgram_t *program, int uniformNum, const vec5_t v)
{
	GLint *uniforms = program->uniforms;
	float *compare = (float *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_FLOAT5)
	{
		ri->Printf( PRINT_WARNING, "GLSL_SetUniformFloat5: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (VectorCompare5(v, compare))
	{
		return;
	}

	VectorCopy5(v, compare);

	qglUniform1fv(uniforms[uniformNum], 5, v);
}

void GLSL_SetUniformMatrix16(shaderProgram_t *program, int uniformNum, const float *matrix, int numElements)
{
	GLint *uniforms = program->uniforms;
	float *compare;

	if (uniforms[uniformNum] == -1)
		return;

	if (uniformsInfo[uniformNum].type != GLSL_MAT16)
	{
		ri->Printf( PRINT_WARNING, "GLSL_SetUniformMatrix16: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}

	if (uniformsInfo[uniformNum].size < numElements)
		return;

	compare = (float *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);
	if (memcmp (matrix, compare, sizeof (float) * 16 * numElements) == 0)
	{
		return;
	}

	Com_Memcpy (compare, matrix, sizeof (float) * 16 * numElements);

	qglUniformMatrix4fv(uniforms[uniformNum], numElements, GL_FALSE, matrix);
}

void GLSL_DeleteGPUShader(shaderProgram_t *program)
{
	if(program->program)
	{
		qglDeleteProgram(program->program);

		Z_Free (program->name);
		Z_Free (program->uniformBuffer);
		Z_Free (program->uniformBufferOffsets);
		Z_Free (program->uniforms);

		Com_Memset(program, 0, sizeof(*program));
	}
}

static bool GLSL_IsValidPermutationForGeneric (int shaderCaps)
{
	if ((shaderCaps & (GENERICDEF_USE_VERTEX_ANIMATION | GENERICDEF_USE_SKELETAL_ANIMATION)) == (GENERICDEF_USE_VERTEX_ANIMATION | GENERICDEF_USE_SKELETAL_ANIMATION))
	{
		return false;
	}

	return true;
}

static bool GLSL_IsValidPermutationForFog (int shaderCaps)
{
	if ((shaderCaps & (FOGDEF_USE_VERTEX_ANIMATION | FOGDEF_USE_SKELETAL_ANIMATION)) == (FOGDEF_USE_VERTEX_ANIMATION | FOGDEF_USE_SKELETAL_ANIMATION))
	{
		return false;
	}

	return true;
}

static bool GLSL_IsValidPermutationForLight (int lightType, int shaderCaps)
{
	if ((shaderCaps & LIGHTDEF_USE_PARALLAXMAP) && !r_parallaxMapping->integer)
		return false;

	if (!lightType && (shaderCaps & LIGHTDEF_USE_PARALLAXMAP))
		return false;

	if (!lightType && (shaderCaps & LIGHTDEF_USE_SHADOWMAP))
		return false;

	return true;
}

int GLSL_BeginLoadGPUShaders(void)
{
	int startTime;
	int i;
	char extradefines[1024];
	int attribs;

	ri->Printf(PRINT_ALL, "------- GLSL_InitGPUShaders -------\n");

	R_IssuePendingRenderCommands();

	startTime = ri->Milliseconds();

	for (i = 0; i < GENERICDEF_COUNT; i++)
	{
		if (!GLSL_IsValidPermutationForGeneric (i))
		{
			continue;
		}

		attribs = ATTR_POSITION | ATTR_TEXCOORD0 | ATTR_NORMAL | ATTR_COLOR | ATTR_LIGHTDIRECTION;;
		extradefines[0] = '\0';

		if (i & GENERICDEF_USE_DEFORM_VERTEXES)
			Q_strcat(extradefines, 1024, "#define USE_DEFORM_VERTEXES\n");

		if (i & GENERICDEF_USE_TCGEN_AND_TCMOD)
		{
			Q_strcat(extradefines, 1024, "#define USE_TCGEN\n");
			Q_strcat(extradefines, 1024, "#define USE_TCMOD\n");
		}

		if (i & GENERICDEF_USE_VERTEX_ANIMATION)
		{
			Q_strcat(extradefines, 1024, "#define USE_VERTEX_ANIMATION\n");
			attribs |= ATTR_POSITION2 | ATTR_NORMAL2;
		}

		if (i & GENERICDEF_USE_SKELETAL_ANIMATION)
		{
			Q_strcat(extradefines, 1024, "#define USE_SKELETAL_ANIMATION\n");
			attribs |= ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS;
		}

		if (i & GENERICDEF_USE_FOG)
			Q_strcat(extradefines, 1024, "#define USE_FOG\n");

		if (i & GENERICDEF_USE_RGBAGEN)
			Q_strcat(extradefines, 1024, "#define USE_RGBAGEN\n");

		if (i & GENERICDEF_USE_LIGHTMAP)
		{
			Q_strcat(extradefines, 1024, "#define USE_LIGHTMAP\n");
			attribs |= ATTR_TEXCOORD1;
		}

		if (i & GENERICDEF_USE_GLOW_BUFFER)
			Q_strcat(extradefines, 1024, "#define USE_GLOW_BUFFER\n");

		if (r_hdr->integer && !glRefConfig.floatLightmap)
			Q_strcat(extradefines, 1024, "#define RGBM_LIGHTMAP\n");

		if (r_parallaxMapping->integer) // Parallax without normal maps...
			Q_strcat(extradefines, 1024, "#define USE_PARALLAXMAP_NONORMALS\n");

		if (r_parallaxMapping->integer && r_parallaxMapping->integer < 2) // Fast parallax mapping...
			Q_strcat(extradefines, 1024, "#define FAST_PARALLAX\n");

		if (!GLSL_BeginLoadGPUShader(&tr.genericShader[i], "generic", attribs, qtrue, extradefines, qtrue, fallbackShader_generic_vp, fallbackShader_generic_fp))
		{
			ri->Error(ERR_FATAL, "Could not load generic shader!");
		}
	}


	attribs = ATTR_POSITION | ATTR_TEXCOORD0;

	if (!GLSL_BeginLoadGPUShader(&tr.textureColorShader, "texturecolor", attribs, qtrue, NULL, qfalse, fallbackShader_texturecolor_vp, fallbackShader_texturecolor_fp))
	{
		ri->Error(ERR_FATAL, "Could not load texturecolor shader!");
	}

	for (i = 0; i < FOGDEF_COUNT; i++)
	{
		if (!GLSL_IsValidPermutationForFog (i))
		{
			continue;
		}

		attribs = ATTR_POSITION | ATTR_POSITION2 | ATTR_NORMAL | ATTR_NORMAL2 | ATTR_TEXCOORD0;
		extradefines[0] = '\0';

		if (i & FOGDEF_USE_DEFORM_VERTEXES)
			Q_strcat(extradefines, 1024, "#define USE_DEFORM_VERTEXES\n");

		if (i & FOGDEF_USE_VERTEX_ANIMATION)
			Q_strcat(extradefines, 1024, "#define USE_VERTEX_ANIMATION\n");

		if (i & FOGDEF_USE_SKELETAL_ANIMATION)
			Q_strcat(extradefines, 1024, "#define USE_SKELETAL_ANIMATION\n");

		if (!GLSL_BeginLoadGPUShader(&tr.fogShader[i], "fogpass", attribs, qtrue, extradefines, qtrue, fallbackShader_fogpass_vp, fallbackShader_fogpass_fp))
		{
			ri->Error(ERR_FATAL, "Could not load fogpass shader!");
		}
	}


	for (i = 0; i < DLIGHTDEF_COUNT; i++)
	{
		attribs = ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD0;
		extradefines[0] = '\0';

		if (i & DLIGHTDEF_USE_DEFORM_VERTEXES)
		{
			Q_strcat(extradefines, 1024, "#define USE_DEFORM_VERTEXES\n");
		}

		if (!GLSL_BeginLoadGPUShader(&tr.dlightShader[i], "dlight", attribs, qtrue, extradefines, qfalse, fallbackShader_dlight_vp, fallbackShader_dlight_fp))
		{
			ri->Error(ERR_FATAL, "Could not load dlight shader!");
		}
	}


	for (i = 0; i < LIGHTDEF_COUNT; i++)
	{
		int lightType = i & LIGHTDEF_LIGHTTYPE_MASK;
		qboolean fastLight = (qboolean)!(r_normalMapping->integer || r_specularMapping->integer);

		// skip impossible combos
		if (!GLSL_IsValidPermutationForLight (lightType, i))
		{
			continue;
		}

		attribs = ATTR_POSITION | ATTR_TEXCOORD0 | ATTR_COLOR | ATTR_NORMAL;

		extradefines[0] = '\0';

		if (r_deluxeSpecular->value > 0.000001f)
			Q_strcat(extradefines, 1024, va("#define r_deluxeSpecular %f\n", r_deluxeSpecular->value));

		if (r_specularIsMetallic->value)
			Q_strcat(extradefines, 1024, "#define SPECULAR_IS_METALLIC\n");

		if (r_dlightMode->integer >= 2)
			Q_strcat(extradefines, 1024, "#define USE_SHADOWMAP\n");

		//if (1)
		//	Q_strcat(extradefines, 1024, "#define SWIZZLE_NORMALMAP\n");

		if (r_hdr->integer && !glRefConfig.floatLightmap)
			Q_strcat(extradefines, 1024, "#define RGBM_LIGHTMAP\n");

		{// Testing
			//Q_strcat(extradefines, 1024, "#define USE_MODELMATRIX\n");
			//attribs |= ATTR_POSITION2 | ATTR_NORMAL2;

			//if (!lightType) lightType = LIGHTDEF_USE_LIGHT_VERTEX;

			//Q_strcat(extradefines, 1024, "#define USE_VERT_TANGENT_SPACE\n");
			//attribs |= ATTR_TANGENT;

			//Q_strcat(extradefines, 1024, "#define USE_TCGEN\n");
			//Q_strcat(extradefines, 1024, "#define USE_TCMOD\n");
		}
		
		//if (!lightType /*|| lightType == LIGHTDEF_USE_LIGHT_VECTOR*/)
		//	lightType = LIGHTDEF_USE_LIGHT_VERTEX;

		if (lightType)
		{
			Q_strcat(extradefines, 1024, "#define USE_LIGHT\n");

			if (fastLight)
				Q_strcat(extradefines, 1024, "#define USE_FAST_LIGHT\n");

			switch (lightType)
			{
				case LIGHTDEF_USE_LIGHTMAP:
					Q_strcat(extradefines, 1024, "#define USE_LIGHTMAP\n");
					if (r_deluxeMapping->integer && !fastLight)
						Q_strcat(extradefines, 1024, "#define USE_DELUXEMAP\n");
					attribs |= ATTR_TEXCOORD1 | ATTR_LIGHTDIRECTION;
					break;
				case LIGHTDEF_USE_LIGHT_VECTOR:
					Q_strcat(extradefines, 1024, "#define USE_LIGHT_VECTOR\n");
					break;
				case LIGHTDEF_USE_LIGHT_VERTEX:
					Q_strcat(extradefines, 1024, "#define USE_LIGHT_VERTEX\n");
					attribs |= ATTR_LIGHTDIRECTION;
					break;
				default:
					break;
			}

			if (r_normalMapping->integer)
			{
				Q_strcat(extradefines, 1024, "#define USE_NORMALMAP\n");

				if (r_normalMapping->integer == 2)
					Q_strcat(extradefines, 1024, "#define USE_OREN_NAYAR\n");

				if (r_normalMapping->integer == 3)
					Q_strcat(extradefines, 1024, "#define USE_TRIACE_OREN_NAYAR\n");

#ifdef USE_VERT_TANGENT_SPACE
				Q_strcat(extradefines, 1024, "#define USE_VERT_TANGENT_SPACE\n");
				attribs |= ATTR_TANGENT;
#endif

				//if ((i & LIGHTDEF_USE_PARALLAXMAP) && r_parallaxMapping->integer)
					Q_strcat(extradefines, 1024, "#define USE_PARALLAXMAP\n");
				//else if (r_parallaxMapping->integer) // Parallax without normal maps...
				//	Q_strcat(extradefines, 1024, "#define USE_PARALLAXMAP_NONORMALS\n");

				if (r_parallaxMapping->integer && r_parallaxMapping->integer < 2) // Fast parallax mapping...
					Q_strcat(extradefines, 1024, "#define FAST_PARALLAX\n");
			}
			else if (r_parallaxMapping->integer) // Parallax without normal maps...
			{
				Q_strcat(extradefines, 1024, "#define USE_NORMALMAP\n");

				//Q_strcat(extradefines, 1024, "#define USE_PARALLAXMAP_NONORMALS\n");
				Q_strcat(extradefines, 1024, "#define USE_PARALLAXMAP\n");
#ifdef USE_VERT_TANGENT_SPACE
				Q_strcat(extradefines, 1024, "#define USE_VERT_TANGENT_SPACE\n");
				attribs |= ATTR_TANGENT;
#endif

				if (r_parallaxMapping->integer && r_parallaxMapping->integer < 2) // Fast parallax mapping...
					Q_strcat(extradefines, 1024, "#define FAST_PARALLAX\n");
			}

			if (r_specularMapping->integer)
			{
				Q_strcat(extradefines, 1024, "#define USE_SPECULARMAP\n");
			}

			if (r_cubeMapping->integer)
				Q_strcat(extradefines, 1024, "#define USE_CUBEMAP\n");
		}
		/*else if (r_normalMapping->integer)
		{
			if (r_parallaxMapping->integer)
				Q_strcat(extradefines, 1024, "#define USE_PARALLAXMAP\n");

#ifdef USE_VERT_TANGENT_SPACE
			Q_strcat(extradefines, 1024, "#define USE_VERT_TANGENT_SPACE\n");
			attribs |= ATTR_TANGENT;
#endif

			Q_strcat(extradefines, 1024, "#define USE_NORMALMAP\n");
		}*/

		if (i & LIGHTDEF_USE_SHADOWMAP)
		{
			Q_strcat(extradefines, 1024, "#define USE_SHADOWMAP\n");

			if (r_sunlightMode->integer == 1)
				Q_strcat(extradefines, 1024, "#define SHADOWMAP_MODULATE\n");
			else if (r_sunlightMode->integer == 2)
				Q_strcat(extradefines, 1024, "#define USE_PRIMARY_LIGHT\n");
		}

		if (i & LIGHTDEF_USE_TCGEN_AND_TCMOD)
		{
			Q_strcat(extradefines, 1024, "#define USE_TCGEN\n");
			Q_strcat(extradefines, 1024, "#define USE_TCMOD\n");
		}

		if (i & LIGHTDEF_ENTITY)
		{
			if (i & LIGHTDEF_USE_VERTEX_ANIMATION)
			{
				Q_strcat(extradefines, 1024, "#define USE_VERTEX_ANIMATION\n");
			}
			else if (i & LIGHTDEF_USE_SKELETAL_ANIMATION)
			{
				Q_strcat(extradefines, 1024, "#define USE_SKELETAL_ANIMATION\n");
				attribs |= ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS;
			}

			Q_strcat(extradefines, 1024, "#define USE_MODELMATRIX\n");
			attribs |= ATTR_POSITION2 | ATTR_NORMAL2;

#ifdef USE_VERT_TANGENT_SPACE
			if (r_normalMapping->integer)
			{
				attribs |= ATTR_TANGENT2;
			}
#endif
		}

		if (i & LIGHTDEF_USE_GLOW_BUFFER)
			Q_strcat(extradefines, 1024, "#define USE_GLOW_BUFFER\n");

		if (!GLSL_BeginLoadGPUShader(&tr.lightallShader[i], "lightall", attribs, qtrue, extradefines, qtrue, fallbackShader_lightall_vp, fallbackShader_lightall_fp))
		{
			ri->Error(ERR_FATAL, "Could not load lightall shader!");
		}
	}
	
	attribs = ATTR_POSITION | ATTR_POSITION2 | ATTR_NORMAL | ATTR_NORMAL2 | ATTR_TEXCOORD0;

	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.shadowmapShader, "shadowfill", attribs, qtrue, extradefines, qtrue, fallbackShader_shadowfill_vp, fallbackShader_shadowfill_fp))
	{
		ri->Error(ERR_FATAL, "Could not load shadowfill shader!");
	}

	attribs = ATTR_POSITION | ATTR_NORMAL;
	extradefines[0] = '\0';

	Q_strcat(extradefines, 1024, "#define USE_PCF\n#define USE_DISCARD\n");

	if (!GLSL_BeginLoadGPUShader(&tr.pshadowShader, "pshadow", attribs, qtrue, extradefines, qtrue, fallbackShader_pshadow_vp, fallbackShader_pshadow_fp))
	{
		ri->Error(ERR_FATAL, "Could not load pshadow shader!");
	}


	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.down4xShader, "down4x", attribs, qtrue, extradefines, qtrue, fallbackShader_down4x_vp, fallbackShader_down4x_fp))
	{
		ri->Error(ERR_FATAL, "Could not load down4x shader!");
	}


	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.bokehShader, "bokeh", attribs, qtrue, extradefines, qtrue, fallbackShader_bokeh_vp, fallbackShader_bokeh_fp))
	{
		ri->Error(ERR_FATAL, "Could not load bokeh shader!");
	}


	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.tonemapShader, "tonemap", attribs, qtrue, extradefines, qtrue, fallbackShader_tonemap_vp, fallbackShader_tonemap_fp))
	{
		ri->Error(ERR_FATAL, "Could not load tonemap shader!");
	}


	for (i = 0; i < 2; i++)
	{
		attribs = ATTR_POSITION | ATTR_TEXCOORD0;
		extradefines[0] = '\0';

		if (!i)
			Q_strcat(extradefines, 1024, "#define FIRST_PASS\n");

		if (!GLSL_BeginLoadGPUShader(&tr.calclevels4xShader[i], "calclevels4x", attribs, qtrue, extradefines, qtrue, fallbackShader_calclevels4x_vp, fallbackShader_calclevels4x_fp))
		{
			ri->Error(ERR_FATAL, "Could not load calclevels4x shader!");
		}		
	}


	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (r_shadowFilter->integer >= 1)
		Q_strcat(extradefines, 1024, "#define USE_SHADOW_FILTER\n");

	if (r_shadowFilter->integer >= 2)
		Q_strcat(extradefines, 1024, "#define USE_SHADOW_FILTER2\n");

	Q_strcat(extradefines, 1024, "#define USE_SHADOW_CASCADE\n");

	Q_strcat(extradefines, 1024, va("#define r_shadowMapSize %d\n", r_shadowMapSize->integer));
	Q_strcat(extradefines, 1024, va("#define r_shadowCascadeZFar %f\n", r_shadowCascadeZFar->value));


	if (!GLSL_BeginLoadGPUShader(&tr.shadowmaskShader, "shadowmask", attribs, qtrue, extradefines, qtrue, fallbackShader_shadowmask_vp, fallbackShader_shadowmask_fp))
	{
		ri->Error(ERR_FATAL, "Could not load shadowmask shader!");
	}


	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.ssaoShader, "ssao", attribs, qtrue, extradefines, qtrue, fallbackShader_ssao_vp, fallbackShader_ssao_fp))
	{
		ri->Error(ERR_FATAL, "Could not load ssao shader!");
	}


	for (i = 0; i < 2; i++)
	{
		attribs = ATTR_POSITION | ATTR_TEXCOORD0;
		extradefines[0] = '\0';

		if (i & 1)
			Q_strcat(extradefines, 1024, "#define USE_VERTICAL_BLUR\n");
		else
			Q_strcat(extradefines, 1024, "#define USE_HORIZONTAL_BLUR\n");


		if (!GLSL_BeginLoadGPUShader(&tr.depthBlurShader[i], "depthBlur", attribs, qtrue, extradefines, qtrue, fallbackShader_depthblur_vp, fallbackShader_depthblur_fp))
		{
			ri->Error(ERR_FATAL, "Could not load depthBlur shader!");
		}
	}

#if 0
	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_InitGPUShader(&tr.testcubeShader, "testcube", attribs, qtrue, extradefines, qtrue, NULL, NULL))
	{
		ri->Error(ERR_FATAL, "Could not load testcube shader!");
	}

	GLSL_InitUniforms(&tr.testcubeShader);

	qglUseProgram(tr.testcubeShader.program);
	GLSL_SetUniformInt(&tr.testcubeShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	qglUseProgram(0);

	GLSL_FinishGPUShader(&tr.testcubeShader);

	numEtcShaders++;
#endif

	attribs = 0;
	extradefines[0] = '\0';
	Q_strcat (extradefines, sizeof (extradefines), "#define BLUR_X");

	if (!GLSL_BeginLoadGPUShader(&tr.gaussianBlurShader[0], "gaussian_blur", attribs, qtrue, extradefines, qtrue, fallbackShader_gaussian_blur_vp, fallbackShader_gaussian_blur_fp))
	{
		ri->Error(ERR_FATAL, "Could not load gaussian_blur (X-direction) shader!");
	}

	attribs = 0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.gaussianBlurShader[1], "gaussian_blur", attribs, qtrue, extradefines, qtrue, fallbackShader_gaussian_blur_vp, fallbackShader_gaussian_blur_fp))
	{
		ri->Error(ERR_FATAL, "Could not load gaussian_blur (Y-direction) shader!");
	}



	//
	// UQ1: Added...
	//

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.darkexpandShader, "darkexpand", attribs, qtrue, extradefines, qtrue, fallbackShader_darkexpand_vp, fallbackShader_darkexpand_fp))
	{
		ri->Error(ERR_FATAL, "Could not load darkexpand shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.multipostShader, "multipost", attribs, qtrue, extradefines, qtrue, fallbackShader_multipost_vp, fallbackShader_multipost_fp))
	{
		ri->Error(ERR_FATAL, "Could not load multipost shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.volumelightShader, "volumelight", attribs, qtrue, extradefines, qtrue, fallbackShader_volumelight_vp, fallbackShader_volumelight_fp))
	{
		ri->Error(ERR_FATAL, "Could not load volumelight shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.bloomDarkenShader, "bloom_darken", attribs, qtrue, extradefines, qtrue, fallbackShader_bloom_darken_vp, fallbackShader_bloom_darken_fp))
	{
		ri->Error(ERR_FATAL, "Could not load bloom_darken shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.bloomBlurShader, "bloom_blur", attribs, qtrue, extradefines, qtrue, fallbackShader_bloom_blur_vp, fallbackShader_bloom_blur_fp))
	{
		ri->Error(ERR_FATAL, "Could not load bloom_blur shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.bloomCombineShader, "bloom_combine", attribs, qtrue, extradefines, qtrue, fallbackShader_bloom_combine_vp, fallbackShader_bloom_combine_fp))
	{
		ri->Error(ERR_FATAL, "Could not load bloom_combine shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.lensflareShader, "lensflare", attribs, qtrue, extradefines, qtrue, fallbackShader_lensflare_vp, fallbackShader_lensflare_fp))
	{
		ri->Error(ERR_FATAL, "Could not load lensflare shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.anamorphicDarkenShader, "anamorphic_darken", attribs, qtrue, extradefines, qtrue, fallbackShader_anamorphic_darken_vp, fallbackShader_anamorphic_darken_fp))
	{
		ri->Error(ERR_FATAL, "Could not load anamorphic_darken shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.anamorphicBlurShader, "anamorphic_blur", attribs, qtrue, extradefines, qtrue, fallbackShader_anamorphic_blur_vp, fallbackShader_anamorphic_blur_fp))
	{
		ri->Error(ERR_FATAL, "Could not load anamorphic_blur shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.anamorphicCombineShader, "anamorphic_combine", attribs, qtrue, extradefines, qtrue, fallbackShader_anamorphic_combine_vp, fallbackShader_anamorphic_combine_fp))
	{
		ri->Error(ERR_FATAL, "Could not load anamorphic_combine shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.hdrShader, "hdr", attribs, qtrue, extradefines, qtrue, fallbackShader_truehdr_vp, fallbackShader_truehdr_fp))
	{
		ri->Error(ERR_FATAL, "Could not load hdr shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.ssgiShader, "ssgi", attribs, qtrue, extradefines, qtrue, fallbackShader_ssgi_vp, fallbackShader_ssgi_fp))
	{
		ri->Error(ERR_FATAL, "Could not load ssgi shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.testshaderShader, "testshader", attribs, qtrue, extradefines, qtrue, fallbackShader_testshader_vp, fallbackShader_testshader_fp))
	{
		ri->Error(ERR_FATAL, "Could not load testshader shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.dofShader, "depthOfField", attribs, qtrue, extradefines, qtrue, fallbackShader_depthOfField_vp, fallbackShader_depthOfField_fp))
	{
		ri->Error(ERR_FATAL, "Could not load depthOfField shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.vibrancyShader, "vibrancy", attribs, qtrue, extradefines, qtrue, fallbackShader_vibrancy_vp, fallbackShader_vibrancy_fp))
	{
		ri->Error(ERR_FATAL, "Could not load vibrancy shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.texturecleanShader, "textureclean", attribs, qtrue, extradefines, qtrue, fallbackShader_textureclean_vp, fallbackShader_textureclean_fp))
	{
		ri->Error(ERR_FATAL, "Could not load textureclean shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.esharpeningShader, "esharpening", attribs, qtrue, extradefines, qtrue, fallbackShader_esharpening_vp, fallbackShader_esharpening_fp))
	{
		ri->Error(ERR_FATAL, "Could not load esharpening shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.esharpening2Shader, "esharpening2", attribs, qtrue, extradefines, qtrue, fallbackShader_esharpening2_vp, fallbackShader_esharpening2_fp))
	{
		ri->Error(ERR_FATAL, "Could not load esharpening2 shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.fxaaShader, "fxaa", attribs, qtrue, extradefines, qtrue, fallbackShader_fxaa_vp, fallbackShader_fxaa_fp))
	{
		ri->Error(ERR_FATAL, "Could not load fxaa shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.generateNormalMapShader, "generateNormalMap", attribs, qtrue, extradefines, qtrue, fallbackShader_generateNormalMap_vp, fallbackShader_generateNormalMap_fp))
	{
		ri->Error(ERR_FATAL, "Could not load generateNormalMap shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.underwaterShader, "underwater", attribs, qtrue, extradefines, qtrue, fallbackShader_underwater_vp, fallbackShader_underwater_fp))
	{
		ri->Error(ERR_FATAL, "Could not load underwater shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.fakedepthShader, "fakedepth", attribs, qtrue, extradefines, qtrue, fallbackShader_fakeDepth_vp, fallbackShader_fakeDepth_fp))
	{
		ri->Error(ERR_FATAL, "Could not load fake depth shader!");
	}


	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.fakedepthSteepParallaxShader, "fakedepthSteepParallax", attribs, qtrue, extradefines, qtrue, fallbackShader_fakeDepthSteepParallax_vp, fallbackShader_fakeDepthSteepParallax_fp))
	{
		ri->Error(ERR_FATAL, "Could not load fake depth steep parallax shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.anaglyphShader, "anaglyph", attribs, qtrue, extradefines, qtrue, fallbackShader_anaglyph_vp, fallbackShader_anaglyph_fp))
	{
		ri->Error(ERR_FATAL, "Could not load anaglyph shader!");
	}

	
	attribs = ATTR_POSITION | ATTR_TEXCOORD0 | ATTR_NORMAL | ATTR_COLOR;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.uniqueskyShader, "uniquesky", attribs, qtrue, extradefines, qtrue, fallbackShader_uniquesky_vp, fallbackShader_uniquesky_fp))
	{
		ri->Error(ERR_FATAL, "Could not load uniquesky shader!");
	}
	


	attribs = ATTR_POSITION | ATTR_TEXCOORD0 | ATTR_COLOR | ATTR_NORMAL;
	extradefines[0] = '\0';

	if (r_deluxeSpecular->value > 0.000001f)
		Q_strcat(extradefines, 1024, va("#define r_deluxeSpecular %f\n", r_deluxeSpecular->value));

	if (r_specularIsMetallic->value)
		Q_strcat(extradefines, 1024, "#define SPECULAR_IS_METALLIC\n");

	if (r_dlightMode->integer >= 2)
		Q_strcat(extradefines, 1024, "#define USE_SHADOWMAP\n");

	if (1)
		Q_strcat(extradefines, 1024, "#define SWIZZLE_NORMALMAP\n");

	if (r_hdr->integer && !glRefConfig.floatLightmap)
		Q_strcat(extradefines, 1024, "#define RGBM_LIGHTMAP\n");

	Q_strcat(extradefines, 1024, "#define USE_LIGHT\n");

	Q_strcat(extradefines, 1024, "#define USE_LIGHTMAP\n");

	if (r_deluxeMapping->integer)
		Q_strcat(extradefines, 1024, "#define USE_DELUXEMAP\n");

	attribs |= ATTR_TEXCOORD1 | ATTR_LIGHTDIRECTION;

	if (r_normalMapping->integer)
	{
		Q_strcat(extradefines, 1024, "#define USE_NORMALMAP\n");

		if (r_normalMapping->integer == 2)
			Q_strcat(extradefines, 1024, "#define USE_OREN_NAYAR\n");

		if (r_normalMapping->integer == 3)
			Q_strcat(extradefines, 1024, "#define USE_TRIACE_OREN_NAYAR\n");

#ifdef USE_VERT_TANGENT_SPACE
		Q_strcat(extradefines, 1024, "#define USE_VERT_TANGENT_SPACE\n");
		attribs |= ATTR_TANGENT;
#endif

		Q_strcat(extradefines, 1024, "#define USE_PARALLAXMAP_NONORMALS\n");
	}

	if (r_specularMapping->integer)
	{
		Q_strcat(extradefines, 1024, "#define USE_SPECULARMAP\n");
	}

	if (r_cubeMapping->integer)
		Q_strcat(extradefines, 1024, "#define USE_CUBEMAP\n");

	Q_strcat(extradefines, 1024, "#define USE_SHADOWMAP\n");

	if (r_sunlightMode->integer == 1)
		Q_strcat(extradefines, 1024, "#define SHADOWMAP_MODULATE\n");
	else if (r_sunlightMode->integer == 2)
		Q_strcat(extradefines, 1024, "#define USE_PRIMARY_LIGHT\n");

	//if (i & LIGHTDEF_USE_TCGEN_AND_TCMOD)
	{
		Q_strcat(extradefines, 1024, "#define USE_TCGEN\n");
		Q_strcat(extradefines, 1024, "#define USE_TCMOD\n");
	}

	//if (i & LIGHTDEF_ENTITY)
	{
		//if (i & LIGHTDEF_USE_VERTEX_ANIMATION)
		{
			Q_strcat(extradefines, 1024, "#define USE_VERTEX_ANIMATION\n");
		}
		/*else if (i & LIGHTDEF_USE_SKELETAL_ANIMATION)
		{
			Q_strcat(extradefines, 1024, "#define USE_SKELETAL_ANIMATION\n");
			attribs |= ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS;
		}*/

		Q_strcat(extradefines, 1024, "#define USE_MODELMATRIX\n");
		attribs |= ATTR_POSITION2 | ATTR_NORMAL2;

#ifdef USE_VERT_TANGENT_SPACE
		if (r_normalMapping->integer)
		{
			attribs |= ATTR_TANGENT2;
		}
#endif
	}

	//if (i & LIGHTDEF_USE_GLOW_BUFFER)
		Q_strcat(extradefines, 1024, "#define USE_GLOW_BUFFER\n");

	if (!GLSL_BeginLoadGPUShader(&tr.waterShader, "uniquewater", attribs, qtrue, extradefines, qtrue, fallbackShader_uniquewater_vp, fallbackShader_uniquewater_fp))
	{
		ri->Error(ERR_FATAL, "Could not load water shader!");
	}

	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_BeginLoadGPUShader(&tr.ssao2Shader, "ssao2", attribs, qtrue, extradefines, qfalse, fallbackShader_ssao2_vp, fallbackShader_ssao2_fp))
	{
		ri->Error(ERR_FATAL, "Could not load ssao2 shader!");
	}

	//
	// UQ1: End Added...
	//

	return startTime;
}

void GLSL_EndLoadGPUShaders ( int startTime )
{
	int i;
	int numGenShaders = 0, numLightShaders = 0, numEtcShaders = 0;

	for (i = 0; i < GENERICDEF_COUNT; i++)
	{
		if (!GLSL_IsValidPermutationForGeneric (i))
		{
			continue;
		}

		if (!GLSL_EndLoadGPUShader(&tr.genericShader[i]))
		{
			ri->Error(ERR_FATAL, "Could not load fogpass shader!");
		}

		GLSL_InitUniforms(&tr.genericShader[i]);

		qglUseProgram(tr.genericShader[i].program);
		GLSL_SetUniformInt(&tr.genericShader[i], UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		GLSL_SetUniformInt(&tr.genericShader[i], UNIFORM_LIGHTMAP,   TB_LIGHTMAP);
		qglUseProgram(0);

#if defined(_DEBUG)
		GLSL_FinishGPUShader(&tr.genericShader[i]);
#endif

		numGenShaders++;
	}

	if (!GLSL_EndLoadGPUShader (&tr.textureColorShader))
	{
		ri->Error(ERR_FATAL, "Could not load texturecolor shader!");
	}
	
	GLSL_InitUniforms(&tr.textureColorShader);

	qglUseProgram(tr.textureColorShader.program);
	GLSL_SetUniformInt(&tr.textureColorShader, UNIFORM_TEXTUREMAP, TB_DIFFUSEMAP);
	qglUseProgram(0);

	GLSL_FinishGPUShader(&tr.textureColorShader);

	numEtcShaders++;

	for (i = 0; i < FOGDEF_COUNT; i++)
	{
		if (!GLSL_IsValidPermutationForFog (i))
		{
			continue;
		}

		if (!GLSL_EndLoadGPUShader(&tr.fogShader[i]))
		{
			ri->Error(ERR_FATAL, "Could not load fogpass shader!");
		}
		
		GLSL_InitUniforms(&tr.fogShader[i]);
#if defined(_DEBUG)
		GLSL_FinishGPUShader(&tr.fogShader[i]);
#endif
		
		numEtcShaders++;
	}


	for (i = 0; i < DLIGHTDEF_COUNT; i++)
	{
		if (!GLSL_EndLoadGPUShader(&tr.dlightShader[i]))
		{
			ri->Error(ERR_FATAL, "Could not load dlight shader!");
		}
		
		GLSL_InitUniforms(&tr.dlightShader[i]);
		
		qglUseProgram(tr.dlightShader[i].program);
		GLSL_SetUniformInt(&tr.dlightShader[i], UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		qglUseProgram(0);

#if defined(_DEBUG)
		GLSL_FinishGPUShader(&tr.dlightShader[i]);
#endif
		
		numEtcShaders++;
	}


	for (i = 0; i < LIGHTDEF_COUNT; i++)
	{
		int lightType = i & LIGHTDEF_LIGHTTYPE_MASK;
		qboolean fastLight = (qboolean)!(r_normalMapping->integer || r_specularMapping->integer);

		// skip impossible combos
		if (!GLSL_IsValidPermutationForLight (lightType, i))
		{
			continue;
		}

		if (!GLSL_EndLoadGPUShader(&tr.lightallShader[i]))
		{
			ri->Error(ERR_FATAL, "Could not load lightall shader!");
		}
		
		GLSL_InitUniforms(&tr.lightallShader[i]);

		qglUseProgram(tr.lightallShader[i].program);
		GLSL_SetUniformInt(&tr.lightallShader[i], UNIFORM_DIFFUSEMAP,  TB_DIFFUSEMAP);
		GLSL_SetUniformInt(&tr.lightallShader[i], UNIFORM_LIGHTMAP,    TB_LIGHTMAP);
		GLSL_SetUniformInt(&tr.lightallShader[i], UNIFORM_NORMALMAP,   TB_NORMALMAP);
		GLSL_SetUniformInt(&tr.lightallShader[i], UNIFORM_DELUXEMAP,   TB_DELUXEMAP);
		GLSL_SetUniformInt(&tr.lightallShader[i], UNIFORM_SPECULARMAP, TB_SPECULARMAP);
		GLSL_SetUniformInt(&tr.lightallShader[i], UNIFORM_SHADOWMAP,   TB_SHADOWMAP);
		GLSL_SetUniformInt(&tr.lightallShader[i], UNIFORM_CUBEMAP,     TB_CUBEMAP);
		GLSL_SetUniformInt(&tr.lightallShader[i], UNIFORM_SUBSURFACEMAP, TB_SUBSURFACEMAP);
		qglUseProgram(0);

#if defined(_DEBUG)
		GLSL_FinishGPUShader(&tr.lightallShader[i]);
#endif
		
		numLightShaders++;
	}

	
	if (!GLSL_EndLoadGPUShader(&tr.shadowmapShader))
	{
		ri->Error(ERR_FATAL, "Could not load shadowfill shader!");
	}
	
	GLSL_InitUniforms(&tr.shadowmapShader);
#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.shadowmapShader);
#endif

	numEtcShaders++;

	if (!GLSL_EndLoadGPUShader(&tr.pshadowShader))
	{
		ri->Error(ERR_FATAL, "Could not load pshadow shader!");
	}
	
	GLSL_InitUniforms(&tr.pshadowShader);

	qglUseProgram(tr.pshadowShader.program);
	GLSL_SetUniformInt(&tr.pshadowShader, UNIFORM_SHADOWMAP, TB_DIFFUSEMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.pshadowShader);
#endif
	
	numEtcShaders++;

	if (!GLSL_EndLoadGPUShader(&tr.down4xShader))
	{
		ri->Error(ERR_FATAL, "Could not load down4x shader!");
	}
	
	GLSL_InitUniforms(&tr.down4xShader);

	qglUseProgram(tr.down4xShader.program);
	GLSL_SetUniformInt(&tr.down4xShader, UNIFORM_TEXTUREMAP, TB_DIFFUSEMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.down4xShader);
#endif
	
	numEtcShaders++;

	if (!GLSL_EndLoadGPUShader(&tr.bokehShader))
	{
		ri->Error(ERR_FATAL, "Could not load bokeh shader!");
	}
	
	GLSL_InitUniforms(&tr.bokehShader);

	qglUseProgram(tr.bokehShader.program);
	GLSL_SetUniformInt(&tr.bokehShader, UNIFORM_TEXTUREMAP, TB_DIFFUSEMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.bokehShader);
#endif
	
	numEtcShaders++;

	if (!GLSL_EndLoadGPUShader(&tr.tonemapShader))
	{
		ri->Error(ERR_FATAL, "Could not load tonemap shader!");
	}
	
	GLSL_InitUniforms(&tr.tonemapShader);

	qglUseProgram(tr.tonemapShader.program);
	GLSL_SetUniformInt(&tr.tonemapShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.tonemapShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.tonemapShader);
#endif
	
	numEtcShaders++;

	for (i = 0; i < 2; i++)
	{
		if (!GLSL_EndLoadGPUShader(&tr.calclevels4xShader[i]))
		{
			ri->Error(ERR_FATAL, "Could not load calclevels4x shader!");
		}
		
		GLSL_InitUniforms(&tr.calclevels4xShader[i]);

		qglUseProgram(tr.calclevels4xShader[i].program);
		GLSL_SetUniformInt(&tr.calclevels4xShader[i], UNIFORM_TEXTUREMAP, TB_DIFFUSEMAP);
		qglUseProgram(0);

#if defined(_DEBUG)
		GLSL_FinishGPUShader(&tr.calclevels4xShader[i]);
#endif
		
		numEtcShaders++;		
	}

	if (!GLSL_EndLoadGPUShader(&tr.shadowmaskShader))
	{
		ri->Error(ERR_FATAL, "Could not load shadowmask shader!");
	}
	
	GLSL_InitUniforms(&tr.shadowmaskShader);

	qglUseProgram(tr.shadowmaskShader.program);
	GLSL_SetUniformInt(&tr.shadowmaskShader, UNIFORM_SCREENDEPTHMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.shadowmaskShader, UNIFORM_SHADOWMAP,  TB_SHADOWMAP);
	GLSL_SetUniformInt(&tr.shadowmaskShader, UNIFORM_SHADOWMAP2, TB_SHADOWMAP2);
	GLSL_SetUniformInt(&tr.shadowmaskShader, UNIFORM_SHADOWMAP3, TB_SHADOWMAP3);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.shadowmaskShader);
#endif
	
	numEtcShaders++;

	if (!GLSL_EndLoadGPUShader(&tr.ssaoShader))
	{
		ri->Error(ERR_FATAL, "Could not load ssao shader!");
	}

	GLSL_InitUniforms(&tr.ssaoShader);

	qglUseProgram(tr.ssaoShader.program);
	GLSL_SetUniformInt(&tr.ssaoShader, UNIFORM_SCREENDEPTHMAP, TB_COLORMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.ssaoShader);
#endif

	numEtcShaders++;

	for (i = 0; i < 2; i++)
	{
		if (!GLSL_EndLoadGPUShader(&tr.depthBlurShader[i]))
		{
			ri->Error(ERR_FATAL, "Could not load depthBlur shader!");
		}

		GLSL_InitUniforms(&tr.depthBlurShader[i]);

		qglUseProgram(tr.depthBlurShader[i].program);
		GLSL_SetUniformInt(&tr.depthBlurShader[i], UNIFORM_SCREENIMAGEMAP, TB_COLORMAP);
		GLSL_SetUniformInt(&tr.depthBlurShader[i], UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
		qglUseProgram(0);

#if defined(_DEBUG)
		GLSL_FinishGPUShader(&tr.depthBlurShader[i]);
#endif

		numEtcShaders++;
	}

	for (i = 0; i < 2; i++)
	{
		if (!GLSL_EndLoadGPUShader(&tr.gaussianBlurShader[i]))
		{
			ri->Error(ERR_FATAL, "Could not load gaussian blur shader!");
		}

		GLSL_InitUniforms(&tr.gaussianBlurShader[i]);

#if defined(_DEBUG)
		GLSL_FinishGPUShader(&tr.gaussianBlurShader[i]);
#endif

		numEtcShaders++;
	}

#if 0
	attribs = ATTR_POSITION | ATTR_TEXCOORD0;
	extradefines[0] = '\0';

	if (!GLSL_InitGPUShader(&tr.testcubeShader, "testcube", attribs, qtrue, extradefines, qtrue, NULL, NULL))
	{
		ri->Error(ERR_FATAL, "Could not load testcube shader!");
	}

	GLSL_InitUniforms(&tr.testcubeShader);

	qglUseProgram(tr.testcubeShader.program);
	GLSL_SetUniformInt(&tr.testcubeShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	qglUseProgram(0);

	GLSL_FinishGPUShader(&tr.testcubeShader);

	numEtcShaders++;
#endif



	//
	// UQ1: Added...
	//

	if (!GLSL_EndLoadGPUShader(&tr.darkexpandShader))
	{
		ri->Error(ERR_FATAL, "Could not load darkexpand shader!");
	}
	
	GLSL_InitUniforms(&tr.darkexpandShader);

	qglUseProgram(tr.darkexpandShader.program);

	GLSL_SetUniformInt(&tr.darkexpandShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.darkexpandShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);

	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.darkexpandShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.darkexpandShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.darkexpandShader);
#endif
	
	numEtcShaders++;



	if (!GLSL_EndLoadGPUShader(&tr.multipostShader))
	{
		ri->Error(ERR_FATAL, "Could not load multipost shader!");
	}
	
	GLSL_InitUniforms(&tr.multipostShader);
	qglUseProgram(tr.multipostShader.program);
	GLSL_SetUniformInt(&tr.multipostShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.multipostShader);
#endif
	
	numEtcShaders++;



	if (!GLSL_EndLoadGPUShader(&tr.volumelightShader))
	{
		ri->Error(ERR_FATAL, "Could not load volumelight shader!");
	}
	
	GLSL_InitUniforms(&tr.volumelightShader);
	qglUseProgram(tr.volumelightShader.program);
	GLSL_SetUniformInt(&tr.volumelightShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.volumelightShader);
#endif
	
	numEtcShaders++;



	if (!GLSL_EndLoadGPUShader(&tr.bloomDarkenShader))
	{
		ri->Error(ERR_FATAL, "Could not load bloom_darken shader!");
	}
	
	GLSL_InitUniforms(&tr.bloomDarkenShader);
	qglUseProgram(tr.bloomDarkenShader.program);
	GLSL_SetUniformInt(&tr.bloomDarkenShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.bloomDarkenShader);
#endif
	
	numEtcShaders++;


	if (!GLSL_EndLoadGPUShader(&tr.bloomBlurShader))
	{
		ri->Error(ERR_FATAL, "Could not load bloom_blur shader!");
	}
	
	GLSL_InitUniforms(&tr.bloomBlurShader);
	qglUseProgram(tr.bloomBlurShader.program);
	GLSL_SetUniformInt(&tr.bloomBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.bloomBlurShader);
#endif
	
	numEtcShaders++;

	if (!GLSL_EndLoadGPUShader(&tr.bloomCombineShader))
	{
		ri->Error(ERR_FATAL, "Could not load bloom_combine shader!");
	}
	
	GLSL_InitUniforms(&tr.bloomCombineShader);
	qglUseProgram(tr.bloomCombineShader.program);
	GLSL_SetUniformInt(&tr.bloomCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.bloomCombineShader, UNIFORM_NORMALMAP,  TB_NORMALMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.bloomCombineShader);
#endif
	
	numEtcShaders++;


	if (!GLSL_EndLoadGPUShader(&tr.lensflareShader))
	{
		ri->Error(ERR_FATAL, "Could not load lensflare shader!");
	}
	
	GLSL_InitUniforms(&tr.lensflareShader);
	qglUseProgram(tr.lensflareShader.program);
	GLSL_SetUniformInt(&tr.lensflareShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.lensflareShader);
#endif
	
	numEtcShaders++;



	if (!GLSL_EndLoadGPUShader(&tr.anamorphicDarkenShader))
	{
		ri->Error(ERR_FATAL, "Could not load anamorphic_darken shader!");
	}
	
	GLSL_InitUniforms(&tr.anamorphicDarkenShader);
	qglUseProgram(tr.anamorphicDarkenShader.program);
	GLSL_SetUniformInt(&tr.anamorphicDarkenShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.anamorphicDarkenShader);
#endif
	
	numEtcShaders++;


	if (!GLSL_EndLoadGPUShader(&tr.anamorphicBlurShader))
	{
		ri->Error(ERR_FATAL, "Could not load anamorphic_blur shader!");
	}
	
	GLSL_InitUniforms(&tr.anamorphicBlurShader);
	qglUseProgram(tr.anamorphicBlurShader.program);
	GLSL_SetUniformInt(&tr.anamorphicBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.anamorphicBlurShader);
#endif
	
	numEtcShaders++;

	if (!GLSL_EndLoadGPUShader(&tr.anamorphicCombineShader))
	{
		ri->Error(ERR_FATAL, "Could not load anamorphic_combine shader!");
	}
	
	GLSL_InitUniforms(&tr.anamorphicCombineShader);
	qglUseProgram(tr.anamorphicCombineShader.program);
	GLSL_SetUniformInt(&tr.anamorphicCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.anamorphicCombineShader, UNIFORM_NORMALMAP,  TB_NORMALMAP);
	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.anamorphicCombineShader);
#endif
	
	numEtcShaders++;


	if (!GLSL_EndLoadGPUShader(&tr.ssgiShader))
	{
		ri->Error(ERR_FATAL, "Could not load ssgi shader!");
	}
	
	GLSL_InitUniforms(&tr.ssgiShader);

	qglUseProgram(tr.ssgiShader.program);

	GLSL_SetUniformInt(&tr.ssgiShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);
	
	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.ssgiShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.ssgiShader, UNIFORM_VIEWINFO, viewInfo);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.ssgiShader);
#endif
	
	numEtcShaders++;


	if (!GLSL_EndLoadGPUShader(&tr.hdrShader))
	{
		ri->Error(ERR_FATAL, "Could not load hdr shader!");
	}
	
	GLSL_InitUniforms(&tr.hdrShader);

	qglUseProgram(tr.hdrShader.program);

	GLSL_SetUniformInt(&tr.hdrShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.hdrShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);
	
	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.hdrShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.hdrShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.hdrShader);
#endif
	
	numEtcShaders++;


	if (!GLSL_EndLoadGPUShader(&tr.dofShader))
	{
		ri->Error(ERR_FATAL, "Could not load depthOfField shader!");
	}
	
	GLSL_InitUniforms(&tr.dofShader);

	qglUseProgram(tr.dofShader.program);

	GLSL_SetUniformInt(&tr.dofShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.dofShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);
	
	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.dofShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.dofShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.dofShader);
#endif
	
	numEtcShaders++;

	if (!GLSL_EndLoadGPUShader(&tr.vibrancyShader))
	{
		ri->Error(ERR_FATAL, "Could not load vibrancy shader!");
	}
	
	GLSL_InitUniforms(&tr.vibrancyShader);

	qglUseProgram(tr.vibrancyShader.program);

	GLSL_SetUniformInt(&tr.vibrancyShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);
	
	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.vibrancyShader, UNIFORM_DIMENSIONS, screensize);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.vibrancyShader);
#endif
	
	numEtcShaders++;

	if (!GLSL_EndLoadGPUShader(&tr.testshaderShader))
	{
		ri->Error(ERR_FATAL, "Could not load testshader shader!");
	}
	
	GLSL_InitUniforms(&tr.testshaderShader);

	qglUseProgram(tr.testshaderShader.program);

	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);
	
	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.testshaderShader, UNIFORM_DIMENSIONS, screensize);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.testshaderShader);
#endif
	
	numEtcShaders++;

	//esharpeningShader
	if (!GLSL_EndLoadGPUShader(&tr.esharpeningShader))
	{
		ri->Error(ERR_FATAL, "Could not load esharpening shader!");
	}
	
	GLSL_InitUniforms(&tr.esharpeningShader);

	qglUseProgram(tr.esharpeningShader.program);

	GLSL_SetUniformInt(&tr.esharpeningShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.esharpeningShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);
	
	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.esharpeningShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.esharpeningShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.esharpeningShader);
#endif
	
	numEtcShaders++;


	//esharpeningShader
	if (!GLSL_EndLoadGPUShader(&tr.esharpening2Shader))
	{
		ri->Error(ERR_FATAL, "Could not load esharpening2 shader!");
	}
	
	GLSL_InitUniforms(&tr.esharpening2Shader);

	qglUseProgram(tr.esharpening2Shader.program);

	GLSL_SetUniformInt(&tr.esharpening2Shader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.esharpening2Shader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);
	
	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.esharpening2Shader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.esharpening2Shader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.esharpening2Shader);
#endif
	
	numEtcShaders++;


	//fxaaShader
	if (!GLSL_EndLoadGPUShader(&tr.fxaaShader))
	{
		ri->Error(ERR_FATAL, "Could not load fxaa shader!");
	}
	
	GLSL_InitUniforms(&tr.fxaaShader);

	qglUseProgram(tr.fxaaShader.program);

	GLSL_SetUniformInt(&tr.fxaaShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.fxaaShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);
	
	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.fxaaShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.fxaaShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.fxaaShader);
#endif
	
	numEtcShaders++;

	//generateNormalMap
	if (!GLSL_EndLoadGPUShader(&tr.generateNormalMapShader))
	{
		ri->Error(ERR_FATAL, "Could not load generateNormalMap shader!");
	}
	
	GLSL_InitUniforms(&tr.generateNormalMapShader);

	qglUseProgram(tr.generateNormalMapShader.program);

	GLSL_SetUniformInt(&tr.generateNormalMapShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.generateNormalMapShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);
	
	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.generateNormalMapShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.generateNormalMapShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.generateNormalMapShader);
#endif
	
	numEtcShaders++;

	//underwaterShader
	if (!GLSL_EndLoadGPUShader(&tr.underwaterShader))
	{
		ri->Error(ERR_FATAL, "Could not load underwater shader!");
	}
	
	GLSL_InitUniforms(&tr.underwaterShader);

	qglUseProgram(tr.underwaterShader.program);

	GLSL_SetUniformInt(&tr.underwaterShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	
	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.underwaterShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.underwaterShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.underwaterShader);
#endif
	
	numEtcShaders++;


	if (!GLSL_EndLoadGPUShader(&tr.texturecleanShader))
	{
		ri->Error(ERR_FATAL, "Could not load textureclean shader!");
	}
	
	GLSL_InitUniforms(&tr.texturecleanShader);

	qglUseProgram(tr.texturecleanShader.program);

	GLSL_SetUniformInt(&tr.texturecleanShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.texturecleanShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);
	
	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.texturecleanShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.texturecleanShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.texturecleanShader);
#endif
	
	numEtcShaders++;



	if (!GLSL_EndLoadGPUShader(&tr.fakedepthShader))
	{
		ri->Error(ERR_FATAL, "Could not load fake depth shader!");
	}
	
	GLSL_InitUniforms(&tr.fakedepthShader);

	qglUseProgram(tr.fakedepthShader.program);

	GLSL_SetUniformInt(&tr.fakedepthShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.fakedepthShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);

	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.fakedepthShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.fakedepthShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	{
		vec4_t local0;
		VectorSet4(local0, r_depthScale->value, r_depthParallax->value, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.fakedepthShader, UNIFORM_LOCAL0, local0);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.fakedepthShader);
#endif
	
	numEtcShaders++;

	
	if (!GLSL_EndLoadGPUShader(&tr.fakedepthSteepParallaxShader))
	{
		ri->Error(ERR_FATAL, "Could not load fake depth steep parallax shader!");
	}
	
	GLSL_InitUniforms(&tr.fakedepthSteepParallaxShader);

	qglUseProgram(tr.fakedepthSteepParallaxShader.program);

	GLSL_SetUniformInt(&tr.fakedepthSteepParallaxShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.fakedepthSteepParallaxShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);

	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.fakedepthSteepParallaxShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.fakedepthSteepParallaxShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	{
		vec4_t local0;
		VectorSet4(local0, r_depthScale->value, r_depthParallax->value, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.fakedepthSteepParallaxShader, UNIFORM_LOCAL0, local0);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.fakedepthSteepParallaxShader);
#endif
	
	numEtcShaders++;



	if (!GLSL_EndLoadGPUShader(&tr.anaglyphShader))
	{
		ri->Error(ERR_FATAL, "Could not load anaglyph shader!");
	}
	
	GLSL_InitUniforms(&tr.anaglyphShader);

	qglUseProgram(tr.anaglyphShader.program);

	GLSL_SetUniformInt(&tr.anaglyphShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.anaglyphShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);

	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.anaglyphShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.anaglyphShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	{
		vec4_t local0;
		VectorSet4(local0, r_trueAnaglyphSeparation->value, r_trueAnaglyphRed->value, r_trueAnaglyphGreen->value, r_trueAnaglyphBlue->value);
		GLSL_SetUniformVec4(&tr.anaglyphShader, UNIFORM_LOCAL0, local0);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.anaglyphShader);
#endif
	
	numEtcShaders++;


	
	if (!GLSL_EndLoadGPUShader(&tr.uniqueskyShader))
	{
		ri->Error(ERR_FATAL, "Could not load uniqueskyShader shader!");
	}
	
	GLSL_InitUniforms(&tr.uniqueskyShader);

	qglUseProgram(tr.uniqueskyShader.program);

	GLSL_SetUniformInt(&tr.uniqueskyShader, UNIFORM_TEXTUREMAP, TB_COLORMAP);
	GLSL_SetUniformInt(&tr.uniqueskyShader, UNIFORM_LEVELSMAP,  TB_LEVELSMAP);

	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.uniqueskyShader, UNIFORM_VIEWINFO, viewInfo);
	}

	qglUseProgram(0);

#if defined(_DEBUG)
	GLSL_FinishGPUShader(&tr.uniqueskyShader);
#endif
	
	numEtcShaders++;
	
	
		
		if (!GLSL_EndLoadGPUShader(&tr.waterShader))
		{
			ri->Error(ERR_FATAL, "Could not load water shader!");
		}
		
		GLSL_InitUniforms(&tr.waterShader);

		qglUseProgram(tr.waterShader.program);

		GLSL_SetUniformInt(&tr.waterShader, UNIFORM_DIFFUSEMAP,  TB_DIFFUSEMAP);
		GLSL_SetUniformInt(&tr.waterShader, UNIFORM_LIGHTMAP,    TB_LIGHTMAP);
		GLSL_SetUniformInt(&tr.waterShader, UNIFORM_NORMALMAP,   TB_NORMALMAP);
		GLSL_SetUniformInt(&tr.waterShader, UNIFORM_DELUXEMAP,   TB_DELUXEMAP);
		GLSL_SetUniformInt(&tr.waterShader, UNIFORM_SPECULARMAP, TB_SPECULARMAP);
		GLSL_SetUniformInt(&tr.waterShader, UNIFORM_SHADOWMAP,   TB_SHADOWMAP);
		GLSL_SetUniformInt(&tr.waterShader, UNIFORM_CUBEMAP,     TB_CUBEMAP);
		GLSL_SetUniformInt(&tr.lightallShader[i], UNIFORM_SUBSURFACEMAP, TB_SUBSURFACEMAP);

		{
			vec4_t viewInfo;

			float zmax = backEnd.viewParms.zFar;
			float zmin = r_znear->value;

			VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
			//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

			GLSL_SetUniformVec4(&tr.waterShader, UNIFORM_VIEWINFO, viewInfo);
		}

		{
			vec2_t screensize;
			screensize[0] = glConfig.vidWidth;
			screensize[1] = glConfig.vidHeight;

			GLSL_SetUniformVec2(&tr.waterShader, UNIFORM_DIMENSIONS, screensize);

			//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
		}

		qglUseProgram(0);

#if defined(_DEBUG)
		GLSL_FinishGPUShader(&tr.waterShader);
#endif
		
		numEtcShaders++;


		if (!GLSL_EndLoadGPUShader(&tr.ssao2Shader))
		{
			ri->Error(ERR_FATAL, "Could not load ssao2 shader!");
		}

		GLSL_InitUniforms(&tr.ssao2Shader);

		qglUseProgram(tr.ssao2Shader.program);
		GLSL_SetUniformInt(&tr.ssao2Shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
		GLSL_SetUniformInt(&tr.ssao2Shader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		GLSL_SetUniformInt(&tr.ssao2Shader, UNIFORM_NORMALMAP, TB_NORMALMAP);
		qglUseProgram(0);

#if defined(_DEBUG)
		GLSL_FinishGPUShader(&tr.ssao2Shader);
#endif

		numEtcShaders++;

	//
	// UQ1: End Added...
	//



	ri->Printf(PRINT_ALL, "loaded %i GLSL shaders (%i gen %i light %i etc) in %5.2f seconds\n", 
		numGenShaders + numLightShaders + numEtcShaders, numGenShaders, numLightShaders, 
		numEtcShaders, (ri->Milliseconds() - startTime) / 1000.0);
}

void GLSL_ShutdownGPUShaders(void)
{
	int i;

	ri->Printf(PRINT_ALL, "------- GLSL_ShutdownGPUShaders -------\n");

	qglDisableVertexAttribArray(ATTR_INDEX_TEXCOORD0);
	qglDisableVertexAttribArray(ATTR_INDEX_TEXCOORD1);
	qglDisableVertexAttribArray(ATTR_INDEX_POSITION);
	qglDisableVertexAttribArray(ATTR_INDEX_POSITION2);
	qglDisableVertexAttribArray(ATTR_INDEX_NORMAL);
#ifdef USE_VERT_TANGENT_SPACE
	qglDisableVertexAttribArray(ATTR_INDEX_TANGENT);
#endif
	qglDisableVertexAttribArray(ATTR_INDEX_NORMAL2);
#ifdef USE_VERT_TANGENT_SPACE
	qglDisableVertexAttribArray(ATTR_INDEX_TANGENT2);
#endif
	qglDisableVertexAttribArray(ATTR_INDEX_COLOR);
	qglDisableVertexAttribArray(ATTR_INDEX_LIGHTDIRECTION);
	qglDisableVertexAttribArray(ATTR_INDEX_BONE_INDEXES);
	qglDisableVertexAttribArray(ATTR_INDEX_BONE_WEIGHTS);
	GLSL_BindNullProgram();

	for ( i = 0; i < GENERICDEF_COUNT; i++)
		GLSL_DeleteGPUShader(&tr.genericShader[i]);

	GLSL_DeleteGPUShader(&tr.textureColorShader);

	for ( i = 0; i < FOGDEF_COUNT; i++)
		GLSL_DeleteGPUShader(&tr.fogShader[i]);

	for ( i = 0; i < DLIGHTDEF_COUNT; i++)
		GLSL_DeleteGPUShader(&tr.dlightShader[i]);

	for ( i = 0; i < LIGHTDEF_COUNT; i++)
		GLSL_DeleteGPUShader(&tr.lightallShader[i]);

	GLSL_DeleteGPUShader(&tr.shadowmapShader);
	GLSL_DeleteGPUShader(&tr.pshadowShader);
	GLSL_DeleteGPUShader(&tr.down4xShader);
	GLSL_DeleteGPUShader(&tr.bokehShader);
	GLSL_DeleteGPUShader(&tr.tonemapShader);

	for ( i = 0; i < 2; i++)
		GLSL_DeleteGPUShader(&tr.calclevels4xShader[i]);

	GLSL_DeleteGPUShader(&tr.shadowmaskShader);
	GLSL_DeleteGPUShader(&tr.ssaoShader);

	for ( i = 0; i < 2; i++)
		GLSL_DeleteGPUShader(&tr.depthBlurShader[i]);



	// UQ1: Added...
	GLSL_DeleteGPUShader(&tr.darkexpandShader);
	GLSL_DeleteGPUShader(&tr.hdrShader);
	GLSL_DeleteGPUShader(&tr.esharpeningShader);
	GLSL_DeleteGPUShader(&tr.esharpening2Shader);
	GLSL_DeleteGPUShader(&tr.fakedepthShader);
	GLSL_DeleteGPUShader(&tr.fakedepthSteepParallaxShader);
	GLSL_DeleteGPUShader(&tr.anaglyphShader);
	GLSL_DeleteGPUShader(&tr.waterShader);
	GLSL_DeleteGPUShader(&tr.ssao2Shader);
	GLSL_DeleteGPUShader(&tr.bloomDarkenShader);
	GLSL_DeleteGPUShader(&tr.bloomBlurShader);
	GLSL_DeleteGPUShader(&tr.bloomCombineShader);
	GLSL_DeleteGPUShader(&tr.lensflareShader);
	GLSL_DeleteGPUShader(&tr.multipostShader);
	GLSL_DeleteGPUShader(&tr.anamorphicDarkenShader);
	GLSL_DeleteGPUShader(&tr.anamorphicBlurShader);
	GLSL_DeleteGPUShader(&tr.anamorphicCombineShader);

	GLSL_DeleteGPUShader(&tr.dofShader);
	GLSL_DeleteGPUShader(&tr.fxaaShader);
	GLSL_DeleteGPUShader(&tr.underwaterShader);
	GLSL_DeleteGPUShader(&tr.texturecleanShader);
	GLSL_DeleteGPUShader(&tr.ssgiShader);
	GLSL_DeleteGPUShader(&tr.volumelightShader);
	GLSL_DeleteGPUShader(&tr.vibrancyShader);
	GLSL_DeleteGPUShader(&tr.testshaderShader);
	GLSL_DeleteGPUShader(&tr.uniqueskyShader);
	GLSL_DeleteGPUShader(&tr.generateNormalMapShader);


	glState.currentProgram = 0;
	qglUseProgram(0);
}


void GLSL_BindProgram(shaderProgram_t * program)
{
	if(!program)
	{
		GLSL_BindNullProgram();
		return;
	}

	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get a call to va() every frame!
		GLimp_LogComment(va("--- GL_BindProgram( %s ) ---\n", program->name));
	}

	if(glState.currentProgram != program)
	{
		qglUseProgram(program->program);
		glState.currentProgram = program;
		backEnd.pc.c_glslShaderBinds++;
	}
}


void GLSL_BindNullProgram(void)
{
	if(r_logFile->integer)
	{
		GLimp_LogComment("--- GL_BindNullProgram ---\n");
	}

	if(glState.currentProgram)
	{
		qglUseProgram(0);
		glState.currentProgram = NULL;
	}
}


void GLSL_VertexAttribsState(uint32_t stateBits)
{
	uint32_t		diff;

	GLSL_VertexAttribPointers(stateBits);

	diff = stateBits ^ glState.vertexAttribsState;
	if(!diff)
	{
		return;
	}

	if(diff & ATTR_POSITION)
	{
		if(stateBits & ATTR_POSITION)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_POSITION )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_POSITION);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_POSITION )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_POSITION);
		}
	}

	if(diff & ATTR_TEXCOORD0)
	{
		if(stateBits & ATTR_TEXCOORD0)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_TEXCOORD )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_TEXCOORD0);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_TEXCOORD )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_TEXCOORD0);
		}
	}

	if(diff & ATTR_TEXCOORD1)
	{
		if(stateBits & ATTR_TEXCOORD1)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_LIGHTCOORD )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_TEXCOORD1);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_LIGHTCOORD )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_TEXCOORD1);
		}
	}

	if(diff & ATTR_NORMAL)
	{
		if(stateBits & ATTR_NORMAL)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_NORMAL )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_NORMAL);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_NORMAL )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_NORMAL);
		}
	}

#ifdef USE_VERT_TANGENT_SPACE
	if(diff & ATTR_TANGENT)
	{
		if(stateBits & ATTR_TANGENT)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_TANGENT )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_TANGENT);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_TANGENT )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_TANGENT);
		}
	}
#endif

	if(diff & ATTR_COLOR)
	{
		if(stateBits & ATTR_COLOR)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_COLOR )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_COLOR);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_COLOR )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_COLOR);
		}
	}

	if(diff & ATTR_LIGHTDIRECTION)
	{
		if(stateBits & ATTR_LIGHTDIRECTION)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_LIGHTDIRECTION )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_LIGHTDIRECTION);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_LIGHTDIRECTION )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_LIGHTDIRECTION);
		}
	}

	if(diff & ATTR_POSITION2)
	{
		if(stateBits & ATTR_POSITION2)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_POSITION2 )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_POSITION2);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_POSITION2 )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_POSITION2);
		}
	}

	if(diff & ATTR_NORMAL2)
	{
		if(stateBits & ATTR_NORMAL2)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_NORMAL2 )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_NORMAL2);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_NORMAL2 )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_NORMAL2);
		}
	}

#ifdef USE_VERT_TANGENT_SPACE
	if(diff & ATTR_TANGENT2)
	{
		if(stateBits & ATTR_TANGENT2)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_TANGENT2 )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_TANGENT2);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_TANGENT2 )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_TANGENT2);
		}
	}
#endif

	if(diff & ATTR_BONE_INDEXES)
	{
		if(stateBits & ATTR_BONE_INDEXES)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_BONE_INDEXES )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_BONE_INDEXES);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_BONE_INDEXES )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_BONE_INDEXES);
		}
	}

	if(diff & ATTR_BONE_WEIGHTS)
	{
		if(stateBits & ATTR_BONE_WEIGHTS)
		{
			GLimp_LogComment("qglEnableVertexAttribArray( ATTR_INDEX_BONE_WEIGHTS )\n");
			qglEnableVertexAttribArray(ATTR_INDEX_BONE_WEIGHTS);
		}
		else
		{
			GLimp_LogComment("qglDisableVertexAttribArray( ATTR_INDEX_BONE_WEIGHTS )\n");
			qglDisableVertexAttribArray(ATTR_INDEX_BONE_WEIGHTS);
		}
	}

	glState.vertexAttribsState = stateBits;
}

void GLSL_UpdateTexCoordVertexAttribPointers ( uint32_t attribBits )
{
	VBO_t *vbo = glState.currentVBO;

	if ( attribBits & ATTR_TEXCOORD0 )
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_TEXCOORD )\n");

		qglVertexAttribPointer(ATTR_INDEX_TEXCOORD0, 2, GL_FLOAT, 0, vbo->stride_st, BUFFER_OFFSET(vbo->ofs_st + sizeof (vec2_t) * glState.vertexAttribsTexCoordOffset[0]));
	}

	if ( attribBits & ATTR_TEXCOORD1 )
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_LIGHTCOORD )\n");

		qglVertexAttribPointer(ATTR_INDEX_TEXCOORD1, 2, GL_FLOAT, 0, vbo->stride_st, BUFFER_OFFSET(vbo->ofs_st + sizeof (vec2_t) * glState.vertexAttribsTexCoordOffset[1]));
	}
}

void GLSL_VertexAttribPointers(uint32_t attribBits)
{
	qboolean animated;
	int newFrame, oldFrame;
	VBO_t *vbo = glState.currentVBO;
	
	if(!vbo)
	{
		ri->Error(ERR_FATAL, "GL_VertexAttribPointers: no VBO bound");
		return;
	}

	// don't just call LogComment, or we will get a call to va() every frame!
	if (r_logFile->integer)
	{
		GLimp_LogComment("--- GL_VertexAttribPointers() ---\n");
	}

	// position/normal/tangent are always set in case of animation
	oldFrame = glState.vertexAttribsOldFrame;
	newFrame = glState.vertexAttribsNewFrame;
	animated = glState.vertexAnimation;
	
	if((attribBits & ATTR_POSITION) && (!(glState.vertexAttribPointersSet & ATTR_POSITION) || animated))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_POSITION )\n");

		qglVertexAttribPointer(ATTR_INDEX_POSITION, 3, GL_FLOAT, 0, vbo->stride_xyz, BUFFER_OFFSET(vbo->ofs_xyz + newFrame * vbo->size_xyz));
		glState.vertexAttribPointersSet |= ATTR_POSITION;
	}

	if((attribBits & ATTR_TEXCOORD0) && !(glState.vertexAttribPointersSet & ATTR_TEXCOORD0))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_TEXCOORD )\n");

		qglVertexAttribPointer(ATTR_INDEX_TEXCOORD0, 2, GL_FLOAT, 0, vbo->stride_st, BUFFER_OFFSET(vbo->ofs_st + sizeof (vec2_t) * glState.vertexAttribsTexCoordOffset[0]));
		glState.vertexAttribPointersSet |= ATTR_TEXCOORD0;
	}

	if((attribBits & ATTR_TEXCOORD1) && !(glState.vertexAttribPointersSet & ATTR_TEXCOORD1))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_LIGHTCOORD )\n");

		qglVertexAttribPointer(ATTR_INDEX_TEXCOORD1, 2, GL_FLOAT, 0, vbo->stride_st, BUFFER_OFFSET(vbo->ofs_st + sizeof (vec2_t) * glState.vertexAttribsTexCoordOffset[1]));
		glState.vertexAttribPointersSet |= ATTR_TEXCOORD1;
	}

	if((attribBits & ATTR_NORMAL) && (!(glState.vertexAttribPointersSet & ATTR_NORMAL) || animated))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_NORMAL )\n");

		qglVertexAttribPointer(ATTR_INDEX_NORMAL, 4, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, vbo->stride_normal, BUFFER_OFFSET(vbo->ofs_normal + newFrame * vbo->size_normal));
		glState.vertexAttribPointersSet |= ATTR_NORMAL;
	}

#ifdef USE_VERT_TANGENT_SPACE
	if((attribBits & ATTR_TANGENT) && (!(glState.vertexAttribPointersSet & ATTR_TANGENT) || animated))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_TANGENT )\n");

		qglVertexAttribPointer(ATTR_INDEX_TANGENT, 4, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, vbo->stride_tangent, BUFFER_OFFSET(vbo->ofs_tangent + newFrame * vbo->size_normal)); // FIXME
		glState.vertexAttribPointersSet |= ATTR_TANGENT;
	}
#endif

	if((attribBits & ATTR_COLOR) && !(glState.vertexAttribPointersSet & ATTR_COLOR))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_COLOR )\n");

		qglVertexAttribPointer(ATTR_INDEX_COLOR, 4, GL_FLOAT, 0, vbo->stride_vertexcolor, BUFFER_OFFSET(vbo->ofs_vertexcolor));
		glState.vertexAttribPointersSet |= ATTR_COLOR;
	}

	if((attribBits & ATTR_LIGHTDIRECTION) && !(glState.vertexAttribPointersSet & ATTR_LIGHTDIRECTION))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_LIGHTDIRECTION )\n");

		qglVertexAttribPointer(ATTR_INDEX_LIGHTDIRECTION, 4, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, vbo->stride_lightdir, BUFFER_OFFSET(vbo->ofs_lightdir));
		glState.vertexAttribPointersSet |= ATTR_LIGHTDIRECTION;
	}

	if((attribBits & ATTR_POSITION2) && (!(glState.vertexAttribPointersSet & ATTR_POSITION2) || animated))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_POSITION2 )\n");

		qglVertexAttribPointer(ATTR_INDEX_POSITION2, 3, GL_FLOAT, 0, vbo->stride_xyz, BUFFER_OFFSET(vbo->ofs_xyz + oldFrame * vbo->size_xyz));
		glState.vertexAttribPointersSet |= ATTR_POSITION2;
	}

	if((attribBits & ATTR_NORMAL2) && (!(glState.vertexAttribPointersSet & ATTR_NORMAL2) || animated))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_NORMAL2 )\n");

		qglVertexAttribPointer(ATTR_INDEX_NORMAL2, 4, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, vbo->stride_normal, BUFFER_OFFSET(vbo->ofs_normal + oldFrame * vbo->size_normal));
		glState.vertexAttribPointersSet |= ATTR_NORMAL2;
	}

#ifdef USE_VERT_TANGENT_SPACE
	if((attribBits & ATTR_TANGENT2) && (!(glState.vertexAttribPointersSet & ATTR_TANGENT2) || animated))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_TANGENT2 )\n");

		qglVertexAttribPointer(ATTR_INDEX_TANGENT2, 4, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, vbo->stride_tangent, BUFFER_OFFSET(vbo->ofs_tangent + oldFrame * vbo->size_normal)); // FIXME
		glState.vertexAttribPointersSet |= ATTR_TANGENT2;
	}
#endif

	if((attribBits & ATTR_BONE_INDEXES) && !(glState.vertexAttribPointersSet & ATTR_BONE_INDEXES))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_BONE_INDEXES )\n");

		qglVertexAttribPointer(ATTR_INDEX_BONE_INDEXES, 4, GL_FLOAT, 0, vbo->stride_boneindexes, BUFFER_OFFSET(vbo->ofs_boneindexes));
		glState.vertexAttribPointersSet |= ATTR_BONE_INDEXES;
	}

	if((attribBits & ATTR_BONE_WEIGHTS) && !(glState.vertexAttribPointersSet & ATTR_BONE_WEIGHTS))
	{
		GLimp_LogComment("qglVertexAttribPointer( ATTR_INDEX_BONE_WEIGHTS )\n");

		qglVertexAttribPointer(ATTR_INDEX_BONE_WEIGHTS, 4, GL_FLOAT, 0, vbo->stride_boneweights, BUFFER_OFFSET(vbo->ofs_boneweights));
		glState.vertexAttribPointersSet |= ATTR_BONE_WEIGHTS;
	}

}


shaderProgram_t *GLSL_GetGenericShaderProgram(int stage)
{
	shaderStage_t *pStage = tess.xstages[stage];
	int shaderAttribs = 0;

	if (tess.fogNum && pStage->adjustColorsForFog)
	{
		shaderAttribs |= GENERICDEF_USE_FOG;
	}

	if (pStage->bundle[1].image[0] && tess.shader->multitextureEnv)
	{
		shaderAttribs |= GENERICDEF_USE_LIGHTMAP;
	}

	switch (pStage->rgbGen)
	{
		case CGEN_LIGHTING_DIFFUSE:
			shaderAttribs |= GENERICDEF_USE_RGBAGEN;
			break;
		default:
			break;
	}

	switch (pStage->alphaGen)
	{
		case AGEN_LIGHTING_SPECULAR:
		case AGEN_PORTAL:
			shaderAttribs |= GENERICDEF_USE_RGBAGEN;
			break;
		default:
			break;
	}

	if (pStage->bundle[0].tcGen != TCGEN_TEXTURE)
	{
		shaderAttribs |= GENERICDEF_USE_TCGEN_AND_TCMOD;
	}

	if (tess.shader->numDeforms && !ShaderRequiresCPUDeforms(tess.shader))
	{
		shaderAttribs |= GENERICDEF_USE_DEFORM_VERTEXES;
	}

	if (glState.vertexAnimation)
	{
		shaderAttribs |= GENERICDEF_USE_VERTEX_ANIMATION;
	}

	if (glState.skeletalAnimation)
	{
		shaderAttribs |= GENERICDEF_USE_SKELETAL_ANIMATION;
	}

	if (pStage->bundle[0].numTexMods)
	{
		shaderAttribs |= GENERICDEF_USE_TCGEN_AND_TCMOD;
	}

	if (pStage->glow)
	{
		shaderAttribs |= GENERICDEF_USE_GLOW_BUFFER;
	}

	return &tr.genericShader[shaderAttribs];
}
