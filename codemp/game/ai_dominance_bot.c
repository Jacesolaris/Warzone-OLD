//includes
#include "ai_main.h"

#ifdef __DOMINANCE_AI__

#include "qcommon/q_shared.h"
#include "g_public.h"
#include "b_local.h"

extern void QDECL BotAI_Print(int type, char *fmt, ...);

//extern gentity_t *NPC;
//Local Variables
extern npcStatic_t NPCS;

//externs
extern bot_state_t	*botstates[MAX_CLIENTS];
//Local Variables
//extern gentity_t		*NPC;
//extern gNPC_t			*NPCInfo;
//extern gclient_t		*client;
//extern usercmd_t		ucmd;
extern visibility_t		enemyVisibility;

extern gNPC_t *New_NPC_t(int entNum);
extern qboolean NPC_ParseParms(const char *NPCName, gentity_t *NPC);
extern void NPC_DefaultScriptFlags(gentity_t *ent);
extern void NPC_Think(gentity_t *self);
extern void GLua_NPCEV_OnThink(gentity_t *self);
extern qboolean NPC_UpdateAngles(qboolean doPitch, qboolean doYaw);
extern void NPC_Begin(gentity_t *ent);
extern void NPC_ExecuteBState(gentity_t *self);
extern void SetNPCGlobals(gentity_t *ent);
extern void NPC_Precache ( gentity_t *spawner );

#define __FAKE_NPC_LIGHTNING_SPAM__

void DOM_InitFakeNPC(gentity_t *bot)
{
	int i = 0;

	bot->NPC = New_NPC_t(bot->s.number);

	//Assign the pointer for bg entity access
	bot->playerState = &bot->client->ps;

	//bot->NPC_type = G_NewString("reborn");
	bot->NPC_type = G_NewString(bot->client->pers.netname);

	// Convert the spaces in the bot name to _ to match npc names...
	for (i = 0; i < strlen(bot->NPC_type); i++)
	{
		if (bot->NPC_type[i] == ' ') 
			bot->NPC_type[i] = '_';
	}

	if (!(bot->s.eFlags & EF_FAKE_NPC_BOT))
		bot->s.eFlags |= EF_FAKE_NPC_BOT;
	if (!(bot->client->ps.eFlags & EF_FAKE_NPC_BOT))
		bot->client->ps.eFlags |= EF_FAKE_NPC_BOT;

	//bot->flags |= FL_NO_KNOCKBACK;//don't fall off ledges

	bot->client->ps.weapon = WP_SABER;//init for later check in NPC_Begin

	NPC_ParseParms(bot->NPC_type, bot);

	NPC_DefaultScriptFlags(bot);

	NPC_Begin(bot);
	bot->s.eType = ET_PLAYER; // Replace ET_NPC

	// UQ1: Mark every NPC's spawn position. For patrolling that spot and stuff...
	VectorCopy(bot->r.currentOrigin, bot->spawn_pos);
	VectorCopy(bot->r.currentOrigin, bot->spawn_pos);

	// Init patrol range...
	if (bot->patrol_range <= 0) bot->patrol_range = 512.0f;

	// Init waypoints...
	bot->wpCurrent = -1;
	bot->wpNext = -1;
	bot->wpLast = -1;
	bot->longTermGoal = -1;

	// Init enemy...
	bot->enemy = NULL;

	bot->client->playerTeam = NPCTEAM_ENEMY;

	if (!(bot->s.eFlags & EF_FAKE_NPC_BOT))
		bot->s.eFlags |= EF_FAKE_NPC_BOT;
	if (!(bot->client->ps.eFlags & EF_FAKE_NPC_BOT))
		bot->client->ps.eFlags |= EF_FAKE_NPC_BOT;

#ifdef __FAKE_NPC_LIGHTNING_SPAM__
	bot->client->ps.fd.forcePowersKnown |= (1 << FP_LIGHTNING);
	bot->client->ps.fd.forcePowerLevel[FP_LIGHTNING] = FORCE_LEVEL_3;
#endif //__FAKE_NPC_LIGHTNING_SPAM__

	bot->client->ps.fd.forcePowersKnown |= (1 << FP_DRAIN);
	bot->client->ps.fd.forcePowerLevel[FP_DRAIN] = FORCE_LEVEL_3;
}

extern void BotChangeViewAngles(bot_state_t *bs, float thinktime);

//action flags
#define ACTION_ATTACK			0x0000001
#define ACTION_USE				0x0000002
#define ACTION_RESPAWN			0x0000008
#define ACTION_JUMP				0x0000010
#define ACTION_MOVEUP			0x0000020
#define ACTION_CROUCH			0x0000080
#define ACTION_MOVEDOWN			0x0000100
#define ACTION_MOVEFORWARD		0x0000200
#define ACTION_MOVEBACK			0x0000800
#define ACTION_MOVELEFT			0x0001000
#define ACTION_MOVERIGHT		0x0002000
#define ACTION_DELAYEDJUMP		0x0008000
#define ACTION_TALK				0x0010000
#define ACTION_GESTURE			0x0020000
#define ACTION_WALK				0x0080000
#define ACTION_FORCEPOWER		0x0100000
#define ACTION_ALT_ATTACK		0x0200000
/*
#define ACTION_AFFIRMATIVE		0x0100000
#define ACTION_NEGATIVE			0x0200000
#define ACTION_GETFLAG			0x0800000
#define ACTION_GUARDBASE		0x1000000
#define ACTION_PATROL			0x2000000
#define ACTION_FOLLOWME			0x8000000
*/

qboolean DOM_FakeNPC_Parse_UCMD (bot_state_t *bs, gentity_t *bot)
{
	qboolean acted = qfalse;

	NPCS.NPC = bot;
	NPCS.client = NPCS.NPC->client;
	NPCS.NPCInfo = bot->NPC;
	NPCS.ucmd = NPCS.NPC->client->pers.cmd;

	// Set angles... Convert to ideal view angles then run the bot code...
	VectorSet(bs->ideal_viewangles, SHORT2ANGLE(NPCS.ucmd.angles[PITCH] + NPCS.client->ps.delta_angles[PITCH]), SHORT2ANGLE(NPCS.ucmd.angles[YAW] + NPCS.client->ps.delta_angles[YAW]), SHORT2ANGLE(NPCS.ucmd.angles[ROLL] + NPCS.client->ps.delta_angles[ROLL]));
	trap->EA_View(bs->client, bs->ideal_viewangles);

	if (NPCS.ucmd.upmove > 0)
	{
		trap->EA_Jump(bs->client);
		acted = qtrue;
	}

	if (NPCS.ucmd.upmove < 0)
	{
		trap->EA_Crouch(bs->client);
		acted = qtrue;
	}
	
	if (NPCS.ucmd.rightmove > 0)
	{
		trap->EA_MoveRight(bs->client);
		acted = qtrue;
	}

	if (NPCS.ucmd.rightmove < 0)
	{
		trap->EA_MoveLeft(bs->client);
		acted = qtrue;
	}

	if (NPCS.ucmd.forwardmove > 0)
	{
		trap->EA_MoveForward(bs->client);
		acted = qtrue;
	}

	if (NPCS.ucmd.forwardmove < 0)
	{
		trap->EA_MoveBack(bs->client);
		acted = qtrue;
	}

	if (NPCS.ucmd.buttons & BUTTON_ATTACK)
	{
		/*
		if (bot->client->ps.weapon == WP_SABER && NPCS.ucmd.rightmove == 0)
		{
			trap->EA_MoveLeft(bs->client); // UQ1: Also move left for a better animation then the plain one...

			if (!(NPCS.ucmd.buttons & BUTTON_WALKING))
				trap->EA_Action(bs->client, ACTION_WALK);
		}
		*/

		trap->EA_Attack(bs->client);
		acted = qtrue;
	}

	if (NPCS.ucmd.buttons & BUTTON_ALT_ATTACK)
	{
		trap->EA_Alt_Attack(bs->client);
		acted = qtrue;
	}

	if (NPCS.ucmd.buttons & BUTTON_USE)
	{
		trap->EA_Use(bs->client);
		acted = qtrue;
	}

	if (NPCS.ucmd.buttons & BUTTON_WALKING)
	{
		trap->EA_Action(bs->client, ACTION_WALK);
		acted = qtrue;
	}

	return acted;
}

vec3_t oldMoveDir;
extern void ClientThink_real( gentity_t *ent );
extern void NPC_ApplyRoff (void);

// UQ1: Now lets see if bots can share NPC AI....
void DOM_StandardBotAI2(bot_state_t *bs, float thinktime)
{
	gentity_t *bot = &g_entities[bs->client];

	NPCS.NPC = bot;
	NPCS.client = NPCS.NPC->client;
	NPCS.NPCInfo = bot->NPC;
	NPCS.ucmd = NPCS.NPC->client->pers.cmd;

	if (!(bot->s.eFlags & EF_FAKE_NPC_BOT))
		bot->s.eFlags |= EF_FAKE_NPC_BOT;
	if (!(bot->client->ps.eFlags & EF_FAKE_NPC_BOT))
		bot->client->ps.eFlags |= EF_FAKE_NPC_BOT;

	if (!bot->NPC)
		DOM_InitFakeNPC(bot);

	SetNPCGlobals(bot);

	memset(&NPCS.ucmd, 0, sizeof(NPCS.ucmd));

	if (bot->health < 1 || bot->client->ps.pm_type == PM_DEAD)
	{
		//RACC - Try to respawn if you're done talking.
		if (rand() % 10 < 5 &&
			(!bs->doChat || bs->chatTime < level.time))
		{
			trap->EA_Attack(bs->client);

			NPCS.NPC->enemy = NPCS.NPCInfo->goalEntity = NULL; // Clear enemy???
		}

		return;
	}

	VectorCopy(oldMoveDir, bot->client->ps.moveDir);
	//or use client->pers.lastCommand?

	NPCS.NPCInfo->last_ucmd.serverTime = level.time - 50;

	//nextthink is set before this so something in here can override it
	NPC_ExecuteBState(bot);

	if (bot->enemy)
		NPC_FaceEnemy( qtrue );

	NPC_UpdateAngles(qtrue, qtrue);

	//G_UpdateClientAnims(bot, 0.5f);
	if (!DOM_FakeNPC_Parse_UCMD(bs, bot))
	{
		// Failed to do anything this frame - fall back to standard AI...
		//DOM_StandardBotAI(bs, thinktime); // UQ1: Uses Dominance AI...
		StandardBotAI(bs, thinktime);
		trap->ICARUS_MaintainTaskManager(bot->s.number);
		return;
	}

	trap->ICARUS_MaintainTaskManager(bot->s.number);
	VectorCopy(bot->r.currentOrigin, bot->client->ps.origin);

	if (bot->client->ps.pm_flags & PMF_DUCKED && bot->r.maxs[2] > bot->client->ps.crouchheight)
	{
		bot->r.maxs[2] = bot->client->ps.crouchheight;
		bot->r.maxs[1] = 8;
		bot->r.maxs[0] = 8;
		bot->r.mins[1] = -8;
		bot->r.mins[0] = -8;
		trap->LinkEntity((sharedEntity_t *)bot);
	}
	else if (!(bot->client->ps.pm_flags & PMF_DUCKED) && bot->r.maxs[2] < bot->client->ps.standheight)
	{
		bot->r.maxs[2] = bot->client->ps.standheight;
		bot->r.maxs[1] = 10;
		bot->r.maxs[0] = 10;
		bot->r.mins[1] = -10;
		bot->r.mins[0] = -10;
		trap->LinkEntity((sharedEntity_t *)bot);
	}

	//trap->Print(S_COLOR_RED "Bot [%s] is using NPC AI.\n", bot->client->pers.netname);
}

#endif //__DOMINANCE_AI__
