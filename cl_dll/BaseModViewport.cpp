//====== Copyright � 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: Standalone user message handlers for BMod
//         MOTD handler works independently without requiring TeamFortressViewport
//
//=============================================================================

#include "cbase.h"
#include "game_controls/iviewportmsgs.h"
#include "c_user_message_register.h"
#include "parsemsg.h"
#include <game_controls/clientmotd.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Frame.h>
#include "iclientmode.h"
#include "cdll_client_int.h"
#include "ienginevgui.h"

// Standalone MOTD storage and panel
// Buffer size matches MAX_MOTD_LENGTH in multiplay_gamerules.cpp (16KB)
static char g_szMOTD[16384];
static bool g_bGotAllMOTD = true;  // Start as true so first message clears buffer
static CClientMOTD *g_pStandaloneMOTD = NULL;

//-----------------------------------------------------------------------------
// Purpose: Create or get the standalone MOTD panel
//-----------------------------------------------------------------------------
static CClientMOTD *GetStandaloneMOTDPanel()
{
	if (!g_pStandaloneMOTD)
	{
		// Get viewport from client mode if available
		vgui::Panel *pParent = NULL;
		if (g_pClientMode)
		{
			pParent = g_pClientMode->GetViewport();
		}

		DevMsg("MOTD: Creating standalone MOTD panel (parent=%p, clientmode=%p)\n", pParent, g_pClientMode);

		g_pStandaloneMOTD = new CClientMOTD(pParent);

		if (g_pStandaloneMOTD)
		{
			// Make it a popup so it displays on top of everything
			g_pStandaloneMOTD->MakePopup();
			g_pStandaloneMOTD->SetKeyBoardInputEnabled(true);
			g_pStandaloneMOTD->SetMouseInputEnabled(true);

			// If no parent was set, explicitly parent to the engine VGUI client panel
			if (!pParent)
			{
				vgui::VPANEL rootPanel = enginevgui->GetPanel(PANEL_CLIENTDLL);
				if (rootPanel)
				{
					g_pStandaloneMOTD->SetParent(rootPanel);
					DevMsg("MOTD: Parented to engine CLIENTDLL panel\n");
				}
			}

			// Center the dialog on screen
			int wide, tall;
			g_pStandaloneMOTD->GetSize(wide, tall);
			int screenW, screenH;
			vgui::surface()->GetScreenSize(screenW, screenH);
			g_pStandaloneMOTD->SetPos((screenW - wide) / 2, (screenH - tall) / 2);
		}
	}
	return g_pStandaloneMOTD;
}

//-----------------------------------------------------------------------------
// Purpose: Handle MOTD user message (2003 protocol)
//         Works standalone without requiring TeamFortressViewport
//-----------------------------------------------------------------------------
void __MsgFunc_MOTD_Standalone( const char *pszName, int iSize, void *pbuf )
{
	// If TeamFortressViewport is available, use it
	if (gViewPortMsgs)
	{
		gViewPortMsgs->MsgFunc_MOTD( pszName, iSize, pbuf );
		return;
	}

	// Standalone MOTD handling for BMod
	if (g_bGotAllMOTD)
	{
		// Previous MOTD was complete, start fresh
		g_szMOTD[0] = '\0';
	}

	BEGIN_READ( pbuf, iSize );

	g_bGotAllMOTD = READ_BYTE() != 0;  // true when this is the last chunk

	int roomInArray = sizeof(g_szMOTD) - strlen(g_szMOTD) - 1;
	const char *chunk = READ_STRING();

	if (roomInArray > 0 && chunk)
	{
		strncat(g_szMOTD, chunk, roomInArray);
		g_szMOTD[sizeof(g_szMOTD) - 1] = '\0';
	}

	// When we have the full message, show the MOTD panel
	if (g_bGotAllMOTD && g_szMOTD[0] != '\0')
	{
		CClientMOTD *pMOTD = GetStandaloneMOTDPanel();
		if (pMOTD)
		{
			// Get server name from engine if connected
			char serverName[256] = "Server";
			if (engine)
			{
				// Try to get net_name convar or server info
				const ConVar *pHostname = cvar->FindVar("hostname");
				if (pHostname && pHostname->GetString()[0])
				{
					Q_strncpy(serverName, pHostname->GetString(), sizeof(serverName));
				}
			}

			DevMsg("MOTD: Showing MOTD panel with %d bytes of content\n", (int)strlen(g_szMOTD));
			pMOTD->Activate(serverName, g_szMOTD);

			// Move to front and request focus
			pMOTD->MoveToFront();
			pMOTD->RequestFocus();
		}
	}
}
// MOTD is now registered via DECLARE_MESSAGE/HOOK_MESSAGE system in hud.cpp
// USER_MESSAGE_REGISTER( MOTD ); // Removed - using CHud member function instead


void __MsgFunc_VGUIMenu( const char *pszName, int iSize, void *pbuf )
{
	// Only call if viewport is available
	if (gViewPortMsgs)
	{
		gViewPortMsgs->MsgFunc_VGUIMenu( pszName, iSize, pbuf );
	}
}
USER_MESSAGE_REGISTER( VGUIMenu );


