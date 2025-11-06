cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( SHADERAPIDX8_SOURCE_FILES )
BEGIN_SRC( SHADERAPIDX8_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/imageloader.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/keyvalues.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/mempool.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"cmaterialsystemstats.cpp"
			"colorformatdx8.cpp"
			"meshdx8.cpp"
			"recording.cpp"
			"shaderapidx8.cpp"
			"shadershadowdx8.cpp"
			"texturedx8.cpp"
			"TransitionTable.cpp"
			"vertexdecl.cpp"
			"vertexshaderdx8.cpp"
		#}
	)
END_SRC( SHADERAPIDX8_SOURCE_FILES "Source Files" )

set( SHADERAPIDX8_HEADER_FILES )
BEGIN_SRC( SHADERAPIDX8_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/dx8sdk/include/d3d8.h"
			"${SRCDIR}/dx8sdk/include/d3d8caps.h"
			"${SRCDIR}/dx8sdk/include/d3d8types.h"
			"${SRCDIR}/dx8sdk/include/d3dx8.h"
			"${SRCDIR}/dx8sdk/include/d3dx8core.h"
			"${SRCDIR}/dx8sdk/include/d3dx8effect.h"
			"${SRCDIR}/dx8sdk/include/d3dx8math.h"
			"${SRCDIR}/dx8sdk/include/d3dx8mesh.h"
			"${SRCDIR}/dx8sdk/include/d3dx8shape.h"
			"${SRCDIR}/dx8sdk/include/d3dx8tex.h"

			#"${SRCDIR}/dx9sdk/include/d3d9.h"
			#"${SRCDIR}/dx9sdk/include/d3d9caps.h"
			#"${SRCDIR}/dx9sdk/include/d3d9types.h"
			#"${SRCDIR}/dx9sdk/include/d3dx9.h"
			#"${SRCDIR}/dx9sdk/include/d3dx9anim.h"
			#"${SRCDIR}/dx9sdk/include/d3dx9core.h"
			#"${SRCDIR}/dx9sdk/include/d3dx9effect.h"
			#"${SRCDIR}/dx9sdk/include/d3dx9math.h"
			#"${SRCDIR}/dx9sdk/include/d3dx9mesh.h"
			#"${SRCDIR}/dx9sdk/include/d3dx9shader.h"
			#"${SRCDIR}/dx9sdk/include/d3dx9shape.h"
			#"${SRCDIR}/dx9sdk/include/d3dx9tex.h"

			"${SRCDIR}/utils/scenemanager/workspace.h"

			"cmaterialsystemstats.h"
			"colorformatdx8.h"
			"dynamicib.h"
			"dynamicvb.h"
			"imeshdx8.h"
			"locald3dtypes.h"
			"recording.h"
			"shaderapidx8.h"
			"shaderapidx8_global.h"
			"shadershadowdx8.h"
			"stubd3ddevice.h"
			"texturedx8.h"
			"TransitionTable.h"
			"vertexdecl.h"
			"vertexshaderdx8.h"

			"${SRCDIR}/dx8sdk/include/d3dx8math.inl"
			"${SRCDIR}/dx9sdk/include/d3dx9math.inl"
		#}
	)
END_SRC( SHADERAPIDX8_HEADER_FILES "Header Files" )

add_object(
	TARGET shaderapidx8
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	INSTALL_OUTNAME "shaderapidx9"
	SOURCES ${SHADERAPIDX8_SOURCE_FILES} ${SHADERAPIDX8_HEADER_FILES}
)

target_include_directories(
	shaderapidx8 PRIVATE

	"${SRCDIR}/dx9sdk/include"

	"${SRCDIR}/materialsystem"
)

target_compile_definitions(
	shaderapidx8 PRIVATE

	SHADERAPIDX9
	IMAGE_LOADER_NO_DXTC
	SHADER_DLL_EXPORT
	strncpy=use_Q_strncpy_instead
	_snprintf=use_Q_snprintf_instead
	PROTECTED_THINGS_ENABLE
)

target_link_libraries(
	shaderapidx8 PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"

	#"${SRCDIR}/dx9sdk/lib/d3d9.lib"
	#"${SRCDIR}/dx9sdk/lib/d3dx9.lib"
	d3d9 # Windows SDK
	$<$<CONFIG:Release>:${SRCDIR}/dx9sdk/native/release/lib/x86/d3dx9.lib>
	$<$<CONFIG:Debug>:${SRCDIR}/dx9sdk/native/debug/lib/x86/d3dx9d.lib>
)

if( ${IS_WINDOWS} )

# Release
install( FILES "${SRCDIR}/dx9sdk/native/release/bin/x86/d3dx9_43.dll" DESTINATION "${GAMEDIR}/bin" )
# Debug
install( FILES "${SRCDIR}/dx9sdk/native/debug/bin/x86/d3dx9d_43.dll" DESTINATION "${GAMEDIR}/bin" )

endif()
