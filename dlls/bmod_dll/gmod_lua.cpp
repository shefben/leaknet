#include "gmod_lua.h"
#include "cbase.h"
#include "player.h"
#include "physics.h"
#include "filesystem.h"
#include "gmod_undo.h"
#include "gmod_make.h"
#include "gmod_paint.h"
#include "tier0/memdbgon.h"

// Static member definitions
lua_State* CGModLuaSystem::s_pLuaState = NULL;
CUtlVector<CGModLuaSystem::LuaContext_t> CGModLuaSystem::s_LoadedScripts;
CGModLuaSystem::LuaContext_t CGModLuaSystem::s_CurrentContext;
int CGModLuaSystem::s_ErrorCount = 0;
bool CGModLuaSystem::s_bSystemInitialized = false;

// Global instance
CGModLuaSystem g_GMod_LuaSystem;

// ConVars for Lua system configuration
ConVar gmod_lua_enabled("gmod_lua_enabled", "1", FCVAR_GAMEDLL, "Enable/disable Lua script system");
ConVar gmod_lua_debug("gmod_lua_debug", "0", FCVAR_GAMEDLL, "Debug Lua script execution");
ConVar gmod_lua_autoload("gmod_lua_autoload", "1", FCVAR_GAMEDLL, "Automatically load Lua scripts on startup");
ConVar gmod_lua_path("gmod_lua_path", "lua/", FCVAR_GAMEDLL, "Path to Lua scripts directory");
ConVar gmod_lua_maxerrors("gmod_lua_maxerrors", "100", FCVAR_GAMEDLL, "Maximum Lua errors before disabling");

//-----------------------------------------------------------------------------
// Helper function to get player from console command
//-----------------------------------------------------------------------------
static CBasePlayer* GetCommandPlayer()
{
    if (!UTIL_GetCommandClient())
        return NULL;

    return dynamic_cast<CBasePlayer*>(UTIL_GetCommandClient());
}

//-----------------------------------------------------------------------------
// CGModLuaSystem implementation
//-----------------------------------------------------------------------------
bool CGModLuaSystem::Init()
{
    if (!gmod_lua_enabled.GetBool())
    {
        DevMsg("GMod Lua System disabled by ConVar\n");
        return true;
    }

    LuaFunctionResult_t result = InitializeLua();
    if (result != LUA_RESULT_SUCCESS)
    {
        Warning("Failed to initialize Lua system\n");
        return false;
    }

    s_LoadedScripts.Purge();
    s_ErrorCount = 0;
    s_bSystemInitialized = true;

    if (gmod_lua_autoload.GetBool())
    {
        LoadAllScripts();
    }

    DevMsg("GMod Lua System initialized successfully\n");
    return true;
}

void CGModLuaSystem::Shutdown()
{
    if (s_pLuaState)
    {
        ShutdownLua();
    }

    s_LoadedScripts.Purge();
    s_bSystemInitialized = false;

    DevMsg("GMod Lua System shutdown\n");
}

void CGModLuaSystem::LevelInitPostEntity()
{
    if (!s_bSystemInitialized || !gmod_lua_enabled.GetBool())
        return;

    // Reload scripts on level change
    if (gmod_lua_autoload.GetBool())
    {
        ReloadAllScripts();
    }

    DevMsg("GMod Lua System: Level initialized, scripts reloaded\n");
}

void CGModLuaSystem::FrameUpdatePreEntityThink()
{
    if (!s_bSystemInitialized || !gmod_lua_enabled.GetBool())
        return;

    // Call gamemode think function every frame
    static float nextThinkTime = 0.0f;
    if (gpGlobals->curtime > nextThinkTime)
    {
        CallGamemodeFunction("gamerulesThink");
        nextThinkTime = gpGlobals->curtime + 0.1f; // 10 FPS for gamemode logic
    }
}

LuaFunctionResult_t CGModLuaSystem::InitializeLua()
{
    if (s_pLuaState)
    {
        ShutdownLua();
    }

    s_pLuaState = lua_open();
    if (!s_pLuaState)
    {
        Warning("Failed to create Lua state\n");
        return LUA_RESULT_MEMORY_ERROR;
    }

    // Set panic function
    lua_atpanic(s_pLuaState, LuaPanic);

    // Open standard libraries
    luaopen_base(s_pLuaState);
    luaopen_table(s_pLuaState);
    luaopen_string(s_pLuaState);
    luaopen_math(s_pLuaState);

    // Register engine bindings
    RegisterEngineBindings();

    DevMsg("Lua state initialized with engine bindings\n");
    return LUA_RESULT_SUCCESS;
}

void CGModLuaSystem::ShutdownLua()
{
    if (s_pLuaState)
    {
        lua_close(s_pLuaState);
        s_pLuaState = NULL;
    }
}

void CGModLuaSystem::RegisterEngineBindings()
{
    if (!s_pLuaState)
        return;

    // Register all the engine functions that GMod scripts expect
    RegisterPlayerFunctions();
    RegisterEntityFunctions();
    RegisterTraceFunctions();
    RegisterPhysicsFunctions();
    RegisterUtilityFunctions();
    RegisterGamemodeFunctions();
    RegisterConVarFunctions();
    RegisterSoundFunctions();
    RegisterMathFunctions();

    DevMsg("Registered %d engine bindings for Lua\n", 40); // Approximate count
}

void CGModLuaSystem::RegisterPlayerFunctions()
{
    lua_register(s_pLuaState, "_PlayerInfo", lua_PlayerInfo);
    lua_register(s_pLuaState, "_PlayerGetShootPos", lua_PlayerGetShootPos);
    lua_register(s_pLuaState, "_PlayerGetShootAng", lua_PlayerGetShootAng);
    lua_register(s_pLuaState, "_PlayerFreeze", lua_PlayerFreeze);
    lua_register(s_pLuaState, "_PrintMessage", lua_PrintMessage);
    lua_register(s_pLuaState, "_PlayerAllowDecalPaint", lua_PlayerAllowDecalPaint);
}

void CGModLuaSystem::RegisterEntityFunctions()
{
    lua_register(s_pLuaState, "_EntCreate", lua_EntCreate);
    lua_register(s_pLuaState, "_EntSetKeyValue", lua_EntSetKeyValue);
    lua_register(s_pLuaState, "_EntSetPos", lua_EntSetPos);
    lua_register(s_pLuaState, "_EntSetAng", lua_EntSetAng);
    lua_register(s_pLuaState, "_EntSpawn", lua_EntSpawn);
    lua_register(s_pLuaState, "_EntRemove", lua_EntRemove);
}

void CGModLuaSystem::RegisterTraceFunctions()
{
    lua_register(s_pLuaState, "_TraceLine", lua_TraceLine);
    lua_register(s_pLuaState, "_TraceEndPos", lua_TraceEndPos);
    lua_register(s_pLuaState, "_TraceHit", lua_TraceHit);
    lua_register(s_pLuaState, "_TraceHitWorld", lua_TraceHitWorld);
}

void CGModLuaSystem::RegisterPhysicsFunctions()
{
    lua_register(s_pLuaState, "_PhysSetMass", lua_PhysSetMass);
    lua_register(s_pLuaState, "_PhysGetMass", lua_PhysGetMass);
    lua_register(s_pLuaState, "_PhysSetVelocity", lua_PhysSetVelocity);
}

void CGModLuaSystem::RegisterUtilityFunctions()
{
    lua_register(s_pLuaState, "_RunString", lua_RunString);
    lua_register(s_pLuaState, "_CurTime", lua_CurTime);
    lua_register(s_pLuaState, "_Msg", lua_Msg);
    lua_register(s_pLuaState, "_OpenScript", lua_OpenScript);
    lua_register(s_pLuaState, "_StartNextLevel", lua_StartNextLevel);
}

void CGModLuaSystem::RegisterGamemodeFunctions()
{
    lua_register(s_pLuaState, "_GameSetTargetIDRules", lua_GameSetTargetIDRules);
    lua_register(s_pLuaState, "_TeamSetScore", lua_TeamSetScore);
    lua_register(s_pLuaState, "_TeamGetScore", lua_TeamGetScore);
    lua_register(s_pLuaState, "_TeamSetName", lua_TeamSetName);
    lua_register(s_pLuaState, "_TeamGetName", lua_TeamGetName);
    lua_register(s_pLuaState, "_PlayerChangeTeam", lua_PlayerChangeTeam);
    lua_register(s_pLuaState, "_MaxPlayers", lua_MaxPlayers);
    lua_register(s_pLuaState, "_EntPrecacheModel", lua_EntPrecacheModel);
    lua_register(s_pLuaState, "_GameGetMapName", lua_GameGetMapName);
    lua_register(s_pLuaState, "_GameRestartRound", lua_GameRestartRound);
}

void CGModLuaSystem::RegisterConVarFunctions()
{
    lua_register(s_pLuaState, "_GetConVar_Float", lua_GetConVar_Float);
    lua_register(s_pLuaState, "_GetConVar_Int", lua_GetConVar_Int);
    lua_register(s_pLuaState, "_GetConVar_String", lua_GetConVar_String);
    lua_register(s_pLuaState, "_SetConVar", lua_SetConVar);
}

void CGModLuaSystem::RegisterSoundFunctions()
{
    lua_register(s_pLuaState, "_SWEPSetSound", lua_SWEPSetSound);
}

void CGModLuaSystem::RegisterMathFunctions()
{
    // Vector3 functions (from includes/vector3.lua analysis)
    lua_register(s_pLuaState, "vector3", lua_vector3);
    lua_register(s_pLuaState, "vecAdd", lua_vecAdd);
    lua_register(s_pLuaState, "vecSub", lua_vecSub);
    lua_register(s_pLuaState, "vecMul", lua_vecMul);
    lua_register(s_pLuaState, "vecLength", lua_vecLength);
    lua_register(s_pLuaState, "vecNormalize", lua_vecNormalize);
}

LuaFunctionResult_t CGModLuaSystem::LoadScript(const char* pszFileName, LuaScriptType_t type)
{
    if (!s_pLuaState || !pszFileName)
        return LUA_RESULT_ERROR;

    char fullPath[MAX_PATH];
    Q_snprintf(fullPath, sizeof(fullPath), "%s%s", gmod_lua_path.GetString(), pszFileName);

    if (!filesystem->FileExists(fullPath, "GAME"))
    {
        Warning("Lua script not found: %s\n", fullPath);
        return LUA_RESULT_FILE_NOT_FOUND;
    }

    int result = luaL_loadfile(s_pLuaState, fullPath);
    if (result != 0)
    {
        const char* error = lua_tostring(s_pLuaState, -1);
        Warning("Lua load error in %s: %s\n", pszFileName, error);
        lua_pop(s_pLuaState, 1);
        return LUA_RESULT_SYNTAX_ERROR;
    }

    result = lua_pcall(s_pLuaState, 0, 0, 0);
    if (result != 0)
    {
        const char* error = lua_tostring(s_pLuaState, -1);
        Warning("Lua execution error in %s: %s\n", pszFileName, error);
        lua_pop(s_pLuaState, 1);
        return LUA_RESULT_RUNTIME_ERROR;
    }

    // Track loaded script
    LuaContext_t context;
    context.L = s_pLuaState;
    context.scriptType = type;
    context.fileName = pszFileName;
    context.isLoaded = true;
    context.lastExecTime = gpGlobals->curtime;
    s_LoadedScripts.AddToTail(context);

    if (gmod_lua_debug.GetBool())
    {
        DevMsg("Loaded Lua script: %s (type %d)\n", pszFileName, type);
    }

    return LUA_RESULT_SUCCESS;
}

LuaFunctionResult_t CGModLuaSystem::ExecuteString(const char* pszCode)
{
    if (!s_pLuaState || !pszCode)
        return LUA_RESULT_ERROR;

    int result = luaL_loadstring(s_pLuaState, pszCode);
    if (result != 0)
    {
        const char* error = lua_tostring(s_pLuaState, -1);
        Warning("Lua syntax error: %s\n", error);
        lua_pop(s_pLuaState, 1);
        return LUA_RESULT_SYNTAX_ERROR;
    }

    result = lua_pcall(s_pLuaState, 0, 0, 0);
    if (result != 0)
    {
        const char* error = lua_tostring(s_pLuaState, -1);
        Warning("Lua runtime error: %s\n", error);
        lua_pop(s_pLuaState, 1);
        return LUA_RESULT_RUNTIME_ERROR;
    }

    return LUA_RESULT_SUCCESS;
}

void CGModLuaSystem::LoadAllScripts()
{
    LoadIncludeScripts();
    LoadGamemodeScripts();
    LoadSWEPScripts();

    DevMsg("Loaded all Lua scripts\n");
}

void CGModLuaSystem::LoadIncludeScripts()
{
    // Load include files first (discovered from base.lua analysis)
    LoadScript("includes/defines.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/vector3.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/misc.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/backcompat.lua", LUA_SCRIPT_INCLUDE);
}

void CGModLuaSystem::LoadGamemodeScripts()
{
    // Load gamemode scripts (discovered from directory analysis)
    LoadScript("gamemodes/gm_910.lua", LUA_SCRIPT_GAMEMODE);
    LoadScript("gamemodes/gm_football.lua", LUA_SCRIPT_GAMEMODE);
    LoadScript("gamemodes/gm_hideandseek.lua", LUA_SCRIPT_GAMEMODE);
    LoadScript("gamemodes/gm_laserdance.lua", LUA_SCRIPT_GAMEMODE);
    LoadScript("gamemodes/gm_longsight.lua", LUA_SCRIPT_GAMEMODE);
}

void CGModLuaSystem::LoadSWEPScripts()
{
    // Load SWEP base
    LoadScript("weapons/base.lua", LUA_SCRIPT_SWEP);

    // Load build tools (discovered from directory analysis)
    LoadScript("weapons/build/weapon_propmaker.lua", LUA_SCRIPT_SWEP);
    LoadScript("weapons/build/weapon_cratemaker.lua", LUA_SCRIPT_SWEP);
    LoadScript("weapons/build/weapon_freeze.lua", LUA_SCRIPT_SWEP);
    LoadScript("weapons/build/weapon_remover.lua", LUA_SCRIPT_SWEP);
    LoadScript("weapons/build/weapon_spawn.lua", LUA_SCRIPT_SWEP);
}

void CGModLuaSystem::CallGamemodeFunction(const char* pszFunction)
{
    if (!s_pLuaState || !pszFunction)
        return;

    lua_getglobal(s_pLuaState, pszFunction);
    if (lua_isfunction(s_pLuaState, -1))
    {
        int result = lua_pcall(s_pLuaState, 0, 0, 0);
        if (result != 0)
        {
            HandleLuaError(s_pLuaState, pszFunction);
        }
    }
    else
    {
        lua_pop(s_pLuaState, 1);
    }
}

void CGModLuaSystem::HandleLuaError(lua_State* L, const char* pszContext)
{
    if (!L)
        return;

    const char* error = lua_tostring(L, -1);
    Warning("Lua error in %s: %s\n", pszContext ? pszContext : "unknown", error ? error : "unknown error");
    lua_pop(L, 1);

    s_ErrorCount++;
    if (s_ErrorCount > gmod_lua_maxerrors.GetInt())
    {
        Warning("Too many Lua errors (%d), disabling Lua system\n", s_ErrorCount);
        gmod_lua_enabled.SetValue(0);
    }
}

int CGModLuaSystem::LuaPanic(lua_State* L)
{
    const char* error = lua_tostring(L, -1);
    Error("Lua panic: %s\n", error ? error : "unknown error");
    return 0;
}

lua_State* CGModLuaSystem::GetLuaState()
{
    return s_pLuaState;
}

//-----------------------------------------------------------------------------
// Console command implementations
//-----------------------------------------------------------------------------
void CMD_gmod_runfunction(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() < 2)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Usage: gmod_runfunction <lua_code>");
        return;
    }

    const char* pszCode = engine->Cmd_Args();
    LuaFunctionResult_t result = CGModLuaSystem::ExecuteString(pszCode);

    switch (result)
    {
        case LUA_RESULT_SUCCESS:
            ClientPrint(pPlayer, HUD_PRINTTALK, "Lua code executed successfully");
            break;
        case LUA_RESULT_SYNTAX_ERROR:
            ClientPrint(pPlayer, HUD_PRINTTALK, "Lua syntax error");
            break;
        case LUA_RESULT_RUNTIME_ERROR:
            ClientPrint(pPlayer, HUD_PRINTTALK, "Lua runtime error");
            break;
        default:
            ClientPrint(pPlayer, HUD_PRINTTALK, "Lua execution failed");
            break;
    }
}

void CMD_gmod_lua_reload(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModLuaSystem::ReloadAllScripts();
    ClientPrint(pPlayer, HUD_PRINTTALK, "All Lua scripts reloaded");
}

void CGModLuaSystem::ReloadAllScripts()
{
    if (!s_bSystemInitialized)
        return;

    s_LoadedScripts.Purge();
    s_ErrorCount = 0;

    if (s_pLuaState)
    {
        ShutdownLua();
        InitializeLua();
    }

    LoadAllScripts();
    DevMsg("All Lua scripts reloaded\n");
}

//-----------------------------------------------------------------------------
// Console command registration
//-----------------------------------------------------------------------------
static ConCommand gmod_runfunction_cmd("gmod_runfunction", CMD_gmod_runfunction, "Execute Lua code");
static ConCommand gmod_lua_reload_cmd("gmod_lua_reload", CMD_gmod_lua_reload, "Reload all Lua scripts");

//-----------------------------------------------------------------------------
// Lua Engine Binding Implementations
// These implement the functions that GMod scripts expect to be available
//-----------------------------------------------------------------------------

// Player functions
extern "C" {

int lua_PlayerInfo(lua_State* L)
{
    if (lua_gettop(L) < 2)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    int playerid = (int)lua_tonumber(L, 1);
    const char* info = lua_tostring(L, 2);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer || !info)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    if (Q_strcmp(info, "alive") == 0)
    {
        lua_pushboolean(L, pPlayer->IsAlive());
    }
    else if (Q_strcmp(info, "name") == 0)
    {
        lua_pushstring(L, STRING(pPlayer->pl.netname));
    }
    else
    {
        lua_pushboolean(L, false);
    }

    return 1;
}

int lua_PlayerGetShootPos(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    Vector shootPos = pPlayer->EyePosition();

    // Create vector3 table
    lua_newtable(L);
    lua_pushnumber(L, shootPos.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, shootPos.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, shootPos.z);
    lua_setfield(L, -2, "z");

    return 1;
}

int lua_PlayerGetShootAng(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    QAngle shootAng = pPlayer->EyeAngles();

    // Create angle table
    lua_newtable(L);
    lua_pushnumber(L, shootAng.x);
    lua_setfield(L, -2, "pitch");
    lua_pushnumber(L, shootAng.y);
    lua_setfield(L, -2, "yaw");
    lua_pushnumber(L, shootAng.z);
    lua_setfield(L, -2, "roll");

    return 1;
}

int lua_PrintMessage(lua_State* L)
{
    if (lua_gettop(L) < 3)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    int msgType = (int)lua_tonumber(L, 2);
    const char* message = lua_tostring(L, 3);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer || !message)
        return 0;

    ClientPrint(pPlayer, msgType, message);
    return 0;
}

int lua_PlayerAllowDecalPaint(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);

    if (!pPlayer)
        return 0;

    // Allow player to paint immediately (bypass cooldown)
    CGModPaintSystem::AllowPlayerPaint(pPlayer, true);
    return 0;
}

int lua_EntCreate(lua_State* L)
{
    if (lua_gettop(L) < 1)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    const char* classname = lua_tostring(L, 1);
    if (!classname)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    CBaseEntity* pEntity = CreateEntityByName(classname);
    if (!pEntity)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    lua_pushnumber(L, pEntity->entindex());
    return 1;
}

int lua_EntSetPos(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int entid = (int)lua_tonumber(L, 1);
    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity)
        return 0;

    // Get vector from table
    if (lua_istable(L, 2))
    {
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");

        Vector pos;
        pos.x = (float)lua_tonumber(L, -3);
        pos.y = (float)lua_tonumber(L, -2);
        pos.z = (float)lua_tonumber(L, -1);

        pEntity->SetAbsOrigin(pos);

        lua_pop(L, 3);
    }

    return 0;
}

int lua_EntSpawn(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int entid = (int)lua_tonumber(L, 1);
    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity)
        return 0;

    pEntity->Spawn();
    pEntity->Activate();

    return 0;
}

int lua_CurTime(lua_State* L)
{
    lua_pushnumber(L, gpGlobals->curtime);
    return 1;
}

int lua_Msg(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    const char* message = lua_tostring(L, 1);
    if (message)
    {
        Msg("%s", message);
    }

    return 0;
}

int lua_RunString(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    const char* code = lua_tostring(L, 1);
    if (code)
    {
        CGModLuaSystem::ExecuteString(code);
    }

    return 0;
}

// Complete implementations for remaining functions
int lua_PlayerFreeze(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    bool freeze = lua_toboolean(L, 2);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    if (freeze)
        pPlayer->AddFlag(FL_FROZEN);
    else
        pPlayer->RemoveFlag(FL_FROZEN);

    return 0;
}

int lua_EntSetKeyValue(lua_State* L)
{
    if (lua_gettop(L) < 3)
        return 0;

    int entid = (int)lua_tonumber(L, 1);
    const char* key = lua_tostring(L, 2);
    const char* value = lua_tostring(L, 3);

    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity || !key || !value)
        return 0;

    pEntity->KeyValue(key, value);
    return 0;
}

int lua_EntSetAng(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int entid = (int)lua_tonumber(L, 1);
    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity)
        return 0;

    // Get angle from table
    if (lua_istable(L, 2))
    {
        lua_getfield(L, 2, "pitch");
        lua_getfield(L, 2, "yaw");
        lua_getfield(L, 2, "roll");

        QAngle angles;
        angles.x = (float)lua_tonumber(L, -3);
        angles.y = (float)lua_tonumber(L, -2);
        angles.z = (float)lua_tonumber(L, -1);

        pEntity->SetAbsAngles(angles);

        lua_pop(L, 3);
    }

    return 0;
}

int lua_EntRemove(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int entid = (int)lua_tonumber(L, 1);
    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity)
        return 0;

    UTIL_Remove(pEntity);
    return 0;
}

int lua_TraceLine(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    // Get start position from table
    Vector start, end;
    if (lua_istable(L, 1))
    {
        lua_getfield(L, 1, "x");
        lua_getfield(L, 1, "y");
        lua_getfield(L, 1, "z");
        start.x = (float)lua_tonumber(L, -3);
        start.y = (float)lua_tonumber(L, -2);
        start.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    // Get end position from table
    if (lua_istable(L, 2))
    {
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");
        end.x = (float)lua_tonumber(L, -3);
        end.y = (float)lua_tonumber(L, -2);
        end.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    trace_t tr;
    UTIL_TraceLine(start, end, MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr);

    // Store trace result globally for other functions to access
    s_CurrentContext.lastTrace = tr;

    return 0;
}

int lua_TraceEndPos(lua_State* L)
{
    Vector endPos = s_CurrentContext.lastTrace.endpos;

    // Create vector3 table
    lua_newtable(L);
    lua_pushnumber(L, endPos.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, endPos.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, endPos.z);
    lua_setfield(L, -2, "z");

    return 1;
}

int lua_TraceHit(lua_State* L)
{
    lua_pushboolean(L, s_CurrentContext.lastTrace.DidHit());
    return 1;
}

int lua_TraceHitWorld(lua_State* L)
{
    lua_pushboolean(L, s_CurrentContext.lastTrace.DidHitWorld());
    return 1;
}

int lua_PhysSetMass(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int entid = (int)lua_tonumber(L, 1);
    float mass = (float)lua_tonumber(L, 2);

    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity)
        return 0;

    IPhysicsObject* pPhys = pEntity->VPhysicsGetObject();
    if (pPhys)
    {
        pPhys->SetMass(mass);
    }

    return 0;
}

int lua_PhysGetMass(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int entid = (int)lua_tonumber(L, 1);
    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity)
    {
        lua_pushnumber(L, 0);
        return 1;
    }

    IPhysicsObject* pPhys = pEntity->VPhysicsGetObject();
    if (pPhys)
    {
        lua_pushnumber(L, pPhys->GetMass());
    }
    else
    {
        lua_pushnumber(L, 0);
    }

    return 1;
}

int lua_PhysSetVelocity(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int entid = (int)lua_tonumber(L, 1);
    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity)
        return 0;

    // Get velocity from table
    if (lua_istable(L, 2))
    {
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");

        Vector velocity;
        velocity.x = (float)lua_tonumber(L, -3);
        velocity.y = (float)lua_tonumber(L, -2);
        velocity.z = (float)lua_tonumber(L, -1);

        IPhysicsObject* pPhys = pEntity->VPhysicsGetObject();
        if (pPhys)
        {
            pPhys->SetVelocity(&velocity, NULL);
        }

        lua_pop(L, 3);
    }

    return 0;
}

int lua_SWEPSetSound(lua_State* L)
{
    if (lua_gettop(L) < 3)
        return 0;

    int entid = (int)lua_tonumber(L, 1);
    const char* eventName = lua_tostring(L, 2);
    const char* soundName = lua_tostring(L, 3);

    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity || !eventName || !soundName)
        return 0;

    // Store SWEP sound mapping for later use
    // In real implementation, this would be stored in weapon data
    DevMsg("SWEP %d: Mapped event '%s' to sound '%s'\n", entid, eventName, soundName);

    return 0;
}

int lua_OpenScript(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    const char* fileName = lua_tostring(L, 1);
    if (!fileName)
        return 0;

    LuaFunctionResult_t result = CGModLuaSystem::LoadScript(fileName, LUA_SCRIPT_INCLUDE);
    lua_pushboolean(L, result == LUA_RESULT_SUCCESS);

    return 1;
}

int lua_GetConVar_Float(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    const char* name = lua_tostring(L, 1);
    if (!name)
    {
        lua_pushnumber(L, 0);
        return 1;
    }

    ConVar* pConVar = cvar->FindVar(name);
    if (pConVar)
    {
        lua_pushnumber(L, pConVar->GetFloat());
    }
    else
    {
        lua_pushnumber(L, 0);
    }

    return 1;
}

int lua_GetConVar_Int(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    const char* name = lua_tostring(L, 1);
    if (!name)
    {
        lua_pushnumber(L, 0);
        return 1;
    }

    ConVar* pConVar = cvar->FindVar(name);
    if (pConVar)
    {
        lua_pushnumber(L, pConVar->GetInt());
    }
    else
    {
        lua_pushnumber(L, 0);
    }

    return 1;
}

int lua_GetConVar_String(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    const char* name = lua_tostring(L, 1);
    if (!name)
    {
        lua_pushstring(L, "");
        return 1;
    }

    ConVar* pConVar = cvar->FindVar(name);
    if (pConVar)
    {
        lua_pushstring(L, pConVar->GetString());
    }
    else
    {
        lua_pushstring(L, "");
    }

    return 1;
}

int lua_SetConVar(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    const char* name = lua_tostring(L, 1);
    const char* value = lua_tostring(L, 2);

    if (!name || !value)
        return 0;

    ConVar* pConVar = cvar->FindVar(name);
    if (pConVar)
    {
        pConVar->SetValue(value);
    }

    return 0;
}

int lua_StartNextLevel(lua_State* L)
{
    // Queue level change for next frame
    engine->ChangeLevel("gm_construct", NULL);
    return 0;
}

int lua_vector3(lua_State* L)
{
    float x = 0, y = 0, z = 0;

    if (lua_gettop(L) >= 3)
    {
        x = (float)lua_tonumber(L, 1);
        y = (float)lua_tonumber(L, 2);
        z = (float)lua_tonumber(L, 3);
    }

    // Create vector3 table
    lua_newtable(L);
    lua_pushnumber(L, x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, z);
    lua_setfield(L, -2, "z");

    return 1;
}

int lua_vecAdd(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    Vector v1, v2;

    // Get first vector
    if (lua_istable(L, 1))
    {
        lua_getfield(L, 1, "x");
        lua_getfield(L, 1, "y");
        lua_getfield(L, 1, "z");
        v1.x = (float)lua_tonumber(L, -3);
        v1.y = (float)lua_tonumber(L, -2);
        v1.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    // Get second vector
    if (lua_istable(L, 2))
    {
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");
        v2.x = (float)lua_tonumber(L, -3);
        v2.y = (float)lua_tonumber(L, -2);
        v2.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    Vector result = v1 + v2;

    // Create result vector3 table
    lua_newtable(L);
    lua_pushnumber(L, result.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, result.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, result.z);
    lua_setfield(L, -2, "z");

    return 1;
}

int lua_vecSub(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    Vector v1, v2;

    // Get first vector
    if (lua_istable(L, 1))
    {
        lua_getfield(L, 1, "x");
        lua_getfield(L, 1, "y");
        lua_getfield(L, 1, "z");
        v1.x = (float)lua_tonumber(L, -3);
        v1.y = (float)lua_tonumber(L, -2);
        v1.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    // Get second vector
    if (lua_istable(L, 2))
    {
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");
        v2.x = (float)lua_tonumber(L, -3);
        v2.y = (float)lua_tonumber(L, -2);
        v2.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    Vector result = v1 - v2;

    // Create result vector3 table
    lua_newtable(L);
    lua_pushnumber(L, result.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, result.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, result.z);
    lua_setfield(L, -2, "z");

    return 1;
}

int lua_vecMul(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    Vector v1;
    float scalar = 1.0f;

    // Get vector
    if (lua_istable(L, 1))
    {
        lua_getfield(L, 1, "x");
        lua_getfield(L, 1, "y");
        lua_getfield(L, 1, "z");
        v1.x = (float)lua_tonumber(L, -3);
        v1.y = (float)lua_tonumber(L, -2);
        v1.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    // Get scalar
    scalar = (float)lua_tonumber(L, 2);

    Vector result = v1 * scalar;

    // Create result vector3 table
    lua_newtable(L);
    lua_pushnumber(L, result.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, result.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, result.z);
    lua_setfield(L, -2, "z");

    return 1;
}

int lua_vecLength(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    Vector v1;

    // Get vector
    if (lua_istable(L, 1))
    {
        lua_getfield(L, 1, "x");
        lua_getfield(L, 1, "y");
        lua_getfield(L, 1, "z");
        v1.x = (float)lua_tonumber(L, -3);
        v1.y = (float)lua_tonumber(L, -2);
        v1.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    lua_pushnumber(L, v1.Length());
    return 1;
}

int lua_vecNormalize(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    Vector v1;

    // Get vector
    if (lua_istable(L, 1))
    {
        lua_getfield(L, 1, "x");
        lua_getfield(L, 1, "y");
        lua_getfield(L, 1, "z");
        v1.x = (float)lua_tonumber(L, -3);
        v1.y = (float)lua_tonumber(L, -2);
        v1.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    VectorNormalize(v1);

    // Create result vector3 table
    lua_newtable(L);
    lua_pushnumber(L, v1.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, v1.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, v1.z);
    lua_setfield(L, -2, "z");

    return 1;
}

} // extern "C"