#include "cbase.h"
#include "gmod_overlay.h"
#include "player.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Console variables - based on IDA strings
ConVar r_screenoverlay("r_screenoverlay", "", FCVAR_NONE, "Screen overlay material");

// Console commands
ConCommand gmod_list_overlays("gmod_list_overlays", CC_GMod_ListOverlays, "List all registered overlays");
ConCommand gmod_show_overlay("gmod_show_overlay", CC_GMod_ShowOverlay, "Show an overlay by name");
ConCommand gmod_hide_overlay("gmod_hide_overlay", CC_GMod_HideOverlay, "Hide current overlay");
ConCommand gmod_reload_overlays("gmod_reload_overlays", CC_GMod_ReloadOverlays, "Reload overlay settings");

// Global instance
CGModOverlaySystem g_GMod_OverlaySystem;
CGModOverlaySystem* g_pGModOverlaySystem = &g_GMod_OverlaySystem;

//-----------------------------------------------------------------------------
// CGModOverlaySystem implementation
//-----------------------------------------------------------------------------
CGModOverlaySystem::CGModOverlaySystem() : CAutoGameSystem()
{
    m_szGlobalOverlayMaterial[0] = '\0';
    m_pGlobalOverlayMaterial = NULL;
    m_bGlobalOverlayActive = false;

    SetDefLessFunc(m_PlayerOverlayStates);
}

CGModOverlaySystem::~CGModOverlaySystem()
{
    Shutdown();
}

bool CGModOverlaySystem::Init()
{
    return true;
}

void CGModOverlaySystem::Shutdown()
{
    // Clear all player overlay states
    FOR_EACH_MAP(m_PlayerOverlayStates, i)
    {
        PlayerOverlayState_t& state = m_PlayerOverlayStates[i];
        if (state.pOverlayMaterial)
        {
            state.pOverlayMaterial->DecrementReferenceCount();
            state.pOverlayMaterial = NULL;
        }
    }
    m_PlayerOverlayStates.RemoveAll();

    // Clear global overlay
    if (m_pGlobalOverlayMaterial)
    {
        m_pGlobalOverlayMaterial->DecrementReferenceCount();
        m_pGlobalOverlayMaterial = NULL;
    }

    m_RegisteredOverlays.RemoveAll();
}

void CGModOverlaySystem::LevelInitPostEntity()
{
    LoadOverlaySettings();
}

void CGModOverlaySystem::FrameUpdatePreEntityThink()
{
    UpdatePlayerOverlays();
}

// Implementation based on GMod_LoadOverlaySettings function found in IDA (0x2413164d)
bool CGModOverlaySystem::LoadOverlaySettings()
{
    // Based on IDA string: "settings/gmod_overlay.txt"
    const char* pszFilename = "settings/gmod_overlay.txt";

    // Check if file exists
    if (!filesystem->FileExists(pszFilename, "GAME"))
    {
        // Based on IDA string: "Error! gmod_overlay.txt couldn't be loaded!"
        DevMsg("Error! gmod_overlay.txt couldn't be loaded!\n");
        return false;
    }

    KeyValues* pKV = new KeyValues("GMod_Overlays");
    if (!pKV->LoadFromFile(filesystem, pszFilename, "GAME"))
    {
        DevMsg("Failed to parse gmod_overlay.txt\n");
        pKV->deleteThis();
        return false;
    }

    // Clear existing overlays
    m_RegisteredOverlays.RemoveAll();

    // Parse overlay definitions
    ParseOverlayFile(pKV);

    pKV->deleteThis();
    DevMsg("Loaded %d overlay definitions from gmod_overlay.txt\n", m_RegisteredOverlays.Count());
    return true;
}

void CGModOverlaySystem::ReloadOverlaySettings()
{
    LoadOverlaySettings();
}

void CGModOverlaySystem::ParseOverlayFile(KeyValues* pKV)
{
    // Parse overlay entries
    for (KeyValues* pOverlay = pKV->GetFirstSubKey(); pOverlay; pOverlay = pOverlay->GetNextKey())
    {
        const char* pszName = pOverlay->GetName();
        const char* pszMaterial = pOverlay->GetString("material", "");
        float flDuration = pOverlay->GetFloat("duration", 5.0f);
        float flFadeIn = pOverlay->GetFloat("fadein", 0.5f);
        float flFadeOut = pOverlay->GetFloat("fadeout", 0.5f);
        int iFlags = 0;

        // Parse flags
        if (pOverlay->GetInt("fullscreen", 1))
            iFlags |= OVERLAY_FLAG_FULLSCREEN;
        if (pOverlay->GetInt("additive", 0))
            iFlags |= OVERLAY_FLAG_ADDITIVE;
        if (pOverlay->GetInt("alphablend", 1))
            iFlags |= OVERLAY_FLAG_ALPHA_BLEND;
        if (pOverlay->GetInt("worldspace", 0))
            iFlags |= OVERLAY_FLAG_WORLD_SPACE;

        RegisterOverlay(pszName, pszMaterial, flDuration, flFadeIn, flFadeOut, iFlags);
    }
}

bool CGModOverlaySystem::RegisterOverlay(const char* pszName, const char* pszMaterialPath, float flDuration, float flFadeIn, float flFadeOut, int iFlags)
{
    if (!pszName || !pszMaterialPath)
        return false;

    OverlayData_t overlayData;
    Q_strncpy(overlayData.szName, pszName, sizeof(overlayData.szName));
    Q_strncpy(overlayData.szMaterialPath, pszMaterialPath, sizeof(overlayData.szMaterialPath));
    overlayData.flDuration = flDuration;
    overlayData.flFadeIn = flFadeIn;
    overlayData.flFadeOut = flFadeOut;
    overlayData.iFlags = iFlags;
    overlayData.bActive = false;

    m_RegisteredOverlays.AddToTail(overlayData);
    return true;
}

OverlayData_t* CGModOverlaySystem::GetOverlayByName(const char* pszName)
{
    if (!pszName)
        return NULL;

    for (int i = 0; i < m_RegisteredOverlays.Count(); i++)
    {
        if (Q_stricmp(m_RegisteredOverlays[i].szName, pszName) == 0)
            return &m_RegisteredOverlays[i];
    }

    return NULL;
}

void CGModOverlaySystem::SetPlayerOverlay(CBasePlayer* pPlayer, const char* pszOverlayName, float flDuration)
{
    if (!pPlayer || !pszOverlayName)
        return;

    OverlayData_t* pOverlayData = GetOverlayByName(pszOverlayName);
    if (!pOverlayData)
    {
        DevMsg("Overlay '%s' not found\n", pszOverlayName);
        return;
    }

    PlayerOverlayState_t* pState = GetPlayerOverlayState(pPlayer);
    if (!pState)
        return;

    // Clean up previous overlay material
    if (pState->pOverlayMaterial)
    {
        pState->pOverlayMaterial->DecrementReferenceCount();
        pState->pOverlayMaterial = NULL;
    }

    // Set new overlay
    pState->currentOverlay = *pOverlayData;
    pState->flOverlayStartTime = gpGlobals->curtime;
    pState->flOverlayEndTime = (flDuration > 0) ? gpGlobals->curtime + flDuration : gpGlobals->curtime + pOverlayData->flDuration;
    pState->pOverlayMaterial = LoadOverlayMaterial(pOverlayData->szMaterialPath);
    pState->bOverlayActive = true;

    DevMsg("Set overlay '%s' for player %d\n", pszOverlayName, pPlayer->entindex());
}

void CGModOverlaySystem::ClearPlayerOverlay(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    PlayerOverlayState_t* pState = GetPlayerOverlayState(pPlayer);
    if (!pState)
        return;

    if (pState->pOverlayMaterial)
    {
        pState->pOverlayMaterial->DecrementReferenceCount();
        pState->pOverlayMaterial = NULL;
    }

    pState->bOverlayActive = false;
}

void CGModOverlaySystem::UpdatePlayerOverlays()
{
    // Update all player overlays
    FOR_EACH_MAP(m_PlayerOverlayStates, i)
    {
        PlayerOverlayState_t& state = m_PlayerOverlayStates[i];

        if (!state.bOverlayActive)
            continue;

        // Check if overlay has expired
        if (state.flOverlayEndTime > 0 && gpGlobals->curtime >= state.flOverlayEndTime)
        {
            if (state.pOverlayMaterial)
            {
                state.pOverlayMaterial->DecrementReferenceCount();
                state.pOverlayMaterial = NULL;
            }
            state.bOverlayActive = false;
        }
    }

    // Clean up disconnected players
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
        if (!pPlayer)
        {
            CleanupPlayerOverlayState(i);
        }
    }
}

PlayerOverlayState_t* CGModOverlaySystem::GetPlayerOverlayState(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return NULL;

    int playerIndex = pPlayer->entindex();
    int index = m_PlayerOverlayStates.Find(playerIndex);

    if (index == m_PlayerOverlayStates.InvalidIndex())
    {
        PlayerOverlayState_t defaultState;
        index = m_PlayerOverlayStates.Insert(playerIndex, defaultState);
    }

    return &m_PlayerOverlayStates[index];
}

void CGModOverlaySystem::CleanupPlayerOverlayState(int playerIndex)
{
    int index = m_PlayerOverlayStates.Find(playerIndex);
    if (index != m_PlayerOverlayStates.InvalidIndex())
    {
        PlayerOverlayState_t& state = m_PlayerOverlayStates[index];
        if (state.pOverlayMaterial)
        {
            state.pOverlayMaterial->DecrementReferenceCount();
            state.pOverlayMaterial = NULL;
        }
        m_PlayerOverlayStates.Remove(playerIndex);
    }
}

IMaterial* CGModOverlaySystem::LoadOverlayMaterial(const char* pszMaterialPath)
{
    if (!pszMaterialPath || !pszMaterialPath[0])
        return NULL;

    IMaterial* pMaterial = materials->FindMaterial(pszMaterialPath, TEXTURE_GROUP_OTHER);
    if (pMaterial)
    {
        pMaterial->IncrementReferenceCount();
        return pMaterial;
    }

    DevMsg("Failed to load overlay material: %s\n", pszMaterialPath);
    return NULL;
}

void CGModOverlaySystem::SetGlobalOverlay(const char* pszMaterialPath)
{
    if (!pszMaterialPath)
        return;

    // Clean up previous global overlay
    if (m_pGlobalOverlayMaterial)
    {
        m_pGlobalOverlayMaterial->DecrementReferenceCount();
        m_pGlobalOverlayMaterial = NULL;
    }

    Q_strncpy(m_szGlobalOverlayMaterial, pszMaterialPath, sizeof(m_szGlobalOverlayMaterial));
    m_pGlobalOverlayMaterial = LoadOverlayMaterial(pszMaterialPath);
    m_bGlobalOverlayActive = (m_pGlobalOverlayMaterial != NULL);

    // Set the r_screenoverlay ConVar (based on IDA analysis)
    r_screenoverlay.SetValue(pszMaterialPath);
}

void CGModOverlaySystem::ClearGlobalOverlay()
{
    if (m_pGlobalOverlayMaterial)
    {
        m_pGlobalOverlayMaterial->DecrementReferenceCount();
        m_pGlobalOverlayMaterial = NULL;
    }

    m_szGlobalOverlayMaterial[0] = '\0';
    m_bGlobalOverlayActive = false;
    r_screenoverlay.SetValue("");
}

void CGModOverlaySystem::ListOverlays()
{
    Msg("Registered overlays (%d):\n", m_RegisteredOverlays.Count());
    for (int i = 0; i < m_RegisteredOverlays.Count(); i++)
    {
        const OverlayData_t& overlay = m_RegisteredOverlays[i];
        Msg("  %s: %s (duration: %.1f, fadein: %.1f, fadeout: %.1f, flags: %d)\n",
            overlay.szName, overlay.szMaterialPath, overlay.flDuration,
            overlay.flFadeIn, overlay.flFadeOut, overlay.iFlags);
    }
}

void CGModOverlaySystem::ShowOverlay(const char* pszName, float flDuration)
{
    if (!pszName)
        return;

    // Note: Server DLL doesn't have a "local player" concept
    // This function is intended for client-side or listen server use
    // For dedicated server, overlays should be applied per-player via SetPlayerOverlay
    DevMsg("ShowOverlay called on server DLL - use SetPlayerOverlay for specific players\n");
}

void CGModOverlaySystem::HideOverlay()
{
    // Note: Server DLL doesn't have a "local player" concept
    // This function is intended for client-side or listen server use
    // For dedicated server, overlays should be cleared per-player via ClearPlayerOverlay
    DevMsg("HideOverlay called on server DLL - use ClearPlayerOverlay for specific players\n");
}

//-----------------------------------------------------------------------------
// Console command implementations
//-----------------------------------------------------------------------------
void CC_GMod_ListOverlays(void)
{
    if (g_pGModOverlaySystem)
    {
        g_pGModOverlaySystem->ListOverlays();
    }
}

void CC_GMod_ShowOverlay(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_show_overlay <overlay_name> [duration]\n");
        return;
    }

    const char* pszName = engine->Cmd_Argv(1);
    float flDuration = -1.0f;

    if (engine->Cmd_Argc() >= 3)
    {
        flDuration = atof(engine->Cmd_Argv(2));
    }

    if (g_pGModOverlaySystem)
    {
        g_pGModOverlaySystem->ShowOverlay(pszName, flDuration);
    }
}

void CC_GMod_HideOverlay(void)
{
    if (g_pGModOverlaySystem)
    {
        g_pGModOverlaySystem->HideOverlay();
    }
}

void CC_GMod_ReloadOverlays(void)
{
    if (g_pGModOverlaySystem)
    {
        g_pGModOverlaySystem->ReloadOverlaySettings();
        Msg("Overlay settings reloaded.\n");
    }
}