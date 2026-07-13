//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Camera Tool - Implementation of Camera tool mode
//          Based on Garry's Mod tool system analysis (found "gm_context camera")
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "engine/IEngineSound.h"

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
// Console variables for camera tool
//-----------------------------------------------------------------------------
ConVar bm_camera_fov("bm_camera_fov", "90", FCVAR_ARCHIVE, "Camera tool field of view");
ConVar bm_camera_quality("bm_camera_quality", "high", FCVAR_ARCHIVE, "Camera tool screenshot quality");

//-----------------------------------------------------------------------------
// Per-weapon camera state - CWeaponTool is not subclassed, so the state that
// used to live in CToolCamera's member variables now lives here, keyed by
// the weapon's EHANDLE (mirrors the g_RemoverStates pattern in tool_remover.cpp).
//-----------------------------------------------------------------------------
struct CameraState_t
{
	EHANDLE	hTool;
	float	flLastPhotoTime;
	int		nPhotoCount;

	CameraState_t()
	{
		flLastPhotoTime = 0.0f;
		nPhotoCount = 0;
	}
};

static CUtlVector<CameraState_t*> g_CameraStates;

static CameraState_t *FindCameraState( CWeaponTool *pTool )
{
	for ( int i = 0; i < g_CameraStates.Count(); i++ )
	{
		if ( g_CameraStates[i]->hTool == pTool )
			return g_CameraStates[i];
	}
	return NULL;
}

static CameraState_t *GetCameraState( CWeaponTool *pTool )
{
	CameraState_t *pState = FindCameraState( pTool );
	if ( !pState )
	{
		pState = new CameraState_t;
		pState->hTool = pTool;
		g_CameraStates.AddToTail( pState );
	}
	return pState;
}

// Removes state records for weapons that no longer exist.
static void CleanupCameraStates()
{
	for ( int i = g_CameraStates.Count() - 1; i >= 0; i-- )
	{
		if ( !g_CameraStates[i]->hTool.Get() )
		{
			delete g_CameraStates[i];
			g_CameraStates.Remove( i );
		}
	}
}

//-----------------------------------------------------------------------------
// Forward declarations of tool helpers
//-----------------------------------------------------------------------------
static void TakePhoto( CWeaponTool *pTool, const Vector &vecTarget );
static void CreatePhotoEffect( CWeaponTool *pTool, const Vector &vecPos );
static void PlayShutterSound( CWeaponTool *pTool );
static void AdjustCameraSettings( CWeaponTool *pTool );

//-----------------------------------------------------------------------------
// Tool implementation for Camera mode
//-----------------------------------------------------------------------------
void Tool_Camera_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	if ( !bPrimary )
	{
		// Secondary attack - adjust camera settings
		AdjustCameraSettings( pTool );
		return;
	}

	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	CameraState_t *pState = GetCameraState( pTool );

	// Check photo delay (prevent spam)
	if ( gpGlobals->curtime - pState->flLastPhotoTime < 1.0f )
		return;

	// Take photo targeting the entity
	TakePhoto( pTool, tr.endpos );

	// Create photo effect
	CreatePhotoEffect( pTool, tr.endpos );

	// Play camera shutter sound - matching IDA finding "NPC_CScanner.TakePhoto"
	PlayShutterSound( pTool );

	// Update photo state
	pState->flLastPhotoTime = gpGlobals->curtime;
	pState->nPhotoCount++;

	DevMsg( "Camera tool: Photo #%d taken targeting %s\n",
		pState->nPhotoCount,
		pEntity ? pEntity->GetClassname() : "world" );
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Camera mode
//-----------------------------------------------------------------------------
void Tool_Camera_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary )
{
	if ( !bPrimary )
		return;

	CameraState_t *pState = GetCameraState( pTool );

	// Take photo of empty space
	TakePhoto( pTool, tr.endpos );
	CreatePhotoEffect( pTool, tr.endpos );
	PlayShutterSound( pTool );

	pState->flLastPhotoTime = gpGlobals->curtime;
	pState->nPhotoCount++;

	DevMsg( "Camera tool: Photo #%d taken of empty space\n", pState->nPhotoCount );
}

//-----------------------------------------------------------------------------
// Tool think for Camera mode
//-----------------------------------------------------------------------------
void Tool_Camera_OnThink( CWeaponTool *pTool )
{
	// Camera tool doesn't need continuous thinking - could add viewfinder
	// effects here. Just keep the per-weapon state registry tidy.
	CleanupCameraStates();
}

//-----------------------------------------------------------------------------
// Take photo
//-----------------------------------------------------------------------------
static void TakePhoto( CWeaponTool *pTool, const Vector &vecTarget )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	// In a full implementation, this would:
	// 1. Capture the current view
	// 2. Apply any camera effects (sepia, black & white, etc.)
	// 3. Save the screenshot to disk
	// 4. Show preview to player

	DevMsg( "Taking photo focused on (%f, %f, %f)\n",
		vecTarget.x, vecTarget.y, vecTarget.z );

	// For now, just send a message to the client
	engine->ClientCommand( pOwner->edict(), "screenshot\n" );

	// Create a brief screen flash effect (like camera flash)
	// This would be sent to the client for rendering
}

//-----------------------------------------------------------------------------
// Create photo effect
//-----------------------------------------------------------------------------
static void CreatePhotoEffect( CWeaponTool *pTool, const Vector &vecPos )
{
	// Create camera flash effect
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = 200.0f;  // Flash brightness
	data.m_flScale = 3.0f;        // Flash size
	data.m_nColor = 255;          // White flash

	DispatchEffect( "GlowSprite", data );

	// Create some particles to simulate photo capture
	data.m_vOrigin = pTool->GetAbsOrigin();
	data.m_vNormal = Vector(0, 0, 1);
	data.m_flScale = 1.0f;

	DispatchEffect( "MuzzleFlash", data );
}

//-----------------------------------------------------------------------------
// Play shutter sound - matching Garry's Mod IDA finding
//-----------------------------------------------------------------------------
static void PlayShutterSound( CWeaponTool *pTool )
{
	// Use the same sound as found in Garry's Mod IDA analysis
	pTool->PlayToolSound( "NPC_CScanner.TakePhoto" );

	// Also play a mechanical shutter sound
	pTool->EmitSound( "Camera.Snapshot" );
}

//-----------------------------------------------------------------------------
// Adjust camera settings
//-----------------------------------------------------------------------------
static void AdjustCameraSettings( CWeaponTool *pTool )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	// Cycle through different camera modes
	// This could open a context menu as found in IDA: "gm_context camera"

	DevMsg( "Camera tool: Adjusting camera settings\n" );

	// In a full implementation, this would:
	// 1. Open camera settings UI
	// 2. Allow adjustment of FOV, filters, quality, etc.
	// 3. Preview changes in real-time

	// For now, just cycle FOV
	float currentFOV = bm_camera_fov.GetFloat();
	if ( currentFOV <= 60 )
		bm_camera_fov.SetValue( 90 );
	else if ( currentFOV <= 90 )
		bm_camera_fov.SetValue( 120 );
	else
		bm_camera_fov.SetValue( 60 );

	// Inform player
	ClientPrintf( pOwner, HUD_PRINTTALK, "Camera FOV set to %.0f", bm_camera_fov.GetFloat() );
}

//-----------------------------------------------------------------------------
// Console command for camera context menu (matching IDA finding)
//-----------------------------------------------------------------------------
CON_COMMAND( bm_context_camera, "Opens camera tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has camera tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_CAMERA )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Camera tool must be equipped and selected" );
		return;
	}

	// In a full implementation, this would open the camera context menu
	// For now, just show camera info
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Camera Tool Settings:" );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "FOV: %.0f", bm_camera_fov.GetFloat() );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Quality: %s", bm_camera_quality.GetString() );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Use secondary fire to adjust settings" );
}
