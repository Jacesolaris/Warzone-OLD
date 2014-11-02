// DEMP2 Weapon

#include "cg_local.h"
#include "fx_local.h"

void FX_DEMP2AddLight ( vec3_t org )
{
	//vec4_t color = { 0.5, 0.5, 1.0, 70.0 }; // r, g, b, intensity
	//trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}

void FX_DEMP2AddExplodeLight ( vec3_t org )
{
	//vec4_t color = { 0.5, 0.5, 1.0, 100.0 }; // r, g, b, intensity
	//trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}


/*
---------------------------
FX_CLONEPOSTOL SETUP
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
		trap->FX_PlayEffectID(cgs.effects.demp2WallImpactEffect, origin, normal, -1, -1, qfalse);
	}
}

/*
---------------------------
FX_CLONEPISTOL_BounceWall
---------------------------
*/
void FX_CLONEPISTOL_BounceWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].WallBounceEffectEnhancedFX;// WallBounceEffectFX;
	//fxHandle_t fx = cg_weapons[weapon].WallBounceEffectEnhancedFX;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID(CG_EnableEnhancedFX(cgs.effects.demp2WallBounceEffect, cgs.effects.demp2WallBounceEffectEnhancedFX), origin, normal, -1, -1, qfalse);
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

//void FX_DEMP2_ProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
//{
//	vec3_t forward;
//
//	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
//	{
//		forward[2] = 1.0f;
//	}
//
//	trap->FX_PlayEffectID( cgs.effects.demp2ProjectileEffect, cent->lerpOrigin, forward, -1, -1, qfalse );
//
//	FX_DEMP2AddLight(cent->lerpOrigin);
//}

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


//void FX_DEMP2_BounceWall(centity_t *cent, const struct weaponInfo_s *weapon)
//{
//	vec3_t forward;
//	int t;
//
//	if (VectorNormalize2(cent->currentState.pos.trDelta, forward) == 0.0f)
//	{
//		forward[2] = 1.0f;
//	}
//
//	if (cent->currentState.generic1 == 6)
//	{
//		if (weapon->missileRenderfx)
//		{
//			for (t = 1; t < (cent->currentState.generic1 - 1); t++)
//			{
//				trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
//
//			}
//		}
//		else
//		{
//			for (t = 1; t < (cent->currentState.generic1 - 1); t++)
//			{
//				trap->FX_PlayEffectID(
//					CG_EnableEnhancedFX(cgs.effects.demp2WallBounceEffect,
//					cgs.effects.demp2WallBounceEffectEnhancedFX), cent->lerpOrigin, forward, -1, -1, qfalse);
//			}
//		}
//	}
//	else
//	{
//		if (weapon->missileRenderfx)
//		{
//			trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
//		}
//		else
//		{
//			trap->FX_PlayEffectID(cgs.effects.demp2ProjectileEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
//		}
//	}
//}

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
//NOTENOTE Fix this after trap calls for all primitives are created.
/*
	vec3_t	dir, chaos,
			c1, c2,
			v1, v2;
	float	len,
			s1, s2, s3;

	VectorSubtract( end, start, dir );
	len = VectorNormalize( dir );

	// Get the base control points, we'll work from there
	VectorMA( start, 0.3333f * len, dir, c1 );
	VectorMA( start, 0.6666f * len, dir, c2 );

	// get some chaos values that really aren't very chaotic :)
	s1 = sin( cg.time * 0.005f ) * 2 + crandom() * 0.2f;
	s2 = sin( cg.time * 0.001f );
	s3 = sin( cg.time * 0.011f );

	VectorSet( chaos, len * 0.01f * s1,
						len * 0.02f * s2,
						len * 0.04f * (s1 + s2 + s3));

	VectorAdd( c1, chaos, c1 );
	VectorScale( chaos, 4.0f, v1 );

	VectorSet( chaos, -len * 0.02f * s3,
						len * 0.01f * (s1 * s2),
						-len * 0.02f * (s1 + s2 * s3));

	VectorAdd( c2, chaos, c2 );
	VectorScale( chaos, 2.0f, v2 );

	VectorSet( chaos, 1.0f, 1.0f, 1.0f );

	FX_AddBezier( start, targ1,
						c1, v1, c2, v2,
						5.0f + s1 * 2, 8.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						chaos, chaos, 0.0f,
						1.0f, trap->R_RegisterShader( "gfx/misc/electric2" ), FX_ALPHA_LINEAR );

	FX_AddBezier( start, targ1,
						c2, v2, c1, v1,
						3.0f + s3, 3.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						chaos, chaos, 0.0f,
						1.0f, trap->R_RegisterShader( "gfx/misc/electric2" ), FX_ALPHA_LINEAR );

	s1 = sin( cg.time * 0.0005f ) + crandom() * 0.1f;
	s2 = sin( cg.time * 0.0025f );
	float cc2 = cos( cg.time * 0.0025f );
	s3 = sin( cg.time * 0.01f ) + crandom() * 0.1f;

	VectorSet( chaos, len * 0.08f * s2,
						len * 0.04f * cc2,//s1 * -s3,
						len * 0.06f * s3 );

	VectorAdd( c1, chaos, c1 );
	VectorScale( chaos, 4.0f, v1 );

	VectorSet( chaos, len * 0.02f * s1 * s3,
						len * 0.04f * s2,
						len * 0.03f * s1 * s2 );

	VectorAdd( c2, chaos, c2 );
	VectorScale( chaos, 3.0f, v2 );

	VectorSet( chaos, 1.0f, 1.0f, 1.0f );

	FX_AddBezier( start, targ1,
						c1, v1, c2, v2,
						4.0f + s3, 8.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						chaos, chaos, 0.0f,
						1.0f, trap->R_RegisterShader( "gfx/misc/electric2" ), FX_ALPHA_LINEAR );

	FX_AddBezier( start, targ1,
						c2, v1, c1, v2,
						5.0f + s1 * 2, 8.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						chaos, chaos, 0.0f,
						1.0f, trap->R_RegisterShader( "gfx/misc/electric2" ), FX_ALPHA_LINEAR );


	VectorMA( start, 14.0f, dir, c1 );

	FX_AddSprite( c1, NULL, NULL, 12.0f + crandom() * 4, 0.0f, 1.0f, 1.0f, random() * 360, 0.0f, 1.0f,
						trap->R_RegisterShader( "gfx/misc/lightningFlash" ));
	FX_AddSprite( c1, NULL, NULL, 6.0f + crandom() * 2, 0.0f, 1.0f, 1.0f, random() * 360, 0.0f, 1.0f,
						trap->R_RegisterShader( "gfx/misc/lightningFlash" ));

	FX_AddSprite( targ1, NULL, NULL, 4.0f + crandom(), 0.0f, 1.0f, 0.0f, chaos, chaos, random() * 360, 0.0f, 10,
						trap->R_RegisterShader( "gfx/misc/lightningFlash" ));
	FX_AddSprite( targ1, NULL, NULL, 8.0f + crandom() * 2, 0.0f, 1.0f, 0.0f, chaos, chaos, random() * 360, 0.0f, 10,
						trap->R_RegisterShader( "gfx/misc/lightningFlash" ));


	//--------------------------------------------

	VectorSubtract( targ2, targ1, dir );
	len = VectorNormalize( dir );

	// Get the base control points, we'll work from there
	VectorMA( targ1, 0.3333f * len, dir, c1 );
	VectorMA( targ1, 0.6666f * len, dir, c2 );

	// get some chaos values that really aren't very chaotic :)
	s1 = sin( cg.time * 0.005f ) * 2 + crandom() * 0.2f;
	s2 = sin( cg.time * 0.001f );
	s3 = sin( cg.time * 0.011f );

	VectorSet( chaos, len * 0.01f * s1,
						len * 0.02f * s2,
						len * 0.04f * (s1 + s2 + s3));

	VectorAdd( c1, chaos, c1 );
	VectorScale( chaos, 4.0f, v1 );

	VectorSet( chaos, -len * 0.02f * s3,
						len * 0.01f * (s1 * s2),
						-len * 0.02f * (s1 + s2 * s3));

	VectorAdd( c2, chaos, c2 );
	VectorScale( chaos, 2.0f, v2 );

	VectorSet( chaos, 1.0f, 1.0f, 1.0f );

	FX_AddBezier( targ1, targ2,
						c1, v1, c2, v2,
						5.0f + s1 * 2, 8.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						chaos, chaos, 0.0f,
						1.0f, trap->R_RegisterShader( "gfx/misc/electric2" ), FX_ALPHA_LINEAR );

	FX_AddBezier( targ1, targ2,
						c2, v2, c1, v1,
						3.0f + s3, 3.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						chaos, chaos, 0.0f,
						1.0f, trap->R_RegisterShader( "gfx/misc/electric2" ), FX_ALPHA_LINEAR );

	s1 = sin( cg.time * 0.0005f ) + crandom() * 0.1f;
	s2 = sin( cg.time * 0.0025f );
	cc2 = cos( cg.time * 0.0025f );
	s3 = sin( cg.time * 0.01f ) + crandom() * 0.1f;

	VectorSet( chaos, len * 0.08f * s2,
						len * 0.04f * cc2,//s1 * -s3,
						len * 0.06f * s3 );

	VectorAdd( c1, chaos, c1 );
	VectorScale( chaos, 4.0f, v1 );

	VectorSet( chaos, len * 0.02f * s1 * s3,
						len * 0.04f * s2,
						len * 0.03f * s1 * s2 );

	VectorAdd( c2, chaos, c2 );
	VectorScale( chaos, 3.0f, v2 );

	VectorSet( chaos, 1.0f, 1.0f, 1.0f );

	FX_AddBezier( targ1, targ2,
						c1, v1, c2, v2,
						4.0f + s3, 8.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						chaos, chaos, 0.0f,
						1.0f, trap->R_RegisterShader( "gfx/misc/electric2" ), FX_ALPHA_LINEAR );

	FX_AddBezier( targ1, targ2,
						c2, v1, c1, v2,
						5.0f + s1 * 2, 8.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						chaos, chaos, 0.0f,
						1.0f, trap->R_RegisterShader( "gfx/misc/electric2" ), FX_ALPHA_LINEAR );


	FX_AddSprite( targ2, NULL, NULL, 4.0f + crandom(), 0.0f, 1.0f, 0.0f, chaos, chaos, random() * 360, 0.0f, 10,
						trap->R_RegisterShader( "gfx/misc/lightningFlash" ));
	FX_AddSprite( targ2, NULL, NULL, 8.0f + crandom() * 2, 0.0f, 1.0f, 0.0f, chaos, chaos, random() * 360, 0.0f, 10,
						trap->R_RegisterShader( "gfx/misc/lightningFlash" ));
*/
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

	FX_DEMP2AddExplodeLight(ex->refEntity.origin);
}