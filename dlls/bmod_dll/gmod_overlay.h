#ifndef GMOD_OVERLAY_H
#define GMOD_OVERLAY_H

#include "cbase.h"
#include "igamesystem.h"
#include "filesystem.h"
#include "materialsystem/imaterial.h"
#include "vgui/IVGui.h"
#include "ienginevgui.h"
#include "utlmap.h"

// Forward declarations
class CBasePlayer;

// Overlay data structure
struct OverlayData_t
{
    char szName[64];
    char szMaterialPath[256];
    float flDuration;
    float flFadeIn;
    float flFadeOut;
    int iFlags;
    bool bActive;

    OverlayData_t()
    {
        szName[0] = '\0';
        szMaterialPath[0] = '\0';
        flDuration = 0.0f;
        flFadeIn = 0.0f;
        flFadeOut = 0.0f;
        iFlags = 0;
        bActive = false;
    }
};

// Screen overlay flags
#define OVERLAY_FLAG_FULLSCREEN     (1 << 0)
#define OVERLAY_FLAG_ADDITIVE       (1 << 1)
#define OVERLAY_FLAG_ALPHA_BLEND    (1 << 2)
#define OVERLAY_FLAG_WORLD_SPACE    (1 << 3)

// Player overlay state
struct PlayerOverlayState_t
{
    OverlayData_t currentOverlay;
    float flOverlayStartTime;
    float flOverlayEndTime;
    IMaterial* pOverlayMaterial;
    bool bOverlayActive;

    PlayerOverlayState_t()
    {
        flOverlayStartTime = 0.0f;
        flOverlayEndTime = 0.0f;
        pOverlayMaterial = NULL;
        bOverlayActive = false;
    }
};

//-----------------------------------------------------------------------------
// GMod Overlay System
//-----------------------------------------------------------------------------
class CGModOverlaySystem : public CAutoGameSystem
{
public:
    CGModOverlaySystem();
    virtual ~CGModOverlaySystem();

    // CAutoGameSystem overrides
    virtual bool Init();
    virtual void Shutdown();
    virtual void LevelInitPostEntity();
    virtual void FrameUpdatePreEntityThink();

    // Core overlay functionality
    bool LoadOverlaySettings();
    void ReloadOverlaySettings();

    // Player overlay management
    void SetPlayerOverlay(CBasePlayer* pPlayer, const char* pszOverlayName, float flDuration = -1.0f);
    void ClearPlayerOverlay(CBasePlayer* pPlayer);
    void UpdatePlayerOverlays();

    // Overlay registration
    bool RegisterOverlay(const char* pszName, const char* pszMaterialPath, float flDuration, float flFadeIn, float flFadeOut, int iFlags);
    OverlayData_t* GetOverlayByName(const char* pszName);

    // Global screen overlay
    void SetGlobalOverlay(const char* pszMaterialPath);
    void ClearGlobalOverlay();

    // Console commands
    void ListOverlays();
    void ShowOverlay(const char* pszName, float flDuration = -1.0f);
    void HideOverlay();

private:
    // Internal data
    CUtlVector<OverlayData_t> m_RegisteredOverlays;
    CUtlMap<int, PlayerOverlayState_t> m_PlayerOverlayStates; // Player overlay states

    // Global overlay state
    char m_szGlobalOverlayMaterial[256];
    IMaterial* m_pGlobalOverlayMaterial;
    bool m_bGlobalOverlayActive;

    // Helper functions
    IMaterial* LoadOverlayMaterial(const char* pszMaterialPath);
    void ParseOverlayFile(KeyValues* pKV);
    PlayerOverlayState_t* GetPlayerOverlayState(CBasePlayer* pPlayer);
    void CleanupPlayerOverlayState(int playerIndex);
};

// Global access
extern CGModOverlaySystem* g_pGModOverlaySystem;

// Console commands
void CC_GMod_ListOverlays(void);
void CC_GMod_ShowOverlay(void);
void CC_GMod_HideOverlay(void);
void CC_GMod_ReloadOverlays(void);

#endif // GMOD_OVERLAY_H