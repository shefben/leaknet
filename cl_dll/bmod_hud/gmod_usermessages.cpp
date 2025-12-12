// GMod-specific usermessage handlers for the 2003 client protocol.
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "gmod_message.h"
#include "usermessages.h"
#include "parsemsg.h"
#include <vgui_controls/Panel.h>

// Forward declarations for HUD message binding (2003 protocol).
void __MsgFunc_GModHint( const char *pszName, int iSize, void *pbuf );
void __MsgFunc_GModToolText( const char *pszName, int iSize, void *pbuf );

// Simple HUD element used only to hook usermessages.
class CGModHudHooks : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CGModHudHooks, vgui::Panel );
public:
	CGModHudHooks( const char *pName ) : CHudElement( pName ), BaseClass( NULL, "GModHudHooks" )
	{
		SetParent( g_pClientMode->GetViewport() );
		SetVisible( false );
	}

	virtual void Init( void )
	{
		HOOK_MESSAGE( GModHint );
		HOOK_MESSAGE( GModToolText );
	}

	// Display a hint using the GMod message system (2003 protocol).
	void MsgFunc_GModHint( const char *pszName, int iSize, void *pbuf )
	{
		BEGIN_READ( pbuf, iSize );

		char text[256];
		Q_strncpy( text, READ_STRING(), sizeof(text) );
		float duration = READ_FLOAT();
		if ( duration <= 0.0f )
			duration = 5.0f;

		if ( !g_pGModMessageManager )
			return;

		CGModMessage *pMsg = g_pGModMessageManager->CreateMessage( "hint_message" );
		if ( pMsg )
		{
			pMsg->SetText( text );
			pMsg->SetCorner( GMOD_CORNER_BOTTOMLEFT );
			pMsg->SetPos( 32, 96 );
			pMsg->SetTime( duration );
		}
	}

	// Display toolgun crosshair text via GMod message system (2003 protocol).
	void MsgFunc_GModToolText( const char *pszName, int iSize, void *pbuf )
	{
		BEGIN_READ( pbuf, iSize );

		char text[256];
		Q_strncpy( text, READ_STRING(), sizeof(text) );

		if ( !g_pGModMessageManager )
			return;

		CGModMessage *pMsg = g_pGModMessageManager->CreateMessage( "tool_text" );
		if ( pMsg )
		{
			pMsg->SetText( text );
			pMsg->SetCorner( GMOD_CORNER_TOPLEFT );
			pMsg->SetPos( 16, 16 );
			pMsg->SetTime( 0.0f ); // stays until replaced
		}
	}
};

DECLARE_HUD_MESSAGE( CGModHudHooks, GModHint );
DECLARE_HUD_MESSAGE( CGModHudHooks, GModToolText );
DECLARE_HUDELEMENT( CGModHudHooks );
