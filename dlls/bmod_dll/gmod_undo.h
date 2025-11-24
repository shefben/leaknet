#ifndef GMOD_UNDO_H
#define GMOD_UNDO_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "utlvector.h"
#include "Color.h"
#include "igamesystem.h"

// Forward declarations
class CBaseEntity;
class CBasePlayer;

// Undo action types discovered in IDA analysis
enum UndoActionType_t
{
    UNDO_ACTION_ENTITY_CREATE = 0,     // Entity spawning
    UNDO_ACTION_ENTITY_REMOVE,         // Entity removal
    UNDO_ACTION_CONSTRAINT_CREATE,     // Constraint creation
    UNDO_ACTION_CONSTRAINT_REMOVE,     // Constraint removal
    UNDO_ACTION_PROPERTY_CHANGE,       // Property modification
    UNDO_ACTION_TOOL_USE,              // Tool usage
    UNDO_ACTION_WELD,                  // Weld constraint
    UNDO_ACTION_ROPE,                  // Rope constraint
    UNDO_ACTION_SPRING,                // Spring constraint
    UNDO_ACTION_BALLSOCKET,            // Ball socket constraint
    UNDO_ACTION_AXIS,                  // Axis constraint
    UNDO_ACTION_SLIDER,                // Slider constraint
    UNDO_ACTION_NOCOLLIDE,             // No collide constraint
    UNDO_ACTION_KEEPUPRIGHT,           // Keep upright constraint
    UNDO_ACTION_COLOR_CHANGE,          // Color modification
    UNDO_ACTION_MATERIAL_CHANGE,       // Material change
    UNDO_ACTION_SCALE_CHANGE,          // Scale modification
    UNDO_ACTION_POSITION_CHANGE,       // Position change
    UNDO_ACTION_ANGLE_CHANGE,          // Angle change
    UNDO_ACTION_VELOCITY_CHANGE,       // Velocity change
    UNDO_ACTION_MAX
};

// Undo action data structure
struct UndoActionData_t
{
    UndoActionType_t actionType;
    int playerIndex;
    int entityIndex;
    int constraintId;
    Vector position;
    QAngle angles;
    Vector velocity;
    AngularImpulse angularVelocity;
    Color color;
    int renderMode;
    int renderFX;
    float scale;
    char materialName[256];
    char modelName[256];
    float timeStamp;
    bool isValid;

    UndoActionData_t()
    {
        memset(this, 0, sizeof(*this));
        actionType = UNDO_ACTION_ENTITY_CREATE;
        playerIndex = -1;
        entityIndex = -1;
        constraintId = -1;
        position = vec3_origin;
        angles = vec3_angle;
        velocity = vec3_origin;
        angularVelocity = vec3_origin;
        color = Color(255, 255, 255, 255);
        renderMode = 0;
        renderFX = 0;
        scale = 1.0f;
        materialName[0] = '\0';
        modelName[0] = '\0';
        timeStamp = 0.0f;
        isValid = false;
    }
};

//-----------------------------------------------------------------------------
// GMod Undo System - Implements complete undo/redo functionality from GMod 9.0.4b
//-----------------------------------------------------------------------------
class CGModUndoSystem : public CAutoGameSystem
{
public:
    CGModUndoSystem() : CAutoGameSystem() {}

    // CAutoGameSystem overrides
    virtual bool Init();
    virtual void Shutdown();
    virtual void LevelInitPostEntity();
    virtual void FrameUpdatePreEntityThink();

    // Undo/Redo functionality
    static void AddUndoAction(CBasePlayer* pPlayer, UndoActionType_t actionType, CBaseEntity* pEntity = NULL);
    static void AddUndoAction(CBasePlayer* pPlayer, const UndoActionData_t& actionData);
    static bool UndoLastAction(CBasePlayer* pPlayer);
    static bool RedoLastAction(CBasePlayer* pPlayer);
    static void UndoAll(CBasePlayer* pPlayer);
    static void ClearUndoHistory(CBasePlayer* pPlayer);

    // Entity tracking for undo
    static void RecordEntitySpawn(CBasePlayer* pPlayer, CBaseEntity* pEntity);
    static void RecordEntityRemoval(CBasePlayer* pPlayer, CBaseEntity* pEntity);
    static void RecordConstraintCreation(CBasePlayer* pPlayer, int constraintId, CBaseEntity* pEnt1, CBaseEntity* pEnt2);
    static void RecordConstraintRemoval(CBasePlayer* pPlayer, int constraintId);
    static void RecordPropertyChange(CBasePlayer* pPlayer, CBaseEntity* pEntity, UndoActionType_t changeType);

    // Undo stack management
    static int GetUndoStackSize(CBasePlayer* pPlayer);
    static int GetRedoStackSize(CBasePlayer* pPlayer);
    static void ListUndoActions(CBasePlayer* pPlayer);
    static void SetMaxUndoActions(int maxActions);
    static int GetMaxUndoActions();

    // Player-specific undo data
    static void OnPlayerDisconnect(CBasePlayer* pPlayer);
    static void OnPlayerConnect(CBasePlayer* pPlayer);

private:
    struct PlayerUndoData_t
    {
        CUtlVector<UndoActionData_t> undoStack;
        CUtlVector<UndoActionData_t> redoStack;
        int maxUndoActions;
        float lastActionTime;

        PlayerUndoData_t()
        {
            maxUndoActions = 100;
            lastActionTime = 0.0f;
        }
    };

    static CUtlVector<PlayerUndoData_t> s_PlayerUndoData;
    static int s_MaxUndoActions;
    static float s_UndoTimeLimit;
    static bool s_bSystemInitialized;

    // Helper functions
    static PlayerUndoData_t* GetPlayerUndoData(CBasePlayer* pPlayer);
    static void CleanupOldActions(PlayerUndoData_t* pData);
    static bool RestoreEntityState(const UndoActionData_t& actionData);
    static bool PerformUndoAction(const UndoActionData_t& actionData);
    static bool PerformRedoAction(const UndoActionData_t& actionData);
    static void CaptureEntityState(CBaseEntity* pEntity, UndoActionData_t& actionData);
};

// Global instance
extern CGModUndoSystem g_GMod_UndoSystem;

// Console command handlers - discovered from IDA string analysis
void CMD_gmod_undo(void);
void CMD_gmod_redo(void);
void CMD_gmod_undo_all(void);
void CMD_gmod_undo_clear(void);
void CMD_gmod_undo_list(void);
void CMD_gmod_undo_count(void);
void CMD_gmod_undo_maxactions(void);
void CMD_gmod_undo_info(void);

// Advanced undo commands
void CMD_gmod_undo_entity(void);
void CMD_gmod_undo_constraints(void);
void CMD_gmod_undo_tool(void);
void CMD_gmod_undo_last_spawn(void);
void CMD_gmod_undo_last_weld(void);
void CMD_gmod_undo_last_constraint(void);

// Undo system events
void CMD_gmod_undo_enable(void);
void CMD_gmod_undo_disable(void);
void CMD_gmod_undo_toggle(void);
void CMD_gmod_undo_autosave(void);

#endif // GMOD_UNDO_H