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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "l_cmd.h"
#include "l_math.h"
#include "l_mem.h"
#include "l_log.h"
#include "l_poly.h"
#include "botlib/l_script.h"
#include "l_qfiles.h"
#include "l_bsp_q3.h"
#include "l_bsp_ja.h"
#include "l_bsp_ent.h"

extern int numthreads;

extern void Sys_PrintHeading(char *heading);
extern void Sys_PrintHeadingVerbose(char *heading);

extern void DoProgress(char label[], int step, int total, qboolean verbose);
extern void printLabelledProgress(char *label, double current, double max);
extern void printLabelledProgressVerbose(char *label, double current, double max);
extern void printProgress(double percentage);
extern void printProgressVerbose(double percentage);
extern void printDetailedProgress(double percentage);
extern void printDetailedProgressVerbose(double percentage);

extern char *va(const char *format, ...);

extern inline void setcolor(int textcol, int backcol);

void JA_ParseEntities (void);
void JA_PrintBSPFileSizes(void);

void GetLeafNums (void);

//=============================================================================

#define WCONVEX_EPSILON		0.5

extern int				q3_nummodels;
q3_dmodel_t		*ja_dmodels;//[MAX_MAP_MODELS];

extern int				q3_numShaders;
q3_dshader_t	*ja_dshaders;//[JA_MAX_MAP_SHADERS];

extern int				q3_entdatasize;
char			*ja_dentdata;//[JA_MAX_MAP_ENTSTRING];

extern int				q3_numleafs;
q3_dleaf_t		*ja_dleafs;//[JA_MAX_MAP_LEAFS];

extern int				q3_numplanes;
q3_dplane_t		*ja_dplanes;//[JA_MAX_MAP_PLANES];

extern int				q3_numnodes;
q3_dnode_t		*ja_dnodes;//[JA_MAX_MAP_NODES];

extern int				q3_numleafsurfaces;
int				*ja_dleafsurfaces;//[JA_MAX_MAP_LEAFFACES];

extern int				q3_numleafbrushes;
int				*ja_dleafbrushes;//[JA_MAX_MAP_LEAFBRUSHES];

extern int				q3_numbrushes;
q3_dbrush_t		*ja_dbrushes;//[JA_MAX_MAP_BRUSHES];

extern int				q3_numbrushsides;
ja_dbrushside_t	*ja_dbrushsides;//[JA_MAX_MAP_BRUSHSIDES];

extern int				q3_numLightBytes;
byte			*ja_lightBytes;//[JA_MAX_MAP_LIGHTING];

extern int				q3_numGridPoints;
byte			*ja_gridData;//[JA_MAX_MAP_LIGHTGRID];

extern int				q3_numVisBytes;
byte			*ja_visBytes;//[JA_MAX_MAP_VISIBILITY];

extern int				q3_numDrawVerts;
ja_drawVert_t	*ja_drawVerts;//[JA_MAX_MAP_DRAW_VERTS];

extern int				q3_numDrawIndexes;
int				*ja_drawIndexes;//[JA_MAX_MAP_DRAW_INDEXES];

extern int				q3_numDrawSurfaces;
ja_dsurface_t	*ja_drawSurfaces;//[JA_MAX_MAP_DRAW_SURFS];

extern int				q3_numFogs;
q3_dfog_t		*ja_dfogs;//[JA_MAX_MAP_FOGS];

char			ja_dbrushsidetextured[JA_MAX_MAP_BRUSHSIDES];

extern qboolean forcesidesvisible;

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void JA_FreeMaxBSP(void)
{
	if (ja_dmodels) FreeMemory(ja_dmodels);
	ja_dmodels = NULL;
	q3_nummodels = 0;
	if (ja_dshaders) FreeMemory(ja_dshaders);
	ja_dshaders = NULL;
	q3_numShaders = 0;
	if (ja_dentdata) FreeMemory(ja_dentdata);
	ja_dentdata = NULL;
	q3_entdatasize = 0;
	if (ja_dleafs) FreeMemory(ja_dleafs);
	ja_dleafs = NULL;
	q3_numleafs = 0;
	if (ja_dplanes) FreeMemory(ja_dplanes);
	ja_dplanes = NULL;
	q3_numplanes = 0;
	if (ja_dnodes) FreeMemory(ja_dnodes);
	ja_dnodes = NULL;
	q3_numnodes = 0;
	if (ja_dleafsurfaces) FreeMemory(ja_dleafsurfaces);
	ja_dleafsurfaces = NULL;
	q3_numleafsurfaces = 0;
	if (ja_dleafbrushes) FreeMemory(ja_dleafbrushes);
	ja_dleafbrushes = NULL;
	q3_numleafbrushes = 0;
	if (ja_dbrushes) FreeMemory(ja_dbrushes);
	ja_dbrushes = NULL;
	q3_numbrushes = 0;
	if (ja_dbrushsides) FreeMemory(ja_dbrushsides);
	ja_dbrushsides = NULL;
	q3_numbrushsides = 0;
	if (ja_lightBytes) FreeMemory(ja_lightBytes);
	ja_lightBytes = NULL;
	q3_numLightBytes = 0;
	if (ja_gridData) FreeMemory(ja_gridData);
	ja_gridData = NULL;
	q3_numGridPoints = 0;
	if (ja_visBytes) FreeMemory(ja_visBytes);
	ja_visBytes = NULL;
	q3_numVisBytes = 0;
	if (ja_drawVerts) FreeMemory(ja_drawVerts);
	ja_drawVerts = NULL;
	q3_numDrawVerts = 0;
	if (ja_drawIndexes) FreeMemory(ja_drawIndexes);
	ja_drawIndexes = NULL;
	q3_numDrawIndexes = 0;
	if (ja_drawSurfaces) FreeMemory(ja_drawSurfaces);
	ja_drawSurfaces = NULL;
	q3_numDrawSurfaces = 0;
	if (ja_dfogs) FreeMemory(ja_dfogs);
	ja_dfogs = NULL;
	q3_numFogs = 0;
} //end of the function JA_FreeMaxBSP


//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void JA_PlaneFromPoints(vec3_t p0, vec3_t p1, vec3_t p2, vec3_t normal, float *dist)
{
	vec3_t t1, t2;

	VectorSubtract(p0, p1, t1);
	VectorSubtract(p2, p1, t2);
	CrossProduct(t1, t2, normal);
	VectorNormalize(normal);

	*dist = DotProduct(p0, normal);
} //end of the function PlaneFromPoints
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void JA_SurfacePlane(ja_dsurface_t *surface, vec3_t normal, float *dist)
{
	int i;
	float *p0, *p1, *p2;
	vec3_t t1, t2;

	p0 = ja_drawVerts[surface->firstVert].xyz;
	for (i = 1; i < surface->numVerts-1; i++)
	{
		p1 = ja_drawVerts[surface->firstVert + ((i) % surface->numVerts)].xyz;
		p2 = ja_drawVerts[surface->firstVert + ((i+1) % surface->numVerts)].xyz;
		VectorSubtract(p0, p1, t1);
		VectorSubtract(p2, p1, t2);
		CrossProduct(t1, t2, normal);
		VectorNormalize(normal);
		if (VectorLength(normal)) break;
	} //end for*/
/*
	float dot;
	for (i = 0; i < surface->numVerts; i++)
	{
		p0 = ja_drawVerts[surface->firstVert + ((i) % surface->numVerts)].xyz;
		p1 = ja_drawVerts[surface->firstVert + ((i+1) % surface->numVerts)].xyz;
		p2 = ja_drawVerts[surface->firstVert + ((i+2) % surface->numVerts)].xyz;
		VectorSubtract(p0, p1, t1);
		VectorSubtract(p2, p1, t2);
		VectorNormalize(t1);
		VectorNormalize(t2);
		dot = DotProduct(t1, t2);
		if (dot > -0.9 && dot < 0.9 &&
			VectorLength(t1) > 0.1 && VectorLength(t2) > 0.1) break;
	} //end for
	CrossProduct(t1, t2, normal);
	VectorNormalize(normal);
*/
	if (VectorLength(normal) < 0.9)
	{
		printf("surface %td bogus normal vector %f %f %f\n", surface - ja_drawSurfaces, normal[0], normal[1], normal[2]);
		printf("t1 = %f %f %f, t2 = %f %f %f\n", t1[0], t1[1], t1[2], t2[0], t2[1], t2[2]);
		for (i = 0; i < surface->numVerts; i++)
		{
			p1 = ja_drawVerts[surface->firstVert + ((i) % surface->numVerts)].xyz;
			Log_Print("p%d = %f %f %f\n", i, p1[0], p1[1], p1[2]);
		} //end for
	} //end if
	*dist = DotProduct(p0, normal);
} //end of the function JA_SurfacePlane
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
q3_dplane_t *ja_surfaceplanes;

void JA_CreatePlanarSurfacePlanes(void)
{
	int i;

	Log_Print("Creating planar surface planes...\n");
	ja_surfaceplanes = (q3_dplane_t *) GetClearedMemory(q3_numDrawSurfaces * sizeof(q3_dplane_t));

	for (i = 0; i < q3_numDrawSurfaces; i++)
	{
		printLabelledProgress("CreatePlanarSurfacePlanes", i, q3_numDrawSurfaces);

		ja_dsurface_t *surface = &ja_drawSurfaces[i];
		if (surface->surfaceType != MST_PLANAR) continue;
		JA_SurfacePlane(surface, ja_surfaceplanes[i].normal, &ja_surfaceplanes[i].dist);
		//Log_Print("normal = %f %f %f, dist = %f\n", ja_surfaceplanes[i].normal[0],
		//											ja_surfaceplanes[i].normal[1],
		//											ja_surfaceplanes[i].normal[2], ja_surfaceplanes[i].dist);
	} //end for
} //end of the function JA_CreatePlanarSurfacePlanes
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
/*
void JA_SurfacePlane(ja_dsurface_t *surface, vec3_t normal, float *dist)
{
	//take the plane information from the lightmap vector
	//VectorCopy(surface->lightmapVecs[2], normal);
	//calculate plane dist with first surface vertex
//	*dist = DotProduct(ja_drawVerts[surface->firstVert].xyz, normal);
	JA_PlaneFromPoints(ja_drawVerts[surface->firstVert].xyz,
						ja_drawVerts[surface->firstVert+1].xyz,
						ja_drawVerts[surface->firstVert+2].xyz, normal, dist);
} //end of the function JA_SurfacePlane*/
//===========================================================================
// returns the amount the face and the winding overlap
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
float JA_FaceOnWinding(ja_dsurface_t *surface, winding_t *winding)
{
	int i;
	float dist, area;
	q3_dplane_t plane;
	vec_t *v1, *v2;
	vec3_t normal, edgevec;
	winding_t *w;

	//copy the winding before chopping
	w = CopyWinding(winding);
	//retrieve the surface plane
	JA_SurfacePlane(surface, plane.normal, &plane.dist);
	//chop the winding with the surface edge planes
	for (i = 0; i < surface->numVerts && w; i++)
	{
		v1 = ja_drawVerts[surface->firstVert + ((i) % surface->numVerts)].xyz;
		v2 = ja_drawVerts[surface->firstVert + ((i+1) % surface->numVerts)].xyz;
		//create a plane through the edge from v1 to v2, orthogonal to the
		//surface plane and with the normal vector pointing inward
		VectorSubtract(v2, v1, edgevec);
		CrossProduct(edgevec, plane.normal, normal);
		VectorNormalize(normal);
		dist = DotProduct(normal, v1);
		//
		ChopWindingInPlace(&w, normal, dist, -0.1); //CLIP_EPSILON
	} //end for
	if (w)
	{
		area = WindingArea(w);
		FreeWinding(w);
		return area;
	} //end if
	return 0;
} //end of the function JA_FaceOnWinding
//===========================================================================
// creates a winding for the given brush side on the given brush
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
winding_t *JA_BrushSideWinding(q3_dbrush_t *brush, ja_dbrushside_t *baseside)
{
	int i;
	q3_dplane_t *baseplane, *plane;
	winding_t *w;
	ja_dbrushside_t *side;
	
	//create a winding for the brush side with the given planenumber
	baseplane = &ja_dplanes[baseside->planeNum];
	w = BaseWindingForPlane(baseplane->normal, baseplane->dist);
	for (i = 0; i < brush->numSides && w; i++)
	{
		side = &ja_dbrushsides[brush->firstSide + i];
		//don't chop with the base plane
		if (side->planeNum == baseside->planeNum) continue;
		//also don't use planes that are almost equal
		plane = &ja_dplanes[side->planeNum];
		if (DotProduct(baseplane->normal, plane->normal) > 0.999
				&& fabs(baseplane->dist - plane->dist) < 0.01) continue;
		//
		plane = &ja_dplanes[side->planeNum^1];
		ChopWindingInPlace(&w, plane->normal, plane->dist, -0.1); //CLIP_EPSILON);
	} //end for
	return w;
} //end of the function JA_BrushSideWinding
//===========================================================================
// fix screwed brush texture references
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
qboolean WindingIsTiny(winding_t *w);

void JA_FindVisibleBrushSides(void)
{
	int i, numtextured, numsides, num_completed = 0;

	memset(ja_dbrushsidetextured, false, JA_MAX_MAP_BRUSHSIDES);
	//
	numsides = 0;
	//create planes for the planar surfaces
	JA_CreatePlanarSurfacePlanes();
	Log_Print("Searching visible brush sides...\n");
	//Log_Print("%6d brush sides", numsides);
	//go over all the brushes
#pragma omp parallel for num_threads(numthreads)
	for (i = 0; i < q3_numbrushes; i++)
	{
		int j, k, we;
		float dot;
		q3_dplane_t *plane;
		ja_dbrushside_t *brushside;
		q3_dbrush_t *brush;
		ja_dsurface_t *surface;
		winding_t *w;

		num_completed++;

#pragma omp critical
		{
			printLabelledProgress("FindVisibleBrushSides", num_completed, q3_numbrushes);
		}

		brush = &ja_dbrushes[i];
		//go over all the sides of the brush
		for (j = 0; j < brush->numSides; j++)
		{
			//qprintf("\r%6d", numsides++);
			numsides++;
			
			brushside = &ja_dbrushsides[brush->firstSide + j];
			//
			w = JA_BrushSideWinding(brush, brushside);
			if (!w)
			{
				ja_dbrushsidetextured[brush->firstSide + j] = true;
				continue;
			} //end if
			else
			{
				//RemoveEqualPoints(w, 0.2);
				if (WindingIsTiny(w))
				{
					FreeWinding(w);
					ja_dbrushsidetextured[brush->firstSide + j] = true;
					continue;
				} //end if
				else
				{
					we = WindingError(w);
					if (we == WE_NOTENOUGHPOINTS
						|| we == WE_SMALLAREA
						|| we == WE_POINTBOGUSRANGE
//						|| we == WE_NONCONVEX
						)
					{
						FreeWinding(w);
						ja_dbrushsidetextured[brush->firstSide + j] = true;
						continue;
					} //end if
				} //end else
			} //end else
			if (WindingArea(w) < 20)
			{
				ja_dbrushsidetextured[brush->firstSide + j] = true;
				continue;
			} //end if
			//find a face for texturing this brush
			for (k = 0; k < q3_numDrawSurfaces; k++)
			{
				surface = &ja_drawSurfaces[k];
				if (surface->surfaceType != MST_PLANAR) continue;
				//
				//JA_SurfacePlane(surface, plane.normal, &plane.dist);
				plane = &ja_surfaceplanes[k];
				//the surface plane and the brush side plane should be pretty much the same
				if (fabs(fabs(plane->dist) - fabs(ja_dplanes[brushside->planeNum].dist)) > 5) continue;
				dot = DotProduct(plane->normal, ja_dplanes[brushside->planeNum].normal);
				if (dot > -0.9 && dot < 0.9) continue;
				//if the face is partly or totally on the brush side
				if (JA_FaceOnWinding(surface, w))
				{
					ja_dbrushsidetextured[brush->firstSide + j] = true;
					//Log_Write("JA_FaceOnWinding");
					break;
				} //end if
			} //end for
			FreeWinding(w);
		} //end for
	} //end for
	
	printLabelledProgress("FindVisibleBrushSides", q3_numbrushes, q3_numbrushes);

	qprintf("\r%6d brush sides\n", numsides);
	numtextured = 0;
	for (i = 0; i < q3_numbrushsides; i++)
	{
		if (forcesidesvisible) ja_dbrushsidetextured[i] = true;
		if (ja_dbrushsidetextured[i]) numtextured++;
	} //end for
	Log_Print("%d brush sides textured out of %d\n", numtextured, q3_numbrushsides);
} //end of the function JA_FindVisibleBrushSides

/*
=============
JA_SwapBlock

If all values are 32 bits, this can be used to swap everything
=============
*/
void JA_SwapBlock( int *block, int sizeOfBlock ) {
	int		i;

	sizeOfBlock >>= 2;
	for ( i = 0 ; i < sizeOfBlock ; i++ ) {
		block[i] = LittleLong( block[i] );
	}
} //end of the function JA_SwapBlock

/*
=============
JA_SwapBSPFile

Byte swaps all data in a bsp file.
=============
*/
void JA_SwapBSPFile( void ) {
	int				i;
	
	// models	
	JA_SwapBlock( (int *)ja_dmodels, q3_nummodels * sizeof( ja_dmodels[0] ) );

	// shaders (don't swap the name)
	for ( i = 0 ; i < q3_numShaders ; i++ ) {
		ja_dshaders[i].contentFlags = LittleLong( ja_dshaders[i].contentFlags );
		ja_dshaders[i].surfaceFlags = LittleLong( ja_dshaders[i].surfaceFlags );
	}

	// planes
	JA_SwapBlock( (int *)ja_dplanes, q3_numplanes * sizeof( ja_dplanes[0] ) );
	
	// nodes
	JA_SwapBlock( (int *)ja_dnodes, q3_numnodes * sizeof( ja_dnodes[0] ) );

	// leafs
	JA_SwapBlock( (int *)ja_dleafs, q3_numleafs * sizeof( ja_dleafs[0] ) );

	// leaffaces
	JA_SwapBlock( (int *)ja_dleafsurfaces, q3_numleafsurfaces * sizeof( ja_dleafsurfaces[0] ) );

	// leafbrushes
	JA_SwapBlock( (int *)ja_dleafbrushes, q3_numleafbrushes * sizeof( ja_dleafbrushes[0] ) );

	// brushes
	JA_SwapBlock( (int *)ja_dbrushes, q3_numbrushes * sizeof( ja_dbrushes[0] ) );

	// brushsides
	JA_SwapBlock( (int *)ja_dbrushsides, q3_numbrushsides * sizeof( ja_dbrushsides[0] ) );

	// vis
	((int *)&ja_visBytes)[0] = LittleLong( ((int *)&ja_visBytes)[0] );
	((int *)&ja_visBytes)[1] = LittleLong( ((int *)&ja_visBytes)[1] );

	// drawverts (don't swap colors )
	for ( i = 0 ; i < q3_numDrawVerts ; i++ ) {
		for (int j = 0; j < 4; j++)
		{
			ja_drawVerts[i].lightmap[j][0] = LittleFloat(ja_drawVerts[i].lightmap[j][0]);
			ja_drawVerts[i].lightmap[j][1] = LittleFloat(ja_drawVerts[i].lightmap[j][1]);
		}
		ja_drawVerts[i].st[0] = LittleFloat( ja_drawVerts[i].st[0] );
		ja_drawVerts[i].st[1] = LittleFloat( ja_drawVerts[i].st[1] );
		ja_drawVerts[i].xyz[0] = LittleFloat( ja_drawVerts[i].xyz[0] );
		ja_drawVerts[i].xyz[1] = LittleFloat( ja_drawVerts[i].xyz[1] );
		ja_drawVerts[i].xyz[2] = LittleFloat( ja_drawVerts[i].xyz[2] );
		ja_drawVerts[i].normal[0] = LittleFloat( ja_drawVerts[i].normal[0] );
		ja_drawVerts[i].normal[1] = LittleFloat( ja_drawVerts[i].normal[1] );
		ja_drawVerts[i].normal[2] = LittleFloat( ja_drawVerts[i].normal[2] );
	}

	// drawindexes
	JA_SwapBlock( (int *)ja_drawIndexes, q3_numDrawIndexes * sizeof( ja_drawIndexes[0] ) );

	// drawsurfs
	JA_SwapBlock( (int *)ja_drawSurfaces, q3_numDrawSurfaces * sizeof( ja_drawSurfaces[0] ) );

	// fogs
	for ( i = 0 ; i < q3_numFogs ; i++ ) {
		ja_dfogs[i].brushNum = LittleLong( ja_dfogs[i].brushNum );
	}
}



/*
=============
JA_CopyLump
=============
*/
int JA_CopyLump( q3_dheader_t	*header, int lump, void **dest, int size ) {
	int		length, ofs;

	length = header->lumps[lump].filelen;
	ofs = header->lumps[lump].fileofs;
	
	/*if ( length % size ) {
		Error ("JA_LoadBSPFile: odd lump size");
	}*/

	*dest = GetMemory(length);

	memcpy( *dest, (byte *)header + ofs, length );

	return length / size;
}

/*
=============
JA_CountTriangles
=============
*/
void JA_CountTriangles( void ) {
	int i, numTris, numPatchTris;
	ja_dsurface_t *surface;

	numTris = numPatchTris = 0;
	for ( i = 0; i < q3_numDrawSurfaces; i++ ) {
		surface = &ja_drawSurfaces[i];

		numTris += surface->numIndexes / 3;

		if ( surface->patchWidth ) {
			numPatchTris += surface->patchWidth * surface->patchHeight * 2;
		}
	}

	Log_Print( "%6d triangles\n", numTris );
	Log_Print( "%6d patch tris\n", numPatchTris );
}

/*
=============
JA_LoadBSPFile
=============
*/
void	JA_LoadBSPFile(struct quakefile_s *qf)
{
	q3_dheader_t	*header;

	// load the file header
	//LoadFile(filename, (void **)&header, offset, length);
	//
	LoadQuakeFile(qf, (void **)&header);

	// swap the header
	JA_SwapBlock( (int *)header, sizeof(*header) );

	if ( header->ident != JA_BSP_IDENT && header->ident != QL_BSP_IDENT ) {
		Error( "%s is not a RBSP file", qf->filename );
	}
	if ( header->version != JA_BSP_VERSION && header->version != QL_BSP_VERSION ) {
		Error( "%s is version %i, not (%i or %i)", qf->filename, header->version, JA_BSP_VERSION, QL_BSP_VERSION );
	}

	//printf("Shaders\n");
	q3_numShaders = JA_CopyLump( header, JA_LUMP_SHADERS, (void *) &ja_dshaders, sizeof(q3_dshader_t) );
	//printf("Models\n");
	q3_nummodels = JA_CopyLump( header, JA_LUMP_MODELS, (void *) &ja_dmodels, sizeof(q3_dmodel_t) );
	//printf("Planes\n");
	q3_numplanes = JA_CopyLump( header, JA_LUMP_PLANES, (void *) &ja_dplanes, sizeof(q3_dplane_t) );
	//printf("Leafs\n");
	q3_numleafs = JA_CopyLump( header, JA_LUMP_LEAFS, (void *) &ja_dleafs, sizeof(q3_dleaf_t) );
	//printf("Nodes\n");
	q3_numnodes = JA_CopyLump( header, JA_LUMP_NODES, (void *) &ja_dnodes, sizeof(q3_dnode_t) );
	//printf("LeafSurfaces\n");
	q3_numleafsurfaces = JA_CopyLump( header, JA_LUMP_LEAFSURFACES, (void *) &ja_dleafsurfaces, sizeof(ja_dleafsurfaces[0]) );
	//printf("LeafBrushes\n");
	q3_numleafbrushes = JA_CopyLump( header, JA_LUMP_LEAFBRUSHES, (void *) &ja_dleafbrushes, sizeof(ja_dleafbrushes[0]) );
	//printf("Brushes\n");
	q3_numbrushes = JA_CopyLump( header, JA_LUMP_BRUSHES, (void *) &ja_dbrushes, sizeof(q3_dbrush_t) );
	//printf("BrushSides\n");
	q3_numbrushsides = JA_CopyLump( header, JA_LUMP_BRUSHSIDES, (void *) &ja_dbrushsides, sizeof(ja_dbrushside_t) );
	//printf("DrawVerts\n");
	q3_numDrawVerts = JA_CopyLump( header, JA_LUMP_DRAWVERTS, (void *) &ja_drawVerts, sizeof(ja_drawVert_t) );
	//printf("DrawSurfaces\n");
	q3_numDrawSurfaces = JA_CopyLump( header, JA_LUMP_SURFACES, (void *) &ja_drawSurfaces, sizeof(ja_dsurface_t) );
	//printf("Fogs\n");
	q3_numFogs = JA_CopyLump( header, JA_LUMP_FOGS, (void *) &ja_dfogs, sizeof(q3_dfog_t) );
	//printf("DrawIndexes\n");
	q3_numDrawIndexes = JA_CopyLump( header, JA_LUMP_DRAWINDEXES, (void *) &ja_drawIndexes, sizeof(ja_drawIndexes[0]) );

	//printf("VisBytes\n");
	q3_numVisBytes = JA_CopyLump( header, JA_LUMP_VISIBILITY, (void *) &ja_visBytes, 1 );
	//printf("LightBytes\n");
	q3_numLightBytes = JA_CopyLump( header, JA_LUMP_LIGHTMAPS, (void *) &ja_lightBytes, 1 );
	//printf("EntData\n");
	q3_entdatasize = JA_CopyLump( header, JA_LUMP_ENTITIES, (void *) &ja_dentdata, 1);

	//printf("GridPoints\n");
	q3_numGridPoints = JA_CopyLump( header, JA_LUMP_LIGHTGRID, (void *) &ja_gridData, 8 );

	JA_CountTriangles();

	FreeMemory( header );		// everything has been copied out
		
	// swap everything
	JA_SwapBSPFile();

	JA_FindVisibleBrushSides();

	//JA_PrintBSPFileSizes();
}


//============================================================================

/*
=============
JA_AddLump
=============
*/
void JA_AddLump( FILE *bspfile, q3_dheader_t *header, int lumpnum, void *data, int len ) {
	q3_lump_t *lump;

	lump = &header->lumps[lumpnum];
	
	lump->fileofs = LittleLong( ftell(bspfile) );
	lump->filelen = LittleLong( len );
	SafeWrite( bspfile, data, (len+3)&~3 );
}

/*
=============
JA_WriteBSPFile

Swaps the bsp file in place, so it should not be referenced again
=============
*/
void	JA_WriteBSPFile( char *filename )
{
	q3_dheader_t	outheader, *header;
	FILE		*bspfile;

	header = &outheader;
	memset( header, 0, sizeof(q3_dheader_t) );
	
	JA_SwapBSPFile();

	header->ident = LittleLong( JA_BSP_IDENT );
	header->version = LittleLong( JA_BSP_VERSION );
	
	bspfile = SafeOpenWrite( filename );
	SafeWrite( bspfile, header, sizeof(q3_dheader_t) );	// overwritten later

	JA_AddLump( bspfile, header, JA_LUMP_SHADERS, ja_dshaders, q3_numShaders*sizeof(q3_dshader_t) );
	JA_AddLump( bspfile, header, JA_LUMP_PLANES, ja_dplanes, q3_numplanes*sizeof(q3_dplane_t) );
	JA_AddLump( bspfile, header, JA_LUMP_LEAFS, ja_dleafs, q3_numleafs*sizeof(q3_dleaf_t) );
	JA_AddLump( bspfile, header, JA_LUMP_NODES, ja_dnodes, q3_numnodes*sizeof(q3_dnode_t) );
	JA_AddLump( bspfile, header, JA_LUMP_BRUSHES, ja_dbrushes, q3_numbrushes*sizeof(q3_dbrush_t) );
	JA_AddLump( bspfile, header, JA_LUMP_BRUSHSIDES, ja_dbrushsides, q3_numbrushsides*sizeof(ja_dbrushside_t) );
	JA_AddLump( bspfile, header, JA_LUMP_LEAFSURFACES, ja_dleafsurfaces, q3_numleafsurfaces*sizeof(ja_dleafsurfaces[0]) );
	JA_AddLump( bspfile, header, JA_LUMP_LEAFBRUSHES, ja_dleafbrushes, q3_numleafbrushes*sizeof(ja_dleafbrushes[0]) );
	JA_AddLump( bspfile, header, JA_LUMP_MODELS, ja_dmodels, q3_nummodels*sizeof(q3_dmodel_t) );
	JA_AddLump( bspfile, header, JA_LUMP_DRAWVERTS, ja_drawVerts, q3_numDrawVerts*sizeof(ja_drawVert_t) );
	JA_AddLump( bspfile, header, JA_LUMP_SURFACES, ja_drawSurfaces, q3_numDrawSurfaces*sizeof(ja_dsurface_t) );
	JA_AddLump( bspfile, header, JA_LUMP_VISIBILITY, ja_visBytes, q3_numVisBytes );
	JA_AddLump( bspfile, header, JA_LUMP_LIGHTMAPS, ja_lightBytes, q3_numLightBytes );
	JA_AddLump( bspfile, header, JA_LUMP_LIGHTGRID, ja_gridData, 8 * q3_numGridPoints );
	JA_AddLump( bspfile, header, JA_LUMP_ENTITIES, ja_dentdata, q3_entdatasize );
	JA_AddLump( bspfile, header, JA_LUMP_FOGS, ja_dfogs, q3_numFogs * sizeof(q3_dfog_t) );
	JA_AddLump( bspfile, header, JA_LUMP_DRAWINDEXES, ja_drawIndexes, q3_numDrawIndexes * sizeof(ja_drawIndexes[0]) );
	
	fseek (bspfile, 0, SEEK_SET);
	SafeWrite (bspfile, header, sizeof(q3_dheader_t));
	fclose (bspfile);	
}

//============================================================================

/*
=============
JA_PrintBSPFileSizes

Dumps info about current file
=============
*/
void JA_PrintBSPFileSizes( void )
{
	if ( !num_entities )
	{
		JA_ParseEntities();
	}

	Sys_PrintHeading("--- BSP Info ---\n");

	Log_Print ("%6i models       %7i\n"
		,q3_nummodels, (int)(q3_nummodels*sizeof(q3_dmodel_t)));
	Log_Print ("%6i shaders      %7i\n"
		,q3_numShaders, (int)(q3_numShaders*sizeof(q3_dshader_t)));
	Log_Print ("%6i brushes      %7i\n"
		,q3_numbrushes, (int)(q3_numbrushes*sizeof(q3_dbrush_t)));
	Log_Print ("%6i brushsides   %7i\n"
		,q3_numbrushsides, (int)(q3_numbrushsides*sizeof(ja_dbrushside_t)));
	Log_Print ("%6i fogs         %7i\n"
		,q3_numFogs, (int)(q3_numFogs*sizeof(q3_dfog_t)));
	Log_Print ("%6i planes       %7i\n"
		,q3_numplanes, (int)(q3_numplanes*sizeof(q3_dplane_t)));
	Log_Print ("%6i entdata      %7i\n", num_entities, q3_entdatasize);

	Log_Print ("\n");

	Log_Print ("%6i nodes        %7i\n"
		,q3_numnodes, (int)(q3_numnodes*sizeof(q3_dnode_t)));
	Log_Print ("%6i leafs        %7i\n"
		,q3_numleafs, (int)(q3_numleafs*sizeof(q3_dleaf_t)));
	Log_Print ("%6i leafsurfaces %7i\n"
		,q3_numleafsurfaces, (int)(q3_numleafsurfaces*sizeof(ja_dleafsurfaces[0])));
	Log_Print ("%6i leafbrushes  %7i\n"
		,q3_numleafbrushes, (int)(q3_numleafbrushes*sizeof(ja_dleafbrushes[0])));
	Log_Print ("%6i drawverts    %7i\n"
		,q3_numDrawVerts, (int)(q3_numDrawVerts*sizeof(ja_drawVerts[0])));
	Log_Print ("%6i drawindexes  %7i\n"
		,q3_numDrawIndexes, (int)(q3_numDrawIndexes*sizeof(ja_drawIndexes[0])));
	Log_Print ("%6i drawsurfaces %7i\n"
		,q3_numDrawSurfaces, (int)(q3_numDrawSurfaces*sizeof(ja_drawSurfaces[0])));

	Log_Print ("%6i lightmaps    %7i\n"
		,q3_numLightBytes / (LIGHTMAP_WIDTH*LIGHTMAP_HEIGHT*3), q3_numLightBytes );
	Log_Print ("       visibility   %7i\n"
		, q3_numVisBytes );
}

/*
================
JA_ParseEntities

Parses the ja_dentdata string into entities
================
*/
void JA_ParseEntities (void)
{
	script_t *script;

	num_entities = 0;
	script = LoadScriptMemory(ja_dentdata, q3_entdatasize, "*Quake3 bsp file");
	SetScriptFlags(script, SCFL_NOSTRINGWHITESPACES |
									SCFL_NOSTRINGESCAPECHARS);

	while(ParseEntity(script))
	{
	} //end while

	FreeScript(script);
} //end of the function JA_ParseEntities


/*
================
JA_UnparseEntities

Generates the ja_dentdata string from all the entities
================
*/
void JA_UnparseEntities (void)
{
	char *buf, *end;
	epair_t *ep;
	char line[2048];
	int i;
	
	buf = ja_dentdata;
	end = buf;
	*end = 0;
	
	for (i=0 ; i<num_entities ; i++)
	{
		ep = entities[i].epairs;
		if (!ep)
			continue;	// ent got removed
		
		strcat (end,"{\n");
		end += 2;
				
		for (ep = entities[i].epairs ; ep ; ep=ep->next)
		{
			sprintf (line, "\"%s\" \"%s\"\n", ep->key, ep->value);
			strcat (end, line);
			end += strlen(line);
		}
		strcat (end,"}\n");
		end += 2;

		if (end > buf + JA_MAX_MAP_ENTSTRING)
			Error ("Entity text too long");
	}
	q3_entdatasize = end - buf + 1;
} //end of the function JA_UnparseEntities


