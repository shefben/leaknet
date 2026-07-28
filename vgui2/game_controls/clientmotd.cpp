//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <wchar.h>

#include <game_controls/clientmotd.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <keyvalues.h>
#include <vgui_controls/ImageList.h>

#include <vgui_controls/HTML.h>
#include <vgui_controls/Button.h>
#include <filesystem.h>

#include <game_controls/iviewport.h>

#include "IGameUIFuncs.h" // for key bindings
extern IGameUIFuncs *gameuifuncs; // for key binding details

using namespace vgui;


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClientMOTD::CClientMOTD(vgui::Panel *parent) : Frame(parent, "ClientMOTD")
{
	m_bFileWritten = false;
	strcpy( m_szTempFileName, "Resource/motd_temp.html");
	m_iScoreBoardKey = KEY_NONE;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

//	SetTitleBarVisible( false ); (do this in the .res file)

	m_pMessage = new HTML(this,"Message");

	LoadControlSettings("Resource/UI/MOTD.res");
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CClientMOTD::~CClientMOTD()
{
	// save the file to disk and then load it
	if( filesystem()->FileExists( m_szTempFileName ) )
	{
		filesystem()->RemoveFile( m_szTempFileName ,"GAME");
	}
}

//-----------------------------------------------------------------------------
// Purpose: called when a button is pressed
//-----------------------------------------------------------------------------
void CClientMOTD::OnCommand( const char *command)
{
    if (!_stricmp(command, "okay"))
    {
		// save the file to disk and then load it
		if( filesystem()->FileExists( m_szTempFileName ) )
		{
			filesystem()->RemoveFile( m_szTempFileName ,"GAME");
		}
		Close();
	}

	BaseClass::OnCommand(command);
}

// IClientMOTD interface calls

//-----------------------------------------------------------------------------
// Purpose: skips leading whitespace so a motd.txt that starts with a newline
//			still resolves to the URL/HTML it contains
//-----------------------------------------------------------------------------
static const char *SkipMOTDLeadingWhitespace( const char *str )
{
	if ( !str )
		return "";

	while ( *str == ' ' || *str == '\t' || *str == '\r' || *str == '\n' )
		str++;

	return str;
}

//-----------------------------------------------------------------------------
// Purpose: shows the MOTD
//-----------------------------------------------------------------------------
void CClientMOTD::Activate( const char *title, const char *msg )
{
	BaseClass::Activate();

	SetTitle( title, false );
	SetControlString( "serverName", title );

	if( IsURL( msg ) )
	{
		m_pMessage->OpenURL( SkipMOTDLeadingWhitespace( msg ) );
	}
	else
	{
		ShowLocalContent( msg );
	}

	if ( m_iScoreBoardKey == KEY_NONE )
	{
		m_iScoreBoardKey = gameuifuncs->GetVGUI2KeyCodeForBind( "showscores" );
	}

	SetVisible( true );
}

void CClientMOTD::Activate( const wchar_t *title, const wchar_t *msg )
{
	BaseClass::Activate();

	SetTitle( title, false );
	SetLabelText( "serverName", title );

	// The old code wrote the raw wchar_t buffer out with a byte count of
	// wcslen(), which truncated the body to half its length and emitted UTF-16
	// bytes into a file the browser reads as ANSI.  Convert first.
	int nAnsiLen = ( msg ? (int)wcslen( msg ) : 0 ) * 4 + 1;
	char *pAnsi = (char *)stackalloc( nAnsiLen );
	pAnsi[0] = 0;
	if ( msg )
	{
		localize()->ConvertUnicodeToANSI( msg, pAnsi, nAnsiLen );
	}

	if( IsURL( pAnsi ) )
	{
		m_pMessage->OpenURL( SkipMOTDLeadingWhitespace( pAnsi ) );
	}
	else
	{
		ShowLocalContent( pAnsi );
	}

	SetVisible( true );
}

//-----------------------------------------------------------------------------
// Purpose: renders an inline MOTD body in the HTML view.  Bodies that already
//			are an HTML document are handed straight to the browser; anything
//			else is plain text and gets wrapped so line breaks survive instead
//			of collapsing into one run-on paragraph.
//-----------------------------------------------------------------------------
void CClientMOTD::ShowLocalContent( const char *msg )
{
	if ( !msg )
	{
		msg = "";
	}

	// save the file to disk and then load it
	if( filesystem()->FileExists( m_szTempFileName ) )
	{
		filesystem()->RemoveFile( m_szTempFileName ,"GAME");
	}

	FileHandle_t f = filesystem()->Open( m_szTempFileName, "w+", "GAME" );
	if ( !f )
	{
		return;
	}

	if ( IsHTML( msg ) )
	{
		filesystem()->Write( msg, strlen( msg ), f );
	}
	else
	{
		// plain-text fallback - no HTML control markup in the body, so present
		// it as preformatted text rather than letting the browser reflow it.
		const char *pHeader =
			"<html><body bgcolor=\"#000000\" text=\"#ffffff\">"
			"<pre style=\"font-family:Verdana,Arial,sans-serif;font-size:12px;white-space:pre-wrap;\">";
		const char *pFooter = "</pre></body></html>";

		filesystem()->Write( pHeader, strlen( pHeader ), f );
		filesystem()->Write( msg, strlen( msg ), f );
		filesystem()->Write( pFooter, strlen( pFooter ), f );
	}

	filesystem()->Close( f );

	char localURL[ MAX_PATH + 8 ];
	Q_strncpy( localURL, "file:///", sizeof( localURL ) );

	int len = filesystem()->GetLocalPathLen( m_szTempFileName );
	if ( len <= 0 )
	{
		return;
	}

	char *pPathData = (char*)stackalloc( len + 1 );
	pPathData[0] = 0;
	if ( !filesystem()->GetLocalPath( m_szTempFileName, pPathData ) )
	{
		return;
	}

	Q_strncat( localURL, pPathData, sizeof( localURL ) );

	m_pMessage->OpenURL( localURL );
}

//-----------------------------------------------------------------------------
// Purpose: sets the localized text of a label
//-----------------------------------------------------------------------------
void CClientMOTD::SetLabelText(const char *textEntryName, const wchar_t *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetText(text);
	}
}

void CClientMOTD::OnKeyCodeTyped( KeyCode key )
{
	if ( key == KEY_SPACE )
	{
		OnCommand("okay");
	}
	else if ( m_iScoreBoardKey!=KEY_NONE && m_iScoreBoardKey == key )
	{
		if ( !gViewPortInterface->IsScoreBoardVisible() )
		{
			gViewPortInterface->HideBackGround();
			gViewPortInterface->ShowScoreBoard();
			SetVisible( false );
		}
	}
	else
	{
		BaseClass::OnKeyCodeTyped( key );
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the string looks like a url
//-----------------------------------------------------------------------------
bool CClientMOTD::IsURL( const char *str )
{
	if ( !str )
		return false;

	// tolerate leading whitespace/newlines from motd.txt
	while ( *str == ' ' || *str == '\t' || *str == '\r' || *str == '\n' )
		str++;

	if ( !_strnicmp( str, "http://", 7 ) && strlen( str ) > 7 )
		return true;

	if ( !_strnicmp( str, "https://", 8 ) && strlen( str ) > 8 )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the body is an inline HTML document rather than
//			plain text
//-----------------------------------------------------------------------------
bool CClientMOTD::IsHTML( const char *str )
{
	if ( !str )
		return false;

	while ( *str == ' ' || *str == '\t' || *str == '\r' || *str == '\n' )
		str++;

	return ( *str == '<' );
}

