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

void FX_PulseRocketProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
{
	vec3_t forward;

	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
	{
		forward[2] = 1.0f;
	}

	if (weapon->missileRenderfx)
	{
		trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	else
	{
		trap->FX_PlayEffectID(cgs.effects.pulserocketShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
}

//void FX_GrenadesProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
//{
//	vec3_t forward;
//
//	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
//	{
//		forward[2] = 1.0f;
//	}
//
//	if (weapon->missileRenderfx)
//	{
//		trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
//	}
//	else
//	{
//		if (cent->currentState.weapon == WP_FRAG_GRENADE)
//	{
//		trap->FX_PlayEffectID(cgs.effects.concussionGrenadeShotEffect2, cent->lerpOrigin, forward, -1, -1, qfalse);
//	}
//	else
//	{
//		trap->FX_PlayEffectID(cgs.effects.concussionGrenadeShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
//	}
//		}
//			if (weapon->missileRenderfx)
//		{
//			trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
//		}
//		else
//		{
//			if (cent->currentState.weapon == WP_FRAG_GRENADE_OLD)
//		{
//			trap->FX_PlayEffectID(cgs.effects.concussionGrenadeShotEffect2, cent->lerpOrigin, forward, -1, -1, qfalse);
//		}
//		else
//		{
//			trap->FX_PlayEffectID(cgs.effects.concussionGrenadeShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
//		}
//			if (weapon->missileRenderfx)
//			{
//				trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
//			}
//			else
//			{
//				if (cent->currentState.weapon == WP_SHOCK_GRENADE)
//			{
//					trap->FX_PlayEffectID(cgs.effects.thermalShockwaveEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
//			}
//			else
//			{
//				trap->FX_PlayEffectID(cgs.effects.pulseGrenadeShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
//			}
//		}
//			if (weapon->missileRenderfx)
//		{
//			trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
//		}
//		else
//		{
//			if (cent->currentState.weapon == WP_PLASMA_GRENADE)
//			{
//				trap->FX_PlayEffectID(cgs.effects.PlasmaFlameBurn, cent->lerpOrigin, forward, -1, -1, qfalse);
//			}
//			else
//			{
//				trap->FX_PlayEffectID(cgs.effects.PlasmaFlameBurnEnhancedFX, cent->lerpOrigin, forward, -1, -1, qfalse);
//			}
//			if (weapon->missileRenderfx)
//			{
//				trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
//		}
//		else
//		{
//			/*if (cent->currentState.weapon == WP_SONIC_GRENADE)//TODO: need to have this grenade to do some awesome shit when it is beeing used
//			//should do same effect as the howler screems with animations so players not can do any combat besides run away from it. mb2 idea :P
//			{
//				trap->FX_PlayEffectID(cgs.effects.sonicGrenadeShotEffect2, cent->lerpOrigin, forward, -1, -1, qfalse);
//				}
//				else
//				{
//					trap->FX_PlayEffectID(cgs.effects.sonicGrenadeShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
//				}
//			}
//			if (weapon->missileRenderfx)
//			{
//				trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
//			}
//			else
//			{*/
//				if (cent->currentState.weapon == WP_THERMAL_GRENADE)
//				{
//					trap->FX_PlayEffectID(cgs.effects.thermalShotEffect2, cent->lerpOrigin, forward, -1, -1, qfalse);
//			}
//			else
//			{
//				trap->FX_PlayEffectID(cgs.effects.thermalShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
//			}
//			if (weapon->missileRenderfx)
//			{
//				trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
//			}
//			else
//			{
//				if (cent->currentState.weapon == WP_THERMAL_GREADE_OLD)
//				{
//					trap->FX_PlayEffectID(cgs.effects.thermalRealShotEffect2, cent->lerpOrigin, forward, -1, -1, qfalse);
//					}
//					else
//					{
//						trap->FX_PlayEffectID(cgs.effects.thermalRealShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
//					}
//				}
//			if (weapon->missileRenderfx)
//			{
//				trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
//			}
//			else
//			{
//				if (cent->currentState.weapon == WP_V_59_GRENADE)
//				{
//					trap->FX_PlayEffectID(cgs.effects.fireGrenadeShotEffect2, cent->lerpOrigin, forward, -1, -1, qfalse);
//					}
//					else
//					{
//						trap->FX_PlayEffectID(cgs.effects.fireGrenadeShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
//					}
//				}
//			}
//		}
//	}
//}

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
		trap->FX_PlayEffectID(CG_EnableEnhancedFX(cgs.effects.rocketExplosionEffect, cgs.effects.rocketExplosionEffectEnhancedFX), origin, normal, -1, -1, qfalse);
}

void FX_PulseRocketHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID(CG_EnableEnhancedFX(cgs.effects.pulserocketExplosionEffect, cgs.effects.pulserocketExplosionEffectEnhancedFX), origin, normal, -1, -1, qfalse);
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
		trap->FX_PlayEffectID(CG_EnableEnhancedFX(cgs.effects.rocketExplosionEffect, cgs.effects.rocketExplosionEffectEnhancedFX), origin, normal, -1, -1, qfalse);
}

void FX_PulseRocketHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
	trap->FX_PlayEffectID(CG_EnableEnhancedFX(cgs.effects.pulserocketExplosionEffect, cgs.effects.pulserocketExplosionEffectEnhancedFX), origin, normal, -1, -1, qfalse);
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
