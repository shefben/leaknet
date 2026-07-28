//========= Copyright (c) 1996-2003, Valve Corporation, All rights reserved. ==
//
// Purpose: BMod MOTD usermessage handler.
//
//=============================================================================

#include "cbase.h"
#include "cdll_client_int.h"
#include "iclientmode.h"
#include "ienginevgui.h"
#include "parsemsg.h"

#include <game_controls/clientmotd.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>

// Keep this in sync with MAX_MOTD_LENGTH in multiplay_gamerules.cpp.
#define BMOD_MAX_MOTD_LENGTH 16384

static char g_szBModMOTD[BMOD_MAX_MOTD_LENGTH];
static bool g_bGotAllBModMOTD = true;
static CClientMOTD *g_pBModMOTD = NULL;

static CClientMOTD *GetBModMOTDPanel()
{
	if ( !g_pBModMOTD )
	{
		vgui::Panel *pParent = NULL;
		if ( g_pClientMode )
		{
			pParent = g_pClientMode->GetViewport();
		}

		g_pBModMOTD = new CClientMOTD( pParent );
		if ( !g_pBModMOTD )
		{
			return NULL;
		}

		if ( !pParent && enginevgui )
		{
			vgui::VPANEL rootPanel = enginevgui->GetPanel( PANEL_CLIENTDLL );
			if ( rootPanel )
			{
				g_pBModMOTD->SetParent( rootPanel );
			}
		}

		g_pBModMOTD->MakePopup();
		g_pBModMOTD->SetKeyBoardInputEnabled( true );
		g_pBModMOTD->SetMouseInputEnabled( true );
	}

	return g_pBModMOTD;
}

// The MOTD panel gets its size from Resource/UI/MOTD.res, which is authored in
// the 640x480 proportional base.  Proportional scaling only uses the screen
// *height* (screenTall / 480), so an .res width of 600 becomes 600 * screenTall
// / 480 - which is 94..100% of the screen width on any 4:3 mode (e.g. 1280x1024
// -> 1280 wide).  That is what makes the dialog look full-screen.  Rescale it
// once to a sane centred dialog and clamp it to 75% of the screen.
#define BMOD_MOTD_BASE_WIDE		512		// in the 640x480 proportional base
#define BMOD_MOTD_BASE_TALL		384
#define BMOD_MOTD_MIN_WIDE		320
#define BMOD_MOTD_MIN_TALL		240

static bool g_bBModMOTDResized = false;

static void LayoutBModMOTDPanel( CClientMOTD *pMOTD )
{
	int screenWide, screenTall;
	vgui::surface()->GetScreenSize( screenWide, screenTall );

	if ( !g_bBModMOTDResized )
	{
		int curWide, curTall;
		pMOTD->GetSize( curWide, curTall );

		int wantWide = vgui::scheme()->GetProportionalScaledValue( BMOD_MOTD_BASE_WIDE );
		int wantTall = vgui::scheme()->GetProportionalScaledValue( BMOD_MOTD_BASE_TALL );

		// never larger than ~75% of the screen in either axis
		const int maxWide = ( screenWide * 3 ) / 4;
		const int maxTall = ( screenTall * 3 ) / 4;
		if ( wantWide > maxWide )
			wantWide = maxWide;
		if ( wantTall > maxTall )
			wantTall = maxTall;
		if ( wantWide < BMOD_MOTD_MIN_WIDE )
			wantWide = BMOD_MOTD_MIN_WIDE;
		if ( wantTall < BMOD_MOTD_MIN_TALL )
			wantTall = BMOD_MOTD_MIN_TALL;

		if ( curWide > 0 && curTall > 0 && ( wantWide != curWide || wantTall != curTall ) )
		{
			// Scale the .res-placed children by the same factor so the HTML view,
			// the server name label and the OK button stay where they belong.
			const float sx = (float)wantWide / (float)curWide;
			const float sy = (float)wantTall / (float)curTall;

			for ( int i = 0; i < pMOTD->GetChildCount(); i++ )
			{
				vgui::Panel *pChild = pMOTD->GetChild( i );
				if ( !pChild )
					continue;

				int x, y, w, t;
				pChild->GetPos( x, y );
				pChild->GetSize( w, t );
				pChild->SetPos( (int)( x * sx ), (int)( y * sy ) );
				pChild->SetSize( (int)( w * sx ), (int)( t * sy ) );
			}

			pMOTD->SetSize( wantWide, wantTall );
			pMOTD->InvalidateLayout();

			DevMsg( "MOTD: resized panel from %dx%d to %dx%d (screen %dx%d)\n",
				curWide, curTall, wantWide, wantTall, screenWide, screenTall );
		}

		g_bBModMOTDResized = true;
	}

	int wide, tall;
	pMOTD->GetSize( wide, tall );

	pMOTD->SetPos( ( screenWide - wide ) / 2, ( screenTall - tall ) / 2 );
}

static void ShowBModMOTDPanel()
{
	CClientMOTD *pMOTD = GetBModMOTDPanel();
	if ( !pMOTD )
	{
		DevMsg( "MOTD: Unable to create BMod MOTD panel\n" );
		return;
	}

	char title[256] = "Message of the Day";
	if ( cvar )
	{
		const ConVar *pHostname = cvar->FindVar( "hostname" );
		if ( pHostname && pHostname->GetString()[0] )
		{
			Q_strncpy( title, pHostname->GetString(), sizeof( title ) );
		}
	}

	DevMsg( "MOTD: Showing BMod MOTD panel with %d bytes of content (%s)\n",
		(int)strlen( g_szBModMOTD ),
		CClientMOTD::IsURL( g_szBModMOTD ) ? "URL" :
			( CClientMOTD::IsHTML( g_szBModMOTD ) ? "inline HTML" : "plain text" ) );
	pMOTD->Activate( title, g_szBModMOTD );
	LayoutBModMOTDPanel( pMOTD );
	pMOTD->MoveToFront();
	pMOTD->RequestFocus();
}

void __MsgFunc_MOTD_Standalone( const char *pszName, int iSize, void *pbuf )
{
	if ( !pbuf || iSize <= 0 )
	{
		return;
	}

	if ( g_bGotAllBModMOTD )
	{
		g_szBModMOTD[0] = '\0';
	}

	BEGIN_READ( pbuf, iSize );

	g_bGotAllBModMOTD = ( READ_BYTE() != 0 );

	const char *pszChunk = READ_STRING();
	if ( pszChunk && pszChunk[0] )
	{
		Q_strncat( g_szBModMOTD, pszChunk, sizeof( g_szBModMOTD ) );
	}

	if ( g_bGotAllBModMOTD && g_szBModMOTD[0] )
	{
		ShowBModMOTDPanel();
	}
}
