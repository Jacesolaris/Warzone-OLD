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
				srcBox[0] = s0 * glConfig.vidWidth;
				srcBox[1] = t0 * glConfig.vidHeight;
				srcBox[2] = (s1 - s0) * glConfig.vidWidth;
				srcBox[3] = (t1 - t0) * glConfig.vidHeight;
			}
			
			FBO_Blit(srcFbo, srcBox, texScale, dstFbo, dstBox, &tr.textureColorShader, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );

			scale *= mul;
			--passes;
		}
	}
}


static qboolean RB_UpdateSunFlareVis(void)
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

		dist = backEnd.viewParms.zFar / 1.75;		// div sqrt(3)

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
			rayBox[0] = srcBox[0] * tr.sunRaysFbo->width  / glConfig.vidWidth;
			rayBox[1] = srcBox[1] * tr.sunRaysFbo->height / glConfig.vidHeight;
			rayBox[2] = srcBox[2] * tr.sunRaysFbo->width  / glConfig.vidWidth;
			rayBox[3] = srcBox[3] * tr.sunRaysFbo->height / glConfig.vidHeight;
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

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.darkexpandShader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.darkexpandShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
}

void RB_Bloom(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t	color;
	vec4i_t halfBox;
	vec2_t	texScale, texHalfScale, texDoubleScale;

	texScale[0] = texScale[1] = 1.0f;
	texHalfScale[0] = texHalfScale[1] = texScale[0] / 2.0;
	texDoubleScale[0] = texDoubleScale[1] = texScale[0] * 2.0;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	halfBox[0] = backEnd.viewParms.viewportX      * tr.bloomRenderFBOImage[0]->width  / (float)glConfig.vidWidth;
	halfBox[1] = backEnd.viewParms.viewportY      * tr.bloomRenderFBOImage[0]->height / (float)glConfig.vidHeight;
	halfBox[2] = backEnd.viewParms.viewportWidth  * tr.bloomRenderFBOImage[0]->width  / (float)glConfig.vidWidth;
	halfBox[3] = backEnd.viewParms.viewportHeight * tr.bloomRenderFBOImage[0]->height / (float)glConfig.vidHeight;

	//
	// Darken to VBO...
	//
	
	if (r_dynamicGlow->integer)
	{
		FBO_BlitFromTexture(tr.glowFboScaled[0]->colorImage[0], NULL, NULL, tr.bloomRenderFBO[0], NULL, NULL, color, 0);
	}
	else
	{
		GLSL_BindProgram(&tr.bloomDarkenShader);

		GL_BindToTMU(tr.fixedLevelsImage, TB_DIFFUSEMAP);

		{
			vec2_t screensize;
			screensize[0] = glConfig.vidWidth;
			screensize[1] = glConfig.vidHeight;

			GLSL_SetUniformVec2(&tr.bloomDarkenShader, UNIFORM_DIMENSIONS, screensize);
		}

		{
			vec4_t local0;
			VectorSet4(local0, r_bloomDarkenPower->value, 0.0, 0.0, 0.0);
			GLSL_SetUniformVec4(&tr.bloomDarkenShader, UNIFORM_LOCAL0, local0);
		}

		FBO_Blit(hdrFbo, NULL, texHalfScale, tr.bloomRenderFBO[1], NULL, &tr.bloomDarkenShader, color, 0);
		FBO_FastBlit(tr.bloomRenderFBO[1], NULL, tr.bloomRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}

	//
	// Blur the new darken'ed VBO...
	//

	for ( int i = 0; i < r_bloomPasses->integer; i++ ) 
	{
#ifdef ___BLOOM_AXIS_UNCOMBINED_SHADER___
		//
		// Bloom X axis... (to VBO 1)
		//

		GLSL_BindProgram(&tr.bloomBlurShader);

		GL_BindToTMU(tr.bloomRenderFBOImage[0], TB_DIFFUSEMAP);

		{
			vec2_t screensize;
			screensize[0] = tr.bloomRenderFBOImage[0]->width;
			screensize[1] = tr.bloomRenderFBOImage[0]->height;

			GLSL_SetUniformVec2(&tr.bloomBlurShader, UNIFORM_DIMENSIONS, screensize);
		}

		{
			vec4_t local0;
			VectorSet4(local0, 1.0, 0.0, 0.0, 0.0);
			GLSL_SetUniformVec4(&tr.bloomBlurShader, UNIFORM_LOCAL0, local0);
		}

		FBO_Blit(tr.bloomRenderFBO[0], NULL, NULL, tr.bloomRenderFBO[1], NULL, &tr.bloomBlurShader, color, 0);
		FBO_FastBlit(tr.bloomRenderFBO[1], NULL, tr.bloomRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		//
		// Bloom Y axis... (back to VBO 0)
		//

		GLSL_BindProgram(&tr.bloomBlurShader);

		GL_BindToTMU(tr.bloomRenderFBOImage[1], TB_DIFFUSEMAP);

		{
			vec2_t screensize;
			screensize[0] = tr.bloomRenderFBOImage[1]->width;
			screensize[1] = tr.bloomRenderFBOImage[1]->height;

			GLSL_SetUniformVec2(&tr.bloomBlurShader, UNIFORM_DIMENSIONS, screensize);
		}

		{
			vec4_t local0;
			VectorSet4(local0, 0.0, 1.0, 0.0, 0.0);
			GLSL_SetUniformVec4(&tr.bloomBlurShader, UNIFORM_LOCAL0, local0);
		}

		FBO_Blit(tr.bloomRenderFBO[0], NULL, NULL, tr.bloomRenderFBO[1], NULL, &tr.bloomBlurShader, color, 0);
		FBO_FastBlit(tr.bloomRenderFBO[1], NULL, tr.bloomRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
#else //___BLOOM_AXIS_UNCOMBINED_SHADER___

		//
		// Bloom X and Y axis... (to VBO 1)
		//

		GLSL_BindProgram(&tr.bloomBlurShader);

		GL_BindToTMU(tr.bloomRenderFBOImage[0], TB_DIFFUSEMAP);

		{
			vec2_t screensize;
			screensize[0] = tr.bloomRenderFBOImage[0]->width;
			screensize[1] = tr.bloomRenderFBOImage[0]->height;

			GLSL_SetUniformVec2(&tr.bloomBlurShader, UNIFORM_DIMENSIONS, screensize);
		}

		{
			vec4_t local0;
			VectorSet4(local0, 0.0, 0.0, /* bloom width */ 3.0, 0.0); // non-flicker version
			//VectorSet4(local0, 0.0, 0.0, /* bloom width */ 3.0 * ((float)irand(90,100) / 100), 0.0); // use flicker???
			GLSL_SetUniformVec4(&tr.bloomBlurShader, UNIFORM_LOCAL0, local0);
		}

		FBO_Blit(tr.bloomRenderFBO[0], NULL, NULL, tr.bloomRenderFBO[1], NULL, &tr.bloomBlurShader, color, 0);
		FBO_FastBlit(tr.bloomRenderFBO[1], NULL, tr.bloomRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);

#endif //___BLOOM_AXIS_UNCOMBINED_SHADER___
	}
	
	//
	// Copy (and upscale) the bloom image to our full screen image...
	//

	FBO_Blit(tr.bloomRenderFBO[0], NULL, texDoubleScale, tr.bloomRenderFBO[2], NULL, &tr.bloomBlurShader, color, 0);

	//
	// Combine the screen with the bloom'ed VBO...
	//
	
	
	GLSL_BindProgram(&tr.bloomCombineShader);
	
	GL_BindToTMU(tr.fixedLevelsImage, TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.bloomCombineShader, UNIFORM_DIFFUSEMAP,   TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.bloomCombineShader, UNIFORM_NORMALMAP,   TB_NORMALMAP);

	GL_BindToTMU(tr.bloomRenderFBOImage[2], TB_NORMALMAP);

	{
		vec4_t local0;
		VectorSet4(local0, r_bloomScale->value, 0.0, 0.0, 0.0);
		if (r_dynamicGlow->integer) VectorSet4(local0, 0.5 * r_bloomScale->value, 0.0, 0.0, 0.0); // Account for already added glow...
		GLSL_SetUniformVec4(&tr.bloomCombineShader, UNIFORM_LOCAL0, local0);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.bloomCombineShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);

	//
	// Render the results now...
	//

	FBO_FastBlit(ldrFbo, NULL, hdrFbo, NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}


void RB_Anamorphic(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t	color;
	vec4i_t halfBox;
	vec2_t	texScale, texHalfScale, texDoubleScale;

	texScale[0] = texScale[1] = 1.0f;
	texHalfScale[0] = texHalfScale[1] = texScale[0] / 8.0;
	texDoubleScale[0] = texDoubleScale[1] = texScale[0] * 8.0;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	halfBox[0] = backEnd.viewParms.viewportX      * tr.anamorphicRenderFBOImage[0]->width  / (float)glConfig.vidWidth;
	halfBox[1] = backEnd.viewParms.viewportY      * tr.anamorphicRenderFBOImage[0]->height / (float)glConfig.vidHeight;
	halfBox[2] = backEnd.viewParms.viewportWidth  * tr.anamorphicRenderFBOImage[0]->width  / (float)glConfig.vidWidth;
	halfBox[3] = backEnd.viewParms.viewportHeight * tr.anamorphicRenderFBOImage[0]->height / (float)glConfig.vidHeight;

	//
	// Darken to VBO...
	//
	
	if (r_dynamicGlow->integer)
	{
		FBO_BlitFromTexture(tr.glowFboScaled[0]->colorImage[0], NULL, NULL, tr.anamorphicRenderFBO[0], NULL, NULL, color, 0);
	}
	else
	{
		GLSL_BindProgram(&tr.anamorphicDarkenShader);

		GL_BindToTMU(tr.fixedLevelsImage, TB_DIFFUSEMAP);

		{
			vec2_t screensize;
			screensize[0] = glConfig.vidWidth;
			screensize[1] = glConfig.vidHeight;

			GLSL_SetUniformVec2(&tr.anamorphicDarkenShader, UNIFORM_DIMENSIONS, screensize);
		}

		{
			vec4_t local0;
			VectorSet4(local0, r_anamorphicDarkenPower->value, 0.0, 0.0, 0.0);
			GLSL_SetUniformVec4(&tr.anamorphicDarkenShader, UNIFORM_LOCAL0, local0);
		}

		FBO_Blit(hdrFbo, NULL, texHalfScale, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicDarkenShader, color, 0);
		FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}

	//
	// Blur the new darken'ed VBO...
	//

	for ( int i = 0; i < /*r_bloomPasses->integer*/ 8; i++ ) 
	{
		//
		// Bloom X axis... (to VBO 1)
		//

		//for (int width = 1; width < 12 ; width++)
		{
			GLSL_BindProgram(&tr.anamorphicBlurShader);

			GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

			{
				vec2_t screensize;
				screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
				screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

				GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
			}

			{
				vec4_t local0;
				//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
				VectorSet4(local0, 1.0, 0.0, 16.0, 0.0);
				GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
			}

			FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0);
			FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
	}
	
	//
	// Copy (and upscale) the bloom image to our full screen image...
	//

	FBO_Blit(tr.anamorphicRenderFBO[0], NULL, texDoubleScale, tr.anamorphicRenderFBO[2], NULL, &tr.anamorphicBlurShader, color, 0);

	//
	// Combine the screen with the bloom'ed VBO...
	//
	
	
	GLSL_BindProgram(&tr.anamorphicCombineShader);
	
	GL_BindToTMU(tr.fixedLevelsImage, TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.anamorphicCombineShader, UNIFORM_DIFFUSEMAP,   TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.anamorphicCombineShader, UNIFORM_NORMALMAP,   TB_NORMALMAP);

	GL_BindToTMU(tr.anamorphicRenderFBOImage[2], TB_NORMALMAP);

	{
		vec4_t local0;
		VectorSet4(local0, 0.6, 0.0, 0.0, 0.0);

		//if (r_dynamicGlow->integer) VectorSet4(local0, 1.5, 0.0, 0.0, 0.0);
		if (r_dynamicGlow->integer) VectorSet4(local0, 1.0, 0.0, 0.0, 0.0); // Account for already added glow...

		GLSL_SetUniformVec4(&tr.anamorphicCombineShader, UNIFORM_LOCAL0, local0);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.anamorphicCombineShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);

	//
	// Render the results now...
	//

	FBO_FastBlit(ldrFbo, NULL, hdrFbo, NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}


void RB_LensFlare(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.lensflareShader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.lensflareShader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.lensflareShader, color, 0);
}


void RB_MultiPost(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.multipostShader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

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
		return false;
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
	vec3_t	deltaVector, angles, deltaAngles;
	vec3_t	fromAnglesCopy;
	vec3_t	fromAngles;
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
	//SV_Trace(results, start, mins, maxs, end, passEntityNum, contentmask, eG2TraceType, useLod);
	ri->CM_BoxTrace(results, start, end, mins, maxs, 0, contentmask, 0);
	results->entityNum = results->fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
}

void RB_VolumetricDLight(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;
	int NUM_VISIBLE_LIGHTS = 0;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	if ( !backEnd.refdef.num_dlights ) {
		return;
	}

	for ( int l = 0 ; l < backEnd.refdef.num_dlights ; l++ ) 
	{
		dlight_t	*dl = &backEnd.refdef.dlights[l];
		
		float x, y, distance;
		
		distance = Distance(backEnd.refdef.vieworg, dl->origin);
		
		if (distance > 2048.0) 
		{
			dl->radius = 0.0;
			continue; // too far away...
		}

		if (!TR_InFOV( dl->origin, backEnd.refdef.vieworg ))
		{
			dl->radius = 0.0;
			continue; // not on screen...
		}

		if (!TR_WorldToScreen(dl->origin, &x, &y)) 
		{
			dl->radius = 0.0;
			continue; // not on screen...
		}

		trace_t trace;
		vec3_t mins = {-1,-1,-1}, maxs = {1,1,1};
		// //|CONTENTS_SHOTCLIP|CONTENTS_TERRAIN//(/*MASK_SOLID|*/CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_SHOTCLIP|CONTENTS_TERRAIN|CONTENTS_BODY)
		Volumetric_Trace( &trace, backEnd.refdef.vieworg, NULL, NULL, dl->origin, -1, (CONTENTS_SOLID|CONTENTS_TERRAIN) );
		//ri->Printf(PRINT_WARNING, "Trace from %f %f %f to %f %f %f.\n", backEnd.refdef.vieworg[0], backEnd.refdef.vieworg[1], backEnd.refdef.vieworg[2], dl->origin[0], dl->origin[1], dl->origin[2]);
		if (trace.fraction != 1.0 && Distance(trace.endpos, dl->origin) > 64)
		{
			dl->radius = 0.0;
			continue; // Can't see this...
		}

		NUM_VISIBLE_LIGHTS++;
	}

	for ( int l = 0 ; l < backEnd.refdef.num_dlights ; l++ ) 
	{
		dlight_t	*dl = &backEnd.refdef.dlights[l];
		
		float x, y, distance;

		if (dl->radius <= 0.0) continue; // Marked as not visible...

		distance = Distance(backEnd.refdef.vieworg, dl->origin);

		//if (distance > 2048.0) continue; // too far away...
		if (!TR_WorldToScreen(dl->origin, &x, &y)) continue; // not on screen...

		/*
		trace_t trace;
		vec3_t mins = {-1,-1,-1}, maxs = {1,1,1};
		// //|CONTENTS_SHOTCLIP|CONTENTS_TERRAIN//(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_SHOTCLIP|CONTENTS_TERRAIN|CONTENTS_BODY)
		Volumetric_Trace( &trace, backEnd.refdef.vieworg, NULL, NULL, dl->origin, -1, (CONTENTS_SOLID|CONTENTS_TERRAIN) );
		//ri->Printf(PRINT_WARNING, "Trace from %f %f %f to %f %f %f.\n", backEnd.refdef.vieworg[0], backEnd.refdef.vieworg[1], backEnd.refdef.vieworg[2], dl->origin[0], dl->origin[1], dl->origin[2]);
		if (Distance(trace.endpos, dl->origin) > 64) continue; // Can't see this...
		*/

		GLSL_BindProgram(&tr.volumelightShader);

		// Pick the specified image option if we can...
		/*if (r_ssgi->integer && r_volumelight->integer >= 5)
		{// Use SSGI Saturation image...
			GL_BindToTMU(tr.anamorphicRenderFBOImage[2], TB_DIFFUSEMAP);
			GLSL_SetUniformInt(&tr.volumelightShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		}
		else*/ if (r_anamorphic->integer && r_volumelight->integer >= 4)
		{// Use Anamorphic image...
			GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);
			GLSL_SetUniformInt(&tr.volumelightShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		}
		else if (r_bloom->integer && r_volumelight->integer >= 3)
		{// Use Bloom image...
			GL_BindToTMU(tr.bloomRenderFBOImage[2], TB_DIFFUSEMAP);
			GLSL_SetUniformInt(&tr.volumelightShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		}
		else if (r_dynamicGlow->integer && r_volumelight->integer >= 2)
		{// Use Dynamic Glow image...
			GL_BindToTMU(tr.glowImage, TB_DIFFUSEMAP);
			GLSL_SetUniformInt(&tr.volumelightShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		}
		// We failed to use what the player specified, select the best option we can...
		else if (r_ssgi->integer && r_volumelight->integer >= 2)
		{// Use SSGI Saturation image...
			GL_BindToTMU(tr.anamorphicRenderFBOImage[2], TB_DIFFUSEMAP);
			GLSL_SetUniformInt(&tr.volumelightShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		}
		else if (r_anamorphic->integer && r_volumelight->integer >= 2)
		{// Use Anamorphic image...
			GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);
			GLSL_SetUniformInt(&tr.volumelightShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		}
		else if (r_bloom->integer && r_volumelight->integer >= 2)
		{// Use Bloom image...
			GL_BindToTMU(tr.bloomRenderFBOImage[2], TB_DIFFUSEMAP);
			GLSL_SetUniformInt(&tr.volumelightShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		}
		else
		{
			GL_BindToTMU(tr.fixedLevelsImage, TB_DIFFUSEMAP);
			GLSL_SetUniformInt(&tr.volumelightShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
		}

		GL_SetModelviewMatrix( backEnd.viewParms.ori.modelMatrix );
		GL_SetProjectionMatrix( backEnd.viewParms.projectionMatrix );

		GLSL_SetUniformMatrix16(&tr.volumelightShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, backEnd.viewParms.projectionMatrix);
		GLSL_SetUniformMatrix16(&tr.volumelightShader, UNIFORM_MODELMATRIX, backEnd.viewParms.ori.modelMatrix);

		GLSL_SetUniformInt(&tr.volumelightShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
		GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);


		{
			vec4_t viewInfo;

			float zmax = backEnd.viewParms.zFar;
			float zmin = r_znear->value;

			VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
			//VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);

			GLSL_SetUniformVec4(&tr.volumelightShader, UNIFORM_VIEWINFO, viewInfo);
		}

		{
			vec2_t screensize;
			screensize[0] = glConfig.vidWidth;
			screensize[1] = glConfig.vidHeight;

			GLSL_SetUniformVec2(&tr.volumelightShader, UNIFORM_DIMENSIONS, screensize);
		}
		
		{
			vec4_t local0;
			local0[0] = dl->origin[0];
			local0[1] = dl->origin[1];
			local0[2] = dl->origin[2];
			local0[3] = ((2048.0 - (distance+0.001)) / 2048.0);// * 0.66666;// / (float)((float)NUM_VISIBLE_LIGHTS/2);

			//float* local0b = local0;
			//local0b+=4;

			//ri->Printf(PRINT_WARNING, "Light %i is at %f %f %f.\n", l, local0[0], local0[1], local0[2]);

			GLSL_SetUniformVec4(&tr.volumelightShader, UNIFORM_LOCAL0, local0);
		}

		{
			vec4_t local2;
			local2[0] = x / 640;
			local2[1] = 1.0 - (y / 480);
			local2[2] = r_testvar->value;
			local2[3] = 0.0;

			//ri->Printf(PRINT_WARNING, "Light %i is at %f %f.\n", l, local2[0], local2[1]);

			GLSL_SetUniformVec4(&tr.volumelightShader, UNIFORM_LOCAL2, local2);
		}

		{
			vec4_t local1;
			local1[0] = dl->color[0];
			local1[1] = dl->color[1];
			local1[2] = dl->color[2];
			local1[3] = 0.0;

			GLSL_SetUniformVec4(&tr.volumelightShader, UNIFORM_LOCAL1, local1);
		}

		{
			vec4_t local3;
			local3[0] = r_volumelightExposure->value;
			local3[1] = r_volumelightDecay->value;
			local3[2] = r_volumelightDensity->value;
			local3[3] = r_volumelightWeight->value;

			GLSL_SetUniformVec4(&tr.volumelightShader, UNIFORM_LOCAL3, local3);
		}

		FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.volumelightShader, color, 0);
		FBO_FastBlit(ldrFbo, ldrBox, hdrFbo, hdrBox, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
}

void RB_HDR(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.hdrShader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

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

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.hdrShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
}

void RB_FakeDepth(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.fakedepthShader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	//qglUseProgramObjectARB(tr.fakedepthShader.program);
	
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

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.fakedepthShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
}

void RB_FakeDepthParallax(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.fakedepthSteepParallaxShader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	//qglUseProgramObjectARB(tr.fakedepthShader.program);
	
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
		VectorSet4(local0, r_depthParallaxScale->value, r_depthParallaxMultiplier->value, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.fakedepthSteepParallaxShader, UNIFORM_LOCAL0, local0);
	}

	{
		vec4_t local1;
		VectorSet4(local1, r_depthParallaxEyeX->value, r_depthParallaxEyeY->value, r_depthParallaxEyeZ->value, 0.0);
		GLSL_SetUniformVec4(&tr.fakedepthSteepParallaxShader, UNIFORM_LOCAL1, local1);
	}

	//FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.fakedepthSteepParallaxShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.fakedepthSteepParallaxShader, color, 0);
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

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	//qglUseProgramObjectARB(tr.fakedepthShader.program);
	
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

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.anaglyphShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
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
	GLSL_SetUniformMatrix16(&tr.ssaoShader, UNIFORM_MODELMATRIX, backEnd.ori.transformMatrix);

	GL_BindToTMU(tr.fixedLevelsImage, TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.ssaoShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.ssaoShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);
	
	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, zmin, zmax);

		GLSL_SetUniformVec4(&tr.ssaoShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.ssaoShader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.ssaoShader, color, 0);
}

void RB_SSAO2(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.ssao2Shader);

	GLSL_SetUniformMatrix16(&tr.ssao2Shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	GLSL_SetUniformMatrix16(&tr.ssao2Shader, UNIFORM_MODELMATRIX, backEnd.ori.transformMatrix);

	GL_BindToTMU(tr.fixedLevelsImage, TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.ssao2Shader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.ssao2Shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);
	
	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.ssao2Shader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.ssao2Shader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t local0;
		VectorSet4(local0, r_ssao2passes->value, 0.0, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.ssao2Shader, UNIFORM_LOCAL0, local0);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.ssao2Shader, color, 0);
}

void RB_TextureClean(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.texturecleanShader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);
	
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
	
	/*
	cvar_t  *r_textureCleanSigma;
	cvar_t  *r_textureCleanBSigma;
	cvar_t  *r_textureCleanMSize;
	*/
	{
		vec4_t local0;
		VectorSet4(local0, r_textureCleanSigma->value, r_textureCleanBSigma->value, r_textureCleanMSize->value, 0);
		GLSL_SetUniformVec4(&tr.texturecleanShader, UNIFORM_LOCAL0, local0);
	}

	//FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.texturecleanShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.texturecleanShader, color, 0);
}

void RB_ESharpening(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.esharpeningShader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	//qglUseProgramObjectARB(tr.esharpeningShader.program);
	
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
	
	//{
	//	vec4_t local0;
	//	VectorSet4(local0, r_textureCleanSigma->value, r_textureCleanBSigma->value, 0, 0);
	//	GLSL_SetUniformVec4(&tr.texturecleanShader, UNIFORM_LOCAL0, local0);
	//}

	//FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.esharpeningShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.esharpeningShader, color, 0);
}


void RB_ESharpening2(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.esharpening2Shader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.esharpening2Shader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.esharpening2Shader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.esharpeningShader, color, 0);
}


void RB_DOF(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.dofShader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	GLSL_SetUniformInt(&tr.dofShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GLSL_SetUniformInt(&tr.dofShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.dofShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);

		GLSL_SetUniformVec4(&tr.dofShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec4_t info;

		info[0] = r_dof->value;
		info[1] = 0.0;//r_testvalue0->value;
		info[2] = 0.0;
		info[3] = 0.0;

		VectorSet4(info, info[0], info[1], info[2], info[3]);

		GLSL_SetUniformVec4(&tr.dofShader, UNIFORM_LOCAL0, info);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.dofShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
}

void RB_Vibrancy(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.vibrancyShader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	GLSL_SetUniformInt(&tr.vibrancyShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);

	{
		vec4_t info;

		info[0] = r_vibrancy->value;
		info[1] = 0.0;
		info[2] = 0.0;
		info[3] = 0.0;

		VectorSet4(info, info[0], info[1], info[2], info[3]);

		GLSL_SetUniformVec4(&tr.vibrancyShader, UNIFORM_LOCAL0, info);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.vibrancyShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
}

void RB_SSGI(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t	color;
	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	{
		vec4i_t halfBox;
		vec2_t	texScale, texHalfScale, texDoubleScale;

		texScale[0] = texScale[1] = 1.0f;
		texHalfScale[0] = texHalfScale[1] = texScale[0] / 8.0;
		texDoubleScale[0] = texDoubleScale[1] = texScale[0] * 8.0;

		halfBox[0] = backEnd.viewParms.viewportX      * tr.anamorphicRenderFBOImage[0]->width  / (float)glConfig.vidWidth;
		halfBox[1] = backEnd.viewParms.viewportY      * tr.anamorphicRenderFBOImage[0]->height / (float)glConfig.vidHeight;
		halfBox[2] = backEnd.viewParms.viewportWidth  * tr.anamorphicRenderFBOImage[0]->width  / (float)glConfig.vidWidth;
		halfBox[3] = backEnd.viewParms.viewportHeight * tr.anamorphicRenderFBOImage[0]->height / (float)glConfig.vidHeight;

		//
		// Darken to VBO...
		//

		if (r_dynamicGlow->integer)
		{
			FBO_BlitFromTexture(tr.glowFboScaled[0]->colorImage[0], NULL, NULL, tr.anamorphicRenderFBO[0], NULL, NULL, color, 0);
		}
		else
		{
			GLSL_BindProgram(&tr.anamorphicDarkenShader);

			GL_BindToTMU(tr.fixedLevelsImage, TB_DIFFUSEMAP);

			{
				vec2_t screensize;
				screensize[0] = glConfig.vidWidth;
				screensize[1] = glConfig.vidHeight;

				GLSL_SetUniformVec2(&tr.anamorphicDarkenShader, UNIFORM_DIMENSIONS, screensize);
			}

			{
				vec4_t local0;
				VectorSet4(local0, r_anamorphicDarkenPower->value, 0.0, 0.0, 0.0);
				GLSL_SetUniformVec4(&tr.anamorphicDarkenShader, UNIFORM_LOCAL0, local0);
			}

			FBO_Blit(hdrFbo, NULL, texHalfScale, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicDarkenShader, color, 0);
			FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

		//
		// Blur the new darken'ed VBO...
		//

//#define __NEW_SATURATION_MAP_METHOD__ // grr slower and crappier...

#ifdef __NEW_SATURATION_MAP_METHOD__
		float SCAN_WIDTH = 16.0;

		//for (int i = 0; i < 2; i++)
		{// Initial blur...
			GLSL_BindProgram(&tr.anamorphicBlurShader);

			GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

			{
				vec2_t screensize;
				screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
				screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

				GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
			}

			{
				vec4_t local0;
				VectorSet4(local0, 0.0, 0.0, SCAN_WIDTH, 1.0);
				GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
			}

			FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0);
			FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

		{// Final blur...
			GLSL_BindProgram(&tr.anamorphicBlurShader);

			GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

			{
				vec2_t screensize;
				screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
				screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

				GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
			}

			{
				vec4_t local0;
				VectorSet4(local0, 0.0, 0.0, SCAN_WIDTH, 2.0);
				GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
			}

			FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0);
			FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
#else //!__NEW_SATURATION_MAP_METHOD__
		//float SCAN_WIDTH = 16.0;
		float SCAN_WIDTH = r_ssgiWidth->value;//8.0;

		{
			//
			// Bloom +-X axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.anamorphicBlurShader);

				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 1.0, 0.0, SCAN_WIDTH, 3.0);
					GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom +-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.anamorphicBlurShader);

				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 0.0, 1.0, SCAN_WIDTH, 3.0);
					GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom XY & -X-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.anamorphicBlurShader);

				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 1.0, 1.0, SCAN_WIDTH, 3.0);
					GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom -X+Y & +X-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.anamorphicBlurShader);

				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, -1.0, 1.0, SCAN_WIDTH, 3.0);
					GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}
		}

		//
		// Do a final blur pass - but this time don't mark it as a ssgi one - so that it uses darkness as well...
		//

		{
			//
			// Bloom +-X axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.anamorphicBlurShader);

				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 1.0, 0.0, SCAN_WIDTH, 0.0);
					GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom +-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.anamorphicBlurShader);

				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 0.0, 1.0, SCAN_WIDTH, 0.0);
					GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom XY & -X-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.anamorphicBlurShader);

				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 1.0, 1.0, SCAN_WIDTH, 0.0);
					GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom -X+Y & +X-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.anamorphicBlurShader);

				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, -1.0, 1.0, SCAN_WIDTH, 0.0);
					GLSL_SetUniformVec4(&tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}
		}
#endif //__NEW_SATURATION_MAP_METHOD__

		//
		// Copy (and upscale) the bloom image to our full screen image...
		//

		FBO_Blit(tr.anamorphicRenderFBO[0], NULL, texDoubleScale, tr.anamorphicRenderFBO[2], NULL, &tr.anamorphicBlurShader, color, 0);
	}

	//
	// Do the SSAO/SSGI...
	//

	GLSL_BindProgram(&tr.ssgiShader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	GLSL_SetUniformInt(&tr.ssgiShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GLSL_SetUniformInt(&tr.ssgiShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);
	GLSL_SetUniformInt(&tr.ssgiShader, UNIFORM_NORMALMAP, TB_NORMALMAP); // really scaled down dynamic glow map
	GL_BindToTMU(tr.anamorphicRenderFBOImage[2], TB_NORMALMAP); // really scaled down dynamic glow map

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.ssgiShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;
		//float zmin = backEnd.viewParms.zNear;

		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);

		//ri->Printf(PRINT_WARNING, "Sent zmin %f, zmax %f, zmax/zmin %f.\n", zmin, zmax, zmax / zmin);

		GLSL_SetUniformVec4(&tr.ssgiShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec4_t local0;
		local0[0] = r_ssgi->value;
		local0[1] = r_ssgiSamples->value;
		local0[2] = 0.0;
		local0[3] = 0.0;

		GLSL_SetUniformVec4(&tr.ssgiShader, UNIFORM_LOCAL0, local0);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.ssgiShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
}


void RB_TestShader(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox, int pass_num)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.testshaderShader);

	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	//GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	//GL_BindToTMU(tr.fixedLevelsImage, TB_DIFFUSEMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

	//GL_SelectTexture(1);
	//GL_Bind(tr.renderDepthImage);
	//GL_SelectTexture(0);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.testshaderShader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	{
		vec4_t viewInfo;

		float zmax = backEnd.viewParms.zFar;
		float zmin = r_znear->value;
		//float zmin = backEnd.viewParms.zNear;

		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);

		//ri->Printf(PRINT_WARNING, "Sent zmin %f, zmax %f, zmax/zmin %f.\n", zmin, zmax, zmax / zmin);

		GLSL_SetUniformVec4(&tr.testshaderShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec4_t l0;
		l0[0] = pass_num;
		l0[1] = r_testshaderValue1->value;
		l0[2] = r_testshaderValue2->value;
		l0[3] = r_testshaderValue3->value;

		GLSL_SetUniformVec4(&tr.testshaderShader, UNIFORM_LOCAL0, l0);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.testshaderShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
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
	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	GLSL_SetUniformMatrix16(&tr.underwaterShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformFloat(&tr.underwaterShader, UNIFORM_TIME, backEnd.refdef.floatTime*5.0/*tr.refdef.floatTime*/);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

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
	GL_BindToTMU(tr.fixedLevelsImage, TB_LEVELSMAP);

	GLSL_SetUniformMatrix16(&tr.fxaaShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth;
		screensize[1] = glConfig.vidHeight;

		GLSL_SetUniformVec2(&tr.fxaaShader, UNIFORM_DIMENSIONS, screensize);
	}
	
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.fxaaShader, color, 0);
}
