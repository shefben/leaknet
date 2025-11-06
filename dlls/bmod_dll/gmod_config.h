//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Configuration File Management System - GMod 9.0.4b compatible
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_CONFIG_H
#define GMOD_CONFIG_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "tier1/utlvector.h"
#include "tier1/utlstring.h"
#include "tier1/KeyValues.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CBasePlayer;

//-----------------------------------------------------------------------------
// Config File Types
//-----------------------------------------------------------------------------
enum ConfigFileType_t
{
    CONFIG_SERVER_SETTINGS = 0,
    CONFIG_TOOL_SETTINGS,
    CONFIG_SPAWN_LISTS,
    CONFIG_PLAYER_BINDINGS,
    CONFIG_ENTITY_LIMITS,
    CONFIG_GAME_RULES,
    CONFIG_CONTEXT_PANELS,
    CONFIG_USER_PREFERENCES,

    CONFIG_TYPE_COUNT
};

//-----------------------------------------------------------------------------
// Config Entry Structure
//-----------------------------------------------------------------------------
struct ConfigEntry_t
{
    CUtlString key;
    CUtlString value;
    CUtlString section;
    CUtlString comment;
    bool modified;
};

//-----------------------------------------------------------------------------
// Config File Class
//-----------------------------------------------------------------------------
class CGModConfigFile
{
public:
    CGModConfigFile(const char* filename);
    ~CGModConfigFile();

    // File operations
    bool Load();
    bool Save();
    bool Reload();
    bool Exists();

    // Value operations
    void SetString(const char* section, const char* key, const char* value, const char* comment = NULL);
    void SetInt(const char* section, const char* key, int value, const char* comment = NULL);
    void SetFloat(const char* section, const char* key, float value, const char* comment = NULL);
    void SetBool(const char* section, const char* key, bool value, const char* comment = NULL);

    const char* GetString(const char* section, const char* key, const char* defaultValue = "");
    int GetInt(const char* section, const char* key, int defaultValue = 0);
    float GetFloat(const char* section, const char* key, float defaultValue = 0.0f);
    bool GetBool(const char* section, const char* key, bool defaultValue = false);

    // Section operations
    void SetSection(const char* section);
    void RemoveSection(const char* section);
    bool HasSection(const char* section);

    // Key operations
    void RemoveKey(const char* section, const char* key);
    bool HasKey(const char* section, const char* key);

    // Utility functions
    void Clear();
    void AddComment(const char* section, const char* comment);
    void SetModified(bool modified = true);
    bool IsModified();

    const char* GetFilename() const { return m_szFilename; }

private:
    char m_szFilename[256];
    KeyValues* m_pKeyValues;
    bool m_bModified;
    bool m_bLoaded;

    void ParseFromKeyValues();
    void WriteToKeyValues();
    const char* GetFullPath();
};

//-----------------------------------------------------------------------------
// Configuration Management System
//-----------------------------------------------------------------------------
class CGModConfigManager
{
public:
    // System management
    static void Initialize();
    static void Shutdown();

    // Config file management
    static CGModConfigFile* GetConfig(ConfigFileType_t type);
    static CGModConfigFile* LoadConfig(const char* filename);
    static void SaveAllConfigs();
    static void ReloadAllConfigs();

    // Server settings
    static void LoadServerSettings();
    static void SaveServerSettings();
    static void ApplyServerSettings();

    // Tool settings
    static void LoadToolSettings();
    static void SaveToolSettings();
    static void SetToolSetting(const char* tool, const char* key, const char* value);
    static const char* GetToolSetting(const char* tool, const char* key, const char* defaultValue = "");

    // Spawn lists
    static void LoadSpawnLists();
    static void SaveSpawnLists();
    static void AddSpawnListEntry(const char* category, const char* model, const char* name);
    static void RemoveSpawnListEntry(const char* category, const char* model);

    // Player settings
    static void LoadPlayerSettings(CBasePlayer* pPlayer);
    static void SavePlayerSettings(CBasePlayer* pPlayer);
    static void SetPlayerSetting(CBasePlayer* pPlayer, const char* key, const char* value);
    static const char* GetPlayerSetting(CBasePlayer* pPlayer, const char* key, const char* defaultValue = "");

    // Entity limits
    static void LoadEntityLimits();
    static void SaveEntityLimits();
    static void SetEntityLimit(const char* entityType, int serverLimit, int clientLimit);
    static void GetEntityLimit(const char* entityType, int& serverLimit, int& clientLimit);

    // Game rules
    static void LoadGameRules();
    static void SaveGameRules();
    static void SetGameRule(const char* rule, const char* value);
    static const char* GetGameRule(const char* rule, const char* defaultValue = "");

    // Context panels
    static void LoadContextPanels();
    static void SaveContextPanels();
    static void AddContextPanelEntry(const char* panel, const char* title, const char* command);

    // Auto-save functionality
    static void EnableAutoSave(bool enable);
    static void SetAutoSaveInterval(float seconds);
    static void PerformAutoSave();

    // Console command handlers
    static void CMD_gm_config_reload(void);
    static void CMD_gm_config_save(void);
    static void CMD_gm_config_reset(void);
    static void CMD_gm_config_list(void);

private:
    static bool m_bInitialized;
    static CUtlVector<CGModConfigFile*> m_ConfigFiles;
    static CGModConfigFile* m_pConfigs[CONFIG_TYPE_COUNT];
    static bool m_bAutoSave;
    static float m_flAutoSaveInterval;
    static float m_flLastAutoSave;

    static void CreateDefaultConfigs();
    static void RegisterConfigFile(ConfigFileType_t type, const char* filename);
    static const char* GetConfigPath(const char* filename);
    static CBasePlayer* GetCommandPlayer();
    static void CheckAutoSave();
};

// Console variables for config system
extern ConVar gm_config_autosave;
extern ConVar gm_config_autosave_interval;
extern ConVar gm_config_backup;

#endif // GMOD_CONFIG_H