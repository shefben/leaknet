//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: User message registration and dispatch system (2007 protocol)
//
//=============================================================================//

#include "cbase.h"
#include "usermessages.h"

// Forward declaration - implemented in game-specific code
void RegisterUserMessages( void );

//-----------------------------------------------------------------------------
// Purpose: Force registration on .dll load
//-----------------------------------------------------------------------------
CUserMessages::CUserMessages()
{
	// Game specific registration function
	RegisterUserMessages();
}

//-----------------------------------------------------------------------------
// Purpose: Clean up allocated message entries
//-----------------------------------------------------------------------------
CUserMessages::~CUserMessages()
{
	int c = m_UserMessages.Count();
	for ( int i = 0; i < c; ++i )
	{
		delete m_UserMessages[ i ];
	}
	m_UserMessages.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Find a user message by name
// Input  : *name - message name to find
// Output : int - index or -1 if not found
//-----------------------------------------------------------------------------
int CUserMessages::LookupUserMessage( const char *name )
{
	int idx = m_UserMessages.Find( name );
	if ( idx == m_UserMessages.InvalidIndex() )
	{
		return -1;
	}

	return idx;
}

//-----------------------------------------------------------------------------
// Purpose: Get the byte size of a user message
// Input  : index - message index
// Output : int - size in bytes, or -1 for variable sized
//-----------------------------------------------------------------------------
int CUserMessages::GetUserMessageSize( int index )
{
	if ( index < 0 || index >= (int)m_UserMessages.Count() )
	{
		Error( "CUserMessages::GetUserMessageSize( %i ) out of range!!!\n", index );
	}

	CUserMessage *entry = m_UserMessages[ index ];
	return entry->size;
}

//-----------------------------------------------------------------------------
// Purpose: Get the name of a user message by index
// Input  : index - message index
// Output : const char* - message name
//-----------------------------------------------------------------------------
const char *CUserMessages::GetUserMessageName( int index )
{
	if ( index < 0 || index >= (int)m_UserMessages.Count() )
	{
		Error( "CUserMessages::GetUserMessageName( %i ) out of range!!!\n", index );
	}

	return m_UserMessages.GetElementName( index );
}

//-----------------------------------------------------------------------------
// Purpose: Check if an index is valid
// Input  : index - message index to check
// Output : bool - true if valid
//-----------------------------------------------------------------------------
bool CUserMessages::IsValidIndex( int index )
{
	return m_UserMessages.IsValidIndex( index );
}

//-----------------------------------------------------------------------------
// Purpose: Register a new user message type (server only)
// Input  : *name - unique message name
//			size - byte size, or -1 for variable sized messages
//-----------------------------------------------------------------------------
void CUserMessages::Register( const char *name, int size )
{
	Assert( name );

	int idx = m_UserMessages.Find( name );
	if ( idx != m_UserMessages.InvalidIndex() )
	{
		Error( "CUserMessages::Register '%s' already registered\n", name );
	}

	CUserMessage *entry = new CUserMessage;
	entry->size = size;
	entry->name = name;

	m_UserMessages.Insert( name, entry );
}

//-----------------------------------------------------------------------------
// Purpose: Hook a client-side handler for a user message (client only)
//          Supports multiple hooks per message - all will be called in order
// Input  : *name - message name to hook
//			hook - callback function using bf_read for message data
//-----------------------------------------------------------------------------
void CUserMessages::HookMessage( const char *name, pfnUserMsgHook hook )
{
#if defined( CLIENT_DLL )
	Assert( name );
	Assert( hook );

	int idx = m_UserMessages.Find( name );
	if ( idx == m_UserMessages.InvalidIndex() )
	{
		DevMsg( "CUserMessages::HookMessage: no such message '%s'\n", name );
		Assert( 0 );
		return;
	}

	CUserMessage *entry = m_UserMessages[ idx ];

	// Add hook to the list - supports multiple hooks per message
	entry->clienthooks.AddToTail( hook );
#else
	Error( "CUserMessages::HookMessage called from server code!!!\n" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Dispatch a user message to all registered client hooks (client only)
//          Uses 2007 protocol with msg_type index and bf_read for efficient
//          binary reading of message data
// Input  : msg_type - message type index (from server)
//			msg_data - bf_read buffer containing message data
// Output : bool - true if message was handled
//-----------------------------------------------------------------------------
bool CUserMessages::DispatchUserMessage( int msg_type, bf_read &msg_data )
{
#if defined( CLIENT_DLL )
	if ( !IsValidIndex( msg_type ) )
	{
		DevMsg( "CUserMessages::DispatchUserMessage: Invalid message index %d\n", msg_type );
		Assert( 0 );
		return false;
	}

	CUserMessage *entry = m_UserMessages[ msg_type ];

	if ( entry->clienthooks.Count() == 0 )
	{
		DevMsg( "CUserMessages::DispatchUserMessage: No hooks registered for '%s' (index %d)\n",
			entry->name ? entry->name : "unknown", msg_type );
		return false;
	}

	// Remember the starting position so each hook sees the same data
	int startbit = msg_data.GetNumBitsRead();

	// Call all registered hooks for this message
	int hookCount = entry->clienthooks.Count();
	for ( int i = 0; i < hookCount; ++i )
	{
		// Reset read position for each hook so they all see complete message
		if ( i > 0 )
		{
			msg_data.Seek( startbit );
		}

		pfnUserMsgHook hook = entry->clienthooks[ i ];
		if ( hook )
		{
			(*hook)( msg_data );
		}
	}

	return true;
#else
	Error( "CUserMessages::DispatchUserMessage called from server code!!!\n" );
	return false;
#endif
}

// Singleton instance
static CUserMessages g_UserMessages;

// Expose to rest of .dll
CUserMessages *usermessages = &g_UserMessages;
