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
			"${SRCDIR}/game_shared/bmod/bmod_usermessages.cpp" # Now safe with preprocessor conditionals
			"hl2_dll/hl2_client.cpp"

			# Physics cannon needed for GMod tools
			"hl2_dll/weapon_physcannon.cpp"
			"hl2_dll/weapon_physcannon.h"

			# Real GMod-style physics gun (distinct weapon/entity from the HL2
			# physcannon above - client c_weapon_gravitygun.cpp already expects
			# this exact ServerClass/DT pairing, see physgun.cpp).
			"physgun.cpp"

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
			"bmod_dll/weapon_slam.cpp"

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
			"bmod_dll/gmod_balloon.cpp"
			"bmod_dll/gmod_balloon.h"
			"bmod_dll/gmod_swep.cpp"
			"bmod_dll/gmod_swep.h"
			"bmod_dll/weapon_scripted.cpp"
			"bmod_dll/gmod_gamemode.cpp"
			"bmod_dll/gmod_gamemode.h"
			"bmod_dll/gmod_make_compat.cpp"
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
			"bmod_dll/gmod_serverrules.cpp"
			"bmod_dll/gmod_serverrules.h"
			"bmod_dll/gmod_config.cpp"
			"bmod_dll/gmod_config.h"
			"bmod_dll/gmod_entities.cpp"
			"bmod_dll/gmod_entities.h"
			"bmod_dll/gmod_materials.cpp"
			"bmod_dll/gmod_materials.h"
			"bmod_dll/gmod_physproperties.cpp"
			"bmod_dll/gmod_physproperties.h"
			"bmod_dll/gmod_dynamite.cpp"
			"bmod_dll/gmod_dynamite.h"
			"bmod_dll/gmod_emitter.cpp"
			"bmod_dll/gmod_emitter.h"
			"bmod_dll/gmod_sprites.cpp"
			"bmod_dll/gmod_sprites.h"
			"bmod_dll/gmod_serverlimits.cpp"
			"bmod_dll/gmod_serverlimits.h"
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
		SUBGROUP "GMod Tool Gun"
		SOURCES
		#{
			"bmod_dll/weapon_tool.cpp"
			"bmod_dll/weapon_tool.h"
			"bmod_dll/tool_dispatch.h"
			"bmod_dll/tool_constraints.cpp"
			"bmod_dll/tool_poser.cpp"
			"bmod_dll/tool_remover.cpp"
			"bmod_dll/tool_simple.cpp"
			"bmod_dll/tool_paint.cpp"
			"bmod_dll/tool_duplicator.cpp"
			"bmod_dll/tool_color.cpp"
			"bmod_dll/tool_material.cpp"
			"bmod_dll/tool_attach.cpp"
			"bmod_dll/tool_gun.cpp"
			"bmod_dll/tool_camera.cpp"
			"bmod_dll/tool_npc.cpp"
			"bmod_dll/tool_inflator.cpp"
		#}
	)
	SRC_GRP(
		SUBGROUP "HL2 NPCs"
		SOURCES
		#{
			# The NPC tool / "gm_context npc" panel spawns these by classname
			# (npc_create <class>). Without their LINK_ENTITY_TO_CLASS in the
			# server binary every spawn failed with "Attempted to create unknown
			# entity type npc_...!". Same file set server_hl2.cmake builds.
			"bmod_dll/ai_allymanager.cpp"
			"npc_leader.cpp"
			"npc_leader.h"

			"bmod_dll/npc_alyx.cpp"
			"bmod_dll/npc_alyx.h"
			"bmod_dll/npc_antlion.cpp"
			"bmod_dll/npc_antliongrub.cpp"
			"bmod_dll/npc_antliongrub.h"
			"bmod_dll/npc_antlionguard.cpp"
			"bmod_dll/npc_assassin.cpp"
			"bmod_dll/npc_assassin.h"
			"bmod_dll/npc_attackchopper.cpp"
			"bmod_dll/npc_barney.cpp"
			"bmod_dll/npc_BaseZombie.cpp"
			"bmod_dll/npc_BaseZombie.h"
			"bmod_dll/npc_breen.cpp"
			"bmod_dll/npc_bullseye.cpp"
			"bmod_dll/npc_bullseye.h"
			"bmod_dll/npc_bullsquid.cpp"
			"bmod_dll/npc_bullsquid.h"
			"bmod_dll/npc_citizen17.cpp"
			"bmod_dll/npc_citizen17.h"
			"bmod_dll/npc_combine.cpp"
			"bmod_dll/npc_combine.h"
			"bmod_dll/npc_combinecamera.cpp"
			"bmod_dll/npc_combinedropship.cpp"
			"bmod_dll/npc_combinee.cpp"
			"bmod_dll/npc_combinee.h"
			"bmod_dll/npc_combineguard.cpp"
			"bmod_dll/npc_combinegunship.cpp"
			"bmod_dll/npc_combines.cpp"
			"bmod_dll/npc_combines.h"
			"bmod_dll/npc_conscript.cpp"
			"bmod_dll/npc_conscript.h"
			"bmod_dll/npc_crabsynth.cpp"
			"bmod_dll/npc_cranedriver.cpp"
			"bmod_dll/npc_crow.cpp"
			"bmod_dll/npc_eli.cpp"
			"bmod_dll/npc_enemyfinder.cpp"
			"bmod_dll/npc_fastzombie.cpp"
			"bmod_dll/npc_headcrab.cpp"
			"bmod_dll/npc_headcrab.h"
			"bmod_dll/npc_houndeye.cpp"
			"bmod_dll/npc_houndeye.h"
			"bmod_dll/npc_hydra.cpp"
			"bmod_dll/npc_hydra.h"
			"bmod_dll/npc_ichthyosaur.cpp"
			"bmod_dll/npc_kleiner.cpp"
			"bmod_dll/npc_launcher.cpp"
			"bmod_dll/npc_lightstalk.cpp"
			"bmod_dll/npc_lightstalk.h"
			"bmod_dll/npc_manhack.cpp"
			"bmod_dll/npc_manhack.h"
			"bmod_dll/npc_metropolice.cpp"
			"bmod_dll/npc_metropolice.h"
			"bmod_dll/npc_missiledefense.cpp"
			"bmod_dll/npc_monk.cpp"
			"bmod_dll/npc_mortarsynth.cpp"
			"bmod_dll/npc_mortarsynth.h"
			"bmod_dll/npc_mossman.cpp"
			"bmod_dll/npc_odell.cpp"
			"bmod_dll/npc_odell.h"
			"bmod_dll/npc_PoisonZombie.cpp"
			"bmod_dll/npc_roller.cpp"
			"bmod_dll/npc_roller.h"
			"bmod_dll/npc_rollerbuddy.cpp"
			"bmod_dll/npc_rollerbuddy.h"
			"bmod_dll/npc_rollerbull.cpp"
			"bmod_dll/npc_rollerdozer.cpp"
			"bmod_dll/npc_rollermine.cpp"
			"bmod_dll/npc_rollerturret.cpp"
			"bmod_dll/npc_scanner.cpp"
			"bmod_dll/npc_spotlight.cpp"
			"bmod_dll/npc_sscanner.cpp"
			"bmod_dll/npc_sscanner.h"
			"bmod_dll/npc_sscanner_beam.cpp"
			"bmod_dll/npc_sscanner_beam.h"
			"bmod_dll/npc_stalker.cpp"
			"bmod_dll/npc_stalker.h"
			"bmod_dll/npc_strider.cpp"
			"bmod_dll/npc_turret_ceiling.cpp"
			"bmod_dll/npc_turret_floor.cpp"
			"bmod_dll/npc_vortigaunt.cpp"
			"bmod_dll/npc_vortigaunt.h"
			"bmod_dll/npc_wscanner.cpp"
			"bmod_dll/npc_wscanner.h"
			"bmod_dll/npc_zombie.cpp"

			# Entities the NPCs above pull in
			"bmod_dll/cbasehelicopter.cpp"
			"bmod_dll/cbasehelicopter.h"
			"bmod_dll/cbasespriteprojectile.cpp"
			"bmod_dll/cbasespriteprojectile.h"
			"bmod_dll/grenade_bugbait.cpp"
			"bmod_dll/grenade_bugbait.h"
			"bmod_dll/grenade_energy.cpp"
			"bmod_dll/grenade_energy.h"
			"bmod_dll/grenade_homer.cpp"
			"bmod_dll/grenade_homer.h"
			"bmod_dll/grenade_pathfollower.cpp"
			"bmod_dll/grenade_pathfollower.h"
			"bmod_dll/grenade_spit.cpp"
			"bmod_dll/grenade_spit.h"
			"bmod_dll/weapon_cguard.cpp"	# also defines CreateConcussiveBlast
			"bmod_dll/player_control.cpp"
			"bmod_dll/player_control.h"
			"bmod_dll/proto_sniper.cpp"
			"bmod_dll/scanner_shield.cpp"
			"bmod_dll/scanner_shield.h"
			"bmod_dll/spotlightend.cpp"
			"bmod_dll/spotlightend.h"
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
