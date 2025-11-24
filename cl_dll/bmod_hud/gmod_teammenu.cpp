//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Team Selection Menu Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_teammenu.h"
#include "vgui_controls/Divider.h"
#include "vgui/IInput.h"
#include "vgui/IVGui.h"
#include "KeyValues.h"
#include "c_playerresource.h"
#include "gamerules.h"
#include "c_baseplayer.h"
#include "cliententitylist.h"
#include "ienginevgui.h"

using namespace vgui;

// Static members
CGModTeamMenu* CGModTeamMenu::s_pInstance = NULL;
bool CGModTeamMenu::s_bInitialized = false;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGModTeamMenu::CGModTeamMenu(VPANEL parent) : BaseClass(NULL, "GMod_TeamMenu")
{
    SetParent(parent);
    SetTitle("Team Selection", true);
    SetVisible(false);
    SetSizeable(false);
    SetMoveable(true);
    SetCloseButtonVisible(true);

    // Initialize data
    m_iSelectedTeam = -1;
    m_iCurrentPlayerTeam = TEAM_UNASSIGNED;
    m_bAutoAssign = false;

    // Create controls
    CreateControls();

    // Set initial size and position
    SetSize(450, 350);
    MoveToCenterOfScreen();

    // Load scheme
    vgui::ivgui()->AddTickSignal(GetVPanel(), false);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CGModTeamMenu::~CGModTeamMenu()
{
    m_Teams.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CGModTeamMenu::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetFgColor(pScheme->GetColor("TeamMenu.FgColor", Color(255, 255, 255, 255)));
    SetBgColor(pScheme->GetColor("TeamMenu.BgColor", Color(32, 32, 32, 240)));

    if (m_pTitleLabel)
    {
        m_pTitleLabel->SetFont(pScheme->GetFont("TeamMenu.TitleFont", "DefaultLarge"));
        m_pTitleLabel->SetFgColor(pScheme->GetColor("TeamMenu.TitleColor", Color(255, 255, 255, 255)));
    }

    if (m_pInstructionLabel)
    {
        m_pInstructionLabel->SetFont(pScheme->GetFont("TeamMenu.InstructFont", "Default"));
        m_pInstructionLabel->SetFgColor(pScheme->GetColor("TeamMenu.InstructColor", Color(200, 200, 200, 255)));
    }
}

//-----------------------------------------------------------------------------
// Purpose: Layout controls
//-----------------------------------------------------------------------------
void CGModTeamMenu::PerformLayout()
{
    BaseClass::PerformLayout();
    LayoutControls();
}

//-----------------------------------------------------------------------------
// Purpose: Handle commands
//-----------------------------------------------------------------------------
void CGModTeamMenu::OnCommand(const char* command)
{
    if (Q_stricmp(command, "Join") == 0)
    {
        JoinSelectedTeam();
    }
    else if (Q_stricmp(command, "AutoAssign") == 0)
    {
        engine->ClientCmd("jointeam 0\n");
        HideMenu();
    }
    else if (Q_stricmp(command, "Spectate") == 0)
    {
        engine->ClientCmd("spectate\n");
        HideMenu();
    }
    else if (Q_stricmp(command, "Cancel") == 0)
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
void CGModTeamMenu::OnKeyCodePressed(KeyCode code)
{
    if (code == KEY_ESCAPE)
    {
        HideMenu();
    }
    else if (code >= KEY_1 && code <= KEY_9)
    {
        int teamNum = code - KEY_1 + 1;
        if (teamNum < m_Teams.Count())
        {
            JoinTeam(m_Teams[teamNum].teamNumber);
        }
    }
    else
    {
        BaseClass::OnKeyCodePressed(code);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle messages
//-----------------------------------------------------------------------------
void CGModTeamMenu::OnMessage(KeyValues *params, vgui::VPANEL fromPanel)
{
    BaseClass::OnMessage(params, fromPanel);
}

//-----------------------------------------------------------------------------
// Purpose: Tick function for updates
//-----------------------------------------------------------------------------
void CGModTeamMenu::OnTick()
{
    BaseClass::OnTick();

    if (!IsVisible())
        return;

    // Update player counts periodically
    static float lastUpdate = 0.0f;
    if (gpGlobals->curtime > lastUpdate + 1.0f)
    {
        UpdatePlayerCounts();
        lastUpdate = gpGlobals->curtime;
    }

    // Update current player team
    C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
    if (pLocalPlayer)
    {
        int newTeam = pLocalPlayer->GetTeamNumber();
        if (newTeam != m_iCurrentPlayerTeam)
        {
            m_iCurrentPlayerTeam = newTeam;
            RefreshTeamList();
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Show the menu
//-----------------------------------------------------------------------------
void CGModTeamMenu::ShowMenu()
{
    RefreshTeamList();
    SetVisible(true);
    MoveToFront();
    RequestFocus();
}

//-----------------------------------------------------------------------------
// Purpose: Hide the menu
//-----------------------------------------------------------------------------
void CGModTeamMenu::HideMenu()
{
    SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose: Toggle menu visibility
//-----------------------------------------------------------------------------
void CGModTeamMenu::ToggleMenu()
{
    if (IsVisible())
        HideMenu();
    else
        ShowMenu();
}

//-----------------------------------------------------------------------------
// Purpose: Check if menu is visible
//-----------------------------------------------------------------------------
bool CGModTeamMenu::IsMenuVisible()
{
    return IsVisible();
}

//-----------------------------------------------------------------------------
// Purpose: Refresh team list
//-----------------------------------------------------------------------------
void CGModTeamMenu::RefreshTeamList()
{
    BuildTeamList();
    PopulateTeamList();
    UpdateTeamInfo();
}

//-----------------------------------------------------------------------------
// Purpose: Update team information
//-----------------------------------------------------------------------------
void CGModTeamMenu::UpdateTeamInfo()
{
    UpdatePlayerCounts();
    UpdateTeamDescription();
}

//-----------------------------------------------------------------------------
// Purpose: Join the selected team
//-----------------------------------------------------------------------------
void CGModTeamMenu::JoinSelectedTeam()
{
    if (m_iSelectedTeam >= 0 && m_iSelectedTeam < m_Teams.Count())
    {
        JoinTeam(m_Teams[m_iSelectedTeam].teamNumber);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Join a specific team
//-----------------------------------------------------------------------------
void CGModTeamMenu::JoinTeam(int teamNumber)
{
    if (CanJoinTeam(teamNumber))
    {
        char command[64];
        Q_snprintf(command, sizeof(command), "jointeam %d\n", teamNumber);
        engine->ClientCmd(command);
        HideMenu();
    }
}

//-----------------------------------------------------------------------------
// Purpose: Set auto assign mode
//-----------------------------------------------------------------------------
void CGModTeamMenu::SetAutoAssign(bool autoAssign)
{
    m_bAutoAssign = autoAssign;
}

//-----------------------------------------------------------------------------
// Purpose: Update player counts for all teams
//-----------------------------------------------------------------------------
void CGModTeamMenu::UpdatePlayerCounts()
{
    for (int i = 0; i < m_Teams.Count(); i++)
    {
        m_Teams[i].playerCount = GetPlayerCountForTeam(m_Teams[i].teamNumber);
    }

    // Update the list display
    PopulateTeamList();
}

//-----------------------------------------------------------------------------
// Purpose: Update team description display
//-----------------------------------------------------------------------------
void CGModTeamMenu::UpdateTeamDescription()
{
    if (m_iSelectedTeam >= 0 && m_iSelectedTeam < m_Teams.Count())
    {
        UpdateSelectedTeamInfo();
    }
}

//-----------------------------------------------------------------------------
// Purpose: Create UI controls
//-----------------------------------------------------------------------------
void CGModTeamMenu::CreateControls()
{
    // Title label
    m_pTitleLabel = new Label(this, "TitleLabel", "Select Your Team");
    m_pTitleLabel->SetContentAlignment(Label::a_center);

    // Instruction label
    m_pInstructionLabel = new Label(this, "InstructionLabel", "Choose a team to join:");
    m_pInstructionLabel->SetContentAlignment(Label::a_west);

    // Team list
    m_pTeamList = new ListPanel(this, "TeamList");
    m_pTeamList->AddColumnHeader(0, "team", "Team", 150, 0);
    m_pTeamList->AddColumnHeader(1, "players", "Players", 80, 0);
    m_pTeamList->AddColumnHeader(2, "status", "Status", 100, 0);
    m_pTeamList->SetSelectIndividualCells(false);
    m_pTeamList->SetEmptyListText("No teams available");
    // Note: SetDragEnabled not available in 2003 VGUI - drag disabled by default

    // Team info labels
    m_pTeamNameLabel = new Label(this, "TeamNameLabel", "");
    m_pTeamNameLabel->SetContentAlignment(Label::a_west);

    m_pTeamDescLabel = new Label(this, "TeamDescLabel", "Select a team to see description");
    m_pTeamDescLabel->SetContentAlignment(Label::a_northwest);
    // Note: SetWrap not available in 2003 VGUI - text wrapping handled differently

    m_pPlayerCountLabel = new Label(this, "PlayerCountLabel", "");
    m_pPlayerCountLabel->SetContentAlignment(Label::a_west);

    // Team logo
    m_pTeamLogoPanel = new ImagePanel(this, "TeamLogoPanel");
    // Note: SetImage with string path not available in 2003 VGUI - would need IImage* object

    // Buttons - 2003 VGUI style (3 parameters only)
    m_pJoinButton = new Button(this, "JoinButton", "Join Team");
    m_pAutoAssignButton = new Button(this, "AutoAssignButton", "Auto Assign");
    m_pSpectateButton = new Button(this, "SpectateButton", "Spectate");
    m_pCancelButton = new Button(this, "CancelButton", "Cancel");

    // Set up event handling
    m_pTeamList->AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: Layout controls on the panel
//-----------------------------------------------------------------------------
void CGModTeamMenu::LayoutControls()
{
    int wide, tall;
    GetSize(wide, tall);

    int margin = 10;
    int buttonHeight = 24;

    // Title
    if (m_pTitleLabel)
        m_pTitleLabel->SetBounds(margin, margin, wide - margin * 2, 20);

    // Instruction
    int ypos = margin + 30;
    if (m_pInstructionLabel)
        m_pInstructionLabel->SetBounds(margin, ypos, wide - margin * 2, 16);

    // Team list
    ypos += 25;
    int listHeight = 120;
    if (m_pTeamList)
        m_pTeamList->SetBounds(margin, ypos, wide - margin * 2, listHeight);

    // Team info section
    ypos += listHeight + margin;
    if (m_pTeamNameLabel)
        m_pTeamNameLabel->SetBounds(margin, ypos, wide - 80 - margin * 2, 16);

    if (m_pTeamLogoPanel)
        m_pTeamLogoPanel->SetBounds(wide - 70, ypos, 64, 64);

    ypos += 20;
    if (m_pPlayerCountLabel)
        m_pPlayerCountLabel->SetBounds(margin, ypos, wide - 80 - margin * 2, 16);

    ypos += 20;
    if (m_pTeamDescLabel)
        m_pTeamDescLabel->SetBounds(margin, ypos, wide - 80 - margin * 2, 50);

    // Buttons
    ypos = tall - buttonHeight - margin;
    int buttonWidth = 80;
    int spacing = 90;

    if (m_pJoinButton)
        m_pJoinButton->SetBounds(margin, ypos, buttonWidth, buttonHeight);

    if (m_pAutoAssignButton)
        m_pAutoAssignButton->SetBounds(margin + spacing, ypos, buttonWidth, buttonHeight);

    if (m_pSpectateButton)
        m_pSpectateButton->SetBounds(margin + spacing * 2, ypos, buttonWidth, buttonHeight);

    if (m_pCancelButton)
        m_pCancelButton->SetBounds(wide - buttonWidth - margin, ypos, buttonWidth, buttonHeight);
}

//-----------------------------------------------------------------------------
// Purpose: Populate the team list
//-----------------------------------------------------------------------------
void CGModTeamMenu::PopulateTeamList()
{
    if (!m_pTeamList)
        return;

    m_pTeamList->DeleteAllItems(); // 2003 VGUI equivalent of RemoveAll()

    for (int i = 0; i < m_Teams.Count(); i++)
    {
        const TeamMenuData_t& team = m_Teams[i];

        KeyValues *data = new KeyValues("team");
        data->SetString("team", team.teamName);
        // 2003-compatible string formatting instead of UTIL_VarArgs
        char playerText[32];
        Q_snprintf(playerText, sizeof(playerText), "%d/%d", team.playerCount, team.maxPlayers);
        data->SetString("players", playerText);
        data->SetString("status", team.canJoin ? "Open" : "Full");
        data->SetInt("index", i);

        int itemID = m_pTeamList->AddItem(data, 0, false, false);

        // Note: SetItemFgColor not available in 2003 VGUI - color coding removed but functionality preserved
        // Team status is still indicated through text in the "Status" column

        data->deleteThis();
    }
}

//-----------------------------------------------------------------------------
// Purpose: Select a team
//-----------------------------------------------------------------------------
void CGModTeamMenu::SelectTeam(int teamIndex)
{
    if (teamIndex >= 0 && teamIndex < m_Teams.Count())
    {
        m_iSelectedTeam = teamIndex;
        UpdateSelectedTeamInfo();

        if (m_pTeamList)
            m_pTeamList->SetSingleSelectedItem(teamIndex);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Update info for selected team
//-----------------------------------------------------------------------------
void CGModTeamMenu::UpdateSelectedTeamInfo()
{
    if (m_iSelectedTeam < 0 || m_iSelectedTeam >= m_Teams.Count())
        return;

    const TeamMenuData_t& team = m_Teams[m_iSelectedTeam];

    if (m_pTeamNameLabel)
        m_pTeamNameLabel->SetText(team.teamName);

    if (m_pTeamDescLabel)
        m_pTeamDescLabel->SetText(team.description);

    if (m_pPlayerCountLabel)
    {
        // 2003-compatible string formatting instead of UTIL_VarArgs
        char playerText[64];
        Q_snprintf(playerText, sizeof(playerText), "Players: %d/%d", team.playerCount, team.maxPlayers);
        m_pPlayerCountLabel->SetText(playerText);
    }

    if (m_pJoinButton)
        m_pJoinButton->SetEnabled(team.canJoin && team.teamNumber != m_iCurrentPlayerTeam);
}

//-----------------------------------------------------------------------------
// Purpose: Build team data list
//-----------------------------------------------------------------------------
void CGModTeamMenu::BuildTeamList()
{
    m_Teams.RemoveAll();

    TeamMenuData_t teamData;

    // Add teams based on game rules
    for (int i = TEAM_SPECTATOR; i < MAX_TEAMS; i++)
    {
        C_Team *pTeam = GetGlobalTeam(i);
        if (!pTeam)
            continue;

        GetTeamData(i, teamData);
        if (teamData.teamName[0] != 0) // Only add teams with names
        {
            m_Teams.AddToTail(teamData);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get team data
//-----------------------------------------------------------------------------
void CGModTeamMenu::GetTeamData(int teamNumber, TeamMenuData_t& data)
{
    data.teamNumber = teamNumber;
    data.playerCount = GetPlayerCountForTeam(teamNumber);
    data.maxPlayers = 32; // Default max
    data.canJoin = CanJoinTeam(teamNumber);
    data.teamColor = GetTeamColor(teamNumber);

    C_Team *pTeam = GetGlobalTeam(teamNumber);
    if (pTeam)
    {
        Q_strncpy(data.teamName, pTeam->Get_Name(), sizeof(data.teamName));
    }
    else
    {
        // Default team names
        switch (teamNumber)
        {
            case TEAM_UNASSIGNED:
                Q_strncpy(data.teamName, "Unassigned", sizeof(data.teamName));
                break;
            case TEAM_SPECTATOR:
                Q_strncpy(data.teamName, "Spectators", sizeof(data.teamName));
                break;
            default:
                Q_snprintf(data.teamName, sizeof(data.teamName), "Team %d", teamNumber);
                break;
        }
    }

    Q_strncpy(data.description, GetTeamDescription(teamNumber), sizeof(data.description));
    Q_strncpy(data.modelPath, "", sizeof(data.modelPath));
}

//-----------------------------------------------------------------------------
// Purpose: Get player count for team
//-----------------------------------------------------------------------------
int CGModTeamMenu::GetPlayerCountForTeam(int teamNumber)
{
    int count = 0;

    if (g_PR)
    {
        for (int i = 1; i <= gpGlobals->maxClients; i++)
        {
            if (g_PR->Get_Connected(i))
            {
                // Get the player entity to check team - 2003 engine approach
                C_BaseEntity *pPlayer = cl_entitylist->GetBaseEntity(i);
                if (pPlayer && pPlayer->GetTeamNumber() == teamNumber)
                    count++;
            }
        }
    }

    return count;
}

//-----------------------------------------------------------------------------
// Purpose: Check if player can join team
//-----------------------------------------------------------------------------
bool CGModTeamMenu::CanJoinTeam(int teamNumber)
{
    if (teamNumber == m_iCurrentPlayerTeam)
        return false;

    // Check team limits
    int playerCount = GetPlayerCountForTeam(teamNumber);
    if (playerCount >= 32) // Max players per team
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Handle item selection - 2003 VGUI style
//-----------------------------------------------------------------------------
void CGModTeamMenu::OnItemSelected()
{
    if (!m_pTeamList)
        return;

    int itemID = m_pTeamList->GetSelectedItem(0);
    if (itemID >= 0)
    {
        KeyValues* itemData = m_pTeamList->GetItem(itemID);
        if (itemData)
        {
            int teamIndex = itemData->GetInt("index");
            SelectTeam(teamIndex);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Handle item double click - 2003 VGUI style
//-----------------------------------------------------------------------------
void CGModTeamMenu::OnItemDoubleClicked()
{
    OnItemSelected();
    JoinSelectedTeam();
}

//-----------------------------------------------------------------------------
// Purpose: Get team color
//-----------------------------------------------------------------------------
Color CGModTeamMenu::GetTeamColor(int teamNumber)
{
    switch (teamNumber)
    {
        case TEAM_UNASSIGNED: return Color(128, 128, 128, 255);
        case TEAM_SPECTATOR: return Color(255, 255, 255, 255);
        case 3: return Color(255, 0, 0, 255); // Red team
        case 4: return Color(0, 0, 255, 255); // Blue team
        case 5: return Color(0, 255, 0, 255); // Green team
        case 6: return Color(255, 255, 0, 255); // Yellow team
        default: return Color(255, 255, 255, 255);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Get team description
//-----------------------------------------------------------------------------
const char* CGModTeamMenu::GetTeamDescription(int teamNumber)
{
    switch (teamNumber)
    {
        case TEAM_UNASSIGNED:
            return "Players not assigned to any team";
        case TEAM_SPECTATOR:
            return "Watch the game without participating";
        case 3:
            return "Red team - Work together to achieve objectives";
        case 4:
            return "Blue team - Coordinate with teammates to win";
        default:
            return "Standard team for multiplayer gameplay";
    }
}

//-----------------------------------------------------------------------------
// Purpose: Initialize team menu system
//-----------------------------------------------------------------------------
void CGModTeamMenu::Initialize()
{
    if (s_bInitialized)
        return;

    // Create the global instance
    VPANEL gameToolParent = enginevgui->GetPanel(PANEL_CLIENTDLL);
    s_pInstance = new CGModTeamMenu(gameToolParent);

    s_bInitialized = true;
    DevMsg("Team Menu System initialized\n");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown team menu system
//-----------------------------------------------------------------------------
void CGModTeamMenu::Shutdown()
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
CGModTeamMenu* CGModTeamMenu::GetInstance()
{
    return s_pInstance;
}

//-----------------------------------------------------------------------------
// Console command handlers
//-----------------------------------------------------------------------------
void CMD_ShowTeamMenu()
{
    CGModTeamMenu* pMenu = CGModTeamMenu::GetInstance();
    if (pMenu)
        pMenu->ShowMenu();
}

void CMD_HideTeamMenu()
{
    CGModTeamMenu* pMenu = CGModTeamMenu::GetInstance();
    if (pMenu)
        pMenu->HideMenu();
}

void CMD_ToggleTeamMenu()
{
    CGModTeamMenu* pMenu = CGModTeamMenu::GetInstance();
    if (pMenu)
        pMenu->ToggleMenu();
}

void CMD_AutoAssign()
{
    engine->ClientCmd("jointeam 0\n");
}

void CMD_JoinTeam()
{
    CCommand args;
    if (args.ArgC() >= 2)
    {
        int teamNum = atoi(args.Arg(1));
        char command[64];
        Q_snprintf(command, sizeof(command), "jointeam %d\n", teamNum);
        engine->ClientCmd(command);
    }
    else
    {
        CMD_ShowTeamMenu();
    }
}

void CMD_Spectate()
{
    engine->ClientCmd("spectate\n");
}

// Register console commands
static ConCommand gm_teammenu_show("gm_teammenu_show", CMD_ShowTeamMenu, "Show team selection menu");
static ConCommand gm_teammenu_hide("gm_teammenu_hide", CMD_HideTeamMenu, "Hide team selection menu");
static ConCommand gm_teammenu_toggle("gm_teammenu_toggle", CMD_ToggleTeamMenu, "Toggle team selection menu");
static ConCommand gm_teammenu("gm_teammenu", CMD_ToggleTeamMenu, "Toggle team selection menu");
static ConCommand gm_autoassign("gm_autoassign", CMD_AutoAssign, "Auto assign to team");
static ConCommand gm_jointeam("gm_jointeam", CMD_JoinTeam, "Join specific team or show team menu");
static ConCommand gm_spectate("gm_spectate", CMD_Spectate, "Join spectators");

//-----------------------------------------------------------------------------
// Client initialization hook
//-----------------------------------------------------------------------------
class CTeamMenuInit : public CAutoGameSystem
{
public:
    CTeamMenuInit() : CAutoGameSystem("TeamMenuInit") {}

    virtual bool Init()
    {
        CGModTeamMenu::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModTeamMenu::Shutdown();
    }
};

static CTeamMenuInit g_TeamMenuInit;