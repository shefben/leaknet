//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Configuration File Management System Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_config.h"
#include "player.h"
#include "tier1/strtools.h"
#include "filesystem.h"

// Initialize static members
bool CGModConfigManager::m_bInitialized = false;
CUtlVector<CGModConfigFile*> CGModConfigManager::m_ConfigFiles;
CGModConfigFile* CGModConfigManager::m_pConfigs[CONFIG_TYPE_COUNT];
bool CGModConfigManager::m_bAutoSave = true;
float CGModConfigManager::m_flAutoSaveInterval = 300.0f; // 5 minutes
float CGModConfigManager::m_flLastAutoSave = 0.0f;

// Console variables for config system
ConVar gm_config_autosave("gm_config_autosave", "1", FCVAR_GAMEDLL, "Enable automatic config saving");
ConVar gm_config_autosave_interval("gm_config_autosave_interval", "300", FCVAR_GAMEDLL, "Auto-save interval in seconds");
ConVar gm_config_backup("gm_config_backup", "1", FCVAR_GAMEDLL, "Create backup copies of config files");

//-----------------------------------------------------------------------------
// Config File Implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGModConfigFile::CGModConfigFile(const char* filename)
{
    Q_strncpy(m_szFilename, filename, sizeof(m_szFilename));
    m_pKeyValues = new KeyValues("Config");
    m_bModified = false;
    m_bLoaded = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CGModConfigFile::~CGModConfigFile()
{
    if (m_bModified)
        Save();

    if (m_pKeyValues)
        m_pKeyValues->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Load config file
//-----------------------------------------------------------------------------
bool CGModConfigFile::Load()
{
    const char* fullPath = GetFullPath();

    if (!filesystem->FileExists(fullPath, "MOD"))
    {
        m_bLoaded = true; // Consider empty file as loaded
        return true;
    }

    m_pKeyValues->Clear();
    bool success = m_pKeyValues->LoadFromFile(filesystem, fullPath, "MOD");

    if (success)
    {
        m_bLoaded = true;
        m_bModified = false;
        DevMsg("Loaded config file: %s\n", m_szFilename);
    }
    else
    {
        Warning("Failed to load config file: %s\n", m_szFilename);
    }

    return success;
}

//-----------------------------------------------------------------------------
// Purpose: Save config file
//-----------------------------------------------------------------------------
bool CGModConfigFile::Save()
{
    if (!m_bLoaded || !m_bModified)
        return true;

    const char* fullPath = GetFullPath();

    // Create backup if enabled
    if (gm_config_backup.GetBool() && filesystem->FileExists(fullPath, "MOD"))
    {
        char backupPath[256];
        Q_snprintf(backupPath, sizeof(backupPath), "%s.bak", fullPath);
        filesystem->RenameFile(fullPath, backupPath, "MOD");
    }

    bool success = m_pKeyValues->SaveToFile(filesystem, fullPath, "MOD");

    if (success)
    {
        m_bModified = false;
        DevMsg("Saved config file: %s\n", m_szFilename);
    }
    else
    {
        Warning("Failed to save config file: %s\n", m_szFilename);
    }

    return success;
}

//-----------------------------------------------------------------------------
// Purpose: Reload config file
//-----------------------------------------------------------------------------
bool CGModConfigFile::Reload()
{
    m_bLoaded = false;
    m_bModified = false;
    return Load();
}

//-----------------------------------------------------------------------------
// Purpose: Check if file exists
//-----------------------------------------------------------------------------
bool CGModConfigFile::Exists()
{
    return filesystem->FileExists(GetFullPath(), "MOD");
}

//-----------------------------------------------------------------------------
// Purpose: Set string value
//-----------------------------------------------------------------------------
void CGModConfigFile::SetString(const char* section, const char* key, const char* value, const char* comment)
{
    if (!m_pKeyValues)
        return;

    KeyValues* pSection = m_pKeyValues->FindKey(section, true);
    if (pSection)
    {
        pSection->SetString(key, value);
        m_bModified = true;

        if (comment && comment[0])
        {
            char commentKey[128];
            Q_snprintf(commentKey, sizeof(commentKey), "%s_comment", key);
            pSection->SetString(commentKey, comment);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Set integer value
//-----------------------------------------------------------------------------
void CGModConfigFile::SetInt(const char* section, const char* key, int value, const char* comment)
{
    if (!m_pKeyValues)
        return;

    KeyValues* pSection = m_pKeyValues->FindKey(section, true);
    if (pSection)
    {
        pSection->SetInt(key, value);
        m_bModified = true;

        if (comment && comment[0])
        {
            char commentKey[128];
            Q_snprintf(commentKey, sizeof(commentKey), "%s_comment", key);
            pSection->SetString(commentKey, comment);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Set float value
//-----------------------------------------------------------------------------
void CGModConfigFile::SetFloat(const char* section, const char* key, float value, const char* comment)
{
    if (!m_pKeyValues)
        return;

    KeyValues* pSection = m_pKeyValues->FindKey(section, true);
    if (pSection)
    {
        pSection->SetFloat(key, value);
        m_bModified = true;

        if (comment && comment[0])
        {
            char commentKey[128];
            Q_snprintf(commentKey, sizeof(commentKey), "%s_comment", key);
            pSection->SetString(commentKey, comment);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Set boolean value
//-----------------------------------------------------------------------------
void CGModConfigFile::SetBool(const char* section, const char* key, bool value, const char* comment)
{
    SetInt(section, key, value ? 1 : 0, comment);
}

//-----------------------------------------------------------------------------
// Purpose: Get string value
//-----------------------------------------------------------------------------
const char* CGModConfigFile::GetString(const char* section, const char* key, const char* defaultValue)
{
    if (!m_pKeyValues)
        return defaultValue;

    KeyValues* pSection = m_pKeyValues->FindKey(section);
    if (pSection)
        return pSection->GetString(key, defaultValue);

    return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get integer value
//-----------------------------------------------------------------------------
int CGModConfigFile::GetInt(const char* section, const char* key, int defaultValue)
{
    if (!m_pKeyValues)
        return defaultValue;

    KeyValues* pSection = m_pKeyValues->FindKey(section);
    if (pSection)
        return pSection->GetInt(key, defaultValue);

    return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get float value
//-----------------------------------------------------------------------------
float CGModConfigFile::GetFloat(const char* section, const char* key, float defaultValue)
{
    if (!m_pKeyValues)
        return defaultValue;

    KeyValues* pSection = m_pKeyValues->FindKey(section);
    if (pSection)
        return pSection->GetFloat(key, defaultValue);

    return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get boolean value
//-----------------------------------------------------------------------------
bool CGModConfigFile::GetBool(const char* section, const char* key, bool defaultValue)
{
    return GetInt(section, key, defaultValue ? 1 : 0) != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Remove section
//-----------------------------------------------------------------------------
void CGModConfigFile::RemoveSection(const char* section)
{
    if (!m_pKeyValues)
        return;

    KeyValues* pSection = m_pKeyValues->FindKey(section);
    if (pSection)
    {
        m_pKeyValues->RemoveSubKey(pSection);
        m_bModified = true;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Check if section exists
//-----------------------------------------------------------------------------
bool CGModConfigFile::HasSection(const char* section)
{
    if (!m_pKeyValues)
        return false;

    return m_pKeyValues->FindKey(section) != NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Remove key
//-----------------------------------------------------------------------------
void CGModConfigFile::RemoveKey(const char* section, const char* key)
{
    if (!m_pKeyValues)
        return;

    KeyValues* pSection = m_pKeyValues->FindKey(section);
    if (pSection)
    {
        KeyValues* pKey = pSection->FindKey(key);
        if (pKey)
        {
            pSection->RemoveSubKey(pKey);
            m_bModified = true;
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Check if key exists
//-----------------------------------------------------------------------------
bool CGModConfigFile::HasKey(const char* section, const char* key)
{
    if (!m_pKeyValues)
        return false;

    KeyValues* pSection = m_pKeyValues->FindKey(section);
    if (pSection)
        return pSection->FindKey(key) != NULL;

    return false;
}

//-----------------------------------------------------------------------------
// Purpose: Clear all data
//-----------------------------------------------------------------------------
void CGModConfigFile::Clear()
{
    if (m_pKeyValues)
    {
        m_pKeyValues->Clear();
        m_bModified = true;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Add comment
//-----------------------------------------------------------------------------
void CGModConfigFile::AddComment(const char* section, const char* comment)
{
    if (!comment || !comment[0])
        return;

    static int commentIndex = 0;
    char commentKey[64];
    Q_snprintf(commentKey, sizeof(commentKey), "_comment_%d", commentIndex++);

    SetString(section, commentKey, comment);
}

//-----------------------------------------------------------------------------
// Purpose: Set modified flag
//-----------------------------------------------------------------------------
void CGModConfigFile::SetModified(bool modified)
{
    m_bModified = modified;
}

//-----------------------------------------------------------------------------
// Purpose: Check if modified
//-----------------------------------------------------------------------------
bool CGModConfigFile::IsModified()
{
    return m_bModified;
}

//-----------------------------------------------------------------------------
// Purpose: Get full file path
//-----------------------------------------------------------------------------
const char* CGModConfigFile::GetFullPath()
{
    static char fullPath[256];
    Q_snprintf(fullPath, sizeof(fullPath), "cfg/gmod/%s", m_szFilename);
    return fullPath;
}

//-----------------------------------------------------------------------------
// Config Manager Implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Initialize config system
//-----------------------------------------------------------------------------
void CGModConfigManager::Initialize()
{
    if (m_bInitialized)
        return;

    // Initialize config array
    for (int i = 0; i < CONFIG_TYPE_COUNT; i++)
        m_pConfigs[i] = NULL;

    // Create default config files
    CreateDefaultConfigs();

    // Load all configs
    ReloadAllConfigs();

    m_flLastAutoSave = gpGlobals->curtime;
    m_bInitialized = true;

    Msg("Configuration System initialized\n");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown config system
//-----------------------------------------------------------------------------
void CGModConfigManager::Shutdown()
{
    if (!m_bInitialized)
        return;

    // Save all configs
    SaveAllConfigs();

    // Clean up config files
    for (int i = 0; i < m_ConfigFiles.Count(); i++)
    {
        delete m_ConfigFiles[i];
    }
    m_ConfigFiles.RemoveAll();

    // Clear config array
    for (int i = 0; i < CONFIG_TYPE_COUNT; i++)
        m_pConfigs[i] = NULL;

    m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Get config file by type
//-----------------------------------------------------------------------------
CGModConfigFile* CGModConfigManager::GetConfig(ConfigFileType_t type)
{
    if (type >= 0 && type < CONFIG_TYPE_COUNT)
        return m_pConfigs[type];

    return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Load specific config file
//-----------------------------------------------------------------------------
CGModConfigFile* CGModConfigManager::LoadConfig(const char* filename)
{
    // Check if already loaded
    for (int i = 0; i < m_ConfigFiles.Count(); i++)
    {
        if (Q_stricmp(m_ConfigFiles[i]->GetFilename(), filename) == 0)
            return m_ConfigFiles[i];
    }

    // Create new config file
    CGModConfigFile* pConfig = new CGModConfigFile(filename);
    pConfig->Load();
    m_ConfigFiles.AddToTail(pConfig);

    return pConfig;
}

//-----------------------------------------------------------------------------
// Purpose: Save all config files
//-----------------------------------------------------------------------------
void CGModConfigManager::SaveAllConfigs()
{
    for (int i = 0; i < m_ConfigFiles.Count(); i++)
    {
        m_ConfigFiles[i]->Save();
    }
}

//-----------------------------------------------------------------------------
// Purpose: Reload all config files
//-----------------------------------------------------------------------------
void CGModConfigManager::ReloadAllConfigs()
{
    for (int i = 0; i < m_ConfigFiles.Count(); i++)
    {
        m_ConfigFiles[i]->Reload();
    }

    // Apply server settings after reload
    ApplyServerSettings();
}

//-----------------------------------------------------------------------------
// Purpose: Create default config files
//-----------------------------------------------------------------------------
void CGModConfigManager::CreateDefaultConfigs()
{
    RegisterConfigFile(CONFIG_SERVER_SETTINGS, "server_settings.cfg");
    RegisterConfigFile(CONFIG_TOOL_SETTINGS, "tool_settings.cfg");
    RegisterConfigFile(CONFIG_SPAWN_LISTS, "spawn_lists.cfg");
    RegisterConfigFile(CONFIG_PLAYER_BINDINGS, "player_bindings.cfg");
    RegisterConfigFile(CONFIG_ENTITY_LIMITS, "entity_limits.cfg");
    RegisterConfigFile(CONFIG_GAME_RULES, "game_rules.cfg");
    RegisterConfigFile(CONFIG_CONTEXT_PANELS, "context_panels.cfg");
    RegisterConfigFile(CONFIG_USER_PREFERENCES, "user_preferences.cfg");
}

//-----------------------------------------------------------------------------
// Purpose: Register config file
//-----------------------------------------------------------------------------
void CGModConfigManager::RegisterConfigFile(ConfigFileType_t type, const char* filename)
{
    if (type < 0 || type >= CONFIG_TYPE_COUNT)
        return;

    CGModConfigFile* pConfig = LoadConfig(filename);
    m_pConfigs[type] = pConfig;
}

//-----------------------------------------------------------------------------
// Purpose: Load server settings
//-----------------------------------------------------------------------------
void CGModConfigManager::LoadServerSettings()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_SERVER_SETTINGS);
    if (!pConfig)
        return;

    // Set default server settings if file is empty
    if (!pConfig->HasSection("Server"))
    {
        pConfig->SetString("Server", "hostname", "Garry's Mod Server", "Server name displayed in browser");
        pConfig->SetInt("Server", "maxplayers", 16, "Maximum number of players");
        pConfig->SetString("Server", "gamemode", "sandbox", "Default game mode");
        pConfig->SetString("Server", "map", "gm_construct", "Default map");
        pConfig->SetBool("Server", "allow_npc", true, "Allow NPC spawning");
        pConfig->SetBool("Server", "allow_weapons", true, "Allow weapon spawning");
        pConfig->SetFloat("Server", "physgun_range", 1000.0f, "Physics gun maximum range");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Save server settings
//-----------------------------------------------------------------------------
void CGModConfigManager::SaveServerSettings()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_SERVER_SETTINGS);
    if (pConfig)
        pConfig->Save();
}

//-----------------------------------------------------------------------------
// Purpose: Apply server settings
//-----------------------------------------------------------------------------
void CGModConfigManager::ApplyServerSettings()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_SERVER_SETTINGS);
    if (!pConfig)
        return;

    // Apply server settings to ConVars
    ConVar* hostname = cvar->FindVar("hostname");
    if (hostname)
        hostname->SetValue(pConfig->GetString("Server", "hostname", "Garry's Mod Server"));

    ConVar* maxplayers = cvar->FindVar("maxplayers");
    if (maxplayers)
        maxplayers->SetValue(pConfig->GetInt("Server", "maxplayers", 16));
}

//-----------------------------------------------------------------------------
// Purpose: Load tool settings
//-----------------------------------------------------------------------------
void CGModConfigManager::LoadToolSettings()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_TOOL_SETTINGS);
    if (!pConfig)
        return;

    // Set default tool settings
    if (!pConfig->HasSection("Tools"))
    {
        pConfig->SetString("Tools", "default_tool", "physgun", "Default selected tool");
        pConfig->SetBool("Tools", "auto_weld", false, "Automatically weld created props");
        pConfig->SetFloat("Tools", "rope_width", 2.0f, "Default rope width");
        pConfig->SetInt("Tools", "prop_limit", 100, "Props per player limit");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Save tool settings
//-----------------------------------------------------------------------------
void CGModConfigManager::SaveToolSettings()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_TOOL_SETTINGS);
    if (pConfig)
        pConfig->Save();
}

//-----------------------------------------------------------------------------
// Purpose: Set tool setting
//-----------------------------------------------------------------------------
void CGModConfigManager::SetToolSetting(const char* tool, const char* key, const char* value)
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_TOOL_SETTINGS);
    if (pConfig)
        pConfig->SetString(tool, key, value);
}

//-----------------------------------------------------------------------------
// Purpose: Get tool setting
//-----------------------------------------------------------------------------
const char* CGModConfigManager::GetToolSetting(const char* tool, const char* key, const char* defaultValue)
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_TOOL_SETTINGS);
    if (pConfig)
        return pConfig->GetString(tool, key, defaultValue);

    return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Load spawn lists
//-----------------------------------------------------------------------------
void CGModConfigManager::LoadSpawnLists()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_SPAWN_LISTS);
    if (!pConfig)
        return;

    // Set default spawn list entries
    if (!pConfig->HasSection("Props"))
    {
        pConfig->SetString("Props", "models/props_c17/oildrum001.mdl", "Oil Drum");
        pConfig->SetString("Props", "models/props_junk/wood_crate001a.mdl", "Wooden Crate");
        pConfig->SetString("Props", "models/props_phx/construct/metal_plate1.mdl", "Metal Plate");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Save spawn lists
//-----------------------------------------------------------------------------
void CGModConfigManager::SaveSpawnLists()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_SPAWN_LISTS);
    if (pConfig)
        pConfig->Save();
}

//-----------------------------------------------------------------------------
// Purpose: Add spawn list entry
//-----------------------------------------------------------------------------
void CGModConfigManager::AddSpawnListEntry(const char* category, const char* model, const char* name)
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_SPAWN_LISTS);
    if (pConfig)
        pConfig->SetString(category, model, name);
}

//-----------------------------------------------------------------------------
// Purpose: Remove spawn list entry
//-----------------------------------------------------------------------------
void CGModConfigManager::RemoveSpawnListEntry(const char* category, const char* model)
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_SPAWN_LISTS);
    if (pConfig)
        pConfig->RemoveKey(category, model);
}

//-----------------------------------------------------------------------------
// Purpose: Load player settings
//-----------------------------------------------------------------------------
void CGModConfigManager::LoadPlayerSettings(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    char filename[64];
    Q_snprintf(filename, sizeof(filename), "player_%s.cfg", STRING(pPlayer->pl.netname));

    CGModConfigFile* pConfig = LoadConfig(filename);
    if (pConfig && !pConfig->HasSection("Player"))
    {
        // Set default player settings
        pConfig->SetString("Player", "preferred_tool", "physgun");
        pConfig->SetBool("Player", "auto_tool_switch", true);
        pConfig->SetInt("Player", "hud_color", 255);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Save player settings
//-----------------------------------------------------------------------------
void CGModConfigManager::SavePlayerSettings(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    char filename[64];
    Q_snprintf(filename, sizeof(filename), "player_%s.cfg", STRING(pPlayer->pl.netname));

    CGModConfigFile* pConfig = LoadConfig(filename);
    if (pConfig)
        pConfig->Save();
}

//-----------------------------------------------------------------------------
// Purpose: Set player setting
//-----------------------------------------------------------------------------
void CGModConfigManager::SetPlayerSetting(CBasePlayer* pPlayer, const char* key, const char* value)
{
    if (!pPlayer)
        return;

    char filename[64];
    Q_snprintf(filename, sizeof(filename), "player_%s.cfg", STRING(pPlayer->pl.netname));

    CGModConfigFile* pConfig = LoadConfig(filename);
    if (pConfig)
        pConfig->SetString("Player", key, value);
}

//-----------------------------------------------------------------------------
// Purpose: Get player setting
//-----------------------------------------------------------------------------
const char* CGModConfigManager::GetPlayerSetting(CBasePlayer* pPlayer, const char* key, const char* defaultValue)
{
    if (!pPlayer)
        return defaultValue;

    char filename[64];
    Q_snprintf(filename, sizeof(filename), "player_%s.cfg", STRING(pPlayer->pl.netname));

    CGModConfigFile* pConfig = LoadConfig(filename);
    if (pConfig)
        return pConfig->GetString("Player", key, defaultValue);

    return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Load entity limits
//-----------------------------------------------------------------------------
void CGModConfigManager::LoadEntityLimits()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_ENTITY_LIMITS);
    if (!pConfig)
        return;

    // Set default entity limits
    if (!pConfig->HasSection("Limits"))
    {
        pConfig->SetInt("Limits", "props_server", 1000);
        pConfig->SetInt("Limits", "props_client", 100);
        pConfig->SetInt("Limits", "ragdolls_server", 200);
        pConfig->SetInt("Limits", "ragdolls_client", 20);
        pConfig->SetInt("Limits", "balloons_server", 200);
        pConfig->SetInt("Limits", "balloons_client", 20);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Save entity limits
//-----------------------------------------------------------------------------
void CGModConfigManager::SaveEntityLimits()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_ENTITY_LIMITS);
    if (pConfig)
        pConfig->Save();
}

//-----------------------------------------------------------------------------
// Purpose: Set entity limit
//-----------------------------------------------------------------------------
void CGModConfigManager::SetEntityLimit(const char* entityType, int serverLimit, int clientLimit)
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_ENTITY_LIMITS);
    if (pConfig)
    {
        char serverKey[64], clientKey[64];
        Q_snprintf(serverKey, sizeof(serverKey), "%s_server", entityType);
        Q_snprintf(clientKey, sizeof(clientKey), "%s_client", entityType);

        pConfig->SetInt("Limits", serverKey, serverLimit);
        pConfig->SetInt("Limits", clientKey, clientLimit);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get entity limit
//-----------------------------------------------------------------------------
void CGModConfigManager::GetEntityLimit(const char* entityType, int& serverLimit, int& clientLimit)
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_ENTITY_LIMITS);
    if (pConfig)
    {
        char serverKey[64], clientKey[64];
        Q_snprintf(serverKey, sizeof(serverKey), "%s_server", entityType);
        Q_snprintf(clientKey, sizeof(clientKey), "%s_client", entityType);

        serverLimit = pConfig->GetInt("Limits", serverKey, 1000);
        clientLimit = pConfig->GetInt("Limits", clientKey, 100);
    }
    else
    {
        serverLimit = 1000;
        clientLimit = 100;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Load game rules
//-----------------------------------------------------------------------------
void CGModConfigManager::LoadGameRules()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_GAME_RULES);
    if (!pConfig)
        return;

    // Set default game rules
    if (!pConfig->HasSection("Rules"))
    {
        pConfig->SetString("Rules", "gamemode", "sandbox");
        pConfig->SetBool("Rules", "friendly_fire", false);
        pConfig->SetBool("Rules", "team_play", false);
        pConfig->SetFloat("Rules", "respawn_time", 5.0f);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Save game rules
//-----------------------------------------------------------------------------
void CGModConfigManager::SaveGameRules()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_GAME_RULES);
    if (pConfig)
        pConfig->Save();
}

//-----------------------------------------------------------------------------
// Purpose: Set game rule
//-----------------------------------------------------------------------------
void CGModConfigManager::SetGameRule(const char* rule, const char* value)
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_GAME_RULES);
    if (pConfig)
        pConfig->SetString("Rules", rule, value);
}

//-----------------------------------------------------------------------------
// Purpose: Get game rule
//-----------------------------------------------------------------------------
const char* CGModConfigManager::GetGameRule(const char* rule, const char* defaultValue)
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_GAME_RULES);
    if (pConfig)
        return pConfig->GetString("Rules", rule, defaultValue);

    return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Load context panels
//-----------------------------------------------------------------------------
void CGModConfigManager::LoadContextPanels()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_CONTEXT_PANELS);
    if (!pConfig)
        return;

    // Set default context panel entries
    if (!pConfig->HasSection("Panels"))
    {
        pConfig->SetString("Panels", "spawn_prop", "gm_spawn prop_physics");
        pConfig->SetString("Panels", "spawn_npc", "gm_spawn npc_citizen");
        pConfig->SetString("Panels", "remove_all", "gm_remove_all");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Save context panels
//-----------------------------------------------------------------------------
void CGModConfigManager::SaveContextPanels()
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_CONTEXT_PANELS);
    if (pConfig)
        pConfig->Save();
}

//-----------------------------------------------------------------------------
// Purpose: Add context panel entry
//-----------------------------------------------------------------------------
void CGModConfigManager::AddContextPanelEntry(const char* panel, const char* title, const char* command)
{
    CGModConfigFile* pConfig = GetConfig(CONFIG_CONTEXT_PANELS);
    if (pConfig)
        pConfig->SetString(panel, title, command);
}

//-----------------------------------------------------------------------------
// Purpose: Enable auto-save
//-----------------------------------------------------------------------------
void CGModConfigManager::EnableAutoSave(bool enable)
{
    m_bAutoSave = enable;
    gm_config_autosave.SetValue(enable ? 1 : 0);
}

//-----------------------------------------------------------------------------
// Purpose: Set auto-save interval
//-----------------------------------------------------------------------------
void CGModConfigManager::SetAutoSaveInterval(float seconds)
{
    m_flAutoSaveInterval = seconds;
    gm_config_autosave_interval.SetValue(seconds);
}

//-----------------------------------------------------------------------------
// Purpose: Perform auto-save
//-----------------------------------------------------------------------------
void CGModConfigManager::PerformAutoSave()
{
    if (!m_bAutoSave)
        return;

    if (gpGlobals->curtime > m_flLastAutoSave + m_flAutoSaveInterval)
    {
        SaveAllConfigs();
        m_flLastAutoSave = gpGlobals->curtime;
        DevMsg("Auto-saved configuration files\n");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Check auto-save
//-----------------------------------------------------------------------------
void CGModConfigManager::CheckAutoSave()
{
    m_bAutoSave = gm_config_autosave.GetBool();
    m_flAutoSaveInterval = gm_config_autosave_interval.GetFloat();

    PerformAutoSave();
}

//-----------------------------------------------------------------------------
// Purpose: Get command player
//-----------------------------------------------------------------------------
CBasePlayer* CGModConfigManager::GetCommandPlayer()
{
    return UTIL_GetCommandClient();
}

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Reload all configs
//-----------------------------------------------------------------------------
void CGModConfigManager::CMD_gm_config_reload(void)
{
    ReloadAllConfigs();

    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
        ClientPrint(pPlayer, HUD_PRINTTALK, "Configuration files reloaded");
}

//-----------------------------------------------------------------------------
// Purpose: Save all configs
//-----------------------------------------------------------------------------
void CGModConfigManager::CMD_gm_config_save(void)
{
    SaveAllConfigs();

    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
        ClientPrint(pPlayer, HUD_PRINTTALK, "Configuration files saved");
}

//-----------------------------------------------------------------------------
// Purpose: Reset configs to defaults
//-----------------------------------------------------------------------------
void CGModConfigManager::CMD_gm_config_reset(void)
{
    // Clear all configs and reload defaults
    for (int i = 0; i < CONFIG_TYPE_COUNT; i++)
    {
        if (m_pConfigs[i])
            m_pConfigs[i]->Clear();
    }

    CreateDefaultConfigs();

    CBasePlayer* pPlayer = GetCommandPlayer();
    if (pPlayer)
        ClientPrint(pPlayer, HUD_PRINTTALK, "Configuration files reset to defaults");
}

//-----------------------------------------------------------------------------
// Purpose: List all config files
//-----------------------------------------------------------------------------
void CGModConfigManager::CMD_gm_config_list(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, "Configuration files:");

    for (int i = 0; i < m_ConfigFiles.Count(); i++)
    {
        CGModConfigFile* pConfig = m_ConfigFiles[i];
        const char* status = pConfig->IsModified() ? "Modified" : "Saved";
        ClientPrint(pPlayer, HUD_PRINTTALK, "  %s [%s]", pConfig->GetFilename(), status);
    }
}

// Register console commands
static ConCommand cmd_gm_config_reload("gm_config_reload", CGModConfigManager::CMD_gm_config_reload, "Reload all configuration files");
static ConCommand cmd_gm_config_save("gm_config_save", CGModConfigManager::CMD_gm_config_save, "Save all configuration files");
static ConCommand cmd_gm_config_reset("gm_config_reset", CGModConfigManager::CMD_gm_config_reset, "Reset configuration files to defaults");
static ConCommand cmd_gm_config_list("gm_config_list", CGModConfigManager::CMD_gm_config_list, "List all configuration files");

//-----------------------------------------------------------------------------
// System initialization hook
//-----------------------------------------------------------------------------
class CConfigSystemInit : public CAutoGameSystem
{
public:
    CConfigSystemInit() : CAutoGameSystem("ConfigSystemInit") {}

    virtual bool Init()
    {
        CGModConfigManager::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModConfigManager::Shutdown();
    }

    virtual void FrameUpdatePostEntityThink()
    {
        // Check auto-save periodically
        CGModConfigManager::CheckAutoSave();
    }

    virtual void LevelInitPostEntity()
    {
        // Load server settings when level loads
        CGModConfigManager::LoadServerSettings();
        CGModConfigManager::ApplyServerSettings();
    }
};

static CConfigSystemInit g_ConfigSystemInit;