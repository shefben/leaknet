#ifndef GMOD_SWEP_H
#define GMOD_SWEP_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "utlvector.h"
#include "igamesystem.h"
#include "gmod_lua.h"

// Forward declarations
class CBaseEntity;
class CBasePlayer;
class CBaseCombatWeapon;

// SWEP types discovered in IDA analysis and from original GMod directory
enum SWEPType_t
{
    SWEP_NONE = 0,
    SWEP_BASE = 1,
    SWEP_PROPMAKER = 2,
    SWEP_CRATEMAKER = 3,
    SWEP_FREEZE = 4,
    SWEP_REMOVER = 5,
    SWEP_SPAWN = 6,
    SWEP_MAX
};

// SWEP script override modes - discovered from base.lua analysis
enum SWEPScriptOverride_t
{
    SWEP_OVERRIDE_NONE = 0,     // Don't override, shoot bullets, make sound and flash
    SWEP_OVERRIDE_NOSHOTS = 1,  // Don't shoot bullets but do make flash/sounds
    SWEP_OVERRIDE_ANIMONLY = 2, // Only play animations
    SWEP_OVERRIDE_NOTHING = 3   // Don't do anything
};

// SWEP data structure - mirrors the Lua base.lua structure
struct SWEPData_t
{
    SWEPType_t swepType;
    char className[256];
    char printName[256];
    char scriptPath[256];
    char viewModel[256];
    char worldModel[256];
    char hudMaterial[256];
    char deathIcon[256];
    char animPrefix[64];
    char primaryAmmoType[64];
    char secondaryAmmoType[64];

    // Primary attack properties
    int damage;
    float primaryShotDelay;
    bool primaryIsAutomatic;
    Vector bulletSpread;
    Vector viewKick;
    Vector viewKickRandom;
    int numShotsPrimary;
    int maxClipPrimary;
    int defClipPrimary;
    int tracerFreqPrimary;
    SWEPScriptOverride_t primaryScriptOverride;

    // Secondary attack properties
    int damageSecondary;
    float secondaryShotDelay;
    bool secondaryIsAutomatic;
    Vector bulletSpreadSecondary;
    Vector viewKickSecondary;
    Vector viewKickRandomSecondary;
    int numShotsSecondary;
    int maxClipSecondary;
    int defClipSecondary;
    int tracerFreqSecondary;
    SWEPScriptOverride_t secondaryScriptOverride;

    // Weapon configuration
    bool weaponSwapHands;
    int weaponFOV;
    int weaponSlot;
    int weaponSlotPos;
    bool firesUnderwater;
    bool reloadsSingly;
    bool isLoaded;
    bool isRegistered;

    SWEPData_t()
    {
        swepType = SWEP_NONE;
        strcpy(className, "weapon_scripted");
        strcpy(printName, "Scripted Weapon");
        strcpy(scriptPath, "");
        strcpy(viewModel, "models/weapons/v_smg_ump45.mdl");
        strcpy(worldModel, "models/weapons/w_smg_ump45.mdl");
        strcpy(hudMaterial, "gmod/SWEP/default");
        strcpy(deathIcon, "swep_default");
        strcpy(animPrefix, "shotgun");
        strcpy(primaryAmmoType, "pistol");
        strcpy(secondaryAmmoType, "pistol");

        damage = 10;
        primaryShotDelay = 0.2f;
        primaryIsAutomatic = true;
        bulletSpread = Vector(0.01f, 0.01f, 0.01f);
        viewKick = Vector(0.5f, 0.0f, 0.0f);
        viewKickRandom = Vector(0.5f, 0.5f, 0.2f);
        numShotsPrimary = 1;
        maxClipPrimary = 25;
        defClipPrimary = 25;
        tracerFreqPrimary = 2;
        primaryScriptOverride = SWEP_OVERRIDE_NONE;

        damageSecondary = 10;
        secondaryShotDelay = 0.2f;
        secondaryIsAutomatic = false;
        bulletSpreadSecondary = Vector(0.001f, 0.001f, 0.001f);
        viewKickSecondary = Vector(0.5f, 0.0f, 0.0f);
        viewKickRandomSecondary = Vector(0.5f, 0.5f, 0.2f);
        numShotsSecondary = 1;
        maxClipSecondary = -1;
        defClipSecondary = 0;
        tracerFreqSecondary = 2;
        secondaryScriptOverride = SWEP_OVERRIDE_NONE;

        weaponSwapHands = false;
        weaponFOV = 70;
        weaponSlot = 5;
        weaponSlotPos = 2;
        firesUnderwater = true;
        reloadsSingly = false;
        isLoaded = false;
        isRegistered = false;
    }
};

//-----------------------------------------------------------------------------
// GMod SWEP System - Implements complete SWEP functionality from GMod 9.0.4b
// Discovered through IDA analysis and lua/weapons/ directory examination
//-----------------------------------------------------------------------------
class CGModSWEPSystem : public CAutoGameSystem
{
public:
    CGModSWEPSystem() : CAutoGameSystem() {}

    // CAutoGameSystem overrides
    virtual bool Init();
    virtual void Shutdown();
    virtual void LevelInitPostEntity();

    // SWEP management functions
    static bool RegisterSWEP(const char* pszClassName, const char* pszScriptPath);
    static bool LoadSWEP(const char* pszClassName);
    static bool UnloadSWEP(const char* pszClassName);
    static SWEPData_t* GetSWEPData(const char* pszClassName);
    static bool IsSWEPRegistered(const char* pszClassName);

    // SWEP instance functions
    static bool InitializeSWEPInstance(CBaseCombatWeapon* pWeapon, const char* pszClassName);
    static void CallSWEPFunction(CBaseCombatWeapon* pWeapon, const char* pszFunction);
    static void CallSWEPFunctionWithArgs(CBaseCombatWeapon* pWeapon, const char* pszFunction, const char* pszArgs);
    static void SetSWEPLuaContext(CBaseCombatWeapon* pWeapon);

    // SWEP events - called from weapon entity
    static void OnSWEPInit(CBaseCombatWeapon* pWeapon);
    static void OnSWEPThink(CBaseCombatWeapon* pWeapon);
    static void OnSWEPPrimaryAttack(CBaseCombatWeapon* pWeapon);
    static void OnSWEPSecondaryAttack(CBaseCombatWeapon* pWeapon);
    static void OnSWEPReload(CBaseCombatWeapon* pWeapon);
    static void OnSWEPDeploy(CBaseCombatWeapon* pWeapon);
    static void OnSWEPHolster(CBaseCombatWeapon* pWeapon);
    static void OnSWEPPickup(CBaseCombatWeapon* pWeapon, CBasePlayer* pPlayer);
    static void OnSWEPDrop(CBaseCombatWeapon* pWeapon, CBasePlayer* pPlayer);
    static void OnSWEPRemove(CBaseCombatWeapon* pWeapon);

    // SWEP property queries
    static int GetSWEPIntProperty(CBaseCombatWeapon* pWeapon, const char* pszProperty);
    static float GetSWEPFloatProperty(CBaseCombatWeapon* pWeapon, const char* pszProperty);
    static bool GetSWEPBoolProperty(CBaseCombatWeapon* pWeapon, const char* pszProperty);
    static const char* GetSWEPStringProperty(CBaseCombatWeapon* pWeapon, const char* pszProperty);
    static Vector GetSWEPVectorProperty(CBaseCombatWeapon* pWeapon, const char* pszProperty);

    // SWEP registration and loading
    static void LoadAllSWEPs();
    static void LoadBaseSWEP();
    static void LoadBuildSWEPs();
    static void ReloadAllSWEPs();

    // Utility functions
    static const char* GetSWEPTypeName(SWEPType_t swepType);
    static SWEPType_t GetSWEPTypeFromName(const char* pszName);

    // Public registry access for console commands
    static CUtlVector<SWEPData_t> s_SWEPRegistry;

private:
    struct SWEPInstance_t
    {
        CBaseCombatWeapon* pWeapon;
        char className[256];
        bool isInitialized;
        float lastThinkTime;

        SWEPInstance_t()
        {
            pWeapon = NULL;
            className[0] = '\0';
            isInitialized = false;
            lastThinkTime = 0.0f;
        }
    };

    static CUtlVector<SWEPInstance_t> s_SWEPInstances;
    static bool s_bSystemInitialized;

    // Helper functions
    static SWEPData_t* FindSWEP(const char* pszClassName);
    static SWEPInstance_t* FindSWEPInstance(CBaseCombatWeapon* pWeapon);
    static bool LoadSWEPScript(SWEPData_t* pSWEPData);
    static void InitializeSWEPDefaults();
    static bool ValidateSWEPScript(const char* pszScriptPath);
};

// Global instance
extern CGModSWEPSystem g_GMod_SWEPSystem;

// Console command handlers - discovered from IDA string analysis
void CMD_gmod_give_swep(void);
void CMD_gmod_reload_sweps(void);
void CMD_gmod_list_sweps(void);

// Individual SWEP commands
void CMD_gmod_give_propmaker(void);
void CMD_gmod_give_cratemaker(void);
void CMD_gmod_give_freeze(void);
void CMD_gmod_give_remover(void);
void CMD_gmod_give_spawn(void);

#endif // GMOD_SWEP_H