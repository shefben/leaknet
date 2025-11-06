//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lua integration system for BarrysMod
// Based on reverse engineering of Garry's Mod server.dll
//
//=============================================================================//

#include "cbase.h"
#include "lua_integration.h"
#include "player.h"
#include "filesystem.h"
#include "convar.h"
#include "tier1/fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console Variables
//-----------------------------------------------------------------------------
ConVar lua_debug("lua_debug", "0", FCVAR_CHEAT, "Enable Lua debugging output");

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------
ConCommand lua_openscript("lua_openscript", LuaOpenScript_f, "Load and execute a Lua script");
ConCommand lua_listbinds("lua_listbinds", LuaListBinds_f, "List all registered Lua functions");

//-----------------------------------------------------------------------------
// Static member initialization
//-----------------------------------------------------------------------------
lua_State* CLuaIntegration::m_pLuaState = NULL;
CUtlVector<LuaFunctionRegistration> CLuaIntegration::m_RegisteredFunctions;
bool CLuaIntegration::m_bInitialized = false;

//-----------------------------------------------------------------------------
// Global instance
//-----------------------------------------------------------------------------
CLuaIntegration g_LuaIntegration;

//-----------------------------------------------------------------------------
// Purpose: Initialize the Lua integration system
//-----------------------------------------------------------------------------
void CLuaIntegration::Initialize()
{
	if (m_bInitialized)
		return;

	// Create new Lua state
	m_pLuaState = luaL_newstate();
	if (!m_pLuaState)
	{
		Warning("Lua Integration: Failed to create Lua state!\n");
		return;
	}

	// Open standard Lua libraries
	luaL_openlibs(m_pLuaState);

	// Set error handler
	lua_atpanic(m_pLuaState, LuaErrorHandler);

	// Register all our C++ functions
	RegisterAllFunctions();

	m_bInitialized = true;

	Msg("Lua Integration: System initialized successfully\n");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown the Lua integration system
//-----------------------------------------------------------------------------
void CLuaIntegration::Shutdown()
{
	if (!m_bInitialized)
		return;

	if (m_pLuaState)
	{
		lua_close(m_pLuaState);
		m_pLuaState = NULL;
	}

	m_RegisteredFunctions.Purge();
	m_bInitialized = false;

	Msg("Lua Integration: System shutdown\n");
}

//-----------------------------------------------------------------------------
// Purpose: Register a C++ function to be callable from Lua
//-----------------------------------------------------------------------------
void CLuaIntegration::RegisterFunction(const char *pszName, LuaCFunction pFunction, const char *pszDescription)
{
	if (!pszName || !pFunction)
		return;

	// Create registration entry
	LuaFunctionRegistration registration;
	Q_strncpy(registration.name, pszName, sizeof(registration.name));
	registration.function = pFunction;
	registration.description = pszDescription;
	registration.valid = true;

	// Add to list
	m_RegisteredFunctions.AddToTail(registration);

	// Register with Lua if state is available
	if (m_pLuaState)
	{
		lua_pushcfunction(m_pLuaState, pFunction);
		lua_setglobal(m_pLuaState, pszName);

		if (lua_debug.GetBool())
		{
			Msg("Lua: Registered function %s - %s\n", pszName, pszDescription ? pszDescription : "No description");
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Load and execute a Lua script file
//-----------------------------------------------------------------------------
bool CLuaIntegration::OpenScript(const char *pszFilename)
{
	if (!IsInitialized() || !pszFilename)
		return false;

	// Construct full path
	char szFullPath[MAX_PATH];
	Q_snprintf(szFullPath, sizeof(szFullPath), "lua/%s.lua", pszFilename);

	// Check if file exists
	if (!filesystem->FileExists(szFullPath, "GAME"))
	{
		Warning("Lua: Script file '%s' not found\n", szFullPath);
		return false;
	}

	// Load and execute the script
	int result = luaL_dofile(m_pLuaState, szFullPath);
	if (result != 0)
	{
		Warning("Lua: Error executing script '%s': %s\n", szFullPath, lua_tostring(m_pLuaState, -1));
		lua_pop(m_pLuaState, 1); // Remove error message
		return false;
	}

	Msg("Lua: Successfully loaded script '%s'\n", szFullPath);
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Call a Lua function
//-----------------------------------------------------------------------------
bool CLuaIntegration::CallFunction(const char *pszFunctionName, int nArgs)
{
	if (!IsInitialized() || !pszFunctionName)
		return false;

	// Get the function
	lua_getglobal(m_pLuaState, pszFunctionName);

	// Check if function exists
	if (!lua_isfunction(m_pLuaState, -1))
	{
		lua_pop(m_pLuaState, 1);
		return false;
	}

	// Call the function
	int result = lua_pcall(m_pLuaState, nArgs, 0, 0);
	if (result != 0)
	{
		Warning("Lua: Error calling function '%s': %s\n", pszFunctionName, lua_tostring(m_pLuaState, -1));
		lua_pop(m_pLuaState, 1); // Remove error message
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Execute Lua think functions (called every game tick)
//-----------------------------------------------------------------------------
void CLuaIntegration::DoThinkFunctions()
{
	if (!IsInitialized())
		return;

	// Call DoLuaThinkFunctions if it exists
	CallFunction("DoLuaThinkFunctions", 0);

	// Call gamerulesThink if it exists
	CallFunction("gamerulesThink", 0);
}

//-----------------------------------------------------------------------------
// Purpose: List all registered Lua functions
//-----------------------------------------------------------------------------
void CLuaIntegration::ListBinds()
{
	Msg("=== Lua Function Bindings ===\n");

	for (int i = 0; i < m_RegisteredFunctions.Count(); i++)
	{
		const LuaFunctionRegistration &registration = m_RegisteredFunctions[i];
		if (registration.valid)
		{
			Msg("%s - %s\n", registration.name,
				registration.description ? registration.description : "No description");
		}
	}

	Msg("Total: %d functions registered\n", m_RegisteredFunctions.Count());
}

//-----------------------------------------------------------------------------
// Purpose: Lua error handler
//-----------------------------------------------------------------------------
int CLuaIntegration::LuaErrorHandler(lua_State *L)
{
	const char *error = lua_tostring(L, -1);
	Warning("Lua Panic: %s\n", error ? error : "Unknown error");
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Register all C++ functions with Lua
//-----------------------------------------------------------------------------
void CLuaIntegration::RegisterAllFunctions()
{
	RegisterCoreFunctions();
	RegisterPlayerFunctions();
	RegisterEntityFunctions();
	RegisterWeaponFunctions();
	RegisterEffectFunctions();
	RegisterUIFunctions();
}

//-----------------------------------------------------------------------------
// Forward declarations for Lua functions
//-----------------------------------------------------------------------------
DECLARE_LUA_FUNCTION(PlayerGiveAmmo);
DECLARE_LUA_FUNCTION(PlayerSetSprint);
DECLARE_LUA_FUNCTION(PlayerShowScoreboard);
DECLARE_LUA_FUNCTION(EntRemove);
DECLARE_LUA_FUNCTION(EffectSetRadius);
DECLARE_LUA_FUNCTION(GModTextStart);

//-----------------------------------------------------------------------------
// Purpose: Register core Lua functions
//-----------------------------------------------------------------------------
void CLuaIntegration::RegisterCoreFunctions()
{
	// Core system functions would go here
}

//-----------------------------------------------------------------------------
// Purpose: Register player-related Lua functions
//-----------------------------------------------------------------------------
void CLuaIntegration::RegisterPlayerFunctions()
{
	REGISTER_LUA_FUNCTION(PlayerGiveAmmo, "Give specified player ammo. Syntax: <playerid> <num amount> <string ammotype> <bool playsounds>");
	REGISTER_LUA_FUNCTION(PlayerSetSprint, "Enable/Disable sprint for player. Syntax: <playerid> <freeze bool>");
	REGISTER_LUA_FUNCTION(PlayerShowScoreboard, "Shows the scoreboard on the specified players screen. Syntax: <playerid>");
}

//-----------------------------------------------------------------------------
// Purpose: Register entity-related Lua functions
//-----------------------------------------------------------------------------
void CLuaIntegration::RegisterEntityFunctions()
{
	REGISTER_LUA_FUNCTION(EntRemove, "Removes entity. Syntax <entindex>");
}

//-----------------------------------------------------------------------------
// Purpose: Register weapon-related Lua functions
//-----------------------------------------------------------------------------
void CLuaIntegration::RegisterWeaponFunctions()
{
	// Weapon functions would go here
}

//-----------------------------------------------------------------------------
// Purpose: Register effect-related Lua functions
//-----------------------------------------------------------------------------
void CLuaIntegration::RegisterEffectFunctions()
{
	REGISTER_LUA_FUNCTION(EffectSetRadius, "Syntax: <float>");
}

//-----------------------------------------------------------------------------
// Purpose: Register UI-related Lua functions
//-----------------------------------------------------------------------------
void CLuaIntegration::RegisterUIFunctions()
{
	REGISTER_LUA_FUNCTION(GModTextStart, "Initialize the text. Syntax: <fontname>");
}

//=============================================================================
// LUA FUNCTION IMPLEMENTATIONS
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: _PlayerGiveAmmo - Give ammo to a player
//-----------------------------------------------------------------------------
int Lua_PlayerGiveAmmo(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 3)
		return CLuaUtility::LuaError(L, "_PlayerGiveAmmo: Not enough arguments");

	int playerID = CLuaUtility::GetInt(L, 1);
	int amount = CLuaUtility::GetInt(L, 2);
	const char *ammoType = CLuaUtility::GetString(L, 3);
	bool playSounds = CLuaUtility::GetBool(L, 4, true);

	CBasePlayer *pPlayer = CLuaUtility::GetPlayerFromID(L, playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerGiveAmmo: Invalid player ID");

	// Give the ammo
	int ammoIndex = GetAmmoDef()->Index(ammoType);
	if (ammoIndex == -1)
		return CLuaUtility::LuaError(L, "_PlayerGiveAmmo: Invalid ammo type");

	int given = pPlayer->GiveAmmo(amount, ammoIndex, true);
	CLuaUtility::PushInt(L, given);

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: _PlayerSetSprint - Enable/disable sprint for player
//-----------------------------------------------------------------------------
int Lua_PlayerSetSprint(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 2)
		return CLuaUtility::LuaError(L, "_PlayerSetSprint: Not enough arguments");

	int playerID = CLuaUtility::GetInt(L, 1);
	bool canSprint = CLuaUtility::GetBool(L, 2);

	CBasePlayer *pPlayer = CLuaUtility::GetPlayerFromID(L, playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetSprint: Invalid player ID");

	// Enable/disable sprint
	pPlayer->SetMaxSpeed(canSprint ? 320.0f : 200.0f);

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: _PlayerShowScoreboard - Show scoreboard to player
//-----------------------------------------------------------------------------
int Lua_PlayerShowScoreboard(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 1)
		return CLuaUtility::LuaError(L, "_PlayerShowScoreboard: Not enough arguments");

	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = CLuaUtility::GetPlayerFromID(L, playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerShowScoreboard: Invalid player ID");

	// Show scoreboard - this would need specific implementation
	// For now, just print a message
	ClientPrint(pPlayer, HUD_PRINTTALK, "Scoreboard shown via Lua");

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: _EntRemove - Remove an entity
//-----------------------------------------------------------------------------
int Lua_EntRemove(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 1)
		return CLuaUtility::LuaError(L, "_EntRemove: Not enough arguments");

	int entIndex = CLuaUtility::GetInt(L, 1);

	CBaseEntity *pEntity = CLuaUtility::GetEntityFromIndex(L, entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntRemove: Invalid entity index");

	// Remove the entity
	UTIL_Remove(pEntity);

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: _EffectSetRadius - Set effect radius
//-----------------------------------------------------------------------------
int Lua_EffectSetRadius(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 1)
		return CLuaUtility::LuaError(L, "_EffectSetRadius: Not enough arguments");

	float radius = CLuaUtility::GetFloat(L, 1);

	// This would set some global effect radius
	// Implementation depends on the specific effect system
	Msg("Effect radius set to: %.2f\n", radius);

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: _GModTextStart - Initialize text display
//-----------------------------------------------------------------------------
int Lua_GModTextStart(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 1)
		return CLuaUtility::LuaError(L, "_GModTextStart: Not enough arguments");

	const char *fontName = CLuaUtility::GetString(L, 1);

	// Initialize text system with font
	Msg("GModText initialized with font: %s\n", fontName);

	return 0;
}

//=============================================================================
// CONSOLE COMMAND IMPLEMENTATIONS
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: lua_openscript console command
//-----------------------------------------------------------------------------
void LuaOpenScript_f(void)
{
	if (engine->Cmd_Argc() < 2)
	{
		Msg("Usage: lua_openscript <filename>\n");
		return;
	}

	if (!CLuaIntegration::IsInitialized())
	{
		Warning("Lua system not initialized!\n");
		return;
	}

	CLuaIntegration::OpenScript(engine->Cmd_Argv(1));
}

//-----------------------------------------------------------------------------
// Purpose: lua_listbinds console command
//-----------------------------------------------------------------------------
void LuaListBinds_f()
{
	if (!CLuaIntegration::IsInitialized())
	{
		Warning("Lua system not initialized!\n");
		return;
	}

	CLuaIntegration::ListBinds();
}