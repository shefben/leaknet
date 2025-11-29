//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lua integration system for BarrysMod
// Based on reverse engineering of Garry's Mod server.dll
//
//=============================================================================//

#ifndef LUA_INTEGRATION_H
#define LUA_INTEGRATION_H
#ifdef _WIN32
#pragma once
#endif

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include "lua_compat.h"

#include "baseentity.h"
#include "convar.h"

//-----------------------------------------------------------------------------
// Lua Function Registration System
//-----------------------------------------------------------------------------

// Function signature for C++ functions callable from Lua
typedef int (*LuaCFunction)(lua_State *L);

// Structure to hold registered function information
struct LuaFunctionRegistration
{
	char			name[128];			// Function name (e.g., "_PlayerGiveAmmo")
	LuaCFunction	function;			// C++ function pointer
	const char*		description;		// Function description/syntax
	bool			valid;				// Registration is valid

	LuaFunctionRegistration()
	{
		name[0] = '\0';
		function = NULL;
		description = NULL;
		valid = false;
	}
};

//-----------------------------------------------------------------------------
// Core Lua Integration Manager
//-----------------------------------------------------------------------------
class CLuaIntegration
{
public:
	// Initialization/Shutdown
	static void		Initialize();
	static void		Shutdown();

	// Function Registration
	static void		RegisterFunction(const char *pszName, LuaCFunction pFunction, const char *pszDescription = NULL);
	static void		RegisterAllFunctions();

	// Script Execution
	static bool		OpenScript(const char *pszFilename);
	static bool		CallFunction(const char *pszFunctionName, int nArgs = 0);
	static void		DoThinkFunctions();

	// Lua State Management
	static lua_State* GetLuaState() { return m_pLuaState; }
	static bool		IsInitialized() { return m_pLuaState != NULL; }

	// Function Listing
	static void		ListBinds();

private:
	static lua_State*	m_pLuaState;
	static CUtlVector<LuaFunctionRegistration> m_RegisteredFunctions;
	static bool			m_bInitialized;

	// Internal helpers
	static int			LuaErrorHandler(lua_State *L);
	static void			RegisterCoreFunctions();
};

//-----------------------------------------------------------------------------
// Lua Console Commands
//-----------------------------------------------------------------------------
void LuaOpenScript_f(void);
void LuaListBinds_f(void);

//-----------------------------------------------------------------------------
// Lua Function Macros for easy registration
//-----------------------------------------------------------------------------
#define DECLARE_LUA_FUNCTION(name) int Lua_##name(lua_State *L)
#define REGISTER_LUA_FUNCTION(name, desc) CLuaIntegration::RegisterFunction("_"#name, Lua_##name, desc)

//-----------------------------------------------------------------------------
// Lua Utility Functions
//-----------------------------------------------------------------------------
class CLuaUtility
{
public:
	// Parameter helpers
	static CBaseEntity*	GetEntityFromIndex(lua_State *L, int index);
	static CBasePlayer*	GetPlayerFromID(lua_State *L, int index);
	static bool			GetBool(lua_State *L, int index, bool defaultValue = false);
	static int			GetInt(lua_State *L, int index, int defaultValue = 0);
	static float		GetFloat(lua_State *L, int index, float defaultValue = 0.0f);
	static const char*	GetString(lua_State *L, int index, const char *defaultValue = "");

	// Return value helpers
	static void			PushEntity(lua_State *L, CBaseEntity *pEntity);
	static void			PushPlayer(lua_State *L, CBasePlayer *pPlayer);
	static void			PushBool(lua_State *L, bool value);
	static void			PushInt(lua_State *L, int value);
	static void			PushFloat(lua_State *L, float value);
	static void			PushString(lua_State *L, const char *value);

	// Error handling
	static int			LuaError(lua_State *L, const char *pszFormat, ...);
};

extern CLuaIntegration g_LuaIntegration;

// Forward declaration - full class definition in gmod_runfunction.h
class CGmodRunFunction;

//-----------------------------------------------------------------------------
// External Lua function registration (from separate files)
//-----------------------------------------------------------------------------
void RegisterLuaEntityFunctions();
void RegisterLuaPlayerFunctions();
void RegisterLuaPhysicsFunctions();
void RegisterLuaFileFunctions();
void RegisterLuaEffectFunctions();
void RegisterLuaGameEventFunctions();

#endif // LUA_INTEGRATION_H
