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

private:
    static lua_State* s_pLuaState;
    static char s_szLastError[1024];
    static bool s_bInitialized;
};

#endif // LUA_WRAPPER_H