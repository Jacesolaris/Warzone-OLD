// Rocket Launcher Weapon

#include "cg_local.h"

void FX_RocketAddLight(vec3_t org, int weapon)
{// should really do light colors too.. hmm.. i will do the other weap colors another time too slow over tv, okay
	vec4_t color = { 0.9f, 0.9f, 0.2f, 80.0f }; // r, g, b, intensity

	if (cg_weapons[weapon].missileDlightColor[0] || cg_weapons[weapon].missileDlightColor[1] || cg_weapons[weapon].missileDlightColor[2])
		VectorCopy(cg_weapons[weapon].missileDlightColor, color);

	AddLightToScene(org, color[3], color[0], color[1], color[2]);
}

/*
---------------------------
FX_RocketProjectileThink
---------------------------
*/

void FX_RocketProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
{
	vec3_t forward;

	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
	{
		forward[2] = 1.0f;
	}
	if (weapon->missileRenderfx)
	{
		PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	else
	{
		PlayEffectID(cgs.effects.rocketShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
	}

	FX_RocketAddLight(cent->lerpOrigin, cent->currentState.weapon);
}

/*
---------------------------
FX_RocketHitWall
---------------------------
*/

//void FX_RocketHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
//{
//	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
//	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;
//
//	if (fx)
//		PlayEffectID(fx, origin, normal, -1, -1, qfalse);
//	else
//		PlayEffectID(CG_EnableEnhancedFX(cgs.effects.rocketExplosionEffect, cgs.effects.rocketExplosionEffectEnhancedFX), origin, normal, -1, -1, qfalse);
//}

void FX_RocketHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	// Set fx to primary weapon fx.
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	fxHandle_t fx2 = cg_weapons[weapon].EnhancedFX_missileWallImpactfx;

	if (!fx) {
		// If there is no primary (missileWallImpactfx) fx. Use original blaster fx.
		fx = cgs.effects.rocketExplosionEffect;

		// If falling back to normal concussion fx, we have no enhanced.
		fx2 = fx; // Force normal fx.
	}

	if (altFire) {
		// If this is alt fire. Override all fx with alt fire fx...
		if (cg_weapons[weapon].altMissileWallImpactfx)
		{// We have alt fx for this weapon. Use it.
			fx = cg_weapons[weapon].altMissileWallImpactfx;
		}

		if (cg_weapons[weapon].EnhancedFX_altmissileWallImpactfx)
		{// We have enhanced alt. Use it.
			fx2 = cg_weapons[weapon].EnhancedFX_altmissileWallImpactfx;
		}
		else
		{// We have no alt enhanced fx.
			fx2 = fx; // Force normal fx.
		}
	}

	// If fx2 (enhanced) does not exist (set fx2 to -1 above), this should return normal fx.
	fx = CG_EnableEnhancedFX(fx, fx2);

	if (fx)
	{// We have fx for this. Play it.
		PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	}
	else
	{// This should never be possible, but just in case, fall back to concussion here.
		PlayEffectID(cgs.effects.rocketExplosionEffect, origin, normal, -1, -1, qfalse);
	}
}

//void FX_PulseRocketHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
//{
//	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
//	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;
//
//	if (fx)
//		PlayEffectID(fx, origin, normal, -1, -1, qfalse);
//	else
//		PlayEffectID(CG_EnableEnhancedFX(cgs.effects.pulserocketExplosionEffect, cgs.effects.pulserocketExplosionEffectEnhancedFX), origin, normal, -1, -1, qfalse);
//}

void FX_PulseRocketHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	// Set fx to primary weapon fx.
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	fxHandle_t fx2 = cg_weapons[weapon].EnhancedFX_missileWallImpactfx;

	if (!fx) {
		// If there is no primary (missileWallImpactfx) fx. Use original blaster fx.
		fx = cgs.effects.pulserocketExplosionEffect;

		// If falling back to normal concussion fx, we have no enhanced.
		fx2 = fx; // Force normal fx.
	}

	if (altFire) {
		// If this is alt fire. Override all fx with alt fire fx...
		if (cg_weapons[weapon].altMissileWallImpactfx)
		{// We have alt fx for this weapon. Use it.
			fx = cg_weapons[weapon].altMissileWallImpactfx;
		}

		if (cg_weapons[weapon].EnhancedFX_altmissileWallImpactfx)
		{// We have enhanced alt. Use it.
			fx2 = cg_weapons[weapon].EnhancedFX_altmissileWallImpactfx;
		}
		else
		{// We have no alt enhanced fx.
			fx2 = fx; // Force normal fx.
		}
	}

	// If fx2 (enhanced) does not exist (set fx2 to -1 above), this should return normal fx.
	fx = CG_EnableEnhancedFX(fx, fx2);

	if (fx)
	{// We have fx for this. Play it.
		PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	}
	else
	{// This should never be possible, but just in case, fall back to concussion here.
		PlayEffectID(cgs.effects.pulserocketExplosionEffect, origin, normal, -1, -1, qfalse);
	}
}

/*
---------------------------
FX_RocketHitPlayer
---------------------------
*/

void FX_RocketHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	// Set fx to primary weapon fx.
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	fxHandle_t fx2 = cg_weapons[weapon].EnhancedFX_fleshImpact;
	//trap->Print("Hit player - weapon %i\n", weapon);
	//trap->Print("FX: %i. FX2: %i.\n", (int)fx, (int)fx2);
	if (!fx) {
		// If there is no primary (missileWallImpactfx) fx. Use original blaster fx.
		fx = cgs.effects.rocketExplosionEffect;
	}

	if (!fx2) {
		if (fx) {
			// We don't have an fx2, but we have an fx. Use it for enhanced as well..
			fx2 = fx;
		}
		else {
			// If there is no primary (missileWallImpactfx) fx. Use original fx.
			fx2 = cgs.effects.rocketExplosionEffect;
		}
	}

	if (altFire) {
		// If this is alt fire. Override all fx with alt fire fx...
		if (cg_weapons[weapon].altFleshImpactEffect)
		{// We have alt fx for this weapon. Use it.
			fx = cg_weapons[weapon].altFleshImpactEffect;
		}

		if (!fx) {
			// If there is no primary (missileWallImpactfx) fx. Use original blaster fx.
			fx = cgs.effects.rocketExplosionEffect;
		}

		if (!fx2) {
			if (fx) {
				// We don't have an fx2, but we have an fx. Use it for enhanced as well..
				fx2 = fx;
			}
			else {
				// If there is no primary (missileWallImpactfx) fx. Use original fx.
				fx2 = cgs.effects.rocketExplosionEffect;
			}
		}

		if (cg_weapons[weapon].EnhancedFX_altfleshImpact)
		{// We have enhanced alt. Use it.
			fx2 = cg_weapons[weapon].EnhancedFX_altfleshImpact;
		}
		else
		{// We have no alt enhanced fx.
			fx2 = fx; // Force normal fx.
		}
	}

	trap->Print("DEBUG: FX: %i. FX2: %i.\n", (int)fx, (int)fx2);

	// If fx2 (enhanced) does not exist (set fx2 to -1 above), this should return normal fx.
	fx = CG_EnableEnhancedFX(fx, fx2);

	if (fx)
	{// We have fx for this. Play it.
		//trap->Print("We have fx\n");
		PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	}
	else
	{
		//trap->Print("We have NO fx\n");
		if (humanoid)
		{
			PlayEffectID(cgs.effects.rocketExplosionEffect, origin, normal, -1, -1, qfalse);
		}
		else
		{
			PlayEffectID(cgs.effects.rocketExplosionEffect, origin, normal, -1, -1, qfalse);
		}
	}
}

//void FX_RocketHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
//{
//	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
//	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;
//
//	if (fx)
//		PlayEffectID(fx, origin, normal, -1, -1, qfalse);
//	else
//		PlayEffectID(CG_EnableEnhancedFX(cgs.effects.rocketExplosionEffect, cgs.effects.rocketExplosionEffectEnhancedFX), origin, normal, -1, -1, qfalse);
//}

//void FX_PulseRocketHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
//{
//	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
//	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;
//
//	if (fx)
//		PlayEffectID(fx, origin, normal, -1, -1, qfalse);
//	else
//	PlayEffectID(CG_EnableEnhancedFX(cgs.effects.pulserocketExplosionEffect, cgs.effects.pulserocketExplosionEffectEnhancedFX), origin, normal, -1, -1, qfalse);
//}

void FX_PulseRocketHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	// Set fx to primary weapon fx.
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	fxHandle_t fx2 = cg_weapons[weapon].EnhancedFX_fleshImpact;

	if (!fx) {
		// If there is no primary (missileWallImpactfx) fx. Use original blaster fx.
		fx = cgs.effects.pulserocketExplosionEffect;

		// If falling back to normal concussion fx, we have no enhanced.
		fx2 = fx; // Force normal fx.
	}

	if (altFire) {
		// If this is alt fire. Override all fx with alt fire fx...
		if (cg_weapons[weapon].altFleshImpactEffect)
		{// We have alt fx for this weapon. Use it.
			fx = cg_weapons[weapon].altFleshImpactEffect;
		}

		if (cg_weapons[weapon].EnhancedFX_altfleshImpact)
		{// We have enhanced alt. Use it.
			fx2 = cg_weapons[weapon].EnhancedFX_altfleshImpact;
		}
		else
		{// We have no alt enhanced fx.
			fx2 = fx; // Force normal fx.
		}
	}

	// If fx2 (enhanced) does not exist (set fx2 to -1 above), this should return normal fx.
	fx = CG_EnableEnhancedFX(fx, fx2);

	if (fx)
	{// We have fx for this. Play it.
		PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	}
	else
	{
		if (humanoid)
		{
			PlayEffectID(cgs.effects.pulserocketExplosionEffect, origin, normal, -1, -1, qfalse);
		}
		else
		{
			PlayEffectID(cgs.effects.pulserocketExplosionEffect, origin, normal, -1, -1, qfalse);
		}
	}
}

/*
---------------------------
FX_RocketAltProjectileThink
---------------------------
*/

void FX_RocketAltProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
{
	vec3_t forward;
	fxHandle_t fx = cg_weapons[cent->currentState.weapon].fleshImpactEffect;

	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
	{
		forward[2] = 1.0f;
	}

	if (fx)
		PlayEffectID(fx, cent->lerpOrigin, forward, -1, -1, qfalse);
	else
		PlayEffectID(cgs.effects.rocketShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);

	FX_RocketAddLight(cent->lerpOrigin, cent->currentState.weapon);
}

void FX_PulseRocketAltProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
{
	vec3_t forward;

	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
	{
		forward[2] = 1.0f;
	}

	if (weapon->altMissileRenderfx)
	{
		PlayEffectID(weapon->altMissileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	else
	{
		PlayEffectID(cgs.effects.pulserocketShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
}
