/*
===========================================================================
Copyright (C) 2011 Andrei Drexler, Richard Allen, James Canete

This file is part of Reaction source code.

Reaction source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Reaction source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Reaction source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "tr_local.h"

extern char currentMapName[128];

extern qboolean		FOG_LINEAR_ENABLE;
extern vec3_t		FOG_LINEAR_COLOR;
extern float		FOG_LINEAR_ALPHA;
extern qboolean		FOG_WORLD_ENABLE;
extern vec3_t		FOG_WORLD_COLOR;
extern vec3_t		FOG_WORLD_COLOR_SUN;
extern float		FOG_WORLD_CLOUDINESS;
extern float		FOG_WORLD_WIND;
extern float		FOG_WORLD_ALPHA;
extern float		FOG_WORLD_FADE_ALTITUDE;
extern qboolean		FOG_LAYER_ENABLE;
extern vec3_t		FOG_LAYER_COLOR;
extern float		FOG_LAYER_SUN_PENETRATION;
extern float		FOG_LAYER_ALPHA;
extern float		FOG_LAYER_CLOUDINESS;
extern float		FOG_LAYER_WIND;
extern float		FOG_LAYER_ALTITUDE_BOTTOM;
extern float		FOG_LAYER_ALTITUDE_TOP;
extern float		FOG_LAYER_ALTITUDE_FADE;
extern vec4_t		FOG_LAYER_BBOX;

extern float		SUN_PHONG_SCALE;
extern float		SHADOW_MINBRIGHT;
extern float		SHADOW_MAXBRIGHT;
extern qboolean		AO_ENABLED;
extern qboolean		AO_BLUR;
extern qboolean		AO_DIRECTIONAL;
extern float		AO_MINBRIGHT;
extern float		AO_MULTBRIGHT;
extern vec3_t		MAP_AMBIENT_CSB;
extern vec3_t		MAP_AMBIENT_CSB_NIGHT;

extern int			NUM_CLOSE_LIGHTS;
extern int			CLOSEST_LIGHTS[MAX_DEFERRED_LIGHTS];
extern vec2_t		CLOSEST_LIGHTS_SCREEN_POSITIONS[MAX_DEFERRED_LIGHTS];
extern vec3_t		CLOSEST_LIGHTS_POSITIONS[MAX_DEFERRED_LIGHTS];
extern float		CLOSEST_LIGHTS_DISTANCES[MAX_DEFERRED_LIGHTS];
extern float		CLOSEST_LIGHTS_HEIGHTSCALES[MAX_DEFERRED_LIGHTS];
extern vec3_t		CLOSEST_LIGHTS_COLORS[MAX_DEFERRED_LIGHTS];
extern float		CLOSEST_LIGHTS_CONEANGLES[MAX_DEFERRED_LIGHTS];
extern vec3_t		CLOSEST_LIGHTS_CONEDIRECTIONS[MAX_DEFERRED_LIGHTS];

#ifdef __PLAYER_BASED_CUBEMAPS__
extern int			currentPlayerCubemap;
extern vec4_t		currentPlayerCubemapVec;
extern float		currentPlayerCubemapDistance;
#endif //__PLAYER_BASED_CUBEMAPS__

void RB_ToneMap(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox, int autoExposure)
{
	vec4i_t srcBox, dstBox;
	vec4_t color;
	static int lastFrameCount = 0;

	if (autoExposure)
	{
		if (lastFrameCount == 0 || tr.frameCount < lastFrameCount || tr.frameCount - lastFrameCount > 5)
		{
			// determine average log luminance
			FBO_t *srcFbo, *dstFbo, *tmp;
			int size = 256;

			lastFrameCount = tr.frameCount;

			VectorSet4(dstBox, 0, 0, size, size);

			FBO_Blit(hdrFbo, hdrBox, NULL, tr.textureScratchFbo[0], dstBox, &tr.calclevels4xShader[0], NULL, 0);

			srcFbo = tr.textureScratchFbo[0];
			dstFbo = tr.textureScratchFbo[1];

			// downscale to 1x1 texture
			while (size > 1)
			{
				VectorSet4(srcBox, 0, 0, size, size);
				//size >>= 2;
				size >>= 1;
				VectorSet4(dstBox, 0, 0, size, size);

				if (size == 1)
					dstFbo = tr.targetLevelsFbo;

				//FBO_Blit(targetFbo, srcBox, NULL, tr.textureScratchFbo[nextScratch], dstBox, &tr.calclevels4xShader[1], NULL, 0);
				FBO_FastBlit(srcFbo, srcBox, dstFbo, dstBox, GL_COLOR_BUFFER_BIT, GL_LINEAR);

				tmp = srcFbo;
				srcFbo = dstFbo;
				dstFbo = tmp;
			}
		}

		// blend with old log luminance for gradual change
		VectorSet4(srcBox, 0, 0, 0, 0);

		color[0] = 
		color[1] =
		color[2] = 1.0f;
		color[3] = 0.03f;

		FBO_Blit(tr.targetLevelsFbo, srcBox, NULL, tr.calcLevelsFbo, NULL,  NULL, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
	}

	// tonemap
	color[0] =
	color[1] =
	color[2] = pow(2, r_cameraExposure->value); //exp2(r_cameraExposure->value);
	color[3] = 1.0f;

	if (autoExposure)
		GL_BindToTMU(tr.calcLevelsImage,  TB_LEVELSMAP);
	else
		GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.tonemapShader, color, 0);
}

/*
=============
RB_BokehBlur


Blurs a part of one framebuffer to another.

Framebuffers can be identical. 
=============
*/
void RB_BokehBlur(FBO_t *src, vec4i_t srcBox, FBO_t *dst, vec4i_t dstBox, float blur)
{
//	vec4i_t srcBox, dstBox;
	vec4_t color;
	
	blur *= 10.0f;

	if (blur < 0.004f)
		return;

	// bokeh blur
	if (blur > 0.0f)
	{
		vec4i_t quarterBox;

		quarterBox[0] = 0;
		quarterBox[1] = tr.quarterFbo[0]->height;
		quarterBox[2] = tr.quarterFbo[0]->width;
		quarterBox[3] = -tr.quarterFbo[0]->height;

		// create a quarter texture
		//FBO_Blit(NULL, NULL, NULL, tr.quarterFbo[0], NULL, NULL, NULL, 0);
		FBO_FastBlit(src, srcBox, tr.quarterFbo[0], quarterBox, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}

#ifndef HQ_BLUR
	if (blur > 1.0f)
	{
		// create a 1/16th texture
		//FBO_Blit(tr.quarterFbo[0], NULL, NULL, tr.textureScratchFbo[0], NULL, NULL, NULL, 0);
		FBO_FastBlit(tr.quarterFbo[0], NULL, tr.textureScratchFbo[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
#endif

	if (blur > 0.0f && blur <= 1.0f)
	{
		// Crossfade original with quarter texture
		VectorSet4(color, 1, 1, 1, blur);

		FBO_Blit(tr.quarterFbo[0], NULL, NULL, dst, dstBox, NULL, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
	}
#ifndef HQ_BLUR
	// ok blur, but can see some pixelization
	else if (blur > 1.0f && blur <= 2.0f)
	{
		// crossfade quarter texture with 1/16th texture
		FBO_Blit(tr.quarterFbo[0], NULL, NULL, dst, dstBox, NULL, NULL, 0);

		VectorSet4(color, 1, 1, 1, blur - 1.0f);

		FBO_Blit(tr.textureScratchFbo[0], NULL, NULL, dst, dstBox, NULL, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
	}
	else if (blur > 2.0f)
	{
		// blur 1/16th texture then replace
		int i;

		for (i = 0; i < 2; i++)
		{
			vec2_t blurTexScale;
			float subblur;

			subblur = ((blur - 2.0f) / 2.0f) / 3.0f * (float)(i + 1);

			blurTexScale[0] =
			blurTexScale[1] = subblur;

			color[0] =
			color[1] =
			color[2] = 0.5f;
			color[3] = 1.0f;

			if (i != 0)
				FBO_Blit(tr.textureScratchFbo[0], NULL, blurTexScale, tr.textureScratchFbo[1], NULL, &tr.bokehShader, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
			else
				FBO_Blit(tr.textureScratchFbo[0], NULL, blurTexScale, tr.textureScratchFbo[1], NULL, &tr.bokehShader, color, 0);
		}

		FBO_Blit(tr.textureScratchFbo[1], NULL, NULL, dst, dstBox, &tr.textureColorShader, NULL, 0);
	}
#else // higher quality blur, but slower
	else if (blur > 1.0f)
	{
		// blur quarter texture then replace
		int i;

		src = tr.quarterFbo[0];
		dst = tr.quarterFbo[1];

		VectorSet4(color, 0.5f, 0.5f, 0.5f, 1);

		for (i = 0; i < 2; i++)
		{
			vec2_t blurTexScale;
			float subblur;

			subblur = (blur - 1.0f) / 2.0f * (float)(i + 1);

			blurTexScale[0] =
			blurTexScale[1] = subblur;

			color[0] =
			color[1] =
			color[2] = 1.0f;
			if (i != 0)
				color[3] = 1.0f;
			else
				color[3] = 0.5f;

			FBO_Blit(tr.quarterFbo[0], NULL, blurTexScale, tr.quarterFbo[1], NULL, &tr.bokehShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
		}

		FBO_Blit(tr.quarterFbo[1], NULL, NULL, dst, dstBox, &tr.textureColorShader, NULL, 0);
	}
#endif
}


static void RB_RadialBlur(FBO_t *srcFbo, FBO_t *dstFbo, int passes, float stretch, float x, float y, float w, float h, float xcenter, float ycenter, float alpha)
{
	vec4i_t srcBox, dstBox;
	vec4_t color;
	const float inc = 1.f / passes;
	const float mul = powf(stretch, inc);
	float scale;

	{
		vec2_t texScale;

		texScale[0] = 
		texScale[1] = 1.0f;

		alpha *= inc;
		VectorSet4(color, alpha, alpha, alpha, 1.0f);

		VectorSet4(srcBox, 0, 0, srcFbo->width, srcFbo->height);
		VectorSet4(dstBox, x, y, w, h);
		FBO_Blit(srcFbo, srcBox, texScale, dstFbo, dstBox, &tr.textureColorShader, color, 0);

		--passes;
		scale = mul;
		while (passes > 0)
		{
			float iscale = 1.f / scale;
			float s0 = xcenter * (1.f - iscale);
			float t0 = (1.0f - ycenter) * (1.f - iscale);
			float s1 = iscale + s0;
			float t1 = iscale + t0;

			if (srcFbo)
			{
				srcBox[0] = s0 * srcFbo->width;
				srcBox[1] = t0 * srcFbo->height;
				srcBox[2] = (s1 - s0) * srcFbo->width;
				srcBox[3] = (t1 - t0) * srcFbo->height;
			}
			else
			{
				srcBox[0] = s0 * glConfig.vidWidth * r_superSampleMultiplier->value;
				srcBox[1] = t0 * glConfig.vidHeight * r_superSampleMultiplier->value;
				srcBox[2] = (s1 - s0) * glConfig.vidWidth * r_superSampleMultiplier->value;
				srcBox[3] = (t1 - t0) * glConfig.vidHeight * r_superSampleMultiplier->value;
			}
			
			FBO_Blit(srcFbo, srcBox, texScale, dstFbo, dstBox, &tr.textureColorShader, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );

			scale *= mul;
			--passes;
		}
	}
}


qboolean RB_UpdateSunFlareVis(void)
{
	GLuint sampleCount = 0;

	tr.sunFlareQueryIndex ^= 1;
	if (!tr.sunFlareQueryActive[tr.sunFlareQueryIndex])
		return qtrue;

	/* debug code */
	if (0)
	{
		int iter;
		for (iter=0 ; ; ++iter)
		{
			GLint available = 0;
			qglGetQueryObjectiv(tr.sunFlareQuery[tr.sunFlareQueryIndex], GL_QUERY_RESULT_AVAILABLE, &available);
			if (available)
				break;
		}

		ri->Printf(PRINT_DEVELOPER, "Waited %d iterations\n", iter);
	}
	
	qglGetQueryObjectuiv(tr.sunFlareQuery[tr.sunFlareQueryIndex], GL_QUERY_RESULT, &sampleCount);
	return (qboolean)(sampleCount > 0);
}

void RB_SunRays(FBO_t *srcFbo, vec4i_t srcBox, FBO_t *dstFbo, vec4i_t dstBox)
{
#if 0 // Warzone does this better and faster...
	vec4_t color;
	float dot;
	const float cutoff = 0.25f;
	qboolean colorize = qtrue;

//	float w, h, w2, h2;
	matrix_t mvp;
	vec4_t pos, hpos;

	dot = DotProduct(tr.sunDirection, backEnd.viewParms.ori.axis[0]);
	if (dot < cutoff)
		return;

	if (!RB_UpdateSunFlareVis())
		return;

	// From RB_DrawSun()
	{
		float dist;
		matrix_t trans, model, mvp;

		Matrix16Translation( backEnd.viewParms.ori.origin, trans );
		Matrix16Multiply( backEnd.viewParms.world.modelMatrix, trans, model );
		Matrix16Multiply(backEnd.viewParms.projectionMatrix, model, mvp);

		//dist = backEnd.viewParms.zFar / 1.75;		// div sqrt(3)
		dist = 4096.0;//backEnd.viewParms.zFar / 1.75;

		VectorScale( tr.sunDirection, dist, pos );
	}

	// project sun point
	//Matrix16Multiply(backEnd.viewParms.projectionMatrix, backEnd.viewParms.world.modelMatrix, mvp);
	Matrix16Transform(mvp, pos, hpos);

	// transform to UV coords
	hpos[3] = 0.5f / hpos[3];

	pos[0] = 0.5f + hpos[0] * hpos[3];
	pos[1] = 0.5f + hpos[1] * hpos[3];

	// initialize quarter buffers
	{
		float mul = 1.f;
		vec2_t texScale;
		vec4i_t rayBox, quarterBox;

		texScale[0] = 
		texScale[1] = 1.0f;

		VectorSet4(color, mul, mul, mul, 1);

		if (srcFbo)
		{
			rayBox[0] = srcBox[0] * tr.sunRaysFbo->width  / srcFbo->width;
			rayBox[1] = srcBox[1] * tr.sunRaysFbo->height / srcFbo->height;
			rayBox[2] = srcBox[2] * tr.sunRaysFbo->width  / srcFbo->width;
			rayBox[3] = srcBox[3] * tr.sunRaysFbo->height / srcFbo->height;
		}
		else
		{
			rayBox[0] = srcBox[0] * tr.sunRaysFbo->width  / (glConfig.vidWidth * r_superSampleMultiplier->value);
			rayBox[1] = srcBox[1] * tr.sunRaysFbo->height / (glConfig.vidHeight * r_superSampleMultiplier->value);
			rayBox[2] = srcBox[2] * tr.sunRaysFbo->width  / (glConfig.vidWidth * r_superSampleMultiplier->value);
			rayBox[3] = srcBox[3] * tr.sunRaysFbo->height / (glConfig.vidHeight * r_superSampleMultiplier->value);
		}

		quarterBox[0] = 0;
		quarterBox[1] = tr.quarterFbo[0]->height;
		quarterBox[2] = tr.quarterFbo[0]->width;
		quarterBox[3] = -tr.quarterFbo[0]->height;

		// first, downsample the framebuffer
		if (colorize)
		{
			FBO_FastBlit(srcFbo, srcBox, tr.quarterFbo[0], quarterBox, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			FBO_Blit(tr.sunRaysFbo, rayBox, NULL, tr.quarterFbo[0], quarterBox, NULL, color, GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO);
		}
		else
		{
			FBO_FastBlit(tr.sunRaysFbo, rayBox, tr.quarterFbo[0], quarterBox, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
	}

	// radial blur passes, ping-ponging between the two quarter-size buffers
	{
		const float stretch_add = 2.f/3.f;
		float stretch = 1.f + stretch_add;
		int i;
		for (i=0; i<2; ++i)
		{
			RB_RadialBlur(tr.quarterFbo[i&1], tr.quarterFbo[(~i) & 1], 5, stretch, 0.f, 0.f, tr.quarterFbo[0]->width, tr.quarterFbo[0]->height, pos[0], pos[1], 1.125f);
			stretch += stretch_add;
		}
	}
	
	// add result back on top of the main buffer
	{
		float mul = 1.f;
		vec2_t texScale;

		texScale[0] = 
		texScale[1] = 1.0f;

		VectorSet4(color, mul, mul, mul, 1);

		FBO_Blit(tr.quarterFbo[0], NULL, texScale, dstFbo, dstBox, &tr.textureColorShader, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
	}
#endif
}

static void RB_BlurAxis(FBO_t *srcFbo, FBO_t *dstFbo, float strength, qboolean horizontal)
{
	float dx, dy;
	float xmul, ymul;
	float weights[3] = {
		0.227027027f,
		0.316216216f,
		0.070270270f,
	};
	float offsets[3] = {
		0.f,
		1.3846153846f,
		3.2307692308f,
	};

	xmul = horizontal;
	ymul = 1.f - xmul;

	xmul *= strength;
	ymul *= strength;

	{
		vec4i_t srcBox, dstBox;
		vec4_t color;
		vec2_t texScale;

		texScale[0] = 
		texScale[1] = 1.0f;

		VectorSet4(color, weights[0], weights[0], weights[0], 1.0f);
		VectorSet4(srcBox, 0, 0, srcFbo->width, srcFbo->height);
		VectorSet4(dstBox, 0, 0, dstFbo->width, dstFbo->height);
		FBO_Blit(srcFbo, srcBox, texScale, dstFbo, dstBox, &tr.textureColorShader, color, 0 );

		VectorSet4(color, weights[1], weights[1], weights[1], 1.0f);
		dx = offsets[1] * xmul;
		dy = offsets[1] * ymul;
		VectorSet4(srcBox, dx, dy, srcFbo->width, srcFbo->height);
		FBO_Blit(srcFbo, srcBox, texScale, dstFbo, dstBox, &tr.textureColorShader, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
		VectorSet4(srcBox, -dx, -dy, srcFbo->width, srcFbo->height);
		FBO_Blit(srcFbo, srcBox, texScale, dstFbo, dstBox, &tr.textureColorShader, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );

		VectorSet4(color, weights[2], weights[2], weights[2], 1.0f);
		dx = offsets[2] * xmul;
		dy = offsets[2] * ymul;
		VectorSet4(srcBox, dx, dy, srcFbo->width, srcFbo->height);
		FBO_Blit(srcFbo, srcBox, texScale, dstFbo, dstBox, &tr.textureColorShader, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
		VectorSet4(srcBox, -dx, -dy, srcFbo->width, srcFbo->height);
		FBO_Blit(srcFbo, srcBox, texScale, dstFbo, dstBox, &tr.textureColorShader, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
	}
}

void RB_HBlur(FBO_t *srcFbo, FBO_t *dstFbo, float strength)
{
	RB_BlurAxis(srcFbo, dstFbo, strength, qtrue);
}

void RB_VBlur(FBO_t *srcFbo, FBO_t *dstFbo, float strength)
{
	RB_BlurAxis(srcFbo, dstFbo, strength, qfalse);
}

void RB_GaussianBlur(FBO_t *srcFbo, FBO_t *intermediateFbo, FBO_t *dstFbo, float spread)
{
	// Blur X
	vec2_t scale;
	VectorSet2 (scale, spread, spread);

	FBO_Blit (srcFbo, NULL, scale, intermediateFbo, NULL, &tr.gaussianBlurShader[0], NULL, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO);

	// Blur Y
	FBO_Blit (intermediateFbo, NULL, scale, dstFbo, NULL, &tr.gaussianBlurShader[1], NULL, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO);
}

void RB_BloomDownscale(image_t *sourceImage, FBO_t *destFBO)
{
	vec2_t invTexRes = { 1.0f / sourceImage->width, 1.0f / sourceImage->height };

	FBO_Bind(destFBO);
	GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO);

	qglViewport(0, 0, destFBO->width, destFBO->height);
	qglClearBufferfv(GL_COLOR, 0, colorBlack);

	GLSL_BindProgram(&tr.dglowDownsample);
	GLSL_SetUniformVec2(&tr.dglowDownsample, UNIFORM_INVTEXRES, invTexRes);
	GL_BindToTMU(sourceImage, 0);

	// Draw fullscreen triangle
	qglDrawArrays(GL_TRIANGLES, 0, 3);
}

void RB_BloomDownscale2(FBO_t *sourceFBO, FBO_t *destFBO)
{
	RB_BloomDownscale(sourceFBO->colorImage[0], destFBO);
}

void RB_BloomUpscale(FBO_t *sourceFBO, FBO_t *destFBO)
{
	image_t *sourceImage = sourceFBO->colorImage[0];
	vec2_t invTexRes = { 1.0f / sourceImage->width, 1.0f / sourceImage->height };

	FBO_Bind(destFBO);
	GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO);

	qglViewport(0, 0, destFBO->width, destFBO->height);
	qglClearBufferfv(GL_COLOR, 0, colorBlack);

	GLSL_BindProgram(&tr.dglowUpsample);
	GLSL_SetUniformVec2(&tr.dglowUpsample, UNIFORM_INVTEXRES, invTexRes);
	GL_BindToTMU(sourceImage, 0);

	// Draw fullscreen triangle
	qglDrawArrays(GL_TRIANGLES, 0, 3);
}


// ======================================================================================================================================
//
//
//                                                      UniqueOne's Shaders...
//
//
// ======================================================================================================================================


void RB_DarkExpand(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.darkexpandShader);

	GLSL_SetUniformInt(&tr.darkexpandShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.darkexpandShader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.darkexpandShader, color, 0);
}

void RB_Bloom(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t	color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	//
	// Darken to VBO...
	//
	
	FBO_BlitFromTexture(tr.glowFboScaled[0]->colorImage[0], NULL, NULL, tr.bloomRenderFBO[0], NULL, NULL, color, 0);

	//
	// Blur the new darken'ed VBO...
	//

	FBO_t *currentIN = tr.bloomRenderFBO[0];
	FBO_t *currentOUT = tr.bloomRenderFBO[1];

	for ( int i = 0; i < r_bloomPasses->integer; i++ ) 
	{
		//
		// Bloom X and Y axis... (to VBO 1)
		//

		GLSL_BindProgram(&tr.bloomBlurShader);

		GLSL_SetUniformInt(&tr.bloomBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		GL_BindToTMU(tr.bloomRenderFBOImage[0], TB_DIFFUSEMAP);

		{
			vec2_t screensize;
			screensize[0] = tr.bloomRenderFBOImage[0]->width;
			screensize[1] = tr.bloomRenderFBOImage[0]->height;

			GLSL_SetUniformVec2(&tr.bloomBlurShader, UNIFORM_DIMENSIONS, screensize);
		}

		FBO_Blit(currentIN, NULL, NULL, currentOUT, NULL, &tr.bloomBlurShader, color, 0);

		if (i+1 < r_bloomPasses->integer)
		{// Flip in/out FBOs...
			FBO_t *tempFBO = currentIN;
			currentIN = currentOUT;
			currentOUT = tempFBO;
		}
	}

	//
	// Combine the screen with the bloom'ed VBO...
	//
	
	
	GLSL_BindProgram(&tr.bloomCombineShader);
	
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.bloomCombineShader, UNIFORM_DIFFUSEMAP,   TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.bloomCombineShader, UNIFORM_NORMALMAP,   TB_NORMALMAP);

	GL_BindToTMU(currentOUT->colorImage[0], TB_NORMALMAP);

	{
		vec4_t local0;
		//VectorSet4(local0, r_bloomScale->value, 0.0, 0.0, 0.0);
		VectorSet4(local0, 0.5 * r_bloomScale->value, 0.0, 0.0, 0.0); // Account for already added glow...
		GLSL_SetUniformVec4(&tr.bloomCombineShader, UNIFORM_LOCAL0, local0);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.bloomCombineShader, color, 0);
}

void RB_CreateAnamorphicImage( void )
{
	vec4_t	color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	vec4i_t srcBox;
	srcBox[0] = 0;
	srcBox[1] = 0;
	srcBox[2] = tr.glowFboScaled[0]->width;
	srcBox[3] = tr.glowFboScaled[0]->height;

	GLSL_BindProgram(&tr.anamorphicBlurShader);

	GLSL_SetUniformInt(&tr.anamorphicBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(tr.glowFboScaled[0]->colorImage[0], TB_DIFFUSEMAP);

	{
		vec2_t screensize;
		screensize[0] = tr.anamorphicRenderFBOImage->width;
		screensize[1] = tr.anamorphicRenderFBOImage->height;

		GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t local0;
		VectorSet4(local0, 1.0, 0.0, 16.0, 0.0);
		GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
	}

	FBO_Blit(tr.glowFboScaled[0], srcBox, NULL, tr.anamorphicRenderFBO, NULL/*dstBox*/, &tr.anamorphicBlurShader, color, 0);
}

void RB_Anamorphic(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t	color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	//
	// Combine the screen with the bloom'ed VBO...
	//
	
	GLSL_BindProgram(&tr.anamorphicCombineShader);
	
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.anamorphicCombineShader, UNIFORM_DIFFUSEMAP,   TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.anamorphicCombineShader, UNIFORM_NORMALMAP,   TB_NORMALMAP);

	GL_BindToTMU(tr.anamorphicRenderFBOImage, TB_NORMALMAP);

	{
		vec4_t local1;
		VectorSet4(local1, r_anamorphicStrength->value, 0.0, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.anamorphicCombineShader, UNIFORM_LOCAL1, local1);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.anamorphicCombineShader, color, 0);
}

void RB_BloomRays(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	image_t *glowImage = tr.glowFboScaled[0]->colorImage[0];

	GLSL_BindProgram(&tr.bloomRaysShader);

	GLSL_SetUniformInt(&tr.bloomRaysShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	GLSL_SetUniformInt(&tr.bloomRaysShader, UNIFORM_GLOWMAP, TB_GLOWMAP);
	GL_BindToTMU(glowImage, TB_GLOWMAP);

	GLSL_SetUniformInt(&tr.bloomRaysShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImage2048, TB_LIGHTMAP);

	GLSL_SetUniformMatrix16(&tr.bloomRaysShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	{
		vec2_t dimensions;
		dimensions[0] = glowImage->width;
		dimensions[1] = glowImage->height;

		GLSL_SetUniformVec2(&tr.bloomRaysShader, UNIFORM_DIMENSIONS, dimensions);
	}

	{
		vec4_t viewInfo;
		float zmax = 2048.0;//3072.0;//backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(&tr.bloomRaysShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec4_t local1;
		VectorSet4(local1, r_bloomRaysDecay->value, r_bloomRaysWeight->value, r_bloomRaysDensity->value, r_bloomRaysStrength->value);
		GLSL_SetUniformVec4(&tr.bloomRaysShader, UNIFORM_LOCAL1, local1);
	}

	{
		vec4_t local2;
		VectorSet4(local2, DAY_NIGHT_CYCLE_ENABLED ? RB_NightScale() : 0.0, r_bloomRaysSamples->integer, r_testshaderValue1->value, r_testshaderValue2->value);
		GLSL_SetUniformVec4(&tr.bloomRaysShader, UNIFORM_LOCAL2, local2);
	}

	FBO_BlitFromTexture(glowImage, NULL, NULL, tr.volumetricFbo, NULL, &tr.bloomRaysShader, color, 0);

	// Combine render and bloomrays...
	GLSL_BindProgram(&tr.volumeLightCombineShader);

	GLSL_SetUniformMatrix16(&tr.volumeLightCombineShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	//GLSL_SetUniformMatrix16(&tr.volumeLightCombineShader, UNIFORM_MODELMATRIX, backEnd.ori.modelMatrix);

	GLSL_SetUniformInt(&tr.volumeLightCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.volumeLightCombineShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.volumetricFBOImage, TB_NORMALMAP);

	vec2_t screensize;
	screensize[0] = tr.volumetricFBOImage->width;
	screensize[1] = tr.volumetricFBOImage->height;
	GLSL_SetUniformVec2(&tr.volumeLightCombineShader, UNIFORM_DIMENSIONS, screensize);

	{
		vec4_t local1;
		VectorSet4(local1, backEnd.refdef.floatTime, 0.0 /*is volumelight shader*/, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.volumeLightCombineShader, UNIFORM_LOCAL1, local1);
	}

	FBO_Blit(hdrFbo, NULL, NULL, ldrFbo, NULL, &tr.volumeLightCombineShader, color, 0);
}

#if 0
void RB_LensFlare(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.lensflareShader);

	GLSL_SetUniformInt(&tr.lensflareShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	vec2_t screensize;
	screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
	screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;
	GLSL_SetUniformVec2(&tr.lensflareShader, UNIFORM_DIMENSIONS, screensize);

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.lensflareShader, color, 0);
}
#endif


void RB_MultiPost(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.multipostShader);

	GLSL_SetUniformInt(&tr.multipostShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.multipostShader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.multipostShader, color, 0);
}

void TR_AxisToAngles ( const vec3_t axis[3], vec3_t angles )
{
    vec3_t right;
    
    // vec3_origin is the origin (0, 0, 0).
    VectorSubtract (vec3_origin, axis[1], right);
    
    if ( axis[0][2] > 0.999f )
    {
        angles[PITCH] = -90.0f;
        angles[YAW] = RAD2DEG (atan2f (-right[0], right[1]));
        angles[ROLL] = 0.0f;
    }
    else if ( axis[0][2] < -0.999f )
    {
        angles[PITCH] = 90.0f;
        angles[YAW] = RAD2DEG (atan2f (-right[0], right[1]));
        angles[ROLL] = 0.0f;
    }
    else
    {
        angles[PITCH] = RAD2DEG (asinf (-axis[0][2]));
        angles[YAW] = RAD2DEG (atan2f (axis[0][1], axis[0][0]));
        angles[ROLL] = RAD2DEG (atan2f (-right[2], axis[2][2]));
    }
}

bool TR_WorldToScreen(vec3_t worldCoord, float *x, float *y)
{
	int	xcenter, ycenter;
	vec3_t	local, transformed;
	vec3_t	vfwd, vright, vup, viewAngles;
	
	TR_AxisToAngles(backEnd.refdef.viewaxis, viewAngles);

	//NOTE: did it this way because most draw functions expect virtual 640x480 coords
	//	and adjust them for current resolution
	xcenter = 640 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn
	ycenter = 480 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn

	VectorSubtract (worldCoord, backEnd.refdef.vieworg, local);

	AngleVectors (viewAngles, vfwd, vright, vup);

	transformed[0] = DotProduct(local,vright);
	transformed[1] = DotProduct(local,vup);
	transformed[2] = DotProduct(local,vfwd);

	// Make sure Z is not negative.
	if(transformed[2] < 0.01)
	{
		//return false;
		//transformed[2] = 2.0 - transformed[2];
	}

	// Simple convert to screen coords.
	float xzi = xcenter / transformed[2] * (90.0/backEnd.refdef.fov_x);
	float yzi = ycenter / transformed[2] * (90.0/backEnd.refdef.fov_y);

	*x = (xcenter + xzi * transformed[0]);
	*y = (ycenter - yzi * transformed[1]);

	return true;
}

qboolean TR_InFOV( vec3_t spot, vec3_t from )
{
	//return qtrue;

	vec3_t	deltaVector, angles, deltaAngles;
	vec3_t	fromAnglesCopy;
	vec3_t	fromAngles;
	//int hFOV = backEnd.refdef.fov_x * 0.5;
	//int vFOV = backEnd.refdef.fov_y * 0.5;
	//int hFOV = 120;
	//int vFOV = 120;
	//int hFOV = backEnd.refdef.fov_x;
	//int vFOV = backEnd.refdef.fov_y;
	//int hFOV = 80;
	//int vFOV = 80;
	int hFOV = backEnd.refdef.fov_x * 0.5;
	int vFOV = backEnd.refdef.fov_y * 0.5;

	TR_AxisToAngles(backEnd.refdef.viewaxis, fromAngles);

	VectorSubtract ( spot, from, deltaVector );
	vectoangles ( deltaVector, angles );
	VectorCopy(fromAngles, fromAnglesCopy);
	
	deltaAngles[PITCH]	= AngleDelta ( fromAnglesCopy[PITCH], angles[PITCH] );
	deltaAngles[YAW]	= AngleDelta ( fromAnglesCopy[YAW], angles[YAW] );

	if ( fabs ( deltaAngles[PITCH] ) <= vFOV && fabs ( deltaAngles[YAW] ) <= hFOV ) 
	{
		return qtrue;
	}

	return qfalse;
}

#include "../cgame/cg_public.h"

void Volumetric_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, const int passEntityNum, const int contentmask )
{
	results->entityNum = ENTITYNUM_NONE;
	ri->CM_BoxTrace(results, start, end, mins, maxs, 0, contentmask, 0);
	results->entityNum = results->fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
}

vec3_t		VOLUMETRIC_ROOF;
qboolean	VOLUMETRIC_HIT_SKY = qfalse;

qboolean Volumetric_Visible(vec3_t from, vec3_t to, qboolean isSun)
{
#if 0
	return qtrue;
#else
	if (isSun)
	{
		VOLUMETRIC_HIT_SKY = qtrue;
		return qtrue;
	}

	trace_t trace;

	VOLUMETRIC_HIT_SKY = qfalse;

	Volumetric_Trace( &trace, from, NULL, NULL, to, -1, (CONTENTS_SOLID|CONTENTS_TERRAIN) );

	if (trace.surfaceFlags & SURF_SKY)
	{
		VOLUMETRIC_HIT_SKY = qtrue;
	}

	if (!(trace.fraction != 1.0 && Distance(trace.endpos, to) > 96))
	{
		return qtrue;
	}

	return qfalse;
#endif
}

void Volumetric_RoofHeight(vec3_t from)
{
	VOLUMETRIC_HIT_SKY = qfalse;

	trace_t trace;
	vec3_t roofh;
	VectorSet(roofh, from[0], from[1], from[2] + 192);
	Volumetric_Trace( &trace, from, NULL, NULL, roofh, -1, (CONTENTS_SOLID|CONTENTS_TERRAIN) );
	VectorSet(VOLUMETRIC_ROOF, trace.endpos[0], trace.endpos[1], trace.endpos[2] - 8.0);

	if (trace.surfaceFlags & SURF_SKY)
	{
		VOLUMETRIC_HIT_SKY = qtrue;
	}
}

extern void R_WorldToLocal (const vec3_t world, vec3_t local);
extern void R_LocalPointToWorld (const vec3_t local, vec3_t world);

extern vec3_t SUN_POSITION;
extern vec2_t SUN_SCREEN_POSITION;
extern qboolean SUN_VISIBLE;

extern void RE_AddDynamicLightToScene(const vec3_t org, float intensity, float r, float g, float b, int additive, qboolean isGlowBased, float heightScale, float coneAngle, vec3_t coneDirection);

int			CLOSE_TOTAL = 0;
int			CLOSE_LIST[MAX_WORLD_GLOW_DLIGHTS];
float		CLOSE_DIST[MAX_WORLD_GLOW_DLIGHTS];
vec3_t		CLOSE_POS[MAX_WORLD_GLOW_DLIGHTS];
float		CLOSE_RADIUS[MAX_WORLD_GLOW_DLIGHTS];
float		CLOSE_HEIGHTSCALES[MAX_WORLD_GLOW_DLIGHTS];
float		CLOSE_CONEANGLE[MAX_WORLD_GLOW_DLIGHTS];
vec3_t		CLOSE_CONEDIRECTION[MAX_WORLD_GLOW_DLIGHTS];
vec4_t		CLOSE_COLORS[MAX_WORLD_GLOW_DLIGHTS];

extern float		MAP_EMISSIVE_COLOR_SCALE;
extern float		MAP_EMISSIVE_COLOR_SCALE_NIGHT;
extern float		MAP_EMISSIVE_RADIUS_SCALE;
extern float		MAP_EMISSIVE_RADIUS_SCALE_NIGHT;

void RB_AddGlowShaderLights ( void )
{
	if (backEnd.refdef.num_dlights < MAX_DLIGHTS && r_dynamiclight->integer >= 4)
	{// Add (close) map glows as dynamic lights as well...
		CLOSE_TOTAL = 0;

		vec3_t playerOrigin;

		if (backEnd.localPlayerValid)
		{
			VectorCopy(backEnd.localPlayerOrigin, playerOrigin);
			//ri->Printf(PRINT_WARNING, "Local player is at %f %f %f.\n", backEnd.localPlayerOrigin[0], backEnd.localPlayerOrigin[1], backEnd.localPlayerOrigin[2]);
		}
		else
		{
			VectorCopy(backEnd.refdef.vieworg, playerOrigin);
			//ri->Printf(PRINT_WARNING, "No Local player! Using vieworg at %f %f %f.\n", backEnd.localPlayerOrigin[0], backEnd.localPlayerOrigin[1], backEnd.localPlayerOrigin[2]);
		}

		//int		farthest_light = -1;
		//float	farthest_distance = 0.0;

		for (int maplight = 0; maplight < NUM_MAP_GLOW_LOCATIONS; maplight++)
		{
			float distance = Distance(playerOrigin, MAP_GLOW_LOCATIONS[maplight]);
			//float distance = Distance(backEnd.refdef.vieworg, MAP_GLOW_LOCATIONS[maplight]);
			qboolean bad = qfalse;

			// We need to have some sanity... Basic max light range...
			if (distance > MAX_WORLD_GLOW_DLIGHT_RANGE) continue;

			// If further then the map's distance cull setting, skip...
			if (distance > tr.distanceCull * 1.732) continue;

			// If occlusion is enabled and this light is further away, skip it, obviously...
			if (r_occlusion->integer && distance > tr.occlusionZfar) continue;

			// If the list is full and this one is as far or further then the current furthest light, skip the calculations...
			//if (farthest_light != -1 && distance >= farthest_distance && !(CLOSE_TOTAL < MAX_WORLD_GLOW_DLIGHTS)) continue;

#if 0
			for (int i = 0; i < CLOSE_TOTAL; i++)
			{
				if (Distance(CLOSE_POS[i], MAP_GLOW_LOCATIONS[maplight]) < 64.0)
				{// Too close to another light...
					bad = qtrue;
					break;
				}
			}

			if (bad) 
			{
				continue;
			}
#endif

#if 0
			if (!R_inPVS(tr.refdef.vieworg, MAP_GLOW_LOCATIONS[maplight], tr.refdef.areamask))
			{// Not in PVS, don't add it...
				continue;
			}
#endif

			if (CLOSE_TOTAL < MAX_WORLD_GLOW_DLIGHTS)
			{// Have free light slots for a new light...
				CLOSE_LIST[CLOSE_TOTAL] = maplight;
				CLOSE_DIST[CLOSE_TOTAL] = distance;
				VectorCopy(MAP_GLOW_LOCATIONS[maplight], CLOSE_POS[CLOSE_TOTAL]);
				CLOSE_RADIUS[CLOSE_TOTAL] = MAP_GLOW_RADIUSES[maplight];
				CLOSE_HEIGHTSCALES[CLOSE_TOTAL] = MAP_GLOW_HEIGHTSCALES[maplight];
				CLOSE_CONEANGLE[CLOSE_TOTAL] = MAP_GLOW_CONEANGLE[maplight];
				VectorCopy(MAP_GLOW_CONEDIRECTION[maplight], CLOSE_CONEDIRECTION[CLOSE_TOTAL]);
				CLOSE_TOTAL++;
				continue;
			}
			else
			{// See if this is closer then one of our other lights...
				int		farthest_light = 0;
				float	farthest_distance = CLOSE_DIST[0];

				for (int i = 1; i < CLOSE_TOTAL; i++)
				{// Find the most distance light in our current list to replace, if this new option is closer...
					if (CLOSE_DIST[i] > farthest_distance)
					{// This one is further!
						farthest_light = i;
						farthest_distance = CLOSE_DIST[i];
					}
				}

				if (distance < farthest_distance)
				{// This light is closer. Replace this one in our array of closest lights...
					CLOSE_LIST[farthest_light] = maplight;
					CLOSE_DIST[farthest_light] = distance;
					VectorCopy(MAP_GLOW_LOCATIONS[maplight], CLOSE_POS[farthest_light]);
					CLOSE_RADIUS[farthest_light] = MAP_GLOW_RADIUSES[maplight];
					CLOSE_HEIGHTSCALES[farthest_light] = MAP_GLOW_HEIGHTSCALES[maplight];
					CLOSE_CONEANGLE[farthest_light] = MAP_GLOW_CONEANGLE[maplight];
					VectorCopy(MAP_GLOW_CONEDIRECTION[maplight], CLOSE_CONEDIRECTION[farthest_light]);
				}
			}
		}

		//ri->Printf(PRINT_WARNING, "VLIGHT DEBUG: %i visible of %i total glow lights.\n", CLOSE_TOTAL, NUM_MAP_GLOW_LOCATIONS);

		int num_colored = 0;
		int num_uncolored = 0;

		for (int i = 0; i < CLOSE_TOTAL && backEnd.refdef.num_dlights < MAX_DLIGHTS; i++)
		{
			if (MAP_GLOW_COLORS_AVILABLE[CLOSE_LIST[i]])
			{
				vec4_t glowColor = { 0 };
				float strength = 1.0 - Q_clamp(0.0, Distance(MAP_GLOW_LOCATIONS[CLOSE_LIST[i]], playerOrigin) / MAX_WORLD_GLOW_DLIGHT_RANGE, 1.0);
				//float strength = 1.0 - Q_clamp(0.0, Distance(MAP_GLOW_LOCATIONS[CLOSE_LIST[i]], backEnd.refdef.vieworg) / MAX_WORLD_GLOW_DLIGHT_RANGE, 1.0);
				VectorCopy4(MAP_GLOW_COLORS[CLOSE_LIST[i]], glowColor);
				VectorScale(glowColor, r_debugEmissiveColorScale->value, glowColor);
				//VectorScale(glowColor, MAP_EMISSIVE_COLOR_SCALE, glowColor);

				//float gMax = max(glowColor[0], max(glowColor[1], glowColor[2]));
				//glowColor[0] /= gMax;
				//glowColor[1] /= gMax;
				//glowColor[2] /= gMax;
				//glowColor[3] = 1.0;

				VectorCopy4(glowColor, CLOSE_COLORS[i]);
				
				float dayNightFactor = mix(MAP_EMISSIVE_RADIUS_SCALE, MAP_EMISSIVE_RADIUS_SCALE_NIGHT, RB_NightScale());
				float radius = CLOSE_RADIUS[i] * strength * dayNightFactor * 0.2 * r_debugEmissiveRadiusScale->value;
				RE_AddDynamicLightToScene( MAP_GLOW_LOCATIONS[CLOSE_LIST[i]], radius, glowColor[0], glowColor[1], glowColor[2], qfalse, qtrue, CLOSE_HEIGHTSCALES[i], CLOSE_CONEANGLE[i], CLOSE_CONEDIRECTION[i]);
				num_colored++;

				//if (radius <= 0 || glowColor[0] < 0.0 || glowColor[1] < 0.0 || glowColor[2] < 0.0 || glowColor[0] > 1.0 || glowColor[1] > 1.0 || glowColor[2] > 1.0)
				//	ri->Printf(PRINT_ALL, "glow location %i color: %f %f %f. Radius %f.\n", num_colored-1, glowColor[0], glowColor[1], glowColor[2], radius);
			}
#if 0
			else
			{
				float strength = 1.0 - Q_clamp(0.0, Distance(MAP_GLOW_LOCATIONS[CLOSE_LIST[i]], playerOrigin) / MAX_WORLD_GLOW_DLIGHT_RANGE, 1.0);
				//float strength = 1.0 - Q_clamp(0.0, Distance(MAP_GLOW_LOCATIONS[CLOSE_LIST[i]], backEnd.refdef.vieworg) / MAX_WORLD_GLOW_DLIGHT_RANGE, 1.0);
				RE_AddDynamicLightToScene( MAP_GLOW_LOCATIONS[CLOSE_LIST[i]], CLOSE_RADIUS[i] * strength, -1.0, -1.0, -1.0, qfalse, qtrue, CLOSE_HEIGHTSCALES[i], CLOSE_CONEANGLE[i], CLOSE_CONEDIRECTION[i]);
				num_uncolored++;
			}
#endif

			backEnd.refdef.num_dlights++;
		}

		//ri->Printf(PRINT_ALL, "Added %i glow lights with color. %i without.\n", num_colored, num_uncolored);
	}
}

void WorldCoordToScreenCoord(vec3_t origin, float *x, float *y)
{
#if 1
	int	xcenter, ycenter;
	vec3_t	local, transformed;
	vec3_t	vfwd, vright, vup, viewAngles;

	TR_AxisToAngles(backEnd.refdef.viewaxis, viewAngles);

	//NOTE: did it this way because most draw functions expect virtual 640x480 coords
	//	and adjust them for current resolution
	xcenter = glConfig.vidWidth / 2;
	ycenter = glConfig.vidHeight / 2;

	VectorSubtract(origin, backEnd.refdef.vieworg, local);

	AngleVectors(viewAngles, vfwd, vright, vup);

	transformed[0] = DotProduct(local, vright);
	transformed[1] = DotProduct(local, vup);
	transformed[2] = DotProduct(local, vfwd);

	// Make sure Z is not negative.
	/*if(transformed[2] < 0.01)
	{
		transformed[2] *= -1.0;
	}*/


	// Simple convert to screen coords.
	float xzi = xcenter / transformed[2] * (95.0 / backEnd.refdef.fov_x);
	float yzi = ycenter / transformed[2] * (106.0 / backEnd.refdef.fov_y);

	*x = (xcenter + xzi * transformed[0]);
	*y = (ycenter - yzi * transformed[1]);

	*x = *x / glConfig.vidWidth;
	//*y = 1.0 - (*y / glConfig.vidHeight);
	*y = (*y / glConfig.vidHeight);
	*y = 1.0 - *y;

	/*if (transformed[2] < 0.01)
	{
		//*x = -1;
		//*y = -1;
		//*x = Q_clamp(-1.0, *x, 1.0);
		//*y = Q_clamp(-1.0, *y, 1.0);
		*x = Q_clamp(-1.0, *x, 2.0);
		*y = Q_clamp(-1.0, *y, 2.0);
	}*/
#else // this just sucks...
	vec4_t pos, hpos;
	VectorSet4(pos, origin[0], origin[1], origin[2], 1.0);

	// project sun point
	Matrix16Transform(glState.modelviewProjection, pos, hpos);

	// transform to UV coords
	hpos[3] = 0.5f / hpos[3];

	pos[0] = 0.5f + hpos[0] * hpos[3];
	pos[1] = 0.5f + hpos[1] * hpos[3];

	*x = pos[0];
	*y = pos[1];
#endif
}

extern int			r_numdlights;

float RB_PixelDistance(float from[2], float to[2])
{
	float x = from[0] - to[1];
	float y = from[1] - to[1];

	if (x < 0) x = -x;
	if (y < 0) y = -y;

	return x + y;
}

extern float SUN_VOLUMETRIC_SCALE;
extern float SUN_VOLUMETRIC_FALLOFF;

qboolean RB_GenerateVolumeLightImage(void)
{
	/*if ( !SUN_VISIBLE )
	{
	VOLUME_LIGHT_INVERTED = qtrue;
	return qfalse; // meh, gonna skip instead and help fps slightly...
	}*/

	float strengthMult = 1.0;
	if (r_dynamiclight->integer < 3 || (r_dynamiclight->integer > 3 && r_dynamiclight->integer < 6))
		strengthMult = 2.0; // because the lower samples result in less color...

							//ri->Printf(PRINT_WARNING, "Sun screen pos is %f %f.\n", SUN_SCREEN_POSITION[0], SUN_SCREEN_POSITION[1]);

	qboolean VOLUME_LIGHT_INVERTED = qfalse;

	vec3_t sunColor;
	VectorSet(sunColor, backEnd.refdef.sunCol[0] * strengthMult, backEnd.refdef.sunCol[1] * strengthMult, backEnd.refdef.sunCol[2] * strengthMult);

	int dlightShader = r_dynamiclight->integer - 1;

	if (r_dynamiclight->integer >= 4)
	{
		dlightShader -= 3;
	}

	shaderProgram_t *shader = &tr.volumeLightShader[dlightShader];

	if (VOLUME_LIGHT_INVERTED)
		shader = &tr.volumeLightInvertedShader[dlightShader];

#define __USING_SHADOW_MAP__

#ifndef __USING_SHADOW_MAP__
	GLSL_BindProgram(shader);

	GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImage4096, TB_LIGHTMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
	}

	GLSL_SetUniformVec2(shader, UNIFORM_VLIGHTPOSITIONS, SUN_SCREEN_POSITION);
	GLSL_SetUniformVec3(shader, UNIFORM_VLIGHTCOLORS, sunColor);
	GLSL_SetUniformFloat(shader, UNIFORM_VLIGHTDISTANCES, 0.1);

	{
		vec4_t local0;
		VectorSet4(local0, 0.0, 0.0, r_volumeLightStrength->value * 0.4 * SUN_VOLUMETRIC_SCALE, SUN_VOLUMETRIC_FALLOFF); // * 0.4 to compensate for old setting.
		GLSL_SetUniformVec4(shader, UNIFORM_LOCAL0, local0);
	}

	{
		vec4_t local1;
		VectorSet4(local1, DAY_NIGHT_CYCLE_ENABLED ? RB_NightScale() : 0.0, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value);
		GLSL_SetUniformVec4(shader, UNIFORM_LOCAL1, local1);
	}


	//FBO_Blit(hdrFbo, NULL, NULL, tr.volumetricFbo, NULL, shader, color, 0);
	FBO_BlitFromTexture(tr.anamorphicRenderFBOImage, NULL, NULL, tr.volumetricFbo, NULL, shader, color, 0);
#else //__USING_SHADOW_MAP__
	FBO_t *oldFbo = glState.currentFBO;

	float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	qglClearBufferfv(GL_COLOR, 1, black);

	vec4_t quadVerts[4];
	vec2_t texCoords[4];
	vec4_t box;

	FBO_Bind(tr.volumetricFbo);

	box[0] = backEnd.viewParms.viewportX      * tr.volumetricFbo->width / ((float)glConfig.vidWidth * r_superSampleMultiplier->value);
	box[1] = backEnd.viewParms.viewportY      * tr.volumetricFbo->height / ((float)glConfig.vidHeight * r_superSampleMultiplier->value);
	box[2] = backEnd.viewParms.viewportWidth  * tr.volumetricFbo->width / ((float)glConfig.vidWidth * r_superSampleMultiplier->value);
	box[3] = backEnd.viewParms.viewportHeight * tr.volumetricFbo->height / ((float)glConfig.vidHeight * r_superSampleMultiplier->value);

	qglViewport(box[0], box[1], box[2], box[3]);
	qglScissor(box[0], box[1], box[2], box[3]);

	box[0] = backEnd.viewParms.viewportX / ((float)glConfig.vidWidth * r_superSampleMultiplier->value);
	box[1] = backEnd.viewParms.viewportY / ((float)glConfig.vidHeight * r_superSampleMultiplier->value);
	box[2] = box[0] + backEnd.viewParms.viewportWidth / ((float)glConfig.vidWidth * r_superSampleMultiplier->value);
	box[3] = box[1] + backEnd.viewParms.viewportHeight / ((float)glConfig.vidHeight * r_superSampleMultiplier->value);

	texCoords[0][0] = box[0]; texCoords[0][1] = box[3];
	texCoords[1][0] = box[2]; texCoords[1][1] = box[3];
	texCoords[2][0] = box[2]; texCoords[2][1] = box[1];
	texCoords[3][0] = box[0]; texCoords[3][1] = box[1];

	box[0] = -1.0f;
	box[1] = -1.0f;
	box[2] = 1.0f;
	box[3] = 1.0f;

	VectorSet4(quadVerts[0], box[0], box[3], 0, 1);
	VectorSet4(quadVerts[1], box[2], box[3], 0, 1);
	VectorSet4(quadVerts[2], box[2], box[1], 0, 1);
	VectorSet4(quadVerts[3], box[0], box[1], 0, 1);

	GL_State(GLS_DEPTHTEST_DISABLE);

	GLSL_BindProgram(shader);

	GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_COLORMAP);
	GL_BindToTMU(tr.linearDepthImageZfar, TB_COLORMAP);

	GLSL_SetUniformInt(shader, UNIFORM_POSITIONMAP, TB_POSITIONMAP);
	GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

	GLSL_SetUniformInt(shader, UNIFORM_SHADOWMAP, TB_SHADOWMAP);
	GL_BindToTMU(tr.sunShadowDepthImage[0], TB_SHADOWMAP);
	GLSL_SetUniformMatrix16(shader, UNIFORM_SHADOWMVP, backEnd.refdef.sunShadowMvp[0]);

	GLSL_SetUniformInt(shader, UNIFORM_SHADOWMAP2, TB_SHADOWMAP2);
	GL_BindToTMU(tr.sunShadowDepthImage[1], TB_SHADOWMAP2);
	GLSL_SetUniformMatrix16(shader, UNIFORM_SHADOWMVP2, backEnd.refdef.sunShadowMvp[1]);

	GLSL_SetUniformInt(shader, UNIFORM_SHADOWMAP3, TB_SHADOWMAP3);
	GL_BindToTMU(tr.sunShadowDepthImage[2], TB_SHADOWMAP3);
	GLSL_SetUniformMatrix16(shader, UNIFORM_SHADOWMVP3, backEnd.refdef.sunShadowMvp[2]);

	GLSL_SetUniformVec3(shader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg);

	GLSL_SetUniformVec3(shader, UNIFORM_VLIGHTCOLORS, sunColor);

	vec4_t sPos;
	sPos[0] = SUN_POSITION[0];
	sPos[1] = SUN_POSITION[1];
	sPos[2] = SUN_POSITION[2];
	sPos[3] = 1.0;
	GLSL_SetUniformVec4(shader, UNIFORM_PRIMARYLIGHTORIGIN, sPos);

	vec4_t vec;
	VectorSet4(vec, r_shadowSamples->value, r_shadowMapSize->value, r_testshaderValue1->value, r_testshaderValue2->value);
	GLSL_SetUniformVec4(shader, UNIFORM_SETTINGS0, vec);

	{
		vec4_t viewInfo;

		float zmax = r_occlusion->integer ? tr.occlusionOriginalZfar : backEnd.viewParms.zFar;

		float zmax2 = backEnd.viewParms.zFar;
		float ymax2 = zmax2 * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax2 = zmax2 * tan(backEnd.viewParms.fovX * M_PI / 360.0f);

		float zmin = r_znear->value;

		vec3_t viewBasis[3];
		VectorScale(backEnd.refdef.viewaxis[0], zmax2, viewBasis[0]);
		VectorScale(backEnd.refdef.viewaxis[1], xmax2, viewBasis[1]);
		VectorScale(backEnd.refdef.viewaxis[2], ymax2, viewBasis[2]);

		GLSL_SetUniformVec3(shader, UNIFORM_VIEWFORWARD, viewBasis[0]);
		GLSL_SetUniformVec3(shader, UNIFORM_VIEWLEFT, viewBasis[1]);
		GLSL_SetUniformVec3(shader, UNIFORM_VIEWUP, viewBasis[2]);

		VectorSet4(viewInfo, zmax / zmin, zmax, /*r_hdr->integer ? 32.0 :*/ 24.0, zmin);

		GLSL_SetUniformVec4(shader, UNIFORM_VIEWINFO, viewInfo);


		{
			vec4_t local0;
			VectorSet4(local0, r_lensflare->integer ? 1.0 : 0.0, 0.0, r_volumeLightStrength->value * 0.4 * SUN_VOLUMETRIC_SCALE, SUN_VOLUMETRIC_FALLOFF); // * 0.4 to compensate for old setting.
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL0, local0);
		}

		{
			vec4_t local1;
			VectorSet4(local1, DAY_NIGHT_CYCLE_ENABLED ? RB_NightScale() : 0.0, 0.0, 0.0, 0.0);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL1, local1);
		}

		{
			vec4_t local2;
			VectorSet4(local2, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL2, local2);
		}

		RB_InstantQuad2(quadVerts, texCoords); //, color, shaderProgram, invTexRes);

											   // reset viewport and scissor
		FBO_Bind(oldFbo);
		SetViewportAndScissor();
	}
#endif //__USING_SHADOW_MAP__

	return qtrue;
}

qboolean RB_VolumetricLight(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;
	int NUM_VISIBLE_LIGHTS = 0;
	int SUN_ID = -1;

	/*if ( !SUN_VISIBLE )
	{
	VOLUME_LIGHT_INVERTED = qtrue;
	return qfalse; // meh, gonna skip instead and help fps slightly...
	}*/

	color[0] =
	color[1] =
	color[2] = 
	color[3] = 1.0f;

	// Combine render and volumetrics...
	GLSL_BindProgram(&tr.volumeLightCombineShader);

	GLSL_SetUniformMatrix16(&tr.volumeLightCombineShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	//GLSL_SetUniformMatrix16(&tr.volumeLightCombineShader, UNIFORM_MODELMATRIX, backEnd.ori.modelMatrix);

	GLSL_SetUniformInt(&tr.volumeLightCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.volumeLightCombineShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.volumetricFBOImage/*tr.volumetricFbo->colorImage[0]*/, TB_NORMALMAP);

	vec2_t screensize;
	screensize[0] = tr.volumetricFBOImage->width;
	screensize[1] = tr.volumetricFBOImage->height;
	GLSL_SetUniformVec2(&tr.volumeLightCombineShader, UNIFORM_DIMENSIONS, screensize);

	{
		vec4_t local1;
		VectorSet4(local1, backEnd.refdef.floatTime, 1.0, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.volumeLightCombineShader, UNIFORM_LOCAL1, local1);
	}

	FBO_Blit(hdrFbo, NULL, NULL, ldrFbo, ldrBox, &tr.volumeLightCombineShader, color, 0);

	//ri->Printf(PRINT_WARNING, "%i visible dlights. %i total dlights.\n", NUM_CLOSE_VLIGHTS, backEnd.refdef.num_dlights);
	return qtrue;
}

void RB_MagicDetail(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.magicdetailShader);

	GLSL_SetUniformMatrix16(&tr.magicdetailShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformInt(&tr.magicdetailShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
	GLSL_SetUniformInt(&tr.magicdetailShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImage2048, TB_LIGHTMAP);

	GLSL_SetUniformMatrix16(&tr.magicdetailShader, UNIFORM_INVEYEPROJECTIONMATRIX, glState.invEyeProjection);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.magicdetailShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;

		//float zmax = backEnd.viewParms.zFar;
		float zmax = 2048.0;//backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, zmin, zmax);

		GLSL_SetUniformVec4(&tr.magicdetailShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec4_t local0;
		VectorSet4(local0, r_magicdetailStrength->value, r_magicdetailMix->value, 0.0, 0.0); // non-flicker version
		GLSL_SetUniformVec4(&tr.magicdetailShader, UNIFORM_LOCAL0, local0);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.magicdetailShader, color, 0);
}

void RB_CellShade(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.cellShadeShader);

	GLSL_SetUniformMatrix16(&tr.cellShadeShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformInt(&tr.cellShadeShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
	GLSL_SetUniformInt(&tr.cellShadeShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.renderNormalImage, TB_NORMALMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.cellShadeShader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.cellShadeShader, color, 0);
}

void RB_Paint(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.paintShader);

	GLSL_SetUniformMatrix16(&tr.paintShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformInt(&tr.paintShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.paintShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t local0;
		VectorSet4(local0, r_testvalue1->value, 0.0, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.paintShader, UNIFORM_LOCAL0, local0);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.paintShader, color, 0);
}

void RB_Anaglyph(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.anaglyphShader);

	GLSL_SetUniformMatrix16(&tr.anaglyphShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformInt(&tr.anaglyphShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.anaglyphShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage/*tr.linearDepthImageZfar*/, TB_LIGHTMAP); // UQ1: uses whacky code to work out linear atm...

	{
		vec4_t viewInfo;

		//float zmax = backEnd.viewParms.zFar;
		float zmax = /*2048.0;*/backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.anaglyphShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.anaglyphShader, UNIFORM_DIMENSIONS, screensize);
	}
	
	{
		vec4_t local0;
		VectorSet4(local0, r_trueAnaglyphSeparation->value, r_trueAnaglyphRed->value, r_trueAnaglyphGreen->value, r_trueAnaglyphBlue->value);
		GLSL_SetUniformVec4(&tr.anaglyphShader, UNIFORM_LOCAL0, local0);
	}

	{
		vec4_t local1;
		VectorSet4(local1, r_trueAnaglyph->value, r_trueAnaglyphMinDistance->value, r_trueAnaglyphMaxDistance->value, r_trueAnaglyphParallax->value);
		GLSL_SetUniformVec4(&tr.anaglyphShader, UNIFORM_LOCAL1, local1);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.anaglyphShader, color, 0);
}

void RB_SSAO(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.ssaoShader);

	GLSL_SetUniformMatrix16(&tr.ssaoShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	GLSL_SetUniformMatrix16(&tr.ssaoShader, UNIFORM_MODELMATRIX, backEnd.ori.modelMatrix);

	GLSL_SetUniformInt(&tr.ssaoShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.ssaoShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImage4096, TB_LIGHTMAP);

	GLSL_SetUniformInt(&tr.ssaoShader, UNIFORM_POSITIONMAP, TB_POSITIONMAP);
	GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

	GLSL_SetUniformInt(&tr.ssaoShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.renderNormalImage, TB_NORMALMAP);

	if (r_normalMappingReal->integer)
	{
		GLSL_SetUniformInt(&tr.ssaoShader, UNIFORM_OVERLAYMAP, TB_OVERLAYMAP);
		GL_BindToTMU(tr.renderNormalDetailedImage, TB_OVERLAYMAP);
	}
	
	{
		vec4_t viewInfo;
		//float zmax = backEnd.viewParms.zFar;
		float zmax = 1024.0;//backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(&tr.ssaoShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = tr.ssaoImage->width;//glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = tr.ssaoImage->height;//glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.ssaoShader, UNIFORM_DIMENSIONS, screensize);
	}

	vec3_t out;
	float dist = 4096.0;//backEnd.viewParms.zFar / 1.75;
	VectorMA(backEnd.refdef.vieworg, dist, backEnd.refdef.sunDir, out);
	GLSL_SetUniformVec4(&tr.ssaoShader, UNIFORM_PRIMARYLIGHTORIGIN, out);

	GLSL_SetUniformVec3(&tr.ssaoShader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg);

	{
		vec4_t local0;
		VectorSet4(local0, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value);
		GLSL_SetUniformVec4(&tr.ssaoShader, UNIFORM_LOCAL0, local0);
	}
	//tr.viewParms.ori.axis
	//TR_AxisToAngles(backEnd.refdef.viewaxis, backEnd.refdef.viewangles);
	//TR_AxisToAngles(tr.refdef.viewaxis, tr.refdef.viewangles);
	//ri->Printf(PRINT_ALL, "be %f. tr %f.\n", backEnd.refdef.viewangles[ROLL], tr.refdef.viewangles[ROLL]);
	
	vec3_t mAngles, mCameraForward, mCameraLeft, mCameraDown;
	TR_AxisToAngles(backEnd.viewParms.ori.axis, mAngles);
	AngleVectors(mAngles, mCameraForward, mCameraLeft, mCameraDown);
	//ri->Printf(PRINT_ALL, "mCameraForward %f. mCameraDown %f. mAngles %f.\n", mCameraForward[ROLL], mCameraDown[ROLL], mAngles[ROLL]);

	{
		vec4_t local1;
		VectorSet4(local1, mCameraForward[PITCH], mCameraForward[YAW], mCameraForward[ROLL], 0.0);
		GLSL_SetUniformVec4(&tr.ssaoShader, UNIFORM_LOCAL1, local1);
	}

	FBO_Blit(hdrFbo, NULL, NULL, tr.ssaoFbo, NULL, &tr.ssaoShader, color, 0);
}

extern float mix(float x, float y, float a);

extern float		MAP_WATER_LEVEL;
extern vec3_t		MAP_INFO_MINS;
extern vec3_t		MAP_INFO_MAXS;
extern vec3_t		MAP_INFO_SIZE;
extern vec3_t		MAP_INFO_PIXELSIZE;
extern vec3_t		MAP_INFO_SCATTEROFFSET;
extern float		MAP_INFO_MAXSIZE;

extern float		WATER_REFLECTIVENESS;
extern float		WATER_CLARITY;
extern float		WATER_UNDERWATER_CLARITY;
extern vec3_t		WATER_COLOR_SHALLOW;
extern vec3_t		WATER_COLOR_DEEP;

#if 0
void RB_SSS(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	//
	// Generate occlusion map...
	//

	GLSL_BindProgram(&tr.sssShader);

	GLSL_SetUniformMatrix16(&tr.sssShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	GLSL_SetUniformMatrix16(&tr.sssShader, UNIFORM_MODELVIEWMATRIX, glState.modelview);
	GLSL_SetUniformMatrix16(&tr.sssShader, UNIFORM_PROJECTIONMATRIX, glState.projection);

	GLSL_SetUniformInt(&tr.sssShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.sssShader, UNIFORM_POSITIONMAP, TB_POSITIONMAP);
	GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

	GLSL_SetUniformInt(&tr.sssShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.renderNormalImage, TB_NORMALMAP);

	GLSL_SetUniformInt(&tr.sssShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImage4096/*tr.linearDepthImageZfar*/, TB_LIGHTMAP);

	GLSL_SetUniformInt(&tr.sssShader, UNIFORM_DELUXEMAP, TB_DELUXEMAP);
	GL_BindToTMU(tr.random2KImage[0], TB_DELUXEMAP);

	GLSL_SetUniformVec3(&tr.sssShader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg);

	vec2_t screensize;
	screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
	screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;
	GLSL_SetUniformVec2(&tr.sssShader, UNIFORM_DIMENSIONS, screensize);
	
	vec4_t viewInfo;
	float zmax = /*4096.0;*/ backEnd.viewParms.zFar;
	float ratio = screensize[0] / screensize[1];
	float xmax = tan(backEnd.viewParms.fovX * ratio * 0.5);
	float ymax = tan(backEnd.viewParms.fovY * 0.5);
	float zmin = r_znear->value;
	VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
	GLSL_SetUniformVec4(&tr.sssShader, UNIFORM_VIEWINFO, viewInfo);

	vec3_t out;
	float dist = 4096.0;//backEnd.viewParms.zFar / 1.75;
	VectorMA(backEnd.refdef.vieworg, dist, backEnd.refdef.sunDir, out);
	GLSL_SetUniformVec4(&tr.sssShader, UNIFORM_PRIMARYLIGHTORIGIN, out);

	vec4_t local0;
	VectorSet4(local0, tr.random2KImage[0]->width, tr.random2KImage[0]->height, SUN_SCREEN_POSITION[0], SUN_SCREEN_POSITION[1]);
	GLSL_SetUniformVec4(&tr.sssShader, UNIFORM_LOCAL0, local0);

	vec4_t local1;
	VectorSet4(local1, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value);
	GLSL_SetUniformVec4(&tr.sssShader, UNIFORM_LOCAL1, local1);

	vec4_t local2;
	VectorSet4(local2, MAP_INFO_SIZE[2], MAP_INFO_MINS[2], MAP_INFO_MAXS[2], 0.0);
	GLSL_SetUniformVec4(&tr.sssShader, UNIFORM_LOCAL2, local2);

	// Light positions (just sun for testing)
	GLSL_SetUniformVec2(&tr.sssShader, UNIFORM_VLIGHTPOSITIONS, SUN_SCREEN_POSITION);

	//FBO_Blit(hdrFbo, hdrBox, NULL, tr.sssFbo1, ldrBox, &tr.sssShader, color, 0);
	//FBO_Blit(hdrFbo, hdrBox, NULL, tr.sssFbo2, ldrBox, &tr.sssShader, color, 0);
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.sssShader, color, 0);
}
#endif

extern qboolean WATER_USE_OCEAN;
extern float WATER_WAVE_HEIGHT;
extern float WATER_EXTINCTION1;
extern float WATER_EXTINCTION2;
extern float WATER_EXTINCTION3;

void RB_WaterPost(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	if (WATER_REFLECTIVENESS > 0.0 && r_glslWater->integer > 1.0 && !(tr.refdef.rdflags & RDF_UNDERWATER))
	{// Generate reflections...
		shaderProgram_t *shader = &tr.waterReflectionShader;

		GLSL_BindProgram(shader);

		GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

		GLSL_SetUniformInt(shader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

		GLSL_SetUniformInt(shader, UNIFORM_POSITIONMAP, TB_POSITIONMAP);
		GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

		GLSL_SetUniformInt(shader, UNIFORM_WATERPOSITIONMAP, TB_WATERPOSITIONMAP);
		GL_BindToTMU(tr.waterPositionMapImage, TB_WATERPOSITIONMAP);

		GLSL_SetUniformInt(shader, UNIFORM_OVERLAYMAP, TB_OVERLAYMAP);
		GL_BindToTMU(tr.waterFoamImage[0], TB_OVERLAYMAP);

		GLSL_SetUniformInt(shader, UNIFORM_SPLATMAP1, TB_SPLATMAP1);
		GL_BindToTMU(tr.waterFoamImage[1], TB_SPLATMAP1);

		GLSL_SetUniformInt(shader, UNIFORM_SPLATMAP2, TB_SPLATMAP2);
		GL_BindToTMU(tr.waterFoamImage[2], TB_SPLATMAP2);

		GLSL_SetUniformInt(shader, UNIFORM_SPLATMAP3, TB_SPLATMAP3);
		GL_BindToTMU(tr.waterFoamImage[3], TB_SPLATMAP3);

		GLSL_SetUniformInt(shader, UNIFORM_DETAILMAP, TB_DETAILMAP);
		GL_BindToTMU(tr.waterCausicsImage, TB_DETAILMAP);

		GLSL_SetUniformInt(shader, UNIFORM_HEIGHTMAP, TB_HEIGHTMAP);
		GL_BindToTMU(tr.waterHeightMapImage, TB_HEIGHTMAP);

		GLSL_SetUniformInt(shader, UNIFORM_SKYCUBEMAP, TB_SKYCUBEMAP);
		GL_BindToTMU(tr.skyCubeMap, TB_SKYCUBEMAP);

		GLSL_SetUniformInt(shader, UNIFORM_SKYCUBEMAPNIGHT, TB_SKYCUBEMAPNIGHT);
		GL_BindToTMU(tr.skyCubeMapNight, TB_SKYCUBEMAPNIGHT);

		GLSL_SetUniformVec3(shader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg);
		GLSL_SetUniformFloat(shader, UNIFORM_TIME, backEnd.refdef.floatTime);


		vec3_t out;
		float dist = 4096.0;
		VectorMA(backEnd.refdef.vieworg, dist, backEnd.refdef.sunDir, out);
		GLSL_SetUniformVec4(shader, UNIFORM_PRIMARYLIGHTORIGIN, out);

		GLSL_SetUniformVec3(shader, UNIFORM_PRIMARYLIGHTCOLOR, backEnd.refdef.sunCol);


		/*GLSL_SetUniformInt(shader, UNIFORM_LIGHTCOUNT, NUM_CLOSE_LIGHTS);
		GLSL_SetUniformVec3xX(shader, UNIFORM_LIGHTPOSITIONS2, CLOSEST_LIGHTS_POSITIONS, NUM_CLOSE_LIGHTS);
		GLSL_SetUniformVec3xX(shader, UNIFORM_LIGHTCOLORS, CLOSEST_LIGHTS_COLORS, NUM_CLOSE_LIGHTS);
		GLSL_SetUniformFloatxX(shader, UNIFORM_LIGHTDISTANCES, CLOSEST_LIGHTS_DISTANCES, NUM_CLOSE_LIGHTS);
		GLSL_SetUniformFloatxX(shader, UNIFORM_LIGHT_CONEANGLES, CLOSEST_LIGHTS_CONEANGLES, NUM_CLOSE_LIGHTS);
		GLSL_SetUniformVec3xX(shader, UNIFORM_LIGHT_CONEDIRECTIONS, CLOSEST_LIGHTS_CONEDIRECTIONS, NUM_CLOSE_LIGHTS);*/

		{
			vec4_t loc;
			VectorSet4(loc, MAP_INFO_MINS[0], MAP_INFO_MINS[1], MAP_INFO_MINS[2], 0.0);
			GLSL_SetUniformVec4(shader, UNIFORM_MINS, loc);

			VectorSet4(loc, MAP_INFO_MAXS[0], MAP_INFO_MAXS[1], MAP_INFO_MAXS[2], 0.0);
			GLSL_SetUniformVec4(shader, UNIFORM_MAXS, loc);

			VectorSet4(loc, MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], (float)SUN_VISIBLE);
			GLSL_SetUniformVec4(shader, UNIFORM_MAPINFO, loc);
		}

		{
			vec4_t loc;
			VectorSet4(loc, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL0, loc);
		}

		{
			vec4_t loc;
			VectorSet4(loc, MAP_WATER_LEVEL, r_glslWater->value, (tr.refdef.rdflags & RDF_UNDERWATER) ? 1.0 : 0.0, WATER_REFLECTIVENESS);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL1, loc);
		}

		{
			vec4_t loc;
			VectorSet4(loc, WATER_COLOR_SHALLOW[0], WATER_COLOR_SHALLOW[1], WATER_COLOR_SHALLOW[2], WATER_CLARITY);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL2, loc);
		}

		{
			vec4_t loc;
			VectorSet4(loc, WATER_COLOR_DEEP[0], WATER_COLOR_DEEP[1], WATER_COLOR_DEEP[2], tr.waterHeightMapImage != tr.whiteImage ? 1.0 : 0.0);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL3, loc);
		}

		{
			vec4_t loc;
			VectorSet4(loc, DAY_NIGHT_CYCLE_ENABLED ? RB_NightScale() : 0.0, WATER_EXTINCTION1, WATER_EXTINCTION2, WATER_EXTINCTION3);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL4, loc);
		}

		{
			vec4_t local7;
			VectorSet4(local7, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value, r_testshaderValue4->value);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL7, local7);
		}

		{
			vec4_t local8;
			VectorSet4(local8, r_testshaderValue5->value, r_testshaderValue6->value, r_testshaderValue7->value, r_testshaderValue8->value);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL8, local8);
		}

		{
			vec4_t loc;
			VectorSet4(loc, WATER_WAVE_HEIGHT, 0.5, WATER_USE_OCEAN, WATER_UNDERWATER_CLARITY);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL10, loc);
		}

		{
			vec4_t viewInfo;
			float zmax = 2048.0;
			float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
			float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
			float zmin = r_znear->value;
			VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
			GLSL_SetUniformVec4(shader, UNIFORM_VIEWINFO, viewInfo);
		}

		{
			vec2_t screensize;
			screensize[0] = tr.waterReflectionRenderImage->width;
			screensize[1] = tr.waterReflectionRenderImage->height;

			GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
		}

		vec4i_t box;
		VectorSet4(box, 0, tr.waterReflectionRenderImage->height, tr.waterReflectionRenderImage->width, -tr.waterReflectionRenderImage->height);
		FBO_Blit(hdrFbo, NULL, NULL, tr.waterReflectionRenderFBO, box, shader, colorWhite, 0);
	}

	{// Actual water draw...
		shaderProgram_t *shader = &tr.waterPostShader[Q_clampi(0, r_glslWater->integer - 1, 2)];

		GLSL_BindProgram(shader);

		GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

		GLSL_SetUniformInt(shader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

		GLSL_SetUniformInt(shader, UNIFORM_POSITIONMAP, TB_POSITIONMAP);
		GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

		GLSL_SetUniformInt(shader, UNIFORM_WATERPOSITIONMAP, TB_WATERPOSITIONMAP);
		GL_BindToTMU(tr.waterPositionMapImage, TB_WATERPOSITIONMAP);

		GLSL_SetUniformInt(shader, UNIFORM_OVERLAYMAP, TB_OVERLAYMAP);
		GL_BindToTMU(tr.waterFoamImage[0], TB_OVERLAYMAP);

		GLSL_SetUniformInt(shader, UNIFORM_SPLATMAP1, TB_SPLATMAP1);
		GL_BindToTMU(tr.waterFoamImage[1], TB_SPLATMAP1);

		GLSL_SetUniformInt(shader, UNIFORM_SPLATMAP2, TB_SPLATMAP2);
		GL_BindToTMU(tr.waterFoamImage[2], TB_SPLATMAP2);

		GLSL_SetUniformInt(shader, UNIFORM_SPLATMAP3, TB_SPLATMAP3);
		GL_BindToTMU(tr.waterFoamImage[3], TB_SPLATMAP3);

		GLSL_SetUniformInt(shader, UNIFORM_DETAILMAP, TB_DETAILMAP);
		GL_BindToTMU(tr.waterCausicsImage, TB_DETAILMAP);

		GLSL_SetUniformInt(shader, UNIFORM_HEIGHTMAP, TB_HEIGHTMAP);
		GL_BindToTMU(tr.waterHeightMapImage, TB_HEIGHTMAP);

		GLSL_SetUniformInt(shader, UNIFORM_SKYCUBEMAP, TB_SKYCUBEMAP);
		GL_BindToTMU(tr.skyCubeMap, TB_SKYCUBEMAP);

		GLSL_SetUniformInt(shader, UNIFORM_SKYCUBEMAPNIGHT, TB_SKYCUBEMAPNIGHT);
		GL_BindToTMU(tr.skyCubeMapNight, TB_SKYCUBEMAPNIGHT);

		GLSL_SetUniformInt(shader, UNIFORM_EMISSIVECUBE, TB_EMISSIVECUBE);
		GL_BindToTMU(tr.waterReflectionRenderImage, TB_EMISSIVECUBE);

		GLSL_SetUniformVec3(shader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg);
		GLSL_SetUniformFloat(shader, UNIFORM_TIME, backEnd.refdef.floatTime);


		vec3_t out;
		float dist = 4096.0;
		VectorMA(backEnd.refdef.vieworg, dist, backEnd.refdef.sunDir, out);
		GLSL_SetUniformVec4(shader, UNIFORM_PRIMARYLIGHTORIGIN, out);

		GLSL_SetUniformVec3(shader, UNIFORM_PRIMARYLIGHTCOLOR, backEnd.refdef.sunCol);


		/*GLSL_SetUniformInt(shader, UNIFORM_LIGHTCOUNT, NUM_CLOSE_LIGHTS);
		GLSL_SetUniformVec3xX(shader, UNIFORM_LIGHTPOSITIONS2, CLOSEST_LIGHTS_POSITIONS, NUM_CLOSE_LIGHTS);
		GLSL_SetUniformVec3xX(shader, UNIFORM_LIGHTCOLORS, CLOSEST_LIGHTS_COLORS, NUM_CLOSE_LIGHTS);
		GLSL_SetUniformFloatxX(shader, UNIFORM_LIGHTDISTANCES, CLOSEST_LIGHTS_DISTANCES, NUM_CLOSE_LIGHTS);
		GLSL_SetUniformFloatxX(shader, UNIFORM_LIGHT_CONEANGLES, CLOSEST_LIGHTS_CONEANGLES, NUM_CLOSE_LIGHTS);
		GLSL_SetUniformVec3xX(shader, UNIFORM_LIGHT_CONEDIRECTIONS, CLOSEST_LIGHTS_CONEDIRECTIONS, NUM_CLOSE_LIGHTS);*/

		{
			vec4_t loc;
			VectorSet4(loc, MAP_INFO_MINS[0], MAP_INFO_MINS[1], MAP_INFO_MINS[2], 0.0);
			GLSL_SetUniformVec4(shader, UNIFORM_MINS, loc);

			VectorSet4(loc, MAP_INFO_MAXS[0], MAP_INFO_MAXS[1], MAP_INFO_MAXS[2], 0.0);
			GLSL_SetUniformVec4(shader, UNIFORM_MAXS, loc);

			VectorSet4(loc, MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], (float)SUN_VISIBLE);
			GLSL_SetUniformVec4(shader, UNIFORM_MAPINFO, loc);
		}

		{
			vec4_t loc;
			VectorSet4(loc, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL0, loc);
		}

		{
			vec4_t loc;
			VectorSet4(loc, MAP_WATER_LEVEL, r_glslWater->value, (tr.refdef.rdflags & RDF_UNDERWATER) ? 1.0 : 0.0, WATER_REFLECTIVENESS);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL1, loc);
		}

		{
			vec4_t loc;
			VectorSet4(loc, WATER_COLOR_SHALLOW[0], WATER_COLOR_SHALLOW[1], WATER_COLOR_SHALLOW[2], WATER_CLARITY);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL2, loc);
		}

		{
			vec4_t loc;
			VectorSet4(loc, WATER_COLOR_DEEP[0], WATER_COLOR_DEEP[1], WATER_COLOR_DEEP[2], tr.waterHeightMapImage != tr.whiteImage ? 1.0 : 0.0);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL3, loc);
		}

		{
			vec4_t loc;
			VectorSet4(loc, DAY_NIGHT_CYCLE_ENABLED ? RB_NightScale() : 0.0, WATER_EXTINCTION1, WATER_EXTINCTION2, WATER_EXTINCTION3);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL4, loc);
		}

		{
			vec4_t local7;
			VectorSet4(local7, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value, r_testshaderValue4->value);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL7, local7);
		}

		{
			vec4_t local8;
			VectorSet4(local8, r_testshaderValue5->value, r_testshaderValue6->value, r_testshaderValue7->value, r_testshaderValue8->value);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL8, local8);
		}

		{
			vec4_t local9;
			VectorSet4(local9, (DAY_NIGHT_CYCLE_ENABLED && RB_NightScale() < /*0.1927*/0.243) ? 1.0 - (RB_NightScale() / 0.243) : 0.0, 0.0, 0.0, 0.0);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL9, local9);
		}

		{
			vec4_t loc;
			VectorSet4(loc, WATER_WAVE_HEIGHT, 0.5, WATER_USE_OCEAN, WATER_UNDERWATER_CLARITY);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL10, loc);
		}

		{
			vec2_t screensize;
			screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
			screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

			GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
		}

		/*{
			vec4_t viewInfo;

			float zmax = r_occlusion->integer ? tr.occlusionOriginalZfar : backEnd.viewParms.zFar;

			float zmax2 = backEnd.viewParms.zFar;
			float ymax2 = zmax2 * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
			float xmax2 = zmax2 * tan(backEnd.viewParms.fovX * M_PI / 360.0f);

			float zmin = r_znear->value;

			vec3_t viewBasis[3];
			VectorScale(backEnd.refdef.viewaxis[0], zmax2, viewBasis[0]);
			VectorScale(backEnd.refdef.viewaxis[1], xmax2, viewBasis[1]);
			VectorScale(backEnd.refdef.viewaxis[2], ymax2, viewBasis[2]);

			GLSL_SetUniformVec3(shader, UNIFORM_VIEWFORWARD, viewBasis[0]);
			GLSL_SetUniformVec3(shader, UNIFORM_VIEWLEFT, viewBasis[1]);
			GLSL_SetUniformVec3(shader, UNIFORM_VIEWUP, viewBasis[2]);

			VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, zmin);

			GLSL_SetUniformVec4(shader, UNIFORM_VIEWINFO, viewInfo);
		}*/

		FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, shader, colorWhite, 0);
	}
}

#if 0
void RB_HBAO(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	shaderProgram_t *shader = &tr.hbaoShader;

	if (r_hbao->integer > 1)
		shader = &tr.hbao2Shader;

	// Generate hbao image...
	GLSL_BindProgram(shader);

	GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformInt(shader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);
	//GLSL_SetUniformInt(shader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	//GL_BindToTMU(tr.renderNormalImage, TB_NORMALMAP);
	GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImage2048, TB_LIGHTMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t local0;
		VectorSet4(local0, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value, r_testshaderValue4->value);
		GLSL_SetUniformVec4(shader, UNIFORM_LOCAL0, local0);
	}

	{
		vec4_t viewInfo;
		//float zmax = backEnd.viewParms.zFar;
		float zmax = 2048.0;//backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(shader, UNIFORM_VIEWINFO, viewInfo);
	}

//#define HBAO_DEBUG
#define HBAO_SINGLE_PASS

#if !defined(HBAO_DEBUG) && !defined(HBAO_SINGLE_PASS)
	FBO_Blit(hdrFbo, hdrBox, NULL, tr.genericFbo2, ldrBox, shader, color, 0);

	// Combine render and hbao...
	GLSL_BindProgram(&tr.hbaoCombineShader);

	GLSL_SetUniformMatrix16(&tr.hbaoCombineShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	GLSL_SetUniformMatrix16(shader, UNIFORM_MODELMATRIX, backEnd.ori.modelMatrix);//backEnd.ori.modelViewMatrix);

	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.hbaoCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.hbaoCombineShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.genericFBO2Image, TB_NORMALMAP);

	vec2_t screensize;
	screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
	screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;
	GLSL_SetUniformVec2(&tr.hbaoCombineShader, UNIFORM_DIMENSIONS, screensize);

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.hbaoCombineShader, color, 0);
#else //defined(HBAO_DEBUG) || defined(HBAO_SINGLE_PASS)
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, shader, color, 0);
#endif //defined(HBAO_DEBUG) || defined(HBAO_SINGLE_PASS)
}
#endif

void RB_ESharpening(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.esharpeningShader);

	GLSL_SetUniformInt(&tr.esharpeningShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	vec2_t screensize;
	screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
	screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;
	GLSL_SetUniformVec2(&tr.esharpeningShader, UNIFORM_DIMENSIONS, screensize);
	
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.esharpeningShader, color, 0);
}

#if 0
void RB_ESharpening2(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.esharpening2Shader);

	GLSL_SetUniformInt(&tr.esharpening2Shader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	vec2_t screensize;
	screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
	screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;
	GLSL_SetUniformVec2(&tr.esharpening2Shader, UNIFORM_DIMENSIONS, screensize);

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.esharpening2Shader, color, 0);
}
#endif

void RB_DofFocusDepth(void)
{
	GLSL_BindProgram(&tr.dofFocusDepthShader);

	GLSL_SetUniformInt(&tr.dofFocusDepthShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImage4096, TB_LIGHTMAP);

	GLSL_SetUniformMatrix16(&tr.dofFocusDepthShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	{
		const float quality = 64.0;// 32.0;
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth / quality;
		screensize[1] = glConfig.vidHeight / quality;

		GLSL_SetUniformVec2(&tr.dofFocusDepthShader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_BlitFromTexture(tr.dofFocusDepthScratchImage, NULL, NULL, tr.dofFocusDepthFbo, NULL, &tr.dofFocusDepthShader, colorWhite, 0);
}

void RB_DOF(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox, int direction)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;
	
	shaderProgram_t *shader = &tr.dofShader[Q_clampi(0, r_dof->integer-1, 2)];

	GLSL_BindProgram(shader);

	GLSL_SetUniformInt(shader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	float zfar = 4096;

	GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImage4096, TB_LIGHTMAP);
	
	GLSL_SetUniformInt(shader, UNIFORM_GLOWMAP, TB_GLOWMAP);
	//GL_BindToTMU(tr.glowFboScaled[0]->colorImage[0], TB_GLOWMAP);
	// Use the most blurred version of glow...
	if (r_anamorphic->integer)
	{
		GL_BindToTMU(tr.anamorphicRenderFBOImage, TB_GLOWMAP);
	}
	else if (r_bloom->integer)
	{
		GL_BindToTMU(tr.bloomRenderFBOImage[0], TB_GLOWMAP);
	}
	else
	{
		GL_BindToTMU(tr.glowFboScaled[0]->colorImage[0], TB_GLOWMAP);
	}

	GLSL_SetUniformInt(shader, UNIFORM_SPECULARMAP, TB_SPECULARMAP);
	GL_BindToTMU(tr.dofFocusDepthImage, TB_SPECULARMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t info;

		info[0] = r_dof->value;
		info[1] = r_dynamicGlow->value;
		info[2] = 0.0;
		info[3] = direction;

		VectorSet4(info, info[0], info[1], info[2], info[3]);

		GLSL_SetUniformVec4(shader, UNIFORM_LOCAL0, info);
	}

	{
		vec4_t loc;
		VectorSet4(loc, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value);
		GLSL_SetUniformVec4(shader, UNIFORM_LOCAL1, loc);
	}

	{
		vec4_t viewInfo;
		//float zmax = backEnd.viewParms.zFar;
		float zmax = zfar;//backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(shader, UNIFORM_VIEWINFO, viewInfo);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, shader, color, 0);
}

extern bool RealInvertMatrix(const float m[16], float invOut[16]);
extern void RealTransposeMatrix(const float m[16], float invOut[16]);
extern void myInverseMatrix (float m[16], float src[16]);

void transpose(float *src, float *dst, const int N, const int M) {
    //#pragma omp parallel for
    for(int n = 0; n<N*M; n++) {
        int i = n/N;
        int j = n%N;
        dst[n] = src[M*j + i];
    }
}

#ifdef __PLAYER_BASED_CUBEMAPS__
extern int			currentPlayerCubemap;
extern vec4_t		currentPlayerCubemapVec;
extern float		currentPlayerCubemapDistance;
#endif //__PLAYER_BASED_CUBEMAPS__

extern qboolean		WATER_ENABLED;
extern int			MAP_LIGHTING_METHOD;
extern float		SKY_LIGHTING_SCALE;
extern float		MAP_GLOW_MULTIPLIER;
extern float		MAP_GLOW_MULTIPLIER_NIGHT;
extern qboolean		MAP_REFLECTION_ENABLED;
extern qboolean		MAP_USE_PALETTE_ON_SKY;
extern float		MAP_HDR_MIN;
extern float		MAP_HDR_MAX;
extern vec3_t		MAP_INFO_PLAYABLE_SIZE;
extern vec3_t		MAP_INFO_PLAYABLE_MAXS;
extern qboolean		PROCEDURAL_SNOW_ENABLED;
extern float		PROCEDURAL_SNOW_HEIGHT_CURVE;
extern float		PROCEDURAL_SNOW_LUMINOSITY_CURVE;
extern float		PROCEDURAL_SNOW_BRIGHTNESS;
extern float		PROCEDURAL_SNOW_LOWEST_ELEVATION;

extern qboolean		MATERIAL_SPECULAR_CHANGED;
extern float		MATERIAL_SPECULAR_STRENGTHS[MATERIAL_LAST];
extern float		MATERIAL_SPECULAR_REFLECTIVENESS[MATERIAL_LAST];

void RB_DeferredLighting(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	shaderProgram_t *shader = &tr.deferredLightingShader[MAP_LIGHTING_METHOD ? 1 : 0];

	GLSL_BindProgram(shader);

	GLSL_SetUniformInt(shader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	//GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	//GL_BindToTMU(tr.linearDepthImageZfar, TB_LIGHTMAP);

	GLSL_SetUniformInt(shader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.renderNormalImage, TB_NORMALMAP);

	GLSL_SetUniformInt(shader, UNIFORM_POSITIONMAP, TB_POSITIONMAP);
	GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

	GLSL_SetUniformInt(shader, UNIFORM_WATERPOSITIONMAP, TB_WATERPOSITIONMAP);
	GL_BindToTMU(tr.waterPositionMapImage, TB_WATERPOSITIONMAP);

	if (r_normalMappingReal->integer)
	{
		GLSL_SetUniformInt(shader, UNIFORM_OVERLAYMAP, TB_OVERLAYMAP);
		GL_BindToTMU(tr.renderNormalDetailedImage, TB_OVERLAYMAP);
	}

	if (r_ao->integer >= 2)
	{
		GLSL_SetUniformInt(shader, UNIFORM_STEEPMAP, TB_STEEPMAP);
		GL_BindToTMU(tr.ssaoImage, TB_STEEPMAP);
	}

	GLSL_SetUniformInt(shader, UNIFORM_GLOWMAP, TB_GLOWMAP);
	GL_BindToTMU(tr.anamorphicRenderFBOImage, TB_GLOWMAP);

	if (SHADOWS_ENABLED)
	{
		if (r_shadowBlur->integer)
		{
			GLSL_SetUniformInt(shader, UNIFORM_SHADOWMAP, TB_SHADOWMAP);
			GL_BindToTMU(tr.screenShadowBlurImage, TB_SHADOWMAP);
		}
		else
		{
			GLSL_SetUniformInt(shader, UNIFORM_SHADOWMAP, TB_SHADOWMAP);
			GL_BindToTMU(tr.screenShadowImage, TB_SHADOWMAP);
		}
	}
	else
	{
		GLSL_SetUniformInt(shader, UNIFORM_SHADOWMAP, TB_SHADOWMAP);
		GL_BindToTMU(tr.whiteImage, TB_SHADOWMAP);
	}

	GLSL_SetUniformInt(shader, UNIFORM_WATER_EDGE_MAP, TB_WATER_EDGE_MAP);
	GL_BindToTMU(tr.shinyImage, TB_WATER_EDGE_MAP);

	GLSL_SetUniformInt(shader, UNIFORM_ROADSCONTROLMAP, TB_ROADSCONTROLMAP);
	GL_BindToTMU(tr.renderPshadowsImage, TB_ROADSCONTROLMAP);

	GLSL_SetUniformInt(shader, UNIFORM_SKYCUBEMAP, TB_SKYCUBEMAP);
	GL_BindToTMU(tr.skyCubeMap, TB_SKYCUBEMAP);

	GLSL_SetUniformInt(shader, UNIFORM_SKYCUBEMAPNIGHT, TB_SKYCUBEMAPNIGHT);
	GL_BindToTMU(tr.skyCubeMapNight, TB_SKYCUBEMAPNIGHT);

	qboolean haveEmissiveCube = qfalse;
	int cubeMapNum = 0;
	vec4_t cubeMapVec;
	float cubeMapRadius;

	if (!r_cubeMapping->integer)
	{// All disabled, don't both with uniforms...
		cubeMapNum = -1;
		cubeMapVec[0] = 0;
		cubeMapVec[1] = 0;
		cubeMapVec[2] = 0;
		cubeMapVec[3] = 0;
	}
#ifndef __REALTIME_CUBEMAP__
	else if (tr.numCubemaps <= 1)
	{
		cubeMapNum = -1;
		cubeMapVec[0] = 0;
		cubeMapVec[1] = 0;
		cubeMapVec[2] = 0;
		cubeMapVec[3] = 0;
		cubeMapRadius = 0;

		GLSL_SetUniformInt(shader, UNIFORM_CUBEMAP, TB_CUBEMAP);
		GL_BindToTMU(tr.blackImage, TB_CUBEMAP);
		GLSL_SetUniformFloat(shader, UNIFORM_CUBEMAPSTRENGTH, 0.0);
		VectorSet4(cubeMapVec, 0.0, 0.0, 0.0, 0.0);
		GLSL_SetUniformVec4(shader, UNIFORM_CUBEMAPINFO, cubeMapVec);
	}
	else
#endif //!__REALTIME_CUBEMAP__
		if (r_cubeMapping->integer >= 1 && !r_lowVram->integer)
		{
			cubeMapNum = currentPlayerCubemap - 1;
#ifdef __REALTIME_CUBEMAP__
			cubeMapVec[0] = backEnd.refdef.realtimeCubemapOrigin[0] - backEnd.refdef.vieworg[0];
			cubeMapVec[1] = backEnd.refdef.realtimeCubemapOrigin[1] - backEnd.refdef.vieworg[1];
			cubeMapVec[2] = backEnd.refdef.realtimeCubemapOrigin[2] - backEnd.refdef.vieworg[2];
#else //!__REALTIME_CUBEMAP__
			//cubeMapVec[0] = currentPlayerCubemapVec[0];
			//cubeMapVec[1] = currentPlayerCubemapVec[1];
			//cubeMapVec[2] = currentPlayerCubemapVec[2];
			cubeMapVec[0] = tr.cubemapOrigins[cubeMapNum][0];
			cubeMapVec[1] = tr.cubemapOrigins[cubeMapNum][1];
			cubeMapVec[2] = tr.cubemapOrigins[cubeMapNum][2];
#endif //__REALTIME_CUBEMAP__
			cubeMapVec[3] = 1.0f;
			cubeMapRadius = 2048.0;// tr.cubemapRadius[currentPlayerCubemap];
			cubeMapVec[3] = cubeMapRadius;

#ifdef __REALTIME_CUBEMAP__
			GLSL_SetUniformInt(shader, UNIFORM_CUBEMAP, TB_CUBEMAP);
			GL_BindToTMU(tr.realtimeCubemap, TB_CUBEMAP);
#else //!__REALTIME_CUBEMAP__
			GLSL_SetUniformInt(shader, UNIFORM_CUBEMAP, TB_CUBEMAP);
			GL_BindToTMU(tr.cubemaps[cubeMapNum], TB_CUBEMAP);
#endif //__REALTIME_CUBEMAP__
			GLSL_SetUniformFloat(shader, UNIFORM_CUBEMAPSTRENGTH, r_cubemapStrength->value * 0.1);
			//VectorScale4(cubeMapVec, 1.0f / cubeMapRadius/*1000.0f*/, cubeMapVec);
			GLSL_SetUniformVec4(shader, UNIFORM_CUBEMAPINFO, cubeMapVec);

#ifdef __EMISSIVE_CUBE_IBL__
			GLSL_SetUniformInt(shader, UNIFORM_EMISSIVECUBE, TB_EMISSIVECUBE);
			if (r_emissiveCubes->integer && tr.emissivemaps[cubeMapNum])
			{
				haveEmissiveCube = qtrue;
				GL_BindToTMU(tr.emissivemaps[cubeMapNum], TB_EMISSIVECUBE);
			}
			else
			{
				GL_BindToTMU(tr.blackCube, TB_EMISSIVECUBE);
			}
#endif //__EMISSIVE_CUBE_IBL__
		}

	int NUM_LIGHTS = r_lowVram->integer ? min(NUM_CLOSE_LIGHTS, min(r_maxDeferredLights->integer, 8.0)) : min(NUM_CLOSE_LIGHTS, min(r_maxDeferredLights->integer, MAX_DEFERRED_LIGHTS));

	float maxDist = 0.0;

	if (NUM_LIGHTS >= r_maxDeferredLights->integer / 2)
	{
		for (int i = 0; i < NUM_LIGHTS; i++)
		{
			//ri->Printf(PRINT_WARNING, "%i - %i %i %i. Range %f. Color %f %f %f.\n", i, (int)CLOSEST_LIGHTS_POSITIONS[i][0], (int)CLOSEST_LIGHTS_POSITIONS[i][1], (int)CLOSEST_LIGHTS_POSITIONS[i][2], CLOSEST_LIGHTS_DISTANCES[i], CLOSEST_LIGHTS_COLORS[i][0], CLOSEST_LIGHTS_COLORS[i][1], CLOSEST_LIGHTS_COLORS[i][2]);

			float dist = Distance(backEnd.refdef.vieworg, CLOSEST_LIGHTS_POSITIONS[i]);
			if (dist > maxDist)
			{
				maxDist = dist;
			}
		}
	}

	qboolean haveConeAngles = qfalse;

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		if (CLOSEST_LIGHTS_CONEANGLES[i] != 0.0)
		{
			haveConeAngles = qtrue;
			break;
		}
	}

	GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	if (MATERIAL_SPECULAR_CHANGED)
	{// Only update as needed..
		GLSL_SetUniformFloatxX(shader, UNIFORM_MATERIAL_SPECULARS, MATERIAL_SPECULAR_STRENGTHS, MATERIAL_LAST);
		GLSL_SetUniformFloatxX(shader, UNIFORM_MATERIAL_REFLECTIVENESS, MATERIAL_SPECULAR_REFLECTIVENESS, MATERIAL_LAST);
	}

	GLSL_SetUniformInt(shader, UNIFORM_LIGHTCOUNT, NUM_LIGHTS);
	GLSL_SetUniformVec3xX(shader, UNIFORM_LIGHTPOSITIONS2, CLOSEST_LIGHTS_POSITIONS, NUM_LIGHTS);
	GLSL_SetUniformVec3xX(shader, UNIFORM_LIGHTCOLORS, CLOSEST_LIGHTS_COLORS, NUM_LIGHTS);
	GLSL_SetUniformFloatxX(shader, UNIFORM_LIGHTDISTANCES, CLOSEST_LIGHTS_DISTANCES, NUM_LIGHTS);
	//GLSL_SetUniformFloatxX(shader, UNIFORM_LIGHTHEIGHTSCALES, CLOSEST_LIGHTS_HEIGHTSCALES, NUM_LIGHTS);
	if (haveConeAngles)
	{
		GLSL_SetUniformFloatxX(shader, UNIFORM_LIGHT_CONEANGLES, CLOSEST_LIGHTS_CONEANGLES, NUM_LIGHTS);
		GLSL_SetUniformVec3xX(shader, UNIFORM_LIGHT_CONEDIRECTIONS, CLOSEST_LIGHTS_CONEDIRECTIONS, NUM_LIGHTS);
	}
	GLSL_SetUniformFloat(shader, UNIFORM_LIGHT_MAX_DISTANCE, (NUM_LIGHTS >= r_maxDeferredLights->integer / 2) ? maxDist : 8192.0);

	GLSL_SetUniformVec3(shader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg);


	vec3_t out;
	float dist = 4096.0;//backEnd.viewParms.zFar / 1.75;
 	VectorMA( backEnd.refdef.vieworg, dist, backEnd.refdef.sunDir, out );
	GLSL_SetUniformVec4(shader, UNIFORM_PRIMARYLIGHTORIGIN,  out);

	//GLSL_SetUniformVec4(shader, UNIFORM_LOCAL2,  backEnd.refdef.sunDir);
	GLSL_SetUniformVec3(shader, UNIFORM_PRIMARYLIGHTCOLOR,   backEnd.refdef.sunCol);

	float useAO = 0.0;
	if (r_ao->integer >= 3 && AO_ENABLED && AO_BLUR)
		useAO = 6.0;// r_ao->integer;
	else if (r_ao->integer >= 3 && AO_ENABLED && StringContainsWord(currentMapName, "mp/")) // Allow mp/ maps to always use blurred AO, if the cvar is set high...
		useAO = 6.0;// r_ao->integer;
	else if (r_ao->integer >= 2 && AO_ENABLED) 
		useAO = 2.0;
	else if (r_ao->integer && AO_ENABLED) 
		useAO = 1.0;
	else 
		useAO = 0.0;

	vec4_t local1;
	VectorSet4(local1, r_blinnPhong->value, SUN_PHONG_SCALE, useAO, r_ssdm->integer ? 1.0 : 0.0);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL1, local1);

	qboolean shadowsEnabled = qfalse;

	if (!(backEnd.refdef.rdflags & RDF_NOWORLDMODEL)
		&& r_sunlightMode->integer >= 2
		&& tr.screenShadowFbo
		&& SHADOWS_ENABLED
		&& RB_NightScale() < 1.0 // Can ignore rendering shadows at night...
		&& r_deferredLighting->integer)
		shadowsEnabled = qtrue;

	vec4_t local2;
	VectorSet4(local2, 0.0 /* UNUSED */, shadowsEnabled ? 1.0 : 0.0, SHADOW_MINBRIGHT, SHADOW_MAXBRIGHT);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL2,  local2);

	vec4_t local3;
	VectorSet4(local3, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value, r_testshaderValue4->value);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL3, local3);

	float dayNightGlowFactor = mix(MAP_EMISSIVE_COLOR_SCALE, MAP_EMISSIVE_COLOR_SCALE_NIGHT, RB_NightScale());

	vec4_t local4;
	VectorSet4(local4, haveConeAngles ? 1.0 : 0.0, 0.0 /* UNUSED */, 0.0 /* UNUSED */, dayNightGlowFactor);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL4, local4);

	vec4_t local5;
	local5[0] = mix(MAP_AMBIENT_CSB[0], MAP_AMBIENT_CSB_NIGHT[0], RB_NightScale());
	local5[1] = mix(MAP_AMBIENT_CSB[1], MAP_AMBIENT_CSB_NIGHT[1], RB_NightScale());
	local5[2] = mix(MAP_AMBIENT_CSB[2], MAP_AMBIENT_CSB_NIGHT[2], RB_NightScale());
	local5[3] = r_truehdr->integer ? 1.0 : 0.0;
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL5, local5);

	vec4_t local6;
	VectorSet4(local6, AO_MINBRIGHT, AO_MULTBRIGHT, r_vibrancy->value, DAY_NIGHT_CYCLE_ENABLED ? RB_NightScale() : 0.0);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL6, local6);

	vec4_t local7;
	VectorSet4(local7, cubeMapNum >= 0 ? 1.0 : 0.0, r_cubemapCullRange->value, 0.0, r_skyLightContribution->value*SKY_LIGHTING_SCALE);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL7, local7);

	vec4_t local8;
	VectorSet4(local8, MAP_REFLECTION_ENABLED ? 1.0 : 0.0, MAP_HDR_MIN, MAP_HDR_MAX, MAP_INFO_PLAYABLE_MAXS[2]);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL8, local8);

	vec4_t local9;
	VectorSet4(local9, PROCEDURAL_SNOW_HEIGHT_CURVE * 0.1, MAP_USE_PALETTE_ON_SKY ? 1.0 : 0.0, PROCEDURAL_SNOW_ENABLED ? 1.0 : 0.0, PROCEDURAL_SNOW_LOWEST_ELEVATION);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL9, local9);

	vec4_t local10;
	VectorSet4(local10, PROCEDURAL_SNOW_LUMINOSITY_CURVE, PROCEDURAL_SNOW_BRIGHTNESS, 0.0 /* UNUSED */, 0.0 /* UNUSED */);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL10, local10);


	extern qboolean		PROCEDURAL_CLOUDS_LAYER;
	extern qboolean		PROCEDURAL_CLOUDS_ENABLED;
	extern qboolean		PROCEDURAL_CLOUDS_LAYER;
	extern float		PROCEDURAL_CLOUDS_CLOUDSCALE;
	extern float		PROCEDURAL_CLOUDS_SPEED;
	extern float		PROCEDURAL_CLOUDS_DARK;
	extern float		PROCEDURAL_CLOUDS_LIGHT;
	extern float		PROCEDURAL_CLOUDS_CLOUDCOVER;
	extern float		PROCEDURAL_CLOUDS_CLOUDALPHA;
	extern float		PROCEDURAL_CLOUDS_SKYTINT;

	vec4_t vector;
	VectorSet4(vector, PROCEDURAL_CLOUDS_LAYER ? 1.0 : 0.0, PROCEDURAL_CLOUDS_CLOUDSCALE, PROCEDURAL_CLOUDS_SPEED, PROCEDURAL_CLOUDS_DARK);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL11, vector);

	VectorSet4(vector, PROCEDURAL_CLOUDS_LIGHT, PROCEDURAL_CLOUDS_CLOUDCOVER, PROCEDURAL_CLOUDS_CLOUDALPHA, PROCEDURAL_CLOUDS_SKYTINT);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL12, vector);

	GLSL_SetUniformFloat(shader, UNIFORM_TIME, backEnd.refdef.floatTime);
	
	{
		vec4_t loc;
		VectorSet4(loc, MAP_INFO_MINS[0], MAP_INFO_MINS[1], MAP_INFO_MINS[2], WATER_ENABLED ? 1.0 : 0.0);
		GLSL_SetUniformVec4(shader, UNIFORM_MINS, loc);

		VectorSet4(loc, MAP_INFO_MAXS[0], MAP_INFO_MAXS[1], MAP_INFO_MAXS[2], 0.0);
		GLSL_SetUniformVec4(shader, UNIFORM_MAXS, loc);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
	}

	/*{
		vec4_t viewInfo;
		float zmax = 4096.0;// 2048.0;
		//float zmax = backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, backEnd.viewParms.fovX);
		GLSL_SetUniformVec4(shader, UNIFORM_VIEWINFO, viewInfo);
	}*/
	
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, shader, color, 0);
}

extern float DISPLACEMENT_MAPPING_STRENGTH;

void RB_SSDM_Generate(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	shaderProgram_t *shader = &tr.ssdmGenerateShader[r_ssdm->integer >= 2 ? 1 : 0]; // r_ssdm 1 = fast. r_ssdm 2 = quality.

	GLSL_BindProgram(shader);

	GLSL_SetUniformInt(shader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImage512, TB_LIGHTMAP);

	GLSL_SetUniformInt(shader, UNIFORM_POSITIONMAP, TB_POSITIONMAP);
	GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

	GLSL_SetUniformInt(shader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.renderNormalImage, TB_NORMALMAP);

	GLSL_SetUniformInt(shader, UNIFORM_GLOWMAP, TB_GLOWMAP);
	
	/*if (r_anamorphic->integer)
	{
		GL_BindToTMU(tr.anamorphicRenderFBOImage, TB_GLOWMAP);
	}
	else if (r_bloom->integer)
	{
		GL_BindToTMU(tr.bloomRenderFBOImage[0], TB_GLOWMAP);
	}
	else*/
	{
		GL_BindToTMU(tr.glowFboScaled[0]->colorImage[0], TB_GLOWMAP);
	}

	GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformVec3(shader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg);

	vec4_t local1;
	VectorSet4(local1, DISPLACEMENT_MAPPING_STRENGTH, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value);
	GLSL_SetUniformVec4(shader, UNIFORM_LOCAL1, local1);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;
		//float zmax = 2048.0;
		//float zmax = backEnd.viewParms.zFar;
		float zmax = 512.0;// r_testvalue0->value;

		//ri->Printf(PRINT_ALL, "zFar is %f.\n", zmax);
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 512.0/*r_testvalue2->value > 0.0 ? r_testvalue2->value : backEnd.viewParms.zFar*/);
		GLSL_SetUniformVec4(shader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec3_t out;
		float dist = 4096.0;//backEnd.viewParms.zFar / 1.75;
		VectorMA(backEnd.refdef.vieworg, dist, backEnd.refdef.sunDir, out);
		GLSL_SetUniformVec4(shader, UNIFORM_PRIMARYLIGHTORIGIN, out);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, tr.ssdmFbo, ldrBox, shader, color, 0);
}

void RB_SSDM(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.ssdmShader);

	GLSL_SetUniformInt(&tr.ssdmShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.ssdmShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImage512, TB_LIGHTMAP);

	GLSL_SetUniformInt(&tr.ssdmShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.renderNormalImage, TB_NORMALMAP);

	GLSL_SetUniformInt(&tr.ssdmShader, UNIFORM_ROADMAP, TB_ROADMAP);
	GL_BindToTMU(tr.ssdmImage, TB_ROADMAP);

	GLSL_SetUniformInt(&tr.ssdmShader, UNIFORM_POSITIONMAP, TB_POSITIONMAP);
	GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

	GLSL_SetUniformMatrix16(&tr.ssdmShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformVec3(&tr.ssdmShader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg);

	vec4_t local1;
	VectorSet4(local1, DISPLACEMENT_MAPPING_STRENGTH, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value);
	GLSL_SetUniformVec4(&tr.ssdmShader, UNIFORM_LOCAL1, local1);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.ssdmShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;
		//float zmax = 2048.0;
		//float zmax = backEnd.viewParms.zFar;
		float zmax = 512.0;// r_testvalue0->value;

		//ri->Printf(PRINT_ALL, "zFar is %f.\n", zmax);
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 512.0/*r_testvalue2->value > 0.0 ? r_testvalue2->value : backEnd.viewParms.zFar*/);
		GLSL_SetUniformVec4(&tr.ssdmShader, UNIFORM_VIEWINFO, viewInfo);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.ssdmShader, color, 0);
}

#if 0
void RB_ScreenSpaceReflections(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.ssrShader);

	GLSL_SetUniformInt(&tr.ssrShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.ssrShader, UNIFORM_GLOWMAP, TB_GLOWMAP);

	GL_BindToTMU(tr.anamorphicRenderFBOImage, TB_GLOWMAP);

	GLSL_SetUniformInt(&tr.ssrShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImageZfar, TB_LIGHTMAP);


	GLSL_SetUniformMatrix16(&tr.ssrShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	vec4_t local1;
	VectorSet4(local1, r_ssr->integer, r_sse->integer, r_ssrStrength->value, r_sseStrength->value);
	GLSL_SetUniformVec4(&tr.ssrShader, UNIFORM_LOCAL1, local1);

	vec4_t local3;
	VectorSet4(local3, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value, r_testshaderValue4->value);
	GLSL_SetUniformVec4(&tr.ssrShader, UNIFORM_LOCAL3, local3);

	{
		vec2_t screensize;
		screensize[0] = tr.anamorphicRenderFBOImage->width;// glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = tr.anamorphicRenderFBOImage->height;// glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.ssrShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;
		//float zmax = 2048.0;
		float zmax = backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, backEnd.viewParms.fovX);
		GLSL_SetUniformVec4(&tr.ssrShader, UNIFORM_VIEWINFO, viewInfo);
		//ri->Printf(PRINT_ALL, "fov %f\n", backEnd.viewParms.fovX);
	}

	//FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.ssrShader, color, 0);

	FBO_Blit(hdrFbo, NULL, NULL, tr.genericFbo2, NULL, &tr.ssrShader, color, 0);

	// Combine render and hbao...
	GLSL_BindProgram(&tr.ssrCombineShader);

	GLSL_SetUniformMatrix16(&tr.ssrCombineShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	GLSL_SetUniformMatrix16(&tr.ssrCombineShader, UNIFORM_MODELMATRIX, backEnd.ori.modelMatrix);

	GLSL_SetUniformInt(&tr.ssrCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.ssrCombineShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.genericFBO2Image, TB_NORMALMAP);

	vec2_t screensize;
	screensize[0] = tr.genericFBO2Image->width;// glConfig.vidWidth * r_superSampleMultiplier->value;
	screensize[1] = tr.genericFBO2Image->height;// glConfig.vidHeight * r_superSampleMultiplier->value;
	GLSL_SetUniformVec2(&tr.ssrCombineShader, UNIFORM_DIMENSIONS, screensize);

	FBO_Blit(hdrFbo, NULL, NULL, ldrFbo, NULL, &tr.ssrCombineShader, color, 0);
}
#endif

void RB_ShowNormals(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.showNormalsShader);

	GLSL_SetUniformInt(&tr.showNormalsShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.showNormalsShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.renderNormalImage, TB_NORMALMAP);

	if (r_normalMappingReal->integer)
	{
		GLSL_SetUniformInt(&tr.showNormalsShader, UNIFORM_OVERLAYMAP, TB_OVERLAYMAP);
		GL_BindToTMU(tr.renderNormalDetailedImage, TB_OVERLAYMAP);
	}

	GLSL_SetUniformMatrix16(&tr.showNormalsShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	vec4_t settings;
	VectorSet4(settings, r_shownormals->integer, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value);
	GLSL_SetUniformVec4(&tr.showNormalsShader, UNIFORM_SETTINGS0, settings);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.showNormalsShader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.showNormalsShader, color, 0);
}

void RB_ShowDepth(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.showDepthShader);

	GLSL_SetUniformInt(&tr.showDepthShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.showDepthShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImageZfar, TB_LIGHTMAP);

	GLSL_SetUniformMatrix16(&tr.showDepthShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	{
		vec4_t viewInfo;
		//float zmax = 2048.0;
		float zmax = backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, backEnd.viewParms.fovX);
		GLSL_SetUniformVec4(&tr.showDepthShader, UNIFORM_VIEWINFO, viewInfo);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.showDepthShader, color, 0);
}

void RB_TestShader(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox, int pass_num)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	//RB_AddGlowShaderLights();

	GLSL_BindProgram(&tr.testshaderShader);
	
	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_POSITIONMAP, TB_POSITIONMAP);
	GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.renderNormalImage, TB_NORMALMAP);
	
	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImageZfar, TB_LIGHTMAP);

	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_DELUXEMAP, TB_DELUXEMAP);
	GL_BindToTMU(tr.paletteImage, TB_DELUXEMAP);

	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_GLOWMAP, TB_GLOWMAP);
	GL_BindToTMU(tr.glowFboScaled[0]->colorImage[0]/*tr.glowImageScaled[5]*/, TB_GLOWMAP);

	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_SPECULARMAP, TB_SPECULARMAP);
	GL_BindToTMU(tr.random2KImage[0], TB_SPECULARMAP);


	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_LIGHTCOUNT, NUM_CLOSE_LIGHTS);
	GLSL_SetUniformVec3xX(&tr.testshaderShader, UNIFORM_LIGHTPOSITIONS2, CLOSEST_LIGHTS_POSITIONS, MAX_DEFERRED_LIGHTS);
	GLSL_SetUniformVec3xX(&tr.testshaderShader, UNIFORM_LIGHTCOLORS, CLOSEST_LIGHTS_COLORS, MAX_DEFERRED_LIGHTS);
	GLSL_SetUniformFloatxX(&tr.testshaderShader, UNIFORM_LIGHTDISTANCES, CLOSEST_LIGHTS_DISTANCES, MAX_DEFERRED_LIGHTS);

	GLSL_SetUniformVec3(&tr.testshaderShader, UNIFORM_VIEWORIGIN,  backEnd.refdef.vieworg);
	GLSL_SetUniformFloat(&tr.testshaderShader, UNIFORM_TIME, backEnd.refdef.floatTime);

	GLSL_SetUniformMatrix16(&tr.testshaderShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	vec3_t out;
	float dist = 4096.0;//backEnd.viewParms.zFar / 1.75;
 	VectorMA( backEnd.refdef.vieworg, dist, backEnd.refdef.sunDir, out );
	GLSL_SetUniformVec4(&tr.testshaderShader, UNIFORM_PRIMARYLIGHTORIGIN,  out);

	//GLSL_SetUniformVec4(&tr.testshaderShader, UNIFORM_LOCAL2,  backEnd.refdef.sunDir);
	GLSL_SetUniformVec3(&tr.testshaderShader, UNIFORM_PRIMARYLIGHTCOLOR,   backEnd.refdef.sunCol);

	vec4_t local2;
	VectorSet4(local2, r_blinnPhong->value, 0.0, 0.0, 0.0);
	GLSL_SetUniformVec4(&tr.testshaderShader, UNIFORM_LOCAL2,  local2);

	{
		vec4_t viewInfo;
		//float zmax = 2048.0;
		float zmax = backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(&tr.testshaderShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec4_t loc;
		VectorSet4(loc, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value);
		GLSL_SetUniformVec4(&tr.testshaderShader, UNIFORM_LOCAL0, loc);
	}
	
	{
		vec4_t loc;
		VectorSet4(loc, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value, r_testshaderValue4->value);
		GLSL_SetUniformVec4(&tr.testshaderShader, UNIFORM_LOCAL1, loc);
	}


	{
		vec4_t loc;
		VectorSet4(loc, MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], (float)SUN_VISIBLE);
		GLSL_SetUniformVec4(&tr.testshaderShader, UNIFORM_MAPINFO, loc);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.testshaderShader, UNIFORM_DIMENSIONS, screensize);
	}
	
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.testshaderShader, color, 0);
}

void RB_ColorCorrection(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.colorCorrectionShader);
	
	GLSL_SetUniformInt(&tr.colorCorrectionShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.colorCorrectionShader, UNIFORM_DELUXEMAP, TB_DELUXEMAP);
	GL_BindToTMU(tr.paletteImage, TB_DELUXEMAP);

#if 0
	GLSL_SetUniformInt(&tr.colorCorrectionShader, UNIFORM_GLOWMAP, TB_GLOWMAP);
	//GL_BindToTMU(tr.glowImageScaled[5], TB_GLOWMAP);

	// Use the most blurred version of glow...
	if (r_anamorphic->integer)
	{
		GL_BindToTMU(tr.anamorphicRenderFBOImage, TB_GLOWMAP);
	}
	else if (r_bloom->integer)
	{
		GL_BindToTMU(tr.bloomRenderFBOImage[0], TB_GLOWMAP);
	}
	else
	{
		GL_BindToTMU(tr.glowFboScaled[0]->colorImage[0], TB_GLOWMAP);
	}
#endif

	GLSL_SetUniformMatrix16(&tr.colorCorrectionShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.colorCorrectionShader, color, 0);
}


void RB_FogPostShader(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.fogPostShader);
	
	GLSL_SetUniformInt(&tr.fogPostShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.fogPostShader, UNIFORM_POSITIONMAP, TB_POSITIONMAP);
	GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

	GLSL_SetUniformInt(&tr.fogPostShader, UNIFORM_WATERPOSITIONMAP, TB_WATERPOSITIONMAP);
	GL_BindToTMU(tr.waterPositionMapImage, TB_WATERPOSITIONMAP);

	//GLSL_SetUniformInt(&tr.fogPostShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	//GL_BindToTMU(tr.renderNormalImage, TB_NORMALMAP);

	GLSL_SetUniformInt(&tr.fogPostShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(/*tr.linearDepthImageZfar*/tr.linearDepthImage4096, TB_LIGHTMAP);


	GLSL_SetUniformVec3(&tr.fogPostShader, UNIFORM_VIEWORIGIN,  backEnd.refdef.vieworg);
	GLSL_SetUniformFloat(&tr.fogPostShader, UNIFORM_TIME, backEnd.refdef.floatTime);

	
	vec3_t out;
	float dist = 4096.0;//backEnd.viewParms.zFar / 1.75;
 	VectorMA( backEnd.refdef.vieworg, dist, backEnd.refdef.sunDir, out );
	GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_PRIMARYLIGHTORIGIN,  out);
	GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL2,  backEnd.refdef.sunDir);

	GLSL_SetUniformVec3(&tr.fogPostShader, UNIFORM_PRIMARYLIGHTCOLOR,   backEnd.refdef.sunCol);

	GLSL_SetUniformMatrix16(&tr.fogPostShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	
	GLSL_SetUniformFloat(&tr.fogPostShader, UNIFORM_TIME, backEnd.refdef.floatTime * 0.3/*r_testvalue3->value*/);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.fogPostShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;
		float zmax = 4096.0;// 2048.0;
		//float zmax = backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec4_t loc;
		VectorSet4(loc, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL0, loc);
	}
	
	{
		vec4_t loc;
		VectorSet4(loc, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value, r_testshaderValue4->value);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL1, loc);
	}

	{
		vec4_t loc;
		VectorSet4(loc, FOG_WORLD_COLOR[0], FOG_WORLD_COLOR[1], FOG_WORLD_COLOR[2], FOG_WORLD_ENABLE ? 1.0 : 0.0);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL2, loc);
	}

	{
		vec4_t loc;
		VectorSet4(loc, FOG_WORLD_COLOR_SUN[0], FOG_WORLD_COLOR_SUN[1], FOG_WORLD_COLOR_SUN[2], FOG_WORLD_ALPHA);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL3, loc);
	}

	{
		vec4_t loc;
		VectorSet4(loc, FOG_WORLD_CLOUDINESS, FOG_LAYER_ENABLE ? 1.0 : 0.0, FOG_LAYER_SUN_PENETRATION, FOG_LAYER_ALTITUDE_BOTTOM);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL4, loc);
	}

	{
		vec4_t loc;
		VectorSet4(loc, FOG_LAYER_COLOR[0], FOG_LAYER_COLOR[1], FOG_LAYER_COLOR[2], FOG_LAYER_ALPHA);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL5, loc);
	}

	{
		vec4_t loc;
		VectorSet4(loc, MAP_INFO_MAXSIZE, FOG_WORLD_WIND, FOG_LAYER_CLOUDINESS, FOG_LAYER_WIND);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL6, loc);
	}

	{
		vec4_t local7;
		VectorSet4(local7, DAY_NIGHT_CYCLE_ENABLED ? RB_NightScale() : 0.0, FOG_LAYER_ALTITUDE_TOP, FOG_LAYER_ALTITUDE_FADE, WATER_ENABLED);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL7, local7);
	}

	GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL8, backEnd.refdef.sunCol);

	GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL9, FOG_LAYER_BBOX);

	{
		vec4_t local10;
		VectorSet4(local10, MAP_INFO_MINS[0], MAP_INFO_MINS[1], MAP_INFO_MINS[2], FOG_WORLD_FADE_ALTITUDE);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL10, local10);
	}

	{
		vec4_t local11;
		VectorSet4(local11, MAP_INFO_MAXS[0], MAP_INFO_MAXS[1], MAP_INFO_MAXS[2], FOG_LINEAR_ENABLE);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL11, local11);
	}

	{
		vec4_t local12;
		VectorSet4(local12, FOG_LINEAR_COLOR[0], FOG_LINEAR_COLOR[1], FOG_LINEAR_COLOR[2], FOG_LINEAR_ALPHA);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_LOCAL12, local12);
	}

	{
		vec4_t loc;
		VectorSet4(loc, MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], (float)SUN_VISIBLE);
		GLSL_SetUniformVec4(&tr.fogPostShader, UNIFORM_MAPINFO, loc);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.fogPostShader, color, 0);
}

void RB_FastBlur(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	shaderProgram_t *shader = &tr.fastBlurShader;

	GLSL_BindProgram(shader);

	GLSL_SetUniformInt(shader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
	
	GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.linearDepthImage4096, TB_LIGHTMAP);

	GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	{
		vec2_t screensize;
		screensize[0] = tr.screenShadowBlurTempFbo->width;
		screensize[1] = tr.screenShadowBlurTempFbo->height;

		GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;
		float zmax = tr.occlusionOriginalZfar;// backEnd.viewParms.zFar; //2048.0;//3072.0;//backEnd.viewParms.zFar;
		//float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		//float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(shader, UNIFORM_VIEWINFO, viewInfo);
	}

	/*vec4i_t dstBox;
	dstBox[0] = backEnd.viewParms.viewportX;
	dstBox[1] = backEnd.viewParms.viewportY;
	dstBox[2] = tr.screenShadowBlurTempFbo->width;
	dstBox[3] = tr.screenShadowBlurTempFbo->height;
	*/

	{
		vec4_t loc;
		VectorSet4(loc, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value);
		GLSL_SetUniformVec4(shader, UNIFORM_LOCAL0, loc);
	}

	GLSL_SetUniformFloatxX(shader, UNIFORM_SHADOWZFAR, tr.refdef.sunShadowCascadeZfar, 5);

#if 0
	{
		vec4_t loc;
		VectorSet4(loc, (r_shadowBlurWidth->value > 0) ? r_shadowBlurWidth->value : 1.0, (r_shadowBlurStep->value > 0) ? r_shadowBlurStep->value : 1.0, 0.0, 0.0);
		GLSL_SetUniformVec4(shader, UNIFORM_SETTINGS0, loc);
	}

	FBO_Blit(hdrFbo, NULL, NULL, ldrFbo, NULL, shader, color, 0);
#else
	float direction = 0.0;

	//for (int pass = 0; pass < r_testvalue0->integer; pass++)
	int pass = 0;
	{
		{// Blur X...
			direction = 0.0;

			{
				vec4_t loc;
				VectorSet4(loc, (r_shadowBlurWidth->value > 0) ? r_shadowBlurWidth->value : 1.0, (r_shadowBlurStep->value > 0) ? r_shadowBlurStep->value : 1.0, direction, 0.0);
				GLSL_SetUniformVec4(shader, UNIFORM_SETTINGS0, loc);
			}

			FBO_Blit(pass == 0 ? tr.screenShadowFbo : tr.screenShadowBlurFbo, NULL, NULL, tr.screenShadowBlurTempFbo, NULL, shader, color, 0);
		}

		{// Blur Y...
			direction = 1.0;

			{
				vec4_t loc;
				VectorSet4(loc, (r_shadowBlurWidth->value > 0) ? r_shadowBlurWidth->value : 1.0, (r_shadowBlurStep->value > 0) ? r_shadowBlurStep->value : 1.0, direction, 0.0);
				GLSL_SetUniformVec4(shader, UNIFORM_SETTINGS0, loc);
			}

			FBO_Blit(tr.screenShadowBlurTempFbo, NULL, NULL, tr.screenShadowBlurFbo, NULL, shader, color, 0);
		}
	}
#endif
}

void RB_DistanceBlur(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox, int direction)
{
	if (r_distanceBlur->integer < 2 || r_distanceBlur->integer >= 5)
	{// Fast blur (original)
		vec4_t color;

		// bloom
		color[0] =
			color[1] =
			color[2] = pow(2, r_cameraExposure->value);
		color[3] = 1.0f;

		GLSL_BindProgram(&tr.distanceBlurShader[0]);

		GLSL_SetUniformInt(&tr.distanceBlurShader[0], UNIFORM_LEVELSMAP, TB_LEVELSMAP);
		GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
		GLSL_SetUniformInt(&tr.distanceBlurShader[0], UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
		GL_BindToTMU(tr.linearDepthImage2048, TB_LIGHTMAP);
		GLSL_SetUniformInt(&tr.distanceBlurShader[0], UNIFORM_POSITIONMAP, TB_POSITIONMAP);
		GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

		GLSL_SetUniformMatrix16(&tr.distanceBlurShader[0], UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

		{
			vec2_t screensize;
			screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
			screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

			GLSL_SetUniformVec2(&tr.distanceBlurShader[0], UNIFORM_DIMENSIONS, screensize);
		}

		{
			vec4_t viewInfo;
			float zmax = 2048.0;
			//float zmax = backEnd.viewParms.zFar;
			float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
			float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
			float zmin = r_znear->value;
			VectorSet4(viewInfo, zmin, zmax, zmax / zmin, MAP_WATER_LEVEL);
			GLSL_SetUniformVec4(&tr.distanceBlurShader[0], UNIFORM_VIEWINFO, viewInfo);
		}

		{
			vec4_t info;

			info[0] = r_distanceBlur->value;
			info[1] = r_dynamicGlow->value;
			info[2] = 0.0;
			info[3] = direction;

			VectorSet4(info, info[0], info[1], info[2], info[3]);

			GLSL_SetUniformVec4(&tr.distanceBlurShader[0], UNIFORM_LOCAL0, info);
		}

		{
			vec4_t loc;
			VectorSet4(loc, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value);
			GLSL_SetUniformVec4(&tr.distanceBlurShader[0], UNIFORM_LOCAL1, loc);
		}

		FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.distanceBlurShader[0], color, 0);
	}
	else
	{// New matso style blur...
		shaderProgram_t *shader = &tr.distanceBlurShader[r_distanceBlur->integer-1];
		vec4_t color;

		// bloom
		color[0] =
			color[1] =
			color[2] = pow(2, r_cameraExposure->value);
		color[3] = 1.0f;

		GLSL_BindProgram(shader);

		GLSL_SetUniformInt(shader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
		GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
		GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
		GL_BindToTMU(tr.linearDepthImage2048, TB_LIGHTMAP);
		GLSL_SetUniformInt(shader, UNIFORM_GLOWMAP, TB_GLOWMAP);
		//GL_BindToTMU(tr.glowFboScaled[0]->colorImage[0], TB_GLOWMAP);
		// Use the most blurred version of glow...
		if (r_anamorphic->integer)
		{
			GL_BindToTMU(tr.anamorphicRenderFBOImage, TB_GLOWMAP);
		}
		else if (r_bloom->integer)
		{
			GL_BindToTMU(tr.bloomRenderFBOImage[0], TB_GLOWMAP);
		}
		else
		{
			GL_BindToTMU(tr.glowFboScaled[0]->colorImage[0], TB_GLOWMAP);
		}
		//GLSL_SetUniformInt(shader, UNIFORM_POSITIONMAP, TB_POSITIONMAP);
		//GL_BindToTMU(tr.renderPositionMapImage, TB_POSITIONMAP);

		GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

		{
			vec2_t screensize;
			screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
			screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

			GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
		}

		{
			vec4_t info;

			info[0] = r_distanceBlur->value;
			info[1] = r_dynamicGlow->value;
			info[2] = 0.0;
			info[3] = direction;

			VectorSet4(info, info[0], info[1], info[2], info[3]);

			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL0, info);
		}

		{
			vec4_t loc;
			VectorSet4(loc, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value, r_testvalue3->value);
			GLSL_SetUniformVec4(shader, UNIFORM_LOCAL1, loc);
		}

		{
			vec4_t viewInfo;
			float zmax = 2048.0;//3072.0;//backEnd.viewParms.zFar;
			float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
			float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
			float zmin = r_znear->value;
			VectorSet4(viewInfo, zmin, zmax, zmax / zmin, MAP_WATER_LEVEL);
			GLSL_SetUniformVec4(shader, UNIFORM_VIEWINFO, viewInfo);
		}

		FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, shader, color, 0);
	}
}

void RB_Underwater(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.underwaterShader);

	GLSL_SetUniformInt(&tr.underwaterShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	GLSL_SetUniformMatrix16(&tr.underwaterShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformFloat(&tr.underwaterShader, UNIFORM_TIME, backEnd.refdef.floatTime*5.0/*tr.refdef.floatTime*/);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.underwaterShader, UNIFORM_DIMENSIONS, screensize);
	}
	
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.underwaterShader, color, 0);
}

void RB_FXAA(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.fxaaShader);

	GLSL_SetUniformInt(&tr.fxaaShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	GLSL_SetUniformMatrix16(&tr.fxaaShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.fxaaShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t local0;
		VectorSet4(local0, r_fxaaScanMod->value, 0.0, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.fxaaShader, UNIFORM_LOCAL0, local0);
	}
	
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.fxaaShader, color, 0);
}

void RB_TXAA(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	GLSL_BindProgram(&tr.txaaShader);

	GLSL_SetUniformInt(&tr.txaaShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.txaaShader, UNIFORM_GLOWMAP, TB_GLOWMAP);
	GL_BindToTMU(tr.txaaPreviousImage, TB_GLOWMAP);

	GLSL_SetUniformMatrix16(&tr.txaaShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.txaaShader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.fxaaShader, colorWhite, 0);
	
	/*
	// Clear the txaaPrevious Image/FBO.
	FBO_t *oldFbo = glState.currentFBO;
	vec4_t oldColorMask;
	VectorCopy4(backEnd.colorMask, oldColorMask);
	FBO_Bind(tr.txaaPreviousFBO);
	qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	qglClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	qglClear(GL_COLOR_BUFFER_BIT);

	// Return to normal FBO...
	FBO_Bind(oldFbo);
	qglColorMask(oldColorMask[0], oldColorMask[1], oldColorMask[2], oldColorMask[3]);
	*/

	// Copy the output to previous image for usage next frame...
	FBO_FastBlit(ldrFbo, ldrBox, tr.txaaPreviousFBO, ldrBox, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void RB_LinearizeDepth(void)
{
	GLSL_BindProgram(&tr.linearizeDepthShader);

	GLSL_SetUniformInt(&tr.linearizeDepthShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

	GLSL_SetUniformMatrix16(&tr.linearizeDepthShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	{
		vec4_t viewInfo;
		float zmax = backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, backEnd.viewParms.fovX);
		GLSL_SetUniformVec4(&tr.linearizeDepthShader, UNIFORM_VIEWINFO, viewInfo);
	}

	FBO_Blit(tr.renderFbo, NULL, NULL, tr.linearizeDepthFbo, NULL, &tr.linearizeDepthShader, colorWhite, 0);

	/*
	if (r_testvalue0->integer)
	{
		int texNum = glState.currenttextures[glState.currenttmu];

		// Make mipmaps for some, so we can with any luck speed up some stuff...
		qglActiveTextureARB(GL_TEXTURE0_ARB);
		qglBindTexture(GL_TEXTURE_2D, tr.linearDepthImage4096->texnum);
		qglGenerateMipmap(GL_TEXTURE_2D);

		qglBindTexture(GL_TEXTURE_2D, tr.linearDepthImageZfar->texnum);
		qglGenerateMipmap(GL_TEXTURE_2D);

		qglBindTexture(GL_TEXTURE_2D, texNum);
	}
	*/
}
