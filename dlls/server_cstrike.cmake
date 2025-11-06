include_guard( GLOBAL )

include( "${CMAKE_CURRENT_LIST_DIR}/server_base.cmake" )

set( SERVER_CSTRIKE_SOURCE_FILES )
BEGIN_SRC( SERVER_CSTRIKE_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SUBGROUP "HL2 DLL"
		SOURCES
		#{

		#}
	)

	SRC_GRP(
		SUBGROUP "CounterStrike DLL"
		SOURCES
		#{
			"${SRCDIR}/game_shared/cstrike/cs_gamemovement.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/cs_gamerules.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/cs_gamerules.h" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/cs_playeranimstate.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/cs_playeranimstate.h" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/cs_player_shared.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/cs_shareddefs.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/cs_shareddefs.h" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/cs_usermessages.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/cs_weapon_parse.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/cs_weapon_parse.h" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/flashbang_projectile.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/flashbang_projectile.h" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/hegrenade_projectile.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/hegrenade_projectile.h" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_ak47.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_aug.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_awp.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_basecsgrenade.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_basecsgrenade.h" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_c4.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_c4.h" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_csbase.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_csbase.h" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_csbasegun.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_csbasegun.h" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_deagle.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_famas.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_fiveseven.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_flashbang.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_g3sg1.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_galil.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_glock.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_hegrenade.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_knife.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_m249.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_m3.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_m4a1.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_mac10.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_mp5navy.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_p228.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_p90.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_scout.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_sg550.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_sg552.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_ump45.cpp" # !TF2 && !HL2 && !HL1
			"${SRCDIR}/game_shared/cstrike/weapon_usp.cpp" # !TF2 && !HL2 && !HL1

			"cstrike/cs_bot_temp.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/cs_client.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/cs_client.h" # !TF2 && !HL2 && !HL1
			"cstrike/cs_eventlog.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/cs_hostage.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/cs_hostage.h" # !TF2 && !HL2 && !HL1
			"cstrike/cs_player.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/cs_player.h" # !TF2 && !HL2 && !HL1
			"cstrike/cs_playermove.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/cs_team.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/cs_team.h" # !TF2 && !HL2 && !HL1
			"cstrike/func_bomb_target.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/func_buy_zone.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/item_assaultsuit.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/item_kevlar.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/mapinfo.cpp" # !TF2 && !HL2 && !HL1
			"cstrike/mapinfo.h" # !TF2 && !HL2 && !HL1
		#}
	)

	SRC_GRP(
		SUBGROUP "HL2 DLL"
		SOURCES
		#{
			"hl2_dll/env_speaker.cpp" # !HL1
		#}
	)
END_SRC( SERVER_CSTRIKE_SOURCE_FILES "Source Files" )

set(
	SERVER_CSTRIKE_EXCLUDE_SOURCES

	"${SRCDIR}/game_shared/weapon_parse_default.cpp"
	
	"basegrenade_concussion.cpp"
	"basegrenade_contact.cpp"
	"basegrenade_timed.cpp"

	"grenadethrown.cpp"
	"grenadethrown.h"

	"h_cycler.cpp"
	"h_cycler.h"

	"monstermaker.cpp"
	"monstermaker.h"

	"physgun.cpp"

	"hl2_dll/antlion_dust.cpp"
	"hl2_dll/assassin_smoke.cpp"
	"hl2_dll/rotorwash.cpp"
	"hl2_dll/rotorwash.h"

	"smoke_trail.cpp"

	"${SRCDIR}/game_shared/hl2_player_shared.h"
	"${SRCDIR}/game_shared/hl2_shareddefs.h"
	"${SRCDIR}/game_shared/hl_movedata.h"
	"${SRCDIR}/game_shared/ragdoll_shared.h"
	"${SRCDIR}/game_shared/solidsetdefaults.h"
	"${SRCDIR}/game_shared/tf2/baseobject_shared.h"
	"${SRCDIR}/game_shared/tf2/basetfvehicle.h"
	"${SRCDIR}/game_shared/tf2/env_laserdesignation.h"
	"${SRCDIR}/game_shared/tf2/ihasbuildpoints.h"
	"${SRCDIR}/game_shared/tf2/tf_obj_base_manned_gun.h"
	"${SRCDIR}/game_shared/tf2/tf_obj_manned_plasmagun_shared.h"
	"${SRCDIR}/game_shared/touchlink.h"

	"CRagdollMagnet.h"
	"EntityFlame.h"
	"physics_bone_follower.h"
	"vehicle_base.h"
	"tf2_dll/tf_func_construction_yard.h"
	"tf2_dll/tf_func_mass_teleport.h"
	"tf2_dll/tf_obj_armor_upgrade.h"
	"tf2_dll/tf_obj_barbed_wire.h"
	"tf2_dll/tf_obj_buff_station.h"
	"tf2_dll/tf_obj_bunker.h"
	"tf2_dll/tf_obj_manned_missilelauncher.h"
	"tf2_dll/tf_obj_sandbag_bunker.h"
	"tf2_dll/tf_vehicle_tank.h"
	"tf2_dll/tf_vehicle_teleport_station.h"
)

add_object(
	TARGET server_cstrike
	MODULE
	INSTALL_OUTNAME "server"
	INSTALL_DEST "${GAMEDIR}/cstrike/bin"
	SOURCES ${SERVER_CSTRIKE_SOURCE_FILES}
)

target_include_directories(
	server_cstrike PRIVATE

	"${SRCDIR}/game_shared/cstrike"
	"${SRCDIR}/dlls/cstrike"
)

target_compile_definitions(
	server_cstrike PRIVATE

	CSTRIKE_DLL
	USES_SAVERESTORE
)

target_use_server_base( server_cstrike SERVER_CSTRIKE_EXCLUDE_SOURCES )