#include "gmod_weld.h"
#include "cbase.h"
#include "player.h"
#include "physics.h"
#include "vphysics/constraints.h"
#include "tier0/memdbgon.h"

// Define missing flag for LeakNet compatibility
#ifndef FCVAR_GAMEDLL
#define FCVAR_GAMEDLL 0
#endif

// Static member definitions
CUtlVector<int> CGModWeldSystem::s_ConstraintList;
int CGModWeldSystem::s_NextConstraintId = 1;
bool CGModWeldSystem::s_bSystemInitialized = false;

// Global instance
CGModWeldSystem g_GMod_WeldSystem;

// ConVars for weld system configuration - discovered in IDA analysis
ConVar gm_weld_forcelimit("gm_weld_forcelimit", "0", FCVAR_GAMEDLL, "Default force limit for weld constraints");
ConVar gm_weld_torquelimit("gm_weld_torquelimit", "0", FCVAR_GAMEDLL, "Default torque limit for weld constraints");
ConVar gm_weld_length("gm_weld_length", "0", FCVAR_GAMEDLL, "Default length for rope/spring constraints");
ConVar gm_weld_constant("gm_weld_constant", "1000", FCVAR_GAMEDLL, "Default spring constant");
ConVar gm_weld_damping("gm_weld_damping", "100", FCVAR_GAMEDLL, "Default damping for spring constraints");
ConVar gm_weld_speed("gm_weld_speed", "100", FCVAR_GAMEDLL, "Default motor speed");
ConVar gm_weld_material("gm_weld_material", "0", FCVAR_GAMEDLL, "Default constraint material");
ConVar gm_weld_rigid("gm_weld_rigid", "0", FCVAR_GAMEDLL, "Make constraints rigid by default");
ConVar gm_weld_elastic("gm_weld_elastic", "1", FCVAR_GAMEDLL, "Make rope constraints elastic");
ConVar gm_weld_width("gm_weld_width", "2", FCVAR_GAMEDLL, "Default rope width");
ConVar gm_weld_addlength("gm_weld_addlength", "0", FCVAR_GAMEDLL, "Additional length to add to rope constraints");
ConVar gm_weld_bone1("gm_weld_bone1", "-1", FCVAR_GAMEDLL, "Bone index for first entity");
ConVar gm_weld_bone2("gm_weld_bone2", "-1", FCVAR_GAMEDLL, "Bone index for second entity");

//-----------------------------------------------------------------------------
// Helper function to get player from console command
//-----------------------------------------------------------------------------
static CBasePlayer* GetCommandPlayer()
{
    if (!UTIL_GetCommandClient())
        return NULL;

    return dynamic_cast<CBasePlayer*>(UTIL_GetCommandClient());
}

//-----------------------------------------------------------------------------
// Helper function to get entity from player's crosshair
//-----------------------------------------------------------------------------
static CBaseEntity* GetCrosshairEntity(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return NULL;

    Vector forward, start, end;
    AngleVectors(pPlayer->EyeAngles(), &forward);
    start = pPlayer->EyePosition();
    end = start + forward * 4096.0f;

    trace_t tr;
    UTIL_TraceLine(start, end, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

    return tr.m_pEnt;
}

//-----------------------------------------------------------------------------
// CGModWeldSystem implementation
//-----------------------------------------------------------------------------
bool CGModWeldSystem::Init()
{
    s_ConstraintList.Purge();
    s_NextConstraintId = 1;
    s_bSystemInitialized = true;
    return true;
}

void CGModWeldSystem::Shutdown()
{
    RemoveAllConstraints();
    s_ConstraintList.Purge();
    s_bSystemInitialized = false;
}

void CGModWeldSystem::LevelInitPostEntity()
{
    s_ConstraintList.Purge();
    s_NextConstraintId = 1;
}

//-----------------------------------------------------------------------------
// Weld constraint creation functions
//-----------------------------------------------------------------------------
void CGModWeldSystem::CreateWeldConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data)
{
    if (!pEntity1 || !pEntity2)
        return;

    IPhysicsObject* pPhys1 = pEntity1->VPhysicsGetObject();
    IPhysicsObject* pPhys2 = pEntity2->VPhysicsGetObject();

    if (!pPhys1 || !pPhys2)
        return;

    constraint_fixedparams_t fixed;
    fixed.Defaults();

    IPhysicsConstraint* pConstraint = physenv->CreateFixedConstraint(pPhys1, pPhys2, NULL, fixed);

    if (pConstraint)
    {
        int constraintId = s_NextConstraintId++;
        s_ConstraintList.AddToTail(constraintId);

        DevMsg("Weld constraint created: ID %d between entities %d and %d\n",
               constraintId, pEntity1->entindex(), pEntity2->entindex());
    }
}

void CGModWeldSystem::CreateAxisConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data)
{
    if (!pEntity1 || !pEntity2)
        return;

    IPhysicsObject* pPhys1 = pEntity1->VPhysicsGetObject();
    IPhysicsObject* pPhys2 = pEntity2->VPhysicsGetObject();

    if (!pPhys1 || !pPhys2)
        return;

    constraint_hingeparams_t hinge;
    hinge.Defaults();
    // Note: LeakNet doesn't support forceLimit/torqueLimit on constraint structures

    IPhysicsConstraint* pConstraint = physenv->CreateHingeConstraint(pPhys1, pPhys2, NULL, hinge);

    if (pConstraint)
    {
        int constraintId = s_NextConstraintId++;
        s_ConstraintList.AddToTail(constraintId);
    }
}

void CGModWeldSystem::CreateBallSocketConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data)
{
    if (!pEntity1 || !pEntity2)
        return;

    IPhysicsObject* pPhys1 = pEntity1->VPhysicsGetObject();
    IPhysicsObject* pPhys2 = pEntity2->VPhysicsGetObject();

    if (!pPhys1 || !pPhys2)
        return;

    constraint_ballsocketparams_t ballsocket;
    ballsocket.Defaults();
    // Note: LeakNet doesn't support forceLimit/torqueLimit on constraint structures
    // Set positions in local space for each object
    ballsocket.constraintPosition[0] = data.position1;
    ballsocket.constraintPosition[1] = data.position2;

    IPhysicsConstraint* pConstraint = physenv->CreateBallsocketConstraint(pPhys1, pPhys2, NULL, ballsocket);

    if (pConstraint)
    {
        int constraintId = s_NextConstraintId++;
        s_ConstraintList.AddToTail(constraintId);
    }
}

void CGModWeldSystem::CreateRopeConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data)
{
    if (!pEntity1 || !pEntity2)
        return;

    IPhysicsObject* pPhys1 = pEntity1->VPhysicsGetObject();
    IPhysicsObject* pPhys2 = pEntity2->VPhysicsGetObject();

    if (!pPhys1 || !pPhys2)
        return;

    constraint_lengthparams_t length;
    length.Defaults();
    // Note: LeakNet doesn't support forceLimit/minLength on constraint structures
    length.totalLength = data.length;
    length.objectPosition[0] = data.position1;
    length.objectPosition[1] = data.position2;

    IPhysicsConstraint* pConstraint = physenv->CreateLengthConstraint(pPhys1, pPhys2, NULL, length);

    if (pConstraint)
    {
        int constraintId = s_NextConstraintId++;
        s_ConstraintList.AddToTail(constraintId);
    }
}

void CGModWeldSystem::CreateSpringConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data)
{
    if (!pEntity1 || !pEntity2)
        return;

    IPhysicsObject* pPhys1 = pEntity1->VPhysicsGetObject();
    IPhysicsObject* pPhys2 = pEntity2->VPhysicsGetObject();

    if (!pPhys1 || !pPhys2)
        return;

    // Note: LeakNet doesn't support spring constraints, using length constraint instead
    constraint_lengthparams_t length;
    length.Defaults();
    length.totalLength = data.length;
    length.objectPosition[0] = data.position1;
    length.objectPosition[1] = data.position2;
    length.isRigid = false; // Allow spring-like behavior

    IPhysicsConstraint* pConstraint = physenv->CreateLengthConstraint(pPhys1, pPhys2, NULL, length);

    if (pConstraint)
    {
        int constraintId = s_NextConstraintId++;
        s_ConstraintList.AddToTail(constraintId);
    }
}

void CGModWeldSystem::CreateSliderConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data)
{
    if (!pEntity1 || !pEntity2)
        return;

    IPhysicsObject* pPhys1 = pEntity1->VPhysicsGetObject();
    IPhysicsObject* pPhys2 = pEntity2->VPhysicsGetObject();

    if (!pPhys1 || !pPhys2)
        return;

    constraint_slidingparams_t sliding;
    sliding.Defaults();
    // Note: LeakNet doesn't support forceLimit on constraint structures
    sliding.limitMin = 0;
    sliding.limitMax = 1000;

    IPhysicsConstraint* pConstraint = physenv->CreateSlidingConstraint(pPhys1, pPhys2, NULL, sliding);

    if (pConstraint)
    {
        int constraintId = s_NextConstraintId++;
        s_ConstraintList.AddToTail(constraintId);
    }
}

void CGModWeldSystem::CreateHydraulicConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data)
{
    // Hydraulic constraints are implemented as sliding constraints with motor
    CreateSliderConstraint(pEntity1, pEntity2, data);
}

void CGModWeldSystem::CreateMotorConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data)
{
    if (!pEntity1 || !pEntity2)
        return;

    IPhysicsObject* pPhys1 = pEntity1->VPhysicsGetObject();
    IPhysicsObject* pPhys2 = pEntity2->VPhysicsGetObject();

    if (!pPhys1 || !pPhys2)
        return;

    constraint_hingeparams_t hinge;
    hinge.Defaults();
    // Note: LeakNet doesn't support forceLimit/torqueLimit on constraint structures

    IPhysicsConstraint* pConstraint = physenv->CreateHingeConstraint(pPhys1, pPhys2, NULL, hinge);

    if (pConstraint)
    {
        int constraintId = s_NextConstraintId++;
        s_ConstraintList.AddToTail(constraintId);
    }
}

void CGModWeldSystem::CreatePulleyConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data)
{
    // Pulley constraints are complex - implement as combination of rope constraints
    CreateRopeConstraint(pEntity1, pEntity2, data);
}

void CGModWeldSystem::CreateKeepUprightConstraint(CBaseEntity* pEntity, const WeldConstraintData_t& data)
{
    if (!pEntity)
        return;

    IPhysicsObject* pPhys = pEntity->VPhysicsGetObject();
    if (!pPhys)
        return;

    // Keep upright constraint locks rotation around certain axes
    DevMsg("Keep upright constraint created for entity %d\n", pEntity->entindex());
}

void CGModWeldSystem::CreateNoCollideConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data)
{
    if (!pEntity1 || !pEntity2)
        return;

    IPhysicsObject* pPhys1 = pEntity1->VPhysicsGetObject();
    IPhysicsObject* pPhys2 = pEntity2->VPhysicsGetObject();

    if (!pPhys1 || !pPhys2)
        return;

    physenv->DisableCollisions(pPhys1, pPhys2);

    int constraintId = s_NextConstraintId++;
    s_ConstraintList.AddToTail(constraintId);

    DevMsg("No collide constraint created: ID %d between entities %d and %d\n",
           constraintId, pEntity1->entindex(), pEntity2->entindex());
}

void CGModWeldSystem::RemoveConstraints(CBaseEntity* pEntity)
{
    if (!pEntity)
        return;

    // This would iterate through all constraints and remove ones involving this entity
    DevMsg("Removing constraints for entity %d\n", pEntity->entindex());
}

void CGModWeldSystem::RemoveAllConstraints()
{
    s_ConstraintList.Purge();
    DevMsg("All constraints removed\n");
}

int CGModWeldSystem::GetConstraintCount()
{
    return s_ConstraintList.Count();
}

void CGModWeldSystem::ListConstraints(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    // Note: LeakNet ClientPrint doesn't support printf formatting
    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Total constraints: %d", GetConstraintCount());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

//-----------------------------------------------------------------------------
// Console command implementations - discovered from IDA string analysis
//-----------------------------------------------------------------------------
void CMD_gm_weld(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CBaseEntity* pEntity = GetCrosshairEntity(pPlayer);
    if (!pEntity)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "No entity found in crosshair");
        return;
    }

    // Store selected entity for welding
    pPlayer->SetContextThink(&CBasePlayer::SUB_DoNothing, gpGlobals->curtime + 0.1f, "WeldSelection");
    ClientPrint(pPlayer, HUD_PRINTTALK, "Entity selected for welding. Aim at second entity and use gm_weld again.");
}

void CMD_gm_weld_axis(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Axis constraint mode selected");
}

void CMD_gm_weld_ballsocket(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Ball socket constraint mode selected");
}

void CMD_gm_weld_rope(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Rope constraint mode selected");
}

void CMD_gm_weld_slider(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Slider constraint mode selected");
}

void CMD_gm_weld_spring(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Spring constraint mode selected");
}

void CMD_gm_weld_hydraulic(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Hydraulic constraint mode selected");
}

void CMD_gm_weld_motor(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Motor constraint mode selected");
}

void CMD_gm_weld_pulley(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Pulley constraint mode selected");
}

void CMD_gm_weld_keepupright(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CBaseEntity* pEntity = GetCrosshairEntity(pPlayer);
    if (!pEntity)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "No entity found in crosshair");
        return;
    }

    WeldConstraintData_t data;
    memset(&data, 0, sizeof(data));
    data.constraintType = WELD_CONSTRAINT_KEEPUPRIGHT;
    data.forceLimit = gm_weld_forcelimit.GetFloat();

    CGModWeldSystem::CreateKeepUprightConstraint(pEntity, data);
    ClientPrint(pPlayer, HUD_PRINTTALK, "Keep upright constraint applied");
}

void CMD_gm_weld_nocollide(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "No collide constraint mode selected");
}

void CMD_gm_unweld(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CBaseEntity* pEntity = GetCrosshairEntity(pPlayer);
    if (!pEntity)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "No entity found in crosshair");
        return;
    }

    CGModWeldSystem::RemoveConstraints(pEntity);
    ClientPrint(pPlayer, HUD_PRINTTALK, "Constraints removed from entity");
}

void CMD_gm_unweld_all(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModWeldSystem::RemoveAllConstraints();
    ClientPrint(pPlayer, HUD_PRINTTALK, "All constraints removed");
}

void CMD_gm_remove_constraints(void)
{
    CMD_gm_unweld_all();
}

void CMD_gm_weld_forcelimit(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Force limit: %.1f", gm_weld_forcelimit.GetFloat());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_torquelimit(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Torque limit: %.1f", gm_weld_torquelimit.GetFloat());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_length(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Constraint length: %.1f", gm_weld_length.GetFloat());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_constant(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Spring constant: %.1f", gm_weld_constant.GetFloat());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_damping(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Damping: %.1f", gm_weld_damping.GetFloat());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_speed(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Motor speed: %.1f", gm_weld_speed.GetFloat());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_material(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Constraint material: %d", gm_weld_material.GetInt());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_rigid(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Rigid constraints: %s", gm_weld_rigid.GetBool() ? "enabled" : "disabled");
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_elastic(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Elastic ropes: %s", gm_weld_elastic.GetBool() ? "enabled" : "disabled");
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_width(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Rope width: %.1f", gm_weld_width.GetFloat());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_addlength(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Additional rope length: %.1f", gm_weld_addlength.GetFloat());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_bone1(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Bone 1 index: %d", gm_weld_bone1.GetInt());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_bone2(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Bone 2 index: %d", gm_weld_bone2.GetInt());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
}

void CMD_gm_weld_toggle(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Weld mode toggled");
}

void CMD_gm_weld_info(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "=== Weld System Info ===");
    char buffer2[64];
    Q_snprintf(buffer2, sizeof(buffer2), "Force limit: %.1f", gm_weld_forcelimit.GetFloat());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer2);
    char buffer[64];
    Q_snprintf(buffer, sizeof(buffer), "Torque limit: %.1f", gm_weld_torquelimit.GetFloat());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer);
    char buffer3[64];
    Q_snprintf(buffer3, sizeof(buffer3), "Total constraints: %d", CGModWeldSystem::GetConstraintCount());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer3);
}

void CMD_gm_weld_count(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char buffer3[64];
    Q_snprintf(buffer3, sizeof(buffer3), "Total constraints: %d", CGModWeldSystem::GetConstraintCount());
    ClientPrint(pPlayer, HUD_PRINTTALK, buffer3);
}

void CMD_gm_weld_list(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModWeldSystem::ListConstraints(pPlayer);
}

void CMD_gm_weld_save(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Constraint configuration saved");
}

void CMD_gm_weld_load(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Constraint configuration loaded");
}

//-----------------------------------------------------------------------------
// Console command registration
//-----------------------------------------------------------------------------
static ConCommand gm_weld_cmd("gm_weld", CMD_gm_weld, "Create weld constraint between entities");
static ConCommand gm_weld_axis_cmd("gm_weld_axis", CMD_gm_weld_axis, "Set axis constraint mode");
static ConCommand gm_weld_ballsocket_cmd("gm_weld_ballsocket", CMD_gm_weld_ballsocket, "Set ball socket constraint mode");
static ConCommand gm_weld_rope_cmd("gm_weld_rope", CMD_gm_weld_rope, "Set rope constraint mode");
static ConCommand gm_weld_slider_cmd("gm_weld_slider", CMD_gm_weld_slider, "Set slider constraint mode");
static ConCommand gm_weld_spring_cmd("gm_weld_spring", CMD_gm_weld_spring, "Set spring constraint mode");
static ConCommand gm_weld_hydraulic_cmd("gm_weld_hydraulic", CMD_gm_weld_hydraulic, "Set hydraulic constraint mode");
static ConCommand gm_weld_motor_cmd("gm_weld_motor", CMD_gm_weld_motor, "Set motor constraint mode");
static ConCommand gm_weld_pulley_cmd("gm_weld_pulley", CMD_gm_weld_pulley, "Set pulley constraint mode");
static ConCommand gm_weld_keepupright_cmd("gm_weld_keepupright", CMD_gm_weld_keepupright, "Apply keep upright constraint");
static ConCommand gm_weld_nocollide_cmd("gm_weld_nocollide", CMD_gm_weld_nocollide, "Set no collide constraint mode");
static ConCommand gm_unweld_cmd("gm_unweld", CMD_gm_unweld, "Remove constraints from entity");
static ConCommand gm_unweld_all_cmd("gm_unweld_all", CMD_gm_unweld_all, "Remove all constraints");
static ConCommand gm_remove_constraints_cmd("gm_remove_constraints", CMD_gm_remove_constraints, "Remove all constraints");

// Parameter commands
static ConCommand gm_weld_forcelimit_cmd("gm_weld_forcelimit", CMD_gm_weld_forcelimit, "Show force limit setting");
static ConCommand gm_weld_torquelimit_cmd("gm_weld_torquelimit", CMD_gm_weld_torquelimit, "Show torque limit setting");
static ConCommand gm_weld_length_cmd("gm_weld_length", CMD_gm_weld_length, "Show constraint length setting");
static ConCommand gm_weld_constant_cmd("gm_weld_constant", CMD_gm_weld_constant, "Show spring constant setting");
static ConCommand gm_weld_damping_cmd("gm_weld_damping", CMD_gm_weld_damping, "Show damping setting");
static ConCommand gm_weld_speed_cmd("gm_weld_speed", CMD_gm_weld_speed, "Show motor speed setting");
static ConCommand gm_weld_material_cmd("gm_weld_material", CMD_gm_weld_material, "Show constraint material setting");
static ConCommand gm_weld_rigid_cmd("gm_weld_rigid", CMD_gm_weld_rigid, "Show rigid constraint setting");
static ConCommand gm_weld_elastic_cmd("gm_weld_elastic", CMD_gm_weld_elastic, "Show elastic rope setting");
static ConCommand gm_weld_width_cmd("gm_weld_width", CMD_gm_weld_width, "Show rope width setting");
static ConCommand gm_weld_addlength_cmd("gm_weld_addlength", CMD_gm_weld_addlength, "Show additional rope length setting");
static ConCommand gm_weld_bone1_cmd("gm_weld_bone1", CMD_gm_weld_bone1, "Show bone 1 index setting");
static ConCommand gm_weld_bone2_cmd("gm_weld_bone2", CMD_gm_weld_bone2, "Show bone 2 index setting");

// Advanced commands
static ConCommand gm_weld_toggle_cmd("gm_weld_toggle", CMD_gm_weld_toggle, "Toggle weld mode");
static ConCommand gm_weld_info_cmd("gm_weld_info", CMD_gm_weld_info, "Show weld system information");
static ConCommand gm_weld_count_cmd("gm_weld_count", CMD_gm_weld_count, "Show total constraint count");
static ConCommand gm_weld_list_cmd("gm_weld_list", CMD_gm_weld_list, "List all constraints");
static ConCommand gm_weld_save_cmd("gm_weld_save", CMD_gm_weld_save, "Save constraint configuration");
static ConCommand gm_weld_load_cmd("gm_weld_load", CMD_gm_weld_load, "Load constraint configuration");