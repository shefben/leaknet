#include "gmod_lua.h"
#include "cbase.h"
#include "player.h"
#include "physics.h"
#include "filesystem.h"
#include "gmod_undo.h"
#include "gmod_make.h"
#include "gmod_paint.h"
#include "gmod_gamemode.h"
#include "ai_basenpc.h"
#include "shake.h"
#include "igameevents.h"
#include "usermessages.h"
#include "tier0/memdbgon.h"

namespace
{
    struct LuaBinding
    {
        const char* name;
        lua_CFunction function;
    };

    template <size_t N>
    int RegisterLuaBindings(lua_State* L, const LuaBinding (&bindings)[N])
    {
        if (!L)
            return 0;

        for (const auto& binding : bindings)
        {
            lua_register(L, binding.name, binding.function);
        }

        return static_cast<int>(N);
    }
}

// Static member definitions
lua_State* CGModLuaSystem::s_pLuaState = NULL;
CUtlVector<LuaContext_t> CGModLuaSystem::s_LoadedScripts;
LuaContext_t CGModLuaSystem::s_CurrentContext;
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

    // Call think functions every frame
    // This matches GMod server.dll's DoLuaThinkFunctions behavior
    if (s_pLuaState)
    {
        // Call DoLuaThinkFunctions - iterates ThinkFunctions table and calls each
        CallGamemodeFunction("DoLuaThinkFunctions");
    }

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

    // Use our wrapper to initialize Lua
    if (!CLuaWrapper::InitializeLua())
    {
        Warning("Failed to initialize Lua wrapper\n");
        return LUA_RESULT_MEMORY_ERROR;
    }

    s_pLuaState = CLuaWrapper::GetLuaState();

    // Register engine bindings
    RegisterEngineBindings();

    DevMsg("Lua state initialized with engine bindings\n");
    return LUA_RESULT_SUCCESS;
}

void CGModLuaSystem::ShutdownLua()
{
    if (s_pLuaState)
    {
        CLuaWrapper::ShutdownLua();
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

    // Register additional GMod 9 global tables
    RegisterUtilTable();
    RegisterPlayerTable();
    RegisterNPCTable();
    RegisterSpawnMenuTable();
    RegisterGModQuadFunctions();
    RegisterGameEventTable();
    RegisterGModTextFunctions();
    RegisterGModRectFunctions();

    DevMsg("Registered engine bindings for Lua\n");
}

void CGModLuaSystem::RegisterPlayerFunctions()
{
    static const LuaBinding bindings[] = {
        {"_PlayerInfo", lua_PlayerInfo},
        {"_PlayerGetShootPos", lua_PlayerGetShootPos},
        {"_PlayerGetShootAng", lua_PlayerGetShootAng},
        {"_PlayerFreeze", lua_PlayerFreeze},
        {"_PrintMessage", lua_PrintMessage},
        {"_PlayerAllowDecalPaint", lua_PlayerAllowDecalPaint},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d player functions\n", registered);
}

void CGModLuaSystem::RegisterEntityFunctions()
{
    static const LuaBinding bindings[] = {
        {"_EntCreate", lua_EntCreate},
        {"_EntSetKeyValue", lua_EntSetKeyValue},
        {"_EntSetPos", lua_EntSetPos},
        {"_EntSetAng", lua_EntSetAng},
        {"_EntSpawn", lua_EntSpawn},
        {"_EntRemove", lua_EntRemove},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d entity functions\n", registered);
}

void CGModLuaSystem::RegisterTraceFunctions()
{
    static const LuaBinding bindings[] = {
        {"_TraceLine", lua_TraceLine},
        {"_TraceEndPos", lua_TraceEndPos},
        {"_TraceHit", lua_TraceHit},
        {"_TraceHitWorld", lua_TraceHitWorld},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d trace functions\n", registered);
}

void CGModLuaSystem::RegisterPhysicsFunctions()
{
    static const LuaBinding bindings[] = {
        {"_PhysSetMass", lua_PhysSetMass},
        {"_PhysGetMass", lua_PhysGetMass},
        {"_PhysSetVelocity", lua_PhysSetVelocity},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);

    // Register the _phys global table (GMod 9 style)
    // This creates _phys.HasPhysics, _phys.Wake, etc.
    lua_newtable(s_pLuaState);

    lua_pushcfunction(s_pLuaState, lua_phys_HasPhysics);
    lua_setfield(s_pLuaState, -2, "HasPhysics");

    lua_pushcfunction(s_pLuaState, lua_phys_IsAsleep);
    lua_setfield(s_pLuaState, -2, "IsAsleep");

    lua_pushcfunction(s_pLuaState, lua_phys_Wake);
    lua_setfield(s_pLuaState, -2, "Wake");

    lua_pushcfunction(s_pLuaState, lua_phys_Sleep);
    lua_setfield(s_pLuaState, -2, "Sleep");

    lua_pushcfunction(s_pLuaState, lua_phys_SetMass);
    lua_setfield(s_pLuaState, -2, "SetMass");

    lua_pushcfunction(s_pLuaState, lua_phys_GetMass);
    lua_setfield(s_pLuaState, -2, "GetMass");

    lua_pushcfunction(s_pLuaState, lua_phys_EnableCollisions);
    lua_setfield(s_pLuaState, -2, "EnableCollisions");

    lua_pushcfunction(s_pLuaState, lua_phys_EnableGravity);
    lua_setfield(s_pLuaState, -2, "EnableGravity");

    lua_pushcfunction(s_pLuaState, lua_phys_EnableDrag);
    lua_setfield(s_pLuaState, -2, "EnableDrag");

    lua_pushcfunction(s_pLuaState, lua_phys_EnableMotion);
    lua_setfield(s_pLuaState, -2, "EnableMotion");

    lua_pushcfunction(s_pLuaState, lua_phys_ApplyForceCenter);
    lua_setfield(s_pLuaState, -2, "ApplyForceCenter");

    lua_pushcfunction(s_pLuaState, lua_phys_ApplyForceOffset);
    lua_setfield(s_pLuaState, -2, "ApplyForceOffset");

    lua_pushcfunction(s_pLuaState, lua_phys_ApplyTorqueCenter);
    lua_setfield(s_pLuaState, -2, "ApplyTorqueCenter");

    lua_pushcfunction(s_pLuaState, lua_phys_ConstraintSetEnts);
    lua_setfield(s_pLuaState, -2, "ConstraintSetEnts");

    lua_pushcfunction(s_pLuaState, lua_phys_GetVelocity);
    lua_setfield(s_pLuaState, -2, "GetVelocity");

    lua_pushcfunction(s_pLuaState, lua_phys_SetVelocity);
    lua_setfield(s_pLuaState, -2, "SetVelocity");

    lua_setglobal(s_pLuaState, "_phys");

    DevMsg("Lua: Registered %d physics functions + _phys table\n", registered);
}

void CGModLuaSystem::RegisterUtilityFunctions()
{
    static const LuaBinding bindings[] = {
        {"_RunString", lua_RunString},
        {"_CurTime", lua_CurTime},
        {"_Msg", lua_Msg},
        {"_OpenScript", lua_OpenScript},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d utility functions\n", registered);
}

void CGModLuaSystem::RegisterGamemodeFunctions()
{
    static const LuaBinding bindings[] = {
        {"_StartNextLevel", lua_StartNextLevel},
        {"AddThinkFunction", lua_AddThinkFunction},
        {"_GameSetTargetIDRules", lua_GameSetTargetIDRules},
        // Team functions (from gmod_gamemode.cpp)
        {"_TeamSetScore", lua_TeamSetScore},
        {"_TeamScore", lua_TeamGetScore},
        {"_TeamSetName", lua_TeamSetName},
        {"_TeamGetName", lua_TeamGetName},
        {"_PlayerChangeTeam", lua_PlayerChangeTeam},
        {"_MaxPlayers", lua_MaxPlayers},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d gamemode functions\n", registered);
}

void CGModLuaSystem::RegisterConVarFunctions()
{
    static const LuaBinding bindings[] = {
        {"_GetConVar_Float", lua_GetConVar_Float},
        {"_GetConVar_Int", lua_GetConVar_Int},
        {"_GetConVar_String", lua_GetConVar_String},
        {"_SetConVar", lua_SetConVar},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d ConVar functions\n", registered);
}

void CGModLuaSystem::RegisterSoundFunctions()
{
    static const LuaBinding bindings[] = {
        {"_SWEPSetSound", lua_SWEPSetSound},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d sound functions\n", registered);
}

void CGModLuaSystem::RegisterMathFunctions()
{
    static const LuaBinding bindings[] = {
        {"vector3", lua_vector3},
        {"vecAdd", lua_vecAdd},
        {"vecSub", lua_vecSub},
        {"vecMul", lua_vecMul},
        {"vecLength", lua_vecLength},
        {"vecNormalize", lua_vecNormalize},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d math/vector functions\n", registered);
}

void CGModLuaSystem::RegisterUtilTable()
{
    // Create _util global table
    lua_newtable(s_pLuaState);

    lua_pushcfunction(s_pLuaState, lua_util_PlayerByName);
    lua_setfield(s_pLuaState, -2, "PlayerByName");

    lua_pushcfunction(s_pLuaState, lua_util_PlayerByUserId);
    lua_setfield(s_pLuaState, -2, "PlayerByUserId");

    lua_pushcfunction(s_pLuaState, lua_util_EntsInBox);
    lua_setfield(s_pLuaState, -2, "EntsInBox");

    lua_pushcfunction(s_pLuaState, lua_util_DropToFloor);
    lua_setfield(s_pLuaState, -2, "DropToFloor");

    lua_pushcfunction(s_pLuaState, lua_util_ScreenShake);
    lua_setfield(s_pLuaState, -2, "ScreenShake");

    lua_pushcfunction(s_pLuaState, lua_util_PointAtEntity);
    lua_setfield(s_pLuaState, -2, "PointAtEntity");

    lua_setglobal(s_pLuaState, "_util");

    DevMsg("Lua: Registered _util table\n");
}

void CGModLuaSystem::RegisterPlayerTable()
{
    // Create _player global table
    lua_newtable(s_pLuaState);

    lua_pushcfunction(s_pLuaState, lua_player_ShowPanel);
    lua_setfield(s_pLuaState, -2, "ShowPanel");

    lua_pushcfunction(s_pLuaState, lua_player_SetContextMenu);
    lua_setfield(s_pLuaState, -2, "SetContextMenu");

    lua_pushcfunction(s_pLuaState, lua_player_GetFlashlight);
    lua_setfield(s_pLuaState, -2, "GetFlashlight");

    lua_pushcfunction(s_pLuaState, lua_player_SetFlashlight);
    lua_setfield(s_pLuaState, -2, "SetFlashlight");

    lua_pushcfunction(s_pLuaState, lua_player_LastHitGroup);
    lua_setfield(s_pLuaState, -2, "LastHitGroup");

    lua_pushcfunction(s_pLuaState, lua_player_ShouldDropWeapon);
    lua_setfield(s_pLuaState, -2, "ShouldDropWeapon");

    lua_setglobal(s_pLuaState, "_player");

    DevMsg("Lua: Registered _player table\n");
}

void CGModLuaSystem::RegisterNPCTable()
{
    // Create _npc global table
    lua_newtable(s_pLuaState);

    lua_pushcfunction(s_pLuaState, lua_npc_ExitScriptedSequence);
    lua_setfield(s_pLuaState, -2, "ExitScriptedSequence");

    lua_pushcfunction(s_pLuaState, lua_npc_SetSchedule);
    lua_setfield(s_pLuaState, -2, "SetSchedule");

    lua_pushcfunction(s_pLuaState, lua_npc_SetLastPosition);
    lua_setfield(s_pLuaState, -2, "SetLastPosition");

    lua_pushcfunction(s_pLuaState, lua_npc_AddRelationship);
    lua_setfield(s_pLuaState, -2, "AddRelationship");

    lua_setglobal(s_pLuaState, "_npc");

    DevMsg("Lua: Registered _npc table\n");
}

void CGModLuaSystem::RegisterSpawnMenuTable()
{
    // Create _spawnmenu global table
    lua_newtable(s_pLuaState);

    lua_pushcfunction(s_pLuaState, lua_spawnmenu_AddItem);
    lua_setfield(s_pLuaState, -2, "AddItem");

    lua_pushcfunction(s_pLuaState, lua_spawnmenu_RemoveCategory);
    lua_setfield(s_pLuaState, -2, "RemoveCategory");

    lua_pushcfunction(s_pLuaState, lua_spawnmenu_RemoveAll);
    lua_setfield(s_pLuaState, -2, "RemoveAll");

    lua_pushcfunction(s_pLuaState, lua_spawnmenu_SetCategory);
    lua_setfield(s_pLuaState, -2, "SetCategory");

    lua_setglobal(s_pLuaState, "_spawnmenu");

    DevMsg("Lua: Registered _spawnmenu table\n");
}

void CGModLuaSystem::RegisterGModQuadFunctions()
{
    // _gmodquad functions are registered as global functions (not as a table)
    // This matches the original GMod 9 behavior
    static const LuaBinding bindings[] = {
        {"_GModQuad_Hide", lua_GModQuad_Hide},
        {"_GModQuad_HideAll", lua_GModQuad_HideAll},
        {"_GModQuad_Start", lua_GModQuad_Start},
        {"_GModQuad_SetVector", lua_GModQuad_SetVector},
        {"_GModQuad_SetTimings", lua_GModQuad_SetTimings},
        {"_GModQuad_SetEntity", lua_GModQuad_SetEntity},
        {"_GModQuad_Send", lua_GModQuad_Send},
        {"_GModQuad_SendAnimate", lua_GModQuad_SendAnimate},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d _GModQuad functions\n", registered);
}

void CGModLuaSystem::RegisterGModTextFunctions()
{
    // _GModText_* functions for screen text display
    // These match the original GMod 9.0.4b behavior exactly
    static const LuaBinding bindings[] = {
        {"_GModText_Start", lua_GModText_Start},
        {"_GModText_SetPos", lua_GModText_SetPos},
        {"_GModText_SetColor", lua_GModText_SetColor},
        {"_GModText_SetFade", lua_GModText_SetFade},
        {"_GModText_SetText", lua_GModText_SetText},
        {"_GModText_SetEffect", lua_GModText_SetEffect},
        {"_GModText_SetAlign", lua_GModText_SetAlign},
        {"_GModText_Send", lua_GModText_Send},
        {"_GModText_Hide", lua_GModText_Hide},
        // Also register without underscore separator for alternate naming convention
        {"_GModTextStart", lua_GModText_Start},
        {"_GModTextSetPos", lua_GModText_SetPos},
        {"_GModTextSetColor", lua_GModText_SetColor},
        {"_GModTextSetFade", lua_GModText_SetFade},
        {"_GModTextSetText", lua_GModText_SetText},
        {"_GModTextSetEffect", lua_GModText_SetEffect},
        {"_GModTextSetAlign", lua_GModText_SetAlign},
        {"_GModTextSend", lua_GModText_Send},
        {"_GModTextHideAll", lua_GModText_Hide},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d _GModText functions\n", registered);
}

void CGModLuaSystem::RegisterGModRectFunctions()
{
    // _GModRect_* functions for screen rectangle display
    static const LuaBinding bindings[] = {
        {"_GModRect_Start", lua_GModRect_Start},
        {"_GModRect_SetPos", lua_GModRect_SetPos},
        {"_GModRect_SetSize", lua_GModRect_SetSize},
        {"_GModRect_SetColor", lua_GModRect_SetColor},
        {"_GModRect_SetID", lua_GModRect_SetID},
        {"_GModRect_Send", lua_GModRect_Send},
        {"_GModRect_Hide", lua_GModRect_Hide},
        // Also register without underscore separator
        {"_GModRectSetPos", lua_GModRect_SetPos},
        {"_GModRectSetSize", lua_GModRect_SetSize},
        {"_GModRectSetColor", lua_GModRect_SetColor},
        {"_GModRectSetID", lua_GModRect_SetID},
        {"_GModRectSend", lua_GModRect_Send},
        {"_GModRectHideAll", lua_GModRect_Hide},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d _GModRect functions\n", registered);
}

void CGModLuaSystem::RegisterGameEventTable()
{
    // Create _gameevent global table
    lua_newtable(s_pLuaState);

    lua_pushcfunction(s_pLuaState, lua_gameevent_Start);
    lua_setfield(s_pLuaState, -2, "Start");

    lua_pushcfunction(s_pLuaState, lua_gameevent_SetString);
    lua_setfield(s_pLuaState, -2, "SetString");

    lua_pushcfunction(s_pLuaState, lua_gameevent_SetInt);
    lua_setfield(s_pLuaState, -2, "SetInt");

    lua_pushcfunction(s_pLuaState, lua_gameevent_Fire);
    lua_setfield(s_pLuaState, -2, "Fire");

    lua_setglobal(s_pLuaState, "_gameevent");

    DevMsg("Lua: Registered _gameevent table\n");
}

LuaFunctionResult_t CGModLuaSystem::LoadScript(const char* pszFileName, LuaScriptType_t type)
{
    if (!s_pLuaState || !pszFileName)
        return LUA_RESULT_ERROR;

    char fullPath[MAX_PATH];

    // Check if path already starts with the lua base path to avoid duplication
    const char* luaPath = gmod_lua_path.GetString();
    if (Q_strnicmp(pszFileName, luaPath, Q_strlen(luaPath)) == 0 ||
        Q_strnicmp(pszFileName, "lua/", 4) == 0)
    {
        // Path already includes lua/ prefix, use as-is
        Q_strncpy(fullPath, pszFileName, sizeof(fullPath));
    }
    else
    {
        // Add lua/ prefix
        Q_snprintf(fullPath, sizeof(fullPath), "%s%s", luaPath, pszFileName);
    }

    if (!filesystem->FileExists(fullPath, "GAME"))
    {
        Warning("Lua script not found: %s\n", fullPath);
        return LUA_RESULT_FILE_NOT_FOUND;
    }

    // Read file using Source Engine filesystem (GAME search path includes mod directory)
    FileHandle_t hFile = filesystem->Open(fullPath, "rb", "GAME");
    if (!hFile)
    {
        Warning("Lua script could not be opened: %s\n", fullPath);
        return LUA_RESULT_FILE_NOT_FOUND;
    }

    unsigned int fileSize = filesystem->Size(hFile);
    char* pBuffer = new char[fileSize + 1];
    filesystem->Read(pBuffer, fileSize, hFile);
    filesystem->Close(hFile);
    pBuffer[fileSize] = '\0';

    // Load and execute the Lua code from buffer
    int status = luaL_loadbuffer(s_pLuaState, pBuffer, fileSize, fullPath);
    if (status != 0)
    {
        const char* error = lua_tostring(s_pLuaState, -1);
        Warning("Lua syntax error in %s: %s\n", pszFileName, error ? error : "unknown");
        lua_pop(s_pLuaState, 1);
        delete[] pBuffer;
        return LUA_RESULT_SYNTAX_ERROR;
    }

    status = lua_pcall(s_pLuaState, 0, LUA_MULTRET, 0);
    if (status != 0)
    {
        const char* error = lua_tostring(s_pLuaState, -1);
        Warning("Lua execution error in %s: %s\n", pszFileName, error ? error : "unknown");
        lua_pop(s_pLuaState, 1);
        delete[] pBuffer;
        return LUA_RESULT_RUNTIME_ERROR;
    }

    // Clean up buffer after successful load
    delete[] pBuffer;

    // Track loaded script
    LuaContext_t context;
    context.L = s_pLuaState;
    context.scriptType = type;
    Q_strncpy(context.fileName, pszFileName, sizeof(context.fileName));
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
    LoadScript("includes/timers.lua", LUA_SCRIPT_INCLUDE);  // Defines DoLuaThinkFunctions
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

LuaContext_t* CGModLuaSystem::GetCurrentContext()
{
    return &s_CurrentContext;
}

void CGModLuaSystem::SetContextPlayer(CBasePlayer* pPlayer)
{
    s_CurrentContext.pContextPlayer = pPlayer;
}

void CGModLuaSystem::SetContextEntity(CBaseEntity* pEntity)
{
    s_CurrentContext.pContextEntity = pEntity;
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
// Additional Lua system methods (compatibility wrapper implementations)
//-----------------------------------------------------------------------------
bool CGModLuaSystem::RunLuaFile(const char* pszFileName)
{
    if (!pszFileName)
        return false;

    DevMsg("Lua: Would run file: %s\n", pszFileName);
    return CLuaWrapper::RunLuaFile(pszFileName);
}

bool CGModLuaSystem::Initialize()
{
    return (InitializeLua() == LUA_RESULT_SUCCESS);
}

// Forward declarations for gameevent Lua helpers
extern "C" {
int Lua_GameEvent_Start(lua_State* L);
int Lua_GameEvent_SetString(lua_State* L);
int Lua_GameEvent_SetPlayerInt(lua_State* L);
int Lua_GameEvent_SetPlayerVector(lua_State* L);
int Lua_GameEvent_Fire(lua_State* L);
}

void CGModLuaSystem::RegisterGlobalFunctions()
{
    RegisterEngineBindings();

    // Game event helpers (parity with gmod server.dll)
    lua_register(s_pLuaState, "GameEvent_Start", Lua_GameEvent_Start);
    lua_register(s_pLuaState, "GameEvent_SetString", Lua_GameEvent_SetString);
    lua_register(s_pLuaState, "GameEvent_SetPlayerInt", Lua_GameEvent_SetPlayerInt);
    lua_register(s_pLuaState, "GameEvent_SetPlayerVector", Lua_GameEvent_SetPlayerVector);
    lua_register(s_pLuaState, "GameEvent_Fire", Lua_GameEvent_Fire);
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

//-----------------------------------------------------------------------------
// Lua gameevent bindings (parity with gmod server.dll)
//-----------------------------------------------------------------------------
static void LuaWarnArgs(lua_State* L, int expected)
{
    int got = lua_gettop(L);
    if (got != expected)
    {
        Msg("Lua warning: Wrong number of args (should have %i args)\n", expected);
    }
}

int Lua_GameEvent_Start(lua_State* L)
{
    LuaWarnArgs(L, 1);
    // Expects: (player)
    CBaseEntity *pEnt = CLuaWrapper::GetLuaEntity(L, 1);
    CBasePlayer *pPlayer = dynamic_cast<CBasePlayer*>(pEnt);
    if (pPlayer)
    {
        // Start a new game event for this player (mirrors sub_22020F40 call)
        CLuaWrapper::StartGameEvent(pPlayer);
    }
    return 0;
}

int Lua_GameEvent_SetString(lua_State* L)
{
    LuaWarnArgs(L, 2);
    // Expects: (player, name)
    CBaseEntity *pEnt = CLuaWrapper::GetLuaEntity(L, 1);
    CBasePlayer *pPlayer = dynamic_cast<CBasePlayer*>(pEnt);
    if (pPlayer && lua_isstring(L, 2))
    {
        const char *str = lua_tostring(L, 2);
        CLuaWrapper::GameEventSetString(str);
    }
    return 0;
}

int Lua_GameEvent_SetPlayerInt(lua_State* L)
{
    LuaWarnArgs(L, 4);
    // Expects: (player, targetPlayer, keyInt, valueInt)
    CBaseEntity *pEntA = CLuaWrapper::GetLuaEntity(L, 1);
    CBasePlayer *pPlayerA = dynamic_cast<CBasePlayer*>(pEntA);
    CBaseEntity *pEntB = CLuaWrapper::GetLuaEntity(L, 2);
    CBasePlayer *pPlayerB = dynamic_cast<CBasePlayer*>(pEntB);
    int key = (int)lua_tonumber(L, 3);
    int val = (int)lua_tonumber(L, 4);
    if (pPlayerA && pPlayerB)
    {
        CLuaWrapper::GameEventSetPlayerInt(pPlayerA, pPlayerB, key, val);
    }
    return 0;
}

int Lua_GameEvent_SetPlayerVector(lua_State* L)
{
    LuaWarnArgs(L, 2);
    // Expects: (player, vector)
    CBaseEntity *pEnt = CLuaWrapper::GetLuaEntity(L, 1);
    CBasePlayer *pPlayer = dynamic_cast<CBasePlayer*>(pEnt);
    Vector vec;
    if (pPlayer && CLuaWrapper::GetVector(L, 2, vec))
    {
        CLuaWrapper::GameEventSetPlayerVector(pPlayer, vec);
    }
    return 0;
}

int Lua_GameEvent_Fire(lua_State* L)
{
    (void)L;
    // Fires the currently staged game event (if any)
    CLuaWrapper::CommitActiveGameEvent();
    return 0;
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
    LuaContext_t* pCtx = CGModLuaSystem::GetCurrentContext();
    if (pCtx)
    {
        pCtx->lastTrace = tr;
    }

    return 0;
}

int lua_TraceEndPos(lua_State* L)
{
    LuaContext_t* pCtx = CGModLuaSystem::GetCurrentContext();
    Vector endPos = pCtx ? pCtx->lastTrace.endpos : vec3_origin;

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
    LuaContext_t* pCtx = CGModLuaSystem::GetCurrentContext();
    lua_pushboolean(L, pCtx ? pCtx->lastTrace.DidHit() : 0);
    return 1;
}

int lua_TraceHitWorld(lua_State* L)
{
    LuaContext_t* pCtx = CGModLuaSystem::GetCurrentContext();
    lua_pushboolean(L, pCtx ? pCtx->lastTrace.DidHitWorld() : 0);
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

    const ConVar* pConVar = cvar->FindVar(name);
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

    const ConVar* pConVar = cvar->FindVar(name);
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

    const ConVar* pConVar = cvar->FindVar(name);
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

    const ConVar* pConVar = cvar->FindVar(name);
    if (pConVar)
    {
        const_cast<ConVar*>(pConVar)->SetValue(value);
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

//-----------------------------------------------------------------------------
// _phys table functions (GMod 9 physics table)
// These are accessed as _phys.HasPhysics, _phys.Wake, etc.
//-----------------------------------------------------------------------------

// Helper function to get physics object from entity ID
static IPhysicsObject* GetPhysicsFromEntityID(lua_State* L, int arg)
{
    if (lua_gettop(L) < arg)
        return NULL;

    int entid = (int)lua_tonumber(L, arg);
    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity)
        return NULL;

    return pEntity->VPhysicsGetObject();
}

int lua_phys_HasPhysics(lua_State* L)
{
    int entid = (int)lua_tonumber(L, 1);
    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    IPhysicsObject* pPhys = pEntity->VPhysicsGetObject();
    lua_pushboolean(L, pPhys != NULL);
    return 1;
}

int lua_phys_IsAsleep(lua_State* L)
{
    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (!pPhys)
    {
        lua_pushboolean(L, true); // No physics = asleep
        return 1;
    }

    lua_pushboolean(L, pPhys->IsAsleep());
    return 1;
}

int lua_phys_Wake(lua_State* L)
{
    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys)
    {
        pPhys->Wake();
    }
    return 0;
}

int lua_phys_Sleep(lua_State* L)
{
    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys)
    {
        pPhys->Sleep();
    }
    return 0;
}

int lua_phys_SetMass(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys)
    {
        float mass = (float)lua_tonumber(L, 2);
        pPhys->SetMass(mass);
    }
    return 0;
}

int lua_phys_GetMass(lua_State* L)
{
    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
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

int lua_phys_EnableCollisions(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys)
    {
        bool enable = lua_toboolean(L, 2) != 0;
        pPhys->EnableCollisions(enable);
    }
    return 0;
}

int lua_phys_EnableGravity(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys)
    {
        bool enable = lua_toboolean(L, 2) != 0;
        pPhys->EnableGravity(enable);
    }
    return 0;
}

int lua_phys_EnableDrag(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys)
    {
        bool enable = lua_toboolean(L, 2) != 0;
        pPhys->EnableDrag(enable);
    }
    return 0;
}

int lua_phys_EnableMotion(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys)
    {
        bool enable = lua_toboolean(L, 2) != 0;
        pPhys->EnableMotion(enable);
    }
    return 0;
}

int lua_phys_ApplyForceCenter(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys && lua_istable(L, 2))
    {
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");

        Vector force;
        force.x = (float)lua_tonumber(L, -3);
        force.y = (float)lua_tonumber(L, -2);
        force.z = (float)lua_tonumber(L, -1);

        pPhys->ApplyForceCenter(force);

        lua_pop(L, 3);
    }
    return 0;
}

int lua_phys_ApplyForceOffset(lua_State* L)
{
    if (lua_gettop(L) < 3)
        return 0;

    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys && lua_istable(L, 2) && lua_istable(L, 3))
    {
        Vector force, offset;

        // Get force vector
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");
        force.x = (float)lua_tonumber(L, -3);
        force.y = (float)lua_tonumber(L, -2);
        force.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);

        // Get offset vector
        lua_getfield(L, 3, "x");
        lua_getfield(L, 3, "y");
        lua_getfield(L, 3, "z");
        offset.x = (float)lua_tonumber(L, -3);
        offset.y = (float)lua_tonumber(L, -2);
        offset.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);

        pPhys->ApplyForceOffset(force, offset);
    }
    return 0;
}

int lua_phys_ApplyTorqueCenter(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys && lua_istable(L, 2))
    {
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");

        Vector torque;
        torque.x = (float)lua_tonumber(L, -3);
        torque.y = (float)lua_tonumber(L, -2);
        torque.z = (float)lua_tonumber(L, -1);

        AngularImpulse angImpulse(torque.x, torque.y, torque.z);
        pPhys->ApplyTorqueCenter(angImpulse);

        lua_pop(L, 3);
    }
    return 0;
}

int lua_phys_ConstraintSetEnts(lua_State* L)
{
    // Takes (constraint, ent1, ent2, bone1, bone2)
    // This would require constraint management - for now, stub it
    if (lua_gettop(L) < 5)
        return 0;

    DevMsg("_phys.ConstraintSetEnts called - constraint system not fully implemented\n");
    return 0;
}

int lua_phys_GetVelocity(lua_State* L)
{
    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys)
    {
        Vector velocity;
        AngularImpulse angVelocity;
        pPhys->GetVelocity(&velocity, &angVelocity);

        // Create vector3 table
        lua_newtable(L);
        lua_pushnumber(L, velocity.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, velocity.y);
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, velocity.z);
        lua_setfield(L, -2, "z");
    }
    else
    {
        lua_newtable(L);
        lua_pushnumber(L, 0);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, 0);
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, 0);
        lua_setfield(L, -2, "z");
    }
    return 1;
}

int lua_phys_SetVelocity(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    IPhysicsObject* pPhys = GetPhysicsFromEntityID(L, 1);
    if (pPhys && lua_istable(L, 2))
    {
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");

        Vector velocity;
        velocity.x = (float)lua_tonumber(L, -3);
        velocity.y = (float)lua_tonumber(L, -2);
        velocity.z = (float)lua_tonumber(L, -1);

        pPhys->SetVelocity(&velocity, NULL);

        lua_pop(L, 3);
    }
    return 0;
}

//-----------------------------------------------------------------------------
// AddThinkFunction - Registers a function to be called during DoLuaThinkFunctions
//-----------------------------------------------------------------------------
int lua_AddThinkFunction(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    // In GMod 9, AddThinkFunction stores a reference to the passed function
    // The DoLuaThinkFunctions Lua function then iterates and calls them
    // We implement this by registering the function in the ThinkFunctions table

    // Get or create the ThinkFunctions table
    lua_getglobal(L, "ThinkFunctions");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "ThinkFunctions");
        lua_getglobal(L, "ThinkFunctions");
    }

    // Get the next index (using luaL_getn for Lua 5.0 compatibility)
    int len = luaL_getn(L, -1);

    // Push the function reference (duplicate from arg 1)
    lua_pushvalue(L, 1);

    // Store it at index len+1
    lua_rawseti(L, -2, len + 1);

    lua_pop(L, 1); // pop ThinkFunctions table
    return 0;
}

//-----------------------------------------------------------------------------
// _GameSetTargetIDRules - Sets target ID rules
// NOTE: Actual implementation is in gmod_gamemode.cpp to avoid duplicate symbols
//-----------------------------------------------------------------------------

int CGModLuaSystem::GetTargetIDRules()
{
    // Forward to gamemode system
    return (int)CGModGamemodeSystem::GetTargetIDRules();
}

//-----------------------------------------------------------------------------
// _util table functions (GMod 9 utility table)
// These are accessed as _util.PlayerByName, _util.DropToFloor, etc.
//-----------------------------------------------------------------------------

int lua_util_PlayerByName(lua_State* L)
{
    if (lua_gettop(L) < 1)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    const char* name = lua_tostring(L, 1);
    if (!name)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    // Search for player by name
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
        if (pPlayer && Q_stristr(STRING(pPlayer->pl.netname), name))
        {
            lua_pushnumber(L, i);
            return 1;
        }
    }

    lua_pushnumber(L, -1);
    return 1;
}

int lua_util_PlayerByUserId(lua_State* L)
{
    if (lua_gettop(L) < 1)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    int userid = (int)lua_tonumber(L, 1);

    // Search for player by userid
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
        if (pPlayer && engine->GetPlayerUserId(pPlayer->edict()) == userid)
        {
            lua_pushnumber(L, i);
            return 1;
        }
    }

    lua_pushnumber(L, -1);
    return 1;
}

int lua_util_EntsInBox(lua_State* L)
{
    if (lua_gettop(L) < 2)
    {
        lua_newtable(L);
        return 1;
    }

    Vector mins, maxs;

    // Get min vector
    if (lua_istable(L, 1))
    {
        lua_getfield(L, 1, "x");
        lua_getfield(L, 1, "y");
        lua_getfield(L, 1, "z");
        mins.x = (float)lua_tonumber(L, -3);
        mins.y = (float)lua_tonumber(L, -2);
        mins.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    // Get max vector
    if (lua_istable(L, 2))
    {
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");
        maxs.x = (float)lua_tonumber(L, -3);
        maxs.y = (float)lua_tonumber(L, -2);
        maxs.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    // Create result table
    lua_newtable(L);
    int resultIndex = 1;

    // Iterate through entities and find ones in box
    CBaseEntity* pEntity = gEntList.FirstEnt();
    while (pEntity)
    {
        Vector pos = pEntity->GetAbsOrigin();
        if (pos.x >= mins.x && pos.x <= maxs.x &&
            pos.y >= mins.y && pos.y <= maxs.y &&
            pos.z >= mins.z && pos.z <= maxs.z)
        {
            lua_pushnumber(L, pEntity->entindex());
            lua_rawseti(L, -2, resultIndex++);
        }
        pEntity = gEntList.NextEnt(pEntity);
    }

    return 1;
}

int lua_util_DropToFloor(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int entid = (int)lua_tonumber(L, 1);
    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    if (!pEntity)
        return 0;

    UTIL_DropToFloor(pEntity, MASK_SOLID);
    return 0;
}

int lua_util_ScreenShake(lua_State* L)
{
    if (lua_gettop(L) < 5)
        return 0;

    Vector pos;
    if (lua_istable(L, 1))
    {
        lua_getfield(L, 1, "x");
        lua_getfield(L, 1, "y");
        lua_getfield(L, 1, "z");
        pos.x = (float)lua_tonumber(L, -3);
        pos.y = (float)lua_tonumber(L, -2);
        pos.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    float amplitude = (float)lua_tonumber(L, 2);
    float frequency = (float)lua_tonumber(L, 3);
    float duration = (float)lua_tonumber(L, 4);
    float radius = (float)lua_tonumber(L, 5);

    UTIL_ScreenShake(pos, amplitude, frequency, duration, radius, SHAKE_START, false);
    return 0;
}

int lua_util_PointAtEntity(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int entid = (int)lua_tonumber(L, 1);
    int targetid = (int)lua_tonumber(L, 2);

    CBaseEntity* pEntity = UTIL_EntityByIndex(entid);
    CBaseEntity* pTarget = UTIL_EntityByIndex(targetid);

    if (!pEntity || !pTarget)
        return 0;

    // Calculate angle to point at target
    Vector dir = pTarget->GetAbsOrigin() - pEntity->GetAbsOrigin();
    QAngle angles;
    VectorAngles(dir, angles);
    pEntity->SetAbsAngles(angles);

    return 0;
}

//-----------------------------------------------------------------------------
// _player table functions (GMod 9 player table)
// These are accessed as _player.ShowPanel, _player.SetFlashlight, etc.
//-----------------------------------------------------------------------------

int lua_player_ShowPanel(lua_State* L)
{
    if (lua_gettop(L) < 3)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    const char* panelName = lua_tostring(L, 2);
    bool show = lua_toboolean(L, 3) != 0;

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer || !panelName)
        return 0;

    // Send panel show/hide message to client
    // This would be implemented via user messages
    DevMsg("_player.ShowPanel: %s panel %s for player %d\n",
           show ? "Showing" : "Hiding", panelName, playerid);

    return 0;
}

int lua_player_SetContextMenu(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    bool enable = lua_toboolean(L, 2) != 0;

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    // Enable/disable context menu for player
    DevMsg("_player.SetContextMenu: %s for player %d\n",
           enable ? "Enabled" : "Disabled", playerid);

    return 0;
}

int lua_player_GetFlashlight(lua_State* L)
{
    if (lua_gettop(L) < 1)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    int playerid = (int)lua_tonumber(L, 1);
    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);

    if (!pPlayer)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    lua_pushboolean(L, pPlayer->FlashlightIsOn());
    return 1;
}

int lua_player_SetFlashlight(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    bool on = lua_toboolean(L, 2) != 0;

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    if (on && !pPlayer->FlashlightIsOn())
        pPlayer->FlashlightTurnOn();
    else if (!on && pPlayer->FlashlightIsOn())
        pPlayer->FlashlightTurnOff();

    return 0;
}

int lua_player_LastHitGroup(lua_State* L)
{
    if (lua_gettop(L) < 1)
    {
        lua_pushnumber(L, 0);
        return 1;
    }

    int playerid = (int)lua_tonumber(L, 1);
    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);

    if (!pPlayer)
    {
        lua_pushnumber(L, 0);
        return 1;
    }

    // Access m_LastHitGroup directly since LastHitGroup() is protected
    // m_LastHitGroup is a public member in CBaseCombatCharacter
    lua_pushnumber(L, 0);  // Stub - LastHitGroup/m_LastHitGroup inaccessible
    return 1;
}

int lua_player_ShouldDropWeapon(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    bool drop = lua_toboolean(L, 2) != 0;

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    // Set whether player should drop weapon on death
    // This would require extending player class - for now stub it
    DevMsg("_player.ShouldDropWeapon: %s for player %d\n",
           drop ? "true" : "false", playerid);

    return 0;
}

//-----------------------------------------------------------------------------
// _npc table functions (GMod 9 NPC table)
// These are accessed as _npc.SetSchedule, _npc.AddRelationship, etc.
//-----------------------------------------------------------------------------

int lua_npc_ExitScriptedSequence(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int npcid = (int)lua_tonumber(L, 1);
    CBaseEntity* pEntity = UTIL_EntityByIndex(npcid);
    if (!pEntity)
        return 0;

    // Try to cast to NPC and exit scripted sequence
    CAI_BaseNPC* pNPC = dynamic_cast<CAI_BaseNPC*>(pEntity);
    if (pNPC)
    {
        pNPC->ExitScriptedSequence();
    }

    return 0;
}

int lua_npc_SetSchedule(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int npcid = (int)lua_tonumber(L, 1);
    int schedule = (int)lua_tonumber(L, 2);

    CBaseEntity* pEntity = UTIL_EntityByIndex(npcid);
    if (!pEntity)
        return 0;

    CAI_BaseNPC* pNPC = dynamic_cast<CAI_BaseNPC*>(pEntity);
    if (pNPC)
    {
        pNPC->SetSchedule(schedule);
    }

    return 0;
}

int lua_npc_SetLastPosition(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int npcid = (int)lua_tonumber(L, 1);
    CBaseEntity* pEntity = UTIL_EntityByIndex(npcid);
    if (!pEntity)
        return 0;

    CAI_BaseNPC* pNPC = dynamic_cast<CAI_BaseNPC*>(pEntity);
    if (!pNPC)
        return 0;

    // Get position from table
    if (lua_istable(L, 2))
    {
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");

        Vector pos;
        pos.x = (float)lua_tonumber(L, -3);
        pos.y = (float)lua_tonumber(L, -2);
        pos.z = (float)lua_tonumber(L, -1);

        // Use SetLastKnownPos via enemy memory if available
        if (pNPC->GetEnemy())
        {
            pNPC->UpdateEnemyMemory(pNPC->GetEnemy(), pos);
        }

        lua_pop(L, 3);
    }

    return 0;
}

int lua_npc_AddRelationship(lua_State* L)
{
    if (lua_gettop(L) < 4)
        return 0;

    int npcid = (int)lua_tonumber(L, 1);
    const char* targetClass = lua_tostring(L, 2);
    int disposition = (int)lua_tonumber(L, 3);
    int priority = (int)lua_tonumber(L, 4);

    CBaseEntity* pEntity = UTIL_EntityByIndex(npcid);
    if (!pEntity || !targetClass)
        return 0;

    CAI_BaseNPC* pNPC = dynamic_cast<CAI_BaseNPC*>(pEntity);
    if (pNPC)
    {
        // Build relationship string in format: "classname D_XX priority"
        char relationshipStr[256];
        const char* dispStr = "D_NU"; // Neutral default
        switch (disposition)
        {
            case 1: dispStr = "D_HT"; break; // Hate
            case 2: dispStr = "D_FR"; break; // Fear
            case 3: dispStr = "D_LI"; break; // Like
            case 4: dispStr = "D_NU"; break; // Neutral
        }
        Q_snprintf(relationshipStr, sizeof(relationshipStr), "%s %s %d",
                   targetClass, dispStr, priority);
        pNPC->AddRelationship(relationshipStr, NULL);
    }

    return 0;
}

//-----------------------------------------------------------------------------
// _spawnmenu table functions (GMod 9 spawn menu table)
// These are accessed as _spawnmenu.AddItem, _spawnmenu.RemoveCategory, etc.
//-----------------------------------------------------------------------------

// Spawn menu item storage
struct SpawnMenuItem_t
{
    char category[64];
    char name[64];
    char model[256];
    int skin;
};

static CUtlVector<SpawnMenuItem_t> s_SpawnMenuItems;
static char s_CurrentSpawnMenuCategory[64] = "Props";

int lua_spawnmenu_AddItem(lua_State* L)
{
    if (lua_gettop(L) < 3)
        return 0;

    SpawnMenuItem_t item;
    Q_strncpy(item.category, lua_tostring(L, 1) ? lua_tostring(L, 1) : "Props", sizeof(item.category));
    Q_strncpy(item.name, lua_tostring(L, 2) ? lua_tostring(L, 2) : "Unknown", sizeof(item.name));
    Q_strncpy(item.model, lua_tostring(L, 3) ? lua_tostring(L, 3) : "", sizeof(item.model));
    item.skin = lua_gettop(L) >= 4 ? (int)lua_tonumber(L, 4) : 0;

    s_SpawnMenuItems.AddToTail(item);

    if (gmod_lua_debug.GetBool())
    {
        DevMsg("_spawnmenu.AddItem: Added '%s' to category '%s'\n", item.name, item.category);
    }

    return 0;
}

int lua_spawnmenu_RemoveCategory(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    const char* category = lua_tostring(L, 1);
    if (!category)
        return 0;

    // Remove all items in this category
    for (int i = s_SpawnMenuItems.Count() - 1; i >= 0; i--)
    {
        if (Q_stricmp(s_SpawnMenuItems[i].category, category) == 0)
        {
            s_SpawnMenuItems.Remove(i);
        }
    }

    return 0;
}

int lua_spawnmenu_RemoveAll(lua_State* L)
{
    (void)L;
    s_SpawnMenuItems.Purge();
    return 0;
}

int lua_spawnmenu_SetCategory(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    const char* category = lua_tostring(L, 1);
    if (category)
    {
        Q_strncpy(s_CurrentSpawnMenuCategory, category, sizeof(s_CurrentSpawnMenuCategory));
    }

    return 0;
}

//-----------------------------------------------------------------------------
// _gmodquad global functions (GMod 9 quad rendering)
// These are global functions: _GModQuad_Hide, _GModQuad_Start, etc.
//-----------------------------------------------------------------------------

// Quad rendering state
struct GModQuad_t
{
    int id;
    Vector pos;
    Vector normal;
    float fadeIn;
    float hold;
    float fadeOut;
    int entityId;
    bool active;
};

static GModQuad_t s_CurrentQuad;
static CUtlVector<GModQuad_t> s_ActiveQuads;

int lua_GModQuad_Hide(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int quadId = (int)lua_tonumber(L, 1);

    // Find and deactivate quad
    for (int i = 0; i < s_ActiveQuads.Count(); i++)
    {
        if (s_ActiveQuads[i].id == quadId)
        {
            s_ActiveQuads[i].active = false;
            break;
        }
    }

    return 0;
}

int lua_GModQuad_HideAll(lua_State* L)
{
    (void)L;
    for (int i = 0; i < s_ActiveQuads.Count(); i++)
    {
        s_ActiveQuads[i].active = false;
    }
    return 0;
}

int lua_GModQuad_Start(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    Q_memset(&s_CurrentQuad, 0, sizeof(s_CurrentQuad));
    s_CurrentQuad.id = (int)lua_tonumber(L, 1);
    s_CurrentQuad.active = true;

    return 0;
}

int lua_GModQuad_SetVector(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    const char* name = lua_tostring(L, 1);
    if (!name)
        return 0;

    Vector vec;
    if (lua_istable(L, 2))
    {
        lua_getfield(L, 2, "x");
        lua_getfield(L, 2, "y");
        lua_getfield(L, 2, "z");
        vec.x = (float)lua_tonumber(L, -3);
        vec.y = (float)lua_tonumber(L, -2);
        vec.z = (float)lua_tonumber(L, -1);
        lua_pop(L, 3);
    }

    if (Q_stricmp(name, "pos") == 0 || Q_stricmp(name, "position") == 0)
        s_CurrentQuad.pos = vec;
    else if (Q_stricmp(name, "normal") == 0)
        s_CurrentQuad.normal = vec;

    return 0;
}

int lua_GModQuad_SetTimings(lua_State* L)
{
    if (lua_gettop(L) < 3)
        return 0;

    s_CurrentQuad.fadeIn = (float)lua_tonumber(L, 1);
    s_CurrentQuad.hold = (float)lua_tonumber(L, 2);
    s_CurrentQuad.fadeOut = (float)lua_tonumber(L, 3);

    return 0;
}

int lua_GModQuad_SetEntity(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    s_CurrentQuad.entityId = (int)lua_tonumber(L, 1);
    return 0;
}

int lua_GModQuad_Send(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);

    if (!pPlayer)
        return 0;

    // Add quad to active list
    s_ActiveQuads.AddToTail(s_CurrentQuad);

    // In a real implementation, this would send a user message to client
    DevMsg("_GModQuad_Send: Sent quad %d to player %d\n", s_CurrentQuad.id, playerid);

    return 0;
}

int lua_GModQuad_SendAnimate(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);

    if (!pPlayer)
        return 0;

    // Add quad to active list with animation flag
    s_ActiveQuads.AddToTail(s_CurrentQuad);

    DevMsg("_GModQuad_SendAnimate: Sent animated quad %d to player %d\n", s_CurrentQuad.id, playerid);

    return 0;
}

//-----------------------------------------------------------------------------
// _gameevent table functions (GMod 9 game event table)
// These are accessed as _gameevent.Start, _gameevent.SetString, etc.
// NOTE: This codebase uses KeyValues-based game events, not IGameEvent
//-----------------------------------------------------------------------------

// Game event state - using KeyValues for this older Source Engine version
static KeyValues* s_pCurrentGameEventKV = NULL;
static char s_CurrentEventName[128] = "";

int lua_gameevent_Start(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    const char* eventName = lua_tostring(L, 1);
    if (!eventName)
        return 0;

    // Clean up any previous event
    if (s_pCurrentGameEventKV)
    {
        s_pCurrentGameEventKV->deleteThis();
        s_pCurrentGameEventKV = NULL;
    }

    // Create new KeyValues-based game event
    s_pCurrentGameEventKV = new KeyValues(eventName);
    Q_strncpy(s_CurrentEventName, eventName, sizeof(s_CurrentEventName));

    return 0;
}

int lua_gameevent_SetString(lua_State* L)
{
    if (lua_gettop(L) < 2 || !s_pCurrentGameEventKV)
        return 0;

    const char* key = lua_tostring(L, 1);
    const char* value = lua_tostring(L, 2);

    if (key && value)
    {
        s_pCurrentGameEventKV->SetString(key, value);
    }

    return 0;
}

int lua_gameevent_SetInt(lua_State* L)
{
    if (lua_gettop(L) < 2 || !s_pCurrentGameEventKV)
        return 0;

    const char* key = lua_tostring(L, 1);
    int value = (int)lua_tonumber(L, 2);

    if (key)
    {
        s_pCurrentGameEventKV->SetInt(key, value);
    }

    return 0;
}

int lua_gameevent_Fire(lua_State* L)
{
    (void)L;

    if (s_pCurrentGameEventKV && gameeventmanager)
    {
        // Fire using the KeyValues-based API with a broadcast filter
        CRecipientFilter filter;
        filter.AddAllPlayers();
        gameeventmanager->FireEvent(s_pCurrentGameEventKV, &filter);
        s_pCurrentGameEventKV = NULL; // Event KeyValues is owned by manager after FireEvent
    }

    return 0;
}

//-----------------------------------------------------------------------------
// GModText functions - Display text on screen for players
// These match the original GMod 9.0.4b _GModText_* functions
//-----------------------------------------------------------------------------

// Current GModText state for building messages
static struct GModText_t {
    char fontName[64];
    char text[256];
    float x, y;
    float r, g, b, a;
    float fadeIn, fadeOut, holdTime;
    int effect;
    int align;
} s_GModText = {"Default", "", 0.5f, 0.5f, 255, 255, 255, 255, 0.1f, 0.1f, 5.0f, 0, 0};

int lua_GModText_Start(lua_State* L)
{
    const char* font = lua_gettop(L) >= 1 ? lua_tostring(L, 1) : "Default";
    if (font)
        Q_strncpy(s_GModText.fontName, font, sizeof(s_GModText.fontName));
    else
        Q_strncpy(s_GModText.fontName, "Default", sizeof(s_GModText.fontName));
    s_GModText.text[0] = '\0';
    return 0;
}

int lua_GModText_SetPos(lua_State* L)
{
    if (lua_gettop(L) >= 2)
    {
        s_GModText.x = (float)lua_tonumber(L, 1);
        s_GModText.y = (float)lua_tonumber(L, 2);
    }
    return 0;
}

int lua_GModText_SetColor(lua_State* L)
{
    int n = lua_gettop(L);
    if (n >= 3)
    {
        s_GModText.r = (float)lua_tonumber(L, 1);
        s_GModText.g = (float)lua_tonumber(L, 2);
        s_GModText.b = (float)lua_tonumber(L, 3);
        s_GModText.a = n >= 4 ? (float)lua_tonumber(L, 4) : 255;
    }
    return 0;
}

int lua_GModText_SetFade(lua_State* L)
{
    if (lua_gettop(L) >= 3)
    {
        s_GModText.fadeIn = (float)lua_tonumber(L, 1);
        s_GModText.fadeOut = (float)lua_tonumber(L, 2);
        s_GModText.holdTime = (float)lua_tonumber(L, 3);
    }
    return 0;
}

int lua_GModText_SetText(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        const char* text = lua_tostring(L, 1);
        if (text)
            Q_strncpy(s_GModText.text, text, sizeof(s_GModText.text));
    }
    return 0;
}

int lua_GModText_SetEffect(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        s_GModText.effect = (int)lua_tonumber(L, 1);
    }
    return 0;
}

int lua_GModText_SetAlign(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        s_GModText.align = (int)lua_tonumber(L, 1);
    }
    return 0;
}

int lua_GModText_Send(lua_State* L)
{
    int playerID = lua_gettop(L) >= 1 ? (int)lua_tonumber(L, 1) : -1;

    CRecipientFilter filter;
    if (playerID > 0)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerID);
        if (!pPlayer)
            return 0;
        filter.AddRecipient(pPlayer);
    }
    else
    {
        filter.AddAllPlayers();
    }
    filter.MakeReliable();

    // Send GModText user message
    UserMessageBegin(filter, "GModText");
        WRITE_STRING(s_GModText.fontName);
        WRITE_STRING(s_GModText.text);
        WRITE_FLOAT(s_GModText.x);
        WRITE_FLOAT(s_GModText.y);
        WRITE_BYTE((int)s_GModText.r);
        WRITE_BYTE((int)s_GModText.g);
        WRITE_BYTE((int)s_GModText.b);
        WRITE_BYTE((int)s_GModText.a);
        WRITE_FLOAT(s_GModText.fadeIn);
        WRITE_FLOAT(s_GModText.holdTime);
        WRITE_FLOAT(s_GModText.fadeOut);
        WRITE_BYTE(s_GModText.effect);
        WRITE_BYTE(s_GModText.align);
    MessageEnd();

    return 0;
}

int lua_GModText_Hide(lua_State* L)
{
    int playerID = lua_gettop(L) >= 1 ? (int)lua_tonumber(L, 1) : -1;

    CRecipientFilter filter;
    if (playerID > 0)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerID);
        if (!pPlayer)
            return 0;
        filter.AddRecipient(pPlayer);
    }
    else
    {
        filter.AddAllPlayers();
    }
    filter.MakeReliable();

    UserMessageBegin(filter, "GModTextHideAll");
    MessageEnd();

    return 0;
}

//-----------------------------------------------------------------------------
// GModRect functions - Display rectangles on screen for players
//-----------------------------------------------------------------------------

static struct GModRect_t {
    float x, y;
    float w, h;
    float r, g, b, a;
    int id;
} s_GModRect = {0, 0, 100, 100, 255, 255, 255, 255, 0};

int lua_GModRect_Start(lua_State* L)
{
    // Reset rect state
    s_GModRect.x = 0;
    s_GModRect.y = 0;
    s_GModRect.w = 100;
    s_GModRect.h = 100;
    s_GModRect.r = 255;
    s_GModRect.g = 255;
    s_GModRect.b = 255;
    s_GModRect.a = 255;
    s_GModRect.id = 0;
    (void)L;
    return 0;
}

int lua_GModRect_SetPos(lua_State* L)
{
    if (lua_gettop(L) >= 2)
    {
        s_GModRect.x = (float)lua_tonumber(L, 1);
        s_GModRect.y = (float)lua_tonumber(L, 2);
    }
    return 0;
}

int lua_GModRect_SetSize(lua_State* L)
{
    if (lua_gettop(L) >= 2)
    {
        s_GModRect.w = (float)lua_tonumber(L, 1);
        s_GModRect.h = (float)lua_tonumber(L, 2);
    }
    return 0;
}

int lua_GModRect_SetColor(lua_State* L)
{
    int n = lua_gettop(L);
    if (n >= 3)
    {
        s_GModRect.r = (float)lua_tonumber(L, 1);
        s_GModRect.g = (float)lua_tonumber(L, 2);
        s_GModRect.b = (float)lua_tonumber(L, 3);
        s_GModRect.a = n >= 4 ? (float)lua_tonumber(L, 4) : 255;
    }
    return 0;
}

int lua_GModRect_SetID(lua_State* L)
{
    if (lua_gettop(L) >= 1)
    {
        s_GModRect.id = (int)lua_tonumber(L, 1);
    }
    return 0;
}

int lua_GModRect_Send(lua_State* L)
{
    int playerID = lua_gettop(L) >= 1 ? (int)lua_tonumber(L, 1) : -1;

    CRecipientFilter filter;
    if (playerID > 0)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerID);
        if (!pPlayer)
            return 0;
        filter.AddRecipient(pPlayer);
    }
    else
    {
        filter.AddAllPlayers();
    }
    filter.MakeReliable();

    UserMessageBegin(filter, "GModRect");
        WRITE_SHORT(s_GModRect.id);
        WRITE_FLOAT(s_GModRect.x);
        WRITE_FLOAT(s_GModRect.y);
        WRITE_FLOAT(s_GModRect.w);
        WRITE_FLOAT(s_GModRect.h);
        WRITE_BYTE((int)s_GModRect.r);
        WRITE_BYTE((int)s_GModRect.g);
        WRITE_BYTE((int)s_GModRect.b);
        WRITE_BYTE((int)s_GModRect.a);
    MessageEnd();

    return 0;
}

int lua_GModRect_Hide(lua_State* L)
{
    int playerID = lua_gettop(L) >= 1 ? (int)lua_tonumber(L, 1) : -1;

    CRecipientFilter filter;
    if (playerID > 0)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerID);
        if (!pPlayer)
            return 0;
        filter.AddRecipient(pPlayer);
    }
    else
    {
        filter.AddAllPlayers();
    }
    filter.MakeReliable();

    UserMessageBegin(filter, "GModRectHideAll");
    MessageEnd();

    return 0;
}

} // extern "C"
