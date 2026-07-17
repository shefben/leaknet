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
#include "basecombatweapon.h"
#include "lua_integration.h"
#include "in_buttons.h"  // IN_ATTACK, IN_FORWARD, etc.
#include "shareddefs.h"  // OBS_MODE constants, TEAM constants
#include "tier0/memdbgon.h"

// Forward declarations for Lua functions defined later in this file (extern "C" linkage)
extern "C" {
int lua_PlayerSpectatorStart(lua_State* L);

static int SourceTeamToGModTeam( int iTeam )
{
	switch ( iTeam )
	{
	case TEAM_UNASSIGNED:
		return 0;
	case TEAM_SPECTATOR:
		return 1;
	default:
		return ( iTeam > TEAM_SPECTATOR ) ? iTeam - 1 : iTeam;
	}
}
int lua_PlayerSpectatorEnd(lua_State* L);
int lua_PlayerSpectatorTarget(lua_State* L);
int lua_PlayerSetChaseCamDistance(lua_State* L);
int lua_PlayerRespawn(lua_State* L);
int lua_PlayerAddDeath(lua_State* L);
int lua_PlayerAddScore(lua_State* L);
int lua_PlayerSilentKill(lua_State* L);
int lua_PlayerIsKeyDown(lua_State* L);
}

namespace
{
    struct LuaBinding
    {
        const char* name;
        lua_CFunction function;
    };

    trace_t g_LegacyLuaTrace;

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

    int LuaObserverModeToSource(int mode)
    {
        // GMod 9 Lua constants are one slot above Source after OBS_MODE_NONE:
        // NONE=0, DEATHCAM=1, FIXED=2, IN_EYE=3, CHASE=4, ROAMING=5.
        switch (mode)
        {
        case 0: return OBS_MODE_NONE;
        case 1: return OBS_MODE_FIXED;   // Beta deathcam fallback.
        case 2: return OBS_MODE_FIXED;
        case 3: return OBS_MODE_IN_EYE;
        case 4: return OBS_MODE_CHASE;
        case 5: return OBS_MODE_ROAMING;
        default: return mode;
        }
    }

    void LuaPushVector(lua_State* L, const Vector& vec)
    {
        lua_newtable(L);
        lua_pushnumber(L, vec.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, vec.y);
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, vec.z);
        lua_setfield(L, -2, "z");
    }

    bool LuaGetVector(lua_State* L, int index, Vector& vec)
    {
        if (lua_istable(L, index))
        {
            lua_getfield(L, index, "x");
            lua_getfield(L, index, "y");
            lua_getfield(L, index, "z");

            vec.x = (float)lua_tonumber(L, -3);
            vec.y = (float)lua_tonumber(L, -2);
            vec.z = (float)lua_tonumber(L, -1);

            lua_pop(L, 3);
            return true;
        }

        if (lua_gettop(L) >= index + 2 && lua_isnumber(L, index))
        {
            vec.x = (float)lua_tonumber(L, index);
            vec.y = (float)lua_tonumber(L, index + 1);
            vec.z = (float)lua_tonumber(L, index + 2);
            return true;
        }

        return false;
    }

    const char* LuaPlayerName(CBasePlayer* pPlayer)
    {
        const char* name = pPlayer ? STRING(pPlayer->pl.netname) : "";
        return name ? name : "";
    }

    void LuaPushPlayerInfoField(lua_State* L, CBasePlayer* pPlayer, const char* field)
    {
        if (!field)
        {
            lua_pushnil(L);
            return;
        }

        if (!pPlayer)
        {
            if (!Q_stricmp(field, "connected"))
                lua_pushboolean(L, false);
            else
                lua_pushnil(L);
            return;
        }

        if (!Q_stricmp(field, "connected"))
            lua_pushboolean(L, true);
        else if (!Q_stricmp(field, "name"))
            lua_pushstring(L, LuaPlayerName(pPlayer));
        else if (!Q_stricmp(field, "userid"))
            lua_pushnumber(L, engine->GetPlayerUserId(pPlayer->edict()));
        else if (!Q_stricmp(field, "ping"))
            lua_pushnumber(L, 0);
        else if (!Q_stricmp(field, "packetloss"))
            lua_pushnumber(L, 0);
        else if (!Q_stricmp(field, "kills"))
            lua_pushnumber(L, pPlayer->FragCount());
        else if (!Q_stricmp(field, "deaths"))
            lua_pushnumber(L, pPlayer->DeathCount());
        else if (!Q_stricmp(field, "team"))
            lua_pushnumber(L, SourceTeamToGModTeam( pPlayer->GetTeamNumber() ));
        else if (!Q_stricmp(field, "alive"))
            lua_pushboolean(L, pPlayer->IsAlive());
        else if (!Q_stricmp(field, "health"))
            lua_pushnumber(L, pPlayer->GetHealth());
        else if (!Q_stricmp(field, "armor"))
            lua_pushnumber(L, pPlayer->ArmorValue());
        else if (!Q_stricmp(field, "model"))
            lua_pushstring(L, STRING(pPlayer->GetModelName()));
        else if (!Q_stricmp(field, "networkid"))
            lua_pushstring(L, "UNKNOWN");
        else if (!Q_stricmp(field, "entindex"))
            lua_pushnumber(L, pPlayer->entindex());
        else if (!Q_stricmp(field, "weapon"))
        {
            CBaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
            lua_pushstring(L, pWeapon ? pWeapon->GetClassname() : "none");
        }
        else
        {
            lua_pushstring(L, "<Not Found>");
        }
    }
}

// Static member definitions
lua_State* CGModLuaSystem::s_pLuaState = NULL;
CUtlVector<LuaContext_t> CGModLuaSystem::s_LoadedScripts;
LuaContext_t CGModLuaSystem::s_CurrentContext;
int CGModLuaSystem::s_ErrorCount = 0;
bool CGModLuaSystem::s_bSystemInitialized = false;

static int s_GModPlayerRawButtons[MAX_PLAYERS];

void CGModLuaSystem::UpdatePlayerRawButtons(CBasePlayer* pPlayer, int buttons)
{
    if (!pPlayer)
        return;

    int playerIndex = pPlayer->entindex();
    if (playerIndex < 0 || playerIndex >= MAX_PLAYERS)
        return;

    s_GModPlayerRawButtons[playerIndex] = buttons;
}

int CGModLuaSystem::GetPlayerRawButtons(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return 0;

    int playerIndex = pPlayer->entindex();
    if (playerIndex < 0 || playerIndex >= MAX_PLAYERS)
        return 0;

    return s_GModPlayerRawButtons[playerIndex];
}

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
    RunFrameLuaThink();
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
		CLuaIntegration::SetLuaState(NULL);
    }
}

void CGModLuaSystem::RegisterEngineBindings()
{
    if (!s_pLuaState)
        return;

    // Share our Lua state with CLuaIntegration so that functions registered
    // via CLuaIntegration::RegisterFunction() use the same state as gamemodes
    CLuaIntegration::SetLuaState(s_pLuaState);

    // Register all the engine functions that GMod scripts expect.
    // NOTE: The extended lua_*_funcs.cpp registrations below run last and win for
    // every duplicated global. The core Register*Functions here only register the
    // names the extended files do NOT cover. Fully-duplicated core tables
    // (Entity/Trace/Sound/GModQuad) are intentionally NOT called here anymore to
    // remove the dead double-registration; the extended files supply those names.
    RegisterPlayerFunctions();
    RegisterPhysicsFunctions();
    RegisterUtilityFunctions();
    RegisterGamemodeFunctions();
    RegisterConVarFunctions();
    RegisterMathFunctions();

    // Register additional GMod 9 global tables
    RegisterUtilTable();
    RegisterPlayerTable();
    RegisterNPCTable();
    RegisterSpawnMenuTable();
    RegisterGameEventTable();
    RegisterGModTextFunctions();
    RegisterGModRectFunctions();

    // Register extended functions from lua_integration subsystem
    // These include _EntPrecacheModel, _GModText_SetTime, and many others
    // needed by GMod gamemodes like melonracer
    RegisterLuaEntityFunctions();
    RegisterLuaPlayerFunctions();
    RegisterLuaPhysicsFunctions();
    RegisterLuaEffectFunctions();
    RegisterLuaGameEventFunctions();

    // File-funcs globals (_PlaySound, _ScreenText, _GetRule, _ServerCommand, ...) live only in
    // lua_file_funcs.cpp's RegisterLuaFileFunctions(), which feeds the separate CLuaIntegration
    // init state -- never the gamemode state. Without this, _PlaySound is nil for gamemodes, so
    // MelonRacer's AddTimer(1,4,_PlaySound,...) stores a nil func and errors every tick until the
    // Lua system is disabled. SetLuaState(s_pLuaState) above makes these land in the gamemode state.
    RegisterLuaFileGlobalsForGamemode();

    // Original gmod exposes file access as the "_file" table in the gamemode state.
    RegisterLuaFileTable(s_pLuaState);

    DevMsg("Registered engine bindings for Lua (including extended functions)\n");
}

void CGModLuaSystem::RegisterPlayerFunctions()
{
    // NOTE: All player globals except _PrintMessage are also registered (last, so
    // they win) by lua_player_funcs.cpp's RegisterLuaPlayerFunctions(). Only the
    // non-duplicated _PrintMessage remains here (its extended copy in
    // lua_file_funcs.cpp is not reachable in the gamemode Lua state).
    static const LuaBinding bindings[] = {
        {"_PrintMessage", lua_PrintMessage},
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
    // Only _PhysSetVelocity is unique here. _PhysSetMass/_PhysGetMass/_PhysEnableMotion/
    // _PhysApplyForce are re-registered by lua_physics_funcs.cpp (win) and additionally
    // re-pointed to the _phys table by includes/backcompat.lua, so the core copies are dead.
    static const LuaBinding bindings[] = {
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
    // Note: GetVelocity/SetVelocity were bmod-only _phys members not present in the
    // original gmod _phys table; removed for exact parity (0 content use). The handlers
    // remain defined (see lua_phys_GetVelocity/SetVelocity) for potential internal use.

    lua_setglobal(s_pLuaState, "_phys");

    DevMsg("Lua: Registered %d physics functions + _phys table\n", registered);
}

void CGModLuaSystem::RegisterUtilityFunctions()
{
    // _CurTime is re-registered (and wins) by lua_player_funcs.cpp; removed here.
    static const LuaBinding bindings[] = {
        {"_RunString", lua_RunString},
        {"_Msg", lua_Msg},
        {"_OpenScript", lua_OpenScript},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d utility functions\n", registered);
}

void CGModLuaSystem::RegisterGamemodeFunctions()
{
    // Only AddThinkFunction (drives the timer/think system) and _TeamGetName are unique.
    // The rest are re-registered (and win) by lua_player_funcs.cpp / lua_gameevent_funcs.cpp.
    static const LuaBinding bindings[] = {
        {"AddThinkFunction", lua_AddThinkFunction},
        {"_TeamGetName", lua_TeamGetName},
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
    // The primary _GModText_* names and most no-underscore aliases are re-registered
    // (and win) by lua_effect_funcs.cpp's RegisterLuaEffectFunctions(). Only these four
    // no-underscore aliases are NOT covered there, so they remain here.
    static const LuaBinding bindings[] = {
        {"_GModTextSetTime", lua_GModText_SetTime},
        {"_GModTextSetEntity", lua_GModText_SetEntity},
        {"_GModTextSetEntityOffset", lua_GModText_SetEntityOffset},
        {"_GModTextSendAnimate", lua_GModText_SendAnimate},
    };

    int registered = RegisterLuaBindings(s_pLuaState, bindings);
    DevMsg("Lua: Registered %d _GModText functions\n", registered);
}

static int lua_GModRect_SetAdditive(lua_State* L)
{
    (void)L;
    return 0;
}

static int lua_GModRect_SetEntity(lua_State* L)
{
    (void)L;
    return 0;
}

void CGModLuaSystem::RegisterGModRectFunctions()
{
    // The primary _GModRect_* names and most no-underscore aliases are re-registered
    // (and win) by lua_effect_funcs.cpp's RegisterLuaEffectFunctions(). Only these eight
    // (SetAdditive/SetEntity variants + a few no-underscore aliases) are NOT covered there.
    static const LuaBinding bindings[] = {
        {"_GModRect_SetAdditive", lua_GModRect_SetAdditive},
        {"_GModRect_SetEntity", lua_GModRect_SetEntity},
        {"_GModRectStart", lua_GModRect_Start},
        {"_GModRectSetAdditive", lua_GModRect_SetAdditive},
        {"_GModRectSetEntity", lua_GModRect_SetEntity},
        {"_GModRectSetTime", lua_GModRect_SetTime},
        {"_GModRectSetDelay", lua_GModRect_SetDelay},
        {"_GModRectSendAnimate", lua_GModRect_SendAnimate},
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
    LoadSWEPScripts();

    DevMsg("Loaded all Lua scripts\n");
}

void CGModLuaSystem::LoadIncludeScripts()
{
    // Keep this in the same order as lua/init/init.lua. Timers depend on
    // luathink.lua, and melonracer depends on player.lua helpers.
    LoadScript("includes/defines.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/concommands.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/backcompat.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/vector3.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/luathink.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/player.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/misc.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/events.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/timers.lua", LUA_SCRIPT_INCLUDE);
    LoadScript("includes/eventhook.lua", LUA_SCRIPT_INCLUDE);
}

void CGModLuaSystem::LoadGamemodeScripts()
{
    // Legacy gamemodes install global callbacks. Loading all of them makes the
    // last script win, so CGModGamemodeSystem loads only the active gamemode.
    DevMsg("Lua gamemode autoload skipped; active gamemode system owns gamemode scripts\n");
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

void CGModLuaSystem::RunFrameLuaThink()
{
    if (!s_bSystemInitialized || !gmod_lua_enabled.GetBool() || !s_pLuaState)
        return;

    static int s_nLastThinkTick = -1;
    if (gpGlobals && gpGlobals->tickcount == s_nLastThinkTick)
        return;

    s_nLastThinkTick = gpGlobals ? gpGlobals->tickcount : s_nLastThinkTick;

    static int s_nLoggedThinkPump = 0;
    if (s_nLoggedThinkPump < 4)
    {
        DevMsg("GMod Lua: running frame think tick %d\n", gpGlobals ? gpGlobals->tickcount : -1);
        ++s_nLoggedThinkPump;
    }

    // Mirrors the beta server's player think hook: run registered Lua think
    // callbacks first, then the active gamemode's per-frame rule callback.
    // NOTE: do NOT also call DoTimers directly here. includes/timers.lua self-registers
    // DoTimers via AddThinkFunction(), so DoLuaThinkFunctions already runs it exactly once
    // per tick. Calling it a second time double-runs every timer; for a timer whose func
    // errors (e.g. a binding missing from the gamemode Lua state), timers.lua:37 aborts
    // before it stamps Timer.time, so the timer re-fires every tick and a direct second
    // call doubles the error rate, hitting gmod_lua_maxerrors twice as fast and disabling
    // the whole Lua system.
    CallGamemodeFunction("DoLuaThinkFunctions");

    CallGamemodeFunction("gamerulesThink");
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
    if (s_bSystemInitialized && s_pLuaState)
        return true;

    LuaFunctionResult_t result = InitializeLua();
    if (result != LUA_RESULT_SUCCESS)
        return false;

    s_LoadedScripts.Purge();
    s_ErrorCount = 0;
    s_bSystemInitialized = true;

    if (gmod_lua_autoload.GetBool())
    {
        LoadAllScripts();
    }

    return true;
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

    // Register beta GMod Lua constants as a fallback. includes/defines.lua
    // normally owns these values; keep this path numerically compatible with it.
    lua_pushinteger(s_pLuaState, 0);
    lua_setglobal(s_pLuaState, "OBS_MODE_NONE");
    lua_pushinteger(s_pLuaState, 1);
    lua_setglobal(s_pLuaState, "OBS_MODE_DEATHCAM");
    lua_pushinteger(s_pLuaState, 2);
    lua_setglobal(s_pLuaState, "OBS_MODE_FIXED");
    lua_pushinteger(s_pLuaState, 3);
    lua_setglobal(s_pLuaState, "OBS_MODE_IN_EYE");
    lua_pushinteger(s_pLuaState, 4);
    lua_setglobal(s_pLuaState, "OBS_MODE_CHASE");
    lua_pushinteger(s_pLuaState, 5);
    lua_setglobal(s_pLuaState, "OBS_MODE_ROAMING");

    // Input keys (used by _PlayerIsKeyDown)
    lua_pushinteger(s_pLuaState, IN_ATTACK);
    lua_setglobal(s_pLuaState, "IN_ATTACK");
    lua_pushinteger(s_pLuaState, IN_ATTACK2);
    lua_setglobal(s_pLuaState, "IN_ATTACK2");
    lua_pushinteger(s_pLuaState, IN_JUMP);
    lua_setglobal(s_pLuaState, "IN_JUMP");
    lua_pushinteger(s_pLuaState, IN_DUCK);
    lua_setglobal(s_pLuaState, "IN_DUCK");
    lua_pushinteger(s_pLuaState, IN_FORWARD);
    lua_setglobal(s_pLuaState, "IN_FORWARD");
    lua_pushinteger(s_pLuaState, IN_BACK);
    lua_setglobal(s_pLuaState, "IN_BACK");
    lua_pushinteger(s_pLuaState, IN_MOVELEFT);
    lua_setglobal(s_pLuaState, "IN_MOVELEFT");
    lua_pushinteger(s_pLuaState, IN_MOVERIGHT);
    lua_setglobal(s_pLuaState, "IN_MOVERIGHT");
    lua_pushinteger(s_pLuaState, IN_USE);
    lua_setglobal(s_pLuaState, "IN_USE");
    lua_pushinteger(s_pLuaState, IN_RELOAD);
    lua_setglobal(s_pLuaState, "IN_RELOAD");
    lua_pushinteger(s_pLuaState, IN_SPEED);
    lua_setglobal(s_pLuaState, "IN_SPEED");
    lua_pushinteger(s_pLuaState, IN_WALK);
    lua_setglobal(s_pLuaState, "IN_WALK");

    // Team numbers from beta includes/defines.lua.
    lua_pushinteger(s_pLuaState, 0);
    lua_setglobal(s_pLuaState, "TEAM_UNASSIGNED");
    lua_pushinteger(s_pLuaState, 1);
    lua_setglobal(s_pLuaState, "TEAM_SPECTATOR");
    lua_pushinteger(s_pLuaState, 2);
    lua_setglobal(s_pLuaState, "TEAM_BLUE");
    lua_pushinteger(s_pLuaState, 3);
    lua_setglobal(s_pLuaState, "TEAM_YELLOW");
    lua_pushinteger(s_pLuaState, 4);
    lua_setglobal(s_pLuaState, "TEAM_GREEN");
    lua_pushinteger(s_pLuaState, 5);
    lua_setglobal(s_pLuaState, "TEAM_RED");

    DevMsg("Lua: Registered game constants (OBS_MODE, IN_*, TEAM_*)\n");
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
        lua_pushnil(L);
        return 1;
    }

    int playerid = (int)lua_tonumber(L, 1);
    const char* info = lua_tostring(L, 2);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    LuaPushPlayerInfoField(L, pPlayer, info);
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

    LuaPushVector(L, shootPos);
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

    // Convert eye angles to forward direction vector for movement
    // MelonRacer uses this to apply forces in the direction player is looking
    Vector forward;
    AngleVectors(shootAng, &forward);

    // Create angle/vector table with both naming conventions for compatibility
    // GMod 9 scripts like MelonRacer expect x/y/z
    lua_newtable(L);

    // Traditional angle names
    lua_pushnumber(L, shootAng.x);
    lua_setfield(L, -2, "pitch");
    lua_pushnumber(L, shootAng.y);
    lua_setfield(L, -2, "yaw");
    lua_pushnumber(L, shootAng.z);
    lua_setfield(L, -2, "roll");

    // GMod 9 vector-style names (forward direction) - used by MelonRacer
    lua_pushnumber(L, forward.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, forward.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, forward.z);
    lua_setfield(L, -2, "z");

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

    const char* sourceClassname = classname;
    if (Q_stricmp(classname, "physics_prop") == 0)
    {
        sourceClassname = "prop_physics";
    }

    CBaseEntity* pEntity = CreateEntityByName(sourceClassname);
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

    Vector pos;
    if (LuaGetVector(L, 2, pos))
        pEntity->SetAbsOrigin(pos);

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

	CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(pEntity);
	if (pPlayer)
	{
		bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
		CBaseEntity::SetAllowPrecache(true);
		pPlayer->Spawn();
		CBaseEntity::SetAllowPrecache(bAllowPrecache);
		DevMsg("GMod Lua: _EntSpawn respawned player %d on team %d\n", entid, pPlayer->GetTeamNumber());
		return 0;
	}

	bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache(true);
	DispatchSpawn(pEntity);
    CBaseEntity::SetAllowPrecache(bAllowPrecache);

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

// Spectator functions for melonracer and other gamemodes
int lua_PlayerSpectatorStart(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    int obsmode = LuaObserverModeToSource((int)lua_tonumber(L, 2));

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    static int s_nLoggedSpectatorStart = 0;
    if (s_nLoggedSpectatorStart < 8)
    {
        DevMsg("GMod Lua: _PlayerSpectatorStart player %d mode %d\n", playerid, obsmode);
        ++s_nLoggedSpectatorStart;
    }

    if ( !pPlayer->IsObserver() )
    {
        pPlayer->StartObserverMode(pPlayer->GetAbsOrigin(), pPlayer->GetAbsAngles());
    }
    pPlayer->SetObserverMode(obsmode);
    return 0;
}

int lua_PlayerSpectatorEnd(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    // End spectator mode by respawning
    pPlayer->StopObserverMode();
    return 0;
}

int lua_PlayerSpectatorTarget(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    int targetid = (int)lua_tonumber(L, 2);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    if (targetid <= 0)
    {
        pPlayer->m_hObserverTarget.Set(NULL);
        pPlayer->m_bAllowNonPlayerObserverTarget = false;
        return 0;
    }

    CBaseEntity* pTarget = UTIL_EntityByIndex(targetid);
    if (pTarget)
    {
        if (!pPlayer->IsObserver())
        {
            pPlayer->StartObserverMode(pPlayer->GetAbsOrigin(), pPlayer->GetAbsAngles());
        }

        pPlayer->m_bAllowNonPlayerObserverTarget = !pTarget->IsPlayer();
        if (!pPlayer->SetObserverTarget(pTarget))
        {
            pPlayer->m_bAllowNonPlayerObserverTarget = !pTarget->IsPlayer();
            pPlayer->m_hObserverTarget.Set(pTarget);
        }

        if (pPlayer->m_iObserverMode != OBS_MODE_CHASE && pPlayer->m_iObserverMode != OBS_MODE_IN_EYE)
        {
            pPlayer->SetObserverMode(OBS_MODE_CHASE);
        }

        static int s_nLoggedSpectatorTarget = 0;
        if (s_nLoggedSpectatorTarget < 8)
        {
            DevMsg("GMod Lua CORE: _PlayerSpectatorTarget player %d target %d (%s)\n",
                playerid, targetid, pTarget->GetClassname());
            ++s_nLoggedSpectatorTarget;
        }
    }
    return 0;
}

int lua_PlayerSetChaseCamDistance(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    float distance = (float)lua_tonumber(L, 2);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    if (distance < 10.0f)
        distance = 10.0f;
    if (distance > 500.0f)
        distance = 500.0f;

    pPlayer->m_flChaseCamDistance = distance;
    return 0;
}

int lua_PlayerRespawn(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    pPlayer->Spawn();
    return 0;
}

int lua_PlayerAddDeath(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    int deaths = (int)lua_tonumber(L, 2);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    pPlayer->IncrementDeathCount(deaths);
    return 0;
}

int lua_PlayerAddScore(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    int score = (int)lua_tonumber(L, 2);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    pPlayer->IncrementFragCount(score);
    return 0;
}

int lua_PlayerSilentKill(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
        return 0;

    // Kill player silently (no death sound/notification)
    pPlayer->CommitSuicide();
    return 0;
}

int lua_PlayerIsKeyDown(lua_State* L)
{
    if (lua_gettop(L) < 2)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    int playerid = (int)lua_tonumber(L, 1);
    int key = (int)lua_tonumber(L, 2);

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (!pPlayer)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    // Source observer/movement code can clear m_nButtons after processing.
    // GMod Lua wants the raw command button state for this frame.
    int rawButtons = CGModLuaSystem::GetPlayerRawButtons(pPlayer);
    bool isDown = ((rawButtons | pPlayer->m_nButtons) & key) != 0;
    lua_pushboolean(L, isDown);
    return 1;
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

    Vector start, end;
    int ignoreEnt = -1;

    if (lua_istable(L, 1))
    {
        Vector direction;
        if (!LuaGetVector(L, 1, start) || !LuaGetVector(L, 2, direction))
            return 0;

        float distance = (float)lua_tonumber(L, 3);
        end = start + direction * distance;
        if (lua_gettop(L) >= 4)
            ignoreEnt = (int)lua_tonumber(L, 4);
    }
    else
    {
        if (!LuaGetVector(L, 1, start) || !LuaGetVector(L, 4, end))
            return 0;

        if (lua_gettop(L) >= 7)
            ignoreEnt = (int)lua_tonumber(L, 7);
    }

    CBaseEntity* pIgnore = ignoreEnt > 0 ? UTIL_EntityByIndex(ignoreEnt) : NULL;
    CTraceFilterSimple filter(pIgnore, COLLISION_GROUP_NONE);
    trace_t tr;
    UTIL_TraceLine(start, end, MASK_SOLID, &filter, &tr);

    g_LegacyLuaTrace = tr;

    lua_pushboolean(L, tr.DidHit());
    return 1;
}

int lua_TraceEndPos(lua_State* L)
{
    LuaPushVector(L, g_LegacyLuaTrace.endpos);
    return 1;
}

int lua_TraceHit(lua_State* L)
{
    lua_pushboolean(L, g_LegacyLuaTrace.DidHit());
    return 1;
}

int lua_TraceHitWorld(lua_State* L)
{
    lua_pushboolean(L, g_LegacyLuaTrace.DidHitWorld());
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
        // TEMP diagnostic: track melon freeze/unfreeze so we can see if RaceStart ever un-pins it.
        DevMsg( "GMod Phys EnableMotion(_phys): ent %d enable=%d (was moveable=%d)\n",
            (int)lua_tonumber(L, 1), enable ? 1 : 0, pPhys->IsMoveable() ? 1 : 0 );
        pPhys->EnableMotion(enable);
        // A frozen (pinned) object sleeps and then ignores async forces; wake it on
        // unfreeze so applied force takes effect (mirrors CPhysicsProp::EnableMotion).
        if (enable)
            pPhys->Wake();
    }
    return 0;
}

int lua_phys_ApplyForceCenter(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

	int entIndex = (int)lua_tonumber(L, 1);
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

		static int s_nLoggedApplyForceCenter = 0;
		if (s_nLoggedApplyForceCenter < 16)
		{
			DevMsg("GMod Lua: _phys.ApplyForceCenter ent %d force (%.1f %.1f %.1f)\n",
				entIndex, force.x, force.y, force.z);
			++s_nLoggedApplyForceCenter;
		}

		// TEMP diagnostic: capture the melon's actual physics state + whether the force changes its
		// velocity. Tells us if the object is frozen (IsMoveable=0 -> ApplyForceCenter returns early),
		// asleep, has an absurd mass, or if the velocity simply isn't changing.
		// Sample OVER TIME (first 5, then every 30th call) so we capture the melon's state after the
		// "GO!!" un-freeze, not just the first ~4s of frozen intermission.
		static int s_physCallCount = 0;
		static int s_physDbg = 0;
		++s_physCallCount;
		if ( s_physDbg < 40 && ( s_physCallCount <= 5 || (s_physCallCount % 30) == 0 ) )
		{
			Vector velBefore( 0, 0, 0 ), velAfter( 0, 0, 0 );
			pPhys->GetVelocity( &velBefore, NULL );
			pPhys->ApplyForceCenter(force);
			pPhys->GetVelocity( &velAfter, NULL );
			DevMsg( "GMod Phys DBG #%d: ent %d moveable=%d asleep=%d mass=%.1f vel (%.1f %.1f %.1f)->(%.1f %.1f %.1f)\n",
				s_physCallCount, entIndex, pPhys->IsMoveable() ? 1 : 0, pPhys->IsAsleep() ? 1 : 0, pPhys->GetMass(),
				velBefore.x, velBefore.y, velBefore.z, velAfter.x, velAfter.y, velAfter.z );
			++s_physDbg;
		}
		else
		{
			pPhys->ApplyForceCenter(force);
		}

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

    // Match includes/luathink.lua so callbacks registered through the C++
    // fallback are still executed by DoLuaThinkFunctions.
    lua_getglobal(L, "gLuaThinkFunctions");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "gLuaThinkFunctions");
        lua_getglobal(L, "gLuaThinkFunctions");
    }

    // Get the next index (using luaL_getn for Lua 5.0 compatibility)
    int len = luaL_getn(L, -1);

    // Push the function reference (duplicate from arg 1)
    lua_pushvalue(L, 1);

    // Store it at index len+1
    lua_rawseti(L, -2, len + 1);

    lua_pop(L, 1); // pop gLuaThinkFunctions table
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
    int entityID;
    float entityOffsetX, entityOffsetY, entityOffsetZ;
} s_GModText = {"Default", "", 0.5f, 0.5f, 255, 255, 255, 255, 0.1f, 0.1f, 5.0f, 0, 0, 0, 0, 0, 0};

static const char *ResolveGModTextFontName(const char *fontName)
{
    if (!fontName || !*fontName)
        return "Default";

    if (!Q_stricmp(fontName, "ImpactMassive") || !Q_stricmp(fontName, "TrebuchetMassive"))
        return "DefaultShadow";

    if (!Q_strnicmp(fontName, "Impact", 6) || !Q_strnicmp(fontName, "Trebuchet", 9))
        return "DefaultShadow";

    return fontName;
}

int lua_GModText_Start(lua_State* L)
{
    const char* font = lua_gettop(L) >= 1 ? lua_tostring(L, 1) : "Default";
    Q_strncpy(s_GModText.fontName, ResolveGModTextFontName(font), sizeof(s_GModText.fontName));
    s_GModText.text[0] = '\0';
    s_GModText.entityID = 0;
    s_GModText.entityOffsetX = 0.0f;
    s_GModText.entityOffsetY = 0.0f;
    s_GModText.entityOffsetZ = 0.0f;
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

int lua_GModText_SetTime(lua_State* L)
{
    int n = lua_gettop(L);
    if (n >= 1)
        s_GModText.holdTime = (float)lua_tonumber(L, 1);
    if (n >= 2)
        s_GModText.fadeIn = (float)lua_tonumber(L, 2);
    if (n >= 3)
        s_GModText.fadeOut = (float)lua_tonumber(L, 3);
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

int lua_GModText_SetEntity(lua_State* L)
{
    if (lua_gettop(L) >= 1)
        s_GModText.entityID = (int)lua_tonumber(L, 1);
    return 0;
}

int lua_GModText_SetEntityOffset(lua_State* L)
{
    Vector offset;
    if (LuaGetVector(L, 1, offset))
    {
        s_GModText.entityOffsetX = offset.x;
        s_GModText.entityOffsetY = offset.y;
        s_GModText.entityOffsetZ = offset.z;
    }
    return 0;
}

int lua_GModText_Send(lua_State* L)
{
    int playerID = lua_gettop(L) >= 1 ? (int)lua_tonumber(L, 1) : -1;
    int textID = lua_gettop(L) >= 2 ? (int)lua_tonumber(L, 2) : 0;

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
        WRITE_SHORT(textID);
        WRITE_STRING(s_GModText.fontName);
        WRITE_STRING(s_GModText.text);
        WRITE_FLOAT(s_GModText.x);
        WRITE_FLOAT(s_GModText.y);
        WRITE_BYTE((int)s_GModText.r);
        WRITE_BYTE((int)s_GModText.g);
        WRITE_BYTE((int)s_GModText.b);
        WRITE_BYTE((int)s_GModText.a);
        WRITE_FLOAT(s_GModText.fadeIn);
        WRITE_FLOAT(s_GModText.fadeOut);
        WRITE_FLOAT(s_GModText.holdTime);
        WRITE_BYTE(s_GModText.effect);
        WRITE_SHORT(s_GModText.entityID);
        WRITE_FLOAT(s_GModText.entityOffsetX);
        WRITE_FLOAT(s_GModText.entityOffsetY);
        WRITE_FLOAT(s_GModText.entityOffsetZ);
    MessageEnd();

    return 0;
}

int lua_GModText_SendAnimate(lua_State* L)
{
    int playerID = lua_gettop(L) >= 1 ? (int)lua_tonumber(L, 1) : -1;
    int textID = lua_gettop(L) >= 2 ? (int)lua_tonumber(L, 2) : 0;
    float scale = lua_gettop(L) >= 3 ? (float)lua_tonumber(L, 3) : 1.0f;
    float duration = lua_gettop(L) >= 4 ? (float)lua_tonumber(L, 4) : 0.5f;

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

    UserMessageBegin(filter, "GModTextAnimate");
        WRITE_SHORT(textID);
        WRITE_FLOAT(s_GModText.x);
        WRITE_FLOAT(s_GModText.y);
        WRITE_BYTE((int)s_GModText.r);
        WRITE_BYTE((int)s_GModText.g);
        WRITE_BYTE((int)s_GModText.b);
        WRITE_BYTE((int)s_GModText.a);
        WRITE_FLOAT(scale);
        WRITE_FLOAT(duration);
    MessageEnd();

    return 0;
}

int lua_GModText_Hide(lua_State* L)
{
    int playerID = lua_gettop(L) >= 1 ? (int)lua_tonumber(L, 1) : -1;
    int textID = lua_gettop(L) >= 2 ? (int)lua_tonumber(L, 2) : 0;
    float fadeTime = lua_gettop(L) >= 3 ? (float)lua_tonumber(L, 3) : 0.0f;
    float delay = lua_gettop(L) >= 4 ? (float)lua_tonumber(L, 4) : 0.0f;

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

    UserMessageBegin(filter, "GModTextHide");
        WRITE_SHORT(textID);
        WRITE_FLOAT(fadeTime);
        WRITE_FLOAT(delay);
    MessageEnd();

    return 0;
}

//-----------------------------------------------------------------------------
// GModRect functions - Display rectangles on screen for players
//-----------------------------------------------------------------------------

static struct GModRect_t {
    char material[256];
    float x, y;
    float w, h;
    float r, g, b, a;
    float holdTime, fadeIn, fadeOut;
    float delay;
    int id;
} s_GModRect = {"", 0, 0, 100, 100, 255, 255, 255, 255, 5.0f, 0.1f, 0.1f, 0, 0};

int lua_GModRect_Start(lua_State* L)
{
    const char *material = lua_gettop(L) >= 1 ? lua_tostring(L, 1) : "";

    // Reset rect state
    Q_strncpy(s_GModRect.material, material ? material : "", sizeof(s_GModRect.material));
    s_GModRect.x = 0;
    s_GModRect.y = 0;
    s_GModRect.w = 100;
    s_GModRect.h = 100;
    s_GModRect.r = 255;
    s_GModRect.g = 255;
    s_GModRect.b = 255;
    s_GModRect.a = 255;
    s_GModRect.holdTime = 5.0f;
    s_GModRect.fadeIn = 0.1f;
    s_GModRect.fadeOut = 0.1f;
    s_GModRect.delay = 0.0f;
    s_GModRect.id = 0;
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

int lua_GModRect_SetTime(lua_State* L)
{
    int n = lua_gettop(L);
    if (n >= 1)
        s_GModRect.holdTime = (float)lua_tonumber(L, 1);
    if (n >= 2)
        s_GModRect.fadeIn = (float)lua_tonumber(L, 2);
    if (n >= 3)
        s_GModRect.fadeOut = (float)lua_tonumber(L, 3);
    return 0;
}

int lua_GModRect_SetDelay(lua_State* L)
{
    if (lua_gettop(L) >= 1)
        s_GModRect.delay = (float)lua_tonumber(L, 1);
    return 0;
}

int lua_GModRect_Send(lua_State* L)
{
    int playerID = lua_gettop(L) >= 1 ? (int)lua_tonumber(L, 1) : -1;
    int rectID = lua_gettop(L) >= 2 ? (int)lua_tonumber(L, 2) : s_GModRect.id;

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
        WRITE_SHORT(rectID);
        WRITE_STRING(s_GModRect.material);
        WRITE_FLOAT(s_GModRect.x);
        WRITE_FLOAT(s_GModRect.y);
        WRITE_FLOAT(s_GModRect.w);
        WRITE_FLOAT(s_GModRect.h);
        WRITE_BYTE((int)s_GModRect.r);
        WRITE_BYTE((int)s_GModRect.g);
        WRITE_BYTE((int)s_GModRect.b);
        WRITE_BYTE((int)s_GModRect.a);
        WRITE_FLOAT(s_GModRect.holdTime);
        WRITE_FLOAT(s_GModRect.fadeIn);
        WRITE_FLOAT(s_GModRect.fadeOut);
        WRITE_FLOAT(s_GModRect.delay);
    MessageEnd();

    return 0;
}

int lua_GModRect_SendAnimate(lua_State* L)
{
    int playerID = lua_gettop(L) >= 1 ? (int)lua_tonumber(L, 1) : -1;
    int rectID = lua_gettop(L) >= 2 ? (int)lua_tonumber(L, 2) : s_GModRect.id;
    float targetX = lua_gettop(L) >= 3 ? (float)lua_tonumber(L, 3) : s_GModRect.x;
    float targetY = lua_gettop(L) >= 4 ? (float)lua_tonumber(L, 4) : s_GModRect.y;

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

    UserMessageBegin(filter, "GModRectAnimate");
        WRITE_SHORT(rectID);
        WRITE_FLOAT(s_GModRect.x);
        WRITE_FLOAT(s_GModRect.y);
        WRITE_FLOAT(s_GModRect.w);
        WRITE_FLOAT(s_GModRect.h);
        WRITE_BYTE((int)s_GModRect.r);
        WRITE_BYTE((int)s_GModRect.g);
        WRITE_BYTE((int)s_GModRect.b);
        WRITE_BYTE((int)s_GModRect.a);
        WRITE_FLOAT(targetX);
        WRITE_FLOAT(targetY);
        WRITE_FLOAT(s_GModRect.delay);
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
