//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Tool System - Face Poser (10) / Eyes Poser (11)
//
//          Real GMod9's face poser is a full flex-slider UI driven by client
//          panels; that is out of scope here. This is an honest, small first
//          pass: selecting a posable target and (for the eyes poser) pointing
//          its look target at the tool's owner via the existing AI look-target
//          system. No flex controller UI is implemented.
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// How long an eyes-poser look target lasts once set (seconds).
//-----------------------------------------------------------------------------
#define POSER_LOOKTARGET_DURATION	8.0f
#define POSER_SELECTION_TIMEOUT		10.0f

//-----------------------------------------------------------------------------
// Returns true if pEntity has a valid, animatable studio model - the minimum
// bar for something to be "posable" at all (props/ragdolls/NPCs qualify,
// brushes/world/point entities do not).
//-----------------------------------------------------------------------------
static bool Poser_IsValidTarget( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return false;

	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating*>( pEntity );
	if ( !pAnimating )
		return false;

	return pAnimating->GetModelPtr() != NULL;
}

//-----------------------------------------------------------------------------
// Tool_Poser_OnUse - primary click selects a posable target; secondary click
// (or re-clicking) just clears the current selection.
//-----------------------------------------------------------------------------
void Tool_Poser_OnUse( CWeaponTool *pTool, int nMode, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	const char *pszToolLabel = ( nMode == TOOL_EYESPOSER ) ? "Eyes Poser" : "Face Poser";

	if ( !bPrimary )
	{
		if ( pTool->GetPendingEntity() )
		{
			pTool->ClearPendingSelection();
			ClientPrint( pOwner, HUD_PRINTTALK, "%s: selection cleared", pszToolLabel );
		}
		return;
	}

	if ( !Poser_IsValidTarget( pEntity ) )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "%s: %s has no posable model", pszToolLabel, pEntity ? pEntity->GetClassname() : "target" );
		return;
	}

	pTool->SetPendingSelection( pEntity, tr.endpos );
	pTool->PlayToolSound( "buttons/button14.wav" );

	if ( nMode == TOOL_EYESPOSER )
	{
		// Full flex/eye slider UI isn't implemented. As a small, honest bit of
		// real behavior, if the target is an NPC we redirect its AI look
		// target at the tool's owner using the existing look-target system
		// (CAI_BaseNPC::AddLookTarget) rather than only printing a message.
		CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>( pEntity );
		if ( pNPC )
		{
			pNPC->AddLookTarget( pOwner, 1.0f, POSER_LOOKTARGET_DURATION );
			ClientPrint( pOwner, HUD_PRINTTALK, "Eyes Poser: %s selected - now looking at you (flex slider UI not implemented)", pEntity->GetClassname() );
		}
		else
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "Eyes Poser: selected %s - not an NPC, no look target to set (flex slider UI not implemented)", pEntity->GetClassname() );
		}
	}
	else
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Face Poser: selected %s - flex slider UI not implemented, use +lookat or manual expressions", pEntity->GetClassname() );
	}
}

//-----------------------------------------------------------------------------
// Tool_Poser_OnTrace - hit world or nothing: deselect.
//-----------------------------------------------------------------------------
void Tool_Poser_OnTrace( CWeaponTool *pTool, int nMode, trace_t &tr, bool bPrimary )
{
	if ( !pTool->GetPendingEntity() )
		return;

	pTool->ClearPendingSelection();

	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( pOwner )
	{
		const char *pszToolLabel = ( nMode == TOOL_EYESPOSER ) ? "Eyes Poser" : "Face Poser";
		ClientPrint( pOwner, HUD_PRINTTALK, "%s: selection cleared", pszToolLabel );
	}
}

//-----------------------------------------------------------------------------
// Tool_Poser_OnThink - just times out a stale pending selection.
//-----------------------------------------------------------------------------
void Tool_Poser_OnThink( CWeaponTool *pTool, int nMode )
{
	CBaseEntity *pPending = pTool->GetPendingEntity();
	if ( !pPending )
		return;

	if ( ( gpGlobals->curtime - pTool->GetPendingTime() ) <= POSER_SELECTION_TIMEOUT )
		return;

	pTool->ClearPendingSelection();

	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( pOwner )
	{
		const char *pszToolLabel = ( nMode == TOOL_EYESPOSER ) ? "Eyes Poser" : "Face Poser";
		ClientPrint( pOwner, HUD_PRINTTALK, "%s: selection timed out", pszToolLabel );
	}
}
