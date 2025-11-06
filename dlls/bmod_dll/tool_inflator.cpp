//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Inflator Tool - Implementation of entity resizing tool
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
#include "bone_setup.h"
#include "studio.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables for inflator tool
//-----------------------------------------------------------------------------
ConVar bm_inflator_scale("bm_inflator_scale", "1.5", FCVAR_ARCHIVE, "Scale multiplier for inflation");
ConVar bm_inflator_min("bm_inflator_min", "0.1", FCVAR_ARCHIVE, "Minimum scale factor");
ConVar bm_inflator_max("bm_inflator_max", "10.0", FCVAR_ARCHIVE, "Maximum scale factor");
ConVar bm_inflator_physics("bm_inflator_physics", "1", FCVAR_ARCHIVE, "Scale physics objects");
ConVar bm_inflator_mass("bm_inflator_mass", "1", FCVAR_ARCHIVE, "Scale mass with size");

//-----------------------------------------------------------------------------
// Entity scale information storage
//-----------------------------------------------------------------------------
struct ScaleInfo_t
{
	CBaseEntity *pEntity;
	float flOriginalScale;
	float flCurrentScale;
	float flOriginalMass;
	Vector vecOriginalSize;

	ScaleInfo_t()
	{
		pEntity = NULL;
		flOriginalScale = 1.0f;
		flCurrentScale = 1.0f;
		flOriginalMass = 0.0f;
		vecOriginalSize = Vector(0,0,0);
	}
};

//-----------------------------------------------------------------------------
// Global scale tracking
//-----------------------------------------------------------------------------
static CUtlVector<ScaleInfo_t*> g_ScaledEntities;

//-----------------------------------------------------------------------------
// Inflator tool class - implements TOOL_INFLATOR mode
//-----------------------------------------------------------------------------
class CToolInflator : public CWeaponTool
{
	DECLARE_CLASS( CToolInflator, CWeaponTool );

public:
	CToolInflator() : m_flLastInflateTime(0.0f) {}

	virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void OnToolThink();

private:
	bool InflateEntity( CBaseEntity *pEntity, float flScaleFactor );
	bool DeflateEntity( CBaseEntity *pEntity, float flScaleFactor );
	bool ResetEntityScale( CBaseEntity *pEntity );
	bool CanInflateEntity( CBaseEntity *pEntity );
	void CreateInflateEffect( const Vector &vecPos, bool bInflate );
	ScaleInfo_t *FindScaleInfo( CBaseEntity *pEntity );
	ScaleInfo_t *GetOrCreateScaleInfo( CBaseEntity *pEntity );
	void CleanupScaleInfo();
	float GetEntityScale( CBaseEntity *pEntity );
	void SetEntityScale( CBaseEntity *pEntity, float flScale );

	// Inflator tool state
	float m_flLastInflateTime;		// Last inflate operation time
};

//-----------------------------------------------------------------------------
// Tool implementation for Inflator mode
//-----------------------------------------------------------------------------
void CToolInflator::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( !pEntity || pEntity == pOwner )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "No valid entity targeted" );
		return;
	}

	if ( !CanInflateEntity( pEntity ) )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Cannot resize %s", pEntity->GetClassname() );
		return;
	}

	if ( bPrimary )
	{
		// Primary attack - inflate (make bigger)
		float flScaleFactor = bm_inflator_scale.GetFloat();

		if ( pOwner->m_nButtons & IN_USE )
		{
			// Use + primary = reset to original size
			if ( ResetEntityScale( pEntity ) )
			{
				CreateInflateEffect( tr.endpos, false );
				PlayToolSound( "garrysmod/balloon_pop_cute.wav" );
				ClientPrint( pOwner, HUD_PRINTTALK, "Reset %s to original size", pEntity->GetClassname() );
			}
		}
		else
		{
			// Just primary = inflate
			if ( InflateEntity( pEntity, flScaleFactor ) )
			{
				CreateInflateEffect( tr.endpos, true );
				PlayToolSound( "weapons/physcannon/energy_sing_loop4.wav" );

				float flCurrentScale = GetEntityScale( pEntity );
				ClientPrint( pOwner, HUD_PRINTTALK, "Inflated %s (Scale: %.2fx)",
					pEntity->GetClassname(), flCurrentScale );
			}
		}
	}
	else
	{
		// Secondary attack - deflate (make smaller)
		float flScaleFactor = 1.0f / bm_inflator_scale.GetFloat();

		if ( DeflateEntity( pEntity, flScaleFactor ) )
		{
			CreateInflateEffect( tr.endpos, false );
			PlayToolSound( "weapons/physcannon/energy_bounce1.wav" );

			float flCurrentScale = GetEntityScale( pEntity );
			ClientPrint( pOwner, HUD_PRINTTALK, "Deflated %s (Scale: %.2fx)",
				pEntity->GetClassname(), flCurrentScale );
		}
	}

	m_flLastInflateTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Inflator mode
//-----------------------------------------------------------------------------
void CToolInflator::OnToolTrace( trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary || !bPrimary )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Inflator tool can only be applied to entities" );
	}
}

//-----------------------------------------------------------------------------
// Tool think for Inflator mode
//-----------------------------------------------------------------------------
void CToolInflator::OnToolThink()
{
	// Clean up invalid scale info
	CleanupScaleInfo();
}

//-----------------------------------------------------------------------------
// Inflate entity (make bigger)
//-----------------------------------------------------------------------------
bool CToolInflator::InflateEntity( CBaseEntity *pEntity, float flScaleFactor )
{
	if ( !pEntity || flScaleFactor <= 0.0f )
		return false;

	ScaleInfo_t *pScaleInfo = GetOrCreateScaleInfo( pEntity );
	if ( !pScaleInfo )
		return false;

	// Calculate new scale
	float flNewScale = pScaleInfo->flCurrentScale * flScaleFactor;
	float flMaxScale = bm_inflator_max.GetFloat();

	if ( flNewScale > flMaxScale )
	{
		flNewScale = flMaxScale;
	}

	// Apply new scale
	SetEntityScale( pEntity, flNewScale );
	pScaleInfo->flCurrentScale = flNewScale;

	// Scale physics if enabled
	if ( bm_inflator_physics.GetBool() )
	{
		IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
		if ( pPhysics && bm_inflator_mass.GetBool() )
		{
			// Scale mass with volume (scale^3)
			float flMassScale = flNewScale * flNewScale * flNewScale;
			float flNewMass = pScaleInfo->flOriginalMass * flMassScale;
			pPhysics->SetMass( flNewMass );
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Deflate entity (make smaller)
//-----------------------------------------------------------------------------
bool CToolInflator::DeflateEntity( CBaseEntity *pEntity, float flScaleFactor )
{
	if ( !pEntity || flScaleFactor <= 0.0f )
		return false;

	ScaleInfo_t *pScaleInfo = GetOrCreateScaleInfo( pEntity );
	if ( !pScaleInfo )
		return false;

	// Calculate new scale
	float flNewScale = pScaleInfo->flCurrentScale * flScaleFactor;
	float flMinScale = bm_inflator_min.GetFloat();

	if ( flNewScale < flMinScale )
	{
		flNewScale = flMinScale;
	}

	// Apply new scale
	SetEntityScale( pEntity, flNewScale );
	pScaleInfo->flCurrentScale = flNewScale;

	// Scale physics if enabled
	if ( bm_inflator_physics.GetBool() )
	{
		IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
		if ( pPhysics && bm_inflator_mass.GetBool() )
		{
			// Scale mass with volume (scale^3)
			float flMassScale = flNewScale * flNewScale * flNewScale;
			float flNewMass = pScaleInfo->flOriginalMass * flMassScale;
			pPhysics->SetMass( flNewMass );
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Reset entity to original scale
//-----------------------------------------------------------------------------
bool CToolInflator::ResetEntityScale( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return false;

	ScaleInfo_t *pScaleInfo = FindScaleInfo( pEntity );
	if ( !pScaleInfo )
		return false; // Entity was never scaled

	// Reset to original scale
	SetEntityScale( pEntity, pScaleInfo->flOriginalScale );
	pScaleInfo->flCurrentScale = pScaleInfo->flOriginalScale;

	// Reset physics
	if ( bm_inflator_physics.GetBool() )
	{
		IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
		if ( pPhysics && bm_inflator_mass.GetBool() )
		{
			pPhysics->SetMass( pScaleInfo->flOriginalMass );
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Check if entity can be inflated
//-----------------------------------------------------------------------------
bool CToolInflator::CanInflateEntity( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return false;

	// Can't inflate players
	if ( pEntity->IsPlayer() )
		return false;

	// Can't inflate world
	if ( pEntity->IsWorld() )
		return false;

	// Entity must be a prop or have a model
	if ( !pEntity->GetModel() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Create inflate effect
//-----------------------------------------------------------------------------
void CToolInflator::CreateInflateEffect( const Vector &vecPos, bool bInflate )
{
	// Create inflation/deflation effect
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = bInflate ? 100.0f : 50.0f;
	data.m_flScale = bInflate ? 2.5f : 1.0f;
	data.m_nColor = bInflate ? 50 : 200; // Green for inflate, blue for deflate

	DispatchEffect( "GlowSprite", data );

	// Create expanding/contracting particles
	for ( int i = 0; i < (bInflate ? 5 : 3); i++ )
	{
		Vector vecOffset = Vector(
			random->RandomFloat(-20, 20),
			random->RandomFloat(-20, 20),
			random->RandomFloat(-20, 20) );

		data.m_vOrigin = vecPos + vecOffset;
		data.m_flScale = bInflate ? 1.5f : 0.5f;
		DispatchEffect( "Sparks", data );
	}
}

//-----------------------------------------------------------------------------
// Find scale info for entity
//-----------------------------------------------------------------------------
ScaleInfo_t *CToolInflator::FindScaleInfo( CBaseEntity *pEntity )
{
	for ( int i = 0; i < g_ScaledEntities.Count(); i++ )
	{
		if ( g_ScaledEntities[i]->pEntity == pEntity )
		{
			return g_ScaledEntities[i];
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Get or create scale info for entity
//-----------------------------------------------------------------------------
ScaleInfo_t *CToolInflator::GetOrCreateScaleInfo( CBaseEntity *pEntity )
{
	ScaleInfo_t *pInfo = FindScaleInfo( pEntity );
	if ( pInfo )
		return pInfo;

	// Create new scale info
	pInfo = new ScaleInfo_t;
	pInfo->pEntity = pEntity;
	pInfo->flOriginalScale = GetEntityScale( pEntity );
	pInfo->flCurrentScale = pInfo->flOriginalScale;

	// Store original physics properties
	IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
	if ( pPhysics )
	{
		pInfo->flOriginalMass = pPhysics->GetMass();
	}

	g_ScaledEntities.AddToTail( pInfo );
	return pInfo;
}

//-----------------------------------------------------------------------------
// Clean up invalid scale info
//-----------------------------------------------------------------------------
void CToolInflator::CleanupScaleInfo()
{
	for ( int i = g_ScaledEntities.Count() - 1; i >= 0; i-- )
	{
		ScaleInfo_t *pInfo = g_ScaledEntities[i];

		if ( !pInfo->pEntity )
		{
			g_ScaledEntities.Remove( i );
			delete pInfo;
		}
	}
}

//-----------------------------------------------------------------------------
// Get entity's current scale
//-----------------------------------------------------------------------------
float CToolInflator::GetEntityScale( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return 1.0f;

	// For older SDK compatibility - model scale is not directly available
	// Check if we have stored scale info
	ScaleInfo_t *pInfo = FindScaleInfo( pEntity );
	if ( pInfo )
	{
		return pInfo->flCurrentScale;
	}

	// Default scale
	return 1.0f;
}

//-----------------------------------------------------------------------------
// Set entity's scale
//-----------------------------------------------------------------------------
void CToolInflator::SetEntityScale( CBaseEntity *pEntity, float flScale )
{
	if ( !pEntity )
		return;

	// Clamp scale to limits
	flScale = clamp( flScale, bm_inflator_min.GetFloat(), bm_inflator_max.GetFloat() );

	// For older SDK compatibility - model scale functions don't exist
	// Store scale in keyvalue for potential custom rendering system
	char szScale[32];
	Q_snprintf( szScale, sizeof(szScale), "%.3f", flScale );
	pEntity->SetKeyValue( "model_scale", szScale );

	// Note: Visual scaling would require custom implementation in older SDK
	// This could be done through:
	// 1. Model replacement with pre-scaled models
	// 2. Custom rendering override system
	// 3. Physics object scaling (if physics exists)

	// Scale physics collision if enabled
	if ( bm_inflator_physics.GetBool() )
	{
		IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
		if ( pPhysics )
		{
			// In a full implementation, this would recreate the physics object
			// with the new scale. For now, just note the limitation.
			DevMsg( "Note: Physics collision scaling not fully implemented\n" );
		}
	}

	DevMsg( "Set entity %s scale to %.2f\n", pEntity->GetClassname(), flScale );
}

//-----------------------------------------------------------------------------
// Console command for inflator context menu
//-----------------------------------------------------------------------------
CON_COMMAND( bm_context_inflator, "Opens inflator tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has inflator tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_INFLATOR )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Inflator tool must be equipped and selected" );
		return;
	}

	ClientPrint( pPlayer, HUD_PRINTTALK, "Inflator Tool:" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Left click: Inflate (make bigger)" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Right click: Deflate (make smaller)" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Use + Left click: Reset to original size" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Scale factor: %.2fx", bm_inflator_scale.GetFloat() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Scale range: %.1f - %.1f", bm_inflator_min.GetFloat(), bm_inflator_max.GetFloat() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Scale physics: %s", bm_inflator_physics.GetBool() ? "Yes" : "No" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Scale mass: %s", bm_inflator_mass.GetBool() ? "Yes" : "No" );
}

//-----------------------------------------------------------------------------
// Console command to reset all entity scales
//-----------------------------------------------------------------------------
CON_COMMAND( bm_inflator_resetall, "Reset all entity scales to original" )
{
	int nReset = 0;

	for ( int i = 0; i < g_ScaledEntities.Count(); i++ )
	{
		ScaleInfo_t *pInfo = g_ScaledEntities[i];

		if ( pInfo->pEntity )
		{
			// Reset to original scale (older SDK compatible)
			char szScale[32];
			Q_snprintf( szScale, sizeof(szScale), "%.3f", pInfo->flOriginalScale );
			pInfo->pEntity->SetKeyValue( "model_scale", szScale );

			// Reset physics
			IPhysicsObject *pPhysics = pInfo->pEntity->VPhysicsGetObject();
			if ( pPhysics )
			{
				pPhysics->SetMass( pInfo->flOriginalMass );
			}

			nReset++;
		}
	}

	// Clear all scale info
	for ( int i = 0; i < g_ScaledEntities.Count(); i++ )
	{
		delete g_ScaledEntities[i];
	}
	g_ScaledEntities.RemoveAll();

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Reset %d entities to original scale", nReset );
	}
}