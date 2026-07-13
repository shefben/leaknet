// Garry's Mod style spawn command (gm_spawn) for spawning entities or models.
#include "cbase.h"
#include "props.h"
#include "ai_basenpc.h"
#include "ndebugoverlay.h"
#include "vstdlib/strtools.h"
#include "gmod_make.h"
#include <string.h>

// Simple heuristic: if it looks like a model path (.mdl or contains / or \\), spawn prop_physics.
static bool IsLikelyModelPath( const char *psz )
{
	if ( !psz || !psz[0] )
		return false;
	return ( Q_stristr( psz, ".mdl" ) != NULL ) || strchr( psz, '/' ) != NULL || strchr( psz, '\\' ) != NULL;
}

static bool LooksLikeRagdoll( const char *psz )
{
	if ( !psz || !psz[0] )
		return false;
	char lower[256]; Q_strncpy( lower, psz, sizeof( lower ) ); Q_strlower( lower );
	return Q_stristr( lower, "ragdoll" ) || Q_stristr( lower, "corpse" ) || Q_stristr( lower, "dead" );
}

static void SpawnPropModel( CBasePlayer *pPlayer, const char *model )
{
	if ( !pPlayer || !model || !model[0] )
		return;

	// Runtime spawning happens well after the map's own precache phase, so
	// CBaseEntity::PrecacheModel() (called from prop_physics/prop_ragdoll's
	// Precache()) would otherwise trip the "too late" assert and, in a debug
	// build, pop a message box and crash. Temporarily lift the gate exactly
	// like npc_create does (dlls/ai_concommands.cpp CC_NPC_Create).
	bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	if ( engine )
	{
		int modelIndex = engine->PrecacheModel( model );
		if ( modelIndex <= 0 )
		{
			// Model precache failed (likely table overflow) - fail gracefully
			Warning( "gm_spawn: Failed to precache model '%s' - too many models precached\n", model );
			CBaseEntity::SetAllowPrecache( bAllowPrecache );
			return;
		}
	}

	Vector forward;
	pPlayer->EyeVectors( &forward );
	Vector start = pPlayer->EyePosition();
	Vector end   = start + forward * 80.0f;

	trace_t tr;
	UTIL_TraceLine( start, end, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	Vector spawnPos = tr.endpos + tr.plane.normal * 4.0f;

	const char *classname = LooksLikeRagdoll( model ) ? "prop_ragdoll" : "prop_physics";

	CBaseEntity *pEnt = CreateEntityByName( classname );
	if ( !pEnt )
	{
		CBaseEntity::SetAllowPrecache( bAllowPrecache );
		return;
	}

	pEnt->KeyValue( "model", model );
	DispatchSpawn( pEnt );
	pEnt->Teleport( &spawnPos, &pPlayer->EyeAngles(), NULL );
	pEnt->Activate();

	CBaseEntity::SetAllowPrecache( bAllowPrecache );
}

static void SpawnByClassname( CBasePlayer *pPlayer, const char *classname )
{
	if ( !pPlayer || !classname || !classname[0] )
		return;

	Vector forward;
	pPlayer->EyeVectors( &forward );
	Vector start = pPlayer->EyePosition();
	Vector end   = start + forward * 80.0f;

	trace_t tr;
	UTIL_TraceLine( start, end, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	Vector spawnPos = tr.endpos + tr.plane.normal * 4.0f;

	// See SpawnPropModel() - runtime spawns need the precache gate lifted or
	// DispatchSpawn()'s Precache() call trips the "too late" assert/crash.
	bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	CBaseEntity *pEnt = CreateEntityByName( classname );
	if ( !pEnt )
	{
		CBaseEntity::SetAllowPrecache( bAllowPrecache );
		return;
	}

	DispatchSpawn( pEnt );
	pEnt->Teleport( &spawnPos, &pPlayer->EyeAngles(), NULL );
	pEnt->Activate();

	CBaseEntity::SetAllowPrecache( bAllowPrecache );
}

CON_COMMAND( gm_spawn, "Spawn an entity or model (server-side, gmod parity)" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: gm_spawn <classname | modelpath>\n" );
		return;
	}

	const char *arg = engine->Cmd_Argv( 1 );
	if ( IsLikelyModelPath( arg ) )
	{
		SpawnPropModel( pPlayer, arg );
	}
	else
	{
		SpawnByClassname( pPlayer, arg );
	}
}
