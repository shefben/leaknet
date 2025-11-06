//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Paint Tool - Implementation of Paint/decal tool
//          Based on Garry's Mod tool system analysis
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "decals.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables for paint tool
//-----------------------------------------------------------------------------
ConVar bm_paint_decal("bm_paint_decal", "scorch", FCVAR_ARCHIVE, "Current decal to paint");
ConVar bm_paint_size("bm_paint_size", "32", FCVAR_ARCHIVE, "Paint decal size (8-128)");
ConVar bm_paint_color_r("bm_paint_color_r", "255", FCVAR_ARCHIVE, "Paint red component (0-255)");
ConVar bm_paint_color_g("bm_paint_color_g", "255", FCVAR_ARCHIVE, "Paint green component (0-255)");
ConVar bm_paint_color_b("bm_paint_color_b", "255", FCVAR_ARCHIVE, "Paint blue component (0-255)");
ConVar bm_paint_permanent("bm_paint_permanent", "0", FCVAR_ARCHIVE, "Paint permanent decals (1) or temporary (0)");

//-----------------------------------------------------------------------------
// Available decal types for painting
//-----------------------------------------------------------------------------
static const char *g_PaintDecals[] =
{
	"scorch",		// Burn mark
	"shot",			// Bullet hole
	"splash",		// Liquid splash
	"crack",		// Crack in surface
	"paint",		// Paint splotch
	"blood",		// Blood splatter
	"oil",			// Oil stain
	"dirt",			// Dirt mark
	"rust",			// Rust stain
	"graffiti01",	// Graffiti tag 1
	"graffiti02",	// Graffiti tag 2
	"lambda",		// Lambda symbol
	"biohazard",	// Biohazard symbol
	"radioactive",	// Radioactive symbol
	"smile",		// Smiley face
	"skull",		// Skull mark
	"crosshair",	// Crosshair
	"target",		// Target circle
	"arrow",		// Arrow mark
	"star",			// Star shape
	NULL
};

//-----------------------------------------------------------------------------
// Paint brush types
//-----------------------------------------------------------------------------
enum PaintBrush_t
{
	BRUSH_SMALL = 0,	// Small brush
	BRUSH_MEDIUM,		// Medium brush
	BRUSH_LARGE,		// Large brush
	BRUSH_SPRAY,		// Spray can effect
	BRUSH_ROLLER,		// Paint roller
	BRUSH_AIRBRUSH,		// Airbrush
	BRUSH_MAX
};

static const char *g_BrushNames[] =
{
	"Small Brush",
	"Medium Brush",
	"Large Brush",
	"Spray Can",
	"Paint Roller",
	"Airbrush"
};

//-----------------------------------------------------------------------------
// Paint tool class - implements TOOL_PAINT mode
//-----------------------------------------------------------------------------
class CToolPaint : public CWeaponTool
{
	DECLARE_CLASS( CToolPaint, CWeaponTool );

public:
	CToolPaint() : m_nSelectedDecal(0), m_nBrushType(BRUSH_MEDIUM), m_flLastPaintTime(0.0f) {}

	virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void OnToolThink();

private:
	void PaintDecal( trace_t &tr );
	void CycleDecal();
	void CycleBrush();
	void CreatePaintEffect( const Vector &vecPos, const Vector &vecNormal );
	const char *GetCurrentDecal();
	int GetPaintSize();
	bool CanPaint();

	// Paint tool state
	int		m_nSelectedDecal;	// Currently selected decal index
	int		m_nBrushType;		// Current brush type
	float	m_flLastPaintTime;	// Last time we painted (for spray effect)
};

//-----------------------------------------------------------------------------
// Tool implementation for Paint mode
//-----------------------------------------------------------------------------
void CToolPaint::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		// Primary attack - paint decal
		if ( CanPaint() )
		{
			PaintDecal( tr );
			CreatePaintEffect( tr.endpos, tr.plane.normal );
			PlayToolSound( "garrysmod/balloon_pop_cute.wav" );

			m_flLastPaintTime = gpGlobals->curtime;

			const char *pszDecal = GetCurrentDecal();
			ClientPrint( pOwner, HUD_PRINTTALK, "Painted: %s", pszDecal );
		}
	}
	else
	{
		// Secondary attack - cycle decal or brush based on held buttons
		if ( pOwner->m_nButtons & IN_USE )
		{
			CycleBrush();
		}
		else
		{
			CycleDecal();
		}
	}
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Paint mode
//-----------------------------------------------------------------------------
void CToolPaint::OnToolTrace( trace_t &tr, bool bPrimary )
{
	if ( bPrimary )
	{
		// Can paint on world surfaces
		if ( CanPaint() )
		{
			PaintDecal( tr );
			CreatePaintEffect( tr.endpos, tr.plane.normal );
			PlayToolSound( "garrysmod/balloon_pop_cute.wav" );

			m_flLastPaintTime = gpGlobals->curtime;

			CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
			if ( pOwner )
			{
				const char *pszDecal = GetCurrentDecal();
				ClientPrint( pOwner, HUD_PRINTTALK, "Painted: %s", pszDecal );
			}
		}
	}
	else
	{
		// Secondary attack - cycle options
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if ( pOwner && (pOwner->m_nButtons & IN_USE) )
		{
			CycleBrush();
		}
		else
		{
			CycleDecal();
		}
	}
}

//-----------------------------------------------------------------------------
// Tool think for Paint mode
//-----------------------------------------------------------------------------
void CToolPaint::OnToolThink()
{
	// Paint tool doesn't need continuous thinking
	// Could add paint dripping effects here
}

//-----------------------------------------------------------------------------
// Paint decal at trace position
//-----------------------------------------------------------------------------
void CToolPaint::PaintDecal( trace_t &tr )
{
	if ( !tr.DidHit() )
		return;

	const char *pszDecal = GetCurrentDecal();
	if ( !pszDecal )
		return;

	// Get decal index
	int nDecalIndex = DECAL_INDEX( pszDecal );
	if ( nDecalIndex < 0 )
	{
		DevMsg( "Unknown decal: %s\n", pszDecal );
		return;
	}

	// Create the decal
	CBaseEntity *pEntity = tr.m_pEnt;
	if ( pEntity )
	{
		// Apply decal to entity
		int nSize = GetPaintSize();

		// Use entity's model for decal placement
		if ( pEntity->GetModel() )
		{
			UTIL_DecalTrace( &tr, pszDecal );
		}

		DevMsg( "Painted decal %s at (%f, %f, %f) on %s\n",
			pszDecal, tr.endpos.x, tr.endpos.y, tr.endpos.z,
			pEntity ? pEntity->GetClassname() : "world" );
	}
}

//-----------------------------------------------------------------------------
// Cycle through decal types
//-----------------------------------------------------------------------------
void CToolPaint::CycleDecal()
{
	// Find next valid decal
	do
	{
		m_nSelectedDecal++;
		if ( g_PaintDecals[m_nSelectedDecal] == NULL )
		{
			m_nSelectedDecal = 0; // Wrap around
		}
	} while ( g_PaintDecals[m_nSelectedDecal] == NULL );

	// Update ConVar
	bm_paint_decal.SetValue( g_PaintDecals[m_nSelectedDecal] );

	// Inform player
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Selected decal: %s",
			g_PaintDecals[m_nSelectedDecal] );
	}
}

//-----------------------------------------------------------------------------
// Cycle through brush types
//-----------------------------------------------------------------------------
void CToolPaint::CycleBrush()
{
	m_nBrushType++;
	if ( m_nBrushType >= BRUSH_MAX )
	{
		m_nBrushType = 0;
	}

	// Adjust paint size based on brush type
	int nSize = 16; // Default size
	switch ( m_nBrushType )
	{
		case BRUSH_SMALL:	nSize = 8;	break;
		case BRUSH_MEDIUM:	nSize = 16;	break;
		case BRUSH_LARGE:	nSize = 32;	break;
		case BRUSH_SPRAY:	nSize = 24;	break;
		case BRUSH_ROLLER:	nSize = 48;	break;
		case BRUSH_AIRBRUSH: nSize = 12; break;
	}

	bm_paint_size.SetValue( nSize );

	// Inform player
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Selected brush: %s (Size: %d)",
			g_BrushNames[m_nBrushType], nSize );
	}
}

//-----------------------------------------------------------------------------
// Create paint application effect
//-----------------------------------------------------------------------------
void CToolPaint::CreatePaintEffect( const Vector &vecPos, const Vector &vecNormal )
{
	// Create paint splash effect
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_vNormal = vecNormal;
	data.m_flMagnitude = GetPaintSize();
	data.m_flScale = 1.0f;

	// Use paint color
	int r = bm_paint_color_r.GetInt();
	int g = bm_paint_color_g.GetInt();
	int b = bm_paint_color_b.GetInt();
	data.m_nColor = (r << 16) | (g << 8) | b;

	DispatchEffect( "Sparks", data );

	// Create paint particles based on brush type
	switch ( m_nBrushType )
	{
		case BRUSH_SPRAY:
			// Multiple small particles for spray effect
			for ( int i = 0; i < 5; i++ )
			{
				data.m_vOrigin = vecPos + Vector(
					random->RandomFloat(-8, 8),
					random->RandomFloat(-8, 8),
					random->RandomFloat(-8, 8) );
				data.m_flScale = 0.5f;
				DispatchEffect( "Sparks", data );
			}
			break;

		case BRUSH_AIRBRUSH:
			// Fine mist effect
			data.m_flScale = 0.3f;
			data.m_flMagnitude = GetPaintSize() * 2;
			DispatchEffect( "GlowSprite", data );
			break;

		default:
			// Standard paint effect
			data.m_flScale = 1.5f;
			DispatchEffect( "GlowSprite", data );
			break;
	}
}

//-----------------------------------------------------------------------------
// Get current decal name
//-----------------------------------------------------------------------------
const char *CToolPaint::GetCurrentDecal()
{
	// Find decal matching the current ConVar
	const char *pszCurrentDecal = bm_paint_decal.GetString();

	for ( int i = 0; g_PaintDecals[i]; i++ )
	{
		if ( !Q_stricmp( g_PaintDecals[i], pszCurrentDecal ) )
		{
			m_nSelectedDecal = i;
			return g_PaintDecals[i];
		}
	}

	// If custom decal from ConVar, use it directly
	return pszCurrentDecal;
}

//-----------------------------------------------------------------------------
// Get paint size with brush modifications
//-----------------------------------------------------------------------------
int CToolPaint::GetPaintSize()
{
	int nBaseSize = bm_paint_size.GetInt();

	// Modify size based on brush type
	switch ( m_nBrushType )
	{
		case BRUSH_SMALL:	return nBaseSize / 2;
		case BRUSH_LARGE:	return nBaseSize * 2;
		case BRUSH_ROLLER:	return nBaseSize * 3;
		case BRUSH_AIRBRUSH: return nBaseSize / 3;
		default:			return nBaseSize;
	}
}

//-----------------------------------------------------------------------------
// Check if we can paint (rate limiting)
//-----------------------------------------------------------------------------
bool CToolPaint::CanPaint()
{
	float flDelay = 0.1f; // Default delay

	// Adjust delay based on brush type
	switch ( m_nBrushType )
	{
		case BRUSH_SPRAY:	flDelay = 0.05f; break; // Fast spray
		case BRUSH_AIRBRUSH: flDelay = 0.03f; break; // Very fast airbrush
		case BRUSH_ROLLER:	flDelay = 0.3f;  break; // Slow roller
		default:			flDelay = 0.1f;  break;
	}

	return (gpGlobals->curtime - m_flLastPaintTime) >= flDelay;
}

//-----------------------------------------------------------------------------
// Console command for paint context menu
//-----------------------------------------------------------------------------
CON_COMMAND( bm_context_paint, "Opens paint tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has paint tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_PAINT )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Paint tool must be equipped and selected" );
		return;
	}

	// Show available decals
	ClientPrint( pPlayer, HUD_PRINTTALK, "Available decals:" );

	for ( int i = 0; g_PaintDecals[i]; i++ )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "  %s", g_PaintDecals[i] );
	}

	ClientPrint( pPlayer, HUD_PRINTTALK, "Current decal: %s", bm_paint_decal.GetString() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Paint size: %d", bm_paint_size.GetInt() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Color: R%d G%d B%d",
		bm_paint_color_r.GetInt(), bm_paint_color_g.GetInt(), bm_paint_color_b.GetInt() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Secondary fire: cycle decals" );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Use + Secondary fire: cycle brushes" );
}

//-----------------------------------------------------------------------------
// Console command to clear all painted decals
//-----------------------------------------------------------------------------
CON_COMMAND( bm_paint_clear, "Clear all painted decals" )
{
	// In a full implementation, this would remove all temporary decals
	// For now, just inform the player
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "All temporary paint cleared" );
	}
}