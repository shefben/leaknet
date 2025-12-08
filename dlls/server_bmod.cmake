include_guard( GLOBAL )

include( "${CMAKE_CURRENT_LIST_DIR}/server_base.cmake" )

set( SERVER_BMOD_SOURCE_FILES )
BEGIN_SRC( SERVER_BMOD_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SUBGROUP "HL2 Base"
		SOURCES
		#{
			"basebludgeonweapon.cpp" # Base for HL2-style weapons
			"basebludgeonweapon.h"

			# Core HL2 files needed for BarrysMod
			"hl2_dll/basehlcombatweapon.cpp"
			"hl2_dll/basehlcombatweapon.h"
			"${SRCDIR}/game_shared/hl2/basehlcombatweapon_shared.cpp"
			"${SRCDIR}/game_shared/hl2/basehlcombatweapon_shared.h"

			"hl2_dll/hl2_player.cpp"
			"hl2_dll/hl2_player.h"
			"hl2_dll/hl2_playerlocaldata.cpp"
			"hl2_dll/hl2_playerlocaldata.h"
			"${SRCDIR}/game_shared/hl2/hl2_gamerules.cpp"
			"${SRCDIR}/game_shared/hl2/hl2_gamerules.h"
			# "${SRCDIR}/game_shared/bmod/bmod_usermessages.cpp" # Commented out due to multiple definition conflicts
			"hl2_dll/hl2_client.cpp"

			# Physics cannon needed for GMod tools
			"hl2_dll/weapon_physcannon.cpp"
			"hl2_dll/weapon_physcannon.h"

			# Basic HL2 weapons for compatibility
			"hl2_dll/weapon_crowbar.cpp"
			"hl2_dll/weapon_pistol.cpp"
			"hl2_dll/weapon_smg1.cpp"
			"hl2_dll/weapon_ar2.cpp"
			"hl2_dll/weapon_ar2.h"
			"hl2_dll/weapon_shotgun.cpp"

			# Additional HL2 weapons
			"bmod_dll/weapon_frag.cpp"
			"bmod_dll/weapon_gauss.cpp"
			"bmod_dll/weapon_rpg.cpp"

			# Grenades needed by weapons
			"bmod_dll/grenade_ar2.cpp"
			"bmod_dll/grenade_frag.cpp"
			"bmod_dll/ar2_explosion.cpp"

			# Map entities
			"bmod_dll/WaterLODControl.cpp"

			"bmod_dll/hl_playermove.cpp"
			"${SRCDIR}/game_shared/hl_gamemovement.cpp"
			"bmod_dll/hl2_eventlog.cpp"
			"bmod_dll/Point_Camera.cpp"
			"bmod_dll/Point_Camera.h"
			"bmod_dll/Func_Monitor.cpp"
			"bmod_dll/Func_Monitor.h"
		#}
	)
	SRC_GRP(
		SUBGROUP "BarrysMod Core"
		SOURCES
		#{
			"bmod_dll/gmod_system.cpp"
			"bmod_dll/gmod_system.h"
			"bmod_dll/gmod_weld.cpp"
			"bmod_dll/gmod_weld.h"
			"bmod_dll/gmod_undo.cpp"
			"bmod_dll/gmod_undo.h"
			"bmod_dll/gmod_lua.cpp"
			"bmod_dll/gmod_lua.h"
			"bmod_dll/gmod_tools.cpp"
			"bmod_dll/gmod_tools.h"
			"bmod_dll/gmod_swep.cpp"
			"bmod_dll/gmod_swep.h"
			"bmod_dll/gmod_gamemode.cpp"
			"bmod_dll/gmod_gamemode.h"
			"bmod_dll/gmod_mod.cpp"
			"bmod_dll/gmod_mod.h"
			"bmod_dll/gmod_overlay.cpp"
			"bmod_dll/gmod_overlay.h"
			"bmod_dll/gmod_expressions.cpp"
			"bmod_dll/gmod_expressions.h"
			"bmod_dll/gmod_death.cpp"
			"bmod_dll/gmod_death.h"
			"bmod_dll/gmod_scheme.cpp"
			"bmod_dll/gmod_scheme.h"
			"bmod_dll/gmod_compatibility_test.cpp"
			"bmod_dll/gmod_cvars_commands.cpp"
			"bmod_dll/lua_integration.cpp"
			"bmod_dll/lua_integration.h"
		"bmod_dll/gmod_paint.cpp"
		"bmod_dll/gmod_spawn.cpp"
		"bmod_dll/gmod_paint.h"
		"bmod_dll/gmod_runfunction.cpp"
		"bmod_dll/gmod_runfunction.h"
		"bmod_dll/gmod_player_start.cpp"
		"bmod_dll/gmod_gamesetup.cpp"
		"bmod_dll/lua_utility.cpp"
		# "bmod_dll/te_gauss.cpp" - TE_GaussExplosion is defined in weapon_gauss.cpp
		"bmod_dll/skill_cvars.cpp"
	#}
	)
	SRC_GRP(
	SUBGROUP "Lua 5.0.3 Integration"
	SOURCES
	#{
		"bmod_dll/lua_wrapper.cpp"
		"bmod_dll/lua_wrapper.h"
		"bmod_dll/lua_compat.cpp"
		"bmod_dll/lua_compat.h"
		"bmod_dll/lua_entity_funcs.cpp"
		"bmod_dll/lua_player_funcs.cpp"
		"bmod_dll/lua_physics_funcs.cpp"
		"bmod_dll/lua_file_funcs.cpp"
		"bmod_dll/lua_effect_funcs.cpp"
		"bmod_dll/lua_gameevent_funcs.cpp"
	#}
	)
	SRC_GRP(
	SUBGROUP "Vehicles"
	SOURCES
	#{
		"bmod_dll/vehicle_base.cpp"
		"bmod_dll/vehicle_baseserver.cpp"
		"bmod_dll/vehicle_jeep.cpp"
		"bmod_dll/vehicle_airboat.cpp"
		"bmod_dll/vehicle_apc.cpp"
		"bmod_dll/vehicle_chopper.cpp"
		"bmod_dll/vehicle_crane.cpp"
		"bmod_dll/vehicle_crane.h"
		"bmod_dll/vehicle_jetski.cpp"
		"bmod_dll/npc_vehicledriver.cpp"
		"bmod_dll/npc_vehicledriver.h"
		"bmod_dll/gmod_vehicle_controls.cpp"
		"bmod_dll/gmod_vehicle_controls.h"
	#}
	)
END_SRC( SERVER_BMOD_SOURCE_FILES "Source Files" )

list(APPEND SERVER_BMOD_SOURCE_FILES
	"${SRCDIR}/dlls/te.cpp"
	"${SRCDIR}/dlls/bmod_dll/npc_barnacle.cpp"
	"${SRCDIR}/public/KeyValues.cpp"
)

if(NOT TARGET lua_503)
	add_library(lua_503 STATIC
		"${SRCDIR}/lua-5.0.3/src/lapi.c"
		"${SRCDIR}/lua-5.0.3/src/lcode.c"
		"${SRCDIR}/lua-5.0.3/src/ldebug.c"
		"${SRCDIR}/lua-5.0.3/src/ldo.c"
		"${SRCDIR}/lua-5.0.3/src/ldump.c"
		"${SRCDIR}/lua-5.0.3/src/lfunc.c"
		"${SRCDIR}/lua-5.0.3/src/lgc.c"
		"${SRCDIR}/lua-5.0.3/src/llex.c"
		"${SRCDIR}/lua-5.0.3/src/lmem.c"
		"${SRCDIR}/lua-5.0.3/src/lobject.c"
		"${SRCDIR}/lua-5.0.3/src/lopcodes.c"
		"${SRCDIR}/lua-5.0.3/src/lparser.c"
		"${SRCDIR}/lua-5.0.3/src/lstate.c"
		"${SRCDIR}/lua-5.0.3/src/lstring.c"
		"${SRCDIR}/lua-5.0.3/src/ltable.c"
		"${SRCDIR}/lua-5.0.3/src/ltm.c"
		"${SRCDIR}/lua-5.0.3/src/lundump.c"
		"${SRCDIR}/lua-5.0.3/src/lvm.c"
		"${SRCDIR}/lua-5.0.3/src/lzio.c"
		"${SRCDIR}/lua-5.0.3/src/lib/lauxlib.c"
		"${SRCDIR}/lua-5.0.3/src/lib/lbaselib.c"
		"${SRCDIR}/lua-5.0.3/src/lib/ldblib.c"
		"${SRCDIR}/lua-5.0.3/src/lib/liolib.c"
		"${SRCDIR}/lua-5.0.3/src/lib/lmathlib.c"
		"${SRCDIR}/lua-5.0.3/src/lib/ltablib.c"
		"${SRCDIR}/lua-5.0.3/src/lib/lstrlib.c"
		"${SRCDIR}/lua-5.0.3/src/lib/loadlib.c"
	)
	target_include_directories(lua_503 PUBLIC "${SRCDIR}/lua-5.0.3/src" "${SRCDIR}/lua-5.0.3/include")
	target_compile_definitions(lua_503 PRIVATE LUA_USE_APICHECK)
endif()

set(
	SERVER_BMOD_EXCLUDE_SOURCES

	"monstermaker.h"
)

add_object(
	TARGET server_bmod
	MODULE
	INSTALL_OUTNAME "server"
	INSTALL_DEST "${GAMEDIR}/bmod/bin"
	SOURCES ${SERVER_BMOD_SOURCE_FILES}
)

target_include_directories(
	server_bmod PRIVATE

	"${SRCDIR}/game_shared/hl2"
    "${SRCDIR}/dlls/hl2_dll"
    "${SRCDIR}/dlls/bmod_dll"
)

target_include_directories(
	server_bmod BEFORE PRIVATE

    "${SRCDIR}/lua-5.0.3/include"
    "${SRCDIR}/lua-5.0.3/src"
)

target_compile_definitions(
	server_bmod PRIVATE

	HL2_DLL
	BMOD_DLL
    USES_SAVERESTORE
)

target_use_server_base( server_bmod SERVER_BMOD_EXCLUDE_SOURCES )

target_link_libraries(
	server_bmod PRIVATE
		lua_503
		"${CMAKE_BINARY_DIR}/materialsystem/${CMAKE_CFG_INTDIR}/materialsystem.lib"
		"${CMAKE_BINARY_DIR}/vphysics/${CMAKE_CFG_INTDIR}/vphysics.lib"
		tier0
		vstdlib
)

add_dependencies(server_bmod materialsystem vphysics)
