#include "dock_mdxm.h"
#include "../imgui_docks/dock_console.h"
#include "../imgui_openjk/gluecode.h"

#include "../imgui_openjk/imgui_openjk_default_docks.h"

DockMDXM::DockMDXM(model_t *mod_) {
	mod = mod_;
	add_dock(this);

}

#include "../tr_debug.h"

const char *DockMDXM::label() {
	return "MDXM";
}

void DockMDXM::imgui() {
	ImGui::Text("%s", mod->name);
}