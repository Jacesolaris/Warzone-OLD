#include "tr_local.h"

extern int R_BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct cplane_s *p);

int occlusionFrame = 0;
int numOccluded = 0;
int numNotOccluded = 0;

#ifdef __SOFTWARE_OCCLUSION__
#include "MaskedOcclusionCulling/MaskedOcclusionCulling.h"

MaskedOcclusionCulling *moc = NULL;

#if defined(__THREADED_OCCLUSION__)
#include "MaskedOcclusionCulling/CullingThreadpool.h"

CullingThreadpool *ctp = NULL;
#endif //defined(__THREADED_OCCLUSION__)

#if defined(__THREADED_OCCLUSION2__)
#include "../client/tinythread.h"
#endif //defined(__THREADED_OCCLUSION2__)

qboolean SOFTWARE_OCCLUSION_INITIALIZED = qfalse;

void OQ_InitOcclusionQuery()
{
	if (SOFTWARE_OCCLUSION_INITIALIZED) return;

	SOFTWARE_OCCLUSION_INITIALIZED = qtrue;

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
	//const int width = glConfig.vidWidth * r_superSampleMultiplier->value;
	//const int height = glConfig.vidHeight * r_superSampleMultiplier->value;
	const int width = 640, height = 480;
	//const int width = 1920, height = 1080;

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
	if (!SOFTWARE_OCCLUSION_INITIALIZED) return;

	SOFTWARE_OCCLUSION_INITIALIZED = qfalse;

#if defined(__THREADED_OCCLUSION__)
	ctp->Flush();
	ctp->SuspendThreads();
	MaskedOcclusionCulling::Destroy(moc);
	delete ctp;
#endif //defined(__THREADED_OCCLUSION__)
}
#endif //__SOFTWARE_OCCLUSION__

#ifndef __SOFTWARE_OCCLUSION__
qboolean RB_CheckOcclusion(mnode_t *node)
{
	//if (node->occlusionCache <= 0) return qfalse;

	if (/*!(tr.viewParms.flags & VPF_DEPTHSHADOW) &&*/ r_occlusion->integer)
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
		//if (!(tr.viewParms.flags & VPF_DEPTHSHADOW) && node->visCounts[tr.visIndex] != tr.visCounts[tr.visIndex]) {
		//	break;
		//}

		// if the bounding volume is outside the frustum, nothing
		// inside can be visible OPTIMIZE: don't do this all the way to leafs?

		if (!r_nocull->integer || SKIP_CULL_FRAME) {
			int		r;

			if (planeBits & 1) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[0]);
				if (r == 2) {
					//node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~1;			// all descendants will also be in front
					break;
				}
			}

			if (planeBits & 2) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[1]);
				if (r == 2) {
					//node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~2;			// all descendants will also be in front
					break;
				}
			}

			if (planeBits & 4) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[2]);
				if (r == 2) {
					//node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~4;			// all descendants will also be in front
					break;
				}
			}

			if (planeBits & 8) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[3]);
				if (r == 2) {
					//node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~8;			// all descendants will also be in front
					break;
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
					break;
				}
			}

			if (node->contents != -1) {
				break;
			}

			/*if (tr.refdef.vieworg[0] >= node->mins[0]
				&& tr.refdef.vieworg[0] <= node->maxs[0]
				&& tr.refdef.vieworg[1] >= node->mins[1]
				&& tr.refdef.vieworg[1] <= node->maxs[1]
				&& tr.refdef.vieworg[2] >= node->mins[2]
				&& tr.refdef.vieworg[2] <= node->maxs[2])
			{// Never occlude a leaf we are inside...
				
			}
			else*/
			{
				node->occluded = RB_CheckOcclusion(node);
				
				if (node->occluded)
					break;
			}
		}

		tess.numIndexes = 0;
		tess.firstIndex = 0;
		tess.numVertexes = 0;
		tess.minIndex = 0;
		tess.maxIndex = 0;

		// node is just a decision point, so go down both sides
		// since we don't care about sort orders, just go positive to negative

		// recurse down the children, front side first
		RB_RecersiveCheckOcclusions(node->children[0], planeBits);

		// tail recurse
		node = node->children[1];
	} while (1);
}
#endif //!__SOFTWARE_OCCLUSION__

void RB_CheckOcclusions ( void )
{
#ifndef __SOFTWARE_OCCLUSION__
	numOccluded = 0;
	numNotOccluded = 0;

	//qglFinish();

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
#endif //__SOFTWARE_OCCLUSION__
}

matrix_t OCCLUSION_MVP;

#ifndef __SOFTWARE_OCCLUSION__
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

#ifdef __SOFTWARE_OCCLUSION__

#else //!__SOFTWARE_OCCLUSION__
	//RB_UpdateVBOs(ATTR_POSITION | ATTR_TEXCOORD0);

	//GLSL_VertexAttribsState(ATTR_POSITION | ATTR_TEXCOORD0);

	R_DrawElementsVBO(tess.numIndexes, tess.firstIndex, tess.minIndex, tess.maxIndex, tess.numVertexes, qfalse);
#endif //__SOFTWARE_OCCLUSION__

	tess.numIndexes = 0;
	tess.numVertexes = 0;
	tess.firstIndex = 0;
	tess.minIndex = 0;
	tess.maxIndex = 0;
}

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
	vec4_t quadVerts[6][4];
	vec3_t center[6];
	float dist[6];

	VectorSet4(quadVerts[0][0], mins[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[0][1], mins[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[0][2], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[0][3], mins[0], mins[1], maxs[2], 1);

	VectorSet4(quadVerts[1][0], maxs[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[1][1], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[1][2], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[1][3], maxs[0], mins[1], mins[2], 1);

	VectorSet4(quadVerts[2][0], mins[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[2][1], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[2][2], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[2][3], maxs[0], mins[1], maxs[2], 1);

	VectorSet4(quadVerts[3][0], maxs[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[3][1], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[3][2], mins[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[3][3], mins[0], mins[1], mins[2], 1);

	VectorSet4(quadVerts[4][0], mins[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[4][1], mins[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[4][2], maxs[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[4][3], maxs[0], mins[1], mins[2], 1);

	VectorSet4(quadVerts[5][0], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[5][1], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[5][2], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[5][3], mins[0], maxs[1], mins[2], 1);

	
	/*
	for (int i = 0; i < 6; i++)
	{
		RB_OcclusionInstantQuad(quadVerts[i]);
	}
	*/

	// Find the closest 3 sides, skip the other 3...
	for (int i = 0; i < 6; i++)
	{
		for (int x = 0; x < 4; x++)
		{
			center[i][0] += quadVerts[i][x][0];
			center[i][1] += quadVerts[i][x][1];
			center[i][2] += quadVerts[i][x][2];
		}

		center[i][0] /= 4.0;
		center[i][1] /= 4.0;
		center[i][2] /= 4.0;
		dist[i] = Distance(center[i], backEnd.refdef.vieworg);
	}

	int bestList[3] = { -1 };
	float bestDist[3] = { 999999.9 };
	
	for (int t = 0; t < 3; t++)
	{
		for (int i = 0; i < 6; i++)
		{
			if (bestList[0] == i || bestList[1] == i || bestList[2] == i)
				continue;

			if (dist[i] < bestDist[t])
			{
				bestDist[t] = dist[i];
				bestList[t] = i;
			}
		}
	}

	for (int i = 0; i < 3; i++)
	{
		RB_OcclusionInstantQuad(quadVerts[bestList[i]]);
	}
}

#else //__SOFTWARE_OCCLUSION__

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
#endif //__SOFTWARE_OCCLUSION__

int numOcclusionQueries = 0;

void RB_OcclusionQuery(mnode_t *node, int planeBits)
{
	do {
		node->occlusionCache = -1;
		node->occluded = qfalse;

		// if the node wasn't marked as potentially visible, exit
		// pvs is skipped for depth shadows
		//if (!(tr.viewParms.flags & VPF_DEPTHSHADOW) && node->visCounts[tr.visIndex] != tr.visCounts[tr.visIndex]) {
		//	break;
		//}

		if (r_occlusionDebug->integer == 2)
			ri->Printf(PRINT_ALL, "Frame: %i. Area %i. Mins: %f %f %f. Maxs %f %f %f.\n", occlusionFrame, node->area, node->mins[0], node->mins[1], node->mins[2], node->maxs[0], node->maxs[1], node->maxs[2]);

		// if the bounding volume is outside the frustum, nothing
		// inside can be visible OPTIMIZE: don't do this all the way to leafs?

		if (!r_nocull->integer || SKIP_CULL_FRAME) {
			int		r;

			if (planeBits & 1) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &backEnd.viewParms.frustum[0]);
				if (r == 2) {
					//node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~1;			// all descendants will also be in front
					break;
				}
			}

			if (planeBits & 2) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &backEnd.viewParms.frustum[1]);
				if (r == 2) {
					//node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~2;			// all descendants will also be in front
					break;
				}
			}

			if (planeBits & 4) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &backEnd.viewParms.frustum[2]);
				if (r == 2) {
					//node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~4;			// all descendants will also be in front
					break;
				}
			}

			if (planeBits & 8) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &backEnd.viewParms.frustum[3]);
				if (r == 2) {
					//node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~8;			// all descendants will also be in front
					break;
				}
			}

			if (planeBits & 16) {
				r = R_BoxOnPlaneSide(node->mins, node->maxs, &backEnd.viewParms.frustum[4]);
				if (r == 2) {
					//node->occluded = qtrue;
					break;						// culled
				}
				if (r == 1) {
					planeBits &= ~16;			// all descendants will also be in front
					break;
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
#ifdef __SOFTWARE_OCCLUSION__
				//MaskedOcclusionCulling::ClipPlanes clip = MaskedOcclusionCulling::CLIP_PLANE_ALL;
				//MaskedOcclusionCulling::ClipPlanes clip = MaskedOcclusionCulling::CLIP_PLANE_SIDES;
				MaskedOcclusionCulling::ClipPlanes clip = MaskedOcclusionCulling::CLIP_PLANE_NONE;

				/* Switched from rend2 normal arrays to using the example's array formats */
				unsigned int		numIndexes = 0;
				unsigned int		indexes[36];
				unsigned int		numVerts = 0;
				ShortVertex			xyz[24];
				ClipspaceVertex		xyz2[24];

				/*if (!node->nummarksurfaces)
				{// Hmm nothing in here... Testing this cube would be a little pointless... Always occluded...
					leaf->occluded[0] = qtrue;
					NUM_EMPTY++;
					continue;
				}*/

				/* Create a cube for this mins/maxs */
				AddCube(node->mins, node->maxs, &numIndexes, indexes, &numVerts, xyz);

				/* Test the occlusion for this cube */

				/* Convert xyz to clip space */
				moc->TransformVertices(OCCLUSION_MVP, (const float*)xyz, (float *)xyz2, numVerts);

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

				/*if (r_occlusionDebug->integer == 3)
				{
					ri->Printf(PRINT_ALL, "MVP is %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", MVP[0], MVP[1], MVP[2], MVP[3], MVP[4], MVP[5], MVP[6], MVP[7], MVP[8], MVP[9], MVP[10], MVP[11], MVP[12], MVP[13], MVP[14], MVP[15], MVP[16]);
				}*/

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
					node->occluded = qfalse;
				}
				else if (result == MaskedOcclusionCulling::OCCLUDED)
				{
					//ri->Printf(PRINT_ALL, "Tested triangle is OCCLUDED\n");
					node->occluded = qtrue;
				}
				else if (result == MaskedOcclusionCulling::VIEW_CULLED)
				{
					//ri->Printf(PRINT_ALL, "Tested triangle is outside view frustum\n");
					node->occluded = qtrue;
				}
#else //!__SOFTWARE_OCCLUSION__
				/* Test the occlusion for this cube */
				qglGenQueries(1, &node->occlusionCache);
				//qglBeginQuery(GL_ANY_SAMPLES_PASSED, node->occlusionCache);
				qglBeginQuery(GL_SAMPLES_PASSED, node->occlusionCache);

				/* Create a cube for this mins/maxs */
				AddOcclusionCube(node->mins, node->maxs);

				qglEndQuery(GL_SAMPLES_PASSED);
#endif //__SOFTWARE_OCCLUSION__

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

#ifdef __SOFTWARE_OCCLUSION__
			OQ_InitOcclusionQuery();

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
			Matrix16Copy(glState.modelviewProjection, OCCLUSION_MVP);

			ctp->ClearBuffer();
			ctp->WakeThreads();

			//MaskedOcclusionCulling::ClipPlanes clip = MaskedOcclusionCulling::CLIP_PLANE_ALL;
			//MaskedOcclusionCulling::ClipPlanes clip = MaskedOcclusionCulling::CLIP_PLANE_SIDES;
			MaskedOcclusionCulling::ClipPlanes clip = MaskedOcclusionCulling::CLIP_PLANE_NONE;

			for (int i = 0; i < tr.world->numWorldSurfaces; i++)
			{
				if (tr.world->surfacesViewCount[i] != tr.viewCount)
					continue;

				msurface_t *surf = tr.world->surfaces + i;
				srfBspSurface_t *bspSurf = (srfBspSurface_t *)surf->data;
				moc->RenderTriangles(bspSurf->verts->xyz, bspSurf->indexes, bspSurf->numIndexes / 3, OCCLUSION_MVP, clip);
			}

			for (int i = 0; i < tr.world->numMergedSurfaces; i++)
			{
				if (tr.world->mergedSurfacesViewCount[i] != tr.viewCount)
					continue;

				msurface_t *surf = tr.world->mergedSurfaces + i;
				srfBspSurface_t *bspSurf = (srfBspSurface_t *)surf->data;
				moc->RenderTriangles(bspSurf->verts->xyz, bspSurf->indexes, bspSurf->numIndexes / 3, OCCLUSION_MVP, clip);
			}
#endif //__THREADED_OCCLUSION__
#else //!__SOFTWARE_OCCLUSION__
			GLSL_VertexAttribsState(ATTR_POSITION);
			GLSL_BindProgram(&tr.occlusionShader);

			GLSL_SetUniformMatrix16(&tr.occlusionShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
			//Matrix16Multiply(backEnd.viewParms.projectionMatrix, backEnd.viewParms.world.modelMatrix, OCCLUSION_MVP);
			//GLSL_SetUniformMatrix16(&tr.occlusionShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, OCCLUSION_MVP);
			GLSL_SetUniformVec4(&tr.occlusionShader, UNIFORM_COLOR, colorWhite);

			// Don't draw into color or depth
			GL_State(0);
			qglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			//qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			GL_State(GLS_DEPTHFUNC_LESS | GLS_DEPTHFUNC_EQUAL);
			qglDepthMask(GL_FALSE);

			GL_Cull(CT_TWO_SIDED);
			//qglDepthRange(0, 0);
			qglDepthRange(0, 1);
#endif //__SOFTWARE_OCCLUSION__

			int planeBits = (tr.viewParms.flags & VPF_FARPLANEFRUSTUM) ? 31 : 15;
			RB_OcclusionQuery(tr.world->nodes, planeBits);

			tess.numIndexes = 0;
			tess.firstIndex = 0;
			tess.numVertexes = 0;
			tess.minIndex = 0;
			tess.maxIndex = 0;

#ifdef __SOFTWARE_OCCLUSION__
#ifdef __THREADED_OCCLUSION__
			ctp->Flush();
			ctp->SuspendThreads();
#else //!__THREADED_OCCLUSION__
			MaskedOcclusionCulling::Destroy(moc);
#endif //__THREADED_OCCLUSION__
#else //!__SOFTWARE_OCCLUSION__

			//qglDepthRange( 0, 1 );
			qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			qglDepthMask(GL_TRUE);
			GL_State(GLS_DEFAULT);
			GL_Cull(CT_FRONT_SIDED);
			R_BindNullVBO();
			R_BindNullIBO();

			qglFlush();
#endif //__SOFTWARE_OCCLUSION__

			if (r_occlusionDebug->integer)
			{
				ri->Printf(PRINT_WARNING, "%i occlusion queries performed this frame (%i).\n", numOcclusionQueries, occlusionFrame);
			}
		}
	}
}
