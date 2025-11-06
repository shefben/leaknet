//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Duplicator Tool - Implementation of entity copying tool
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
#include "vphysics/constraints.h"
#include "keyvalues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables for duplicator tool
//-----------------------------------------------------------------------------
ConVar bm_duplicator_limit("bm_duplicator_limit", "50", FCVAR_ARCHIVE, "Maximum entities per duplication");
ConVar bm_duplicator_delay("bm_duplicator_delay", "0.1", FCVAR_ARCHIVE, "Delay between entity spawns during duplication");
ConVar bm_duplicator_constraints("bm_duplicator_constraints", "1", FCVAR_ARCHIVE, "Copy constraints with entities");

//-----------------------------------------------------------------------------
// Entity information storage for copying
//-----------------------------------------------------------------------------
struct DupeEntityInfo_t
{
	char szClassName[64];
	char szModel[256];
	Vector vecPosition;
	QAngle angAngles;
	Vector vecVelocity;
	QAngle angVelocity;
	int nHealth;
	int nMaxHealth;
	float flMass;
	int nMaterial;
	int nSkin;
	int nBodyGroup;
	KeyValues *pKeyValues;

	// Rendering info
	int nRenderMode;
	int nRenderFX;
	int r, g, b, a;
	char szMaterialOverride[256];

	// Physics info
	bool bMotionEnabled;
	bool bCollisionEnabled;
	float flLinearDamping;
	float flAngularDamping;

	DupeEntityInfo_t()
	{
		pKeyValues = NULL;
		szMaterialOverride[0] = '\0';
	}

	~DupeEntityInfo_t()
	{
		if ( pKeyValues )
		{
			pKeyValues->deleteThis();
		}
	}
};

//-----------------------------------------------------------------------------
// Constraint information for duplication
//-----------------------------------------------------------------------------
struct DupeConstraintInfo_t
{
	int nType;					// Constraint type
	int nEntity1Index;			// Index of first entity in dupe list
	int nEntity2Index;			// Index of second entity (-1 for world)
	Vector vecPos1;				// Position on entity 1
	Vector vecPos2;				// Position on entity 2
	QAngle angConstraint;		// Constraint angles
	float flLength;				// Length (for rope, etc.)
	float flStrength;			// Constraint strength
	bool bCollisionEnabled;		// Collision between constrained entities
};

//-----------------------------------------------------------------------------
// Duplication data storage
//-----------------------------------------------------------------------------
struct DuplicationData_t
{
	CUtlVector<DupeEntityInfo_t*> m_Entities;
	CUtlVector<DupeConstraintInfo_t> m_Constraints;
	Vector m_vecCenterPoint;		// Center point of original selection
	int m_nTotalEntities;
	bool m_bValid;

	DuplicationData_t()
	{
		m_vecCenterPoint = Vector(0, 0, 0);
		m_nTotalEntities = 0;
		m_bValid = false;
	}

	~DuplicationData_t()
	{
		Clear();
	}

	void Clear()
	{
		for ( int i = 0; i < m_Entities.Count(); i++ )
		{
			delete m_Entities[i];
		}
		m_Entities.RemoveAll();
		m_Constraints.RemoveAll();
		m_bValid = false;
	}
};

//-----------------------------------------------------------------------------
// Duplicator tool class - implements TOOL_DUPLICATOR mode
//-----------------------------------------------------------------------------
class CToolDuplicator : public CWeaponTool
{
	DECLARE_CLASS( CToolDuplicator, CWeaponTool );

public:
	CToolDuplicator() : m_flLastCopyTime(0.0f), m_flLastPasteTime(0.0f) {}
	virtual ~CToolDuplicator() { m_DupeData.Clear(); }

	virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void OnToolThink();

private:
	bool CopyEntity( CBaseEntity *pEntity );
	bool CopyArea( const Vector &vecCenter, float flRadius );
	bool PasteEntities( const Vector &vecPastePos );
	void CreateCopyEffect( const Vector &vecPos );
	void CreatePasteEffect( const Vector &vecPos );
	void SaveEntityInfo( CBaseEntity *pEntity, DupeEntityInfo_t *pInfo );
	CBaseEntity *CreateEntityFromInfo( DupeEntityInfo_t *pInfo, const Vector &vecOffset );
	void FindConstraints( CBaseEntity *pEntity, const Vector &vecCenter, float flRadius );
	void RecreateConstraints( const CUtlVector<CBaseEntity*> &newEntities );
	bool CanCopy();
	bool CanPaste();

	// Duplicator state
	DuplicationData_t m_DupeData;		// Stored duplication data
	float m_flLastCopyTime;				// Last copy operation time
	float m_flLastPasteTime;			// Last paste operation time
};

//-----------------------------------------------------------------------------
// Tool implementation for Duplicator mode
//-----------------------------------------------------------------------------
void CToolDuplicator::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		// Primary attack - copy entity or area
		if ( pEntity && pEntity != pOwner )
		{
			if ( CanCopy() )
			{
				if ( pOwner->m_nButtons & IN_USE )
				{
					// Use + primary = copy area around entity
					if ( CopyArea( pEntity->GetAbsOrigin(), 256.0f ) )
					{
						CreateCopyEffect( pEntity->GetAbsOrigin() );
						PlayToolSound( "garrysmod/save_sound1.wav" );
						ClientPrint( pOwner, HUD_PRINTTALK, "Copied area (%d entities)", m_DupeData.m_nTotalEntities );
					}
				}
				else
				{
					// Just primary = copy single entity
					if ( CopyEntity( pEntity ) )
					{
						CreateCopyEffect( pEntity->GetAbsOrigin() );
						PlayToolSound( "garrysmod/save_sound1.wav" );
						ClientPrint( pOwner, HUD_PRINTTALK, "Copied %s", pEntity->GetClassname() );
					}
				}
				m_flLastCopyTime = gpGlobals->curtime;
			}
		}
		else
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "No valid entity to copy" );
		}
	}
	else
	{
		// Secondary attack - paste entities
		if ( CanPaste() && m_DupeData.m_bValid )
		{
			if ( PasteEntities( tr.endpos ) )
			{
				CreatePasteEffect( tr.endpos );
				PlayToolSound( "garrysmod/save_sound2.wav" );
				ClientPrint( pOwner, HUD_PRINTTALK, "Pasted %d entities", m_DupeData.m_nTotalEntities );
			}
			else
			{
				ClientPrint( pOwner, HUD_PRINTTALK, "Failed to paste entities" );
			}
			m_flLastPasteTime = gpGlobals->curtime;
		}
		else if ( !m_DupeData.m_bValid )
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "No entities copied yet" );
		}
	}
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Duplicator mode
//-----------------------------------------------------------------------------
void CToolDuplicator::OnToolTrace( trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( !bPrimary )
	{
		// Secondary attack - paste at traced position
		if ( CanPaste() && m_DupeData.m_bValid )
		{
			if ( PasteEntities( tr.endpos ) )
			{
				CreatePasteEffect( tr.endpos );
				PlayToolSound( "garrysmod/save_sound2.wav" );
				ClientPrint( pOwner, HUD_PRINTTALK, "Pasted %d entities", m_DupeData.m_nTotalEntities );
			}
			m_flLastPasteTime = gpGlobals->curtime;
		}
	}
}

//-----------------------------------------------------------------------------
// Tool think for Duplicator mode
//-----------------------------------------------------------------------------
void CToolDuplicator::OnToolThink()
{
	// Duplicator tool doesn't need continuous thinking
}

//-----------------------------------------------------------------------------
// Copy single entity
//-----------------------------------------------------------------------------
bool CToolDuplicator::CopyEntity( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return false;

	// Clear previous data
	m_DupeData.Clear();

	// Create new entity info
	DupeEntityInfo_t *pInfo = new DupeEntityInfo_t;
	SaveEntityInfo( pEntity, pInfo );

	m_DupeData.m_Entities.AddToTail( pInfo );
	m_DupeData.m_vecCenterPoint = pEntity->GetAbsOrigin();
	m_DupeData.m_nTotalEntities = 1;
	m_DupeData.m_bValid = true;

	return true;
}

//-----------------------------------------------------------------------------
// Copy area of entities
//-----------------------------------------------------------------------------
bool CToolDuplicator::CopyArea( const Vector &vecCenter, float flRadius )
{
	// Clear previous data
	m_DupeData.Clear();

	// Find all entities in radius
	CBaseEntity *pEntity = NULL;
	int nEntityCount = 0;
	Vector vecBounds = Vector( flRadius, flRadius, flRadius );

	while ( (pEntity = gEntList.FindEntityInSphere( pEntity, vecCenter, flRadius )) != NULL )
	{
		// Skip invalid entities
		if ( !pEntity || pEntity->IsPlayer() || pEntity->IsWorld() )
			continue;

		// Skip entities without physics
		if ( !pEntity->VPhysicsGetObject() )
			continue;

		// Check entity limit
		if ( nEntityCount >= bm_duplicator_limit.GetInt() )
		{
			DevMsg( "Duplicator: Hit entity limit (%d)\n", bm_duplicator_limit.GetInt() );
			break;
		}

		// Save entity info
		DupeEntityInfo_t *pInfo = new DupeEntityInfo_t;
		SaveEntityInfo( pEntity, pInfo );
		m_DupeData.m_Entities.AddToTail( pInfo );

		nEntityCount++;
	}

	if ( nEntityCount > 0 )
	{
		m_DupeData.m_vecCenterPoint = vecCenter;
		m_DupeData.m_nTotalEntities = nEntityCount;
		m_DupeData.m_bValid = true;

		// Find constraints if enabled
		if ( bm_duplicator_constraints.GetBool() )
		{
			FindConstraints( NULL, vecCenter, flRadius );
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Paste entities at new location
//-----------------------------------------------------------------------------
bool CToolDuplicator::PasteEntities( const Vector &vecPastePos )
{
	if ( !m_DupeData.m_bValid || m_DupeData.m_Entities.Count() == 0 )
		return false;

	// Calculate offset from original center to paste position
	Vector vecOffset = vecPastePos - m_DupeData.m_vecCenterPoint;

	// Create all entities
	CUtlVector<CBaseEntity*> newEntities;
	for ( int i = 0; i < m_DupeData.m_Entities.Count(); i++ )
	{
		CBaseEntity *pNewEntity = CreateEntityFromInfo( m_DupeData.m_Entities[i], vecOffset );
		if ( pNewEntity )
		{
			newEntities.AddToTail( pNewEntity );
		}
		else
		{
			newEntities.AddToTail( NULL );
		}

		// Add delay between spawns to prevent lag
		if ( bm_duplicator_delay.GetFloat() > 0.0f && i < m_DupeData.m_Entities.Count() - 1 )
		{
			// In a full implementation, this would use a timer system
			// For now, just continue spawning
		}
	}

	// Recreate constraints
	if ( bm_duplicator_constraints.GetBool() )
	{
		RecreateConstraints( newEntities );
	}

	return newEntities.Count() > 0;
}

//-----------------------------------------------------------------------------
// Save entity information for duplication
//-----------------------------------------------------------------------------
void CToolDuplicator::SaveEntityInfo( CBaseEntity *pEntity, DupeEntityInfo_t *pInfo )
{
	if ( !pEntity || !pInfo )
		return;

	// Basic entity info
	Q_strncpy( pInfo->szClassName, pEntity->GetClassname(), sizeof(pInfo->szClassName) );

	const model_t *pModel = pEntity->GetModel();
	if ( pModel )
	{
		Q_strncpy( pInfo->szModel, modelinfo->GetModelName( pModel ), sizeof(pInfo->szModel) );
	}

	pInfo->vecPosition = pEntity->GetAbsOrigin();
	pInfo->angAngles = pEntity->GetAbsAngles();
	pInfo->nHealth = pEntity->GetHealth();
	pInfo->nMaxHealth = pEntity->GetMaxHealth();

	// Rendering info
	pEntity->GetRenderColor( pInfo->r, pInfo->g, pInfo->b, pInfo->a );
	pInfo->nRenderMode = pEntity->GetRenderMode();
	pInfo->nRenderFX = pEntity->GetRenderFX();

	// Physics info
	IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
	if ( pPhysics )
	{
		pPhysics->GetVelocity( &pInfo->vecVelocity, &pInfo->angVelocity );
		pInfo->flMass = pPhysics->GetMass();
		pInfo->bMotionEnabled = pPhysics->IsMotionEnabled();
		pInfo->bCollisionEnabled = pPhysics->IsCollisionEnabled();
		pInfo->flLinearDamping = pPhysics->GetDamping( NULL, &pInfo->flAngularDamping );
	}

	// Additional properties for specific entity types
	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating*>( pEntity );
	if ( pAnimating )
	{
		pInfo->nSkin = pAnimating->GetSkin();
		pInfo->nBodyGroup = pAnimating->GetBody();
	}

	DevMsg( "Saved entity info: %s at (%f,%f,%f)\n",
		pInfo->szClassName, pInfo->vecPosition.x, pInfo->vecPosition.y, pInfo->vecPosition.z );
}

//-----------------------------------------------------------------------------
// Create entity from saved info
//-----------------------------------------------------------------------------
CBaseEntity *CToolDuplicator::CreateEntityFromInfo( DupeEntityInfo_t *pInfo, const Vector &vecOffset )
{
	if ( !pInfo )
		return NULL;

	// Create the entity
	CBaseEntity *pEntity = CreateEntityByName( pInfo->szClassName );
	if ( !pEntity )
	{
		DevMsg( "Failed to create entity: %s\n", pInfo->szClassName );
		return NULL;
	}

	// Set basic properties
	pEntity->SetAbsOrigin( pInfo->vecPosition + vecOffset );
	pEntity->SetAbsAngles( pInfo->angAngles );

	if ( pInfo->szModel[0] )
	{
		pEntity->SetModel( pInfo->szModel );
	}

	// Set owner for cleanup
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		pEntity->SetOwnerEntity( pOwner );
	}

	// Spawn the entity
	pEntity->Spawn();
	pEntity->Activate();

	// Restore rendering properties
	pEntity->SetRenderColor( pInfo->r, pInfo->g, pInfo->b, pInfo->a );
	pEntity->SetRenderMode( (RenderMode_t)pInfo->nRenderMode );
	pEntity->SetRenderFX( (RenderFx_t)pInfo->nRenderFX );

	// Restore physics properties
	IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
	if ( pPhysics && pInfo->flMass > 0 )
	{
		pPhysics->SetMass( pInfo->flMass );
		pPhysics->SetVelocity( &pInfo->vecVelocity, &pInfo->angVelocity );
		pPhysics->EnableMotion( pInfo->bMotionEnabled );
		pPhysics->EnableCollisions( pInfo->bCollisionEnabled );
		pPhysics->SetDamping( &pInfo->flLinearDamping, &pInfo->flAngularDamping );
	}

	// Restore additional properties
	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating*>( pEntity );
	if ( pAnimating )
	{
		pAnimating->SetSkin( pInfo->nSkin );
		pAnimating->SetBody( pInfo->nBodyGroup );
	}

	DevMsg( "Created entity: %s at (%f,%f,%f)\n",
		pInfo->szClassName,
		(pInfo->vecPosition + vecOffset).x,
		(pInfo->vecPosition + vecOffset).y,
		(pInfo->vecPosition + vecOffset).z );

	return pEntity;
}

//-----------------------------------------------------------------------------
// Find constraints in area (placeholder)
//-----------------------------------------------------------------------------
void CToolDuplicator::FindConstraints( CBaseEntity *pEntity, const Vector &vecCenter, float flRadius )
{
	// This would scan for constraints between entities in the copy area
	// Implementation would require access to constraint system
	DevMsg( "Finding constraints in area (not fully implemented)\n" );
}

//-----------------------------------------------------------------------------
// Recreate constraints (placeholder)
//-----------------------------------------------------------------------------
void CToolDuplicator::RecreateConstraints( const CUtlVector<CBaseEntity*> &newEntities )
{
	// This would recreate constraints between the newly created entities
	// Implementation would require constraint creation system
	DevMsg( "Recreating constraints (not fully implemented)\n" );
}

//-----------------------------------------------------------------------------
// Create copy effect
//-----------------------------------------------------------------------------
void CToolDuplicator::CreateCopyEffect( const Vector &vecPos )
{
	// Create scanning effect
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = 100.0f;
	data.m_flScale = 3.0f;
	data.m_nColor = 100; // Blue

	DispatchEffect( "GlowSprite", data );

	// Create scan lines
	for ( int i = 0; i < 4; i++ )
	{
		data.m_vOrigin = vecPos + Vector( 0, 0, i * 20 );
		data.m_flScale = 1.0f;
		DispatchEffect( "Sparks", data );
	}
}

//-----------------------------------------------------------------------------
// Create paste effect
//-----------------------------------------------------------------------------
void CToolDuplicator::CreatePasteEffect( const Vector &vecPos )
{
	// Create materialization effect
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = 150.0f;
	data.m_flScale = 4.0f;
	data.m_nColor = 50; // Green

	DispatchEffect( "TeleportSplash", data );

	// Create sparkles
	data.m_vOrigin = vecPos + Vector( 0, 0, 30 );
	data.m_flScale = 2.0f;
	DispatchEffect( "GlowSprite", data );
}

//-----------------------------------------------------------------------------
// Check if we can copy (rate limiting)
//-----------------------------------------------------------------------------
bool CToolDuplicator::CanCopy()
{
	return (gpGlobals->curtime - m_flLastCopyTime) >= 1.0f;
}

//-----------------------------------------------------------------------------
// Check if we can paste (rate limiting)
//-----------------------------------------------------------------------------
bool CToolDuplicator::CanPaste()
{
	return (gpGlobals->curtime - m_flLastPasteTime) >= 2.0f;
}

//-----------------------------------------------------------------------------
// Console command for duplicator context menu
//-----------------------------------------------------------------------------
CON_COMMAND( bm_context_duplicator, "Opens duplicator tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has duplicator tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_DUPLICATOR )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Duplicator tool must be equipped and selected" );
		return;
	}

	ClientPrint( pPlayer, HUD_PRINTTALK, "Duplicator Tool:" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Left click: Copy single entity" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Use + Left click: Copy area" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Right click: Paste entities" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Entity limit: %d", bm_duplicator_limit.GetInt() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Copy constraints: %s", bm_duplicator_constraints.GetBool() ? "Yes" : "No" );
}