#include "../qcommon/q_shared.h"
#include "../cgame/cg_local.h"
#include "../ui/ui_shared.h"
#include "../game/surfaceflags.h"

// =======================================================================================================================================
//
//                                                             Foliage Rendering...
//
// =======================================================================================================================================

qboolean	FOLIAGE_LOADED = qfalse;
int			FOLIAGE_NUM_POSITIONS = 0;
vec3_t		FOLIAGE_POSITIONS[262144];
int			FOLIAGE_GRASS_SELECTION[262144];
float		FOLIAGE_GRASS_ANGLES[262144];
float		FOLIAGE_GRASS_SCALE[262144];
int			FOLIAGE_PLANT_SELECTION[262144];
float		FOLIAGE_PLANT_ANGLES[262144];
float		FOLIAGE_PLANT_SCALE[262144];
int			FOLIAGE_TREE_SELECTION[262144];
float		FOLIAGE_TREE_ANGLES[262144];
float		FOLIAGE_TREE_SCALE[262144];

qhandle_t	FOLIAGE_GRASS_MODEL[3] = { 0 };
qhandle_t	FOLIAGE_PLANT_MODEL[27] = { 0 };
qhandle_t	FOLIAGE_TREE_MODEL[3] = { 0 };

void FOLIAGE_AddFoliageEntityToScene ( refEntity_t *ent )
{
	AddRefEntityToScene(ent);
}

void FOLIAGE_AddToScreen( int num, qboolean treeOnly ) {
	refEntity_t		re;
	vec3_t			angles;
	qboolean		skip_tinyStuff = qfalse;
	qboolean		skip_smallStuff = qfalse;
	qboolean		skip_mediumStuff = qfalse;
	float			dist = Distance(FOLIAGE_POSITIONS[num], cg.refdef.vieworg);

	if (dist > 384.0)
	{// Let's skip smaller models when far from camera for FPS sake...
		skip_tinyStuff = qtrue;
	}

	if (dist > 512.0)
	{// Let's skip smaller models when far from camera for FPS sake...
		skip_smallStuff = qtrue;
	}
	
	if (dist > 2048.0)
	{// Let's skip medium models when far from camera for FPS sake...
		skip_mediumStuff = qtrue;
	}

	memset( &re, 0, sizeof( re ) );

	VectorCopy(FOLIAGE_POSITIONS[num], re.origin);

	re.reType = RT_MODEL;

	if (!treeOnly || (dist < 5000.0 && FOLIAGE_GRASS_SCALE[num] >= 1.0)
		/*&& !(skip_mediumStuff && FOLIAGE_GRASS_SCALE[num] > 0.75)
		&& !(skip_smallStuff && FOLIAGE_GRASS_SCALE[num] <= 0.75)
		&& !(skip_tinyStuff && FOLIAGE_GRASS_SCALE[num] <= 0.5)*/)
	{// Graw grass...
		re.hModel = FOLIAGE_GRASS_MODEL[0];
		VectorSet(re.modelScale, FOLIAGE_GRASS_SCALE[num], FOLIAGE_GRASS_SCALE[num], FOLIAGE_GRASS_SCALE[num]);
		angles[PITCH] = angles[ROLL] = 0.0f;
		angles[YAW] = FOLIAGE_GRASS_ANGLES[num];
		VectorCopy(angles, re.angles);
		AnglesToAxis(angles, re.axis);
		ScaleModelAxis( &re );
		FOLIAGE_AddFoliageEntityToScene( &re );
	}

	if (FOLIAGE_TREE_SELECTION[num] != 0)
	{// Add the tree model...
		re.hModel = FOLIAGE_TREE_MODEL[FOLIAGE_TREE_SELECTION[num]-1];
		VectorSet(re.modelScale, FOLIAGE_TREE_SCALE[num], FOLIAGE_TREE_SCALE[num], FOLIAGE_TREE_SCALE[num]);
		angles[PITCH] = angles[ROLL] = 0.0f;
		angles[YAW] = FOLIAGE_TREE_ANGLES[num];
		VectorCopy(angles, re.angles);
		AnglesToAxis(angles, re.axis);
		re.origin[2] += 128.0; // the tree model digs into ground too much...
		ScaleModelAxis( &re );
		FOLIAGE_AddFoliageEntityToScene( &re );
	}
	else if (FOLIAGE_PLANT_SELECTION[num] != 0 
		&& !treeOnly || (dist < 5000.0 && FOLIAGE_PLANT_SCALE[num] >= 1.0)
		&& (!(skip_mediumStuff && FOLIAGE_PLANT_SCALE[num] > 0.75) || (dist < 5000.0 && FOLIAGE_PLANT_SCALE[num] >= 1.0))
		&& !(skip_smallStuff && FOLIAGE_PLANT_SCALE[num] <= 0.75)
		&& !(skip_tinyStuff && FOLIAGE_GRASS_SCALE[num] <= 0.5))
	{// Add plant model as well...
		re.hModel = FOLIAGE_PLANT_MODEL[FOLIAGE_PLANT_SELECTION[num]-1];
		VectorSet(re.modelScale, FOLIAGE_PLANT_SCALE[num], FOLIAGE_PLANT_SCALE[num], FOLIAGE_PLANT_SCALE[num]);
		angles[PITCH] = angles[ROLL] = 0.0f;
		angles[YAW] = FOLIAGE_PLANT_ANGLES[num];
		VectorCopy(angles, re.angles);
		AnglesToAxis(angles, re.axis);
		ScaleModelAxis( &re );
		FOLIAGE_AddFoliageEntityToScene( &re );
	}
}

extern qboolean InFOV( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV );

qboolean FOLIAGE_FloorIsGrassAt ( vec3_t org )
{
	trace_t tr;
	vec3_t org1, org2;
	float pitch = 0;

	VectorCopy(org, org1);
	VectorCopy(org, org2);
	org2[2] -= 20.0f;

	CG_Trace( &tr, org1, NULL, NULL, org2, -1, CONTENTS_SOLID|CONTENTS_TERRAIN );
	
	if (tr.startsolid || tr.allsolid)
	{
		return qfalse;
	}

	if (tr.fraction == 1.0)
	{
		return qfalse;
	}

	if (Distance(org, tr.endpos) >= 20.0)
	{
		return qfalse;
	}

	switch (tr.surfaceFlags & MATERIAL_MASK)
	{
	case MATERIAL_SHORTGRASS:
	case MATERIAL_LONGGRASS:
		return qtrue;
		break;
	default:
		break;
	}

	return qfalse;
}

qboolean FOLIAGE_LoadFoliagePositions( void )
{
	fileHandle_t	f;
	int				i = 0;
	
	trap->FS_Open( va( "foliage/%s.foliage", cgs.currentmapname), &f, FS_READ );

	if ( !f )
	{
		return qfalse;
	}

	trap->FS_Read( &FOLIAGE_NUM_POSITIONS, sizeof(int), f );

	for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
	{
		trap->FS_Read( &FOLIAGE_POSITIONS[i], sizeof(vec3_t), f );
		trap->FS_Read( &FOLIAGE_GRASS_SELECTION[i], sizeof(int), f );
		if (FOLIAGE_GRASS_SELECTION[i] >= 3)
		{// Old file... Report to user that they need to update with /genfoliage...
			FOLIAGE_NUM_POSITIONS = 0;
			trap->Print( "^1*** ^3%s^5: Failed to open foliage file ^7foliage/%s.foliage^5. Old version detected. Use /genfoliage to regenerate.\n", GAME_VERSION, cgs.currentmapname );
			return qfalse;
		}
		trap->FS_Read( &FOLIAGE_GRASS_ANGLES[i], sizeof(float), f );
		trap->FS_Read( &FOLIAGE_GRASS_SCALE[i], sizeof(float), f );
		trap->FS_Read( &FOLIAGE_PLANT_SELECTION[i], sizeof(int), f );
		trap->FS_Read( &FOLIAGE_PLANT_ANGLES[i], sizeof(float), f );
		trap->FS_Read( &FOLIAGE_PLANT_SCALE[i], sizeof(float), f );
		trap->FS_Read( &FOLIAGE_TREE_SELECTION[i], sizeof(int), f );
		trap->FS_Read( &FOLIAGE_TREE_ANGLES[i], sizeof(float), f );
		trap->FS_Read( &FOLIAGE_TREE_SCALE[i], sizeof(float), f );
	}

	trap->FS_Close(f);

	trap->Print( "^1*** ^3%s^5: Successfully loaded %i grass points from foliage file ^7foliage/%s.foliage^5.\n", GAME_VERSION,
			  FOLIAGE_NUM_POSITIONS, cgs.currentmapname );

	return qtrue;
}

qboolean FOLIAGE_SaveFoliagePositions( void )
{
	fileHandle_t	f;
	int				i = 0;
	
	trap->FS_Open( va( "foliage/%s.foliage", cgs.currentmapname), &f, FS_WRITE );

	if ( !f )
	{
		trap->Print( "^1*** ^3%s^5: Failed to open foliage file ^7foliage/%s.foliage^5 for save.\n", GAME_VERSION, cgs.currentmapname );
		return qfalse;
	}

	trap->FS_Write( &FOLIAGE_NUM_POSITIONS, sizeof(int), f );

	for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
	{
		trap->FS_Write( &FOLIAGE_POSITIONS[i], sizeof(vec3_t), f );
		trap->FS_Write( &FOLIAGE_GRASS_SELECTION[i], sizeof(int), f );
		trap->FS_Write( &FOLIAGE_GRASS_ANGLES[i], sizeof(float), f );
		trap->FS_Write( &FOLIAGE_GRASS_SCALE[i], sizeof(float), f );
		trap->FS_Write( &FOLIAGE_PLANT_SELECTION[i], sizeof(int), f );
		trap->FS_Write( &FOLIAGE_PLANT_ANGLES[i], sizeof(float), f );
		trap->FS_Write( &FOLIAGE_PLANT_SCALE[i], sizeof(float), f );
		trap->FS_Write( &FOLIAGE_TREE_SELECTION[i], sizeof(int), f );
		trap->FS_Write( &FOLIAGE_TREE_ANGLES[i], sizeof(float), f );
		trap->FS_Write( &FOLIAGE_TREE_SCALE[i], sizeof(float), f );
	}

	trap->FS_Close(f);

	trap->Print( "^1*** ^3%s^5: Successfully saved %i grass points to foliage file ^7foliage/%s.foliage^5.\n", GAME_VERSION,
			  FOLIAGE_NUM_POSITIONS, cgs.currentmapname );

	return qtrue;
}

qboolean FOLIAGE_IgnoreFoliageOnMap( void )
{
	if (StringContainsWord(cgs.currentmapname, "eisley")
		|| StringContainsWord(cgs.currentmapname, "desert")
		|| StringContainsWord(cgs.currentmapname, "tatooine")
		|| StringContainsWord(cgs.currentmapname, "hoth")
		|| StringContainsWord(cgs.currentmapname, "mp/ctf1")
		|| StringContainsWord(cgs.currentmapname, "mp/ctf2")
		|| StringContainsWord(cgs.currentmapname, "mp/ctf4")
		|| StringContainsWord(cgs.currentmapname, "mp/ctf5")
		|| StringContainsWord(cgs.currentmapname, "mp/ffa1")
		|| StringContainsWord(cgs.currentmapname, "mp/ffa2")
		|| StringContainsWord(cgs.currentmapname, "mp/ffa3")
		|| StringContainsWord(cgs.currentmapname, "mp/ffa4")
		|| StringContainsWord(cgs.currentmapname, "mp/ffa5")
		|| StringContainsWord(cgs.currentmapname, "mp/duel1")
		|| StringContainsWord(cgs.currentmapname, "mp/duel2")
		|| StringContainsWord(cgs.currentmapname, "mp/duel3")
		|| StringContainsWord(cgs.currentmapname, "mp/duel4")
		|| StringContainsWord(cgs.currentmapname, "mp/duel5")
		|| StringContainsWord(cgs.currentmapname, "mp/duel7")
		|| StringContainsWord(cgs.currentmapname, "mp/duel9")
		|| StringContainsWord(cgs.currentmapname, "mp/duel10")
		|| StringContainsWord(cgs.currentmapname, "bespin_streets")
		|| StringContainsWord(cgs.currentmapname, "bespin_platform"))
	{// Ignore this map... We know we don't need grass here...
		return qtrue;
	}

	return qfalse;
}

void FOLIAGE_DrawGrass( void )
{
	int spot = 0;

	if (FOLIAGE_IgnoreFoliageOnMap())
	{// Ignore this map... We know we don't need grass here...
		FOLIAGE_NUM_POSITIONS = 0;
		FOLIAGE_LOADED = qtrue;
		return;
	}

	if (!FOLIAGE_LOADED)
	{
		FOLIAGE_LoadFoliagePositions();
		FOLIAGE_LOADED = qtrue;
	}

	if (FOLIAGE_NUM_POSITIONS <= 0)
	{
		return;
	}

	if (!FOLIAGE_GRASS_MODEL[0])
	{// Init/register all foliage models...
		FOLIAGE_GRASS_MODEL[0] = trap->R_RegisterModel( "models/pop/foliages/sch_weed_a.md3" );
		FOLIAGE_GRASS_MODEL[1] = trap->R_RegisterModel( "models/pop/foliages/sch_weed_b.md3" );
		FOLIAGE_GRASS_MODEL[2] = trap->R_RegisterModel( "models/warzone/foliage/grass33.md3" );

		FOLIAGE_PLANT_MODEL[0] = trap->R_RegisterModel( "models/warzone/foliage/plant03.md3" );
		FOLIAGE_PLANT_MODEL[1] = trap->R_RegisterModel( "models/warzone/foliage/plant05.md3" );
		FOLIAGE_PLANT_MODEL[2] = trap->R_RegisterModel( "models/warzone/foliage/plant10.md3" );
		FOLIAGE_PLANT_MODEL[3] = trap->R_RegisterModel( "models/warzone/foliage/plant11.md3" );
		FOLIAGE_PLANT_MODEL[4] = trap->R_RegisterModel( "models/warzone/foliage/plant12.md3" );
		FOLIAGE_PLANT_MODEL[5] = trap->R_RegisterModel( "models/warzone/foliage/plant14.md3" );
		FOLIAGE_PLANT_MODEL[6] = trap->R_RegisterModel( "models/warzone/foliage/plant16.md3" );
		FOLIAGE_PLANT_MODEL[7] = trap->R_RegisterModel( "models/warzone/foliage/plant20.md3" );
		FOLIAGE_PLANT_MODEL[8] = trap->R_RegisterModel( "models/warzone/foliage/plant21.md3" );
		FOLIAGE_PLANT_MODEL[9] = trap->R_RegisterModel( "models/warzone/foliage/plant22.md3" );
		FOLIAGE_PLANT_MODEL[10] = trap->R_RegisterModel( "models/warzone/foliage/plant23.md3" );
		FOLIAGE_PLANT_MODEL[11] = trap->R_RegisterModel( "models/warzone/foliage/plant27.md3" );
		FOLIAGE_PLANT_MODEL[12] = trap->R_RegisterModel( "models/warzone/foliage/plant28.md3" );
		FOLIAGE_PLANT_MODEL[13] = trap->R_RegisterModel( "models/warzone/foliage/plant29.md3" );
		FOLIAGE_PLANT_MODEL[14] = trap->R_RegisterModel( "models/warzone/foliage/plant30.md3" );
		FOLIAGE_PLANT_MODEL[15] = trap->R_RegisterModel( "models/warzone/foliage/plant31.md3" );
		FOLIAGE_PLANT_MODEL[16] = trap->R_RegisterModel( "models/warzone/foliage/plant32.md3" );
		FOLIAGE_PLANT_MODEL[17] = trap->R_RegisterModel( "models/warzone/foliage/plant33.md3" );
		FOLIAGE_PLANT_MODEL[18] = trap->R_RegisterModel( "models/warzone/foliage/plant36.md3" );
		FOLIAGE_PLANT_MODEL[19] = trap->R_RegisterModel( "models/warzone/foliage/plant64.md3" );
		FOLIAGE_PLANT_MODEL[20] = trap->R_RegisterModel( "models/warzone/foliage/plant65.md3" );
		FOLIAGE_PLANT_MODEL[21] = trap->R_RegisterModel( "models/warzone/foliage/plant66.md3" );
		FOLIAGE_PLANT_MODEL[22] = trap->R_RegisterModel( "models/warzone/foliage/plant68.md3" );
		FOLIAGE_PLANT_MODEL[23] = trap->R_RegisterModel( "models/warzone/foliage/plant78.md3" );
		FOLIAGE_PLANT_MODEL[24] = trap->R_RegisterModel( "models/warzone/foliage/plant79.md3" );
		FOLIAGE_PLANT_MODEL[25] = trap->R_RegisterModel( "models/warzone/foliage/plant80.md3" );
		FOLIAGE_PLANT_MODEL[26] = trap->R_RegisterModel( "models/warzone/foliage/plant81.md3" );
		
		FOLIAGE_TREE_MODEL[0] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );
		FOLIAGE_TREE_MODEL[1] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );
		FOLIAGE_TREE_MODEL[2] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );
	}

	//omp_init_lock(&refent_lock);

//#pragma omp parallel for schedule(dynamic) // multithreaded to speed this loop up...
	for (spot = 0; spot < FOLIAGE_NUM_POSITIONS; spot++)
	{
		// Draw anything closeish to us...
		int			len = 0;
		int			link = 0;
		vec3_t		delta;
		qboolean	treeOnly = qfalse;

		VectorSubtract( FOLIAGE_POSITIONS[spot], cg.refdef.vieworg, delta );
		len = VectorLength( delta );

		if ( (FOLIAGE_TREE_SELECTION[spot] != 0 && len > 3192.0) || (len < 5000.0 && FOLIAGE_GRASS_SCALE[spot] >= 1.0))
		{
			treeOnly = qtrue;
		}
		else if ( len > 3192.0 ) 
		{
			continue;
		}

		if ( FOLIAGE_TREE_SELECTION[spot] != 0 )
		{// Tree here, wider view FOV check...
			if ( len > 4096 && !InFOV( FOLIAGE_POSITIONS[spot], cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x + 3, cg.refdef.fov_y + 3 ))
				continue;
			else if ( len > 2048 && !InFOV( FOLIAGE_POSITIONS[spot], cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x + 5, cg.refdef.fov_y + 5 ))
				continue;
			else if ( len > 1024 && !InFOV( FOLIAGE_POSITIONS[spot], cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x + 10, cg.refdef.fov_y + 10 ))
				continue;
			else if ( len > 256 && !InFOV( FOLIAGE_POSITIONS[spot], cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x + 20, cg.refdef.fov_y + 20 ))
				continue;
		}
		else
		{// No tree here, can use minimal FOV check...
			if ( len > 4096 && !InFOV( FOLIAGE_POSITIONS[spot], cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x + 1, cg.refdef.fov_y + 1 ))
				continue;
			else if ( len > 2048 && !InFOV( FOLIAGE_POSITIONS[spot], cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x + 2, cg.refdef.fov_y + 2 ))
				continue;
			else if ( len > 1024 && !InFOV( FOLIAGE_POSITIONS[spot], cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x + 3, cg.refdef.fov_y + 3 ))
				continue;
			else if ( len > 256 && !InFOV( FOLIAGE_POSITIONS[spot], cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x + 20, cg.refdef.fov_y + 20 ))
				continue;
		}

		FOLIAGE_AddToScreen( spot, treeOnly );
	}

	//omp_destroy_lock(&refent_lock);
}

// =======================================================================================================================================
//
//                                                             Foliage Generation...
//
// =======================================================================================================================================

float FOLIAGE_FloorHeightAt ( vec3_t org )
{
	trace_t tr;
	vec3_t org1, org2, slopeangles;
	float pitch = 0;

	VectorCopy(org, org1);

	VectorCopy(org, org2);
	org2[2]= -65536.0f;

	CG_Trace( &tr, org1, NULL, NULL, org2, -1, MASK_PLAYERSOLID );
	
	if (tr.startsolid)
	{
		return 65536.0f;
	}

	if (tr.endpos[2] > cg.mapcoordsMaxs[2]+2000)
		return 65536.0f;

	if (tr.endpos[2] < -65000 || tr.endpos[2] < cg.mapcoordsMins[2]-2000)
		return -65536.0f;

	if ( tr.surfaceFlags & SURF_SKY )
	{// Sky...
		return 65536.0f;
	}
	
	// Added -- Check slope...
	vectoangles( tr.plane.normal, slopeangles );

	pitch = slopeangles[0];
	
	if (pitch > 180)
		pitch -= 360;

	if (pitch < -180)
		pitch += 360;

	pitch += 90.0f;

	if (pitch > 46.0f || pitch < -46.0f)
		return 65536.0f; // bad slope...

	if ( tr.startsolid || tr.allsolid /*|| tr.contents & CONTENTS_WATER*/ )
	{
		return 65536.0f;
	}

	/*
	if (DO_THOROUGH && BadHeightNearby( org ))
	{
		return 65536.0f;
	}
	*/

	/*
	if (!CG_HaveRoofAbove(org))
	{
		return 65536.0f;
	}
	*/

	switch( tr.surfaceFlags & MATERIAL_MASK )
	{
	case MATERIAL_SHORTGRASS:		// 5			// manicured lawn
	case MATERIAL_LONGGRASS:		// 6			// long jungle grass
		break;
	default:
		// Not grass...
		return 65536.0f;
		break;
	}

	return tr.endpos[2];
}

extern void AIMod_GetMapBounts ( void );
extern float RoofHeightAt ( vec3_t org );

qboolean FOLIAGE_CheckInSolid (vec3_t position)
{
	trace_t	trace;
	vec3_t	end, mins, maxs;
	vec3_t pos;

	int contents = CONTENTS_TRIGGER;
	int clipmask = MASK_DEADSOLID;
	
	VectorSet(mins, -64, -64, DEFAULT_MINS_2);
	VectorSet(maxs, 64, 64, DEFAULT_MAXS_2);

	VectorCopy(position, pos);
	pos[2]+=28;
	VectorCopy(pos, end);
	end[2] += mins[2];
	mins[2] = 0;

	CG_Trace(&trace, position, mins, maxs, end, -1, clipmask);
	if(trace.allsolid || trace.startsolid)
	{
		return qtrue;
	}

	if(trace.fraction < 1.0)
	{
		return qtrue;
	}

	return qfalse;
}

qboolean FOLIAGE_Check_Width ( vec3_t origin ) 
{
	vec3_t		org;
	VectorCopy(origin, org);
	org[2]+=18;
	if (FOLIAGE_CheckInSolid(org)) return qfalse;
	return qtrue;
}

extern float aw_percent_complete;
extern char task_string1[255];
extern char task_string2[255];
extern char task_string3[255];
extern char last_node_added_string[255];
extern clock_t	aw_stage_start_time;

typedef long int	intvec_t;
typedef intvec_t	intvec3_t[3];

#define FOLIAGE_MOD_NAME		"aimod"
float	FOLIAGE_FILE_VERSION =	1.1f;

void FOLIAGE_GenerateFoliage_Real ( qboolean USE_WAYPOINTS, float density )
{
	fileHandle_t	f;
	int				i, j;
	char			filename[60];
//	vmCvar_t		mapname;
	short int		objNum[3] = { 0, 0, 0 },
	objFlags, numLinks;
	int				flags;
	vec3_t			vec;
	short int		fl2;
	int				target;
	char			name[] = FOLIAGE_MOD_NAME;
	char			nm[64] = "";
	float			version;
	char			map[64] = "";
	char			mp[64] = "";
	/*short*/ int		numberNodes;
	short int		temp, fix_aas_nodes;

	if (FOLIAGE_IgnoreFoliageOnMap())
	{// Ignore this map... We know we don't need grass here...
		FOLIAGE_NUM_POSITIONS = 0;
		FOLIAGE_LOADED = qtrue;
		return;
	}

	FOLIAGE_NUM_POSITIONS = 0;

	for (i = 0; i < 262144; i++)
	{// Make sure our list is empty...
		FOLIAGE_GRASS_SELECTION[i] = 0;
		FOLIAGE_PLANT_SELECTION[i] = 0;
		FOLIAGE_TREE_SELECTION[i] = 0;
	}

	i = 0;

	if (USE_WAYPOINTS)
	{
		///////////////////
		//open the node file for reading, return false on error
		trap->FS_Open( va( "nodes/%s.bwp", cgs.currentmapname), &f, FS_READ );

		if ( !f )
		{
			return;
		}

		strcpy( mp, cgs.currentmapname);
		trap->FS_Read( &nm, strlen( name) + 1, f );									//read in a string the size of the mod name (+1 is because all strings end in hex '00')
		trap->FS_Read( &version, sizeof(float), f );			//read and make sure the version is the same

		if ( version != FOLIAGE_FILE_VERSION && version != 1.0f )
		{
			trap->Print( "^1*** ^3WARNING^5: Reading from ^7nodes/%s.bwp^3 failed^5!!!\n", filename );
			trap->Print( "^1*** ^3       ^5  Old node file detected.\n" );
			trap->FS_Close( f );
			return;
		}

		trap->FS_Read( &map, strlen( mp) + 1, f );			//make sure the file is for the current map
		if ( Q_stricmp( map, mp) != 0 )
		{
			trap->Print( "^1*** ^3WARNING^5: Reading from ^7nodes/%s.bwp^3 failed^5!!!\n", filename );
			trap->Print( "^1*** ^3       ^5  Node file is not for this map!\n" );
			trap->FS_Close( f );
			return;
		}

		if (version == FOLIAGE_FILE_VERSION)
		{
			trap->FS_Read( &numberNodes, sizeof(/*short*/ int), f ); //read in the number of nodes in the map
		}
		else
		{
			trap->FS_Read( &temp, sizeof(short int), f ); //read in the number of nodes in the map
			numberNodes = temp;
		}

		for ( i = 0; i < numberNodes; i++ )					//loop through all the nodes
		{
			//read in all the node info stored in the file
			trap->FS_Read( &vec, sizeof(vec3_t), f );
			trap->FS_Read( &flags, sizeof(int), f );
			trap->FS_Read( objNum, sizeof(short int) * 3, f );
			trap->FS_Read( &objFlags, sizeof(short int), f );
			trap->FS_Read( &numLinks, sizeof(short int), f );

			if (FOLIAGE_FloorIsGrassAt(vec))
			{
				FOLIAGE_GRASS_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
				FOLIAGE_GRASS_ANGLES[FOLIAGE_NUM_POSITIONS] = 0.0f;
				FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] = 0.0f;

				FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
				FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = 0.0f;
				FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = 0.0f;

				FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
				FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = 0.0f;
				FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = 0.0f;

				VectorCopy(vec, FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS]);
				FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS][2] -= 18.0;

				FOLIAGE_GRASS_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 2);
				FOLIAGE_GRASS_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
				FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,125) / 100.0);

				if (random() * 30 >= 29 && RoofHeightAt(vec) - vec[2] > 1024.0)
				{// Add tree... 1 in every 30 positions... If there is room above for a tree...
					FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
					FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
					FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
				}
				else if (random() * 100 >= 66)
				{// Add plant... 1 in every 3 positions...
					FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
					FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
					FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
				}

				FOLIAGE_NUM_POSITIONS++;
			}

			//loop through all of the links and read the data
			for ( j = 0; j < numLinks; j++ )
			{
				if (version == FOLIAGE_FILE_VERSION)
				{
					trap->FS_Read( &target, sizeof(/*short*/ int), f );
				}
				else
				{
					trap->FS_Read( &temp, sizeof(short int), f );
					target = temp;
				}

				trap->FS_Read( &fl2, sizeof(short int), f );
			}
		}

		trap->FS_Read( &fix_aas_nodes, sizeof(short int), f );
		trap->FS_Close( f );							//close the file

		trap->Print( "^1*** ^3%s^5: Successfully generated %i grass points from waypoint file ^7nodes/%s.bwp^5.\n", GAME_VERSION,
			FOLIAGE_NUM_POSITIONS, cgs.currentmapname );

		// Save the generated info to a file for next time...
		FOLIAGE_SaveFoliagePositions();

		FOLIAGE_LOADED = qtrue;
	}
	else
	{// Advanced method for multi-level maps...
		float		startx = -65530, starty = -65530, startz = -65530;
		int			grassSpotCount = 0;
		vec3_t		*grassSpotList;
		float		map_size, temp;
		vec3_t		mapMins, mapMaxs;
		int			total_tests = 0, final_tests = 0;
		int			start_time = trap->Milliseconds();
		int			update_timer = 0;
		int			parallel_x = 0;
		int			parallel_x_max = 0;
		int			parallel_y_max = 0;
		float		scatter = 0;
		float		scatter_avg = 0;
		float		scatter_min = 0;
		float		scatter_z = 0;
		float		scatter_max = 0;
		float		scatter_x = 0;
		clock_t		previous_time = 0;
		float		offsetY = 0.0;

		trap->UpdateScreen();

		AIMod_GetMapBounts();

		if (!cg.mapcoordsValid)
		{
			trap->Print("^4*** ^3AUTO-FOLIAGE^4: ^7Map Coordinates are invalid. Can not use auto-waypointer!\n");
			return;
		}

		grassSpotList = (vec3_t *)malloc((sizeof(vec3_t)+1)*262144);

		VectorCopy(cg.mapcoordsMins, mapMins);
		VectorCopy(cg.mapcoordsMaxs, mapMaxs);

		if (mapMaxs[0] < mapMins[0])
		{
			temp = mapMins[0];
			mapMins[0] = mapMaxs[0];
			mapMaxs[0] = temp;
		}

		if (mapMaxs[1] < mapMins[1])
		{
			temp = mapMins[1];
			mapMins[1] = mapMaxs[1];
			mapMaxs[1] = temp;
		}

		if (mapMaxs[2] < mapMins[2])
		{
			temp = mapMins[2];
			mapMins[2] = mapMaxs[2];
			mapMaxs[2] = temp;
		}

		trap->S_Shutup(qtrue);

		mapMaxs[0]+=2048;
		mapMaxs[1]+=2048;
		mapMaxs[2]+=2048;

		mapMins[0]-=2048;
		mapMins[1]-=2048;
		mapMins[2]-=2048;

		startx = mapMaxs[0];
		starty = mapMaxs[1];
		startz = mapMaxs[2];

		map_size = Distance(mapMins, mapMaxs);

		trap->Print( va( "^4*** ^3AUTO-FOLIAGE^4: ^5Map bounds are ^3%.2f %.2f %.2f ^5to ^3%.2f %.2f %.2f^5.\n", mapMins[0], mapMins[1], mapMins[2], mapMaxs[0], mapMaxs[1], mapMaxs[2]) );
		strcpy( task_string1, va("^5Map bounds are ^3%.2f %.2f %.2f ^7to ^3%.2f %.2f %.2f^5.", mapMins[0], mapMins[1], mapMins[2], mapMaxs[0], mapMaxs[1], mapMaxs[2]) );
		trap->UpdateScreen();

		trap->Print( va( "^4*** ^3AUTO-FOLIAGE^4: ^5Generating foliage points. This could take a while... (Map size ^3%.2f^5)\n", map_size) );
		strcpy( task_string2, va("^5Generating foliage points. This could take a while... (Map size ^3%.2f^5)", map_size) );
		trap->UpdateScreen();

		trap->Print( va( "^4*** ^3AUTO-FOLIAGE^4: ^5Finding foliage points...\n") );
		strcpy( task_string3, va("^5Finding foliage points...") );
		trap->UpdateScreen();

		//
		// Create bulk temporary nodes...
		//

		// Let's vary the scatter distances :)
		scatter = density;
		scatter_min = scatter * 0.33;
		scatter_max = scatter * 2.0;
		scatter_z = scatter_min;

		scatter_avg = (scatter + scatter_min + scatter_max) / 3.0;
		scatter_x = scatter;

		parallel_x_max = ((mapMaxs[0] - mapMins[0]) / scatter_avg);
		parallel_y_max = ((mapMaxs[1] - mapMins[1]) / scatter_avg);

		total_tests = ((mapMaxs[0] - mapMins[0]) / scatter_avg);
		total_tests *= ((mapMaxs[1] - mapMins[1]) / scatter_avg);
		total_tests *= ((mapMaxs[2] - mapMins[2]) / scatter_min);

		final_tests = 0;
		previous_time = clock();

		omp_set_nested(0);

		scatter_x = scatter;

		final_tests = 0;
		previous_time = clock();
		aw_stage_start_time = clock();

		for (parallel_x = 0; parallel_x < parallel_x_max; parallel_x++) // To OMP this sucker...
		{
			int		x;
			int		parallel_y = 0;
			float	scatter_y = scatter;
			float	scatter_mult_X = 1.0;
			vec3_t	last_org;

			// Vary X scatter distance...
			if (scatter_x == scatter) scatter_x = scatter_min;
			else if (scatter_x == scatter) scatter_x = scatter_max;
			else scatter_x = scatter;

			x = startx - (parallel_x * (scatter_x * scatter_mult_X));

			if (offsetY == 0.0)
				offsetY = (scatter_y * 0.25);
			else if (offsetY == scatter_y * 0.25)
				offsetY = (scatter_y * 0.5);
			else if (offsetY == scatter_y * 0.5)
				offsetY = (scatter_y * 0.75);
			else
				offsetY = 0.0;

#pragma omp parallel for ordered schedule(dynamic)
			for (parallel_y = 0; parallel_y < parallel_y_max; parallel_y++) // To OMP this sucker...
			{
				int		z, y;
				float	current_height = mapMaxs[2]; // Init the current height to max map height...
				float	scatter_mult_Y = 1.0;

				// Vary Y scatter distance...
				if (scatter_y == scatter) scatter_y = scatter_min;
				else if (scatter_y == scatter) scatter_y = scatter_max;
				else scatter_y = scatter;

				y = (starty + offsetY) - (parallel_y * (scatter_y * scatter_mult_Y));

				for (z = startz; z >= mapMins[2]; z -= scatter_min)
				{
					vec3_t		new_org, org;
					float		floor = 0;
					clock_t		current_time = clock();

					// Update the current test number...
					final_tests++;

					if (final_tests > 1000)
					{
						if(omp_get_thread_num() == 0)
						{// Draw a nice little progress bar ;)
							aw_percent_complete = (float)((float)final_tests/(float)total_tests)*100.0f;

							if (current_time - previous_time > 500) // update display every 500ms...
							{
								previous_time = current_time;
								trap->UpdateScreen();
							}
						}
					}

					if (z >= current_height)
					{// We can skip down to this position...
						continue;
					}

					// Set this test location's origin...
					VectorSet(new_org, x, y, z);

					// Find the ground at this point...
					floor = FOLIAGE_FloorHeightAt(new_org);

					// Set the point found on the floor as the test location...
					VectorSet(org, new_org[0], new_org[1], floor);

					if (floor < mapMins[2])
					{// Can skip this one!
						// Mark current hit location to continue from...
						current_height = mapMins[2]-2048; // so we still update final_tests
						continue; // so we still update final_tests
					}
					else if (floor > mapMaxs[2])
					{// Marks a start-solid or on top of the sky... Skip...
						current_height = z - scatter_min;
						continue;
					}
					else if (Distance(org, last_org) < density)
					{
						current_height = floor;
						continue;
					}

					if (!FOLIAGE_Check_Width(org))
					{// Not wide enough for a player to fit!
						current_height = floor;
						continue; // because omp critical can not "continue"...
					}

#pragma omp critical (__ADD_TEMP_NODE__)
					{
						sprintf(last_node_added_string, "^5Adding foliage point ^3%i ^5at ^7%f %f %f^5.", grassSpotCount, org[0], org[1], org[2]+8);

						grassSpotList[grassSpotCount][0] = org[0];
						grassSpotList[grassSpotCount][1] = org[1];
						grassSpotList[grassSpotCount][2] = org[2]+8;

						last_org[0] = grassSpotList[grassSpotCount][0];
						last_org[1] = grassSpotList[grassSpotCount][1];
						last_org[2] = grassSpotList[grassSpotCount][2];
						grassSpotCount++;
					}

					current_height = floor;
				}
			}
		}

		trap->S_Shutup(qfalse);

		aw_percent_complete = 0.0f;

		FOLIAGE_NUM_POSITIONS = 0;

		for (i = 0; i < 262144; i++)
		{// Make sure our list is empty...
			FOLIAGE_GRASS_SELECTION[i] = 0;
			FOLIAGE_PLANT_SELECTION[i] = 0;
			FOLIAGE_TREE_SELECTION[i] = 0;
		}

		if (grassSpotCount > 0)
		{// Ok, we have spots, copy and set up foliage types/scales/angles, and save to file...
			for (i = 0; i < grassSpotCount; i++)
			{
				vec[0] = grassSpotList[i][0];
				vec[1] = grassSpotList[i][1];
				vec[2] = grassSpotList[i][2];

				FOLIAGE_GRASS_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
				FOLIAGE_GRASS_ANGLES[FOLIAGE_NUM_POSITIONS] = 0.0f;
				FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] = 0.0f;

				FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
				FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = 0.0f;
				FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = 0.0f;

				FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
				FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = 0.0f;
				FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = 0.0f;

				VectorCopy(vec, FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS]);
				FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS][2] -= 18.0;

				FOLIAGE_GRASS_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 2);
				FOLIAGE_GRASS_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
				FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,125) / 100.0);

				if (random() * 30 >= 29 && RoofHeightAt(vec) - vec[2] > 1024.0)
				{// Add tree... 1 in every 30 positions... If there is room above for a tree...
					FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
					FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
					FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
				}
				else if (random() * 100 >= 66)
				{// Add plant... 1 in every 3 positions...
					FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
					FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
					FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
				}

				FOLIAGE_NUM_POSITIONS++;
			}

			trap->Print( "^1*** ^3%s^5: Successfully generated %i grass points...\n", GAME_VERSION, FOLIAGE_NUM_POSITIONS );

			// Save the generated info to a file for next time...
			FOLIAGE_SaveFoliagePositions();
		}
		else
		{
			trap->Print( "^1*** ^3%s^5: Did not find any grass points on this map...\n", GAME_VERSION );
		}

		free(grassSpotList);

		FOLIAGE_LOADED = qtrue;
	}
}

extern qboolean CPU_CHECKED;
extern int UQ_Get_CPU_Info( void );

void FOLIAGE_GenerateFoliage ( void )
{
	char	str[MAX_TOKEN_CHARS];

	// UQ1: Check if we have an SSE CPU.. It can speed up our memory allocation by a lot!
	if (!CPU_CHECKED)
		UQ_Get_CPU_Info();

	if ( trap->Cmd_Argc() < 2 )
	{
		trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Usage:\n" );
		trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3/genfoliage <method> <density>^5. Density is optional.\n" );
		trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^5Available methods are:\n" );
		trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"standard\" ^5- Standard method. Allows you to set a density.\n");
		trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"waypoints\" ^5- Use waypoint locations. Density is ignored.\n");
		trap->UpdateScreen();
		return;
	}

	trap->Cmd_Argv( 1, str, sizeof(str) );
	
	if ( Q_stricmp( str, "standard") == 0 )
	{
		if ( trap->Cmd_Argc() >= 2 )
		{// Override normal density...
			int dist = 32;

			trap->Cmd_Argv( 2, str, sizeof(str) );
			dist = atoi(str);

			if (dist <= 4)
			{// Fallback and warning...
				dist = 32;
				trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Warning: ^5Invalid density set (%i). Using default (%i)...\n", atoi(str), 32 );
			}

			FOLIAGE_GenerateFoliage_Real(qfalse, (float)dist);
		}
		else
		{
			FOLIAGE_GenerateFoliage_Real(qfalse, 32.0);
		}
	}
	else if ( Q_stricmp( str, "waypoints") == 0 )
	{
		FOLIAGE_GenerateFoliage_Real(qtrue, 32.0);
	}
}
