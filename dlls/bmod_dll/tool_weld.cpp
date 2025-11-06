//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Weld Tool - Implementation of welding constraint tool
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
// Console variables for weld tool
//-----------------------------------------------------------------------------
ConVar bm_weld_strength("bm_weld_strength", "10000", FCVAR_ARCHIVE, "Weld constraint strength");
ConVar bm_weld_forcelimit("bm_weld_forcelimit", "0", FCVAR_ARCHIVE, "Weld force limit (0 = unbreakable)");
ConVar bm_weld_nocollide("bm_weld_nocollide", "1", FCVAR_ARCHIVE, "Disable collision between welded objects");
ConVar bm_weld_freeze("bm_weld_freeze", "0", FCVAR_ARCHIVE, "Freeze objects when welding");

//-----------------------------------------------------------------------------
// Weld information storage
//-----------------------------------------------------------------------------
struct WeldInfo_t
{
	CBaseEntity *pEntity1;
	CBaseEntity *pEntity2;
	Vector vecLocalPos1;
	Vector vecLocalPos2;
	QAngle angLocalAngles1;
	QAngle angLocalAngles2;
	IPhysicsConstraint *pConstraint;
	float flCreateTime;

	WeldInfo_t()
	{
		pEntity1 = NULL;
		pEntity2 = NULL;
		pConstraint = NULL;
		flCreateTime = 0.0f;
		vecLocalPos1 = Vector(0,0,0);
		vecLocalPos2 = Vector(0,0,0);
		angLocalAngles1 = QAngle(0,0,0);
		angLocalAngles2 = QAngle(0,0,0);
	}
};

//-----------------------------------------------------------------------------
// Global weld tracking
//-----------------------------------------------------------------------------
static CUtlVector<WeldInfo_t*> g_WeldConstraints;

//-----------------------------------------------------------------------------
// Weld tool class - implements TOOL_WELD mode
//-----------------------------------------------------------------------------
class CToolWeld : public CWeaponTool
{
	DECLARE_CLASS( CToolWeld, CWeaponTool );

public:
	CToolWeld() : m_pFirstEntity(NULL), m_vecFirstPos(Vector(0,0,0)), m_flFirstTime(0.0f) {}

	virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void OnToolThink();

private:
	bool CreateWeld( CBaseEntity *pEnt1, CBaseEntity *pEnt2, const Vector &vecPos1, const Vector &vecPos2 );
	bool CanWeldEntities( CBaseEntity *pEnt1, CBaseEntity *pEnt2 );
	void RemoveWeld( CBaseEntity *pEntity );
	void CreateWeldEffect( const Vector &vecPos );
	void CreateSelectionEffect( const Vector &vecPos );
	WeldInfo_t *FindWeld( CBaseEntity *pEnt1, CBaseEntity *pEnt2 );
	void CleanupWelds();

	// Weld tool state
	CBaseEntity		*m_pFirstEntity;	// First entity selected for welding
	Vector			m_vecFirstPos;		// Position on first entity
	float			m_flFirstTime;		// Time first entity was selected
};

//-----------------------------------------------------------------------------
// Tool implementation for Weld mode
//-----------------------------------------------------------------------------
void CToolWeld::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		// Primary attack - select entity for welding
		if ( pEntity && pEntity != pOwner && pEntity->VPhysicsGetObject() )
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
				ClientPrint( pOwner, HUD_PRINTTALK, "Click second entity to weld" );
			}
			else if ( pEntity != m_pFirstEntity )
			{
				// Second entity selection - create weld
				if ( CanWeldEntities( m_pFirstEntity, pEntity ) )
				{
					if ( CreateWeld( m_pFirstEntity, pEntity, m_vecFirstPos, tr.endpos ) )
					{
						CreateWeldEffect( tr.endpos );
						PlayToolSound( "weapons/physcannon/energy_sing_loop4.wav" );

						ClientPrint( pOwner, HUD_PRINTTALK, "Welded %s to %s",
							m_pFirstEntity->GetClassname(), pEntity->GetClassname() );
					}
					else
					{
						ClientPrint( pOwner, HUD_PRINTTALK, "Failed to create weld" );
					}
				}
				else
				{
					ClientPrint( pOwner, HUD_PRINTTALK, "Cannot weld these entities together" );
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
		else
		{
			// Invalid entity or world
			if ( m_pFirstEntity )
			{
				m_pFirstEntity = NULL;
				m_vecFirstPos = Vector(0,0,0);
				ClientPrint( pOwner, HUD_PRINTTALK, "Selection cancelled" );
			}
			else
			{
				ClientPrint( pOwner, HUD_PRINTTALK, "No valid entity targeted" );
			}
		}
	}
	else
	{
		// Secondary attack - remove weld
		if ( pEntity && pEntity->VPhysicsGetObject() )
		{
			RemoveWeld( pEntity );
			PlayToolSound( "weapons/physcannon/physcannon_claws_close.wav" );
			ClientPrint( pOwner, HUD_PRINTTALK, "Removed welds from %s", pEntity->GetClassname() );
		}
		else
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "No valid entity to unweld" );
		}
	}
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Weld mode
//-----------------------------------------------------------------------------
void CToolWeld::OnToolTrace( trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		// Cancel selection when clicking empty space
		if ( m_pFirstEntity )
		{
			m_pFirstEntity = NULL;
			m_vecFirstPos = Vector(0,0,0);
			ClientPrint( pOwner, HUD_PRINTTALK, "Selection cancelled" );
		}
	}
}

//-----------------------------------------------------------------------------
// Tool think for Weld mode
//-----------------------------------------------------------------------------
void CToolWeld::OnToolThink()
{
	// Cancel selection after timeout
	if ( m_pFirstEntity && (gpGlobals->curtime - m_flFirstTime) > 10.0f )
	{
		m_pFirstEntity = NULL;
		m_vecFirstPos = Vector(0,0,0);

		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if ( pOwner )
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "Selection timed out" );
		}
	}

	// Clean up invalid welds
	CleanupWelds();
}

//-----------------------------------------------------------------------------
// Create weld constraint between two entities
//-----------------------------------------------------------------------------
bool CToolWeld::CreateWeld( CBaseEntity *pEnt1, CBaseEntity *pEnt2, const Vector &vecPos1, const Vector &vecPos2 )
{
	if ( !pEnt1 || !pEnt2 )
		return false;

	IPhysicsObject *pPhys1 = pEnt1->VPhysicsGetObject();
	IPhysicsObject *pPhys2 = pEnt2->VPhysicsGetObject();

	if ( !pPhys1 || !pPhys2 )
		return false;

	// Check if entities are already welded
	if ( FindWeld( pEnt1, pEnt2 ) )
	{
		DevMsg( "Entities already welded\n" );
		return false;
	}

	// Create constraint
	constraint_fixedparams_t fixed;
	fixed.Defaults();
	fixed.InitWithCurrentObjectState( pPhys1, pPhys2 );
	fixed.constraint.forceLimit = bm_weld_forcelimit.GetFloat();
	fixed.constraint.torqueLimit = bm_weld_forcelimit.GetFloat();

	IPhysicsConstraint *pConstraint = physenv->CreateFixedConstraint( pPhys1, pPhys2, NULL, fixed );
	if ( !pConstraint )
	{
		DevMsg( "Failed to create weld constraint\n" );
		return false;
	}

	// Store weld information
	WeldInfo_t *pWeldInfo = new WeldInfo_t;
	pWeldInfo->pEntity1 = pEnt1;
	pWeldInfo->pEntity2 = pEnt2;
	pWeldInfo->vecLocalPos1 = vecPos1;
	pWeldInfo->vecLocalPos2 = vecPos2;
	pWeldInfo->pConstraint = pConstraint;
	pWeldInfo->flCreateTime = gpGlobals->curtime;

	// Calculate local positions and angles
	VMatrix mat1, mat2;
	pPhys1->GetPositionMatrix( &mat1 );
	pPhys2->GetPositionMatrix( &mat2 );

	VMatrix invMat1 = mat1.InverseTR();
	Vector vecLocal1 = invMat1 * vecPos1;
	pWeldInfo->vecLocalPos1 = vecLocal1;

	VMatrix invMat2 = mat2.InverseTR();
	Vector vecLocal2 = invMat2 * vecPos2;
	pWeldInfo->vecLocalPos2 = vecLocal2;

	g_WeldConstraints.AddToTail( pWeldInfo );

	// Disable collision between entities if requested
	if ( bm_weld_nocollide.GetBool() )
	{
		PhysDisableEntityCollisions( pEnt1, pEnt2 );
	}

	// Freeze entities if requested
	if ( bm_weld_freeze.GetBool() )
	{
		pPhys1->EnableMotion( false );
		pPhys2->EnableMotion( false );
	}

	DevMsg( "Created weld between %s and %s\n", pEnt1->GetClassname(), pEnt2->GetClassname() );
	return true;
}

//-----------------------------------------------------------------------------
// Check if entities can be welded
//-----------------------------------------------------------------------------
bool CToolWeld::CanWeldEntities( CBaseEntity *pEnt1, CBaseEntity *pEnt2 )
{
	if ( !pEnt1 || !pEnt2 )
		return false;

	// Can't weld to self
	if ( pEnt1 == pEnt2 )
		return false;

	// Both entities must have physics
	if ( !pEnt1->VPhysicsGetObject() || !pEnt2->VPhysicsGetObject() )
		return false;

	// Can't weld players
	if ( pEnt1->IsPlayer() || pEnt2->IsPlayer() )
		return false;

	// Can't weld to world (use different constraint type)
	if ( pEnt1->IsWorld() || pEnt2->IsWorld() )
		return false;

	// Check if already welded
	if ( FindWeld( pEnt1, pEnt2 ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Remove welds involving the specified entity
//-----------------------------------------------------------------------------
void CToolWeld::RemoveWeld( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return;

	for ( int i = g_WeldConstraints.Count() - 1; i >= 0; i-- )
	{
		WeldInfo_t *pWeld = g_WeldConstraints[i];

		if ( pWeld->pEntity1 == pEntity || pWeld->pEntity2 == pEntity )
		{
			// Remove the physics constraint
			if ( pWeld->pConstraint )
			{
				physenv->DestroyConstraint( pWeld->pConstraint );
			}

			// Re-enable collision if it was disabled
			if ( bm_weld_nocollide.GetBool() && pWeld->pEntity1 && pWeld->pEntity2 )
			{
				PhysEnableEntityCollisions( pWeld->pEntity1, pWeld->pEntity2 );
			}

			// Remove from list and delete
			g_WeldConstraints.Remove( i );
			delete pWeld;
		}
	}
}

//-----------------------------------------------------------------------------
// Create weld effect
//-----------------------------------------------------------------------------
void CToolWeld::CreateWeldEffect( const Vector &vecPos )
{
	// Create welding spark effect
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = 100.0f;
	data.m_flScale = 2.0f;
	data.m_nColor = 255; // Bright white/yellow

	DispatchEffect( "Sparks", data );

	// Create bright flash
	data.m_flMagnitude = 200.0f;
	data.m_flScale = 3.0f;
	DispatchEffect( "GlowSprite", data );

	// Create secondary sparks
	for ( int i = 0; i < 3; i++ )
	{
		data.m_vOrigin = vecPos + Vector(
			random->RandomFloat(-16, 16),
			random->RandomFloat(-16, 16),
			random->RandomFloat(-16, 16) );
		data.m_flScale = 1.0f;
		DispatchEffect( "Sparks", data );
	}
}

//-----------------------------------------------------------------------------
// Create selection effect
//-----------------------------------------------------------------------------
void CToolWeld::CreateSelectionEffect( const Vector &vecPos )
{
	// Create selection highlight
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = 50.0f;
	data.m_flScale = 1.5f;
	data.m_nColor = 100; // Blue highlight

	DispatchEffect( "GlowSprite", data );
}

//-----------------------------------------------------------------------------
// Find existing weld between entities
//-----------------------------------------------------------------------------
WeldInfo_t *CToolWeld::FindWeld( CBaseEntity *pEnt1, CBaseEntity *pEnt2 )
{
	for ( int i = 0; i < g_WeldConstraints.Count(); i++ )
	{
		WeldInfo_t *pWeld = g_WeldConstraints[i];

		if ( (pWeld->pEntity1 == pEnt1 && pWeld->pEntity2 == pEnt2) ||
			 (pWeld->pEntity1 == pEnt2 && pWeld->pEntity2 == pEnt1) )
		{
			return pWeld;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Clean up invalid welds
//-----------------------------------------------------------------------------
void CToolWeld::CleanupWelds()
{
	for ( int i = g_WeldConstraints.Count() - 1; i >= 0; i-- )
	{
		WeldInfo_t *pWeld = g_WeldConstraints[i];

		// Check if entities are still valid
		if ( !pWeld->pEntity1 || !pWeld->pEntity2 ||
			 !pWeld->pEntity1->VPhysicsGetObject() ||
			 !pWeld->pEntity2->VPhysicsGetObject() )
		{
			// Clean up constraint
			if ( pWeld->pConstraint )
			{
				physenv->DestroyConstraint( pWeld->pConstraint );
			}

			g_WeldConstraints.Remove( i );
			delete pWeld;
		}
	}
}

//-----------------------------------------------------------------------------
// Console command for weld context menu
//-----------------------------------------------------------------------------
CON_COMMAND( bm_context_weld, "Opens weld tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has weld tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_WELD )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Weld tool must be equipped and selected" );
		return;
	}

	ClientPrint( pPlayer, HUD_PRINTTALK, "Weld Tool:" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Left click: Select entities to weld" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Right click: Remove welds" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Strength: %.0f", bm_weld_strength.GetFloat() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Force limit: %.0f", bm_weld_forcelimit.GetFloat() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "No collide: %s", bm_weld_nocollide.GetBool() ? "Yes" : "No" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Auto freeze: %s", bm_weld_freeze.GetBool() ? "Yes" : "No" );
}

//-----------------------------------------------------------------------------
// Console command to remove all welds
//-----------------------------------------------------------------------------
CON_COMMAND( bm_weld_removeall, "Remove all weld constraints" )
{
	int nRemoved = 0;

	for ( int i = g_WeldConstraints.Count() - 1; i >= 0; i-- )
	{
		WeldInfo_t *pWeld = g_WeldConstraints[i];

		if ( pWeld->pConstraint )
		{
			physenv->DestroyConstraint( pWeld->pConstraint );
		}

		// Re-enable collision if it was disabled
		if ( bm_weld_nocollide.GetBool() && pWeld->pEntity1 && pWeld->pEntity2 )
		{
			PhysEnableEntityCollisions( pWeld->pEntity1, pWeld->pEntity2 );
		}

		delete pWeld;
		nRemoved++;
	}

	g_WeldConstraints.RemoveAll();

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Removed %d weld constraints", nRemoved );
	}
}

//-----------------------------------------------------------------------------
// Console command to list all welds
//-----------------------------------------------------------------------------
CON_COMMAND( bm_weld_list, "List all weld constraints" )
{
	Msg( "Active weld constraints: %d\n", g_WeldConstraints.Count() );

	for ( int i = 0; i < g_WeldConstraints.Count(); i++ )
	{
		WeldInfo_t *pWeld = g_WeldConstraints[i];

		Msg( "%d. %s <-> %s (Age: %.1fs)\n",
			i + 1,
			pWeld->pEntity1 ? pWeld->pEntity1->GetClassname() : "NULL",
			pWeld->pEntity2 ? pWeld->pEntity2->GetClassname() : "NULL",
			gpGlobals->curtime - pWeld->flCreateTime );
	}
}