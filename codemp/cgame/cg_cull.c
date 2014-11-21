#include "cg_local.h"

//
// UQ1: New distance and FOV checking on common stuff to make sure we never render stuff we don't have to...
//

extern qboolean CG_InFOV( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV );

qboolean CullVisible ( vec3_t org1, vec3_t org2, int ignore )
{
	trace_t tr;
	vec3_t orgA, orgB;

	VectorCopy(org1, orgA);
	orgA[2]+=32;
	VectorCopy(org2, orgB);
	orgB[2]+=32;

	CG_Trace( &tr, orgA, NULL, NULL, orgB, ignore, CONTENTS_OPAQUE );

	if ( tr.fraction == 1 )
	{// Completely visible!
		return ( qtrue );
	}
	else if (Distance(tr.endpos, org2) < 256)
	{// Close enough!
		return ( qtrue );
	}

	return ( qfalse );
}

qboolean ShouldCull ( vec3_t org )
{
	if (Distance(cg.refdef.vieworg, org) > 3072.0) return qtrue; // TOO FAR! CULLED!
	if (!CG_InFOV( org, cg.refdef.vieworg, cg.refdef.viewangles, cg.refdef.fov_x*1.2, cg.refdef.fov_y*1.2)) return qtrue; // NOT ON SCREEN! CULLED!
	if (!CullVisible(cg.refdef.vieworg, org, cg.clientNum)) return qtrue; // NOT VISIBLE TO US! CULLED!

	return qfalse;
}

void AddRefEntityToScene ( refEntity_t *ent )
{
	if (!ent->ignoreCull && ShouldCull(ent->origin)) return;

	trap->R_AddRefEntityToScene( ent );
}

void PlayEffectID ( int id, vec3_t org, vec3_t fwd, int vol, int rad, qboolean isPortal )
{
	if (!isPortal && ShouldCull(org)) return;

	trap->FX_PlayEffectID( id, org, fwd, vol, rad, isPortal );
}

void AddLightToScene ( const vec3_t org, float intensity, float r, float g, float b )
{
	if (ShouldCull((float *)org)) return;

	trap->R_AddLightToScene(org, intensity, r, g, b);
}
