//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Gun Tool - Implementation of Gun tool mode
//          Based on Garry's Mod tool system analysis
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "in_buttons.h"
#include "util.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Forward declarations of tool helpers - Gun mode has no persistent per-weapon
// state (the old CToolGun had no member variables that survived between
// calls), so unlike the constraint-family tools there's no static registry
// here, just free functions.
//-----------------------------------------------------------------------------
static void FireBullet( CWeaponTool *pTool, trace_t &tr );
static void CreateMuzzleFlash( CWeaponTool *pTool );
static void DamageEntity( CBaseEntity *pEntity, const CTakeDamageInfo &info );

//-----------------------------------------------------------------------------
// Tool implementation for Gun mode
//-----------------------------------------------------------------------------
void Tool_Gun_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	if ( !bPrimary )
		return;

	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	FireBullet( pTool, tr );
	CreateMuzzleFlash( pTool );

	if ( pEntity && pEntity != pOwner )
	{
		CTakeDamageInfo info( pTool, pOwner, 25.0f, DMG_BULLET );
		info.SetDamagePosition( tr.endpos );
		info.SetDamageForce( tr.m_pEnt->GetAbsOrigin() - tr.endpos );

		DamageEntity( pEntity, info );
	}

	pTool->PlayToolSound( "Weapon_Pistol.Single" );
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Gun mode - fire into empty space
//-----------------------------------------------------------------------------
void Tool_Gun_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary )
{
	if ( !bPrimary )
		return;

	FireBullet( pTool, tr );
	CreateMuzzleFlash( pTool );
	pTool->PlayToolSound( "Weapon_Pistol.Single" );
}

//-----------------------------------------------------------------------------
// Tool think for Gun mode
//-----------------------------------------------------------------------------
void Tool_Gun_OnThink( CWeaponTool *pTool )
{
	// Gun tool doesn't need continuous thinking
}

//-----------------------------------------------------------------------------
// Fire bullet
//-----------------------------------------------------------------------------
static void FireBullet( CWeaponTool *pTool, trace_t &tr )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	Vector vecStart = pOwner->EyePosition();
	Vector vecEnd = tr.endpos;

	// Create bullet impact effect
	CEffectData data;
	data.m_vOrigin = vecEnd;
	data.m_vNormal = tr.plane.normal;
	data.m_flScale = 1.0f;

	DispatchEffect( "Impact", data );

	// Create bullet tracer if far enough
	float flDistance = vecStart.DistTo( vecEnd );
	if ( flDistance > 100.0f )
	{
		UTIL_Tracer( vecStart, vecEnd, 0, TRACER_TYPE_DEFAULT, 6000, true, "BulletTracer01" );
	}
}

//-----------------------------------------------------------------------------
// Create muzzle flash
//-----------------------------------------------------------------------------
static void CreateMuzzleFlash( CWeaponTool *pTool )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	// Create muzzle flash effect
	CEffectData data;
	data.m_nEntIndex = pTool->entindex();
	data.m_flScale = 1.0f;

	DispatchEffect( "MuzzleFlash", data );
}

//-----------------------------------------------------------------------------
// Damage entity
//-----------------------------------------------------------------------------
static void DamageEntity( CBaseEntity *pEntity, const CTakeDamageInfo &info )
{
	if ( !pEntity )
		return;

	// Apply damage
	pEntity->TakeDamage( info );

	// Add some physics force if it's a physics object
	IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
	if ( pPhysics )
	{
		Vector vecForce = info.GetDamageForce() * 50.0f; // Scale the force
		pPhysics->ApplyForceOffset( vecForce, info.GetDamagePosition() );
	}
}
