#ifndef GMOD_GAMEMODE_H
#define GMOD_GAMEMODE_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "utlvector.h"
#include "igamesystem.h"
#include "gmod_lua.h"

// Forward declarations
class CBaseEntity;
class CBasePlayer;

// Gamemode types discovered from lua/gamemodes/ directory analysis
enum GamemodeType_t
{
    GAMEMODE_NONE = 0,
    GAMEMODE_BUILD = 1,
    GAMEMODE_FOOTBALL = 2,
    GAMEMODE_HIDEANDSEEK = 3,
    GAMEMODE_LASERDANCE = 4,
    GAMEMODE_LONGSIGHT = 5,
    GAMEMODE_910 = 6,
    GAMEMODE_BIRDMAN = 7,
    GAMEMODE_BIRDPOO = 8,
    GAMEMODE_MELONRACER = 9,
    GAMEMODE_TPC = 10,
    GAMEMODE_ZOMBIE = 11,
    GAMEMODE_MAX
};

// Gamemode state
enum GamemodeState_t
{
    GAMEMODE_STATE_INACTIVE = 0,
    GAMEMODE_STATE_LOADING = 1,
    GAMEMODE_STATE_ACTIVE = 2,
    GAMEMODE_STATE_ENDING = 3
};

// Target ID rules (discovered from _GameSetTargetIDRules analysis)
enum TargetIDRules_t
{
    TARGETID_NEVER = 0,
    TARGETID_ALWAYS = 1,
    TARGETID_SAME_TEAM = 2,
    TARGETID_DIFFERENT_TEAM = 3
};

// Gamemode data structure
struct GamemodeData_t
{
    GamemodeType_t gamemodeType;
    char gamemodeName[256];
    char displayName[256];
    char description[512];
    char scriptPath[256];
    char author[256];
    char version[64];

    // Script files for modular gamemodes - simplified for LeakNet compatibility
    char scriptFiles[10][256]; // Support up to 10 script files
    int scriptFileCount;

    // Configuration
    int maxPlayers;
    int minPlayers;
    bool teamBased;
    bool allowSpectators;
    float roundTime;
    int maxRounds;
    TargetIDRules_t targetIDRules;

    // State
    bool isLoaded;
    bool isActive;
    GamemodeState_t state;

    GamemodeData_t()
    {
        gamemodeType = GAMEMODE_NONE;
        strcpy(gamemodeName, "");
        strcpy(displayName, "Unknown Gamemode");
        strcpy(description, "");
        strcpy(scriptPath, "");
        strcpy(author, "");
        strcpy(version, "1.0");

        // Initialize script files array
        for (int i = 0; i < 10; i++)
            scriptFiles[i][0] = '\0';
        scriptFileCount = 0;

        maxPlayers = 32;
        minPlayers = 1;
        teamBased = false;
        allowSpectators = true;
        roundTime = 300.0f; // 5 minutes
        maxRounds = 10;
        targetIDRules = TARGETID_ALWAYS;

        isLoaded = false;
        isActive = false;
        state = GAMEMODE_STATE_INACTIVE;
    }
};

//-----------------------------------------------------------------------------
// GMod Gamemode System - Implements complete gamemode functionality from GMod 9.0.4b
// Discovered through lua/gamemodes/ directory examination and IDA analysis
//-----------------------------------------------------------------------------
class CGModGamemodeSystem : public CAutoGameSystem
{
public:
    CGModGamemodeSystem() : CAutoGameSystem() {}

    // CAutoGameSystem overrides
    virtual bool Init();
    virtual void Shutdown();
    virtual void LevelInitPreEntity();
    virtual void LevelInitPostEntity();
    virtual void LevelShutdownPostEntity();
    virtual void FrameUpdatePreEntityThink();

    // Gamemode management functions
    static bool RegisterGamemode(const char* pszGamemodeName, const char* pszScriptPath);
    static bool LoadGamemode(const char* pszGamemodeName);
    static bool UnloadGamemode(const char* pszGamemodeName);
    static bool SetActiveGamemode(const char* pszGamemodeName);
    static GamemodeData_t* GetGamemodeData(const char* pszGamemodeName);
    static GamemodeData_t* GetActiveGamemode();
    static bool IsGamemodeActive(const char* pszGamemodeName);

    // Gamemode events - called by game engine
    static void OnMapStart();
    static void OnMapEnd();
    static void OnRoundStart();
    static void OnRoundEnd();
    static void OnPlayerConnect(CBasePlayer* pPlayer);
    static void OnPlayerDisconnect(CBasePlayer* pPlayer);
    static void OnPlayerSpawn(CBasePlayer* pPlayer);
    static void OnPlayerDeath(CBasePlayer* pPlayer, CBaseEntity* pKiller);
    static void OnPlayerChangeTeam(CBasePlayer* pPlayer, int newTeam);
    static void OnPlayerChat(CBasePlayer* pPlayer, const char* pszMessage);

    // Gamemode Lua function calls
    static void CallGamemodeFunction(const char* pszFunction);
    static void CallGamemodeFunctionWithPlayer(const char* pszFunction, CBasePlayer* pPlayer);
    static void CallGamemodeFunctionWithArgs(const char* pszFunction, const char* pszArgs);

    // Game rules functions
    static void SetTargetIDRules(TargetIDRules_t rules);
    static TargetIDRules_t GetTargetIDRules();
    static void SetTeamScore(int team, int score);
    static int GetTeamScore(int team);
    static void SetTeamName(int team, const char* pszName);
    static const char* GetTeamName(int team);

    // Player management
    static void ChangePlayerTeam(CBasePlayer* pPlayer, int newTeam);
    static void SpawnPlayer(CBasePlayer* pPlayer);
    static void KillPlayer(CBasePlayer* pPlayer);
    static void FreezePlayer(CBasePlayer* pPlayer, bool freeze);

    // Gamemode loading and registration
    static void LoadAllGamemodes();
    static void ReloadAllGamemodes();
    static void LoadGamemodeFiles(GamemodeData_t* pGamemode);

    // Utility functions
    static const char* GetGamemodeTypeName(GamemodeType_t gamemodeType);
    static GamemodeType_t GetGamemodeTypeFromName(const char* pszName);
    static int GetMaxPlayers();

private:
    struct GamemodeState_t
    {
        int currentRound;
        float roundStartTime;
        float roundEndTime;
        bool inProgress;
        CUtlVector<int> teamScores;

        GamemodeState_t()
        {
            currentRound = 0;
            roundStartTime = 0.0f;
            roundEndTime = 0.0f;
            inProgress = false;
        }
    };

    static CUtlVector<GamemodeData_t> s_GamemodeRegistry;
    static GamemodeData_t* s_pActiveGamemode;
    static GamemodeState_t s_GamemodeState;
    static TargetIDRules_t s_TargetIDRules;
    static char s_TeamNames[32][64]; // Support up to 32 teams with 64-char names
    static CUtlVector<int> s_TeamScores;
    static bool s_bSystemInitialized;

    // Helper functions
    static GamemodeData_t* FindGamemode(const char* pszGamemodeName);
    static bool LoadGamemodeScript(GamemodeData_t* pGamemode);
    static void InitializeGamemodeDefaults();
    static bool ValidateGamemodeScript(const char* pszScriptPath);
    static void SetupDefaultTeams();
};

// Global instance
extern CGModGamemodeSystem g_GMod_GamemodeSystem;

// Console command handlers - discovered from IDA analysis
void CMD_gmod_gamemode(void);
void CMD_gmod_reload_gamemodes(void);
void CMD_gmod_list_gamemodes(void);
void CMD_gmod_gamemode_restart(void);

// Individual gamemode commands
void CMD_gmod_gamemode_build(void);
void CMD_gmod_gamemode_football(void);
void CMD_gmod_gamemode_hideandseek(void);
void CMD_gmod_gamemode_laserdance(void);
void CMD_gmod_gamemode_longsight(void);

// Additional Lua engine binding functions needed by gamemodes
extern "C" {
    int lua_GameSetTargetIDRules(lua_State* L);
    int lua_TeamSetScore(lua_State* L);
    int lua_TeamGetScore(lua_State* L);
    int lua_TeamSetName(lua_State* L);
    int lua_TeamGetName(lua_State* L);
    int lua_PlayerChangeTeam(lua_State* L);
    int lua_MaxPlayers(lua_State* L);
    int lua_EntPrecacheModel(lua_State* L);
    int lua_GameGetMapName(lua_State* L);
    int lua_GameRestartRound(lua_State* L);
}

#endif // GMOD_GAMEMODE_H