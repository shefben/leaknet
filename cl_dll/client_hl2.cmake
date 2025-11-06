include_guard( GLOBAL )

include( "${CMAKE_CURRENT_LIST_DIR}/client_base.cmake" )

set( CLIENT_HL2_SOURCE_FILES )
BEGIN_SRC( CLIENT_HL2_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SUBGROUP "HL2 DLL"
		SOURCES
		#{
			"${SRCDIR}/game_shared/hl2/basehlcombatweapon_shared.cpp" # !CSTRIKE && !TF2 && !HL1 # SanyaSho: moved from client_base.cmake
			"${SRCDIR}/game_shared/hl2/basehlcombatweapon_shared.h"

			"hl2_dll/c_vehicle_jeep.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/fx_hl2_impacts.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_dll/hud_damageindicator.cpp" # !CSTRIKE && !TF2
			"hl2_hud/clientmode_hlnormal.cpp" # !CSTRIKE && !TF2
			"hl2_hud/clientmode_hlnormal.h"
			"hl2_hud/c_antlion_dust.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_ar2_explosion.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_assassin_smoke.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_barnacle.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_barney.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_basehelicopter.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_basehelicopter.h"
			"hl2_hud/c_basehlcombatweapon.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_hud/c_basehlcombatweapon.h"
			"hl2_hud/c_basehlplayer.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_hud/c_basehlplayer.h" # !CSTRIKE && !HL1
			"hl2_hud/c_combineguard.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_corpse.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_corpse.h" # !CSTRIKE && !TF2
			"hl2_hud/c_energy_wave.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_extinguisher.cpp" # !CSTRIKE && !TF2
			"hl2_hud/C_Func_Monitor.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_hl2_playerlocaldata.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_hl2_playerlocaldata.h" # !CSTRIKE && !TF2
			"hl2_hud/c_npc_combinegunship.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_hud/c_npc_hydra.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_npc_manhack.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_particle_storm.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_particle_storm.h" # !CSTRIKE && !TF2
			"hl2_hud/c_plasma_beam_node.cpp" # !CSTRIKE && !TF2
			"hl2_hud/C_Point_Camera.cpp" # !CSTRIKE && !TF2
			"hl2_hud/C_Point_Camera.h" # !CSTRIKE && !TF2
			"hl2_hud/c_rotorwash.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_spotlight_end.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_strider.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_te_concussiveexplosion.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_te_flare.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_vehicle_crane.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_hud/C_WaterLODControl.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_weapon_gravitygun.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_weapon_sticklauncher.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_hud/c_weapon__stubs_hl2.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_hud/energy_wave_effect.cpp" # !CSTRIKE && !TF2
			"hl2_hud/energy_wave_effect.h" # !CSTRIKE && !TF2
			"hl2_hud/fx_antlion.cpp" # !TF2
			"hl2_hud/fx_bugbait.cpp" # !CSTRIKE
			"hl2_hud/fx_hl2_tracers.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_hud/hl2_clientmode.cpp" # !CSTRIKE && !TF2 && !HL1
			"${SRCDIR}/game_shared/hl2/hl2_gamerules.cpp" # !CSTRIKE && !TF2 && !HL1
			"${SRCDIR}/game_shared/hl2/hl2_gamerules.h" # !CSTRIKE && !TF2 && !HL1
			"${SRCDIR}/game_shared/hl2/hl2_usermessages.cpp" # !CSTRIKE && !TF2 && !HL1
			"${SRCDIR}/game_shared/hl_gamemovement.cpp" # !CSTRIKE && !TF2 && !HL1
			"${SRCDIR}/game_shared/hl2_shareddefs.h" # !CSTRIKE && !TF2
			"hl2_hud/hl_in_main.cpp" # !CSTRIKE && !TF2
			"hl2_hud/hl_prediction.cpp" # !CSTRIKE && !TF2
			"hl2_hud/hud_ammo.cpp" # !CSTRIKE && !TF2
			"hl2_hud/hud_battery.cpp" # !CSTRIKE && !TF2
			"hl2_hud/hud_blood.cpp" # !CSTRIKE && !HL1
			"hl2_hud/hud_flashlight.cpp" # !CSTRIKE # HL2DBG Noprecmp?
			"hl2_hud/hud_health.cpp" # !CSTRIKE && !TF2
			"hl2_hud/hud_quickinfo.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_hud/hud_suitpower.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_hud/hud_suitpower.h" # !CSTRIKE && !TF2 && !HL1
			"hl2_hud/hud_weaponselection.cpp" # !CSTRIKE && !TF2
			"hl2_hud/hud_zoom.cpp" # !CSTRIKE && !TF2 && !HL1
			"hl2_hud/shieldproxy.cpp" # !CSTRIKE && !TF2
			"hl2_hud/vgui_rootpanel_hl2.cpp" # !CSTRIKE && !TF2
			"hl2_hud/WaterLODMaterialProxy.cpp" # !CSTRIKE && !TF2
		#}
	)
END_SRC( CLIENT_HL2_SOURCE_FILES "Source Files" )

set(
	CLIENT_HL2_EXCLUDE_SOURCES

	"BaseModViewport.cpp"
	"BaseModViewport.h"
)

add_object(
	TARGET client_hl2
	MODULE
	INSTALL_DEST "${GAMEDIR}/hl2/bin"
	INSTALL_OUTNAME "client"
	SOURCES ${CLIENT_HL2_SOURCE_FILES}
)

target_include_directories(
	client_hl2 PRIVATE

	"hl2_dll"
    "hl2_hud"
    "hl2_hud/elements"
    "${SRCDIR}/game_shared/hl2"
)

target_compile_definitions(
	client_hl2 PRIVATE

	HL2_CLIENT_DLL
)

target_use_client_base( client_hl2 CLIENT_HL2_EXCLUDE_SOURCES )
