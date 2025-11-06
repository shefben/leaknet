//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Emitter/Particle System - GMod 9.0.4b compatible implementation
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_EMITTER_H
#define GMOD_EMITTER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CBasePlayer;

//-----------------------------------------------------------------------------
// Emitter Types
//-----------------------------------------------------------------------------
enum EmitterType_t
{
    EMITTER_FIRE = 0,
    EMITTER_SMOKE,
    EMITTER_STEAM,
    EMITTER_SPARKS,
    EMITTER_DUST,
    EMITTER_BLOOD,
    EMITTER_WATER,
    EMITTER_CUSTOM,

    EMITTER_TYPE_COUNT
};

//-----------------------------------------------------------------------------
// Particle Emitter Entity
//-----------------------------------------------------------------------------
class CPropEmitter : public CBaseEntity
{
    DECLARE_CLASS(CPropEmitter, CBaseEntity);
    DECLARE_DATADESC();

public:
    void Spawn();
    void Precache();
    void Think();
    void UpdateOnRemove();

    // Emitter controls
    void StartEmitting();
    void StopEmitting();
    void SetEmitterType(EmitterType_t type);
    void SetEmitRate(float rate);
    void SetParticleSize(float size);
    void SetParticleLifetime(float lifetime);
    void SetEmitDirection(const Vector& direction);
    void SetEmitSpeed(float speed);

    // Input functions
    void InputStart(inputdata_t &inputdata);
    void InputStop(inputdata_t &inputdata);
    void InputToggle(inputdata_t &inputdata);
    void InputSetType(inputdata_t &inputdata);
    void InputSetRate(inputdata_t &inputdata);

    // Factory function
    static CPropEmitter* CreateEmitter(const Vector& position, const QAngle& angles,
                                      EmitterType_t type, CBasePlayer* pOwner);

private:
    EmitterType_t m_EmitterType;
    bool m_bEmitting;
    float m_flEmitRate;
    float m_flParticleSize;
    float m_flParticleLifetime;
    float m_flEmitSpeed;
    Vector m_vecEmitDirection;
    float m_flNextEmitTime;

    void InitEmitter();
    void EmitParticles();
    void CreateParticleEffect();
    const char* GetEffectName();
};

//-----------------------------------------------------------------------------
// Emitter Management System
//-----------------------------------------------------------------------------
class CGModEmitterSystem
{
public:
    // System management
    static void Initialize();
    static void Shutdown();

    // Emitter creation
    static bool CanCreateEmitter(CBasePlayer* pPlayer);
    static CPropEmitter* CreatePlayerEmitter(CBasePlayer* pPlayer, const Vector& position,
                                           const QAngle& angles, EmitterType_t type);

    // Emitter management
    static void UpdateAllEmitters();
    static void CleanupEmitters();
    static int GetEmitterCount();
    static int GetPlayerEmitterCount(CBasePlayer* pPlayer);

    // Type utilities
    static const char* GetEmitterTypeName(EmitterType_t type);
    static EmitterType_t GetEmitterTypeFromName(const char* name);

    // Console command handlers
    static void CMD_gm_emitter_spawn(void);
    static void CMD_gm_emitter_remove(void);
    static void CMD_gm_emitter_clear(void);
    static void CMD_gm_emitter_start_all(void);
    static void CMD_gm_emitter_stop_all(void);

private:
    static bool m_bInitialized;
    static float m_flPlayerDelays[MAX_PLAYERS];
    static float m_flLastSpawnTimes[MAX_PLAYERS];

    static CBasePlayer* GetCommandPlayer();
    static int GetPlayerIndex(CBasePlayer* pPlayer);
    static bool CheckPlayerDelay(CBasePlayer* pPlayer);
    static void UpdatePlayerDelay(CBasePlayer* pPlayer);
};

// Console variables for emitter system
extern ConVar gm_emitter_type;
extern ConVar gm_emitter_delay;
extern ConVar gm_emitter_rate;
extern ConVar gm_emitter_size;
extern ConVar gm_emitter_lifetime;
extern ConVar gm_emitter_speed;
extern ConVar gm_emitter_enabled;

#endif // GMOD_EMITTER_H