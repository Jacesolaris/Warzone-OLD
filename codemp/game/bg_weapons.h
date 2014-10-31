#pragma once

// Filename:-	bg_weapons.h
//
// This crosses both client and server.  It could all be crammed into bg_public, but isolation of this type of data is best.

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
	WP_A280,
	WP_DC15,
	WP_WESTARM5,
	WP_T21,
	WP_EE3,
	WP_CLONE_PISTOL1,
	WP_DLT20A,
	WP_CLONERIFLE,
	WP_WESTER_PISTOL,
	WP_ELG_3A,
	WP_S5_PISTOL,
	WP_Z6_BLASTER_CANON,
	WP_E60_ROCKET_LAUNCHER,
	WP_CW_ROCKET_LAUNCHER,

	//WP_FRAG_GRENADE,//fraggrenade
	//WP_FRAG_GRENADE_OLD,//oldfraggrenade
	//WP_SHOCK_GRENADE,//kes_pgren
	//WP_PLASMA_GRENADE,//plasma
	//WP_SONIC_GRENADE,//sonic_detonator
	//WP_THERMAL_GRENADE,//Stormtroopers
	//WP_THERMAL_GREADE_OLD,//thermal
	//WP_V_59_GRENADE,//V-59_Concussion


	WP_BOWCASTER,
	WP_REPEATER,
	WP_DEMP2,
	WP_FLECHETTE,
	WP_CONCUSSION,
	// Add new guns here...



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
//	char	classname[32];		// Spawning name

	int		ammoIndex;			// Index to proper ammo slot
	int		ammoLow;			// Count when ammo is low

	int		energyPerShot;		// Amount of energy used per shot
	int		fireTime;			// Amount of time between firings
	int		range;				// Range of weapon

	int		altEnergyPerShot;	// Amount of energy used for alt-fire
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
