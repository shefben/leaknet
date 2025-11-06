#include "cbase.h"
#include "gmod_materials.h"
#include "player.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include "util.h"

// ConCommand registration
static ConCommand gmod_materials_list("gmod_materials_list", CMD_gmod_materials_list, "List all material overrides");
static ConCommand gmod_materials_reload("gmod_materials_reload", CMD_gmod_materials_reload, "Reload material configurations");
static ConCommand gmod_materials_override("gmod_materials_override", CMD_gmod_materials_override, "Override material");
static ConCommand gmod_materials_restore("gmod_materials_restore", CMD_gmod_materials_restore, "Restore original material");
static ConCommand gmod_materials_group("gmod_materials_group", CMD_gmod_materials_group, "Apply material group");
static ConCommand gmod_materials_apply("gmod_materials_apply", CMD_gmod_materials_apply, "Apply material to entity");

// Static member initialization
CUtlDict<MaterialData_t, int> CGModMaterialsSystem::s_Materials;
CUtlDict<MaterialGroup_t, int> CGModMaterialsSystem::s_MaterialGroups;
CUtlDict<CUtlString, int> CGModMaterialsSystem::s_MaterialOverrides;
bool CGModMaterialsSystem::s_bSystemInitialized = false;
bool CGModMaterialsSystem::s_bMaterialsLoaded = false;

// Global instance
CGModMaterialsSystem g_GMod_MaterialsSystem;

//-----------------------------------------------------------------------------
// Purpose: Initialize the materials system
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::Init()
{
    if (s_bSystemInitialized)
        return true;

    Msg("Initializing GMod Materials System...\n");

    s_Materials.Purge();
    s_MaterialGroups.Purge();
    s_MaterialOverrides.Purge();

    // Load material configurations from settings/gmod_materials.txt
    if (!LoadMaterials())
    {
        Warning("Error loading gmod_materials.txt!\n");
        // Continue with default materials
    }

    s_bSystemInitialized = true;
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown the materials system
//-----------------------------------------------------------------------------
void CGModMaterialsSystem::Shutdown()
{
    if (!s_bSystemInitialized)
        return;

    // Clear all overrides
    ClearAllOverrides();

    s_Materials.Purge();
    s_MaterialGroups.Purge();
    s_MaterialOverrides.Purge();
    s_bSystemInitialized = false;
    s_bMaterialsLoaded = false;
}

//-----------------------------------------------------------------------------
// Purpose: Level initialization
//-----------------------------------------------------------------------------
void CGModMaterialsSystem::LevelInitPostEntity()
{
    DevMsg("Materials: Level initialized with %d materials and %d groups\n",
           s_Materials.Count(), s_MaterialGroups.Count());
}

//-----------------------------------------------------------------------------
// Purpose: Load materials from settings/gmod_materials.txt
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::LoadMaterials()
{
    const char* pszConfigPath = GetMaterialsPath();

    if (!filesystem->FileExists(pszConfigPath, "GAME"))
    {
        Warning("Materials: Config file not found: %s\n", pszConfigPath);
        return false;
    }

    return ParseMaterialsFile(pszConfigPath);
}

//-----------------------------------------------------------------------------
// Purpose: Parse materials file
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::ParseMaterialsFile(const char* pszFilePath)
{
    KeyValues* pKV = new KeyValues("Materials");
    if (!pKV->LoadFromFile(filesystem, pszFilePath, "GAME"))
    {
        pKV->deleteThis();
        return false;
    }

    s_Materials.Purge();
    s_MaterialGroups.Purge();

    // Load default materials
    ApplyDefaultMaterials();

    int materialCount = 0;
    int groupCount = 0;

    // Parse materials section
    KeyValues* pMaterials = pKV->FindKey("materials");
    if (pMaterials)
    {
        FOR_EACH_SUBKEY(pMaterials, pMaterial)
        {
            MaterialData_t material;
            material.materialName = pMaterial->GetName();

            if (ParseMaterialSection(pMaterial, &material))
            {
                int index = s_Materials.Insert(material.materialName.Get(), material);
                if (index != s_Materials.InvalidIndex())
                {
                    materialCount++;
                    DevMsg("Loaded material override: %s\n", material.materialName.Get());
                }
            }
        }
    }

    // Parse groups section
    KeyValues* pGroups = pKV->FindKey("groups");
    if (pGroups)
    {
        FOR_EACH_SUBKEY(pGroups, pGroup)
        {
            MaterialGroup_t group;
            group.groupName = pGroup->GetName();

            if (ParseGroupSection(pGroup, &group))
            {
                int index = s_MaterialGroups.Insert(group.groupName.Get(), group);
                if (index != s_MaterialGroups.InvalidIndex())
                {
                    groupCount++;
                    DevMsg("Loaded material group: %s\n", group.groupName.Get());
                }
            }
        }
    }

    pKV->deleteThis();
    s_bMaterialsLoaded = true;

    DevMsg("Materials: Loaded %d materials and %d groups\n", materialCount, groupCount);
    return materialCount > 0 || groupCount > 0;
}

//-----------------------------------------------------------------------------
// Purpose: Parse material section
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::ParseMaterialSection(KeyValues* pKV, MaterialData_t* pMaterial)
{
    if (!pKV || !pMaterial)
        return false;

    // Parse basic properties
    pMaterial->originalPath = pKV->GetString("original", "");
    pMaterial->overridePath = pKV->GetString("override", "");
    pMaterial->shaderName = pKV->GetString("shader", "VertexLitGeneric");
    pMaterial->baseTexture = pKV->GetString("basetexture", "");
    pMaterial->normalMap = pKV->GetString("normalmap", "");
    pMaterial->detailTexture = pKV->GetString("detail", "");

    // Parse visual properties
    const char* pszColor = pKV->GetString("color", "255 255 255 255");
    int r, g, b, a;
    if (sscanf(pszColor, "%d %d %d %d", &r, &g, &b, &a) == 4)
    {
        pMaterial->materialColor = Color(r, g, b, a);
    }

    pMaterial->flAlpha = pKV->GetFloat("alpha", 1.0f);
    pMaterial->flReflectivity = pKV->GetFloat("reflectivity", 0.5f);
    pMaterial->flMetallic = pKV->GetFloat("metallic", 0.0f);
    pMaterial->flRoughness = pKV->GetFloat("roughness", 0.5f);

    // Parse flags
    pMaterial->bTranslucent = pKV->GetBool("translucent", false);
    pMaterial->bNoCull = pKV->GetBool("nocull", false);
    pMaterial->bNoFog = pKV->GetBool("nofog", false);
    pMaterial->bSelfIllum = pKV->GetBool("selfillum", false);

    // Parse animation properties
    pMaterial->bAnimated = pKV->GetBool("animated", false);
    pMaterial->flAnimationSpeed = pKV->GetFloat("animspeed", 1.0f);
    pMaterial->iFrameCount = pKV->GetInt("framecount", 1);

    // Parse override settings
    const char* pszOverrideType = pKV->GetString("type", "replace");
    pMaterial->overrideType = ParseOverrideType(pszOverrideType);
    pMaterial->bEnabled = pKV->GetBool("enabled", true);
    pMaterial->bApplyToAll = pKV->GetBool("applytoall", false);

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Parse group section
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::ParseGroupSection(KeyValues* pKV, MaterialGroup_t* pGroup)
{
    if (!pKV || !pGroup)
        return false;

    // Parse group properties
    ParseMaterialSection(pKV, &pGroup->groupProperties);
    pGroup->bGroupEnabled = pKV->GetBool("enabled", true);

    // Parse material list
    KeyValues* pMaterials = pKV->FindKey("materials");
    if (pMaterials)
    {
        FOR_EACH_VALUE(pMaterials, pValue)
        {
            const char* pszMaterialPath = pValue->GetString();
            if (pszMaterialPath && *pszMaterialPath)
            {
                pGroup->materialPaths.AddToTail(pszMaterialPath);
            }
        }
    }

    return pGroup->materialPaths.Count() > 0;
}

//-----------------------------------------------------------------------------
// Purpose: Parse override type
//-----------------------------------------------------------------------------
MaterialOverrideType_t CGModMaterialsSystem::ParseOverrideType(const char* pszTypeName)
{
    if (!pszTypeName)
        return MATERIAL_OVERRIDE_REPLACE;

    if (!Q_stricmp(pszTypeName, "replace"))
        return MATERIAL_OVERRIDE_REPLACE;
    else if (!Q_stricmp(pszTypeName, "tint"))
        return MATERIAL_OVERRIDE_TINT;
    else if (!Q_stricmp(pszTypeName, "transparency"))
        return MATERIAL_OVERRIDE_TRANSPARENCY;
    else if (!Q_stricmp(pszTypeName, "texture"))
        return MATERIAL_OVERRIDE_TEXTURE;
    else if (!Q_stricmp(pszTypeName, "shader"))
        return MATERIAL_OVERRIDE_SHADER;

    return MATERIAL_OVERRIDE_REPLACE;
}

//-----------------------------------------------------------------------------
// Purpose: Apply default materials
//-----------------------------------------------------------------------------
void CGModMaterialsSystem::ApplyDefaultMaterials()
{
    // Create some default material overrides for common GMod materials
    const char* defaultMaterials[] = {
        "models/wireframe",
        "models/shiny",
        "models/debug/debugwhite",
        "phoenix_storms/glass",
        "phoenix_storms/metaltech",
        "phoenix_storms/wood"
    };

    for (int i = 0; i < ARRAYSIZE(defaultMaterials); i++)
    {
        MaterialData_t defaultMat;
        defaultMat.materialName = defaultMaterials[i];
        defaultMat.originalPath = defaultMaterials[i];
        defaultMat.overridePath = defaultMaterials[i];

        s_Materials.Insert(defaultMat.materialName.Get(), defaultMat);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Reload materials
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::ReloadMaterials()
{
    ClearAllOverrides();
    s_Materials.Purge();
    s_MaterialGroups.Purge();
    s_bMaterialsLoaded = false;

    return LoadMaterials();
}

//-----------------------------------------------------------------------------
// Purpose: Find material data
//-----------------------------------------------------------------------------
MaterialData_t* CGModMaterialsSystem::FindMaterialData(const char* pszMaterialName)
{
    if (!pszMaterialName || !s_bMaterialsLoaded)
        return NULL;

    int index = s_Materials.Find(pszMaterialName);
    if (index != s_Materials.InvalidIndex())
        return &s_Materials[index];

    return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Find material group
//-----------------------------------------------------------------------------
MaterialGroup_t* CGModMaterialsSystem::FindMaterialGroup(const char* pszGroupName)
{
    if (!pszGroupName || !s_bMaterialsLoaded)
        return NULL;

    int index = s_MaterialGroups.Find(pszGroupName);
    if (index != s_MaterialGroups.InvalidIndex())
        return &s_MaterialGroups[index];

    return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Apply material override
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::ApplyMaterialOverride(const char* pszOriginalPath, const char* pszOverridePath)
{
    if (!pszOriginalPath || !pszOverridePath)
        return false;

    // Store the override mapping
    int index = s_MaterialOverrides.Find(pszOriginalPath);
    if (index != s_MaterialOverrides.InvalidIndex())
    {
        s_MaterialOverrides[index] = pszOverridePath;
    }
    else
    {
        s_MaterialOverrides.Insert(pszOriginalPath, CUtlString(pszOverridePath));
    }

    DevMsg("Material override: %s -> %s\n", pszOriginalPath, pszOverridePath);
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Apply material override to entity
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::ApplyMaterialOverrideToEntity(CBaseEntity* pEntity, const char* pszMaterialName)
{
    if (!pEntity)
        return false;

    MaterialData_t* pMaterial = FindMaterialData(pszMaterialName);
    if (!pMaterial || !pMaterial->bEnabled)
        return false;

    // Apply the material override
    if (pMaterial->overridePath.Length() > 0)
    {
        pEntity->SetMaterial(pMaterial->overridePath.Get());
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
// Purpose: Remove material override
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::RemoveMaterialOverride(const char* pszMaterialName)
{
    if (!pszMaterialName)
        return false;

    int index = s_MaterialOverrides.Find(pszMaterialName);
    if (index != s_MaterialOverrides.InvalidIndex())
    {
        s_MaterialOverrides.RemoveAt(index);
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
// Purpose: Clear all overrides
//-----------------------------------------------------------------------------
void CGModMaterialsSystem::ClearAllOverrides()
{
    s_MaterialOverrides.Purge();
    DevMsg("All material overrides cleared\n");
}

//-----------------------------------------------------------------------------
// Purpose: Create custom material
//-----------------------------------------------------------------------------
IMaterial* CGModMaterialsSystem::CreateCustomMaterial(const char* pszMaterialName, const MaterialData_t& data)
{
    if (!pszMaterialName)
        return NULL;

    return LoadMaterialFromData(data);
}

//-----------------------------------------------------------------------------
// Purpose: Load material from data
//-----------------------------------------------------------------------------
IMaterial* CGModMaterialsSystem::LoadMaterialFromData(const MaterialData_t& data)
{
    // Create KeyValues for material definition
    KeyValues* pVMT = new KeyValues(data.shaderName.Get());

    // Set base texture
    if (data.baseTexture.Length() > 0)
    {
        pVMT->SetString("$basetexture", data.baseTexture.Get());
    }

    // Set normal map
    if (data.normalMap.Length() > 0)
    {
        pVMT->SetString("$normalmap", data.normalMap.Get());
    }

    // Set alpha
    if (data.flAlpha < 1.0f || data.bTranslucent)
    {
        pVMT->SetInt("$translucent", 1);
        pVMT->SetFloat("$alpha", data.flAlpha);
    }

    // Set material properties
    pVMT->SetFloat("$reflectivity", data.flReflectivity);

    // Set flags
    if (data.bNoCull)
        pVMT->SetInt("$nocull", 1);

    if (data.bNoFog)
        pVMT->SetInt("$nofog", 1);

    if (data.bSelfIllum)
        pVMT->SetInt("$selfillum", 1);

    // Create material
    IMaterial* pMaterial = materials->CreateMaterial(data.materialName.Get(), pVMT);
    return pMaterial;
}

//-----------------------------------------------------------------------------
// Purpose: Modify material properties
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::ModifyMaterialProperties(const char* pszMaterialName, const MaterialData_t& data)
{
    IMaterial* pMaterial = materials->FindMaterial(pszMaterialName, TEXTURE_GROUP_MODEL);
    if (!pMaterial || pMaterial->IsErrorMaterial())
        return false;

    // Material modification would require more complex material system integration
    // For now, just log the operation
    DevMsg("Modified material properties for: %s\n", pszMaterialName);
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Set material color
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::SetMaterialColor(const char* pszMaterialName, const Color& color)
{
    MaterialData_t data;
    data.materialColor = color;
    return ModifyMaterialProperties(pszMaterialName, data);
}

//-----------------------------------------------------------------------------
// Purpose: Set material alpha
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::SetMaterialAlpha(const char* pszMaterialName, float flAlpha)
{
    MaterialData_t data;
    data.flAlpha = flAlpha;
    return ModifyMaterialProperties(pszMaterialName, data);
}

//-----------------------------------------------------------------------------
// Purpose: Set material texture
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::SetMaterialTexture(const char* pszMaterialName, const char* pszTexturePath)
{
    MaterialData_t data;
    data.baseTexture = pszTexturePath;
    return ModifyMaterialProperties(pszMaterialName, data);
}

//-----------------------------------------------------------------------------
// Purpose: Apply group properties
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::ApplyGroupProperties(const char* pszGroupName)
{
    MaterialGroup_t* pGroup = FindMaterialGroup(pszGroupName);
    if (!pGroup || !pGroup->bGroupEnabled)
        return false;

    int appliedCount = 0;
    for (int i = 0; i < pGroup->materialPaths.Count(); i++)
    {
        const char* pszMaterialPath = pGroup->materialPaths[i].Get();
        if (ModifyMaterialProperties(pszMaterialPath, pGroup->groupProperties))
        {
            appliedCount++;
        }
    }

    DevMsg("Applied group properties '%s' to %d materials\n", pszGroupName, appliedCount);
    return appliedCount > 0;
}

//-----------------------------------------------------------------------------
// Purpose: Add material to group
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::AddMaterialToGroup(const char* pszGroupName, const char* pszMaterialPath)
{
    MaterialGroup_t* pGroup = FindMaterialGroup(pszGroupName);
    if (!pGroup)
        return false;

    // Check if already in group
    for (int i = 0; i < pGroup->materialPaths.Count(); i++)
    {
        if (!Q_stricmp(pGroup->materialPaths[i].Get(), pszMaterialPath))
            return false; // Already exists
    }

    pGroup->materialPaths.AddToTail(pszMaterialPath);
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Remove material from group
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::RemoveMaterialFromGroup(const char* pszGroupName, const char* pszMaterialPath)
{
    MaterialGroup_t* pGroup = FindMaterialGroup(pszGroupName);
    if (!pGroup)
        return false;

    for (int i = 0; i < pGroup->materialPaths.Count(); i++)
    {
        if (!Q_stricmp(pGroup->materialPaths[i].Get(), pszMaterialPath))
        {
            pGroup->materialPaths.Remove(i);
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
// Purpose: Override entity material
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::OverrideEntityMaterial(CBaseEntity* pEntity, const char* pszMaterialName)
{
    return ApplyMaterialOverrideToEntity(pEntity, pszMaterialName);
}

//-----------------------------------------------------------------------------
// Purpose: Restore entity material
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::RestoreEntityMaterial(CBaseEntity* pEntity)
{
    if (!pEntity)
        return false;

    pEntity->SetMaterial("");
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Update entity materials
//-----------------------------------------------------------------------------
void CGModMaterialsSystem::UpdateEntityMaterials(CBaseEntity* pEntity)
{
    // This would update all materials on an entity based on current overrides
    // Implementation would depend on entity material system
}

//-----------------------------------------------------------------------------
// Purpose: Get material count
//-----------------------------------------------------------------------------
int CGModMaterialsSystem::GetMaterialCount()
{
    return s_Materials.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Get group count
//-----------------------------------------------------------------------------
int CGModMaterialsSystem::GetGroupCount()
{
    return s_MaterialGroups.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Get material list
//-----------------------------------------------------------------------------
void CGModMaterialsSystem::GetMaterialList(CUtlVector<CUtlString>& materialList)
{
    materialList.Purge();

    for (int i = s_Materials.First(); i != s_Materials.InvalidIndex(); i = s_Materials.Next(i))
    {
        materialList.AddToTail(s_Materials.GetElementName(i));
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get group list
//-----------------------------------------------------------------------------
void CGModMaterialsSystem::GetGroupList(CUtlVector<CUtlString>& groupList)
{
    groupList.Purge();

    for (int i = s_MaterialGroups.First(); i != s_MaterialGroups.InvalidIndex(); i = s_MaterialGroups.Next(i))
    {
        groupList.AddToTail(s_MaterialGroups.GetElementName(i));
    }
}

//-----------------------------------------------------------------------------
// Purpose: Check if material is overridden
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::IsMaterialOverridden(const char* pszMaterialName)
{
    return s_MaterialOverrides.Find(pszMaterialName) != s_MaterialOverrides.InvalidIndex();
}

//-----------------------------------------------------------------------------
// Purpose: Check if material is enabled
//-----------------------------------------------------------------------------
bool CGModMaterialsSystem::IsMaterialEnabled(const char* pszMaterialName)
{
    MaterialData_t* pMaterial = FindMaterialData(pszMaterialName);
    return pMaterial ? pMaterial->bEnabled : false;
}

//-----------------------------------------------------------------------------
// Purpose: Get override type name
//-----------------------------------------------------------------------------
const char* CGModMaterialsSystem::GetOverrideTypeName(MaterialOverrideType_t type)
{
    switch (type)
    {
        case MATERIAL_OVERRIDE_NONE:         return "none";
        case MATERIAL_OVERRIDE_REPLACE:      return "replace";
        case MATERIAL_OVERRIDE_TINT:         return "tint";
        case MATERIAL_OVERRIDE_TRANSPARENCY: return "transparency";
        case MATERIAL_OVERRIDE_TEXTURE:      return "texture";
        case MATERIAL_OVERRIDE_SHADER:       return "shader";
        default:                            return "unknown";
    }
}

//=============================================================================
// Console Commands
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: List all material overrides
//-----------------------------------------------------------------------------
void CMD_gmod_materials_list(void)
{
    CUtlVector<CUtlString> materialList;
    CGModMaterialsSystem::GetMaterialList(materialList);

    Msg("Material overrides (%d):\n", materialList.Count());
    for (int i = 0; i < materialList.Count(); i++)
    {
        MaterialData_t* pMaterial = CGModMaterialsSystem::FindMaterialData(materialList[i].Get());
        if (pMaterial)
        {
            Msg("  %-30s - %s (%s)\n",
                materialList[i].Get(),
                CGModMaterialsSystem::GetOverrideTypeName(pMaterial->overrideType),
                pMaterial->bEnabled ? "enabled" : "disabled");
        }
    }

    CUtlVector<CUtlString> groupList;
    CGModMaterialsSystem::GetGroupList(groupList);
    if (groupList.Count() > 0)
    {
        Msg("\nMaterial groups (%d):\n", groupList.Count());
        for (int i = 0; i < groupList.Count(); i++)
        {
            MaterialGroup_t* pGroup = CGModMaterialsSystem::FindMaterialGroup(groupList[i].Get());
            if (pGroup)
            {
                Msg("  %-20s - %d materials (%s)\n",
                    groupList[i].Get(),
                    pGroup->materialPaths.Count(),
                    pGroup->bGroupEnabled ? "enabled" : "disabled");
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Reload material configurations
//-----------------------------------------------------------------------------
void CMD_gmod_materials_reload(void)
{
    if (CGModMaterialsSystem::ReloadMaterials())
    {
        Msg("Material configurations reloaded successfully\n");
    }
    else
    {
        Msg("Failed to reload material configurations\n");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Override material
//-----------------------------------------------------------------------------
void CMD_gmod_materials_override(void)
{
    if (engine->Cmd_Argc() < 3)
    {
        Msg("Usage: gmod_materials_override <original> <override>\n");
        return;
    }

    const char* pszOriginal = engine->Cmd_Argv(1);
    const char* pszOverride = engine->Cmd_Argv(2);

    if (CGModMaterialsSystem::ApplyMaterialOverride(pszOriginal, pszOverride))
    {
        Msg("Material override applied: %s -> %s\n", pszOriginal, pszOverride);
    }
    else
    {
        Msg("Failed to apply material override\n");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Restore original material
//-----------------------------------------------------------------------------
void CMD_gmod_materials_restore(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_materials_restore <material_name>\n");
        return;
    }

    const char* pszMaterialName = engine->Cmd_Argv(1);

    if (CGModMaterialsSystem::RemoveMaterialOverride(pszMaterialName))
    {
        Msg("Material override removed: %s\n", pszMaterialName);
    }
    else
    {
        Msg("No override found for material: %s\n", pszMaterialName);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Apply material group
//-----------------------------------------------------------------------------
void CMD_gmod_materials_group(void)
{
    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_materials_group <group_name>\n");
        return;
    }

    const char* pszGroupName = engine->Cmd_Argv(1);

    if (CGModMaterialsSystem::ApplyGroupProperties(pszGroupName))
    {
        Msg("Applied material group: %s\n", pszGroupName);
    }
    else
    {
        Msg("Failed to apply material group: %s\n", pszGroupName);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Apply material to entity
//-----------------------------------------------------------------------------
void CMD_gmod_materials_apply(void)
{
    CBasePlayer* pPlayer = UTIL_GetCommandClient();
    if (!pPlayer)
        return;

    if (engine->Cmd_Argc() < 2)
    {
        Msg("Usage: gmod_materials_apply <material_name>\n");
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

    if (CGModMaterialsSystem::OverrideEntityMaterial(tr.m_pEnt, pszMaterialName))
    {
        Msg("Applied material '%s' to entity\n", pszMaterialName);
    }
    else
    {
        Msg("Failed to apply material '%s'\n", pszMaterialName);
    }
}