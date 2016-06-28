/* marker */
#define MAP_C



/* dependencies */
#include "q3map2.h"
#include "inifile.h"

extern void SetEntityBounds( entity_t *e );
extern void LoadEntityIndexMap( entity_t *e );
extern void AdjustBrushesForOrigin( entity_t *ent );

float			TREE_SCALE_MULTIPLIER = 2.5;
char			TREE_MODELS[16][128] = { 0 };
float			TREE_OFFSETS[16] = { -4.0 };
float			TREE_SCALES[16] = { 1.0 };

void FOLIAGE_LoadClimateData( char *filename )
{
	int i = 0;

	// Read all the tree info from the new .climate ini files...
	TREE_SCALE_MULTIPLIER = atof(IniRead(filename, "TREES", "treeScaleMultiplier", "1.0"));

	Sys_Printf("Tree scale for this climate is %f.\n", TREE_SCALE_MULTIPLIER);

	for (i = 0; i < 9; i++)
	{
		strcpy(TREE_MODELS[i], IniRead(filename, "TREES", va("treeModel%i", i), ""));
		TREE_OFFSETS[i] = atof(IniRead(filename, "TREES", va("treeZoffset%i", i), "-4.0"));
		TREE_SCALES[i] = atof(IniRead(filename, "TREES", va("treeScale%i", i), "1.0"));

		Sys_Printf("Tree %i - Model %s - Offset %f - Scale %f.\n", i, TREE_MODELS[i], TREE_OFFSETS[i], TREE_SCALES[i]);
	}
}

#define			FOLIAGE_MAX_FOLIAGES 2097152

int				FOLIAGE_NUM_POSITIONS = 0;
vec3_t			FOLIAGE_POSITIONS[FOLIAGE_MAX_FOLIAGES];
float			FOLIAGE_TREE_ANGLES[FOLIAGE_MAX_FOLIAGES];
int				FOLIAGE_TREE_SELECTION[FOLIAGE_MAX_FOLIAGES];
float			FOLIAGE_TREE_SCALE[FOLIAGE_MAX_FOLIAGES];

qboolean FOLIAGE_LoadFoliagePositions( char *filename )
{
	FILE			*f;
	int				i = 0;
	int				fileCount = 0;
	int				treeCount = 0;

	f = fopen (filename, "rb");

	if ( !f )
	{
		return qfalse;
	}

	fread( &fileCount, sizeof(int), 1, f );

	for (i = 0; i < fileCount; i++)
	{
		vec3_t	unneededVec3;
		int		unneededInt;
		float	unneededFloat;

		fread( &FOLIAGE_POSITIONS[treeCount], sizeof(vec3_t), 1, f );
		fread( &unneededVec3, sizeof(vec3_t), 1, f );
		fread( &unneededInt, sizeof(int), 1, f );
		fread( &unneededFloat, sizeof(float), 1, f );
		fread( &unneededFloat, sizeof(float), 1, f );
		fread( &FOLIAGE_TREE_SELECTION[treeCount], sizeof(int), 1, f );
		fread( &FOLIAGE_TREE_ANGLES[treeCount], sizeof(float), 1, f );
		fread( &FOLIAGE_TREE_SCALE[treeCount], sizeof(float), 1, f );

		if (FOLIAGE_TREE_SELECTION[treeCount] > 0)
		{// Only keep positions with trees...
			treeCount++;
		}
	}

	FOLIAGE_NUM_POSITIONS = treeCount;

	fclose(f);

	return qtrue;
}


//#define __ADD_TREES_EARLY__ // Add trees to map's brush list instanty...

extern void MoveBrushesToWorld( entity_t *ent );
extern qboolean StringContainsWord(const char *haystack, const char *needle);

void GenerateMapForest ( void )
{
	if (generateforest)
	{
		int i;

#if defined(__ADD_TREES_EARLY__)
		Sys_Printf( "Adding %i trees to bsp.\n", FOLIAGE_NUM_POSITIONS );
#endif
		
		for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
		{
			printLabelledProgress("GenerateMapForest", i, FOLIAGE_NUM_POSITIONS);

			const char		*classname, *value;
			float			lightmapScale;
			vec3_t          lightmapAxis;
			int			    smoothNormals;
			int				vertTexProj;
			char			shader[ MAX_QPATH ];
			shaderInfo_t	*celShader = NULL;
			brush_t			*brush;
			parseMesh_t		*patch;
			qboolean		funcGroup;
			char			castShadows, recvShadows;
			qboolean		forceNonSolid, forceNoClip, forceNoTJunc, forceMeta;
			vec3_t          minlight, minvertexlight, ambient, colormod;
			float           patchQuality, patchSubdivision;

			/* setup */
			entitySourceBrushes = 0;
			mapEnt = &entities[ numEntities ];
			numEntities++;
			memset( mapEnt, 0, sizeof( *mapEnt ) );
			
			mapEnt->mapEntityNum = 0;

			//mapEnt->mapEntityNum = numMapEntities;
			//numMapEntities++;

			//mapEnt->mapEntityNum = 0 - numMapEntities;
			
			//mapEnt->forceSubmodel = qtrue;

			VectorCopy(FOLIAGE_POSITIONS[i], mapEnt->origin);
			mapEnt->origin[2] += TREE_OFFSETS[FOLIAGE_TREE_SELECTION[i]-1];

			{
				char str[32];
				sprintf( str, "%f %f %f", mapEnt->origin[ 0 ], mapEnt->origin[ 1 ], mapEnt->origin[ 2 ] );
				SetKeyValue( mapEnt, "origin", str );
			}

			{
				char str[32];
				sprintf( str, "%f", FOLIAGE_TREE_SCALE[i]*2.0*TREE_SCALE_MULTIPLIER*TREE_SCALES[FOLIAGE_TREE_SELECTION[i]-1] );
				SetKeyValue( mapEnt, "modelscale", str );
			}

			{
				char str[32];
				sprintf( str, "%f", FOLIAGE_TREE_ANGLES[i] );
				SetKeyValue( mapEnt, "angle", str );
			}

			/*{
				char str[32];
				sprintf( str, "%i", 2 );
				SetKeyValue( mapEnt, "spawnflags", str );
			}*/
		
			//Sys_Printf( "Generated tree at %f %f %f.\n", mapEnt->origin[0], mapEnt->origin[1], mapEnt->origin[2] );


			/* ydnar: get classname */
			SetKeyValue( mapEnt, "classname", "misc_model");
			classname = ValueForKey( mapEnt, "classname" );
			
			SetKeyValue( mapEnt, "model", TREE_MODELS[FOLIAGE_TREE_SELECTION[i]-1]); // test tree

			//Sys_Printf( "Generated tree at %f %f %f. Model %s.\n", mapEnt->origin[0], mapEnt->origin[1], mapEnt->origin[2], TROPICAL_TREES[FOLIAGE_TREE_SELECTION[i]-1] );

			funcGroup = qfalse;

			/* get explicit shadow flags */
			GetEntityShadowFlags( mapEnt, NULL, &castShadows, &recvShadows, (funcGroup || mapEnt->mapEntityNum == 0) ? qtrue : qfalse );

			/* vortex: get lightmap scaling value for this entity */
			GetEntityLightmapScale( mapEnt, &lightmapScale, 0);

			/* vortex: get lightmap axis for this entity */
			GetEntityLightmapAxis( mapEnt, lightmapAxis, NULL );

			/* vortex: per-entity normal smoothing */
			GetEntityNormalSmoothing( mapEnt, &smoothNormals, 0);

			/* vortex: per-entity _minlight, _ambient, _color, _colormod  */
			GetEntityMinlightAmbientColor( mapEnt, NULL, minlight, minvertexlight, ambient, colormod, qtrue );
			if( mapEnt == &entities[ 0 ] )
			{
				/* worldspawn have it empty, since it's keys sets global parms */
				VectorSet( minlight, 0, 0, 0 );
				VectorSet( minvertexlight, 0, 0, 0 );
				VectorSet( ambient, 0, 0, 0 );
				VectorSet( colormod, 1, 1, 1 );
			}

			/* vortex: _patchMeta, _patchQuality, _patchSubdivide support */
			GetEntityPatchMeta( mapEnt, &forceMeta, &patchQuality, &patchSubdivision, 1.0, patchSubdivisions);

			/* vortex: vertical texture projection */
			if( strcmp( "", ValueForKey( mapEnt, "_vtcproj" ) ) || strcmp( "", ValueForKey( mapEnt, "_vp" ) ) )
			{
				vertTexProj = IntForKey(mapEnt, "_vtcproj");
				if (vertTexProj <= 0.0f)
					vertTexProj = IntForKey(mapEnt, "_vp");
			}
			else
				vertTexProj = 0;

			/* ydnar: get cel shader :) for this entity */
			value = ValueForKey( mapEnt, "_celshader" );
			if( value[ 0 ] == '\0' )	
				value = ValueForKey( &entities[ 0 ], "_celshader" );
			if( value[ 0 ] != '\0' )
			{
				sprintf( shader, "textures/%s", value );
				celShader = ShaderInfoForShader( shader );
				//Sys_FPrintf (SYS_VRB, "Entity %d (%s) has cel shader %s\n", mapEnt->mapEntityNum, classname, celShader->shader );
			}
			else
				celShader = NULL;

			/* vortex: _nonsolid forces detail non-solid brush */
			forceNonSolid = ((IntForKey(mapEnt, "_nonsolid") > 0) || (IntForKey(mapEnt, "_ns") > 0)) ? qtrue : qfalse;

			/* vortex: preserve original face winding, don't clip by bsp tree */
			forceNoClip = ((IntForKey(mapEnt, "_noclip") > 0) || (IntForKey(mapEnt, "_nc") > 0)) ? qtrue : qfalse;

			/* vortex: do not apply t-junction fixing (preserve original face winding) */
			forceNoTJunc = ((IntForKey(mapEnt, "_notjunc") > 0) || (IntForKey(mapEnt, "_ntj") > 0)) ? qtrue : qfalse;

			/* attach stuff to everything in the entity */
			for( brush = mapEnt->brushes; brush != NULL; brush = brush->next )
			{
				brush->entityNum = mapEnt->mapEntityNum;
				brush->mapEntityNum = mapEnt->mapEntityNum;
				brush->castShadows = castShadows;
				brush->recvShadows = recvShadows;
				brush->lightmapScale = lightmapScale;
				VectorCopy( lightmapAxis, brush->lightmapAxis ); /* vortex */
				brush->smoothNormals = smoothNormals; /* vortex */
				brush->noclip = forceNoClip; /* vortex */
				brush->noTJunc = forceNoTJunc; /* vortex */
				brush->vertTexProj = vertTexProj; /* vortex */
				VectorCopy( minlight, brush->minlight ); /* vortex */
				VectorCopy( minvertexlight, brush->minvertexlight ); /* vortex */
				VectorCopy( ambient, brush->ambient ); /* vortex */
				VectorCopy( colormod, brush->colormod ); /* vortex */
				brush->celShader = celShader;
				if (forceNonSolid == qtrue)
				{
					brush->detail = qtrue;
					brush->nonsolid = qtrue;
					brush->noclip = qtrue;
				}
			}

			for( patch = mapEnt->patches; patch != NULL; patch = patch->next )
			{
				patch->entityNum = mapEnt->mapEntityNum;
				patch->mapEntityNum = mapEnt->mapEntityNum;
				patch->castShadows = castShadows;
				patch->recvShadows = recvShadows;
				patch->lightmapScale = lightmapScale;
				VectorCopy( lightmapAxis, patch->lightmapAxis ); /* vortex */
				patch->smoothNormals = smoothNormals; /* vortex */
				patch->vertTexProj = vertTexProj; /* vortex */
				patch->celShader = celShader;
				patch->patchMeta = forceMeta; /* vortex */
				patch->patchQuality = patchQuality; /* vortex */
				patch->patchSubdivisions = patchSubdivision; /* vortex */
				VectorCopy( minlight, patch->minlight ); /* vortex */
				VectorCopy( minvertexlight, patch->minvertexlight ); /* vortex */
				VectorCopy( ambient, patch->ambient ); /* vortex */
				VectorCopy( colormod, patch->colormod ); /* vortex */
				patch->nonsolid = forceNonSolid;
			}

			/* vortex: store map entity num */
			{
				char buf[32];
				sprintf( buf, "%i", mapEnt->mapEntityNum ); 
				SetKeyValue( mapEnt, "_mapEntityNum", buf );
			}

			/* ydnar: gs mods: set entity bounds */
			SetEntityBounds( mapEnt );

			/* ydnar: gs mods: load shader index map (equivalent to old terrain alphamap) */
			LoadEntityIndexMap( mapEnt );
			
			/* get entity origin and adjust brushes */
			GetVectorForKey( mapEnt, "origin", mapEnt->origin );
			if ( mapEnt->originbrush_origin[ 0 ] || mapEnt->originbrush_origin[ 1 ] || mapEnt->originbrush_origin[ 2 ] )
				AdjustBrushesForOrigin( mapEnt );


#if defined(__ADD_TREES_EARLY__)
			AddTriangleModels( 0, qtrue, qtrue );
			EmitBrushes( mapEnt->brushes, &mapEnt->firstBrush, &mapEnt->numBrushes );
			//MoveBrushesToWorld( mapEnt );
			numEntities--;
#endif
		}

#if defined(__ADD_TREES_EARLY__)
		Sys_Printf( "Finished adding trees.\n" );
#endif
	}
}
