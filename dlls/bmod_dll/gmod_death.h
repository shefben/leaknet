#ifndef GMOD_DEATH_H
#define GMOD_DEATH_H

#include "cbase.h"
#include "igamesystem.h"
#include "igameevents.h"

// Forward declarations
class CBasePlayer;

// Death notification data
struct DeathNotification_t
{
    char szVictimName[64];
    char szKillerName[64];
    char szWeaponName[64];
    char szDeathIcon[64];
    int iVictimTeam;
    int iKillerTeam;
    float flDeathTime;
    bool bSuicide;
    bool bHeadshot;
    bool bCritKill;

    DeathNotification_t()
    {
        szVictimName[0] = '\0';
        szKillerName[0] = '\0';
        szWeaponName[0] = '\0';
        szDeathIcon[0] = '\0';
        iVictimTeam = 0;
        iKillerTeam = 0;
        flDeathTime = 0.0f;
        bSuicide = false;
        bHeadshot = false;
        bCritKill = false;
    }
};

// Player death statistics
struct PlayerDeathStats_t
{
    int iDeaths; // Based on m_iDeaths from IDA
    int iKills;
    int iSuicides;
    float flLastDeathTime;
    int iDeathFrame; // Based on m_iDeathFrame from IDA
    int iDeathPose; // Based on m_iDeathPose from IDA
    float flDeathTime; // Based on m_flDeathTime from IDA

    PlayerDeathStats_t()
    {
        iDeaths = 0;
        iKills = 0;
        iSuicides = 0;
        flLastDeathTime = 0.0f;
        iDeathFrame = 0;
        iDeathPose = 0;
        flDeathTime = 0.0f;
    }
};

//-----------------------------------------------------------------------------
// GMod Death Entity (based on "gmod_death" string from IDA)
//-----------------------------------------------------------------------------
class CGModDeathEntity : public CBaseEntity
{
    DECLARE_CLASS(CGModDeathEntity, CBaseEntity);
    DECLARE_DATADESC();

public:
    CGModDeathEntity();
    virtual ~CGModDeathEntity();

    virtual void Spawn();
    virtual void Precache();

    // Death trigger functionality
    void TriggerDeath(CBasePlayer* pPlayer, bool bInstant = true);
    void SetDeathMessage(const char* pszMessage);
    void SetDeathEffect(const char* pszEffect);

    // Input handlers
    void InputKillPlayer(inputdata_t& inputdata);
    void InputSetDeathMessage(inputdata_t& inputdata);
    void InputSetDeathEffect(inputdata_t& inputdata);

    // Output events
    COutputEvent m_OnPlayerDeath;
    COutputEvent m_OnPlayerKilled;

private:
    char m_szDeathMessage[256];
    char m_szDeathEffect[64];
    bool m_bInstantDeath;
    float m_flDamageAmount;
};

//-----------------------------------------------------------------------------
// GMod Death System
//-----------------------------------------------------------------------------
class CGModDeathSystem : public CAutoGameSystem, public IGameEventListener
{
public:
    CGModDeathSystem();
    virtual ~CGModDeathSystem();

    // CAutoGameSystem overrides
    virtual bool Init();
    virtual void Shutdown();
    virtual void LevelInitPostEntity();
    virtual void FrameUpdatePreEntityThink();

    // IGameEventListener overrides
    virtual void FireGameEvent(KeyValues* event);
    virtual void GameEventsUpdated() {}
    virtual bool IsLocalListener() { return true; }

    // Death notification management (based on CHudDeathNotice from IDA)
    void AddDeathNotification(const DeathNotification_t& deathData);
    void ClearDeathNotifications();
    void UpdateDeathNotifications();
    int GetMaxDeathNotices() const { return m_iMaxDeathNotices; }
    void SetMaxDeathNotices(int iMax) { m_iMaxDeathNotices = iMax; }

    // Player death statistics
    void RecordPlayerDeath(CBasePlayer* pVictim, CBasePlayer* pKiller, const char* pszWeapon);
    PlayerDeathStats_t* GetPlayerDeathStats(CBasePlayer* pPlayer);
    void ResetPlayerDeathStats(CBasePlayer* pPlayer);

    // Death notification materials (based on "deathnotify/%s" and "materials/deathnotify/*.vmt")
    bool LoadDeathNotificationMaterials();
    const char* GetDeathIconForWeapon(const char* pszWeaponName);

    // HUD death notice functionality (based on HudPlayerDeath from IDA)
    void ShowPlayerDeathHUD(CBasePlayer* pPlayer);
    void HidePlayerDeathHUD(CBasePlayer* pPlayer);
    bool IsPlayerDeathHUDVisible(CBasePlayer* pPlayer);

    // Console commands
    void ListDeathStats();
    void ResetDeathStats();
    void TestDeathNotification();

private:
    // Death notifications
    CUtlVector<DeathNotification_t> m_DeathNotifications;
    float m_flDeathNoticeTime; // Based on hud_deathnotice_time ConVar
    int m_iMaxDeathNotices; // Based on MaxDeathNotices from IDA

    // Player death statistics
    CUtlVector<PlayerDeathStats_t> m_PlayerDeathStats; // Player death stats

    // Death notification materials - simplified for LeakNet compatibility
    char m_WeaponDeathIcons[50][2][256]; // weapon_name, death_icon pairs
    int m_WeaponDeathIconCount;

    // Helper functions
    void ParseDeathEvent(KeyValues* event);
    PlayerDeathStats_t* GetOrCreatePlayerDeathStats(int playerIndex);
    void CleanupPlayerDeathStats(int playerIndex);
    void LoadWeaponDeathIcons();
    void PrecacheDeathMaterials();
};

// Global access
extern CGModDeathSystem* g_pGModDeathSystem;

// Console commands
void CC_GMod_ListDeathStats(void);
void CC_GMod_ResetDeathStats(void);
void CC_GMod_TestDeathNotice(void);

#endif // GMOD_DEATH_H