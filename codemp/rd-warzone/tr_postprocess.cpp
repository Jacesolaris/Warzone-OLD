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

	halfBox[0] = backEnd.viewParms.viewportX      * tr.bloomRenderFBOImage[0]->width  / ((float)glConfig.vidWidth * r_superSampleMultiplier->value);
	halfBox[1] = backEnd.viewParms.viewportY      * tr.bloomRenderFBOImage[0]->height / ((float)glConfig.vidHeight * r_superSampleMultiplier->value);
	halfBox[2] = backEnd.viewParms.viewportWidth  * tr.bloomRenderFBOImage[0]->width  / ((float)glConfig.vidWidth * r_superSampleMultiplier->value);
	halfBox[3] = backEnd.viewParms.viewportHeight * tr.bloomRenderFBOImage[0]->height / ((float)glConfig.vidHeight * r_superSampleMultiplier->value);

	//
	// Darken to VBO...
	//
	
	FBO_BlitFromTexture(tr.glowFboScaled[0]->colorImage[0], NULL, NULL, tr.bloomRenderFBO[0], NULL, NULL, color, 0);

	//
	// Blur the new darken'ed VBO...
	//

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

		{
			vec4_t local0;
			VectorSet4(local0, 0.0, 0.0, /* bloom width */ 3.0, 0.0); // non-flicker version
			//VectorSet4(local0, 0.0, 0.0, /* bloom width */ 3.0 * ((float)irand(90,100) / 100), 0.0); // use flicker???
			GLSL_SetUniformVec4(&tr.bloomBlurShader, UNIFORM_LOCAL0, local0);
		}

		FBO_Blit(tr.bloomRenderFBO[0], NULL, NULL, tr.bloomRenderFBO[1], NULL, &tr.bloomBlurShader, color, 0);
		FBO_FastBlit(tr.bloomRenderFBO[1], NULL, tr.bloomRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
	
	//
	// Copy (and upscale) the bloom image to our full screen image...
	//

	FBO_Blit(tr.bloomRenderFBO[0], NULL, texDoubleScale, tr.bloomRenderFBO[2], NULL, &tr.bloomBlurShader, color, 0);

	//
	// Combine the screen with the bloom'ed VBO...
	//
	
	
	GLSL_BindProgram(&tr.bloomCombineShader);
	
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.bloomCombineShader, UNIFORM_DIFFUSEMAP,   TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.bloomCombineShader, UNIFORM_NORMALMAP,   TB_NORMALMAP);

	GL_BindToTMU(tr.bloomRenderFBOImage[2], TB_NORMALMAP);

	{
		vec4_t local0;
		VectorSet4(local0, r_bloomScale->value, 0.0, 0.0, 0.0);
		VectorSet4(local0, 0.5 * r_bloomScale->value, 0.0, 0.0, 0.0); // Account for already added glow...
		GLSL_SetUniformVec4(&tr.bloomCombineShader, UNIFORM_LOCAL0, local0);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.bloomCombineShader, color, 0);

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

	halfBox[0] = backEnd.viewParms.viewportX      * tr.anamorphicRenderFBOImage[0]->width  / ((float)glConfig.vidWidth * r_superSampleMultiplier->value);
	halfBox[1] = backEnd.viewParms.viewportY      * tr.anamorphicRenderFBOImage[0]->height / ((float)glConfig.vidHeight * r_superSampleMultiplier->value);
	halfBox[2] = backEnd.viewParms.viewportWidth  * tr.anamorphicRenderFBOImage[0]->width  / ((float)glConfig.vidWidth * r_superSampleMultiplier->value);
	halfBox[3] = backEnd.viewParms.viewportHeight * tr.anamorphicRenderFBOImage[0]->height / ((float)glConfig.vidHeight * r_superSampleMultiplier->value);

	//
	// Darken to VBO...
	//
	
	FBO_BlitFromTexture(tr.glowFboScaled[0]->colorImage[0], NULL, NULL, tr.anamorphicRenderFBO[0], NULL, NULL, color, 0);
	
	//
	// Blur the new darken'ed VBO...
	//

	//
	// Bloom X axis... (to VBO 1)
	//

	GLSL_BindProgram(&tr.anamorphicBlurShader);

	GLSL_SetUniformInt(&tr.anamorphicBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
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
	
	//
	// Copy (and upscale) the bloom image to our full screen image...
	//

	FBO_Blit(tr.anamorphicRenderFBO[0], NULL, texDoubleScale, tr.anamorphicRenderFBO[2], NULL, &tr.anamorphicBlurShader, color, 0);

	//
	// Combine the screen with the bloom'ed VBO...
	//
	
	GLSL_BindProgram(&tr.anamorphicCombineShader);
	
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.anamorphicCombineShader, UNIFORM_DIFFUSEMAP,   TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.anamorphicCombineShader, UNIFORM_NORMALMAP,   TB_NORMALMAP);

	GL_BindToTMU(tr.anamorphicRenderFBOImage[2], TB_NORMALMAP);

	{
		vec4_t local0;
		VectorSet4(local0, 1.0, 0.0, 0.0, 0.0); // Account for already added glow...
		GLSL_SetUniformVec4(&tr.anamorphicCombineShader, UNIFORM_LOCAL0, local0);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.anamorphicCombineShader, color, 0);

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

	GLSL_SetUniformInt(&tr.lensflareShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	vec2_t screensize;
	screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
	screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;
	GLSL_SetUniformVec2(&tr.lensflareShader, UNIFORM_DIMENSIONS, screensize);

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
	int hFOV = 120;
	int vFOV = 120;

	TR_AxisToAngles(tr.refdef.viewaxis, fromAngles);

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

qboolean Volumetric_Visible(vec3_t from, vec3_t to)
{
	trace_t trace;

	Volumetric_Trace( &trace, from, NULL, NULL, to, -1, (CONTENTS_SOLID|CONTENTS_TERRAIN) );

	if (trace.fraction != 1.0 && Distance(trace.endpos, to) > 64)
		return qfalse;

	return qtrue;
}

vec3_t roof;

void Volumetric_RoofHeight(vec3_t from)
{
	trace_t trace;
	vec3_t roofh;
	VectorSet(roofh, from[0]+192, from[1], from[2]);
	Volumetric_Trace( &trace, from, NULL, NULL, roofh, -1, (CONTENTS_SOLID|CONTENTS_TERRAIN) );
	VectorSet(roof, trace.endpos[0]-8.0, trace.endpos[1], trace.endpos[2]);
}

extern void R_WorldToLocal (const vec3_t world, vec3_t local);
extern void R_LocalPointToWorld (const vec3_t local, vec3_t world);

extern vec2_t SUN_SCREEN_POSITION;
extern qboolean SUN_VISIBLE;

qboolean RB_VolumetricLight(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;
	int NUM_VISIBLE_LIGHTS = 0;
	int SUN_ID = 17;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	//ri->Printf(PRINT_WARNING, "%i dlights.\n", backEnd.refdef.num_dlights);

	if ( !backEnd.refdef.num_dlights && !SUN_VISIBLE ) {
		//ri->Printf(PRINT_WARNING, "0 dlights.\n");
		return qfalse;
	}

	//
	// UQ1: Now going to allow a maximum of MAX_VOLUMETRIC_LIGHTS volumetric lights on screen for FPS...
	//
	const int	MAX_VOLUMETRIC_LIGHTS = 16;
	int			NUM_CLOSE_LIGHTS = 0;
	int			CLOSEST_LIGHTS[MAX_VOLUMETRIC_LIGHTS] = {0};
	vec2_t		CLOSEST_LIGHTS_POSITIONS[MAX_VOLUMETRIC_LIGHTS] = {0};
	float		CLOSEST_LIGHTS_DISTANCES[MAX_VOLUMETRIC_LIGHTS] = {0};
	vec3_t		CLOSEST_LIGHTS_COLORS[MAX_VOLUMETRIC_LIGHTS] = {0};

	float strengthMult = 1.0;
	if (r_dynamiclight->integer < 3) strengthMult = 2.0; // because the lower samples result in less color...

	for ( int l = 0 ; l < backEnd.refdef.num_dlights ; l++ ) 
	{
		dlight_t	*dl = &backEnd.refdef.dlights[l];
		
		float x, y, distance;

		distance = Distance(backEnd.refdef.vieworg, dl->origin);

		if (!TR_InFOV( dl->origin, backEnd.refdef.vieworg ))
		{
			continue; // not on screen...
		}

		/*if (!TR_WorldToScreen(dl->origin, &x, &y)) 
		{
			continue; // not on screen...
		}
		*/

		if (!Volumetric_Visible(backEnd.refdef.vieworg, dl->origin))
		{// Trace to actual position failed... Try above...
			vec3_t tmpOrg;
			vec3_t eyeOrg;
			vec3_t tmpRoof;
			vec3_t eyeRoof;

			// Calculate ceiling heights at both positions...
			Volumetric_RoofHeight(dl->origin);
			VectorCopy(roof, tmpRoof);
			Volumetric_RoofHeight(backEnd.refdef.vieworg);
			VectorCopy(roof, eyeRoof);

			VectorSet(tmpOrg, tmpRoof[0], dl->origin[1], dl->origin[2]);
			VectorSet(eyeOrg, backEnd.refdef.vieworg[0], backEnd.refdef.vieworg[1], backEnd.refdef.vieworg[2]);
			if (!Volumetric_Visible(eyeOrg, tmpOrg))
			{// Trace to above position failed... Try trace from above viewer...
				VectorSet(tmpOrg, dl->origin[0], dl->origin[1], dl->origin[2]);
				VectorSet(eyeOrg, eyeRoof[0], backEnd.refdef.vieworg[1], backEnd.refdef.vieworg[2]);
				if (!Volumetric_Visible(eyeOrg, tmpOrg))
				{// Trace from above viewer failed... Try trace from above, to above...
					VectorSet(tmpOrg, tmpRoof[0], dl->origin[1], dl->origin[2]);
					VectorSet(eyeOrg, eyeRoof[0], backEnd.refdef.vieworg[1], backEnd.refdef.vieworg[2]);
					if (!Volumetric_Visible(eyeOrg, tmpOrg))
					{// Trace from/to above viewer failed...
						continue; // Can't see this...
					}
				}
			}
		}

		int	xcenter, ycenter;
		vec3_t	local, transformed;
		vec3_t	vfwd, vright, vup, viewAngles;

		TR_AxisToAngles(backEnd.refdef.viewaxis, viewAngles);

		//NOTE: did it this way because most draw functions expect virtual 640x480 coords
		//	and adjust them for current resolution
		xcenter = 640 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn
		ycenter = 480 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn

		VectorSubtract (dl->origin, backEnd.refdef.vieworg, local);

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

		x = (xcenter + xzi * transformed[0]);
		y = (ycenter - yzi * transformed[1]);

		float depth = (distance/2048.0);
		if (depth > 1.0) depth = 1.0;

		if (NUM_CLOSE_LIGHTS < MAX_VOLUMETRIC_LIGHTS-1)
		{// Have free light slots for a new light...
			CLOSEST_LIGHTS[NUM_CLOSE_LIGHTS] = l;
			CLOSEST_LIGHTS_POSITIONS[NUM_CLOSE_LIGHTS][0] = x / 640;
			CLOSEST_LIGHTS_POSITIONS[NUM_CLOSE_LIGHTS][1] = 1.0 - (y / 480);
			CLOSEST_LIGHTS_DISTANCES[NUM_CLOSE_LIGHTS] = depth;
			CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][0] = dl->color[0]*min(r_volumeLightStrength->value*4.0*strengthMult, 1.0);
			CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][1] = dl->color[1]*min(r_volumeLightStrength->value*4.0*strengthMult, 1.0);
			CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][2] = dl->color[2]*min(r_volumeLightStrength->value*4.0*strengthMult, 1.0);
			NUM_CLOSE_LIGHTS++;
			continue;
		}
		else
		{// See if this is closer then one of our other lights...
			int		farthest_light = 0;
			float	farthest_distance = 0.0;

			for (int i = 0; i < NUM_CLOSE_LIGHTS; i++)
			{// Find the most distance light in our current list to replace, if this new option is closer...
				dlight_t	*thisLight = &backEnd.refdef.dlights[CLOSEST_LIGHTS[i]];
				float		dist = Distance(thisLight->origin, backEnd.refdef.vieworg);

				if (dist > farthest_distance)
				{// This one is further!
					farthest_light = i;
					farthest_distance = dist;
					break;
				}
			}

			if (Distance(dl->origin, backEnd.refdef.vieworg) < farthest_distance)
			{// This light is closer. Replace this one in our array of closest lights...
				CLOSEST_LIGHTS[farthest_light] = l;
				CLOSEST_LIGHTS_POSITIONS[farthest_light][0] = x / 640;
				CLOSEST_LIGHTS_POSITIONS[farthest_light][1] = 1.0 - (y / 480);
				CLOSEST_LIGHTS_DISTANCES[farthest_light] = depth;
				CLOSEST_LIGHTS_COLORS[farthest_light][0] = dl->color[0]*min(r_volumeLightStrength->value*4.0*strengthMult, 1.0);
				CLOSEST_LIGHTS_COLORS[farthest_light][1] = dl->color[1]*min(r_volumeLightStrength->value*4.0*strengthMult, 1.0);
				CLOSEST_LIGHTS_COLORS[farthest_light][2] = dl->color[2]*min(r_volumeLightStrength->value*4.0*strengthMult, 1.0);
			}
		}
	}

	if ( SUN_VISIBLE )
	{// Add sun...
		//SUN_SCREEN_POSITION
		if (NUM_CLOSE_LIGHTS < MAX_VOLUMETRIC_LIGHTS-1)
		{// Have free light slots for a new light...
			CLOSEST_LIGHTS_POSITIONS[NUM_CLOSE_LIGHTS][0] = SUN_SCREEN_POSITION[0];
			CLOSEST_LIGHTS_POSITIONS[NUM_CLOSE_LIGHTS][1] = SUN_SCREEN_POSITION[1];
			CLOSEST_LIGHTS_DISTANCES[NUM_CLOSE_LIGHTS] = 0.9;
			CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][0] = (tr.sunLight[0] / 255.0)*r_volumeLightStrength->value*strengthMult;
			CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][1] = (tr.sunLight[1] / 255.0)*r_volumeLightStrength->value*strengthMult;
			CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][2] = (tr.sunLight[2] / 255.0)*r_volumeLightStrength->value*strengthMult;
			SUN_ID = NUM_CLOSE_LIGHTS;
			NUM_CLOSE_LIGHTS++;
		}
		else
		{// See if this is closer then one of our other lights...
			int		farthest_light = 0;
			float	farthest_distance = 0.0;

			for (int i = 0; i < NUM_CLOSE_LIGHTS; i++)
			{// Find the most distance light in our current list to replace, if this new option is closer...
				dlight_t	*thisLight = &backEnd.refdef.dlights[CLOSEST_LIGHTS[i]];
				float		dist = Distance(thisLight->origin, backEnd.refdef.vieworg);

				if (dist > farthest_distance)
				{// This one is further!
					farthest_light = i;
					farthest_distance = dist;
					break;
				}
			}

			CLOSEST_LIGHTS_POSITIONS[farthest_light][0] = SUN_SCREEN_POSITION[0];
			CLOSEST_LIGHTS_POSITIONS[farthest_light][1] = SUN_SCREEN_POSITION[1];
			CLOSEST_LIGHTS_DISTANCES[farthest_light] = 0.9;
			CLOSEST_LIGHTS_COLORS[farthest_light][0] = (tr.sunLight[0] / 255.0)*r_volumeLightStrength->value*strengthMult;
			CLOSEST_LIGHTS_COLORS[farthest_light][1] = (tr.sunLight[1] / 255.0)*r_volumeLightStrength->value*strengthMult;
			CLOSEST_LIGHTS_COLORS[farthest_light][2] = (tr.sunLight[2] / 255.0)*r_volumeLightStrength->value*strengthMult;
			SUN_ID = farthest_light;
		}
	}

	//ri->Printf(PRINT_WARNING, "%i volume lights.\n", NUM_CLOSE_LIGHTS);

	// None to draw...
	if (NUM_CLOSE_LIGHTS <= 0) {
		//ri->Printf(PRINT_WARNING, "0 visible dlights. %i total dlights.\n", backEnd.refdef.num_dlights);
		return qfalse;
	}

	GLSL_BindProgram(&tr.volumeLightShader[r_dynamiclight->integer - 1]);

	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.volumeLightShader[r_dynamiclight->integer - 1], UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);
	GLSL_SetUniformInt(&tr.volumeLightShader[r_dynamiclight->integer - 1], UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);

	GL_SetModelviewMatrix( backEnd.viewParms.ori.modelMatrix );
	GL_SetProjectionMatrix( backEnd.viewParms.projectionMatrix );

	GLSL_SetUniformMatrix16(&tr.volumeLightShader[r_dynamiclight->integer - 1], UNIFORM_MODELVIEWPROJECTIONMATRIX, backEnd.viewParms.projectionMatrix);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.volumeLightShader[r_dynamiclight->integer - 1], UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;

		//float zmax = backEnd.viewParms.zFar;
		float zmax = 2048.0;//backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, (float)SUN_ID);

		GLSL_SetUniformVec4(&tr.volumeLightShader[r_dynamiclight->integer - 1], UNIFORM_VIEWINFO, viewInfo);
	}

	GLSL_SetUniformInt(&tr.volumeLightShader[r_dynamiclight->integer - 1], UNIFORM_LIGHTCOUNT, NUM_CLOSE_LIGHTS);
	GLSL_SetUniformVec2x16(&tr.volumeLightShader[r_dynamiclight->integer - 1], UNIFORM_LIGHTPOSITIONS, CLOSEST_LIGHTS_POSITIONS, MAX_VOLUMETRIC_LIGHTS);
	GLSL_SetUniformVec3x16(&tr.volumeLightShader[r_dynamiclight->integer - 1], UNIFORM_LIGHTCOLORS, CLOSEST_LIGHTS_COLORS, MAX_VOLUMETRIC_LIGHTS);
	GLSL_SetUniformFloatx16(&tr.volumeLightShader[r_dynamiclight->integer - 1], UNIFORM_LIGHTDISTANCES, CLOSEST_LIGHTS_DISTANCES, MAX_VOLUMETRIC_LIGHTS);

//#define VOLUME_LIGHT_DEBUG
//#define VOLUME_LIGHT_SINGLE_PASS

#if !defined(VOLUME_LIGHT_DEBUG) && !defined(VOLUME_LIGHT_SINGLE_PASS)
	FBO_Blit(hdrFbo, hdrBox, NULL, tr.genericFbo2, ldrBox, &tr.volumeLightShader[r_dynamiclight->integer - 1], color, 0);

	// Combine render and hbao...
	GLSL_BindProgram(&tr.volumeLightCombineShader);

	GLSL_SetUniformMatrix16(&tr.volumeLightCombineShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	GLSL_SetUniformMatrix16(&tr.volumeLightCombineShader, UNIFORM_MODELMATRIX, backEnd.ori.transformMatrix);

	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.volumeLightCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.volumeLightCombineShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.genericFBO2Image, TB_NORMALMAP);

	vec2_t screensize;
	screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
	screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;
	GLSL_SetUniformVec2(&tr.volumeLightCombineShader, UNIFORM_DIMENSIONS, screensize);

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.volumeLightCombineShader, color, 0);
#else //defined(VOLUME_LIGHT_DEBUG) || defined(VOLUME_LIGHT_SINGLE_PASS)
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.volumeLightShader[r_dynamiclight->integer - 1], color, 0);
#endif //defined(VOLUME_LIGHT_DEBUG) || defined(VOLUME_LIGHT_SINGLE_PASS)

	//ri->Printf(PRINT_WARNING, "%i visible dlights. %i total dlights.\n", NUM_CLOSE_LIGHTS, backEnd.refdef.num_dlights);
	return qtrue;
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

	GLSL_SetUniformInt(&tr.hdrShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.hdrShader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.hdrShader, color, 0);
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

	GLSL_SetUniformInt(&tr.magicdetailShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
	GLSL_SetUniformInt(&tr.magicdetailShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

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
		VectorSet4(local0, r_magicdetailStrength->value, 0.0, 0.0, 0.0); // non-flicker version
		GLSL_SetUniformVec4(&tr.magicdetailShader, UNIFORM_LOCAL0, local0);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.magicdetailShader, color, 0);
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

	GLSL_SetUniformInt(&tr.fakedepthShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	{
		vec4_t viewInfo;

		//float zmax = backEnd.viewParms.zFar;
		float zmax = 2048.0;//backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);

		GLSL_SetUniformVec4(&tr.fakedepthShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.fakedepthShader, UNIFORM_DIMENSIONS, screensize);
	}
	
	{
		vec4_t local0;
		VectorSet4(local0, r_depthScale->value, r_depthParallax->value, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.fakedepthShader, UNIFORM_LOCAL0, local0);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.fakedepthShader, color, 0);
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

	GLSL_SetUniformInt(&tr.fakedepthSteepParallaxShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
	
	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.fakedepthSteepParallaxShader, UNIFORM_DIMENSIONS, screensize);
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

	GLSL_SetUniformInt(&tr.anaglyphShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.anaglyphShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

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
	GLSL_SetUniformMatrix16(&tr.ssaoShader, UNIFORM_MODELMATRIX, backEnd.ori.transformMatrix);
	
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	GLSL_SetUniformInt(&tr.ssaoShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.ssaoShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);
	
	{
		vec4_t viewInfo;

		//float zmax = backEnd.viewParms.zFar;
		float zmax = 2048.0;//backEnd.viewParms.zFar;
		float zmin = r_znear->value;

		VectorSet4(viewInfo, zmax / zmin, zmax, zmin, zmax);

		GLSL_SetUniformVec4(&tr.ssaoShader, UNIFORM_VIEWINFO, viewInfo);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.ssaoShader, UNIFORM_DIMENSIONS, screensize);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.ssaoShader, color, 0);
}

void RB_RBM(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t		color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.rbmShader);

	GLSL_SetUniformMatrix16(&tr.rbmShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	GLSL_SetUniformMatrix16(&tr.rbmShader, UNIFORM_MODELMATRIX, backEnd.ori.transformMatrix);

	GLSL_SetUniformVec3(&tr.rbmShader, UNIFORM_VIEWORIGIN,  backEnd.refdef.vieworg);

	GLSL_SetUniformInt(&tr.rbmShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);
	GLSL_SetUniformInt(&tr.rbmShader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.normalDetailedImage, TB_NORMALMAP);
	GLSL_SetUniformInt(&tr.rbmShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);
	
	{
		vec4_t local0;
		VectorSet4(local0, r_rbmStrength->value, 0.0, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.rbmShader, UNIFORM_LOCAL0, local0);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.rbmShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;
		//float zmax = backEnd.viewParms.zFar;
		float zmax = 2048.0;//backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(&tr.rbmShader, UNIFORM_VIEWINFO, viewInfo);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.rbmShader, color, 0);
}

void RB_WaterPost(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t		color;

	shaderProgram_t *shader = &tr.waterPostShader;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(shader);

	GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformInt(shader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);
	GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);
	GLSL_SetUniformInt(shader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.normalDetailedImage, TB_NORMALMAP);
	GLSL_SetUniformInt(shader, UNIFORM_SPECULARMAP, TB_SPECULARMAP);
	GL_BindToTMU(tr.foliageImage, TB_SPECULARMAP);
	
	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
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

	GLSL_SetUniformFloat(shader, UNIFORM_TIME, backEnd.refdef.floatTime);

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, shader, color, 0);
}

void RB_SSS(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t		color;

	shaderProgram_t *shader = &tr.sssShader;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	if (r_sss->integer >= 2) shader = &tr.sss2Shader;

	GLSL_BindProgram(shader);

	GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformInt(shader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);
	GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);
	GLSL_SetUniformInt(shader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.glowImage, TB_NORMALMAP);
	
	if (r_sss->integer < 2)
	{
		GLSL_SetUniformInt(shader, UNIFORM_SPECULARMAP, TB_SPECULARMAP);
		GL_BindToTMU(tr.foliageImage, TB_SPECULARMAP);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t loc;
		VectorSet4(loc, r_sssMinRange->value, r_sssMaxRange->value, 0.0, 0.0);
		GLSL_SetUniformVec4(shader, UNIFORM_LOCAL0, loc);
	}

	if ( SUN_VISIBLE )
	{// Add sun...
		//SUN_SCREEN_POSITION
		vec4_t loc;
		VectorSet4(loc, SUN_SCREEN_POSITION[0], SUN_SCREEN_POSITION[1], 0.0, 0.0);
		GLSL_SetUniformVec4(shader, UNIFORM_LOCAL1, loc);
	}

	{
		vec4_t viewInfo;
		float zmax = 2048.0;//backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(shader, UNIFORM_VIEWINFO, viewInfo);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, shader, color, 0);
}

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
	//GLSL_SetUniformMatrix16(shader, UNIFORM_INVEYEPROJECTIONMATRIX, glState.invEyeProjection);
	GLSL_SetUniformMatrix16(shader, UNIFORM_INVEYEPROJECTIONMATRIX, glState.invProjection);
	GLSL_SetUniformMatrix16(shader, UNIFORM_MODELMATRIX, backEnd.ori.transformMatrix);

	GLSL_SetUniformInt(shader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);
	GLSL_SetUniformInt(shader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.normalDetailedImage, TB_NORMALMAP);
	GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);
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
	GLSL_SetUniformMatrix16(&tr.hbaoCombineShader, UNIFORM_MODELMATRIX, backEnd.ori.transformMatrix);

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

void RB_TextureClean(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.texturecleanShader);

	GLSL_SetUniformInt(&tr.texturecleanShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
	
	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.texturecleanShader, UNIFORM_DIMENSIONS, screensize);
	}
	
	{
		vec4_t local0;
		VectorSet4(local0, r_textureCleanSigma->value, r_textureCleanBSigma->value, r_textureCleanMSize->value, 0);
		GLSL_SetUniformVec4(&tr.texturecleanShader, UNIFORM_LOCAL0, local0);
	}

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

	GLSL_SetUniformInt(&tr.esharpeningShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	vec2_t screensize;
	screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
	screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;
	GLSL_SetUniformVec2(&tr.esharpeningShader, UNIFORM_DIMENSIONS, screensize);
	
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

	GLSL_SetUniformInt(&tr.esharpening2Shader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

	vec2_t screensize;
	screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
	screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;
	GLSL_SetUniformVec2(&tr.esharpening2Shader, UNIFORM_DIMENSIONS, screensize);

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.esharpening2Shader, color, 0);
}


void RB_DOF(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox, int direction)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	if (r_dof->integer >= 3)
	{
		GLSL_BindProgram(&tr.dof2Shader);

		GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

		GLSL_SetUniformInt(&tr.dof2Shader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
		GLSL_SetUniformInt(&tr.dof2Shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
		GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

		{
			vec2_t screensize;
			screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
			screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

			GLSL_SetUniformVec2(&tr.dof2Shader, UNIFORM_DIMENSIONS, screensize);
		}

		{
			vec4_t info;

			info[0] = r_dof->value;
			info[1] = r_testvalue0->value;
			info[2] = r_testvalue1->value;
			info[3] = direction;

			VectorSet4(info, info[0], info[1], info[2], info[3]);

			GLSL_SetUniformVec4(&tr.dof2Shader, UNIFORM_LOCAL0, info);
		}

		FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.dof2Shader, color, 0);
	}
	else
	{
#if 0
		GLSL_BindProgram(&tr.dofShader);

		GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

		GLSL_SetUniformInt(&tr.dofShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
		GLSL_SetUniformInt(&tr.dofShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
		GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

		{
			vec2_t screensize;
			screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
			screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

			GLSL_SetUniformVec2(&tr.dofShader, UNIFORM_DIMENSIONS, screensize);
		}

		{
			vec4_t viewInfo;

			//float zmax = backEnd.viewParms.zFar;
			float zmax = 2048.0;//backEnd.viewParms.zFar;
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

		FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.dofShader, color, 0);
#else
		GLSL_BindProgram(&tr.dofShader);

		GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);

		GLSL_SetUniformInt(&tr.dofShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
		GLSL_SetUniformInt(&tr.dofShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
		GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

		{
			vec2_t screensize;
			screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
			screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

			GLSL_SetUniformVec2(&tr.dofShader, UNIFORM_DIMENSIONS, screensize);
		}

		{
			vec4_t info;

			info[0] = r_dof->value;
			info[1] = r_testvalue0->value;
			info[2] = r_testvalue1->value;
			info[3] = direction;

			VectorSet4(info, info[0], info[1], info[2], info[3]);

			GLSL_SetUniformVec4(&tr.dofShader, UNIFORM_LOCAL0, info);
		}

		FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.dofShader, color, 0);
#endif
	}
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

	GLSL_SetUniformInt(&tr.vibrancyShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_DIFFUSEMAP);

	{
		vec4_t info;
		VectorSet4(info, r_vibrancy->value, 0.0, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.vibrancyShader, UNIFORM_LOCAL0, info);
	}

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.vibrancyShader, color, 0);
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

		halfBox[0] = backEnd.viewParms.viewportX      * tr.anamorphicRenderFBOImage[0]->width  / ((float)glConfig.vidWidth * r_superSampleMultiplier->value);
		halfBox[1] = backEnd.viewParms.viewportY      * tr.anamorphicRenderFBOImage[0]->height / ((float)glConfig.vidHeight * r_superSampleMultiplier->value);
		halfBox[2] = backEnd.viewParms.viewportWidth  * tr.anamorphicRenderFBOImage[0]->width  / ((float)glConfig.vidWidth * r_superSampleMultiplier->value);
		halfBox[3] = backEnd.viewParms.viewportHeight * tr.anamorphicRenderFBOImage[0]->height / ((float)glConfig.vidHeight * r_superSampleMultiplier->value);

		//
		// Darken to VBO...
		//

		FBO_BlitFromTexture(tr.glowFboScaled[0]->colorImage[0], NULL, NULL, tr.anamorphicRenderFBO[0], NULL, NULL, color, 0);

		//
		// Blur the new darken'ed VBO...
		//

		float SCAN_WIDTH = r_ssgiWidth->value;

		//for (int i = 0; i < 2; i++)
		{
			//
			// Bloom +-X axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.ssgiBlurShader);

				GLSL_SetUniformInt(&tr.ssgiBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 1.0, 0.0, SCAN_WIDTH, 3.0);
					GLSL_SetUniformVec4(&tr.ssgiBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom +-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.ssgiBlurShader);

				GLSL_SetUniformInt(&tr.ssgiBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 0.0, 1.0, SCAN_WIDTH, 3.0);
					GLSL_SetUniformVec4(&tr.ssgiBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom XY & -X-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.ssgiBlurShader);

				GLSL_SetUniformInt(&tr.ssgiBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 1.0, 1.0, SCAN_WIDTH, 3.0);
					GLSL_SetUniformVec4(&tr.ssgiBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom -X+Y & +X-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.ssgiBlurShader);

				GLSL_SetUniformInt(&tr.ssgiBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, -1.0, 1.0, SCAN_WIDTH, 3.0);
					GLSL_SetUniformVec4(&tr.ssgiBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}
		}

		//
		// Do a final blur pass - but this time don't mark it as a ssgi one - so that it uses darkness as well...
		//

		//for (int i = 0; i < 2; i++)
		{
			//
			// Bloom +-X axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.ssgiBlurShader);

				GLSL_SetUniformInt(&tr.ssgiBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 1.0, 0.0, SCAN_WIDTH, 0.0);
					GLSL_SetUniformVec4(&tr.ssgiBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom +-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.ssgiBlurShader);

				GLSL_SetUniformInt(&tr.ssgiBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 0.0, 1.0, SCAN_WIDTH, 0.0);
					GLSL_SetUniformVec4(&tr.ssgiBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom XY & -X-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.ssgiBlurShader);

				GLSL_SetUniformInt(&tr.ssgiBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, 1.0, 1.0, SCAN_WIDTH, 0.0);
					GLSL_SetUniformVec4(&tr.ssgiBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}

			//
			// Bloom -X+Y & +X-Y axis... (to VBO 1)
			//

			{
				GLSL_BindProgram(&tr.ssgiBlurShader);

				GLSL_SetUniformInt(&tr.ssgiBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
				GL_BindToTMU(tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP);

				{
					vec2_t screensize;
					screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
					screensize[1] = tr.anamorphicRenderFBOImage[0]->height;

					GLSL_SetUniformVec2(&tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize);
				}

				{
					vec4_t local0;
					//VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
					VectorSet4(local0, -1.0, 1.0, SCAN_WIDTH, 0.0);
					GLSL_SetUniformVec4(&tr.ssgiBlurShader, UNIFORM_LOCAL0, local0);
				}

				FBO_Blit(tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0);
				FBO_FastBlit(tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			}
		}

		//
		// Copy (and upscale) the bloom image to our full screen image...
		//

		FBO_Blit(tr.anamorphicRenderFBO[0], NULL, texDoubleScale, tr.anamorphicRenderFBO[2], NULL, &tr.ssgiBlurShader, color, 0);
	}

	//
	// Do the SSAO/SSGI...
	//
	shaderProgram_t *shader = &tr.ssgi1Shader;

	if (r_ssgi->integer >= 7)
		shader = &tr.ssgi7Shader;
	else if (r_ssgi->integer >= 6)
		shader = &tr.ssgi6Shader;
	else if (r_ssgi->integer >= 5)
		shader = &tr.ssgi5Shader;
	else if (r_ssgi->integer >= 4)
		shader = &tr.ssgi4Shader;
	else if (r_ssgi->integer >= 3)
		shader = &tr.ssgi3Shader;
	else if (r_ssgi->integer >= 2)
		shader = &tr.ssgi2Shader;

	GLSL_BindProgram(shader);

	GLSL_SetUniformInt(shader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
	GLSL_SetUniformInt(shader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);
	GLSL_SetUniformInt(shader, UNIFORM_NORMALMAP, TB_NORMALMAP);
	GL_BindToTMU(tr.normalDetailedImage, TB_NORMALMAP);
	//GL_BindToTMU(tr.normalImage, TB_NORMALMAP);
	GLSL_SetUniformInt(shader, UNIFORM_DELUXEMAP, TB_DELUXEMAP);
	GL_BindToTMU(tr.anamorphicRenderFBOImage[2], TB_DELUXEMAP);
	GLSL_SetUniformInt(shader, UNIFORM_SPECULARMAP, TB_SPECULARMAP);
	GL_BindToTMU(tr.randomImage, TB_SPECULARMAP);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(shader, UNIFORM_DIMENSIONS, screensize);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}

	{
		vec4_t viewInfo;
		//float zmax = backEnd.viewParms.zFar;
		float zmax = 2048.0;//backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		//VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(shader, UNIFORM_VIEWINFO, viewInfo);
	}

	
	/*{
		vec4_t local0;
		local0[0] = r_testvalue1->value;
		local0[1] = 0.0;
		local0[2] = 0.0;
		local0[3] = 0.0;

		GLSL_SetUniformVec4(shader, UNIFORM_LOCAL0, local0);

		//ri->Printf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
	}*/
	

	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, shader, color, 0);
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

	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
	GLSL_SetUniformInt(&tr.testshaderShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

	GLSL_SetUniformMatrix16(&tr.testshaderShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	{
		vec4_t local0;
		VectorSet4(local0, r_testvalue0->value, r_testvalue1->value, 0.0, 0.0);
		GLSL_SetUniformVec4(&tr.testshaderShader, UNIFORM_LOCAL0, local0);
	}

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.testshaderShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;
		float zmax = 2048.0;//backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(&tr.testshaderShader, UNIFORM_VIEWINFO, viewInfo);
	}
	
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.testshaderShader, color, 0);
}

void RB_FastBlur(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.fastBlurShader);

	GLSL_SetUniformInt(&tr.fastBlurShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
	GLSL_SetUniformInt(&tr.fastBlurShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

	GLSL_SetUniformMatrix16(&tr.fastBlurShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.fastBlurShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;
		float zmax = 2048.0;//3072.0;//backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(&tr.fastBlurShader, UNIFORM_VIEWINFO, viewInfo);
	}
	
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.fastBlurShader, color, 0);
}

void RB_DistanceBlur(FBO_t *hdrFbo, vec4i_t hdrBox, FBO_t *ldrFbo, vec4i_t ldrBox)
{
	vec4_t color;

	// bloom
	color[0] =
		color[1] =
		color[2] = pow(2, r_cameraExposure->value);
	color[3] = 1.0f;

	GLSL_BindProgram(&tr.distanceBlurShader);

	GLSL_SetUniformInt(&tr.distanceBlurShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP);
	GL_BindToTMU(hdrFbo->colorImage[0], TB_LEVELSMAP);
	GLSL_SetUniformInt(&tr.distanceBlurShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP);
	GL_BindToTMU(tr.renderDepthImage, TB_LIGHTMAP);

	GLSL_SetUniformMatrix16(&tr.distanceBlurShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	{
		vec2_t screensize;
		screensize[0] = glConfig.vidWidth * r_superSampleMultiplier->value;
		screensize[1] = glConfig.vidHeight * r_superSampleMultiplier->value;

		GLSL_SetUniformVec2(&tr.distanceBlurShader, UNIFORM_DIMENSIONS, screensize);
	}

	{
		vec4_t viewInfo;
		float zmax = 2048.0;//3072.0;//backEnd.viewParms.zFar;
		float ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
		float xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
		float zmin = r_znear->value;
		VectorSet4(viewInfo, zmin, zmax, zmax / zmin, 0.0);
		GLSL_SetUniformVec4(&tr.distanceBlurShader, UNIFORM_VIEWINFO, viewInfo);
	}
	
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.distanceBlurShader, color, 0);
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
	
	FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.fxaaShader, color, 0);
}
