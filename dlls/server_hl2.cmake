include_guard( GLOBAL )

include( "${CMAKE_CURRENT_LIST_DIR}/server_base.cmake" )

set( SERVER_HL2_SOURCE_FILES )
BEGIN_SRC( SERVER_HL2_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SUBGROUP "HL2 DLL"
		SOURCES
		#{
			"basebludgeonweapon.cpp" # !CSTRIKE && !TF2 && !HL1 # SanyaSho: moved from server_base.cmake
			"basebludgeonweapon.h" # !CSTRIKE && !HL1

			"hl2_dll/ai_interactions.h" # !CSTRIKE && !TF2

			"hl2_dll/ai_allymanager.cpp" # !CSTRIKE
			"hl2_dll/ar2_explosion.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/ar2_explosion.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/basehlcombatweapon.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/basehlcombatweapon.h" # !CSTRIKE && !HL1
			"${SRCDIR}/game_shared/hl2/basehlcombatweapon_shared.cpp" # !CSTRIKE && !TF2 && !HL1
			"${SRCDIR}/game_shared/hl2/basehlcombatweapon_shared.h" # !CSTRIKE && !HL1
			"hl2_dll/cbasehelicopter.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/cbasehelicopter.h" # !CSTRIKE && !TF2
			"hl2_dll/cbasespriteprojectile.cpp" # !CSTRIKE && !TF2
			"hl2_dll/cbasespriteprojectile.h" # !CSTRIKE && !TF2
			"hl2_dll/energy_wave.cpp" # !CSTRIKE && !TF2
			"hl2_dll/energy_wave.h" # !CSTRIKE && !TF2
			"hl2_dll/env_speaker.cpp" # !HL1
			"hl2_dll/extinguisherjet.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/extinguisherjet.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/Func_Monitor.cpp" # !CSTRIKE && !TF2
			"hl2_dll/Func_Monitor.h"
			"hl2_dll/func_recharge.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/func_tank.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_ar2.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_ar2.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_beam.cpp" # !CSTRIKE && !TF2
			"hl2_dll/grenade_beam.h" # !CSTRIKE && !TF2
			"hl2_dll/grenade_brickbat.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_brickbat.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_bugbait.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_bugbait.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_energy.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_energy.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_frag.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_frag.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_homer.cpp" # !CSTRIKE && !TF2
			"hl2_dll/grenade_homer.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_hopwire.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_hopwire.h"
			"hl2_dll/grenade_molotov.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_molotov.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_pathfollower.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_pathfollower.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_satchel.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_satchel.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_scanner.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_scanner.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_spit.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_spit.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_tripmine.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_tripmine.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_tripwire.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/grenade_tripwire.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/hl2_client.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/hl2_eventlog.cpp" # !CSTRIKE && !TF2 && !HL1
			"${SRCDIR}/game_shared/hl2/hl2_gamerules.cpp" # !CSTRIKE && !TF2 && !HL1
			"${SRCDIR}/game_shared/hl2/hl2_gamerules.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/hl2_player.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/hl2_player.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/hl2_playerlocaldata.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/hl2_playerlocaldata.h" # !CSTRIKE && !TF2 && !HL1
			"${SRCDIR}/game_shared/hl2/hl2_usermessages.cpp" # !CSTRIKE && !TF2 && !HL1
			"${SRCDIR}/game_shared/hl_gamemovement.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/hl_playermove.cpp" # !CSTRIKE && !TF2
			"hl2_dll/item_ammo.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/item_battery.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/item_healthkit.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/item_suit.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/look_door.cpp" # !CSTRIKE && !TF2
			"hl2_dll/monster_dummy.cpp" # !CSTRIKE && !TF2
			"hl2_dll/npc_alyx.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_alyx.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_antlion.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_antliongrub.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_antliongrub.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_antlionguard.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_assassin.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_assassin.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_attackchopper.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_barnacle.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_barnacle.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_barney.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_BaseZombie.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_BaseZombie.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_breen.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_bullseye.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_bullseye.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_bullsquid.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_bullsquid.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_citizen17.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_citizen17.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_combine.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_combine.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_combinecamera.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_combinedropship.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_combinee.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_combinee.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_combineguard.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_combinegunship.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_combines.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_combines.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_conscript.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_conscript.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_crabsynth.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_cranedriver.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_crow.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_eli.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_enemyfinder.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_fastzombie.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_headcrab.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_headcrab.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_houndeye.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_houndeye.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_hydra.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_hydra.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_ichthyosaur.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_kleiner.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_launcher.cpp" # !CSTRIKE && !TF2 && !HL1
			"npc_leader.cpp" # !CSTRIKE && !TF2 && !HL1 # SanyaSho: moved from server_base.cmake
			"npc_leader.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_lightstalk.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_lightstalk.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_manhack.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_manhack.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_metropolice.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_metropolice.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_missiledefense.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_monk.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_mortarsynth.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_mortarsynth.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_mossman.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_odell.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_odell.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_PoisonZombie.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_roller.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_roller.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_rollerbuddy.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_rollerbuddy.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_rollerbull.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_rollerdozer.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_rollermine.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_rollerturret.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_scanner.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_spotlight.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_sscanner.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_sscanner.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_sscanner_beam.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_sscanner_beam.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_stalker.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_stalker.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_strider.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_turret_ceiling.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_turret_floor.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_vehicledriver.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_vehicledriver.h"
			"hl2_dll/npc_vortigaunt.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_vortigaunt.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_wscanner.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_wscanner.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/npc_zombie.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/player_control.cpp" # !CSTRIKE && !TF2
			"hl2_dll/player_control.h" # !CSTRIKE && !TF2
			"hl2_dll/player_manhack.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/player_missile.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/point_apc_controller.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/Point_Camera.cpp" # !CSTRIKE && !TF2
			"hl2_dll/Point_Camera.h"
			"hl2_dll/proto_sniper.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/scanner_shield.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/scanner_shield.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/hl2_ents.cpp"
			"hl2_dll/spotlightend.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/spotlightend.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/vehicle_airboat.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/vehicle_apc.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/vehicle_base.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/vehicle_baseserver.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/vehicle_chopper.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/vehicle_crane.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/vehicle_crane.h"
			"hl2_dll/vehicle_jeep.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/WaterLODControl.cpp" # !CSTRIKE && !TF2
			"hl2_dll/weapon_ar1.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_ar2.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_ar2.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_binoculars.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_brickbat.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_brickbat.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_bugbait.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_cguard.cpp" # !CSTRIKE && !TF2 && !HL1
			"weapon_cubemap.cpp" # !CSTRIKE && !TF2 && !HL1 # SanyaSho: moved from server_base.cmake
			"hl2_dll/weapon_crowbar.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_extinguisher.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_flaregun.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_flaregun.h" # !CSTRIKE && !HL1
			"hl2_dll/weapon_frag.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_gauss.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_gauss.h" # !CSTRIKE && !HL1
			"hl2_dll/weapon_hgm1.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_hopwire.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_iceaxe.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_immolator.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_irifle.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_manhack.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_molotov.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_molotov.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_physcannon.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_physcannon.h"
			"hl2_dll/weapon_pistol.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_rollerwand.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_rpg.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_rpg.h" # !CSTRIKE && !HL1
			"hl2_dll/weapon_shotgun.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_slam.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_slam.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_smg1.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_smg2.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_sniperrifle.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_stickylauncher.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_stunstick.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_thumper.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_tripwire.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/weapon_tripwire.h" # !CSTRIKE && !TF2 && !HL1
		#}
	)
END_SRC( SERVER_HL2_SOURCE_FILES "Source Files" )

set(
	SERVER_HL2_EXCLUDE_SOURCES

	"monstermaker.h"
)

add_object(
	TARGET server_hl2
	MODULE
	INSTALL_OUTNAME "server"
	INSTALL_DEST "${GAMEDIR}/hl2/bin"
	SOURCES ${SERVER_HL2_SOURCE_FILES}
)

target_include_directories(
	server_hl2 PRIVATE

	"${SRCDIR}/game_shared/hl2"
    "${SRCDIR}/dlls/hl2_dll"
)

target_compile_definitions(
	server_hl2 PRIVATE

	HL2_DLL
    USES_SAVERESTORE
)

target_use_server_base( server_hl2 SERVER_HL2_EXCLUDE_SOURCES )
