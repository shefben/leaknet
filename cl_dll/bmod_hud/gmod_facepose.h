//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Face Posing System - GMod 9.0.4b compatible implementation
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_FACEPOSE_H
#define GMOD_FACEPOSE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "c_basehlplayer.h"

//-----------------------------------------------------------------------------
// Face Posing System - Controls facial expressions via flex controllers
//-----------------------------------------------------------------------------

#define MAX_FACE_POSE_FLEXES 62 // gm_facepose_flex0 through gm_facepose_flex61

class CGModFacePose
{
public:
    // Initialization
    static void Initialize();
    static void Shutdown();

    // Console command handlers
    static void CMD_gm_facepose_reload(void);

    // Flex control management
    static void SetFlexValue(int flexIndex, float value);
    static float GetFlexValue(int flexIndex);
    static void ResetAllFlexes();

    // Apply flex values to player
    static void ApplyFlexesToPlayer(C_BaseHLPlayer* pPlayer);
    static void UpdateFacePose();

    // Face scale control
    static void SetFaceScale(float scale);
    static float GetFaceScale();

    // Callback functions (need public access for ConVar registration)
    static void FlexCallback(ConVar *var, const char *pOldString);

private:
    static float m_flFlexValues[MAX_FACE_POSE_FLEXES];
    static float m_flFaceScale;
    static bool m_bInitialized;

    // Console variable creation
    static void CreateFlexConVars();
    static void DestroyFlexConVars();
};

// Console variable declarations
extern ConVar gm_facescale;
extern ConVar* g_pFlexConVars[MAX_FACE_POSE_FLEXES];

#endif // GMOD_FACEPOSE_H