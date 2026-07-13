//========= Copyright (c) 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Prop Panel - Displays props from settings/menu_props/*.txt
//          Based on Garry's Mod 9 spawn menu implementation
//
//=============================================================================

#ifndef BMOD_PROPPANEL_H
#define BMOD_PROPPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/PanelListPanel.h>
#include "utlvector.h"

// Forward declarations
class CPropGridPanel;
class CPropCategoryList;

//-----------------------------------------------------------------------------
// Prop item types based on prefix character
//-----------------------------------------------------------------------------
enum PropItemType_t
{
	PROPITEM_PROP = 0,		// Default - no prefix - gmod_makeprop
	PROPITEM_RAGDOLL,		// # prefix - gmod_makeragdoll
	PROPITEM_EFFECT,		// ! prefix - gmod_makeeffect
	PROPITEM_ENTITY,		// = prefix - gm_makeentity (or run command directly)
	PROPITEM_VEHICLE,		// : prefix - vehicle spawn
	PROPITEM_SPRITE,		// ' prefix - sprite spawn
	PROPITEM_LABEL,			// ~ or @ prefix - category label (not spawnable)
	PROPITEM_CUSTOM			// + prefix - custom command
};

//-----------------------------------------------------------------------------
// Structure for a single prop entry
//-----------------------------------------------------------------------------
struct PropEntry_t
{
	char			szDisplayName[128];		// Display name shown on button
	char			szModelPath[256];		// Model path for spawning
	char			szCommand[64];			// Console command to execute
	char			szIconPath[256];		// Material path for icon (if exists)
	PropItemType_t	itemType;				// Type of prop item
	bool			bHasIcon;				// Whether icon material exists
	int				nButtonWidth;			// Button width (67 with icon, 100 without)
	int				nButtonHeight;			// Button height (67 with icon, 19 without)
};

//-----------------------------------------------------------------------------
// Structure for a prop category (one .txt file)
//-----------------------------------------------------------------------------
struct PropCategory_t
{
	char						szName[64];			// Category name (from filename)
	char						szFilePath[256];	// Full path to .txt file
	CUtlVector<PropEntry_t>		*pEntries;			// Pointer to entries (allocated dynamically)
	bool						bLoaded;			// Whether entries have been parsed

	PropCategory_t()
	{
		szName[0] = '\0';
		szFilePath[0] = '\0';
		pEntries = NULL;
		bLoaded = false;
	}

	~PropCategory_t()
	{
		if ( pEntries )
		{
			delete pEntries;
			pEntries = NULL;
		}
	}

	// Prevent copy (CUtlVector can't be copied)
	PropCategory_t( const PropCategory_t &other )
	{
		Q_strncpy( szName, other.szName, sizeof(szName) );
		Q_strncpy( szFilePath, other.szFilePath, sizeof(szFilePath) );
		bLoaded = other.bLoaded;
		// Create new entries vector and copy contents
		if ( other.pEntries )
		{
			pEntries = new CUtlVector<PropEntry_t>();
			for ( int i = 0; i < other.pEntries->Count(); i++ )
			{
				pEntries->AddToTail( (*other.pEntries)[i] );
			}
		}
		else
		{
			pEntries = NULL;
		}
	}

	PropCategory_t &operator=( const PropCategory_t &other )
	{
		if ( this != &other )
		{
			Q_strncpy( szName, other.szName, sizeof(szName) );
			Q_strncpy( szFilePath, other.szFilePath, sizeof(szFilePath) );
			bLoaded = other.bLoaded;
			if ( pEntries )
				delete pEntries;
			if ( other.pEntries )
			{
				pEntries = new CUtlVector<PropEntry_t>();
				for ( int i = 0; i < other.pEntries->Count(); i++ )
				{
					pEntries->AddToTail( (*other.pEntries)[i] );
				}
			}
			else
			{
				pEntries = NULL;
			}
		}
		return *this;
	}
};

//-----------------------------------------------------------------------------
// CPropItemButton - A button that represents a single spawnable prop
//-----------------------------------------------------------------------------
class CPropItemButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( CPropItemButton, vgui::Button );

public:
	CPropItemButton( vgui::Panel *parent, const char *panelName, const PropEntry_t &entry );
	virtual ~CPropItemButton();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnMousePressed( vgui::MouseCode code );
	virtual void Paint();

	const PropEntry_t &GetEntry() const { return m_Entry; }

private:
	PropEntry_t		m_Entry;
	int				m_nTextureID;		// Material texture ID for icon
	bool			m_bImageLoaded;
};

//-----------------------------------------------------------------------------
// CPropLabelPanel - A label for category sections within the grid
//-----------------------------------------------------------------------------
class CPropLabelPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CPropLabelPanel, vgui::Panel );

public:
	CPropLabelPanel( vgui::Panel *parent, const char *panelName, const char *labelText );
	virtual ~CPropLabelPanel();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Paint();

private:
	char	m_szLabel[128];
	vgui::HFont m_hFont;
};

//-----------------------------------------------------------------------------
// CPropGridPanel - Grid panel that displays props from a category
//-----------------------------------------------------------------------------
class CPropGridPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CPropGridPanel, vgui::Panel );

public:
	CPropGridPanel( vgui::Panel *parent, const char *panelName );
	virtual ~CPropGridPanel();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnMouseWheeled( int delta );

	// Category management
	void SetCategory( PropCategory_t *pCategory );
	void ClearItems();

	// Scroll management
	void SetScrollOffset( int offset );
	int GetScrollOffset() const { return m_nScrollOffset; }
	int GetMaxScrollOffset() const;

private:
	void CreateItemButtons();
	void LayoutItems();
	bool CheckIconExists( const char *modelPath, char *outIconPath, int outPathSize );
	PropItemType_t GetItemType( const char *displayName, const char **outCleanName );
	const char *GetSpawnCommand( PropItemType_t type );

	PropCategory_t				*m_pCurrentCategory;
	CUtlVector<vgui::Panel*>	m_ItemPanels;		// Mix of buttons and labels
	int							m_nScrollOffset;
	int							m_nTotalHeight;		// Total content height
	int							m_nItemSpacing;
	int							m_nRowSpacing;
};

//-----------------------------------------------------------------------------
// CPropCategoryList - Category picker. Real GMod9 uses a single dropdown at
// the top of the spawn panel (not a vertical button list) to choose which
// settings/menu_props/*.txt file is showing below it.
//-----------------------------------------------------------------------------
class CPropCategoryList : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CPropCategoryList, vgui::Panel );
	DECLARE_PANELMAP();

public:
	CPropCategoryList( vgui::Panel *parent, const char *panelName );
	virtual ~CPropCategoryList();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnTextChanged( const char *text );

	void LoadCategories();
	void SelectCategory( int index );
	int GetSelectedCategory() const { return m_nSelectedCategory; }
	PropCategory_t *GetCategory( int index );
	int GetCategoryCount() const { return m_Categories.Count(); }

private:
	void ParseCategoryFile( PropCategory_t &category );

	CUtlVector<PropCategory_t>	m_Categories;
	vgui::ComboBox				*m_pCombo;
	int							m_nSelectedCategory;
};

//-----------------------------------------------------------------------------
// CPropPanel - Main prop panel containing category list and prop grid
//-----------------------------------------------------------------------------
class CPropPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CPropPanel, vgui::Panel );

public:
	CPropPanel( vgui::Panel *parent, const char *panelName );
	virtual ~CPropPanel();

	DECLARE_PANELMAP();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnCommand( const char *command );
	virtual void OnMouseWheeled( int delta );
	virtual void OnScrollBarSliderMoved( int position );

	void ReloadProps();

private:
	CPropCategoryList	*m_pCategoryList;
	CPropGridPanel		*m_pPropGrid;
	vgui::ScrollBar		*m_pScrollBar;
};

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define PROP_BUTTON_WIDTH_ICON		67		// Width with icon
#define PROP_BUTTON_HEIGHT_ICON		67		// Height with icon
#define PROP_BUTTON_WIDTH_TEXT		100		// Width without icon
#define PROP_BUTTON_HEIGHT_TEXT		19		// Height without icon
#define PROP_CATEGORY_WIDTH			120		// Category button width
#define PROP_CATEGORY_HEIGHT		20		// Category button height
#define PROP_LABEL_HEIGHT			16		// Label height

#endif // BMOD_PROPPANEL_H
