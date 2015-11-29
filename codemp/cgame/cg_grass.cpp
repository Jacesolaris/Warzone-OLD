extern "C" {
#include "../qcommon/q_shared.h"
#include "../cgame/cg_local.h"
#include "../ui/ui_shared.h"
#include "../game/surfaceflags.h"

	// =======================================================================================================================================
	//
	//                                                             Foliage Rendering...
	//
	// =======================================================================================================================================

//#define		FOLIAGE_MAX_FOLIAGES 262144
//#define		FOLIAGE_MAX_FOLIAGES 524288
#define			FOLIAGE_MAX_FOLIAGES 1048576

//#define		__USE_ALL_GRASSES__ // Use all available grass shaders? Slower!
#define		__USE_EXTRA_GRASSES__ // Use extra available grass shaders? Slower!
#define		__USE_ALL_PLANTS__ // Use all available plant shaders? Slower!

#define			__USE_FOV_CULL__ // Enables FOV culling...

//#define		__USE_CLOSE_TREE_CULL__ // Can help FPS a bit but can cause stuff to flash into/out-of existance a little...
//#define		__DEBUG_CLOSE_FOV_CULLS__

#define			__USE_CLOSE_FOV_TREE_CULL__ // Can help FPS a lot in dense tree areas...
//#define		 __DEBUG_CLOSE_TREE_CULL__

//#define		__USE_PVS_CULLS__
//#define		__DEBUG_PVS_CULLS__

//#define		__NO_GRASS_AT_TREES__ // Don't draw grass at the same position as a tree (for FPS)... Little impact..
//#define		__NO_GRASS_AT_PLANTS__ // Don't draw grass at the same position as a plant (for FPS)... Little impact..
//#define		__NO_TREES__ // Don't draw trees...

#ifndef __USE_ALL_GRASSES__
#ifndef __USE_EXTRA_GRASSES__
#define		GRASS_SCALE_MULTIPLIER 1.0 // Scale down grass model by this much...
#else //!__USE_EXTRA_GRASSES__
//#define		GRASS_SCALE_MULTIPLIER 0.6 // Scale down grass model by this much...
#define		GRASS_SCALE_MULTIPLIER 1.0 // Scale down grass model by this much...
#endif //__USE_EXTRA_GRASSES__
#else //__USE_ALL_GRASSES__
#define		GRASS_SCALE_MULTIPLIER 0.6 // Scale down grass model by this much...
//#define		GRASS_SCALE_MULTIPLIER 1.0 // Scale down grass model by this much...
#endif //__USE_ALL_GRASSES__

	qboolean	FOLIAGE_LOADED = qfalse;
	int			FOLIAGE_NUM_POSITIONS = 0;
	vec3_t		FOLIAGE_POSITIONS[FOLIAGE_MAX_FOLIAGES];
	int			FOLIAGE_GRASS_SELECTION[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_GRASS_ANGLES[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_GRASS_SCALE[FOLIAGE_MAX_FOLIAGES];
	int			FOLIAGE_PLANT_SELECTION[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_PLANT_ANGLES[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_PLANT_SCALE[FOLIAGE_MAX_FOLIAGES];
	int			FOLIAGE_TREE_SELECTION[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_TREE_ANGLES[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_TREE_SCALE[FOLIAGE_MAX_FOLIAGES];

	int			IN_RANGE_FOLIAGES[65536];
	int			IN_RANGE_FOLIAGES_COUNT = 0;

	typedef enum {
		FOLIAGE_VISIBLE_NONE,
		FOLIAGE_VISIBLE_FULL,
		FOLIAGE_VISIBLE_TREE_ONLY,
	} foliageVisibilities_t;

	int			VISIBLE_FOLIAGES[65536];
	int			VISIBLE_FOLIAGES_VISTYPE[65536];
	int			VISIBLE_FOLIAGES_COUNT = 0;
	qboolean	VISIBLE_FOLIAGES_UPDATING = qfalse;

	qhandle_t	FOLIAGE_GRASS_MODEL[3] = { 0 };
	int			FOLIAGE_GRASS_SHADERNUM[FOLIAGE_MAX_FOLIAGES] = { 0 };
	qhandle_t	FOLIAGE_PLANT_MODEL[27] = { 0 };
	int			FOLIAGE_PLANT_SHADERNUM[FOLIAGE_MAX_FOLIAGES] = { 0 };
	qhandle_t	FOLIAGE_TREE_MODEL[3] = { 0 };
	
	qhandle_t	FOLIAGE_GRASS_SHADERS[37] = {0};
	qhandle_t	FOLIAGE_PLANT_SHADERS[81] = {0};

	extern qboolean InFOV( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV );
}

#include "../client/tinythread.h"
#include "../client/fast_mutex.h"

using namespace tthread;

thread		*FOLIAGE_UPDATE_THREAD = NULL;
qboolean	FOLIAGE_UPDATE_THREAD_RUNNING = qfalse;
qboolean	FOLIAGE_UPDATE_THREAD_QUIT = qfalse;

#ifdef __USE_CLOSE_TREE_CULL__
#ifdef __DEBUG_CLOSE_TREE_CULL__
int			LAST_CULL_INFO = 0;
#endif //__DEBUG_CLOSE_TREE_CULL__
#endif //__USE_CLOSE_TREE_CULL__

vec3_t		LAST_ORG = { 0 };

tthread::fast_mutex foliage_update_lock;

void FOLIAGE_UpdateThread (void * aArg)
{
	while (!FOLIAGE_UPDATE_THREAD_QUIT)
	{
		vec3_t viewOrg, viewAngles;

		VectorCopy(cg.refdef.vieworg, viewOrg);
		VectorCopy(cg.refdef.viewangles, viewAngles);

		if (Distance(cg.refdef.vieworg, LAST_ORG) > 128.0)
		{// Update in range list...
			int		NEW_IN_RANGE_FOLIAGES[65536];
			int		NEW_IN_RANGE_FOLIAGES_COUNT = 0;
			int		NUM_CLOSE_TREES = 0;

			VectorCopy(cg.refdef.vieworg, LAST_ORG);

#ifdef __USE_CLOSE_TREE_CULL__
			for (int spot = 0; spot < FOLIAGE_NUM_POSITIONS; spot++)
			{
				float dist = Distance( FOLIAGE_POSITIONS[spot], viewOrg);

				if (FOLIAGE_TREE_SELECTION[spot] != 0 && dist < 1024.0 )
				{
					NUM_CLOSE_TREES++;
					
					if (NUM_CLOSE_TREES > 13) break;
				}
			}
#endif //__USE_CLOSE_TREE_CULL__

			NEW_IN_RANGE_FOLIAGES_COUNT = 0;

			for (int spot = 0; spot < FOLIAGE_NUM_POSITIONS; spot++)
			{
				float MAX_DIST = 8192.0;//16550.0;//10000.0;//12000;//8192.0;//12000;//16550;
				float USE_DIST = MAX_DIST;
				float USE_TREE_DIST = MAX_DIST;
				float dist = Distance( FOLIAGE_POSITIONS[spot], viewOrg);

#ifdef __USE_CLOSE_TREE_CULL__
				// Scale how much we add by how many blocking trees are close to us...
				if (NUM_CLOSE_TREES > 11)
				{
					USE_DIST = 1500.0;
					USE_TREE_DIST = 4096.0;
#ifdef __DEBUG_CLOSE_TREE_CULL__
					LAST_CULL_INFO = 5;
#endif //__DEBUG_CLOSE_TREE_CULL__
				}
				else if (NUM_CLOSE_TREES > 9)
				{
					USE_DIST = 2048.0;
					USE_TREE_DIST = 6144.0;
#ifdef __DEBUG_CLOSE_TREE_CULL__
					LAST_CULL_INFO = 4;
#endif //__DEBUG_CLOSE_TREE_CULL__
				}
				else if (NUM_CLOSE_TREES > 7)
				{
					USE_DIST = 3192.0;
					USE_TREE_DIST = 8192.0;
#ifdef __DEBUG_CLOSE_TREE_CULL__
					LAST_CULL_INFO = 3;
#endif //__DEBUG_CLOSE_TREE_CULL__
				}
				else if (NUM_CLOSE_TREES > 6)
				{
					USE_DIST = 5512.0;
					USE_TREE_DIST = 10000.0;
#ifdef __DEBUG_CLOSE_TREE_CULL__
					LAST_CULL_INFO = 2;
#endif //__DEBUG_CLOSE_TREE_CULL__
				}
				else if (NUM_CLOSE_TREES > 5)
				{
					USE_DIST = 5512.0;
					USE_TREE_DIST = MAX_DIST;
#ifdef __DEBUG_CLOSE_TREE_CULL__
					LAST_CULL_INFO = 1;
#endif //__DEBUG_CLOSE_TREE_CULL__
				}
#ifdef __DEBUG_CLOSE_TREE_CULL__
				else
				{
					LAST_CULL_INFO = 0;
				}
#endif //__DEBUG_CLOSE_TREE_CULL__
#endif //__USE_CLOSE_TREE_CULL__

				if (dist < USE_DIST)
				{
#ifndef __NO_TREES__
					if (FOLIAGE_TREE_SELECTION[spot] != 0 && FOLIAGE_TREE_SCALE[spot] >= 1.0)
					{// Distant large trees are ok...
						NEW_IN_RANGE_FOLIAGES[NEW_IN_RANGE_FOLIAGES_COUNT] = spot;
						NEW_IN_RANGE_FOLIAGES_COUNT++;
					}
					else if (dist < 10000 && FOLIAGE_TREE_SELECTION[spot] != 0)
					{// Less distant smaller trees are ok...
						NEW_IN_RANGE_FOLIAGES[NEW_IN_RANGE_FOLIAGES_COUNT] = spot;
						NEW_IN_RANGE_FOLIAGES_COUNT++;
					}
					else 
#endif //__NO_TREES__
					if (dist < 2048.0/*5000.0*/ && FOLIAGE_GRASS_SCALE[spot] >= 1.0)
					{// Medium range large grasses are ok...
						NEW_IN_RANGE_FOLIAGES[NEW_IN_RANGE_FOLIAGES_COUNT] = spot;
						NEW_IN_RANGE_FOLIAGES_COUNT++;
					}
					else if (dist < 2048.0/*5000.0*/ && FOLIAGE_PLANT_SELECTION[spot] != 0 && FOLIAGE_PLANT_SCALE[spot] >= 1.0)
					{// Medium range large plants are ok...
						NEW_IN_RANGE_FOLIAGES[NEW_IN_RANGE_FOLIAGES_COUNT] = spot;
						NEW_IN_RANGE_FOLIAGES_COUNT++;
					}
					else if (dist < 1024.0/*2048.0*/ && FOLIAGE_GRASS_SCALE[spot] > 0.75)
					{// Small stuff at close range is ok...
						NEW_IN_RANGE_FOLIAGES[NEW_IN_RANGE_FOLIAGES_COUNT] = spot;
						NEW_IN_RANGE_FOLIAGES_COUNT++;
					}
					else if (dist < 1024.0/*2048.0*/ && FOLIAGE_PLANT_SELECTION[spot] != 0 && FOLIAGE_PLANT_SCALE[spot] > 0.75)
					{// Small stuff at close range is ok...
						NEW_IN_RANGE_FOLIAGES[NEW_IN_RANGE_FOLIAGES_COUNT] = spot;
						NEW_IN_RANGE_FOLIAGES_COUNT++;
					}
					else if (dist < 512.0/*1024.0*/ && FOLIAGE_GRASS_SCALE[spot] > 0.5)
					{// Small stuff at close range is ok...
						NEW_IN_RANGE_FOLIAGES[NEW_IN_RANGE_FOLIAGES_COUNT] = spot;
						NEW_IN_RANGE_FOLIAGES_COUNT++;
					}
					else if (dist < 512.0/*1024.0*/ && FOLIAGE_PLANT_SELECTION[spot] != 0 && FOLIAGE_PLANT_SCALE[spot] > 0.5)
					{// Small stuff at close range is ok...
						NEW_IN_RANGE_FOLIAGES[NEW_IN_RANGE_FOLIAGES_COUNT] = spot;
						NEW_IN_RANGE_FOLIAGES_COUNT++;
					}
					else if (dist < 256.0/*768.0*/)
					{// Tiny stuff at close range is ok...
						NEW_IN_RANGE_FOLIAGES[NEW_IN_RANGE_FOLIAGES_COUNT] = spot;
						NEW_IN_RANGE_FOLIAGES_COUNT++;
					}
				}
#ifndef __NO_TREES__
				else if (dist < USE_TREE_DIST)
				{
					if (FOLIAGE_TREE_SELECTION[spot] != 0 && FOLIAGE_TREE_SCALE[spot] >= 1.0)
					{// Distant large trees are ok...
						NEW_IN_RANGE_FOLIAGES[NEW_IN_RANGE_FOLIAGES_COUNT] = spot;
						NEW_IN_RANGE_FOLIAGES_COUNT++;
					}
					else if (dist < 10000 && FOLIAGE_TREE_SELECTION[spot] != 0)
					{// Less distant smaller trees are ok...
						NEW_IN_RANGE_FOLIAGES[NEW_IN_RANGE_FOLIAGES_COUNT] = spot;
						NEW_IN_RANGE_FOLIAGES_COUNT++;
					}
				}
#endif //__NO_TREES__
			}

			foliage_update_lock.lock();
			memcpy(IN_RANGE_FOLIAGES, NEW_IN_RANGE_FOLIAGES, sizeof(int)*65536);
			IN_RANGE_FOLIAGES_COUNT = NEW_IN_RANGE_FOLIAGES_COUNT;
			foliage_update_lock.unlock();
		}

		this_thread::sleep_for(chrono::milliseconds(200));
	}

	FOLIAGE_UPDATE_THREAD_RUNNING = qfalse;
}

void FOLIAGE_StartUpdateThread ( void )
{
	if (!FOLIAGE_UPDATE_THREAD_RUNNING && !FOLIAGE_UPDATE_THREAD_QUIT)
	{// Run in background thread...
		FOLIAGE_UPDATE_THREAD_QUIT = qfalse;
		FOLIAGE_UPDATE_THREAD_RUNNING = qtrue;
		FOLIAGE_UPDATE_THREAD = new thread (FOLIAGE_UpdateThread, (void *)NULL);
	}
}

void FOLIAGE_ShutdownUpdateThread ( void )
{
	FOLIAGE_UPDATE_THREAD_QUIT = qtrue;
	
	while (FOLIAGE_UPDATE_THREAD_RUNNING)
	{
		this_thread::sleep_for(chrono::milliseconds(1));
	}
}

extern "C" {
	void FOLIAGE_CheckUpdateThread ( void )
	{
		FOLIAGE_StartUpdateThread();
	}

	void FOLIAGE_KillUpdateThread ( void )
	{
		FOLIAGE_ShutdownUpdateThread();
	}

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

#ifdef __NO_TREES__
		if (treeOnly) return;
#endif //__NO_TREES__

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
#ifdef __USE_ALL_GRASSES__
			if (FOLIAGE_GRASS_SHADERNUM[num] > 0)
			{// Need to specify a shader...
				re.customShader = FOLIAGE_GRASS_SHADERS[FOLIAGE_GRASS_SHADERNUM[num]];
			}
#endif //__USE_ALL_GRASSES__

#ifdef __USE_EXTRA_GRASSES__
			if (FOLIAGE_GRASS_SHADERNUM[num] > 0)
			{// Need to specify a shader...
				re.customShader = FOLIAGE_GRASS_SHADERS[FOLIAGE_GRASS_SHADERNUM[num]];
			}
#endif //__USE_EXTRA_GRASSES__

#ifdef __NO_GRASS_AT_TREES__
			if (FOLIAGE_TREE_SELECTION[num] == 0)
			{
#ifdef __NO_GRASS_AT_PLANTS__
				if (FOLIAGE_PLANT_SELECTION[num] == 0)
				{
#endif //__NO_GRASS_AT_PLANTS__
					re.hModel = FOLIAGE_GRASS_MODEL[0];
					VectorSet(re.modelScale, FOLIAGE_GRASS_SCALE[num] * GRASS_SCALE_MULTIPLIER, FOLIAGE_GRASS_SCALE[num] * GRASS_SCALE_MULTIPLIER, FOLIAGE_GRASS_SCALE[num] * GRASS_SCALE_MULTIPLIER);
					angles[PITCH] = angles[ROLL] = 0.0f;
					angles[YAW] = FOLIAGE_GRASS_ANGLES[num];
					VectorCopy(angles, re.angles);
					AnglesToAxis(angles, re.axis);
					ScaleModelAxis( &re );
					FOLIAGE_AddFoliageEntityToScene( &re );
#ifdef __NO_GRASS_AT_PLANTS__
				}
#endif //__NO_GRASS_AT_PLANTS__
			}
#else //!__NO_GRASS_AT_TREES__
#ifdef __NO_GRASS_AT_PLANTS__
			if (FOLIAGE_PLANT_SELECTION[num] == 0)
			{
#endif //__NO_GRASS_AT_PLANTS__
				re.hModel = FOLIAGE_GRASS_MODEL[0];
				VectorSet(re.modelScale, FOLIAGE_GRASS_SCALE[num] * GRASS_SCALE_MULTIPLIER, FOLIAGE_GRASS_SCALE[num] * GRASS_SCALE_MULTIPLIER, FOLIAGE_GRASS_SCALE[num] * GRASS_SCALE_MULTIPLIER);
				angles[PITCH] = angles[ROLL] = 0.0f;
				angles[YAW] = FOLIAGE_GRASS_ANGLES[num];
				VectorCopy(angles, re.angles);
				AnglesToAxis(angles, re.axis);
				ScaleModelAxis( &re );
				FOLIAGE_AddFoliageEntityToScene( &re );
#ifdef __NO_GRASS_AT_PLANTS__
			}
#endif //__NO_GRASS_AT_PLANTS__
#endif //__NO_GRASS_AT_TREES__
		}

		re.customShader = 0;

#ifndef __NO_TREES__
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
		else 
#endif //__NO_TREES__
		if (FOLIAGE_PLANT_SELECTION[num] != 0 
			&& !treeOnly || (dist < 5000.0 && FOLIAGE_PLANT_SCALE[num] >= 1.0)
			&& (!(skip_mediumStuff && FOLIAGE_PLANT_SCALE[num] > 0.75) || (dist < 5000.0 && FOLIAGE_PLANT_SCALE[num] >= 1.0))
			&& !(skip_smallStuff && FOLIAGE_PLANT_SCALE[num] <= 0.75)
			&& !(skip_tinyStuff && FOLIAGE_GRASS_SCALE[num] <= 0.5))
		{// Add plant model as well...
#ifdef __USE_ALL_PLANTS__
			if (FOLIAGE_PLANT_SHADERNUM[num] > 0)
			{// Need to specify a shader...
				re.customShader = FOLIAGE_PLANT_SHADERS[FOLIAGE_PLANT_SHADERNUM[num]];
			}
#endif //__USE_ALL_PLANTS__

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

		for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
		{
#ifdef __USE_EXTRA_GRASSES__
			if (irand(0,5) <= 3)
				FOLIAGE_GRASS_SHADERNUM[i] = 0;
			else
				FOLIAGE_GRASS_SHADERNUM[i] = irand(1,36);
#else //!__USE_EXTRA_GRASSES__
			FOLIAGE_GRASS_SHADERNUM[i] = irand(1,36);
#endif //__USE_EXTRA_GRASSES__
			FOLIAGE_PLANT_SHADERNUM[i] = irand(1,81);
		}

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
			//FOLIAGE_GRASS_MODEL[0] = trap->R_RegisterModel( "models/warzone/foliage/grass_dense.md3" );
			//FOLIAGE_GRASS_MODEL[1] = trap->R_RegisterModel( "models/warzone/foliage/grass_cross.md3" );
			//FOLIAGE_GRASS_MODEL[2] = trap->R_RegisterModel( "models/warzone/foliage/grass_cross.md3" );
			//FOLIAGE_GRASS_MODEL[1] = trap->R_RegisterModel( "models/warzone/foliage/grass_dense.md3" );
			//FOLIAGE_GRASS_MODEL[2] = trap->R_RegisterModel( "models/warzone/foliage/grass_dense.md3" );

#if !defined (__USE_ALL_GRASSES__) && !defined (__USE_EXTRA_GRASSES__)
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
#else //!__USE_ALL_GRASSES__ && !__USE_EXTRA_GRASSES__
			int last = 0;

			for (int i = 0; i < 27; i++)
			{
				/*if (last > 8) last = 0;

				if (last >= 7)
				{
					FOLIAGE_PLANT_MODEL[i] = trap->R_RegisterModel("models/pop/foliages/sch_weed_a.md3");
				}
				else if (last >= 6)
				{
					FOLIAGE_PLANT_MODEL[i] = trap->R_RegisterModel("models/pop/foliages/sch_weed_b.md3");
				}
				else if (last >= 4)
				{
					FOLIAGE_PLANT_MODEL[i] = trap->R_RegisterModel("models/warzone/foliage/plant03.md3");
				}
				else
				{
					FOLIAGE_PLANT_MODEL[i] = trap->R_RegisterModel("models/warzone/foliage/grass_cross.md3");
				}

				last++;*/

				FOLIAGE_PLANT_MODEL[i] = trap->R_RegisterModel("models/warzone/foliage/plant03.md3");
			}
#endif //__USE_ALL_GRASSES__

			FOLIAGE_TREE_MODEL[0] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );
			FOLIAGE_TREE_MODEL[1] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );
			FOLIAGE_TREE_MODEL[2] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );

			for (int i = 1; i < 37; i++)
			{
				char	shaderName[128] = {0};
				int		shaderNum = i;//irand(1,36);

				if (shaderNum < 10)
					sprintf(shaderName, "models/warzone/foliage/grass0%i.png", shaderNum);
				else
					sprintf(shaderName, "models/warzone/foliage/grass%i.png", shaderNum);

				FOLIAGE_GRASS_SHADERS[i] = trap->R_RegisterShader(shaderName);
			}

			for (int i = 1; i < 81; i++)
			{
				char	shaderName[128] = {0};
				int		shaderNum = i;//irand(1,81);

				if (shaderNum < 10)
					sprintf(shaderName, "models/warzone/foliage/plant0%i.png", shaderNum);
				else
					sprintf(shaderName, "models/warzone/foliage/plant%i.png", shaderNum);

				FOLIAGE_PLANT_SHADERS[i] = trap->R_RegisterShader(shaderName);
			}
		}

		FOLIAGE_CheckUpdateThread();

		vec3_t viewOrg, viewAngles;

		VectorCopy(cg.refdef.vieworg, viewOrg);
		VectorCopy(cg.refdef.viewangles, viewAngles);

		foliage_update_lock.lock();

		//clock_t waitBegin = clock();
		VISIBLE_FOLIAGES_COUNT = 0;
		//clock_t waitEnd = clock();
		//int waitTime = waitEnd - waitBegin;

		//if (waitTime > 0) trap->Print("Foliage system waited %i ms for lock.\n");

#ifdef __USE_FOV_CULL__

#ifdef __DEBUG_CLOSE_TREE_CULL__
		trap->Print("Foliage cull type: %i.\n", LAST_CULL_INFO);
#endif //__DEBUG_CLOSE_TREE_CULL__

#ifdef __USE_CLOSE_FOV_TREE_CULL__
		int FOV_CLOSE_TREE_COUNT = 0;

#ifndef __NO_TREES__
		for (int spot = 0; spot < IN_RANGE_FOLIAGES_COUNT; spot++)
		{// Draw anything closeish to us...
			qboolean	treeOnly = qfalse;
			float		len = Distance(FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg);

			if (FOV_CLOSE_TREE_COUNT > 9) break;

			if ( FOLIAGE_TREE_SELECTION[IN_RANGE_FOLIAGES[spot]] != 0 )
			{
				if (len <= 850.0)
				{
					if ( /*len <= 192.0 ||*/ InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 20, cg.refdef.fov_y + 20 ))
						FOV_CLOSE_TREE_COUNT++;
					else
						continue;
				}
			}
		}
#endif //__NO_TREES__

		int FOV_ULTRA_CLOSE_TREE_COUNT = 0;

#ifndef __NO_TREES__
		for (int spot = 0; spot < IN_RANGE_FOLIAGES_COUNT; spot++)
		{// Draw anything closeish to us...
			qboolean	treeOnly = qfalse;
			float		len = Distance(FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg);

			if (FOV_ULTRA_CLOSE_TREE_COUNT > 3) break;

			if ( FOLIAGE_TREE_SELECTION[IN_RANGE_FOLIAGES[spot]] != 0 )
			{
				if (len <= 450.0)
				{
					if ( len <= 96.0 || InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, 150, 150 ))
						FOV_ULTRA_CLOSE_TREE_COUNT++;
					else
						continue;
				}
			}
		}
#endif //__NO_TREES__
#endif //__USE_CLOSE_FOV_TREE_CULL__

		//
		// Select and draw visible foliages from the in-range list...
		//

#ifdef __DEBUG_CLOSE_FOV_CULLS__
		int NUM_CLOSE_FOV_TREE_CULLS = 0;
#endif //__DEBUG_CLOSE_FOV_CULLS__
		
#ifdef __DEBUG_PVS_CULLS__
		int NUM_PVS_CULLS = 0;
#endif //__DEBUG_PVS_CULLS__

		for (int spot = 0; spot < IN_RANGE_FOLIAGES_COUNT; spot++)
		{// Draw anything closeish to us...
			qboolean	treeOnly = qfalse;
			float		len = Distance(FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg);

#ifndef __NO_TREES__
			if ( (FOLIAGE_TREE_SELECTION[IN_RANGE_FOLIAGES[spot]] != 0 && len > 3192.0) 
				|| (len < 5000.0 && FOLIAGE_GRASS_SCALE[IN_RANGE_FOLIAGES[spot]] >= 1.0))
			{
				treeOnly = qtrue;
			}
			else 
#endif //__NO_TREES__
			if ( len > 3192.0 ) 
			{
				continue;
			}

#ifndef __NO_TREES__
			if ( FOLIAGE_TREE_SELECTION[IN_RANGE_FOLIAGES[spot]] != 0 )
			{// Tree here, wider view FOV check...
				if ( len > 4096 && !InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 3, cg.refdef.fov_y + 3 ))
					continue;
				else if ( len > 2048 && !InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 5, cg.refdef.fov_y + 5 ))
					continue;
				else if ( len > 1024 && !InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 10, cg.refdef.fov_y + 10 ))
					continue;
				else if ( len > 256 && !InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 20, cg.refdef.fov_y + 20 ))
					continue;

#ifdef __USE_CLOSE_FOV_TREE_CULL__
				if ( (FOV_CLOSE_TREE_COUNT > 7 || FOV_ULTRA_CLOSE_TREE_COUNT > 3) 
					&& len > 2048.0 
					&& InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 20, cg.refdef.fov_y + 20 ))
				{
#ifdef __DEBUG_CLOSE_FOV_CULLS__
					NUM_CLOSE_FOV_TREE_CULLS++;
#endif //__DEBUG_CLOSE_FOV_CULLS__
					continue; // Trees cover this up hopefully...
				}

				if ( FOV_CLOSE_TREE_COUNT > 9 
					&& len > 3192.0 
					&& InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 20, cg.refdef.fov_y + 20 ))
				{
#ifdef __DEBUG_CLOSE_FOV_CULLS__
					NUM_CLOSE_FOV_TREE_CULLS++;
#endif //__DEBUG_CLOSE_FOV_CULLS__
					continue; // Trees cover this up hopefully...
				}
#endif //__USE_CLOSE_FOV_TREE_CULL__
			}
			else
#endif //__NO_TREES__
			{// No tree here, can use minimal FOV check...
				if ( len > 4096 && !InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 1, cg.refdef.fov_y + 1 ))
					continue;
				else if ( len > 2048 && !InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 2, cg.refdef.fov_y + 2 ))
					continue;
				else if ( len > 1024 && !InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 3, cg.refdef.fov_y + 3 ))
					continue;
				else if ( len > 256 && !InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 20, cg.refdef.fov_y + 5 ))
					continue;

#ifdef __USE_CLOSE_FOV_TREE_CULL__
				if ( (FOV_CLOSE_TREE_COUNT > 7 || FOV_ULTRA_CLOSE_TREE_COUNT > 3) 
					&& len > 1500.0 
					&& InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 20, cg.refdef.fov_y + 20 ))
				{
#ifdef __DEBUG_CLOSE_FOV_CULLS__
					NUM_CLOSE_FOV_TREE_CULLS++;
#endif //__DEBUG_CLOSE_FOV_CULLS__
					continue; // Trees cover this up hopefully...
				}

				if ( FOV_CLOSE_TREE_COUNT > 9 
					&& len > 2048.0 
					&& InFOV( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, viewAngles, cg.refdef.fov_x + 5, cg.refdef.fov_y + 5 ))
				{
#ifdef __DEBUG_CLOSE_FOV_CULLS__
					NUM_CLOSE_FOV_TREE_CULLS++;
#endif //__DEBUG_CLOSE_FOV_CULLS__
					continue; // Trees cover this up hopefully...
				}
#endif //__USE_CLOSE_FOV_TREE_CULL__
			}

#ifdef __USE_PVS_CULLS__
			if( !trap->R_InPVS( FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg, cg.refdef.areamask ) ) 
			{
#ifdef __DEBUG_PVS_CULLS__
				NUM_PVS_CULLS++;
#endif //__DEBUG_PVS_CULLS__
				continue;
			}
#endif //__USE_PVS_CULLS__

			VISIBLE_FOLIAGES[VISIBLE_FOLIAGES_COUNT] = IN_RANGE_FOLIAGES[spot];
			
			if (treeOnly)
				VISIBLE_FOLIAGES_VISTYPE[VISIBLE_FOLIAGES_COUNT] = FOLIAGE_VISIBLE_TREE_ONLY;
			else
				VISIBLE_FOLIAGES_VISTYPE[VISIBLE_FOLIAGES_COUNT] = FOLIAGE_VISIBLE_FULL;

			VISIBLE_FOLIAGES_COUNT++;
		}

#ifdef __DEBUG_CLOSE_FOV_CULLS__
		trap->Print("Foliage CLOSE FOV culls: %i.\n", NUM_CLOSE_FOV_TREE_CULLS);
#endif //__DEBUG_CLOSE_FOV_CULLS__
#ifdef __DEBUG_PVS_CULLS__
		trap->Print("Foliage PVS culls: %i.\n", NUM_PVS_CULLS);
#endif //__DEBUG_PVS_CULLS__

		for (spot = 0; spot < VISIBLE_FOLIAGES_COUNT; spot++)
		{
			if (VISIBLE_FOLIAGES_VISTYPE[spot] == FOLIAGE_VISIBLE_TREE_ONLY)
				FOLIAGE_AddToScreen( VISIBLE_FOLIAGES[spot], qtrue );
			else
				FOLIAGE_AddToScreen( VISIBLE_FOLIAGES[spot], qfalse );
		}

#else //!__USE_FOV_CULL__
		for (int spot = 0; spot < IN_RANGE_FOLIAGES_COUNT; spot++)
		{
			float		len = Distance(FOLIAGE_POSITIONS[IN_RANGE_FOLIAGES[spot]], viewOrg);

			if ( (FOLIAGE_TREE_SELECTION[IN_RANGE_FOLIAGES[spot]] != 0 && len > 3192.0) 
				|| (len < 5000.0 && FOLIAGE_GRASS_SCALE[IN_RANGE_FOLIAGES[spot]] >= 1.0))
				FOLIAGE_AddToScreen( IN_RANGE_FOLIAGES[spot], qtrue );
			else
				FOLIAGE_AddToScreen( IN_RANGE_FOLIAGES[spot], qfalse );
		}
#endif //__USE_FOV_CULL__

		foliage_update_lock.unlock();
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

		CG_Trace( &tr, org1, NULL, NULL, org2, -1, MASK_PLAYERSOLID|CONTENTS_WATER );

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

		if ( /*tr.contents & CONTENTS_WATER ||*/ (tr.surfaceFlags & MATERIAL_MASK) == MATERIAL_WATER )
		{// Water... I'm just gonna ignore these!
			return -65536.0f;
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

		if ( tr.startsolid || tr.allsolid )
		{
			return 65536.0f;
		}

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

		VectorSet(mins, -15, -15, DEFAULT_MINS_2);
		VectorSet(maxs, 15, 15, DEFAULT_MAXS_2);

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

#define FOLIAGE_MOD_NAME		"aimod"
float	FOLIAGE_FILE_VERSION =	1.1f;

	void FOLIAGE_GenerateFoliage_Real ( qboolean USE_WAYPOINTS, float density, qboolean ADD_MORE )
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

		if (!ADD_MORE)
		{
			FOLIAGE_NUM_POSITIONS = 0;

			for (i = 0; i < FOLIAGE_MAX_FOLIAGES; i++)
			{// Make sure our list is empty...
				FOLIAGE_GRASS_SELECTION[i] = 0;
				FOLIAGE_PLANT_SELECTION[i] = 0;
				FOLIAGE_TREE_SELECTION[i] = 0;
			}
		}

		i = 0;

		if (USE_WAYPOINTS && !ADD_MORE)
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
			
			/*if ( Q_stricmp( map, mp) != 0 )
			{
				trap->Print( "^1*** ^3WARNING^5: Reading from ^7nodes/%s.bwp^3 failed^5!!!\n", filename );
				trap->Print( "^1*** ^3       ^5  Node file is not for this map!\n" );
				trap->FS_Close( f );
				return;
			}*/

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

			for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
			{
#ifdef __USE_EXTRA_GRASSES__
				if (irand(0,5) <= 3)
					FOLIAGE_GRASS_SHADERNUM[i] = 0;
				else
					FOLIAGE_GRASS_SHADERNUM[i] = irand(1,36);
#else //!__USE_EXTRA_GRASSES__
				FOLIAGE_GRASS_SHADERNUM[i] = irand(1,36);
#endif //__USE_EXTRA_GRASSES__
				FOLIAGE_PLANT_SHADERNUM[i] = irand(1,81);
			}

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

			grassSpotList = (vec3_t *)malloc((sizeof(vec3_t)+1)*FOLIAGE_MAX_FOLIAGES);

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
			aw_percent_complete = 0;

			for (parallel_x = 0; parallel_x < parallel_x_max; parallel_x++) // To OMP this sucker...
			{
				int		x;
				int		parallel_y = 0;
				float	scatter_y = scatter;
				float	scatter_mult_X = 1.0;
				vec3_t	last_org;

				if (grassSpotCount >= FOLIAGE_MAX_FOLIAGES)
				{
					continue;
				}

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

				if(omp_get_thread_num() == 0)
				{// Draw a nice little progress bar ;)
					if (clock() - previous_time > 500) // update display every 500ms...
					{
						previous_time = clock();
						trap->UpdateScreen();
					}
				}

#pragma omp parallel for ordered schedule(dynamic)
				for (parallel_y = 0; parallel_y < parallel_y_max; parallel_y++) // To OMP this sucker...
				{
					int		z, y;
					float	current_height = mapMaxs[2]; // Init the current height to max map height...
					float	scatter_mult_Y = 1.0;

					if (grassSpotCount >= FOLIAGE_MAX_FOLIAGES)
					{
						continue;
					}

					aw_percent_complete = (float)((float)parallel_x / (float)parallel_x_max) * 100.0f;
					//aw_percent_complete = (float)((float)((parallel_x+65536) / (parallel_x_max+65536)) + (float)((float)((parallel_y+65536) / (parallel_y_max+65536)) * (float)(1.0 / (parallel_x_max+65536)))) * 100.0f;

					// Vary Y scatter distance...
					if (scatter_y == scatter) scatter_y = scatter_min;
					else if (scatter_y == scatter) scatter_y = scatter_max;
					else scatter_y = scatter;

					y = (starty + offsetY) - (parallel_y * (scatter_y * scatter_mult_Y));

					if(omp_get_thread_num() == 0)
					{// Draw a nice little progress bar ;)
						if (clock() - previous_time > 500) // update display every 500ms...
						{
							previous_time = clock();
							trap->UpdateScreen();
						}
					}

					for (z = startz; z >= mapMins[2]; z -= scatter_min)
					{
						vec3_t		new_org, org;
						float		floor = 0;

						if (grassSpotCount >= FOLIAGE_MAX_FOLIAGES)
						{
							continue;
						}

						//aw_percent_complete = (float)((float)((parallel_x+65536) / (parallel_x_max+65536)) + (float)((float)((parallel_y+65536) / (parallel_y_max+65536)) * (float)(1.0 / (parallel_x_max+65536)))) * 100.0f;

						// Update the current test number...
						final_tests++;

						if(omp_get_thread_num() == 0)
						{// Draw a nice little progress bar ;)
							if (clock() - previous_time > 500) // update display every 500ms...
							{
								previous_time = clock();
								trap->UpdateScreen();
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
						/*else if (Distance(org, last_org) < density)
						{
							current_height = floor;
							continue;
						}*/

						/*if (!FOLIAGE_Check_Width(org))
						{// Not wide enough for a player to fit!
							current_height = floor;
							continue;
						}*/

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

				if (grassSpotCount >= FOLIAGE_MAX_FOLIAGES)
				{
					break;
				}
			}

			if (grassSpotCount >= FOLIAGE_MAX_FOLIAGES)
			{
				trap->Print( "^1*** ^3%s^5: Too many foliage points detected... Try again with a higher density value...\n", GAME_VERSION );
				return;
			}

			trap->S_Shutup(qfalse);

			aw_percent_complete = 0.0f;

			if (!ADD_MORE) 
			{
				FOLIAGE_NUM_POSITIONS = 0;

				for (i = 0; i < FOLIAGE_MAX_FOLIAGES; i++)
				{// Make sure our list is empty...
					FOLIAGE_GRASS_SELECTION[i] = 0;
					FOLIAGE_PLANT_SELECTION[i] = 0;
					FOLIAGE_TREE_SELECTION[i] = 0;
				}
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

					if (density >= 64)
					{
						if (random() * 10 >= 9 && RoofHeightAt(vec) - vec[2] > 1024.0)
						{// Add tree... 1 in every 10 positions... If there is room above for a tree...
							FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
							FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
						}
						else if (random() * 100 >= 25)
						{// Add plant... 1 in every 1.25 positions...
							FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
							FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
						}
					}
					else if (density >= 48)
					{
						if (random() * 20 >= 19 && RoofHeightAt(vec) - vec[2] > 1024.0)
						{// Add tree... 1 in every 20 positions... If there is room above for a tree...
							FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
							FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
						}
						else if (random() * 100 >= 50)
						{// Add plant... 1 in every 2 positions...
							FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
							FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
						}
					}
					else
					{
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
					}

					FOLIAGE_NUM_POSITIONS++;
				}

				for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
				{
#ifdef __USE_EXTRA_GRASSES__
					if (irand(0,5) <= 3)
						FOLIAGE_GRASS_SHADERNUM[i] = 0;
					else
						FOLIAGE_GRASS_SHADERNUM[i] = irand(1,36);
#else //!__USE_EXTRA_GRASSES__
					FOLIAGE_GRASS_SHADERNUM[i] = irand(1,36);
#endif //__USE_EXTRA_GRASSES__
					FOLIAGE_PLANT_SHADERNUM[i] = irand(1,81);
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
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"add\" ^5- Standard method. Allows you to set a density. This one adds to current list of foliages.\n");
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

				FOLIAGE_GenerateFoliage_Real(qfalse, (float)dist, qfalse);
			}
			else
			{
				FOLIAGE_GenerateFoliage_Real(qfalse, 32.0, qfalse);
			}
		}
		else if ( Q_stricmp( str, "waypoints") == 0 )
		{
			FOLIAGE_GenerateFoliage_Real(qtrue, 32.0, qfalse);
		}
		else if ( Q_stricmp( str, "add") == 0 )
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

				FOLIAGE_GenerateFoliage_Real(qfalse, (float)dist, qtrue);
			}
			else
			{
				FOLIAGE_GenerateFoliage_Real(qfalse, 32.0, qtrue);
			}
		}
	}
}
