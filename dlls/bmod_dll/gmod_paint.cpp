#include "cbase.h"
#include "gmod_paint.h"
#include "player.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "te_effect_dispatch.h"
#include "util.h"
#include "gmod_lua.h"

// ConVar and ConCommand registration
static ConCommand gm_paintmode("gm_paintmode", CMD_gm_paintmode, "Toggle paint mode on/off");
static ConCommand gmod_allowpaint("gmod_allowpaint", CMD_gmod_allowpaint, "Allow player to paint immediately");

// Static member initialization
CUtlVector<PaintConfig_t> CGModPaintSystem::s_PaintConfigs;
CUtlVector<PaintData_t> CGModPaintSystem::s_PlayerPaintData;
bool CGModPaintSystem::s_bSystemInitialized = false;
bool CGModPaintSystem::s_bPaintConfigLoaded = false;
bool CGModPaintSystem::s_bGlobalPaintMode = true;

// Global instance
CGModPaintSystem g_GMod_PaintSystem;

//-----------------------------------------------------------------------------
// Purpose: Initialize the paint system
//-----------------------------------------------------------------------------
bool CGModPaintSystem::Init()
{
    if (s_bSystemInitialized)
        return true;

    Msg("Initializing GMod Paint System...\n");

    s_PaintConfigs.Purge();
    s_PlayerPaintData.Purge();

    // Load paint configuration from settings/gmod_paint.txt
    if (!LoadPaintConfig())
    {
        Warning("GmPaint: Failed to load paint configuration\n");
    }

    s_bSystemInitialized = true;
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown the paint system
//-----------------------------------------------------------------------------
void CGModPaintSystem::Shutdown()
{
    if (!s_bSystemInitialized)
        return;

    s_PaintConfigs.Purge();
    s_PlayerPaintData.Purge();
    s_bSystemInitialized = false;
    s_bPaintConfigLoaded = false;
}

//-----------------------------------------------------------------------------
// Purpose: Level initialization
//-----------------------------------------------------------------------------
void CGModPaintSystem::LevelInitPostEntity()
{
    // Reset all player paint data for new level
    s_PlayerPaintData.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Load paint configuration from settings/gmod_paint.txt
//-----------------------------------------------------------------------------
bool CGModPaintSystem::LoadPaintConfig()
{
    const char* pszConfigPath = "settings/gmod_paint.txt";

    if (!filesystem->FileExists(pszConfigPath, "GAME"))
    {
        Warning("Error loading 'gmod_paint.txt'\n");
        return false;
    }

    return ParsePaintConfig(pszConfigPath);
}

//-----------------------------------------------------------------------------
// Purpose: Parse paint configuration file
//-----------------------------------------------------------------------------
bool CGModPaintSystem::ParsePaintConfig(const char* pszConfigPath)
{
    KeyValues* pKV = new KeyValues("PaintConfig");
    if (!pKV->LoadFromFile(filesystem, pszConfigPath, "GAME"))
    {
        pKV->deleteThis();
        return false;
    }

    s_PaintConfigs.Purge();

    // Default paint configurations based on IDA discovered types
    PaintConfig_t blueConfig;
    blueConfig.materialName = "sprites/paintsplat_blue";
    blueConfig.soundName = "SprayCan.Paint";
    blueConfig.paintColor = Color(0, 100, 255, 255);
    s_PaintConfigs.AddToTail(blueConfig);

    PaintConfig_t greenConfig;
    greenConfig.materialName = "sprites/paintsplat_green";
    greenConfig.soundName = "SprayCan.Paint";
    greenConfig.paintColor = Color(0, 255, 100, 255);
    s_PaintConfigs.AddToTail(greenConfig);

    PaintConfig_t pinkConfig;
    pinkConfig.materialName = "sprites/paintsplat_pink";
    pinkConfig.soundName = "SprayCan.Paint";
    pinkConfig.paintColor = Color(255, 100, 200, 255);
    s_PaintConfigs.AddToTail(pinkConfig);

    // Parse additional configurations from file
    FOR_EACH_SUBKEY(pKV, pSubKey)
    {
        PaintConfig_t config;
        config.materialName = pSubKey->GetString("material", "sprites/paintsplat");
        config.soundName = pSubKey->GetString("sound", "SprayCan.Paint");
        config.flDecalSize = pSubKey->GetFloat("size", 64.0f);
        config.flDecalDuration = pSubKey->GetFloat("duration", 60.0f);
        config.bEnabled = pSubKey->GetBool("enabled", true);

        // Parse color
        const char* pszColor = pSubKey->GetString("color", "255 255 255 255");
        int r, g, b, a;
        if (sscanf(pszColor, "%d %d %d %d", &r, &g, &b, &a) == 4)
        {
            config.paintColor = Color(r, g, b, a);
        }

        s_PaintConfigs.AddToTail(config);
    }

    pKV->deleteThis();
    s_bPaintConfigLoaded = true;
    DevMsg("GmPaint: Loaded %d paint configurations\n", s_PaintConfigs.Count());
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Check if player can paint
//-----------------------------------------------------------------------------
bool CGModPaintSystem::CanPlayerPaint(CBasePlayer* pPlayer)
{
    if (!pPlayer || !s_bSystemInitialized || !s_bGlobalPaintMode)
        return false;

    PaintData_t* pData = GetPlayerPaintData(pPlayer);
    if (!pData)
        return false;

    // Check cooldown timer
    if (gpGlobals->curtime < pData->flNextPaintTime)
        return false;

    return pData->bCanPaint;
}

//-----------------------------------------------------------------------------
// Purpose: Allow or disallow player to paint
//-----------------------------------------------------------------------------
void CGModPaintSystem::AllowPlayerPaint(CBasePlayer* pPlayer, bool bAllow)
{
    if (!pPlayer)
        return;

    PaintData_t* pData = GetPlayerPaintData(pPlayer);
    if (!pData)
        return;

    pData->bCanPaint = bAllow;

    if (bAllow)
    {
        // Reset cooldown when explicitly allowing
        pData->flNextPaintTime = 0.0f;
        DevMsg("Player %s is now allowed to paint\n", STRING(pPlayer->pl.netname));
    }
    else
    {
        DevMsg("Player %s is no longer allowed to paint\n", STRING(pPlayer->pl.netname));
    }
}

//-----------------------------------------------------------------------------
// Purpose: Create paint decal at location
//-----------------------------------------------------------------------------
bool CGModPaintSystem::CreatePaintDecal(CBasePlayer* pPlayer, const Vector& origin, const Vector& normal, PaintSplatType_t type)
{
    if (!CanPlayerPaint(pPlayer))
        return false;

    if (type < 0 || type >= PAINT_SPLAT_MAX)
        type = PAINT_SPLAT_BLUE;

    if (!ValidatePaintLocation(origin, normal))
        return false;

    // Create the decal
    const char* pszDecalName = GetPaintSplatName(type);
    UTIL_DecalTrace(origin, origin + normal * 64.0f, pszDecalName);

    // Create splash effect
    CreatePaintSplash(origin, normal, type);

    // Play sound
    PlayPaintSound(pPlayer, origin);

    // Update player paint data
    PaintData_t* pData = GetPlayerPaintData(pPlayer);
    if (pData)
    {
        pData->vecPaintCursor = origin;
        pData->flPaintTime = gpGlobals->curtime;
        pData->iPaintCount++;
        pData->flNextPaintTime = gpGlobals->curtime + GetPaintCooldownTime();
        UpdatePlayerPaintTime(pPlayer);
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Update paint cursor position
//-----------------------------------------------------------------------------
void CGModPaintSystem::UpdatePaintCursor(CBasePlayer* pPlayer, const Vector& cursor)
{
    PaintData_t* pData = GetPlayerPaintData(pPlayer);
    if (pData)
    {
        pData->vecPaintCursor = cursor;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Set paint mode for player
//-----------------------------------------------------------------------------
void CGModPaintSystem::SetPaintMode(CBasePlayer* pPlayer, bool bEnabled)
{
    PaintData_t* pData = GetPlayerPaintData(pPlayer);
    if (pData)
    {
        pData->bCanPaint = bEnabled;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Create paint splash effect
//-----------------------------------------------------------------------------
void CGModPaintSystem::CreatePaintSplash(const Vector& origin, const Vector& normal, PaintSplatType_t type)
{
    // Create particle effect based on paint type
    CEffectData data;
    data.m_vOrigin = origin;
    data.m_vNormal = normal;
    data.m_nColor = GetPaintSplatColor(type).GetRawColor();

    const char* pszEffectName = NULL;
    switch (type)
    {
        case PAINT_SPLAT_BLUE:
            pszEffectName = "PaintSplatBlue";
            break;
        case PAINT_SPLAT_GREEN:
            pszEffectName = "PaintSplatGreen";
            break;
        case PAINT_SPLAT_PINK:
            pszEffectName = "PaintSplatPink";
            break;
        default:
            pszEffectName = "PaintSplatBlue";
            break;
    }

    DispatchEffect(pszEffectName, data);
}

//-----------------------------------------------------------------------------
// Purpose: Play paint sound effect
//-----------------------------------------------------------------------------
void CGModPaintSystem::PlayPaintSound(CBasePlayer* pPlayer, const Vector& origin)
{
    if (!pPlayer)
        return;

    // Play SprayCan.Paint sound discovered in IDA
    pPlayer->EmitSound("SprayCan.Paint");
}

//-----------------------------------------------------------------------------
// Purpose: Get player paint data
//-----------------------------------------------------------------------------
PaintData_t* CGModPaintSystem::GetPlayerPaintData(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return NULL;

    EnsurePlayerPaintData(pPlayer);

    int index = GetPlayerPaintIndex(pPlayer);
    if (index != -1)
        return &s_PlayerPaintData[index];

    return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Reset player paint data
//-----------------------------------------------------------------------------
void CGModPaintSystem::ResetPlayerPaintData(CBasePlayer* pPlayer)
{
    int index = GetPlayerPaintIndex(pPlayer);
    if (index != -1)
    {
        s_PlayerPaintData.Remove(index);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Update player paint time
//-----------------------------------------------------------------------------
void CGModPaintSystem::UpdatePlayerPaintTime(CBasePlayer* pPlayer)
{
    PaintData_t* pData = GetPlayerPaintData(pPlayer);
    if (pData)
    {
        pData->flPaintTime = gpGlobals->curtime;
        pData->flPainTime = gpGlobals->curtime; // Legacy typo member
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get paint splat name
//-----------------------------------------------------------------------------
const char* CGModPaintSystem::GetPaintSplatName(PaintSplatType_t type)
{
    switch (type)
    {
        case PAINT_SPLAT_BLUE:  return "PaintSplatBlue";
        case PAINT_SPLAT_GREEN: return "PaintSplatGreen";
        case PAINT_SPLAT_PINK:  return "PaintSplatPink";
        default:                return "PaintSplatBlue";
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get paint splat color
//-----------------------------------------------------------------------------
Color CGModPaintSystem::GetPaintSplatColor(PaintSplatType_t type)
{
    switch (type)
    {
        case PAINT_SPLAT_BLUE:  return Color(0, 100, 255, 255);
        case PAINT_SPLAT_GREEN: return Color(0, 255, 100, 255);
        case PAINT_SPLAT_PINK:  return Color(255, 100, 200, 255);
        default:                return Color(0, 100, 255, 255);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get player paint index
//-----------------------------------------------------------------------------
int CGModPaintSystem::GetPlayerPaintIndex(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return -1;

    int playerIndex = pPlayer->entindex();
    for (int i = 0; i < s_PlayerPaintData.Count(); i++)
    {
        // Use memory address comparison as unique identifier
        if (&s_PlayerPaintData[i] == GetPlayerPaintData(pPlayer))
            return i;
    }

    return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Ensure player paint data exists
//-----------------------------------------------------------------------------
void CGModPaintSystem::EnsurePlayerPaintData(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    int playerIndex = pPlayer->entindex();

    // Check if data already exists
    for (int i = 0; i < s_PlayerPaintData.Count(); i++)
    {
        // Simple check - expand array if needed
        if (s_PlayerPaintData.Count() <= playerIndex)
        {
            s_PlayerPaintData.SetSize(playerIndex + 1);
            break;
        }
    }

    // Ensure we have enough entries
    while (s_PlayerPaintData.Count() <= playerIndex)
    {
        PaintData_t newData;
        s_PlayerPaintData.AddToTail(newData);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Validate paint location
//-----------------------------------------------------------------------------
bool CGModPaintSystem::ValidatePaintLocation(const Vector& origin, const Vector& normal)
{
    // Basic validation - ensure normal is valid
    if (normal.Length() < 0.1f)
        return false;

    // Could add more validation here (distance checks, surface type, etc.)
    return true;
}

//=============================================================================
// Console Commands
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Toggle paint mode - Ggm_paintmode command from IDA
//-----------------------------------------------------------------------------
void CMD_gm_paintmode(void)
{
    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() > 1)
    {
        bool bEnabled = atoi(engine->Cmd_Argv(1)) != 0;
        CGModPaintSystem::SetPaintMode(pPlayer, bEnabled);
        Msg("Paint mode %s for %s\n", bEnabled ? "enabled" : "disabled", STRING(pPlayer->pl.netname));
    }
    else
    {
        PaintData_t* pData = CGModPaintSystem::GetPlayerPaintData(pPlayer);
        if (pData)
        {
            bool bNewMode = !pData->bCanPaint;
            CGModPaintSystem::SetPaintMode(pPlayer, bNewMode);
            Msg("Paint mode %s for %s\n", bNewMode ? "enabled" : "disabled", STRING(pPlayer->pl.netname));
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Allow player to paint immediately - _PlayerAllowDecalPaint wrapper
//-----------------------------------------------------------------------------
void CMD_gmod_allowpaint(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Wrong syntax used on PlayerAllowDecalPaint\n");
        Msg("Allow the player to spraypaint now rather than waiting the 30 or so seconds. Syntax: <playerid> \n");
        return;
    }

    int playerid = atoi(engine->Cmd_Argv(1));
    CBasePlayer* pPlayer = UTIL_PlayerByIndex(playerid);

    if (!pPlayer)
    {
        Msg("Invalid player ID: %d\n", playerid);
        return;
    }

    CGModPaintSystem::AllowPlayerPaint(pPlayer, true);
    Msg("Player %s is now allowed to paint\n", STRING(pPlayer->pl.netname));
}