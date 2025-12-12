//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Stub VGui screen implementations for GMod 9 compatibility
//          Provides panel factories for teleport_countdown_screen and slideshow_display_screen
//
//=============================================================================//

#include "cbase.h"
#include "c_vguiscreen.h"
#include <vgui/IVGui.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Teleport Countdown Screen - displays countdown timer during teleportation
//-----------------------------------------------------------------------------
class CTeleportCountdownScreen : public CVGuiScreenPanel
{
	DECLARE_CLASS( CTeleportCountdownScreen, CVGuiScreenPanel );

public:
	CTeleportCountdownScreen( vgui::Panel *parent, const char *panelName );

	virtual bool Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData );
	virtual void OnTick();

private:
	vgui::Label *m_pCountdownLabel;
	float m_flCountdownEnd;
};

DECLARE_VGUI_SCREEN_FACTORY( CTeleportCountdownScreen, "teleport_countdown_screen" );

CTeleportCountdownScreen::CTeleportCountdownScreen( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_pCountdownLabel = NULL;
	m_flCountdownEnd = 0.0f;
}

bool CTeleportCountdownScreen::Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData )
{
	if (!BaseClass::Init(pKeyValues, pInitData))
		return false;

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_pCountdownLabel = dynamic_cast<vgui::Label*>(FindChildByName( "CountdownLabel" ));

	return true;
}

void CTeleportCountdownScreen::OnTick()
{
	BaseClass::OnTick();

	if (!GetEntity())
		return;

	// Update countdown display if label exists
	if (m_pCountdownLabel)
	{
		char buf[32];
		float remaining = max( 0.0f, m_flCountdownEnd - gpGlobals->curtime );
		Q_snprintf( buf, sizeof(buf), "%.1f", remaining );
		m_pCountdownLabel->SetText( buf );
	}
}

//-----------------------------------------------------------------------------
// Slideshow Display Screen - displays rotating images
//-----------------------------------------------------------------------------
class CSlideshowDisplayScreen : public CVGuiScreenPanel
{
	DECLARE_CLASS( CSlideshowDisplayScreen, CVGuiScreenPanel );

public:
	CSlideshowDisplayScreen( vgui::Panel *parent, const char *panelName );

	virtual bool Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData );
	virtual void OnTick();

private:
	vgui::ImagePanel *m_pImagePanel;
	float m_flNextSlideTime;
	int m_nCurrentSlide;
};

DECLARE_VGUI_SCREEN_FACTORY( CSlideshowDisplayScreen, "slideshow_display_screen" );

CSlideshowDisplayScreen::CSlideshowDisplayScreen( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_pImagePanel = NULL;
	m_flNextSlideTime = 0.0f;
	m_nCurrentSlide = 0;
}

bool CSlideshowDisplayScreen::Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData )
{
	if (!BaseClass::Init(pKeyValues, pInitData))
		return false;

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_pImagePanel = dynamic_cast<vgui::ImagePanel*>(FindChildByName( "SlideshowImage" ));

	return true;
}

void CSlideshowDisplayScreen::OnTick()
{
	BaseClass::OnTick();

	if (!GetEntity())
		return;

	// Slideshow logic would go here if needed
	// For now this is just a stub to prevent "MetaClass missing" errors
}
