#include "tr_local.h"

inline mdxmVertex_t *firstVertex(mdxmSurface_t *surf) {
	return (mdxmVertex_t *) ((byte *)surf + surf->ofsVerts);
}

inline mdxmSurface_t *firstSurface(mdxmHeader_t *header, mdxmLOD_t *lod) {
	return (mdxmSurface_t *) ( (byte *)lod + sizeof (mdxmLOD_t) + (header->numSurfaces * sizeof(mdxmLODSurfOffset_t)) );
}

inline mdxmLOD_t *firstLod(mdxmHeader_t *header) {
	return (mdxmLOD_t *) ( (byte *)header + header->ofsLODs );
}

inline mdxmSurfHierarchy_t *firstSurfHierarchy(mdxmHeader_t *header) {
	return (mdxmSurfHierarchy_t *)( (byte *)header + header->ofsSurfHierarchy );
}

inline mdxmLOD_t *next(mdxmLOD_t *lod) {
	return (mdxmLOD_t *)( (byte *)lod + lod->ofsEnd );
}

inline mdxmSurface_t *next(mdxmSurface_t *surf) {
	return (mdxmSurface_t *)( (byte *)surf + surf->ofsEnd );
}

inline mdxmSurfHierarchy_t *next(mdxmSurfHierarchy_t *surfHierarchy) {
	return (mdxmSurfHierarchy_t *)( (byte *)surfHierarchy + (intptr_t)( &((mdxmSurfHierarchy_t *)0)->childIndexes[ surfHierarchy->numChildren ] ));
}

