#include "cbase.h"
#include "gmod_physproperties.h"
#include "player.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "physics.h"
#include "vphysics/objects.h"
#include "engine/IEngineSound.h"
#include "te_effect_dispatch.h"
#include "util.h"

// ConCommand registration
static ConCommand gmod_physproperties_list("gmod_physproperties_list", CMD_gmod_physproperties_list, "List all physics properties");
static ConCommand gmod_physproperties_reload("gmod_physproperties_reload", CMD_gmod_physproperties_reload, "Reload physics properties");
static ConCommand gmod_physproperties_info("gmod_physproperties_info", CMD_gmod_physproperties_info, "Show material physics info");
static ConCommand gmod_physproperties_apply("gmod_physproperties_apply", CMD_gmod_physproperties_apply, "Apply physics properties to entity");

// Static member initialization
CUtlDict<PhysProperties_t, int> CGModPhysPropertiesSystem::s_PhysProperties;
CUtlDict<SurfaceData_t, int> CGModPhysPropertiesSystem::s_SurfaceData;
bool CGModPhysPropertiesSystem::s_bSystemInitialized = false;
bool CGModPhysPropertiesSystem::s_bPropertiesLoaded = false;

// Global instance
CGModPhysPropertiesSystem g_GMod_PhysPropertiesSystem;

//-----------------------------------------------------------------------------
// Purpose: Initialize the physics properties system
//-----------------------------------------------------------------------------
bool CGModPhysPropertiesSystem::Init()
{
    if (s_bSystemInitialized)
        return true;

    Msg("Initializing GMod Physics Properties System...\n");

    s_PhysProperties.Purge();
    s_SurfaceData.Purge();

    // Load physics properties from settings/gmod_physproperties.txt
    if (!LoadPhysProperties())
    {
        Warning("GError loading gmod_physproperties.txt..\n");
        // Continue with default properties
    }

    s_bSystemInitialized = true;
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown the physics properties system
//-----------------------------------------------------------------------------
void CGModPhysPropertiesSystem::Shutdown()
{
    if (!s_bSystemInitialized)
        return;

    s_PhysProperties.Purge();
    s_SurfaceData.Purge();
    s_bSystemInitialized = false;
    s_bPropertiesLoaded = false;
}

//-----------------------------------------------------------------------------
// Purpose: Level initialization
//-----------------------------------------------------------------------------
void CGModPhysPropertiesSystem::LevelInitPostEntity()
{
    DevMsg("PhysProperties: Level initialized with %d material properties\n", s_PhysProperties.Count());
}

//-----------------------------------------------------------------------------
// Purpose: Load physics properties from settings/gmod_physproperties.txt
//-----------------------------------------------------------------------------
bool CGModPhysPropertiesSystem::LoadPhysProperties()
{
    const char* pszConfigPath = GetPhysPropertiesPath();

    if (!filesystem->FileExists(pszConfigPath, "GAME"))
    {
        Warning("PhysProperties: Config file not found: %s\n", pszConfigPath);
        return false;
    }

    return ParsePhysPropertiesFile(pszConfigPath);
}

//-----------------------------------------------------------------------------
// Purpose: Parse physics properties file
//-----------------------------------------------------------------------------
bool CGModPhysPropertiesSystem::ParsePhysPropertiesFile(const char* pszFilePath)
{
    KeyValues* pKV = new KeyValues("PhysProperties");
    if (!pKV->LoadFromFile(filesystem, pszFilePath, "GAME"))
    {
        pKV->deleteThis();
        return false;
    }

    s_PhysProperties.Purge();
    s_SurfaceData.Purge();

    // Load default material properties
    ApplyDefaultProperties(NULL);

    int materialCount = 0;
    int surfaceCount = 0;

    // Parse materials section
    KeyValues* pMaterials = pKV->FindKey("materials");
    if (pMaterials)
    {
        FOR_EACH_SUBKEY(pMaterials, pMaterial)
        {
            PhysProperties_t properties;
            properties.materialName = pMaterial->GetName();

            if (ParseMaterialSection(pMaterial, &properties))
            {
                int index = s_PhysProperties.Insert(properties.materialName.Get(), properties);
                if (index != s_PhysProperties.InvalidIndex())
                {
                    materialCount++;
                    DevMsg("Loaded physics properties for: %s\n", properties.materialName.Get());
                }
            }
        }
    }

    // Parse surfaces section
    KeyValues* pSurfaces = pKV->FindKey("surfaces");
    if (pSurfaces)
    {
        FOR_EACH_SUBKEY(pSurfaces, pSurface)
        {
            SurfaceData_t surface;
            surface.surfaceName = pSurface->GetName();

            if (ParseSurfaceSection(pSurface, &surface))
            {
                int index = s_SurfaceData.Insert(surface.surfaceName.Get(), surface);
                if (index != s_SurfaceData.InvalidIndex())
                {
                    surfaceCount++;
                    DevMsg("Loaded surface data for: %s\n", surface.surfaceName.Get());
                }
            }
        }
    }

    pKV->deleteThis();
    s_bPropertiesLoaded = true;

    DevMsg("PhysProperties: Loaded %d materials and %d surfaces\n", materialCount, surfaceCount);
    return materialCount > 0;
}

//-----------------------------------------------------------------------------
// Purpose: Parse material section
//-----------------------------------------------------------------------------
bool CGModPhysPropertiesSystem::ParseMaterialSection(KeyValues* pKV, PhysProperties_t* pProperties)
{
    if (!pKV || !pProperties)
        return false;

    // Parse basic physics properties
    pProperties->flDensity = pKV->GetFloat("density", 1000.0f);
    pProperties->flFriction = pKV->GetFloat("friction", 0.7f);
    pProperties->flRestitution = pKV->GetFloat("restitution", 0.4f);
    pProperties->flDamping = pKV->GetFloat("damping", 0.1f);

    // Parse break properties
    pProperties->flBreakStress = pKV->GetFloat("breakstress", 0.0f);
    pProperties->flBreakStrain = pKV->GetFloat("breakstrain", 0.0f);
    pProperties->bBreakable = pKV->GetBool("breakable", false);

    // Parse sounds
    pProperties->impactSoundLight = pKV->GetString("impact_light", "");
    pProperties->impactSoundHeavy = pKV->GetString("impact_heavy", "");
    pProperties->breakSound = pKV->GetString("break_sound", "");
    pProperties->rollSound = pKV->GetString("roll_sound", "");

    // Parse effects
    pProperties->breakEffect = pKV->GetString("break_effect", "");
    pProperties->impactEffect = pKV->GetString("impact_effect", "");
    pProperties->scrapeEffect = pKV->GetString("scrape_effect", "");

    // Parse gameplay properties
    pProperties->flMassMult = pKV->GetFloat("mass_mult", 1.0f);
    pProperties->flHealthMult = pKV->GetFloat("health_mult", 1.0f);
    pProperties->bFloats = pKV->GetBool("floats", false);
    pProperties->bClimbable = pKV->GetBool("climbable", true);

    // Parse material type
    const char* pszType = pKV->GetString("type", "default");
    pProperties->materialType = ParseMaterialType(pszType);

    // Parse surface property
    pProperties->surfaceProperty = pKV->GetString("surface", "default");

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Parse surface section
//-----------------------------------------------------------------------------
bool CGModPhysPropertiesSystem::ParseSurfaceSection(KeyValues* pKV, SurfaceData_t* pSurface)
{
    if (!pKV || !pSurface)
        return false;

    pSurface->flHardness = pKV->GetFloat("hardness", 0.5f);
    pSurface->flRoughness = pKV->GetFloat("roughness", 0.5f);
    pSurface->bReflective = pKV->GetBool("reflective", false);

    // Parse color
    const char* pszColor = pKV->GetString("color", "255 255 255 255");
    int r, g, b, a;
    if (sscanf(pszColor, "%d %d %d %d", &r, &g, &b, &a) == 4)
    {
        pSurface->surfaceColor = Color(r, g, b, a);
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Parse material type
//-----------------------------------------------------------------------------
PhysMaterialType_t CGModPhysPropertiesSystem::ParseMaterialType(const char* pszTypeName)
{
    if (!pszTypeName)
        return PHYSMAT_DEFAULT;

    if (!Q_stricmp(pszTypeName, "metal"))
        return PHYSMAT_METAL;
    else if (!Q_stricmp(pszTypeName, "wood"))
        return PHYSMAT_WOOD;
    else if (!Q_stricmp(pszTypeName, "concrete"))
        return PHYSMAT_CONCRETE;
    else if (!Q_stricmp(pszTypeName, "glass"))
        return PHYSMAT_GLASS;
    else if (!Q_stricmp(pszTypeName, "flesh"))
        return PHYSMAT_FLESH;
    else if (!Q_stricmp(pszTypeName, "plastic"))
        return PHYSMAT_PLASTIC;
    else if (!Q_stricmp(pszTypeName, "rubber"))
        return PHYSMAT_RUBBER;
    else if (!Q_stricmp(pszTypeName, "ice"))
        return PHYSMAT_ICE;
    else if (!Q_stricmp(pszTypeName, "sand"))
        return PHYSMAT_SAND;
    else if (!Q_stricmp(pszTypeName, "water"))
        return PHYSMAT_WATER;

    return PHYSMAT_DEFAULT;
}

//-----------------------------------------------------------------------------
// Purpose: Apply default properties
//-----------------------------------------------------------------------------
void CGModPhysPropertiesSystem::ApplyDefaultProperties(PhysProperties_t* pProperties)
{
    // Create default material entries
    const char* defaultMaterials[] = {
        "default", "metal", "wood", "concrete", "glass", "flesh", "plastic", "rubber", "ice", "sand", "water"
    };

    for (int i = 0; i < ARRAYSIZE(defaultMaterials); i++)
    {
        PhysProperties_t defaultProps;
        defaultProps.materialName = defaultMaterials[i];
        defaultProps.materialType = (PhysMaterialType_t)i;

        // Set type-specific defaults
        switch (defaultProps.materialType)
        {
            case PHYSMAT_METAL:
                defaultProps.flDensity = 7800.0f;
                defaultProps.flFriction = 0.8f;
                defaultProps.flRestitution = 0.2f;
                defaultProps.impactSoundLight = "Metal.ImpactSoft";
                defaultProps.impactSoundHeavy = "Metal.ImpactHard";
                break;

            case PHYSMAT_WOOD:
                defaultProps.flDensity = 600.0f;
                defaultProps.flFriction = 0.7f;
                defaultProps.flRestitution = 0.3f;
                defaultProps.bBreakable = true;
                defaultProps.flBreakStress = 500.0f;
                break;

            case PHYSMAT_GLASS:
                defaultProps.flDensity = 2500.0f;
                defaultProps.flFriction = 0.6f;
                defaultProps.flRestitution = 0.1f;
                defaultProps.bBreakable = true;
                defaultProps.flBreakStress = 100.0f;
                break;

            case PHYSMAT_WATER:
                defaultProps.flDensity = 1000.0f;
                defaultProps.flFriction = 0.1f;
                defaultProps.flRestitution = 0.0f;
                defaultProps.bFloats = true;
                break;

            default:
                // Keep default values
                break;
        }

        s_PhysProperties.Insert(defaultProps.materialName.Get(), defaultProps);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Reload physics properties
//-----------------------------------------------------------------------------
bool CGModPhysPropertiesSystem::ReloadPhysProperties()
{
    s_PhysProperties.Purge();
    s_SurfaceData.Purge();
    s_bPropertiesLoaded = false;

    return LoadPhysProperties();
}

//-----------------------------------------------------------------------------
// Purpose: Find physics properties
//-----------------------------------------------------------------------------
PhysProperties_t* CGModPhysPropertiesSystem::FindPhysProperties(const char* pszMaterialName)
{
    if (!pszMaterialName || !s_bPropertiesLoaded)
        return NULL;

    int index = s_PhysProperties.Find(pszMaterialName);
    if (index != s_PhysProperties.InvalidIndex())
        return &s_PhysProperties[index];

    // Try default material
    index = s_PhysProperties.Find("default");
    if (index != s_PhysProperties.InvalidIndex())
        return &s_PhysProperties[index];

    return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Find physics properties for entity
//-----------------------------------------------------------------------------
PhysProperties_t* CGModPhysPropertiesSystem::FindPhysPropertiesForEntity(CBaseEntity* pEntity)
{
    if (!pEntity)
        return NULL;

    // Try to get material from model
    const char* pszModelName = STRING(pEntity->GetModelName());
    if (pszModelName && *pszModelName)
    {
        // Extract material from model path or use model name
        return FindPhysProperties("default");
    }

    return FindPhysProperties("default");
}

//-----------------------------------------------------------------------------
// Purpose: Find surface data
//-----------------------------------------------------------------------------
SurfaceData_t* CGModPhysPropertiesSystem::FindSurfaceData(const char* pszSurfaceName)
{
    if (!pszSurfaceName || !s_bPropertiesLoaded)
        return NULL;

    int index = s_SurfaceData.Find(pszSurfaceName);
    if (index != s_SurfaceData.InvalidIndex())
        return &s_SurfaceData[index];

    return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Apply physics properties
//-----------------------------------------------------------------------------
bool CGModPhysPropertiesSystem::ApplyPhysProperties(IPhysicsObject* pPhysicsObject, const char* pszMaterialName)
{
    if (!pPhysicsObject)
        return false;

    PhysProperties_t* pProperties = FindPhysProperties(pszMaterialName);
    if (!pProperties)
        return false;

    // Apply density (affects mass)
    float volume = pPhysicsObject->GetVolume();
    if (volume > 0.0f)
    {
        float mass = volume * pProperties->flDensity;
        pPhysicsObject->SetMass(mass);
    }

    // Apply material properties
    physicssurfaceprops->SetWorldMaterialIndexTable(0); // Reset material table

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Apply physics properties to entity
//-----------------------------------------------------------------------------
bool CGModPhysPropertiesSystem::ApplyPhysPropertiesToEntity(CBaseEntity* pEntity, const char* pszMaterialName)
{
    if (!pEntity)
        return false;

    PhysProperties_t* pProperties = FindPhysProperties(pszMaterialName);
    if (!pProperties)
        return false;

    // Apply to physics object if it exists
    IPhysicsObject* pPhysics = pEntity->GetPhysicsObject();
    if (pPhysics)
    {
        ApplyPhysProperties(pPhysics, pszMaterialName);
    }

    // Apply health multiplier
    if (pProperties->flHealthMult != 1.0f)
    {
        float newHealth = pEntity->GetHealth() * pProperties->flHealthMult;
        pEntity->SetHealth(newHealth);
        pEntity->SetMaxHealth(newHealth);
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Update entity physics properties
//-----------------------------------------------------------------------------
void CGModPhysPropertiesSystem::UpdateEntityPhysProperties(CBaseEntity* pEntity)
{
    if (!pEntity)
        return;

    PhysProperties_t* pProperties = FindPhysPropertiesForEntity(pEntity);
    if (pProperties)
    {
        ApplyPhysPropertiesToEntity(pEntity, pProperties->materialName.Get());
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle impact event
//-----------------------------------------------------------------------------
void CGModPhysPropertiesSystem::OnImpact(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const Vector& impactPos, float impactForce)
{
    if (!pEntity1)
        return;

    PhysProperties_t* pProperties = FindPhysPropertiesForEntity(pEntity1);
    if (!pProperties)
        return;

    // Play impact sound
    const char* pszSound = (impactForce > 500.0f) ?
        pProperties->impactSoundHeavy.Get() :
        pProperties->impactSoundLight.Get();

    if (pszSound && *pszSound)
    {
        pEntity1->EmitSound(pszSound);
    }

    // Create impact effect
    if (pProperties->impactEffect.Length() > 0)
    {
        CEffectData data;
        data.m_vOrigin = impactPos;
        DispatchEffect(pProperties->impactEffect.Get(), data);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle break event
//-----------------------------------------------------------------------------
void CGModPhysPropertiesSystem::OnBreak(CBaseEntity* pEntity, const Vector& breakPos)
{
    if (!pEntity)
        return;

    PhysProperties_t* pProperties = FindPhysPropertiesForEntity(pEntity);
    if (!pProperties)
        return;

    // Play break sound
    if (pProperties->breakSound.Length() > 0)
    {
        pEntity->EmitSound(pProperties->breakSound.Get());
    }

    // Create break effect
    if (pProperties->breakEffect.Length() > 0)
    {
        CEffectData data;
        data.m_vOrigin = breakPos;
        DispatchEffect(pProperties->breakEffect.Get(), data);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle scrape event
//-----------------------------------------------------------------------------
void CGModPhysPropertiesSystem::OnScrape(CBaseEntity* pEntity, const Vector& scrapePos, const Vector& scrapeDir)
{
    if (!pEntity)
        return;

    PhysProperties_t* pProperties = FindPhysPropertiesForEntity(pEntity);
    if (!pProperties)
        return;

    // Create scrape effect
    if (pProperties->scrapeEffect.Length() > 0)
    {
        CEffectData data;
        data.m_vOrigin = scrapePos;
        data.m_vNormal = scrapeDir;
        DispatchEffect(pProperties->scrapeEffect.Get(), data);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Calculate mass
//-----------------------------------------------------------------------------
float CGModPhysPropertiesSystem::CalculateMass(const char* pszMaterialName, float volume)
{
    PhysProperties_t* pProperties = FindPhysProperties(pszMaterialName);
    if (!pProperties || volume <= 0.0f)
        return 1.0f;

    return volume * pProperties->flDensity * pProperties->flMassMult;
}

//-----------------------------------------------------------------------------
// Purpose: Calculate health
//-----------------------------------------------------------------------------
float CGModPhysPropertiesSystem::CalculateHealth(const char* pszMaterialName, float baseHealth)
{
    PhysProperties_t* pProperties = FindPhysProperties(pszMaterialName);
    if (!pProperties)
        return baseHealth;

    return baseHealth * pProperties->flHealthMult;
}

//-----------------------------------------------------------------------------
// Purpose: Check if material should float
//-----------------------------------------------------------------------------
bool CGModPhysPropertiesSystem::ShouldFloat(const char* pszMaterialName)
{
    PhysProperties_t* pProperties = FindPhysProperties(pszMaterialName);
    return pProperties ? pProperties->bFloats : false;
}

//-----------------------------------------------------------------------------
// Purpose: Get material density
//-----------------------------------------------------------------------------
float CGModPhysPropertiesSystem::GetMaterialDensity(const char* pszMaterialName)
{
    PhysProperties_t* pProperties = FindPhysProperties(pszMaterialName);
    return pProperties ? pProperties->flDensity : 1000.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Get physics properties count
//-----------------------------------------------------------------------------
int CGModPhysPropertiesSystem::GetPhysPropertiesCount()
{
    return s_PhysProperties.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Get material list
//-----------------------------------------------------------------------------
void CGModPhysPropertiesSystem::GetMaterialList(CUtlVector<CUtlString>& materialList)
{
    materialList.Purge();

    for (int i = s_PhysProperties.First(); i != s_PhysProperties.InvalidIndex(); i = s_PhysProperties.Next(i))
    {
        materialList.AddToTail(s_PhysProperties.GetElementName(i));
    }
}

//-----------------------------------------------------------------------------
// Purpose: Check if material is registered
//-----------------------------------------------------------------------------
bool CGModPhysPropertiesSystem::IsMaterialRegistered(const char* pszMaterialName)
{
    return FindPhysProperties(pszMaterialName) != NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Get material type
//-----------------------------------------------------------------------------
PhysMaterialType_t CGModPhysPropertiesSystem::GetMaterialType(const char* pszMaterialName)
{
    PhysProperties_t* pProperties = FindPhysProperties(pszMaterialName);
    return pProperties ? pProperties->materialType : PHYSMAT_DEFAULT;
}

//-----------------------------------------------------------------------------
// Purpose: Get material type name
//-----------------------------------------------------------------------------
const char* CGModPhysPropertiesSystem::GetMaterialTypeName(PhysMaterialType_t type)
{
    switch (type)
    {
        case PHYSMAT_DEFAULT:   return "default";
        case PHYSMAT_METAL:     return "metal";
        case PHYSMAT_WOOD:      return "wood";
        case PHYSMAT_CONCRETE:  return "concrete";
        case PHYSMAT_GLASS:     return "glass";
        case PHYSMAT_FLESH:     return "flesh";
        case PHYSMAT_PLASTIC:   return "plastic";
        case PHYSMAT_RUBBER:    return "rubber";
        case PHYSMAT_ICE:       return "ice";
        case PHYSMAT_SAND:      return "sand";
        case PHYSMAT_WATER:     return "water";
        default:                return "unknown";
    }
}

//=============================================================================
// Console Commands
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: List all physics properties
//-----------------------------------------------------------------------------
void CMD_gmod_physproperties_list(void)
{
    CUtlVector<CUtlString> materialList;
    CGModPhysPropertiesSystem::GetMaterialList(materialList);

    Msg("Physics properties (%d materials):\n", materialList.Count());
    for (int i = 0; i < materialList.Count(); i++)
    {
        PhysProperties_t* pProperties = CGModPhysPropertiesSystem::FindPhysProperties(materialList[i].Get());
        if (pProperties)
        {
            Msg("  %-15s - Density: %.1f, Type: %s\n",
                materialList[i].Get(),
                pProperties->flDensity,
                CGModPhysPropertiesSystem::GetMaterialTypeName(pProperties->materialType));
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Reload physics properties
//-----------------------------------------------------------------------------
void CMD_gmod_physproperties_reload(void)
{
    if (CGModPhysPropertiesSystem::ReloadPhysProperties())
    {
        Msg("Physics properties reloaded successfully\n");
    }
    else
    {
        Msg("Failed to reload physics properties\n");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Show material physics info
//-----------------------------------------------------------------------------
void CMD_gmod_physproperties_info(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_physproperties_info <material_name>\n");
        return;
    }

    const char* pszMaterialName = engine->Cmd_Argv(1);
    PhysProperties_t* pProperties = CGModPhysPropertiesSystem::FindPhysProperties(pszMaterialName);

    if (!pProperties)
    {
        Msg("Material not found: %s\n", pszMaterialName);
        return;
    }

    Msg("Physics properties for '%s':\n", pszMaterialName);
    Msg("  Type: %s\n", CGModPhysPropertiesSystem::GetMaterialTypeName(pProperties->materialType));
    Msg("  Density: %.2f kg/m³\n", pProperties->flDensity);
    Msg("  Friction: %.2f\n", pProperties->flFriction);
    Msg("  Restitution: %.2f\n", pProperties->flRestitution);
    Msg("  Damping: %.2f\n", pProperties->flDamping);
    Msg("  Breakable: %s\n", pProperties->bBreakable ? "yes" : "no");
    if (pProperties->bBreakable)
    {
        Msg("  Break Stress: %.2f\n", pProperties->flBreakStress);
    }
    Msg("  Floats: %s\n", pProperties->bFloats ? "yes" : "no");
    Msg("  Mass Multiplier: %.2f\n", pProperties->flMassMult);
    Msg("  Health Multiplier: %.2f\n", pProperties->flHealthMult);
}

//-----------------------------------------------------------------------------
// Purpose: Apply physics properties to entity
//-----------------------------------------------------------------------------
void CMD_gmod_physproperties_apply(void)
{
    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_physproperties_apply <material_name>\n");
        return;
    }

    const char* pszMaterialName = engine->Cmd_Argv(1);

    // Get entity player is looking at
    trace_t tr;
    Vector forward;
    pPlayer->EyeVectors(&forward);
    UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + forward * 1000.0f, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

    if (!tr.DidHit() || !tr.m_pEnt)
    {
        Msg("No entity found\n");
        return;
    }

    if (CGModPhysPropertiesSystem::ApplyPhysPropertiesToEntity(tr.m_pEnt, pszMaterialName))
    {
        Msg("Applied physics properties '%s' to entity\n", pszMaterialName);
    }
    else
    {
        Msg("Failed to apply physics properties '%s'\n", pszMaterialName);
    }
}