#include "cg_local.h"
#include "fx_local.h"

//#define __OLD_BOLT_GLOWS__

//
// 3D Blaster bolts...
//

qboolean DEFAULT_BLASTER_SHADERS_INITIALIZED = qfalse;

int numBoltGlowIndexes = 0;
qhandle_t boltBoltIndexes[1024] = { 0 };
#ifdef __OLD_BOLT_GLOWS__
qhandle_t boltGlowIndexes[1024] = { 0 };
#endif //__OLD_BOLT_GLOWS__
vec3_t    boltLightColors[1024] = { 0 };

void CG_MakeShaderBoltGlow(qhandle_t boltShader, qhandle_t newBoltGlowShader, vec3_t lightColor)
{// Make a list of all the glows matching the original bolts...
	boltBoltIndexes[numBoltGlowIndexes] = boltShader;
#ifdef __OLD_BOLT_GLOWS__
	boltGlowIndexes[numBoltGlowIndexes] = newBoltGlowShader;
#endif //__OLD_BOLT_GLOWS__
	VectorCopy(lightColor, boltLightColors[numBoltGlowIndexes]);
	numBoltGlowIndexes++;
}

void CG_RegisterDefaultBlasterShaders(void)
{// New blaster 3D bolt shader colors can be registered here and reused, rather then for each gun...
	if (DEFAULT_BLASTER_SHADERS_INITIALIZED) return;

	DEFAULT_BLASTER_SHADERS_INITIALIZED = qtrue;

	//
	//
	// Add any new bolt/glow-color/light-color combos here...
	//
	// Note: do not resuse the same shader with a different glow. Make a new shader for each if a different glow is needed...
	//

	cgs.media.whiteBlasterShot = trap->R_RegisterShader("laserbolt_white"); // the basic color of the actual bolt...
	CG_MakeShaderBoltGlow(cgs.media.whiteBlasterShot, trap->R_RegisterShader("laserbolt_white_glow"), colorWhite); // a glow shader matching the bolt color and glow color...

	cgs.media.yellowBlasterShot = trap->R_RegisterShader("laserbolt_yellow");
	CG_MakeShaderBoltGlow(cgs.media.yellowBlasterShot, trap->R_RegisterShader("laserbolt_yellow_glow"), colorYellow);

	cgs.media.redBlasterShot = trap->R_RegisterShader("laserbolt_red");
	CG_MakeShaderBoltGlow(cgs.media.redBlasterShot, trap->R_RegisterShader("laserbolt_red_glow"), colorRed);

	cgs.media.blueBlasterShot = trap->R_RegisterShader("laserbolt_blue");
	CG_MakeShaderBoltGlow(cgs.media.blueBlasterShot, trap->R_RegisterShader("laserbolt_blue_glow"), colorBlue);

	cgs.media.greenBlasterShot = trap->R_RegisterShader("laserbolt_green");
	CG_MakeShaderBoltGlow(cgs.media.greenBlasterShot, trap->R_RegisterShader("laserbolt_green_glow"), colorGreen);
	
	cgs.media.PurpleBlasterShot = trap->R_RegisterShader("laserbolt_purple");
	vec3_t		colorPurple = { 1.0, 0.0, 1.0 };
	CG_MakeShaderBoltGlow(cgs.media.PurpleBlasterShot, trap->R_RegisterShader("laserbolt_purple_glow"), colorPurple);

	cgs.media.orangeBlasterShot = trap->R_RegisterShader("laserbolt_orange");
	CG_MakeShaderBoltGlow(cgs.media.orangeBlasterShot, trap->R_RegisterShader("laserbolt_orange_glow"), colorOrange);

	//custom gfx files for other bolts 

	cgs.media.BlasterBolt_Cap_BluePurple = trap->R_RegisterShader("BlasterBolt_Line_BluePurple");
	CG_MakeShaderBoltGlow(cgs.media.BlasterBolt_Cap_BluePurple, trap->R_RegisterShader("BlasterBolt_Cap_BluePurple"), colorPurple);

	//
	// UQ1: Adding saber colors here... Stoiss: Add any extra colors here...
	//

	cgs.media.whiteSaber = trap->R_RegisterShader("laserbolt_white"); // the basic color of the actual bolt...
	CG_MakeShaderBoltGlow(cgs.media.whiteSaber, trap->R_RegisterShader("laserbolt_white_glow"), colorWhite);

	cgs.media.yellowSaber = trap->R_RegisterShader("laserbolt_yellow");
	CG_MakeShaderBoltGlow(cgs.media.yellowSaber, trap->R_RegisterShader("laserbolt_yellow_glow"), colorYellow);

	cgs.media.redSaber = trap->R_RegisterShader("laserbolt_red");
	CG_MakeShaderBoltGlow(cgs.media.redSaber, trap->R_RegisterShader("laserbolt_red_glow"), colorRed);

	cgs.media.blueSaber = trap->R_RegisterShader("laserbolt_blue");
	CG_MakeShaderBoltGlow(cgs.media.blueSaber, trap->R_RegisterShader("laserbolt_blue_glow"), colorBlue);

	cgs.media.greenSaber = trap->R_RegisterShader("laserbolt_green");
	CG_MakeShaderBoltGlow(cgs.media.greenSaber, trap->R_RegisterShader("laserbolt_green_glow"), colorGreen);

	cgs.media.purpleSaber = trap->R_RegisterShader("laserbolt_purple");
	CG_MakeShaderBoltGlow(cgs.media.purpleSaber, trap->R_RegisterShader("laserbolt_purple_glow"), colorPurple);

	cgs.media.orangeSaber = trap->R_RegisterShader("laserbolt_orange");
	CG_MakeShaderBoltGlow(cgs.media.orangeSaber, trap->R_RegisterShader("laserbolt_orange_glow"), colorOrange);

	cgs.media.bluePurpleSaber = trap->R_RegisterShader("BlasterBolt_Line_BluePurple");
	CG_MakeShaderBoltGlow(cgs.media.bluePurpleSaber, trap->R_RegisterShader("BlasterBolt_Cap_BluePurple"), colorPurple);
}

float *CG_Get3DWeaponBoltLightColor(qhandle_t boltShader)
{
	for (int i = 0; i < numBoltGlowIndexes; i++)
	{
		if (boltBoltIndexes[i] == boltShader)
		{
			return boltLightColors[i];
		}
	}

	return colorBlack;
}

#ifdef __OLD_BOLT_GLOWS__
qhandle_t CG_Get3DWeaponBoltGlowColor(qhandle_t boltShader)
{
	for (int i = 0; i < numBoltGlowIndexes; i++)
	{
		if (boltBoltIndexes[i] == boltShader)
		{
			return boltGlowIndexes[i];
		}
	}

	return -1;
}
#endif //__OLD_BOLT_GLOWS__

qhandle_t CG_Get3DWeaponBoltColor(const struct weaponInfo_s *weaponInfo, qboolean altFire)
{
	if (!weaponInfo) return cgs.media.whiteBlasterShot; // Fallback...

	if (altFire)
	{
		if (weaponInfo->bolt3DShaderAlt)
		{
			return weaponInfo->bolt3DShaderAlt;
		}

		return -1; // Fall back to old system...
	}
	else
	{
		if (weaponInfo->bolt3DShader)
		{
			return weaponInfo->bolt3DShader;
		}

		return -1; // Fall back to old system...
	}

	return cgs.media.whiteBlasterShot; // Fallback...
}

float CG_Get3DWeaponBoltLength(const struct weaponInfo_s *weaponInfo, qboolean altFire)
{
	if (!weaponInfo) return 1.0; // Default size...

	if (altFire)
	{
		if (weaponInfo->bolt3DLengthAlt)
		{
			return weaponInfo->bolt3DLengthAlt;
		}

		return 1.0; // Default size...
	}
	else
	{
		if (weaponInfo->bolt3DLength)
		{
			return weaponInfo->bolt3DLength;
		}

		return 1.0; // Default size...
	}

	return 1.0; // Default size...
}

float CG_Get3DWeaponBoltWidth(const struct weaponInfo_s *weaponInfo, qboolean altFire)
{
	if (!weaponInfo) return 1.0; // Default size...

	if (altFire)
	{
		if (weaponInfo->bolt3DWidthAlt)
		{
			return weaponInfo->bolt3DWidthAlt;
		}

		return 1.0; // Default size...
	}
	else
	{
		if (weaponInfo->bolt3DWidth)
		{
			return weaponInfo->bolt3DWidth;
		}

		return 1.0; // Default size...
	}

	return 1.0; // Default size...
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

#ifdef __OLD_BOLT_GLOWS__
	// Now add glow...
	memset(&ent, 0, sizeof(refEntity_t));

	qhandle_t glowColorShader = CG_Get3DWeaponBoltGlowColor(shader);

	if (glowColorShader)
#endif //__OLD_BOLT_GLOWS__
	{// Now add glow...
#ifdef __OLD_BOLT_GLOWS__
		vec3_t org2, back;
		VectorMA(org, -((length * 1.25 * 16.0) - length * 16.0) * 4.0, fwd, org2);
		VectorCopy(org2, ent.origin);
		VectorCopy(fwd, ent.axis[0]);

		ent.saberLength = length * 1.25 * 16.0 * 1.5;

		float radius2 = radius * 16.0;
		float radiusRange = radius2 * 0.075f;
		float radiusStart = radius2 - radiusRange;
		float radiusmult = 1.0;
		ent.radius = (radiusStart + crandom() * radiusRange)*radiusmult;

		ent.renderfx |= RF_RGB_TINT;
		ent.shaderRGBA[0] = 255;
		ent.shaderRGBA[1] = 255;
		ent.shaderRGBA[2] = 255;
		ent.shaderRGBA[3] = 128.0;// cg_gunX.value;// 64;
		ent.reType = RT_SABER_GLOW;
		ent.customShader = glowColorShader;

		AddRefEntityToScene(&ent);
#endif //__OLD_BOLT_GLOWS__

		// Add light as well...
		vec3_t lightColor;
		VectorCopy(CG_Get3DWeaponBoltLightColor(shader), lightColor);
		trap->R_AddLightToScene(org, 200 + (rand() & 31), lightColor[0] * 0.15, lightColor[1] * 0.15, lightColor[2] * 0.15);
	}
}

//
// 3D Saber bolts stuff...
//

qhandle_t CG_GetSaberBoltColor(saber_colors_t color)
{
	switch (color)
	{
	case SABER_RED:
		return cgs.media.redSaber;
		break;
	case SABER_ORANGE:
		return cgs.media.orangeSaber;
		break;
	case SABER_YELLOW:
		return cgs.media.yellowSaber;
		break;
	case SABER_GREEN:
		return cgs.media.greenSaber;
		break;
	case SABER_BLUE:
		return cgs.media.blueSaber;
		break;
	case SABER_PURPLE:
		return cgs.media.purpleSaber;
		break;
	case SABER_WHITE:
	case SABER_BLACK:
	case SABER_RGB:
	case SABER_PIMP:
	case SABER_SCRIPTED:
	default:
		return cgs.media.whiteSaber;
		break;
	}

	return cgs.media.whiteSaber; // Fallback...
}

void FX_SaberBolt3D(vec3_t org, vec3_t fwd, float length, float radius, qhandle_t shader)
{
	refEntity_t ent;

	// Draw the bolt core...
	memset(&ent, 0, sizeof(refEntity_t));
	ent.reType = RT_MODEL;

	ent.customShader = shader;

	ent.modelScale[0] = length;
	ent.modelScale[1] = radius;
	ent.modelScale[2] = radius;

	ent.renderfx |= RF_RGB_TINT;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;

	VectorCopy(org, ent.origin);
	vectoangles(fwd, ent.angles);
	AnglesToAxis(ent.angles, ent.axis);
	ScaleModelAxis(&ent);

	ent.hModel = trap->R_RegisterModel("models/warzone/lasers/laserbolt.md3");

	AddRefEntityToScene(&ent);

#ifdef __OLD_BOLT_GLOWS__
	// Now add glow...
	memset(&ent, 0, sizeof(refEntity_t));

	qhandle_t glowColorShader = CG_Get3DWeaponBoltGlowColor(shader);

	if (glowColorShader)
#endif //__OLD_BOLT_GLOWS__
	{// Now add glow...
#ifdef __OLD_BOLT_GLOWS__
		vec3_t org2, back;
		VectorMA(org, -((length * 1.25 * 16.0) - length * 16.0) * 4.0, fwd, org2);
		VectorCopy(org2, ent.origin);
		VectorCopy(fwd, ent.axis[0]);

		ent.saberLength = length * 1.25 * 16.0 * 1.5 * 1.05;// cg_testvalue0.value;
		
		float radius2 = radius * cg_saberGlowRadius.value;
		float radiusRange = radius2 * 0.075f;
		float radiusStart = radius2 - radiusRange;
		float radiusmult = cg_saberGlowRadiusMult.value;
		ent.radius = (radiusStart + crandom() * radiusRange)*radiusmult;

		ent.renderfx |= RF_RGB_TINT;
		ent.shaderRGBA[0] = 255;
		ent.shaderRGBA[1] = 255;
		ent.shaderRGBA[2] = 255;
		ent.shaderRGBA[3] = cg_saberGlowAlphalevel.value;
		ent.reType = RT_SABER_GLOW;
		ent.customShader = glowColorShader;

		AddRefEntityToScene(&ent);
#endif //__OLD_BOLT_GLOWS__

		// Add light as well...
		vec3_t lightColor;
		VectorCopy(CG_Get3DWeaponBoltLightColor(shader), lightColor);
		trap->R_AddLightToScene(org, 200 + (rand() & 31), lightColor[0] * 0.15, lightColor[1] * 0.15, lightColor[2] * 0.15);
	}
}

void CG_Do3DSaber(vec3_t origin, vec3_t dir, float length, float lengthMax, float radius, saber_colors_t color)
{
	vec3_t		mid;

	if (length < 0.5f)
	{
		// if the thing is so short, just forget even adding me.
		return;
	}

	// Find the midpoint of the saber for lighting purposes
	VectorMA(origin, length * 0.5f, dir, mid);

	float len = length / lengthMax;
	FX_SaberBolt3D(mid, dir, cg_saberLengthMult.value * len, cg_saberRadiusMult.value, CG_GetSaberBoltColor(color));
}
