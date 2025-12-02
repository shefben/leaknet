//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Server Rules & Game Mode System Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_serverrules.h"
#include "player.h"
#include "ai_basenpc.h"
#include "tier1/strtools.h"
#include "game.h"
#include "physics.h"

// Initialize static members
bool CGModServerRules::m_bInitialized = false;
GMod_GameMode_t CGModServerRules::m_CurrentGameMode = GAMEMODE_SANDBOX;
float CGModServerRules::m_flLastCleanupTime = 0.0f;

// Console variables for server rules
ConVar gm_sv_gamemode("gm_sv_gamemode", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Game mode (0=Sandbox, 1=Build, 2=RP, 3=DM, 4=Team, 5=Custom)");
ConVar gm_sv_allownpc("gm_sv_allownpc", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Allow NPC spawning");
ConVar gm_sv_npchealthmultiplier("gm_sv_npchealthmultiplier", "1.0", FCVAR_GAMEDLL, "NPC health multiplier");
ConVar gm_sv_allowweapons("gm_sv_allowweapons", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Allow weapon spawning");
ConVar gm_sv_allowphysgun("gm_sv_allowphysgun", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Allow physics gun usage");
ConVar gm_sv_allowtoolgun("gm_sv_allowtoolgun", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Allow tool gun usage");
ConVar gm_sv_maxprops("gm_sv_maxprops", "1000", FCVAR_GAMEDLL, "Maximum props on server");
ConVar gm_sv_maxragdolls("gm_sv_maxragdolls", "200", FCVAR_GAMEDLL, "Maximum ragdolls on server");
ConVar gm_sv_maxnpcs("gm_sv_maxnpcs", "50", FCVAR_GAMEDLL, "Maximum NPCs on server");
ConVar gm_sv_respawntime("gm_sv_respawntime", "5.0", FCVAR_GAMEDLL, "Player respawn time in seconds");
ConVar gm_sv_noclip("gm_sv_noclip", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Allow noclip for all players");
ConVar gm_sv_god("gm_sv_god", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY, "God mode for all players");
ConVar gm_sv_gravity("gm_sv_gravity", "600", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Server gravity setting");
ConVar gm_sv_airaccelerate("gm_sv_airaccelerate", "10", FCVAR_GAMEDLL, "Air acceleration multiplier");
ConVar gm_sv_friction("gm_sv_friction", "4", FCVAR_GAMEDLL, "Ground friction multiplier");
ConVar gm_sv_bounce("gm_sv_bounce", "0", FCVAR_GAMEDLL, "Physics bounce multiplier");
ConVar gm_sv_allowchat("gm_sv_allowchat", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Allow text chat");
ConVar gm_sv_allowvoice("gm_sv_allowvoice", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Allow voice chat");
ConVar gm_sv_teamplay("gm_sv_teamplay", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Enable team play mode");
ConVar gm_sv_friendly_fire("gm_sv_friendly_fire", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Enable friendly fire in team mode");

// Additional cvars required by client gmod_menus.cpp
ConVar gm_sv_allweapons("gm_sv_allweapons", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED, "Give all weapons to players");
ConVar gm_sv_allowignite("gm_sv_allowignite", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow igniting objects");
ConVar gm_sv_playerdamage("gm_sv_playerdamage", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED, "Enable player damage");
ConVar gm_sv_pvpdamage("gm_sv_pvpdamage", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED, "Enable PvP damage");
ConVar gm_sv_teamdamage("gm_sv_teamdamage", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED, "Enable team damage");
ConVar gm_sv_allowspawning("gm_sv_allowspawning", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow spawning entities");
ConVar gm_sv_allowmultigun("gm_sv_allowmultigun", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED, "Allow multiple physics gun targets");

//-----------------------------------------------------------------------------
// Purpose: Initialize server rules system
//-----------------------------------------------------------------------------
void CGModServerRules::Initialize()
{
    if (m_bInitialized)
        return;

    SetGameMode((GMod_GameMode_t)gm_sv_gamemode.GetInt());
    ApplyGameModeRules();

    m_flLastCleanupTime = gpGlobals->curtime;
    m_bInitialized = true;

    Msg("Server Rules System initialized - Mode: %s\n", GetGameModeName(m_CurrentGameMode));
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown server rules system
//-----------------------------------------------------------------------------
void CGModServerRules::Shutdown()
{
    if (!m_bInitialized)
        return;

    m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Set game mode
//-----------------------------------------------------------------------------
void CGModServerRules::SetGameMode(GMod_GameMode_t mode)
{
    if (mode >= GAMEMODE_COUNT || mode < 0)
        mode = GAMEMODE_SANDBOX;

    m_CurrentGameMode = mode;
    gm_sv_gamemode.SetValue((int)mode);

    ApplyGameModeRules();
    UpdateGameModeSettings();

    BroadcastMessage(UTIL_VarArgs("Game mode changed to: %s", GetGameModeName(mode)));
}

//-----------------------------------------------------------------------------
// Purpose: Get current game mode
//-----------------------------------------------------------------------------
GMod_GameMode_t CGModServerRules::GetGameMode()
{
    return m_CurrentGameMode;
}

//-----------------------------------------------------------------------------
// Purpose: Get game mode name
//-----------------------------------------------------------------------------
const char* CGModServerRules::GetGameModeName(GMod_GameMode_t mode)
{
    switch (mode)
    {
        case GAMEMODE_SANDBOX: return "Sandbox";
        case GAMEMODE_BUILD: return "Build";
        case GAMEMODE_ROLEPLAY: return "Roleplay";
        case GAMEMODE_DEATHMATCH: return "Deathmatch";
        case GAMEMODE_TEAMPLAY: return "Team Play";
        case GAMEMODE_CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

//-----------------------------------------------------------------------------
// Purpose: Check if player can spawn
//-----------------------------------------------------------------------------
bool CGModServerRules::CanPlayerSpawn(CBasePlayer* pPlayer)
{
    if (!pPlayer || !m_bInitialized)
        return true;

    // Different rules for different game modes
    switch (m_CurrentGameMode)
    {
        case GAMEMODE_DEATHMATCH:
        case GAMEMODE_TEAMPLAY:
            // Check respawn timer
            if (pPlayer->GetDeathTime() + gm_sv_respawntime.GetFloat() > gpGlobals->curtime)
                return false;
            break;

        default:
            break;
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Check if player can respawn
//-----------------------------------------------------------------------------
bool CGModServerRules::CanPlayerRespawn(CBasePlayer* pPlayer)
{
    return CanPlayerSpawn(pPlayer);
}

//-----------------------------------------------------------------------------
// Purpose: Handle player connection
//-----------------------------------------------------------------------------
void CGModServerRules::HandlePlayerConnect(CBasePlayer* pPlayer)
{
    if (!pPlayer || !m_bInitialized)
        return;

    // Send game mode info to player
    ClientPrint(pPlayer, HUD_PRINTTALK, "Server Mode: %s", GetGameModeName(m_CurrentGameMode));

    // Apply player-specific rules based on game mode
    ApplyGameModeRules();
}

//-----------------------------------------------------------------------------
// Purpose: Handle player disconnection
//-----------------------------------------------------------------------------
void CGModServerRules::HandlePlayerDisconnect(CBasePlayer* pPlayer)
{
    if (!pPlayer || !m_bInitialized)
        return;

    // Cleanup player-owned entities if needed
    ValidatePlayerStates();
}

//-----------------------------------------------------------------------------
// Purpose: Check if NPC can be spawned
//-----------------------------------------------------------------------------
bool CGModServerRules::CanSpawnNPC(CBasePlayer* pPlayer, const char* npcClass)
{
    if (!pPlayer || !npcClass || !m_bInitialized)
        return false;

    if (!gm_sv_allownpc.GetBool())
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "NPC spawning is disabled");
        return false;
    }

    // Check NPC count limits
    int npcCount = 0;
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "npc_*")) != NULL)
    {
        npcCount++;
    }

    if (npcCount >= gm_sv_maxnpcs.GetInt())
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Maximum NPCs reached (%d/%d)", npcCount, gm_sv_maxnpcs.GetInt());
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Modify NPC health based on multiplier
//-----------------------------------------------------------------------------
void CGModServerRules::ModifyNPCHealth(CAI_BaseNPC* pNPC)
{
    if (!pNPC || !m_bInitialized)
        return;

    float multiplier = gm_sv_npchealthmultiplier.GetFloat();
    if (multiplier != 1.0f)
    {
        int newHealth = (int)(pNPC->GetMaxHealth() * multiplier);
        pNPC->SetMaxHealth(newHealth);
        pNPC->SetHealth(newHealth);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle NPC spawn
//-----------------------------------------------------------------------------
void CGModServerRules::HandleNPCSpawn(CAI_BaseNPC* pNPC, CBasePlayer* pOwner)
{
    if (!pNPC || !m_bInitialized)
        return;

    ModifyNPCHealth(pNPC);
}

//-----------------------------------------------------------------------------
// Purpose: Check if entity can be spawned
//-----------------------------------------------------------------------------
bool CGModServerRules::CanSpawnEntity(CBasePlayer* pPlayer, const char* entityClass)
{
    if (!pPlayer || !entityClass || !m_bInitialized)
        return false;

    // Check specific entity restrictions based on game mode
    switch (m_CurrentGameMode)
    {
        case GAMEMODE_BUILD:
            // Only allow building-related entities
            if (Q_stristr(entityClass, "weapon_") && !gm_sv_allowweapons.GetBool())
                return false;
            break;

        case GAMEMODE_DEATHMATCH:
        case GAMEMODE_TEAMPLAY:
            // Restrict building items in combat modes
            if (Q_stristr(entityClass, "prop_") && Q_stristr(entityClass, "static"))
                return false;
            break;

        default:
            break;
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Handle entity spawn
//-----------------------------------------------------------------------------
void CGModServerRules::HandleEntitySpawn(CBaseEntity* pEntity, CBasePlayer* pOwner)
{
    if (!pEntity || !m_bInitialized)
        return;

    // Track entity ownership
    if (pOwner && pEntity)
    {
        pEntity->SetOwnerEntity(pOwner);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Check weapon pickup permissions
//-----------------------------------------------------------------------------
bool CGModServerRules::CanPlayerPickupWeapon(CBasePlayer* pPlayer, CBaseCombatWeapon* pWeapon)
{
    if (!pPlayer || !pWeapon || !m_bInitialized)
        return true;

    if (!gm_sv_allowweapons.GetBool())
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Check weapon drop permissions
//-----------------------------------------------------------------------------
bool CGModServerRules::CanPlayerDropWeapon(CBasePlayer* pPlayer, CBaseCombatWeapon* pWeapon)
{
    if (!pPlayer || !pWeapon || !m_bInitialized)
        return true;

    return gm_sv_allowweapons.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: Check physics gun grab permissions
//-----------------------------------------------------------------------------
bool CGModServerRules::CanGrabEntity(CBasePlayer* pPlayer, CBaseEntity* pEntity)
{
    if (!pPlayer || !pEntity || !m_bInitialized)
        return true;

    if (!gm_sv_allowphysgun.GetBool())
        return false;

    // Check if entity is owned by another player in team mode
    if (gm_sv_teamplay.GetBool() && pEntity->GetOwnerEntity())
    {
        CBasePlayer* pOwner = ToBasePlayer(pEntity->GetOwnerEntity());
        if (pOwner && pOwner != pPlayer && pOwner->GetTeamNumber() != pPlayer->GetTeamNumber())
            return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Check weld permissions
//-----------------------------------------------------------------------------
bool CGModServerRules::CanWeldEntity(CBasePlayer* pPlayer, CBaseEntity* pEntity)
{
    if (!pPlayer || !pEntity || !m_bInitialized)
        return true;

    return gm_sv_allowtoolgun.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: Check chat permissions
//-----------------------------------------------------------------------------
bool CGModServerRules::CanPlayerChat(CBasePlayer* pPlayer)
{
    if (!pPlayer || !m_bInitialized)
        return true;

    return gm_sv_allowchat.GetBool();
}

//-----------------------------------------------------------------------------
// Purpose: Filter chat messages
//-----------------------------------------------------------------------------
void CGModServerRules::FilterChatMessage(CBasePlayer* pPlayer, char* message, int maxlen)
{
    if (!pPlayer || !message || !m_bInitialized)
        return;

    // Basic chat filtering can be added here
}

//-----------------------------------------------------------------------------
// Purpose: Apply game mode specific rules
//-----------------------------------------------------------------------------
void CGModServerRules::ApplyGameModeRules()
{
    if (!m_bInitialized)
        return;

    // Apply physics settings
    ConVar* sv_gravity = cvar->FindVar("sv_gravity");
    if (sv_gravity)
        sv_gravity->SetValue(gm_sv_gravity.GetInt());

    ConVar* sv_airaccelerate = cvar->FindVar("sv_airaccelerate");
    if (sv_airaccelerate)
        sv_airaccelerate->SetValue(gm_sv_airaccelerate.GetInt());

    ConVar* sv_friction = cvar->FindVar("sv_friction");
    if (sv_friction)
        sv_friction->SetValue(gm_sv_friction.GetInt());

    // Apply team play settings
    ConVar* mp_teamplay = cvar->FindVar("mp_teamplay");
    if (mp_teamplay)
        mp_teamplay->SetValue(gm_sv_teamplay.GetBool() ? 1 : 0);

    ConVar* mp_friendlyfire = cvar->FindVar("mp_friendlyfire");
    if (mp_friendlyfire)
        mp_friendlyfire->SetValue(gm_sv_friendly_fire.GetBool() ? 1 : 0);
}

//-----------------------------------------------------------------------------
// Purpose: Update game mode settings
//-----------------------------------------------------------------------------
void CGModServerRules::UpdateGameModeSettings()
{
    m_CurrentGameMode = (GMod_GameMode_t)gm_sv_gamemode.GetInt();
    if (m_CurrentGameMode >= GAMEMODE_COUNT || m_CurrentGameMode < 0)
        m_CurrentGameMode = GAMEMODE_SANDBOX;
}

//-----------------------------------------------------------------------------
// Purpose: Reset all player states
//-----------------------------------------------------------------------------
void CGModServerRules::ResetPlayerStates()
{
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
        if (pPlayer && pPlayer->IsConnected())
        {
            // Reset player states based on current game mode
            if (gm_sv_god.GetBool())
                pPlayer->SetFlags(pPlayer->GetFlags() | FL_GODMODE);
            else
                pPlayer->SetFlags(pPlayer->GetFlags() & ~FL_GODMODE);

            if (gm_sv_noclip.GetBool())
                pPlayer->SetMoveType(MOVETYPE_NOCLIP);
            else if (pPlayer->GetMoveType() == MOVETYPE_NOCLIP)
                pPlayer->SetMoveType(MOVETYPE_WALK);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Validate player states
//-----------------------------------------------------------------------------
void CGModServerRules::ValidatePlayerStates()
{
    // Clean up orphaned entities periodically
    if (gpGlobals->curtime > m_flLastCleanupTime + 30.0f)
    {
        m_flLastCleanupTime = gpGlobals->curtime;
        // Cleanup logic would go here
    }
}

//-----------------------------------------------------------------------------
// Purpose: Broadcast message to all players
//-----------------------------------------------------------------------------
void CGModServerRules::BroadcastMessage(const char* message)
{
    if (!message)
        return;

    UTIL_ClientPrintAll(HUD_PRINTTALK, message);
}

//-----------------------------------------------------------------------------
// Purpose: Get command client player
//-----------------------------------------------------------------------------
CBasePlayer* CGModServerRules::GetCommandPlayer()
{
    return UTIL_GetCommandClient();
}

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Set game mode
//-----------------------------------------------------------------------------
void CGModServerRules::CMD_gm_gamemode(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CCommand args;
    if (args.ArgC() < 2)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Current game mode: %s (%d)",
                   GetGameModeName(m_CurrentGameMode), (int)m_CurrentGameMode);
        ClientPrint(pPlayer, HUD_PRINTTALK, "Usage: gm_gamemode <0-5> (0=Sandbox, 1=Build, 2=RP, 3=DM, 4=Team, 5=Custom)");
        return;
    }

    int mode = atoi(args.Arg(1));
    SetGameMode((GMod_GameMode_t)mode);
}

//-----------------------------------------------------------------------------
// Purpose: Restart game/round
//-----------------------------------------------------------------------------
void CGModServerRules::CMD_gm_restart(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    BroadcastMessage("Game restarting...");

    // Reset all players
    ResetPlayerStates();

    // Clean up entities
    engine->ServerCommand("mp_restartgame 1\n");
}

//-----------------------------------------------------------------------------
// Purpose: Cleanup entities
//-----------------------------------------------------------------------------
void CGModServerRules::CMD_gm_cleanup(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    int count = 0;

    // Remove props
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "prop_*")) != NULL)
    {
        if (pEntity->GetOwnerEntity() != pPlayer)
            continue;
        UTIL_Remove(pEntity);
        count++;
    }

    ClientPrint(pPlayer, HUD_PRINTTALK, "Cleaned up %d entities", count);
}

//-----------------------------------------------------------------------------
// Purpose: Toggle noclip for all players
//-----------------------------------------------------------------------------
void CGModServerRules::CMD_gm_noclip_all(void)
{
    gm_sv_noclip.SetValue(!gm_sv_noclip.GetBool());
    ResetPlayerStates();
    BroadcastMessage(gm_sv_noclip.GetBool() ? "Noclip enabled for all players" : "Noclip disabled for all players");
}

//-----------------------------------------------------------------------------
// Purpose: Toggle god mode for all players
//-----------------------------------------------------------------------------
void CGModServerRules::CMD_gm_god_all(void)
{
    gm_sv_god.SetValue(!gm_sv_god.GetBool());
    ResetPlayerStates();
    BroadcastMessage(gm_sv_god.GetBool() ? "God mode enabled for all players" : "God mode disabled for all players");
}

//-----------------------------------------------------------------------------
// Purpose: Freeze all players
//-----------------------------------------------------------------------------
void CGModServerRules::CMD_gm_freeze_all(void)
{
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
        if (pPlayer && pPlayer->IsConnected())
        {
            pPlayer->SetFlags(pPlayer->GetFlags() | FL_FROZEN);
        }
    }
    BroadcastMessage("All players frozen");
}

//-----------------------------------------------------------------------------
// Purpose: Unfreeze all players
//-----------------------------------------------------------------------------
void CGModServerRules::CMD_gm_unfreeze_all(void)
{
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
        if (pPlayer && pPlayer->IsConnected())
        {
            pPlayer->SetFlags(pPlayer->GetFlags() & ~FL_FROZEN);
        }
    }
    BroadcastMessage("All players unfrozen");
}

// Register console commands
static ConCommand cmd_gm_gamemode("gm_gamemode", CGModServerRules::CMD_gm_gamemode, "Set or display game mode");
static ConCommand cmd_gm_restart("gm_restart", CGModServerRules::CMD_gm_restart, "Restart game/round");
static ConCommand cmd_gm_cleanup("gm_cleanup", CGModServerRules::CMD_gm_cleanup, "Cleanup your entities");
static ConCommand cmd_gm_noclip_all("gm_noclip_all", CGModServerRules::CMD_gm_noclip_all, "Toggle noclip for all players");
static ConCommand cmd_gm_god_all("gm_god_all", CGModServerRules::CMD_gm_god_all, "Toggle god mode for all players");
static ConCommand cmd_gm_freeze_all("gm_freeze_all", CGModServerRules::CMD_gm_freeze_all, "Freeze all players");
static ConCommand cmd_gm_unfreeze_all("gm_unfreeze_all", CGModServerRules::CMD_gm_unfreeze_all, "Unfreeze all players");

//-----------------------------------------------------------------------------
// System initialization hook
//-----------------------------------------------------------------------------
class CServerRulesInit : public CAutoGameSystem
{
public:
    CServerRulesInit() : CAutoGameSystem("ServerRulesInit") {}

    virtual bool Init()
    {
        CGModServerRules::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModServerRules::Shutdown();
    }

    virtual void FrameUpdatePostEntityThink()
    {
        // Update server rules periodically
        CGModServerRules::ValidatePlayerStates();
    }

    virtual void LevelInitPostEntity()
    {
        // Apply game mode rules when level loads
        CGModServerRules::ApplyGameModeRules();
    }
};

static CServerRulesInit g_ServerRulesInit;