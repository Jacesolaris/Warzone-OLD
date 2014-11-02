// Bryar Pistol Weapon Effects

#include "cg_local.h"
#include "fx_local.h"

void FX_BryarAddLight ( vec3_t org )
{
	//vec4_t color = { 0.7, 0.7, 0.0, 50.0 }; // r, g, b, intensity
	//trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}

/*
-------------------------

	MAIN FIRE

-------------------------
FX_BryarProjectileThink
-------------------------
*/
void FX_BryarProjectileThink(  centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	trap->FX_PlayEffectID( cgs.effects.bryarShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_BryarAddLight(cent->lerpOrigin);
}

/*
-------------------------
FX_BryarHitWall
-------------------------
*/
void FX_BryarHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID(CG_EnableEnhancedFX(cgs.effects.bryarWallImpactEffect, cgs.effects.bryarWallImpactEffectEnhancedFX), origin, normal, -1, -1, qfalse);
}

/*
-------------------------
FX_BryarHitPlayer
-------------------------
*/
void FX_BryarHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
	{
		if (humanoid)
		{
			trap->FX_PlayEffectID(cgs.effects.bryarFleshImpactEffect, origin, normal, -1, -1, qfalse);
		}
		else
		{
			trap->FX_PlayEffectID(cgs.effects.bryarDroidImpactEffect, origin, normal, -1, -1, qfalse);
		}
	}
}


/*
-------------------------

	ALT FIRE

-------------------------
FX_BryarAltProjectileThink
-------------------------
*/
void FX_BryarAltProjectileThink(  centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;
	int t;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	// see if we have some sort of extra charge going on
	for (t = 1; t < cent->currentState.generic1; t++ )
	{
		// just add ourselves over, and over, and over when we are charged
		trap->FX_PlayEffectID( cgs.effects.bryarPowerupShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );
	}

	//	for ( int t = 1; t < cent->gent->count; t++ )	// The single player stores the charge in count, which isn't accessible on the client

	trap->FX_PlayEffectID( cgs.effects.bryarShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_BryarAddLight(cent->lerpOrigin);
}

/*
-------------------------
FX_BryarAltHitWall
-------------------------
*/
void FX_BryarAltHitWall(vec3_t origin, vec3_t normal, int power, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
	{
		switch (power)
		{
		case 4:
		case 5:
			trap->FX_PlayEffectID(
				CG_EnableEnhancedFX(cgs.effects.bryarWallImpactEffect3, cgs.effects.bryarWallImpactEffect3EnhancedFX), origin, normal, -1, -1, qfalse);
			break;

		case 2:
		case 3:
			trap->FX_PlayEffectID(
				CG_EnableEnhancedFX(cgs.effects.bryarWallImpactEffect2, cgs.effects.bryarWallImpactEffect2EnhancedFX), origin, normal, -1, -1, qfalse);
			break;

		default:
			trap->FX_PlayEffectID(
				CG_EnableEnhancedFX(cgs.effects.bryarWallImpactEffect, cgs.effects.bryarWallImpactEffectEnhancedFX), origin, normal, -1, -1, qfalse);
			break;
		}
	}
}

/*
-------------------------
FX_BryarAltHitPlayer
-------------------------
*/
void FX_BryarAltHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
	{
		if (humanoid)
		{
			trap->FX_PlayEffectID(cgs.effects.bryarFleshImpactEffect, origin, normal, -1, -1, qfalse);
		}
		else
		{
			trap->FX_PlayEffectID(cgs.effects.bryarDroidImpactEffect, origin, normal, -1, -1, qfalse);
		}
	}
}


//TURRET

void FX_TurretAddLight ( vec3_t org )
{
	//vec4_t color = { 0.9, 0.7, 0.0, 60.0 }; // r, g, b, intensity
	//trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}

/*
-------------------------
FX_TurretProjectileThink
-------------------------
*/
void FX_TurretProjectileThink(  centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	trap->FX_PlayEffectID( cgs.effects.turretShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_TurretAddLight(cent->lerpOrigin);
}

/*
-------------------------
FX_TurretHitWall
-------------------------
*/
void FX_TurretHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID(
		CG_EnableEnhancedFX(cgs.effects.bryarWallImpactEffect, cgs.effects.bryarWallImpactEffectEnhancedFX), origin, normal, -1, -1, qfalse);
}

/*
-------------------------
FX_TurretHitPlayer
-------------------------
*/
void FX_TurretHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
	{
		if (humanoid)
		{
			trap->FX_PlayEffectID(cgs.effects.bryarFleshImpactEffect, origin, normal, -1, -1, qfalse);
		}
		else
		{
			trap->FX_PlayEffectID(cgs.effects.bryarDroidImpactEffect, origin, normal, -1, -1, qfalse);
		}
	}
}



//CONCUSSION (yeah, should probably make a new file for this.. or maybe just move all these stupid semi-redundant fx_ functions into one file)

void FX_ConcussionAddLight ( vec3_t org )
{
	//vec4_t color = { 0.5, 0.5, 1.0, 60.0 }; // r, g, b, intensity
	//trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}

/*
-------------------------
FX_ConcussionHitWall
-------------------------
*/
void FX_ConcussionHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].missileWallImpactfx;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID( cgs.effects.concussionImpactEffect, origin, normal, -1, -1, qfalse );
}

/*
-------------------------
FX_ConcussionHitPlayer
-------------------------
*/
void FX_ConcussionHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID( cgs.effects.concussionImpactEffect, origin, normal, -1, -1, qfalse );
}

/*
-------------------------
FX_ConcussionProjectileThink
-------------------------
*/
void FX_ConcussionProjectileThink(  centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	if (weapon->missileRenderfx)
		trap->FX_PlayEffectID(weapon->missileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	else	
		trap->FX_PlayEffectID( cgs.effects.concussionShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse );

	FX_ConcussionAddLight(cent->lerpOrigin);
}

//void FX_ConcussionProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
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
//		trap->FX_PlayEffectID(weapon->altMissileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
//	}
//	else
//	{
//		if (cent->currentState.weapon == WP_DC15 || cent->currentState.weapon == WP_CLONERIFLE)
//		{
//			trap->FX_PlayEffectID(cgs.effects.fireGrenadeFireBlob, cent->lerpOrigin, forward, -1, -1, qfalse);
//
//		}
//		else
//		{
//			trap->FX_PlayEffectID(cgs.effects.concussionShotEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
//			
//		}
//	}
//}

/*
---------------------------
FX_ConcAltShot
---------------------------
*/
static vec3_t WHITE	={1.0f,1.0f,1.0f};
static vec3_t BRIGHT={0.75f,0.5f,1.0f};

void FX_ConcAltShot( vec3_t start, vec3_t end, int weapon )
{
	if (weapon != WP_CONCUSSION 
		&& (cg_weapons[weapon].missileDlightColor[0] > 0 || cg_weapons[weapon].missileDlightColor[1] > 0 || cg_weapons[weapon].missileDlightColor[2] > 0))
	{
		vec3_t mainColor, beefColor;

		VectorCopy(cg_weapons[weapon].missileDlightColor, mainColor);
		VectorCopy(cg_weapons[weapon].missileDlightColor, beefColor);
		beefColor[0] *= 0.4;
		beefColor[1] *= 0.4;
		beefColor[2] *= 0.4;

		//"concussion/beam"
		trap->FX_AddLine( start, end, 0.1f, 10.0f, 0.0f,
							1.0f, 0.0f, 0.0f,
							mainColor, mainColor, 0.0f,
							175, trap->R_RegisterShader( "gfx/effects/whiteline2" ),
							FX_SIZE_LINEAR | FX_ALPHA_LINEAR );

		
		// add some beef
		trap->FX_AddLine( start, end, 0.1f, 7.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						beefColor, beefColor, 0.0f,
						150, trap->R_RegisterShader( "gfx/misc/whiteline2" ),
						FX_SIZE_LINEAR | FX_ALPHA_LINEAR );
	}
	else
	{
		//"concussion/beam"
		trap->FX_AddLine( start, end, 0.1f, 10.0f, 0.0f,
							1.0f, 0.0f, 0.0f,
							WHITE, WHITE, 0.0f,
							175, trap->R_RegisterShader( "gfx/effects/blueLine" ),
							FX_SIZE_LINEAR | FX_ALPHA_LINEAR );

		// add some beef
		trap->FX_AddLine( start, end, 0.1f, 7.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						BRIGHT, BRIGHT, 0.0f,
						150, trap->R_RegisterShader( "gfx/misc/whiteline2" ),
						FX_SIZE_LINEAR | FX_ALPHA_LINEAR );
	}
}
