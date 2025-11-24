#include "gmod_tools.h"
#include "cbase.h"
#include "player.h"
#include "physics.h"
#include "gmod_weld.h"
#include "gmod_undo.h"
#include "tier0/memdbgon.h"

// Define missing flag for LeakNet compatibility
#ifndef FCVAR_GAMEDLL
#define FCVAR_GAMEDLL 0
#endif

// Static member definitions
CUtlVector<CGModToolsSystem::PlayerToolData_t> CGModToolsSystem::s_PlayerToolData;
CUtlVector<GMToolData_t> CGModToolsSystem::s_ToolRegistry;
bool CGModToolsSystem::s_bSystemInitialized = false;

// Global instance
CGModToolsSystem g_GMod_ToolsSystem;

// ConVars for tool system configuration - discovered from IDA analysis
ConVar gm_toolweapon("gm_toolweapon", "1", FCVAR_GAMEDLL, "Current tool weapon selection");
ConVar gm_toolmode("gm_toolmode", "0", FCVAR_GAMEDLL, "Current tool mode");
ConVar gm_wepselmode("gm_wepselmode", "0", FCVAR_GAMEDLL, "Weapon selection mode");
ConVar gm_tool_delay("gm_tool_delay", "0.5", FCVAR_GAMEDLL, "Delay between tool uses");
ConVar gm_tool_range("gm_tool_range", "4096", FCVAR_GAMEDLL, "Maximum tool range");
ConVar gm_context_enabled("gm_context_enabled", "1", FCVAR_GAMEDLL, "Enable context menu system");

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
// Helper function to get arguments from console command
//-----------------------------------------------------------------------------
static const char* GetCommandArg(int index)
{
    return engine->Cmd_Argv(index);
}

static int GetCommandArgCount()
{
    return engine->Cmd_Argc();
}

//-----------------------------------------------------------------------------
// CGModToolsSystem implementation
//-----------------------------------------------------------------------------
bool CGModToolsSystem::Init()
{
    s_PlayerToolData.Purge();
    s_ToolRegistry.Purge();

    InitializeToolRegistry();
    LoadToolConfiguration();
    s_bSystemInitialized = true;

    DevMsg("GMod Tools System initialized with %d tools\n", s_ToolRegistry.Count());
    return true;
}

void CGModToolsSystem::Shutdown()
{
    SaveToolConfiguration();
    s_PlayerToolData.Purge();
    s_ToolRegistry.Purge();
    s_bSystemInitialized = false;

    DevMsg("GMod Tools System shutdown\n");
}

void CGModToolsSystem::LevelInitPostEntity()
{
    // Reset player tool data on level change
    for (int i = 0; i < s_PlayerToolData.Count(); i++)
    {
        s_PlayerToolData[i].lastToolUse = 0.0f;
        s_PlayerToolData[i].toolUseCount = 0;
        s_PlayerToolData[i].contextMenuOpen = false;
    }

    DevMsg("GMod Tools System: Level initialized, player data reset\n");
}

CGModToolsSystem::PlayerToolData_t* CGModToolsSystem::GetPlayerToolData(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return NULL;

    int playerIndex = pPlayer->entindex();

    // Ensure we have enough slots
    while (s_PlayerToolData.Count() <= playerIndex)
    {
        s_PlayerToolData.AddToTail();
    }

    return &s_PlayerToolData[playerIndex];
}

void CGModToolsSystem::InitializeToolRegistry()
{
    // Initialize all tool types with default data
    for (int i = 0; i < TOOL_MAX; i++)
    {
        GMToolData_t toolData;
        toolData.toolType = (GMToolType_t)i;

        switch (i)
        {
            case TOOL_WELD:
                Q_strncpy(toolData.toolName, "weld", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Weld Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Welds two objects together", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
            case TOOL_AXIS:
                Q_strncpy(toolData.toolName, "axis", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Axis Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Creates axis constraint", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
            case TOOL_BALLSOCKET:
                Q_strncpy(toolData.toolName, "ballsocket", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Ball Socket Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Creates ball socket joint", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
            case TOOL_ROPE:
                Q_strncpy(toolData.toolName, "rope", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Rope Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Creates rope constraint", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
            case TOOL_SPRING:
                Q_strncpy(toolData.toolName, "spring", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Spring Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Creates spring constraint", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
            case TOOL_HYDRAULIC:
                Q_strncpy(toolData.toolName, "hydraulic", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Hydraulic Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Creates hydraulic constraint", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
            case TOOL_MOTOR:
                Q_strncpy(toolData.toolName, "motor", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Motor Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Creates motor constraint", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
            case TOOL_PULLEY:
                Q_strncpy(toolData.toolName, "pulley", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Pulley Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Creates pulley constraint", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
            case TOOL_KEEPUPRIGHT:
                Q_strncpy(toolData.toolName, "keepupright", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Keep Upright Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Keeps object upright", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
            case TOOL_NOCOLLIDE:
                Q_strncpy(toolData.toolName, "nocollide", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "No Collide Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Disables collision between objects", sizeof(toolData.description));
                toolData.delay = 0.2f;
                break;
            case TOOL_THRUSTER:
                Q_strncpy(toolData.toolName, "thruster", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Thruster Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Creates thruster", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
            case TOOL_WHEEL:
                Q_strncpy(toolData.toolName, "wheel", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Wheel Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Creates wheel", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
            case TOOL_REMOVER:
                Q_strncpy(toolData.toolName, "remover", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Remover Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Removes entities", sizeof(toolData.description));
                toolData.delay = 0.1f;
                break;
            case TOOL_IGNITE:
                Q_strncpy(toolData.toolName, "ignite", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Ignite Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Ignites entities", sizeof(toolData.description));
                toolData.delay = 0.3f;
                break;
            case TOOL_CAMERA:
                Q_strncpy(toolData.toolName, "camera", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Camera Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Creates camera", sizeof(toolData.description));
                toolData.delay = 1.0f;
                break;
            default:
                Q_strncpy(toolData.toolName, "unknown", sizeof(toolData.toolName));
                Q_strncpy(toolData.displayName, "Unknown Tool", sizeof(toolData.displayName));
                Q_strncpy(toolData.description, "Unknown tool type", sizeof(toolData.description));
                toolData.delay = 0.5f;
                break;
        }

        s_ToolRegistry.AddToTail(toolData);
    }
}

void CGModToolsSystem::SetPlayerTool(CBasePlayer* pPlayer, GMToolType_t toolType)
{
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return;

    pData->currentTool = toolType;
    gm_toolweapon.SetValue((int)toolType);

    if (pPlayer)
    {
        const char* toolName = GetToolName(toolType);
        char szMessage[128];
        Q_snprintf(szMessage, sizeof(szMessage), "Selected tool: %s", toolName);
        ClientPrint(pPlayer, HUD_PRINTTALK, szMessage);
    }
}

GMToolType_t CGModToolsSystem::GetPlayerTool(CBasePlayer* pPlayer)
{
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return TOOL_WELD;

    return pData->currentTool;
}

void CGModToolsSystem::SetPlayerToolMode(CBasePlayer* pPlayer, int mode)
{
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return;

    pData->toolMode = mode;
    gm_toolmode.SetValue(mode);

    if (pPlayer)
    {
        char szMessage[128];
        Q_snprintf(szMessage, sizeof(szMessage), "Tool mode set to: %d", mode);
        ClientPrint(pPlayer, HUD_PRINTTALK, szMessage);
    }
}

int CGModToolsSystem::GetPlayerToolMode(CBasePlayer* pPlayer)
{
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return 0;

    return pData->toolMode;
}

void CGModToolsSystem::SetPlayerContext(CBasePlayer* pPlayer, GMContextType_t contextType)
{
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return;

    pData->contextType = contextType;

    if (pPlayer)
    {
        const char* contextName = GetContextName(contextType);
        char szMessage[128];
        Q_snprintf(szMessage, sizeof(szMessage), "Context set to: %s", contextName);
        ClientPrint(pPlayer, HUD_PRINTTALK, szMessage);
    }
}

GMContextType_t CGModToolsSystem::GetPlayerContext(CBasePlayer* pPlayer)
{
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return CONTEXT_NONE;

    return pData->contextType;
}

bool CGModToolsSystem::ExecuteTool(CBasePlayer* pPlayer, CBaseEntity* pTarget, const Vector& tracePos, const Vector& traceNormal)
{
    if (!pPlayer || !ValidateToolUse(pPlayer, GetPlayerTool(pPlayer)))
        return false;

    GMToolType_t toolType = GetPlayerTool(pPlayer);
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return false;

    // Check cooldown
    float delay = gm_tool_delay.GetFloat();
    GMToolData_t* pToolData = GetToolData(toolType);
    if (pToolData)
        delay = pToolData->delay;

    if (gpGlobals->curtime - pData->lastToolUse < delay)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Tool is on cooldown");
        return false;
    }

    bool success = false;

    // Execute tool based on type
    switch (toolType)
    {
        case TOOL_WELD:
        {
            if (pTarget)
            {
                // Create weld constraint using the weld system
                WeldConstraintData_t weldData;
                weldData.constraintType = WELD_CONSTRAINT_WELD;
                weldData.position1 = tracePos;
                weldData.position2 = tracePos;
                weldData.forceLimit = 0.0f;  // Unlimited
                weldData.torqueLimit = 0.0f; // Unlimited

                // Find second entity for welding (stored from previous use)
                static CBaseEntity* s_pFirstEntity = NULL;
                static CBasePlayer* s_pFirstUser = NULL;

                if (s_pFirstEntity && s_pFirstUser == pPlayer)
                {
                    CGModWeldSystem::CreateWeldConstraint(s_pFirstEntity, pTarget, weldData);
                    CGModUndoSystem::RecordConstraintCreation(pPlayer, 0, s_pFirstEntity, pTarget);
                    ClientPrint(pPlayer, HUD_PRINTTALK, "Welded entities together");
                    s_pFirstEntity = NULL;
                    s_pFirstUser = NULL;
                    success = true;
                }
                else
                {
                    s_pFirstEntity = pTarget;
                    s_pFirstUser = pPlayer;
                    ClientPrint(pPlayer, HUD_PRINTTALK, "First entity selected. Select second entity to weld.");
                    success = true;
                }
            }
            break;
        }

        case TOOL_REMOVER:
        {
            if (pTarget)
            {
                CGModUndoSystem::RecordEntityRemoval(pPlayer, pTarget);
                UTIL_Remove(pTarget);
                ClientPrint(pPlayer, HUD_PRINTTALK, "Entity removed");
                success = true;
            }
            break;
        }

        case TOOL_IGNITE:
        {
            if (pTarget)
            {
                CBaseAnimating* pAnimating = dynamic_cast<CBaseAnimating*>(pTarget);
                if (pAnimating)
                {
                    pAnimating->Ignite(30.0f);
                    ClientPrint(pPlayer, HUD_PRINTTALK, "Entity ignited");
                    success = true;
                }
                else
                {
                    ClientPrint(pPlayer, HUD_PRINTTALK, "Entity cannot be ignited");
                }
            }
            break;
        }

        case TOOL_NOCOLLIDE:
        {
            if (pTarget)
            {
                // Disable collision
                pTarget->SetCollisionGroup(COLLISION_GROUP_DEBRIS);
                ClientPrint(pPlayer, HUD_PRINTTALK, "Collision disabled");
                success = true;
            }
            break;
        }

        default:
        {
            char szMessage[128];
            Q_snprintf(szMessage, sizeof(szMessage), "Tool not yet implemented: %s", GetToolName(toolType));
            ClientPrint(pPlayer, HUD_PRINTTALK, szMessage);
            break;
        }
    }

    if (success)
    {
        pData->lastToolUse = gpGlobals->curtime;
        pData->toolUseCount++;
    }

    return success;
}

bool CGModToolsSystem::ValidateToolUse(CBasePlayer* pPlayer, GMToolType_t toolType)
{
    if (!pPlayer || !pPlayer->IsAlive())
        return false;

    if (toolType < 0 || toolType >= TOOL_MAX)
        return false;

    return true;
}

const char* CGModToolsSystem::GetToolName(GMToolType_t toolType)
{
    GMToolData_t* pToolData = GetToolData(toolType);
    if (pToolData)
        return pToolData->displayName;

    return "Unknown";
}

const char* CGModToolsSystem::GetContextName(GMContextType_t contextType)
{
    switch (contextType)
    {
        case CONTEXT_NPC: return "NPC";
        case CONTEXT_CAMERA: return "Camera";
        case CONTEXT_ENTITY: return "Entity";
        case CONTEXT_WORLD: return "World";
        default: return "None";
    }
}

GMToolData_t* CGModToolsSystem::GetToolData(GMToolType_t toolType)
{
    if (toolType < 0 || toolType >= s_ToolRegistry.Count())
        return NULL;

    return &s_ToolRegistry[toolType];
}

void CGModToolsSystem::LoadToolConfiguration()
{
    // Tool configuration would be loaded from files here
    DevMsg("Tool configuration loaded\n");
}

void CGModToolsSystem::SaveToolConfiguration()
{
    // Tool configuration would be saved to files here
    DevMsg("Tool configuration saved\n");
}

//-----------------------------------------------------------------------------
// Console command implementations - discovered from IDA analysis
//-----------------------------------------------------------------------------
void CMD_gm_toolweapon(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (GetCommandArgCount() < 2)
    {
        int currentTool = (int)CGModToolsSystem::GetPlayerTool(pPlayer);
        char szMessage[128];
        Q_snprintf(szMessage, sizeof(szMessage), "Current tool: %d (%s)", currentTool, CGModToolsSystem::GetToolName((GMToolType_t)currentTool));
        ClientPrint(pPlayer, HUD_PRINTTALK, szMessage);
        return;
    }

    int toolType = atoi(GetCommandArg(1));
    if (toolType >= 0 && toolType < TOOL_MAX)
    {
        CGModToolsSystem::SetPlayerTool(pPlayer, (GMToolType_t)toolType);
    }
    else
    {
        char szMessage[128];
        Q_snprintf(szMessage, sizeof(szMessage), "Invalid tool type: %d (range: 0-%d)", toolType, TOOL_MAX - 1);
        ClientPrint(pPlayer, HUD_PRINTTALK, szMessage);
    }
}

void CMD_gm_toolmode(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (GetCommandArgCount() < 2)
    {
        int currentMode = CGModToolsSystem::GetPlayerToolMode(pPlayer);
        char szMessage[128];
        Q_snprintf(szMessage, sizeof(szMessage), "Current tool mode: %d", currentMode);
        ClientPrint(pPlayer, HUD_PRINTTALK, szMessage);
        return;
    }

    int mode = atoi(GetCommandArg(1));
    CGModToolsSystem::SetPlayerToolMode(pPlayer, mode);
}

void CMD_gm_context(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (GetCommandArgCount() < 2)
    {
        GMContextType_t currentContext = CGModToolsSystem::GetPlayerContext(pPlayer);
        char szMessage[128];
        Q_snprintf(szMessage, sizeof(szMessage), "Current context: %s", CGModToolsSystem::GetContextName(currentContext));
        ClientPrint(pPlayer, HUD_PRINTTALK, szMessage);
        return;
    }

    const char* contextName = GetCommandArg(1);
    GMContextType_t contextType = CONTEXT_NONE;

    if (Q_stricmp(contextName, "npc") == 0)
        contextType = CONTEXT_NPC;
    else if (Q_stricmp(contextName, "camera") == 0)
        contextType = CONTEXT_CAMERA;
    else if (Q_stricmp(contextName, "entity") == 0)
        contextType = CONTEXT_ENTITY;
    else if (Q_stricmp(contextName, "world") == 0)
        contextType = CONTEXT_WORLD;

    CGModToolsSystem::SetPlayerContext(pPlayer, contextType);
}

void CMD_gm_wepselmode(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (GetCommandArgCount() < 2)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Usage: gm_wepselmode <0-2>");
        return;
    }

    int mode = atoi(GetCommandArg(1));
    if (mode >= 0 && mode < WEAPONSEL_MAX)
    {
        CGModToolsSystem::SetWeaponSelectionMode(pPlayer, (GMWeaponSelMode_t)mode);
        char szMessage[128];
        Q_snprintf(szMessage, sizeof(szMessage), "Weapon selection mode set to: %d", mode);
        ClientPrint(pPlayer, HUD_PRINTTALK, szMessage);
    }
    else
    {
        char szMessage[128];
        Q_snprintf(szMessage, sizeof(szMessage), "Invalid weapon selection mode: %d (range: 0-%d)", mode, WEAPONSEL_MAX - 1);
        ClientPrint(pPlayer, HUD_PRINTTALK, szMessage);
    }
}

// Button command implementations (discovered gm_button%i pattern in IDA)
void CMD_gm_button0(void) { CGModToolsSystem::SetPlayerToolMode(GetCommandPlayer(), 0); }
void CMD_gm_button1(void) { CGModToolsSystem::SetPlayerToolMode(GetCommandPlayer(), 1); }
void CMD_gm_button2(void) { CGModToolsSystem::SetPlayerToolMode(GetCommandPlayer(), 2); }
void CMD_gm_button3(void) { CGModToolsSystem::SetPlayerToolMode(GetCommandPlayer(), 3); }
void CMD_gm_button4(void) { CGModToolsSystem::SetPlayerToolMode(GetCommandPlayer(), 4); }
void CMD_gm_button5(void) { CGModToolsSystem::SetPlayerToolMode(GetCommandPlayer(), 5); }
void CMD_gm_button6(void) { CGModToolsSystem::SetPlayerToolMode(GetCommandPlayer(), 6); }
void CMD_gm_button7(void) { CGModToolsSystem::SetPlayerToolMode(GetCommandPlayer(), 7); }
void CMD_gm_button8(void) { CGModToolsSystem::SetPlayerToolMode(GetCommandPlayer(), 8); }
void CMD_gm_button9(void) { CGModToolsSystem::SetPlayerToolMode(GetCommandPlayer(), 9); }

//-----------------------------------------------------------------------------
// Console command registration
//-----------------------------------------------------------------------------
static ConCommand gm_toolweapon_cmd("gm_toolweapon", CMD_gm_toolweapon, "Set or get current tool weapon");
static ConCommand gm_toolmode_cmd("gm_toolmode", CMD_gm_toolmode, "Set or get current tool mode");
static ConCommand gm_context_cmd("gm_context", CMD_gm_context, "Set or get current context");
static ConCommand gm_wepselmode_cmd("gm_wepselmode", CMD_gm_wepselmode, "Set weapon selection mode");

// Button commands
static ConCommand gm_button0_cmd("gm_button0", CMD_gm_button0, "Button 0");
static ConCommand gm_button1_cmd("gm_button1", CMD_gm_button1, "Button 1");
static ConCommand gm_button2_cmd("gm_button2", CMD_gm_button2, "Button 2");
static ConCommand gm_button3_cmd("gm_button3", CMD_gm_button3, "Button 3");
static ConCommand gm_button4_cmd("gm_button4", CMD_gm_button4, "Button 4");
static ConCommand gm_button5_cmd("gm_button5", CMD_gm_button5, "Button 5");
static ConCommand gm_button6_cmd("gm_button6", CMD_gm_button6, "Button 6");
static ConCommand gm_button7_cmd("gm_button7", CMD_gm_button7, "Button 7");
static ConCommand gm_button8_cmd("gm_button8", CMD_gm_button8, "Button 8");
static ConCommand gm_button9_cmd("gm_button9", CMD_gm_button9, "Button 9");

// Tool-specific command implementations discovered from IDA analysis
void CMD_gm_tool_weld(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_WELD); }
void CMD_gm_tool_axis(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_AXIS); }
void CMD_gm_tool_ballsocket(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_BALLSOCKET); }
void CMD_gm_tool_rope(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_ROPE); }
void CMD_gm_tool_spring(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_SPRING); }
void CMD_gm_tool_hydraulic(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_HYDRAULIC); }
void CMD_gm_tool_motor(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_MOTOR); }
void CMD_gm_tool_pulley(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_PULLEY); }
void CMD_gm_tool_keepupright(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_KEEPUPRIGHT); }
void CMD_gm_tool_nocollide(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_NOCOLLIDE); }
void CMD_gm_tool_thruster(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_THRUSTER); }
void CMD_gm_tool_wheel(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_WHEEL); }
void CMD_gm_tool_remover(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_REMOVER); }
void CMD_gm_tool_ignite(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_IGNITE); }
void CMD_gm_tool_camera(void) { CGModToolsSystem::SetPlayerTool(GetCommandPlayer(), TOOL_CAMERA); }

// Context menu command implementations discovered from IDA analysis
void CMD_gm_context_npc(void) { CGModToolsSystem::SetPlayerContext(GetCommandPlayer(), CONTEXT_NPC); }
void CMD_gm_context_camera(void) { CGModToolsSystem::SetPlayerContext(GetCommandPlayer(), CONTEXT_CAMERA); }
void CMD_gm_context_entity(void) { CGModToolsSystem::SetPlayerContext(GetCommandPlayer(), CONTEXT_ENTITY); }
void CMD_gm_context_world(void) { CGModToolsSystem::SetPlayerContext(GetCommandPlayer(), CONTEXT_WORLD); }

// Missing function implementations from the header
void CGModToolsSystem::SetWeaponSelectionMode(CBasePlayer* pPlayer, GMWeaponSelMode_t mode)
{
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return;

    pData->weaponSelMode = mode;
}

GMWeaponSelMode_t CGModToolsSystem::GetWeaponSelectionMode(CBasePlayer* pPlayer)
{
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return WEAPONSEL_NORMAL;

    return pData->weaponSelMode;
}

void CGModToolsSystem::ShowContextMenu(CBasePlayer* pPlayer, CBaseEntity* pTarget)
{
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return;

    pData->contextMenuOpen = true;
    // Context menu display logic would go here
}

void CGModToolsSystem::HideContextMenu(CBasePlayer* pPlayer)
{
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return;

    pData->contextMenuOpen = false;
    // Context menu hide logic would go here
}

bool CGModToolsSystem::CanUseTool(CBasePlayer* pPlayer, GMToolType_t toolType)
{
    return ValidateToolUse(pPlayer, toolType);
}

void CGModToolsSystem::ResetToolCooldown(CBasePlayer* pPlayer)
{
    PlayerToolData_t* pData = GetPlayerToolData(pPlayer);
    if (!pData)
        return;

    pData->lastToolUse = 0.0f;
}

void CGModToolsSystem::RegisterTool(GMToolType_t toolType, const GMToolData_t& toolData)
{
    if (toolType >= 0 && toolType < s_ToolRegistry.Count())
    {
        s_ToolRegistry[toolType] = toolData;
    }
}

CBaseEntity* CGModToolsSystem::GetPlayerTargetEntity(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return NULL;

    Vector forward, start, end;
    AngleVectors(pPlayer->EyeAngles(), &forward);
    start = pPlayer->EyePosition();
    end = start + forward * gm_tool_range.GetFloat();

    trace_t tr;
    UTIL_TraceLine(start, end, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

    return tr.m_pEnt;
}

Vector CGModToolsSystem::GetPlayerTracePosition(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return vec3_origin;

    Vector forward, start, end;
    AngleVectors(pPlayer->EyeAngles(), &forward);
    start = pPlayer->EyePosition();
    end = start + forward * gm_tool_range.GetFloat();

    trace_t tr;
    UTIL_TraceLine(start, end, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

    return tr.endpos;
}

//-----------------------------------------------------------------------------
// Additional console command registrations for individual tools and context commands
//-----------------------------------------------------------------------------
static ConCommand gm_tool_weld_cmd("gm_tool_weld", CMD_gm_tool_weld, "Select weld tool");
static ConCommand gm_tool_axis_cmd("gm_tool_axis", CMD_gm_tool_axis, "Select axis tool");
static ConCommand gm_tool_ballsocket_cmd("gm_tool_ballsocket", CMD_gm_tool_ballsocket, "Select ballsocket tool");
static ConCommand gm_tool_rope_cmd("gm_tool_rope", CMD_gm_tool_rope, "Select rope tool");
static ConCommand gm_tool_spring_cmd("gm_tool_spring", CMD_gm_tool_spring, "Select spring tool");
static ConCommand gm_tool_hydraulic_cmd("gm_tool_hydraulic", CMD_gm_tool_hydraulic, "Select hydraulic tool");
static ConCommand gm_tool_motor_cmd("gm_tool_motor", CMD_gm_tool_motor, "Select motor tool");
static ConCommand gm_tool_pulley_cmd("gm_tool_pulley", CMD_gm_tool_pulley, "Select pulley tool");
static ConCommand gm_tool_keepupright_cmd("gm_tool_keepupright", CMD_gm_tool_keepupright, "Select keep upright tool");
static ConCommand gm_tool_nocollide_cmd("gm_tool_nocollide", CMD_gm_tool_nocollide, "Select no collide tool");
static ConCommand gm_tool_thruster_cmd("gm_tool_thruster", CMD_gm_tool_thruster, "Select thruster tool");
static ConCommand gm_tool_wheel_cmd("gm_tool_wheel", CMD_gm_tool_wheel, "Select wheel tool");
static ConCommand gm_tool_remover_cmd("gm_tool_remover", CMD_gm_tool_remover, "Select remover tool");
static ConCommand gm_tool_ignite_cmd("gm_tool_ignite", CMD_gm_tool_ignite, "Select ignite tool");
static ConCommand gm_tool_camera_cmd("gm_tool_camera", CMD_gm_tool_camera, "Select camera tool");

// Context command registrations
static ConCommand gm_context_npc_cmd("gm_context_npc", CMD_gm_context_npc, "Set NPC context");
static ConCommand gm_context_camera_cmd("gm_context_camera", CMD_gm_context_camera, "Set camera context");
static ConCommand gm_context_entity_cmd("gm_context_entity", CMD_gm_context_entity, "Set entity context");
static ConCommand gm_context_world_cmd("gm_context_world", CMD_gm_context_world, "Set world context");