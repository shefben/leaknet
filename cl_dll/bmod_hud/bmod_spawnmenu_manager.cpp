//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Spawn Menu Manager - Integration with client systems
//
//=============================================================================

#include "cbase.h"
#include "bmod_spawnmenu.h"
#include "vgui_int.h"
#include "ienginevgui.h"
#include "iclientmode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Spawn menu manager class - handles initialization and cleanup
//-----------------------------------------------------------------------------
class CSpawnMenuManager : public IGameSystem
{
public:
	CSpawnMenuManager();
	virtual ~CSpawnMenuManager();

	// IGameSystem interface
	virtual char const *Name() { return "CSpawnMenuManager"; }
	virtual bool Init();
	virtual void PostInit() {}
	virtual void Shutdown();
	virtual void LevelInitPreEntity() {}
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPreEntity() {}
	virtual void LevelShutdownPostEntity();
	virtual void OnSave() {}
	virtual void OnRestore() {}
	virtual void SafeRemoveIfDesired() {}
	virtual bool IsPerFrame() { return false; }

	// CLIENT_DLL specific IGameSystem methods
	virtual void PreRender() {}
	virtual void Update( float frametime ) {}

	// Spawn menu access
	CClientSpawnDialog *GetSpawnMenu() { return m_pSpawnMenu; }
	void CreateSpawnMenu();
	void DestroySpawnMenu();

private:
	CClientSpawnDialog *m_pSpawnMenu;
	bool m_bInitialized;
};

//-----------------------------------------------------------------------------
// Global spawn menu manager
//-----------------------------------------------------------------------------
static CSpawnMenuManager g_SpawnMenuManager;
IGameSystem *SpawnMenuManager() { return &g_SpawnMenuManager; }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CSpawnMenuManager::CSpawnMenuManager()
{
	m_pSpawnMenu = NULL;
	m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CSpawnMenuManager::~CSpawnMenuManager()
{
	DestroySpawnMenu();
}

//-----------------------------------------------------------------------------
// Initialize the spawn menu system
//-----------------------------------------------------------------------------
bool CSpawnMenuManager::Init()
{
	if ( m_bInitialized )
		return true;

	Msg( "Initializing BarrysMod Spawn Menu System\n" );

	// Create the spawn menu
	CreateSpawnMenu();

	m_bInitialized = true;
	return true;
}

//-----------------------------------------------------------------------------
// Shutdown the spawn menu system
//-----------------------------------------------------------------------------
void CSpawnMenuManager::Shutdown()
{
	if ( !m_bInitialized )
		return;

	Msg( "Shutting down BarrysMod Spawn Menu System\n" );

	DestroySpawnMenu();
	m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Level initialization
//-----------------------------------------------------------------------------
void CSpawnMenuManager::LevelInitPostEntity()
{
	if ( !m_pSpawnMenu )
	{
		CreateSpawnMenu();
	}
}

//-----------------------------------------------------------------------------
// Level shutdown
//-----------------------------------------------------------------------------
void CSpawnMenuManager::LevelShutdownPostEntity()
{
	if ( m_pSpawnMenu )
	{
		m_pSpawnMenu->ShowPanel( false );
	}
}

//-----------------------------------------------------------------------------
// Create the spawn menu
//-----------------------------------------------------------------------------
void CSpawnMenuManager::CreateSpawnMenu()
{
	if ( m_pSpawnMenu )
		return;

	// Get the client DLL root panel
	vgui::VPANEL pParent = VGui_GetClientDLLRootPanel();
	if ( !pParent )
	{
		Warning( "Failed to get client DLL root panel for spawn menu\n" );
		return;
	}

	// Create the spawn menu
	m_pSpawnMenu = new CClientSpawnDialog( NULL );
	if ( !m_pSpawnMenu )
	{
		Warning( "Failed to create spawn menu\n" );
		return;
	}

	// Set parent to root panel
	m_pSpawnMenu->SetParent( pParent );
	m_pSpawnMenu->SetVisible( false );

	Msg( "Spawn menu created successfully\n" );
}

//-----------------------------------------------------------------------------
// Destroy the spawn menu
//-----------------------------------------------------------------------------
void CSpawnMenuManager::DestroySpawnMenu()
{
	if ( m_pSpawnMenu )
	{
		m_pSpawnMenu->MarkForDeletion();
		m_pSpawnMenu = NULL;
	}
}

//-----------------------------------------------------------------------------
// Console command to show spawn menu
//-----------------------------------------------------------------------------
CON_COMMAND( spawnmenu, "Opens the BarrysMod spawn menu" )
{
	CClientSpawnDialog *pSpawnMenu = g_SpawnMenuManager.GetSpawnMenu();
	if ( pSpawnMenu )
	{
		bool bCurrentlyVisible = pSpawnMenu->IsVisible();
		pSpawnMenu->ShowPanel( !bCurrentlyVisible );

		if ( !bCurrentlyVisible )
		{
			Msg( "Spawn menu opened\n" );
		}
		else
		{
			Msg( "Spawn menu closed\n" );
		}
	}
	else
	{
		Warning( "Spawn menu not available\n" );
	}
}

//-----------------------------------------------------------------------------
// Console command to reload spawn menu
//-----------------------------------------------------------------------------
CON_COMMAND( bm_reloadspawnmenu, "Reloads the BarrysMod spawn menu configuration" )
{
	CClientSpawnDialog *pSpawnMenu = g_SpawnMenuManager.GetSpawnMenu();
	if ( pSpawnMenu )
	{
		pSpawnMenu->ReloadSpawnMenu();
		Msg( "Spawn menu configuration reloaded\n" );
	}
	else
	{
		Warning( "Spawn menu not available\n" );
	}
}

//-----------------------------------------------------------------------------
// Console command to create complete spawn list
//-----------------------------------------------------------------------------
CON_COMMAND( bm_makecompletespawnlist, "Creates a complete spawn list for props" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: bm_makecompletespawnlist <path>\n" );
		Msg( "Example: bm_makecompletespawnlist cstrike/models/\n" );
		Msg( "WARNING: this WILL take a long time.\n" );
		return;
	}

	const char *path = engine->Cmd_Argv(1);
	CClientSpawnDialog *pSpawnMenu = g_SpawnMenuManager.GetSpawnMenu();
	if ( pSpawnMenu )
	{
		pSpawnMenu->ScanPropsRecursive( path );
		Msg( "Started scanning props in: %s\n", path );
	}
	else
	{
		Warning( "Spawn menu not available\n" );
	}
}

//-----------------------------------------------------------------------------
// Test command to show spawn menu information
//-----------------------------------------------------------------------------
CON_COMMAND( bm_spawnmenu_info, "Shows information about the spawn menu" )
{
	CClientSpawnDialog *pSpawnMenu = g_SpawnMenuManager.GetSpawnMenu();
	if ( pSpawnMenu )
	{
		Msg( "=== BarrysMod Spawn Menu Information ===\n" );
		Msg( "Spawn Menu: %s\n", pSpawnMenu ? "Created" : "Not Created" );
		Msg( "Visible: %s\n", pSpawnMenu->IsVisible() ? "Yes" : "No" );
		Msg( "Name: %s\n", pSpawnMenu->GetName() );

		CToolButtonsPanel *pToolPanel = pSpawnMenu->GetToolButtonsPanel();
		Msg( "Tool Buttons Panel: %s\n", pToolPanel ? "Created" : "Not Created" );
		if ( pToolPanel )
		{
			Msg( "Current Tool Mode: %d\n", pToolPanel->GetCurrentToolMode() );
		}

		CContextPanel *pContextPanel = pSpawnMenu->GetContextPanel();
		Msg( "Context Panel: %s\n", pContextPanel ? "Created" : "Not Created" );
	}
	else
	{
		Msg( "Spawn menu not available\n" );
	}
}

//-----------------------------------------------------------------------------
// Console command for tool mode switching
//-----------------------------------------------------------------------------
CON_COMMAND( bm_toolmode, "Sets the current tool mode" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: bm_toolmode <mode_number>\n" );
		Msg( "Available modes:\n" );
		Msg( "  1 - Gun\n" );
		Msg( "  2 - Physgun\n" );
		Msg( "  3 - Camera\n" );
		Msg( "  4 - NPC\n" );
		Msg( "  5 - NextBot\n" );
		Msg( "  ... (see tool definitions)\n" );
		return;
	}

	int toolMode = atoi( engine->Cmd_Argv(1) );
	CClientSpawnDialog *pSpawnMenu = g_SpawnMenuManager.GetSpawnMenu();
	if ( pSpawnMenu )
	{
		CToolButtonsPanel *pToolPanel = pSpawnMenu->GetToolButtonsPanel();
		if ( pToolPanel )
		{
			pToolPanel->SetToolMode( toolMode );
			Msg( "Tool mode set to: %d\n", toolMode );
		}
		else
		{
			Warning( "Tool buttons panel not available\n" );
		}
	}
	else
	{
		Warning( "Spawn menu not available\n" );
	}
}

//-----------------------------------------------------------------------------
// Console command for context menu
//-----------------------------------------------------------------------------
CON_COMMAND( bm_context, "Shows a context menu" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: bm_context <context_type>\n" );
		Msg( "Examples: bm_context npc, bm_context camera\n" );
		return;
	}

	const char *contextType = engine->Cmd_Argv(1);
	CClientSpawnDialog *pSpawnMenu = g_SpawnMenuManager.GetSpawnMenu();
	if ( pSpawnMenu )
	{
		CContextPanel *pContextPanel = pSpawnMenu->GetContextPanel();
		if ( pContextPanel )
		{
			pContextPanel->ShowContext( contextType );
			Msg( "Showing context: %s\n", contextType );
		}
		else
		{
			Warning( "Context panel not available\n" );
		}
	}
	else
	{
		Warning( "Spawn menu not available\n" );
	}
}