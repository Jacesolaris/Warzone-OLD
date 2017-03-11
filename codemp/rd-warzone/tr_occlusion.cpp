#include "tr_local.h"

#if defined(__ORIGINAL_OCCLUSION__)

#if defined(__SOFTWARE_OCCLUSION__)

#include "MaskedOcclusionCulling/MaskedOcclusionCulling.h"

MaskedOcclusionCulling *moc = NULL;

#if defined(__THREADED_OCCLUSION__)
#include "MaskedOcclusionCulling/CullingThreadpool.h"

CullingThreadpool *ctp = NULL;
#endif //defined(__THREADED_OCCLUSION__)

#if defined(__THREADED_OCCLUSION2__)
#include "../client/tinythread.h"
#endif //defined(__THREADED_OCCLUSION2__)

void OQ_InitOcclusionQuery()
{
#if defined(__THREADED_OCCLUSION__)
	// Flush denorms to zero to avoid performance issues with small values
	_mm_setcsr(_mm_getcsr() | 0x8040);

	moc = MaskedOcclusionCulling::Create();

	////////////////////////////////////////////////////////////////////////////////////////
	// Print which version (instruction set) is being used
	////////////////////////////////////////////////////////////////////////////////////////

	/*MaskedOcclusionCulling::Implementation implementation = moc->GetImplementation();

	switch (implementation)
	{
	case MaskedOcclusionCulling::SSE2: ri->Printf(PRINT_ALL, "MaskedOcclusionCulling - Using SSE2 version\n"); break;
	case MaskedOcclusionCulling::SSE41: ri->Printf(PRINT_ALL, "MaskedOcclusionCulling - Using SSE41 version\n"); break;
	case MaskedOcclusionCulling::AVX2: ri->Printf(PRINT_ALL, "MaskedOcclusionCulling - Using AVX2 version\n"); break;
	}*/

	////////////////////////////////////////////////////////////////////////////////////////
	// Setup and state related code
	////////////////////////////////////////////////////////////////////////////////////////

	// Setup a rendertarget with near clip plane at w = 1.0
	//const int width = glConfig.vidWidth * r_superSampleMultiplier->value, height = glConfig.vidHeight * r_superSampleMultiplier->value;
	const int width = 640, height = 480;
	int numThreads = Q_max(std::thread::hardware_concurrency() - 2, 2);// 1;
	//ctp = new CullingThreadpool(numThreads, 2, numThreads);
	ctp = new CullingThreadpool(numThreads, numThreads, numThreads, numThreads);
	ctp->SetBuffer(moc);
	ctp->SetResolution(width, height);
	ctp->SetNearClipPlane(r_znear->value);
#endif //defined(__THREADED_OCCLUSION__)
}

void OQ_ShutdownOcclusionQuery()
{
#if defined(__THREADED_OCCLUSION__)
	ctp->Flush();
	ctp->SuspendThreads();
	MaskedOcclusionCulling::Destroy(moc);
	delete ctp;
#endif //defined(__THREADED_OCCLUSION__)
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

//#define __SORT_AREAS__

void RB_LeafOcclusion()
{
	int i;

	// only do this when we change frustum, as we're rendering quite a few things
	if (!tr.updateOcclusion[0])
	{
		return;
	}
	tr.updateOcclusion[0] = qfalse;

	if (r_occlusion->integer)
	{
		mnode_t *leaf;
		matrix_t MVP;
		Matrix16Multiply(backEnd.viewParms.projectionMatrix, backEnd.viewParms.world.modelMatrix, MVP);

#if !defined(__THREADED_OCCLUSION__)
		// Flush denorms to zero to avoid performance issues with small values
		_mm_setcsr(_mm_getcsr() | 0x8040);

		MaskedOcclusionCulling *moc = MaskedOcclusionCulling::Create();

		////////////////////////////////////////////////////////////////////////////////////////
		// Print which version (instruction set) is being used
		////////////////////////////////////////////////////////////////////////////////////////

		/*MaskedOcclusionCulling::Implementation implementation = moc->GetImplementation();

		switch (implementation)
		{
		case MaskedOcclusionCulling::SSE2: ri->Printf(PRINT_ALL, "Using SSE2 version\n"); break;
		case MaskedOcclusionCulling::SSE41: ri->Printf(PRINT_ALL, "Using SSE41 version\n"); break;
		case MaskedOcclusionCulling::AVX2: ri->Printf(PRINT_ALL, "Using AVX2 version\n"); break;
		}*/

		////////////////////////////////////////////////////////////////////////////////////////
		// Setup and state related code
		////////////////////////////////////////////////////////////////////////////////////////

		// Setup a rendertarget with near clip plane at w = 1.0
		//const int width = glConfig.vidWidth * r_superSampleMultiplier->value, height = glConfig.vidHeight * r_superSampleMultiplier->value;
		const int width = 640, height = 480;
		moc->SetResolution(width, height);
		moc->SetNearClipPlane(r_znear->value);

		// Clear the depth buffer
		moc->ClearBuffer();
#else //defined(__THREADED_OCCLUSION__)
		ctp->ClearBuffer();
		ctp->WakeThreads();
#endif //__THREADED_OCCLUSION__

		////////////////////////////////////////////////////////////////////////////////////////
		// Render some occluders
		////////////////////////////////////////////////////////////////////////////////////////

		int NUM_VISIBLE = 0;
		int NUM_OCCLUDED = 0;
		int NUM_CULLED = 0;
		int NUM_EMPTY = 0;

		/* Switched from rend2 normal arrays to using the example's array formats */
		unsigned int		numIndexes = 0;
		unsigned int		indexes[36];
		unsigned int		numVerts = 0;
		ShortVertex			xyz[24];
		ClipspaceVertex		xyz2[24];

#ifdef __SORT_AREAS__
		/* Sort from close to far */
		int			numSorted = 0;
		qboolean	SORTED_ADDED[8192] = { qfalse };
		int			SORTED_LEAF_IDS[8192] = { -1 };
		vec3_t		SORTED_MINS_LIST[8192];
		vec3_t		SORTED_MAXS_LIST[8192];

		while (numSorted < tr.world->numVisibleLeafs)
		{
			int			best = -1;
			float		bestDistance = 999999.9;

			for (i = 0; i < tr.world->numVisibleLeafs; i++)
			{
				leaf = tr.world->visibleLeafs[i];

				if (SORTED_ADDED[i]) continue;

				vec3_t mins, maxs, center;
				VectorSubtract(leaf->mins, backEnd.ori.viewOrigin, mins);
				VectorSubtract(leaf->maxs, backEnd.ori.viewOrigin, maxs);
				VectorSet(center, maxs[0] - mins[0], maxs[1] - mins[1], maxs[2] - mins[2]);
				float dist = VectorLength(center);

				if (dist <= bestDistance)
				{
					best = i;
					bestDistance = dist;
				}
			}

			SORTED_LEAF_IDS[numSorted] = best;
			SORTED_ADDED[best] = qtrue;
			VectorCopy(tr.world->visibleLeafs[0][best]->mins, SORTED_MINS_LIST[numSorted]);
			VectorCopy(tr.world->visibleLeafs[0][best]->maxs, SORTED_MAXS_LIST[numSorted]);
			numSorted++;
		}

		if (r_occlusionDebug->integer == 5)
		{
			ri->Printf(PRINT_ALL, "%i sorted leafs from original %i unsorted.\n", numSorted, tr.world->numVisibleLeafs);

			for (i = 0; i < numSorted; i++)
			{
				ri->Printf(PRINT_ALL, "Sorted leaf %i is %i.\n", i, SORTED_LEAF_IDS[i]);
			}
		}
#else //!__SORT_AREAS__
		int numSorted = tr.world->numVisibleLeafs;
#endif //__SORT_AREAS__

		int numRendered = 0;
		qboolean threadsAwake = qfalse;

		for (i = 0; i < numSorted; i++)
		{
			//MaskedOcclusionCulling::ClipPlanes clip = MaskedOcclusionCulling::CLIP_PLANE_ALL;
			//MaskedOcclusionCulling::ClipPlanes clip = MaskedOcclusionCulling::CLIP_PLANE_SIDES;
			MaskedOcclusionCulling::ClipPlanes clip = MaskedOcclusionCulling::CLIP_PLANE_NONE;

			numVerts = 0;
			numIndexes = 0;

#ifdef __SORT_AREAS__
			leaf = tr.world->visibleLeafs[SORTED_LEAF_IDS[i]];
#else //!__SORT_AREAS__
			leaf = tr.world->visibleLeafs[i];
#endif //__SORT_AREAS__

			if (!leaf->nummarksurfaces)
			{// Hmm nothing in here... Testing this cube would be a little pointless... Always occluded...
				leaf->occluded[0] = qtrue;
				NUM_EMPTY++;
				continue;
			}

#if 0
			{/* Early skip close stuff... Will see if I need this later */
				vec3_t mins, maxs;
				VectorSubtract(SORTED_MINS_LIST[i], backEnd.ori.viewOrigin, mins);
				VectorSubtract(SORTED_MAXS_LIST[i], backEnd.ori.viewOrigin, maxs);

				if (VectorLength(mins) < 4096.0 || VectorLength(maxs) < 4096.0)
				{
					leaf->occluded[0] = qfalse;
					NUM_VISIBLE++;
					continue;
				}
			}
#endif

			/* Create a cube for this mins/maxs */
			AddCube(leaf->mins/*SORTED_MINS_LIST[i]*/, leaf->maxs/*SORTED_MAXS_LIST[i]*/, &numIndexes, indexes, &numVerts, xyz);

			/* Test the occlusion for this cube */

			/* Convert xyz to clip space */
			moc->TransformVertices(MVP, (const float*)xyz, (float *)xyz2, numVerts);

			/*for (int t = 0; t < numVerts; t++)
			{
			xyz2[t].w = 1.0 / xyz2[t].z;
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

			if (r_occlusionDebug->integer == 3)
			{
				ri->Printf(PRINT_ALL, "MVP is %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", MVP[0], MVP[1], MVP[2], MVP[3], MVP[4], MVP[5], MVP[6], MVP[7], MVP[8], MVP[9], MVP[10], MVP[11], MVP[12], MVP[13], MVP[14], MVP[15], MVP[16]);
			}

#ifndef __THREADED_OCCLUSION__
			MaskedOcclusionCulling::CullingResult result = moc->TestTriangles((float*)xyz2, (unsigned int*)indexes, numIndexes / 3, nullptr, clip);
			//moc->RenderTriangles((float*)xyz2, (unsigned int*)indexes, numIndexes / 3, nullptr, clip);
#else //__THREADED_OCCLUSION__
			MaskedOcclusionCulling::CullingResult result = ctp->TestTriangles((float*)xyz2, (unsigned int*)indexes, numIndexes / 3, clip);
			//ctp->RenderTriangles((float*)xyz2, (unsigned int*)indexes, numIndexes / 3, clip);
#endif //__THREADED_OCCLUSION__

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

			if (numRendered <= i)
			{
				const int NUM_RENDERS_AHEAD = 1;

				while (numRendered < i + NUM_RENDERS_AHEAD && numRendered < numSorted)
				{// Draw some more occluders... Always maintain NUM_RENDERS_AHEAD surface renders ahead of the occlusion tests...
					numVerts = 0;
					numIndexes = 0;

					mnode_t *leaf2 = tr.world->visibleLeafs[0][SORTED_LEAF_IDS[numRendered]];

					if (!leaf2->nummarksurfaces)
					{// Hmm nothing in here... No point drawing...
						numRendered++;
						continue;
					}

					/* Create a cube for this mins/maxs */
					AddCube(SORTED_MINS_LIST[numRendered], SORTED_MAXS_LIST[numRendered], &numIndexes, indexes, &numVerts, xyz);

					/* Convert xyz to clip space */
					moc->TransformVertices(MVP, (const float*)xyz, (float *)xyz2, numVerts);

#ifndef __THREADED_OCCLUSION__
					moc->RenderTriangles((float*)xyz2, (unsigned int*)indexes, numIndexes / 3, nullptr, clip);
#else //__THREADED_OCCLUSION__
					ctp->RenderTriangles((float*)xyz2, (unsigned int*)indexes, numIndexes / 3, clip);
#endif //__THREADED_OCCLUSION__

					numRendered++;
					continue;
				}
#ifdef __THREADED_OCCLUSION__
				if (r_occlusion->integer == 2)
					ctp->Flush();
#endif //__THREADED_OCCLUSION__
			}

			//ri->Printf(PRINT_ALL, "rendered leaf %d, pos %d, query %d\n", leaf, querynum, occlusionCache[querynum]);
		}

#ifdef __THREADED_OCCLUSION__
		ctp->Flush();
		ctp->SuspendThreads();
#else //!__THREADED_OCCLUSION__
		MaskedOcclusionCulling::Destroy(moc);
#endif //__THREADED_OCCLUSION__

		if (r_occlusionDebug->integer == 1)
			ri->Printf(PRINT_ALL, "%i queries. %i visible. %i occluded. %i culled. %i empty. %i percent removed.\n", tr.world->numVisibleLeafs, NUM_VISIBLE, NUM_OCCLUDED, NUM_CULLED, NUM_EMPTY, int(float((float((NUM_OCCLUDED + NUM_CULLED + NUM_EMPTY)) / float(tr.world->numVisibleLeafs[0]))) * 100.0));

		tr.updateVisibleSurfaces[0] = qtrue;
	}
}

#if defined(__THREADED_OCCLUSION2__)
using namespace tthread;

thread *OCCLUSION_THREAD;

void Occlusion_UpdateThread(void * aArg)
{
	RB_LeafOcclusion();
}

void Occlusion_FinishThread()
{
	if (OCCLUSION_THREAD && OCCLUSION_THREAD->joinable())
	{
		OCCLUSION_THREAD->join();
		OCCLUSION_THREAD = NULL;
	}
	else
	{
		OCCLUSION_THREAD = NULL;
	}
}
#endif //defined(__THREADED_OCCLUSION2__)

const void	*RB_DrawOcclusion(const void *data) {
	const drawOcclusionCommand_t	*cmd;

	// finish any 2D drawing if needed
	if (tess.numIndexes) {
		RB_EndSurface();
	}

	cmd = (const drawOcclusionCommand_t *)data;

	backEnd.viewParms = cmd->viewParms;

#if defined(__THREADED_OCCLUSION2__)
	if (OCCLUSION_THREAD)
	{
		OCCLUSION_THREAD->join();
		OCCLUSION_THREAD = NULL;
	}
	else
	{
		OCCLUSION_THREAD = NULL;
	}

	// Run in background thread...
	OCCLUSION_THREAD = new thread(Occlusion_UpdateThread, (void *)0);
	OCCLUSION_THREAD->detach();
#else //defined(__THREADED_OCCLUSION__)
	RB_LeafOcclusion();
#endif //defined(__THREADED_OCCLUSION__)

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

#include "MaskedOcclusionCulling/CullingThreadpool.h"

CullingThreadpool *ctp = NULL;

void OQ_InitOcclusionQuery()
{
#if !defined(__LEAF_OCCLUSION__)
	// Flush denorms to zero to avoid performance issues with small values
	_mm_setcsr(_mm_getcsr() | 0x8040);

	moc = MaskedOcclusionCulling::Create();

	////////////////////////////////////////////////////////////////////////////////////////
	// Print which version (instruction set) is being used
	////////////////////////////////////////////////////////////////////////////////////////

	/*MaskedOcclusionCulling::Implementation implementation = moc->GetImplementation();

	switch (implementation)
	{
	case MaskedOcclusionCulling::SSE2: ri->Printf(PRINT_ALL, "MaskedOcclusionCulling - Using SSE2 version\n"); break;
	case MaskedOcclusionCulling::SSE41: ri->Printf(PRINT_ALL, "MaskedOcclusionCulling - Using SSE41 version\n"); break;
	case MaskedOcclusionCulling::AVX2: ri->Printf(PRINT_ALL, "MaskedOcclusionCulling - Using AVX2 version\n"); break;
	}*/

	////////////////////////////////////////////////////////////////////////////////////////
	// Setup and state related code
	////////////////////////////////////////////////////////////////////////////////////////

	// Setup a rendertarget with near clip plane at w = 1.0
	//const int width = glConfig.vidWidth * r_superSampleMultiplier->value, height = glConfig.vidHeight * r_superSampleMultiplier->value;
	const int width = 640, height = 480;
	int numThreads = Q_max(std::thread::hardware_concurrency() - 2, 2);// 1;
	ctp = new CullingThreadpool(numThreads, 2, numThreads);
	//ctp = new CullingThreadpool(numThreads, numThreads, numThreads, numThreads);
	ctp->SetBuffer(moc);
	ctp->SetResolution(width, height);
	ctp->SetNearClipPlane(r_znear->value);
#endif //!defined(__LEAF_OCCLUSION__)
}

void OQ_ShutdownOcclusionQuery()
{
#if !defined(__LEAF_OCCLUSION__)
	ctp->Flush();
	ctp->SuspendThreads();
	MaskedOcclusionCulling::Destroy(moc);
	delete ctp;
#endif //!defined(__LEAF_OCCLUSION__)
}

struct ShortVertex { float x, y, z; };
struct ClipspaceVertex { float x, y, z, w; };

int PREVIOUS_FRAME_VISIBLE = 0;
int PREVIOUS_FRAME_OCCLUDED = 0;
int PREVIOUS_FRAME_CULLED = 0;
int PREVIOUS_FRAME_TOTAL = 0;

void RB_InitOcclusionFrame(void)
{
#if !defined(__LEAF_OCCLUSION__)
	if (r_occlusion->integer)
	{
		if (r_occlusion->integer == 2)
		{
			ri->Printf(PRINT_ALL, "OCCLUSION DEBUG: time %i. visible %i. occluded %i. culled %i. total %i. %i percent removed.\n", backEnd.refdef.time, PREVIOUS_FRAME_VISIBLE, PREVIOUS_FRAME_OCCLUDED, PREVIOUS_FRAME_CULLED, PREVIOUS_FRAME_TOTAL, int(((float)(PREVIOUS_FRAME_OCCLUDED + PREVIOUS_FRAME_CULLED) / (float)PREVIOUS_FRAME_TOTAL) * 100.0));
		}

		PREVIOUS_FRAME_VISIBLE = 0;
		PREVIOUS_FRAME_OCCLUDED = 0;
		PREVIOUS_FRAME_CULLED = 0;
		PREVIOUS_FRAME_TOTAL = 0;

		ctp->Flush();
		ctp->SuspendThreads();
		ctp->ClearBuffer();
		ctp->WakeThreads();
	}
#endif //!defined(__LEAF_OCCLUSION__)
}

qboolean RB_CheckOcclusion(matrix_t MVP, shaderCommands_t *input)
{
	qboolean occluded = qfalse;

#if !defined(__LEAF_OCCLUSION__)
	if (r_occlusion->integer 
		&& !backEnd.depthFill 
		&& !(backEnd.refdef.rdflags & RDF_NOWORLDMODEL) 
		/*&& backEnd.viewParms.targetFbo != tr.renderCubeFbo*/)
	{
		unsigned int		numIndexes = 0;
		unsigned int		indexes[SHADER_MAX_INDEXES];
		unsigned int		numVertexes = 0;
		ShortVertex			xyz[SHADER_MAX_VERTEXES];
		ClipspaceVertex		xyz2[SHADER_MAX_VERTEXES];

		MaskedOcclusionCulling::ClipPlanes clip = MaskedOcclusionCulling::CLIP_PLANE_ALL;// CLIP_PLANE_NONE;

		/*
		if (input->multiDrawPrimitives)
		{
			R_DrawMultiElementsVBO(input->multiDrawPrimitives, input->multiDrawMinIndex, input->multiDrawMaxIndex, input->multiDrawNumIndexes, input->multiDrawFirstIndex, input->numVertexes, tesselation);
		}
		else
		{
			R_DrawElementsVBO(input->numIndexes, input->firstIndex, input->minIndex, input->maxIndex, input->numVertexes, tesselation);
		}
		*/

		if (input->multiDrawPrimitives)
		{
			/*for (int t = 0; t < input->numVertexes; t++)
			{
				xyz[t].x = input->xyz[t][0];
				xyz[t].y = input->xyz[t][1];
				xyz[t].z = input->xyz[t][2];
				numVertexes++;
			}

			numIndexes = 0;// input->multiDrawNumIndexes;

			memcpy(indexes, input->multiDrawFirstIndex, sizeof(unsigned int) * SHADER_MAX_INDEXES);*/
			return qfalse;
		}
		else
		{
			for (int t = 0; t < input->numVertexes; t++)
			{
				xyz[t].x = input->xyz[t][0];
				xyz[t].y = input->xyz[t][1];
				xyz[t].z = input->xyz[t][2];
				numVertexes++;
			}

			numIndexes = input->numIndexes;
			memcpy(indexes, input->indexes, sizeof(unsigned int) * SHADER_MAX_INDEXES);
		}

		/* Convert xyz to clip space */
		moc->TransformVertices(MVP, (const float*)xyz, (float *)xyz2, numVertexes);
		
		//MaskedOcclusionCulling::CullingResult result = ctp->TestTriangles((float*)xyz2, (unsigned int*)indexes, numIndexes/* / 3*/, clip);
		MaskedOcclusionCulling::CullingResult result = ctp->TestTriangles((float*)xyz2, input->indexes, numIndexes / 4, clip);
		//result = moc->TestRect(-0.6f, -0.6f, -0.4f, -0.4f, 100);

		PREVIOUS_FRAME_TOTAL++;

		if (result == MaskedOcclusionCulling::VISIBLE)
		{
			//ri->Printf(PRINT_ALL, "Tested triangle is VISIBLE\n");
			occluded = qfalse;
			PREVIOUS_FRAME_VISIBLE++;
		}
		else if (result == MaskedOcclusionCulling::OCCLUDED)
		{
			//ri->Printf(PRINT_ALL, "Tested triangle is OCCLUDED\n");
			occluded = qtrue;
			PREVIOUS_FRAME_OCCLUDED++;
		}
		else if (result == MaskedOcclusionCulling::VIEW_CULLED)
		{
			//ri->Printf(PRINT_ALL, "Tested triangle is outside view frustum\n");
			occluded = qtrue;
			PREVIOUS_FRAME_CULLED++;
		}

		// Render it to the occlusion buffer if it is not occluded...
		//if (!occluded)
		//	moc->RenderTriangles((float*)xyz2, (unsigned int*)indexes, numIndexes / 3, nullptr, clip);
	}
#endif //!defined(__LEAF_OCCLUSION__)

	return occluded;
}

#else //!defined(__SOFTWARE_OCCLUSION__)

#define MAX_OCCLUSION_QUERIES 131072//262144//524288//1048576//16384

static GLuint occlusionCache[MAX_OCCLUSION_QUERIES];
#ifdef __VBO_BASED_OCCLUSION__
static VBO_t *occlusionQueryTarget[MAX_OCCLUSION_QUERIES];
#else //!__VBO_BASED_OCCLUSION__
static mnode_t *occlusionQueryTarget[MAX_OCCLUSION_QUERIES];
#endif //__VBO_BASED_OCCLUSION__
static int occlusionQueryCount[MAX_OCCLUSION_QUERIES];
static int occlusionCachePos = 0;

void RB_UpdateOcclusion()
{
	int numOccluded = 0;

	// first, check any outstanding queries
	for (int i = 0; i < occlusionCachePos; i++)
	{
		GLuint result;

		qglGetQueryObjectuiv(occlusionCache[i], GL_QUERY_RESULT_AVAILABLE, &result);

		if (result)
		{
			qglGetQueryObjectuiv(occlusionCache[i], GL_QUERY_RESULT, &result);
			//ri->Printf(PRINT_ALL, "leaf %d count %d query %d has %d samples!\n", occlusionQueryTarget[i], occlusionQueryCount[i], occlusionCache[i], result);
			if (!result)
			{
				occlusionQueryTarget[i]->occluded = qtrue;
				numOccluded++;
			}
		}
	}
	
	if (r_occlusion->integer > 1)
		ri->Printf(PRINT_WARNING, "%i occluded. %i total.\n", numOccluded, occlusionCachePos);

	occlusionCachePos = 0;
}

extern void RB_InstantQuad(vec4_t quadVerts[4]);

void AddOcclusionCube(const vec3_t mins, const vec3_t maxs)
{
	vec4_t quadVerts[4];

	VectorSet4(quadVerts[0], mins[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[1], mins[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[2], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[3], mins[0], mins[1], maxs[2], 1);
	RB_InstantQuad(quadVerts);

	VectorSet4(quadVerts[0], maxs[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[2], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[3], maxs[0], mins[1], mins[2], 1);
	RB_InstantQuad(quadVerts);

	VectorSet4(quadVerts[0], mins[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[1], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[2], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[3], maxs[0], mins[1], maxs[2], 1);
	RB_InstantQuad(quadVerts);

	VectorSet4(quadVerts[0], maxs[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[2], mins[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[3], mins[0], mins[1], mins[2], 1);
	RB_InstantQuad(quadVerts);

	VectorSet4(quadVerts[0], mins[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[1], mins[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[2], maxs[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[3], maxs[0], mins[1], mins[2], 1);
	RB_InstantQuad(quadVerts);

	VectorSet4(quadVerts[0], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[2], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[3], mins[0], maxs[1], mins[2], 1);
	RB_InstantQuad(quadVerts);
}

void RB_LeafOcclusion()
{
#ifdef __VBO_BASED_OCCLUSION__
	if (r_occlusion->integer)
	{
		occlusionCachePos = 0;

		FBO_Bind(tr.renderFbo);
		glState.currentFBO = tr.renderFbo;

		// Don't draw into color or depth
		GL_State(0);
		qglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		GL_State(GLS_DEPTHFUNC_LESS | GLS_DEPTHFUNC_EQUAL);

		GL_Cull(CT_TWO_SIDED);
		//qglDepthRange(0, 0);

		vec4_t color;
		color[0] = 1.0f;
		color[1] = 1.0f;
		color[2] = 1.0f;
		color[3] = 1.0f;

		tess.numIndexes = 0;
		tess.firstIndex = 0;
		tess.numVertexes = 0;
		tess.minIndex = 0;
		tess.maxIndex = 0;

		/* Switched from rend2 normal arrays to using the example's array formats */
		for (int i = 0; i < tr.world->numWorldVbos; i++)
		{
			VBO_t *leaf = tr.world->vbos[i];

			if (backEnd.refdef.vieworg[0] >= tr.world->vboMins[i][0]
				&& backEnd.refdef.vieworg[0] <= tr.world->vboMaxs[i][0]
				&& backEnd.refdef.vieworg[1] >= tr.world->vboMins[i][1]
				&& backEnd.refdef.vieworg[1] <= tr.world->vboMaxs[i][1]
				&& backEnd.refdef.vieworg[2] >= tr.world->vboMins[i][2]
				&& backEnd.refdef.vieworg[2] <= tr.world->vboMaxs[i][2])
			{// Never occlude a leaf we are inside...
				leaf->occluded = qfalse;
				continue;
			}

			/* Test the occlusion for this cube */
			qglGenQueries(1, &occlusionCache[occlusionCachePos]);
			qglBeginQuery(GL_SAMPLES_PASSED, occlusionCache[occlusionCachePos]);
			occlusionQueryTarget[occlusionCachePos] = leaf;
			occlusionCachePos++;

			/* Create a cube for this mins/maxs */
			AddOcclusionCube(tr.world->vboMins[i], tr.world->vboMaxs[i]);

			qglEndQuery(GL_SAMPLES_PASSED);

			//ri->Printf(PRINT_ALL, "rendered leaf %d, pos %d, query %d\n", leaf, querynum, occlusionCache[querynum]);
		}

		tess.numIndexes = 0;
		tess.firstIndex = 0;
		tess.numVertexes = 0;
		tess.minIndex = 0;
		tess.maxIndex = 0;

		//if (r_occlusion->integer > 1)
		qglFinish();
		//qglFlush();

		RB_UpdateOcclusion();

		//qglDepthRange( 0, 1 );
		qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		GL_State(GLS_DEFAULT);
		GL_Cull(CT_FRONT_SIDED);
		R_BindNullVBO();
		R_BindNullIBO();
	}
#else //!__VBO_BASED_OCCLUSION__
	if (r_occlusion->integer)
	{
		occlusionCachePos = 0;

		//shaderProgram_t *shader = &tr.occlusionShader;
		FBO_Bind(tr.renderFbo);
		glState.currentFBO = tr.renderFbo;
		/*RB_UpdateVBOs(ATTR_POSITION);
		GL_Bind(tr.whiteImage);
		GLSL_VertexAttribsState(ATTR_POSITION);
		GLSL_BindProgram(shader);*/

		// Don't draw into color or depth
		GL_State(0);
		qglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		//qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		//GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);
		//GL_State(/*GLS_DEPTHMASK_TRUE|GLS_DEPTHFUNC_LESS|GLS_DEPTHFUNC_EQUAL|*/GLS_POLYMODE_LINE);
		GL_State(GLS_DEPTHFUNC_LESS|GLS_DEPTHFUNC_EQUAL);

		GL_Cull(CT_TWO_SIDED);
		//qglDepthRange(0, 0);

		vec4_t color;
		color[0] = 1.0f;
		color[1] = 1.0f;
		color[2] = 1.0f;
		color[3] = 1.0f;

		//GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

		//GLSL_SetUniformVec4(shader, UNIFORM_COLOR, color);

		tess.numIndexes = 0;
		tess.firstIndex = 0;
		tess.numVertexes = 0;
		tess.minIndex = 0;
		tess.maxIndex = 0;

		/* Switched from rend2 normal arrays to using the example's array formats */
		for (int i = 0; i < tr.world->numVisibleLeafs; i++)
		{
			mnode_t *leaf = tr.world->visibleLeafs[i];

			//if (!leaf->nummarksurfaces)
			if (leaf->nummarksurfaces < 64)
			{// Hmm nothing in here... Testing this cube would be a little pointless... Always occluded...
				//leaf->occluded = qtrue;
				continue;
			}

			if (backEnd.refdef.vieworg[0] >= leaf->mins[0] 
				&& backEnd.refdef.vieworg[0] <= leaf->maxs[0]
				&& backEnd.refdef.vieworg[1] >= leaf->mins[1] 
				&& backEnd.refdef.vieworg[1] <= leaf->maxs[1]
				&& backEnd.refdef.vieworg[2] >= leaf->mins[2]
				&& backEnd.refdef.vieworg[2] <= leaf->maxs[2])
			{// Never occlude a leaf we are inside...
				leaf->occluded = qfalse;
				continue;
			}

			if (occlusionCachePos + 1 >= MAX_OCCLUSION_QUERIES)
			{// never over-run the max queries array...
				leaf->occluded = qfalse;
				//continue;
				ri->Printf(PRINT_WARNING, "Occlusion system hit MAX_OCCLUSION_QUERIES limit. %i extra leafs will be presumed visible!\n", tr.world->numVisibleLeafs - i);
				break;
			}

			vec3_t center;
			VectorSet(center, (leaf->mins[0] + leaf->maxs[0]) * 0.5f, (leaf->mins[1] + leaf->maxs[1]) * 0.5f, (leaf->mins[2] + leaf->maxs[2]) * 0.5f);
			if (Distance(center, backEnd.refdef.vieworg) <= 2048.0)//r_testvalue0->value)
			{// Just skip checking occlusion on close leafs...
				/*ri->Printf(PRINT_ALL, "Distance from %i %i %i to %i %i %i is %i (< %i).\n"
					, (int)backEnd.refdef.vieworg[0], (int)backEnd.refdef.vieworg[1], (int)backEnd.refdef.vieworg[2]
					, (int)center[0], (int)center[1], (int)center[2]
					, (int)Distance(center, backEnd.refdef.vieworg), (int)r_testvalue0->value
				);*/
				continue;
			}

			/* Test the occlusion for this cube */
			qglGenQueries(1, &occlusionCache[occlusionCachePos]);
			qglBeginQuery(GL_SAMPLES_PASSED, occlusionCache[occlusionCachePos]);
			occlusionQueryTarget[occlusionCachePos] = leaf;
			occlusionCachePos++;

			/* Create a cube for this mins/maxs */
			AddOcclusionCube(leaf->mins, leaf->maxs);

			qglEndQuery(GL_SAMPLES_PASSED);

			//ri->Printf(PRINT_ALL, "rendered leaf %d, pos %d, query %d\n", leaf, querynum, occlusionCache[querynum]);
		}

		tess.numIndexes = 0;
		tess.firstIndex = 0;
		tess.numVertexes = 0;
		tess.minIndex = 0;
		tess.maxIndex = 0;

		//if (r_occlusion->integer > 1)
			qglFinish();
		//qglFlush();

		RB_UpdateOcclusion();

		//qglDepthRange( 0, 1 );
		qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		GL_State(GLS_DEFAULT);
		GL_Cull(CT_FRONT_SIDED);
		R_BindNullVBO();
		R_BindNullIBO();
	}
#endif //__VBO_BASED_OCCLUSION__
}

const void	*RB_DrawOcclusion(const void *data) {
	const drawOcclusionCommand_t	*cmd;

	// finish any 2D drawing if needed
	if (tess.numIndexes) {
		RB_EndSurface();
	}

	cmd = (const drawOcclusionCommand_t *)data;

	/*backEnd.viewParms = cmd->viewParms;

	RB_LeafOcclusion();*/

	return (const void *)(cmd + 1);
}

void	R_AddDrawOcclusionCmd(viewParms_t *parms) 
{
	/*
	drawOcclusionCommand_t	*cmd;

	cmd = (drawOcclusionCommand_t *)R_GetCommandBuffer(sizeof(*cmd));
	if (!cmd) {
		return;
	}
	cmd->commandId = RC_DRAW_OCCLUSION;

	cmd->viewParms = *parms;
	*/
}

void OQ_InitOcclusionQuery()
{
}

void OQ_ShutdownOcclusionQuery()
{
}

void RB_InitOcclusionFrame(void)
{
}

qboolean RB_CheckOcclusion(matrix_t MVP, shaderCommands_t *input)
{
	return qfalse;
}

#endif //defined(__SOFTWARE_OCCLUSION__)

#endif //__ORIGINAL_OCCLUSION__
