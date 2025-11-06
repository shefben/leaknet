include_guard( GLOBAL )

include( "${CMAKE_CURRENT_LIST_DIR}/server_base.cmake" )

set( SERVER_TF2_SOURCE_FILES )
BEGIN_SRC( SERVER_TF2_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SUBGROUP "TF2 DLL"
		SOURCES
		#{
			"${SRCDIR}/game_shared/env_meteor_shared.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/env_meteor_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/basetfplayer_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/mapdata_shared.h" # !CSTRIKE && !HL1 && !HL2

			"${SRCDIR}/game_shared/tf2/tfclassdata_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/baseobject_shared.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/baseobject_shared.h" # !CSTRIKE && !HL1
			"${SRCDIR}/game_shared/tf2/basetfcombatweapon_shared.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/basetfcombatweapon_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/basetfplayer_shared.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/basetfvehicle.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/basetfvehicle.h" # !CSTRIKE && !HL1
			"${SRCDIR}/game_shared/tf2/env_laserdesignation.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/env_laserdesignation.h" # !CSTRIKE && !HL1
			"${SRCDIR}/game_shared/tf2/gasoline_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/grenade_antipersonnel.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/grenade_antipersonnel.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/grenade_base_empable.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/grenade_base_empable.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/grenade_emp.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/grenade_emp.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/grenade_limpetmine.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/grenade_limpetmine.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/grenade_objectsapper.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/grenade_objectsapper.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/grenade_rocket.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/grenade_rocket.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/grenade_stickybomb.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/ihasbuildpoints.h" # !CSTRIKE && !HL1
			"${SRCDIR}/game_shared/tf2/plasmaprojectile.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/plasmaprojectile.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/plasmaprojectile_shared.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/plasmaprojectile_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/techtree.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/techtree.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/techtree_parse.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamerules.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamerules.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_base_manned_gun.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_obj_base_manned_gun.h" # !CSTRIKE && !HL1
			"${SRCDIR}/game_shared/tf2/tf_obj_basedrivergun_shared.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_obj_basedrivergun_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_baseupgrade_shared.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_obj_baseupgrade_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_driver_machinegun_shared.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_obj_driver_machinegun_shared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_manned_plasmagun.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_obj_manned_plasmagun.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_obj_manned_plasmagun_shared.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_obj_manned_plasmagun_shared.h" # !CSTRIKE && !HL1
			"${SRCDIR}/game_shared/tf2/tf_playeranimstate.h"
			"${SRCDIR}/game_shared/tf2/tf_shareddefs.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_shield_mobile_shared.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_usermessages.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tfclassdata_shared.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/vehicle_mortar_shared.h"
			"${SRCDIR}/game_shared/tf2/weapon_arcwelder.cpp" # Noprecomp?
			"${SRCDIR}/game_shared/tf2/weapon_basecombatobject.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_basecombatobject.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_builder.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_builder.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_basegrenade.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_combat_basegrenade.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combat_burstrifle.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_combat_grenade.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_combat_grenade_emp.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_combat_laserrifle.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_combat_plasma_grenade_launcher.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_combat_plasmarifle.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_combat_shotgun.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_combat_usedwithshieldbase.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_combat_usedwithshieldbase.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_combatshield.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_combatshield.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_drainbeam.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_drainbeam.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_flame_thrower.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_flame_thrower.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_gas_can.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_gas_can.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_grenade_rocket.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_grenade_rocket.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_harpoon.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_limpetmine.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_limpetmine.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_minigun.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_obj_empgenerator.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_obj_rallyflag.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_objectselection.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_objectselection.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_plasmarifle.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_repairgun.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_repairgun.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_rocketlauncher.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_rocketlauncher.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_shield.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_shield.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/weapon_shieldgrenade.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_twohandedcontainer.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/weapon_twohandedcontainer.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_gamemovement.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf_gamemovement.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_movedata.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_reconvars.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf_reconvars.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_shareddefs.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_shieldshared.cpp" # Noprecomp?
			"${SRCDIR}/game_shared/tf_shieldshared.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf_tacticalmap.cpp" # Noprecomp?
			"${SRCDIR}/game_shared/tf_vehicleshared.h" # !CSTRIKE && !HL1 && !HL2

			"tf2_dll/controlzone.h" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/basecombatcharacter_tf2.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/bot_base.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/bot_base.h"
			"tf2_dll/controlzone.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/demo_entities.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/entity_burn_effect.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/entity_burn_effect.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/env_fallingrocks.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/env_meteor.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/env_meteor.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/fire_damage_mgr.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/fire_damage_mgr.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/gasoline_blob.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/gasoline_blob.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/info_act.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/info_act.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/info_add_resources.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/info_buildpoint.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/info_buildpoint.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/info_customtech.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/info_customtech.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/info_input_playsound.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/info_input_resetbanks.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/info_input_resetobjects.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/info_input_respawnplayers.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/info_minimappulse.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/info_output_team.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/info_vehicle_bay.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/info_vehicle_bay.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/mapdata_server.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/menu_base.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/menu_base.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/mortar_round.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/mortar_round.h"
			"tf2_dll/npc_bug_builder.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/npc_bug_builder.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/npc_bug_hole.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/npc_bug_hole.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/npc_bug_warrior.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/npc_bug_warrior.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_assist.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_assist.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_buildsentrygun.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_buildsentrygun.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_buildshieldwall.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_buildshieldwall.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_events.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_events.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_heal.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_heal.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_helpers.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_helpers.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_killmortarguy.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_killmortarguy.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_mortar_attack.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_mortar_attack.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_player.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_player.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_repair.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_repair.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_resourcepump.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_resourcepump.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/order_resupply.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/order_resupply.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/orders.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/orders.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/ragdoll_shadow.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/ragdoll_shadow.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/resource_chunk.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/resource_chunk.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/sensor_tf_team.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/team_messages.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/team_messages.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf2_eventlog.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_ai_hint.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_basecombatweapon.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_basecombatweapon.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_basefourwheelvehicle.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_basefourwheelvehicle.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_client.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_filters.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_flare.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_flare.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_func_construction_yard.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_func_construction_yard.h" # !CSTRIKE && !HL1
			"tf2_dll/tf_func_mass_teleport.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_func_mass_teleport.h" # !CSTRIKE && !HL1
			"tf2_dll/tf_func_no_build.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_func_no_build.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_func_resource.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_func_resource.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_func_weldable_door.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_func_weldable_door.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_hintmanager.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_hintmanager.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_armor_upgrade.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_armor_upgrade.h" # !CSTRIKE && !HL1
			"tf2_dll/tf_obj_barbed_wire.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_barbed_wire.h" # !CSTRIKE && !HL1
			"tf2_dll/tf_obj_buff_station.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_buff_station.h" # !CSTRIKE && !HL1
			"tf2_dll/tf_obj_bunker.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_bunker.h" # !CSTRIKE && !HL1
			"tf2_dll/tf_obj_dragonsteeth.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_dragonsteeth.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_empgenerator.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_empgenerator.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_explosives.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_manned_missilelauncher.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_manned_missilelauncher.h" # !CSTRIKE && !HL1
			"tf2_dll/tf_obj_manned_shield.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_mapdefined.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_mapdefined.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_mcv_selection_panel.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_mcv_selection_panel.h"
			"tf2_dll/tf_obj_mortar.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_mortar.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_powerpack.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_powerpack.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_rallyflag.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_rallyflag.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_resourcepump.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_resourcepump.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_respawn_station.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_respawn_station.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_resupply.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_resupply.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_sandbag_bunker.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_sandbag_bunker.h" # !CSTRIKE && !HL1
			"tf2_dll/tf_obj_selfheal.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_selfheal.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_sentrygun.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_sentrygun.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_shieldwall.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_shieldwall.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_tower.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_tower.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_obj_tunnel.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_vehicleboost.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_obj_vehicleboost.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_player.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_player.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_player_death.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_player_resource.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_player_resource.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_playerclass.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_playerclass.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_playerlocaldata.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_playerlocaldata.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_playermove.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_shield.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_shield.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_shield_flat.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_shield_flat.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_shieldgrenade.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_shieldgrenade.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_stats.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_stats.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_stressentities.cpp" # TF2DEBUG
			"tf2_dll/tf_team.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_team.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_teamspawnpoint.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_vehicle_battering_ram.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_vehicle_battering_ram.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_vehicle_flatbed.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_vehicle_flatbed.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_vehicle_mortar.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_vehicle_mortar.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_vehicle_motorcycle.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_vehicle_siege_tower.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_vehicle_siege_tower.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_vehicle_tank.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_vehicle_tank.h" # !CSTRIKE && !HL1
			"tf2_dll/tf_vehicle_teleport_station.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_vehicle_teleport_station.h" # !CSTRIKE && !HL1
			"tf2_dll/tf_vehicle_wagon.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_vehicle_wagon.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_walker_base.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_walker_base.h"
			"tf2_dll/tf_walker_ministrider.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_walker_ministrider.h"
			"tf2_dll/tf_walker_strider.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_walker_strider.h"
			"tf2_dll/trigger_fall.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/trigger_skybox.cpp" # !CSTRIKE && !HL2 && !HL1

			"iscorer.h" # !CSTRIKE && !HL1 && !HL2
			"monstermaker.h" # !CSTRIKE && !HL1 && !HL2
			"spark.h" # !CSTRIKE && !HL1 && !HL2
		#}
	)

	SRC_GRP(
		SUBGROUP "TF2 DLL//TF2 Classes"
		SOURCES
		#{
			"tf2_dll/tf_class_commando.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_class_commando.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_class_defender.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_class_defender.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_class_escort.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_class_escort.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_class_infiltrator.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_class_infiltrator.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_class_medic.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_class_medic.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_class_pyro.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_class_pyro.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_class_recon.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_class_recon.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_class_sapper.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_class_sapper.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_class_sniper.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_class_sniper.h" # !CSTRIKE && !HL1 && !HL2
			"tf2_dll/tf_class_support.cpp" # !CSTRIKE && !HL2 && !HL1
			"tf2_dll/tf_class_support.h" # !CSTRIKE && !HL1 && !HL2
		#}
	)

	SRC_GRP(
		SUBGROUP "TF2 DLL//TF2 Movement"
		SOURCES
		#{
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_chooser.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_chooser.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_commando.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_commando.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_defender.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_defender.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_escort.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_escort.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_infiltrator.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_infiltrator.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_medic.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_medic.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_pyro.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_pyro.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_recon.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_recon.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_sapper.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_sapper.h" # !CSTRIKE && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_sniper.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_sniper.h" # !CSTRIKE && !HL1 && !HL2
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_support.cpp" # !CSTRIKE && !HL2 && !HL1
			"${SRCDIR}/game_shared/tf2/tf_gamemovement_support.h" # !CSTRIKE && !HL1 && !HL2
		#}
	)
END_SRC( SERVER_TF2_SOURCE_FILES "Source Files" )

set(
	SERVER_TF2_EXCLUDE_SOURCES

	"cterrainmorph.cpp" # !TF2
	"physgun.cpp" # !CSTRIKE && !TF2
	"ai_squad.h" # !TF2
	"vehicle_base.h" # !CSTRIKE && !TF2 && !HL1

	"hl2_dll/antlion_dust.cpp" # !CSTRIKE && !TF2
	"hl2_dll/assassin_smoke.cpp" # !CSTRIKE && !TF2
	"hl2_dll/assassin_smoke.h" # !TF2
	"hl2_dll/rotorwash.cpp" # !CSTRIKE && !TF2
	"hl2_dll/rotorwash.h" # !CSTRIKE && !TF2

	"${SRCDIR}/game_shared/hl2_shareddefs.h" # !CSTRIKE && !TF2
	"${SRCDIR}/game_shared/hl_movedata.h" # !CSTRIKE && !TF2
)

add_object(
	TARGET server_tf2
	MODULE
	INSTALL_OUTNAME "server"
	INSTALL_DEST "${GAMEDIR}/tf2/bin"
	SOURCES ${SERVER_TF2_SOURCE_FILES}
)

target_include_directories(
	server_tf2 PRIVATE

	"${SRCDIR}/game_shared/tf2"
	"${SRCDIR}/dlls/tf2_dll"
)

target_compile_definitions(
	server_tf2 PRIVATE

	TF2_DLL
)

target_use_server_base( server_tf2 SERVER_TF2_EXCLUDE_SOURCES )