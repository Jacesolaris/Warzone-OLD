#include "b_local.h"
#include "g_nav.h"
#include "anims.h"

extern int gWPNum;
extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];

extern float MAX_LINK_DISTANCE;

extern int DOM_GetNearestWP(vec3_t org, int badwp);
extern int NPC_GetNextNode(gentity_t *NPC);
extern qboolean UQ1_UcmdMoveForDir ( gentity_t *self, usercmd_t *cmd, vec3_t dir, qboolean walk, vec3_t dest );
extern qboolean NPC_ValidEnemy2( gentity_t *self, gentity_t *ent );
extern qboolean NPC_FindEnemy( qboolean checkAlerts );
extern qboolean NPC_DoLiftPathing(gentity_t *NPC);
extern void NPC_NewWaypointJump ( void );
extern qboolean NPC_NeedsHeal ( gentity_t *NPC );

void TeleportNPC( gentity_t *player, vec3_t origin, vec3_t angles ) {
//	gentity_t	*tent;
	qboolean	isNPC = qfalse;
	qboolean	noAngles;

	if (player->s.eType == ET_NPC)
	{
		isNPC = qtrue;
	}

	noAngles = (angles[0] > 999999.0) ? qtrue : qfalse;

	// use temp events at source and destination to prevent the effect
	// from getting dropped by a second player event
#if 0 // UQ1: We don't want fx...
	if ( player->client->sess.sessionTeam != FACTION_SPECTATOR ) {
		tent = G_TempEntity( player->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = player->s.clientNum;

		tent = G_TempEntity( origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = player->s.clientNum;
	}
#endif //0

	// unlink to make sure it can't possibly interfere with G_KillBox
	trap->UnlinkEntity ((sharedEntity_t *)player);

	VectorCopy ( origin, player->client->ps.origin );
	player->client->ps.origin[2] += 1;

	// spit the player out
	if ( !noAngles ) {
		AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
		VectorScale( player->client->ps.velocity, 400, player->client->ps.velocity );
		player->client->ps.pm_time = 160;		// hold time
		player->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;

		// set angles
		SetClientViewAngle( player, angles );
	}

	// toggle the teleport bit so the client knows to not lerp
	player->client->ps.eFlags ^= EF_TELEPORT_BIT;

#if 0 // UQ1: Nope...
	// kill anything at the destination
	if ( player->client->sess.sessionTeam != FACTION_SPECTATOR ) {
		G_KillBox (player);
	}
#endif //0

	// save results of pmove
	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

	if (isNPC)
	{
		player->s.eType = ET_NPC;
	}

	// use the precise origin for linking
	VectorCopy( player->client->ps.origin, player->r.currentOrigin );

	if ( player->client->sess.sessionTeam != FACTION_SPECTATOR ) {
		trap->LinkEntity ((sharedEntity_t *)player);
	}
}

int NPC_FindPadawanGoal( gentity_t *NPC )
{
	vec3_t traceGoal, fwd, right, up, start, end;
	trace_t tr;

	if (NPC->padawanNoWaypointTime > level.time) return -1;
	if (!NPC->parent) return -1;
	if (!NPC_IsAlive(NPC, NPC->parent)) return -1;

	AngleVectors( NPC->parent->client->ps.viewangles, fwd, right, up );
	CalcMuzzlePoint( NPC->parent, fwd, right, up, end );
	VectorMA( end, 512, fwd, traceGoal );
	VectorCopy(NPC->parent->r.currentOrigin, start);
	start[2] += 48;
	trap->Trace( &tr, start, NULL, NULL, traceGoal, NPCS.NPC->s.number, MASK_SHOT, qfalse, 0, 0 );
	//trap->Print("Trace end %f %f %f.\n", tr.endpos[0], tr.endpos[1], tr.endpos[2]);

	if (NPC->longTermGoal < 0 
		|| NPC->longTermGoal > gWPNum 
		|| Distance(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) > 1000
		|| Distance(gWPArray[NPC->longTermGoal]->origin, tr.endpos) > 192)
	{// Only if we need a new goal...
		int goalWP = DOM_GetNearWP(tr.endpos, -1);

		if (goalWP < 0) goalWP = DOM_GetNearWP(NPC->parent->r.currentOrigin, -1);

		if (goalWP < 0) return -1;

		NPC->padawanNoWaypointTime = level.time + 1000;

		if (Distance(gWPArray[goalWP]->origin, NPC->r.currentOrigin) < 128)
		{
			//trap->Print("Padawan failed to find a goal WP\n");
			return -1;
		}

		//trap->Print("Padawan found a goal WP: %i. Distance %f. Postition %f %f %f.\n", goalWP, Distance(gWPArray[goalWP]->origin, NPC->r.currentOrigin), gWPArray[goalWP]->origin[0], gWPArray[goalWP]->origin[1], gWPArray[goalWP]->origin[2]);
		return goalWP;
	}

	return -1;
}

extern void G_SpeechEvent( gentity_t *self, int event );

void G_AddPadawanCombatCommentEvent( gentity_t *self, int event, int speakDebounceTime )
{
	if ( !self->NPC )
	{
		return;
	}

	if ( !self->client || self->client->ps.pm_type >= PM_DEAD )
	{
		return;
	}

	if ( self->NPC->padawanCombatCommentDebounceTime > level.time )
	{
		return;
	}

	if ( self->NPC->blockedSpeechDebounceTime > level.time )
	{
		return;
	}

	if ( trap->ICARUS_TaskIDPending( (sharedEntity_t *)self, TID_CHAN_VOICE ) )
	{
		return;
	}

	//FIXME: Also needs to check for teammates. Don't want
	//		everyone babbling at once

	//NOTE: was losing too many speech events, so we do it directly now, screw networking!
	G_SpeechEvent( self, event );
	
	//won't speak again for 5 seconds (unless otherwise specified)
	self->NPC->padawanCombatCommentDebounceTime = level.time + ((speakDebounceTime==0) ? 15000+irand(0,15000) : speakDebounceTime);

	// also disable normal (original jka) combat speech for 5 secs...
	self->NPC->blockedSpeechDebounceTime = level.time + ((speakDebounceTime==0) ? 5000 : speakDebounceTime);
}

void G_AddPadawanIdleNoReplyCommentEvent( gentity_t *self, int event, int speakDebounceTime )
{
	if ( !self->NPC )
	{
		return;
	}

	if ( !self->client || self->client->ps.pm_type >= PM_DEAD )
	{
		return;
	}

	if ( self->NPC->padawanCommentDebounceTime > level.time )
	{
		return;
	}

	if ( trap->ICARUS_TaskIDPending( (sharedEntity_t *)self, TID_CHAN_VOICE ) )
	{
		return;
	}

	//FIXME: Also needs to check for teammates. Don't want
	//		everyone babbling at once

	//NOTE: was losing too many speech events, so we do it directly now, screw networking!
	G_SpeechEvent( self, event );

	//won't speak again for 5 seconds (unless otherwise specified)
	self->NPC->padawanCommentDebounceTime = level.time + ((speakDebounceTime==0) ? 15000+irand(0,15000) : speakDebounceTime);
}

void G_AddPadawanCommentEvent( gentity_t *self, int event, int speakDebounceTime )
{
	if ( !self->NPC )
	{
		return;
	}

	if ( !self->client || self->client->ps.pm_type >= PM_DEAD )
	{
		return;
	}

	if ( self->NPC->padawanCommentDebounceTime > level.time )
	{
		return;
	}

	if ( trap->ICARUS_TaskIDPending( (sharedEntity_t *)self, TID_CHAN_VOICE ) )
	{
		return;
	}

	//FIXME: Also needs to check for teammates. Don't want
	//		everyone babbling at once

	//NOTE: was losing too many speech events, so we do it directly now, screw networking!
	G_SpeechEvent( self, event );

	if (self->parent)
	{// Tell our master to reply...
		self->parent->padawan_reply_time = level.time + 10000;
		self->parent->padawan_reply_waiting = qtrue;
	}

	//won't speak again for 5 seconds (unless otherwise specified)
	self->NPC->padawanCommentDebounceTime = level.time + ((speakDebounceTime==0) ? 30000+irand(0,30000) : speakDebounceTime);
}

void NPC_SetNewPadawanGoalAndPath( void )
{
	gentity_t	*NPC = NPCS.NPC;

	if (NPC->npc_dumb_route_time > level.time)
	{// Try to use JKA routing as a backup until timer runs out...
		if ( UpdateGoal() )
		{
			if (NPC_CombatMoveToGoal( qtrue, qfalse ))
			{// Worked!
				return;
			}
		}

		// Failed... Idle...
		NPCS.ucmd.forwardmove = 0;
		NPCS.ucmd.rightmove = 0;
		NPCS.ucmd.upmove = 0;
		NPC_PickRandomIdleAnimantion(NPC);
		return;
	}

	if (NPC->wpSeenTime > level.time)
	{
		NPC_PickRandomIdleAnimantion(NPC);
		return; // wait for next route creation...
	}

	if (!NPC_FindNewWaypoint())
	{
		NPC->npc_dumb_route_time = level.time + 10000;
		return; // wait before trying to get a new waypoint...
	}

	NPC->longTermGoal = DOM_GetNearestWP(NPCS.NPCInfo->goalEntity->r.currentOrigin, NPC->wpCurrent);

	if (NPC->longTermGoal >= 0)
	{
		memset(NPC->pathlist, WAYPOINT_NONE, sizeof(int)*MAX_WPARRAY_SIZE);
		NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, qfalse);

		if (NPC->pathsize > 0)
		{
			//G_Printf("NPC Waypointing Debug: NPC %i created a %i waypoint path for a random goal between waypoints %i and %i.", NPC->s.number, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal);
			NPC->wpLast = -1;
			NPC->wpNext = NPC_GetNextNode(NPC);		//move to this node first, since it's where our path starts from
		}
		else
		{
			NPC->npc_dumb_route_time = level.time + 10000;
			//G_Printf("NPC Waypointing Debug: NPC %i failed to create a route between waypoints %i and %i.", NPC->s.number, NPC->wpCurrent, NPC->longTermGoal);
			// Delay before next route creation...
			NPC->wpSeenTime = level.time + 1000;//30000;
			NPC_PickRandomIdleAnimantion(NPC);
			return;
		}
	}
	else
	{
		//G_Printf("NPC Waypointing Debug: NPC %i failed to find a goal waypoint.", NPC->s.number);
		
		//trap->Print("Unable to find goal waypoint.\n");

		NPC->npc_dumb_route_time = level.time + 10000;

		// Delay before next route creation...
		NPC->wpSeenTime = level.time + 10000;//30000;
		NPC_PickRandomIdleAnimantion(NPC);
		return;
	}

	// Delay before next route creation...
	NPC->wpSeenTime = level.time + 1000;//30000;
	// Delay before giving up on this new waypoint/route...
	NPC->wpTravelTime = level.time + 15000;
}

qboolean NPC_FollowPadawanRoute( void ) 
{// Quick method of following bot routes...
	gentity_t	*NPC = NPCS.NPC;
	usercmd_t	ucmd = NPCS.ucmd;
	float		wpDist = 0.0;
	qboolean	onMover1 = qfalse;
	qboolean	onMover2 = qfalse;

	NPCS.NPCInfo->combatMove = qtrue;

	if ( !NPC->parent || !NPC_IsAlive(NPC, NPC->parent) )
	{
		//trap->Print("parent dead!\n");
		return qfalse;
	}

	if (!NPCS.NPCInfo->goalEntity)
	{
		//trap->Print("no goal!\n");
		//return qfalse;
		NPCS.NPCInfo->goalEntity = NPC->parent;
	}

	if (NPC_GetOffPlayer(NPC))
	{// Get off of their head!
		//trap->Print("head!\n");
		return qfalse;
	}

	if (NPC_MoverCrushCheck(NPC))
	{// There is a mover gonna crush us... Step back...
		//trap->Print("mover!\n");
		return qtrue;
	}

	if (Distance(NPC->r.currentOrigin, NPCS.NPCInfo->goalEntity->r.currentOrigin) <= 128)
	{// Close enough already... Don't move...
		//trap->Print("close!\n");
		return qfalse;
	}

	if (DistanceHorizontal(NPC->r.currentOrigin, NPC->npc_previous_pos) > 3)
	{
		NPC->last_move_time = level.time;
		VectorCopy(NPC->r.currentOrigin, NPC->npc_previous_pos);
	}

	if ( NPC->wpCurrent >= 0 && NPC->wpCurrent < gWPNum )
	{
		vec3_t upOrg, upOrg2;

		VectorCopy(NPC->r.currentOrigin, upOrg);
		upOrg[2]+=18;

		VectorCopy(gWPArray[NPC->wpCurrent]->origin, upOrg2);
		upOrg2[2]+=18;

		if (OrgVisible(upOrg, upOrg2, NPC->s.number))
		{
			NPC->wpSeenTime = level.time;
		}

		if (NPC_IsJetpacking(NPC))
		{// Jetpacking... Ignore heights...
			wpDist = DistanceHorizontal(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);
		}
		else if (NPC->s.groundEntityNum != ENTITYNUM_NONE)
		{// On somebody's head or in the air...
			wpDist = DistanceHorizontal(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);
		}
		else
		{// On ground...
			wpDist = Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);
		}

	}

	if ( NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum 
		|| NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum 
		//|| wpDist > MAX_LINK_DISTANCE
		|| NPC->wpSeenTime < level.time - 5000
		|| NPC->wpTravelTime < level.time 
		|| NPC->last_move_time < level.time - 5000 
		|| Distance(gWPArray[NPC->longTermGoal]->origin, NPCS.NPCInfo->goalEntity->r.currentOrigin) > 256.0)
	{// We hit a problem in route, or don't have one yet.. Find a new goal and path...

		if (wpDist > MAX_LINK_DISTANCE || NPC->wpTravelTime < level.time )
		{
			NPC_RoutingIncreaseCost( NPC->wpLast, NPC->wpCurrent );
		}

		NPC_ClearPathData(NPC);
		NPC_SetNewPadawanGoalAndPath();
		G_ClearEnemy(NPC); // UQ1: Give up...

		if (!(NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum))
		{
			NPC->wpTravelTime = level.time + 15000;
			NPC->wpSeenTime = level.time;
			NPC->last_move_time = level.time;
		}
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// FIXME: Try to roam out of problems...
		NPC_ClearPathData(NPC);
		ucmd.forwardmove = 0;
		ucmd.rightmove = 0;
		ucmd.upmove = 0;
		NPC_PickRandomIdleAnimantion(NPC);
		//trap->Print("NO WP!\n");
		return qfalse; // next think...
	}

	if (Distance(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) < 48
		|| (NPC->s.groundEntityNum != ENTITYNUM_NONE && DistanceHorizontal(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) < 48))
	{// We're at out goal! Find a new goal...
		NPC_ClearPathData(NPC);
		ucmd.forwardmove = 0;
		ucmd.rightmove = 0;
		ucmd.upmove = 0;
		NPC_PickRandomIdleAnimantion(NPC);
		//trap->Print("AT DEST!\n");
		return qfalse; // next think...
	}

	if (NPC_DoLiftPathing(NPC))
	{
		//trap->Print("lift!\n");
		return qtrue;
	}

	if (wpDist < 48)
	{// At current node.. Pick next in the list...
		NPC->wpLast = NPC->wpCurrent;
		NPC->wpCurrent = NPC->wpNext;
		NPC->wpNext = NPC_GetNextNode(NPC);

		if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
		{// FIXME: Try to roam out of problems...
			NPC_ClearPathData(NPC);
			ucmd.forwardmove = 0;
			ucmd.rightmove = 0;
			ucmd.upmove = 0;
			NPC_PickRandomIdleAnimantion(NPC);
			//trap->Print("????\n");
			return qfalse; // next think...
		}

		NPC->wpTravelTime = level.time + 15000;
		NPC->wpSeenTime = level.time;
	}

	{
		vec3_t upOrg, upOrg2;

		VectorCopy(NPC->r.currentOrigin, upOrg);
		upOrg[2]+=18;

		VectorCopy(gWPArray[NPC->wpCurrent]->origin, upOrg2);
		upOrg2[2]+=18;

		if (OrgVisible(upOrg, upOrg2, NPC->s.number))
		{
			NPC->wpSeenTime = level.time;
		}
	}

	NPC_FacePosition( gWPArray[NPC->wpCurrent]->origin, qfalse );
	VectorSubtract( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->movedir );

	if (VectorLength(NPC->client->ps.velocity) < 8 && NPC_RoutingJumpWaypoint( NPC->wpLast, NPC->wpCurrent ))
	{// We need to jump to get to this waypoint...
		if (NPC_Jump(NPC, gWPArray[NPC->wpCurrent]->origin))
		{
			//trap->Print("NPC JUMP DEBUG: NPC_FollowEnemyRoute\n");
			VectorCopy( NPC->movedir, NPC->client->ps.moveDir );
			return qtrue;
		}
	}
	else if (VectorLength(NPC->client->ps.velocity) < 8)
	{// If this is a new waypoint, we may need to jump to it...
		NPC_NewWaypointJump();
	}

	if (!UQ1_UcmdMoveForDir( NPC, &NPCS.ucmd, NPC->movedir, qfalse, gWPArray[NPC->wpCurrent]->origin )) { /*NPC_PickRandomIdleAnimantion(NPC);*/ return qtrue; }
	VectorCopy( NPC->movedir, NPC->client->ps.moveDir );
	//NPC_SelectMoveAnimation(qfalse);

	return qtrue;
}

qboolean NPC_PadawanMove( void )
{
	gentity_t	*NPC = NPCS.NPC;
	usercmd_t	*ucmd = &NPCS.ucmd;

	if (NPC->enemy && NPC_IsAlive(NPC, NPC->enemy) && NPC_ValidEnemy2(NPC, NPC->enemy))
	{// Keep fighting who we are fighting...
		return qtrue;
	}

	if (NPC->s.NPC_class == CLASS_PADAWAN)
	{
		G_ClearEnemy( NPCS.NPC );

		if (NPC->parent && NPC_IsAlive(NPC, NPC->parent))
		{
			int			goalWP = -1;
			vec3_t		goal, angles;
			float		dist;
			gentity_t	*goalEnt = NULL;

			if (!NPC->padawanGoalEntity)
			{
				NPC->padawanGoalEntity = G_Spawn();
				NPC->padawanGoalEntity->s.eType = ET_NPCGOAL;
				NPC->padawanGoalEntity->inuse = qtrue;
				trap->LinkEntity ((sharedEntity_t *)NPC->padawanGoalEntity);
			}

			goalEnt = NPC->padawanGoalEntity;

			if (NPC->padawanWaitTime > level.time)
			{// Wait a moment where you are...
				vec3_t traceGoal, fwd, right, up, start, end;
				trace_t tr;

				AngleVectors( NPC->parent->client->ps.viewangles, fwd, right, up );
				CalcMuzzlePoint( NPC->parent, fwd, right, up, end );
				VectorMA( end, 512, fwd, traceGoal );
				VectorCopy(NPC->parent->r.currentOrigin, start);
				start[2] += 48;
				trap->Trace( &tr, start, NULL, NULL, traceGoal, NPCS.NPC->s.number, MASK_SHOT, qfalse, 0, 0 );

				if (NPC->longTermGoal >= 0 
					&& (Distance(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) > 1000 || Distance(gWPArray[NPC->longTermGoal]->origin, tr.endpos) > 192))
				{// Player has looked somewhere else, start moving there instantly...
					NPC->padawanWaitTime = 0;

					if (!(NPC->enemy && NPC_IsAlive(NPC, NPC->enemy)))
						NPC_FindEnemy( qtrue );
				}
				else if (NPC->longTermGoal >= 0 && NPC->padawanWaitTime < level.time + 500 && NPC->padawanReturnToPlayerTime < level.time)
				{// Head back to jedi...
					NPC->padawanReturnToPlayerTime = level.time + 5000;
					NPC->longTermGoal = -1;

					ucmd->forwardmove = 0;
					ucmd->rightmove = 0;
					ucmd->upmove = 0;
					NPC_PickRandomIdleAnimantion(NPC);

					if (!(NPC->enemy && NPC_IsAlive(NPC, NPC->enemy)))
						NPC_FindEnemy( qtrue );

					return qtrue;
				}
				else
				{// Wait here for a second...
					ucmd->forwardmove = 0;
					ucmd->rightmove = 0;
					ucmd->upmove = 0;
					NPC_PickRandomIdleAnimantion(NPC);

					if (!(NPC->enemy && NPC_IsAlive(NPC, NPC->enemy)))
						NPC_FindEnemy( qtrue );

					return qtrue;
				}
			}

			if (NPC->padawanReturnToPlayerTime >= level.time)
			{
				goalWP = -1;
				NPC->longTermGoal = goalWP;
			}
			else
			{
				goalWP = NPC_FindPadawanGoal(NPC);
			}

			if (goalWP >= 0) 
			{// Have a new goal... Set it up...
				NPC->longTermGoal = goalWP;
			}

			if (NPC->longTermGoal < 0 || NPC->longTermGoal > gWPNum) NPC->longTermGoal = -1;

			goalWP = NPC->longTermGoal;

			if (goalWP < 0)
			{// Have no current goal... Head to jedi master...
				VectorCopy(NPC->parent->r.currentOrigin, goal);
			}
			else
			{
				VectorCopy(gWPArray[goalWP]->origin, goal);
			}

			dist = DistanceHorizontal(goal, NPC->r.currentOrigin);

			VectorCopy(goal, goalEnt->r.currentOrigin);
			VectorCopy(goal, goalEnt->s.origin);
			G_SetOrigin( goalEnt, goal );

			VectorSet(angles, 0.0, 0.0, 0.0);
			G_SetAngles( goalEnt, angles );

			if (NPC->enemy && NPC_IsAlive(NPC, NPC->enemy))
			{// Continue using normal movement...
				if (NPC_FollowEnemyRoute())
				{// Try using waypoints...
					return qtrue;
				}
			}
			else if (dist > 112 && dist < 256)
			{// If clear then move stright there...
				NPC_FacePosition( goal, qfalse );

				NPCS.NPCInfo->goalEntity = goalEnt;

				if ( UpdateGoal() )
				{
					Padawan_CheckForce();

					if (dist > 512 
						&& TIMER_Done( NPCS.NPC, "protect" )
						&& NPCS.NPC->client->ps.fd.forcePowerLevel[FP_PROTECT] > 0)
					{// When the master is a fair way away, use force protect to get to him safer...
						ForceProtect( NPCS.NPC );
						TIMER_Set( NPCS.NPC, "protect", irand(15000, 30000) );
					}

					if (NPC_CombatMoveToGoal( qtrue, qfalse ))
					{// All is good in the world...
						return qtrue;
					}
					else if (NPC->parent->client->ps.groundEntityNum == ENTITYNUM_NONE)
					{// Out master is in the air... Don't jump!
						NPC_ClearGoal();
						NPCS.NPCInfo->goalEntity = NULL;
						NPCS.NPCInfo->tempGoal = NULL;

						ucmd->forwardmove = 0;
						ucmd->rightmove = 0;
						ucmd->upmove = 0;
						NPC_PickRandomIdleAnimantion(NPC);

						return qtrue;
					}
					else if (NPC_FollowPadawanRoute())
					{// Try using waypoints...
						NPCS.NPCInfo->goalEntity = goalEnt;
						NPC_FacePosition( goal, qfalse );
						return qtrue;
					}
				}
				
				if (irand(0,2) > 0)
				{// Random idle sound... Yawns, giggles, etc...
					G_AddPadawanIdleNoReplyCommentEvent( NPC, EV_PADAWAN_IDLE_NOREPLY, 10000+irand(0,15000) );
				}

				//trap->Print("dist > 96 && dist < 512 FAIL!\n");
			}
			else if (dist >= 256 && NPC_FollowPadawanRoute())
			{// Try using waypoints...
				Padawan_CheckForce();

				if (dist > 512 
					&& TIMER_Done( NPCS.NPC, "protect" )
					&& NPCS.NPC->client->ps.fd.forcePowerLevel[FP_PROTECT] > 0)
				{// When the master is a fair way away, use force protect to get to him safer...
					ForceProtect( NPCS.NPC );
					TIMER_Set( NPCS.NPC, "protect", irand(15000, 30000) );
				}

				NPCS.NPCInfo->goalEntity = goalEnt;
				NPC_FacePosition( goal, qfalse );
				return qtrue;
			}
			else if (dist >= 256)
			{
				NPC_FacePosition( goal, qfalse );

				NPCS.NPCInfo->goalEntity = goalEnt;

				if ( UpdateGoal() )
				{
					Padawan_CheckForce();

					if (dist > 512 
						&& TIMER_Done( NPCS.NPC, "protect" )
						&& NPCS.NPC->client->ps.fd.forcePowerLevel[FP_PROTECT] > 0)
					{// When the master is a fair way away, use force protect to get to him safer...
						ForceProtect( NPCS.NPC );
						TIMER_Set( NPCS.NPC, "protect", irand(15000, 30000) );
					}

					if (NPC_CombatMoveToGoal( qtrue, qfalse ))
					{// All is good in the world...
						return qtrue;
					}
					else if (NPC->parent->client->ps.groundEntityNum == ENTITYNUM_NONE)
					{// Out master is in the air... Don't jump!
						NPC_ClearGoal();
						NPCS.NPCInfo->goalEntity = NULL;
						NPCS.NPCInfo->tempGoal = NULL;

						ucmd->forwardmove = 0;
						ucmd->rightmove = 0;
						ucmd->upmove = 0;
						NPC_PickRandomIdleAnimantion(NPC);

						return qtrue;
					}
				}
			}
//#if 0
			else if (dist < 96)
			{// If clear then move back a bit...
				NPC_FacePosition( goal, qfalse );

				NPCS.NPCInfo->goalEntity = goalEnt;

				if ( UpdateGoal() )
				{
					//if (walk) NPCS.ucmd.buttons |= BUTTON_WALKING;
					//Jedi_Move( NPCS.NPCInfo->goalEntity, qfalse );
					NPC_CombatMoveToGoal( qtrue, qtrue );
					return qtrue;
				}

				if (irand(0,2) > 0)
				{// Random idle sound... Yawns, giggles, etc...
					G_AddPadawanIdleNoReplyCommentEvent( NPC, EV_PADAWAN_IDLE_NOREPLY, 10000+irand(0,15000) );
				}
			}
//#endif
			else if (dist <= 128)
			{// Perfect distance... Stay idle...
				NPC_ClearGoal();
				NPCS.NPCInfo->goalEntity = NULL;
				NPCS.NPCInfo->tempGoal = NULL;

				ucmd->forwardmove = 0;
				ucmd->rightmove = 0;
				ucmd->upmove = 0;
				NPC_PickRandomIdleAnimantion(NPC);

				if (irand(0,2) > 0)
				{// Random idle sound... Yawns, giggles, etc...
					G_AddPadawanIdleNoReplyCommentEvent( NPC, EV_PADAWAN_IDLE_NOREPLY, 10000+irand(0,15000) );
				}
				else
				{// Say something random to our master... And expect a reply...
					G_AddPadawanCommentEvent( NPC, EV_PADAWAN_IDLE, 30000+irand(0,30000) );
				}

				NPC->padawanWaitTime = level.time + 1500;

				return qtrue;
			}
			else if (NPC->parent->s.groundEntityNum != ENTITYNUM_NONE
				&& Distance(NPC->parent->r.currentOrigin, NPC->r.currentOrigin) > 4096)
			{// Padawan is too far from jedi. Teleport to him... Only if they are not in mid air...
				if (NPC->parent->client->ps.groundEntityNum == ENTITYNUM_NONE)
				{// Out master is in the air... Don't teleport!
					NPC_ClearGoal();
					NPCS.NPCInfo->goalEntity = NULL;
					NPCS.NPCInfo->tempGoal = NULL;

					ucmd->forwardmove = 0;
					ucmd->rightmove = 0;
					ucmd->upmove = 0;
					NPC_PickRandomIdleAnimantion(NPC);

					return qtrue;
				}
				else if (NPC_FollowPadawanRoute())
				{// Try using waypoints...
					NPCS.NPCInfo->goalEntity = goalEnt;
					NPC_FacePosition( goal, qfalse );
					return qtrue;
				}
				else if (NPC->nextPadawanTeleportThink <= level.time)
				{// Desperation, teleport to master...
					int waypoint = DOM_GetNearWP(NPC->parent->r.currentOrigin, NPC->parent->wpCurrent);

					NPC->nextPadawanTeleportThink = level.time + 5000;

					if (waypoint >= 0 && waypoint < gWPNum)
					{
						TeleportNPC( NPC, gWPArray[waypoint]->origin, NPC->s.angles );

						NPC_ClearGoal();
						NPCS.NPCInfo->goalEntity = NULL;
						NPCS.NPCInfo->tempGoal = NULL;

						ucmd->forwardmove = 0;
						ucmd->rightmove = 0;
						ucmd->upmove = 0;
						NPC_PickRandomIdleAnimantion(NPC);

						//trap->Print("TELEPORT!\n");

						return qtrue;
					}
				}
			}
		}
		else if (irand(0,2) > 0)
		{// Random idle sound... Yawns, giggles, etc...
			G_AddPadawanIdleNoReplyCommentEvent( NPC, EV_PADAWAN_IDLE_NOREPLY, 10000+irand(0,15000) );
		}

		return qtrue;
	}

	return qfalse;
}

qboolean NPC_NeedPadawan_Spawn ( void )
{// UQ1: Because I don't want to end up with a map full of padawans without a jedi...
	int i;
	int padawan_count = 0;
	int jedi_count = 0;

	for (i = 0; i < MAX_GENTITIES; i++)
	{// Find the closest jedi to follow...
		gentity_t *parent2 = &g_entities[i];

		if ( parent2
			&& NPC_IsAlive(parent2, parent2)
			&& parent2->client
			&& parent2->client->sess.sessionTeam == FACTION_REBEL
			&& (parent2->client->NPC_class == CLASS_JEDI || parent2->client->NPC_class == CLASS_LUKE || parent2->client->NPC_class == CLASS_KYLE || (parent2->s.eType == ET_PLAYER && parent2->s.primaryWeapon == WP_SABER)))
		{// This is a jedi on our team...
			jedi_count++;
		}
		else if ( parent2
			&& parent2->client
			&& NPC_IsAlive(parent2, parent2)
			//&& parent2->client->sess.sessionTeam == FACTION_REBEL
			&& parent2->client->NPC_class == CLASS_PADAWAN)
		{// This is a padawan on our team...
			if (parent2->client->sess.sessionTeam != FACTION_REBEL)
				parent2->client->sess.sessionTeam = FACTION_REBEL; // must have been manually spawned.. set team info...

			padawan_count++;
		}
	}

	if (jedi_count >= padawan_count)
	{// If we have equal to or more jedi then padawans, then we need a new padawan...
		return qtrue;
	}

	return qfalse;
}

qboolean Padawan_CheckForce ( void )
{// UQ1: New code to make better use of force powers...
	//
	// Give them any force powers they might need...
	//
	if (!(NPCS.NPC->client->ps.fd.forcePowersKnown & (1 << FP_TEAM_HEAL))) 
	{
		NPCS.NPC->client->ps.fd.forcePowersKnown |= (1 << FP_TEAM_HEAL);
		NPCS.NPC->client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] = 3;
	}
	if (!(NPCS.NPC->client->ps.fd.forcePowersKnown & (1 << FP_HEAL))) 
	{
		NPCS.NPC->client->ps.fd.forcePowersKnown |= (1 << FP_HEAL);
		NPCS.NPC->client->ps.fd.forcePowerLevel[FP_HEAL] = 3;
	}
	if (!(NPCS.NPC->client->ps.fd.forcePowersKnown & (1 << FP_PROTECT))) 
	{
		NPCS.NPC->client->ps.fd.forcePowersKnown |= (1 << FP_PROTECT);
		NPCS.NPC->client->ps.fd.forcePowerLevel[FP_PROTECT] = 3;
	}
	if (!(NPCS.NPC->client->ps.fd.forcePowersKnown & (1 << FP_ABSORB))) 
	{
		NPCS.NPC->client->ps.fd.forcePowersKnown |= (1 << FP_ABSORB);
		NPCS.NPC->client->ps.fd.forcePowerLevel[FP_ABSORB] = 3;
	}
	if (!(NPCS.NPC->client->ps.fd.forcePowersKnown & (1 << FP_TELEPATHY))) 
	{
		NPCS.NPC->client->ps.fd.forcePowersKnown |= (1 << FP_TELEPATHY);
		NPCS.NPC->client->ps.fd.forcePowerLevel[FP_TELEPATHY] = 3;
	}
	if (!(NPCS.NPC->client->ps.fd.forcePowersKnown & (1 << FP_PUSH))) 
	{
		NPCS.NPC->client->ps.fd.forcePowersKnown |= (1 << FP_PUSH);
		NPCS.NPC->client->ps.fd.forcePowerLevel[FP_PUSH] = 3;
	}
	if (!(NPCS.NPC->client->ps.fd.forcePowersKnown & (1 << FP_PULL))) 
	{
		NPCS.NPC->client->ps.fd.forcePowersKnown |= (1 << FP_PULL);
		NPCS.NPC->client->ps.fd.forcePowerLevel[FP_PULL] = 3;
	}
	if (!(NPCS.NPC->client->ps.fd.forcePowersKnown & (1 << FP_SABERTHROW))) 
	{
		NPCS.NPC->client->ps.fd.forcePowersKnown |= (1 << FP_SABERTHROW);
		NPCS.NPC->client->ps.fd.forcePowerLevel[FP_SABERTHROW] = 3;
	}

	/*if ( NPCS.NPC->client->ps.fd.forcePowersActive&(1<<FP_DRAIN) )
	{//when draining, don't move
		return qtrue;
	}

	if ( NPCS.NPC->client->ps.fd.forcePowersActive&(1<<FP_TEAM_HEAL) )
	{//when team healing, don't move
		return qtrue;
	}

	if ( NPCS.NPC->client->ps.fd.forcePowersActive&(1<<FP_HEAL) )
	{//lvl 1 healing, don't move
		return qtrue;
	}*/

	// UQ1: Special heals/protects/absorbs - mainly for padawans...
	if ( TIMER_Done( NPCS.NPC, "teamheal" )
		&& NPCS.NPC->parent
		&& NPC_IsAlive(NPCS.NPC, NPCS.NPC->parent)
		&& Distance(NPCS.NPC->parent->r.currentOrigin, NPCS.NPC->r.currentOrigin) < 256
		&& NPCS.NPC->client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] > 0
		&& (NPCS.NPC->s.NPC_class == CLASS_PADAWAN)
		&& NPC_NeedsHeal( NPCS.NPC->parent )
		&& Q_irand( 0, 20 ) < 2)
	{// Team heal our jedi???
		NPC_FacePosition(NPCS.NPC->parent->r.currentOrigin, qtrue);
		ForceTeamHeal( NPCS.NPC );
		TIMER_Set( NPCS.NPC, "teamheal", irand(5000, 15000) );
		return qtrue;
	}
	else if ( TIMER_Done( NPCS.NPC, "heal" )
		&& NPCS.NPC->client->ps.fd.forcePowerLevel[FP_HEAL] > 0
		&& (NPCS.NPC->s.NPC_class == CLASS_PADAWAN)
		&& NPC_NeedsHeal( NPCS.NPC )
		&& Q_irand( 0, 20 ) < 2)
	{// We need to heal...
		ForceHeal( NPCS.NPC );
		TIMER_Set( NPCS.NPC, "heal", irand(5000, 15000) );
		return qtrue;
	}
	else if ( TIMER_Done( NPCS.NPC, "protect" )
		&& NPCS.NPC->client->ps.fd.forcePowerLevel[FP_PROTECT] > 0
		&& (Q_irand( 0, 20 ) < 2 ))
	{// General buff...
		ForceProtect( NPCS.NPC );
		TIMER_Set( NPCS.NPC, "protect", irand(15000, 30000) );
		return qtrue;
	}
	else if ( TIMER_Done( NPCS.NPC, "absorb" )
		&& NPCS.NPC->client->ps.fd.forcePowerLevel[FP_ABSORB] > 0
		&& (Q_irand( 0, 20 ) < 2))
	{// General buff...
		ForceAbsorb( NPCS.NPC );
		TIMER_Set( NPCS.NPC, "absorb", irand(15000, 30000) );
		return qtrue;
	}

	return qfalse;
}

void NPC_Padawan_CopyParentFlags ( gentity_t *me, gentity_t *parent )
{
	if (!me || !NPC_IsAlive(me, me))
	{
		return;
	}
	
	if (!parent || !NPC_IsAlive(me, parent))
	{// Reset all flags if parent is dead...
		if (me->flags & FL_NOTARGET)
		{
			me->flags &= ~FL_NOTARGET;
		}

		if (me->flags & FL_GODMODE)
		{
			me->flags &= ~FL_GODMODE;
		}

		return;
	}

	// Copy my parent's NOTARGET flag...
	if ((parent->flags & FL_NOTARGET) && !(me->flags & FL_NOTARGET))
	{
		me->flags |= FL_NOTARGET;
	}
	else if (me->flags & FL_NOTARGET)
	{
		me->flags &= ~FL_NOTARGET;
	}

	// Copy my parent's GODMODE flag...
	if ((parent->flags & FL_GODMODE) && !(me->flags & FL_GODMODE))
	{
		me->flags |= FL_GODMODE;
	}
	else if (me->flags & FL_GODMODE)
	{
		me->flags &= ~FL_GODMODE;
	}
}

void NPC_DoPadawanStuff ( void )
{
	int			i = 0;
	gentity_t	*me = NPCS.NPC;
	gentity_t	*parent = me->parent;
	int			best_parent_dist = 999999;
	gentity_t	*best_parent = NULL;

	if (!(g_gametype.integer == GT_WARZONE || g_gametype.integer == GT_INSTANCE) )
	{// not in other gametypes...
		return;
	}

	if (NPCS.NPC->client->NPC_class != CLASS_PADAWAN)
	{
		return; // This is only for padawans...
	}

	NPC_Padawan_CopyParentFlags(me, parent);

	if (NPC_GetOffPlayer(NPCS.NPC))
	{// Get off of their head!
		return;
	}

	if (NPC_MoverCrushCheck(NPCS.NPC))
	{// There is a mover gonna crush us... Step back...
		return;
	}

	if (me->nextPadawanThink > level.time) return;

	// Does he need to heal up, or use a buff???
	Padawan_CheckForce();

	me->nextPadawanThink = level.time + 5000;

	me->NPC->combatMove = qtrue;

	if (me->client->sess.sessionTeam != FACTION_REBEL)
		me->client->sess.sessionTeam = FACTION_REBEL; // must have been manually spawned.. set team info...

	if (parent && NPC_IsAlive(me, parent))
	{
		parent->padawan = me;
		NPC_Padawan_CopyParentFlags(me, parent);

		if (parent->enemy && NPC_IsAlive(parent, parent->enemy))
		{// Padawan assists jedi...
			float Jdist = Distance(me->r.currentOrigin, parent->r.currentOrigin);
			float Edist = Distance(me->r.currentOrigin, parent->enemy->r.currentOrigin);

			if ( Jdist <= 384 && Edist <= 384 )
			{
				if (!me->enemy || !NPC_IsAlive(me, me->enemy))
					me->enemy = parent->enemy;
			}
		}
		else
		{
			if (me->enemy && NPC_IsAlive(me, me->enemy))
			{// Jedi assists padawan... No range limit on jedi helping the padawan...
				if (!parent->enemy || !NPC_IsAlive(me, parent->enemy))
					parent->enemy = me->enemy;
			}
		}

		if (!(me->enemy && NPC_IsAlive(me, me->enemy)))
		{
			if (parent->client->ps.saberHolstered > 0)
			{// Copy our master's saber holster setting...
				if (me->client->ps.saberHolstered != 2)
				{
					me->client->ps.saberHolstered = 2;
				}
			}
			else
			{// Copy our master's saber holster setting...
				if (me->client->ps.saberHolstered != 0)
				{
					me->client->ps.saberHolstered = 0;
				}
			}
		}

		if (!me->enemy || !NPC_IsAlive(me, me->enemy))
			NPC_ClearGoal();

		return; // Already have a master to follow...
	}

	if (me->enemy && NPC_IsAlive(me, me->enemy) && NPC_ValidEnemy2(me, me->enemy))
	{// Keep fighting who we are fighting...
		return;
	}

	//
	// Looks like we need a new master, if we are in fact a padawan...
	//

	me->isPadawan = qtrue; // Mark us as a padawan for the next check to not need to do a string compare..
	parent = NULL;

	// We need to select a master...
	for (i = 0; i < MAX_GENTITIES; i++)
	{// Find the closest jedi to follow...
		gentity_t *parent2 = &g_entities[i];

		if ( parent2
			&& NPC_IsAlive(parent2, parent2)
			&& parent2->client
			&& parent2->client->sess.sessionTeam == me->client->sess.sessionTeam
			&& (parent2->client->NPC_class == CLASS_JEDI || parent2->client->NPC_class == CLASS_LUKE || parent2->client->NPC_class == CLASS_KYLE || parent2->s.eType == ET_PLAYER)
			&& (!parent2->padawan || !NPC_IsAlive(parent2, parent2->padawan)) )
		{// This is a jedi on our team...
			float dist = Distance(me->r.currentOrigin, parent2->r.currentOrigin);
			
			if (dist < best_parent_dist)
			{
				if (dist < best_parent_dist)
				{// Found a new best jedi...
					best_parent = parent2;
					best_parent_dist = dist;
				}
			}
		}
	}

	if (best_parent)
	{// Set our new master...
		parent = best_parent;
		me->parent = parent;
		parent->padawan = me;

		NPC_Padawan_CopyParentFlags(me, parent);

		//trap->Print("Padawan %s found a new jedi (%s).\n", me->client->pers.netname, parent->client->pers.netname);

		if (parent->enemy && NPC_IsAlive(me, parent->enemy))
		{// Padawan assists jedi...
			float Jdist = Distance(me->r.currentOrigin, parent->r.currentOrigin);
			float Edist = Distance(me->r.currentOrigin, parent->enemy->r.currentOrigin);

			if ( Jdist <= 384 && Edist <= 384 )
			{
				if (!me->enemy || !NPC_IsAlive(me, me->enemy))
					me->enemy = parent->enemy;
			}
		}
		else
		{
			if (me->enemy && NPC_IsAlive(me, me->enemy))
			{// Jedi assists padawan... No range limit on jedi helping the padawan...
				if (!parent->enemy || !NPC_IsAlive(me, parent->enemy))
					parent->enemy = me->enemy;
			}
		}

		if (!(me->enemy && NPC_IsAlive(me, me->enemy)))
		{
			if (parent->client->ps.saberHolstered > 0)
			{// Copy our master's saber holster setting...
				if (me->client->ps.saberHolstered != 2)
				{
					me->client->ps.saberHolstered = 2;
				}
			}
			else
			{// Copy our master's saber holster setting...
				if (me->client->ps.saberHolstered != 0)
				{
					me->client->ps.saberHolstered = 0;
				}
			}
		}
	}
	else
	{// Need to check again next time...
		//trap->Print("Padawan %s failed to find a new jedi.\n", me->client->pers.netname);
		me->parent = NULL;
	}

	if (!me->enemy || !NPC_IsAlive(me, me->enemy))
	{// Not in combat...
		if (me->enemy && !NPC_IsAlive(me, me->enemy))
		{// Looks like we just killed someone... Make a kill comment...
			G_AddPadawanCombatCommentEvent( me, EV_PADAWAN_COMBAT_KILL_TALK, 10000+irand(0,15000) );
		}

		NPC_ClearGoal();
	}
	else
	{// In combat...
		if (irand(0,2) > 0)
		{// Make random combat comments...
			G_AddPadawanCombatCommentEvent( me, EV_PADAWAN_COMBAT_TALK, 10000+irand(0,15000) );
		}
	}
}
