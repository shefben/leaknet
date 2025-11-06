//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Camera Tool - Implementation of Camera tool mode
//          Based on Garry's Mod tool system analysis (found "gm_context camera")
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables for camera tool
//-----------------------------------------------------------------------------
ConVar bm_camera_fov("bm_camera_fov", "90", FCVAR_ARCHIVE, "Camera tool field of view");
ConVar bm_camera_quality("bm_camera_quality", "high", FCVAR_ARCHIVE, "Camera tool screenshot quality");

//-----------------------------------------------------------------------------
// Camera tool class - implements TOOL_CAMERA mode
//-----------------------------------------------------------------------------
class CToolCamera : public CWeaponTool
{
	DECLARE_CLASS( CToolCamera, CWeaponTool );

public:
	CToolCamera() : m_flLastPhotoTime(0.0f), m_nPhotoCount(0) {}

	virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void OnToolThink();

private:
	void TakePhoto( const Vector &vecTarget );
	void CreatePhotoEffect( const Vector &vecPos );
	void PlayShutterSound();
	void AdjustCameraSettings();

	// Camera state
	float	m_flLastPhotoTime;
	int		m_nPhotoCount;
};

//-----------------------------------------------------------------------------
// Tool implementation for Camera mode
//-----------------------------------------------------------------------------
void CToolCamera::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	if ( !bPrimary )
	{
		// Secondary attack - adjust camera settings
		AdjustCameraSettings();
		return;
	}

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	// Check photo delay (prevent spam)
	if ( gpGlobals->curtime - m_flLastPhotoTime < 1.0f )
		return;

	// Take photo targeting the entity
	TakePhoto( tr.endpos );

	// Create photo effect
	CreatePhotoEffect( tr.endpos );

	// Play camera shutter sound - matching IDA finding "NPC_CScanner.TakePhoto"
	PlayShutterSound();

	// Update photo state
	m_flLastPhotoTime = gpGlobals->curtime;
	m_nPhotoCount++;

	DevMsg( "Camera tool: Photo #%d taken targeting %s\n",
		m_nPhotoCount,
		pEntity ? pEntity->GetClassname() : "world" );
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Camera mode
//-----------------------------------------------------------------------------
void CToolCamera::OnToolTrace( trace_t &tr, bool bPrimary )
{
	if ( !bPrimary )
		return;

	// Take photo of empty space
	TakePhoto( tr.endpos );
	CreatePhotoEffect( tr.endpos );
	PlayShutterSound();

	m_flLastPhotoTime = gpGlobals->curtime;
	m_nPhotoCount++;

	DevMsg( "Camera tool: Photo #%d taken of empty space\n", m_nPhotoCount );
}

//-----------------------------------------------------------------------------
// Tool think for Camera mode
//-----------------------------------------------------------------------------
void CToolCamera::OnToolThink()
{
	// Camera tool doesn't need continuous thinking
	// Could add viewfinder effects here
}

//-----------------------------------------------------------------------------
// Take photo
//-----------------------------------------------------------------------------
void CToolCamera::TakePhoto( const Vector &vecTarget )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
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
void CToolCamera::CreatePhotoEffect( const Vector &vecPos )
{
	// Create camera flash effect
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = 200.0f;  // Flash brightness
	data.m_flScale = 3.0f;        // Flash size
	data.m_nColor = 255;          // White flash

	DispatchEffect( "GlowSprite", data );

	// Create some particles to simulate photo capture
	data.m_vOrigin = GetAbsOrigin();
	data.m_vNormal = Vector(0, 0, 1);
	data.m_flScale = 1.0f;

	DispatchEffect( "MuzzleFlash", data );
}

//-----------------------------------------------------------------------------
// Play shutter sound - matching Garry's Mod IDA finding
//-----------------------------------------------------------------------------
void CToolCamera::PlayShutterSound()
{
	// Use the same sound as found in Garry's Mod IDA analysis
	PlayToolSound( "NPC_CScanner.TakePhoto" );

	// Also play a mechanical shutter sound
	EmitSound( "Camera.Snapshot" );
}

//-----------------------------------------------------------------------------
// Adjust camera settings
//-----------------------------------------------------------------------------
void CToolCamera::AdjustCameraSettings()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
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
	ClientPrint( pOwner, HUD_PRINTTALK, "Camera FOV set to %.0f", bm_camera_fov.GetFloat() );
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
		ClientPrint( pPlayer, HUD_PRINTTALK, "Camera tool must be equipped and selected" );
		return;
	}

	// In a full implementation, this would open the camera context menu
	// For now, just show camera info
	ClientPrint( pPlayer, HUD_PRINTTALK, "Camera Tool Settings:" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "FOV: %.0f", bm_camera_fov.GetFloat() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Quality: %s", bm_camera_quality.GetString() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Use secondary fire to adjust settings" );
}