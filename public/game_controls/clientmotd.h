//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef CLIENTMOTD_H
#define CLIENTMOTD_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>

#include <stdlib.h> // _MAX_PATH

namespace vgui
{
	class TextEntry;
}

//-----------------------------------------------------------------------------
// Purpose: displays the MOTD
//-----------------------------------------------------------------------------
class CClientMOTD : public vgui::Frame
{
private:
	typedef vgui::Frame BaseClass;


public:
	CClientMOTD(vgui::Panel *parent);
	virtual ~CClientMOTD();

	virtual void Activate(const char *title,const char *message);
	virtual void Activate(const wchar_t *title,const wchar_t *message);

	// Content sniffing for the MOTD body.  Retail GMod lets a server ship either
	// an http(s) URL or an inline HTML document; anything else is plain text.
	static bool IsURL( const char *str );
	static bool IsHTML( const char *str );

protected:
	// vgui overrides
	virtual void OnCommand( const char *command);
	virtual void OnKeyCodeTyped( vgui::KeyCode key );

private:

	// helper functions
	void SetLabelText(const char *textEntryName, const wchar_t *text);

	// writes the body to the temp file and points the HTML view at it
	void ShowLocalContent( const char *msg );

	vgui::HTML *m_pMessage;

	bool m_bFileWritten;
	char m_szTempFileName[ MAX_PATH ];

	int m_iScoreBoardKey;
};


#endif // CLIENTMOTD_H
