#include "cbase.h"
#include "gmod_death.h"
#include "player.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "convar.h"
#include "gameeventdefs.h"
#include "igameevents.h"

// Helper function to find player by user ID (LeakNet compatibility)
static CBasePlayer* UTIL_PlayerByUserId(int userid)
{
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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Console variables - based on IDA strings
ConVar hud_deathnotice_time("hud_deathnotice_time", "6", FCVAR_NONE, "Amount of time death notices appear");

// Console commands
ConCommand gmod_list_death_stats("gmod_list_death_stats", CC_GMod_ListDeathStats, "List death statistics for all players");
ConCommand gmod_reset_death_stats("gmod_reset_death_stats", CC_GMod_ResetDeathStats, "Reset all death statistics");
ConCommand gmod_test_death_notice("gmod_test_death_notice", CC_GMod_TestDeathNotice, "Test death notification");

// Global instance
CGModDeathSystem g_GMod_DeathSystem;
CGModDeathSystem* g_pGModDeathSystem = &g_GMod_DeathSystem;

//-----------------------------------------------------------------------------
// CGModDeathEntity implementation (based on "gmod_death" from IDA)
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(gmod_death, CGModDeathEntity);

BEGIN_DATADESC(CGModDeathEntity)
    DEFINE_KEYFIELD(m_szDeathMessage, FIELD_STRING, "death_message"),
    DEFINE_KEYFIELD(m_szDeathEffect, FIELD_STRING, "death_effect"),
    DEFINE_KEYFIELD(m_bInstantDeath, FIELD_BOOLEAN, "instant_death"),
    DEFINE_KEYFIELD(m_flDamageAmount, FIELD_FLOAT, "damage_amount"),

    DEFINE_INPUTFUNC(FIELD_VOID, "KillPlayer", InputKillPlayer),
    DEFINE_INPUTFUNC(FIELD_STRING, "SetDeathMessage", InputSetDeathMessage),
    DEFINE_INPUTFUNC(FIELD_STRING, "SetDeathEffect", InputSetDeathEffect),

    DEFINE_OUTPUT(m_OnPlayerDeath, "OnPlayerDeath"),
    DEFINE_OUTPUT(m_OnPlayerKilled, "OnPlayerKilled"),
END_DATADESC()

CGModDeathEntity::CGModDeathEntity()
{
    m_szDeathMessage[0] = '\0';
    m_szDeathEffect[0] = '\0';
    m_bInstantDeath = true;
    m_flDamageAmount = 1000.0f;
}

CGModDeathEntity::~CGModDeathEntity()
{
}

void CGModDeathEntity::Spawn()
{
    BaseClass::Spawn();
    Precache();
}

void CGModDeathEntity::Precache()
{
    BaseClass::Precache();

    // Precache death effects if specified
    if (m_szDeathEffect[0] != '\0')
    {
        // TODO: PrecacheParticleSystem not available in LeakNet
        // PrecacheParticleSystem(m_szDeathEffect);
    }
}

void CGModDeathEntity::TriggerDeath(CBasePlayer* pPlayer, bool bInstant)
{
    if (!pPlayer)
        return;

    if (bInstant || m_bInstantDeath)
    {
        // Instant death
        pPlayer->TakeDamage(CTakeDamageInfo(this, this, m_flDamageAmount, DMG_GENERIC));
    }
    else
    {
        // Gradual damage
        pPlayer->TakeDamage(CTakeDamageInfo(this, this, m_flDamageAmount * 0.1f, DMG_POISON));
    }

    // Fire output events
    m_OnPlayerDeath.FireOutput(pPlayer, this);
    m_OnPlayerKilled.FireOutput(pPlayer, this);

    DevMsg("gmod_death entity killed player %s\n", STRING(pPlayer->pl.netname));
}

void CGModDeathEntity::SetDeathMessage(const char* pszMessage)
{
    if (pszMessage)
    {
        Q_strncpy(m_szDeathMessage, pszMessage, sizeof(m_szDeathMessage));
    }
}

void CGModDeathEntity::SetDeathEffect(const char* pszEffect)
{
    if (pszEffect)
    {
        Q_strncpy(m_szDeathEffect, pszEffect, sizeof(m_szDeathEffect));
    }
}

void CGModDeathEntity::InputKillPlayer(inputdata_t& inputdata)
{
    CBaseEntity* pActivator = inputdata.pActivator;
    CBasePlayer* pPlayer = ToBasePlayer(pActivator);

    if (pPlayer)
    {
        TriggerDeath(pPlayer, m_bInstantDeath);
    }
    else
    {
        // Find all players in area and kill them
        for (int i = 1; i <= gpGlobals->maxClients; i++)
        {
            CBasePlayer* pLoopPlayer = UTIL_PlayerByIndex(i);
            if (pLoopPlayer && pLoopPlayer->IsAlive())
            {
                // Check if player is in range (simple distance check)
                float flDistance = (GetAbsOrigin() - pLoopPlayer->GetAbsOrigin()).Length();
                if (flDistance <= 512.0f) // 512 unit radius
                {
                    TriggerDeath(pLoopPlayer, m_bInstantDeath);
                }
            }
        }
    }
}

void CGModDeathEntity::InputSetDeathMessage(inputdata_t& inputdata)
{
    SetDeathMessage(inputdata.value.String());
}

void CGModDeathEntity::InputSetDeathEffect(inputdata_t& inputdata)
{
    SetDeathEffect(inputdata.value.String());
}

//-----------------------------------------------------------------------------
// CGModDeathSystem implementation
//-----------------------------------------------------------------------------
CGModDeathSystem::CGModDeathSystem() : CAutoGameSystem()
{
    m_flDeathNoticeTime = 6.0f;
    m_iMaxDeathNotices = 10; // Based on MaxDeathNotices from IDA

    // Note: SetDefLessFunc not available in LeakNet - using vector-based storage instead
}

CGModDeathSystem::~CGModDeathSystem()
{
    Shutdown();
}

bool CGModDeathSystem::Init()
{
    // Note: Game event listening not available in LeakNet - events would need to be handled differently
    // TODO: Implement alternative event handling mechanism for LeakNet
    // ListenForGameEvent("player_death");
    // ListenForGameEvent("player_hurt");
    // ListenForGameEvent("player_spawn");

    return true;
}

void CGModDeathSystem::Shutdown()
{
    // Note: StopListeningForAllEvents not available in LeakNet
    // TODO: Clean up event listeners when implemented

    // Clear all data
    m_DeathNotifications.RemoveAll();
    m_PlayerDeathStats.RemoveAll();

    // Clear weapon death icons array
    m_WeaponDeathIconCount = 0;
}

void CGModDeathSystem::LevelInitPostEntity()
{
    LoadDeathNotificationMaterials();
    m_flDeathNoticeTime = hud_deathnotice_time.GetFloat();
}

void CGModDeathSystem::FrameUpdatePreEntityThink()
{
    UpdateDeathNotifications();
}

void CGModDeathSystem::FireGameEvent(KeyValues* event)
{
    const char* pszEventName = event->GetName();

    if (Q_stricmp(pszEventName, "player_death") == 0)
    {
        ParseDeathEvent(event);
    }
    else if (Q_stricmp(pszEventName, "player_spawn") == 0)
    {
        // Clear any lingering death HUD for spawning player
        int userid = event->GetInt("userid");
        CBasePlayer* pPlayer = UTIL_PlayerByUserId(userid);
        if (pPlayer)
        {
            HidePlayerDeathHUD(pPlayer);
        }
    }
}

void CGModDeathSystem::ParseDeathEvent(KeyValues* event)
{
    // Parse player_death event
    int victim_userid = event->GetInt("userid");
    int attacker_userid = event->GetInt("attacker");
    const char* pszWeapon = event->GetString("weapon", "");
    bool bHeadshot = event->GetInt("headshot", 0) != 0;

    CBasePlayer* pVictim = UTIL_PlayerByUserId(victim_userid);
    CBasePlayer* pAttacker = UTIL_PlayerByUserId(attacker_userid);

    if (!pVictim)
        return;

    // Record death statistics
    RecordPlayerDeath(pVictim, pAttacker, pszWeapon);

    // Create death notification
    DeathNotification_t deathData;
    Q_strncpy(deathData.szVictimName, STRING(pVictim->pl.netname), sizeof(deathData.szVictimName));

    if (pAttacker && pAttacker != pVictim)
    {
        Q_strncpy(deathData.szKillerName, STRING(pAttacker->pl.netname), sizeof(deathData.szKillerName));
        deathData.bSuicide = false;
        deathData.iKillerTeam = pAttacker->GetTeamNumber();
    }
    else
    {
        Q_strncpy(deathData.szKillerName, "", sizeof(deathData.szKillerName));
        deathData.bSuicide = true;
        deathData.iKillerTeam = 0;
    }

    Q_strncpy(deathData.szWeaponName, pszWeapon, sizeof(deathData.szWeaponName));
    Q_strncpy(deathData.szDeathIcon, GetDeathIconForWeapon(pszWeapon), sizeof(deathData.szDeathIcon));
    deathData.iVictimTeam = pVictim->GetTeamNumber();
    deathData.flDeathTime = gpGlobals->curtime;
    deathData.bHeadshot = bHeadshot;
    deathData.bCritKill = false; // Could be determined by damage flags

    AddDeathNotification(deathData);

    // Show death HUD for victim
    ShowPlayerDeathHUD(pVictim);

    DevMsg("Death: %s killed %s with %s\n",
        deathData.szKillerName[0] ? deathData.szKillerName : "world",
        deathData.szVictimName, deathData.szWeaponName);
}

void CGModDeathSystem::AddDeathNotification(const DeathNotification_t& deathData)
{
    // Add to notification list
    m_DeathNotifications.AddToTail(deathData);

    // Limit number of notifications (based on MaxDeathNotices from IDA)
    while (m_DeathNotifications.Count() > m_iMaxDeathNotices)
    {
        m_DeathNotifications.Remove(0);
    }
}

void CGModDeathSystem::UpdateDeathNotifications()
{
    float flCurrentTime = gpGlobals->curtime;
    float flNoticeTime = hud_deathnotice_time.GetFloat();

    // Remove expired notifications
    for (int i = m_DeathNotifications.Count() - 1; i >= 0; i--)
    {
        if (flCurrentTime - m_DeathNotifications[i].flDeathTime > flNoticeTime)
        {
            m_DeathNotifications.Remove(i);
        }
    }
}

void CGModDeathSystem::ClearDeathNotifications()
{
    m_DeathNotifications.RemoveAll();
}

void CGModDeathSystem::RecordPlayerDeath(CBasePlayer* pVictim, CBasePlayer* pKiller, const char* pszWeapon)
{
    if (!pVictim)
        return;

    // Update victim stats
    PlayerDeathStats_t* pVictimStats = GetOrCreatePlayerDeathStats(pVictim->entindex());
    pVictimStats->iDeaths++;
    pVictimStats->flLastDeathTime = gpGlobals->curtime;
    pVictimStats->flDeathTime = gpGlobals->curtime; // Based on m_flDeathTime from IDA
    pVictimStats->iDeathFrame = gpGlobals->framecount; // Based on m_iDeathFrame from IDA

    if (pVictim == pKiller || !pKiller)
    {
        pVictimStats->iSuicides++;
    }

    // Update killer stats
    if (pKiller && pKiller != pVictim)
    {
        PlayerDeathStats_t* pKillerStats = GetOrCreatePlayerDeathStats(pKiller->entindex());
        pKillerStats->iKills++;
    }
}

PlayerDeathStats_t* CGModDeathSystem::GetPlayerDeathStats(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return NULL;

    // For vector-based storage, use direct index access (assuming 1-based player indices)
    int index = pPlayer->entindex() - 1;
    if (index < 0 || index >= m_PlayerDeathStats.Count())
        return NULL;

    return &m_PlayerDeathStats[index];
}

PlayerDeathStats_t* CGModDeathSystem::GetOrCreatePlayerDeathStats(int playerIndex)
{
    // For vector-based storage, use direct index access (assuming 1-based player indices)
    int index = playerIndex - 1;

    // Ensure the vector is large enough
    while (m_PlayerDeathStats.Count() <= index)
    {
        PlayerDeathStats_t newStats;
        m_PlayerDeathStats.AddToTail(newStats);
    }

    return &m_PlayerDeathStats[index];
}

void CGModDeathSystem::ResetPlayerDeathStats(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    // For vector-based storage, use direct index access (assuming 1-based player indices)
    int index = pPlayer->entindex() - 1;
    if (index >= 0 && index < m_PlayerDeathStats.Count())
    {
        PlayerDeathStats_t& stats = m_PlayerDeathStats[index];
        stats.iDeaths = 0;
        stats.iKills = 0;
        stats.iSuicides = 0;
        stats.flLastDeathTime = 0.0f;
        stats.iDeathFrame = 0;
        stats.iDeathPose = 0;
        stats.flDeathTime = 0.0f;
    }
}

void CGModDeathSystem::CleanupPlayerDeathStats(int playerIndex)
{
    // For vector-based storage, remove by index (assuming 1-based player indices)
    int index = playerIndex - 1;
    if (index >= 0 && index < m_PlayerDeathStats.Count())
    {
        m_PlayerDeathStats.Remove(index);
    }
}

// Implementation based on "deathnotify/%s" and "materials/deathnotify/*.vmt" from IDA
bool CGModDeathSystem::LoadDeathNotificationMaterials()
{
    LoadWeaponDeathIcons();
    PrecacheDeathMaterials();
    return true;
}

void CGModDeathSystem::LoadWeaponDeathIcons()
{
    // Load weapon to death icon mappings using array structure
    // Based on "materials/deathnotify/*.vmt" pattern from IDA

    m_WeaponDeathIconCount = 0;

    // Common weapon death icons - store weapon name in [x][0] and icon in [x][1]
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "crowbar", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_crowbar", 256);
    m_WeaponDeathIconCount++;

    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "pistol", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_pistol", 256);
    m_WeaponDeathIconCount++;

    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "357", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_357", 256);
    m_WeaponDeathIconCount++;

    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "smg1", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_smg1", 256);
    m_WeaponDeathIconCount++;

    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "ar2", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_ar2", 256);
    m_WeaponDeathIconCount++;

    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "shotgun", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_shotgun", 256);
    m_WeaponDeathIconCount++;

    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "crossbow", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_crossbow", 256);
    m_WeaponDeathIconCount++;

    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "grenade", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_grenade", 256);
    m_WeaponDeathIconCount++;

    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "rpg", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_rpg", 256);
    m_WeaponDeathIconCount++;

    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "physics", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_physics", 256);
    m_WeaponDeathIconCount++;

    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "combine_ball", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_ball", 256);
    m_WeaponDeathIconCount++;

    // Default/unknown weapon (empty string)
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][0], "", 256);
    Q_strncpy(m_WeaponDeathIcons[m_WeaponDeathIconCount][1], "deathnotify/d_skull", 256);
    m_WeaponDeathIconCount++;
}

void CGModDeathSystem::PrecacheDeathMaterials()
{
    // Precache all death notification materials
    for (int i = 0; i < m_WeaponDeathIconCount; i++)
    {
        const char* pszMaterial = m_WeaponDeathIcons[i][1]; // Icon path is in [i][1]
        if (pszMaterial && pszMaterial[0])
        {
            PrecacheMaterial(pszMaterial);
        }
    }
}

const char* CGModDeathSystem::GetDeathIconForWeapon(const char* pszWeaponName)
{
    if (!pszWeaponName || !pszWeaponName[0])
        return "deathnotify/d_skull";

    // Search through weapon death icon array
    for (int i = 0; i < m_WeaponDeathIconCount; i++)
    {
        if (Q_stricmp(m_WeaponDeathIcons[i][0], pszWeaponName) == 0)
        {
            return m_WeaponDeathIcons[i][1]; // Return the icon path
        }
    }

    // Return default death icon
    return "deathnotify/d_skull";
}

void CGModDeathSystem::ShowPlayerDeathHUD(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    // Show death HUD elements (based on HudPlayerDeath from IDA)
    // This would typically involve sending user messages or setting HUD flags
    DevMsg("Showing death HUD for player %d\n", pPlayer->entindex());
}

void CGModDeathSystem::HidePlayerDeathHUD(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    // Hide death HUD elements
    DevMsg("Hiding death HUD for player %d\n", pPlayer->entindex());
}

bool CGModDeathSystem::IsPlayerDeathHUDVisible(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return false;

    // Check if death HUD is currently visible for player
    return !pPlayer->IsAlive();
}

void CGModDeathSystem::ListDeathStats()
{
    Msg("Death Statistics:\n");
    Msg("Active death notifications: %d/%d\n", m_DeathNotifications.Count(), m_iMaxDeathNotices);

    for (int i = 0; i < m_PlayerDeathStats.Count(); i++)
    {
        const PlayerDeathStats_t& stats = m_PlayerDeathStats[i];

        // For vector-based storage, the player index would typically be i+1 or stored within the struct
        int playerIndex = i + 1; // Assuming 1-based player indices
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerIndex);
        char szName[32];
        if (pPlayer) {
            Q_snprintf(szName, sizeof(szName), "Player_%d", pPlayer->entindex());
        } else {
            Q_snprintf(szName, sizeof(szName), "Player_%d", playerIndex);
        }
        const char* pszName = szName;

        Msg("Player %s: Deaths=%d, Kills=%d, Suicides=%d, Last Death=%.1f\n",
            pszName, stats.iDeaths, stats.iKills, stats.iSuicides, stats.flLastDeathTime);
    }
}

void CGModDeathSystem::ResetDeathStats()
{
    m_PlayerDeathStats.RemoveAll();
    m_DeathNotifications.RemoveAll();
    Msg("All death statistics reset.\n");
}

void CGModDeathSystem::TestDeathNotification()
{
    // Create test death notification
    DeathNotification_t testData;
    Q_strncpy(testData.szVictimName, "TestVictim", sizeof(testData.szVictimName));
    Q_strncpy(testData.szKillerName, "TestKiller", sizeof(testData.szKillerName));
    Q_strncpy(testData.szWeaponName, "crowbar", sizeof(testData.szWeaponName));
    Q_strncpy(testData.szDeathIcon, GetDeathIconForWeapon("crowbar"), sizeof(testData.szDeathIcon));
    testData.iVictimTeam = 2;
    testData.iKillerTeam = 1;
    testData.flDeathTime = gpGlobals->curtime;
    testData.bSuicide = false;
    testData.bHeadshot = true;
    testData.bCritKill = false;

    AddDeathNotification(testData);
    Msg("Test death notification added.\n");
}

//-----------------------------------------------------------------------------
// Console command implementations
//-----------------------------------------------------------------------------
void CC_GMod_ListDeathStats(void)
{
    if (g_pGModDeathSystem)
    {
        g_pGModDeathSystem->ListDeathStats();
    }
}

void CC_GMod_ResetDeathStats(void)
{
    if (g_pGModDeathSystem)
    {
        g_pGModDeathSystem->ResetDeathStats();
    }
}

void CC_GMod_TestDeathNotice(void)
{
    if (g_pGModDeathSystem)
    {
        g_pGModDeathSystem->TestDeathNotification();
    }
}