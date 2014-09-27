// Rocket Launcher Weapon

#include "cg_local.h"

void FX_RocketAddLight ( vec3_t org )
{
	vec4_t color = { 0.9, 0.9, 0.2, 80.0 }; // r, g, b, intensity
	trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}

/*
---------------------------
FX_RocketProjectileThink
---------------------------
*/

void FX_RocketProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	trap->FX_PlayEffectID( cgs.effects.rocketShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_RocketAddLight(cent->lerpOrigin);
}

/*
---------------------------
FX_RocketHitWall
---------------------------
*/

void FX_RocketHitWall( vec3_t origin, vec3_t normal )
{
	trap->FX_PlayEffectID( cgs.effects.rocketExplosionEffect, origin, normal, -1, -1, qfalse );
}

/*
---------------------------
FX_RocketHitPlayer
---------------------------
*/

void FX_RocketHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid )
{
	trap->FX_PlayEffectID( cgs.effects.rocketExplosionEffect, origin, normal, -1, -1, qfalse );
}

/*
---------------------------
FX_RocketAltProjectileThink
---------------------------
*/

void FX_RocketAltProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	trap->FX_PlayEffectID( cgs.effects.rocketShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_RocketAddLight(cent->lerpOrigin);
}
