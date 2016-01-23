#include "tr_local.h"
#include "../cgame/cg_public.h"

extern char currentMapName[128];

extern	world_t		s_worldData;


extern image_t	*R_FindImageFile( const char *name, imgType_t type, int flags );



#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_TERRAIN)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_ALL				(0xFFFFFFFFu)



#define MAP_INFO_TRACEMAP_SIZE 1024

vec3_t  MAP_INFO_MINS;
vec3_t  MAP_INFO_MAXS;
vec3_t	MAP_INFO_SIZE;
vec3_t	MAP_INFO_PIXELSIZE;
vec3_t	MAP_INFO_SCATTEROFFSET;

void Mapping_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, const int passEntityNum, const int contentmask )
{
	results->entityNum = ENTITYNUM_NONE;
	ri->CM_BoxTrace(results, start, end, mins, maxs, 0, contentmask, 0);
	results->entityNum = results->fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
}

void R_SetupMapInfo ( void )
{
	world_t	*w;
	int i;

	VectorSet(MAP_INFO_MINS, 128000, 128000, 128000);
	VectorSet(MAP_INFO_MAXS, -128000, -128000, -128000);

	w = &s_worldData;

	// Find the map min/maxs...
	for (i = 0; i < w->numsurfaces; i++)
	{
		msurface_t *surf = &w->surfaces[i];

		if (surf->cullinfo.type & CULLINFO_SPHERE)
		{
			vec3_t surfOrigin;

			VectorCopy(surf->cullinfo.localOrigin, surfOrigin);

			if (surfOrigin[0] < MAP_INFO_MINS[0])
				MAP_INFO_MINS[0] = surfOrigin[0];
			if (surfOrigin[0] > MAP_INFO_MAXS[0])
				MAP_INFO_MAXS[0] = surfOrigin[0];

			if (surfOrigin[1] < MAP_INFO_MINS[1])
				MAP_INFO_MINS[1] = surfOrigin[1];
			if (surfOrigin[1] > MAP_INFO_MAXS[1])
				MAP_INFO_MAXS[1] = surfOrigin[1];

			if (surfOrigin[2] < MAP_INFO_MINS[2])
				MAP_INFO_MINS[2] = surfOrigin[2];
			if (surfOrigin[2] > MAP_INFO_MAXS[2])
				MAP_INFO_MAXS[2] = surfOrigin[2];
		}
		else if (surf->cullinfo.type & CULLINFO_BOX)
		{
			if (surf->cullinfo.bounds[0][0] < MAP_INFO_MINS[0])
				MAP_INFO_MINS[0] = surf->cullinfo.bounds[0][0];
			if (surf->cullinfo.bounds[0][0] > MAP_INFO_MAXS[0])
				MAP_INFO_MAXS[0] = surf->cullinfo.bounds[0][0];

			if (surf->cullinfo.bounds[1][0] < MAP_INFO_MINS[0])
				MAP_INFO_MINS[0] = surf->cullinfo.bounds[1][0];
			if (surf->cullinfo.bounds[1][0] > MAP_INFO_MAXS[0])
				MAP_INFO_MAXS[0] = surf->cullinfo.bounds[1][0];

			if (surf->cullinfo.bounds[0][1] < MAP_INFO_MINS[1])
				MAP_INFO_MINS[1] = surf->cullinfo.bounds[0][1];
			if (surf->cullinfo.bounds[0][1] > MAP_INFO_MAXS[1])
				MAP_INFO_MAXS[1] = surf->cullinfo.bounds[0][1];

			if (surf->cullinfo.bounds[1][1] < MAP_INFO_MINS[1])
				MAP_INFO_MINS[1] = surf->cullinfo.bounds[1][1];
			if (surf->cullinfo.bounds[1][1] > MAP_INFO_MAXS[1])
				MAP_INFO_MAXS[1] = surf->cullinfo.bounds[1][1];

			if (surf->cullinfo.bounds[0][2] < MAP_INFO_MINS[2])
				MAP_INFO_MINS[2] = surf->cullinfo.bounds[0][2];
			if (surf->cullinfo.bounds[0][2] > MAP_INFO_MAXS[2])
				MAP_INFO_MAXS[2] = surf->cullinfo.bounds[0][2];

			if (surf->cullinfo.bounds[1][2] < MAP_INFO_MINS[2])
				MAP_INFO_MINS[2] = surf->cullinfo.bounds[1][2];
			if (surf->cullinfo.bounds[1][2] > MAP_INFO_MAXS[2])
				MAP_INFO_MAXS[2] = surf->cullinfo.bounds[1][2];
		}
	}

	// Move in from map edges a bit before starting traces...
	MAP_INFO_MINS[0] += 64.0;
	MAP_INFO_MINS[1] += 64.0;
	MAP_INFO_MINS[2] += 64.0;

	MAP_INFO_MAXS[0] -= 64.0;
	MAP_INFO_MAXS[1] -= 64.0;
	MAP_INFO_MAXS[2] -= 64.0;

	memset(MAP_INFO_SIZE, 0, sizeof(MAP_INFO_SIZE));
	memset(MAP_INFO_PIXELSIZE, 0, sizeof(MAP_INFO_PIXELSIZE));
	memset(MAP_INFO_SCATTEROFFSET, 0, sizeof(MAP_INFO_SCATTEROFFSET));

	MAP_INFO_SIZE[0] = MAP_INFO_MAXS[0] - MAP_INFO_MINS[0];
	MAP_INFO_SIZE[1] = MAP_INFO_MAXS[1] - MAP_INFO_MINS[1];
	MAP_INFO_SIZE[2] = MAP_INFO_MAXS[2] - MAP_INFO_MINS[2];
	MAP_INFO_PIXELSIZE[0] = MAP_INFO_TRACEMAP_SIZE / MAP_INFO_SIZE[0];
	MAP_INFO_PIXELSIZE[1] = MAP_INFO_TRACEMAP_SIZE / MAP_INFO_SIZE[1];
	MAP_INFO_SCATTEROFFSET[0] = MAP_INFO_SIZE[0] / MAP_INFO_TRACEMAP_SIZE;
	MAP_INFO_SCATTEROFFSET[1] = MAP_INFO_SIZE[1] / MAP_INFO_TRACEMAP_SIZE;
}

void R_CreateRandom2KImage ( void )
{
	// Hopefully now we have a map image... Save it...
	byte	data;
	int		i = 0;

	// write tga
	fileHandle_t f = ri->FS_FOpenFileWrite( "gfx/random2K.tga", qfalse);

	// header
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 0
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 1
	data = 2; ri->FS_Write( &data, sizeof(data), f );	// 2 : uncompressed type
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 3
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 4
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 5
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 6
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 7
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 8
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 9
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 10
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 11
	data = MAP_INFO_TRACEMAP_SIZE & 255; ri->FS_Write( &data, sizeof(data), f );	// 12 : width
	data = MAP_INFO_TRACEMAP_SIZE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 13 : width
	data = MAP_INFO_TRACEMAP_SIZE & 255; ri->FS_Write( &data, sizeof(data), f );	// 14 : height
	data = MAP_INFO_TRACEMAP_SIZE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 15 : height
	data = 32; ri->FS_Write( &data, sizeof(data), f );	// 16 : pixel size
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 17

	for( int i = 0; i < MAP_INFO_TRACEMAP_SIZE; i++ ) {
		for( int j = 0; j < MAP_INFO_TRACEMAP_SIZE; j++ ) {
			data = irand(0,255); ri->FS_Write( &data, sizeof(data), f );	// b
			data = irand(0,255); ri->FS_Write( &data, sizeof(data), f );	// g
			data = irand(0,255); ri->FS_Write( &data, sizeof(data), f );	// r
			data = irand(0,255); ri->FS_Write( &data, sizeof(data), f );	// a
		}
	}

	// footer
	i = 0; ri->FS_Write( &i, sizeof(i), f );	// extension area offset, 4 bytes
	i = 0; ri->FS_Write( &i, sizeof(i), f );	// developer directory offset, 4 bytes
	ri->FS_Write( "TRUEVISION-XFILE.\0", 18, f );

	ri->FS_FCloseFile( f );
}

void R_CreateGrassImages ( void )
{
#define GRASS_TEX_SCALE 200
#define GRASS_NUM_MASKS 10

	//
	// Generate mask images...
	//

	UINT8	*red[GRASS_TEX_SCALE];
	UINT8	*green[GRASS_TEX_SCALE];
	UINT8	*blue[GRASS_TEX_SCALE];
	UINT8	*alpha[GRASS_TEX_SCALE];

	for (int i = 0; i < GRASS_TEX_SCALE; i++)
	{
		red[i] = (UINT8*)malloc(sizeof(UINT8)*GRASS_TEX_SCALE);
		green[i] = (UINT8*)malloc(sizeof(UINT8)*GRASS_TEX_SCALE);
		blue[i] = (UINT8*)malloc(sizeof(UINT8)*GRASS_TEX_SCALE);
		alpha[i] = (UINT8*)malloc(sizeof(UINT8)*GRASS_TEX_SCALE);

		memset(red[i], 0, sizeof(UINT8)*GRASS_TEX_SCALE);
		memset(green[i], 0, sizeof(UINT8)*GRASS_TEX_SCALE);
		memset(blue[i], 0, sizeof(UINT8)*GRASS_TEX_SCALE);
		memset(alpha[i], 0, sizeof(UINT8)*GRASS_TEX_SCALE);
	}

	//Load NUM_DIFFERENT_LAYERS alpha maps into the material.
    //Each one has the same pixel pattern, but a few less total pixels.
    //This way, the grass gradually thins as it gets to the top.
	for (int l = 0; l < GRASS_NUM_MASKS; l++) 
	{
		//Thin the density as it approaches the top layer
		//The bottom layer will have 1000, the top layer 100.
		float density = l / (float) 60;
		int numGrass = (int) (4000 - ((3500 * density) + 500));

		//Generate the points
		for (int j = 0; j < numGrass; j++) 
		{
			int curPointX = irand(0, GRASS_TEX_SCALE-1);
			int curPointY = irand(0, GRASS_TEX_SCALE-1);

			green[curPointX][curPointY] = (1 - (density * 255));
		}

		// Hopefully now we have a map image... Save it...
		byte	data;
		int		i = 0;

		// write tga
		fileHandle_t f = ri->FS_FOpenFileWrite( va( "grassImage/grassMask%i.tga", (GRASS_NUM_MASKS-1)-l ), qfalse);

		// header
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 0
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 1
		data = 2; ri->FS_Write( &data, sizeof(data), f );	// 2 : uncompressed type
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 3
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 4
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 5
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 6
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 7
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 8
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 9
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 10
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 11
		data = GRASS_TEX_SCALE & 255; ri->FS_Write( &data, sizeof(data), f );	// 12 : width
		data = GRASS_TEX_SCALE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 13 : width
		data = GRASS_TEX_SCALE & 255; ri->FS_Write( &data, sizeof(data), f );	// 14 : height
		data = GRASS_TEX_SCALE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 15 : height
		data = 32; ri->FS_Write( &data, sizeof(data), f );	// 16 : pixel size
		data = 0; ri->FS_Write( &data, sizeof(data), f );	// 17

		for( int x = 0; x < GRASS_TEX_SCALE; x++ ) {
			for( int y = 0; y < GRASS_TEX_SCALE; y++ ) {
				data = 0; ri->FS_Write( &data, sizeof(data), f );	// b
				data = green[x][y]; ri->FS_Write( &data, sizeof(data), f );	// g
				data = 0; ri->FS_Write( &data, sizeof(data), f );	// r
				data = 255; ri->FS_Write( &data, sizeof(data), f );	// a
			}
		}

		// footer
		i = 0; ri->FS_Write( &i, sizeof(i), f );	// extension area offset, 4 bytes
		i = 0; ri->FS_Write( &i, sizeof(i), f );	// developer directory offset, 4 bytes
		ri->FS_Write( "TRUEVISION-XFILE.\0", 18, f );

		ri->FS_FCloseFile( f );
	}

	for (int i = 0; i < GRASS_TEX_SCALE; i++)
	{
		free(red[i]);
		free(green[i]);
		free(blue[i]);
		free(alpha[i]);
	}

	//
	// Generate grass image...
	//

	// Hopefully now we have a map image... Save it...
	byte	data;
	int		i = 0;

	// write tga
	fileHandle_t f = ri->FS_FOpenFileWrite( "grassImage/grassImage.tga", qfalse);

	// header
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 0
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 1
	data = 2; ri->FS_Write( &data, sizeof(data), f );	// 2 : uncompressed type
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 3
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 4
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 5
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 6
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 7
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 8
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 9
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 10
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 11
	data = GRASS_TEX_SCALE & 255; ri->FS_Write( &data, sizeof(data), f );	// 12 : width
	data = GRASS_TEX_SCALE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 13 : width
	data = GRASS_TEX_SCALE & 255; ri->FS_Write( &data, sizeof(data), f );	// 14 : height
	data = GRASS_TEX_SCALE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 15 : height
	data = 32; ri->FS_Write( &data, sizeof(data), f );	// 16 : pixel size
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 17

	for( int i = 0; i < GRASS_TEX_SCALE; i++ ) {
		for( int j = 0; j < GRASS_TEX_SCALE; j++ ) {
			data = irand(0,20)+15; ri->FS_Write( &data, sizeof(data), f );	// b
			data = irand(0,70)+45; ri->FS_Write( &data, sizeof(data), f );	// g
			data = irand(0,20)+20; ri->FS_Write( &data, sizeof(data), f );	// r
			data = irand(0,255); ri->FS_Write( &data, sizeof(data), f );	// a
		}
	}

	// footer
	i = 0; ri->FS_Write( &i, sizeof(i), f );	// extension area offset, 4 bytes
	i = 0; ri->FS_Write( &i, sizeof(i), f );	// developer directory offset, 4 bytes
	ri->FS_Write( "TRUEVISION-XFILE.\0", 18, f );

	ri->FS_FCloseFile( f );
}

void R_CreateBspMapImage ( void )
{
	// Now we have a mins/maxs for map, we can generate a map image...
	float	z;
	UINT8	*red[MAP_INFO_TRACEMAP_SIZE];
	UINT8	*green[MAP_INFO_TRACEMAP_SIZE];
	UINT8	*blue[MAP_INFO_TRACEMAP_SIZE];

	for (int i = 0; i < MAP_INFO_TRACEMAP_SIZE; i++)
	{
		red[i] = (UINT8*)malloc(sizeof(UINT8)*MAP_INFO_TRACEMAP_SIZE);
		green[i] = (UINT8*)malloc(sizeof(UINT8)*MAP_INFO_TRACEMAP_SIZE);
		blue[i] = (UINT8*)malloc(sizeof(UINT8)*MAP_INFO_TRACEMAP_SIZE);
	}

	// Create the map...
#pragma omp parallel for schedule(dynamic)
	for (int imageX = 0; imageX < MAP_INFO_TRACEMAP_SIZE; imageX++)
	{
		int x = MAP_INFO_MINS[0] + (imageX * MAP_INFO_SCATTEROFFSET[0]);

		for (int imageY = 0; imageY < MAP_INFO_TRACEMAP_SIZE; imageY++)
		{
			int y = MAP_INFO_MINS[1] + (imageY * MAP_INFO_SCATTEROFFSET[1]);

			for (z = MAP_INFO_MAXS[2]; z > MAP_INFO_MINS[2]; z -= 48.0)
			{
				trace_t		tr;
				vec3_t		pos, down;
				qboolean	FOUND = qfalse;

				VectorSet(pos, x, y, z);
				VectorSet(down, x, y, -65536);

				Mapping_Trace( &tr, pos, NULL, NULL, down, ENTITYNUM_NONE, /*MASK_ALL*/MASK_PLAYERSOLID|CONTENTS_WATER/*|CONTENTS_OPAQUE*/ );

				if (tr.startsolid || tr.allsolid)
				{// Try again from below this spot...
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					continue;
				}

				if (tr.endpos[2] <= MAP_INFO_MINS[2])
				{// Went off map...
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					break;
				}

				if ( tr.surfaceFlags & SURF_SKY )
				{// Sky...
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					continue;
				}

				/*
				if ( tr.surfaceFlags & SURF_NOIMPACT )
				{// don't make missile explosions
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					continue;
				}
				*/

				if ( tr.surfaceFlags & SURF_NODRAW )
				{// don't generate a drawsurface at all
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					continue;
				}

				/*
				if ( tr.surfaceFlags & SURF_NOSTEPS )
				{// no footstep sounds
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					continue;
				}

				if ( tr.surfaceFlags & SURF_NODLIGHT )
				{// don't dlight even if solid (solid lava, skies)
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					continue;
				}

				if ( tr.surfaceFlags & SURF_NOMISCENTS )
				{// no client models allowed on this surface
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					continue;
				}
				*/

				/*
				if ( tr.contents & CONTENTS_TRIGGER )
				{// Trigger hurt???
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					continue;
				}

				if (tr.contents & CONTENTS_DETAIL || tr.contents & CONTENTS_NODROP )
				{
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					continue;
				}
				*/

				/*
				if (tr.contents & CONTENTS_NOSHOT )
				{
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					continue;
				}
				*/

				/*
				if (tr.contents & CONTENTS_TRANSLUCENT)
				{
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					continue;
				}
				*/


				float DIST_FROM_ROOF = MAP_INFO_MAXS[2] - tr.endpos[2];
				float HEIGHT_COLOR_MULT = ((1.0 - (DIST_FROM_ROOF / MAP_INFO_SIZE[2])) * 0.5) + 0.5;

				if (tr.contents & CONTENTS_WATER)
				{
					red[imageX][imageY] = 0.3*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.6*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 1.0*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				}

				int MATERIAL_TYPE = (tr.surfaceFlags & MATERIAL_MASK);

				switch( MATERIAL_TYPE )
				{
				case MATERIAL_WATER:			// 13			// light covering of water on a surface
					red[imageX][imageY] = 0.3*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.6*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 1.0*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_SHORTGRASS:		// 5			// manicured lawn
				case MATERIAL_LONGGRASS:		// 6			// long jungle grass
					red[imageX][imageY] = 0.1*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.4*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.1*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_SAND:				// 8			// sandy beach
					red[imageX][imageY] = 0.8*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.8*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
#if 0
				case MATERIAL_CARPET:			// 27			// lush carpet
					red[imageX][imageY] = 0.7*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.7*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_GRAVEL:			// 9			// lots of small stones
					red[imageX][imageY] = 0.2*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.5*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.5*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_ROCK:				// 23			//
					red[imageX][imageY] = 0.6*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.6*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.6*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_TILES:			// 26			// tiled floor
					red[imageX][imageY] = 0.4*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.4*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.4*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_SOLIDWOOD:		// 1			// freshly cut timber
				case MATERIAL_HOLLOWWOOD:		// 2			// termite infested creaky wood
					red[imageX][imageY] = 0.2*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.7*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.2*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_SOLIDMETAL:		// 3			// solid girders
				case MATERIAL_HOLLOWMETAL:		// 4			// hollow metal machines -- UQ1: Used for weapons to force lower parallax and high reflection...
					red[imageX][imageY] = 0.9*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.9*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.9*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_DRYLEAVES:		// 19			// dried up leaves on the floor
				case MATERIAL_GREENLEAVES:		// 20			// fresh leaves still on a tree
					red[imageX][imageY] = 0.2*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.7*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.2*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_FABRIC:			// 21			// Cotton sheets
				case MATERIAL_CANVAS:			// 22			// tent material
					red[imageX][imageY] = 0.8*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.4*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.4*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_MARBLE:			// 12			// marble floors
					red[imageX][imageY] = 0.8*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.8*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.8*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
#endif
				case MATERIAL_SNOW:				// 14			// freshly laid snow
				case MATERIAL_ICE:				// 15			// packed snow/solid ice
					red[imageX][imageY] = 1*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 1*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 1*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_MUD:				// 17			// wet soil
				case MATERIAL_DIRT:				// 7			// hard mud
					red[imageX][imageY] = 0.1*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.2*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.1*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
#if 0
				case MATERIAL_CONCRETE:			// 11			// hardened concrete pavement
				case MATERIAL_PLASTER:			// 28			// drywall style plaster
					red[imageX][imageY] = 0.4*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.4*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.4*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_FLESH:			// 16			// hung meat, corpses in the world
					red[imageX][imageY] = 0.6*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.3*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.3*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_RUBBER:			// 24			// hard tire like rubber
				case MATERIAL_PLASTIC:			// 25			//
					red[imageX][imageY] = 0.1*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.1*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.1*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_ARMOR:			// 30			// body armor
					red[imageX][imageY] = 0.7*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.7*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.7*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_SHATTERGLASS:		// 29			// glass with the Crisis Zone style shattering
				case MATERIAL_GLASS:			// 10			//
				case MATERIAL_BPGLASS:			// 18			// bulletproof glass
					red[imageX][imageY] = 0.9*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.9*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.9*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				case MATERIAL_COMPUTER:			// 31			// computers/electronic equipment
					red[imageX][imageY] = 0.9*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.9*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.1*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
#endif
				default:
					/*
					red[imageX][imageY] = 0*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0*255*HEIGHT_COLOR_MULT;
					*/
					red[imageX][imageY] = 0.6*255*HEIGHT_COLOR_MULT;
					green[imageX][imageY] = 0.6*255*HEIGHT_COLOR_MULT;
					blue[imageX][imageY] = 0.6*255*HEIGHT_COLOR_MULT;
					FOUND = qtrue;
					break;
				}

				if (FOUND) break;
			}
		}
	}

	// Hopefully now we have a map image... Save it...
	byte data;

	// write tga
	fileHandle_t f = ri->FS_FOpenFileWrite( va( "mapImage/%s.tga", currentMapName ), qfalse);

	// header
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 0
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 1
	data = 2; ri->FS_Write( &data, sizeof(data), f );	// 2 : uncompressed type
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 3
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 4
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 5
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 6
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 7
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 8
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 9
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 10
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 11
	data = MAP_INFO_TRACEMAP_SIZE & 255; ri->FS_Write( &data, sizeof(data), f );	// 12 : width
	data = MAP_INFO_TRACEMAP_SIZE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 13 : width
	data = MAP_INFO_TRACEMAP_SIZE & 255; ri->FS_Write( &data, sizeof(data), f );	// 14 : height
	data = MAP_INFO_TRACEMAP_SIZE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 15 : height
	data = 32; ri->FS_Write( &data, sizeof(data), f );	// 16 : pixel size
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 17

	for( int i = 0; i < MAP_INFO_TRACEMAP_SIZE; i++ ) {
		for( int j = 0; j < MAP_INFO_TRACEMAP_SIZE; j++ ) {
			data = blue[i][j]; ri->FS_Write( &data, sizeof(data), f );	// b
			data = green[i][j]; ri->FS_Write( &data, sizeof(data), f );	// g
			data = red[i][j]; ri->FS_Write( &data, sizeof(data), f );	// r
			data = 255; ri->FS_Write( &data, sizeof(data), f );	// a
		}
	}

	int i;

	// footer
	i = 0; ri->FS_Write( &i, sizeof(i), f );	// extension area offset, 4 bytes
	i = 0; ri->FS_Write( &i, sizeof(i), f );	// developer directory offset, 4 bytes
	ri->FS_Write( "TRUEVISION-XFILE.\0", 18, f );

	ri->FS_FCloseFile( f );

	for (int i = 0; i < MAP_INFO_TRACEMAP_SIZE; i++)
	{
		free(red[i]);
		free(green[i]);
		free(blue[i]);
	}
}

void R_CreateHeightMapImage ( void )
{
	// Now we have a mins/maxs for map, we can generate a map image...
	float	z;
	UINT8	*red[MAP_INFO_TRACEMAP_SIZE];
	UINT8	*green[MAP_INFO_TRACEMAP_SIZE];
	UINT8	*blue[MAP_INFO_TRACEMAP_SIZE];
	UINT8	*alpha[MAP_INFO_TRACEMAP_SIZE];

	for (int i = 0; i < MAP_INFO_TRACEMAP_SIZE; i++)
	{
		red[i] = (UINT8*)malloc(sizeof(UINT8)*MAP_INFO_TRACEMAP_SIZE);
		green[i] = (UINT8*)malloc(sizeof(UINT8)*MAP_INFO_TRACEMAP_SIZE);
		blue[i] = (UINT8*)malloc(sizeof(UINT8)*MAP_INFO_TRACEMAP_SIZE);
		alpha[i] = (UINT8*)malloc(sizeof(UINT8)*MAP_INFO_TRACEMAP_SIZE);
	}

	// Create the map...
#pragma omp parallel for schedule(dynamic)
	//for (int x = (int)MAP_INFO_MINS[0]; x < (int)MAP_INFO_MAXS[0]; x += MAP_INFO_SCATTEROFFSET[0])
	for (int imageX = 0; imageX < MAP_INFO_TRACEMAP_SIZE; imageX++)
	{
		int x = MAP_INFO_MINS[0] + (imageX * MAP_INFO_SCATTEROFFSET[0]);

		for (int imageY = 0; imageY < MAP_INFO_TRACEMAP_SIZE; imageY++)
		{
			int y = MAP_INFO_MINS[1] + (imageY * MAP_INFO_SCATTEROFFSET[1]);

			for (z = MAP_INFO_MAXS[2]; z > MAP_INFO_MINS[2]; z -= 48.0)
			{
				trace_t		tr;
				vec3_t		pos, down;
				qboolean	FOUND = qfalse;

				VectorSet(pos, x, y, z);
				VectorSet(down, x, y, -65536);

				Mapping_Trace( &tr, pos, NULL, NULL, down, ENTITYNUM_NONE, /*MASK_ALL*/MASK_PLAYERSOLID|CONTENTS_WATER/*|CONTENTS_OPAQUE*/ );

				if (tr.startsolid || tr.allsolid)
				{// Try again from below this spot...
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					alpha[imageX][imageY] = 0;
					continue;
				}

				if (tr.endpos[2] <= MAP_INFO_MINS[2])
				{// Went off map...
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					alpha[imageX][imageY] = 0;
					break;
				}

				if ( tr.surfaceFlags & SURF_SKY )
				{// Sky...
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					alpha[imageX][imageY] = 0;
					continue;
				}

				if ( tr.surfaceFlags & SURF_NODRAW )
				{// don't generate a drawsurface at all
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					alpha[imageX][imageY] = 0;
					continue;
				}

				float DIST_FROM_ROOF = MAP_INFO_MAXS[2] - tr.endpos[2];
				float HEIGHT_COLOR_MULT = (1.0 - (DIST_FROM_ROOF / MAP_INFO_SIZE[2]));

				red[imageX][imageY] = tr.plane.normal[0]*255;		// surface plane X
				green[imageX][imageY] = tr.plane.normal[1]*255;		// surface plane Y
				blue[imageX][imageY] = tr.plane.normal[2]*255;		// surface plane Z
				alpha[imageX][imageY] = HEIGHT_COLOR_MULT*255;		// height map
				break;
			}
		}
	}

	// Hopefully now we have a map image... Save it...
	byte data;

	// write tga
	fileHandle_t f = ri->FS_FOpenFileWrite( va( "heightMapImage/%s.tga", currentMapName ), qfalse);

	// header
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 0
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 1
	data = 2; ri->FS_Write( &data, sizeof(data), f );	// 2 : uncompressed type
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 3
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 4
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 5
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 6
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 7
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 8
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 9
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 10
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 11
	data = MAP_INFO_TRACEMAP_SIZE & 255; ri->FS_Write( &data, sizeof(data), f );	// 12 : width
	data = MAP_INFO_TRACEMAP_SIZE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 13 : width
	data = MAP_INFO_TRACEMAP_SIZE & 255; ri->FS_Write( &data, sizeof(data), f );	// 14 : height
	data = MAP_INFO_TRACEMAP_SIZE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 15 : height
	data = 32; ri->FS_Write( &data, sizeof(data), f );	// 16 : pixel size
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 17

	for( int i = 0; i < MAP_INFO_TRACEMAP_SIZE; i++ ) {
		for( int j = 0; j < MAP_INFO_TRACEMAP_SIZE; j++ ) {
			data = blue[i][j]; ri->FS_Write( &data, sizeof(data), f );	// b
			data = green[i][j]; ri->FS_Write( &data, sizeof(data), f );	// g
			data = red[i][j]; ri->FS_Write( &data, sizeof(data), f );	// r
			data = alpha[i][j]; ri->FS_Write( &data, sizeof(data), f );	// a
		}
	}

	int i;

	// footer
	i = 0; ri->FS_Write( &i, sizeof(i), f );	// extension area offset, 4 bytes
	i = 0; ri->FS_Write( &i, sizeof(i), f );	// developer directory offset, 4 bytes
	ri->FS_Write( "TRUEVISION-XFILE.\0", 18, f );

	ri->FS_FCloseFile( f );

	for (int i = 0; i < MAP_INFO_TRACEMAP_SIZE; i++)
	{
		free(red[i]);
		free(green[i]);
		free(blue[i]);
		free(alpha[i]);
	}
}

void R_CreateFoliageMapImage ( void )
{
	// Now we have a mins/maxs for map, we can generate a map image...
	float	z;
	UINT8	*red[MAP_INFO_TRACEMAP_SIZE];
	UINT8	*green[MAP_INFO_TRACEMAP_SIZE];
	UINT8	*blue[MAP_INFO_TRACEMAP_SIZE];
	UINT8	*alpha[MAP_INFO_TRACEMAP_SIZE];

	for (int i = 0; i < MAP_INFO_TRACEMAP_SIZE; i++)
	{
		red[i] = (UINT8*)malloc(sizeof(UINT8)*MAP_INFO_TRACEMAP_SIZE);
		green[i] = (UINT8*)malloc(sizeof(UINT8)*MAP_INFO_TRACEMAP_SIZE);
		blue[i] = (UINT8*)malloc(sizeof(UINT8)*MAP_INFO_TRACEMAP_SIZE);
		alpha[i] = (UINT8*)malloc(sizeof(UINT8)*MAP_INFO_TRACEMAP_SIZE);
	}

	// Create the map...
#pragma omp parallel for schedule(dynamic)
	//for (int x = (int)MAP_INFO_MINS[0]; x < (int)MAP_INFO_MAXS[0]; x += MAP_INFO_SCATTEROFFSET[0])
	for (int imageX = 0; imageX < MAP_INFO_TRACEMAP_SIZE; imageX++)
	{
		int x = MAP_INFO_MINS[0] + (imageX * MAP_INFO_SCATTEROFFSET[0]);

		for (int imageY = 0; imageY < MAP_INFO_TRACEMAP_SIZE; imageY++)
		{
			int y = MAP_INFO_MINS[1] + (imageY * MAP_INFO_SCATTEROFFSET[1]);

			for (z = MAP_INFO_MAXS[2]; z > MAP_INFO_MINS[2]; z -= 48.0)
			{
				trace_t		tr;
				vec3_t		pos, down;
				qboolean	FOUND = qfalse;

				VectorSet(pos, x, y, z);
				VectorSet(down, x, y, -65536);

				Mapping_Trace( &tr, pos, NULL, NULL, down, ENTITYNUM_NONE, /*MASK_ALL*/MASK_PLAYERSOLID|CONTENTS_WATER/*|CONTENTS_OPAQUE*/ );

				if (tr.startsolid || tr.allsolid)
				{// Try again from below this spot...
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					alpha[imageX][imageY] = 0;
					continue;
				}

				if (tr.endpos[2] <= MAP_INFO_MINS[2])
				{// Went off map...
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					alpha[imageX][imageY] = 0;
					break;
				}

				if ( tr.surfaceFlags & SURF_SKY )
				{// Sky...
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					alpha[imageX][imageY] = 0;
					continue;
				}

				if ( tr.surfaceFlags & SURF_NODRAW )
				{// don't generate a drawsurface at all
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					alpha[imageX][imageY] = 0;
					continue;
				}

				float DIST_FROM_ROOF = MAP_INFO_MAXS[2] - tr.endpos[2];
				float HEIGHT_COLOR_MULT = (1.0 - (DIST_FROM_ROOF / MAP_INFO_SIZE[2]));

				int MATERIAL_TYPE = (tr.surfaceFlags & MATERIAL_MASK);

				switch( MATERIAL_TYPE )
				{
				case MATERIAL_SHORTGRASS:		// 5					// manicured lawn
				case MATERIAL_LONGGRASS:		// 6					// long jungle grass
				case MATERIAL_MUD:				// 17					// wet soil
				case MATERIAL_DIRT:				// 7					// hard mud
					red[imageX][imageY] = HEIGHT_COLOR_MULT*255;		// height map
					green[imageX][imageY] = irand(0,255);				// foliage option 1
					blue[imageX][imageY] = irand(0,255);				// foliage option 2
					alpha[imageX][imageY] = irand(0,255);				// foliage option 3
					FOUND = qtrue;
					break;
				default:
					red[imageX][imageY] = 0;
					green[imageX][imageY] = 0;
					blue[imageX][imageY] = 0;
					alpha[imageX][imageY] = 0;
					FOUND = qtrue;
					break;
				}

				if (FOUND) break;
			}
		}
	}

	// Hopefully now we have a map image... Save it...
	byte data;

	// write tga
	fileHandle_t f = ri->FS_FOpenFileWrite( va( "foliageMapImage/%s.tga", currentMapName ), qfalse);

	// header
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 0
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 1
	data = 2; ri->FS_Write( &data, sizeof(data), f );	// 2 : uncompressed type
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 3
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 4
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 5
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 6
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 7
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 8
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 9
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 10
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 11
	data = MAP_INFO_TRACEMAP_SIZE & 255; ri->FS_Write( &data, sizeof(data), f );	// 12 : width
	data = MAP_INFO_TRACEMAP_SIZE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 13 : width
	data = MAP_INFO_TRACEMAP_SIZE & 255; ri->FS_Write( &data, sizeof(data), f );	// 14 : height
	data = MAP_INFO_TRACEMAP_SIZE >> 8; ri->FS_Write( &data, sizeof(data), f );	// 15 : height
	data = 32; ri->FS_Write( &data, sizeof(data), f );	// 16 : pixel size
	data = 0; ri->FS_Write( &data, sizeof(data), f );	// 17

	for( int i = 0; i < MAP_INFO_TRACEMAP_SIZE; i++ ) {
		for( int j = 0; j < MAP_INFO_TRACEMAP_SIZE; j++ ) {
			data = blue[i][j]; ri->FS_Write( &data, sizeof(data), f );	// b
			data = green[i][j]; ri->FS_Write( &data, sizeof(data), f );	// g
			data = red[i][j]; ri->FS_Write( &data, sizeof(data), f );	// r
			data = alpha[i][j]; ri->FS_Write( &data, sizeof(data), f );	// a
		}
	}

	int i;

	// footer
	i = 0; ri->FS_Write( &i, sizeof(i), f );	// extension area offset, 4 bytes
	i = 0; ri->FS_Write( &i, sizeof(i), f );	// developer directory offset, 4 bytes
	ri->FS_Write( "TRUEVISION-XFILE.\0", 18, f );

	ri->FS_FCloseFile( f );

	for (int i = 0; i < MAP_INFO_TRACEMAP_SIZE; i++)
	{
		free(red[i]);
		free(green[i]);
		free(blue[i]);
		free(alpha[i]);
	}
}

void R_LoadMapInfo ( void )
{
	R_SetupMapInfo();

	if (!ri->FS_FileExists(va( "gfx/random2K.tga", currentMapName )))
	{
		R_CreateRandom2KImage();
		tr.random2KImage = R_FindImageFile("gfx/random2K.tga", IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
	}
	else
	{
		tr.random2KImage = R_FindImageFile("gfx/random2K.tga", IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
	}

#if 0
	if (!ri->FS_FileExists(va( "mapImage/%s.tga", currentMapName )))
	{
		R_CreateBspMapImage();
		tr.mapImage = R_FindImageFile(va( "mapImage/%s.tga", currentMapName ), IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
	}
	else
	{
		tr.mapImage = R_FindImageFile(va( "mapImage/%s.tga", currentMapName ), IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
	}

	if (!ri->FS_FileExists(va( "heightMapImage/%s.tga", currentMapName )))
	{
		R_CreateHeightMapImage();
		tr.foliageMapImage = R_FindImageFile(va( "heightMapImage/%s.tga", currentMapName ), IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
	}
	else
	{
		tr.foliageMapImage = R_FindImageFile(va( "heightMapImage/%s.tga", currentMapName ), IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
	}

	if (!ri->FS_FileExists(va( "foliageMapImage/%s.tga", currentMapName )))
	{
		R_CreateFoliageMapImage();
		tr.foliageMapImage = R_FindImageFile(va( "foliageMapImage/%s.tga", currentMapName ), IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
	}
	else
	{
		tr.foliageMapImage = R_FindImageFile(va( "foliageMapImage/%s.tga", currentMapName ), IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
	}

	if (!ri->FS_FileExists("grassImage/grassMask0.tga")
		|| !ri->FS_FileExists("grassImage/grassMask1.tga")
		|| !ri->FS_FileExists("grassImage/grassMask2.tga")
		|| !ri->FS_FileExists("grassImage/grassMask3.tga")
		|| !ri->FS_FileExists("grassImage/grassMask4.tga")
		|| !ri->FS_FileExists("grassImage/grassMask5.tga")
		|| !ri->FS_FileExists("grassImage/grassMask6.tga")
		|| !ri->FS_FileExists("grassImage/grassMask7.tga")
		|| !ri->FS_FileExists("grassImage/grassMask8.tga")
		|| !ri->FS_FileExists("grassImage/grassMask9.tga")
		|| !ri->FS_FileExists("grassImage/grassImage.tga"))
	{
		R_CreateGrassImages();

		for (int i = 0; i < 10; i++)
		{
			tr.grassMaskImage[i] = R_FindImageFile(va( "grassImage/grassMask%i.tga", i) , IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
		}

		tr.grassImage = R_FindImageFile("grassImage/grassImage.tga" , IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
	}
	else
	{
		for (int i = 0; i < 10; i++)
		{
			tr.grassMaskImage[i] = R_FindImageFile(va( "grassImage/grassMask%i.tga", i) , IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
		}

		tr.grassImage = R_FindImageFile("grassImage/grassImage.tga" , IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION);
	}
#endif
}
