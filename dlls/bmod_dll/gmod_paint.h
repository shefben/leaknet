#ifndef GMOD_PAINT_H
#define GMOD_PAINT_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "utlvector.h"
#include "utlstring.h"

// Forward declarations
class CBaseEntity;
class CBasePlayer;

// Paint splash types discovered from IDA
enum PaintSplatType_t
{
    PAINT_SPLAT_BLUE = 0,
    PAINT_SPLAT_GREEN,
    PAINT_SPLAT_PINK,
    PAINT_SPLAT_MAX
};

// Paint data structure based on IDA member analysis
struct PaintData_t
{
    Vector vecPaintCursor;      // m_vecPaintCursor - Current paint cursor position
    float flPaintTime;          // m_flPaintTime - Last paint time
    Vector vecPaintStart;       // m_vecPaintStart - Paint start position
    float flPainTime;           // m_painTime - Pain time (legacy typo from IDA)

    bool bCanPaint;             // Permission to paint
    float flNextPaintTime;      // Next allowed paint time
    int iPaintCount;            // Number of paints used

    PaintData_t()
    {
        vecPaintCursor.Init();
        flPaintTime = 0.0f;
        vecPaintStart.Init();
        flPainTime = 0.0f;
        bCanPaint = true;
        flNextPaintTime = 0.0f;
        iPaintCount = 0;
    }
};

// Paint configuration data loaded from settings/gmod_paint.txt
struct PaintConfig_t
{
    CUtlString materialName;
    CUtlString soundName;
    float flDecalSize;
    float flDecalDuration;
    Color paintColor;
    bool bEnabled;

    PaintConfig_t()
    {
        flDecalSize = 64.0f;
        flDecalDuration = 60.0f;
        paintColor = Color(255, 255, 255, 255);
        bEnabled = true;
    }
};

//-----------------------------------------------------------------------------
// GMod Paint System - Implements player decal/spray functionality
// Based on IDA analysis: gmod_paint.txt, _PlayerAllowDecalPaint, GmPaint
//-----------------------------------------------------------------------------
class CGModPaintSystem : public CAutoGameSystem
{
public:
    CGModPaintSystem() : CAutoGameSystem("GmPaint") {}

    // CAutoGameSystem overrides
    virtual bool Init() override;
    virtual void Shutdown() override;
    virtual void LevelInitPostEntity() override;

    // Paint system functions discovered from IDA
    static bool LoadPaintConfig(); // Loads settings/gmod_paint.txt
    static bool CanPlayerPaint(CBasePlayer* pPlayer);
    static void AllowPlayerPaint(CBasePlayer* pPlayer, bool bAllow = true);
    static bool CreatePaintDecal(CBasePlayer* pPlayer, const Vector& origin, const Vector& normal, PaintSplatType_t type);
    static void UpdatePaintCursor(CBasePlayer* pPlayer, const Vector& cursor);
    static void SetPaintMode(CBasePlayer* pPlayer, bool bEnabled);

    // Paint splash effects (PaintSplatBlue, PaintSplatGreen, PaintSplatPink)
    static void CreatePaintSplash(const Vector& origin, const Vector& normal, PaintSplatType_t type);
    static void PlayPaintSound(CBasePlayer* pPlayer, const Vector& origin);

    // Player paint data management
    static PaintData_t* GetPlayerPaintData(CBasePlayer* pPlayer);
    static void ResetPlayerPaintData(CBasePlayer* pPlayer);
    static void UpdatePlayerPaintTime(CBasePlayer* pPlayer);

    // Utility functions
    static const char* GetPaintSplatName(PaintSplatType_t type);
    static Color GetPaintSplatColor(PaintSplatType_t type);
    static float GetPaintCooldownTime() { return 30.0f; } // 30 second cooldown discovered in IDA

private:
    static CUtlVector<PaintConfig_t> s_PaintConfigs;
    static CUtlVector<PaintData_t> s_PlayerPaintData;
    static bool s_bSystemInitialized;
    static bool s_bPaintConfigLoaded;
    static bool s_bGlobalPaintMode;

    // Helper functions
    static bool ParsePaintConfig(const char* pszConfigPath);
    static int GetPlayerPaintIndex(CBasePlayer* pPlayer);
    static void EnsurePlayerPaintData(CBasePlayer* pPlayer);
    static bool ValidatePaintLocation(const Vector& origin, const Vector& normal);
};

// Global instance
extern CGModPaintSystem g_GMod_PaintSystem;

// Console command handlers discovered from IDA analysis
void CMD_gm_paintmode(void);               // Ggm_paintmode - Toggle paint mode
void CMD_gmod_allowpaint(void);            // _PlayerAllowDecalPaint wrapper

#endif // GMOD_PAINT_H