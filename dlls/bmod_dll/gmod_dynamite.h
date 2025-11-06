//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Dynamite System - GMod 9.0.4b compatible implementation
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_DYNAMITE_H
#define GMOD_DYNAMITE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CBasePlayer;

//-----------------------------------------------------------------------------
// Dynamite Entity Class
//-----------------------------------------------------------------------------
class CGModDynamite : public CBaseEntity
{
    DECLARE_CLASS(CGModDynamite, CBaseEntity);
    DECLARE_DATADESC();

public:
    void Spawn();
    void Precache();
    void Think();
    void Touch(CBaseEntity *pOther);
    void Explode();
    void SetTimer(float flTime);
    void SetPower(float flPower);

    static CGModDynamite* CreateDynamite(const Vector& position, const QAngle& angles, CBasePlayer* pOwner);

private:
    float m_flExplodeTime;
    float m_flExplosionPower;
    bool m_bArmed;

    void InitDynamite();
    void PlayTickSound();
};

//-----------------------------------------------------------------------------
// Dynamite Management System
//-----------------------------------------------------------------------------
class CGModDynamiteSystem
{
public:
    // System management
    static void Initialize();
    static void Shutdown();

    // Dynamite creation with delay management
    static bool CanCreateDynamite(CBasePlayer* pPlayer);
    static CGModDynamite* CreatePlayerDynamite(CBasePlayer* pPlayer, const Vector& position, const QAngle& angles);

    // Delay system management
    static void UpdatePlayerDelay(CBasePlayer* pPlayer);
    static float GetPlayerDelay(CBasePlayer* pPlayer);
    static void ResetPlayerDelay(CBasePlayer* pPlayer);

    // Console command handlers
    static void CMD_gm_dynamite_spawn(void);
    static void CMD_gm_dynamite_explode_all(void);
    static void CMD_gm_dynamite_clear(void);

private:
    static bool m_bInitialized;
    static float m_flPlayerDelays[MAX_PLAYERS];
    static float m_flLastSpawnTimes[MAX_PLAYERS];

    static CBasePlayer* GetCommandPlayer();
    static int GetPlayerIndex(CBasePlayer* pPlayer);
    static void CleanupDynamite();
};

// Console variables for dynamite system
extern ConVar gm_dynamite_delay;
extern ConVar gm_dynamite_delay_add;
extern ConVar gm_dynamite_power;
extern ConVar gm_dynamite_timer;
extern ConVar gm_dynamite_sound;

#endif // GMOD_DYNAMITE_H