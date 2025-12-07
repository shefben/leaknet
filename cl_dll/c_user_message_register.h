//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: Client-side user message hook registration system (2003 protocol)
//
// Provides two ways to register user message handlers:
// 1. HOOK_MESSAGE macro - for registering at initialization time
// 2. USER_MESSAGE_REGISTER macro - for static/global registration
//
//=============================================================================

#ifndef C_USER_MESSAGE_REGISTER_H
#define C_USER_MESSAGE_REGISTER_H
#ifdef _WIN32
#pragma once
#endif

#include "usermessages.h"

//-----------------------------------------------------------------------------
// USER_MESSAGE_REGISTER: Global/static registration macro
//
// Use this when you want to declare message handlers globally without needing
// to call them from an initialization function. Registers a function with the
// naming convention __MsgFunc_<msgName>
//
// Example:
//   void __MsgFunc_SayText( const char *pszName, int iSize, void *pbuf ) { ... }
//   USER_MESSAGE_REGISTER( SayText )
//-----------------------------------------------------------------------------
#define USER_MESSAGE_REGISTER( msgName ) \
	static CUserMessageRegister userMessageRegister_##msgName( #msgName, __MsgFunc_##msgName );

//-----------------------------------------------------------------------------
// HOOK_MESSAGE: Traditional registration macro
//
// Use this within initialization functions to hook message handlers.
// The handler function must be named __MsgFunc_<name>
//
// Example:
//   void __MsgFunc_SayText( const char *pszName, int iSize, void *pbuf ) { ... }
//   // In Init():
//   HOOK_MESSAGE( SayText );
//-----------------------------------------------------------------------------
#define HOOK_MESSAGE( name ) \
	usermessages->HookMessage( #name, __MsgFunc_##name );

//-----------------------------------------------------------------------------
// Purpose: Static registration class for user message handlers
//          Creates a linked list of handlers registered at global init time
//-----------------------------------------------------------------------------
class CUserMessageRegister
{
public:
	CUserMessageRegister( const char *pMessageName, pfnUserMsgHook pHookFn );

	// Called during client initialization to register all static hooks
	static void RegisterAll();

private:
	const char *m_pMessageName;
	pfnUserMsgHook m_pHookFn;

	// Linked list of all CUserMessageRegister instances
	static CUserMessageRegister *s_pHead;
	CUserMessageRegister *m_pNext;
};

#endif // C_USER_MESSAGE_REGISTER_H
