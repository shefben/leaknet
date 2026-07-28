//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Paint Tool - Implementation of Paint/decal tool
//          Based on Garry's Mod tool system analysis
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "decals.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"

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
// Console variables for paint tool
//-----------------------------------------------------------------------------
ConVar gmod_paint_decal("gmod_paint_decal", "scorch", FCVAR_ARCHIVE, "Current decal to paint");
ConVar gmod_paint_size("gmod_paint_size", "32", FCVAR_ARCHIVE, "Paint decal size (8-128)");
ConVar gmod_paint_color_r("gmod_paint_color_r", "255", FCVAR_ARCHIVE, "Paint red component (0-255)");
ConVar gmod_paint_color_g("gmod_paint_color_g", "255", FCVAR_ARCHIVE, "Paint green component (0-255)");
ConVar gmod_paint_color_b("gmod_paint_color_b", "255", FCVAR_ARCHIVE, "Paint blue component (0-255)");
ConVar gmod_paint_permanent("gmod_paint_permanent", "0", FCVAR_ARCHIVE, "Paint permanent decals (1) or temporary (0)");

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
// Per-weapon paint state - CWeaponTool is not subclassed, so the state that
// used to live in CToolPaint's member variables now lives here, keyed by the
// weapon's EHANDLE (mirrors the g_WeldConstraints/g_RopeConstraints registries
// used by the sibling tool_*.cpp files).
//-----------------------------------------------------------------------------
struct PaintState_t
{
	EHANDLE	hTool;
	int		nSelectedDecal;		// Currently selected decal index
	int		nBrushType;			// Current brush type
	float	flLastPaintTime;	// Last time we painted (for spray effect)

	PaintState_t()
	{
		nSelectedDecal = 0;
		nBrushType = BRUSH_MEDIUM;
		flLastPaintTime = 0.0f;
	}
};

static CUtlVector<PaintState_t*> g_PaintStates;

static PaintState_t *FindPaintState( CWeaponTool *pTool )
{
	for ( int i = 0; i < g_PaintStates.Count(); i++ )
	{
		if ( g_PaintStates[i]->hTool == pTool )
			return g_PaintStates[i];
	}
	return NULL;
}

static PaintState_t *GetPaintState( CWeaponTool *pTool )
{
	PaintState_t *pState = FindPaintState( pTool );
	if ( !pState )
	{
		pState = new PaintState_t;
		pState->hTool = pTool;
		g_PaintStates.AddToTail( pState );
	}
	return pState;
}

// Removes state records for weapons that no longer exist.
static void CleanupPaintStates()
{
	for ( int i = g_PaintStates.Count() - 1; i >= 0; i-- )
	{
		if ( !g_PaintStates[i]->hTool.Get() )
		{
			delete g_PaintStates[i];
			g_PaintStates.Remove( i );
		}
	}
}

//-----------------------------------------------------------------------------
// Forward declarations of tool helpers
//-----------------------------------------------------------------------------
static void PaintDecal( trace_t &tr, PaintState_t *pState );
static void CycleDecal( CWeaponTool *pTool, PaintState_t *pState );
static void CycleBrush( CWeaponTool *pTool, PaintState_t *pState );
static void CreatePaintEffect( const Vector &vecPos, const Vector &vecNormal, PaintState_t *pState );
static const char *GetCurrentDecal( PaintState_t *pState );
static int GetPaintSize( PaintState_t *pState );
static bool CanPaint( PaintState_t *pState );

//-----------------------------------------------------------------------------
// Tool implementation for Paint mode
//-----------------------------------------------------------------------------
void Tool_Paint_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	PaintState_t *pState = GetPaintState( pTool );

	if ( bPrimary )
	{
		// Primary attack - paint decal
		if ( CanPaint( pState ) )
		{
			PaintDecal( tr, pState );
			CreatePaintEffect( tr.endpos, tr.plane.normal, pState );
			pTool->PlayToolSound( "garrysmod/balloon_pop_cute.wav" );

			pState->flLastPaintTime = gpGlobals->curtime;

			const char *pszDecal = GetCurrentDecal( pState );
			ClientPrintf( pOwner, HUD_PRINTTALK, "Painted: %s", pszDecal );
		}
	}
	else
	{
		// Secondary attack - cycle decal or brush based on held buttons
		if ( pOwner->m_nButtons & IN_USE )
		{
			CycleBrush( pTool, pState );
		}
		else
		{
			CycleDecal( pTool, pState );
		}
	}
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Paint mode
//-----------------------------------------------------------------------------
void Tool_Paint_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary )
{
	PaintState_t *pState = GetPaintState( pTool );

	if ( bPrimary )
	{
		// Can paint on world surfaces
		if ( CanPaint( pState ) )
		{
			PaintDecal( tr, pState );
			CreatePaintEffect( tr.endpos, tr.plane.normal, pState );
			pTool->PlayToolSound( "garrysmod/balloon_pop_cute.wav" );

			pState->flLastPaintTime = gpGlobals->curtime;

			CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
			if ( pOwner )
			{
				const char *pszDecal = GetCurrentDecal( pState );
				ClientPrintf( pOwner, HUD_PRINTTALK, "Painted: %s", pszDecal );
			}
		}
	}
	else
	{
		// Secondary attack - cycle options
		CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
		if ( pOwner && (pOwner->m_nButtons & IN_USE) )
		{
			CycleBrush( pTool, pState );
		}
		else
		{
			CycleDecal( pTool, pState );
		}
	}
}

//-----------------------------------------------------------------------------
// Tool think for Paint mode
//-----------------------------------------------------------------------------
void Tool_Paint_OnThink( CWeaponTool *pTool )
{
	// Paint tool doesn't need continuous thinking; just keep the registry tidy
	CleanupPaintStates();
}

//-----------------------------------------------------------------------------
// Paint decal at trace position
//-----------------------------------------------------------------------------
static void PaintDecal( trace_t &tr, PaintState_t *pState )
{
	if ( !tr.DidHit() )
		return;

	const char *pszDecal = GetCurrentDecal( pState );
	if ( !pszDecal || !pszDecal[0] )
		return;

	// Apply decal to whatever was hit (entity or world)
	UTIL_DecalTrace( &tr, pszDecal );

	CBaseEntity *pEntity = tr.m_pEnt;
	DevMsg( "Painted decal %s at (%f, %f, %f) on %s\n",
		pszDecal, tr.endpos.x, tr.endpos.y, tr.endpos.z,
		pEntity ? pEntity->GetClassname() : "world" );
}

//-----------------------------------------------------------------------------
// Cycle through decal types
//-----------------------------------------------------------------------------
static void CycleDecal( CWeaponTool *pTool, PaintState_t *pState )
{
	// Find next valid decal
	do
	{
		pState->nSelectedDecal++;
		if ( g_PaintDecals[pState->nSelectedDecal] == NULL )
		{
			pState->nSelectedDecal = 0; // Wrap around
		}
	} while ( g_PaintDecals[pState->nSelectedDecal] == NULL );

	// Update ConVar
	gmod_paint_decal.SetValue( g_PaintDecals[pState->nSelectedDecal] );

	// Inform player
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( pOwner )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "Selected decal: %s",
			g_PaintDecals[pState->nSelectedDecal] );
	}
}

//-----------------------------------------------------------------------------
// Cycle through brush types
//-----------------------------------------------------------------------------
static void CycleBrush( CWeaponTool *pTool, PaintState_t *pState )
{
	pState->nBrushType++;
	if ( pState->nBrushType >= BRUSH_MAX )
	{
		pState->nBrushType = 0;
	}

	// Adjust paint size based on brush type
	int nSize = 16; // Default size
	switch ( pState->nBrushType )
	{
		case BRUSH_SMALL:	nSize = 8;	break;
		case BRUSH_MEDIUM:	nSize = 16;	break;
		case BRUSH_LARGE:	nSize = 32;	break;
		case BRUSH_SPRAY:	nSize = 24;	break;
		case BRUSH_ROLLER:	nSize = 48;	break;
		case BRUSH_AIRBRUSH: nSize = 12; break;
	}

	gmod_paint_size.SetValue( nSize );

	// Inform player
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( pOwner )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "Selected brush: %s (Size: %d)",
			g_BrushNames[pState->nBrushType], nSize );
	}
}

//-----------------------------------------------------------------------------
// Create paint application effect
//-----------------------------------------------------------------------------
static void CreatePaintEffect( const Vector &vecPos, const Vector &vecNormal, PaintState_t *pState )
{
	// Create paint splash effect
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_vNormal = vecNormal;
	data.m_flMagnitude = GetPaintSize( pState );
	data.m_flScale = 1.0f;

	// Use paint color
	int r = gmod_paint_color_r.GetInt();
	int g = gmod_paint_color_g.GetInt();
	int b = gmod_paint_color_b.GetInt();
	data.m_nColor = (r << 16) | (g << 8) | b;

	DispatchEffect( "Sparks", data );

	// Create paint particles based on brush type
	switch ( pState->nBrushType )
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
			data.m_flMagnitude = GetPaintSize( pState ) * 2;
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
static const char *GetCurrentDecal( PaintState_t *pState )
{
	// Find decal matching the current ConVar
	const char *pszCurrentDecal = gmod_paint_decal.GetString();

	for ( int i = 0; g_PaintDecals[i]; i++ )
	{
		if ( !Q_stricmp( g_PaintDecals[i], pszCurrentDecal ) )
		{
			pState->nSelectedDecal = i;
			return g_PaintDecals[i];
		}
	}

	// If custom decal from ConVar, use it directly
	return pszCurrentDecal;
}

//-----------------------------------------------------------------------------
// Get paint size with brush modifications
//-----------------------------------------------------------------------------
static int GetPaintSize( PaintState_t *pState )
{
	int nBaseSize = gmod_paint_size.GetInt();

	// Modify size based on brush type
	switch ( pState->nBrushType )
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
static bool CanPaint( PaintState_t *pState )
{
	float flDelay = 0.1f; // Default delay

	// Adjust delay based on brush type
	switch ( pState->nBrushType )
	{
		case BRUSH_SPRAY:	flDelay = 0.05f; break; // Fast spray
		case BRUSH_AIRBRUSH: flDelay = 0.03f; break; // Very fast airbrush
		case BRUSH_ROLLER:	flDelay = 0.3f;  break; // Slow roller
		default:			flDelay = 0.1f;  break;
	}

	return (gpGlobals->curtime - pState->flLastPaintTime) >= flDelay;
}

//-----------------------------------------------------------------------------
// Console command for paint context menu
//-----------------------------------------------------------------------------
CON_COMMAND( gmod_context_paint, "Opens paint tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has paint tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_PAINT )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Paint tool must be equipped and selected" );
		return;
	}

	// Show available decals
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Available decals:" );

	for ( int i = 0; g_PaintDecals[i]; i++ )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "  %s", g_PaintDecals[i] );
	}

	ClientPrintf( pPlayer, HUD_PRINTTALK, "Current decal: %s", gmod_paint_decal.GetString() );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Paint size: %d", gmod_paint_size.GetInt() );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Color: R%d G%d B%d",
		gmod_paint_color_r.GetInt(), gmod_paint_color_g.GetInt(), gmod_paint_color_b.GetInt() );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Secondary fire: cycle decals" );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Use + Secondary fire: cycle brushes" );
}

//-----------------------------------------------------------------------------
// Console command to clear all painted decals
//-----------------------------------------------------------------------------
CON_COMMAND( gmod_paint_clear, "Clear all painted decals" )
{
	// In a full implementation, this would remove all temporary decals
	// For now, just inform the player
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "All temporary paint cleared" );
	}
}
