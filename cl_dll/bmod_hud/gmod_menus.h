#ifndef GMOD_MENUS_H
#define GMOD_MENUS_H

#pragma once

#include "vgui_controls/Frame.h"

namespace vgui
{
class ListPanel;
class Button;
class TextEntry;
class CheckButton;
class RichText;
} // namespace vgui

// Simple dialogs that replicate the original GMod 9 main menu helpers.
class CGModModManagerDialog : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CGModModManagerDialog, vgui::Frame );

public:
	explicit CGModModManagerDialog( vgui::VPANEL parent );

	void ActivateAndRefresh();

protected:
	virtual void OnCommand( const char *command );

private:
	void PopulateModList();
	void EnableDisableSelected();
	void AdjustLayout();

	vgui::ListPanel *m_pList;
	vgui::Button *m_pToggleButton;
	vgui::RichText *m_pDetails;
};

class CGModBaseServerDialog : public vgui::Frame
{
public:
	DECLARE_CLASS_SIMPLE( CGModBaseServerDialog, vgui::Frame );

	explicit CGModBaseServerDialog( vgui::VPANEL parent, bool multiplayer );

	void ActivateAndRefresh();

protected:
	virtual void OnCommand( const char *command );
	void AdjustLayout();

	void PopulateMapList();
	const char *GetSelectedMap() const;
	void ApplyCommonCvars() const;

	vgui::ListPanel *m_pMapList;
	vgui::TextEntry *m_pHostname;
	vgui::TextEntry *m_pPassword;
	vgui::TextEntry *m_pMaxPlayers;

	vgui::CheckButton *m_pAllWeapons;
	vgui::CheckButton *m_pAllowIgnite;
	vgui::CheckButton *m_pNoClip;
	vgui::CheckButton *m_pPlayerDamage;
	vgui::CheckButton *m_pPvPDamage;
	vgui::CheckButton *m_pTeamDamage;
	vgui::CheckButton *m_pAllowNPCs;
	vgui::CheckButton *m_pAllowSpawning;
	vgui::CheckButton *m_pAllowMultiGun;
	vgui::CheckButton *m_pAllowPhysgun;

	vgui::TextEntry *m_pLimitRagdolls;
	vgui::TextEntry *m_pLimitThrusters;
	vgui::TextEntry *m_pLimitProps;
	vgui::TextEntry *m_pLimitBalloons;
	vgui::TextEntry *m_pLimitEffects;
	vgui::TextEntry *m_pLimitSprites;
	vgui::TextEntry *m_pLimitEmitters;
	vgui::TextEntry *m_pLimitWheels;
	vgui::TextEntry *m_pLimitNPCs;
	vgui::TextEntry *m_pLimitVehicles;

	bool m_bMultiplayer;
};

class CGModMultiplayerDialog : public CGModBaseServerDialog
{
	DECLARE_CLASS_SIMPLE( CGModMultiplayerDialog, CGModBaseServerDialog );

public:
	explicit CGModMultiplayerDialog( vgui::VPANEL parent );

protected:
	virtual void OnCommand( const char *command );
};

class CGModSingleplayerDialog : public CGModBaseServerDialog
{
	DECLARE_CLASS_SIMPLE( CGModSingleplayerDialog, CGModBaseServerDialog );

public:
	explicit CGModSingleplayerDialog( vgui::VPANEL parent );

protected:
	virtual void OnCommand( const char *command );
};

void ShowModMenu();
void ShowMPMenu();
void ShowSPMenu();

#endif // GMOD_MENUS_H
