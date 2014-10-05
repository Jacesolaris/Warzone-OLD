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
extern char *G_ValidateUserinfo( const char *userinfo );

extern void Load_NPC_Names ( void );
extern void SelectNPCNameFromList( gentity_t *NPC );
extern char *Get_NPC_Name ( int NAME_ID );

//#define __FAKE_NPC_LIGHTNING_SPAM__

void DOM_SetFakeNPCName(gentity_t *ent)
{// UQ1: Find their name type to send to an id to the client for names...
	if (ent->s.NPC_NAME_ID > 0) return;

	// Load names on first check...
	Load_NPC_Names();

	switch( ent->s.NPC_class )
	{
	case CLASS_STORMTROOPER:
		ent->s.NPC_NAME_ID = irand(100, 999);
		strcpy(ent->client->pers.netname, va("TK-%i", ent->s.NPC_NAME_ID));
		break;
	case CLASS_SWAMPTROOPER:
		ent->s.NPC_NAME_ID = irand(100, 999);
		strcpy(ent->client->pers.netname, va("TS-%i", ent->s.NPC_NAME_ID));
		break;
	case CLASS_IMPWORKER:
		ent->s.NPC_NAME_ID = irand(100, 999);
		strcpy(ent->client->pers.netname, va("IW-%i", ent->s.NPC_NAME_ID));
		break;
	case CLASS_SHADOWTROOPER:
		ent->s.NPC_NAME_ID = irand(100, 999);
		strcpy(ent->client->pers.netname, va("ST-%i", ent->s.NPC_NAME_ID));
		break;
	default:
		SelectNPCNameFromList(ent);
		strcpy(ent->client->pers.netname, Get_NPC_Name(ent->s.NPC_NAME_ID));
		break;
	}

	{
		char *s, userinfo[MAX_INFO_STRING];

		trap->GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );

		// check for malformed or illegal info strings
		s = G_ValidateUserinfo( userinfo );
		if ( s && *s ) {
			return;
		}

		Info_SetValueForKey( userinfo, "name", ent->client->pers.netname );

		//trap->Print("NPC %i given name %s.\n", ent->s.number, ent->client->pers.netname);

		ClientUserinfoChanged( ent->s.number );
	}
}

void DOM_InitFakeNPC(gentity_t *bot)
{
	int i = 0;

	bot->NPC = New_NPC_t(bot->s.number);

	//Assign the pointer for bg entity access
	bot->playerState = &bot->client->ps;

	//bot->NPC_type = G_NewString("reborn");
	bot->NPC_type = Q_strlwr( G_NewString(bot->client->pers.netname) );

	// Convert the spaces in the bot name to _ to match npc names...
	for (i = 0; i < strlen(bot->NPC_type); i++)
	{
		if (bot->NPC_type[i] == ' ') 
			bot->NPC_type[i] = '_';
	}

	//set origin
	bot->s.pos.trType = TR_INTERPOLATE;
	bot->s.pos.trTime = level.time;
	VectorCopy( bot->r.currentOrigin, bot->s.pos.trBase );
	VectorClear( bot->s.pos.trDelta );
	bot->s.pos.trDuration = 0;
	//set angles
	bot->s.apos.trType = TR_INTERPOLATE;
	bot->s.apos.trTime = level.time;
	//VectorCopy( newent->r.currentOrigin, newent->s.apos.trBase );
	//Why was the origin being used as angles? Typo I'm assuming -rww
	VectorCopy( bot->s.angles, bot->s.apos.trBase );

	VectorClear( bot->s.apos.trDelta );
	bot->s.apos.trDuration = 0;

	bot->NPC->combatPoint = -1;

	if (!(bot->s.eFlags & EF_FAKE_NPC_BOT))
		bot->s.eFlags |= EF_FAKE_NPC_BOT;
	if (!(bot->client->ps.eFlags & EF_FAKE_NPC_BOT))
		bot->client->ps.eFlags |= EF_FAKE_NPC_BOT;

	//bot->flags |= FL_NO_KNOCKBACK;//don't fall off ledges

	bot->client->ps.weapon = WP_SABER;//init for later check in NPC_Begin

	NPC_DefaultScriptFlags(bot);

	NPC_ParseParms(bot->NPC_type, bot);

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
	
	if (bs)
	{
		// Set angles... Convert to ideal view angles then run the bot code...
		VectorSet(bs->ideal_viewangles, SHORT2ANGLE(NPCS.ucmd.angles[PITCH] + NPCS.client->ps.delta_angles[PITCH]), SHORT2ANGLE(NPCS.ucmd.angles[YAW] + NPCS.client->ps.delta_angles[YAW]), SHORT2ANGLE(NPCS.ucmd.angles[ROLL] + NPCS.client->ps.delta_angles[ROLL]));
		VectorCopy(bs->ideal_viewangles, bs->viewangles);
		VectorCopy(bs->ideal_viewangles, bot->client->ps.viewangles);
		//VectorCopy(bs->ideal_viewangles, bot->r.currentAngles);
		//VectorCopy(bs->ideal_viewangles, bot->s.angles);
		trap->EA_View(bs->client, bs->ideal_viewangles);
	}

	/*
	if (NPCS.NPCInfo->goalEntity)
	{
		vec3_t dir;
		VectorSubtract(NPCS.NPCInfo->goalEntity->r.currentOrigin, bot->r.currentOrigin, dir);
		VectorCopy(dir, bot->client->ps.moveDir);
	}

	if (NPCS.ucmd.buttons & BUTTON_WALKING)
	{
		trap->EA_Action(bot->s.number, ACTION_WALK);
		trap->EA_Move(bot->s.number, bot->client->ps.moveDir, 5000.0);
	}
	else
	{
		trap->EA_Move(bot->s.number, bot->client->ps.moveDir, 5000.0);
	}
	*/

	if (NPCS.NPC->enemy && Distance(NPCS.NPC->r.currentOrigin, NPCS.NPC->enemy->r.currentOrigin) > 64)
	{
		//vec3_t dir;

		NPCS.NPCInfo->goalEntity = NPCS.NPC->enemy;
		
		/*
		VectorSubtract(NPCS.NPCInfo->goalEntity->r.currentOrigin, bot->r.currentOrigin, dir);
		VectorCopy(dir, bot->client->ps.moveDir);

		if (NPCS.ucmd.buttons & BUTTON_WALKING)
		{
			trap->EA_Action(bot->s.number, ACTION_WALK);
			trap->EA_Move(bot->s.number, bot->client->ps.moveDir, 5000.0);
			acted = qtrue;
		}
		else
		{
			trap->EA_Move(bot->s.number, bot->client->ps.moveDir, 5000.0);
			acted = qtrue;
		}
		*/

		NPC_MoveToGoal(qtrue);
		acted = qtrue;
	}

	/*
	if (NPCS.ucmd.upmove > 0)
	{
		trap->EA_Jump(bot->s.number);
		acted = qtrue;
	}
	
	if (NPCS.ucmd.upmove < 0)
	{
		trap->EA_Crouch(bot->s.number);
		acted = qtrue;
	}
	
	if (NPCS.ucmd.rightmove > 0)
	{
		trap->EA_MoveRight(bot->s.number);
		acted = qtrue;
	}

	if (NPCS.ucmd.rightmove < 0)
	{
		trap->EA_MoveLeft(bot->s.number);
		acted = qtrue;
	}

	if (NPCS.ucmd.forwardmove > 0)
	{
		trap->EA_MoveForward(bot->s.number);
		acted = qtrue;
	}

	if (NPCS.ucmd.forwardmove < 0)
	{
		trap->EA_MoveBack(bot->s.number);
		acted = qtrue;
	}
	*/

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

		trap->EA_Attack(bot->s.number);
		acted = qtrue;
	}

	if (NPCS.ucmd.buttons & BUTTON_ALT_ATTACK)
	{
		trap->EA_Alt_Attack(bot->s.number);
		acted = qtrue;
	}

	if (NPCS.ucmd.buttons & BUTTON_USE)
	{
		trap->EA_Use(bot->s.number);
		acted = qtrue;
	}

	/*
	if (NPCS.ucmd.buttons & BUTTON_WALKING)
	{
		trap->EA_Action(bot->s.number, ACTION_WALK);
		acted = qtrue;
	}
	*/

	return acted;
}

vec3_t oldMoveDir;
extern void ClientThink_real( gentity_t *ent );
extern void NPC_ApplyRoff (void);
extern void NPC_Think ( gentity_t *self);

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

	DOM_SetFakeNPCName(bot); // Make sure they have a name...

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

	NPC_Think(bot);
	DOM_FakeNPC_Parse_UCMD(bs, bot);

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
	else if (!(bot->client->ps.pm_flags & PMF_DUCKED) && (bot->r.maxs[2] < bot->client->ps.standheight || bot->r.maxs[1] > 10))
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
