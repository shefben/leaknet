//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Color System - GMod 9.0.4b compatible implementation
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_COLOR_H
#define GMOD_COLOR_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "Color.h"

//-----------------------------------------------------------------------------
// Color Component Types
//-----------------------------------------------------------------------------
enum ColorComponent_t
{
    COLOR_RED = 0,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_ALPHA,
    COLOR_RENDERMODE,
    COLOR_RENDERFX,

    COLOR_COMPONENT_COUNT
};

//-----------------------------------------------------------------------------
// GMod Color System - Manages material color settings
//-----------------------------------------------------------------------------
class CGModColorSystem
{
public:
    // System initialization
    static void Initialize();
    static void Shutdown();

    // Color management
    static void SetColorComponent(ColorComponent_t component, float value);
    static float GetColorComponent(ColorComponent_t component);

    static void SetColor(float r, float g, float b, float a = 1.0f);
    static Color GetColor();

    static void SetRenderMode(int renderMode);
    static int GetRenderMode();

    static void SetRenderFX(int renderFX);
    static int GetRenderFX();

    // Apply colors to entities
    static void ApplyColorToEntity(CBaseEntity* pEntity);
    static void ApplyColorToEntity(CBaseEntity* pEntity, const Color& color, int renderMode = 0, int renderFX = 0);

    // Color presets
    static void SetColorPreset(const char* presetName);
    static void SaveColorPreset(const char* presetName);
    static void LoadColorPreset(const char* presetName);

    // Console command handlers
    static void CMD_gm_color_apply(void);
    static void CMD_gm_color_reset(void);
    static void CMD_gm_color_save(void);
    static void CMD_gm_color_load(void);

    // Utility functions
    static void UpdateFromConVars();
    static const char* GetColorComponentName(ColorComponent_t component);

private:
    static float m_flColorComponents[COLOR_COMPONENT_COUNT];
    static bool m_bInitialized;

    // Internal functions
    static void ResetToDefaults();
    static void ClampColorValues();
    static void ConVarCallback(ColorComponent_t component);

    // Console variable callbacks
    static void RedCallback(ConVar *var, const char *pOldString);
    static void GreenCallback(ConVar *var, const char *pOldString);
    static void BlueCallback(ConVar *var, const char *pOldString);
    static void AlphaCallback(ConVar *var, const char *pOldString);
    static void RenderModeCallback(ConVar *var, const char *pOldString);
    static void RenderFXCallback(ConVar *var, const char *pOldString);
};

// Console variables for color system
extern ConVar gm_colourset_r;
extern ConVar gm_colourset_g;
extern ConVar gm_colourset_b;
extern ConVar gm_colourset_a;
extern ConVar gm_colourset_rm;
extern ConVar gm_colourset_fx;

#endif // GMOD_COLOR_H