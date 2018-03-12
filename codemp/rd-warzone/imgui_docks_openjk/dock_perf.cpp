#include "dock_perf.h"
#include "../imgui_docks/dock_console.h"
#include <map>

DockPerf::DockPerf() {}

#include "../tr_debug.h"

const char *DockPerf::label() {
	return "Perf";
}

void Cvar_SetInt(cvar_t *cvar, int value) {
	ri->Cvar_Set(cvar->name, va("%i", value));
}

namespace ImGui {
	// roses are red, qboolean is no bool
	extern bool Checkbox(char *label, qboolean *var);
}

namespace ImGui {
	bool CvarBool(cvar_t *cvar) {
		bool changed = false;
		if (cvar->displayInfoSet && cvar->displayName && cvar->displayName[0]) {
			changed = ImGui::Checkbox(cvar->displayName, (qboolean *)&cvar->integer);
			if (cvar->displayInfoSet && cvar->description && cvar->description[0]) {
				ImGui::SetTooltip(cvar->description);
			}
		} else {
			changed = ImGui::Checkbox(cvar->name, (qboolean *)&cvar->integer);
			if (cvar->displayInfoSet && cvar->description && cvar->description[0]) {
				ImGui::SetTooltip(cvar->description);
			}
		}
		if (changed)
			Cvar_SetInt(cvar, cvar->integer);
		return changed;
	}
	bool CvarInt(cvar_t *cvar) {
		bool changed = false;
		if (cvar->displayInfoSet && cvar->displayName && cvar->displayName[0]) {
			
			changed = ImGui::DragInt(cvar->name, &cvar->integer, cvar->dragspeed, cvar->min, cvar->max);
			if (cvar->displayInfoSet && cvar->description && cvar->description[0]) {
				ImGui::SetTooltip(cvar->description);
			}
		} else {
			changed = ImGui::DragInt(cvar->name, &cvar->integer, cvar->dragspeed, cvar->min, cvar->max);
			if (cvar->displayInfoSet && cvar->description && cvar->description[0]) {
				ImGui::SetTooltip(cvar->description);
			}
		}
		if (changed)
			Cvar_SetInt(cvar, cvar->integer);
		return changed;
	}

	bool Cvar(cvar_t *cvar) {
		bool changed = false;
		switch (cvar->typeinfo) {
			case CvarType_Bool:
				changed = ImGui::CvarBool(cvar);
				break;

			case CvarType_Int:
				changed = ImGui::CvarInt(cvar);
				break;

			default:
				ImGui::Text("Don't know how to show cvar->typeinfo=%d, please implement it for ImGui::Cvar()", cvar->typeinfo);
		}
		return changed;
	}
}

void showCvar(char *info, cvar_t *cvar) {
	ImGui::Text(info);
	//ImGui::SameLine();
	ImGui::Cvar(cvar);
}

void DockPerf::imgui() {

	//r_shadowBlur->typeinfo = CvarType_Bool;
	//r_dynamicGlow->typeinfo = CvarType_Bool;
	////r_bloom, 2);
	//r_anamorphic->typeinfo = CvarType_Bool;
	//r_ssdm->typeinfo = CvarType_Bool;
	////r_ao, 3);
	////r_cartoon, 3);
	//r_ssdo->typeinfo = CvarType_Bool;
	//r_sss->typeinfo = CvarType_Bool;
	//r_deferredLighting->typeinfo = CvarType_Bool;
	//r_ssr->typeinfo = CvarType_Bool;
	//r_sse->typeinfo = CvarType_Bool;
	//r_magicdetail->typeinfo = CvarType_Bool;
	//r_hbao->typeinfo = CvarType_Bool;
	////r_glslWater, 3);
	//r_fogPost->typeinfo = CvarType_Bool;
	//r_multipost->typeinfo = CvarType_Bool;
	////r_dof, 3);
	//r_lensflare->typeinfo = CvarType_Bool;
	//r_testshader->typeinfo = CvarType_Bool;
	//r_colorCorrection->typeinfo = CvarType_Bool;
	//r_esharpening->typeinfo = CvarType_Bool;
	//r_esharpening2->typeinfo = CvarType_Bool;
	//r_darkexpand->typeinfo = CvarType_Bool;
	////r_distanceBlur, 5);
	//r_volumeLight->typeinfo = CvarType_Bool;
	//r_fxaa->typeinfo = CvarType_Bool;
	//r_showdepth->typeinfo = CvarType_Bool;
	////r_shownormals, 2);
	////r_trueAnaglyph, 2);
	//r_occlusion->typeinfo = CvarType_Bool;

	if (ImGui::CollapsingHeader("void RB_PostProcess() { ... }")) {
		ImGui::PushItemWidth(100);
		showCvar("if (ENABLE_DISPLACEMENT_MAPPING) RB_SSDM_Generate()"         , r_ssdm            );
		showCvar("if (r_ao >= 2) RB_SSAO()"                                    , r_ao              );
		showCvar("if (r_cartoon >= 2) RB_CellShade()"                          , r_cartoon         );
		showCvar("if (r_cartoon >= 3) RB_Paint()"                              , r_cartoon         );
		showCvar("if (AO_DIRECTIONAL) RB_SSDO()"                               , r_ssdo            );
		showCvar("RB_SSS()"                                                    , r_sss             );
		showCvar("if (r_bloom >= 2 || r_anamorphic) RB_CreateAnamorphicImage()", r_anamorphic      );
		showCvar("if (!LATE_LIGHTING_ENABLED) RB_DeferredLighting()"           , r_deferredLighting);
		showCvar("if (r_ssr > 0 || r_sse > 0) RB_ScreenSpaceReflections()"     , r_ssr             );
		//showCvar("RB_Underwater",r_underwater); // commented out in RB_PostProcess too
		showCvar("RB_MagicDetail"                                              , r_magicdetail     );
		showCvar("Screen RB_GaussianBlur()"                                    , r_screenBlurSlow  );
		showCvar("Screen RB_FastBlur()"                                        , r_screenBlurFast  );
		showCvar("RB_HBAO"                                                     , r_hbao            );
		showCvar("RB_SSDM (ENABLE_DISPLACEMENT_MAPPING)"                       , r_ssdm            );
		showCvar("RB_WaterPost (r_glslWater->integer <= 2 && WATER_ENABLED)"   , r_glslWater       );
		showCvar("RB_FogPostShader (FOG_POST_ENABLED && LATE_LIGHTING_ENABLED)", r_fogPost         );
		showCvar("RB_MultiPost"                                                , r_multipost       );
		showCvar("RB_DOF"                                                      , r_dof             );
		showCvar("RB_LensFlare"                                                , r_lensflare       );
		showCvar("RB_TestShader"                                               , r_testshader      );
		showCvar("RB_ColorCorrection"                                          , r_colorCorrection );
		showCvar("RB_DeferredLighting (LATE_LIGHTING_ENABLED == 1)"            , r_deferredLighting);
		showCvar("RB_FogPostShader (FOG_POST_ENABLED && LATE_LIGHTING_ENABLED)", r_fogPost         );
		showCvar("RB_DeferredLighting (LATE_LIGHTING_ENABLED >= 2)"            , r_deferredLighting);
		showCvar("RB_ESharpening"                                              , r_esharpening     );
		showCvar("RB_ESharpening2"                                             , r_esharpening2    );
		showCvar("RB_DarkExpand"                                               , r_darkexpand      );
		showCvar("RB_DistanceBlur"                                             , r_distanceBlur    );
		showCvar("Bloom"                                                       , r_bloom           );
		showCvar("Anamorphic"                                                  , r_anamorphic      );
		showCvar("RB_VolumetricLight (r_dynamiclight != 0)"                    , r_volumeLight     );
		showCvar("Bloom Rays"                                                  , r_bloom           );
		showCvar("FXAA"                                                        , r_fxaa            );
		showCvar("Show Depth"                                                  , r_showdepth       );
		showCvar("Show Normals"                                                , r_shownormals     );
		showCvar("Anaglyph"                                                    , r_trueAnaglyph    );
		ImGui::PopItemWidth();
	}

	for (auto perfdata : performancelog) {
		const char *name = perfdata.first.c_str();
		int starttime = perfdata.second.starttime;
		int stoptime = perfdata.second.stoptime;
		int delta = stoptime - starttime;
		float milliseconds = (float)delta;
		const float maxTime = 1000.0 / 60.0; // 16.66ms is the absolute maximum, if a frame takes like 10ms its already in the very evil range...
		float maxPercent = (milliseconds / maxTime) * 100.0;
		ImGui::Text("%s: start=%10d stop=%10d milliseconds=%10.2f maxPercent=%10.2f", name, starttime, stoptime, milliseconds, maxPercent);
	}




}