#ifndef GMOD_LUA_H
#define GMOD_LUA_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "igamesystem.h"
#include "convar.h"
#include "utlvector.h"
#include "utlsymbol.h"

// Lua 5.0 includes (determined correct version from directory analysis)
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// Forward declarations
class CBaseEntity;
class CBasePlayer;

// Lua function result types
enum LuaFunctionResult_t
{
    LUA_RESULT_SUCCESS = 0,
    LUA_RESULT_ERROR,
    LUA_RESULT_SYNTAX_ERROR,
    LUA_RESULT_MEMORY_ERROR,
    LUA_RESULT_RUNTIME_ERROR,
    LUA_RESULT_FILE_NOT_FOUND
};

// Lua script types discovered in GMod directory analysis
enum LuaScriptType_t
{
    LUA_SCRIPT_GAMEMODE = 0,    // Gamemode scripts (gm_910.lua, etc.)
    LUA_SCRIPT_SWEP,            // SWEP weapon scripts
    LUA_SCRIPT_SENT,            // Scripted entity scripts
    LUA_SCRIPT_INCLUDE,         // Include files (defines.lua, vector3.lua, etc.)
    LUA_SCRIPT_MISC,            // Miscellaneous scripts
    LUA_SCRIPT_MAX
};

// Lua execution context
struct LuaContext_t
{
    lua_State* L;
    LuaScriptType_t scriptType;
    char fileName[256];
    char directoryPath[256];
    bool isLoaded;
    bool isRunning;
    int errorCount;
    float lastExecTime;
    CBasePlayer* pContextPlayer;
    CBaseEntity* pContextEntity;
    trace_t lastTrace;

    LuaContext_t()
    {
        L = NULL;
        scriptType = LUA_SCRIPT_MISC;
        fileName[0] = '\0';
        directoryPath[0] = '\0';
        isLoaded = false;
        isRunning = false;
        errorCount = 0;
        lastExecTime = 0.0f;
        pContextPlayer = NULL;
        pContextEntity = NULL;
    }
};

//-----------------------------------------------------------------------------
// GMod Lua System - Implements complete Lua integration from GMod 9.0.4b
// Including SWEP system, gamemode system, and all engine bindings
//-----------------------------------------------------------------------------
class CGModLuaSystem : public CAutoGameSystem
{
public:
    CGModLuaSystem() : CAutoGameSystem() {}

    // CAutoGameSystem overrides
    virtual bool Init();
    virtual void Shutdown();
    virtual void LevelInitPostEntity();
    virtual void FrameUpdatePreEntityThink();

    // Core Lua functionality
    static LuaFunctionResult_t InitializeLua();
    static void ShutdownLua();
    static LuaFunctionResult_t LoadScript(const char* pszFileName, LuaScriptType_t type);
    static LuaFunctionResult_t ExecuteString(const char* pszCode);
    static LuaFunctionResult_t ExecuteFunction(const char* pszFunctionName, int numArgs = 0);
    static LuaFunctionResult_t ReloadScript(const char* pszFileName);

    // SWEP system integration
    static bool LoadSWEP(const char* pszWeaponName);
    static bool InitializeSWEP(CBaseEntity* pWeapon, const char* pszWeaponName);
    static void CallSWEPFunction(CBaseEntity* pWeapon, const char* pszFunction);
    static int GetSWEPProperty(CBaseEntity* pWeapon, const char* pszProperty);
    static const char* GetSWEPStringProperty(CBaseEntity* pWeapon, const char* pszProperty);

    // Gamemode system integration
    static bool LoadGamemode(const char* pszGamemodeName);
    static void CallGamemodeFunction(const char* pszFunction);
    static void SetGamemodeProperty(const char* pszProperty, const char* pszValue);
    static const char* GetGamemodeProperty(const char* pszProperty);

    // Script management
    static void LoadAllScripts();
    static void LoadGamemodeScripts();
    static void LoadSWEPScripts();
    static void LoadIncludeScripts();
    static void ReloadAllScripts();

    // Engine binding registration
    static void RegisterGamemodeFunctions();

    // Error handling
    static void HandleLuaError(lua_State* L, const char* pszContext);
    static void PrintLuaError(const char* pszError);
    static int GetErrorCount();
    static void ClearErrorCount();

    // Utility functions
    static lua_State* GetLuaState();
    static LuaContext_t* GetCurrentContext();
    static void SetContextPlayer(CBasePlayer* pPlayer);
    static void SetContextEntity(CBaseEntity* pEntity);

private:
    static lua_State* s_pLuaState;
    static CUtlVector<LuaContext_t> s_LoadedScripts;
    static LuaContext_t s_CurrentContext;
    static int s_ErrorCount;
    static bool s_bSystemInitialized;

    // Internal functions
    static void RegisterEngineBindings();
    static void RegisterPlayerFunctions();
    static void RegisterEntityFunctions();
    static void RegisterTraceFunctions();
    static void RegisterPhysicsFunctions();
    static void RegisterUtilityFunctions();
    static void RegisterConVarFunctions();
    static void RegisterSoundFunctions();
    static void RegisterMathFunctions();
    static int LuaPanic(lua_State* L);
};

// Global instance
extern CGModLuaSystem g_GMod_LuaSystem;

// Console command handlers - discovered from GMod directory analysis
void CMD_gmod_runfunction(void);
void CMD_gmod_lua_run(void);
void CMD_gmod_lua_reload(void);
void CMD_gmod_lua_load(void);
void CMD_gmod_lua_list(void);
void CMD_gmod_lua_errors(void);
void CMD_gmod_lua_clear(void);

// Gamemode commands
void CMD_gmod_gamemode(void);
void CMD_gmod_gamemode_list(void);
void CMD_gmod_gamemode_reload(void);

// SWEP commands
void CMD_gmod_swep_list(void);
void CMD_gmod_swep_reload(void);
void CMD_gmod_swep_give(void);

//-----------------------------------------------------------------------------
// Lua Engine Bindings - Functions exposed to Lua scripts
// These match the functions discovered in GMod SWEP/gamemode analysis
//-----------------------------------------------------------------------------

// Player functions
extern "C" {
    // Player info and control
    int lua_PlayerInfo(lua_State* L);           // _PlayerInfo(playerid, "info_type")
    int lua_PlayerGetShootPos(lua_State* L);    // _PlayerGetShootPos(playerid)
    int lua_PlayerGetShootAng(lua_State* L);    // _PlayerGetShootAng(playerid)
    int lua_PlayerFreeze(lua_State* L);         // _PlayerFreeze(playerid, freeze)
    int lua_PrintMessage(lua_State* L);         // _PrintMessage(playerid, type, message)
    int lua_PlayerAllowDecalPaint(lua_State* L); // _PlayerAllowDecalPaint(playerid)

    // Entity functions
    int lua_EntCreate(lua_State* L);            // _EntCreate(classname)
    int lua_EntSetKeyValue(lua_State* L);       // _EntSetKeyValue(entid, key, value)
    int lua_EntSetPos(lua_State* L);            // _EntSetPos(entid, vector)
    int lua_EntSetAng(lua_State* L);            // _EntSetAng(entid, angle)
    int lua_EntSpawn(lua_State* L);             // _EntSpawn(entid)
    int lua_EntRemove(lua_State* L);            // _EntRemove(entid)

    // Trace functions
    int lua_TraceLine(lua_State* L);            // _TraceLine(start, direction, distance, ignore)
    int lua_TraceEndPos(lua_State* L);          // _TraceEndPos()
    int lua_TraceHit(lua_State* L);             // _TraceHit()
    int lua_TraceHitWorld(lua_State* L);        // _TraceHitWorld()

    // Physics functions
    int lua_PhysSetMass(lua_State* L);          // _PhysSetMass(entid, mass)
    int lua_PhysGetMass(lua_State* L);          // _PhysGetMass(entid)
    int lua_PhysSetVelocity(lua_State* L);      // _PhysSetVelocity(entid, velocity)

    // SWEP functions
    int lua_SWEPSetSound(lua_State* L);         // _SWEPSetSound(weaponid, slot, sound)

    // Utility functions
    int lua_RunString(lua_State* L);            // _RunString(code)
    int lua_CurTime(lua_State* L);              // _CurTime()
    int lua_Msg(lua_State* L);                  // _Msg(message)
    int lua_OpenScript(lua_State* L);           // _OpenScript(filename)

    // ConVar functions
    int lua_GetConVar_Float(lua_State* L);      // _GetConVar_Float(name)
    int lua_GetConVar_Int(lua_State* L);        // _GetConVar_Int(name)
    int lua_GetConVar_String(lua_State* L);     // _GetConVar_String(name)
    int lua_SetConVar(lua_State* L);            // _SetConVar(name, value)

    // Game functions
    int lua_StartNextLevel(lua_State* L);       // _StartNextLevel()

    // Vector functions (vector3 support from includes/vector3.lua)
    int lua_vector3(lua_State* L);              // vector3(x, y, z)
    int lua_vecAdd(lua_State* L);               // vecAdd(v1, v2)
    int lua_vecSub(lua_State* L);               // vecSub(v1, v2)
    int lua_vecMul(lua_State* L);               // vecMul(v, scalar)
    int lua_vecLength(lua_State* L);            // vecLength(v)
    int lua_vecNormalize(lua_State* L);         // vecNormalize(v)
}

#endif // GMOD_LUA_H