#pragma once

// Filename:-	bg_weapons.h
//
// This crosses both client and server.  It could all be crammed into bg_public, but isolation of this type of data is best.

#ifndef WEAPONS_H
#define WEAPONS_H

typedef enum {
	WP_NONE,

	// =================================================================
	//
	// Melee weapons...
	//
	// =================================================================

	// START: Do not change anything between these 2 comments...
	WP_STUN_BATON,
	WP_FIRST_USEABLE = WP_STUN_BATON,
	// END: ^^^ Do not change...

	WP_MELEE,
	// Add new melee weapons here...



	// START: Do not change anything between these 2 comments...
	WP_SABER,
	WP_NUM_MELEE_WEAPONS = WP_SABER,
	// END: ^^^ Do not change...

	// =================================================================
	//
	// Guns...
	//
	// =================================================================

	WP_BRYAR_PISTOL,
	WP_BRYAR_OLD,
	WP_BLASTER,
	WP_DISRUPTOR,
	WP_BOWCASTER,
	WP_REPEATER,
	WP_DEMP2,
	WP_FLECHETTE,
	WP_CONCUSSION,
	// Add new guns here...
	WP_A280,
	WP_DC15,
	WP_WESTARM5,
	WP_T21,
	WP_EE3,
	WP_DC_15S_CLONE_PISTOL,
	WP_DLT20A,
	WP_CLONERIFLE,
	WP_WESTER_PISTOL,
	WP_ELG_3A,
	WP_S5_PISTOL,
	WP_Z6_BLASTER_CANON,
	WP_WOOKIE_BOWCASTER,
	WP_WOOKIES_PISTOL,
	WP_CLONE_BLASTER,
	WP_DC15_EXT,
	WP_E60_ROCKET_LAUNCHER,
	WP_CW_ROCKET_LAUNCHER,
	WP_TESTGUN,
	WP_FRAG_GRENADE,
	WP_FRAG_GRENADE_OLD,
	WP_DC_17_CLONE_PISTOL,
	WP_SPOTING_BLASTER,
	WP_A200_ACP_BATTLERIFLE,
	WP_A200_ACP_PISTOL,
	WP_ACP_ARRAYGUN,
	WP_ACP_SNIPER_RIFLE,
	WP_ARC_CASTER_IMPERIAL,
	WP_BOWCASTER_CLASSIC,
	WP_WOOKIE_BOWCASTER_SCOPE,
	/*
	WP_ARC_CASTER_MK1,
	
	
	WP_BRYAR_CARBINE,
	WP_BRYAR_RIFLE,*/



	// START: Do not change anything between these 2 comments...
	WP_ROCKET_LAUNCHER,
	WP_NUM_GUNS = WP_ROCKET_LAUNCHER,
	// END: ^^^ Do not change...

	// =================================================================
	//
	// Grenades and explosives here...
	//
	// =================================================================

	WP_THERMAL,
	WP_TRIP_MINE,
	// Add new grenades/explosives here...



	// START: Do not change anything between these 2 comments...
	WP_DET_PACK,
	WP_NUM_USEABLE = WP_DET_PACK,
	// END: ^^^ Do not change...

	// =================================================================
	//
	// Special weapons - do not add normal guns or melee weapons here...
	//
	// =================================================================

	WP_EMPLACED_GUN,
	WP_TURRET,

	WP_ALL_WEAPONS,
	WP_NUM_WEAPONS = WP_ALL_WEAPONS
} weapon_t;


typedef enum //# ammo_e
{
	AMMO_NONE,
	AMMO_FORCE,		// AMMO_PHASER
	AMMO_BLASTER,	// AMMO_STARFLEET,
	AMMO_POWERCELL,	// AMMO_ALIEN,
	AMMO_METAL_BOLTS,
	AMMO_ROCKETS,
	AMMO_EMPLACED,
	AMMO_THERMAL,
	AMMO_TRIPMINE,
	AMMO_DETPACK,
	AMMO_MAX
} ammo_t;


typedef struct weaponData_s
{
	char	classname[32];		// Spawning name

	int		fireTime;			// Amount of time between firings
	int		range;				// Range of weapon

	int		altFireTime;		// Amount of time between alt-firings
	int		altRange;			// Range of alt-fire

	int		chargeSubTime;		// ms interval for subtracting ammo during charge
	int		altChargeSubTime;	// above for secondary

	int		chargeSub;			// amount to subtract during charge on each interval
	int		altChargeSub;		// above for secondary

	int		maxCharge;			// stop subtracting once charged for this many ms
	int		altMaxCharge;		// above for secondary
} weaponData_t;


extern weaponData_t weaponData[WP_NUM_WEAPONS];


// Specific weapon information

#define FIRST_WEAPON		WP_BRYAR_PISTOL		// this is the first weapon for next and prev weapon switching
#define MAX_PLAYER_WEAPONS	WP_NUM_WEAPONS-1	// this is the max you can switch to and get with the give all.


#define DEFAULT_SHOTGUN_SPREAD	700
#define DEFAULT_SHOTGUN_COUNT	11

#define	LIGHTNING_RANGE		768

typedef enum {
	SCOPE_NONE,
	SCOPE_BINOCULARS,
	SCOPE_IRONSIGHT_SHORT,
	SCOPE_IRONSIGHT_MID,
	SCOPE_SCOPE_DISRUPTOR,
	SCOPE_SCOPE_BOWCASTER, // these should be renamed to some "scope company/model names"
	SCOPE_SCOPE_EE3_BLASTTECH_SHORT, // rename this - eg: SCOPE_CLASS_BLASTECH_EE3_SHORT or whatever
	SCOPE_SCOPE_A280_BLASTTECH_LONG_SHORT, // rename this
	SCOPE_SCOPE_DLT20A_BLASTTECH_LONG_SHORT, // rename this
	SCOPE_SCOPE_ACP_SNIPER, // Add anything you want. rename as you want.
	SCOPE_SCOPE_BOWCASTER_CLASSIC,
	SCOPE_SCOPE_WOOKIE_BOWCASTER_SCOPE,
	SCOPE_MAX_SCOPES
} scopeTypes_t;

typedef struct scopeData_s
{
	char	scopename[64];
	char	scopeModel[128];		// For the md3 we attach to tag_scope later
	char	scopeModelShader[128];	// For alternate shader to use on attached scope later
	char	gunMaskShader[128];
	char	maskShader[128];
	char	insertShader[128];
	char	lightShader[128];
	char	tickShader[128];
	char	chargeShader[128];
	char	zoomStartSound[128];
	char	zoomEndSound[128];
} scopeData_t;

extern scopeData_t scopeData[];

/*

*/

#endif //WEAPONS_H
