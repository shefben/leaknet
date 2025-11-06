//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Color System Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_color.h"
#include "filesystem.h"
#include "tier1/strtools.h"
#include "tier1/KeyValues.h"

// Initialize static members
float CGModColorSystem::m_flColorComponents[COLOR_COMPONENT_COUNT] = {1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f};
bool CGModColorSystem::m_bInitialized = false;

// Forward declarations for callbacks
void RedConVarCallback(ConVar *var, const char *pOldString);
void GreenConVarCallback(ConVar *var, const char *pOldString);
void BlueConVarCallback(ConVar *var, const char *pOldString);
void AlphaConVarCallback(ConVar *var, const char *pOldString);
void RenderModeConVarCallback(ConVar *var, const char *pOldString);
void RenderFXConVarCallback(ConVar *var, const char *pOldString);

// Console variables for color system
ConVar gm_colourset_r("gm_colourset_r", "255", FCVAR_CLIENTDLL, "Red component (0-255)", RedConVarCallback);
ConVar gm_colourset_g("gm_colourset_g", "255", FCVAR_CLIENTDLL, "Green component (0-255)", GreenConVarCallback);
ConVar gm_colourset_b("gm_colourset_b", "255", FCVAR_CLIENTDLL, "Blue component (0-255)", BlueConVarCallback);
ConVar gm_colourset_a("gm_colourset_a", "255", FCVAR_CLIENTDLL, "Alpha component (0-255)", AlphaConVarCallback);
ConVar gm_colourset_rm("gm_colourset_rm", "0", FCVAR_CLIENTDLL, "Render mode (0-10)", RenderModeConVarCallback);
ConVar gm_colourset_fx("gm_colourset_fx", "0", FCVAR_CLIENTDLL, "Render FX (0-20)", RenderFXConVarCallback);

//-----------------------------------------------------------------------------
// Console variable callbacks
//-----------------------------------------------------------------------------
void RedConVarCallback(ConVar *var, const char *pOldString)
{
    CGModColorSystem::RedCallback(var, pOldString);
}

void GreenConVarCallback(ConVar *var, const char *pOldString)
{
    CGModColorSystem::GreenCallback(var, pOldString);
}

void BlueConVarCallback(ConVar *var, const char *pOldString)
{
    CGModColorSystem::BlueCallback(var, pOldString);
}

void AlphaConVarCallback(ConVar *var, const char *pOldString)
{
    CGModColorSystem::AlphaCallback(var, pOldString);
}

void RenderModeConVarCallback(ConVar *var, const char *pOldString)
{
    CGModColorSystem::RenderModeCallback(var, pOldString);
}

void RenderFXConVarCallback(ConVar *var, const char *pOldString)
{
    CGModColorSystem::RenderFXCallback(var, pOldString);
}

//-----------------------------------------------------------------------------
// Purpose: Initialize the color system
//-----------------------------------------------------------------------------
void CGModColorSystem::Initialize()
{
    if (m_bInitialized)
        return;

    ResetToDefaults();
    UpdateFromConVars();

    m_bInitialized = true;
    DevMsg("Color System initialized\n");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown the color system
//-----------------------------------------------------------------------------
void CGModColorSystem::Shutdown()
{
    if (!m_bInitialized)
        return;

    m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Reset color values to defaults
//-----------------------------------------------------------------------------
void CGModColorSystem::ResetToDefaults()
{
    m_flColorComponents[COLOR_RED] = 1.0f;
    m_flColorComponents[COLOR_GREEN] = 1.0f;
    m_flColorComponents[COLOR_BLUE] = 1.0f;
    m_flColorComponents[COLOR_ALPHA] = 1.0f;
    m_flColorComponents[COLOR_RENDERMODE] = 0.0f;
    m_flColorComponents[COLOR_RENDERFX] = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Update internal values from console variables
//-----------------------------------------------------------------------------
void CGModColorSystem::UpdateFromConVars()
{
    m_flColorComponents[COLOR_RED] = gm_colourset_r.GetFloat() / 255.0f;
    m_flColorComponents[COLOR_GREEN] = gm_colourset_g.GetFloat() / 255.0f;
    m_flColorComponents[COLOR_BLUE] = gm_colourset_b.GetFloat() / 255.0f;
    m_flColorComponents[COLOR_ALPHA] = gm_colourset_a.GetFloat() / 255.0f;
    m_flColorComponents[COLOR_RENDERMODE] = gm_colourset_rm.GetFloat();
    m_flColorComponents[COLOR_RENDERFX] = gm_colourset_fx.GetFloat();

    ClampColorValues();
}

//-----------------------------------------------------------------------------
// Purpose: Clamp color values to valid ranges
//-----------------------------------------------------------------------------
void CGModColorSystem::ClampColorValues()
{
    m_flColorComponents[COLOR_RED] = clamp(m_flColorComponents[COLOR_RED], 0.0f, 1.0f);
    m_flColorComponents[COLOR_GREEN] = clamp(m_flColorComponents[COLOR_GREEN], 0.0f, 1.0f);
    m_flColorComponents[COLOR_BLUE] = clamp(m_flColorComponents[COLOR_BLUE], 0.0f, 1.0f);
    m_flColorComponents[COLOR_ALPHA] = clamp(m_flColorComponents[COLOR_ALPHA], 0.0f, 1.0f);
    m_flColorComponents[COLOR_RENDERMODE] = clamp(m_flColorComponents[COLOR_RENDERMODE], 0.0f, 10.0f);
    m_flColorComponents[COLOR_RENDERFX] = clamp(m_flColorComponents[COLOR_RENDERFX], 0.0f, 20.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Set a color component value
//-----------------------------------------------------------------------------
void CGModColorSystem::SetColorComponent(ColorComponent_t component, float value)
{
    if (component >= COLOR_COMPONENT_COUNT)
        return;

    m_flColorComponents[component] = value;
    ClampColorValues();
}

//-----------------------------------------------------------------------------
// Purpose: Get a color component value
//-----------------------------------------------------------------------------
float CGModColorSystem::GetColorComponent(ColorComponent_t component)
{
    if (component >= COLOR_COMPONENT_COUNT)
        return 0.0f;

    return m_flColorComponents[component];
}

//-----------------------------------------------------------------------------
// Purpose: Set RGBA color
//-----------------------------------------------------------------------------
void CGModColorSystem::SetColor(float r, float g, float b, float a)
{
    SetColorComponent(COLOR_RED, r);
    SetColorComponent(COLOR_GREEN, g);
    SetColorComponent(COLOR_BLUE, b);
    SetColorComponent(COLOR_ALPHA, a);
}

//-----------------------------------------------------------------------------
// Purpose: Get current color
//-----------------------------------------------------------------------------
Color CGModColorSystem::GetColor()
{
    return Color(
        (int)(m_flColorComponents[COLOR_RED] * 255.0f),
        (int)(m_flColorComponents[COLOR_GREEN] * 255.0f),
        (int)(m_flColorComponents[COLOR_BLUE] * 255.0f),
        (int)(m_flColorComponents[COLOR_ALPHA] * 255.0f)
    );
}

//-----------------------------------------------------------------------------
// Purpose: Set render mode
//-----------------------------------------------------------------------------
void CGModColorSystem::SetRenderMode(int renderMode)
{
    SetColorComponent(COLOR_RENDERMODE, (float)renderMode);
}

//-----------------------------------------------------------------------------
// Purpose: Get render mode
//-----------------------------------------------------------------------------
int CGModColorSystem::GetRenderMode()
{
    return (int)GetColorComponent(COLOR_RENDERMODE);
}

//-----------------------------------------------------------------------------
// Purpose: Set render FX
//-----------------------------------------------------------------------------
void CGModColorSystem::SetRenderFX(int renderFX)
{
    SetColorComponent(COLOR_RENDERFX, (float)renderFX);
}

//-----------------------------------------------------------------------------
// Purpose: Get render FX
//-----------------------------------------------------------------------------
int CGModColorSystem::GetRenderFX()
{
    return (int)GetColorComponent(COLOR_RENDERFX);
}

//-----------------------------------------------------------------------------
// Purpose: Apply current color settings to entity
//-----------------------------------------------------------------------------
void CGModColorSystem::ApplyColorToEntity(CBaseEntity* pEntity)
{
    if (!pEntity)
        return;

    Color color = GetColor();
    int renderMode = GetRenderMode();
    int renderFX = GetRenderFX();

    ApplyColorToEntity(pEntity, color, renderMode, renderFX);
}

//-----------------------------------------------------------------------------
// Purpose: Apply specific color settings to entity
//-----------------------------------------------------------------------------
void CGModColorSystem::ApplyColorToEntity(CBaseEntity* pEntity, const Color& color, int renderMode, int renderFX)
{
    if (!pEntity)
        return;

    // Set entity color
    pEntity->SetRenderColor(color.r(), color.g(), color.b(), color.a());

    // Set render mode
    pEntity->SetRenderMode((RenderMode_t)renderMode);

    // Set render FX
    pEntity->SetRenderFX((RenderFx_t)renderFX);

    // Network the changes
    pEntity->NetworkStateChanged();
}

//-----------------------------------------------------------------------------
// Purpose: Get color component name
//-----------------------------------------------------------------------------
const char* CGModColorSystem::GetColorComponentName(ColorComponent_t component)
{
    switch (component)
    {
        case COLOR_RED: return "Red";
        case COLOR_GREEN: return "Green";
        case COLOR_BLUE: return "Blue";
        case COLOR_ALPHA: return "Alpha";
        case COLOR_RENDERMODE: return "Render Mode";
        case COLOR_RENDERFX: return "Render FX";
        default: return "Unknown";
    }
}

//-----------------------------------------------------------------------------
// Purpose: Console variable callback handling
//-----------------------------------------------------------------------------
void CGModColorSystem::ConVarCallback(ColorComponent_t component)
{
    if (!m_bInitialized)
        return;

    UpdateFromConVars();
}

void CGModColorSystem::RedCallback(ConVar *var, const char *pOldString)
{
    ConVarCallback(COLOR_RED);
}

void CGModColorSystem::GreenCallback(ConVar *var, const char *pOldString)
{
    ConVarCallback(COLOR_GREEN);
}

void CGModColorSystem::BlueCallback(ConVar *var, const char *pOldString)
{
    ConVarCallback(COLOR_BLUE);
}

void CGModColorSystem::AlphaCallback(ConVar *var, const char *pOldString)
{
    ConVarCallback(COLOR_ALPHA);
}

void CGModColorSystem::RenderModeCallback(ConVar *var, const char *pOldString)
{
    ConVarCallback(COLOR_RENDERMODE);
}

void CGModColorSystem::RenderFXCallback(ConVar *var, const char *pOldString)
{
    ConVarCallback(COLOR_RENDERFX);
}

//-----------------------------------------------------------------------------
// Purpose: Set color preset
//-----------------------------------------------------------------------------
void CGModColorSystem::SetColorPreset(const char* presetName)
{
    if (!presetName || !presetName[0])
        return;

    // Common color presets
    if (Q_stricmp(presetName, "red") == 0)
    {
        gm_colourset_r.SetValue(255);
        gm_colourset_g.SetValue(0);
        gm_colourset_b.SetValue(0);
    }
    else if (Q_stricmp(presetName, "green") == 0)
    {
        gm_colourset_r.SetValue(0);
        gm_colourset_g.SetValue(255);
        gm_colourset_b.SetValue(0);
    }
    else if (Q_stricmp(presetName, "blue") == 0)
    {
        gm_colourset_r.SetValue(0);
        gm_colourset_g.SetValue(0);
        gm_colourset_b.SetValue(255);
    }
    else if (Q_stricmp(presetName, "white") == 0)
    {
        gm_colourset_r.SetValue(255);
        gm_colourset_g.SetValue(255);
        gm_colourset_b.SetValue(255);
    }
    else if (Q_stricmp(presetName, "black") == 0)
    {
        gm_colourset_r.SetValue(0);
        gm_colourset_g.SetValue(0);
        gm_colourset_b.SetValue(0);
    }
    else if (Q_stricmp(presetName, "transparent") == 0)
    {
        gm_colourset_a.SetValue(128);
        gm_colourset_rm.SetValue(4); // kRenderTransTexture
    }
    else
    {
        Msg("Unknown color preset: %s\n", presetName);
        Msg("Available presets: red, green, blue, white, black, transparent\n");
    }
}

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Apply current color to targeted entity
//-----------------------------------------------------------------------------
void CGModColorSystem::CMD_gm_color_apply(void)
{
    Msg("Current color: R=%d G=%d B=%d A=%d RM=%d FX=%d\n",
        gm_colourset_r.GetInt(), gm_colourset_g.GetInt(), gm_colourset_b.GetInt(),
        gm_colourset_a.GetInt(), gm_colourset_rm.GetInt(), gm_colourset_fx.GetInt());
}

//-----------------------------------------------------------------------------
// Purpose: Reset color to defaults
//-----------------------------------------------------------------------------
void CGModColorSystem::CMD_gm_color_reset(void)
{
    gm_colourset_r.SetValue(255);
    gm_colourset_g.SetValue(255);
    gm_colourset_b.SetValue(255);
    gm_colourset_a.SetValue(255);
    gm_colourset_rm.SetValue(0);
    gm_colourset_fx.SetValue(0);

    Msg("Color settings reset to defaults\n");
}

//-----------------------------------------------------------------------------
// Purpose: Save color preset
//-----------------------------------------------------------------------------
void CGModColorSystem::CMD_gm_color_save(void)
{
    // Implementation would save current settings to file
    Msg("Color settings saved\n");
}

//-----------------------------------------------------------------------------
// Purpose: Load color preset
//-----------------------------------------------------------------------------
void CGModColorSystem::CMD_gm_color_load(void)
{
    // Implementation would load settings from file
    Msg("Color settings loaded\n");
}

static ConCommand gm_color_apply("gm_color_apply", CGModColorSystem::CMD_gm_color_apply, "Show current color settings");
static ConCommand gm_color_reset("gm_color_reset", CGModColorSystem::CMD_gm_color_reset, "Reset color settings to defaults");
static ConCommand gm_color_save("gm_color_save", CGModColorSystem::CMD_gm_color_save, "Save current color settings");
static ConCommand gm_color_load("gm_color_load", CGModColorSystem::CMD_gm_color_load, "Load saved color settings");

//-----------------------------------------------------------------------------
// Client initialization hook
//-----------------------------------------------------------------------------
class CColorSystemInit : public CAutoGameSystem
{
public:
    CColorSystemInit() : CAutoGameSystem("ColorSystemInit") {}

    virtual bool Init()
    {
        CGModColorSystem::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModColorSystem::Shutdown();
    }
};

static CColorSystemInit g_ColorSystemInit;