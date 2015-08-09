// DEMP2 Weapon

#include "cg_local.h"
#include "fx_local.h"

/*
---------------------------
FX_Clonepistol_ProjectileThink
---------------------------
*/
void FX_Clonepistol_ProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
{
	vec3_t forward;
	int t;

	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
	{
		forward[2] = 1.0f;
	}

	if (cent->currentState.generic1 == 6)
	{
		for (t = 1; t < (cent->currentState.generic1 - 1); t++)
		{
			PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
		}
	}
	else
	{
		if (weapon->missileRenderfx)
		{
			PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
		}
		else
		{
			PlayEffectID(cgs.effects.demp2ProjectileEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
		}
	}

	//AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0f, 1.0f, 1.0f );
}

/*
---------------------------
FX_Clonepistol_HitWall
---------------------------
*/
//i added this stuff here under but
void FX_Clonepistol_HitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{

	// Set fx to primary weapon fx.
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	fxHandle_t fx2 = cg_weapons[weapon].EnhancedFX_missileWallImpactfx;

	if (!fx) {
		// If there is no primary (missileWallImpactfx) fx. Use original blaster fx.
		fx = cgs.effects.demp2WallImpactEffect;

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
		PlayEffectID(cgs.effects.demp2WallImpactEffect, origin, normal, -1, -1, qfalse);
	}
}

/*
---------------------------
FX_Clonepistol_BounceWall
---------------------------
*///its like it is the same here just call with some othere cmd
void FX_Clonepistol_BounceWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{

	// Set fx to primary weapon fx.
	fxHandle_t fx = cg_weapons[weapon].WallBounceEffectFX;
	fxHandle_t fx2 = cg_weapons[weapon].EnhancedFX_WallBouncefx;

	if (!fx) {
		// If there is no primary (WallBounceEffectFX) fx. Use original blaster fx.
		fx = cgs.effects.demp2WallBounceEffect;

		// If falling back to normal concussion fx, we have no enhanced.
		fx2 = fx; // Force normal fx.
	}

	if (altFire) {
		// If this is alt fire. Override all fx with alt fire fx...
		if (cg_weapons[weapon].altWallBounceEffectFX)
		{// We have alt fx for this weapon. Use it.
			fx = cg_weapons[weapon].altWallBounceEffectFX;
		}

		if (cg_weapons[weapon].EnhancedFX_altWallBouncefx)
		{// We have enhanced alt. Use it.
			fx2 = cg_weapons[weapon].EnhancedFX_altWallBouncefx;
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
		PlayEffectID(cgs.effects.demp2WallBounceEffect, origin, normal, -1, -1, qfalse);
	}
}



/*
---------------------------
FX_Clonepistol_HitPlayer
---------------------------
*/

void FX_Clonepistol_HitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
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
		PlayEffectID(cgs.effects.demp2FleshImpactEffect, origin, normal, -1, -1, qfalse);

}

/*
---------------------------
FX_DEMP2_ProjectileThink
---------------------------
*/

void FX_DEMP2_ProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
{
	vec3_t forward;
	int t;

	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
	{
		forward[2] = 1.0f;
	}

	if (cent->currentState.generic1 == 6)
	{
		for (t = 1; t < (cent->currentState.generic1 - 1); t++) 
		{
			PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
		}
	}
	else
	{
		if (weapon->missileRenderfx)
		{
			PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
		}
		else
		{
			PlayEffectID(cgs.effects.demp2ProjectileEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
		}
	}

	//AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0f, 1.0f, 1.0f );
}

/*
---------------------------
FX_DEMP2_HitWall
---------------------------
*/

void FX_DEMP2_HitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		PlayEffectID(
		CG_EnableEnhancedFX(cgs.effects.demp2WallImpactEffect, cgs.effects.demp2EnhancedFX_missileWallImpactfx), origin, normal, -1, -1, qfalse);
}

/*
---------------------------
FX_DEMP2_HitPlayer
---------------------------
*/

void FX_DEMP2_HitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
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
	PlayEffectID( cgs.effects.demp2FleshImpactEffect, origin, normal, -1, -1, qfalse );
}

void FX_Lightning_AltBeam(centity_t *shotby, vec3_t end, qboolean hit)
{// "hit" is only used when hitting target. set to qfalse to shoot normal.

	// When you want to draw the beam
	vec3_t muzzlePos, hitPos;
	int i = 0;
	static vec3_t white = { 1.0f, 1.0f, 1.0f }; // will need to be adjusted to match gun height
	vec3_t hitPosition = { 13.0, 0.0, 4.6 };
	float minimumBeamThickness = 2.0;
	float maximumBeamThickness = 28.0f;
	float lifeTimeOfBeamInMilliseconds = 300;
	if (hit)
	{
		VectorCopy(end, hitPos);
		VectorAdd(WP_MuzzlePoint[WP_ARC_CASTER_IMPERIAL], shotby->lerpOrigin, muzzlePos);
	}
	else
	{
		VectorCopy(end, hitPos);
		VectorAdd(WP_MuzzlePoint[WP_ARC_CASTER_IMPERIAL], shotby->lerpOrigin, muzzlePos);
	}

	for (i = 0; i < 3; i++)
	{// This means do it 3 times. Should get 3 beams all converging on the target.
		trap->FX_AddLine(
			muzzlePos, hitPos,
			minimumBeamThickness,
			maximumBeamThickness,
			0.0f,
			1.0f, 0.0f, 0.0f,
			white,
			white,
			0.0f,
			lifeTimeOfBeamInMilliseconds,
			trap->R_RegisterShader("gfx/blasters/electricity_deform"),
			FX_SIZE_LINEAR |
			FX_ALPHA_LINEAR);
	}
}

/*
---------------------------
FX_DEMP2_AltBeam
---------------------------
*/
void FX_DEMP2_AltBeam( vec3_t start, vec3_t end, vec3_t normal, //qboolean spark,
								vec3_t targ1, vec3_t targ2 )
{
	static vec3_t WHITE	={1.0f,1.0f,1.0f};
	static vec3_t BRIGHT={0.75f,0.5f,1.0f};

	// UQ1: Let's at least give it something...
	//"concussion/beam"
	trap->FX_AddLine( start, end, 0.3f, 15.0f, 0.0f,
							1.0f, 0.0f, 0.0f,
							WHITE, WHITE, 0.0f,
							175, trap->R_RegisterShader( "gfx_base/misc/lightningFlash"/*"gfx/effects/blueLine"*/ ),
							FX_SIZE_LINEAR | FX_ALPHA_LINEAR );

	// add some beef
	trap->FX_AddLine( start, end, 0.3f, 11.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						BRIGHT, BRIGHT, 0.0f,
						150, trap->R_RegisterShader( "gfx_base/misc/electric2"/*"gfx/misc/whiteline2"*/ ),
						FX_SIZE_LINEAR | FX_ALPHA_LINEAR );
}

//---------------------------------------------
void FX_DEMP2_AltDetonate( vec3_t org, float size )
{
	localEntity_t	*ex;

	ex = CG_AllocLocalEntity();
	ex->leType = LE_FADE_SCALE_MODEL;
	memset( &ex->refEntity, 0, sizeof( refEntity_t ));

	ex->refEntity.renderfx |= RF_VOLUMETRIC;

	ex->startTime = cg.time;
	ex->endTime = ex->startTime + 800;//1600;

	ex->radius = size;
	ex->refEntity.customShader = cgs.media.demp2ShellShader;
	ex->refEntity.hModel = cgs.media.demp2Shell;
	VectorCopy( org, ex->refEntity.origin );

	ex->color[0] = ex->color[1] = ex->color[2] = 255.0f;
}