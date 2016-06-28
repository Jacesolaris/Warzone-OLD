// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_marks.c -- wall marks

#include "cg_local.h"

qboolean CG_AtmosphericKludge(); // below

/*
===================================================================

MARK POLYS

===================================================================
*/


markPoly_t	cg_activeMarkPolys;			// double linked list
markPoly_t	*cg_freeMarkPolys;			// single linked list
markPoly_t	cg_markPolys[MAX_MARK_POLYS];
static		int	markTotal;

/*
===================
CG_InitMarkPolys

This is called at startup and for tournament restarts
===================
*/
void	CG_InitMarkPolys( void ) {
	int		i;

	memset( cg_markPolys, 0, sizeof(cg_markPolys) );

	cg_activeMarkPolys.nextMark = &cg_activeMarkPolys;
	cg_activeMarkPolys.prevMark = &cg_activeMarkPolys;
	cg_freeMarkPolys = cg_markPolys;
	for ( i = 0 ; i < MAX_MARK_POLYS - 1 ; i++ ) {
		cg_markPolys[i].nextMark = &cg_markPolys[i+1];
	}
}


/*
==================
CG_FreeMarkPoly
==================
*/
void CG_FreeMarkPoly( markPoly_t *le ) {
	if ( !le->prevMark ) {
		trap->Error( ERR_DROP, "CG_FreeLocalEntity: not active" );
	}

	// remove from the doubly linked active list
	le->prevMark->nextMark = le->nextMark;
	le->nextMark->prevMark = le->prevMark;

	// the free list is only singly linked
	le->nextMark = cg_freeMarkPolys;
	cg_freeMarkPolys = le;
}

/*
===================
CG_AllocMark

Will allways succeed, even if it requires freeing an old active mark
===================
*/
markPoly_t	*CG_AllocMark( void ) {
	markPoly_t	*le;
	int time;

	if ( !cg_freeMarkPolys ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		time = cg_activeMarkPolys.prevMark->time;
		while (cg_activeMarkPolys.prevMark && time == cg_activeMarkPolys.prevMark->time) {
			CG_FreeMarkPoly( cg_activeMarkPolys.prevMark );
		}
	}

	le = cg_freeMarkPolys;
	cg_freeMarkPolys = cg_freeMarkPolys->nextMark;

	memset( le, 0, sizeof( *le ) );

	// link into the active list
	le->nextMark = cg_activeMarkPolys.nextMark;
	le->prevMark = &cg_activeMarkPolys;
	cg_activeMarkPolys.nextMark->prevMark = le;
	cg_activeMarkPolys.nextMark = le;
	return le;
}



/*
=================
CG_ImpactMark

origin should be a point within a unit of the plane
dir should be the plane normal

temporary marks will not be stored or randomly oriented, but immediately
passed to the renderer.
=================
*/
#define	MAX_MARK_FRAGMENTS	128
#define	MAX_MARK_POINTS		384

void CG_ImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir,
				   float orientation, float red, float green, float blue, float alpha,
				   qboolean alphaFade, float radius, qboolean temporary ) {
	matrix3_t		axis;
	float			texCoordScale;
	vec3_t			originalPoints[4];
	byte			colors[4];
	int				i, j, k;
	int				numFragments;
	markFragment_t	markFragments[MAX_MARK_FRAGMENTS], *mf;
	vec3_t			markPoints[MAX_MARK_POINTS];
	vec3_t			projection;

	if (!markShader)
	{
#ifdef _DEBUG
		trap->Print("CG_ImpactMark called with no shader.\n");
		//assert(markShader);
#endif //_DEBUG
		return;
	}

	if ( !cg_marks.integer ) {
		return;
	}
	else if (cg_marks.integer == 2)
	{
		trap->R_AddDecalToScene(markShader, origin, dir, orientation, red, green, blue, alpha,
			alphaFade, radius, temporary);
		return;
	}

	if ( radius <= 0 ) {
		trap->Error( ERR_DROP, "CG_ImpactMark called with <= 0 radius" );
	}

	//if ( markTotal >= MAX_MARK_POLYS ) {
	//	return;
	//}

	// create the texture axis
	VectorNormalize2( dir, axis[0] );
	PerpendicularVector( axis[1], axis[0] );
	RotatePointAroundVector( axis[2], axis[0], axis[1], orientation );
	CrossProduct( axis[0], axis[2], axis[1] );

	texCoordScale = 0.5 * 1.0 / radius;

	// create the full polygon
	for ( i = 0 ; i < 3 ; i++ ) {
		originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
		originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
		originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
		originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
	}

	// get the fragments
	VectorScale( dir, -20, projection );
	numFragments = trap->R_MarkFragments( 4, (const vec3_t *) originalPoints, projection, MAX_MARK_POINTS, markPoints[0], MAX_MARK_FRAGMENTS, markFragments );

	colors[0] = red * 255;
	colors[1] = green * 255;
	colors[2] = blue * 255;
	colors[3] = alpha * 255;

	for ( i = 0, mf = markFragments ; i < numFragments ; i++, mf++ ) {
		polyVert_t	*v;
		polyVert_t	verts[MAX_VERTS_ON_POLY];
		markPoly_t	*mark;

		// we have an upper limit on the complexity of polygons
		// that we store persistantly
		if ( mf->numPoints > MAX_VERTS_ON_POLY ) {
			mf->numPoints = MAX_VERTS_ON_POLY;
		}
		for ( j = 0, v = verts ; j < mf->numPoints ; j++, v++ ) {
			vec3_t		delta;

			VectorCopy( markPoints[mf->firstPoint + j], v->xyz );

			VectorSubtract( v->xyz, origin, delta );
			v->st[0] = 0.5 + DotProduct( delta, axis[1] ) * texCoordScale;
			v->st[1] = 0.5 + DotProduct( delta, axis[2] ) * texCoordScale;
			for ( k=0; k<4; k++ )
				v->modulate[k] = colors[k];
		}

		// if it is a temporary (shadow) mark, add it immediately and forget about it
		if ( temporary ) {
			trap->R_AddPolysToScene( markShader, mf->numPoints, verts, 1 );
			continue;
		}

		// otherwise save it persistantly
		mark = CG_AllocMark();
		mark->time = cg.time;
		mark->alphaFade = alphaFade;
		mark->markShader = markShader;
		mark->poly.numVerts = mf->numPoints;
		mark->color[0] = red;
		mark->color[1] = green;
		mark->color[2] = blue;
		mark->color[3] = alpha;
		memcpy( mark->verts, verts, mf->numPoints * sizeof( verts[0] ) );
		markTotal++;
	}
}


/*
===============
CG_AddMarks
===============
*/
#define	MARK_TOTAL_TIME		10000
#define	MARK_FADE_TIME		1000

void CG_AddMarks( void ) {
	int			j;
	markPoly_t	*mp, *next;
	int			t;
	int			fade;

	if ( !cg_marks.integer ) {
		return;
	}

	mp = cg_activeMarkPolys.nextMark;
	for ( ; mp != &cg_activeMarkPolys ; mp = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = mp->nextMark;

		// see if it is time to completely remove it
		if ( cg.time > mp->time + MARK_TOTAL_TIME ) {
			CG_FreeMarkPoly( mp );
			continue;
		}

		// fade out the energy bursts
		//if ( mp->markShader == cgs.media.energyMarkShader ) {
		if (0) {

			fade = 450 - 450 * ( (cg.time - mp->time ) / 3000.0 );
			if ( fade < 255 ) {
				if ( fade < 0 ) {
					fade = 0;
				}
				if ( mp->verts[0].modulate[0] != 0 ) {
					for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
						mp->verts[j].modulate[0] = mp->color[0] * fade;
						mp->verts[j].modulate[1] = mp->color[1] * fade;
						mp->verts[j].modulate[2] = mp->color[2] * fade;
					}
				}
			}
		}

		// fade all marks out with time
		t = mp->time + MARK_TOTAL_TIME - cg.time;
		if ( t < MARK_FADE_TIME ) {
			fade = 255 * t / MARK_FADE_TIME;
			if ( mp->alphaFade ) {
				for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
					mp->verts[j].modulate[3] = fade;
				}
			}
			else
			{
				float f = (float)t / MARK_FADE_TIME;
				for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
					mp->verts[j].modulate[0] = mp->color[0] * f;
					mp->verts[j].modulate[1] = mp->color[1] * f;
					mp->verts[j].modulate[2] = mp->color[2] * f;
				}
			}
		}
		else
		{
			for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
				mp->verts[j].modulate[0] = mp->color[0];
				mp->verts[j].modulate[1] = mp->color[1];
				mp->verts[j].modulate[2] = mp->color[2];
			}
		}

		trap->R_AddPolysToScene( mp->markShader, mp->poly.numVerts, mp->verts, 1 );
	}
}





/*
**  	Brand new (ultra-simple, but looks nice) weather FX using JKA's efx system...
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
	VectorSet(testpoint, spot[0], spot[1], skyPoint);
	CG_Trace( &tr, spot, NULL, NULL, testpoint, cg.clientNum, MASK_SOLID );//MASK_ALL );
	
	if (tr.fraction == 1 || tr.endpos[2] >= skyPoint-768)
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

	trap->FX_PlayEffectID(lightningExplode, spot, down, 0, 0, qfalse);

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
		MAX_FRAME_PARTICLES = 128;
		break;
	case WEATHER_RAIN_STORM:
		MAX_FRAME_PARTICLES = 128;
		break;
	case WEATHER_SNOW:
		MAX_FRAME_PARTICLES = 32;
		break;
	case WEATHER_HEAVY_SNOW:
		MAX_FRAME_PARTICLES = 128;
		break;
	case WEATHER_SNOW_STORM:
		MAX_FRAME_PARTICLES = 128;
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

		if (CG_AtmosphericBadSpotForParticle( spot ))
			continue;

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

	if( !Q_stricmp( cgs.currentmapname, "baldemnic3" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7heavyrain^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_HEAVY_RAIN;
  	  	return( kludgeResult = qtrue );
  	}

	if( StringContainsWord( cgs.currentmapname, "baldemnic" ) )
  	{
		trap->Print("^1*** ^3Warzone^5 atmospherics ^7forced^5 to ^7rainstorm^5 for this map.\n");
  	  	ATMOSPHERIC_WEATHER_TYPE = WEATHER_RAIN_STORM;
  	  	return( kludgeResult = qtrue );
  	}

  	return( kludgeResult = qfalse );
}

