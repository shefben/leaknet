//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Team Selection Menu - GMod 9.0.4b compatible VGUI interface
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_TEAMMENU_H
#define GMOD_TEAMMENU_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Panel.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ListPanel.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "tier1/utlvector.h"
#include "c_team.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
namespace vgui
{
    class Divider;
}

//-----------------------------------------------------------------------------
// Team Information Structure
//-----------------------------------------------------------------------------
struct TeamMenuData_t
{
    int teamNumber;
    char teamName[64];
    char description[256];
    Color teamColor;
    int playerCount;
    int maxPlayers;
    bool canJoin;
    char modelPath[128];
};

//-----------------------------------------------------------------------------
// GMod Team Menu Panel
//-----------------------------------------------------------------------------
class CGModTeamMenu : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CGModTeamMenu, vgui::Frame);

public:
    CGModTeamMenu(vgui::VPANEL parent);
    virtual ~CGModTeamMenu();

    // VGUI overrides
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void PerformLayout();
    virtual void OnCommand(const char* command);
    virtual void OnKeyCodePressed(vgui::KeyCode code);
    virtual void OnMessage(const KeyValues *params, vgui::VPANEL fromPanel);
    virtual void OnTick();

    // Menu management
    void ShowMenu();
    void HideMenu();
    void ToggleMenu();
    bool IsMenuVisible();

    // Team management
    void RefreshTeamList();
    void UpdateTeamInfo();
    void JoinSelectedTeam();
    void JoinTeam(int teamNumber);
    void SetAutoAssign(bool autoAssign);

    // Display functions
    void UpdatePlayerCounts();
    void UpdateTeamDescription();

    // Static management functions
    static void Initialize();
    static void Shutdown();
    static CGModTeamMenu* GetInstance();

private:
    // UI Controls
    vgui::Label* m_pTitleLabel;
    vgui::Label* m_pInstructionLabel;
    vgui::ListPanel* m_pTeamList;
    vgui::Label* m_pTeamNameLabel;
    vgui::Label* m_pTeamDescLabel;
    vgui::Label* m_pPlayerCountLabel;
    vgui::ImagePanel* m_pTeamLogoPanel;
    vgui::Button* m_pJoinButton;
    vgui::Button* m_pAutoAssignButton;
    vgui::Button* m_pSpectateButton;
    vgui::Button* m_pCancelButton;

    // Data
    CUtlVector<TeamMenuData_t> m_Teams;
    int m_iSelectedTeam;
    int m_iCurrentPlayerTeam;
    bool m_bAutoAssign;

    // UI Helper functions
    void CreateControls();
    void LayoutControls();
    void PopulateTeamList();
    void SelectTeam(int teamNumber);
    void UpdateSelectedTeamInfo();

    // Team data management
    void BuildTeamList();
    void GetTeamData(int teamNumber, TeamMenuData_t& data);
    int GetPlayerCountForTeam(int teamNumber);
    bool CanJoinTeam(int teamNumber);

    // Event handlers
    MESSAGE_FUNC_PARAMS(OnItemSelected, "ItemSelected", data);
    MESSAGE_FUNC_PARAMS(OnItemDoubleClicked, "ItemDoubleClicked", data);

    // Utility functions
    Color GetTeamColor(int teamNumber);
    const char* GetTeamDescription(int teamNumber);

    static CGModTeamMenu* s_pInstance;
    static bool s_bInitialized;
};

//-----------------------------------------------------------------------------
// Console command handlers
//-----------------------------------------------------------------------------
void CMD_ShowTeamMenu();
void CMD_HideTeamMenu();
void CMD_ToggleTeamMenu();
void CMD_AutoAssign();
void CMD_JoinTeam();
void CMD_Spectate();

#endif // GMOD_TEAMMENU_H