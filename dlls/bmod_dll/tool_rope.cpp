//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Rope Tool - Implementation of rope constraint tool
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
#include "physics_saverestore.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables for rope tool
//-----------------------------------------------------------------------------
ConVar bm_rope_length("bm_rope_length", "200", FCVAR_ARCHIVE, "Rope length in units");
ConVar bm_rope_width("bm_rope_width", "2", FCVAR_ARCHIVE, "Rope visual width");
ConVar bm_rope_material("bm_rope_material", "cable/rope", FCVAR_ARCHIVE, "Rope material");
ConVar bm_rope_segments("bm_rope_segments", "8", FCVAR_ARCHIVE, "Number of rope segments");
ConVar bm_rope_rigid("bm_rope_rigid", "0", FCVAR_ARCHIVE, "Create rigid rope constraint");

//-----------------------------------------------------------------------------
// Rope information storage
//-----------------------------------------------------------------------------
struct RopeInfo_t
{
	CBaseEntity *pEntity1;
	CBaseEntity *pEntity2;
	Vector vecPos1;
	Vector vecPos2;
	float flLength;
	IPhysicsConstraint *pConstraint;
	CBaseEntity *pRopeEntity;		// Visual rope entity
	float flCreateTime;

	RopeInfo_t()
	{
		pEntity1 = NULL;
		pEntity2 = NULL;
		pConstraint = NULL;
		pRopeEntity = NULL;
		flLength = 0.0f;
		flCreateTime = 0.0f;
		vecPos1 = Vector(0,0,0);
		vecPos2 = Vector(0,0,0);
	}
};

//-----------------------------------------------------------------------------
// Global rope tracking
//-----------------------------------------------------------------------------
static CUtlVector<RopeInfo_t*> g_RopeConstraints;

//-----------------------------------------------------------------------------
// Rope tool class - implements TOOL_ROPE mode
//-----------------------------------------------------------------------------
class CToolRope : public CWeaponTool
{
	DECLARE_CLASS( CToolRope, CWeaponTool );

public:
	CToolRope() : m_pFirstEntity(NULL), m_vecFirstPos(Vector(0,0,0)), m_flFirstTime(0.0f) {}

	virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void OnToolThink();

private:
	bool CreateRope( CBaseEntity *pEnt1, CBaseEntity *pEnt2, const Vector &vecPos1, const Vector &vecPos2 );
	bool CanRopeEntities( CBaseEntity *pEnt1, CBaseEntity *pEnt2 );
	void RemoveRope( CBaseEntity *pEntity );
	void CreateRopeEffect( const Vector &vecStart, const Vector &vecEnd );
	void CreateSelectionEffect( const Vector &vecPos );
	RopeInfo_t *FindRope( CBaseEntity *pEnt1, CBaseEntity *pEnt2 );
	void CleanupRopes();
	CBaseEntity *CreateVisualRope( const Vector &vecStart, const Vector &vecEnd );

	// Rope tool state
	CBaseEntity		*m_pFirstEntity;	// First entity selected for roping
	Vector			m_vecFirstPos;		// Position on first entity
	float			m_flFirstTime;		// Time first entity was selected
};

//-----------------------------------------------------------------------------
// Tool implementation for Rope mode
//-----------------------------------------------------------------------------
void CToolRope::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		// Primary attack - select entity for roping
		if ( pEntity && pEntity != pOwner )
		{
			if ( !m_pFirstEntity )
			{
				// First entity selection
				m_pFirstEntity = pEntity;
				m_vecFirstPos = tr.endpos;
				m_flFirstTime = gpGlobals->curtime;

				CreateSelectionEffect( tr.endpos );
				PlayToolSound( "buttons/button14.wav" );

				ClientPrint( pOwner, HUD_PRINTTALK, "Selected first entity: %s", pEntity->GetClassname() );
				ClientPrint( pOwner, HUD_PRINTTALK, "Click second entity to create rope" );
			}
			else if ( pEntity != m_pFirstEntity )
			{
				// Second entity selection - create rope
				if ( CanRopeEntities( m_pFirstEntity, pEntity ) )
				{
					if ( CreateRope( m_pFirstEntity, pEntity, m_vecFirstPos, tr.endpos ) )
					{
						CreateRopeEffect( m_vecFirstPos, tr.endpos );
						PlayToolSound( "physics/wood/wood_strain2.wav" );

						float flDistance = m_vecFirstPos.DistTo( tr.endpos );
						ClientPrint( pOwner, HUD_PRINTTALK, "Created rope: %.1f units",
							flDistance );
					}
					else
					{
						ClientPrint( pOwner, HUD_PRINTTALK, "Failed to create rope" );
					}
				}
				else
				{
					ClientPrint( pOwner, HUD_PRINTTALK, "Cannot rope these entities together" );
				}

				// Reset selection
				m_pFirstEntity = NULL;
				m_vecFirstPos = Vector(0,0,0);
			}
			else
			{
				// Same entity clicked twice - reset
				m_pFirstEntity = NULL;
				m_vecFirstPos = Vector(0,0,0);
				ClientPrint( pOwner, HUD_PRINTTALK, "Selection cancelled" );
			}
		}
		else if ( !pEntity )
		{
			// Clicked on world - can create rope to world
			if ( m_pFirstEntity )
			{
				if ( CreateRope( m_pFirstEntity, NULL, m_vecFirstPos, tr.endpos ) )
				{
					CreateRopeEffect( m_vecFirstPos, tr.endpos );
					PlayToolSound( "physics/wood/wood_strain2.wav" );

					float flDistance = m_vecFirstPos.DistTo( tr.endpos );
					ClientPrint( pOwner, HUD_PRINTTALK, "Created rope to world: %.1f units",
						flDistance );
				}

				m_pFirstEntity = NULL;
				m_vecFirstPos = Vector(0,0,0);
			}
			else
			{
				ClientPrint( pOwner, HUD_PRINTTALK, "No valid entity targeted" );
			}
		}
	}
	else
	{
		// Secondary attack - remove rope
		if ( pEntity )
		{
			RemoveRope( pEntity );
			PlayToolSound( "physics/wood/wood_crate_break1.wav" );
			ClientPrint( pOwner, HUD_PRINTTALK, "Removed ropes from %s", pEntity->GetClassname() );
		}
	}
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Rope mode
//-----------------------------------------------------------------------------
void CToolRope::OnToolTrace( trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		// Can create rope to world
		if ( m_pFirstEntity )
		{
			if ( CreateRope( m_pFirstEntity, NULL, m_vecFirstPos, tr.endpos ) )
			{
				CreateRopeEffect( m_vecFirstPos, tr.endpos );
				PlayToolSound( "physics/wood/wood_strain2.wav" );

				float flDistance = m_vecFirstPos.DistTo( tr.endpos );
				ClientPrint( pOwner, HUD_PRINTTALK, "Created rope to world: %.1f units",
					flDistance );
			}

			m_pFirstEntity = NULL;
			m_vecFirstPos = Vector(0,0,0);
		}
	}
}

//-----------------------------------------------------------------------------
// Tool think for Rope mode
//-----------------------------------------------------------------------------
void CToolRope::OnToolThink()
{
	// Cancel selection after timeout
	if ( m_pFirstEntity && (gpGlobals->curtime - m_flFirstTime) > 10.0f )
	{
		m_pFirstEntity = NULL;
		m_vecFirstPos = Vector(0,0,0);

		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if ( pOwner )
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "Rope selection timed out" );
		}
	}

	// Clean up invalid ropes
	CleanupRopes();
}

//-----------------------------------------------------------------------------
// Create rope constraint between entities
//-----------------------------------------------------------------------------
bool CToolRope::CreateRope( CBaseEntity *pEnt1, CBaseEntity *pEnt2, const Vector &vecPos1, const Vector &vecPos2 )
{
	if ( !pEnt1 )
		return false;

	IPhysicsObject *pPhys1 = pEnt1->VPhysicsGetObject();
	if ( !pPhys1 )
		return false;

	IPhysicsObject *pPhys2 = NULL;
	if ( pEnt2 )
	{
		pPhys2 = pEnt2->VPhysicsGetObject();
		if ( !pPhys2 )
			return false;
	}

	// Calculate rope length
	float flLength = vecPos1.DistTo( vecPos2 );
	float flMaxLength = bm_rope_length.GetFloat();

	if ( flLength > flMaxLength )
	{
		flLength = flMaxLength;
	}

	// Create constraint
	IPhysicsConstraint *pConstraint = NULL;

	if ( bm_rope_rigid.GetBool() )
	{
		// Create fixed distance constraint (rigid rope)
		constraint_lengthparams_t length;
		length.Defaults();
		length.InitWorldspace( pPhys1, pPhys2, vecPos1, vecPos2 );
		length.constraint.forceLimit = 0; // Unbreakable
		length.constraint.torqueLimit = 0;
		length.minLength = flLength;
		length.maxLength = flLength;

		pConstraint = physenv->CreateLengthConstraint( pPhys1, pPhys2, NULL, length );
	}
	else
	{
		// Create rope constraint (flexible)
		constraint_ropeParams_t rope;
		rope.Defaults();
		rope.InitWorldspace( pPhys1, pPhys2, vecPos1, vecPos2 );
		rope.constraint.forceLimit = 0; // Unbreakable
		rope.constraint.torqueLimit = 0;
		rope.totalLength = flLength;

		pConstraint = physenv->CreateRopeConstraint( pPhys1, pPhys2, NULL, rope );
	}

	if ( !pConstraint )
	{
		DevMsg( "Failed to create rope constraint\n" );
		return false;
	}

	// Create visual rope
	CBaseEntity *pRopeEntity = CreateVisualRope( vecPos1, vecPos2 );

	// Store rope information
	RopeInfo_t *pRopeInfo = new RopeInfo_t;
	pRopeInfo->pEntity1 = pEnt1;
	pRopeInfo->pEntity2 = pEnt2;
	pRopeInfo->vecPos1 = vecPos1;
	pRopeInfo->vecPos2 = vecPos2;
	pRopeInfo->flLength = flLength;
	pRopeInfo->pConstraint = pConstraint;
	pRopeInfo->pRopeEntity = pRopeEntity;
	pRopeInfo->flCreateTime = gpGlobals->curtime;

	g_RopeConstraints.AddToTail( pRopeInfo );

	DevMsg( "Created rope between %s and %s (Length: %.1f)\n",
		pEnt1->GetClassname(),
		pEnt2 ? pEnt2->GetClassname() : "world",
		flLength );

	return true;
}

//-----------------------------------------------------------------------------
// Check if entities can be roped
//-----------------------------------------------------------------------------
bool CToolRope::CanRopeEntities( CBaseEntity *pEnt1, CBaseEntity *pEnt2 )
{
	if ( !pEnt1 )
		return false;

	// Can't rope to self
	if ( pEnt1 == pEnt2 )
		return false;

	// First entity must have physics
	if ( !pEnt1->VPhysicsGetObject() )
		return false;

	// Second entity can be NULL (world) or must have physics
	if ( pEnt2 && !pEnt2->VPhysicsGetObject() )
		return false;

	// Can't rope players directly
	if ( pEnt1->IsPlayer() || (pEnt2 && pEnt2->IsPlayer()) )
		return false;

	// Check if already roped
	if ( FindRope( pEnt1, pEnt2 ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Remove ropes involving the specified entity
//-----------------------------------------------------------------------------
void CToolRope::RemoveRope( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return;

	for ( int i = g_RopeConstraints.Count() - 1; i >= 0; i-- )
	{
		RopeInfo_t *pRope = g_RopeConstraints[i];

		if ( pRope->pEntity1 == pEntity || pRope->pEntity2 == pEntity )
		{
			// Remove the physics constraint
			if ( pRope->pConstraint )
			{
				physenv->DestroyConstraint( pRope->pConstraint );
			}

			// Remove visual rope
			if ( pRope->pRopeEntity )
			{
				UTIL_Remove( pRope->pRopeEntity );
			}

			// Remove from list and delete
			g_RopeConstraints.Remove( i );
			delete pRope;
		}
	}
}

//-----------------------------------------------------------------------------
// Create visual rope effect
//-----------------------------------------------------------------------------
void CToolRope::CreateRopeEffect( const Vector &vecStart, const Vector &vecEnd )
{
	// Create rope placement effect
	CEffectData data;
	data.m_vOrigin = vecStart;
	data.m_vStart = vecStart;
	data.m_vAngles = (vecEnd - vecStart).Normalized();
	data.m_flMagnitude = vecStart.DistTo( vecEnd );
	data.m_flScale = 1.0f;

	DispatchEffect( "Sparks", data );

	// Create endpoint effects
	data.m_vOrigin = vecStart;
	data.m_flScale = 0.5f;
	DispatchEffect( "GlowSprite", data );

	data.m_vOrigin = vecEnd;
	DispatchEffect( "GlowSprite", data );
}

//-----------------------------------------------------------------------------
// Create selection effect
//-----------------------------------------------------------------------------
void CToolRope::CreateSelectionEffect( const Vector &vecPos )
{
	// Create selection highlight
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = 30.0f;
	data.m_flScale = 1.0f;
	data.m_nColor = 150; // Yellow

	DispatchEffect( "GlowSprite", data );
}

//-----------------------------------------------------------------------------
// Find existing rope between entities
//-----------------------------------------------------------------------------
RopeInfo_t *CToolRope::FindRope( CBaseEntity *pEnt1, CBaseEntity *pEnt2 )
{
	for ( int i = 0; i < g_RopeConstraints.Count(); i++ )
	{
		RopeInfo_t *pRope = g_RopeConstraints[i];

		if ( (pRope->pEntity1 == pEnt1 && pRope->pEntity2 == pEnt2) ||
			 (pRope->pEntity1 == pEnt2 && pRope->pEntity2 == pEnt1) )
		{
			return pRope;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Create visual rope entity
//-----------------------------------------------------------------------------
CBaseEntity *CToolRope::CreateVisualRope( const Vector &vecStart, const Vector &vecEnd )
{
	// In a full implementation, this would create a rope entity with keyframe rope
	// For now, just return NULL (constraint works without visual)
	return NULL;
}

//-----------------------------------------------------------------------------
// Clean up invalid ropes
//-----------------------------------------------------------------------------
void CToolRope::CleanupRopes()
{
	for ( int i = g_RopeConstraints.Count() - 1; i >= 0; i-- )
	{
		RopeInfo_t *pRope = g_RopeConstraints[i];

		// Check if entities are still valid
		bool bInvalid = false;

		if ( !pRope->pEntity1 || !pRope->pEntity1->VPhysicsGetObject() )
			bInvalid = true;

		if ( pRope->pEntity2 && (!pRope->pEntity2 || !pRope->pEntity2->VPhysicsGetObject()) )
			bInvalid = true;

		if ( bInvalid )
		{
			// Clean up constraint
			if ( pRope->pConstraint )
			{
				physenv->DestroyConstraint( pRope->pConstraint );
			}

			if ( pRope->pRopeEntity )
			{
				UTIL_Remove( pRope->pRopeEntity );
			}

			g_RopeConstraints.Remove( i );
			delete pRope;
		}
	}
}

//-----------------------------------------------------------------------------
// Console command for rope context menu
//-----------------------------------------------------------------------------
CON_COMMAND( bm_context_rope, "Opens rope tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has rope tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_ROPE )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Rope tool must be equipped and selected" );
		return;
	}

	ClientPrint( pPlayer, HUD_PRINTTALK, "Rope Tool:" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Left click: Select entities to rope" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Right click: Remove ropes" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Length: %.0f units", bm_rope_length.GetFloat() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Width: %.0f", bm_rope_width.GetFloat() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Material: %s", bm_rope_material.GetString() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Rigid: %s", bm_rope_rigid.GetBool() ? "Yes" : "No" );
}

//-----------------------------------------------------------------------------
// Console command to remove all ropes
//-----------------------------------------------------------------------------
CON_COMMAND( bm_rope_removeall, "Remove all rope constraints" )
{
	int nRemoved = 0;

	for ( int i = g_RopeConstraints.Count() - 1; i >= 0; i-- )
	{
		RopeInfo_t *pRope = g_RopeConstraints[i];

		if ( pRope->pConstraint )
		{
			physenv->DestroyConstraint( pRope->pConstraint );
		}

		if ( pRope->pRopeEntity )
		{
			UTIL_Remove( pRope->pRopeEntity );
		}

		delete pRope;
		nRemoved++;
	}

	g_RopeConstraints.RemoveAll();

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Removed %d rope constraints", nRemoved );
	}
}