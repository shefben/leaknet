#ifndef GMOD_RUNFUNCTION_H
#define GMOD_RUNFUNCTION_H

#pragma once

#include "cbase.h"
#include "baseentity.h"

// Forward declarations
class CBasePlayer;

//-----------------------------------------------------------------------------
// CGmodRunFunction - Entity that executes Lua functions when triggered
// Based on IDA analysis: CGmodRunFunction, RunFunction, InputRunFunction
//-----------------------------------------------------------------------------
class CGmodRunFunction : public CBaseEntity
{
    DECLARE_CLASS(CGmodRunFunction, CBaseEntity);
    DECLARE_DATADESC();

public:
    CGmodRunFunction();

    virtual void Spawn();
    virtual void Precache();
    virtual int ObjectCaps() { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

    // Input handlers discovered from IDA
    void InputRunFunction(inputdata_t &inputData);
    void InputSetFunction(inputdata_t &inputData);
    void InputSetDelay(inputdata_t &inputData);

    // Direct function calls
    void RunFunction();
    void SetFunction(const char* pszFunctionName);
    void SetDelay(float flDelay);

    // Execution methods
    void ExecuteLuaFunction();
    void ExecuteDelayedFunction();

private:
    string_t m_sFunctionName;       // Name of Lua function to execute
    float m_flDelay;                // Delay before execution
    bool m_bExecuteOnce;            // Execute only once
    bool m_bHasExecuted;            // Has already executed

    COutputEvent m_OnFunctionStart; // Fired when function starts
    COutputEvent m_OnFunctionEnd;   // Fired when function completes
    COutputEvent m_OnFunctionFail;  // Fired when function fails

    EHANDLE m_hActivator;           // Entity that activated this
};

//-----------------------------------------------------------------------------
// Game Setup System - Handles gmod_gamesetup functionality
// Based on IDA discovery: gmod_gamesetup, m_GamemodeScript
//-----------------------------------------------------------------------------
class CGModGameSetup
{
public:
    // Game setup functions
    static bool InitializeGame();
    static bool LoadGameScript(const char* pszScriptName);
    static bool ExecuteGameSetup();
    static void ResetGameState();

    // Gamemode script management
    static void SetGamemodeScript(const char* pszScriptPath);
    static const char* GetGamemodeScript();
    static bool IsGamemodeLoaded();

    // Player management
    static void SetupPlayer(CBasePlayer* pPlayer);
    static void CleanupPlayer(CBasePlayer* pPlayer);
    static void RespawnAllPlayers();

    // Game state management
    static void StartGame();
    static void EndGame();
    static void RestartGame();
    static bool IsGameActive();

private:
    static CUtlString s_GamemodeScript;    // m_GamemodeScript from IDA
    static bool s_bGameInitialized;
    static bool s_bGameActive;
    static float s_flGameStartTime;
};

// Console command handlers
void CMD_gmod_runfunction(void);
void CMD_gmod_gamesetup(void);

#endif // GMOD_RUNFUNCTION_H