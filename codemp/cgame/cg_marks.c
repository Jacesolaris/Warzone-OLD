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
**  	Add atmospheric effects to view.
**
**  	Current supported effects are rain and snow.
*/

/*

// The default values.
#define MAX_ATMOSPHERIC_PARTICLES  	  	1000  	// maximum # of particles
#define MAX_ATMOSPHERIC_DISTANCE  	  	1000  	// maximum distance from refdef origin that particles are visible
#define MAX_ATMOSPHERIC_HEIGHT  	  	4096  	// maximum world height (FIXME: since 1.27 this should be 65536)
#define MIN_ATMOSPHERIC_HEIGHT  	  	-4096  	// minimum world height (FIXME: since 1.27 this should be -65536)
#define MAX_ATMOSPHERIC_EFFECTSHADERS  	6  	  	// maximum different effectshaders for an atmospheric effect
#define ATMOSPHERIC_DROPDELAY  	  	  	1000
#define ATMOSPHERIC_CUTHEIGHT  	  	  	800


#define ATMOSPHERIC_RAIN_SPEED  	  	1.1f * DEFAULT_GRAVITY
#define ATMOSPHERIC_RAIN_HEIGHT  	  	150

#define ATMOSPHERIC_SNOW_SPEED  	  	0.1f * DEFAULT_GRAVITY
#define ATMOSPHERIC_SNOW_HEIGHT  	  	10

*/

/*efine MAX_ATMOSPHERIC_PARTICLES  	  	2000  	// maximum # of particles
#define MAX_ATMOSPHERIC_DISTANCE  	  	1000  	// maximum distance from refdef origin that particles are visible
#define MAX_ATMOSPHERIC_HEIGHT  	  	4096  	// maximum world height (FIXME: since 1.27 this should be 65536)
#define MIN_ATMOSPHERIC_HEIGHT  	  	-4096  	// minimum world height (FIXME: since 1.27 this should be -65536)
#define MAX_ATMOSPHERIC_EFFECTSHADERS  	6  	  	// maximum different effectshaders for an atmospheric effect
#define ATMOSPHERIC_RAIN_DROPDELAY  	100
#define ATMOSPHERIC_SNOW_DROPDELAY  	1000
#define ATMOSPHERIC_CUTHEIGHT  	  	  	800*/

#define MAX_ATMOSPHERIC_PARTICLES  	  	10000  	// maximum # of particles
#define MAX_ATMOSPHERIC_DISTANCE  	  	2000  	// maximum distance from refdef origin that particles are visible
#define MAX_ATMOSPHERIC_HEIGHT  	  	65536//8096  	// maximum world height (FIXME: since 1.27 this should be 65536)
#define MIN_ATMOSPHERIC_HEIGHT  	  	-65536//-8096  	// minimum world height (FIXME: since 1.27 this should be -65536)
#define MAX_ATMOSPHERIC_EFFECTSHADERS  	6  	  	// maximum different effectshaders for an atmospheric effect
#define ATMOSPHERIC_RAIN_DROPDELAY  	30
#define ATMOSPHERIC_SNOW_DROPDELAY  	30
#define ATMOSPHERIC_CUTHEIGHT  	  	  	800

#define ATMOSPHERIC_RAIN_SPEED  	  	2.1f * DEFAULT_GRAVITY
#define ATMOSPHERIC_RAIN_HEIGHT  	  	150

#define ATMOSPHERIC_STORM_SPEED  	  	2.9f * DEFAULT_GRAVITY
#define ATMOSPHERIC_STORM_HEIGHT  	  	150

#define ATMOSPHERIC_SNOW_SPEED  	  	0.1f * DEFAULT_GRAVITY
#define ATMOSPHERIC_SNOW_HEIGHT  	  	10

#define ATMOSPHERIC_HEAVY_SNOW_SPEED  	0.25f * DEFAULT_GRAVITY
#define ATMOSPHERIC_HEAVY_SNOW_HEIGHT  	10


typedef struct cg_atmosphericParticle_s {
  	vec3_t pos, delta, deltaNormalized, colour, surfacenormal;
  	float height, minz, weight;
  	qboolean active;
  	int contents, surface, nextDropTime;
  	qhandle_t *effectshader;
} cg_atmosphericParticle_t;

typedef struct cg_atmosphericEffect_s {
  	cg_atmosphericParticle_t particles[MAX_ATMOSPHERIC_PARTICLES];
  	qhandle_t effectshaders[MAX_ATMOSPHERIC_EFFECTSHADERS];
  	qhandle_t effectwatershader, effectlandshader;
  	int lastRainTime, numDrops;
  	int gustStartTime, gustEndTime;
  	int baseStartTime, baseEndTime;
  	int gustMinTime, gustMaxTime;
  	int changeMinTime, changeMaxTime;
  	int baseMinTime, baseMaxTime;
  	float baseWeight, gustWeight;
  	int baseDrops, gustDrops;
  	int numEffectShaders;
  	qboolean waterSplash, landSplash;
  	vec3_t baseVec, gustVec;

  	qboolean (*ParticleCheckVisible)( cg_atmosphericParticle_t *particle );
  	qboolean (*ParticleGenerate)( cg_atmosphericParticle_t *particle, vec3_t currvec, float currweight );
  	void (*ParticleRender)( cg_atmosphericParticle_t *particle );
} cg_atmosphericEffect_t;

static cg_atmosphericEffect_t cg_atmFx;

/*
**  Render utility functions
*/

void CG_EffectMark(  	qhandle_t markShader, const vec3_t origin, const vec3_t dir, float alpha, float radius ) {
  	// 'quick' version of the CG_ImpactMark function

  	vec3_t  	  	  	axis[3];
  	float  	  	  	texCoordScale;
  	vec3_t  	  	  	originalPoints[4];
  	byte  	  	  	colors[4];
  	int  	  	  	  	i;
  	polyVert_t  	  	*v;
  	polyVert_t  	  	verts[4];

  	if ( !cg_marks.integer ) {
  	  	return;
  	}

  	if ( radius <= 0 ) {
  	  	//trap->Print( "CG_EffectMark called with <= 0 radius\n" );
		return;
  	}

  	// create the texture axis
  	VectorNormalize2( dir, axis[0] );
  	PerpendicularVector( axis[1], axis[0] );
  	VectorSet( axis[2], 1, 0, 0 );  	  	  	// This is _wrong_, but the function is for water anyway (i.e. usually flat)
  	CrossProduct( axis[0], axis[2], axis[1] );

  	texCoordScale = 0.5 * 1.0 / radius;

  	// create the full polygon
  	for ( i = 0 ; i < 3 ; i++ ) {
  	  	originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
  	  	originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
  	  	originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
  	  	originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
  	}

  	colors[0] = 127;
  	colors[1] = 127;
  	colors[2] = 127;
  	colors[3] = alpha * 255;

  	for ( i = 0, v = verts ; i < 4 ; i++, v++ ) {
  	  	vec3_t  	  	delta;

  	  	VectorCopy( originalPoints[i], v->xyz );

  	  	VectorSubtract( v->xyz, origin, delta );
  	  	v->st[0] = 0.5 + DotProduct( delta, axis[1] ) * texCoordScale;
  	  	v->st[1] = 0.5 + DotProduct( delta, axis[2] ) * texCoordScale;
  	  	*(int *)v->modulate = *(int *)colors;
  	}

	trap->R_AddPolysToScene( markShader, 4, verts, 1 );
}

/*
**  	Raindrop management functions
*/

static qboolean CG_RainParticleCheckVisible( cg_atmosphericParticle_t *particle )
{
  	// Check the raindrop is visible and still going, wrapping if necessary.

  	float moved;
  	vec3_t distance;

  	if( !particle || !particle->active )
  	  	return( qfalse );

  	moved = (cg.time - cg_atmFx.lastRainTime) * 0.001;  	// Units moved since last frame
  	VectorMA( particle->pos, moved, particle->delta, particle->pos );
  	if( particle->pos[2] + ATMOSPHERIC_CUTHEIGHT < particle->minz )
  	  	return( particle->active = qfalse );

  	VectorSubtract( cg.refdef.vieworg, particle->pos, distance );
  	if( sqrt( distance[0] * distance[0] + distance[1] * distance[1] ) > MAX_ATMOSPHERIC_DISTANCE )
  	  	return( particle->active = qfalse );

  	return( qtrue );
}

static qboolean CG_RainParticleGenerate( cg_atmosphericParticle_t *particle, vec3_t currvec, float currweight )
{
  	// Attempt to 'spot' a raindrop somewhere below a sky texture.

  	float angle, distance, origz;
  	vec3_t testpoint, testend;
  	trace_t tr;

  	angle = random() * 2*M_PI;
  	distance = 20 + MAX_ATMOSPHERIC_DISTANCE * random();

  	testpoint[0] = testend[0] = cg.refdef.vieworg[0] + sin(angle) * distance;
  	testpoint[1] = testend[1] = cg.refdef.vieworg[1] + cos(angle) * distance;
  	testpoint[2] = origz = cg.refdef.vieworg[2];
  	testend[2] = testpoint[2] + MAX_ATMOSPHERIC_HEIGHT;

  	while( 1 )
  	{
  	  	if( testpoint[2] >= MAX_ATMOSPHERIC_HEIGHT )
  	  	  	return( qfalse );
  	  	if( testend[2] >= MAX_ATMOSPHERIC_HEIGHT )
  	  	  	testend[2] = MAX_ATMOSPHERIC_HEIGHT - 1;
  	  	CG_Trace( &tr, testpoint, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
  	  	if( tr.startsolid || tr.allsolid )  	  	  	// Stuck in something, skip over it.
  	  	{
  	  	  	testpoint[2] += 64;
  	  	  	testend[2] = testpoint[2] + MAX_ATMOSPHERIC_HEIGHT;
  	  	}
  	  	else if( tr.fraction == 1 )  	  	// Didn't hit anything, we're (probably) outside the world
		{
  	  	  	return( qfalse );
		}
  	  	else if( tr.surfaceFlags & SURF_SKY )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NOIMPACT) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOIMPACT) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
  	  	else 
		{
			return( qfalse );
		}
  	}

  	particle->active = qtrue;
  	particle->colour[0] = 0.6 + 0.2 * random();
  	particle->colour[1] = 0.6 + 0.2 * random();
  	particle->colour[2] = 0.6 + 0.2 * random();
  	VectorCopy( tr.endpos, particle->pos );
  	VectorCopy( currvec, particle->delta );
  	particle->delta[2] += crandom() * 100;
  	VectorNormalize2( particle->delta, particle->deltaNormalized );
  	particle->height = ATMOSPHERIC_RAIN_HEIGHT + crandom() * 100;
  	particle->weight = currweight;
  	particle->effectshader = &cg_atmFx.effectshaders[0];

  	distance =  	((float)(tr.endpos[2] - MIN_ATMOSPHERIC_HEIGHT)) / -particle->delta[2];
  	VectorMA( tr.endpos, distance, particle->delta, testend );

  	CG_Trace( &tr, particle->pos, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
  	particle->minz = tr.endpos[2];
  	tr.endpos[2]--;
  	VectorCopy( tr.plane.normal, particle->surfacenormal );
  	particle->surface = tr.surfaceFlags;
  	particle->contents = CG_PointContents( tr.endpos, ENTITYNUM_NONE );

  	return( qtrue );
}

static void CG_RainParticleRender( cg_atmosphericParticle_t *particle )
{
  	// Draw a raindrop

  	vec3_t  	  	forward, right;
  	polyVert_t  	verts[4];
  	vec2_t  	  	line;
  	float  	  	len, frac;
  	vec3_t  	  	start, finish;

  	if( !particle->active )
  	  	return;

  	VectorCopy( particle->pos, start );
  	len = particle->height;

	if (rand()%100 < 2)
	{
		//trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/rain.efx"), start, particle->deltaNormalized);
		trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/fog.efx"), start, particle->deltaNormalized, 0, 0, qfalse);
		//particle->active = qfalse;
		//return;
	}

  	if( start[2] <= particle->minz )
  	{
  	  	// Stop rain going through surfaces.
  	  	len = particle->height - particle->minz + start[2];
  	  	frac = start[2];
  	  	VectorMA( start, len - particle->height, particle->deltaNormalized, start );

  	  	if( !cg_lowEffects.integer )
  	  	{
  	  	  	frac = (ATMOSPHERIC_CUTHEIGHT - particle->minz + frac) / (float) ATMOSPHERIC_CUTHEIGHT;
  	  	  	// Splash effects on different surfaces
  	  	  	if( particle->contents & (CONTENTS_WATER|CONTENTS_SLIME) )
  	  	  	{
  	  	  	  	// Water splash
  	  	  	  	if( cg_atmFx.effectwatershader && frac > 0 && frac < 1 )
  	  	  	  	  	CG_EffectMark( cg_atmFx.effectwatershader, start, particle->surfacenormal, frac * 0.5, 8 - frac * 8 );
  	  	  	}
  	  	  	else if( !(particle->contents & CONTENTS_LAVA) && !(particle->surface & (SURF_NODAMAGE|SURF_NOIMPACT|SURF_NOMARKS|SURF_SKY)) )
  	  	  	{
  	  	  	  	// Solid splash
  	  	  	  	if( cg_atmFx.effectlandshader && frac > 0 && frac < 1  )
  	  	  	  	  	//CG_ImpactMark( cg_atmFx.effectlandshader, start, particle->surfacenormal, 0, 1, 1, 1, frac * 0.5, qfalse, 3 - frac * 2, qtrue );
				{
					if (rand()%50 < 2)
					{
						trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/water_splash_rain.efx"), start, particle->deltaNormalized, 0, 0, qfalse);
						return;
					}

					CG_ImpactMark( cg_atmFx.effectlandshader, start, particle->surfacenormal, 0, 1, 1, 1, frac * 0.5, qfalse, 3 - frac * 2, qtrue );
				}
  	  	  	}
  	  	}
  	}
  	if( len <= 0 )
  	  	return;

  	VectorCopy( particle->deltaNormalized, forward );
  	VectorMA( start, -len, forward, finish );

  	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
  	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

  	VectorScale( cg.refdef.viewaxis[1], line[1], right );
  	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
  	VectorNormalize( right );

  	VectorMA( finish, particle->weight, right, verts[0].xyz );
  	verts[0].st[0] = 1;
  	verts[0].st[1] = 0;
  	verts[0].modulate[0] = 255;
  	verts[0].modulate[1] = 255;
  	verts[0].modulate[2] = 255;
  	verts[0].modulate[3] = 0;

  	VectorMA( finish, -particle->weight, right, verts[1].xyz );
  	verts[1].st[0] = 0;
  	verts[1].st[1] = 0;
  	verts[1].modulate[0] = 255;
  	verts[1].modulate[1] = 255;
  	verts[1].modulate[2] = 255;
  	verts[1].modulate[3] = 0;

  	VectorMA( start, -particle->weight, right, verts[2].xyz );
  	verts[2].st[0] = 0;
  	verts[2].st[1] = 1;
  	verts[2].modulate[0] = 255;
  	verts[2].modulate[1] = 255;
  	verts[2].modulate[2] = 255;
  	verts[2].modulate[3] = 127;

  	VectorMA( start, particle->weight, right, verts[3].xyz );
  	verts[3].st[0] = 1;
  	verts[3].st[1] = 1;
  	verts[3].modulate[0] = 255;
  	verts[3].modulate[1] = 255;
  	verts[3].modulate[2] = 255;
  	verts[3].modulate[3] = 127;

  	trap->R_AddPolysToScene( *particle->effectshader, 4, verts, 1 );
}

/*
**  	Storm management functions
*/

static qboolean CG_StormParticleGenerate( cg_atmosphericParticle_t *particle, vec3_t currvec, float currweight )
{
  	// Attempt to 'spot' a raindrop somewhere below a sky texture.

  	float angle, distance, origz;
  	vec3_t testpoint, testend;
  	trace_t tr;

  	angle = random() * 2*M_PI;
  	distance = 20 + MAX_ATMOSPHERIC_DISTANCE * random();

  	testpoint[0] = testend[0] = cg.refdef.vieworg[0] + sin(angle) * distance;
  	testpoint[1] = testend[1] = cg.refdef.vieworg[1] + cos(angle) * distance;
  	testpoint[2] = origz = cg.refdef.vieworg[2];
  	testend[2] = testpoint[2] + MAX_ATMOSPHERIC_HEIGHT;

  	while( 1 )
  	{
  	  	if( testpoint[2] >= MAX_ATMOSPHERIC_HEIGHT )
  	  	  	return( qfalse );
  	  	if( testend[2] >= MAX_ATMOSPHERIC_HEIGHT )
  	  	  	testend[2] = MAX_ATMOSPHERIC_HEIGHT - 1;
  	  	CG_Trace( &tr, testpoint, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
  	  	if( tr.startsolid || tr.allsolid )  	  	  	// Stuck in something, skip over it.
  	  	{
  	  	  	testpoint[2] += 64;
  	  	  	testend[2] = testpoint[2] + MAX_ATMOSPHERIC_HEIGHT;
  	  	}
  	  	else if( tr.fraction == 1 )  	  	// Didn't hit anything, we're (probably) outside the world
		{
  	  	  	return( qfalse );
		}
  	  	else if( tr.surfaceFlags & SURF_SKY )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NOIMPACT) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOIMPACT) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
  	  	else 
		{
			return( qfalse );
		}
  	}

  	particle->active = qtrue;
  	particle->colour[0] = 0.6 + 0.2 * random();
  	particle->colour[1] = 0.6 + 0.2 * random();
  	particle->colour[2] = 0.6 + 0.2 * random();
  	VectorCopy( tr.endpos, particle->pos );
  	VectorCopy( currvec, particle->delta );
  	particle->delta[2] += crandom() * 100;
  	VectorNormalize2( particle->delta, particle->deltaNormalized );
  	particle->height = ATMOSPHERIC_STORM_HEIGHT + crandom() * 100;
  	particle->weight = currweight;
  	particle->effectshader = &cg_atmFx.effectshaders[0];

  	distance =  	((float)(tr.endpos[2] - MIN_ATMOSPHERIC_HEIGHT)) / -particle->delta[2];
  	VectorMA( tr.endpos, distance, particle->delta, testend );

  	CG_Trace( &tr, particle->pos, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
  	particle->minz = tr.endpos[2];
  	tr.endpos[2]--;
  	VectorCopy( tr.plane.normal, particle->surfacenormal );
  	particle->surface = tr.surfaceFlags;
  	particle->contents = CG_PointContents( tr.endpos, ENTITYNUM_NONE );

  	return( qtrue );
}

static void CG_StormParticleRender( cg_atmosphericParticle_t *particle )
{
  	// Draw a raindrop

  	vec3_t  	  	forward, right;
  	polyVert_t  	verts[4];
  	vec2_t  	  	line;
  	float  	  	len, frac;
  	vec3_t  	  	start, finish;

  	if( !particle->active )
  	  	return;

  	VectorCopy( particle->pos, start );
  	len = particle->height;
	if (rand()%100 < 2)
	{
		//trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/rain.efx"), start, particle->deltaNormalized);
		trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/fog.efx"), start, particle->deltaNormalized, 0, 0, qfalse);
		//particle->active = qfalse;
		//return;
	}

  	if( start[2] <= particle->minz )
  	{
  	  	// Stop rain going through surfaces.
  	  	len = particle->height - particle->minz + start[2];
  	  	frac = start[2];
  	  	VectorMA( start, len - particle->height, particle->deltaNormalized, start );

  	  	if( !cg_lowEffects.integer )
  	  	{
  	  	  	frac = (ATMOSPHERIC_CUTHEIGHT - particle->minz + frac) / (float) ATMOSPHERIC_CUTHEIGHT;
  	  	  	// Splash effects on different surfaces
  	  	  	if( particle->contents & (CONTENTS_WATER|CONTENTS_SLIME) )
  	  	  	{
  	  	  	  	// Water splash
  	  	  	  	if( cg_atmFx.effectwatershader && frac > 0 && frac < 1 )
  	  	  	  	  	CG_EffectMark( cg_atmFx.effectwatershader, start, particle->surfacenormal, frac * 0.5, 8 - frac * 8 );
  	  	  	}
  	  	  	else if( !(particle->contents & CONTENTS_LAVA) && !(particle->surface & (SURF_NODAMAGE|SURF_NOIMPACT|SURF_NOMARKS|SURF_SKY)) )
  	  	  	{
  	  	  	  	// Solid splash
  	  	  	  	if( cg_atmFx.effectlandshader && frac > 0 && frac < 1  )
				{
					if (rand()%100 < 2)
					{
						trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/water_splash_rain.efx"), start, particle->deltaNormalized, 0, 0, qfalse);
						return;
					}
					CG_ImpactMark( cg_atmFx.effectlandshader, start, particle->surfacenormal, 0, 1, 1, 1, frac * 0.5, qfalse, 3 - frac * 2, qtrue );
				}
  	  	  	}
  	  	}
  	}
  	if( len <= 0 )
  	  	return;

  	VectorCopy( particle->deltaNormalized, forward );
  	VectorMA( start, -len, forward, finish );

  	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
  	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

  	VectorScale( cg.refdef.viewaxis[1], line[1], right );
  	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
  	VectorNormalize( right );

  	VectorMA( finish, particle->weight, right, verts[0].xyz );
  	verts[0].st[0] = 1;
  	verts[0].st[1] = 0;
  	verts[0].modulate[0] = 255;
  	verts[0].modulate[1] = 255;
  	verts[0].modulate[2] = 255;
  	verts[0].modulate[3] = 0;

  	VectorMA( finish, -particle->weight, right, verts[1].xyz );
  	verts[1].st[0] = 0;
  	verts[1].st[1] = 0;
  	verts[1].modulate[0] = 255;
  	verts[1].modulate[1] = 255;
  	verts[1].modulate[2] = 255;
  	verts[1].modulate[3] = 0;

  	VectorMA( start, -particle->weight, right, verts[2].xyz );
  	verts[2].st[0] = 0;
  	verts[2].st[1] = 1;
  	verts[2].modulate[0] = 255;
  	verts[2].modulate[1] = 255;
  	verts[2].modulate[2] = 255;
  	verts[2].modulate[3] = 127;

  	VectorMA( start, particle->weight, right, verts[3].xyz );
  	verts[3].st[0] = 1;
  	verts[3].st[1] = 1;
  	verts[3].modulate[0] = 255;
  	verts[3].modulate[1] = 255;
  	verts[3].modulate[2] = 255;
  	verts[3].modulate[3] = 127;

  	trap->R_AddPolysToScene( *particle->effectshader, 4, verts, 1 );
}


static void CG_MeteorParticleRender( cg_atmosphericParticle_t *particle )
{
  	// Draw a raindrop

  	float  	  	len;
  	vec3_t  	start;

  	if( !particle->active )
  	  	return;

  	VectorCopy( particle->pos, start );
  	len = particle->height;
	if (rand()%150 < 2)
	{
		trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/meteor_big.efx"), start, particle->deltaNormalized, 0, 0, qfalse);
		particle->active = qfalse;
		return;
	}
}

/*
**  	Snow management functions
*/

static qboolean CG_SnowParticleGenerate( cg_atmosphericParticle_t *particle, vec3_t currvec, float currweight )
{
  	// Attempt to 'spot' a raindrop somewhere below a sky texture.

  	float angle, distance, origz;
  	vec3_t testpoint, testend;
  	trace_t tr;

  	angle = random() * 2*M_PI;
  	distance = 20 + MAX_ATMOSPHERIC_DISTANCE * random();

  	testpoint[0] = testend[0] = cg.refdef.vieworg[0] + sin(angle) * distance;
  	testpoint[1] = testend[1] = cg.refdef.vieworg[1] + cos(angle) * distance;
  	testpoint[2] = origz = cg.refdef.vieworg[2];
  	testend[2] = testpoint[2] + MAX_ATMOSPHERIC_HEIGHT;

  	while( 1 )
  	{
  	  	if( testpoint[2] >= MAX_ATMOSPHERIC_HEIGHT )
  	  	  	return( qfalse );
  	  	if( testend[2] >= MAX_ATMOSPHERIC_HEIGHT )
  	  	  	testend[2] = MAX_ATMOSPHERIC_HEIGHT - 1;
  	  	CG_Trace( &tr, testpoint, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
  	  	if( tr.startsolid || tr.allsolid )  	  	  	// Stuck in something, skip over it.
  	  	{
  	  	  	testpoint[2] += 64;
  	  	  	testend[2] = testpoint[2] + MAX_ATMOSPHERIC_HEIGHT;
  	  	}
  	  	else if( tr.fraction == 1 )  	  	// Didn't hit anything, we're (probably) outside the world
		{
  	  	  	return( qfalse );
		}
  	  	else if( tr.surfaceFlags & SURF_SKY )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NOIMPACT) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOIMPACT) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
  	  	else 
		{
			return( qfalse );
		}
  	}

  	particle->active = qtrue;
  	particle->colour[0] = 0.6 + 0.2 * random();
  	particle->colour[1] = 0.6 + 0.2 * random();
  	particle->colour[2] = 0.6 + 0.2 * random();
  	VectorCopy( tr.endpos, particle->pos );
  	VectorCopy( currvec, particle->delta );
  	particle->delta[2] += crandom() * 25;
  	VectorNormalize2( particle->delta, particle->deltaNormalized );
  	particle->height = ATMOSPHERIC_SNOW_HEIGHT + crandom() * 8;
  	particle->weight = particle->height * 0.5f;
  	particle->effectshader = &cg_atmFx.effectshaders[ (int) (random() * ( cg_atmFx.numEffectShaders - 1 )) ];

  	distance =  	((float)(tr.endpos[2] - MIN_ATMOSPHERIC_HEIGHT)) / -particle->delta[2];
  	VectorMA( tr.endpos, distance, particle->delta, testend );
  	CG_Trace( &tr, particle->pos, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
  	particle->minz = tr.endpos[2];
  	tr.endpos[2]--;
  	VectorCopy( tr.plane.normal, particle->surfacenormal );
  	particle->surface = tr.surfaceFlags;
  	particle->contents = CG_PointContents( tr.endpos, ENTITYNUM_NONE );

  	return( qtrue );
}

static void CG_SnowParticleRender( cg_atmosphericParticle_t *particle )
{
  	// Draw a snowflake

  	vec3_t  	  	forward, right;
  	polyVert_t  	verts[4];
  	vec2_t  	  	line;
  	float  	  	len, frac, sinTumbling, cosTumbling, particleWidth;
  	vec3_t  	  	start, finish;

  	if( !particle->active )
  	  	return;

  	VectorCopy( particle->pos, start );

  	sinTumbling = sin( particle->pos[2] * 0.03125f );
  	cosTumbling = cos( ( particle->pos[2] + particle->pos[1] )  * 0.03125f );

  	start[0] += 24 * ( 1 - particle->deltaNormalized[2] ) * sinTumbling;
  	start[1] += 24 * ( 1 - particle->deltaNormalized[2] ) * cosTumbling;

  	len = particle->height;
  	if( start[2] <= particle->minz )
  	{
  	  	// Stop snow going through surfaces.
  	  	len = particle->height - particle->minz + start[2];
  	  	frac = start[2];
  	  	VectorMA( start, len - particle->height, particle->deltaNormalized, start );
  	}
  	if( len <= 0 )
  	  	return;

  	VectorCopy( particle->deltaNormalized, forward );
  	VectorMA( start, -( len * sinTumbling ), forward, finish );

  	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
  	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

  	VectorScale( cg.refdef.viewaxis[1], line[1], right );
  	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
  	VectorNormalize( right );

  	particleWidth = cosTumbling * particle->weight;

  	VectorMA( finish, particleWidth, right, verts[0].xyz );
  	verts[0].st[0] = 1;
  	verts[0].st[1] = 0;
  	verts[0].modulate[0] = 255;
  	verts[0].modulate[1] = 255;
  	verts[0].modulate[2] = 255;
  	verts[0].modulate[3] = 255;

  	VectorMA( finish, -particleWidth, right, verts[1].xyz );
  	verts[1].st[0] = 0;
  	verts[1].st[1] = 0;
  	verts[1].modulate[0] = 255;
  	verts[1].modulate[1] = 255;
  	verts[1].modulate[2] = 255;
  	verts[1].modulate[3] = 255;

  	VectorMA( start, -particleWidth, right, verts[2].xyz );
  	verts[2].st[0] = 0;
  	verts[2].st[1] = 1;
  	verts[2].modulate[0] = 255;
  	verts[2].modulate[1] = 255;
  	verts[2].modulate[2] = 255;
  	verts[2].modulate[3] = 255;

  	VectorMA( start, particleWidth, right, verts[3].xyz );
  	verts[3].st[0] = 1;
  	verts[3].st[1] = 1;
  	verts[3].modulate[0] = 255;
  	verts[3].modulate[1] = 255;
  	verts[3].modulate[2] = 255;
  	verts[3].modulate[3] = 255;

	if (rand()%600 < 2)
	{
		//trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/snow.efx"), start, particle->deltaNormalized);
		trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/fog.efx"), start, particle->deltaNormalized, 0, 0, qfalse);
		//particle->active = qfalse;
		//return;
	}
  	trap->R_AddPolysToScene( *particle->effectshader, 4, verts, 1 );
}

/*
**  	Heavy Snow management functions
*/

static qboolean CG_HeavySnowParticleGenerate( cg_atmosphericParticle_t *particle, vec3_t currvec, float currweight )
{
  	// Attempt to 'spot' a raindrop somewhere below a sky texture.

  	float angle, distance, origz;
  	vec3_t testpoint, testend;
  	trace_t tr;

  	angle = random() * 2*M_PI;
  	distance = 20 + MAX_ATMOSPHERIC_DISTANCE * random();

  	testpoint[0] = testend[0] = cg.refdef.vieworg[0] + sin(angle) * distance;
  	testpoint[1] = testend[1] = cg.refdef.vieworg[1] + cos(angle) * distance;
  	testpoint[2] = origz = cg.refdef.vieworg[2];
  	testend[2] = testpoint[2] + MAX_ATMOSPHERIC_HEIGHT;

  	while( 1 )
  	{
  	  	if( testpoint[2] >= MAX_ATMOSPHERIC_HEIGHT )
  	  	  	return( qfalse );
  	  	if( testend[2] >= MAX_ATMOSPHERIC_HEIGHT )
  	  	  	testend[2] = MAX_ATMOSPHERIC_HEIGHT - 1;
  	  	CG_Trace( &tr, testpoint, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
  	  	if( tr.startsolid || tr.allsolid )  	  	  	// Stuck in something, skip over it.
  	  	{
  	  	  	testpoint[2] += 64;
  	  	  	testend[2] = testpoint[2] + MAX_ATMOSPHERIC_HEIGHT;
  	  	}
  	  	else if( tr.fraction == 1 )  	  	// Didn't hit anything, we're (probably) outside the world
		{
  	  	  	return( qfalse );
		}
  	  	else if( tr.surfaceFlags & SURF_SKY )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NOIMPACT) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOIMPACT) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
  	  	else 
		{
			return( qfalse );
		}
  	}

  	particle->active = qtrue;
  	particle->colour[0] = 0.6 + 0.2 * random();
  	particle->colour[1] = 0.6 + 0.2 * random();
  	particle->colour[2] = 0.6 + 0.2 * random();
  	VectorCopy( tr.endpos, particle->pos );
  	VectorCopy( currvec, particle->delta );
  	particle->delta[2] += crandom() * 25;
  	VectorNormalize2( particle->delta, particle->deltaNormalized );
  	particle->height = ATMOSPHERIC_HEAVY_SNOW_HEIGHT + crandom() * 8;
  	particle->weight = particle->height * 0.5f;
  	particle->effectshader = &cg_atmFx.effectshaders[ (int) (random() * ( cg_atmFx.numEffectShaders - 1 )) ];

  	distance =  	((float)(tr.endpos[2] - MIN_ATMOSPHERIC_HEIGHT)) / -particle->delta[2];
  	VectorMA( tr.endpos, distance, particle->delta, testend );
  	CG_Trace( &tr, particle->pos, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
  	particle->minz = tr.endpos[2];
  	tr.endpos[2]--;
  	VectorCopy( tr.plane.normal, particle->surfacenormal );
  	particle->surface = tr.surfaceFlags;
  	particle->contents = CG_PointContents( tr.endpos, ENTITYNUM_NONE );

  	return( qtrue );
}

static void CG_HeavySnowParticleRender( cg_atmosphericParticle_t *particle )
{
  	// Draw a snowflake

  	vec3_t  	  	forward, right;
  	polyVert_t  	verts[4];
  	vec2_t  	  	line;
  	float  	  	len, frac, sinTumbling, cosTumbling, particleWidth;
  	vec3_t  	  	start, finish;

  	if( !particle->active )
  	  	return;

  	VectorCopy( particle->pos, start );

  	sinTumbling = sin( particle->pos[2] * 0.03125f );
  	cosTumbling = cos( ( particle->pos[2] + particle->pos[1] )  * 0.03125f );

  	start[0] += 24 * ( 1 - particle->deltaNormalized[2] ) * sinTumbling;
  	start[1] += 24 * ( 1 - particle->deltaNormalized[2] ) * cosTumbling;

  	len = particle->height;
  	if( start[2] <= particle->minz )
  	{
  	  	// Stop snow going through surfaces.
  	  	len = particle->height - particle->minz + start[2];
  	  	frac = start[2];
  	  	VectorMA( start, len - particle->height, particle->deltaNormalized, start );
  	}
  	if( len <= 0 )
  	  	return;

  	VectorCopy( particle->deltaNormalized, forward );
  	VectorMA( start, -( len * sinTumbling ), forward, finish );

  	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
  	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

  	VectorScale( cg.refdef.viewaxis[1], line[1], right );
  	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
  	VectorNormalize( right );

  	particleWidth = cosTumbling * particle->weight;

  	VectorMA( finish, particleWidth, right, verts[0].xyz );
  	verts[0].st[0] = 1;
  	verts[0].st[1] = 0;
  	verts[0].modulate[0] = 255;
  	verts[0].modulate[1] = 255;
  	verts[0].modulate[2] = 255;
  	verts[0].modulate[3] = 255;

  	VectorMA( finish, -particleWidth, right, verts[1].xyz );
  	verts[1].st[0] = 0;
  	verts[1].st[1] = 0;
  	verts[1].modulate[0] = 255;
  	verts[1].modulate[1] = 255;
  	verts[1].modulate[2] = 255;
  	verts[1].modulate[3] = 255;

  	VectorMA( start, -particleWidth, right, verts[2].xyz );
  	verts[2].st[0] = 0;
  	verts[2].st[1] = 1;
  	verts[2].modulate[0] = 255;
  	verts[2].modulate[1] = 255;
  	verts[2].modulate[2] = 255;
  	verts[2].modulate[3] = 255;

  	VectorMA( start, particleWidth, right, verts[3].xyz );
  	verts[3].st[0] = 1;
  	verts[3].st[1] = 1;
  	verts[3].modulate[0] = 255;
  	verts[3].modulate[1] = 255;
  	verts[3].modulate[2] = 255;
  	verts[3].modulate[3] = 255;

	if (rand()%70 < 2)
	{
		//trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/snow.efx"), start, particle->deltaNormalized);
		trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/fog.efx"), start, particle->deltaNormalized, 0, 0, qfalse);
		//particle->active = qfalse;
		return;
	}
  	trap->R_AddPolysToScene( *particle->effectshader, 4, verts, 1 );
}

/*
**  	Set up gust parameters.
*/

static void CG_EffectGust()
{
  	// Generate random values for the next gust

  	int diff;

  	cg_atmFx.baseEndTime = cg.time + cg_atmFx.baseMinTime + (rand() % (cg_atmFx.baseMaxTime - cg_atmFx.baseMinTime));
  	diff = cg_atmFx.changeMaxTime - cg_atmFx.changeMinTime;

  	cg_atmFx.gustStartTime = cg_atmFx.baseEndTime + cg_atmFx.changeMinTime + (diff ? (rand() % diff) : 0);
  	diff = cg_atmFx.gustMaxTime - cg_atmFx.gustMinTime;

  	cg_atmFx.gustEndTime = cg_atmFx.gustStartTime + cg_atmFx.gustMinTime + (diff ? (rand() % diff) : 0);
  	diff = cg_atmFx.changeMaxTime - cg_atmFx.changeMinTime;

  	cg_atmFx.baseStartTime = cg_atmFx.gustEndTime + cg_atmFx.changeMinTime + (diff ? (rand() % diff) : 0);
}

static qboolean CG_EffectGustCurrent( vec3_t curr, float *weight, int *num )
{
  	// Calculate direction for new drops.

  	vec3_t temp;
  	float frac;

  	if( cg.time < cg_atmFx.baseEndTime )
  	{
  	  	VectorCopy( cg_atmFx.baseVec, curr );
  	  	*weight = cg_atmFx.baseWeight;
  	  	*num = cg_atmFx.baseDrops;
  	}
  	else {
  	  	VectorSubtract( cg_atmFx.gustVec, cg_atmFx.baseVec, temp );
  	  	if( cg.time < cg_atmFx.gustStartTime )
  	  	{
  	  	  	frac = ((float)(cg.time - cg_atmFx.baseEndTime))/((float)(cg_atmFx.gustStartTime - cg_atmFx.baseEndTime));
  	  	  	VectorMA( cg_atmFx.baseVec, frac, temp, curr );
  	  	  	*weight = cg_atmFx.baseWeight + (cg_atmFx.gustWeight - cg_atmFx.baseWeight) * frac;
  	  	  	*num = cg_atmFx.baseDrops + ((float)(cg_atmFx.gustDrops - cg_atmFx.baseDrops)) * frac;
  	  	}
  	  	else if( cg.time < cg_atmFx.gustEndTime )
  	  	{
  	  	  	VectorCopy( cg_atmFx.gustVec, curr );
  	  	  	*weight = cg_atmFx.gustWeight;
  	  	  	*num = cg_atmFx.gustDrops;
  	  	}
  	  	else
  	  	{
  	  	  	frac = 1.0 - ((float)(cg.time - cg_atmFx.gustEndTime))/((float)(cg_atmFx.baseStartTime - cg_atmFx.gustEndTime));
  	  	  	VectorMA( cg_atmFx.baseVec, frac, temp, curr );
  	  	  	*weight = cg_atmFx.baseWeight + (cg_atmFx.gustWeight - cg_atmFx.baseWeight) * frac;
  	  	  	*num = cg_atmFx.baseDrops + ((float)(cg_atmFx.gustDrops - cg_atmFx.baseDrops)) * frac;
  	  	  	if( cg.time >= cg_atmFx.baseStartTime )
  	  	  	  	return( qtrue );
  	  	}
  	}
  	return( qfalse );
}

static void CG_EP_ParseFloats( char *floatstr, float *f1, float *f2 )
{
  	// Parse the float or floats

  	char *middleptr;
  	char buff[64];

  	Q_strncpyz( buff, floatstr, sizeof(buff) );
  	for( middleptr = buff; *middleptr && *middleptr != ' '; middleptr++ );
  	if( *middleptr )
  	{
  	  	*middleptr++ = 0;
  	  	*f1 = atof( floatstr );
  	  	*f2 = atof( middleptr );
  	}
  	else {
  	  	*f1 = *f2 = atof( floatstr );
  	}
}

int ATMOSPHERIC_DROPDELAY = 100;

void CG_EffectParse( const char *effectstr )
{
  	// Split the string into it's component parts.

  	float bmin, bmax, cmin, cmax, gmin, gmax, bdrop, gdrop, wsplash, lsplash;
  	int count;
  	char *startptr, *eqptr, *endptr, *type;
  	char workbuff[128];

  	if( CG_AtmosphericKludge() )
  	  	return;

  	  	// Set up some default values
  	cg_atmFx.baseVec[0] = cg_atmFx.baseVec[1] = 0;
  	cg_atmFx.gustVec[0] = cg_atmFx.gustVec[1] = 100;
  	bmin = 5;
  	bmax = 10;
  	cmin = 1;
  	cmax = 1;
  	gmin = 0;
  	gmax = 2;
  	//bdrop = gdrop = 300;
	bdrop = 200;
	gdrop = 300;
  	cg_atmFx.baseWeight = 0.7f;
  	cg_atmFx.gustWeight = 1.5f;
  	wsplash = 1;
  	lsplash = 1;
  	type = NULL;

  	  	// Parse the parameter string
  	Q_strncpyz( workbuff, effectstr, sizeof(workbuff) );
  	for( startptr = workbuff; *startptr; )
  	{
  	  	for( eqptr = startptr; *eqptr && *eqptr != '=' && *eqptr != ','; eqptr++ );
  	  	if( !*eqptr )
  	  	  	break;  	  	  	// No more string
  	  	if( *eqptr == ',' )
  	  	{
  	  	  	startptr = eqptr + 1;  	// Bad argument, continue
  	  	  	continue;
  	  	}
  	  	*eqptr++ = 0;
  	  	for( endptr = eqptr; *endptr && *endptr != ','; endptr++ );
  	  	if( *endptr )
  	  	  	*endptr++ = 0;

  	  	if( !type )
  	  	{
  	  	  	if( Q_stricmp( startptr, "T" ) ) {
  	  	  	  	cg_atmFx.numDrops = 0;
  	  	  	  	trap->Print( "Atmospheric effect must start with a type.\n" );
  	  	  	  	return;
  	  	  	}
  	  	  	if( !Q_stricmp( eqptr, "RAIN" ) ) {
  	  	  	  	type = "rain";
  	  	  	  	cg_atmFx.ParticleCheckVisible = &CG_RainParticleCheckVisible;
  	  	  	  	cg_atmFx.ParticleGenerate = &CG_RainParticleGenerate;
  	  	  	  	cg_atmFx.ParticleRender = &CG_RainParticleRender;

  	  	  	  	cg_atmFx.baseVec[2] = cg_atmFx.gustVec[2] = - ATMOSPHERIC_RAIN_SPEED;
				ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	  	} else if( !Q_stricmp( eqptr, "STORM" ) ) {
  	  	  	  	type = "storm";
  	  	  	  	cg_atmFx.ParticleCheckVisible = &CG_RainParticleCheckVisible;
  	  	  	  	cg_atmFx.ParticleGenerate = &CG_StormParticleGenerate;
  	  	  	  	cg_atmFx.ParticleRender = &CG_StormParticleRender;

  	  	  	  	cg_atmFx.baseVec[2] = cg_atmFx.gustVec[2] = - ATMOSPHERIC_RAIN_SPEED;
				ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	  	} else if( !Q_stricmp( eqptr, "SNOW" ) ) {
  	  	  	  	type = "snow";
  	  	  	  	cg_atmFx.ParticleCheckVisible = &CG_RainParticleCheckVisible;
  	  	  	  	cg_atmFx.ParticleGenerate = &CG_SnowParticleGenerate;
  	  	  	  	cg_atmFx.ParticleRender = &CG_SnowParticleRender;

  	  	  	  	cg_atmFx.baseVec[2] = cg_atmFx.gustVec[2] = - ATMOSPHERIC_SNOW_SPEED;
				ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_SNOW_DROPDELAY;
  	  	  	} else if( !Q_stricmp( eqptr, "HEAVYSNOW" ) ) {
  	  	  	  	type = "heavysnow";
  	  	  	  	cg_atmFx.ParticleCheckVisible = &CG_RainParticleCheckVisible;
  	  	  	  	cg_atmFx.ParticleGenerate = &CG_HeavySnowParticleGenerate;
  	  	  	  	cg_atmFx.ParticleRender = &CG_HeavySnowParticleRender;

  	  	  	  	cg_atmFx.baseVec[2] = cg_atmFx.gustVec[2] = - ATMOSPHERIC_SNOW_SPEED;
				ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_SNOW_DROPDELAY;
  	  	  	} else {
  	  	  	  	cg_atmFx.numDrops = 0;
  	  	  	  	trap->Print( "^1*** ^3Dark Tide^4 - ^5Only effect type '^7rain^5', '^7storm^5', '^7snow^5' and '^7heavysnow^5' are currently supported.\n" );
  	  	  	  	return;
  	  	  	}
  	  	}
  	  	else {
  	  	  	if( !Q_stricmp( startptr, "B" ) )
  	  	  	  	CG_EP_ParseFloats( eqptr, &bmin, &bmax );
  	  	  	else if( !Q_stricmp( startptr, "C" ) )
  	  	  	  	CG_EP_ParseFloats( eqptr, &cmin, &cmax );
  	  	  	else if( !Q_stricmp( startptr, "G" ) )
  	  	  	  	CG_EP_ParseFloats( eqptr, &gmin, &gmax );
  	  	  	else if( !Q_stricmp( startptr, "BV" ) )
  	  	  	  	CG_EP_ParseFloats( eqptr, &cg_atmFx.baseVec[0], &cg_atmFx.baseVec[1] );
  	  	  	else if( !Q_stricmp( startptr, "GV" ) )
  	  	  	  	CG_EP_ParseFloats( eqptr, &cg_atmFx.gustVec[0], &cg_atmFx.gustVec[1] );
  	  	  	else if( !Q_stricmp( startptr, "W" ) )
  	  	  	  	CG_EP_ParseFloats( eqptr, &cg_atmFx.baseWeight, &cg_atmFx.gustWeight );
  	  	  	else if( !Q_stricmp( startptr, "S" ) )
  	  	  	  	CG_EP_ParseFloats( eqptr, &wsplash, &lsplash );
  	  	  	else if( !Q_stricmp( startptr, "D" ) )
  	  	  	  	CG_EP_ParseFloats( eqptr, &bdrop, &gdrop );
  	  	  	else trap->Print( "Unknown effect key '%s'.\n", startptr );
  	  	}
  	  	startptr = endptr;
  	}

  	if( !type )
  	{
  	  	// No effects

  	  	cg_atmFx.numDrops = -1;
  	  	return;
  	}
  	  	
  	cg_atmFx.baseMinTime = 1000 * bmin;
  	cg_atmFx.baseMaxTime = 1000 * bmax;
  	cg_atmFx.changeMinTime = 1000 * cmin;
  	cg_atmFx.changeMaxTime = 1000 * cmax;
  	cg_atmFx.gustMinTime = 1000 * gmin;
  	cg_atmFx.gustMaxTime = 1000 * gmax;
  	cg_atmFx.baseDrops = bdrop;
  	cg_atmFx.gustDrops = gdrop;
  	cg_atmFx.waterSplash = wsplash;
  	cg_atmFx.landSplash = lsplash;

  	cg_atmFx.numDrops = (cg_atmFx.baseDrops > cg_atmFx.gustDrops) ? cg_atmFx.baseDrops : cg_atmFx.gustDrops;
  	if( cg_atmFx.numDrops > MAX_ATMOSPHERIC_PARTICLES )
  	  	cg_atmFx.numDrops = MAX_ATMOSPHERIC_PARTICLES;

  	  	// Load graphics

  	// Rain
  	if( type == "rain" ) {
  	  	cg_atmFx.numEffectShaders = 1;
  	  	if( !(cg_atmFx.effectshaders[0] = trap->R_RegisterShader( "gfx/atmosphere/raindrop" )) )
  	  	  	cg_atmFx.effectshaders[0] = -1;
  	  	if( cg_atmFx.waterSplash )
  	  	  	cg_atmFx.effectwatershader = trap->R_RegisterShader( "gfx/atmosphere/raindropwater" );
  	  	if( cg_atmFx.landSplash )
  	  	  	cg_atmFx.effectlandshader = trap->R_RegisterShader( "gfx/atmosphere/raindropsolid" );

  	// Storm
  	} else if( type == "storm" ) {
  	  	cg_atmFx.numEffectShaders = 1;
  	  	if( !(cg_atmFx.effectshaders[0] = trap->R_RegisterShader( "gfx/atmosphere/raindrop" )) )
  	  	  	cg_atmFx.effectshaders[0] = -1;
  	  	if( cg_atmFx.waterSplash )
  	  	  	cg_atmFx.effectwatershader = trap->R_RegisterShader( "gfx/atmosphere/raindropwater" );
  	  	if( cg_atmFx.landSplash )
  	  	  	cg_atmFx.effectlandshader = trap->R_RegisterShader( "gfx/atmosphere/raindropsolid" );

  	// Snow
  	} else if( type == "snow" ) {
  	  	for( cg_atmFx.numEffectShaders = 0; cg_atmFx.numEffectShaders < 6; cg_atmFx.numEffectShaders++ ) {
  	  	  	if( !( cg_atmFx.effectshaders[cg_atmFx.numEffectShaders] = trap->R_RegisterShader( va("gfx/atmosphere/snowflake0%i", cg_atmFx.numEffectShaders ) ) ) )
  	  	  	  	cg_atmFx.effectshaders[cg_atmFx.numEffectShaders] = -1;  	// we had some kind of a problem
  	  	}
  	  	cg_atmFx.waterSplash = 0;
  	  	cg_atmFx.landSplash = 0;
  	
  	} else if( type == "heavysnow" ) {
  	  	for( cg_atmFx.numEffectShaders = 0; cg_atmFx.numEffectShaders < 6; cg_atmFx.numEffectShaders++ ) {
  	  	  	if( !( cg_atmFx.effectshaders[cg_atmFx.numEffectShaders] = trap->R_RegisterShader( va("gfx/atmosphere/snowflake0%i", cg_atmFx.numEffectShaders ) ) ) )
  	  	  	  	cg_atmFx.effectshaders[cg_atmFx.numEffectShaders] = -1;  	// we had some kind of a problem
  	  	}
  	  	cg_atmFx.waterSplash = 0;
  	  	cg_atmFx.landSplash = 0;

  	// This really should never happen
	} else
  	  	cg_atmFx.numEffectShaders = 0;

  	  	// Initialise atmospheric effect to prevent all particles falling at the start
  	for( count = 0; count < cg_atmFx.numDrops; count++ )
  	  	cg_atmFx.particles[count].nextDropTime = ATMOSPHERIC_DROPDELAY + (rand() % ATMOSPHERIC_DROPDELAY);

  	CG_EffectGust();
}

/*
** Main render loop
*/

// General Stuff.
qboolean shown = qfalse; // DarkTide Init Message shown?
qboolean rain = qfalse;

// Storm Stuff.
qboolean storm = qfalse; // A storm?
int next_lightning = 0; // Next strike.
int lightning1 = -1; // Lightning1's effect ID.
int lightning2 = -1; // Lightning2's effect ID.
int lightning3 = -1; // Lightning3's effect ID.
// End of Storm Stuff.

void CG_LightningFlash( vec3_t currvec )
{
	// Attempt to 'spot' a raindrop somewhere below a sky texture.

  	float angle, distance, origz;
  	vec3_t testpoint, testend;
  	trace_t tr;
	int choice;

  	angle = random() * 2*M_PI;
  	distance = 20 + MAX_ATMOSPHERIC_DISTANCE * random();

  	testpoint[0] = testend[0] = cg.refdef.vieworg[0] + sin(angle) * distance;
  	testpoint[1] = testend[1] = cg.refdef.vieworg[1] + cos(angle) * distance;
  	testpoint[2] = origz = cg.refdef.vieworg[2];
  	testend[2] = testpoint[2] + MAX_ATMOSPHERIC_HEIGHT;

  	while( 1 )
  	{
  	  	if( testpoint[2] >= MAX_ATMOSPHERIC_HEIGHT )
  	  	  	return;
  	  	if( testend[2] >= MAX_ATMOSPHERIC_HEIGHT )
  	  	  	testend[2] = MAX_ATMOSPHERIC_HEIGHT - 1;
  	  	CG_Trace( &tr, testpoint, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
		if( tr.startsolid || tr.allsolid )  	  	  	// Stuck in something, skip over it.
  	  	{
  	  	  	testpoint[2] += 64;
  	  	  	testend[2] = testpoint[2] + MAX_ATMOSPHERIC_HEIGHT;
  	  	}
  	  	else if( tr.fraction == 1 )  	  	// Didn't hit anything, we're (probably) outside the world
		{
  	  	  	return;
		}
  	  	else if( tr.surfaceFlags & SURF_SKY )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NOIMPACT) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOIMPACT) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
		else if( (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_TRANSLUCENT) )  	// Hit sky, this is where we start.
		{
			testpoint[2] -= 128.0; // UQ1: because jka seems to use wierd skies...
  	  	  	break;
		}
  	  	else 
		{
			return;
		}
  	}

	if (lightning1 == -1)
	{// Register the effect the first time.
		lightning1 = trap->FX_RegisterEffect("effects/atmospherics/lightning_flash1.efx");
		lightning2 = trap->FX_RegisterEffect("effects/atmospherics/lightning_flash2.efx");
		lightning3 = trap->FX_RegisterEffect("effects/atmospherics/lightning_flash3.efx");
	}

	choice = rand()%3;
	if (choice == 1)
		trap->FX_PlayEffectID(lightning1, tr.endpos, currvec, 0, 0, qfalse);
	if (choice == 2)
		trap->FX_PlayEffectID(lightning2, tr.endpos, currvec, 0, 0, qfalse);
	else
		trap->FX_PlayEffectID(lightning3, tr.endpos, currvec, 0, 0, qfalse);

	trap->FX_PlayEffectID(trap->FX_RegisterEffect("effects/env/lightning_explode.efx"), tr.endpos, currvec, 0, 0, qfalse);

	//trap->S_StartSound (NULL, es->number, CHAN_ANNOUNCER, trap->S_RegisterSound(va("sound/atmospherics/thunder.wav")) );
	//trap->S_StartLocalSound(trap->S_RegisterSound(va("sound/atmospherics/thunder%i.wav"), rand()%3), CHAN_AUTO);
}

int next_rain = 0; // Rain sound effects.

void CG_AddAtmosphericEffects()
{
  	// Add atmospheric effects (e.g. rain, snow etc.) to view

  	int curr, max, currnum;
  	cg_atmosphericParticle_t *particle;
  	vec3_t currvec;
  	float currweight;

	if( CG_AtmosphericKludge() )
	{// Let's force atmosphere.
		if (!shown)
		{
			trap->Print("^1*** ^3Dark Tide^5 atmospherics is being Initialized.\n");
			trap->Print("^1*** ^3Dark Tide^5 atmospherics are ^7forced^5 for this map.\n");
			trap->Print("^1*** ^3Dark Tide^5 atmospherics Initialized OK.\n");
			shown = qtrue;
		}
	}

  	if( cg_atmFx.numDrops <= 0 || cg_atmFx.numEffectShaders == 0 )
  	  	return;

  	max = cg_lowEffects.integer ? (cg_atmFx.numDrops >> 1) : cg_atmFx.numDrops;
  	if( CG_EffectGustCurrent( currvec, &currweight, &currnum ) )
  	  	CG_EffectGust();  	  	  	// Recalculate gust parameters
  	for( curr = 0; curr < max; curr++ )
  	{
  	  	particle = &cg_atmFx.particles[curr];
  	  	if( !cg_atmFx.ParticleCheckVisible( particle ) )
  	  	{
  	  	  	// Effect has terminated / fallen from screen view

  	  	  	if( !particle->nextDropTime )
  	  	  	{
  	  	  	  	// Stop rain being synchronized 
  	  	  	  	particle->nextDropTime = rand() % ATMOSPHERIC_DROPDELAY;
  	  	  	}
  	  	  	else if( currnum < curr || particle->nextDropTime > cg.time )
  	  	  	  	continue;
  	  	  	if( !cg_atmFx.ParticleGenerate( particle, currvec, currweight ) )
  	  	  	{
  	  	  	  	// Ensure it doesn't attempt to generate every frame, to prevent
  	  	  	  	// 'clumping' when there's only a small sky area available.
  	  	  	  	particle->nextDropTime = cg.time + ATMOSPHERIC_DROPDELAY;
  	  	  	  	continue;
  	  	  	}
  	  	}

  	  	cg_atmFx.ParticleRender( particle );
  	}

	if (storm == qtrue)
	{// Some lightning explosions randomly?
		if (next_lightning <= cg.time && currvec)
		{
			if (rand()%6 < 2) // Occasionally we want it to strike twice quickly.
				next_lightning = cg.time + 100 + rand()%200;
			else
				next_lightning = cg.time + 1000 + rand()%3000;

			CG_LightningFlash( currvec );
		}
	}

	if (next_rain <= cg.time && rain == qtrue)
	{
		trap->S_StartLocalSound(trap->S_RegisterSound("sound/atmospherics/rain.wav"), CHAN_AUTO);
		next_rain = cg.time + 27000;
	}

	if (next_rain <= cg.time && storm == qtrue)
	{
		trap->S_StartLocalSound(trap->S_RegisterSound("sound/atmospherics/heavy_rain.wav"), CHAN_AUTO);
		next_rain = cg.time + 27000;
	}

  	cg_atmFx.lastRainTime = cg.time;
}


/*
**  	G_AtmosphericKludge
*/

static qboolean kludgeChecked, kludgeResult;
qboolean CG_AtmosphericKludge()
{
  	// Activate effects for specified kludge maps that don't
  	// have it specified for them.

  	if( kludgeChecked )
  	  	return( kludgeResult );
  	kludgeChecked = qtrue;
  	kludgeResult = qfalse;
	storm = qfalse;
	rain = qfalse;

	//
	// Supported Game Maps...
	//
	if( !Q_stricmp( cgs.mapname, "maps/mp/ctf2.bsp" ) )
  	{// hoth
  	  	CG_EffectParse( "T=SNOW" );
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_SNOW_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/mp/ctf3.bsp" ) )
  	{// yavin
  	  	CG_EffectParse( "T=STORM" );
		storm = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/mp/ctf4.bsp" ) )
  	{// coruscant streets
  	  	CG_EffectParse( "T=RAIN" );
		rain = qtrue;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/mp/duel6.bsp" ) )
  	{// yavin training
  	  	CG_EffectParse( "T=STORM" );
		storm = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/mp/duel9.bsp" ) )
  	{// hoth canyon
  	  	CG_EffectParse( "T=SNOW" );
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_SNOW_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/hoth2.bsp" ) )
  	{// hoth 2 sp
  	  	CG_EffectParse( "T=SNOW" );
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_SNOW_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	//
	// JK2 Maps...
	//
  	if( !Q_stricmp( cgs.mapname, "maps/ffa_bespin.bsp" ) )
  	{
  	  	CG_EffectParse( "T=STORM" );
		storm = qtrue;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/ffa_ns_streets.bsp" ) )
  	{
  	  	CG_EffectParse( "T=RAIN" );
		rain = qtrue;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/ffa_yavin.bsp" ) )
  	{
  	  	CG_EffectParse( "T=RAIN" );
		rain = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/bespin_streets.bsp" ) )
  	{
  	  	CG_EffectParse( "T=STORM" );
		storm = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/bespin_platform.bsp" ) )
  	{
  	  	CG_EffectParse( "T=STORM" );
		storm = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/artus_topside.bsp" ) )
  	{
  	  	CG_EffectParse( "T=SNOW" );
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_SNOW_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/yavin_canyon.bsp" ) )
  	{
  	  	CG_EffectParse( "T=STORM" );
		storm = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/yavin_swamp.bsp" ) )
  	{
  	  	CG_EffectParse( "T=RAIN" );
		rain = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	// Supported user maps.
	if( !Q_stricmp( cgs.mapname, "maps/ffa_coruscant.bsp" ) )
  	{
  	  	CG_EffectParse( "T=STORM" );
		storm = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/ffa_coruscant.bsp" ) )
  	{
  	  	CG_EffectParse( "T=STORM" );
		storm = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/imphoth_a.bsp" ) )
  	{
  	  	CG_EffectParse( "T=SNOW" );
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_SNOW_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/imphoth_b.bsp" ) )
  	{
  	  	CG_EffectParse( "T=SNOW" );
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_SNOW_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/jedicouncilgc2.bsp" ) )
  	{
  	  	CG_EffectParse( "T=STORM" );
		storm = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/jedicouncilgc.bsp" ) )
  	{
  	  	CG_EffectParse( "T=STORM" );
		storm = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/bespinaflstyle3.bsp" ) )
  	{
  	  	CG_EffectParse( "T=STORM" );
		storm = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/ffa_kujarforest.bsp" ) )
  	{
  	  	CG_EffectParse( "T=RAIN" );
		rain = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/wookievillage.bsp" ) )
  	{
  	  	CG_EffectParse( "T=RAIN" );
		rain = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/ewok_village.bsp" ) )
  	{
  	  	CG_EffectParse( "T=RAIN" );
		rain = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

	if( !Q_stricmp( cgs.mapname, "maps/coruscant_promenade.bsp" ) )
  	{
  	  	CG_EffectParse( "T=RAIN" );
		rain = qtrue;
		ATMOSPHERIC_DROPDELAY = ATMOSPHERIC_RAIN_DROPDELAY;
  	  	return( kludgeResult = qtrue );
  	}

  	return( kludgeResult = qfalse );
}

