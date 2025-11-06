#ifndef GMOD_PHYSPROPERTIES_H
#define GMOD_PHYSPROPERTIES_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "utlvector.h"
#include "utlstring.h"
#include "utldict.h"

// Forward declarations
class CBaseEntity;
class IPhysicsObject;

// Physics material types discovered from GMod analysis
enum PhysMaterialType_t
{
    PHYSMAT_DEFAULT = 0,
    PHYSMAT_METAL,
    PHYSMAT_WOOD,
    PHYSMAT_CONCRETE,
    PHYSMAT_GLASS,
    PHYSMAT_FLESH,
    PHYSMAT_PLASTIC,
    PHYSMAT_RUBBER,
    PHYSMAT_ICE,
    PHYSMAT_SAND,
    PHYSMAT_WATER,
    PHYSMAT_MAX
};

// Physics properties configuration data loaded from settings/gmod_physproperties.txt
struct PhysProperties_t
{
    CUtlString materialName;
    CUtlString surfaceProperty;

    // Basic physics properties
    float flDensity;           // Material density (kg/m³)
    float flFriction;          // Surface friction coefficient
    float flRestitution;       // Bounce/elasticity coefficient
    float flDamping;           // Energy damping factor

    // Break properties
    float flBreakStress;       // Stress required to break
    float flBreakStrain;       // Strain required to break
    bool bBreakable;           // Can this material break

    // Sound properties
    CUtlString impactSoundLight;   // Light impact sound
    CUtlString impactSoundHeavy;   // Heavy impact sound
    CUtlString breakSound;         // Break/shatter sound
    CUtlString rollSound;          // Rolling sound

    // Visual properties
    CUtlString breakEffect;        // Break particle effect
    CUtlString impactEffect;       // Impact particle effect
    CUtlString scrapeEffect;       // Scrape particle effect

    // Gameplay properties
    float flMassMult;          // Mass multiplier
    float flHealthMult;        // Health multiplier
    bool bFloats;              // Does this material float
    bool bClimbable;           // Can players climb on this

    PhysMaterialType_t materialType;

    PhysProperties_t()
    {
        flDensity = 1000.0f;       // Default water density
        flFriction = 0.7f;         // Default friction
        flRestitution = 0.4f;      // Default bounce
        flDamping = 0.1f;          // Default damping
        flBreakStress = 0.0f;      // Unbreakable by default
        flBreakStrain = 0.0f;
        bBreakable = false;
        flMassMult = 1.0f;
        flHealthMult = 1.0f;
        bFloats = false;
        bClimbable = true;
        materialType = PHYSMAT_DEFAULT;
    }
};

// Surface properties for different material interactions
struct SurfaceData_t
{
    CUtlString surfaceName;
    float flHardness;          // Surface hardness
    float flRoughness;         // Surface roughness
    Color surfaceColor;        // Material color tint
    bool bReflective;          // Is surface reflective

    SurfaceData_t()
    {
        flHardness = 0.5f;
        flRoughness = 0.5f;
        surfaceColor = Color(255, 255, 255, 255);
        bReflective = false;
    }
};

//-----------------------------------------------------------------------------
// GMod Physics Properties System - Manages material physics properties
// Based on IDA analysis: settings/gmod_physproperties.txt, PhysProperties
//-----------------------------------------------------------------------------
class CGModPhysPropertiesSystem : public CAutoGameSystem
{
public:
    CGModPhysPropertiesSystem() : CAutoGameSystem("PhysProperties") {}

    // CAutoGameSystem overrides
    virtual bool Init() override;
    virtual void Shutdown() override;
    virtual void LevelInitPostEntity() override;

    // Physics properties functions discovered from IDA
    static bool LoadPhysProperties(); // Loads settings/gmod_physproperties.txt
    static bool ReloadPhysProperties();
    static PhysProperties_t* FindPhysProperties(const char* pszMaterialName);
    static PhysProperties_t* FindPhysPropertiesForEntity(CBaseEntity* pEntity);
    static SurfaceData_t* FindSurfaceData(const char* pszSurfaceName);

    // Physics property application
    static bool ApplyPhysProperties(IPhysicsObject* pPhysicsObject, const char* pszMaterialName);
    static bool ApplyPhysPropertiesToEntity(CBaseEntity* pEntity, const char* pszMaterialName);
    static void UpdateEntityPhysProperties(CBaseEntity* pEntity);

    // Material interaction system
    static void OnImpact(CBaseEntity* pEntity1, CBaseEntity* pEntity2, const Vector& impactPos, float impactForce);
    static void OnBreak(CBaseEntity* pEntity, const Vector& breakPos);
    static void OnScrape(CBaseEntity* pEntity, const Vector& scrapePos, const Vector& scrapeDir);

    // Physics calculation helpers
    static float CalculateMass(const char* pszMaterialName, float volume);
    static float CalculateHealth(const char* pszMaterialName, float baseHealth);
    static bool ShouldFloat(const char* pszMaterialName);
    static float GetMaterialDensity(const char* pszMaterialName);

    // Material query functions
    static int GetPhysPropertiesCount();
    static void GetMaterialList(CUtlVector<CUtlString>& materialList);
    static bool IsMaterialRegistered(const char* pszMaterialName);
    static PhysMaterialType_t GetMaterialType(const char* pszMaterialName);

    // Utility functions
    static const char* GetMaterialTypeName(PhysMaterialType_t type);
    static const char* GetPhysPropertiesPath() { return "settings/gmod_physproperties.txt"; }

private:
    static CUtlDict<PhysProperties_t, int> s_PhysProperties;
    static CUtlDict<SurfaceData_t, int> s_SurfaceData;
    static bool s_bSystemInitialized;
    static bool s_bPropertiesLoaded;

    // Helper functions based on IDA analysis
    static bool ParsePhysPropertiesFile(const char* pszFilePath);
    static bool ParseMaterialSection(KeyValues* pKV, PhysProperties_t* pProperties);
    static bool ParseSurfaceSection(KeyValues* pKV, SurfaceData_t* pSurface);
    static PhysMaterialType_t ParseMaterialType(const char* pszTypeName);
    static void ApplyDefaultProperties(PhysProperties_t* pProperties);
};

// Global instance
extern CGModPhysPropertiesSystem g_GMod_PhysPropertiesSystem;

// Console command handlers
void CMD_gmod_physproperties_list(void);
void CMD_gmod_physproperties_reload(void);
void CMD_gmod_physproperties_info(void);
void CMD_gmod_physproperties_apply(void);

#endif // GMOD_PHYSPROPERTIES_H