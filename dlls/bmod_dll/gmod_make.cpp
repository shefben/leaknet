#include "gmod_make.h"
#include "cbase.h"
#include "player.h"
#include "physics.h"
#include "props.h"
#include "ragdoll_shared.h"
#include "filesystem.h"
#include "gmod_undo.h"
#include "gmod_serverlimits.h"
#include "tier0/memdbgon.h"

// Static member definitions
CUtlVector<CGModMakeSystem::PlayerEntityTracker_t> CGModMakeSystem::s_PlayerTrackers;
CUtlVector<CUtlString> CGModMakeSystem::s_ValidModels;
CUtlVector<CUtlString> CGModMakeSystem::s_ValidRagdollModels;
bool CGModMakeSystem::s_bSystemInitialized = false;

// Global instance
CGModMakeSystem g_GMod_MakeSystem;

// ConVars for make system configuration - discovered in IDA analysis
ConVar gmod_make_default_mass("gmod_make_default_mass", "50", FCVAR_GAMEDLL, "Default mass for created props");
ConVar gmod_make_default_material("gmod_make_default_material", "default", FCVAR_GAMEDLL, "Default material for created props");
ConVar gmod_make_validate_models("gmod_make_validate_models", "1", FCVAR_GAMEDLL, "Validate model paths before creation");
ConVar gmod_make_precache_models("gmod_make_precache_models", "1", FCVAR_GAMEDLL, "Precache models on creation");
ConVar gmod_make_track_entities("gmod_make_track_entities", "1", FCVAR_GAMEDLL, "Track player-created entities");
ConVar gmod_make_max_props("gmod_make_max_props", "100", FCVAR_GAMEDLL, "Maximum props per player");
ConVar gmod_make_max_ragdolls("gmod_make_max_ragdolls", "20", FCVAR_GAMEDLL, "Maximum ragdolls per player");
ConVar gmod_make_max_effects("gmod_make_max_effects", "50", FCVAR_GAMEDLL, "Maximum effects per player");
ConVar gmod_make_spawn_distance("gmod_make_spawn_distance", "200", FCVAR_GAMEDLL, "Default spawn distance from player");
ConVar gmod_make_snap_to_ground("gmod_make_snap_to_ground", "0", FCVAR_GAMEDLL, "Snap spawned entities to ground");

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
// Helper function to get arguments from console command
//-----------------------------------------------------------------------------
static const char* GetCommandArg(int index)
{
    return engine->Cmd_Argv(index);
}

static int GetCommandArgCount()
{
    return engine->Cmd_Argc();
}

//-----------------------------------------------------------------------------
// CGModMakeSystem implementation
//-----------------------------------------------------------------------------
bool CGModMakeSystem::Init()
{
    s_PlayerTrackers.Purge();
    s_ValidModels.Purge();
    s_ValidRagdollModels.Purge();

    LoadModelLists();
    s_bSystemInitialized = true;

    DevMsg("GMod Make System initialized with %d models, %d ragdoll models\n",
           s_ValidModels.Count(), s_ValidRagdollModels.Count());

    return true;
}

void CGModMakeSystem::Shutdown()
{
    for (int i = 0; i < s_PlayerTrackers.Count(); i++)
    {
        s_PlayerTrackers[i].props.Purge();
        s_PlayerTrackers[i].ragdolls.Purge();
        s_PlayerTrackers[i].effects.Purge();
        s_PlayerTrackers[i].other.Purge();
    }
    s_PlayerTrackers.Purge();
    s_ValidModels.Purge();
    s_ValidRagdollModels.Purge();
    s_bSystemInitialized = false;

    DevMsg("GMod Make System shutdown\n");
}

void CGModMakeSystem::LevelInitPostEntity()
{
    // Clear all tracked entities when level changes
    for (int i = 0; i < s_PlayerTrackers.Count(); i++)
    {
        s_PlayerTrackers[i].props.Purge();
        s_PlayerTrackers[i].ragdolls.Purge();
        s_PlayerTrackers[i].effects.Purge();
        s_PlayerTrackers[i].other.Purge();
    }

    DevMsg("GMod Make System: Level initialized, entity tracking cleared\n");
}

CGModMakeSystem::PlayerEntityTracker_t* CGModMakeSystem::GetPlayerTracker(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return NULL;

    int playerIndex = pPlayer->entindex();

    // Ensure we have enough slots
    while (s_PlayerTrackers.Count() <= playerIndex)
    {
        s_PlayerTrackers.AddToTail();
    }

    return &s_PlayerTrackers[playerIndex];
}

void CGModMakeSystem::LoadModelLists()
{
    // Load common prop models
    s_ValidModels.AddToTail("models/props_c17/oildrum001.mdl");
    s_ValidModels.AddToTail("models/props_c17/concrete_barrier001a.mdl");
    s_ValidModels.AddToTail("models/props_c17/chair02a.mdl");
    s_ValidModels.AddToTail("models/props_c17/chair_stool01a.mdl");
    s_ValidModels.AddToTail("models/props_c17/furniturechair001a.mdl");
    s_ValidModels.AddToTail("models/props_c17/furniturecouch001a.mdl");
    s_ValidModels.AddToTail("models/props_c17/furnituretable001a.mdl");
    s_ValidModels.AddToTail("models/props_c17/shelfunit01a.mdl");
    s_ValidModels.AddToTail("models/props_c17/bench01a.mdl");
    s_ValidModels.AddToTail("models/props_c17/canister01a.mdl");
    s_ValidModels.AddToTail("models/props_c17/canister02a.mdl");
    s_ValidModels.AddToTail("models/props_combine/breenconsole.mdl");
    s_ValidModels.AddToTail("models/props_combine/breenchair.mdl");
    s_ValidModels.AddToTail("models/props_combine/breentable.mdl");

    // Load common ragdoll models
    s_ValidRagdollModels.AddToTail("models/humans/group01/male_01.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/male_02.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/male_03.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/male_04.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/male_05.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/male_06.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/male_07.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/male_08.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/male_09.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/female_01.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/female_02.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/female_03.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/female_04.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/female_06.mdl");
    s_ValidRagdollModels.AddToTail("models/humans/group01/female_07.mdl");
    s_ValidRagdollModels.AddToTail("models/combine_soldier.mdl");
    s_ValidRagdollModels.AddToTail("models/police.mdl");
    s_ValidRagdollModels.AddToTail("models/barney.mdl");
    s_ValidRagdollModels.AddToTail("models/alyx.mdl");
}

Vector CGModMakeSystem::GetPlayerCrosshairPosition(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return vec3_origin;

    Vector forward, start, end;
    AngleVectors(pPlayer->EyeAngles(), &forward);
    start = pPlayer->EyePosition();
    end = start + forward * gmod_make_spawn_distance.GetFloat();

    trace_t tr;
    UTIL_TraceLine(start, end, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

    Vector spawnPos = tr.endpos;

    if (gmod_make_snap_to_ground.GetBool())
    {
        // Trace down to find ground
        trace_t groundTr;
        UTIL_TraceLine(spawnPos, spawnPos + Vector(0, 0, -1000), MASK_SOLID, NULL, COLLISION_GROUP_NONE, &groundTr);
        if (groundTr.fraction < 1.0f)
        {
            spawnPos = groundTr.endpos;
        }
    }

    return spawnPos;
}

QAngle CGModMakeSystem::GetPlayerCrosshairAngles(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return vec3_angle;

    return pPlayer->EyeAngles();
}

CBaseEntity* CGModMakeSystem::GetCrosshairEntity(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return NULL;

    Vector forward, start, end;
    AngleVectors(pPlayer->EyeAngles(), &forward);
    start = pPlayer->EyePosition();
    end = start + forward * 4096.0f;

    trace_t tr;
    UTIL_TraceLine(start, end, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

    return tr.m_pEnt;
}

bool CGModMakeSystem::IsValidModel(const char* pszModel)
{
    if (!pszModel || !*pszModel)
        return false;

    if (!gmod_make_validate_models.GetBool())
        return true;

    // Check if model exists in file system
    return filesystem->FileExists(pszModel, "GAME");
}

bool CGModMakeSystem::IsValidRagdollModel(const char* pszModel)
{
    if (!IsValidModel(pszModel))
        return false;

    // Additional ragdoll-specific validation could go here
    return true;
}

const char* CGModMakeSystem::GetModelPath(const char* pszModel)
{
    if (!pszModel || !*pszModel)
        return "models/error.mdl";

    // If no path specified, try to find model in common locations
    if (!Q_strstr(pszModel, "/"))
    {
        static char fullPath[MAX_PATH];

        // Try props_c17 first
        Q_snprintf(fullPath, sizeof(fullPath), "models/props_c17/%s", pszModel);
        if (filesystem->FileExists(fullPath, "GAME"))
            return fullPath;

        // Try props_combine
        Q_snprintf(fullPath, sizeof(fullPath), "models/props_combine/%s", pszModel);
        if (filesystem->FileExists(fullPath, "GAME"))
            return fullPath;

        // Try humans/group01
        Q_snprintf(fullPath, sizeof(fullPath), "models/humans/group01/%s", pszModel);
        if (filesystem->FileExists(fullPath, "GAME"))
            return fullPath;
    }

    return pszModel;
}

void CGModMakeSystem::PrecacheModel(const char* pszModel)
{
    if (!gmod_make_precache_models.GetBool())
        return;

    if (IsValidModel(pszModel))
    {
        engine->PrecacheModel(pszModel);
    }
}

CBaseEntity* CGModMakeSystem::CreateProp(CBasePlayer* pPlayer, const char* pszModel, const Vector& position, const QAngle& angles)
{
    if (!pPlayer || !CanPlayerCreateEntity(pPlayer, GMAKE_PROP))
        return NULL;

    const char* modelPath = GetModelPath(pszModel);
    if (!IsValidModel(modelPath))
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Invalid model: %s", pszModel);
        return NULL;
    }

    PrecacheModel(modelPath);

    // Create physics prop
    CDynamicProp* pProp = (CDynamicProp*)CreateEntityByName("prop_physics");
    if (!pProp)
        return NULL;

    pProp->SetModel(modelPath);
    pProp->SetAbsOrigin(position);
    pProp->SetAbsAngles(angles);
    pProp->Spawn();
    pProp->Activate();

    // Set default physics properties
    SetEntityPhysics(pProp, gmod_make_default_mass.GetFloat(), gmod_make_default_material.GetString());

    // Track entity
    TrackPlayerEntity(pPlayer, pProp, GMAKE_PROP);

    // Record for undo system
    CGModUndoSystem::RecordEntitySpawn(pPlayer, pProp);

    ClientPrint(pPlayer, HUD_PRINTTALK, "Prop created: %s", modelPath);
    return pProp;
}

CRagdollProp* CGModMakeSystem::CreateRagdoll(CBasePlayer* pPlayer, const char* pszModel, const Vector& position, const QAngle& angles)
{
    if (!pPlayer || !CanPlayerCreateEntity(pPlayer, GMAKE_RAGDOLL))
        return NULL;

    const char* modelPath = GetModelPath(pszModel);
    if (!IsValidRagdollModel(modelPath))
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Invalid ragdoll model: %s", pszModel);
        return NULL;
    }

    PrecacheModel(modelPath);

    // Create ragdoll prop
    CRagdollProp* pRagdoll = (CRagdollProp*)CreateEntityByName("prop_ragdoll");
    if (!pRagdoll)
        return NULL;

    pRagdoll->SetModel(modelPath);
    pRagdoll->SetAbsOrigin(position);
    pRagdoll->SetAbsAngles(angles);
    pRagdoll->Spawn();
    pRagdoll->Activate();

    // Track entity
    TrackPlayerEntity(pPlayer, pRagdoll, GMAKE_RAGDOLL);

    // Record for undo system
    CGModUndoSystem::RecordEntitySpawn(pPlayer, pRagdoll);

    ClientPrint(pPlayer, HUD_PRINTTALK, "Ragdoll created: %s", modelPath);
    return pRagdoll;
}

CBaseEntity* CGModMakeSystem::CreateEffect(CBasePlayer* pPlayer, const char* pszEffect, const Vector& position, const QAngle& angles)
{
    if (!pPlayer || !CanPlayerCreateEntity(pPlayer, GMAKE_EFFECT))
        return NULL;

    // Create info_particle_system
    CBaseEntity* pEffect = CreateEntityByName("info_particle_system");
    if (!pEffect)
        return NULL;

    pEffect->SetAbsOrigin(position);
    pEffect->SetAbsAngles(angles);
    pEffect->Spawn();
    pEffect->Activate();

    // Track entity
    TrackPlayerEntity(pPlayer, pEffect, GMAKE_EFFECT);

    // Record for undo system
    CGModUndoSystem::RecordEntitySpawn(pPlayer, pEffect);

    ClientPrint(pPlayer, HUD_PRINTTALK, "Effect created: %s", pszEffect);
    return pEffect;
}

void CGModMakeSystem::ConfigureEntity(CBaseEntity* pEntity, const GMakeEntityData_t& data)
{
    if (!pEntity)
        return;

    // Set rendering properties
    SetEntityRendering(pEntity, data.color, data.renderMode, data.renderFX);

    // Set scale
    if (data.scale != 1.0f)
    {
        SetEntityScale(pEntity, data.scale);
    }

    // Set physics properties
    if (data.mass > 0.0f)
    {
        SetEntityPhysics(pEntity, data.mass, data.material.Get());
    }

    // Set velocity
    IPhysicsObject* pPhys = pEntity->GetPhysicsObject();
    if (pPhys && (data.velocity != vec3_origin || data.angularVelocity != vec3_angle))
    {
        Vector angVel(data.angularVelocity.x, data.angularVelocity.y, data.angularVelocity.z);
        pPhys->SetVelocity(&data.velocity, &angVel);
    }

    // Set other properties
    if (data.frozen)
    {
        if (pPhys)
            pPhys->EnableMotion(false);
    }

    if (data.nocollide)
    {
        pEntity->SetCollisionGroup(COLLISION_GROUP_DEBRIS);
    }

    if (data.notsolid)
    {
        pEntity->SetSolid(SOLID_NONE);
    }

    // Set health
    if (data.health != 100.0f)
    {
        pEntity->SetHealth(data.health);
        pEntity->SetMaxHealth(data.health);
    }
}

void CGModMakeSystem::SetEntityPhysics(CBaseEntity* pEntity, float mass, const char* pszMaterial)
{
    if (!pEntity)
        return;

    IPhysicsObject* pPhys = pEntity->GetPhysicsObject();
    if (!pPhys)
        return;

    if (mass > 0.0f)
    {
        pPhys->SetMass(mass);
    }

    // Set material properties (simplified)
    if (pszMaterial && *pszMaterial)
    {
        // Material surface properties would go here
    }
}

void CGModMakeSystem::SetEntityRendering(CBaseEntity* pEntity, const Color& color, int renderMode, int renderFX)
{
    if (!pEntity)
        return;

    pEntity->SetRenderColor(color.r(), color.g(), color.b(), color.a());
    pEntity->SetRenderMode((RenderMode_t)renderMode);
    pEntity->SetRenderFX((RenderFx_t)renderFX);
}

void CGModMakeSystem::SetEntityScale(CBaseEntity* pEntity, float scale)
{
    if (!pEntity || scale <= 0.0f)
        return;

    // Scale the entity (simplified implementation)
    pEntity->SetModelScale(scale);
}

bool CGModMakeSystem::CanPlayerCreateEntity(CBasePlayer* pPlayer, GMakeEntityType_t type)
{
    if (!pPlayer)
        return false;

    int currentCount = GetPlayerEntityCount(pPlayer, type);
    int maxCount = 0;

    switch (type)
    {
        case GMAKE_PROP:
            maxCount = gmod_make_max_props.GetInt();
            break;
        case GMAKE_RAGDOLL:
            maxCount = gmod_make_max_ragdolls.GetInt();
            break;
        case GMAKE_EFFECT:
            maxCount = gmod_make_max_effects.GetInt();
            break;
        default:
            maxCount = 100; // Default limit
            break;
    }

    if (currentCount >= maxCount)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Entity limit reached (%d/%d)", currentCount, maxCount);
        return false;
    }

    return true;
}

int CGModMakeSystem::GetPlayerEntityCount(CBasePlayer* pPlayer, GMakeEntityType_t type)
{
    PlayerEntityTracker_t* pTracker = GetPlayerTracker(pPlayer);
    if (!pTracker)
        return 0;

    switch (type)
    {
        case GMAKE_PROP:
            return pTracker->props.Count();
        case GMAKE_RAGDOLL:
            return pTracker->ragdolls.Count();
        case GMAKE_EFFECT:
            return pTracker->effects.Count();
        default:
            return pTracker->other.Count();
    }
}

void CGModMakeSystem::TrackPlayerEntity(CBasePlayer* pPlayer, CBaseEntity* pEntity, GMakeEntityType_t type)
{
    if (!gmod_make_track_entities.GetBool() || !pPlayer || !pEntity)
        return;

    PlayerEntityTracker_t* pTracker = GetPlayerTracker(pPlayer);
    if (!pTracker)
        return;

    EHANDLE hEntity = pEntity;

    switch (type)
    {
        case GMAKE_PROP:
            pTracker->props.AddToTail(hEntity);
            break;
        case GMAKE_RAGDOLL:
            pTracker->ragdolls.AddToTail(hEntity);
            break;
        case GMAKE_EFFECT:
            pTracker->effects.AddToTail(hEntity);
            break;
        default:
            pTracker->other.AddToTail(hEntity);
            break;
    }
}

void CGModMakeSystem::CleanupPlayerEntities(CBasePlayer* pPlayer)
{
    PlayerEntityTracker_t* pTracker = GetPlayerTracker(pPlayer);
    if (!pTracker)
        return;

    // Remove all tracked entities
    for (int i = 0; i < pTracker->props.Count(); i++)
    {
        if (pTracker->props[i].Get())
            UTIL_Remove(pTracker->props[i].Get());
    }

    for (int i = 0; i < pTracker->ragdolls.Count(); i++)
    {
        if (pTracker->ragdolls[i].Get())
            UTIL_Remove(pTracker->ragdolls[i].Get());
    }

    for (int i = 0; i < pTracker->effects.Count(); i++)
    {
        if (pTracker->effects[i].Get())
            UTIL_Remove(pTracker->effects[i].Get());
    }

    for (int i = 0; i < pTracker->other.Count(); i++)
    {
        if (pTracker->other[i].Get())
            UTIL_Remove(pTracker->other[i].Get());
    }

    // Clear tracking lists
    pTracker->props.Purge();
    pTracker->ragdolls.Purge();
    pTracker->effects.Purge();
    pTracker->other.Purge();

    ClientPrint(pPlayer, HUD_PRINTTALK, "All your entities have been removed");
}

//-----------------------------------------------------------------------------
// Console command implementations
//-----------------------------------------------------------------------------
void CMD_gmod_makeprop(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    const char* pszModel = "models/props_c17/oildrum001.mdl";
    if (GetCommandArgCount() > 1)
    {
        pszModel = GetCommandArg(1);
    }

    Vector position = CGModMakeSystem::GetPlayerCrosshairPosition(pPlayer);
    QAngle angles = CGModMakeSystem::GetPlayerCrosshairAngles(pPlayer);

    CGModMakeSystem::CreateProp(pPlayer, pszModel, position, angles);
}

void CMD_gmod_makeragdoll(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    const char* pszModel = "models/humans/group01/male_01.mdl";
    if (GetCommandArgCount() > 1)
    {
        pszModel = GetCommandArg(1);
    }

    Vector position = CGModMakeSystem::GetPlayerCrosshairPosition(pPlayer);
    QAngle angles = CGModMakeSystem::GetPlayerCrosshairAngles(pPlayer);

    CGModMakeSystem::CreateRagdoll(pPlayer, pszModel, position, angles);
}

void CMD_gmod_makeeffect(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    const char* pszEffect = "steam";
    if (GetCommandArgCount() > 1)
    {
        pszEffect = GetCommandArg(1);
    }

    Vector position = CGModMakeSystem::GetPlayerCrosshairPosition(pPlayer);
    QAngle angles = CGModMakeSystem::GetPlayerCrosshairAngles(pPlayer);

    CGModMakeSystem::CreateEffect(pPlayer, pszEffect, position, angles);
}

void CMD_gmod_freeze(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CBaseEntity* pEntity = CGModMakeSystem::GetCrosshairEntity(pPlayer);
    if (!pEntity)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "No entity found in crosshair");
        return;
    }

    IPhysicsObject* pPhys = pEntity->GetPhysicsObject();
    if (pPhys)
    {
        pPhys->EnableMotion(false);
        ClientPrint(pPlayer, HUD_PRINTTALK, "Entity frozen");
    }
}

void CMD_gmod_unfreeze(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CBaseEntity* pEntity = CGModMakeSystem::GetCrosshairEntity(pPlayer);
    if (!pEntity)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "No entity found in crosshair");
        return;
    }

    IPhysicsObject* pPhys = pEntity->GetPhysicsObject();
    if (pPhys)
    {
        pPhys->EnableMotion(true);
        pPhys->Wake();
        ClientPrint(pPlayer, HUD_PRINTTALK, "Entity unfrozen");
    }
}

void CMD_gmod_deleteall(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CGModMakeSystem::CleanupPlayerEntities(pPlayer);
}

void CMD_gmod_setmodel(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    if (GetCommandArgCount() < 2)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Usage: gmod_setmodel <model>");
        return;
    }

    CBaseEntity* pEntity = CGModMakeSystem::GetCrosshairEntity(pPlayer);
    if (!pEntity)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "No entity found in crosshair");
        return;
    }

    const char* pszModel = GetCommandArg(1);
    if (CGModMakeSystem::IsValidModel(pszModel))
    {
        pEntity->SetModel(pszModel);
        ClientPrint(pPlayer, HUD_PRINTTALK, "Model changed to: %s", pszModel);
    }
    else
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Invalid model: %s", pszModel);
    }
}

void CMD_gmod_setcolor(void)
{
    CBasePlayer* pPlayer = GetCommandPlayer();
    if (!pPlayer)
        return;

    CBaseEntity* pEntity = CGModMakeSystem::GetCrosshairEntity(pPlayer);
    if (!pEntity)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "No entity found in crosshair");
        return;
    }

    int r = 255, g = 255, b = 255, a = 255;

    if (GetCommandArgCount() >= 4)
    {
        r = atoi(GetCommandArg(1));
        g = atoi(GetCommandArg(2));
        b = atoi(GetCommandArg(3));
    }

    if (GetCommandArgCount() >= 5)
    {
        a = atoi(GetCommandArg(4));
    }

    Color color(r, g, b, a);
    CGModMakeSystem::SetEntityRendering(pEntity, color, 0, 0);
    ClientPrint(pPlayer, HUD_PRINTTALK, "Color changed to: %d %d %d %d", r, g, b, a);
}

//-----------------------------------------------------------------------------
// Console command registration
//-----------------------------------------------------------------------------
static ConCommand gmod_makeprop_cmd("gmod_makeprop", CMD_gmod_makeprop, "Create a prop at crosshair");
static ConCommand gmod_makeragdoll_cmd("gmod_makeragdoll", CMD_gmod_makeragdoll, "Create a ragdoll at crosshair");
static ConCommand gmod_makeeffect_cmd("gmod_makeeffect", CMD_gmod_makeeffect, "Create an effect at crosshair");
static ConCommand gmod_freeze_cmd("gmod_freeze", CMD_gmod_freeze, "Freeze entity in crosshair");
static ConCommand gmod_unfreeze_cmd("gmod_unfreeze", CMD_gmod_unfreeze, "Unfreeze entity in crosshair");
static ConCommand gmod_deleteall_cmd("gmod_deleteall", CMD_gmod_deleteall, "Delete all your entities");
static ConCommand gmod_setmodel_cmd("gmod_setmodel", CMD_gmod_setmodel, "Set model of entity in crosshair");
static ConCommand gmod_setcolor_cmd("gmod_setcolor", CMD_gmod_setcolor, "Set color of entity in crosshair");