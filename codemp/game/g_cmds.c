// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "bg_saga.h"
//#include "bg_class.h"


#include "ui/menudef.h"			// for the voice chats

void WP_SetSaber( int entNum, saberInfo_t *sabers, int saberNum, const char *saberName );

void Cmd_NPC_f( gentity_t *ent );
void SetTeamQuick(gentity_t *ent, int team, qboolean doBegin);

//[EXPsys]
// But remember to update this structure as well.
int experienceLevel[NUM_EXP_LEVELS] = {			// Experience needed to proceed to next level.
	//0,						// EXP_LEVEL_0
	400,					// EXP_LEVEL_1
	900,					// EXP_LEVEL_2
	1333,					// EXP_LEVEL_3
	2233,					// EXP_LEVEL_4
	3566,					// EXP_LEVEL_5
	5799,					// EXP_LEVEL_6
	9365,					// EXP_LEVEL_7
	15164,					// EXP_LEVEL_8
	24529,					// EXP_LEVEL_9
	39693,					// EXP_LEVEL_10
};

void LevelMessage(gentity_t *ent, char* message, qboolean silent, int level)
{
	if (!silent && ent->account.level == level)
		trap->SendServerCommand(ent - g_entities, va("chat \"%s\n\"", message));
}

//[CLASSsys]
//void UpdateTrooper1( gentity_t *ent, qboolean silent); // Now it is declared, so you could have the code below. And do this for all of them. But not needed anymore.

// We'll make this the function to deal with leveling the character.
/*
void UpdateTrooper1(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Alle spillere kommer til å få "base-settings" når de joiner spillet. Her legger du bare inn forandringene som er nødvendige.
	// Det er mulig at du kommer til å miste disse forandringene når spilleren dør nå. Det må vi i så fall fikse. Men la oss prøve nå.
	// Du må legge inn forandringer per class her.
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		LevelMessage(ent, "No Upgrade before Lvl 2", silent, EXP_LEVEL_1);
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_BLASTER] = 200;
		LevelMessage(ent, "Your Pistol is now Lvl 2 in dmg, and a ammo upgrade from 150 to 200.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_2;
		client->skillLevel[SK_REPEATER] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Pistol Fire rate is now Lvl 2, And your repeater is now Lvl 2 in dmg.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		LevelMessage(ent, "No Upgrade in Lvl 4 wait for next Lvlup", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.stats[SK_REPEATERUPGRADE] = FORCE_LEVEL_2;
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_THERMAL] = 1;
		LevelMessage(ent, "Your Repeater Fire rate is now Lvl 2 and thermal is lvl 2 with ammo from 1 to 2.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_DETPACK] = 3;
		client->skillLevel[SK_SENTRY] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Detpack is now Lvl 3 with ammo upgrade from 1 to 3 and sentry gun is lvl 2", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		LevelMessage(ent, "No Upgrade in Lvl 7 wait for next Lvlup.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->skillLevel[SK_REPEATER] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_METAL_BOLTS] = 250;
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_DETPACK] = 5;
		LevelMessage(ent, "Your Repeater and Detpack is now Lvl 3 with Metal bolt ammo upgrade and 2 more Detpack", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_3;
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_3;
		LevelMessage(ent, "Your Postol and Pistol fire rate Full upgrade to Lvl 3.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.stats[SK_REPEATERUPGRADE] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_METAL_BOLTS] = 300;
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_THERMAL] = 3;
		client->skillLevel[SK_SENTRY] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Repeater, Thermal, Sentry gun is now Lvl 3 with Metal Ammo on 300 Thermal ammo 3 \n Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateTrooper2(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client; // When making a function, and not a function decleration, you do not add the ; here

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		LevelMessage(ent, "EOC CLASS Lvl 1 UPDATED", silent, EXP_LEVEL_1);
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_BLASTER] = 175;
		LevelMessage(ent, "Your Pistol is now Lvl 2 and Ammo upgrade from 150 to 175.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->skillLevel[SK_SEEKER] = FORCE_LEVEL_2;
		client->ps.stats[STAT_MAX_DODGE] = 60;
		LevelMessage(ent, "Your seeker is now Lvl 2 and Dodge is Upgrade from 50 to 60.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->ps.stats[STAT_ARMOR] = 35;
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Pistol Fire rate is now Lvl 2 and amor Upgrade set to from 25 to 35.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_DETPACK] = 3;
		LevelMessage(ent, "Your Detpack is now Lvl 2 Ammo Update from 1 to 3.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->ps.stats[STAT_MAX_DODGE] = 75;
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_3;
		LevelMessage(ent, "Your Pistol fire rate is now Lvl 3 and is full upgrade.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		LevelMessage(ent, "No Upgrade in Lvl 7 wait for next Lvlup.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_3;
		client->ps.stats[STAT_ARMOR] = 70;
		LevelMessage(ent, "Your Pistol is now Lvl 3 And Amor set from 35 to 70.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->skillLevel[SK_SEEKER] = FORCE_LEVEL_3;
		client->ps.stats[STAT_MAX_DODGE] = 85;
		LevelMessage(ent, "Your Seeker is now Lvl 3 And Dodge set from 75 to 85.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.stats[STAT_MAX_DODGE] = 100;
		client->ps.stats[STAT_ARMOR] = 100;
		LevelMessage(ent, "Amor and Dodge set to 100 Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}

}

void UpdateTrooper3(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_2;
		client->ps.stats[STAT_ARMOR] = 35;
		LevelMessage(ent, "Your Pistol is now Updated Lvl 2 And Amor set from 25 to 35.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_2;
		client->skillLevel[SK_DISRUPTOR] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Dirsuptor, Pistol fire rate is Updated to Lvl 2.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->ps.ammo[AMMO_POWERCELL] = 20;
		client->ps.stats[STAT_MAX_DODGE] = 65;
		LevelMessage(ent, "Your Dirsuptor Powercell Updated by 5 And Dodge set to 65.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		LevelMessage(ent, "No Upgrade in Lvl 5 wait for next Lvlup.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->skillLevel[SK_ROCKET] = FORCE_LEVEL_2;
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_3;
		LevelMessage(ent, "Your Rocket Lucher is Updated to Lvl 2 And Pistol Updated to Lvl 3.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_3;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		client->ps.stats[STAT_ARMOR] = 50;
		LevelMessage(ent, "Your Pistol Fire rate updated is now Lvl 3 Dodge Updated set to 65 Amor set to 50.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		LevelMessage(ent, "No Upgrade in Lvl 9 wait for next Lvlup.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->skillLevel[SK_DISRUPTOR] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_POWERCELL] = 25;
		LevelMessage(ent, "Your Dirsuptor Rifel is now Lvl 3 irsuptor Powercell Updated by 5.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->skillLevel[SK_ROCKET] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_ROCKETS] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 100;
		client->ps.stats[STAT_ARMOR] = 75;
		LevelMessage(ent, "Your Rocket Luncher is now Lvl 3 Ammo set to 3 Dodge set to 100 Amor 75\n Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateJediKnight1(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.ammo[AMMO_FORCE] = 60;
		LevelMessage(ent, "Your Force Push, Saber Defense is now Lvl 2 Your ForcePower Rate set to 60.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 2;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 2;
		LevelMessage(ent, "Your Pull, Jump, Saber Throw is now Lvl 2.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->ps.fd.forcePowerLevel[FP_SEE] = 2;
		client->ps.fd.forcePowerLevel[FP_GRIP] = 2;
		LevelMessage(ent, "Your Force Seeing, Grip is now Lvl 2.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		LevelMessage(ent, "No Upgrade in Lvl 5 wait for next Lvlup.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->ps.ammo[AMMO_FORCE] = 85;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 3;
		LevelMessage(ent, "Your Saber Throw is now Lvl 3 Force Power rate set to 85 Dodge Set to 75.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 3;
		LevelMessage(ent, "Your Force Push, Jump is now Lvl 3.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.ammo[AMMO_FORCE] = 100;
		client->ps.stats[STAT_MAX_DODGE] = 85;
		LevelMessage(ent, "Your Forcepower rate set to 100 Dodge set to 85 is now Lvl 2.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		LevelMessage(ent, "No Upgrade in Lvl 9 wait for next Lvlup.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.stats[STAT_MAX_DODGE] = 85;
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 3;
		client->ps.fd.forcePowerLevel[FP_SEE] = 3;
		client->ps.fd.forcePowerLevel[FP_GRIP] = 3;
		LevelMessage(ent, "Your Saber Defense, Seeing, Grip is now Lvl 3 And Dodge set Up to 100, Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateJediKnight2(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 2;
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.stats[STAT_MAX_DODGE] = 65;
		LevelMessage(ent, "Your force Push, saberthrow is now Lvl 2.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		LevelMessage(ent, "No Upgrade in Lvl 3 wait for next Lvlup.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		client->ps.ammo[AMMO_FORCE] = 65;
		LevelMessage(ent, "Your saber defense is now Lvl 2 And forcepower rate set to 65.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		client->ps.fd.forcePowerLevel[FP_SPEED] = 2;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		LevelMessage(ent, "Your force Pull, Speed is now Lvl 2 Dodge set to 75.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 3;
		LevelMessage(ent, "Your force Push, Jump is now Lvl 3.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->ps.fd.forcePowerLevel[FP_SEE] = 2;
		client->ps.ammo[AMMO_FORCE] = 75;
		LevelMessage(ent, "Your force Seeing is now Lvl 2 ForcePower Rate is set to 75.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		LevelMessage(ent, "No Upgrade in Lvl 8 wait for next Lvlup.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_SPEED] = 3;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 3;
		LevelMessage(ent, "Your force Speed, Saber throw is now Lvl 3.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.ammo[AMMO_FORCE] = 100;
		client->ps.stats[STAT_MAX_DODGE] = 100;
		client->ps.fd.forcePowerLevel[FP_PULL] = 3;
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;

		LevelMessage(ent, "Your force Pull, Saber defense is now Lvl 3 Forcepower rate and Dodge to 100, Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateJediKnight3(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_HEAL] = 2;
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 2;
		client->ps.stats[STAT_MAX_DODGE] = 65;
		LevelMessage(ent, "Your force Healing, Push, Jump  is now Lvl 2.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		LevelMessage(ent, "No Upgrade in Lvl 3 wait for next Lvlup.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		client->ps.ammo[AMMO_FORCE] = 75;
		LevelMessage(ent, "Your force Pull, Saber Defense is now Lvl 2, Forcepower rate is 75.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.fd.forcePowerLevel[FP_SEE] = 2;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 2;
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		LevelMessage(ent, "Your Force Seeing, Saber Throw Is Lvl 2, Push Lvl 3.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{

		LevelMessage(ent, "No Upgrade in Lvl 6 wait for next Lvlup.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->ps.fd.forcePowerLevel[FP_HEAL] = 3;
		client->ps.ammo[AMMO_FORCE] = 85;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		LevelMessage(ent, "Your force Healing is now Lvl 3, ForcePower rate set to 85, Dodge set to 75.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.fd.forcePowerLevel[FP_PULL] = 3;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 3;
		LevelMessage(ent, "Your force Pull, Jump is now Lvl 3.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] = 3;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 3;
		LevelMessage(ent, "Your Saber Offense, Saber Throw is now Lvl 3.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 3;
		client->ps.ammo[AMMO_FORCE] = 100;
		client->ps.stats[STAT_MAX_DODGE] = 100;
		LevelMessage(ent, "Your Saber Defense is now Lvl 3, ForcePower rate set to 100, Dodge set to 100 Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateSmuggler1(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_2;
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Pistel|Thermal is now Lvl 2.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.stats[STAT_ARMOR] = 35;
		client->ps.stats[STAT_MAX_DODGE] = 35;
		client->ps.ammo[AMMO_THERMAL] = 2;
		LevelMessage(ent, "Your AMOR|DODGE has been increesed by 10 And Thermal Ammo Upgrade by 2", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_2;
		client->skillLevel[SK_BLASTER] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Blaster|Blaster fire rate is now Lvl 2.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		LevelMessage(ent, "There Is no Upgrade in This Lvl", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->skillLevel[SK_BOWCASTER] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_BLASTER] = 200;
		client->skillLevel[SK_CLOAK] = FORCE_LEVEL_2;
		client->ps.stats[STAT_ARMOR] = 50;
		LevelMessage(ent, "Your BowCaster|Cloak Ablilty is now Lvl 2, and Ammo Upgrade by 50, Amor upgrade by 15", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{

		client->ps.stats[STAT_MAX_DODGE] = 50;
		client->skillLevel[SK_BLASTER] = FORCE_LEVEL_3;
		client->skillLevel[SK_SEEKER] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Balster is now Lvl 3 Seeker Lvl 2, Dodge Upgrade by 15", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		LevelMessage(ent, "No Lvl Upgrade in this Lvl Here", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_BLASTER] = 250;
		client->ps.ammo[AMMO_THERMAL] = 5;
		client->skillLevel[SK_SEEKER] = FORCE_LEVEL_3;
		LevelMessage(ent, "Your Blaster fire rate|Seeker is now Lvl 3, Ammo Upgrade by 50|Thermal Upgrade by 2", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_3;
		client->skillLevel[SK_BOWCASTER] = FORCE_LEVEL_3;
		client->skillLevel[SK_CLOAK] = FORCE_LEVEL_3;
		client->ps.stats[STAT_ARMOR] = 75;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		LevelMessage(ent, "Your Pisto|Bowcaster|Cloak is now Lvl 3, Amor|Dodge Upgrade by 25 Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateSmuggler2(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_2;
		client->ps.stats[STAT_ARMOR] = 35;
		client->ps.stats[STAT_MAX_DODGE] = 35;
		LevelMessage(ent, "Your Pistol is now Lvl 2, Amor|Dodge Upgrade by 10.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->skillLevel[SK_DISRUPTOR] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_POWERCELL] = 30;
		LevelMessage(ent, "Your Disruptor is now Lvl 2, Ammo powercell upgrade by 10", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->ps.ammo[AMMO_METAL_BOLTS] = 175;
		client->skillLevel[SK_FLECHETTE] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Flechette Gun is now Lvl 2, Ammo Metal bolts Upgrade by 25.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->skillLevel[SK_FORCEFIELD] = FORCE_LEVEL_2;
		client->ps.stats[STAT_MAX_DODGE] = 45;
		client->ps.stats[STAT_ARMOR] = 45;
		LevelMessage(ent, "Your ForceField is now Lvl 2, Amor|Dodge Upgrade by 10.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		LevelMessage(ent, "No upgrade in this Lvl", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_3;
		client->skillLevel[SK_DISRUPTOR] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_POWERCELL] = 45;
		LevelMessage(ent, "Your Sistol|Disruptor is now Lvl 3, Ammo Powercell Upgrade by 15.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		LevelMessage(ent, "No Upgrade in this Lvl", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->skillLevel[SK_FORCEFIELD] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_METAL_BOLTS] = 200;
		LevelMessage(ent, "Your Forcefield now Lvl 3, Ammo Metal bolts Upgrade by 25.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->skillLevel[SK_FLECHETTE] = FORCE_LEVEL_3;
		client->ps.stats[STAT_ARMOR] = 65;
		client->ps.stats[STAT_MAX_DODGE] = 65;
		client->ps.ammo[AMMO_POWERCELL] = 60;
		client->ps.ammo[AMMO_METAL_BOLTS] = 250;
		LevelMessage(ent, "Your Flechette Gun is now Lvl 3, Amor|Dodge upgrade by 20, Ammo Powercell upgrade by 15\n Ammo Metal bolts upgrade by 50 Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}
//
void UpdateSmuggler3(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->skillLevel[SK_BLASTER] = FORCE_LEVEL_2;
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_2;
		client->ps.stats[STAT_ARMOR] = 45;
		client->ps.stats[STAT_MAX_DODGE] = 35;
		LevelMessage(ent, "Your Blaster|Blaster fire rate is now Lvl 2, Amor Upgrade by 20 doge by 10.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.ammo[AMMO_BLASTER] = 125;
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_DETPACK] = 5;
		LevelMessage(ent, "Your Thermal is now Lvl 2, Blaster ammo upgrade by 25 Detpack by 4.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		LevelMessage(ent, "No Upgrade in this Lvl.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_2;
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Pistol|Thermal is now Lvl 2.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_DETPACK] = 7;
		client->ps.ammo[AMMO_BLASTER] = 150;
		LevelMessage(ent, "Your Detpack is now Lvl 3, Detpack ammo Upgrade by 2, Ammo blaster by 25.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->skillLevel[SK_SEEKER] = FORCE_LEVEL_2;
		client->skillLevel[SK_SENTRY] = FORCE_LEVEL_2;
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_3;
		LevelMessage(ent, "Your Seeker|Sentry is now Lvl 2,Blaster fire rate is now Lvl 3.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.stats[STAT_ARMOR] = 55;
		client->ps.stats[STAT_MAX_DODGE] = 50;
		client->ps.ammo[AMMO_BLASTER] = 175;
		LevelMessage(ent, "Your Amor is Upgrade by 10 dodge by 15, Ammo blaster by 25.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_3;
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_THERMAL] = 3;
		client->ps.ammo[AMMO_DETPACK] = 10;
		LevelMessage(ent, "Your Pistol|Thermal is now Lvl 3, Ammo Thermal Upgrade by 2, Detpack by 3.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.stats[STAT_ARMOR] = 75;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		client->skillLevel[SK_SEEKER] = FORCE_LEVEL_2;
		client->skillLevel[SK_SENTRY] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_BLASTER] = 225;
		LevelMessage(ent, "Your Seeker|Sentry is now Lvl 3, Amor|Dodge Upgrade to Max Blaster Ammo by 25 Full Upgrade Class succesfull..", silent, EXP_LEVEL_10);
	}
}
//
void UpdateJediConsular1(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] = 2;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 2;
		LevelMessage(ent, "Your SaberOffense|Jump is now Lvl 2.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.stats[STAT_MAX_DODGE] = 55;
		LevelMessage(ent, "Your Push now Lvl 2, Dodge has been Upgrade by 5.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		LevelMessage(ent, "No Upgrades in this Lvl", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.fd.forcePowerLevel[FP_TELEPATHY] = 2;
		client->ps.fd.forcePowerLevel[FP_ABSORB] = 2;
		client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] = 2;
		LevelMessage(ent, "Your Mindtrick|Absorb|Teamheal is now Lvl 2.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->ps.stats[STAT_MAX_DODGE] = 65;
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		client->ps.fd.forcePowerLevel[FP_SEE] = 2;
		LevelMessage(ent, "Your SaberDefense|Seeing is now Lvl 2, Dodge Upgrade by 10.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		LevelMessage(ent, "NO Upgrades in this Lvl", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		LevelMessage(ent, "Your Push is now Lvl 3, Pull is Lvl 2.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_TELEPATHY] = 3;
		client->ps.fd.forcePowerLevel[FP_PULL] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 70;
		LevelMessage(ent, "Your Midtrick|Pull is now Lvl 3 Dodge Upgrade by 5.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 1;
		client->ps.fd.forcePowerLevel[FP_SEE] = 1;
		client->ps.fd.forcePowerLevel[FP_ABSORB] = 1;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 1;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		LevelMessage(ent, "Your SaberDefense|Seeing|Absorb|Jump is now Lvl 3, Dodge Upgrade by 5 Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}
//
void UpdateJediConsular2(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] = 2;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 2;
		LevelMessage(ent, "Your SaberOffense|Jump is now Lvl 2.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.stats[STAT_MAX_DODGE] = 55;
		LevelMessage(ent, "Your Push now Lvl 2, Dodge has been Upgrade by 5.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		LevelMessage(ent, "No Upgrades in this Lvl", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.fd.forcePowerLevel[FP_TELEPATHY] = 2;
		client->ps.fd.forcePowerLevel[FP_PROTECT] = 2;
		client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] = 2;
		LevelMessage(ent, "Your Mindtrick|Protect|Teamheal is now Lvl 2.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->ps.stats[STAT_MAX_DODGE] = 65;
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		client->ps.fd.forcePowerLevel[FP_SEE] = 2;
		LevelMessage(ent, "Your SaberDefense|Seeing is now Lvl 2, Dodge Upgrade by 10.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		LevelMessage(ent, "NO Upgrades in this Lvl", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] = 3;
		LevelMessage(ent, "Your Push|TeamHeal is now Lvl 3, Pull is Lvl 2.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_TELEPATHY] = 3;
		client->ps.fd.forcePowerLevel[FP_PULL] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 70;
		LevelMessage(ent, "Your Midtrick|Pull is now Lvl 3 Dodge Upgrade by 5.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 1;
		client->ps.fd.forcePowerLevel[FP_SEE] = 1;
		client->ps.fd.forcePowerLevel[FP_PROTECT] = 1;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 1;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		LevelMessage(ent, "Your SaberDefense|Seeing|Protect|Jump is now Lvl 3, Dodge Upgrade by 5 Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}
//
void UpdateJediConsular3(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] = 2;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 2;
		LevelMessage(ent, "Your SaberOffense|Jump is now Lvl 2.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.stats[STAT_MAX_DODGE] = 55;
		LevelMessage(ent, "Your Push now Lvl 2, Dodge has been Upgrade by 5.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		LevelMessage(ent, "No Upgrades in this Lvl", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 2;
		client->ps.fd.forcePowerLevel[FP_TEAM_FORCE] = 2;
		client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] = 2;
		LevelMessage(ent, "Your SaberThrow|TeamForce|Teamheal is now Lvl 2.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->ps.stats[STAT_MAX_DODGE] = 65;
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		client->ps.fd.forcePowerLevel[FP_SEE] = 2;
		LevelMessage(ent, "Your SaberDefense|Seeing is now Lvl 2, Dodge Upgrade by 10.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		LevelMessage(ent, "NO Upgrades in this Lvl", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] = 3;
		LevelMessage(ent, "Your Push|TeamHeal is now Lvl 3, Pull is Lvl 2.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 3;
		client->ps.fd.forcePowerLevel[FP_PULL] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 70;
		LevelMessage(ent, "Your SaberThrow|Pull is now Lvl 3 Dodge Upgrade by 5.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 1;
		client->ps.fd.forcePowerLevel[FP_SEE] = 1;
		client->ps.fd.forcePowerLevel[FP_TEAM_FORCE] = 1;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 1;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		LevelMessage(ent, "Your SaberDefense|Seeing|TeamForce|Jump is now Lvl 3, Dodge Upgrade by 5 Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateBountyhunter1(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->skillLevel[SK_FLECHETTE] = FORCE_LEVEL_2;
		client->ps.stats[STAT_ARMOR] = 30;
		LevelMessage(ent, "Your Flechetter gun is now Lvl 2, Amor Upgrade by 5.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.stats[STAT_MAX_DODGE] = 30;
		client->skillLevel[SK_REPEATERUPGRADE] = FORCE_LEVEL_2;
		client->ps.jetpackFuel = 50;
		LevelMessage(ent, "Your Repeater fire rate is now Lvl 2, Dodge Upgrade by 5 JetFuel by 25.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->skillLevel[SK_REPEATER] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_METAL_BOLTS] = 115;
		client->ps.ammo[AMMO_THERMAL] = 2;
		LevelMessage(ent, "Your Repeater is now Lvl 2, Repeater Metalbolts Upgrade by 15, Thermal Upgrade by 1.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		LevelMessage(ent, "NO Upgrades in this Lvl", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_DETPACK] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 45;
		client->ps.stats[STAT_ARMOR] = 45;
		LevelMessage(ent, "Your Detpack is now Lvl 2, Detpack ammo Upgrade by 2 Dodge|Amor Upgrade by 15 .", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->skillLevel[SK_JETPACK] = FORCE_LEVEL_2;
		client->skillLevel[SK_FLAMETHROWER] = FORCE_LEVEL_2;
		client->ps.jetpackFuel = 50;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		client->ps.stats[STAT_ARMOR] = 75;
		LevelMessage(ent, "Your Jetpack|FlameThrow is now Lvl 2 Fuel Upgrade by 25, Dodge|Amor upgrade by 30.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.ammo[AMMO_METAL_BOLTS] = 200;
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_THERMAL] = 3;
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_DETPACK] = 5;
		client->ps.jetpackFuel = 75;
		LevelMessage(ent, "Your Thermal|Detpack is now Lvl 3, Thermal Upgrade by 1 Detpack By 3 JetFuel by 25.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->skillLevel[SK_FLECHETTE] = FORCE_LEVEL_3;
		client->skillLevel[SK_FLAMETHROWER] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_METAL_BOLTS] = 250;
		LevelMessage(ent, "Your Flechetter|Flamethrow is now Lvl 3, MetalBolts Upgrade by 50.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->skillLevel[SK_REPEATERUPGRADE] = FORCE_LEVEL_3;
		client->skillLevel[SK_REPEATER] = FORCE_LEVEL_3;
		client->ps.jetpackFuel = 100;
		client->ps.stats[STAT_MAX_DODGE] = 100;
		client->ps.stats[STAT_ARMOR] = 100;
		LevelMessage(ent, "Your Repeater|Repeater Fire rate is now Lvl 3, JetFuel Upgrade by 25 Dodge|Amor by 25 Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}
//
void UpdateBountyhunter2(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_2;
		client->ps.stats[STAT_ARMOR] = 30;
		LevelMessage(ent, "Your Pistol is now Lvl 2, Amor Upgrade by 5.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.stats[STAT_MAX_DODGE] = 30;
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_2;
		client->ps.jetpackFuel = 50;
		LevelMessage(ent, "Your Blaster fire rate is now Lvl 2, Dodge Upgrade by 5 JetFuel by 25.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->skillLevel[SK_BLASTER] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_BLASTER] = 165;
		LevelMessage(ent, "Your Repeater is now Lvl 2, Blaster Ammo Upgrade by 15, Thermal Upgrade by 1.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		LevelMessage(ent, "NO Upgrades in this Lvl", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_DETPACK] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 45;
		client->ps.stats[STAT_ARMOR] = 45;
		LevelMessage(ent, "Your Detpack is now Lvl 2, Detpack ammo Upgrade by 2 Dodge|Amor Upgrade by 15 .", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->skillLevel[SK_JETPACK] = FORCE_LEVEL_2;
		client->skillLevel[SK_FORCEFIELD] = FORCE_LEVEL_2;
		client->ps.jetpackFuel = 50;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		client->ps.stats[STAT_ARMOR] = 75;
		LevelMessage(ent, "Your Jetpack|Forcefield is now Lvl 2 Fuel Upgrade by 25, Dodge|Amor upgrade by 30.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.ammo[AMMO_BLASTER] = 185;
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_3;
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_DETPACK] = 5;
		client->ps.jetpackFuel = 75;
		LevelMessage(ent, "Your Thermal|Detpack is now Lvl 3, Blaster Ammo Updated by 20, Detpack By 3, JetFuel by 25.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->skillLevel[SK_FLECHETTE] = FORCE_LEVEL_3;
		client->skillLevel[SK_FORCEFIELD] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_BLASTER] = 200;
		LevelMessage(ent, "Your Flechetter|Forcefield is now Lvl 3, Blaster Ammo Upgrade by 15.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_3;
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_3;
		client->ps.jetpackFuel = 100;
		client->ps.stats[STAT_MAX_DODGE] = 100;
		client->ps.stats[STAT_ARMOR] = 100;
		LevelMessage(ent, "Your Pistol|Blaster Fire rate is now Lvl 3, JetFuel Upgrade by 25 Dodge|Amor by 25 Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateBountyhunter3(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->skillLevel[SK_REPEATER] = FORCE_LEVEL_2;
		client->ps.stats[STAT_ARMOR] = 40;
		LevelMessage(ent, "Your Repeater gun is now Lvl 2, Amor Upgrade by 10.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.stats[STAT_MAX_DODGE] = 40;
		client->skillLevel[SK_REPEATERUPGRADE] = FORCE_LEVEL_2;
		client->ps.jetpackFuel = 50;
		LevelMessage(ent, "Your Repeater fire rate is now Lvl 2, Dodge Upgrade by 5, JetFuel by 25.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_METAL_BOLTS] = 150;
		client->ps.ammo[SK_DETPACK] = 7;
		client->skillLevel[SK_SENTRY] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Blaster fire rate|Sentry Gun is now Lvl 2, Detpack Upgrade by 5, Metalbolts by 50 .", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->skillLevel[SK_JETPACK] = FORCE_LEVEL_2;
		client->skillLevel[SK_FLAMETHROWER] = FORCE_LEVEL_2;
		client->ps.jetpackFuel = 50;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		client->ps.stats[STAT_ARMOR] = 75;
		LevelMessage(ent, "Your Jetpack|FlameThrow is now Lvl 2, Fuel Upgrade by 25, Dodge|Amor upgrade by 30", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_DETPACK] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 45;
		client->ps.stats[STAT_ARMOR] = 45;
		client->ps.ammo[AMMO_METAL_BOLTS] = 200;
		LevelMessage(ent, "Your Detpack is now Lvl 2, Detpack ammo Upgrade by 2 Dodge|Amor Upgrade by 15 MetalBolt 50.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		LevelMessage(ent, "NO Upgrades in this Lvl.", silent, EXP_LEVEL_);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.ammo[AMMO_METAL_BOLTS] = 2;
		client->skillLevel[SK_BLASTER] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_THERMAL] = 3;
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_DETPACK] = 5;
		client->ps.jetpackFuel = 75;
		LevelMessage(ent, "Your Blaster Rifel is now Lvl 2, Thermal Upgrade by 1, Detpack By 3, JetFuel by 25.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->skillLevel[SK_SENTRY] = FORCE_LEVEL_3;
		client->skillLevel[SK_FLAMETHROWER] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_METAL_BOLTS] = 250;
		LevelMessage(ent, "Your Sentry Gun|Flamethrow is now Lvl 3, MetalBolts Upgrade by 50.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->skillLevel[SK_REPEATERUPGRADE] = FORCE_LEVEL_3;
		client->skillLevel[SK_REPEATER] = FORCE_LEVEL_3;
		client->ps.jetpackFuel = 100;
		client->ps.stats[STAT_MAX_DODGE] = 100;
		client->ps.stats[STAT_ARMOR] = 100;
		client->ps.ammo[AMMO_METAL_BOLTS] = 300;
		LevelMessage(ent, "Your Repeater|Repeater Fire rate is now Lvl 3, JetFuel Upgrade by 25, MetalBolts by 50 Dodge|Amor by 25, Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateSithworrior1(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		LevelMessage(ent, "EOC CLASS LOADED FOR SITH WORRIOR CLASS 1", silent, EXP_LEVEL_1);
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.stats[STAT_MAX_DODGE] = 65;
		LevelMessage(ent, "Your force Push, Saber Defense is now Lvl 2, Dodge set to 65.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 2;
		client->ps.fd.forcePowerLevel[FP_LIGHTNING] = 2;
		client->ps.ammo[AMMO_FORCE] = 75;
		LevelMessage(ent, "Your force Lightning, Jump is now Lvl 2.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		LevelMessage(ent, "No Upgrade in Lvl 4 wait for next Lvlup.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.fd.forcePowerLevel[FP_GRIP] = 2;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 2;
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		LevelMessage(ent, "Your force Grip, Pull, Saber throw is now Lvl 2.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->ps.stats[STAT_MAX_DODGE] = 75;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 3;
		LevelMessage(ent, "Your force Jump is now Lvl 3, Dodge set to 75.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->ps.stats[STAT_MAX_DODGE] = 75;
		LevelMessage(ent, "No Upgrade in Lvl 7 wait for next Lvlup.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 85;
		LevelMessage(ent, "Your Saber Defense is now Lvl 3, Dodge set to 85.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		client->ps.fd.forcePowerLevel[FP_GRIP] = 3;
		LevelMessage(ent, "Your force Push, Grip is now Lvl 3.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 3;
		client->ps.fd.forcePowerLevel[FP_LIGHTNING] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 100;
		client->ps.ammo[AMMO_FORCE] = 100;
		LevelMessage(ent, "Your force Lightning, Saber Throw is now Lvl 3 Dodge set to 100 \n Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateSithworrior2(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.stats[STAT_MAX_DODGE] = 65;
		LevelMessage(ent, "Your force Push, Saber Defense is now Lvl 2, Dodge set to 65.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 2;
		client->ps.fd.forcePowerLevel[FP_LIGHTNING] = 2;
		client->ps.ammo[AMMO_FORCE] = 75;
		LevelMessage(ent, "Your force Lightning, Jump is now Lvl 2.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		LevelMessage(ent, "No Upgrade in Lvl 4 wait for next Lvlup.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.fd.forcePowerLevel[FP_GRIP] = 2;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 2;
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		LevelMessage(ent, "Your force Grip, Pull, Saber throw is now Lvl 2.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->ps.stats[STAT_MAX_DODGE] = 75;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 3;
		LevelMessage(ent, "Your force Jump is now Lvl 3, Dodge set to 75.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		LevelMessage(ent, "No Upgrade in Lvl 7 wait for next Lvlup.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 85;
		LevelMessage(ent, "Your Saber Defense is now Lvl 3, Dodge set to 85.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		client->ps.fd.forcePowerLevel[FP_GRIP] = 3;
		LevelMessage(ent, "Your force Push, Grip is now Lvl 3.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 3;
		client->ps.fd.forcePowerLevel[FP_LIGHTNING] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 100;
		LevelMessage(ent, "Your force Lightning, Saber Throw is now Lvl 3, Dodge set to 100 \n Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}
void UpdateSithworrior3(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.stats[STAT_MAX_DODGE] = 65;
		LevelMessage(ent, "Your force Push, Saber Defense is now Lvl 2, Dodge set to 65.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.fd.forcePowerLevel[FP_SPEED] = 2;
		client->ps.fd.forcePowerLevel[FP_LIGHTNING] = 2;
		client->ps.ammo[AMMO_FORCE] = 75;
		LevelMessage(ent, "Your force Speed, Jump is now Lvl 2, ForcePower rate set to 75.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		LevelMessage(ent, "No Upgrade in Lvl 4 wait for next Lvlup.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.fd.forcePowerLevel[FP_TELEPATHY] = 2;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 2;
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		LevelMessage(ent, "Your force MindTrick, Pull, Saber throw is now Lvl 2.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->ps.stats[STAT_MAX_DODGE] = 75;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 3;
		client->ps.fd.forcePowerLevel[FP_SPEED] = 3;
		LevelMessage(ent, "Your force Jump|Speed is now Lvl 3, Dodge set to 75.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		LevelMessage(ent, "No Upgrade in Lvl 7 wait for next Lvlup.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 85;
		LevelMessage(ent, "Your Saber Defense is now Lvl 3, Dodge set to 85.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		client->ps.fd.forcePowerLevel[FP_TELEPATHY] = 3;
		client->ps.fd.forcePowerLevel[FP_GRIP] = 2;
		LevelMessage(ent, "Your force Push, Mindtrick is now Lvl 3 Grip Lvl 2.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 3;
		client->ps.fd.forcePowerLevel[FP_LIGHTNING] = 3;
		client->ps.stats[STAT_MAX_DODGE] = 100;
		client->ps.fd.forcePowerLevel[FP_GRIP] = 3;
		LevelMessage(ent, "Your force Lightning, Saber Throw, Grip is now Lvl 3, Dodge set to 100 \n Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateImperialAgent1(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_2;
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_2;
		client->ps.stats[STAT_ARMOR] = 35;
		client->ps.stats[STAT_MAX_DODGE] = 35;
		LevelMessage(ent, "Your Pistol|Detpack is now Lvl 2, Amor|Dodge Upgrade by 10.", silent, EXP_LEVEL_2);
	}if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.cloakFuel = 75;
		client->skillLevel[SK_SENTRY] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_BLASTER] = 150;
		LevelMessage(ent, "Your Sentry Gun is now Lvl 2, Cloak Battery Upgrade by 25, Ammo Blaster by 100.", silent, EXP_LEVEL_3);
	}if (ent->account.level >= EXP_LEVEL_4)
	{
		client->skillLevel[SK_BOWCASTER] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_POWERCELL] = 150;
		client->ps.ammo[AMMO_DETPACK] = 5;
		LevelMessage(ent, "Your BowCaster is now Lvl 2, PowerCell Upgrade by 50, Detpack Ammo by 5.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		LevelMessage(ent, "No Upgrade In this Lvl.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_3;
		client->skillLevel[SK_CLOAK] = FORCE_LEVEL_3;
		client->ps.stats[STAT_ARMOR] = 55;
		client->ps.stats[STAT_MAX_DODGE] = 55;
		LevelMessage(ent, "Your Pistol|Cloak is now Lvl 3, Amor|Dodge Upgrade by 20.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_2;
		client->skillLevel[SK_BOWCASTER] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_POWERCELL] = 250;
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_DETPACK] = 8;
		LevelMessage(ent, "Your BowCaster|Detpack is now Lvl 3, Blaster fire Rate Is Lvl 2, Ammo Powercell 50, Detpack Ammo by 3.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.ammo[AMMO_BLASTER] = 150;
		client->ps.stats[STAT_ARMOR] = 80;
		client->ps.stats[STAT_MAX_DODGE] = 80;
		LevelMessage(ent, "Your force Lightning is now Lvl 2 Amor|Dodge by 25.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_RAGE] = 2;
		client->ps.ammo[AMMO_BLASTER] = 200;
		client->ps.ammo[AMMO_POWERCELL] = 300;
		LevelMessage(ent, "Your Streangt is now Lvl 2 Incress powerup for Fast Meleee Attack and Gun Shots, Ammo Blaster|Powercell upgrade by 50.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.fd.forcePowerLevel[FP_RAGE] = 2;
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_2;
		client->skillLevel[SK_SENTRY] = FORCE_LEVEL_3;
		LevelMessage(ent, "Your Strength Now Lvl 2, Blaster Fire rate|Sentry is now Lvl 3, Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateImperialAgent2(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.stats[STAT_ARMOR] = 40;
		client->ps.stats[STAT_MAX_DODGE] = 40;
		client->skillLevel[SK_FORCEFIELD] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Amor|Dodge Upgrade by 15 ForceField is now Lvl 2.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_THERMAL] = 3;
		LevelMessage(ent, "Your Thermal is now Lvl 2, thermal Ammo Upgrade by 2.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		LevelMessage(ent, "No Upgrade in this Lvl.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_METAL_BOLTS] = 150;
		LevelMessage(ent, "Your Pistol is now Lvl 2, Metalbolts Upgrade by 50.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->skillLevel[SK_FLECHETTE] = FORCE_LEVEL_2;
		client->skillLevel[SK_SEEKER] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Flechetter Gun|Seeker is now Lvl 2.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->ps.stats[STAT_ARMOR] = 80;
		client->ps.stats[STAT_MAX_DODGE] = 80;
		client->skillLevel[SK_FORCEFIELD] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your ForceFieald is now Lvl 2, Amor|Dodge Upgraded by 40.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->skillLevel[SK_PISTOL] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_METAL_BOLTS] = 100;
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_THERMAL] = 2;
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_DETPACK] = 5;
		LevelMessage(ent, "Your Pistol|Detpack is now Lvl 3, Thermal Lvl 2.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.stats[STAT_ARMOR] = 85;
		client->ps.stats[STAT_MAX_DODGE] = 85;
		LevelMessage(ent, "Your Amor|Dodge upgrade by 5.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_3;
		client->skillLevel[SK_SEEKER] = FORCE_LEVEL_3;
		client->skillLevel[SK_FORCEFIELD] = FORCE_LEVEL_3;
		client->skillLevel[SK_FLECHETTE] = FORCE_LEVEL_3;
		LevelMessage(ent, "Your Flechetter Gun|ForceField|Thermal Bombs|Seeker is now Lvl 3, Full Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}
void UpdateImperialAgent3(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.ammo[AMMO_BLASTER] = 150;
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_2;
		client->ps.stats[STAT_ARMOR] = 35;
		client->ps.stats[STAT_MAX_DODGE] = 35;
		client->ps.ammo[AMMO_DETPACK] = 3;
		LevelMessage(ent, "Your Thermal is now Lvl 2, Amor|Dodge Upgrade by 10 Ammo Blaster by 50, Detpack Ammo By 2.", silent, EXP_LEVEL_2);
	}if (ent->account.level >= EXP_LEVEL_3)
	{
		LevelMessage(ent, "No Upgrade in this Lvl.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->ps.ammo[AMMO_BLASTER] = 250;
		client->ps.stats[STAT_ARMOR] = 50;
		client->ps.stats[STAT_MAX_DODGE] = 50;
		LevelMessage(ent, "Your Amor|Dodge Upgrade by 15, Ammo Blaster Upgrade by 50", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_2;
		client->skillLevel[SK_BLASTER] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_BLASTER] = 300;
		LevelMessage(ent, "Your Blaster|Blaster Fire Rate is now Lvl 2, Ammo Blaster Upgrade by 50.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		client->skillLevel[SK_CLOAK] = FORCE_LEVEL_2;
		client->ps.cloakFuel = 50;
		client->skillLevel[SK_SENTRY] = FORCE_LEVEL_2;
		LevelMessage(ent, "Your Cloak|Sentry Gun is now Lvl 2 Cloak Battery Upgrade by 25.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->ps.ammo[AMMO_THERMAL] = 3;
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_2;
		client->ps.ammo[AMMO_DETPACK] = 5;
		LevelMessage(ent, "Your Detpack is now Lvl 3, Thermal Ammo Upgrade by 2, Detpack by 2.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->skillLevel[SK_THERMAL] = FORCE_LEVEL_3;
		client->ps.stats[STAT_ARMOR] = 75;
		client->ps.stats[STAT_MAX_DODGE] = 75;
		LevelMessage(ent, "Your Thermal is now Lvl 2´, Amor|Dodge Upgrade by 25.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->skillLevel[SK_CLOAK] = FORCE_LEVEL_3;
		client->ps.cloakFuel = 75;
		client->skillLevel[SK_SENTRY] = FORCE_LEVEL_3;
		LevelMessage(ent, "Your Cloak|Sentry Gun is now Lvl 3, Cloak Battery Upgrade by 25.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->skillLevel[SK_BLASTERRATEOFFIREUPGRADE] = FORCE_LEVEL_3;
		client->skillLevel[SK_BLASTER] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_THERMAL] = 5;
		client->skillLevel[SK_DETPACK] = FORCE_LEVEL_3;
		client->ps.ammo[AMMO_DETPACK] = 7;
		client->ps.cloakFuel = 100;
		LevelMessage(ent, "Your Blaster|Blaster Fire rate|Detpack is now Lvl 3, Thermal Ammo|Detpack Upgrade by 2, Cloak Battery By 25, Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}
void UpdateSithinquisitor1(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;
	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_LIGHTNING] = 3;
		client->ps.fd.forcePowerLevel[FP_TEAM_FORCE] = 2;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 2;
		LevelMessage(ent, "Your force Lightning is now Lvl 3|TeamForce|Jump is now Lvl 2.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		client->ps.fd.forcePowerLevel[FP_SEE] = 2;
		LevelMessage(ent, "Your Push|Pull|Seeing is now Lvl 2.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->ps.fd.forcePowerLevel[FP_GRIP] = 2;
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		LevelMessage(ent, "Your SaberDefense|Grip is now Lvl 2.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] = 3;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 2;
		LevelMessage(ent, "Your SaberOffense is now Lvl 3|Saberthrow Lvl 2.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		LevelMessage(ent, "No Upgrade in this Lvl.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->ps.fd.forcePowerLevel[FP_GRIP] = 3;
		client->ps.fd.forcePowerLevel[FP_PULL] = 3;
		LevelMessage(ent, "Your force Grip|Pull is now Lvl 3.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.fd.forcePowerLevel[FP_RAGE] = 2;
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		LevelMessage(ent, "Your Push is now Lvl 3.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_SEE] = 3;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 3;
		LevelMessage(ent, "Your force Seeing|Saberthrow is now Lvl 3.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 3;
		client->ps.fd.forcePowerLevel[FP_TEAM_FORCE] = 3;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 3;
		LevelMessage(ent, "Your SaberDefense|Jump|TeamForce is now Lvl 3, Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}
void UpdateSithinquisitor2(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] = 2;
		client->ps.fd.forcePowerLevel[FP_TEAM_FORCE] = 2;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 2;
		LevelMessage(ent, "Your TeamHeal|TeamForce|Jump is now Lvl 2.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		client->ps.fd.forcePowerLevel[FP_SEE] = 2;
		LevelMessage(ent, "Your Push|Pull|Seeing is now Lvl 2.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->ps.fd.forcePowerLevel[FP_DRAIN] = 2;
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		LevelMessage(ent, "Your SaberDefense|Draine is now Lvl 2.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] = 3;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 2;
		LevelMessage(ent, "Your SaberOffense is now Lvl 3|Saberthrow Lvl 2.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		LevelMessage(ent, "No Upgrade in this Lvl.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->ps.fd.forcePowerLevel[FP_DRAIN] = 3;
		client->ps.fd.forcePowerLevel[FP_PULL] = 3;
		LevelMessage(ent, "Your force Draine|Pull is now Lvl 3.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.fd.forcePowerLevel[FP_RAGE] = 2;
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] = 3;
		LevelMessage(ent, "Your Push|TeamHeal is now Lvl 3.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_SEE] = 3;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 3;
		LevelMessage(ent, "Your force Seeing|Saberthrow is now Lvl 3.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 3;
		client->ps.fd.forcePowerLevel[FP_TEAM_FORCE] = 3;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 3;
		LevelMessage(ent, "Your SaberDefense|Jump|TeamForce is now Lvl 3, Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}
void UpdateSithinquisitor3(gentity_t *ent, qboolean silent)
{
	gclient_t *client = ent->client;

	// Nå kommer spilleren alltid til å få alt ovenfor.
	if (ent->account.level >= EXP_LEVEL_1)
	{
		// Og her kan vi forandre på det som skal til for level 0/1
		// ingen forandringer fra base-setting for level 0
	}
	if (ent->account.level >= EXP_LEVEL_2)
	{
		client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] = 2;
		client->ps.fd.forcePowerLevel[FP_GRIP] = 2;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 2;
		LevelMessage(ent, "Your TeamHeal|Grip|Jump is now Lvl 2.", silent, EXP_LEVEL_2);
	}
	if (ent->account.level >= EXP_LEVEL_3)
	{
		client->ps.fd.forcePowerLevel[FP_PUSH] = 2;
		client->ps.fd.forcePowerLevel[FP_PULL] = 2;
		client->ps.fd.forcePowerLevel[FP_SEE] = 2;
		LevelMessage(ent, "Your Push|Pull|Seeing is now Lvl 2.", silent, EXP_LEVEL_3);
	}
	if (ent->account.level >= EXP_LEVEL_4)
	{
		client->ps.fd.forcePowerLevel[FP_DRAIN] = 2;
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 2;
		LevelMessage(ent, "Your SaberDefense|Draine is now Lvl 2.", silent, EXP_LEVEL_4);
	}
	if (ent->account.level >= EXP_LEVEL_5)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] = 3;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 2;
		LevelMessage(ent, "Your SaberOffense is now Lvl 3|Saberthrow Lvl 2.", silent, EXP_LEVEL_5);
	}
	if (ent->account.level >= EXP_LEVEL_6)
	{
		LevelMessage(ent, "No Upgrade in this Lvl.", silent, EXP_LEVEL_6);
	}
	if (ent->account.level >= EXP_LEVEL_7)
	{
		client->ps.fd.forcePowerLevel[FP_DRAIN] = 3;
		client->ps.fd.forcePowerLevel[FP_PULL] = 3;
		LevelMessage(ent, "Your force Draine|Pull is now Lvl 3.", silent, EXP_LEVEL_7);
	}
	if (ent->account.level >= EXP_LEVEL_8)
	{
		client->ps.fd.forcePowerLevel[FP_RAGE] = 2;
		client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
		client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] = 3;
		LevelMessage(ent, "Your Push|TeamHeal is now Lvl 3.", silent, EXP_LEVEL_8);
	}
	if (ent->account.level >= EXP_LEVEL_9)
	{
		client->ps.fd.forcePowerLevel[FP_SEE] = 3;
		client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 3;
		LevelMessage(ent, "Your force Seeing|Saberthrow is now Lvl 3.", silent, EXP_LEVEL_9);
	}
	if (ent->account.level >= EXP_LEVEL_10)
	{
		client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] = 3;
		client->ps.fd.forcePowerLevel[FP_GRIP] = 3;
		client->ps.fd.forcePowerLevel[FP_LEVITATION] = 3;
		LevelMessage(ent, "Your SaberDefense|Jump|Grip is now Lvl 3, Upgrade Class succesfull.", silent, EXP_LEVEL_10);
	}
}

void UpdateCharacter(gentity_t *ent, qboolean silent)
{
	switch (ent->account.playerclass)

	{
	case PCLASS_TROOPER_1://Tank class to take alot of damage from npc's
		UpdateTrooper1(ent, silent);
		break;
	case PCLASS_TROOPER_2://Tank class to take alot of damage from npc's
		UpdateTrooper2(ent, silent);
		break;
	case PCLASS_TROOPER_3://Tank class to take alot of damage from npc's
		UpdateTrooper3(ent, silent);
		break;
	case PCLASS_JEDIKNIGHT_1://Close DPS to do damage over time to help the team to take down and project the tank
		UpdateJediKnight1(ent, silent);
		break;
	case PCLASS_JEDIKNIGHT_2://Close DPS to do damage over time to help the team to take down and project the tank
		UpdateJediKnight2(ent, silent);
		break;
	case PCLASS_JEDIKNIGHT_3://Close DPS to do damage over time to help the team to take down and project the tank
		UpdateJediKnight3(ent, silent);
		break;
	case PCLASS_SMUGGLER_1://Range DPS to do damage from long distions wiht lightning saber throw and more
		UpdateSmuggler1(ent, silent);
		break;
	case PCLASS_SMUGGLER_2://Range DPS to do damage from long distions wiht lightning saber throw and more
		UpdateSmuggler2(ent, silent);
		break;
	case PCLASS_SMUGGLER_3://Range DPS to do damage from long distions wiht lightning saber throw and more
		UpdateSmuggler3(ent, silent);
		break;
	case PCLASS_JEDI_CONSULAR_1://Healer the importen one to heal Tank and dps from damage/Healing spec. Team healing/Dogde Healing and normal healing
		UpdateJediConsular1(ent, silent);
		break;
	case PCLASS_JEDI_CONSULAR_2://Healer the importen one to heal Tank and dps from damage/Healing spec. Team healing/Dogde Healing and normal healing
		UpdateJediConsular2(ent, silent);
		break;
	case PCLASS_JEDI_CONSULAR_3://Healer the importen one to heal Tank and dps from damage/Healing spec. Team healing/Dogde Healing and normal healing
		UpdateJediConsular3(ent, silent);
		break;
	case PCLASS_BOUNTYHUNTER_1://Tank class to take alot of damage from npc's
		UpdateBountyhunter1(ent, silent);
		break;
	case PCLASS_BOUNTYHUNTER_2:
		UpdateBountyhunter2(ent, silent);
		break;
	case PCLASS_BOUNTYHUNTER_3:
		UpdateBountyhunter3(ent, silent);
		break;
	case PCLASS_SITHWORRIOR_1://Close DPS to do damage over time to help the team to take down and project the tank
		UpdateSithworrior1(ent, silent);
		break;
	case PCLASS_SITHWORRIOR_2://Close DPS to do damage over time to help the team to take down and project the tank
		UpdateSithworrior2(ent, silent);
		break;
	case PCLASS_SITHWORRIOR_3:
		UpdateSithworrior3(ent, silent);
		break;
	case PCLASS_IPPERIAL_AGENT_1://Range DPS to do damage from long distions wiht lightning saber throw and more
		UpdateImperialAgent1(ent, silent);
		break;
	case PCLASS_IPPERIAL_AGENT_2:
		UpdateImperialAgent2(ent, silent);
		break;
	case PCLASS_IPPERIAL_AGENT_3:
		UpdateImperialAgent3(ent, silent);
		break;
	case PCLASS_SITH_INQUISITOR_1://Healer the importen one to heal Tank and dps from damage/Healing spec. Team healing/Dogde Healing and normal healing
		UpdateSithinquisitor1(ent, silent);
		break;
	case PCLASS_SITH_INQUISITOR_2:
		UpdateSithinquisitor2(ent, silent);
		break;
	case PCLASS_SITH_INQUISITOR_3:
		UpdateSithinquisitor3(ent, silent);
		break;
	default:
		break;

	}

}*/
//[/CLASSsys]
//[EXPsys]
void GiveExperiance(gentity_t *ent, int amount) {
	if (!ent->client) return;

	if (!g_experianceEnabled.integer) return;

	if (amount <= 0) return;

	if (ent->account.level == NUM_EXP_LEVELS)	// We're already max level. No need to gain more experience 
		return;

	ent->client->ps.stats[STAT_EXP] += amount;

	if (ent->client->ps.stats[STAT_EXP] >= experienceLevel[ent->account.level])
	{
		ent->account.level++;	// Increase level by 1
		ent->client->ps.stats[STAT_EXP_COUNT] = experienceLevel[ent->account.level];	// Set the required experience for next level.
		trap->SendServerCommand(ent - g_entities, va("maxexperience %i", ent->client->ps.stats[STAT_EXP_COUNT]));
		ent->client->ps.stats[STAT_EXP] = 0;	// Reset experience to 0 to start on the next level.
#ifndef __MMO__
		// UQ1: TODO - Make this an EVENT.
		trap->SendServerCommand(-1, va("print \"%s^7 has leveled up, and is now level %i!\n\"", ent->client->pers.netname, ent->account.level));
		trap->SendServerCommand(-1, va("chat \"%s^7 has leveled up, and is now level %i!\n\"", ent->client->pers.netname, ent->account.level));
#endif //__MMO__
		//UpdateCharacter(ent, qfalse);	// Update character, print the messages. so it's NOT silent (qfalse on silent)
	}
	//	ent->client->ps.persistant[PERS_EXPERIANCE_COUNT] += amount;

	if (ent->r.svFlags & SVF_OLD_CLIENT) {
#ifndef __MMO__
		// UQ1: TODO - Make this an EVENT.
		trap->SendServerCommand(ent - g_entities, va("print \"+%i experiance\n\"", amount));
#endif //__MMO__
	}
	else {
		trap->SendServerCommand(ent - g_entities, va("experiance %i", ent->client->ps.stats[STAT_EXP]));
	}
}

void TakeExperiance(gentity_t *ent, int amount) {
	if (!ent->client) return;

	if (!g_experianceEnabled.integer) return;

	if (amount <= 0) return;

	if (amount > ent->client->ps.stats[STAT_EXP])
		ent->client->ps.stats[STAT_EXP] = 0;
	else
		ent->client->ps.stats[STAT_EXP] -= amount;

	if (ent->r.svFlags & SVF_OLD_CLIENT) {
		trap->SendServerCommand(ent - g_entities, va("print \"-%i experiance\n\"", amount));
	}
	else {
		trap->SendServerCommand(ent - g_entities, va("experiance %i", ent->client->ps.stats[STAT_EXP]));
	}
}

void TradeExperiance(gentity_t *from, gentity_t *to, int amount) {
	if (!from->client) return;
	if (!to->client) return;

	if (!g_experianceEnabled.integer) return;

	if (amount <= 0) return;

	// we can't take more experiance than they have - no cheating!
	if (amount > from->client->ps.stats[STAT_EXP])
		amount = from->client->ps.stats[STAT_EXP];

	from->client->ps.stats[STAT_EXP] -= amount;
	to->client->ps.stats[STAT_EXP] += amount;

	TakeExperiance(from, amount);
	GiveExperiance(to, amount);
}
//[/EXPsys]

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted, scoreFlags, accuracy, perfect;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedClients;

	if (numSorted > MAX_CLIENT_SCORE_SEND)
	{
		numSorted = MAX_CLIENT_SCORE_SEND;
	}

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected == CON_CONNECTING ) {
			ping = -1;
		} else {
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}

		if( cl->accuracy_shots ) {
			accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		}
		else {
			accuracy = 0;
		}
		perfect = ( cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;

		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
			cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy,
			//[EXPsys]
			cl->ps.stats[STAT_EXP],
			cl->ps.stats[STAT_EXP_COUNT],
			//[/EXPsys]
			cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT],
			cl->ps.persistant[PERS_DEFEND_COUNT],
			cl->ps.persistant[PERS_ASSIST_COUNT],
			perfect,
			cl->ps.persistant[PERS_CAPTURES]);
		j = strlen(entry);
		if (stringlength + j > 1022)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	//still want to know the total # of clients
	i = level.numConnectedClients;

	trap->SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i,
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}

/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap->Argc();
	for ( i = start ; i < c ; i++ ) {
		trap->Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
StringIsInteger
==================
*/
qboolean StringIsInteger( const char *s ) {
	int			i=0, len=0;
	qboolean	foundDigit=qfalse;

	for ( i=0, len=strlen( s ); i<len; i++ )
	{
		if ( !isdigit( s[i] ) )
			return qfalse;

		foundDigit = qtrue;
	}

	return foundDigit;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, const char *s, qboolean allowconnecting ) {
	gclient_t	*cl;
	int			idnum;
	char		cleanInput[MAX_NETNAME];

	if ( StringIsInteger( s ) )
	{// numeric values could be slot numbers
		idnum = atoi( s );
		if ( idnum >= 0 && idnum < level.maxclients )
		{
			cl = &level.clients[idnum];
			if ( cl->pers.connected == CON_CONNECTED )
				return idnum;
			else if ( allowconnecting && cl->pers.connected == CON_CONNECTING )
				return idnum;
		}
	}

	Q_strncpyz( cleanInput, s, sizeof(cleanInput) );
	Q_StripColor( cleanInput );

	for ( idnum=0,cl=level.clients; idnum < level.maxclients; idnum++,cl++ )
	{// check for a name match
		if ( cl->pers.connected != CON_CONNECTED )
			if ( !allowconnecting || cl->pers.connected < CON_CONNECTING )
				continue;

		if ( !Q_stricmp( cl->pers.netname_nocolor, cleanInput ) )
			return idnum;
	}

	trap->SendServerCommand( to-g_entities, va( "print \"User %s is not on the server\n\"", s ) );
	return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
//[VisualWeapons]
extern qboolean G_ClientPlugin(void);
//[/VisualWeapons]
void G_Give( gentity_t *ent, const char *name, const char *args, int argc )
{
	gitem_t		*it;
	int			i;
	qboolean	give_all = qfalse;
	gentity_t	*it_ent;
	trace_t		trace;

	if ( !Q_stricmp( name, "all" ) )
		give_all = qtrue;

	if ( give_all )
	{
		for ( i=0; i<HI_NUM_HOLDABLE; i++ )
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
	}

	if ( give_all || !Q_stricmp( name, "health") )
	{
		if ( argc == 3 )
			ent->health = Com_Clampi( 1, ent->client->ps.stats[STAT_MAX_HEALTH], atoi( args ) );
		else
		{
			if ( level.gametype == GT_SIEGE && ent->client->siegeClass != -1 )
				ent->health = bgSiegeClasses[ent->client->siegeClass].maxhealth;
			else
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if ( !give_all )
			return;
	}

	if ( give_all || !Q_stricmp( name, "armor" ) || !Q_stricmp( name, "shield" ) )
	{
		if ( argc == 3 )
			ent->client->ps.stats[STAT_ARMOR] = Com_Clampi( 0, ent->client->ps.stats[STAT_MAX_HEALTH], atoi( args ) );
		else
		{
			if ( level.gametype == GT_SIEGE && ent->client->siegeClass != -1 )
				ent->client->ps.stats[STAT_ARMOR] = bgSiegeClasses[ent->client->siegeClass].maxarmor;
			else
				ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_HEALTH];
		}

		if ( !give_all )
			return;
	}

	if ( give_all || !Q_stricmp( name, "force" ) )
	{
		if ( argc == 3 )
			ent->client->ps.fd.forcePower = Com_Clampi( 0, ent->client->ps.fd.forcePowerMax, atoi( args ) );
		else
			ent->client->ps.fd.forcePower = ent->client->ps.fd.forcePowerMax;

		if ( !give_all )
			return;
	}

	if ( give_all || !Q_stricmp( name, "weapons" ) )
	{
		ent->client->ps.temporaryWeapon = WP_ALL_WEAPONS;

		if ( !give_all )
			return;
	}

	if ( !give_all && !Q_stricmp( name, "weaponnum" ) )
	{
		ent->client->ps.temporaryWeapon = atoi( args );
		return;
	}

	if ( give_all || !Q_stricmp( name, "ammo" ) )
	{
		int num = 999;
		if ( argc == 3 )
			num = Com_Clampi( 0, 999, atoi( args ) );
#ifndef __MMO__
		for ( i=AMMO_BLASTER; i<AMMO_MAX; i++ )
			ent->client->ps.ammo[i] = num;
#endif //__MMO__
		if ( !give_all )
			return;
	}

	if ( !Q_stricmp( name, "excellent" ) ) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if ( !Q_stricmp( name, "impressive" ) ) {
		ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
	if ( !Q_stricmp( name, "gauntletaward" ) ) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if ( !Q_stricmp( name, "defend" ) ) {
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if ( !Q_stricmp( name, "assist" ) ) {
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}

	//[EXPsys]
	/*if (Q_stricmp(name, "Experiance") == 0) {
		int num = 10000;
		if (trap_Argc() == 3 + baseArg) {
			trap_Argv(2 + baseArg, arg, sizeof(arg));
			num = atoi(arg);
		}

		ent->client->ps.persistant[PERS_EXPERIANCE] += num;
		return;
	}*/
	//[/EXPsys]
	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem( name );
		if ( !it )
			return;

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem( it_ent, it );
		if ( !it_ent || !it_ent->inuse )
			return;
		FinishSpawningItem( it_ent );
		if ( !it_ent || !it_ent->inuse )
			return;
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item( it_ent, ent, &trace );
		if ( it_ent->inuse )
			G_FreeEntity( it_ent );
	}
}

void Cmd_Give_f( gentity_t *ent )
{
	char name[MAX_TOKEN_CHARS] = {0};

	trap->Argv( 1, name, sizeof( name ) );
	G_Give( ent, name, ConcatArgs( 2 ), trap->Argc() );
}

void Cmd_GiveOther_f( gentity_t *ent )
{
	char		name[MAX_TOKEN_CHARS] = {0};
	int			i;
	char		otherindex[MAX_TOKEN_CHARS];
	gentity_t	*otherEnt = NULL;

	if ( trap->Argc () < 3 ) {
		trap->SendServerCommand( ent-g_entities, "print \"Usage: giveother <player id> <givestring>\n\"" );
		return;
	}

	trap->Argv( 1, otherindex, sizeof( otherindex ) );
	i = ClientNumberFromString( ent, otherindex, qfalse );
	if ( i == -1 ) {
		return;
	}

	otherEnt = &g_entities[i];
	if ( !otherEnt->inuse || !otherEnt->client ) {
		return;
	}

	if ( (otherEnt->health <= 0 || otherEnt->client->tempSpectate >= level.time || otherEnt->client->sess.sessionTeam == TEAM_SPECTATOR) )
	{
		// Intentionally displaying for the command user
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "MUSTBEALIVE" ) ) );
		return;
	}

	trap->Argv( 2, name, sizeof( name ) );

	G_Give( otherEnt, name, ConcatArgs( 3 ), trap->Argc()-1 );
}

/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f( gentity_t *ent ) {
	char *msg = NULL;

	ent->flags ^= FL_GODMODE;
	if ( !(ent->flags & FL_GODMODE) )
		msg = "godmode OFF";
	else
		msg = "godmode ON";

	trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", msg ) );
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char *msg = NULL;

	ent->flags ^= FL_NOTARGET;
	if ( !(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF";
	else
		msg = "notarget ON";

	trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", msg ) );
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char *msg = NULL;

	ent->client->noclip = !ent->client->noclip;
	if ( !ent->client->noclip )
		msg = "noclip OFF";
	else
		msg = "noclip ON";

	trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", msg ) );
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent )
{
	if ( !ent->client->pers.localClient )
	{
		trap->SendServerCommand(ent-g_entities, "print \"The levelshot command must be executed by a local client\n\"");
		return;
	}

	// doesn't work in single player
	if ( level.gametype == GT_SINGLE_PLAYER )
	{
		trap->SendServerCommand(ent-g_entities, "print \"Must not be in singleplayer mode for levelshot\n\"" );
		return;
	}

	BeginIntermission();
	trap->SendServerCommand( ent-g_entities, "clientLevelShot" );
}

#if 0
/*
==================
Cmd_TeamTask_f

From TA.
==================
*/
void Cmd_TeamTask_f( gentity_t *ent ) {
	char userinfo[MAX_INFO_STRING];
	char		arg[MAX_TOKEN_CHARS];
	int task;
	int client = ent->client - level.clients;

	if ( trap->Argc() != 2 ) {
		return;
	}
	trap->Argv( 1, arg, sizeof( arg ) );
	task = atoi( arg );

	trap->GetUserinfo(client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	trap->SetUserinfo(client, userinfo);
	ClientUserinfoChanged(client);
}
#endif

void G_Kill( gentity_t *ent ) {
	if ((level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL) &&
		level.numPlayingClients > 1 && !level.warmupTime)
	{
		if (!g_allowDuelSuicide.integer)
		{
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "ATTEMPTDUELKILL")) );
			return;
		}
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	G_Kill( ent );
}

void Cmd_KillOther_f( gentity_t *ent )
{
	int			i;
	char		otherindex[MAX_TOKEN_CHARS];
	gentity_t	*otherEnt = NULL;

	if ( trap->Argc () < 2 ) {
		trap->SendServerCommand( ent-g_entities, "print \"Usage: killother <player id>\n\"" );
		return;
	}

	trap->Argv( 1, otherindex, sizeof( otherindex ) );
	i = ClientNumberFromString( ent, otherindex, qfalse );
	if ( i == -1 ) {
		return;
	}

	otherEnt = &g_entities[i];
	if ( !otherEnt->inuse || !otherEnt->client ) {
		return;
	}

	if ( (otherEnt->health <= 0 || otherEnt->client->tempSpectate >= level.time || otherEnt->client->sess.sessionTeam == TEAM_SPECTATOR) )
	{
		// Intentionally displaying for the command user
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "MUSTBEALIVE" ) ) );
		return;
	}

	G_Kill( otherEnt );
}

gentity_t *G_GetDuelWinner(gclient_t *client)
{
	gclient_t *wCl;
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		wCl = &level.clients[i];

		if (wCl && wCl != client && /*wCl->ps.clientNum != client->ps.clientNum &&*/
			wCl->pers.connected == CON_CONNECTED && wCl->sess.sessionTeam != TEAM_SPECTATOR)
		{
			return &g_entities[wCl->ps.clientNum];
		}
	}

	return NULL;
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
	client->ps.fd.forceDoInit = 1; //every time we change teams make sure our force powers are set right

	if (level.gametype == GT_SIEGE)
	{ //don't announce these things in siege
		return;
	}

#ifndef __MMO__
	if ( client->sess.sessionTeam == TEAM_RED ) {
		trap->SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEREDTEAM")) );
	} else if ( client->sess.sessionTeam == TEAM_BLUE ) {
		trap->SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBLUETEAM")));
	} else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
		trap->SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHESPECTATORS")));
	} else if ( client->sess.sessionTeam == TEAM_FREE ) {
		if (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
		{
			/*
			gentity_t *currentWinner = G_GetDuelWinner(client);

			if (currentWinner && currentWinner->client)
			{
				trap->SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s %s\n\"",
				currentWinner->client->pers.netname, G_GetStringEdString("MP_SVGAME", "VERSUS"), client->pers.netname));
			}
			else
			{
				trap->SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
				client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
			}
			*/
			//NOTE: Just doing a vs. once it counts two players up
		}
		else
		{
			trap->SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
		}
	}
#endif //__MMO__

	G_LogPrintf( "ChangeTeam: %i [%s] (%s) \"%s^7\" %s -> %s\n", (int)(client - level.clients), client->sess.IP, client->pers.guid, client->pers.netname, TeamName( oldTeam ), TeamName( client->sess.sessionTeam ) );
}

qboolean G_PowerDuelCheckFail(gentity_t *ent)
{
	int			loners = 0;
	int			doubles = 0;

	if (!ent->client || ent->client->sess.duelTeam == DUELTEAM_FREE)
	{
		return qtrue;
	}

	G_PowerDuelCount(&loners, &doubles, qfalse);

	if (ent->client->sess.duelTeam == DUELTEAM_LONE && loners >= 1)
	{
		return qtrue;
	}

	if (ent->client->sess.duelTeam == DUELTEAM_DOUBLE && doubles >= 2)
	{
		return qtrue;
	}

	return qfalse;
}

/*
=================
SetTeam
=================
*/
qboolean g_dontPenalizeTeam = qfalse;
qboolean g_preventTeamBegin = qfalse;
void SetTeam( gentity_t *ent, char *s ) {
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	int					teamLeader;

	// fix: this prevents rare creation of invalid players
	if (!ent->inuse)
	{
		return;
	}

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE; // SPECTATOR_SCOREBOARD disabling this for now since it is totally broken on client side
	} else if ( !Q_stricmp( s, "follow1" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	} else if ( !Q_stricmp( s, "follow2" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	} else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else if ( level.gametype >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			//For now, don't do this. The legalize function will set powers properly now.
			/*
			if (g_forceBasedTeams.integer)
			{
				if (ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					team = TEAM_BLUE;
				}
				else
				{
					team = TEAM_RED;
				}
			}
			else
			{
			*/
				team = PickTeam( clientNum );
			//}
		}

		if ( g_teamForceBalance.integer && !g_jediVmerc.integer ) {
			int		counts[TEAM_NUM_TEAMS];

			//JAC: Invalid clientNum was being used
			counts[TEAM_BLUE] = TeamCount( ent-g_entities, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( ent-g_entities, TEAM_RED );

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_DARKSIDE)
				{
					trap->SendServerCommand( ent->client->ps.clientNum,
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED_SWITCH")) );
				}
				else
				*/
				{
					//JAC: Invalid clientNum was being used
					trap->SendServerCommand( ent-g_entities,
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED")) );
				}
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					trap->SendServerCommand( ent->client->ps.clientNum,
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE_SWITCH")) );
				}
				else
				*/
				{
					//JAC: Invalid clientNum was being used
					trap->SendServerCommand( ent-g_entities,
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE")) );
				}
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

		//For now, don't do this. The legalize function will set powers properly now.
		/*
		if (g_forceBasedTeams.integer)
		{
			if (team == TEAM_BLUE && ent->client->ps.fd.forceSide != FORCE_LIGHTSIDE)
			{
				trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBELIGHT")) );
				return;
			}
			if (team == TEAM_RED && ent->client->ps.fd.forceSide != FORCE_DARKSIDE)
			{
				trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEDARK")) );
				return;
			}
		}
		*/

	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	oldTeam = client->sess.sessionTeam;

	if (level.gametype == GT_SIEGE)
	{
		if (client->tempSpectate >= level.time &&
			team == TEAM_SPECTATOR)
		{ //sorry, can't do that.
			return;
		}

		if ( team == oldTeam && team != TEAM_SPECTATOR )
			return;

		client->sess.siegeDesiredTeam = team;
		//oh well, just let them go.
		/*
		if (team != TEAM_SPECTATOR)
		{ //can't switch to anything in siege unless you want to switch to being a fulltime spectator
			//fill them in on their objectives for this team now
			trap->SendServerCommand(ent-g_entities, va("sb %i", client->sess.siegeDesiredTeam));

			trap->SendServerCommand( ent-g_entities, va("print \"You will be on the selected team the next time the round begins.\n\"") );
			return;
		}
		*/
		if (client->sess.sessionTeam != TEAM_SPECTATOR &&
			team != TEAM_SPECTATOR)
		{ //not a spectator now, and not switching to spec, so you have to wait til you die.
			//trap->SendServerCommand( ent-g_entities, va("print \"You will be on the selected team the next time you respawn.\n\"") );
			qboolean doBegin;
			if (ent->client->tempSpectate >= level.time)
			{
				doBegin = qfalse;
			}
			else
			{
				doBegin = qtrue;
			}

			if (doBegin)
			{
				// Kill them so they automatically respawn in the team they wanted.
				if (ent->health > 0)
				{
					ent->flags &= ~FL_GODMODE;
					ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
					player_die( ent, ent, ent, 100000, MOD_TEAM_CHANGE );
				}
			}

			if (ent->client->sess.sessionTeam != ent->client->sess.siegeDesiredTeam)
			{
				SetTeamQuick(ent, ent->client->sess.siegeDesiredTeam, qfalse);
			}

			return;
		}
	}

	// override decision if limiting the players
	if ( (level.gametype == GT_DUEL)
		&& level.numNonSpectatorClients >= 2 )
	{
		team = TEAM_SPECTATOR;
	}
	else if ( (level.gametype == GT_POWERDUEL)
		&& (level.numPlayingClients >= 3 || G_PowerDuelCheckFail(ent)) )
	{
		team = TEAM_SPECTATOR;
	}
	else if ( g_maxGameClients.integer > 0 &&
		level.numNonSpectatorClients >= g_maxGameClients.integer )
	{
		team = TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	//
	// execute the team change
	//

	//If it's siege then show the mission briefing for the team you just joined.
//	if (level.gametype == GT_SIEGE && team != TEAM_SPECTATOR)
//	{
//		trap->SendServerCommand(clientNum, va("sb %i", team));
//	}

	// if the player was dead leave the body
	if ( client->ps.stats[STAT_HEALTH] <= 0 && client->sess.sessionTeam != TEAM_SPECTATOR ) {
		MaintainBodyQueue(ent);
	}

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if ( oldTeam != TEAM_SPECTATOR ) {
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		g_dontPenalizeTeam = qtrue;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE);
		g_dontPenalizeTeam = qfalse;

	}
	// they go to the end of the line for tournaments
	if ( team == TEAM_SPECTATOR && oldTeam != team )
		AddTournamentQueue( client );

	// clear votes if going to spectator (specs can't vote)
	if ( team == TEAM_SPECTATOR )
		G_ClearVote( ent );
	// also clear team votes if switching red/blue or going to spec
	G_ClearTeamVote( ent, oldTeam );

	client->sess.sessionTeam = (team_t)team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	client->sess.teamLeader = qfalse;
	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		teamLeader = TeamLeader( team );
		// if there is no team leader or the team leader is a bot and this client is not a bot
		if ( teamLeader == -1 || ( !(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
			//SetLeader( team, clientNum );
		}
	}
	// make sure there is a team leader on the team the player came from
	if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
		CheckTeamLeader( oldTeam );
	}

	BroadcastTeamChange( client, oldTeam );

	//make a disappearing effect where they were before teleporting them to the appropriate spawn point,
	//if we were not on the spec team
	if (oldTeam != TEAM_SPECTATOR)
	{
		gentity_t *tent = G_TempEntity( client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = clientNum;
	}

	// get and distribute relevent paramters
	if ( !ClientUserinfoChanged( clientNum ) )
		return;

	if (!g_preventTeamBegin)
	{
		ClientBegin( clientNum, qfalse );
	}
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
extern void G_LeaveVehicle( gentity_t *ent, qboolean ConCheck );
void StopFollowing( gentity_t *ent ) {
	int i=0;
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.weapon = WP_NONE;
	G_LeaveVehicle( ent, qfalse ); // clears m_iVehicleNum as well
	ent->client->ps.emplacedIndex = 0;
	//ent->client->ps.m_iVehicleNum = 0;
	ent->client->ps.viewangles[ROLL] = 0.0f;
	ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
	ent->client->ps.forceHandExtendTime = 0;
	ent->client->ps.zoomMode = 0;
	ent->client->ps.zoomLocked = 0;
	ent->client->ps.zoomLockTime = 0;
	ent->client->ps.saberMove = LS_NONE;
	ent->client->ps.legsAnim = 0;
	ent->client->ps.legsTimer = 0;
	ent->client->ps.torsoAnim = 0;
	ent->client->ps.torsoTimer = 0;
	ent->client->ps.isJediMaster = qfalse; // major exploit if you are spectating somebody and they are JM and you reconnect
	ent->client->ps.cloakFuel = 100; // so that fuel goes away after stop following them
	ent->client->ps.jetpackFuel = 100; // so that fuel goes away after stop following them
	ent->health = ent->client->ps.stats[STAT_HEALTH] = 100; // so that you don't keep dead angles if you were spectating a dead person
	ent->client->ps.bobCycle = 0;
	ent->client->ps.pm_type = PM_SPECTATOR;
	ent->client->ps.eFlags &= ~EF_DISINTEGRATION;
	for ( i=0; i<PW_NUM_POWERUPS; i++ )
		ent->client->ps.powerups[i] = 0;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	oldTeam = ent->client->sess.sessionTeam;

	if ( trap->Argc() != 2 ) {
		switch ( oldTeam ) {
		case TEAM_BLUE:
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTBLUETEAM")) );
			break;
		case TEAM_RED:
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTREDTEAM")) );
			break;
		case TEAM_FREE:
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTFREETEAM")) );
			break;
		case TEAM_SPECTATOR:
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTSPECTEAM")) );
			break;
		}
		return;
	}

	if ( ent->client->switchTeamTime > level.time ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	if (gEscaping)
	{
		return;
	}

	// if they are playing a tournament game, count as a loss
	if ( level.gametype == GT_DUEL
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {//in a tournament game
		//disallow changing teams
		trap->SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Duel\n\"" );
		return;
		//FIXME: why should this be a loss???
		//ent->client->sess.losses++;
	}

	if (level.gametype == GT_POWERDUEL)
	{ //don't let clients change teams manually at all in powerduel, it will be taken care of through automated stuff
		trap->SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Power Duel\n\"" );
		return;
	}

	trap->Argv( 1, s, sizeof( s ) );

	SetTeam( ent, s );

	// fix: update team switch time only if team change really happend
	if (oldTeam != ent->client->sess.sessionTeam)
		ent->client->switchTeamTime = level.time + 5000;
}

/*
=================
Cmd_DuelTeam_f
=================
*/
void Cmd_DuelTeam_f(gentity_t *ent)
{
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if (level.gametype != GT_POWERDUEL)
	{ //don't bother doing anything if this is not power duel
		return;
	}

	/*
	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		trap->SendServerCommand( ent-g_entities, va("print \"You cannot change your duel team unless you are a spectator.\n\""));
		return;
	}
	*/

	if ( trap->Argc() != 2 )
	{ //No arg so tell what team we're currently on.
		oldTeam = ent->client->sess.duelTeam;
		switch ( oldTeam )
		{
		case DUELTEAM_FREE:
			trap->SendServerCommand( ent-g_entities, va("print \"None\n\"") );
			break;
		case DUELTEAM_LONE:
			trap->SendServerCommand( ent-g_entities, va("print \"Single\n\"") );
			break;
		case DUELTEAM_DOUBLE:
			trap->SendServerCommand( ent-g_entities, va("print \"Double\n\"") );
			break;
		default:
			break;
		}
		return;
	}

	if ( ent->client->switchDuelTeamTime > level.time )
	{ //debounce for changing
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	trap->Argv( 1, s, sizeof( s ) );

	oldTeam = ent->client->sess.duelTeam;

	if (!Q_stricmp(s, "free"))
	{
		ent->client->sess.duelTeam = DUELTEAM_FREE;
	}
	else if (!Q_stricmp(s, "single"))
	{
		ent->client->sess.duelTeam = DUELTEAM_LONE;
	}
	else if (!Q_stricmp(s, "double"))
	{
		ent->client->sess.duelTeam = DUELTEAM_DOUBLE;
	}
	else
	{
		trap->SendServerCommand( ent-g_entities, va("print \"'%s' not a valid duel team.\n\"", s) );
	}

	if (oldTeam == ent->client->sess.duelTeam)
	{ //didn't actually change, so don't care.
		return;
	}

	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{ //ok..die
		int curTeam = ent->client->sess.duelTeam;
		ent->client->sess.duelTeam = oldTeam;
		G_Damage(ent, ent, ent, NULL, ent->client->ps.origin, 99999, DAMAGE_NO_PROTECTION, MOD_SUICIDE);
		ent->client->sess.duelTeam = curTeam;
	}
	//reset wins and losses
	ent->client->sess.wins = 0;
	ent->client->sess.losses = 0;

	//get and distribute relevent paramters
	if ( ClientUserinfoChanged( ent->s.number ) )
		return;

	ent->client->switchDuelTeamTime = level.time + 5000;
}

int G_TeamForSiegeClass(const char *clName)
{
	int i = 0;
	int team = SIEGETEAM_TEAM1;
	siegeTeam_t *stm = BG_SiegeFindThemeForTeam(team);
	siegeClass_t *scl;

	if (!stm)
	{
		return 0;
	}

	while (team <= SIEGETEAM_TEAM2)
	{
		scl = stm->classes[i];

		if (scl && scl->name[0])
		{
			if (!Q_stricmp(clName, scl->name))
			{
				return team;
			}
		}

		i++;
		if (i >= MAX_SIEGE_CLASSES || i >= stm->numClasses)
		{
			if (team == SIEGETEAM_TEAM2)
			{
				break;
			}
			team = SIEGETEAM_TEAM2;
			stm = BG_SiegeFindThemeForTeam(team);
			i = 0;
		}
	}

	return 0;
}

/*
=================
Cmd_SiegeClass_f
=================
*/
void Cmd_SiegeClass_f( gentity_t *ent )
{
	char className[64];
	int team = 0;
	int preScore;
	qboolean startedAsSpec = qfalse;

	//if (level.gametype != GT_SIEGE)
	//{ //classes are only valid for this gametype
	//	return;
	//}

	if (!ent->client)
	{
		return;
	}

	if (trap->Argc() < 1)
	{
		return;
	}

	if ( ent->client->switchClassTime > level.time )
	{
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCLASSSWITCH")) );
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		startedAsSpec = qtrue;
	}

	trap->Argv( 1, className, sizeof( className ) );

	team = G_TeamForSiegeClass(className);

	if (!team)
	{ //not a valid class name
		return;
	}

	if (ent->client->sess.sessionTeam != team && ent->client->sess.sessionTeam != TEAM_FREE)
	{ //try changing it then
		g_preventTeamBegin = qtrue;
		if (team == TEAM_RED)
		{
			SetTeam(ent, "red");
		}
		else if (team == TEAM_BLUE)
		{
			SetTeam(ent, "blue");
		}
		g_preventTeamBegin = qfalse;

		if (ent->client->sess.sessionTeam != team)
		{ //failed, oh well
			if (ent->client->sess.sessionTeam != TEAM_SPECTATOR ||
				ent->client->sess.siegeDesiredTeam != team)
			{
				trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCLASSTEAM")) );
				return;
			}
		}
	}

	//preserve 'is score
	preScore = ent->client->ps.persistant[PERS_SCORE];

	//Make sure the class is valid for the team
	BG_SiegeCheckClassLegality(team, className);

	//Set the session data
	strcpy(ent->client->sess.siegeClass, className);

	// get and distribute relevent paramters
	if ( !ClientUserinfoChanged( ent->s.number ) )
		return;

	if (ent->client->tempSpectate < level.time)
	{
		// Kill him (makes sure he loses flags, etc)
		if (ent->health > 0 && !startedAsSpec)
		{
			ent->flags &= ~FL_GODMODE;
			ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
			player_die (ent, ent, ent, 100000, MOD_SUICIDE);
		}

		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR || startedAsSpec)
		{ //respawn them instantly.
			ClientBegin( ent->s.number, qfalse );
		}
	}
	//set it back after we do all the stuff
	ent->client->ps.persistant[PERS_SCORE] = preScore;

	ent->client->switchClassTime = level.time + 5000;
}

/*
=================
Cmd_ForceChanged_f
=================
*/
void Cmd_ForceChanged_f( gentity_t *ent )
{
	char fpChStr[1024];
	const char *buf;
//	Cmd_Kill_f(ent);
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{ //if it's a spec, just make the changes now
		//trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "FORCEAPPLIED")) );
		//No longer print it, as the UI calls this a lot.
		WP_InitForcePowers( ent );
		goto argCheck;
	}

	buf = G_GetStringEdString("MP_SVGAME", "FORCEPOWERCHANGED");

	strcpy(fpChStr, buf);

	trap->SendServerCommand( ent-g_entities, va("print \"%s%s\n\"", S_COLOR_GREEN, fpChStr) );

	ent->client->ps.fd.forceDoInit = 1;
argCheck:
	if (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
	{ //If this is duel, don't even bother changing team in relation to this.
		return;
	}

	if (trap->Argc() > 1)
	{
		char	arg[MAX_TOKEN_CHARS];

		trap->Argv( 1, arg, sizeof( arg ) );

		if ( arg[0] )
		{ //if there's an arg, assume it's a combo team command from the UI.
			Cmd_Team_f(ent);
		}
	}
}

extern qboolean WP_SaberStyleValidForSaber( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int saberAnimLevel );
extern qboolean WP_UseFirstValidSaberStyle( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int *saberAnimLevel );
qboolean G_SetSaber(gentity_t *ent, int saberNum, char *saberName, qboolean siegeOverride)
{
	char truncSaberName[MAX_QPATH] = {0};

	if ( !siegeOverride && level.gametype == GT_SIEGE && ent->client->siegeClass != -1 &&
		(bgSiegeClasses[ent->client->siegeClass].saberStance || bgSiegeClasses[ent->client->siegeClass].saber1[0] || bgSiegeClasses[ent->client->siegeClass].saber2[0]) )
	{ //don't let it be changed if the siege class has forced any saber-related things
		return qfalse;
	}

	Q_strncpyz( truncSaberName, saberName, sizeof( truncSaberName ) );

	if ( saberNum == 0 && (!Q_stricmp( "none", truncSaberName ) || !Q_stricmp( "remove", truncSaberName )) )
	{ //can't remove saber 0 like this
		Q_strncpyz( truncSaberName, DEFAULT_SABER, sizeof( truncSaberName ) );
	}

	//Set the saber with the arg given. If the arg is
	//not a valid sabername defaults will be used.
	WP_SetSaber( ent->s.number, ent->client->saber, saberNum, truncSaberName );

	if ( !ent->client->saber[0].model[0] )
	{
		assert(0); //should never happen!
		Q_strncpyz( ent->client->pers.saber1, DEFAULT_SABER, sizeof( ent->client->pers.saber1 ) );
	}
	else
		Q_strncpyz( ent->client->pers.saber1, ent->client->saber[0].name, sizeof( ent->client->pers.saber1 ) );

	if ( !ent->client->saber[1].model[0] )
		Q_strncpyz( ent->client->pers.saber2, "none", sizeof( ent->client->pers.saber2 ) );
	else
		Q_strncpyz( ent->client->pers.saber2, ent->client->saber[1].name, sizeof( ent->client->pers.saber2 ) );

	if ( !WP_SaberStyleValidForSaber( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, ent->client->ps.fd.saberAnimLevel ) )
	{
		WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &ent->client->ps.fd.saberAnimLevel );
		ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = ent->client->ps.fd.saberAnimLevel;
	}

	return qtrue;
}

/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if ( ent->client->sess.spectatorState == SPECTATOR_NOT && ent->client->switchTeamTime > level.time ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	if ( trap->Argc() != 2 ) {
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( ent );
		}
		return;
	}

	trap->Argv( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg, qfalse );
	if ( i == -1 ) {
		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) {
		return;
	}

	// can't follow another spectator
	if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}

	if ( level.clients[ i ].tempSpectate >= level.time ) {
		return;
	}

	// if they are playing a tournament game, count as a loss
	if ( (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {
		//WTF???
		ent->client->sess.losses++;
	}

	// first set them to spectator
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator" );
		// fix: update team switch time only if team change really happend
		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
			ent->client->switchTeamTime = level.time + 5000;
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int		clientnum;
	int		original;
	qboolean	looped = qfalse;

	if ( ent->client->sess.spectatorState == SPECTATOR_NOT && ent->client->switchTeamTime > level.time ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	// if they are playing a tournament game, count as a loss
	if ( (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {\
		//WTF???
		ent->client->sess.losses++;
	}
	// first set them to spectator
	if ( ent->client->sess.spectatorState == SPECTATOR_NOT ) {
		SetTeam( ent, "spectator" );
		// fix: update team switch time only if team change really happend
		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
			ent->client->switchTeamTime = level.time + 5000;
	}

	if ( dir != 1 && dir != -1 ) {
		trap->Error( ERR_DROP, "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;

	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients )
		{
			//JAC: Avoid /team follow1 crash
			if ( looped )
			{
				clientnum = original;
				break;
			}
			else
			{
				clientnum = 0;
				looped = qtrue;
			}
		}
		if ( clientnum < 0 ) {
			if ( looped )
			{
				clientnum = original;
				break;
			}
			else
			{
				clientnum = level.maxclients - 1;
				looped = qtrue;
			}
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].tempSpectate >= level.time ) {
			return;
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}

void Cmd_FollowNext_f( gentity_t *ent ) {
	Cmd_FollowCycle_f( ent, 1 );
}

void Cmd_FollowPrev_f( gentity_t *ent ) {
	Cmd_FollowCycle_f( ent, -1 );
}

/*
==================
G_Say
==================
*/

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, char *locMsg )
{
	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( other->client->pers.connected != CON_CONNECTED ) {
		return;
	}
	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
		return;
	}
	/*
	// no chatting to players in tournaments
	if ( (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREE ) {
		//Hmm, maybe some option to do so if allowed?  Or at least in developer mode...
		return;
	}
	*/
	//They've requested I take this out.

	if (level.gametype == GT_SIEGE &&
		ent->client && (ent->client->tempSpectate >= level.time || ent->client->sess.sessionTeam == TEAM_SPECTATOR) &&
		other->client->sess.sessionTeam != TEAM_SPECTATOR &&
		other->client->tempSpectate < level.time)
	{ //siege temp spectators should not communicate to ingame players
		return;
	}

	if (locMsg)
	{
		trap->SendServerCommand( other-g_entities, va("%s \"%s\" \"%s\" \"%c\" \"%s\" %i",
			mode == SAY_TEAM ? "ltchat" : "lchat",
			name, locMsg, color, message, ent->s.number));
	}
	else
	{
		trap->SendServerCommand( other-g_entities, va("%s \"%s%c%c%s\" %i",
			mode == SAY_TEAM ? "tchat" : "chat",
			name, Q_COLOR_ESCAPE, color, message, ent->s.number));
	}
}

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int			j;
	gentity_t	*other;
	int			color;
	char		name[64];
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];
	char		*locMsg = NULL;

	if ( level.gametype < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	Q_strncpyz( text, chatText, sizeof(text) );

	Q_strstrip( text, "\n\r", "  " );

	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, text );
		Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		break;
	case SAY_TEAM:
		G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, text );
		if (Team_GetLocationMsg(ent, location, sizeof(location)))
		{
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ",
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
			locMsg = location;
		}
		else
		{
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ",
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		}
		color = COLOR_CYAN;
		break;
	case SAY_TELL:
		if (target && target->inuse && target->client && level.gametype >= GT_TEAM &&
			target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			Team_GetLocationMsg(ent, location, sizeof(location)))
		{
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
			locMsg = location;
		}
		else
		{
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		}
		color = COLOR_MAGENTA;
		break;
	}

	if ( target ) {
		G_SayTo( ent, target, mode, color, name, text, locMsg );
		return;
	}

	// echo the text to the console
	if ( dedicated.integer ) {
		trap->Print( "%s%s\n", name, text);
	}

	// send it to all the appropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_SayTo( ent, other, mode, color, name, text, locMsg );
	}
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent ) {
	char *p = NULL;

	if ( trap->Argc () < 2 )
		return;

	p = ConcatArgs( 1 );

	if ( strlen( p ) >= MAX_SAY_TEXT ) {
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_Say_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}

	G_Say( ent, NULL, SAY_ALL, p );
}

/*
==================
Cmd_SayTeam_f
==================
*/
static void Cmd_SayTeam_f( gentity_t *ent ) {
	char *p = NULL;

	if ( trap->Argc () < 2 )
		return;

	p = ConcatArgs( 1 );

	if ( strlen( p ) >= MAX_SAY_TEXT ) {
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_SayTeam_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}

	G_Say( ent, NULL, (level.gametype>=GT_TEAM) ? SAY_TEAM : SAY_ALL, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap->Argc () < 3 ) {
		trap->SendServerCommand( ent-g_entities, "print \"Usage: tell <player id> <message>\n\"" );
		return;
	}

	trap->Argv( 1, arg, sizeof( arg ) );
	targetNum = ClientNumberFromString( ent, arg, qfalse );
	if ( targetNum == -1 ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	if ( strlen( p ) >= MAX_SAY_TEXT ) {
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_Tell_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
	G_Say( ent, target, SAY_TELL, p );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, p );
	}
}

//siege voice command
static void Cmd_VoiceCommand_f(gentity_t *ent)
{
	gentity_t *te;
	char arg[MAX_TOKEN_CHARS];
	char *s;
	int i = 0;

	if (level.gametype < GT_TEAM)
	{
		return;
	}

	if (trap->Argc() < 2)
	{
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		ent->client->tempSpectate >= level.time)
	{
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOICECHATASSPEC")) );
		return;
	}

	trap->Argv(1, arg, sizeof(arg));

	if (arg[0] == '*')
	{ //hmm.. don't expect a * to be prepended already. maybe someone is trying to be sneaky.
		return;
	}

	s = va("*%s", arg);

	//now, make sure it's a valid sound to be playing like this.. so people can't go around
	//screaming out death sounds or whatever.
	while (i < MAX_CUSTOM_SIEGE_SOUNDS)
	{
		if (!bg_customSiegeSoundNames[i])
		{
			break;
		}
		if (!Q_stricmp(bg_customSiegeSoundNames[i], s))
		{ //it matches this one, so it's ok
			break;
		}
		i++;
	}

	if (i == MAX_CUSTOM_SIEGE_SOUNDS || !bg_customSiegeSoundNames[i])
	{ //didn't find it in the list
		return;
	}

	te = G_TempEntity(vec3_origin, EV_VOICECMD_SOUND);
	te->s.groundEntityNum = ent->s.number;
	te->s.eventParm = G_SoundIndex((char *)bg_customSiegeSoundNames[i]);
	te->r.svFlags |= SVF_BROADCAST;
}


static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};
static size_t numgc_orders = ARRAY_LEN( gc_orders );

void Cmd_GameCommand_f( gentity_t *ent ) {
	int				targetNum;
	unsigned int	order;
	gentity_t		*target;
	char			arg[MAX_TOKEN_CHARS] = {0};

	if ( trap->Argc() != 3 ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"Usage: gc <player id> <order 0-%d>\n\"", numgc_orders - 1 ) );
		return;
	}

	trap->Argv( 2, arg, sizeof( arg ) );
	order = atoi( arg );

	if ( order >= numgc_orders ) {
		trap->SendServerCommand( ent-g_entities, va("print \"Bad order: %i\n\"", order));
		return;
	}

	trap->Argv( 1, arg, sizeof( arg ) );
	targetNum = ClientNumberFromString( ent, arg, qfalse );
	if ( targetNum == -1 )
		return;

	target = &g_entities[targetNum];
	if ( !target->inuse || !target->client )
		return;

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, gc_orders[order] );
	G_Say( ent, target, SAY_TELL, gc_orders[order] );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT) )
		G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	//JAC: This wasn't working for non-spectators since s.origin doesn't update for active players.
	if(ent->client && ent->client->sess.sessionTeam != TEAM_SPECTATOR )
	{//active players use currentOrigin
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->r.currentOrigin ) ) );
	}
	else
	{
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
	}
	//trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
}

static const char *gameNames[] = {
	"Free For All",
	"Holocron FFA",
	"Jedi Master",
	"Duel",
	"Power Duel",
	"Single Player",
	"Team FFA",
	"Siege",
	"Capture the Flag",
	"Capture the Ysalamiri"
};

/*
==================
Cmd_CallVote_f
==================
*/
extern void SiegeClearSwitchData(void); //g_saga.c

qboolean G_VoteCapturelimit( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	int n = Com_Clampi( 0, 0x7FFFFFFF, atoi( arg2 ) );
	Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, n );
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	return qtrue;
}

qboolean G_VoteClientkick( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	int n = atoi ( arg2 );

	if ( n < 0 || n >= level.maxclients ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"invalid client number %d.\n\"", n ) );
		return qfalse;
	}

	if ( g_entities[n].client->pers.connected == CON_DISCONNECTED ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"there is no client with the client number %d.\n\"", n ) );
		return qfalse;
	}

	Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, g_entities[n].client->pers.netname );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	return qtrue;
}

qboolean G_VoteFraglimit( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	int n = Com_Clampi( 0, 0x7FFFFFFF, atoi( arg2 ) );
	Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, n );
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	return qtrue;
}

qboolean G_VoteGametype( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	int gt = atoi( arg2 );

	// ffa, ctf, tdm, etc
	if ( arg2[0] && isalpha( arg2[0] ) ) {
		gt = BG_GetGametypeForString( arg2 );
		if ( gt == -1 )
		{
			trap->SendServerCommand( ent-g_entities, va( "print \"Gametype (%s) unrecognised, defaulting to FFA/Deathmatch\n\"", arg2 ) );
			gt = GT_FFA;
		}
	}
	// numeric but out of range
	else if ( gt < 0 || gt >= GT_MAX_GAME_TYPE ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"Gametype (%i) is out of range, defaulting to FFA/Deathmatch\n\"", gt ) );
		gt = GT_FFA;
	}

	// logically invalid gametypes, or gametypes not fully implemented in MP
	/*if ( gt == GT_SINGLE_PLAYER ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"This gametype is not supported (%s).\n\"", arg2 ) );
		return qfalse;
	}*/

	level.votingGametype = qtrue;
	level.votingGametypeTo = gt;

	Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, gt );
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, gameNames[gt] );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	return qtrue;
}

qboolean G_VoteKick( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	int clientid = ClientNumberFromString( ent, arg2, qtrue );
	gentity_t *target = NULL;

	if ( clientid == -1 )
		return qfalse;

	target = &g_entities[clientid];
	if ( !target || !target->inuse || !target->client )
		return qfalse;

	Com_sprintf( level.voteString, sizeof( level.voteString ), "clientkick %d", clientid );
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "kick %s", target->client->pers.netname );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	return qtrue;
}

const char *G_GetArenaInfoByMap( const char *map );

void Cmd_MapList_f( gentity_t *ent ) {
	int i, toggle=0;
	char map[24] = "--", buf[512] = {0};

	Q_strcat( buf, sizeof( buf ), "Map list:" );

	for ( i=0; i<level.arenas.num; i++ ) {
		Q_strncpyz( map, Info_ValueForKey( level.arenas.infos[i], "map" ), sizeof( map ) );
		Q_StripColor( map );

		if ( G_DoesMapSupportGametype( map, level.gametype ) ) {
			char *tmpMsg = va( " ^%c%s", (++toggle&1) ? COLOR_GREEN : COLOR_YELLOW, map );
			if ( strlen( buf ) + strlen( tmpMsg ) >= sizeof( buf ) ) {
				trap->SendServerCommand( ent-g_entities, va( "print \"%s\"", buf ) );
				buf[0] = '\0';
			}
			Q_strcat( buf, sizeof( buf ), tmpMsg );
		}
	}

	trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", buf ) );
}

qboolean G_VoteMap( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	char s[MAX_CVAR_VALUE_STRING] = {0}, bspName[MAX_QPATH] = {0}, *mapName = NULL, *mapName2 = NULL;
	fileHandle_t fp = NULL_FILE;
	const char *arenaInfo;

	// didn't specify a map, show available maps
	if ( numArgs < 3 ) {
		Cmd_MapList_f( ent );
		return qfalse;
	}

	if ( strchr( arg2, '\\' ) ) {
		trap->SendServerCommand( ent-g_entities, "print \"Can't have mapnames with a \\\n\"" );
		return qfalse;
	}

	Com_sprintf( bspName, sizeof(bspName), "maps/%s.bsp", arg2 );
	if ( trap->FS_Open( bspName, &fp, FS_READ ) <= 0 ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"Can't find map %s on server\n\"", bspName ) );
		if( fp != NULL_FILE )
			trap->FS_Close( fp );
		return qfalse;
	}
	trap->FS_Close( fp );

	if ( !G_DoesMapSupportGametype( arg2, level.gametype ) ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "NOVOTE_MAPNOTSUPPORTEDBYGAME" ) ) );
		return qfalse;
	}

	// preserve the map rotation
	trap->Cvar_VariableStringBuffer( "nextmap", s, sizeof( s ) );
	if ( *s )
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
	else
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );

	arenaInfo = G_GetArenaInfoByMap(arg2);
	if ( arenaInfo ) {
		mapName = Info_ValueForKey( arenaInfo, "longname" );
		mapName2 = Info_ValueForKey( arenaInfo, "map" );
	}

	if ( !mapName || !mapName[0] )
		mapName = "ERROR";

	if ( !mapName2 || !mapName2[0] )
		mapName2 = "ERROR";

	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "map %s (%s)", mapName, mapName2 );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	return qtrue;
}

qboolean G_VoteMapRestart( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	int n = Com_Clampi( 0, 60, atoi( arg2 ) );
	if ( numArgs < 3 )
		n = 5;
	Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, n );
	Q_strncpyz( level.voteDisplayString, level.voteString, sizeof( level.voteDisplayString ) );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	return qtrue;
}

qboolean G_VoteNextmap( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	char s[MAX_CVAR_VALUE_STRING];

	trap->Cvar_VariableStringBuffer( "nextmap", s, sizeof( s ) );
	if ( !*s ) {
		trap->SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
		return qfalse;
	}
	SiegeClearSwitchData();
	Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
	Q_strncpyz( level.voteDisplayString, level.voteString, sizeof( level.voteDisplayString ) );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	return qtrue;
}

qboolean G_VoteTimelimit( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	float tl = Com_Clamp( 0.0f, 35790.0f, atof( arg2 ) );
	if ( Q_isintegral( tl ) )
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, (int)tl );
	else
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %.3f", arg1, tl );
	Q_strncpyz( level.voteDisplayString, level.voteString, sizeof( level.voteDisplayString ) );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	return qtrue;
}

qboolean G_VoteWarmup( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	int n = Com_Clampi( 0, 1, atoi( arg2 ) );
	Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, n );
	Q_strncpyz( level.voteDisplayString, level.voteString, sizeof( level.voteDisplayString ) );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	return qtrue;
}

typedef struct voteString_s {
	const char	*string;
	const char	*aliases;	// space delimited list of aliases, will always show the real vote string
	qboolean	(*func)(gentity_t *ent, int numArgs, const char *arg1, const char *arg2);
	int			numArgs;	// number of REQUIRED arguments, not total/optional arguments
	uint32_t	validGT;	// bit-flag of valid gametypes
	qboolean	voteDelay;	// if true, will delay executing the vote string after it's accepted by g_voteDelay
	const char	*shortHelp;	// NULL if no arguments needed
} voteString_t;

static voteString_t validVoteStrings[] = {
	//	vote string				aliases										# args	valid gametypes							exec delay		short help
	{	"capturelimit",			"caps",				G_VoteCapturelimit,		1,		GTB_CTF|GTB_CTY,						qtrue,			"<num>" },
	{	"clientkick",			NULL,				G_VoteClientkick,		1,		GTB_ALL,								qfalse,			"<clientnum>" },
	{	"fraglimit",			"frags",			G_VoteFraglimit,		1,		GTB_ALL & ~(GTB_SIEGE|GTB_CTF|GTB_CTY),	qtrue,			"<num>" },
	{	"g_doWarmup",			"dowarmup warmup",	G_VoteWarmup,			1,		GTB_ALL,								qtrue,			"<0-1>" },
	{	"g_gametype",			"gametype gt mode",	G_VoteGametype,			1,		GTB_ALL,								qtrue,			"<num or name>" },
	{	"kick",					NULL,				G_VoteKick,				1,		GTB_ALL,								qfalse,			"<client name>" },
	{	"map",					NULL,				G_VoteMap,				0,		GTB_ALL,								qtrue,			"<name>" },
	{	"map_restart",			"restart",			G_VoteMapRestart,		0,		GTB_ALL,								qtrue,			"<optional delay>" },
	{	"nextmap",				NULL,				G_VoteNextmap,			0,		GTB_ALL,								qtrue,			NULL },
	{	"timelimit",			"time",				G_VoteTimelimit,		1,		GTB_ALL,								qtrue,			"<num>" },
};
static const int validVoteStringsSize = ARRAY_LEN( validVoteStrings );

void Cmd_CallVote_f( gentity_t *ent ) {
	int				i=0, numArgs=0;
	char			arg1[MAX_CVAR_VALUE_STRING] = {0};
	char			arg2[MAX_CVAR_VALUE_STRING] = {0};
	voteString_t	*vote = NULL;

	// not allowed to vote at all
	if ( !g_allowVote.integer ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "NOVOTE" ) ) );
		return;
	}

	// vote in progress
	else if ( level.voteTime ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "VOTEINPROGRESS" ) ) );
		return;
	}

	// can't vote as a spectator, except in (power)duel
	else if ( level.gametype != GT_DUEL && level.gametype != GT_POWERDUEL && ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "NOSPECVOTE" ) ) );
		return;
	}

	// make sure it is a valid command to vote on
	numArgs = trap->Argc();
	trap->Argv( 1, arg1, sizeof( arg1 ) );
	if ( numArgs > 1 )
		Q_strncpyz( arg2, ConcatArgs( 2 ), sizeof( arg2 ) );

	// filter ; \n \r
	if ( Q_strchrs( arg1, ";\r\n" ) || Q_strchrs( arg2, ";\r\n" ) ) {
		trap->SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	// check for invalid votes
	for ( i=0; i<validVoteStringsSize; i++ ) {
		if ( !(g_allowVote.integer & (1<<i)) )
			continue;

		if ( !Q_stricmp( arg1, validVoteStrings[i].string ) )
			break;

		// see if they're using an alias, and set arg1 to the actual vote string
		if ( validVoteStrings[i].aliases ) {
			char tmp[MAX_TOKEN_CHARS] = {0}, *p = NULL;
			const char *delim = " ";
			Q_strncpyz( tmp, validVoteStrings[i].aliases, sizeof( tmp ) );
			p = strtok( tmp, delim );
			while ( p != NULL ) {
				if ( !Q_stricmp( arg1, p ) ) {
					Q_strncpyz( arg1, validVoteStrings[i].string, sizeof( arg1 ) );
					goto validVote;
				}
				p = strtok( NULL, delim );
			}
		}
	}
	// invalid vote string, abandon ship
	if ( i == validVoteStringsSize ) {
		char buf[1024] = {0};
		int toggle = 0;
		trap->SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap->SendServerCommand( ent-g_entities, "print \"Allowed vote strings are: \"" );
		for ( i=0; i<validVoteStringsSize; i++ ) {
			if ( !(g_allowVote.integer & (1<<i)) )
				continue;

			toggle = !toggle;
			if ( validVoteStrings[i].shortHelp ) {
				Q_strcat( buf, sizeof( buf ), va( "^%c%s %s ",
					toggle ? COLOR_GREEN : COLOR_YELLOW,
					validVoteStrings[i].string,
					validVoteStrings[i].shortHelp ) );
			}
			else {
				Q_strcat( buf, sizeof( buf ), va( "^%c%s ",
					toggle ? COLOR_GREEN : COLOR_YELLOW,
					validVoteStrings[i].string ) );
			}
		}

		//FIXME: buffer and send in multiple messages in case of overflow
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", buf ) );
		return;
	}

validVote:
	vote = &validVoteStrings[i];
	if ( !(vote->validGT & (1<<level.gametype)) ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"%s is not applicable in this gametype.\n\"", arg1 ) );
		return;
	}

	if ( numArgs < vote->numArgs+2 ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"%s requires more arguments: %s\n\"", arg1, vote->shortHelp ) );
		return;
	}

	level.votingGametype = qfalse;

	level.voteExecuteDelay = vote->voteDelay ? g_voteDelay.integer : 0;

	// there is still a vote to be executed, execute it and store the new vote
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap->SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
	}

	// pass the args onto vote-specific handlers for parsing/filtering
	if ( vote->func ) {
		if ( !vote->func( ent, numArgs, arg1, arg2 ) )
			return;
	}
	// otherwise assume it's a command
	else {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Q_strncpyz( level.voteDisplayString, level.voteString, sizeof( level.voteDisplayString ) );
		Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	}
	Q_strstrip( level.voteStringClean, "\"\n\r", NULL );

	trap->SendServerCommand( -1, va( "print \"%s^7 %s (%s)\n\"", ent->client->pers.netname, G_GetStringEdString( "MP_SVGAME", "PLCALLEDVOTE" ), level.voteStringClean ) );

	// start the voting, the caller automatically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i=0; i<level.maxclients; i++ ) {
		level.clients[i].mGameFlags &= ~PSG_VOTED;
		level.clients[i].pers.vote = 0;
	}

	ent->client->mGameFlags |= PSG_VOTED;
	ent->client->pers.vote = 1;

	trap->SetConfigstring( CS_VOTE_TIME,	va( "%i", level.voteTime ) );
	trap->SetConfigstring( CS_VOTE_STRING,	level.voteDisplayString );
	trap->SetConfigstring( CS_VOTE_YES,		va( "%i", level.voteYes ) );
	trap->SetConfigstring( CS_VOTE_NO,		va( "%i", level.voteNo ) );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64] = {0};

	if ( !level.voteTime ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEINPROG")) );
		return;
	}
	if ( ent->client->mGameFlags & PSG_VOTED ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "VOTEALREADY")) );
		return;
	}
	if (level.gametype != GT_DUEL && level.gametype != GT_POWERDUEL)
	{
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
			return;
		}
	}

	trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLVOTECAST")) );

	ent->client->mGameFlags |= PSG_VOTED;

	trap->Argv( 1, msg, sizeof( msg ) );

	if ( tolower( msg[0] ) == 'y' || msg[0] == '1' ) {
		level.voteYes++;
		ent->client->pers.vote = 1;
		trap->SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	} else {
		level.voteNo++;
		ent->client->pers.vote = 2;
		trap->SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

qboolean G_TeamVoteLeader( gentity_t *ent, int cs_offset, team_t team, int numArgs, const char *arg1, const char *arg2 ) {
	int clientid = numArgs == 2 ? ent->s.number : ClientNumberFromString( ent, arg2, qfalse );
	gentity_t *target = NULL;

	if ( clientid == -1 )
		return qfalse;

	target = &g_entities[clientid];
	if ( !target || !target->inuse || !target->client )
		return qfalse;

	if ( target->client->sess.sessionTeam != team )
	{
		trap->SendServerCommand( ent-g_entities, va( "print \"User %s is not on your team\n\"", arg2 ) );
		return qfalse;
	}

	Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "leader %d", clientid );
	Q_strncpyz( level.teamVoteDisplayString[cs_offset], level.teamVoteString[cs_offset], sizeof( level.teamVoteDisplayString[cs_offset] ) );
	Q_strncpyz( level.teamVoteStringClean[cs_offset], level.teamVoteString[cs_offset], sizeof( level.teamVoteStringClean[cs_offset] ) );
	return qtrue;
}

//[ClassSyS]
/*
==================
Cmd_ChangeClasses_f
==================
*/
//void Cmd_ChangeClasses_f(gentity_t *ent)
//{
//	char *name;
//	//   int i;
//
//	// Get second parameter// cmd to call from console :)
//	name = ConcatArgs(1);
//	//I'm not really sure about this one - I think it gets the value of the first argument.
//	//if you passed 0 as an argument it would return the command itself i think...
//
//	Com_Printf("2nd parameter (%i chars) is %s\n", strlen(name), name);
//	//this is just for debugging. remove it later when you feel it works properly.
//
//	// the next part is a massive if-then-else tree. like i said b4, if two strings are
//	//equal, then the function Q_stricmp() should return 0. here, the argument of the 
//	//changeclass command is checked against various names like scout, sniper, spy, etc. 
//	//if any of the expressions evaluates to true, then the corresponding class is 
//	//assigned to the player. Otherwise it continues checking until it finds a matching 
//	//class.
//	// if it finds that the class you want to change to doesn't exist, then it will give 
//	//you a message "Invalid class " where is the class you gave.
//
//	if (Q_stricmp(name, "padawan") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_PLAYER;
//	}
//	else if (Q_stricmp(name, "trooper1") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_TROOPER_1;
//		ent->account.playerclass || PCLASS_TROOPER_1;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "trooper2") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_TROOPER_2;
//		ent->account.playerclass || PCLASS_TROOPER_2;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "trooper3") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_TROOPER_3;
//		ent->account.playerclass || PCLASS_TROOPER_3;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "jediknight1") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_JEDIKNIGHT_1;
//		ent->account.playerclass || PCLASS_JEDIKNIGHT_1;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "jediknight2") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_JEDIKNIGHT_2;
//		ent->account.playerclass || PCLASS_JEDIKNIGHT_2;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "jediknight3") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_JEDIKNIGHT_3;
//		ent->account.playerclass || PCLASS_JEDIKNIGHT_3;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "smuggler1") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_SMUGGLER_1;
//		ent->account.playerclass || PCLASS_SMUGGLER_1;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "smuggler2") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_SMUGGLER_2;
//		ent->account.playerclass || PCLASS_SMUGGLER_2;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "smuggler3") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_SMUGGLER_3;
//		ent->account.playerclass || PCLASS_SMUGGLER_3,
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "jediconsular1") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_JEDI_CONSULAR_1;
//		ent->account.playerclass || PCLASS_JEDI_CONSULAR_1;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "jediconsular2") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_JEDI_CONSULAR_2;
//		ent->account.playerclass || PCLASS_JEDI_CONSULAR_2;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "jediconsular3") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_JEDI_CONSULAR_3;
//		ent->account.playerclass || PCLASS_JEDI_CONSULAR_3;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//
//	else if (Q_stricmp(name, "bountyhunter1") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_BOUNTYHUNTER_1;
//		ent->account.playerclass || PCLASS_BOUNTYHUNTER_1;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "bountyhunter2") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_BOUNTYHUNTER_2;
//		ent->account.playerclass || PCLASS_BOUNTYHUNTER_2;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "bountyhunter3") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_BOUNTYHUNTER_3;
//		ent->account.playerclass || PCLASS_BOUNTYHUNTER_3;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "sithworrior1") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_SITHWORRIOR_1;
//		ent->account.playerclass || PCLASS_SITHWORRIOR_1;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "sithworrior2") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_SITHWORRIOR_2;
//		ent->account.playerclass || PCLASS_SITHWORRIOR_2;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "sithworrior3") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_SITHWORRIOR_3;
//		ent->account.playerclass || PCLASS_SITHWORRIOR_3;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "ipperialagent1") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_IPPERIAL_AGENT_1;
//		ent->account.playerclass || PCLASS_IPPERIAL_AGENT_1;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "ipperialagent2") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_IPPERIAL_AGENT_2;
//		ent->account.playerclass || PCLASS_IPPERIAL_AGENT_2;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "ipperialagent3") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_IPPERIAL_AGENT_3;
//		ent->account.playerclass || PCLASS_IPPERIAL_AGENT_3;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "sithinquisitor1") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_SITH_INQUISITOR_1;
//		ent->account.playerclass || PCLASS_SITH_INQUISITOR_1;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "sithinquisitor2") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_SITH_INQUISITOR_2;
//		ent->account.playerclass || PCLASS_SITH_INQUISITOR_2;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else if (Q_stricmp(name, "sithinquisitor3") == 0) {
//		ent->client->pers.nextplayerclasses = PCLASS_SITH_INQUISITOR_3;
//		ent->account.playerclass || PCLASS_SITH_INQUISITOR_3;
//		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
//		player_die(ent, ent, ent, 100000, MOD_SUICIDE);
//	}
//	else {
//		Com_Printf("Invalid class %s \n", name);
//		return;
//	}
//
//	Com_Printf("Your next player class is %s \n", name);
//	//	ent->changeClass = qtrue;
//	return;
//}

//[ClassSyS]

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
	team_t	team = ent->client->sess.sessionTeam;
	int		i=0, cs_offset=0, numArgs=0;
	char	arg1[MAX_CVAR_VALUE_STRING] = {0};
	char	arg2[MAX_CVAR_VALUE_STRING] = {0};

	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	// not allowed to vote at all
	if ( !g_allowTeamVote.integer ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
		return;
	}

	// vote in progress
	else if ( level.teamVoteTime[cs_offset] ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADY")) );
		return;
	}

	// can't vote as a spectator
	else if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSPECVOTE")) );
		return;
	}

	// make sure it is a valid command to vote on
	numArgs = trap->Argc();
	trap->Argv( 1, arg1, sizeof( arg1 ) );
	if ( numArgs > 1 )
		Q_strncpyz( arg2, ConcatArgs( 2 ), sizeof( arg2 ) );

	// filter ; \n \r
	if ( Q_strchrs( arg1, ";\r\n" ) || Q_strchrs( arg2, ";\r\n" ) ) {
		trap->SendServerCommand( ent-g_entities, "print \"Invalid team vote string.\n\"" );
		return;
	}

	// pass the args onto vote-specific handlers for parsing/filtering
	if ( !Q_stricmp( arg1, "leader" ) ) {
		if ( !G_TeamVoteLeader( ent, cs_offset, team, numArgs, arg1, arg2 ) )
			return;
	}
	else {
		trap->SendServerCommand( ent-g_entities, "print \"Invalid team vote string.\n\"" );
		trap->SendServerCommand( ent-g_entities, va("print \"Allowed team vote strings are: ^%c%s %s\n\"", COLOR_GREEN, "leader", "<optional client name or number>" ));
		return;
	}

	Q_strstrip( level.teamVoteStringClean[cs_offset], "\"\n\r", NULL );

	for ( i=0; i<level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED )
			continue;
		if ( level.clients[i].sess.sessionTeam == team )
			trap->SendServerCommand( i, va("print \"%s^7 called a team vote (%s)\n\"", ent->client->pers.netname, level.teamVoteStringClean[cs_offset] ) );
	}

	// start the voting, the caller autoamtically votes yes
	level.teamVoteTime[cs_offset] = level.time;
	level.teamVoteYes[cs_offset] = 1;
	level.teamVoteNo[cs_offset] = 0;

	for ( i=0; i<level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED )
			continue;
		if ( level.clients[i].sess.sessionTeam == team ) {
			level.clients[i].mGameFlags &= ~PSG_TEAMVOTED;
			level.clients[i].pers.teamvote = 0;
		}
	}
	ent->client->mGameFlags |= PSG_TEAMVOTED;
	ent->client->pers.teamvote = 1;

	trap->SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
	trap->SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteDisplayString[cs_offset] );
	trap->SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	trap->SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
	team_t		team = ent->client->sess.sessionTeam;
	int			cs_offset=0;
	char		msg[64] = {0};

	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOTEAMVOTEINPROG")) );
		return;
	}
	if ( ent->client->mGameFlags & PSG_TEAMVOTED ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADYCAST")) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
		return;
	}

	trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLTEAMVOTECAST")) );

	ent->client->mGameFlags |= PSG_TEAMVOTED;

	trap->Argv( 1, msg, sizeof( msg ) );

	if ( tolower( msg[0] ) == 'y' || msg[0] == '1' ) {
		level.teamVoteYes[cs_offset]++;
		ent->client->pers.teamvote = 1;
		trap->SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	} else {
		level.teamVoteNo[cs_offset]++;
		ent->client->pers.teamvote = 2;
		trap->SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( trap->Argc() != 5 ) {
		trap->SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap->Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap->Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}

void G_LeaveVehicle( gentity_t* ent, qboolean ConCheck ) {

	if (ent->client->ps.m_iVehicleNum)
	{ //tell it I'm getting off
		gentity_t *veh = &g_entities[ent->client->ps.m_iVehicleNum];

		if (veh->inuse && veh->client && veh->m_pVehicle)
		{
			if ( ConCheck ) { // check connection
				clientConnected_t pCon = ent->client->pers.connected;
				ent->client->pers.connected = CON_DISCONNECTED;
				veh->m_pVehicle->m_pVehicleInfo->Eject(veh->m_pVehicle, (bgEntity_t *)ent, qtrue);
				ent->client->pers.connected = pCon;
			} else { // or not.
				veh->m_pVehicle->m_pVehicleInfo->Eject(veh->m_pVehicle, (bgEntity_t *)ent, qtrue);
			}
		}
	}

	ent->client->ps.m_iVehicleNum = 0;
}

int G_ItemUsable(playerState_t *ps, int forcedUse)
{
	vec3_t fwd, fwdorg, dest, pos;
	vec3_t yawonly;
	vec3_t mins, maxs;
	vec3_t trtest;
	trace_t tr;

	// fix: dead players shouldn't use items
	if (ps->stats[STAT_HEALTH] <= 0) {
		return 0;
	}

	if (ps->m_iVehicleNum)
	{
		return 0;
	}

	if (ps->pm_flags & PMF_USE_ITEM_HELD)
	{ //force to let go first
		return 0;
	}

	if (!forcedUse)
	{
		forcedUse = bg_itemlist[ps->stats[STAT_HOLDABLE_ITEM]].giTag;
	}

	if (!BG_IsItemSelectable(ps, forcedUse))
	{
		return 0;
	}

	switch (forcedUse)
	{
	case HI_MEDPAC:
	case HI_MEDPAC_BIG:
		if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH])
		{
			return 0;
		}

		if (ps->stats[STAT_HEALTH] <= 0)
		{
			return 0;
		}

		return 1;
	case HI_SEEKER:
		if (ps->eFlags & EF_SEEKERDRONE)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SEEKER_ALREADYDEPLOYED);
			return 0;
		}

		return 1;
	case HI_SENTRY_GUN:
		if (ps->fd.sentryDeployed)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_ALREADYPLACED);
			return 0;
		}

		yawonly[ROLL] = 0;
		yawonly[PITCH] = 0;
		yawonly[YAW] = ps->viewangles[YAW];

		VectorSet( mins, -8, -8, 0 );
		VectorSet( maxs, 8, 8, 24 );

		AngleVectors(yawonly, fwd, NULL, NULL);

		fwdorg[0] = ps->origin[0] + fwd[0]*64;
		fwdorg[1] = ps->origin[1] + fwd[1]*64;
		fwdorg[2] = ps->origin[2] + fwd[2]*64;

		trtest[0] = fwdorg[0] + fwd[0]*16;
		trtest[1] = fwdorg[1] + fwd[1]*16;
		trtest[2] = fwdorg[2] + fwd[2]*16;

		trap->Trace(&tr, ps->origin, mins, maxs, trtest, ps->clientNum, MASK_PLAYERSOLID, qfalse, 0, 0);

		if ((tr.fraction != 1 && tr.entityNum != ps->clientNum) || tr.startsolid || tr.allsolid)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_NOROOM);
			return 0;
		}

		return 1;
	case HI_SHIELD:
		mins[0] = -8;
		mins[1] = -8;
		mins[2] = 0;

		maxs[0] = 8;
		maxs[1] = 8;
		maxs[2] = 8;

		AngleVectors (ps->viewangles, fwd, NULL, NULL);
		fwd[2] = 0;
		VectorMA(ps->origin, 64, fwd, dest);
		trap->Trace(&tr, ps->origin, mins, maxs, dest, ps->clientNum, MASK_SHOT, qfalse, 0, 0 );
		if (tr.fraction > 0.9 && !tr.startsolid && !tr.allsolid)
		{
			VectorCopy(tr.endpos, pos);
			VectorSet( dest, pos[0], pos[1], pos[2] - 4096 );
			trap->Trace( &tr, pos, mins, maxs, dest, ps->clientNum, MASK_SOLID, qfalse, 0, 0 );
			if ( !tr.startsolid && !tr.allsolid )
			{
				return 1;
			}
		}
		G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SHIELD_NOROOM);
		return 0;
	case HI_JETPACK: //do something?
		return 1;
	case HI_HEALTHDISP:
		return 1;
	case HI_AMMODISP:
		return 1;
	case HI_EWEB:
		return 1;
	case HI_CLOAK:
		return 1;
	default:
		return 1;
	}
}

void saberKnockDown(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other);

void Cmd_ToggleSaber_f(gentity_t *ent)
{
	if (ent->client->ps.fd.forceGripCripple)
	{ //if they are being gripped, don't let them unholster their saber
		if (ent->client->ps.saberHolstered)
		{
			return;
		}
	}

	if (ent->client->ps.saberInFlight)
	{
		if (ent->client->ps.saberEntityNum)
		{ //turn it off in midair
			saberKnockDown(&g_entities[ent->client->ps.saberEntityNum], ent, ent);
		}
		return;
	}

	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

//	if (ent->client->ps.duelInProgress && !ent->client->ps.saberHolstered)
//	{
//		return;
//	}

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.saberLockTime >= level.time)
	{
		return;
	}

	if (ent->client && ent->client->ps.weaponTime < 1)
	{
		if (ent->client->ps.saberHolstered == 2)
		{
			ent->client->ps.saberHolstered = 0;

			if (ent->client->saber[0].soundOn)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
			}
			if (ent->client->saber[1].soundOn)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
			}
		}
		else
		{
			ent->client->ps.saberHolstered = 2;
			if (ent->client->saber[0].soundOff)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
			}
			if (ent->client->saber[1].soundOff &&
				ent->client->saber[1].model[0])
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
			}
			//prevent anything from being done for 400ms after holster
			ent->client->ps.weaponTime = 400;
		}
	}
}

extern vmCvar_t		d_saberStanceDebug;

extern qboolean WP_SaberCanTurnOffSomeBlades( saberInfo_t *saber );

//#define __TEST_ALL_STANCES__ // Enable this to test all stances...
//#define __TEST_ORIGINAL_STANCES__ // Enable this to test old stances...

void Cmd_SaberAttackCycle_f(gentity_t *ent)
{
	int selectLevel = 0;
	//int ANIM_STYLE = ent->client->ps.saberMoveStyle;
	//int ORIGINAL_ANIM_STYLE = ent->client->ps.saberMoveStyle;
	qboolean usingSiegeStyle = qfalse;

	if ( !ent || !ent->client )
	{
		return;
	}
	if ( ent->client->ps.weapon != WP_SABER )
	{
        return;
	}
	/*
	if (ent->client->ps.weaponTime > 0)
	{ //no switching attack level when busy
		return;
	}
	*/

	//
	// FIXME - remove ALL trap->SendServerCommand and add CGAME display...
	//

#if 0

#ifdef __TEST_ALL_STANCES__
	ent->client->ps.fd.saberAnimLevelBase = SS_FAST;

	/*
	SS_NONE=0,
	SS_FAST,
	SS_MEDIUM,
	SS_STRONG,
	SS_DESANN,
	SS_TAVION,
	SS_DUAL,
	SS_STAFF,
	SS_NUM_SABER_STYLES
	*/

	selectLevel = ent->client->ps.fd.saberAnimLevel;

	selectLevel++;

	if (selectLevel >= SS_NUM_SABER_STYLES)
	{
		selectLevel = SS_FAST;
		ANIM_STYLE++;
	}

	if (ANIM_STYLE > 5) ANIM_STYLE = 0;

	trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7STYLE #%i - STANCE #%i^5.\n\"", ANIM_STYLE, selectLevel));

	if (ent->client->ps.weaponTime <= 0)
	{ //not busy, set it now
		ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
	}
	else
	{ //can't set it now or we might cause unexpected chaining, so queue it
		ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
	}

	ent->client->ps.saberMoveStyle = ANIM_STYLE;
	return;

#else //!__TEST_ALL_STANCES__

#ifndef __TEST_ORIGINAL_STANCES__
	if (ent->client->saber[0].model[0] && ent->client->saber[1].model[0])
	{ //dual sabers
		//
		// Dual Sabers...
		//

		ent->client->ps.fd.saberAnimLevelBase = SS_DUAL;

		if (ANIM_STYLE == 1
			&& ent->client->ps.fd.saberAnimLevel == SS_DESANN)
		{// Cycle to #5.
			ent->client->ps.saberHolstered = 1; // Turn OFF one saber...
			ANIM_STYLE = 0;
			selectLevel = SS_MEDIUM;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER^5.\n\""));
		}
		else if (ANIM_STYLE == 1
			&& ent->client->ps.fd.saberAnimLevel == SS_TAVION)
		{// Cycle to #4.
			ANIM_STYLE = 1;
			selectLevel = SS_DESANN;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUAL SABER #4^5.\n\""));
		}
		else if (ANIM_STYLE == 2
			&& ent->client->ps.fd.saberAnimLevel == SS_DESANN)
		{// Cycle to #3.
			ANIM_STYLE = 1;
			selectLevel = SS_TAVION;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUAL SABER #3^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_DUAL)
		{// Cycle to #2.
			ANIM_STYLE = 2;
			selectLevel = SS_DESANN;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUAL SABER #2^5.\n\""));
		}
		else
		{// Cycle to #1. (Original)
			ent->client->ps.saberHolstered = 0; // Turn ON both sabers...
			ANIM_STYLE = 0;
			selectLevel = SS_DUAL;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUAL SABER #1^5.\n\""));
		}

		if (ent->client->ps.weaponTime <= 0)
		{ //not busy, set it now
			ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
		}
		else
		{ //can't set it now or we might cause unexpected chaining, so queue it
			ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
		}

		ent->client->ps.saberMoveStyle = ANIM_STYLE;

		//				G_Printf("^1*** ^3Saber Stance Change:^5 Player ^7%s^5 changed to animation style #^7%i^5, saber level #^7%i^5.\n", ent->client->pers.netname, 
		//					ANIM_STYLE, selectLevel);

		return;
	}
	else if (ent->client->saber[0].numBlades > 1)
	{ //use staff stance then. -- (dual blade)
		//
		// Dual Blade Saber...
		//

		ent->client->ps.fd.saberAnimLevelBase = SS_STAFF;

		if (ANIM_STYLE == 1
			&& ent->client->ps.fd.saberAnimLevel == SS_TAVION)
		{// Cycle to #5.
			ANIM_STYLE = 1;
			selectLevel = SS_DESANN;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE BLADE #2^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_DUAL)
		{// Cycle to #4.
			ent->client->ps.saberHolstered = 1; // Turn OFF one blade...
			ANIM_STYLE = 1;
			selectLevel = SS_TAVION;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE BLADE #1^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_TAVION)
		{// Cycle to #3.
			ANIM_STYLE = 0;
			selectLevel = SS_DUAL;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUALBLADE #3^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_STAFF)
		{// Cycle to #2.
			ANIM_STYLE = 0;
			selectLevel = SS_TAVION;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUALBLADE #2^5.\n\""));
		}
		else
		{// Cycle to #1. (Original)
			ent->client->ps.saberHolstered = 0; // Turn ON both blades...
			ANIM_STYLE = 0;
			selectLevel = SS_STAFF;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUALBLADE #1^5.\n\""));
		}

		if (ent->client->ps.weaponTime <= 0)
		{ //not busy, set it now
			ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
		}
		else
		{ //can't set it now or we might cause unexpected chaining, so queue it
			ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
		}

		ent->client->ps.saberMoveStyle = ANIM_STYLE;

		//				G_Printf("^1*** ^3Saber Stance Change:^5 Player ^7%s^5 changed to animation style #^7%i^5, saber level #^7%i^5.\n", ent->client->pers.netname, 
		//					ANIM_STYLE, selectLevel);

		return;
	}
#else //__TEST_ORIGINAL_STANCES__
	if (ent->client->saber[0].model[0] && ent->client->saber[1].model[0])
	{ //dual sabers
		//
		// Dual Sabers...
		//

		ent->client->ps.fd.saberAnimLevelBase = SS_DUAL;

		if ( ent->client->ps.saberHolstered == 1 )
		{//have one holstered...
			if (ANIM_STYLE == 4
				&& ent->client->ps.fd.saberAnimLevel == 3)
			{// Done. Cycle back to single saber modes.
				ANIM_STYLE = 0;
				selectLevel = SS_DUAL;
				ent->client->ps.saberHolstered = 0;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUAL SABER #1^5.\n\""));
			}
			else if (ANIM_STYLE == 3
				&& ent->client->ps.fd.saberAnimLevel == 3)
			{// Cycle to #6.
				ANIM_STYLE = 4;
				selectLevel = SS_STRONG;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #6^5.\n\""));
			}
			else if (ANIM_STYLE == 2
				&& ent->client->ps.fd.saberAnimLevel == 4)
			{// Cycle to #5.
				ANIM_STYLE = 3;
				selectLevel = SS_STRONG;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #5^5.\n\""));
			}
			else if (ANIM_STYLE == 1
				&& ent->client->ps.fd.saberAnimLevel == 5)
			{// Cycle to #4.
				ANIM_STYLE = 2;
				selectLevel = SS_DESANN;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #4^5.\n\""));
			}
			else if (ANIM_STYLE == 0
				&& ent->client->ps.fd.saberAnimLevel == SS_FAST)
			{// Cycle to #3.
				ANIM_STYLE = 1;
				selectLevel = SS_TAVION;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #3^5.\n\""));
			}
			else if (ANIM_STYLE == 0
				&& ent->client->ps.fd.saberAnimLevel == SS_DUAL)
			{// Cycle to #2.
				ANIM_STYLE = 0;
				selectLevel = SS_FAST;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #2^5.\n\""));
			}
			else
			{// Cycle to #1. (Original)
				ANIM_STYLE = 0;
				selectLevel = SS_DUAL;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #1^5.\n\""));
			}

			if (ent->client->ps.weaponTime <= 0)
			{ //not busy, set it now
				ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
			}
			else
			{ //can't set it now or we might cause unexpected chaining, so queue it
				ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
			}

			ent->client->ps.saberMoveStyle = ANIM_STYLE;

			//				G_Printf("^1*** ^3Saber Stance Change:^5 Player ^7%s^5 changed to animation style #^7%i^5, saber level #^7%i^5.\n", ent->client->pers.netname, 
			//					ANIM_STYLE, selectLevel);

			return;
		}
		else if ( ent->client->ps.saberHolstered == 0)
		{//have none holstered...
			if (ANIM_STYLE == 4
				&& ent->client->ps.fd.saberAnimLevel == SS_STRONG)
			{// Done. Cycle back to single saber modes.
				ANIM_STYLE = 0;
				selectLevel = SS_DUAL;
				ent->client->ps.saberHolstered = 1;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #1^5.\n\""));
			}
			else if (ANIM_STYLE == 3
				&& ent->client->ps.fd.saberAnimLevel == SS_STRONG)
			{// Cycle to #5.
				ANIM_STYLE = 4;
				selectLevel = SS_STRONG;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUAL SABER #5^5.\n\""));
			}
			else if (ANIM_STYLE == 2
				&& ent->client->ps.fd.saberAnimLevel == SS_DESANN)
			{// Cycle to #4.
				ANIM_STYLE = 3;
				selectLevel = SS_STRONG;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUAL SABER #4^5.\n\""));
			}
			else if (ANIM_STYLE == 1
				&& ent->client->ps.fd.saberAnimLevel == SS_TAVION)
			{// Cycle to #3.
				ANIM_STYLE = 2;
				selectLevel = SS_DESANN;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUAL SABER #3^5.\n\""));
			}
			else if (ANIM_STYLE == 0
				&& ent->client->ps.fd.saberAnimLevel == SS_DUAL)
			{// Cycle to #2.
				ANIM_STYLE = 1;
				selectLevel = SS_TAVION;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUAL SABER #2^5.\n\""));
			}
			else
			{// Cycle to #1. (Original)
				ANIM_STYLE = 0;
				selectLevel = SS_DUAL;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUAL SABER #1^5.\n\""));
			}

			if (ent->client->ps.weaponTime <= 0)
			{ //not busy, set it now
				ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
			}
			else
			{ //can't set it now or we might cause unexpected chaining, so queue it
				ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
			}

			ent->client->ps.saberMoveStyle = ANIM_STYLE;

			//				G_Printf("^1*** ^3Saber Stance Change:^5 Player ^7%s^5 changed to animation style #^7%i^5, saber level #^7%i^5.\n", ent->client->pers.netname, 
			//					ANIM_STYLE, selectLevel);

			return;
		}
	}
	else if (ent->client->saber[0].numBlades > 1)
	{ //use staff stance then. -- (dual blade)
		//
		// Dual Blade Saber...
		//

		ent->client->ps.fd.saberAnimLevelBase = SS_STAFF;

		if ( ent->client->ps.saberHolstered == 1 )
		{//have one holstered...
			if (ANIM_STYLE == 3
				&& ent->client->ps.fd.saberAnimLevel == SS_DESANN)
			{// Done. Cycle back to single saber modes.
				ANIM_STYLE = 0;
				selectLevel = SS_STAFF;
				ent->client->ps.saberHolstered = 0;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUALBLADE #1^5.\n\""));
			}
			else if (ANIM_STYLE == 2
				&& ent->client->ps.fd.saberAnimLevel == SS_TAVION)
			{// Cycle to #5.
				ANIM_STYLE = 3;
				selectLevel = SS_DESANN;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLEBLADE #5^5.\n\""));
			}
			else if (ANIM_STYLE == 2
				&& ent->client->ps.fd.saberAnimLevel == SS_DESANN)
			{// Cycle to #4.
				ANIM_STYLE = 2;
				selectLevel = SS_TAVION;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLEBLADE #4^5.\n\""));
			}
			else if (ANIM_STYLE == 1
				&& ent->client->ps.fd.saberAnimLevel == SS_TAVION)
			{// Cycle to #3.
				ANIM_STYLE = 2;
				selectLevel = SS_DESANN;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLEBLADE #3^5.\n\""));
			}
			else if (ANIM_STYLE == 0
				&& ent->client->ps.fd.saberAnimLevel == SS_STAFF)
			{// Cycle to #2.
				ANIM_STYLE = 1;
				selectLevel = SS_TAVION;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLEBLADE #2^5.\n\""));
			}
			else
			{// Cycle to #1. (Original)
				ANIM_STYLE = 0;
				selectLevel = SS_STAFF;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLEBLADE #1^5.\n\""));
			}

			if (ent->client->ps.weaponTime <= 0)
			{ //not busy, set it now
				ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
			}
			else
			{ //can't set it now or we might cause unexpected chaining, so queue it
				ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
			}

			ent->client->ps.saberMoveStyle = ANIM_STYLE;

			//				G_Printf("^1*** ^3Saber Stance Change:^5 Player ^7%s^5 changed to animation style #^7%i^5, saber level #^7%i^5.\n", ent->client->pers.netname, 
			//					ANIM_STYLE, selectLevel);

			return;
		}
		else if ( ent->client->ps.saberHolstered == 0)
		{//have none holstered...
			if (ANIM_STYLE == 4
				&& ent->client->ps.fd.saberAnimLevel == SS_DESANN)
			{// Done. Cycle back to single saber modes.
				ANIM_STYLE = 0;
				selectLevel = SS_STAFF;
				ent->client->ps.saberHolstered = 1;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLEBLADE #1^5.\n\""));
			}
			else if (ANIM_STYLE == 3
				&& ent->client->ps.fd.saberAnimLevel == SS_DESANN)
			{// Cycle to #5.
				ANIM_STYLE = 4;
				selectLevel = SS_DESANN;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUALBLADE #7^5.\n\""));
			}
			else if (ANIM_STYLE == 3
				&& ent->client->ps.fd.saberAnimLevel == SS_STRONG)
			{// Cycle to #5.
				ANIM_STYLE = 3;
				selectLevel = SS_DESANN;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUALBLADE #6^5.\n\""));
			}
			else if (ANIM_STYLE == 2
				&& ent->client->ps.fd.saberAnimLevel == SS_TAVION)
			{// Cycle to #4.
				ANIM_STYLE = 3;
				selectLevel = SS_STRONG;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUALBLADE #5^5.\n\""));
			}
			else if (ANIM_STYLE == 2
				&& ent->client->ps.fd.saberAnimLevel == SS_DESANN)
			{// Cycle to #4.
				ANIM_STYLE = 2;
				selectLevel = SS_TAVION;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUALBLADE #4^5.\n\""));
			}
			else if (ANIM_STYLE == 1
				&& ent->client->ps.fd.saberAnimLevel == SS_TAVION)
			{// Cycle to #3.
				ANIM_STYLE = 2;
				selectLevel = SS_DESANN;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUALBLADE #3^5.\n\""));
			}
			else if (ANIM_STYLE == 0
				&& ent->client->ps.fd.saberAnimLevel == SS_STAFF)
			{// Cycle to #2.
				ANIM_STYLE = 1;
				selectLevel = SS_TAVION;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUALBLADE #2^5.\n\""));
			}
			else
			{// Cycle to #1. (Original)
				ANIM_STYLE = 0;
				selectLevel = SS_STAFF;
				trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7DUALBLADE #1^5.\n\""));
			}

			if (ent->client->ps.weaponTime <= 0)
			{ //not busy, set it now
				ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
			}
			else
			{ //can't set it now or we might cause unexpected chaining, so queue it
				ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
			}

			ent->client->ps.saberMoveStyle = ANIM_STYLE;

			//				G_Printf("^1*** ^3Saber Stance Change:^5 Player ^7%s^5 changed to animation style #^7%i^5, saber level #^7%i^5.\n", ent->client->pers.netname, 
			//					ANIM_STYLE, selectLevel);

			return;
		}
	}
#endif //__TEST_ORIGINAL_STANCES__

	if (ent->client->saberCycleQueue)
	{ //resume off of the queue if we haven't gotten a chance to update it yet
		selectLevel = ent->client->saberCycleQueue;
	}
	else
	{
		selectLevel = ent->client->ps.fd.saberAnimLevel;
	}

	if (level.gametype == GT_SIEGE &&
		ent->client->siegeClass != -1 &&
		bgSiegeClasses[ent->client->siegeClass].saberStance)
	{ //we have a flag of useable stances so cycle through it instead
		int i = selectLevel+1;

		usingSiegeStyle = qtrue;

		while (i != selectLevel)
		{ //cycle around upward til we hit the next style or end up back on this one
			if (i >= SS_NUM_SABER_STYLES)
			{ //loop back around to the first valid
				i = SS_FAST;
			}

			if (bgSiegeClasses[ent->client->siegeClass].saberStance & (1 << i))
			{ //we can use this one, select it and break out.
				selectLevel = i;
				break;
			}
			i++;
		}

		if (d_saberStanceDebug.integer)
		{
			trap->SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to cycle given class stance.\n\"") );
		}

		ent->client->ps.saberMoveStyle = 0; // UQ1: In seige always use style 0 (original)...
	}
	else
	{
		//
		// Single Saber...
		//

#ifdef __TEST_ORIGINAL_STANCES__
		if (ANIM_STYLE == 3
			&& ent->client->ps.fd.saberAnimLevel == SS_STRONG)
		{// Cycle to #10.
			ANIM_STYLE = 4;
			selectLevel = SS_STRONG;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #10^5.\n\""));
		}
		else if (ANIM_STYLE == 2
			&& ent->client->ps.fd.saberAnimLevel == SS_DESANN)
		{// Cycle to #9.
			ANIM_STYLE = 3;
			selectLevel = SS_STRONG;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #9^5.\n\""));
		}
		else if (ANIM_STYLE == 1
			&& ent->client->ps.fd.saberAnimLevel == SS_TAVION)
		{// Cycle to #8.
			ANIM_STYLE = 2;
			selectLevel = SS_DESANN;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #8^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_TAVION)
		{// Cycle to #7.
			ANIM_STYLE = 1;
			selectLevel = SS_TAVION;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #7^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_DESANN)
		{// Cycle to #6.
			ANIM_STYLE = 0;
			selectLevel = SS_TAVION;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #6^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_DUAL)
		{// Cycle to #5.
			ANIM_STYLE = 0;
			selectLevel = SS_DESANN;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #5^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_STRONG)
		{// Cycle to #4.
			ANIM_STYLE = 0;
			selectLevel = SS_DUAL;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #4^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_MEDIUM)
		{// Cycle to #3.
			ANIM_STYLE = 0;
			selectLevel = SS_STRONG;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #3^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_FAST)
		{// Cycle to #2.
			ANIM_STYLE = 0;
			selectLevel = SS_MEDIUM;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #2^5.\n\""));
		}
		else
		{// Cycle to #1. (Original)
			ANIM_STYLE = 0;
			selectLevel = SS_FAST;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #1^5.\n\""));
		}

		if (ent->client->ps.weaponTime <= 0)
		{ //not busy, set it now
			ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
		}
		else
		{ //can't set it now or we might cause unexpected chaining, so queue it
			ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
		}

		ent->client->ps.saberMoveStyle = ANIM_STYLE;

#else //!__TEST_ORIGINAL_STANCES__
		// UQ1: These are all (except db stance) the original stances... Enough for now...
		if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_DESANN)
		{// Cycle to #6.
			ANIM_STYLE = 0;
			selectLevel = SS_TAVION;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #6^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_DUAL)
		{// Cycle to #5.
			ANIM_STYLE = 0;
			selectLevel = SS_DESANN;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #5^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_STRONG)
		{// Cycle to #4.
			ANIM_STYLE = 0;
			selectLevel = SS_DUAL;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #4^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_MEDIUM)
		{// Cycle to #3.
			ANIM_STYLE = 0;
			selectLevel = SS_STRONG;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #3^5.\n\""));
		}
		else if (ANIM_STYLE == 0
			&& ent->client->ps.fd.saberAnimLevel == SS_FAST)
		{// Cycle to #2.
			ANIM_STYLE = 0;
			selectLevel = SS_MEDIUM;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #2^5.\n\""));
		}
		else
		{// Cycle to #1. (Original)
			ANIM_STYLE = 0;
			selectLevel = SS_FAST;
			trap->SendServerCommand( ent->client->ps.clientNum, va("cp \"^3Saber Stance Change:\n\n^7SINGLE SABER #1^5.\n\""));
		}

		if (ent->client->ps.weaponTime <= 0)
		{ //not busy, set it now
			ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
		}
		else
		{ //can't set it now or we might cause unexpected chaining, so queue it
			ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
		}

		ent->client->ps.saberMoveStyle = ANIM_STYLE;

#endif //__TEST_ORIGINAL_STANCES__

		//				G_Printf("^1*** ^3Saber Stance Change:^5 Player ^7%s^5 changed to animation style #^7%i^5, saber level #^7%i^5.\n", ent->client->pers.netname, 
		//					ANIM_STYLE, selectLevel);

		return;
	}
	/*
	#ifndef FINAL_BUILD
	switch ( selectLevel )
	{
	case FORCE_LEVEL_1:
	trap->SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sfast\n\"", S_COLOR_BLUE) );
	break;
	case FORCE_LEVEL_2:
	trap->SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %smedium\n\"", S_COLOR_YELLOW) );
	break;
	case FORCE_LEVEL_3:
	trap->SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sstrong\n\"", S_COLOR_RED) );
	break;
	}
	#endif
	*/
	if ( !usingSiegeStyle )
	{
		//make sure it's valid, change it if not
		WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &selectLevel );
	}

	if (ent->client->ps.weaponTime <= 0)
	{ //not busy, set it now
		ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
	}
	else
	{ //can't set it now or we might cause unexpected chaining, so queue it
		ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
	}
#endif //__TEST_ALL_STANCES__
#endif //0

	/*
	selectLevel = SS_FAST;
	selectLevel = SS_MEDIUM;
	selectLevel = SS_STRONG;
	selectLevel = SS_TAVION;
	selectLevel = SS_DESANN;
	selectLevel = SS_DUAL;
	selectLevel = SS_STAFF;
	*/

	if (ent->client->saber[0].model[0] && ent->client->saber[1].model[0])
	{// Dual Sabers...
		selectLevel = SS_DUAL;
	}
	else if (ent->client->saber[0].numBlades > 1)
	{// Dual Blade...
		selectLevel = SS_STAFF;
	}
	else
	{// Single Saber...
		selectLevel = SS_DUAL;
	}

	ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
}

qboolean G_OtherPlayersDueling(void)
{
	int i = 0;
	gentity_t *ent;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];

		if (ent && ent->inuse && ent->client && ent->client->ps.duelInProgress)
		{
			return qtrue;
		}
		i++;
	}

	return qfalse;
}

void Cmd_EngageDuel_f(gentity_t *ent)
{
	trace_t tr;
	vec3_t forward, fwdOrg;

	if (!g_privateDuel.integer)
	{
		return;
	}

	if (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
	{ //rather pointless in this mode..
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
		return;
	}

	//if (level.gametype >= GT_TEAM && level.gametype != GT_SIEGE)
	if (level.gametype >= GT_TEAM)
	{ //no private dueling in team modes
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
		return;
	}

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

	/*
	if (!ent->client->ps.saberHolstered)
	{ //must have saber holstered at the start of the duel
		return;
	}
	*/
	//NOTE: No longer doing this..

	if (ent->client->ps.saberInFlight)
	{
		return;
	}

	if (ent->client->ps.duelInProgress)
	{
		return;
	}

	//New: Don't let a player duel if he just did and hasn't waited 10 seconds yet (note: If someone challenges him, his duel timer will reset so he can accept)
	/*if (ent->client->ps.fd.privateDuelTime > level.time)
	{
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_JUSTDID")) );
		return;
	}

	if (G_OtherPlayersDueling())
	{
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_BUSY")) );
		return;
	}*/

	AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

	fwdOrg[0] = ent->client->ps.origin[0] + forward[0]*256;
	fwdOrg[1] = ent->client->ps.origin[1] + forward[1]*256;
	fwdOrg[2] = (ent->client->ps.origin[2]+ent->client->ps.viewheight) + forward[2]*256;

	trap->Trace(&tr, ent->client->ps.origin, NULL, NULL, fwdOrg, ent->s.number, MASK_PLAYERSOLID, qfalse, 0, 0);

	if (tr.fraction != 1 && tr.entityNum < MAX_CLIENTS)
	{
		gentity_t *challenged = &g_entities[tr.entityNum];

		if (!challenged || !challenged->client || !challenged->inuse ||
			challenged->health < 1 || challenged->client->ps.stats[STAT_HEALTH] < 1 ||
			challenged->client->ps.weapon != WP_SABER || challenged->client->ps.duelInProgress ||
			challenged->client->ps.saberInFlight)
		{
			return;
		}

		if (level.gametype >= GT_TEAM && OnSameTeam(ent, challenged))
		{
			return;
		}

		if (challenged->client->ps.duelIndex == ent->s.number && challenged->client->ps.duelTime >= level.time)
		{
			trap->SendServerCommand( /*challenged-g_entities*/-1, va("print \"%s %s %s!\n\"", challenged->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELACCEPT"), ent->client->pers.netname) );

			ent->client->ps.duelInProgress = qtrue;
			challenged->client->ps.duelInProgress = qtrue;

			ent->client->ps.duelTime = level.time + 2000;
			challenged->client->ps.duelTime = level.time + 2000;

			G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
			G_AddEvent(challenged, EV_PRIVATE_DUEL, 1);

			//Holster their sabers now, until the duel starts (then they'll get auto-turned on to look cool)

			if (!ent->client->ps.saberHolstered)
			{
				if (ent->client->saber[0].soundOff)
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
				}
				if (ent->client->saber[1].soundOff &&
					ent->client->saber[1].model[0])
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
				}
				ent->client->ps.weaponTime = 400;
				ent->client->ps.saberHolstered = 2;
			}
			if (!challenged->client->ps.saberHolstered)
			{
				if (challenged->client->saber[0].soundOff)
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[0].soundOff);
				}
				if (challenged->client->saber[1].soundOff &&
					challenged->client->saber[1].model[0])
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[1].soundOff);
				}
				challenged->client->ps.weaponTime = 400;
				challenged->client->ps.saberHolstered = 2;
			}
		}
		else
		{
			//Print the message that a player has been challenged in private, only announce the actual duel initiation in private
			trap->SendServerCommand( challenged-g_entities, va("cp \"%s %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGE")) );
			trap->SendServerCommand( ent-g_entities, va("cp \"%s %s\n\"", G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGED"), challenged->client->pers.netname) );
		}

		challenged->client->ps.fd.privateDuelTime = 0; //reset the timer in case this player just got out of a duel. He should still be able to accept the challenge.

		ent->client->ps.forceHandExtend = HANDEXTEND_DUELCHALLENGE;
		ent->client->ps.forceHandExtendTime = level.time + 1000;

		ent->client->ps.duelIndex = challenged->s.number;
		ent->client->ps.duelTime = level.time + 5000;
	}
}

#ifndef FINAL_BUILD
extern stringID_table_t animTable[MAX_ANIMATIONS+1];

void Cmd_DebugSetSaberMove_f(gentity_t *self)
{
	int argNum = trap->Argc();
	char arg[MAX_STRING_CHARS];

	if (argNum < 2)
	{
		return;
	}

	trap->Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	self->client->ps.saberMove = atoi(arg);
	self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;

	if (self->client->ps.saberMove >= LS_MOVE_MAX)
	{
		self->client->ps.saberMove = LS_MOVE_MAX-1;
	}

	Com_Printf("Anim for move: %s\n", animTable[saberMoveData/*[self->client->ps.saberMoveStyle]*/[self->client->ps.saberMove].animToUse].name);
}

void Cmd_DebugSetBodyAnim_f(gentity_t *self)
{
	int argNum = trap->Argc();
	char arg[MAX_STRING_CHARS];
	int i = 0;

	if (argNum < 2)
	{
		return;
	}

	trap->Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	while (i < MAX_ANIMATIONS)
	{
		if (!Q_stricmp(arg, animTable[i].name))
		{
			break;
		}
		i++;
	}

	if (i == MAX_ANIMATIONS)
	{
		Com_Printf("Animation '%s' does not exist\n", arg);
		return;
	}

	G_SetAnim(self, NULL, SETANIM_BOTH, i, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);

	Com_Printf("Set body anim to %s\n", arg);
}
#endif

void StandardSetBodyAnim(gentity_t *self, int anim, int flags)
{
	G_SetAnim(self, NULL, SETANIM_BOTH, anim, flags, 0);
}

void DismembermentTest(gentity_t *self);

void Bot_SetForcedMovement(int bot, int forward, int right, int up);

#ifndef FINAL_BUILD
extern void DismembermentByNum(gentity_t *self, int num);
extern void G_SetVehDamageFlags( gentity_t *veh, int shipSurf, int damageLevel );
#endif

qboolean TryGrapple(gentity_t *ent)
{
	if (ent->client->ps.weaponTime > 0)
	{ //weapon busy
		return qfalse;
	}
	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{ //force power or knockdown or something
		return qfalse;
	}
	if (ent->client->grappleState)
	{ //already grappling? but weapontime should be > 0 then..
		return qfalse;
	}

	if (ent->client->ps.weapon != WP_SABER && ent->client->ps.weapon != WP_MELEE)
	{
		return qfalse;
	}

	if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
	{
		Cmd_ToggleSaber_f(ent);
		if (!ent->client->ps.saberHolstered)
		{ //must have saber holstered
			return qfalse;
		}
	}

	//G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_PA_1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
	G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_GRAB, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
	if (ent->client->ps.torsoAnim == BOTH_KYLE_GRAB)
	{ //providing the anim set succeeded..
		ent->client->ps.torsoTimer += 500; //make the hand stick out a little longer than it normally would
		if (ent->client->ps.legsAnim == ent->client->ps.torsoAnim)
		{
			ent->client->ps.legsTimer = ent->client->ps.torsoTimer;
		}
		ent->client->ps.weaponTime = ent->client->ps.torsoTimer;
		return qtrue;
	}

	return qfalse;
}

void Cmd_TargetUse_f( gentity_t *ent )
{
	if ( trap->Argc() > 1 )
	{
		char sArg[MAX_STRING_CHARS] = {0};
		gentity_t *targ;

		trap->Argv( 1, sArg, sizeof( sArg ) );
		targ = G_Find( NULL, FOFS( targetname ), sArg );

		while ( targ )
		{
			if ( targ->use )
				targ->use( targ, ent, ent );
			targ = G_Find( targ, FOFS( targetname ), sArg );
		}
	}
}

void Cmd_TheDestroyer_f( gentity_t *ent ) {
	if ( !ent->client->ps.saberHolstered || ent->client->ps.weapon != WP_SABER )
		return;

	Cmd_ToggleSaber_f( ent );
}

void Cmd_BotMoveForward_f( gentity_t *ent ) {
	int arg = 4000;
	int bCl = 0;
	char sarg[MAX_STRING_CHARS];

	assert( trap->Argc() > 1 );
	trap->Argv( 1, sarg, sizeof( sarg ) );

	assert( sarg[0] );
	bCl = atoi( sarg );
	Bot_SetForcedMovement( bCl, arg, -1, -1 );
}

void Cmd_BotMoveBack_f( gentity_t *ent ) {
	int arg = -4000;
	int bCl = 0;
	char sarg[MAX_STRING_CHARS];

	assert( trap->Argc() > 1 );
	trap->Argv( 1, sarg, sizeof( sarg ) );

	assert( sarg[0] );
	bCl = atoi( sarg );
	Bot_SetForcedMovement( bCl, arg, -1, -1 );
}

void Cmd_BotMoveRight_f( gentity_t *ent ) {
	int arg = 4000;
	int bCl = 0;
	char sarg[MAX_STRING_CHARS];

	assert( trap->Argc() > 1 );
	trap->Argv( 1, sarg, sizeof( sarg ) );

	assert( sarg[0] );
	bCl = atoi( sarg );
	Bot_SetForcedMovement( bCl, -1, arg, -1 );
}

void Cmd_BotMoveLeft_f( gentity_t *ent ) {
	int arg = -4000;
	int bCl = 0;
	char sarg[MAX_STRING_CHARS];

	assert( trap->Argc() > 1 );
	trap->Argv( 1, sarg, sizeof( sarg ) );

	assert( sarg[0] );
	bCl = atoi( sarg );
	Bot_SetForcedMovement( bCl, -1, arg, -1 );
}

void Cmd_BotMoveUp_f( gentity_t *ent ) {
	int arg = 4000;
	int bCl = 0;
	char sarg[MAX_STRING_CHARS];

	assert( trap->Argc() > 1 );
	trap->Argv( 1, sarg, sizeof( sarg ) );

	assert( sarg[0] );
	bCl = atoi( sarg );
	Bot_SetForcedMovement( bCl, -1, -1, arg );
}

extern void SpecialItemThink(gentity_t *ent);

void Cmd_DropItem_f( gentity_t *pent ) {
	// UQ1: Added this command to drop items on a map for the AWP system to pathtest to...
	gentity_t *ent = G_Spawn();
	int wDisable = 0;
#define DISP_HEALTH_ITEM		"item_medpak_instant"
#define TOSSED_ITEM_OWNER_NOTOUCH_DUR	20000
	gitem_t *item = BG_FindItem(DISP_HEALTH_ITEM);

	ent->random = 0;
	ent->wait = 0;

	G_SetOrigin(ent, pent->r.currentOrigin);
	VectorCopy(pent->r.currentOrigin, ent->s.origin);
	VectorCopy(pent->r.currentOrigin, ent->r.currentOrigin);

	ent->s.eType = ET_ITEM;

	RegisterItem( item );

	ent->item = item;
	// some movers spawn on the second frame, so delay item
	// spawns until the third frame so they can ride trains
	ent->nextthink = level.time + FRAMETIME * 2;
	//ent->think = FinishSpawningItem;

	ent->physicsBounce = 0.50;		// items are bouncy

	

	RegisterItem( item );
	ent->item = item;

	//go away if no one wants me
	ent->genericValue5 = level.time + 600000;
	ent->think = SpecialItemThink;
	ent->nextthink = level.time + 50;
	ent->clipmask = MASK_SOLID;

	ent->physicsBounce = 0.50;		// items are bouncy
	VectorSet (ent->r.mins, -8, -8, -0);
	VectorSet (ent->r.maxs, 8, 8, 16);

	ent->s.eType = ET_ITEM;
	ent->s.modelindex = ent->item - bg_itemlist;		// store item number in modelindex

	ent->r.contents = CONTENTS_TRIGGER;
	ent->touch = Touch_Item;

	//can't touch owner for x seconds
	ent->genericValue11 = ent->r.ownerNum;
	ent->genericValue10 = level.time + TOSSED_ITEM_OWNER_NOTOUCH_DUR;

	//so we know to remove when picked up, not respawn
	ent->genericValue9 = 1;

	//kind of a lame value to use, but oh well. This means don't
	//pick up this item clientside with prediction, because we
	//aren't sending over all the data necessary for the player
	//to know if he can.
	ent->s.brokenLimbs = 1;

	//since it uses my server-only physics
	ent->s.eFlags |= EF_CLIENTSMOOTH;
}

void Cmd_AddBot_f( gentity_t *ent ) {
	//because addbot isn't a recognized command unless you're the server, but it is in the menus regardless
	trap->SendServerCommand( ent-g_entities, va( "print \"%s.\n\"", G_GetStringEdString( "MP_SVGAME", "ONLY_ADD_BOTS_AS_SERVER" ) ) );
}

extern qboolean Account_Login(int clientNum, char *user, char *pass, qboolean skipPass, account_t *storeAccount);
void Cmd_Alogin_f(gentity_t *ent)
{
	int clientNum = ent->s.number;
	char		cmd[MAX_TOKEN_CHARS] = { 0 };
	char	pass[MAX_STRING_CHARS]; // passord å logge inn med
	char	user[MAX_STRING_CHARS]; // bruker å logge inn på

	if (trap->Argc() != 3) // Må ha 3 argumenter, /aLogin user pass, alogin teller som 1, altså trenger vi 3
	{
		trap->SendServerCommand(clientNum, "print \"alogin <username> <password>\n\""); // Feilmelding, hvis man ikke gir riktig antall argumenter
		return; // Avslutt kommandoen
	}

	trap->Argv(1, user, sizeof(user)); // username, leses inn i user variablen
	trap->Argv(2, pass, sizeof(pass)); // password, leses inn i pass variablen

	Account_Login(clientNum, user, pass, qfalse, NULL); // Meldinger gjøres i funksjonen.

	return;
}

extern qboolean Account_Register(int clientNum, char *user, char *pass);
void Cmd_Aregister_f(gentity_t *ent)
{
	int clientNum = ent->s.number;
	char		cmd[MAX_TOKEN_CHARS] = { 0 };
	char	pass[MAX_STRING_CHARS]; // passord å logge inn med
	char	confirm[MAX_STRING_CHARS]; // confirm password xP
	char	user[MAX_STRING_CHARS]; // bruker å logge inn på

	if (trap->Argc() != 4) // Må ha 4 argumenter, /aRegister user pass <confirm pass>, aregister teller som 1, altså trenger vi 4
	{
		trap->SendServerCommand(clientNum, "print \"aregister <username> <password> <confirm password>\n\""); // Feilmelding, hvis man ikke gir riktig antall argumenter
		return; // Avslutt kommandoen
	}

	trap->Argv(1, user, sizeof(user)); // username, leses inn i user variablen
	trap->Argv(2, pass, sizeof(pass)); // password, leses inn i pass variablen
	trap->Argv(3, confirm, sizeof(confirm)); // confirmation, leses inn i confirm variablen

	if (Q_stricmp(pass, confirm))
	{
		trap->SendServerCommand(clientNum, "print \"Your password and confirmation password do not match.\n\""); // Feilmelding, hvis man ikke har skrevet riktig passord.
		return;
	}


	if (Account_Register(clientNum, user, pass))
	{
		trap->SendServerCommand(clientNum, "print \"Registration successful.\n\"");
		Account_Login(clientNum, user, pass, qfalse, NULL);	// Vi logger oss inn også =)
	}
	else trap->SendServerCommand(clientNum, "print \"Registration failed.\n\"");

	return;
}



void Cmd_Acheckaccount_f(gentity_t *ent)
{
	int clientNum = ent->s.number;
	char		cmd[MAX_TOKEN_CHARS] = { 0 };
	account_t account;
	char	user[MAX_STRING_CHARS];
	
	if (trap->Argc() != 2)
	{
		trap->SendServerCommand(clientNum, "print \"acheckaccount <username> <password> <confirm password>\n\""); // Feilmelding, hvis man ikke gir riktig antall argumenter

		return; // Avslutt kommandoen
	}

	trap->Argv(1, user, sizeof(user)); // username, leses inn i user variablen

	Account_Login(clientNum, user, NULL, qtrue, &account);

	trap->SendServerCommand(clientNum, va("print \"Username: %s\nPassword: %s\nPermissions: %i\n\"", account.username, account.password, account.permissions));

}

//[Create Dungeon]
void Cmd_Clear_Dungeon_F(gentity_t *ent){

	Clear_Dungeon();
}
void Cmd_Save_Dungeon_F(gentity_t *ent){

	Save_Dungeon();
}
void Cmd_Load_Dungeon_F(gentity_t *ent){

	Load_Dungeon();
}
//[/Create Dungeon]

/*
=================
ClientCommand
=================
*/

#define CMD_NOINTERMISSION		(1<<0)
#define CMD_CHEAT				(1<<1)
#define CMD_ALIVE				(1<<2)

typedef struct command_s {
	const char	*name;
	void		(*func)(gentity_t *ent);
	int			flags;
} command_t;

int cmdcmp( const void *a, const void *b ) {
	return Q_stricmp( (const char *)a, ((command_t*)b)->name );
}

/* This array MUST be sorted correctly by alphabetical name field */
command_t commands[] = {
	{ "addbot",				Cmd_AddBot_f,				0 },
	{ "d_clear",			Cmd_Clear_Dungeon_F,		CMD_CHEAT | CMD_ALIVE },//[Create Dungeon]
	{ "d_save",				Cmd_Save_Dungeon_F,			CMD_CHEAT | CMD_ALIVE },
	{ "d_load",				Cmd_Load_Dungeon_F,			CMD_CHEAT | CMD_ALIVE },//[/Create Dungeon]
	{ "alogin",				Cmd_Alogin_f,			    0 },
	{ "aregister",			Cmd_Aregister_f,		    0 },
	{ "acheckaccount",		Cmd_Acheckaccount_f,		0 },
	{ "callteamvote",		Cmd_CallTeamVote_f,			CMD_NOINTERMISSION },
	{ "callvote",			Cmd_CallVote_f,				CMD_NOINTERMISSION },
	{ "debugBMove_Back",	Cmd_BotMoveBack_f,			CMD_CHEAT|CMD_ALIVE },
	{ "debugBMove_Forward",	Cmd_BotMoveForward_f,		CMD_CHEAT|CMD_ALIVE },
	{ "debugBMove_Left",	Cmd_BotMoveLeft_f,			CMD_CHEAT|CMD_ALIVE },
	{ "debugBMove_Right",	Cmd_BotMoveRight_f,			CMD_CHEAT|CMD_ALIVE },
	{ "debugBMove_Up",		Cmd_BotMoveUp_f,			CMD_CHEAT|CMD_ALIVE },
	{ "dropitem",			Cmd_DropItem_f,				CMD_CHEAT|CMD_ALIVE },
	{ "duelteam",			Cmd_DuelTeam_f,				CMD_NOINTERMISSION },
	{ "follow",				Cmd_Follow_f,				CMD_NOINTERMISSION },
	{ "follownext",			Cmd_FollowNext_f,			CMD_NOINTERMISSION },
	{ "followprev",			Cmd_FollowPrev_f,			CMD_NOINTERMISSION },
	{ "forcechanged",		Cmd_ForceChanged_f,			0 },
	{ "gc",					Cmd_GameCommand_f,			CMD_NOINTERMISSION },
	{ "give",				Cmd_Give_f,					CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "giveother",			Cmd_GiveOther_f,			CMD_CHEAT|CMD_NOINTERMISSION },
	{ "god",				Cmd_God_f,					CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "kill",				Cmd_Kill_f,					CMD_ALIVE|CMD_NOINTERMISSION },
	{ "killother",			Cmd_KillOther_f,			CMD_CHEAT|CMD_NOINTERMISSION },
//	{ "kylesmash",			TryGrapple,					0 },
	{ "levelshot",			Cmd_LevelShot_f,			CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "maplist",			Cmd_MapList_f,				CMD_NOINTERMISSION },
	{ "noclip",				Cmd_Noclip_f,				CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "notarget",			Cmd_Notarget_f,				CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "npc",				Cmd_NPC_f,					CMD_CHEAT|CMD_ALIVE },
	{ "say",				Cmd_Say_f,					0 },
	{ "say_team",			Cmd_SayTeam_f,				0 },
	{ "score",				Cmd_Score_f,				0 },
	{ "setviewpos",			Cmd_SetViewpos_f,			CMD_CHEAT|CMD_NOINTERMISSION },
	{ "siegeclass",			Cmd_SiegeClass_f,			CMD_NOINTERMISSION },
	{ "team",				Cmd_Team_f,					CMD_NOINTERMISSION },
//	{ "teamtask",			Cmd_TeamTask_f,				CMD_NOINTERMISSION },
	{ "teamvote",			Cmd_TeamVote_f,				CMD_NOINTERMISSION },
	{ "tell",				Cmd_Tell_f,					0 },
	{ "thedestroyer",		Cmd_TheDestroyer_f,			CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "t_use",				Cmd_TargetUse_f,			CMD_CHEAT|CMD_ALIVE },
	{ "voice_cmd",			Cmd_VoiceCommand_f,			CMD_NOINTERMISSION },
	{ "vote",				Cmd_Vote_f,					CMD_NOINTERMISSION },
	{ "where",				Cmd_Where_f,				CMD_NOINTERMISSION },

};

//[Create Dungeon]
//the max autosave file size define
#define MAX_AUTOSAVE_FILESIZE 1024
// We will be using Dungeons/<mapname>.DungeonsDataFiles for our save files
// I guess we can call them missions? ofc :P
void Save_Dungeon()
{
	fileHandle_t	f;
	char			lineBuf[MAX_QPATH];
	char			fileBuf[MAX_AUTOSAVE_FILESIZE];
	char			loadPath[MAX_QPATH];
	int				len;
	gentity_t*		npc;

	fileBuf[0] = '\0';

	trap->Print("^5Saving Dungeons File Data...");

	//Com_sprintf(loadPath, MAX_QPATH, "maps/%s.autosp", mapname.string);
	Com_sprintf(loadPath, sizeof(loadPath), "dungeons/%s.DungeonsDataFiles", level.dungeonmapfiles);

	len = trap->FS_Open(loadPath, &f, FS_WRITE);
	if (!f)
	{
		trap->Print("^5Couldn't create Dungeon Data File.\n");
		return;
	}

	//newent->targetname = ent->NPC_targetname;
	//newent->classname = "NPC";
	//newent->NPC_type = ent->NPC_type;

	//find all npc's
	npc = &g_entities[0];
	for (; npc - g_entities < MAX_GENTITIES; npc++)
	{
		if (!npc->inuse)
			continue;
		if (npc->NPC || npc->s.eType == ET_NPC)
		{
			//if(!Q_stricmp("vehicle", npc->NPC_type))
			//	isVehicle = 1;


			Com_sprintf(lineBuf, MAX_QPATH, "%f %f %f %i %s\n",
				npc->r.currentOrigin[0], npc->r.currentOrigin[1],
				npc->r.currentOrigin[2], npc->NPC->isVehicle, npc->NPC_type);
			strcat(fileBuf, lineBuf);
		}
	}

	if (fileBuf[0] != '\0')
	{//actually written something
		trap->FS_Write(fileBuf, strlen(fileBuf), f);
	}
	trap->FS_Close(f);
	trap->Print("^5Done.\n");

}

gentity_t *NPC_SpawnType(gentity_t *ent, char *npc_type, char *targetname, qboolean isVehicle, qboolean load, vec3_t positionData);
void Load_Dungeon()
{
	char			*s;
	int				len;
	fileHandle_t	f;
	char			buf[MAX_AUTOSAVE_FILESIZE];
	char			loadPath[MAX_QPATH];
	char			targetname[MAX_INFO_STRING];
	vec3_t			positionData;
	int				isVehicle;
	char			*npc_type;

	trap->Print("^5Loading Dungeon File Data...");

	//Com_sprintf(loadPath, MAX_QPATH, "maps/%s.autosp", mapname.string);
	Com_sprintf(loadPath, sizeof(loadPath), "dungeons/%s.DungeonsDataFiles", level.dungeonmapfiles);
	//[/RawMapName]

	len = trap->FS_Open(loadPath, &f, FS_READ);
	if (!f)
	{
		trap->Print("^5No Dungeon Data File Found.\n");
		return;
	}
	if (!len)
	{ //empty file
		trap->Print("^5Empty Dungeon file!\n");
		trap->FS_Close(f);
		return;
	}

	//	Mission_Clear(); // Prepare map for loading, by removing all current npcs.
	//NPC_SpawnType


	trap->FS_Read(buf, len, f);
	trap->FS_Close(f);

	s = buf;

	while (*s != '\0' && s - buf < len)
	{
		if (*s == '\n')
		{//hop over newlines
			s++;
			continue;
		}

		sscanf(s, "%f %f %f %i %s", &positionData[0], &positionData[1], &positionData[2], &isVehicle, targetname);

		//Create_Autosave( positionData, sizeData, teleportPlayers );
		//	if(isVehicle)
		//	{
		//		npc_type = "vehicle";
		//	}else 
		npc_type = targetname;

		NPC_SpawnType(NULL, npc_type, targetname, isVehicle, qtrue, positionData);

		//advance to the end of the linehead
		while (*s != '\n' && *s != '\0' && s - buf < len)
		{
			s++;
		}
	}

	trap->Print("^5Done.\n");

}

void Clear_Dungeon()
{
	gentity_t *npc = &g_entities[0];
	// This function/cmd, will remove all npcs on the map and all npc spawners, so that the npcs won't come back.

	for (; npc - g_entities < MAX_GENTITIES; npc++)
	{
		if (npc->inuse)
		{
			if (npc->NPC || npc->s.eType == ET_NPC)
			{
				npc->health = 0;
				npc->client->ps.stats[STAT_HEALTH] = 0;
				if (npc->die)
				{
					npc->die(npc, npc, npc, 100, MOD_UNKNOWN);
				}
			}
			else if (npc->NPC_type && npc->classname && npc->classname[0] && Q_stricmp("NPC_starfleet", npc->classname) != 0)
			{//A spawner, remove it
				//	Com_Printf( S_COLOR_GREEN"Removing NPC spawner %s with NPC named %s\n", player->NPC_type, player->NPC_targetname );
				G_FreeEntity(npc);
				//FIXME: G_UseTargets2(player, player, player->NPC_target & player->target);?
			}
		}
	}
}
//[/Create Dungeon]

// Account system
// Returns true if login successful,
// returns false if login failed.
qboolean Account_Login(int clientNum, char *user, char *pass, qboolean skipPass, account_t *storeAccount)
{
	gentity_t *ent = &g_entities[clientNum];	// Henter client entity.
	qhandle_t fh;
	char *fileData;
	int *p;
	int length;
	qboolean result = qfalse;
	account_t *account;

	if (!ent || !ent->inuse)		// Sjekker om ent er NULL, eller om entity'n ikke er i bruk
		return qfalse;			// Login failed


	// Åpner fil som ligger i accounts folder. Filen skal ha navnet til kontoen/account
	length = trap->FS_Open(va("accounts/%s.acc", user), &fh, FS_READ);

	if (!fh || length <= 0)		// Noe feil skjedde med filen, fant ikke brukeren
	{
		trap->SendServerCommand(ent - g_entities, va("print \"Couldn't find user: %s\n\"", user));
		trap->FS_Close(fh);
		return qfalse;
	}

	if (length != (sizeof(account_t)+sizeof(int)))		// To prevent people from creating buffer overflow exploits with edited account files.
	{
		trap->SendServerCommand(ent - g_entities, va("print \"Corrupt save file.\n\""));
		trap->FS_Close(fh);
		return qfalse;
	}

	// Allokerer minne for å lese inn filen
	fileData = (char*)dlmalloc(length);

	trap->FS_Read(fileData, length, fh); // Leser inn fra fil.
	p = (int*)fileData;					// pointer får å jobbe på dataen.

	if (*p != ACCOUNT_VERSION)			// Check save version. Currently no backwards compatability.
	{
		trap->SendServerCommand(ent - g_entities, va("print \"Savefile is version %i, needs to be version %i.\n\"", *p, ACCOUNT_VERSION));
		dlfree(fileData);
		trap->FS_Close(fh);
		return qfalse;
	}
	p++;
	account = (account_t*)p;

	if (skipPass || !Q_stricmp(account->password, pass))				// bare tester, vi regner med at passordet står først i filen.
	{
		// Her må vi legge inn mer for å gjøre kontoen brukbar
		//return qtrue;					// Passordet er riktig, login successful.
		result = qtrue;
		ent->account = *account;			// Apply the account information.
	}
	else
	{
		result = qfalse;				// Passorder er feil, login failed.
		memset(&ent->account, 0, sizeof(account_t));	// Clear the account information, just in case.
	}

	if (storeAccount)					// We are using the login function to read the account information into an additional storage container.
		*storeAccount = *account;

	dlfree(fileData);						// Frigi minne, vi vil ikke ha memory leaks
	trap->FS_Close(fh);				// Husk å lukk filer. Ellers vil spillet crashe, når man har åpnet 64 filer.

	if (result)
	{
		// We have to load the experience manually into the PERS_EXPERIENCE. Hopefully you won't need to do this with anything else
		// The playerclass is loaded automaticly, and with the changes to the code we made, it will be used correctly.
		ent->client->ps.stats[STAT_EXP] = ent->account.experience;
		ent->client->ps.stats[STAT_EXP_COUNT] = experienceLevel[ent->account.level];	// Set the required experience for next level.
		trap->SendServerCommand(ent - g_entities, va("maxexperience %i", ent->client->ps.stats[STAT_EXP_COUNT]));
		trap->SendServerCommand(clientNum, va("print \"Welcome %s, login successful.\n\"", user));
	}
	else trap->SendServerCommand(clientNum, "print \"Login failed.\n\"");

	return result;						// Returner resultat
}

// Lage en konto
qboolean Account_Register(int clientNum, char *user, char *pass)
{
	gentity_t *ent = &g_entities[clientNum];	// Henter client entity.
	qhandle_t fh;
	//char fileData[2048] = { 0 };				// Maks filstørrelse.
	char *fileData;
	int *p;
	//int length;

	if (!ent || !ent->inuse)		// Sjekker om ent er NULL, eller om entity'n ikke er i bruk
		return qfalse;			// Login failed	

	fileData = (char*)dlmalloc(sizeof(account_t)+sizeof(int));
	p = (int*)fileData;
	*p = ACCOUNT_VERSION; // Version number for the save file. When you change the system, you will need to change this value.
	p++;


	// Åpner fil som ligger i accounts folder. Filen skal ha navnet til kontoen/account
	trap->FS_Open(va("accounts/%s.acc", user), &fh, FS_WRITE); // Vis skal skrive denne gangen.

	if (!fh)			// Noe feil skjedde med filen, fant ikke brukeren
	{
		dlfree(fileData);
		trap->SendServerCommand(ent - g_entities, va("print \"Couldn't create user: %s\n\"", user));
		//	trap_FS_FCloseFile(fh);
		return qfalse;
	}


	// The initial setup of the account is done on register 
	// So here we fill in basic information
	Q_strncpyz(ent->account.username, user, 64);	// Account username
	Q_strncpyz(ent->account.password, pass, 64);	// Account password	
	ent->account.permissions = 1337;				// Account permissions, 32 bit, bitmask Currently testing with setting to 1337, should be set to 0 when not testing
	ent->account.playerclass = 0;
	//ent->account.playerclasses = 0;

	//	ent->account.playerclass, user;  <<-- wth is this o.o? lol was a try xD but i see it is 0 :P not good 
	//
	// When you add more to the account_t structure, you need to add their default values here:
	// ent->account.<new value> = <default value>;

	// And of course change the version number each time you add a new variable to the account_t structure.
	// (At the moment this will 'break' all old accounts, we can add compatability next time)

	memcpy(p, &ent->account, sizeof(account_t));

	//trap_FS_Write(fileData, length, fh);	// Skriv informasjon til fil.
	trap->FS_Write(fileData, sizeof(account_t)+sizeof(int), fh);
	dlfree(fileData); // Free memory to avoid memory leak

	trap->FS_Close(fh);
	return qtrue;
}

qboolean hasAccount(account_t *account)
{
	if (account->username[0] != 0)
		return qtrue;
	else return qfalse;
}

// Every time you change anything on an account, that has to be saved right away. Just do UpdateAccount(&ent->account);
// It will return qtrue/1 if it succeeds, and qfalse/0 if it fails.
qboolean UpdateAccount(account_t *account, gentity_t *ent)
{
	char *fileData;
	int *p;
	fileHandle_t fh;
	account_t *p_account;

	fileData = (char*)dlmalloc(sizeof(account_t)+sizeof(int));
	p = (int*)fileData;
	*p = ACCOUNT_VERSION; // Version number for the save file. When you change the system, you will need to change this value.
	p++;

	if (!hasAccount(account))
	{
		dlfree(fileData);
		return qfalse;
	}

	trap->FS_Open(va("accounts/%s.acc", account->username), &fh, FS_WRITE);

	if (!fh)
	{
		dlfree(fileData);
		trap->FS_Close(fh);
		return qfalse;
	}
	memcpy(p, account, sizeof(account_t));
	p_account = (account_t*)p;

	// Add changes from active system.
	p_account->experience = ent->client->ps.stats[STAT_EXP];


	trap->FS_Write(fileData, sizeof(account_t)+sizeof(int), fh);
	dlfree(fileData); // Free memory to avoid memory leak
	trap->FS_Close(fh);

	return qtrue;
}

static const size_t numCommands = ARRAY_LEN( commands );

void ClientCommand( int clientNum ) {
	gentity_t	*ent = NULL;
	char		cmd[MAX_TOKEN_CHARS] = {0};
	command_t	*command = NULL;

	ent = g_entities + clientNum;
	if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
		G_SecurityLogPrintf( "ClientCommand(%d) without an active connection\n", clientNum );
		return;		// not fully in game yet
	}

	trap->Argv( 0, cmd, sizeof( cmd ) );

	command = (command_t *)bsearch( cmd, commands, numCommands, sizeof( commands[0] ), cmdcmp );
	if ( !command )
	{
		trap->SendServerCommand( clientNum, va( "print \"Unknown command %s\n\"", cmd ) );
		return;
	}

	else if ( (command->flags & CMD_NOINTERMISSION)
		&& ( level.intermissionQueued || level.intermissiontime ) )
	{
		trap->SendServerCommand( clientNum, va( "print \"%s (%s)\n\"", G_GetStringEdString( "MP_SVGAME", "CANNOT_TASK_INTERMISSION" ), cmd ) );
		return;
	}

	else if ( (command->flags & CMD_CHEAT)
		&& !sv_cheats.integer )
	{
		trap->SendServerCommand( clientNum, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "NOCHEATS" ) ) );
		return;
	}

	else if ( (command->flags & CMD_ALIVE)
		&& (ent->health <= 0
			|| ent->client->tempSpectate >= level.time
			|| ent->client->sess.sessionTeam == TEAM_SPECTATOR) )
	{
		trap->SendServerCommand( clientNum, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "MUSTBEALIVE" ) ) );
		return;
	}

	else
		command->func( ent );
}