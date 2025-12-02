//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: HUD message and command hook macros (2007 protocol)
//
//=============================================================================//

#if !defined( HUD_MACROS_H )
#define HUD_MACROS_H
#ifdef _WIN32
#pragma once
#endif

#include "usermessages.h"
#include "parsemsg.h"

//-----------------------------------------------------------------------------
// Message Hooks (2007 protocol)
//-----------------------------------------------------------------------------

// Hook a message handler to the usermessages system
#define HOOK_MESSAGE(x) usermessages->HookMessage(#x, __MsgFunc_##x );

// Message declaration for non-CHudElement classes
// Creates wrapper that initializes READ_* functions and forwards to class method
#define DECLARE_MESSAGE(y, x) void __MsgFunc_##x( bf_read &msg ) \
	{											\
		BEGIN_READ( msg );						\
		y.MsgFunc_##x( msg );					\
	}

// Message declaration for CHudElement classes that use the hud element factory
#define DECLARE_HUD_MESSAGE(y, x) void __MsgFunc_##x( bf_read &msg ) \
	{																\
		BEGIN_READ( msg );											\
		CHudElement *pElement = gHUD.FindElement( #y );				\
		if ( pElement )												\
		{															\
			((y *)pElement)->MsgFunc_##x( msg );					\
		}															\
	}

//-----------------------------------------------------------------------------
// Standalone Message Handlers (for messages not tied to HUD elements)
//-----------------------------------------------------------------------------

// Declare a standalone message handler function
// Use this for messages that don't go to a specific class
#define DECLARE_MESSAGE_FUNC(x) void __MsgFunc_##x( bf_read &msg )

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
