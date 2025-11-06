//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Color Tool - Implementation of Color/tint changing tool
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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables for color tool
//-----------------------------------------------------------------------------
ConVar bm_color_r("bm_color_r", "255", FCVAR_ARCHIVE, "Red component (0-255)");
ConVar bm_color_g("bm_color_g", "255", FCVAR_ARCHIVE, "Green component (0-255)");
ConVar bm_color_b("bm_color_b", "255", FCVAR_ARCHIVE, "Blue component (0-255)");
ConVar bm_color_a("bm_color_a", "255", FCVAR_ARCHIVE, "Alpha component (0-255)");
ConVar bm_color_mode("bm_color_mode", "0", FCVAR_ARCHIVE, "Color mode: 0=RGB, 1=HSV");

//-----------------------------------------------------------------------------
// Preset colors - common colors for quick selection
//-----------------------------------------------------------------------------
struct ColorPreset_t
{
	const char *pszName;
	int r, g, b, a;
};

static ColorPreset_t g_ColorPresets[] =
{
	{ "White",		255, 255, 255, 255 },
	{ "Black",		0,   0,   0,   255 },
	{ "Red",		255, 0,   0,   255 },
	{ "Green",		0,   255, 0,   255 },
	{ "Blue",		0,   0,   255, 255 },
	{ "Yellow",		255, 255, 0,   255 },
	{ "Cyan",		0,   255, 255, 255 },
	{ "Magenta",	255, 0,   255, 255 },
	{ "Orange",		255, 165, 0,   255 },
	{ "Purple",		128, 0,   128, 255 },
	{ "Pink",		255, 192, 203, 255 },
	{ "Gray",		128, 128, 128, 255 },
	{ "Brown",		165, 42,  42,  255 },
	{ "Gold",		255, 215, 0,   255 },
	{ "Silver",		192, 192, 192, 255 },
	{ "Transparent", 255, 255, 255, 128 },
	{ "Invisible",	255, 255, 255, 0   },
	{ NULL, 0, 0, 0, 0 }
};

//-----------------------------------------------------------------------------
// Color tool class - implements TOOL_COLOR mode
//-----------------------------------------------------------------------------
class CToolColor : public CWeaponTool
{
	DECLARE_CLASS( CToolColor, CWeaponTool );

public:
	CToolColor() : m_nSelectedPreset(0) {}

	virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void OnToolThink();

private:
	bool ApplyColor( CBaseEntity *pEntity, int r, int g, int b, int a );
	void CycleColorPreset();
	void CreateColorEffect( const Vector &vecPos, int r, int g, int b );
	void GetCurrentColor( int &r, int &g, int &b, int &a );
	void SetCurrentColor( int r, int g, int b, int a );
	void RandomizeColor();

	// Color tool state
	int		m_nSelectedPreset;	// Currently selected color preset index
};

//-----------------------------------------------------------------------------
// Tool implementation for Color mode
//-----------------------------------------------------------------------------
void CToolColor::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		// Primary attack - apply color
		if ( pEntity && pEntity != pOwner )
		{
			int r, g, b, a;
			GetCurrentColor( r, g, b, a );

			if ( ApplyColor( pEntity, r, g, b, a ) )
			{
				CreateColorEffect( tr.endpos, r, g, b );
				PlayToolSound( "garrysmod/balloon_pop_cute.wav" );

				ClientPrint( pOwner, HUD_PRINTTALK, "Applied color: R%d G%d B%d A%d", r, g, b, a );
			}
			else
			{
				ClientPrint( pOwner, HUD_PRINTTALK, "Cannot apply color to %s", pEntity->GetClassname() );
			}
		}
		else
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "No valid entity targeted" );
		}
	}
	else
	{
		// Secondary attack - cycle color preset
		CycleColorPreset();
	}
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Color mode
//-----------------------------------------------------------------------------
void CToolColor::OnToolTrace( trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Color tool can only be applied to entities" );
	}
	else
	{
		// Secondary attack - cycle color preset
		CycleColorPreset();
	}
}

//-----------------------------------------------------------------------------
// Tool think for Color mode
//-----------------------------------------------------------------------------
void CToolColor::OnToolThink()
{
	// Color tool doesn't need continuous thinking
}

//-----------------------------------------------------------------------------
// Apply color to entity
//-----------------------------------------------------------------------------
bool CToolColor::ApplyColor( CBaseEntity *pEntity, int r, int g, int b, int a )
{
	if ( !pEntity )
		return false;

	// Clamp values to valid range
	r = clamp( r, 0, 255 );
	g = clamp( g, 0, 255 );
	b = clamp( b, 0, 255 );
	a = clamp( a, 0, 255 );

	// Set entity render color
	pEntity->SetRenderColor( r, g, b );

	// Set render mode based on alpha
	if ( a < 255 )
	{
		pEntity->SetRenderMode( kRenderTransAlpha );
		pEntity->SetRenderColorA( a );
	}
	else
	{
		pEntity->SetRenderMode( kRenderNormal );
		pEntity->SetRenderColorA( 255 );
	}

	DevMsg( "Applied color R%d G%d B%d A%d to entity %s\n", r, g, b, a, pEntity->GetClassname() );
	return true;
}

//-----------------------------------------------------------------------------
// Cycle through color presets
//-----------------------------------------------------------------------------
void CToolColor::CycleColorPreset()
{
	// Find next valid preset
	do
	{
		m_nSelectedPreset++;
		if ( g_ColorPresets[m_nSelectedPreset].pszName == NULL )
		{
			m_nSelectedPreset = 0; // Wrap around
		}
	} while ( g_ColorPresets[m_nSelectedPreset].pszName == NULL );

	// Update ConVars
	ColorPreset_t *pPreset = &g_ColorPresets[m_nSelectedPreset];
	SetCurrentColor( pPreset->r, pPreset->g, pPreset->b, pPreset->a );

	// Inform player
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Selected color: %s (R%d G%d B%d A%d)",
			pPreset->pszName, pPreset->r, pPreset->g, pPreset->b, pPreset->a );
	}
}

//-----------------------------------------------------------------------------
// Create color application effect
//-----------------------------------------------------------------------------
void CToolColor::CreateColorEffect( const Vector &vecPos, int r, int g, int b )
{
	// Create colored sparkle effect
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = 75.0f;
	data.m_flScale = 2.0f;
	// Pack RGB into color field
	data.m_nColor = (r << 16) | (g << 8) | b;

	DispatchEffect( "GlowSprite", data );

	// Create colored particles
	data.m_vOrigin = vecPos + Vector(0, 0, 15);
	data.m_vNormal = Vector(0, 0, 1);
	data.m_flScale = 1.5f;

	DispatchEffect( "Sparks", data );
}

//-----------------------------------------------------------------------------
// Get current color from ConVars
//-----------------------------------------------------------------------------
void CToolColor::GetCurrentColor( int &r, int &g, int &b, int &a )
{
	r = bm_color_r.GetInt();
	g = bm_color_g.GetInt();
	b = bm_color_b.GetInt();
	a = bm_color_a.GetInt();
}

//-----------------------------------------------------------------------------
// Set current color ConVars
//-----------------------------------------------------------------------------
void CToolColor::SetCurrentColor( int r, int g, int b, int a )
{
	bm_color_r.SetValue( r );
	bm_color_g.SetValue( g );
	bm_color_b.SetValue( b );
	bm_color_a.SetValue( a );
}

//-----------------------------------------------------------------------------
// Randomize color values
//-----------------------------------------------------------------------------
void CToolColor::RandomizeColor()
{
	int r = random->RandomInt( 0, 255 );
	int g = random->RandomInt( 0, 255 );
	int b = random->RandomInt( 0, 255 );
	int a = random->RandomInt( 128, 255 ); // Keep somewhat visible

	SetCurrentColor( r, g, b, a );
}

//-----------------------------------------------------------------------------
// Console command for color context menu
//-----------------------------------------------------------------------------
CON_COMMAND( bm_context_color, "Opens color tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has color tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_COLOR )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Color tool must be equipped and selected" );
		return;
	}

	// Show available color presets
	ClientPrint( pPlayer, HUD_PRINTTALK, "Available color presets:" );

	for ( int i = 0; g_ColorPresets[i].pszName; i++ )
	{
		ColorPreset_t *pPreset = &g_ColorPresets[i];
		ClientPrint( pPlayer, HUD_PRINTTALK, "  %s (R%d G%d B%d A%d)",
			pPreset->pszName, pPreset->r, pPreset->g, pPreset->b, pPreset->a );
	}

	ClientPrint( pPlayer, HUD_PRINTTALK, "Current: R%d G%d B%d A%d",
		bm_color_r.GetInt(), bm_color_g.GetInt(), bm_color_b.GetInt(), bm_color_a.GetInt() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Use secondary fire to cycle presets" );
}

//-----------------------------------------------------------------------------
// Console command to set color directly
//-----------------------------------------------------------------------------
CON_COMMAND( bm_color_set, "Set color values directly" )
{
	if ( args.ArgC() < 4 )
	{
		Msg( "Usage: bm_color_set <r> <g> <b> [a]\n" );
		return;
	}

	int r = atoi( args.Arg(1) );
	int g = atoi( args.Arg(2) );
	int b = atoi( args.Arg(3) );
	int a = (args.ArgC() >= 5) ? atoi( args.Arg(4) ) : 255;

	// Clamp values
	r = clamp( r, 0, 255 );
	g = clamp( g, 0, 255 );
	b = clamp( b, 0, 255 );
	a = clamp( a, 0, 255 );

	bm_color_r.SetValue( r );
	bm_color_g.SetValue( g );
	bm_color_b.SetValue( b );
	bm_color_a.SetValue( a );

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Color set to: R%d G%d B%d A%d", r, g, b, a );
	}
}

//-----------------------------------------------------------------------------
// Console command to randomize color
//-----------------------------------------------------------------------------
CON_COMMAND( bm_color_random, "Set random color values" )
{
	int r = random->RandomInt( 0, 255 );
	int g = random->RandomInt( 0, 255 );
	int b = random->RandomInt( 0, 255 );
	int a = random->RandomInt( 128, 255 );

	bm_color_r.SetValue( r );
	bm_color_g.SetValue( g );
	bm_color_b.SetValue( b );
	bm_color_a.SetValue( a );

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Random color: R%d G%d B%d A%d", r, g, b, a );
	}
}