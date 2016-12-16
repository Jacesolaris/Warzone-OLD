//This file contains functions relate to the saber impact behavior of OJP Enhanced's saber system.
//Stoiss NOTE //[NewSaberSys] new functions call has been modified or delete in this section here and does not behave like OJP anymore
#include "g_local.h"
#include "bg_local.h"

extern int G_PowerLevelForSaberAnim(gentity_t *ent, int saberNum, qboolean mySaberHit);
extern qboolean BG_SuperBreakWinAnim(int anim);
extern stringID_table_t StanceTable[];
extern qboolean BG_SabersOff(playerState_t *ps);
extern qboolean BG_StabDownAnim(int anim);
extern qboolean G_ClientIdleInWorld(gentity_t *ent);
extern qboolean PM_StaggerAnim(int anim);
extern qboolean BG_SaberInNonIdleDamageMove(playerState_t *ps, int AnimIndex);
extern qboolean WP_SabersCheckLock(gentity_t *ent1, gentity_t *ent2);
//extern bot_state_t *botstates[MAX_CLIENTS];
extern stringID_table_t animTable[MAX_ANIMATIONS + 1];
//[SaberSys]
#ifdef __MISSILES_AUTO_PARRY__
extern void WP_SaberBlockNonRandom(gentity_t *self, vec3_t hitloc, qboolean missileBlock);
#else
extern void WP_SaberBlock(gentity_t *playerent, vec3_t hitloc, qboolean missileBlock);
#endif //__MISSILES_AUTO_PARRY__
//[/SaberSys]
extern void G_SaberPerformeBounce(gentity_t* self, gentity_t* other, qboolean bodyhit);
extern int OJP_SaberCanBlock(gentity_t *self, gentity_t *atk, qboolean checkBBoxBlock, vec3_t point, int rSaberNum, int rBladeNum);
extern int PM_SaberBounceForAttack(int move);
qboolean SaberAttacking(gentity_t *self);
extern qboolean saberKnockOutOfHand(gentity_t *saberent, gentity_t *saberOwner, vec3_t velocity);
extern qboolean BG_InKnockDown(int anim);
extern qboolean BG_SaberInTransitionAny(int move);
qboolean PM_SaberInBrokenParry(int move);
extern void G_Stagger(gentity_t *hitEnt, gentity_t *atk, int currentBP);
extern qboolean CheckManualBlocking(gentity_t *attacker, gentity_t *defender);

void SabBeh_AttackVsAttack(gentity_t *self,	gentity_t *otherOwner)
{//set the saber behavior for two attacking blades hitting each other

	if (WP_SabersCheckLock(self, otherOwner))
	{
		self->client->ps.saberBlocked = BLOCKED_NONE;
		otherOwner->client->ps.saberBlocked = BLOCKED_NONE;

	}
	
	if (WP_SabersCheckLock(otherOwner, self))
	{
		self->client->ps.saberBlocked = BLOCKED_NONE;
		otherOwner->client->ps.saberBlocked = BLOCKED_NONE;

	}
	
}

void SabBeh_AttackVsBlock( gentity_t *attacker, gentity_t *blocker, vec3_t hitLoc, qboolean hitSaberBlade)
{//set the saber behavior for an attacking vs blocking/parrying blade impact
	qboolean startSaberLock = qfalse;

	if (SaberAttacking(attacker))		//Perfect blocked
	{
		G_SaberPerformeBounce(blocker, attacker, qfalse);

		if (!(WP_PlayerSaberAttack(blocker) && (blocker->client->pers.cmd.buttons & BUTTON_SPECIALBUTTON2) &&
			(blocker->client->nStaggerTime < level.time) && !(BG_InKnockDown(blocker->client->ps.legsAnim) ||
			BG_InKnockDown(blocker->client->ps.torsoAnim) || (blocker->client->ps.forceHandExtend == HANDEXTEND_KNOCKDOWN))))
		{ // Also can't be staggering or be in knock-down

		}
	}

	if(!OnSameTeam(attacker, blocker) || g_friendlySaber.integer)
	{//don't do parries or charge/regen DP unless we're in a situation where we can actually hurt the target.
		if(!startSaberLock)
		{//normal saber blocks
			//update the blocker's block move
			blocker->client->ps.saberLockFrame = 0; //break out of saberlocks.
			//[SaberSys]
#ifdef __MISSILES_AUTO_PARRY__
			WP_SaberBlockNonRandom(blocker, hitLoc, qfalse);
#else
			WP_SaberBlock(blocker, hitLoc, qfalse);
#endif //__MISSILES_AUTO_PARRY__
			//[/SaberSys]
		}
	}
}

void Update_Saberblocking(gentity_t *self, gentity_t *otherOwner, vec3_t hitLoc, qboolean *didHit, qboolean otherHitSaberBlade)
{	
	if(!otherOwner)
	{//not a saber-on-saber hit, no mishap handling.
		return;
	}

	if(BG_SaberInNonIdleDamageMove(&self->client->ps, self->localAnimIndex) )
	{//self is attacking
		if(BG_SaberInNonIdleDamageMove(&otherOwner->client->ps, otherOwner->localAnimIndex)) 
		{//and otherOwner is attacking
			OJP_SaberCanBlock(otherOwner, self, qfalse, hitLoc, -1, -1);
		}
		else if(OJP_SaberCanBlock(otherOwner, self, qfalse, hitLoc, -1, -1))
		{//and otherOwner is blocking or parrying
			//this is called with dual with both sabers[DUALRAWR]
			SabBeh_AttackVsBlock(self, otherOwner, hitLoc, otherHitSaberBlade);
			*didHit = qfalse;
		}
		else
		{//otherOwner in some other state
			G_Stagger(self, otherOwner, qfalse);
		}
	}
	else if( OJP_SaberCanBlock(self, otherOwner, qfalse, hitLoc, -1, -1) )
	{//self is blocking or parrying
		if(BG_SaberInNonIdleDamageMove(&otherOwner->client->ps, otherOwner->localAnimIndex))
		{//and otherOwner is attacking
			SabBeh_AttackVsBlock(otherOwner, self, hitLoc, qtrue);
		}
		else if(OJP_SaberCanBlock(otherOwner, self, qfalse, hitLoc, -1, -1))
		{//and otherOwner is blocking or parrying
			CheckManualBlocking(self, otherOwner);
		}
		else
		{//otherOwner in some other state
			G_Stagger(otherOwner, self, qfalse);
		}
	}
	else
	{//whatever other states self can be in.  (returns, bounces, or something)
		G_SaberPerformeBounce(otherOwner, self, qfalse);
		G_SaberPerformeBounce(self, otherOwner, qfalse);
	}
}

//[NewSaberSys]
void G_SaberPerformeBounce(gentity_t* self, gentity_t* other, qboolean bodyhit)
{
	if (((other->client && other->client->ps.stats[STAT_HEALTH] > 0) || (!other->client && g_entities[other->s.number].health > 0)) &&
		!BG_SaberInSpecialAttack(self->client->ps.torsoAnim) && // Unless we're doing a special attack...
		(!PM_StaggerAnim(self->client->ps.torsoAnim) && !bodyhit)) //Or staggering and its not the bodyhit
	{//The attack didn't kill your opponent, bounce the saber back to prevent passthru.
		if (SaberAttacking(self))
		{
			self->client->ps.saberMove = PM_SaberBounceForAttack(self->client->ps.saberMove);
			self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
		}
		else
		{
			self->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
		}
	}
}
//[/NewSaberSys]

//[NewSaberSys]
void G_Stagger(gentity_t *hitEnt, gentity_t *atk, int currentBP)
{
	if (PM_StaggerAnim(hitEnt->client->ps.torsoAnim) || PM_StaggerAnim(atk->client->ps.torsoAnim))
	{
		return;
	}
	if (PM_InGetUpAnimation(hitEnt->client->ps.legsAnim))
	{
		return;
	}
	if (hitEnt->client->ps.forceHandExtend == HANDEXTEND_KNOCKDOWN &&
		hitEnt->client->ps.forceHandExtendTime > level.time)
	{
		return;//Don't allow stagger if the defender is in a slap animation.
	}

	if ((atk->client->ps.torsoAnim != BOTH_JUMPFLIPSLASHDOWN1 || atk->client->ps.torsoTimer < 800)
		&& (atk->client->ps.torsoAnim != BOTH_JUMPATTACK6 || atk->client->ps.torsoTimer < 200))
	{
		return;// Only yellow DFA or dual butterfly can stagger opponents
	}

	G_SetAnim(hitEnt, &(hitEnt->client->pers.cmd), SETANIM_TORSO, BOTH_BASHED1, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD, 0);

	if (PM_StaggerAnim(hitEnt->client->ps.torsoAnim))
	{
		hitEnt->client->ps.saberMove = LS_NONE;
		hitEnt->client->ps.saberBlocked = BLOCKED_NONE; // Needed to prevent nudge bugging the stagger.
		hitEnt->client->ps.weaponTime = hitEnt->client->ps.torsoTimer;
		hitEnt->client->nStaggerTime = hitEnt->client->ps.torsoTimer + level.time;
	}
}
//[/NewSaberSys]

//[NewSaberSys]
qboolean WP_PlayerSaberAttack(gentity_t *self)
{
	//We have a lot of different checks to see if we are actually attacking
	if (BG_SaberInAttack(self->client->ps.saberMove))
	{
		return qtrue;
	}

	if (PM_InSaberAnim(self->client->ps.torsoAnim) && !self->client->ps.saberBlocked &&
		self->client->ps.saberMove != LS_READY && self->client->ps.saberMove != LS_NONE)
	{
		if (self->client->ps.saberMove < LS_PARRY_UP || self->client->ps.saberMove > LS_REFLECT_LL)
		{
			return qtrue;
		}
	}

	if (PM_SaberInBrokenParry(self->client->ps.saberMove))
	{
		return qtrue;
	}

	if (self->client->pers.cmd.buttons & BUTTON_ATTACK)
	{ //don't block when the player is trying to slash
		return qtrue;
	}


	if (SaberAttacking(self))
	{ //attacking, can't block now
		return qtrue;
	}

	if (self->client->ps.saberMove != LS_READY &&
		!self->client->ps.saberBlocking)
	{
		return qtrue;
	}

	return qfalse;
}
//[/NewSaberSys]