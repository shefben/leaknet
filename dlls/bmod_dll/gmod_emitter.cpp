//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Emitter/Particle System Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_emitter.h"
#include "player.h"
#include "tier1/strtools.h"
#include "te_effect_dispatch.h"
#include "util.h"

// Initialize static members
bool CGModEmitterSystem::m_bInitialized = false;
float CGModEmitterSystem::m_flPlayerDelays[MAX_PLAYERS];
float CGModEmitterSystem::m_flLastSpawnTimes[MAX_PLAYERS];

// Console variables for emitter system
ConVar gm_emitter_type("gm_emitter_type", "0", FCVAR_GAMEDLL, "Default emitter type (0-7)");
ConVar gm_emitter_delay("gm_emitter_delay", "2.0", FCVAR_GAMEDLL, "Delay between emitter spawns");
ConVar gm_emitter_rate("gm_emitter_rate", "10.0", FCVAR_GAMEDLL, "Default particle emit rate");
ConVar gm_emitter_size("gm_emitter_size", "1.0", FCVAR_GAMEDLL, "Default particle size");
ConVar gm_emitter_lifetime("gm_emitter_lifetime", "5.0", FCVAR_GAMEDLL, "Default particle lifetime");
ConVar gm_emitter_speed("gm_emitter_speed", "100.0", FCVAR_GAMEDLL, "Default particle speed");
ConVar gm_emitter_enabled("gm_emitter_enabled", "1", FCVAR_GAMEDLL, "Enable emitter system");

//-----------------------------------------------------------------------------
// Emitter Entity Implementation
//-----------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(prop_emitter, CPropEmitter);

BEGIN_DATADESC(CPropEmitter)
    DEFINE_FIELD(m_EmitterType, FIELD_INTEGER),
    DEFINE_FIELD(m_bEmitting, FIELD_BOOLEAN),
    DEFINE_FIELD(m_flEmitRate, FIELD_FLOAT),
    DEFINE_FIELD(m_flParticleSize, FIELD_FLOAT),
    DEFINE_FIELD(m_flParticleLifetime, FIELD_FLOAT),
    DEFINE_FIELD(m_flEmitSpeed, FIELD_FLOAT),
    DEFINE_FIELD(m_vecEmitDirection, FIELD_VECTOR),
    DEFINE_FIELD(m_flNextEmitTime, FIELD_TIME),

    DEFINE_THINKFUNC(Think),

    DEFINE_INPUTFUNC(FIELD_VOID, "Start", InputStart),
    DEFINE_INPUTFUNC(FIELD_VOID, "Stop", InputStop),
    DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
    DEFINE_INPUTFUNC(FIELD_INTEGER, "SetType", InputSetType),
    DEFINE_INPUTFUNC(FIELD_FLOAT, "SetRate", InputSetRate),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Spawn the emitter entity
//-----------------------------------------------------------------------------
void CPropEmitter::Spawn()
{
    Precache();

    SetModel("models/props_lab/clipboard.mdl");
    SetSolid(SOLID_BBOX);
    SetMoveType(MOVETYPE_VPHYSICS);

    UTIL_SetSize(this, Vector(-4, -4, -4), Vector(4, 4, 4));

    // Create physics object
    VPhysicsInitNormal(SOLID_BBOX, 0, false);

    InitEmitter();

    SetThink(&CPropEmitter::Think);
    SetNextThink(gpGlobals->curtime + 0.1f);
}

//-----------------------------------------------------------------------------
// Purpose: Precache models and effects
//-----------------------------------------------------------------------------
void CPropEmitter::Precache()
{
    PrecacheModel("models/props_lab/clipboard.mdl");

    // Precache particle effects
    PrecacheParticleSystem("env_fire_small");
    PrecacheParticleSystem("env_steam");
    PrecacheParticleSystem("env_dust_1");
    PrecacheParticleSystem("blood_impact_red_01");
}

//-----------------------------------------------------------------------------
// Purpose: Initialize emitter properties
//-----------------------------------------------------------------------------
void CPropEmitter::InitEmitter()
{
    m_EmitterType = (EmitterType_t)gm_emitter_type.GetInt();
    m_bEmitting = false;
    m_flEmitRate = gm_emitter_rate.GetFloat();
    m_flParticleSize = gm_emitter_size.GetFloat();
    m_flParticleLifetime = gm_emitter_lifetime.GetFloat();
    m_flEmitSpeed = gm_emitter_speed.GetFloat();
    m_vecEmitDirection = Vector(0, 0, 1);
    m_flNextEmitTime = 0.0f;

    // Set color based on emitter type
    switch (m_EmitterType)
    {
        case EMITTER_FIRE: SetRenderColor(255, 128, 0, 255); break;
        case EMITTER_SMOKE: SetRenderColor(128, 128, 128, 255); break;
        case EMITTER_STEAM: SetRenderColor(200, 200, 255, 255); break;
        case EMITTER_SPARKS: SetRenderColor(255, 255, 0, 255); break;
        case EMITTER_DUST: SetRenderColor(150, 120, 90, 255); break;
        case EMITTER_BLOOD: SetRenderColor(255, 0, 0, 255); break;
        case EMITTER_WATER: SetRenderColor(0, 128, 255, 255); break;
        default: SetRenderColor(255, 255, 255, 255); break;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Think function for particle emission
//-----------------------------------------------------------------------------
void CPropEmitter::Think()
{
    if (!gm_emitter_enabled.GetBool())
    {
        SetNextThink(gpGlobals->curtime + 1.0f);
        return;
    }

    if (m_bEmitting && gpGlobals->curtime >= m_flNextEmitTime)
    {
        EmitParticles();
        m_flNextEmitTime = gpGlobals->curtime + (1.0f / m_flEmitRate);
    }

    SetNextThink(gpGlobals->curtime + 0.1f);
}

//-----------------------------------------------------------------------------
// Purpose: Update on remove
//-----------------------------------------------------------------------------
void CPropEmitter::UpdateOnRemove()
{
    StopEmitting();
    BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Start emitting particles
//-----------------------------------------------------------------------------
void CPropEmitter::StartEmitting()
{
    m_bEmitting = true;
    m_flNextEmitTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Stop emitting particles
//-----------------------------------------------------------------------------
void CPropEmitter::StopEmitting()
{
    m_bEmitting = false;
}

//-----------------------------------------------------------------------------
// Purpose: Set emitter type
//-----------------------------------------------------------------------------
void CPropEmitter::SetEmitterType(EmitterType_t type)
{
    if (type >= EMITTER_TYPE_COUNT || type < 0)
        type = EMITTER_FIRE;

    m_EmitterType = type;
    InitEmitter(); // Reinitialize with new type
}

//-----------------------------------------------------------------------------
// Purpose: Set emit rate
//-----------------------------------------------------------------------------
void CPropEmitter::SetEmitRate(float rate)
{
    m_flEmitRate = clamp(rate, 0.1f, 100.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Set particle size
//-----------------------------------------------------------------------------
void CPropEmitter::SetParticleSize(float size)
{
    m_flParticleSize = clamp(size, 0.1f, 10.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Set particle lifetime
//-----------------------------------------------------------------------------
void CPropEmitter::SetParticleLifetime(float lifetime)
{
    m_flParticleLifetime = clamp(lifetime, 0.1f, 60.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Set emit direction
//-----------------------------------------------------------------------------
void CPropEmitter::SetEmitDirection(const Vector& direction)
{
    m_vecEmitDirection = direction;
    VectorNormalize(m_vecEmitDirection);
}

//-----------------------------------------------------------------------------
// Purpose: Set emit speed
//-----------------------------------------------------------------------------
void CPropEmitter::SetEmitSpeed(float speed)
{
    m_flEmitSpeed = clamp(speed, 1.0f, 1000.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Emit particles
//-----------------------------------------------------------------------------
void CPropEmitter::EmitParticles()
{
    CreateParticleEffect();
}

//-----------------------------------------------------------------------------
// Purpose: Create particle effect
//-----------------------------------------------------------------------------
void CPropEmitter::CreateParticleEffect()
{
    Vector origin = GetAbsOrigin();
    QAngle angles = GetAbsAngles();

    CEffectData data;
    data.m_vOrigin = origin;
    data.m_vAngles = angles;
    data.m_flScale = m_flParticleSize;

    switch (m_EmitterType)
    {
        case EMITTER_FIRE:
            DispatchEffect("env_fire_small", data);
            break;

        case EMITTER_SMOKE:
            data.m_nColor = 128; // Gray smoke
            DispatchEffect("smokestack", data);
            break;

        case EMITTER_STEAM:
            DispatchEffect("env_steam", data);
            break;

        case EMITTER_SPARKS:
            data.m_nMagnitude = 5;
            data.m_nRadius = 3;
            DispatchEffect("MetalSpark", data);
            break;

        case EMITTER_DUST:
            DispatchEffect("env_dust_1", data);
            break;

        case EMITTER_BLOOD:
            data.m_nColor = 0; // Red blood
            DispatchEffect("blood_impact_red_01", data);
            break;

        case EMITTER_WATER:
            data.m_nMagnitude = 3;
            DispatchEffect("WaterSplashQuiet", data);
            break;

        default:
            // Custom effect or fallback
            DispatchEffect("GlassImpact", data);
            break;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get effect name for particle system
//-----------------------------------------------------------------------------
const char* CPropEmitter::GetEffectName()
{
    switch (m_EmitterType)
    {
        case EMITTER_FIRE: return "env_fire_small";
        case EMITTER_SMOKE: return "smokestack";
        case EMITTER_STEAM: return "env_steam";
        case EMITTER_SPARKS: return "MetalSpark";
        case EMITTER_DUST: return "env_dust_1";
        case EMITTER_BLOOD: return "blood_impact_red_01";
        case EMITTER_WATER: return "WaterSplashQuiet";
        default: return "GlassImpact";
    }
}

//-----------------------------------------------------------------------------
// Input functions
//-----------------------------------------------------------------------------
void CPropEmitter::InputStart(inputdata_t &inputdata)
{
    StartEmitting();
}

void CPropEmitter::InputStop(inputdata_t &inputdata)
{
    StopEmitting();
}

void CPropEmitter::InputToggle(inputdata_t &inputdata)
{
    if (m_bEmitting)
        StopEmitting();
    else
        StartEmitting();
}

void CPropEmitter::InputSetType(inputdata_t &inputdata)
{
    SetEmitterType((EmitterType_t)inputdata.value.Int());
}

void CPropEmitter::InputSetRate(inputdata_t &inputdata)
{
    SetEmitRate(inputdata.value.Float());
}

//-----------------------------------------------------------------------------
// Purpose: Create emitter entity
//-----------------------------------------------------------------------------
CPropEmitter* CPropEmitter::CreateEmitter(const Vector& position, const QAngle& angles,
                                         EmitterType_t type, CBasePlayer* pOwner)
{
    CPropEmitter* pEmitter = (CPropEmitter*)CreateEntityByName("prop_emitter");
    if (pEmitter)
    {
        pEmitter->SetAbsOrigin(position);
        pEmitter->SetAbsAngles(angles);
        pEmitter->SetOwnerEntity(pOwner);
        pEmitter->SetEmitterType(type);
        pEmitter->Spawn();
    }

    return pEmitter;
}

//-----------------------------------------------------------------------------
// Emitter System Management Implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Initialize emitter system
//-----------------------------------------------------------------------------
void CGModEmitterSystem::Initialize()
{
    if (m_bInitialized)
        return;

    // Reset all player delays
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        m_flPlayerDelays[i] = 0.0f;
        m_flLastSpawnTimes[i] = 0.0f;
    }

    m_bInitialized = true;
    Msg("Emitter System initialized\n");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown emitter system
//-----------------------------------------------------------------------------
void CGModEmitterSystem::Shutdown()
{
    if (!m_bInitialized)
        return;

    CleanupEmitters();
    m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Check if player can create emitter
//-----------------------------------------------------------------------------
bool CGModEmitterSystem::CanCreateEmitter(CBasePlayer* pPlayer)
{
    if (!pPlayer || !m_bInitialized || !gm_emitter_enabled.GetBool())
        return false;

    return CheckPlayerDelay(pPlayer);
}

//-----------------------------------------------------------------------------
// Purpose: Create emitter for player
//-----------------------------------------------------------------------------
CPropEmitter* CGModEmitterSystem::CreatePlayerEmitter(CBasePlayer* pPlayer, const Vector& position,
                                                     const QAngle& angles, EmitterType_t type)
{
    if (!CanCreateEmitter(pPlayer))
        return NULL;

    CPropEmitter* pEmitter = CPropEmitter::CreateEmitter(position, angles, type, pPlayer);
    if (pEmitter)
    {
        UpdatePlayerDelay(pPlayer);
        ClientPrint(pPlayer, HUD_PRINTTALK, "Emitter spawned: %s", GetEmitterTypeName(type));
    }

    return pEmitter;
}

//-----------------------------------------------------------------------------
// Purpose: Update all emitters
//-----------------------------------------------------------------------------
void CGModEmitterSystem::UpdateAllEmitters()
{
    // Emitters update themselves via Think function
}

//-----------------------------------------------------------------------------
// Purpose: Cleanup all emitters
//-----------------------------------------------------------------------------
void CGModEmitterSystem::CleanupEmitters()
{
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "prop_emitter")) != NULL)
    {
        UTIL_Remove(pEntity);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get total emitter count
//-----------------------------------------------------------------------------
int CGModEmitterSystem::GetEmitterCount()
{
    int count = 0;
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "prop_emitter")) != NULL)
    {
        count++;
    }
    return count;
}

//-----------------------------------------------------------------------------
// Purpose: Get player's emitter count
//-----------------------------------------------------------------------------
int CGModEmitterSystem::GetPlayerEmitterCount(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return 0;

    int count = 0;
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "prop_emitter")) != NULL)
    {
        if (pEntity->GetOwnerEntity() == pPlayer)
            count++;
    }
    return count;
}

//-----------------------------------------------------------------------------
// Purpose: Get emitter type name
//-----------------------------------------------------------------------------
const char* CGModEmitterSystem::GetEmitterTypeName(EmitterType_t type)
{
    switch (type)
    {
        case EMITTER_FIRE: return "Fire";
        case EMITTER_SMOKE: return "Smoke";
        case EMITTER_STEAM: return "Steam";
        case EMITTER_SPARKS: return "Sparks";
        case EMITTER_DUST: return "Dust";
        case EMITTER_BLOOD: return "Blood";
        case EMITTER_WATER: return "Water";
        case EMITTER_CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get emitter type from name
//-----------------------------------------------------------------------------
EmitterType_t CGModEmitterSystem::GetEmitterTypeFromName(const char* name)
{
    if (!name) return EMITTER_FIRE;

    if (Q_stricmp(name, "fire") == 0) return EMITTER_FIRE;
    if (Q_stricmp(name, "smoke") == 0) return EMITTER_SMOKE;
    if (Q_stricmp(name, "steam") == 0) return EMITTER_STEAM;
    if (Q_stricmp(name, "sparks") == 0) return EMITTER_SPARKS;
    if (Q_stricmp(name, "dust") == 0) return EMITTER_DUST;
    if (Q_stricmp(name, "blood") == 0) return EMITTER_BLOOD;
    if (Q_stricmp(name, "water") == 0) return EMITTER_WATER;
    if (Q_stricmp(name, "custom") == 0) return EMITTER_CUSTOM;

    return EMITTER_FIRE;
}

//-----------------------------------------------------------------------------
// Purpose: Check player delay
//-----------------------------------------------------------------------------
bool CGModEmitterSystem::CheckPlayerDelay(CBasePlayer* pPlayer)
{
    int playerIndex = GetPlayerIndex(pPlayer);
    if (playerIndex < 0)
        return false;

    float currentTime = gpGlobals->curtime;
    float delay = gm_emitter_delay.GetFloat();
    float timeSinceLastSpawn = currentTime - m_flLastSpawnTimes[playerIndex];

    if (timeSinceLastSpawn < delay)
    {
        float timeLeft = delay - timeSinceLastSpawn;
        ClientPrint(pPlayer, HUD_PRINTTALK, "Emitter delay: %.1f seconds remaining", timeLeft);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Update player delay
//-----------------------------------------------------------------------------
void CGModEmitterSystem::UpdatePlayerDelay(CBasePlayer* pPlayer)
{
    int playerIndex = GetPlayerIndex(pPlayer);
    if (playerIndex >= 0)
    {
        m_flLastSpawnTimes[playerIndex] = gpGlobals->curtime;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get player index
//-----------------------------------------------------------------------------
int CGModEmitterSystem::GetPlayerIndex(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return -1;

    int index = pPlayer->entindex() - 1;
    if (index < 0 || index >= MAX_PLAYERS)
        return -1;

    return index;
}

//-----------------------------------------------------------------------------
// Purpose: Get command client
//-----------------------------------------------------------------------------
CBasePlayer* CGModEmitterSystem::GetCommandPlayer()
{
    return UTIL_GetCommandClient();
}

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Spawn emitter at crosshair
//-----------------------------------------------------------------------------
void CGModEmitterSystem::CMD_gm_emitter_spawn(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    Vector eyePos = pPlayer->EyePosition();
    Vector forward;
    pPlayer->EyeVectors(&forward);

    Vector spawnPos = eyePos + forward * 100.0f;
    QAngle spawnAngles = pPlayer->EyeAngles();

    EmitterType_t type = (EmitterType_t)gm_emitter_type.GetInt();
    CreatePlayerEmitter(pPlayer, spawnPos, spawnAngles, type);
}

//-----------------------------------------------------------------------------
// Purpose: Remove player's emitters
//-----------------------------------------------------------------------------
void CGModEmitterSystem::CMD_gm_emitter_remove(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    int count = 0;
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "prop_emitter")) != NULL)
    {
        if (pEntity->GetOwnerEntity() == pPlayer)
        {
            UTIL_Remove(pEntity);
            count++;
        }
    }

    ClientPrint(pPlayer, HUD_PRINTTALK, "Removed %d emitter(s)", count);
}

//-----------------------------------------------------------------------------
// Purpose: Clear all emitters
//-----------------------------------------------------------------------------
void CGModEmitterSystem::CMD_gm_emitter_clear(void)
{
    int count = GetEmitterCount();
    CleanupEmitters();

    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
        ClientPrint(pPlayer, HUD_PRINTTALK, "Cleared %d emitter(s)", count);
}

//-----------------------------------------------------------------------------
// Purpose: Start all emitters
//-----------------------------------------------------------------------------
void CGModEmitterSystem::CMD_gm_emitter_start_all(void)
{
    int count = 0;
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "prop_emitter")) != NULL)
    {
        CPropEmitter* pEmitter = dynamic_cast<CPropEmitter*>(pEntity);
        if (pEmitter)
        {
            pEmitter->StartEmitting();
            count++;
        }
    }

    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
        ClientPrint(pPlayer, HUD_PRINTTALK, "Started %d emitter(s)", count);
}

//-----------------------------------------------------------------------------
// Purpose: Stop all emitters
//-----------------------------------------------------------------------------
void CGModEmitterSystem::CMD_gm_emitter_stop_all(void)
{
    int count = 0;
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "prop_emitter")) != NULL)
    {
        CPropEmitter* pEmitter = dynamic_cast<CPropEmitter*>(pEntity);
        if (pEmitter)
        {
            pEmitter->StopEmitting();
            count++;
        }
    }

    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
        ClientPrint(pPlayer, HUD_PRINTTALK, "Stopped %d emitter(s)", count);
}

// Register console commands
static ConCommand cmd_gm_emitter_spawn("gm_emitter_spawn", CGModEmitterSystem::CMD_gm_emitter_spawn, "Spawn emitter at crosshair");
static ConCommand cmd_gm_emitter_remove("gm_emitter_remove", CGModEmitterSystem::CMD_gm_emitter_remove, "Remove your emitters");
static ConCommand cmd_gm_emitter_clear("gm_emitter_clear", CGModEmitterSystem::CMD_gm_emitter_clear, "Clear all emitters");
static ConCommand cmd_gm_emitter_start_all("gm_emitter_start_all", CGModEmitterSystem::CMD_gm_emitter_start_all, "Start all emitters");
static ConCommand cmd_gm_emitter_stop_all("gm_emitter_stop_all", CGModEmitterSystem::CMD_gm_emitter_stop_all, "Stop all emitters");

//-----------------------------------------------------------------------------
// System initialization hook
//-----------------------------------------------------------------------------
class CEmitterSystemInit : public CAutoGameSystem
{
public:
    CEmitterSystemInit() : CAutoGameSystem("EmitterSystemInit") {}

    virtual bool Init()
    {
        CGModEmitterSystem::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModEmitterSystem::Shutdown();
    }

    virtual void FrameUpdatePostEntityThink()
    {
        CGModEmitterSystem::UpdateAllEmitters();
    }
};

static CEmitterSystemInit g_EmitterSystemInit;