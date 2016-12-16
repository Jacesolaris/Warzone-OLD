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


#if 0
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
	gentity_t *NPC = NPCS.NPC;

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
	gentity_t	*NPC = NPCS.NPC;
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

	NPC_FacePosition( gWPArray[NPC->wpCurrent]->origin, qfalse );
	VectorSubtract( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->movedir );
	UQ1_UcmdMoveForDir( NPC, &NPCS.ucmd, NPC->movedir, qtrue, gWPArray[NPC->wpCurrent]->origin );
	VectorCopy( NPC->movedir, NPC->client->ps.moveDir );

	if (NPCS.ucmd.forwardmove == 0 && NPCS.ucmd.rightmove == 0 && NPCS.ucmd.upmove == 0)
		NPC_PickRandomIdleAnimantion(NPC);
	else
		NPC_SelectMoveAnimation(qtrue);

	return qtrue;
}
#else
qboolean NPC_PatrolArea( void ) 
{// Quick method of patroling... The fastest way possible. Single waypoint using it's neighbours...
	gentity_t	*NPC = NPCS.NPC;

	if (gWPNum <= 0)
	{// No waypoints available...
		NPC_PickRandomIdleAnimantion(NPC);
		return qfalse;
	}

	if (NPC->noWaypointTime > level.time)
	{// Wait...
		NPC_PickRandomIdleAnimantion(NPC);
		return qtrue;
	}

	if (NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// Should only ever happen once...
		NPC->longTermGoal = DOM_GetNearestWP(NPC->r.currentOrigin, -1);
	}

	if (NPC->longTermGoal < 0)
	{
		return qfalse;
	}

	if (gWPArray[NPC->longTermGoal]->neighbornum <= 0)
	{// This wp happens to have no neighbours.. Just stand idle...
		NPC_PickRandomIdleAnimantion(NPC);
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
		return qfalse; // next think...
	}

	NPC_FacePosition( gWPArray[NPC->wpCurrent]->origin, qfalse );
	VectorSubtract( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->movedir );
	UQ1_UcmdMoveForDir_NoAvoidance/*UQ1_UcmdMoveForDir*/( NPC, &NPCS.ucmd, NPC->movedir, qtrue, gWPArray[NPC->wpCurrent]->origin );
	VectorCopy( NPC->movedir, NPC->client->ps.moveDir );

	if (NPCS.ucmd.forwardmove == 0 && NPCS.ucmd.rightmove == 0 && NPCS.ucmd.upmove == 0)
		NPC_PickRandomIdleAnimantion(NPC);
	else
		NPC_SelectMoveAnimation(qtrue);

	return qtrue;
}
#endif