//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Tool System - Ignite / Magnetise / NoCollide / Dynamite /
//          Statue tool modes. These are all simple single-click or
//          pending-selection property toggles on props, so they share a
//          single dispatch file (Tool_Simple_On*, see tool_dispatch.h).
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "util.h"
#include "baseanimating.h"
#include "physics.h"
#include "ragdoll_shared.h"
#include "vphysics_interface.h"
#include "eventqueue.h"
#include "explode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables
//-----------------------------------------------------------------------------
// Authentic GMod 9 cvar names - the build menu's context panels, third-party
// mods and Lua scripts all reference these exact names.
ConVar gmod_ignite_duration( "gm_firelength", "30", FCVAR_ARCHIVE, "Fire duration (seconds) applied by the Ignite tool" );

ConVar gmod_magnetise_force( "gm_magnetstrength", "400", FCVAR_ARCHIVE, "Acceleration (units/s^2) pulling magnetised props together" );
ConVar gmod_magnetise_range( "gmod_magnetise_range", "512", FCVAR_ARCHIVE, "Max distance between two magnetised props for the pull to apply" );

// Dynamite settings live in gmod_dynamite.cpp under their GMod 9 names.
extern ConVar gm_dynamite_power;
extern ConVar gm_dynamite_delay;

//-----------------------------------------------------------------------------
// TOOL_MAGNETISE - entities currently marked magnetic. Every valid pair
// within gmod_magnetise_range of each other gets pulled together every think.
//-----------------------------------------------------------------------------
static CUtlVector<EHANDLE> g_MagnetisedEntities;

//-----------------------------------------------------------------------------
// TOOL_NOCOLLIDE - true pairwise no-collide via IPhysicsEnvironment, tracked
// so secondary fire / gmod_nocollide_removeall can turn collisions back on.
//-----------------------------------------------------------------------------
struct NoCollideInfo_t
{
	EHANDLE hEntity1;
	EHANDLE hEntity2;
	float	flCreateTime;
};
static CUtlVector<NoCollideInfo_t*> g_NoCollidePairs;

//-----------------------------------------------------------------------------
// TOOL_STATUE - entities currently frozen (EnableMotion(false)) by this tool.
//-----------------------------------------------------------------------------
static CUtlVector<EHANDLE> g_StatuedEntities;

//-----------------------------------------------------------------------------
// TOOL_DYNAMITE - explosions we've queued, so secondary fire / console
// commands can defuse them before they detonate.
//-----------------------------------------------------------------------------
static CUtlVector<EHANDLE> g_PendingDynamite;

//-----------------------------------------------------------------------------
// Small shared helpers
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// A ragdoll is many physics objects, one per phys bone. VPhysicsGetObject()
// only ever returns the root (the pelvis), so freezing or no-colliding a
// ragdoll through it leaves every other limb loose. VPhysicsGetObjectList() is
// overridden by CRagdollProp to hand back the whole set, and falls back to the
// single object for ordinary props.
//-----------------------------------------------------------------------------
#define SIMPLE_MAX_PHYSOBJECTS	RAGDOLL_MAX_ELEMENTS

static void Simple_SetEntityMotionEnabled( CBaseEntity *pEntity, bool bEnable )
{
	if ( !pEntity )
		return;

	IPhysicsObject *pList[SIMPLE_MAX_PHYSOBJECTS];
	int count = pEntity->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
	for ( int i = 0; i < count; i++ )
	{
		if ( !pList[i] )
			continue;

		pList[i]->EnableMotion( bEnable );
		if ( bEnable )
		{
			pList[i]->Wake();
		}
	}
}

static void Simple_SetEntityPairCollisions( CBaseEntity *pEnt1, CBaseEntity *pEnt2, bool bEnable )
{
	if ( !pEnt1 || !pEnt2 )
		return;

	IPhysicsObject *pList1[SIMPLE_MAX_PHYSOBJECTS];
	IPhysicsObject *pList2[SIMPLE_MAX_PHYSOBJECTS];
	int count1 = pEnt1->VPhysicsGetObjectList( pList1, ARRAYSIZE(pList1) );
	int count2 = pEnt2->VPhysicsGetObjectList( pList2, ARRAYSIZE(pList2) );

	for ( int i = 0; i < count1; i++ )
	{
		if ( !pList1[i] )
			continue;

		for ( int j = 0; j < count2; j++ )
		{
			if ( !pList2[j] )
				continue;

			if ( bEnable )
				physenv->EnableCollisions( pList1[i], pList2[j] );
			else
				physenv->DisableCollisions( pList1[i], pList2[j] );
		}

		pList1[i]->RecheckCollisionFilter();
	}

	for ( int j = 0; j < count2; j++ )
	{
		if ( pList2[j] )
			pList2[j]->RecheckCollisionFilter();
	}
}

static int Simple_FindHandle( CUtlVector<EHANDLE> &list, CBaseEntity *pEntity )
{
	for ( int i = 0; i < list.Count(); i++ )
	{
		if ( list[i].Get() == pEntity )
			return i;
	}
	return -1;
}

static NoCollideInfo_t *Simple_FindNoCollidePair( CBaseEntity *pEnt1, CBaseEntity *pEnt2 )
{
	for ( int i = 0; i < g_NoCollidePairs.Count(); i++ )
	{
		NoCollideInfo_t *pInfo = g_NoCollidePairs[i];
		CBaseEntity *pA = pInfo->hEntity1.Get();
		CBaseEntity *pB = pInfo->hEntity2.Get();

		if ( ( pA == pEnt1 && pB == pEnt2 ) || ( pA == pEnt2 && pB == pEnt1 ) )
			return pInfo;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// TOOL_IGNITE
//-----------------------------------------------------------------------------
static void Simple_Ignite_OnUse( CBasePlayer *pOwner, CWeaponTool *pTool, CBaseEntity *pEntity, bool bPrimary )
{
	// Ignite() only exists on CBaseAnimating in this codebase (baseanimating.h),
	// not on plain CBaseEntity - most props/NPCs are CBaseAnimating so this
	// covers the common case.
	CBaseAnimating *pAnim = dynamic_cast<CBaseAnimating*>( pEntity );
	if ( !pAnim )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "This entity cannot be set on fire" );
		return;
	}

	if ( bPrimary )
	{
		pAnim->Ignite( gmod_ignite_duration.GetFloat() );
		pTool->PlayToolSound( "fire/ignite.wav" );

		char buf[128];
		Q_snprintf( buf, sizeof(buf), "Ignited %s for %.0f seconds", pEntity->GetClassname(), gmod_ignite_duration.GetFloat() );
		ClientPrint( pOwner, HUD_PRINTTALK, buf );
	}
	else
	{
		// NOTE: this codebase has no Extinguish() on CBaseEntity/CBaseAnimating
		// (only CFire::Extinguish(), which belongs to the separate fire entity,
		// not the burning prop itself), so we can't put the fire out from here.
		ClientPrint( pOwner, HUD_PRINTTALK, "No extinguish method is available in this build - the fire must burn out" );
	}
}

//-----------------------------------------------------------------------------
// TOOL_MAGNETISE
//-----------------------------------------------------------------------------
static void Simple_Magnetise_OnUse( CBasePlayer *pOwner, CWeaponTool *pTool, CBaseEntity *pEntity, bool bPrimary )
{
	if ( !bPrimary )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Magnetise: left click toggles magnetism on a prop" );
		return;
	}

	if ( !pEntity->VPhysicsGetObject() )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "This entity has no physics object to magnetise" );
		return;
	}

	int idx = Simple_FindHandle( g_MagnetisedEntities, pEntity );
	char buf[128];

	if ( idx >= 0 )
	{
		g_MagnetisedEntities.Remove( idx );
		Q_snprintf( buf, sizeof(buf), "%s is no longer magnetic", pEntity->GetClassname() );
	}
	else
	{
		g_MagnetisedEntities.AddToTail( pEntity );
		Q_snprintf( buf, sizeof(buf), "%s is now magnetic", pEntity->GetClassname() );
	}

	pTool->PlayToolSound( "buttons/button14.wav" );
	ClientPrint( pOwner, HUD_PRINTTALK, buf );
}

static void Simple_Magnetise_Think( void )
{
	// Drop dead handles first.
	for ( int i = g_MagnetisedEntities.Count() - 1; i >= 0; i-- )
	{
		if ( !g_MagnetisedEntities[i].Get() )
			g_MagnetisedEntities.Remove( i );
	}

	float flRange = gmod_magnetise_range.GetFloat();
	float flForce = gmod_magnetise_force.GetFloat();

	int nCount = g_MagnetisedEntities.Count();
	for ( int i = 0; i < nCount; i++ )
	{
		CBaseEntity *pEnt1 = g_MagnetisedEntities[i].Get();
		IPhysicsObject *pPhys1 = pEnt1 ? pEnt1->VPhysicsGetObject() : NULL;
		if ( !pPhys1 )
			continue;

		for ( int j = i + 1; j < nCount; j++ )
		{
			CBaseEntity *pEnt2 = g_MagnetisedEntities[j].Get();
			IPhysicsObject *pPhys2 = pEnt2 ? pEnt2->VPhysicsGetObject() : NULL;
			if ( !pPhys2 )
				continue;

			Vector vecDelta = pEnt2->GetAbsOrigin() - pEnt1->GetAbsOrigin();
			float flDist = vecDelta.Length();
			if ( flDist < 1.0f || flDist > flRange )
				continue;

			Vector vecDir = vecDelta / flDist;

			// Scale by mass so the resulting acceleration (not force) is
			// constant regardless of how heavy either prop is.
			pPhys1->ApplyForceCenter( vecDir * flForce * pPhys1->GetMass() );
			pPhys2->ApplyForceCenter( -vecDir * flForce * pPhys2->GetMass() );
		}
	}
}

//-----------------------------------------------------------------------------
// TOOL_NOCOLLIDE
//-----------------------------------------------------------------------------
static void Simple_NoCollide_OnUse( CBasePlayer *pOwner, CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	if ( !bPrimary )
	{
		// Secondary fire cancels a pending selection, or restores collision
		// for any pair involving the targeted entity.
		if ( pTool->GetPendingEntity() )
		{
			pTool->ClearPendingSelection();
			ClientPrint( pOwner, HUD_PRINTTALK, "No-collide selection cancelled" );
			return;
		}

		bool bFound = false;
		for ( int i = g_NoCollidePairs.Count() - 1; i >= 0; i-- )
		{
			NoCollideInfo_t *pInfo = g_NoCollidePairs[i];
			CBaseEntity *pA = pInfo->hEntity1.Get();
			CBaseEntity *pB = pInfo->hEntity2.Get();

			if ( pA == pEntity || pB == pEntity )
			{
				Simple_SetEntityPairCollisions( pA, pB, true );

				g_NoCollidePairs.Remove( i );
				delete pInfo;
				bFound = true;
			}
		}

		ClientPrint( pOwner, HUD_PRINTTALK, bFound ? "Collision restored" : "This entity has no no-collide pairs" );
		return;
	}

	if ( !pEntity->VPhysicsGetObject() )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "This entity has no physics object" );
		return;
	}

	CBaseEntity *pPending = pTool->GetPendingEntity();
	if ( !pPending )
	{
		pTool->SetPendingSelection( pEntity, tr.endpos );
		ClientPrint( pOwner, HUD_PRINTTALK, "Selected first entity - click a second entity to no-collide them" );
		return;
	}

	if ( pPending == pEntity )
	{
		pTool->ClearPendingSelection();
		ClientPrint( pOwner, HUD_PRINTTALK, "Selection cancelled" );
		return;
	}

	IPhysicsObject *pPhys1 = pPending->VPhysicsGetObject();
	IPhysicsObject *pPhys2 = pEntity->VPhysicsGetObject();

	if ( !pPhys1 || !pPhys2 )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Both entities need a physics object" );
		pTool->ClearPendingSelection();
		return;
	}

	if ( Simple_FindNoCollidePair( pPending, pEntity ) )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "These entities are already no-collided" );
		pTool->ClearPendingSelection();
		return;
	}

	// physenv->DisableCollisions()/EnableCollisions() is a true pairwise
	// no-collide (public/vphysics_interface.h) - no need for the
	// COLLISION_GROUP_DEBRIS approximation fallback. Cross every physics object
	// on both sides so ragdoll limbs are all covered, not just the root.
	Simple_SetEntityPairCollisions( pPending, pEntity, false );

	NoCollideInfo_t *pInfo = new NoCollideInfo_t;
	pInfo->hEntity1 = pPending;
	pInfo->hEntity2 = pEntity;
	pInfo->flCreateTime = gpGlobals->curtime;
	g_NoCollidePairs.AddToTail( pInfo );

	pTool->PlayToolSound( "physics/body/body_medium_impact_soft1.wav" );

	char buf[160];
	Q_snprintf( buf, sizeof(buf), "No-collided %s with %s", pPending->GetClassname(), pEntity->GetClassname() );
	ClientPrint( pOwner, HUD_PRINTTALK, buf );

	pTool->ClearPendingSelection();
}

//-----------------------------------------------------------------------------
// TOOL_DYNAMITE
//-----------------------------------------------------------------------------
static void Simple_Dynamite_OnUse( CBasePlayer *pOwner, CWeaponTool *pTool, CBaseEntity *pEntity, bool bPrimary )
{
	if ( !bPrimary )
	{
		// Secondary fire defuses every pending dynamite this tool has queued.
		int nRemoved = 0;
		for ( int i = g_PendingDynamite.Count() - 1; i >= 0; i-- )
		{
			CBaseEntity *pExplosion = g_PendingDynamite[i].Get();
			if ( pExplosion )
			{
				g_EventQueue.CancelEvents( pExplosion );
				UTIL_Remove( pExplosion );
				nRemoved++;
			}
		}
		g_PendingDynamite.RemoveAll();

		char buf[64];
		Q_snprintf( buf, sizeof(buf), "Defused %d pending dynamite", nRemoved );
		ClientPrint( pOwner, HUD_PRINTTALK, buf );
		return;
	}

	Vector vecOrigin = pEntity->GetAbsOrigin();

	// env_explosion (dlls/explode.cpp) is a stock HL2 entity already in the
	// build; ExplosionCreate() in that file uses the same
	// Create->KeyValue->Spawn(again) ordering so iMagnitude is baked into
	// m_spriteScale before the explosion actually triggers.
	CBaseEntity *pExplosion = CBaseEntity::Create( "env_explosion", vecOrigin, vec3_angle, pOwner );
	if ( !pExplosion )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Failed to create dynamite" );
		return;
	}

	char szMagnitude[16];
	Q_snprintf( szMagnitude, sizeof(szMagnitude), "%d", gm_dynamite_power.GetInt() );
	pExplosion->KeyValue( "iMagnitude", szMagnitude );
	pExplosion->AddSpawnFlags( SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS );
	pExplosion->m_nRenderMode = kRenderTransAdd;
	pExplosion->SetOwnerEntity( pOwner );
	pExplosion->Spawn();

	float flDelay = gm_dynamite_delay.GetFloat();

	// Delayed detonation via the standard entity I/O event queue (already
	// used elsewhere in this codebase, e.g. logicrelay.cpp) rather than a
	// custom Think - "Explode" is a FIELD_VOID input on env_explosion.
	// pExplosion is passed as both target and caller so a later
	// CancelEvents(pExplosion) (secondary fire / defuse) cancels exactly
	// this one event and nothing else.
	g_EventQueue.AddEvent( pExplosion, "Explode", flDelay, pOwner, pExplosion );

	g_PendingDynamite.AddToTail( pExplosion );

	pTool->PlayToolSound( "weapons/tripmine/mine_activate.wav" );

	char buf[160];
	Q_snprintf( buf, sizeof(buf), "Dynamite attached to %s, detonating in %.0fs", pEntity->GetClassname(), flDelay );
	ClientPrint( pOwner, HUD_PRINTTALK, buf );
}

//-----------------------------------------------------------------------------
// TOOL_STATUE
//-----------------------------------------------------------------------------
static void Simple_Statue_OnUse( CBasePlayer *pOwner, CWeaponTool *pTool, CBaseEntity *pEntity, bool bPrimary )
{
	if ( !bPrimary )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Statue: left click freezes/unfreezes a prop" );
		return;
	}

	IPhysicsObject *pPhys = pEntity->VPhysicsGetObject();
	if ( !pPhys )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "This entity has no physics object to freeze" );
		return;
	}

	int idx = Simple_FindHandle( g_StatuedEntities, pEntity );
	char buf[128];

	// Whole entity, not just the root - a ragdoll frozen through
	// VPhysicsGetObject() would keep every limb but the pelvis flopping.
	if ( idx >= 0 )
	{
		Simple_SetEntityMotionEnabled( pEntity, true );
		g_StatuedEntities.Remove( idx );
		Q_snprintf( buf, sizeof(buf), "%s is now unfrozen", pEntity->GetClassname() );
	}
	else
	{
		Simple_SetEntityMotionEnabled( pEntity, false );
		g_StatuedEntities.AddToTail( pEntity );
		Q_snprintf( buf, sizeof(buf), "%s is now a statue (frozen)", pEntity->GetClassname() );
	}

	pTool->PlayToolSound( "buttons/button9.wav" );
	ClientPrint( pOwner, HUD_PRINTTALK, buf );
}

//-----------------------------------------------------------------------------
// Dispatch: Tool_Simple_OnUse / OnTrace / OnThink (declared in tool_dispatch.h)
//-----------------------------------------------------------------------------
void Tool_Simple_OnUse( CWeaponTool *pTool, int nMode, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner || !pEntity )
		return;

	switch ( nMode )
	{
		case TOOL_IGNITE:
			Simple_Ignite_OnUse( pOwner, pTool, pEntity, bPrimary );
			break;

		case TOOL_MAGNETISE:
			Simple_Magnetise_OnUse( pOwner, pTool, pEntity, bPrimary );
			break;

		case TOOL_NOCOLLIDE:
			Simple_NoCollide_OnUse( pOwner, pTool, pEntity, tr, bPrimary );
			break;

		case TOOL_DYNAMITE:
			Simple_Dynamite_OnUse( pOwner, pTool, pEntity, bPrimary );
			break;

		case TOOL_STATUE:
			Simple_Statue_OnUse( pOwner, pTool, pEntity, bPrimary );
			break;

		default:
			break;
	}
}

void Tool_Simple_OnTrace( CWeaponTool *pTool, int nMode, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	if ( nMode == TOOL_NOCOLLIDE )
	{
		if ( pTool->GetPendingEntity() )
		{
			pTool->ClearPendingSelection();
			ClientPrint( pOwner, HUD_PRINTTALK, "No-collide selection cancelled" );
		}
		else
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "No valid target" );
		}
		return;
	}

	// Ignite/Magnetise/Dynamite/Statue all require an entity to act on.
	ClientPrint( pOwner, HUD_PRINTTALK, "No valid target" );
}

void Tool_Simple_OnThink( CWeaponTool *pTool, int nMode )
{
	if ( nMode == TOOL_MAGNETISE )
	{
		Simple_Magnetise_Think();
	}

	// Ignite/NoCollide/Dynamite/Statue need no per-frame work here: fire
	// burns out on its own, no-collide pairs stay disabled until removed
	// explicitly, dynamite detonates via its queued "Explode" event, and
	// statues just stay frozen until toggled again.
}

//-----------------------------------------------------------------------------
// Console command helpers
//-----------------------------------------------------------------------------
CON_COMMAND( gmod_simple_list, "List active Ignite/Magnetise/NoCollide/Dynamite/Statue tool state" )
{
	Msg( "Magnetised props: %d\n", g_MagnetisedEntities.Count() );
	Msg( "No-collide pairs: %d\n", g_NoCollidePairs.Count() );
	Msg( "Statued (frozen) props: %d\n", g_StatuedEntities.Count() );
	Msg( "Pending dynamite: %d\n", g_PendingDynamite.Count() );
}

CON_COMMAND( gmod_magnetise_clearall, "Remove magnetism from all props" )
{
	int nCount = g_MagnetisedEntities.Count();
	g_MagnetisedEntities.RemoveAll();

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		char buf[64];
		Q_snprintf( buf, sizeof(buf), "Cleared %d magnetised props", nCount );
		ClientPrint( pPlayer, HUD_PRINTTALK, buf );
	}
}

CON_COMMAND( gmod_nocollide_removeall, "Restore collision for all no-collided pairs" )
{
	int nRemoved = 0;
	for ( int i = g_NoCollidePairs.Count() - 1; i >= 0; i-- )
	{
		NoCollideInfo_t *pInfo = g_NoCollidePairs[i];
		CBaseEntity *pA = pInfo->hEntity1.Get();
		CBaseEntity *pB = pInfo->hEntity2.Get();

		Simple_SetEntityPairCollisions( pA, pB, true );

		delete pInfo;
		nRemoved++;
	}
	g_NoCollidePairs.RemoveAll();

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		char buf[64];
		Q_snprintf( buf, sizeof(buf), "Restored collision for %d pairs", nRemoved );
		ClientPrint( pPlayer, HUD_PRINTTALK, buf );
	}
}
