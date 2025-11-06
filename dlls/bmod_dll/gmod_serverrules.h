//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Server Rules & Game Mode System - GMod 9.0.4b compatible
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_SERVERRULES_H
#define GMOD_SERVERRULES_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CBasePlayer;
class CAI_BaseNPC;

//-----------------------------------------------------------------------------
// Game Mode Types
//-----------------------------------------------------------------------------
enum GMod_GameMode_t
{
    GAMEMODE_SANDBOX = 0,
    GAMEMODE_BUILD,
    GAMEMODE_ROLEPLAY,
    GAMEMODE_DEATHMATCH,
    GAMEMODE_TEAMPLAY,
    GAMEMODE_CUSTOM,

    GAMEMODE_COUNT
};

//-----------------------------------------------------------------------------
// Server Rules Management System
//-----------------------------------------------------------------------------
class CGModServerRules
{
public:
    // System initialization
    static void Initialize();
    static void Shutdown();

    // Game mode management
    static void SetGameMode(GMod_GameMode_t mode);
    static GMod_GameMode_t GetGameMode();
    static const char* GetGameModeName(GMod_GameMode_t mode);

    // Player management
    static bool CanPlayerSpawn(CBasePlayer* pPlayer);
    static bool CanPlayerRespawn(CBasePlayer* pPlayer);
    static void HandlePlayerConnect(CBasePlayer* pPlayer);
    static void HandlePlayerDisconnect(CBasePlayer* pPlayer);

    // NPC management
    static bool CanSpawnNPC(CBasePlayer* pPlayer, const char* npcClass);
    static void ModifyNPCHealth(CAI_BaseNPC* pNPC);
    static void HandleNPCSpawn(CAI_BaseNPC* pNPC, CBasePlayer* pOwner);

    // Entity spawn rules
    static bool CanSpawnEntity(CBasePlayer* pPlayer, const char* entityClass);
    static void HandleEntitySpawn(CBaseEntity* pEntity, CBasePlayer* pOwner);

    // Weapon management
    static bool CanPlayerPickupWeapon(CBasePlayer* pPlayer, CBaseCombatWeapon* pWeapon);
    static bool CanPlayerDropWeapon(CBasePlayer* pPlayer, CBaseCombatWeapon* pWeapon);

    // Physics rules
    static bool CanGrabEntity(CBasePlayer* pPlayer, CBaseEntity* pEntity);
    static bool CanWeldEntity(CBasePlayer* pPlayer, CBaseEntity* pEntity);

    // Chat and communication
    static bool CanPlayerChat(CBasePlayer* pPlayer);
    static void FilterChatMessage(CBasePlayer* pPlayer, char* message, int maxlen);

    // Console command handlers
    static void CMD_gm_gamemode(void);
    static void CMD_gm_restart(void);
    static void CMD_gm_cleanup(void);
    static void CMD_gm_noclip_all(void);
    static void CMD_gm_god_all(void);
    static void CMD_gm_freeze_all(void);
    static void CMD_gm_unfreeze_all(void);

    // Utility functions
    static void ApplyGameModeRules();
    static void ResetPlayerStates();
    static void BroadcastMessage(const char* message);

private:
    static bool m_bInitialized;
    static GMod_GameMode_t m_CurrentGameMode;
    static float m_flLastCleanupTime;

    static CBasePlayer* GetCommandPlayer();
    static void UpdateGameModeSettings();
    static void ValidatePlayerStates();
};

// Console variables for server rules
extern ConVar gm_sv_gamemode;
extern ConVar gm_sv_allownpc;
extern ConVar gm_sv_npchealthmultiplier;
extern ConVar gm_sv_allowweapons;
extern ConVar gm_sv_allowphysgun;
extern ConVar gm_sv_allowtoolgun;
extern ConVar gm_sv_maxprops;
extern ConVar gm_sv_maxragdolls;
extern ConVar gm_sv_maxnpcs;
extern ConVar gm_sv_respawntime;
extern ConVar gm_sv_noclip;
extern ConVar gm_sv_god;
extern ConVar gm_sv_gravity;
extern ConVar gm_sv_airaccelerate;
extern ConVar gm_sv_friction;
extern ConVar gm_sv_bounce;
extern ConVar gm_sv_allowchat;
extern ConVar gm_sv_allowvoice;
extern ConVar gm_sv_teamplay;
extern ConVar gm_sv_friendly_fire;

#endif // GMOD_SERVERRULES_H