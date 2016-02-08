/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_shade.c

#include "tr_local.h" 

/*

  THIS ENTIRE FILE IS BACK END

  This file deals with applying shaders to surface data in the tess struct.
*/

color4ub_t	styleColors[MAX_LIGHT_STYLES];

extern void RB_DrawSurfaceSprites( shaderStage_t *stage, shaderCommands_t *input);

/*
==================
R_DrawElements

==================
*/

void R_DrawElementsVBO( int numIndexes, glIndex_t firstIndex, glIndex_t minIndex, glIndex_t maxIndex )
{
	if (r_tesselation->integer)
	{
		GLint MaxPatchVertices = 0;
		qglGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
		//printf("Max supported patch vertices %d\n", MaxPatchVertices);	
		qglPatchParameteri(GL_PATCH_VERTICES, 3);
		qglDrawRangeElements(GL_PATCHES, minIndex, maxIndex, numIndexes, GL_INDEX_TYPE, BUFFER_OFFSET(firstIndex * sizeof(glIndex_t)));
	}
	else
		qglDrawRangeElements(GL_TRIANGLES, minIndex, maxIndex, numIndexes, GL_INDEX_TYPE, BUFFER_OFFSET(firstIndex * sizeof(glIndex_t)));
}


static void R_DrawMultiElementsVBO( int multiDrawPrimitives, glIndex_t *multiDrawMinIndex, glIndex_t *multiDrawMaxIndex, 
	GLsizei *multiDrawNumIndexes, glIndex_t **multiDrawFirstIndex)
{
	if (r_tesselation->integer)
	{
		GLint MaxPatchVertices = 0;
		qglGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
		//printf("Max supported patch vertices %d\n", MaxPatchVertices);	
		qglPatchParameteri(GL_PATCH_VERTICES, 3);
		qglMultiDrawElements(GL_PATCHES, multiDrawNumIndexes, GL_INDEX_TYPE, (const GLvoid **)multiDrawFirstIndex, multiDrawPrimitives);
	}
	else
		qglMultiDrawElements(GL_TRIANGLES, multiDrawNumIndexes, GL_INDEX_TYPE, (const GLvoid **)multiDrawFirstIndex, multiDrawPrimitives);
}


/*
=============================================================

SURFACE SHADERS

=============================================================
*/

shaderCommands_t	tess;


/*
=================
R_BindAnimatedImageToTMU

=================
*/
void R_BindAnimatedImageToTMU( textureBundle_t *bundle, int tmu ) {
	int		index;

	if ( bundle->isVideoMap ) {
		int oldtmu = glState.currenttmu;
		GL_SelectTexture(tmu);
		ri->CIN_RunCinematic(bundle->videoMapHandle);
		ri->CIN_UploadCinematic(bundle->videoMapHandle);
		GL_SelectTexture(oldtmu);
		return;
	}

	if ( bundle->numImageAnimations <= 1 ) {
		GL_BindToTMU( bundle->image[0], tmu);
		return;
	}

	if (backEnd.currentEntity->e.renderfx & RF_SETANIMINDEX )
	{
		index = backEnd.currentEntity->e.skinNum;
	}
	else
	{
		// it is necessary to do this messy calc to make sure animations line up
		// exactly with waveforms of the same frequency
		index = Q_ftol( tess.shaderTime * bundle->imageAnimationSpeed * FUNCTABLE_SIZE );
		index >>= FUNCTABLE_SIZE2;

		if ( index < 0 ) {
			index = 0;	// may happen with shader time offsets
		}
	}

	if ( bundle->oneShotAnimMap )
	{
		if ( index >= bundle->numImageAnimations )
		{
			// stick on last frame
			index = bundle->numImageAnimations - 1;
		}
	}
	else
	{
		// loop
		index %= bundle->numImageAnimations;
	}

	GL_BindToTMU( bundle->image[ index ], tmu );
}


/*
================
DrawTris

Draws triangle outlines for debugging
================
*/
static void DrawTris (shaderCommands_t *input) {
	GL_Bind( tr.whiteImage );

	GL_State( GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE );
	qglDepthRange( 0, 0 );

	{
		shaderProgram_t *sp = &tr.textureColorShader;
		vec4_t color;

		GLSL_VertexAttribsState(ATTR_POSITION);
		GLSL_BindProgram(sp);
		
		GLSL_SetUniformMatrix16(sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
		VectorSet4(color, 1, 1, 1, 1);
		GLSL_SetUniformVec4(sp, UNIFORM_COLOR, color);

		if (input->multiDrawPrimitives)
		{
			R_DrawMultiElementsVBO(input->multiDrawPrimitives, input->multiDrawMinIndex, input->multiDrawMaxIndex, input->multiDrawNumIndexes, input->multiDrawFirstIndex);
		}
		else
		{
			R_DrawElementsVBO(input->numIndexes, input->firstIndex, input->minIndex, input->maxIndex);
		}
	}

	qglDepthRange( 0, 1 );
}


/*
================
DrawNormals

Draws vertex normals for debugging
================
*/
static void DrawNormals (shaderCommands_t *input) {
	//FIXME: implement this
}

/*
==============
RB_BeginSurface

We must set some things up before beginning any tesselation,
because a surface may be forced to perform a RB_End due
to overflow.
==============
*/
void RB_BeginSurface( shader_t *shader, int fogNum, int cubemapIndex ) {

	shader_t *state = (shader->remappedShader) ? shader->remappedShader : shader;

	tess.numIndexes = 0;
	tess.firstIndex = 0;
	tess.numVertexes = 0;
	tess.multiDrawPrimitives = 0;
	tess.shader = state;
	tess.fogNum = fogNum;
	tess.cubemapIndex = cubemapIndex;
	//tess.dlightBits = 0;		// will be OR'd in by surface functions
	//tess.pshadowBits = 0;       // will be OR'd in by surface functions
	tess.xstages = state->stages;
	tess.numPasses = state->numUnfoggedPasses;
	tess.currentStageIteratorFunc = state->optimalStageIteratorFunc;
	tess.useInternalVBO = qtrue;

	tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
	if (tess.shader->clampTime && tess.shaderTime >= tess.shader->clampTime) {
		tess.shaderTime = tess.shader->clampTime;
	}

	if (backEnd.viewParms.flags & VPF_SHADOWMAP)
	{
		tess.currentStageIteratorFunc = RB_StageIteratorGeneric;
	}
}



extern float EvalWaveForm( const waveForm_t *wf );
extern float EvalWaveFormClamped( const waveForm_t *wf );


static void ComputeTexMods( shaderStage_t *pStage, int bundleNum, float *outMatrix, float *outOffTurb)
{
	int tm;
	float matrix[6], currentmatrix[6];
	textureBundle_t *bundle = &pStage->bundle[bundleNum];

	matrix[0] = 1.0f; matrix[2] = 0.0f; matrix[4] = 0.0f;
	matrix[1] = 0.0f; matrix[3] = 1.0f; matrix[5] = 0.0f;

	currentmatrix[0] = 1.0f; currentmatrix[2] = 0.0f; currentmatrix[4] = 0.0f;
	currentmatrix[1] = 0.0f; currentmatrix[3] = 1.0f; currentmatrix[5] = 0.0f;

	outMatrix[0] = 1.0f; outMatrix[2] = 0.0f;
	outMatrix[1] = 0.0f; outMatrix[3] = 1.0f;

	outOffTurb[0] = 0.0f; outOffTurb[1] = 0.0f; outOffTurb[2] = 0.0f; outOffTurb[3] = 0.0f;

	for ( tm = 0; tm < bundle->numTexMods ; tm++ ) {
		switch ( bundle->texMods[tm].type )
		{
			
		case TMOD_NONE:
			tm = TR_MAX_TEXMODS;		// break out of for loop
			break;

		case TMOD_TURBULENT:
			RB_CalcTurbulentFactors(&bundle->texMods[tm].wave, &outOffTurb[2], &outOffTurb[3]);
			break;

		case TMOD_ENTITY_TRANSLATE:
			RB_CalcScrollTexMatrix( backEnd.currentEntity->e.shaderTexCoord, matrix );
			break;

		case TMOD_SCROLL:
			RB_CalcScrollTexMatrix( bundle->texMods[tm].scroll,
									 matrix );
			break;

		case TMOD_SCALE:
			RB_CalcScaleTexMatrix( bundle->texMods[tm].scale,
								  matrix );
			break;
		
		case TMOD_STRETCH:
			RB_CalcStretchTexMatrix( &bundle->texMods[tm].wave, 
								   matrix );
			break;

		case TMOD_TRANSFORM:
			RB_CalcTransformTexMatrix( &bundle->texMods[tm],
									 matrix );
			break;

		case TMOD_ROTATE:
			RB_CalcRotateTexMatrix( bundle->texMods[tm].rotateSpeed,
									matrix );
			break;

		default:
			ri->Error( ERR_DROP, "ERROR: unknown texmod '%d' in shader '%s'", bundle->texMods[tm].type, tess.shader->name );
			break;
		}

		switch ( bundle->texMods[tm].type )
		{	
		case TMOD_NONE:
		case TMOD_TURBULENT:
		default:
			break;

		case TMOD_ENTITY_TRANSLATE:
		case TMOD_SCROLL:
		case TMOD_SCALE:
		case TMOD_STRETCH:
		case TMOD_TRANSFORM:
		case TMOD_ROTATE:
			outMatrix[0] = matrix[0] * currentmatrix[0] + matrix[2] * currentmatrix[1];
			outMatrix[1] = matrix[1] * currentmatrix[0] + matrix[3] * currentmatrix[1];

			outMatrix[2] = matrix[0] * currentmatrix[2] + matrix[2] * currentmatrix[3];
			outMatrix[3] = matrix[1] * currentmatrix[2] + matrix[3] * currentmatrix[3];

			outOffTurb[0] = matrix[0] * currentmatrix[4] + matrix[2] * currentmatrix[5] + matrix[4];
			outOffTurb[1] = matrix[1] * currentmatrix[4] + matrix[3] * currentmatrix[5] + matrix[5];

			currentmatrix[0] = outMatrix[0];
			currentmatrix[1] = outMatrix[1];
			currentmatrix[2] = outMatrix[2];
			currentmatrix[3] = outMatrix[3];
			currentmatrix[4] = outOffTurb[0];
			currentmatrix[5] = outOffTurb[1];
			break;
		}
	}
}


static void ComputeDeformValues(int *deformGen, vec5_t deformParams)
{
	// u_DeformGen
	*deformGen = DGEN_NONE;
	if(!ShaderRequiresCPUDeforms(tess.shader))
	{
		deformStage_t  *ds;

		// only support the first one
		ds = &tess.shader->deforms[0];

		switch (ds->deformation)
		{
			case DEFORM_WAVE:
				*deformGen = ds->deformationWave.func;

				deformParams[0] = ds->deformationWave.base;
				deformParams[1] = ds->deformationWave.amplitude;
				deformParams[2] = ds->deformationWave.phase;
				deformParams[3] = ds->deformationWave.frequency;
				deformParams[4] = ds->deformationSpread;
				break;

			case DEFORM_BULGE:
				*deformGen = DGEN_BULGE;

				deformParams[0] = 0;
				deformParams[1] = ds->bulgeHeight; // amplitude
				deformParams[2] = ds->bulgeWidth;  // phase
				deformParams[3] = ds->bulgeSpeed;  // frequency
				deformParams[4] = 0;
				break;

			default:
				break;
		}
	}
}

static void ComputeShaderColors( shaderStage_t *pStage, vec4_t baseColor, vec4_t vertColor, int blend, colorGen_t *forceRGBGen, alphaGen_t *forceAlphaGen )
{
	colorGen_t rgbGen = pStage->rgbGen;
	alphaGen_t alphaGen = pStage->alphaGen;

	baseColor[0] =  
   	baseColor[1] = 
   	baseColor[2] = 
   	baseColor[3] = 1.0f; 
   	
   	vertColor[0] = 
   	vertColor[1] = 
   	vertColor[2] = 
   	vertColor[3] = 0.0f;

	if ( forceRGBGen != NULL && *forceRGBGen != CGEN_BAD )
	{
		rgbGen = *forceRGBGen;
	}

	if ( forceAlphaGen != NULL && *forceAlphaGen != AGEN_IDENTITY )
	{
		alphaGen = *forceAlphaGen;
	}

	switch ( rgbGen )
	{
		case CGEN_IDENTITY_LIGHTING:
			baseColor[0] = 
			baseColor[1] =
			baseColor[2] = tr.identityLight;
			break;
		case CGEN_EXACT_VERTEX:
		case CGEN_EXACT_VERTEX_LIT:
			baseColor[0] = 
			baseColor[1] =
			baseColor[2] = 
			baseColor[3] = 0.0f;

			vertColor[0] =
			vertColor[1] =
			vertColor[2] = 
			vertColor[3] = 1.0f;
			break;
		case CGEN_CONST:
			baseColor[0] = pStage->constantColor[0] / 255.0f;
			baseColor[1] = pStage->constantColor[1] / 255.0f;
			baseColor[2] = pStage->constantColor[2] / 255.0f;
			baseColor[3] = pStage->constantColor[3] / 255.0f;
			break;
		case CGEN_VERTEX:
			baseColor[0] = 
			baseColor[1] =
			baseColor[2] =
			baseColor[3] = 0.0f;

			vertColor[0] =
			vertColor[1] =
			vertColor[2] = tr.identityLight;
			vertColor[3] = 1.0f;
			break;
		case CGEN_VERTEX_LIT:
			baseColor[0] = 
			baseColor[1] =
			baseColor[2] = 
			baseColor[3] = 0.0f;

			vertColor[0] =
			vertColor[1] =
			vertColor[2] = 
			vertColor[3] = tr.identityLight;
			break;
		case CGEN_ONE_MINUS_VERTEX:
			baseColor[0] = 
			baseColor[1] =
			baseColor[2] = tr.identityLight;

			vertColor[0] =
			vertColor[1] =
			vertColor[2] = -tr.identityLight;
			break;
		case CGEN_FOG:
			{
				if (!r_fog->integer)
					break;

				fog_t		*fog;

				fog = tr.world->fogs + tess.fogNum;

				baseColor[0] = ((unsigned char *)(&fog->colorInt))[0] / 255.0f;
				baseColor[1] = ((unsigned char *)(&fog->colorInt))[1] / 255.0f;
				baseColor[2] = ((unsigned char *)(&fog->colorInt))[2] / 255.0f;
				baseColor[3] = ((unsigned char *)(&fog->colorInt))[3] / 255.0f;
			}
			break;
		case CGEN_WAVEFORM:
			baseColor[0] = 
			baseColor[1] = 
			baseColor[2] = RB_CalcWaveColorSingle( &pStage->rgbWave );
			break;
		case CGEN_ENTITY:
		case CGEN_LIGHTING_DIFFUSE_ENTITY:
			if (backEnd.currentEntity)
			{
				baseColor[0] = ((unsigned char *)backEnd.currentEntity->e.shaderRGBA)[0] / 255.0f;
				baseColor[1] = ((unsigned char *)backEnd.currentEntity->e.shaderRGBA)[1] / 255.0f;
				baseColor[2] = ((unsigned char *)backEnd.currentEntity->e.shaderRGBA)[2] / 255.0f;
				baseColor[3] = ((unsigned char *)backEnd.currentEntity->e.shaderRGBA)[3] / 255.0f;

				if ( alphaGen == AGEN_IDENTITY &&
					backEnd.currentEntity->e.shaderRGBA[3] == 255 )
				{
					alphaGen = AGEN_SKIP;
				}
			}
			break;
		case CGEN_ONE_MINUS_ENTITY:
			if (backEnd.currentEntity)
			{
				baseColor[0] = 1.0f - ((unsigned char *)backEnd.currentEntity->e.shaderRGBA)[0] / 255.0f;
				baseColor[1] = 1.0f - ((unsigned char *)backEnd.currentEntity->e.shaderRGBA)[1] / 255.0f;
				baseColor[2] = 1.0f - ((unsigned char *)backEnd.currentEntity->e.shaderRGBA)[2] / 255.0f;
				baseColor[3] = 1.0f - ((unsigned char *)backEnd.currentEntity->e.shaderRGBA)[3] / 255.0f;
			}
			break;
		case CGEN_LIGHTMAPSTYLE:
			VectorScale4 (styleColors[pStage->lightmapStyle], 1.0f / 255.0f, baseColor);
			break;
		case CGEN_IDENTITY:
		case CGEN_LIGHTING_DIFFUSE:
		case CGEN_BAD:
			break;
	}

	//
	// alphaGen
	//
	switch ( alphaGen )
	{
		case AGEN_SKIP:
			break;
		case AGEN_CONST:
			if ( rgbGen != CGEN_CONST ) {
				baseColor[3] = pStage->constantColor[3] / 255.0f;
				vertColor[3] = 0.0f;
			}
			break;
		case AGEN_WAVEFORM:
			baseColor[3] = RB_CalcWaveAlphaSingle( &pStage->alphaWave );
			vertColor[3] = 0.0f;
			break;
		case AGEN_ENTITY:
			if (backEnd.currentEntity)
			{
				baseColor[3] = ((unsigned char *)backEnd.currentEntity->e.shaderRGBA)[3] / 255.0f;
			}
			vertColor[3] = 0.0f;
			break;
		case AGEN_ONE_MINUS_ENTITY:
			if (backEnd.currentEntity)
			{
				baseColor[3] = 1.0f - ((unsigned char *)backEnd.currentEntity->e.shaderRGBA)[3] / 255.0f;
			}
			vertColor[3] = 0.0f;
			break;
		case AGEN_VERTEX:
			if ( rgbGen != CGEN_VERTEX ) {
				baseColor[3] = 0.0f;
				vertColor[3] = 1.0f;
			}
			break;
		case AGEN_ONE_MINUS_VERTEX:
			baseColor[3] = 1.0f;
			vertColor[3] = -1.0f;
			break;
		case AGEN_IDENTITY:
		case AGEN_LIGHTING_SPECULAR:
		case AGEN_PORTAL:
			// Done entirely in vertex program
			baseColor[3] = 1.0f;
			vertColor[3] = 0.0f;
			break;
	}

	if ( forceAlphaGen != NULL )
	{
		*forceAlphaGen = alphaGen;
	}

	if ( forceRGBGen != NULL )
	{
		*forceRGBGen = rgbGen;
	}
	
	// multiply color by overbrightbits if this isn't a blend
	if (tr.overbrightBits 
	 && !((blend & GLS_SRCBLEND_BITS) == GLS_SRCBLEND_DST_COLOR)
	 && !((blend & GLS_SRCBLEND_BITS) == GLS_SRCBLEND_ONE_MINUS_DST_COLOR)
	 && !((blend & GLS_DSTBLEND_BITS) == GLS_DSTBLEND_SRC_COLOR)
	 && !((blend & GLS_DSTBLEND_BITS) == GLS_DSTBLEND_ONE_MINUS_SRC_COLOR))
	{
		float scale = 1 << tr.overbrightBits;

		baseColor[0] *= scale;
		baseColor[1] *= scale;
		baseColor[2] *= scale;
		vertColor[0] *= scale;
		vertColor[1] *= scale;
		vertColor[2] *= scale;
	}

	// FIXME: find some way to implement this.
#if 0
	// if in greyscale rendering mode turn all color values into greyscale.
	if(r_greyscale->integer)
	{
		int scale;
		
		for(i = 0; i < tess.numVertexes; i++)
		{
			scale = (tess.svars.colors[i][0] + tess.svars.colors[i][1] + tess.svars.colors[i][2]) / 3;
			tess.svars.colors[i][0] = tess.svars.colors[i][1] = tess.svars.colors[i][2] = scale;
		}
	}
#endif
}


static void ComputeFogValues(vec4_t fogDistanceVector, vec4_t fogDepthVector, float *eyeT)
{
	// from RB_CalcFogTexCoords()
	fog_t  *fog;
	vec3_t  local;

	if (!tess.fogNum)
		return;

	if (!r_fog->integer)
		return;

	fog = tr.world->fogs + tess.fogNum;

	VectorSubtract( backEnd.ori.origin, backEnd.viewParms.ori.origin, local );
	fogDistanceVector[0] = -backEnd.ori.modelMatrix[2];
	fogDistanceVector[1] = -backEnd.ori.modelMatrix[6];
	fogDistanceVector[2] = -backEnd.ori.modelMatrix[10];
	fogDistanceVector[3] = DotProduct( local, backEnd.viewParms.ori.axis[0] );

	// scale the fog vectors based on the fog's thickness
	VectorScale4(fogDistanceVector, fog->tcScale, fogDistanceVector);

	// rotate the gradient vector for this orientation
	if ( fog->hasSurface ) {
		fogDepthVector[0] = fog->surface[0] * backEnd.ori.axis[0][0] + 
			fog->surface[1] * backEnd.ori.axis[0][1] + fog->surface[2] * backEnd.ori.axis[0][2];
		fogDepthVector[1] = fog->surface[0] * backEnd.ori.axis[1][0] + 
			fog->surface[1] * backEnd.ori.axis[1][1] + fog->surface[2] * backEnd.ori.axis[1][2];
		fogDepthVector[2] = fog->surface[0] * backEnd.ori.axis[2][0] + 
			fog->surface[1] * backEnd.ori.axis[2][1] + fog->surface[2] * backEnd.ori.axis[2][2];
		fogDepthVector[3] = -fog->surface[3] + DotProduct( backEnd.ori.origin, fog->surface );

		*eyeT = DotProduct( backEnd.ori.viewOrigin, fogDepthVector ) + fogDepthVector[3];
	} else {
		*eyeT = 1;	// non-surface fog always has eye inside
	}
}


static void ComputeFogColorMask( shaderStage_t *pStage, vec4_t fogColorMask )
{
	if (r_fog->integer)
	{
		switch(pStage->adjustColorsForFog)
		{
		case ACFF_MODULATE_RGB:
			fogColorMask[0] =
				fogColorMask[1] =
				fogColorMask[2] = 1.0f;
			fogColorMask[3] = 0.0f;
			break;
		case ACFF_MODULATE_ALPHA:
			fogColorMask[0] =
				fogColorMask[1] =
				fogColorMask[2] = 0.0f;
			fogColorMask[3] = 1.0f;
			break;
		case ACFF_MODULATE_RGBA:
			fogColorMask[0] =
				fogColorMask[1] =
				fogColorMask[2] =
				fogColorMask[3] = 1.0f;
			break;
		default:
			fogColorMask[0] =
				fogColorMask[1] =
				fogColorMask[2] =
				fogColorMask[3] = 0.0f;
			break;
		}
	}
	else
	{
		fogColorMask[0] =
			fogColorMask[1] =
			fogColorMask[2] =
			fogColorMask[3] = 0.0f;
	}
}

static void ProjectPshadowVBOGLSL( void ) {
#if 0
	int		l;
	vec3_t	origin;
	float	radius;

	int deformGen;
	vec5_t deformParams;

	shaderCommands_t *input = &tess;

	if ( !backEnd.refdef.num_pshadows ) {
		return;
	}
	
	ComputeDeformValues(&deformGen, deformParams);

	for ( l = 0 ; l < backEnd.refdef.num_pshadows ; l++ ) {
		pshadow_t	*ps;
		shaderProgram_t *sp;
		vec4_t vector;

		if ( !( tess.pshadowBits & ( 1 << l ) ) ) {
			continue;	// this surface definately doesn't have any of this shadow
		}

		ps = &backEnd.refdef.pshadows[l];
		VectorCopy( ps->lightOrigin, origin );
		radius = ps->lightRadius;

		sp = &tr.pshadowShader;

		GLSL_BindProgram(sp);

		GLSL_SetUniformMatrix16(sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

		VectorCopy(origin, vector);
		vector[3] = 1.0f;
		GLSL_SetUniformVec4(sp, UNIFORM_LIGHTORIGIN, vector);

		VectorScale(ps->lightViewAxis[0], 1.0f / ps->viewRadius, vector);
		GLSL_SetUniformVec3(sp, UNIFORM_LIGHTFORWARD, vector);

		VectorScale(ps->lightViewAxis[1], 1.0f / ps->viewRadius, vector);
		GLSL_SetUniformVec3(sp, UNIFORM_LIGHTRIGHT, vector);

		VectorScale(ps->lightViewAxis[2], 1.0f / ps->viewRadius, vector);
		GLSL_SetUniformVec3(sp, UNIFORM_LIGHTUP, vector);

		GLSL_SetUniformFloat(sp, UNIFORM_LIGHTRADIUS, radius);
	  
		// include GLS_DEPTHFUNC_EQUAL so alpha tested surfaces don't add light
		// where they aren't rendered
		GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHFUNC_EQUAL );

		GL_BindToTMU( tr.pshadowMaps[l], TB_DIFFUSEMAP );

		//
		// draw
		//

		if (input->multiDrawPrimitives)
		{
			R_DrawMultiElementsVBO(input->multiDrawPrimitives, input->multiDrawMinIndex, input->multiDrawMaxIndex, input->multiDrawNumIndexes, input->multiDrawFirstIndex);
		}
		else
		{
			R_DrawElementsVBO(input->numIndexes, input->firstIndex, input->minIndex, input->maxIndex);
		}

		backEnd.pc.c_totalIndexes += tess.numIndexes;
		//backEnd.pc.c_dlightIndexes += tess.numIndexes;
	}
#endif
}



/*
===================
RB_FogPass

Blends a fog texture on top of everything else
===================
*/
static void RB_FogPass( void ) {
	fog_t		*fog;
	vec4_t  color;
	vec4_t	fogDistanceVector, fogDepthVector = {0, 0, 0, 0};
	float	eyeT = 0;
	shaderProgram_t *sp;

	int deformGen;
	vec5_t deformParams;

	ComputeDeformValues(&deformGen, deformParams);

	{
		int index = 0;

		if (deformGen != DGEN_NONE)
			index |= FOGDEF_USE_DEFORM_VERTEXES;

		if (glState.vertexAnimation)
			index |= FOGDEF_USE_VERTEX_ANIMATION;

		if (glState.skeletalAnimation)
			index |= FOGDEF_USE_SKELETAL_ANIMATION;
		
		sp = &tr.fogShader[index];
	}

	backEnd.pc.c_fogDraws++;

	GLSL_BindProgram(sp);

	fog = tr.world->fogs + tess.fogNum;

	GLSL_SetUniformMatrix16(sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

	GLSL_SetUniformFloat(sp, UNIFORM_VERTEXLERP, glState.vertexAttribsInterpolation);
	
	GLSL_SetUniformInt(sp, UNIFORM_DEFORMGEN, deformGen);
	if (deformGen != DGEN_NONE)
	{
		GLSL_SetUniformFloat5(sp, UNIFORM_DEFORMPARAMS, deformParams);
		GLSL_SetUniformFloat(sp, UNIFORM_TIME, tess.shaderTime);
	}

	color[0] = ((unsigned char *)(&fog->colorInt))[0] / 255.0f;
	color[1] = ((unsigned char *)(&fog->colorInt))[1] / 255.0f;
	color[2] = ((unsigned char *)(&fog->colorInt))[2] / 255.0f;
	color[3] = ((unsigned char *)(&fog->colorInt))[3] / 255.0f;
	GLSL_SetUniformVec4(sp, UNIFORM_COLOR, color);

	ComputeFogValues(fogDistanceVector, fogDepthVector, &eyeT);

	GLSL_SetUniformVec4(sp, UNIFORM_FOGDISTANCE, fogDistanceVector);
	GLSL_SetUniformVec4(sp, UNIFORM_FOGDEPTH, fogDepthVector);
	GLSL_SetUniformFloat(sp, UNIFORM_FOGEYET, eyeT);

	if ( tess.shader->fogPass == FP_EQUAL ) {
		GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHFUNC_EQUAL );
	} else {
		GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
	}

	if (tess.multiDrawPrimitives)
	{
		shaderCommands_t *input = &tess;
		R_DrawMultiElementsVBO(input->multiDrawPrimitives, input->multiDrawMinIndex, input->multiDrawMaxIndex, input->multiDrawNumIndexes, input->multiDrawFirstIndex);
	}
	else
	{
		R_DrawElementsVBO(tess.numIndexes, tess.firstIndex, tess.minIndex, tess.maxIndex);
	}
}


static unsigned int RB_CalcShaderVertexAttribs( const shader_t *shader )
{
	unsigned int vertexAttribs = shader->vertexAttribs;

	if(glState.vertexAnimation)
	{
		vertexAttribs &= ~ATTR_COLOR;
		vertexAttribs |= ATTR_POSITION2;
		if (vertexAttribs & ATTR_NORMAL)
		{
			vertexAttribs |= ATTR_NORMAL2;
#ifdef USE_VERT_TANGENT_SPACE
			vertexAttribs |= ATTR_TANGENT2;
#endif
		}
	}

	if (glState.skeletalAnimation)
	{
		vertexAttribs |= ATTR_BONE_WEIGHTS;
		vertexAttribs |= ATTR_BONE_INDEXES;
	}

	return vertexAttribs;
}

static void UpdateTexCoords ( const shaderStage_t *stage )
{
	uint32_t updateAttribs = 0;
	if ( stage->bundle[0].image[0] != NULL )
	{
		switch (stage->bundle[0].tcGen)
		{
			case TCGEN_LIGHTMAP:
			case TCGEN_LIGHTMAP1:
			case TCGEN_LIGHTMAP2:
			case TCGEN_LIGHTMAP3:
			{
				int newLightmapIndex = stage->bundle[0].tcGen - TCGEN_LIGHTMAP + 1;
				if (newLightmapIndex != glState.vertexAttribsTexCoordOffset[0])
				{
					glState.vertexAttribsTexCoordOffset[0] = newLightmapIndex;
					updateAttribs |= ATTR_TEXCOORD0;
				}

				break;
			}

			case TCGEN_TEXTURE:
				if (glState.vertexAttribsTexCoordOffset[0] != 0)
				{
					glState.vertexAttribsTexCoordOffset[0] = 0;
					updateAttribs |= ATTR_TEXCOORD0;
				}
				break;

			default:
				break;
		}
	}

	if ( stage->bundle[TB_LIGHTMAP].image[0] != NULL )
	{
		switch (stage->bundle[TB_LIGHTMAP].tcGen)
		{
			case TCGEN_LIGHTMAP:
			case TCGEN_LIGHTMAP1:
			case TCGEN_LIGHTMAP2:
			case TCGEN_LIGHTMAP3:
			{
				int newLightmapIndex = stage->bundle[TB_LIGHTMAP].tcGen - TCGEN_LIGHTMAP + 1;
				if (newLightmapIndex != glState.vertexAttribsTexCoordOffset[1])
				{
					glState.vertexAttribsTexCoordOffset[1] = newLightmapIndex;
					updateAttribs |= ATTR_TEXCOORD1;
				}

				break;
			}

			case TCGEN_TEXTURE:
				if (glState.vertexAttribsTexCoordOffset[1] != 0)
				{
					glState.vertexAttribsTexCoordOffset[1] = 0;
					updateAttribs |= ATTR_TEXCOORD1;
				}
				break;

			default:
				break;
		}
	}

	if ( updateAttribs != 0 )
	{
		GLSL_UpdateTexCoordVertexAttribPointers (updateAttribs);
	}
}


int			overlaySwayTime = 0;
qboolean	overlaySwayDown = qfalse;
float		overlaySway = 0.0;

void RB_AdvanceOverlaySway ( void )
{
	if (overlaySwayTime > ri->Milliseconds()) 
		return;

	if (overlaySwayDown)
	{
		overlaySway -= 0.00016;

		if (overlaySway < 0.0)
		{
			overlaySway += 0.00032;
			overlaySwayDown = qfalse;
		}
	}
	else
	{
		overlaySway += 0.00016;

		if (overlaySway > 0.0016)
		{
			overlaySway -= 0.00032;
			overlaySwayDown = qtrue;
		}
	}

	overlaySwayTime = ri->Milliseconds() + 50;
}

void RB_SetMaterialBasedProperties(shaderProgram_t *sp, shaderStage_t *pStage)
{
	vec4_t	local1, local3, local4, local5;
	float	specularScale = 1.0;
	float	materialType = 0.0;
	float   parallaxScale = 1.0;
	float	cubemapScale = 0.0;
	float	isMetalic = 0.0;
	float	useSteepParallax = 0.0;
	float	hasOverlay = 0.0;
	float	doSway = 0.0;
	float	hasSteepMap = 0.0;

	if (pStage->bundle[TB_OVERLAYMAP].overlayLoaded 
		&& pStage->hasRealOverlayMap 
		&& pStage->bundle[TB_OVERLAYMAP].image[0] != tr.whiteImage)
	{
		hasOverlay = 1.0;
	}

	if (pStage->bundle[TB_STEEPMAP].steepMapLoaded
		&& pStage->hasRealSteepMap 
		&& pStage->bundle[TB_STEEPMAP].image[0] != tr.blackImage)
	{
		hasSteepMap = 1.0;
	}

	if (pStage->isWater && r_glslWater->integer)
	{
		specularScale = 1.5;
		materialType = (float)MATERIAL_WATER;
		parallaxScale = 2.0;
	}
	else
	{
		switch( tess.shader->surfaceFlags & MATERIAL_MASK )
		{
		case MATERIAL_WATER:			// 13			// light covering of water on a surface
			specularScale = 1.0;
			cubemapScale = 1.5;
			materialType = (float)MATERIAL_WATER;
			parallaxScale = 2.0;
			break;
		case MATERIAL_SHORTGRASS:		// 5			// manicured lawn
			specularScale = 0.53;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_SHORTGRASS;
			parallaxScale = 2.5;
			break;
		case MATERIAL_LONGGRASS:		// 6			// long jungle grass
			specularScale = 0.5;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_LONGGRASS;
			parallaxScale = 3.0;
			break;
		case MATERIAL_SAND:				// 8			// sandy beach
			specularScale = 0.0;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_SAND;
			parallaxScale = 2.5;
			break;
		case MATERIAL_CARPET:			// 27			// lush carpet
			specularScale = 0.0;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_CARPET;
			parallaxScale = 2.5;
			break;
		case MATERIAL_GRAVEL:			// 9			// lots of small stones
			specularScale = 0.0;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_GRAVEL;
			parallaxScale = 3.0;
			break;
		case MATERIAL_ROCK:				// 23			//
			specularScale = 0.0;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_ROCK;
			parallaxScale = 3.0;
			useSteepParallax = 1.0;
			break;
		case MATERIAL_TILES:			// 26			// tiled floor
			specularScale = 0.86;
			cubemapScale = 0.9;
			materialType = (float)MATERIAL_TILES;
			parallaxScale = 2.5;
			useSteepParallax = 1.0;
			break;
		case MATERIAL_SOLIDWOOD:		// 1			// freshly cut timber
			specularScale = 0.0;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_SOLIDWOOD;
			parallaxScale = 2.5;
			//useSteepParallax = 1.0;
			break;
		case MATERIAL_HOLLOWWOOD:		// 2			// termite infested creaky wood
			specularScale = 0.0;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_HOLLOWWOOD;
			parallaxScale = 2.5;
			//useSteepParallax = 1.0;
			break;
		case MATERIAL_SOLIDMETAL:		// 3			// solid girders
			specularScale = 0.98;
			cubemapScale = 0.98;
			materialType = (float)MATERIAL_SOLIDMETAL;
			parallaxScale = 0.005;
			isMetalic = 1.0;
			break;
		case MATERIAL_HOLLOWMETAL:		// 4			// hollow metal machines -- UQ1: Used for weapons to force lower parallax and high reflection...
			specularScale = 1.92;
			cubemapScale = 1.92;
			materialType = (float)MATERIAL_HOLLOWMETAL;
			parallaxScale = 2.0;
			isMetalic = 1.0;
			break;
		case MATERIAL_DRYLEAVES:		// 19			// dried up leaves on the floor
			specularScale = 0.0;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_DRYLEAVES;
			parallaxScale = 0.0;
			//useSteepParallax = 1.0;
			break;
		case MATERIAL_GREENLEAVES:		// 20			// fresh leaves still on a tree
			specularScale = 0.75;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_GREENLEAVES;
			parallaxScale = 0.0; // GreenLeaves should NEVER be parallaxed.. It's used for surfaces with an alpha channel and parallax screws it up...
			//useSteepParallax = 1.0;
			break;
		case MATERIAL_FABRIC:			// 21			// Cotton sheets
			specularScale = 0.48;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_FABRIC;
			parallaxScale = 2.5;
			break;
		case MATERIAL_CANVAS:			// 22			// tent material
			specularScale = 0.45;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_CANVAS;
			parallaxScale = 2.5;
			break;
		case MATERIAL_MARBLE:			// 12			// marble floors
			specularScale = 0.86;
			cubemapScale = 1.0;
			materialType = (float)MATERIAL_MARBLE;
			parallaxScale = 2.0;
			//useSteepParallax = 1.0;
			break;
		case MATERIAL_SNOW:				// 14			// freshly laid snow
			specularScale = 0.65;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_SNOW;
			parallaxScale = 3.0;
			useSteepParallax = 1.0;
			break;
		case MATERIAL_MUD:				// 17			// wet soil
			specularScale = 0.0;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_MUD;
			parallaxScale = 3.0;
			useSteepParallax = 1.0;
			break;
		case MATERIAL_DIRT:				// 7			// hard mud
			specularScale = 0.0;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_DIRT;
			parallaxScale = 3.0;
			useSteepParallax = 1.0;
			break;
		case MATERIAL_CONCRETE:			// 11			// hardened concrete pavement
			specularScale = 0.3;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_CONCRETE;
			parallaxScale = 3.0;
			//useSteepParallax = 1.0;
			break;
		case MATERIAL_FLESH:			// 16			// hung meat, corpses in the world
			specularScale = 0.2;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_FLESH;
			parallaxScale = 1.0;
			break;
		case MATERIAL_RUBBER:			// 24			// hard tire like rubber
			specularScale = 0.0;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_RUBBER;
			parallaxScale = 1.0;
			break;
		case MATERIAL_PLASTIC:			// 25			//
			specularScale = 0.88;
			cubemapScale = 0.5;
			materialType = (float)MATERIAL_PLASTIC;
			parallaxScale = 1.0;
			break;
		case MATERIAL_PLASTER:			// 28			// drywall style plaster
			specularScale = 0.4;
			cubemapScale = 0.0;
			materialType = (float)MATERIAL_PLASTER;
			parallaxScale = 2.0;
			break;
		case MATERIAL_SHATTERGLASS:		// 29			// glass with the Crisis Zone style shattering
			specularScale = 0.88;
			cubemapScale = 1.0;
			materialType = (float)MATERIAL_SHATTERGLASS;
			parallaxScale = 1.0;
			break;
		case MATERIAL_ARMOR:			// 30			// body armor
			specularScale = 0.4;
			cubemapScale = 2.0;
			materialType = (float)MATERIAL_ARMOR;
			parallaxScale = 2.0;
			isMetalic = 1.0;
			break;
		case MATERIAL_ICE:				// 15			// packed snow/solid ice
			specularScale = 0.9;
			cubemapScale = 0.8;
			parallaxScale = 2.0;
			materialType = (float)MATERIAL_ICE;
			useSteepParallax = 1.0;
			break;
		case MATERIAL_GLASS:			// 10			//
			specularScale = 0.95;
			cubemapScale = 1.0;
			materialType = (float)MATERIAL_GLASS;
			parallaxScale = 1.0;
			break;
		case MATERIAL_BPGLASS:			// 18			// bulletproof glass
			specularScale = 0.93;
			cubemapScale = 0.93;
			materialType = (float)MATERIAL_BPGLASS;
			parallaxScale = 1.0;
			break;
		case MATERIAL_COMPUTER:			// 31			// computers/electronic equipment
			specularScale = 0.92;
			cubemapScale = 0.92;
			materialType = (float)MATERIAL_COMPUTER;
			parallaxScale = 2.0;
			break;
		default:
			specularScale = 0.0;
			cubemapScale = 0.0;
			materialType = (float)0.0;
			parallaxScale = 1.0;
			break;
		}
	}

	// Shader overrides material...
	if (pStage->cubeMapScale > 0.0) cubemapScale = pStage->cubeMapScale;

	qboolean realNormalMap = qfalse;

	if (pStage->bundle[TB_NORMALMAP].image[0])
	{
		if (!pStage->bundle[TB_NORMALMAP].image[0]->generatedNormalMap)
		{
			realNormalMap = qtrue;
		}
	}

	switch( tess.shader->surfaceFlags & MATERIAL_MASK )
	{// Switch to avoid doing string checks on everything else...
		case MATERIAL_SHORTGRASS:		// 5			// manicured lawn
		case MATERIAL_LONGGRASS:		// 6			// long jungle grass
		//case MATERIAL_SOLIDWOOD:		// 1			// freshly cut timber
		//case MATERIAL_HOLLOWWOOD:		// 2			// termite infested creaky wood
		case MATERIAL_DRYLEAVES:		// 19			// dried up leaves on the floor
		case MATERIAL_GREENLEAVES:		// 20			// fresh leaves still on a tree
			if (pStage->bundle[TB_DIFFUSEMAP].image[0] 
				&& (StringContainsWord(pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName, "foliage/") || StringContainsWord(pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName, "foliages/")))
			{
				doSway = 0.7;
			}
			else if (pStage->bundle[TB_DIFFUSEMAP].image[0] 
				&& !StringContainsWord(pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName, "bark")
				&& !StringContainsWord(pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName, "giant_tree")
				&& (StringContainsWord(pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName, "yavin/tree") || StringContainsWord(pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName, "trees")))
			{
				doSway = 0.7;
			}
			break;
		default:
			break;
	}

	VectorSet4(local1, parallaxScale, (float)pStage->hasSpecular, specularScale, materialType);
	GLSL_SetUniformVec4(sp, UNIFORM_LOCAL1, local1);
	//GLSL_SetUniformVec4(sp, UNIFORM_LOCAL2, pStage->subsurfaceExtinctionCoefficient);
	VectorSet4(local3, 0.0/*pStage->subsurfaceRimScalar*/, 0.0/*pStage->subsurfaceMaterialThickness*/, 0.0/*pStage->subsurfaceSpecularPower*/, cubemapScale);
	GLSL_SetUniformVec4(sp, UNIFORM_LOCAL3, local3);
	VectorSet4(local4, (float)realNormalMap, isMetalic, 0.0/*(float)pStage->hasRealSubsurfaceMap*/, doSway);
	GLSL_SetUniformVec4(sp, UNIFORM_LOCAL4, local4);
	VectorSet4(local5, hasOverlay, overlaySway, r_blinnPhong->value, hasSteepMap);
	GLSL_SetUniformVec4(sp, UNIFORM_LOCAL5, local5);

	/*if (backEnd.viewParms.targetFbo == tr.renderCubeFbo)
	{
		VectorSet4(local5, 0.0, 0.0, 0.0, 0.0);
		GLSL_SetUniformVec4(sp, UNIFORM_LOCAL5, local5);
	}
	else
	{
		VectorSet4(local5, r_imageBasedLighting->value, 0.0, 0.0, 0.0);
		GLSL_SetUniformVec4(sp, UNIFORM_LOCAL5, local5);
	}*/

	//GLSL_SetUniformFloat(sp, UNIFORM_TIME, tess.shaderTime);
	GLSL_SetUniformFloat(sp, UNIFORM_TIME, backEnd.refdef.floatTime);
}

void RB_SetStageImageDimensions(shaderProgram_t *sp, shaderStage_t *pStage)
{
	vec2_t dimensions;

	dimensions[0] = pStage->bundle[0].image[0]->width;
	dimensions[1] = pStage->bundle[0].image[0]->height;

	if (pStage->bundle[TB_DIFFUSEMAP].image[0])
	{
		dimensions[0] = pStage->bundle[TB_DIFFUSEMAP].image[0]->width;
		dimensions[1] = pStage->bundle[TB_DIFFUSEMAP].image[0]->height;
	}
	else if (pStage->bundle[TB_NORMALMAP].image[0])
	{
		dimensions[0] = pStage->bundle[TB_NORMALMAP].image[0]->width;
		dimensions[1] = pStage->bundle[TB_NORMALMAP].image[0]->height;
	}
	else if (pStage->bundle[TB_SPECULARMAP].image[0])
	{
		dimensions[0] = pStage->bundle[TB_SPECULARMAP].image[0]->width;
		dimensions[1] = pStage->bundle[TB_SPECULARMAP].image[0]->height;
	}
	/*else if (pStage->bundle[TB_SUBSURFACEMAP].image[0])
	{
		dimensions[0] = pStage->bundle[TB_SUBSURFACEMAP].image[0]->width;
		dimensions[1] = pStage->bundle[TB_SUBSURFACEMAP].image[0]->height;
	}*/
	else if (pStage->bundle[TB_OVERLAYMAP].image[0])
	{
		dimensions[0] = pStage->bundle[TB_OVERLAYMAP].image[0]->width;
		dimensions[1] = pStage->bundle[TB_OVERLAYMAP].image[0]->height;
	}
	else if (pStage->bundle[TB_STEEPMAP].image[0])
	{
		dimensions[0] = pStage->bundle[TB_STEEPMAP].image[0]->width;
		dimensions[1] = pStage->bundle[TB_STEEPMAP].image[0]->height;
	}

	GLSL_SetUniformVec2(sp, UNIFORM_DIMENSIONS, dimensions);
}

extern qboolean modelviewProjectionChanged;
extern qboolean modelviewChanged;

extern image_t *skyImage;

float waveTime = 0.5;
float waveFreq = 0.1;

static void RB_IterateStagesGeneric( shaderCommands_t *input )
{
	vec4_t	fogDistanceVector, fogDepthVector = {0, 0, 0, 0};
	float	eyeT = 0;
	int		deformGen;
	vec5_t	deformParams;

	ComputeDeformValues(&deformGen, deformParams);

	ComputeFogValues(fogDistanceVector, fogDepthVector, &eyeT);

	for ( int stage = 0; stage < MAX_SHADER_STAGES; stage++ )
	{
		shaderStage_t *pStage = input->xstages[stage];
		shaderProgram_t *sp;
		vec4_t texMatrix;
		vec4_t texOffTurb;
		int stateBits;
		colorGen_t forceRGBGen = CGEN_BAD;
		alphaGen_t forceAlphaGen = AGEN_IDENTITY;
		qboolean isGeneric = qtrue;
		qboolean isWater = qfalse;
		qboolean isGrass = qfalse;
		qboolean multiPass = qtrue;
		int passNum = 0, passMax = 0;

		if ( !pStage )
		{
			break;
		}

		if ( pStage->isSurfaceSprite )
		{
#ifdef __SURFACESPRITES__
			if (!r_surfaceSprites->integer)
#endif //__SURFACESPRITES__
			{
				continue;
			}
		}

		if (backEnd.depthFill)
		{
			if (pStage->glslShaderGroup == tr.lightallShader)
			{
				int index = 0;

				if (backEnd.currentEntity && backEnd.currentEntity != &tr.worldEntity)
				{
					index |= LIGHTDEF_ENTITY;

					if (glState.vertexAnimation)
					{
						index |= LIGHTDEF_USE_VERTEX_ANIMATION;
					}

					if (glState.skeletalAnimation)
					{
						index |= LIGHTDEF_USE_SKELETAL_ANIMATION;
					}
				}

				if (pStage->stateBits & GLS_ATEST_BITS)
				{
					index |= LIGHTDEF_USE_TCGEN_AND_TCMOD;
				}

				sp = &pStage->glslShaderGroup[index];
				isGeneric = qfalse;
			}
			else
			{
				int shaderAttribs = 0;

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

				if (pStage->stateBits & GLS_ATEST_BITS)
				{
					shaderAttribs |= GENERICDEF_USE_TCGEN_AND_TCMOD;
				}

				sp = &tr.genericShader[shaderAttribs];
				isGeneric = qtrue;
			}
		}
		else if (pStage->glslShaderGroup == tr.lightallShader)
		{
			int index = pStage->glslShaderIndex;

			if (backEnd.currentEntity && backEnd.currentEntity != &tr.worldEntity)
			{
				index |= LIGHTDEF_ENTITY;

				if (glState.vertexAnimation)
				{
					index |= LIGHTDEF_USE_VERTEX_ANIMATION;
				}

				if (glState.skeletalAnimation)
				{
					index |= LIGHTDEF_USE_SKELETAL_ANIMATION;
				} 
			}

			if (r_sunlightMode->integer >= 2 && (backEnd.viewParms.flags & VPF_USESUNLIGHT) && (index & LIGHTDEF_LIGHTTYPE_MASK))
			{
				index |= LIGHTDEF_USE_SHADOWMAP;
			}
			/*else if (r_dlightMode->integer >= 2 && tr.refdef.num_dlights && (index & LIGHTDEF_LIGHTTYPE_MASK))
			{
				index |= LIGHTDEF_USE_SHADOWMAP;
			}*/

			if (r_lightmap->integer && index & LIGHTDEF_USE_LIGHTMAP)
			{
				index = LIGHTDEF_USE_LIGHTMAP;
			}

			/*
			if (index & LIGHTDEF_ENTITY|LIGHTDEF_USE_TCGEN_AND_TCMOD)
			{// Never ever use this f*cking combo...
				//index &= ~LIGHTDEF_ENTITY;
				index &= ~LIGHTDEF_USE_TCGEN_AND_TCMOD;
			}
			*/

			sp = &pStage->glslShaderGroup[index];
			isGeneric = qfalse;

			backEnd.pc.c_lightallDraws++;
		}
		else
		{
			sp = GLSL_GetGenericShaderProgram(stage);
			isGeneric = qtrue;

			backEnd.pc.c_genericDraws++;
		}

		if (pStage->isWater && r_glslWater->integer)
		{
			//if (stage <= 0) 
			//if (pStage->bundle[TB_DIFFUSEMAP].image[0])
			{
				sp = &tr.waterShader;
				pStage->glslShaderGroup = &tr.waterShader;
				GLSL_BindProgram(sp);
				
				GLSL_SetUniformMatrix16(sp, UNIFORM_MODELMATRIX, backEnd.ori.transformMatrix);
				GLSL_SetUniformMatrix16(sp, UNIFORM_INVEYEPROJECTIONMATRIX, glState.invEyeProjection);

				RB_SetMaterialBasedProperties(sp, pStage);

				GLSL_SetUniformFloat(sp, UNIFORM_TIME, tess.shaderTime);
				// Update wave variable
				//waveTime += waveFreq;
				//GLSL_SetUniformFloat(sp, UNIFORM_TIME, waveTime);

				GLSL_SetUniformInt(sp, UNIFORM_SPECULARMAP, TB_SPECULARMAP);
				GL_BindToTMU(tr.random2KImage, TB_SPECULARMAP);
				GLSL_SetUniformInt(sp, UNIFORM_SCREENDEPTHMAP, TB_LEVELSMAP);
				GL_BindToTMU(tr.renderDepthImage, TB_LEVELSMAP);

				isGeneric = qfalse;
				isWater = qtrue;
				//multiPass = qtrue;
				passMax = 8;
			}
			//else
			//{
			//	continue;
			//}
		}
//#define EXPERIMENTAL_GRASS // Need a  WorldViewMatrix - how the hell do I do that?
#ifdef EXPERIMENTAL_GRASS
		else if (( tess.shader->surfaceFlags & MATERIAL_MASK ) == MATERIAL_SHORTGRASS || ( tess.shader->surfaceFlags & MATERIAL_MASK ) == MATERIAL_LONGGRASS)
		{
			if (stage <= 0) 
			{
				sp = &tr.grassShader;
				pStage->glslShaderGroup = &tr.grassShader;
				GLSL_BindProgram(sp);
				GLSL_SetUniformFloat(sp, UNIFORM_TIME, tess.shaderTime);

				GLSL_SetUniformMatrix16(sp, UNIFORM_MODELMATRIX, backEnd.ori.transformMatrix);
				GLSL_SetUniformMatrix16(sp, UNIFORM_INVEYEPROJECTIONMATRIX, glState.invEyeProjection);
				
				RB_SetMaterialBasedProperties(sp, pStage);

				GLSL_SetUniformInt(sp, UNIFORM_SPECULARMAP, TB_SPECULARMAP);
				GL_BindToTMU(tr.grassMaskImage[0], TB_SPECULARMAP);
				GLSL_SetUniformInt(sp, UNIFORM_SCREENDEPTHMAP, TB_LEVELSMAP);
				GL_BindToTMU(tr.renderDepthImage, TB_LEVELSMAP);

				isGeneric = qfalse;
				isGrass = qtrue;
				multiPass = qtrue;
				passMax = 10;
			}
			else
			{
				continue;
			}
		}
#endif //EXPERIMENTAL_GRASS
		/*else if (( tess.shader->surfaceFlags & MATERIAL_MASK ) == MATERIAL_GREENLEAVES)
		{
			
		}*/
		else
		{
			if (!sp || !sp->program)
			{
				pStage->glslShaderGroup = tr.lightallShader;
				sp = &pStage->glslShaderGroup[0];
			}

			GLSL_BindProgram(sp);
		}

		while (1)
		{
			RB_SetMaterialBasedProperties(sp, pStage);
		
			stateBits = pStage->stateBits;

			if ( backEnd.currentEntity )
			{
				assert(backEnd.currentEntity->e.renderfx >= 0);

				if ( backEnd.currentEntity->e.renderfx & RF_DISINTEGRATE1 )
				{
					// we want to be able to rip a hole in the thing being disintegrated, and by doing the depth-testing it avoids some kinds of artefacts, but will probably introduce others?
					stateBits = GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHMASK_TRUE | GLS_ATEST_GE_192;
				}

				if ( backEnd.currentEntity->e.renderfx & RF_RGB_TINT )
				{//want to use RGBGen from ent
					forceRGBGen = CGEN_ENTITY;
				}

				if ( backEnd.currentEntity->e.renderfx & RF_FORCE_ENT_ALPHA )
				{
					stateBits = GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
					if ( backEnd.currentEntity->e.renderfx & RF_ALPHA_DEPTH )
					{ //depth write, so faces through the model will be stomped over by nearer ones. this works because
						//we draw RF_FORCE_ENT_ALPHA stuff after everything else, including standard alpha surfs.
						stateBits |= GLS_DEPTHMASK_TRUE;
					}
				}
			}

			//
			// UQ1: Split up uniforms by what is actually used...
			//

			{// UQ1: Used by both generic and lightall...
				RB_SetStageImageDimensions(sp, pStage);

				GLSL_SetUniformMatrix16(sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
				GLSL_SetUniformVec3(sp, UNIFORM_LOCALVIEWORIGIN, backEnd.ori.viewOrigin);
				GLSL_SetUniformFloat(sp, UNIFORM_VERTEXLERP, glState.vertexAttribsInterpolation);

				GLSL_SetUniformVec3(sp, UNIFORM_VIEWORIGIN, backEnd.viewParms.ori.origin);

				GLSL_SetUniformMatrix16(sp, UNIFORM_MODELMATRIX, backEnd.ori.transformMatrix);
				GLSL_SetUniformVec4(sp, UNIFORM_NORMALSCALE, pStage->normalScale);
				GLSL_SetUniformVec4(sp, UNIFORM_SPECULARSCALE, pStage->specularScale);


				if (glState.skeletalAnimation)
				{
					GLSL_SetUniformMatrix16(sp, UNIFORM_BONE_MATRICES, &glState.boneMatrices[0][0], glState.numBones);
				}

				GLSL_SetUniformInt(sp, UNIFORM_DEFORMGEN, deformGen);
				if (deformGen != DGEN_NONE)
				{
					GLSL_SetUniformFloat5(sp, UNIFORM_DEFORMPARAMS, deformParams);
					GLSL_SetUniformFloat(sp, UNIFORM_TIME, tess.shaderTime);
				}

				GLSL_SetUniformInt(sp, UNIFORM_TCGEN0, pStage->bundle[0].tcGen);
				if (pStage->bundle[0].tcGen == TCGEN_VECTOR)
				{
					vec3_t vec;

					VectorCopy(pStage->bundle[0].tcGenVectors[0], vec);
					GLSL_SetUniformVec3(sp, UNIFORM_TCGEN0VECTOR0, vec);
					VectorCopy(pStage->bundle[0].tcGenVectors[1], vec);
					GLSL_SetUniformVec3(sp, UNIFORM_TCGEN0VECTOR1, vec);
				}

				{
					vec4_t baseColor;
					vec4_t vertColor;

					ComputeShaderColors(pStage, baseColor, vertColor, stateBits, &forceRGBGen, &forceAlphaGen);

					if ((backEnd.refdef.colorScale != 1.0f) && !(backEnd.refdef.rdflags & RDF_NOWORLDMODEL))
					{
						// use VectorScale to only scale first three values, not alpha
						VectorScale(baseColor, backEnd.refdef.colorScale, baseColor);
						VectorScale(vertColor, backEnd.refdef.colorScale, vertColor);
					}

					if ( backEnd.currentEntity != NULL &&
						(backEnd.currentEntity->e.renderfx & RF_FORCE_ENT_ALPHA) )
					{
						vertColor[3] = backEnd.currentEntity->e.shaderRGBA[3] / 255.0f;
					}

					GLSL_SetUniformVec4(sp, UNIFORM_BASECOLOR, baseColor);
					GLSL_SetUniformVec4(sp, UNIFORM_VERTCOLOR, vertColor);
				}

				if (pStage->rgbGen == CGEN_LIGHTING_DIFFUSE ||
					pStage->rgbGen == CGEN_LIGHTING_DIFFUSE_ENTITY ||
					(pStage->isWater && r_glslWater->integer))
				{
					vec4_t vec;

					VectorScale(backEnd.currentEntity->ambientLight, 1.0f / 255.0f, vec);
					GLSL_SetUniformVec3(sp, UNIFORM_AMBIENTLIGHT, vec);

					VectorScale(backEnd.currentEntity->directedLight, 1.0f / 255.0f, vec);
					GLSL_SetUniformVec3(sp, UNIFORM_DIRECTEDLIGHT, vec);

					VectorCopy(backEnd.currentEntity->lightDir, vec);
					vec[3] = 0.0f;
					GLSL_SetUniformVec4(sp, UNIFORM_LIGHTORIGIN, vec);
					GLSL_SetUniformVec3(sp, UNIFORM_MODELLIGHTDIR, backEnd.currentEntity->modelLightDir);

					if (!isGeneric)
					{
						GLSL_SetUniformFloat(sp, UNIFORM_LIGHTRADIUS, 0.0f);
					}
				}

				ComputeTexMods( pStage, TB_DIFFUSEMAP, texMatrix, texOffTurb );
				GLSL_SetUniformVec4(sp, UNIFORM_DIFFUSETEXMATRIX, texMatrix);
				GLSL_SetUniformVec4(sp, UNIFORM_DIFFUSETEXOFFTURB, texOffTurb);
			}

			if (isGeneric)
			{// UQ1: Only generic uses these...
				if (pStage->alphaGen == AGEN_PORTAL)
				{
					GLSL_SetUniformFloat(sp, UNIFORM_PORTALRANGE, tess.shader->portalRange);
				}

				if (r_fog->integer)
				{
					if ( input->fogNum )
					{
						vec4_t fogColorMask;
						GLSL_SetUniformVec4(sp, UNIFORM_FOGDISTANCE, fogDistanceVector);
						GLSL_SetUniformVec4(sp, UNIFORM_FOGDEPTH, fogDepthVector);
						GLSL_SetUniformFloat(sp, UNIFORM_FOGEYET, eyeT);

						ComputeFogColorMask(pStage, fogColorMask);
						GLSL_SetUniformVec4(sp, UNIFORM_FOGCOLORMASK, fogColorMask);
					}
				}

				GLSL_SetUniformInt(sp, UNIFORM_COLORGEN, forceRGBGen);
				GLSL_SetUniformInt(sp, UNIFORM_ALPHAGEN, forceAlphaGen);
			}
			else
			{// UQ1: Only lightall uses these...
				//GLSL_SetUniformFloat(sp, UNIFORM_MAPLIGHTSCALE, backEnd.refdef.mapLightScale);

				//
				// testing cube map
				//
				if (!(tr.viewParms.flags & VPF_NOCUBEMAPS) && input->cubemapIndex && r_cubeMapping->integer)
				{
					vec4_t vec;

					GL_BindToTMU( tr.cubemaps[input->cubemapIndex - 1], TB_CUBEMAP);

					vec[0] = tr.cubemapOrigins[input->cubemapIndex - 1][0] - backEnd.viewParms.ori.origin[0];
					vec[1] = tr.cubemapOrigins[input->cubemapIndex - 1][1] - backEnd.viewParms.ori.origin[1];
					vec[2] = tr.cubemapOrigins[input->cubemapIndex - 1][2] - backEnd.viewParms.ori.origin[2];
					vec[3] = 1.0f;

					float dist = Distance(tr.refdef.vieworg, tr.cubemapOrigins[input->cubemapIndex - 1]);
					float mult = r_cubemapCullFalloffMult->value - (r_cubemapCullFalloffMult->value * 0.04);

					if (dist < r_cubemapCullRange->value)
					{// In range for full effect...
						GLSL_SetUniformFloat(sp, UNIFORM_CUBEMAPSTRENGTH, 1.0);
					}
					else if (dist >= r_cubemapCullRange->value && dist < r_cubemapCullRange->value * mult)
					{// Further scale the strength of the cubemap by the fade-out distance...
						float extraDist =		dist - r_cubemapCullRange->value;
						float falloffDist =		(r_cubemapCullRange->value * mult) - r_cubemapCullRange->value;
						float strength =		(falloffDist - extraDist) / falloffDist;

						strength = CLAMP(strength, 0.0, 1.0);
						GLSL_SetUniformFloat(sp, UNIFORM_CUBEMAPSTRENGTH, strength);
					}
					else
					{// Out of range completely...
						GLSL_SetUniformFloat(sp, UNIFORM_CUBEMAPSTRENGTH, 0.0);
					}
					
					VectorScale4(vec, 1.0f / 1000.0f, vec);

					GLSL_SetUniformVec4(sp, UNIFORM_CUBEMAPINFO, vec);
				}
			}

			//
			//
			//

			if (pStage->bundle[TB_STEEPMAP].image[0])
			{
				//ri->Printf(PRINT_WARNING, "Image bound to steep map %i x %i.\n", pStage->bundle[TB_STEEPMAP].image[0]->width, pStage->bundle[TB_STEEPMAP].image[0]->height);
				R_BindAnimatedImageToTMU( &pStage->bundle[TB_STEEPMAP], TB_STEEPMAP);
			}
			else
			{
				GL_BindToTMU( tr.whiteImage, TB_STEEPMAP );
			}

			/*if (pStage->bundle[TB_SUBSURFACEMAP].image[0])
			{
				R_BindAnimatedImageToTMU( &pStage->bundle[TB_SUBSURFACEMAP], TB_SUBSURFACEMAP);
			}
			else
			{
				GL_BindToTMU( tr.whiteImage, TB_SUBSURFACEMAP );
			}*/

			if (pStage->bundle[TB_OVERLAYMAP].image[0])
			{
				R_BindAnimatedImageToTMU( &pStage->bundle[TB_OVERLAYMAP], TB_OVERLAYMAP);
			}
			else
			{
				/*
				if (pStage->isWater && r_glslWater->integer)
				{
					if ( !skyImage ) 
					{
						//ri->Printf(PRINT_WARNING, "Have no skyImage!\n");
						GLSL_SetUniformInt(sp, UNIFORM_OVERLAYMAP, TB_OVERLAYMAP);
						GL_BindToTMU(tr.blackImage, TB_OVERLAYMAP);
						//vec4_t l0;
						//VectorSet4(l0, 0, 0, 0, 0);
						//GLSL_SetUniformVec4(sp, UNIFORM_LOCAL10, l0);
					}
					else
					{
						//ri->Printf(PRINT_WARNING, "Have skyImage! YAY!\n");
						GLSL_SetUniformInt(sp, UNIFORM_OVERLAYMAP, TB_OVERLAYMAP);
						GL_BindToTMU(skyImage, TB_OVERLAYMAP);
						
						//vec4_t l0;
						//VectorSet4(l0, skyImage->width, skyImage->height, 0, 0);
						//GLSL_SetUniformVec4(sp, UNIFORM_LOCAL10, l0);
					}
				}
				else*/
				{
					GL_BindToTMU( tr.blackImage, TB_OVERLAYMAP );
				}
			}

			//
			// do multitexture
			//
			if ( backEnd.depthFill )
			{
				if (!(pStage->stateBits & GLS_ATEST_BITS))
					GL_BindToTMU( tr.whiteImage, 0 );
				else if ( pStage->bundle[TB_COLORMAP].image[0] != 0 )
					R_BindAnimatedImageToTMU( &pStage->bundle[TB_COLORMAP], TB_COLORMAP );
			}
			else if ( !isGeneric && (pStage->glslShaderGroup == tr.lightallShader || (pStage->isWater && r_glslWater->integer) ) )
			{
				int i;
				vec4_t enableTextures;

				if (r_sunlightMode->integer && (backEnd.viewParms.flags & VPF_USESUNLIGHT) /*&& (pStage->glslShaderIndex & LIGHTDEF_LIGHTTYPE_MASK)*/)
				{
					GL_BindToTMU(tr.screenShadowImage, TB_SHADOWMAP);
					GLSL_SetUniformVec3(sp, UNIFORM_PRIMARYLIGHTAMBIENT, backEnd.refdef.sunAmbCol);
					GLSL_SetUniformVec3(sp, UNIFORM_PRIMARYLIGHTCOLOR,   backEnd.refdef.sunCol);
					GLSL_SetUniformVec4(sp, UNIFORM_PRIMARYLIGHTORIGIN,  backEnd.refdef.sunDir);
				}

				VectorSet4(enableTextures, 0, 0, 0, 0);
				if ((r_lightmap->integer == 1 || r_lightmap->integer == 2) && pStage->bundle[TB_LIGHTMAP].image[0])
				{
					for (i = 0; i < NUM_TEXTURE_BUNDLES; i++)
					{
						if (i == TB_LIGHTMAP)
							R_BindAnimatedImageToTMU( &pStage->bundle[TB_LIGHTMAP], i);
						else
							GL_BindToTMU( tr.whiteImage, i );
					}
				}
				else if (r_lightmap->integer == 3 && pStage->bundle[TB_DELUXEMAP].image[0])
				{
					for (i = 0; i < NUM_TEXTURE_BUNDLES; i++)
					{
						if (i == TB_LIGHTMAP)
							R_BindAnimatedImageToTMU( &pStage->bundle[TB_DELUXEMAP], i);
						else
							GL_BindToTMU( tr.whiteImage, i );
					}
				}
				else
				{
					qboolean light = (qboolean)((pStage->glslShaderIndex & LIGHTDEF_LIGHTTYPE_MASK) != 0);
					qboolean fastLight = (qboolean)!(r_normalMapping->integer || r_specularMapping->integer);

					if (pStage->bundle[TB_DIFFUSEMAP].image[0])
						R_BindAnimatedImageToTMU( &pStage->bundle[TB_DIFFUSEMAP], TB_DIFFUSEMAP);

					if (pStage->bundle[TB_LIGHTMAP].image[0])
						R_BindAnimatedImageToTMU( &pStage->bundle[TB_LIGHTMAP], TB_LIGHTMAP);

					// bind textures that are sampled and used in the glsl shader, and
					// bind whiteImage to textures that are sampled but zeroed in the glsl shader
					//
					// alternatives:
					//  - use the last bound texture
					//     -> costs more to sample a higher res texture then throw out the result
					//  - disable texture sampling in glsl shader with #ifdefs, as before
					//     -> increases the number of shaders that must be compiled
					//
					if ((light || (pStage->isWater && r_glslWater->integer) || pStage->hasRealNormalMap || pStage->hasSpecular /*|| pStage->hasRealSubsurfaceMap*/ || pStage->hasRealOverlayMap || pStage->hasRealSteepMap) && !fastLight)
					{
						if (r_normalMapping->integer
							&& !input->shader->isPortal
							&& !input->shader->isSky
							&& !pStage->glow
							&& !pStage->bundle[TB_DIFFUSEMAP].normalsLoaded2
							&& (!pStage->bundle[TB_NORMALMAP].image[0] || pStage->bundle[TB_NORMALMAP].image[0] == tr.whiteImage)
							&& pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName[0] 
							&& pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName[0] != '*'
							&& pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName[0] != '$'
							&& pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName[0] != '_'
							&& pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName[0] != '!'
							&& !(pStage->bundle[TB_DIFFUSEMAP].image[0]->flags & IMGFLAG_CUBEMAP)
							&& pStage->bundle[TB_DIFFUSEMAP].image[0]->type != IMGTYPE_NORMAL 
							&& pStage->bundle[TB_DIFFUSEMAP].image[0]->type != IMGTYPE_SPECULAR 
							/*&& pStage->bundle[TB_DIFFUSEMAP].image[0]->type != IMGTYPE_SUBSURFACE*/ 
							&& pStage->bundle[TB_DIFFUSEMAP].image[0]->type != IMGTYPE_OVERLAY 
							&& pStage->bundle[TB_DIFFUSEMAP].image[0]->type != IMGTYPE_STEEPMAP 
							// gfx dirs can be exempted I guess...
							&& !(r_disableGfxDirEnhancement->integer && StringContainsWord(pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName, "gfx/")))
						{// How did this happen??? Oh well, generate a normal map now...
							char imgname[64];
							//ri->Printf(PRINT_WARNING, "Realtime generating normal map for %s.\n", pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName);
							sprintf(imgname, "%s_n", pStage->bundle[TB_DIFFUSEMAP].image[0]->imgName);
							pStage->bundle[TB_NORMALMAP].image[0] = R_CreateNormalMapGLSL( imgname, NULL, pStage->bundle[TB_DIFFUSEMAP].image[0]->width, pStage->bundle[TB_DIFFUSEMAP].image[0]->height, pStage->bundle[TB_DIFFUSEMAP].image[0]->flags, pStage->bundle[TB_DIFFUSEMAP].image[0] );
							
							if (pStage->bundle[TB_NORMALMAP].image[0]) 
							{
								pStage->hasRealNormalMap = true;
								RB_SetMaterialBasedProperties(sp, pStage);

								if (pStage->normalScale[0] == 0 && pStage->normalScale[1] == 0 && pStage->normalScale[2] == 0)
								{
									VectorSet4(pStage->normalScale, r_baseNormalX->value, r_baseNormalY->value, 1.0f, r_baseParallax->value);
									GLSL_SetUniformVec4(sp, UNIFORM_NORMALSCALE, pStage->normalScale);
								}
							}

							pStage->bundle[TB_DIFFUSEMAP].normalsLoaded2 = qtrue;
						}

						if (pStage->bundle[TB_NORMALMAP].image[0])
						{
							R_BindAnimatedImageToTMU( &pStage->bundle[TB_NORMALMAP], TB_NORMALMAP);
							enableTextures[0] = 1.0f;
						}
						else if (r_normalMapping->integer)
						{
							GL_BindToTMU( tr.whiteImage, TB_NORMALMAP );
						}

						if (pStage->bundle[TB_DELUXEMAP].image[0])
						{
							R_BindAnimatedImageToTMU( &pStage->bundle[TB_DELUXEMAP], TB_DELUXEMAP);
							enableTextures[1] = 1.0f;
						}
						else if (r_deluxeMapping->integer)
						{
							GL_BindToTMU( tr.whiteImage, TB_DELUXEMAP );
						}

						if (pStage->bundle[TB_SPECULARMAP].image[0])
						{
							R_BindAnimatedImageToTMU( &pStage->bundle[TB_SPECULARMAP], TB_SPECULARMAP);
							enableTextures[2] = 1.0f;
						}
						else if (r_specularMapping->integer)
						{
							GL_BindToTMU( tr.whiteImage, TB_SPECULARMAP );
						}
					}

					enableTextures[3] = (r_cubeMapping->integer && !(tr.viewParms.flags & VPF_NOCUBEMAPS) && input->cubemapIndex) ? 1.0f : 0.0f;
				}

				GLSL_SetUniformVec4(sp, UNIFORM_ENABLETEXTURES, enableTextures);
			}
			else if ( pStage->bundle[1].image[0] != 0 )
			{
				R_BindAnimatedImageToTMU( &pStage->bundle[0], 0 );
				R_BindAnimatedImageToTMU( &pStage->bundle[1], 1 );
			}
			else 
			{
				//
				// set state
				//
				R_BindAnimatedImageToTMU( &pStage->bundle[0], 0 );
			}


			if (isWater)
			{
				vec4_t loc;
				VectorSet4(loc, (float)0.4, 6.0, 2.0, 0.2); // grassLength, grassLayer, wavespeed, wavesize
				GLSL_SetUniformVec4(sp, UNIFORM_LOCAL5, loc);
				//GLSL_SetUniformFloat(sp, UNIFORM_TIME, waveTime);
			}

#if 0
			if (isGrass)
			{
				vec4_t loc;
				VectorSet4(loc, (float)passNum, r_grassLength->value, r_grassWaveSpeed->value, r_grassWaveSize->value); // grassLength, grassLayer, wavespeed, wavesize
				GLSL_SetUniformVec4(sp, UNIFORM_LOCAL5, loc);

				GLSL_SetUniformInt(sp, UNIFORM_SPECULARMAP, TB_SPECULARMAP);
				GL_BindToTMU(tr.grassMaskImage[passNum], TB_SPECULARMAP);

				GLSL_SetUniformInt(sp, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
				GL_BindToTMU(tr.grassImage, TB_DIFFUSEMAP);
			}
#endif

			UpdateTexCoords (pStage);

			GL_State( stateBits );

			//
			// draw
			//
			if (input->multiDrawPrimitives)
			{
				R_DrawMultiElementsVBO(input->multiDrawPrimitives, input->multiDrawMinIndex, input->multiDrawMaxIndex, input->multiDrawNumIndexes, input->multiDrawFirstIndex);
			}
			else
			{
				R_DrawElementsVBO(input->numIndexes, input->firstIndex, input->minIndex, input->maxIndex);
			}


			passNum++;

			if (multiPass && passNum > passMax)
			{// Finished all passes...
				multiPass = qfalse;
			}

			if (!multiPass) 
			{
				break;
			}
		}

		if (pStage->isWater && r_glslWater->integer)
		{
			break;
		}
		
		// allow skipping out to show just lightmaps during development
		if ( r_lightmap->integer && ( pStage->bundle[0].isLightmap || pStage->bundle[1].isLightmap ) )
		{
			break;
		}

		if (backEnd.depthFill)
			break;
	}
}


static void RB_RenderShadowmap( shaderCommands_t *input )
{
	int deformGen;
	vec5_t deformParams;

	ComputeDeformValues(&deformGen, deformParams);

	{
		shaderProgram_t *sp = &tr.shadowmapShader;

		vec4_t vector;

		GLSL_BindProgram(sp);

		GLSL_SetUniformMatrix16(sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

		GLSL_SetUniformMatrix16(sp, UNIFORM_MODELMATRIX, backEnd.ori.transformMatrix);

		GLSL_SetUniformFloat(sp, UNIFORM_VERTEXLERP, glState.vertexAttribsInterpolation);

		GLSL_SetUniformInt(sp, UNIFORM_DEFORMGEN, deformGen);
		if (deformGen != DGEN_NONE)
		{
			GLSL_SetUniformFloat5(sp, UNIFORM_DEFORMPARAMS, deformParams);
			GLSL_SetUniformFloat(sp, UNIFORM_TIME, tess.shaderTime);
		}

		VectorCopy(backEnd.viewParms.ori.origin, vector);
		vector[3] = 1.0f;
		GLSL_SetUniformVec4(sp, UNIFORM_LIGHTORIGIN, vector);
		GLSL_SetUniformFloat(sp, UNIFORM_LIGHTRADIUS, backEnd.viewParms.zFar);

		GL_State( 0 );

		//
		// do multitexture
		//
		//if ( pStage->glslShaderGroup )
		{
			//
			// draw
			//

			if (input->multiDrawPrimitives)
			{
				R_DrawMultiElementsVBO(input->multiDrawPrimitives, input->multiDrawMinIndex, input->multiDrawMaxIndex, input->multiDrawNumIndexes, input->multiDrawFirstIndex);
			}
			else
			{
				R_DrawElementsVBO(input->numIndexes, input->firstIndex, input->minIndex, input->maxIndex);
			}
		}
	}
}


/*
** RB_StageIteratorGeneric
*/
void RB_StageIteratorGeneric( void )
{
	shaderCommands_t *input;
	unsigned int vertexAttribs = 0;

	input = &tess;
	
	if (!input->numVertexes || !input->numIndexes)
	{
		return;
	}

	if (tess.useInternalVBO)
	{
		RB_DeformTessGeometry();
	}

	vertexAttribs = RB_CalcShaderVertexAttribs( input->shader );

	if (tess.useInternalVBO)
	{
		RB_UpdateVBOs(vertexAttribs);
	}
	else
	{
		backEnd.pc.c_staticVboDraws++;
	}

	//
	// log this call
	//
	if ( r_logFile->integer ) 
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment( va("--- RB_StageIteratorGeneric( %s ) ---\n", tess.shader->name) );
	}

	//
	// set face culling appropriately
	//
	if ((backEnd.viewParms.flags & VPF_DEPTHSHADOW))
	{
		//GL_Cull( CT_TWO_SIDED );
		
		if (input->shader->cullType == CT_TWO_SIDED)
			GL_Cull( CT_TWO_SIDED );
		else if (input->shader->cullType == CT_FRONT_SIDED)
			GL_Cull( CT_BACK_SIDED );
		else
			GL_Cull( CT_FRONT_SIDED );
		
	}
	else
		GL_Cull( input->shader->cullType );

	// set polygon offset if necessary
	if ( input->shader->polygonOffset )
	{
		qglEnable( GL_POLYGON_OFFSET_FILL );
		qglPolygonOffset( r_offsetFactor->value, r_offsetUnits->value );
	}

	//
	// Set vertex attribs and pointers
	//
	GLSL_VertexAttribsState(vertexAttribs);

	//
	// UQ1: Set up any special shaders needed for this surface/contents type...
	//

	if ((tess.shader->isWater && r_glslWater->integer)
		|| (tess.shader->contentFlags & CONTENTS_WATER) 
		/*|| (tess.shader->contentFlags & CONTENTS_LAVA)*/ 
		|| (tess.shader->surfaceFlags & MATERIAL_MASK) == MATERIAL_WATER) 
	{
		if (input && input->xstages[0] && input->xstages[0]->isWater == 0 && r_glslWater->integer) // In case it is already set, no need looping more then once on the same shader...
		{
			int isWater = 1;

			if (tess.shader->contentFlags & CONTENTS_LAVA)
				isWater = 2;

			for ( int stage = 0; stage < MAX_SHADER_STAGES; stage++ )
			{
				if (input->xstages[stage])
				{
					input->xstages[stage]->isWater = isWater;
					//input->xstages[stage]->glslShaderGroup = tr.lightallShader;
				}
			}
		}
	}

	//
	// render depth if in depthfill mode
	//
	if (backEnd.depthFill)
	{
		RB_IterateStagesGeneric( input );

		//
		// reset polygon offset
		//
		if ( input->shader->polygonOffset )
		{
			qglDisable( GL_POLYGON_OFFSET_FILL );
		}

		return;
	}

	//
	// render shadowmap if in shadowmap mode
	//
	if (backEnd.viewParms.flags & VPF_SHADOWMAP)
	{
		if ( input->shader->sort == SS_OPAQUE )
		{
			RB_RenderShadowmap( input );
		}
		//
		// reset polygon offset
		//
		if ( input->shader->polygonOffset )
		{
			qglDisable( GL_POLYGON_OFFSET_FILL );
		}

		return;
	}

	//
	//
	// call shader function
	//
	RB_IterateStagesGeneric( input );

	// Now check for surfacesprites.
#if 0
	if (r_surfaceSprites->integer)
	{
		//for ( int stage = 1; stage < MAX_SHADER_STAGES/*tess.shader->numUnfoggedPasses*/; stage++ )
		for ( int stage = 0; stage < MAX_SHADER_STAGES/*tess.shader->numUnfoggedPasses*/; stage++ )
		{
			if (tess.xstages[stage])
			{
				if (tess.xstages[stage]->ss && tess.xstages[stage]->ss->surfaceSpriteType)
				{	// Draw the surfacesprite
					RB_DrawSurfaceSprites( tess.xstages[stage], input);
				}
			}
		}
	}
#endif

	//
	// now do fog
	//
	if ( r_fog->integer && tess.fogNum && tess.shader->fogPass ) {
		RB_FogPass();
	}

	//
	// reset polygon offset
	//
	if ( input->shader->polygonOffset )
	{
		qglDisable( GL_POLYGON_OFFSET_FILL );
	}
}


/*
** RB_EndSurface
*/
void RB_EndSurface( void ) {
	shaderCommands_t *input;

	input = &tess;

	if (input->numIndexes == 0 || input->numVertexes == 0) {
		return;
	}

	if (input->indexes[SHADER_MAX_INDEXES-1] != 0) {
		ri->Error (ERR_DROP, "RB_EndSurface() - SHADER_MAX_INDEXES hit");
	}	
	if (input->xyz[SHADER_MAX_VERTEXES-1][0] != 0) {
		ri->Error (ERR_DROP, "RB_EndSurface() - SHADER_MAX_VERTEXES hit");
	}

	if ( tess.shader == tr.shadowShader ) {
		RB_ShadowTessEnd();
		return;
	}

	// for debugging of sort order issues, stop rendering after a given sort value
	if ( r_debugSort->integer && r_debugSort->integer < tess.shader->sort ) {
		return;
	}

	//
	// update performance counters
	//
	backEnd.pc.c_shaders++;
	backEnd.pc.c_vertexes += tess.numVertexes;
	backEnd.pc.c_indexes += tess.numIndexes;
	backEnd.pc.c_totalIndexes += tess.numIndexes * tess.numPasses;

	//
	// call off to shader specific tess end function
	//
	tess.currentStageIteratorFunc();

	//
	// draw debugging stuff
	//
	if ( r_showtris->integer ) {
		DrawTris (input);
	}
	if ( r_shownormals->integer ) {
		DrawNormals (input);
	}
	// clear shader so we can tell we don't have any unclosed surfaces
	tess.numIndexes = 0;
	tess.numVertexes = 0;
	tess.firstIndex = 0;
	tess.multiDrawPrimitives = 0;

	GLimp_LogComment( "----------\n" );
}
