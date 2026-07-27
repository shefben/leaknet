#include "gmod_gamemode.h"
#include "cbase.h"
#include "player.h"
#include "team.h"
#include "filesystem.h"
#include "gmod_lua.h"
#include "in_buttons.h"
#include <string.h>
#include "tier0/memdbgon.h"

// Static member definitions
CUtlVector<GamemodeData_t> CGModGamemodeSystem::s_GamemodeRegistry;
GamemodeData_t* CGModGamemodeSystem::s_pActiveGamemode = NULL;
CGModGamemodeSystem::GamemodeState_t CGModGamemodeSystem::s_GamemodeState;
TargetIDRules_t CGModGamemodeSystem::s_TargetIDRules = TARGETID_ALWAYS;
char CGModGamemodeSystem::s_TeamNames[32][64] = { 0 };
CUtlVector<int> CGModGamemodeSystem::s_TeamScores;
bool CGModGamemodeSystem::s_bSystemInitialized = false;

// Global instance
CGModGamemodeSystem g_GMod_GamemodeSystem;

// ConVars for gamemode system configuration
ConVar gmod_gamemode_enabled("gmod_gamemode_enabled", "1", FCVAR_GAMEDLL, "Enable/disable gamemode system");
ConVar gmod_gamemode_debug("gmod_gamemode_debug", "0", FCVAR_GAMEDLL, "Debug gamemode system");
ConVar gmod_gamemode_autoload("gmod_gamemode_autoload", "1", FCVAR_GAMEDLL, "Automatically load gamemodes on startup");
ConVar gmod_gamemode_current("gmod_gamemode_current", "build", FCVAR_GAMEDLL, "Current active gamemode");
ConVar gmod_gamemode_path("gmod_gamemode_path", "lua/gamemodes/", FCVAR_GAMEDLL, "Path to gamemode scripts");

//-----------------------------------------------------------------------------
// Helper function to get player from console command
//-----------------------------------------------------------------------------
static int GModTeamToSourceTeam( int iTeam )
{
	switch ( iTeam )
	{
	case 0:
		return TEAM_UNASSIGNED;
	case 1:
		return TEAM_SPECTATOR;
	default:
		return iTeam + 1;
	}
}

static const char *GetDefaultGModTeamName( int iTeam )
{
	switch ( iTeam )
	{
	case TEAM_UNASSIGNED:
		return "Unassigned";
	case TEAM_SPECTATOR:
		return "Spectators";
	case 3:
		return "Blue Team";
	case 4:
		return "Yellow Team";
	case 5:
		return "Green Team";
	case 6:
		return "Red Team";
	default:
		return "Unassigned";
	}
}

static void EnsureGModTeamManagers()
{
	for ( int iTeam = TEAM_UNASSIGNED; iTeam <= GModTeamToSourceTeam( 5 ); ++iTeam )
	{
		if ( GetGlobalTeam( iTeam ) )
			continue;

		while ( GetNumberOfTeams() < iTeam )
		{
			int iNewTeam = GetNumberOfTeams() + 1;
			CTeam *pTeam = (CTeam *)CreateEntityByName( "team_manager" );
			if ( !pTeam )
			{
				Warning( "GMod Gamemode System: failed to create team_manager for team %d\n", iNewTeam );
				return;
			}

			pTeam->Init( GetDefaultGModTeamName( iNewTeam ), iNewTeam );
			g_Teams.AddToTail( pTeam );
			DevMsg( "GMod Gamemode System: created team_manager %d (%s)\n", iNewTeam, GetDefaultGModTeamName( iNewTeam ) );
		}
	}
}

static CBasePlayer* GetCommandPlayer()
{
    if (!UTIL_GetCommandClient())
        return NULL;

    return dynamic_cast<CBasePlayer*>(UTIL_GetCommandClient());
}

static bool CallLuaPlayerFunction(const char* pszFunction, CBasePlayer* pPlayer)
{
    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L || !pszFunction || !pPlayer)
        return false;

    int stackTop = lua_gettop(L);
    lua_getglobal(L, pszFunction);
    if (!lua_isfunction(L, -1))
    {
        lua_settop(L, stackTop);
        return false;
    }

    lua_pushnumber(L, pPlayer->entindex());
    int result = lua_pcall(L, 1, 0, 0);
    if (result != 0)
    {
        CGModLuaSystem::HandleLuaError(L, pszFunction);
        lua_settop(L, stackTop);
        return false;
    }

    lua_settop(L, stackTop);
    return true;
}

static bool CallLuaPlayerBoolFunction(const char* pszFunction, CBasePlayer* pPlayer, bool& bReturn)
{
    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L || !pszFunction || !pPlayer)
        return false;

    int stackTop = lua_gettop(L);
    lua_getglobal(L, pszFunction);
    if (!lua_isfunction(L, -1))
    {
        lua_settop(L, stackTop);
        return false;
    }

    lua_pushnumber(L, pPlayer->entindex());
    int result = lua_pcall(L, 1, 1, 0);
    if (result != 0)
    {
        CGModLuaSystem::HandleLuaError(L, pszFunction);
        lua_settop(L, stackTop);
        return false;
    }

    bReturn = lua_toboolean(L, -1) != 0;
    lua_settop(L, stackTop);
    return true;
}

static void CallLuaPlayerEventHook(const char* pszHookName, CBasePlayer* pPlayer)
{
    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L || !pszHookName || !pPlayer)
        return;

    int stackTop = lua_gettop(L);
    lua_getglobal(L, "DoEventHook");
    if (!lua_isfunction(L, -1))
    {
        lua_settop(L, stackTop);
        return;
    }

    lua_pushstring(L, pszHookName);
    lua_pushnumber(L, pPlayer->entindex());
    int result = lua_pcall(L, 2, 0, 0);
    if (result != 0)
    {
        CGModLuaSystem::HandleLuaError(L, "DoEventHook");
    }

    lua_settop(L, stackTop);
}

static void CallLuaPlayerKeyFunction(const char* pszFunction, CBasePlayer* pPlayer, int button)
{
    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L || !pszFunction || !pPlayer)
        return;

    int stackTop = lua_gettop(L);
    lua_getglobal(L, pszFunction);
    if (!lua_isfunction(L, -1))
    {
        lua_settop(L, stackTop);
        return;
    }

    lua_pushnumber(L, pPlayer->entindex());
    lua_pushnumber(L, button);
    int result = lua_pcall(L, 2, 0, 0);
    if (result != 0)
    {
        CGModLuaSystem::HandleLuaError(L, pszFunction);
    }

    lua_settop(L, stackTop);
}

static void CallLuaKeyEventHook(const char* pszHookName, CBasePlayer* pPlayer, int button)
{
    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L || !pszHookName || !pPlayer)
        return;

    int stackTop = lua_gettop(L);
    lua_getglobal(L, "DoEventHook");
    if (!lua_isfunction(L, -1))
    {
        lua_settop(L, stackTop);
        return;
    }

    lua_pushstring(L, pszHookName);
    lua_pushnumber(L, pPlayer->entindex());
    lua_pushnumber(L, button);
    int result = lua_pcall(L, 3, 0, 0);
    if (result != 0)
    {
        CGModLuaSystem::HandleLuaError(L, "DoEventHook");
    }

    lua_settop(L, stackTop);
}

//-----------------------------------------------------------------------------
// CGModGamemodeSystem implementation
//-----------------------------------------------------------------------------
bool CGModGamemodeSystem::Init()
{
    if (!gmod_gamemode_enabled.GetBool())
    {
        DevMsg("GMod Gamemode System disabled by ConVar\n");
        return true;
    }

    s_GamemodeRegistry.Purge();
    s_pActiveGamemode = NULL;
    memset(s_TeamNames, 0, sizeof(s_TeamNames));
    s_TeamScores.Purge();

    InitializeGamemodeDefaults();
    SetupDefaultTeams();
    s_bSystemInitialized = true;

    if (gmod_gamemode_autoload.GetBool())
    {
        LoadAllGamemodes();
    }

    DevMsg("GMod Gamemode System initialized with %d gamemodes\n", s_GamemodeRegistry.Count());
    return true;
}

void CGModGamemodeSystem::Shutdown()
{
    if (s_pActiveGamemode)
    {
        CallGamemodeFunction("onGamemodeEnd");
        s_pActiveGamemode = NULL;
    }

    s_GamemodeRegistry.Purge();
    memset(s_TeamNames, 0, sizeof(s_TeamNames));
    s_TeamScores.Purge();
    s_bSystemInitialized = false;

    DevMsg("GMod Gamemode System shutdown\n");
}

void CGModGamemodeSystem::LevelInitPreEntity()
{
    if (!s_bSystemInitialized)
        return;

    DevMsg("GMod Gamemode System: Level init pre-entity\n");
}

void CGModGamemodeSystem::LevelInitPostEntity()
{
    if (!s_bSystemInitialized)
        return;

    EnsureGModTeamManagers();

    // Map-based gamemode auto-detection
    const char* mapName = STRING(gpGlobals->mapname);
    const char* detectedGamemode = NULL;

    if (mapName && mapName[0])
    {
        // Check map name prefixes for gamemode detection
        if (Q_strnicmp(mapName, "gm_melonrace", 12) == 0 || Q_strnicmp(mapName, "gm_melonlockdown", 16) == 0)
        {
            detectedGamemode = "melonracer";
        }
        else if (Q_strnicmp(mapName, "gm_football", 11) == 0)
        {
            detectedGamemode = "football";
        }
        else if (Q_strnicmp(mapName, "gm_hideandseek", 14) == 0 || Q_strnicmp(mapName, "gm_has", 6) == 0)
        {
            detectedGamemode = "hideandseek";
        }
        else if (Q_strnicmp(mapName, "gm_laserdance", 13) == 0)
        {
            detectedGamemode = "laserdance";
        }
        else if (Q_strnicmp(mapName, "gm_longsight", 12) == 0)
        {
            detectedGamemode = "longsight";
        }
        else if (Q_strnicmp(mapName, "gm_910", 6) == 0)
        {
            detectedGamemode = "910";
        }
        else if (Q_strnicmp(mapName, "gm_tpc", 6) == 0)
        {
            detectedGamemode = "tpc";
        }
        else if (Q_strnicmp(mapName, "gm_", 3) == 0)
        {
            // Generic gm_ prefix defaults to build mode
            detectedGamemode = "build";
        }

        if (detectedGamemode)
        {
            DevMsg("GMod Gamemode System: Auto-detected gamemode '%s' from map '%s'\n", detectedGamemode, mapName);
        }
    }

    // Real GMod 9 always runs the server's currently selected gamemode
    // (default "build"/sandbox) on EVERY map, regardless of map name - that's
    // how a stock HL2 map gets the sandbox loadout when loaded in GMod. The
    // gm_ prefix detection above only picks a map-specific override; when it
    // finds nothing, fall back to whatever gamemode is already selected
    // instead of leaving s_pActiveGamemode NULL forever (that left players
    // with the vanilla HL2 weapon loadout - no physgun/toolgun - on any map
    // that doesn't start with "gm_"). See [[gamemode-map-prefix-gate]].
    const char* gamemodeToActivate = detectedGamemode ? detectedGamemode : gmod_gamemode_current.GetString();
    if (gamemodeToActivate && gamemodeToActivate[0])
    {
        if (SetActiveGamemode(gamemodeToActivate))
        {
            DevMsg("GMod Gamemode System: Activated gamemode '%s'\n", gamemodeToActivate);
        }
        else
        {
            Warning("GMod Gamemode System: Failed to activate gamemode '%s'\n", gamemodeToActivate);
        }
    }
}

void CGModGamemodeSystem::LevelShutdownPostEntity()
{
    if (!s_bSystemInitialized)
        return;

    OnMapEnd();

    DevMsg("GMod Gamemode System: Level shutdown\n");
}

void CGModGamemodeSystem::FrameUpdatePreEntityThink()
{
    if (!s_bSystemInitialized || !s_pActiveGamemode)
        return;

    // The beta server runs gamerulesThink from the Lua think path after
    // DoLuaThinkFunctions. Keep this system from issuing a second call.
}

void CGModGamemodeSystem::PostGamemodeLoad()
{
    // Hook point for post-load initialization; currently a no-op stub.
}

void CGModGamemodeSystem::UnloadGamemode()
{
    if (s_pActiveGamemode)
    {
        UnloadGamemode(s_pActiveGamemode->gamemodeName);
    }
}

bool CGModGamemodeSystem::RegisterGamemode(const char* pszGamemodeName, const char* pszScriptPath)
{
    if (!pszGamemodeName || !pszScriptPath || !s_bSystemInitialized)
        return false;

    // Check if already registered
    if (FindGamemode(pszGamemodeName))
    {
        DevMsg("Gamemode %s already registered\n", pszGamemodeName);
        return true;
    }

    // Validate script exists
    if (!ValidateGamemodeScript(pszScriptPath))
    {
        Warning("Gamemode script not found: %s\n", pszScriptPath);
        return false;
    }

    GamemodeData_t gamemodeData;
    Q_strncpy(gamemodeData.gamemodeName, pszGamemodeName, sizeof(gamemodeData.gamemodeName));
    Q_strncpy(gamemodeData.scriptPath, pszScriptPath, sizeof(gamemodeData.scriptPath));
    gamemodeData.gamemodeType = GetGamemodeTypeFromName(pszGamemodeName);

    s_GamemodeRegistry.AddToTail(gamemodeData);

    if (gmod_gamemode_debug.GetBool())
    {
        DevMsg("Registered gamemode: %s -> %s\n", pszGamemodeName, pszScriptPath);
    }

    return true;
}

bool CGModGamemodeSystem::LoadGamemode(const char* pszGamemodeName)
{
    if (!pszGamemodeName || !s_bSystemInitialized)
        return false;

    GamemodeData_t* pGamemode = FindGamemode(pszGamemodeName);
    if (!pGamemode)
    {
        Warning("Gamemode not found: %s\n", pszGamemodeName);
        return false;
    }

    if (pGamemode->isLoaded)
    {
        DevMsg("Gamemode %s already loaded\n", pszGamemodeName);
        return true;
    }

    bool success = LoadGamemodeScript(pGamemode);
    if (success)
    {
        pGamemode->isLoaded = true;
        DevMsg("GMod Gamemode System: Loaded Lua script %s\n", pGamemode->scriptPath);
        if (gmod_gamemode_debug.GetBool())
        {
            DevMsg("Loaded gamemode: %s\n", pszGamemodeName);
        }
    }
    else
    {
        Warning("Failed to load gamemode script: %s\n", pGamemode->scriptPath);
    }

    return success;
}

bool CGModGamemodeSystem::UnloadGamemode(const char* pszGamemodeName)
{
    if (!pszGamemodeName || !s_bSystemInitialized)
        return false;

    GamemodeData_t* pGamemode = FindGamemode(pszGamemodeName);
    if (!pGamemode)
        return false;

    if (s_pActiveGamemode == pGamemode)
    {
        CallGamemodeFunction("onGamemodeEnd");
        s_pActiveGamemode = NULL;
    }

    pGamemode->isLoaded = false;
    pGamemode->isActive = false;
    pGamemode->state = GAMEMODE_STATE_INACTIVE;

    if (gmod_gamemode_debug.GetBool())
    {
        DevMsg("Unloaded gamemode: %s\n", pszGamemodeName);
    }

    return true;
}

bool CGModGamemodeSystem::SetActiveGamemode(const char* pszGamemodeName)
{
    if (!pszGamemodeName || !s_bSystemInitialized)
        return false;

    GamemodeData_t* pGamemode = FindGamemode(pszGamemodeName);
    if (!pGamemode)
    {
        Warning("Gamemode not found: %s\n", pszGamemodeName);
        return false;
    }

    // End current gamemode
    if (s_pActiveGamemode && s_pActiveGamemode != pGamemode)
    {
        CallGamemodeFunction("onGamemodeEnd");
        s_pActiveGamemode->isActive = false;
        s_pActiveGamemode->state = GAMEMODE_STATE_INACTIVE;
    }

    if (!LoadGamemodeScript(pGamemode))
    {
        Warning("Failed to load gamemode: %s\n", pszGamemodeName);
        return false;
    }
    pGamemode->isLoaded = true;
    DevMsg("GMod Gamemode System: Loaded Lua script %s\n", pGamemode->scriptPath);

    // Set new active gamemode
    s_pActiveGamemode = pGamemode;
    s_pActiveGamemode->isActive = true;
    s_pActiveGamemode->state = GAMEMODE_STATE_ACTIVE;

    // Update ConVar
    gmod_gamemode_current.SetValue(pszGamemodeName);

    // Initialize gamemode
    OnMapStart();

    if (gmod_gamemode_debug.GetBool())
    {
        DevMsg("Set active gamemode: %s\n", pszGamemodeName);
    }

    return true;
}

GamemodeData_t* CGModGamemodeSystem::GetGamemodeData(const char* pszGamemodeName)
{
    if (!pszGamemodeName || !s_bSystemInitialized)
        return NULL;

    return FindGamemode(pszGamemodeName);
}

GamemodeData_t* CGModGamemodeSystem::GetActiveGamemode()
{
    return s_pActiveGamemode;
}

bool CGModGamemodeSystem::IsGamemodeActive(const char* pszGamemodeName)
{
    if (!pszGamemodeName || !s_pActiveGamemode)
        return false;

    return Q_stricmp(s_pActiveGamemode->gamemodeName, pszGamemodeName) == 0;
}

// Gamemode event implementations
void CGModGamemodeSystem::OnMapStart()
{
    if (!s_pActiveGamemode)
        return;

    s_GamemodeState.currentRound = 1;
    s_GamemodeState.roundStartTime = gpGlobals->curtime;
    s_GamemodeState.inProgress = true;

    CallGamemodeFunction("gamerulesStartMap");
    OnRoundStart();
}

void CGModGamemodeSystem::OnMapEnd()
{
    if (!s_pActiveGamemode)
        return;

    OnRoundEnd();
    s_GamemodeState.inProgress = false;

    CallGamemodeFunction("gamerulesEndMap");
}

void CGModGamemodeSystem::OnRoundStart()
{
    if (!s_pActiveGamemode)
        return;

    s_GamemodeState.roundStartTime = gpGlobals->curtime;
    CallGamemodeFunction("gamerulesRoundStart");
}

void CGModGamemodeSystem::OnRoundEnd()
{
    if (!s_pActiveGamemode)
        return;

    s_GamemodeState.roundEndTime = gpGlobals->curtime;
    CallGamemodeFunction("gamerulesRoundEnd");
}

enum GModLuaArgType_t
{
    GMOD_LUA_ARG_NUMBER = 0,
    GMOD_LUA_ARG_STRING,
    GMOD_LUA_ARG_BOOL
};

struct GModLuaArg_t
{
    GModLuaArgType_t type;
    double numberValue;
    const char* stringValue;
    bool boolValue;
};

static GModLuaArg_t LuaArgNumber(double value)
{
    GModLuaArg_t arg;
    arg.type = GMOD_LUA_ARG_NUMBER;
    arg.numberValue = value;
    arg.stringValue = "";
    arg.boolValue = false;
    return arg;
}

static GModLuaArg_t LuaArgString(const char* value)
{
    GModLuaArg_t arg;
    arg.type = GMOD_LUA_ARG_STRING;
    arg.numberValue = 0.0;
    arg.stringValue = value ? value : "";
    arg.boolValue = false;
    return arg;
}

static GModLuaArg_t LuaArgBool(bool value)
{
    GModLuaArg_t arg;
    arg.type = GMOD_LUA_ARG_BOOL;
    arg.numberValue = 0.0;
    arg.stringValue = "";
    arg.boolValue = value;
    return arg;
}

static int LuaPlayerID(CBasePlayer* pPlayer)
{
    return pPlayer ? pPlayer->entindex() : 0;
}

static int LuaEntityID(CBaseEntity* pEntity)
{
    return pEntity ? pEntity->entindex() : 0;
}

static void PushLuaArg(lua_State* L, const GModLuaArg_t& arg)
{
    switch (arg.type)
    {
    case GMOD_LUA_ARG_STRING:
        lua_pushstring(L, arg.stringValue ? arg.stringValue : "");
        break;
    case GMOD_LUA_ARG_BOOL:
        lua_pushboolean(L, arg.boolValue);
        break;
    case GMOD_LUA_ARG_NUMBER:
    default:
        lua_pushnumber(L, arg.numberValue);
        break;
    }
}

static bool PushLuaFunction(lua_State* L, const char* pszFunction, int stackTop)
{
    if (!L || !pszFunction || !pszFunction[0])
        return false;

    lua_getglobal(L, pszFunction);
    if (!lua_isfunction(L, -1))
    {
        lua_settop(L, stackTop);
        return false;
    }

    return true;
}

static bool CallLuaNoResult(const char* pszFunction, const GModLuaArg_t* pArgs, int nArgs)
{
    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L)
        return false;

    int stackTop = lua_gettop(L);
    if (!PushLuaFunction(L, pszFunction, stackTop))
        return false;

    for (int i = 0; i < nArgs; ++i)
        PushLuaArg(L, pArgs[i]);

    if (lua_pcall(L, nArgs, 0, 0) != 0)
    {
        Warning("Lua error in %s: %s\n", pszFunction, lua_tostring(L, -1));
        lua_settop(L, stackTop);
        return false;
    }

    lua_settop(L, stackTop);
    return true;
}

static bool CallLuaBoolResult(const char* pszFunction, const GModLuaArg_t* pArgs, int nArgs, bool defaultValue)
{
    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L)
        return defaultValue;

    int stackTop = lua_gettop(L);
    if (!PushLuaFunction(L, pszFunction, stackTop))
        return defaultValue;

    for (int i = 0; i < nArgs; ++i)
        PushLuaArg(L, pArgs[i]);

    if (lua_pcall(L, nArgs, 1, 0) != 0)
    {
        Warning("Lua error in %s: %s\n", pszFunction, lua_tostring(L, -1));
        lua_settop(L, stackTop);
        return defaultValue;
    }

    bool result = defaultValue;
    if (lua_isboolean(L, -1))
        result = lua_toboolean(L, -1) != 0;
    else if (lua_isnumber(L, -1))
        result = lua_tonumber(L, -1) != 0.0;

    lua_settop(L, stackTop);
    return result;
}

static bool CallLuaSayResult(const char* pszFunction, const GModLuaArg_t* pArgs, int nArgs, const char* pszDefaultText, char* pszOutText, int iOutTextSize)
{
    if (!pszOutText || iOutTextSize <= 0)
        return true;

    Q_strncpy(pszOutText, pszDefaultText ? pszDefaultText : "", iOutTextSize);

    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L)
        return true;

    int stackTop = lua_gettop(L);
    if (!PushLuaFunction(L, pszFunction, stackTop))
        return true;

    for (int i = 0; i < nArgs; ++i)
        PushLuaArg(L, pArgs[i]);

    if (lua_pcall(L, nArgs, 1, 0) != 0)
    {
        Warning("Lua error in %s: %s\n", pszFunction, lua_tostring(L, -1));
        lua_settop(L, stackTop);
        return true;
    }

    bool allow = true;
    if (lua_isboolean(L, -1))
    {
        allow = lua_toboolean(L, -1) != 0;
    }
    else if (lua_isstring(L, -1))
    {
        Q_strncpy(pszOutText, lua_tostring(L, -1), iOutTextSize);
        allow = pszOutText[0] != '\0';
    }

    lua_settop(L, stackTop);
    return allow;
}

void CGModGamemodeSystem::OnPlayerConnect(const char* pszName, const char* pszAddress, const char* pszSteamID)
{
    GModLuaArg_t args[3];
    args[0] = LuaArgString(pszName);
    args[1] = LuaArgString(pszAddress);
    args[2] = LuaArgString(pszSteamID);
    CallLuaNoResult("eventPlayerConnect", args, 3);
}

void CGModGamemodeSystem::OnPlayerDisconnect(const char* pszName, int iUserID, const char* pszAddress, const char* pszSteamID, const char* pszReason)
{
    GModLuaArg_t args[5];
    args[0] = LuaArgString(pszName);
    args[1] = LuaArgNumber(iUserID);
    args[2] = LuaArgString(pszAddress);
    args[3] = LuaArgString(pszSteamID);
    args[4] = LuaArgString(pszReason);
    CallLuaNoResult("eventPlayerDisconnect", args, 5);
}

void CGModGamemodeSystem::OnPlayerInitialSpawn(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    GModLuaArg_t args[1];
    args[0] = LuaArgNumber(LuaPlayerID(pPlayer));
    CallLuaNoResult("eventPlayerInitialSpawn", args, 1);
}

void CGModGamemodeSystem::OnPlayerActive(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    GModLuaArg_t args[3];
    args[0] = LuaArgString(STRING(pPlayer->pl.netname));
    args[1] = LuaArgNumber(LuaPlayerID(pPlayer));
    args[2] = LuaArgString("");
    CallLuaNoResult("eventPlayerActive", args, 3);
}

void CGModGamemodeSystem::OnPlayerChangeTeam(const char* pszName, int iUserID, int iNewTeam, int iOldTeam)
{
    GModLuaArg_t args[4];
    args[0] = LuaArgString(pszName);
    args[1] = LuaArgNumber(iUserID);
    args[2] = LuaArgNumber(iNewTeam);
    args[3] = LuaArgNumber(iOldTeam);
    CallLuaNoResult("eventPlayerChangeTeam", args, 4);
}

void CGModGamemodeSystem::OnPlayerNameChange(int iUserID, const char* pszNewName, const char* pszOldName)
{
    GModLuaArg_t args[3];
    args[0] = LuaArgNumber(iUserID);
    args[1] = LuaArgString(pszNewName);
    args[2] = LuaArgString(pszOldName);
    CallLuaNoResult("eventPlayerNameChange", args, 3);
}

void CGModGamemodeSystem::OnPlayerHurt(int iUserID, int iNewHealth, int iAttacker)
{
    GModLuaArg_t args[3];
    args[0] = LuaArgNumber(iUserID);
    args[1] = LuaArgNumber(iNewHealth);
    args[2] = LuaArgNumber(iAttacker);
    CallLuaNoResult("eventPlayerHurt", args, 3);
}

void CGModGamemodeSystem::OnPlayerKilled(int iUserID, int iAttacker, const char* pszWeapon)
{
    GModLuaArg_t args[3];
    args[0] = LuaArgNumber(iUserID);
    args[1] = LuaArgNumber(iAttacker);
    args[2] = LuaArgString(pszWeapon);
    CallLuaNoResult("eventPlayerKilled", args, 3);
}

bool CGModGamemodeSystem::OnPlayerSay(CBasePlayer* pPlayer, const char* pszText, bool bTeamOnly, char* pszOutText, int iOutTextSize)
{
    if (!pPlayer)
        return true;

    GModLuaArg_t args[3];
    args[0] = LuaArgNumber(LuaPlayerID(pPlayer));
    args[1] = LuaArgString(pszText);
    args[2] = LuaArgBool(bTeamOnly);
    return CallLuaSayResult("eventPlayerSay", args, 3, pszText, pszOutText, iOutTextSize);
}

bool CGModGamemodeSystem::OnPlayerUseEntity(CBasePlayer* pPlayer, CBaseEntity* pEntity)
{
    if (!pPlayer || !pEntity)
        return false;

    GModLuaArg_t args[2];
    args[0] = LuaArgNumber(LuaPlayerID(pPlayer));
    args[1] = LuaArgNumber(LuaEntityID(pEntity));
    return CallLuaBoolResult("eventPlayerUseEntity", args, 2, false);
}

void CGModGamemodeSystem::OnNPCKilled(int iKillerID, CBaseEntity* pKilled, CBaseEntity* pInflictor)
{
    GModLuaArg_t args[3];
    args[0] = LuaArgNumber(iKillerID);
    args[1] = LuaArgNumber(LuaEntityID(pKilled));
    args[2] = LuaArgNumber(LuaEntityID(pInflictor));
    CallLuaNoResult("eventNPCKilled", args, 3);
}

bool CGModGamemodeSystem::CanPlayerSpawnProp(CBasePlayer* pPlayer, const char* pszModelName)
{
    GModLuaArg_t args[2];
    args[0] = LuaArgNumber(LuaPlayerID(pPlayer));
    args[1] = LuaArgString(pszModelName);
    return CallLuaBoolResult("eventPlayerSpawnProp", args, 2, true);
}

void CGModGamemodeSystem::OnPlayerPropSpawned(CBasePlayer* pPlayer, CBaseEntity* pProp)
{
    GModLuaArg_t args[2];
    args[0] = LuaArgNumber(LuaPlayerID(pPlayer));
    args[1] = LuaArgNumber(LuaEntityID(pProp));
    CallLuaNoResult("eventPlayerPropSpawned", args, 2);
}

bool CGModGamemodeSystem::CanPlayerSpawnRagdoll(CBasePlayer* pPlayer, const char* pszModelName)
{
    GModLuaArg_t args[2];
    args[0] = LuaArgNumber(LuaPlayerID(pPlayer));
    args[1] = LuaArgString(pszModelName);
    return CallLuaBoolResult("eventPlayerSpawnRagdoll", args, 2, true);
}

void CGModGamemodeSystem::OnPlayerRagdollSpawned(CBasePlayer* pPlayer, CBaseEntity* pRagdoll)
{
    GModLuaArg_t args[2];
    args[0] = LuaArgNumber(LuaPlayerID(pPlayer));
    args[1] = LuaArgNumber(LuaEntityID(pRagdoll));
    CallLuaNoResult("eventPlayerRagdollSpawned", args, 2);
}

bool CGModGamemodeSystem::CanPlayerDuplicateProp(CBasePlayer* pPlayer, CBaseEntity* pProp)
{
    GModLuaArg_t args[2];
    args[0] = LuaArgNumber(LuaPlayerID(pPlayer));
    args[1] = LuaArgNumber(LuaEntityID(pProp));
    return CallLuaBoolResult("eventPlayerDuplicateProp", args, 2, true);
}

bool CGModGamemodeSystem::CanPlayerDuplicateRagdoll(CBasePlayer* pPlayer, CBaseEntity* pRagdoll)
{
    GModLuaArg_t args[2];
    args[0] = LuaArgNumber(LuaPlayerID(pPlayer));
    args[1] = LuaArgNumber(LuaEntityID(pRagdoll));
    return CallLuaBoolResult("eventPlayerDuplicateRagdoll", args, 2, true);
}

void CGModGamemodeSystem::OnPlayerConnect(CBasePlayer* pPlayer)
{
    if (!s_pActiveGamemode || !pPlayer)
        return;

    char args[64];
    Q_snprintf(args, sizeof(args), "%d", pPlayer->entindex());
    CallGamemodeFunctionWithArgs("gamerulesPlayerConnect", args);
}

void CGModGamemodeSystem::OnPlayerDisconnect(CBasePlayer* pPlayer)
{
    if (!s_pActiveGamemode || !pPlayer)
        return;

    char args[64];
    Q_snprintf(args, sizeof(args), "%d", pPlayer->entindex());
    CallGamemodeFunctionWithArgs("gamerulesPlayerDisconnect", args);
}

void CGModGamemodeSystem::OnPlayerSpawn(CBasePlayer* pPlayer)
{
    if (!s_pActiveGamemode || !pPlayer)
        return;

    bool bCalled = CallLuaPlayerFunction("eventPlayerSpawn", pPlayer);
    DevMsg("GMod Gamemode System: eventPlayerSpawn for player %d %s\n",
        pPlayer->entindex(), bCalled ? "called" : "not found/failed");
}

void CGModGamemodeSystem::OnPlayerInput(CBasePlayer* pPlayer, int buttonsPressed, int buttonsReleased)
{
    if (!s_pActiveGamemode || !pPlayer)
        return;

    for (int i = 0; i < 22; ++i)
    {
        int button = 1 << i;
		if (buttonsPressed & button)
		{
			CallLuaPlayerKeyFunction("eventKeyPressed", pPlayer, button);
			CallLuaKeyEventHook("eventKeyPressed", pPlayer, button);
		}

		if (buttonsReleased & button)
		{
			CallLuaPlayerKeyFunction("eventKeyReleased", pPlayer, button);
			CallLuaKeyEventHook("eventKeyReleased", pPlayer, button);
		}
    }
}

void CGModGamemodeSystem::OnPlayerDeath(CBasePlayer* pPlayer, CBaseEntity* pKiller)
{
    if (!s_pActiveGamemode || !pPlayer)
        return;

    char args[128];
    int killerID = pKiller ? pKiller->entindex() : 0;
    Q_snprintf(args, sizeof(args), "%d, %d", pPlayer->entindex(), killerID);
    CallGamemodeFunctionWithArgs("gamerulesPlayerDeath", args);
}

void CGModGamemodeSystem::OnPlayerChangeTeam(CBasePlayer* pPlayer, int newTeam)
{
    if (!s_pActiveGamemode || !pPlayer)
        return;

    char args[128];
    Q_snprintf(args, sizeof(args), "%d, %d", pPlayer->entindex(), newTeam);
    CallGamemodeFunctionWithArgs("gamerulesPlayerChangeTeam", args);
}

void CGModGamemodeSystem::OnPropBreak(CBaseEntity* pBreaker, CBaseEntity* pProp)
{
    if (!s_pActiveGamemode || !pProp)
        return;

    CBasePlayer* pBreakerPlayer = ToBasePlayer(pBreaker);

    GModLuaArg_t args[2];
    args[0] = LuaArgNumber(pBreakerPlayer ? LuaPlayerID(pBreakerPlayer) : LuaEntityID(pBreaker));
    args[1] = LuaArgNumber(LuaEntityID(pProp));
    CallLuaNoResult("eventPropBreak", args, 2);
}

void CGModGamemodeSystem::OnPlayerChat(CBasePlayer* pPlayer, const char* pszMessage)
{
    if (!s_pActiveGamemode || !pPlayer || !pszMessage)
        return;

    char args[512];
    Q_snprintf(args, sizeof(args), "%d, \"%s\"", pPlayer->entindex(), pszMessage);
    CallGamemodeFunctionWithArgs("gamerulesPlayerChat", args);
}

// Gamemode Lua function calls
void CGModGamemodeSystem::CallGamemodeFunction(const char* pszFunction)
{
    if (!s_pActiveGamemode || !pszFunction)
        return;

    CGModLuaSystem::CallGamemodeFunction(pszFunction);
}

void CGModGamemodeSystem::CallGamemodeFunctionWithPlayer(const char* pszFunction, CBasePlayer* pPlayer)
{
    if (!s_pActiveGamemode || !pszFunction || !pPlayer)
        return;

    char args[64];
    Q_snprintf(args, sizeof(args), "%d", pPlayer->entindex());
    CallGamemodeFunctionWithArgs(pszFunction, args);
}

void CGModGamemodeSystem::CallGamemodeFunctionWithArgs(const char* pszFunction, const char* pszArgs)
{
    if (!s_pActiveGamemode || !pszFunction)
        return;

    char luaCode[512];
    if (pszArgs && pszArgs[0])
    {
        Q_snprintf(luaCode, sizeof(luaCode), "if type(%s)==\"function\" then %s(%s) end", pszFunction, pszFunction, pszArgs);
    }
    else
    {
        Q_snprintf(luaCode, sizeof(luaCode), "if type(%s)==\"function\" then %s() end", pszFunction, pszFunction);
    }

    CGModLuaSystem::ExecuteString(luaCode);
}

bool CGModGamemodeSystem::PickDefaultSpawnTeam(CBasePlayer* pPlayer)
{
    if (!s_pActiveGamemode || !pPlayer)
        return false;

    bool bHandled = false;
    if (CallLuaPlayerBoolFunction("PickDefaultSpawnTeam", pPlayer, bHandled))
    {
        CallLuaPlayerEventHook("PickDefaultSpawnTeam", pPlayer);
        return bHandled;
    }

    return false;
}

void CGModGamemodeSystem::PlayerSpawnChooseModel(CBasePlayer* pPlayer)
{
    if (!s_pActiveGamemode || !pPlayer)
        return;

    CallLuaPlayerFunction("PlayerSpawnChooseModel", pPlayer);
}

void CGModGamemodeSystem::GiveDefaultItems(CBasePlayer* pPlayer)
{
    if (!s_pActiveGamemode || !pPlayer)
        return;

    if (CallLuaPlayerFunction("GiveDefaultItems", pPlayer))
    {
        CallLuaPlayerEventHook("GiveDefaultItems", pPlayer);
    }
}

// Game rules functions
void CGModGamemodeSystem::SetTargetIDRules(TargetIDRules_t rules)
{
    s_TargetIDRules = rules;
}

TargetIDRules_t CGModGamemodeSystem::GetTargetIDRules()
{
    return s_TargetIDRules;
}

void CGModGamemodeSystem::SetTeamScore(int team, int score)
{
    // Ensure team scores array is large enough
    while (s_TeamScores.Count() <= team)
    {
        s_TeamScores.AddToTail(0);
    }

    s_TeamScores[team] = score;

    // Update team manager if available (older engine version may not expose setters)
    CTeam* pTeam = GetGlobalTeam(team);
    if (pTeam)
    {
        pTeam->m_iScore = score;
    }
}

int CGModGamemodeSystem::GetTeamScore(int team)
{
    if (team < 0 || team >= s_TeamScores.Count())
        return 0;

    return s_TeamScores[team];
}

void CGModGamemodeSystem::SetTeamName(int team, const char* pszName)
{
    if (!pszName)
        return;

    if (team >= 0 && team < 32)
    {
        Q_strncpy(s_TeamNames[team], pszName, sizeof(s_TeamNames[team]));
    }

    // Update team manager if available
    CTeam* pTeam = GetGlobalTeam(team);
    (void)pTeam;
}

const char* CGModGamemodeSystem::GetTeamName(int team)
{
    if (team < 0 || team >= 32)
        return "";

    return s_TeamNames[team];
}

// Player management
void CGModGamemodeSystem::ChangePlayerTeam(CBasePlayer* pPlayer, int newTeam)
{
    if (!pPlayer)
        return;

    EnsureGModTeamManagers();

    if (newTeam > 0 && !GetGlobalTeam(newTeam))
    {
        Warning("GMod Gamemode System: refusing invalid ChangeTeam(%d) for player %d\n", newTeam, pPlayer->entindex());
        return;
    }

    pPlayer->ChangeTeam(newTeam);
    OnPlayerChangeTeam(pPlayer, newTeam);
}

void CGModGamemodeSystem::SpawnPlayer(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    pPlayer->Spawn();
    OnPlayerSpawn(pPlayer);
}

void CGModGamemodeSystem::KillPlayer(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    pPlayer->CommitSuicide();
}

void CGModGamemodeSystem::FreezePlayer(CBasePlayer* pPlayer, bool freeze)
{
    if (!pPlayer)
        return;

    if (freeze)
        pPlayer->AddFlag(FL_FROZEN);
    else
        pPlayer->RemoveFlag(FL_FROZEN);
}

void CGModGamemodeSystem::LoadAllGamemodes()
{
    // Register built-in gamemodes discovered from lua/gamemodes/ analysis.
    // Legacy gamemode scripts define globals, so only the active gamemode is executed.
    RegisterGamemode("build", "lua/gamemodes/default/init.lua");
    RegisterGamemode("default", "lua/gamemodes/default/init.lua");
    RegisterGamemode("sandbox", "lua/gamemodes/default/init.lua");
    RegisterGamemode("football", "lua/gamemodes/gm_football.lua");
    RegisterGamemode("hideandseek", "lua/gamemodes/gm_hideandseek.lua");
    RegisterGamemode("laserdance", "lua/gamemodes/gm_laserdance.lua");
    RegisterGamemode("longsight", "lua/gamemodes/gm_longsight.lua");
    RegisterGamemode("910", "lua/gamemodes/gm_910.lua");

    // Additional gamemodes with multi-file structure
    RegisterGamemode("melonracer", "lua/gamemodes/melonracer/init.lua");
    RegisterGamemode("tpc", "lua/gamemodes/tpc/init.lua");

    DevMsg("Registered %d gamemodes\n", s_GamemodeRegistry.Count());
}

void CGModGamemodeSystem::ReloadAllGamemodes()
{
    // Unload all gamemodes
    for (int i = 0; i < s_GamemodeRegistry.Count(); i++)
    {
        s_GamemodeRegistry[i].isLoaded = false;
        s_GamemodeRegistry[i].isActive = false;
    }

    s_pActiveGamemode = NULL;

    // Reload all gamemodes
    LoadAllGamemodes();

    // Restore active gamemode
    const char* currentGamemode = gmod_gamemode_current.GetString();
    if (currentGamemode && currentGamemode[0])
    {
        SetActiveGamemode(currentGamemode);
    }

    DevMsg("Reloaded all gamemodes\n");
}

void CGModGamemodeSystem::LoadGamemodeFiles(GamemodeData_t* pGamemode)
{
    if (!pGamemode)
        return;

    // Load additional script files for modular gamemodes
    for (int i = 0; i < pGamemode->scriptFileCount; i++)
    {
        CGModLuaSystem::LoadScript(pGamemode->scriptFiles[i], LUA_SCRIPT_GAMEMODE);
    }
}

const char* CGModGamemodeSystem::GetGamemodeTypeName(GamemodeType_t gamemodeType)
{
    switch (gamemodeType)
    {
        case GAMEMODE_BUILD: return "build";
        case GAMEMODE_FOOTBALL: return "football";
        case GAMEMODE_HIDEANDSEEK: return "hideandseek";
        case GAMEMODE_LASERDANCE: return "laserdance";
        case GAMEMODE_LONGSIGHT: return "longsight";
        case GAMEMODE_910: return "910";
        case GAMEMODE_BIRDMAN: return "birdman";
        case GAMEMODE_BIRDPOO: return "birdpoo";
        case GAMEMODE_MELONRACER: return "melonracer";
        case GAMEMODE_TPC: return "tpc";
        case GAMEMODE_ZOMBIE: return "zombie";
        default: return "none";
    }
}

GamemodeType_t CGModGamemodeSystem::GetGamemodeTypeFromName(const char* pszName)
{
    if (!pszName)
        return GAMEMODE_NONE;

    if (Q_stricmp(pszName, "build") == 0) return GAMEMODE_BUILD;
    if (Q_stricmp(pszName, "football") == 0) return GAMEMODE_FOOTBALL;
    if (Q_stricmp(pszName, "hideandseek") == 0) return GAMEMODE_HIDEANDSEEK;
    if (Q_stricmp(pszName, "laserdance") == 0) return GAMEMODE_LASERDANCE;
    if (Q_stricmp(pszName, "longsight") == 0) return GAMEMODE_LONGSIGHT;
    if (Q_stricmp(pszName, "910") == 0) return GAMEMODE_910;
    if (Q_stricmp(pszName, "birdman") == 0) return GAMEMODE_BIRDMAN;
    if (Q_stricmp(pszName, "birdpoo") == 0) return GAMEMODE_BIRDPOO;
    if (Q_stricmp(pszName, "melonracer") == 0) return GAMEMODE_MELONRACER;
    if (Q_stricmp(pszName, "tpc") == 0) return GAMEMODE_TPC;
    if (Q_stricmp(pszName, "zombie") == 0) return GAMEMODE_ZOMBIE;

    return GAMEMODE_NONE;
}

int CGModGamemodeSystem::GetMaxPlayers()
{
    if (s_pActiveGamemode)
        return s_pActiveGamemode->maxPlayers;

    return 32; // Default
}

// Helper function implementations
GamemodeData_t* CGModGamemodeSystem::FindGamemode(const char* pszGamemodeName)
{
    if (!pszGamemodeName)
        return NULL;

    for (int i = 0; i < s_GamemodeRegistry.Count(); i++)
    {
        if (Q_stricmp(s_GamemodeRegistry[i].gamemodeName, pszGamemodeName) == 0)
        {
            return &s_GamemodeRegistry[i];
        }
    }

    return NULL;
}

bool CGModGamemodeSystem::LoadGamemodeScript(GamemodeData_t* pGamemode)
{
    if (!pGamemode)
        return false;

    LuaFunctionResult_t result = CGModLuaSystem::LoadScript(pGamemode->scriptPath, LUA_SCRIPT_GAMEMODE);
    if (result == LUA_RESULT_SUCCESS)
    {
        LoadGamemodeFiles(pGamemode);
    }

    return result == LUA_RESULT_SUCCESS;
}

void CGModGamemodeSystem::InitializeGamemodeDefaults()
{
    // Initialize default gamemode settings
    s_TargetIDRules = TARGETID_ALWAYS;
    s_GamemodeState.currentRound = 0;
    s_GamemodeState.inProgress = false;

    DevMsg("Gamemode defaults initialized\n");
}

bool CGModGamemodeSystem::ValidateGamemodeScript(const char* pszScriptPath)
{
    if (!pszScriptPath)
        return false;

    return filesystem->FileExists(pszScriptPath, "GAME");
}

void CGModGamemodeSystem::SetupDefaultTeams()
{
    // Setup default teams (discovered from gamemode analysis)
    SetTeamName(GModTeamToSourceTeam(0), "Unassigned");
    SetTeamName(GModTeamToSourceTeam(1), "Spectators");

    // Beta GMod team layout from includes/defines.lua.
    SetTeamName(GModTeamToSourceTeam(2), "Blue Team");
    SetTeamName(GModTeamToSourceTeam(3), "Yellow Team");
    SetTeamName(GModTeamToSourceTeam(4), "Green Team");
    SetTeamName(GModTeamToSourceTeam(5), "Red Team");

    SetTeamScore(GModTeamToSourceTeam(0), 0);
    SetTeamScore(GModTeamToSourceTeam(1), 0);
    SetTeamScore(GModTeamToSourceTeam(2), 0);
    SetTeamScore(GModTeamToSourceTeam(3), 0);
    SetTeamScore(GModTeamToSourceTeam(4), 0);
    SetTeamScore(GModTeamToSourceTeam(5), 0);
}

//-----------------------------------------------------------------------------
// Console command implementations
//-----------------------------------------------------------------------------
void CMD_gmod_gamemode(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() < 2)
    {
        GamemodeData_t* pActiveGamemode = CGModGamemodeSystem::GetActiveGamemode();
        if (pActiveGamemode)
        {
            ClientPrint(pPlayer, HUD_PRINTTALK, "Current gamemode: %s", pActiveGamemode->gamemodeName);
        }
        else
        {
            ClientPrint(pPlayer, HUD_PRINTTALK, "No active gamemode");
        }
        return;
    }

    const char* gamemodeName = engine->Cmd_Argv(1);
    if (CGModGamemodeSystem::SetActiveGamemode(gamemodeName))
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Set gamemode to: %s", gamemodeName);
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to set gamemode: %s", gamemodeName);
    }
}

void CMD_gmod_reload_gamemodes(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModGamemodeSystem::ReloadAllGamemodes();
    ClientPrint(pPlayer, HUD_PRINTTALK, "All gamemodes reloaded");
}

void CMD_gmod_list_gamemodes(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Gamemode listing not implemented in this build");
}

void CMD_gmod_gamemode_restart(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    GamemodeData_t* pActiveGamemode = CGModGamemodeSystem::GetActiveGamemode();
    if (pActiveGamemode)
    {
        CGModGamemodeSystem::OnRoundEnd();
        CGModGamemodeSystem::OnRoundStart();
        ClientPrint(pPlayer, HUD_PRINTTALK, "Gamemode restarted");
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "No active gamemode to restart");
    }
}

// Individual gamemode commands
void CMD_gmod_gamemode_build(void) { CGModGamemodeSystem::SetActiveGamemode("build"); }
void CMD_gmod_gamemode_football(void) { CGModGamemodeSystem::SetActiveGamemode("football"); }
void CMD_gmod_gamemode_hideandseek(void) { CGModGamemodeSystem::SetActiveGamemode("hideandseek"); }
void CMD_gmod_gamemode_laserdance(void) { CGModGamemodeSystem::SetActiveGamemode("laserdance"); }
void CMD_gmod_gamemode_longsight(void) { CGModGamemodeSystem::SetActiveGamemode("longsight"); }

//-----------------------------------------------------------------------------
// Additional Lua engine binding functions needed by gamemodes
//-----------------------------------------------------------------------------
extern "C" {

int lua_GameSetTargetIDRules(lua_State* L)
{
    if (lua_gettop(L) < 1)
        return 0;

    int rules = (int)lua_tonumber(L, 1);
    if (rules >= 0 && rules < 4)
    {
        CGModGamemodeSystem::SetTargetIDRules((TargetIDRules_t)rules);
    }

    return 0;
}

int lua_TeamSetScore(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int team = GModTeamToSourceTeam( (int)lua_tonumber(L, 1) );
    int score = (int)lua_tonumber(L, 2);

    CGModGamemodeSystem::SetTeamScore(team, score);
    return 0;
}

int lua_TeamGetScore(lua_State* L)
{
    if (lua_gettop(L) < 1)
    {
        lua_pushnumber(L, 0);
        return 1;
    }

    int team = GModTeamToSourceTeam( (int)lua_tonumber(L, 1) );
    int score = CGModGamemodeSystem::GetTeamScore(team);

    lua_pushnumber(L, score);
    return 1;
}

int lua_TeamSetName(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int team = GModTeamToSourceTeam( (int)lua_tonumber(L, 1) );
    const char* name = lua_tostring(L, 2);

    if (name)
    {
        CGModGamemodeSystem::SetTeamName(team, name);
    }

    return 0;
}

int lua_TeamGetName(lua_State* L)
{
    if (lua_gettop(L) < 1)
    {
        lua_pushstring(L, "");
        return 1;
    }

    int team = GModTeamToSourceTeam( (int)lua_tonumber(L, 1) );
    const char* name = CGModGamemodeSystem::GetTeamName(team);

    lua_pushstring(L, name ? name : "");
    return 1;
}

int lua_PlayerChangeTeam(lua_State* L)
{
    if (lua_gettop(L) < 2)
        return 0;

    int playerid = (int)lua_tonumber(L, 1);
    int team = GModTeamToSourceTeam( (int)lua_tonumber(L, 2) );

    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);
    if (pPlayer)
    {
        CGModGamemodeSystem::ChangePlayerTeam(pPlayer, team);
    }

    return 0;
}

int lua_MaxPlayers(lua_State* L)
{
    int maxPlayers = CGModGamemodeSystem::GetMaxPlayers();
    lua_pushnumber(L, maxPlayers);
    return 1;
}

int lua_EntPrecacheModel(lua_State* L)
{
    (void)L;
    return 0;
}

int lua_GameGetMapName(lua_State* L)
{
    lua_pushstring(L, STRING(gpGlobals->mapname));
    return 1;
}

int lua_GameRestartRound(lua_State* L)
{
    CGModGamemodeSystem::OnRoundEnd();
    CGModGamemodeSystem::OnRoundStart();
    return 0;
}

} // extern "C"

//-----------------------------------------------------------------------------
// Console command registration
//-----------------------------------------------------------------------------
static ConCommand gmod_gamemode_cmd("gmod_gamemode", CMD_gmod_gamemode, "Set or get current gamemode");
static ConCommand gmod_reload_gamemodes_cmd("gmod_reload_gamemodes", CMD_gmod_reload_gamemodes, "Reload all gamemodes");
static ConCommand gmod_list_gamemodes_cmd("gmod_list_gamemodes", CMD_gmod_list_gamemodes, "List all available gamemodes");
static ConCommand gmod_gamemode_restart_cmd("gmod_gamemode_restart", CMD_gmod_gamemode_restart, "Restart current gamemode");

// Individual gamemode command registration
static ConCommand gmod_gamemode_build_cmd("gmod_gamemode_build", CMD_gmod_gamemode_build, "Set gamemode to build");
static ConCommand gmod_gamemode_football_cmd("gmod_gamemode_football", CMD_gmod_gamemode_football, "Set gamemode to football");
static ConCommand gmod_gamemode_hideandseek_cmd("gmod_gamemode_hideandseek", CMD_gmod_gamemode_hideandseek, "Set gamemode to hide and seek");
static ConCommand gmod_gamemode_laserdance_cmd("gmod_gamemode_laserdance", CMD_gmod_gamemode_laserdance, "Set gamemode to laser dance");
static ConCommand gmod_gamemode_longsight_cmd("gmod_gamemode_longsight", CMD_gmod_gamemode_longsight, "Set gamemode to longsight");
