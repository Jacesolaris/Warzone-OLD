#include "dock_models.h"

#include "../imgui_docks/dock_console.h"
//#include <renderergl2/tr_model_kung.h>
#include "rd-warzone/tr_glsl.h"
#include "../tr_local.h"

DockModels::DockModels() {}

const char *DockModels::label() {
	return "Models";
}

char *modeltype2string(modtype_t type) {
	switch (type) {
		case MOD_BAD   : return "MOD_BAD   "; break; 
		case MOD_BRUSH : return "MOD_BRUSH "; break; 
		case MOD_MESH  : return "MOD_MESH  "; break; 
		case MOD_MDR   : return "MOD_MDR   "; break; 
		case MOD_IQM   : return "MOD_IQM   "; break; 
		case MOD_MDXM  : return "MOD_MDXM"  ; break; 
		case MOD_MDXA  : return "MOD_MDXA"  ; break;
	}
	return "Unknown modtype_t";
}

void imgui_mdxm(model_t *mod) {
	mdxmData_t *glm = mod->data.glm;
	mdxmHeader_t *header = glm->header;
	ImGui::Text("ident=%d", header->ident);
	ImGui::Text("version=%d", header->version);
	ImGui::Text("name=%s", header->name);
	ImGui::Text("animName=%s", header->animName);
	ImGui::Text("animIndex=%d", header->animIndex);
	ImGui::Text("numBones=%d", header->numBones);
	ImGui::Text("numLODs=%d", header->numLODs);
	ImGui::Text("ofsLODs=%d", header->ofsLODs);
	ImGui::Text("numSurfaces=%d", header->numSurfaces);
	ImGui::Text("ofsSurfHierarchy=%d", header->ofsSurfHierarchy);
	ImGui::Text("ofsEnd=%d", header->ofsEnd);

	mdxmSurfHierarchy_t *surfInfo = (mdxmSurfHierarchy_t *)( (byte *)header + header->ofsSurfHierarchy);
	mdxmSurfHierarchy_t *iterator = surfInfo;
 	for (int i=0 ; i<header->numSurfaces; i++) {
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
		snprintf(tmp, sizeof(tmp), "name=%s flags=%d shader=%s shaderIndex=%d parentIndex=%d numChildren=%d childIndexes[0]=%d",
			iterator->name,
			iterator->flags,
			iterator->shader,
			iterator->shaderIndex,
			iterator->parentIndex,
			iterator->numChildren,
			iterator->childIndexes[0]
		);
		if (ImGui::CollapsingHeader(tmp)) {

		}

		break; // fix getting next surface pointer, seen somewhere example code, too tired now lol
		iterator++; 
	}
}

void DockModels::imgui() {
	for (int i=0; i<tr.numModels; i++) {
		auto model = tr.models[i];
		
		char buf[512];
		sprintf(buf, "model[%d] name=%s type=%s", i, model->name, modeltype2string(model->type));
		if (ImGui::CollapsingHeader(buf)) {
			
			switch (model->type) {
				case MOD_MDXM: imgui_mdxm(model); break;
			}
		}



	}
		
}