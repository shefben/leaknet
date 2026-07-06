//========= Copyright (c) 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Prop Panel Implementation
//          Based on Garry's Mod 9 spawn menu - IDA decompilation of sub_24191A00
//
//=============================================================================

#include "cbase.h"
#include "bmod_proppanel.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/ScrollBar.h>
#include "filesystem.h"
#include "KeyValues.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialSystem.h"
#include "sharedInterface.h"
#include "cdll_int.h"  // For engine->GetGameDirectory()

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

// Use the global filesystem from sharedInterface.h
// Renamed to g_pFullFilesystem to avoid conflict with vgui2::filesystem()
#define g_pFullFilesystem (::filesystem)

static bool BM_IsSpecialFindName( const char *pFileName )
{
	return ( !pFileName || !pFileName[0] ||
		Q_strcmp( pFileName, "." ) == 0 ||
		Q_strcmp( pFileName, ".." ) == 0 );
}

static void BM_BuildCategoryNameFromFileName( const char *pFileName, char *pOutName, int outNameSize )
{
	Q_strncpy( pOutName, pFileName ? pFileName : "", outNameSize );

	char *dot = Q_strrchr( pOutName, '.' );
	if ( dot )
		*dot = '\0';

	for ( int i = 0; pOutName[i]; i++ )
	{
		if ( pOutName[i] == '_' )
			pOutName[i] = ' ';
	}
}

static bool BM_HasPropCategory( CUtlVector<PropCategory_t> &categories, const char *pDisplayName )
{
	for ( int i = 0; i < categories.Count(); i++ )
	{
		if ( Q_stricmp( categories[i].szName, pDisplayName ) == 0 )
			return true;
	}

	return false;
}

static void BM_AddPropCategoryFile( CUtlVector<PropCategory_t> &categories, const char *pDirectory, const char *pFileName )
{
	if ( BM_IsSpecialFindName( pFileName ) || Q_stricmp( pFileName, "complete_dump.txt" ) == 0 )
		return;

	char displayName[64];
	BM_BuildCategoryNameFromFileName( pFileName, displayName, sizeof( displayName ) );
	if ( BM_HasPropCategory( categories, displayName ) )
		return;

	PropCategory_t cat;
	Q_strncpy( cat.szName, displayName, sizeof( cat.szName ) );
	Q_snprintf( cat.szFilePath, sizeof( cat.szFilePath ), "%s/%s", pDirectory, pFileName );
	cat.bLoaded = false;

	categories.AddToTail( cat );
}

static void BM_LoadPropCategoriesFromDirectory( CUtlVector<PropCategory_t> &categories, const char *pDirectory )
{
	char searchPath[256];
	Q_snprintf( searchPath, sizeof( searchPath ), "%s/*.txt", pDirectory );

	FileFindHandle_t findHandle = FILESYSTEM_INVALID_FIND_HANDLE;
	const char *pFileName = g_pFullFilesystem->FindFirst( searchPath, &findHandle );
	while ( pFileName )
	{
		BM_AddPropCategoryFile( categories, pDirectory, pFileName );
		pFileName = g_pFullFilesystem->FindNext( findHandle );
	}

	if ( findHandle != FILESYSTEM_INVALID_FIND_HANDLE )
	{
		g_pFullFilesystem->FindClose( findHandle );
	}
}

//=============================================================================
// CPropItemButton Implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CPropItemButton::CPropItemButton( Panel *parent, const char *panelName, const PropEntry_t &entry )
	: BaseClass( parent, panelName, entry.szDisplayName )
{
	m_Entry = entry;
	m_nTextureID = -1;
	m_bImageLoaded = false;

	// Set button size based on whether it has an icon
	if ( entry.bHasIcon )
	{
		SetSize( PROP_BUTTON_WIDTH_ICON, PROP_BUTTON_HEIGHT_ICON );
	}
	else
	{
		SetSize( PROP_BUTTON_WIDTH_TEXT, PROP_BUTTON_HEIGHT_TEXT );
	}

	SetVisible( true );
	SetMouseInputEnabled( true );
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CPropItemButton::~CPropItemButton()
{
}

//-----------------------------------------------------------------------------
// Apply scheme settings
//-----------------------------------------------------------------------------
void CPropItemButton::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	if ( m_Entry.bHasIcon )
	{
		// Image button style - transparent background
		SetBgColor( Color( 0, 0, 0, 0 ) );
		SetFgColor( Color( 255, 255, 255, 255 ) );
		SetPaintBorderEnabled( false );

		// Try to load the icon material
		if ( !m_bImageLoaded && m_Entry.szIconPath[0] )
		{
			DevMsg( "SpawnMenu: Loading icon texture '%s' for '%s'\n", m_Entry.szIconPath, m_Entry.szDisplayName );
			m_nTextureID = surface()->CreateNewTextureID();
			surface()->DrawSetTextureFile( m_nTextureID, m_Entry.szIconPath, true, false );
			m_bImageLoaded = true;
		}
		else if ( !m_Entry.szIconPath[0] )
		{
			DevMsg( "SpawnMenu: WARNING - bHasIcon=true but szIconPath is empty for '%s'\n", m_Entry.szDisplayName );
		}
	}
	else
	{
		// Text button style - use scheme Button colors with visible defaults
		SetBgColor( pScheme->GetColor( "Button.BgColor", Color( 80, 80, 80, 255 ) ) );
		SetFgColor( pScheme->GetColor( "Button.TextColor", Color( 255, 255, 255, 255 ) ) );

		// 2003 VGUI: SetArmedColor takes both fg and bg
		Color armedFg = pScheme->GetColor( "Button.ArmedTextColor", Color( 255, 255, 255, 255 ) );
		Color armedBg = pScheme->GetColor( "Button.ArmedBgColor", Color( 100, 100, 100, 255 ) );
		SetArmedColor( armedFg, armedBg );

		Color depressedFg = pScheme->GetColor( "Button.DepressedTextColor", Color( 255, 255, 255, 255 ) );
		Color depressedBg = pScheme->GetColor( "Button.DepressedBgColor", Color( 50, 50, 50, 255 ) );
		SetDepressedColor( depressedFg, depressedBg );

		// Use SpawnPanelButton font (matches original GMod) with fallback
		HFont font = pScheme->GetFont( "SpawnPanelButton", IsProportional() );
		if ( !font )
		{
			font = pScheme->GetFont( "DefaultSmall", IsProportional() );
		}
		if ( !font )
		{
			font = pScheme->GetFont( "Default", IsProportional() );
		}
		if ( font )
		{
			SetFont( font );
		}
	}
}

//-----------------------------------------------------------------------------
// Mouse pressed - spawn the prop
//-----------------------------------------------------------------------------
void CPropItemButton::OnMousePressed( MouseCode code )
{
	if ( code == MOUSE_LEFT )
	{
		// Execute spawn command
		char cmd[512];

		if ( m_Entry.itemType == PROPITEM_ENTITY || m_Entry.itemType == PROPITEM_CUSTOM )
		{
			// For entities and custom commands, the model path IS the command
			Q_snprintf( cmd, sizeof(cmd), "%s", m_Entry.szModelPath );
		}
		else
		{
			Q_snprintf( cmd, sizeof(cmd), "%s \"%s\"", m_Entry.szCommand, m_Entry.szModelPath );
		}

		engine->ClientCmd( cmd );
		Msg( "Spawn: %s\n", cmd );
	}

	BaseClass::OnMousePressed( code );
}

//-----------------------------------------------------------------------------
// Paint - draw icon if present
//-----------------------------------------------------------------------------
void CPropItemButton::Paint()
{
	if ( m_Entry.bHasIcon && m_bImageLoaded && m_nTextureID != -1 )
	{
		// Draw the icon material
		surface()->DrawSetTexture( m_nTextureID );
		surface()->DrawSetColor( 255, 255, 255, 255 );

		int wide, tall;
		GetSize( wide, tall );
		surface()->DrawTexturedRect( 0, 0, wide, tall );

		// Draw border on hover
		if ( IsArmed() )
		{
			surface()->DrawSetColor( 255, 255, 0, 255 );
			surface()->DrawOutlinedRect( 0, 0, wide, tall );
		}
	}
	else
	{
		// Let base class handle text drawing
		BaseClass::Paint();
	}
}

//=============================================================================
// CPropLabelPanel Implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CPropLabelPanel::CPropLabelPanel( Panel *parent, const char *panelName, const char *labelText )
	: BaseClass( parent, panelName )
{
	Q_strncpy( m_szLabel, labelText, sizeof(m_szLabel) );
	m_hFont = 0;
	SetSize( PROP_BUTTON_WIDTH_TEXT, PROP_LABEL_HEIGHT );
	SetMouseInputEnabled( false );
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CPropLabelPanel::~CPropLabelPanel()
{
}

//-----------------------------------------------------------------------------
// Apply scheme settings
//-----------------------------------------------------------------------------
void CPropLabelPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBgColor( Color( 0, 0, 0, 0 ) );

	// Use SpawnPanelTitle font (matches original GMod) with fallback
	m_hFont = pScheme->GetFont( "SpawnPanelTitle", IsProportional() );
	if ( !m_hFont )
	{
		m_hFont = pScheme->GetFont( "DefaultSmall", IsProportional() );
	}
	if ( !m_hFont )
	{
		m_hFont = pScheme->GetFont( "Default", IsProportional() );
	}
}

//-----------------------------------------------------------------------------
// Paint
//-----------------------------------------------------------------------------
void CPropLabelPanel::Paint()
{
	int wide, tall;
	GetSize( wide, tall );

	// Draw label text with underline
	surface()->DrawSetTextFont( m_hFont );
	surface()->DrawSetTextColor( 200, 200, 100, 255 );  // Yellow-ish for labels
	surface()->DrawSetTextPos( 2, 2 );

	wchar_t wLabel[128];
	localize()->ConvertANSIToUnicode( m_szLabel, wLabel, sizeof(wLabel) );
	surface()->DrawPrintText( wLabel, wcslen(wLabel) );

	// Draw underline
	surface()->DrawSetColor( 100, 100, 50, 255 );
	surface()->DrawLine( 0, tall - 1, wide, tall - 1 );
}

//=============================================================================
// CPropGridPanel Implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CPropGridPanel::CPropGridPanel( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_pCurrentCategory = NULL;
	m_nScrollOffset = 0;
	m_nTotalHeight = 0;
	m_nItemSpacing = 4;
	m_nRowSpacing = 4;

	SetMouseInputEnabled( true );
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CPropGridPanel::~CPropGridPanel()
{
	ClearItems();
}

//-----------------------------------------------------------------------------
// Apply scheme settings
//-----------------------------------------------------------------------------
void CPropGridPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetBgColor( pScheme->GetColor( "PropGrid.BgColor", Color( 30, 30, 30, 200 ) ) );
}

//-----------------------------------------------------------------------------
// Perform layout
//-----------------------------------------------------------------------------
void CPropGridPanel::PerformLayout()
{
	BaseClass::PerformLayout();
	LayoutItems();
}

//-----------------------------------------------------------------------------
// Mouse wheel scrolling
//-----------------------------------------------------------------------------
void CPropGridPanel::OnMouseWheeled( int delta )
{
	int newOffset = m_nScrollOffset - (delta * 30);
	SetScrollOffset( newOffset );
}

//-----------------------------------------------------------------------------
// Set the current category to display
//-----------------------------------------------------------------------------
void CPropGridPanel::SetCategory( PropCategory_t *pCategory )
{
	if ( m_pCurrentCategory == pCategory )
		return;

	m_pCurrentCategory = pCategory;
	m_nScrollOffset = 0;

	ClearItems();

	if ( pCategory )
	{
		CreateItemButtons();
		LayoutItems();
	}
}

//-----------------------------------------------------------------------------
// Clear all item buttons
//-----------------------------------------------------------------------------
void CPropGridPanel::ClearItems()
{
	for ( int i = 0; i < m_ItemPanels.Count(); i++ )
	{
		if ( m_ItemPanels[i] )
		{
			m_ItemPanels[i]->MarkForDeletion();
		}
	}
	m_ItemPanels.RemoveAll();
	m_nTotalHeight = 0;
}

//-----------------------------------------------------------------------------
// Set scroll offset
//-----------------------------------------------------------------------------
void CPropGridPanel::SetScrollOffset( int offset )
{
	int maxOffset = GetMaxScrollOffset();

	if ( offset < 0 )
		offset = 0;
	if ( offset > maxOffset )
		offset = maxOffset;

	if ( m_nScrollOffset != offset )
	{
		m_nScrollOffset = offset;
		LayoutItems();
	}
}

//-----------------------------------------------------------------------------
// Get maximum scroll offset
//-----------------------------------------------------------------------------
int CPropGridPanel::GetMaxScrollOffset() const
{
	// Cast away const to call GetTall() - 2003 VGUI doesn't have const-correct methods
	int viewHeight = const_cast<CPropGridPanel*>(this)->GetTall();
	if ( m_nTotalHeight <= viewHeight )
		return 0;
	return m_nTotalHeight - viewHeight;
}

//-----------------------------------------------------------------------------
// Check if icon material exists for a model
// Based on IDA decompilation of sub_24191A00 icon detection logic
// Icon files are stored in mods/spawnicons/materials/gmod/<modelpath>.vmt
// and are always lowercase.
//-----------------------------------------------------------------------------
bool CPropGridPanel::CheckIconExists( const char *modelPath, char *outIconPath, int outPathSize )
{
	if ( !modelPath || !modelPath[0] )
		return false;

	// Build icon path: materials/gmod/<model_path_without_extension>.vmt
	char iconVmtPath[256];
	char modelPathClean[256];

	Q_strncpy( modelPathClean, modelPath, sizeof(modelPathClean) );

	// Remove .mdl extension if present
	char *dot = Q_strrchr( modelPathClean, '.' );
	if ( dot )
		*dot = '\0';

	// Convert to lowercase - GMod icon files are always lowercase
	Q_strlower( modelPathClean );

	// Check for materials/gmod/<modelpath>.vmt
	// With mods/spawnicons in the search path, this will find:
	// mods/spawnicons/materials/gmod/<modelpath>.vmt
	Q_snprintf( iconVmtPath, sizeof(iconVmtPath), "materials/gmod/%s.vmt", modelPathClean );

	if ( g_pFullFilesystem->FileExists( iconVmtPath, "GAME" ) )
	{
		// Icon exists - return the material path (without materials/ prefix and .vmt)
		Q_snprintf( outIconPath, outPathSize, "gmod/%s", modelPathClean );
		DevMsg( "SpawnMenu: Found icon for '%s' -> '%s'\n", modelPath, outIconPath );
		return true;
	}

	DevMsg( "SpawnMenu: No icon for '%s' (checked '%s')\n", modelPath, iconVmtPath );
	return false;
}

//-----------------------------------------------------------------------------
// Get item type from prefix character
// Based on IDA decompilation of GMod_SpawnMenu_BuildItems (0x24192f80)
//-----------------------------------------------------------------------------
PropItemType_t CPropGridPanel::GetItemType( const char *displayName, const char **outCleanName )
{
	if ( !displayName || !displayName[0] )
	{
		*outCleanName = displayName;
		return PROPITEM_PROP;
	}

	char prefix = displayName[0];
	*outCleanName = displayName + 1;  // Skip prefix character

	switch ( prefix )
	{
		case '#':
			return PROPITEM_RAGDOLL;
		case '!':
			return PROPITEM_EFFECT;
		case '=':
			return PROPITEM_ENTITY;
		case ':':
			return PROPITEM_VEHICLE;
		case '\'':
			return PROPITEM_SPRITE;
		case '~':
		case '@':
			return PROPITEM_LABEL;
		case '+':
			return PROPITEM_CUSTOM;
		default:
			*outCleanName = displayName;  // No prefix, use full name
			return PROPITEM_PROP;
	}
}

//-----------------------------------------------------------------------------
// Get spawn command for item type
//-----------------------------------------------------------------------------
const char *CPropGridPanel::GetSpawnCommand( PropItemType_t type )
{
	switch ( type )
	{
		case PROPITEM_RAGDOLL:
			return "gmod_makeragdoll";
		case PROPITEM_EFFECT:
			return "gmod_makeeffect";
		case PROPITEM_ENTITY:
			return "gm_makeentity";
		case PROPITEM_VEHICLE:
			return "gmod_makevehicle";
		case PROPITEM_SPRITE:
			return "gmod_makesprite";
		case PROPITEM_CUSTOM:
			return "";  // Custom command is in the model path field
		case PROPITEM_PROP:
		default:
			return "gmod_makeprop";
	}
}

//-----------------------------------------------------------------------------
// Create item buttons from category entries
//-----------------------------------------------------------------------------
void CPropGridPanel::CreateItemButtons()
{
	if ( !m_pCurrentCategory || !m_pCurrentCategory->pEntries )
		return;

	// Get our scheme to propagate to child panels
	vgui::HScheme myScheme = GetScheme();

	CUtlVector<PropEntry_t> &entries = *m_pCurrentCategory->pEntries;

	for ( int i = 0; i < entries.Count(); i++ )
	{
		PropEntry_t &entry = entries[i];

		// Get item type from prefix
		const char *cleanName;
		entry.itemType = GetItemType( entry.szDisplayName, &cleanName );

		if ( entry.itemType == PROPITEM_LABEL )
		{
			// Create a label panel for category headers
			char labelName[64];
			Q_snprintf( labelName, sizeof(labelName), "Label%d", i );

			CPropLabelPanel *pLabel = new CPropLabelPanel( this, labelName, cleanName );
			pLabel->SetScheme( myScheme );
			pLabel->SetVisible( true );
			m_ItemPanels.AddToTail( pLabel );
		}
		else
		{
			// Check for icon
			entry.bHasIcon = CheckIconExists( entry.szModelPath, entry.szIconPath, sizeof(entry.szIconPath) );

			// Set button dimensions
			if ( entry.bHasIcon )
			{
				entry.nButtonWidth = PROP_BUTTON_WIDTH_ICON;
				entry.nButtonHeight = PROP_BUTTON_HEIGHT_ICON;
			}
			else
			{
				entry.nButtonWidth = PROP_BUTTON_WIDTH_TEXT;
				entry.nButtonHeight = PROP_BUTTON_HEIGHT_TEXT;
			}

			// Set spawn command
			Q_strncpy( entry.szCommand, GetSpawnCommand( entry.itemType ), sizeof(entry.szCommand) );

			// Update display name to clean version (without prefix)
			char cleanDisplayName[128];
			Q_strncpy( cleanDisplayName, cleanName, sizeof(cleanDisplayName) );
			Q_strncpy( entry.szDisplayName, cleanDisplayName, sizeof(entry.szDisplayName) );

			// Create button
			char buttonName[64];
			Q_snprintf( buttonName, sizeof(buttonName), "PropButton%d", i );

			CPropItemButton *pButton = new CPropItemButton( this, buttonName, entry );
			pButton->SetScheme( myScheme );
			pButton->SetMouseInputEnabled( true );
			m_ItemPanels.AddToTail( pButton );
		}
	}
}

//-----------------------------------------------------------------------------
// Layout items in a grid with mixed sizes
// Based on IDA decompilation showing 67x67 for icons, 100x19 for text
//-----------------------------------------------------------------------------
void CPropGridPanel::LayoutItems()
{
	int panelWidth = GetWide() - 10;  // Leave margin
	int startX = 5;
	int startY = 5 - m_nScrollOffset;

	int currentX = startX;
	int currentY = startY;
	int rowHeight = 0;
	int maxRowWidth = panelWidth;

	// Layout items - flow them left to right, wrapping
	for ( int i = 0; i < m_ItemPanels.Count(); i++ )
	{
		Panel *pPanel = m_ItemPanels[i];
		if ( !pPanel )
			continue;

		int itemWidth, itemHeight;
		pPanel->GetSize( itemWidth, itemHeight );

		// Check if this is a label (takes full width)
		CPropLabelPanel *pLabel = dynamic_cast<CPropLabelPanel*>( pPanel );
		if ( pLabel )
		{
			// Labels go on their own row
			if ( currentX != startX )
			{
				// Move to next row first
				currentY += rowHeight + m_nRowSpacing;
				currentX = startX;
				rowHeight = 0;
			}

			pLabel->SetPos( currentX, currentY );
			pLabel->SetWide( maxRowWidth );

			currentY += itemHeight + m_nRowSpacing;
			rowHeight = 0;
			continue;
		}

		// Check if item fits in current row
		if ( currentX + itemWidth > maxRowWidth && currentX != startX )
		{
			// Move to next row
			currentY += rowHeight + m_nRowSpacing;
			currentX = startX;
			rowHeight = 0;
		}

		// Position the item
		pPanel->SetPos( currentX, currentY );

		// Update position for next item
		currentX += itemWidth + m_nItemSpacing;
		if ( itemHeight > rowHeight )
			rowHeight = itemHeight;

		// Show/hide based on visibility in viewport
		int panelTop = currentY;
		int panelBottom = currentY + itemHeight;
		bool visible = ( panelBottom > 0 && panelTop < GetTall() );
		pPanel->SetVisible( visible );
	}

	// Calculate total content height
	m_nTotalHeight = currentY + rowHeight + m_nScrollOffset + 10;
}

//=============================================================================
// CPropCategoryButton Implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CPropCategoryButton::CPropCategoryButton( Panel *parent, const char *panelName, const char *categoryName, int categoryIndex )
	: BaseClass( parent, panelName, categoryName )
{
	m_nCategoryIndex = categoryIndex;

	char cmd[64];
	Q_snprintf( cmd, sizeof(cmd), "SelectCategory %d", categoryIndex );
	SetCommand( cmd );

	SetSize( PROP_CATEGORY_WIDTH, PROP_CATEGORY_HEIGHT );
	SetVisible( true );
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CPropCategoryButton::~CPropCategoryButton()
{
}

//-----------------------------------------------------------------------------
// Apply scheme settings
//-----------------------------------------------------------------------------
void CPropCategoryButton::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// Use visible button colors
	SetBgColor( pScheme->GetColor( "Button.BgColor", Color( 70, 70, 70, 255 ) ) );
	SetFgColor( pScheme->GetColor( "Button.TextColor", Color( 255, 255, 255, 255 ) ) );

	// 2003 VGUI: SetArmedColor takes both fg and bg
	Color armedFg = pScheme->GetColor( "Button.ArmedTextColor", Color( 255, 255, 255, 255 ) );
	Color armedBg = pScheme->GetColor( "Button.ArmedBgColor", Color( 100, 100, 100, 255 ) );
	SetArmedColor( armedFg, armedBg );

	// Use SpawnPanelButton font (matches original GMod) with fallback
	HFont font = pScheme->GetFont( "SpawnPanelButton", IsProportional() );
	if ( !font )
	{
		font = pScheme->GetFont( "DefaultSmall", IsProportional() );
	}
	if ( !font )
	{
		font = pScheme->GetFont( "Default", IsProportional() );
	}
	if ( font )
	{
		SetFont( font );
	}
}

//=============================================================================
// CPropCategoryList Implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CPropCategoryList::CPropCategoryList( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_nSelectedCategory = -1;
	SetMouseInputEnabled( true );
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CPropCategoryList::~CPropCategoryList()
{
	for ( int i = 0; i < m_CategoryButtons.Count(); i++ )
	{
		if ( m_CategoryButtons[i] )
		{
			m_CategoryButtons[i]->MarkForDeletion();
		}
	}
	m_CategoryButtons.RemoveAll();
	m_Categories.Purge();
}

//-----------------------------------------------------------------------------
// Apply scheme settings
//-----------------------------------------------------------------------------
void CPropCategoryList::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetBgColor( pScheme->GetColor( "CategoryList.BgColor", Color( 40, 40, 40, 200 ) ) );
}

//-----------------------------------------------------------------------------
// Perform layout
//-----------------------------------------------------------------------------
void CPropCategoryList::PerformLayout()
{
	BaseClass::PerformLayout();

	int buttonY = 5;
	for ( int i = 0; i < m_CategoryButtons.Count(); i++ )
	{
		if ( m_CategoryButtons[i] )
		{
			m_CategoryButtons[i]->SetPos( 5, buttonY );
			m_CategoryButtons[i]->SetWide( GetWide() - 10 );
			buttonY += PROP_CATEGORY_HEIGHT + 2;
		}
	}
}

//-----------------------------------------------------------------------------
// Handle commands
//-----------------------------------------------------------------------------
void CPropCategoryList::OnCommand( const char *command )
{
	if ( Q_strnicmp( command, "SelectCategory ", 15 ) == 0 )
	{
		int index = atoi( command + 15 );
		SelectCategory( index );

		// Forward to parent
		PostActionSignal( new KeyValues( "Command", "command", command ) );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

//-----------------------------------------------------------------------------
// Load categories from settings/menu_props/*.txt
// Based on IDA decompilation of sub_241935A0
//-----------------------------------------------------------------------------
void CPropCategoryList::LoadCategories()
{
	// Clear existing
	for ( int i = 0; i < m_CategoryButtons.Count(); i++ )
	{
		if ( m_CategoryButtons[i] )
		{
			m_CategoryButtons[i]->MarkForDeletion();
		}
	}
	m_CategoryButtons.RemoveAll();
	m_Categories.Purge();

	// Ensure spawnicons search path is added for icon materials
	// This adds mods/spawnicons to the search path which contains materials/gmod/models/*.vmt
	static bool bSpawniconsPathAdded = false;
	if ( !bSpawniconsPathAdded )
	{
		const char *pModPath = engine->GetGameDirectory();
		if ( pModPath )
		{
			char spawnicons_path[MAX_PATH];
			Q_snprintf( spawnicons_path, sizeof(spawnicons_path), "%s/mods/spawnicons", pModPath );

			// Check if the directory exists before adding
			if ( g_pFullFilesystem->IsDirectory( spawnicons_path, NULL ) )
			{
				g_pFullFilesystem->AddSearchPath( spawnicons_path, "GAME", PATH_ADD_TO_TAIL );
				Msg( "SpawnMenu: Added spawnicons search path: %s\n", spawnicons_path );
				bSpawniconsPathAdded = true;
			}
			else
			{
				Msg( "SpawnMenu: Spawnicons directory not found: %s\n", spawnicons_path );
			}
		}
	}

	// Scan base GMod prop categories and enabled mod categories copied into -modcache.
	BM_LoadPropCategoriesFromDirectory( m_Categories, "settings/menu_props" );
	BM_LoadPropCategoriesFromDirectory( m_Categories, "mods/-modcache/settings/menu_props" );

	Msg( "Loaded %d prop categories\n", m_Categories.Count() );

	// Get our scheme to propagate to child buttons
	vgui::HScheme myScheme = GetScheme();

	// Create category buttons
	for ( int i = 0; i < m_Categories.Count(); i++ )
	{
		char buttonName[64];
		Q_snprintf( buttonName, sizeof(buttonName), "CatButton%d", i );

		CPropCategoryButton *pButton = new CPropCategoryButton( this, buttonName, m_Categories[i].szName, i );
		pButton->SetScheme( myScheme );
		pButton->SetMouseInputEnabled( true );
		m_CategoryButtons.AddToTail( pButton );
	}

	// Select first category by default
	if ( m_Categories.Count() > 0 )
	{
		SelectCategory( 0 );
	}

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Select a category
//-----------------------------------------------------------------------------
void CPropCategoryList::SelectCategory( int index )
{
	if ( index < 0 || index >= m_Categories.Count() )
		return;

	m_nSelectedCategory = index;

	// Parse the category file if not already loaded
	if ( !m_Categories[index].bLoaded )
	{
		ParseCategoryFile( m_Categories[index] );
	}

	// Update button appearance
	for ( int i = 0; i < m_CategoryButtons.Count(); i++ )
	{
		if ( m_CategoryButtons[i] )
		{
			m_CategoryButtons[i]->SetSelected( i == index );
		}
	}
}

//-----------------------------------------------------------------------------
// Get a category
//-----------------------------------------------------------------------------
PropCategory_t *CPropCategoryList::GetCategory( int index )
{
	if ( index < 0 || index >= m_Categories.Count() )
		return NULL;

	return &m_Categories[index];
}

//-----------------------------------------------------------------------------
// Parse a category .txt file
// Format from GMod: "DisplayName" "models/path/model.mdl"
//-----------------------------------------------------------------------------
void CPropCategoryList::ParseCategoryFile( PropCategory_t &category )
{
	if ( category.bLoaded )
		return;

	// Allocate entries vector if needed
	if ( !category.pEntries )
	{
		category.pEntries = new CUtlVector<PropEntry_t>();
	}
	category.pEntries->Purge();

	KeyValues *pKV = new KeyValues( "Props" );
	if ( pKV->LoadFromFile( g_pFullFilesystem, category.szFilePath, "MOD" ) )
	{
		// Iterate through all entries
		for ( KeyValues *pEntry = pKV->GetFirstSubKey(); pEntry; pEntry = pEntry->GetNextKey() )
		{
			PropEntry_t entry;
			memset( &entry, 0, sizeof(entry) );

			Q_strncpy( entry.szDisplayName, pEntry->GetName(), sizeof(entry.szDisplayName) );
			Q_strncpy( entry.szModelPath, pEntry->GetString(), sizeof(entry.szModelPath) );

			category.pEntries->AddToTail( entry );
		}

		Msg( "Parsed category '%s': %d entries\n", category.szName, category.pEntries->Count() );
	}
	else
	{
		Msg( "Failed to parse category file: %s\n", category.szFilePath );
	}

	pKV->deleteThis();
	category.bLoaded = true;
}

//=============================================================================
// CPropPanel Implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CPropPanel::CPropPanel( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	// Load the scheme for prop panel
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile( "resource/SpawnMenuScheme.res", "SpawnMenuScheme" );
	SetScheme( scheme );

	// Create category list on left
	m_pCategoryList = new CPropCategoryList( this, "CategoryList" );
	m_pCategoryList->SetVisible( true );
	m_pCategoryList->SetScheme( scheme );
	m_pCategoryList->SetMouseInputEnabled( true );
	m_pCategoryList->AddActionSignalTarget( this );

	// Create prop grid on right
	m_pPropGrid = new CPropGridPanel( this, "PropGrid" );
	m_pPropGrid->SetVisible( true );
	m_pPropGrid->SetScheme( scheme );
	m_pPropGrid->SetMouseInputEnabled( true );

	// Create scrollbar
	m_pScrollBar = new ScrollBar( this, "ScrollBar", true );
	m_pScrollBar->SetVisible( true );
	m_pScrollBar->AddActionSignalTarget( this );
	m_pScrollBar->SetScheme( scheme );

	// Load categories
	m_pCategoryList->LoadCategories();
	PropCategory_t *pInitialCategory = m_pCategoryList->GetCategory( m_pCategoryList->GetSelectedCategory() );
	if ( !pInitialCategory )
	{
		pInitialCategory = m_pCategoryList->GetCategory( 0 );
	}
	if ( pInitialCategory )
	{
		m_pPropGrid->SetCategory( pInitialCategory );
	}

	SetMouseInputEnabled( true );
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CPropPanel::~CPropPanel()
{
}

//-----------------------------------------------------------------------------
// Apply scheme settings
//-----------------------------------------------------------------------------
void CPropPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetBgColor( pScheme->GetColor( "PropPanel.BgColor", Color( 20, 20, 20, 230 ) ) );
}

//-----------------------------------------------------------------------------
// Perform layout
//-----------------------------------------------------------------------------
void CPropPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize( wide, tall );

	// Category list on left (130px wide)
	m_pCategoryList->SetBounds( 0, 0, 130, tall );

	// Scrollbar on right (16px wide)
	m_pScrollBar->SetBounds( wide - 16, 0, 16, tall );

	// Prop grid in center
	m_pPropGrid->SetBounds( 130, 0, wide - 130 - 16, tall );

	// Update scrollbar range
	int maxScroll = m_pPropGrid->GetMaxScrollOffset();
	m_pScrollBar->SetRange( 0, maxScroll + tall );
	m_pScrollBar->SetRangeWindow( tall );
	m_pScrollBar->SetValue( m_pPropGrid->GetScrollOffset() );
}

//-----------------------------------------------------------------------------
// Handle commands
//-----------------------------------------------------------------------------
void CPropPanel::OnCommand( const char *command )
{
	if ( Q_strnicmp( command, "SelectCategory ", 15 ) == 0 )
	{
		int index = atoi( command + 15 );

		PropCategory_t *pCat = m_pCategoryList->GetCategory( index );
		if ( pCat )
		{
			m_pPropGrid->SetCategory( pCat );
			PerformLayout();  // Update scrollbar
		}
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

//-----------------------------------------------------------------------------
// Mouse wheel scrolling
//-----------------------------------------------------------------------------
void CPropPanel::OnMouseWheeled( int delta )
{
	m_pPropGrid->OnMouseWheeled( delta );
	m_pScrollBar->SetValue( m_pPropGrid->GetScrollOffset() );
}

//-----------------------------------------------------------------------------
// Reload props
//-----------------------------------------------------------------------------
void CPropPanel::ReloadProps()
{
	m_pCategoryList->LoadCategories();

	// Select first category
	PropCategory_t *pCat = m_pCategoryList->GetCategory( 0 );
	if ( pCat )
	{
		m_pPropGrid->SetCategory( pCat );
	}

	InvalidateLayout();
}
