// Copyright (c) BarrysMod.
// Derma manager implementation: skins + control registry + root overlay.

#include "cbase.h"
#include "derma_manager.h"
#include "filesystem.h"
#include "vstdlib/strtools.h"
#include <string.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/IPanel.h>
#include <Color.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/Slider.h>
#include <KeyValues.h>

//-----------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------
static bool ParseColorString( const char *pszText, Color &outColor )
{
	if ( !pszText || !pszText[0] )
		return false;

	int r = 0, g = 0, b = 0, a = 255;
	int parsed = sscanf( pszText, "%d %d %d %d", &r, &g, &b, &a );
	if ( parsed < 3 )
		return false;

	outColor = Color( clamp( r, 0, 255 ), clamp( g, 0, 255 ), clamp( b, 0, 255 ), clamp( a, 0, 255 ) );
	return true;
}

//-----------------------------------------------------------------------------
// CDermaSkin
//-----------------------------------------------------------------------------
CDermaSkin::CDermaSkin() :
	m_pKV( NULL )
{
	m_szName[0] = 0;
}

CDermaSkin::~CDermaSkin()
{
	if ( m_pKV )
	{
		m_pKV->deleteThis();
		m_pKV = NULL;
	}
}

bool CDermaSkin::LoadFromFile( const char *pszFilename )
{
	if ( !pszFilename )
		return false;

	if ( m_pKV )
	{
		m_pKV->deleteThis();
		m_pKV = NULL;
	}

	m_pKV = new KeyValues( "DermaSkin" );
	if ( !m_pKV->LoadFromFile( filesystem, pszFilename, "GAME" ) )
	{
		m_pKV->deleteThis();
		m_pKV = NULL;
		return false;
	}

	// Derive name from filename (no path / extension)
	const char *base = pszFilename;
	const char *slash = max( strrchr( pszFilename, '\\' ), strrchr( pszFilename, '/' ) );
	if ( slash )
		base = slash + 1;
	Q_strncpy( m_szName, base, sizeof( m_szName ) );
	char *dot = strrchr( m_szName, '.' );
	if ( dot )
		*dot = '\0';
	return true;
}

Color CDermaSkin::GetColor( const char *pszPath, const Color &fallback ) const
{
	if ( !m_pKV || !pszPath )
		return fallback;

	KeyValues *kv = m_pKV->FindKey( pszPath );
	if ( !kv )
		return fallback;

	Color c = fallback;
	ParseColorString( kv->GetString(), c );
	return c;
}

const char* CDermaSkin::GetFont( const char *pszPath ) const
{
	if ( !m_pKV || !pszPath )
		return NULL;

	KeyValues *kv = m_pKV->FindKey( pszPath );
	if ( !kv )
		return NULL;

	return kv->GetString();
}

//-----------------------------------------------------------------------------
// Simple root overlay to host Derma-created panels
//-----------------------------------------------------------------------------
class CDermaRootPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CDermaRootPanel, vgui::EditablePanel );
public:
	CDermaRootPanel( vgui::Panel *pParent, const char *pName ) :
		BaseClass( pParent, pName )
	{
		SetVisible( true );
		SetPaintBackgroundEnabled( false );
		SetKeyBoardInputEnabled( true );
		SetMouseInputEnabled( true );
	}

	virtual void PerformLayout()
	{
		BaseClass::PerformLayout();
		int w, t;
		if ( GetParent() )
		{
			GetParent()->GetSize( w, t );
			SetSize( w, t );
		}
		else
		{
			GetSize( w, t );
		}
		SetPos( 0, 0 );
	}
};

//-----------------------------------------------------------------------------
// Control creation helpers (1:1 with GMod Derma names)
//-----------------------------------------------------------------------------
static vgui::Panel* CreateDFrame( vgui::Panel *pParent, const char *pName )
{
	vgui::Frame *p = new vgui::Frame( pParent, pName ? pName : "DFrame" );
	p->SetSize( 640, 480 );
	p->SetTitle( "Frame", true );
	p->SetVisible( true );
	p->SetMoveable( true );
	p->SetSizeable( true );
	return p;
}

static vgui::Panel* CreateDButton( vgui::Panel *pParent, const char *pName )
{
	vgui::Button *p = new vgui::Button( pParent, pName ? pName : "DButton", "" );
	p->SetCommand( "ButtonCommand" );
	return p;
}

static vgui::Panel* CreateDLabel( vgui::Panel *pParent, const char *pName )
{
	vgui::Label *p = new vgui::Label( pParent, pName ? pName : "DLabel", "" );
	return p;
}

static vgui::Panel* CreateDListView( vgui::Panel *pParent, const char *pName )
{
	vgui::ListPanel *p = new vgui::ListPanel( pParent, pName ? pName : "DListView" );
	return p;
}

static vgui::Panel* CreateDPropertySheet( vgui::Panel *pParent, const char *pName )
{
	vgui::PropertySheet *p = new vgui::PropertySheet( pParent, pName ? pName : "DPropertySheet" );
	return p;
}

static vgui::Panel* CreateDSlider( vgui::Panel *pParent, const char *pName )
{
	vgui::Slider *p = new vgui::Slider( pParent, pName ? pName : "DSlider" );
	p->SetRange( 0, 100 );
	p->SetValue( 0 );
	return p;
}

static vgui::Panel* CreateDPanel( vgui::Panel *pParent, const char *pName )
{
	return new vgui::EditablePanel( pParent, pName ? pName : "DPanel" );
}

//-----------------------------------------------------------------------------
// CDermaManager
//-----------------------------------------------------------------------------
CDermaManager& CDermaManager::Get()
{
	static CDermaManager s_Instance;
	return s_Instance;
}

CDermaManager::CDermaManager() :
	m_pRootPanel( NULL )
{
	Q_strncpy( m_szDefaultSkin, "default", sizeof( m_szDefaultSkin ) );
}

CDermaManager::~CDermaManager()
{
	Shutdown();
}

bool CDermaManager::Init( vgui::VPANEL parent )
{
	// Register built-in controls
	RegisterControl( "DFrame", CreateDFrame );
	RegisterControl( "DButton", CreateDButton );
	RegisterControl( "DLabel", CreateDLabel );
	RegisterControl( "DListView", CreateDListView );
	RegisterControl( "DPropertySheet", CreateDPropertySheet );
	RegisterControl( "DSlider", CreateDSlider );
	RegisterControl( "DPanel", CreateDPanel );

	ReloadSkins();
	AttachRoot( parent );
	return true;
}

void CDermaManager::Shutdown()
{
	DetachRoot();

	for ( int i = 0; i < m_Skins.Count(); ++i )
	{
		delete m_Skins[i].pSkin;
	}
	m_Skins.RemoveAll();
	m_ControlFactories.RemoveAll();
}

void CDermaManager::ReloadSkins()
{
	for ( int i = 0; i < m_Skins.Count(); ++i )
	{
		delete m_Skins[i].pSkin;
	}
	m_Skins.RemoveAll();

	// Enumerate resource/skins
	FileFindHandle_t handle;
	const char *pFile = filesystem->FindFirst( "resource/skins/*.res", &handle );
	while ( pFile )
	{
		char fullPath[MAX_PATH];
		Q_snprintf( fullPath, sizeof( fullPath ), "resource/skins/%s", pFile );

		CDermaSkin *pSkin = new CDermaSkin();
		if ( pSkin->LoadFromFile( fullPath ) )
		{
			SkinEntry_t entry;
			entry.pSkin = pSkin;
			entry.sName = pFile;
			m_Skins.AddToTail( entry );
		}
		else
		{
			delete pSkin;
		}

		pFile = filesystem->FindNext( handle );
	}
	filesystem->FindClose( handle );
}

void CDermaManager::RegisterControl( const char *pszClassName, DermaCreateFn fn )
{
	ControlFactory_t entry;
	entry.fn = fn;
	entry.sClass = pszClassName;
	m_ControlFactories.AddToTail( entry );
}

vgui::Panel* CDermaManager::CreatePanel( const char *pszClassName, vgui::Panel *pParent, const char *pName )
{
	if ( !pszClassName )
		return NULL;

	CUtlSymbol sym( pszClassName );
	for ( int i = 0; i < m_ControlFactories.Count(); ++i )
	{
		if ( m_ControlFactories[i].sClass == sym )
		{
			return m_ControlFactories[i].fn( pParent, pName );
		}
	}
	return NULL;
}

CDermaSkin* CDermaManager::GetSkin( const char *pszSkinName )
{
	if ( !pszSkinName || !pszSkinName[0] )
		return GetDefaultSkin();

	CUtlSymbol sym( pszSkinName );
	for ( int i = 0; i < m_Skins.Count(); ++i )
	{
		if ( m_Skins[i].sName == sym )
			return m_Skins[i].pSkin;
	}
	return GetDefaultSkin();
}

CDermaSkin* CDermaManager::GetDefaultSkin()
{
	return GetSkin( m_szDefaultSkin );
}

void CDermaManager::ApplySkinToPanel( vgui::Panel *pPanel, const char *pszSkin )
{
	if ( !pPanel )
		return;

	CDermaSkin *pSkin = GetSkin( pszSkin );
	if ( !pSkin )
		return;

	Color bg = pSkin->GetColor( "Colors/Panel/Bg", pPanel->GetBgColor() );
	Color fg = pSkin->GetColor( "Colors/Panel/Fg", pPanel->GetFgColor() );
	pPanel->SetBgColor( bg );
	pPanel->SetFgColor( fg );
}

void CDermaManager::AttachRoot( vgui::VPANEL parent )
{
	if ( m_pRootPanel )
		return;

	vgui::Panel *pParent = vgui::ipanel()->GetPanel( parent, "DermaRoot" );
	m_pRootPanel = new CDermaRootPanel( pParent, "DermaRoot" );
	m_pRootPanel->SetProportional( true );
	m_pRootPanel->SetVisible( true );
	m_pRootPanel->SetPaintBackgroundEnabled( false );
	m_pRootPanel->MoveToFront();
	m_pRootPanel->InvalidateLayout( true, true );
}

void CDermaManager::DetachRoot()
{
	if ( !m_pRootPanel )
		return;

	m_pRootPanel->MarkForDeletion();
	m_pRootPanel = NULL;
}

//-----------------------------------------------------------------------------
// Global helpers
//-----------------------------------------------------------------------------
CDermaSkin* Derma_GetSkin( const char *pszSkinName )
{
	return CDermaManager::Get().GetSkin( pszSkinName );
}

vgui::Panel* Derma_Create( const char *pszClass, vgui::Panel *pParent, const char *pName )
{
	return CDermaManager::Get().CreatePanel( pszClass, pParent, pName );
}

void Derma_ApplySkin( vgui::Panel *pPanel, const char *pszSkin )
{
	CDermaManager::Get().ApplySkinToPanel( pPanel, pszSkin );
}
