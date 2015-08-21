#include "../qcommon/q_shared.h"
#include "../cgame/cg_local.h"
#include "../ui/ui_shared.h"
#include "../game/surfaceflags.h"

qboolean	GRASSES_LOADED = qfalse;
int			NUM_GRASS_POSITIONS = 0;
vec3_t		GRASS_POSITIONS[262144];
int			GRASS_SELECTION[262144];
float		GRASS_ANGLES[262144];
float		GRASS_SCALE[262144];
int			TREE_TYPE[262144];
float		TREE_SCALE[262144];

qhandle_t	GRASS_MODELS[7] = { 0, 0, 0, 0, 0, 0, 0 };
qhandle_t	TREE_MODEL[3] = { 0, 0, 0 };

void CG_AddGrassModel( int num ) {
	refEntity_t		re;
	vec3_t			angles;

	memset( &re, 0, sizeof( re ) );

	if (!GRASS_MODELS[0])
	{
		GRASS_MODELS[0] = trap->R_RegisterModel( "models/map_objects/yavin/grass_b.md3" );
		GRASS_MODELS[1] = trap->R_RegisterModel( "models/pop/foliages/alp_weedy_c.md3" );
		GRASS_MODELS[2] = trap->R_RegisterModel( "models/pop/foliages/sch_weed_a.md3" );
		GRASS_MODELS[3] = trap->R_RegisterModel( "models/pop/foliages/sch_weed_b.md3" );
		GRASS_MODELS[4] = trap->R_RegisterModel( "models/pop/foliages/pop_flower_a.md3" );
		GRASS_MODELS[5] = trap->R_RegisterModel( "models/pop/foliages/pop_flower_b.md3" );
		GRASS_MODELS[6] = trap->R_RegisterModel( "models/pop/foliages/pop_flower_c.md3" );
		TREE_MODEL[0] = trap->R_RegisterModel( "models/map_objects/yavin/tree06_b.md3" );
		TREE_MODEL[1] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );
		TREE_MODEL[2] = trap->R_RegisterModel( "models/map_objects/yavin/tree09_b.md3" );
	}

	VectorCopy(GRASS_POSITIONS[num], re.origin);
	angles[PITCH] = angles[ROLL] = 0.0f;
	angles[YAW] = GRASS_ANGLES[num];

	re.reType = RT_MODEL;
	re.hModel = GRASS_MODELS[0];

	VectorCopy(angles, re.angles);
	AnglesToAxis(angles, re.axis);

	VectorSet(re.modelScale, GRASS_SCALE[num], GRASS_SCALE[num], GRASS_SCALE[num]);
	ScaleModelAxis( &re );

	AddRefEntityToScene( &re );

	if (TREE_TYPE[num] == 0)
	{// Add a smaller version inside this one to fill the massive hole in the center of the model...
		if (GRASS_SELECTION[num] == 0)
		{// Smaller grass...
			re.origin[2] += 12.0;
			VectorSet(re.modelScale, GRASS_SCALE[num] / 2.0, GRASS_SCALE[num] / 2.0, GRASS_SCALE[num] / 2.0);
			ScaleModelAxis( &re );
			AddRefEntityToScene( &re );
		}
		else if (GRASS_SELECTION[num] == 4 || GRASS_SELECTION[num] == 5 || GRASS_SELECTION[num] == 6)
		{// Flowers...
			re.origin[2] += 12.0;
			re.hModel = GRASS_MODELS[GRASS_SELECTION[num]];
			VectorSet(re.modelScale, GRASS_SCALE[num] * 0.2, GRASS_SCALE[num] * 0.2, GRASS_SCALE[num] * 0.2);
			ScaleModelAxis( &re );
			AddRefEntityToScene( &re );
		}
		else
		{// Add one of the other random grass options inside...
			re.hModel = GRASS_MODELS[GRASS_SELECTION[num]];
			VectorSet(re.modelScale, GRASS_SCALE[num] * 1.5, GRASS_SCALE[num] * 1.5, GRASS_SCALE[num] * 1.5);
			ScaleModelAxis( &re );
			AddRefEntityToScene( &re );
		}
	}
	else
	{// Add a tree type...
		re.hModel = TREE_MODEL[TREE_TYPE[num]];
		VectorSet(re.modelScale, TREE_SCALE[num], TREE_SCALE[num], TREE_SCALE[num]);
		ScaleModelAxis( &re );

		AddRefEntityToScene( &re );
	}
}

extern qboolean InFOV( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV );

qboolean FloorIsGrassAt ( vec3_t org )
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

#define GRASS_MOD_NAME	"aimod"
float	GRASS_NOD_VERSION = 1.1f;

void
AIMOD_NODES_LoadGrass ( void )
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
	char			name[] = GRASS_MOD_NAME;
	char			nm[64] = "";
	float			version;
	char			map[64] = "";
	char			mp[64] = "";
	/*short*/ int		numberNodes;
	short int		temp, fix_aas_nodes;

	NUM_GRASS_POSITIONS = 0;
	GRASSES_LOADED = qfalse;

	i = 0;

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

	if ( version != GRASS_NOD_VERSION && version != 1.0f )
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

	if (version == GRASS_NOD_VERSION)
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

		if (FloorIsGrassAt(vec))
		{
			VectorCopy(vec, GRASS_POSITIONS[NUM_GRASS_POSITIONS]);
			GRASS_POSITIONS[NUM_GRASS_POSITIONS][2] -= 18.0;
			GRASS_SELECTION[NUM_GRASS_POSITIONS] = irand(0,6);
			/*
			switch(GRASS_SELECTION[NUM_GRASS_POSITIONS])
			{
			case 1:
				GRASS_ANGLES[NUM_GRASS_POSITIONS] = irand(0,180);
				GRASS_SCALE[NUM_GRASS_POSITIONS] = (float)((float)irand(65,95) / 100.0);
				break;
			case 2:
				GRASS_ANGLES[NUM_GRASS_POSITIONS] = irand(0,180);
				GRASS_SCALE[NUM_GRASS_POSITIONS] = (float)((float)irand(65,95) / 100.0);
				break;
			case 3:
				GRASS_ANGLES[NUM_GRASS_POSITIONS] = irand(0,180);
				GRASS_SCALE[NUM_GRASS_POSITIONS] = (float)((float)irand(65,95) / 100.0);
				break;
			case 4:
				GRASS_ANGLES[NUM_GRASS_POSITIONS] = irand(0,180);
				GRASS_SCALE[NUM_GRASS_POSITIONS] = (float)((float)irand(25,40) / 100.0);
				break;
			case 5:
				GRASS_ANGLES[NUM_GRASS_POSITIONS] = irand(0,180);
				GRASS_SCALE[NUM_GRASS_POSITIONS] = (float)((float)irand(25,40) / 100.0);
				break;
			case 6:
				GRASS_ANGLES[NUM_GRASS_POSITIONS] = irand(0,180);
				GRASS_SCALE[NUM_GRASS_POSITIONS] = (float)((float)irand(25,40) / 100.0);
				break;
			default:
				GRASS_ANGLES[NUM_GRASS_POSITIONS] = irand(0,180);
				GRASS_SCALE[NUM_GRASS_POSITIONS] = (float)((float)irand(65,95) / 100.0);
				break;
			}
			*/
			GRASS_ANGLES[NUM_GRASS_POSITIONS] = irand(0,180);
			GRASS_SCALE[NUM_GRASS_POSITIONS] = (float)((float)irand(65,95) / 100.0);
			

			if (irand(0,10) >= 10)
			{
				TREE_TYPE[NUM_GRASS_POSITIONS] = irand(0,3);
				TREE_SCALE[NUM_GRASS_POSITIONS] = (float)((float)irand(45,85) / 100.0);
			}
			else
			{
				TREE_TYPE[NUM_GRASS_POSITIONS] = 0;
				TREE_SCALE[NUM_GRASS_POSITIONS] = 0.0f;
			}

			NUM_GRASS_POSITIONS++;
		}

		//loop through all of the links and read the data
		for ( j = 0; j < numLinks; j++ )
		{
			if (version == GRASS_NOD_VERSION)
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
	trap->Print( "^1*** ^3%s^5: Successfully loaded %i grass points from waypoint file ^7nodes/%s.bwp^5.\n", GAME_VERSION,
			  NUM_GRASS_POSITIONS, cgs.currentmapname );

	return;
}

void DrawGrass()
{
	int spot = 0;

	if (!GRASSES_LOADED)
	{
		AIMOD_NODES_LoadGrass();
		GRASSES_LOADED = qtrue;
	}

	if (NUM_GRASS_POSITIONS <= 0)
	{
		return;
	}

	for (spot = 0; spot < NUM_GRASS_POSITIONS; spot++)
	{
		// Draw anything closeish to us...
		int		len = 0;
		int		link = 0;
		vec3_t	delta;

		if (!InFOV( GRASS_POSITIONS[spot], cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x + 20, cg.refdef.fov_y + 20 ))
			continue;

		VectorSubtract( GRASS_POSITIONS[spot], cg.refdef.vieworg, delta );
		len = VectorLength( delta );
		
		if ( len > 3192 ) continue;

		CG_AddGrassModel( spot );
	}
}
