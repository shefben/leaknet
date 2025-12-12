//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Centralized skill system ConVars for NPC health, damage values,
//          weapon damage, and item properties.
//
//=============================================================================//

#include "cbase.h"
#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// NPC HEALTH AND DAMAGE CVARS
//=============================================================================

// Barnacle
ConVar sk_barnacle_health( "sk_barnacle_health", "35" );

// Barney
ConVar sk_barney_health( "sk_barney_health", "35" );

// Bullseye
ConVar sk_bullseye_health( "sk_bullseye_health", "35" );

// Bullsquid
ConVar sk_bullsquid_health( "sk_bullsquid_health", "120" );
ConVar sk_bullsquid_dmg_bite( "sk_bullsquid_dmg_bite", "15" );
ConVar sk_bullsquid_dmg_whip( "sk_bullsquid_dmg_whip", "25" );

// Citizen
ConVar sk_citizen_health( "sk_citizen_health", "40" );

// Conscript
ConVar sk_conscript_health( "sk_conscript_health", "50" );

// Cremator
ConVar sk_Cremator_health( "sk_Cremator_health", "100" );

// Combine Soldier
ConVar sk_combine_s_health( "sk_combine_s_health", "50" );
ConVar sk_combine_s_kick( "sk_combine_s_kick", "10" );

// Combine Elite
ConVar sk_combine_elite_health( "sk_combine_elite_health", "70" );

// Combine Guard
ConVar sk_combine_guard_health( "sk_combine_guard_health", "70" );
ConVar sk_combine_guard_kick( "sk_combine_guard_kick", "15" );
ConVar sk_combineguard_health( "sk_combineguard_health", "200" );

// Strider
ConVar sk_strider_health( "sk_strider_health", "350" );
ConVar sk_strider_num_missiles1( "sk_strider_num_missiles1", "5" );
ConVar sk_strider_num_missiles2( "sk_strider_num_missiles2", "7" );
ConVar sk_strider_num_missiles3( "sk_strider_num_missiles3", "7" );

// Headcrab
ConVar sk_headcrab_health( "sk_headcrab_health", "10" );
ConVar sk_headcrab_melee_dmg( "sk_headcrab_melee_dmg", "5" );

// Fast Headcrab
ConVar sk_headcrab_fast_health( "sk_headcrab_fast_health", "10" );

// Poison Headcrab
ConVar sk_headcrab_poison_health( "sk_headcrab_poison_health", "35" );

// Houndeye
ConVar sk_houndeye_health( "sk_houndeye_health", "80" );
ConVar sk_houndeye_dmg_blast( "sk_houndeye_dmg_blast", "25" );

// Manhack
ConVar sk_manhack_health( "sk_manhack_health", "25" );
ConVar sk_manhack_melee_dmg( "sk_manhack_melee_dmg", "20" );

// Metropolice
ConVar sk_metropolice_health( "sk_metropolice_health", "40" );
ConVar sk_metropolice_stitch_reaction( "sk_metropolice_stitch_reaction", "1.0" );
ConVar sk_metropolice_stitch_tight_hitcount( "sk_metropolice_stitch_tight_hitcount", "2" );
ConVar sk_metropolice_stitch_at_hitcount( "sk_metropolice_stitch_at_hitcount", "1" );
ConVar sk_metropolice_stitch_behind_hitcount( "sk_metropolice_stitch_behind_hitcount", "3" );
ConVar sk_metropolice_stitch_along_hitcount( "sk_metropolice_stitch_along_hitcount", "2" );

// Mortar Synth
ConVar sk_mortarsynth_health( "sk_mortarsynth_health", "100" );

// Rollermine
ConVar sk_rollermine_shock( "sk_rollermine_shock", "10" );
ConVar sk_rollermine_stun_delay( "sk_rollermine_stun_delay", "3" );
ConVar sk_rollermine_vehicle_intercept( "sk_rollermine_vehicle_intercept", "1" );

// Scanner (City Scanner)
ConVar sk_scanner_health( "sk_scanner_health", "30" );
ConVar sk_scanner_dmg_dive( "sk_scanner_dmg_dive", "25" );
ConVar sk_scanner_dmg_gas( "sk_scanner_dmg_gas", "10" );

// Shield Scanner
ConVar sk_sscanner_health( "sk_sscanner_health", "50" );

// Stalker
ConVar sk_stalker_health( "sk_stalker_health", "50" );
ConVar sk_stalker_melee_dmg( "sk_stalker_melee_dmg", "5" );

// Vortigaunt
ConVar sk_vortigaunt_health( "sk_vortigaunt_health", "100" );
ConVar sk_vortigaunt_dmg_claw( "sk_vortigaunt_dmg_claw", "10" );
ConVar sk_vortigaunt_dmg_rake( "sk_vortigaunt_dmg_rake", "25" );
ConVar sk_vortigaunt_dmg_zap( "sk_vortigaunt_dmg_zap", "50" );
ConVar sk_vortigaunt_armor_charge( "sk_vortigaunt_armor_charge", "30" );

// Waste Scanner
ConVar sk_wscanner_health( "sk_wscanner_health", "50" );
ConVar sk_wscanner_laser_dmg( "sk_wscanner_laser_dmg", "10" );

// Zombie (regular zombie defined elsewhere but adding here for completeness)
// ConVar sk_zombie_health - defined in hl1_npc_zombie.cpp
// ConVar sk_zombie_dmg_one_slash - defined in hl1_npc_zombie.cpp
// ConVar sk_zombie_dmg_both_slash - defined in hl1_npc_zombie.cpp

// Poison Zombie
ConVar sk_zombie_poison_health( "sk_zombie_poison_health", "175" );
ConVar sk_zombie_poison_dmg_spit( "sk_zombie_poison_dmg_spit", "20" );

// Antlion
ConVar sk_antlion_health( "sk_antlion_health", "30" );
ConVar sk_antlion_swipe_damage( "sk_antlion_swipe_damage", "5" );
ConVar sk_antlion_jump_damage( "sk_antlion_jump_damage", "5" );

// Antlion Guard
ConVar sk_antlionguard_health( "sk_antlionguard_health", "500" );
ConVar sk_antlionguard_dmg_charge( "sk_antlionguard_dmg_charge", "20" );
ConVar sk_antlionguard_dmg_shove( "sk_antlionguard_dmg_shove", "10" );

// Antlion Grub
ConVar sk_antliongrub_health( "sk_antliongrub_health", "5" );

// Ichthyosaur
ConVar sk_ichthyosaur_health( "sk_ichthyosaur_health", "200" );
ConVar sk_ichthyosaur_melee_dmg( "sk_ichthyosaur_melee_dmg", "8" );

// Combine Gunship
ConVar sk_gunship_burst_size( "sk_gunship_burst_size", "15" );
ConVar sk_gunship_burst_miss( "sk_gunship_burst_miss", "0" );
ConVar sk_gunship_health_increments( "sk_gunship_health_increments", "5" );
ConVar sk_npc_dmg_gunship( "sk_npc_dmg_gunship", "40" );
ConVar sk_npc_dmg_gunship_to_plr( "sk_npc_dmg_gunship_to_plr", "3" );

// Combine Helicopter
ConVar sk_npc_dmg_helicopter( "sk_npc_dmg_helicopter", "6" );
ConVar sk_npc_dmg_helicopter_to_plr( "sk_npc_dmg_helicopter_to_plr", "3" );
ConVar sk_helicopter_grenadedamage( "sk_helicopter_grenadedamage", "30" );
ConVar sk_helicopter_grenaderadius( "sk_helicopter_grenaderadius", "275" );
ConVar sk_helicopter_grenadeforce( "sk_helicopter_grenadeforce", "55000" );

// Combine Dropship
ConVar sk_npc_dmg_dropship( "sk_npc_dmg_dropship", "2" );

// Combine APC
ConVar sk_apc_health( "sk_apc_health", "750" );

// Hydra
ConVar sk_hydra_health( "sk_hydra_health", "100" );
ConVar sk_hydra_stab_damage( "sk_hydra_stab_damage", "20" );

// Crow
ConVar sk_crow_health( "sk_crow_health", "5" );
ConVar sk_crow_melee_dmg( "sk_crow_melee_dmg", "0" );

//=============================================================================
// WEAPON DAMAGE CVARS
//=============================================================================

// AR2
ConVar sk_plr_dmg_ar2( "sk_plr_dmg_ar2", "8" );
ConVar sk_npc_dmg_ar2( "sk_npc_dmg_ar2", "3" );
ConVar sk_max_ar2( "sk_max_ar2", "60" );
ConVar sk_max_ar2_altfire( "sk_max_ar2_altfire", "3" );

// AR2 Grenade - defined in hl2_gamerules.cpp
// sk_plr_dmg_ar2_grenade, sk_npc_dmg_ar2_grenade, sk_max_ar2_grenade
ConVar sk_ar2_grenade_radius( "sk_ar2_grenade_radius", "250" );

// Alyx Gun
ConVar sk_plr_dmg_alyxgun( "sk_plr_dmg_alyxgun", "5" );
ConVar sk_npc_dmg_alyxgun( "sk_npc_dmg_alyxgun", "3" );
ConVar sk_max_alyxgun( "sk_max_alyxgun", "150" );

// Pistol
ConVar sk_plr_dmg_pistol( "sk_plr_dmg_pistol", "5" );
ConVar sk_npc_dmg_pistol( "sk_npc_dmg_pistol", "3" );
ConVar sk_max_pistol( "sk_max_pistol", "150" );

// SMG1
ConVar sk_plr_dmg_smg1( "sk_plr_dmg_smg1", "4" );
ConVar sk_npc_dmg_smg1( "sk_npc_dmg_smg1", "3" );
ConVar sk_max_smg1( "sk_max_smg1", "225" );

// SMG1 Grenade
ConVar sk_plr_dmg_smg1_grenade( "sk_plr_dmg_smg1_grenade", "100" );
ConVar sk_npc_dmg_smg1_grenade( "sk_npc_dmg_smg1_grenade", "50" );
ConVar sk_max_smg1_grenade( "sk_max_smg1_grenade", "3" );
ConVar sk_smg1_grenade_radius( "sk_smg1_grenade_radius", "250" );

// Buckshot (Shotgun) - defined in hl2_gamerules.cpp
// sk_plr_dmg_buckshot, sk_npc_dmg_buckshot, sk_max_buckshot
ConVar sk_plr_num_shotgun_pellets( "sk_plr_num_shotgun_pellets", "7" );

// RPG
ConVar sk_plr_dmg_rpg_round( "sk_plr_dmg_rpg_round", "100" );
ConVar sk_npc_dmg_rpg_round( "sk_npc_dmg_rpg_round", "50" );
ConVar sk_max_rpg_round( "sk_max_rpg_round", "3" );

// Sniper - defined in hl2_gamerules.cpp
// sk_plr_dmg_sniper_round, sk_npc_dmg_sniper_round, sk_max_sniper_round
// sk_dmg_sniper_penetrate_plr, sk_dmg_sniper_penetrate_npc

// .357 Magnum
ConVar sk_plr_dmg_357( "sk_plr_dmg_357", "40" );
ConVar sk_npc_dmg_357( "sk_npc_dmg_357", "30" );
ConVar sk_max_357( "sk_max_357", "12" );

// Crossbow
ConVar sk_plr_dmg_crossbow( "sk_plr_dmg_crossbow", "100" );
ConVar sk_npc_dmg_crossbow( "sk_npc_dmg_crossbow", "10" );
ConVar sk_max_crossbow( "sk_max_crossbow", "10" );

// Airboat Gun
ConVar sk_plr_dmg_airboat( "sk_plr_dmg_airboat", "3" );
ConVar sk_npc_dmg_airboat( "sk_npc_dmg_airboat", "3" );

// Grenade (generic) - defined in hl2_gamerules.cpp
// sk_plr_dmg_grenade, sk_npc_dmg_grenade, sk_max_grenade

// Frag Grenade
ConVar sk_plr_dmg_fraggrenade( "sk_plr_dmg_fraggrenade", "150" );
ConVar sk_npc_dmg_fraggrenade( "sk_npc_dmg_fraggrenade", "75" );
ConVar sk_fraggrenade_radius( "sk_fraggrenade_radius", "250" );

// Tripmine
ConVar sk_plr_dmg_tripmine( "sk_plr_dmg_tripmine", "150" );
ConVar sk_npc_dmg_tripmine( "sk_npc_dmg_tripmine", "125" );
ConVar sk_tripmine_radius( "sk_tripmine_radius", "200" );

// Tripwire
ConVar sk_dmg_tripwire( "sk_dmg_tripwire", "150" );
ConVar sk_tripwire_radius( "sk_tripwire_radius", "200" );

// Energy Grenade (Mortar Synth)
ConVar sk_dmg_energy_grenade( "sk_dmg_energy_grenade", "2" );
ConVar sk_energy_grenade_radius( "sk_energy_grenade_radius", "100" );

// Spit Grenade (Bullsquid)
ConVar sk_dmg_spit_grenade( "sk_dmg_spit_grenade", "5" );
ConVar sk_spit_grenade_radius( "sk_spit_grenade_radius", "50" );

// Molotov - defined in hl2_gamerules.cpp
// sk_plr_dmg_molotov, sk_npc_dmg_molotov

// Brickbat - defined in hl2_gamerules.cpp
// sk_plr_dmg_brickbat, sk_npc_dmg_brickbat

// Jeep Gauss
ConVar sk_max_gauss_round( "sk_max_gauss_round", "30" );
ConVar sk_jeep_gauss_damage( "sk_jeep_gauss_damage", "15" );

// Combine Ball
ConVar sk_combineball_seek_angle( "sk_combineball_seek_angle", "15" );
ConVar sk_combineball_guidefactor( "sk_combineball_guidefactor", "1.0" );

//=============================================================================
// HEALTH/SUIT CHARGE DISTRIBUTION
//=============================================================================

// sk_suitcharger defined in func_recharge.cpp
// sk_battery defined in item_battery.cpp
// sk_healthcharger defined in item_healthkit.cpp
// sk_healthkit defined in item_healthkit.cpp
// sk_healthvial defined in item_healthkit.cpp

// Citadel chargers (higher capacity)
ConVar sk_suitcharger_citadel( "sk_suitcharger_citadel", "500" );
ConVar sk_suitcharger_citadel_maxarmor( "sk_suitcharger_citadel_maxarmor", "200" );

//=============================================================================
// ALLY REGENERATION
//=============================================================================

ConVar sk_ally_regen_time( "sk_ally_regen_time", "0.2" );
