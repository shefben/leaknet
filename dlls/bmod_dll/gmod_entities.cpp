#include "cbase.h"
#include "gmod_entities.h"
#include "player.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "entityfactory.h"
#include "physics.h"
#include "util.h"

// ConCommand registration
static ConCommand gmod_entity_list("gmod_entity_list", CMD_gmod_entity_list, "List all configured entities");
static ConCommand gmod_entity_create("gmod_entity_create", CMD_gmod_entity_create, "Create configured entity");
static ConCommand gmod_entity_config("gmod_entity_config", CMD_gmod_entity_config, "Show entity configuration");
static ConCommand gmod_entity_reload("gmod_entity_reload", CMD_gmod_entity_reload, "Reload entity configurations");

// Static member initialization
CUtlDict<EntityConfig_t, int> CGModEntitySystem::s_EntityConfigs;
CUtlVector<EntityFactoryData_t> CGModEntitySystem::s_CustomEntities;
bool CGModEntitySystem::s_bSystemInitialized = false;
bool CGModEntitySystem::s_bConfigsLoaded = false;

// Global instance
CGModEntitySystem g_GMod_EntitySystem;

//-----------------------------------------------------------------------------
// Purpose: Initialize the entity system
//-----------------------------------------------------------------------------
bool CGModEntitySystem::Init()
{
    if (s_bSystemInitialized)
        return true;

    Msg("Initializing GMod Entity System...\n");

    s_EntityConfigs.Purge();
    s_CustomEntities.Purge();

    // Load entity configurations from settings/entities/
    if (!LoadEntityConfigs())
    {
        Warning("GMod Entity System: Failed to load entity configurations\n");
    }

    s_bSystemInitialized = true;
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown the entity system
//-----------------------------------------------------------------------------
void CGModEntitySystem::Shutdown()
{
    if (!s_bSystemInitialized)
        return;

    s_EntityConfigs.Purge();
    s_CustomEntities.Purge();
    s_bSystemInitialized = false;
    s_bConfigsLoaded = false;
}

//-----------------------------------------------------------------------------
// Purpose: Level initialization
//-----------------------------------------------------------------------------
void CGModEntitySystem::LevelInitPostEntity()
{
    // Entity configurations persist across levels
    DevMsg("GMod Entity System: Level initialized with %d entity configs\n", s_EntityConfigs.Count());
}

//-----------------------------------------------------------------------------
// Purpose: Load all entity configurations from settings/entities/
//-----------------------------------------------------------------------------
bool CGModEntitySystem::LoadEntityConfigs()
{
    const char* pszEntitiesDir = GetEntitiesDirectory();

    if (!filesystem->IsDirectory(pszEntitiesDir, "GAME"))
    {
        // Create directory if it doesn't exist
        filesystem->CreateDirHierarchy(pszEntitiesDir, "GAME");
        DevMsg("Created entities directory: %s\n", pszEntitiesDir);
    }

    return ScanEntityDirectory();
}

//-----------------------------------------------------------------------------
// Purpose: Scan entity directory for configuration files
//-----------------------------------------------------------------------------
bool CGModEntitySystem::ScanEntityDirectory()
{
    const char* pszSearchPath = "settings/entities/*.txt";
    FileFindHandle_t findHandle;

    const char* pszFileName = filesystem->FindFirstEx(pszSearchPath, "GAME", &findHandle);
    int configCount = 0;

    while (pszFileName)
    {
        if (!filesystem->FindIsDirectory(findHandle))
        {
            // Extract entity name from filename (remove .txt extension)
            char entityName[256];
            Q_strncpy(entityName, pszFileName, sizeof(entityName));
            char* pExtension = Q_strstr(entityName, ".txt");
            if (pExtension)
                *pExtension = '\0';

            if (LoadEntityConfig(entityName))
            {
                configCount++;
                DevMsg("Loaded entity config: %s\n", entityName);
            }
        }

        pszFileName = filesystem->FindNext(findHandle);
    }

    filesystem->FindClose(findHandle);

    s_bConfigsLoaded = true;
    DevMsg("GMod Entity System: Loaded %d entity configurations\n", configCount);
    return configCount > 0;
}

//-----------------------------------------------------------------------------
// Purpose: Load specific entity configuration
//-----------------------------------------------------------------------------
bool CGModEntitySystem::LoadEntityConfig(const char* pszEntityName)
{
    if (!pszEntityName || !*pszEntityName)
        return false;

    char configPath[512];
    BuildEntityConfigPath(pszEntityName, configPath, sizeof(configPath));

    if (!filesystem->FileExists(configPath, "GAME"))
    {
        Warning("Entity config not found: %s\n", configPath);
        return false;
    }

    EntityConfig_t config;
    if (!ParseEntityConfig(configPath, &config))
    {
        Warning("Failed to parse entity config: %s\n", configPath);
        return false;
    }

    // Store configuration
    config.entityName = pszEntityName;
    int index = s_EntityConfigs.Insert(pszEntityName, config);

    return index != s_EntityConfigs.InvalidIndex();
}

//-----------------------------------------------------------------------------
// Purpose: Parse entity configuration file
//-----------------------------------------------------------------------------
bool CGModEntitySystem::ParseEntityConfig(const char* pszConfigPath, EntityConfig_t* pConfig)
{
    if (!pConfig)
        return false;

    KeyValues* pKV = new KeyValues("EntityConfig");
    if (!pKV->LoadFromFile(filesystem, pszConfigPath, "GAME"))
    {
        pKV->deleteThis();
        return false;
    }

    // Parse basic properties
    pConfig->className = pKV->GetString("classname", "prop_physics");
    pConfig->displayName = pKV->GetString("displayname", "");
    pConfig->description = pKV->GetString("description", "");
    pConfig->model = pKV->GetString("model", "");
    pConfig->material = pKV->GetString("material", "");
    pConfig->spawnSound = pKV->GetString("spawnsound", "");

    // Parse physics properties
    KeyValues* pPhysics = pKV->FindKey("physics");
    if (pPhysics)
    {
        pConfig->flMass = pPhysics->GetFloat("mass", 1.0f);
        pConfig->flHealth = pPhysics->GetFloat("health", 100.0f);
        pConfig->bBreakable = pPhysics->GetBool("breakable", true);
        pConfig->bSolid = pPhysics->GetBool("solid", true);
        pConfig->bStatic = pPhysics->GetBool("static", false);
    }

    // Parse appearance properties
    KeyValues* pAppearance = pKV->FindKey("appearance");
    if (pAppearance)
    {
        const char* pszColor = pAppearance->GetString("color", "1.0 1.0 1.0");
        sscanf(pszColor, "%f %f %f", &pConfig->vecColor.x, &pConfig->vecColor.y, &pConfig->vecColor.z);
        pConfig->flScale = pAppearance->GetFloat("scale", 1.0f);
        pConfig->iSkin = pAppearance->GetInt("skin", 0);
    }

    // Parse behavior properties
    KeyValues* pBehavior = pKV->FindKey("behavior");
    if (pBehavior)
    {
        pConfig->bCanPick = pBehavior->GetBool("canpick", true);
        pConfig->bCanFreeze = pBehavior->GetBool("canfreeze", true);
        pConfig->bCanWeld = pBehavior->GetBool("canweld", true);
        pConfig->bIsEnabled = pBehavior->GetBool("enabled", true);
    }

    pKV->deleteThis();
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Save entity configuration
//-----------------------------------------------------------------------------
bool CGModEntitySystem::SaveEntityConfig(const char* pszEntityName, const EntityConfig_t& config)
{
    if (!pszEntityName || !*pszEntityName)
        return false;

    char configPath[512];
    BuildEntityConfigPath(pszEntityName, configPath, sizeof(configPath));

    return WriteEntityConfigFile(configPath, config);
}

//-----------------------------------------------------------------------------
// Purpose: Write entity configuration file
//-----------------------------------------------------------------------------
bool CGModEntitySystem::WriteEntityConfigFile(const char* pszConfigPath, const EntityConfig_t& config)
{
    KeyValues* pKV = new KeyValues("EntityConfig");

    // Write basic properties
    pKV->SetString("classname", config.className.Get());
    pKV->SetString("displayname", config.displayName.Get());
    pKV->SetString("description", config.description.Get());
    pKV->SetString("model", config.model.Get());
    pKV->SetString("material", config.material.Get());
    pKV->SetString("spawnsound", config.spawnSound.Get());

    // Write physics properties
    KeyValues* pPhysics = pKV->CreateNewKey();
    pPhysics->SetName("physics");
    pPhysics->SetFloat("mass", config.flMass);
    pPhysics->SetFloat("health", config.flHealth);
    pPhysics->SetBool("breakable", config.bBreakable);
    pPhysics->SetBool("solid", config.bSolid);
    pPhysics->SetBool("static", config.bStatic);

    // Write appearance properties
    KeyValues* pAppearance = pKV->CreateNewKey();
    pAppearance->SetName("appearance");
    char colorStr[64];
    Q_snprintf(colorStr, sizeof(colorStr), "%.2f %.2f %.2f", config.vecColor.x, config.vecColor.y, config.vecColor.z);
    pAppearance->SetString("color", colorStr);
    pAppearance->SetFloat("scale", config.flScale);
    pAppearance->SetInt("skin", config.iSkin);

    // Write behavior properties
    KeyValues* pBehavior = pKV->CreateNewKey();
    pBehavior->SetName("behavior");
    pBehavior->SetBool("canpick", config.bCanPick);
    pBehavior->SetBool("canfreeze", config.bCanFreeze);
    pBehavior->SetBool("canweld", config.bCanWeld);
    pBehavior->SetBool("enabled", config.bIsEnabled);

    bool success = pKV->SaveToFile(filesystem, pszConfigPath, "GAME");
    pKV->deleteThis();

    return success;
}

//-----------------------------------------------------------------------------
// Purpose: Find entity configuration
//-----------------------------------------------------------------------------
EntityConfig_t* CGModEntitySystem::FindEntityConfig(const char* pszEntityName)
{
    if (!pszEntityName || !s_bConfigsLoaded)
        return NULL;

    int index = s_EntityConfigs.Find(pszEntityName);
    if (index != s_EntityConfigs.InvalidIndex())
        return &s_EntityConfigs[index];

    return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Register custom entity
//-----------------------------------------------------------------------------
bool CGModEntitySystem::RegisterCustomEntity(const char* pszEntityName, const EntityConfig_t& config)
{
    if (!pszEntityName || !*pszEntityName)
        return false;

    EntityFactoryData_t factoryData;
    factoryData.factoryName = pszEntityName;
    factoryData.baseClass = config.className;
    factoryData.config = config;
    factoryData.bRegistered = true;

    s_CustomEntities.AddToTail(factoryData);

    DevMsg("Registered custom entity: %s (base: %s)\n", pszEntityName, config.className.Get());
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Create configured entity
//-----------------------------------------------------------------------------
CBaseEntity* CGModEntitySystem::CreateConfiguredEntity(const char* pszEntityName, const Vector& origin, const QAngle& angles)
{
    EntityConfig_t* pConfig = FindEntityConfig(pszEntityName);
    if (!pConfig || !pConfig->bIsEnabled)
        return NULL;

    // Create base entity
    CBaseEntity* pEntity = CreateEntityByName(pConfig->className.Get());
    if (!pEntity)
        return NULL;

    // Set position
    pEntity->SetAbsOrigin(origin);
    pEntity->SetAbsAngles(angles);

    // Apply configuration
    if (!ApplyEntityConfig(pEntity, *pConfig))
    {
        UTIL_Remove(pEntity);
        return NULL;
    }

    // Spawn the entity
    DispatchSpawn(pEntity);

    return pEntity;
}

//-----------------------------------------------------------------------------
// Purpose: Apply entity configuration
//-----------------------------------------------------------------------------
bool CGModEntitySystem::ApplyEntityConfig(CBaseEntity* pEntity, const EntityConfig_t& config)
{
    if (!pEntity)
        return false;

    // Set model if specified
    if (config.model.Length() > 0)
    {
        pEntity->SetModel(config.model.Get());
    }

    // Set health
    if (config.flHealth > 0)
    {
        pEntity->SetHealth(config.flHealth);
        pEntity->SetMaxHealth(config.flHealth);
    }

    // Set appearance properties
    if (config.flScale != 1.0f)
    {
        pEntity->SetModelScale(config.flScale);
    }

    if (config.iSkin != 0)
    {
        pEntity->m_nSkin = config.iSkin;
    }

    // Set physics properties if it's a physics object
    IPhysicsObject* pPhysics = pEntity->GetPhysicsObject();
    if (pPhysics && config.flMass > 0)
    {
        pPhysics->SetMass(config.flMass);

        if (config.bStatic)
        {
            pPhysics->EnableMotion(false);
        }
    }

    // Play spawn sound
    if (config.spawnSound.Length() > 0)
    {
        pEntity->EmitSound(config.spawnSound.Get());
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Validate entity configuration
//-----------------------------------------------------------------------------
bool CGModEntitySystem::ValidateEntityConfig(const EntityConfig_t& config)
{
    // Basic validation
    if (config.className.Length() == 0)
        return false;

    if (config.flMass <= 0.0f)
        return false;

    if (config.flScale <= 0.0f)
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Get entity configuration count
//-----------------------------------------------------------------------------
int CGModEntitySystem::GetEntityConfigCount()
{
    return s_EntityConfigs.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Get entity configuration list
//-----------------------------------------------------------------------------
void CGModEntitySystem::GetEntityConfigList(CUtlVector<CUtlString>& configList)
{
    configList.Purge();

    for (int i = s_EntityConfigs.First(); i != s_EntityConfigs.InvalidIndex(); i = s_EntityConfigs.Next(i))
    {
        configList.AddToTail(s_EntityConfigs.GetElementName(i));
    }
}

//-----------------------------------------------------------------------------
// Purpose: Check if entity is configured
//-----------------------------------------------------------------------------
bool CGModEntitySystem::IsEntityConfigured(const char* pszEntityName)
{
    return FindEntityConfig(pszEntityName) != NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Check if entity is enabled
//-----------------------------------------------------------------------------
bool CGModEntitySystem::IsEntityEnabled(const char* pszEntityName)
{
    EntityConfig_t* pConfig = FindEntityConfig(pszEntityName);
    return pConfig && pConfig->bIsEnabled;
}

//-----------------------------------------------------------------------------
// Purpose: Get entity config path
//-----------------------------------------------------------------------------
const char* CGModEntitySystem::GetEntityConfigPath(const char* pszEntityName)
{
    static char configPath[512];
    BuildEntityConfigPath(pszEntityName, configPath, sizeof(configPath));
    return configPath;
}

//-----------------------------------------------------------------------------
// Purpose: Build entity config path
//-----------------------------------------------------------------------------
void CGModEntitySystem::BuildEntityConfigPath(const char* pszEntityName, char* pszPath, int maxLen)
{
    Q_snprintf(pszPath, maxLen, "settings/entities/%s.txt", pszEntityName);
}

//=============================================================================
// Console Commands
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: List all configured entities
//-----------------------------------------------------------------------------
void CMD_gmod_entity_list(void)
{
    CUtlVector<CUtlString> configList;
    CGModEntitySystem::GetEntityConfigList(configList);

    Msg("Configured entities (%d):\n", configList.Count());
    for (int i = 0; i < configList.Count(); i++)
    {
        EntityConfig_t* pConfig = CGModEntitySystem::FindEntityConfig(configList[i].Get());
        if (pConfig)
        {
            Msg("  %s (%s) - %s\n",
                configList[i].Get(),
                pConfig->className.Get(),
                pConfig->bIsEnabled ? "enabled" : "disabled");
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Create configured entity
//-----------------------------------------------------------------------------
void CMD_gmod_entity_create(void)
{
    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_entity_create <entity_name>\n");
        return;
    }

    const char* pszEntityName = engine->Cmd_Argv(1);

    // Get player's aim position
    Vector forward;
    pPlayer->EyeVectors(&forward);
    Vector origin = pPlayer->EyePosition() + forward * 100.0f;
    QAngle angles = vec3_angle;

    CBaseEntity* pEntity = CGModEntitySystem::CreateConfiguredEntity(pszEntityName, origin, angles);
    if (pEntity)
    {
        Msg("Created entity: %s\n", pszEntityName);
    }
    else
    {
        Msg("Failed to create entity: %s\n", pszEntityName);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Show entity configuration
//-----------------------------------------------------------------------------
void CMD_gmod_entity_config(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_entity_config <entity_name>\n");
        return;
    }

    const char* pszEntityName = engine->Cmd_Argv(1);
    EntityConfig_t* pConfig = CGModEntitySystem::FindEntityConfig(pszEntityName);

    if (!pConfig)
    {
        Msg("Entity config not found: %s\n", pszEntityName);
        return;
    }

    Msg("Entity configuration for '%s':\n", pszEntityName);
    Msg("  Class: %s\n", pConfig->className.Get());
    Msg("  Display Name: %s\n", pConfig->displayName.Get());
    Msg("  Model: %s\n", pConfig->model.Get());
    Msg("  Mass: %.2f\n", pConfig->flMass);
    Msg("  Health: %.2f\n", pConfig->flHealth);
    Msg("  Scale: %.2f\n", pConfig->flScale);
    Msg("  Enabled: %s\n", pConfig->bIsEnabled ? "yes" : "no");
}

//-----------------------------------------------------------------------------
// Purpose: Reload entity configurations
//-----------------------------------------------------------------------------
void CMD_gmod_entity_reload(void)
{
    CGModEntitySystem::s_EntityConfigs.Purge();
    CGModEntitySystem::s_bConfigsLoaded = false;

    if (CGModEntitySystem::LoadEntityConfigs())
    {
        Msg("Entity configurations reloaded successfully\n");
    }
    else
    {
        Msg("Failed to reload entity configurations\n");
    }
}