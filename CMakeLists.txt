cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE )

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

if( UNIX )
	set( CMAKE_SKIP_BUILD_RPATH FALSE )
	set( CMAKE_BUILD_WITH_INSTALL_RPATH TRUE )
	if( APPLE )
		set( CMAKE_MACOSX_RPATH ON )
		set( CMAKE_INSTALL_RPATH "@loader_path" )
	else()
		set( CMAKE_INSTALL_RPATH "$\{ORIGIN\}" )
	endif()
endif()

project( LeakNet CXX C )

# For some reason, checking if CMAKE_BUILD_TYPE is defined is unreliable
# So simply check if it's empty instead
if( "${CMAKE_BUILD_TYPE}" STREQUAL "" )
	set( CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE )
endif()

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_VISIBILITY_PRESET default ) #hidden # @TODO(SanyaSho): Linux support
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

set( BUILD_GROUP "game" CACHE STRING "Build Group" )
set_property( CACHE BUILD_GROUP PROPERTY STRINGS "game" "dedicated" "everything" ) # should be same as in //cmake/groups.cmake


set( SRCDIR "${CMAKE_CURRENT_LIST_DIR}" )
# Unified beta build drop location (matches 2003 engine layout)
get_filename_component( BETA_BUILD_DIR "${CMAKE_CURRENT_LIST_DIR}/../beta_build" ABSOLUTE )
set( GAMEDIR "${BETA_BUILD_DIR}" CACHE STRING "Build output folder" FORCE )

set( LIBPUBLIC "${SRCDIR}/lib/public" )
set( LIBCOMMON "${SRCDIR}/lib/common" )
link_directories( ${LIBPUBLIC} ${LIBCOMMON} )

include( "cmake/base.cmake" )
include( "cmake/platform_dirs.cmake" )
include( "cmake/video_base.cmake" )
include( "cmake/postbuild.cmake" )
include( "cmake/misc/install_lib.cmake" )

# DEBUG / NDEBUG selection.
#   * _DEBUG (debug CRT + symbols) is applied to EVERY target in the Debug config so the CRT ABI
#     stays consistent across all DLLs, including the vphysics<->engine boundary.
#   * IVP/Havok physics middleware ships as NDEBUG in retail Source. Its ivu_types.hxx requires
#     EXACTLY ONE of DEBUG/NDEBUG and, under DEBUG, enables internal IVP_ASSERT/IVP_IF self-
#     consistency checks that int3 (software breakpoint) on edge-case-but-nonfatal physics states
#     the retail engine silently tolerates (degenerate contacts, e.g. on a map whose world
#     collision failed to load). Targets that opt in via the IVP_NDEBUG target property therefore
#     get NDEBUG instead of DEBUG so those checks compile out, matching retail physics. _DEBUG is
#     intentionally kept for them so they remain a debug build (symbols, debug CRT). The property
#     is set on the physics targets just below (search: IVP_NDEBUG).
add_compile_definitions(
	$<$<AND:$<CONFIG:Debug>,$<NOT:$<BOOL:$<TARGET_PROPERTY:IVP_NDEBUG>>>>:DEBUG>
	$<$<CONFIG:Debug>:_DEBUG>
)
add_compile_definitions(
	$<$<OR:$<CONFIG:Release>,$<AND:$<CONFIG:Debug>,$<BOOL:$<TARGET_PROPERTY:IVP_NDEBUG>>>>:NDEBUG>
)

if( ${IS_WINDOWS} )
	include( "cmake/windows_base.cmake" )
elseif( ${IS_LINUX} OR ${IS_OSX} )
	include( "cmake/posix_base.cmake" )
endif()

include( "cmake/misc/srcgrp.cmake" )

include( "cmake/groups.cmake" )


# Build the physics middleware DLL (vphysics) and its IVP/Havok static libs as NDEBUG so IVP's
# internal self-consistency asserts compile out (see the DEBUG/NDEBUG comment above). All of these
# libs link only into vphysics.dll, so the whole DLL is NDEBUG-consistent (no intra-DLL DEBUG vs
# NDEBUG layout/ODR mismatch). Guarded with if(TARGET) so build groups that omit a physics target
# do not error.
foreach( _phys_tgt vphysics ivp_physics ivp_compact_builder hk_math hk_base havana_constraints )
	if( TARGET ${_phys_tgt} )
		set_target_properties( ${_phys_tgt} PROPERTIES IVP_NDEBUG TRUE )
	endif()
endforeach()


# Store all targets in a variable name ( See: https://stackoverflow.com/questions/37434946/how-do-i-iterate-over-all-cmake-targets-programmatically/62311397#62311397 )
macro( get_all_targets_recursive targets dir )
    get_property( subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES )

    foreach( subdir ${subdirectories} )
        get_all_targets_recursive( ${targets} ${subdir} )
    endforeach()

    get_property( current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS )
    list( APPEND ${targets} ${current_targets} )
endmacro()

function( get_all_targets var )
    set( targets )
    get_all_targets_recursive( targets ${CMAKE_CURRENT_SOURCE_DIR} )
    set( ${var} ${targets} PARENT_SCOPE )
endfunction()

get_all_targets( ALL_TARGETS )

message( "--- Targets for \"${BUILD_GROUP}\":" )

# Iterates over all the targets and add memoverride stuff for debugging
foreach( target ${ALL_TARGETS} )
	get_target_property( target_type ${target} TYPE )
	message( "---- ${target} (${target_type})" )
endforeach()

# Populate the BMod game directory with the original GMod 9.0.4b data that
# the client DLL consumes at run time.  Previously only gameevents.res was
# deployed, leaving the spawn-menu scheme/font files, spawn icons, and GMod
# materials outside the game search path (which produces invisible text and
# the engine's green missing-texture fallback).
set( BMOD_ASSET_SOURCE "${SRCDIR}/gmod_9_0_4b" )
set( BMOD_ASSET_DESTINATION "${GAMEDIR}/bmod" )
foreach( BMOD_ASSET_DIR cfg fonts lua materials models mods resource scripts settings sound )
	if( EXISTS "${BMOD_ASSET_SOURCE}/${BMOD_ASSET_DIR}" )
		file( COPY "${BMOD_ASSET_SOURCE}/${BMOD_ASSET_DIR}" DESTINATION "${BMOD_ASSET_DESTINATION}" )
	endif()
endforeach()

# The engine executes cfg/config.cfg at startup; config_default.cfg is only a
# template and is never loaded by itself.  Seed the real config once so the
# shipped GMod controls (including Q -> +menu) are active, while preserving a
# player's customized bindings on subsequent CMake runs.
if( EXISTS "${BMOD_ASSET_SOURCE}/cfg/config_default.cfg" AND
	NOT EXISTS "${BMOD_ASSET_DESTINATION}/cfg/config.cfg" )
	configure_file(
		"${BMOD_ASSET_SOURCE}/cfg/config_default.cfg"
		"${BMOD_ASSET_DESTINATION}/cfg/config.cfg"
		COPYONLY
	)
endif()

# Engine-era player remapping can request this material even though it was not
# shipped with the extracted GMod 9 content.  Ship a neutral fallback instead
# of allowing the material system's bright-green error texture.
file( COPY "${SRCDIR}/bmod_assets/materials" DESTINATION "${BMOD_ASSET_DESTINATION}" )

# Keep the project-local resource override available for both the engine and
# BMod, while preserving its gameevents addition.
file( MAKE_DIRECTORY "${GAMEDIR}/resource" )
file( COPY "${SRCDIR}/resource/gameevents.res" DESTINATION "${GAMEDIR}/resource" )
file( COPY "${SRCDIR}/resource/SpawnMenuScheme.res" DESTINATION "${BMOD_ASSET_DESTINATION}/resource" )

# GMod's build/spawn dialog requests this exact resource path.  The extracted
# 9.0.4b data references it from client.dll but does not contain the file, so
# deploy the project-local BMod layout into the active mod instead of allowing
# a mounted game's Resource/UI file (or no file at all) to satisfy the lookup.
file( MAKE_DIRECTORY "${BMOD_ASSET_DESTINATION}/resource/ui" )
file( COPY "${SRCDIR}/resource/ui/menu_toolbuttons.res"
	DESTINATION "${BMOD_ASSET_DESTINATION}/resource/ui" )

# GameUI only resolves its Create Server pages and localization from the active
# mod's resource path.  Keep these base UI assets with BMod instead of relying
# on the mounted HL2 search path, which can be absent after a beta reinstall.
foreach( BMOD_GAMEUI_RESOURCE
	CreateMultiplayerGameServerPage.res
	CreateMultiplayerGameGameplayPage.res
	gameui_english.txt
	TrackerScheme.res
)
	file( COPY "${SRCDIR}/resource/${BMOD_GAMEUI_RESOURCE}"
		DESTINATION "${BMOD_ASSET_DESTINATION}/resource" )
	# Legacy GameUI resolves these files from the engine-root Resource path,
	# while BMod HUD code resolves them through the active mod path.  Deploy to
	# both locations so a beta reinstall cannot leave either UI path blank.
	file( COPY "${SRCDIR}/resource/${BMOD_GAMEUI_RESOURCE}"
		DESTINATION "${GAMEDIR}/resource" )
endforeach()
