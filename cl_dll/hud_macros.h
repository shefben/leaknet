//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: HUD message and command hook macros (2003 protocol)
//
//=============================================================================

#if !defined( HUD_MACROS_H )
#define HUD_MACROS_H
#ifdef _WIN32
#pragma once
#endif

#include "usermessages.h"

//-----------------------------------------------------------------------------
// Message Hooks (2003 protocol)
//-----------------------------------------------------------------------------

// Hook a message handler to the usermessages system
#define HOOK_MESSAGE(x) usermessages->HookMessage(#x, __MsgFunc_##x );

// Message declaration for non-CHudElement classes
// Uses raw buffer with name, size, and buffer pointer (2003 protocol)
#define DECLARE_MESSAGE(y, x) void __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
	{											\
		y.MsgFunc_##x(pszName, iSize, pbuf );	\
	}

// Message declaration for CHudElement classes that use the hud element factory
// Uses raw buffer with name, size, and buffer pointer (2003 protocol)
#define DECLARE_HUD_MESSAGE(y, x) void __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
	{																\
		CHudElement *pElement = gHUD.FindElement( #y );				\
		if ( pElement )												\
		{															\
			((y *)pElement)->MsgFunc_##x(pszName, iSize, pbuf );	\
		}															\
	}

//-----------------------------------------------------------------------------
// Standalone Message Handlers (for messages not tied to HUD elements)
//-----------------------------------------------------------------------------

// Declare a standalone message handler function
// Uses raw buffer with name, size, and buffer pointer (2003 protocol)
#define DECLARE_MESSAGE_FUNC(x) void __MsgFunc_##x(const char *pszName, int iSize, void *pbuf)

//-----------------------------------------------------------------------------
// Command Hooks
//-----------------------------------------------------------------------------

// Hook a command to a static function
#define HOOK_COMMAND(x, y) static ConCommand x( #x, __CmdFunc_##y );

// Command declaration for non CHudElement classes
#define DECLARE_COMMAND(y, x) void __CmdFunc_##x( void ) \
	{							\
		y.UserCmd_##x( );		\
	}

// Command declaration for CHudElement classes that use the hud element factory
#define DECLARE_HUD_COMMAND(y, x) void __CmdFunc_##x( void )		\
	{																\
		CHudElement *pElement = gHUD.FindElement( #y );				\
		if ( pElement )												\
		{															\
			((y *)pElement)->UserCmd_##x( );						\
		}															\
	}

#define DECLARE_HUD_COMMAND_NAME(y, x, name) void __CmdFunc_##x( void )	\
	{																\
		CHudElement *pElement = gHUD.FindElement( name );			\
		if ( pElement )												\
		{															\
			((y *)pElement)->UserCmd_##x( );						\
		}															\
	}

#endif // HUD_MACROS_H
