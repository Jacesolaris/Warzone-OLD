#include "dock_mdxm.h"
#include "../imgui_docks/dock_console.h"
#include "../imgui_openjk/gluecode.h"

#include "../imgui_openjk/imgui_openjk_default_docks.h"
#include "../compose_models.h"

qboolean model_upload_mdxm_to_gpu(model_t *mod);



void DockMDXM::imgui_mdxm_list_surfhierarchy(mdxmHeader_t *header) {
	mdxmSurfHierarchy_t *surfHierarchy = firstSurfHierarchy(header);
 	for (int surface_id=0 ; surface_id<header->numSurfaces; surface_id++) {
		/*
			char		name[MAX_QPATH];
			unsigned int flags;
			char		shader[MAX_QPATH];
			int			shaderIndex;		// for in-game use (carcass defaults to 0)
			int			parentIndex;		// this points to the index in the file of the parent surface. -1 if null/root
			int			numChildren;		// number of surfaces which are children of this one
			int			childIndexes[1];	// [mdxmSurfHierarch_t->numChildren] (variable sized)		
		*/
		char tmp[512];
		snprintf(tmp, sizeof(tmp), "mdxmSurfHierarchy[%d] name=%s flags=%d shader=%s shaderIndex=%d parentIndex=%d numChildren=%d childIndexes[0]=%d",
			surface_id,
			surfHierarchy->name,
			surfHierarchy->flags,
			surfHierarchy->shader,
			surfHierarchy->shaderIndex,
			surfHierarchy->parentIndex,
			surfHierarchy->numChildren,
			surfHierarchy->childIndexes[0]
		);
		if (ImGui::CollapsingHeader(tmp)) {
		}
		
		surfHierarchy = next(surfHierarchy);
	}
}

void DockMDXM::imgui_mdxm_surface_vertices(mdxmHeader_t *header, mdxmSurface_t *surf) {
	mdxmVertex_t *vert = firstVertex(surf);

	//ImGui::Text("verts=%p type=%s numVerts=%d numTriangles=%d numBoneReferences=%d", 
	//	vert->,
	//	surfacetypeToString((surfaceType_t)surf->ident),
	//	surf->numVerts,
	//	surf->numTriangles,
	//	surf->numBoneReferences
	//);

	for (int vert_id=0; vert_id<surf->numVerts; vert_id++) {
		char dragString[128];

		snprintf(dragString, sizeof(dragString), "verts[%i]", vert_id);
		ImGui::DragFloat3(dragString, vert->vertCoords);
		vert++;
	}



}

void DockMDXM::imgui_mdxm_surface(mdxmHeader_t *header, mdxmSurface_t *surf) {
	ImGui::PushID(surf);

	char strBoneReferences[128];
	snprintf(strBoneReferences, sizeof(strBoneReferences), "%d bone references", surf->numBoneReferences);
	if (ImGui::CollapsingHeader(strBoneReferences)) {
		
		int *boneRef = (int *) ( (byte *)surf + surf->ofsBoneReferences);
		for (int j=0 ; j<surf->numBoneReferences; j++) {
			char tmpName[128];
			snprintf(tmpName, sizeof(tmpName), "boneReference[%d]", j);
			ImGui::DragInt(tmpName, boneRef + j);
			if (boneRef[j] < 0)
				boneRef[j] = 0;
			if (boneRef[j] >= header->numBones)
				boneRef[j] = header->numBones - 1;
		}
	}
	
	if (ImGui::Button("verts *= 2")) {
		
		mdxmVertex_t *vert = firstVertex(surf);
		for (int vertex_id=0; vertex_id<surf->numVerts; vertex_id++) {
			vert->vertCoords[0] *= 2.0;
			vert->vertCoords[1] *= 2.0;
			vert->vertCoords[2] *= 2.0;
			vert++;
		}

		model_upload_mdxm_to_gpu(mod);
	}
	if (ImGui::Button("verts /= 2")) {
		mdxmVertex_t *vert = firstVertex(surf);
		for (int vertex_id=0; vertex_id<surf->numVerts; vertex_id++) {
			vert->vertCoords[0] /= 2.0;
			vert->vertCoords[1] /= 2.0;
			vert->vertCoords[2] /= 2.0;
			vert++;
		}
		model_upload_mdxm_to_gpu(mod);
	
	}

	char strVerts[128];
	snprintf(strVerts, sizeof(strVerts), "%d vertices", surf->numVerts);
	if (ImGui::CollapsingHeader(strVerts)) {
		imgui_mdxm_surface_vertices(header, surf);
	}

	ImGui::PopID();
}

const char *toString(surfaceType_t t);
const char *toString(modtype_t type);

void DockMDXM::imgui_mdxm_list_lods(mdxmHeader_t *header) {
	mdxmLOD_t *lod = firstLod(header);
	for (int lod_id=0; lod_id<header->numLODs; lod_id++) {
		char tmp[512];
		snprintf(tmp, sizeof(tmp), "mdxmLOD_t[%d] ofsEnd=%d", lod_id, lod->ofsEnd );
		if (ImGui::CollapsingHeader(tmp)) {
			mdxmSurface_t *surf = firstSurface(header, lod);
			for (int i=0; i<header->numSurfaces; i++) {


				char tmp[512];
				snprintf(tmp, sizeof(tmp), "surf id=%d ptr=%p type=%s numVerts=%d numTriangles=%d numBoneReferences=%d", 
					i,
					surf,
					toString((surfaceType_t)surf->ident),
					surf->numVerts,
					surf->numTriangles,
					surf->numBoneReferences
				);
				if (ImGui::CollapsingHeader(tmp)) {
					imgui_mdxm_surface(header, surf);
				}
				surf = next(surf);
			}
		}
		lod = next(lod);
	}
}



DockMDXM::DockMDXM(model_t *mod_) {
	mod = mod_;
	add_dock(this);
	header = mdxmHeader(mod_);
	auto surfHierarchy = firstSurfHierarchy(header);
	for (int i=0; i<header->numSurfaces; i++) {
		strcpy(lookupSurfNames[i], surfHierarchy->name);
		surfHierarchy = next(surfHierarchy);
	}
}

#include "../tr_debug.h"

const char *DockMDXM::label() {
	return "MDXM";
}

void DockMDXM::imgui() {
	ImGui::Text("%s", mod->name);

	if (ImGui::CollapsingHeader("mdxmSurfHierarchy_t")) {
		imgui_mdxm_list_surfhierarchy(header);
	}
	if (ImGui::CollapsingHeader("lods")) {
		imgui_mdxm_list_lods(header);
	}

}