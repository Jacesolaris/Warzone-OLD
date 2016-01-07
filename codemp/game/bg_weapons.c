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
	{30, 	2.5,	-1	},	// WP_BRYAR_PISTOL,
	{30, 	3,		-1	},	// WP_BRYAR_OLD,
	{30,	8,		-6	},	// WP_BLASTER,
	{20,	4,		-6	},	// WP_DISRUPTOR,
	{30,	7,		-6	},	// WP_BOWCASTER,
	{30,	6,	    -6	},	// WP_REPEATER,
	{12,	6,		-6	},	// WP_DEMP2,
	{12,	6,		-6	},	// WP_FLECHETTE,
	{12,	4,  	-10	},	// WP_CONCUSSION,
	//add new weapon here under the last new weapon.
	{12,	7,		-6	},	// WP_A280,
	{30,	9,      -6	},	// WP_DC15,
	{30,	9,  	-6	},	// WP_WESTARM5,
	{30,	4,		-6	},	// WP_T21,
	{30,	7,		-6	},	// WP_EE3,
	{30, 	3,		-1	},	// WP_DC_15S_CLONE_PISTOL,
	{30,	8,		-6	},	// WP_DLT_19,
	{30,	9,	    -6	},	// WP_DC_15A_Rifle,
	{30, 	3.5,  	-1	},	// WP_WESTER_PISTOL,
	{30, 	3.5,  	-1	},	// WP_ELG_3A,
	{30, 	3,  	-1	},	// WP_S5_PISTOL,
	{12,	6,	    -6	},	// WP_Z6_BLASTER_CANON,
	{30,	7,		-6	},	// WP_WOOKIE_BOWCASTER,
	{30, 	3.5,  	-1	},	// WP_WOOKIES_PISTOL,
	{30,	7,		-6	},	// WP_CLONE_BLASTER,
	{30,	9.5,  	-6	},	// WP_DC15_EXT,
	{30,	8,		-4	},  // WP_E60_ROCKET_LAUNCHER,
	{30,	8,		-4	},  // WP_CW_ROCKET_LAUNCHER,
	{30,    4,		-1	},	// WP_TESTGUN,
	{30,    3,		-1	},	// WP_DC_17_CLONE_PISTOL,
	{30,    4,		-1	},	// WP_SPOTING_BLASTER,
	{30,    6.5,	-1	},	// WP_A200_ACP_BATTLERIFLE,
	{30,    4,		-1	},	// WP_A200_ACP_PISTOL,
	{30,    6,		-1	},	// WP_ACP_ARRAYGUN,
	{30,	7,		-6	},	// WP_ACP_SNIPER_RIFLE,
	{12,	6,		-6	},	// WP_ARC_CASTER_IMPERIAL,
	{30,	7,		-6	},	// WP_BOWCASTER_CLASSIC,
	{30,	7,		-6	},	// WP_WOOKIE_BOWCASTER_SCOPE,
	{30,	7.5,	-6	},	// WP_BRYAR_CARBINE,
	{30,	7.5,	-6	},	// WP_BRYAR_RIFLE,
	{30,	7.5,	-6	},	// WP_BRYAR_RIFLE_SCOPE,
	{30,	9,	    -6	},	// WP_PULSECANON,
	{30,	7,		-6	},	// WP_PROTON_CARBINE_RIFLE,
	{30, 	2.5,	-1	},	// WP_DH_17_PISTOL,
	//old weapons
	{30,	6,		-14	},	// WP_ROCKET_LAUNCHER,
	{12,	0,		-4	},	// WP_THERMAL,
	{12,	0,		-4	},	// WP_FRAG_GRENADE,
	{12,	0,		-4	},	// WP_FRAG_GRENADE_OLD,
	{12,	0,		-10	},	// WP_TRIP_MINE,
	{12,	0,		-4	},	// WP_DET_PACK,

};
//ready for 2 handed pistols when ready for it.
vec3_t WP_FirstPistolMuzzle[WP_NUM_WEAPONS] = 
{//	Fwd,	right,	up.
	{ 12,	6,		0	},

};
vec3_t WP_SecondPistolMuzzle[WP_NUM_WEAPONS] = 
{//	Fwd,	right,	up.
	{ 12,  -6,		0	},

};

//New clean weapon table NOTE* need to remeber to put the WP_NAME same place as you have added it in BG_weapon.h else it gets messed up in the weapon table
weaponData_t weaponData[WP_NUM_WEAPONS] = {
	// char	classname[32];							fireTime	range		altFireTime		altRange	chargeSubTime	altChargeSubTime	chargeSub	altChargeSub	maxCharge	altMaxCharge
	{ "No Weapon",									0,			0,			0,				0,			0,				0,					0,			0,			 	0,			0	},
	{ "Stun Baton",									400,		8192,		400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Melee",										400,		8192,		400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Light Saber",								100,		8192,		100,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Bryar Pistol",								800,		8192,		800,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Old Bryar Pistol",							600,		8192,		400,			8192,		0,				200,				0,			1,				0,			1500	},
	{ "E-11 Blaster Rifle",							350,		8192,		275,			8192,		0,				200,				0,			3,				0,			1700	},
	{ "Tenloss Disruptor Rifle",					600,		8192,		1300,			8192,		0,				0,					0,			0,				0,			0	},	
	{ "Modified Wookie Crossbow",					800,		8192,		750,			8192,		400,			0,					5,			0,				1700,		0	},
	{ "Imperial Heavy Repeater",					175,		8192,		800,			8192,		0,				0,					0,			0,				0,			0	},
	{ "DEMP 2",										500,		8192,		900,			8192,		0,				250,				0,			3,				0,			2100	},
	{ "Golan Arms Flechette",						700,		8192,		800,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Concussion Rifle",							800,		8192,		1200,			8192,		0,				0,					0,			0,				0,			0	},
	//Place new Guns under here.
	{ "A280 Clone Blaster",							250,		8192,		1500,			8192,		0,				0,					0,			3,				0,			1700	},
	{ "DC-15 Blaster",								175,		8192,		900,			8192,		0,				3,					0,			3,			    0,			2100	},
	{ "Westarm 5 Blaster",							175,		8192,		900,			8192,		0,				3,					0,			250,		    0,			2100	},
	{ "T-21 Blaster Rifle",							175,		8192,		1200,			8192,		0,				100,				0,			0,				0,			0	},
	{ "EE-3 Blaster Rifle",							250,		8192,		1200,			8192,		0,				0,					0,			0,				0,			0	},
	{ "DC-15-S Blaster Pistol",						450,		8192,		900,			8192,		0,				250,				0,			3,				0,			2100	},
	{ "DLT-19 Heavy Blaster Rifle",					175,		8192,		1200,			8192,		0,				0,					0,			0,				0,			0	},
	{ "DC-15A_Rifle",								175,		8192,		900,			8192,		0,				250,				0,			3,				0,			2100	},
	{ "Wester Pistol",								800,		8192,		800,			8192,		0,				200,				0,			1,				0,			1500	},
	{ "ELG-3A Pistol",								800,		8192,		800,			8192,		0,				200,				0,			1,				0,			1500	},
	{ "S-5 Pistol",									800,		8192,		800,			8192,		0,				200,				0,			1,				0,			1500	},
	{ "Z-6 Rotary Blaster Cannon",					125,		8192,		2800,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Wookiee Bowcaster",							350,		8192,		650,			8192,		0,				0,					5,			0,				1700,		0	},
	{ "Wookie Pistol",								500,		8192,		250,			8192,		0,				200,				0,			1,				0,			1500	},
	{ "DC-15-S Blaster",							350,		8192,		275,			8192,		0,				0,					0,			0,				0,			0	},
	{ "DC-15 Ext Blaster",							175,		8192,		900,			8192,		0,				250,				0,			3,				0,			2100	},
	{ "E-60 Pulse Rocket Launcher",					3000,		8192,		5000,			8192,		0,				0,					0,			0,				0,			0	},
	{ "CW Pulse Rocket Launcher",					3000,		8192,		5000,			8192,		0,				0,					0,			0,				0,			0	},
	{ "TEST GUN",									350,		8192,		500,			8192,		0,				0,					0,			0,				0,			0	},
	{ "DC-17 Clone Blaster Pistol",					375,		8192,		875,			8192,		0,				250,				0,			3,				0,			2100	},
	{ "Spoting Blaster",							350,		8192,		500,			8192,		0,				0,					0,			0,				0,			0	},
	{ "ACP ArrayGun",								350,		8192,		500,			8192,		0,				0,					0,			0,				0,			0	},
	{ "ACP Pistol",									350,		8192,		500,			8192,		0,				0,					0,			0,				0,			0	},
	{ "ACP DoubleBarrel ArrayGun",					350,		8192,		500,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Acp Sniper Rifle",							850,		8192,		1200,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Arc Caster Imperial",						1200,		8192,		300,			8192,		0,				0,					0,			0,				1500,			0	},
	{ "Rifle Bowcaster Classic",					550,		8192,		350,			8192,		0,				0,					0,			3,				0,			1700	},
	{ "Heavy Wookie Bowcaster",						550,		8192,		350,			8192,		0,				0,					0,			3,				0,			1700	},
	{ "Bryar Carbine Blaster",						350,		8192,		250,			8192,		0,				200,				0,			3,				0,			1700	},
	{ "Bryar Rifle",								275,		8192,		600,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Bryar Sniper Rifle",							250,		8192,		1200,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Pulse Canon",								200,		8192,		2800,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Proton Carbine Rifle",						350,		8192,		275,			8192,		0,				200,				0,			3,				0,			1700	},
	{ "DH-17 Pistol",								325,		8192,		275,			8192,		0,				200,				0,			3,				0,			1700	},
	//Old Weapons. do not add anything under here only above where new guns is added. Stoiss - UQ1: Grenades should be below here.
	{ "Merr-Sonn Missile System",					3000,		8192,		5000,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Thermal Detonator",							800,		8192,		400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Frag Grenade",								800,		8192,		400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Old Frag Grenade",							800,		8192,		400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Trip Mine",									800,		8192,		400,			8192,		0,				0,					0,			0,				0,			0	},	
	{ "Det Pack",									800,		8192,		400,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Emplaced Gun",								100,		8192,		100,			8192,		0,				0,					0,			0,				0,			0	},
	{ "Turret",										100,		8192,		100,			8192,		0,				0,					0,			0,				0,			0	},
	
	};

qboolean IsRollWithPistols(int weapon)
{
	switch (weapon)
	{
	case WP_BRYAR_PISTOL:
	case WP_BRYAR_OLD:
	case WP_DC_15S_CLONE_PISTOL:
	case WP_WESTER_PISTOL:
	case WP_ELG_3A:
	case WP_S5_PISTOL:
	case WP_WOOKIES_PISTOL:
	case WP_TESTGUN:
	case WP_THERMAL:
	case WP_FRAG_GRENADE:
	case WP_FRAG_GRENADE_OLD:
	case WP_DC_17_CLONE_PISTOL:
	case WP_SPOTING_BLASTER:
	case WP_A200_ACP_PISTOL:
		return qtrue;
	default:
		break;
	}

	return qfalse;
}
//theses weapons have charge option if they not are listed under the WeaponIsSniperNoCharge function.
qboolean WeaponSniperCharge(int weapon)
{
	switch (weapon)
	{
	case WP_DISRUPTOR:
	case WP_DLT_19:
	case WP_WOOKIE_BOWCASTER_SCOPE:
	case WP_BRYAR_RIFLE_SCOPE:
	case WP_A280:
	case WP_EE3:
	case WP_ACP_SNIPER_RIFLE:
	case WP_BOWCASTER_CLASSIC:
		return qtrue;
	default:
		break;
	}

	return qfalse;
}
// theses weapons only have scopes with no charge options.
qboolean WeaponIsSniperNoCharge ( int weapon )
{
	switch (weapon)
	{
	case WP_A280:
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
	// char	scopename[64],							char scopeModel[128],							char scopeModelShader[128],						char gunMaskShader[128],				char	maskShader[128],					char	insertShader[128],					char	lightShader[128],						char	tickShader[128],				char	chargeShader[128],				int scopeViewX,	int scopeViewY,	int scopeViewW,	int scopeViewH,		char	zoomStartSound[128],				char	zoomEndSound[128]										qboolean instantZoom,	float scopeZoomMin	float scopeZoomMax	float scopeZoomSpeed
	"No Scope",										"",												"",												"",										"",											"",											"",												"",										"",										0,				0,				640,			480,				"",											"",																qtrue,					1.0,				1.0,				0.0,
	"Binoculars",									"",												"",												"",										"",											"",											"",												"",										"",										0,				0,				640,			480,				"sound/interface/zoomstart.wav",			"sound/interface/zoomend.wav",									qfalse,					1.5,				3.0,				0.1,
	"Short Range Viewfinder",						"",												"",												"",										"",											"",											"",												"",										"",										0,				0,				640,			480,				"",											"",																qtrue,					1.2,				2.0,				0.0,
	"Mid Range Viewfinder",							"",												"",												"",										"",											"",											"",												"",										"",										0,				0,				640,			480,				"",											"",																qtrue,					1.5,				5.0,				0.0,
	"Tenloss Disruptor Scope",						"",												"",												"",  									"gfx/2d/cropCircle2",						"gfx/2d/cropCircle",						"gfx/2d/cropCircleGlow",						"gfx/2d/insertTick",					"gfx/2d/crop_charge",					128,			50,				382,			382,				"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",							qfalse,					1.0,				3.0,				0.2,
	"Blastech Bowcaster Scope",						"",												"",												"",										"gfx/2d/bowMask",							"gfx/2d/bowInsert",							"",												"",										"",										0,				0,				0,				0,					"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",							qfalse,					1.2,				1.2,				0.4,
	"Blastech EE3 Scope",							"",												"",												"",										"gfx/2d/fett/cropCircle2",					"gfx/2d/fett/cropCircle",					"gfx/2d/fett/cropCircleGlow",					"",										"gfx/2d/fett/crop_charge",				0,				0,				640,			480,				"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",							qfalse,					1.2,				1.2,				0.3,
	"Blastech A280 Scope",							"",												"",												"gfx/2D/arcMask",						"gfx/2d/a280cropCircle2",					"gfx/2d/a280cropCircle",					"",												"",										"",										134,			56,				364,			364,				"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",							qfalse,					1.5,				1.5,				0.2,
	"Blastech DLT 19 Scope",						"",												"",												"gfx/2d/DLT-19_HeavyBlaster/scope_mask_overlay","gfx/2d/DLT-19_HeavyBlaster/scope_mask",					"gfx/2d/a280cropCircle",					"",												"",										"",										134,			56,				364,			364,				"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",	qfalse,					1.0,				1.7,				0.2,
	"BlastTech ACP HAMaR Scope",					"",												"",												"gfx/2d/acp_sniperrifle/scope_mask_overlay", "gfx/2d/acp_sniperrifle/scope_mask",	"",											"",												"",										"",										162,			20,				316,			440,				"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",							qfalse,					1.0,				2.0,				0.2,
	"BlastTech Rifle Bowcaster Scope",				"",												"",												"",										"gfx/2d/Bowcaster/lensmask",				"gfx/2d/Bowcaster/lensmask_zoom",			"",												"",										"",										0,				0,				640,			480,				"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",							qtrue,					1.2,				1.2,				0.0,
	"BlastTech Rifle Heavy Bowcaster Scope",		"",												"",												"gfx/2d/Bowcaster_Heavy/scope_mask_overlay","gfx/2d/Bowcaster_Heavy/scope_mask",	"",											"",												"",										"",										0,				0,				640,			480,				"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",							qtrue,					1.2,				1.2,				0.0,
	"BlastTech Bryar Sniper Scope",					"",												"",												"",										"gfx/2d/Bryar_Rifle/scope_mask",			"",											"",												"",										"",										112,			36,				414,			414,				"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",							qfalse,					1.5,				1.5,				0.2,
	"BlastTech DH-17 Pistol Scope",					"",												"",												"gfx/2d/DH-17_Pistol/scope_mask_overlay","gfx/2d/DH-17_Pistol/scope_mask",			"",											"",												"",										"",										0,				0,				640,			480,				"sound/weapons/disruptor/zoomstart.wav",	"sound/weapons/disruptor/zoomend.wav",							qtrue,					1.2,				1.2,				0.0,
};
