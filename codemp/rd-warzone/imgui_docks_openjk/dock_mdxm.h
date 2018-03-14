#ifndef DOCK_MDXM
#define DOCK_MDXM

#include "../imgui_docks/dock.h"
#include <string>
#include "../imgui/imgui_dock.h"
#include "../tr_local.h"

class DockMDXM : public Dock {
public:
	model_t *mod;
	DockMDXM(model_t *mod_);
	virtual const char *label();
	virtual void imgui();
};

#endif