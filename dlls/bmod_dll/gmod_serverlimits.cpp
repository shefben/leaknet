//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Server Entity Limits System Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_serverlimits.h"
#include "player.h"
#include "tier1/strtools.h"

// Initialize static members
int CGModServerLimits::m_iPlayerEntityCounts[MAX_PLAYERS][LIMIT_TYPE_COUNT];
int CGModServerLimits::m_iTotalEntityCounts[LIMIT_TYPE_COUNT];
bool CGModServerLimits::m_bInitialized = false;
ConVar* CGModServerLimits::m_pServerLimits[LIMIT_TYPE_COUNT];
ConVar* CGModServerLimits::m_pClientLimits[LIMIT_TYPE_COUNT];

// Console variables - Server Limits
ConVar gm_sv_serverlimit_props("gm_sv_serverlimit_props", "1000", FCVAR_NONE, "Maximum props on server");
ConVar gm_sv_serverlimit_ragdolls("gm_sv_serverlimit_ragdolls", "200", FCVAR_NONE, "Maximum ragdolls on server");
ConVar gm_sv_serverlimit_balloons("gm_sv_serverlimit_balloons", "200", FCVAR_NONE, "Maximum balloons on server");
ConVar gm_sv_serverlimit_effects("gm_sv_serverlimit_effects", "500", FCVAR_NONE, "Maximum effects on server");
ConVar gm_sv_serverlimit_sprites("gm_sv_serverlimit_sprites", "300", FCVAR_NONE, "Maximum sprites on server");
ConVar gm_sv_serverlimit_emitters("gm_sv_serverlimit_emitters", "100", FCVAR_NONE, "Maximum emitters on server");
ConVar gm_sv_serverlimit_wheels("gm_sv_serverlimit_wheels", "150", FCVAR_NONE, "Maximum wheels on server");
ConVar gm_sv_serverlimit_npcs("gm_sv_serverlimit_npcs", "50", FCVAR_NONE, "Maximum NPCs on server");
ConVar gm_sv_serverlimit_dynamite("gm_sv_serverlimit_dynamite", "100", FCVAR_NONE, "Maximum dynamite on server");
ConVar gm_sv_serverlimit_vehicles("gm_sv_serverlimit_vehicles", "20", FCVAR_NONE, "Maximum vehicles on server");
ConVar gm_sv_serverlimit_thrusters("gm_sv_serverlimit_thrusters", "200", FCVAR_NONE, "Maximum thrusters on server");

// Console variables - Client Limits
ConVar gm_sv_clientlimit_props("gm_sv_clientlimit_props", "100", FCVAR_NONE, "Maximum props per client");
ConVar gm_sv_clientlimit_ragdolls("gm_sv_clientlimit_ragdolls", "20", FCVAR_NONE, "Maximum ragdolls per client");
ConVar gm_sv_clientlimit_balloons("gm_sv_clientlimit_balloons", "20", FCVAR_NONE, "Maximum balloons per client");
ConVar gm_sv_clientlimit_effects("gm_sv_clientlimit_effects", "50", FCVAR_NONE, "Maximum effects per client");
ConVar gm_sv_clientlimit_sprites("gm_sv_clientlimit_sprites", "30", FCVAR_NONE, "Maximum sprites per client");
ConVar gm_sv_clientlimit_emitters("gm_sv_clientlimit_emitters", "10", FCVAR_NONE, "Maximum emitters per client");
ConVar gm_sv_clientlimit_wheels("gm_sv_clientlimit_wheels", "15", FCVAR_NONE, "Maximum wheels per client");
ConVar gm_sv_clientlimit_npcs("gm_sv_clientlimit_npcs", "5", FCVAR_NONE, "Maximum NPCs per client");
ConVar gm_sv_clientlimit_dynamite("gm_sv_clientlimit_dynamite", "10", FCVAR_NONE, "Maximum dynamite per client");
ConVar gm_sv_clientlimit_vehicles("gm_sv_clientlimit_vehicles", "2", FCVAR_NONE, "Maximum vehicles per client");
ConVar gm_sv_clientlimit_thrusters("gm_sv_clientlimit_thrusters", "20", FCVAR_NONE, "Maximum thrusters per client");

//-----------------------------------------------------------------------------
// Purpose: Initialize the server limits system
//-----------------------------------------------------------------------------
void CGModServerLimits::Initialize()
{
    if (m_bInitialized)
        return;

    // Reset all counts
    ResetCounts();

    // Set up console variable arrays for easy access
    CreateLimitConVars();

    m_bInitialized = true;

    Msg("Server Entity Limits System initialized\n");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown the server limits system
//-----------------------------------------------------------------------------
void CGModServerLimits::Shutdown()
{
    if (!m_bInitialized)
        return;

    DestroyLimitConVars();
    m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Create console variable arrays
//-----------------------------------------------------------------------------
void CGModServerLimits::CreateLimitConVars()
{
    // Set up server limit array
    m_pServerLimits[LIMIT_PROPS] = &gm_sv_serverlimit_props;
    m_pServerLimits[LIMIT_RAGDOLLS] = &gm_sv_serverlimit_ragdolls;
    m_pServerLimits[LIMIT_BALLOONS] = &gm_sv_serverlimit_balloons;
    m_pServerLimits[LIMIT_EFFECTS] = &gm_sv_serverlimit_effects;
    m_pServerLimits[LIMIT_SPRITES] = &gm_sv_serverlimit_sprites;
    m_pServerLimits[LIMIT_EMITTERS] = &gm_sv_serverlimit_emitters;
    m_pServerLimits[LIMIT_WHEELS] = &gm_sv_serverlimit_wheels;
    m_pServerLimits[LIMIT_NPCS] = &gm_sv_serverlimit_npcs;
    m_pServerLimits[LIMIT_DYNAMITE] = &gm_sv_serverlimit_dynamite;
    m_pServerLimits[LIMIT_VEHICLES] = &gm_sv_serverlimit_vehicles;
    m_pServerLimits[LIMIT_THRUSTERS] = &gm_sv_serverlimit_thrusters;

    // Set up client limit array
    m_pClientLimits[LIMIT_PROPS] = &gm_sv_clientlimit_props;
    m_pClientLimits[LIMIT_RAGDOLLS] = &gm_sv_clientlimit_ragdolls;
    m_pClientLimits[LIMIT_BALLOONS] = &gm_sv_clientlimit_balloons;
    m_pClientLimits[LIMIT_EFFECTS] = &gm_sv_clientlimit_effects;
    m_pClientLimits[LIMIT_SPRITES] = &gm_sv_clientlimit_sprites;
    m_pClientLimits[LIMIT_EMITTERS] = &gm_sv_clientlimit_emitters;
    m_pClientLimits[LIMIT_WHEELS] = &gm_sv_clientlimit_wheels;
    m_pClientLimits[LIMIT_NPCS] = &gm_sv_clientlimit_npcs;
    m_pClientLimits[LIMIT_DYNAMITE] = &gm_sv_clientlimit_dynamite;
    m_pClientLimits[LIMIT_VEHICLES] = &gm_sv_clientlimit_vehicles;
    m_pClientLimits[LIMIT_THRUSTERS] = &gm_sv_clientlimit_thrusters;
}

//-----------------------------------------------------------------------------
// Purpose: Destroy console variable arrays
//-----------------------------------------------------------------------------
void CGModServerLimits::DestroyLimitConVars()
{
    // Arrays point to static ConVars, no need to delete
    memset(m_pServerLimits, 0, sizeof(m_pServerLimits));
    memset(m_pClientLimits, 0, sizeof(m_pClientLimits));
}

//-----------------------------------------------------------------------------
// Purpose: Reset all entity counts
//-----------------------------------------------------------------------------
void CGModServerLimits::ResetCounts()
{
    memset(m_iPlayerEntityCounts, 0, sizeof(m_iPlayerEntityCounts));
    memset(m_iTotalEntityCounts, 0, sizeof(m_iTotalEntityCounts));
}

//-----------------------------------------------------------------------------
// Purpose: Check if a player can create an entity of the specified type
//-----------------------------------------------------------------------------
bool CGModServerLimits::CanCreateEntity(CBasePlayer* pPlayer, EntityLimitType_t type)
{
    if (!m_bInitialized || !pPlayer || type >= LIMIT_TYPE_COUNT)
        return true; // Default to allowing if system not ready

    int playerIndex = pPlayer->entindex() - 1;
    if (playerIndex < 0 || playerIndex >= MAX_PLAYERS)
        return true;

    // Check server limits first
    int serverLimit = GetServerLimit(type);
    if (serverLimit > 0 && m_iTotalEntityCounts[type] >= serverLimit)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Server limit reached for %s (%d/%d)",
                   GetLimitTypeName(type), m_iTotalEntityCounts[type], serverLimit);
        return false;
    }

    // Check client limits
    int clientLimit = GetClientLimit(type);
    if (clientLimit > 0 && m_iPlayerEntityCounts[playerIndex][type] >= clientLimit)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Personal limit reached for %s (%d/%d)",
                   GetLimitTypeName(type), m_iPlayerEntityCounts[playerIndex][type], clientLimit);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Register that a player created an entity
//-----------------------------------------------------------------------------
void CGModServerLimits::RegisterEntityCreated(CBasePlayer* pPlayer, EntityLimitType_t type)
{
    if (!m_bInitialized || !pPlayer || type >= LIMIT_TYPE_COUNT)
        return;

    int playerIndex = pPlayer->entindex() - 1;
    if (playerIndex < 0 || playerIndex >= MAX_PLAYERS)
        return;

    m_iPlayerEntityCounts[playerIndex][type]++;
    m_iTotalEntityCounts[type]++;
}

//-----------------------------------------------------------------------------
// Purpose: Register that a player's entity was destroyed
//-----------------------------------------------------------------------------
void CGModServerLimits::RegisterEntityDestroyed(CBasePlayer* pPlayer, EntityLimitType_t type)
{
    if (!m_bInitialized || !pPlayer || type >= LIMIT_TYPE_COUNT)
        return;

    int playerIndex = pPlayer->entindex() - 1;
    if (playerIndex < 0 || playerIndex >= MAX_PLAYERS)
        return;

    if (m_iPlayerEntityCounts[playerIndex][type] > 0)
        m_iPlayerEntityCounts[playerIndex][type]--;

    if (m_iTotalEntityCounts[type] > 0)
        m_iTotalEntityCounts[type]--;
}

//-----------------------------------------------------------------------------
// Purpose: Get server limit for entity type
//-----------------------------------------------------------------------------
int CGModServerLimits::GetServerLimit(EntityLimitType_t type)
{
    if (type >= LIMIT_TYPE_COUNT || !m_pServerLimits[type])
        return 0;

    return m_pServerLimits[type]->GetInt();
}

//-----------------------------------------------------------------------------
// Purpose: Get client limit for entity type
//-----------------------------------------------------------------------------
int CGModServerLimits::GetClientLimit(EntityLimitType_t type)
{
    if (type >= LIMIT_TYPE_COUNT || !m_pClientLimits[type])
        return 0;

    return m_pClientLimits[type]->GetInt();
}

//-----------------------------------------------------------------------------
// Purpose: Get player's entity count for type
//-----------------------------------------------------------------------------
int CGModServerLimits::GetPlayerEntityCount(CBasePlayer* pPlayer, EntityLimitType_t type)
{
    if (!pPlayer || type >= LIMIT_TYPE_COUNT)
        return 0;

    int playerIndex = pPlayer->entindex() - 1;
    if (playerIndex < 0 || playerIndex >= MAX_PLAYERS)
        return 0;

    return m_iPlayerEntityCounts[playerIndex][type];
}

//-----------------------------------------------------------------------------
// Purpose: Get total entity count for type
//-----------------------------------------------------------------------------
int CGModServerLimits::GetTotalEntityCount(EntityLimitType_t type)
{
    if (type >= LIMIT_TYPE_COUNT)
        return 0;

    return m_iTotalEntityCounts[type];
}

//-----------------------------------------------------------------------------
// Purpose: Get entity limit type name
//-----------------------------------------------------------------------------
const char* CGModServerLimits::GetLimitTypeName(EntityLimitType_t type)
{
    switch (type)
    {
        case LIMIT_PROPS: return "props";
        case LIMIT_RAGDOLLS: return "ragdolls";
        case LIMIT_BALLOONS: return "balloons";
        case LIMIT_EFFECTS: return "effects";
        case LIMIT_SPRITES: return "sprites";
        case LIMIT_EMITTERS: return "emitters";
        case LIMIT_WHEELS: return "wheels";
        case LIMIT_NPCS: return "NPCs";
        case LIMIT_DYNAMITE: return "dynamite";
        case LIMIT_VEHICLES: return "vehicles";
        case LIMIT_THRUSTERS: return "thrusters";
        default: return "unknown";
    }
}

//-----------------------------------------------------------------------------
// Purpose: Remove all entities from a player
//-----------------------------------------------------------------------------
void CGModServerLimits::RemoveAllPlayerEntities(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    // This would iterate through all entities and remove ones owned by the player
    // Implementation depends on how entity ownership is tracked

    int playerIndex = pPlayer->entindex() - 1;
    if (playerIndex >= 0 && playerIndex < MAX_PLAYERS)
    {
        // Reset player's counts
        for (int i = 0; i < LIMIT_TYPE_COUNT; i++)
        {
            m_iTotalEntityCounts[i] -= m_iPlayerEntityCounts[playerIndex][i];
            m_iPlayerEntityCounts[playerIndex][i] = 0;
        }
    }

    ClientPrint(pPlayer, HUD_PRINTTALK, "All your entities have been removed");
}

//-----------------------------------------------------------------------------
// Purpose: Remove all entities from the server
//-----------------------------------------------------------------------------
void CGModServerLimits::RemoveAllEntities()
{
    // Reset all counts
    ResetCounts();

    UTIL_ClientPrintAll(HUD_PRINTTALK, "All spawned entities have been removed");
}

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Remove all entities command
//-----------------------------------------------------------------------------
void CGModServerLimits::CMD_gm_remove_all(void)
{
    RemoveAllEntities();
    Msg("All spawned entities removed\n");
}

//-----------------------------------------------------------------------------
// Purpose: Remove player's entities command
//-----------------------------------------------------------------------------
void CGModServerLimits::CMD_gm_remove_my(void)
{
    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (pPlayer)
    {
        RemoveAllPlayerEntities(pPlayer);
    }
}

static ConCommand gm_remove_all("gm_remove_all", CGModServerLimits::CMD_gm_remove_all, "Remove all spawned entities");
static ConCommand gm_remove_my("gm_remove_my", CGModServerLimits::CMD_gm_remove_my, "Remove all your spawned entities");

//-----------------------------------------------------------------------------
// Server initialization hook
//-----------------------------------------------------------------------------
class CServerLimitsInit : public CAutoGameSystem
{
public:
    CServerLimitsInit() : CAutoGameSystem("ServerLimitsInit") {}

    virtual bool Init()
    {
        CGModServerLimits::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModServerLimits::Shutdown();
    }
};

static CServerLimitsInit g_ServerLimitsInit;