//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Remover Tool - Implementation of entity removal tool
//          Based on Garry's Mod tool system analysis
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "props.h"
#include "physics.h"

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
// Console variables for remover tool
//-----------------------------------------------------------------------------
ConVar bm_remover_mode("bm_remover_mode", "0", FCVAR_ARCHIVE, "Removal mode: 0=single, 1=area, 2=type");
ConVar bm_remover_radius("bm_remover_radius", "128", FCVAR_ARCHIVE, "Removal radius for area mode");
ConVar bm_remover_filter("bm_remover_filter", "", FCVAR_ARCHIVE, "Entity class filter for type mode");
ConVar bm_remover_confirm("bm_remover_confirm", "1", FCVAR_ARCHIVE, "Require confirmation for area/type removal");
ConVar bm_remover_effects("bm_remover_effects", "1", FCVAR_ARCHIVE, "Show removal effects");

//-----------------------------------------------------------------------------
// Removal modes
//-----------------------------------------------------------------------------
enum RemovalMode_t
{
	REMOVE_SINGLE = 0,		// Remove single entity
	REMOVE_AREA,			// Remove all entities in area
	REMOVE_TYPE,			// Remove all entities of specific type
	REMOVE_MAX
};

static const char *g_RemovalModeNames[] =
{
	"Single Entity",
	"Area Removal",
	"Type Removal"
};

//-----------------------------------------------------------------------------
// Per-weapon remover state - CWeaponTool is not subclassed, so the state that
// used to live in CToolRemover's member variables now lives here, keyed by
// the weapon's EHANDLE (mirrors the g_WeldConstraints/g_RopeConstraints
// per-constraint registries used by the sibling tool_*.cpp files).
//-----------------------------------------------------------------------------
struct RemoverState_t
{
	EHANDLE	hTool;
	float	flLastRemoveTime;	// Last removal operation time
	bool	bConfirmPending;	// Waiting for confirmation
	Vector	vecPendingPos;		// Position for pending area removal

	RemoverState_t()
	{
		flLastRemoveTime = 0.0f;
		bConfirmPending = false;
		vecPendingPos = Vector( 0, 0, 0 );
	}
};

static CUtlVector<RemoverState_t*> g_RemoverStates;

static RemoverState_t *FindRemoverState( CWeaponTool *pTool )
{
	for ( int i = 0; i < g_RemoverStates.Count(); i++ )
	{
		if ( g_RemoverStates[i]->hTool == pTool )
			return g_RemoverStates[i];
	}
	return NULL;
}

static RemoverState_t *GetRemoverState( CWeaponTool *pTool )
{
	RemoverState_t *pState = FindRemoverState( pTool );
	if ( !pState )
	{
		pState = new RemoverState_t;
		pState->hTool = pTool;
		g_RemoverStates.AddToTail( pState );
	}
	return pState;
}

// Removes state records for weapons that no longer exist.
static void CleanupRemoverStates()
{
	for ( int i = g_RemoverStates.Count() - 1; i >= 0; i-- )
	{
		if ( !g_RemoverStates[i]->hTool.Get() )
		{
			delete g_RemoverStates[i];
			g_RemoverStates.Remove( i );
		}
	}
}

//-----------------------------------------------------------------------------
// Forward declarations of tool helpers
//-----------------------------------------------------------------------------
static bool RemoveSingleEntity( CBaseEntity *pEntity, CWeaponTool *pTool );
static int RemoveEntitiesInArea( const Vector &vecCenter, float flRadius, CWeaponTool *pTool );
static int RemoveEntitiesOfType( const char *pszClassName, CWeaponTool *pTool );
static bool CanRemoveEntity( CBaseEntity *pEntity, CWeaponTool *pTool );
static void CreateRemovalEffect( const Vector &vecPos );
static void CreateAreaEffect( const Vector &vecCenter, float flRadius );
static void CycleRemovalMode( CWeaponTool *pTool );
static RemovalMode_t GetRemovalMode();

//-----------------------------------------------------------------------------
// Tool implementation for Remover mode
//-----------------------------------------------------------------------------
void Tool_Remover_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	RemoverState_t *pState = GetRemoverState( pTool );

	if ( bPrimary )
	{
		// Primary attack - remove entity/entities
		RemovalMode_t mode = GetRemovalMode();

		switch ( mode )
		{
			case REMOVE_SINGLE:
			{
				if ( pEntity && CanRemoveEntity( pEntity, pTool ) )
				{
					if ( RemoveSingleEntity( pEntity, pTool ) )
					{
						CreateRemovalEffect( tr.endpos );
						pTool->PlayToolSound( "weapons/physcannon/energy_disintegrate4.wav" );
						ClientPrintf( pOwner, HUD_PRINTTALK, "Removed %s", pEntity->GetClassname() );
					}
				}
				else
				{
					ClientPrintf( pOwner, HUD_PRINTTALK, "Cannot remove this entity" );
				}
				break;
			}

			case REMOVE_AREA:
			{
				float flRadius = bm_remover_radius.GetFloat();

				if ( bm_remover_confirm.GetBool() && !pState->bConfirmPending )
				{
					// Show confirmation for area removal
					pState->bConfirmPending = true;
					pState->vecPendingPos = tr.endpos;

					CreateAreaEffect( tr.endpos, flRadius );
					ClientPrintf( pOwner, HUD_PRINTTALK, "Area removal (radius: %.0f)", flRadius );
					ClientPrintf( pOwner, HUD_PRINTTALK, "Click again to confirm, right-click to cancel" );
				}
				else
				{
					// Perform area removal
					int nRemoved = RemoveEntitiesInArea( tr.endpos, flRadius, pTool );
					if ( nRemoved > 0 )
					{
						CreateAreaEffect( tr.endpos, flRadius );
						pTool->PlayToolSound( "weapons/physcannon/energy_disintegrate5.wav" );
						ClientPrintf( pOwner, HUD_PRINTTALK, "Removed %d entities in area", nRemoved );
					}
					else
					{
						ClientPrintf( pOwner, HUD_PRINTTALK, "No removable entities in area" );
					}
					pState->bConfirmPending = false;
				}
				break;
			}

			case REMOVE_TYPE:
			{
				if ( pEntity )
				{
					const char *pszClassName = pEntity->GetClassname();

					if ( bm_remover_confirm.GetBool() && !pState->bConfirmPending )
					{
						// Show confirmation for type removal
						pState->bConfirmPending = true;
						bm_remover_filter.SetValue( pszClassName );

						ClientPrintf( pOwner, HUD_PRINTTALK, "Type removal: %s", pszClassName );
						ClientPrintf( pOwner, HUD_PRINTTALK, "Click again to confirm, right-click to cancel" );
					}
					else
					{
						// Perform type removal
						int nRemoved = RemoveEntitiesOfType( pszClassName, pTool );
						if ( nRemoved > 0 )
						{
							CreateRemovalEffect( tr.endpos );
							pTool->PlayToolSound( "weapons/physcannon/energy_disintegrate5.wav" );
							ClientPrintf( pOwner, HUD_PRINTTALK, "Removed %d entities of type %s", nRemoved, pszClassName );
						}
						else
						{
							ClientPrintf( pOwner, HUD_PRINTTALK, "No removable entities of type %s", pszClassName );
						}
						pState->bConfirmPending = false;
					}
				}
				break;
			}
		}
	}
	else
	{
		// Secondary attack - cycle mode or cancel confirmation
		if ( pState->bConfirmPending )
		{
			// Cancel pending operation
			pState->bConfirmPending = false;
			ClientPrintf( pOwner, HUD_PRINTTALK, "Removal cancelled" );
		}
		else
		{
			// Cycle removal mode
			CycleRemovalMode( pTool );
		}
	}

	pState->flLastRemoveTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Remover mode
//-----------------------------------------------------------------------------
void Tool_Remover_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	RemoverState_t *pState = GetRemoverState( pTool );

	if ( bPrimary )
	{
		RemovalMode_t mode = GetRemovalMode();

		if ( mode == REMOVE_AREA )
		{
			// Area removal can work on empty space
			float flRadius = bm_remover_radius.GetFloat();

			if ( bm_remover_confirm.GetBool() && !pState->bConfirmPending )
			{
				pState->bConfirmPending = true;
				pState->vecPendingPos = tr.endpos;

				CreateAreaEffect( tr.endpos, flRadius );
				ClientPrintf( pOwner, HUD_PRINTTALK, "Area removal (radius: %.0f)", flRadius );
				ClientPrintf( pOwner, HUD_PRINTTALK, "Click again to confirm, right-click to cancel" );
			}
			else
			{
				int nRemoved = RemoveEntitiesInArea( tr.endpos, flRadius, pTool );
				if ( nRemoved > 0 )
				{
					CreateAreaEffect( tr.endpos, flRadius );
					pTool->PlayToolSound( "weapons/physcannon/energy_disintegrate5.wav" );
					ClientPrintf( pOwner, HUD_PRINTTALK, "Removed %d entities in area", nRemoved );
				}
				pState->bConfirmPending = false;
			}
		}
		else
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "No entity targeted" );
		}
	}
	else
	{
		// Cancel or cycle mode
		if ( pState->bConfirmPending )
		{
			pState->bConfirmPending = false;
			ClientPrintf( pOwner, HUD_PRINTTALK, "Removal cancelled" );
		}
		else
		{
			CycleRemovalMode( pTool );
		}
	}
}

//-----------------------------------------------------------------------------
// Tool think for Remover mode
//-----------------------------------------------------------------------------
void Tool_Remover_OnThink( CWeaponTool *pTool )
{
	RemoverState_t *pState = GetRemoverState( pTool );

	// Cancel confirmation after timeout
	if ( pState->bConfirmPending && (gpGlobals->curtime - pState->flLastRemoveTime) > 5.0f )
	{
		pState->bConfirmPending = false;

		CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
		if ( pOwner )
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "Removal confirmation timed out" );
		}
	}

	CleanupRemoverStates();
}

//-----------------------------------------------------------------------------
// Remove single entity
//-----------------------------------------------------------------------------
static bool RemoveSingleEntity( CBaseEntity *pEntity, CWeaponTool *pTool )
{
	if ( !pEntity || !CanRemoveEntity( pEntity, pTool ) )
		return false;

	// Create removal effect before removing
	if ( bm_remover_effects.GetBool() )
	{
		CreateRemovalEffect( pEntity->GetAbsOrigin() );
	}

	// Remove the entity
	UTIL_Remove( pEntity );

	DevMsg( "Removed entity: %s\n", pEntity->GetClassname() );
	return true;
}

//-----------------------------------------------------------------------------
// Remove entities in area
//-----------------------------------------------------------------------------
static int RemoveEntitiesInArea( const Vector &vecCenter, float flRadius, CWeaponTool *pTool )
{
	int nRemoved = 0;
	CUtlVector<CBaseEntity*> entitiesToRemove;

	// Find all removable entities in radius
	CBaseEntity *pEntity = NULL;
	while ( (pEntity = gEntList.FindEntityInSphere( pEntity, vecCenter, flRadius )) != NULL )
	{
		if ( CanRemoveEntity( pEntity, pTool ) )
		{
			entitiesToRemove.AddToTail( pEntity );
		}
	}

	// Remove entities
	for ( int i = 0; i < entitiesToRemove.Count(); i++ )
	{
		CBaseEntity *pEnt = entitiesToRemove[i];
		if ( pEnt )
		{
			if ( bm_remover_effects.GetBool() )
			{
				CreateRemovalEffect( pEnt->GetAbsOrigin() );
			}
			UTIL_Remove( pEnt );
			nRemoved++;
		}
	}

	return nRemoved;
}

//-----------------------------------------------------------------------------
// Remove entities of specific type
//-----------------------------------------------------------------------------
static int RemoveEntitiesOfType( const char *pszClassName, CWeaponTool *pTool )
{
	if ( !pszClassName || !pszClassName[0] )
		return 0;

	int nRemoved = 0;
	CUtlVector<CBaseEntity*> entitiesToRemove;

	// Find all entities of this type
	CBaseEntity *pEntity = NULL;
	while ( (pEntity = gEntList.NextEnt( pEntity )) != NULL )
	{
		if ( FClassnameIs( pEntity, pszClassName ) && CanRemoveEntity( pEntity, pTool ) )
		{
			entitiesToRemove.AddToTail( pEntity );
		}
	}

	// Remove entities
	for ( int i = 0; i < entitiesToRemove.Count(); i++ )
	{
		CBaseEntity *pEnt = entitiesToRemove[i];
		if ( pEnt )
		{
			if ( bm_remover_effects.GetBool() )
			{
				CreateRemovalEffect( pEnt->GetAbsOrigin() );
			}
			UTIL_Remove( pEnt );
			nRemoved++;
		}
	}

	return nRemoved;
}

//-----------------------------------------------------------------------------
// Check if entity can be removed
//-----------------------------------------------------------------------------
static bool CanRemoveEntity( CBaseEntity *pEntity, CWeaponTool *pTool )
{
	if ( !pEntity )
		return false;

	// Can't remove players
	if ( pEntity->IsPlayer() )
		return false;

	// Can't remove world
	if ( pEntity->entindex() == 0 )
		return false;

	// Can't remove essential game entities (spawn points, etc.)
	const char *pszClassName = pEntity->GetClassname();
	if ( !Q_stricmp( pszClassName, "info_player_start" ) ||
		 !Q_stricmp( pszClassName, "info_player_deathmatch" ) ||
		 !Q_stricmp( pszClassName, "info_player_terrorist" ) ||
		 !Q_stricmp( pszClassName, "info_player_counterterrorist" ) ||
		 !Q_stricmp( pszClassName, "worldspawn" ) )
	{
		return false;
	}

	// Check if entity is owned by someone else
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	CBaseEntity *pEntityOwner = pEntity->GetOwnerEntity();
	if ( pEntityOwner && pEntityOwner != pOwner && pEntityOwner->IsPlayer() )
	{
		// Only allow removing your own entities
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Create removal effect
//-----------------------------------------------------------------------------
static void CreateRemovalEffect( const Vector &vecPos )
{
	if ( !bm_remover_effects.GetBool() )
		return;

	// Create disintegration effect
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = 150.0f;
	data.m_flScale = 2.0f;
	data.m_nColor = 255; // Red

	DispatchEffect( "Sparks", data );

	// Create energy dissolve effect
	data.m_flMagnitude = 200.0f;
	data.m_flScale = 3.0f;
	DispatchEffect( "GlowSprite", data );

	// Create particle burst
	for ( int i = 0; i < 6; i++ )
	{
		Vector vecOffset = Vector(
			random->RandomFloat(-30, 30),
			random->RandomFloat(-30, 30),
			random->RandomFloat(-30, 30) );

		data.m_vOrigin = vecPos + vecOffset;
		data.m_flScale = 1.0f;
		DispatchEffect( "Sparks", data );
	}
}

//-----------------------------------------------------------------------------
// Create area effect preview
//-----------------------------------------------------------------------------
static void CreateAreaEffect( const Vector &vecCenter, float flRadius )
{
	// Create area preview effect
	CEffectData data;
	data.m_vOrigin = vecCenter;
	data.m_flMagnitude = flRadius;
	data.m_flScale = 1.0f;
	data.m_nColor = 255; // Red warning

	DispatchEffect( "GlowSprite", data );

	// Create radius indicators
	for ( int i = 0; i < 8; i++ )
	{
		float flAngle = (i * 360.0f / 8.0f) * M_PI / 180.0f;
		Vector vecPos = vecCenter + Vector(
			cos(flAngle) * flRadius,
			sin(flAngle) * flRadius,
			0 );

		data.m_vOrigin = vecPos;
		data.m_flScale = 0.5f;
		DispatchEffect( "Sparks", data );
	}
}

//-----------------------------------------------------------------------------
// Cycle removal mode
//-----------------------------------------------------------------------------
static void CycleRemovalMode( CWeaponTool *pTool )
{
	int nCurrentMode = bm_remover_mode.GetInt();
	nCurrentMode = (nCurrentMode + 1) % REMOVE_MAX;
	bm_remover_mode.SetValue( nCurrentMode );

	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( pOwner )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "Removal mode: %s",
			g_RemovalModeNames[nCurrentMode] );
	}
}

//-----------------------------------------------------------------------------
// Get current removal mode
//-----------------------------------------------------------------------------
static RemovalMode_t GetRemovalMode()
{
	int nMode = bm_remover_mode.GetInt();
	return (RemovalMode_t)clamp( nMode, 0, REMOVE_MAX - 1 );
}

//-----------------------------------------------------------------------------
// Console command for remover context menu
//-----------------------------------------------------------------------------
CON_COMMAND( bm_context_remover, "Opens remover tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has remover tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_REMOVER )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Remover tool must be equipped and selected" );
		return;
	}

	int nMode = bm_remover_mode.GetInt();
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Remover Tool:" );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Current mode: %s", g_RemovalModeNames[nMode] );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Left click: Remove" );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Right click: Change mode" );

	switch ( nMode )
	{
		case REMOVE_SINGLE:
			ClientPrintf( pPlayer, HUD_PRINTTALK, "Mode: Remove single entities" );
			break;

		case REMOVE_AREA:
			ClientPrintf( pPlayer, HUD_PRINTTALK, "Mode: Remove entities in radius %.0f", bm_remover_radius.GetFloat() );
			break;

		case REMOVE_TYPE:
			ClientPrintf( pPlayer, HUD_PRINTTALK, "Mode: Remove all entities of clicked type" );
			if ( bm_remover_filter.GetString()[0] )
			{
				ClientPrintf( pPlayer, HUD_PRINTTALK, "Filter: %s", bm_remover_filter.GetString() );
			}
			break;
	}

	ClientPrintf( pPlayer, HUD_PRINTTALK, "Confirmation: %s", bm_remover_confirm.GetBool() ? "On" : "Off" );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Effects: %s", bm_remover_effects.GetBool() ? "On" : "Off" );
}

//-----------------------------------------------------------------------------
// Console command for emergency cleanup
//-----------------------------------------------------------------------------
CON_COMMAND( bm_remover_cleanup, "Remove all props and physics objects" )
{
	int nRemoved = 0;
	CUtlVector<CBaseEntity*> entitiesToRemove;

	// Find all removable physics objects
	CBaseEntity *pEntity = NULL;
	while ( (pEntity = gEntList.NextEnt( pEntity )) != NULL )
	{
		if ( !pEntity->IsPlayer() && pEntity->entindex() != 0 && pEntity->VPhysicsGetObject() )
		{
			const char *pszClassName = pEntity->GetClassname();
			if ( Q_stristr( pszClassName, "prop_" ) || Q_stristr( pszClassName, "physics_" ) )
			{
				entitiesToRemove.AddToTail( pEntity );
			}
		}
	}

	// Remove entities
	for ( int i = 0; i < entitiesToRemove.Count(); i++ )
	{
		UTIL_Remove( entitiesToRemove[i] );
		nRemoved++;
	}

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Emergency cleanup: Removed %d physics objects", nRemoved );
	}
}
