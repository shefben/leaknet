//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Attach Tool Family - RTCamera(21) Thruster(23)
//          PhysProps(24) Balloon(26) Emitter(27) Sprite(28) Wheel(29).
//
//          All of these are "click a prop, attach/apply a decorator to it"
//          tools. They are dispatched here as free functions (declared in
//          tool_dispatch.h) rather than as CWeaponTool subclasses, since
//          "weapon_tool" is the only entity class for every tool mode.
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "physics.h"
#include "vphysics/constraints.h"
#include "gmod_balloon.h"
#include "Point_Camera.h"

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
// Console variables
//-----------------------------------------------------------------------------
ConVar bm_thruster_force( "bm_thruster_force", "2000", FCVAR_ARCHIVE, "Continuous force (in kg*in/s^2) the Thruster tool applies to attached props" );

// "0 means don't change" for mass, "-1 means don't change" for friction/elasticity,
// matching the existing "sentinel value" convention used elsewhere in this codebase.
ConVar bm_physprops_mass( "bm_physprops_mass", "0", FCVAR_ARCHIVE, "Mass to force onto the targeted prop with the PhysProps tool (0 = don't change)" );
ConVar bm_physprops_friction( "bm_physprops_friction", "-1", FCVAR_ARCHIVE, "Friction to force onto the targeted prop with the PhysProps tool (-1 = don't change, currently unsupported)" );
ConVar bm_physprops_elasticity( "bm_physprops_elasticity", "-1", FCVAR_ARCHIVE, "Elasticity to force onto the targeted prop with the PhysProps tool (-1 = don't change, currently unsupported)" );

//-----------------------------------------------------------------------------
// Thruster tool state - just a list of props that get a continuous upward
// force applied every server frame. There's no dedicated thruster entity in
// this codebase yet, so it's tracked here instead.
//-----------------------------------------------------------------------------
static CUtlVector<EHANDLE> g_ThrusterEntities;

static int Attach_FindThruster( CBaseEntity *pEntity )
{
	for ( int i = 0; i < g_ThrusterEntities.Count(); i++ )
	{
		if ( g_ThrusterEntities[i].Get() == pEntity )
			return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Wheel tool state - hinge constraints created between two clicked props.
//-----------------------------------------------------------------------------
struct WheelHingeInfo_t
{
	EHANDLE				hEntity1;
	EHANDLE				hEntity2;
	IPhysicsConstraint	*pConstraint;
	float				flCreateTime;

	WheelHingeInfo_t()
	{
		pConstraint = NULL;
		flCreateTime = 0.0f;
	}
};

static CUtlVector<WheelHingeInfo_t*> g_WheelHinges;

static int Attach_RemoveWheelHinges( CBaseEntity *pEntity )
{
	int nRemoved = 0;

	for ( int i = g_WheelHinges.Count() - 1; i >= 0; i-- )
	{
		WheelHingeInfo_t *pInfo = g_WheelHinges[i];

		if ( pInfo->hEntity1.Get() == pEntity || pInfo->hEntity2.Get() == pEntity )
		{
			if ( pInfo->pConstraint )
			{
				physenv->DestroyConstraint( pInfo->pConstraint );
			}

			g_WheelHinges.Remove( i );
			delete pInfo;
			nRemoved++;
		}
	}

	return nRemoved;
}

static void Attach_CleanupWheelHinges()
{
	for ( int i = g_WheelHinges.Count() - 1; i >= 0; i-- )
	{
		WheelHingeInfo_t *pInfo = g_WheelHinges[i];

		if ( !pInfo->hEntity1.Get() || !pInfo->hEntity2.Get() )
		{
			if ( pInfo->pConstraint )
			{
				physenv->DestroyConstraint( pInfo->pConstraint );
			}

			g_WheelHinges.Remove( i );
			delete pInfo;
		}
	}
}

//-----------------------------------------------------------------------------
// Removes any children of pParent whose classname matches pszClassname -
// used to undo the Emitter/Sprite tools without needing our own registry
// (the parented child entities ARE the state).
//-----------------------------------------------------------------------------
static int Attach_RemoveChildrenOfClass( CBaseEntity *pParent, const char *pszClassname )
{
	if ( !pParent )
		return 0;

	int nRemoved = 0;
	CBaseEntity *pChild = pParent->FirstMoveChild();

	while ( pChild )
	{
		CBaseEntity *pNext = pChild->NextMovePeer();

		if ( FClassnameIs( pChild, pszClassname ) )
		{
			UTIL_Remove( pChild );
			nRemoved++;
		}

		pChild = pNext;
	}

	return nRemoved;
}

//-----------------------------------------------------------------------------
// RTCamera(21) - spawns a point_camera facing back toward the player.
// point_camera IS this codebase's local equivalent (LINK_ENTITY_TO_CLASS in
// Point_Camera.cpp, already compiled per server_bmod.cmake), so no fallback
// entity is needed.
//-----------------------------------------------------------------------------
static void Attach_CreateCamera( CWeaponTool *pTool, CBasePlayer *pOwner, const Vector &vecPos )
{
	CBaseEntity *pCamera = CreateEntityByName( "point_camera" );
	if ( !pCamera )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "Failed to create RT Camera (point_camera unavailable)" );
		return;
	}

	Vector vecFacing = pOwner->EyePosition() - vecPos;
	QAngle angFacing;
	VectorAngles( vecFacing, angFacing );

	pCamera->SetAbsOrigin( vecPos );
	pCamera->SetAbsAngles( angFacing );
	pCamera->Spawn();
	pCamera->Activate();

	pTool->PlayToolSound( "buttons/button14.wav" );
	ClientPrintf( pOwner, HUD_PRINTTALK, "RT Camera placed" );
}

//-----------------------------------------------------------------------------
// Wheel(29) - two-click flow shared through CWeaponTool's pending-selection
// state, same as the constraint-family tools.
//-----------------------------------------------------------------------------
static void Attach_WheelOnUse( CWeaponTool *pTool, CBasePlayer *pOwner, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	if ( !bPrimary )
	{
		int nRemoved = Attach_RemoveWheelHinges( pEntity );
		if ( nRemoved > 0 )
		{
			pTool->PlayToolSound( "weapons/physcannon/physcannon_claws_close.wav" );
			ClientPrintf( pOwner, HUD_PRINTTALK, "Wheel: removed %d hinge(s) from %s", nRemoved, pEntity->GetClassname() );
		}
		else
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "Wheel: %s has no hinge to remove", pEntity->GetClassname() );
		}
		return;
	}

	if ( !pEntity->VPhysicsGetObject() )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "Wheel: %s has no physics object", pEntity->GetClassname() );
		return;
	}

	CBaseEntity *pPending = pTool->GetPendingEntity();

	if ( !pPending )
	{
		pTool->SetPendingSelection( pEntity, tr.endpos );
		pTool->PlayToolSound( "buttons/button14.wav" );
		ClientPrintf( pOwner, HUD_PRINTTALK, "Wheel: selected %s, click the axle prop to attach a hinge", pEntity->GetClassname() );
		return;
	}

	if ( pPending == pEntity )
	{
		pTool->ClearPendingSelection();
		ClientPrintf( pOwner, HUD_PRINTTALK, "Wheel: selection cancelled" );
		return;
	}

	IPhysicsObject *pPhysRef = pPending->VPhysicsGetObject();
	IPhysicsObject *pPhysAtt = pEntity->VPhysicsGetObject();

	if ( !pPhysRef || !pPhysAtt )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "Wheel: both props need a physics object" );
		pTool->ClearPendingSelection();
		return;
	}

	Vector vecPendingPos = pTool->GetPendingPos();
	Vector vecAxis = tr.endpos - vecPendingPos;

	if ( vecAxis.LengthSqr() < 1.0f )
	{
		vecAxis = Vector( 0, 0, 1 );
	}
	else
	{
		VectorNormalize( vecAxis );
	}

	constraint_hingeparams_t hinge;
	hinge.Defaults();
	hinge.worldPosition = tr.endpos;
	hinge.worldAxisDirection = vecAxis;
	hinge.hingeAxis.SetAxisFriction( 0, 0, 0 );

	IPhysicsConstraint *pConstraint = physenv->CreateHingeConstraint( pPhysRef, pPhysAtt, NULL, hinge );

	if ( pConstraint )
	{
		WheelHingeInfo_t *pInfo = new WheelHingeInfo_t;
		pInfo->hEntity1 = pPending;
		pInfo->hEntity2 = pEntity;
		pInfo->pConstraint = pConstraint;
		pInfo->flCreateTime = gpGlobals->curtime;
		g_WheelHinges.AddToTail( pInfo );

		pTool->PlayToolSound( "weapons/physcannon/energy_sing_loop4.wav" );
		ClientPrintf( pOwner, HUD_PRINTTALK, "Wheel: hinged %s to %s", pPending->GetClassname(), pEntity->GetClassname() );
	}
	else
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "Wheel: failed to create hinge constraint" );
	}

	pTool->ClearPendingSelection();
}

//-----------------------------------------------------------------------------
// Tool_Attach_OnUse - dispatched when the trace hit a valid, usable entity
// (already non-null/non-world/non-player, per tool_dispatch.h's contract).
//-----------------------------------------------------------------------------
void Tool_Attach_OnUse( CWeaponTool *pTool, int nMode, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	switch ( nMode )
	{
	case TOOL_RTCAMERA:
		if ( bPrimary )
		{
			Attach_CreateCamera( pTool, pOwner, tr.endpos );
		}
		else if ( FClassnameIs( pEntity, "point_camera" ) )
		{
			UTIL_Remove( pEntity );
			pTool->PlayToolSound( "buttons/button10.wav" );
			ClientPrintf( pOwner, HUD_PRINTTALK, "RT Camera removed" );
		}
		else
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "That isn't an RT Camera" );
		}
		break;

	case TOOL_THRUSTER:
		if ( !pEntity->VPhysicsGetObject() )
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "Thruster: %s has no physics object", pEntity->GetClassname() );
			break;
		}

		if ( bPrimary )
		{
			if ( Attach_FindThruster( pEntity ) == -1 )
			{
				g_ThrusterEntities.AddToTail( pEntity );
				pTool->PlayToolSound( "buttons/button14.wav" );
				ClientPrintf( pOwner, HUD_PRINTTALK, "Thruster attached to %s (force %.0f)", pEntity->GetClassname(), bm_thruster_force.GetFloat() );
			}
			else
			{
				ClientPrintf( pOwner, HUD_PRINTTALK, "%s already has a thruster", pEntity->GetClassname() );
			}
		}
		else
		{
			int nIndex = Attach_FindThruster( pEntity );
			if ( nIndex != -1 )
			{
				g_ThrusterEntities.Remove( nIndex );
				pTool->PlayToolSound( "buttons/button10.wav" );
				ClientPrintf( pOwner, HUD_PRINTTALK, "Thruster removed from %s", pEntity->GetClassname() );
			}
			else
			{
				ClientPrintf( pOwner, HUD_PRINTTALK, "%s has no thruster", pEntity->GetClassname() );
			}
		}
		break;

	case TOOL_PHYSPROPS:
	{
		IPhysicsObject *pPhys = pEntity->VPhysicsGetObject();
		if ( !pPhys )
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "PhysProps: %s has no physics object", pEntity->GetClassname() );
			break;
		}

		if ( bPrimary )
		{
			bool bChanged = false;

			if ( bm_physprops_mass.GetFloat() > 0.0f )
			{
				pPhys->SetMass( bm_physprops_mass.GetFloat() );
				bChanged = true;
			}

			// No IPhysicsObject setter exists for friction/elasticity in this engine
			// (only SetMaterialIndex(), which swaps the whole physical material) -
			// tell the player rather than silently doing nothing.
			if ( bm_physprops_friction.GetFloat() >= 0.0f || bm_physprops_elasticity.GetFloat() >= 0.0f )
			{
				ClientPrintf( pOwner, HUD_PRINTTALK, "PhysProps: friction/elasticity overrides aren't supported by this engine yet" );
			}

			if ( bChanged )
			{
				pTool->PlayToolSound( "buttons/button14.wav" );
				ClientPrintf( pOwner, HUD_PRINTTALK, "PhysProps: %s mass set to %.1f", pEntity->GetClassname(), pPhys->GetMass() );
			}
			else
			{
				ClientPrintf( pOwner, HUD_PRINTTALK, "PhysProps: bm_physprops_mass is 0, nothing changed" );
			}
		}
		else
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "PhysProps: %s mass is %.1f", pEntity->GetClassname(), pPhys->GetMass() );
		}
		break;
	}

	case TOOL_BALLOON:
		if ( bPrimary )
		{
			CGModBalloon *pBalloon = CGModBalloonSystem::CreateBalloon( tr.endpos, pOwner );
			if ( pBalloon )
			{
				pBalloon->CreateRope( pEntity );
				pTool->PlayToolSound( "garrysmod/balloon_pop_cute.wav" );
				ClientPrintf( pOwner, HUD_PRINTTALK, "Balloon attached to %s", pEntity->GetClassname() );
			}
			else
			{
				ClientPrintf( pOwner, HUD_PRINTTALK, "Failed to create balloon" );
			}
		}
		else if ( FClassnameIs( pEntity, "gmod_balloon" ) )
		{
			UTIL_Remove( pEntity );
			ClientPrintf( pOwner, HUD_PRINTTALK, "Balloon removed" );
		}
		else
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "That isn't a balloon" );
		}
		break;

	case TOOL_EMITTER:
		if ( bPrimary )
		{
			CBaseEntity *pEmitter = CreateEntityByName( "info_particle_system" );
			if ( pEmitter )
			{
				pEmitter->SetAbsOrigin( tr.endpos );
				pEmitter->SetParent( pEntity );
				pEmitter->Spawn();
				pEmitter->Activate();

				pTool->PlayToolSound( "buttons/button14.wav" );
				ClientPrintf( pOwner, HUD_PRINTTALK, "Emitter attached to %s", pEntity->GetClassname() );
			}
			else
			{
				ClientPrintf( pOwner, HUD_PRINTTALK, "Failed to create emitter (info_particle_system unavailable)" );
			}
		}
		else
		{
			int nRemoved = Attach_RemoveChildrenOfClass( pEntity, "info_particle_system" );
			ClientPrintf( pOwner, HUD_PRINTTALK, "Removed %d emitter(s) from %s", nRemoved, pEntity->GetClassname() );
		}
		break;

	case TOOL_SPRITE:
		if ( bPrimary )
		{
			CBaseEntity *pSprite = CreateEntityByName( "env_sprite" );
			if ( pSprite )
			{
				pSprite->KeyValue( "model", "sprites/glow01.vmt" );
				pSprite->SetAbsOrigin( tr.endpos );
				pSprite->SetParent( pEntity );
				pSprite->Spawn();
				pSprite->Activate();

				pTool->PlayToolSound( "buttons/button14.wav" );
				ClientPrintf( pOwner, HUD_PRINTTALK, "Sprite attached to %s", pEntity->GetClassname() );
			}
			else
			{
				ClientPrintf( pOwner, HUD_PRINTTALK, "Failed to create sprite (env_sprite unavailable)" );
			}
		}
		else
		{
			int nRemoved = Attach_RemoveChildrenOfClass( pEntity, "env_sprite" );
			ClientPrintf( pOwner, HUD_PRINTTALK, "Removed %d sprite(s) from %s", nRemoved, pEntity->GetClassname() );
		}
		break;

	case TOOL_WHEEL:
		Attach_WheelOnUse( pTool, pOwner, pEntity, tr, bPrimary );
		break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// Tool_Attach_OnTrace - dispatched when the trace hit world or nothing.
// None of these tools have a valid target in that case except Wheel, which
// just cancels a pending selection.
//-----------------------------------------------------------------------------
void Tool_Attach_OnTrace( CWeaponTool *pTool, int nMode, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	if ( nMode == TOOL_WHEEL )
	{
		if ( pTool->GetPendingEntity() )
		{
			pTool->ClearPendingSelection();
			ClientPrintf( pOwner, HUD_PRINTTALK, "Wheel: selection cancelled" );
		}
		else
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "Wheel: no valid target" );
		}
		return;
	}

	ClientPrintf( pOwner, HUD_PRINTTALK, "No valid target for this tool" );
}

//-----------------------------------------------------------------------------
// Tool_Attach_OnThink - only Thruster (continuous force) and Wheel (pending
// selection timeout / hinge cleanup) need per-frame work.
//-----------------------------------------------------------------------------
static void Attach_ThrusterThink()
{
	float flForce = bm_thruster_force.GetFloat();

	for ( int i = g_ThrusterEntities.Count() - 1; i >= 0; i-- )
	{
		CBaseEntity *pEntity = g_ThrusterEntities[i].Get();
		IPhysicsObject *pPhys = pEntity ? pEntity->VPhysicsGetObject() : NULL;

		if ( !pPhys )
		{
			g_ThrusterEntities.Remove( i );
			continue;
		}

		pPhys->Wake();
		// ApplyForceCenter takes an impulse, so scale the ConVar's force (in
		// kg*in/s^2) by this frame's duration to get a continuous push.
		pPhys->ApplyForceCenter( Vector( 0, 0, flForce * gpGlobals->frametime ) );
	}
}

static void Attach_WheelThink( CWeaponTool *pTool )
{
	if ( pTool->GetPendingEntity() && ( gpGlobals->curtime - pTool->GetPendingTime() ) > 10.0f )
	{
		pTool->ClearPendingSelection();

		CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
		if ( pOwner )
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "Wheel: selection timed out" );
		}
	}

	Attach_CleanupWheelHinges();
}

void Tool_Attach_OnThink( CWeaponTool *pTool, int nMode )
{
	switch ( nMode )
	{
	case TOOL_THRUSTER:
		Attach_ThrusterThink();
		break;

	case TOOL_WHEEL:
		Attach_WheelThink( pTool );
		break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// Console command helpers
//-----------------------------------------------------------------------------
CON_COMMAND( bm_attach_list, "List entities/constraints tracked by the Attach tool family (Thruster/Wheel)" )
{
	Msg( "Thruster entities: %d\n", g_ThrusterEntities.Count() );
	for ( int i = 0; i < g_ThrusterEntities.Count(); i++ )
	{
		CBaseEntity *pEntity = g_ThrusterEntities[i].Get();
		Msg( "%d. %s\n", i + 1, pEntity ? pEntity->GetClassname() : "NULL" );
	}

	Msg( "Wheel hinges: %d\n", g_WheelHinges.Count() );
	for ( int i = 0; i < g_WheelHinges.Count(); i++ )
	{
		WheelHingeInfo_t *pInfo = g_WheelHinges[i];
		Msg( "%d. %s <-> %s\n", i + 1,
			pInfo->hEntity1.Get() ? pInfo->hEntity1.Get()->GetClassname() : "NULL",
			pInfo->hEntity2.Get() ? pInfo->hEntity2.Get()->GetClassname() : "NULL" );
	}
}

CON_COMMAND( bm_thruster_removeall, "Remove all thrusters created by the Thruster tool" )
{
	int nRemoved = g_ThrusterEntities.Count();
	g_ThrusterEntities.RemoveAll();

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Removed %d thrusters", nRemoved );
	}
}

CON_COMMAND( bm_wheel_removeall, "Remove all wheel hinge constraints created by the Wheel tool" )
{
	int nRemoved = 0;

	for ( int i = g_WheelHinges.Count() - 1; i >= 0; i-- )
	{
		WheelHingeInfo_t *pInfo = g_WheelHinges[i];

		if ( pInfo->pConstraint )
		{
			physenv->DestroyConstraint( pInfo->pConstraint );
		}

		delete pInfo;
		nRemoved++;
	}

	g_WheelHinges.RemoveAll();

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Removed %d wheel hinges", nRemoved );
	}
}
