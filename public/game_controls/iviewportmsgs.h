//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: interface class between viewport msg funcs and "c" based client dll
//          Updated for 2007 usermessage protocol (bf_read)
//
//=============================================================================//
#if !defined( IVIEWPORTMSGS_H )
#define IVIEWPORTMSGS_H
#ifdef _WIN32
#pragma once
#endif

#include "bitbuf.h"

class IViewPortMsgs
{
public:
	// Message Handlers (2007 protocol - bf_read)
	virtual int MsgFunc_ValClass( bf_read &msg ) = 0;
	virtual int MsgFunc_TeamNames( bf_read &msg ) = 0;
	virtual int MsgFunc_Feign( bf_read &msg ) = 0;
	virtual int MsgFunc_Detpack( bf_read &msg ) = 0;
	virtual int MsgFunc_VGUIMenu( bf_read &msg ) = 0;
	virtual int MsgFunc_MOTD( bf_read &msg ) = 0;
	virtual int MsgFunc_BuildSt( bf_read &msg ) = 0;
	virtual int MsgFunc_RandomPC( bf_read &msg ) = 0;
	virtual int MsgFunc_ServerName( bf_read &msg ) = 0;
	virtual int MsgFunc_ScoreInfo( bf_read &msg ) = 0;
	virtual int MsgFunc_TeamScore( bf_read &msg ) = 0;
	virtual int MsgFunc_TeamInfo( bf_read &msg ) = 0;
	virtual int MsgFunc_Spectator( bf_read &msg ) = 0;
	virtual int MsgFunc_SpecFade( bf_read &msg ) = 0;
	virtual int MsgFunc_ResetFade( bf_read &msg ) = 0;
};


extern IViewPortMsgs *gViewPortMsgs;

#endif // IVIEWPORTMSGS_H
