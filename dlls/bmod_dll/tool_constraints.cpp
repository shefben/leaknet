//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Constraint Tool Family - Rope/Elastic/Weld/Ballsocket/
//          Pulley/EasyWeld/EasyBall/Axis/Slider/NailGun.
//
//          All of these tools share the same "click a prop, then click a
//          second prop (or world)" flow, using CWeaponTool's shared pending-
//          selection state (see weapon_tool.h) instead of per-instance
//          member storage, since CWeaponTool is the only class ever
//          instantiated for "weapon_tool" (see tool_dispatch.h).
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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables
//-----------------------------------------------------------------------------
ConVar bm_rope_length( "bm_rope_length", "200", FCVAR_ARCHIVE, "Maximum rope constraint length in units" );
ConVar bm_elastic_constant( "bm_elastic_constant", "50", FCVAR_ARCHIVE, "Elastic (spring) constraint spring constant" );
ConVar bm_elastic_damping( "bm_elastic_damping", "1", FCVAR_ARCHIVE, "Elastic (spring) constraint damping" );
ConVar bm_weld_nocollide( "bm_weld_nocollide", "1", FCVAR_ARCHIVE, "Disable collision between welded objects" );

//-----------------------------------------------------------------------------
// Registry of every constraint created by this tool family, so a right-click
// (secondary attack) can find and remove constraints touching an entity.
//-----------------------------------------------------------------------------
struct ConstraintInfo_t
{
	int					nMode;
	EHANDLE				hEntity1;
	EHANDLE				hEntity2;			// invalid/NULL if bWorldAttached
	bool				bWorldAttached;		// true if this constraint anchors hEntity1 to the world
	bool				bDisabledCollision;	// true if we disabled collisions between hEntity1/hEntity2
	IPhysicsConstraint	*pConstraint;		// used by every mode except Elastic
	IPhysicsSpring		*pSpring;			// used only by Elastic
	float				flCreateTime;

	ConstraintInfo_t()
	{
		nMode = TOOL_NONE;
		bWorldAttached = false;
		bDisabledCollision = false;
		pConstraint = NULL;
		pSpring = NULL;
		flCreateTime = 0.0f;
	}
};

static CUtlVector<ConstraintInfo_t*> g_ToolConstraints;

//-----------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------
static const char *GetConstraintModeName( int nMode )
{
	switch ( nMode )
	{
		case TOOL_ROPE:			return "Rope";
		case TOOL_ELASTIC:		return "Elastic";
		case TOOL_WELD:			return "Weld";
		case TOOL_BALLSOCKET:	return "Ballsocket";
		case TOOL_PULLEY:		return "Pulley";
		case TOOL_EASYWELD:		return "EasyWeld";
		case TOOL_EASYBALL:		return "EasyBall";
		case TOOL_AXIS:			return "Axis";
		case TOOL_SLIDER:		return "Slider";
		case TOOL_NAILGUN:		return "Nailgun";
	}
	return "Constraint";
}

// EasyWeld/EasyBall are the same physics as Weld/Ballsocket, just without the
// chatty step-by-step prompts (per design - "fewer steps/prompts, not
// different physics").
static bool IsVerboseConstraintMode( int nMode )
{
	return ( nMode != TOOL_EASYWELD && nMode != TOOL_EASYBALL );
}

// Only tools that create a length/spring-like constraint make sense anchored
// to a fixed world point - Weld/Ballsocket/EasyWeld/EasyBall/Axis/Nailgun all
// need a second real physics object to attach to.
static bool SupportsWorldAttach( int nMode )
{
	switch ( nMode )
	{
		case TOOL_ROPE:
		case TOOL_ELASTIC:
		case TOOL_PULLEY:
		case TOOL_SLIDER:
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Creates the actual vphysics constraint/spring for a given mode. pPhys2 may
// be g_PhysWorldObject (world attach) for the modes SupportsWorldAttach()
// allows. Returns the created IPhysicsConstraint, or NULL and fills ppSpring
// for TOOL_ELASTIC, or NULL/NULL on failure.
//-----------------------------------------------------------------------------
static IPhysicsConstraint *CreateConstraintForMode( int nMode, IPhysicsObject *pPhys1, IPhysicsObject *pPhys2,
													 const Vector &vecPos1, const Vector &vecPos2, IPhysicsSpring **ppSpring )
{
	*ppSpring = NULL;

	switch ( nMode )
	{
		case TOOL_ROPE:
		{
			constraint_lengthparams_t length;
			length.Defaults();
			length.InitWorldspace( pPhys1, pPhys2, vecPos1, vecPos2, false );

			float flMaxLength = bm_rope_length.GetFloat();
			if ( length.totalLength > flMaxLength )
				length.totalLength = flMaxLength;

			return physenv->CreateLengthConstraint( pPhys1, pPhys2, NULL, length );
		}

		case TOOL_ELASTIC:
		{
			springparams_t spring;
			spring.constant = bm_elastic_constant.GetFloat();
			spring.damping = bm_elastic_damping.GetFloat();
			spring.naturalLength = vecPos1.DistTo( vecPos2 );
			spring.startPosition = vecPos1;
			spring.endPosition = vecPos2;
			spring.useLocalPositions = false;

			*ppSpring = physenv->CreateSpring( pPhys1, pPhys2, &spring );
			return NULL;
		}

		case TOOL_WELD:
		case TOOL_EASYWELD:
		{
			constraint_fixedparams_t fixed;
			fixed.Defaults();
			fixed.InitWithCurrentObjectState( pPhys1, pPhys2 );

			return physenv->CreateFixedConstraint( pPhys1, pPhys2, NULL, fixed );
		}

		case TOOL_BALLSOCKET:
		case TOOL_EASYBALL:
		case TOOL_NAILGUN:
		{
			// Use the midpoint between the two click positions as the shared
			// pivot point, converted into each object's local space.
			Vector vecShared = ( vecPos1 + vecPos2 ) * 0.5f;

			constraint_ballsocketparams_t ballsocket;
			ballsocket.Defaults();
			pPhys1->WorldToLocal( ballsocket.constraintPosition[0], vecShared );
			pPhys2->WorldToLocal( ballsocket.constraintPosition[1], vecShared );

			return physenv->CreateBallsocketConstraint( pPhys1, pPhys2, NULL, ballsocket );
		}

		case TOOL_PULLEY:
		{
			constraint_pulleyparams_t pulley;
			pulley.Defaults();
			pulley.pulleyPosition[0] = vecPos1;
			pulley.pulleyPosition[1] = vecPos2;
			pPhys1->WorldToLocal( pulley.objectPosition[0], vecPos1 );
			pPhys2->WorldToLocal( pulley.objectPosition[1], vecPos2 );
			pulley.totalLength = vecPos1.DistTo( vecPos2 );
			pulley.gearRatio = 1.0f;

			return physenv->CreatePulleyConstraint( pPhys1, pPhys2, NULL, pulley );
		}

		case TOOL_AXIS:
		{
			constraint_hingeparams_t hinge;
			hinge.Defaults();
			hinge.worldPosition = vecPos1;

			Vector vecAxis = vecPos2 - vecPos1;
			VectorNormalize( vecAxis );
			hinge.worldAxisDirection = vecAxis;

			return physenv->CreateHingeConstraint( pPhys1, pPhys2, NULL, hinge );
		}

		case TOOL_SLIDER:
		{
			constraint_slidingparams_t sliding;
			sliding.Defaults();

			Vector vecDir = vecPos2 - vecPos1;
			float flDist = VectorNormalize( vecDir );

			sliding.InitWithCurrentObjectState( pPhys1, pPhys2, vecDir );
			sliding.limitMin = 0.0f;
			sliding.limitMax = flDist;

			return physenv->CreateSlidingConstraint( pPhys1, pPhys2, NULL, sliding );
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Destroys the physics-side resources of a constraint entry (but does not
// remove it from the registry or delete it).
//-----------------------------------------------------------------------------
static void DestroyConstraintInfo( ConstraintInfo_t *pInfo )
{
	if ( pInfo->pConstraint )
	{
		physenv->DestroyConstraint( pInfo->pConstraint );
		pInfo->pConstraint = NULL;
	}

	if ( pInfo->pSpring )
	{
		physenv->DestroySpring( pInfo->pSpring );
		pInfo->pSpring = NULL;
	}

	if ( pInfo->bDisabledCollision )
	{
		CBaseEntity *pEnt1 = pInfo->hEntity1.Get();
		CBaseEntity *pEnt2 = pInfo->hEntity2.Get();

		if ( pEnt1 && pEnt2 )
		{
			IPhysicsObject *pPhys1 = pEnt1->VPhysicsGetObject();
			IPhysicsObject *pPhys2 = pEnt2->VPhysicsGetObject();

			if ( pPhys1 && pPhys2 )
				physenv->EnableCollisions( pPhys1, pPhys2 );
		}
	}
}

//-----------------------------------------------------------------------------
// Attempts to build and register a constraint between pEnt1 and pEnt2.
// pEnt2 may be NULL to mean "attach to the world" (only valid for modes
// where SupportsWorldAttach() is true - caller is expected to have checked).
//-----------------------------------------------------------------------------
static bool CreateAndRegisterConstraint( int nMode, CBaseEntity *pEnt1, CBaseEntity *pEnt2,
										  const Vector &vecPos1, const Vector &vecPos2 )
{
	if ( !pEnt1 )
		return false;

	IPhysicsObject *pPhys1 = pEnt1->VPhysicsGetObject();
	IPhysicsObject *pPhys2 = pEnt2 ? pEnt2->VPhysicsGetObject() : g_PhysWorldObject;

	if ( !pPhys1 || !pPhys2 )
		return false;

	IPhysicsSpring *pSpring = NULL;
	IPhysicsConstraint *pConstraint = CreateConstraintForMode( nMode, pPhys1, pPhys2, vecPos1, vecPos2, &pSpring );

	if ( !pConstraint && !pSpring )
		return false;

	ConstraintInfo_t *pInfo = new ConstraintInfo_t;
	pInfo->nMode = nMode;
	pInfo->hEntity1 = pEnt1;
	pInfo->hEntity2 = pEnt2;
	pInfo->bWorldAttached = ( pEnt2 == NULL );
	pInfo->pConstraint = pConstraint;
	pInfo->pSpring = pSpring;
	pInfo->flCreateTime = gpGlobals->curtime;

	if ( ( nMode == TOOL_WELD || nMode == TOOL_EASYWELD ) && pEnt2 && bm_weld_nocollide.GetBool() )
	{
		physenv->DisableCollisions( pPhys1, pPhys2 );
		pInfo->bDisabledCollision = true;
	}

	g_ToolConstraints.AddToTail( pInfo );
	return true;
}

//-----------------------------------------------------------------------------
// Removes every constraint touching pEntity (right-click / secondary attack).
//-----------------------------------------------------------------------------
static void RemoveConstraintsOnEntity( CBasePlayer *pOwner, CBaseEntity *pEntity )
{
	if ( !pEntity )
		return;

	int nRemoved = 0;

	for ( int i = g_ToolConstraints.Count() - 1; i >= 0; i-- )
	{
		ConstraintInfo_t *pInfo = g_ToolConstraints[i];
		CBaseEntity *pEnt1 = pInfo->hEntity1.Get();
		CBaseEntity *pEnt2 = pInfo->hEntity2.Get();

		if ( pEnt1 == pEntity || pEnt2 == pEntity )
		{
			DestroyConstraintInfo( pInfo );
			g_ToolConstraints.Remove( i );
			delete pInfo;
			nRemoved++;
		}
	}

	if ( pOwner )
	{
		char szBuf[64];
		Q_snprintf( szBuf, sizeof( szBuf ), "Removed %d constraint(s)", nRemoved );
		ClientPrint( pOwner, HUD_PRINTTALK, szBuf );
	}
}

//-----------------------------------------------------------------------------
// Clears out constraints whose entities have gone away (removed/killed).
//-----------------------------------------------------------------------------
static void CleanupConstraints()
{
	for ( int i = g_ToolConstraints.Count() - 1; i >= 0; i-- )
	{
		ConstraintInfo_t *pInfo = g_ToolConstraints[i];
		CBaseEntity *pEnt1 = pInfo->hEntity1.Get();
		CBaseEntity *pEnt2 = pInfo->hEntity2.Get();

		bool bInvalid = ( !pEnt1 || !pEnt1->VPhysicsGetObject() );

		if ( !pInfo->bWorldAttached && ( !pEnt2 || !pEnt2->VPhysicsGetObject() ) )
			bInvalid = true;

		if ( bInvalid )
		{
			DestroyConstraintInfo( pInfo );
			g_ToolConstraints.Remove( i );
			delete pInfo;
		}
	}
}

//-----------------------------------------------------------------------------
// Tool_Constraint_OnUse - trace hit a valid, usable prop.
//-----------------------------------------------------------------------------
void Tool_Constraint_OnUse( CWeaponTool *pTool, int nMode, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	if ( !bPrimary )
	{
		RemoveConstraintsOnEntity( pOwner, pEntity );
		return;
	}

	CBaseEntity *pPending = pTool->GetPendingEntity();

	if ( !pPending )
	{
		IPhysicsObject *pPhys = pEntity->VPhysicsGetObject();
		if ( !pPhys )
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "That object has no physics" );
			return;
		}

		pTool->SetPendingSelection( pEntity, tr.endpos );
		pTool->PlayToolSound( "buttons/button14.wav" );

		if ( IsVerboseConstraintMode( nMode ) )
		{
			char szBuf[128];
			Q_snprintf( szBuf, sizeof( szBuf ), "%s: selected %s - click a second prop", GetConstraintModeName( nMode ), pEntity->GetClassname() );
			ClientPrint( pOwner, HUD_PRINTTALK, szBuf );
		}
		return;
	}

	if ( pPending == pEntity )
	{
		pTool->ClearPendingSelection();
		ClientPrint( pOwner, HUD_PRINTTALK, "Selection cancelled" );
		return;
	}

	IPhysicsObject *pPhys2 = pEntity->VPhysicsGetObject();
	if ( !pPhys2 )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "That object has no physics" );
		return;
	}

	bool bOk = CreateAndRegisterConstraint( nMode, pPending, pEntity, pTool->GetPendingPos(), tr.endpos );

	char szBuf[128];
	if ( bOk )
	{
		pTool->PlayToolSound( "weapons/physcannon/energy_sing_loop4.wav" );
		Q_snprintf( szBuf, sizeof( szBuf ), "%s created between %s and %s", GetConstraintModeName( nMode ), pPending->GetClassname(), pEntity->GetClassname() );
	}
	else
	{
		Q_snprintf( szBuf, sizeof( szBuf ), "Failed to create %s constraint", GetConstraintModeName( nMode ) );
	}
	ClientPrint( pOwner, HUD_PRINTTALK, szBuf );

	pTool->ClearPendingSelection();
}

//-----------------------------------------------------------------------------
// Tool_Constraint_OnTrace - trace hit world or nothing.
//-----------------------------------------------------------------------------
void Tool_Constraint_OnTrace( CWeaponTool *pTool, int nMode, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	if ( !bPrimary )
		return;

	CBaseEntity *pPending = pTool->GetPendingEntity();
	if ( !pPending )
		return;

	if ( !SupportsWorldAttach( nMode ) )
	{
		pTool->ClearPendingSelection();
		ClientPrint( pOwner, HUD_PRINTTALK, "Selection cancelled - aim at a prop" );
		return;
	}

	bool bOk = CreateAndRegisterConstraint( nMode, pPending, NULL, pTool->GetPendingPos(), tr.endpos );

	char szBuf[128];
	if ( bOk )
	{
		pTool->PlayToolSound( "weapons/physcannon/energy_sing_loop4.wav" );
		Q_snprintf( szBuf, sizeof( szBuf ), "%s created to world", GetConstraintModeName( nMode ) );
	}
	else
	{
		Q_snprintf( szBuf, sizeof( szBuf ), "Failed to create %s constraint to world", GetConstraintModeName( nMode ) );
	}
	ClientPrint( pOwner, HUD_PRINTTALK, szBuf );

	pTool->ClearPendingSelection();
}

//-----------------------------------------------------------------------------
// Tool_Constraint_OnThink - pending-selection timeout + registry cleanup.
//-----------------------------------------------------------------------------
void Tool_Constraint_OnThink( CWeaponTool *pTool, int nMode )
{
	CBaseEntity *pPending = pTool->GetPendingEntity();
	if ( pPending && ( gpGlobals->curtime - pTool->GetPendingTime() ) > 10.0f )
	{
		pTool->ClearPendingSelection();

		CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
		if ( pOwner )
			ClientPrint( pOwner, HUD_PRINTTALK, "Selection timed out" );
	}

	CleanupConstraints();
}

//-----------------------------------------------------------------------------
// Console command to remove all tool constraints
//-----------------------------------------------------------------------------
CON_COMMAND( bm_constraint_removeall, "Remove all tool constraints" )
{
	int nRemoved = g_ToolConstraints.Count();

	for ( int i = g_ToolConstraints.Count() - 1; i >= 0; i-- )
	{
		ConstraintInfo_t *pInfo = g_ToolConstraints[i];
		DestroyConstraintInfo( pInfo );
		delete pInfo;
	}

	g_ToolConstraints.RemoveAll();

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		char szBuf[64];
		Q_snprintf( szBuf, sizeof( szBuf ), "Removed %d constraint(s)", nRemoved );
		ClientPrint( pPlayer, HUD_PRINTTALK, szBuf );
	}
}

//-----------------------------------------------------------------------------
// Console command to list all tool constraints
//-----------------------------------------------------------------------------
CON_COMMAND( bm_constraint_list, "List all tool constraints" )
{
	Msg( "Active tool constraints: %d\n", g_ToolConstraints.Count() );

	for ( int i = 0; i < g_ToolConstraints.Count(); i++ )
	{
		ConstraintInfo_t *pInfo = g_ToolConstraints[i];
		CBaseEntity *pEnt1 = pInfo->hEntity1.Get();
		CBaseEntity *pEnt2 = pInfo->hEntity2.Get();

		Msg( "%d. %s: %s <-> %s (Age: %.1fs)\n",
			i + 1,
			GetConstraintModeName( pInfo->nMode ),
			pEnt1 ? pEnt1->GetClassname() : "NULL",
			pInfo->bWorldAttached ? "world" : ( pEnt2 ? pEnt2->GetClassname() : "NULL" ),
			gpGlobals->curtime - pInfo->flCreateTime );
	}
}
