// Rocket Launcher Weapon

#include "cg_local.h"

void FX_RocketAddLight ( vec3_t org, int weapon )
{// should really do light colors too.. hmm.. i will do the other weap colors another time too slow over tv, okay
	vec4_t color = { 0.9, 0.9, 0.2, 80.0 }; // r, g, b, intensity

	if (cg_weapons[weapon].missileDlightColor[0] || cg_weapons[weapon].missileDlightColor[1] || cg_weapons[weapon].missileDlightColor[2])
		VectorCopy(cg_weapons[weapon].missileDlightColor, color);

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

	FX_RocketAddLight(cent->lerpOrigin, cent->currentState.weapon);
}

/*
---------------------------
FX_RocketHitWall
---------------------------
*/

void FX_RocketHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
	trap->FX_PlayEffectID( cgs.effects.rocketExplosionEffect, origin, normal, -1, -1, qfalse );
}

/*
---------------------------
FX_RocketHitPlayer
---------------------------
*/

void FX_RocketHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
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
	fxHandle_t fx = cg_weapons[cent->currentState.weapon].fleshImpactEffect;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	if (fx)
		trap->FX_PlayEffectID(fx, cent->lerpOrigin, forward, -1, -1, qfalse);
	else
	trap->FX_PlayEffectID( cgs.effects.rocketShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_RocketAddLight(cent->lerpOrigin, cent->currentState.weapon);
}
