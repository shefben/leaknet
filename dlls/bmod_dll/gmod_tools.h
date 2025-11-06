#ifndef GMOD_TOOLS_H
#define GMOD_TOOLS_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "utlvector.h"
#include "igamesystem.h"

// Forward declarations
class CBaseEntity;
class CBasePlayer;

// Tool types discovered in IDA analysis
enum GMToolType_t
{
    TOOL_NONE = 0,
    TOOL_WELD = 1,
    TOOL_AXIS = 2,
    TOOL_BALLSOCKET = 3,
    TOOL_ROPE = 4,
    TOOL_SPRING = 5,
    TOOL_HYDRAULIC = 6,
    TOOL_MOTOR = 7,
    TOOL_PULLEY = 8,
    TOOL_KEEPUPRIGHT = 9,
    TOOL_NOCOLLIDE = 10,
    TOOL_THRUSTER = 11,
    TOOL_WHEEL = 12,
    TOOL_REMOVER = 13,
    TOOL_IGNITE = 14,
    TOOL_CAMERA = 15,
    TOOL_MAX
};

// Context types discovered in IDA analysis
enum GMContextType_t
{
    CONTEXT_NONE = 0,
    CONTEXT_NPC,
    CONTEXT_CAMERA,
    CONTEXT_ENTITY,
    CONTEXT_WORLD,
    CONTEXT_MAX
};

// Weapon selection modes discovered in IDA analysis
enum GMWeaponSelMode_t
{
    WEAPONSEL_NORMAL = 0,
    WEAPONSEL_FASTSWITCH = 1,
    WEAPONSEL_BUCKETS = 2,
    WEAPONSEL_MAX
};

// Tool data structure
struct GMToolData_t
{
    GMToolType_t toolType;
    char toolName[256];
    char displayName[256];
    char description[256];
    char model[256];
    char material[256];
    float delay;
    bool enabled;
    int maxUses;
    float range;

    GMToolData_t()
    {
        toolType = TOOL_NONE;
        toolName[0] = '\0';
        displayName[0] = '\0';
        description[0] = '\0';
        model[0] = '\0';
        material[0] = '\0';
        delay = 0.5f;
        enabled = true;
        maxUses = -1;
        range = 4096.0f;
    }
};

//-----------------------------------------------------------------------------
// GMod Tools System - Implements complete tool functionality from GMod 9.0.4b
// Discovered through IDA analysis of gm_toolweapon, gm_toolmode, gm_context handlers
//-----------------------------------------------------------------------------
class CGModToolsSystem : public CAutoGameSystem
{
public:
    CGModToolsSystem() : CAutoGameSystem() {}

    // CAutoGameSystem overrides
    virtual bool Init();
    virtual void Shutdown();
    virtual void LevelInitPostEntity();

    // Tool management functions
    static void SetPlayerTool(CBasePlayer* pPlayer, GMToolType_t toolType);
    static GMToolType_t GetPlayerTool(CBasePlayer* pPlayer);
    static void SetPlayerToolMode(CBasePlayer* pPlayer, int mode);
    static int GetPlayerToolMode(CBasePlayer* pPlayer);

    // Context system functions
    static void SetPlayerContext(CBasePlayer* pPlayer, GMContextType_t contextType);
    static GMContextType_t GetPlayerContext(CBasePlayer* pPlayer);
    static void ShowContextMenu(CBasePlayer* pPlayer, CBaseEntity* pTarget);
    static void HideContextMenu(CBasePlayer* pPlayer);

    // Weapon selection system
    static void SetWeaponSelectionMode(CBasePlayer* pPlayer, GMWeaponSelMode_t mode);
    static GMWeaponSelMode_t GetWeaponSelectionMode(CBasePlayer* pPlayer);

    // Tool execution functions
    static bool ExecuteTool(CBasePlayer* pPlayer, CBaseEntity* pTarget, const Vector& tracePos, const Vector& traceNormal);
    static bool CanUseTool(CBasePlayer* pPlayer, GMToolType_t toolType);
    static void ResetToolCooldown(CBasePlayer* pPlayer);

    // Tool registration and management
    static void RegisterTool(GMToolType_t toolType, const GMToolData_t& toolData);
    static GMToolData_t* GetToolData(GMToolType_t toolType);
    static void LoadToolConfiguration();
    static void SaveToolConfiguration();

    // Utility functions
    static const char* GetToolName(GMToolType_t toolType);
    static const char* GetContextName(GMContextType_t contextType);
    static CBaseEntity* GetPlayerTargetEntity(CBasePlayer* pPlayer);
    static Vector GetPlayerTracePosition(CBasePlayer* pPlayer);

private:
    struct PlayerToolData_t
    {
        GMToolType_t currentTool;
        int toolMode;
        GMContextType_t contextType;
        GMWeaponSelMode_t weaponSelMode;
        float lastToolUse;
        int toolUseCount;
        bool contextMenuOpen;

        PlayerToolData_t()
        {
            currentTool = TOOL_WELD;
            toolMode = 0;
            contextType = CONTEXT_NONE;
            weaponSelMode = WEAPONSEL_NORMAL;
            lastToolUse = 0.0f;
            toolUseCount = 0;
            contextMenuOpen = false;
        }
    };

    static CUtlVector<PlayerToolData_t> s_PlayerToolData;
    static CUtlVector<GMToolData_t> s_ToolRegistry;
    static bool s_bSystemInitialized;

    // Helper functions
    static PlayerToolData_t* GetPlayerToolData(CBasePlayer* pPlayer);
    static void InitializeToolRegistry();
    static bool ValidateToolUse(CBasePlayer* pPlayer, GMToolType_t toolType);
};

// Global instance
extern CGModToolsSystem g_GMod_ToolsSystem;

// Console command handlers - discovered from IDA string analysis
void CMD_gm_toolweapon(void);
void CMD_gm_toolmode(void);
void CMD_gm_context(void);
void CMD_gm_wepselmode(void);

// Tool-specific commands
void CMD_gm_tool_weld(void);
void CMD_gm_tool_axis(void);
void CMD_gm_tool_ballsocket(void);
void CMD_gm_tool_rope(void);
void CMD_gm_tool_spring(void);
void CMD_gm_tool_hydraulic(void);
void CMD_gm_tool_motor(void);
void CMD_gm_tool_pulley(void);
void CMD_gm_tool_keepupright(void);
void CMD_gm_tool_nocollide(void);
void CMD_gm_tool_thruster(void);
void CMD_gm_tool_wheel(void);
void CMD_gm_tool_remover(void);
void CMD_gm_tool_ignite(void);
void CMD_gm_tool_camera(void);

// Context menu commands
void CMD_gm_context_npc(void);
void CMD_gm_context_camera(void);
void CMD_gm_context_entity(void);
void CMD_gm_context_world(void);

// Button commands (discovered in IDA: gm_button%i pattern)
void CMD_gm_button0(void);
void CMD_gm_button1(void);
void CMD_gm_button2(void);
void CMD_gm_button3(void);
void CMD_gm_button4(void);
void CMD_gm_button5(void);
void CMD_gm_button6(void);
void CMD_gm_button7(void);
void CMD_gm_button8(void);
void CMD_gm_button9(void);

#endif // GMOD_TOOLS_H