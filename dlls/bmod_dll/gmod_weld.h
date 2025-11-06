#ifndef GMOD_WELD_H
#define GMOD_WELD_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "igamesystem.h"

// Forward declarations
class CBaseEntity;
class CBasePlayer;

// Weld constraint types discovered in IDA analysis
enum WeldConstraintType_t
{
    WELD_CONSTRAINT_WELD = 0,       // Standard weld constraint
    WELD_CONSTRAINT_AXIS,           // Axis constraint
    WELD_CONSTRAINT_BALLSOCKET,     // Ball and socket joint
    WELD_CONSTRAINT_ROPE,           // Rope constraint
    WELD_CONSTRAINT_SLIDER,         // Slider constraint
    WELD_CONSTRAINT_SPRING,         // Spring constraint
    WELD_CONSTRAINT_HYDRAULIC,      // Hydraulic constraint
    WELD_CONSTRAINT_MOTOR,          // Motor constraint
    WELD_CONSTRAINT_PULLEY,         // Pulley constraint
    WELD_CONSTRAINT_KEEPUPRIGHT,    // Keep upright constraint
    WELD_CONSTRAINT_NOCOLLIDE,      // No collide constraint
    WELD_CONSTRAINT_MAX
};

// Weld constraint data structure
struct WeldConstraintData_t
{
    int constraintType;
    Vector position1;
    Vector position2;
    QAngle angles1;
    QAngle angles2;
    float forceLimit;
    float torqueLimit;
    float length;
    float constant;
    float damping;
    float speed;
    float material;
    bool rigid;
    bool nocollide;
    bool deleteOnBreak;
    bool elastic;
    int bone1;
    int bone2;
};

//-----------------------------------------------------------------------------
// GMod Weld System - Implements complete constraint system from GMod 9.0.4b
//-----------------------------------------------------------------------------
class CGModWeldSystem : public CAutoGameSystem
{
public:
    CGModWeldSystem() : CAutoGameSystem() {}

    // CAutoGameSystem overrides
    virtual bool Init();
    virtual void Shutdown();
    virtual void LevelInitPostEntity();

    // Weld constraint creation functions
    static void CreateWeldConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data);
    static void CreateAxisConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data);
    static void CreateBallSocketConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data);
    static void CreateRopeConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data);
    static void CreateSliderConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data);
    static void CreateSpringConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data);
    static void CreateHydraulicConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data);
    static void CreateMotorConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data);
    static void CreatePulleyConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data);
    static void CreateKeepUprightConstraint(CBaseEntity* pEntity, const WeldConstraintData_t& data);
    static void CreateNoCollideConstraint(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const WeldConstraintData_t& data);

    // Constraint management
    static void RemoveConstraints(CBaseEntity* pEntity);
    static void RemoveAllConstraints();
    static int GetConstraintCount();
    static void ListConstraints(CBasePlayer* pPlayer);

    // Constraint breaking system
    static void SetConstraintBreakForce(int constraintId, float force);
    static void SetConstraintBreakTorque(int constraintId, float torque);
    static bool IsConstraintBroken(int constraintId);

private:
    static CUtlVector<int> s_ConstraintList;
    static int s_NextConstraintId;
    static bool s_bSystemInitialized;
};

// Global instance
extern CGModWeldSystem g_GMod_WeldSystem;

// Console command handlers - discovered from IDA string analysis
void CMD_gm_weld(void);
void CMD_gm_weld_axis(void);
void CMD_gm_weld_ballsocket(void);
void CMD_gm_weld_rope(void);
void CMD_gm_weld_slider(void);
void CMD_gm_weld_spring(void);
void CMD_gm_weld_hydraulic(void);
void CMD_gm_weld_motor(void);
void CMD_gm_weld_pulley(void);
void CMD_gm_weld_keepupright(void);
void CMD_gm_weld_nocollide(void);
void CMD_gm_unweld(void);
void CMD_gm_unweld_all(void);
void CMD_gm_remove_constraints(void);

// Weld parameter console commands
void CMD_gm_weld_forcelimit(void);
void CMD_gm_weld_torquelimit(void);
void CMD_gm_weld_length(void);
void CMD_gm_weld_constant(void);
void CMD_gm_weld_damping(void);
void CMD_gm_weld_speed(void);
void CMD_gm_weld_material(void);
void CMD_gm_weld_rigid(void);
void CMD_gm_weld_elastic(void);
void CMD_gm_weld_width(void);
void CMD_gm_weld_addlength(void);
void CMD_gm_weld_bone1(void);
void CMD_gm_weld_bone2(void);

// Advanced weld functions
void CMD_gm_weld_toggle(void);
void CMD_gm_weld_info(void);
void CMD_gm_weld_count(void);
void CMD_gm_weld_list(void);
void CMD_gm_weld_save(void);
void CMD_gm_weld_load(void);

#endif // GMOD_WELD_H