#include "gmod_swep.h"
#include "cbase.h"
#include "player.h"
#include "basecombatweapon.h"
#include "filesystem.h"
#include "gmod_lua.h"
#include "gmod_undo.h"
#include "tier0/memdbgon.h"

// Define missing flag for LeakNet compatibility
#ifndef FCVAR_GAMEDLL
#define FCVAR_GAMEDLL 0
#endif

// Static member definitions
CUtlVector<SWEPData_t> CGModSWEPSystem::s_SWEPRegistry;
CUtlVector<CGModSWEPSystem::SWEPInstance_t> CGModSWEPSystem::s_SWEPInstances;
bool CGModSWEPSystem::s_bSystemInitialized = false;

// Global instance
CGModSWEPSystem g_GMod_SWEPSystem;

// ConVars for SWEP system configuration
ConVar gmod_swep_enabled("gmod_swep_enabled", "1", FCVAR_GAMEDLL, "Enable/disable SWEP system");
ConVar gmod_swep_debug("gmod_swep_debug", "0", FCVAR_GAMEDLL, "Debug SWEP system");
ConVar gmod_swep_autoload("gmod_swep_autoload", "1", FCVAR_GAMEDLL, "Automatically load SWEPs on startup");
ConVar gmod_swep_path("gmod_swep_path", "lua/weapons/", FCVAR_GAMEDLL, "Path to SWEP scripts");

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
// CGModSWEPSystem implementation
//-----------------------------------------------------------------------------
bool CGModSWEPSystem::Init()
{
    if (!gmod_swep_enabled.GetBool())
    {
        DevMsg("GMod SWEP System disabled by ConVar\n");
        return true;
    }

    s_SWEPRegistry.Purge();
    s_SWEPInstances.Purge();

    InitializeSWEPDefaults();

    if (gmod_swep_autoload.GetBool())
    {
        LoadAllSWEPs();
    }

    s_bSystemInitialized = true;

    DevMsg("GMod SWEP System initialized with %d SWEPs\n", s_SWEPRegistry.Count());
    return true;
}

void CGModSWEPSystem::Shutdown()
{
    s_SWEPRegistry.Purge();
    s_SWEPInstances.Purge();
    s_bSystemInitialized = false;

    DevMsg("GMod SWEP System shutdown\n");
}

void CGModSWEPSystem::LevelInitPostEntity()
{
    if (!s_bSystemInitialized)
        return;

    // Clean up old instances
    s_SWEPInstances.Purge();

    if (gmod_swep_autoload.GetBool())
    {
        ReloadAllSWEPs();
    }

    DevMsg("GMod SWEP System: Level initialized, SWEPs reloaded\n");
}

bool CGModSWEPSystem::RegisterSWEP(const char* pszClassName, const char* pszScriptPath)
{
    if (!pszClassName || !pszScriptPath || !s_bSystemInitialized)
        return false;

    // Check if already registered
    if (FindSWEP(pszClassName))
    {
        DevMsg("SWEP %s already registered\n", pszClassName);
        return true;
    }

    // Validate script exists
    if (!ValidateSWEPScript(pszScriptPath))
    {
        Warning("SWEP script not found: %s\n", pszScriptPath);
        return false;
    }

    SWEPData_t swepData;
    swepData.className = pszClassName;
    swepData.scriptPath = pszScriptPath;
    swepData.isRegistered = true;

    s_SWEPRegistry.AddToTail(swepData);

    if (gmod_swep_debug.GetBool())
    {
        DevMsg("Registered SWEP: %s -> %s\n", pszClassName, pszScriptPath);
    }

    return true;
}

bool CGModSWEPSystem::LoadSWEP(const char* pszClassName)
{
    if (!pszClassName || !s_bSystemInitialized)
        return false;

    SWEPData_t* pSWEPData = FindSWEP(pszClassName);
    if (!pSWEPData)
    {
        Warning("SWEP not found: %s\n", pszClassName);
        return false;
    }

    if (pSWEPData->isLoaded)
    {
        DevMsg("SWEP %s already loaded\n", pszClassName);
        return true;
    }

    bool success = LoadSWEPScript(pSWEPData);
    if (success)
    {
        pSWEPData->isLoaded = true;
        if (gmod_swep_debug.GetBool())
        {
            DevMsg("Loaded SWEP: %s\n", pszClassName);
        }
    }
    else
    {
        Warning("Failed to load SWEP script: %s\n", pSWEPData->scriptPath.Get());
    }

    return success;
}

bool CGModSWEPSystem::UnloadSWEP(const char* pszClassName)
{
    if (!pszClassName || !s_bSystemInitialized)
        return false;

    SWEPData_t* pSWEPData = FindSWEP(pszClassName);
    if (!pSWEPData)
        return false;

    pSWEPData->isLoaded = false;

    // Remove all instances of this SWEP
    for (int i = s_SWEPInstances.Count() - 1; i >= 0; i--)
    {
        if (Q_stricmp(s_SWEPInstances[i].className.Get(), pszClassName) == 0)
        {
            s_SWEPInstances.Remove(i);
        }
    }

    if (gmod_swep_debug.GetBool())
    {
        DevMsg("Unloaded SWEP: %s\n", pszClassName);
    }

    return true;
}

SWEPData_t* CGModSWEPSystem::GetSWEPData(const char* pszClassName)
{
    if (!pszClassName || !s_bSystemInitialized)
        return NULL;

    return FindSWEP(pszClassName);
}

bool CGModSWEPSystem::IsSWEPRegistered(const char* pszClassName)
{
    return FindSWEP(pszClassName) != NULL;
}

bool CGModSWEPSystem::InitializeSWEPInstance(CBaseCombatWeapon* pWeapon, const char* pszClassName)
{
    if (!pWeapon || !pszClassName || !s_bSystemInitialized)
        return false;

    SWEPData_t* pSWEPData = FindSWEP(pszClassName);
    if (!pSWEPData || !pSWEPData->isLoaded)
    {
        Warning("SWEP not loaded: %s\n", pszClassName);
        return false;
    }

    // Check if instance already exists
    SWEPInstance_t* pInstance = FindSWEPInstance(pWeapon);
    if (pInstance)
    {
        DevMsg("SWEP instance already exists for weapon %d\n", pWeapon->entindex());
        return true;
    }

    // Create new instance
    SWEPInstance_t instance;
    instance.pWeapon = pWeapon;
    instance.className = pszClassName;
    instance.isInitialized = false;
    s_SWEPInstances.AddToTail(instance);

    // Set Lua context and call initialization
    SetSWEPLuaContext(pWeapon);
    OnSWEPInit(pWeapon);

    // Mark as initialized
    pInstance = FindSWEPInstance(pWeapon);
    if (pInstance)
    {
        pInstance->isInitialized = true;
    }

    if (gmod_swep_debug.GetBool())
    {
        DevMsg("Initialized SWEP instance: %s (weapon %d)\n", pszClassName, pWeapon->entindex());
    }

    return true;
}

void CGModSWEPSystem::CallSWEPFunction(CBaseCombatWeapon* pWeapon, const char* pszFunction)
{
    if (!pWeapon || !pszFunction || !s_bSystemInitialized)
        return;

    SWEPInstance_t* pInstance = FindSWEPInstance(pWeapon);
    if (!pInstance || !pInstance->isInitialized)
        return;

    SetSWEPLuaContext(pWeapon);
    CGModLuaSystem::CallGamemodeFunction(pszFunction);
}

void CGModSWEPSystem::CallSWEPFunctionWithArgs(CBaseCombatWeapon* pWeapon, const char* pszFunction, const char* pszArgs)
{
    if (!pWeapon || !pszFunction || !s_bSystemInitialized)
        return;

    SWEPInstance_t* pInstance = FindSWEPInstance(pWeapon);
    if (!pInstance || !pInstance->isInitialized)
        return;

    SetSWEPLuaContext(pWeapon);

    char luaCode[512];
    if (pszArgs && pszArgs[0])
    {
        Q_snprintf(luaCode, sizeof(luaCode), "%s(%s)", pszFunction, pszArgs);
    }
    else
    {
        Q_snprintf(luaCode, sizeof(luaCode), "%s()", pszFunction);
    }

    CGModLuaSystem::ExecuteString(luaCode);
}

void CGModSWEPSystem::SetSWEPLuaContext(CBaseCombatWeapon* pWeapon)
{
    if (!pWeapon)
        return;

    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L)
        return;

    CBasePlayer* pOwner = dynamic_cast<CBasePlayer*>(pWeapon->GetOwner());
    int ownerID = pOwner ? pOwner->entindex() : 0;

    // Set global variables that SWEPs expect
    char luaCode[256];
    Q_snprintf(luaCode, sizeof(luaCode),
        "MyIndex = %d; Owner = %d; CurrentTime = %f;",
        pWeapon->entindex(), ownerID, gpGlobals->curtime);

    CGModLuaSystem::ExecuteString(luaCode);
}

// SWEP event implementations
void CGModSWEPSystem::OnSWEPInit(CBaseCombatWeapon* pWeapon)
{
    CallSWEPFunction(pWeapon, "onInit");
}

void CGModSWEPSystem::OnSWEPThink(CBaseCombatWeapon* pWeapon)
{
    if (!pWeapon)
        return;

    SWEPInstance_t* pInstance = FindSWEPInstance(pWeapon);
    if (!pInstance || !pInstance->isInitialized)
        return;

    // Throttle think calls to avoid performance issues
    if (gpGlobals->curtime - pInstance->lastThinkTime < 0.1f)
        return;

    pInstance->lastThinkTime = gpGlobals->curtime;
    CallSWEPFunction(pWeapon, "onThink");
}

void CGModSWEPSystem::OnSWEPPrimaryAttack(CBaseCombatWeapon* pWeapon)
{
    CallSWEPFunction(pWeapon, "onPrimaryAttack");
}

void CGModSWEPSystem::OnSWEPSecondaryAttack(CBaseCombatWeapon* pWeapon)
{
    CallSWEPFunction(pWeapon, "onSecondaryAttack");
}

void CGModSWEPSystem::OnSWEPReload(CBaseCombatWeapon* pWeapon)
{
    CallSWEPFunction(pWeapon, "onReload");
}

void CGModSWEPSystem::OnSWEPDeploy(CBaseCombatWeapon* pWeapon)
{
    CallSWEPFunction(pWeapon, "Deploy");
}

void CGModSWEPSystem::OnSWEPHolster(CBaseCombatWeapon* pWeapon)
{
    CallSWEPFunction(pWeapon, "Holster");
}

void CGModSWEPSystem::OnSWEPPickup(CBaseCombatWeapon* pWeapon, CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    char args[64];
    Q_snprintf(args, sizeof(args), "%d", pPlayer->entindex());
    CallSWEPFunctionWithArgs(pWeapon, "onPickup", args);
}

void CGModSWEPSystem::OnSWEPDrop(CBaseCombatWeapon* pWeapon, CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    char args[64];
    Q_snprintf(args, sizeof(args), "%d", pPlayer->entindex());
    CallSWEPFunctionWithArgs(pWeapon, "onDrop", args);
}

void CGModSWEPSystem::OnSWEPRemove(CBaseCombatWeapon* pWeapon)
{
    CallSWEPFunction(pWeapon, "onRemove");

    // Remove instance
    for (int i = s_SWEPInstances.Count() - 1; i >= 0; i--)
    {
        if (s_SWEPInstances[i].pWeapon == pWeapon)
        {
            s_SWEPInstances.Remove(i);
            break;
        }
    }
}

// SWEP property query functions
int CGModSWEPSystem::GetSWEPIntProperty(CBaseCombatWeapon* pWeapon, const char* pszProperty)
{
    if (!pWeapon || !pszProperty)
        return 0;

    SetSWEPLuaContext(pWeapon);

    char luaCode[128];
    Q_snprintf(luaCode, sizeof(luaCode), "return %s()", pszProperty);

    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L)
        return 0;

    if (luaL_dostring(L, luaCode) == 0)
    {
        if (lua_isnumber(L, -1))
        {
            int result = (int)lua_tonumber(L, -1);
            lua_pop(L, 1);
            return result;
        }
        lua_pop(L, 1);
    }

    return 0;
}

float CGModSWEPSystem::GetSWEPFloatProperty(CBaseCombatWeapon* pWeapon, const char* pszProperty)
{
    if (!pWeapon || !pszProperty)
        return 0.0f;

    SetSWEPLuaContext(pWeapon);

    char luaCode[128];
    Q_snprintf(luaCode, sizeof(luaCode), "return %s()", pszProperty);

    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L)
        return 0.0f;

    if (luaL_dostring(L, luaCode) == 0)
    {
        if (lua_isnumber(L, -1))
        {
            float result = (float)lua_tonumber(L, -1);
            lua_pop(L, 1);
            return result;
        }
        lua_pop(L, 1);
    }

    return 0.0f;
}

bool CGModSWEPSystem::GetSWEPBoolProperty(CBaseCombatWeapon* pWeapon, const char* pszProperty)
{
    if (!pWeapon || !pszProperty)
        return false;

    SetSWEPLuaContext(pWeapon);

    char luaCode[128];
    Q_snprintf(luaCode, sizeof(luaCode), "return %s()", pszProperty);

    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L)
        return false;

    if (luaL_dostring(L, luaCode) == 0)
    {
        if (lua_isboolean(L, -1))
        {
            bool result = lua_toboolean(L, -1) != 0;
            lua_pop(L, 1);
            return result;
        }
        lua_pop(L, 1);
    }

    return false;
}

CUtlString CGModSWEPSystem::GetSWEPStringProperty(CBaseCombatWeapon* pWeapon, const char* pszProperty)
{
    if (!pWeapon || !pszProperty)
        return CUtlString("");

    SetSWEPLuaContext(pWeapon);

    char luaCode[128];
    Q_snprintf(luaCode, sizeof(luaCode), "return %s()", pszProperty);

    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L)
        return CUtlString("");

    if (luaL_dostring(L, luaCode) == 0)
    {
        if (lua_isstring(L, -1))
        {
            const char* result = lua_tostring(L, -1);
            CUtlString str(result ? result : "");
            lua_pop(L, 1);
            return str;
        }
        lua_pop(L, 1);
    }

    return CUtlString("");
}

Vector CGModSWEPSystem::GetSWEPVectorProperty(CBaseCombatWeapon* pWeapon, const char* pszProperty)
{
    if (!pWeapon || !pszProperty)
        return vec3_origin;

    SetSWEPLuaContext(pWeapon);

    char luaCode[128];
    Q_snprintf(luaCode, sizeof(luaCode), "result = %s(); return result.x, result.y, result.z", pszProperty);

    lua_State* L = CGModLuaSystem::GetLuaState();
    if (!L)
        return vec3_origin;

    if (luaL_dostring(L, luaCode) == 0)
    {
        if (lua_gettop(L) >= 3 && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1))
        {
            Vector result;
            result.x = (float)lua_tonumber(L, -3);
            result.y = (float)lua_tonumber(L, -2);
            result.z = (float)lua_tonumber(L, -1);
            lua_pop(L, 3);
            return result;
        }
        lua_settop(L, 0); // Clear stack
    }

    return vec3_origin;
}

void CGModSWEPSystem::LoadAllSWEPs()
{
    LoadBaseSWEP();
    LoadBuildSWEPs();

    DevMsg("Loaded all SWEPs\n");
}

void CGModSWEPSystem::LoadBaseSWEP()
{
    // Register and load base SWEP
    RegisterSWEP("weapon_scripted", "lua/weapons/base.lua");
    LoadSWEP("weapon_scripted");
}

void CGModSWEPSystem::LoadBuildSWEPs()
{
    // Load build weapons discovered from lua/weapons/build/ directory analysis
    RegisterSWEP("weapon_propmaker", "lua/weapons/build/weapon_propmaker.lua");
    RegisterSWEP("weapon_cratemaker", "lua/weapons/build/weapon_cratemaker.lua");
    RegisterSWEP("weapon_freeze", "lua/weapons/build/weapon_freeze.lua");
    RegisterSWEP("weapon_remover", "lua/weapons/build/weapon_remover.lua");
    RegisterSWEP("weapon_spawn", "lua/weapons/build/weapon_spawn.lua");

    LoadSWEP("weapon_propmaker");
    LoadSWEP("weapon_cratemaker");
    LoadSWEP("weapon_freeze");
    LoadSWEP("weapon_remover");
    LoadSWEP("weapon_spawn");
}

void CGModSWEPSystem::ReloadAllSWEPs()
{
    // Unload all SWEPs
    for (int i = 0; i < s_SWEPRegistry.Count(); i++)
    {
        s_SWEPRegistry[i].isLoaded = false;
    }

    s_SWEPInstances.Purge();

    // Reload all SWEPs
    LoadAllSWEPs();

    DevMsg("Reloaded all SWEPs\n");
}

const char* CGModSWEPSystem::GetSWEPTypeName(SWEPType_t swepType)
{
    switch (swepType)
    {
        case SWEP_BASE: return "base";
        case SWEP_PROPMAKER: return "propmaker";
        case SWEP_CRATEMAKER: return "cratemaker";
        case SWEP_FREEZE: return "freeze";
        case SWEP_REMOVER: return "remover";
        case SWEP_SPAWN: return "spawn";
        default: return "none";
    }
}

SWEPType_t CGModSWEPSystem::GetSWEPTypeFromName(const char* pszName)
{
    if (!pszName)
        return SWEP_NONE;

    if (Q_stricmp(pszName, "base") == 0) return SWEP_BASE;
    if (Q_stricmp(pszName, "propmaker") == 0) return SWEP_PROPMAKER;
    if (Q_stricmp(pszName, "cratemaker") == 0) return SWEP_CRATEMAKER;
    if (Q_stricmp(pszName, "freeze") == 0) return SWEP_FREEZE;
    if (Q_stricmp(pszName, "remover") == 0) return SWEP_REMOVER;
    if (Q_stricmp(pszName, "spawn") == 0) return SWEP_SPAWN;

    return SWEP_NONE;
}

// Helper function implementations
SWEPData_t* CGModSWEPSystem::FindSWEP(const char* pszClassName)
{
    if (!pszClassName)
        return NULL;

    for (int i = 0; i < s_SWEPRegistry.Count(); i++)
    {
        if (Q_stricmp(s_SWEPRegistry[i].className.Get(), pszClassName) == 0)
        {
            return &s_SWEPRegistry[i];
        }
    }

    return NULL;
}

CGModSWEPSystem::SWEPInstance_t* CGModSWEPSystem::FindSWEPInstance(CBaseCombatWeapon* pWeapon)
{
    if (!pWeapon)
        return NULL;

    for (int i = 0; i < s_SWEPInstances.Count(); i++)
    {
        if (s_SWEPInstances[i].pWeapon == pWeapon)
        {
            return &s_SWEPInstances[i];
        }
    }

    return NULL;
}

bool CGModSWEPSystem::LoadSWEPScript(SWEPData_t* pSWEPData)
{
    if (!pSWEPData)
        return false;

    LuaFunctionResult_t result = CGModLuaSystem::LoadScript(pSWEPData->scriptPath.Get(), LUA_SCRIPT_SWEP);
    return result == LUA_RESULT_SUCCESS;
}

void CGModSWEPSystem::InitializeSWEPDefaults()
{
    // Initialize default SWEP types with their properties
    DevMsg("SWEP defaults initialized\n");
}

bool CGModSWEPSystem::ValidateSWEPScript(const char* pszScriptPath)
{
    if (!pszScriptPath)
        return false;

    return filesystem->FileExists(pszScriptPath, "GAME");
}

//-----------------------------------------------------------------------------
// Console command implementations discovered from IDA analysis
//-----------------------------------------------------------------------------
void CMD_gmod_give_swep(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() < 2)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Usage: gmod_give_swep <swep_name>");
        return;
    }

    const char* swepName = engine->Cmd_Argv(1);
    if (!CGModSWEPSystem::IsSWEPRegistered(swepName))
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "SWEP not found: %s", swepName);
        return;
    }

    // Give the weapon to the player
    CBaseCombatWeapon* pWeapon = pPlayer->GiveNamedItem(swepName);
    if (pWeapon)
    {
        CGModSWEPSystem::InitializeSWEPInstance(pWeapon, swepName);
        ClientPrint(pPlayer, HUD_PRINTTALK, "Given SWEP: %s", swepName);
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to give SWEP: %s", swepName);
    }
}

void CMD_gmod_reload_sweps(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModSWEPSystem::ReloadAllSWEPs();
    ClientPrint(pPlayer, HUD_PRINTTALK, "All SWEPs reloaded");
}

void CMD_gmod_list_sweps(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Available SWEPs:");

    int count = 0;
    for (int i = 0; i < CGModSWEPSystem::s_SWEPRegistry.Count(); i++)
    {
        const SWEPData_t& swep = CGModSWEPSystem::s_SWEPRegistry[i];
        if (swep.isRegistered)
        {
            ClientPrint(pPlayer, HUD_PRINTTALK, "  %s (%s)", swep.className.Get(), swep.isLoaded ? "loaded" : "not loaded");
            count++;
        }
    }

    ClientPrint(pPlayer, HUD_PRINTTALK, "Total: %d SWEPs", count);
}

// Individual SWEP commands discovered from IDA analysis
void CMD_gmod_give_propmaker(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer) return;

    CBaseCombatWeapon* pWeapon = pPlayer->GiveNamedItem("weapon_propmaker");
    if (pWeapon)
    {
        CGModSWEPSystem::InitializeSWEPInstance(pWeapon, "weapon_propmaker");
        ClientPrint(pPlayer, HUD_PRINTTALK, "Given Prop Maker");
    }
}

void CMD_gmod_give_cratemaker(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer) return;

    CBaseCombatWeapon* pWeapon = pPlayer->GiveNamedItem("weapon_cratemaker");
    if (pWeapon)
    {
        CGModSWEPSystem::InitializeSWEPInstance(pWeapon, "weapon_cratemaker");
        ClientPrint(pPlayer, HUD_PRINTTALK, "Given Crate Maker");
    }
}

void CMD_gmod_give_freeze(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer) return;

    CBaseCombatWeapon* pWeapon = pPlayer->GiveNamedItem("weapon_freeze");
    if (pWeapon)
    {
        CGModSWEPSystem::InitializeSWEPInstance(pWeapon, "weapon_freeze");
        ClientPrint(pPlayer, HUD_PRINTTALK, "Given Freeze Ray");
    }
}

void CMD_gmod_give_remover(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer) return;

    CBaseCombatWeapon* pWeapon = pPlayer->GiveNamedItem("weapon_remover");
    if (pWeapon)
    {
        CGModSWEPSystem::InitializeSWEPInstance(pWeapon, "weapon_remover");
        ClientPrint(pPlayer, HUD_PRINTTALK, "Given Remover");
    }
}

void CMD_gmod_give_spawn(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer) return;

    CBaseCombatWeapon* pWeapon = pPlayer->GiveNamedItem("weapon_spawn");
    if (pWeapon)
    {
        CGModSWEPSystem::InitializeSWEPInstance(pWeapon, "weapon_spawn");
        ClientPrint(pPlayer, HUD_PRINTTALK, "Given Spawn Gun");
    }
}

//-----------------------------------------------------------------------------
// Console command registration
//-----------------------------------------------------------------------------
static ConCommand gmod_give_swep_cmd("gmod_give_swep", CMD_gmod_give_swep, "Give a SWEP to the player");
static ConCommand gmod_reload_sweps_cmd("gmod_reload_sweps", CMD_gmod_reload_sweps, "Reload all SWEPs");
static ConCommand gmod_list_sweps_cmd("gmod_list_sweps", CMD_gmod_list_sweps, "List all available SWEPs");

// Individual SWEP command registration
static ConCommand gmod_give_propmaker_cmd("gmod_give_propmaker", CMD_gmod_give_propmaker, "Give prop maker SWEP");
static ConCommand gmod_give_cratemaker_cmd("gmod_give_cratemaker", CMD_gmod_give_cratemaker, "Give crate maker SWEP");
static ConCommand gmod_give_freeze_cmd("gmod_give_freeze", CMD_gmod_give_freeze, "Give freeze ray SWEP");
static ConCommand gmod_give_remover_cmd("gmod_give_remover", CMD_gmod_give_remover, "Give remover SWEP");
static ConCommand gmod_give_spawn_cmd("gmod_give_spawn", CMD_gmod_give_spawn, "Give spawn gun SWEP");