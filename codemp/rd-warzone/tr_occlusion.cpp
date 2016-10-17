#include "tr_local.h"
#include "tr_occlusion.h"

void OQ_InitOcclusionQuery()
{

}

void OQ_ShutdownOcclusionQuery()
{

}

struct ShortVertex { float x, y, z; };
struct ClipspaceVertex { float x, y, z, w; };

void AddQuadStamp2(vec3_t quadVerts[4], unsigned int *numIndexes, unsigned int *indexes, unsigned int *numVerts, ShortVertex *xyz)
{
	int             ndx;

	ndx = *numVerts;

	// triangle indexes for a simple quad
	indexes[*numIndexes] = ndx;
	indexes[*numIndexes + 1] = ndx + 1;
	indexes[*numIndexes + 2] = ndx + 3;

	indexes[*numIndexes + 3] = ndx + 3;
	indexes[*numIndexes + 4] = ndx + 1;
	indexes[*numIndexes + 5] = ndx + 2;

	xyz[ndx + 0].x = quadVerts[0][0];
	xyz[ndx + 0].y = quadVerts[0][1];
	xyz[ndx + 0].z = quadVerts[0][2];

	xyz[ndx + 1].x = quadVerts[1][0];
	xyz[ndx + 1].y = quadVerts[1][1];
	xyz[ndx + 1].z = quadVerts[1][2];

	xyz[ndx + 2].x = quadVerts[2][0];
	xyz[ndx + 2].y = quadVerts[2][1];
	xyz[ndx + 2].z = quadVerts[2][2];

	xyz[ndx + 3].x = quadVerts[3][0];
	xyz[ndx + 3].y = quadVerts[3][1];
	xyz[ndx + 3].z = quadVerts[3][2];

	*numVerts += 4;
	*numIndexes += 6;
}

void AddCube(const vec3_t mins, const vec3_t maxs, unsigned int *numIndexes, unsigned int *indexes, unsigned int *numVerts, ShortVertex *xyz)
{
	vec3_t quadVerts[4];

	VectorSet(quadVerts[0], mins[0], mins[1], mins[2]);
	VectorSet(quadVerts[1], mins[0], maxs[1], mins[2]);
	VectorSet(quadVerts[2], mins[0], maxs[1], maxs[2]);
	VectorSet(quadVerts[3], mins[0], mins[1], maxs[2]);
	AddQuadStamp2(quadVerts, numIndexes, indexes, numVerts, xyz);

	VectorSet(quadVerts[0], maxs[0], mins[1], maxs[2]);
	VectorSet(quadVerts[1], maxs[0], maxs[1], maxs[2]);
	VectorSet(quadVerts[2], maxs[0], maxs[1], mins[2]);
	VectorSet(quadVerts[3], maxs[0], mins[1], mins[2]);
	AddQuadStamp2(quadVerts, numIndexes, indexes, numVerts, xyz);

	VectorSet(quadVerts[0], mins[0], mins[1], maxs[2]);
	VectorSet(quadVerts[1], mins[0], maxs[1], maxs[2]);
	VectorSet(quadVerts[2], maxs[0], maxs[1], maxs[2]);
	VectorSet(quadVerts[3], maxs[0], mins[1], maxs[2]);
	AddQuadStamp2(quadVerts, numIndexes, indexes, numVerts, xyz);

	VectorSet(quadVerts[0], maxs[0], mins[1], mins[2]);
	VectorSet(quadVerts[1], maxs[0], maxs[1], mins[2]);
	VectorSet(quadVerts[2], mins[0], maxs[1], mins[2]);
	VectorSet(quadVerts[3], mins[0], mins[1], mins[2]);
	AddQuadStamp2(quadVerts, numIndexes, indexes, numVerts, xyz);

	VectorSet(quadVerts[0], mins[0], mins[1], mins[2]);
	VectorSet(quadVerts[1], mins[0], mins[1], maxs[2]);
	VectorSet(quadVerts[2], maxs[0], mins[1], maxs[2]);
	VectorSet(quadVerts[3], maxs[0], mins[1], mins[2]);
	AddQuadStamp2(quadVerts, numIndexes, indexes, numVerts, xyz);

	VectorSet(quadVerts[0], maxs[0], maxs[1], mins[2]);
	VectorSet(quadVerts[1], maxs[0], maxs[1], maxs[2]);
	VectorSet(quadVerts[2], mins[0], maxs[1], maxs[2]);
	VectorSet(quadVerts[3], mins[0], maxs[1], mins[2]);
	AddQuadStamp2(quadVerts, numIndexes, indexes, numVerts, xyz);
}

extern void RB_UpdateMatrixes(void);

void RB_UpdateOcclusion()
{

}

extern void R_RotateForViewer(void);

void RB_LeafOcclusion()
{
	int i;

	// first, check any outstanding queries
	RB_UpdateOcclusion();

	// only do this when we change frustum, as we're rendering quite a few things
	if (!tr.updateOcclusion[0])
	{
		return;
	}
	tr.updateOcclusion[0] = qfalse;

	if (r_occlusion->integer)
	{
		mnode_t *leaf;

		backEnd.ori = backEnd.viewParms.world;
		GL_SetModelviewMatrix(backEnd.ori.modelMatrix);
		GL_SetProjectionMatrix(backEnd.viewParms.projectionMatrix);

		// Flush denorms to zero to avoid performance issues with small values
		_mm_setcsr(_mm_getcsr() | 0x8040);

		MaskedOcclusionCulling *moc = MaskedOcclusionCulling::Create();

		////////////////////////////////////////////////////////////////////////////////////////
		// Print which version (instruction set) is being used
		////////////////////////////////////////////////////////////////////////////////////////

		MaskedOcclusionCulling::Implementation implementation = moc->GetImplementation();

		/*switch (implementation)
		{
		case MaskedOcclusionCulling::SSE2: ri->Printf(PRINT_ALL, "Using SSE2 version\n"); break;
		case MaskedOcclusionCulling::SSE41: ri->Printf(PRINT_ALL, "Using SSE41 version\n"); break;
		case MaskedOcclusionCulling::AVX2: ri->Printf(PRINT_ALL, "Using AVX2 version\n"); break;
		}*/

		////////////////////////////////////////////////////////////////////////////////////////
		// Setup and state related code
		////////////////////////////////////////////////////////////////////////////////////////

		// Setup a rendertarget with near clip plane at w = 1.0
		const int width = glConfig.vidWidth * r_superSampleMultiplier->value, height = glConfig.vidHeight * r_superSampleMultiplier->value;
		moc->SetResolution(width, height);
		//moc->SetNearClipPlane(r_znear->value);

		// Clear the depth buffer
		moc->ClearBuffer();

		////////////////////////////////////////////////////////////////////////////////////////
		// Render some occluders
		////////////////////////////////////////////////////////////////////////////////////////

		int NUM_VISIBLE = 0;
		int NUM_OCCLUDED = 0;
		int NUM_CULLED = 0;

		/* Switched from rend2 normal arrays to using the example's array formats */
		unsigned int		numIndexes = 0;
		unsigned int		indexes[36];
		unsigned int		numVerts = 0;
		ShortVertex			xyz[24];
		ClipspaceVertex		xyz2[24];

		for (i = 0; i < tr.world->numVisibleLeafs[0]; i++)
		{
			leaf = tr.world->visibleLeafs[0][i];

			numVerts = 0;
			numIndexes = 0;

#if 0
			{/* Early skip close stuff... Will see if I need this later */
				vec3_t mins, maxs;
				VectorSubtract(leaf->mins, backEnd.ori.viewOrigin, mins);
				VectorSubtract(leaf->maxs, backEnd.ori.viewOrigin, maxs);

				if (VectorLength(mins) < 4096.0 || VectorLength(maxs) < 4096.0)
				{
					leaf->occluded[0] = qfalse;
					NUM_VISIBLE++;
					continue;
				}
			}
#endif

			vec3_t mins, maxs;
			VectorCopy(leaf->mins, mins);
			VectorCopy(leaf->maxs, maxs);
			//VectorSubtract(leaf->mins, backEnd.ori.viewOrigin, mins);
			//VectorSubtract(leaf->maxs, backEnd.ori.viewOrigin, maxs);

			//if (r_occlusionDebug->integer >= 2)
			//	ri->Printf(PRINT_ALL, "[2] mins is %f %f %f. maxs is %f %f %f.\n", mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2]);

			/* Create a cube for this mins/maxs */
			AddCube(mins, maxs, &numIndexes, indexes, &numVerts, xyz);

			/* Convert xyz to clip space */
			moc->TransformVertices(glState.modelviewProjection, (const float*)xyz, (float *)xyz2, numVerts);

			/*for (int t = 0; t < numVerts; t++)
			{
				xyz2[t].x /= xyz2[t].w;
				xyz2[t].y /= xyz2[t].w;
				xyz2[t].z /= xyz2[t].w;
				//xyz2[t].w /= xyz2[t].w;
			}*/

			
			/*vec4_t eye, in, out;
			for (int t = 0; t < numVerts; t++)
			{// None of this gives between -1 and 1... grrr....
				VectorSet(in, xyz[t].x, xyz[t].y, xyz[t].z);
				
				if (r_occlusion->integer == 4)
					R_TransformModelToClip(in, glState.modelview, glState.projection, eye, out);
				else if (r_occlusion->integer == 3) // tr_main code uses this...
					R_TransformModelToClip(in, tr.ori.modelMatrix, tr.viewParms.projectionMatrix, eye, out);
				else if (r_occlusion->integer == 2) // xyc suggested this...
					R_TransformModelToClip(in, tr.ori.modelMatrix, backEnd.viewParms.projectionMatrix, eye, out);
				else // tr_flares uses this...
					R_TransformModelToClip(in, backEnd.ori.modelMatrix, backEnd.viewParms.projectionMatrix, eye, out);

				xyz2[t].x = out[0];
				xyz2[t].y = out[1];
				xyz2[t].z = out[2];
				xyz2[t].w = out[3];
			}*/

			/* Debug xyz values */
			if (r_occlusionDebug->integer == 2)
			{
				for (int t = 0; t < numVerts; t++)
				{
					ri->Printf(PRINT_ALL, "xyz is %f %f %f. xyz2 is %f %f %f %f.\n"
						, xyz[t].x, xyz[t].y, xyz[t].z
						, xyz2[t].x, xyz2[t].y, xyz2[t].z, xyz2[t].w);
				}
			}

			/* Test the occlusion for this cube */
			MaskedOcclusionCulling::CullingResult result = moc->TestTriangles((float*)xyz2, (unsigned int*)indexes, numIndexes / 3, nullptr, MaskedOcclusionCulling::CLIP_PLANE_SIDES/*CLIP_PLANE_ALL*/);

			//result = moc->TestRect(-0.6f, -0.6f, -0.4f, -0.4f, 100);

			if (result == MaskedOcclusionCulling::VISIBLE)
			{
				//ri->Printf(PRINT_ALL, "Tested triangle is VISIBLE\n");
				leaf->occluded[0] = qfalse;
				NUM_VISIBLE++;
			}
			else if (result == MaskedOcclusionCulling::OCCLUDED)
			{
				//ri->Printf(PRINT_ALL, "Tested triangle is OCCLUDED\n");
				leaf->occluded[0] = qtrue;
				NUM_OCCLUDED++;
			}
			else if (result == MaskedOcclusionCulling::VIEW_CULLED)
			{
				//ri->Printf(PRINT_ALL, "Tested triangle is outside view frustum\n");
				leaf->occluded[0] = qtrue;
				NUM_CULLED++;
			}

			moc->RenderTriangles((float*)xyz2, (unsigned int*)indexes, numIndexes / 3, nullptr, MaskedOcclusionCulling::CLIP_PLANE_SIDES/*CLIP_PLANE_ALL*/);

			//ri->Printf(PRINT_ALL, "rendered leaf %d, pos %d, query %d\n", leaf, querynum, occlusionCache[querynum]);
		}

		MaskedOcclusionCulling::Destroy(moc);

		if (r_occlusionDebug->integer == 1)
			ri->Printf(PRINT_ALL, "%i queries. %i visible. %i occluded. %i culled.\n", tr.world->numVisibleLeafs[0], NUM_VISIBLE, NUM_OCCLUDED, NUM_CULLED);
	}
}


const void	*RB_DrawOcclusion(const void *data) {
	const drawOcclusionCommand_t	*cmd;

	// finish any 2D drawing if needed
	if (tess.numIndexes) {
		RB_EndSurface();
	}

	cmd = (const drawOcclusionCommand_t *)data;

	backEnd.viewParms = cmd->viewParms;

	RB_LeafOcclusion();

	return (const void *)(cmd + 1);
}

void	R_AddDrawOcclusionCmd(viewParms_t *parms) {
	drawOcclusionCommand_t	*cmd;

	cmd = (drawOcclusionCommand_t *)R_GetCommandBuffer(sizeof(*cmd));
	if (!cmd) {
		return;
	}
	cmd->commandId = RC_DRAW_OCCLUSION;

	cmd->viewParms = *parms;
}
