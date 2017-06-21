#include "tr_local.h"

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

extern int R_BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct cplane_s *p);

int occlusionFrame = 0;
int numOccluded = 0;
int numNotOccluded = 0;

qboolean RB_CheckOcclusion(mnode_t *node)
{
	//if (node->occlusionCache <= 0) return qfalse;

	if (!(tr.viewParms.flags & VPF_DEPTHSHADOW) && r_occlusion->integer)
	{
		/*if (node->occluded)
		{
			if (r_occlusionDebug->integer == 4)
				ri->Printf(PRINT_WARNING, "Area %i was not tested for occlusion. It was occluded by the Q3 vis system.\n", node->area);

			return qtrue;
		}*/

		/* Occlusion culling check */
		GLuint result;
		qglGetQueryObjectuiv(node->occlusionCache, GL_QUERY_RESULT_AVAILABLE, &result);

		if (result)
		{
			qglGetQueryObjectuiv(node->occlusionCache, GL_QUERY_RESULT, &result);

			if (result <= 0)
			{// Occlusion culled...
				if (r_occlusionDebug->integer == 3)
					ri->Printf(PRINT_WARNING, "Area %i occlusion culled. Cache pos %u. Occluded with %u samples found.\n", node->area, node->occlusionCache, result);

				numOccluded++;
				return qtrue;
			}
			else
			{
				if (r_occlusionDebug->integer == 3)
					ri->Printf(PRINT_WARNING, "Area %i. Cache pos %u. Not occluded with %u samples found.\n", node->area, node->occlusionCache, result);
			}
		}
		else
		{
			if (r_occlusionDebug->integer == 3)
				ri->Printf(PRINT_WARNING, "Area %i failed to get occlusion result in time. Cache pos %u.\n", node->area, node->occlusionCache);
		}
		/* Occlusion culling check */
	}

	numNotOccluded++;
	return qfalse;
}

void RB_RecersiveCheckOcclusions(mnode_t *node, int planeBits)
{
	do {
		node->occluded = qfalse;

		// if the node wasn't marked as potentially visible, exit
		// pvs is skipped for depth shadows
		if (!(tr.viewParms.flags & VPF_DEPTHSHADOW) && node->visCounts[tr.visIndex] != tr.visCounts[tr.visIndex]) {
			break;
		}

		// if the bounding volume is outside the frustum, nothing
		// inside can be visible OPTIMIZE: don't do this all the way to leafs?

		if (!r_nocull->integer || SKIP_CULL_FRAME) {
			int		r;

			if (planeBits & 1) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[0]);
				if (r == 2) {
					node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~1;			// all descendants will also be in front
				}
			}

			if (planeBits & 2) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[1]);
				if (r == 2) {
					node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~2;			// all descendants will also be in front
				}
			}

			if (planeBits & 4) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[2]);
				if (r == 2) {
					node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~4;			// all descendants will also be in front
				}
			}

			if (planeBits & 8) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[3]);
				if (r == 2) {
					node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~8;			// all descendants will also be in front
				}
			}

			if (planeBits & 16) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[4]);
				if (r == 2) {
					node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~16;			// all descendants will also be in front
				}
			}

			if (tr.refdef.vieworg[0] >= node->mins[0]
				&& tr.refdef.vieworg[0] <= node->maxs[0]
				&& tr.refdef.vieworg[1] >= node->mins[1]
				&& tr.refdef.vieworg[1] <= node->maxs[1]
				&& tr.refdef.vieworg[2] >= node->mins[2]
				&& tr.refdef.vieworg[2] <= node->maxs[2])
			{// Never occlude a leaf we are inside...
				
			}
			else
			{
				node->occluded = RB_CheckOcclusion(node);
			}
		}

		tess.numIndexes = 0;
		tess.firstIndex = 0;
		tess.numVertexes = 0;
		tess.minIndex = 0;
		tess.maxIndex = 0;

		if (node->contents != -1) {
			break;
		}

		// node is just a decision point, so go down both sides
		// since we don't care about sort orders, just go positive to negative

		// recurse down the children, front side first
		RB_RecersiveCheckOcclusions(node->children[0], planeBits);

		// tail recurse
		node = node->children[1];
	} while (1);
}

void RB_CheckOcclusions ( void )
{
	numOccluded = 0;
	numNotOccluded = 0;

	int planeBits = (tr.viewParms.flags & VPF_FARPLANEFRUSTUM) ? 31 : 15;
	RB_RecersiveCheckOcclusions(tr.world->nodes, planeBits);

	if (r_occlusionDebug->integer)
	{
		int total = numOccluded + numNotOccluded;
		float fnumNotOccluded = numNotOccluded;
		float fnumOccluded = numOccluded;
		float ftotal = total;
		float fperc = (numOccluded / ftotal) * 100.0;
		int perc = fperc;
		ri->Printf(PRINT_ALL, "Occlusion frame %i. visible nodes %i. occluded nodes %i. %i percent occluded.\n", occlusionFrame, numNotOccluded, numOccluded, perc);
	}
}

void RB_OcclusionQuad(vec4_t quadVerts[4], vec2_t texCoords[4])
{
	//	GLimp_LogComment("--- RB_InstantQuad2 ---\n");					// FIXME: REIMPLEMENT (wasn't implemented in ioq3 to begin with) --eez

	tess.numVertexes = 0;
	tess.numIndexes = 0;
	tess.firstIndex = 0;

	VectorCopy4(quadVerts[0], tess.xyz[tess.numVertexes]);
	VectorCopy2(texCoords[0], tess.texCoords[tess.numVertexes][0]);
	tess.numVertexes++;

	VectorCopy4(quadVerts[1], tess.xyz[tess.numVertexes]);
	VectorCopy2(texCoords[1], tess.texCoords[tess.numVertexes][0]);
	tess.numVertexes++;

	VectorCopy4(quadVerts[2], tess.xyz[tess.numVertexes]);
	VectorCopy2(texCoords[2], tess.texCoords[tess.numVertexes][0]);
	tess.numVertexes++;

	VectorCopy4(quadVerts[3], tess.xyz[tess.numVertexes]);
	VectorCopy2(texCoords[3], tess.texCoords[tess.numVertexes][0]);
	tess.numVertexes++;

	tess.indexes[tess.numIndexes++] = 0;
	tess.indexes[tess.numIndexes++] = 1;
	tess.indexes[tess.numIndexes++] = 2;
	tess.indexes[tess.numIndexes++] = 0;
	tess.indexes[tess.numIndexes++] = 2;
	tess.indexes[tess.numIndexes++] = 3;
	tess.minIndex = 0;
	tess.maxIndex = 3;

	RB_UpdateVBOs(ATTR_POSITION | ATTR_TEXCOORD0);

	GLSL_VertexAttribsState(ATTR_POSITION | ATTR_TEXCOORD0);

	R_DrawElementsVBO(tess.numIndexes, tess.firstIndex, tess.minIndex, tess.maxIndex, tess.numVertexes, qfalse);

	tess.numIndexes = 0;
	tess.numVertexes = 0;
	tess.firstIndex = 0;
	tess.minIndex = 0;
	tess.maxIndex = 0;
}

matrix_t OCCLUSION_MVP;

void RB_OcclusionInstantQuad(vec4_t quadVerts[4])
{
	vec2_t texCoords[4];

	VectorSet2(texCoords[0], 0.0f, 0.0f);
	VectorSet2(texCoords[1], 1.0f, 0.0f);
	VectorSet2(texCoords[2], 1.0f, 1.0f);
	VectorSet2(texCoords[3], 0.0f, 1.0f);

	RB_OcclusionQuad(quadVerts, texCoords);
}

void AddOcclusionCube(const vec3_t mins, const vec3_t maxs)
{
	vec4_t quadVerts[4];

	VectorSet4(quadVerts[0], mins[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[1], mins[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[2], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[3], mins[0], mins[1], maxs[2], 1);
	RB_OcclusionInstantQuad(quadVerts);

	VectorSet4(quadVerts[0], maxs[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[2], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[3], maxs[0], mins[1], mins[2], 1);
	RB_OcclusionInstantQuad(quadVerts);

	VectorSet4(quadVerts[0], mins[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[1], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[2], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[3], maxs[0], mins[1], maxs[2], 1);
	RB_OcclusionInstantQuad(quadVerts);

	VectorSet4(quadVerts[0], maxs[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[2], mins[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[3], mins[0], mins[1], mins[2], 1);
	RB_OcclusionInstantQuad(quadVerts);

	VectorSet4(quadVerts[0], mins[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[1], mins[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[2], maxs[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[3], maxs[0], mins[1], mins[2], 1);
	RB_OcclusionInstantQuad(quadVerts);

	VectorSet4(quadVerts[0], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[2], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[3], mins[0], maxs[1], mins[2], 1);
	RB_OcclusionInstantQuad(quadVerts);
}

int numOcclusionQueries = 0;

void RB_OcclusionQuery(mnode_t *node, int planeBits)
{
	do {
		node->occlusionCache = -1;
		node->occluded = qfalse;

		// if the node wasn't marked as potentially visible, exit
		// pvs is skipped for depth shadows
		if (!(tr.viewParms.flags & VPF_DEPTHSHADOW) && node->visCounts[tr.visIndex] != tr.visCounts[tr.visIndex]) {
			break;
		}

		if (r_occlusionDebug->integer == 2)
			ri->Printf(PRINT_ALL, "Frame: %i. Area %i. Mins: %f %f %f. Maxs %f %f %f.\n", occlusionFrame, node->area, node->mins[0], node->mins[1], node->mins[2], node->maxs[0], node->maxs[1], node->maxs[2]);

		// if the bounding volume is outside the frustum, nothing
		// inside can be visible OPTIMIZE: don't do this all the way to leafs?

		if (!r_nocull->integer || SKIP_CULL_FRAME) {
			int		r;

			if (planeBits & 1) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &backEnd.viewParms.frustum[0]);
				if (r == 2) {
					node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~1;			// all descendants will also be in front
				}
			}

			if (planeBits & 2) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &backEnd.viewParms.frustum[1]);
				if (r == 2) {
					node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~2;			// all descendants will also be in front
				}
			}

			if (planeBits & 4) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &backEnd.viewParms.frustum[2]);
				if (r == 2) {
					node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~4;			// all descendants will also be in front
				}
			}

			if (planeBits & 8) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &backEnd.viewParms.frustum[3]);
				if (r == 2) {
					node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~8;			// all descendants will also be in front
				}
			}

			if (planeBits & 16) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &backEnd.viewParms.frustum[4]);
				if (r == 2) {
					node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~16;			// all descendants will also be in front
				}
			}

			if (node->contents != -1) {
				break;
			}

			if (backEnd.refdef.vieworg[0] >= node->mins[0]
				&& backEnd.refdef.vieworg[0] <= node->maxs[0]
				&& backEnd.refdef.vieworg[1] >= node->mins[1]
				&& backEnd.refdef.vieworg[1] <= node->maxs[1]
				&& backEnd.refdef.vieworg[2] >= node->mins[2]
				&& backEnd.refdef.vieworg[2] <= node->maxs[2])
			{// Never occlude a node we are inside...
				
			}
			else
			{
				/* Test the occlusion for this cube */
				qglGenQueries(1, &node->occlusionCache);
				//qglBeginQuery(GL_ANY_SAMPLES_PASSED, node->occlusionCache);
				qglBeginQuery(GL_SAMPLES_PASSED, node->occlusionCache);

				/* Create a cube for this mins/maxs */
				AddOcclusionCube(node->mins, node->maxs);

				qglEndQuery(GL_SAMPLES_PASSED);

				numOcclusionQueries++;
			}
		}

		tess.numIndexes = 0;
		tess.firstIndex = 0;
		tess.numVertexes = 0;
		tess.minIndex = 0;
		tess.maxIndex = 0;

		if (node->contents != -1) {
			break;
		}

		// node is just a decision point, so go down both sides
		// since we don't care about sort orders, just go positive to negative

		// recurse down the children, front side first
		RB_OcclusionQuery(node->children[0], planeBits);

		// tail recurse
		node = node->children[1];
	} while (1);

	tess.numIndexes = 0;
	tess.firstIndex = 0;
	tess.numVertexes = 0;
	tess.minIndex = 0;
	tess.maxIndex = 0;
}

void RB_OcclusionCulling(void)
{
	numOcclusionQueries = 0;

	if (!r_nocull->integer || SKIP_CULL_FRAME)
	{
		if (r_occlusion->integer)
		{
			occlusionFrame++;

			GLSL_BindProgram(&tr.textureColorShader);

			//GLSL_SetUniformMatrix16(&tr.textureColorShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
			Matrix16Multiply(backEnd.viewParms.projectionMatrix, backEnd.viewParms.world.modelMatrix, OCCLUSION_MVP);
			GLSL_SetUniformMatrix16(&tr.textureColorShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, OCCLUSION_MVP);
			vec4_t col = { 1, 1, 1, 0.3 };
			GLSL_SetUniformVec4(&tr.textureColorShader, UNIFORM_COLOR, colorWhite);

			// Don't draw into color or depth
			GL_State(0);
			qglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			//qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			GL_State(GLS_DEPTHFUNC_LESS | GLS_DEPTHFUNC_EQUAL);
			//qglDepthMask(GL_FALSE);

			GL_Cull(CT_TWO_SIDED);
			//qglDepthRange(0, 0);

			vec4_t color;
			color[0] = 1.0f;
			color[1] = 1.0f;
			color[2] = 1.0f;
			color[3] = 1.0f;

			int planeBits = (tr.viewParms.flags & VPF_FARPLANEFRUSTUM) ? 31 : 15;
			RB_OcclusionQuery(tr.world->nodes, planeBits);

			tess.numIndexes = 0;
			tess.firstIndex = 0;
			tess.numVertexes = 0;
			tess.minIndex = 0;
			tess.maxIndex = 0;


			//qglDepthRange( 0, 1 );
			qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			qglDepthMask(GL_TRUE);
			GL_State(GLS_DEFAULT);
			GL_Cull(CT_FRONT_SIDED);
			R_BindNullVBO();
			R_BindNullIBO();

			if (r_occlusionDebug->integer)
			{
				ri->Printf(PRINT_WARNING, "%i occlusion queries performed this frame (%i).\n", numOcclusionQueries, occlusionFrame);
			}
		}
	}
}


#endif //defined(__SOFTWARE_OCCLUSION__)
