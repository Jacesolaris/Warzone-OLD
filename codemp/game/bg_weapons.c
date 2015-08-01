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
	{70,    4,		-1	},	// WP_SPOTING_BLASTER,
	{70,    4,		-1	},	// WP_A200_ACP_BATTLERIFLE,
	{70,    4,		-1	},	// WP_A200_ACP_PISTOL,
	{70,    4,		-1	},	// WP_ACP_ARRAYGUN,
	{70,	6,	    -6	},	// WP_ACP_SNIPER_RIFLE,
	{12,	6,		-6	},	// WP_ARC_CASTER_IMPERIAL,
	{30,	3.5,	-6	},	// WP_BOWCASTER_CLASSIC,
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
	{ "Spoting Blaster",				AMMO_BLASTER,		5,			2,				350,		8192,		3,					500,			8192,		0,				0,					0,			0,				0,			0	},
	{ "ACP ArrayGun",					AMMO_BLASTER,		5,			2,				350,		8192,		3,					500,			8192,		0,				0,					0,			0,				0,			0	},
	{ "ACP Pistol",						AMMO_BLASTER,		5,			2,				350,		8192,		3,					500,			8192,		0,				0,					0,			0,				0,			0	},
	{ "ACP DoubleBarrel ArrayGun",		AMMO_BLASTER,		5,			2,				350,		8192,		3,					500,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Acp Sniper Rifle",				AMMO_BLASTER,		5,			2,				850,		8192,		3,					1200,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Arc Caster Imperial",			AMMO_BLASTER,		5,			2,				150,		8192,		3,					200,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Rifle Bowcaster Classic",		AMMO_BLASTER,		5,			2,				300,		8192,		3,					225,			8192,		0,				0,					0,			3,				0,			1700	},


	//Old Weapons. do not add anything under here only above where new guns is added. Stoiss
	{ "Merr-Sonn Missile System",		AMMO_ROCKETS,		5,			4,				3000,		8192,		2,					5000,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Thermal Detonator",				AMMO_THERMAL,		0,			1,				800,		8192,		1,					400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Trip Mine",						AMMO_TRIPMINE,		0,			1,				800,		8192,		1,					400,			8192,		0,				0,					0,			0,				0,			0	},	
	{ "Det Pack",						AMMO_DETPACK,		0,			1,				800,		8192,		1,					400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Emplaced Gun",					AMMO_NONE,			0,			0,				100,		8192,		3,					100,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Turret",							AMMO_NONE,			0,			0,				100,		8192,		3,					100,			8192,		0,				0,					0,			0,				0,			0	},
	
	};

qboolean IsSniperRifle ( int weapon )
{
	switch (weapon)
	{
	case WP_DISRUPTOR:
	case WP_A280:
	case WP_EE3:
	case WP_ACP_SNIPER_RIFLE:
	case WP_DLT20A:
	case WP_BOWCASTER_CLASSIC:
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
	case WP_ACP_SNIPER_RIFLE:
	case WP_BOWCASTER_CLASSIC:
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
	"BlastTech ACP HAMaR Scope",					"",												"",												"gfx/2d/acp_sniperrifle/scope_mask_overlay", "gfx/2d/acp_sniperrifle/scope_mask",	"",											"",												"",										"",										"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",
	"BlastTech Rifle Bowcaster",					"",												"",												"",										"gfx/2d/Bowcaster/lensmask",				"gfx/2d/Bowcaster/lensmask_zoom",			"",												"",										"",										"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",
};
