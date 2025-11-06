#include "cbase.h"
#include "gmod_scheme.h"
#include "player.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "convar.h"
#include "vgui/IScheme.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/Controls.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Console variables - based on IDA strings
ConVar hud_reloadscheme("hud_reloadscheme", "0", FCVAR_NONE, "Reload HUD scheme");

// Console commands
ConCommand gmod_list_schemes("gmod_list_schemes", CC_GMod_ListSchemes, "List all registered schemes");
ConCommand gmod_load_scheme("gmod_load_scheme", CC_GMod_LoadScheme, "Load a scheme by name");
ConCommand gmod_unload_scheme("gmod_unload_scheme", CC_GMod_UnloadScheme, "Unload a scheme by name");
ConCommand gmod_set_scheme("gmod_set_scheme", CC_GMod_SetScheme, "Set active scheme");
ConCommand gmod_reload_schemes("gmod_reload_schemes", CC_GMod_ReloadSchemes, "Reload all schemes");
ConCommand gmod_scheme_info("gmod_scheme_info", CC_GMod_SchemeInfo, "Show scheme information");
ConCommand gmod_test_scheme("gmod_test_scheme", CC_GMod_TestScheme, "Test a scheme");

// Global instance
CGModSchemeSystem g_GMod_SchemeSystem;
CGModSchemeSystem* g_pGModSchemeSystem = &g_GMod_SchemeSystem;

//-----------------------------------------------------------------------------
// CGModSchemeSystem implementation
//-----------------------------------------------------------------------------
CGModSchemeSystem::CGModSchemeSystem() : CAutoGameSystem()
{
    m_szActiveGlobalScheme[0] = '\0';
    Q_strncpy(m_szDefaultScheme, "ClientScheme", sizeof(m_szDefaultScheme));

    SetDefLessFunc(m_PlayerSchemePrefs);
}

CGModSchemeSystem::~CGModSchemeSystem()
{
    Shutdown();
}

bool CGModSchemeSystem::Init()
{
    // Note: VGUI scheme interface is not available in server DLL
    // Server-side scheme system works with basic scheme data only
    DevMsg("GMod Scheme System: Init() - Server DLL mode\n");
    return true;
}

void CGModSchemeSystem::Shutdown()
{
    // Unload all schemes
    for (int i = 0; i < m_RegisteredSchemes.Count(); i++)
    {
        SchemeData_t& scheme = m_RegisteredSchemes[i];
        if (scheme.hScheme != 0)
        {
            // VGUI handles scheme cleanup automatically
            scheme.hScheme = 0;
        }
    }

    // Clear all data
    m_RegisteredSchemes.RemoveAll();
    m_PlayerSchemePrefs.RemoveAll();
}

void CGModSchemeSystem::LevelInitPostEntity()
{
    LoadSchemeSettings();
}

bool CGModSchemeSystem::LoadSchemeSettings()
{
    // Load all built-in schemes (based on IDA findings)
    bool bSuccess = true;

    bSuccess &= LoadClientScheme();
    bSuccess &= LoadGModScheme();
    bSuccess &= LoadCombineScheme();
    bSuccess &= LoadSpawnMenuScheme();

    DevMsg("Loaded %d UI schemes\n", m_RegisteredSchemes.Count());
    return bSuccess;
}

void CGModSchemeSystem::ReloadSchemeSettings()
{
    // Clear existing schemes
    m_RegisteredSchemes.RemoveAll();

    // Reload all schemes
    LoadSchemeSettings();
}

void CGModSchemeSystem::ReloadAllSchemes()
{
    // Based on hud_reloadscheme ConVar functionality
    for (int i = 0; i < m_RegisteredSchemes.Count(); i++)
    {
        SchemeData_t& scheme = m_RegisteredSchemes[i];
        if (scheme.bLoaded)
        {
            UnloadScheme(scheme.szName);
            LoadScheme(scheme.szName);
        }
    }

    DevMsg("All schemes reloaded.\n");
}

// Implementation based on "resource/ClientScheme.res" from IDA
bool CGModSchemeSystem::LoadClientScheme()
{
    const char* pszName = "ClientScheme";
    const char* pszPath = "resource/ClientScheme.res";
    const char* pszDescription = "Default client UI scheme";

    return RegisterScheme(pszName, pszPath, pszDescription);
}

// Implementation based on "resource/gmod_scheme.res" from IDA
bool CGModSchemeSystem::LoadGModScheme()
{
    const char* pszName = "GMod";
    const char* pszPath = "resource/gmod_scheme.res";
    const char* pszDescription = "Garry's Mod UI scheme";

    if (!filesystem->FileExists(pszPath, "GAME"))
    {
        DevMsg("GMod scheme file not found: %s\n", pszPath);
        return false;
    }

    return RegisterScheme(pszName, pszPath, pszDescription);
}

// Implementation based on "resource/CombinePanelScheme.res" from IDA
bool CGModSchemeSystem::LoadCombineScheme()
{
    const char* pszName = "CombineScheme";
    const char* pszPath = "resource/CombinePanelScheme.res";
    const char* pszDescription = "Combine panel UI scheme";

    if (!filesystem->FileExists(pszPath, "GAME"))
    {
        DevMsg("Combine scheme file not found: %s\n", pszPath);
        return false;
    }

    return RegisterScheme(pszName, pszPath, pszDescription);
}

// Implementation based on "resource/SpawnMenuScheme.res" from IDA
bool CGModSchemeSystem::LoadSpawnMenuScheme()
{
    const char* pszName = "SpawnMenuScheme";
    const char* pszPath = "resource/SpawnMenuScheme.res";
    const char* pszDescription = "Spawn menu UI scheme";

    if (!filesystem->FileExists(pszPath, "GAME"))
    {
        DevMsg("Spawn menu scheme file not found: %s\n", pszPath);
        return false;
    }

    return RegisterScheme(pszName, pszPath, pszDescription);
}

bool CGModSchemeSystem::RegisterScheme(const char* pszName, const char* pszResourcePath, const char* pszDescription)
{
    if (!pszName || !pszResourcePath)
        return false;

    // Check if scheme already exists
    if (GetSchemeByName(pszName))
    {
        DevMsg("Scheme '%s' already registered\n", pszName);
        return false;
    }

    SchemeData_t schemeData;
    Q_strncpy(schemeData.szName, pszName, sizeof(schemeData.szName));
    Q_strncpy(schemeData.szResourcePath, pszResourcePath, sizeof(schemeData.szResourcePath));
    Q_strncpy(schemeData.szDescription, pszDescription ? pszDescription : "", sizeof(schemeData.szDescription));
    schemeData.hScheme = 0;
    schemeData.bLoaded = false;
    schemeData.bActive = false;
    schemeData.flLoadTime = gpGlobals->curtime;

    m_RegisteredSchemes.AddToTail(schemeData);

    // Attempt to load the scheme immediately
    LoadScheme(pszName);

    return true;
}

SchemeData_t* CGModSchemeSystem::GetSchemeByName(const char* pszName)
{
    if (!pszName)
        return NULL;

    for (int i = 0; i < m_RegisteredSchemes.Count(); i++)
    {
        if (Q_stricmp(m_RegisteredSchemes[i].szName, pszName) == 0)
            return &m_RegisteredSchemes[i];
    }

    return NULL;
}

bool CGModSchemeSystem::LoadScheme(const char* pszName)
{
    SchemeData_t* pScheme = GetSchemeByName(pszName);
    if (!pScheme)
    {
        DevMsg("Scheme '%s' not found\n", pszName);
        return false;
    }

    if (pScheme->bLoaded)
    {
        DevMsg("Scheme '%s' already loaded\n", pszName);
        return true;
    }

    // Load scheme from file
    if (!LoadSchemeFromFile(pScheme->szResourcePath, pScheme))
    {
        DevMsg("Failed to load scheme '%s' from %s\n", pszName, pScheme->szResourcePath);
        return false;
    }

    pScheme->bLoaded = true;
    pScheme->flLoadTime = gpGlobals->curtime;

    DevMsg("Loaded scheme '%s' from %s\n", pszName, pScheme->szResourcePath);
    return true;
}

bool CGModSchemeSystem::UnloadScheme(const char* pszName)
{
    SchemeData_t* pScheme = GetSchemeByName(pszName);
    if (!pScheme)
        return false;

    if (!pScheme->bLoaded)
        return true;

    // Unload VGUI scheme
    if (pScheme->hScheme != 0)
    {
        // VGUI handles scheme cleanup automatically
        pScheme->hScheme = 0;
    }

    pScheme->bLoaded = false;
    pScheme->bActive = false;

    DevMsg("Unloaded scheme '%s'\n", pszName);
    return true;
}

bool CGModSchemeSystem::LoadSchemeFromFile(const char* pszResourcePath, SchemeData_t* pSchemeData)
{
    if (!pszResourcePath || !pSchemeData)
        return false;

    // Check if file exists
    if (!filesystem->FileExists(pszResourcePath, "GAME"))
    {
        DevMsg("Scheme file not found: %s\n", pszResourcePath);
        return false;
    }

    // Note: VGUI scheme loading is not available in server DLL
    // Set dummy scheme handle for server-side compatibility
    pSchemeData->hScheme = 1; // Use non-zero value to indicate "loaded"

    // Parse scheme file for additional data
    KeyValues* pKV = new KeyValues("Scheme");
    if (pKV->LoadFromFile(filesystem, pszResourcePath, "GAME"))
    {
        ParseSchemeFile(pKV, pSchemeData);
    }
    pKV->deleteThis();

    return true;
}

void CGModSchemeSystem::ParseSchemeFile(KeyValues* pKV, SchemeData_t* pSchemeData)
{
    if (!pKV || !pSchemeData)
        return;

    // Parse scheme metadata
    KeyValues* pScheme = pKV->FindKey("Scheme");
    if (pScheme)
    {
        // Get scheme information
        const char* pszDescription = pScheme->GetString("Description", "");
        if (pszDescription[0] != '\0')
        {
            Q_strncpy(pSchemeData->szDescription, pszDescription, sizeof(pSchemeData->szDescription));
        }
    }
}

void CGModSchemeSystem::ParseSchemeColors(KeyValues* pKV, CUtlVector<SchemeColor_t>& colors)
{
    KeyValues* pColors = pKV->FindKey("Colors");
    if (!pColors)
        return;

    for (KeyValues* pColor = pColors->GetFirstSubKey(); pColor; pColor = pColor->GetNextKey())
    {
        const char* pszName = pColor->GetName();
        const char* pszValue = pColor->GetString();

        // Parse color value (format: "r g b" or "r g b a")
        int r, g, b, a = 255;
        if (sscanf(pszValue, "%d %d %d %d", &r, &g, &b, &a) >= 3)
        {
            SchemeColor_t color(pszName, r, g, b, a);
            colors.AddToTail(color);
        }
    }
}

void CGModSchemeSystem::ParseSchemeFonts(KeyValues* pKV, CUtlVector<SchemeFont_t>& fonts)
{
    KeyValues* pFonts = pKV->FindKey("Fonts");
    if (!pFonts)
        return;

    for (KeyValues* pFont = pFonts->GetFirstSubKey(); pFont; pFont = pFont->GetNextKey())
    {
        const char* pszName = pFont->GetName();

        SchemeFont_t font;
        Q_strncpy(font.szName, pszName, sizeof(font.szName));
        Q_strncpy(font.szFontName, pFont->GetString("name", "Arial"), sizeof(font.szFontName));
        font.iTall = pFont->GetInt("tall", 12);
        font.iWeight = pFont->GetInt("weight", 400);
        font.bAntiAlias = (pFont->GetInt("antialias", 1) != 0);

        fonts.AddToTail(font);
    }
}

void CGModSchemeSystem::SetPlayerScheme(CBasePlayer* pPlayer, const char* pszSchemeName)
{
    if (!pPlayer || !pszSchemeName)
        return;

    SchemeData_t* pScheme = GetSchemeByName(pszSchemeName);
    if (!pScheme)
    {
        DevMsg("Scheme '%s' not found\n", pszSchemeName);
        return;
    }

    PlayerSchemePrefs_t* pPrefs = GetPlayerSchemePrefs(pPlayer);
    if (!pPrefs)
        return;

    Q_strncpy(pPrefs->szActiveScheme, pszSchemeName, sizeof(pPrefs->szActiveScheme));

    DevMsg("Set scheme '%s' for player %d\n", pszSchemeName, pPlayer->entindex());
}

const char* CGModSchemeSystem::GetPlayerScheme(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return m_szDefaultScheme;

    PlayerSchemePrefs_t* pPrefs = GetPlayerSchemePrefs(pPlayer);
    if (!pPrefs || pPrefs->szActiveScheme[0] == '\0')
        return m_szDefaultScheme;

    return pPrefs->szActiveScheme;
}

void CGModSchemeSystem::ResetPlayerScheme(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    PlayerSchemePrefs_t* pPrefs = GetPlayerSchemePrefs(pPlayer);
    if (!pPrefs)
        return;

    Q_strncpy(pPrefs->szActiveScheme, m_szDefaultScheme, sizeof(pPrefs->szActiveScheme));
    pPrefs->bCustomColors = false;
    pPrefs->bCustomFonts = false;
    // Note: customColors/customFonts removed for LeakNet compatibility
}

PlayerSchemePrefs_t* CGModSchemeSystem::GetPlayerSchemePrefs(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return NULL;

    int playerIndex = pPlayer->entindex();
    int index = m_PlayerSchemePrefs.Find(playerIndex);

    if (index == m_PlayerSchemePrefs.InvalidIndex())
    {
        PlayerSchemePrefs_t newPrefs;
        Q_strncpy(newPrefs.szActiveScheme, m_szDefaultScheme, sizeof(newPrefs.szActiveScheme));
        index = m_PlayerSchemePrefs.Insert(playerIndex, newPrefs);
    }

    return &m_PlayerSchemePrefs[index];
}

void CGModSchemeSystem::CleanupPlayerSchemePrefs(int playerIndex)
{
    int index = m_PlayerSchemePrefs.Find(playerIndex);
    if (index != m_PlayerSchemePrefs.InvalidIndex())
    {
        m_PlayerSchemePrefs.RemoveAt(index);
    }
}

void CGModSchemeSystem::ApplySchemeToPlayer(CBasePlayer* pPlayer, const char* pszSchemeName)
{
    if (!pPlayer || !pszSchemeName)
        return;

    SetPlayerScheme(pPlayer, pszSchemeName);
    // Additional logic for applying scheme to player's UI would go here
}

void CGModSchemeSystem::ApplyGlobalScheme(const char* pszSchemeName)
{
    if (!pszSchemeName)
        return;

    SchemeData_t* pScheme = GetSchemeByName(pszSchemeName);
    if (!pScheme)
    {
        DevMsg("Scheme '%s' not found\n", pszSchemeName);
        return;
    }

    Q_strncpy(m_szActiveGlobalScheme, pszSchemeName, sizeof(m_szActiveGlobalScheme));
    pScheme->bActive = true;

    // Deactivate other schemes
    for (int i = 0; i < m_RegisteredSchemes.Count(); i++)
    {
        if (Q_stricmp(m_RegisteredSchemes[i].szName, pszSchemeName) != 0)
        {
            m_RegisteredSchemes[i].bActive = false;
        }
    }

    DevMsg("Applied global scheme '%s'\n", pszSchemeName);
}

void CGModSchemeSystem::RestoreDefaultScheme()
{
    ApplyGlobalScheme(m_szDefaultScheme);
}

Color CGModSchemeSystem::GetSchemeColor(const char* pszSchemeName, const char* pszColorName)
{
    // Note: VGUI scheme access is not available in server DLL
    // Return default white color for server-side compatibility
    if (!pszSchemeName || !pszColorName)
        return Color(255, 255, 255, 255);

    SchemeData_t* pScheme = GetSchemeByName(pszSchemeName);
    if (!pScheme)
        return Color(255, 255, 255, 255);

    // Return default color since VGUI scheme access is client-side only
    return Color(255, 255, 255, 255);
}

vgui::HFont CGModSchemeSystem::GetSchemeFont(const char* pszSchemeName, const char* pszFontName)
{
    // Note: VGUI font access is not available in server DLL
    // Return invalid font for server-side compatibility
    if (!pszSchemeName || !pszFontName)
        return vgui::INVALID_FONT;

    SchemeData_t* pScheme = GetSchemeByName(pszSchemeName);
    if (!pScheme)
        return vgui::INVALID_FONT;

    // Return invalid font since VGUI access is client-side only
    return vgui::INVALID_FONT;
}

void CGModSchemeSystem::ListSchemes()
{
    Msg("Registered UI schemes (%d):\n", m_RegisteredSchemes.Count());
    for (int i = 0; i < m_RegisteredSchemes.Count(); i++)
    {
        const SchemeData_t& scheme = m_RegisteredSchemes[i];
        Msg("  %s: %s (%s) - %s%s\n",
            scheme.szName,
            scheme.szResourcePath,
            scheme.szDescription,
            scheme.bLoaded ? "loaded" : "not loaded",
            scheme.bActive ? ", active" : "");
    }

    if (m_szActiveGlobalScheme[0] != '\0')
    {
        Msg("Active global scheme: %s\n", m_szActiveGlobalScheme);
    }
}

void CGModSchemeSystem::ShowSchemeInfo(const char* pszSchemeName)
{
    if (!pszSchemeName)
        return;

    SchemeData_t* pScheme = GetSchemeByName(pszSchemeName);
    if (!pScheme)
    {
        Msg("Scheme '%s' not found\n", pszSchemeName);
        return;
    }

    Msg("Scheme Information: %s\n", pScheme->szName);
    Msg("  Resource Path: %s\n", pScheme->szResourcePath);
    Msg("  Description: %s\n", pScheme->szDescription);
    Msg("  Loaded: %s\n", pScheme->bLoaded ? "Yes" : "No");
    Msg("  Active: %s\n", pScheme->bActive ? "Yes" : "No");
    Msg("  Load Time: %.2f\n", pScheme->flLoadTime);
    Msg("  VGUI Handle: %d\n", pScheme->hScheme);
}

void CGModSchemeSystem::TestScheme(const char* pszSchemeName)
{
    if (!pszSchemeName)
        return;

    SchemeData_t* pScheme = GetSchemeByName(pszSchemeName);
    if (!pScheme)
    {
        Msg("Scheme '%s' not found\n", pszSchemeName);
        return;
    }

    // Apply scheme temporarily
    ApplyGlobalScheme(pszSchemeName);
    Msg("Applied test scheme '%s'\n", pszSchemeName);
}

//-----------------------------------------------------------------------------
// Console command implementations
//-----------------------------------------------------------------------------
void CC_GMod_ListSchemes(void)
{
    if (g_pGModSchemeSystem)
    {
        g_pGModSchemeSystem->ListSchemes();
    }
}

void CC_GMod_LoadScheme(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_load_scheme <scheme_name>\n");
        return;
    }

    const char* pszName = engine->Cmd_Argv(1);

    if (g_pGModSchemeSystem)
    {
        if (g_pGModSchemeSystem->LoadScheme(pszName))
        {
            Msg("Scheme '%s' loaded successfully\n", pszName);
        }
        else
        {
            Msg("Failed to load scheme '%s'\n", pszName);
        }
    }
}

void CC_GMod_UnloadScheme(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_unload_scheme <scheme_name>\n");
        return;
    }

    const char* pszName = engine->Cmd_Argv(1);

    if (g_pGModSchemeSystem)
    {
        if (g_pGModSchemeSystem->UnloadScheme(pszName))
        {
            Msg("Scheme '%s' unloaded successfully\n", pszName);
        }
        else
        {
            Msg("Failed to unload scheme '%s'\n", pszName);
        }
    }
}

void CC_GMod_SetScheme(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_set_scheme <scheme_name>\n");
        return;
    }

    const char* pszName = engine->Cmd_Argv(1);

    if (g_pGModSchemeSystem)
    {
        g_pGModSchemeSystem->ApplyGlobalScheme(pszName);
    }
}

void CC_GMod_ReloadSchemes(void)
{
    if (g_pGModSchemeSystem)
    {
        g_pGModSchemeSystem->ReloadAllSchemes();
    }
}

void CC_GMod_SchemeInfo(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_scheme_info <scheme_name>\n");
        return;
    }

    const char* pszName = engine->Cmd_Argv(1);

    if (g_pGModSchemeSystem)
    {
        g_pGModSchemeSystem->ShowSchemeInfo(pszName);
    }
}

void CC_GMod_TestScheme(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_test_scheme <scheme_name>\n");
        return;
    }

    const char* pszName = engine->Cmd_Argv(1);

    if (g_pGModSchemeSystem)
    {
        g_pGModSchemeSystem->TestScheme(pszName);
    }
}