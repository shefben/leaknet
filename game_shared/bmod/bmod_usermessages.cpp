//========= Copyright � 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BMod (Barry's Mod / Garry's Mod clone) user messages registration
//          Based on GMod 9.0.4b message definitions from reverse engineering
//
//=============================================================================

#include "cbase.h"
#include "usermessages.h"
#include "shake.h"
#include "voice_gamemgr.h"
#include <bitbuf.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void RegisterBModUserMessages( void )
{
	DevMsg("BMod: RegisterUserMessages called - registering BMod user messages\n");
	// =======================================
	// Standard HL2/Source messages
	// Sizes match GMod 9.0.4b registration
	// =======================================
	usermessages->Register( "Geiger", 1 );
	usermessages->Register( "Train", 1 );
	usermessages->Register( "HudText", -1 );
	usermessages->Register( "SayText", -1 );
	usermessages->Register( "TextMsg", -1 );
	usermessages->Register( "HudMsg", -1 );           // HUD message display
	usermessages->Register( "ResetHUD", 1 );          // Called every respawn
	usermessages->Register( "GameTitle", 0 );         // Game title display
	usermessages->Register( "ItemPickup", -1 );
	usermessages->Register( "ShowMenu", -1 );
	usermessages->Register( "Shake", 13 );            // Screen shake
	usermessages->Register( "Fade", 10 );             // Screen fade
	usermessages->Register( "VGUIMenu", -1 );         // VGUI menu display
	usermessages->Register( "MOTD", -1 );             // Message of the Day (multiplayer)
	usermessages->Register( "Battery", 2 );
	usermessages->Register( "Damage", 18 );
	usermessages->Register( "VoiceMask", 9 );
	usermessages->Register( "RequestState", 0 );
	usermessages->Register( "CloseCaption", 6 );      // Closed captions
	usermessages->Register( "HintText", -1 );         // Hint display
	usermessages->Register( "SquadMemberDied", 0 );   // Squad death notification
	usermessages->Register( "AmmoDenied", 2 );        // Ammo pickup denied
	usermessages->Register( "CreditsMsg", 1 );        // Credits display
	usermessages->Register( "TerrainMod", -1 );       // Terrain modification
	usermessages->Register( "InitHUD", 0 );           // Called when player joins

	// =======================================
	// Additional standard messages (GMod 9 compatibility)
	// =======================================
	usermessages->Register( "GameMode", -1 );         // Gamemode info
	usermessages->Register( "ClearDecals", 0 );       // Clear all decals
	usermessages->Register( "DeathMsg", -1 );         // Death notification
	usermessages->Register( "TeamChange", -1 );       // Team change notification

	// =======================================
	// GMod Text/HUD overlay messages
	// Used for on-screen text and shape rendering
	// =======================================
	usermessages->Register( "GModText", -1 );         // Display text on screen
	usermessages->Register( "GModTextAnimate", -1 );  // Animate text properties
	usermessages->Register( "GModTextHide", -1 );     // Hide specific text
	usermessages->Register( "GModTextHideAll", 0 );   // Hide all text

	usermessages->Register( "GModRect", -1 );         // Display rectangle on screen
	usermessages->Register( "GModRectAnimate", -1 );  // Animate rectangle properties
	usermessages->Register( "GModRectHide", -1 );     // Hide specific rectangle
	usermessages->Register( "GModRectHideAll", 0 );   // Hide all rectangles

	// =======================================
	// GMod system messages
	// =======================================
	usermessages->Register( "GModVersion", -1 );      // Version info exchange

	// =======================================
	// GMod Spawn menu messages (original GMod 9 format)
	// Used for spawn list synchronization
	// =======================================
	usermessages->Register( "GModAddSpawnItem", -1 );      // Add item to spawn list
	usermessages->Register( "GModRemoveSpawnItem", -1 );   // Remove item from spawn list
	usermessages->Register( "GModRemoveSpawnCat", -1 );    // Remove spawn category
	usermessages->Register( "GModRemoveSpawnAll", 0 );     // Clear all spawn items
	usermessages->Register( "Spawn_SetCategory", -1 );     // Set current spawn category

	// BMod custom spawn list message (combined format with opcodes)
	// Format: byte opcode, then depends on opcode:
	//   SPAWNMSG_CLEAR (0): no additional data
	//   SPAWNMSG_ADD (1): string category, string displayname, string modelpath, byte isragdoll
	//   SPAWNMSG_RELOAD (2): no additional data
	usermessages->Register( "GModSpawnList", -1 );

	// =======================================
	// GMod World Quad messages
	// Used for rendering quads in 3D space
	// =======================================
	usermessages->Register( "WQuad", -1 );            // Display world quad
	usermessages->Register( "WQuadHide", -1 );        // Hide specific quad
	usermessages->Register( "WQuadHideAll", 0 );      // Hide all quads
	usermessages->Register( "WQuadAnimate", -1 );     // Animate quad properties

	// =======================================
	// BMod additional messages for tools/features
	// =======================================
	usermessages->Register( "GModToolInfo", -1 );     // Tool selection/info updates
	usermessages->Register( "GModToolError", -1 );    // Tool error messages
	usermessages->Register( "GModToolText", -1 );     // Toolgun crosshair text
	usermessages->Register( "GModHint", -1 );         // Hint message display
	usermessages->Register( "GModUndo", -1 );         // Undo notification
	usermessages->Register( "GModOverlay", -1 );      // Overlay effects (paint, etc)
}

//-----------------------------------------------------------------------------
// CUserMessages implementation
// DISABLED - Using main usermessages.cpp implementation to avoid conflicts
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Force registration on .dll load - DISABLED
//-----------------------------------------------------------------------------
/*
CUserMessages::CUserMessages()
{
	// Game specific registration function calls BMod implementation above
	RegisterBModUserMessages();
}
*/

/*
CUserMessages::~CUserMessages()
{
	int c = m_UserMessages.Count();
	for ( int i = 0; i < c; ++i )
	{
		delete m_UserMessages[ i ];
	}
	m_UserMessages.RemoveAll();
}
*/

//-----------------------------------------------------------------------------
// Purpose: Look up user message by name - DISABLED
//-----------------------------------------------------------------------------
/*
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
// Purpose: Get user message size by index
//-----------------------------------------------------------------------------
int CUserMessages::GetUserMessageSize( int index )
{
	if ( index < 0 || index >= (int)m_UserMessages.Count() )
	{
		Error( "CUserMessages::GetUserMessageSize( %i ) out of range!!!\n", index );
	}

	CUserMessage *e = m_UserMessages[ index ];
	return e->size;
}

//-----------------------------------------------------------------------------
// Purpose: Get user message name by index
//-----------------------------------------------------------------------------
const char *CUserMessages::GetUserMessageName( int index )
{
	if ( index < 0 || index >= (int)m_UserMessages.Count() )
	{
		Error( "CUserMessages::GetUserMessageSize( %i ) out of range!!!\n", index );
	}

	return m_UserMessages.GetElementName( index );
}

//-----------------------------------------------------------------------------
// Purpose: Check if index is valid
//-----------------------------------------------------------------------------
bool CUserMessages::IsValidIndex( int index )
{
	return m_UserMessages.IsValidIndex( index );
}

//-----------------------------------------------------------------------------
// Purpose: Register a user message
//-----------------------------------------------------------------------------
void CUserMessages::Register( const char *name, int size )
{
	Assert( name );
	int idx = m_UserMessages.Find( name );
	if ( idx != m_UserMessages.InvalidIndex() )
	{
		Error( "CUserMessages::Register '%s' already registered\n", name );
	}

	CUserMessage * entry = new CUserMessage;
	entry->size = size;
	entry->name = name;

	m_UserMessages.Insert( name, entry );
}

//-----------------------------------------------------------------------------
// Purpose: Hook a message (client-side only)
//-----------------------------------------------------------------------------
void CUserMessages::HookMessage( const char *name, pfnUserMsgHook hook )
{
#if defined( CLIENT_DLL )
	Assert( name );
	Assert( hook );

	int idx = m_UserMessages.Find( name );
	if ( idx == m_UserMessages.InvalidIndex() )
	{
		DevMsg( "CUserMessages::HookMessage:  no such message %s\n", name );
		Assert( 0 );
		return;
	}

	int i = m_UserMessages[ idx ]->clienthooks.AddToTail();
	m_UserMessages[ idx ]->clienthooks[i] = hook;

#else
	Error( "CUserMessages::HookMessage called from server code!!!\n" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Dispatch user message (client-side only)
//-----------------------------------------------------------------------------
bool CUserMessages::DispatchUserMessage( int msg_type, bf_read &msg_data )
{
#if defined( CLIENT_DLL )
	if ( msg_type < 0 || msg_type >= (int)m_UserMessages.Count() )
	{
		DevMsg( "CUserMessages::DispatchUserMessage:  Bogus msg type %i (max == %i)\n", msg_type, m_UserMessages.Count() );
		Assert( 0 );
		return false;
	}

	CUserMessage *entry = m_UserMessages[ msg_type ];

	if ( !entry )
	{
		DevMsg( "CUserMessages::DispatchUserMessage:  Missing client entry for msg type %i\n", msg_type );
		Assert( 0 );
		return false;
	}

	if ( entry->clienthooks.Count() == 0 )
	{
		DevMsg( "CUserMessages::DispatchUserMessage:  missing client hook for %s\n", GetUserMessageName(msg_type) );
		Assert( 0 );
		return false;
	}

	for (int i = 0; i < entry->clienthooks.Count(); i++  )
	{
		bf_read msg_copy = msg_data;

		pfnUserMsgHook hook = entry->clienthooks[i];
		(*hook)( msg_copy );
	}
	return true;
#else
	Error( "CUserMessages::DispatchUserMessage called from server code!!!\n" );
	return false;
#endif
}

// Singleton instance
// Note: usermessages global is defined in game_shared/usermessages.cpp
// static CUserMessages g_UserMessages;
// CUserMessages *usermessages = &g_UserMessages;

*/

// ALL DUPLICATE IMPLEMENTATIONS ABOVE HAVE BEEN DISABLED
// Using main game_shared/usermessages.cpp implementation instead
