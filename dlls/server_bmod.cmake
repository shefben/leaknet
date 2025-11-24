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
			"${SRCDIR}/game_shared/hl2/hl2_usermessages.cpp"
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
			"bmod_dll/lua_integration.cpp"
			"bmod_dll/lua_integration.h"
		#}
	)
	SRC_GRP(
		SUBGROUP "Lua 5.0.3 Integration"
		SOURCES
		#{
			"bmod_dll/lua_wrapper.cpp"
			"bmod_dll/lua_wrapper.h"
		#}
	)
END_SRC( SERVER_BMOD_SOURCE_FILES "Source Files" )

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