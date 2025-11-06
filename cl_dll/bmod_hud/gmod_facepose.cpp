//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Face Posing System Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_facepose.h"
#include "c_basehlplayer.h"
#include "engine/IEngineSound.h"
#include "tier1/convar.h"
#include "tier1/strtools.h"

// Initialize static members
float CGModFacePose::m_flFlexValues[MAX_FACE_POSE_FLEXES] = {0.0f};
float CGModFacePose::m_flFaceScale = 1.0f;
bool CGModFacePose::m_bInitialized = false;

// Console variables
ConVar* g_pFlexConVars[MAX_FACE_POSE_FLEXES] = {NULL};
ConVar gm_facescale("gm_facescale", "1.0", FCVAR_CLIENTDLL, "Face scale multiplier for expressions");

// Flex callback function
void FlexConVarCallback(ConVar *var, const char *pOldString)
{
    CGModFacePose::FlexCallback(var, pOldString);
}

//-----------------------------------------------------------------------------
// Purpose: Initialize the face posing system
//-----------------------------------------------------------------------------
void CGModFacePose::Initialize()
{
    if (m_bInitialized)
        return;

    // Reset all flex values
    ResetAllFlexes();

    // Create all flex console variables
    CreateFlexConVars();

    m_bInitialized = true;

    DevMsg("Face Posing System initialized with %d flex controls\n", MAX_FACE_POSE_FLEXES);
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown the face posing system
//-----------------------------------------------------------------------------
void CGModFacePose::Shutdown()
{
    if (!m_bInitialized)
        return;

    DestroyFlexConVars();
    m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Create console variables for all flex controls
//-----------------------------------------------------------------------------
void CGModFacePose::CreateFlexConVars()
{
    for (int i = 0; i < MAX_FACE_POSE_FLEXES; i++)
    {
        char varName[32];
        char varDesc[64];

        Q_snprintf(varName, sizeof(varName), "gm_facepose_flex%d", i);
        Q_snprintf(varDesc, sizeof(varDesc), "Facial flex control %d (0.0-1.0)", i);

        // Create ConVar with callback
        g_pFlexConVars[i] = new ConVar(varName, "0.0", FCVAR_CLIENTDLL, varDesc, FlexConVarCallback);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Destroy console variables
//-----------------------------------------------------------------------------
void CGModFacePose::DestroyFlexConVars()
{
    for (int i = 0; i < MAX_FACE_POSE_FLEXES; i++)
    {
        if (g_pFlexConVars[i])
        {
            delete g_pFlexConVars[i];
            g_pFlexConVars[i] = NULL;
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Console variable callback for flex changes
//-----------------------------------------------------------------------------
void CGModFacePose::FlexCallback(ConVar *var, const char *pOldString)
{
    if (!m_bInitialized)
        return;

    // Find which flex this ConVar represents
    for (int i = 0; i < MAX_FACE_POSE_FLEXES; i++)
    {
        if (g_pFlexConVars[i] == var)
        {
            float newValue = var->GetFloat();

            // Clamp value between 0.0 and 1.0
            newValue = clamp(newValue, 0.0f, 1.0f);

            SetFlexValue(i, newValue);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Set flex value for a specific index
//-----------------------------------------------------------------------------
void CGModFacePose::SetFlexValue(int flexIndex, float value)
{
    if (flexIndex < 0 || flexIndex >= MAX_FACE_POSE_FLEXES)
        return;

    m_flFlexValues[flexIndex] = clamp(value, 0.0f, 1.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Get flex value for a specific index
//-----------------------------------------------------------------------------
float CGModFacePose::GetFlexValue(int flexIndex)
{
    if (flexIndex < 0 || flexIndex >= MAX_FACE_POSE_FLEXES)
        return 0.0f;

    return m_flFlexValues[flexIndex];
}

//-----------------------------------------------------------------------------
// Purpose: Reset all flex values to 0
//-----------------------------------------------------------------------------
void CGModFacePose::ResetAllFlexes()
{
    for (int i = 0; i < MAX_FACE_POSE_FLEXES; i++)
    {
        m_flFlexValues[i] = 0.0f;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Apply flex values to the local player
//-----------------------------------------------------------------------------
void CGModFacePose::ApplyFlexesToPlayer(C_BaseHLPlayer* pPlayer)
{
    if (!pPlayer)
        return;

    // Apply each flex value to the player model
    for (int i = 0; i < MAX_FACE_POSE_FLEXES; i++)
    {
        if (m_flFlexValues[i] > 0.0f)
        {
            // Apply flex to player with face scale multiplier
            float scaledValue = m_flFlexValues[i] * m_flFaceScale;
            pPlayer->SetFlexWeight(i, scaledValue);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Update face pose for current frame
//-----------------------------------------------------------------------------
void CGModFacePose::UpdateFacePose()
{
    if (!m_bInitialized)
        return;

    C_BaseHLPlayer* pLocalPlayer = C_BaseHLPlayer::GetLocalPlayer();
    if (pLocalPlayer)
    {
        ApplyFlexesToPlayer(pLocalPlayer);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Set face scale multiplier
//-----------------------------------------------------------------------------
void CGModFacePose::SetFaceScale(float scale)
{
    m_flFaceScale = max(0.0f, scale);
}

//-----------------------------------------------------------------------------
// Purpose: Get face scale multiplier
//-----------------------------------------------------------------------------
float CGModFacePose::GetFaceScale()
{
    return m_flFaceScale;
}

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Reload face posing system
//-----------------------------------------------------------------------------
void CGModFacePose::CMD_gm_facepose_reload(void)
{
    Shutdown();
    Initialize();
    Msg("Face posing system reloaded\n");
}

static ConCommand gm_facepose_reload("gm_facepose_reload", CGModFacePose::CMD_gm_facepose_reload, "Reload the face posing system");

//-----------------------------------------------------------------------------
// Client initialization hook
//-----------------------------------------------------------------------------
class CFacePoseInit : public CAutoGameSystem
{
public:
    CFacePoseInit() : CAutoGameSystem("FacePoseInit") {}

    virtual bool Init()
    {
        CGModFacePose::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModFacePose::Shutdown();
    }

    virtual void LevelInitPreEntity()
    {
        // Update face pose each frame
        CGModFacePose::UpdateFacePose();
    }
};

static CFacePoseInit g_FacePoseInit;