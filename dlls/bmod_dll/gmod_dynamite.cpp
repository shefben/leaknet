//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Dynamite System Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_dynamite.h"
#include "player.h"
#include "tier1/strtools.h"
#include "explode.h"
#include "soundent.h"
#include "te_effect_dispatch.h"
#include "util.h"

// Initialize static members
bool CGModDynamiteSystem::m_bInitialized = false;
float CGModDynamiteSystem::m_flPlayerDelays[MAX_PLAYERS];
float CGModDynamiteSystem::m_flLastSpawnTimes[MAX_PLAYERS];

// Console variables for dynamite system
ConVar gm_dynamite_delay("gm_dynamite_delay", "5.0", FCVAR_GAMEDLL, "Base delay between dynamite spawns (seconds)");
ConVar gm_dynamite_delay_add("gm_dynamite_delay_add", "2.0", FCVAR_GAMEDLL, "Additional delay added each time (anti-spam)");
ConVar gm_dynamite_power("gm_dynamite_power", "100.0", FCVAR_GAMEDLL, "Default explosion power for dynamite");
ConVar gm_dynamite_timer("gm_dynamite_timer", "5.0", FCVAR_GAMEDLL, "Default timer for dynamite explosions");
ConVar gm_dynamite_sound("gm_dynamite_sound", "1", FCVAR_GAMEDLL, "Enable dynamite tick sounds");

//-----------------------------------------------------------------------------
// Dynamite Entity Implementation
//-----------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(gmod_dynamite, CGModDynamite);

BEGIN_DATADESC(CGModDynamite)
    DEFINE_FIELD(m_flExplodeTime, FIELD_TIME),
    DEFINE_FIELD(m_flExplosionPower, FIELD_FLOAT),
    DEFINE_FIELD(m_bArmed, FIELD_BOOLEAN),

    DEFINE_THINKFUNC(Think),
    DEFINE_ENTITYFUNC(Touch),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Spawn the dynamite entity
//-----------------------------------------------------------------------------
void CGModDynamite::Spawn()
{
    Precache();

    SetModel("models/props_c17/oildrum001_explosive.mdl");
    SetSolid(SOLID_BBOX);
    SetMoveType(MOVETYPE_VPHYSICS);

    UTIL_SetSize(this, Vector(-8, -8, -8), Vector(8, 8, 8));

    // Create physics object
    VPhysicsInitNormal(SOLID_BBOX, 0, false);

    InitDynamite();

    SetThink(&CGModDynamite::Think);
    SetNextThink(gpGlobals->curtime + 0.1f);

    SetTouch(&CGModDynamite::Touch);
}

//-----------------------------------------------------------------------------
// Purpose: Precache models and sounds
//-----------------------------------------------------------------------------
void CGModDynamite::Precache()
{
    PrecacheModel("models/props_c17/oildrum001_explosive.mdl");
    PrecacheScriptSound("Grenade.Blip");
    PrecacheScriptSound("BaseExplosionEffect.Sound");
}

//-----------------------------------------------------------------------------
// Purpose: Initialize dynamite properties
//-----------------------------------------------------------------------------
void CGModDynamite::InitDynamite()
{
    m_flExplosionPower = gm_dynamite_power.GetFloat();
    m_flExplodeTime = gpGlobals->curtime + gm_dynamite_timer.GetFloat();
    m_bArmed = true;

    // Set color to indicate it's dangerous
    SetRenderColor(255, 128, 0, 255);
    SetRenderMode(kRenderTransColor);
}

//-----------------------------------------------------------------------------
// Purpose: Think function for countdown
//-----------------------------------------------------------------------------
void CGModDynamite::Think()
{
    if (!m_bArmed)
        return;

    float flTimeLeft = m_flExplodeTime - gpGlobals->curtime;

    if (flTimeLeft <= 0.0f)
    {
        Explode();
        return;
    }

    // Play tick sound when getting close to explosion
    if (flTimeLeft <= 3.0f && gm_dynamite_sound.GetBool())
    {
        PlayTickSound();
    }

    // Flash faster as time gets closer
    if (flTimeLeft <= 2.0f)
    {
        SetNextThink(gpGlobals->curtime + 0.1f);

        // Flash red/orange
        if ((int)(flTimeLeft * 10) % 2)
            SetRenderColor(255, 0, 0, 255);
        else
            SetRenderColor(255, 128, 0, 255);
    }
    else
    {
        SetNextThink(gpGlobals->curtime + 0.5f);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle touch events
//-----------------------------------------------------------------------------
void CGModDynamite::Touch(CBaseEntity *pOther)
{
    // Don't detonate on owner for first second
    if (pOther == GetOwnerEntity() && gpGlobals->curtime < GetSpawnTime() + 1.0f)
        return;

    BaseClass::Touch(pOther);
}

//-----------------------------------------------------------------------------
// Purpose: Explode the dynamite
//-----------------------------------------------------------------------------
void CGModDynamite::Explode()
{
    if (!m_bArmed)
        return;

    m_bArmed = false;

    Vector vecOrigin = GetAbsOrigin();

    // Create explosion effect
    ExplosionCreate(vecOrigin, GetAbsAngles(), GetOwnerEntity(),
                   (int)m_flExplosionPower, 200,
                   SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS);

    // Apply damage in radius
    RadiusDamage(CTakeDamageInfo(this, GetOwnerEntity(), m_flExplosionPower, DMG_BLAST),
                 vecOrigin, m_flExplosionPower * 3.0f, CLASS_NONE, NULL);

    // Create light effect
    CBroadcastRecipientFilter filter;
    te->Explosion(filter, 0.0f, &vecOrigin, g_sModelIndexFireball, 10, 15, TE_EXPLFLAG_NONE, 200, 0);

    // Sound effect
    EmitSound("BaseExplosionEffect.Sound");

    // Remove the entity
    UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
// Purpose: Set explosion timer
//-----------------------------------------------------------------------------
void CGModDynamite::SetTimer(float flTime)
{
    m_flExplodeTime = gpGlobals->curtime + flTime;
}

//-----------------------------------------------------------------------------
// Purpose: Set explosion power
//-----------------------------------------------------------------------------
void CGModDynamite::SetPower(float flPower)
{
    m_flExplosionPower = flPower;
}

//-----------------------------------------------------------------------------
// Purpose: Play tick sound
//-----------------------------------------------------------------------------
void CGModDynamite::PlayTickSound()
{
    EmitSound("Grenade.Blip");
}

//-----------------------------------------------------------------------------
// Purpose: Create dynamite entity
//-----------------------------------------------------------------------------
CGModDynamite* CGModDynamite::CreateDynamite(const Vector& position, const QAngle& angles, CBasePlayer* pOwner)
{
    CGModDynamite* pDynamite = (CGModDynamite*)CreateEntityByName("gmod_dynamite");
    if (pDynamite)
    {
        pDynamite->SetAbsOrigin(position);
        pDynamite->SetAbsAngles(angles);
        pDynamite->SetOwnerEntity(pOwner);
        pDynamite->Spawn();
    }

    return pDynamite;
}

//-----------------------------------------------------------------------------
// Dynamite System Management Implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Initialize dynamite system
//-----------------------------------------------------------------------------
void CGModDynamiteSystem::Initialize()
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
    Msg("Dynamite System initialized\n");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown dynamite system
//-----------------------------------------------------------------------------
void CGModDynamiteSystem::Shutdown()
{
    if (!m_bInitialized)
        return;

    CleanupDynamite();
    m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Check if player can create dynamite (delay system)
//-----------------------------------------------------------------------------
bool CGModDynamiteSystem::CanCreateDynamite(CBasePlayer* pPlayer)
{
    if (!pPlayer || !m_bInitialized)
        return false;

    int playerIndex = GetPlayerIndex(pPlayer);
    if (playerIndex < 0)
        return false;

    float currentTime = gpGlobals->curtime;
    float playerDelay = GetPlayerDelay(pPlayer);
    float timeSinceLastSpawn = currentTime - m_flLastSpawnTimes[playerIndex];

    if (timeSinceLastSpawn < playerDelay)
    {
        float timeLeft = playerDelay - timeSinceLastSpawn;
        ClientPrint(pPlayer, HUD_PRINTTALK, "Dynamite delay: %.1f seconds remaining", timeLeft);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Create dynamite for player with delay management
//-----------------------------------------------------------------------------
CGModDynamite* CGModDynamiteSystem::CreatePlayerDynamite(CBasePlayer* pPlayer, const Vector& position, const QAngle& angles)
{
    if (!CanCreateDynamite(pPlayer))
        return NULL;

    CGModDynamite* pDynamite = CGModDynamite::CreateDynamite(position, angles, pPlayer);
    if (pDynamite)
    {
        UpdatePlayerDelay(pPlayer);
        ClientPrint(pPlayer, HUD_PRINTTALK, "Dynamite spawned - Timer: %.1f seconds", gm_dynamite_timer.GetFloat());
    }

    return pDynamite;
}

//-----------------------------------------------------------------------------
// Purpose: Update player delay (incremental anti-spam system)
//-----------------------------------------------------------------------------
void CGModDynamiteSystem::UpdatePlayerDelay(CBasePlayer* pPlayer)
{
    int playerIndex = GetPlayerIndex(pPlayer);
    if (playerIndex < 0)
        return;

    // Increment delay each time
    m_flPlayerDelays[playerIndex] += gm_dynamite_delay_add.GetFloat();

    // Cap maximum delay at 60 seconds
    if (m_flPlayerDelays[playerIndex] > 60.0f)
        m_flPlayerDelays[playerIndex] = 60.0f;

    m_flLastSpawnTimes[playerIndex] = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Get current delay for player
//-----------------------------------------------------------------------------
float CGModDynamiteSystem::GetPlayerDelay(CBasePlayer* pPlayer)
{
    int playerIndex = GetPlayerIndex(pPlayer);
    if (playerIndex < 0)
        return gm_dynamite_delay.GetFloat();

    float baseDelay = gm_dynamite_delay.GetFloat();
    float additionalDelay = m_flPlayerDelays[playerIndex];

    return baseDelay + additionalDelay;
}

//-----------------------------------------------------------------------------
// Purpose: Reset player delay
//-----------------------------------------------------------------------------
void CGModDynamiteSystem::ResetPlayerDelay(CBasePlayer* pPlayer)
{
    int playerIndex = GetPlayerIndex(pPlayer);
    if (playerIndex >= 0)
    {
        m_flPlayerDelays[playerIndex] = 0.0f;
        m_flLastSpawnTimes[playerIndex] = 0.0f;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get player index for delay tracking
//-----------------------------------------------------------------------------
int CGModDynamiteSystem::GetPlayerIndex(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return -1;

    int index = pPlayer->entindex() - 1;
    if (index < 0 || index >= MAX_PLAYERS)
        return -1;

    return index;
}

//-----------------------------------------------------------------------------
// Purpose: Get command client player
//-----------------------------------------------------------------------------
CBasePlayer* CGModDynamiteSystem::GetCommandPlayer()
{
    return UTIL_GetCommandClient();
}

//-----------------------------------------------------------------------------
// Purpose: Cleanup all dynamite entities
//-----------------------------------------------------------------------------
void CGModDynamiteSystem::CleanupDynamite()
{
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "gmod_dynamite")) != NULL)
    {
        UTIL_Remove(pEntity);
    }
}

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Spawn dynamite at player's crosshair
//-----------------------------------------------------------------------------
void CGModDynamiteSystem::CMD_gm_dynamite_spawn(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    // Get player's eye position and direction
    Vector eyePos = pPlayer->EyePosition();
    Vector forward;
    pPlayer->EyeVectors(&forward);

    Vector spawnPos = eyePos + forward * 100.0f;
    QAngle spawnAngles = pPlayer->EyeAngles();

    CreatePlayerDynamite(pPlayer, spawnPos, spawnAngles);
}

//-----------------------------------------------------------------------------
// Purpose: Explode all dynamite immediately
//-----------------------------------------------------------------------------
void CGModDynamiteSystem::CMD_gm_dynamite_explode_all(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    int count = 0;
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "gmod_dynamite")) != NULL)
    {
        CGModDynamite* pDynamite = dynamic_cast<CGModDynamite*>(pEntity);
        if (pDynamite)
        {
            pDynamite->SetTimer(0.1f);
            count++;
        }
    }

    ClientPrint(pPlayer, HUD_PRINTTALK, "Detonating %d dynamite(s)", count);
}

//-----------------------------------------------------------------------------
// Purpose: Remove all dynamite
//-----------------------------------------------------------------------------
void CGModDynamiteSystem::CMD_gm_dynamite_clear(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    int count = 0;
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "gmod_dynamite")) != NULL)
    {
        UTIL_Remove(pEntity);
        count++;
    }

    ClientPrint(pPlayer, HUD_PRINTTALK, "Removed %d dynamite(s)", count);
}

// Register console commands
static ConCommand cmd_gm_dynamite_spawn("gm_dynamite_spawn", CGModDynamiteSystem::CMD_gm_dynamite_spawn, "Spawn dynamite at crosshair");
static ConCommand cmd_gm_dynamite_explode_all("gm_dynamite_explode_all", CGModDynamiteSystem::CMD_gm_dynamite_explode_all, "Explode all dynamite immediately");
static ConCommand cmd_gm_dynamite_clear("gm_dynamite_clear", CGModDynamiteSystem::CMD_gm_dynamite_clear, "Remove all dynamite");

//-----------------------------------------------------------------------------
// System initialization hook
//-----------------------------------------------------------------------------
class CDynamiteSystemInit : public CAutoGameSystem
{
public:
    CDynamiteSystemInit() : CAutoGameSystem("DynamiteSystemInit") {}

    virtual bool Init()
    {
        CGModDynamiteSystem::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModDynamiteSystem::Shutdown();
    }

    virtual void LevelShutdownPostEntity()
    {
        // Reset all player delays on level change
        for (int i = 1; i <= gpGlobals->maxClients; i++)
        {
            CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
            if (pPlayer)
            {
                CGModDynamiteSystem::ResetPlayerDelay(pPlayer);
            }
        }
    }
};

static CDynamiteSystemInit g_DynamiteSystemInit;