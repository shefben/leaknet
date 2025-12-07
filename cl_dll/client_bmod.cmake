include_guard( GLOBAL )

include( "${CMAKE_CURRENT_LIST_DIR}/client_base.cmake" )

set( CLIENT_BMOD_SOURCE_FILES )
BEGIN_SRC( CLIENT_BMOD_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SUBGROUP "HL2 Base"
		SOURCES
		#{
			"${SRCDIR}/game_shared/hl2/basehlcombatweapon_shared.cpp"
			"${SRCDIR}/game_shared/hl2/basehlcombatweapon_shared.h"
		#}
	)
	SRC_GRP(
		SUBGROUP "BarrysMod HUD"
		SOURCES
		#{
			"bmod_hud/bmod_spawnmenu.cpp"
			"bmod_hud/bmod_spawnmenu.h"
			"bmod_hud/bmod_spawnmenu_manager.cpp"
			"bmod_hud/clientmode_hlnormal.cpp"
			"bmod_hud/clientmode_hlnormal.h"
			#"bmod_hud/context_panel.cpp"
			"bmod_hud/context_panel.h"
			"bmod_hud/concommand.cpp"
			"bmod_hud/c_antlion_dust.cpp"
			"bmod_hud/c_ar2_explosion.cpp"
			"bmod_hud/c_assassin_smoke.cpp"
			"bmod_hud/c_barnacle.cpp"
			"bmod_hud/c_barney.cpp"
			"bmod_hud/c_basehelicopter.cpp"
			"bmod_hud/c_basehelicopter.h"
			"bmod_hud/c_basehlcombatweapon.cpp"
			"bmod_hud/c_basehlcombatweapon.h"
			"bmod_hud/c_basehlplayer.cpp"
			"bmod_hud/c_basehlplayer.h"
			"bmod_hud/c_combineguard.cpp"
			"bmod_hud/c_corpse.cpp"
			"bmod_hud/c_corpse.h"
			"bmod_hud/c_energy_wave.cpp"
			"bmod_hud/c_extinguisher.cpp"
			"bmod_hud/C_Func_Monitor.cpp"
			"bmod_hud/c_hl2_playerlocaldata.cpp"
			"bmod_hud/c_hl2_playerlocaldata.h"
			"bmod_hud/c_npc_combinegunship.cpp"
			"bmod_hud/c_npc_hydra.cpp"
			"bmod_hud/c_npc_manhack.cpp"
			"bmod_hud/c_particle_storm.cpp"
			"bmod_hud/c_particle_storm.h"
			"bmod_hud/c_plasma_beam_node.cpp"
			"bmod_hud/C_Point_Camera.cpp"
			"bmod_hud/C_Point_Camera.h"
			"bmod_hud/c_rotorwash.cpp"
			"bmod_hud/c_spotlight_end.cpp"
			"bmod_hud/c_strider.cpp"
			"bmod_hud/c_te_concussiveexplosion.cpp"
			"bmod_hud/c_te_flare.cpp"
			"bmod_hud/c_vehicle_crane.cpp"
			"bmod_dll/c_vehicle_jeep.cpp"
			"bmod_hud/C_WaterLODControl.cpp"
			"bmod_hud/c_weapon_gravitygun.cpp"
			"bmod_hud/c_weapon_sticklauncher.cpp"
			"bmod_hud/c_weapon_tool.cpp"
			"bmod_hud/c_weapon__stubs_hl2.cpp"
			"bmod_hud/c_physics_thruster.cpp"
			"bmod_hud/c_physics_thruster.h"
			"bmod_hud/energy_wave_effect.cpp"
			"bmod_hud/energy_wave_effect.h"
			"bmod_hud/fx_antlion.cpp"
			"bmod_hud/fx_bugbait.cpp"
			"bmod_hud/fx_hl2_tracers.cpp"
			"bmod_hud/gmod_color.cpp"
			"bmod_hud/gmod_color.h"
			"bmod_hud/gmod_client_cvars.cpp"
			"bmod_hud/bmod_render_extensions.cpp"
			"bmod_hud/bmod_render_extensions.h"
			"bmod_hud/bmod_material_extensions.cpp"
			"bmod_hud/bmod_material_extensions.h"
			"bmod_hud/derma_manager.cpp"
			"bmod_hud/derma_manager.h"
			"bmod_hud/gmod_client_init.cpp"
			"bmod_hud/gmod_postprocess.cpp"
			"bmod_hud/gmod_postprocess.h"
			"bmod_hud/gmod_physgun_fx.cpp"
			"bmod_hud/bmod_vguiscreens.cpp"
			"bmod_hud/gmod_usermessages.cpp"
			"bmod_hud/gmod_spawnmenu_net.cpp"
			"bmod_hud/gmod_menus.cpp"
			"bmod_hud/gmod_menus.h"
			"bmod_hud/gmod_commandmenu.cpp"
			"bmod_hud/gmod_commandmenu.h"
			"bmod_hud/gmod_facepose.cpp"
			"bmod_hud/gmod_facepose.h"
			"bmod_hud/gmod_message.cpp"
			"bmod_hud/gmod_message.h"
			"bmod_hud/gmod_spawnlist.cpp"
			"bmod_hud/gmod_spawnlist.h"
			"bmod_hud/gmod_teammenu.cpp"
			"bmod_hud/gmod_teammenu.h"
			"bmod_hud/hl2_clientmode.cpp"
			"bmod_hud/hl_in_main.cpp"
			"bmod_hud/hl_prediction.cpp"
			"bmod_hud/hud_ammo.cpp"
			"bmod_hud/hud_battery.cpp"
			"bmod_hud/hud_blood.cpp"
			"bmod_hud/hud_flashlight.cpp"
			"bmod_hud/hud_health.cpp"
			"bmod_hud/hud_quickinfo.cpp"
			"bmod_dll/hud_damageindicator.cpp"
			"bmod_hud/hud_suitpower.cpp"
			"${SRCDIR}/game_shared/hl2/hl2_gamerules.cpp"
			# "${SRCDIR}/game_shared/bmod/bmod_usermessages.cpp" # Commented out due to multiple definition conflicts with usermessages.cpp
			"${SRCDIR}/game_shared/hl_gamemovement.cpp"
			"bmod_hud/hud_suitpower.h"
			"bmod_hud/hud_weaponselection.cpp"
			"bmod_hud/hud_zoom.cpp"
			"bmod_hud/shieldproxy.cpp"
			"bmod_hud/vgui_rootpanel_hl2.cpp"
			"bmod_hud/WaterLODMaterialProxy.cpp"
		#}
	)
END_SRC( CLIENT_BMOD_SOURCE_FILES "Source Files" )

set(
	CLIENT_BMOD_EXCLUDE_SOURCES
)

add_object(
	TARGET client_bmod
	MODULE
	INSTALL_OUTNAME "client"
	INSTALL_DEST "${GAMEDIR}/bmod/bin"
	SOURCES ${CLIENT_BMOD_SOURCE_FILES}
)

target_include_directories(
	client_bmod PRIVATE

	"${SRCDIR}/game_shared/hl2"
	"${SRCDIR}/cl_dll/hl2_hud"
	"${SRCDIR}/cl_dll/bmod_hud"
	"${SRCDIR}/dlls/bmod_dll"
)

target_compile_definitions(
	client_bmod PRIVATE

	HL2_CLIENT_DLL
	BMOD_CLIENT_DLL
)

target_use_client_base( client_bmod CLIENT_BMOD_EXCLUDE_SOURCES )
