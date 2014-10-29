// Bowcaster Weapon

#include "cg_local.h"

void FX_BowcasterAddLight ( vec3_t org )
{
	vec4_t color = { 0.3, 1.0, 0.3, 100.0 }; // r, g, b, intensity
	trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}

/*
---------------------------
FX_BowcasterProjectileThink
---------------------------
*/

void FX_BowcasterProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	if (weapon->missileRenderfx)
		trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID( cgs.effects.bowcasterShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_BowcasterAddLight(cent->lerpOrigin);
}

/*
---------------------------
FX_BowcasterHitWall
---------------------------
*/

void FX_BowcasterHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID(CG_EnableEnhancedFX(cgs.effects.bowcasterImpactEffect,
		cgs.effects.bowcasterImpactEffectEnhancedFX), origin, normal, -1, -1, qfalse);
}

/*
---------------------------
FX_BowcasterHitPlayer
---------------------------
*/

void FX_BowcasterHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
	trap->FX_PlayEffectID( cgs.effects.bowcasterImpactEffect, origin, normal, -1, -1, qfalse );
}

/*
------------------------------
FX_BowcasterAltProjectileThink
------------------------------
*/

void FX_BowcasterAltProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	if (weapon->altMissileRenderfx)
		trap->FX_PlayEffectID(weapon->altMissileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	else if (weapon->missileRenderfx)
		trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID( cgs.effects.bowcasterShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_BowcasterAddLight(cent->lerpOrigin);
}

