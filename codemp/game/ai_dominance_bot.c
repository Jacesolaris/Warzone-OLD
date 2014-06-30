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


void DOM_InitFakeNPC(gentity_t *bot)
{
	bot->NPC = New_NPC_t(bot->s.number);

	//Assign the pointer for bg entity access
	bot->playerState = &bot->client->ps;

	//bot->NPC_type = G_NewString("reborn");
	bot->NPC_type = G_NewString(bot->client->pers.netname);

	//bot->flags |= FL_NO_KNOCKBACK;//don't fall off ledges

	bot->client->ps.weapon = WP_SABER;//init for later check in NPC_Begin

	NPC_ParseParms(bot->NPC_type, bot);

	NPC_DefaultScriptFlags(bot);

	bot->client->NPC_class = CLASS_BOT_FAKE_NPC;
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

	bot->client->NPC_class = CLASS_BOT_FAKE_NPC;

	bot->client->playerTeam = NPCTEAM_ENEMY;
}

extern void BotChangeViewAngles(bot_state_t *bs, float thinktime);

void DOM_FakeNPC_Parse_UCMD (bot_state_t *bs, gentity_t *bot)
{
	NPCS.NPC = bot;
	NPCS.client = NPCS.NPC->client;
	NPCS.NPCInfo = bot->NPC;
	NPCS.ucmd = NPCS.NPC->client->pers.cmd;

	// Set angles... Convert to ideal view angles then run the bot code...
	VectorSet(bs->ideal_viewangles, SHORT2ANGLE(NPCS.ucmd.angles[PITCH] + NPCS.client->ps.delta_angles[PITCH]), SHORT2ANGLE(NPCS.ucmd.angles[YAW] + NPCS.client->ps.delta_angles[YAW]), SHORT2ANGLE(NPCS.ucmd.angles[ROLL] + NPCS.client->ps.delta_angles[ROLL]));
	trap->EA_View(bs->client, bs->ideal_viewangles);

	if (NPCS.ucmd.upmove > 0)
		trap->EA_Jump(bs->client);

	if (NPCS.ucmd.upmove < 0)
		trap->EA_Crouch(bs->client);
	
	if (NPCS.ucmd.rightmove > 0)
		trap->EA_MoveRight(bs->client);

	if (NPCS.ucmd.rightmove < 0)
		trap->EA_MoveLeft(bs->client);

	if (NPCS.ucmd.forwardmove > 0)
		trap->EA_MoveForward(bs->client);

	if (NPCS.ucmd.forwardmove < 0)
		trap->EA_MoveBack(bs->client);

	if (NPCS.ucmd.buttons & BUTTON_ATTACK)
		trap->EA_Attack(bs->client);

	if (NPCS.ucmd.buttons & BUTTON_ALT_ATTACK)
		trap->EA_Alt_Attack(bs->client);

	if (NPCS.ucmd.buttons & BUTTON_USE)
		trap->EA_Use(bs->client);
}

vec3_t oldMoveDir;

// UQ1: Now lets see if bots can share NPC AI....
void DOM_StandardBotAI2(bot_state_t *bs, float thinktime)
{
	gentity_t *bot = &g_entities[bs->client];

	NPCS.NPC = bot;
	NPCS.client = NPCS.NPC->client;
	NPCS.NPCInfo = bot->NPC;
	NPCS.ucmd = NPCS.NPC->client->pers.cmd;

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
		}

		return;
	}

	VectorCopy(oldMoveDir, bot->client->ps.moveDir);
	//or use client->pers.lastCommand?

	NPCS.NPCInfo->last_ucmd.serverTime = level.time - 50;

	//nextthink is set before this so something in here can override it
	NPC_ExecuteBState(bot);

	NPC_Think(bot); // test

	NPC_UpdateAngles(qtrue, qtrue);
	//memcpy(&NPCS.ucmd, &NPCS.NPCInfo->last_ucmd, sizeof(usercmd_t));
	DOM_FakeNPC_Parse_UCMD(bs, bot);
	ClientThink(bot->s.number, &NPCS.ucmd);

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
