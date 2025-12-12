//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GMod 9 CVars and Console Commands for BarrysMod
// Based on reverse engineering of Garry's Mod 9.0.4b server.dll
//
// This file implements missing CVars and console commands identified
// during the IDA Pro analysis of GMod 9.0.4b.
//
// NOTE: Commands that are already defined in gmod_system.cpp are NOT
// included here to avoid linker errors.
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "convar.h"
#include "usermessages.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// MISSING CVARS FROM GMOD 9.0.4b (CVR_* prefix in IDA)
// Note: g_debug_physcannon is already defined in weapon_physcannon.cpp
//=============================================================================

// Player info fetch control
ConVar sv_fetchPlayerInfo("sv_fetchPlayerInfo", "1", FCVAR_GAMEDLL, "Enable fetching player info from Steam");

// Camera control CVars (used by camera tool)
// These are typically controlled via +/- commands
ConVar gm_cam_static("gm_cam_static", "0", FCVAR_NONE, "Camera static mode");
ConVar gm_cam_prop("gm_cam_prop", "0", FCVAR_NONE, "Camera prop follow mode");
ConVar gm_cam_view("gm_cam_view", "0", FCVAR_NONE, "Camera view mode");

// Debug CVars (g_debug_physcannon is in weapon_physcannon.cpp)
ConVar gm_debug_pistoltracepos("gm_debug_pistoltracepos", "0", FCVAR_CHEAT, "Debug pistol trace position");

// Prop camera creation
ConVar gm_make_propcam("gm_make_propcam", "0", FCVAR_NONE, "Enable prop camera creation");

// Physgun/tool CVars (from IDA: gmod_physgun_*)
ConVar gmod_physgun_maxrange("gmod_physgun_maxrange", "4096", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum range of the physics gun");
ConVar gmod_physgun_maxforce("gmod_physgun_maxforce", "5000", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum force of the physics gun");
ConVar gmod_physgun_halo("gmod_physgun_halo", "1", FCVAR_GAMEDLL, "Show physgun halo effect");
ConVar gmod_physgun_wheelspeed("gmod_physgun_wheelspeed", "10", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Physgun scroll wheel rotation speed");

// Noclip and admin CVars
ConVar gmod_noclip("gmod_noclip", "0", FCVAR_GAMEDLL, "Enable noclip for players");
ConVar gmod_admin("gmod_admin", "", FCVAR_GAMEDLL | FCVAR_PROTECTED, "Admin password");

// Tool CVars
ConVar gmod_tool_default("gmod_tool_default", "weld", FCVAR_GAMEDLL, "Default tool when spawning");
ConVar gmod_tool_faceposer_enabled("gmod_tool_faceposer_enabled", "1", FCVAR_GAMEDLL, "Enable face poser tool");
ConVar gmod_tool_balloon_enabled("gmod_tool_balloon_enabled", "1", FCVAR_GAMEDLL, "Enable balloon tool");
ConVar gmod_tool_thruster_enabled("gmod_tool_thruster_enabled", "1", FCVAR_GAMEDLL, "Enable thruster tool");
ConVar gmod_tool_wheel_enabled("gmod_tool_wheel_enabled", "1", FCVAR_GAMEDLL, "Enable wheel tool");
ConVar gmod_tool_light_enabled("gmod_tool_light_enabled", "1", FCVAR_GAMEDLL, "Enable light tool");
ConVar gmod_tool_lamp_enabled("gmod_tool_lamp_enabled", "1", FCVAR_GAMEDLL, "Enable lamp tool");
ConVar gmod_tool_dynamite_enabled("gmod_tool_dynamite_enabled", "1", FCVAR_GAMEDLL, "Enable dynamite tool");
ConVar gmod_tool_emitter_enabled("gmod_tool_emitter_enabled", "1", FCVAR_GAMEDLL, "Enable emitter tool");

// Spawn CVars
ConVar gmod_spawn_effect("gmod_spawn_effect", "1", FCVAR_GAMEDLL, "Play spawn effect when creating props");
ConVar gmod_spawn_limit_props("gmod_spawn_limit_props", "100", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum props per player");
ConVar gmod_spawn_limit_ragdolls("gmod_spawn_limit_ragdolls", "10", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum ragdolls per player");
ConVar gmod_spawn_limit_effects("gmod_spawn_limit_effects", "50", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum effects per player");
ConVar gmod_spawn_limit_npcs("gmod_spawn_limit_npcs", "10", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum NPCs per player");
ConVar gmod_spawn_limit_vehicles("gmod_spawn_limit_vehicles", "4", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum vehicles per player");
ConVar gmod_spawn_limit_balloons("gmod_spawn_limit_balloons", "50", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum balloons per player");
ConVar gmod_spawn_limit_dynamite("gmod_spawn_limit_dynamite", "10", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum dynamite per player");
ConVar gmod_spawn_limit_thrusters("gmod_spawn_limit_thrusters", "20", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum thrusters per player");
ConVar gmod_spawn_limit_wheels("gmod_spawn_limit_wheels", "20", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum wheels per player");
ConVar gmod_spawn_limit_lights("gmod_spawn_limit_lights", "20", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum lights per player");
ConVar gmod_spawn_limit_lamps("gmod_spawn_limit_lamps", "10", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum lamps per player");
ConVar gmod_spawn_limit_emitters("gmod_spawn_limit_emitters", "10", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Maximum emitters per player");

//=============================================================================
// GMOD 9 LEGACY CVARS (gm_sv_* prefix from config.cfg)
// These are the original GMod 9 server CVars with "gm_sv_" prefix
//=============================================================================

// Server gameplay permissions
ConVar gm_sv_allweapons("gm_sv_allweapons", "1", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Allow all weapons on server");
ConVar gm_sv_allowignite("gm_sv_allowignite", "1", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Allow igniting props/NPCs");
ConVar gm_sv_noclip("gm_sv_noclip", "1", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Allow noclip for players");
ConVar gm_sv_playerdamage("gm_sv_playerdamage", "1", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Players can take damage");
ConVar gm_sv_pvpdamage("gm_sv_pvpdamage", "0", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Players can damage each other");
ConVar gm_sv_teamdamage("gm_sv_teamdamage", "0", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Team damage enabled");
ConVar gm_sv_allownpc("gm_sv_allownpc", "1", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Allow NPC spawning");
ConVar gm_sv_allowspawning("gm_sv_allowspawning", "1", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Allow prop spawning");
ConVar gm_sv_allowmultigun("gm_sv_allowmultigun", "1", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Allow multiple physics guns");
ConVar gm_sv_allowphysgun("gm_sv_allowphysgun", "1", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Allow physics gun");

// Per-client spawn limits (gm_sv_clientlimit_*)
ConVar gm_sv_clientlimit_ragdolls("gm_sv_clientlimit_ragdolls", "10", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Max ragdolls per client");
ConVar gm_sv_clientlimit_thrusters("gm_sv_clientlimit_thrusters", "20", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Max thrusters per client");
ConVar gm_sv_clientlimit_props("gm_sv_clientlimit_props", "100", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Max props per client");
ConVar gm_sv_clientlimit_balloons("gm_sv_clientlimit_balloons", "50", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Max balloons per client");
ConVar gm_sv_clientlimit_effects("gm_sv_clientlimit_effects", "50", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Max effects per client");
ConVar gm_sv_clientlimit_sprites("gm_sv_clientlimit_sprites", "20", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Max sprites per client");
ConVar gm_sv_clientlimit_emitters("gm_sv_clientlimit_emitters", "10", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Max emitters per client");
ConVar gm_sv_clientlimit_wheels("gm_sv_clientlimit_wheels", "20", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Max wheels per client");
ConVar gm_sv_clientlimit_npcs("gm_sv_clientlimit_npcs", "10", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Max NPCs per client");
ConVar gm_sv_clientlimit_vehicles("gm_sv_clientlimit_vehicles", "4", FCVAR_GAMEDLL | FCVAR_REPLICATED, "Max vehicles per client");

//=============================================================================
// CAMERA +/- COMMAND HANDLERS (CVR_plus_gm_cam_* / CVR_minus_gm_cam_*)
//=============================================================================

void CC_plus_gm_cam_static(void)
{
	gm_cam_static.SetValue(1);
}

void CC_minus_gm_cam_static(void)
{
	gm_cam_static.SetValue(0);
}

void CC_plus_gm_cam_prop(void)
{
	gm_cam_prop.SetValue(1);
}

void CC_minus_gm_cam_prop(void)
{
	gm_cam_prop.SetValue(0);
}

void CC_plus_gm_cam_view(void)
{
	gm_cam_view.SetValue(1);
}

void CC_minus_gm_cam_view(void)
{
	gm_cam_view.SetValue(0);
}

// Register +/- commands
static ConCommand plus_gm_cam_static("+gm_cam_static", CC_plus_gm_cam_static, "Hold camera static");
static ConCommand minus_gm_cam_static("-gm_cam_static", CC_minus_gm_cam_static, "Release camera static");
static ConCommand plus_gm_cam_prop("+gm_cam_prop", CC_plus_gm_cam_prop, "Hold camera prop mode");
static ConCommand minus_gm_cam_prop("-gm_cam_prop", CC_minus_gm_cam_prop, "Release camera prop mode");
static ConCommand plus_gm_cam_view("+gm_cam_view", CC_plus_gm_cam_view, "Hold camera view mode");
static ConCommand minus_gm_cam_view("-gm_cam_view", CC_minus_gm_cam_view, "Release camera view mode");

//=============================================================================
// NON-DUPLICATE CONSOLE COMMANDS FROM GMOD 9.0.4b
// Note: Many commands already exist in gmod_system.cpp, so only unique ones here
//=============================================================================

// CMD_optionselect - Option selection (used by menus)
void CC_GMod_OptionSelect(void)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (!pPlayer)
		return;

	if (engine->Cmd_Argc() < 2)
	{
		Msg("Usage: optionselect <option_number>\n");
		return;
	}

	int option = atoi(engine->Cmd_Argv(1));
	DevMsg("GMod: Option %d selected by player %s\n", option, STRING(pPlayer->pl.netname));

	// This is used by VGUI menus to report which option was selected
	// Could trigger a Lua hook here
}
static ConCommand optionselect("optionselect", CC_GMod_OptionSelect, "Select a menu option");

// lua_run - Run Lua code (server-side)
void CC_Lua_Run(void)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();

	// For dedicated servers, allow console use
	// For listen servers, require admin
	if (pPlayer)
	{
		DevMsg("lua_run: Player %s executed Lua\n", STRING(pPlayer->pl.netname));
	}

	if (engine->Cmd_Argc() < 2)
	{
		Msg("Usage: lua_run <lua code>\n");
		return;
	}

	// Concatenate all arguments into the Lua string
	char luaCode[4096];
	luaCode[0] = '\0';

	for (int i = 1; i < engine->Cmd_Argc(); i++)
	{
		if (i > 1)
			Q_strncat(luaCode, " ", sizeof(luaCode));
		Q_strncat(luaCode, engine->Cmd_Argv(i), sizeof(luaCode));
	}

	DevMsg("lua_run: Executing: %s\n", luaCode);

	// Execute via Lua integration
	// CLuaIntegration::ExecuteString(luaCode);
}
static ConCommand lua_run("lua_run", CC_Lua_Run, "Execute Lua code on the server");

// lua_run_cl - Run Lua code on client (sends to client)
void CC_Lua_Run_CL(void)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (!pPlayer)
		return;

	if (engine->Cmd_Argc() < 2)
	{
		Msg("Usage: lua_run_cl <lua code>\n");
		return;
	}

	// Concatenate arguments
	char luaCode[4096];
	luaCode[0] = '\0';

	for (int i = 1; i < engine->Cmd_Argc(); i++)
	{
		if (i > 1)
			Q_strncat(luaCode, " ", sizeof(luaCode));
		Q_strncat(luaCode, engine->Cmd_Argv(i), sizeof(luaCode));
	}

	DevMsg("lua_run_cl: Sending to client: %s\n", luaCode);

	// Send to client to execute
	// This would require a client-side Lua system
}
static ConCommand lua_run_cl("lua_run_cl", CC_Lua_Run_CL, "Execute Lua code on the client");

//=============================================================================
// ADMIN COMMANDS
//=============================================================================

// gmod_setadmin - Check/set admin status
void CC_GMod_SetAdmin(void)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();

	if (engine->Cmd_Argc() < 2)
	{
		Msg("Usage: gmod_setadmin <password>\n");
		return;
	}

	const char *password = engine->Cmd_Argv(1);
	const char *adminPassword = gmod_admin.GetString();

	if (adminPassword[0] == '\0')
	{
		Msg("Admin system not configured. Set gmod_admin to a password first.\n");
		return;
	}

	if (Q_strcmp(password, adminPassword) == 0)
	{
		if (pPlayer)
		{
			Msg("Admin access granted to %s\n", STRING(pPlayer->pl.netname));
		}
		else
		{
			Msg("Admin access granted (console)\n");
		}
	}
	else
	{
		Msg("Invalid admin password.\n");
	}
}
static ConCommand gmod_setadmin("gmod_setadmin", CC_GMod_SetAdmin, "Authenticate as admin");

//=============================================================================
// SPAWN NPC COMMAND
//=============================================================================

void CC_GMod_SpawnNPC(void)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (!pPlayer)
		return;

	if (engine->Cmd_Argc() < 2)
	{
		Msg("Usage: gmod_spawnnpc <npc_classname>\n");
		Msg("Example: gmod_spawnnpc npc_combine_s\n");
		return;
	}

	const char *npcClass = engine->Cmd_Argv(1);

	// Do trace to find spawn location
	Vector vecStart = pPlayer->EyePosition();
	Vector vecDir;
	AngleVectors(pPlayer->EyeAngles(), &vecDir);
	Vector vecEnd = vecStart + vecDir * 4096.0f;

	trace_t tr;
	UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

	Vector spawnPos = tr.endpos + Vector(0, 0, 32); // Spawn slightly above ground

	// Create the NPC
	CBaseEntity *pNPC = CreateEntityByName(npcClass);
	if (pNPC)
	{
		pNPC->SetAbsOrigin(spawnPos);
		pNPC->SetAbsAngles(pPlayer->EyeAngles());
		DispatchSpawn(pNPC);
		pNPC->Activate();

		DevMsg("GMod: Spawned NPC '%s' at (%.0f, %.0f, %.0f)\n",
			npcClass, spawnPos.x, spawnPos.y, spawnPos.z);
	}
	else
	{
		Msg("Failed to spawn NPC: %s\n", npcClass);
	}
}
static ConCommand gmod_spawnnpc("gmod_spawnnpc", CC_GMod_SpawnNPC, "Spawn an NPC at crosshair");

//=============================================================================
// TOOL COMMAND
//=============================================================================

void CC_GMod_Tool(void)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (!pPlayer)
		return;

	if (engine->Cmd_Argc() < 2)
	{
		Msg("Current tool: %s\n", gmod_tool_default.GetString());
		Msg("Usage: gmod_tool <toolname>\n");
		Msg("Available tools: weld, axis, rope, pulley, slider, ball, wheel, thruster, light, lamp, balloon, dynamite, emitter, faceposer\n");
		return;
	}

	const char *toolName = engine->Cmd_Argv(1);
	gmod_tool_default.SetValue(toolName);

	DevMsg("GMod: Tool changed to '%s' for player %s\n", toolName, STRING(pPlayer->pl.netname));
}
static ConCommand gmod_tool("gmod_tool", CC_GMod_Tool, "Set current tool");
