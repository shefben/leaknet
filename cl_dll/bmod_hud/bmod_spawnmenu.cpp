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
#include <vgui_controls/Slider.h>
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
ConVar gmod_snapangles("gmod_snapangles", "45", FCVAR_ARCHIVE, "Snap angles for rotation in spawn menu");

//-----------------------------------------------------------------------------
// Global spawn menu instance
//-----------------------------------------------------------------------------
CClientSpawnDialog *g_pSpawnMenu = NULL;

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

	if ( g_pSpawnMenu )
	{
		g_pSpawnMenu->ShowPanel( true );
		g_pSpawnMenu->ShowToolContext( engine->Cmd_Argv( 1 ) );
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
	m_bSecluded = false;
	memset( m_nRestoreBounds, 0, sizeof(m_nRestoreBounds) );
	m_szLastContext[0] = '\0';

	// Original GMod 9 build-menu layout, recovered from client.dll.  The
	// resource file can override every value before the dialog is created.
	m_nLayoutDialogWide = 790;
	m_nLayoutPanelTop = 10;
	m_nLayoutPanelBottom = 10;
	m_nLayoutPropX = 10;
	m_nLayoutPropWide = 238;
	m_nLayoutToolX = 270;
	m_nLayoutToolWide = 510;
	m_nLayoutContextX = 545;
	m_nLayoutContextWide = 235;
	m_nLayoutContextBottom = 15;
	m_nLayoutColumnGap = 8;
	LoadBuildMenuResource();

	// Load the scheme
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile( SPAWNMENU_SCHEME_FILE, SPAWNMENU_SCHEME_NAME );
	SetScheme( scheme );

	// Set global instance
	g_pSpawnMenu = this;

	// Get screen dimensions for sizing
	int screenWide, screenTall;
	vgui::surface()->GetScreenSize( screenWide, screenTall );

	// Set initial size - real GMod9's build menu splits roughly evenly
	// between the spawn (left) and tools (right) halves; the old 790px width
	// with a 238px tool column was much narrower than the reference layout.
	m_nConsoleHeight = screenTall - 100;
	if ( m_nConsoleHeight > 3000 )
		m_nConsoleHeight = 3000;
	if ( m_nConsoleHeight < 550 )
		m_nConsoleHeight = 550;

	const int nWide = m_nLayoutDialogWide;
	SetSize( nWide, m_nConsoleHeight );
	SetPos( (screenWide - nWide) / 2, (screenTall - m_nConsoleHeight) / 2 );

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

	// Create the always-visible spawnable-props panel
	CreatePropPanel();

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
	m_pMainPanel->SetSize( GetWide(), m_nConsoleHeight );
	m_pMainPanel->SetVisible( false );
	m_pMainPanel->SetMouseInputEnabled( true );
	m_pMainPanel->SetPos( 0, 0 );
}

//-----------------------------------------------------------------------------
// Create the spawnable-props panel (left side, always visible - real GMod9
// never hides this behind a tool's settings, see PerformLayout)
//-----------------------------------------------------------------------------
void CClientSpawnDialog::CreatePropPanel()
{
	m_pPropPanel = new CPropPanel( m_pMainPanel, "PropPanel" );
	m_pPropPanel->SetParent( m_pMainPanel );
	m_pPropPanel->SetVisible( true );
	m_pPropPanel->SetMouseInputEnabled( true );
	m_pPropPanel->SetScheme( GetScheme() );
}

//-----------------------------------------------------------------------------
// Create tool buttons panel
//-----------------------------------------------------------------------------
void CClientSpawnDialog::CreateToolButtonsPanel()
{
	m_pToolButtonsPanel = new CToolButtonsPanel( m_pMainPanel, "ToolButtonsPanel" );
	m_pToolButtonsPanel->SetParent( m_pMainPanel );
	m_pToolButtonsPanel->SetVisible( true );
	m_pToolButtonsPanel->SetMouseInputEnabled( true );

	// Propagate scheme to child panel
	m_pToolButtonsPanel->SetScheme( GetScheme() );
}

//-----------------------------------------------------------------------------
// Create context panel - a small floating "tool settings" box (like the
// reference screenshot's "Bloom Settings" box) anchored over the bottom-right
// corner of the tool buttons panel. Hidden until a tool's gm_context is shown.
//-----------------------------------------------------------------------------
void CClientSpawnDialog::CreateContextPanel()
{
	m_pContextPanel = new CContextPanel( m_pMainPanel, "ContextPanel" );
	m_pContextPanel->SetParent( m_pMainPanel );
	m_pContextPanel->SetVisible( false );
	m_pContextPanel->SetMouseInputEnabled( true );

	// Propagate scheme to child panel
	m_pContextPanel->SetScheme( GetScheme() );
}

//-----------------------------------------------------------------------------
// Create minimize button - sits at the top-right corner of the floating
// context box and hides just that box (not the whole spawn menu).
//-----------------------------------------------------------------------------
void CClientSpawnDialog::CreateMinimizeButton()
{
	m_pMinimizeButton = new vgui::Button( m_pMainPanel, "ContextMinimize", "Seclude" );
	m_pMinimizeButton->SetCommand( "ContextMinimize" );
	m_pMinimizeButton->AddActionSignalTarget( this );
	m_pMinimizeButton->SetVisible( false );
}

//-----------------------------------------------------------------------------
// Load menu configuration from files
//-----------------------------------------------------------------------------
void CClientSpawnDialog::LoadMenuConfiguration()
{
	LoadBuildMenuResource();
	LoadGModMenuConfiguration();
}

//-----------------------------------------------------------------------------
// Load the build-menu geometry from the same VGUI resource used by GMod 9.
// The resource also contains the templates applied to dynamically-created
// labels and buttons by CToolButtonsPanel::ApplyResourceSettings.
//-----------------------------------------------------------------------------
void CClientSpawnDialog::LoadBuildMenuResource()
{
	KeyValues *pResource = new KeyValues( "BuildMenuResource" );
	if ( pResource->LoadFromFile( filesystem, TOOLBUTTONS_RES_FILE, "MOD" ) )
	{
		KeyValues *pLayout = pResource->FindKey( "BuildMenuLayout" );
		if ( pLayout )
		{
			m_nLayoutDialogWide = max( pLayout->GetInt( "dialog_wide", m_nLayoutDialogWide ), 1 );
			m_nLayoutPanelTop = max( pLayout->GetInt( "panel_top", m_nLayoutPanelTop ), 0 );
			m_nLayoutPanelBottom = max( pLayout->GetInt( "panel_bottom", m_nLayoutPanelBottom ), 0 );
			m_nLayoutPropX = max( pLayout->GetInt( "prop_x", m_nLayoutPropX ), 0 );
			m_nLayoutPropWide = max( pLayout->GetInt( "prop_wide", m_nLayoutPropWide ), 1 );
			m_nLayoutToolX = max( pLayout->GetInt( "tool_x", m_nLayoutToolX ), 0 );
			m_nLayoutToolWide = max( pLayout->GetInt( "tool_wide", m_nLayoutToolWide ), 1 );
			m_nLayoutContextX = max( pLayout->GetInt( "context_x", m_nLayoutContextX ), 0 );
			m_nLayoutContextWide = max( pLayout->GetInt( "context_wide", m_nLayoutContextWide ), 1 );
			m_nLayoutContextBottom = max( pLayout->GetInt( "context_bottom", m_nLayoutContextBottom ), 0 );
			m_nLayoutColumnGap = max( pLayout->GetInt( "column_gap", m_nLayoutColumnGap ), 0 );
		}
	}
	pResource->deleteThis();
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
	}
	if ( m_pPropPanel )
	{
		m_pPropPanel->ReloadProps();
	}
}

//-----------------------------------------------------------------------------
// Show/hide panel
//-----------------------------------------------------------------------------
void CClientSpawnDialog::ShowPanel( bool bShow )
{
	// Opening or closing the menu always leaves the secluded state - the whole
	// dialog is put back together first so both paths start from one layout.
	const bool bWasSecluded = m_bSecluded;
	if ( bWasSecluded )
	{
		SetSecluded( false );
	}

	if ( BaseClass::IsVisible() == bShow && !bWasSecluded )
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

		// Re-show the selected tool's settings box so the menu always opens the
		// way GMod 9 does: the floating context panel reflects the current tool
		// (and re-reads its convar values, picking up console-made changes).
		if ( m_szLastContext[0] )
			ShowToolContext( m_szLastContext );
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

	// Secluded: the frame has already been shrunk to the settings box, so the
	// box simply fills it and everything else stays hidden.
	if ( m_bSecluded )
	{
		if ( m_pContextPanel )
		{
			m_pContextPanel->SetBounds( 0, 0, GetWide(), GetTall() );
		}
		if ( m_pMinimizeButton )
		{
			int buttonW, buttonH;
			m_pMinimizeButton->GetSize( buttonW, buttonH );
			m_pMinimizeButton->SetPos( max( GetWide() - buttonW - 2, 0 ), 2 );
			m_pMinimizeButton->MoveToFront();
		}
		return;
	}

	const int contentTall = max( GetTall() - m_nLayoutPanelTop - m_nLayoutPanelBottom, 1 );
	const int propWide = min( m_nLayoutPropWide, max( GetWide() - m_nLayoutPropX, 1 ) );

	const bool bContextVisible = ( m_pContextPanel && m_pContextPanel->IsContextVisible() );
	const int boxX = min( m_nLayoutContextX, max( GetWide() - 1, 0 ) );
	const int boxWide = min( m_nLayoutContextWide, max( GetWide() - boxX, 1 ) );

	int toolWide = min( m_nLayoutToolWide, max( GetWide() - m_nLayoutToolX - m_nLayoutPanelTop, 1 ) );
	if ( bContextVisible )
	{
		// The settings box is a column of its own - clip the tool list so its
		// buttons reflow to the left of it instead of being covered by it.
		toolWide = min( toolWide, max( boxX - m_nLayoutToolX - m_nLayoutColumnGap, 1 ) );
	}

	if ( m_pPropPanel )
	{
		m_pPropPanel->SetBounds( m_nLayoutPropX, m_nLayoutPanelTop, propWide, contentTall );
	}
	if ( m_pToolButtonsPanel )
	{
		m_pToolButtonsPanel->SetBounds( m_nLayoutToolX, m_nLayoutPanelTop, toolWide, contentTall );
	}

	if ( m_pContextPanel )
	{
		const int boxTall = max( GetTall() - m_nLayoutPanelTop - m_nLayoutContextBottom, 1 );
		m_pContextPanel->SetBounds( boxX, m_nLayoutPanelTop, boxWide, boxTall );

		if ( m_pMinimizeButton )
		{
			int buttonW, buttonH;
			m_pMinimizeButton->GetSize( buttonW, buttonH );
			m_pMinimizeButton->SetPos( boxX + boxWide - buttonW - 2, m_nLayoutPanelTop + 2 );
		}
	}
}

//-----------------------------------------------------------------------------
// Seclude / restore - "Seclude" packs the build menu away and keeps only the
// current tool's settings box on screen; re-opening the menu (or pressing the
// button again, which now reads "Rejoin") brings the whole dialog back.
//-----------------------------------------------------------------------------
void CClientSpawnDialog::SetSecluded( bool bSecluded )
{
	if ( m_bSecluded == bSecluded )
		return;

	if ( bSecluded )
	{
		// Nothing to seclude if no tool settings are on screen.
		if ( !m_pContextPanel || !m_pContextPanel->IsContextVisible() )
			return;

		GetBounds( m_nRestoreBounds[0], m_nRestoreBounds[1], m_nRestoreBounds[2], m_nRestoreBounds[3] );

		int boxX, boxY, boxWide, boxTall;
		m_pContextPanel->GetBounds( boxX, boxY, boxWide, boxTall );

		m_bSecluded = true;

		if ( m_pPropPanel )
			m_pPropPanel->SetVisible( false );
		if ( m_pToolButtonsPanel )
			m_pToolButtonsPanel->SetVisible( false );
		if ( m_pMinimizeButton )
			m_pMinimizeButton->SetText( "Rejoin" );

		// Drop the frame chrome so what's left looks like the standalone
		// settings box, and give the keyboard back to the game.
		SetTitleBarVisible( false );
		SetPaintBackgroundEnabled( false );
		SetSizeable( false );
		SetKeyBoardInputEnabled( false );
		SetBounds( m_nRestoreBounds[0] + boxX, m_nRestoreBounds[1] + boxY, boxWide, boxTall );
	}
	else
	{
		m_bSecluded = false;

		SetTitleBarVisible( true );
		SetPaintBackgroundEnabled( true );
		SetSizeable( true );
		SetKeyBoardInputEnabled( IsVisible() );
		SetBounds( m_nRestoreBounds[0], m_nRestoreBounds[1], m_nRestoreBounds[2], m_nRestoreBounds[3] );

		if ( m_pPropPanel )
			m_pPropPanel->SetVisible( true );
		if ( m_pToolButtonsPanel )
			m_pToolButtonsPanel->SetVisible( true );
		if ( m_pMinimizeButton )
			m_pMinimizeButton->SetText( "Seclude" );
	}

	InvalidateLayout( true );
	if ( m_pContextPanel )
	{
		m_pContextPanel->InvalidateLayout( true );
		m_pContextPanel->MoveToFront();
	}
	if ( m_pMinimizeButton )
	{
		m_pMinimizeButton->MoveToFront();
	}
}

//-----------------------------------------------------------------------------
// Show a tool's settings box (gm_context <name>) and its Seclude button.
// "props"/"spawnmenu"/empty mean "hide the box" (see CContextPanel::ShowContext).
//-----------------------------------------------------------------------------
void CClientSpawnDialog::ShowToolContext( const char *contextType )
{
	if ( !m_pContextPanel )
		return;

	m_pContextPanel->ShowContext( contextType );

	bool bVisible = m_pContextPanel->IsContextVisible();

	// Track the active tool's context across menu close/open. Tools without a
	// context clear it so reopening the menu doesn't show a stale panel.
	if ( bVisible )
	{
		Q_strncpy( m_szLastContext, contextType, sizeof(m_szLastContext) );
	}
	else
	{
		m_szLastContext[0] = '\0';
	}
	// A tool without settings collapses the settings column, so the tool list
	// has to be re-laid out either way.
	if ( !bVisible && m_bSecluded )
	{
		SetSecluded( false );
	}

	InvalidateLayout( true );
	if ( bVisible )
	{
		m_pContextPanel->InvalidateLayout( true );
		m_pContextPanel->MoveToFront();
	}

	if ( m_pMinimizeButton )
	{
		m_pMinimizeButton->SetVisible( bVisible );
		if ( bVisible )
		{
			m_pMinimizeButton->MoveToFront();
		}
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

		// Q is the +menu bind; while the menu is open the frame captures
		// keyboard input, so close it here to get toggle behavior.
		case vgui::KEY_Q:
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
		// "Seclude" packs the build menu away and keeps the tool settings box
		// on screen; pressing it again ("Rejoin") brings the menu back.
		SetSecluded( !m_bSecluded );
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

	vgui::HFont font = pScheme->GetFont( "SpawnMenuButton", IsProportional() );
	if ( font == vgui::INVALID_FONT )
	{
		font = pScheme->GetFont( "Default", IsProportional() );
	}

	if ( font != vgui::INVALID_FONT )
	{
		for ( int i = 0; i < m_ToolButtons.Count(); ++i )
		{
			vgui::Label *label = dynamic_cast<vgui::Label *>( m_ToolButtons[i].pPanel );
			if ( label )
			{
				label->SetFont( font );
			}
		}
	}
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
static bool BM_ExtractContextFromCommand( const char *command, char *contextName, int contextNameSize )
{
	if ( !contextName || contextNameSize <= 0 )
		return false;

	contextName[0] = '\0';
	if ( !command )
		return false;

	const char *contextCommand = Q_stristr( command, "gm_context" );
	if ( !contextCommand )
		return false;

	contextCommand += Q_strlen( "gm_context" );
	while ( *contextCommand == ' ' || *contextCommand == '\t' )
	{
		contextCommand++;
	}

	int length = 0;
	while ( *contextCommand && *contextCommand != ';' &&
		*contextCommand != ' ' && *contextCommand != '\t' &&
		*contextCommand != '\r' && *contextCommand != '\n' )
	{
		if ( length < contextNameSize - 1 )
		{
			contextName[length++] = *contextCommand;
		}
		contextCommand++;
	}

	contextName[length] = '\0';
	return length > 0;
}

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

				// Apply the per-tool settings panel immediately.  Waiting for the
				// queued console command made the floating dialog unreliable, and
				// tools without gm_context left the previous tool's panel behind.
				if ( g_pSpawnMenu )
				{
					char contextName[128];
					if ( BM_ExtractContextFromCommand( entry.szCommand, contextName, sizeof(contextName) ) )
					{
						g_pSpawnMenu->ShowToolContext( contextName );
					}
					else
					{
						g_pSpawnMenu->ShowToolContext( NULL );
					}
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

void CToolButtonsPanel::ApplyResourceSettings()
{
	KeyValues *pResource = new KeyValues( "BuildMenuResource" );
	if ( !pResource->LoadFromFile( filesystem, TOOLBUTTONS_RES_FILE, "MOD" ) )
	{
		pResource->deleteThis();
		return;
	}

	KeyValues *pPanelSettings = pResource->FindKey( "ToolButtonsPanel" );
	if ( pPanelSettings )
	{
		ApplySettings( pPanelSettings );
	}

	KeyValues *pLabelSettings = pResource->FindKey( "BuildMenuLabelTemplate" );
	KeyValues *pButtonSettings = pResource->FindKey( "BuildMenuButtonTemplate" );
	KeyValues *pDoubleButtonSettings = pResource->FindKey( "BuildMenuDoubleButtonTemplate" );

	for ( int i = 0; i < m_ToolButtons.Count(); i++ )
	{
		ToolButton_t &entry = m_ToolButtons[i];
		KeyValues *pSettings = entry.bIsLabel ? pLabelSettings :
			( entry.bDoubleHeight ? pDoubleButtonSettings : pButtonSettings );
		if ( entry.pPanel && pSettings )
		{
			entry.pPanel->ApplySettings( pSettings );
			entry.pPanel->SetVisible( true );
		}
	}

	pResource->deleteThis();
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
	newEntry.pButton->AddActionSignalTarget( this );
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

	// LoadFromFile renames the root key to the file's first section (e.g.
	// "menu"), and chains any further top-level sections via GetNextKey - so
	// look for "menu"/"clear" along the root chain, not as subkeys.
	KeyValues *pMenu = NULL;
	for ( KeyValues *pRoot = pConfig; pRoot; pRoot = pRoot->GetNextKey() )
	{
		if ( Q_stricmp( pRoot->GetName(), "clear" ) == 0 && pRoot->GetInt() != 0 )
		{
			ClearToolButtons();
		}
		else if ( Q_stricmp( pRoot->GetName(), "menu" ) == 0 && !pMenu )
		{
			pMenu = pRoot;
		}
	}

	if ( !pMenu )
	{
		pMenu = pConfig->FindKey( "menu" );
	}

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

	ApplyResourceSettings();
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

	m_Controls.RemoveAll();
	m_ComboCommands.RemoveAll();
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

//-----------------------------------------------------------------------------
// "toggle" control - a check button bound to a convar (e.g. "Bloom On")
//-----------------------------------------------------------------------------
void CContextPanel::AddContextToggle( KeyValues *pControl, int &x, int &y, int &rowTall )
{
	if ( !m_pContextContent || !pControl )
		return;

	BM_FlushContextRow( x, y, rowTall );

	const char *name = pControl->GetString( "name", "" );
	const char *label = pControl->GetString( "label", name );

	char panelName[64];
	Q_snprintf( panelName, sizeof(panelName), "ContextToggle%d", m_Controls.Count() );

	vgui::CheckButton *pCheck = new vgui::CheckButton( m_pContextContent, panelName, label );
	pCheck->AddActionSignalTarget( this );
	pCheck->SetBounds( 4, y, max( GetWide() - 28, 160 ), 20 );
	pCheck->SetVisible( true );
	pCheck->SetMouseInputEnabled( true );
	pCheck->SetScheme( GetScheme() );

	const ConVar *pVar = cvar->FindVar( name );
	if ( pVar )
	{
		pCheck->SetSelected( pVar->GetInt() != 0 );
	}

	ContextControl_t control;
	memset( &control, 0, sizeof(control) );
	control.type = CONTEXT_CTRL_TOGGLE;
	Q_strncpy( control.szConVar, name, sizeof(control.szConVar) );
	control.pControl = pCheck;
	m_Controls.AddToTail( control );

	y += 20 + 3;
}

//-----------------------------------------------------------------------------
// "slider" control - label + slider + live value readout bound to a convar
//-----------------------------------------------------------------------------
void CContextPanel::AddContextSlider( KeyValues *pControl, int &x, int &y, int &rowTall )
{
	if ( !m_pContextContent || !pControl )
		return;

	BM_FlushContextRow( x, y, rowTall );

	const char *name = pControl->GetString( "name", "" );
	const char *label = pControl->GetString( "label", name );
	const float flMin = pControl->GetFloat( "min", 0.0f );
	const float flMax = pControl->GetFloat( "max", 1.0f );
	const bool bInteger = pControl->GetInt( "integer", 0 ) != 0;

	const int contentWide = max( GetWide() - 28, 160 );
	const int labelWide = 64;
	const int valueWide = 36;
	const int sliderWide = max( contentWide - labelWide - valueWide - 4, 40 );
	const int rowHigh = 20;

	char panelName[64];
	Q_snprintf( panelName, sizeof(panelName), "ContextSliderLabel%d", m_Controls.Count() );

	vgui::Label *pLabel = new vgui::Label( m_pContextContent, panelName, label );
	pLabel->SetContentAlignment( vgui::Label::a_west );
	pLabel->SetFgColor( Color( 235, 235, 205, 255 ) );
	pLabel->SetPaintBackgroundEnabled( false );
	pLabel->SetBounds( 4, y, labelWide, rowHigh );
	pLabel->SetMouseInputEnabled( false );

	Q_snprintf( panelName, sizeof(panelName), "ContextSlider%d", m_Controls.Count() );

	vgui::Slider *pSlider = new vgui::Slider( m_pContextContent, panelName );
	pSlider->AddActionSignalTarget( this );
	pSlider->SetBounds( 4 + labelWide, y, sliderWide, rowHigh );
	pSlider->SetVisible( true );
	pSlider->SetMouseInputEnabled( true );
	pSlider->SetScheme( GetScheme() );

	Q_snprintf( panelName, sizeof(panelName), "ContextSliderValue%d", m_Controls.Count() );

	vgui::Label *pValue = new vgui::Label( m_pContextContent, panelName, "" );
	pValue->SetContentAlignment( vgui::Label::a_east );
	pValue->SetFgColor( Color( 235, 235, 205, 255 ) );
	pValue->SetPaintBackgroundEnabled( false );
	pValue->SetBounds( 4 + labelWide + sliderWide + 2, y, valueWide, rowHigh );
	pValue->SetMouseInputEnabled( false );

	ContextControl_t control;
	memset( &control, 0, sizeof(control) );
	control.type = CONTEXT_CTRL_SLIDER;
	Q_strncpy( control.szConVar, name, sizeof(control.szConVar) );
	control.flMin = flMin;
	control.flMax = flMax;
	control.bInteger = bInteger;
	control.pControl = pSlider;
	control.pValueLabel = pValue;

	// Integer sliders step whole values; float sliders use 200 steps
	float flCurrent = flMin;
	const ConVar *pVar = cvar->FindVar( name );
	if ( pVar )
	{
		flCurrent = pVar->GetFloat();
	}
	if ( flCurrent < flMin ) flCurrent = flMin;
	if ( flCurrent > flMax ) flCurrent = flMax;

	if ( bInteger )
	{
		pSlider->SetRange( (int)flMin, (int)flMax );
		pSlider->SetValue( (int)flCurrent, false );
	}
	else
	{
		pSlider->SetRange( 0, 200 );
		const float flRange = ( flMax > flMin ) ? ( flMax - flMin ) : 1.0f;
		pSlider->SetValue( (int)( ( flCurrent - flMin ) / flRange * 200.0f + 0.5f ), false );
	}

	UpdateSliderValueLabel( control, flCurrent );
	m_Controls.AddToTail( control );

	y += rowHigh + 3;
}

//-----------------------------------------------------------------------------
// "keyvaluecombobox" control - preset dropdown loaded from a kv file
// (e.g. bloom presets). Selecting an item applies its settings.
//-----------------------------------------------------------------------------
void CContextPanel::AddContextComboBox( KeyValues *pControl, int &x, int &y, int &rowTall )
{
	if ( !m_pContextContent || !pControl )
		return;

	BM_FlushContextRow( x, y, rowTall );

	const char *kvFile = pControl->GetString( "kvfile", "" );
	if ( !kvFile[0] )
		return;

	const char *settingName = pControl->GetString( "name", "" );

	char panelName[64];
	Q_snprintf( panelName, sizeof(panelName), "ContextCombo%d", m_Controls.Count() );

	vgui::ComboBox *pCombo = new vgui::ComboBox( m_pContextContent, panelName, 8, false );
	pCombo->AddActionSignalTarget( this );
	pCombo->SetBounds( 4, y, max( GetWide() - 28, 160 ), 20 );
	pCombo->SetVisible( true );
	pCombo->SetMouseInputEnabled( true );
	pCombo->SetScheme( GetScheme() );

	KeyValues *pItems = new KeyValues( "ContextItems" );
	if ( pItems->LoadFromFile( filesystem, kvFile, "MOD" ) )
	{
		// LoadFromFile renames the root to the file's first section - the
		// preset entries are that section's subkeys.
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

			ContextComboCommand_t comboCommand;
			Q_strncpy( comboCommand.szCommand, command, sizeof(comboCommand.szCommand) );
			int commandIndex = m_ComboCommands.AddToTail( comboCommand );

			KeyValues *pUserData = new KeyValues( "ComboItem", "command_index", commandIndex );
			pCombo->AddItem( pItem->GetName(), pUserData );
			pUserData->deleteThis();
		}
	}
	pItems->deleteThis();

	ContextControl_t control;
	memset( &control, 0, sizeof(control) );
	control.type = CONTEXT_CTRL_COMBO;
	Q_strncpy( control.szConVar, settingName, sizeof(control.szConVar) );
	control.pControl = pCombo;
	m_Controls.AddToTail( control );

	y += 20 + 3;
}

//-----------------------------------------------------------------------------
// Control lookup + slider math
//-----------------------------------------------------------------------------
CContextPanel::ContextControl_t *CContextPanel::FindControl( vgui::Panel *pPanel )
{
	for ( int i = 0; i < m_Controls.Count(); i++ )
	{
		if ( m_Controls[i].pControl == pPanel )
			return &m_Controls[i];
	}
	return NULL;
}

float CContextPanel::GetSliderConValue( const ContextControl_t &control, int sliderPos ) const
{
	if ( control.bInteger )
		return (float)sliderPos;

	const float flRange = ( control.flMax > control.flMin ) ? ( control.flMax - control.flMin ) : 1.0f;
	return control.flMin + flRange * ( (float)sliderPos / 200.0f );
}

void CContextPanel::UpdateSliderValueLabel( ContextControl_t &control, float value )
{
	if ( !control.pValueLabel )
		return;

	char text[32];
	if ( control.bInteger )
	{
		Q_snprintf( text, sizeof(text), "%d", (int)value );
	}
	else
	{
		Q_snprintf( text, sizeof(text), "%.2f", value );
	}
	control.pValueLabel->SetText( text );
}

//-----------------------------------------------------------------------------
// Action signal handlers
//-----------------------------------------------------------------------------
void CContextPanel::OnSliderMoved( vgui::Panel *panel, int position )
{
	ContextControl_t *pControl = FindControl( panel );
	if ( !pControl || pControl->type != CONTEXT_CTRL_SLIDER || !pControl->szConVar[0] )
		return;

	float value = GetSliderConValue( *pControl, position );
	UpdateSliderValueLabel( *pControl, value );

	char command[128];
	if ( pControl->bInteger )
	{
		Q_snprintf( command, sizeof(command), "%s %d\n", pControl->szConVar, (int)value );
	}
	else
	{
		Q_snprintf( command, sizeof(command), "%s %f\n", pControl->szConVar, value );
	}
	engine->ClientCmd( command );
}

void CContextPanel::OnCheckButtonChecked( vgui::Panel *panel, int state )
{
	ContextControl_t *pControl = FindControl( panel );
	if ( !pControl || pControl->type != CONTEXT_CTRL_TOGGLE || !pControl->szConVar[0] )
		return;

	char command[128];
	Q_snprintf( command, sizeof(command), "%s %d\n", pControl->szConVar, state ? 1 : 0 );
	engine->ClientCmd( command );
}

void CContextPanel::OnTextChanged( vgui::Panel *panel )
{
	ContextControl_t *pControl = FindControl( panel );
	if ( !pControl || pControl->type != CONTEXT_CTRL_COMBO )
		return;

	vgui::ComboBox *pCombo = dynamic_cast<vgui::ComboBox *>( panel );
	if ( !pCombo )
		return;

	KeyValues *pUserData = pCombo->GetActiveItemUserData();
	if ( !pUserData )
		return;

	int commandIndex = pUserData->GetInt( "command_index", -1 );
	if ( commandIndex < 0 || commandIndex >= m_ComboCommands.Count() )
		return;

	if ( m_ComboCommands[commandIndex].szCommand[0] )
	{
		char command[560];
		Q_snprintf( command, sizeof(command), "%s\n", m_ComboCommands[commandIndex].szCommand );
		engine->ClientCmd( command );
	}
}

vgui::MessageMapItem_t CContextPanel::m_MessageMap[] =
{
	MAP_MESSAGE_PTR_INT( CContextPanel, "SliderMoved", OnSliderMoved, "panel", "position" ),
	MAP_MESSAGE_PTR_INT( CContextPanel, "CheckButtonChecked", OnCheckButtonChecked, "panel", "state" ),
	MAP_MESSAGE_PTR( CContextPanel, "TextChanged", OnTextChanged, "panel" ),
};
IMPLEMENT_PANELMAP( CContextPanel, vgui::EditablePanel );

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
				AddContextComboBox( pControl, x, y, rowTall );
			}
			else if ( Q_stricmp( controlType, "smalltext" ) == 0 || Q_stricmp( controlType, "text" ) == 0 )
			{
				AddContextLabel( pControl->GetString( "label", "" ), x, y, rowTall, pControl->GetInt( "tall", 20 ) );
			}
			else if ( Q_stricmp( controlType, "toggle" ) == 0 )
			{
				AddContextToggle( pControl, x, y, rowTall );
			}
			else if ( Q_stricmp( controlType, "slider" ) == 0 )
			{
				AddContextSlider( pControl, x, y, rowTall );
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
	// "props"/"spawnmenu"/empty used to mean "show the prop panel instead of
	// a tool's settings" back when this box also hosted the prop panel; the
	// prop panel is now a permanent sibling on the dialog (CClientSpawnDialog::
	// m_pPropPanel), so those aliases just mean "nothing to show".
	if ( !contextType || !contextType[0] || Q_stricmp( contextType, "null" ) == 0 ||
		Q_stricmp( contextType, "props" ) == 0 || Q_stricmp( contextType, "spawnmenu" ) == 0 )
	{
		HideContext();
		return;
	}

	Q_strncpy( m_szCurrentContext, contextType, sizeof(m_szCurrentContext) );
	m_bContextVisible = true;
	SetVisible( true );

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
	SetVisible( false );

	ClearContextContent();
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
