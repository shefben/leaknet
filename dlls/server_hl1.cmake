include_guard( GLOBAL )

include( "${CMAKE_CURRENT_LIST_DIR}/server_base.cmake" )

set( SERVER_HL1_SOURCE_FILES )
BEGIN_SRC( SERVER_HL1_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SUBGROUP "HL2 DLL"
		SOURCES
		#{
			"hl2_dll/ai_interactions.h" # !CSTRIKE && !TF2

			"hl2_dll/ai_allymanager.cpp" # !CSTRIKE
			#"hl2_dll/antlion_dust.cpp" # !CSTRIKE && !TF2
			#"hl2_dll/antlion_dust.h"
			#"hl2_dll/assassin_smoke.cpp" # !CSTRIKE && !TF2
			#"hl2_dll/assassin_smoke.h" # !TF2
			"hl2_dll/cbasespriteprojectile.cpp" # !CSTRIKE && !TF2
			"hl2_dll/cbasespriteprojectile.h" # !CSTRIKE && !TF2
			"hl2_dll/energy_wave.cpp" # !CSTRIKE && !TF2
			"hl2_dll/energy_wave.h" # !CSTRIKE && !TF2
			"hl2_dll/Func_Monitor.cpp" # !CSTRIKE && !TF2
			"hl2_dll/Func_Monitor.h"
			"hl2_dll/grenade_beam.cpp" # !CSTRIKE && !TF2
			"hl2_dll/grenade_beam.h" # !CSTRIKE && !TF2
			"hl2_dll/grenade_homer.cpp" # !CSTRIKE && !TF2
			"hl2_dll/hl_playermove.cpp" # !CSTRIKE && !TF2
			"hl2_dll/look_door.cpp" # !CSTRIKE && !TF2
			"hl2_dll/monster_dummy.cpp" # !CSTRIKE && !TF2
			"hl2_dll/player_control.cpp" # !CSTRIKE && !TF2
			"hl2_dll/player_control.h" # !CSTRIKE && !TF2
			"hl2_dll/Point_Camera.cpp" # !CSTRIKE && !TF2
			"hl2_dll/Point_Camera.h"
			#"hl2_dll/rotorwash.cpp" # !CSTRIKE && !TF2
			#"hl2_dll/rotorwash.h" # !CSTRIKE && !TF2
			"hl2_dll/WaterLODControl.cpp" # !CSTRIKE && !TF2

			"hl2_dll/cbasehelicopter.h" # !CSTRIKE && !TF2
			"hl2_dll/grenade_hopwire.h"
			"hl2_dll/npc_vehicledriver.h"
			"hl2_dll/vehicle_crane.h"
			"hl2_dll/weapon_physcannon.h"
		#}
	)

	SRC_GRP(
		SUBGROUP "HL1 DLL"
		SOURCES
		#{
			"${SRCDIR}/game_shared/hl1/hl1_basecombatweapon_shared.cpp" # !CSTRIKE && !TF2 && !HL2
			"${SRCDIR}/game_shared/hl1/hl1_basecombatweapon_shared.h" # !CSTRIKE && !TF2 && !HL2
			"${SRCDIR}/game_shared/hl1/hl1_gamemovement.cpp" # !CSTRIKE && !TF2 && !HL2
			"${SRCDIR}/game_shared/hl1/hl1_gamemovement.h" # !CSTRIKE && !TF2 && !HL2
			"${SRCDIR}/game_shared/hl1/hl1_gamerules.cpp" # !CSTRIKE && !TF2 && !HL2
			"${SRCDIR}/game_shared/hl1/hl1_gamerules.h" # !CSTRIKE && !TF2 && !HL2
			"${SRCDIR}/game_shared/hl1/hl1_usermessages.cpp" # !CSTRIKE && !TF2 && !HL2

			"${SRCDIR}/game_shared/hl1/hl1_player_shared.h" # !CSTRIKE && !TF2 && !HL2

			"hl1_cbasehelicopter.h" # !CSTRIKE && !TF2 && !HL2

			"hl1_dll/hl1_ai_basenpc.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_ai_basenpc.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_basecombatweapon.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_basegrenade.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_basegrenade.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_client.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_ents.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_ents.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_env_speaker.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_eventlog.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_func_recharge.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_func_tank.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_grenade_mp5.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_grenade_mp5.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_grenade_spit.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_grenade_spit.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_items.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_items.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_item_ammo.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_item_battery.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_item_healthkit.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_item_longjump.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_item_suit.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_monstermaker.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_monstermaker.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_aflock.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_agrunt.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_apache.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_barnacle.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_barnacle.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_barney.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_barney.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_bigmomma.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_bloater.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_bullsquid.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_bullsquid.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_controller.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_gargantua.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_gargantua.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_gman.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_hassassin.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_headcrab.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_headcrab.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_hgrunt.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_hgrunt.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_hornet.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_hornet.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_houndeye.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_houndeye.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_ichthyosaur.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_ichthyosaur.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_leech.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_nihilanth.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_osprey.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_roach.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_scientist.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_scientist.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_snark.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_snark.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_talker.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_talker.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_tentacle.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_turret.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_vortigaunt.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_vortigaunt.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_zombie.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_npc_zombie.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_player.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_player.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_te_beamfollow.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_te_boltstick.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weaponbox.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_357.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_crossbow.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_crowbar.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_egon.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_gauss.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_glock.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_handgrenade.cpp" # !CSTRIKE && !TF2 && !HL2
			#"hl1_dll/hl1_weapon_handgrenade.h"
			"hl1_dll/hl1_weapon_hornetgun.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_mp5.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_mp5.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_rpg.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_rpg.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_satchel.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_satchel.h" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_shotgun.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_snark.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1_dll/hl1_weapon_tripmine.cpp" # !CSTRIKE && !TF2 && !HL2
		#}
	)
END_SRC( SERVER_HL1_SOURCE_FILES "Source Files" )

set(
	SERVER_HL1_EXCLUDE_SOURCES

	"monstermaker.cpp"
	"monstermaker.h"
	"${SRCDIR}/game_shared/ragdoll_shared.h"
	"${SRCDIR}/game_shared/solidsetdefaults.h"
	"${SRCDIR}/game_shared/tf2/baseobject_shared.h"
	"${SRCDIR}/game_shared/tf2/basetfvehicle.h"
	"${SRCDIR}/game_shared/tf2/env_laserdesignation.h"
	"${SRCDIR}/game_shared/tf2/ihasbuildpoints.h"
	"${SRCDIR}/game_shared/tf2/tf_gamemovement_sapper.h"
	"${SRCDIR}/game_shared/tf2/tf_obj_base_manned_gun.h"
	"${SRCDIR}/game_shared/tf2/tf_obj_manned_plasmagun_shared.h"
	"${SRCDIR}/game_shared/touchlink.h"
	"CRagdollMagnet.h"
	"EntityFlame.h"
	"physics_bone_follower.h"
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
	"vehicle_base.h"
)

add_object(
	TARGET server_hl1
	MODULE
	INSTALL_OUTNAME "server"
	INSTALL_DEST "${GAMEDIR}/hl1/bin"
	SOURCES ${SERVER_HL1_SOURCE_FILES}
)

target_include_directories(
	server_hl1 PRIVATE

    "${SRCDIR}/game_shared/hl1"
    "${SRCDIR}/dlls/hl1_dll"
    "${SRCDIR}/game_shared/hl2"
    "${SRCDIR}/dlls/hl2_dll"
)

target_compile_definitions(
	server_hl1 PRIVATE

    HL1_DLL
    USES_SAVERESTORE
)

target_use_server_base( server_hl1 SERVER_HL1_EXCLUDE_SOURCES )
