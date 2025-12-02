//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "NewGameDialog.h"

#include "EngineInterface.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/CheckButton.h>
#include <KeyValues.h>
#include <vgui_controls/Label.h>
#include <vgui/ISurface.h>
#include <vgui_controls/RadioButton.h>
#include <stdio.h>
#include "ModInfo.h"
#include "FileSystem.h"

using namespace vgui;

#include "GameUI_Interface.h"
#include "taskframe.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CNewGameDialog::CNewGameDialog(vgui::Panel *parent) : CTaskFrame(parent, "NewGameDialog")
{
	SetBounds(0, 0, 372, 160);
	SetSizeable( false );

	MakePopup();
	g_pTaskbar->AddTask(GetVPanel());

	//vgui::surface()->CreatePopup( GetVPanel(), false );

	SetTitle("#GameUI_NewGame", true);

	new vgui::Label( this, "HelpText", "#GameUI_NewGameHelpText" );

	// radio buttons to tab between controls
	m_pTraining = new vgui::RadioButton(this, "Training", "#GameUI_TrainingRoom");
	m_pTraining->SetSelected( true );
	m_nPlayMode = 0;
	m_pEasy = new vgui::RadioButton(this, "Easy", "#GameUI_Easy");
	m_pMedium = new vgui::RadioButton(this, "Medium", "#GameUI_Medium");
	m_pHard = new vgui::RadioButton(this, "Hard", "#GameUI_Hard");

	vgui::Button *play = new vgui::Button( this, "Play", "#GameUI_Play" );
	play->SetCommand( "Play" );

	vgui::Button *cancel = new vgui::Button( this, "Cancel", "#GameUI_Cancel" );
	cancel->SetCommand( "Close" );

	LoadControlSettings("Resource\\NewGameDialog.res");

	// create KeyValues object to load/save server config options
	m_pServerConfig = new KeyValues( "Server Config" );

	// load the server config data from serverconfig.txt
	LoadServerSettings();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CNewGameDialog::~CNewGameDialog()
{
	if (m_pServerConfig)
	{
		m_pServerConfig->deleteThis();
		m_pServerConfig = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *command - 
//-----------------------------------------------------------------------------
void CNewGameDialog::OnCommand( const char *command )
{
	if ( !_stricmp( command, "Play" ) )
	{
		// Determine mode
		m_nPlayMode = 0;

		if ( m_pEasy->IsSelected() )
		{
			m_nPlayMode = 1;
		}
		else if ( m_pMedium->IsSelected() )
		{
			m_nPlayMode = 2;
		}
		else if ( m_pHard->IsSelected() )
		{
			m_nPlayMode = 3;
		}

		char mapcommand[ 512 ];
		mapcommand[ 0 ] = 0;

		// Find appropriate key
		if ( m_nPlayMode == 0 )
		{
			sprintf(mapcommand, "disconnect\nmaxplayers 1\ndeathmatch 0\nmap %s\n", ModInfo().GetTrainMap() );
		}
		else
		{
			sprintf(mapcommand, "disconnect\nmaxplayers 1\ndeathmatch 0\nskill %i\nmap %s\n", m_nPlayMode, ModInfo().GetStartMap() );
		}

		// Save current difficulty setting to server config before starting the game
		SaveServerSettings();

		engine->ClientCmd( mapcommand );
		OnClose();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNewGameDialog::OnClose()
{
	BaseClass::OnClose();
	MarkForDeletion();
}

//-----------------------------------------------------------------------------
// Purpose: Loads server settings from serverconfig.txt
//-----------------------------------------------------------------------------
void CNewGameDialog::LoadServerSettings()
{
	if (!m_pServerConfig)
		return;

	// load the server config data from serverconfig.txt
	m_pServerConfig->LoadFromFile( filesystem(), "cfg/serverconfig.txt", "MOD" );
	if (!m_pServerConfig->GetFirstSubKey())
	{
		// File doesn't exist or is empty, try from HL2 directory
		m_pServerConfig->LoadFromFile( filesystem(), "cfg/serverconfig.txt", "HL2" );
	}

	// Load difficulty setting (skill level)
	int savedSkill = m_pServerConfig->GetInt("skill", 1); // default to easy
	m_nPlayMode = savedSkill;

	// Set the appropriate radio button based on saved skill level
	if (savedSkill == 0)
	{
		m_pTraining->SetSelected(true);
	}
	else if (savedSkill == 1)
	{
		m_pEasy->SetSelected(true);
	}
	else if (savedSkill == 2)
	{
		m_pMedium->SetSelected(true);
	}
	else if (savedSkill >= 3)
	{
		m_pHard->SetSelected(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Saves server settings to serverconfig.txt
//-----------------------------------------------------------------------------
void CNewGameDialog::SaveServerSettings()
{
	if (!m_pServerConfig)
		return;

	// Update server config with current difficulty setting
	m_pServerConfig->SetInt("skill", m_nPlayMode);

	// Save to file in MOD directory first, fall back to HL2 if needed
	bool saved = m_pServerConfig->SaveToFile( filesystem(), "cfg/serverconfig.txt", "MOD" );
	if (!saved)
	{
		m_pServerConfig->SaveToFile( filesystem(), "cfg/serverconfig.txt", "HL2" );
	}
}

