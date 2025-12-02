//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: User message registration and dispatch system (2007 protocol)
//
//=============================================================================//

#ifndef USERMESSAGES_H
#define USERMESSAGES_H
#ifdef _WIN32
#pragma once
#endif

#include "utldict.h"
#include "utlvector.h"
#include "bitbuf.h"

// Client dispatch function for usermessages - uses bf_read for efficient binary reading
typedef void (*pfnUserMsgHook)(bf_read &msg);

//-----------------------------------------------------------------------------
// Purpose: Individual user message entry
//-----------------------------------------------------------------------------
class CUserMessage
{
public:
	CUserMessage() : size(0), name(NULL) {}

	// byte size of message, or -1 for variable sized
	int				size;
	// Message name (pointer to registered name string)
	const char		*name;
	// Client only dispatch functions for message (supports multiple hooks)
	CUtlVector<pfnUserMsgHook>	clienthooks;
};

//-----------------------------------------------------------------------------
// Purpose: Interface for registering and dispatching usermessages
// Shared code creates same ordered list on client/server
//-----------------------------------------------------------------------------
class CUserMessages
{
public:
	CUserMessages();
	~CUserMessages();

	// Returns -1 if not found, otherwise, returns appropriate index
	int		LookupUserMessage( const char *name );
	int		GetUserMessageSize( int index );
	const char *GetUserMessageName( int index );
	bool	IsValidIndex( int index );
	int		GetNumMessages() const { return m_UserMessages.Count(); }

	// Server only - register a new message type
	void	Register( const char *name, int size );

	// Client only - hook a message handler
	void	HookMessage( const char *name, pfnUserMsgHook hook );

	// Client only - dispatch an incoming message by type index
	bool	DispatchUserMessage( int msg_type, bf_read &msg_data );

private:
	CUtlDict< CUserMessage*, int >	m_UserMessages;
};

extern CUserMessages *usermessages;

#endif // USERMESSAGES_H
