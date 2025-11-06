#ifndef GMOD_SYSTEM_H
#define GMOD_SYSTEM_H

#include "cbase.h"
#include "igamesystem.h"
#include "igameevents.h"

// Forward declarations for all GMod systems
class CGModWeldSystem;
class CGModUndoSystem;
class CGModLuaSystem;
class CGModToolsSystem;
class CGModSWEPSystem;
class CGModGamemodeSystem;
class CGModModSystem;
class CGModOverlaySystem;
class CGModExpressionsSystem;
class CGModDeathSystem;
class CGModSchemeSystem;

// GMod system initialization phases
enum GMod_InitPhase_t
{
    GMOD_INIT_PHASE_EARLY = 0,      // Early initialization (before game systems)
    GMOD_INIT_PHASE_CORE,           // Core system initialization
    GMOD_INIT_PHASE_GAMEMODE,       // Gamemode loading and initialization
    GMOD_INIT_PHASE_LATE,           // Late initialization (after all systems)
    GMOD_INIT_PHASE_COMPLETE        // Initialization complete
};

// GMod system state
struct GMod_SystemState_t
{
    bool bInitialized;
    bool bGamemodeLoaded;
    bool bLuaInitialized;
    bool bConfigLoaded;
    char szCurrentGamemode[64];
    char szMapName[64];
    float flInitTime;
    int iPlayersConnected;

    GMod_SystemState_t()
    {
        bInitialized = false;
        bGamemodeLoaded = false;
        bLuaInitialized = false;
        bConfigLoaded = false;
        szCurrentGamemode[0] = '\0';
        szMapName[0] = '\0';
        flInitTime = 0.0f;
        iPlayersConnected = 0;
    }
};

//-----------------------------------------------------------------------------
// Main GMod System Manager - Coordinates all subsystems
// Based on IDA analysis of server initialization flow
//-----------------------------------------------------------------------------
class CGModSystem : public CAutoGameSystem, public IGameEventListener
{
public:
    CGModSystem();
    virtual ~CGModSystem();

    // CAutoGameSystem overrides - main HL2 engine hooks
    virtual bool Init();                    // Called during engine startup
    virtual void PostInit();               // Called after all systems init
    virtual void Shutdown();               // Called during engine shutdown
    virtual void LevelInitPreEntity();     // Called before entities spawn
    virtual void LevelInitPostEntity();    // Called after entities spawn (main gamemode load)
    virtual void LevelShutdownPreEntity(); // Called before level cleanup
    virtual void LevelShutdownPostEntity();// Called after level cleanup
    virtual void FrameUpdatePreEntityThink(); // Called every frame before entity think
    virtual void FrameUpdatePostEntityThink(); // Called every frame after entity think

    // IGameEventListener overrides
    virtual void FireGameEvent(KeyValues* event);
    virtual void GameEventsUpdated() {}
    virtual bool IsLocalListener() { return true; }

    // GMod initialization flow (based on IDA analysis)
    void InitializeGModSystems(GMod_InitPhase_t phase);
    bool LoadServerConfiguration();     // Load server.cfg, listenserver.cfg, etc.
    bool LoadGamemodeConfiguration();   // Load gamemode init.lua files
    bool LoadMapConfiguration();        // Load map-specific init files
    void InitializeLuaEnvironment();    // Initialize Lua 5.0 environment

    // Player connection handling (based on server event analysis)
    void OnPlayerConnect(CBasePlayer* pPlayer);
    void OnPlayerDisconnect(CBasePlayer* pPlayer);
    void OnPlayerSpawn(CBasePlayer* pPlayer);
    void OnPlayerDeath(CBasePlayer* pPlayer);

    // Gamemode management (based on CInfoGmodGameMode analysis)
    bool LoadGamemode(const char* pszGamemodeName);
    void UnloadCurrentGamemode();
    const char* GetCurrentGamemode() const { return m_SystemState.szCurrentGamemode; }
    bool IsGamemodeLoaded() const { return m_SystemState.bGamemodeLoaded; }

    // System state management
    const GMod_SystemState_t& GetSystemState() const { return m_SystemState; }
    bool IsSystemInitialized() const { return m_SystemState.bInitialized; }
    void ReloadAllSystems();

    // Configuration management (based on settings file analysis)
    void LoadAllConfigurations();
    void ReloadConfigurations();
    void SaveSystemConfiguration();

    // Console commands
    void ShowSystemStatus();
    void ResetAllSystems();
    void RunSystemDiagnostics();

    // Access to subsystems
    CGModWeldSystem* GetWeldSystem() const;
    CGModUndoSystem* GetUndoSystem() const;
    CGModLuaSystem* GetLuaSystem() const;
    CGModToolsSystem* GetToolsSystem() const;
    CGModSWEPSystem* GetSWEPSystem() const;
    CGModGamemodeSystem* GetGamemodeSystem() const;
    CGModModSystem* GetModSystem() const;
    CGModOverlaySystem* GetOverlaySystem() const;
    CGModExpressionsSystem* GetExpressionsSystem() const;
    CGModDeathSystem* GetDeathSystem() const;
    CGModSchemeSystem* GetSchemeSystem() const;

private:
    // System state
    GMod_SystemState_t m_SystemState;
    GMod_InitPhase_t m_CurrentInitPhase;

    // Initialization helpers
    void InitializePhaseEarly();       // Initialize core systems
    void InitializePhaseCore();        // Initialize game systems
    void InitializePhaseGamemode();    // Load and initialize gamemode
    void InitializePhaseLate();        // Finalize initialization

    // Configuration loading helpers
    bool LoadServerAllowsConfiguration();  // Load cfg/server_allows.txt
    bool LoadServerLimitsConfiguration();  // Load server limit ConVars
    void ParseConfigurationFile(const char* pszFilename);
    void ApplyConfigurationSettings();

    // Lua environment helpers
    void InitializeLuaGlobals();
    void LoadLuaInitFiles();            // Load lua/init/*.lua files
    void LoadGamemodeLuaFiles(const char* pszGamemode);

    // Event handling helpers
    void HandlePlayerEvent(KeyValues* event);
    void HandleGameEvent(KeyValues* event);
    void HandleSystemEvent(KeyValues* event);

    // Subsystem references (managed by respective systems)
    CGModWeldSystem* m_pWeldSystem;
    CGModUndoSystem* m_pUndoSystem;
    CGModLuaSystem* m_pLuaSystem;
    CGModToolsSystem* m_pToolsSystem;
    CGModSWEPSystem* m_pSWEPSystem;
    CGModGamemodeSystem* m_pGamemodeSystem;
    CGModModSystem* m_pModSystem;
    CGModOverlaySystem* m_pOverlaySystem;
    CGModExpressionsSystem* m_pExpressionsSystem;
    CGModDeathSystem* m_pDeathSystem;
    CGModSchemeSystem* m_pSchemeSystem;

    // Internal state
    bool m_bSystemsReady;
    float m_flLastDiagnosticTime;
    int m_iInitializationErrors;
};

// Global access to main GMod system
extern CGModSystem* g_pGModSystem;

// Console commands for system management
void CC_GMod_SystemStatus(void);
void CC_GMod_ReloadSystems(void);
void CC_GMod_ResetSystems(void);
void CC_GMod_SystemDiagnostics(void);
void CC_GMod_LoadGamemode(void);

#endif // GMOD_SYSTEM_H