include_guard( GLOBAL )

include( "${CMAKE_CURRENT_LIST_DIR}/client_base.cmake" )

set( CLIENT_CSTRIKE_SOURCE_FILES )
BEGIN_SRC( CLIENT_CSTRIKE_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SUBGROUP "HL2 DLL"
		SOURCES
		#{
			"hl2_hud/fx_antlion.cpp"
			"hl2_hud/c_basehelicopter.h"
			"hl2_hud/c_basehlcombatweapon.h"
			"hl2_hud/clientmode_hlnormal.h"
		#}
	)

	SRC_GRP(
		SUBGROUP "CounterStrike DLL"
		SOURCES
		#{
			"${SRCDIR}/game_shared/cstrike/cs_gamemovement.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/cs_gamerules.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/cs_gamerules.h" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/cs_playeranimstate.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/cs_playeranimstate.h" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/cs_player_shared.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/cs_shareddefs.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/cs_usermessages.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/cs_weapon_parse.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/cs_weapon_parse.h" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_ak47.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_aug.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_awp.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_basecsgrenade.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_basecsgrenade.h" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_c4.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_c4.h" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_csbase.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_csbase.h" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_csbasegun.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_csbasegun.h" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_deagle.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_famas.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_fiveseven.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_flashbang.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_g3sg1.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_galil.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_glock.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_hegrenade.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_knife.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_knife.h" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_m249.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_m3.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_m4a1.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_mac10.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_mp5navy.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_p228.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_p90.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_scout.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_sg550.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_sg552.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_ump45.cpp" # !TF2 && !HL1 && !HL2
			"${SRCDIR}/game_shared/cstrike/weapon_usp.cpp" # !TF2 && !HL1 && !HL2

			"cstrike/clientmode_csnormal.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/clientmode_csnormal.h" # !TF2 && !HL1 && !HL2
			"cstrike/cs_hud_ammo.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/cs_hud_damageindicator.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/cs_hud_health.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/cs_hud_weaponselection.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/cs_in_main.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/cs_prediction.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/c_csrootpanel.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/c_csrootpanel.h" # !TF2 && !HL1 && !HL2
			"cstrike/c_cs_player.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/c_cs_player.h" # !TF2 && !HL1 && !HL2
			"cstrike/c_cs_team.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/c_cs_team.h" # !TF2 && !HL1 && !HL2
			"cstrike/fx_cs_impacts.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/hud_account.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/hud_armor.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/hud_c4.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/hud_progressbar.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/hud_roundtimer.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/hud_shopping_cart.cpp" # !TF2 && !HL1 && !HL2
		#}
	)

	SRC_GRP(
		SUBGROUP "CounterStrike DLL//vgui"
		SOURCES
		#{
			"cstrike/VGUI/buymenu.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/buymenu.h" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/buysubmenu.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/buysubmenu.h" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/counterstrikeviewport.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/counterstrikeviewport.h" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/counterstrikeviewport_interface.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/cstrikeclassmenu.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/cstrikeclassmenu.h" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/cstrikeclientscoreboard.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/cstrikeclientscoreboard.h" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/cstrikespectatorgui.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/cstrikespectatorgui.h" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/cstriketeammenu.cpp" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/cstriketeammenu.h" # !TF2 && !HL1 && !HL2
			"cstrike/vgui_rootpanel_cs.cpp" # !TF2 && !HL1 && !HL2
			
			"cstrike/mouseoverpanelbutton.h" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/buymouseoverhtmlbutton.h" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/buymouseoverpanelbutton.h" # !TF2 && !HL1 && !HL2
			#"cstrike/VGUI/icstrikeviewport.h" # !TF2 && !HL1 && !HL2
			"cstrike/VGUI/icstrikeviewportmsgs.h" # !TF2 && !HL1 && !HL2
		#}
	)

	SRC_GRP(
		SUBGROUP "HL2 DLL"
		SOURCES
		#{
			"hl2_hud/fx_antlion.cpp" # !TF2
		#}
	)
END_SRC( CLIENT_CSTRIKE_SOURCE_FILES "Source Files" )

set(
	CLIENT_CSTRIKE_EXCLUDE_SOURCES

	"${SRCDIR}/game_shared/hl2/basehlcombatweapon_shared.cpp"
	"${SRCDIR}/game_shared/weapon_parse_default.cpp"
	"death.cpp"
)

add_object(
	TARGET client_cstrike
	MODULE
	INSTALL_DEST "${GAMEDIR}/cstrike/bin"
	INSTALL_OUTNAME "client"
	SOURCES ${CLIENT_CSTRIKE_SOURCE_FILES}
)

target_include_directories(
	client_cstrike PRIVATE

	"cstrike"
	"cstrike/VGUI"
	"${SRCDIR}/game_shared/cstrike"
)

target_compile_definitions(
	client_cstrike PRIVATE

	CSTRIKE_DLL
)

target_use_client_base( client_cstrike CLIENT_CSTRIKE_EXCLUDE_SOURCES )
