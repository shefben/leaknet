#ifndef LUA_WRAPPER_H
#define LUA_WRAPPER_H

#pragma once

// Lua 5.0.3 C++ Wrapper for GMod Integration
// This provides a clean C++ interface to Lua functionality

extern "C" {
    // Include Lua headers in C mode
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

#include "lua_compat.h"
#include "cbase.h"
#include "igameevents.h"
#include "KeyValues.h"

class CLuaWrapper
{
public:
    static bool InitializeLua();
    static void ShutdownLua();
    static lua_State* GetLuaState() { return s_pLuaState; }

    // Lua script execution
    static bool RunLuaString(const char* pszLuaCode);
    static bool RunLuaFile(const char* pszFilename);

    // Lua stack management
    static void PushString(const char* pszString);
    static void PushNumber(double dNumber);
    static void PushBool(bool bValue);
    static void PushNil();

    static const char* GetString(int iIndex);
    static double GetNumber(int iIndex);
    static bool GetBool(int iIndex);
    static int GetTop();
    static void Pop(int iNum);

    // Lua function calls
    static bool CallLuaFunction(const char* pszFunctionName, int iArgs = 0, int iReturns = 0);

    // Error handling
    static const char* GetLastError() { return s_szLastError; }

    // Helpers for Lua bindings
    static CBaseEntity* GetLuaEntity(lua_State* L, int index);
    static const char* GetString(lua_State* L, int index, const char* defaultValue = "");
    static int GetInt(lua_State* L, int index, int defaultValue = 0);
    static float GetFloat(lua_State* L, int index, float defaultValue = 0.0f);
    static bool GetBool(lua_State* L, int index, bool defaultValue = false);
    static bool GetVector(lua_State* L, int index, Vector& outVec);

    // GameEvent helpers (parity with gmod server.dll)
    static void StartGameEvent(CBasePlayer* pPlayer);
	static void GameEventSetString(const char* pszName);
	static void GameEventSetPlayerInt(CBasePlayer* pSource, CBasePlayer* pTarget, int key, int value);
	static void GameEventSetPlayerVector(CBasePlayer* pSource, const Vector& vec);
	static void CommitActiveGameEvent();

private:
	static lua_State* s_pLuaState;
	static char s_szLastError[1024];
	static bool s_bInitialized;
	static KeyValues *s_pActiveEvent;
};

#endif // LUA_WRAPPER_H
