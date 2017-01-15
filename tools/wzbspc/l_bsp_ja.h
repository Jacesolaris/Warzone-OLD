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

#include "q3files.h"
//#include "surfaceflags.h"

#define JA_BSP_IDENT	(('P'<<24)+('S'<<16)+('B'<<8)+'R')
#define JA_BSP_VERSION	1

#define	JA_LUMP_ENTITIES		0
#define	JA_LUMP_SHADERS		1
#define	JA_LUMP_PLANES			2
#define	JA_LUMP_NODES			3
#define	JA_LUMP_LEAFS			4
#define	JA_LUMP_LEAFSURFACES	5
#define	JA_LUMP_LEAFBRUSHES	6
#define	JA_LUMP_MODELS			7
#define	JA_LUMP_BRUSHES		8
#define	JA_LUMP_BRUSHSIDES		9
#define	JA_LUMP_DRAWVERTS		10
#define	JA_LUMP_DRAWINDEXES	11
#define	JA_LUMP_FOGS			12
#define	JA_LUMP_SURFACES		13
#define	JA_LUMP_LIGHTMAPS		14
#define	JA_LUMP_LIGHTGRID		15
#define	JA_LUMP_VISIBILITY		16
#define JA_LUMP_LIGHTARRAY		17
#define	JA_HEADER_LUMPS		18

#define	JA_MAX_MAP_MODELS			0x1000
#define	JA_MAX_MAP_BRUSHES			0x200000//0x100000//0x400000
#define	JA_MAX_MAP_ENTITIES		0x80000
#define	JA_MAX_MAP_ENTSTRING		0x80000
#define	JA_MAX_MAP_SHADERS			0x800

#define	JA_MAX_MAP_AREAS			0x100		/* MAX_MAP_AREA_BYTES in q_shared must match! */
#define	JA_MAX_MAP_FOGS			30
#define	JA_MAX_MAP_PLANES			0x4000000//0x7000000
#define	JA_MAX_MAP_NODES			0x800000//0xA00000
#define	JA_MAX_MAP_BRUSHSIDES		0x2000000//0x1000000//0xF00000
#define	JA_MAX_MAP_LEAFS			0x800000//0xA00000
#define	JA_MAX_MAP_LEAFFACES		0x800000//0xA00000
#define	JA_MAX_MAP_LEAFBRUSHES		0x800000//0xA00000
#define	JA_MAX_MAP_PORTALS			0x800000//0xA00000
#ifndef __BASEJKA_LIGHTGRID__
#define	JA_MAX_MAP_LIGHTGRID		0x800000//0xF00000
#else //__BASEJKA_LIGHTGRID__
#define	JA_MAX_MAP_LIGHTGRID		65535 // matching JKA
#endif //__BASEJKA_LIGHTGRID__
#define	JA_MAX_MAP_VISIBILITY		0x3FFFFFFF

#define	JA_MAX_MAP_DRAW_SURFS		0x200000
#define	JA_MAX_MAP_DRAW_VERTS		0x4000000
#define	JA_MAX_MAP_DRAW_INDEXES	0x4000000

#define JA_MAX_LIGHTMAPS 4

typedef struct {
	int			planeNum;			/* positive plane side faces out of the leaf */
	int			shaderNum;
	int			surfaceNum;			/* RBSP */
} ja_dbrushside_t;

typedef struct {
	vec3_t		xyz;
	float		st[2];
	float		lightmap[JA_MAX_LIGHTMAPS][2];	/* RBSP */
	vec3_t		normal;
	byte		color[JA_MAX_LIGHTMAPS][4];	/* RBSP */
} ja_drawVert_t;

typedef struct {
	int			shaderNum;
	int			fogNum;
	int			surfaceType;

	int			firstVert;
	int			numVerts;

	int			firstIndex;
	int			numIndexes;

	byte		lightmapStyles[JA_MAX_LIGHTMAPS];						/* RBSP */
	byte		vertexStyles[JA_MAX_LIGHTMAPS];							/* RBSP */
	int			lightmapNum[JA_MAX_LIGHTMAPS];							/* RBSP */
	int			lightmapX[JA_MAX_LIGHTMAPS], lightmapY[JA_MAX_LIGHTMAPS];	/* RBSP */
	int			lightmapWidth, lightmapHeight;

	vec3_t		lightmapOrigin;
	vec3_t		lightmapVecs[3];	/* on patches, [ 0 ] and [ 1 ] are lodbounds */

	int			patchWidth;
	int			patchHeight;
} ja_dsurface_t;

extern	int				q3_nummodels;
extern	q3_dmodel_t		*ja_dmodels;//[MAX_MAP_MODELS];

extern	int				q3_numShaders;
extern	q3_dshader_t	*ja_dshaders;//[Q3_MAX_MAP_SHADERS];

extern	int				q3_entdatasize;
extern	char			*ja_dentdata;//[Q3_MAX_MAP_ENTSTRING];

extern	int				q3_numleafs;
extern	q3_dleaf_t		*ja_dleafs;//[Q3_MAX_MAP_LEAFS];

extern	int				q3_numplanes;
extern	q3_dplane_t		*ja_dplanes;//[Q3_MAX_MAP_PLANES];

extern	int				q3_numnodes;
extern	q3_dnode_t		*ja_dnodes;//[Q3_MAX_MAP_NODES];

extern	int				q3_numleafsurfaces;
extern	int				*ja_dleafsurfaces;//[Q3_MAX_MAP_LEAFFACES];

extern	int				q3_numleafbrushes;
extern	int				*ja_dleafbrushes;//[Q3_MAX_MAP_LEAFBRUSHES];

extern	int				q3_numbrushes;
extern	q3_dbrush_t		*ja_dbrushes;//[Q3_MAX_MAP_BRUSHES];

extern	int				q3_numbrushsides;
extern	ja_dbrushside_t	*ja_dbrushsides;//[Q3_MAX_MAP_BRUSHSIDES];

extern	int				q3_numLightBytes;
extern	byte			*ja_lightBytes;//[Q3_MAX_MAP_LIGHTING];

extern	int				q3_numGridPoints;
extern	byte			*ja_gridData;//[Q3_MAX_MAP_LIGHTGRID];

extern	int				q3_numVisBytes;
extern	byte			*ja_visBytes;//[Q3_MAX_MAP_VISIBILITY];

extern	int				q3_numDrawVerts;
extern	ja_drawVert_t	*ja_drawVerts;//[Q3_MAX_MAP_DRAW_VERTS];

extern	int				q3_numDrawIndexes;
extern	int				*ja_drawIndexes;//[Q3_MAX_MAP_DRAW_INDEXES];

extern	int				q3_numDrawSurfaces;
extern	ja_dsurface_t	*ja_drawSurfaces;//[Q3_MAX_MAP_DRAW_SURFS];

extern	int				q3_numFogs;
extern	q3_dfog_t		*ja_dfogs;//[Q3_MAX_MAP_FOGS];

extern	char			ja_dbrushsidetextured[JA_MAX_MAP_BRUSHSIDES];

void JA_LoadBSPFile(struct quakefile_s *qf);
void JA_FreeMaxBSP(void);
void JA_ParseEntities (void);
