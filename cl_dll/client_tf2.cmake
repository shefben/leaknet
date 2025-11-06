include_guard( GLOBAL )

include( "${CMAKE_CURRENT_LIST_DIR}/client_base.cmake" )

set( CLIENT_TF2_SOURCE_FILES )
BEGIN_SRC( CLIENT_TF2_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SUBGROUP "HL2 DLL"
		SOURCES
		#{
			"hl2_hud/fx_bugbait.cpp" # !CSTRIKE
			"hl2_hud/hud_blood.cpp" # !CSTRIKE && !HL1
			"hl2_hud/hud_flashlight.cpp" # !CSTRIKE
		#}
	)

	SRC_GRP(
		SUBGROUP "TF2 DLL"
		SOURCES
		#{
			"${SRCDIR}/game_shared/env_meteor_shared.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/env_meteor_shared.h" # !CSTRIKE && !HL1 && !HL2

			"${SRCDIR}/dlls/tf2_dll/c_obj_armor_upgrade.cpp" # !CSTRIKE && !HL1 && !HL2

			"${SRCDIR}/game_shared/tf2/tfclassdata_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/baseobject_shared.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/baseobject_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/basetfcombatweapon_shared.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/basetfcombatweapon_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/basetfplayer_shared.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/basetfvehicle.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/basetfvehicle.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/env_laserdesignation.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/env_laserdesignation.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/gasoline_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/grenade_base_empable.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/grenade_base_empable.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/grenade_emp.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/grenade_emp.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/plasmaprojectile.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/plasmaprojectile.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/plasmaprojectile_shared.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/plasmaprojectile_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/techtree.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/techtree.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/techtree_parse.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamerules.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamerules.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_hints.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_base_manned_gun.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_base_manned_gun.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_basedrivergun_shared.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_basedrivergun_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_baseupgrade_shared.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_baseupgrade_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_driver_machinegun_shared.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_driver_machinegun_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_manned_plasmagun.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_manned_plasmagun_shared.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_shareddefs.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_shield_mobile_shared.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_usermessages.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tfclassdata_shared.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/vehicle_mortar_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_arcwelder.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_basecombatobject.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_basecombatobject.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_basegrenade.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_basegrenade.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_burstrifle.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_grenade.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_grenade_emp.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_laserrifle.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_plasma_grenade_launcher.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_plasmarifle.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_shotgun.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_usedwithshieldbase.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_usedwithshieldbase.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combatshield.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combatshield.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_drainbeam.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_drainbeam.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_flame_thrower.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_flame_thrower.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_gas_can.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_gas_can.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_grenade_rocket.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_grenade_rocket.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_harpoon.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_limpetmine.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_minigun.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_obj_rallyflag.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_objectselection.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_repairgun.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_repairgun.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_rocketlauncher.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_rocketlauncher.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_shield.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_shield.h"
			"${SRCDIR}/game_shared/tf2/weapon_shieldgrenade.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_twohandedcontainer.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_twohandedcontainer.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_gamemovement.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_gamemovement.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_movedata.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_reconvars.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_reconvars.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_shareddefs.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_shieldshared.cpp" # !CSTRIKE && !HL1 && !HL2 # Noprecomp?
			"${SRCDIR}/game_shared/tf_shieldshared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_tacticalmap.cpp" # !CSTRIKE && !HL1 && !HL2 # Noprecomp?
			"${SRCDIR}/game_shared/tf_vehicleshared.h" # !CSTRIKE && !HL1 && !HL2

			"TeamBitmapImage.cpp" # !CSTRIKE && !HL1 && !HL2
			"thermalmaterialproxy.cpp" # !CSTRIKE && !HL1 && !HL2

			"tf2_hud/commanderoverlay.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/commanderoverlay.h"
			"tf2_hud/infiltratorcamomaterialproxy.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/ObjectBuildAlphaProxy.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/ObjectControlPanel.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/ObjectControlPanel.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/paneleffect.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/ProxyTFPlayer.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/RespawnWaveVGuiScreen.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/teammaterialproxy.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/VGuiScreenVehicleBay.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_basecombatcharacter_tf2.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_basefourwheelvehicle.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_basefourwheelvehicle.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_baseobject.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_baseobject.h"
			"tf2_hud/c_basetfplayer.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_basetfplayer.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_controlzone.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_controlzone.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_demo_entities.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_demo_entities.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_effect_shootingstar.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_effect_shootingstar.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_entity_burn_effect.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_entity_burn_effect.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_env_meteor.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_env_meteor.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_func_construction_yard.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_func_resource.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_func_resource.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_gasoline_blob.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_gasoline_blob.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_grenade_antipersonnel.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_grenade_limpetmine.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_grenade_objectsapper.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_grenade_rocket.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_harpoon.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_hint_events.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_hint_events.h"
			"tf2_hud/c_info_act.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_info_act.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_info_customtech.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_maker_bughole.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_barbed_wire.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_barbed_wire.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_base_manned_gun.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_base_manned_gun.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_buff_station.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_bunker.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_dragonsteeth.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_empgenerator.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_explosives.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_manned_missilelauncher.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_manned_shield.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_mapdefined.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_mapdefined.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_mcv_selection_panel.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_mortar.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_powerpack.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_rallyflag.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_resourcepump.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_resourcepump.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_respawn_station.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_respawn_station.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_resupply.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_sandbag_bunker.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_selfheal.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_sentrygun.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_shieldwall.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_tower.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_tunnel.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_obj_vehicleboost.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_objectsentrygun.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_assist.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_assist.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_buildsentrygun.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_buildsentrygun.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_buildshieldwall.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_buildshieldwall.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_heal.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_heal.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_killmortarguy.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_killmortarguy.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_mortar_attack.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_mortar_attack.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_player.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_player.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_repair.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_repair.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_resourcepump.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_resourcepump.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_respawnstation.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_respawnstation.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_order_resupply.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_ragdoll_shadow.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_resource_chunk.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_shield.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_shield.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_shield_flat.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf2rootpanel.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf2rootpanel.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_basecombatweapon.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_basecombatweapon.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_basehint.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_basehint.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_flare.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_hintmanager.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_hintmanager.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_hints.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_hints.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_playerclass.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_playerclass.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tfplayerlocaldata.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tfplayerresource.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tfplayerresource.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tfteam.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tfteam.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_vehicle_battering_ram.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_vehicle_flatbed.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_vehicle_mortar.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_vehicle_motorcycle.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_vehicle_siege_tower.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_vehicle_tank.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_vehicle_tank.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_vehicle_teleport_station.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_vehicle_teleport_station.h"
			"tf2_hud/c_vehicle_wagon.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_walker_base.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_walker_base.h"
			"tf2_hud/c_walker_ministrider.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_walker_ministrider.h"
			"tf2_hud/c_walker_strider.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_walker_strider.h"
			"tf2_hud/c_weapon__stubs_tf2.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_weapon_builder.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_weapon_builder.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_weapon_twohandedcontainer.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/clientmode_commander.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/clientmode_commander.h"
			"tf2_hud/clientmode_tfbase.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/clientmode_tfbase.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/clientmode_tfnormal.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/clientmode_tfnormal.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/commanderoverlaypanel.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/commanderoverlaypanel.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/env_objecteffects.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/env_objecteffects.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/fx_tf2_blood.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/fx_tf2_buildeffects.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/fx_tf2_impacts.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/fx_tf2_tracers.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/ground_line.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/ground_line.h"
			"tf2_hud/hintitembase.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hintitembase.h"
			"tf2_hud/hintitemobjectbase.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hintitemobjectbase.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hintitemorderbase.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hintitemorderbase.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_ammo.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_ammo.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_commander_statuspanel.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_commander_statuspanel.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_damageindicator.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_deathnotice.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_emp.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_health.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_minimap.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_minimap.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_numeric.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_numeric.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_orders.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_orders.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_resources.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_target_id.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_targetreticle.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_targetreticle.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_technologytreedoc.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_technologytreedoc.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_timer.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_timer.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_vehicle_role.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_vehicle_role.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/hud_weaponselection.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/itfhintitem.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/iusesmortarpanel.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/mapdata.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/mapdata.h"
			"tf2_hud/minimap_players.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/minimap_resourcezone.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/minimap_trace.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/minimap_trace.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/overlay_orders.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/panel_effects.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/paneleffect.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playerandobjectenumerator.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playerandobjectenumerator.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlay.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlay.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlayclass.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlayclass.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlayhealth.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlayhealth.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlayname.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlayname.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlayselected.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlayselected.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlaysquad.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/playeroverlaysquad.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/proxy_shield.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/proxy_sunroof.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/resourcezoneoverlay.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/tf_clientmode.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/tf_in_main.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/tf_prediction.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/vgui_healthbar.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/vgui_rootpanel_tf2.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/vgui_rotation_slider.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/vgui_rotation_slider.h" # !CSTRIKE && !HL1 && !HL2

			"tf2_hud/maxplayers.h"

			"vgui_bitmappanel.cpp" # !CSTRIKE && !HL1 && !HL2

			"vgui_entityimagepanel.cpp" # !CSTRIKE && !HL1 && !HL2
			"vgui_entitypanel.cpp" # !CSTRIKE && !HL1 && !HL2

			"vgui_imagehealthpanel.cpp" # !CSTRIKE && !HL1 && !HL2

			"${SRCDIR}/game_shared/basecombatweapon_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/basetfplayer_shared.h" # !CSTRIKE && !HL1 && !HL2
		#}
	)

	SRC_GRP(
		SUBGROUP "TF2 DLL//TF2 Classes"
		SOURCES
		#{
			"tf2_hud/c_tf_class_commando.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_commando.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_defender.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_defender.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_escort.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_escort.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_infiltrator.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_infiltrator.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_medic.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_medic.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_pyro.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_pyro.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_recon.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_recon.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_sapper.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_sapper.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_sniper.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_sniper.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_support.cpp" # !CSTRIKE && !HL1 && !HL2
			"tf2_hud/c_tf_class_support.h" # !CSTRIKE && !HL1 && !HL2
		#}
	)

	SRC_GRP(
		SUBGROUP "TF2 DLL//TF2 Movement"
		SOURCES
		#{
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_chooser.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_chooser.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_commando.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_commando.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_defender.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_defender.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_escort.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_escort.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_infiltrator.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_infiltrator.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_medic.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_medic.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_pyro.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_pyro.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_recon.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_recon.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_sapper.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_sapper.h"
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_sniper.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_sniper.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_support.cpp" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_support.h" # !CSTRIKE && !HL1 && !HL2
		#}
	)
END_SRC( CLIENT_TF2_SOURCE_FILES "Source Files" )

set(
	CLIENT_TF2_EXCLUDE_SOURCES

	"BaseModViewport.cpp"
	"BaseModViewport.h"
	"death.cpp"
	"hud_numericdisplay.cpp"
	"hud_numericdisplay.h"
)

add_object(
	TARGET client_tf2
	MODULE
	INSTALL_DEST "${GAMEDIR}/tf2/bin"
	INSTALL_OUTNAME "client"
	SOURCES ${CLIENT_TF2_SOURCE_FILES}
)

target_include_directories(
	client_tf2 PRIVATE

	"tf2_hud"
	"${SRCDIR}/statemachine"
	"${SRCDIR}/game_shared/tf2"
)

target_compile_definitions(
	client_tf2 PRIVATE

	TF2_CLIENT_DLL
)

target_use_client_base( client_tf2 CLIENT_TF2_EXCLUDE_SOURCES )
