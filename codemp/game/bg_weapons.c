// Copyright (C) 2001-2002 Raven Software
//
// bg_weapons.c -- part of bg_pmove functionality

#include "qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

qboolean BG_HaveWeapon ( const playerState_t *ps, int weapon );

// Muzzle point table...
vec3_t WP_MuzzlePoint[WP_NUM_WEAPONS] =
{//	Fwd,	right,	up.
	{0,		0,		 0	},	// WP_NONE,
	{0,		8,		 0	},	// WP_STUN_BATON,
	{0,	    8,		 0	},	// WP_MELEE,
	{8,		16,		 0	},	// WP_SABER,
	{70, 	2.5,	-1	},	// WP_BRYAR_PISTOL,
	{70, 	3,		-1	},	// WP_BRYAR_OLD,
	{12,	6,		-6	},	// WP_BLASTER,
	{12,	6,		-6	},	// WP_DISRUPTOR,
	{70,	5,		-6	},	// WP_BOWCASTER,
	{12,	4,	    -6	},	// WP_REPEATER,
	{12,	6,		-6	},	// WP_DEMP2,
	{12,	6,		-6	},	// WP_FLECHETTE,
	{12,	4,  	-10	},	// WP_CONCUSSION,
	//add new weapon here under the last new weapon.
	{12,	5,		-6	},	// WP_A280,
	{70,	9,      -6	},	// WP_DC15,
	{70,	9,  	-6	},	// WP_WESTARM5,
	{70,	4,		-6	},	// WP_T21,
	{70,	6,	    -6	},	// WP_EE3,
	{70, 	3,		-1	},	// WP_DC_15S_CLONE_PISTOL,
	{70,	3.5,	-6	},	// WP_DLT20A,
	{70,	9,	    -6	},	// WP_CLONERIFLE,
	{70, 	3.5,  	-1	},	// WP_WESTER_PISTOL,
	{70, 	3.5,  	-1	},	// WP_ELG_3A,
	{70, 	3,  	-1	},	// WP_S5_PISTOL,
	{12,	6,	    -6	},	// WP_Z6_BLASTER_CANON,
	{30,	3.5,	-6	},	// WP_WOOKIE_BOWCASTER,
	{70, 	3.5,  	-1	},	// WP_WOOKIES_PISTOL,
	{12,	4.5,	-6	},	// WP_CLONE_BLASTER,
	{70,	9.5,  	-6	},	// WP_DC15_EXT,
	{12,	8,		-4	},  // WP_E60_ROCKET_LAUNCHER,
	{12,	8,		-4	},  // WP_CW_ROCKET_LAUNCHER,
	{70,    4,		-1	},	// WP_TESTGUN,
	{12,	0,		-4	},	// WP_FRAG_GRENADE,
	{12,	0,		-4	},	// WP_FRAG_GRENADE_OLD,
	{70,    4,		-1	},	// WP_DC_17_CLONE_PISTOL,
	//old weapons
	{30,	6,		-14	},	// WP_ROCKET_LAUNCHER,
	{12,	0,		-4	},	// WP_THERMAL,
	{12,	0,		-10	},	// WP_TRIP_MINE,
	{12,	0,		-4	},	// WP_DET_PACK,

};
//ready for 2 handed pistols when ready for it.
vec3_t WP_FirstPistolMuzzle = { 12, 6, 0 };
vec3_t WP_SecondPistolMuzzle = { 12, -6, 0 };

//New clean weapon table NOTE* need to remeber to put the WP_NAME same place as you have added it in BG_weapon.h else it gets messed up in the weapon table
weaponData_t weaponData[WP_NUM_WEAPONS] = {
	// char	classname[32];				ammoIndex			ammoLow		energyPerShot	fireTime	range		altEnergyPerShot	altFireTime		altRange	chargeSubTime	altChargeSubTime	chargeSub	altChargeSub	maxCharge	altMaxCharge
	{ "No Weapon",						AMMO_NONE,			0,			0,				0,			0,			0,					0,				0,			0,				0,					0,			0,			 	0,			0	},
	{ "Stun Baton",						AMMO_NONE,			5,			0,				400,		8192,		0,					400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Melee",							AMMO_NONE,			5,			0,				400,		8192,		0,					400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Light Saber",					AMMO_NONE,			5,			0,				100,		8192,		0,					100,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Bryar Pistol",					AMMO_BLASTER,		0,			0,				800,		8192,		0,					800,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Old Bryar Pistol",				AMMO_BLASTER,		15,			2,				600/*400*/,	8192,		2,					400,			8192,		0,				200,				0,			1,				0,			1500	},
	{ "E-11 Blaster Rifle",				AMMO_BLASTER,		5,			2,				300/*350*/,	8192,		3,					250,			8192,		0,				200,				0,			3,				0,			1700	},
	{ "Tenloss Disruptor Rifle",		AMMO_POWERCELL,		5,			5,				600,		8192,		6,					1300,			8192,		0,				0,					0,			0,				0,			0	},	
	{ "Modified Wookie Crossbow",		AMMO_POWERCELL,		5,			5,				800/*1000*/,8192,		5,					750,			8192,		400,			0,					5,			0,				1700,		0	},
	{ "Imperial Heavy Repeater",		AMMO_METAL_BOLTS,	5,			8,				175/*150*/,	8192,		15,					800,			8192,		0,				0,					0,			0,				0,			0	},
	{ "DEMP 2",							AMMO_POWERCELL,		5,			8,				500,		8192,		6,					900,			8192,		0,				250,				0,			3,				0,			2100	},
	{ "Golan Arms Flechette",			AMMO_METAL_BOLTS,	5,			10,				700,		8192,		15,					800,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Concussion Rifle",				AMMO_METAL_BOLTS,	40,			40,				800,		8192,		50,					1200,			8192,		0,				0,					0,			0,				0,			0	},
	//Place new Guns under here.
	{ "A280 Clone Blaster",				AMMO_BLASTER,		5,			5,				350,		8192,		6,					1500,			8192,		0,				0,					0,			3,				0,			1700	},
	{ "DC-15 Blaster",					AMMO_METAL_BOLTS,	5,			8,				250/*200*/,	8192,		6,					900,			8192,		0,				3,					0,			3,			    0,			2100	},
	{ "Westarm 5 Blaster",				AMMO_METAL_BOLTS,	5,			8,				250/*200*/,	8192,		6,					900,			8192,		0,				3,					0,			250,		    0,			2100	},
	{ "T-21 Blaster Rifle",				AMMO_BLASTER,		5,			2,				350/*550*/,	8192,		3,					250,			8192,		0,				0,					0,			0,				0,			0	},
	{ "EE-3 Blaster Rifle",				AMMO_BLASTER,		5,			2,				250,		8192,		3,					1200,			8192,		0,				0,					0,			0,				0,			0	},
	{ "DC-15-S Blaster Pistol",			AMMO_POWERCELL,		5,			8,				450,		8192,		6,					900,			8192,		0,				250,				0,			3,				0,			2100	},
	{ "DLT-20-A Blaster Rifle",			AMMO_BLASTER,		5,			2,				350,		8192,		3,					1200,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Clone Trooper Rifle",			AMMO_BLASTER,		5,			8,				250/*200*/,	8192,		6,					900,			8192,		0,				250,				0,			3,				0,			2100	},
	{ "Wester Pistol",					AMMO_BLASTER,		15,			2,				400,		8192,		2,					400,			8192,		0,				200,				0,			1,				0,			1500	},
	{ "ELG-3A Pistol",					AMMO_BLASTER,		15,			2,				400,		8192,		2,					400,			8192,		0,				200,				0,			1,				0,			1500	},
	{ "S-5 Pistol",						AMMO_BLASTER,		15,			2,				400,		8192,		2,					250,			8192,		0,				200,				0,			1,				0,			1500	},
	{ "Z-6 Rotary Blaster Cannon",		AMMO_METAL_BOLTS,	5,			1,				125,		8192,		15,					2800,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Wookiee Bowcaster",				AMMO_POWERCELL,		5,			5,				350,		8192,		5,					650,			8192,		0,				0,					5,			0,				1700,		0	},
	{ "Wookie Pistol",					AMMO_BLASTER,		15,			2,				500,		8192,		2,					250,			8192,		0,				200,				0,			1,				0,			1500	},
	{ "DC-15-S Clone Blaster",			AMMO_BLASTER,		5,			2,				325,		8192,		3,					145,			8192,		0,				0,					0,			0,				0,			0	},
	{ "DC-15 Ext Blaster",				AMMO_BLASTER,		5,			8,				250/*200*/,	8192,		6,					900,			8192,		0,				250,				0,			3,				0,			2100	},
	{ "E-60 Pulse Rocket Launcher",		AMMO_ROCKETS,		5,			4,				3000,		8192,		2,					5000,			8192,		0,				0,					0,			0,				0,			0	},
	{ "CW Pulse Rocket Launcher",		AMMO_ROCKETS,		5,			4,				3000,		8192,		2,					5000,			8192,		0,				0,					0,			0,				0,			0	},
	{ "TEST GUN",						AMMO_BLASTER,		5,			2,				350,		8192,		3,					500,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Frag Grenade",					AMMO_THERMAL,		0,			1,				800,		8192,		1,					400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Old Frag Grenade",				AMMO_THERMAL,		0,			1,				800,		8192,		1,					400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "DC-17 Clone Blaster Pistol",		AMMO_POWERCELL,		5,			8,				375,		8192,		6,					875,			8192,		0,				250,				0,			3,				0,			2100	},
	
	//Old Weapons. do not add anything under here only above where new guns is added. Stoiss
	{ "Merr-Sonn Missile System",		AMMO_ROCKETS,		5,			4,				3000,		8192,		2,					5000,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Thermal Detonator",				AMMO_THERMAL,		0,			1,				800,		8192,		1,					400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Trip Mine",						AMMO_TRIPMINE,		0,			1,				800,		8192,		1,					400,			8192,		0,				0,					0,			0,				0,			0	},	
	{ "Det Pack",						AMMO_DETPACK,		0,			1,				800,		8192,		1,					400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Emplaced Gun",					AMMO_NONE,			0,			0,				100,		8192,		3,					100,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Turret",							AMMO_NONE,			0,			0,				100,		8192,		3,					100,			8192,		0,				0,					0,			0,				0,			0	},
	
	};


//keeping the old weapon table just for safty
//weaponData_t weaponData[WP_NUM_WEAPONS] =
//{
//	{	// WP_NONE
////		"No Weapon",			//	char	classname[32];		// Spawning name
//		AMMO_NONE,				//	int		ammoIndex;			// Index to proper ammo slot
//		0,						//	int		ammoLow;			// Count when ammo is low
//		0,						//	int		energyPerShot;		// Amount of energy used per shot
//		0,						//	int		fireTime;			// Amount of time between firings
//		0,						//	int		range;				// Range of weapon
//		0,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		0,						//	int		altFireTime;		// Amount of time between alt-firings
//		0,						//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_STUN_BATON
////		"Stun Baton",			//	char	classname[32];		// Spawning name
//		AMMO_NONE,				//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		0,						//	int		energyPerShot;		// Amount of energy used per shot
//		400,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		0,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		400,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_MELEE
////		"Melee",			//	char	classname[32];		// Spawning name
//		AMMO_NONE,				//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		0,						//	int		energyPerShot;		// Amount of energy used per shot
//		400,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		0,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		400,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_SABER,
////		"Lightsaber",			//	char	classname[32];		// Spawning name
//		AMMO_NONE,				//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		0,						//	int		energyPerShot;		// Amount of energy used per shot
//		100,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		0,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		100,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_BRYAR_PISTOL,
////		"Bryar Pistol",			//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		0,//15,						//	int		ammoLow;			// Count when ammo is low
//		0,//2,						//	int		energyPerShot;		// Amount of energy used per shot
//		800,//400,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		0,//2,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		800,//400,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,//200,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,//1,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0,//1500					//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_BRYAR_OLD,
////		"Bryar Pistol",			//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		15,						//	int		ammoLow;			// Count when ammo is low
//		2,						//	int		energyPerShot;		// Amount of energy used per shot
//		400,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		2,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		400,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		200,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		1,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		1500					//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_BLASTER
////		"E11 Blaster Rifle",	//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		2,						//	int		energyPerShot;		// Amount of energy used per shot
//		350,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		3,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		150,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_DISRUPTOR
////		"Tenloss Disruptor Rifle",//	char	classname[32];		// Spawning name
//		AMMO_POWERCELL,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		5,						//	int		energyPerShot;		// Amount of energy used per shot
//		600,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		6,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		1300,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		200,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		3,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		1700					//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_A280
////		"A280",	//	char	classname[32];		// Spawning name
//		AMMO_POWERCELL,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		5,						//	int		energyPerShot;		// Amount of energy used per shot
//		200,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		6,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		1500,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		200,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		3,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		1700					//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_DC15
//		//"Dc- 15 Blaster",		//	char	classname[32];		// Spawning name
//		AMMO_METAL_BOLTS,		//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		8,						//	int		energyPerShot;		// Amount of energy used per shot
//		200,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		6,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		900,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		250,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		3,						//	int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		2100					//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_WESTARM5
//		//"Westarm 5 Blaster",		//	char	classname[32];		// Spawning name
//		AMMO_METAL_BOLTS,		//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		8,						//	int		energyPerShot;		// Amount of energy used per shot
//		200,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		6,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		900,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		250,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		3,						//	int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		2100					//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_T21
//		//		"T-21 Blaster Rifle",	//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		2,						//	int		energyPerShot;		// Amount of energy used per shot
//		550,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		3,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		150,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_EE3
//		//"EE-3 Blaster Rifle",	//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		2,						//	int		energyPerShot;		// Amount of energy used per shot
//		250,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		3,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		1200,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	
//		//"WP_DC_15S_CLONE_PISTOL",	//	char	classname[32];		// Spawning name
//		AMMO_POWERCELL,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		8,						//	int		energyPerShot;		// Amount of energy used per shot
//		400,//500,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		6,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		900,						//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		250,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		3,						//	int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		2100					//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_DLT20A
//		//"DLT-20a Blaster Rifle",	//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		2,						//	int		energyPerShot;		// Amount of energy used per shot
//		250,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		3,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		1200,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_CLONERIFLE
//		//"Clone Trooper Rifle",//	char	classname[32];		// Spawning name
//		AMMO_POWERCELL,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		8,						//	int		energyPerShot;		// Amount of energy used per shot
//		200,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		6,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		900,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		250,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		3,						//	int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		2100					//	int		altMaxCharge;		// above for secondary
//	},
//	{	//WP_WESTER_PISTOL,
//		//"Wester Pistol",		//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		15,						//	int		ammoLow;			// Count when ammo is low
//		2,						//	int		energyPerShot;		// Amount of energy used per shot
//		400,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		2,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		400,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		200,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		1,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		1500					//	int		altMaxCharge;		// above for secondary
//	},
//	{	//WP_ELG_3A,
//		//"ELG-3A Diplomat's Pistol",		//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		15,						//	int		ammoLow;			// Count when ammo is low
//		2,						//	int		energyPerShot;		// Amount of energy used per shot
//		600,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		2,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		400,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		200,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		1,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		1500					//	int		altMaxCharge;		// above for secondary
//	},
//	{	//WP_S5_PISTOL,
//		//"s5 Pistol",		//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		15,						//	int		ammoLow;			// Count when ammo is low
//		2,						//	int		energyPerShot;		// Amount of energy used per shot
//		300,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		2,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		250,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		200,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		1,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		1500					//	int		altMaxCharge;		// above for secondary
//	},
//
//	
//	{	//WP_Z6_BLASTER_CANON
//		//"Z-6 rotary blaster cannon",//	char	classname[32];		// Spawning name
//		AMMO_METAL_BOLTS,		//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		8,						//	int		energyPerShot;		// Amount of energy used per shot
//		100,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		6,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		2800,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		250,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		3,						//	int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		2100					//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_WOOKIE_BOWCASTER
//		//"Wookiee Bowcaster",	//	char	classname[32];		// Spawning name
//		AMMO_POWERCELL,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		5,						//	int		energyPerShot;		// Amount of energy used per shot
//		350,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		5,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		650,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		400,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,					//	int		altChargeSubTime;	// above for secondary
//		5,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		1700,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0					//	int		altMaxCharge;		// above for secondary
//	},
//	{	//WP_WOOKIES_PISTOL,
//		//"Wookie Pistol",		//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		15,						//	int		ammoLow;			// Count when ammo is low
//		2,						//	int		energyPerShot;		// Amount of energy used per shot
//		300,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		2,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		250,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		200,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		1,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		1500					//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_CLONE_BLASTER
//		//"DC-15s Clone Blaster",	//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		2,						//	int		energyPerShot;		// Amount of energy used per shot
//		325,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		3,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		145,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//
//
//	{	// WP_DC15_EXT
//		//"dc-15_ext",				//	char	classname[32];		// Spawning name
//		AMMO_POWERCELL,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		8,						//	int		energyPerShot;		// Amount of energy used per shot
//		200,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		6,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		900,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		250,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		3,						//	int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		2100					//	int		altMaxCharge;		// above for secondary
//	},
//	//{    // WP_E60_ROCKET_LAUNCHER
//	//	//		"Merr-Sonn Missile System",	//	char	classname[32];		// Spawning name
//	//	AMMO_ROCKETS,			//	int		ammoIndex;			// Index to proper ammo slot
//	//	5,						//	int		ammoLow;			// Count when ammo is low
//	//	1,						//	int		energyPerShot;		// Amount of energy used per shot
//	//	3000,					//	int		fireTime;			// Amount of time between firings
//	//	8192,					//	int		range;				// Range of weapon
//	//	2,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//	//	5000,					//	int		altFireTime;		// Amount of time between alt-firings
//	//	8192,					//	int		altRange;			// Range of alt-fire
//	//	0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//	//	0,						//	int		altChargeSubTime;	// above for secondary
//	//	0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//	//	0,						//int		altChargeSub;		// above for secondary
//	//	0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//	//	0						//	int		altMaxCharge;		// above for secondary
//	//},
//	//{    // WP_CW_ROCKET_LAUNCHER
//	//	//		"Merr-Sonn Missile System",	//	char	classname[32];		// Spawning name
//	//	AMMO_ROCKETS,			//	int		ammoIndex;			// Index to proper ammo slot
//	//	5,						//	int		ammoLow;			// Count when ammo is low
//	//	1,						//	int		energyPerShot;		// Amount of energy used per shot
//	//	3000,					//	int		fireTime;			// Amount of time between firings
//	//	8192,					//	int		range;				// Range of weapon
//	//	2,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//	//	5000,					//	int		altFireTime;		// Amount of time between alt-firings
//	//	8192,					//	int		altRange;			// Range of alt-fire
//	//	0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//	//	0,						//	int		altChargeSubTime;	// above for secondary
//	//	0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//	//	0,						//int		altChargeSub;		// above for secondary
//	//	0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//	//	0						//	int		altMaxCharge;		// above for secondary
//	//},
//	{	// WP_TESTGUN
//		//"Clone Trooper Rifle",//	char	classname[32];		// Spawning name
//		AMMO_BLASTER,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		2,						//	int		energyPerShot;		// Amount of energy used per shot
//		350,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		3,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		500,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_BOWCASTER
////		"Wookiee Bowcaster",		//	char	classname[32];		// Spawning name
//		AMMO_POWERCELL,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		5,						//	int		energyPerShot;		// Amount of energy used per shot
//		1000,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		5,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		750,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		400,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,					//	int		altChargeSubTime;	// above for secondary
//		5,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		1700,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0					//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_REPEATER
////		"Imperial Heavy Repeater",//	char	classname[32];		// Spawning name
//		AMMO_METAL_BOLTS,		//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		8,						//	int		energyPerShot;		// Amount of energy used per shot
//		150,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		15,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		800,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_DEMP2
////		"DEMP2",				//	char	classname[32];		// Spawning name
//		AMMO_POWERCELL,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		8,						//	int		energyPerShot;		// Amount of energy used per shot
//		500,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		6,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		900,						//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		250,					//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		3,						//	int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		2100					//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_FLECHETTE
////		"Golan Arms Flechette",	//	char	classname[32];		// Spawning name
//		AMMO_METAL_BOLTS,		//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		10,						//	int		energyPerShot;		// Amount of energy used per shot
//		700,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		15,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		800,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_CONCUSSION
////		"Concussion Rifle",		//	char	classname[32];		// Spawning name
//		AMMO_METAL_BOLTS,		//	int		ammoIndex;			// Index to proper ammo slot
//		40,						//	int		ammoLow;			// Count when ammo is low
//		40,						//	int		energyPerShot;		// Amount of energy used per shot
//		800,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		50,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		1200,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//	int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_ROCKET_LAUNCHER
////		"Merr-Sonn Missile System",	//	char	classname[32];		// Spawning name
//		AMMO_ROCKETS,			//	int		ammoIndex;			// Index to proper ammo slot
//		5,						//	int		ammoLow;			// Count when ammo is low
//		1,						//	int		energyPerShot;		// Amount of energy used per shot
//		3000,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		2,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		5000,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_THERMAL
////		"Thermal Detonator",	//	char	classname[32];		// Spawning name
//		AMMO_THERMAL,				//	int		ammoIndex;			// Index to proper ammo slot
//		0,						//	int		ammoLow;			// Count when ammo is low
//		1,						//	int		energyPerShot;		// Amount of energy used per shot
//		800,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		1,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		400,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_TRIP_MINE
////		"Trip Mine",			//	char	classname[32];		// Spawning name
//		AMMO_TRIPMINE,				//	int		ammoIndex;			// Index to proper ammo slot
//		0,						//	int		ammoLow;			// Count when ammo is low
//		1,						//	int		energyPerShot;		// Amount of energy used per shot
//		800,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		1,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		400,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_DET_PACK
////		"Det Pack",				//	char	classname[32];		// Spawning name
//		AMMO_DETPACK,				//	int		ammoIndex;			// Index to proper ammo slot
//		0,						//	int		ammoLow;			// Count when ammo is low
//		1,						//	int		energyPerShot;		// Amount of energy used per shot
//		800,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		0,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		400,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_EMPLCACED_GUN
////		"Emplaced Gun",			//	char	classname[32];		// Spawning name
//		/*AMMO_BLASTER*/0,			//	int		ammoIndex;			// Index to proper ammo slot
//		/*5*/0,						//	int		ammoLow;			// Count when ammo is low
//		/*2*/0,						//	int		energyPerShot;		// Amount of energy used per shot
//		100,					//	int		fireTime;			// Amount of time between firings
//		8192,					//	int		range;				// Range of weapon
//		/*3*/0,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		100,					//	int		altFireTime;		// Amount of time between alt-firings
//		8192,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	},
//	{	// WP_TURRET - NOTE NOT ACTUALLY USEABLE BY PLAYER!
////		"Emplaced Gun",			//	char	classname[32];		// Spawning name
//		/*AMMO_BLASTER*/0,			//	int		ammoIndex;			// Index to proper ammo slot
//		/*5*/0,						//	int		ammoLow;			// Count when ammo is low
//		/*2*/0,						//	int		energyPerShot;		// Amount of energy used per shot
//		0,					//	int		fireTime;			// Amount of time between firings
//		0,					//	int		range;				// Range of weapon
//		/*3*/0,						//	int		altEnergyPerShot;	// Amount of energy used for alt-fire
//		0,					//	int		altFireTime;		// Amount of time between alt-firings
//		0,					//	int		altRange;			// Range of alt-fire
//		0,						//	int		chargeSubTime;		// ms interval for subtracting ammo during charge
//		0,						//	int		altChargeSubTime;	// above for secondary
//		0,						//	int		chargeSub;			// amount to subtract during charge on each interval
//		0,						//int		altChargeSub;		// above for secondary
//		0,						//	int		maxCharge;			// stop subtracting once charged for this many ms
//		0						//	int		altMaxCharge;		// above for secondary
//	}
//};

qboolean IsSniperRifle ( int weapon )
{
	switch (weapon)
	{
	case WP_DISRUPTOR:
	case WP_A280:
	case WP_EE3:
	case WP_DLT20A:
		return qtrue;
	default:
		break;
	}

	return qfalse;
}

qboolean SniperRifleCharges ( int weapon )
{
	switch (weapon)
	{
	case WP_A280:
	case WP_DLT20A:
	case WP_EE3:
		return qfalse;
	default:
		break;
	}

	return qtrue;
}

qboolean BG_HaveWeapon ( const playerState_t *ps, int weapon )
{
	if (ps->primaryWeapon == weapon && weapon <= WP_NUM_USEABLE) return qtrue;
	if (ps->secondaryWeapon == weapon && weapon <= WP_NUM_USEABLE) return qtrue;
	if (ps->temporaryWeapon == weapon && weapon <= WP_NUM_USEABLE) return qtrue;
	if (ps->temporaryWeapon == WP_ALL_WEAPONS && weapon <= WP_NUM_USEABLE) return qtrue;

	return qfalse;
}

qboolean HaveWeapon ( playerState_t *ps, int weapon )
{
	if (ps->primaryWeapon == weapon && weapon <= WP_NUM_USEABLE) return qtrue;
	if (ps->secondaryWeapon == weapon && weapon <= WP_NUM_USEABLE) return qtrue;
	if (ps->temporaryWeapon == weapon && weapon <= WP_NUM_USEABLE) return qtrue;
	if (ps->temporaryWeapon == WP_ALL_WEAPONS && weapon <= WP_NUM_USEABLE) return qtrue;

	return qfalse;
}

// NOTE: "" means unused/ignore
scopeData_t scopeData[] = {
	// char	scopename[64],							char scopeModel[128],							char scopeModelShader[128],						char gunMaskShader[128],				char	maskShader[128],					char	insertShader[128],					char	lightShader[128],						char	tickShader[128],				char	chargeShader[128],				char	zoomStartSound[128],				char	zoomEndSound[128]
	"No Scope",										"",												"",												"",										"",											"",											"",												"",										"",										"",											"",
	"Binoculars",									"",												"",												"",										"",											"",											"",												"",										"",										"sound/interface/zoomstart.wav",			"sound/interface/zoomend.wav",
	"Short Range Viewfinder",						"",												"",												"",										"",											"",											"",												"",										"",										"",											"",
	"Mid Range Viewfinder",							"",												"",												"",										"",											"",											"",												"",										"",										"",											"",
	"Tenloss Disruptor Scope",						"",												"",												"",  									"gfx/2d/cropCircle2",						"gfx/2d/cropCircle",						"gfx/2d/cropCircleGlow",						"gfx/2d/insertTick",					"gfx/2d/crop_charge",					"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",
	"Blastech Bowcaster Scope",						"",												"",												"",										"gfx/2d/bowMask",							"gfx/2d/bowInsert",							"",												"",										"",										"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav", // probably wrong. needs research
	"Blastech EE3 Scope",							"",												"",												"",										"gfx/2d/fett/cropCircle2",					"gfx/2d/fett/cropCircle",					"gfx/2d/fett/cropCircleGlow",					"",										"gfx/2d/fett/crop_charge",				"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",
	"Blastecg A280 Scope",							"",												"",												"gfx/2D/arcMask",						"gfx/2d/a280cropCircle2",					"gfx/2d/a280cropCircle",					"",												"",										"",										"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",
	"Blastech DLT 20A Scope",						"",												"",												"gfx/2D/arcMask",						"gfx/2d/a280cropCircle2",					"gfx/2d/a280cropCircle",					"",												"",										"",										"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",
	/*"Type 4 Scope",									"",												"",												"gfx/2D/arcMask",						"gfx/2d/a280cropCircle2",					"gfx/2d/a280cropCircle",					"",												"",										"",										"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",
	"Type 5 Scope",									"",												"",												"gfx/2D/arcMask",						"gfx/2d/a280cropCircle2",					"gfx/2d/a280cropCircle",					"",												"",										"",										"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",*/
};
