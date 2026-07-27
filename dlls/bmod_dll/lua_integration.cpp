//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lua integration system for BarrysMod
// Based on reverse engineering of Garry's Mod server.dll
//
//=============================================================================//

#include "cbase.h"
#include "lua_integration.h"
#include "gmod_lua.h"
#include "player.h"
#include "filesystem.h"
#include "convar.h"
#include "fmtstr.h"
#include "ammodef.h"
#include "KeyValues.h"

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
// Console Commands - original gmod 9.0.4b names and help strings
//-----------------------------------------------------------------------------
ConCommand lua_command("lua", LuaRunCommand_f, "Runs a LUA string command on the server");
// Yes, the help text is wrong - the original gmod copy/pastes it here too, and
// lua_listbinds output is compared against real gmod dumps, so keep it verbatim.
ConCommand lua_openscript("lua_openscript", LuaOpenScript_f, "Runs a LUA string command on the server");
ConCommand lua_listbinds("lua_listbinds", LuaListBinds_f, "Lists source engine functions available to LUA");

//-----------------------------------------------------------------------------
// Static member initialization
//-----------------------------------------------------------------------------
lua_State* CLuaIntegration::m_pLuaState = NULL;
CUtlVector<LuaFunctionRegistration> CLuaIntegration::m_RegisteredFunctions;
bool CLuaIntegration::m_bInitialized = false;
bool CLuaIntegration::m_bAllowLuaCommand = true;
bool CLuaIntegration::m_bAllowOpenScriptCommand = true;
CUtlVector<CUtlSymbol> CLuaIntegration::m_DisabledFunctions;

//-----------------------------------------------------------------------------
// Purpose: Only server admins (or a listen/single-player host) may drive the Lua
//          console commands, matching the original gmod gate.
//-----------------------------------------------------------------------------
static bool LuaCommandAllowedForCaller()
{
	// Original gmod accepts the command when it comes from the server console
	// (dedicated console / rcon) or when the server is a listen/single-player
	// game; clients on a multiplayer server are refused.
	return UTIL_GetCommandClient() == NULL || gpGlobals->maxClients <= 1;
}

static void LuaPrintToCaller(const char *pszMessage)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (pPlayer)
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, pszMessage);
	else
		Msg("%s", pszMessage);
}

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

	// cfg/lua.txt "Disable" section removes the bind entirely, exactly like the original.
	if (IsFunctionDisabled(NULL, pszName))
		return;

	// Create registration entry
	LuaFunctionRegistration registration;
	registration.table[0] = '\0';
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
// Purpose: Register a C++ function as a member of one of the original gmod
//          global tables (_file, _swep, _phys, _npc, _player, _util,
//          _spawnmenu, _gameevent). Creates the table on first use.
//-----------------------------------------------------------------------------
void CLuaIntegration::RegisterTableFunction(const char *pszTable, const char *pszName, LuaCFunction pFunction, const char *pszDescription)
{
	if (!pszTable || !pszName || !pFunction)
		return;

	if (IsFunctionDisabled(pszTable, pszName))
		return;

	LuaFunctionRegistration registration;
	Q_strncpy(registration.table, pszTable, sizeof(registration.table));
	Q_strncpy(registration.name, pszName, sizeof(registration.name));
	registration.function = pFunction;
	registration.description = pszDescription;
	registration.valid = true;

	m_RegisteredFunctions.AddToTail(registration);

	if (!m_pLuaState)
		return;

	lua_getglobal(m_pLuaState, pszTable);
	if (!lua_istable(m_pLuaState, -1))
	{
		lua_pop(m_pLuaState, 1);
		lua_newtable(m_pLuaState);
		lua_pushvalue(m_pLuaState, -1);
		lua_setglobal(m_pLuaState, pszTable);
	}

	lua_pushcfunction(m_pLuaState, pFunction);
	lua_setfield(m_pLuaState, -2, pszName);
	lua_pop(m_pLuaState, 1);

	if (lua_debug.GetBool())
	{
		Msg("Lua: Registered function %s.%s - %s\n", pszTable, pszName,
			pszDescription ? pszDescription : "No description");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Drop the bind list. Called before rebuilding the bindings so that
//          repeated Lua (re)initialisation does not accumulate duplicates.
//-----------------------------------------------------------------------------
void CLuaIntegration::ClearRegistrations()
{
	m_RegisteredFunctions.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: Process cfg/lua.txt, the original gmod "Lua Config" file.
//          Settings/AllowLuaCommand           - enables the "lua" command
//          Settings/AllowLua_OpenScriptCommand- enables "lua_openscript"
//          Disable/<name|table.name>          - stops the bind being registered
//-----------------------------------------------------------------------------
void CLuaIntegration::LoadLuaConfig()
{
	m_bAllowLuaCommand = true;
	m_bAllowOpenScriptCommand = true;
	m_DisabledFunctions.RemoveAll();

	Msg("Processing Lua config file..\n");

	KeyValues *pConfig = new KeyValues("Lua Settings");
	if (!pConfig)
	{
		Msg("Couldn't create keyvalues!..\n");
		return;
	}

	if (!pConfig->LoadFromFile(filesystem, "cfg/lua.txt", NULL))
	{
		Msg("Couldn't load from \"cfg/lua.txt\"!\n");
		pConfig->deleteThis();
		return;
	}

	KeyValues *pSettings = pConfig->FindKey("Settings");
	if (pSettings)
	{
		m_bAllowLuaCommand = pSettings->GetInt("AllowLuaCommand", 1) == 1;
		m_bAllowOpenScriptCommand = pSettings->GetInt("AllowLua_OpenScriptCommand", 1) == 1;
	}

	KeyValues *pDisable = pConfig->FindKey("Disable");
	if (pDisable)
	{
		for (KeyValues *pKey = pDisable->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey())
		{
			if (pKey->GetInt() != 1)
				continue;

			m_DisabledFunctions.AddToTail(CUtlSymbol(pKey->GetName()));
			Msg("Lua: Removed \"%s\"\n", pKey->GetName());
		}
	}

	pConfig->deleteThis();
	Msg("Finished Lua Config.\n");
}

//-----------------------------------------------------------------------------
// Purpose: Is this bind listed in the cfg/lua.txt "Disable" section?
//-----------------------------------------------------------------------------
bool CLuaIntegration::IsFunctionDisabled(const char *pszTable, const char *pszName)
{
	if (m_DisabledFunctions.Count() == 0 || !pszName)
		return false;

	char szKey[192];
	if (pszTable && pszTable[0])
		Q_snprintf(szKey, sizeof(szKey), "%s.%s", pszTable, pszName);
	else
		Q_strncpy(szKey, pszName, sizeof(szKey));

	CUtlSymbol symbol(szKey);
	for (int i = 0; i < m_DisabledFunctions.Count(); i++)
	{
		if (m_DisabledFunctions[i] == symbol)
			return true;
	}

	return false;
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
	// Output format is the original gmod one, so that wiki dumps of
	// "lua_listbinds" line up character for character:
	//     * [[_EntSetPos]] - Sets the position of the entity. ...
	//     * [[_file]].[[Read]] - Reads a file into a string. ...
	for (int i = 0; i < m_RegisteredFunctions.Count(); i++)
	{
		const LuaFunctionRegistration &registration = m_RegisteredFunctions[i];
		if (!registration.valid)
			continue;

		Msg("* ");
		if (registration.table[0])
			Msg("[[%s]].", registration.table);

		Msg("[[%s]] - %s\n", registration.name,
			registration.description ? registration.description : "");
	}
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
// Purpose: "lua" console command - runs a LUA string command on the server
//-----------------------------------------------------------------------------
void LuaRunCommand_f(void)
{
	if (!CLuaIntegration::IsLuaCommandAllowed())
	{
		Msg("The \"lua\" command has been disabled by the server admin\n");
		return;
	}

	if (!LuaCommandAllowedForCaller())
	{
		LuaPrintToCaller("This command can only be used by server admins.\n");
		return;
	}

	if (engine->Cmd_Argc() <= 1)
	{
		LuaPrintToCaller("Usage:\n   lua \"<command>\"\n");
		return;
	}

	const char *pszCode = engine->Cmd_Args();
	if (!pszCode || !pszCode[0])
	{
		Msg("LUA: Attempted to run a NULL string!?\n");
		return;
	}

	CGModLuaSystem::ExecuteString(pszCode);
}

//-----------------------------------------------------------------------------
// Purpose: lua_openscript console command
//-----------------------------------------------------------------------------
void LuaOpenScript_f(void)
{
	if (!CLuaIntegration::IsOpenScriptCommandAllowed())
	{
		Msg("The \"lua_openscript\" command has been disabled by the server admin\n");
		return;
	}

	if (!LuaCommandAllowedForCaller())
	{
		LuaPrintToCaller("This command can only be used by server admins. Try using 'lua_openscript' to open scripts on the client side.\n");
		return;
	}

	if (engine->Cmd_Argc() <= 1)
	{
		LuaPrintToCaller("Usage:\n   lua_openscript \"<filename>\"\n");
		return;
	}

	CGModLuaSystem::LoadScript(engine->Cmd_Args(), LUA_SCRIPT_MISC);
}

//-----------------------------------------------------------------------------
// Purpose: lua_listbinds console command
//-----------------------------------------------------------------------------
void LuaListBinds_f()
{
	if (!LuaCommandAllowedForCaller())
	{
		LuaPrintToCaller("This command can only be used by server admins.\n");
		return;
	}

	CLuaIntegration::ListBinds();
}
