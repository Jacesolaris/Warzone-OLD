#include "b_local.h"
#include "g_nav.h"
#include "anims.h"

extern int gWPNum;
extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];

extern int GROUND_TIME[MAX_GENTITIES];

extern int DOM_GetNearestWP(vec3_t org, int badwp);
extern int NPC_GetNextNode(gentity_t *NPC);
extern qboolean UQ1_UcmdMoveForDir ( gentity_t *self, usercmd_t *cmd, vec3_t dir, qboolean walk, vec3_t dest );
extern qboolean Warzone_PointNearMoverEntityLocation( vec3_t org );

qboolean NPC_IsJetpacking ( gentity_t *self )
{
	if (NPCS.NPC->s.eFlags & EF_JETPACK_ACTIVE || NPCS.NPC->s.eFlags & EF_JETPACK_FLAMING || NPCS.NPC->s.eFlags & EF_JETPACK_HOVER)
		return qtrue;

	return qfalse;
}

qboolean NPC_JetpackFallingEmergencyCheck (gentity_t *NPC)
{
	trace_t		tr;
	vec3_t testPos, downPos;
	vec3_t mins, maxs;

	VectorSet(mins, -8, -8, -1);
	VectorSet(maxs, 8, 8, 1);
	
	VectorCopy(NPC->r.currentOrigin, testPos);
	VectorCopy(NPC->r.currentOrigin, downPos);

	downPos[2] -= 192.0;
	testPos[2] += 16.0;

	trap->Trace( &tr, testPos, mins, maxs, downPos, NPC->s.number, MASK_PLAYERSOLID, 0, 0, 0 );

	if (tr.entityNum != ENTITYNUM_NONE)
	{
		return qfalse;
	}
	else if (tr.fraction == 1.0f)
	{
		return qtrue;
	}

	return qfalse;
}

qboolean NPC_JetpackHeightCheck (gentity_t *NPC)
{
	int i;

	if (gWPNum <= 0)
	{// If we have no waypoints, then just accept this height...
		return qtrue;
	}

	if (NPC->npc_jetpack_height_last_check > level.time - 5000)
	{
		if (NPC->npc_jetpack_height[0] == 0 && NPC->npc_jetpack_height[1] == 0 && NPC->npc_jetpack_height[2] == 0)
		{// The last check failed... We need to go down...
			return qfalse;
		}
		else if (DistanceVertical(NPC->npc_jetpack_height, NPC->r.currentOrigin) < 384)
		{// A waypoint is in range... We are not too high off the ground...
			return qtrue;
		}
		else
		{// We are too high!
			return qfalse;
		}
	}

	if (NPC->wpCurrent >= 0 && NPC->wpCurrent < gWPNum)
	{// We have a waypoint, if in range of that, then let's skip the loop...
		if (DistanceVertical(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 384)
		{// All good!
			// Record this height so that we can skip this for loop for a while...
			NPC->npc_jetpack_height_last_check = level.time;
			VectorCopy(gWPArray[NPC->wpCurrent]->origin, NPC->npc_jetpack_height);
			return qtrue;
		}
	}

	// I could do similar by remberring the last-time-we-were-on-the-ground position and using that, but this is more accurate...
	for (i = 0; i < gWPNum; i++)
	{
		if (Distance(gWPArray[i]->origin, NPC->r.currentOrigin) < 512)
		{
			if (DistanceVertical(gWPArray[i]->origin, NPC->r.currentOrigin) < 384)
			{// A waypoint is in range... We are not too high off the ground...
				// Record this height so that we can skip this for loop for a while...
				NPC->npc_jetpack_height_last_check = level.time;
				VectorCopy(gWPArray[i]->origin, NPC->npc_jetpack_height);
				return qtrue; 
			}
		}
	}

	// So we can skip the check for a bit...
	NPC->npc_jetpack_height_last_check = level.time;
	VectorClear(NPC->npc_jetpack_height);

	// No waypoint was found that we are in range of... We need to go down!
	return qfalse;
}

void NPC_JetpackCombatThink ( void )
{
	gentity_t	*self = NPCS.NPC;
	usercmd_t	*ucmd = &NPCS.ucmd;

	if (!NPC_IsJetpacking(self) && !NPC_JetpackFallingEmergencyCheck(self))
	{// We have jetpack off and are falling... Turn it on and save yourself!
		ucmd->upmove = 127;
		self->client->ps.velocity[2] = 400;

		self->client->ps.eFlags |= EF_JETPACK_ACTIVE;
		self->client->ps.eFlags &= ~EF_JETPACK_HOVER;

		self->s.eFlags |= EF_JETPACK_ACTIVE;
		self->s.eFlags &= ~EF_JETPACK_HOVER;

		if (self->client->ps.velocity[2] > 100)
		{// Also hit the afterburner...
			self->client->ps.eFlags |= EF_JETPACK_FLAMING;
			self->s.eFlags |= EF_JETPACK_FLAMING;
		}
		else
		{// Turn off afterburner...
			self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
			self->s.eFlags &= ~EF_JETPACK_FLAMING;
		}

		self->client->ps.pm_type = PM_JETPACK;
	}
	else if (!NPC_JetpackHeightCheck(self))
	{// We are too high... Go down...
		ucmd->upmove = -50.0;

		self->client->ps.eFlags |= EF_JETPACK_HOVER;
		self->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
		self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
		self->s.eFlags |= EF_JETPACK_HOVER;
		self->s.eFlags &= ~EF_JETPACK_ACTIVE;
		self->s.eFlags &= ~EF_JETPACK_FLAMING;
		self->client->ps.pm_type = PM_JETPACK;
	}
	else if (self->r.currentOrigin[2] > self->enemy->r.currentOrigin[2] + 384)
	{// Have an enemy and we are too far above him...
		ucmd->upmove = -50;

		self->client->ps.eFlags |= EF_JETPACK_HOVER;
		self->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
		self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
		self->s.eFlags |= EF_JETPACK_HOVER;
		self->s.eFlags &= ~EF_JETPACK_ACTIVE;
		self->s.eFlags &= ~EF_JETPACK_FLAMING;
		self->client->ps.pm_type = PM_JETPACK;
	}
	else if (self->r.currentOrigin[2] < self->enemy->r.currentOrigin[2] + 128)
	{// Have an enemy and we are too far below him...
		ucmd->upmove = 50;

		self->client->ps.eFlags |= EF_JETPACK_ACTIVE;
		self->client->ps.eFlags &= ~EF_JETPACK_HOVER;

		self->s.eFlags |= EF_JETPACK_ACTIVE;
		self->s.eFlags &= ~EF_JETPACK_HOVER;

		if (self->client->ps.velocity[2] > 100)
		{// Also hit the afterburner...
			self->client->ps.eFlags |= EF_JETPACK_FLAMING;
			self->s.eFlags |= EF_JETPACK_FLAMING;
		}
		else
		{// Turn off afterburner...
			self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
			self->s.eFlags &= ~EF_JETPACK_FLAMING;
		}

		self->client->ps.pm_type = PM_JETPACK;
	}
	else if (self->client->ps.groundEntityNum != ENTITYNUM_WORLD
		&& self->client->ps.velocity[2] > 0
		&& GROUND_TIME[self->s.number] < level.time - 300) 
	{// Have jetpack and jumping, make sure jetpack is active...
		self->client->ps.eFlags |= EF_JETPACK_ACTIVE;
		self->client->ps.eFlags &= ~EF_JETPACK_HOVER;

		self->s.eFlags |= EF_JETPACK_ACTIVE;
		self->s.eFlags &= ~EF_JETPACK_HOVER;

		if (self->client->ps.velocity[2] > 100)
		{// Also hit the afterburner...
			self->client->ps.eFlags |= EF_JETPACK_FLAMING;
			self->s.eFlags |= EF_JETPACK_FLAMING;
		}
		else
		{// Turn off afterburner...
			self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
			self->s.eFlags &= ~EF_JETPACK_FLAMING;
		}

		self->client->ps.pm_type = PM_JETPACK;
	}
	else if (self->client->ps.pm_type == PM_JETPACK
		&& self->client->ps.groundEntityNum != ENTITYNUM_WORLD
		&& self->client->ps.velocity[2] < 0
		&& GROUND_TIME[self->s.number] < level.time - 300) 
	{// Hover at this height...
		self->client->ps.velocity[2] = 0;
		self->client->ps.eFlags |= EF_JETPACK_HOVER;
		self->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
		self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
		self->s.eFlags |= EF_JETPACK_HOVER;
		self->s.eFlags &= ~EF_JETPACK_ACTIVE;
		self->s.eFlags &= ~EF_JETPACK_FLAMING;
		self->client->ps.pm_type = PM_JETPACK;
	}
	else if ( GROUND_TIME[self->client->ps.clientNum] >= level.time )
	{// On the ground. Make sure jetpack is deactivated...
		self->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
		self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
		self->client->ps.eFlags &= ~EF_JETPACK_HOVER;
		self->s.eFlags &= ~EF_JETPACK_ACTIVE;
		self->s.eFlags &= ~EF_JETPACK_FLAMING;
		self->s.eFlags &= ~EF_JETPACK_HOVER;
		self->client->ps.pm_type = PM_NORMAL;
	}
}

void NPC_JetpackTravelThink ( void )
{
	gentity_t	*self = NPCS.NPC;
	usercmd_t	*ucmd = &NPCS.ucmd;

	if (self->wpCurrent >= 0 
		&& self->wpCurrent < gWPNum
		&& (Warzone_PointNearMoverEntityLocation(self->r.currentOrigin) || Warzone_PointNearMoverEntityLocation(gWPArray[self->wpCurrent]->origin)))
	{// We can use the jetpack for this instead of waiting...
		while ((self->wpCurrent >= 0 && self->wpCurrent < gWPNum) && Warzone_PointNearMoverEntityLocation(gWPArray[self->wpCurrent]->origin))
		{// Find the first waypoint in our path that is not near the mover to head to...
			self->wpLast = self->wpCurrent;
			self->wpCurrent = self->wpNext;
			self->wpNext = NPC_GetNextNode(self);
			self->wpSeenTime = level.time;
		}

		if (self->wpCurrent >= 0 && self->wpCurrent < gWPNum)
		{// And go there...
			NPC_FacePosition( gWPArray[self->wpCurrent]->origin, qfalse );
			VectorSubtract( gWPArray[self->wpCurrent]->origin, self->r.currentOrigin, self->movedir );
			UQ1_UcmdMoveForDir( self, ucmd, self->movedir, qfalse, gWPArray[self->wpCurrent]->origin );

			ucmd->upmove = 50.0;

			self->client->ps.eFlags |= EF_JETPACK_ACTIVE;
			self->client->ps.eFlags &= ~EF_JETPACK_HOVER;

			self->s.eFlags |= EF_JETPACK_ACTIVE;
			self->s.eFlags &= ~EF_JETPACK_HOVER;

			if (self->client->ps.velocity[2] > 100)
			{// Also hit the afterburner...
				self->client->ps.eFlags |= EF_JETPACK_FLAMING;
				self->s.eFlags |= EF_JETPACK_FLAMING;
			}
			else
			{// Turn off afterburner...
				self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
				self->s.eFlags &= ~EF_JETPACK_FLAMING;
			}

			self->client->ps.pm_type = PM_JETPACK;

			VectorCopy( self->movedir, self->client->ps.moveDir );

			if (DistanceHorizontal(gWPArray[self->wpCurrent]->origin, self->r.currentOrigin) > 64 
				|| !OrgVisible(self->r.currentOrigin, gWPArray[self->wpCurrent]->origin, self->s.number))
				return; // Make sure we go up until we are close...
		}
	}

	if (!NPC_IsJetpacking(self) && !NPC_JetpackFallingEmergencyCheck(self))
	{// We have jetpack off and are falling... Turn it on and save yourself!
		if (self->wpCurrent >= 0 && self->wpCurrent < gWPNum)
		{
			NPC_FacePosition( gWPArray[self->wpCurrent]->origin, qfalse );
			VectorSubtract( gWPArray[self->wpCurrent]->origin, self->r.currentOrigin, self->movedir );
			UQ1_UcmdMoveForDir( self, ucmd, self->movedir, qfalse, gWPArray[self->wpCurrent]->origin );
		}

		ucmd->upmove = 127;
		self->client->ps.velocity[2] = 400;

		self->client->ps.eFlags |= EF_JETPACK_ACTIVE;
		self->client->ps.eFlags &= ~EF_JETPACK_HOVER;

		self->s.eFlags |= EF_JETPACK_ACTIVE;
		self->s.eFlags &= ~EF_JETPACK_HOVER;

		if (self->client->ps.velocity[2] > 100)
		{// Also hit the afterburner...
			self->client->ps.eFlags |= EF_JETPACK_FLAMING;
			self->s.eFlags |= EF_JETPACK_FLAMING;
		}
		else
		{// Turn off afterburner...
			self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
			self->s.eFlags &= ~EF_JETPACK_FLAMING;
		}

		self->client->ps.pm_type = PM_JETPACK;

		if (self->wpCurrent >= 0 && self->wpCurrent < gWPNum)
		{
			VectorCopy( self->movedir, self->client->ps.moveDir );
		}

		return;
	}

	if (!(self->wpCurrent >= 0 && self->wpCurrent < gWPNum))
	{// No valid waypoint to go to... Find one...
		self->wpCurrent = DOM_GetNearestWP(self->r.currentOrigin, self->wpCurrent);
	}

	if (!(self->wpCurrent >= 0 && self->wpCurrent < gWPNum))
	{// Still no valid waypoint to go to... Give up...
		return;
	}
	
	if (GROUND_TIME[self->client->ps.clientNum] < level.time && NPC_IsJetpacking(self))
	{// Land at waypoint...
		if (DistanceHorizontal(gWPArray[self->wpCurrent]->origin, self->r.currentOrigin) < 24)
		{// We are directly above our waypoint... Land...
			NPC_FacePosition( gWPArray[self->wpCurrent]->origin, qfalse );
			VectorSubtract( gWPArray[self->wpCurrent]->origin, self->r.currentOrigin, self->movedir );
			UQ1_UcmdMoveForDir( self, ucmd, self->movedir, qfalse, gWPArray[self->wpCurrent]->origin );

			ucmd->upmove = -50.0;

			self->client->ps.eFlags |= EF_JETPACK_HOVER;
			self->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
			self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
			self->s.eFlags |= EF_JETPACK_HOVER;
			self->s.eFlags &= ~EF_JETPACK_ACTIVE;
			self->s.eFlags &= ~EF_JETPACK_FLAMING;
			self->client->ps.pm_type = PM_JETPACK;

			VectorCopy( self->movedir, self->client->ps.moveDir );
		}
		else if (gWPArray[self->wpCurrent]->origin[2]+64 < self->r.currentOrigin[2] || !NPC_JetpackHeightCheck(self))
		{// Our waypoint is below us... Go down...
			NPC_FacePosition( gWPArray[self->wpCurrent]->origin, qfalse );
			VectorSubtract( gWPArray[self->wpCurrent]->origin, self->r.currentOrigin, self->movedir );
			UQ1_UcmdMoveForDir( self, ucmd, self->movedir, qfalse, gWPArray[self->wpCurrent]->origin );

			ucmd->upmove = -50.0;

			self->client->ps.eFlags |= EF_JETPACK_HOVER;
			self->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
			self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
			self->s.eFlags |= EF_JETPACK_HOVER;
			self->s.eFlags &= ~EF_JETPACK_ACTIVE;
			self->s.eFlags &= ~EF_JETPACK_FLAMING;
			self->client->ps.pm_type = PM_JETPACK;

			VectorCopy( self->movedir, self->client->ps.moveDir );
		}
		else if (gWPArray[self->wpCurrent]->origin[2]+64 > self->r.currentOrigin[2])
		{// Our waypoint is above us... Go up...
			NPC_FacePosition( gWPArray[self->wpCurrent]->origin, qfalse );
			VectorSubtract( gWPArray[self->wpCurrent]->origin, self->r.currentOrigin, self->movedir );
			UQ1_UcmdMoveForDir( self, ucmd, self->movedir, qfalse, gWPArray[self->wpCurrent]->origin );

			ucmd->upmove = 50.0;

			self->client->ps.eFlags |= EF_JETPACK_ACTIVE;
			self->client->ps.eFlags &= ~EF_JETPACK_HOVER;

			self->s.eFlags |= EF_JETPACK_ACTIVE;
			self->s.eFlags &= ~EF_JETPACK_HOVER;

			if (self->client->ps.velocity[2] > 100)
			{// Also hit the afterburner...
				self->client->ps.eFlags |= EF_JETPACK_FLAMING;
				self->s.eFlags |= EF_JETPACK_FLAMING;
			}
			else
			{// Turn off afterburner...
				self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
				self->s.eFlags &= ~EF_JETPACK_FLAMING;
			}

			self->client->ps.pm_type = PM_JETPACK;

			VectorCopy( self->movedir, self->client->ps.moveDir );
		}
		else
		{// We are at the right height... We need to hover a little over it until in range...
			NPC_FacePosition( gWPArray[self->wpCurrent]->origin, qfalse );
			VectorSubtract( gWPArray[self->wpCurrent]->origin, self->r.currentOrigin, self->movedir );
			UQ1_UcmdMoveForDir( self, ucmd, self->movedir, qfalse, gWPArray[self->wpCurrent]->origin );

			ucmd->upmove = 0.0;

			self->client->ps.velocity[2] = 0;
			self->client->ps.eFlags |= EF_JETPACK_HOVER;
			self->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
			self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
			self->s.eFlags |= EF_JETPACK_HOVER;
			self->s.eFlags &= ~EF_JETPACK_ACTIVE;
			self->s.eFlags &= ~EF_JETPACK_FLAMING;

			self->client->ps.pm_type = PM_JETPACK;

			VectorCopy( self->movedir, self->client->ps.moveDir );
		}
	}
	else if ( GROUND_TIME[self->client->ps.clientNum] >= level.time || !NPC_IsJetpacking(self) )
	{// On the ground. Make sure jetpack is deactivated...
		self->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
		self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
		self->client->ps.eFlags &= ~EF_JETPACK_HOVER;
		self->s.eFlags &= ~EF_JETPACK_ACTIVE;
		self->s.eFlags &= ~EF_JETPACK_FLAMING;
		self->s.eFlags &= ~EF_JETPACK_HOVER;
		self->client->ps.pm_type = PM_NORMAL;
	}
}

void NPC_DoFlyStuff ( void )
{// UQ1's uber AI jetpack usage code...
	gentity_t	*self = NPCS.NPC;
	usercmd_t	*ucmd = &NPCS.ucmd;

	if (!self || !self->client || !NPC_IsAlive(self)) return;

	if (self->client->ps.groundEntityNum == ENTITYNUM_WORLD)
	{
		GROUND_TIME[self->s.number] = level.time;
	}

	if (self->client->ps.eFlags & EF_JETPACK)
	{
		qboolean HAVE_ENEMY = (qboolean)(self->enemy && NPC_IsAlive(self->enemy));

		if (HAVE_ENEMY)
		{
			NPC_JetpackCombatThink();
		}
		else
		{
			NPC_JetpackTravelThink();
		}
	}
}

void NPC_CheckFlying ( void )
{
	if ( NPCS.NPC->client->NPC_class == CLASS_VEHICLE )
	{// Vehicles... Don't do normal jetpack stuff...

	}
	else
	{
		if (NPCS.NPC->health <= 0 || !NPC_IsAlive(NPCS.NPC) || (NPCS.NPC->client->ps.eFlags & EF_DEAD) || NPCS.NPC->client->ps.pm_type == PM_DEAD)
		{// Dead... Return to normal...
			NPCS.NPC->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
			NPCS.NPC->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
			NPCS.NPC->client->ps.eFlags &= ~EF_JETPACK_HOVER;
			NPCS.NPC->s.eFlags &= ~EF_JETPACK_ACTIVE;
			NPCS.NPC->s.eFlags &= ~EF_JETPACK_FLAMING;
			NPCS.NPC->s.eFlags &= ~EF_JETPACK_HOVER;
			NPCS.NPC->client->ps.pm_type = PM_DEAD;
		}
		else
		{// If this NPC has a jetpack... Let's make use of it...
			if (NPC_IsJetpacking( NPCS.NPC ))
			{// Are we flying???
				NPC_DoFlyStuff();
			}
		}
	}
}
