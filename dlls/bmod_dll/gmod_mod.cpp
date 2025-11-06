#include "gmod_mod.h"
#include "cbase.h"
#include "player.h"
#include "filesystem.h"
#include "tier0/memdbgon.h"

// Define missing flag for LeakNet compatibility
#ifndef FCVAR_GAMEDLL
#define FCVAR_GAMEDLL 0
#endif

// Static member definitions
CUtlVector<ModData_t> CGModModSystem::s_ModRegistry;
char CGModModSystem::s_ModCachePath[256] = "mods/-modcache/modcache.txt";
bool CGModModSystem::s_bSystemInitialized = false;
bool CGModModSystem::s_bModCacheLoaded = false;

// Global instance
CGModModSystem g_GMod_ModSystem;

// ConVars for mod system configuration
ConVar gmod_mod_enabled("gmod_mod_enabled", "1", FCVAR_GAMEDLL, "Enable/disable mod system");
ConVar gmod_mod_debug("gmod_mod_debug", "0", FCVAR_GAMEDLL, "Debug mod system");
ConVar gmod_mod_autoload("gmod_mod_autoload", "1", FCVAR_GAMEDLL, "Automatically load mods on startup");

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
// CGModModSystem implementation based on IDA analysis
//-----------------------------------------------------------------------------
bool CGModModSystem::Init()
{
    if (!gmod_mod_enabled.GetBool())
    {
        DevMsg("GMod Mod System disabled by ConVar\n");
        return true;
    }

    s_ModRegistry.Purge();
    Q_strncpy(s_ModCachePath, "mods/-modcache/modcache.txt", sizeof(s_ModCachePath)); // Discovered from IDA: 0x24242ff0
    s_bModCacheLoaded = false;

    if (gmod_mod_autoload.GetBool())
    {
        LoadModCache();
        ScanModDirectory();
    }

    s_bSystemInitialized = true;

    DevMsg("GMod Mod System initialized with %d mods\n", s_ModRegistry.Count());
    return true;
}

void CGModModSystem::Shutdown()
{
    SaveModCache();
    s_ModRegistry.Purge();
    s_bSystemInitialized = false;

    DevMsg("GMod Mod System shutdown\n");
}

void CGModModSystem::LevelInitPostEntity()
{
    if (!s_bSystemInitialized)
        return;

    // Load any enabled mods
    for (int i = 0; i < s_ModRegistry.Count(); i++)
    {
        if (s_ModRegistry[i].state == MOD_STATE_ENABLED && !s_ModRegistry[i].isLoaded)
        {
            LoadMod(s_ModRegistry[i].modName);
        }
    }

    DevMsg("GMod Mod System: Level initialized, mods loaded\n");
}

// Implementation of functions discovered from IDA analysis
bool CGModModSystem::LoadModCache()
{
    if (!s_bSystemInitialized)
        return false;

    // Implementation of GMod_LoadModCache function (IDA: 0x241B5230)
    if (!filesystem->FileExists(s_ModCachePath, "GAME"))
    {
        if (gmod_mod_debug.GetBool())
        {
            DevMsg("Mod cache file not found: %s\n", s_ModCachePath);
        }
        return false;
    }

    FileHandle_t hFile = filesystem->Open(s_ModCachePath, "r", "GAME");
    if (hFile == FILESYSTEM_INVALID_HANDLE)
    {
        Warning("Failed to open mod cache: %s\n", s_ModCachePath);
        return false;
    }

    char line[512];
    while (filesystem->ReadLine(line, sizeof(line), hFile))
    {
        // Parse mod cache entries
        char modName[256];
        char state[64];
        if (sscanf(line, "%255s %63s", modName, state) == 2)
        {
            ModData_t* pMod = FindMod(modName);
            if (!pMod)
            {
                ModData_t modData;
                Q_strncpy(modData.modName, modName, sizeof(modData.modName));
                BuildModPaths(modName, &modData);
                s_ModRegistry.AddToTail(modData);
                pMod = &s_ModRegistry[s_ModRegistry.Count() - 1];
            }

            if (Q_stricmp(state, "enabled") == 0)
                pMod->state = MOD_STATE_ENABLED;
            else if (Q_stricmp(state, "disabled") == 0)
                pMod->state = MOD_STATE_DISABLED;
        }
    }

    filesystem->Close(hFile);
    s_bModCacheLoaded = true;

    if (gmod_mod_debug.GetBool())
    {
        DevMsg("Loaded mod cache with %d entries\n", s_ModRegistry.Count());
    }

    return true;
}

bool CGModModSystem::SaveModCache()
{
    if (!s_bSystemInitialized)
        return false;

    FileHandle_t hFile = filesystem->Open(s_ModCachePath, "w", "GAME");
    if (hFile == FILESYSTEM_INVALID_HANDLE)
    {
        Warning("Failed to create mod cache: %s\n", s_ModCachePath);
        return false;
    }

    for (int i = 0; i < s_ModRegistry.Count(); i++)
    {
        const ModData_t& mod = s_ModRegistry[i];
        const char* stateName = GetModStateName(mod.state);
        filesystem->FPrintf(hFile, "%s %s\n", mod.modName, stateName);
    }

    filesystem->Close(hFile);

    if (gmod_mod_debug.GetBool())
    {
        DevMsg("Saved mod cache with %d entries\n", s_ModRegistry.Count());
    }

    return true;
}

bool CGModModSystem::LoadModInfo(const char* pszModName)
{
    if (!pszModName || !s_bSystemInitialized)
        return false;

    // Implementation of GMod_LoadModInfo function (IDA: 0x241B6090)
    ModData_t* pMod = FindMod(pszModName);
    if (!pMod)
        return false;

    // Check if mod is disabled (discovered string: "mods/%s/DISABLED")
    if (CheckForDisabledFile(pszModName))
    {
        pMod->isDisabled = true;
        pMod->state = MOD_STATE_DISABLED;
        return true;
    }

    // Load modinfo.txt (discovered string: "mods/%s/modinfo.txt")
    if (filesystem->FileExists(pMod->modInfoPath, "GAME"))
    {
        if (ParseModInfo(pMod->modInfoPath, pMod))
        {
            pMod->hasModInfo = true;
            if (gmod_mod_debug.GetBool())
            {
                DevMsg("Loaded mod info for: %s\n", pszModName);
            }
            return true;
        }
        else
        {
            Warning("Failed to parse modinfo.txt for mod: %s\n", pszModName);
        }
    }

    return false;
}

bool CGModModSystem::ScanModDirectory()
{
    if (!s_bSystemInitialized)
        return false;

    // Scan mods directory (discovered string: "mods/*")
    FileFindHandle_t findHandle;
    const char* pFileName = filesystem->FindFirst("mods/*", &findHandle);

    while (pFileName != NULL)
    {
        if (filesystem->FindIsDirectory(findHandle))
        {
            // Skip special directories
            if (Q_strcmp(pFileName, ".") != 0 && Q_strcmp(pFileName, "..") != 0 && Q_strcmp(pFileName, "-modcache") != 0)
            {
                // Check if mod already exists
                if (!FindMod(pFileName))
                {
                    ModData_t modData;
                    Q_strncpy(modData.modName, pFileName, sizeof(modData.modName));
                    BuildModPaths(pFileName, &modData);

                    // Determine initial state
                    if (CheckForDisabledFile(pFileName))
                    {
                        modData.state = MOD_STATE_DISABLED;
                        modData.isDisabled = true;
                    }
                    else
                    {
                        modData.state = MOD_STATE_ENABLED;
                    }

                    s_ModRegistry.AddToTail(modData);

                    // Load mod info
                    LoadModInfo(pFileName);

                    if (gmod_mod_debug.GetBool())
                    {
                        DevMsg("Discovered mod: %s\n", pFileName);
                    }
                }
            }
        }

        pFileName = filesystem->FindNext(findHandle);
    }

    filesystem->FindClose(findHandle);

    if (gmod_mod_debug.GetBool())
    {
        DevMsg("Scanned mods directory, found %d mods\n", s_ModRegistry.Count());
    }

    return true;
}

bool CGModModSystem::LoadMod(const char* pszModName)
{
    if (!pszModName || !s_bSystemInitialized)
        return false;

    ModData_t* pMod = FindMod(pszModName);
    if (!pMod)
    {
        Warning("Mod not found: %s\n", pszModName);
        return false;
    }

    if (pMod->isLoaded)
    {
        DevMsg("Mod already loaded: %s\n", pszModName);
        return true;
    }

    if (pMod->isDisabled)
    {
        DevMsg("Mod is disabled: %s\n", pszModName);
        return false;
    }

    // Load mod materials if available
    LoadModMaterials(pszModName);

    pMod->isLoaded = true;
    pMod->state = MOD_STATE_ENABLED;

    if (gmod_mod_debug.GetBool())
    {
        DevMsg("Loaded mod: %s\n", pszModName);
    }

    return true;
}

bool CGModModSystem::UnloadMod(const char* pszModName)
{
    if (!pszModName || !s_bSystemInitialized)
        return false;

    ModData_t* pMod = FindMod(pszModName);
    if (!pMod || !pMod->isLoaded)
        return false;

    pMod->isLoaded = false;

    if (gmod_mod_debug.GetBool())
    {
        DevMsg("Unloaded mod: %s\n", pszModName);
    }

    return true;
}

bool CGModModSystem::EnableMod(const char* pszModName)
{
    if (!pszModName || !s_bSystemInitialized)
        return false;

    ModData_t* pMod = FindMod(pszModName);
    if (!pMod)
        return false;

    if (RemoveDisabledFile(pszModName))
    {
        pMod->isDisabled = false;
        pMod->state = MOD_STATE_ENABLED;

        if (gmod_mod_debug.GetBool())
        {
            DevMsg("Enabled mod: %s\n", pszModName);
        }

        return true;
    }

    return false;
}

bool CGModModSystem::DisableMod(const char* pszModName)
{
    if (!pszModName || !s_bSystemInitialized)
        return false;

    ModData_t* pMod = FindMod(pszModName);
    if (!pMod)
        return false;

    if (CreateDisabledFile(pszModName))
    {
        pMod->isDisabled = true;
        pMod->state = MOD_STATE_DISABLED;

        // Unload if currently loaded
        if (pMod->isLoaded)
        {
            UnloadMod(pszModName);
        }

        if (gmod_mod_debug.GetBool())
        {
            DevMsg("Disabled mod: %s\n", pszModName);
        }

        return true;
    }

    return false;
}

// Material override system implementation (discovered: mods/%s/materials/%s.vtf/.vmt)
bool CGModModSystem::LoadModMaterials(const char* pszModName)
{
    if (!pszModName || !s_bSystemInitialized)
        return false;

    ModData_t* pMod = FindMod(pszModName);
    if (!pMod)
        return false;

    char materialPath[MAX_PATH];
    Q_snprintf(materialPath, sizeof(materialPath), "mods/%s/materials", pszModName);

    if (filesystem->IsDirectory(materialPath, "GAME"))
    {
        pMod->hasMaterials = true;

        if (gmod_mod_debug.GetBool())
        {
            DevMsg("Mod %s has material overrides\n", pszModName);
        }

        return true;
    }

    return false;
}

bool CGModModSystem::CheckModMaterialOverride(const char* pszMaterialName, char* pszOverridePath, int maxLen)
{
    if (!pszMaterialName || !pszOverridePath || !s_bSystemInitialized)
        return false;

    // Check each loaded mod for material overrides
    for (int i = 0; i < s_ModRegistry.Count(); i++)
    {
        const ModData_t& mod = s_ModRegistry[i];
        if (mod.isLoaded && mod.hasMaterials)
        {
            // Check for .vmt override (discovered string: "mods/%s/materials/%s.vmt")
            char vmtPath[MAX_PATH];
            Q_snprintf(vmtPath, sizeof(vmtPath), "mods/%s/materials/%s.vmt", mod.modName, pszMaterialName);

            if (filesystem->FileExists(vmtPath, "GAME"))
            {
                Q_strncpy(pszOverridePath, vmtPath, maxLen);
                return true;
            }

            // Check for .vtf override (discovered string: "mods/%s/materials/%s.vtf")
            char vtfPath[MAX_PATH];
            Q_snprintf(vtfPath, sizeof(vtfPath), "mods/%s/materials/%s.vtf", mod.modName, pszMaterialName);

            if (filesystem->FileExists(vtfPath, "GAME"))
            {
                Q_strncpy(pszOverridePath, vtfPath, maxLen);
                return true;
            }
        }
    }

    return false;
}

// Query functions
ModData_t* CGModModSystem::FindMod(const char* pszModName)
{
    if (!pszModName)
        return NULL;

    for (int i = 0; i < s_ModRegistry.Count(); i++)
    {
        if (Q_stricmp(s_ModRegistry[i].modName, pszModName) == 0)
        {
            return &s_ModRegistry[i];
        }
    }

    return NULL;
}

bool CGModModSystem::IsModLoaded(const char* pszModName)
{
    ModData_t* pMod = FindMod(pszModName);
    return pMod && pMod->isLoaded;
}

bool CGModModSystem::IsModDisabled(const char* pszModName)
{
    ModData_t* pMod = FindMod(pszModName);
    return pMod && pMod->isDisabled;
}

int CGModModSystem::GetModCount()
{
    return s_ModRegistry.Count();
}

void CGModModSystem::GetModList(char modList[][256], int& modCount, int maxMods)
{
    modCount = 0;
    for (int i = 0; i < s_ModRegistry.Count() && modCount < maxMods; i++)
    {
        Q_strncpy(modList[modCount], s_ModRegistry[i].modName, 256);
        modCount++;
    }
}

const char* CGModModSystem::GetModStateName(ModState_t state)
{
    switch (state)
    {
        case MOD_STATE_ENABLED: return "enabled";
        case MOD_STATE_DISABLED: return "disabled";
        case MOD_STATE_LOADING: return "loading";
        case MOD_STATE_ERROR: return "error";
        default: return "unknown";
    }
}

// Helper function implementations
bool CGModModSystem::ValidateModDirectory(const char* pszModName)
{
    if (!pszModName)
        return false;

    char modPath[MAX_PATH];
    Q_snprintf(modPath, sizeof(modPath), "mods/%s", pszModName); // Discovered string: "mods/%s"

    return filesystem->IsDirectory(modPath, "GAME");
}

bool CGModModSystem::ParseModInfo(const char* pszModInfoPath, ModData_t* pModData)
{
    if (!pszModInfoPath || !pModData)
        return false;

    FileHandle_t hFile = filesystem->Open(pszModInfoPath, "r", "GAME");
    if (hFile == FILESYSTEM_INVALID_HANDLE)
        return false;

    char line[512];
    while (filesystem->ReadLine(line, sizeof(line), hFile))
    {
        // Simple key-value parsing
        char key[256], value[256];
        if (sscanf(line, "\"%255[^\"]\"\t\"%255[^\"]\"", key, value) == 2)
        {
            if (Q_stricmp(key, "title") == 0)
                Q_strncpy(pModData->title, value, sizeof(pModData->title));
            else if (Q_stricmp(key, "author") == 0)
                Q_strncpy(pModData->author, value, sizeof(pModData->author));
            else if (Q_stricmp(key, "description") == 0)
                Q_strncpy(pModData->description, value, sizeof(pModData->description));
            else if (Q_stricmp(key, "version") == 0)
                Q_strncpy(pModData->version, value, sizeof(pModData->version));
            else if (Q_stricmp(key, "website") == 0)
                Q_strncpy(pModData->website, value, sizeof(pModData->website));
        }
    }

    filesystem->Close(hFile);
    return true;
}

bool CGModModSystem::CreateDisabledFile(const char* pszModName)
{
    if (!pszModName)
        return false;

    char disabledPath[MAX_PATH];
    Q_snprintf(disabledPath, sizeof(disabledPath), "mods/%s/DISABLED", pszModName); // Discovered string: "mods/%s/DISABLED"

    FileHandle_t hFile = filesystem->Open(disabledPath, "w", "GAME");
    if (hFile == FILESYSTEM_INVALID_HANDLE)
        return false;

    filesystem->Close(hFile);
    return true;
}

bool CGModModSystem::RemoveDisabledFile(const char* pszModName)
{
    if (!pszModName)
        return false;

    char disabledPath[MAX_PATH];
    Q_snprintf(disabledPath, sizeof(disabledPath), "mods/%s/DISABLED", pszModName);

    if (filesystem->FileExists(disabledPath, "GAME"))
    {
        filesystem->RemoveFile(disabledPath, "GAME");
        return true;
    }

    return false;
}

bool CGModModSystem::CheckForDisabledFile(const char* pszModName)
{
    if (!pszModName)
        return false;

    char disabledPath[MAX_PATH];
    Q_snprintf(disabledPath, sizeof(disabledPath), "mods/%s/DISABLED", pszModName);

    return filesystem->FileExists(disabledPath, "GAME");
}

void CGModModSystem::BuildModPaths(const char* pszModName, ModData_t* pModData)
{
    if (!pszModName || !pModData)
        return;

    // Build standard mod paths based on discovered strings
    Q_snprintf(pModData->modPath, sizeof(pModData->modPath), "mods/%s/", pszModName); // Discovered string: "mods/%s/"
    Q_snprintf(pModData->modInfoPath, sizeof(pModData->modInfoPath), "mods/%s/modinfo.txt", pszModName); // Discovered string: "mods/%s/modinfo.txt"
}

//-----------------------------------------------------------------------------
// Console command implementations
//-----------------------------------------------------------------------------
void CMD_gmod_list_mods(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Available mods:");

    char modList[100][256]; // Support up to 100 mods
    int modCount;
    CGModModSystem::GetModList(modList, modCount, 100);

    for (int i = 0; i < modCount; i++)
    {
        ModData_t* pMod = CGModModSystem::FindMod(modList[i]);
        if (pMod)
        {
            const char* status = pMod->isLoaded ? "loaded" : (pMod->isDisabled ? "disabled" : "available");
            char szModInfo[512];
            Q_snprintf(szModInfo, sizeof(szModInfo), "  %s (%s)", pMod->modName, status);
            ClientPrint(pPlayer, HUD_PRINTTALK, szModInfo);
        }
    }

    char szMessage[256];
    Q_snprintf(szMessage, sizeof(szMessage), "Total: %d mods", modCount);
    ClientPrint(pPlayer, HUD_PRINTTALK, szMessage);
}

void CMD_gmod_load_mod(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() < 2)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Usage: gmod_load_mod <mod_name>");
        return;
    }

    const char* modName = engine->Cmd_Argv(1);
    if (CGModModSystem::LoadMod(modName))
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Loaded mod: %s", modName);
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to load mod: %s", modName);
    }
}

void CMD_gmod_unload_mod(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() < 2)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Usage: gmod_unload_mod <mod_name>");
        return;
    }

    const char* modName = engine->Cmd_Argv(1);
    if (CGModModSystem::UnloadMod(modName))
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Unloaded mod: %s", modName);
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to unload mod: %s", modName);
    }
}

void CMD_gmod_enable_mod(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() < 2)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Usage: gmod_enable_mod <mod_name>");
        return;
    }

    const char* modName = engine->Cmd_Argv(1);
    if (CGModModSystem::EnableMod(modName))
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Enabled mod: %s", modName);
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to enable mod: %s", modName);
    }
}

void CMD_gmod_disable_mod(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() < 2)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Usage: gmod_disable_mod <mod_name>");
        return;
    }

    const char* modName = engine->Cmd_Argv(1);
    if (CGModModSystem::DisableMod(modName))
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Disabled mod: %s", modName);
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to disable mod: %s", modName);
    }
}

void CMD_gmod_reload_mods(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModModSystem::SaveModCache();
    CGModModSystem::s_ModRegistry.Purge();
    CGModModSystem::ScanModDirectory();
    CGModModSystem::LoadModCache();

    ClientPrint(pPlayer, HUD_PRINTTALK, "Reloaded mod system");
}

//-----------------------------------------------------------------------------
// Console command registration
//-----------------------------------------------------------------------------
static ConCommand gmod_list_mods_cmd("gmod_list_mods", CMD_gmod_list_mods, "List all available mods");
static ConCommand gmod_load_mod_cmd("gmod_load_mod", CMD_gmod_load_mod, "Load a mod");
static ConCommand gmod_unload_mod_cmd("gmod_unload_mod", CMD_gmod_unload_mod, "Unload a mod");
static ConCommand gmod_enable_mod_cmd("gmod_enable_mod", CMD_gmod_enable_mod, "Enable a mod");
static ConCommand gmod_disable_mod_cmd("gmod_disable_mod", CMD_gmod_disable_mod, "Disable a mod");
static ConCommand gmod_reload_mods_cmd("gmod_reload_mods", CMD_gmod_reload_mods, "Reload mod system");