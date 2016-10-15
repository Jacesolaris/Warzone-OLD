#include "tr_local.h"
#include "tr_occlusion.h"

#define __SOFTWARE_OCCLUSION__

#if 0

int main(int argc, char* argv[])
{
	// Flush denorms to zero to avoid performance issues with small values
	_mm_setcsr(_mm_getcsr() | 0x8040);

	MaskedOcclusionCulling *moc = MaskedOcclusionCulling::Create();

	////////////////////////////////////////////////////////////////////////////////////////
	// Print which version (instruction set) is being used
	////////////////////////////////////////////////////////////////////////////////////////

	MaskedOcclusionCulling::Implementation implementation = moc->GetImplementation();
	switch (implementation) {
	case MaskedOcclusionCulling::SSE2: printf("Using SSE2 version\n"); break;
	case MaskedOcclusionCulling::SSE41: printf("Using SSE41 version\n"); break;
	case MaskedOcclusionCulling::AVX2: printf("Using AVX2 version\n"); break;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Setup and state related code
	////////////////////////////////////////////////////////////////////////////////////////

	// Setup a 1920 x 1080 rendertarget with near clip plane at w = 1.0
	const int width = 1920, height = 1080;
	moc->SetResolution(width, height);
	moc->SetNearClipPlane(1.0f);

	// Clear the depth buffer
	moc->ClearBuffer();

	////////////////////////////////////////////////////////////////////////////////////////
	// Render some occluders
	////////////////////////////////////////////////////////////////////////////////////////
	struct ClipspaceVertex { float x, y, z, w; };

	// A triangle that intersects the view frustum
	ClipspaceVertex triVerts[] = { { 5, 0, 0, 10 }, { 30, 0, 0, 20 }, { 10, 50, 0, 40 } };
	unsigned int triIndices[] = { 0, 1, 2 };

	// Render the triangle
	moc->RenderTriangles((float*)triVerts, triIndices, 1);

	// A quad completely within the view frustum
	ClipspaceVertex quadVerts[] = { { -150, -150, 0, 200 }, { -10, -65, 0, 75 }, { 0, 0, 0, 20 }, { -40, 10, 0, 50 } };
	unsigned int quadIndices[] = { 0, 1, 2, 0, 2, 3 };

	// Render the quad. As an optimization, indicate that clipping is not required as it is 
	// completely inside the view frustum
	moc->RenderTriangles((float*)quadVerts, quadIndices, 2, nullptr, MaskedOcclusionCulling::CLIP_PLANE_NONE);

	// A triangle specified on struct of arrays (SoA) form
	float SoAVerts[] = {
		10, 10,   7, // x-coordinates
		-10, -7, -10, // y-coordinates
		10, 10,  10  // w-coordinates
	};

	// Set vertex layout (stride, y offset, w offset)
	MaskedOcclusionCulling::VertexLayout SoAVertexLayout(sizeof(float), 3 * sizeof(float), 6 * sizeof(float));

	// Render triangle with SoA layout
	moc->RenderTriangles((float*)SoAVerts, triIndices, 1, nullptr, MaskedOcclusionCulling::CLIP_PLANE_ALL, nullptr, SoAVertexLayout);


	////////////////////////////////////////////////////////////////////////////////////////
	// Perform some occlusion queries
	////////////////////////////////////////////////////////////////////////////////////////

	// A triangle, partly overlapped by the quad
	ClipspaceVertex oqTriVerts[] = { { 0, 50, 0, 200 }, { -60, -60, 0, 200 }, { 20, -40, 0, 200 } };
	unsigned int oqTriIndices[] = { 0, 1, 2 };

	// Perform an occlusion query. The triangle is visible and the query should return VISIBLE
	MaskedOcclusionCulling::CullingResult result;
	result = moc->TestTriangles((float*)oqTriVerts, oqTriIndices, 1);
	if (result == MaskedOcclusionCulling::VISIBLE)
		printf("Tested triangle is VISIBLE\n");
	else if (result == MaskedOcclusionCulling::OCCLUDED)
		printf("Tested triangle is OCCLUDED\n");
	else if (result == MaskedOcclusionCulling::VIEW_CULLED)
		printf("Tested triangle is outside view frustum\n");

	// Render the occlusion query triangle to show its position
	moc->RenderTriangles((float*)oqTriVerts, oqTriIndices, 1);


	// Perform an occlusion query testing if a rectangle is visible. The rectangle is completely 
	// behind the previously drawn quad, so the query should indicate that it's occluded
	result = moc->TestRect(-0.6f, -0.6f, -0.4f, -0.4f, 100);
	if (result == MaskedOcclusionCulling::VISIBLE)
		printf("Tested rect is VISIBLE\n");
	else if (result == MaskedOcclusionCulling::OCCLUDED)
		printf("Tested rect is OCCLUDED\n");
	else if (result == MaskedOcclusionCulling::VIEW_CULLED)
		printf("Tested rect is outside view frustum\n");

	// Compute a per pixel depth buffer from the hierarchical depth buffer, used for visualization.
	float *perPixelZBuffer = new float[width * height];
	moc->ComputePixelDepthBuffer(perPixelZBuffer);

	// Tonemap the image
	unsigned char *image = new unsigned char[width * height * 3];
	TonemapDepth(perPixelZBuffer, image, width, height);
	WriteBMP("image.bmp", image, width, height);
	delete[] image;

	// Destroy occlusion culling object and free hierarchical z-buffer
	MaskedOcclusionCulling::Destroy(moc);

}

#endif //__SOFTWARE_OCCLUSION__

#ifndef __SOFTWARE_OCCLUSION__

extern void R_DrawMultiElementsVBO( int multiDrawPrimitives, glIndex_t *multiDrawMinIndex, glIndex_t *multiDrawMaxIndex, 
	GLsizei *multiDrawNumIndexes, glIndex_t **multiDrawFirstIndex, glIndex_t numVerts, qboolean tesselation);

#define MAX_OCCLUSION_QUERIES 16384//1048576//16384

static GLuint occlusionCache[MAX_OCCLUSION_QUERIES];
static mnode_t *occlusionQueryTarget[MAX_OCCLUSION_QUERIES];
static qboolean occlusionQueryFinished[MAX_OCCLUSION_QUERIES];
static int occlusionQueryCount[MAX_OCCLUSION_QUERIES];
static int occlusionCachePos = 0;
static unsigned int lastOcclusionQueryCount = 0;
#endif //__SOFTWARE_OCCLUSION__

void OQ_InitOcclusionQuery()
{
#ifndef __SOFTWARE_OCCLUSION__
	occlusionCachePos = 0;
	lastOcclusionQueryCount = 0;
#endif //__SOFTWARE_OCCLUSION__
}

void OQ_ShutdownOcclusionQuery()
{
#ifndef __SOFTWARE_OCCLUSION__
	int i;
	for (i = 0; i < occlusionCachePos; i++)
	{
		qglDeleteQueries(1, &occlusionCache[i]);
	}
	occlusionCachePos = 0;
	lastOcclusionQueryCount = 0;
#endif //__SOFTWARE_OCCLUSION__
}

/*
==============
Tess_AddQuadStampExt2
==============
*/
void Tess_AddQuadStampExt2(vec4_t quadVerts[4], const vec4_t color, float s1, float t1, float s2, float t2, qboolean calcNormals)
{
	//int             i;
	//vec4_t          plane;
	int             ndx;

	GLimp_LogComment("--- Tess_AddQuadStampExt2 ---\n");

	//Tess_CheckOverflow(4, 6);

	ndx = tess.numVertexes;

	// triangle indexes for a simple quad
	tess.indexes[tess.numIndexes] = ndx;
	tess.indexes[tess.numIndexes + 1] = ndx + 1;
	tess.indexes[tess.numIndexes + 2] = ndx + 3;

	tess.indexes[tess.numIndexes + 3] = ndx + 3;
	tess.indexes[tess.numIndexes + 4] = ndx + 1;
	tess.indexes[tess.numIndexes + 5] = ndx + 2;

	VectorCopy4(quadVerts[0], tess.xyz[ndx + 0]);
	VectorCopy4(quadVerts[1], tess.xyz[ndx + 1]);
	VectorCopy4(quadVerts[2], tess.xyz[ndx + 2]);
	VectorCopy4(quadVerts[3], tess.xyz[ndx + 3]);

	// constant normal all the way around
	//if(calcNormals)
	//{
	//	PlaneFromPoints(plane, quadVerts[0], quadVerts[1], quadVerts[2]);
	//}
	//else
	//{
	//	VectorNegate(backEnd.viewParms.ori.axis[0], plane);
	//}

	//tess.normal[ndx][0] = tess.normal[ndx + 1][0] = tess.normal[ndx + 2][0] = tess.normal[ndx + 3][0] = plane[0];
	//tess.normal[ndx][1] = tess.normal[ndx + 1][1] = tess.normal[ndx + 2][1] = tess.normal[ndx + 3][1] = plane[1];
	//tess.normal[ndx][2] = tess.normal[ndx + 1][2] = tess.normal[ndx + 2][2] = tess.normal[ndx + 3][2] = plane[2];

	// standard square texture coordinates
	tess.texCoords[ndx][0][0] = s1;
	tess.texCoords[ndx][0][1] = t1;
	tess.texCoords[ndx][0][2] = 0;
	tess.texCoords[ndx][0][3] = 1;

	tess.texCoords[ndx + 1][0][0] = s2;
	tess.texCoords[ndx + 1][0][1] = t1;
	tess.texCoords[ndx + 1][0][2] = 0;
	tess.texCoords[ndx + 1][0][3] = 1;

	tess.texCoords[ndx + 2][0][0] = s2;
	tess.texCoords[ndx + 2][0][1] = t2;
	tess.texCoords[ndx + 2][0][2] = 0;
	tess.texCoords[ndx + 2][0][3] = 1;

	tess.texCoords[ndx + 3][0][0] = s1;
	tess.texCoords[ndx + 3][0][1] = t2;
	tess.texCoords[ndx + 3][0][2] = 0;
	tess.texCoords[ndx + 3][0][3] = 1;

	// constant color all the way around
	// should this be identity and let the shader specify from entity?
#ifndef __SOFTWARE_OCCLUSION__
	for(i = 0; i < 4; i++)
	{
		VectorCopy4(color, tess.vertexColors[ndx + i]);
	}
#endif //!__SOFTWARE_OCCLUSION__

	tess.numVertexes += 4;
	tess.numIndexes += 6;
}

void Tess_AddQuadStamp2(vec4_t quadVerts[4], const vec4_t color)
{
	Tess_AddQuadStampExt2(quadVerts, color, 0, 0, 1, 1, qfalse);
}

void Tess_AddCube(const vec3_t mins, const vec3_t maxs, const vec4_t color)
{
	vec4_t quadVerts[4];

#if 0//def __SOFTWARE_OCCLUSION__
	VectorSet4(quadVerts[0], mins[0], mins[1], mins[2], mins[2]);
	VectorSet4(quadVerts[1], mins[0], maxs[1], mins[2], mins[2]);
	VectorSet4(quadVerts[2], mins[0], maxs[1], maxs[2], maxs[2]);
	VectorSet4(quadVerts[3], mins[0], mins[1], maxs[2], maxs[2]);
	Tess_AddQuadStamp2(quadVerts, color);

	VectorSet4(quadVerts[0], maxs[0], mins[1], maxs[2], maxs[2]);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], maxs[2], maxs[2]);
	VectorSet4(quadVerts[2], maxs[0], maxs[1], mins[2], mins[2]);
	VectorSet4(quadVerts[3], maxs[0], mins[1], mins[2], mins[2]);
	Tess_AddQuadStamp2(quadVerts, color);

	VectorSet4(quadVerts[0], mins[0], mins[1], maxs[2], maxs[2]);
	VectorSet4(quadVerts[1], mins[0], maxs[1], maxs[2], maxs[2]);
	VectorSet4(quadVerts[2], maxs[0], maxs[1], maxs[2], maxs[2]);
	VectorSet4(quadVerts[3], maxs[0], mins[1], maxs[2], maxs[2]);
	Tess_AddQuadStamp2(quadVerts, color);

	VectorSet4(quadVerts[0], maxs[0], mins[1], mins[2], mins[2]);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], mins[2], mins[2]);
	VectorSet4(quadVerts[2], mins[0], maxs[1], mins[2], mins[2]);
	VectorSet4(quadVerts[3], mins[0], mins[1], mins[2], mins[2]);
	Tess_AddQuadStamp2(quadVerts, color);

	VectorSet4(quadVerts[0], mins[0], mins[1], mins[2], mins[2]);
	VectorSet4(quadVerts[1], mins[0], mins[1], maxs[2], maxs[2]);
	VectorSet4(quadVerts[2], maxs[0], mins[1], maxs[2], maxs[2]);
	VectorSet4(quadVerts[3], maxs[0], mins[1], mins[2], mins[2]);
	Tess_AddQuadStamp2(quadVerts, color);

	VectorSet4(quadVerts[0], maxs[0], maxs[1], mins[2], mins[2]);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], maxs[2], maxs[2]);
	VectorSet4(quadVerts[2], mins[0], maxs[1], maxs[2], maxs[2]);
	VectorSet4(quadVerts[3], mins[0], maxs[1], mins[2], mins[2]);
#else //!__SOFTWARE_OCCLUSION__
	VectorSet4(quadVerts[0], mins[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[1], mins[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[2], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[3], mins[0], mins[1], maxs[2], 1);
	Tess_AddQuadStamp2(quadVerts, color);

	VectorSet4(quadVerts[0], maxs[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[2], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[3], maxs[0], mins[1], mins[2], 1);
	Tess_AddQuadStamp2(quadVerts, color);

	VectorSet4(quadVerts[0], mins[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[1], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[2], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[3], maxs[0], mins[1], maxs[2], 1);
	Tess_AddQuadStamp2(quadVerts, color);

	VectorSet4(quadVerts[0], maxs[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[2], mins[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[3], mins[0], mins[1], mins[2], 1);
	Tess_AddQuadStamp2(quadVerts, color);

	VectorSet4(quadVerts[0], mins[0], mins[1], mins[2], 1);
	VectorSet4(quadVerts[1], mins[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[2], maxs[0], mins[1], maxs[2], 1);
	VectorSet4(quadVerts[3], maxs[0], mins[1], mins[2], 1);
	Tess_AddQuadStamp2(quadVerts, color);

	VectorSet4(quadVerts[0], maxs[0], maxs[1], mins[2], 1);
	VectorSet4(quadVerts[1], maxs[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[2], mins[0], maxs[1], maxs[2], 1);
	VectorSet4(quadVerts[3], mins[0], maxs[1], mins[2], 1);
	Tess_AddQuadStamp2(quadVerts, color);
#endif //__SOFTWARE_OCCLUSION__
}

extern void RB_UpdateMatrixes ( void );

void RB_UpdateOcclusion()
{
#ifndef __SOFTWARE_OCCLUSION__
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
#endif //__SOFTWARE_OCCLUSION__
}

#ifdef __SOFTWARE_OCCLUSION__
////////////////////////////////////////////////////////////////////////////////////////
// Image utility functions, minimal BMP writer and depth buffer tone mapping
////////////////////////////////////////////////////////////////////////////////////////

static void WriteBMP(const char *filename, const unsigned char *data, int w, int h)
{
	short header[] = { 0x4D42, 0, 0, 0, 0, 26, 0, 12, 0, (short)w, (short)h, 1, 24 };
	FILE *f = fopen(filename, "wb");
	fwrite(header, 1, sizeof(header), f);
	fwrite(data, 1, w * h * 3, f);
	fclose(f);
}

static void TonemapDepth(float *depth, unsigned char *image, int w, int h)
{
	// Find min/max w coordinate (discard cleared pixels)
	float minW = FLT_MAX, maxW = 0.0f;
	for (int i = 0; i < w*h; ++i)
	{
		if (depth[i] > 0.0f)
		{
			minW = min(minW, depth[i]);
			maxW = max(maxW, depth[i]);
		}
	}

	// Tonemap depth values
	for (int i = 0; i < w*h; ++i)
	{
		int intensity = 0;
		if (depth[i] > 0)
			intensity = (unsigned char)(223.0*(depth[i] - minW) / (maxW - minW) + 32.0);

		image[i * 3 + 0] = intensity;
		image[i * 3 + 1] = intensity;
		image[i * 3 + 2] = intensity;
	}
}
#endif //__SOFTWARE_OCCLUSION__

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

#ifndef __SOFTWARE_OCCLUSION__
	lastOcclusionQueryCount++;

	//ri.Printf(PRINT_ALL, "Beginning occlusion query %d, %d leafs\n", lastOcclusionQueryCount, tr.world->numVisibleLeafs[0]);

	GL_Bind( tr.whiteImage );
	GL_Cull( CT_TWO_SIDED );

	// Don't draw into color or depth
	GL_State(0);
	qglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	
	/* Testing */
	//GL_State(GLS_DEFAULT);
	//qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//qglDepthMask(GL_FALSE);
	//GL_State( GLS_DEPTHMASK_TRUE | GLS_DEPTHFUNC_LESS );
	//qglDisable(GL_BLEND);
	//qglDepthFunc(GL_LESS);
	//qglDepthMask(GL_TRUE);
	/* Testing */

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

			shaderProgram_t *shader = &tr.occlusionShader;

			tess.numVertexes = 0;
			tess.numIndexes = 0;
			tess.firstIndex = 0;

			vec3_t mins, maxs;

			VectorSubtract(leaf->mins, backEnd.ori.viewOrigin, mins);
			VectorSubtract(leaf->maxs, backEnd.ori.viewOrigin, maxs);

			if (VectorLength(mins) < 4096.0 || VectorLength(maxs) < 4096.0)
				continue;

			Tess_AddCube(mins, maxs, colorWhite);

			//ri->Printf(PRINT_ALL, "Mins is %f %f %f. Maxs is %f %f %f.\n", leaf->mins[0], leaf->mins[1], leaf->mins[2], leaf->maxs[0], leaf->maxs[1], leaf->maxs[2]);

			RB_UpdateVBOs(ATTR_POSITION);
			GLSL_VertexAttribsState(ATTR_POSITION);
			GLSL_BindProgram(shader);

			GLSL_SetUniformMatrix16(shader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);

			//GLSL_SetUniformVec3(shader, UNIFORM_VIEWORIGIN,  backEnd.refdef.vieworg);

			color[0] = 1.0f;
			color[1] = 1.0f;
			color[2] = 1.0f;
			color[3] = 1.0f;

			GLSL_SetUniformVec4(shader, UNIFORM_COLOR, color);

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
#else //__SOFTWARE_OCCLUSION__

	if (r_occlusion->integer)
	{
		mnode_t *leaf;

		RB_UpdateMatrixes();

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
		moc->SetNearClipPlane(r_znear->value);
		
		// Clear the depth buffer
		moc->ClearBuffer();

		////////////////////////////////////////////////////////////////////////////////////////
		// Render some occluders
		////////////////////////////////////////////////////////////////////////////////////////
		struct ClipspaceVertex { float x, y, z, w; };

		int NUM_VISIBLE = 0;
		int NUM_OCCLUDED = 0;
		int NUM_CULLED = 0;

		for (i = 0; i < tr.world->numVisibleLeafs[0]; i++)
		{
			leaf = tr.world->visibleLeafs[0][i];

			tess.numVertexes = 0;
			tess.numIndexes = 0;
			tess.firstIndex = 0;

			vec3_t mins, maxs;
			VectorSubtract(leaf->mins, backEnd.ori.viewOrigin, mins);
			VectorSubtract(leaf->maxs, backEnd.ori.viewOrigin, maxs);

			//vec4_t mins, maxs, eye;
			//R_TransformModelToClip(leaf->mins, backEnd.ori.modelMatrix, backEnd.viewParms.projectionMatrix, eye, mins);
			//R_TransformModelToClip(leaf->maxs, backEnd.ori.modelMatrix, backEnd.viewParms.projectionMatrix, eye, maxs);

			/*if (VectorLength(mins) < 4096.0 || VectorLength(maxs) < 4096.0)
			{
				leaf->occluded[0] = qfalse;
				NUM_VISIBLE++;
				continue;
			}*/

			Tess_AddCube(mins, maxs, colorWhite);

			// Render the triangle
			//moc->RenderTriangles((float*)triVerts, triIndices, 1);

			// Render the quad. As an optimization, indicate that clipping is not required as it is 
			// completely inside the view frustum
			//moc->RenderTriangles((float*)quadVerts, quadIndices, 2, nullptr, MaskedOcclusionCulling::CLIP_PLANE_NONE);

			//moc->RenderTriangles((const float*)tess.xyz, tess.indexes, tess.numIndexes - tess.firstIndex, nullptr, MaskedOcclusionCulling::CLIP_PLANE_ALL);

			MaskedOcclusionCulling::CullingResult result;

			
			//if (r_occlusion->integer == 2)
			//	result = moc->TestTriangles((float*)tess.xyz, tess.indexes, tess.numIndexes - tess.firstIndex, glState.modelviewProjection, MaskedOcclusionCulling::CLIP_PLANE_ALL);
			//else
			//	result = moc->TestTriangles((float*)tess.xyz, tess.indexes, tess.numIndexes - tess.firstIndex, nullptr, MaskedOcclusionCulling::CLIP_PLANE_ALL);
			
			MaskedOcclusionCulling::VertexLayout TessVertexLayout(sizeof(float), 2 * sizeof(float), 3 * sizeof(float));
			MaskedOcclusionCulling::VertexLayout TessVertexLayout2(sizeof(float), 2 * sizeof(float), 4 * sizeof(float));

			// A triangle specified on struct of arrays (SoA) form
			/*float SoAVerts[] = {
				10, 10, 7, // x-coordinates
				-10, -7, -10, // y-coordinates
				10, 10, 10  // w-coordinates
			};*/

			// Set vertex layout (stride, y offset, w offset)
			MaskedOcclusionCulling::VertexLayout SoAVertexLayout(sizeof(float), 3 * sizeof(float), 6 * sizeof(float));

			if (r_occlusion->integer == 6)
				result = moc->TestTriangles((float*)tess.xyz, tess.indexes, tess.numIndexes - tess.firstIndex, glState.modelviewProjection, MaskedOcclusionCulling::CLIP_PLANE_ALL, nullptr, SoAVertexLayout);
			else if (r_occlusion->integer == 5)
				result = moc->TestTriangles((float*)tess.xyz, tess.indexes, tess.numIndexes - tess.firstIndex, nullptr, MaskedOcclusionCulling::CLIP_PLANE_ALL, nullptr, SoAVertexLayout);
			else if (r_occlusion->integer == 4)
				result = moc->TestTriangles((float*)tess.xyz, tess.indexes, tess.numIndexes - tess.firstIndex, glState.modelviewProjection, MaskedOcclusionCulling::CLIP_PLANE_ALL, nullptr, TessVertexLayout2);
			else if (r_occlusion->integer == 3)
				result = moc->TestTriangles((float*)tess.xyz, tess.indexes, tess.numIndexes - tess.firstIndex, nullptr, MaskedOcclusionCulling::CLIP_PLANE_ALL, nullptr, TessVertexLayout2);
			else if (r_occlusion->integer == 2)
				result = moc->TestTriangles((float*)tess.xyz, tess.indexes, tess.numIndexes - tess.firstIndex, glState.modelviewProjection, MaskedOcclusionCulling::CLIP_PLANE_ALL, nullptr, TessVertexLayout);
			else
				result = moc->TestTriangles((float*)tess.xyz, tess.indexes, tess.numIndexes - tess.firstIndex, nullptr, MaskedOcclusionCulling::CLIP_PLANE_ALL, nullptr, TessVertexLayout);

			/*
			vec4 clipSpacePos = projectionMatrix * (viewMatrix * vec4(point3D, 1.0));
			vec3 ndcSpacePos = clipSpacePos.xyz / clipSpacePos.w;
			*/

			/*
			vec4_t clipSpaceMins, clipSpaceMaxs;
			vec4_t ndcMins, ndcMaxs;
			vec3_t ndcSpaceMins, ndcSpaceMaxs;

			if (r_occlusion->integer == 1)
			{
				moc->TransformVertices(glState.modelviewProjection, mins, ndcSpaceMins, 1);
				moc->TransformVertices(glState.modelviewProjection, maxs, ndcSpaceMaxs, 1);
			}
			else
			{
				moc->TransformVertices(backEnd.ori.modelMatrix, mins, clipSpaceMins, 1);
				moc->TransformVertices(backEnd.ori.modelMatrix, maxs, clipSpaceMaxs, 1);
				moc->TransformVertices(backEnd.viewParms.projectionMatrix, clipSpaceMins, ndcMins, 1);
				moc->TransformVertices(backEnd.viewParms.projectionMatrix, clipSpaceMaxs, ndcMaxs, 1);
				VectorSet4(ndcSpaceMins, ndcMins[0] / ndcMins[4], ndcMins[1] / ndcMins[4], ndcMins[2] / ndcMins[4], ndcMins[3]);
				VectorSet4(ndcSpaceMaxs, ndcMaxs[0] / ndcMaxs[4], ndcMaxs[1] / ndcMaxs[4], ndcMaxs[2] / ndcMaxs[4], ndcMaxs[3]);
			}

			result = moc->TestRect(ndcSpaceMins[0], ndcSpaceMins[1], ndcSpaceMaxs[0], ndcSpaceMaxs[1], ndcSpaceMins[3]);
			*/

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

			//ri->Printf(PRINT_ALL, "rendered leaf %d, pos %d, query %d\n", leaf, querynum, occlusionCache[querynum]);

			tess.numVertexes = 0;
			tess.numIndexes = 0;
			tess.firstIndex = 0;
		}

		/*
		// Compute a per pixel depth buffer from the hierarchical depth buffer, used for visualization.
		float *perPixelZBuffer = new float[width * height];
		moc->ComputePixelDepthBuffer(perPixelZBuffer);

		// Tonemap the image
		unsigned char *image = new unsigned char[width * height * 3];
		TonemapDepth(perPixelZBuffer, image, width, height);
		WriteBMP("image.bmp", image, width, height);
		delete[] image;
		*/

		MaskedOcclusionCulling::Destroy(moc);

		ri->Printf(PRINT_ALL, "%i queries. %i visible. %i occluded. %i culled.\n", tr.world->numVisibleLeafs[0], NUM_VISIBLE, NUM_OCCLUDED, NUM_CULLED);
	}

#endif //__SOFTWARE_OCCLUSION__
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
