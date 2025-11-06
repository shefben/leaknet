#ifndef GMOD_MOD_H
#define GMOD_MOD_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "utlvector.h"
#include "igamesystem.h"

// Forward declarations
class CBaseEntity;
class CBasePlayer;

// Mod state discovered from IDA analysis
enum ModState_t
{
    MOD_STATE_UNKNOWN = 0,
    MOD_STATE_ENABLED = 1,
    MOD_STATE_DISABLED = 2,
    MOD_STATE_LOADING = 3,
    MOD_STATE_ERROR = 4
};

// Mod data structure based on IDA string analysis
struct ModData_t
{
    char modName[256];
    char modPath[256];
    char modInfoPath[256];
    char title[256];
    char author[256];
    char description[512];
    char version[64];
    char website[256];

    ModState_t state;
    bool hasModInfo;
    bool isDisabled;
    bool isLoaded;

    // Material override support (discovered: mods/%s/materials/%s.vtf and .vmt)
    bool hasMaterials;
    char materialPaths[50][256]; // Support up to 50 material overrides
    int materialPathCount;

    ModData_t()
    {
        modName[0] = '\0';
        modPath[0] = '\0';
        modInfoPath[0] = '\0';
        title[0] = '\0';
        author[0] = '\0';
        description[0] = '\0';
        version[0] = '\0';
        website[0] = '\0';

        state = MOD_STATE_UNKNOWN;
        hasModInfo = false;
        isDisabled = false;
        isLoaded = false;
        hasMaterials = false;

        // Initialize material paths array
        for (int i = 0; i < 50; i++)
            materialPaths[i][0] = '\0';
        materialPathCount = 0;
    }
};

//-----------------------------------------------------------------------------
// GMod Mod System - Implements mod loading/management discovered through IDA analysis
// Based on strings: mods/%s, mods/%s/modinfo.txt, mods/%s/DISABLED, mods/-modcache/modcache.txt
//-----------------------------------------------------------------------------
class CGModModSystem : public CAutoGameSystem
{
public:
    CGModModSystem() : CAutoGameSystem() {}

    // CAutoGameSystem overrides
    virtual bool Init();
    virtual void Shutdown();
    virtual void LevelInitPostEntity();

    // Mod management functions discovered from IDA
    static bool LoadModCache(); // Function: GMod_LoadModCache (0x241B5230)
    static bool SaveModCache();
    static bool LoadModInfo(const char* pszModName); // Function: GMod_LoadModInfo (0x241B6090)
    static bool ScanModDirectory();
    static bool LoadMod(const char* pszModName);
    static bool UnloadMod(const char* pszModName);
    static bool EnableMod(const char* pszModName);
    static bool DisableMod(const char* pszModName);

    // Mod query functions
    static ModData_t* FindMod(const char* pszModName);
    static bool IsModLoaded(const char* pszModName);
    static bool IsModDisabled(const char* pszModName);
    static int GetModCount();
    static void GetModList(char modList[][256], int& modCount, int maxMods);

    // Material override system (discovered: mods/%s/materials/%s.vtf/.vmt)
    static bool LoadModMaterials(const char* pszModName);
    static bool CheckModMaterialOverride(const char* pszMaterialName, char* pszOverridePath, int maxLen);

    // Utility functions
    static const char* GetModStateName(ModState_t state);
    static const char* GetModsDirectory() { return "mods"; }

private:
    static CUtlVector<ModData_t> s_ModRegistry;
    static char s_ModCachePath[256]; // "mods/-modcache/modcache.txt"
    static bool s_bSystemInitialized;
    static bool s_bModCacheLoaded;

    // Helper functions based on IDA analysis
    static bool ValidateModDirectory(const char* pszModName);
    static bool ParseModInfo(const char* pszModInfoPath, ModData_t* pModData);
    static bool CreateDisabledFile(const char* pszModName);
    static bool RemoveDisabledFile(const char* pszModName);
    static bool CheckForDisabledFile(const char* pszModName);
    static void BuildModPaths(const char* pszModName, ModData_t* pModData);
};

// Global instance
extern CGModModSystem g_GMod_ModSystem;

// Console command handlers discovered from IDA analysis
void CMD_gmod_list_mods(void);
void CMD_gmod_load_mod(void);
void CMD_gmod_unload_mod(void);
void CMD_gmod_enable_mod(void);
void CMD_gmod_disable_mod(void);
void CMD_gmod_reload_mods(void);

#endif // GMOD_MOD_H