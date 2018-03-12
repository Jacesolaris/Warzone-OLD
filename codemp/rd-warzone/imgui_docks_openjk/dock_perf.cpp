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

void showCvar(char *description, cvar_t *cvar) {
	bool changed = ImGui::DragInt(description, &cvar->integer);
	if (changed)
		Cvar_SetInt(cvar, cvar->integer);
}

void DockPerf::imgui() {




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