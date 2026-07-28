//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Color Tool - Implementation of Color/tint changing tool
//          Based on Garry's Mod tool system analysis
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"

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
// Console variables for color tool
//-----------------------------------------------------------------------------
ConVar gmod_color_r("gmod_color_r", "255", FCVAR_ARCHIVE, "Red component (0-255)");
ConVar gmod_color_g("gmod_color_g", "255", FCVAR_ARCHIVE, "Green component (0-255)");
ConVar gmod_color_b("gmod_color_b", "255", FCVAR_ARCHIVE, "Blue component (0-255)");
ConVar gmod_color_a("gmod_color_a", "255", FCVAR_ARCHIVE, "Alpha component (0-255)");

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
// Per-player preset cycling state. The applied color itself lives in the
// (server-wide) gmod_color_* ConVars, but which preset index secondary-fire
// should advance to next is naturally per-player.
//-----------------------------------------------------------------------------
struct ColorToolState_t
{
	CHandle<CBasePlayer>	hOwner;
	int						nSelectedPreset;

	ColorToolState_t()
	{
		nSelectedPreset = 0;
	}
};

static CUtlVector<ColorToolState_t*> g_ColorToolStates;

static ColorToolState_t *FindColorState( CBasePlayer *pOwner )
{
	for ( int i = 0; i < g_ColorToolStates.Count(); i++ )
	{
		if ( g_ColorToolStates[i]->hOwner.Get() == pOwner )
			return g_ColorToolStates[i];
	}
	return NULL;
}

static ColorToolState_t *GetOrCreateColorState( CBasePlayer *pOwner )
{
	ColorToolState_t *pState = FindColorState( pOwner );
	if ( pState )
		return pState;

	pState = new ColorToolState_t;
	pState->hOwner = pOwner;
	g_ColorToolStates.AddToTail( pState );
	return pState;
}

//-----------------------------------------------------------------------------
// Apply color to entity
//-----------------------------------------------------------------------------
static bool ApplyColor( CBaseEntity *pEntity, int r, int g, int b, int a )
{
	if ( !pEntity )
		return false;

	r = clamp( r, 0, 255 );
	g = clamp( g, 0, 255 );
	b = clamp( b, 0, 255 );
	a = clamp( a, 0, 255 );

	pEntity->SetRenderColor( r, g, b, a );
	pEntity->SetRenderMode( a < 255 ? kRenderTransAlpha : kRenderNormal );

	return true;
}

//-----------------------------------------------------------------------------
// Get/set current color from ConVars
//-----------------------------------------------------------------------------
static void GetCurrentColor( int &r, int &g, int &b, int &a )
{
	r = gmod_color_r.GetInt();
	g = gmod_color_g.GetInt();
	b = gmod_color_b.GetInt();
	a = gmod_color_a.GetInt();
}

static void SetCurrentColor( int r, int g, int b, int a )
{
	gmod_color_r.SetValue( r );
	gmod_color_g.SetValue( g );
	gmod_color_b.SetValue( b );
	gmod_color_a.SetValue( a );
}

//-----------------------------------------------------------------------------
// Cycle through color presets for this player
//-----------------------------------------------------------------------------
static void CycleColorPreset( CBasePlayer *pOwner )
{
	ColorToolState_t *pState = GetOrCreateColorState( pOwner );

	do
	{
		pState->nSelectedPreset++;
		if ( g_ColorPresets[pState->nSelectedPreset].pszName == NULL )
		{
			pState->nSelectedPreset = 0; // Wrap around
		}
	} while ( g_ColorPresets[pState->nSelectedPreset].pszName == NULL );

	ColorPreset_t *pPreset = &g_ColorPresets[pState->nSelectedPreset];
	SetCurrentColor( pPreset->r, pPreset->g, pPreset->b, pPreset->a );

	ClientPrintf( pOwner, HUD_PRINTTALK, "Selected color: %s (R%d G%d B%d A%d)",
		pPreset->pszName, pPreset->r, pPreset->g, pPreset->b, pPreset->a );
}

//-----------------------------------------------------------------------------
// Tool dispatch - Colour(16)
//-----------------------------------------------------------------------------
void Tool_Color_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		if ( pEntity && pEntity != pOwner )
		{
			int r, g, b, a;
			GetCurrentColor( r, g, b, a );

			if ( ApplyColor( pEntity, r, g, b, a ) )
			{
				pTool->PlayToolSound( "garrysmod/balloon_pop_cute.wav" );
				ClientPrintf( pOwner, HUD_PRINTTALK, "Applied color: R%d G%d B%d A%d", r, g, b, a );
			}
			else
			{
				ClientPrintf( pOwner, HUD_PRINTTALK, "Cannot apply color to %s", pEntity->GetClassname() );
			}
		}
		else
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "No valid entity targeted" );
		}
	}
	else
	{
		CycleColorPreset( pOwner );
	}
}

void Tool_Color_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "Color tool can only be applied to entities" );
	}
	else
	{
		CycleColorPreset( pOwner );
	}
}

void Tool_Color_OnThink( CWeaponTool *pTool )
{
	// Prune preset-cycling state belonging to players who have since disconnected.
	for ( int i = g_ColorToolStates.Count() - 1; i >= 0; i-- )
	{
		if ( !g_ColorToolStates[i]->hOwner.Get() )
		{
			delete g_ColorToolStates[i];
			g_ColorToolStates.Remove( i );
		}
	}
}

//-----------------------------------------------------------------------------
// Console command for color context menu
//-----------------------------------------------------------------------------
CON_COMMAND( gmod_context_color, "Opens color tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_COLOUR )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Color tool must be equipped and selected" );
		return;
	}

	// Show available color presets
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Available color presets:" );

	for ( int i = 0; g_ColorPresets[i].pszName; i++ )
	{
		ColorPreset_t *pPreset = &g_ColorPresets[i];
		ClientPrintf( pPlayer, HUD_PRINTTALK, "  %s (R%d G%d B%d A%d)",
			pPreset->pszName, pPreset->r, pPreset->g, pPreset->b, pPreset->a );
	}

	ClientPrintf( pPlayer, HUD_PRINTTALK, "Current: R%d G%d B%d A%d",
		gmod_color_r.GetInt(), gmod_color_g.GetInt(), gmod_color_b.GetInt(), gmod_color_a.GetInt() );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Use secondary fire to cycle presets" );
}

//-----------------------------------------------------------------------------
// Console command to set color directly
//-----------------------------------------------------------------------------
CON_COMMAND( gmod_color_set, "Set color values directly" )
{
	if ( engine->Cmd_Argc() < 4 )
	{
		Msg( "Usage: gmod_color_set <r> <g> <b> [a]\n" );
		return;
	}

	int r = atoi( engine->Cmd_Argv(1) );
	int g = atoi( engine->Cmd_Argv(2) );
	int b = atoi( engine->Cmd_Argv(3) );
	int a = (engine->Cmd_Argc() >= 5) ? atoi( engine->Cmd_Argv(4) ) : 255;

	// Clamp values
	r = clamp( r, 0, 255 );
	g = clamp( g, 0, 255 );
	b = clamp( b, 0, 255 );
	a = clamp( a, 0, 255 );

	SetCurrentColor( r, g, b, a );

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Color set to: R%d G%d B%d A%d", r, g, b, a );
	}
}

//-----------------------------------------------------------------------------
// Console command to randomize color
//-----------------------------------------------------------------------------
CON_COMMAND( gmod_color_random, "Set random color values" )
{
	int r = random->RandomInt( 0, 255 );
	int g = random->RandomInt( 0, 255 );
	int b = random->RandomInt( 0, 255 );
	int a = random->RandomInt( 128, 255 );

	SetCurrentColor( r, g, b, a );

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Random color: R%d G%d B%d A%d", r, g, b, a );
	}
}
