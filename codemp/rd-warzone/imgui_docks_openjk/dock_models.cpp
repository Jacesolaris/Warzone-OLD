#include "dock_models.h"

#include "../imgui_docks/dock_console.h"
#include "../imgui_docks_openjk/dock_mdxm.h"
//#include <renderergl2/tr_model_kung.h>
#include "rd-warzone/tr_glsl.h"
#include "../tr_local.h"
#include "../compose_models.h"

DockModels::DockModels() {}

const char *DockModels::label() {
	return "Models";
}

const char *toString(surfaceType_t t) {
	switch (t) {
		case SF_BAD:			return "SF_BAD";
		case SF_SKIP:			return "SF_SKIP";
		case SF_FACE:			return "SF_FACE";
		case SF_GRID:			return "SF_GRID";
		case SF_TRIANGLES:		return "SF_TRIANGLES";
		case SF_POLY:			return "SF_POLY";
		case SF_MDV:			return "SF_MDV";
		case SF_MDR:			return "SF_MDR";
		case SF_IQM:			return "SF_IQM";
		case SF_MDX:			return "SF_MDX";
		case SF_FLARE:			return "SF_FLARE";
		case SF_ENTITY:			return "SF_ENTITY";
		case SF_DISPLAY_LIST:	return "SF_DISPLAY_LIST";
		case SF_VBO_MESH:		return "SF_VBO_MESH";
		case SF_VBO_MDVMESH:	return "SF_VBO_MDVMESH";
	}
	return "missing surfaceType_t switch";
}

const char *toString(modtype_t type) {
	switch (type) {
		case MOD_BAD   : return "MOD_BAD";
		case MOD_BRUSH : return "MOD_BRUSH";
		case MOD_MESH  : return "MOD_MESH";
		case MOD_MDR   : return "MOD_MDR";
		case MOD_IQM   : return "MOD_IQM";
		case MOD_MDXM  : return "MOD_MDXM";
		case MOD_MDXA  : return "MOD_MDXA";
	}
	return "missing modtype_t switch";
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
	if (ImGui::Button("Open")) {
		new DockMDXM(mod);
	}
}

void DockModels::imgui() {
	for (int i=0; i<tr.numModels; i++) {
		auto model = tr.models[i];
		char buf[512];
		sprintf(buf, "model[%d] name=%s type=%s", i, model->name, toString(model->type));
		if (ImGui::CollapsingHeader(buf)) {
			ImGui::PushID(model);
			switch (model->type) {
				case MOD_MDXM: imgui_mdxm(model); break;
			}
			ImGui::PopID();
		}
	}
}
