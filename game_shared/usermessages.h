//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: User message registration and dispatch system (2003 protocol)
//
//=============================================================================

#ifndef USERMESSAGES_H
#define USERMESSAGES_H
#ifdef _WIN32
#pragma once
#endif

#include "utldict.h"
#include "utlvector.h"

// Client dispatch function for usermessages (2003 protocol)
// pszName: message name
// iSize: message size in bytes
// pbuf: raw message data buffer
typedef void (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);

//-----------------------------------------------------------------------------
// Purpose: Individual user message entry
//-----------------------------------------------------------------------------
struct UserMessage
{
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

	// Client only - dispatch an incoming message (2003 protocol)
	bool	DispatchUserMessage( const char *pszName, int iSize, void *pbuf );

private:
	CUtlDict< UserMessage*, int >	m_UserMessages;
};

extern CUserMessages *usermessages;

#endif // USERMESSAGES_H
