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
#include "rope.h"
#include "physics_prop_ragdoll.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Invisible point entity used as a keyframe_rope endpoint so the visual rope
// can be strung between the exact click positions (rope endpoints are entity
// origins, and there is no plain "info_target" in this codebase). Parented to
// the clicked prop so it follows it; left unparented for world attach points.
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS( gmod_ropeanchor, CPointEntity );

//-----------------------------------------------------------------------------
// Console variables - authentic GMod 9 per-mode constraint cvars. The names
// must match GMod 9 exactly (settings/context_panels/*.txt, third-party mods
// and Lua scripts all use them).
//-----------------------------------------------------------------------------
ConVar gm_weld_rope_type( "gm_weld_rope_type", "1", FCVAR_NONE, "Rope tool: visual cable type (1-5)" );
ConVar gm_weld_rope_width( "gm_weld_rope_width", "2", FCVAR_NONE, "Rope tool: visual cable width" );
ConVar gm_weld_rope_length( "gm_weld_rope_length", "0", FCVAR_NONE, "Rope tool: extra rope length added to the click distance" );
ConVar gm_weld_rope_forcelimit( "gm_weld_rope_forcelimit", "0", FCVAR_NONE, "Rope tool: force needed to snap the rope (0 = unbreakable)" );
ConVar gm_weld_rope_rigid( "gm_weld_rope_rigid", "0", FCVAR_NONE, "Rope tool: rigid rope (objects can't move closer either)" );

ConVar gm_weld_spr_springy( "gm_weld_spr_springy", "50", FCVAR_NONE, "Elastic tool: spring constant" );
ConVar gm_weld_spr_damping( "gm_weld_spr_damping", "1", FCVAR_NONE, "Elastic tool: spring damping" );
ConVar gm_weld_spr_type( "gm_weld_spr_type", "1", FCVAR_NONE, "Elastic tool: visual cable type (1-5)" );
ConVar gm_weld_spr_width( "gm_weld_spr_width", "2", FCVAR_NONE, "Elastic tool: visual cable width" );
ConVar gm_weld_spr_length( "gm_weld_spr_length", "0", FCVAR_NONE, "Elastic tool: extra natural length added to the click distance" );
ConVar gm_weld_spr_forcelimit( "gm_weld_spr_forcelimit", "0", FCVAR_NONE, "Elastic tool: force limit when rigid (0 = unbreakable)" );
ConVar gm_weld_spr_rigid( "gm_weld_spr_rigid", "0", FCVAR_NONE, "Elastic tool: create a rigid rope instead of a spring" );

ConVar gm_weld_weld_forcelimit( "gm_weld_weld_forcelimit", "0", FCVAR_NONE, "Weld tool: force needed to break the weld (0 = unbreakable)" );
ConVar gm_weld_weld_nocollide( "gm_weld_weld_nocollide", "0", FCVAR_NONE, "Weld tool: don't collide welded objects" );
ConVar gm_weld_weldez_forcelimit( "gm_weld_weldez_forcelimit", "0", FCVAR_NONE, "Easy Weld tool: force needed to break the weld (0 = unbreakable)" );
ConVar gm_weld_weldez_nocollide( "gm_weld_weldez_nocollide", "0", FCVAR_NONE, "Easy Weld tool: don't collide welded objects" );

ConVar gm_weld_ball_forcelimit( "gm_weld_ball_forcelimit", "0", FCVAR_NONE, "Ballsocket tool: force needed to break the joint (0 = unbreakable)" );
ConVar gm_weld_ball_nocollide( "gm_weld_ball_nocollide", "0", FCVAR_NONE, "Ballsocket tool: don't collide joined objects" );
ConVar gm_weld_ballez_forcelimit( "gm_weld_ballez_forcelimit", "0", FCVAR_NONE, "Easy Ball tool: force needed to break the joint (0 = unbreakable)" );
ConVar gm_weld_ballez_nocollide( "gm_weld_ballez_nocollide", "0", FCVAR_NONE, "Easy Ball tool: don't collide joined objects" );
ConVar gm_weld_nail_forcelimit( "gm_weld_nail_forcelimit", "0", FCVAR_NONE, "Nail gun: force needed to break the nail (0 = unbreakable)" );
ConVar gm_weld_nail_nocollide( "gm_weld_nail_nocollide", "0", FCVAR_NONE, "Nail gun: don't collide nailed objects" );

ConVar gm_weld_pulley_forcelimit( "gm_weld_pulley_forcelimit", "0", FCVAR_NONE, "Pulley tool: force needed to snap the pulley (0 = unbreakable)" );
ConVar gm_weld_pulley_rigid( "gm_weld_pulley_rigid", "0", FCVAR_NONE, "Pulley tool: rigid pulley rope" );
ConVar gm_weld_pulley_type( "gm_weld_pulley_type", "1", FCVAR_NONE, "Pulley tool: visual cable type (1-5)" );
ConVar gm_weld_pulley_width( "gm_weld_pulley_width", "2", FCVAR_NONE, "Pulley tool: visual cable width" );

ConVar gm_weld_slider_forcelimit( "gm_weld_slider_forcelimit", "0", FCVAR_NONE, "Slider tool: force needed to break the slider (0 = unbreakable)" );
ConVar gm_weld_slider_friction( "gm_weld_slider_friction", "0", FCVAR_NONE, "Slider tool: sliding friction" );
ConVar gm_weld_slider_type( "gm_weld_slider_type", "1", FCVAR_NONE, "Slider tool: visual cable type (1-5)" );
ConVar gm_weld_slider_width( "gm_weld_slider_width", "2", FCVAR_NONE, "Slider tool: visual cable width" );

ConVar gm_weld_axis_forcelimit( "gm_weld_axis_forcelimit", "0", FCVAR_NONE, "Axis tool: force needed to break the hinge (0 = unbreakable)" );
ConVar gm_weld_axis_friction( "gm_weld_axis_friction", "0", FCVAR_NONE, "Axis tool: rotational friction torque" );
ConVar gm_weld_axis_nocollide( "gm_weld_axis_nocollide", "0", FCVAR_NONE, "Axis tool: don't collide hinged objects" );

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
	bool				bVisualOnly;		// rope strung between two world points - no physics constraint at all
	bool				bDisabledCollision;	// true if we disabled collisions between hEntity1/hEntity2
	IPhysicsConstraint	*pConstraint;		// used by every mode except Elastic
	IPhysicsSpring		*pSpring;			// used only by Elastic
	EHANDLE				hRope;				// visual keyframe_rope (rope-like modes only)
	EHANDLE				hAnchor1;			// gmod_ropeanchor rope endpoints at the click positions
	EHANDLE				hAnchor2;
	float				flCreateTime;

	ConstraintInfo_t()
	{
		nMode = TOOL_NONE;
		bWorldAttached = false;
		bVisualOnly = false;
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

// Rope-like modes get a visible cable strung between the two click points.
static bool WantsRopeVisual( int nMode )
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
// Per-mode settings lookups, driven by the authentic GMod 9 cvars above.
//-----------------------------------------------------------------------------
static float ConstraintForceLimitForMode( int nMode )
{
	switch ( nMode )
	{
		case TOOL_ROPE:			return gm_weld_rope_forcelimit.GetFloat();
		case TOOL_ELASTIC:		return gm_weld_spr_forcelimit.GetFloat();
		case TOOL_WELD:			return gm_weld_weld_forcelimit.GetFloat();
		case TOOL_EASYWELD:		return gm_weld_weldez_forcelimit.GetFloat();
		case TOOL_BALLSOCKET:	return gm_weld_ball_forcelimit.GetFloat();
		case TOOL_EASYBALL:		return gm_weld_ballez_forcelimit.GetFloat();
		case TOOL_NAILGUN:		return gm_weld_nail_forcelimit.GetFloat();
		case TOOL_PULLEY:		return gm_weld_pulley_forcelimit.GetFloat();
		case TOOL_SLIDER:		return gm_weld_slider_forcelimit.GetFloat();
		case TOOL_AXIS:			return gm_weld_axis_forcelimit.GetFloat();
	}
	return 0.0f;
}

static bool ConstraintNoCollideForMode( int nMode )
{
	switch ( nMode )
	{
		case TOOL_WELD:			return gm_weld_weld_nocollide.GetBool();
		case TOOL_EASYWELD:		return gm_weld_weldez_nocollide.GetBool();
		case TOOL_BALLSOCKET:	return gm_weld_ball_nocollide.GetBool();
		case TOOL_EASYBALL:		return gm_weld_ballez_nocollide.GetBool();
		case TOOL_NAILGUN:		return gm_weld_nail_nocollide.GetBool();
		case TOOL_AXIS:			return gm_weld_axis_nocollide.GetBool();
	}
	return false;
}

// GMod 9 rope "type" picks the cable material used for the visual.
static const char *RopeMaterialForType( int nType )
{
	switch ( nType )
	{
		case 2:		return "cable/rope.vmt";
		case 3:		return "cable/chain.vmt";
		case 4:		return "cable/redlaser.vmt";
		case 5:		return "cable/physbeam.vmt";
	}
	return "cable/cable.vmt";
}

static void RopeVisualSettingsForMode( int nMode, const char **ppszMaterial, int *pnWidth )
{
	int nType = 1;
	float flWidth = 2.0f;

	switch ( nMode )
	{
		case TOOL_ROPE:
			nType = gm_weld_rope_type.GetInt();
			flWidth = gm_weld_rope_width.GetFloat();
			break;
		case TOOL_ELASTIC:
			nType = gm_weld_spr_type.GetInt();
			flWidth = gm_weld_spr_width.GetFloat();
			break;
		case TOOL_PULLEY:
			nType = gm_weld_pulley_type.GetInt();
			flWidth = gm_weld_pulley_width.GetFloat();
			break;
		case TOOL_SLIDER:
			nType = gm_weld_slider_type.GetInt();
			flWidth = gm_weld_slider_width.GetFloat();
			break;
	}

	*ppszMaterial = RopeMaterialForType( nType );
	*pnWidth = max( (int)( flWidth + 0.5f ), 1 );
}

//-----------------------------------------------------------------------------
// Resolves the physics object a click actually landed on. Ragdolls are made
// of many physics objects (one per phys bone), so constraining a ragdoll must
// use the clicked limb's object (trace physicsbone), not VPhysicsGetObject()
// - that's what lets ropes tied to a hand/foot make swings.
//-----------------------------------------------------------------------------
static IPhysicsObject *PhysObjectFromBone( CBaseEntity *pEntity, int nPhysBone )
{
	if ( !pEntity )
		return NULL;

	CRagdollProp *pRagdollProp = dynamic_cast<CRagdollProp*>( pEntity );
	if ( pRagdollProp )
	{
		ragdoll_t *pRagdoll = pRagdollProp->GetRagdoll();

		if ( nPhysBone >= 0 && nPhysBone < pRagdoll->listCount && pRagdoll->list[nPhysBone].pObject )
			return pRagdoll->list[nPhysBone].pObject;

		return ( pRagdoll->listCount > 0 ) ? pRagdoll->list[0].pObject : NULL;
	}

	return pEntity->VPhysicsGetObject();
}

//-----------------------------------------------------------------------------
// Creates a gmod_ropeanchor at vecPos, parented to pAttachTo (or free-standing
// for a world attach point), for use as a keyframe_rope endpoint.
//-----------------------------------------------------------------------------
static CBaseEntity *CreateRopeAnchor( CBaseEntity *pAttachTo, const Vector &vecPos )
{
	CBaseEntity *pAnchor = CBaseEntity::Create( "gmod_ropeanchor", vecPos, vec3_angle, NULL );
	if ( !pAnchor )
		return NULL;

	if ( pAttachTo )
		pAnchor->SetParent( pAttachTo );

	return pAnchor;
}

//-----------------------------------------------------------------------------
// Strings the visible cable between the two click points and stores the
// created entities on pInfo. Purely cosmetic - failure here never fails the
// constraint itself.
//-----------------------------------------------------------------------------
static void AttachRopeVisual( ConstraintInfo_t *pInfo, int nMode, CBaseEntity *pEnt1, const Vector &vecPos1,
							   CBaseEntity *pEnt2, const Vector &vecPos2 )
{
	CBaseEntity *pAnchor1 = CreateRopeAnchor( pEnt1, vecPos1 );
	CBaseEntity *pAnchor2 = CreateRopeAnchor( pEnt2, vecPos2 );

	if ( !pAnchor1 || !pAnchor2 )
	{
		if ( pAnchor1 )
			UTIL_Remove( pAnchor1 );
		if ( pAnchor2 )
			UTIL_Remove( pAnchor2 );
		return;
	}

	const char *pszMaterial = "cable/cable.vmt";
	int nWidth = 2;
	RopeVisualSettingsForMode( nMode, &pszMaterial, &nWidth );

	CRopeKeyframe *pRope = CRopeKeyframe::Create( pAnchor1, pAnchor2, 0, 0, nWidth, pszMaterial, 8 );
	if ( !pRope )
	{
		UTIL_Remove( pAnchor1 );
		UTIL_Remove( pAnchor2 );
		return;
	}

	pInfo->hAnchor1 = pAnchor1;
	pInfo->hAnchor2 = pAnchor2;
	pInfo->hRope = pRope;
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
			length.InitWorldspace( pPhys1, pPhys2, vecPos1, vecPos2, gm_weld_rope_rigid.GetBool() );

			// GMod 9: the rope's length is the click distance plus the "extra
			// rope length" setting (which may be negative to make it taut).
			length.totalLength = max( length.totalLength + gm_weld_rope_length.GetFloat(), 1.0f );
			length.constraint.forceLimit = gm_weld_rope_forcelimit.GetFloat();

			return physenv->CreateLengthConstraint( pPhys1, pPhys2, NULL, length );
		}

		case TOOL_ELASTIC:
		{
			if ( gm_weld_spr_rigid.GetBool() )
			{
				// Rigid elastic behaves like a rigid rope in GMod 9.
				constraint_lengthparams_t length;
				length.Defaults();
				length.InitWorldspace( pPhys1, pPhys2, vecPos1, vecPos2, true );
				length.totalLength = max( length.totalLength + gm_weld_spr_length.GetFloat(), 1.0f );
				length.constraint.forceLimit = gm_weld_spr_forcelimit.GetFloat();

				return physenv->CreateLengthConstraint( pPhys1, pPhys2, NULL, length );
			}

			springparams_t spring;
			spring.constant = gm_weld_spr_springy.GetFloat();
			spring.damping = gm_weld_spr_damping.GetFloat();
			spring.naturalLength = max( vecPos1.DistTo( vecPos2 ) + gm_weld_spr_length.GetFloat(), 1.0f );
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
			fixed.constraint.forceLimit = ConstraintForceLimitForMode( nMode );

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
			ballsocket.constraint.forceLimit = ConstraintForceLimitForMode( nMode );

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
			pulley.isRigid = gm_weld_pulley_rigid.GetBool();
			pulley.constraint.forceLimit = gm_weld_pulley_forcelimit.GetFloat();

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
			hinge.hingeAxis.torque = gm_weld_axis_friction.GetFloat();
			hinge.constraint.forceLimit = gm_weld_axis_forcelimit.GetFloat();

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
			sliding.friction = gm_weld_slider_friction.GetFloat();
			sliding.constraint.forceLimit = gm_weld_slider_forcelimit.GetFloat();

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

	if ( pInfo->hRope.Get() )
	{
		UTIL_Remove( pInfo->hRope.Get() );
		pInfo->hRope = NULL;
	}

	if ( pInfo->hAnchor1.Get() )
	{
		UTIL_Remove( pInfo->hAnchor1.Get() );
		pInfo->hAnchor1 = NULL;
	}

	if ( pInfo->hAnchor2.Get() )
	{
		UTIL_Remove( pInfo->hAnchor2.Get() );
		pInfo->hAnchor2 = NULL;
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
// Attempts to build and register a constraint between the two click points.
// Either entity may be NULL to mean "that click landed on the world":
//  - one NULL: attach the other entity to the world (modes where
//    SupportsWorldAttach() is true - caller is expected to have checked)
//  - both NULL: purely visual rope strung between two world points
//    (TOOL_ROPE only - caller is expected to have checked)
//-----------------------------------------------------------------------------
static bool CreateAndRegisterConstraint( int nMode, CBaseEntity *pEnt1, const Vector &vecClick1, int nBone1,
										  CBaseEntity *pEnt2, const Vector &vecClick2, int nBone2 )
{
	// Keep the real physics entity in slot 1 so the registry/cleanup logic
	// has one shape regardless of which click hit the world.
	CBaseEntity *pFirst = pEnt1;
	CBaseEntity *pSecond = pEnt2;
	Vector vecPos1 = vecClick1;
	Vector vecPos2 = vecClick2;
	int nFirstBone = nBone1;
	int nSecondBone = nBone2;

	if ( !pFirst && pSecond )
	{
		pFirst = pSecond;
		pSecond = NULL;
		vecPos1 = vecClick2;
		vecPos2 = vecClick1;
		nFirstBone = nBone2;
		nSecondBone = nBone1;
	}

	ConstraintInfo_t *pInfo = new ConstraintInfo_t;
	pInfo->nMode = nMode;
	pInfo->flCreateTime = gpGlobals->curtime;

	if ( !pFirst )
	{
		// World-to-world: no physics, just the cable.
		pInfo->bVisualOnly = true;
		pInfo->bWorldAttached = true;
	}
	else
	{
		IPhysicsObject *pPhys1 = PhysObjectFromBone( pFirst, nFirstBone );
		IPhysicsObject *pPhys2 = pSecond ? PhysObjectFromBone( pSecond, nSecondBone ) : g_PhysWorldObject;

		if ( !pPhys1 || !pPhys2 )
		{
			delete pInfo;
			return false;
		}

		IPhysicsSpring *pSpring = NULL;
		IPhysicsConstraint *pConstraint = CreateConstraintForMode( nMode, pPhys1, pPhys2, vecPos1, vecPos2, &pSpring );

		if ( !pConstraint && !pSpring )
		{
			delete pInfo;
			return false;
		}

		pInfo->hEntity1 = pFirst;
		pInfo->hEntity2 = pSecond;
		pInfo->bWorldAttached = ( pSecond == NULL );
		pInfo->pConstraint = pConstraint;
		pInfo->pSpring = pSpring;

		if ( pSecond && ConstraintNoCollideForMode( nMode ) )
		{
			physenv->DisableCollisions( pPhys1, pPhys2 );
			pInfo->bDisabledCollision = true;
		}
	}

	if ( WantsRopeVisual( nMode ) )
	{
		AttachRopeVisual( pInfo, nMode, pFirst, vecPos1, pSecond, vecPos2 );
	}

	if ( pInfo->bVisualOnly && !pInfo->hRope.Get() )
	{
		// Nothing physical and the cable failed - nothing to keep.
		delete pInfo;
		return false;
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
// Removes world-to-world ropes with an endpoint near vecPos (right-click on
// the world - those ropes touch no prop, so RemoveConstraintsOnEntity can
// never reach them).
//-----------------------------------------------------------------------------
static void RemoveVisualOnlyRopesNear( CBasePlayer *pOwner, const Vector &vecPos )
{
	int nRemoved = 0;

	for ( int i = g_ToolConstraints.Count() - 1; i >= 0; i-- )
	{
		ConstraintInfo_t *pInfo = g_ToolConstraints[i];
		if ( !pInfo->bVisualOnly )
			continue;

		CBaseEntity *pAnchor1 = pInfo->hAnchor1.Get();
		CBaseEntity *pAnchor2 = pInfo->hAnchor2.Get();

		bool bNear = ( pAnchor1 && pAnchor1->GetAbsOrigin().DistTo( vecPos ) < 64.0f ) ||
					 ( pAnchor2 && pAnchor2->GetAbsOrigin().DistTo( vecPos ) < 64.0f );

		if ( bNear )
		{
			DestroyConstraintInfo( pInfo );
			g_ToolConstraints.Remove( i );
			delete pInfo;
			nRemoved++;
		}
	}

	if ( nRemoved && pOwner )
	{
		char szBuf[64];
		Q_snprintf( szBuf, sizeof( szBuf ), "Removed %d rope(s)", nRemoved );
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

		bool bInvalid;
		if ( pInfo->bVisualOnly )
		{
			// No entities to validate - only dies if the rope itself is gone.
			bInvalid = ( pInfo->hRope.Get() == NULL );
		}
		else
		{
			CBaseEntity *pEnt1 = pInfo->hEntity1.Get();
			CBaseEntity *pEnt2 = pInfo->hEntity2.Get();

			bInvalid = ( !pEnt1 || !pEnt1->VPhysicsGetObject() );

			if ( !pInfo->bWorldAttached && ( !pEnt2 || !pEnt2->VPhysicsGetObject() ) )
				bInvalid = true;
		}

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

	if ( !pTool->HasPendingSelection() )
	{
		IPhysicsObject *pPhys = PhysObjectFromBone( pEntity, tr.physicsbone );
		if ( !pPhys )
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "That object has no physics" );
			return;
		}

		pTool->SetPendingSelection( pEntity, tr.endpos, tr.physicsbone );
		pTool->PlayToolSound( "buttons/button14.wav" );

		if ( IsVerboseConstraintMode( nMode ) )
		{
			char szBuf[128];
			Q_snprintf( szBuf, sizeof( szBuf ), "%s: selected %s - click a second prop", GetConstraintModeName( nMode ), pEntity->GetClassname() );
			ClientPrint( pOwner, HUD_PRINTTALK, szBuf );
		}
		return;
	}

	CBaseEntity *pPending = pTool->GetPendingEntity();	// NULL when the first click landed on the world

	if ( pPending && pPending == pEntity )
	{
		pTool->ClearPendingSelection();
		ClientPrint( pOwner, HUD_PRINTTALK, "Selection cancelled" );
		return;
	}

	IPhysicsObject *pPhys2 = PhysObjectFromBone( pEntity, tr.physicsbone );
	if ( !pPhys2 )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "That object has no physics" );
		return;
	}

	bool bOk = CreateAndRegisterConstraint( nMode, pPending, pTool->GetPendingPos(), pTool->GetPendingPhysBone(),
											pEntity, tr.endpos, tr.physicsbone );

	char szBuf[128];
	if ( bOk )
	{
		pTool->PlayToolSound( "weapons/physcannon/energy_sing_loop4.wav" );
		Q_snprintf( szBuf, sizeof( szBuf ), "%s created between %s and %s", GetConstraintModeName( nMode ),
			pPending ? pPending->GetClassname() : "world", pEntity->GetClassname() );
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
	{
		// Right-click on the world: clean up world-to-world ropes near the
		// hit point (they touch no prop, so entity-based removal misses them).
		if ( tr.fraction < 1.0f )
			RemoveVisualOnlyRopesNear( pOwner, tr.endpos );
		return;
	}

	if ( !pTool->HasPendingSelection() )
	{
		// First click landed on the world - a valid rope starting point for
		// the world-attach modes; other constraint modes keep waiting for a
		// prop.
		if ( SupportsWorldAttach( nMode ) && tr.fraction < 1.0f )
		{
			pTool->SetPendingSelection( NULL, tr.endpos );
			pTool->PlayToolSound( "buttons/button14.wav" );

			if ( IsVerboseConstraintMode( nMode ) )
			{
				char szBuf[128];
				Q_snprintf( szBuf, sizeof( szBuf ), "%s: selected world point - click a prop or another point", GetConstraintModeName( nMode ) );
				ClientPrint( pOwner, HUD_PRINTTALK, szBuf );
			}
		}
		return;
	}

	if ( !SupportsWorldAttach( nMode ) )
	{
		pTool->ClearPendingSelection();
		ClientPrint( pOwner, HUD_PRINTTALK, "Selection cancelled - aim at a prop" );
		return;
	}

	if ( tr.fraction >= 1.0f )
		return;	// shot into the sky - keep the pending selection

	CBaseEntity *pPending = pTool->GetPendingEntity();

	if ( !pPending && nMode != TOOL_ROPE )
	{
		// Both clicks on the world only makes sense as a decorative rope.
		pTool->ClearPendingSelection();
		ClientPrint( pOwner, HUD_PRINTTALK, "Selection cancelled - aim at a prop" );
		return;
	}

	bool bOk = CreateAndRegisterConstraint( nMode, pPending, pTool->GetPendingPos(), pTool->GetPendingPhysBone(),
											NULL, tr.endpos, -1 );

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
	if ( pTool->HasPendingSelection() && ( gpGlobals->curtime - pTool->GetPendingTime() ) > 10.0f )
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
CON_COMMAND( gmod_constraint_removeall, "Remove all tool constraints" )
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
CON_COMMAND( gmod_constraint_list, "List all tool constraints" )
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
