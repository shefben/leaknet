#ifndef GMOD_MAKE_H
#define GMOD_MAKE_H

#pragma once

#include "cbase.h"
#include "igameevents.h"
#include "convar.h"
#include "utlvector.h"
#include "igamesystem.h"

// Forward declarations
class CBaseEntity;
class CBasePlayer;
class CRagdollProp;
class CDynamicProp;

// Entity creation types discovered in IDA analysis
enum GMakeEntityType_t
{
    GMAKE_PROP = 0,                // Standard prop_physics
    GMAKE_RAGDOLL,                 // Ragdoll entity
    GMAKE_EFFECT,                  // Effect entity
    GMAKE_SPRITE,                  // Sprite entity
    GMAKE_LIGHT,                   // Light entity
    GMAKE_SOUND,                   // Sound entity
    GMAKE_DYNAMIC_PROP,            // prop_dynamic
    GMAKE_PHYSICS_PROP,            // prop_physics
    GMAKE_STATIC_PROP,             // prop_static
    GMAKE_ROPE,                    // Rope entity
    GMAKE_BEAM,                    // Beam entity
    GMAKE_PARTICLE,                // Particle entity
    GMAKE_MAX
};

// Entity creation parameters
struct GMakeEntityData_t
{
    GMakeEntityType_t entityType;
    char modelPath[256];
    char materialPath[256];
    Vector position;
    QAngle angles;
    Vector velocity;
    QAngle angularVelocity;
    float scale;
    Color color;
    int renderMode;
    int renderFX;
    float mass;
    char material[256];
    bool frozen;
    bool nocollide;
    bool notsolid;
    int skin;
    int bodygroup;
    float health;
    float damage;
    bool breakable;
    bool persistent;

    GMakeEntityData_t()
    {
        entityType = GMAKE_PROP;
        position = vec3_origin;
        angles = vec3_angle;
        velocity = vec3_origin;
        angularVelocity = vec3_angle;
        scale = 1.0f;
        color = Color(255, 255, 255, 255);
        renderMode = 0;
        renderFX = 0;
        mass = 0.0f;
        frozen = false;
        nocollide = false;
        notsolid = false;
        skin = 0;
        bodygroup = 0;
        health = 100.0f;
        damage = 0.0f;
        breakable = true;
        persistent = false;
    }
};

//-----------------------------------------------------------------------------
// GMod Make System - Implements entity creation commands from GMod 9.0.4b
//-----------------------------------------------------------------------------
class CGModMakeSystem : public CAutoGameSystem
{
public:
    CGModMakeSystem() : CAutoGameSystem("GMod Make System") {}

    // CAutoGameSystem overrides
    virtual bool Init() override;
    virtual void Shutdown() override;
    virtual void LevelInitPostEntity() override;

    // Entity creation functions
    static CBaseEntity* CreateProp(CBasePlayer* pPlayer, const char* pszModel, const Vector& position, const QAngle& angles);
    static CRagdollProp* CreateRagdoll(CBasePlayer* pPlayer, const char* pszModel, const Vector& position, const QAngle& angles);
    static CBaseEntity* CreateEffect(CBasePlayer* pPlayer, const char* pszEffect, const Vector& position, const QAngle& angles);
    static CBaseEntity* CreateSprite(CBasePlayer* pPlayer, const char* pszSprite, const Vector& position);
    static CBaseEntity* CreateSound(CBasePlayer* pPlayer, const char* pszSound, const Vector& position);
    static CBaseEntity* CreateRope(CBasePlayer* pPlayer, const Vector& start, const Vector& end);
    static CBaseEntity* CreateBeam(CBasePlayer* pPlayer, const Vector& start, const Vector& end);

    // Entity configuration
    static void ConfigureEntity(CBaseEntity* pEntity, const GMakeEntityData_t& data);
    static void SetEntityPhysics(CBaseEntity* pEntity, float mass, const char* pszMaterial);
    static void SetEntityRendering(CBaseEntity* pEntity, const Color& color, int renderMode, int renderFX);
    static void SetEntityScale(CBaseEntity* pEntity, float scale);

    // Model validation and loading
    static bool IsValidModel(const char* pszModel);
    static bool IsValidRagdollModel(const char* pszModel);
    static const char* GetModelPath(const char* pszModel);
    static void PrecacheModel(const char* pszModel);

    // Entity tracking and limits
    static int GetPlayerEntityCount(CBasePlayer* pPlayer, GMakeEntityType_t type);
    static bool CanPlayerCreateEntity(CBasePlayer* pPlayer, GMakeEntityType_t type);
    static void TrackPlayerEntity(CBasePlayer* pPlayer, CBaseEntity* pEntity, GMakeEntityType_t type);
    static void CleanupPlayerEntities(CBasePlayer* pPlayer);

    // Utility functions
    static Vector GetPlayerCrosshairPosition(CBasePlayer* pPlayer);
    static QAngle GetPlayerCrosshairAngles(CBasePlayer* pPlayer);
    static CBaseEntity* GetCrosshairEntity(CBasePlayer* pPlayer);

private:
    struct PlayerEntityTracker_t
    {
        CUtlVector<EHANDLE> props;
        CUtlVector<EHANDLE> ragdolls;
        CUtlVector<EHANDLE> effects;
        CUtlVector<EHANDLE> other;
    };

    static CUtlVector<PlayerEntityTracker_t> s_PlayerTrackers;
    static CUtlVector<const char*> s_ValidModels;
    static CUtlVector<const char*> s_ValidRagdollModels;
    static bool s_bSystemInitialized;

    static PlayerEntityTracker_t* GetPlayerTracker(CBasePlayer* pPlayer);
    static void LoadModelLists();
    static void ValidateModelPath(const char* pszModel);
};

// Global instance
extern CGModMakeSystem g_GMod_MakeSystem;

// Console command handlers - discovered from IDA string analysis
void CMD_gmod_makeprop(void);
void CMD_gmod_makeragdoll(void);
void CMD_gmod_makeeffect(void);
void CMD_gmod_makesprite(void);
void CMD_gmod_makelight(void);
void CMD_gmod_makesound(void);
void CMD_gmod_makerope(void);
void CMD_gmod_makebeam(void);
void CMD_gmod_makeparticle(void);

// Entity modification commands
void CMD_gmod_setmodel(void);
void CMD_gmod_setmaterial(void);
void CMD_gmod_setscale(void);
void CMD_gmod_setcolor(void);
void CMD_gmod_setmass(void);
void CMD_gmod_freeze(void);
void CMD_gmod_unfreeze(void);
void CMD_gmod_setnocollide(void);
void CMD_gmod_setsolid(void);
void CMD_gmod_setnotsolid(void);

// Advanced entity commands
void CMD_gmod_clone(void);
void CMD_gmod_copy(void);
void CMD_gmod_paste(void);
void CMD_gmod_duplicate(void);
void CMD_gmod_mirror(void);
void CMD_gmod_align(void);
void CMD_gmod_snap(void);

// Entity property commands
void CMD_gmod_setskin(void);
void CMD_gmod_setbodygroup(void);
void CMD_gmod_sethealth(void);
void CMD_gmod_setbreakable(void);
void CMD_gmod_setpersistent(void);

// Batch operations
void CMD_gmod_selectall(void);
void CMD_gmod_deleteall(void);
void CMD_gmod_freezeall(void);
void CMD_gmod_unfreezeall(void);

#endif // GMOD_MAKE_H