#include "tr_local.h"

extern void R_DrawMultiElementsVBO( int multiDrawPrimitives, glIndex_t *multiDrawMinIndex, glIndex_t *multiDrawMaxIndex, 
	GLsizei *multiDrawNumIndexes, glIndex_t **multiDrawFirstIndex, glIndex_t numVerts, qboolean tesselation);

#define MAX_OCCLUSION_QUERIES 8192//1048576//16384

static GLuint occlusionCache[MAX_OCCLUSION_QUERIES];
static mnode_t *occlusionQueryTarget[MAX_OCCLUSION_QUERIES];
static qboolean occlusionQueryFinished[MAX_OCCLUSION_QUERIES];
static int occlusionQueryCount[MAX_OCCLUSION_QUERIES];
static int occlusionCachePos = 0;
static unsigned int lastOcclusionQueryCount = 0;

void OQ_InitOcclusionQuery()
{
	occlusionCachePos = 0;
	lastOcclusionQueryCount = 0;
}

void OQ_ShutdownOcclusionQuery()
{
	int i;
	for (i = 0; i < occlusionCachePos; i++)
	{
		qglDeleteQueries(1, &occlusionCache[i]);
	}
	occlusionCachePos = 0;
	lastOcclusionQueryCount = 0;
}

extern void R_WorldToLocal (const vec3_t world, vec3_t local);
extern void RB_UpdateMatrixes ( void );
extern void RB_ExternalIterateStagesGeneric( shaderCommands_t *input );

void RB_LeafOcclusion()
{
	int i;

	// first, check any outstanding queries
	for (i = 0; i < occlusionCachePos; i++)
	{
		GLuint result;
		if (occlusionQueryFinished[i])
			continue;

		qglGetQueryObjectuiv(occlusionCache[i], GL_QUERY_RESULT_AVAILABLE, &result);
		if (result)
		{
			occlusionQueryFinished[i] = qtrue;
			qglGetQueryObjectuiv(occlusionCache[i], GL_QUERY_RESULT, &result);
			//ri->Printf(PRINT_ALL, "leaf %d count %d query %d has %d samples!\n", occlusionQueryTarget[i], occlusionQueryCount[i], occlusionCache[i], result);
			if (!result && occlusionQueryCount[i] == lastOcclusionQueryCount)
			{
				occlusionQueryTarget[i]->occluded[0] = qtrue;
				tr.updateVisibleSurfaces[0] = qtrue;
			}
		}
	}

	// only do this when we change frustum, as we're rendering quite a few things
	if (!tr.updateOcclusion[0])
	{
		return;
	}
	tr.updateOcclusion[0] = qfalse;

	lastOcclusionQueryCount++;

	//ri.Printf(PRINT_ALL, "Beginning occlusion query %d, %d leafs\n", lastOcclusionQueryCount, tr.world->numVisibleLeafs[0]);

	GL_Bind( tr.whiteImage );
	GL_Cull( CT_TWO_SIDED );

	// Don't draw into color or depth
	GL_State(0);
	qglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	
	/* Testing */
	//qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//qglDepthMask(GL_FALSE);
	//GL_State( GLS_DEPTHMASK_TRUE | GLS_DEPTHFUNC_LESS );
	//qglDisable(GL_BLEND);
	//qglDepthFunc(GL_LESS);
	//qglDepthMask(GL_TRUE);
	/* Testing */

#if 0
	ri.Printf(PRINT_ALL, "modelview:\n");
	Matrix16Dump(backEnd.viewParms.world.modelMatrix);
	ri.Printf(PRINT_ALL, "projection:\n");
	Matrix16Dump(backEnd.viewParms.projectionMatrix);
#endif

	if (r_occlusion->integer)
	{
		vec4_t color;
		mnode_t *leaf;
		int lastquery;

		RB_UpdateMatrixes();

		lastquery = 0;
		for (i = 0; i < tr.world->numVisibleLeafs[0]; i++)
		{
			int querynum, j;
			leaf = tr.world->visibleLeafs[0][i];

			if (leaf->occluded[0])
				continue;

			// look for an available query
			querynum = occlusionCachePos;
			for (j = lastquery; j < occlusionCachePos; j++)
			{
				if (occlusionQueryFinished[j])
				{
					querynum = j;
					lastquery = querynum + 1;
					break;
				}
			}

			if (querynum == MAX_OCCLUSION_QUERIES - 1)
				break;

			if (querynum == occlusionCachePos)
			{
				qglGenQueries(1, &occlusionCache[occlusionCachePos]);
				occlusionCachePos++;
			}

			//shaderProgram_t *shader = &tr.genericShader[0]; // ??
			//shaderProgram_t *shader = &tr.shadowPassShader;
			shaderProgram_t *shader = &tr.textureColorShader;

			tess.numVertexes = 0;
			tess.numIndexes = 0;
			tess.firstIndex = 0;

			vec3_t mins, maxs;

			VectorSubtract(leaf->mins, backEnd.ori.viewOrigin, mins);
			VectorSubtract(leaf->maxs, backEnd.ori.viewOrigin, maxs);

			tess.xyz[tess.numVertexes][0] = mins[0];
			tess.xyz[tess.numVertexes][1] = mins[1];
			tess.xyz[tess.numVertexes][2] = mins[2];
			tess.numVertexes++;

			tess.xyz[tess.numVertexes][0] = mins[0];
			tess.xyz[tess.numVertexes][1] = maxs[1];
			tess.xyz[tess.numVertexes][2] = mins[2];
			tess.numVertexes++;

			tess.xyz[tess.numVertexes][0] = maxs[0];
			tess.xyz[tess.numVertexes][1] = maxs[1];
			tess.xyz[tess.numVertexes][2] = mins[2];
			tess.numVertexes++;

			tess.xyz[tess.numVertexes][0] = maxs[0];
			tess.xyz[tess.numVertexes][1] = mins[1];
			tess.xyz[tess.numVertexes][2] = mins[2];
			tess.numVertexes++;

			tess.xyz[tess.numVertexes][0] = mins[0];
			tess.xyz[tess.numVertexes][1] = mins[1];
			tess.xyz[tess.numVertexes][2] = maxs[2];
			tess.numVertexes++;

			tess.xyz[tess.numVertexes][0] = mins[0];
			tess.xyz[tess.numVertexes][1] = maxs[1];
			tess.xyz[tess.numVertexes][2] = maxs[2];
			tess.numVertexes++;

			tess.xyz[tess.numVertexes][0] = maxs[0];
			tess.xyz[tess.numVertexes][1] = maxs[1];
			tess.xyz[tess.numVertexes][2] = maxs[2];
			tess.numVertexes++;

			tess.xyz[tess.numVertexes][0] = maxs[0];
			tess.xyz[tess.numVertexes][1] = mins[1];
			tess.xyz[tess.numVertexes][2] = maxs[2];
			tess.numVertexes++;

			//231-301-015-154-126-265-734-304-762-237-456-567
			tess.indexes[tess.numIndexes++] = 2;
			tess.indexes[tess.numIndexes++] = 3;
			tess.indexes[tess.numIndexes++] = 1;

			tess.indexes[tess.numIndexes++] = 3;
			tess.indexes[tess.numIndexes++] = 0;
			tess.indexes[tess.numIndexes++] = 1;

			tess.indexes[tess.numIndexes++] = 0;
			tess.indexes[tess.numIndexes++] = 1;
			tess.indexes[tess.numIndexes++] = 5;

			tess.indexes[tess.numIndexes++] = 1;
			tess.indexes[tess.numIndexes++] = 5;
			tess.indexes[tess.numIndexes++] = 4;

			tess.indexes[tess.numIndexes++] = 1;
			tess.indexes[tess.numIndexes++] = 2;
			tess.indexes[tess.numIndexes++] = 6;

			tess.indexes[tess.numIndexes++] = 2;
			tess.indexes[tess.numIndexes++] = 6;
			tess.indexes[tess.numIndexes++] = 5;

			tess.indexes[tess.numIndexes++] = 7;
			tess.indexes[tess.numIndexes++] = 3;
			tess.indexes[tess.numIndexes++] = 4;

			tess.indexes[tess.numIndexes++] = 3;
			tess.indexes[tess.numIndexes++] = 0;
			tess.indexes[tess.numIndexes++] = 4;

			tess.indexes[tess.numIndexes++] = 7;
			tess.indexes[tess.numIndexes++] = 6;
			tess.indexes[tess.numIndexes++] = 2;

			tess.indexes[tess.numIndexes++] = 2;
			tess.indexes[tess.numIndexes++] = 3;
			tess.indexes[tess.numIndexes++] = 7;

			tess.indexes[tess.numIndexes++] = 4;
			tess.indexes[tess.numIndexes++] = 5;
			tess.indexes[tess.numIndexes++] = 6;

			tess.indexes[tess.numIndexes++] = 5;
			tess.indexes[tess.numIndexes++] = 6;
			tess.indexes[tess.numIndexes++] = 7;

			//ri->Printf(PRINT_ALL, "Mins is %f %f %f. Maxs is %f %f %f.\n", leaf->mins[0], leaf->mins[1], leaf->mins[2], leaf->maxs[0], leaf->maxs[1], leaf->maxs[2]);

			RB_UpdateVBOs(ATTR_POSITION);
			GLSL_VertexAttribsState(ATTR_POSITION);
			GLSL_BindProgram(shader);

			GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

			GLSL_SetUniformVec3(shader, UNIFORM_VIEWORIGIN,  backEnd.refdef.vieworg);

			color[0] = 1.0f;
			color[1] = 1.0f;
			color[2] = 1.0f;
			color[3] = 1.0f;

			GLSL_SetUniformVec4(shader, UNIFORM_COLOR, color);

			GL_BindToTMU( tr.whiteImage, TB_DIFFUSEMAP );

			qglBeginQuery(GL_SAMPLES_PASSED, occlusionCache[querynum]);

			if (r_tesselation->integer)
			{
				if (tess.multiDrawPrimitives)
				{
					R_DrawMultiElementsVBO(tess.multiDrawPrimitives, tess.multiDrawMinIndex, tess.multiDrawMaxIndex, tess.multiDrawNumIndexes, tess.multiDrawFirstIndex, tess.numVertexes, qtrue);
				}
				else
				{
					R_DrawElementsVBO(tess.numIndexes - tess.firstIndex, tess.firstIndex, tess.minIndex, tess.maxIndex, tess.numVertexes, qtrue);
				}
			}
			else
			{
				if (tess.multiDrawPrimitives)
				{
					R_DrawMultiElementsVBO(tess.multiDrawPrimitives, tess.multiDrawMinIndex, tess.multiDrawMaxIndex, tess.multiDrawNumIndexes, tess.multiDrawFirstIndex, tess.numVertexes, qfalse);
				}
				else
				{
					R_DrawElementsVBO(tess.numIndexes - tess.firstIndex, tess.firstIndex, tess.minIndex, tess.maxIndex, tess.numVertexes, qfalse);
				}
			}

			qglEndQuery(GL_SAMPLES_PASSED);
			occlusionQueryTarget[querynum] = leaf;
			occlusionQueryFinished[querynum] = qfalse;
			occlusionQueryCount[querynum] = lastOcclusionQueryCount;
			//ri->Printf(PRINT_ALL, "rendered leaf %d, pos %d, query %d\n", leaf, querynum, occlusionCache[querynum]);

			tess.numVertexes = 0;
			tess.numIndexes = 0;
			tess.firstIndex = 0;
		}
	}

	//ri->Printf(PRINT_ALL, "tr.world->numVisibleLeafs[0] is %i.\n", tr.world->numVisibleLeafs[0]);

	qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	GL_State(GLS_DEFAULT);
	GL_Cull( CT_FRONT_SIDED );
	R_BindNullVBO();
	R_BindNullIBO();

	/* Testing */
	/*qglFinish();
	for (i = 0; i < occlusionCachePos; i++)
	{
		GLuint result;
		if (occlusionQueryFinished[i])
			continue;

		qglGetQueryObjectuiv(occlusionCache[i], GL_QUERY_RESULT_AVAILABLE, &result);
		if (result)
		{
			occlusionQueryFinished[i] = qtrue;
			qglGetQueryObjectuiv(occlusionCache[i], GL_QUERY_RESULT, &result);
			//ri->Printf(PRINT_ALL, "leaf %d count %d query %d has %d samples!\n", occlusionQueryTarget[i], occlusionQueryCount[i], occlusionCache[i], result);
			if (!result && occlusionQueryCount[i] == lastOcclusionQueryCount)
			{
				occlusionQueryTarget[i]->occluded[0] = qtrue;
				tr.updateVisibleSurfaces[0] = qtrue;
			}
		}
	}*/
}


const void	*RB_DrawOcclusion( const void *data ) {
	const drawOcclusionCommand_t	*cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	cmd = (const drawOcclusionCommand_t *)data;

	backEnd.viewParms = cmd->viewParms;

	RB_LeafOcclusion();

	return (const void *)(cmd + 1);
}

void	R_AddDrawOcclusionCmd( viewParms_t *parms ) {
	drawOcclusionCommand_t	*cmd;

	cmd = (drawOcclusionCommand_t *)R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_DRAW_OCCLUSION;

	cmd->viewParms = *parms;
}
