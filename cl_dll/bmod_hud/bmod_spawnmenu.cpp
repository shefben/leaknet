//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Spawn Menu - Identical to Garry's Mod spawn menu system
//
//=============================================================================

#include "cbase.h"
#include "bmod_spawnmenu.h"
#include "bmod_proppanel.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/KeyCode.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/EditablePanel.h>
#include "filesystem.h"
#include "KeyValues.h"
#include "convar.h"
#include "iclientmode.h"
#include "ienginevgui.h"
#include "vstdlib/icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar gm_toolweapon;

static bool BM_IsSpecialFindName( const char *pFileName )
{
	return ( !pFileName || !pFileName[0] ||
		Q_strcmp( pFileName, "." ) == 0 ||
		Q_strcmp( pFileName, ".." ) == 0 );
}

//-----------------------------------------------------------------------------
// Console variables matching Garry's Mod
//-----------------------------------------------------------------------------
ConVar bm_snapangles("bm_snapangles", "45", FCVAR_ARCHIVE, "Snap angles for rotation in spawn menu");

//-----------------------------------------------------------------------------
// Global spawn menu instance
//-----------------------------------------------------------------------------
CClientSpawnDialog *g_pSpawnMenu = NULL;

//-----------------------------------------------------------------------------
// Console commands
//-----------------------------------------------------------------------------
CON_COMMAND( spawnmenu, "Opens the spawn menu" )
{
	if ( g_pSpawnMenu )
	{
		g_pSpawnMenu->ShowPanel( true );
	}
}

CON_COMMAND( gm_reloadspawnmenu, "Reloads the spawn menu configuration" )
{
	if ( g_pSpawnMenu )
	{
		g_pSpawnMenu->ReloadSpawnMenu();
	}
}

CON_COMMAND( gm_makecompletespawnlist, "Creates a complete spawn list" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg("Example:\n gm_makecompletespawnlist cstrike/models/\nWARNING: this WILL take a long time.");
		return;
	}

	const char *path = engine->Cmd_Argv(1);
	if ( g_pSpawnMenu )
	{
		g_pSpawnMenu->ScanPropsRecursive( path );
	}
}

CON_COMMAND( gm_context, "Show a GMod build-menu context panel" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: gm_context <context>\n" );
		return;
	}

	if ( g_pSpawnMenu && g_pSpawnMenu->GetContextPanel() )
	{
		g_pSpawnMenu->ShowPanel( true );
		g_pSpawnMenu->GetContextPanel()->ShowContext( engine->Cmd_Argv( 1 ) );
	}
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CClientSpawnDialog::CClientSpawnDialog( vgui::Panel *parent ) : BaseClass( parent, PANEL_SPAWNMENU )
{
	Msg("CClientSpawnDialog constructor\n");

	m_pViewPort = NULL;

	// Initialize all panel pointers to NULL
	m_pMainPanel = NULL;
	m_pPanel43 = NULL;
	m_pContextPanel = NULL;
	m_pToolButtonsPanel = NULL;
	m_pMinimizeButton = NULL;
	m_pPanel52 = NULL;
	m_pPanel54 = NULL;
	m_pPanel55 = NULL;
	m_pPanel122 = NULL;
	m_pPanel123 = NULL;
	m_pPanel124 = NULL;
	m_pPanel127 = NULL;

	// Initialize member variables
	m_szBuffer[0] = '\0';
	memset( m_bFlags, 0, sizeof(m_bFlags) );
	m_nConsoleHeight = 0;
	m_bInitialized = false;
	m_bAutoUpdate = true;
	m_bVisible = false;
	m_bShowingContext = false;

	// Load the scheme
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile( SPAWNMENU_SCHEME_FILE, SPAWNMENU_SCHEME_NAME );
	SetScheme( scheme );

	// Set global instance
	g_pSpawnMenu = this;

	// Get screen dimensions for sizing
	int screenWide, screenTall;
	vgui::surface()->GetScreenSize( screenWide, screenTall );

	// Set initial size - 790 pixels wide, calculated height
	m_nConsoleHeight = screenTall - 100;
	if ( m_nConsoleHeight > 3000 )
		m_nConsoleHeight = 3000;
	if ( m_nConsoleHeight < 550 )
		m_nConsoleHeight = 550;

	SetSize( 790, m_nConsoleHeight );
	SetPos( (screenWide - 790) / 2, (screenTall - m_nConsoleHeight) / 2 );

	Initialize();
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CClientSpawnDialog::~CClientSpawnDialog()
{
	g_pSpawnMenu = NULL;
}

//-----------------------------------------------------------------------------
// Initialize the spawn menu - matches Garry's Mod initialization
//-----------------------------------------------------------------------------
void CClientSpawnDialog::Initialize()
{
	if ( m_bInitialized )
		return;

	// Create main panel
	CreateMainPanel();

	// Create tool buttons panel
	CreateToolButtonsPanel();

	// Create context panel
	CreateContextPanel();

	// Create minimize button
	CreateMinimizeButton();

	// Load menu configuration
	LoadMenuConfiguration();

	m_bInitialized = true;
}

//-----------------------------------------------------------------------------
// Create main panel container
//-----------------------------------------------------------------------------
void CClientSpawnDialog::CreateMainPanel()
{
	m_pMainPanel = new vgui::Panel( this, "MainPanel" );
	m_pMainPanel->SetSize( 790, m_nConsoleHeight );
	m_pMainPanel->SetVisible( false );
	m_pMainPanel->SetMouseInputEnabled( true );
	m_pMainPanel->SetPos( 0, 0 );
}

//-----------------------------------------------------------------------------
// Create tool buttons panel
//-----------------------------------------------------------------------------
void CClientSpawnDialog::CreateToolButtonsPanel()
{
	m_pToolButtonsPanel = new CToolButtonsPanel( m_pMainPanel, "ToolButtonsPanel" );
	m_pToolButtonsPanel->SetParent( m_pMainPanel );
	m_pToolButtonsPanel->SetSize( 238, m_nConsoleHeight - 20 );
	m_pToolButtonsPanel->SetPos( 10, 10 );
	m_pToolButtonsPanel->SetVisible( true );
	m_pToolButtonsPanel->SetMouseInputEnabled( true );

	// Propagate scheme to child panel
	m_pToolButtonsPanel->SetScheme( GetScheme() );
}

//-----------------------------------------------------------------------------
// Create context panel
//-----------------------------------------------------------------------------
void CClientSpawnDialog::CreateContextPanel()
{
	m_pContextPanel = new CContextPanel( m_pMainPanel, "ContextPanel" );
	m_pContextPanel->SetParent( m_pMainPanel );
	m_pContextPanel->SetSize( 510, m_nConsoleHeight - 20 );
	m_pContextPanel->SetPos( 270, 10 );
	m_pContextPanel->SetVisible( true );
	m_pContextPanel->SetMouseInputEnabled( true );

	// Propagate scheme to child panel
	m_pContextPanel->SetScheme( GetScheme() );
}

//-----------------------------------------------------------------------------
// Create minimize button
//-----------------------------------------------------------------------------
void CClientSpawnDialog::CreateMinimizeButton()
{
	m_pMinimizeButton = new vgui::Button( m_pMainPanel, "ContextMinimize", "Seclude" );
	m_pMinimizeButton->SetVisible( false );

	// Position at top right of context panel
	int contextX, contextY, contextW, contextH;
	m_pContextPanel->GetBounds( contextX, contextY, contextW, contextH );

	int buttonW, buttonH;
	m_pMinimizeButton->GetSize( buttonW, buttonH );

	m_pMinimizeButton->SetPos(
		contextX + contextW - buttonW,
		contextY + contextH - buttonH - 18
	);
}

//-----------------------------------------------------------------------------
// Load menu configuration from files
//-----------------------------------------------------------------------------
void CClientSpawnDialog::LoadMenuConfiguration()
{
	LoadGModMenuConfiguration();
}

//-----------------------------------------------------------------------------
// Load GModMenu configuration - matches Garry's Mod file loading
//-----------------------------------------------------------------------------
void CClientSpawnDialog::LoadGModMenuConfiguration()
{
	KeyValues *pDefaultConfig = new KeyValues( "GModMenu" );

	// Load default configuration first
	if ( filesystem->FileExists( DEFAULT_CONFIG_FILE, "MOD" ) )
	{
		if ( pDefaultConfig->LoadFromFile( filesystem, DEFAULT_CONFIG_FILE, "MOD" ) )
		{
			LoadFromKeyValueInternal( pDefaultConfig );
		}
	}
	pDefaultConfig->deleteThis();

	// Load additional menu configurations from settings/menu_main/
	FileFindHandle_t findHandle;
	const char *pFilename = filesystem->FindFirst( "settings/menu_main/*.txt", &findHandle );

	while ( pFilename )
	{
		if ( Q_stricmp( "default.txt", pFilename ) != 0 )  // Skip default.txt as we already loaded it
		{
			char fullPath[256];
			Q_snprintf( fullPath, sizeof(fullPath), "settings/menu_main/%s", pFilename );

			KeyValues *pConfig = new KeyValues( "GModMenu" );
			if ( filesystem->FileExists( fullPath, "MOD" ) )
			{
				if ( pConfig->LoadFromFile( filesystem, fullPath, "MOD" ) )
				{
					LoadFromKeyValueInternal( pConfig );
				}
			}
			pConfig->deleteThis();
		}

		pFilename = filesystem->FindNext( findHandle );
	}
	filesystem->FindClose( findHandle );

	// Tool entries are loaded directly from settings/menu_main/*.txt by CToolButtonsPanel.
}

//-----------------------------------------------------------------------------
// Load configuration from KeyValues
//-----------------------------------------------------------------------------
void CClientSpawnDialog::LoadFromKeyValueInternal( KeyValues *pKeyValues )
{
	// Process configuration keys here
	// This would handle spawn menu configuration similar to Garry's Mod
	for ( KeyValues *pSubKey = pKeyValues->GetFirstSubKey(); pSubKey; pSubKey = pSubKey->GetNextKey() )
	{
		const char *keyName = pSubKey->GetName();
		const char *keyValue = pSubKey->GetString();

		// Process specific configuration keys
		if ( Q_stricmp( keyName, "AutoUpdate" ) == 0 )
		{
			m_bAutoUpdate = pSubKey->GetInt() != 0; // 2003 engine compatibility - GetBool() not available
		}
		// Add more configuration processing as needed
	}
}

//-----------------------------------------------------------------------------
// Scan for props recursively - matches Garry's Mod prop scanning
//-----------------------------------------------------------------------------
void CClientSpawnDialog::ScanPropsRecursive( const char *path )
{
	// Convert forward slashes to backslashes
	char searchPath[512];
	Q_strncpy( searchPath, path, sizeof(searchPath) );

	int len = Q_strlen( searchPath );
	for ( int i = 0; i < len; i++ )
	{
		if ( searchPath[i] == '/' )
			searchPath[i] = '\\';
	}

	Msg( "Scanning for [%s]\n", searchPath );

	// Create complete dump configuration
	KeyValues *pCompleteDump = new KeyValues( "CompleteDump" );

	// Save initial dump file
	pCompleteDump->SaveToFile( filesystem, COMPLETE_DUMP_FILE, "MOD" );

	// Search for .mdl files
	char modelPattern[512];
	Q_snprintf( modelPattern, sizeof(modelPattern), "%s*.mdl", searchPath );

	FileFindHandle_t findHandle;
	const char *pFilename = filesystem->FindFirst( modelPattern, &findHandle );

	// Add section header
	char sectionName[512];
	Q_snprintf( sectionName, sizeof(sectionName), "~%s", searchPath );
	pCompleteDump->SetString( sectionName, "1" );

	while ( pFilename )
	{
		char fullModelPath[512];
		Q_snprintf( fullModelPath, sizeof(fullModelPath), "%s%s", searchPath, pFilename );
		pCompleteDump->SetString( pFilename, fullModelPath );

		Msg( "[%s][%s]\n", pFilename, fullModelPath );

		pFilename = filesystem->FindNext( findHandle );
	}
	filesystem->FindClose( findHandle );

	// Save updated dump file
	pCompleteDump->SaveToFile( filesystem, COMPLETE_DUMP_FILE, "MOD" );
	pCompleteDump->deleteThis();

	// Recursively scan subdirectories
	char dirPattern[512];
	Q_snprintf( dirPattern, sizeof(dirPattern), "%s*", searchPath );

	FileFindHandle_t dirHandle;
	const char *pDirname = filesystem->FindFirst( dirPattern, &dirHandle );

	while ( pDirname )
	{
		char fullDirPath[512];
		Q_snprintf( fullDirPath, sizeof(fullDirPath), "%s%s", searchPath, pDirname );

		if ( filesystem->IsDirectory( fullDirPath, "MOD" ) &&
			 Q_stricmp( ".", pDirname ) != 0 &&
			 Q_stricmp( "..", pDirname ) != 0 )
		{
			char subDirPath[512];
			Q_snprintf( subDirPath, sizeof(subDirPath), "%s%s/", searchPath, pDirname );
			ScanPropsRecursive( subDirPath );
		}

		pDirname = filesystem->FindNext( dirHandle );
	}
	filesystem->FindClose( dirHandle );
}

//-----------------------------------------------------------------------------
// Reload spawn menu
//-----------------------------------------------------------------------------
void CClientSpawnDialog::ReloadSpawnMenu()
{
	LoadMenuConfiguration();
	if ( m_pToolButtonsPanel )
	{
		m_pToolButtonsPanel->LoadToolButtons();
	}
	if ( m_pContextPanel )
	{
		m_pContextPanel->LoadContextConfiguration();
		m_pContextPanel->ReloadProps();
	}
}

//-----------------------------------------------------------------------------
// Show/hide panel
//-----------------------------------------------------------------------------
void CClientSpawnDialog::ShowPanel( bool bShow )
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	m_bVisible = bShow;

	if ( bShow )
	{
		Activate();
		SetVisible( true );
		SetMouseInputEnabled( true );
		SetKeyBoardInputEnabled( true );
		SetKeyBoardInputEnabled( true );
		MoveToFront();
		RequestFocus();

		if ( m_pMainPanel )
			m_pMainPanel->SetVisible( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );

		if ( m_pMainPanel )
			m_pMainPanel->SetVisible( false );
	}

	// Optional: Show background if viewport is available
	if ( m_pViewPort )
		m_pViewPort->ShowBackGround( bShow );
}

//-----------------------------------------------------------------------------
// Apply scheme settings
//-----------------------------------------------------------------------------
void CClientSpawnDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// Set colors and fonts to match Garry's Mod
	SetBgColor( pScheme->GetColor( "SpawnMenuBackground", Color( 0, 0, 0, 180 ) ) );
	SetFgColor( pScheme->GetColor( "SpawnMenuTitle", Color( 255, 255, 255, 255 ) ) );
}

//-----------------------------------------------------------------------------
// Perform layout
//-----------------------------------------------------------------------------
void CClientSpawnDialog::PerformLayout()
{
	BaseClass::PerformLayout();

	// Update panel positions based on current size
	if ( m_pMainPanel )
	{
		m_pMainPanel->SetSize( GetWide(), GetTall() );
	}
}

//-----------------------------------------------------------------------------
// Handle key input
//-----------------------------------------------------------------------------
void CClientSpawnDialog::OnKeyCodePressed( vgui::KeyCode code )
{
	switch ( code )
	{
		case vgui::KEY_ESCAPE:
			ShowPanel( false );
			break;

		default:
			BaseClass::OnKeyCodePressed( code );
			break;
	}
}

//-----------------------------------------------------------------------------
// Handle commands
//-----------------------------------------------------------------------------
void CClientSpawnDialog::OnCommand( const char *command )
{
	if ( Q_stricmp( command, "ContextMinimize" ) == 0 )
	{
		ShowPanel( false );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

//=============================================================================
// CToolButtonsPanel Implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CToolButtonsPanel::CToolButtonsPanel( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_nCurrentToolMode = TOOL_NONE;

	// Initialize tool button arrays
	for ( int i = 0; i < ARRAYSIZE( m_pToolButtons ); i++ )
	{
		m_pToolButtons[i] = NULL;
		m_bButtonStates[i] = false;
	}

	LoadToolButtons();
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CToolButtonsPanel::~CToolButtonsPanel()
{
	ClearToolButtons();
}

//-----------------------------------------------------------------------------
// Apply scheme settings
//-----------------------------------------------------------------------------
void CToolButtonsPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// Set background color for tool buttons panel
	SetBgColor( pScheme->GetColor( "ToolButtonsBackground", Color( 50, 50, 50, 200 ) ) );
}

//-----------------------------------------------------------------------------
// Perform layout
//-----------------------------------------------------------------------------
void CToolButtonsPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int y = 4;
	const int x = 4;
	const int width = GetWide() - 8;
	const int spacing = 1;
	const int buttonWidth = ( width - spacing ) / 2;
	int currentX = x;
	int rowHeight = 0;

	for ( int i = 0; i < m_ToolButtons.Count(); i++ )
	{
		if ( !m_ToolButtons[i].pPanel )
			continue;

		if ( m_ToolButtons[i].bIsLabel || m_ToolButtons[i].bDoubleHeight )
		{
			if ( currentX != x )
			{
				y += rowHeight + spacing;
				currentX = x;
				rowHeight = 0;
			}

			const int height = m_ToolButtons[i].bIsLabel ? 17 : 34;
			m_ToolButtons[i].pPanel->SetBounds( x, y, width, height );
			y += height + spacing;
			continue;
		}

		const int height = 16;
		m_ToolButtons[i].pPanel->SetBounds( currentX, y, buttonWidth, height );

		if ( currentX == x )
		{
			currentX = x + buttonWidth + spacing;
			rowHeight = height;
		}
		else
		{
			y += height + spacing;
			currentX = x;
			rowHeight = 0;
		}
	}

	if ( currentX != x )
	{
		y += rowHeight + spacing;
	}
}

//-----------------------------------------------------------------------------
// Handle commands
//-----------------------------------------------------------------------------
void CToolButtonsPanel::OnCommand( const char *command )
{
	if ( Q_strnicmp( command, "menu_entry_", 11 ) == 0 )
	{
		int entryIndex = atoi( command + 11 );
		if ( entryIndex >= 0 && entryIndex < m_ToolButtons.Count() )
		{
			ToolButton_t &entry = m_ToolButtons[entryIndex];
			if ( entry.szCommand[0] )
			{
				if ( entry.toolID >= 0 )
				{
					SetToolMode( entry.toolID );
				}

				char fullCommand[512];
				Q_snprintf( fullCommand, sizeof(fullCommand), "%s\n", entry.szCommand );
				engine->ClientCmd( fullCommand );
			}
		}
	}
	else if ( Q_strnicmp( command, "tool_", 5 ) == 0 )
	{
		int toolID = atoi( command + 5 );
		SetToolMode( toolID );

		char toolCommand[64];
		Q_snprintf( toolCommand, sizeof(toolCommand), "gm_toolmode %d\n", toolID );
		engine->ClientCmd( toolCommand );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

static int BM_ExtractToolModeFromCommand( const char *command )
{
	const char *toolMode = Q_strstr( command, "gm_toolmode" );
	if ( !toolMode )
		return -1;

	toolMode += Q_strlen( "gm_toolmode" );
	while ( *toolMode == ' ' || *toolMode == '\t' )
	{
		toolMode++;
	}

	return atoi( toolMode );
}

void CToolButtonsPanel::ClearToolButtons()
{
	for ( int i = 0; i < m_ToolButtons.Count(); i++ )
	{
		if ( m_ToolButtons[i].pPanel )
		{
			m_ToolButtons[i].pPanel->MarkForDeletion();
		}
	}

	m_ToolButtons.RemoveAll();
	for ( int i = 0; i < ARRAYSIZE( m_pToolButtons ); i++ )
	{
		m_pToolButtons[i] = NULL;
		m_bButtonStates[i] = false;
	}
	m_nCurrentToolMode = TOOL_NONE;
}

void CToolButtonsPanel::CreateMenuLabel( const char *labelText )
{
	ToolButton_t newEntry;
	memset( &newEntry, 0, sizeof(newEntry) );
	newEntry.toolID = -1;
	newEntry.bIsLabel = true;
	Q_strncpy( newEntry.szName, labelText ? labelText : "", sizeof(newEntry.szName) );

	char labelName[64];
	Q_snprintf( labelName, sizeof(labelName), "BuildMenuLabel%d", m_ToolButtons.Count() );

	vgui::Label *pLabel = new vgui::Label( this, labelName, newEntry.szName );
	pLabel->SetContentAlignment( vgui::Label::a_west );
	pLabel->SetTextInset( 2, 0 );
	pLabel->SetFgColor( Color( 230, 230, 170, 255 ) );
	pLabel->SetBgColor( Color( 0, 0, 0, 0 ) );
	pLabel->SetPaintBackgroundEnabled( false );
	pLabel->SetMouseInputEnabled( false );
	pLabel->SetScheme( GetScheme() );

	newEntry.pPanel = pLabel;
	m_ToolButtons.AddToTail( newEntry );
}

void CToolButtonsPanel::CreateMenuButton( const char *buttonText, const char *command, bool bDoubleHeight )
{
	ToolButton_t newEntry;
	memset( &newEntry, 0, sizeof(newEntry) );
	newEntry.toolID = BM_ExtractToolModeFromCommand( command ? command : "" );
	newEntry.bIsLabel = false;
	newEntry.bDoubleHeight = bDoubleHeight;
	Q_strncpy( newEntry.szName, buttonText ? buttonText : "", sizeof(newEntry.szName) );
	Q_strncpy( newEntry.szCommand, command ? command : "", sizeof(newEntry.szCommand) );
	Q_strncpy( newEntry.szDescription, newEntry.szCommand, sizeof(newEntry.szDescription) );

	char buttonName[64];
	Q_snprintf( buttonName, sizeof(buttonName), "BuildMenuButton%d", m_ToolButtons.Count() );

	char menuCommand[64];
	Q_snprintf( menuCommand, sizeof(menuCommand), "menu_entry_%d", m_ToolButtons.Count() );

	newEntry.pButton = new vgui::Button( this, buttonName, newEntry.szName );
	newEntry.pPanel = newEntry.pButton;
	newEntry.pButton->SetCommand( menuCommand );
	newEntry.pButton->SetTooltip( newEntry.szCommand );
	newEntry.pButton->SetContentAlignment( vgui::Label::a_west );
	newEntry.pButton->SetTextInset( 4, 0 );
	newEntry.pButton->SetVisible( true );
	newEntry.pButton->SetMouseInputEnabled( true );
	newEntry.pButton->SetScheme( GetScheme() );

	m_ToolButtons.AddToTail( newEntry );

	if ( newEntry.toolID >= 0 && newEntry.toolID < ARRAYSIZE( m_pToolButtons ) )
	{
		m_pToolButtons[newEntry.toolID] = newEntry.pButton;
	}
}

void CToolButtonsPanel::LoadToolButtonsFromFile( const char *fileName )
{
	KeyValues *pConfig = new KeyValues( "GModMenu" );
	if ( !pConfig->LoadFromFile( filesystem, fileName, "MOD" ) )
	{
		pConfig->deleteThis();
		return;
	}

	KeyValues *pClear = pConfig->FindKey( "clear" );
	if ( pClear && pClear->GetInt() != 0 )
	{
		ClearToolButtons();
	}

	KeyValues *pMenu = pConfig->FindKey( "menu" );
	if ( pMenu )
	{
		for ( KeyValues *pEntry = pMenu->GetFirstSubKey(); pEntry; pEntry = pEntry->GetNextKey() )
		{
			const char *rawName = pEntry->GetName();
			const char *entryCommand = pEntry->GetString();
			if ( !rawName || !rawName[0] )
				continue;

			bool bDoubleHeight = false;
			if ( rawName[0] == '~' || rawName[0] == '@' )
			{
				CreateMenuLabel( rawName + 1 );
				continue;
			}
			else if ( rawName[0] == '#' )
			{
				bDoubleHeight = true;
				rawName++;
			}

			CreateMenuButton( rawName, entryCommand, bDoubleHeight );
		}
	}

	pConfig->deleteThis();
}

bool CToolButtonsPanel::HasLoadedMenuFile( CUtlVector<LoadedMenuFile_t> &loadedFiles, const char *fileName )
{
	for ( int i = 0; i < loadedFiles.Count(); i++ )
	{
		if ( Q_stricmp( loadedFiles[i].szName, fileName ) == 0 )
			return true;
	}

	return false;
}

void CToolButtonsPanel::MarkLoadedMenuFile( CUtlVector<LoadedMenuFile_t> &loadedFiles, const char *fileName )
{
	LoadedMenuFile_t loadedFile;
	Q_strncpy( loadedFile.szName, fileName ? fileName : "", sizeof( loadedFile.szName ) );
	loadedFiles.AddToTail( loadedFile );
}

void CToolButtonsPanel::LoadToolButtonsFromDirectory( const char *directory, bool bSkipDefault, CUtlVector<LoadedMenuFile_t> &loadedFiles )
{
	char searchPath[256];
	Q_snprintf( searchPath, sizeof( searchPath ), "%s/*.txt", directory );

	FileFindHandle_t findHandle = FILESYSTEM_INVALID_FIND_HANDLE;
	const char *pFilename = filesystem->FindFirst( searchPath, &findHandle );
	while ( pFilename )
	{
		if ( !BM_IsSpecialFindName( pFilename ) )
		{
			bool bIsDefault = ( Q_stricmp( pFilename, "default.txt" ) == 0 );
			if ( !( bSkipDefault && bIsDefault ) && !HasLoadedMenuFile( loadedFiles, pFilename ) )
			{
				char fullPath[256];
				Q_snprintf( fullPath, sizeof( fullPath ), "%s/%s", directory, pFilename );
				LoadToolButtonsFromFile( fullPath );
				MarkLoadedMenuFile( loadedFiles, pFilename );
			}
		}

		pFilename = filesystem->FindNext( findHandle );
	}

	if ( findHandle != FILESYSTEM_INVALID_FIND_HANDLE )
	{
		filesystem->FindClose( findHandle );
	}
}

//-----------------------------------------------------------------------------
// Load tool buttons - matches Garry's Mod tool system
//-----------------------------------------------------------------------------
void CToolButtonsPanel::LoadToolButtons()
{
	ClearToolButtons();

	CUtlVector<LoadedMenuFile_t> loadedFiles;
	LoadToolButtonsFromFile( DEFAULT_CONFIG_FILE );
	MarkLoadedMenuFile( loadedFiles, "default.txt" );
	LoadToolButtonsFromDirectory( "settings/menu_main", true, loadedFiles );
	LoadToolButtonsFromDirectory( "mods/-modcache/settings/menu_main", true, loadedFiles );

	if ( m_ToolButtons.Count() == 0 )
	{
		CreateToolButton( 0, "Rope", "gm_toolmode 0; gm_context rope;" );
		CreateToolButton( 1, "Elastic", "gm_toolmode 1; gm_context spring;" );
		CreateToolButton( 2, "Weld", "gm_toolmode 2; gm_context weld;" );
		CreateToolButton( 3, "Ballsocket", "gm_toolmode 3; gm_context ballsocket;" );
		CreateToolButton( 4, "Pulley", "gm_toolmode 4; gm_context pulley;" );
	}

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Create a tool button
//-----------------------------------------------------------------------------
void CToolButtonsPanel::CreateToolButton( int toolID, const char *toolName, const char *description )
{
	char toolCommand[256];
	Q_snprintf( toolCommand, sizeof(toolCommand), "gm_toolmode %d", toolID );
	CreateMenuButton( toolName, toolCommand, false );

	if ( m_ToolButtons.Count() > 0 )
	{
		ToolButton_t &entry = m_ToolButtons[m_ToolButtons.Count() - 1];
		entry.toolID = toolID;
		Q_strncpy( entry.szDescription, description ? description : "", sizeof(entry.szDescription) );
		if ( toolID >= 0 && toolID < ARRAYSIZE( m_pToolButtons ) )
		{
			m_pToolButtons[toolID] = entry.pButton;
		}
	}
}

//-----------------------------------------------------------------------------
// Set current tool mode
//-----------------------------------------------------------------------------
void CToolButtonsPanel::SetToolMode( int toolMode )
{
	if ( toolMode == m_nCurrentToolMode )
		return;

	// Deselect previous tool button
	if ( m_nCurrentToolMode >= 0 && m_nCurrentToolMode < ARRAYSIZE( m_pToolButtons ) && m_pToolButtons[m_nCurrentToolMode] )
	{
		m_pToolButtons[m_nCurrentToolMode]->SetSelected( false );
		m_bButtonStates[m_nCurrentToolMode] = false;
	}

	// Select new tool button
	m_nCurrentToolMode = toolMode;
	if ( toolMode >= 0 && toolMode < ARRAYSIZE( m_pToolButtons ) && m_pToolButtons[toolMode] )
	{
		m_pToolButtons[toolMode]->SetSelected( true );
		m_bButtonStates[toolMode] = true;
	}

	Msg( "Tool mode changed to: %d\n", toolMode );
}

//=============================================================================
// CContextPanel Implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CContextPanel::CContextPanel( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_szCurrentContext[0] = '\0';
	m_bContextVisible = false;
	m_pContextContent = NULL;
	m_pPropPanel = NULL;

	// Create the prop panel as the main content
	// Note: CPropPanel loads its own scheme in its constructor
	m_pPropPanel = new CPropPanel( this, "PropPanel" );
	m_pPropPanel->SetVisible( true );
	m_pPropPanel->SetMouseInputEnabled( true );

	LoadContextConfiguration();
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CContextPanel::~CContextPanel()
{
	if ( m_pContextContent )
	{
		m_pContextContent->MarkForDeletion();
	}
	if ( m_pPropPanel )
	{
		m_pPropPanel->MarkForDeletion();
	}
}

//-----------------------------------------------------------------------------
// Apply scheme settings
//-----------------------------------------------------------------------------
void CContextPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// Set context panel colors
	SetBgColor( pScheme->GetColor( "ContextPanelBG", Color( 40, 40, 40, 200 ) ) );
}

//-----------------------------------------------------------------------------
// Perform layout
//-----------------------------------------------------------------------------
void CContextPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( m_pContextContent )
	{
		m_pContextContent->SetBounds( 5, 5, GetWide() - 10, GetTall() - 10 );
	}

	// Size the prop panel to fill the context area
	if ( m_pPropPanel )
	{
		m_pPropPanel->SetBounds( 5, 5, GetWide() - 10, GetTall() - 10 );
	}
}

//-----------------------------------------------------------------------------
// Mouse wheel scrolling - forward to prop panel
//-----------------------------------------------------------------------------
void CContextPanel::OnMouseWheeled( int delta )
{
	if ( m_pPropPanel )
	{
		m_pPropPanel->OnMouseWheeled( delta );
	}
}

//-----------------------------------------------------------------------------
// Reload props
//-----------------------------------------------------------------------------
void CContextPanel::ReloadProps()
{
	if ( m_pPropPanel )
	{
		m_pPropPanel->ReloadProps();
	}
}

//-----------------------------------------------------------------------------
// Handle commands
//-----------------------------------------------------------------------------
void CContextPanel::OnCommand( const char *command )
{
	if ( Q_strnicmp( command, "context_", 8 ) == 0 )
	{
		const char *contextType = command + 8;
		ShowContext( contextType );
	}
	else if ( command && command[0] )
	{
		char fullCommand[512];
		Q_snprintf( fullCommand, sizeof(fullCommand), "%s\n", command );
		engine->ClientCmd( fullCommand );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CContextPanel::ClearContextContent()
{
	if ( m_pContextContent )
	{
		m_pContextContent->MarkForDeletion();
		m_pContextContent = NULL;
	}
}

static void BM_FlushContextRow( int &x, int &y, int &rowTall )
{
	if ( x != 0 )
	{
		y += rowTall + 4;
		x = 0;
		rowTall = 0;
	}
}

void CContextPanel::AddContextLabel( const char *text, int &x, int &y, int &rowTall, int tall )
{
	if ( !m_pContextContent )
		return;

	BM_FlushContextRow( x, y, rowTall );

	char panelName[64];
	Q_snprintf( panelName, sizeof(panelName), "ContextLabel%d", y );

	vgui::Label *pLabel = new vgui::Label( m_pContextContent, panelName, text ? text : "" );
	pLabel->SetContentAlignment( vgui::Label::a_west );
	pLabel->SetTextInset( 4, 0 );
	pLabel->SetFgColor( Color( 235, 235, 205, 255 ) );
	pLabel->SetBgColor( Color( 0, 0, 0, 0 ) );
	pLabel->SetPaintBackgroundEnabled( false );
	pLabel->SetBounds( 4, y, max( GetWide() - 28, 100 ), tall );
	pLabel->SetMouseInputEnabled( false );
	y += tall + 3;
}

void CContextPanel::AddContextButton( const char *text, const char *command, int &x, int &y, int &rowTall, int columns )
{
	if ( !m_pContextContent || !text || !text[0] )
		return;

	if ( columns < 1 )
		columns = 1;
	if ( x >= columns )
		BM_FlushContextRow( x, y, rowTall );

	const int spacing = 4;
	const int contentWide = max( GetWide() - 28, 160 );
	const int buttonWide = max( ( contentWide - spacing * ( columns - 1 ) ) / columns, 48 );
	const int buttonTall = 20;
	const int buttonX = 4 + x * ( buttonWide + spacing );

	char panelName[64];
	Q_snprintf( panelName, sizeof(panelName), "ContextButton%d_%d", y, x );

	vgui::Button *pButton = new vgui::Button( m_pContextContent, panelName, text );
	pButton->SetCommand( command ? command : "" );
	pButton->AddActionSignalTarget( this );
	pButton->SetContentAlignment( vgui::Label::a_west );
	pButton->SetTextInset( 4, 0 );
	pButton->SetBounds( buttonX, y, buttonWide, buttonTall );
	pButton->SetVisible( true );
	pButton->SetMouseInputEnabled( true );
	pButton->SetScheme( GetScheme() );

	x++;
	if ( buttonTall > rowTall )
		rowTall = buttonTall;
	if ( x >= columns )
		BM_FlushContextRow( x, y, rowTall );
}

void CContextPanel::AddContextKeyValueButtons( KeyValues *pControl, int &x, int &y, int &rowTall, int columns )
{
	if ( !pControl )
		return;

	const char *label = pControl->GetString( "label", "" );
	if ( label[0] )
	{
		AddContextLabel( label, x, y, rowTall, 16 );
	}

	const char *kvFile = pControl->GetString( "kvfile", "" );
	if ( !kvFile[0] )
		return;

	const char *settingName = pControl->GetString( "name", "" );
	KeyValues *pItems = new KeyValues( "ContextItems" );
	if ( pItems->LoadFromFile( filesystem, kvFile, "MOD" ) )
	{
		for ( KeyValues *pItem = pItems->GetFirstSubKey(); pItem; pItem = pItem->GetNextKey() )
		{
			char command[512];
			command[0] = '\0';

			if ( pItem->GetFirstSubKey() )
			{
				for ( KeyValues *pSub = pItem->GetFirstSubKey(); pSub; pSub = pSub->GetNextKey() )
				{
					char oneCommand[256];
					Q_snprintf( oneCommand, sizeof(oneCommand), "%s %s; ", pSub->GetName(), pSub->GetString() );
					Q_strncat( command, oneCommand, sizeof(command) );
				}
			}
			else if ( settingName[0] )
			{
				Q_snprintf( command, sizeof(command), "%s \"%s\"", settingName, pItem->GetString() );
			}

			AddContextButton( pItem->GetName(), command, x, y, rowTall, columns );
		}
	}
	pItems->deleteThis();
}

bool CContextPanel::LoadContextPanelFile( const char *fileName, const char *contextType )
{
	KeyValues *pConfig = new KeyValues( "ContextPanel" );
	if ( !pConfig->LoadFromFile( filesystem, fileName, "MOD" ) )
	{
		pConfig->deleteThis();
		return false;
	}

	if ( Q_stricmp( pConfig->GetName(), contextType ) != 0 )
	{
		pConfig->deleteThis();
		return false;
	}

	int x = 0;
	int y = 4;
	int rowTall = 0;

	const char *title = pConfig->GetString( "title", "" );
	if ( title[0] )
	{
		AddContextLabel( title, x, y, rowTall, 22 );
	}

	KeyValues *pControls = pConfig->FindKey( "controls" );
	int columns = pControls ? pControls->GetInt( "columns", 1 ) : 1;
	if ( columns < 1 )
		columns = 1;
	if ( columns > 4 )
		columns = 4;

	if ( pControls )
	{
		for ( KeyValues *pControl = pControls->GetFirstSubKey(); pControl; pControl = pControl->GetNextKey() )
		{
			const char *controlType = pControl->GetName();
			if ( Q_stricmp( controlType, "columns" ) == 0 )
				continue;

			if ( Q_stricmp( controlType, "button" ) == 0 )
			{
				AddContextButton( pControl->GetString( "label", "" ), pControl->GetString( "command", "" ), x, y, rowTall, columns );
			}
			else if ( Q_stricmp( controlType, "keyvaluecombobox" ) == 0 )
			{
				AddContextKeyValueButtons( pControl, x, y, rowTall, columns );
			}
			else if ( Q_stricmp( controlType, "smalltext" ) == 0 || Q_stricmp( controlType, "text" ) == 0 )
			{
				AddContextLabel( pControl->GetString( "label", "" ), x, y, rowTall, pControl->GetInt( "tall", 20 ) );
			}
			else if ( Q_stricmp( controlType, "toggle" ) == 0 || Q_stricmp( controlType, "Toggle" ) == 0 )
			{
				const char *name = pControl->GetString( "name", "" );
				char command[256];
				Q_snprintf( command, sizeof(command), "toggle %s", name );
				AddContextButton( pControl->GetString( "label", name ), command, x, y, rowTall, columns );
			}
			else if ( Q_stricmp( controlType, "slider" ) == 0 )
			{
				AddContextLabel( pControl->GetString( "label", pControl->GetString( "name", "Slider" ) ), x, y, rowTall, 16 );
			}
		}
	}

	BM_FlushContextRow( x, y, rowTall );
	pConfig->deleteThis();
	return true;
}

bool CContextPanel::LoadContextPanel( const char *contextType )
{
	if ( !contextType || !contextType[0] )
		return false;

	char preferredPath[256];
	Q_snprintf( preferredPath, sizeof(preferredPath), "settings/context_panels/%s.txt", contextType );
	if ( filesystem->FileExists( preferredPath, "MOD" ) && LoadContextPanelFile( preferredPath, contextType ) )
		return true;

	Q_snprintf( preferredPath, sizeof(preferredPath), "mods/-modcache/settings/context_panels/%s.txt", contextType );
	if ( filesystem->FileExists( preferredPath, "MOD" ) && LoadContextPanelFile( preferredPath, contextType ) )
		return true;

	if ( LoadContextPanelFromDirectory( "settings/context_panels", contextType ) )
		return true;

	if ( LoadContextPanelFromDirectory( "mods/-modcache/settings/context_panels", contextType ) )
		return true;

	return false;
}

bool CContextPanel::LoadContextPanelFromDirectory( const char *directory, const char *contextType )
{
	char searchPath[256];
	Q_snprintf( searchPath, sizeof( searchPath ), "%s/*.txt", directory );

	FileFindHandle_t findHandle = FILESYSTEM_INVALID_FIND_HANDLE;
	const char *pFilename = filesystem->FindFirst( searchPath, &findHandle );
	while ( pFilename )
	{
		if ( !BM_IsSpecialFindName( pFilename ) )
		{
			char fullPath[256];
			Q_snprintf( fullPath, sizeof( fullPath ), "%s/%s", directory, pFilename );
			if ( LoadContextPanelFile( fullPath, contextType ) )
			{
				filesystem->FindClose( findHandle );
				return true;
			}
		}

		pFilename = filesystem->FindNext( findHandle );
	}

	if ( findHandle != FILESYSTEM_INVALID_FIND_HANDLE )
	{
		filesystem->FindClose( findHandle );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Show context menu
//-----------------------------------------------------------------------------
void CContextPanel::ShowContext( const char *contextType )
{
	Q_strncpy( m_szCurrentContext, contextType ? contextType : "", sizeof(m_szCurrentContext) );
	m_bContextVisible = true;

	if ( !contextType || !contextType[0] || Q_stricmp( contextType, "props" ) == 0 || Q_stricmp( contextType, "spawnmenu" ) == 0 )
	{
		ClearContextContent();
		if ( m_pPropPanel )
			m_pPropPanel->SetVisible( true );
		return;
	}

	if ( m_pPropPanel )
		m_pPropPanel->SetVisible( false );

	ClearContextContent();
	m_pContextContent = new vgui::Panel( this, "ContextContent" );
	m_pContextContent->SetVisible( true );
	m_pContextContent->SetMouseInputEnabled( true );
	m_pContextContent->SetBounds( 5, 5, GetWide() - 10, GetTall() - 10 );
	m_pContextContent->SetScheme( GetScheme() );

	if ( !LoadContextPanel( contextType ) )
	{
		int x = 0;
		int y = 4;
		int rowTall = 0;
		char message[128];
		Q_snprintf( message, sizeof(message), "No settings for %s", contextType );
		AddContextLabel( message, x, y, rowTall, 20 );
	}

	Msg( "Showing context: %s\n", contextType );
}

//-----------------------------------------------------------------------------
// Hide context menu
//-----------------------------------------------------------------------------
void CContextPanel::HideContext()
{
	m_bContextVisible = false;
	m_szCurrentContext[0] = '\0';

	if ( m_pContextContent )
	{
		m_pContextContent->SetVisible( false );
	}
}

//-----------------------------------------------------------------------------
// Load context configuration
//-----------------------------------------------------------------------------
void CContextPanel::LoadContextConfiguration()
{
	// Load context panel configurations from settings/context_panels/
	FileFindHandle_t findHandle;
	const char *pFilename = filesystem->FindFirst( "settings/context_panels/*.txt", &findHandle );

	while ( pFilename )
	{
		char fullPath[256];
		Q_snprintf( fullPath, sizeof(fullPath), "settings/context_panels/%s", pFilename );

		KeyValues *pConfig = new KeyValues( "ContextPanel" );
		if ( filesystem->FileExists( fullPath, "MOD" ) )
		{
			if ( pConfig->LoadFromFile( filesystem, fullPath, "MOD" ) )
			{
				// Process context panel configuration
				Msg( "Loaded context panel config: %s\n", pFilename );
			}
		}
		pConfig->deleteThis();

		pFilename = filesystem->FindNext( findHandle );
	}
	filesystem->FindClose( findHandle );
}
