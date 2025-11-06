#ifndef GMOD_MATERIALS_H
#define GMOD_MATERIALS_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "utlvector.h"
#include "utlstring.h"
#include "utldict.h"

// Forward declarations
class CBaseEntity;
class IMaterial;

// Material override types
enum MaterialOverrideType_t
{
    MATERIAL_OVERRIDE_NONE = 0,
    MATERIAL_OVERRIDE_REPLACE,      // Replace entirely
    MATERIAL_OVERRIDE_TINT,         // Apply color tint
    MATERIAL_OVERRIDE_TRANSPARENCY, // Change transparency
    MATERIAL_OVERRIDE_TEXTURE,      // Replace texture only
    MATERIAL_OVERRIDE_SHADER,       // Replace shader
    MATERIAL_OVERRIDE_MAX
};

// Material properties discovered from GMod analysis
struct MaterialData_t
{
    CUtlString materialName;
    CUtlString originalPath;
    CUtlString overridePath;
    CUtlString shaderName;
    CUtlString baseTexture;
    CUtlString normalMap;
    CUtlString detailTexture;

    // Visual properties
    Color materialColor;
    float flAlpha;
    float flReflectivity;
    float flMetallic;
    float flRoughness;
    bool bTranslucent;
    bool bNoCull;
    bool bNoFog;
    bool bSelfIllum;

    // Animation properties
    bool bAnimated;
    float flAnimationSpeed;
    int iFrameCount;

    // Override settings
    MaterialOverrideType_t overrideType;
    bool bEnabled;
    bool bApplyToAll;

    MaterialData_t()
    {
        materialColor = Color(255, 255, 255, 255);
        flAlpha = 1.0f;
        flReflectivity = 0.5f;
        flMetallic = 0.0f;
        flRoughness = 0.5f;
        bTranslucent = false;
        bNoCull = false;
        bNoFog = false;
        bSelfIllum = false;
        bAnimated = false;
        flAnimationSpeed = 1.0f;
        iFrameCount = 1;
        overrideType = MATERIAL_OVERRIDE_NONE;
        bEnabled = true;
        bApplyToAll = false;
    }
};

// Material group for batch operations
struct MaterialGroup_t
{
    CUtlString groupName;
    CUtlVector<CUtlString> materialPaths;
    MaterialData_t groupProperties;
    bool bGroupEnabled;

    MaterialGroup_t()
    {
        bGroupEnabled = true;
    }
};

//-----------------------------------------------------------------------------
// GMod Materials System - Manages material overrides and custom materials
// Based on IDA analysis: settings/gmod_materials.txt, Materials system
//-----------------------------------------------------------------------------
class CGModMaterialsSystem : public CAutoGameSystem
{
public:
    CGModMaterialsSystem() : CAutoGameSystem("Materials") {}

    // CAutoGameSystem overrides
    virtual bool Init() override;
    virtual void Shutdown() override;
    virtual void LevelInitPostEntity() override;

    // Material system functions discovered from IDA
    static bool LoadMaterials(); // Loads settings/gmod_materials.txt
    static bool ReloadMaterials();
    static MaterialData_t* FindMaterialData(const char* pszMaterialName);
    static MaterialGroup_t* FindMaterialGroup(const char* pszGroupName);

    // Material override system
    static bool ApplyMaterialOverride(const char* pszOriginalPath, const char* pszOverridePath);
    static bool ApplyMaterialOverrideToEntity(CBaseEntity* pEntity, const char* pszMaterialName);
    static bool RemoveMaterialOverride(const char* pszMaterialName);
    static void ClearAllOverrides();

    // Material manipulation
    static IMaterial* CreateCustomMaterial(const char* pszMaterialName, const MaterialData_t& data);
    static bool ModifyMaterialProperties(const char* pszMaterialName, const MaterialData_t& data);
    static bool SetMaterialColor(const char* pszMaterialName, const Color& color);
    static bool SetMaterialAlpha(const char* pszMaterialName, float flAlpha);
    static bool SetMaterialTexture(const char* pszMaterialName, const char* pszTexturePath);

    // Material group operations
    static bool ApplyGroupProperties(const char* pszGroupName);
    static bool AddMaterialToGroup(const char* pszGroupName, const char* pszMaterialPath);
    static bool RemoveMaterialFromGroup(const char* pszGroupName, const char* pszMaterialPath);

    // Entity material operations
    static bool OverrideEntityMaterial(CBaseEntity* pEntity, const char* pszMaterialName);
    static bool RestoreEntityMaterial(CBaseEntity* pEntity);
    static void UpdateEntityMaterials(CBaseEntity* pEntity);

    // Material query functions
    static int GetMaterialCount();
    static int GetGroupCount();
    static void GetMaterialList(CUtlVector<CUtlString>& materialList);
    static void GetGroupList(CUtlVector<CUtlString>& groupList);
    static bool IsMaterialOverridden(const char* pszMaterialName);
    static bool IsMaterialEnabled(const char* pszMaterialName);

    // Utility functions
    static const char* GetOverrideTypeName(MaterialOverrideType_t type);
    static const char* GetMaterialsPath() { return "settings/gmod_materials.txt"; }

private:
    static CUtlDict<MaterialData_t, int> s_Materials;
    static CUtlDict<MaterialGroup_t, int> s_MaterialGroups;
    static CUtlDict<CUtlString, int> s_MaterialOverrides; // original -> override mapping
    static bool s_bSystemInitialized;
    static bool s_bMaterialsLoaded;

    // Helper functions based on IDA analysis
    static bool ParseMaterialsFile(const char* pszFilePath);
    static bool ParseMaterialSection(KeyValues* pKV, MaterialData_t* pMaterial);
    static bool ParseGroupSection(KeyValues* pKV, MaterialGroup_t* pGroup);
    static MaterialOverrideType_t ParseOverrideType(const char* pszTypeName);
    static void ApplyDefaultMaterials();
    static IMaterial* LoadMaterialFromData(const MaterialData_t& data);
};

// Global instance
extern CGModMaterialsSystem g_GMod_MaterialsSystem;

// Console command handlers
void CMD_gmod_materials_list(void);
void CMD_gmod_materials_reload(void);
void CMD_gmod_materials_override(void);
void CMD_gmod_materials_restore(void);
void CMD_gmod_materials_group(void);
void CMD_gmod_materials_apply(void);

#endif // GMOD_MATERIALS_H