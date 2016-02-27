extern "C" {
#include "../qcommon/q_shared.h"
#include "../cgame/cg_local.h"
#include "../ui/ui_shared.h"
#include "../game/surfaceflags.h"
#include "../qcommon/inifile.h"

extern qboolean InFOV( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV );

	// =======================================================================================================================================
	//
	//                                                             Foliage Rendering...
	//
	// =======================================================================================================================================


#define			FOLIAGE_MAX_FOLIAGES 1048576

	// =======================================================================================================================================
	//
	// BEGIN - FOLIAGE OPTIONS
	//
	// =======================================================================================================================================

	//#define		__NO_PLANTS__ // Disable plants and only draw grass for everything... Was just for testing FPS difference...

	// =======================================================================================================================================
	//
	// END - FOLIAGE OPTIONS
	//
	// =======================================================================================================================================


	// =======================================================================================================================================
	//
	// These settings below are here for future adjustment and expansion...
	//
	// TODO: * Load <climateType>.climate file with all grasses/plants/trees md3's and textures lists.
	//       * Add climate selection option to the header of <mapname>.foliage
	//
	// =======================================================================================================================================

char		CURRENT_CLIMATE_OPTION[256] = { 0 };

float		NUM_TREE_TYPES = 9;

float		NUM_PLANT_SHADERS = 0;

#define		PLANT_SCALE_MULTIPLIER 1.0

#define		MAX_PLANT_SHADERS 100

float		TREE_SCALE_MULTIPLIER = 1.0;

	static const char *TropicalPlantsList[] = {
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
		"models/warzone/foliage/plant71.png",
		"models/warzone/foliage/plant72.png",
		"models/warzone/foliage/plant73.png",
		"models/warzone/foliage/plant74.png",
		"models/warzone/foliage/plant75.png",
		"models/warzone/foliage/plant76.png",
		"models/warzone/foliage/plant77.png",
		"models/warzone/foliage/plant78.png",
		"models/warzone/foliage/plant79.png",
		"models/warzone/foliage/plant80.png",
		"models/warzone/foliage/plant81.png",
		"models/warzone/foliage/plant82.png",
		"models/warzone/foliage/plant83.png",
		"models/warzone/foliage/plant84.png",
		"models/warzone/foliage/plant85.png",
		"models/warzone/foliage/plant86.png",
		"models/warzone/foliage/plant87.png",
		"models/warzone/foliage/plant88.png",
		"models/warzone/foliage/plant89.png",
		"models/warzone/foliage/plant90.png",
		"models/warzone/foliage/plant91.png",
		"models/warzone/foliage/plant92.png",
		"models/warzone/foliage/plant93.png",
		"models/warzone/foliage/plant94.png",
		"models/warzone/foliage/plant95.png",
		"models/warzone/foliage/plant96.png",
		"models/warzone/foliage/plant97.png",
		"models/warzone/foliage/plant98.png",
		"models/warzone/foliage/plant99.png",
		"models/warzone/foliage/plant100.png",
	};

	static const char *SpringPlantsList[] = {
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
		"models/warzone/foliage/plant71.png",
		"models/warzone/foliage/plant72.png",
		"models/warzone/foliage/plant73.png",
		"models/warzone/foliage/plant74.png",
		"models/warzone/foliage/plant75.png",
		"models/warzone/foliage/plant76.png",
		"models/warzone/foliage/plant77.png",
		"models/warzone/foliage/plant78.png",
		"models/warzone/foliage/plant79.png",
		"models/warzone/foliage/plant80.png",
		"models/warzone/foliage/plant81.png",
		"models/warzone/foliage/plant82.png",
		"models/warzone/foliage/plant83.png",
		"models/warzone/foliage/plant84.png",
		"models/warzone/foliage/plant85.png",
		"models/warzone/foliage/plant86.png",
		"models/warzone/foliage/plant87.png",
		"models/warzone/foliage/plant88.png",
		"models/warzone/foliage/plant89.png",
		"models/warzone/foliage/plant90.png",
		"models/warzone/foliage/plant91.png",
		"models/warzone/foliage/plant92.png",
		"models/warzone/foliage/plant93.png",
		"models/warzone/foliage/plant94.png",
		"models/warzone/foliage/plant95.png",
		"models/warzone/foliage/plant96.png",
		"models/warzone/foliage/plant97.png",
		"models/warzone/foliage/plant98.png",
		"models/warzone/foliage/plant99.png",
		"models/warzone/foliage/plant100.png",
	};

	static const char *EndorPlantsList[] = {
		"models/warzone/foliage/fern01.png",
		"models/warzone/foliage/fern02.png",
		"models/warzone/foliage/fern03.png",
		"models/warzone/foliage/fern04.png",
		"models/warzone/foliage/fern05.png",
		"models/warzone/foliage/fern06.png",
		"models/warzone/foliage/fern07.png",
		"models/warzone/foliage/fern01.png",
		"models/warzone/foliage/fern02.png",
		"models/warzone/foliage/fern03.png",
		"models/warzone/foliage/fern04.png",
		"models/warzone/foliage/fern05.png",
		"models/warzone/foliage/fern06.png",
		"models/warzone/foliage/fern07.png",
		"models/warzone/foliage/plant09.png",
		"models/warzone/foliage/plant13.png",
		"models/warzone/foliage/plant60.png",
		"models/warzone/foliage/plant72.png",
		"models/warzone/foliage/plant88.png",
		"models/warzone/foliage/fern01.png",
		"models/warzone/foliage/fern02.png",
		"models/warzone/foliage/fern03.png",
		"models/warzone/foliage/fern04.png",
		"models/warzone/foliage/fern05.png",
		"models/warzone/foliage/fern06.png",
		"models/warzone/foliage/fern07.png",
		"models/warzone/foliage/fern01.png",
		"models/warzone/foliage/fern02.png",
		"models/warzone/foliage/fern03.png",
		"models/warzone/foliage/fern04.png",
		"models/warzone/foliage/fern05.png",
		"models/warzone/foliage/fern06.png",
		"models/warzone/foliage/fern07.png",
		"models/warzone/foliage/plant09.png",
		"models/warzone/foliage/plant13.png",
		"models/warzone/foliage/plant60.png",
		"models/warzone/foliage/plant72.png",
		"models/warzone/foliage/plant88.png",
		"models/warzone/foliage/fern01.png",
		"models/warzone/foliage/fern02.png",
		"models/warzone/foliage/fern03.png",
		"models/warzone/foliage/fern04.png",
		"models/warzone/foliage/fern05.png",
		"models/warzone/foliage/fern06.png",
		"models/warzone/foliage/fern07.png",
		"models/warzone/foliage/fern01.png",
		"models/warzone/foliage/fern02.png",
		"models/warzone/foliage/fern03.png",
		"models/warzone/foliage/fern04.png",
		"models/warzone/foliage/fern05.png",
		"models/warzone/foliage/fern06.png",
		"models/warzone/foliage/fern07.png",
		"models/warzone/foliage/plant09.png",
		"models/warzone/foliage/plant13.png",
		"models/warzone/foliage/plant60.png",
		"models/warzone/foliage/plant72.png",
		"models/warzone/foliage/plant88.png",
		"models/warzone/foliage/fern01.png",
		"models/warzone/foliage/fern02.png",
		"models/warzone/foliage/fern03.png",
		"models/warzone/foliage/fern04.png",
		"models/warzone/foliage/fern05.png",
		"models/warzone/foliage/fern06.png",
		"models/warzone/foliage/fern07.png",
		"models/warzone/foliage/fern01.png",
		"models/warzone/foliage/fern02.png",
		"models/warzone/foliage/fern03.png",
		"models/warzone/foliage/fern04.png",
		"models/warzone/foliage/fern05.png",
		"models/warzone/foliage/fern06.png",
		"models/warzone/foliage/fern07.png",
		"models/warzone/foliage/plant09.png",
		"models/warzone/foliage/plant13.png",
		"models/warzone/foliage/plant60.png",
		"models/warzone/foliage/plant72.png",
		"models/warzone/foliage/plant88.png",
		"models/warzone/foliage/fern01.png",
		"models/warzone/foliage/fern02.png",
		"models/warzone/foliage/fern03.png",
		"models/warzone/foliage/fern04.png",
		"models/warzone/foliage/fern05.png",
		"models/warzone/foliage/fern06.png",
		"models/warzone/foliage/fern07.png",
		"models/warzone/foliage/fern01.png",
		"models/warzone/foliage/fern02.png",
		"models/warzone/foliage/fern03.png",
		"models/warzone/foliage/fern04.png",
		"models/warzone/foliage/fern05.png",
		"models/warzone/foliage/fern06.png",
		"models/warzone/foliage/fern07.png",
		"models/warzone/foliage/plant09.png",
		"models/warzone/foliage/plant13.png",
		"models/warzone/foliage/plant60.png",
		"models/warzone/foliage/plant72.png",
		"models/warzone/foliage/plant88.png",
		"models/warzone/foliage/fern01.png",
		"models/warzone/foliage/fern02.png",
		"models/warzone/foliage/fern03.png",
		"models/warzone/foliage/fern04.png",
		"models/warzone/foliage/fern05.png",
	};

	static const char *SnowPlantsList[] = {
		"models/warzone/foliage/plant22.png",
		"models/warzone/foliage/plant23.png",
		"models/warzone/foliage/plant34.png",
		"models/warzone/foliage/plant38.png",
		"models/warzone/foliage/plant39.png",
		"models/warzone/foliage/plant45.png",
		"models/warzone/foliage/plant46.png",
		"models/warzone/foliage/plant47.png",
		"models/warzone/foliage/plant93.png",
		"models/warzone/foliage/plant96.png",
		"models/warzone/foliage/plant98.png",
		"models/warzone/foliage/plant22.png",
		"models/warzone/foliage/plant23.png",
		"models/warzone/foliage/plant34.png",
		"models/warzone/foliage/plant38.png",
		"models/warzone/foliage/plant39.png",
		"models/warzone/foliage/plant45.png",
		"models/warzone/foliage/plant46.png",
		"models/warzone/foliage/plant47.png",
		"models/warzone/foliage/plant93.png",
		"models/warzone/foliage/plant96.png",
		"models/warzone/foliage/plant98.png",
		"models/warzone/foliage/plant22.png",
		"models/warzone/foliage/plant23.png",
		"models/warzone/foliage/plant34.png",
		"models/warzone/foliage/plant38.png",
		"models/warzone/foliage/plant39.png",
		"models/warzone/foliage/plant45.png",
		"models/warzone/foliage/plant46.png",
		"models/warzone/foliage/plant47.png",
		"models/warzone/foliage/plant93.png",
		"models/warzone/foliage/plant96.png",
		"models/warzone/foliage/plant98.png",
		"models/warzone/foliage/plant22.png",
		"models/warzone/foliage/plant23.png",
		"models/warzone/foliage/plant34.png",
		"models/warzone/foliage/plant38.png",
		"models/warzone/foliage/plant39.png",
		"models/warzone/foliage/plant45.png",
		"models/warzone/foliage/plant46.png",
		"models/warzone/foliage/plant47.png",
		"models/warzone/foliage/plant93.png",
		"models/warzone/foliage/plant96.png",
		"models/warzone/foliage/plant98.png",
		"models/warzone/foliage/plant22.png",
		"models/warzone/foliage/plant23.png",
		"models/warzone/foliage/plant34.png",
		"models/warzone/foliage/plant38.png",
		"models/warzone/foliage/plant39.png",
		"models/warzone/foliage/plant45.png",
		"models/warzone/foliage/plant46.png",
		"models/warzone/foliage/plant47.png",
		"models/warzone/foliage/plant93.png",
		"models/warzone/foliage/plant96.png",
		"models/warzone/foliage/plant98.png",
		"models/warzone/foliage/plant22.png",
		"models/warzone/foliage/plant23.png",
		"models/warzone/foliage/plant34.png",
		"models/warzone/foliage/plant38.png",
		"models/warzone/foliage/plant39.png",
		"models/warzone/foliage/plant45.png",
		"models/warzone/foliage/plant46.png",
		"models/warzone/foliage/plant47.png",
		"models/warzone/foliage/plant93.png",
		"models/warzone/foliage/plant96.png",
		"models/warzone/foliage/plant98.png",
		"models/warzone/foliage/plant22.png",
		"models/warzone/foliage/plant23.png",
		"models/warzone/foliage/plant34.png",
		"models/warzone/foliage/plant38.png",
		"models/warzone/foliage/plant39.png",
		"models/warzone/foliage/plant45.png",
		"models/warzone/foliage/plant46.png",
		"models/warzone/foliage/plant47.png",
		"models/warzone/foliage/plant93.png",
		"models/warzone/foliage/plant96.png",
		"models/warzone/foliage/plant98.png",
		"models/warzone/foliage/plant22.png",
		"models/warzone/foliage/plant23.png",
		"models/warzone/foliage/plant34.png",
		"models/warzone/foliage/plant38.png",
		"models/warzone/foliage/plant39.png",
		"models/warzone/foliage/plant45.png",
		"models/warzone/foliage/plant46.png",
		"models/warzone/foliage/plant47.png",
		"models/warzone/foliage/plant93.png",
		"models/warzone/foliage/plant96.png",
		"models/warzone/foliage/plant98.png",
		"models/warzone/foliage/plant22.png",
		"models/warzone/foliage/plant23.png",
		"models/warzone/foliage/plant34.png",
		"models/warzone/foliage/plant38.png",
		"models/warzone/foliage/plant39.png",
		"models/warzone/foliage/plant45.png",
		"models/warzone/foliage/plant46.png",
		"models/warzone/foliage/plant47.png",
		"models/warzone/foliage/plant93.png",
		"models/warzone/foliage/plant96.png",
		"models/warzone/foliage/plant98.png",
		"models/warzone/foliage/plant22.png",
	};

	// =======================================================================================================================================
	//
	// CVAR related defines...
	//
	// =======================================================================================================================================

#define			__FOLIAGE_DENSITY__ cg_foliageDensity.value

	float		FOLIAGE_AREA_SIZE =					512;
	float		FOLIAGE_VISIBLE_DISTANCE =			FOLIAGE_AREA_SIZE*cg_foliageGrassRangeMult.value;
	float		FOLIAGE_PLANT_VISIBLE_DISTANCE =	FOLIAGE_AREA_SIZE*cg_foliagePlantRangeMult.value;
	float		FOLIAGE_TREE_VISIBLE_DISTANCE =		FOLIAGE_AREA_SIZE*cg_foliageTreeRangeMult.value;

#define		FOLIAGE_AREA_MAX				131072
#define		FOLIAGE_AREA_MAX_FOLIAGES		256

typedef enum {
	FOLIAGE_PASS_GRASS,
	FOLIAGE_PASS_PLANT,
	FOLIAGE_PASS_CLOSETREE,
	FOLIAGE_PASS_TREE,
} foliagePassTypes_t;

	// =======================================================================================================================================
	//
	// These maybe should have been a stuct, but many separate variables let us use static memory allocations...
	//
	// =======================================================================================================================================

	int			FOLIAGE_AREAS_COUNT = 0;
	int			FOLIAGE_AREAS_LIST_COUNT[FOLIAGE_AREA_MAX];
	int			FOLIAGE_AREAS_LIST[FOLIAGE_AREA_MAX][FOLIAGE_AREA_MAX_FOLIAGES];
	int			FOLIAGE_AREAS_TREES_LIST_COUNT[FOLIAGE_AREA_MAX];
	int			FOLIAGE_AREAS_TREES_LIST[FOLIAGE_AREA_MAX][FOLIAGE_AREA_MAX_FOLIAGES];
	vec3_t		FOLIAGE_AREAS_MINS[FOLIAGE_AREA_MAX];
	vec3_t		FOLIAGE_AREAS_MAXS[FOLIAGE_AREA_MAX];


	qboolean	FOLIAGE_LOADED = qfalse;
	int			FOLIAGE_NUM_POSITIONS = 0;
	vec3_t		FOLIAGE_POSITIONS[FOLIAGE_MAX_FOLIAGES];
	vec3_t		FOLIAGE_NORMALS[FOLIAGE_MAX_FOLIAGES];
	int			FOLIAGE_PLANT_SELECTION[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_PLANT_ANGLES[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_PLANT_SCALE[FOLIAGE_MAX_FOLIAGES];
	int			FOLIAGE_TREE_SELECTION[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_TREE_ANGLES[FOLIAGE_MAX_FOLIAGES];
	float		FOLIAGE_TREE_SCALE[FOLIAGE_MAX_FOLIAGES];


	qhandle_t	FOLIAGE_PLANT_MODEL[5] = { 0 };
	qhandle_t	FOLIAGE_GRASS_BILLBOARD_SHADER = 0;
	qhandle_t	FOLIAGE_TREE_MODEL[16] = { 0 };
	float		FOLIAGE_TREE_RADIUS[16] = { 0 };
	float		FOLIAGE_TREE_ZOFFSET[16] = { 0 };
	qhandle_t	FOLIAGE_TREE_BILLBOARD_SHADER[16] = { 0 };
	float		FOLIAGE_TREE_BILLBOARD_SIZE[16] = { 0 };

	qhandle_t	FOLIAGE_PLANT_SHADERS[MAX_PLANT_SHADERS] = {0};

	int			IN_RANGE_AREAS_LIST_COUNT = 0;
	int			IN_RANGE_AREAS_LIST[1024];
	float		IN_RANGE_AREAS_DISTANCE[1024];
	int			IN_RANGE_TREE_AREAS_LIST_COUNT = 0;
	int			IN_RANGE_TREE_AREAS_LIST[8192];
	float		IN_RANGE_TREE_AREAS_DISTANCE[8192];

#define FOLIAGE_SOLID_TREES_MAX 4
	int			FOLIAGE_SOLID_TREES[FOLIAGE_SOLID_TREES_MAX];
	float		FOLIAGE_SOLID_TREES_DIST[FOLIAGE_SOLID_TREES_MAX];

	float		OLD_FOLIAGE_DENSITY = 64.0;

	// =======================================================================================================================================
	//
	// Area System... This allows us to manipulate in realtime which foliages we should use...
	//
	// =======================================================================================================================================

	int FOLIAGE_AreaNumForOrg( vec3_t moveOrg )
	{
		for (int areaNum = 0; areaNum < FOLIAGE_AREAS_COUNT; areaNum++)
		{
			if (FOLIAGE_AREAS_MINS[areaNum][0] < moveOrg[0]
				&& FOLIAGE_AREAS_MINS[areaNum][1] < moveOrg[1]
				&& FOLIAGE_AREAS_MAXS[areaNum][0] >= moveOrg[0]
				&& FOLIAGE_AREAS_MAXS[areaNum][1] >= moveOrg[1])
			{
				return areaNum;
			}
		}

		return qfalse;
	}

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

	void FOLIAGE_Setup_Foliage_Areas( void )
	{
		int		areaNum = 0, i = 0;
		vec3_t	mins, maxs, mapMins, mapMaxs;

		VectorSet(mapMins, 128000, 128000, 0);
		VectorSet(mapMaxs, -128000, -128000, 0);

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
		int ZERO_SCALE_REMOVED = 0;

		FOLIAGE_AREAS_COUNT = 0;

		for (areaNum = 0; areaNum < FOLIAGE_AREA_MAX; areaNum++)
		{
			if (mins[1] > mapMaxs[1]) break; // found our last area...

			FOLIAGE_AREAS_LIST_COUNT[areaNum] = 0;
			FOLIAGE_AREAS_TREES_LIST_COUNT[areaNum] = 0;

			while (FOLIAGE_AREAS_LIST_COUNT[areaNum] == 0 && mins[1] <= mapMaxs[1])
			{// While loop is so we can skip zero size areas for speed...
				VectorCopy(mins, FOLIAGE_AREAS_MINS[areaNum]);
				VectorCopy(maxs, FOLIAGE_AREAS_MAXS[areaNum]);

				// Assign foliages to the area lists...
				for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
				{
					if (FOLIAGE_In_Bounds(areaNum, i))
					{
						qboolean OVER_DENSITY = qfalse;

						if (FOLIAGE_AREAS_LIST_COUNT[areaNum] > FOLIAGE_AREA_MAX_FOLIAGES)
						{
							//trap->Print("*** Area %i has more then %i foliages ***\n", areaNum, (int)FOLIAGE_AREA_MAX_FOLIAGES);
							break;
						}

						if (FOLIAGE_TREE_SELECTION[i] <= 0)
						{// Never remove trees...
							if (FOLIAGE_PLANT_SCALE[i] <= 0)
							{// Zero scale plant... Remove...
								ZERO_SCALE_REMOVED++;
								continue;
							}

							for (int j = 0; j < FOLIAGE_AREAS_LIST_COUNT[areaNum]; j++)
							{// Let's use a density setting to improve FPS...
								if (DistanceHorizontal(FOLIAGE_POSITIONS[i], FOLIAGE_POSITIONS[FOLIAGE_AREAS_LIST[areaNum][j]]) < __FOLIAGE_DENSITY__ * FOLIAGE_PLANT_SCALE[i])
								{// Adding this would go over density setting...
									OVER_DENSITY = qtrue;
									DENSITY_REMOVED++;
									break;
								}
							}
						}

						if (!OVER_DENSITY)
						{
							FOLIAGE_AREAS_LIST[areaNum][FOLIAGE_AREAS_LIST_COUNT[areaNum]] = i;
							FOLIAGE_AREAS_LIST_COUNT[areaNum]++;

							if (FOLIAGE_TREE_SELECTION[i] > 0)
							{// Also make a trees list for faster tree selection...
								FOLIAGE_AREAS_TREES_LIST[areaNum][FOLIAGE_AREAS_TREES_LIST_COUNT[areaNum]] = i;
								FOLIAGE_AREAS_TREES_LIST_COUNT[areaNum]++;
							}
						}
					}
				}

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

		trap->Print("Generated %i foliage areas. %i used of %i total foliages. %i removed by density setting. %i removed due to zero scale.\n", FOLIAGE_AREAS_COUNT, FOLIAGE_NUM_POSITIONS - DENSITY_REMOVED, FOLIAGE_NUM_POSITIONS, DENSITY_REMOVED, ZERO_SCALE_REMOVED);
	}

	void FOLIAGE_Check_CVar_Change ( void )
	{
		if (__FOLIAGE_DENSITY__ != OLD_FOLIAGE_DENSITY)
		{
			FOLIAGE_Setup_Foliage_Areas();
		}
	}
}

qboolean FOLIAGE_Box_In_FOV ( vec3_t mins, vec3_t maxs )
{
	vec3_t mins2, maxs2, edge, edge2;

	VectorSet(mins2, mins[0], mins[1], cg.refdef.vieworg[2]);
	VectorSet(maxs2, maxs[0], maxs[1], cg.refdef.vieworg[2]);
	VectorSet(edge, maxs2[0], mins2[1], maxs2[2]);
	VectorSet(edge2, mins2[0], maxs2[1], maxs2[2]);

	/*if (!InFOV( mins, cg.refdef.vieworg, cg.refdef.viewangles, 100, 180 )
		&& !InFOV( maxs, cg.refdef.vieworg, cg.refdef.viewangles, 100, 180 )
		&& !InFOV( edge, cg.refdef.vieworg, cg.refdef.viewangles, 100, 180 )
		&& !InFOV( edge2, cg.refdef.vieworg, cg.refdef.viewangles, 100, 180 ))
		return qfalse;*/

	if (!InFOV( mins2, cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x, cg.refdef.fov_y/*180*/ )
		&& !InFOV( maxs2, cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x, cg.refdef.fov_y/*180*/ )
		&& !InFOV( edge, cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x, cg.refdef.fov_y/*180*/ )
		&& !InFOV( edge2, cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x, cg.refdef.fov_y/*180*/ ))
		return qfalse;

	return qtrue;
}

qboolean FOLIAGE_In_FOV ( vec3_t mins, vec3_t maxs )
{
	vec3_t edge, edge2;

	VectorSet(edge, maxs[0], mins[1], maxs[2]);
	VectorSet(edge2, mins[0], maxs[1], maxs[2]);

	// FIXME: Go back to 180 Y axis later, if we can boost the speed of the system somewhere else. Using FOV_Y
	//        causes some issues when looking down at an area (something about InFOV seems to not like going
	//        under 0 and looping back to 360?)
	if (!InFOV( mins, cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x, cg.refdef.fov_y/*180*/ )
		&& !InFOV( maxs, cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x, cg.refdef.fov_y/*180*/ )
		&& !InFOV( edge, cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x, cg.refdef.fov_y/*180*/ )
		&& !InFOV( edge2, cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x, cg.refdef.fov_y/*180*/ ))
		return qfalse;

	return qtrue;
}

vec3_t		LAST_ORG = { 0 };
vec3_t		LAST_ANG = { 0 };

void FOLIAGE_VisibleAreaSortGrass( void )
{// Sorted furthest to closest...
	int i, j, increment, temp;
	float tempDist;

	increment = 3;

	while (increment > 0)
	{
		for (i=0; i < IN_RANGE_AREAS_LIST_COUNT; i++)
		{
			temp = IN_RANGE_AREAS_LIST[i];
			tempDist = IN_RANGE_AREAS_DISTANCE[i];

			j = i;

			while ((j >= increment) && (IN_RANGE_AREAS_DISTANCE[j-increment] < tempDist))
			{
				IN_RANGE_AREAS_LIST[j] = IN_RANGE_AREAS_LIST[j - increment];
				IN_RANGE_AREAS_DISTANCE[j] = IN_RANGE_AREAS_DISTANCE[j - increment];
				j = j - increment;
			}

			IN_RANGE_AREAS_LIST[j] = temp;
			IN_RANGE_AREAS_DISTANCE[j] = tempDist;
		}

		if (increment/2 != 0)
			increment = increment/2;
		else if (increment == 1)
			increment = 0;
		else
			increment = 1;
	}
}

void FOLIAGE_VisibleAreaSortTrees( void )
{// Sorted closest to furthest...
	int i, j, increment, temp;
	float tempDist;

	increment = 3;

	while (increment > 0)
	{
		for (i=0; i < IN_RANGE_TREE_AREAS_LIST_COUNT; i++)
		{
			temp = IN_RANGE_TREE_AREAS_LIST[i];
			tempDist = IN_RANGE_TREE_AREAS_DISTANCE[i];

			j = i;

			while ((j >= increment) && (IN_RANGE_TREE_AREAS_DISTANCE[j-increment] > tempDist))
			{
				IN_RANGE_TREE_AREAS_LIST[j] = IN_RANGE_TREE_AREAS_LIST[j - increment];
				IN_RANGE_TREE_AREAS_DISTANCE[j] = IN_RANGE_TREE_AREAS_DISTANCE[j - increment];
				j = j - increment;
			}

			IN_RANGE_TREE_AREAS_LIST[j] = temp;
			IN_RANGE_TREE_AREAS_DISTANCE[j] = tempDist;
		}

		if (increment/2 != 0)
			increment = increment/2;
		else if (increment == 1)
			increment = 0;
		else
			increment = 1;
	}
}


void FOLIAGE_Calc_In_Range_Areas( void )
{
	for (int i = 0; i < FOLIAGE_SOLID_TREES_MAX; i++)
	{
		FOLIAGE_SOLID_TREES[i] = -1;
		FOLIAGE_SOLID_TREES_DIST[i] = 131072.0;
	}

	FOLIAGE_VISIBLE_DISTANCE =		FOLIAGE_AREA_SIZE*cg_foliageGrassRangeMult.value;
	FOLIAGE_TREE_VISIBLE_DISTANCE = FOLIAGE_AREA_SIZE*cg_foliageTreeRangeMult.value;

	if (cg_foliageTreeRangeMult.value < 8.0) 
	{
		FOLIAGE_TREE_VISIBLE_DISTANCE = FOLIAGE_AREA_SIZE*8.0;
		trap->Cvar_Set("cg_foliageTreeRangeMult", "8.0");
		trap->Print("WARNING: Minimum tree range multiplier is 8.0. Cvar has been changed.\n");
	}

	if (Distance(cg.refdef.vieworg, LAST_ORG) > 128.0 || Distance(cg.refdef.viewangles, LAST_ANG) > 0.0)//50.0)
	{// Update in range list...
		VectorCopy(cg.refdef.vieworg, LAST_ORG);
		VectorCopy(cg.refdef.viewangles, LAST_ANG);

		IN_RANGE_AREAS_LIST_COUNT = 0;
		IN_RANGE_TREE_AREAS_LIST_COUNT = 0;

		// Calculate currently-in-range areas to use...
		for (int i = 0; i < FOLIAGE_AREAS_COUNT; i++)
		{
			float minsDist = DistanceHorizontal(FOLIAGE_AREAS_MINS[i], cg.refdef.vieworg);
			float maxsDist = DistanceHorizontal(FOLIAGE_AREAS_MAXS[i], cg.refdef.vieworg);

			if (minsDist < FOLIAGE_VISIBLE_DISTANCE 
				|| maxsDist < FOLIAGE_VISIBLE_DISTANCE)
			{
				if (minsDist <= FOLIAGE_AREA_SIZE * 2.0 || maxsDist <= FOLIAGE_AREA_SIZE * 2.0 || FOLIAGE_Box_In_FOV( FOLIAGE_AREAS_MINS[i], FOLIAGE_AREAS_MAXS[i] ))
				{
					IN_RANGE_AREAS_LIST[IN_RANGE_AREAS_LIST_COUNT] = i;

					if (minsDist < maxsDist)
						IN_RANGE_AREAS_DISTANCE[IN_RANGE_AREAS_LIST_COUNT] = minsDist;
					else
						IN_RANGE_AREAS_DISTANCE[IN_RANGE_AREAS_LIST_COUNT] = maxsDist;

					IN_RANGE_AREAS_LIST_COUNT++;
				}
			}
			else if (minsDist < FOLIAGE_TREE_VISIBLE_DISTANCE
				|| maxsDist < FOLIAGE_TREE_VISIBLE_DISTANCE)
			{
				if (FOLIAGE_Box_In_FOV( FOLIAGE_AREAS_MINS[i], FOLIAGE_AREAS_MAXS[i] ))
				{
					IN_RANGE_TREE_AREAS_LIST[IN_RANGE_TREE_AREAS_LIST_COUNT] = i;

					if (minsDist < maxsDist)
						IN_RANGE_TREE_AREAS_DISTANCE[IN_RANGE_TREE_AREAS_LIST_COUNT] = minsDist;
					else
						IN_RANGE_TREE_AREAS_DISTANCE[IN_RANGE_TREE_AREAS_LIST_COUNT] = maxsDist;

					IN_RANGE_TREE_AREAS_LIST_COUNT++;
				}
			}
		}

		if (cg_foliageAreaSorting.integer)
		{
			FOLIAGE_VisibleAreaSortGrass();
			FOLIAGE_VisibleAreaSortTrees();

			/*for (int i = 0; i < IN_RANGE_AREAS_LIST_COUNT; i++)
			{
				trap->Print("[%i] dist %f.\n", i, IN_RANGE_AREAS_DISTANCE[i]);
			}*/
		}

		//trap->Print("There are %i foliage areas in range. %i tree areas.\n", IN_RANGE_AREAS_LIST_COUNT, IN_RANGE_TREE_AREAS_LIST_COUNT);
	}
}

extern "C" {
	// =======================================================================================================================================
	//
	// Collision detection... Only really needed for AWP on client, but there is a CVAR to turn client side predicion on as well.
	//
	// =======================================================================================================================================

	qboolean FOLIAGE_TreeSolidBlocking_AWP(vec3_t moveOrg)
	{
		int	CLOSE_AREA_LIST[8192];
		int	CLOSE_AREA_LIST_COUNT = 0;

		for (int areaNum = 0; areaNum < FOLIAGE_AREAS_COUNT; areaNum++)
		{
			float DIST = DistanceHorizontal(FOLIAGE_AREAS_MINS[areaNum], moveOrg);
			float DIST2 = DistanceHorizontal(FOLIAGE_AREAS_MAXS[areaNum], moveOrg);

			if (DIST < FOLIAGE_AREA_SIZE * 2.0 || DIST2 < FOLIAGE_AREA_SIZE * 2.0)
			{
				CLOSE_AREA_LIST[CLOSE_AREA_LIST_COUNT] = areaNum;
				CLOSE_AREA_LIST_COUNT++;
			}
		}

		for (int areaListPos = 0; areaListPos < CLOSE_AREA_LIST_COUNT; areaListPos++)
		{
			int areaNum = CLOSE_AREA_LIST[areaListPos];

			for (int treeNum = 0; treeNum < FOLIAGE_AREAS_TREES_LIST_COUNT[areaNum]; treeNum++)
			{
				int		THIS_TREE_NUM = FOLIAGE_AREAS_TREES_LIST[areaNum][treeNum];
				int		THIS_TREE_TYPE = FOLIAGE_TREE_SELECTION[THIS_TREE_NUM]-1;
				float	TREE_RADIUS = FOLIAGE_TREE_RADIUS[THIS_TREE_TYPE] * FOLIAGE_TREE_SCALE[THIS_TREE_NUM]*TREE_SCALE_MULTIPLIER;
				float	DIST = DistanceHorizontal(FOLIAGE_POSITIONS[THIS_TREE_NUM], moveOrg);

				TREE_RADIUS += 64.0; // Extra space around the tree for player body to fit as well...

				if (FOLIAGE_TREE_SELECTION[THIS_TREE_NUM] > 0 && DIST <= TREE_RADIUS)
				{
					return qtrue;
				}
			}
		}

		return qfalse;
	}

	qboolean FOLIAGE_TreeSolidBlocking_AWP_Path(vec3_t from, vec3_t to)
	{
		int	CLOSE_AREA_LIST[8192];
		int	CLOSE_AREA_LIST_COUNT = 0;

		float fullDist = DistanceHorizontal(from, to);

		for (int areaNum = 0; areaNum < FOLIAGE_AREAS_COUNT; areaNum++)
		{
			float DIST = DistanceHorizontal(FOLIAGE_AREAS_MINS[areaNum], to);
			float DIST2 = DistanceHorizontal(FOLIAGE_AREAS_MAXS[areaNum], to);

			if (DIST < FOLIAGE_AREA_SIZE * 2.0 || DIST2 < FOLIAGE_AREA_SIZE * 2.0)
			{
				CLOSE_AREA_LIST[CLOSE_AREA_LIST_COUNT] = areaNum;
				CLOSE_AREA_LIST_COUNT++;
			}
		}

		vec3_t dir, angles, forward;
		VectorSubtract( to, from, dir );
		vectoangles(dir, angles);
		AngleVectors( angles, forward, NULL, NULL );

		for (int areaListPos = 0; areaListPos < CLOSE_AREA_LIST_COUNT; areaListPos++)
		{
			int areaNum = CLOSE_AREA_LIST[areaListPos];

			for (int treeNum = 0; treeNum < FOLIAGE_AREAS_TREES_LIST_COUNT[areaNum]; treeNum++)
			{
				int		THIS_TREE_NUM = FOLIAGE_AREAS_TREES_LIST[areaNum][treeNum];
				int		THIS_TREE_TYPE = FOLIAGE_TREE_SELECTION[THIS_TREE_NUM]-1;
				float	TREE_RADIUS = FOLIAGE_TREE_RADIUS[THIS_TREE_TYPE] * FOLIAGE_TREE_SCALE[THIS_TREE_NUM]*TREE_SCALE_MULTIPLIER;
				float	DIST = DistanceHorizontal(FOLIAGE_POSITIONS[THIS_TREE_NUM], from);

				if (FOLIAGE_TREE_SELECTION[THIS_TREE_NUM] <= 0) continue;
				if ( fullDist < DIST ) continue;

				TREE_RADIUS += 64.0; // Extra space around the tree for player body to fit as well...

				// Check at positions along this path...
				for (int test = 8; test < DIST; test += 8)
				{
					vec3_t pos;
					
					VectorMA( from, test, forward, pos );

					float DIST2 = DistanceHorizontal(FOLIAGE_POSITIONS[THIS_TREE_NUM], pos);

					if (DIST2 <= TREE_RADIUS)
					{
						return qtrue;
					}
				}
			}
		}

		return qfalse;
	}

	qboolean FOLIAGE_TreeSolidBlocking(vec3_t moveOrg)
	{
		if (cg.predictedPlayerState.pm_type == PM_SPECTATOR) return qfalse;
		if (cgs.clientinfo[cg.clientNum].team == FACTION_SPECTATOR) return qfalse;

		for (int tree = 0; tree < FOLIAGE_SOLID_TREES_MAX; tree++)
		{
			if (FOLIAGE_SOLID_TREES[tree] >= 0)
			{
				int		THIS_TREE_NUM = FOLIAGE_SOLID_TREES[tree];
				int		THIS_TREE_TYPE = FOLIAGE_TREE_SELECTION[THIS_TREE_NUM]-1;
				float	TREE_RADIUS = FOLIAGE_TREE_RADIUS[THIS_TREE_TYPE] * FOLIAGE_TREE_SCALE[THIS_TREE_NUM]*TREE_SCALE_MULTIPLIER;
				float	TREE_HEIGHT = FOLIAGE_TREE_SCALE[THIS_TREE_NUM]*2.5*FOLIAGE_TREE_BILLBOARD_SIZE[FOLIAGE_TREE_SELECTION[THIS_TREE_NUM]-1]*TREE_SCALE_MULTIPLIER;
				float	DIST = DistanceHorizontal(FOLIAGE_POSITIONS[THIS_TREE_NUM], moveOrg);
				float	hDist = 0;

				if (cg.renderingThirdPerson)
					hDist = DistanceHorizontal(FOLIAGE_POSITIONS[THIS_TREE_NUM], cg_entities[cg.clientNum].lerpOrigin);
				else
					hDist = DistanceHorizontal(FOLIAGE_POSITIONS[THIS_TREE_NUM], cg.refdef.vieworg);

				if (DIST <= TREE_RADIUS 
					&& (moveOrg[2] >= FOLIAGE_POSITIONS[THIS_TREE_NUM][2] && moveOrg[2] <= FOLIAGE_POSITIONS[THIS_TREE_NUM][2] + (TREE_HEIGHT*2.0))
					&& DIST < hDist				// Move pos would be closer
					&& hDist > TREE_RADIUS)		// Not already stuck in tree...
				{
					//trap->Print("CLIENT: Blocked by tree %i. Radius %f. Distance %f. Type %i.\n", tree, TREE_RADIUS, DIST, THIS_TREE_TYPE);
					return qtrue;
				}
			}
		}

		return qfalse;
	}

	// =======================================================================================================================================
	//
	// Actual foliage drawing code...
	//
	// =======================================================================================================================================

	void FOLIAGE_AddFoliageEntityToScene ( refEntity_t *ent )
	{
		AddRefEntityToScene(ent);
	}

	void FOLIAGE_AddToScreen( int num, int passType ) {
		refEntity_t		re;
		vec3_t			angles;
		float			dist = 0;
		float			distFadeScale = 1.0;
		float			minFoliageScale = cg_foliageMinFoliageScale.value;

		if (FOLIAGE_TREE_SELECTION[num] <= 0 && FOLIAGE_PLANT_SCALE[num] < minFoliageScale) return;

		if (cg.renderingThirdPerson) 
			dist = Distance/*Horizontal*/(FOLIAGE_POSITIONS[num], cg_entities[cg.clientNum].lerpOrigin);
		else 
			dist = Distance/*Horizontal*/(FOLIAGE_POSITIONS[num], cg.refdef.vieworg);

		// Cull anything in the area outside of cvar specified radius...
		if (passType == FOLIAGE_PASS_TREE && dist > FOLIAGE_TREE_VISIBLE_DISTANCE) return;
		if (passType != FOLIAGE_PASS_TREE && passType != FOLIAGE_PASS_CLOSETREE && dist > FOLIAGE_PLANT_VISIBLE_DISTANCE && FOLIAGE_TREE_SELECTION[num] <= 0) return;

		memset( &re, 0, sizeof( re ) );

		//re.renderfx |= RF_FORCE_ENT_ALPHA;
		//re.renderfx |= RF_ALPHA_DEPTH;

		VectorCopy(FOLIAGE_POSITIONS[num], re.origin);

		FOLIAGE_VISIBLE_DISTANCE =			FOLIAGE_AREA_SIZE*cg_foliageGrassRangeMult.value;
		FOLIAGE_PLANT_VISIBLE_DISTANCE =	FOLIAGE_VISIBLE_DISTANCE;//FOLIAGE_AREA_SIZE*cg_foliagePlantRangeMult.value;
		FOLIAGE_TREE_VISIBLE_DISTANCE =		FOLIAGE_AREA_SIZE*cg_foliageTreeRangeMult.value;

//#define __GRASS_ONLY__

		if (dist <= FOLIAGE_PLANT_VISIBLE_DISTANCE)
		{// Draw grass...
			qboolean skipGrass = qfalse;
			qboolean skipPlant = qfalse;
			float minGrassScale = ((dist / FOLIAGE_PLANT_VISIBLE_DISTANCE) * 0.7) + 0.3;

#ifndef __GRASS_ONLY__
			if (dist > FOLIAGE_VISIBLE_DISTANCE) skipGrass = qtrue;
			if (FOLIAGE_PLANT_SCALE[num] <= minGrassScale && dist > FOLIAGE_VISIBLE_DISTANCE) skipPlant = qtrue;
			if (FOLIAGE_PLANT_SCALE[num] < minFoliageScale) skipPlant = qtrue;

			if (passType == FOLIAGE_PASS_PLANT && !skipPlant && FOLIAGE_PLANT_SELECTION[num] != 0)
			{// Add plant model as well...
				float PLANT_SCALE = FOLIAGE_PLANT_SCALE[num]*PLANT_SCALE_MULTIPLIER*distFadeScale;//*0.55;

				re.customShader = FOLIAGE_PLANT_SHADERS[FOLIAGE_PLANT_SELECTION[num]-1];

				re.reType = RT_PLANT;//RT_MODEL;

				re.origin[2] += /*8.0 **/ (1.0 - FOLIAGE_PLANT_SCALE[num]);

				re.hModel = FOLIAGE_PLANT_MODEL[4];

				VectorSet(re.modelScale, PLANT_SCALE, PLANT_SCALE, PLANT_SCALE);

				vectoangles( FOLIAGE_NORMALS[num], angles );
				angles[PITCH] += 90;
				//angles[YAW] += FOLIAGE_PLANT_ANGLES[num];

				VectorCopy(angles, re.angles);
				AnglesToAxis(angles, re.axis);

				// Add extra rotation so it's different to grass angle...
				//RotateAroundDirection( re.axis, FOLIAGE_PLANT_ANGLES[num]+45 );

				ScaleModelAxis( &re );

				FOLIAGE_AddFoliageEntityToScene( &re );
			}
			
			if ((passType == FOLIAGE_PASS_GRASS || passType == FOLIAGE_PASS_CLOSETREE) && !skipGrass && dist <= FOLIAGE_VISIBLE_DISTANCE)
#endif //__GRASS_ONLY__
			{
				float GRASS_SCALE = FOLIAGE_PLANT_SCALE[num]*PLANT_SCALE_MULTIPLIER*distFadeScale;//*0.5;

				re.reType = RT_GRASS;//RT_MODEL;

				re.origin[2] += /*8.0 **/ (1.0 - FOLIAGE_PLANT_SCALE[num]);

				re.customShader = FOLIAGE_GRASS_BILLBOARD_SHADER;
				
				// Allow user to adjust level of foliage detail by adjusting md3 lods...
				if (cg_foliageDetail.integer >= 3)
				{
					if (dist < FOLIAGE_AREA_SIZE*1.5)
						re.hModel = FOLIAGE_PLANT_MODEL[0];
					else if (dist < FOLIAGE_AREA_SIZE*2.75)
						re.hModel = FOLIAGE_PLANT_MODEL[1];
					else if (dist < FOLIAGE_AREA_SIZE*3.5)
						re.hModel = FOLIAGE_PLANT_MODEL[2];
					else
						re.hModel = FOLIAGE_PLANT_MODEL[3];
				}
				else if (cg_foliageDetail.integer >= 2)
				{
					if (dist < FOLIAGE_AREA_SIZE*1.5)
						re.hModel = FOLIAGE_PLANT_MODEL[1];
					else if (dist < FOLIAGE_AREA_SIZE*2.75)
						re.hModel = FOLIAGE_PLANT_MODEL[2];
					else
						re.hModel = FOLIAGE_PLANT_MODEL[3];
				}
				else if (cg_foliageDetail.integer >= 1)
				{
					if (dist < FOLIAGE_AREA_SIZE*1.5)
						re.hModel = FOLIAGE_PLANT_MODEL[2];
					else
						re.hModel = FOLIAGE_PLANT_MODEL[3];
				}
				else
				{
					re.hModel = FOLIAGE_PLANT_MODEL[3];
				}

				VectorSet(re.modelScale, GRASS_SCALE, GRASS_SCALE, GRASS_SCALE);

				vectoangles( FOLIAGE_NORMALS[num], angles );
				angles[PITCH] += 90;
				//angles[YAW] += FOLIAGE_PLANT_ANGLES[num];

				VectorCopy(angles, re.angles);
				AnglesToAxis(angles, re.axis);

				//RotateAroundDirection( re.axis, FOLIAGE_PLANT_ANGLES[num] );

				ScaleModelAxis( &re );

				FOLIAGE_AddFoliageEntityToScene( &re );
			}
		}

		if ((passType == FOLIAGE_PASS_CLOSETREE || FOLIAGE_PASS_TREE) && FOLIAGE_TREE_SELECTION[num] > 0)
		{// Add the tree model...
			VectorCopy(FOLIAGE_POSITIONS[num], re.origin);
			re.customShader = 0;
			re.renderfx = 0;

			if (dist > FOLIAGE_AREA_SIZE*cg_foliageTreeBillboardRangeMult.value || dist > FOLIAGE_TREE_VISIBLE_DISTANCE)
			{
				re.reType = RT_SPRITE;

				re.radius = FOLIAGE_TREE_SCALE[num]*2.5*FOLIAGE_TREE_BILLBOARD_SIZE[FOLIAGE_TREE_SELECTION[num]-1]*TREE_SCALE_MULTIPLIER;

				re.customShader = FOLIAGE_TREE_BILLBOARD_SHADER[FOLIAGE_TREE_SELECTION[num]-1];

				re.shaderRGBA[0] = 255;
				re.shaderRGBA[1] = 255;
				re.shaderRGBA[2] = 255;
				re.shaderRGBA[3] = 255;

				re.origin[2] += re.radius;
				re.origin[2] += FOLIAGE_TREE_ZOFFSET[FOLIAGE_TREE_SELECTION[num]-1];

				angles[PITCH] = angles[ROLL] = 0.0f;
				angles[YAW] = FOLIAGE_TREE_ANGLES[num];

				VectorCopy(angles, re.angles);
				AnglesToAxis(angles, re.axis);

				FOLIAGE_AddFoliageEntityToScene( &re );
			}
			else
			{
				re.reType = RT_MODEL;
				re.hModel = FOLIAGE_TREE_MODEL[FOLIAGE_TREE_SELECTION[num]-1];

				VectorSet(re.modelScale, FOLIAGE_TREE_SCALE[num]*2.5*TREE_SCALE_MULTIPLIER, FOLIAGE_TREE_SCALE[num]*2.5*TREE_SCALE_MULTIPLIER, FOLIAGE_TREE_SCALE[num]*2.5*TREE_SCALE_MULTIPLIER);

				re.origin[2] += FOLIAGE_TREE_ZOFFSET[FOLIAGE_TREE_SELECTION[num]-1];

				angles[PITCH] = angles[ROLL] = 0.0f;
				angles[YAW] = 270.0 - FOLIAGE_TREE_ANGLES[num];

				VectorCopy(angles, re.angles);
				AnglesToAxis(angles, re.axis);

				ScaleModelAxis( &re );

				FOLIAGE_AddFoliageEntityToScene( &re );

				//
				// Create a list of the closest trees for generating solids...
				//

				float	furthestDist = 0.0;
				int		furthestNum = 0;

				for (int tree = 0; tree < FOLIAGE_SOLID_TREES_MAX; tree++)
				{
					if (FOLIAGE_SOLID_TREES_DIST[tree] > furthestDist)
					{
						furthestNum = tree;
						furthestDist = FOLIAGE_SOLID_TREES_DIST[tree];
					}
				}


				if (dist < FOLIAGE_SOLID_TREES_DIST[furthestNum])
				{
					//trap->Print("Set solid tree %i at %f %f %f. Dist %f.\n", furthestNum, FOLIAGE_POSITIONS[num][0], FOLIAGE_POSITIONS[num][1], FOLIAGE_POSITIONS[num][2], dist);
					FOLIAGE_SOLID_TREES[furthestNum] = num;
					FOLIAGE_SOLID_TREES_DIST[furthestNum] = dist;
				}
			}
		}
	}

	extern int BG_SiegeGetPairedValue(char *buf, char *key, char *outbuf);

	qboolean FOLIAGE_LoadMapClimateInfo( void )
	{
		fileHandle_t	f;
		int				fLen = 0;
		char			fileBuffer[4096];
		char			parseBuf[4096];

		memset(CURRENT_CLIMATE_OPTION, 0, sizeof(CURRENT_CLIMATE_OPTION));

		fLen = trap->FS_Open(va("foliage/%s.climateInfo", cgs.currentmapname), &f, FS_READ);

		if (!f || fLen < 0)
		{//couldn't open file, just use the defaults
			trap->Print( "^1*** ^3%s^5: No map climate info file ^7foliage/%s.climateInfo^5. Using default climate option.\n", GAME_VERSION, cgs.currentmapname );
			return qfalse;
		}

		if (fLen == 0)
		{//file was empty, just use the defaults
			trap->FS_Close(f);
			trap->Print( "^1*** ^3%s^5: No map climate info file ^7foliage/%s.climateInfo^5. Using default climate option.\n", GAME_VERSION, cgs.currentmapname );
			return qfalse;
		}

		if (fLen >= 4096)
		{
			trap->Print("^1Error: foliage/%s.climateInfo is over the climateInfo filesize limit.^7\n", cgs.currentmapname);
			trap->FS_Close(f);
			trap->Print( "^1*** ^3%s^5: No map climate info file ^7foliage/%s.climateInfo^5. Using default climate option.\n", GAME_VERSION, cgs.currentmapname );
			return qfalse;
		}

		trap->FS_Read(fileBuffer, fLen, f);
		fileBuffer[fLen] = 0;
		trap->FS_Close(f);

		if (BG_SiegeGetPairedValue(fileBuffer, "climateSelection", parseBuf))
		{
			strcpy(CURRENT_CLIMATE_OPTION, parseBuf);
		}

		trap->Print( "^1*** ^3%s^5: Successfully loaded climateInfo file ^7foliage/%s.climateInfo^5. Using ^3%s^5 climate option.\n", GAME_VERSION, cgs.currentmapname, CURRENT_CLIMATE_OPTION );

		return qtrue;
	}

	qboolean FOLIAGE_LoadFoliagePositions( void )
	{
		fileHandle_t	f;
		int				i = 0;
		int				numPositions = 0;
		int				numRemovedPositions = 0;
		float			minFoliageScale = cg_foliageMinFoliageScale.value;

		trap->FS_Open( va( "foliage/%s.foliage", cgs.currentmapname), &f, FS_READ );

		if ( !f )
		{
			return qfalse;
		}

		FOLIAGE_NUM_POSITIONS = 0;

		trap->FS_Read( &numPositions, sizeof(int), f );

		for (i = 0; i < numPositions; i++)
		{
			trap->FS_Read( &FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS], sizeof(vec3_t), f );
			trap->FS_Read( &FOLIAGE_NORMALS[FOLIAGE_NUM_POSITIONS], sizeof(vec3_t), f );
			trap->FS_Read( &FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS], sizeof(int), f );
			trap->FS_Read( &FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS], sizeof(float), f );
			trap->FS_Read( &FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS], sizeof(float), f );
			trap->FS_Read( &FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS], sizeof(int), f );
			trap->FS_Read( &FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS], sizeof(float), f );
			trap->FS_Read( &FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS], sizeof(float), f );

			if (FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] <= 0 && FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] < minFoliageScale)
			{// Below our minimum size specified by cg_foliageMinFoliageScale. Remove!
				FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
				FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
				numRemovedPositions++;
			}
			else
			{
				FOLIAGE_NUM_POSITIONS++;
			}
		}

		trap->FS_Close(f);

		trap->Print( "^1*** ^3%s^5: Successfully loaded %i foliage points (%i removed by cg_foliageMinFoliageScale) from foliage file ^7foliage/%s.foliage^5.\n", GAME_VERSION,
			FOLIAGE_NUM_POSITIONS, numRemovedPositions, cgs.currentmapname );

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

		for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
		{
			trace_t tr;
			vec3_t	up, down;
			VectorCopy(FOLIAGE_POSITIONS[i], up);
			up[2]+=128;
			VectorCopy(FOLIAGE_POSITIONS[i], down);
			down[2]-=128;
			CG_Trace(&tr, up, NULL, NULL, down, -1, MASK_SOLID);
			VectorCopy(tr.plane.normal, FOLIAGE_NORMALS[i]);
		}

		trap->FS_Write( &FOLIAGE_NUM_POSITIONS, sizeof(int), f );

		for (i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
		{
			trap->FS_Write( &FOLIAGE_POSITIONS[i], sizeof(vec3_t), f );
			trap->FS_Write( &FOLIAGE_NORMALS[i], sizeof(vec3_t), f );
			trap->FS_Write( &FOLIAGE_PLANT_SELECTION[i], sizeof(int), f );
			trap->FS_Write( &FOLIAGE_PLANT_ANGLES[i], sizeof(float), f );
			trap->FS_Write( &FOLIAGE_PLANT_SCALE[i], sizeof(float), f );
			trap->FS_Write( &FOLIAGE_TREE_SELECTION[i], sizeof(int), f );
			trap->FS_Write( &FOLIAGE_TREE_ANGLES[i], sizeof(float), f );
			trap->FS_Write( &FOLIAGE_TREE_SCALE[i], sizeof(float), f );
		}

		trap->FS_Close(f);

		FOLIAGE_Setup_Foliage_Areas();

		trap->Print( "^1*** ^3%s^5: Successfully saved %i grass points to foliage file ^7foliage/%s.foliage^5.\n", GAME_VERSION,
			FOLIAGE_NUM_POSITIONS, cgs.currentmapname );

		return qtrue;
	}

	qboolean FOLIAGE_IgnoreFoliageOnMap( void )
	{
#if 0
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

#endif
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
			FOLIAGE_LoadMapClimateInfo();
		}

		if (FOLIAGE_NUM_POSITIONS <= 0)
		{
			return;
		}

		if (!FOLIAGE_PLANT_MODEL[0])
		{// Init/register all foliage models...
			TREE_SCALE_MULTIPLIER = 1.0;

			FOLIAGE_PLANT_MODEL[0] = trap->R_RegisterModel( "models/warzone/foliage/uqgrass.md3" );
			FOLIAGE_PLANT_MODEL[1] = trap->R_RegisterModel( "models/warzone/foliage/uqgrass_lod.md3" );
			FOLIAGE_PLANT_MODEL[2] = trap->R_RegisterModel( "models/warzone/foliage/uqgrass_lod2.md3" );
			FOLIAGE_PLANT_MODEL[3] = trap->R_RegisterModel( "models/warzone/foliage/uqgrass_lod3.md3" );
			FOLIAGE_PLANT_MODEL[4] = trap->R_RegisterModel( "models/warzone/foliage/uqplant.md3" );


			if (!strcmp(CURRENT_CLIMATE_OPTION, "springpineforest"))
			{
				FOLIAGE_GRASS_BILLBOARD_SHADER = trap->R_RegisterShader( "models/warzone/foliage/grasspineforest" );

				for (int i = 0; i < MAX_PLANT_SHADERS; i++)
				{
					FOLIAGE_PLANT_SHADERS[i] = trap->R_RegisterShader(SpringPlantsList[i]);
				}

				FOLIAGE_TREE_MODEL[0] = trap->R_RegisterModel( "models/warzone/trees/uqconifer7.md3" );
				FOLIAGE_TREE_MODEL[1] = trap->R_RegisterModel( "models/warzone/trees/uqconifer7.md3" );
				FOLIAGE_TREE_MODEL[2] = trap->R_RegisterModel( "models/warzone/trees/uqconifer7.md3" );
				FOLIAGE_TREE_MODEL[3] = trap->R_RegisterModel( "models/warzone/trees/uqconifer8.md3" );
				FOLIAGE_TREE_MODEL[4] = trap->R_RegisterModel( "models/warzone/trees/uqconifer8.md3" );
				FOLIAGE_TREE_MODEL[5] = trap->R_RegisterModel( "models/warzone/trees/uqconifer8.md3" );
				FOLIAGE_TREE_MODEL[6] = trap->R_RegisterModel( "models/warzone/trees/uqconifer8.md3" );
				FOLIAGE_TREE_MODEL[7] = trap->R_RegisterModel( "models/warzone/trees/uqbigtree1.md3" );
				FOLIAGE_TREE_MODEL[8] = trap->R_RegisterModel( "models/warzone/trees/uqbigtree2.md3" );

				FOLIAGE_TREE_BILLBOARD_SHADER[0] = trap->R_RegisterShader("models/warzone/billboard/uqconifer7");
				FOLIAGE_TREE_BILLBOARD_SHADER[1] = trap->R_RegisterShader("models/warzone/billboard/uqconifer7");
				FOLIAGE_TREE_BILLBOARD_SHADER[2] = trap->R_RegisterShader("models/warzone/billboard/uqconifer7");
				FOLIAGE_TREE_BILLBOARD_SHADER[3] = trap->R_RegisterShader("models/warzone/billboard/uqconifer8");
				FOLIAGE_TREE_BILLBOARD_SHADER[4] = trap->R_RegisterShader("models/warzone/billboard/uqconifer8");
				FOLIAGE_TREE_BILLBOARD_SHADER[5] = trap->R_RegisterShader("models/warzone/billboard/uqconifer8");
				FOLIAGE_TREE_BILLBOARD_SHADER[6] = trap->R_RegisterShader("models/warzone/billboard/uqconifer8");
				FOLIAGE_TREE_BILLBOARD_SHADER[7] = trap->R_RegisterShader("models/warzone/billboard/uqbigtree1");
				FOLIAGE_TREE_BILLBOARD_SHADER[8] = trap->R_RegisterShader("models/warzone/billboard/uqbigtree2");

				FOLIAGE_TREE_BILLBOARD_SIZE[0] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[1] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[2] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[3] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[4] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[5] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[6] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[7] = 183.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[8] = 203.0;

				FOLIAGE_TREE_RADIUS[0] = 24.0;
				FOLIAGE_TREE_RADIUS[1] = 24.0;
				FOLIAGE_TREE_RADIUS[2] = 24.0;
				FOLIAGE_TREE_RADIUS[3] = 24.0;
				FOLIAGE_TREE_RADIUS[4] = 24.0;
				FOLIAGE_TREE_RADIUS[5] = 24.0;
				FOLIAGE_TREE_RADIUS[6] = 24.0;
				FOLIAGE_TREE_RADIUS[7] = 38.0;
				FOLIAGE_TREE_RADIUS[8] = 38.0;

				FOLIAGE_TREE_ZOFFSET[0] = -4.0;
				FOLIAGE_TREE_ZOFFSET[1] = -4.0;
				FOLIAGE_TREE_ZOFFSET[2] = -4.0;
				FOLIAGE_TREE_ZOFFSET[3] = -4.0;
				FOLIAGE_TREE_ZOFFSET[4] = -4.0;
				FOLIAGE_TREE_ZOFFSET[5] = -4.0;
				FOLIAGE_TREE_ZOFFSET[6] = -4.0;
				FOLIAGE_TREE_ZOFFSET[7] = -64.0;
				FOLIAGE_TREE_ZOFFSET[8] = -64.0;
			}
			else if (!strcmp(CURRENT_CLIMATE_OPTION, "endorredwoodforest"))
			{
				TREE_SCALE_MULTIPLIER = 1.5;

				FOLIAGE_GRASS_BILLBOARD_SHADER = trap->R_RegisterShader( "models/warzone/foliage/ferngrass" );

				for (int i = 0; i < MAX_PLANT_SHADERS; i++)
				{
					FOLIAGE_PLANT_SHADERS[i] = trap->R_RegisterShader(EndorPlantsList[i]);
				}

				FOLIAGE_TREE_MODEL[0] = trap->R_RegisterModel( "models/warzone/trees/uqredwood1.md3" );
				FOLIAGE_TREE_MODEL[1] = trap->R_RegisterModel( "models/warzone/trees/uqredwood2.md3" );
				FOLIAGE_TREE_MODEL[2] = trap->R_RegisterModel( "models/warzone/trees/uqredwood3.md3" );
				FOLIAGE_TREE_MODEL[3] = trap->R_RegisterModel( "models/warzone/trees/uqredwood4.md3" );
				FOLIAGE_TREE_MODEL[4] = trap->R_RegisterModel( "models/warzone/trees/uqredwood5.md3" );
				FOLIAGE_TREE_MODEL[5] = trap->R_RegisterModel( "models/warzone/trees/uqredwood6.md3" );
				FOLIAGE_TREE_MODEL[6] = trap->R_RegisterModel( "models/warzone/trees/uqredwood7.md3" );
				FOLIAGE_TREE_MODEL[7] = trap->R_RegisterModel( "models/warzone/trees/uqsmalltree2.md3" );
				FOLIAGE_TREE_MODEL[8] = trap->R_RegisterModel( "models/warzone/trees/uqsmalltree2.md3" );

				FOLIAGE_TREE_BILLBOARD_SHADER[0] = trap->R_RegisterShader("models/warzone/billboard/uqredwood1");
				FOLIAGE_TREE_BILLBOARD_SHADER[1] = trap->R_RegisterShader("models/warzone/billboard/uqredwood2");
				FOLIAGE_TREE_BILLBOARD_SHADER[2] = trap->R_RegisterShader("models/warzone/billboard/uqredwood3");
				FOLIAGE_TREE_BILLBOARD_SHADER[3] = trap->R_RegisterShader("models/warzone/billboard/uqredwood4");
				FOLIAGE_TREE_BILLBOARD_SHADER[4] = trap->R_RegisterShader("models/warzone/billboard/uqredwood5");
				FOLIAGE_TREE_BILLBOARD_SHADER[5] = trap->R_RegisterShader("models/warzone/billboard/uqredwood6");
				FOLIAGE_TREE_BILLBOARD_SHADER[6] = trap->R_RegisterShader("models/warzone/billboard/uqredwood7");
				FOLIAGE_TREE_BILLBOARD_SHADER[7] = trap->R_RegisterShader("models/warzone/billboard/uqsmalltree2");
				FOLIAGE_TREE_BILLBOARD_SHADER[8] = trap->R_RegisterShader("models/warzone/billboard/uqsmalltree2");


				FOLIAGE_TREE_BILLBOARD_SIZE[0] = 64.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[1] = 96.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[2] = 132.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[3] = 132.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[4] = 132.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[5] = 132.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[6] = 132.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[7] = 68.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[8] = 68.0;

				FOLIAGE_TREE_RADIUS[0] = 24.0;
				FOLIAGE_TREE_RADIUS[1] = 24.0;
				FOLIAGE_TREE_RADIUS[2] = 24.0;
				FOLIAGE_TREE_RADIUS[3] = 24.0;
				FOLIAGE_TREE_RADIUS[4] = 24.0;
				FOLIAGE_TREE_RADIUS[5] = 24.0;
				FOLIAGE_TREE_RADIUS[6] = 24.0;
				FOLIAGE_TREE_RADIUS[7] = 22.0;
				FOLIAGE_TREE_RADIUS[8] = 22.0;

				FOLIAGE_TREE_ZOFFSET[0] = -64.0;
				FOLIAGE_TREE_ZOFFSET[1] = -64.0;
				FOLIAGE_TREE_ZOFFSET[2] = -64.0;
				FOLIAGE_TREE_ZOFFSET[3] = -64.0;
				FOLIAGE_TREE_ZOFFSET[4] = -64.0;
				FOLIAGE_TREE_ZOFFSET[5] = -64.0;
				FOLIAGE_TREE_ZOFFSET[6] = -64.0;
				FOLIAGE_TREE_ZOFFSET[7] = -4.0;
				FOLIAGE_TREE_ZOFFSET[8] = -4.0;
			}
			else if (!strcmp(CURRENT_CLIMATE_OPTION, "snowpineforest"))
			{
				FOLIAGE_GRASS_BILLBOARD_SHADER = trap->R_RegisterShader( "models/warzone/foliage/grasssnowpineforest" );

				for (int i = 0; i < MAX_PLANT_SHADERS; i++)
				{
					FOLIAGE_PLANT_SHADERS[i] = trap->R_RegisterShader(SnowPlantsList[i]);
				}

				FOLIAGE_TREE_MODEL[0] = trap->R_RegisterModel( "models/warzone/trees/uqsnowpine1.md3" );
				FOLIAGE_TREE_MODEL[1] = trap->R_RegisterModel( "models/warzone/trees/uqsnowpine2.md3" );
				FOLIAGE_TREE_MODEL[2] = trap->R_RegisterModel( "models/warzone/trees/uqsnowpine3.md3" );
				FOLIAGE_TREE_MODEL[3] = trap->R_RegisterModel( "models/warzone/trees/uqsnowpine6.md3" );
				FOLIAGE_TREE_MODEL[4] = trap->R_RegisterModel( "models/warzone/trees/uqsnowpine7.md3" );
				FOLIAGE_TREE_MODEL[5] = trap->R_RegisterModel( "models/warzone/trees/uqsnowpine8.md3" );
				FOLIAGE_TREE_MODEL[6] = trap->R_RegisterModel( "models/warzone/trees/uqsnowpine9.md3" );
				FOLIAGE_TREE_MODEL[7] = trap->R_RegisterModel( "models/warzone/trees/uqsnowpine4.md3" );
				FOLIAGE_TREE_MODEL[8] = trap->R_RegisterModel( "models/warzone/trees/uqsnowpine4.md3" );

				FOLIAGE_TREE_BILLBOARD_SHADER[0] = trap->R_RegisterShader("models/warzone/billboard/uqsnowpine1");
				FOLIAGE_TREE_BILLBOARD_SHADER[1] = trap->R_RegisterShader("models/warzone/billboard/uqsnowpine2");
				FOLIAGE_TREE_BILLBOARD_SHADER[2] = trap->R_RegisterShader("models/warzone/billboard/uqsnowpine3");
				FOLIAGE_TREE_BILLBOARD_SHADER[3] = trap->R_RegisterShader("models/warzone/billboard/uqsnowpine6");
				FOLIAGE_TREE_BILLBOARD_SHADER[4] = trap->R_RegisterShader("models/warzone/billboard/uqsnowpine7");
				FOLIAGE_TREE_BILLBOARD_SHADER[5] = trap->R_RegisterShader("models/warzone/billboard/uqsnowpine8");
				FOLIAGE_TREE_BILLBOARD_SHADER[6] = trap->R_RegisterShader("models/warzone/billboard/uqsnowpine9");
				FOLIAGE_TREE_BILLBOARD_SHADER[7] = trap->R_RegisterShader("models/warzone/billboard/uqsnowpine4");
				FOLIAGE_TREE_BILLBOARD_SHADER[8] = trap->R_RegisterShader("models/warzone/billboard/uqsnowpine4");

				FOLIAGE_TREE_BILLBOARD_SIZE[0] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[1] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[2] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[3] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[4] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[5] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[6] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[7] = 203.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[8] = 203.0;

				FOLIAGE_TREE_RADIUS[0] = 24.0;
				FOLIAGE_TREE_RADIUS[1] = 24.0;
				FOLIAGE_TREE_RADIUS[2] = 24.0;
				FOLIAGE_TREE_RADIUS[3] = 24.0;
				FOLIAGE_TREE_RADIUS[4] = 24.0;
				FOLIAGE_TREE_RADIUS[5] = 24.0;
				FOLIAGE_TREE_RADIUS[6] = 24.0;
				FOLIAGE_TREE_RADIUS[7] = 38.0;
				FOLIAGE_TREE_RADIUS[8] = 38.0;

				FOLIAGE_TREE_ZOFFSET[0] = -4.0;
				FOLIAGE_TREE_ZOFFSET[1] = -4.0;
				FOLIAGE_TREE_ZOFFSET[2] = -4.0;
				FOLIAGE_TREE_ZOFFSET[3] = -4.0;
				FOLIAGE_TREE_ZOFFSET[4] = -4.0;
				FOLIAGE_TREE_ZOFFSET[5] = -4.0;
				FOLIAGE_TREE_ZOFFSET[6] = -4.0;
				FOLIAGE_TREE_ZOFFSET[7] = -64.0;
				FOLIAGE_TREE_ZOFFSET[8] = -64.0;
			}
			else if (!strcmp(CURRENT_CLIMATE_OPTION, "tropicalold"))
			{
				FOLIAGE_GRASS_BILLBOARD_SHADER = trap->R_RegisterShader( "models/warzone/foliage/grasstropical" );

				for (int i = 0; i < MAX_PLANT_SHADERS; i++)
				{
					FOLIAGE_PLANT_SHADERS[i] = trap->R_RegisterShader(TropicalPlantsList[i]);
				}

				FOLIAGE_TREE_MODEL[0] = trap->R_RegisterModel( "models/warzone/trees/fanpalm2.md3" );
				FOLIAGE_TREE_MODEL[1] = trap->R_RegisterModel( "models/warzone/trees/fanpalm2.md3" );
				FOLIAGE_TREE_MODEL[2] = trap->R_RegisterModel( "models/warzone/trees/giant1.md3" );
				FOLIAGE_TREE_MODEL[3] = trap->R_RegisterModel( "models/warzone/trees/anvilpalm1.md3" );
				FOLIAGE_TREE_MODEL[4] = trap->R_RegisterModel( "models/warzone/trees/gkbjungletreenew27v2weeping.md3" );
				FOLIAGE_TREE_MODEL[5] = trap->R_RegisterModel( "models/warzone/trees/gkbjungletreenew27v2weeping.md3" );
				FOLIAGE_TREE_MODEL[6] = trap->R_RegisterModel( "models/warzone/trees/gkbjungletreenew24v2weeping.md3" );
				FOLIAGE_TREE_MODEL[7] = trap->R_RegisterModel( "models/warzone/trees/gkbjungletreenew24v2weeping.md3" );
				FOLIAGE_TREE_MODEL[8] = trap->R_RegisterModel( "models/warzone/trees/manfern.md3" );

				FOLIAGE_TREE_BILLBOARD_SHADER[0] = trap->R_RegisterShader("models/warzone/billboard/fanpalm2");
				FOLIAGE_TREE_BILLBOARD_SHADER[1] = trap->R_RegisterShader("models/warzone/billboard/fanpalm2");
				FOLIAGE_TREE_BILLBOARD_SHADER[2] = trap->R_RegisterShader("models/warzone/billboard/giant1");
				FOLIAGE_TREE_BILLBOARD_SHADER[3] = trap->R_RegisterShader("models/warzone/billboard/anvilpalm1");
				FOLIAGE_TREE_BILLBOARD_SHADER[4] = trap->R_RegisterShader("models/warzone/billboard/gkbjungletreenew27v2weeping");
				FOLIAGE_TREE_BILLBOARD_SHADER[5] = trap->R_RegisterShader("models/warzone/billboard/gkbjungletreenew27v2weeping");
				FOLIAGE_TREE_BILLBOARD_SHADER[6] = trap->R_RegisterShader("models/warzone/billboard/gkbjungletreenew24v2weeping");
				FOLIAGE_TREE_BILLBOARD_SHADER[7] = trap->R_RegisterShader("models/warzone/billboard/gkbjungletreenew24v2weeping");
				FOLIAGE_TREE_BILLBOARD_SHADER[8] = trap->R_RegisterShader("models/warzone/billboard/manfern");

				FOLIAGE_TREE_BILLBOARD_SIZE[0] = 118.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[1] = 118.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[2] = 204.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[3] = 112.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[4] = 163.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[5] = 163.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[6] = 183.6;
				FOLIAGE_TREE_BILLBOARD_SIZE[7] = 183.6;
				FOLIAGE_TREE_BILLBOARD_SIZE[8] = 153.0;

				FOLIAGE_TREE_RADIUS[0] = 24.0;
				FOLIAGE_TREE_RADIUS[1] = 24.0;
				FOLIAGE_TREE_RADIUS[2] = 72.0;
				FOLIAGE_TREE_RADIUS[3] = 52.0;
				FOLIAGE_TREE_RADIUS[4] = 52.0;
				FOLIAGE_TREE_RADIUS[5] = 52.0;
				FOLIAGE_TREE_RADIUS[6] = 72.0;
				FOLIAGE_TREE_RADIUS[7] = 72.0;
				FOLIAGE_TREE_RADIUS[8] = 72.0;

				FOLIAGE_TREE_ZOFFSET[0] = -64.0;
				FOLIAGE_TREE_ZOFFSET[1] = -64.0;
				FOLIAGE_TREE_ZOFFSET[2] = -64.0;
				FOLIAGE_TREE_ZOFFSET[3] = -64.0;
				FOLIAGE_TREE_ZOFFSET[4] = -64.0;
				FOLIAGE_TREE_ZOFFSET[5] = -64.0;
				FOLIAGE_TREE_ZOFFSET[6] = -64.0;
				FOLIAGE_TREE_ZOFFSET[7] = -64.0;
				FOLIAGE_TREE_ZOFFSET[8] = -64.0;
			}
			else // Default to new tropical...
			{
				FOLIAGE_GRASS_BILLBOARD_SHADER = trap->R_RegisterShader( "models/warzone/foliage/grasstropical" );

				for (int i = 0; i < MAX_PLANT_SHADERS; i++)
				{
					FOLIAGE_PLANT_SHADERS[i] = trap->R_RegisterShader(TropicalPlantsList[i]);
				}

				FOLIAGE_TREE_MODEL[0] = trap->R_RegisterModel( "models/warzone/trees/palm1.md3" );
				FOLIAGE_TREE_MODEL[1] = trap->R_RegisterModel( "models/warzone/trees/fanpalm2.md3" );
				FOLIAGE_TREE_MODEL[2] = trap->R_RegisterModel( "models/warzone/trees/giant1.md3" );
				FOLIAGE_TREE_MODEL[3] = trap->R_RegisterModel( "models/warzone/trees/anvilpalm1.md3" );
				FOLIAGE_TREE_MODEL[4] = trap->R_RegisterModel( "models/warzone/trees/willow.md3" );
				FOLIAGE_TREE_MODEL[5] = trap->R_RegisterModel( "models/warzone/trees/willow2.md3" );
				FOLIAGE_TREE_MODEL[6] = trap->R_RegisterModel( "models/warzone/trees/willow3.md3" );
				FOLIAGE_TREE_MODEL[7] = trap->R_RegisterModel( "models/warzone/trees/willow4.md3" );
				FOLIAGE_TREE_MODEL[8] = trap->R_RegisterModel( "models/warzone/trees/manfern.md3" );

				FOLIAGE_TREE_BILLBOARD_SHADER[0] = trap->R_RegisterShader("models/warzone/billboard/palm1");
				FOLIAGE_TREE_BILLBOARD_SHADER[1] = trap->R_RegisterShader("models/warzone/billboard/fanpalm2");
				FOLIAGE_TREE_BILLBOARD_SHADER[2] = trap->R_RegisterShader("models/warzone/billboard/giant1");
				FOLIAGE_TREE_BILLBOARD_SHADER[3] = trap->R_RegisterShader("models/warzone/billboard/anvilpalm1");
				FOLIAGE_TREE_BILLBOARD_SHADER[4] = trap->R_RegisterShader("models/warzone/billboard/willow");
				FOLIAGE_TREE_BILLBOARD_SHADER[5] = trap->R_RegisterShader("models/warzone/billboard/willow2");
				FOLIAGE_TREE_BILLBOARD_SHADER[6] = trap->R_RegisterShader("models/warzone/billboard/willow3");
				FOLIAGE_TREE_BILLBOARD_SHADER[7] = trap->R_RegisterShader("models/warzone/billboard/willow4");
				FOLIAGE_TREE_BILLBOARD_SHADER[8] = trap->R_RegisterShader("models/warzone/billboard/manfern");

				FOLIAGE_TREE_BILLBOARD_SIZE[0] = 128.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[1] = 118.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[2] = 204.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[3] = 112.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[4] = 183.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[5] = 183.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[6] = 203.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[7] = 203.0;
				FOLIAGE_TREE_BILLBOARD_SIZE[8] = 153.0;

				FOLIAGE_TREE_RADIUS[0] = 24.0;
				FOLIAGE_TREE_RADIUS[1] = 24.0;
				FOLIAGE_TREE_RADIUS[2] = 72.0;
				FOLIAGE_TREE_RADIUS[3] = 52.0;
				FOLIAGE_TREE_RADIUS[4] = 38.0;
				FOLIAGE_TREE_RADIUS[5] = 38.0;
				FOLIAGE_TREE_RADIUS[6] = 38.0;
				FOLIAGE_TREE_RADIUS[7] = 38.0;
				FOLIAGE_TREE_RADIUS[8] = 72.0;

				FOLIAGE_TREE_ZOFFSET[0] = -64.0;
				FOLIAGE_TREE_ZOFFSET[1] = -64.0;
				FOLIAGE_TREE_ZOFFSET[2] = -64.0;
				FOLIAGE_TREE_ZOFFSET[3] = -64.0;
				FOLIAGE_TREE_ZOFFSET[4] = -64.0;
				FOLIAGE_TREE_ZOFFSET[5] = -64.0;
				FOLIAGE_TREE_ZOFFSET[6] = -64.0;
				FOLIAGE_TREE_ZOFFSET[7] = -64.0;
				FOLIAGE_TREE_ZOFFSET[8] = -64.0;
			}
		}

		FOLIAGE_Check_CVar_Change();

		vec3_t viewOrg, viewAngles;

		VectorCopy(cg.refdef.vieworg, viewOrg);
		VectorCopy(cg.refdef.viewangles, viewAngles);

		FOLIAGE_Calc_In_Range_Areas();

		for (int CURRENT_AREA = 0; CURRENT_AREA < IN_RANGE_TREE_AREAS_LIST_COUNT; CURRENT_AREA++)
		{// Draw trees first...
			int CURRENT_AREA_ID = IN_RANGE_TREE_AREAS_LIST[CURRENT_AREA];

			for (int spot = 0; spot < FOLIAGE_AREAS_TREES_LIST_COUNT[CURRENT_AREA_ID]; spot++)
			{
				FOLIAGE_AddToScreen( FOLIAGE_AREAS_TREES_LIST[CURRENT_AREA_ID][spot], FOLIAGE_PASS_TREE );
			}
		}

		for (int CURRENT_AREA = 0; CURRENT_AREA < IN_RANGE_AREAS_LIST_COUNT; CURRENT_AREA++)
		{
			int CURRENT_AREA_ID = IN_RANGE_AREAS_LIST[CURRENT_AREA];

			for (int spot = 0; spot < FOLIAGE_AREAS_TREES_LIST_COUNT[CURRENT_AREA_ID]; spot++)
			{// Draw close trees second...
				if (FOLIAGE_TREE_SELECTION[FOLIAGE_AREAS_TREES_LIST[CURRENT_AREA_ID][spot]] > 0)
					FOLIAGE_AddToScreen( FOLIAGE_AREAS_TREES_LIST[CURRENT_AREA_ID][spot], FOLIAGE_PASS_CLOSETREE );
			}
		}

		for (int CURRENT_AREA = 0; CURRENT_AREA < IN_RANGE_AREAS_LIST_COUNT; CURRENT_AREA++)
		{// Draw grass list...
			int CURRENT_AREA_ID = IN_RANGE_AREAS_LIST[CURRENT_AREA];

			for (int spot = 0; spot < FOLIAGE_AREAS_LIST_COUNT[CURRENT_AREA_ID]; spot++)
			{
				if (FOLIAGE_TREE_SELECTION[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]] <= 0)
					FOLIAGE_AddToScreen( FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot], FOLIAGE_PASS_GRASS );
			}
		}

		for (int CURRENT_AREA = 0; CURRENT_AREA < IN_RANGE_AREAS_LIST_COUNT; CURRENT_AREA++)
		{// Draw plants last...
			int CURRENT_AREA_ID = IN_RANGE_AREAS_LIST[CURRENT_AREA];

			for (int spot = 0; spot < FOLIAGE_AREAS_LIST_COUNT[CURRENT_AREA_ID]; spot++)
			{
				if (FOLIAGE_TREE_SELECTION[FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot]] <= 0)
					FOLIAGE_AddToScreen( FOLIAGE_AREAS_LIST[CURRENT_AREA_ID][spot], FOLIAGE_PASS_PLANT );
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

	int GENFOLIAGE_ALLOW_MATERIAL = -1;

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

		if (GENFOLIAGE_ALLOW_MATERIAL >= 0 && materialType == GENFOLIAGE_ALLOW_MATERIAL) return qtrue;

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

	qboolean FOLIAGE_CheckFoliageAlready(vec3_t pos, float density)
	{
		int areaNum = FOLIAGE_AreaNumForOrg( pos );

		for (int spot = 0; spot < FOLIAGE_AREAS_LIST_COUNT[areaNum]; spot++)
		{
			int THIS_FOLIAGE = FOLIAGE_AREAS_LIST[areaNum][spot];

			if (Distance/*Horizontal*/(pos, FOLIAGE_POSITIONS[THIS_FOLIAGE]) <= density)
				return qtrue;
		}

		return qfalse;
	}

#define MAX_SLOPE 46.0

	qboolean FOLIAGE_CheckSlope(vec3_t normal)
	{
		vec3_t slopeangles;
		vectoangles( normal, slopeangles );

		float pitch = slopeangles[0];
	
		if (pitch > 180)
			pitch -= 360;

		if (pitch < -180)
			pitch += 360;

		pitch += 90.0f;

		if (pitch > MAX_SLOPE || pitch < -MAX_SLOPE)
			return qfalse;

		return qtrue;
	}
	
	qboolean FOLIAGE_CheckSlopeChange(vec3_t normal, vec3_t normal2)
	{
		/*
		vec3_t slopeDiff;

		slopeDiff[0] = normal[0] - normal2[0];
		slopeDiff[1] = normal[1] - normal2[1];
		slopeDiff[2] = normal[2] - normal2[2];

		if (slopeDiff[0] > GRASS_SLOPE_MAX_DIFF) return qfalse;
		if (slopeDiff[0] < -GRASS_SLOPE_MAX_DIFF) return qfalse;

		if (slopeDiff[1] > GRASS_SLOPE_MAX_DIFF) return qfalse;
		if (slopeDiff[1] < -GRASS_SLOPE_MAX_DIFF) return qfalse;

		if (slopeDiff[2] > GRASS_SLOPE_MAX_DIFF) return qfalse;
		if (slopeDiff[2] < -GRASS_SLOPE_MAX_DIFF) return qfalse;
		*/

		vec3_t slopeAngles1, slopeAngles2;

		vectoangles( normal, slopeAngles1 );

		float pitch1 = slopeAngles1[0];
	
		if (pitch1 > 180)
			pitch1 -= 360;

		if (pitch1 < -180)
			pitch1 += 360;

		pitch1 += 90.0f;

		vectoangles( normal2, slopeAngles2 );

		float pitch2 = slopeAngles2[0];
	
		if (pitch2 > 180)
			pitch2 -= 360;

		if (pitch2 < -180)
			pitch2 += 360;

		pitch2 += 90.0f;

		if (pitch1 - pitch2 > GRASS_SLOPE_MAX_DIFF || pitch1 - pitch2 < -GRASS_SLOPE_MAX_DIFF)
			return qfalse;

		return qtrue;
	}

	qboolean FOLIAGE_CheckSlopesAround(vec3_t groundpos, vec3_t slope, float scale)
	{
		trace_t		tr;
		vec3_t		pos, down;
		vec3_t		pos2, down2;

		VectorCopy(groundpos, pos);
		pos[2] += 96.0;
		VectorCopy(groundpos, down);
		down[2] -= 96.0;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] += (GRASS_MODEL_WIDTH * scale);
		down2[0] += (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (tr.fraction == 1.0) return qfalse; // Didn't hit anything... Started trace in floor???
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (!FOLIAGE_CheckSlope(tr.plane.normal)) return qfalse;
		if (!FOLIAGE_CheckSlopeChange(slope, tr.plane.normal)) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] -= (GRASS_MODEL_WIDTH * scale);
		down2[0] -= (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (tr.fraction == 1.0) return qfalse; // Didn't hit anything... Started trace in floor???
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (!FOLIAGE_CheckSlope(tr.plane.normal)) return qfalse;
		if (!FOLIAGE_CheckSlopeChange(slope, tr.plane.normal)) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[1] += (GRASS_MODEL_WIDTH * scale);
		down2[1] += (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (tr.fraction == 1.0) return qfalse; // Didn't hit anything... Started trace in floor???
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (!FOLIAGE_CheckSlope(tr.plane.normal)) return qfalse;
		if (!FOLIAGE_CheckSlopeChange(slope, tr.plane.normal)) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[1] -= (GRASS_MODEL_WIDTH * scale);
		down2[1] -= (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (tr.fraction == 1.0) return qfalse; // Didn't hit anything... Started trace in floor???
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (!FOLIAGE_CheckSlope(tr.plane.normal)) return qfalse;
		if (!FOLIAGE_CheckSlopeChange(slope, tr.plane.normal)) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] += (GRASS_MODEL_WIDTH * scale);
		pos2[1] += (GRASS_MODEL_WIDTH * scale);
		down2[0] += (GRASS_MODEL_WIDTH * scale);
		down2[1] += (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (tr.fraction == 1.0) return qfalse; // Didn't hit anything... Started trace in floor???
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (!FOLIAGE_CheckSlope(tr.plane.normal)) return qfalse;
		if (!FOLIAGE_CheckSlopeChange(slope, tr.plane.normal)) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] += (GRASS_MODEL_WIDTH * scale);
		pos2[1] -= (GRASS_MODEL_WIDTH * scale);
		down2[0] += (GRASS_MODEL_WIDTH * scale);
		down2[1] -= (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (tr.fraction == 1.0) return qfalse; // Didn't hit anything... Started trace in floor???
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (!FOLIAGE_CheckSlope(tr.plane.normal)) return qfalse;
		if (!FOLIAGE_CheckSlopeChange(slope, tr.plane.normal)) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] -= (GRASS_MODEL_WIDTH * scale);
		pos2[1] += (GRASS_MODEL_WIDTH * scale);
		down2[0] -= (GRASS_MODEL_WIDTH * scale);
		down2[1] += (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (tr.fraction == 1.0) return qfalse; // Didn't hit anything... Started trace in floor???
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (!FOLIAGE_CheckSlope(tr.plane.normal)) return qfalse;
		if (!FOLIAGE_CheckSlopeChange(slope, tr.plane.normal)) return qfalse;

		VectorCopy(pos, pos2);
		VectorCopy(down, down2);
		pos2[0] -= (GRASS_MODEL_WIDTH * scale);
		pos2[1] -= (GRASS_MODEL_WIDTH * scale);
		down2[0] -= (GRASS_MODEL_WIDTH * scale);
		down2[1] -= (GRASS_MODEL_WIDTH * scale);

		CG_Trace( &tr, pos2, NULL, NULL, down2, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

		// Slope too different...
		if (tr.fraction == 1.0) return qfalse; // Didn't hit anything... Started trace in floor???
		if (!MaterialIsValidForGrass(int(tr.surfaceFlags & MATERIAL_MASK))) return qfalse;
		if (!FOLIAGE_CheckSlope(tr.plane.normal)) return qfalse;
		if (!FOLIAGE_CheckSlopeChange(slope, tr.plane.normal)) return qfalse;

		return qtrue;
	}

	void FOLIAGE_GenerateFoliage_Real ( float density, int num_clusters, float tree_density, int num_dense_areas, qboolean ADD_MORE, float check_density, qboolean ADD_SLOPES )
	{
		int				i;
		vec3_t			vec;

		if (FOLIAGE_IgnoreFoliageOnMap())
		{// Ignore this map... We know we don't need grass here...
			FOLIAGE_NUM_POSITIONS = 0;
			FOLIAGE_LOADED = qtrue;
			return;
		}

		if (!ADD_MORE && !ADD_SLOPES)
		{
			FOLIAGE_NUM_POSITIONS = 0;

			for (i = 0; i < FOLIAGE_MAX_FOLIAGES; i++)
			{// Make sure our list is empty...
				FOLIAGE_PLANT_SELECTION[i] = 0;
				FOLIAGE_TREE_SELECTION[i] = 0;
			}
		}

		i = 0;


		float		startx = -131072, starty = -131072, startz = -131072;
		int			grassSpotCount = 0;
		vec3_t		*grassSpotList;
		float		*grassSpotScale;
		float		map_size, temp;
		vec3_t		mapMins, mapMaxs;
		int			start_time = trap->Milliseconds();
		int			update_timer = 0;
		clock_t		previous_time = 0;
		float		offsetY = 0.0;

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

		trap->Print( va( "^4*** ^3AUTO-FOLIAGE^4: ^5Generating foliage points. This could take a while... (Map size ^3%.2f^5)\n", map_size) );
		strcpy( task_string2, va("^5Generating foliage points. This could take a while... (Map size ^3%.2f^5)", map_size) );

		trap->Print( va( "^4*** ^3AUTO-FOLIAGE^4: ^5Finding foliage points...\n") );
		strcpy( task_string3, va("^5Finding foliage points...") );

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

		vec3_t	MAP_SCALE;
		vec3_t	CLUSTER_SPOTS[FOLIAGE_AREA_MAX/8];
		float	CLUSTER_SPOTS_SIZES[FOLIAGE_AREA_MAX/8];

		MAP_SCALE[0] = mapMaxs[0] - mapMins[0];
		MAP_SCALE[1] = mapMaxs[1] - mapMins[1];
		MAP_SCALE[2] = 0;

		if (num_clusters > 0)
		{
			for (int d = 0; d < num_clusters; d++)
			{
				int NUM_FAILS = 0;

				CLUSTER_SPOTS_SIZES[d] = irand(128, 384);

				for (int d2 = 0; d2 < d; d2++)
				{
					vec3_t spot;

					while (1)
					{
						spot[0] = mapMins[0] + (irand(0, MAP_SCALE[0]/2)+irand(0, MAP_SCALE[0]/2));
						spot[1] = mapMins[1] + (irand(0, MAP_SCALE[1]/2)+irand(0, MAP_SCALE[1]/2));
						spot[2] = 0;

						if (DistanceHorizontal(CLUSTER_SPOTS[d], spot) < CLUSTER_SPOTS_SIZES[d])
						{
							NUM_FAILS++;
							continue;
						}

						// Ok, not in range of another one, add this spot to the list...
						break;
					}

					CLUSTER_SPOTS[d][0] = spot[0];
					CLUSTER_SPOTS[d][1] = spot[1];
					CLUSTER_SPOTS[d][2] = 0;
				}

				if (NUM_FAILS > 50) break; // Failed 50 times in a row, assume we filled the map...
			}
		}

		float yoff = density * 1.25;

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

					//if(omp_get_thread_num() == 0)
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

					CG_Trace( &tr, pos, NULL, NULL, down, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME );

					if (tr.endpos[2] <= mapMins[2])
					{// Went off map...
						break;
					}

					if ( tr.surfaceFlags & SURF_SKY )
					{// Sky...
						continue;
					}

					if ( tr.contents & CONTENTS_WATER )
					{// Anything below here is underwater...
						break;
					}

					if ( tr.contents & CONTENTS_LAVA )
					{// Anything below here is under lava...
						break;
					}

					if ( tr.surfaceFlags & SURF_NODRAW )
					{// don't generate a drawsurface at all
						continue;
					}

					if (ADD_SLOPES)
					{
						vec3_t	slopeangles;
						float	pitch = 0;

						vectoangles( tr.plane.normal, slopeangles );

						pitch = slopeangles[0];

						if (pitch > 180)
							pitch -= 360;

						if (pitch < -180)
							pitch += 360;

						pitch += 90.0f;

						if (pitch < 32.0 && pitch > -32.0)
						{// This is not enough of a slope...
							continue;
						}
					}

					if ((ADD_MORE || ADD_SLOPES) && check_density > 0 && FOLIAGE_CheckFoliageAlready(tr.endpos, check_density))
					{// Already a foliage here...
						continue;
					}

					if (MaterialIsValidForGrass((tr.surfaceFlags & MATERIAL_MASK)))
					{
						// Look around here for a different slope angle... Cull if found...
						for (float scale = 1.00; scale >= 0.05 && scale >= cg_foliageMinFoliageScale.value; scale -= 0.05)
						{
							if (ADD_SLOPES && scale < 1.0) break;

							if (FOLIAGE_CheckSlopesAround(tr.endpos, tr.plane.normal, scale))
							{
//#pragma omp critical (__ADD_TEMP_NODE__)
								{
									if (num_clusters > 0)
									{
										int		THIS_CLUSTER = -1;
										float	THIS_CLUSTER_SIZE_MULT = 1.0;
										float	closestDist = 99999;
										int		closest = -1;

										for (int spot = 0; spot < num_clusters; spot++)
										{
											float spotDist = DistanceHorizontal(CLUSTER_SPOTS[spot], pos);

											if (spotDist < CLUSTER_SPOTS_SIZES[spot]+irand(0,256) /* edge randomization */ && spotDist < closestDist)
											{
												closestDist = spotDist;
												closest = spot;
											}
										}

										if (closest >= 0)
										{
											THIS_CLUSTER = closest;

											if ((closestDist / CLUSTER_SPOTS_SIZES[THIS_CLUSTER]) > 1.0)
												THIS_CLUSTER_SIZE_MULT = (float)irand(10,40) / 100.0; /* edge randomization */
											else
												THIS_CLUSTER_SIZE_MULT = 1.0 - (closestDist / CLUSTER_SPOTS_SIZES[THIS_CLUSTER]);
										}

										if (THIS_CLUSTER >= 0)
										{
											grassSpotScale[grassSpotCount] = scale * THIS_CLUSTER_SIZE_MULT;
										}
										else if (irand(0,(int)(density*2.0)) == 0)
										{
											grassSpotScale[grassSpotCount] = -1.0;
										}
										else
										{
											break;
										}
									}
									else
									{
										grassSpotScale[grassSpotCount] = scale;
									}

									sprintf(last_node_added_string, "^5Adding foliage point ^3%i ^5at ^7%f %f %f^5.", grassSpotCount, tr.endpos[0], tr.endpos[1], tr.endpos[2]+8);

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

		if (!ADD_MORE && !ADD_SLOPES) 
		{
			FOLIAGE_NUM_POSITIONS = 0;

			for (i = 0; i < FOLIAGE_MAX_FOLIAGES; i++)
			{// Make sure our list is empty...
				FOLIAGE_PLANT_SELECTION[i] = 0;
				FOLIAGE_TREE_SELECTION[i] = 0;
			}
		}

		if (grassSpotCount > 0)
		{// Ok, we have spots, copy and set up foliage types/scales/angles, and save to file...
			vec3_t	DENSE_SPOTS[FOLIAGE_AREA_MAX/8];

			for (int d = 0; d < num_dense_areas; d++)
			{
				int NUM_FAILS = 0;

				for (int d2 = 0; d2 < d; d2++)
				{
					vec3_t spot;

					while (1)
					{
						spot[0] = mapMins[0] + (irand(0, MAP_SCALE[0]/2)+irand(0, MAP_SCALE[0]/2));
						spot[1] = mapMins[1] + (irand(0, MAP_SCALE[1]/2)+irand(0, MAP_SCALE[1]/2));
						spot[2] = 0;

						if (DistanceHorizontal(DENSE_SPOTS[d], spot) < 4096)
						{
							NUM_FAILS++;
							continue;
						}

						// Ok, not in range of another one, add this spot to the list...
						break;
					}

					DENSE_SPOTS[d][0] = spot[0];
					DENSE_SPOTS[d][1] = spot[1];
					DENSE_SPOTS[d][2] = 0;
				}

				if (NUM_FAILS > 50) break; // Failed 50 times in a row, assume we filled the map...
			}

			for (i = 0; i < grassSpotCount; i++)
			{
				qboolean DO_TREE = qfalse;

				vec[0] = grassSpotList[i][0];
				vec[1] = grassSpotList[i][1];
				vec[2] = grassSpotList[i][2];

				float USE_TREE_DENSITY = tree_density;

				for (int d = 0; d < num_dense_areas; d++)
				{
					if (DistanceHorizontal(DENSE_SPOTS[d], FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS]) <= 4096)
					{
						USE_TREE_DENSITY = tree_density / 6;//sqrt(tree_density);
						break;
					}
				}

				FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
				FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = 0.0f;
				if (num_clusters <= 0)
				{
					if (grassSpotScale[i] > 0)
					{
						FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = grassSpotScale[i];

						if (tree_density > 0 
							&& irand(0, (int)USE_TREE_DENSITY) >= (int)USE_TREE_DENSITY 
							&& RoofHeightAt(vec) - vec[2] > 1024.0)
						{
							DO_TREE = qtrue;
						}
					}
				}
				else if (grassSpotScale[i] > 0)
				{
					FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = grassSpotScale[i];
				}
				else
				{// Only tree allowed here...
					if (!ADD_SLOPES 
						&& tree_density > 0 
						&& irand(0, (int)USE_TREE_DENSITY) >= (int)USE_TREE_DENSITY 
						&& RoofHeightAt(vec) - vec[2] > 1024.0)
					{
						DO_TREE = qtrue;
					}
					else
					{
						continue;
					}
				}

				FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = 0;
				FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = 0.0f;
				FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = 0.0f;

				VectorCopy(vec, FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS]);
				FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS][2] -= 18.0;

				FOLIAGE_PLANT_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
				
				if (FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] == 0)
					FOLIAGE_PLANT_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(100*cg_foliageMinFoliageScale.value,100) / 100.0);

				if (DO_TREE)
				{// Add tree... 
					FOLIAGE_TREE_SELECTION[FOLIAGE_NUM_POSITIONS] = irand(1, NUM_TREE_TYPES);
					FOLIAGE_TREE_ANGLES[FOLIAGE_NUM_POSITIONS] = (int)(random() * 180);
					FOLIAGE_TREE_SCALE[FOLIAGE_NUM_POSITIONS] = (float)((float)irand(65,150) / 100.0);
				}
				else if (irand(0, 255) <= 0)
				{// Add plant... 1 in every 256 positions...
					FOLIAGE_PLANT_SELECTION[FOLIAGE_NUM_POSITIONS] = irand(1,MAX_PLANT_SHADERS-1);
				}

				trace_t tr;
				vec3_t	up, down;
				VectorCopy(FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS], up);
				up[2]+=128;
				VectorCopy(FOLIAGE_POSITIONS[FOLIAGE_NUM_POSITIONS], down);
				down[2]-=128;
				CG_Trace(&tr, up, NULL, NULL, down, -1, MASK_SOLID);
				VectorCopy(tr.plane.normal, FOLIAGE_NORMALS[FOLIAGE_NUM_POSITIONS]);

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

	extern qboolean CPU_CHECKED;
	extern int UQ_Get_CPU_Info( void );

	void FOLIAGE_FoliageRescale ( void )
	{
		int			update_timer = 0;
		clock_t		previous_time = 0;
		vec3_t		mapMins, mapMaxs;
		float		temp;

		AIMod_GetMapBounts();

		if (!cg.mapcoordsValid)
		{
			trap->Print("^4*** ^3AUTO-FOLIAGE^4: ^7Map Coordinates are invalid. Can not use auto-waypointer!\n");
			return;
		}

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

		previous_time = clock();
		aw_stage_start_time = clock();
		aw_percent_complete = 0;

		trap->Print( va( "^4*** ^3AUTO-FOLIAGE^4: ^5Map bounds are ^3%.2f %.2f %.2f ^5to ^3%.2f %.2f %.2f^5.\n", mapMins[0], mapMins[1], mapMins[2], mapMaxs[0], mapMaxs[1], mapMaxs[2]) );
		strcpy( task_string1, va("^5Map bounds are ^3%.2f %.2f %.2f ^7to ^3%.2f %.2f %.2f^5.", mapMins[0], mapMins[1], mapMins[2], mapMaxs[0], mapMaxs[1], mapMaxs[2]) );
		trap->UpdateScreen();

		trap->Print( va( "^4*** ^3AUTO-FOLIAGE^4: ^5Rescaling foliage points. This could take a while...\n") );
		strcpy( task_string2, va("^5Rescaling foliage points. This could take a while...") );
		trap->UpdateScreen();

		trap->Print( va( "^4*** ^3AUTO-FOLIAGE^4: ^5Rescaling foliage points...\n") );
		strcpy( task_string3, va("^5Rescaling foliage points...") );
		trap->UpdateScreen();

		trap->UpdateScreen();

		trap->S_Shutup(qtrue);

		int NUM_RESCALED = 0;

		for (int i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
		{// Check current list...
			vec3_t		pos, down;
			trace_t		tr;

			aw_percent_complete = (float)((float)i / (float)FOLIAGE_NUM_POSITIONS) * 100.0;

			if (clock() - previous_time > 500) // update display every 500ms...
			{
				previous_time = clock();
				trap->UpdateScreen();
			}

			VectorCopy(FOLIAGE_POSITIONS[i], pos);
			pos[2] += 128.0;
			VectorCopy(pos, down);
			down[2] = mapMins[2];

			CG_Trace( &tr, pos, NULL, NULL, down, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_WATER );

			// Look around here for a different slope angle... Cull if found...
			for (float scale = 1.00; scale > FOLIAGE_PLANT_SCALE[i] && scale >= cg_foliageMinFoliageScale.value; scale -= 0.05)
			{
				qboolean	FOUND = qfalse;

				if (FOLIAGE_CheckSlopesAround(tr.endpos, tr.plane.normal, scale))
				{
					FOLIAGE_PLANT_SCALE[i] = scale;

					NUM_RESCALED++;

					sprintf(last_node_added_string, "^5Plant point ^3%i ^5at ^7%f %f %f^5 rescaled.", i, FOLIAGE_POSITIONS[i][0], FOLIAGE_POSITIONS[i][1], FOLIAGE_POSITIONS[i][2]);

					FOUND = qtrue;
				}
				else
				{
					FOLIAGE_PLANT_SCALE[i] = 0;
				}

				if (FOUND) break;
			}
		}

		trap->S_Shutup(qfalse);

		aw_percent_complete = 0.0f;

		trap->Print( "^1*** ^3%s^5: Successfully rescaled %i grass points...\n", GAME_VERSION, FOLIAGE_NUM_POSITIONS );

		// Save the generated info to a file for next time...
		FOLIAGE_SaveFoliagePositions();
	}

	void FOLIAGE_FoliageReplant ( int plantPercentage )
	{
		int NUM_REPLACED = 0;

		for (int i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
		{// Check current list...
			FOLIAGE_PLANT_SELECTION[i] = 0;

			if (FOLIAGE_TREE_SELECTION[i] <= 0)
			{
				if (irand(0,100) <= plantPercentage)
				{// Replace...
					FOLIAGE_PLANT_SELECTION[i] = irand(1,MAX_PLANT_SHADERS-1);
					NUM_REPLACED++;
				}
				else
				{
					FOLIAGE_PLANT_SELECTION[i] = 0;
					NUM_REPLACED++;
				}
			}
		}

		trap->Print( "^1*** ^3%s^5: Successfully replaced %i trees...\n", GAME_VERSION, NUM_REPLACED );

		// Save the generated info to a file for next time...
		FOLIAGE_SaveFoliagePositions();
	}

	void FOLIAGE_FoliageRetree ( void )
	{
		int NUM_REPLACED = 0;

		for (int i = 0; i < FOLIAGE_NUM_POSITIONS; i++)
		{// Check current list...
			if (FOLIAGE_TREE_SELECTION[i] > 0)
			{// Tree here... Replace...
				FOLIAGE_TREE_SELECTION[i] = irand(1, NUM_TREE_TYPES);
				NUM_REPLACED++;
			}
		}

		trap->Print( "^1*** ^3%s^5: Successfully replaced %i trees...\n", GAME_VERSION, NUM_REPLACED );

		// Save the generated info to a file for next time...
		FOLIAGE_SaveFoliagePositions();
	}

	void FOLIAGE_GenerateFoliage ( void )
	{
		char	str[MAX_TOKEN_CHARS];

		// UQ1: Check if we have an SSE CPU.. It can speed up our memory allocation by a lot!
		if (!CPU_CHECKED)
			UQ_Get_CPU_Info();

		GENFOLIAGE_ALLOW_MATERIAL = -1;

		if ( trap->Cmd_Argc() < 2 )
		{
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Usage:\n" );
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3/genfoliage <method> <density> <num_clusters> <tree_density> <num_tree_dense_areas> <check_density>^5. Use num_clusters 0 for even spread.\n" );
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^5Available methods are:\n" );
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"standard\" ^5- Create new foliage map.\n");
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"concrete\" ^5- Create new foliage map. Allow concrete material.\n");
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"sand\" ^5- Create new foliage map. Allow sand material.\n");
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"gravel\" ^5- Create new foliage map. Allow gravel material.\n");
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"rock\" ^5- Create new foliage map. Allow rock material.\n");
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"snow\" ^5- Create new foliage map. Allow snow material.\n");
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"add\" ^5- Add more to current list of foliages. Allows <check_density> to check for another foliage before adding.\n");
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"addslopes\" ^5- Adds more grass/plants only (on slopes). Allows <check_density> to check for another foliage before adding.\n");
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"rescale\" ^5- Check and fix scale of current grasses/plants.\n");
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"replant\" ^5- Reselect all grasses/plants (for updating between versions).\n");
			trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3\"retree\" ^5- Reselect all tree types (for updating between versions).\n");
			trap->UpdateScreen();
			return;
		}

		trap->Cmd_Argv( 1, str, sizeof(str) );

		if ( Q_stricmp( str, "standard") == 0 )
		{
			if ( trap->Cmd_Argc() >= 2 )
			{// Override normal density...
				int dist = 48;
				int num_clusters = 0;
				int tree_dist = 48;
				int num_dense_areas = 0;

				trap->Cmd_Argv( 2, str, sizeof(str) );
				dist = atoi(str);

				if (dist <= 4)
				{// Fallback and warning...
					dist = 48;
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Warning: ^5Invalid density set (%i). Using default (%i)...\n", atoi(str), 48 );
				}

				trap->Cmd_Argv( 3, str, sizeof(str) );
				num_clusters = atoi(str);

				trap->Cmd_Argv( 4, str, sizeof(str) );
				tree_dist = atoi(str);

				trap->Cmd_Argv( 5, str, sizeof(str) );
				num_dense_areas = atoi(str);

				if (num_dense_areas > FOLIAGE_AREA_MAX/8)
				{
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Error: ^5Invalid num_dense_areas set (%i). Maximum is (%i)...\n", atoi(str), FOLIAGE_AREA_MAX/8 );
					return;
				}

				FOLIAGE_GenerateFoliage_Real((float)dist, num_clusters, (float)tree_dist, num_dense_areas, qfalse, 32.0, qfalse);
			}
			else
			{
				FOLIAGE_GenerateFoliage_Real(48.0, 0, 256.0, 0, qfalse, 32.0, qfalse);
			}
		}
		else if ( Q_stricmp( str, "concrete") == 0 )
		{
			GENFOLIAGE_ALLOW_MATERIAL = MATERIAL_CONCRETE;

			if ( trap->Cmd_Argc() >= 2 )
			{// Override normal density...
				int dist = 48;
				int num_clusters = 0;
				int tree_dist = 48;
				int num_dense_areas = 0;

				trap->Cmd_Argv( 2, str, sizeof(str) );
				dist = atoi(str);

				if (dist <= 4)
				{// Fallback and warning...
					dist = 48;
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Warning: ^5Invalid density set (%i). Using default (%i)...\n", atoi(str), 48 );
				}

				trap->Cmd_Argv( 3, str, sizeof(str) );
				num_clusters = atoi(str);

				trap->Cmd_Argv( 4, str, sizeof(str) );
				tree_dist = atoi(str);

				trap->Cmd_Argv( 5, str, sizeof(str) );
				num_dense_areas = atoi(str);

				if (num_dense_areas > FOLIAGE_AREA_MAX/8)
				{
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Error: ^5Invalid num_dense_areas set (%i). Maximum is (%i)...\n", atoi(str), FOLIAGE_AREA_MAX/8 );
					return;
				}

				FOLIAGE_GenerateFoliage_Real((float)dist, num_clusters, (float)tree_dist, num_dense_areas, qfalse, 32.0, qfalse);
			}
			else
			{
				FOLIAGE_GenerateFoliage_Real(48.0, 0, 256.0, 0, qfalse, 32.0, qfalse);
			}
		}
		else if ( Q_stricmp( str, "sand") == 0 )
		{
			GENFOLIAGE_ALLOW_MATERIAL = MATERIAL_SAND;

			if ( trap->Cmd_Argc() >= 2 )
			{// Override normal density...
				int dist = 48;
				int num_clusters = 0;
				int tree_dist = 48;
				int num_dense_areas = 0;

				trap->Cmd_Argv( 2, str, sizeof(str) );
				dist = atoi(str);

				if (dist <= 4)
				{// Fallback and warning...
					dist = 48;
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Warning: ^5Invalid density set (%i). Using default (%i)...\n", atoi(str), 48 );
				}

				trap->Cmd_Argv( 3, str, sizeof(str) );
				num_clusters = atoi(str);

				trap->Cmd_Argv( 4, str, sizeof(str) );
				tree_dist = atoi(str);

				trap->Cmd_Argv( 5, str, sizeof(str) );
				num_dense_areas = atoi(str);

				if (num_dense_areas > FOLIAGE_AREA_MAX/8)
				{
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Error: ^5Invalid num_dense_areas set (%i). Maximum is (%i)...\n", atoi(str), FOLIAGE_AREA_MAX/8 );
					return;
				}

				FOLIAGE_GenerateFoliage_Real((float)dist, num_clusters, (float)tree_dist, num_dense_areas, qfalse, 32.0, qfalse);
			}
			else
			{
				FOLIAGE_GenerateFoliage_Real(48.0, 0, 256.0, 0, qfalse, 32.0, qfalse);
			}
		}
		else if ( Q_stricmp( str, "rock") == 0 )
		{
			GENFOLIAGE_ALLOW_MATERIAL = MATERIAL_ROCK;

			if ( trap->Cmd_Argc() >= 2 )
			{// Override normal density...
				int dist = 48;
				int num_clusters = 0;
				int tree_dist = 48;
				int num_dense_areas = 0;

				trap->Cmd_Argv( 2, str, sizeof(str) );
				dist = atoi(str);

				if (dist <= 4)
				{// Fallback and warning...
					dist = 48;
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Warning: ^5Invalid density set (%i). Using default (%i)...\n", atoi(str), 48 );
				}

				trap->Cmd_Argv( 3, str, sizeof(str) );
				num_clusters = atoi(str);

				trap->Cmd_Argv( 4, str, sizeof(str) );
				tree_dist = atoi(str);

				trap->Cmd_Argv( 5, str, sizeof(str) );
				num_dense_areas = atoi(str);

				if (num_dense_areas > FOLIAGE_AREA_MAX/8)
				{
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Error: ^5Invalid num_dense_areas set (%i). Maximum is (%i)...\n", atoi(str), FOLIAGE_AREA_MAX/8 );
					return;
				}

				FOLIAGE_GenerateFoliage_Real((float)dist, num_clusters, (float)tree_dist, num_dense_areas, qfalse, 32.0, qfalse);
			}
			else
			{
				FOLIAGE_GenerateFoliage_Real(48.0, 0, 256.0, 0, qfalse, 32.0, qfalse);
			}
		}
		else if ( Q_stricmp( str, "gravel") == 0 )
		{
			GENFOLIAGE_ALLOW_MATERIAL = MATERIAL_GRAVEL;

			if ( trap->Cmd_Argc() >= 2 )
			{// Override normal density...
				int dist = 48;
				int num_clusters = 0;
				int tree_dist = 48;
				int num_dense_areas = 0;

				trap->Cmd_Argv( 2, str, sizeof(str) );
				dist = atoi(str);

				if (dist <= 4)
				{// Fallback and warning...
					dist = 48;
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Warning: ^5Invalid density set (%i). Using default (%i)...\n", atoi(str), 48 );
				}

				trap->Cmd_Argv( 3, str, sizeof(str) );
				num_clusters = atoi(str);

				trap->Cmd_Argv( 4, str, sizeof(str) );
				tree_dist = atoi(str);

				trap->Cmd_Argv( 5, str, sizeof(str) );
				num_dense_areas = atoi(str);

				if (num_dense_areas > FOLIAGE_AREA_MAX/8)
				{
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Error: ^5Invalid num_dense_areas set (%i). Maximum is (%i)...\n", atoi(str), FOLIAGE_AREA_MAX/8 );
					return;
				}

				FOLIAGE_GenerateFoliage_Real((float)dist, num_clusters, (float)tree_dist, num_dense_areas, qfalse, 32.0, qfalse);
			}
			else
			{
				FOLIAGE_GenerateFoliage_Real(48.0, 0, 256.0, 0, qfalse, 32.0, qfalse);
			}
		}
		else if ( Q_stricmp( str, "snow") == 0 )
		{
			GENFOLIAGE_ALLOW_MATERIAL = MATERIAL_SNOW;

			if ( trap->Cmd_Argc() >= 2 )
			{// Override normal density...
				int dist = 48;
				int num_clusters = 0;
				int tree_dist = 48;
				int num_dense_areas = 0;

				trap->Cmd_Argv( 2, str, sizeof(str) );
				dist = atoi(str);

				if (dist <= 4)
				{// Fallback and warning...
					dist = 48;
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Warning: ^5Invalid density set (%i). Using default (%i)...\n", atoi(str), 48 );
				}

				trap->Cmd_Argv( 3, str, sizeof(str) );
				num_clusters = atoi(str);

				trap->Cmd_Argv( 4, str, sizeof(str) );
				tree_dist = atoi(str);

				trap->Cmd_Argv( 5, str, sizeof(str) );
				num_dense_areas = atoi(str);

				if (num_dense_areas > FOLIAGE_AREA_MAX/8)
				{
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Error: ^5Invalid num_dense_areas set (%i). Maximum is (%i)...\n", atoi(str), FOLIAGE_AREA_MAX/8 );
					return;
				}

				FOLIAGE_GenerateFoliage_Real((float)dist, num_clusters, (float)tree_dist, num_dense_areas, qfalse, 32.0, qfalse);
			}
			else
			{
				FOLIAGE_GenerateFoliage_Real(48.0, 0, 256.0, 0, qfalse, 32.0, qfalse);
			}
		}
		else if ( Q_stricmp( str, "add") == 0 )
		{
			if ( trap->Cmd_Argc() >= 2 )
			{// Override normal density...
				int dist = 48;
				int num_clusters = 0;
				int tree_dist = 64;
				int num_dense_areas = 0;
				int	check_density = dist;

				trap->Cmd_Argv( 2, str, sizeof(str) );
				dist = atoi(str);

				if (dist <= 4)
				{// Fallback and warning...
					dist = 48;
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Warning: ^5Invalid density set (%i). Using default (%i)...\n", atoi(str), 48 );
				}

				check_density = dist;

				trap->Cmd_Argv( 3, str, sizeof(str) );
				num_clusters = atoi(str);

				trap->Cmd_Argv( 4, str, sizeof(str) );
				tree_dist = atoi(str);

				trap->Cmd_Argv( 5, str, sizeof(str) );
				num_dense_areas = atoi(str);

				if (num_dense_areas > FOLIAGE_AREA_MAX/8)
				{
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Error: ^5Invalid num_dense_areas set (%i). Maximum is (%i)...\n", atoi(str), FOLIAGE_AREA_MAX/8 );
					return;
				}

				trap->Cmd_Argv( 6, str, sizeof(str) );
				check_density = atoi(str);

				FOLIAGE_GenerateFoliage_Real((float)dist, num_clusters, (float)tree_dist, num_dense_areas, qtrue, check_density, qfalse);
			}
			else
			{
				FOLIAGE_GenerateFoliage_Real(48.0, 0, 256.0, 0, qtrue, 32.0, qfalse);
			}
		}
		else if ( Q_stricmp( str, "addslopes") == 0 )
		{
			if ( trap->Cmd_Argc() >= 2 )
			{// Override normal density...
				int dist = 48;
				int num_clusters = 0;
				int tree_dist = 64;
				int num_dense_areas = 0;
				int	check_density = dist;

				trap->Cmd_Argv( 2, str, sizeof(str) );
				dist = atoi(str);

				if (dist <= 4)
				{// Fallback and warning...
					dist = 32;
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Warning: ^5Invalid density set (%i). Using default (%i)...\n", atoi(str), 48 );
				}

				check_density = dist;

				trap->Cmd_Argv( 3, str, sizeof(str) );
				num_clusters = atoi(str);

				trap->Cmd_Argv( 4, str, sizeof(str) );
				tree_dist = atoi(str);

				trap->Cmd_Argv( 5, str, sizeof(str) );
				num_dense_areas = atoi(str);

				if (num_dense_areas > FOLIAGE_AREA_MAX/8)
				{
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Error: ^5Invalid num_dense_areas set (%i). Maximum is (%i)...\n", atoi(str), FOLIAGE_AREA_MAX/8 );
					return;
				}

				trap->Cmd_Argv( 6, str, sizeof(str) );
				check_density = atoi(str);

				FOLIAGE_GenerateFoliage_Real((float)dist, num_clusters, (float)tree_dist, num_dense_areas, qfalse, check_density, qtrue);
			}
			else
			{
				FOLIAGE_GenerateFoliage_Real(48.0, 0, 256.0, 0, qfalse, 48.0, qtrue);
			}
		}
		else if ( Q_stricmp( str, "rescale") == 0 )
		{
			FOLIAGE_FoliageRescale();
		}
		else if ( Q_stricmp( str, "replant") == 0 )
		{
			if ( trap->Cmd_Argc() >= 2 )
			{// Override normal density...
				int plantPercentage = 20;

				trap->Cmd_Argv( 2, str, sizeof(str) );
				plantPercentage = atoi(str);

				if (plantPercentage <= 0)
				{// Fallback and warning...
					plantPercentage = 20;
					trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Warning: ^5Invalid plant percentage set (%i). Using default (%i)...\n", atoi(str), plantPercentage );
				}
			
				FOLIAGE_FoliageReplant(plantPercentage);
			}
			else
			{
				trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^7Usage:\n" );
				trap->Print( "^4*** ^3AUTO-FOLIAGE^4: ^3/genfoliage replant <plantPercent>^5. Use plantPercent 0 for default.\n" );
			}
		}
		else if ( Q_stricmp( str, "retree") == 0 )
		{
			FOLIAGE_FoliageRetree();
		}
	}
}
