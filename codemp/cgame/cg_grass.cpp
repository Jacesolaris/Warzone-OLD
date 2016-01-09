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


//#define		FOLIAGE_MAX_FOLIAGES 524288
#define			FOLIAGE_MAX_FOLIAGES 1048576

//
// BEGIN - FOLIAGE OPTIONS
//

//#define			__FOLIAGE_AREA_DEBUGGING__ // Enables debugging info display of foliages not assigned to an area...

#define			__PREGENERATE_NORMALS__ // Generate normals for each foliage at map load instead of in realtime... Seems slower???!?!?!?!

//#define		__NO_PLANTS__ // Disable plants...
//#define		__USE_ALL_PLANTS__ // Use all available plant shaders? Slower!
#define			__USE_EXTRA_PLANTS__ // Use extra available plant shaders? Slower!
//#define		__USE_SINGLE_PLANT__ // Use single plant shader/model only.. Just for testing fps difference.

//#define			__DISTANT_MEDIUM_PLANTLIFE__ // Draw medium and large plantlife further away?
#define			__DISTANT_MEDIUM_PLANTLIFE_RANGE__ 3500.0 //5000.0

//#define		__NO_GRASS_AT_PLANTS__ // Don't draw grass at the same position as a plant (for FPS)... Little impact..

#define			__USE_FOLIAGE_DENSITY__ // Turn on foliage density system...
#define			__FOLIAGE_DENSITY__ cg_foliageDensity.value //96.0//64.0//32.0//16.0
//
// END - FOLIAGE OPTIONS
//

#define		GRASS_SCALE_MULTIPLIER 0.6
#define		PLANT_SCALE_MULTIPLIER 1.0

#ifdef __USE_ALL_PLANTS__
#define		NUM_PLANT_SHADERS 81
#else //!__USE_ALL_PLANTS__
#define		NUM_PLANT_SHADERS 98//182

static const char *GoodPlantsList[] = {
"models/warzone/foliage/plant01.png",
"models/warzone/foliage/plant02.png",
"models/warzone/foliage/plant03.png",
"models/warzone/foliage/plant04.png",
"models/warzone/foliage/plant05.png",
"models/warzone/foliage/plant06.png",
"models/warzone/foliage/plant07.png",
"models/warzone/foliage/plant08.png",
"models/warzone/foliage/plant09.png",
"models/warzone/foliage/plant10.png",
"models/warzone/foliage/plant11.png",
"models/warzone/foliage/plant12.png",
"models/warzone/foliage/plant13.png",
"models/warzone/foliage/plant14.png",
"models/warzone/foliage/plant15.png",
"models/warzone/foliage/plant16.png",
"models/warzone/foliage/plant17.png",
"models/warzone/foliage/plant18.png",
"models/warzone/foliage/plant19.png",
"models/warzone/foliage/plant20.png",
"models/warzone/foliage/plant21.png",
"models/warzone/foliage/plant22.png",
"models/warzone/foliage/plant23.png",
"models/warzone/foliage/plant24.png",
"models/warzone/foliage/plant25.png",
"models/warzone/foliage/plant26.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant34.png",
"models/warzone/foliage/plant35.png",
"models/warzone/foliage/plant36.png",
"models/warzone/foliage/plant37.png",
"models/warzone/foliage/plant38.png",
"models/warzone/foliage/plant39.png",
"models/warzone/foliage/plant40.png",
"models/warzone/foliage/plant41.png",
"models/warzone/foliage/plant42.png",
"models/warzone/foliage/plant43.png",
"models/warzone/foliage/plant44.png",
"models/warzone/foliage/plant45.png",
"models/warzone/foliage/plant46.png",
"models/warzone/foliage/plant47.png",
"models/warzone/foliage/plant48.png",
"models/warzone/foliage/plant49.png",
"models/warzone/foliage/plant50.png",
"models/warzone/foliage/plant51.png",
"models/warzone/foliage/plant52.png",
"models/warzone/foliage/plant53.png",
"models/warzone/foliage/plant54.png",
"models/warzone/foliage/plant55.png",
"models/warzone/foliage/plant56.png",
"models/warzone/foliage/plant57.png",
"models/warzone/foliage/plant58.png",
"models/warzone/foliage/plant59.png",
"models/warzone/foliage/plant60.png",
"models/warzone/foliage/plant61.png",
"models/warzone/foliage/plant62.png",
"models/warzone/foliage/plant63.png",
"models/warzone/foliage/plant64.png",
"models/warzone/foliage/plant65.png",
"models/warzone/foliage/plant66.png",
"models/warzone/foliage/plant67.png",
"models/warzone/foliage/plant68.png",
"models/warzone/foliage/plant69.png",
"models/warzone/foliage/plant70.png",
// More of these because they add color
/*"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",
"models/warzone/foliage/plant27.png",
"models/warzone/foliage/plant28.png",
"models/warzone/foliage/plant29.png",
"models/warzone/foliage/plant30.png",
"models/warzone/foliage/plant31.png",
"models/warzone/foliage/plant32.png",
"models/warzone/foliage/plant33.png",*/

"models/warzone/foliage/plant60.png",
"models/warzone/foliage/plant61.png",
"models/warzone/foliage/plant62.png",
"models/warzone/foliage/plant63.png",
"models/warzone/foliage/plant67.png",
"models/warzone/foliage/plant69.png",
"models/warzone/foliage/plant70.png",
"models/warzone/foliage/plant60.png",
"models/warzone/foliage/plant61.png",
"models/warzone/foliage/plant62.png",
"models/warzone/foliage/plant63.png",
"models/warzone/foliage/plant67.png",
"models/warzone/foliage/plant69.png",
"models/warzone/foliage/plant70.png",
"models/warzone/foliage/plant60.png",
"models/warzone/foliage/plant61.png",
"models/warzone/foliage/plant62.png",
"models/warzone/foliage/plant63.png",
"models/warzone/foliage/plant67.png",
"models/warzone/foliage/plant69.png",
"models/warzone/foliage/plant70.png",
"models/warzone/foliage/plant60.png",
"models/warzone/foliage/plant61.png",
"models/warzone/foliage/plant62.png",
"models/warzone/foliage/plant63.png",
"models/warzone/foliage/plant67.png",
"models/warzone/foliage/plant69.png",
"models/warzone/foliage/plant70.png",
};

#endif //__USE_ALL_PLANTS__

	qboolean	FOLIAGE_LOADED = qfalse;
	int			FOLIAGE_NUM_POSITIONS = 0;
	vec3_t		FOLIAGE_POSITIONS[FOLIAGE_MAX_FOLIAGES];
#ifdef __PREGENERATE_NORMALS__
	vec3_t		FOLIAGE_NORMALS[FOLIAGE_MAX_FOLIAGES];
#endif //__PREGENERATE_NORMALS__
	int			FOLIAGE_GRASS_SELECTION[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_GRASS_ANGLES[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_GRASS_SCALE[FOLIAGE_MAX_FOLIAGES];
	int			FOLIAGE_PLANT_SELECTION[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_PLANT_ANGLES[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_PLANT_SCALE[FOLIAGE_MAX_FOLIAGES];
	int			FOLIAGE_TREE_SELECTION[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_TREE_ANGLES[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_TREE_SCALE[FOLIAGE_MAX_FOLIAGES];

	float		FOLIAGE_AREA_SIZE =				1024;
	float		FOLIAGE_VISIBLE_DISTANCE =		FOLIAGE_AREA_SIZE*2;
	float		FOLIAGE_TREE_VISIBLE_DISTANCE = FOLIAGE_AREA_SIZE*5;

	#define		FOLIAGE_AREA_MAX				65550
	#define		FOLIAGE_AREA_MAX_FOLIAGES		256

	int			FOLIAGE_AREAS_COUNT = 0;
	int			FOLIAGE_AREAS_LIST_COUNT[FOLIAGE_AREA_MAX];
	int			FOLIAGE_AREAS_LIST[FOLIAGE_AREA_MAX][FOLIAGE_AREA_MAX_FOLIAGES];
	vec3_t		FOLIAGE_AREAS_MINS[FOLIAGE_AREA_MAX];
	vec3_t		FOLIAGE_AREAS_MAXS[FOLIAGE_AREA_MAX];

	float OLD_FOLIAGE_DENSITY = 64.0;

	qboolean FOLIAGE_In_Bounds( int areaNum, int foliageNum )
	{
		if (foliageNum >= FOLIAGE_NUM_POSITIONS) return qfalse;

		if (FOLIAGE_AREAS_MINS[areaNum][0] < FOLIAGE_POSITIONS[foliageNum][0]
			&& FOLIAGE_AREAS_MINS[areaNum][1] < FOLIAGE_POSITIONS[foliageNum][1]
			&& FOLIAGE_AREAS_MAXS[areaNum][0] >= FOLIAGE_POSITIONS[foliageNum][0]
			&& FOLIAGE_AREAS_MAXS[areaNum][1] >= FOLIAGE_POSITIONS[foliageNum][1])
		{
			return qtrue;
		}

		return qfalse;
	}

	void FOLIAGE_DEBUG_Check_Foliage_In_Areas( void )
	{
#ifdef __FOLIAGE_AREA_DEBUGGING__
		for (int j = 0; j < FOLIAGE_NUM_POSITIONS; j++)
		{
			qboolean found = qfalse;

			for (int i = 0; i < FOLIAGE_AREAS_COUNT; i++)
			{
				if (FOLIAGE_AREAS_LIST[i][FOLIAGE_AREAS_LIST_COUNT[i]] == j)
				{
					found = qtrue;
					break;
				}
			}

			if (!found) trap->Print("Foliage %i (at %f %f %f) is not in an area.\n", j, FOLIAGE_POSITIONS[j][0], FOLIAGE_POSITIONS[j][1], FOLIAGE_POSITIONS[j][2]);
		}
#endif //__FOLIAGE_AREA_DEBUGGING__
	}

	void FOLIAGE_Setup_Foliage_Areas( void )
	{
		int		areaNum = 0, i = 0;
		vec3_t	mins, maxs, mapMins, mapMaxs;

		VectorSet(mapMins, 128000, 128000, 0);
		VectorSet(mapMaxs, -128000, -128000, 0);

		//trap->Print("Generating foliage areas.\n");

		// Find map bounds first... Reduce area numbers...
		for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
		{
			if (FOLIAGE_POSITIONS[i][0] < mapMins[0])
				mapMins[0] = FOLIAGE_POSITIONS[i][0];

			if (FOLIAGE_POSITIONS[i][0] > mapMaxs[0])
				mapMaxs[0] = FOLIAGE_POSITIONS[i][0];

			if (FOLIAGE_POSITIONS[i][1] < mapMins[1])
				mapMins[1] = FOLIAGE_POSITIONS[i][1];

			if (FOLIAGE_POSITIONS[i][1] > mapMaxs[1])
				mapMaxs[1] = FOLIAGE_POSITIONS[i][1];
		}

		mapMins[0] -= 1024.0;
		mapMins[1] -= 1024.0;
		mapMaxs[0] += 1024.0;
		mapMaxs[1] += 1024.0;

		VectorSet(mins, mapMins[0], mapMins[1], 0);
		VectorSet(maxs, mapMins[0] + FOLIAGE_AREA_SIZE, mapMins[1] + FOLIAGE_AREA_SIZE, 0);

		int DENSITY_REMOVED = 0;
		FOLIAGE_AREAS_COUNT = 0;

		for (areaNum = 0; areaNum < FOLIAGE_AREA_MAX; areaNum++)
		{
			if (mins[1] > mapMaxs[1]) break; // found our last area...

			FOLIAGE_AREAS_LIST_COUNT[areaNum] = 0;

			while (FOLIAGE_AREAS_LIST_COUNT[areaNum] == 0 && mins[1] <= mapMaxs[1])
			{// While loop is so we can skip zero size areas for speed...
				VectorCopy(mins, FOLIAGE_AREAS_MINS[areaNum]);
				VectorCopy(maxs, FOLIAGE_AREAS_MAXS[areaNum]);

				// Assign foliages to the area lists...
				for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
				{
					if (FOLIAGE_In_Bounds(areaNum, i))
					{
#ifdef __USE_FOLIAGE_DENSITY__
						qboolean OVER_DENSITY = qfalse;

						if (FOLIAGE_AREAS_LIST_COUNT[areaNum] > FOLIAGE_AREA_MAX_FOLIAGES)
						{
							//trap->Print("*** Area %i has more then %i foliages ***\n", areaNum, (int)FOLIAGE_AREA_MAX_FOLIAGES);
							break;
						}

						for (int j = 0; j < FOLIAGE_AREAS_LIST_COUNT[areaNum]; j++)
						{// Let's use a density setting to improve FPS...
							if (DistanceHorizontal(FOLIAGE_POSITIONS[i], FOLIAGE_POSITIONS[FOLIAGE_AREAS_LIST[areaNum][j]]) < __FOLIAGE_DENSITY__)
							{// Adding this would go over density setting...
								OVER_DENSITY = qtrue;
								DENSITY_REMOVED++;

								if (FOLIAGE_TREE_SELECTION[i] > 0 && FOLIAGE_TREE_SELECTION[FOLIAGE_AREAS_LIST[areaNum][j]] <= 0)
								{// The new one is a tree. Use it instead...
									FOLIAGE_AREAS_LIST[areaNum][j] = i;
								}

								break;
							}
						}

						if (!OVER_DENSITY)
						{
							FOLIAGE_AREAS_LIST[areaNum][FOLIAGE_AREAS_LIST_COUNT[areaNum]] = i;
							FOLIAGE_AREAS_LIST_COUNT[areaNum]++;
						}

#else //!__USE_FOLIAGE_DENSITY__
						FOLIAGE_AREAS_LIST[areaNum][FOLIAGE_AREAS_LIST_COUNT[areaNum]] = i;
						FOLIAGE_AREAS_LIST_COUNT[areaNum]++;
#endif //__USE_FOLIAGE_DENSITY__
					}
				}

				//if (FOLIAGE_AREAS_LIST_COUNT[areaNum] > 0)
				//	trap->Print("Foliage area %i is between %f %f and %f %f. %i foliages in area.\n", areaNum, mins[0], mins[1], maxs[0], maxs[1], FOLIAGE_AREAS_LIST_COUNT[areaNum]);

				mins[0] += FOLIAGE_AREA_SIZE;
				maxs[0] = mins[0] + FOLIAGE_AREA_SIZE;

				if (mins[0] > mapMaxs[0])
				{
					mins[0] = mapMins[0];
					maxs[0] = mapMins[0] + FOLIAGE_AREA_SIZE;

					mins[1] += FOLIAGE_AREA_SIZE;
					maxs[1] = mins[1] + FOLIAGE_AREA_SIZE;
				}
			}
		}

		FOLIAGE_AREAS_COUNT = areaNum;
		OLD_FOLIAGE_DENSITY = __FOLIAGE_DENSITY__;

		FOLIAGE_DEBUG_Check_Foliage_In_Areas();

		trap->Print("Generated %i foliage areas. %i total foliages. %i removed by density setting.\n", FOLIAGE_AREAS_COUNT, FOLIAGE_NUM_POSITIONS, DENSITY_REMOVED);
	}

	void FOLIAGE_Check_CVar_Change ( void )
	{
		if (__FOLIAGE_DENSITY__ != OLD_FOLIAGE_DENSITY)
		{
			FOLIAGE_Setup_Foliage_Areas();
		}
	}

	qhandle_t	FOLIAGE_GRASS_MODEL[3] = { 0 };
	int			FOLIAGE_GRASS_SHADERNUM[FOLIAGE_MAX_FOLIAGES] = { 0 };
	qhandle_t	FOLIAGE_GRASS_BILLBOARD_SHADER[5] = { 0 };
	qhandle_t	FOLIAGE_GRASS_BILLBOARD_MODEL[3] = { 0 };
	qhandle_t	FOLIAGE_PLANT_MODEL[27] = { 0 };
	int			FOLIAGE_PLANT_SHADERNUM[FOLIAGE_MAX_FOLIAGES] = { 0 };
	qhandle_t	FOLIAGE_PLANT_BILLBOARD_MODEL[27] = { 0 };
	qhandle_t	FOLIAGE_TREE_MODEL[3] = { 0 };
	
	qhandle_t	FOLIAGE_GRASS_SHADERS[37] = {0};
	qhandle_t	FOLIAGE_PLANT_SHADERS[NUM_PLANT_SHADERS] = {0};

	extern qboolean InFOV( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV );
}

int IN_RANGE_AREAS_LIST_COUNT = 0;
int IN_RANGE_AREAS_LIST[1024];
int IN_RANGE_TREE_AREAS_LIST_COUNT = 0;
int IN_RANGE_TREE_AREAS_LIST[8192];

qboolean FOLIAGE_Box_In_FOV ( vec3_t mins, vec3_t maxs )
{
	vec3_t edge, edge2;

	VectorSet(edge, maxs[0], mins[1], maxs[2]);
	VectorSet(edge2, mins[0], maxs[1], maxs[2]);
	
	if (!InFOV( mins, cg.refdef.vieworg, cg.refdef.viewangles, 100, 140 )
		&& !InFOV( maxs, cg.refdef.vieworg, cg.refdef.viewangles, 100, 140 )
		&& !InFOV( edge, cg.refdef.vieworg, cg.refdef.viewangles, 100, 140 )
		&& !InFOV( edge2, cg.refdef.vieworg, cg.refdef.viewangles, 100, 140 ))
		return qfalse;

	return qtrue;
}

vec3_t		LAST_ORG = { 0 };
vec3_t		LAST_ANG = { 0 };

void FOLIAGE_Calc_In_Range_Areas( void )
{
	if (Distance(cg.refdef.vieworg, LAST_ORG) > 128.0 || Distance(cg.refdef.viewangles, LAST_ANG) > 30.0)
	{// Update in range list...
		VectorCopy(cg.refdef.vieworg, LAST_ORG);
		VectorCopy(cg.refdef.viewangles, LAST_ANG);

		IN_RANGE_AREAS_LIST_COUNT = 0;
		IN_RANGE_TREE_AREAS_LIST_COUNT = 0;

		// Calculate currently-in-range areas to use...
		for (int i = 0; i < FOLIAGE_AREAS_COUNT; i++)
		{
			if (DistanceHorizontal(FOLIAGE_AREAS_MINS[i], cg.refdef.vieworg) < FOLIAGE_VISIBLE_DISTANCE 
				|| DistanceHorizontal(FOLIAGE_AREAS_MAXS[i], cg.refdef.vieworg) < FOLIAGE_VISIBLE_DISTANCE)
			{
				if (FOLIAGE_Box_In_FOV( FOLIAGE_AREAS_MINS[i], FOLIAGE_AREAS_MAXS[i] ))
				{
					IN_RANGE_AREAS_LIST[IN_RANGE_AREAS_LIST_COUNT] = i;
					IN_RANGE_AREAS_LIST_COUNT++;
				}
			}
			else if (DistanceHorizontal(FOLIAGE_AREAS_MINS[i], cg.refdef.vieworg) < FOLIAGE_TREE_VISIBLE_DISTANCE
				|| DistanceHorizontal(FOLIAGE_AREAS_MAXS[i], cg.refdef.vieworg) < FOLIAGE_TREE_VISIBLE_DISTANCE)
			{
				if (FOLIAGE_Box_In_FOV( FOLIAGE_AREAS_MINS[i], FOLIAGE_AREAS_MAXS[i] ))
				{
					IN_RANGE_TREE_AREAS_LIST[IN_RANGE_TREE_AREAS_LIST_COUNT] = i;
					IN_RANGE_TREE_AREAS_LIST_COUNT++;
				}
			}
		}

		//trap->Print("There are %i foliage areas in range. %i tree areas.\n", IN_RANGE_AREAS_LIST_COUNT, IN_RANGE_TREE_AREAS_LIST_COUNT);
	}
}

void FOLIAGE_StartUpdateThread ( void )
{
}

void FOLIAGE_ShutdownUpdateThread ( void )
{
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
		float			distFadeScale = 1.0;

#ifndef __PREGENERATE_NORMALS__
		trace_t tr;
		vec3_t	up, down, normal;
		VectorCopy(FOLIAGE_POSITIONS[num], up);
		up[2]+=128;
		VectorCopy(FOLIAGE_POSITIONS[num], down);
		down[2]-=128;
		CG_Trace(&tr, up, NULL, NULL, down, -1, MASK_SOLID);
		VectorCopy(tr.plane.normal, normal);
#endif //__PREGENERATE_NORMALS__

		if (dist > 1024.0)//512.0)
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

#ifdef __DISTANT_MEDIUM_PLANTLIFE__
		if ((!treeOnly || (dist < __DISTANT_MEDIUM_PLANTLIFE_RANGE__ && FOLIAGE_GRASS_SCALE[num] >= 1.0)))
#else
		if (!treeOnly)
#endif //__DISTANT_MEDIUM_PLANTLIFE__
		{// Graw grass...
#ifdef __NO_GRASS_AT_PLANTS__
			if (FOLIAGE_PLANT_SELECTION[num] == 0)
			{
#endif //__NO_GRASS_AT_PLANTS__
				float GRASS_SCALE = FOLIAGE_GRASS_SCALE[num] * GRASS_SCALE_MULTIPLIER * 1.5 * distFadeScale;

				re.reType = RT_MODEL;

				re.origin[2] -= 16.0;

				if (dist < 128)
				{
					re.customShader = FOLIAGE_GRASS_BILLBOARD_SHADER[0];
					re.hModel = FOLIAGE_GRASS_MODEL[0];
				}
				else if (dist < 256)
				{
					re.customShader = FOLIAGE_GRASS_BILLBOARD_SHADER[1];
					re.hModel = FOLIAGE_GRASS_MODEL[0];
				}
				else if (dist < 384)
				{
					re.customShader = FOLIAGE_GRASS_BILLBOARD_SHADER[2];
					re.hModel = FOLIAGE_GRASS_MODEL[0];
				}
				else if (dist < 512)
				{
					re.customShader = FOLIAGE_GRASS_BILLBOARD_SHADER[2];
					re.hModel = FOLIAGE_GRASS_MODEL[0];
				}
				else if (dist < 1024)
				{
					re.customShader = FOLIAGE_GRASS_BILLBOARD_SHADER[3];
					re.hModel = FOLIAGE_GRASS_MODEL[1];
				}
				else
				{
					re.customShader = FOLIAGE_GRASS_BILLBOARD_SHADER[4];
					re.hModel = FOLIAGE_GRASS_MODEL[2];
				}

				VectorSet(re.modelScale, GRASS_SCALE, GRASS_SCALE, GRASS_SCALE);

#ifdef __PREGENERATE_NORMALS__
				vectoangles( FOLIAGE_NORMALS[num], angles );
#else //!__PREGENERATE_NORMALS__
				vectoangles( normal, angles );
#endif //__PREGENERATE_NORMALS__
				angles[PITCH] += 90;
				//angles[YAW] += FOLIAGE_GRASS_ANGLES[num];

				VectorCopy(angles, re.angles);
				AnglesToAxis(angles, re.axis);
				ScaleModelAxis( &re );

				FOLIAGE_AddFoliageEntityToScene( &re );
#ifdef __NO_GRASS_AT_PLANTS__
			}
#endif //__NO_GRASS_AT_PLANTS__
		}

		VectorCopy(FOLIAGE_POSITIONS[num], re.origin);
		re.customShader = 0;
		re.renderfx = 0;

		if (FOLIAGE_TREE_SELECTION[num] > 0)
		{// Add the tree model...
			re.reType = RT_MODEL;
			re.hModel = FOLIAGE_TREE_MODEL[FOLIAGE_TREE_SELECTION[num]-1];
			
			//if (FOLIAGE_TREE_SELECTION[num]-1 > 1)
			//	re.hModel = FOLIAGE_TREE_MODEL[2];
			//else
			//	re.hModel = FOLIAGE_TREE_MODEL[0];

			//if (FOLIAGE_TREE_SELECTION[num]-1 == 0)
				VectorSet(re.modelScale, FOLIAGE_TREE_SCALE[num]*2.5, FOLIAGE_TREE_SCALE[num]*2.5, FOLIAGE_TREE_SCALE[num]*2.5);
			//else
			//	VectorSet(re.modelScale, FOLIAGE_TREE_SCALE[num], FOLIAGE_TREE_SCALE[num], FOLIAGE_TREE_SCALE[num]);

			angles[PITCH] = angles[ROLL] = 0.0f;
			angles[YAW] = FOLIAGE_TREE_ANGLES[num];
			VectorCopy(angles, re.angles);
			AnglesToAxis(angles, re.axis);
			//if (FOLIAGE_TREE_SELECTION[num]-1 > 1) re.origin[2] += 128.0; // the tree model digs into ground too much...
			ScaleModelAxis( &re );

			FOLIAGE_AddFoliageEntityToScene( &re );
		}
#ifndef __NO_PLANTS__
		else if (FOLIAGE_PLANT_SELECTION[num] != 0 
#ifdef __DISTANT_MEDIUM_PLANTLIFE__
			&& (!treeOnly || (dist < __DISTANT_MEDIUM_PLANTLIFE_RANGE__ && FOLIAGE_PLANT_SCALE[num] >= 1.0))
#else //!__DISTANT_MEDIUM_PLANTLIFE__
			&& !treeOnly
#endif //__DISTANT_MEDIUM_PLANTLIFE__
			)
		{// Add plant model as well...

#if defined(__USE_SINGLE_PLANT__)
			re.customShader = FOLIAGE_PLANT_SHADERS[FOLIAGE_PLANT_SHADERNUM[1]];
#elif defined(__USE_ALL_PLANTS__) || defined(__USE_EXTRA_PLANTS__)

			if (FOLIAGE_PLANT_SHADERNUM[num] > 0)
			{// Need to specify a shader...
				re.customShader = FOLIAGE_PLANT_SHADERS[FOLIAGE_PLANT_SHADERNUM[num]];
			}

			/*
			// Origin based test...
			float v0 = re.origin[0];
			float v1 = re.origin[1];
			float v2 = re.origin[2];

			if (v0 < 0.0) v0 *= -1.0;
			if (v1 < 0.0) v1 *= -1.0;
			if (v2 < 0.0) v2 *= -1.0;

			float t0 = sqrt(sqrt(v0+v1));
			float t1 = sqrt(sqrt(v1+v2));
			float t2 = sqrt(sqrt(v2+v0));

			int		type = int(t0 + t1 + t2);

			re.customShader = FOLIAGE_PLANT_SHADERS[type];
			*/

#endif //defined(__USE_ALL_PLANTS__) || defined(__USE_EXTRA_PLANTS__)

			float PLANT_SCALE = FOLIAGE_PLANT_SCALE[num]*PLANT_SCALE_MULTIPLIER*distFadeScale;

			re.reType = RT_MODEL;

			if (dist < 512)
			{
				re.hModel = FOLIAGE_GRASS_MODEL[0];
			}
			else if (dist < 1024)
			{
				re.hModel = FOLIAGE_GRASS_MODEL[1];
			}
			else
			{
				re.hModel = FOLIAGE_GRASS_MODEL[2];
			}

			VectorSet(re.modelScale, PLANT_SCALE, PLANT_SCALE, PLANT_SCALE);

#ifdef __PREGENERATE_NORMALS__
			vectoangles( FOLIAGE_NORMALS[num], angles );
#else //!__PREGENERATE_NORMALS__
			vectoangles( normal, angles );
#endif //__PREGENERATE_NORMALS__
			angles[PITCH] += 90;
			//angles[YAW] += FOLIAGE_PLANT_ANGLES[num];

			VectorCopy(angles, re.angles);
			AnglesToAxis(angles, re.axis);
			ScaleModelAxis( &re );

			FOLIAGE_AddFoliageEntityToScene( &re );
		}
#endif //__NO_PLANTS__
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
			FOLIAGE_GRASS_SHADERNUM[i] = irand(1,36);
			FOLIAGE_PLANT_SHADERNUM[i] = irand(1,NUM_PLANT_SHADERS-1);

			if (FOLIAGE_TREE_SELECTION[i] > 0) FOLIAGE_TREE_SELECTION[i] = irand(1,3);

#ifdef __PREGENERATE_NORMALS__
			trace_t tr;
			vec3_t	up, down;
			VectorCopy(FOLIAGE_POSITIONS[i], up);
			up[2]+=128;
			VectorCopy(FOLIAGE_POSITIONS[i], down);
			down[2]-=128;
			CG_Trace(&tr, up, NULL, NULL, down, -1, MASK_SOLID);
			VectorCopy(tr.plane.normal, FOLIAGE_NORMALS[i]);
#endif //__PREGENERATE_NORMALS__
		}
		
		/*if (FOLIAGE_NUM_POSITIONS > 100000)
		{
			FOLIAGE_AREA_SIZE =				1024.0;
			FOLIAGE_VISIBLE_DISTANCE =		FOLIAGE_AREA_SIZE;
			FOLIAGE_TREE_VISIBLE_DISTANCE = FOLIAGE_AREA_SIZE*3;
		}
		else if (FOLIAGE_NUM_POSITIONS > 20000)
		{
			FOLIAGE_AREA_SIZE =				512.0;
			FOLIAGE_VISIBLE_DISTANCE =		FOLIAGE_AREA_SIZE*3;
			FOLIAGE_TREE_VISIBLE_DISTANCE = FOLIAGE_AREA_SIZE*7;
		}
		else if (FOLIAGE_NUM_POSITIONS > 10000)
		{
			FOLIAGE_AREA_SIZE =				768.0;
			FOLIAGE_VISIBLE_DISTANCE =		FOLIAGE_AREA_SIZE*2;
			FOLIAGE_TREE_VISIBLE_DISTANCE = FOLIAGE_AREA_SIZE*5;
		}
		else
		{
			FOLIAGE_AREA_SIZE =				1024.0;
			FOLIAGE_VISIBLE_DISTANCE =		FOLIAGE_AREA_SIZE*2;
			FOLIAGE_TREE_VISIBLE_DISTANCE = FOLIAGE_AREA_SIZE*5;
		}*/

		trap->Print( "^1*** ^3%s^5: Successfully loaded %i foliage points from foliage file ^7foliage/%s.foliage^5.\n", GAME_VERSION,
			FOLIAGE_NUM_POSITIONS, cgs.currentmapname );

		FOLIAGE_Setup_Foliage_Areas();

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

		FOLIAGE_Setup_Foliage_Areas();

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
			FOLIAGE_GRASS_MODEL[0] = trap->R_RegisterModel( "models/warzone/foliage/grass01_LOD0.md3" );
			FOLIAGE_GRASS_MODEL[1] = trap->R_RegisterModel( "models/warzone/foliage/grass01_LOD1.md3" );
			FOLIAGE_GRASS_MODEL[2] = trap->R_RegisterModel( "models/warzone/foliage/grass01_LOD2.md3" );

			FOLIAGE_GRASS_BILLBOARD_MODEL[0] = trap->R_RegisterModel( "models/warzone/foliage/grass_cross.md3" );
			FOLIAGE_GRASS_BILLBOARD_SHADER[0] = trap->R_RegisterShader( "models/warzone/foliage/maingrass1024.png" );
			FOLIAGE_GRASS_BILLBOARD_SHADER[1] = trap->R_RegisterShader( "models/warzone/foliage/maingrass512.png" );
			FOLIAGE_GRASS_BILLBOARD_SHADER[2] = trap->R_RegisterShader( "models/warzone/foliage/maingrass256.png" );
			FOLIAGE_GRASS_BILLBOARD_SHADER[3] = trap->R_RegisterShader( "models/warzone/foliage/maingrass128.png" );
			FOLIAGE_GRASS_BILLBOARD_SHADER[4] = trap->R_RegisterShader( "models/warzone/foliage/maingrass64.png" );

			//int plantModel1 = trap->R_RegisterModel("models/warzone/foliage/plant03.md3");
			int plantModel2 = FOLIAGE_GRASS_MODEL[0];//trap->R_RegisterModel( "models/warzone/foliage/grass_dense.md3" );
			int plantModel3 = FOLIAGE_GRASS_MODEL[0];//trap->R_RegisterModel( "models/warzone/foliage/grass_cross.md3" );

			for (int i = 0; i < 27; i++)
			{
				//if (i <= 9)
				//	FOLIAGE_PLANT_MODEL[i] = plantModel3;
				//else if (i <= 18)
					FOLIAGE_PLANT_MODEL[i] = plantModel2;
				//else
				//	FOLIAGE_PLANT_MODEL[i] = plantModel1;

				FOLIAGE_PLANT_BILLBOARD_MODEL[i] = plantModel3;
			}

			//FOLIAGE_TREE_MODEL[0] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );
			//FOLIAGE_TREE_MODEL[1] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );
			//FOLIAGE_TREE_MODEL[2] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );

			FOLIAGE_TREE_MODEL[0] = trap->R_RegisterModel( "models/warzone/trees/fanpalm1.md3" );
			//FOLIAGE_TREE_MODEL[1] = FOLIAGE_TREE_MODEL[0];
			FOLIAGE_TREE_MODEL[1] = trap->R_RegisterModel( "models/warzone/trees/giant1.md3" );
			//FOLIAGE_TREE_MODEL[2] = FOLIAGE_TREE_MODEL[0];
			//FOLIAGE_TREE_MODEL[1] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );
			//FOLIAGE_TREE_MODEL[2] = trap->R_RegisterModel( "models/map_objects/yavin/tree08_b.md3" );
			FOLIAGE_TREE_MODEL[2] = trap->R_RegisterModel( "models/warzone/trees/anvilpalm1.md3" );

			for (int i = 1; i < 37; i++)
			{
				char	shaderName[128] = {0};
				int		shaderNum = i;

				if (shaderNum < 10)
					sprintf(shaderName, "models/warzone/foliage/grass0%i.png", shaderNum);
				else
					sprintf(shaderName, "models/warzone/foliage/grass%i.png", shaderNum);

				FOLIAGE_GRASS_SHADERS[i] = trap->R_RegisterShader(shaderName);
			}

#ifndef __USE_ALL_PLANTS__
			int current = 0;

			for (int i = 1; i < NUM_PLANT_SHADERS; i++)
			{
				FOLIAGE_PLANT_SHADERS[i] = trap->R_RegisterShader(GoodPlantsList[current]);

				// Loop the best list...
				current++;

				//if (current >= 29)
				//	current = 0; // Return to start...
			}
#else //!__USE_ALL_PLANTS__
			for (int i = 1; i < NUM_PLANT_SHADERS; i++)
			{
				char	shaderName[128] = {0};
				int		shaderNum = i;

				if (shaderNum < 10)
					sprintf(shaderName, "models/warzone/foliage/plant0%i.png", shaderNum);
				else
					sprintf(shaderName, "models/warzone/foliage/plant%i.png", shaderNum);

				FOLIAGE_PLANT_SHADERS[i] = trap->R_RegisterShader(shaderName);
			}
#endif //__USE_ALL_PLANTS__
		}

		FOLIAGE_Check_CVar_Change();

		FOLIAGE_CheckUpdateThread();

		vec3_t viewOrg, viewAngles;

		VectorCopy(cg.refdef.vieworg, viewOrg);
		VectorCopy(cg.refdef.viewangles, viewAngles);

		FOLIAGE_Calc_In_Range_Areas();

		for (int CURRENT_AREA = 0; CURRENT_AREA < IN_RANGE_AREAS_LIST_COUNT; CURRENT_AREA++)
		{
			int CURRENT_AREA_ID = IN_RANGE_AREAS_LIST[CURRENT_AREA];

			for (int spot = 0; spot < FOLIAGE_AREAS_LIST_COUNT[CURRENT_AREA_ID]; spot++)
			{
				
				// Check FOV of this foliage...
				if (DistanceHorizontal(FOLIAGE_POSITIONS[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]], viewOrg) > 256)
				{
					if (FOLIAGE_TREE_SELECTION[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]] > 0)
					{
						if (!InFOV( FOLIAGE_POSITIONS[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]], viewOrg, viewAngles, cg.refdef.fov_x + 20, cg.refdef.fov_y + 5 )) 
							continue;
					}
					else
					{
						if (!InFOV( FOLIAGE_POSITIONS[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]], viewOrg, viewAngles, cg.refdef.fov_x + 3, cg.refdef.fov_y + 3 )) 
							continue;
					}
				}
				

				FOLIAGE_AddToScreen( FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot], qfalse );
			}
		}

		for (int CURRENT_AREA = 0; CURRENT_AREA < IN_RANGE_TREE_AREAS_LIST_COUNT; CURRENT_AREA++)
		{
			int CURRENT_AREA_ID = IN_RANGE_TREE_AREAS_LIST[CURRENT_AREA];

			for (int spot = 0; spot < FOLIAGE_AREAS_LIST_COUNT[CURRENT_AREA_ID]; spot++)
			{
				//if (FOLIAGE_TREE_SELECTION[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]] <= 0 && FOLIAGE_PLANT_SELECTION[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]] <= 0)
				//	continue; // No point if there's no plant or tree here...

				
				// Check FOV of this foliage...
				if (DistanceHorizontal(FOLIAGE_POSITIONS[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]], viewOrg) > 512)
				{
					if (FOLIAGE_TREE_SELECTION[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]] > 0)
					{
						if (!InFOV( FOLIAGE_POSITIONS[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]], viewOrg, viewAngles, cg.refdef.fov_x + 20, cg.refdef.fov_y + 5 )) 
							continue;
					}
					else
					{
						if (!InFOV( FOLIAGE_POSITIONS[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]], viewOrg, viewAngles, cg.refdef.fov_x + 3, cg.refdef.fov_y + 3 )) 
							continue;
					}
				}
				

				FOLIAGE_AddToScreen( FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot], qtrue );
			}
		}
	}

	// =======================================================================================================================================
	//
	//                                                             Foliage Generation...
	//
	// =======================================================================================================================================

	extern void AIMod_GetMapBounts ( void );
	extern float RoofHeightAt ( vec3_t org );

	extern float aw_percent_complete;
	extern char task_string1[255];
	extern char task_string2[255];
	extern char task_string3[255];
	extern char last_node_added_string[255];
	extern clock_t	aw_stage_start_time;

	qboolean MaterialIsValidForGrass(int materialType)
	{
		switch( materialType )
		{
		case MATERIAL_SHORTGRASS:		// 5					// manicured lawn
		case MATERIAL_LONGGRASS:		// 6					// long jungle grass
		case MATERIAL_MUD:				// 17					// wet soil
		case MATERIAL_DIRT:				// 7					// hard mud
			return qtrue;
			break;
		default:
			break;
		}

		return qfalse;
	}

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

		if (MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK)))
			return qtrue;

		return qfalse;
	}


#define GRASS_MODEL_WIDTH cg_foliageModelWidth.value
#define GRASS_SLOPE_MAX_DIFF cg_foliageMaxSlopeChange.value
#define GRASS_HEIGHT_MAX_DIFF cg_foliageMaxHeightChange.value
#define GRASS_SLOPE_UP_HEIGHT cg_foliageSlopeCheckHeight.value

	qboolean FOLIAGE_CheckSlopesAround(vec3_t pos, vec3_t down, vec3_t groundpos, vec3_t slope, float scale)
	{
		trace_t		tr;
		vec3_t		pos2, down2;
		float		HEIGHT_SCALE = 1.0;

		if (scale <= 0.3)
			HEIGHT_SCALE = 0.5;
		else if (scale <= 0.5)
			HEIGHT_SCALE = 0.7;
		else if (scale <= 0.7)
			HEIGHT_SCALE = 0.9;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] += (GRASS_MODEL_WIDTH * scale);
		down2[0] += (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (Distance(slope, tr.plane.normal) > GRASS_SLOPE_MAX_DIFF) return qfalse;
		if (DistanceVertical(groundpos, tr.endpos) > GRASS_HEIGHT_MAX_DIFF * HEIGHT_SCALE /*&& Distance(slope, tr.plane.normal) > 0.0*/) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] -= (GRASS_MODEL_WIDTH * scale);
		down2[0] -= (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (Distance(slope, tr.plane.normal) > GRASS_SLOPE_MAX_DIFF) return qfalse;
		if (DistanceVertical(groundpos, tr.endpos) > GRASS_HEIGHT_MAX_DIFF * HEIGHT_SCALE /*&& Distance(slope, tr.plane.normal) > 0.0*/) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[1] += (GRASS_MODEL_WIDTH * scale);
		down2[1] += (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (Distance(slope, tr.plane.normal) > GRASS_SLOPE_MAX_DIFF) return qfalse;
		if (DistanceVertical(groundpos, tr.endpos) > GRASS_HEIGHT_MAX_DIFF * HEIGHT_SCALE /*&& Distance(slope, tr.plane.normal) > 0.0*/) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[1] -= (GRASS_MODEL_WIDTH * scale);
		down2[1] -= (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (Distance(slope, tr.plane.normal) > GRASS_SLOPE_MAX_DIFF) return qfalse;
		if (DistanceVertical(groundpos, tr.endpos) > GRASS_HEIGHT_MAX_DIFF * HEIGHT_SCALE /*&& Distance(slope, tr.plane.normal) > 0.0*/) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] += (GRASS_MODEL_WIDTH * scale);
		pos2[1] += (GRASS_MODEL_WIDTH * scale);
		down2[0] += (GRASS_MODEL_WIDTH * scale);
		down2[1] += (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (Distance(slope, tr.plane.normal) > GRASS_SLOPE_MAX_DIFF) return qfalse;
		if (DistanceVertical(groundpos, tr.endpos) > GRASS_HEIGHT_MAX_DIFF * HEIGHT_SCALE /*&& Distance(slope, tr.plane.normal) > 0.0*/) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] += (GRASS_MODEL_WIDTH * scale);
		pos2[1] -= (GRASS_MODEL_WIDTH * scale);
		down2[0] += (GRASS_MODEL_WIDTH * scale);
		down2[1] -= (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (Distance(slope, tr.plane.normal) > GRASS_SLOPE_MAX_DIFF) return qfalse;
		if (DistanceVertical(groundpos, tr.endpos) > GRASS_HEIGHT_MAX_DIFF * HEIGHT_SCALE /*&& Distance(slope, tr.plane.normal) > 0.0*/) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] -= (GRASS_MODEL_WIDTH * scale);
		pos2[1] += (GRASS_MODEL_WIDTH * scale);
		down2[0] -= (GRASS_MODEL_WIDTH * scale);
		down2[1] += (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (Distance(slope, tr.plane.normal) > GRASS_SLOPE_MAX_DIFF) return qfalse;
		if (DistanceVertical(groundpos, tr.endpos) > GRASS_HEIGHT_MAX_DIFF * HEIGHT_SCALE /*&& Distance(slope, tr.plane.normal) > 0.0*/) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] -= (GRASS_MODEL_WIDTH * scale);
		pos2[1] -= (GRASS_MODEL_WIDTH * scale);
		down2[0] -= (GRASS_MODEL_WIDTH * scale);
		down2[1] -= (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (Distance(slope, tr.plane.normal) > GRASS_SLOPE_MAX_DIFF) return qfalse;
		if (DistanceVertical(groundpos, tr.endpos) > GRASS_HEIGHT_MAX_DIFF * HEIGHT_SCALE /*&& Distance(slope, tr.plane.normal) > 0.0*/) return qfalse;

		return qtrue;
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

					if (FOLIAGE_NUM_POSITIONS >= 100000)
					{
						if (random() * 7 >= 6 && RoofHeightAt(vec) - vec[2] > 1024.0)
						{// Add tree... 1 in every 8 positions... If there is room above for a tree...
							FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
							FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
						}
						else if (random() * 100 >= 50)
						{// Add plant... 1 in every 2 positions...
							FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
							FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);

							if (FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] == 0)
								FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
						}
					}
					else if (FOLIAGE_NUM_POSITIONS >= 20000)
					{
						if (random() * 10 >= 9 && RoofHeightAt(vec) - vec[2] > 1024.0)
						{// Add tree... 1 in every 10 positions... If there is room above for a tree...
							FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
							FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
						}
						else if (random() * 100 >= 50)
						{// Add plant... 1 in every 2 positions...
							FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
							FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);

							if (FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] == 0)
								FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
						}
					}
					else if (FOLIAGE_NUM_POSITIONS >= 10000)
					{
						if (random() * 12 >= 11 && RoofHeightAt(vec) - vec[2] > 1024.0)
						{// Add tree... 1 in every 12 positions... If there is room above for a tree...
							FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
							FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
						}
						else if (random() * 100 >= 50)
						{// Add plant... 1 in every 2 positions...
							FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
							FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);

							if (FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] == 0)
								FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
						}
					}
					else
					{
						if (random() * 18 >= 17 && RoofHeightAt(vec) - vec[2] > 1024.0)
						{// Add tree... 1 in every 17 positions... If there is room above for a tree...
							FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
							FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
						}
						else if (random() * 100 >= 66)
						{// Add plant... 1 in every 1.33 positions...
							FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
							FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);

							if (FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] == 0)
								FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
						}
					}

#ifdef __PREGENERATE_NORMALS__
					trace_t tr;
					vec3_t	up, down;
					VectorCopy(FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS], up);
					up[2]+=128;
					VectorCopy(FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS], down);
					down[2]-=128;
					CG_Trace(&tr, up, NULL, NULL, down, -1, MASK_SOLID);
					VectorCopy(tr.plane.normal, FOLIAGE_NORMALS[FOLIAGE_NUM_POSITIONS]);
#endif //__PREGENERATE_NORMALS__

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
				FOLIAGE_GRASS_SHADERNUM[i] = irand(1,36);
				FOLIAGE_PLANT_SHADERNUM[i] = irand(1,NUM_PLANT_SHADERS-1);
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
			float		*grassSpotScale;
			float		map_size, temp;
			vec3_t		mapMins, mapMaxs;
			int			start_time = trap->Milliseconds();
			int			update_timer = 0;
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
			grassSpotScale = (float *)malloc((sizeof(float)+1)*FOLIAGE_MAX_FOLIAGES);

			memset(grassSpotScale, 0, (sizeof(float)+1)*FOLIAGE_MAX_FOLIAGES);

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

			previous_time = clock();
			aw_stage_start_time = clock();
			aw_percent_complete = 0;

			grassSpotCount = 0;

			// Create the map...
			vec3_t MAP_INFO_SIZE;
			
			MAP_INFO_SIZE[0] = mapMaxs[0] - mapMins[0];
			MAP_INFO_SIZE[1] = mapMaxs[1] - mapMins[1];
			MAP_INFO_SIZE[2] = mapMaxs[2] - mapMins[2];

			float yoff = density * 0.5;

//#pragma omp parallel for schedule(dynamic)
			for (int x = (int)mapMins[0]; x <= (int)mapMaxs[0]; x += density)
			{
				if (grassSpotCount >= FOLIAGE_MAX_FOLIAGES)
				{
					continue;
				}

				float current =  MAP_INFO_SIZE[0] - (mapMaxs[0] - (float)x);
				float complete = current / MAP_INFO_SIZE[0];

				aw_percent_complete = (float)(complete * 100.0);

				if (yoff == density * 0.75)
					yoff = density * 1.25;
				else if (yoff == density * 1.25)
					yoff = density;
				else
					yoff = density * 0.75;

				for (float y = mapMins[1]; y <= mapMaxs[1]; y += yoff/*density*/)
				{
					if (grassSpotCount >= FOLIAGE_MAX_FOLIAGES)
					{
						break;
					}

					for (float z = mapMaxs[2]; z >= mapMins[2]; z -= 48.0)
					{
						if (grassSpotCount >= FOLIAGE_MAX_FOLIAGES)
						{
							break;
						}

						if(omp_get_thread_num() == 0)
						{// Draw a nice little progress bar ;)
							if (clock() - previous_time > 500) // update display every 500ms...
							{
								previous_time = clock();
								trap->UpdateScreen();
							}
						}

						trace_t		tr;
						vec3_t		pos, down;
						qboolean	FOUND = qfalse;

						VectorSet(pos, x, y, z);
						pos[2] += 8.0;
						VectorCopy(pos, down);
						down[2] = mapMins[2];

						CG_Trace( &tr, pos, NULL, NULL, down, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

						if (tr.endpos[2] <= mapMins[2])
						{// Went off map...
							break;
						}

						if ( tr.surfaceFlags & SURF_SKY )
						{// Sky...
							continue;
						}

						if ( tr.surfaceFlags & SURF_NODRAW )
						{// don't generate a drawsurface at all
							continue;
						}

						if ( tr.contents & CONTENTS_WATER )
						{// Anything below here is underwater...
							break;
						}

						if (MaterialIsValidForGrass((tr.surfaceFlags & MATERIAL_MASK)))
						{
							// Look around here for a different slope angle... Cull if found...
							for (float scale = 1.00; scale >= 0.05; scale -= 0.05)
							{
								if (FOLIAGE_CheckSlopesAround(pos, down, tr.endpos, tr.plane.normal, scale))
								{
#pragma omp critical (__ADD_TEMP_NODE__)
									{
										sprintf(last_node_added_string, "^5Adding foliage point ^3%i ^5at ^7%f %f %f^5.", grassSpotCount, tr.endpos[0], tr.endpos[1], tr.endpos[2]+8);

										if (scale == 1.0)
											grassSpotScale[grassSpotCount] = (irand(75, 100) / 100.0);
										else
											grassSpotScale[grassSpotCount] = scale;

										VectorSet(grassSpotList[grassSpotCount], tr.endpos[0], tr.endpos[1], tr.endpos[2]+8);
										grassSpotCount++;
										FOUND = qtrue;
									}
								}

								if (FOUND) break;
							}

							if (FOUND) break;
						}
					}
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
					FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] = grassSpotScale[i];

					FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
					FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = 0.0f;
					FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = grassSpotScale[i];

					FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
					FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = 0.0f;
					FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = 0.0f;

					VectorCopy(vec, FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS]);
					FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS][2] -= 18.0;

					FOLIAGE_GRASS_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 2);
					FOLIAGE_GRASS_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
					
					if (FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] == 0)
						FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,125) / 100.0);

					if (density >= 96)
					{
						if (random() * 7 >= 6 && RoofHeightAt(vec) - vec[2] > 1024.0)
						{// Add tree... 1 in every 8 positions... If there is room above for a tree...
							FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
							FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
						}
						else if (random() * 100 >= 50)
						{// Add plant... 1 in every 2 positions...
							FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
							FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);

							if (FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] == 0)
								FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
						}
					}
					else if (density >= 64)
					{
						if (random() * 10 >= 9 && RoofHeightAt(vec) - vec[2] > 1024.0)
						{// Add tree... 1 in every 10 positions... If there is room above for a tree...
							FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
							FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
						}
						else if (random() * 100 >= 50)
						{// Add plant... 1 in every 2 positions...
							FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
							FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);

							if (FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] == 0)
								FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
						}
					}
					else if (density >= 48)
					{
						if (random() * 12 >= 11 && RoofHeightAt(vec) - vec[2] > 1024.0)
						{// Add tree... 1 in every 12 positions... If there is room above for a tree...
							FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
							FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
						}
						else if (random() * 100 >= 50)
						{// Add plant... 1 in every 2 positions...
							FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
							FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);

							if (FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] == 0)
								FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
						}
					}
					else
					{
						if (random() * 18 >= 17 && RoofHeightAt(vec) - vec[2] > 1024.0)
						{// Add tree... 1 in every 17 positions... If there is room above for a tree...
							FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 3);
							FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
							FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
						}
						else if (random() * 100 >= 66)
						{// Add plant... 1 in every 1.33 positions...
							FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = (int)(random() * 27);
							FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);

							if (FOLIAGE_GRASS_SCALE[FOLIAGE_NUM_POSITIONS] == 0)
								FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(35,125) / 100.0);
						}
					}

#ifdef __PREGENERATE_NORMALS__
					trace_t tr;
					vec3_t	up, down;
					VectorCopy(FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS], up);
					up[2]+=128;
					VectorCopy(FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS], down);
					down[2]-=128;
					CG_Trace(&tr, up, NULL, NULL, down, -1, MASK_SOLID);
					VectorCopy(tr.plane.normal, FOLIAGE_NORMALS[FOLIAGE_NUM_POSITIONS]);
#endif //__PREGENERATE_NORMALS__

					FOLIAGE_NUM_POSITIONS++;
				}

				for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
				{
					FOLIAGE_GRASS_SHADERNUM[i] = irand(1,36);
					FOLIAGE_PLANT_SHADERNUM[i] = irand(1,NUM_PLANT_SHADERS-1);
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
