// GMod spawnmenu networking glue for the 2003 client protocol.
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "gmod_spawnlist.h"
#include "usermessages.h"
#include "parsemsg.h"
#include "iclientmode.h"
#include <vgui_controls/Panel.h>
#include "bmod_spawnmenu.h"
#include "bmod_proppanel.h"

// Message opcode definitions
enum GModSpawnMsgType
{
	SPAWNMSG_CLEAR = 0,
	SPAWNMSG_ADD   = 1,
	SPAWNMSG_RELOAD= 2
};

// SPAWNMSG_ADD entry types (trailing byte)
enum GModSpawnEntryType
{
	SPAWNENTRY_PROP    = 0,
	SPAWNENTRY_RAGDOLL = 1,
	SPAWNENTRY_COMMAND = 2	// server-categorized SWEP / spawnable weapon
};

// Forward declarations for HUD message binding (2003 protocol).
void __MsgFunc_GModSpawnList( const char *pszName, int iSize, void *pbuf );
void __MsgFunc_Spawn_SetCategory( const char *pszName, int iSize, void *pbuf );

// HUD element to receive usermessages and update spawnmenu data.
class CGModSpawnMenuNet : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CGModSpawnMenuNet, vgui::Panel );
public:
	CGModSpawnMenuNet( const char *pName ) : CHudElement( pName ), BaseClass( NULL, "GModSpawnMenuNet" )
	{
		SetParent( g_pClientMode->GetViewport() );
		SetVisible( false );
	}

	virtual void Init( void )
	{
		HOOK_MESSAGE( GModSpawnList );
		HOOK_MESSAGE( Spawn_SetCategory );
	}

	// Server/Lua tells us which dropdown category to show. Must stay hooked -
	// dispatching an unhooked usermessage Host_Error-disconnects the client.
	void MsgFunc_Spawn_SetCategory( const char *pszName, int iSize, void *pbuf )
	{
		BEGIN_READ( pbuf, iSize );

		char category[64];
		Q_strncpy( category, READ_STRING(), sizeof( category ) );

		CPropPanel *pPropPanel = g_pSpawnMenu ? g_pSpawnMenu->GetPropPanel() : NULL;
		if ( pPropPanel )
		{
			pPropPanel->SelectCategoryByName( category );
		}
	}

	void MsgFunc_GModSpawnList( const char *pszName, int iSize, void *pbuf )
	{
		BEGIN_READ( pbuf, iSize );

		CPropPanel *pPropPanel = g_pSpawnMenu ? g_pSpawnMenu->GetPropPanel() : NULL;

		int op = READ_BYTE();
		switch ( op )
		{
		case SPAWNMSG_CLEAR:
			CGModSpawnList::Shutdown();
			CGModSpawnList::Initialize();
			if ( pPropPanel )
			{
				pPropPanel->ClearNetworkedEntries();
			}
			break;

		case SPAWNMSG_ADD:
		{
			char category[64];
			char display[128];
			char model[256];

			Q_strncpy( category, READ_STRING(), sizeof( category ) );
			Q_strncpy( display, READ_STRING(), sizeof( display ) );
			Q_strncpy( model, READ_STRING(), sizeof( model ) );
			int entryType = READ_BYTE();

			if ( entryType == SPAWNENTRY_COMMAND )
			{
				// Server-categorized SWEP / weapon: shows up as a dropdown
				// category in the build menu; clicking runs the command.
				if ( pPropPanel )
				{
					pPropPanel->AddNetworkedEntry( category, display, model );
				}
				break;
			}

			// Add entry to spawn list
			CGModSpawnList::AddEntry( model, category );

			// Update flags on the freshly added entry
			int idx = CGModSpawnList::GetEntryCount() - 1;
			SpawnListEntry_t *pEntry = CGModSpawnList::GetEntry( idx );
			if ( pEntry )
			{
				Q_strncpy( pEntry->displayName, display, sizeof( pEntry->displayName ) );
				pEntry->isRagdoll = ( entryType == SPAWNENTRY_RAGDOLL );
			}
			break;
		}

		case SPAWNMSG_RELOAD:
			CGModSpawnList::ReloadSpawnMenu();
			if ( pPropPanel )
			{
				pPropPanel->RefreshAfterNetworkUpdate();
			}
			break;

		default:
			break;
		}
	}
};

DECLARE_HUD_MESSAGE( CGModSpawnMenuNet, GModSpawnList );
DECLARE_HUD_MESSAGE( CGModSpawnMenuNet, Spawn_SetCategory );
DECLARE_HUDELEMENT( CGModSpawnMenuNet );

//-----------------------------------------------------------------------------
// Ask the server for its SWEP/weapon spawn list each time a level starts.
//-----------------------------------------------------------------------------
class CGModSpawnListRequest : public CAutoGameSystem
{
public:
	CGModSpawnListRequest() : CAutoGameSystem( "CGModSpawnListRequest" ) {}

	virtual void LevelInitPostEntity()
	{
		engine->ServerCmd( "gmod_request_sweps\n" );
	}
};

static CGModSpawnListRequest g_GModSpawnListRequest;
