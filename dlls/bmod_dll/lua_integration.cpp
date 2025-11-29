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
#include "fmtstr.h"
#include "ammodef.h"

extern "C" {
	int luaopen_base(lua_State *L);
	int luaopen_table(lua_State *L);
	int luaopen_io(lua_State *L);
	int luaopen_string(lua_State *L);
	int luaopen_math(lua_State *L);
	int luaopen_debug(lua_State *L);
	int luaopen_loadlib(lua_State *L);
}

static void LuaOpenAllLibs(lua_State *L)
{
	static const luaL_reg libs[] = {
		{"", luaopen_base},
		{LUA_TABLIBNAME, luaopen_table},
		{LUA_IOLIBNAME, luaopen_io},
		{LUA_STRLIBNAME, luaopen_string},
		{LUA_MATHLIBNAME, luaopen_math},
		{LUA_DBLIBNAME, luaopen_debug},
		{LUA_LOADLIBNAME, luaopen_loadlib},
		{NULL, NULL}
	};

	for (const luaL_reg* lib = libs; lib->func; ++lib)
	{
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 0);
	}
}

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
	LuaOpenAllLibs(m_pLuaState);

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
	// Core functions (defined in this file)
	RegisterCoreFunctions();

	// External function registrations (from separate files)
	RegisterLuaEntityFunctions();
	RegisterLuaPlayerFunctions();
	RegisterLuaPhysicsFunctions();
	RegisterLuaFileFunctions();
	RegisterLuaEffectFunctions();
	RegisterLuaGameEventFunctions();
}

//-----------------------------------------------------------------------------
// Purpose: Register core Lua functions (system, math, etc.)
//-----------------------------------------------------------------------------
void CLuaIntegration::RegisterCoreFunctions()
{
	// Register some core utility functions that don't fit in other categories

	// Math constants
	lua_pushnumber(m_pLuaState, 3.14159265358979323846);
	lua_setglobal(m_pLuaState, "PI");

	// Engine tick rate (HL2 beta uses 66 tick rate ~= 0.015 per tick)
	lua_pushnumber(m_pLuaState, 0.015);
	lua_setglobal(m_pLuaState, "TICK_INTERVAL");
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
