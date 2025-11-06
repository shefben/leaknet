//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Command Menu Panel - GMod 9.0.4b compatible VGUI interface
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_COMMANDMENU_H
#define GMOD_COMMANDMENU_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Panel.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ListPanel.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/TextEntry.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "tier1/utlvector.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
namespace vgui
{
    class ImagePanel;
    class Divider;
}

//-----------------------------------------------------------------------------
// Command Menu Item Types
//-----------------------------------------------------------------------------
enum CommandMenuItem_t
{
    COMMAND_TOOL = 0,
    COMMAND_WEAPON,
    COMMAND_ENTITY,
    COMMAND_NPC,
    COMMAND_VEHICLE,
    COMMAND_EFFECT,
    COMMAND_UTILITY,
    COMMAND_SEPARATOR,

    COMMAND_TYPE_COUNT
};

//-----------------------------------------------------------------------------
// Command Menu Item Structure
//-----------------------------------------------------------------------------
struct CommandMenuData_t
{
    CommandMenuItem_t type;
    char name[64];
    char command[128];
    char description[256];
    char icon[64];
    bool enabled;
    int category;
};

//-----------------------------------------------------------------------------
// GMod Command Menu Panel
//-----------------------------------------------------------------------------
class CGModCommandMenu : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CGModCommandMenu, vgui::Frame);

public:
    CGModCommandMenu(vgui::VPANEL parent);
    virtual ~CGModCommandMenu();

    // VGUI overrides
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
    virtual void PerformLayout();
    virtual void OnCommand(const char* command);
    virtual void OnKeyCodePressed(vgui::KeyCode code);
    virtual void OnMessage(const KeyValues *params, vgui::VPANEL fromPanel);

    // Menu management
    void ShowMenu();
    void HideMenu();
    void ToggleMenu();
    bool IsMenuVisible();

    // Command management
    void AddCommand(const CommandMenuData_t& data);
    void RemoveCommand(const char* name);
    void ClearCommands();
    void RefreshCommandList();

    // Category management
    void SetCategory(int category);
    int GetCategory();
    void UpdateCategoryFilter();

    // Search functionality
    void SetSearchFilter(const char* filter);
    void ClearSearchFilter();

    // Static management functions
    static void Initialize();
    static void Shutdown();
    static CGModCommandMenu* GetInstance();
    static void RegisterDefaultCommands();

private:
    // UI Controls
    vgui::ListPanel* m_pCommandList;
    vgui::ComboBox* m_pCategoryCombo;
    vgui::TextEntry* m_pSearchBox;
    vgui::Button* m_pExecuteButton;
    vgui::Button* m_pCloseButton;
    vgui::Label* m_pTitleLabel;
    vgui::Label* m_pDescriptionLabel;
    vgui::ImagePanel* m_pIconPanel;

    // Data
    CUtlVector<CommandMenuData_t> m_Commands;
    char m_szSearchFilter[64];
    int m_iCurrentCategory;
    int m_iSelectedCommand;

    // UI Helper functions
    void CreateControls();
    void LayoutControls();
    void PopulateCommandList();
    void UpdateDescription();
    void ExecuteSelectedCommand();

    // Event handlers
    MESSAGE_FUNC_PARAMS(OnItemSelected, "ItemSelected", data);
    MESSAGE_FUNC_PARAMS(OnItemDoubleClicked, "ItemDoubleClicked", data);
    MESSAGE_FUNC_PARAMS(OnTextChanged, "TextChanged", data);
    MESSAGE_FUNC_PARAMS(OnCategoryChanged, "CategoryChanged", data);

    // Utility functions
    const char* GetCategoryName(int category);
    bool MatchesFilter(const CommandMenuData_t& data);
    void SortCommands();

    static CGModCommandMenu* s_pInstance;
    static bool s_bInitialized;
};

//-----------------------------------------------------------------------------
// Console command handlers
//-----------------------------------------------------------------------------
void CMD_ShowCommandMenu();
void CMD_HideCommandMenu();
void CMD_ToggleCommandMenu();

#endif // GMOD_COMMANDMENU_H