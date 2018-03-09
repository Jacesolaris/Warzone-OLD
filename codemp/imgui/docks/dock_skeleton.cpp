#include "dock_skeleton.h"
#include "../include_imgui.h"

DockSkeleton::DockSkeleton() {
}

const char *DockSkeleton::label() {
	return "Skeleton";
}

void DockSkeleton::imgui() {
	ImGui::Button("DockSkeleton");
}