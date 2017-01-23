#include "b_local.h"
#include "g_nav.h"
#include "anims.h"

extern int gWPNum;
extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];

extern int DOM_GetNearestWP(vec3_t org, int badwp);
extern int NPC_GetNextNode(gentity_t *NPC);
extern qboolean UQ1_UcmdMoveForDir ( gentity_t *self, usercmd_t *cmd, vec3_t dir, qboolean walk, vec3_t dest );
//extern qboolean NPC_CoverpointVisible ( gentity_t *NPC, int coverWP );
extern int DOM_GetRandomCloseWP(vec3_t org, int badwp, int unused);
extern int DOM_GetNearestVisibleWP(vec3_t org, int ignore, int badwp);
extern int DOM_GetNearestVisibleWP_Goal(vec3_t org, int ignore, int badwp);
extern int DOM_GetNearestVisibleWP_NOBOX(vec3_t org, int ignore, int badwp);
extern gentity_t *NPC_PickEnemyExt( qboolean checkAlerts );
extern qboolean NPC_MoveDirClear( int forwardmove, int rightmove, qboolean reset );
extern void G_UcmdMoveForDir( gentity_t *self, usercmd_t *cmd, vec3_t dir, vec3_t dest );

qboolean DOM_NPC_ClearPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum );
extern int DOM_GetRandomCloseVisibleWP(gentity_t *ent, vec3_t org, int ignoreEnt, int badwp);


#if defined(__OLD_PATROL__)

int NPC_GetPatrolWP(gentity_t *NPC)
{
	int		i, NUM_FOUND = 0, FOUND_LIST[1024];
	float	PATROL_RANGE = NPC->patrol_range;
	float	flLen;

	for (i = 0; i < gWPNum; i++)
	{
		if (gWPArray[i] 
			&& gWPArray[i]->inuse 
			&& DistanceVertical(gWPArray[i]->origin, NPC->r.currentOrigin) <= 32.0//24.0
			&& DistanceVertical(gWPArray[i]->origin, NPC->spawn_pos) <= 32.0)//24.0)
		{
			vec3_t org, org2;

			flLen = Distance(NPC->r.currentOrigin, gWPArray[i]->origin);

			if (flLen < PATROL_RANGE && flLen >= PATROL_RANGE * 0.4/*128.0*/)
			{
				VectorCopy(NPC->r.currentOrigin, org);
				org[2]+=8;

				VectorCopy(gWPArray[i]->origin, org2);
				org2[2]+=8;

				if (DOM_NPC_ClearPathToSpot( NPC, org2, NPC->s.number ))
				{
					FOUND_LIST[NUM_FOUND] = i;
					NUM_FOUND++;

					if (NUM_FOUND > 1024) break; // hit max num...
				}
			}
		}
	}

	if (NUM_FOUND <= 0)
	{
		return -1;
	}

	// Return a random one...
	return FOUND_LIST[Q_irand(0, NUM_FOUND)];
}

qboolean NPC_FindNewPatrolWaypoint()
{
	gentity_t *NPC = aiEnt;

	if (NPC->noWaypointTime > level.time)
	{// Only try to find a new waypoint every 10 seconds...
		NPC_PickRandomIdleAnimantion(NPC);
		return qfalse;
	}

	//NPC->patrol_range = 512.0;
	NPC->patrol_range = 384.0;
	//NPC->patrol_range = 450.0;

	NPC->noWaypointTime = level.time + 5000 + irand(0, 5000); // 5 to 10 seconds before we try again... (it will run avoidance in the meantime)

	NPC->wpCurrent = NPC_GetPatrolWP(NPC);

	if (NPC->wpCurrent <= 0 || NPC->wpCurrent >= gWPNum)
	{
		NPC_PickRandomIdleAnimantion(NPC);
		return qfalse;
	}

	NPC->wpNext = NPC->wpCurrent;
	NPC->longTermGoal = NPC->wpCurrent;

	NPC->wpTravelTime = level.time + 10000;

	if (NPC->wpSeenTime < NPC->noWaypointTime)
		NPC->wpSeenTime = NPC->noWaypointTime; // also make sure we don't try to make a new route for the same length of time...

	//G_Printf("NPC Waypointing Debug: NPC %i [%s] (spawn pos %f %f %f) found a patrol waypoint for itself at %f %f %f (patrol range %f).", NPC->s.number, NPC->NPC_type, NPC->spawn_pos[0], NPC->spawn_pos[1], NPC->spawn_pos[2], gWPArray[NPC->wpCurrent]->origin[0], gWPArray[NPC->wpCurrent]->origin[1], gWPArray[NPC->wpCurrent]->origin[2], NPC->patrol_range);
	return qtrue; // all good, we have a new waypoint...
}

qboolean NPC_PatrolArea( void ) 
{// Quick method of patroling...
	gentity_t	*NPC = aiEnt;
	qboolean	ENEMY_VISIBLE = qfalse;
	qboolean	HUNTING_ENEMY = qfalse;
	qboolean	FORCED_COVERSPOT_FIND = qfalse;

	if (gWPNum <= 0)
	{// No waypoints available...
		NPC_PickRandomIdleAnimantion(NPC);
		return qfalse;
	}

	if (NPC->enemy && NPC_IsAlive(NPC, NPC->enemy))
	{// Chase them...
		NPC->return_home = qtrue;
		return qfalse;
	}
	else if (NPC->return_home)
	{// Returning home after chase and kill...
		return qfalse;
	}

	if ( !NPC->enemy )
	{
		switch (NPC->client->NPC_class)
		{
		case CLASS_CIVILIAN:
		case CLASS_CIVILIAN_R2D2:
		case CLASS_CIVILIAN_R5D2:
		case CLASS_CIVILIAN_PROTOCOL:
		case CLASS_CIVILIAN_WEEQUAY:
		case CLASS_GENERAL_VENDOR:
		case CLASS_WEAPONS_VENDOR:
		case CLASS_ARMOR_VENDOR:
		case CLASS_SUPPLIES_VENDOR:
		case CLASS_FOOD_VENDOR:
		case CLASS_MEDICAL_VENDOR:
		case CLASS_GAMBLER_VENDOR:
		case CLASS_TRADE_VENDOR:
		case CLASS_ODDITIES_VENDOR:
		case CLASS_DRUG_VENDOR:
		case CLASS_TRAVELLING_VENDOR:
			// These guys have no enemies...
			break;
		default:
			if ( NPC->client->enemyTeam != NPCTEAM_NEUTRAL )
			{
				NPC->enemy = NPC_PickEnemyExt( qtrue );

				if (NPC->enemy)
				{
					if (NPC->client->ps.weapon == WP_SABER)
						G_AddVoiceEvent( NPC, Q_irand( EV_JDETECTED1, EV_JDETECTED3 ), 15000 + irand(0, 30000) );
					else
					{
						G_AddVoiceEvent( NPC, Q_irand( EV_DETECTED1, EV_DETECTED5 ), 15000 + irand(0, 30000) );
					}
				}
			}
			break;
		}
	}

	if (!NPC->return_home
		&& (NPC->r.currentOrigin[2] > NPC->spawn_pos[2]+24 || NPC->r.currentOrigin[2] < NPC->spawn_pos[2]-24))
	{// We have fallen... Set this spot as our new patrol location...
		VectorCopy(NPC->r.currentOrigin, NPC->spawn_pos);
		NPC->longTermGoal = -1;
		NPC->wpCurrent = -1;
		NPC->pathsize = -1;
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->wpTravelTime < level.time)
	{// Patrol Point...
		NPC_FindNewPatrolWaypoint();
		NPC->return_home = qfalse;
		NPC_PickRandomIdleAnimantion(NPC);
		return qfalse; // next think...
	}

	if (Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 64)
	{// We're at out goal! Find a new goal...
		NPC->longTermGoal = -1;
		NPC->wpCurrent = -1;
		NPC->pathsize = -1;
		NPC_PickRandomIdleAnimantion(NPC);
		return qfalse; // next think...
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum)
	{// FIXME: Try to roam out of problems...
		//trap->Print("PATROL: Lost.\n");
		NPC_PickRandomIdleAnimantion(NPC);
		return qfalse; // next think...
	}

	NPC_FacePosition( NPC, gWPArray[NPC->wpCurrent]->origin, qfalse );
	VectorSubtract( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->movedir );
	UQ1_UcmdMoveForDir( NPC, &aiEnt->client->pers.cmd, NPC->movedir, qtrue, gWPArray[NPC->wpCurrent]->origin );
	VectorCopy( NPC->movedir, NPC->client->ps.moveDir );

	if (aiEnt->client->pers.cmd.forwardmove == 0 && aiEnt->client->pers.cmd.rightmove == 0 && aiEnt->client->pers.cmd.upmove == 0)
		NPC_PickRandomIdleAnimantion(NPC);
	else
		NPC_SelectMoveAnimation(qtrue);

	return qtrue;
}

#elif defined(__SIMPLE_PATROL__)

qboolean NPC_PatrolArea( gentity_t *aiEnt)
{// Quick method of patroling... The fastest way possible. Single waypoint using it's neighbours...
	gentity_t	*NPC = aiEnt;
	usercmd_t	*ucmd = &aiEnt->client->pers.cmd;

	ucmd->forwardmove = 0;
	ucmd->rightmove = 0;
	ucmd->upmove = 0;

	if (gWPNum <= 0)
	{// No waypoints available...
		NPC_PickRandomIdleAnimantion(NPC);

		NPC->last_move_time = level.time;
		VectorCopy(NPC->r.currentOrigin, NPC->npc_previous_pos);
		return qfalse;
	}

	if (NPC->noWaypointTime > level.time)
	{// Wait...
		NPC_PickRandomIdleAnimantion(NPC);

		NPC->last_move_time = level.time;
		VectorCopy(NPC->r.currentOrigin, NPC->npc_previous_pos);
		return qtrue;
	}

	if (NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// Should only ever happen once...
		NPC->longTermGoal = DOM_GetNearestWP(NPC->r.currentOrigin, -1);
	}

	if (NPC->longTermGoal < 0)
	{
		NPC->last_move_time = level.time;
		VectorCopy(NPC->r.currentOrigin, NPC->npc_previous_pos);
		return qfalse;
	}

	if (gWPArray[NPC->longTermGoal]->neighbornum <= 0)
	{// This wp happens to have no neighbours.. Just stand idle...
		NPC_PickRandomIdleAnimantion(NPC);

		NPC->last_move_time = level.time;
		VectorCopy(NPC->r.currentOrigin, NPC->npc_previous_pos);
		return qfalse;
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum)
	{// Pick a new patrol neighbour...
		if (NPC->wpLast == NPC->longTermGoal)
		{// At home wp. Head to a random link...
			int choice = irand(0, gWPArray[NPC->longTermGoal]->neighbornum-1);
			NPC->wpCurrent = gWPArray[NPC->longTermGoal]->neighbors[choice].num;
		}
		else
		{// At a link. Head home...
			NPC->wpCurrent = NPC->longTermGoal;
		}
	}

	if (Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 64)
	{// We're at out goal! Wait, then find a new goal...
		NPC->wpLast = NPC->wpCurrent;
		NPC->wpCurrent = -1;
		NPC->noWaypointTime = level.time + 10000;
		NPC_PickRandomIdleAnimantion(NPC);

		NPC->last_move_time = level.time;
		VectorCopy(NPC->r.currentOrigin, NPC->npc_previous_pos);
		return qfalse; // next think...
	}

	if (DistanceHorizontal(NPC->r.currentOrigin, NPC->npc_previous_pos) > 3)
	{
		NPC->last_move_time = level.time;
		VectorCopy(NPC->r.currentOrigin, NPC->npc_previous_pos);
	}

	NPC_FacePosition(aiEnt, gWPArray[NPC->wpCurrent]->origin, qfalse );
	VectorSubtract( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->movedir );
	VectorCopy(NPC->movedir, NPC->client->ps.moveDir);

	if (!UQ1_UcmdMoveForDir(NPC, &aiEnt->client->pers.cmd, NPC->movedir, qtrue, gWPArray[NPC->wpCurrent]->origin))
	{
		if (NPC->bot_strafe_jump_timer > level.time)
		{
			ucmd->upmove = 127;

			if (NPC->s.eType == ET_PLAYER)
			{
				trap->EA_Jump(NPC->s.number);
			}
		}
		else if (NPC->bot_strafe_left_timer > level.time)
		{
			ucmd->rightmove = -127;
			trap->EA_MoveLeft(NPC->s.number);
		}
		else if (NPC->bot_strafe_right_timer > level.time)
		{
			ucmd->rightmove = 127;
			trap->EA_MoveRight(NPC->s.number);
		}

		if (NPC->last_move_time < level.time - 2000)
		{
			ucmd->upmove = 127;

			if (NPC->s.eType == ET_PLAYER)
			{
				trap->EA_Jump(NPC->s.number);
			}
		}
	}
	else if (NPC->bot_strafe_jump_timer > level.time)
	{
		ucmd->upmove = 127;

		if (NPC->s.eType == ET_PLAYER)
		{
			trap->EA_Jump(NPC->s.number);
		}
	}
	else if (NPC->bot_strafe_left_timer > level.time)
	{
		ucmd->rightmove = -127;
		trap->EA_MoveLeft(NPC->s.number);
	}
	else if (NPC->bot_strafe_right_timer > level.time)
	{
		ucmd->rightmove = 127;
		trap->EA_MoveRight(NPC->s.number);
	}

	if (NPC->last_move_time < level.time - 4000)
	{
		NPC_PickRandomIdleAnimantion(NPC);
		return qtrue;
	}

	if (NPC->last_move_time < level.time - 2000)
	{
		ucmd->upmove = 127;

		if (NPC->s.eType == ET_PLAYER)
		{
			trap->EA_Jump(NPC->s.number);
		}
	}

	if (aiEnt->client->pers.cmd.forwardmove == 0 && aiEnt->client->pers.cmd.rightmove == 0 && aiEnt->client->pers.cmd.upmove == 0)
		NPC_PickRandomIdleAnimantion(NPC);
	//else
	//	NPC_SelectMoveAnimation(aiEnt, qtrue);

	return qtrue;
}

#else

extern qboolean NPC_DoLiftPathing(gentity_t *NPC);
extern void NPC_NewWaypointJump(gentity_t *aiEnt);
extern qboolean Warzone_SpawnpointNearMoverEntityLocation(vec3_t org);

qboolean WAYPOINT_PARTOL_BAD_LIST_INITIALIZED = qfalse;
qboolean WAYPOINT_PARTOL_BAD_LIST[MAX_WPARRAY_SIZE] = { qfalse };

void NPC_Patrol_MakeBadList(void)
{
	if (WAYPOINT_PARTOL_BAD_LIST_INITIALIZED) return;

	for (int i = 0; i < gWPNum; i++)
	{
		if (Warzone_SpawnpointNearMoverEntityLocation(gWPArray[i]->origin))
		{
			WAYPOINT_PARTOL_BAD_LIST[i] = qtrue;
		}
	}

	WAYPOINT_PARTOL_BAD_LIST_INITIALIZED = qtrue;
}

#define MAX_BEST_PATROL_WAYPOINT_LIST 1024

int NPC_FindPatrolGoal(gentity_t *NPC)
{
	int bestWaypointsNum = 0;
	int bestWaypoints[MAX_BEST_PATROL_WAYPOINT_LIST] = { -1 };

	NPC_Patrol_MakeBadList(); // Init if it hasn't been checked yet...

	for (int i = 0; i < gWPNum && bestWaypointsNum < MAX_BEST_PATROL_WAYPOINT_LIST; i++)
	{
		if (WAYPOINT_PARTOL_BAD_LIST[i]) continue;

		float spawnDist = Distance(gWPArray[i]->origin, NPC->spawn_pos);
		float spawnDistHeight = DistanceVertical(gWPArray[i]->origin, NPC->spawn_pos);
		
		if (spawnDist < 2048.0 && spawnDist > 1024.0)
		{// If this spot is close to me, but not too close, then maybe add it to the best list...
			if (spawnDistHeight < spawnDist * 0.15)
			{// This point is at a height close to the original spawn position... Looks good...
				bestWaypoints[bestWaypointsNum] = i;
				bestWaypointsNum++;
			}
		}
	}

	if (bestWaypointsNum <= 0)
	{// Failed to find any, try a second method, allowing more options...
		for (int i = 0; i < gWPNum && bestWaypointsNum < MAX_BEST_PATROL_WAYPOINT_LIST; i++)
		{
			if (WAYPOINT_PARTOL_BAD_LIST[i]) continue;

			float spawnDist = Distance(gWPArray[i]->origin, NPC->spawn_pos);
			float spawnDistHeight = DistanceVertical(gWPArray[i]->origin, NPC->spawn_pos);

			if (spawnDist < 1024.0 && spawnDist > 256.0)
			{// If this spot is close to me, but not too close, then maybe add it to the best list...
				if (spawnDistHeight < spawnDist * 0.15)
				{// This point is at a height close to the original spawn position... Looks good...
					bestWaypoints[bestWaypointsNum] = i;
					bestWaypointsNum++;
				}
			}
		}
	}

	if (bestWaypointsNum <= 0)
	{// Failed...
		return -1;
	}

	int selected = bestWaypoints[irand_big(0, bestWaypointsNum - 1)];

	//trap->Print("NPC %s selected patrol point %i, which is %i distance from spawnpoint. spawn %i %i %i. wp %i %i %i.\n", NPC->client->pers.netname, selected, (int)Distance(NPC->spawn_pos, gWPArray[selected]->origin), (int)NPC->spawn_pos[0], (int)NPC->spawn_pos[1], (int)NPC->spawn_pos[2], (int)gWPArray[selected]->origin[0], (int)gWPArray[selected]->origin[1], (int)gWPArray[selected]->origin[2]);
	return bestWaypoints[irand_big(0, bestWaypointsNum - 1)];
}

void NPC_SetNewPatrolGoalAndPath(gentity_t *aiEnt)
{
	if (aiEnt->next_pathfind_time > level.time)
	{
		return;
	}

	aiEnt->next_pathfind_time = level.time + 10000 + irand(0, 1000);

	gentity_t	*NPC = aiEnt;

	if (NPC_IsJedi(NPC) && (!(NPC->client->ps.fd.forcePowersKnown & (1 << FP_LEVITATION)) || NPC->client->ps.fd.forcePowerLevel[FP_LEVITATION] < 3))
	{
		// Give all Jedi/Sith NPCs jump 3...
		NPC->client->ps.fd.forcePowersKnown |= (1 << FP_LEVITATION);
		NPC->client->ps.fd.forcePowerLevel[FP_LEVITATION] = 3;
	}
	else if (!(NPC->client->ps.fd.forcePowersKnown & (1 << FP_LEVITATION)) || NPC->client->ps.fd.forcePowerLevel[FP_LEVITATION] < 2)
	{// Give all NPCs jump 2 just for pathing the map and not getting stuck..
		NPC->client->ps.fd.forcePowersKnown |= (1 << FP_LEVITATION);
		NPC->client->ps.fd.forcePowerLevel[FP_LEVITATION] = 2;
	}

	if (NPC->wpSeenTime > level.time)
	{
		NPC_PickRandomIdleAnimantion(NPC);
		return; // wait for next route creation...
	}

	if (!NPC_FindNewWaypoint(aiEnt))
	{
		//trap->Print("Unable to find waypoint.\n");
		//player_die(NPC, NPC, NPC, 99999, MOD_CRUSH);
		return; // wait before trying to get a new waypoint...
	}

	//
	// First try preferred goal...
	//

	if (NPC->return_home)
	{// Returning home...
		NPC->longTermGoal = DOM_GetNearestWP(NPC->spawn_pos, NPC->wpCurrent);
	}
	else
	{// Find a new generic goal...
		NPC->longTermGoal = NPC_FindPatrolGoal(NPC);
	}

	if (NPC->longTermGoal >= 0)
	{
		memset(NPC->pathlist, WAYPOINT_NONE, sizeof(int)*MAX_WPARRAY_SIZE);
		NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, (qboolean)(irand(0, 5) <= 0));

		if (NPC->pathsize > 0)
		{
#ifdef ___AI_PATHING_DEBUG___
			trap->Print("NPC Waypointing Debug: NPC %i created a %i waypoint path for a random goal between waypoints %i and %i.\n", NPC->s.number, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal);
#endif //___AI_PATHING_DEBUG___
			NPC->wpLast = -1;
			NPC->wpNext = NPC_GetNextNode(NPC);		//move to this node first, since it's where our path starts from

													// Delay before next route creation...
			NPC->wpSeenTime = level.time + 1000;//30000;
												// Delay before giving up on this new waypoint/route...
			NPC->wpTravelTime = level.time + 15000;
			NPC->last_move_time = level.time;
			return;
		}
	}
}

qboolean NPC_PatrolArea(gentity_t *aiEnt)
{
	gentity_t	*NPC = aiEnt;
	usercmd_t	*ucmd = &aiEnt->client->pers.cmd;
	float		wpDist = 0.0;
	qboolean	onMover1 = qfalse;
	qboolean	onMover2 = qfalse;

	aiEnt->NPC->combatMove = qtrue;

	if (!NPC_HaveValidEnemy(aiEnt))
	{
		switch (NPC->client->NPC_class)
		{
		case CLASS_CIVILIAN:
		case CLASS_CIVILIAN_R2D2:
		case CLASS_CIVILIAN_R5D2:
		case CLASS_CIVILIAN_PROTOCOL:
		case CLASS_CIVILIAN_WEEQUAY:
		case CLASS_GENERAL_VENDOR:
		case CLASS_WEAPONS_VENDOR:
		case CLASS_ARMOR_VENDOR:
		case CLASS_SUPPLIES_VENDOR:
		case CLASS_FOOD_VENDOR:
		case CLASS_MEDICAL_VENDOR:
		case CLASS_GAMBLER_VENDOR:
		case CLASS_TRADE_VENDOR:
		case CLASS_ODDITIES_VENDOR:
		case CLASS_DRUG_VENDOR:
		case CLASS_TRAVELLING_VENDOR:
			NPC->r.contents = 0;
			NPC->clipmask = MASK_NPCSOLID&~CONTENTS_BODY;
			// These guys have no enemies...
			break;
		default:
			break;
		}
	}

	G_ClearEnemy(NPC);

	if (NPC_GetOffPlayer(NPC))
	{// Get off of their head!
		return qtrue;
	}

	if (NPC_MoverCrushCheck(NPC))
	{// There is a mover gonna crush us... Step back...
		return qtrue;
	}

	ucmd->forwardmove = 0;
	ucmd->rightmove = 0;
	ucmd->upmove = 0;
	
	if (NPC->noWaypointTime > level.time)
	{
		NPC_PickRandomIdleAnimantion(NPC);
		return qtrue;
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum
		|| NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum
		|| NPC->wpSeenTime < level.time - 5000
		|| NPC->wpTravelTime < level.time
		|| NPC->last_move_time < level.time - 15000)
	{// We hit a problem in route, or don't have one yet.. Find a new goal and path...
#ifdef ___AI_PATHING_DEBUG___
		if (NPC->wpSeenTime < level.time - 5000) trap->Print("PATHING DEBUG: %i wpSeenTime.\n", NPC->s.number);
		if (NPC->wpTravelTime < level.time) trap->Print("PATHING DEBUG: %i wpTravelTime.\n", NPC->s.number);
		if (NPC->last_move_time < level.time - 5000) trap->Print("PATHING DEBUG: %i last_move_time.\n", NPC->s.number);
		if ((NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum) && (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum)) trap->Print("PATHING DEBUG: %i wpCurrent & longTermGoal.\n", NPC->s.number);
		else if (NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum) trap->Print("PATHING DEBUG: %i longTermGoal.\n", NPC->s.number);
		else if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum) trap->Print("PATHING DEBUG: %i wpCurrent.\n", NPC->s.number);
#endif //___AI_PATHING_DEBUG___

		NPC_RoutingIncreaseCost(NPC->wpLast, NPC->wpCurrent);

		NPC_ClearPathData(NPC);
		NPC_SetNewPatrolGoalAndPath(aiEnt);
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// FIXME: Try to roam out of problems...
#ifdef ___AI_PATHING_DEBUG___
		trap->Print("PATHING DEBUG: NO PATH!\n");
#endif //___AI_PATHING_DEBUG___
		NPC_ClearPathData(NPC);
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		ucmd->upmove = 0;
		NPC_PickRandomIdleAnimantion(NPC);
		return qtrue; // next think...
	}

	if (Distance(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) < 48
		|| (NPC->s.groundEntityNum != ENTITYNUM_NONE && DistanceHorizontal(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) < 48))
	{// We're at our goal! Find a new goal...
#ifdef ___AI_PATHING_DEBUG___
		trap->Print("PATHING DEBUG: HIT GOAL!\n");
#endif //___AI_PATHING_DEBUG___
		NPC_ClearPathData(NPC);
		NPC->noWaypointTime = level.time + 10000; // Idle at least 10 seconds at this point before finding a new patrol position...
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		ucmd->upmove = 0;
		NPC_PickRandomIdleAnimantion(NPC);
		return qtrue; // next think...
	}

	if (DistanceHorizontal(NPC->r.currentOrigin, NPC->npc_previous_pos) > 3)
	{
		NPC->last_move_time = level.time;
		VectorCopy(NPC->r.currentOrigin, NPC->npc_previous_pos);
	}

	if (NPC->wpCurrent >= 0 && NPC->wpCurrent < gWPNum)
	{
		vec3_t upOrg, upOrg2;

		VectorCopy(NPC->r.currentOrigin, upOrg);
		upOrg[2] += 18;

		VectorCopy(gWPArray[NPC->wpCurrent]->origin, upOrg2);
		upOrg2[2] += 18;

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
			wpDist = DistanceHorizontal(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);//Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);
		}

		//if (wpDist > 512) trap->Print("To far! (%f)\n", wpDist);
	}

	if (wpDist < 48)
	{// At current node.. Pick next in the list...
	 //trap->Print("HIT WP %i. Next WP is %i.\n", NPC->wpCurrent, NPC->wpNext);

		NPC->wpLast = NPC->wpCurrent;
		NPC->wpCurrent = NPC->wpNext;
		NPC->wpNext = NPC_GetNextNode(NPC);

		if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
		{// FIXME: Try to roam out of problems...
#ifdef ___AI_PATHING_DEBUG___
			trap->Print("PATHING DEBUG: HIT ROUTE END!\n");
#endif //___AI_PATHING_DEBUG___
			NPC_ClearPathData(NPC);
			ucmd->forwardmove = 0;
			ucmd->rightmove = 0;
			ucmd->upmove = 0;
			NPC_PickRandomIdleAnimantion(NPC);
			return qtrue; // next think...
		}

		NPC->wpTravelTime = level.time + 15000;
		NPC->wpSeenTime = level.time;
	}

	NPC_FacePosition(aiEnt, gWPArray[NPC->wpCurrent]->origin, qfalse);
	NPC->s.angles[PITCH] = NPC->client->ps.viewangles[PITCH] = 0; // Init view PITCH angle so we always look forward, not down or up...
	VectorSubtract(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->movedir);

	if (NPC_DoLiftPathing(NPC))
	{
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		ucmd->upmove = 0;
		NPC_PickRandomIdleAnimantion(NPC);
		return qtrue;
	}

	if (VectorLength(NPC->client->ps.velocity) < 8 && NPC_RoutingJumpWaypoint(NPC->wpLast, NPC->wpCurrent))
	{// We need to jump to get to this waypoint...
		if (NPC_Jump(NPC, gWPArray[NPC->wpCurrent]->origin))
		{
			//trap->Print("NPC JUMP DEBUG: NPC_FollowRoutes\n");
			VectorCopy(NPC->movedir, NPC->client->ps.moveDir);
			return qtrue;
		}
	}
	else if (VectorLength(NPC->client->ps.velocity) < 8)
	{// If this is a new waypoint, we may need to jump to it...
		NPC_NewWaypointJump(aiEnt);
	}

	if (NPC_IsCivilian(NPC))
	{
		if (NPC->npc_cower_runaway)
		{// A civilian running away from combat...
			if (!UQ1_UcmdMoveForDir(NPC, &aiEnt->client->pers.cmd, NPC->movedir, qfalse, gWPArray[NPC->wpCurrent]->origin))
			{
				if (aiEnt->client->pers.cmd.forwardmove == 0 && aiEnt->client->pers.cmd.rightmove == 0 && aiEnt->client->pers.cmd.upmove == 0)
					NPC_PickRandomIdleAnimantion(NPC);
				else
					NPC_SelectMoveRunAwayAnimation(NPC);

				return qtrue;
			}

			if (aiEnt->client->pers.cmd.forwardmove == 0 && aiEnt->client->pers.cmd.rightmove == 0 && aiEnt->client->pers.cmd.upmove == 0)
				NPC_PickRandomIdleAnimantion(NPC);
			else
				NPC_SelectMoveRunAwayAnimation(NPC);

			VectorCopy(NPC->movedir, NPC->client->ps.moveDir);

			return qtrue;
		}
		else if (NPC_IsCivilianHumanoid(NPC))
		{// Civilian humanoid... Force walk/run anims...
			if (NPC_PointIsMoverLocation(gWPArray[NPC->wpCurrent]->origin))
			{// When nearby a mover, run!
				if (!UQ1_UcmdMoveForDir(NPC, ucmd, NPC->movedir, qfalse, gWPArray[NPC->wpCurrent]->origin))
				{
					if (aiEnt->client->pers.cmd.forwardmove == 0 && aiEnt->client->pers.cmd.rightmove == 0 && aiEnt->client->pers.cmd.upmove == 0)
						NPC_PickRandomIdleAnimantion(NPC);
					else
						NPC_SelectMoveAnimation(aiEnt, qfalse);

					return qtrue;
				}

				if (aiEnt->client->pers.cmd.forwardmove == 0 && aiEnt->client->pers.cmd.rightmove == 0 && aiEnt->client->pers.cmd.upmove == 0)
					NPC_PickRandomIdleAnimantion(NPC);
				else
					NPC_SelectMoveAnimation(aiEnt, qtrue); // UQ1: Always set civilian walk animation...
			}
			else if (!UQ1_UcmdMoveForDir(NPC, ucmd, NPC->movedir, qtrue, gWPArray[NPC->wpCurrent]->origin))
			{
				if (aiEnt->client->pers.cmd.forwardmove == 0 && aiEnt->client->pers.cmd.rightmove == 0 && aiEnt->client->pers.cmd.upmove == 0)
					NPC_PickRandomIdleAnimantion(NPC);
				else
					NPC_SelectMoveAnimation(aiEnt, qtrue);

				return qtrue;
			}

			if (aiEnt->client->pers.cmd.forwardmove == 0 && aiEnt->client->pers.cmd.rightmove == 0 && aiEnt->client->pers.cmd.upmove == 0)
				NPC_PickRandomIdleAnimantion(NPC);
			else
				NPC_SelectMoveAnimation(aiEnt, qtrue);
		}
		else
		{// Civilian non-humanoid... let bg_ set anim...
			return qtrue;
		}
	}
	else if (g_gametype.integer == GT_WARZONE || (NPC->r.svFlags & SVF_BOT))
	{
		if (!UQ1_UcmdMoveForDir(NPC, ucmd, NPC->movedir, qtrue, gWPArray[NPC->wpCurrent]->origin))
		{
			ucmd->forwardmove = 0;
			ucmd->rightmove = 0;
			ucmd->upmove = 0;
			NPC_PickRandomIdleAnimantion(NPC);
			return qtrue;
		}
	}
	else
	{
		if (!UQ1_UcmdMoveForDir(NPC, ucmd, NPC->movedir, qtrue, gWPArray[NPC->wpCurrent]->origin))
		{
			if (NPC->bot_strafe_jump_timer > level.time)
			{
				// Switch to running whenever jumping... Check collision when we failed the first move...
				UQ1_UcmdMoveForDir(NPC, ucmd, NPC->movedir, qfalse, gWPArray[NPC->wpCurrent]->origin);

				ucmd->upmove = 127;

				if (NPC->s.eType == ET_PLAYER)
				{
					trap->EA_Jump(NPC->s.number);
				}
			}
			else if (NPC->bot_strafe_left_timer > level.time)
			{
				ucmd->rightmove = -127;
				trap->EA_MoveLeft(NPC->s.number);
			}
			else if (NPC->bot_strafe_right_timer > level.time)
			{
				ucmd->rightmove = 127;
				trap->EA_MoveRight(NPC->s.number);
			}

			if (NPC->last_move_time < level.time - 2000)
			{
				ucmd->upmove = 127;

				if (NPC->s.eType == ET_PLAYER)
				{
					trap->EA_Jump(NPC->s.number);
				}
			}
		}
		else if (NPC->bot_strafe_jump_timer > level.time)
		{
			// Switch to running whenever jumping... No need to check collision...
			UQ1_UcmdMoveForDir_NoAvoidance(NPC, ucmd, NPC->movedir, qfalse, gWPArray[NPC->wpCurrent]->origin);

			ucmd->upmove = 127;

			if (NPC->s.eType == ET_PLAYER)
			{
				trap->EA_Jump(NPC->s.number);
			}
		}
		else if (NPC->bot_strafe_left_timer > level.time)
		{
			ucmd->rightmove = -127;
			trap->EA_MoveLeft(NPC->s.number);
		}
		else if (NPC->bot_strafe_right_timer > level.time)
		{
			ucmd->rightmove = 127;
			trap->EA_MoveRight(NPC->s.number);
		}

		if (NPC->last_move_time < level.time - 4000)
		{
			NPC_ClearPathData(NPC);
			
			ucmd->forwardmove = 0;
			ucmd->rightmove = 0;
			ucmd->upmove = 0;
			NPC_PickRandomIdleAnimantion(NPC);
			return qtrue;
		}

		if (NPC->last_move_time < level.time - 2000)
		{
			// Switch to running whenever jumping... No need to check collision...
			UQ1_UcmdMoveForDir_NoAvoidance(NPC, ucmd, NPC->movedir, qfalse, gWPArray[NPC->wpCurrent]->origin);

			ucmd->upmove = 127;

			if (NPC->s.eType == ET_PLAYER)
			{
				trap->EA_Jump(NPC->s.number);
			}
		}
	}

	VectorCopy(NPC->movedir, NPC->client->ps.moveDir);

	return qtrue;
}
#endif