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
