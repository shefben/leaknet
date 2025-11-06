#ifndef GMOD_SCHEME_H
#define GMOD_SCHEME_H

#include "cbase.h"
#include "igamesystem.h"
#include "filesystem.h"
#include "vgui/IScheme.h"
#include "Color.h"
#include "utlmap.h"

// Forward declarations
class CBasePlayer;

// Scheme data structure
struct SchemeData_t
{
    char szName[64];
    char szResourcePath[256];
    char szDescription[128];
    vgui::HScheme hScheme;
    bool bLoaded;
    bool bActive;
    float flLoadTime;

    SchemeData_t()
    {
        szName[0] = '\0';
        szResourcePath[0] = '\0';
        szDescription[0] = '\0';
        hScheme = 0;
        bLoaded = false;
        bActive = false;
        flLoadTime = 0.0f;
    }
};

// Scheme color definition
struct SchemeColor_t
{
    char szName[64];
    int r, g, b, a;

    SchemeColor_t()
    {
        szName[0] = '\0';
        r = g = b = a = 255;
    }

    SchemeColor_t(const char* pszName, int red, int green, int blue, int alpha = 255)
    {
        Q_strncpy(szName, pszName, sizeof(szName));
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }
};

// Scheme font definition
struct SchemeFont_t
{
    char szName[64];
    char szFontName[64];
    int iTall;
    int iWeight;
    int iFlags;
    bool bAntiAlias;

    SchemeFont_t()
    {
        szName[0] = '\0';
        szFontName[0] = '\0';
        iTall = 12;
        iWeight = 400;
        iFlags = 0;
        bAntiAlias = true;
    }
};

// Player scheme preferences (simplified for LeakNet compatibility)
struct PlayerSchemePrefs_t
{
    char szActiveScheme[64];
    bool bCustomColors;
    bool bCustomFonts;
    // Note: Custom colors/fonts vectors removed for LeakNet compatibility
    // Server DLL doesn't need complex scheme customization

    PlayerSchemePrefs_t()
    {
        szActiveScheme[0] = '\0';
        bCustomColors = false;
        bCustomFonts = false;
    }
};

//-----------------------------------------------------------------------------
// GMod UI Scheme System
//-----------------------------------------------------------------------------
class CGModSchemeSystem : public CAutoGameSystem
{
public:
    CGModSchemeSystem();
    virtual ~CGModSchemeSystem();

    // CAutoGameSystem overrides
    virtual bool Init();
    virtual void Shutdown();
    virtual void LevelInitPostEntity();

    // Core scheme functionality
    bool LoadSchemeSettings();
    void ReloadSchemeSettings(); // Based on hud_reloadscheme ConVar
    void ReloadAllSchemes();

    // Scheme management
    bool RegisterScheme(const char* pszName, const char* pszResourcePath, const char* pszDescription);
    SchemeData_t* GetSchemeByName(const char* pszName);
    bool LoadScheme(const char* pszName);
    bool UnloadScheme(const char* pszName);
    void ListSchemes();

    // Built-in scheme loading (based on IDA findings)
    bool LoadGModScheme(); // "resource/gmod_scheme.res"
    bool LoadClientScheme(); // "resource/ClientScheme.res"
    bool LoadCombineScheme(); // "resource/CombinePanelScheme.res"
    bool LoadSpawnMenuScheme(); // "resource/SpawnMenuScheme.res"

    // Player scheme preferences
    void SetPlayerScheme(CBasePlayer* pPlayer, const char* pszSchemeName);
    const char* GetPlayerScheme(CBasePlayer* pPlayer);
    void ResetPlayerScheme(CBasePlayer* pPlayer);

    // Scheme application
    void ApplySchemeToPlayer(CBasePlayer* pPlayer, const char* pszSchemeName);
    void ApplyGlobalScheme(const char* pszSchemeName);
    void RestoreDefaultScheme();

    // Color and font management
    bool AddSchemeColor(const char* pszSchemeName, const char* pszColorName, int r, int g, int b, int a = 255);
    bool AddSchemeFont(const char* pszSchemeName, const char* pszFontName, const char* pszFontFile, int iTall, int iWeight = 400);
    Color GetSchemeColor(const char* pszSchemeName, const char* pszColorName);
    vgui::HFont GetSchemeFont(const char* pszSchemeName, const char* pszFontName);

    // Console commands
    void ShowSchemeInfo(const char* pszSchemeName);
    void TestScheme(const char* pszSchemeName);

private:
    // Internal data
    CUtlVector<SchemeData_t> m_RegisteredSchemes;
    CUtlMap<int, PlayerSchemePrefs_t> m_PlayerSchemePrefs; // Player entindex -> scheme prefs

    // Active schemes
    char m_szActiveGlobalScheme[64];
    char m_szDefaultScheme[64];

    // VGUI scheme interface is accessed globally via vgui::scheme()

    // Helper functions
    void ParseSchemeFile(KeyValues* pKV, SchemeData_t* pSchemeData);
    void ParseSchemeColors(KeyValues* pKV, CUtlVector<SchemeColor_t>& colors);
    void ParseSchemeFonts(KeyValues* pKV, CUtlVector<SchemeFont_t>& fonts);
    PlayerSchemePrefs_t* GetPlayerSchemePrefs(CBasePlayer* pPlayer);
    void CleanupPlayerSchemePrefs(int playerIndex);
    bool LoadSchemeFromFile(const char* pszResourcePath, SchemeData_t* pSchemeData);
    vgui::HScheme CreateVGUIScheme(const SchemeData_t* pSchemeData);
};

// Global access
extern CGModSchemeSystem* g_pGModSchemeSystem;

// Console commands
void CC_GMod_ListSchemes(void);
void CC_GMod_LoadScheme(void);
void CC_GMod_UnloadScheme(void);
void CC_GMod_SetScheme(void);
void CC_GMod_ReloadSchemes(void);
void CC_GMod_SchemeInfo(void);
void CC_GMod_TestScheme(void);

#endif // GMOD_SCHEME_H