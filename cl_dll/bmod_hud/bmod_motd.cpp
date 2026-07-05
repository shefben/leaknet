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

static void CenterBModMOTDPanel( CClientMOTD *pMOTD )
{
	int wide, tall;
	pMOTD->GetSize( wide, tall );

	int screenWide, screenTall;
	vgui::surface()->GetScreenSize( screenWide, screenTall );

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

	DevMsg( "MOTD: Showing BMod MOTD panel with %d bytes of content\n", (int)strlen( g_szBModMOTD ) );
	pMOTD->Activate( title, g_szBModMOTD );
	CenterBModMOTDPanel( pMOTD );
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
