//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Remover Tool - Implementation of entity removal tool
//          Based on Garry's Mod tool system analysis
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "props.h"
#include "physics.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

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
// Remover tool class - implements TOOL_REMOVER mode
//-----------------------------------------------------------------------------
class CToolRemover : public CWeaponTool
{
	DECLARE_CLASS( CToolRemover, CWeaponTool );

public:
	CToolRemover() : m_flLastRemoveTime(0.0f), m_bConfirmPending(false), m_vecPendingPos(Vector(0,0,0)) {}

	virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void OnToolThink();

private:
	bool RemoveSingleEntity( CBaseEntity *pEntity );
	int RemoveEntitiesInArea( const Vector &vecCenter, float flRadius );
	int RemoveEntitiesOfType( const char *pszClassName );
	bool CanRemoveEntity( CBaseEntity *pEntity );
	void CreateRemovalEffect( const Vector &vecPos );
	void CreateAreaEffect( const Vector &vecCenter, float flRadius );
	void CycleRemovalMode();
	RemovalMode_t GetRemovalMode();
	void ProcessConfirmation( bool bConfirm );

	// Remover tool state
	float m_flLastRemoveTime;		// Last removal operation time
	bool m_bConfirmPending;			// Waiting for confirmation
	Vector m_vecPendingPos;			// Position for pending area removal
};

//-----------------------------------------------------------------------------
// Tool implementation for Remover mode
//-----------------------------------------------------------------------------
void CToolRemover::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		// Primary attack - remove entity/entities
		RemovalMode_t mode = GetRemovalMode();

		switch ( mode )
		{
			case REMOVE_SINGLE:
			{
				if ( pEntity && CanRemoveEntity( pEntity ) )
				{
					if ( RemoveSingleEntity( pEntity ) )
					{
						CreateRemovalEffect( tr.endpos );
						PlayToolSound( "weapons/physcannon/energy_disintegrate4.wav" );
						ClientPrint( pOwner, HUD_PRINTTALK, "Removed %s", pEntity->GetClassname() );
					}
				}
				else
				{
					ClientPrint( pOwner, HUD_PRINTTALK, "Cannot remove this entity" );
				}
				break;
			}

			case REMOVE_AREA:
			{
				float flRadius = bm_remover_radius.GetFloat();

				if ( bm_remover_confirm.GetBool() && !m_bConfirmPending )
				{
					// Show confirmation for area removal
					m_bConfirmPending = true;
					m_vecPendingPos = tr.endpos;

					CreateAreaEffect( tr.endpos, flRadius );
					ClientPrint( pOwner, HUD_PRINTTALK, "Area removal (radius: %.0f)", flRadius );
					ClientPrint( pOwner, HUD_PRINTTALK, "Click again to confirm, right-click to cancel" );
				}
				else
				{
					// Perform area removal
					int nRemoved = RemoveEntitiesInArea( tr.endpos, flRadius );
					if ( nRemoved > 0 )
					{
						CreateAreaEffect( tr.endpos, flRadius );
						PlayToolSound( "weapons/physcannon/energy_disintegrate5.wav" );
						ClientPrint( pOwner, HUD_PRINTTALK, "Removed %d entities in area", nRemoved );
					}
					else
					{
						ClientPrint( pOwner, HUD_PRINTTALK, "No removable entities in area" );
					}
					m_bConfirmPending = false;
				}
				break;
			}

			case REMOVE_TYPE:
			{
				if ( pEntity )
				{
					const char *pszClassName = pEntity->GetClassname();

					if ( bm_remover_confirm.GetBool() && !m_bConfirmPending )
					{
						// Show confirmation for type removal
						m_bConfirmPending = true;
						bm_remover_filter.SetValue( pszClassName );

						ClientPrint( pOwner, HUD_PRINTTALK, "Type removal: %s", pszClassName );
						ClientPrint( pOwner, HUD_PRINTTALK, "Click again to confirm, right-click to cancel" );
					}
					else
					{
						// Perform type removal
						int nRemoved = RemoveEntitiesOfType( pszClassName );
						if ( nRemoved > 0 )
						{
							CreateRemovalEffect( tr.endpos );
							PlayToolSound( "weapons/physcannon/energy_disintegrate5.wav" );
							ClientPrint( pOwner, HUD_PRINTTALK, "Removed %d entities of type %s", nRemoved, pszClassName );
						}
						else
						{
							ClientPrint( pOwner, HUD_PRINTTALK, "No removable entities of type %s", pszClassName );
						}
						m_bConfirmPending = false;
					}
				}
				break;
			}
		}
	}
	else
	{
		// Secondary attack - cycle mode or cancel confirmation
		if ( m_bConfirmPending )
		{
			// Cancel pending operation
			m_bConfirmPending = false;
			ClientPrint( pOwner, HUD_PRINTTALK, "Removal cancelled" );
		}
		else
		{
			// Cycle removal mode
			CycleRemovalMode();
		}
	}

	m_flLastRemoveTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Remover mode
//-----------------------------------------------------------------------------
void CToolRemover::OnToolTrace( trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		RemovalMode_t mode = GetRemovalMode();

		if ( mode == REMOVE_AREA )
		{
			// Area removal can work on empty space
			float flRadius = bm_remover_radius.GetFloat();

			if ( bm_remover_confirm.GetBool() && !m_bConfirmPending )
			{
				m_bConfirmPending = true;
				m_vecPendingPos = tr.endpos;

				CreateAreaEffect( tr.endpos, flRadius );
				ClientPrint( pOwner, HUD_PRINTTALK, "Area removal (radius: %.0f)", flRadius );
				ClientPrint( pOwner, HUD_PRINTTALK, "Click again to confirm, right-click to cancel" );
			}
			else
			{
				int nRemoved = RemoveEntitiesInArea( tr.endpos, flRadius );
				if ( nRemoved > 0 )
				{
					CreateAreaEffect( tr.endpos, flRadius );
					PlayToolSound( "weapons/physcannon/energy_disintegrate5.wav" );
					ClientPrint( pOwner, HUD_PRINTTALK, "Removed %d entities in area", nRemoved );
				}
				m_bConfirmPending = false;
			}
		}
		else
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "No entity targeted" );
		}
	}
	else
	{
		// Cancel or cycle mode
		if ( m_bConfirmPending )
		{
			m_bConfirmPending = false;
			ClientPrint( pOwner, HUD_PRINTTALK, "Removal cancelled" );
		}
		else
		{
			CycleRemovalMode();
		}
	}
}

//-----------------------------------------------------------------------------
// Tool think for Remover mode
//-----------------------------------------------------------------------------
void CToolRemover::OnToolThink()
{
	// Cancel confirmation after timeout
	if ( m_bConfirmPending && (gpGlobals->curtime - m_flLastRemoveTime) > 5.0f )
	{
		m_bConfirmPending = false;

		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if ( pOwner )
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "Removal confirmation timed out" );
		}
	}
}

//-----------------------------------------------------------------------------
// Remove single entity
//-----------------------------------------------------------------------------
bool CToolRemover::RemoveSingleEntity( CBaseEntity *pEntity )
{
	if ( !pEntity || !CanRemoveEntity( pEntity ) )
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
int CToolRemover::RemoveEntitiesInArea( const Vector &vecCenter, float flRadius )
{
	int nRemoved = 0;
	CUtlVector<CBaseEntity*> entitiesToRemove;

	// Find all removable entities in radius
	CBaseEntity *pEntity = NULL;
	while ( (pEntity = gEntList.FindEntityInSphere( pEntity, vecCenter, flRadius )) != NULL )
	{
		if ( CanRemoveEntity( pEntity ) )
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
int CToolRemover::RemoveEntitiesOfType( const char *pszClassName )
{
	if ( !pszClassName || !pszClassName[0] )
		return 0;

	int nRemoved = 0;
	CUtlVector<CBaseEntity*> entitiesToRemove;

	// Find all entities of this type
	CBaseEntity *pEntity = NULL;
	while ( (pEntity = gEntList.NextEnt( pEntity )) != NULL )
	{
		if ( FClassnameIs( pEntity, pszClassName ) && CanRemoveEntity( pEntity ) )
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
bool CToolRemover::CanRemoveEntity( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return false;

	// Can't remove players
	if ( pEntity->IsPlayer() )
		return false;

	// Can't remove world
	if ( pEntity->IsWorld() )
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
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
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
void CToolRemover::CreateRemovalEffect( const Vector &vecPos )
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
void CToolRemover::CreateAreaEffect( const Vector &vecCenter, float flRadius )
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
void CToolRemover::CycleRemovalMode()
{
	int nCurrentMode = bm_remover_mode.GetInt();
	nCurrentMode = (nCurrentMode + 1) % REMOVE_MAX;
	bm_remover_mode.SetValue( nCurrentMode );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Removal mode: %s",
			g_RemovalModeNames[nCurrentMode] );
	}
}

//-----------------------------------------------------------------------------
// Get current removal mode
//-----------------------------------------------------------------------------
RemovalMode_t CToolRemover::GetRemovalMode()
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
		ClientPrint( pPlayer, HUD_PRINTTALK, "Remover tool must be equipped and selected" );
		return;
	}

	int nMode = bm_remover_mode.GetInt();
	ClientPrint( pPlayer, HUD_PRINTTALK, "Remover Tool:" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Current mode: %s", g_RemovalModeNames[nMode] );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Left click: Remove" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Right click: Change mode" );

	switch ( nMode )
	{
		case REMOVE_SINGLE:
			ClientPrint( pPlayer, HUD_PRINTTALK, "Mode: Remove single entities" );
			break;

		case REMOVE_AREA:
			ClientPrint( pPlayer, HUD_PRINTTALK, "Mode: Remove entities in radius %.0f", bm_remover_radius.GetFloat() );
			break;

		case REMOVE_TYPE:
			ClientPrint( pPlayer, HUD_PRINTTALK, "Mode: Remove all entities of clicked type" );
			if ( bm_remover_filter.GetString()[0] )
			{
				ClientPrint( pPlayer, HUD_PRINTTALK, "Filter: %s", bm_remover_filter.GetString() );
			}
			break;
	}

	ClientPrint( pPlayer, HUD_PRINTTALK, "Confirmation: %s", bm_remover_confirm.GetBool() ? "On" : "Off" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Effects: %s", bm_remover_effects.GetBool() ? "On" : "Off" );
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
		if ( !pEntity->IsPlayer() && !pEntity->IsWorld() && pEntity->VPhysicsGetObject() )
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
		ClientPrint( pPlayer, HUD_PRINTTALK, "Emergency cleanup: Removed %d physics objects", nRemoved );
	}
}