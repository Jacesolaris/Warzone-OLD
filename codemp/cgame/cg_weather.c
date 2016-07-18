//// Nerevar's way to produce weather
//#include "cg_local.h" 
//
//#define RAINDISTANCE 500
//
//void CG_DrawRain(void) {
//	float rainDistance, rainNumber;
//	sfxHandle_t sfx;
//	if (cg_rain.integer <= 0)
//		return;
//	else if (cg_rain.integer > 2000)
//	trap->Cvar_Set("cg_rain", "10");		
//	rainDistance = cg_rainDistance.value < 10.0f ? 10.0f : cg_rainDistance.value;
//	rainNumber = cg_rain.value * (rainDistance / 1000.0f);
//	if (cg_rain.integer >= 1000) {
//		sfx = cgs.media.heavyRain;
//	}else if (cg_rain.integer >= 400) {
//		sfx = cgs.media.regularRain;
//	} else {
//		sfx = cgs.media.lightRain;
//	}
//	trap->S_UpdateEntityPosition(ENTITYNUM_NONE, cg.refdef.vieworg);
//	trap->S_AddLoopingSound(ENTITYNUM_NONE, cg.refdef.vieworg, vec3_origin, sfx, NULL);
//	if (cg.we.raintime <= cg.time) {
//		int i;						
//		for (i = 0; i < (int)rainNumber; i++) {
//			vec3_t start, fwd, end;
//			trace_t tr;
//				
//			float angle = cg.refdef.viewangles[YAW];
//			int seed = trap->Milliseconds() - i * 15;
//			float distance = Q_irand(0, 2.0f * rainDistance) - rainDistance;				
//				
//			AngleVectors(cg.refdef.viewangles, fwd, NULL, NULL);	 //<-Viewangles YAW		
//			VectorMA(cg.refdef.vieworg, rainDistance, fwd, start);
//			start[2] = cg.refdef.vieworg[2];
//				
//			start[0] = start[0] + (cos(angle)*distance);
//			start[1] = start[1] + (sin(angle)*distance);
//				
//			angle += DEG2RAD(90);
//			distance = Q_random(&seed) * 2.0f * rainDistance - rainDistance;
//				
//			start[0] = start[0] + (cos(angle)*distance);
//			start[1] = start[1] + (sin(angle)*distance);
//				
//			//GET SKY
//					
//			VectorCopy(start,end);
//			end[2] += 9999;				
//					
//			CG_Trace(&tr, start, NULL, NULL, end, 0, MASK_SOLID);
//				
//			if (tr.surfaceFlags & SURF_SKY) {
//				vec3_t angles;
//				if (tr.endpos[2] - start[2] > 200)
//					tr.endpos[2] = start[2] + 200;
//				tr.endpos[2] += Q_crandom(&seed) * 100.0f;
//				angles[PITCH] = 0;
//				angles[YAW] = 90;
//				angles[ROLL] = 0;
//				PlayEffectID(cgs.effects.rain, tr.endpos, angles, -1, 1, qfalse);
//			}
//		}			
//		cg.we.raintime = cg.time + 30;				
//	}
//}
//
//static qboolean CG_TraceToNotSky(trace_t *tr, const vec3_t start, const vec3_t end, const float size) {
//	int i;
//	/* scan 4 corners, if anyone of them isn't sky, then return and let's search with lower size */
//	for (i = 0; i < 4; i++) {
//		vec3_t offset, trStart, trEnd;
//		if (i == 0) VectorSet(offset, -size, -size, 0);
//		if (i == 1) VectorSet(offset, -size, size, 0);
//		if (i == 2) VectorSet(offset, size, -size, 0);
//		if (i == 3) VectorSet(offset, size, size, 0);
//		VectorSet(trStart, start[0] + offset[0], start[1] + offset[1], start[2]);
//		VectorSet(trEnd, end[0] + offset[0], end[1] + offset[1], end[2]);
//		CG_Trace(tr, trStart, NULL, NULL, trEnd, 0, MASK_SHOT);
//		if (!(tr->surfaceFlags & SURF_SKY)) {
//			return qtrue;
//		}
//	}
//	return qfalse;
//}
//////void CG_DrawSun(void) {
//////	int i;
//////	float size = 0.0f;
//////	trace_t tr;
//////	refEntity_t re;
//////	
//////	if (cg_sun.integer <= 0)
//////		return;
//////
//////	//CALCULATE SIZE //eats too much fps
//////	CG_Trace(&tr, cg.refdef.vieworg, NULL, NULL, cg.we.sunorigin, 0, MASK_SHOT);
//////	if (!(tr.surfaceFlags & SURF_SKY)) //not even worth it
//////		return;
//////	
//////	for (i = 31; i >= 0; i--) {
//////		qboolean notSky = CG_TraceToNotSky(&tr, cg.refdef.vieworg, cg.we.sunorigin, i);
//////		size = i;
//////		if (notSky)
//////			continue;
//////		if (tr.surfaceFlags & SURF_SKY) {
//////			float f, step = 1.0f / cg_sun.value;
//////			for (f = (float)(i + 1); f > (float)(i); f -= step) {
//////				notSky = CG_TraceToNotSky(&tr, cg.refdef.vieworg, cg.we.sunorigin, f);
//////				size = f;
//////				if (notSky)
//////					continue;
//////				if (tr.surfaceFlags & SURF_SKY) {
//////					break;
//////				}
//////			}
//////			break;
//////		}
//////	}
//////	size *= cg_sunSize.value;
//////	if (size <= 0.0f)
//////		return;
//////	memset(&re, 0, sizeof(refEntity_t));
//////	re.reType = RT_SPRITE;
//////	re.customShader = cgs.media.saberFlare;	
//////	VectorCopy(tr.endpos,re.origin);
//////		
//////	re.radius = size * Distance(re.origin, cg.refdef.vieworg) / 20;
//////	re.renderfx = RF_NODEPTH | RF_MINLIGHT;	
//////				
//////	re.shaderRGBA[0] = 255;
//////	re.shaderRGBA[1] = 230; //0.9
//////	re.shaderRGBA[2] = 191; //0.75
//////	re.shaderRGBA[3] = 255; 
//////		
//////	AddRefEntityToScene( &re );
//////}

#include "cg_local.h"

qboolean CG_AtmosphericKludge(); // below


/*
**  	UniqueOne's Brand new (ultra-simple, but looks nice) weather FX using JKA's efx system...
**
**  	Current supported effects are rain, heavy rain, storm, snow and heavy snow.
*/

// Weather types...
typedef enum {
	WEATHER_NONE,
	WEATHER_RAIN,
	WEATHER_HEAVY_RAIN,
	WEATHER_RAIN_STORM,
	WEATHER_SNOW,
	WEATHER_HEAVY_SNOW,
	WEATHER_SNOW_STORM,
};

// Max MAP height...
#define		MAX_ATMOSPHERIC_HEIGHT  	  	524288  	// maximum world height (FIXME: since 1.27 this should be 65536)

// Runtime values...
qboolean	ATMOSPHERICS_INITIIALIZED = qfalse;

float		ATMOSPHERIC_MAX_MAP_HEIGHT = -MAX_ATMOSPHERIC_HEIGHT;

int			ATMOSPHERIC_WEATHER_TYPE = WEATHER_NONE;
int			ATMOSPHERIC_NEXT_LIGHTNING_FLASH_TIME = 0;
int			ATMOSPHERIC_NEXT_SOUND_TIME = 0;

qhandle_t	lightning1 = -1;
qhandle_t	lightning2 = -1;
qhandle_t	lightning3 = -1;
qhandle_t	lightningExplode = -1;


float CG_GetSkyHeight ( trace_t *tr )
{
	int x, y;

	if (ATMOSPHERIC_MAX_MAP_HEIGHT <= -MAX_ATMOSPHERIC_HEIGHT)
	{// Find map's highest point... Once...
		// Try to load pre-created info from our map's .mapInfo file...
		ATMOSPHERIC_MAX_MAP_HEIGHT = atof(IniRead(va("maps/%s.mapInfo", cgs.currentmapname), "MAPINFO", "SKY_HEIGHT", "-999999.0"));

		if (ATMOSPHERIC_MAX_MAP_HEIGHT <= -999999.0)
		{
			for (x = -MAX_ATMOSPHERIC_HEIGHT; x < MAX_ATMOSPHERIC_HEIGHT; x += 256)
			{
				for (y = -MAX_ATMOSPHERIC_HEIGHT; y < MAX_ATMOSPHERIC_HEIGHT; y += 256)
				{
					vec3_t testpoint, testend;
					testpoint[0] = testend[0] = x;
					testpoint[1] = testend[1] = y;
					testpoint[2] = MAX_ATMOSPHERIC_HEIGHT;
					testend[2] = -MAX_ATMOSPHERIC_HEIGHT;

					CG_Trace( tr, testpoint, NULL, NULL, testend, ENTITYNUM_NONE, MASK_ALL );

					if (tr->endpos[2] > ATMOSPHERIC_MAX_MAP_HEIGHT) 
						ATMOSPHERIC_MAX_MAP_HEIGHT = tr->endpos[2];
				}
			}

			// Write newly created info to our map's .mapInfo file for future map loads...
			IniWrite(va("maps/%s.mapInfo", cgs.currentmapname), "MAPINFO", "SKY_HEIGHT", va("%f", ATMOSPHERIC_MAX_MAP_HEIGHT));
		}

		//trap->Print("^3Atmospheric height is at %f.\n", ATMOSPHERIC_MAX_MAP_HEIGHT);
	}

	return ATMOSPHERIC_MAX_MAP_HEIGHT - 128;
}

qboolean CG_AtmosphericBadSpotForParticle ( vec3_t spot )
{
	trace_t tr;
	vec3_t testpoint;
	VectorSet(testpoint, spot[0], spot[1], -MAX_ATMOSPHERIC_HEIGHT);
	CG_Trace( &tr, testpoint, NULL, NULL, spot, ENTITYNUM_NONE, MASK_ALL );
	
	if (tr.fraction == 1 || tr.endpos[2] <= -MAX_ATMOSPHERIC_HEIGHT)
		return qtrue;

	return qfalse;
}

qboolean CG_AtmosphericSkyVisibleFrom ( vec3_t spot, float skyPoint )
{
	trace_t tr;
	vec3_t testpoint;
	VectorSet(testpoint, spot[0], spot[1], MAX_ATMOSPHERIC_HEIGHT/*skyPoint*/);
	CG_Trace( &tr, spot, NULL, NULL, testpoint, cg.clientNum, MASK_SOLID );//MASK_ALL );
	
	if ((tr.fraction == 1 || tr.endpos[2] >= skyPoint-768) && tr.endpos[2] <= ATMOSPHERIC_MAX_MAP_HEIGHT/*MAX_ATMOSPHERIC_HEIGHT*/)
		return qtrue;

	//trap->Print("spot: %f. skyPoint: %f. tr: %f.\n", spot[2], skyPoint, tr.endpos[2]);

	return qfalse;
}

void CG_LightningFlash( vec3_t spot )
{
	// Attempt to 'spot' a lightning flash somewhere below the sky.

	int			choice;
	vec3_t		down = { 0, 0, -1 };
	float		scale = 1.0;
	//vec3_t		lightSpot;

	if (CG_AtmosphericBadSpotForParticle( spot ))
	{// Not here... Outside map...
		//trap->Print("Flash failed at %f %f %f.\n", spot[0], spot[1], spot[2]);
		return;
	}

	if (lightning1 == -1)
	{// Register the effect the first time.
		lightning1 = trap->FX_RegisterEffect("effects/atmospherics/lightning_flash1.efx");
		lightning2 = trap->FX_RegisterEffect("effects/atmospherics/lightning_flash2.efx");
		lightning3 = trap->FX_RegisterEffect("effects/atmospherics/lightning_flash3.efx");
		lightningExplode = trap->FX_RegisterEffect("effects/env/lightning_explode.efx");
	}

	scale = (spot[2] - cg.refdef.vieworg[2]) / 256.0;

	//VectorSet(down, cg.refdef.vieworg[0], cg.refdef.vieworg[1], ATMOSPHERIC_MAX_MAP_HEIGHT);
	//VectorSubtract( cg.refdef.vieworg, down, down );

	choice = rand()%3;

	if (choice == 1)
		trap->FX_PlayEffectID(lightning1, spot, down, 0, scale, qfalse);
	if (choice == 2)
		trap->FX_PlayEffectID(lightning2, spot, down, 0, scale, qfalse);
	else
		trap->FX_PlayEffectID(lightning3, spot, down, 0, scale, qfalse);

	//trap->FX_PlayEffectID(lightningExplode, spot, down, 0, 0, qfalse);

	//trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/atmospherics/huge_lightning.efx"), spot, down, 0, 0, qfalse);
	//trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/atmospherics/lightning_storm_huge.efx"), spot, down, 0, 0, qfalse);

	//trap->Print("Flash OK at %f %f %f.\n", spot[0], spot[1], spot[2]);

	trap->S_StartLocalSound(trap->S_RegisterSound(va("sound/atmospherics/thunder%i.wav", rand()%3)), CHAN_AUTO);
}

qboolean CG_CheckRangedFog( void )
{
	if (ATMOSPHERIC_WEATHER_TYPE == WEATHER_SNOW_STORM)
	{
		trap->R_SetRangedFog(512.0);
		return qtrue;
	}

	return qfalse;
}

void CG_AddVolumetricFog( void )
{
	int i;

	for (i = 0; i < 16; i++)
	{
		vec3_t direction = { 0, 1, 0 };
		vec3_t spot = { ((rand()%512)+cg.refdef.vieworg[0])-256, ((rand()%512)+cg.refdef.vieworg[1])-256, ((rand()%512)+cg.refdef.vieworg[2])-256 };
		trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/atmospherics/fog.efx"), spot, direction, 0, 0, qfalse);
	}
}

void CG_AddAtmosphericEffects()
{
	int MAX_FRAME_PARTICLES = 32;
	int i, sizeX, sizeY, skyHeight;
	trace_t tr;

  	// Add atmospheric effects (e.g. rain, snow etc.) to view
	if (!ATMOSPHERICS_INITIIALIZED)
	{
		CG_AtmosphericKludge();
		ATMOSPHERICS_INITIIALIZED = qtrue;
	}

	if (ATMOSPHERIC_WEATHER_TYPE == WEATHER_NONE)
	{
		return;
	}

	skyHeight = CG_GetSkyHeight(&tr)-128;

	sizeX = 2048;
	sizeY = 2048;

	if (ATMOSPHERIC_WEATHER_TYPE == WEATHER_RAIN_STORM)
	{// Some lightning explosions randomly?
		if (ATMOSPHERIC_NEXT_LIGHTNING_FLASH_TIME <= cg.time)
		{// ready for our next lightning flash...
			vec3_t spot = { ((rand()%8192)+cg.refdef.vieworg[0])-4096, ((rand()%8192)+cg.refdef.vieworg[1])-4096, ATMOSPHERIC_MAX_MAP_HEIGHT-256 };

			if (rand()%6 < 2) // Occasionally we want it to strike twice quickly.
				ATMOSPHERIC_NEXT_LIGHTNING_FLASH_TIME = cg.time + rand()%300;
			else
				ATMOSPHERIC_NEXT_LIGHTNING_FLASH_TIME = cg.time + 1000 + rand()%3000;
		
			CG_LightningFlash(spot);
		}
	}

	switch (ATMOSPHERIC_WEATHER_TYPE)
	{
	case WEATHER_RAIN:
		MAX_FRAME_PARTICLES = 32;
		break;
	case WEATHER_HEAVY_RAIN:
		MAX_FRAME_PARTICLES = 64;//128; // reduced for more FPS
		break;
	case WEATHER_RAIN_STORM:
		MAX_FRAME_PARTICLES = 64;//128; // reduced for more FPS
		break;
	case WEATHER_SNOW:
		MAX_FRAME_PARTICLES = 32;
		break;
	case WEATHER_HEAVY_SNOW:
		MAX_FRAME_PARTICLES = 64;//128; // reduced for more FPS
		break;
	case WEATHER_SNOW_STORM:
		MAX_FRAME_PARTICLES = 64;//128; // reduced for more FPS
		break;
	default:
		MAX_FRAME_PARTICLES = 32;
		break;
	}

	if (cg_atmosphericFrameParticleOverride.integer)
		MAX_FRAME_PARTICLES = cg_atmosphericFrameParticleOverride.integer;

	for (i = 0; i < MAX_FRAME_PARTICLES; i++)
	{
		vec3_t spot = { ((rand()%sizeX)+cg.refdef.vieworg[0])-1024, ((rand()%sizeY)+cg.refdef.vieworg[1])-1024, cg.refdef.vieworg[2]+256 };
		vec3_t down = { 0, 0, -1 };

		//if (CG_AtmosphericBadSpotForParticle( spot ))
		//	continue;

		if (!CG_AtmosphericSkyVisibleFrom ( spot, skyHeight ))
			continue;

		switch (ATMOSPHERIC_WEATHER_TYPE)
		{
		case WEATHER_RAIN:
			trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/atmospherics/atmospheric_rain.efx"), spot, down, 0, 0, qfalse);

			if (ATMOSPHERIC_NEXT_SOUND_TIME <= cg.time)
			{
				trap->S_StartLocalSound(trap->S_RegisterSound("sound/atmospherics/rain.wav"), CHAN_AUTO);
				ATMOSPHERIC_NEXT_SOUND_TIME = cg.time + 27000;
			}
			break;
		case WEATHER_HEAVY_RAIN:
			trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/atmospherics/atmospheric_heavyrain.efx"), spot, down, 0, 0, qfalse);

			if (ATMOSPHERIC_NEXT_SOUND_TIME <= cg.time)
			{
				trap->S_StartLocalSound(trap->S_RegisterSound("sound/atmospherics/heavy_rain.wav"), CHAN_AUTO);
				ATMOSPHERIC_NEXT_SOUND_TIME = cg.time + 27000;
			}
			break;
		case WEATHER_RAIN_STORM:
			trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/atmospherics/atmospheric_storm.efx"), spot, down, 0, 0, qfalse);

			if (ATMOSPHERIC_NEXT_SOUND_TIME <= cg.time)
			{
				trap->S_StartLocalSound(trap->S_RegisterSound("sound/atmospherics/heavy_rain.wav"), CHAN_AUTO);
				ATMOSPHERIC_NEXT_SOUND_TIME = cg.time + 27000;
			}
			break;
		case WEATHER_SNOW:
			trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/atmospherics/atmospheric_snow.efx"), spot, down, 0, 0, qfalse);
			break;
		case WEATHER_HEAVY_SNOW:
			trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/atmospherics/atmospheric_heavysnow.efx"), spot, down, 0, 0, qfalse);
			break;
		case WEATHER_SNOW_STORM:
			trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/atmospherics/atmospheric_snowstorm.efx"), spot, down, 0, 0, qfalse);
			break;
		default:
			break;
		}
	}

	if (ATMOSPHERIC_WEATHER_TYPE == WEATHER_SNOW_STORM)
	{// Would you like some volumetric fog with that? Oh yes please! - This is, however, very FPS costly...
		CG_AddVolumetricFog();
	}
}


/*
**  	G_AtmosphericKludge
*/

static qboolean kludgeChecked, kludgeResult;
qboolean CG_AtmosphericKludge()
{
  	// Activate effects for specified kludge maps that don't
  	// have it specified for them.

	char *atmosphericString = NULL;

  	if( kludgeChecked )
  	  	return( kludgeResult );

  	kludgeChecked = qtrue;
  	kludgeResult = qfalse;

	//
	// Check the ini file with the map...
	//

	atmosphericString = (char*)IniRead(va("maps/%s.atmospherics", cgs.currentmapname), "ATMOSPHERICS", "WEATHER_TYPE", "");

	if (!Q_stricmp(atmosphericString, "rain"))
	{
		trap->Print("^1*** ^3Warzone^5 atmospherics set to ^7rain^5 for this map.\n");
		ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN;
  	  	return( kludgeResult = qtrue );
	}
	else if (!Q_stricmp(atmosphericString, "heavyrain"))
	{
		trap->Print("^1*** ^3Warzone^5 atmospherics set to ^7heavyrain^5 for this map.\n");
		ATMOSPHERIC_WEATHER_TYPE = WEATHER_HEAVY_RAIN;
  	  	return( kludgeResult = qtrue );
	}
	else if (!Q_stricmp(atmosphericString, "rainstorm") || !Q_stricmp(atmosphericString, "storm"))
	{
		trap->Print("^1*** ^3Warzone^5 atmospherics set to ^7rainstorm^5 for this map.\n");
		ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN_STORM;
  	  	return( kludgeResult = qtrue );
	}
	else if (!Q_stricmp(atmosphericString, "snow"))
	{
		trap->Print("^1*** ^3Warzone^5 atmospherics set to ^7snow^5 for this map.\n");
		ATMOSPHERIC_WEATHER_TYPE = WEATHER_SNOW;
  	  	return( kludgeResult = qtrue );
	}
	else if (!Q_stricmp(atmosphericString, "heavysnow"))
	{
		trap->Print("^1*** ^3Warzone^5 atmospherics set to ^7heavysnow^5 for this map.\n");
		ATMOSPHERIC_WEATHER_TYPE = WEATHER_HEAVY_SNOW;
  	  	return( kludgeResult = qtrue );
	}
	else if (!Q_stricmp(atmosphericString, "snowstorm"))
	{
		trap->Print("^1*** ^3Warzone^5 atmospherics set to ^7snowstorm^5 for this map.\n");
		ATMOSPHERIC_WEATHER_TYPE = WEATHER_SNOW_STORM;
  	  	return( kludgeResult = qtrue );
	}

	//
	// And some hard coded maps, if not overridden above...
	//

	if( !Q_stricmp( cgs.currentmapname, "maps/ffa_coruscant" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7rainstorm^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN_STORM;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.currentmapname, "maps/ffa_coruscant" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7rainstorm^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN_STORM;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.currentmapname, "maps/imphoth_a" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7snow^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_SNOW;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.currentmapname, "maps/imphoth_b" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7snow^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_SNOW;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.currentmapname, "maps/jedicouncilgc2" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7rainstorm^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN_STORM;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.currentmapname, "maps/jedicouncilgc" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7rainstorm^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN_STORM;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.currentmapname, "maps/bespinaflstyle3" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7rainstorm^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN_STORM;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.currentmapname, "maps/ffa_kujarforest" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7rain^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.currentmapname, "maps/wookievillage" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7rain^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.currentmapname, "maps/ewok_village" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7rain^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.currentmapname, "maps/coruscant_promenade" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7heavyrain^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_HEAVY_RAIN;
  	  	return( kludgeResult = qtrue );
  	}

	if( StringContainsWord( cgs.currentmapname, "baldemnic" ) && !StringContainsWord( cgs.currentmapname, "baldemnic11" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7rainstorm^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN_STORM;
  	  	return( kludgeResult = qtrue );
  	}

  	return( kludgeResult = qfalse );
}
