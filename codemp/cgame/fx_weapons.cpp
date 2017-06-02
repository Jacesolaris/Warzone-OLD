// Bryar Pistol Weapon Effects

#include "cg_local.h"
#include "fx_local.h"

//
// UniqueOne's New - GENERIC - Weapon FX Code...
//

/*
-------------------------
FX_WeaponHitWall
-------------------------
*/
void FX_WeaponHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	// Set fx to primary weapon fx.
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	fxHandle_t fx2 = cg_weapons[weapon].EnhancedFX_missileWallImpactfx;

	if (!fx) {
		// If there is no primary (missileWallImpactfx) fx. Use original blaster fx.
		fx = cgs.effects.blasterWallImpactEffect;
		
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
		PlayEffectID(
			CG_EnableEnhancedFX(cgs.effects.demp2WallImpactEffect, cgs.effects.blasterEnhancedFX_missileWallImpactfx), origin, normal, -1, -1, qfalse);
	}
}

/*
-------------------------
FX_WeaponHitPlayer
-------------------------
*/
void FX_WeaponHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	// Set fx to primary weapon fx.
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	fxHandle_t fx2 = cg_weapons[weapon].EnhancedFX_fleshImpact;

	if (!fx) {
		// If there is no primary (missileWallImpactfx) fx. Use original blaster fx.
		fx = cgs.effects.blasterFleshImpactEffect;
		
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
			PlayEffectID( cgs.effects.blasterFleshImpactEffect, origin, normal, -1, -1, qfalse );
		}
		else
		{
			PlayEffectID(cgs.effects.blasterDroidImpactEffect, origin, normal, -1, -1, qfalse);
		}
	}
}

void FX_WeaponBolt3D(vec3_t org, vec3_t fwd, float length, float radius, qhandle_t shader)
{
	refEntity_t ent;

	// Draw the bolt core...
	memset(&ent, 0, sizeof(refEntity_t));
	ent.reType = RT_MODEL;

	ent.customShader = shader;

	ent.modelScale[0] = length;
	ent.modelScale[1] = radius;
	ent.modelScale[2] = radius;

	VectorCopy(org, ent.origin);
	vectoangles(fwd, ent.angles);
	AnglesToAxis(ent.angles, ent.axis);
	ScaleModelAxis(&ent);

	ent.hModel = trap->R_RegisterModel("models/warzone/lasers/laserbolt.md3");

	AddRefEntityToScene(&ent);

	// Now add glow... Maybe one day if the bloom/anamorphic glows are not enough... Probably not worth drawing a second model for...
	/*ent.modelScale[0] = length * 1.25;
	ent.modelScale[1] = radius * 3.0;
	ent.modelScale[2] = radius * 3.0;

	ent.customShader = trap->R_RegisterShader("laserbolt_glow");

	vec3_t org2, back;
	VectorMA(org, -(((length * 1.25) - length) / 2.0), fwd, org2);
	VectorCopy(org2, ent.origin);
	vectoangles(fwd, ent.angles);
	AnglesToAxis(ent.angles, ent.axis);
	ScaleModelAxis(&ent);

	AddRefEntityToScene(&ent);*/
}

/*
-------------------------
FX_WeaponProjectileThink
-------------------------
*/
void FX_WeaponProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
{
	vec3_t forward;

	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
	{
		forward[2] = 1.0f;
	}

	qhandle_t bolt3D = CG_Get3DWeaponBoltColor(weapon, qfalse);

	if (bolt3D)
	{// New 3D bolt enabled...
		FX_WeaponBolt3D(cent->lerpOrigin, forward, CG_Get3DWeaponBoltLength(weapon, qfalse), CG_Get3DWeaponBoltWidth(weapon, qfalse), bolt3D);
	}
	else if (weapon->missileRenderfx)
	{// Old 2D system...
		PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	else
	{// Omg, we still have these?!?!?!
		PlayEffectID(cgs.effects.blasterShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
	}

	//AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0f, 1.0f, 1.0f );
}

/*
-------------------------
FX_WeaponProjectileThink
-------------------------
*/
void FX_WeaponAltProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
{
	vec3_t forward;

	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
	{
		forward[2] = 1.0f;
	}
	
	qhandle_t bolt3D = CG_Get3DWeaponBoltColor(weapon, qtrue);

	if (bolt3D)
	{// New 3D bolt enabled...
		FX_WeaponBolt3D(cent->lerpOrigin, forward, CG_Get3DWeaponBoltLength(weapon, qtrue), CG_Get3DWeaponBoltWidth(weapon, qtrue), bolt3D);
	}
	else if (weapon->altMissileRenderfx)
	{// Old 2D system...
		PlayEffectID(weapon->altMissileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	else
	{// Omg, we still have these?!?!?!
		PlayEffectID(cgs.effects.blasterShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
	}

	//AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0f, 1.0f, 1.0f );
}

void FX_ThermalProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
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
		PlayEffectID(cgs.effects.thermalRealShotEffect2, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	if (weapon->missileRenderfx)
	{
		PlayEffectID(cgs.effects.thermalRealShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
}


void FX_PulseGrenadeProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
{
	vec3_t forward;

	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
	{
		forward[2] = 1.0f;
	}

	if (weapon->missileRenderfx)
	{
		PlayEffectID(weapon->altMissileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	else
	{
		PlayEffectID(cgs.effects.fireGrenadeShotEffect2, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	if (weapon->missileRenderfx)
	{
		PlayEffectID(cgs.effects.fireGrenadeShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	else if (weapon->missileRenderfx)
	{
		PlayEffectID(weapon->altMissileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	else
	{
		PlayEffectID(cgs.effects.concussionGrenadeShotEffect2, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	if (weapon->missileRenderfx)
	{
		PlayEffectID(cgs.effects.concussionGrenadeShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	else if (weapon->missileRenderfx)
	{
		PlayEffectID(weapon->altMissileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	else
	{
		PlayEffectID(cgs.effects.pulseGrenadeShotEffect2, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	if (weapon->missileRenderfx)
	{
		PlayEffectID(cgs.effects.pulseGrenadeShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
	}

}
