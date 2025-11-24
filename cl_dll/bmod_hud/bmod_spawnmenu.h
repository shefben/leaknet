//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Spawn Menu - Identical to Garry's Mod spawn menu system
//
//=============================================================================

#ifndef BMOD_SPAWNMENU_H
#define BMOD_SPAWNMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/EditablePanel.h>
#include "vgui/ILocalize.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "utlvector.h"

//-----------------------------------------------------------------------------
// Panel name definition
//-----------------------------------------------------------------------------
#define PANEL_SPAWNMENU "SpawnMenuPanel"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CToolButtonsPanel;
class CContextPanel;

//-----------------------------------------------------------------------------
// Simple viewport interface for spawn menu
//-----------------------------------------------------------------------------
class ISimpleViewPort
{
public:
	virtual void ShowBackGround( bool bShow ) = 0;
};

//-----------------------------------------------------------------------------
// Main spawn menu dialog - simplified version of Garry's Mod implementation
//-----------------------------------------------------------------------------
class CClientSpawnDialog : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CClientSpawnDialog, vgui::Frame );

public:
	CClientSpawnDialog( vgui::Panel *parent = NULL );
	virtual ~CClientSpawnDialog();

	// Panel management
	virtual void ShowPanel( bool bShow );
	virtual const char *GetName( void ) { return PANEL_SPAWNMENU; }

	// vgui::Frame overrides
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnKeyCodePressed( vgui::KeyCode code );
	virtual void OnCommand( const char *command );

	// Menu management
	void Initialize();
	void LoadMenuConfiguration();
	void ReloadSpawnMenu();

	// Panel access
	CToolButtonsPanel *GetToolButtonsPanel() { return m_pToolButtonsPanel; }
	CContextPanel *GetContextPanel() { return m_pContextPanel; }

	// External access methods
	void ScanPropsRecursive( const char *path );

protected:
	// UI Panel management
	void CreateMainPanel();
	void CreateToolButtonsPanel();
	void CreateContextPanel();
	void CreateMinimizeButton();

	// Configuration
	void LoadGModMenuConfiguration();
	void LoadFromKeyValueInternal( KeyValues *pKeyValues );

private:
	ISimpleViewPort	*m_pViewPort;

	// Main UI panels - matches Garry's Mod structure
	vgui::Panel		*m_pMainPanel;			// Main container panel
	vgui::Panel		*m_pPanel43;			// Additional panel slot
	CContextPanel	*m_pContextPanel;		// Context panel for menus
	CToolButtonsPanel *m_pToolButtonsPanel;	// Tool buttons panel
	vgui::Button	*m_pMinimizeButton;		// Minimize/close button
	vgui::Panel		*m_pPanel52;			// Additional panel slot

	// Additional panels
	vgui::Panel		*m_pPanel54;
	vgui::Panel		*m_pPanel55;
	vgui::Panel		*m_pPanel122;
	vgui::Panel		*m_pPanel123;
	vgui::Panel		*m_pPanel124;
	vgui::Panel		*m_pPanel127;

	// Menu configuration
	char			m_szBuffer[256];		// General purpose buffer
	bool			m_bFlags[21];			// Various boolean flags
	int				m_nConsoleHeight;		// Height for console integration
	bool			m_bInitialized;			// Whether menu is initialized
	bool			m_bAutoUpdate;			// Auto-update setting

	// Menu state
	bool			m_bVisible;
	bool			m_bShowingContext;
};

//-----------------------------------------------------------------------------
// Tool buttons panel - handles tool selection
//-----------------------------------------------------------------------------
class CToolButtonsPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CToolButtonsPanel, vgui::EditablePanel );

public:
	CToolButtonsPanel( vgui::Panel *parent, const char *panelName );
	virtual ~CToolButtonsPanel();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnCommand( const char *command );

	// Tool management
	void LoadToolButtons();
	void SetToolMode( int toolMode );
	int GetCurrentToolMode() const { return m_nCurrentToolMode; }

protected:
	void CreateToolButton( int toolID, const char *toolName, const char *description );

private:
	struct ToolButton_t
	{
		vgui::Button	*pButton;
		int				toolID;
		char			szName[64];
		char			szDescription[256];
	};

	CUtlVector<ToolButton_t>	m_ToolButtons;
	int							m_nCurrentToolMode;

	// Tool button arrays - matches Garry's Mod structure
	vgui::Button				*m_pToolButtons[20];	// Up to 20 tool buttons
	bool						m_bButtonStates[20];	// Button states
};

//-----------------------------------------------------------------------------
// Context panel - handles contextual spawn menus
//-----------------------------------------------------------------------------
class CContextPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CContextPanel, vgui::EditablePanel );

public:
	CContextPanel( vgui::Panel *parent, const char *panelName );
	virtual ~CContextPanel();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnCommand( const char *command );

	// Context menu management
	void ShowContext( const char *contextType );
	void HideContext();
	void LoadContextConfiguration();

private:
	char			m_szCurrentContext[64];
	bool			m_bContextVisible;
	vgui::Panel		*m_pContextContent;
};

//-----------------------------------------------------------------------------
// Constants - matching Garry's Mod values
//-----------------------------------------------------------------------------
#define PANEL_SPAWNMENU			"spawnmenu"
#define SPAWNMENU_SCHEME_FILE	"resource/SpawnMenuScheme.res"
#define SPAWNMENU_SCHEME_NAME	"SpawnMenuScheme"
#define TOOLBUTTONS_RES_FILE	"resource/ui/menu_toolbuttons.res"

// Menu configuration paths
#define MENU_MAIN_PATH			"settings/menu_main/"
#define MENU_PROPS_PATH			"settings/menu_props/"
#define CONTEXT_PANELS_PATH		"settings/context_panels/"
#define DEFAULT_CONFIG_FILE		"settings/menu_main/default.txt"
#define COMPLETE_DUMP_FILE		"settings/menu_props/complete_dump.txt"

// Tool modes
enum ToolMode_t
{
	TOOL_NONE = 0,
	TOOL_GUN = 1,
	TOOL_PHYSGUN = 2,
	TOOL_CAMERA = 3,
	TOOL_NPC = 4,
	TOOL_NEXTBOT = 5,
	TOOL_MATERIAL = 6,
	TOOL_COLOR = 7,
	TOOL_PAINT = 8,
	TOOL_INFLATOR = 9,
	TOOL_DUPLICATOR = 10,
	TOOL_CONSTRAINER = 11,
	TOOL_AXIS = 12,
	TOOL_BALLSOCKET = 13,
	TOOL_ROPE = 14,
	TOOL_PULLEY = 15,
	// Additional tools can be added here
	TOOL_MAX = 20
};

//-----------------------------------------------------------------------------
// Global spawn menu instance
//-----------------------------------------------------------------------------
extern CClientSpawnDialog *g_pSpawnMenu;

//-----------------------------------------------------------------------------
// Console commands
//-----------------------------------------------------------------------------
void CC_SpawnMenu();
void CC_ReloadSpawnMenu();
void CC_MakeCompleteSpawnList();

#endif // BMOD_SPAWNMENU_H