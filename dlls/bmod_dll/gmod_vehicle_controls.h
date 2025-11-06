//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Vehicle/Wheel/Thruster Control System - GMod 9.0.4b compatible
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_VEHICLE_CONTROLS_H
#define GMOD_VEHICLE_CONTROLS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

//-----------------------------------------------------------------------------
// Control Types
//-----------------------------------------------------------------------------
enum ControlType_t
{
    CONTROL_THRUSTER = 0,
    CONTROL_WHEEL_FORWARD,
    CONTROL_WHEEL_BACKWARD,
    CONTROL_CAMERA_STATIC,
    CONTROL_CAMERA_PROP,
    CONTROL_CAMERA_VIEW,

    CONTROL_TYPE_COUNT
};

//-----------------------------------------------------------------------------
// Vehicle Control System - Manages wheel, thruster, and camera controls
//-----------------------------------------------------------------------------
class CGModVehicleControls
{
public:
    // System initialization
    static void Initialize();
    static void Shutdown();

    // Control state management
    static void SetControlState(CBasePlayer* pPlayer, ControlType_t type, bool state);
    static bool GetControlState(CBasePlayer* pPlayer, ControlType_t type);

    // Update functions
    static void UpdatePlayerControls(CBasePlayer* pPlayer);
    static void UpdateThrusterControls(CBasePlayer* pPlayer);
    static void UpdateWheelControls(CBasePlayer* pPlayer);
    static void UpdateCameraControls(CBasePlayer* pPlayer);

    // Wheel management
    static void SetAllWheelsOn(CBasePlayer* pPlayer, bool state);
    static void SetWheelForward(CBasePlayer* pPlayer, bool state);
    static void SetWheelBackward(CBasePlayer* pPlayer, bool state);

    // Console command handlers
    static void CMD_gm_thrust_start(void);
    static void CMD_gm_thrust_stop(void);
    static void CMD_gm_wheelf_start(void);
    static void CMD_gm_wheelf_stop(void);
    static void CMD_gm_wheelb_start(void);
    static void CMD_gm_wheelb_stop(void);
    static void CMD_gm_wheel_allon(void);
    static void CMD_gm_wheel_alloff(void);
    static void CMD_gm_cam_static_start(void);
    static void CMD_gm_cam_static_stop(void);
    static void CMD_gm_cam_prop_start(void);
    static void CMD_gm_cam_prop_stop(void);
    static void CMD_gm_cam_view_start(void);
    static void CMD_gm_cam_view_stop(void);

    // Entity creation
    static void CMD_gm_makeentity(void);

private:
    static bool m_bInitialized;
    static bool m_PlayerControlStates[MAX_PLAYERS][CONTROL_TYPE_COUNT];

    // Internal functions
    static void ResetPlayerControls(int playerIndex);
    static CBasePlayer* GetCommandPlayer();
};

#endif // GMOD_VEHICLE_CONTROLS_H