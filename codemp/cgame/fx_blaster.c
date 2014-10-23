// Blaster Weapon

#include "cg_local.h"

void FX_BlasterAddLight ( vec3_t org )
{
	vec4_t color = { 1.0, 0.2, 0.0, 100.0 }; // r, g, b, intensity
	trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}

/*
-------------------------
FX_BlasterProjectileThink
-------------------------
*/

void FX_BlasterProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	if (weapon->missileRenderfx)
		trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	else	
	trap->FX_PlayEffectID( cgs.effects.blasterShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_BlasterAddLight(cent->lerpOrigin);
}

/*
-------------------------
FX_BlasterAltFireThink
-------------------------
*/
void FX_BlasterAltFireThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	if (weapon->missileRenderfx)
		trap->FX_PlayEffectID(weapon->altMissileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID(cgs.effects.blasterShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_BlasterAddLight(cent->lerpOrigin);
}

/*
-------------------------
FX_BlasterWeaponHitWall
-------------------------
*/
void FX_BlasterWeaponHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
	trap->FX_PlayEffectID( cgs.effects.blasterWallImpactEffect, origin, normal, -1, -1, qfalse );
}

/*
-------------------------
FX_BlasterWeaponHitPlayer
-------------------------
*/
void FX_BlasterWeaponHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire )
{
	if ( humanoid )
	{
		trap->FX_PlayEffectID( cgs.effects.blasterFleshImpactEffect, origin, normal, -1, -1, qfalse );
	}
	else
	{
		trap->FX_PlayEffectID( cgs.effects.blasterDroidImpactEffect, origin, normal, -1, -1, qfalse );
	}
}
