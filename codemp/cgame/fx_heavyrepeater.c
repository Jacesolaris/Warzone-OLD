// Heavy Repeater Weapon

#include "cg_local.h"

void FX_RepeaterAddLight ( vec3_t org )
{
	vec4_t color = { 0.8, 0.8, 0.0, 50.0 }; // r, g, b, intensity
	trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}

void FX_RepeaterAltAddLight ( vec3_t org )
{
	vec4_t color = { 0.5, 0.5, 1.0, 50.0 }; // r, g, b, intensity
	trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}

void FX_RepeaterAltAddExplodeLight ( vec3_t org )
{
	vec4_t color = { 0.5, 0.5, 1.0, 100.0 }; // r, g, b, intensity
	trap->R_AddLightToScene( org, color[3], color[0], color[1], color[2] );
}

/*
---------------------------
FX_RepeaterProjectileThink
---------------------------
*/

void FX_RepeaterProjectileThink(centity_t *cent, const struct weaponInfo_s *weapon)
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
		trap->FX_PlayEffectID(cgs.effects.repeaterProjectileEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
}

/*
------------------------
FX_RepeaterHitWall
------------------------
*/

void FX_RepeaterHitWall( vec3_t origin, vec3_t normal, int weapon, qboolean altFire )
{
	fxHandle_t fx = cg_weapons[weapon].wallImpactEffectEnhancedFX;// missileWallImpactfx;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID(CG_EnableEnhancedFX(cgs.effects.repeaterWallImpactEffect, cgs.effects.repeaterWallImpactEffectEnhancedFX), origin, normal, -1, -1, qfalse);
}

/*
------------------------
FX_RepeaterHitPlayer
------------------------
*/

void FX_RepeaterHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid) // meh screw hit player. more work then its worth
{
		trap->FX_PlayEffectID( cgs.effects.repeaterFleshImpactEffect, origin, normal, -1, -1, qfalse );
}

static void CG_DistortionOrb( centity_t *cent )
{
	refEntity_t ent;
	vec3_t ang;
	float scale = 0.5f;
	float vLen;

	if (!cg_renderToTextureFX.integer)
	{
		return;
	}
	memset( &ent, 0, sizeof( ent ) );

	VectorCopy( cent->lerpOrigin, ent.origin );

	VectorSubtract(ent.origin, cg.refdef.vieworg, ent.axis[0]);
	vLen = VectorLength(ent.axis[0]);
	if (VectorNormalize(ent.axis[0]) <= 0.1f)
	{	// Entity is right on vieworg.  quit.
		return;
	}

//	VectorCopy(cg.refdef.viewaxis[2], ent.axis[2]);
//	CrossProduct(ent.axis[0], ent.axis[2], ent.axis[1]);
	vectoangles(ent.axis[0], ang);
	ang[ROLL] = cent->trickAlpha;
	cent->trickAlpha += 16; //spin the half-sphere to give a "screwdriver" effect
	AnglesToAxis(ang, ent.axis);

	//radius must be a power of 2, and is the actual captured texture size
	if (vLen < 128)
	{
		ent.radius = 256;
	}
	else if (vLen < 256)
	{
		ent.radius = 128;
	}
	else if (vLen < 512)
	{
		ent.radius = 64;
	}
	else
	{
		ent.radius = 32;
	}

	VectorScale(ent.axis[0], scale, ent.axis[0]);
	VectorScale(ent.axis[1], scale, ent.axis[1]);
	VectorScale(ent.axis[2], -scale, ent.axis[2]);

	ent.hModel = cgs.media.halfShieldModel;
	ent.customShader = 0;//cgs.media.halfShieldShader;

#if 1
	ent.renderfx = (RF_DISTORTION|RF_RGB_TINT);

	//tint the whole thing a shade of blue
	ent.shaderRGBA[0] = 200.0f;
	ent.shaderRGBA[1] = 200.0f;
	ent.shaderRGBA[2] = 255.0f;
#else //no tint
	ent.renderfx = RF_DISTORTION;
#endif

	trap->R_AddRefEntityToScene( &ent );

	FX_RepeaterAltAddExplodeLight(cent->lerpOrigin);
}

/*
------------------------------
FX_RepeaterAltProjectileThink
-----------------------------
*/

void FX_RepeaterAltProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon )
{
	vec3_t forward;

	if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
	{
		forward[2] = 1.0f;
	}

	if (/*cg_repeaterOrb.integer*/0)
	{
		CG_DistortionOrb(cent);
	}

	if (weapon->altMissileRenderfx)
	{
		trap->FX_PlayEffectID(weapon->altMissileRenderfx, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
	else
	{
		trap->FX_PlayEffectID(cgs.effects.repeaterAltProjectileEffect, cent->lerpOrigin, forward, -1, -1, qfalse);
	}
}

/*
------------------------
FX_RepeaterAltHitWall
------------------------
*/

void FX_RepeaterAltHitWall(vec3_t origin, vec3_t normal, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].wallImpactEffectEnhancedFX;//missileWallImpactfx;
	if (altFire) fx = cg_weapons[weapon].altMissileWallImpactfx;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
		trap->FX_PlayEffectID( cgs.effects.repeaterAltWallImpactEffect, origin, normal, -1, -1, qfalse );
}

/*
------------------------
FX_RepeaterAltHitPlayer
------------------------
*/

void FX_RepeaterAltHitPlayer(vec3_t origin, vec3_t normal, qboolean humanoid, int weapon, qboolean altFire)
{
	fxHandle_t fx = cg_weapons[weapon].fleshImpactEffect;
	if (altFire) fx = cg_weapons[weapon].altFleshImpactEffect;

	if (fx)
		trap->FX_PlayEffectID(fx, origin, normal, -1, -1, qfalse);
	else
	trap->FX_PlayEffectID( cgs.effects.repeaterAltWallImpactEffect, origin, normal, -1, -1, qfalse );
}
