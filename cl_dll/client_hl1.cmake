include_guard( GLOBAL )

include( "${CMAKE_CURRENT_LIST_DIR}/client_base.cmake" )

set( CLIENT_HL1_SOURCE_FILES )
BEGIN_SRC( CLIENT_HL1_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SUBGROUP "HL2 DLL"
		SOURCES
		#{
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
			"hl2_hud/c_combineguard.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_corpse.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_corpse.h" # !CSTRIKE && !TF2
			"hl2_hud/c_energy_wave.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_extinguisher.cpp" # !CSTRIKE && !TF2
			"hl2_hud/C_Func_Monitor.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_hl2_playerlocaldata.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_hl2_playerlocaldata.h" # !CSTRIKE && !TF2
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
			"hl2_hud/C_WaterLODControl.cpp" # !CSTRIKE && !TF2
			"hl2_hud/c_weapon_gravitygun.cpp" # !CSTRIKE && !TF2
			"hl2_hud/energy_wave_effect.cpp" # !CSTRIKE && !TF2
			"hl2_hud/energy_wave_effect.h" # !CSTRIKE && !TF2
			"hl2_hud/fx_antlion.cpp" # !TF2
			"hl2_hud/fx_bugbait.cpp" # !CSTRIKE
			"hl2_hud/hl_in_main.cpp" # !CSTRIKE && !TF2
			"hl2_hud/hl_prediction.cpp" # !CSTRIKE && !TF2
			"hl2_hud/hud_ammo.cpp" # !CSTRIKE && !TF2
			"hl2_hud/hud_battery.cpp" # !CSTRIKE && !TF2
			"hl2_hud/hud_flashlight.cpp" # !CSTRIKE
			"hl2_hud/hud_health.cpp" # !CSTRIKE && !TF2
			"hl2_hud/hud_weaponselection.cpp" # !CSTRIKE && !TF2
			"hl2_hud/shieldproxy.cpp" # !CSTRIKE && !TF2
			"hl2_hud/vgui_rootpanel_hl2.cpp" # !CSTRIKE && !TF2
			"hl2_hud/WaterLODMaterialProxy.cpp" # !CSTRIKE && !TF2

			"hl2_hud/c_basehlcombatweapon.h"
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
			"${SRCDIR}/game_shared/hl1/hl1_usermessages.cpp" # !CSTRIKE && !TF2 && !HL2

			"${SRCDIR}/game_shared/hl1/hl1_player_shared.h" # !CSTRIKE && !TF2 && !HL2

			"hl1/hl1_clientmode.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1/hl1_c_beamfollow.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1/hl1_c_player.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1/hl1_c_player.h" # !CSTRIKE && !TF2 && !HL2
			"hl1/hl1_c_rpg_rocket.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1/hl1_c_stickybolt.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1/hl1_c_weapon__stubs.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1/hl1_fx_gibs.cpp" # !CSTRIKE && !TF2 && !HL2
			"hl1/hl1_fx_impacts.cpp" # !CSTRIKE && !TF2 && !HL2
		#}
	)
END_SRC( CLIENT_HL1_SOURCE_FILES "Source Files" )

set(
	CLIENT_HL1_EXCLUDE_SOURCES

	"BaseModViewport.cpp"
	"BaseModViewport.h"

	"${SRCDIR}/game_shared/hl2/hl2_gamerules.h" # !CSTRIKE && !TF2 && !HL1
	"hl2_hud/c_basehlplayer.h" # !CSTRIKE && !HL1
	"hl2_hud/hud_suitpower.h" # !CSTRIKE && !TF2 && !HL1

	"hud_numericdisplay.h" # !TF2 && !HL1
)

add_object(
	TARGET client_hl1
	MODULE
	INSTALL_DEST "${GAMEDIR}/hl1/bin"
	INSTALL_OUTNAME "client"
	SOURCES ${CLIENT_HL1_SOURCE_FILES}
)

target_include_directories(
	client_hl1 PRIVATE

	"hl1_hud"
	"hl2_dll"
	"hl2_hud"
	"hl2_hud/elements"
	"${SRCDIR}/game_shared/hl1"
	"${SRCDIR}/game_shared/hl2"
)

target_compile_definitions(
	client_hl1 PRIVATE

	HL1_CLIENT_DLL
)

target_use_client_base( client_hl1 CLIENT_HL1_EXCLUDE_SOURCES )
