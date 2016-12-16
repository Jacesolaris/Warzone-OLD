#include "b_local.h"

// These define the working combat range for these suckers
#define MIN_DISTANCE		54
#define MIN_DISTANCE_SQR	( MIN_DISTANCE * MIN_DISTANCE )

#define MAX_DISTANCE		128
#define MAX_DISTANCE_SQR	( MAX_DISTANCE * MAX_DISTANCE )

#define LSTATE_CLEAR		0
#define LSTATE_WAITING		1

/*
-------------------------
NPC_Howler_Precache
-------------------------
*/
void NPC_Howler_Precache( void )
{
}


/*
-------------------------
Howler_Idle
-------------------------
*/
void Howler_Idle(gentity_t *aiEnt) {
}


/*
-------------------------
Howler_Patrol
-------------------------
*/
void Howler_Patrol( gentity_t *aiEnt)
{
	vec3_t dif;

	aiEnt->NPC->localState = LSTATE_CLEAR;

	//If we have somewhere to go, then do that
	if ( UpdateGoal(aiEnt) )
	{
		aiEnt->client->pers.cmd.buttons &= ~BUTTON_WALKING;
		NPC_MoveToGoal(aiEnt, qtrue );
	}
	else
	{
		if ( TIMER_Done( aiEnt, "patrolTime" ))
		{
			TIMER_Set( aiEnt, "patrolTime", crandom() * 5000 + 5000 );
		}
	}

	//rwwFIXMEFIXME: Care about all clients, not just client 0
	//OJK: clientnum 0
	VectorSubtract( g_entities[0].r.currentOrigin, aiEnt->r.currentOrigin, dif );

	if ( VectorLengthSquared( dif ) < 256 * 256 )
	{
		G_SetEnemy( aiEnt, &g_entities[0] );
	}

	if ( NPC_CheckEnemyExt(aiEnt, qtrue ) == qfalse )
	{
		Howler_Idle(aiEnt);
		return;
	}
}

/*
-------------------------
Howler_Move
-------------------------
*/
void Howler_Move(gentity_t *aiEnt, qboolean visible )
{
	if ( aiEnt->NPC->localState != LSTATE_WAITING )
	{
		aiEnt->NPC->goalEntity = aiEnt->enemy;
		NPC_MoveToGoal(aiEnt, qtrue );
		aiEnt->NPC->goalRadius = MAX_DISTANCE;	// just get us within combat range
	}
}

//---------------------------------------------------------
void Howler_TryDamage(gentity_t *aiEnt, gentity_t *enemy, int damage )
{
	vec3_t	end, dir;
	trace_t	tr;

	if ( !enemy )
	{
		return;
	}

	AngleVectors( aiEnt->client->ps.viewangles, dir, NULL, NULL );
	VectorMA( aiEnt->r.currentOrigin, MIN_DISTANCE, dir, end );

	// Should probably trace from the mouth, but, ah well.
	trap->Trace( &tr, aiEnt->r.currentOrigin, vec3_origin, vec3_origin, end, aiEnt->s.number, MASK_SHOT, qfalse, 0, 0 );

	if ( tr.entityNum != ENTITYNUM_WORLD )
	{
		G_Damage( &g_entities[tr.entityNum], aiEnt, aiEnt, dir, tr.endpos, damage, DAMAGE_NO_KNOCKBACK, MOD_MELEE );
	}
}

//------------------------------
void Howler_Attack(gentity_t *aiEnt)
{
	if ( !TIMER_Exists( aiEnt, "attacking" ))
	{
		// Going to do ATTACK1
		TIMER_Set( aiEnt, "attacking", 1700 + random() * 200 );
		NPC_SetAnim( aiEnt, SETANIM_BOTH, BOTH_ATTACK1, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD );

		TIMER_Set( aiEnt, "attack_dmg", 200 ); // level two damage
	}

	// Need to do delayed damage since the attack animations encapsulate multiple mini-attacks
	if ( TIMER_Done2( aiEnt, "attack_dmg", qtrue ))
	{
		Howler_TryDamage(aiEnt, aiEnt->enemy, 5 );
	}

	// Just using this to remove the attacking flag at the right time
	TIMER_Done2( aiEnt, "attacking", qtrue );
}

//----------------------------------
void Howler_Combat(gentity_t *aiEnt)
{
	float distance;
	qboolean advance;

	// If we cannot see our target or we have somewhere to go, then do that
	if ( !NPC_ClearLOS4(aiEnt, aiEnt->enemy ) || UpdateGoal(aiEnt))
	{
		aiEnt->NPC->combatMove = qtrue;
		aiEnt->NPC->goalEntity = aiEnt->enemy;
		aiEnt->NPC->goalRadius = MAX_DISTANCE;	// just get us within combat range

		NPC_MoveToGoal(aiEnt, qtrue );
		return;
	}

	// Sometimes I have problems with facing the enemy I'm attacking, so force the issue so I don't look dumb
	NPC_FaceEnemy(aiEnt, qtrue );

	distance	= DistanceHorizontalSquared( aiEnt->r.currentOrigin, aiEnt->enemy->r.currentOrigin );
	advance = (qboolean)( distance > MIN_DISTANCE_SQR ? qtrue : qfalse  );

	if (( advance || aiEnt->NPC->localState == LSTATE_WAITING ) && TIMER_Done( aiEnt, "attacking" )) // waiting monsters can't attack
	{
		if ( TIMER_Done2( aiEnt, "takingPain", qtrue ))
		{
			aiEnt->NPC->localState = LSTATE_CLEAR;
		}
		else
		{
			Howler_Move(aiEnt, qtrue );
		}
	}
	else
	{
		Howler_Attack(aiEnt);
	}
}

/*
-------------------------
NPC_Howler_Pain
-------------------------
*/
void NPC_Howler_Pain( gentity_t *self, gentity_t *attacker, int damage )
{
	if ( damage >= 10 )
	{
		TIMER_Remove( self, "attacking" );
		TIMER_Set( self, "takingPain", 2900 );

		VectorCopy( self->NPC->lastPathAngles, self->s.angles );

		NPC_SetAnim( self, SETANIM_BOTH, BOTH_PAIN1, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD );

		if ( self->NPC )
		{
			self->NPC->localState = LSTATE_WAITING;
		}
	}
}


/*
-------------------------
NPC_BSHowler_Default
-------------------------
*/
void NPC_BSHowler_Default(gentity_t *aiEnt)
{
	if ( aiEnt->enemy )
		Howler_Combat(aiEnt);
	else if ( aiEnt->NPC->scriptFlags & SCF_LOOK_FOR_ENEMIES )
		Howler_Patrol(aiEnt);
	else
		Howler_Idle(aiEnt);

	NPC_UpdateAngles(aiEnt, qtrue, qtrue );
}
