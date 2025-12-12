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

	if ( engine )
	{
		engine->PrecacheModel( model );
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
		return;

	pEnt->KeyValue( "model", model );
	DispatchSpawn( pEnt );
	pEnt->Teleport( &spawnPos, &pPlayer->EyeAngles(), NULL );
	pEnt->Activate();
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

	CBaseEntity *pEnt = CreateEntityByName( classname );
	if ( !pEnt )
		return;

	DispatchSpawn( pEnt );
	pEnt->Teleport( &spawnPos, &pPlayer->EyeAngles(), NULL );
	pEnt->Activate();
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
