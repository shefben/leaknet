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

// Message opcode definitions
enum GModSpawnMsgType
{
	SPAWNMSG_CLEAR = 0,
	SPAWNMSG_ADD   = 1,
	SPAWNMSG_RELOAD= 2
};

// Forward declaration for HUD message binding (2003 protocol).
void __MsgFunc_GModSpawnList( const char *pszName, int iSize, void *pbuf );

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
	}

	void MsgFunc_GModSpawnList( const char *pszName, int iSize, void *pbuf )
	{
		BEGIN_READ( pbuf, iSize );

		int op = READ_BYTE();
		switch ( op )
		{
		case SPAWNMSG_CLEAR:
			CGModSpawnList::Shutdown();
			CGModSpawnList::Initialize();
			break;

		case SPAWNMSG_ADD:
		{
			char category[64];
			char display[128];
			char model[256];
			bool ragdoll = false;

			Q_strncpy( category, READ_STRING(), sizeof( category ) );
			Q_strncpy( display, READ_STRING(), sizeof( display ) );
			Q_strncpy( model, READ_STRING(), sizeof( model ) );
			ragdoll = READ_BYTE() ? true : false;

			// Add entry to spawn list
			CGModSpawnList::AddEntry( model, category );

			// Update flags on the freshly added entry
			int idx = CGModSpawnList::GetEntryCount() - 1;
			SpawnListEntry_t *pEntry = CGModSpawnList::GetEntry( idx );
			if ( pEntry )
			{
				Q_strncpy( pEntry->displayName, display, sizeof( pEntry->displayName ) );
				pEntry->isRagdoll = ragdoll;
			}
			break;
		}

		case SPAWNMSG_RELOAD:
			CGModSpawnList::ReloadSpawnMenu();
			break;

		default:
			break;
		}
	}
};

DECLARE_HUD_MESSAGE( CGModSpawnMenuNet, GModSpawnList );
DECLARE_HUDELEMENT( CGModSpawnMenuNet );
