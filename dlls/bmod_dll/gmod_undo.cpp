#include "gmod_undo.h"
#include "cbase.h"
#include "player.h"
#include "physics.h"
#include "cmodel.h"
#include "tier0/memdbgon.h"

// Define missing flag for LeakNet compatibility
#ifndef FCVAR_GAMEDLL
#define FCVAR_GAMEDLL 0
#endif

// Static member definitions
CUtlVector<CGModUndoSystem::PlayerUndoData_t> CGModUndoSystem::s_PlayerUndoData;
int CGModUndoSystem::s_MaxUndoActions = 100;
float CGModUndoSystem::s_UndoTimeLimit = 300.0f; // 5 minutes
bool CGModUndoSystem::s_bSystemInitialized = false;

// Global instance
CGModUndoSystem g_GMod_UndoSystem;

// ConVars for undo system configuration
ConVar gmod_undo_enabled("gmod_undo_enabled", "1", FCVAR_GAMEDLL, "Enable/disable undo system");
ConVar gmod_undo_maxactions("gmod_undo_maxactions", "100", FCVAR_GAMEDLL, "Maximum number of undo actions per player");
ConVar gmod_undo_timelimit("gmod_undo_timelimit", "300", FCVAR_GAMEDLL, "Time limit for undo actions (seconds)");
ConVar gmod_undo_autosave("gmod_undo_autosave", "1", FCVAR_GAMEDLL, "Automatically save undo actions");
ConVar gmod_undo_debug("gmod_undo_debug", "0", FCVAR_GAMEDLL, "Debug undo system actions");

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
// CGModUndoSystem implementation
//-----------------------------------------------------------------------------
bool CGModUndoSystem::Init()
{
    s_PlayerUndoData.Purge();
    s_MaxUndoActions = gmod_undo_maxactions.GetInt();
    s_UndoTimeLimit = gmod_undo_timelimit.GetFloat();
    s_bSystemInitialized = true;

    DevMsg("GMod Undo System initialized (max actions: %d, time limit: %.1f seconds)\n",
           s_MaxUndoActions, s_UndoTimeLimit);

    return true;
}

void CGModUndoSystem::Shutdown()
{
    for (int i = 0; i < s_PlayerUndoData.Count(); i++)
    {
        s_PlayerUndoData[i].undoStack.Purge();
        s_PlayerUndoData[i].redoStack.Purge();
    }
    s_PlayerUndoData.Purge();
    s_bSystemInitialized = false;

    DevMsg("GMod Undo System shutdown\n");
}

void CGModUndoSystem::LevelInitPostEntity()
{
    // Clear all undo data when level changes
    for (int i = 0; i < s_PlayerUndoData.Count(); i++)
    {
        s_PlayerUndoData[i].undoStack.Purge();
        s_PlayerUndoData[i].redoStack.Purge();
    }

    DevMsg("GMod Undo System: Level initialized, undo history cleared\n");
}

void CGModUndoSystem::FrameUpdatePreEntityThink()
{
    if (!s_bSystemInitialized)
        return;

    // Cleanup old undo actions periodically
    static float nextCleanupTime = 0.0f;
    if (gpGlobals->curtime > nextCleanupTime)
    {
        for (int i = 0; i < s_PlayerUndoData.Count(); i++)
        {
            CleanupOldActions(&s_PlayerUndoData[i]);
        }
        nextCleanupTime = gpGlobals->curtime + 10.0f; // Cleanup every 10 seconds
    }
}

CGModUndoSystem::PlayerUndoData_t* CGModUndoSystem::GetPlayerUndoData(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return NULL;

    int playerIndex = pPlayer->entindex();

    // Ensure we have enough slots
    while (s_PlayerUndoData.Count() <= playerIndex)
    {
        s_PlayerUndoData.AddToTail();
    }

    return &s_PlayerUndoData[playerIndex];
}

void CGModUndoSystem::CleanupOldActions(PlayerUndoData_t* pData)
{
    if (!pData)
        return;

    float currentTime = gpGlobals->curtime;

    // Remove actions older than time limit
    for (int i = pData->undoStack.Count() - 1; i >= 0; i--)
    {
        if (currentTime - pData->undoStack[i].timeStamp > s_UndoTimeLimit)
        {
            pData->undoStack.Remove(i);
        }
    }

    // Limit stack size
    while (pData->undoStack.Count() > s_MaxUndoActions)
    {
        pData->undoStack.Remove(0);
    }
}

void CGModUndoSystem::CaptureEntityState(CBaseEntity* pEntity, UndoActionData_t& actionData)
{
    if (!pEntity)
        return;

    actionData.entityIndex = pEntity->entindex();
    actionData.position = pEntity->GetAbsOrigin();
    actionData.angles = pEntity->GetAbsAngles();

    IPhysicsObject* pPhys = pEntity->VPhysicsGetObject();
    if (pPhys)
    {
        pPhys->GetVelocity(&actionData.velocity, &actionData.angularVelocity);
    }

    color32 renderColor = pEntity->GetRenderColor();
    actionData.color = Color(renderColor.r, renderColor.g, renderColor.b, renderColor.a);
    actionData.renderMode = pEntity->GetRenderMode();
    actionData.renderFX = pEntity->GetRenderFX();

    if (pEntity->GetModel())
    {
        // Store model info - simplified approach for LeakNet compatibility
        Q_strncpy(actionData.modelName, "model", sizeof(actionData.modelName));
    }

    actionData.timeStamp = gpGlobals->curtime;
    actionData.isValid = true;
}

void CGModUndoSystem::AddUndoAction(CBasePlayer* pPlayer, UndoActionType_t actionType, CBaseEntity* pEntity)
{
    if (!gmod_undo_enabled.GetBool() || !pPlayer)
        return;

    UndoActionData_t actionData;
    actionData.actionType = actionType;
    actionData.playerIndex = pPlayer->entindex();

    if (pEntity)
    {
        CaptureEntityState(pEntity, actionData);
    }

    AddUndoAction(pPlayer, actionData);
}

void CGModUndoSystem::AddUndoAction(CBasePlayer* pPlayer, const UndoActionData_t& actionData)
{
    if (!gmod_undo_enabled.GetBool() || !pPlayer)
        return;

    PlayerUndoData_t* pData = GetPlayerUndoData(pPlayer);
    if (!pData)
        return;

    // Clear redo stack when new action is added
    pData->redoStack.Purge();

    // Add to undo stack
    pData->undoStack.AddToTail(actionData);
    pData->lastActionTime = gpGlobals->curtime;

    // Limit stack size
    while (pData->undoStack.Count() > s_MaxUndoActions)
    {
        pData->undoStack.Remove(0);
    }

    if (gmod_undo_debug.GetBool())
    {
        DevMsg("Undo action added for player %s: type %d, entity %d\n",
               STRING(pPlayer->pl.netname), actionData.actionType, actionData.entityIndex);
    }
}

bool CGModUndoSystem::UndoLastAction(CBasePlayer* pPlayer)
{
    if (!gmod_undo_enabled.GetBool() || !pPlayer)
        return false;

    PlayerUndoData_t* pData = GetPlayerUndoData(pPlayer);
    if (!pData || pData->undoStack.Count() == 0)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Nothing to undo");
        return false;
    }

    // Get last action
    UndoActionData_t actionData = pData->undoStack[pData->undoStack.Count() - 1];
    pData->undoStack.Remove(pData->undoStack.Count() - 1);

    // Perform undo
    bool success = PerformUndoAction(actionData);

    if (success)
    {
        // Add to redo stack
        pData->redoStack.AddToTail(actionData);
        ClientPrint(pPlayer, HUD_PRINTTALK, "Action undone");

        if (gmod_undo_debug.GetBool())
        {
            DevMsg("Undo performed for player %s: type %d\n", STRING(pPlayer->pl.netname), actionData.actionType);
        }
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to undo action");
    }

    return success;
}

bool CGModUndoSystem::RedoLastAction(CBasePlayer* pPlayer)
{
    if (!gmod_undo_enabled.GetBool() || !pPlayer)
        return false;

    PlayerUndoData_t* pData = GetPlayerUndoData(pPlayer);
    if (!pData || pData->redoStack.Count() == 0)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Nothing to redo");
        return false;
    }

    // Get last redo action
    UndoActionData_t actionData = pData->redoStack[pData->redoStack.Count() - 1];
    pData->redoStack.Remove(pData->redoStack.Count() - 1);

    // Perform redo
    bool success = PerformRedoAction(actionData);

    if (success)
    {
        // Add back to undo stack
        pData->undoStack.AddToTail(actionData);
        ClientPrint(pPlayer, HUD_PRINTTALK, "Action redone");

        if (gmod_undo_debug.GetBool())
        {
            DevMsg("Redo performed for player %s: type %d\n", STRING(pPlayer->pl.netname), actionData.actionType);
        }
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to redo action");
    }

    return success;
}

bool CGModUndoSystem::PerformUndoAction(const UndoActionData_t& actionData)
{
    switch (actionData.actionType)
    {
        case UNDO_ACTION_ENTITY_CREATE:
        {
            // Remove the entity that was created
            CBaseEntity* pEntity = UTIL_EntityByIndex(actionData.entityIndex);
            if (pEntity)
            {
                UTIL_Remove(pEntity);
                return true;
            }
            break;
        }

        case UNDO_ACTION_ENTITY_REMOVE:
        {
            // Recreate the entity that was removed (if possible)
            // This is complex and may not always be possible
            break;
        }

        case UNDO_ACTION_PROPERTY_CHANGE:
        case UNDO_ACTION_COLOR_CHANGE:
        case UNDO_ACTION_POSITION_CHANGE:
        case UNDO_ACTION_ANGLE_CHANGE:
        {
            // Restore entity state
            return RestoreEntityState(actionData);
        }

        case UNDO_ACTION_CONSTRAINT_CREATE:
        {
            // Remove the constraint that was created
            // This would require tracking constraint IDs
            break;
        }

        default:
            break;
    }

    return false;
}

bool CGModUndoSystem::PerformRedoAction(const UndoActionData_t& actionData)
{
    // Redo is typically the reverse of undo
    switch (actionData.actionType)
    {
        case UNDO_ACTION_ENTITY_CREATE:
        {
            // Recreate the entity (if possible)
            break;
        }

        case UNDO_ACTION_ENTITY_REMOVE:
        {
            // Remove the entity again
            CBaseEntity* pEntity = UTIL_EntityByIndex(actionData.entityIndex);
            if (pEntity)
            {
                UTIL_Remove(pEntity);
                return true;
            }
            break;
        }

        default:
            break;
    }

    return false;
}

bool CGModUndoSystem::RestoreEntityState(const UndoActionData_t& actionData)
{
    CBaseEntity* pEntity = UTIL_EntityByIndex(actionData.entityIndex);
    if (!pEntity)
        return false;

    // Restore position and angles
    pEntity->SetAbsOrigin(actionData.position);
    pEntity->SetAbsAngles(actionData.angles);

    // Restore physics velocity
    IPhysicsObject* pPhys = pEntity->VPhysicsGetObject();
    if (pPhys)
    {
        pPhys->SetVelocity(&actionData.velocity, &actionData.angularVelocity);
    }

    // Restore rendering properties
    int r, g, b, a;
    actionData.color.GetColor(r, g, b, a);
    pEntity->SetRenderColor(r, g, b, a);
    pEntity->SetRenderMode((RenderMode_t)actionData.renderMode);
    pEntity->SetRenderFX((RenderFx_t)actionData.renderFX);

    return true;
}

void CGModUndoSystem::UndoAll(CBasePlayer* pPlayer)
{
    if (!gmod_undo_enabled.GetBool() || !pPlayer)
        return;

    PlayerUndoData_t* pData = GetPlayerUndoData(pPlayer);
    if (!pData)
        return;

    int undoCount = 0;
    while (pData->undoStack.Count() > 0)
    {
        if (UndoLastAction(pPlayer))
            undoCount++;
        else
            break; // Stop if undo fails
    }

    char msg[256];
    Q_snprintf(msg, sizeof(msg), "Undid %d actions", undoCount);
    ClientPrint(pPlayer, HUD_PRINTTALK, msg);
}

void CGModUndoSystem::ClearUndoHistory(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    PlayerUndoData_t* pData = GetPlayerUndoData(pPlayer);
    if (!pData)
        return;

    pData->undoStack.Purge();
    pData->redoStack.Purge();

    ClientPrint(pPlayer, HUD_PRINTTALK, "Undo history cleared");
}

int CGModUndoSystem::GetUndoStackSize(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return 0;

    PlayerUndoData_t* pData = GetPlayerUndoData(pPlayer);
    if (!pData)
        return 0;

    return pData->undoStack.Count();
}

int CGModUndoSystem::GetRedoStackSize(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return 0;

    PlayerUndoData_t* pData = GetPlayerUndoData(pPlayer);
    if (!pData)
        return 0;

    return pData->redoStack.Count();
}

void CGModUndoSystem::ListUndoActions(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    PlayerUndoData_t* pData = GetPlayerUndoData(pPlayer);
    if (!pData)
        return;

    char msg[256];

    ClientPrint(pPlayer, HUD_PRINTTALK, "=== Undo History ===");

    Q_snprintf(msg, sizeof(msg), "Undo actions: %d", pData->undoStack.Count());
    ClientPrint(pPlayer, HUD_PRINTTALK, msg);

    Q_snprintf(msg, sizeof(msg), "Redo actions: %d", pData->redoStack.Count());
    ClientPrint(pPlayer, HUD_PRINTTALK, msg);

    for (int i = pData->undoStack.Count() - 1; i >= MAX(0, pData->undoStack.Count() - 5); i--)
    {
        const UndoActionData_t& action = pData->undoStack[i];
        Q_snprintf(msg, sizeof(msg), "%d. Type: %d, Entity: %d, Time: %.1f",
                   i + 1, action.actionType, action.entityIndex, gpGlobals->curtime - action.timeStamp);
        ClientPrint(pPlayer, HUD_PRINTTALK, msg);
    }
}

void CGModUndoSystem::RecordEntitySpawn(CBasePlayer* pPlayer, CBaseEntity* pEntity)
{
    AddUndoAction(pPlayer, UNDO_ACTION_ENTITY_CREATE, pEntity);
}

void CGModUndoSystem::RecordEntityRemoval(CBasePlayer* pPlayer, CBaseEntity* pEntity)
{
    AddUndoAction(pPlayer, UNDO_ACTION_ENTITY_REMOVE, pEntity);
}

void CGModUndoSystem::RecordConstraintCreation(CBasePlayer* pPlayer, int constraintId, CBaseEntity* pEnt1, CBaseEntity* pEnt2)
{
    UndoActionData_t actionData;
    actionData.actionType = UNDO_ACTION_CONSTRAINT_CREATE;
    actionData.playerIndex = pPlayer->entindex();
    actionData.constraintId = constraintId;
    actionData.entityIndex = pEnt1 ? pEnt1->entindex() : -1;
    actionData.timeStamp = gpGlobals->curtime;
    actionData.isValid = true;

    AddUndoAction(pPlayer, actionData);
}

void CGModUndoSystem::RecordConstraintRemoval(CBasePlayer* pPlayer, int constraintId)
{
    UndoActionData_t actionData;
    actionData.actionType = UNDO_ACTION_CONSTRAINT_REMOVE;
    actionData.playerIndex = pPlayer->entindex();
    actionData.constraintId = constraintId;
    actionData.timeStamp = gpGlobals->curtime;
    actionData.isValid = true;

    AddUndoAction(pPlayer, actionData);
}

void CGModUndoSystem::RecordPropertyChange(CBasePlayer* pPlayer, CBaseEntity* pEntity, UndoActionType_t changeType)
{
    AddUndoAction(pPlayer, changeType, pEntity);
}

void CGModUndoSystem::OnPlayerConnect(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    PlayerUndoData_t* pData = GetPlayerUndoData(pPlayer);
    if (pData)
    {
        pData->undoStack.Purge();
        pData->redoStack.Purge();
        pData->maxUndoActions = s_MaxUndoActions;
        pData->lastActionTime = gpGlobals->curtime;
    }

    DevMsg("Undo system initialized for player %s\n", STRING(pPlayer->pl.netname));
}

void CGModUndoSystem::OnPlayerDisconnect(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    PlayerUndoData_t* pData = GetPlayerUndoData(pPlayer);
    if (pData)
    {
        pData->undoStack.Purge();
        pData->redoStack.Purge();
    }

    DevMsg("Undo system cleaned up for player %s\n", STRING(pPlayer->pl.netname));
}

//-----------------------------------------------------------------------------
// Console command implementations
//-----------------------------------------------------------------------------
void CMD_gmod_undo(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModUndoSystem::UndoLastAction(pPlayer);
}

void CMD_gmod_redo(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModUndoSystem::RedoLastAction(pPlayer);
}

void CMD_gmod_undo_all(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModUndoSystem::UndoAll(pPlayer);
}

void CMD_gmod_undo_clear(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModUndoSystem::ClearUndoHistory(pPlayer);
}

void CMD_gmod_undo_list(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModUndoSystem::ListUndoActions(pPlayer);
}

void CMD_gmod_undo_count(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    int undoCount = CGModUndoSystem::GetUndoStackSize(pPlayer);
    int redoCount = CGModUndoSystem::GetRedoStackSize(pPlayer);

    char msg[256];
    Q_snprintf(msg, sizeof(msg), "Undo actions: %d, Redo actions: %d", undoCount, redoCount);
    ClientPrint(pPlayer, HUD_PRINTTALK, msg);
}

void CMD_gmod_undo_info(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char msg[256];

    ClientPrint(pPlayer, HUD_PRINTTALK, "=== Undo System Info ===");

    Q_snprintf(msg, sizeof(msg), "Enabled: %s", gmod_undo_enabled.GetBool() ? "Yes" : "No");
    ClientPrint(pPlayer, HUD_PRINTTALK, msg);

    Q_snprintf(msg, sizeof(msg), "Max actions: %d", gmod_undo_maxactions.GetInt());
    ClientPrint(pPlayer, HUD_PRINTTALK, msg);

    Q_snprintf(msg, sizeof(msg), "Time limit: %.1f seconds", gmod_undo_timelimit.GetFloat());
    ClientPrint(pPlayer, HUD_PRINTTALK, msg);

    Q_snprintf(msg, sizeof(msg), "Your undo count: %d", CGModUndoSystem::GetUndoStackSize(pPlayer));
    ClientPrint(pPlayer, HUD_PRINTTALK, msg);

    Q_snprintf(msg, sizeof(msg), "Your redo count: %d", CGModUndoSystem::GetRedoStackSize(pPlayer));
    ClientPrint(pPlayer, HUD_PRINTTALK, msg);
}

void CMD_gmod_undo_entity(void)
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

    // Find and undo actions related to this entity
    ClientPrint(pPlayer, HUD_PRINTTALK, "Entity-specific undo not yet implemented");
}

void CMD_gmod_undo_constraints(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Constraint undo not yet implemented");
}

void CMD_gmod_undo_tool(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Tool undo not yet implemented");
}

void CMD_gmod_undo_last_spawn(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Last spawn undo not yet implemented");
}

void CMD_gmod_undo_last_weld(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Last weld undo not yet implemented");
}

void CMD_gmod_undo_last_constraint(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Last constraint undo not yet implemented");
}

void CMD_gmod_undo_enable(void)
{
    gmod_undo_enabled.SetValue(1);
    Msg("Undo system enabled\n");
}

void CMD_gmod_undo_disable(void)
{
    gmod_undo_enabled.SetValue(0);
    Msg("Undo system disabled\n");
}

void CMD_gmod_undo_toggle(void)
{
    bool enabled = gmod_undo_enabled.GetBool();
    gmod_undo_enabled.SetValue(!enabled);
    Msg("Undo system %s\n", !enabled ? "enabled" : "disabled");
}

void CMD_gmod_undo_autosave(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    char msg[256];
    Q_snprintf(msg, sizeof(msg), "Autosave: %s", gmod_undo_autosave.GetBool() ? "enabled" : "disabled");
    ClientPrint(pPlayer, HUD_PRINTTALK, msg);
}

//-----------------------------------------------------------------------------
// Console command registration
//-----------------------------------------------------------------------------
static ConCommand gmod_undo_cmd("gmod_undo", CMD_gmod_undo, "Undo last action");
static ConCommand gmod_redo_cmd("gmod_redo", CMD_gmod_redo, "Redo last undone action");
static ConCommand gmod_undo_all_cmd("gmod_undo_all", CMD_gmod_undo_all, "Undo all actions");
static ConCommand gmod_undo_clear_cmd("gmod_undo_clear", CMD_gmod_undo_clear, "Clear undo history");
static ConCommand gmod_undo_list_cmd("gmod_undo_list", CMD_gmod_undo_list, "List recent undo actions");
static ConCommand gmod_undo_count_cmd("gmod_undo_count", CMD_gmod_undo_count, "Show undo/redo count");
static ConCommand gmod_undo_info_cmd("gmod_undo_info", CMD_gmod_undo_info, "Show undo system information");

// Advanced undo commands
static ConCommand gmod_undo_entity_cmd("gmod_undo_entity", CMD_gmod_undo_entity, "Undo actions for specific entity");
static ConCommand gmod_undo_constraints_cmd("gmod_undo_constraints", CMD_gmod_undo_constraints, "Undo constraint actions");
static ConCommand gmod_undo_tool_cmd("gmod_undo_tool", CMD_gmod_undo_tool, "Undo tool actions");
static ConCommand gmod_undo_last_spawn_cmd("gmod_undo_last_spawn", CMD_gmod_undo_last_spawn, "Undo last spawn");
static ConCommand gmod_undo_last_weld_cmd("gmod_undo_last_weld", CMD_gmod_undo_last_weld, "Undo last weld");
static ConCommand gmod_undo_last_constraint_cmd("gmod_undo_last_constraint", CMD_gmod_undo_last_constraint, "Undo last constraint");

// System control commands
static ConCommand gmod_undo_enable_cmd("gmod_undo_enable", CMD_gmod_undo_enable, "Enable undo system");
static ConCommand gmod_undo_disable_cmd("gmod_undo_disable", CMD_gmod_undo_disable, "Disable undo system");
static ConCommand gmod_undo_toggle_cmd("gmod_undo_toggle", CMD_gmod_undo_toggle, "Toggle undo system");
static ConCommand gmod_undo_autosave_cmd("gmod_undo_autosave", CMD_gmod_undo_autosave, "Show autosave status");