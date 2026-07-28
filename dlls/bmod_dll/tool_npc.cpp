//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod NPC Tool - Implementation of NPC spawning tool
//          Based on Garry's Mod tool system analysis (found "gm_context npc")
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "npcevent.h"
#include "ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// ClientPrintf() only substitutes literal param1-4 strings into msg_name - it
// has no printf-style formatting of its own - so this wrapper formats first.
//-----------------------------------------------------------------------------
static void ClientPrintf( CBasePlayer *pPlayer, int msg_dest, const char *pszFormat, ... )
{
	char szBuf[256];
	va_list args;
	va_start( args, pszFormat );
	Q_vsnprintf( szBuf, sizeof( szBuf ), pszFormat, args );
	va_end( args );
	ClientPrint( pPlayer, msg_dest, szBuf );
}

//-----------------------------------------------------------------------------
// Console variables for NPC tool
//-----------------------------------------------------------------------------
// Weapon the build menu's "Weapon to give AI" combo box sets (owned by
// dlls/ai_concommands.cpp, shared with npc_create).
extern ConVar npc_create_equipment;

ConVar gmod_npc_type("gmod_npc_type", "npc_citizen", FCVAR_ARCHIVE, "Default NPC type to spawn");
ConVar gmod_npc_health("gmod_npc_health", "100", FCVAR_ARCHIVE, "Default NPC health");
ConVar gmod_npc_limit("gmod_npc_limit", "20", FCVAR_ARCHIVE, "Maximum NPCs per player");

//-----------------------------------------------------------------------------
// NPC type definitions - common Half-Life 2 NPCs
//-----------------------------------------------------------------------------
struct NPCInfo_t
{
	const char *pszClassName;
	const char *pszDisplayName;
	const char *pszDescription;
	int nHealth;
	bool bRequiresModel;
};

// Classnames must match the LINK_ENTITY_TO_CLASS names the 2003 beta NPCs
// actually register (dlls/bmod_dll/npc_*.cpp) - retail HL2 spellings like
// npc_zombie_fast or npc_scanner do not exist here and CreateEntityByName
// would just log "Attempted to create unknown entity type".
static NPCInfo_t g_NPCInfo[] =
{
	{ "npc_citizen",		"Citizen",			"Friendly citizen NPC",		100,	false },
	{ "npc_barney",			"Barney",			"Security guard",			100,	false },
	{ "npc_alyx",			"Alyx",				"Alyx Vance",				100,	false },
	{ "npc_eli",			"Eli",				"Eli Vance",				100,	false },
	{ "npc_kleiner",		"Kleiner",			"Dr. Kleiner",				100,	false },
	{ "npc_vortigaunt",		"Vortigaunt",		"Friendly Vortigaunt",		100,	false },
	{ "npc_zombie",			"Zombie",			"Slow zombie",				50,		false },
	{ "npc_fastzombie",		"Fast Zombie",		"Fast zombie",				50,		false },
	{ "npc_poisonzombie",	"Poison Zombie",	"Poison zombie",			175,	false },
	{ "npc_headcrab",		"Headcrab",			"Standard headcrab",		25,		false },
	{ "npc_headcrab_fast",	"Fast Headcrab",	"Fast headcrab",			25,		false },
	{ "npc_headcrab_poison","Poison Headcrab",	"Poison headcrab",			35,		false },
	{ "npc_combine_s",		"Combine Soldier",	"Civil Protection",			50,		false },
	{ "npc_metropolice",	"Civil Protection",	"Metro police",				40,		false },
	{ "npc_cscanner",		"Scanner",			"City scanner",				30,		false },
	{ "npc_manhack",		"Manhack",			"Flying manhack",			25,		false },
	{ "npc_antlion",		"Antlion",			"Antlion warrior",			60,		false },
	{ "npc_antlionguard",	"Antlion Guard",	"Large antlion guard",		500,	false },
	{ NULL, NULL, NULL, 0, false }
};

//-----------------------------------------------------------------------------
// Per-weapon NPC tool state - CWeaponTool is not subclassed, so the state
// that used to live in CToolNPC's member variables now lives here, keyed by
// the weapon's EHANDLE (mirrors the g_RemoverStates pattern in tool_remover.cpp).
//-----------------------------------------------------------------------------
struct NPCToolState_t
{
	EHANDLE	hTool;
	int		nSelectedNPC;	// Currently selected NPC type index
	int		nNPCCount;		// NPCs spawned this session

	NPCToolState_t()
	{
		nSelectedNPC = 0;
		nNPCCount = 0;
	}
};

static CUtlVector<NPCToolState_t*> g_NPCToolStates;

//-----------------------------------------------------------------------------
// Who spawned which NPC.
//
// This deliberately does NOT use SetOwnerEntity(): an entity never collides with
// its owner and traces started by the owner skip it, so an NPC "owned" by the
// player who spawned it could be walked through and could not be shot or killed
// by that player. Ownership is bookkeeping only, so keep it out of the entity.
//-----------------------------------------------------------------------------
struct SpawnedNPC_t
{
	EHANDLE hNPC;
	EHANDLE hSpawner;
};

static CUtlVector<SpawnedNPC_t> g_SpawnedNPCs;

static void PruneSpawnedNPCs()
{
	for ( int i = g_SpawnedNPCs.Count() - 1; i >= 0; i-- )
	{
		if ( !g_SpawnedNPCs[i].hNPC.Get() || !g_SpawnedNPCs[i].hSpawner.Get() )
		{
			g_SpawnedNPCs.Remove( i );
		}
	}
}

static void RegisterSpawnedNPC( CBaseEntity *pNPC, CBasePlayer *pSpawner )
{
	if ( !pNPC || !pSpawner )
		return;

	PruneSpawnedNPCs();

	SpawnedNPC_t entry;
	entry.hNPC = pNPC;
	entry.hSpawner = pSpawner;
	g_SpawnedNPCs.AddToTail( entry );
}

static CBasePlayer *GetNPCSpawner( CBaseEntity *pNPC )
{
	if ( !pNPC )
		return NULL;

	for ( int i = 0; i < g_SpawnedNPCs.Count(); i++ )
	{
		if ( g_SpawnedNPCs[i].hNPC.Get() == pNPC )
			return ToBasePlayer( g_SpawnedNPCs[i].hSpawner.Get() );
	}

	return NULL;
}

static NPCToolState_t *FindNPCToolState( CWeaponTool *pTool )
{
	for ( int i = 0; i < g_NPCToolStates.Count(); i++ )
	{
		if ( g_NPCToolStates[i]->hTool == pTool )
			return g_NPCToolStates[i];
	}
	return NULL;
}

static NPCToolState_t *GetNPCToolState( CWeaponTool *pTool )
{
	NPCToolState_t *pState = FindNPCToolState( pTool );
	if ( !pState )
	{
		pState = new NPCToolState_t;
		pState->hTool = pTool;
		g_NPCToolStates.AddToTail( pState );
	}
	return pState;
}

// Removes state records for weapons that no longer exist.
static void CleanupNPCToolStates()
{
	for ( int i = g_NPCToolStates.Count() - 1; i >= 0; i-- )
	{
		if ( !g_NPCToolStates[i]->hTool.Get() )
		{
			delete g_NPCToolStates[i];
			g_NPCToolStates.Remove( i );
		}
	}
}

//-----------------------------------------------------------------------------
// Forward declarations of tool helpers
//-----------------------------------------------------------------------------
static CBaseEntity *SpawnNPC( CWeaponTool *pTool, const char *pszNPCClass, const Vector &vecPos, const QAngle &angFacing );
static void DeleteNPC( CWeaponTool *pTool, CBaseEntity *pNPC );
static void CycleNPCType( CWeaponTool *pTool, NPCToolState_t *pState );
static int GetPlayerNPCCount( CBasePlayer *pPlayer );
static bool CanSpawnNPC( CBasePlayer *pPlayer );
static void CreateSpawnEffect( const Vector &vecPos );
static const NPCInfo_t *GetCurrentNPCInfo( NPCToolState_t *pState );

//-----------------------------------------------------------------------------
// Tool implementation for NPC mode
//-----------------------------------------------------------------------------
void Tool_NPC_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	NPCToolState_t *pState = GetNPCToolState( pTool );

	if ( bPrimary )
	{
		// Primary attack - spawn NPC or delete existing NPC
		if ( pEntity && pEntity->MyNPCPointer() )
		{
			// Clicking on an NPC - delete it
			DeleteNPC( pTool, pEntity );
		}
		else
		{
			// Clicking on empty space or non-NPC - spawn NPC
			if ( !CanSpawnNPC( pOwner ) )
			{
				ClientPrintf( pOwner, HUD_PRINTTALK, "NPC limit reached (%d/%d)",
					GetPlayerNPCCount( pOwner ), gmod_npc_limit.GetInt() );
				return;
			}

			const NPCInfo_t *pNPCInfo = GetCurrentNPCInfo( pState );
			if ( pNPCInfo )
			{
				// Calculate spawn position and angle
				Vector vecSpawnPos = tr.endpos;
				QAngle angSpawn = pOwner->EyeAngles();
				angSpawn.x = 0; // Only use yaw for NPC facing

				// Spawn the NPC
				CBaseEntity *pNPC = SpawnNPC( pTool, pNPCInfo->pszClassName, vecSpawnPos, angSpawn );
				if ( pNPC )
				{
					CreateSpawnEffect( vecSpawnPos );
					pTool->PlayToolSound( "physics/wood/wood_crate_break5.wav" );

					pState->nNPCCount++;

					ClientPrintf( pOwner, HUD_PRINTTALK, "Spawned %s (%d/%d)",
						pNPCInfo->pszDisplayName,
						GetPlayerNPCCount( pOwner ),
						gmod_npc_limit.GetInt() );
				}
				else
				{
					ClientPrintf( pOwner, HUD_PRINTTALK, "Failed to spawn %s", pNPCInfo->pszDisplayName );
				}
			}
		}
	}
	else
	{
		// Secondary attack - cycle NPC type
		CycleNPCType( pTool, pState );
	}
}

//-----------------------------------------------------------------------------
// Tool trace implementation for NPC mode
//-----------------------------------------------------------------------------
void Tool_NPC_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary )
{
	// Same as OnUse but with no entity
	Tool_NPC_OnUse( pTool, NULL, tr, bPrimary );
}

//-----------------------------------------------------------------------------
// Tool think for NPC mode
//-----------------------------------------------------------------------------
void Tool_NPC_OnThink( CWeaponTool *pTool )
{
	CleanupNPCToolStates();
}

//-----------------------------------------------------------------------------
// Spawn NPC
//-----------------------------------------------------------------------------
static CBaseEntity *SpawnNPC( CWeaponTool *pTool, const char *pszNPCClass, const Vector &vecPos, const QAngle &angFacing )
{
	if ( !pszNPCClass )
		return NULL;

	// Models/sounds for an NPC that isn't in the map were never precached at
	// level load, so open the precache window like npc_create does
	// (dlls/ai_concommands.cpp) - otherwise Spawn() sets an unprecached model.
	const bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	// Create the NPC entity
	CBaseEntity *pNPC = CreateEntityByName( pszNPCClass );
	if ( !pNPC )
	{
		CBaseEntity::SetAllowPrecache( bAllowPrecache );
		DevMsg( "Failed to create NPC of type %s\n", pszNPCClass );
		return NULL;
	}

	// Angles can be set up front; the position is applied with Teleport() after
	// Spawn() so the collision representation moves with it.
	pNPC->SetAbsAngles( angFacing );

	// Remember who spawned it for cleanup/limit tracking. Do NOT SetOwnerEntity()
	// here - that would make the NPC non-solid to (and unshootable by) its spawner.
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	RegisterSpawnedNPC( pNPC, pOwner );

	// Set health if specified
	const NPCInfo_t *pNPCInfo = GetCurrentNPCInfo( GetNPCToolState( pTool ) );
	if ( pNPCInfo && pNPCInfo->nHealth > 0 )
	{
		pNPC->SetHealth( pNPCInfo->nHealth );
		pNPC->SetMaxHealth( pNPCInfo->nHealth );
	}

	// Spawn the NPC
	pNPC->Precache();
	DispatchSpawn( pNPC );
	pNPC->Activate();

	CBaseEntity::SetAllowPrecache( bAllowPrecache );

	CAI_BaseNPC *pBaseNPC = pNPC->MyNPCPointer();
	if ( pBaseNPC && ( pBaseNPC->CapabilitiesGet() & bits_CAP_MOVE_FLY ) )
	{
		Vector vecFly = vecPos;
		vecFly.z += 36;
		pNPC->Teleport( &vecFly, NULL, NULL );
	}
	else
	{
		Vector vecDrop = vecPos;
		vecDrop.z += 12;
		pNPC->Teleport( &vecDrop, NULL, NULL );

		if ( pBaseNPC )
		{
			UTIL_DropToFloor( pBaseNPC, MASK_NPCSOLID );
		}
	}

	if ( pBaseNPC )
	{
		trace_t trFit;
		Vector vUpBit = pBaseNPC->GetAbsOrigin();
		vUpBit.z += 1;

		AI_TraceHull( pBaseNPC->GetAbsOrigin(), vUpBit, pBaseNPC->GetHullMins(), pBaseNPC->GetHullMaxs(),
			MASK_NPCSOLID, pBaseNPC, COLLISION_GROUP_NONE, &trFit );

		if ( trFit.startsolid || trFit.fraction < 1.0f )
		{
			UTIL_Remove( pNPC );
			return NULL;
		}

		pBaseNPC->Relink();
	}

	DevMsg( "Spawned NPC %s at (%f, %f, %f)\n",
		pszNPCClass, vecPos.x, vecPos.y, vecPos.z );

	return pNPC;
}

//-----------------------------------------------------------------------------
// Delete NPC
//-----------------------------------------------------------------------------
static void DeleteNPC( CWeaponTool *pTool, CBaseEntity *pNPC )
{
	if ( !pNPC )
		return;

	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	// Check if player owns this NPC
	if ( GetNPCSpawner( pNPC ) != pOwner )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "You can only delete NPCs you spawned" );
		return;
	}

	// Create deletion effect
	Vector vecPos = pNPC->GetAbsOrigin();
	CreateSpawnEffect( vecPos );

	// Remove the NPC
	const char *pszClassName = pNPC->GetClassname();
	UTIL_Remove( pNPC );

	pTool->PlayToolSound( "physics/wood/wood_crate_break5.wav" );
	ClientPrintf( pOwner, HUD_PRINTTALK, "Deleted %s", pszClassName );
}

//-----------------------------------------------------------------------------
// Cycle NPC type
//-----------------------------------------------------------------------------
static void CycleNPCType( CWeaponTool *pTool, NPCToolState_t *pState )
{
	// Find next valid NPC type
	do
	{
		pState->nSelectedNPC++;
		if ( g_NPCInfo[pState->nSelectedNPC].pszClassName == NULL )
		{
			pState->nSelectedNPC = 0; // Wrap around
		}
	} while ( g_NPCInfo[pState->nSelectedNPC].pszClassName == NULL );

	// Update ConVar
	gmod_npc_type.SetValue( g_NPCInfo[pState->nSelectedNPC].pszClassName );

	// Inform player
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( pOwner )
	{
		const NPCInfo_t *pInfo = GetCurrentNPCInfo( pState );
		ClientPrintf( pOwner, HUD_PRINTTALK, "Selected: %s - %s",
			pInfo->pszDisplayName, pInfo->pszDescription );
	}
}

//-----------------------------------------------------------------------------
// Get player NPC count
//-----------------------------------------------------------------------------
static int GetPlayerNPCCount( CBasePlayer *pPlayer )
{
	if ( !pPlayer )
		return 0;

	PruneSpawnedNPCs();

	int nCount = 0;
	for ( int i = 0; i < g_SpawnedNPCs.Count(); i++ )
	{
		if ( g_SpawnedNPCs[i].hSpawner.Get() == pPlayer )
		{
			nCount++;
		}
	}

	return nCount;
}

//-----------------------------------------------------------------------------
// Check if player can spawn NPC
//-----------------------------------------------------------------------------
static bool CanSpawnNPC( CBasePlayer *pPlayer )
{
	if ( !pPlayer )
		return false;

	// Check NPC limit
	int nCurrentCount = GetPlayerNPCCount( pPlayer );
	int nLimit = gmod_npc_limit.GetInt();

	return nCurrentCount < nLimit;
}

//-----------------------------------------------------------------------------
// Create spawn effect
//-----------------------------------------------------------------------------
static void CreateSpawnEffect( const Vector &vecPos )
{
	// Create teleport-in effect
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = 100.0f;
	data.m_flScale = 2.0f;

	DispatchEffect( "TeleportSplash", data );

	// Create some sparks
	data.m_nEntIndex = 0;
	data.m_vOrigin = vecPos + Vector(0, 0, 20);
	data.m_vNormal = Vector(0, 0, 1);
	data.m_flScale = 1.0f;

	DispatchEffect( "Sparks", data );
}

//-----------------------------------------------------------------------------
// Get current NPC info
//-----------------------------------------------------------------------------
static const NPCInfo_t *GetCurrentNPCInfo( NPCToolState_t *pState )
{
	// Find NPC info matching the current ConVar
	const char *pszCurrentType = gmod_npc_type.GetString();

	for ( int i = 0; g_NPCInfo[i].pszClassName; i++ )
	{
		if ( !Q_stricmp( g_NPCInfo[i].pszClassName, pszCurrentType ) )
		{
			pState->nSelectedNPC = i;
			return &g_NPCInfo[i];
		}
	}

	// Default to first NPC if not found
	pState->nSelectedNPC = 0;
	return &g_NPCInfo[0];
}

//-----------------------------------------------------------------------------
// Spawns an NPC where the player is looking. This is what the build menu's NPC
// panel (settings/context_panels/npc.txt) calls: npc_create is FCVAR_CHEAT, so
// on a release engine with maxplayers > 1 and sv_cheats 0 it is refused before
// the game DLL ever sees it.
//-----------------------------------------------------------------------------
CON_COMMAND( gm_spawnnpc, "Spawns an NPC of the given class where you are looking" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	if ( engine->Cmd_Argc() < 2 )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Usage: gm_spawnnpc <npc classname>" );
		return;
	}

	const char *pszClass = engine->Cmd_Argv( 1 );

	if ( !CanSpawnNPC( pPlayer ) )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "NPC limit reached (%d/%d)",
			GetPlayerNPCCount( pPlayer ), gmod_npc_limit.GetInt() );
		return;
	}

	// Where the player is aiming
	trace_t tr;
	Vector vecForward;
	pPlayer->EyeVectors( &vecForward );
	UTIL_TraceLine( pPlayer->EyePosition(), pPlayer->EyePosition() + vecForward * MAX_TRACE_LENGTH,
		MASK_NPCSOLID, pPlayer, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction == 1.0f )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Aim at the ground to spawn an NPC" );
		return;
	}

	// Nothing in the map precached this NPC, so open the precache window the
	// same way npc_create does (dlls/ai_concommands.cpp).
	const bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	CBaseEntity *pEntity = CreateEntityByName( pszClass );
	if ( !pEntity )
	{
		CBaseEntity::SetAllowPrecache( bAllowPrecache );
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Unknown NPC class %s", pszClass );
		return;
	}

	QAngle angSpawn = pPlayer->EyeAngles();
	angSpawn.x = 0;
	angSpawn.z = 0;

	pEntity->SetAbsAngles( angSpawn );

	const char *pszEquipment = npc_create_equipment.GetString();
	if ( pszEquipment && pszEquipment[0] )
	{
		pEntity->KeyValue( "additionalequipment", pszEquipment );
	}

	pEntity->Precache();
	DispatchSpawn( pEntity );
	pEntity->Activate();

	CBaseEntity::SetAllowPrecache( bAllowPrecache );

	// Place it the same way npc_create does (dlls/ai_concommands.cpp): move with
	// Teleport() so the collision representation follows, drop walkers to the
	// floor, then verify the hull actually fits before keeping the NPC. Setting
	// the origin before Spawn() left the NPC unlinked at the aim point, which is
	// why they ended up somewhere else and were not solid.
	CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
	if ( pNPC && ( pNPC->CapabilitiesGet() & bits_CAP_MOVE_FLY ) )
	{
		Vector vecSpawn = tr.endpos - vecForward * 36;
		pEntity->Teleport( &vecSpawn, NULL, NULL );
	}
	else
	{
		Vector vecSpawn = tr.endpos;
		vecSpawn.z += 12;
		pEntity->Teleport( &vecSpawn, NULL, NULL );

		if ( pNPC )
		{
			UTIL_DropToFloor( pNPC, MASK_NPCSOLID );
		}
	}

	if ( pNPC )
	{
		trace_t trFit;
		Vector vUpBit = pNPC->GetAbsOrigin();
		vUpBit.z += 1;

		AI_TraceHull( pNPC->GetAbsOrigin(), vUpBit, pNPC->GetHullMins(), pNPC->GetHullMaxs(),
			MASK_NPCSOLID, pNPC, COLLISION_GROUP_NONE, &trFit );

		if ( trFit.startsolid || trFit.fraction < 1.0f )
		{
			UTIL_Remove( pEntity );
			ClientPrintf( pPlayer, HUD_PRINTTALK, "Can't spawn %s there - not enough room", pszClass );
			return;
		}

		pNPC->Relink();
	}

	// Bookkeeping only - see RegisterSpawnedNPC(): using SetOwnerEntity() here is
	// what made spawned NPCs non-solid to, and unkillable by, the player.
	RegisterSpawnedNPC( pEntity, pPlayer );

	CreateSpawnEffect( pEntity->GetAbsOrigin() );

	ClientPrintf( pPlayer, HUD_PRINTTALK, "Spawned %s (%d/%d)", pszClass,
		GetPlayerNPCCount( pPlayer ), gmod_npc_limit.GetInt() );
}

//-----------------------------------------------------------------------------
// Console command for NPC context menu (matching IDA finding)
//-----------------------------------------------------------------------------
CON_COMMAND( gmod_context_npc, "Opens NPC tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has NPC tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_NPCSPAWN )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "NPC tool must be equipped and selected" );
		return;
	}

	// In a full implementation, this would open the NPC context menu
	// For now, show available NPCs
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Available NPCs:" );

	for ( int i = 0; g_NPCInfo[i].pszClassName; i++ )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "  %s - %s",
			g_NPCInfo[i].pszDisplayName, g_NPCInfo[i].pszDescription );
	}

	ClientPrintf( pPlayer, HUD_PRINTTALK, "Current: %s", gmod_npc_type.GetString() );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Use secondary fire to cycle NPC types" );
}

//-----------------------------------------------------------------------------
// Console command to clean up player NPCs
//-----------------------------------------------------------------------------
CON_COMMAND( gmod_npc_cleanup, "Removes all NPCs spawned by the player" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	PruneSpawnedNPCs();

	int nCount = 0;
	for ( int i = g_SpawnedNPCs.Count() - 1; i >= 0; i-- )
	{
		if ( g_SpawnedNPCs[i].hSpawner.Get() != pPlayer )
			continue;

		UTIL_Remove( g_SpawnedNPCs[i].hNPC.Get() );
		g_SpawnedNPCs.Remove( i );
		nCount++;
	}

	ClientPrintf( pPlayer, HUD_PRINTTALK, "Removed %d NPCs", nCount );
}
