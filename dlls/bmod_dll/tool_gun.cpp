//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Gun Tool - Implementation of Gun tool mode
//          Based on Garry's Mod tool system analysis
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "util.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Gun tool class - implements TOOL_GUN mode
//-----------------------------------------------------------------------------
class CToolGun : public CWeaponTool
{
	DECLARE_CLASS( CToolGun, CWeaponTool );

public:
	CToolGun() {}

	virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void OnToolThink();

private:
	void FireBullet( trace_t &tr );
	void CreateMuzzleFlash();
	void DamageEntity( CBaseEntity *pEntity, const CTakeDamageInfo &info );
};

//-----------------------------------------------------------------------------
// Tool implementation for Gun mode
//-----------------------------------------------------------------------------
void CToolGun::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	if ( !bPrimary )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	// Fire bullet
	FireBullet( tr );

	// Create muzzle flash
	CreateMuzzleFlash();

	// Apply damage to entity
	if ( pEntity && pEntity != pOwner )
	{
		CTakeDamageInfo info( this, pOwner, 25.0f, DMG_BULLET );
		info.SetDamagePosition( tr.endpos );
		info.SetDamageForce( tr.m_pEnt->GetAbsOrigin() - tr.endpos );

		DamageEntity( pEntity, info );
	}

	// Play sound
	PlayToolSound( "Weapon_Pistol.Single" );
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Gun mode
//-----------------------------------------------------------------------------
void CToolGun::OnToolTrace( trace_t &tr, bool bPrimary )
{
	if ( !bPrimary )
		return;

	// Fire into empty space
	FireBullet( tr );
	CreateMuzzleFlash();
	PlayToolSound( "Weapon_Pistol.Single" );
}

//-----------------------------------------------------------------------------
// Tool think for Gun mode
//-----------------------------------------------------------------------------
void CToolGun::OnToolThink()
{
	// Gun tool doesn't need continuous thinking
}

//-----------------------------------------------------------------------------
// Fire bullet
//-----------------------------------------------------------------------------
void CToolGun::FireBullet( trace_t &tr )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
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
void CToolGun::CreateMuzzleFlash()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	// Create muzzle flash effect
	CEffectData data;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = LookupAttachment( "muzzle" );
	data.m_flScale = 1.0f;

	DispatchEffect( "MuzzleFlash", data );
}

//-----------------------------------------------------------------------------
// Damage entity
//-----------------------------------------------------------------------------
void CToolGun::DamageEntity( CBaseEntity *pEntity, const CTakeDamageInfo &info )
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