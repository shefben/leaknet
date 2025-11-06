//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Spawn Menu - Identical to Garry's Mod spawn menu system
//
//=============================================================================

#include "cbase.h"
#include "bmod_spawnmenu.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/EditablePanel.h>
#include "filesystem.h"
#include "KeyValues.h"
#include "convar.h"
#include "iclientmode.h"
#include "ienginevgui.h"
#include "tier0/icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

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
	m_pToolButtonsPanel->SetMouseInputEnabled( false );
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
	m_pContextPanel->SetMouseInputEnabled( false );
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

	// Load tool buttons resource file
	LoadControlSettingsFromFile( TOOLBUTTONS_RES_FILE );
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
			m_bAutoUpdate = pSubKey->GetBool();
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
		case KEY_ESCAPE:
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
	for ( int i = 0; i < 20; i++ )
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
	// Cleanup tool buttons
	for ( int i = 0; i < m_ToolButtons.Count(); i++ )
	{
		if ( m_ToolButtons[i].pButton )
		{
			m_ToolButtons[i].pButton->MarkForDeletion();
		}
	}
	m_ToolButtons.RemoveAll();
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

	// Layout tool buttons in a grid
	int buttonWidth = 32;
	int buttonHeight = 32;
	int spacing = 4;
	int buttonsPerRow = (GetWide() - spacing) / (buttonWidth + spacing);

	for ( int i = 0; i < m_ToolButtons.Count(); i++ )
	{
		if ( m_ToolButtons[i].pButton )
		{
			int row = i / buttonsPerRow;
			int col = i % buttonsPerRow;

			int x = spacing + col * (buttonWidth + spacing);
			int y = spacing + row * (buttonHeight + spacing);

			m_ToolButtons[i].pButton->SetBounds( x, y, buttonWidth, buttonHeight );
		}
	}
}

//-----------------------------------------------------------------------------
// Handle commands
//-----------------------------------------------------------------------------
void CToolButtonsPanel::OnCommand( const char *command )
{
	// Check if command is a tool selection
	if ( Q_strnicmp( command, "tool_", 5 ) == 0 )
	{
		int toolID = atoi( command + 5 );
		SetToolMode( toolID );

		// Send tool mode command to server
		char toolCommand[64];
		Q_snprintf( toolCommand, sizeof(toolCommand), "gm_toolmode %d", toolID );
		engine->ClientCmd( toolCommand );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

//-----------------------------------------------------------------------------
// Load tool buttons - matches Garry's Mod tool system
//-----------------------------------------------------------------------------
void CToolButtonsPanel::LoadToolButtons()
{
	// Clear existing buttons
	for ( int i = 0; i < m_ToolButtons.Count(); i++ )
	{
		if ( m_ToolButtons[i].pButton )
		{
			m_ToolButtons[i].pButton->MarkForDeletion();
		}
	}
	m_ToolButtons.RemoveAll();

	// Create default tool buttons matching Garry's Mod
	CreateToolButton( TOOL_GUN, "Gun", "Gun tool for shooting" );
	CreateToolButton( TOOL_PHYSGUN, "Physgun", "Physics manipulation tool" );
	CreateToolButton( TOOL_CAMERA, "Camera", "Camera tool for screenshots" );
	CreateToolButton( TOOL_NPC, "NPC", "NPC spawning tool" );
	CreateToolButton( TOOL_NEXTBOT, "NextBot", "NextBot spawning tool" );
	CreateToolButton( TOOL_MATERIAL, "Material", "Material application tool" );
	CreateToolButton( TOOL_COLOR, "Color", "Color modification tool" );
	CreateToolButton( TOOL_PAINT, "Paint", "Paint tool" );
	CreateToolButton( TOOL_INFLATOR, "Inflator", "Size modification tool" );
	CreateToolButton( TOOL_DUPLICATOR, "Duplicator", "Entity duplication tool" );
	CreateToolButton( TOOL_CONSTRAINER, "Constrainer", "Constraint creation tool" );
	CreateToolButton( TOOL_AXIS, "Axis", "Axis constraint tool" );
	CreateToolButton( TOOL_BALLSOCKET, "BallSocket", "Ball socket constraint tool" );
	CreateToolButton( TOOL_ROPE, "Rope", "Rope constraint tool" );
	CreateToolButton( TOOL_PULLEY, "Pulley", "Pulley constraint tool" );

	// Set default tool mode
	SetToolMode( TOOL_PHYSGUN );
}

//-----------------------------------------------------------------------------
// Create a tool button
//-----------------------------------------------------------------------------
void CToolButtonsPanel::CreateToolButton( int toolID, const char *toolName, const char *description )
{
	ToolButton_t newButton;
	newButton.toolID = toolID;
	Q_strncpy( newButton.szName, toolName, sizeof(newButton.szName) );
	Q_strncpy( newButton.szDescription, description, sizeof(newButton.szDescription) );

	// Create the button
	char buttonName[64];
	Q_snprintf( buttonName, sizeof(buttonName), "ToolButton%d", toolID );

	char command[64];
	Q_snprintf( command, sizeof(command), "tool_%d", toolID );

	newButton.pButton = new vgui::Button( this, buttonName, toolName );
	newButton.pButton->SetCommand( command );
	newButton.pButton->SetVisible( true );

	// Set tooltip
	newButton.pButton->SetTooltip( description );

	m_ToolButtons.AddToTail( newButton );

	// Store in array for quick access
	if ( toolID < 20 )
	{
		m_pToolButtons[toolID] = newButton.pButton;
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
	if ( m_nCurrentToolMode < 20 && m_pToolButtons[m_nCurrentToolMode] )
	{
		m_pToolButtons[m_nCurrentToolMode]->SetSelected( false );
		m_bButtonStates[m_nCurrentToolMode] = false;
	}

	// Select new tool button
	m_nCurrentToolMode = toolMode;
	if ( toolMode < 20 && m_pToolButtons[toolMode] )
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
	else
	{
		BaseClass::OnCommand( command );
	}
}

//-----------------------------------------------------------------------------
// Show context menu
//-----------------------------------------------------------------------------
void CContextPanel::ShowContext( const char *contextType )
{
	Q_strncpy( m_szCurrentContext, contextType, sizeof(m_szCurrentContext) );
	m_bContextVisible = true;

	// Create context content panel if needed
	if ( !m_pContextContent )
	{
		m_pContextContent = new vgui::Panel( this, "ContextContent" );
		m_pContextContent->SetVisible( true );
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