//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Server Entity Limits System - GMod 9.0.4b compatible
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_SERVERLIMITS_H
#define GMOD_SERVERLIMITS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

//-----------------------------------------------------------------------------
// Entity Types for Limit Tracking
//-----------------------------------------------------------------------------
enum EntityLimitType_t
{
    LIMIT_PROPS = 0,
    LIMIT_RAGDOLLS,
    LIMIT_BALLOONS,
    LIMIT_EFFECTS,
    LIMIT_SPRITES,
    LIMIT_EMITTERS,
    LIMIT_WHEELS,
    LIMIT_NPCS,
    LIMIT_DYNAMITE,
    LIMIT_VEHICLES,
    LIMIT_THRUSTERS,

    LIMIT_TYPE_COUNT
};

//-----------------------------------------------------------------------------
// Server Limits Management System
//-----------------------------------------------------------------------------
class CGModServerLimits
{
public:
    // System initialization
    static void Initialize();
    static void Shutdown();

    // Limit checking
    static bool CanCreateEntity(CBasePlayer* pPlayer, EntityLimitType_t type);
    static void RegisterEntityCreated(CBasePlayer* pPlayer, EntityLimitType_t type);
    static void RegisterEntityDestroyed(CBasePlayer* pPlayer, EntityLimitType_t type);

    // Limit management
    static int GetServerLimit(EntityLimitType_t type);
    static int GetClientLimit(EntityLimitType_t type);
    static int GetPlayerEntityCount(CBasePlayer* pPlayer, EntityLimitType_t type);
    static int GetTotalEntityCount(EntityLimitType_t type);

    // Cleanup functions
    static void RemoveAllPlayerEntities(CBasePlayer* pPlayer);
    static void RemoveAllEntities();
    static void RemovePlayerEntitiesOfType(CBasePlayer* pPlayer, EntityLimitType_t type);

    // Console commands
    static void CMD_gm_remove_all(void);
    static void CMD_gm_remove_my(void);

    // Utility functions
    static const char* GetLimitTypeName(EntityLimitType_t type);
    static EntityLimitType_t GetEntityLimitType(CBaseEntity* pEntity);

private:
    // Player entity tracking
    static int m_iPlayerEntityCounts[MAX_PLAYERS][LIMIT_TYPE_COUNT];
    static int m_iTotalEntityCounts[LIMIT_TYPE_COUNT];
    static bool m_bInitialized;

    // Console variable references
    static ConVar* m_pServerLimits[LIMIT_TYPE_COUNT];
    static ConVar* m_pClientLimits[LIMIT_TYPE_COUNT];

    // Internal functions
    static void CreateLimitConVars();
    static void DestroyLimitConVars();
    static void ResetCounts();
};

// Console variables for server limits
extern ConVar gm_sv_serverlimit_props;
extern ConVar gm_sv_clientlimit_props;
extern ConVar gm_sv_serverlimit_ragdolls;
extern ConVar gm_sv_clientlimit_ragdolls;
extern ConVar gm_sv_serverlimit_balloons;
extern ConVar gm_sv_clientlimit_balloons;
extern ConVar gm_sv_serverlimit_effects;
extern ConVar gm_sv_clientlimit_effects;
extern ConVar gm_sv_serverlimit_sprites;
extern ConVar gm_sv_clientlimit_sprites;
extern ConVar gm_sv_serverlimit_emitters;
extern ConVar gm_sv_clientlimit_emitters;
extern ConVar gm_sv_serverlimit_wheels;
extern ConVar gm_sv_clientlimit_wheels;
extern ConVar gm_sv_serverlimit_npcs;
extern ConVar gm_sv_clientlimit_npcs;
extern ConVar gm_sv_serverlimit_dynamite;
extern ConVar gm_sv_clientlimit_dynamite;
extern ConVar gm_sv_serverlimit_vehicles;
extern ConVar gm_sv_clientlimit_vehicles;
extern ConVar gm_sv_serverlimit_thrusters;
extern ConVar gm_sv_clientlimit_thrusters;

#endif // GMOD_SERVERLIMITS_H