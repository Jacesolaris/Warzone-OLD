cmake_minimum_required(VERSION 2.8.8)

# Make sure the user is not executing this script directly
if(NOT InWarzone)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InWarzone)

set(MPWarzoneIncludeDirectories ${MPDir})
set(MPWarzoneIncludeDirectories ${MPWarzoneIncludeDirectories} "${MPDir}/rd-warzone")

set(MPWarzoneFiles
  "${MPDir}/client/fast_mutex.h"
	"${MPDir}/client/tinythread.cpp"
	"${MPDir}/client/tinythread.h"
	"${MPDir}/qcommon/inifile.h"
	"${MPDir}/qcommon/inifile.cpp"
	"${MPDir}/rd-warzone/iqm.h"
	"${MPDir}/rd-warzone/qgl.h"
	"${MPDir}/rd-warzone/G2_API.cpp"
	"${MPDir}/rd-warzone/G2_bolts.cpp"
	"${MPDir}/rd-warzone/G2_bones.cpp"
	"${MPDir}/rd-warzone/G2_misc.cpp"
	"${MPDir}/rd-warzone/G2_surfaces.cpp"
	"${MPDir}/rd-warzone/tr_animation.cpp"
	"${MPDir}/rd-warzone/tr_awesomium.cpp"
	"${MPDir}/rd-warzone/tr_backend.cpp"
	"${MPDir}/rd-warzone/tr_bsp.cpp"
	"${MPDir}/rd-warzone/tr_cache.cpp"
	"${MPDir}/rd-warzone/tr_cmds.cpp"
	"${MPDir}/rd-warzone/tr_curve.cpp"
	"${MPDir}/rd-warzone/tr_decals.cpp"
	"${MPDir}/rd-warzone/tr_extensions.cpp"
	"${MPDir}/rd-warzone/tr_extramath.cpp"
	"${MPDir}/rd-warzone/tr_extramath.h"
	"${MPDir}/rd-warzone/tr_extratypes.h"
	"${MPDir}/rd-warzone/tr_fbo.cpp"
	"${MPDir}/rd-warzone/tr_fbo.h"
	"${MPDir}/rd-warzone/tr_flares.cpp"
	"${MPDir}/rd-warzone/tr_ghoul2.cpp"
	"${MPDir}/rd-warzone/tr_glsl.cpp"
	"${MPDir}/rd-warzone/tr_image.cpp"
	"${MPDir}/rd-warzone/tr_init.cpp"
	"${MPDir}/rd-warzone/tr_instancing.cpp"
	"${MPDir}/rd-warzone/tr_instancing.h"
	"${MPDir}/rd-warzone/tr_light.cpp"
	"${MPDir}/rd-warzone/tr_local.h"
	"${MPDir}/rd-warzone/tr_main.cpp"
	"${MPDir}/rd-warzone/tr_marks.cpp"
	"${MPDir}/rd-warzone/tr_mapping.cpp"
	"${MPDir}/rd-warzone/tr_mesh.cpp"
	"${MPDir}/rd-warzone/tr_model.cpp"
	"${MPDir}/rd-warzone/tr_model_iqm.cpp"
	"${MPDir}/rd-warzone/tr_occlusion.cpp"
	"${MPDir}/rd-warzone/tr_ocean.cpp"
	"${MPDir}/rd-warzone/tr_ocean.h"
	"${MPDir}/rd-warzone/tr_postprocess.cpp"
	"${MPDir}/rd-warzone/tr_postprocess.h"
	"${MPDir}/rd-warzone/tr_scene.cpp"
	"${MPDir}/rd-warzone/tr_shade.cpp"
	"${MPDir}/rd-warzone/tr_shade_calc.cpp"
	"${MPDir}/rd-warzone/tr_shader.cpp"
	"${MPDir}/rd-warzone/tr_shadows.cpp"
	"${MPDir}/rd-warzone/tr_skin.cpp"
	"${MPDir}/rd-warzone/tr_sky.cpp"
	"${MPDir}/rd-warzone/tr_subs.cpp"
	"${MPDir}/rd-warzone/tr_surface.cpp"
	"${MPDir}/rd-warzone/tr_vbo.cpp"
	"${MPDir}/rd-warzone/tr_world.cpp"
	"${MPDir}/rd-warzone/VectorUtils3.cpp"
	"${MPDir}/rd-warzone/VectorUtils3.h"
	"${MPDir}/game/ckheap.h"
	)
source_group("renderer" FILES ${MPWarzoneFiles})

file(GLOB MPWarzoneGLSLFiles "${MPDir}/rd-warzone/glsl/*.glsl")
source_group("renderer\\glsl" FILES ${MPWarzoneGLSLFiles})
set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneGLSLFiles})

file(GLOB MPWarzoneGLSLFiles2 "${MPDir}/rd-warzone/glsl2/*.glsl")
source_group("renderer\\glsl2" FILES ${MPWarzoneGLSLFiles2})
set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneGLSLFiles2})

set(MPWarzoneGhoul2Files
	"${MPDir}/ghoul2/g2_local.h"
	"${MPDir}/ghoul2/ghoul2_shared.h"
	"${MPDir}/ghoul2/G2_gore.cpp"
	"${MPDir}/ghoul2/G2_gore.h")
source_group("ghoul2" FILES ${MPWarzoneGhoul2Files})
set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneGhoul2Files})

set(MPWarzoneAwesomiumFiles
	"${MPDir}/rd-warzone/Awesomium/BitmapSurface.h"
	"${MPDir}/rd-warzone/Awesomium/ChildProcess.h"
	"${MPDir}/rd-warzone/Awesomium/DataPak.h"
	"${MPDir}/rd-warzone/Awesomium/DataSource.h"
	"${MPDir}/rd-warzone/Awesomium/JSArray.h"
	"${MPDir}/rd-warzone/Awesomium/JSObject.h"
	"${MPDir}/rd-warzone/Awesomium/JSValue.h"
	"${MPDir}/rd-warzone/Awesomium/Platform.h"
	"${MPDir}/rd-warzone/Awesomium/PrintConfig.h"
	"${MPDir}/rd-warzone/Awesomium/ResourceInterceptor.h"
	"${MPDir}/rd-warzone/Awesomium/STLHelpers.h"
	"${MPDir}/rd-warzone/Awesomium/Surface.h"
	"${MPDir}/rd-warzone/Awesomium/WebConfig.h"
	"${MPDir}/rd-warzone/Awesomium/WebCore.h"
	"${MPDir}/rd-warzone/Awesomium/WebKeyboardCodes.h"
	"${MPDir}/rd-warzone/Awesomium/WebKeyboardEvent.h"
	"${MPDir}/rd-warzone/Awesomium/WebMenuItem.h"
	"${MPDir}/rd-warzone/Awesomium/WebPreferences.h"
	"${MPDir}/rd-warzone/Awesomium/WebSession.h"
	"${MPDir}/rd-warzone/Awesomium/WebString.h"
	"${MPDir}/rd-warzone/Awesomium/WebStringArray.h"
	"${MPDir}/rd-warzone/Awesomium/WebTouchEvent.h"
	"${MPDir}/rd-warzone/Awesomium/WebURL.h"
	"${MPDir}/rd-warzone/Awesomium/WebView.h"
	"${MPDir}/rd-warzone/Awesomium/WebViewListener.h")
source_group("Awesomium Headers" FILES ${MPWarzoneAwesomiumFiles})
set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneAwesomiumFiles})

set(MPWarzoneMaskedOcclusionCullingFiles
	"${MPDir}/rd-warzone/MaskedOcclusionCulling/CullingThreadpool.cpp"
	"${MPDir}/rd-warzone/MaskedOcclusionCulling/CullingThreadpool.h"
	"${MPDir}/rd-warzone/MaskedOcclusionCulling/MaskedOcclusionCulling.cpp"
	"${MPDir}/rd-warzone/MaskedOcclusionCulling/MaskedOcclusionCulling.h"
	"${MPDir}/rd-warzone/MaskedOcclusionCulling/MaskedOcclusionCullingAVX2.cpp"
	"${MPDir}/rd-warzone/MaskedOcclusionCulling/MaskedOcclusionCullingCommon.inl"
	"${MPDir}/rd-warzone/MaskedOcclusionCulling/README.md")
source_group("MaskedOcclusionCulling" FILES ${MPWarzoneMaskedOcclusionCullingFiles})
set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneMaskedOcclusionCullingFiles})

set(MPWarzoneRdCommonFiles
	"${MPDir}/rd-common/mdx_format.h"
	"${MPDir}/rd-common/tr_common.h"
	"${MPDir}/rd-common/tr_font.cpp"
	"${MPDir}/rd-common/tr_font.h"
	"${MPDir}/rd-common/tr_image_load.cpp"
	"${MPDir}/rd-common/tr_image_jpg.cpp"
	"${MPDir}/rd-common/tr_image_tga.cpp"
	"${MPDir}/rd-common/tr_image_png.cpp"
	"${MPDir}/rd-common/tr_noise.cpp"
	"${MPDir}/rd-common/tr_public.h")
source_group("rd-common" FILES ${MPWarzoneRdCommonFiles})
set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneRdCommonFiles})

set(MPWarzoneQcommonFiles
  "${MPDir}/qcommon/inifile.cpp"
  "${MPDir}/qcommon/inifile.h"
	"${MPDir}/qcommon/GenericParser2.cpp"
	"${MPDir}/qcommon/matcomp.cpp"
	"${MPDir}/qcommon/q_math.cpp"
	"${MPDir}/qcommon/q_shared.cpp")
source_group("qcommon" FILES ${MPWarzoneQcommonFiles})
set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneQcommonFiles})

file(GLOB_RECURSE MPWarzoneJpegFiles "${WarzoneLibDir}/jpeg-8c/*.c" "${WarzoneLibDir}/jpeg-8c/*.h")
source_group("jpeg-8c" FILES ${MPWarzoneJpegFiles})
set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneJpegFiles})

if(UseInternalPNG)
	set(MPWarzoneLibPngFiles
		"${WarzoneLibDir}/libpng/png.c"
		"${WarzoneLibDir}/libpng/pngerror.c"
		"${WarzoneLibDir}/libpng/pngget.c"
		"${WarzoneLibDir}/libpng/pngmem.c"
		"${WarzoneLibDir}/libpng/pngpread.c"
		"${WarzoneLibDir}/libpng/pngread.c"
		"${WarzoneLibDir}/libpng/pngrio.c"
		"${WarzoneLibDir}/libpng/pngrtran.c"
		"${WarzoneLibDir}/libpng/pngrutil.c"
		"${WarzoneLibDir}/libpng/pngset.c"
		"${WarzoneLibDir}/libpng/pngtrans.c"
		"${WarzoneLibDir}/libpng/pngwio.c"
		"${WarzoneLibDir}/libpng/pngwrite.c"
		"${WarzoneLibDir}/libpng/pngwtran.c"
		"${WarzoneLibDir}/libpng/pngwutil.c")
	source_group("libpng" FILES ${MPWarzoneLibPngFiles})
	set(MPWarzoneIncludeDirectories ${MPWarzoneIncludeDirectories} "${WarzoneLibDir}/libpng")
	set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneLibPngFiles})
else(UseInternalPNG)
	find_package(PNG REQUIRED)
	set(MPWarzoneIncludeDirectories ${MPWarzoneIncludeDirectories} ${PNG_INCLUDE_DIR})
	set(MPWarzoneLibraries ${MPWarzoneLibraries} ${PNG_LIBRARIES})
endif(UseInternalPNG)

if(UseInternalZlib)
	set(MPWarzoneZLibFiles
		"${WarzoneLibDir}/zlib/adler32.c"
		"${WarzoneLibDir}/zlib/compress.c"
		"${WarzoneLibDir}/zlib/crc32.c"
		"${WarzoneLibDir}/zlib/deflate.c"
		"${WarzoneLibDir}/zlib/gzclose.c"
		"${WarzoneLibDir}/zlib/gzlib.c"
		"${WarzoneLibDir}/zlib/gzread.c"
		"${WarzoneLibDir}/zlib/gzwrite.c"
		"${WarzoneLibDir}/zlib/infback.c"
		"${WarzoneLibDir}/zlib/inffast.c"
		"${WarzoneLibDir}/zlib/inflate.c"
		"${WarzoneLibDir}/zlib/inftrees.c"
		"${WarzoneLibDir}/zlib/trees.c"
		"${WarzoneLibDir}/zlib/uncompr.c"
		"${WarzoneLibDir}/zlib/zutil.c"
		"${WarzoneLibDir}/zlib/crc32.h"
		"${WarzoneLibDir}/zlib/deflate.h"
		"${WarzoneLibDir}/zlib/gzguts.h"
		"${WarzoneLibDir}/zlib/inffast.h"
		"${WarzoneLibDir}/zlib/inffixed.h"
		"${WarzoneLibDir}/zlib/inflate.h"
		"${WarzoneLibDir}/zlib/inftrees.h"
		"${WarzoneLibDir}/zlib/trees.h"
		"${WarzoneLibDir}/zlib/zconf.h"
		"${WarzoneLibDir}/zlib/zlib.h"
		"${WarzoneLibDir}/zlib/zutil.h")
	source_group("zlib" FILES ${MPWarzoneZLibFiles})
	set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneZLibFiles})

	set(MPWarzoneIncludeDirectories ${MPWarzoneIncludeDirectories} "${WarzoneLibDir}/zlib")
else(UseInternalZlib)
	find_package(ZLIB REQUIRED)
	set(MPWarzoneIncludeDirectories ${MPWarzoneIncludeDirectories} ${ZLIB_INCLUDE_DIR})
	set(MPWarzoneLibraries ${MPWarzoneLibraries} ${ZLIB_LIBRARY})
endif(UseInternalZlib)

if(NOT WIN32)
	find_package(OpenGL REQUIRED)
	set(MPWarzoneIncludeDirectories ${MPWarzoneIncludeDirectories} ${OPENGL_INCLUDE_DIR})
	set(MPWarzoneLibraries ${MPWarzoneLibraries} ${OPENGL_LIBRARIES})

	find_package(SDL2 REQUIRED)
	set(MPWarzoneIncludeDirectories ${MPWarzoneIncludeDirectories} ${SDL2_INCLUDE_DIR})
	set(MPWarzoneLibraries ${MPWarzoneLibraries} ${SDL2_LIBRARY})
endif(NOT WIN32)


if(WIN32)
	set(MPWarzoneWin32Files
		"${MPDir}/win32/win_gamma.cpp"
		"${MPDir}/win32/win_glimp.cpp"
		"${MPDir}/win32/win_qgl.cpp")
	source_group("win32" FILES ${MPWarzoneWin32Files})
	set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneWin32Files})
else(WIN32)
	set(MPWarzoneSysFiles
		"${MPDir}/sdl/sdl_glimp.cpp")
	source_group("sys" FILES ${MPWarzoneSysFiles})
	set(MPWarzoneFiles ${MPWarzoneFiles} ${MPWarzoneSysFiles})
endif(WIN32)

source_group("renderer" FILES ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp)
set(MPWarzoneFiles ${MPWarzoneFiles} ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp)

source_group("renderer" FILES ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders2.cpp)
set(MPWarzoneFiles ${MPWarzoneFiles} ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders2.cpp)

set(MPWarzoneIncludeDirectories ${MPWarzoneIncludeDirectories} ${WarzoneLibDir})
add_library(${MPWarzone} SHARED ${MPWarzoneFiles})

if(NOT WIN32)
	# remove "lib" prefix for .so/.dylib files
	set_target_properties(${MPWarzone} PROPERTIES PREFIX "")
endif(NOT WIN32)

if(WIN32)
	install(TARGETS ${MPWarzone} RUNTIME DESTINATION ".")
else(WIN32)
	if(MakeApplicationBundles AND BuildMPEngine)
		install(TARGETS ${MPWarzone} LIBRARY DESTINATION "${MPEngine}.app/Contents/MacOS/")
	else(MakeApplicationBundles AND BuildMPEngine)
		install(TARGETS ${MPWarzone} LIBRARY DESTINATION ".")
	endif(MakeApplicationBundles AND BuildMPEngine)
endif(WIN32)
set_target_properties(${MPWarzone} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "${SharedDefines};${ReleaseDefines}")
set_target_properties(${MPWarzone} PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "${SharedDefines};${ReleaseDefines}")
set_target_properties(${MPWarzone} PROPERTIES COMPILE_DEFINITIONS_RELEASE "${SharedDefines};${ReleaseDefines}")
set_target_properties(${MPWarzone} PROPERTIES COMPILE_DEFINITIONS_DEBUG "${SharedDefines};${DebugDefines}")
set_target_properties(${MPWarzone} PROPERTIES INCLUDE_DIRECTORIES "${MPWarzoneIncludeDirectories}")
set_target_properties(${MPWarzone} PROPERTIES PROJECT_LABEL "MP Warzone Renderer")
target_link_libraries(${MPWarzone} ${MPWarzoneLibraries} "../../../lib/awesomium")

# GLSL shader file generator
add_executable(compact_glsl ${MPDir}/rd-warzone/glsl/compact.cpp)
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders2.cpp
	COMMAND compact_glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp ${MPWarzoneGLSLFiles}
	COMMAND compact_glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders2.cpp ${MPWarzoneGLSLFiles2}
	DEPENDS compact_glsl ${MPWarzoneGLSLFiles} ${MPWarzoneGLSLFiles2})
