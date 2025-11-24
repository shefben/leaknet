//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Command Menu Panel Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_commandmenu.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Divider.h"
#include "vgui/IInput.h"
#include "vgui/IVGui.h"
#include "KeyValues.h"
#include "ienginevgui.h"

using namespace vgui;

// Static members
CGModCommandMenu* CGModCommandMenu::s_pInstance = NULL;
bool CGModCommandMenu::s_bInitialized = false;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGModCommandMenu::CGModCommandMenu(VPANEL parent) : BaseClass(NULL, "GMod_CommandMenu")
{
    SetParent(parent);
    SetTitle("Command Menu", true);
    SetVisible(false);
    SetSizeable(true);
    SetMoveable(true);
    SetCloseButtonVisible(true);

    // Initialize data
    m_szSearchFilter[0] = 0;
    m_iCurrentCategory = -1; // All categories
    m_iSelectedCommand = -1;

    // Create controls
    CreateControls();

    // Set initial size and position
    SetSize(500, 400);
    SetPos(100, 100);

    // Load scheme
    vgui::ivgui()->AddTickSignal(GetVPanel(), false);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CGModCommandMenu::~CGModCommandMenu()
{
    ClearCommands();
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CGModCommandMenu::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetFgColor(pScheme->GetColor("CommandMenu.FgColor", Color(255, 255, 255, 255)));
    SetBgColor(pScheme->GetColor("CommandMenu.BgColor", Color(64, 64, 64, 200)));

    if (m_pTitleLabel)
    {
        m_pTitleLabel->SetFont(pScheme->GetFont("CommandMenu.TitleFont", "DefaultLarge"));
        m_pTitleLabel->SetFgColor(pScheme->GetColor("CommandMenu.TitleColor", Color(255, 255, 255, 255)));
    }

    if (m_pDescriptionLabel)
    {
        m_pDescriptionLabel->SetFont(pScheme->GetFont("CommandMenu.DescFont", "Default"));
        m_pDescriptionLabel->SetFgColor(pScheme->GetColor("CommandMenu.DescColor", Color(200, 200, 200, 255)));
    }
}

//-----------------------------------------------------------------------------
// Purpose: Layout controls
//-----------------------------------------------------------------------------
void CGModCommandMenu::PerformLayout()
{
    BaseClass::PerformLayout();
    LayoutControls();
}

//-----------------------------------------------------------------------------
// Purpose: Handle commands
//-----------------------------------------------------------------------------
void CGModCommandMenu::OnCommand(const char* command)
{
    if (Q_stricmp(command, "Execute") == 0)
    {
        ExecuteSelectedCommand();
    }
    else if (Q_stricmp(command, "Close") == 0)
    {
        HideMenu();
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle key presses
//-----------------------------------------------------------------------------
void CGModCommandMenu::OnKeyCodePressed(KeyCode code)
{
    if (code == KEY_ESCAPE)
    {
        HideMenu();
    }
    else if (code == KEY_ENTER)
    {
        ExecuteSelectedCommand();
    }
    else
    {
        BaseClass::OnKeyCodePressed(code);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle messages
//-----------------------------------------------------------------------------
void CGModCommandMenu::OnMessage(KeyValues *params, vgui::VPANEL fromPanel)
{
    BaseClass::OnMessage(params, fromPanel);
}

//-----------------------------------------------------------------------------
// Purpose: Show the menu
//-----------------------------------------------------------------------------
void CGModCommandMenu::ShowMenu()
{
    RefreshCommandList();
    SetVisible(true);
    MoveToFront();
    RequestFocus();

    if (m_pSearchBox)
        m_pSearchBox->RequestFocus();
}

//-----------------------------------------------------------------------------
// Purpose: Hide the menu
//-----------------------------------------------------------------------------
void CGModCommandMenu::HideMenu()
{
    SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose: Toggle menu visibility
//-----------------------------------------------------------------------------
void CGModCommandMenu::ToggleMenu()
{
    if (IsVisible())
        HideMenu();
    else
        ShowMenu();
}

//-----------------------------------------------------------------------------
// Purpose: Check if menu is visible
//-----------------------------------------------------------------------------
bool CGModCommandMenu::IsMenuVisible()
{
    return IsVisible();
}

//-----------------------------------------------------------------------------
// Purpose: Add command to menu
//-----------------------------------------------------------------------------
void CGModCommandMenu::AddCommand(const CommandMenuData_t& data)
{
    CommandMenuData_t newData = data;
    m_Commands.AddToTail(newData);
}

//-----------------------------------------------------------------------------
// Purpose: Remove command from menu
//-----------------------------------------------------------------------------
void CGModCommandMenu::RemoveCommand(const char* name)
{
    for (int i = m_Commands.Count() - 1; i >= 0; i--)
    {
        if (Q_stricmp(m_Commands[i].name, name) == 0)
        {
            m_Commands.Remove(i);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Clear all commands
//-----------------------------------------------------------------------------
void CGModCommandMenu::ClearCommands()
{
    m_Commands.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: Refresh command list display
//-----------------------------------------------------------------------------
void CGModCommandMenu::RefreshCommandList()
{
    PopulateCommandList();
}

//-----------------------------------------------------------------------------
// Purpose: Set category filter
//-----------------------------------------------------------------------------
void CGModCommandMenu::SetCategory(int category)
{
    m_iCurrentCategory = category;
    UpdateCategoryFilter();
}

//-----------------------------------------------------------------------------
// Purpose: Get current category
//-----------------------------------------------------------------------------
int CGModCommandMenu::GetCategory()
{
    return m_iCurrentCategory;
}

//-----------------------------------------------------------------------------
// Purpose: Update category filter
//-----------------------------------------------------------------------------
void CGModCommandMenu::UpdateCategoryFilter()
{
    if (m_pCategoryCombo)
    {
        m_pCategoryCombo->ActivateItem(m_iCurrentCategory + 1); // +1 for "All" option
    }
    PopulateCommandList();
}

//-----------------------------------------------------------------------------
// Purpose: Set search filter
//-----------------------------------------------------------------------------
void CGModCommandMenu::SetSearchFilter(const char* filter)
{
    Q_strncpy(m_szSearchFilter, filter ? filter : "", sizeof(m_szSearchFilter));
    PopulateCommandList();
}

//-----------------------------------------------------------------------------
// Purpose: Clear search filter
//-----------------------------------------------------------------------------
void CGModCommandMenu::ClearSearchFilter()
{
    SetSearchFilter("");
    if (m_pSearchBox)
        m_pSearchBox->SetText("");
}

//-----------------------------------------------------------------------------
// Purpose: Create UI controls
//-----------------------------------------------------------------------------
void CGModCommandMenu::CreateControls()
{
    // Title label
    m_pTitleLabel = new Label(this, "TitleLabel", "Garry's Mod - Command Menu");
    m_pTitleLabel->SetContentAlignment(Label::a_center);

    // Category combo box
    m_pCategoryCombo = new ComboBox(this, "CategoryCombo", 6, false);
    m_pCategoryCombo->AddItem("All", NULL);
    m_pCategoryCombo->AddItem("Tools", NULL);
    m_pCategoryCombo->AddItem("Weapons", NULL);
    m_pCategoryCombo->AddItem("Entities", NULL);
    m_pCategoryCombo->AddItem("NPCs", NULL);
    m_pCategoryCombo->AddItem("Vehicles", NULL);
    m_pCategoryCombo->AddItem("Effects", NULL);
    m_pCategoryCombo->AddItem("Utilities", NULL);
    m_pCategoryCombo->ActivateItem(0);

    // Search box
    m_pSearchBox = new TextEntry(this, "SearchBox");
    // Note: SetPlaceholderText not available in 2003 VGUI - functionality preserved without placeholder

    // Command list
    m_pCommandList = new ListPanel(this, "CommandList");
    m_pCommandList->AddColumnHeader(0, "name", "Command", 200, 0);
    m_pCommandList->AddColumnHeader(1, "description", "Description", 250, 0);
    m_pCommandList->SetSelectIndividualCells(false);
    m_pCommandList->SetEmptyListText("No commands available");
    // Note: SetDragEnabled not available in 2003 VGUI - drag disabled by default in 2003

    // Description label
    m_pDescriptionLabel = new Label(this, "DescriptionLabel", "Select a command to see its description");
    m_pDescriptionLabel->SetContentAlignment(Label::a_northwest);
    // Note: SetWrap not available in 2003 VGUI - text wrapping handled differently in 2003

    // Icon panel
    m_pIconPanel = new ImagePanel(this, "IconPanel");
    // Note: SetImage with string path not available in 2003 VGUI - would need IImage* object

    // Buttons - 2003 VGUI style (3 parameters only)
    m_pExecuteButton = new Button(this, "ExecuteButton", "Execute");
    m_pCloseButton = new Button(this, "CloseButton", "Close");

    // Set up event handling
    m_pCommandList->AddActionSignalTarget(this);
    m_pCategoryCombo->AddActionSignalTarget(this);
    m_pSearchBox->AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: Layout controls on the panel
//-----------------------------------------------------------------------------
void CGModCommandMenu::LayoutControls()
{
    int wide, tall;
    GetSize(wide, tall);

    int margin = 10;
    int buttonHeight = 24;
    int comboHeight = 20;

    // Title
    if (m_pTitleLabel)
        m_pTitleLabel->SetBounds(margin, margin, wide - margin * 2, 20);

    // Category and search controls
    int ypos = margin + 30;
    if (m_pCategoryCombo)
        m_pCategoryCombo->SetBounds(margin, ypos, 120, comboHeight);

    if (m_pSearchBox)
        m_pSearchBox->SetBounds(margin + 130, ypos, wide - margin * 2 - 130, comboHeight);

    // Command list
    ypos += comboHeight + margin;
    int listHeight = tall - ypos - 80 - margin * 2;
    if (m_pCommandList)
        m_pCommandList->SetBounds(margin, ypos, wide - margin * 2, listHeight);

    // Description
    ypos += listHeight + margin;
    if (m_pDescriptionLabel)
        m_pDescriptionLabel->SetBounds(margin, ypos, wide - margin * 2 - 40, 40);

    // Icon
    if (m_pIconPanel)
        m_pIconPanel->SetBounds(wide - margin - 32, ypos, 32, 32);

    // Buttons
    ypos += 50;
    if (m_pExecuteButton)
        m_pExecuteButton->SetBounds(margin, ypos, 80, buttonHeight);

    if (m_pCloseButton)
        m_pCloseButton->SetBounds(wide - margin - 80, ypos, 80, buttonHeight);
}

//-----------------------------------------------------------------------------
// Purpose: Populate the command list
//-----------------------------------------------------------------------------
void CGModCommandMenu::PopulateCommandList()
{
    if (!m_pCommandList)
        return;

    m_pCommandList->DeleteAllItems();

    for (int i = 0; i < m_Commands.Count(); i++)
    {
        const CommandMenuData_t& cmd = m_Commands[i];

        // Check filters
        if (!MatchesFilter(cmd))
            continue;

        if (!cmd.enabled)
            continue;

        // Add to list - 2003 ListPanel uses KeyValues data directly
        KeyValues *data = new KeyValues("item");
        data->SetString("name", cmd.name);
        data->SetString("description", cmd.description);
        data->SetInt("index", i);

        int itemID = m_pCommandList->AddItem(data, 0, false, false);
        // Note: SetItemText not available in 2003 - data comes from KeyValues

        data->deleteThis();
    }

    m_pCommandList->SortList();
}

//-----------------------------------------------------------------------------
// Purpose: Update description for selected command
//-----------------------------------------------------------------------------
void CGModCommandMenu::UpdateDescription()
{
    if (!m_pDescriptionLabel)
        return;

    if (m_iSelectedCommand >= 0 && m_iSelectedCommand < m_Commands.Count())
    {
        const CommandMenuData_t& cmd = m_Commands[m_iSelectedCommand];
        m_pDescriptionLabel->SetText(cmd.description);
    }
    else
    {
        m_pDescriptionLabel->SetText("Select a command to see its description");
    }
}

//-----------------------------------------------------------------------------
// Purpose: Execute the selected command
//-----------------------------------------------------------------------------
void CGModCommandMenu::ExecuteSelectedCommand()
{
    if (m_iSelectedCommand >= 0 && m_iSelectedCommand < m_Commands.Count())
    {
        const CommandMenuData_t& cmd = m_Commands[m_iSelectedCommand];
        engine->ClientCmd(cmd.command);
        HideMenu();
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle item selection
//-----------------------------------------------------------------------------
// Purpose: Handle item selection - 2003 VGUI style (no parameters)
//-----------------------------------------------------------------------------
void CGModCommandMenu::OnItemSelected()
{
    if (!m_pCommandList)
        return;

    // Get selected item in 2003 VGUI ListPanel style
    int selectedRow = m_pCommandList->GetSelectedItem(0);
    if (selectedRow >= 0)
    {
        int itemID = m_pCommandList->GetItemIDFromRow(selectedRow);
        if (itemID >= 0)
        {
            KeyValues* itemData = m_pCommandList->GetItem(itemID);
            if (itemData)
            {
                m_iSelectedCommand = itemData->GetInt("index");
                UpdateDescription();
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle item double click
//-----------------------------------------------------------------------------
void CGModCommandMenu::OnItemDoubleClicked()
{
    OnItemSelected();
    ExecuteSelectedCommand();
}

//-----------------------------------------------------------------------------
// Purpose: Handle search text change
//-----------------------------------------------------------------------------
void CGModCommandMenu::OnTextChanged()
{
    if (m_pSearchBox)
    {
        char text[64];
        m_pSearchBox->GetText(text, sizeof(text));
        SetSearchFilter(text);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle category change
//-----------------------------------------------------------------------------
void CGModCommandMenu::OnCategoryChanged()
{
    if (m_pCategoryCombo)
    {
        int selected = m_pCategoryCombo->GetActiveItem();
        SetCategory(selected - 1); // -1 for "All" option
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get category name
//-----------------------------------------------------------------------------
const char* CGModCommandMenu::GetCategoryName(int category)
{
    switch (category)
    {
        case COMMAND_TOOL: return "Tools";
        case COMMAND_WEAPON: return "Weapons";
        case COMMAND_ENTITY: return "Entities";
        case COMMAND_NPC: return "NPCs";
        case COMMAND_VEHICLE: return "Vehicles";
        case COMMAND_EFFECT: return "Effects";
        case COMMAND_UTILITY: return "Utilities";
        default: return "Unknown";
    }
}

//-----------------------------------------------------------------------------
// Purpose: Check if command matches current filters
//-----------------------------------------------------------------------------
bool CGModCommandMenu::MatchesFilter(const CommandMenuData_t& data)
{
    // Category filter
    if (m_iCurrentCategory >= 0 && data.category != m_iCurrentCategory)
        return false;

    // Search filter
    if (m_szSearchFilter[0] != 0)
    {
        if (!Q_stristr(data.name, m_szSearchFilter) &&
            !Q_stristr(data.description, m_szSearchFilter))
            return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Sort commands alphabetically
//-----------------------------------------------------------------------------
void CGModCommandMenu::SortCommands()
{
    // Simple bubble sort for commands
    for (int i = 0; i < m_Commands.Count() - 1; i++)
    {
        for (int j = 0; j < m_Commands.Count() - 1 - i; j++)
        {
            if (Q_stricmp(m_Commands[j].name, m_Commands[j + 1].name) > 0)
            {
                CommandMenuData_t temp = m_Commands[j];
                m_Commands[j] = m_Commands[j + 1];
                m_Commands[j + 1] = temp;
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Initialize command menu system
//-----------------------------------------------------------------------------
void CGModCommandMenu::Initialize()
{
    if (s_bInitialized)
        return;

    // Create the global instance
    VPANEL gameToolParent = enginevgui->GetPanel(PANEL_CLIENTDLL);
    s_pInstance = new CGModCommandMenu(gameToolParent);

    RegisterDefaultCommands();

    s_bInitialized = true;
    DevMsg("Command Menu System initialized\n");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown command menu system
//-----------------------------------------------------------------------------
void CGModCommandMenu::Shutdown()
{
    if (!s_bInitialized)
        return;

    if (s_pInstance)
    {
        s_pInstance->MarkForDeletion();
        s_pInstance = NULL;
    }

    s_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Get global instance
//-----------------------------------------------------------------------------
CGModCommandMenu* CGModCommandMenu::GetInstance()
{
    return s_pInstance;
}

//-----------------------------------------------------------------------------
// Purpose: Register default commands
//-----------------------------------------------------------------------------
void CGModCommandMenu::RegisterDefaultCommands()
{
    if (!s_pInstance)
        return;

    CommandMenuData_t cmd;

    // Tools
    Q_strncpy(cmd.name, "Physics Gun", sizeof(cmd.name));
    Q_strncpy(cmd.command, "gm_tool physgun", sizeof(cmd.command));
    Q_strncpy(cmd.description, "Grab and manipulate objects", sizeof(cmd.description));
    cmd.type = COMMAND_TOOL;
    cmd.category = COMMAND_TOOL;
    cmd.enabled = true;
    s_pInstance->AddCommand(cmd);

    Q_strncpy(cmd.name, "Tool Gun", sizeof(cmd.name));
    Q_strncpy(cmd.command, "gm_tool toolgun", sizeof(cmd.command));
    Q_strncpy(cmd.description, "Advanced building and welding tool", sizeof(cmd.description));
    s_pInstance->AddCommand(cmd);

    // Entities
    Q_strncpy(cmd.name, "Spawn Prop", sizeof(cmd.name));
    Q_strncpy(cmd.command, "gm_spawn prop_physics", sizeof(cmd.command));
    Q_strncpy(cmd.description, "Spawn a physics prop", sizeof(cmd.description));
    cmd.type = COMMAND_ENTITY;
    cmd.category = COMMAND_ENTITY;
    s_pInstance->AddCommand(cmd);

    Q_strncpy(cmd.name, "Spawn Balloon", sizeof(cmd.name));
    Q_strncpy(cmd.command, "gm_spawn gmod_balloon", sizeof(cmd.command));
    Q_strncpy(cmd.description, "Spawn a balloon entity", sizeof(cmd.description));
    s_pInstance->AddCommand(cmd);

    // Effects
    Q_strncpy(cmd.name, "Spawn Emitter", sizeof(cmd.name));
    Q_strncpy(cmd.command, "gm_emitter_spawn", sizeof(cmd.command));
    Q_strncpy(cmd.description, "Spawn a particle emitter", sizeof(cmd.description));
    cmd.type = COMMAND_EFFECT;
    cmd.category = COMMAND_EFFECT;
    s_pInstance->AddCommand(cmd);

    Q_strncpy(cmd.name, "Spawn Dynamite", sizeof(cmd.name));
    Q_strncpy(cmd.command, "gm_dynamite_spawn", sizeof(cmd.command));
    Q_strncpy(cmd.description, "Spawn explosive dynamite", sizeof(cmd.description));
    s_pInstance->AddCommand(cmd);

    // Utilities
    Q_strncpy(cmd.name, "Remove All", sizeof(cmd.name));
    Q_strncpy(cmd.command, "gm_remove_all", sizeof(cmd.command));
    Q_strncpy(cmd.description, "Remove all entities from the map", sizeof(cmd.description));
    cmd.type = COMMAND_UTILITY;
    cmd.category = COMMAND_UTILITY;
    s_pInstance->AddCommand(cmd);

    Q_strncpy(cmd.name, "Cleanup", sizeof(cmd.name));
    Q_strncpy(cmd.command, "gm_cleanup", sizeof(cmd.command));
    Q_strncpy(cmd.description, "Clean up your entities", sizeof(cmd.description));
    s_pInstance->AddCommand(cmd);

    s_pInstance->SortCommands();
}

//-----------------------------------------------------------------------------
// Console command handlers
//-----------------------------------------------------------------------------
void CMD_ShowCommandMenu()
{
    CGModCommandMenu* pMenu = CGModCommandMenu::GetInstance();
    if (pMenu)
        pMenu->ShowMenu();
}

void CMD_HideCommandMenu()
{
    CGModCommandMenu* pMenu = CGModCommandMenu::GetInstance();
    if (pMenu)
        pMenu->HideMenu();
}

void CMD_ToggleCommandMenu()
{
    CGModCommandMenu* pMenu = CGModCommandMenu::GetInstance();
    if (pMenu)
        pMenu->ToggleMenu();
}

// Register console commands
static ConCommand gm_commandmenu_show("gm_commandmenu_show", CMD_ShowCommandMenu, "Show command menu");
static ConCommand gm_commandmenu_hide("gm_commandmenu_hide", CMD_HideCommandMenu, "Hide command menu");
static ConCommand gm_commandmenu_toggle("gm_commandmenu_toggle", CMD_ToggleCommandMenu, "Toggle command menu");
static ConCommand gm_commandmenu("+gm_commandmenu", CMD_ShowCommandMenu, "Show command menu");

//-----------------------------------------------------------------------------
// Client initialization hook
//-----------------------------------------------------------------------------
class CCommandMenuInit : public CAutoGameSystem
{
public:
    CCommandMenuInit() : CAutoGameSystem("CommandMenuInit") {}

    virtual bool Init()
    {
        CGModCommandMenu::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModCommandMenu::Shutdown();
    }
};

static CCommandMenuInit g_CommandMenuInit;