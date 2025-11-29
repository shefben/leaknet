#include "cbase.h"
#include "gmod_system.h"
#include "player.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "convar.h"
#include "gameeventdefs.h"
#include "igameevents.h"
#include "gameinterface.h"

// Include all GMod subsystems
#include "gmod_weld.h"
#include "gmod_undo.h"
#include "gmod_lua.h"
#include "gmod_tools.h"
#include "gmod_swep.h"
#include "gmod_gamemode.h"
#include "gmod_mod.h"
#include "gmod_overlay.h"
#include "gmod_expressions.h"
#include "gmod_death.h"
#include "gmod_scheme.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Console variables for GMod system
ConVar gmod_enable("gmod_enable", "1", FCVAR_NONE, "Enable GMod functionality");
ConVar gmod_gamemode("gmod_gamemode", "sandbox", FCVAR_NONE, "Current gamemode");
ConVar gmod_debug("gmod_debug", "0", FCVAR_NONE, "Enable GMod debug output");

// Console commands for system management
ConCommand gmod_system_status("gmod_system_status", CC_GMod_SystemStatus, "Show GMod system status");
ConCommand gmod_reload_systems("gmod_reload_systems", CC_GMod_ReloadSystems, "Reload all GMod systems");
ConCommand gmod_reset_systems("gmod_reset_systems", CC_GMod_ResetSystems, "Reset all GMod systems");
ConCommand gmod_system_diagnostics("gmod_system_diagnostics", CC_GMod_SystemDiagnostics, "Run system diagnostics");
ConCommand gmod_load_gamemode("gmod_load_gamemode", CC_GMod_LoadGamemode, "Load a gamemode");

// Global instance
CGModSystem g_GMod_System;
CGModSystem* g_pGModSystem = &g_GMod_System;

// Global pointers to all GMod subsystems - needed for system coordination
// These point to the actual global instances defined in each subsystem's .cpp file
extern CGModWeldSystem g_GMod_WeldSystem;
extern CGModUndoSystem g_GMod_UndoSystem;
extern CGModLuaSystem g_GMod_LuaSystem;
extern CGModToolsSystem g_GMod_ToolsSystem;
extern CGModSWEPSystem g_GMod_SWEPSystem;
extern CGModGamemodeSystem g_GMod_GamemodeSystem;
extern CGModModSystem g_GMod_ModSystem;
extern CGModOverlaySystem g_GMod_OverlaySystem;
extern CGModExpressionsSystem g_GMod_ExpressionsSystem;
extern CGModDeathSystem g_GMod_DeathSystem;
extern CGModSchemeSystem g_GMod_SchemeSystem;

// Global pointers for easy access
CGModWeldSystem* g_pGModWeldSystem = &g_GMod_WeldSystem;
CGModUndoSystem* g_pGModUndoSystem = &g_GMod_UndoSystem;
CGModLuaSystem* g_pGModLuaSystem = &g_GMod_LuaSystem;
CGModToolsSystem* g_pGModToolsSystem = &g_GMod_ToolsSystem;
CGModSWEPSystem* g_pGModSWEPSystem = &g_GMod_SWEPSystem;
CGModGamemodeSystem* g_pGModGamemodeSystem = &g_GMod_GamemodeSystem;
CGModModSystem* g_pGModModSystem = &g_GMod_ModSystem;
extern CGModOverlaySystem* g_pGModOverlaySystem;
extern CGModExpressionsSystem* g_pGModExpressionsSystem;
extern CGModDeathSystem* g_pGModDeathSystem;
extern CGModSchemeSystem* g_pGModSchemeSystem;

//-----------------------------------------------------------------------------
// CGModSystem implementation - Main system coordinator
//-----------------------------------------------------------------------------
CGModSystem::CGModSystem() : CAutoGameSystem()
{
    m_CurrentInitPhase = GMOD_INIT_PHASE_EARLY;
    m_bSystemsReady = false;
    m_flLastDiagnosticTime = 0.0f;
    m_iInitializationErrors = 0;

    // Initialize subsystem pointers
    m_pWeldSystem = NULL;
    m_pUndoSystem = NULL;
    m_pLuaSystem = NULL;
    m_pToolsSystem = NULL;
    m_pSWEPSystem = NULL;
    m_pGamemodeSystem = NULL;
    m_pModSystem = NULL;
    m_pOverlaySystem = NULL;
    m_pExpressionsSystem = NULL;
    m_pDeathSystem = NULL;
    m_pSchemeSystem = NULL;
}

CGModSystem::~CGModSystem()
{
    Shutdown();
}

bool CGModSystem::Init()
{
    DevMsg("GMod System: Starting initialization...\n");

    // NOTE: Game event registration moved to PostInit() because
    // CGameEventManager::LoadEventsFromFile() may not have been called yet
    // during the early Init() phase. See PostInit() for event registration.

    // Initialize early phase systems
    InitializeGModSystems(GMOD_INIT_PHASE_EARLY);

    return true;
}

void CGModSystem::PostInit()
{
    // Register for game events now that GameEventManager has loaded gameevents.res
    // This was moved from Init() because events may not be registered that early
    ListenForGameEvent("player_connect");
    ListenForGameEvent("player_disconnect");
    ListenForGameEvent("player_spawn");
    ListenForGameEvent("player_death");
    ListenForGameEvent("game_newmap");
    ListenForGameEvent("server_spawn");
    ListenForGameEvent("server_cvar");
    ListenForGameEvent("server_shutdown");

    // Initialize core systems after all game systems are loaded
    InitializeGModSystems(GMOD_INIT_PHASE_CORE);

    // Get references to all subsystems (they're created as globals)
    m_pWeldSystem = g_pGModWeldSystem;
    m_pUndoSystem = g_pGModUndoSystem;
    m_pLuaSystem = g_pGModLuaSystem;
    m_pToolsSystem = g_pGModToolsSystem;
    m_pSWEPSystem = g_pGModSWEPSystem;
    m_pGamemodeSystem = g_pGModGamemodeSystem;
    m_pModSystem = g_pGModModSystem;
    m_pOverlaySystem = g_pGModOverlaySystem;
    m_pExpressionsSystem = g_pGModExpressionsSystem;
    m_pDeathSystem = g_pGModDeathSystem;
    m_pSchemeSystem = g_pGModSchemeSystem;

    DevMsg("GMod System: Post-initialization complete\n");
}

void CGModSystem::Shutdown()
{
    // Stop listening to all game events
    if (gameeventmanager)
    {
        gameeventmanager->RemoveListener(this);
    }

    m_SystemState.bInitialized = false;
    m_SystemState.bGamemodeLoaded = false;
    m_SystemState.bLuaInitialized = false;
    m_bSystemsReady = false;

    DevMsg("GMod System: Shutdown complete\n");
}

void CGModSystem::LevelInitPreEntity()
{
    DevMsg("GMod System: Level pre-entity initialization\n");

    // Store map name
    const char* pszMapName = STRING(gpGlobals->mapname);
    Q_strncpy(m_SystemState.szMapName, pszMapName, sizeof(m_SystemState.szMapName));

    // Load server and map configuration
    LoadServerConfiguration();
    LoadMapConfiguration();
}

void CGModSystem::LevelInitPostEntity()
{
    DevMsg("GMod System: Level post-entity initialization\n");

    // Initialize gamemode phase - this is the main gamemode loading
    InitializeGModSystems(GMOD_INIT_PHASE_GAMEMODE);

    // Initialize late phase systems
    InitializeGModSystems(GMOD_INIT_PHASE_LATE);

    // Mark system as fully initialized
    m_CurrentInitPhase = GMOD_INIT_PHASE_COMPLETE;
    m_SystemState.bInitialized = true;
    m_SystemState.flInitTime = gpGlobals->curtime;
    m_bSystemsReady = true;

    DevMsg("GMod System: Full initialization complete - gamemode: %s, map: %s\n",
        m_SystemState.szCurrentGamemode, m_SystemState.szMapName);
}

void CGModSystem::LevelShutdownPreEntity()
{
    DevMsg("GMod System: Level shutdown starting\n");

    // Unload current gamemode
    UnloadCurrentGamemode();

    // Reset system state
    m_SystemState.bGamemodeLoaded = false;
    m_SystemState.iPlayersConnected = 0;
    m_bSystemsReady = false;
}

void CGModSystem::LevelShutdownPostEntity()
{
    DevMsg("GMod System: Level shutdown complete\n");
}

void CGModSystem::FrameUpdatePreEntityThink()
{
    if (!m_bSystemsReady)
        return;

    // Run diagnostics periodically
    if (gmod_debug.GetBool() && gpGlobals->curtime > m_flLastDiagnosticTime + 10.0f)
    {
        m_flLastDiagnosticTime = gpGlobals->curtime;
        // Could run lightweight diagnostics here
    }
}

void CGModSystem::FrameUpdatePostEntityThink()
{
    // Update system state
    if (m_bSystemsReady)
    {
        // Update player count
        int iConnectedPlayers = 0;
        for (int i = 1; i <= gpGlobals->maxClients; i++)
        {
            CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
            if (pPlayer)
                iConnectedPlayers++;
        }
        m_SystemState.iPlayersConnected = iConnectedPlayers;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Helper to listen for game events (compatibility wrapper)
//-----------------------------------------------------------------------------
void CGModSystem::ListenForGameEvent(const char* pszEventName)
{
    if (!pszEventName)
        return;

    // Get the game event manager and add listener for this event
    if (gameeventmanager)
    {
        gameeventmanager->AddListener(this, pszEventName);
    }
}

//-----------------------------------------------------------------------------
// Helper function to get player by user ID (for game events)
//-----------------------------------------------------------------------------
static CBasePlayer* GetPlayerByUserId(int userid)
{
    // Iterate through all players to find matching user ID
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
        if (pPlayer && engine->GetPlayerUserId(pPlayer->edict()) == userid)
        {
            return pPlayer;
        }
    }
    return NULL;
}

void CGModSystem::FireGameEvent(KeyValues* event)
{
    const char* pszEventName = event->GetName();

    if (Q_stricmp(pszEventName, "player_connect") == 0)
    {
        int userid = event->GetInt("userid");
        CBasePlayer* pPlayer = GetPlayerByUserId(userid);
        if (pPlayer)
        {
            OnPlayerConnect(pPlayer);
        }
    }
    else if (Q_stricmp(pszEventName, "player_disconnect") == 0)
    {
        int userid = event->GetInt("userid");
        CBasePlayer* pPlayer = GetPlayerByUserId(userid);
        if (pPlayer)
        {
            OnPlayerDisconnect(pPlayer);
        }
    }
    else if (Q_stricmp(pszEventName, "player_spawn") == 0)
    {
        int userid = event->GetInt("userid");
        CBasePlayer* pPlayer = GetPlayerByUserId(userid);
        if (pPlayer)
        {
            OnPlayerSpawn(pPlayer);
        }
    }
    else if (Q_stricmp(pszEventName, "player_death") == 0)
    {
        int userid = event->GetInt("userid");
        CBasePlayer* pPlayer = GetPlayerByUserId(userid);
        if (pPlayer)
        {
            OnPlayerDeath(pPlayer);
        }
    }
    else if (Q_stricmp(pszEventName, "server_spawn") == 0)
    {
        // Server has started - ensure systems are ready
        if (!m_bSystemsReady && m_CurrentInitPhase == GMOD_INIT_PHASE_COMPLETE)
        {
            m_bSystemsReady = true;
            DevMsg("GMod System: Server spawn event - systems ready\n");
        }
    }
}

//-----------------------------------------------------------------------------
// Initialization phases (based on IDA analysis of server startup)
//-----------------------------------------------------------------------------
void CGModSystem::InitializeGModSystems(GMod_InitPhase_t phase)
{
    m_CurrentInitPhase = phase;

    switch (phase)
    {
        case GMOD_INIT_PHASE_EARLY:
            InitializePhaseEarly();
            break;

        case GMOD_INIT_PHASE_CORE:
            InitializePhaseCore();
            break;

        case GMOD_INIT_PHASE_GAMEMODE:
            InitializePhaseGamemode();
            break;

        case GMOD_INIT_PHASE_LATE:
            InitializePhaseLate();
            break;

        default:
            break;
    }
}

void CGModSystem::InitializePhaseEarly()
{
    DevMsg("GMod System: Early initialization phase\n");

    // Initialize Lua environment first (everything depends on it)
    InitializeLuaEnvironment();

    // Load core configurations
    LoadAllConfigurations();
}

void CGModSystem::InitializePhaseCore()
{
    DevMsg("GMod System: Core initialization phase\n");

    // Core systems should already be initialized by their constructors
    // Just verify they're working properly
    if (!m_pLuaSystem || !m_pLuaSystem->IsInitialized())
    {
        Warning("GMod System: Lua system not properly initialized!\n");
        m_iInitializationErrors++;
    }
}

void CGModSystem::InitializePhaseGamemode()
{
    DevMsg("GMod System: Gamemode initialization phase\n");

    // Load the specified gamemode (based on IDA analysis of gamemode loading)
    const char* pszGamemode = gmod_gamemode.GetString();
    if (pszGamemode && pszGamemode[0])
    {
        LoadGamemode(pszGamemode);
    }
    else
    {
        // Default to sandbox if no gamemode specified
        LoadGamemode("sandbox");
    }
}

void CGModSystem::InitializePhaseLate()
{
    DevMsg("GMod System: Late initialization phase\n");

    // Finalize all systems
    if (m_pGamemodeSystem && m_SystemState.bGamemodeLoaded)
    {
        // Initialize gamemode-specific functionality
        m_pGamemodeSystem->PostGamemodeLoad();
    }

    // Load Lua init files (based on IDA analysis: init/init.lua, lua/init/*.lua)
    LoadLuaInitFiles();
}

//-----------------------------------------------------------------------------
// Configuration loading (based on IDA analysis of config file loading)
//-----------------------------------------------------------------------------
bool CGModSystem::LoadServerConfiguration()
{
    DevMsg("GMod System: Loading server configuration\n");

    // Load server.cfg or listenserver.cfg (based on IDA strings)
    const char* pszServerCfg = "server.cfg";
    if (engine->IsDedicatedServer())
    {
        DevMsg("Executing dedicated server config file\n");
        char szCommand[256];
        Q_snprintf(szCommand, sizeof(szCommand), "exec %s\n", pszServerCfg);
        engine->ServerCommand(szCommand);
    }
    else
    {
        DevMsg("Executing listen server config file\n");
        engine->ServerCommand("exec listenserver.cfg\n");
    }

    // Load server_allows.txt (based on IDA analysis)
    LoadServerAllowsConfiguration();

    // Load server limits configuration
    LoadServerLimitsConfiguration();

    m_SystemState.bConfigLoaded = true;
    return true;
}

bool CGModSystem::LoadGamemodeConfiguration()
{
    const char* pszGamemode = m_SystemState.szCurrentGamemode;
    if (!pszGamemode[0])
        return false;

    DevMsg("GMod System: Loading gamemode configuration for %s\n", pszGamemode);

    // Load gamemode init.lua file (based on IDA strings: gamemodes/%s/init.lua)
    LoadGamemodeLuaFiles(pszGamemode);

    return true;
}

bool CGModSystem::LoadMapConfiguration()
{
    DevMsg("GMod System: Loading map configuration\n");

    // Load map-specific init files if they exist
    char szMapInitFile[MAX_PATH];
    Q_snprintf(szMapInitFile, sizeof(szMapInitFile), "maps/%s_init.lua", m_SystemState.szMapName);

    if (filesystem->FileExists(szMapInitFile, "GAME"))
    {
        DevMsg("Loading Map Init File .. [%s]\n", szMapInitFile);
        if (m_pLuaSystem)
        {
            m_pLuaSystem->RunLuaFile(szMapInitFile);
        }
    }
    else
    {
        DevMsg("Init file not found.\n");
    }

    return true;
}

bool CGModSystem::LoadServerAllowsConfiguration()
{
    // Based on IDA string: "cfg/server_allows.txt"
    const char* pszFilename = "cfg/server_allows.txt";

    if (!filesystem->FileExists(pszFilename, "GAME"))
    {
        Warning("Couldn't find the 'cfg/server_allows.txt'! Can't continue without it.\n");
        return false;
    }

    KeyValues* pKV = new KeyValues("Server Allows");
    if (!pKV->LoadFromFile(filesystem, pszFilename, "GAME"))
    {
        Warning("Failed to parse server_allows.txt\n");
        pKV->deleteThis();
        return false;
    }

    // Parse server allows settings
    KeyValues* pPlayerModels = pKV->FindKey("playermodels");
    if (!pPlayerModels)
    {
        Warning("Couldn't find the playermodels key (in cfg/server_allows.txt). Can't continue without it.\n");
        pKV->deleteThis();
        return false;
    }

    // Process player model restrictions
    // (Implementation would depend on how player models are managed)

    pKV->deleteThis();
    DevMsg("Loaded server allows configuration\n");
    return true;
}

bool CGModSystem::LoadServerLimitsConfiguration()
{
    // Based on IDA strings: gm_sv_serverlimit_* ConVars
    // These are already defined as ConVars in the respective systems

    DevMsg("Server limits configuration loaded\n");
    return true;
}

//-----------------------------------------------------------------------------
// Lua environment management
//-----------------------------------------------------------------------------
void CGModSystem::InitializeLuaEnvironment()
{
    DevMsg("GMod System: Initializing Lua environment\n");

    if (m_pLuaSystem)
    {
        if (m_pLuaSystem->Initialize())
        {
            m_SystemState.bLuaInitialized = true;
            InitializeLuaGlobals();
        }
        else
        {
            Warning("Failed to initialize Lua system!\n");
            m_iInitializationErrors++;
        }
    }
}

void CGModSystem::InitializeLuaGlobals()
{
    if (!m_pLuaSystem || !m_pLuaSystem->IsInitialized())
        return;

    // Register global Lua functions and variables
    // This would include engine bindings, gamemode functions, etc.
    m_pLuaSystem->RegisterGlobalFunctions();
}

void CGModSystem::LoadLuaInitFiles()
{
    if (!m_pLuaSystem || !m_pLuaSystem->IsInitialized())
        return;

    DevMsg("GMod System: Loading Lua init files\n");

    // Load init/init.lua (based on IDA string)
    if (filesystem->FileExists("init/init.lua", "GAME"))
    {
        DevMsg("Lua Script: init/init.lua\n");
        m_pLuaSystem->RunLuaFile("init/init.lua");
    }

    // Load all files from lua/init/*.lua
    FileFindHandle_t findHandle;
    const char* pszFilename = filesystem->FindFirst("lua/init/*.lua", &findHandle);
    while (pszFilename)
    {
        char szFullPath[MAX_PATH];
        Q_snprintf(szFullPath, sizeof(szFullPath), "lua/init/%s", pszFilename);

        DevMsg("Loading Lua init file: %s\n", szFullPath);
        m_pLuaSystem->RunLuaFile(szFullPath);

        pszFilename = filesystem->FindNext(findHandle);
    }
    filesystem->FindClose(findHandle);
}

void CGModSystem::LoadGamemodeLuaFiles(const char* pszGamemode)
{
    if (!m_pLuaSystem || !m_pLuaSystem->IsInitialized() || !pszGamemode)
        return;

    // Load gamemode init.lua (based on IDA strings)
    char szGamemodeInit[MAX_PATH];
    Q_snprintf(szGamemodeInit, sizeof(szGamemodeInit), "gamemodes/%s/init.lua", pszGamemode);

    if (filesystem->FileExists(szGamemodeInit, "GAME"))
    {
        DevMsg("Loading gamemode init file: %s\n", szGamemodeInit);
        m_pLuaSystem->RunLuaFile(szGamemodeInit);
    }
}

//-----------------------------------------------------------------------------
// Player event handling
//-----------------------------------------------------------------------------
void CGModSystem::OnPlayerConnect(CBasePlayer* pPlayer)
{
    if (!pPlayer || !m_bSystemsReady)
        return;

    DevMsg("GMod System: Player %s connected\n", STRING(pPlayer->pl.netname));

    // Initialize player in all subsystems
    if (m_pUndoSystem)
        m_pUndoSystem->InitializePlayer(pPlayer);

    if (m_pToolsSystem)
        m_pToolsSystem->InitializePlayer(pPlayer);

    if (m_pGamemodeSystem)
        m_pGamemodeSystem->OnPlayerConnect(pPlayer);
}

void CGModSystem::OnPlayerDisconnect(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    DevMsg("GMod System: Player %s disconnected\n", STRING(pPlayer->pl.netname));

    // Clean up player from all subsystems
    if (m_pUndoSystem)
        m_pUndoSystem->CleanupPlayer(pPlayer);

    if (m_pToolsSystem)
        m_pToolsSystem->CleanupPlayer(pPlayer);

    if (m_pGamemodeSystem)
        m_pGamemodeSystem->OnPlayerDisconnect(pPlayer);
}

void CGModSystem::OnPlayerSpawn(CBasePlayer* pPlayer)
{
    if (!pPlayer || !m_bSystemsReady)
        return;

    DevMsg("GMod System: Player %s spawned\n", STRING(pPlayer->pl.netname));

    // Notify all subsystems of player spawn
    if (m_pGamemodeSystem)
        m_pGamemodeSystem->OnPlayerSpawn(pPlayer);

    if (m_pSWEPSystem)
        m_pSWEPSystem->OnPlayerSpawn(pPlayer);
}

void CGModSystem::OnPlayerDeath(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    DevMsg("GMod System: Player %s died\n", STRING(pPlayer->pl.netname));

    // Notify all subsystems of player death
    if (m_pGamemodeSystem)
        m_pGamemodeSystem->OnPlayerDeath(pPlayer, NULL);
}

//-----------------------------------------------------------------------------
// Gamemode management
//-----------------------------------------------------------------------------
bool CGModSystem::LoadGamemode(const char* pszGamemodeName)
{
    if (!pszGamemodeName || !pszGamemodeName[0])
        return false;

    DevMsg("GMod System: Loading gamemode: %s\n", pszGamemodeName);

    // Unload current gamemode
    UnloadCurrentGamemode();

    // Set new gamemode
    Q_strncpy(m_SystemState.szCurrentGamemode, pszGamemodeName, sizeof(m_SystemState.szCurrentGamemode));
    gmod_gamemode.SetValue(pszGamemodeName);

    // Load gamemode configuration
    LoadGamemodeConfiguration();

    // Initialize gamemode in subsystem
    if (m_pGamemodeSystem)
    {
        if (m_pGamemodeSystem->LoadGamemode(pszGamemodeName))
        {
            m_SystemState.bGamemodeLoaded = true;
            DevMsg("GMod System: Gamemode %s loaded successfully\n", pszGamemodeName);
            return true;
        }
        else
        {
            Warning("GMod System: Failed to load gamemode %s\n", pszGamemodeName);
        }
    }

    return false;
}

void CGModSystem::UnloadCurrentGamemode()
{
    if (m_SystemState.bGamemodeLoaded && m_pGamemodeSystem)
    {
        DevMsg("GMod System: Unloading gamemode: %s\n", m_SystemState.szCurrentGamemode);
        m_pGamemodeSystem->UnloadGamemode();
        m_SystemState.bGamemodeLoaded = false;
        m_SystemState.szCurrentGamemode[0] = '\0';
    }
}

//-----------------------------------------------------------------------------
// System management
//-----------------------------------------------------------------------------
void CGModSystem::LoadAllConfigurations()
{
    DevMsg("GMod System: Loading all configurations\n");

    // Load configurations for all subsystems
    if (m_pOverlaySystem)
        m_pOverlaySystem->LoadOverlaySettings();

    if (m_pExpressionsSystem)
        m_pExpressionsSystem->LoadExpressions();

    if (m_pSchemeSystem)
        m_pSchemeSystem->LoadSchemeSettings();

    if (m_pModSystem)
        m_pModSystem->LoadModCache();
}

void CGModSystem::ReloadAllSystems()
{
    DevMsg("GMod System: Reloading all systems\n");

    // Reload all subsystem configurations
    LoadAllConfigurations();

    // Reload gamemode if one is loaded
    if (m_SystemState.bGamemodeLoaded)
    {
        const char* pszCurrentGamemode = m_SystemState.szCurrentGamemode;
        UnloadCurrentGamemode();
        LoadGamemode(pszCurrentGamemode);
    }
}

void CGModSystem::ReloadConfigurations()
{
    LoadAllConfigurations();
}

void CGModSystem::ResetAllSystems()
{
    DevMsg("GMod System: Resetting all systems\n");

    // Reset all subsystems to their default states
    if (m_pUndoSystem)
        m_pUndoSystem->ClearAllUndo();

    if (m_pWeldSystem)
        m_pWeldSystem->RemoveAllConstraints();

    // Reset system state
    m_SystemState.bInitialized = false;
    m_SystemState.bGamemodeLoaded = false;
    m_SystemState.bLuaInitialized = false;
    m_SystemState.bConfigLoaded = false;
    m_SystemState.szCurrentGamemode[0] = '\0';
    m_SystemState.iPlayersConnected = 0;
    m_iInitializationErrors = 0;

    // Reload everything
    ReloadAllSystems();
}

void CGModSystem::ShowSystemStatus()
{
    Msg("=== GMod System Status ===\n");
    Msg("Initialized: %s\n", m_SystemState.bInitialized ? "Yes" : "No");
    Msg("Systems Ready: %s\n", m_bSystemsReady ? "Yes" : "No");
    Msg("Lua Initialized: %s\n", m_SystemState.bLuaInitialized ? "Yes" : "No");
    Msg("Gamemode Loaded: %s\n", m_SystemState.bGamemodeLoaded ? "Yes" : "No");
    Msg("Current Gamemode: %s\n", m_SystemState.szCurrentGamemode[0] ? m_SystemState.szCurrentGamemode : "None");
    Msg("Current Map: %s\n", m_SystemState.szMapName[0] ? m_SystemState.szMapName : "None");
    Msg("Players Connected: %d\n", m_SystemState.iPlayersConnected);
    Msg("Init Phase: %d\n", (int)m_CurrentInitPhase);
    Msg("Init Errors: %d\n", m_iInitializationErrors);
    Msg("Init Time: %.2f\n", m_SystemState.flInitTime);
    Msg("========================\n");
}

void CGModSystem::RunSystemDiagnostics()
{
    Msg("=== GMod System Diagnostics ===\n");

    // Check all subsystems
    int iSystemErrors = 0;

    if (!m_pLuaSystem || !m_pLuaSystem->IsInitialized())
    {
        Msg("ERROR: Lua system not initialized\n");
        iSystemErrors++;
    }

    if (!m_pWeldSystem)
    {
        Msg("ERROR: Weld system not found\n");
        iSystemErrors++;
    }

    if (!m_pUndoSystem)
    {
        Msg("ERROR: Undo system not found\n");
        iSystemErrors++;
    }

    // Add more diagnostic checks as needed

    if (iSystemErrors == 0)
    {
        Msg("All systems operational\n");
    }
    else
    {
        Msg("Found %d system errors\n", iSystemErrors);
    }

    Msg("==============================\n");
}

//-----------------------------------------------------------------------------
// Subsystem accessors
//-----------------------------------------------------------------------------
CGModWeldSystem* CGModSystem::GetWeldSystem() const { return m_pWeldSystem; }
CGModUndoSystem* CGModSystem::GetUndoSystem() const { return m_pUndoSystem; }
CGModLuaSystem* CGModSystem::GetLuaSystem() const { return m_pLuaSystem; }
CGModToolsSystem* CGModSystem::GetToolsSystem() const { return m_pToolsSystem; }
CGModSWEPSystem* CGModSystem::GetSWEPSystem() const { return m_pSWEPSystem; }
CGModGamemodeSystem* CGModSystem::GetGamemodeSystem() const { return m_pGamemodeSystem; }
CGModModSystem* CGModSystem::GetModSystem() const { return m_pModSystem; }
CGModOverlaySystem* CGModSystem::GetOverlaySystem() const { return m_pOverlaySystem; }
CGModExpressionsSystem* CGModSystem::GetExpressionsSystem() const { return m_pExpressionsSystem; }
CGModDeathSystem* CGModSystem::GetDeathSystem() const { return m_pDeathSystem; }
CGModSchemeSystem* CGModSystem::GetSchemeSystem() const { return m_pSchemeSystem; }

//-----------------------------------------------------------------------------
// Console command implementations
//-----------------------------------------------------------------------------
void CC_GMod_SystemStatus(void)
{
    if (g_pGModSystem)
    {
        g_pGModSystem->ShowSystemStatus();
    }
}

void CC_GMod_ReloadSystems(void)
{
    if (g_pGModSystem)
    {
        g_pGModSystem->ReloadAllSystems();
        Msg("All GMod systems reloaded.\n");
    }
}

void CC_GMod_ResetSystems(void)
{
    if (g_pGModSystem)
    {
        g_pGModSystem->ResetAllSystems();
        Msg("All GMod systems reset.\n");
    }
}

void CC_GMod_SystemDiagnostics(void)
{
    if (g_pGModSystem)
    {
        g_pGModSystem->RunSystemDiagnostics();
    }
}

void CC_GMod_LoadGamemode(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_load_gamemode <gamemode_name>\n");
        return;
    }

    const char* pszGamemode = engine->Cmd_Argv(1);

    if (g_pGModSystem)
    {
        if (g_pGModSystem->LoadGamemode(pszGamemode))
        {
            Msg("Gamemode '%s' loaded successfully\n", pszGamemode);
        }
        else
        {
            Msg("Failed to load gamemode '%s'\n", pszGamemode);
        }
    }
}

//-----------------------------------------------------------------------------
// GMod 9 compatibility console commands
// Based on IDA Pro analysis of GMod server.dll CMD_* functions
//-----------------------------------------------------------------------------

// gm_sv_setrules - Sets the rules from console commands (enforce rule changes)
void CC_GMod_SetRules(void)
{
    // Re-apply server rules without map restart
    if (g_pGModSystem)
    {
        g_pGModSystem->ReloadConfigurations();
        Msg("Server rules have been reloaded and enforced.\n");
    }
}
static ConCommand gm_sv_setrules("gm_sv_setrules", CC_GMod_SetRules,
    "Sets the rules from the console commands. Call this if you have changed a rule and need to enforce it without restarting the map");

// gm_showhelp - Show help (calls Lua GM:ShowHelp hook)
void CC_GMod_ShowHelp(void)
{
    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    // Call Lua hook if available
    if (g_pGModSystem && g_pGModSystem->GetLuaSystem())
    {
        CGModLuaSystem* pLua = g_pGModSystem->GetLuaSystem();
        if (pLua->IsInitialized())
        {
            // Run Lua hook: GM:ShowHelp(player)
            char szCmd[256];
            Q_snprintf(szCmd, sizeof(szCmd), "hook.Call(\"ShowHelp\", GAMEMODE, Entity(%d))", pPlayer->entindex());
            CGModLuaSystem::ExecuteString(szCmd);
        }
    }

    DevMsg("ShowHelp hook called for player %d\n", pPlayer->entindex());
}
static ConCommand gm_showhelp("gm_showhelp", CC_GMod_ShowHelp, "Shows the help screen (calls GM:ShowHelp hook)");

// gm_showteam - Show team selection (calls Lua GM:ShowTeam hook)
void CC_GMod_ShowTeam(void)
{
    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    // Call Lua hook if available
    if (g_pGModSystem && g_pGModSystem->GetLuaSystem())
    {
        CGModLuaSystem* pLua = g_pGModSystem->GetLuaSystem();
        if (pLua->IsInitialized())
        {
            // Run Lua hook: GM:ShowTeam(player)
            char szCmd[256];
            Q_snprintf(szCmd, sizeof(szCmd), "hook.Call(\"ShowTeam\", GAMEMODE, Entity(%d))", pPlayer->entindex());
            CGModLuaSystem::ExecuteString(szCmd);
        }
    }

    DevMsg("ShowTeam hook called for player %d\n", pPlayer->entindex());
}
static ConCommand gm_showteam("gm_showteam", CC_GMod_ShowTeam, "Shows the team selection (calls GM:ShowTeam hook)");

// gm_showspare1 - Spare hook 1 (calls Lua GM:ShowSpare1 hook)
void CC_GMod_ShowSpare1(void)
{
    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    // Call Lua hook if available
    if (g_pGModSystem && g_pGModSystem->GetLuaSystem())
    {
        CGModLuaSystem* pLua = g_pGModSystem->GetLuaSystem();
        if (pLua->IsInitialized())
        {
            // Run Lua hook: GM:ShowSpare1(player)
            char szCmd[256];
            Q_snprintf(szCmd, sizeof(szCmd), "hook.Call(\"ShowSpare1\", GAMEMODE, Entity(%d))", pPlayer->entindex());
            CGModLuaSystem::ExecuteString(szCmd);
        }
    }

    DevMsg("ShowSpare1 hook called for player %d\n", pPlayer->entindex());
}
static ConCommand gm_showspare1("gm_showspare1", CC_GMod_ShowSpare1, "Shows spare menu 1 (calls GM:ShowSpare1 hook)");

// gm_showspare2 - Spare hook 2 (calls Lua GM:ShowSpare2 hook)
void CC_GMod_ShowSpare2(void)
{
    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    // Call Lua hook if available
    if (g_pGModSystem && g_pGModSystem->GetLuaSystem())
    {
        CGModLuaSystem* pLua = g_pGModSystem->GetLuaSystem();
        if (pLua->IsInitialized())
        {
            // Run Lua hook: GM:ShowSpare2(player)
            char szCmd[256];
            Q_snprintf(szCmd, sizeof(szCmd), "hook.Call(\"ShowSpare2\", GAMEMODE, Entity(%d))", pPlayer->entindex());
            CGModLuaSystem::ExecuteString(szCmd);
        }
    }

    DevMsg("ShowSpare2 hook called for player %d\n", pPlayer->entindex());
}
static ConCommand gm_showspare2("gm_showspare2", CC_GMod_ShowSpare2, "Shows spare menu 2 (calls GM:ShowSpare2 hook)");

// gm_explode - Explode the player
void CC_GMod_Explode(void)
{
    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    // Create an explosion at player's position
    Vector vecOrigin = pPlayer->GetAbsOrigin();

    // Kill the player
    CTakeDamageInfo info(pPlayer, pPlayer, 1000, DMG_BLAST);
    pPlayer->TakeDamage(info);

    // Create explosion effect
    CPASFilter filter(vecOrigin);
    te->Explosion(filter, 0.0f, &vecOrigin,
        -1,  // No custom explosion model
        100,  // Radius
        15,   // Magnitude
        TE_EXPLFLAG_NONE,
        200,  // Damage radius
        0);   // No decal

    DevMsg("Player %d exploded!\n", pPlayer->entindex());
}
static ConCommand gm_explode("gm_explode", CC_GMod_Explode, "Boom! Explode yourself");

// lua - Run a Lua command (GMod 9 compatibility alias)
void CC_LuaCommand(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: lua <lua_code>\n");
        return;
    }

    // Combine all arguments into a single Lua command
    char szLuaCmd[4096];
    szLuaCmd[0] = '\0';

    for (int i = 1; i < engine->Cmd_Argc(); i++)
    {
        if (i > 1)
            Q_strncat(szLuaCmd, " ", sizeof(szLuaCmd));
        Q_strncat(szLuaCmd, engine->Cmd_Argv(i), sizeof(szLuaCmd));
    }

    if (g_pGModSystem && g_pGModSystem->GetLuaSystem())
    {
        CGModLuaSystem* pLua = g_pGModSystem->GetLuaSystem();
        if (pLua->IsInitialized())
        {
            CGModLuaSystem::ExecuteString(szLuaCmd);
            DevMsg("Executed Lua: %s\n", szLuaCmd);
        }
        else
        {
            Warning("Lua system not initialized!\n");
        }
    }
}
static ConCommand lua_cmd("lua", CC_LuaCommand, "Runs a Lua string command on the server");

// SearchPaths - Debug command to list all search paths
// Note: GetSearchPath not available in HL2 beta IFileSystem
void CC_SearchPaths(void)
{
    Msg("=== Search Paths ===\n");

    // GetSearchPath not available in HL2 beta engine
    // Print basic info instead
    Msg("GAME: (use -game parameter)\n");
    Msg("MOD: (use -mod parameter)\n");
    Msg("Note: GetSearchPath not available in HL2 beta engine\n");

    Msg("====================\n");
}
static ConCommand searchpaths_cmd("SearchPaths", CC_SearchPaths, "List all search paths");

// gm_makeentity - Create an entity (GMod 9 compatibility)
void CC_GMod_MakeEntity(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gm_makeentity <classname>\n");
        return;
    }

    const char* pszClassName = engine->Cmd_Argv(1);

    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    // Get spawn position (in front of player)
    Vector vecForward;
    pPlayer->EyeVectors(&vecForward);
    Vector vecOrigin = pPlayer->EyePosition() + vecForward * 100;

    // Create the entity
    CBaseEntity* pEntity = CreateEntityByName(pszClassName);
    if (pEntity)
    {
        pEntity->SetAbsOrigin(vecOrigin);
        pEntity->SetAbsAngles(QAngle(0, pPlayer->EyeAngles().y, 0));
        DispatchSpawn(pEntity);
        pEntity->Activate();

        Msg("Created entity '%s' at player position\n", pszClassName);
    }
    else
    {
        Msg("Failed to create entity '%s'\n", pszClassName);
    }
}
static ConCommand gm_makeentity("gm_makeentity", CC_GMod_MakeEntity, "Create an entity by classname");

// optionselect - Option selection (for menu systems)
void CC_OptionSelect(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: optionselect <option_id>\n");
        return;
    }

    int iOption = atoi(engine->Cmd_Argv(1));

    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    // Call Lua hook for option selection
    if (g_pGModSystem && g_pGModSystem->GetLuaSystem())
    {
        CGModLuaSystem* pLua = g_pGModSystem->GetLuaSystem();
        if (pLua->IsInitialized())
        {
            char szCmd[256];
            Q_snprintf(szCmd, sizeof(szCmd), "hook.Call(\"OptionSelect\", GAMEMODE, Entity(%d), %d)",
                pPlayer->entindex(), iOption);
            CGModLuaSystem::ExecuteString(szCmd);
        }
    }

    DevMsg("OptionSelect: player %d selected option %d\n", pPlayer->entindex(), iOption);
}
static ConCommand optionselect_cmd("optionselect", CC_OptionSelect, "Select a menu option");

// SetModel - Set player model (GMod 9 compatibility)
void CC_SetModel(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: SetModel <model_path>\n");
        return;
    }

    const char* pszModel = engine->Cmd_Argv(1);

    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    // Set the player model
    pPlayer->SetModel(pszModel);

    Msg("Player model set to: %s\n", pszModel);
}
static ConCommand setmodel_cmd("SetModel", CC_SetModel, "Choose a player model");
