#include "cbase.h"
#include "gmod_menus.h"

#include "filesystem.h"
#include "cdll_int.h"
#include "ienginevgui.h"
#include "convar.h"
#include "KeyValues.h"
#include "fmtstr.h"
#include "vstdlib/strtools.h"
#include <vgui/ISurface.h>

#include <vgui/ISurface.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/Label.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IVEngineClient *engine;
extern IEngineVGui *enginevgui;

// --------------------------------------------------------------------------------------------------
// Utility helpers
// --------------------------------------------------------------------------------------------------
static int GetIntFromTextEntry( vgui::TextEntry *entry, int fallback )
{
	if ( !entry )
		return fallback;

	char buf[64];
	entry->GetText( buf, sizeof( buf ) );
	return Q_atoi( buf ) ? Q_atoi( buf ) : fallback;
}

static void SetTextIfPresent( vgui::TextEntry *entry, const char *text )
{
	if ( entry )
	{
		entry->SetText( text ? text : "" );
	}
}

static void SetConVarBool( const char *name, bool value )
{
	if ( engine )
	{
		engine->ClientCmd( CFmtStr( "%s %d\n", name, value ? 1 : 0 ) );
	}
}

static void SetConVarInt( const char *name, int value )
{
	if ( engine )
	{
		engine->ClientCmd( CFmtStr( "%s %d\n", name, value ) );
	}
}

// --------------------------------------------------------------------------------------------------
// Mod Manager
// --------------------------------------------------------------------------------------------------
CGModModManagerDialog::CGModModManagerDialog( vgui::VPANEL parent ) : BaseClass( NULL, "GModModManager" )
{
	SetParent( parent );
	SetSize( 640, 420 );
	SetTitle( "Mod Manager", true );
	SetSizeable( true );
	SetMoveable( true );

	m_pList = new vgui::ListPanel( this, "ModList" );
	m_pList->AddColumnHeader( 0, "Name", "Name", 160 );
	m_pList->AddColumnHeader( 1, "Status", "Status", 80 );
	m_pList->AddColumnHeader( 2, "Version", "Version", 80 );
	m_pList->AddColumnHeader( 3, "Folder", "Folder", 140 );
	m_pList->SetBounds( 10, 35, 360, 340 );

	m_pToggleButton = new vgui::Button( this, "Toggle", "Enable/Disable" );
	m_pToggleButton->SetBounds( 10, 380, 120, 24 );
	m_pToggleButton->SetCommand( new KeyValues( "Command", "command", "Toggle" ) );

	vgui::Button *refresh = new vgui::Button( this, "Refresh", "Refresh" );
	refresh->SetBounds( 140, 380, 80, 24 );
	refresh->SetCommand( new KeyValues( "Command", "command", "Refresh" ) );

	vgui::Button *close = new vgui::Button( this, "Close", "Close" );
	close->SetBounds( 230, 380, 60, 24 );
	close->SetCommand( new KeyValues( "Command", "command", "Close" ) );

	m_pDetails = new vgui::RichText( this, "Details" );
	m_pDetails->SetBounds( 380, 35, 240, 340 );

	ActivateAndRefresh();
}

void CGModModManagerDialog::ActivateAndRefresh()
{
	AdjustLayout();
	PopulateModList();
	MoveToFront();
	SetVisible( true );
	RequestFocus();
}

void CGModModManagerDialog::PopulateModList()
{
	m_pList->DeleteAllItems();
	m_pDetails->SetText( "" );

	FileFindHandle_t handle = FILESYSTEM_INVALID_FIND_HANDLE;
	const char *pszFile = filesystem->FindFirst( "mods/*", &handle );
	while ( pszFile )
	{
		if ( pszFile[0] != '.' )
		{
			char modInfoPath[MAX_PATH];
			Q_snprintf( modInfoPath, sizeof( modInfoPath ), "mods/%s/modinfo.txt", pszFile );

			char disabledPath[MAX_PATH];
			Q_snprintf( disabledPath, sizeof( disabledPath ), "mods/%s/DISABLED", pszFile );

			KeyValues *kv = new KeyValues( "modinfo" );
			const bool hasInfo = kv->LoadFromFile( filesystem, modInfoPath, "MOD" );

			const bool disabled = filesystem->FileExists( disabledPath, "MOD" );
			const char *modName = hasInfo ? kv->GetString( "name", pszFile ) : pszFile;
			const char *version = hasInfo ? kv->GetString( "version", "n/a" ) : "n/a";
			const char *author = hasInfo ? kv->GetString( "author_name", "" ) : "";

			KeyValues *row = new KeyValues( "row" );
			row->SetString( "Name", modName );
			row->SetString( "Status", disabled ? "Disabled" : "Enabled" );
			row->SetString( "Version", version );
			row->SetString( "Folder", pszFile );
			row->SetString( "Author", author );
			m_pList->AddItem( row, 0, false, false );

			kv->deleteThis();
		}

		pszFile = filesystem->FindNext( handle );
	}

	if ( handle != FILESYSTEM_INVALID_FIND_HANDLE )
	{
		filesystem->FindClose( handle );
	}
}

void CGModModManagerDialog::EnableDisableSelected()
{
	int itemID = m_pList->GetSelectedItem( 0 );
	if ( itemID == -1 )
		return;

	KeyValues *data = m_pList->GetItem( itemID );
	if ( !data )
		return;

	const char *folder = data->GetString( "Folder", "" );
	if ( !folder[0] )
		return;

	char disabledPath[MAX_PATH];
	Q_snprintf( disabledPath, sizeof( disabledPath ), "mods/%s/DISABLED", folder );

	if ( filesystem->FileExists( disabledPath, "MOD" ) )
	{
		filesystem->RemoveFile( disabledPath, "MOD" );
	}
	else
	{
		FileHandle_t f = filesystem->Open( disabledPath, "w", "MOD" );
		if ( f != FILESYSTEM_INVALID_HANDLE )
		{
			filesystem->Close( f );
		}
	}

	PopulateModList();
}

void CGModModManagerDialog::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "Toggle" ) )
	{
		EnableDisableSelected();
	}
	else if ( !Q_stricmp( command, "Refresh" ) )
	{
		PopulateModList();
	}
	else if ( !Q_stricmp( command, "Close" ) )
	{
		Close();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CGModModManagerDialog::AdjustLayout()
{
	int x, y, ww, wt;
	vgui::surface()->GetWorkspaceBounds( x, y, ww, wt );

	const int baseW = 640;
	const int contentH = 420;
	float scale = 1.0f;
	if ( ww > 0 && wt > 0 )
	{
		float sx = ( ww - 30 ) / (float)baseW;
		float sy = ( wt - 30 ) / (float)contentH;
		scale = ( sx < sy ? sx : sy );
		if ( scale > 1.0f )
			scale = 1.0f;
		if ( scale < 0.5f )
			scale = 0.5f;
	}

	int w = (int)( baseW * scale );
	int h = (int)( contentH * scale );
	SetSize( w, h );

	auto S = [scale]( int v ) { return (int)( v * scale ); };

	m_pList->SetBounds( S( 10 ), S( 35 ), S( 360 ), S( 340 ) );
	m_pDetails->SetBounds( S( 380 ), S( 35 ), S( 240 ), S( 340 ) );
	m_pToggleButton->SetBounds( S( 10 ), S( 380 ), S( 120 ), S( 24 ) );

	if ( vgui::Button *refresh = dynamic_cast<vgui::Button*>( FindChildByName( "Refresh" ) ) )
	{
		refresh->SetBounds( S( 140 ), S( 380 ), S( 80 ), S( 24 ) );
	}
	if ( vgui::Button *close = dynamic_cast<vgui::Button*>( FindChildByName( "Close" ) ) )
	{
		close->SetBounds( S( 230 ), S( 380 ), S( 60 ), S( 24 ) );
	}

	SetPos( x + ( ww - w ) / 2, y + ( wt - h ) / 2 );
}

// --------------------------------------------------------------------------------------------------
// Base server dialog
// --------------------------------------------------------------------------------------------------
CGModBaseServerDialog::CGModBaseServerDialog( vgui::VPANEL parent, bool multiplayer )
	: BaseClass( NULL, multiplayer ? "GModMultiplayer" : "GModSingleplayer" )
	, m_bMultiplayer( multiplayer )
{
	SetParent( parent );
	SetSize( 720, 520 );
	SetTitle( multiplayer ? "Create Multiplayer Server" : "Start Singleplayer Game", true );
	SetSizeable( true );
	SetMoveable( true );

	const int leftMargin = 10;
	const int topMargin = 35;

	m_pMapList = new vgui::ListPanel( this, "MapList" );
	m_pMapList->AddColumnHeader( 0, "Map", "Map", 200 );
	m_pMapList->SetBounds( leftMargin, topMargin, 260, 420 );

	int controlX = 280;
	int lineY = topMargin;

	m_pHostname = new vgui::TextEntry( this, "Hostname" );
	m_pHostname->SetBounds( controlX + 120, lineY, 200, 20 );
	vgui::Label *hostnameLabel = new vgui::Label( this, "HostnameLabel", "Server Name:" );
	hostnameLabel->SetPos( controlX, lineY );
	lineY += 28;

	m_pPassword = new vgui::TextEntry( this, "Password" );
	m_pPassword->SetBounds( controlX + 120, lineY, 200, 20 );
	vgui::Label *passwordLabel = new vgui::Label( this, "PasswordLabel", "Password:" );
	passwordLabel->SetPos( controlX, lineY );
	lineY += 28;

	m_pMaxPlayers = new vgui::TextEntry( this, "MaxPlayers" );
	m_pMaxPlayers->SetBounds( controlX + 120, lineY, 60, 20 );
	vgui::Label *maxPlayersLabel = new vgui::Label( this, "MaxPlayersLabel", "Max Players:" );
	maxPlayersLabel->SetPos( controlX, lineY );
	lineY += 36;

	auto makeCheck = [&]( const char *name, const char *label, int yOffset, vgui::CheckButton **out )
	{
		*out = new vgui::CheckButton( this, name, label );
		(*out)->SetBounds( controlX, yOffset, 200, 20 );
	};

	makeCheck( "AllWeapons", "All Weapons", lineY, &m_pAllWeapons ); lineY += 20;
	makeCheck( "AllowIgnite", "Allow Ignite", lineY, &m_pAllowIgnite ); lineY += 20;
	makeCheck( "NoClip", "Allow NoClip", lineY, &m_pNoClip ); lineY += 20;
	makeCheck( "PlayerDamage", "Player Damage", lineY, &m_pPlayerDamage ); lineY += 20;
	makeCheck( "PvPDamage", "PvP Damage", lineY, &m_pPvPDamage ); lineY += 20;
	makeCheck( "TeamDamage", "Team Damage", lineY, &m_pTeamDamage ); lineY += 20;
	makeCheck( "AllowNPCs", "Allow NPCs", lineY, &m_pAllowNPCs ); lineY += 20;
	makeCheck( "AllowSpawning", "Allow Spawning", lineY, &m_pAllowSpawning ); lineY += 20;
	makeCheck( "AllowMultiGun", "Allow Multigun", lineY, &m_pAllowMultiGun ); lineY += 20;
	makeCheck( "AllowPhysgun", "Allow Physgun", lineY, &m_pAllowPhysgun ); lineY += 28;

	auto makeLimit = [&]( const char *name, const char *label, int yOffset, vgui::TextEntry **out )
	{
		vgui::Label *lbl = new vgui::Label( this, CFmtStr( "%sLabel", name ), label );
		lbl->SetPos( controlX, yOffset );
		*out = new vgui::TextEntry( this, name );
		(*out)->SetBounds( controlX + 140, yOffset, 60, 20 );
	};

	makeLimit( "LimitRagdolls", "Max Ragdolls:", lineY, &m_pLimitRagdolls ); lineY += 22;
	makeLimit( "LimitThrusters", "Max Thrusters:", lineY, &m_pLimitThrusters ); lineY += 22;
	makeLimit( "LimitProps", "Max Props:", lineY, &m_pLimitProps ); lineY += 22;
	makeLimit( "LimitBalloons", "Max Balloons:", lineY, &m_pLimitBalloons ); lineY += 22;
	makeLimit( "LimitEffects", "Max Effects:", lineY, &m_pLimitEffects ); lineY += 22;
	makeLimit( "LimitSprites", "Max Sprites:", lineY, &m_pLimitSprites ); lineY += 22;
	makeLimit( "LimitEmitters", "Max Emitters:", lineY, &m_pLimitEmitters ); lineY += 22;
	makeLimit( "LimitWheels", "Max Wheels:", lineY, &m_pLimitWheels ); lineY += 22;
	makeLimit( "LimitNPCs", "Max NPCs:", lineY, &m_pLimitNPCs ); lineY += 22;
	makeLimit( "LimitVehicles", "Max Vehicles:", lineY, &m_pLimitVehicles ); lineY += 30;

	vgui::Button *start = new vgui::Button( this, "Start", multiplayer ? "Start Multiplayer" : "Start Singleplayer" );
	start->SetBounds( controlX, lineY, 150, 24 );
	start->SetCommand( new KeyValues( "Command", "command", "Start" ) );

	vgui::Button *close = new vgui::Button( this, "Close", "Close" );
	close->SetBounds( controlX + 160, lineY, 80, 24 );
	close->SetCommand( new KeyValues( "Command", "command", "Close" ) );

	PopulateMapList();
}

void CGModBaseServerDialog::ActivateAndRefresh()
{
	AdjustLayout();
	PopulateMapList();
	MoveToFront();
	SetVisible( true );
	RequestFocus();
}

void CGModBaseServerDialog::PopulateMapList()
{
	m_pMapList->DeleteAllItems();

	FileFindHandle_t handle = FILESYSTEM_INVALID_FIND_HANDLE;
	const char *pszFile = filesystem->FindFirst( "maps/*.bsp", &handle );
	while ( pszFile )
	{
		if ( pszFile[0] != '.' )
		{
			char mapName[256];
			Q_strncpy( mapName, pszFile, sizeof( mapName ) );
			char *dot = Q_strrchr( mapName, '.' );
			if ( dot )
			{
				*dot = 0;
			}
			KeyValues *row = new KeyValues( "row" );
			row->SetString( "Map", mapName );
			m_pMapList->AddItem( row, 0, false, false );
		}
		pszFile = filesystem->FindNext( handle );
	}
	if ( handle != FILESYSTEM_INVALID_FIND_HANDLE )
	{
		filesystem->FindClose( handle );
	}

	if ( m_pMapList->GetItemCount() > 0 )
	{
		m_pMapList->SetSingleSelectedItem( m_pMapList->GetItemIDFromRow( 0 ) );
	}
}

const char *CGModBaseServerDialog::GetSelectedMap() const
{
	int itemID = m_pMapList->GetSelectedItem( 0 );
	if ( itemID == -1 )
		return NULL;

	KeyValues *data = m_pMapList->GetItem( itemID );
	return data ? data->GetString( "Map", NULL ) : NULL;
}

void CGModBaseServerDialog::ApplyCommonCvars() const
{
	SetConVarBool( "gm_sv_allweapons", m_pAllWeapons->IsSelected() );
	SetConVarBool( "gm_sv_allowignite", m_pAllowIgnite->IsSelected() );
	SetConVarBool( "gm_sv_noclip", m_pNoClip->IsSelected() );
	SetConVarBool( "gm_sv_playerdamage", m_pPlayerDamage->IsSelected() );
	SetConVarBool( "gm_sv_pvpdamage", m_pPvPDamage->IsSelected() );
	SetConVarBool( "gm_sv_teamdamage", m_pTeamDamage->IsSelected() );
	SetConVarBool( "gm_sv_allownpc", m_pAllowNPCs->IsSelected() );
	SetConVarBool( "gm_sv_allowspawning", m_pAllowSpawning->IsSelected() );
	SetConVarBool( "gm_sv_allowmultigun", m_pAllowMultiGun->IsSelected() );
	SetConVarBool( "gm_sv_allowphysgun", m_pAllowPhysgun->IsSelected() );

	SetConVarInt( "gm_sv_clientlimit_ragdolls", GetIntFromTextEntry( m_pLimitRagdolls, 2 ) );
	SetConVarInt( "gm_sv_clientlimit_thrusters", GetIntFromTextEntry( m_pLimitThrusters, 10 ) );
	SetConVarInt( "gm_sv_clientlimit_props", GetIntFromTextEntry( m_pLimitProps, 30 ) );
	SetConVarInt( "gm_sv_clientlimit_balloons", GetIntFromTextEntry( m_pLimitBalloons, 10 ) );
	SetConVarInt( "gm_sv_clientlimit_effects", GetIntFromTextEntry( m_pLimitEffects, 10 ) );
	SetConVarInt( "gm_sv_clientlimit_sprites", GetIntFromTextEntry( m_pLimitSprites, 1 ) );
	SetConVarInt( "gm_sv_clientlimit_emitters", GetIntFromTextEntry( m_pLimitEmitters, 1 ) );
	SetConVarInt( "gm_sv_clientlimit_wheels", GetIntFromTextEntry( m_pLimitWheels, 1 ) );
	SetConVarInt( "gm_sv_clientlimit_npcs", GetIntFromTextEntry( m_pLimitNPCs, 1 ) );
	SetConVarInt( "gm_sv_clientlimit_vehicles", GetIntFromTextEntry( m_pLimitVehicles, 1 ) );

	// Standard server cvars
	char buf[128];
	m_pHostname->GetText( buf, sizeof( buf ) );
	engine->ClientCmd( CFmtStr( "hostname \"%s\"\n", buf ) );

	char passBuf[64];
	m_pPassword->GetText( passBuf, sizeof( passBuf ) );
	engine->ClientCmd( CFmtStr( "sv_password \"%s\"\n", passBuf ) );
}

void CGModBaseServerDialog::AdjustLayout()
{
	int x, y, ww, wt;
	vgui::surface()->GetWorkspaceBounds( x, y, ww, wt );

	const int baseW = 720;
	const int contentH = 615; // based on default layout height when unscaled
	float scale = 1.0f;
	if ( ww > 0 && wt > 0 )
	{
		float sx = ( ww - 30 ) / (float)baseW;
		float sy = ( wt - 30 ) / (float)contentH;
		scale = ( sx < sy ? sx : sy );
		if ( scale > 1.0f )
			scale = 1.0f;
		if ( scale < 0.5f )
			scale = 0.5f;
	}

	int w = (int)( baseW * scale );
	int h = (int)( contentH * scale );
	SetSize( w, h );

	auto S = [scale]( int v ) { return (int)( v * scale ); };

	const int leftMargin = S( 10 );
	const int topMargin = S( 35 );

	m_pMapList->SetBounds( leftMargin, topMargin, S( 260 ), S( 420 ) );

	int controlX = S( 280 );
	int lineY = topMargin;

	m_pHostname->SetBounds( controlX + S( 120 ), lineY, S( 200 ), S( 20 ) );
	if ( vgui::Label *hostnameLabel = dynamic_cast<vgui::Label*>( FindChildByName( "HostnameLabel" ) ) )
	{
		hostnameLabel->SetPos( controlX, lineY );
	}
	lineY += S( 28 );

	m_pPassword->SetBounds( controlX + S( 120 ), lineY, S( 200 ), S( 20 ) );
	if ( vgui::Label *passwordLabel = dynamic_cast<vgui::Label*>( FindChildByName( "PasswordLabel" ) ) )
	{
		passwordLabel->SetPos( controlX, lineY );
	}
	lineY += S( 28 );

	m_pMaxPlayers->SetBounds( controlX + S( 120 ), lineY, S( 60 ), S( 20 ) );
	if ( vgui::Label *maxPlayersLabel = dynamic_cast<vgui::Label*>( FindChildByName( "MaxPlayersLabel" ) ) )
	{
		maxPlayersLabel->SetPos( controlX, lineY );
	}
	lineY += S( 36 );

	auto layoutChecks = [&]( vgui::CheckButton *btn )
	{
		if ( btn )
		{
			btn->SetBounds( controlX, lineY, S( 200 ), S( 20 ) );
			lineY += S( 20 );
		}
	};

	layoutChecks( m_pAllWeapons );
	layoutChecks( m_pAllowIgnite );
	layoutChecks( m_pNoClip );
	layoutChecks( m_pPlayerDamage );
	layoutChecks( m_pPvPDamage );
	layoutChecks( m_pTeamDamage );
	layoutChecks( m_pAllowNPCs );
	layoutChecks( m_pAllowSpawning );
	layoutChecks( m_pAllowMultiGun );
	layoutChecks( m_pAllowPhysgun );
	lineY += S( 8 );

	auto layoutLimit = [&]( const char *name, vgui::TextEntry *entry )
	{
		if ( !entry )
			return;
		if ( vgui::Label *lbl = dynamic_cast<vgui::Label*>( FindChildByName( CFmtStr( "%sLabel", name ) ) ) )
		{
			lbl->SetPos( controlX, lineY );
		}
		entry->SetBounds( controlX + S( 140 ), lineY, S( 60 ), S( 20 ) );
		lineY += S( 22 );
	};

	layoutLimit( "LimitRagdolls", m_pLimitRagdolls );
	layoutLimit( "LimitThrusters", m_pLimitThrusters );
	layoutLimit( "LimitProps", m_pLimitProps );
	layoutLimit( "LimitBalloons", m_pLimitBalloons );
	layoutLimit( "LimitEffects", m_pLimitEffects );
	layoutLimit( "LimitSprites", m_pLimitSprites );
	layoutLimit( "LimitEmitters", m_pLimitEmitters );
	layoutLimit( "LimitWheels", m_pLimitWheels );
	layoutLimit( "LimitNPCs", m_pLimitNPCs );
	layoutLimit( "LimitVehicles", m_pLimitVehicles );

	if ( vgui::Button *start = dynamic_cast<vgui::Button*>( FindChildByName( "Start" ) ) )
	{
		start->SetBounds( controlX, lineY, S( 150 ), S( 24 ) );
	}
	if ( vgui::Button *close = dynamic_cast<vgui::Button*>( FindChildByName( "Close" ) ) )
	{
		close->SetBounds( controlX + S( 160 ), lineY, S( 80 ), S( 24 ) );
	}

	SetPos( x + ( ww - w ) / 2, y + ( wt - h ) / 2 );
}

void CGModBaseServerDialog::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "Close" ) )
	{
		Close();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

// --------------------------------------------------------------------------------------------------
// Multiplayer dialog
// --------------------------------------------------------------------------------------------------
CGModMultiplayerDialog::CGModMultiplayerDialog( vgui::VPANEL parent )
	: BaseClass( parent, true )
{
	SetTitle( "Create Multiplayer Server", true );
	SetTextIfPresent( m_pHostname, "Garry's Mod Server" );
	SetTextIfPresent( m_pMaxPlayers, "4" );
}

void CGModMultiplayerDialog::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "Start" ) )
	{
		const char *mapName = GetSelectedMap();
		if ( !mapName )
		{
			Msg( "Select a map first.\n" );
			return;
		}

		ApplyCommonCvars();

		char maxPlayersBuf[16];
		m_pMaxPlayers->GetText( maxPlayersBuf, sizeof( maxPlayersBuf ) );
		if ( !Q_strlen( maxPlayersBuf ) )
		{
			Q_strncpy( maxPlayersBuf, "4", sizeof( maxPlayersBuf ) );
		}

		char cmd[512];
		Q_snprintf(
			cmd,
			sizeof( cmd ),
			"progress_enable; disconnect; wait; wait; wait; wait; maxplayers %s; sv_cheats 0; net_start; map %s",
			maxPlayersBuf,
			mapName );
		engine->ClientCmd( cmd );
		Close();
		return;
	}

	BaseClass::OnCommand( command );
}

// --------------------------------------------------------------------------------------------------
// Singleplayer dialog
// --------------------------------------------------------------------------------------------------
CGModSingleplayerDialog::CGModSingleplayerDialog( vgui::VPANEL parent )
	: BaseClass( parent, false )
{
	SetTitle( "Start Singleplayer Game", true );
	SetTextIfPresent( m_pHostname, "Singleplayer" );
	SetTextIfPresent( m_pMaxPlayers, "1" );

	// Singleplayer defaults: hide multiplayer-only toggles
	m_pAllowSpawning->SetSelected( true );
	m_pAllWeapons->SetSelected( true );
	m_pAllowNPCs->SetSelected( true );
}

void CGModSingleplayerDialog::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "Start" ) )
	{
		const char *mapName = GetSelectedMap();
		if ( !mapName )
		{
			Msg( "Select a map first.\n" );
			return;
		}

		ApplyCommonCvars();

		char cmd[512];
		Q_snprintf(
			cmd,
			sizeof( cmd ),
			"progress_enable; disconnect; wait; wait; wait; wait; maxplayers 1; sv_cheats 1; map %s;",
			mapName );
		engine->ClientCmd( cmd );
		Close();
		return;
	}

	BaseClass::OnCommand( command );
}

// --------------------------------------------------------------------------------------------------
// Console command entry points
// --------------------------------------------------------------------------------------------------
static CGModModManagerDialog *s_pModMenu = NULL;
static CGModMultiplayerDialog *s_pMPMenu = NULL;
static CGModSingleplayerDialog *s_pSPMenu = NULL;

void ShowModMenu()
{
	if ( !enginevgui )
		return;

	if ( !s_pModMenu )
	{
		s_pModMenu = new CGModModManagerDialog( enginevgui->GetPanel( PANEL_CLIENTDLL ) );
	}

	s_pModMenu->ActivateAndRefresh();
}

void ShowMPMenu()
{
	if ( !enginevgui )
		return;

	if ( !s_pMPMenu )
	{
		s_pMPMenu = new CGModMultiplayerDialog( enginevgui->GetPanel( PANEL_CLIENTDLL ) );
	}

	s_pMPMenu->ActivateAndRefresh();
}

void ShowSPMenu()
{
	if ( !enginevgui )
		return;

	if ( !s_pSPMenu )
	{
		s_pSPMenu = new CGModSingleplayerDialog( enginevgui->GetPanel( PANEL_CLIENTDLL ) );
	}

	s_pSPMenu->ActivateAndRefresh();
}

// Match original client console commands
CON_COMMAND( showmodmenu, "Opens the Garry's Mod mod manager" )
{
	ShowModMenu();
}

CON_COMMAND( showmpmenu, "Opens the Garry's Mod multiplayer server creator" )
{
	ShowMPMenu();
}

CON_COMMAND( showspmenu, "Opens the Garry's Mod singleplayer creator" )
{
	ShowSPMenu();
}
