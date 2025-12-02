//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef CREATEMULTIPLAYERGAMEDIALOG_H
#define CREATEMULTIPLAYERGAMEDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyDialog.h>

class CCreateMultiplayerGameServerPage;
class CCreateMultiplayerGameBotPage;

//-----------------------------------------------------------------------------
// Purpose: dialog for launching a listenserver
//-----------------------------------------------------------------------------
class CCreateMultiplayerGameDialog : public vgui::PropertyDialog
{
public:
	CCreateMultiplayerGameDialog(vgui::Panel *parent);
	~CCreateMultiplayerGameDialog();

protected:
	virtual void OnOK();
	virtual void OnClose();
	virtual void SetTitle(const char *title, bool surfaceTitle);

public:
	// methods to access server config data
	KeyValues *GetServerConfig() { return m_pServerConfig; }

private:
	typedef vgui::PropertyDialog BaseClass;

	CCreateMultiplayerGameServerPage *m_pServerPage;
	CCreateMultiplayerGameBotPage *m_pBotPage;

	// for loading/saving bot config
	KeyValues *m_pBotSavedData;

	// for loading/saving server config
	KeyValues *m_pServerConfig;
};


#endif // CREATEMULTIPLAYERGAMEDIALOG_H
