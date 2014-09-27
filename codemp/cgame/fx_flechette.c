// Golan Arms Flechette Weapon

#include "cg_local.h"

void FX_FlechetteAddLight ( vec3_t org )
{
	vec4_t color = { 0.9, 0.8, 0.0, 60.0 }; // r, g, b, intensity
	trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}

/*
-------------------------
FX_FlechetteProjectileThink
-------------------------
*/

void FX_FlechetteProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	trap->FX_PlayEffectID( cgs.effects.flechetteShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_FlechetteAddLight(cent->lerpOrigin);
}

/*
-------------------------
FX_FlechetteWeaponHitWall
-------------------------
*/
void FX_FlechetteWeaponHitWall( vec3_t origin, vec3_t normal )
{
	trap->FX_PlayEffectID( cgs.effects.flechetteWallImpactEffect, origin, normal, -1, -1, qfalse );
}

/*
-------------------------
FX_FlechetteWeaponHitPlayer
-------------------------
*/
void FX_FlechetteWeaponHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid )
{
//	if ( humanoid )
//	{
		trap->FX_PlayEffectID( cgs.effects.flechetteFleshImpactEffect, origin, normal, -1, -1, qfalse );
//	}
//	else
//	{
//		trap->FX_PlayEffect( "blaster/droid_impact", origin, normal );
//	}
}


/*
-------------------------
FX_FlechetteProjectileThink
-------------------------
*/

void FX_FlechetteAltProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	trap->FX_PlayEffectID( cgs.effects.flechetteAltShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_FlechetteAddLight(cent->lerpOrigin);
}
