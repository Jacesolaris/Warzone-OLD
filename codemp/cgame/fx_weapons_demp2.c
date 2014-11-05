// DEMP2 Weapon

#include "cg_local.h"
#include "fx_local.h"

/*
---------------------------
FX_CLONEPISTOL_ProjectileThink
---------------------------
*/
void FX_CLONEPISTOL_ProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
{
	vec3_t forward;
	int t;

	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
	{
		forward[2] = 1.0f;
	}

	if (cent->currentState.generic1 == 6)
	{
		if (weapon->missileRenderfx)
		{
			for (t = 1; t < (cent->currentState.generic1 - 1); t++)
			{
				trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);

			}
		}
		else
		{
			for (t = 1; t < (cent->currentState.generic1 - 1); t++)
			{
				trap->FX_PlayEffectID(weapon->shotEffectFx, cent->lerpOrigin, forward, -1, -1, qfalse);
			}
		}
	}
	else
	{
		if (weapon->missileRenderfx)
		{
			trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
		}
		else
		{
			trap->FX_PlayEffectID(cgs.effects.demp2ProjectileEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
		}
	}
}

/*
---------------------------
FX_CLONEPISTOL_HitWall
---------------------------
*/

void FX_CLONEPISTOL_HitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	fxHandle_t fx2 = cg_weapons[weapon].wallImpactEffectEnhancedFX;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx2)
	{
		trap->FX_PlayEffectID(CG_EnableEnhancedFX(fx, fx2), origin, normal, -1, -1, qfalse);
	}
	else if (fx)
	{
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	}
	else
	{
		trap->FX_PlayEffectID(CG_EnableEnhancedFX(cgs.effects.demp2WallImpactEffect, 
			cgs.effects.demp2WallImpactEffectEnhancedFX), origin, normal, -1, -1, qfalse);

	}
}

/*
---------------------------
FX_CLONEPISTOL_BounceWall
---------------------------
*/
void FX_CLONEPISTOL_BounceWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].WallBounceEffectEnhancedFX;
	fxHandle_t fx2 = cg_weapons[weapon].WallBounceEffectEnhancedFX;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx2)
	{
		trap->FX_PlayEffectID(CG_EnableEnhancedFX(fx, fx2), origin, normal, -1, -1, qfalse);
	}
	else if (fx)
	{
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	}
	else
		trap->FX_PlayEffectID(CG_EnableEnhancedFX(cgs.effects.demp2WallBounceEffect, 
		cgs.effects.demp2WallBounceEffectEnhancedFX), origin, normal, -1, -1, qfalse);
}


/*
---------------------------
FX_CLONEPISTOL_HitPlayer
---------------------------
*/

void FX_CLONEPISTOL_HitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID(cgs.effects.demp2FleshImpactEffect, origin, normal, -1, -1, qfalse);
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
		if (weapon->missileRenderfx)
		{
			for (t = 1; t < (cent->currentState.generic1 - 1); t++) 
			{
				trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);

			}
		}
		else
		{
			for (t = 1; t < (cent->currentState.generic1 - 1); t++)
			{
				trap->FX_PlayEffectID(weapon->shotEffectFx, cent->lerpOrigin, forward, -1, -1, qfalse);
			}
		}
	}
	else
	{
		if (weapon->missileRenderfx)
		{
			trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
		}
		else
		{
			trap->FX_PlayEffectID(cgs.effects.demp2ProjectileEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
		}
	}
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
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID(
		CG_EnableEnhancedFX(cgs.effects.demp2WallImpactEffect, cgs.effects.demp2WallImpactEffectEnhancedFX), origin, normal, -1, -1, qfalse);
}

/*
---------------------------
FX_DEMP2_HitPlayer
---------------------------
*/

void FX_DEMP2_HitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
	trap->FX_PlayEffectID( cgs.effects.demp2FleshImpactEffect, origin, normal, -1, -1, qfalse );
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
							175, trap->R_RegisterShader( "gfx/misc/lightningFlash"/*"gfx/effects/blueLine"*/ ),
							FX_SIZE_LINEAR | FX_ALPHA_LINEAR );

	// add some beef
	trap->FX_AddLine( start, end, 0.3f, 11.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						BRIGHT, BRIGHT, 0.0f,
						150, trap->R_RegisterShader( "gfx/misc/electric2"/*"gfx/misc/whiteline2"*/ ),
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