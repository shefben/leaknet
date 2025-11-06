#ifndef GMOD_ENTITIES_H
#define GMOD_ENTITIES_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "utlvector.h"
#include "utlstring.h"
#include "utldict.h"

// Forward declarations
class CBaseEntity;
class CBasePlayer;

// Entity configuration data loaded from settings/entities/%s.txt
struct EntityConfig_t
{
    CUtlString entityName;
    CUtlString className;
    CUtlString displayName;
    CUtlString description;
    CUtlString model;
    CUtlString material;
    CUtlString spawnSound;

    // Physics properties
    float flMass;
    float flHealth;
    bool bBreakable;
    bool bSolid;
    bool bStatic;

    // Appearance properties
    Vector vecColor;
    float flScale;
    int iSkin;

    // Behavior properties
    bool bCanPick;
    bool bCanFreeze;
    bool bCanWeld;
    bool bIsEnabled;

    EntityConfig_t()
    {
        flMass = 1.0f;
        flHealth = 100.0f;
        bBreakable = true;
        bSolid = true;
        bStatic = false;
        vecColor = Vector(1.0f, 1.0f, 1.0f);
        flScale = 1.0f;
        iSkin = 0;
        bCanPick = true;
        bCanFreeze = true;
        bCanWeld = true;
        bIsEnabled = true;
    }
};

// Entity factory data for custom entities
struct EntityFactoryData_t
{
    CUtlString factoryName;
    CUtlString baseClass;
    EntityConfig_t config;
    bool bRegistered;

    EntityFactoryData_t()
    {
        bRegistered = false;
    }
};

//-----------------------------------------------------------------------------
// GMod Entity Configuration System - Loads entity definitions from settings/entities/%s.txt
// Based on IDA string analysis: settings/entities/%s.txt pattern
//-----------------------------------------------------------------------------
class CGModEntitySystem : public CAutoGameSystem
{
public:
    CGModEntitySystem() : CAutoGameSystem("GMod Entity System") {}

    // CAutoGameSystem overrides
    virtual bool Init() override;
    virtual void Shutdown() override;
    virtual void LevelInitPostEntity() override;

    // Entity configuration functions discovered from IDA
    static bool LoadEntityConfigs(); // Loads all entity configs from settings/entities/
    static bool LoadEntityConfig(const char* pszEntityName); // Load specific entity config
    static bool SaveEntityConfig(const char* pszEntityName, const EntityConfig_t& config);
    static EntityConfig_t* FindEntityConfig(const char* pszEntityName);
    static bool RegisterCustomEntity(const char* pszEntityName, const EntityConfig_t& config);

    // Entity creation with custom configurations
    static CBaseEntity* CreateConfiguredEntity(const char* pszEntityName, const Vector& origin, const QAngle& angles);
    static bool ApplyEntityConfig(CBaseEntity* pEntity, const EntityConfig_t& config);
    static bool ValidateEntityConfig(const EntityConfig_t& config);

    // Entity query functions
    static int GetEntityConfigCount();
    static void GetEntityConfigList(CUtlVector<CUtlString>& configList);
    static bool IsEntityConfigured(const char* pszEntityName);
    static bool IsEntityEnabled(const char* pszEntityName);

    // Utility functions
    static const char* GetEntityConfigPath(const char* pszEntityName);
    static const char* GetEntitiesDirectory() { return "settings/entities"; }

private:
    static CUtlDict<EntityConfig_t, int> s_EntityConfigs;
    static CUtlVector<EntityFactoryData_t> s_CustomEntities;
    static bool s_bSystemInitialized;
    static bool s_bConfigsLoaded;

    // Helper functions based on IDA analysis
    static bool ParseEntityConfig(const char* pszConfigPath, EntityConfig_t* pConfig);
    static bool ScanEntityDirectory();
    static void BuildEntityConfigPath(const char* pszEntityName, char* pszPath, int maxLen);
    static bool WriteEntityConfigFile(const char* pszConfigPath, const EntityConfig_t& config);
};

// Global instance
extern CGModEntitySystem g_GMod_EntitySystem;

// Console command handlers
void CMD_gmod_entity_list(void);
void CMD_gmod_entity_create(void);
void CMD_gmod_entity_config(void);
void CMD_gmod_entity_reload(void);

#endif // GMOD_ENTITIES_H