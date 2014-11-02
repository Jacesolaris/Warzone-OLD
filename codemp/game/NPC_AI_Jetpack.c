#include "b_local.h"
#include "g_nav.h"
#include "anims.h"

extern int gWPNum;
extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];
extern int GROUND_TIME[MAX_GENTITIES];

extern int DOM_GetNearestWP(vec3_t org, int badwp);
extern int NPC_GetNextNode(gentity_t *NPC);
extern qboolean UQ1_UcmdMoveForDir ( gentity_t *self, usercmd_t *cmd, vec3_t dir, qboolean walk, vec3_t dest );
extern qboolean JKG_PointNearMoverEntityLocation( vec3_t org );

qboolean NPC_IsJetpacking ( gentity_t *self )
{
	if (NPCS.NPC->s.eFlags & EF_JETPACK_ACTIVE || NPCS.NPC->s.eFlags & EF_JETPACK_FLAMING || NPCS.NPC->s.eFlags & EF_JETPACK_HOVER)
		return qtrue;

	return qfalse;
}

void NPC_JetpackCombatThink ( void )
{
	gentity_t	*self = NPCS.NPC;
	usercmd_t	*ucmd = &NPCS.ucmd;

	if (self->r.currentOrigin[2] > self->enemy->r.currentOrigin[2] + 384)
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
		&& (JKG_PointNearMoverEntityLocation(self->r.currentOrigin) || JKG_PointNearMoverEntityLocation(gWPArray[self->wpCurrent]->origin) /*|| gWPArray[self->wpCurrent]->origin[2] - self->r.currentOrigin[2] > 256*/))
	{// We can use the jetpack for this instead of waiting...
		while ((self->wpCurrent >= 0 && self->wpCurrent < gWPNum) && JKG_PointNearMoverEntityLocation(gWPArray[self->wpCurrent]->origin))
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

			//if (DistanceHorizontal(gWPArray[self->wpCurrent]->origin, self->r.currentOrigin) > 64)
			if (!OrgVisible(self->r.currentOrigin, gWPArray[self->wpCurrent]->origin, self->s.number))
				return; // Make sure we go up until we are close...
		}
	}

	if (GROUND_TIME[self->client->ps.clientNum] < level.time && NPC_IsJetpacking(self))
	{// No enemy... Land...
		if (!(self->wpCurrent >= 0 && self->wpCurrent < gWPNum) /*|| !OrgVisible(self->r.currentOrigin, gWPArray[self->wpCurrent]->origin, self->s.number)*/)
		{// No valid waypoint to go to... Find one...
			self->wpCurrent = DOM_GetNearestWP(self->r.currentOrigin, self->wpCurrent);
		}

		if (self->wpCurrent >= 0 && self->wpCurrent < gWPNum)
		{// Seems that we have a valid waypoint...
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
			else if (gWPArray[self->wpCurrent]->origin[2]+64 < self->r.currentOrigin[2])
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
				ucmd->upmove = 0.0;

				self->client->ps.velocity[2] = 0;
				self->client->ps.eFlags |= EF_JETPACK_HOVER;
				self->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
				self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
				self->s.eFlags |= EF_JETPACK_HOVER;
				self->s.eFlags &= ~EF_JETPACK_ACTIVE;
				self->s.eFlags &= ~EF_JETPACK_FLAMING;
				self->client->ps.pm_type = PM_JETPACK;
			}
		}
		else
		{// No waypoint... Just try to land...
			ucmd->upmove = -50.0;

			self->client->ps.velocity[2] = 0;
			self->client->ps.eFlags |= EF_JETPACK_HOVER;
			self->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;
			self->client->ps.eFlags &= ~EF_JETPACK_FLAMING;
			self->s.eFlags |= EF_JETPACK_HOVER;
			self->s.eFlags &= ~EF_JETPACK_ACTIVE;
			self->s.eFlags &= ~EF_JETPACK_FLAMING;
			self->client->ps.pm_type = PM_JETPACK;
		}
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
