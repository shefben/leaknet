//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Vehicle/Wheel/Thruster Control System Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_vehicle_controls.h"
#include "player.h"
#include "tier1/strtools.h"

// Initialize static members
bool CGModVehicleControls::m_bInitialized = false;
bool CGModVehicleControls::m_PlayerControlStates[MAX_PLAYERS][CONTROL_TYPE_COUNT];

//-----------------------------------------------------------------------------
// Purpose: Initialize vehicle control system
//-----------------------------------------------------------------------------
void CGModVehicleControls::Initialize()
{
    if (m_bInitialized)
        return;

    // Reset all player control states
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        ResetPlayerControls(i);
    }

    m_bInitialized = true;
    Msg("Vehicle Control System initialized\n");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown vehicle control system
//-----------------------------------------------------------------------------
void CGModVehicleControls::Shutdown()
{
    m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Reset control states for a player
//-----------------------------------------------------------------------------
void CGModVehicleControls::ResetPlayerControls(int playerIndex)
{
    if (playerIndex < 0 || playerIndex >= MAX_PLAYERS)
        return;

    for (int i = 0; i < CONTROL_TYPE_COUNT; i++)
    {
        m_PlayerControlStates[playerIndex][i] = false;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Set control state for player
//-----------------------------------------------------------------------------
void CGModVehicleControls::SetControlState(CBasePlayer* pPlayer, ControlType_t type, bool state)
{
    if (!pPlayer || type >= CONTROL_TYPE_COUNT)
        return;

    int playerIndex = pPlayer->entindex() - 1;
    if (playerIndex < 0 || playerIndex >= MAX_PLAYERS)
        return;

    m_PlayerControlStates[playerIndex][type] = state;
}

//-----------------------------------------------------------------------------
// Purpose: Get control state for player
//-----------------------------------------------------------------------------
bool CGModVehicleControls::GetControlState(CBasePlayer* pPlayer, ControlType_t type)
{
    if (!pPlayer || type >= CONTROL_TYPE_COUNT)
        return false;

    int playerIndex = pPlayer->entindex() - 1;
    if (playerIndex < 0 || playerIndex >= MAX_PLAYERS)
        return false;

    return m_PlayerControlStates[playerIndex][type];
}

//-----------------------------------------------------------------------------
// Purpose: Get command client player
//-----------------------------------------------------------------------------
CBasePlayer* CGModVehicleControls::GetCommandPlayer()
{
    return UTIL_GetCommandClient();
}

//-----------------------------------------------------------------------------
// Purpose: Update all player controls
//-----------------------------------------------------------------------------
void CGModVehicleControls::UpdatePlayerControls(CBasePlayer* pPlayer)
{
    if (!m_bInitialized || !pPlayer)
        return;

    UpdateThrusterControls(pPlayer);
    UpdateWheelControls(pPlayer);
    UpdateCameraControls(pPlayer);
}

//-----------------------------------------------------------------------------
// Purpose: Update thruster controls
//-----------------------------------------------------------------------------
void CGModVehicleControls::UpdateThrusterControls(CBasePlayer* pPlayer)
{
    if (!GetControlState(pPlayer, CONTROL_THRUSTER))
        return;

    // Find all thrusters owned by this player and activate them
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "gmod_thruster")) != NULL)
    {
        if (pEntity->GetOwnerEntity() == pPlayer)
        {
            // Activate thruster
            pEntity->AcceptInput("Activate", pPlayer, pPlayer);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Update wheel controls
//-----------------------------------------------------------------------------
void CGModVehicleControls::UpdateWheelControls(CBasePlayer* pPlayer)
{
    bool forwardState = GetControlState(pPlayer, CONTROL_WHEEL_FORWARD);
    bool backwardState = GetControlState(pPlayer, CONTROL_WHEEL_BACKWARD);

    if (!forwardState && !backwardState)
        return;

    // Find all wheels owned by this player
    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "gmod_wheel")) != NULL)
    {
        if (pEntity->GetOwnerEntity() == pPlayer)
        {
            if (forwardState)
            {
                pEntity->AcceptInput("MotorOn", pPlayer, pPlayer);
                pEntity->AcceptInput("SetTorque", pPlayer, pPlayer, "100");
            }
            else if (backwardState)
            {
                pEntity->AcceptInput("MotorOn", pPlayer, pPlayer);
                pEntity->AcceptInput("SetTorque", pPlayer, pPlayer, "-100");
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Update camera controls
//-----------------------------------------------------------------------------
void CGModVehicleControls::UpdateCameraControls(CBasePlayer* pPlayer)
{
    // Camera control implementation would go here
    // This would control spectator cameras, prop cameras, etc.
}

//-----------------------------------------------------------------------------
// Purpose: Set all wheels on/off
//-----------------------------------------------------------------------------
void CGModVehicleControls::SetAllWheelsOn(CBasePlayer* pPlayer, bool state)
{
    if (!pPlayer)
        return;

    CBaseEntity* pEntity = NULL;
    while ((pEntity = gEntList.FindEntityByClassname(pEntity, "gmod_wheel")) != NULL)
    {
        if (pEntity->GetOwnerEntity() == pPlayer)
        {
            if (state)
            {
                pEntity->AcceptInput("MotorOn", pPlayer, pPlayer);
            }
            else
            {
                pEntity->AcceptInput("MotorOff", pPlayer, pPlayer);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Thruster controls
//-----------------------------------------------------------------------------
void CGModVehicleControls::CMD_gm_thrust_start(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_THRUSTER, true);
    }
}

void CGModVehicleControls::CMD_gm_thrust_stop(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_THRUSTER, false);

        // Turn off all thrusters
        CBaseEntity* pEntity = NULL;
        while ((pEntity = gEntList.FindEntityByClassname(pEntity, "gmod_thruster")) != NULL)
        {
            if (pEntity->GetOwnerEntity() == pPlayer)
            {
                pEntity->AcceptInput("Deactivate", pPlayer, pPlayer);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Wheel forward controls
//-----------------------------------------------------------------------------
void CGModVehicleControls::CMD_gm_wheelf_start(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_WHEEL_FORWARD, true);
        SetControlState(pPlayer, CONTROL_WHEEL_BACKWARD, false);
    }
}

void CGModVehicleControls::CMD_gm_wheelf_stop(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_WHEEL_FORWARD, false);

        // Turn off wheels
        CBaseEntity* pEntity = NULL;
        while ((pEntity = gEntList.FindEntityByClassname(pEntity, "gmod_wheel")) != NULL)
        {
            if (pEntity->GetOwnerEntity() == pPlayer)
            {
                pEntity->AcceptInput("MotorOff", pPlayer, pPlayer);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Wheel backward controls
//-----------------------------------------------------------------------------
void CGModVehicleControls::CMD_gm_wheelb_start(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_WHEEL_BACKWARD, true);
        SetControlState(pPlayer, CONTROL_WHEEL_FORWARD, false);
    }
}

void CGModVehicleControls::CMD_gm_wheelb_stop(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_WHEEL_BACKWARD, false);

        // Turn off wheels
        CBaseEntity* pEntity = NULL;
        while ((pEntity = gEntList.FindEntityByClassname(pEntity, "gmod_wheel")) != NULL)
        {
            if (pEntity->GetOwnerEntity() == pPlayer)
            {
                pEntity->AcceptInput("MotorOff", pPlayer, pPlayer);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Wheel all on/off
//-----------------------------------------------------------------------------
void CGModVehicleControls::CMD_gm_wheel_allon(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetAllWheelsOn(pPlayer, true);
        ClientPrint(pPlayer, HUD_PRINTTALK, "All wheels activated");
    }
}

void CGModVehicleControls::CMD_gm_wheel_alloff(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetAllWheelsOn(pPlayer, false);
        ClientPrint(pPlayer, HUD_PRINTTALK, "All wheels deactivated");
    }
}

//-----------------------------------------------------------------------------
// Camera controls
//-----------------------------------------------------------------------------
void CGModVehicleControls::CMD_gm_cam_static_start(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_CAMERA_STATIC, true);
    }
}

void CGModVehicleControls::CMD_gm_cam_static_stop(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_CAMERA_STATIC, false);
    }
}

void CGModVehicleControls::CMD_gm_cam_prop_start(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_CAMERA_PROP, true);
    }
}

void CGModVehicleControls::CMD_gm_cam_prop_stop(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_CAMERA_PROP, false);
    }
}

void CGModVehicleControls::CMD_gm_cam_view_start(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_CAMERA_VIEW, true);
    }
}

void CGModVehicleControls::CMD_gm_cam_view_stop(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
    {
        SetControlState(pPlayer, CONTROL_CAMERA_VIEW, false);
    }
}

//-----------------------------------------------------------------------------
// Entity creation
//-----------------------------------------------------------------------------
void CGModVehicleControls::CMD_gm_makeentity(void)
{
    CCommand args;
    CBasePlayer* pPlayer = GetCommandPlayer();

    if (!pPlayer)
        return;

    if (args.ArgC() < 2)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Usage: gm_makeentity <classname>");
        return;
    }

    const char* className = args.Arg(1);

    // Get player's eye position and direction
    Vector eyePos = pPlayer->EyePosition();
    Vector forward;
    pPlayer->EyeVectors(&forward);

    Vector spawnPos = eyePos + forward * 100.0f;

    // Create the entity
    CBaseEntity* pEntity = CreateEntityByName(className);
    if (pEntity)
    {
        pEntity->SetAbsOrigin(spawnPos);
        pEntity->SetOwnerEntity(pPlayer);
        pEntity->Spawn();

        ClientPrint(pPlayer, HUD_PRINTTALK, "Created entity: %s", className);
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to create entity: %s", className);
    }
}

// Register console commands
static ConCommand cmd_gm_thrust_start("+gm_thrust", CGModVehicleControls::CMD_gm_thrust_start, "Start thruster control");
static ConCommand cmd_gm_thrust_stop("-gm_thrust", CGModVehicleControls::CMD_gm_thrust_stop, "Stop thruster control");
static ConCommand cmd_gm_wheelf_start("+gm_wheelf", CGModVehicleControls::CMD_gm_wheelf_start, "Start forward wheel control");
static ConCommand cmd_gm_wheelf_stop("-gm_wheelf", CGModVehicleControls::CMD_gm_wheelf_stop, "Stop forward wheel control");
static ConCommand cmd_gm_wheelb_start("+gm_wheelb", CGModVehicleControls::CMD_gm_wheelb_start, "Start backward wheel control");
static ConCommand cmd_gm_wheelb_stop("-gm_wheelb", CGModVehicleControls::CMD_gm_wheelb_stop, "Stop backward wheel control");
static ConCommand cmd_gm_wheel_allon("gm_wheel_allon", CGModVehicleControls::CMD_gm_wheel_allon, "Turn on all wheels");
static ConCommand cmd_gm_wheel_alloff("gm_wheel_alloff", CGModVehicleControls::CMD_gm_wheel_alloff, "Turn off all wheels");
static ConCommand cmd_gm_cam_static_start("+gm_cam_static", CGModVehicleControls::CMD_gm_cam_static_start, "Start static camera");
static ConCommand cmd_gm_cam_static_stop("-gm_cam_static", CGModVehicleControls::CMD_gm_cam_static_stop, "Stop static camera");
static ConCommand cmd_gm_cam_prop_start("+gm_cam_prop", CGModVehicleControls::CMD_gm_cam_prop_start, "Start prop camera");
static ConCommand cmd_gm_cam_prop_stop("-gm_cam_prop", CGModVehicleControls::CMD_gm_cam_prop_stop, "Stop prop camera");
static ConCommand cmd_gm_cam_view_start("+gm_cam_view", CGModVehicleControls::CMD_gm_cam_view_start, "Start view camera");
static ConCommand cmd_gm_cam_view_stop("-gm_cam_view", CGModVehicleControls::CMD_gm_cam_view_stop, "Stop view camera");
static ConCommand cmd_gm_makeentity("gm_makeentity", CGModVehicleControls::CMD_gm_makeentity, "Create entity at eye position");

//-----------------------------------------------------------------------------
// System initialization hook
//-----------------------------------------------------------------------------
class CVehicleControlsInit : public CAutoGameSystem
{
public:
    CVehicleControlsInit() : CAutoGameSystem("VehicleControlsInit") {}

    virtual bool Init()
    {
        CGModVehicleControls::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModVehicleControls::Shutdown();
    }

    virtual void FrameUpdatePostEntityThink()
    {
        // Update controls for all players
        for (int i = 1; i <= gpGlobals->maxClients; i++)
        {
            CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
            if (pPlayer && pPlayer->IsConnected())
            {
                CGModVehicleControls::UpdatePlayerControls(pPlayer);
            }
        }
    }
};

static CVehicleControlsInit g_VehicleControlsInit;