//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod NPC Tool - Implementation of NPC spawning tool
//          Based on Garry's Mod tool system analysis (found "gm_context npc")
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "npcevent.h"
#include "ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables for NPC tool
//-----------------------------------------------------------------------------
ConVar bm_npc_type("bm_npc_type", "npc_citizen", FCVAR_ARCHIVE, "Default NPC type to spawn");
ConVar bm_npc_health("bm_npc_health", "100", FCVAR_ARCHIVE, "Default NPC health");
ConVar bm_npc_limit("bm_npc_limit", "20", FCVAR_ARCHIVE, "Maximum NPCs per player");

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

static NPCInfo_t g_NPCInfo[] =
{
	{ "npc_citizen",		"Citizen",			"Friendly citizen NPC",		100,	false },
	{ "npc_barney",			"Barney",			"Security guard",			100,	false },
	{ "npc_alyx",			"Alyx",				"Alyx Vance",				100,	false },
	{ "npc_eli",			"Eli",				"Eli Vance",				100,	false },
	{ "npc_kleiner",		"Kleiner",			"Dr. Kleiner",				100,	false },
	{ "npc_vortigaunt",		"Vortigaunt",		"Friendly Vortigaunt",		100,	false },
	{ "npc_zombie",			"Zombie",			"Slow zombie",				50,		false },
	{ "npc_zombie_fast",	"Fast Zombie",		"Fast zombie",				50,		false },
	{ "npc_zombie_poison",	"Poison Zombie",	"Poison zombie",			175,	false },
	{ "npc_headcrab",		"Headcrab",			"Standard headcrab",		25,		false },
	{ "npc_headcrab_fast",	"Fast Headcrab",	"Fast headcrab",			25,		false },
	{ "npc_headcrab_poison","Poison Headcrab",	"Poison headcrab",			35,		false },
	{ "npc_combine_s",		"Combine Soldier",	"Civil Protection",			50,		false },
	{ "npc_metropolice",	"Civil Protection",	"Metro police",				40,		false },
	{ "npc_scanner",		"Scanner",			"City scanner",				30,		false },
	{ "npc_manhack",		"Manhack",			"Flying manhack",			25,		false },
	{ "npc_antlion",		"Antlion",			"Antlion warrior",			60,		false },
	{ "npc_antlionguard",	"Antlion Guard",	"Large antlion guard",		500,	false },
	{ NULL, NULL, NULL, 0, false }
};

//-----------------------------------------------------------------------------
// NPC tool class - implements TOOL_NPC mode
//-----------------------------------------------------------------------------
class CToolNPC : public CWeaponTool
{
	DECLARE_CLASS( CToolNPC, CWeaponTool );

public:
	CToolNPC() : m_nSelectedNPC(0), m_nNPCCount(0) {}

	virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void OnToolThink();

private:
	CBaseEntity *SpawnNPC( const char *pszNPCClass, const Vector &vecPos, const QAngle &angFacing );
	void DeleteNPC( CBaseEntity *pNPC );
	void CycleNPCType();
	int GetPlayerNPCCount( CBasePlayer *pPlayer );
	bool CanSpawnNPC( CBasePlayer *pPlayer );
	void CreateSpawnEffect( const Vector &vecPos );
	const NPCInfo_t *GetCurrentNPCInfo();

	// NPC tool state
	int		m_nSelectedNPC;		// Currently selected NPC type index
	int		m_nNPCCount;		// NPCs spawned this session
};

//-----------------------------------------------------------------------------
// Tool implementation for NPC mode
//-----------------------------------------------------------------------------
void CToolNPC::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		// Primary attack - spawn NPC or delete existing NPC
		if ( pEntity && pEntity->MyNPCPointer() )
		{
			// Clicking on an NPC - delete it
			DeleteNPC( pEntity );
		}
		else
		{
			// Clicking on empty space or non-NPC - spawn NPC
			if ( !CanSpawnNPC( pOwner ) )
			{
				ClientPrint( pOwner, HUD_PRINTTALK, "NPC limit reached (%d/%d)",
					GetPlayerNPCCount( pOwner ), bm_npc_limit.GetInt() );
				return;
			}

			const NPCInfo_t *pNPCInfo = GetCurrentNPCInfo();
			if ( pNPCInfo )
			{
				// Calculate spawn position and angle
				Vector vecSpawnPos = tr.endpos;
				QAngle angSpawn = pOwner->EyeAngles();
				angSpawn.x = 0; // Only use yaw for NPC facing

				// Spawn the NPC
				CBaseEntity *pNPC = SpawnNPC( pNPCInfo->pszClassName, vecSpawnPos, angSpawn );
				if ( pNPC )
				{
					CreateSpawnEffect( vecSpawnPos );
					PlayToolSound( "physics/wood/wood_crate_break5.wav" );

					m_nNPCCount++;

					ClientPrint( pOwner, HUD_PRINTTALK, "Spawned %s (%d/%d)",
						pNPCInfo->pszDisplayName,
						GetPlayerNPCCount( pOwner ),
						bm_npc_limit.GetInt() );
				}
				else
				{
					ClientPrint( pOwner, HUD_PRINTTALK, "Failed to spawn %s", pNPCInfo->pszDisplayName );
				}
			}
		}
	}
	else
	{
		// Secondary attack - cycle NPC type
		CycleNPCType();
	}
}

//-----------------------------------------------------------------------------
// Tool trace implementation for NPC mode
//-----------------------------------------------------------------------------
void CToolNPC::OnToolTrace( trace_t &tr, bool bPrimary )
{
	// Same as OnToolUse but with no entity
	OnToolUse( NULL, tr, bPrimary );
}

//-----------------------------------------------------------------------------
// Tool think for NPC mode
//-----------------------------------------------------------------------------
void CToolNPC::OnToolThink()
{
	// NPC tool doesn't need continuous thinking
}

//-----------------------------------------------------------------------------
// Spawn NPC
//-----------------------------------------------------------------------------
CBaseEntity *CToolNPC::SpawnNPC( const char *pszNPCClass, const Vector &vecPos, const QAngle &angFacing )
{
	if ( !pszNPCClass )
		return NULL;

	// Create the NPC entity
	CBaseEntity *pNPC = CreateEntityByName( pszNPCClass );
	if ( !pNPC )
	{
		DevMsg( "Failed to create NPC of type %s\n", pszNPCClass );
		return NULL;
	}

	// Set position and angles
	pNPC->SetAbsOrigin( vecPos );
	pNPC->SetAbsAngles( angFacing );

	// Set owner for cleanup tracking
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		pNPC->SetOwnerEntity( pOwner );
	}

	// Set health if specified
	const NPCInfo_t *pNPCInfo = GetCurrentNPCInfo();
	if ( pNPCInfo && pNPCInfo->nHealth > 0 )
	{
		pNPC->SetHealth( pNPCInfo->nHealth );
		pNPC->SetMaxHealth( pNPCInfo->nHealth );
	}

	// Spawn the NPC
	pNPC->Spawn();
	pNPC->Activate();

	DevMsg( "Spawned NPC %s at (%f, %f, %f)\n",
		pszNPCClass, vecPos.x, vecPos.y, vecPos.z );

	return pNPC;
}

//-----------------------------------------------------------------------------
// Delete NPC
//-----------------------------------------------------------------------------
void CToolNPC::DeleteNPC( CBaseEntity *pNPC )
{
	if ( !pNPC )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	// Check if player owns this NPC
	if ( pNPC->GetOwnerEntity() != pOwner )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "You can only delete NPCs you spawned" );
		return;
	}

	// Create deletion effect
	Vector vecPos = pNPC->GetAbsOrigin();
	CreateSpawnEffect( vecPos );

	// Remove the NPC
	const char *pszClassName = pNPC->GetClassname();
	UTIL_Remove( pNPC );

	PlayToolSound( "physics/wood/wood_crate_break5.wav" );
	ClientPrint( pOwner, HUD_PRINTTALK, "Deleted %s", pszClassName );
}

//-----------------------------------------------------------------------------
// Cycle NPC type
//-----------------------------------------------------------------------------
void CToolNPC::CycleNPCType()
{
	// Find next valid NPC type
	do
	{
		m_nSelectedNPC++;
		if ( g_NPCInfo[m_nSelectedNPC].pszClassName == NULL )
		{
			m_nSelectedNPC = 0; // Wrap around
		}
	} while ( g_NPCInfo[m_nSelectedNPC].pszClassName == NULL );

	// Update ConVar
	bm_npc_type.SetValue( g_NPCInfo[m_nSelectedNPC].pszClassName );

	// Inform player
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		const NPCInfo_t *pInfo = GetCurrentNPCInfo();
		ClientPrint( pOwner, HUD_PRINTTALK, "Selected: %s - %s",
			pInfo->pszDisplayName, pInfo->pszDescription );
	}
}

//-----------------------------------------------------------------------------
// Get player NPC count
//-----------------------------------------------------------------------------
int CToolNPC::GetPlayerNPCCount( CBasePlayer *pPlayer )
{
	if ( !pPlayer )
		return 0;

	int nCount = 0;

	// Count NPCs owned by this player
	CBaseEntity *pEntity = NULL;
	while ( (pEntity = gEntList.NextEnt( pEntity )) != NULL )
	{
		if ( pEntity->MyNPCPointer() && pEntity->GetOwnerEntity() == pPlayer )
		{
			nCount++;
		}
	}

	return nCount;
}

//-----------------------------------------------------------------------------
// Check if player can spawn NPC
//-----------------------------------------------------------------------------
bool CToolNPC::CanSpawnNPC( CBasePlayer *pPlayer )
{
	if ( !pPlayer )
		return false;

	// Check NPC limit
	int nCurrentCount = GetPlayerNPCCount( pPlayer );
	int nLimit = bm_npc_limit.GetInt();

	return nCurrentCount < nLimit;
}

//-----------------------------------------------------------------------------
// Create spawn effect
//-----------------------------------------------------------------------------
void CToolNPC::CreateSpawnEffect( const Vector &vecPos )
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
const NPCInfo_t *CToolNPC::GetCurrentNPCInfo()
{
	// Find NPC info matching the current ConVar
	const char *pszCurrentType = bm_npc_type.GetString();

	for ( int i = 0; g_NPCInfo[i].pszClassName; i++ )
	{
		if ( !Q_stricmp( g_NPCInfo[i].pszClassName, pszCurrentType ) )
		{
			m_nSelectedNPC = i;
			return &g_NPCInfo[i];
		}
	}

	// Default to first NPC if not found
	m_nSelectedNPC = 0;
	return &g_NPCInfo[0];
}

//-----------------------------------------------------------------------------
// Console command for NPC context menu (matching IDA finding)
//-----------------------------------------------------------------------------
CON_COMMAND( bm_context_npc, "Opens NPC tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has NPC tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_NPC )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "NPC tool must be equipped and selected" );
		return;
	}

	// In a full implementation, this would open the NPC context menu
	// For now, show available NPCs
	ClientPrint( pPlayer, HUD_PRINTTALK, "Available NPCs:" );

	for ( int i = 0; g_NPCInfo[i].pszClassName; i++ )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "  %s - %s",
			g_NPCInfo[i].pszDisplayName, g_NPCInfo[i].pszDescription );
	}

	ClientPrint( pPlayer, HUD_PRINTTALK, "Current: %s", bm_npc_type.GetString() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Use secondary fire to cycle NPC types" );
}

//-----------------------------------------------------------------------------
// Console command to clean up player NPCs
//-----------------------------------------------------------------------------
CON_COMMAND( bm_npc_cleanup, "Removes all NPCs spawned by the player" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	int nCount = 0;

	// Remove all NPCs owned by this player
	CBaseEntity *pEntity = NULL;
	while ( (pEntity = gEntList.NextEnt( pEntity )) != NULL )
	{
		if ( pEntity->MyNPCPointer() && pEntity->GetOwnerEntity() == pPlayer )
		{
			UTIL_Remove( pEntity );
			nCount++;
		}
	}

	ClientPrint( pPlayer, HUD_PRINTTALK, "Removed %d NPCs", nCount );
}