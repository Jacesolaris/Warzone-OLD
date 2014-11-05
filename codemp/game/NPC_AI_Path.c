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

extern vmCvar_t npc_pathing;

// Recorded in g_mover.c
extern vec3_t		MOVER_LIST[1024];
extern vec3_t		MOVER_LIST_TOP[1024];
extern int			MOVER_LIST_NUM;

/*///////////////////////////////////////////////////
NPC_GetNextNode
if the bot has reached a node, this function selects the next node
that he will go to, and returns it
right now it's being developed, feel free to experiment
*////////////////////////////////////////////////////

int NPC_GetNextNode(gentity_t *NPC)
{
	short int node = WAYPOINT_NONE;

	//we should never call this in BOTSTATE_MOVE with no goal
	//setup the goal/path in HandleIdleState
	if (NPC->longTermGoal == WAYPOINT_NONE)
	{
		return WAYPOINT_NONE;
	}

	if (NPC->pathsize <= 0)	//if the bot is at the end of his path, this shouldn't have been called
	{
		//NPC->longTermGoal = WAYPOINT_NONE;	//reset to having no goal
		return WAYPOINT_NONE;
	}

	if (NPC->npc_cower_runaway || NPC->isPadawan)
		NPC_ShortenPath(NPC); // Shorten any path we can, if we are running away from combat...

	if (NPC->pathsize <= 0)	//if the bot is at the end of his path, this shouldn't have been called
	{
		//NPC->longTermGoal = WAYPOINT_NONE;	//reset to having no goal
		return WAYPOINT_NONE;
	}

	node = NPC->pathlist[NPC->pathsize-1];	//pathlist is in reverse order
	NPC->pathsize--;	//mark that we've moved another node

	if (NPC->pathsize <= 0)
	{
		if (NPC->wpCurrent < 0)
		{
			NPC->wpCurrent = NPC->longTermGoal;
		}
		else
		{
			node = NPC->longTermGoal;
		}
	}
	return node;
}

qboolean NPC_ShortenJump(gentity_t *NPC, int node)
{
	float MAX_JUMP_DISTANCE = 192.0;
	float dist = Distance(gWPArray[node]->origin, NPC->r.currentOrigin);
	
	if (NPC_IsJetpacking(NPC)) return qfalse;

	if (NPC_IsJedi(NPC)) MAX_JUMP_DISTANCE = 512.0; // Jedi can jump further...

	if (dist <= MAX_JUMP_DISTANCE
		&& NPC_TryJump( NPC, gWPArray[node]->origin ))
	{// Looks like we can jump there... Let's do that instead of failing!
		//trap->Print("%s is shortening path using jump.\n", NPC->client->pers.netname);
		return qtrue; // next think...
	}
	else if (dist <= MAX_JUMP_DISTANCE && NPC_RoutingSimpleJump( NPC->wpLast, NPC->wpCurrent ))
	{// UQ1: Testing new jump...
		return qtrue;
	}

	return qfalse;
}

void NPC_ShortenPath(gentity_t *NPC)
{
	qboolean	found = qfalse;
	int			position = -1;
	int			x = 0;

	for (x = 0; x < gWPArray[NPC->wpCurrent]->neighbornum; x++)
	{
		if (gWPArray[NPC->wpCurrent]->neighbors[x].num == NPC->longTermGoal)
		{// Our current wp links direct to our goal!
			//int shortenedBy = NPC->pathsize;
			//trap->Print("%s found a shorter (direct to goal! - shortened by %i) path.\n", NPC->NPC_type, shortenedBy);
			NPC->pathsize = 0;
			found = qtrue;
			break;
		}

		for (position = 0; position < NPC->pathsize; position++)
		{
			if (gWPArray[NPC->wpCurrent]->neighbors[x].num == NPC->pathlist[position])
			{// The current wp links direct to this node!
				//int shortenedBy = NPC->pathsize - position;
				//trap->Print("%s found a shorter (shortened by %i) path.\n", NPC->NPC_type, shortenedBy);
				NPC->pathsize = position;
				found = qtrue;
				break;
			}

			if (NPC_ShortenJump(NPC, NPC->pathlist[position]))
			{// Can we jump to a position closer to the end goal?
				//int shortenedBy = NPC->pathsize - position;
				NPC->pathsize = position;
				found = qtrue;
				//trap->Print("%s found a shorter path using jump (shortened by %i).\n", NPC->NPC_type, shortenedBy);
				break;
			}
		}

		if (found) break;
	}
}

qboolean NPC_FindNewWaypoint( void )
{
	gentity_t	*NPC = NPCS.NPC;

	//if (NPC->noWaypointTime > level.time)
	//{// Only try to find a new waypoint every 5 seconds...
	//	NPC_PickRandomIdleAnimantion(NPC);
	//	return qfalse;
	//}

	// Try to find a visible waypoint first...
	//NPC->wpCurrent = DOM_GetRandomCloseVisibleWP(NPC, NPC->r.currentOrigin, NPC->s.number, -1);
	//NPC->wpCurrent = DOM_GetRandomCloseWP(NPC->r.currentOrigin, NPC->wpCurrent, -1);
	NPC->wpCurrent = DOM_GetNearestWP(NPC->r.currentOrigin, NPC->wpCurrent);
	//NPC->noWaypointTime = level.time + 3000; // 3 seconds before we try again... (it will run avoidance in the meantime)
	/*
	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum)
	{// Just select the closest, even if none are visible...
		NPC->wpCurrent = DOM_GetNearestWP(NPC->r.currentOrigin, NPC->wpCurrent);
	}
	*/

	//if (NPC->wpSeenTime < NPC->noWaypointTime)
	//	NPC->wpSeenTime = NPC->noWaypointTime; // also make sure we don't try to make a new route for the same length of time...

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum)
	{
		NPC_PickRandomIdleAnimantion(NPC);
		//G_Printf("NPC Waypointing Debug: NPC %i (%s) failed to find a waypoint for itself.", NPC->s.number, NPC->NPC_type);
		return qfalse; // failed... try again after som avoidance code...
	}

	return qtrue; // all good, we have a new waypoint...
}

void NPC_SetEnemyGoal( void )
{
	qboolean IS_COVERPOINT = qfalse;
	int			COVERPOINT_WP = -1;
	int			COVERPOINT_OFC_WP = -1;
	gentity_t	*NPC = NPCS.NPC;

	//if (NPC->wpSeenTime > level.time)
	//	return; // wait for next route creation...

	/*
	if (NPC->wpTravelTime < level.time)
		G_Printf("wp travel time\n");
	else 
		G_Printf("Bad wps (lt: %i) (ps: %i) (wc: %i) (wn: %i)\n", NPC->longTermGoal, NPC->pathsize, NPC->wpCurrent, NPC->wpNext);
	*/

	if (!NPC_FindNewWaypoint())
		return; // wait before trying to get a new waypoint...

	// UQ1: Gunner NPCs find cover...
	if (NPC->client->ps.weapon != WP_SABER)
	{// Should we find a cover point???
		if (NPC->enemy->wpCurrent <= 0 || NPC->enemy->wpCurrent < gWPNum)
		{// Find a new waypoint for them...
			//NPC->enemy->wpCurrent = DOM_GetRandomCloseVisibleWP(NPC->enemy, NPC->enemy->r.currentOrigin, NPC->enemy->s.number, -1);
			NPC->enemy->wpCurrent = DOM_GetNearestWP(NPC->enemy->r.currentOrigin, NPC->enemy->s.number);
		}

		if (NPC->enemy->wpCurrent > 0 
			&& NPC->enemy->wpCurrent < gWPNum
			&& Distance(gWPArray[NPC->enemy->wpCurrent]->origin, NPC->enemy->r.currentOrigin) <= 256)
		{
			/*int i = 0;

			for (i = 0; i < num_cover_spots; i++)
			{
				qboolean BAD = qfalse;

				if (Distance(NPC->r.currentOrigin, gWPArray[cover_nodes[i]]->origin) <= 2048.0f
					&& Distance(NPC->enemy->r.currentOrigin, gWPArray[cover_nodes[i]]->origin) <= 2048.0f)
				{// Range looks good from both places...
					int thisWP = cover_nodes[i];
					
					// OK, looking good so far... Let's see how the visibility is...
					if (NPC_IsCoverpointFor(thisWP, NPC->enemy))
					{// Looks good for a cover point...
						int j = 0;
						int z = 0;

						for (z = 0; z < MAX_GENTITIES; z++)
						{// Now just check to make sure noone else is using it... 30 stormies behind a barrel anyone???
							gentity_t *ent = &g_entities[z];

							if (!ent) continue;
							if (!ent->inuse) continue;

							if (ent->coverpointGoal == thisWP
								|| ent->wpCurrent == thisWP
								|| ent->wpNext == thisWP)
							{// Meh, he already claimed it!
								BAD = qtrue;
								break;
							}
						}

						// Twas a stormie barrel... *sigh*
						if (BAD) continue;

						// So far, so good... Now check if a link from it can see the enemy.. (to dip in and out of cover to/from)
						for (j = 0; j < gWPArray[thisWP]->neighbornum; j++)
						{
							int lookWP = gWPArray[thisWP]->neighbors[j].num;

							if (!NPC_IsCoverpointFor(lookWP, NPC->enemy))
							{// Yes! Found one!
								COVERPOINT_WP = thisWP;
								COVERPOINT_OFC_WP = lookWP;
								IS_COVERPOINT = qtrue;
								break;
							}
						}

						if (IS_COVERPOINT) break; // We got one!
					}
				}

				if (IS_COVERPOINT) break; // We got one!
			}

			if (IS_COVERPOINT)
			{// WooHoo!!!! We got one! *dance*
				NPC->longTermGoal = NPC->coverpointGoal = COVERPOINT_WP;
				NPC->coverpointOFC = COVERPOINT_OFC_WP;
			}*/

			if (NPC->longTermGoal <= 0)
			{// Fallback...
				//NPC->longTermGoal = DOM_GetRandomCloseVisibleWP(NPC->enemy, NPC->enemy->r.currentOrigin, NPC->enemy->s.number, -1);
				NPC->longTermGoal = DOM_GetNearestWP(NPC->enemy->r.currentOrigin, NPC->enemy->s.number);
			}
		}
		else
		{// Just head toward them....
			//NPC->longTermGoal = DOM_GetRandomCloseVisibleWP(NPC->enemy, NPC->enemy->r.currentOrigin, NPC->enemy->s.number, -1);
			NPC->longTermGoal = DOM_GetNearestWP(NPC->enemy->r.currentOrigin, NPC->enemy->s.number);
		}
	}
	else
	{
		//NPC->longTermGoal = DOM_GetRandomCloseVisibleWP(NPC->enemy, NPC->enemy->r.currentOrigin, NPC->enemy->s.number, -1);
		NPC->longTermGoal = DOM_GetNearestWP(NPC->enemy->r.currentOrigin, NPC->enemy->s.number);
	}

	if (NPC->longTermGoal > 0)
	{
		memset(NPC->pathlist, WAYPOINT_NONE, MAX_WPARRAY_SIZE);
		NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, qfalse);

		//if (NPC->pathsize <= 0) // Use the alternate (older) A* pathfinding code as alternative/fallback...
			//NPC->pathsize = DOM_FindIdealPathtoWP(NULL, NPC->wpCurrent, NPC->longTermGoal, -1, NPC->pathlist);
			//NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, qtrue);
		
		if (NPC->pathsize > 0)
		{
			/*
			if (NPC->enemy->s.eType == ET_PLAYER)
			{
				if (IS_COVERPOINT)
					G_Printf("NPC Waypointing Debug: NPC %i (%s) created a %i waypoint COVERPOINT path between waypoints %i and %i for enemy player %s.", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->client->pers.netname);
				else
					G_Printf("NPC Waypointing Debug: NPC %i (%s) created a %i waypoint path between waypoints %i and %i for enemy player %s.", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->client->pers.netname);
			}
			else
			{
				if (IS_COVERPOINT)
					G_Printf("NPC Waypointing Debug: NPC %i (%s) created a %i waypoint COVERPOINT path between waypoints %i and %i for enemy %s.", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->classname);
				else
					G_Printf("NPC Waypointing Debug: NPC %i (%s) created a %i waypoint path between waypoints %i and %i for enemy %s.", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->classname);
			}
			*/

			NPC->wpLast = NPC->wpCurrent;
			NPC->wpNext = NPC_GetNextNode(NPC);		//move to this node first, since it's where our path starts from

			//G_Printf("New: wps (lt: %i) (ps: %i) (wc: %i) (wn: %i)\n", NPC->longTermGoal, NPC->pathsize, NPC->wpCurrent, NPC->wpNext);

			if (NPC->client->ps.weapon == WP_SABER)
			{
				G_AddVoiceEvent( NPC, Q_irand( EV_JCHASE1, EV_JCHASE3 ), 15000 + irand(0, 30000) );
			}
			else
			{
				int choice = irand(0,13);

				switch (choice)
				{
				case 0:
					G_AddVoiceEvent( NPC, EV_OUTFLANK1, 15000 + irand(0, 30000) );
					break;
				case 1:
					G_AddVoiceEvent( NPC, EV_OUTFLANK2, 15000 + irand(0, 30000) );
					break;
				case 2:
					G_AddVoiceEvent( NPC, EV_CHASE1, 15000 + irand(0, 30000) );
					break;
				case 3:
					G_AddVoiceEvent( NPC, EV_CHASE2, 15000 + irand(0, 30000) );
					break;
				case 4:
					G_AddVoiceEvent( NPC, EV_CHASE3, 15000 + irand(0, 30000) );
					break;
				case 5:
					G_AddVoiceEvent( NPC, EV_COVER1, 15000 + irand(0, 30000) );
					break;
				case 6:
					G_AddVoiceEvent( NPC, EV_COVER2, 15000 + irand(0, 30000) );
					break;
				case 7:
					G_AddVoiceEvent( NPC, EV_COVER3, 15000 + irand(0, 30000) );
					break;
				case 8:
					G_AddVoiceEvent( NPC, EV_COVER4, 15000 + irand(0, 30000) );
					break;
				case 9:
					G_AddVoiceEvent( NPC, EV_COVER5, 15000 + irand(0, 30000) );
					break;
				case 10:
					G_AddVoiceEvent( NPC, EV_ESCAPING1, 15000 + irand(0, 30000) );
					break;
				case 11:
					G_AddVoiceEvent( NPC, EV_ESCAPING2, 15000 + irand(0, 30000) );
					break;
				case 12:
					G_AddVoiceEvent( NPC, EV_ESCAPING3, 15000 + irand(0, 30000) );
					break;
				default:
					G_AddVoiceEvent( NPC, EV_COVER5, 15000 + irand(0, 30000) );
					break;
				}
			}
		}
		else if (NPC->enemy->s.eType == ET_PLAYER)
		{
			//G_Printf("NPC Waypointing Debug: NPC %i (%s) failed to create a route between waypoints %i and %i for enemy player %s.", NPC->s.number, NPC->NPC_type, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->client->pers.netname);
			NPC->longTermGoal = NPC->coverpointOFC = NPC->coverpointGoal = -1;
			// Delay before next route creation...
			NPC->wpSeenTime = level.time + 2000;
			return;
		}
	}
	else
	{
		//if (NPC->enemy->s.eType == ET_PLAYER)
		//	G_Printf("NPC Waypointing Debug: NPC %i (%s) failed to find a waypoint for enemy player %s.", NPC->s.number, NPC->NPC_type, NPC->enemy->client->pers.netname);

		NPC->longTermGoal = NPC->coverpointOFC = NPC->coverpointGoal = -1;

		// Delay before next route creation...
		NPC->wpSeenTime = level.time + 2000;
		return;
	}

	// Delay before next route creation...
	NPC->wpSeenTime = level.time + 2000;
	// Delay before giving up on this new waypoint/route...
	NPC->wpTravelTime = level.time + 10000;
}

qboolean NPC_CopyPathFromNearbyNPC( void )
{
	gentity_t	*NPC = NPCS.NPC;
	int i = 0;

	for (i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
	{
		gentity_t *test = &g_entities[i];

		if (i == NPC->s.number) continue;
		if (!test) continue;
		if (!test->inuse) continue;
		if (test->s.eType != ET_NPC) continue;
		if (test->pathsize <= 0) continue;
		if (test->client->NPC_class != NPC->client->NPC_class) continue; // Only copy from same NPC classes???
		if (Distance(NPC->r.currentOrigin, test->r.currentOrigin) > 128) continue;
		if (test->wpCurrent <= 0) continue;
		if (test->longTermGoal <= 0) continue;
		if (test->npc_dumb_route_time > level.time) continue;
		
		// Don't let them be copied again for 2 seconds...
		test->npc_dumb_route_time = level.time + 2000;

		// Seems we found one!
		memcpy(NPC->pathlist, test->pathlist, sizeof(int)*test->pathsize);
		NPC->pathsize = test->pathsize;
		NPC->wpCurrent = test->wpCurrent;
		NPC->wpNext = test->wpNext;
		NPC->wpLast = test->wpLast;
		NPC->longTermGoal = test->longTermGoal;
		
		// Delay before next route creation...
		NPC->wpSeenTime = level.time + 2000;
		// Delay before giving up on this new waypoint/route...
		NPC->wpTravelTime = level.time + 10000;
		
		// Don't let me be copied for 5 seconds...
		NPC->npc_dumb_route_time = level.time + 5000;

		//G_Printf("NPC Waypointing Debug: NPC %i (%s) copied a %i waypoint path between waypoints %i and %i from %i (%s).", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, test->s.number, test->NPC_type);
		return qtrue;
	}

	return qfalse;
}

int NPC_FindGoal( gentity_t *NPC )
{
	int waypoint = irand(0, gWPNum-1);
	int tries = 0;

	while (gWPArray[waypoint]->inuse == qfalse || gWPArray[waypoint]->wpIsBad == qtrue)
	{
		if (tries > 10) return -1; // Try again next frame...

		waypoint = irand(0, gWPNum-1);
		tries++;
	}

	return waypoint;
}

int NPC_FindTeamGoal( gentity_t *NPC )
{
	int waypoint = -1;
	int i;
	
	for ( i = 0; i < MAX_GENTITIES; i++)
	{
		gentity_t *ent = &g_entities[i];

		if (!ent) continue;
		if (ent == NPC) continue;
		if (ent->s.eType != ET_NPC && ent->s.eType != ET_PLAYER) continue;
		if (!ent->client) continue;
		if (!NPC_ValidEnemy(ent)) continue;

		if (ent->s.eType == ET_PLAYER)
		{
			if (ent->wpCurrent < 0 || ent->wpCurrent >= gWPNum
				|| Distance(ent->r.currentOrigin, gWPArray[ent->wpCurrent]->origin) > 128.0)
			{// Their current waypoint is invalid. Find one for them...
				//ent->wpCurrent = DOM_GetRandomCloseVisibleWP(ent, ent->r.currentOrigin, ent->s.number, -1);
				ent->wpCurrent = DOM_GetNearestWP(ent->r.currentOrigin, ent->wpCurrent);
			}
		}

		if (ent->wpCurrent <= 0 || ent->wpCurrent >= gWPNum) continue;

		// Looks ok...
		waypoint = ent->wpCurrent;
		//strcpy(enemy_name, ent->NPC_type);
		break;
	}

	if (waypoint < 0) 
	{// If nothing found then wander...
		waypoint = NPC_FindGoal( NPC );
		//trap->Print("%s failed to find enemy goal.\n", NPC->NPC_type);
	}
	else
	{
		//trap->Print("%s found enemy (%s) goal.\n", NPC->NPC_type, enemy_name);
	}

	return waypoint;
}

void NPC_SetNewGoalAndPath( void )
{
	gentity_t	*NPC = NPCS.NPC;
	qboolean	padawanPath = qfalse;

	//if (NPC->client->NPC_class != CLASS_TRAVELLING_VENDOR)
	//	if (NPC_CopyPathFromNearbyNPC()) 
	//		return;

	if (NPC->wpSeenTime > level.time)
	{
		NPC_PickRandomIdleAnimantion(NPC);
		return; // wait for next route creation...
	}

	if (!NPC_FindNewWaypoint())
	{
		//trap->Print("Unable to find waypoint.\n");
		//player_die(NPC, NPC, NPC, 99999, MOD_CRUSH);
		return; // wait before trying to get a new waypoint...
	}

	if (NPC->isPadawan)
	{
		if (!NPC->parent || !NPC_IsAlive(NPC->parent))
		{
			NPC->parent = NULL;
			return; // wait...
		}

		if (NPC->parent && NPC_IsAlive(NPC->parent))
		{// Need a new path to our master...
			padawanPath = qtrue;
		}
	}

	//
	// First try preferred goal...
	//

	if (NPC->return_home)
	{// Returning home...
		//NPC->longTermGoal = DOM_GetRandomCloseVisibleWP(NPC, NPC->spawn_pos, NPC->s.number, -1);
		NPC->longTermGoal = DOM_GetNearestWP(NPC->spawn_pos, NPC->wpCurrent);
	}
	else
	{// Find a new generic goal...
		if (g_gametype.integer >= GT_TEAM)
		{
			trap->Cvar_Update(&npc_pathing);

			if (padawanPath) 
				NPC->longTermGoal = NPC_FindPadawanGoal( NPC );
			else if (npc_pathing.integer == 1 && irand(0,5) == 0) // 1 in 6 will head straight to the enemy... When npc_pathing == 2, all NPCs head to random spots...
				NPC->longTermGoal = NPC_FindTeamGoal( NPC );
			else // 5 out of every 6 will use a totally random spot to spread them out... When npc_pathing == 2, all NPCs head to random spots...
				NPC->longTermGoal = NPC_FindGoal( NPC );
		}
		else
		{
			NPC->longTermGoal = NPC_FindGoal( NPC );
		}
	}

	if (NPC->longTermGoal >= 0)
	{
		memset(NPC->pathlist, WAYPOINT_NONE, sizeof(int)*MAX_WPARRAY_SIZE);
		NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, (qboolean)irand(0,1));

		if (NPC->pathsize > 0)
		{
			//trap->Print("NPC Waypointing Debug: NPC %i created a %i waypoint path for a random goal between waypoints %i and %i.\n", NPC->s.number, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal);
			NPC->wpLast = -1;
			NPC->wpNext = NPC_GetNextNode(NPC);		//move to this node first, since it's where our path starts from

			// Delay before next route creation...
			NPC->wpSeenTime = level.time + 1000;//30000;
			// Delay before giving up on this new waypoint/route...
			NPC->wpTravelTime = level.time + 10000;
			return;
		}
	}

	//
	// We failed - Pick a random goal to reduce calls to this function...
	//

	NPC->longTermGoal = NPC_FindGoal( NPC );

	if (NPC->longTermGoal >= 0)
	{
		memset(NPC->pathlist, WAYPOINT_NONE, sizeof(int)*MAX_WPARRAY_SIZE);
		NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, (qboolean)irand(0,1));

		if (NPC->pathsize > 0)
		{
			//trap->Print("NPC Waypointing Debug: NPC %i created a %i waypoint path for a random goal between waypoints %i and %i.\n", NPC->s.number, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal);
			NPC->wpLast = -1;
			NPC->wpNext = NPC_GetNextNode(NPC);		//move to this node first, since it's where our path starts from
			// Delay before next route creation...
			NPC->wpSeenTime = level.time + 1000;//30000;
			// Delay before giving up on this new waypoint/route...
			NPC->wpTravelTime = level.time + 10000;
			return;
		}
		else
		{
			//trap->Print("NPC Waypointing Debug: NPC %i failed to create a route between waypoints %i and %i.\n", NPC->s.number, NPC->wpCurrent, NPC->longTermGoal);
			// Delay before next route creation...
			NPC->wpSeenTime = level.time + 1000;//30000;
			NPC_PickRandomIdleAnimantion(NPC);
			return;
		}
	}
	else
	{
		//trap->Print("NPC Waypointing Debug: NPC %i failed to create a route between waypoints %i and %i.\n", NPC->s.number, NPC->wpCurrent, NPC->longTermGoal);
		// Delay before next route creation...
		NPC->wpSeenTime = level.time + 1000;//30000;
		NPC_PickRandomIdleAnimantion(NPC);
		return;
	}
}

/*
void NPC_SetNewWarzoneGoalAndPath()
{
	gentity_t	*NPC = NPCS.NPC;

	//if (NPC->client->NPC_class == CLASS_TRAVELLING_VENDOR)
	//{
	//	NPC_SetNewGoalAndPath(); // Use normal waypointing...
	//	return;
	//}

	if (NPC->wpSeenTime > level.time)
	{
		NPC_PickRandomIdleAnimantion(NPC);
		return; // wait for next route creation...
	}

	if (!NPC_FindNewWaypoint())
	{
		return; // wait before trying to get a new waypoint...
	}

	// Find a new warzone goal...
	NPC->longTermGoal = NPC_FindWarzoneGoal( NPC );

	if (NPC->longTermGoal <= 0) // Backup - Find a new generic goal...
		NPC->longTermGoal = NPC_FindGoal( NPC );

	if (NPC->longTermGoal >= 0)
	{
		memset(NPC->pathlist, WAYPOINT_NONE, sizeof(int)*MAX_WPARRAY_SIZE);
		NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, (qboolean)irand(0,1));

		if (NPC->pathsize > 0)
		{
			//G_Printf("NPC Waypointing Debug: NPC %i created a %i waypoint path for a random goal between waypoints %i and %i.", NPC->s.number, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal);
			NPC->wpLast = -1;
			NPC->wpNext = NPC_GetNextNode(NPC);		//move to this node first, since it's where our path starts from
		}
		else
		{
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

		// Delay before next route creation...
		NPC->wpSeenTime = level.time + 1000;//30000;
		NPC_PickRandomIdleAnimantion(NPC);
		return;
	}

	// Delay before next route creation...
	NPC->wpSeenTime = level.time + 1000;//30000;
	// Delay before giving up on this new waypoint/route...
	//NPC->wpTravelTime = level.time + 10000;
}
*/

qboolean DOM_NPC_ClearPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum )
{
	trace_t	trace;
	vec3_t	/*start, end, dir,*/ org, destorg;
//	float	dist, drop;
//	float	i;

	//Offset the step height
	//vec3_t	mins = {-18, -18, -24};
	//vec3_t	mins = {-8, -8, -6};
	vec3_t	mins = {-10, -10, -6};
	//vec3_t	maxs = {18, 18, 48};
	//vec3_t	maxs = {8, 8, NPC->client->ps.crouchheight};
	//vec3_t	maxs = {8, 8, 16};
	vec3_t	maxs = {10, 10, 16};

	VectorCopy(NPC->s.origin, org);
	//org[2]+=STEPSIZE;
	org[2]+=16;

	VectorCopy(dest, destorg);
	//destorg[2]+=STEPSIZE;
	destorg[2]+=16;

	trap->Trace( &trace, org, NULL/*mins*/, NULL/*maxs*/, destorg, NPC->s.number, MASK_PLAYERSOLID/*NPC->clipmask*/, 0, 0, 0 );

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		//G_Printf("SOLID!\n");
		return qfalse;
	}

	/*
	if ( trace.fraction < 1.0f )
	{//hit something
		if ( (impactEntNum != ENTITYNUM_NONE && trace.entityNum == impactEntNum ))
		{//hit what we're going after
			//G_Printf("OK!\n");
			return qtrue;
		}
		else
		{
			//G_Printf("TRACE FAIL! - NPC %i hit entity %i (%s).\n", NPC->s.number, trace.entityNum, g_entities[trace.entityNum].classname);
			return qfalse;
		}
	}
	*/

	if ( trace.fraction < 1.0f )
		return qfalse;

	/*
	//otherwise, clear path in a straight line.  
	//Now at intervals of my size, go along the trace and trace down STEPSIZE to make sure there is a solid floor.
	VectorSubtract( dest, NPC->r.currentOrigin, dir );
	dist = VectorNormalize( dir );
	if ( dest[2] > NPC->r.currentOrigin[2] )
	{//going up, check for steps
		drop = STEPSIZE;
	}
	else
	{//going down or level, check for moderate drops
		drop = 64;
	}
	for ( i = NPC->r.maxs[0]*2; i < dist; i += NPC->r.maxs[0]*2 )
	{//FIXME: does this check the last spot, too?  We're assuming that should be okay since the enemy is there?
		VectorMA( NPC->r.currentOrigin, i, dir, start );
		VectorCopy( start, end );
		end[2] -= drop;
		trap->Trace( &trace, start, mins, NPC->r.maxs, end, NPC->s.number, NPC->clipmask );//NPC->r.mins?
		if ( trace.fraction < 1.0f || trace.allsolid || trace.startsolid )
		{//good to go
			continue;
		}
		G_Printf("FLOOR!\n");
		//no floor here! (or a long drop?)
		return qfalse;
	}*/
	//we made it!
	return qtrue;
}

qboolean NPC_PointIsMoverLocation( vec3_t org )
{// Never spawn near a mover location...
	int i = 0;

	for (i = 0; i < MOVER_LIST_NUM; i++)
	{
		if (DistanceHorizontal(org, MOVER_LIST[i]) >= 128.0) continue;

		return qtrue;
	}

	return qfalse;
}

void NPC_ClearPathData ( gentity_t *NPC )
{
	NPC->longTermGoal = -1;
	NPC->wpCurrent = -1;
	NPC->pathsize = -1;
	NPC->longTermGoal = NPC->coverpointOFC = NPC->coverpointGoal = -1;

	//NPC->wpSeenTime = 0;
}

qboolean NPC_RoutingJumpWaypoint ( int wpLast, int wpCurrent )
{
	int			link = 0;
	qboolean	found = qfalse;

	if (wpLast < 0 || wpLast > gWPNum) return qfalse;

	for (link = 0; link < gWPArray[wpLast]->neighbornum; link++)
	{
		if (gWPArray[wpLast]->neighbors[link].num == wpCurrent) 
		{// found it!
			found = qtrue;
			break;
		}
	}

	if (found && gWPArray[wpLast]->neighbors[link].forceJumpTo > 0)
	{
		return qtrue;
	}

	return qfalse;
}

qboolean NPC_RoutingIncreaseCost ( int wpLast, int wpCurrent )
{
	int			link = 0;
	qboolean	found = qfalse;

	if (wpLast < 0 || wpLast > gWPNum) return qfalse;
	if (wpCurrent < 0 || wpCurrent > gWPNum) return qfalse;

	for (link = 0; link < gWPArray[wpLast]->neighbornum; link++)
	{
		if (gWPArray[wpLast]->neighbors[link].num == wpCurrent) 
		{// found it!
			gWPArray[wpLast]->neighbors[link].cost *= 2;

			//gWPArray[wpLast]->neighbors[link].forceJumpTo = 1;

			if (gWPArray[wpLast]->neighbors[link].cost < 1) 
				gWPArray[wpLast]->neighbors[link].cost = 2;

			return qtrue;
		}
	}

	return qfalse;
}

int CheckForFuncAbove(vec3_t org, int ignore)
{
	gentity_t *fent;
	vec3_t under, org2;
	trace_t tr;

	VectorCopy(org, org2);
	org2[2]+=16.0;

	VectorCopy(org, under);

	under[2] += 16550;

	trap->Trace(&tr, org2, NULL, NULL, under, ignore, MASK_SOLID, qfalse, 0, 0);

	if (tr.fraction == 1)
	{
		return 0;
	}

	fent = &g_entities[tr.entityNum];

	if (!fent)
	{
		return 0;
	}

	if (strstr(fent->classname, "func_"))
	{
		if (tr.allsolid || Distance(tr.endpos, org) < 128) return 0; // Not above us... It's a door!

		return 1; //there's a func brush here
	}

	return 0;
}

//see if there's a func_* ent under the given pos.
//kind of badly done, but this shouldn't happen
//often.
int CheckForFunc(vec3_t org, int ignore)
{
	gentity_t *fent;
	vec3_t under;
	trace_t tr;

	VectorCopy(org, under);

	under[2] -= 64;

	trap->Trace(&tr, org, NULL, NULL, under, ignore, MASK_SOLID, qfalse, 0, 0);

	if (tr.fraction == 1)
	{
		return 0;
	}

	fent = &g_entities[tr.entityNum];

	if (!fent)
	{
		return 0;
	}

	if (strstr(fent->classname, "func_"))
	{
		return 1; //there's a func brush here
	}

	return 0;
}

int WaitingForNow(vec3_t goalpos)
{ //checks if the bot is doing something along the lines of waiting for an elevator to raise up
	vec3_t		xybot, xywp, a, goalpos2;
	qboolean	have_goalpos2 = qfalse;

	if (NPCS.NPC->wpCurrent < 0 || NPCS.NPC->wpCurrent >= gWPNum)
	{
		return 0;
	}

	if (NPCS.NPC->wpNext >= 0 && NPCS.NPC->wpNext < gWPNum)
	{
		VectorCopy(gWPArray[NPCS.NPC->wpNext]->origin, goalpos2);
		have_goalpos2 = qtrue;
	}

	if ((int)goalpos[0] != (int)gWPArray[NPCS.NPC->wpCurrent]->origin[0] ||
		(int)goalpos[1] != (int)gWPArray[NPCS.NPC->wpCurrent]->origin[1] ||
		(int)goalpos[2] != (int)gWPArray[NPCS.NPC->wpCurrent]->origin[2])
	{
		return 0;
	}

	if (CheckForFuncAbove(goalpos, NPCS.NPC->s.number) || (have_goalpos2 && CheckForFuncAbove(goalpos2, NPCS.NPC->s.number)))
	{// Squisher above alert!
		return 1;
	}

	VectorCopy(NPCS.NPC->r.currentOrigin, xybot);
	VectorCopy(gWPArray[NPCS.NPC->wpCurrent]->origin, xywp);

	xybot[2] = 0;
	xywp[2] = 0;

	VectorSubtract(xybot, xywp, a);

	if (VectorLength(a) < 16)
	{
		if (CheckForFunc(NPCS.NPC->r.currentOrigin, NPCS.NPC->s.number))
		{
			return 1; //we're probably standing on an elevator and riding up/down. Or at least we hope so.
		}
	}
	else if (VectorLength(a) < 64 && CheckForFunc(NPCS.NPC->r.currentOrigin, NPCS.NPC->s.number))
	{
		NPCS.NPC->useDebounceTime = level.time + 2000;
	}

	return 0;
}

qboolean NPC_HaveValidEnemy( void )
{
	gentity_t	*NPC = NPCS.NPC;
	return NPC_IsAlive(NPC->enemy);
}

qboolean NPC_FollowRoutes( void ) 
{// Quick method of following bot routes...
	gentity_t	*NPC = NPCS.NPC;
	usercmd_t	ucmd = NPCS.ucmd;
	float		wpDist = 0.0;
	qboolean	padawanPath = qfalse;

	NPCS.NPCInfo->combatMove = qtrue;

	if ( !NPC_HaveValidEnemy() )
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
			if ( NPC->client->enemyTeam != NPCTEAM_NEUTRAL )
			{
				NPC->enemy = NPC_PickEnemyExt( qtrue );

				if (NPC_HaveValidEnemy())
				{
					if (NPC->client->ps.weapon == WP_SABER)
						G_AddVoiceEvent( NPC, Q_irand( EV_JDETECTED1, EV_JDETECTED3 ), 15000 + irand(0, 30000) );
					else
					{
						G_AddVoiceEvent( NPC, Q_irand( EV_DETECTED1, EV_DETECTED5 ), 15000 + irand(0, 30000) );
					}

					//trap->Print("Found enemy!\n");
					return qfalse;
				}
			}
			break;
		}
	}

	G_ClearEnemy(NPC);

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

		//if (wpDist > 512) trap->Print("To far! (%f)\n", wpDist);
	}

#if 0
	if ( (NPC->wpCurrent >= 0 && NPC->wpCurrent < gWPNum && NPC->longTermGoal >= 0 && NPC->longTermGoal < gWPNum && wpDist <= 512)
		&& (NPC->wpSeenTime < level.time - 1000 || NPC->wpTravelTime < level.time || NPC->last_move_time < level.time - 1000) )
	{// Try this for 2 seconds before giving up...
		float MAX_JUMP_DISTANCE = 192.0;
		
		if (NPC_IsJedi(NPC)) MAX_JUMP_DISTANCE = 512.0; // Jedi can jump further...

		if (Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) <= MAX_JUMP_DISTANCE
			&& NPC_TryJump( NPC, gWPArray[NPC->wpCurrent]->origin ))
		{// Looks like we can jump there... Let's do that instead of failing!
			//trap->Print("%s is jumping to waypoint.\n", NPC->client->pers.netname);
			return qtrue; // next think...
		}
	}
#endif

	if (NPC->isPadawan)
	{
		if (NPC->nextPadawanWaypointThink < level.time)
		{
			if (NPC->parent
				&& (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum 
				|| NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum
				|| Distance(NPC->parent->r.currentOrigin, gWPArray[NPC->longTermGoal]->origin) > 128))
			{// Need a new path to our master...
				padawanPath = qtrue;
			}
			else if (NPC->parent 
				&& NPC->parent->wpCurrent >= 0 && NPC->parent->wpCurrent < gWPNum 
				&& Distance(NPC->parent->r.currentOrigin, gWPArray[NPC->parent->wpCurrent]->origin) > 128)
			{// Need a new path to our master...
				padawanPath = qtrue;
			}

			NPC->nextPadawanWaypointThink = level.time + 5000; // only look for a new route every 5 seconds..
		}
	}

	if (NPC->wpCurrent >= 0 
		&& NPC->wpCurrent < gWPNum
		&& WaitingForNow(gWPArray[NPC->wpCurrent]->origin))
	{// We are on a mover/lift/etc... Idle...
		ucmd.forwardmove = 0;
		ucmd.rightmove = 0;
		ucmd.upmove = 0;
		NPC_PickRandomIdleAnimantion(NPC);

		if (DistanceHorizontal(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 48)
		{// Most likely on an elevator... Allow hitting waypoints all the way up/down...
			NPC->wpLast = NPC->wpCurrent;
			NPC->wpCurrent = NPC->wpNext;
			NPC->wpNext = NPC_GetNextNode(NPC);
			NPC->wpSeenTime = level.time;
		}

		return qfalse; // next think...
	}

	/*if (NPC->wpSeenTime >= level.time - 5000
		&& NPC->wpCurrent >= 0 
		&& NPC->wpCurrent < gWPNum
		&& wpDist <= 512)
	{

	}
	else*/ if ( padawanPath
		|| NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum 
		|| NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum 
		|| wpDist > 512
		|| NPC->wpSeenTime < level.time - 5000
		|| NPC->wpTravelTime < level.time 
		|| NPC->last_move_time < level.time - 5000 )
	{// We hit a problem in route, or don't have one yet.. Find a new goal and path...
		//if (wpDist > 512) trap->Print("wpCurrent too far.\n");
		//if (NPC->wpSeenTime < level.time - 5000) trap->Print("wpSeenTime.\n");
		//if (NPC->wpTravelTime < level.time) trap->Print("wpTravelTime.\n");
		//if (NPC->last_move_time < level.time - 5000) trap->Print("last_move_time.\n");

		if (!padawanPath && (wpDist > 512 || NPC->wpTravelTime < level.time) )
		{
			NPC_RoutingIncreaseCost( NPC->wpLast, NPC->wpCurrent );
		}

		NPC_ClearPathData(NPC);
		NPC_SetNewGoalAndPath();

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
		return qfalse; // next think...
	}
	
	if (Distance(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) < 48
		|| (NPC->s.groundEntityNum != ENTITYNUM_NONE && DistanceHorizontal(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) < 48))

	{// We're at out goal! Find a new goal...
		//trap->Print("HIT GOAL!\n");
		NPC_ClearPathData(NPC);
		ucmd.forwardmove = 0;
		ucmd.rightmove = 0;
		ucmd.upmove = 0;
		NPC_PickRandomIdleAnimantion(NPC);
		return qfalse; // next think...
	}

#if 0
	if (wpDist > 58 
		&& DistanceHorizontal(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 48
		&& NPC_PointIsMoverLocation(gWPArray[NPC->wpCurrent]->origin))
	{// Most likely on an elevator... Allow hitting waypoints all the way up/down...
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
			return qfalse; // next think...
		}

		if (DistanceVertical(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) > 48)
		{
			// Wait idle...
			ucmd.forwardmove = 0;
			ucmd.rightmove = 0;
			ucmd.upmove = 0;
			NPC_PickRandomIdleAnimantion(NPC);

			NPC->wpTravelTime = level.time + 10000;
			NPC->wpSeenTime = level.time;
			return qtrue;
		}
		
		NPC->wpTravelTime = level.time + 10000;
		NPC->wpSeenTime = level.time;
	}
#endif

	if (wpDist < 48)
	{// At current node.. Pick next in the list...
		//trap->Print("HIT WP %i. Next WP is %i.\n", NPC->wpCurrent, NPC->wpNext);

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
			return qfalse; // next think...
		}

		NPC->wpTravelTime = level.time + 10000;
		NPC->wpSeenTime = level.time;
	}

	NPC_FacePosition( gWPArray[NPC->wpCurrent]->origin, qfalse );
	VectorSubtract( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->movedir );
	
	if (DistanceHorizontal(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 48
		&& wpDist > 96
		&& NPC_PointIsMoverLocation(gWPArray[NPC->wpCurrent]->origin))
	{// Most likely on an elevator... Idle...
		NPCS.ucmd.forwardmove = 0;
		NPCS.ucmd.rightmove = 0;
		NPCS.ucmd.upmove = 0;
		NPC_PickRandomIdleAnimantion(NPC);
		return qtrue;
	}

	if (NPC_RoutingJumpWaypoint( NPC->wpLast, NPC->wpCurrent ))
	{// We need to jump to get to this waypoint...
		/*if (NPC_Jump(NPC, gWPArray[NPC->wpCurrent]->origin))
		{
			VectorCopy( NPC->movedir, NPC->client->ps.moveDir );
			return qtrue;
		}
		else*/ if (NPC_RoutingSimpleJump( NPC->wpLast, NPC->wpCurrent ))
		{// UQ1: Testing new jump...
			return qtrue;
		}
	}

	if (NPC_IsCivilian(NPC))
	{
		if (NPC->npc_cower_runaway)
		{// A civilian running away from combat...
			if (!UQ1_UcmdMoveForDir( NPC, &NPCS.ucmd, NPC->movedir, qfalse, gWPArray[NPC->wpCurrent]->origin )) 
			{
				if (NPCS.ucmd.forwardmove == 0 && NPCS.ucmd.rightmove == 0 && NPCS.ucmd.upmove == 0)
					NPC_PickRandomIdleAnimantion(NPC);
				else
					NPC_SelectMoveRunAwayAnimation();

				return qtrue;
			}

			if (NPCS.ucmd.forwardmove == 0 && NPCS.ucmd.rightmove == 0 && NPCS.ucmd.upmove == 0)
				NPC_PickRandomIdleAnimantion(NPC);
			else
				NPC_SelectMoveRunAwayAnimation();

			VectorCopy( NPC->movedir, NPC->client->ps.moveDir );

			return qtrue;
		}
		else if (NPC_IsCivilianHumanoid(NPC))
		{// Civilian humanoid... Force walk/run anims...
			if (NPC_PointIsMoverLocation(gWPArray[NPC->wpCurrent]->origin))
			{// When nearby a mover, run!
				if (!UQ1_UcmdMoveForDir( NPC, &NPCS.ucmd, NPC->movedir, qfalse, gWPArray[NPC->wpCurrent]->origin )) 
				{ 
					if (NPCS.ucmd.forwardmove == 0 && NPCS.ucmd.rightmove == 0 && NPCS.ucmd.upmove == 0)
						NPC_PickRandomIdleAnimantion(NPC);
					else
						NPC_SelectMoveAnimation(qfalse);

					return qtrue; 
				}

				if (NPCS.ucmd.forwardmove == 0 && NPCS.ucmd.rightmove == 0 && NPCS.ucmd.upmove == 0)
					NPC_PickRandomIdleAnimantion(NPC);
				else
					NPC_SelectMoveAnimation(qtrue); // UQ1: Always set civilian walk animation...
			}
			else if (!UQ1_UcmdMoveForDir( NPC, &NPCS.ucmd, NPC->movedir, qtrue, gWPArray[NPC->wpCurrent]->origin )) 
			{
				if (NPCS.ucmd.forwardmove == 0 && NPCS.ucmd.rightmove == 0 && NPCS.ucmd.upmove == 0)
					NPC_PickRandomIdleAnimantion(NPC);
				else
					NPC_SelectMoveAnimation(qtrue);

				return qtrue;
			}

			if (NPCS.ucmd.forwardmove == 0 && NPCS.ucmd.rightmove == 0 && NPCS.ucmd.upmove == 0)
				NPC_PickRandomIdleAnimantion(NPC);
			else
				NPC_SelectMoveAnimation(qtrue);
		}
		else
		{// Civilian non-humanoid... let bg_ set anim...
			return qtrue;
		}
	}
	else if (g_gametype.integer == GT_WARZONE || (NPC->r.svFlags & SVF_BOT))
	{
		if (!UQ1_UcmdMoveForDir( NPC, &NPCS.ucmd, NPC->movedir, qfalse, gWPArray[NPC->wpCurrent]->origin )) 
		{ 
			return qtrue; 
		}
	}
	else 
	{
		qboolean walk = qtrue;

		if (NPC_HaveValidEnemy()) walk = qfalse;

		if (!UQ1_UcmdMoveForDir( NPC, &NPCS.ucmd, NPC->movedir, walk, gWPArray[NPC->wpCurrent]->origin )) 
		{
			return qtrue; 
		}
	}

	VectorCopy( NPC->movedir, NPC->client->ps.moveDir );

	return qtrue;
}

void NPC_SetNewEnemyGoalAndPath( void )
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

	//NPC->longTermGoal = DOM_GetRandomCloseVisibleWP(NPC, NPC->enemy->r.currentOrigin, NPC->s.number, -1);
	//NPC->longTermGoal = DOM_GetRandomCloseWP(NPCS.NPCInfo->goalEntity->r.currentOrigin, NPC->wpCurrent, -1);
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
	NPC->wpTravelTime = level.time + 10000;
}

extern int jediSpeechDebounceTime[TEAM_NUM_TEAMS];//used to stop several jedi AI from speaking all at once
extern int groupSpeechDebounceTime[TEAM_NUM_TEAMS];//used to stop several group AI from speaking all at once

qboolean NPC_FollowEnemyRoute( void ) 
{// Quick method of following bot routes...
	gentity_t	*NPC = NPCS.NPC;
	usercmd_t	ucmd = NPCS.ucmd;
	float		wpDist = 0.0;

	NPCS.NPCInfo->combatMove = qtrue;

	if ( !NPC_HaveValidEnemy() )
	{
		return qfalse;
	}

	if ((NPC->client->ps.weapon == WP_SABER || NPC->client->ps.weapon == WP_MELEE)
		&& Distance(NPC->r.currentOrigin, NPCS.NPCInfo->goalEntity->r.currentOrigin) <= 48)
	{// Close enough already... Don't move...
		//trap->Print("close!\n");
		return qfalse;
	}
	else if ( !(NPC->client->ps.weapon == WP_SABER || NPC->client->ps.weapon == WP_MELEE)
		&& NPC_ClearLOS4( NPCS.NPCInfo->goalEntity ))
	{// Already visible to shoot... Don't move...
		//trap->Print("close wp!\n");
		return qfalse;
	}

#ifdef __NPC_USE_SABER_BLOCKING__
	// Never block when travelling...
	//NPC->client->ps.powerups[PW_BLOCK] = 0;
	NPC->blockToggleTime = level.time + 250; // 250 ms between toggles...
#endif //__NPC_USE_SABER_BLOCKING__

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

#if 0
	if ( (NPC->wpCurrent >= 0 && NPC->wpCurrent < gWPNum && NPC->longTermGoal >= 0 && NPC->longTermGoal < gWPNum && wpDist <= 512)
		&& (NPC->wpSeenTime < level.time - 1000 || NPC->wpTravelTime < level.time || NPC->last_move_time < level.time - 1000) )
	{// Try this for 2 seconds before giving up...
		float MAX_JUMP_DISTANCE = 192.0;
		
		if (NPC_IsJedi(NPC)) MAX_JUMP_DISTANCE = 512.0; // Jedi can jump further...

		if (Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) <= MAX_JUMP_DISTANCE
			&& NPC_TryJump( NPC, gWPArray[NPC->wpCurrent]->origin ))
		{// Looks like we can jump there... Let's do that instead of failing!
			//trap->Print("%s is jumping to waypoint.\n", NPC->client->pers.netname);
			return qtrue; // next think...
		}
	}
#endif

	if (NPC->wpCurrent >= 0 
		&& NPC->wpCurrent < gWPNum
		&& WaitingForNow(gWPArray[NPC->wpCurrent]->origin))
	{// We are on a mover/lift/etc... Idle...
		ucmd.forwardmove = 0;
		ucmd.rightmove = 0;
		ucmd.upmove = 0;
		NPC_PickRandomIdleAnimantion(NPC);

		if (DistanceHorizontal(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 48)
		{// Most likely on an elevator... Allow hitting waypoints all the way up/down...
			NPC->wpLast = NPC->wpCurrent;
			NPC->wpCurrent = NPC->wpNext;
			NPC->wpNext = NPC_GetNextNode(NPC);
			NPC->wpSeenTime = level.time;
		}

		return qfalse; // next think...
	}

	/*if (NPC->wpSeenTime >= level.time - 5000
		&& NPC->wpCurrent >= 0 
		&& NPC->wpCurrent < gWPNum
		&& wpDist <= 512)
	{

	}
	else*/ if ( NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum 
		|| NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum 
		|| wpDist > 512
		|| NPC->wpSeenTime < level.time - 5000
		|| NPC->wpTravelTime < level.time 
		|| NPC->last_move_time < level.time - 5000 
		|| Distance(gWPArray[NPC->longTermGoal]->origin, NPC->enemy->r.currentOrigin) > 256.0)
	{// We hit a problem in route, or don't have one yet.. Find a new goal and path...

		if (wpDist > 512 || NPC->wpTravelTime < level.time )
		{
			NPC_RoutingIncreaseCost( NPC->wpLast, NPC->wpCurrent );
		}

		NPC_ClearPathData(NPC);
		NPC_SetNewEnemyGoalAndPath();
		G_ClearEnemy(NPC); // UQ1: Give up...

		if (NPC_IsJedi(NPCS.NPC))
		{
			if ( !Q_irand( 0, 10 ) && NPCS.NPCInfo->blockedSpeechDebounceTime < level.time && jediSpeechDebounceTime[NPCS.NPC->client->playerTeam] < level.time )
			{
				G_AddVoiceEvent( NPCS.NPC, Q_irand( EV_JLOST1, EV_JLOST3 ), 10000 );
				jediSpeechDebounceTime[NPCS.NPC->client->playerTeam] = NPCS.NPCInfo->blockedSpeechDebounceTime = level.time + 10000;
			}
		}
		else
		{
			if ( !Q_irand( 0, 10 ) && NPCS.NPCInfo->blockedSpeechDebounceTime < level.time && groupSpeechDebounceTime[NPCS.NPC->client->playerTeam] < level.time )
			{
				G_AddVoiceEvent( NPCS.NPC, Q_irand( EV_GIVEUP1, EV_GIVEUP4 ), 10000 );
				groupSpeechDebounceTime[NPCS.NPC->client->playerTeam] = NPCS.NPCInfo->blockedSpeechDebounceTime = level.time + 10000;
			}
		}

		if (!(NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum))
		{
			NPC->wpTravelTime = level.time + 10000;
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

	if (wpDist > 58 
		&& DistanceHorizontal(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 48
		&& NPC_PointIsMoverLocation(gWPArray[NPC->wpCurrent]->origin))
	{// Most likely on an elevator... Allow hitting waypoints all the way up/down...
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
			return qfalse; // next think...
		}

		if (DistanceVertical(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) > 48)
		{
			// Wait idle...
			ucmd.forwardmove = 0;
			ucmd.rightmove = 0;
			ucmd.upmove = 0;
			NPC_PickRandomIdleAnimantion(NPC);

			NPC->wpTravelTime = level.time + 10000;
			NPC->wpSeenTime = level.time;
			return qtrue;
		}
		
		NPC->wpTravelTime = level.time + 10000;
		NPC->wpSeenTime = level.time;
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

		NPC->wpTravelTime = level.time + 10000;
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

	if (DistanceHorizontal(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 48
		&& wpDist > 96
		&& NPC_PointIsMoverLocation(gWPArray[NPC->wpCurrent]->origin))
	{// Most likely on an elevator... Idle...
		NPCS.ucmd.forwardmove = 0;
		NPCS.ucmd.rightmove = 0;
		NPCS.ucmd.upmove = 0;
		NPC_PickRandomIdleAnimantion(NPC);
		return qtrue;
	}

	if (NPC_RoutingJumpWaypoint( NPC->wpLast, NPC->wpCurrent ))
	{// We need to jump to get to this waypoint...
		/*if (NPC_Jump(NPC, gWPArray[NPC->wpCurrent]->origin))
		{
			VectorCopy( NPC->movedir, NPC->client->ps.moveDir );
			return qtrue;
		}
		else*/ if (NPC_RoutingSimpleJump( NPC->wpLast, NPC->wpCurrent ))
		{// UQ1: Testing new jump...
			return qtrue;
		}
	}

	if (!UQ1_UcmdMoveForDir( NPC, &NPCS.ucmd, NPC->movedir, qfalse, gWPArray[NPC->wpCurrent]->origin )) { /*NPC_PickRandomIdleAnimantion(NPC);*/ return qtrue; }
	VectorCopy( NPC->movedir, NPC->client->ps.moveDir );
	//NPC_SelectMoveAnimation(qfalse);

	return qtrue;
}
