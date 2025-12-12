//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: User message registration and dispatch system (2003 protocol)
//
//=============================================================================

#include "cbase.h"
#include "usermessages.h"

// Forward declarations - implemented in game-specific code
#if defined( BMOD_DLL )
void RegisterBModUserMessages( void );
#elif defined( HL1_DLL )
void RegisterHL1UserMessages( void );
#elif defined( CSTRIKE_DLL )
void RegisterCSUserMessages( void );
#elif defined( HL2_DLL )
void RegisterHL2UserMessages( void );
#elif defined( TF_DLL )
void RegisterTFUserMessages( void );
#else
// Default implementation
void RegisterUserMessages( void );
#endif

//-----------------------------------------------------------------------------
// Purpose: Force registration on .dll load
//-----------------------------------------------------------------------------
CUserMessages::CUserMessages()
{
#if defined( BMOD_DLL )
	RegisterBModUserMessages();
#elif defined( HL1_DLL )
	RegisterHL1UserMessages();
#elif defined( CSTRIKE_DLL )
	RegisterCSUserMessages();
#elif defined( HL2_DLL )
	RegisterHL2UserMessages();
#elif defined( TF_DLL )
	RegisterTFUserMessages();
#else
	RegisterUserMessages();
#endif
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

	UserMessage *entry = m_UserMessages[ index ];
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

	UserMessage *entry = new UserMessage;
	entry->size = size;
	entry->name = name;

	m_UserMessages.Insert( name, entry );
}

//-----------------------------------------------------------------------------
// Purpose: Hook a client-side handler for a user message (client only)
//          Supports multiple hooks per message - all will be called in order
// Input  : *name - message name to hook
//			hook - callback function (2003 protocol signature)
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

	UserMessage *entry = m_UserMessages[ idx ];

	// Add hook to the list - supports multiple hooks per message
	entry->clienthooks.AddToTail( hook );
#else
	Error( "CUserMessages::HookMessage called from server code!!!\n" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Dispatch a user message to all registered client hooks (client only)
//          Uses 2003 protocol with message name and raw buffer
// Input  : pszName - message name
//			iSize - message size in bytes
//			pbuf - raw message data buffer
// Output : bool - true if message was handled
//-----------------------------------------------------------------------------
bool CUserMessages::DispatchUserMessage( const char *pszName, int iSize, void *pbuf )
{
#if defined( CLIENT_DLL )
	int idx = m_UserMessages.Find( pszName );
	if ( idx == m_UserMessages.InvalidIndex() )
	{
		DevMsg( "CUserMessages::DispatchUserMessage: Unknown message '%s'\n", pszName );
		Assert( 0 );
		return false;
	}

	UserMessage *entry = m_UserMessages[ idx ];

	if ( entry->clienthooks.Count() == 0 )
	{
		DevMsg( "CUserMessages::DispatchUserMessage: No hooks registered for '%s'\n", pszName );
		return false;
	}

	// Call all registered hooks for this message
	int hookCount = entry->clienthooks.Count();
	for ( int i = 0; i < hookCount; ++i )
	{
		pfnUserMsgHook hook = entry->clienthooks[ i ];
		if ( hook )
		{
			(*hook)( pszName, iSize, pbuf );
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

// Implementation of RegisterUserMessages
// Registers essential user messages for HL2/Source Engine and GMod compatibility
void RegisterUserMessages( void )
{
	DevMsg("RegisterUserMessages: Registering standard HL2/Source user messages\n");

	// =======================================
	// Standard HL2/Source messages
	// Sizes match Source Engine 2003 specification
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
