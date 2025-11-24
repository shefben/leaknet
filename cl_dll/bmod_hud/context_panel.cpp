//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Context Panel System - Garry's Mod style context menus for tools
// Based on reverse engineering of Garry's Mod client.dll
//
//=============================================================================//

#include "cbase.h"
#include "context_panel.h"
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include "vgui_controls/AnimationController.h"
#include "filesystem.h"
#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Console Variables
//-----------------------------------------------------------------------------
ConVar context_panel_width("context_panel_width", "300", FCVAR_ARCHIVE, "Width of context panels");
ConVar context_panel_height("context_panel_height", "400", FCVAR_ARCHIVE, "Height of context panels");

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------
ConCommand gm_context("gm_context", ContextMode_f, "Set context panel mode");
ConCommand context_build("context_build", ContextBuild_f, "Toggle context panel build mode");
ConCommand context_reload("context_reload", ContextReload_f, "Reload context panel settings");

//-----------------------------------------------------------------------------
// Global context panel manager
//-----------------------------------------------------------------------------
CContextPanelManager *g_pContextPanelManager = NULL;

//=============================================================================
// CONTEXT CONTROL BASE CLASS
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CContextControl::CContextControl(Panel *parent, const char *name, ContextControlType_t type)
	: BaseClass(parent, name)
{
	m_ControlType = type;
	m_bSelected = false;
	m_bMouseOver = false;
	m_szCommand[0] = '\0';

	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CContextControl::~CContextControl()
{
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CContextControl::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBgColor(pScheme->GetColor("ContextPanel.BgColor", Color(60, 60, 60, 180)));
	SetFgColor(pScheme->GetColor("ContextPanel.FgColor", Color(255, 255, 255, 255)));
}

//-----------------------------------------------------------------------------
// Purpose: Layout the control
//-----------------------------------------------------------------------------
void CContextControl::PerformLayout()
{
	BaseClass::PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Mouse entered
//-----------------------------------------------------------------------------
void CContextControl::OnCursorEntered()
{
	BaseClass::OnCursorEntered();
	m_bMouseOver = true;
	RequestFocus();
}

//-----------------------------------------------------------------------------
// Purpose: Mouse exited
//-----------------------------------------------------------------------------
void CContextControl::OnCursorExited()
{
	BaseClass::OnCursorExited();
	m_bMouseOver = false;
}

//-----------------------------------------------------------------------------
// Purpose: Send command to engine
//-----------------------------------------------------------------------------
void CContextControl::SendCommand()
{
	if (m_szCommand[0] != '\0')
	{
		engine->ClientCmd_Unrestricted(m_szCommand);
	}
}

//=============================================================================
// CONTEXT BUTTON CONTROL
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CContextButton::CContextButton(Panel *parent, const char *name)
	: BaseClass(parent, name, CONTEXT_CONTROL_BUTTON)
{
	m_pButton = new Button(this, "Button", "");
	m_pButton->SetCommand("ButtonPressed");
	m_pButton->AddActionSignalTarget(this);

	m_szText[0] = '\0';
	m_szImage[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CContextButton::~CContextButton()
{
}

//-----------------------------------------------------------------------------
// Purpose: Set the button value
//-----------------------------------------------------------------------------
void CContextButton::SetValue(const char *value)
{
	SetText(value);
}

//-----------------------------------------------------------------------------
// Purpose: Get the button value
//-----------------------------------------------------------------------------
const char* CContextButton::GetValue()
{
	return m_szText;
}

//-----------------------------------------------------------------------------
// Purpose: Set button text
//-----------------------------------------------------------------------------
void CContextButton::SetText(const char *text)
{
	if (text)
	{
		Q_strncpy(m_szText, text, sizeof(m_szText));
		m_pButton->SetText(text);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set button image
//-----------------------------------------------------------------------------
void CContextButton::SetImage(const char *imageName)
{
	if (imageName)
	{
		Q_strncpy(m_szImage, imageName, sizeof(m_szImage));
		// Set button image if supported
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle mouse press
//-----------------------------------------------------------------------------
void CContextButton::OnMousePressed(MouseCode code)
{
	if (code == MOUSE_LEFT)
	{
		SendCommand();
		SetSelected(!IsSelected());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Paint the control
//-----------------------------------------------------------------------------
void CContextButton::Paint()
{
	BaseClass::Paint();

	if (IsSelected() || m_bMouseOver)
	{
		surface()->DrawSetColor(255, 255, 255, m_bMouseOver ? 40 : 20);
		surface()->DrawFilledRect(0, 0, GetWide(), GetTall());
	}
}

//=============================================================================
// CONTEXT TEXT ENTRY CONTROL
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CContextTextEntry::CContextTextEntry(Panel *parent, const char *name)
	: BaseClass(parent, name, CONTEXT_CONTROL_TEXTENTRY)
{
	m_pTextEntry = new TextEntry(this, "TextEntry");
	m_pTextEntry->AddActionSignalTarget(this);
	m_szValue[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CContextTextEntry::~CContextTextEntry()
{
}

//-----------------------------------------------------------------------------
// Purpose: Set text value
//-----------------------------------------------------------------------------
void CContextTextEntry::SetValue(const char *value)
{
	if (value)
	{
		Q_strncpy(m_szValue, value, sizeof(m_szValue));
		m_pTextEntry->SetText(value);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get text value
//-----------------------------------------------------------------------------
const char* CContextTextEntry::GetValue()
{
	return m_szValue;
}

//-----------------------------------------------------------------------------
// Purpose: Handle text changed
//-----------------------------------------------------------------------------
void CContextTextEntry::OnTextChanged()
{
	m_pTextEntry->GetText(m_szValue, sizeof(m_szValue));
	SendCommand();
}

//=============================================================================
// CONTEXT COMBOBOX CONTROL
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CContextComboBox::CContextComboBox(Panel *parent, const char *name)
	: BaseClass(parent, name, CONTEXT_CONTROL_COMBOBOX)
{
	m_pComboBox = new ComboBox(this, "ComboBox", 10, false);
	m_pComboBox->AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CContextComboBox::~CContextComboBox()
{
	RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: Set combo value
//-----------------------------------------------------------------------------
void CContextComboBox::SetValue(const char *value)
{
	if (value)
	{
		// Find and select the item with this value
		for (int i = 0; i < m_ItemValues.Count(); i++)
		{
			if (Q_stricmp(m_ItemValues[i], value) == 0)
			{
				m_pComboBox->ActivateItem(i);
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get combo value
//-----------------------------------------------------------------------------
const char* CContextComboBox::GetValue()
{
	int activeItem = m_pComboBox->GetActiveItem();
	if (activeItem >= 0 && activeItem < m_ItemValues.Count())
	{
		return m_ItemValues[activeItem];
	}
	return "";
}

//-----------------------------------------------------------------------------
// Purpose: Add item to combo
//-----------------------------------------------------------------------------
void CContextComboBox::AddItem(const char *itemText, const char *itemValue)
{
	if (itemText && itemValue)
	{
		m_pComboBox->AddItem(itemText, NULL);

		char *storedValue = new char[strlen(itemValue) + 1];
		Q_strcpy(storedValue, itemValue);
		m_ItemValues.AddToTail(storedValue);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove all items
//-----------------------------------------------------------------------------
void CContextComboBox::RemoveAll()
{
	m_pComboBox->RemoveAll();

	for (int i = 0; i < m_ItemValues.Count(); i++)
	{
		delete[] m_ItemValues[i];
	}
	m_ItemValues.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Handle selection changed
//-----------------------------------------------------------------------------
void CContextComboBox::OnTextChanged()
{
	SendCommand();
}

//=============================================================================
// MAIN CONTEXT PANEL
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CContextPanel::CContextPanel(Panel *parent, const char *name)
	: BaseClass(parent, name)
{
	m_szToolName[0] = '\0';
	m_szCurrentFile[0] = '\0';
	m_bBuildMode = false;
	m_bVisible = false;
	m_iScrollPosition = 0;

	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);
	SetVisible(false);

	// Create scroll bar
	m_pVerticalScrollBar = new ScrollBar(this, "VerticalScrollBar", true);
	m_pVerticalScrollBar->SetVisible(false);
	m_pVerticalScrollBar->AddActionSignalTarget(this);

	// Create background panel
	m_pBackground = new Panel(this, "Background");
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CContextPanel::~CContextPanel()
{
	RemoveAllControls();
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CContextPanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBgColor(pScheme->GetColor("ContextPanelBG", Color(45, 45, 45, 200)));
	SetBorder(pScheme->GetBorder("ContextPanelBorder"));
}

//-----------------------------------------------------------------------------
// Purpose: Layout the panel
//-----------------------------------------------------------------------------
void CContextPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize(wide, tall);

	// Layout background
	m_pBackground->SetBounds(0, 0, wide, tall);

	// Layout scroll bar
	m_pVerticalScrollBar->SetBounds(wide - 16, 0, 16, tall);

	// Layout controls
	LayoutControls();
}

//-----------------------------------------------------------------------------
// Purpose: Handle size changes
//-----------------------------------------------------------------------------
void CContextPanel::OnSizeChanged(int newWide, int newTall)
{
	BaseClass::OnSizeChanged(newWide, newTall);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Show/hide the panel
//-----------------------------------------------------------------------------
void CContextPanel::ShowPanel(bool bShow)
{
	if (bShow != m_bVisible)
	{
		m_bVisible = bShow;
		SetVisible(bShow);

		if (bShow)
		{
			RequestFocus();
			MoveToFront();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Load context from file
//-----------------------------------------------------------------------------
void CContextPanel::LoadFromFile(const char *filename)
{
	if (!filename)
		return;

	Q_strncpy(m_szCurrentFile, filename, sizeof(m_szCurrentFile));

	KeyValues *kv = new KeyValues("ContextPanel");
	if (kv->LoadFromFile(::filesystem, filename, "GAME"))
	{
		// Load controls from keyvalues
		for (KeyValues *controlKV = kv->GetFirstSubKey(); controlKV; controlKV = controlKV->GetNextKey())
		{
			LoadControlFromKeyValues(controlKV);
		}
	}
	kv->deleteThis();

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Save context to file
//-----------------------------------------------------------------------------
void CContextPanel::SaveToFile(const char *filename)
{
	if (!filename)
		return;

	KeyValues *kv = new KeyValues("ContextPanel");

	// Save all controls to keyvalues
	for (int i = 0; i < m_Controls.Count(); i++)
	{
		KeyValues *controlKV = SaveControlToKeyValues(m_Controls[i]);
		if (controlKV)
		{
			kv->AddSubKey(controlKV);
		}
	}

	kv->SaveToFile(::filesystem, filename, "GAME");
	kv->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Set tool context
//-----------------------------------------------------------------------------
void CContextPanel::SetToolContext(const char *toolName)
{
	if (toolName)
	{
		Q_strncpy(m_szToolName, toolName, sizeof(m_szToolName));

		// Load context file for this tool
		char filename[MAX_PATH];
		Q_snprintf(filename, sizeof(filename), "settings/context_panels/%s.txt", toolName);
		LoadFromFile(filename);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Reset to defaults
//-----------------------------------------------------------------------------
void CContextPanel::ResetToDefaults()
{
	RemoveAllControls();

	// Load default context file
	LoadFromFile("settings/context_panels/default.txt");
}

//-----------------------------------------------------------------------------
// Purpose: Create a context control
//-----------------------------------------------------------------------------
CContextControl* CContextPanel::CreateControl(ContextControlType_t type, const char *name)
{
	CContextControl *control = NULL;

	switch (type)
	{
	case CONTEXT_CONTROL_BUTTON:
		control = new CContextButton(this, name);
		break;
	case CONTEXT_CONTROL_TEXTENTRY:
		control = new CContextTextEntry(this, name);
		break;
	case CONTEXT_CONTROL_COMBOBOX:
		control = new CContextComboBox(this, name);
		break;
	// Add other control types as needed
	default:
		control = new CContextButton(this, name); // Default to button
		break;
	}

	if (control)
	{
		m_Controls.AddToTail(control);
		InvalidateLayout();
	}

	return control;
}

//-----------------------------------------------------------------------------
// Purpose: Remove a control
//-----------------------------------------------------------------------------
void CContextPanel::RemoveControl(CContextControl *control)
{
	if (control)
	{
		m_Controls.FindAndRemove(control);
		control->MarkForDeletion();
		InvalidateLayout();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove all controls
//-----------------------------------------------------------------------------
void CContextPanel::RemoveAllControls()
{
	for (int i = 0; i < m_Controls.Count(); i++)
	{
		m_Controls[i]->MarkForDeletion();
	}
	m_Controls.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Find a control by name
//-----------------------------------------------------------------------------
CContextControl* CContextPanel::FindControl(const char *name)
{
	for (int i = 0; i < m_Controls.Count(); i++)
	{
		if (Q_stricmp(m_Controls[i]->GetName(), name) == 0)
		{
			return m_Controls[i];
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Set build mode
//-----------------------------------------------------------------------------
void CContextPanel::SetBuildMode(bool buildMode)
{
	m_bBuildMode = buildMode;

	// Update all controls for build mode
	for (int i = 0; i < m_Controls.Count(); i++)
	{
		// Enable dragging/resizing in build mode
		m_Controls[i]->SetMouseInputEnabled(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle context commands
//-----------------------------------------------------------------------------
void CContextPanel::OnContextCommand(const char *command)
{
	if (command)
	{
		engine->ClientCmd_Unrestricted(command);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle mouse press
//-----------------------------------------------------------------------------
void CContextPanel::OnMousePressed(MouseCode code)
{
	BaseClass::OnMousePressed(code);

	if (m_bBuildMode && code == MOUSE_RIGHT)
	{
		// Open context menu for creating new controls
		Menu *menu = new Menu(this, "ContextMenu");
		menu->AddMenuItem("button", "Create Button", new KeyValues("CreateControl", "type", CONTEXT_CONTROL_BUTTON), this);
		menu->AddMenuItem("textentry", "Create TextEntry", new KeyValues("CreateControl", "type", CONTEXT_CONTROL_TEXTENTRY), this);
		menu->AddMenuItem("combobox", "Create ComboBox", new KeyValues("CreateControl", "type", CONTEXT_CONTROL_COMBOBOX), this);

		int mx, my;
		input()->GetCursorPos(mx, my);
		menu->SetPos(mx, my);
		menu->SetVisible(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle key presses
//-----------------------------------------------------------------------------
void CContextPanel::OnKeyCodePressed(KeyCode code)
{
	BaseClass::OnKeyCodePressed(code);

	if (m_bBuildMode)
	{
		if (code == KEY_DELETE)
		{
			// Delete selected controls
			for (int i = m_Controls.Count() - 1; i >= 0; i--)
			{
				if (m_Controls[i]->IsSelected())
				{
					RemoveControl(m_Controls[i]);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Paint the panel
//-----------------------------------------------------------------------------
void CContextPanel::Paint()
{
	BaseClass::Paint();

	if (m_bBuildMode)
	{
		// Draw build mode indicators
		surface()->DrawSetColor(255, 255, 0, 100);
		surface()->DrawOutlinedRect(0, 0, GetWide(), GetTall());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Layout controls
//-----------------------------------------------------------------------------
void CContextPanel::LayoutControls()
{
	int yPos = 5 - m_iScrollPosition;
	int controlHeight = 25;
	int spacing = 5;

	for (int i = 0; i < m_Controls.Count(); i++)
	{
		CContextControl *control = m_Controls[i];
		control->SetBounds(5, yPos, GetWide() - 30, controlHeight);
		yPos += controlHeight + spacing;
	}

	// Update scroll bar
	UpdateScrollBar();
}

//-----------------------------------------------------------------------------
// Purpose: Update scroll bar
//-----------------------------------------------------------------------------
void CContextPanel::UpdateScrollBar()
{
	int totalHeight = m_Controls.Count() * 30;
	int visibleHeight = GetTall();

	if (totalHeight > visibleHeight)
	{
		m_pVerticalScrollBar->SetVisible(true);
		m_pVerticalScrollBar->SetRange(0, totalHeight - visibleHeight);
		m_pVerticalScrollBar->SetRangeWindow(visibleHeight);
		m_pVerticalScrollBar->SetButtonPressedScrollValue(15);
	}
	else
	{
		m_pVerticalScrollBar->SetVisible(false);
		m_iScrollPosition = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Save control to keyvalues
//-----------------------------------------------------------------------------
KeyValues* CContextPanel::SaveControlToKeyValues(CContextControl *control)
{
	if (!control)
		return NULL;

	KeyValues *kv = new KeyValues("Control");
	kv->SetString("name", control->GetName());
	kv->SetInt("type", control->GetControlType());
	kv->SetString("value", control->GetValue());
	kv->SetString("command", control->GetCommandString());

	int x, y, w, h;
	control->GetBounds(x, y, w, h);
	kv->SetInt("x", x);
	kv->SetInt("y", y);
	kv->SetInt("w", w);
	kv->SetInt("h", h);

	return kv;
}

//-----------------------------------------------------------------------------
// Purpose: Load control from keyvalues
//-----------------------------------------------------------------------------
void CContextPanel::LoadControlFromKeyValues(KeyValues *kv)
{
	if (!kv)
		return;

	const char *name = kv->GetString("name", "control");
	ContextControlType_t type = (ContextControlType_t)kv->GetInt("type", CONTEXT_CONTROL_BUTTON);
	const char *value = kv->GetString("value", "");
	const char *command = kv->GetString("command", "");

	CContextControl *control = CreateControl(type, name);
	if (control)
	{
		control->SetValue(value);
		control->SetCommandString(command);

		int x = kv->GetInt("x", 0);
		int y = kv->GetInt("y", 0);
		int w = kv->GetInt("w", 100);
		int h = kv->GetInt("h", 25);
		control->SetBounds(x, y, w, h);
	}
}

//=============================================================================
// FACE CONTEXT PANEL
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFaceContextPanel::CFaceContextPanel(Panel *parent, const char *name)
	: BaseClass(parent, name)
{
	m_iEntityIndex = -1;
	m_iFaceIndex = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFaceContextPanel::~CFaceContextPanel()
{
}

//-----------------------------------------------------------------------------
// Purpose: Set face data
//-----------------------------------------------------------------------------
void CFaceContextPanel::SetFaceData(int entityIndex, int faceIndex)
{
	m_iEntityIndex = entityIndex;
	m_iFaceIndex = faceIndex;

	// Load face context configuration
	LoadFromFile("settings/context_panels/face.txt");
}

//-----------------------------------------------------------------------------
// Purpose: Show material browser
//-----------------------------------------------------------------------------
void CFaceContextPanel::ShowMaterialBrowser()
{
	// Implementation for material browser
	DevMsg("Face Context: Show material browser for entity %d, face %d\n", m_iEntityIndex, m_iFaceIndex);
}

//-----------------------------------------------------------------------------
// Purpose: Show color picker
//-----------------------------------------------------------------------------
void CFaceContextPanel::ShowColorPicker()
{
	// Implementation for color picker
	DevMsg("Face Context: Show color picker for entity %d, face %d\n", m_iEntityIndex, m_iFaceIndex);
}

//-----------------------------------------------------------------------------
// Purpose: Handle face context commands
//-----------------------------------------------------------------------------
void CFaceContextPanel::OnContextCommand(const char *command)
{
	if (Q_strstr(command, "material"))
	{
		ShowMaterialBrowser();
	}
	else if (Q_strstr(command, "color"))
	{
		ShowColorPicker();
	}
	else
	{
		BaseClass::OnContextCommand(command);
	}
}

//=============================================================================
// CONTEXT PANEL MANAGER
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CContextPanelManager::CContextPanelManager()
{
	m_pContextPanel = NULL;
	m_pFaceContextPanel = NULL;
	m_bInitialized = false;

	// Create panels
	m_pContextPanel = new CContextPanel(NULL, "ContextPanel");
	m_pFaceContextPanel = new CFaceContextPanel(NULL, "FaceContextPanel");

	// Set initial sizes
	m_pContextPanel->SetSize(context_panel_width.GetInt(), context_panel_height.GetInt());
	m_pFaceContextPanel->SetSize(context_panel_width.GetInt(), context_panel_height.GetInt());

	m_bInitialized = true;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CContextPanelManager::~CContextPanelManager()
{
	if (m_pContextPanel)
	{
		m_pContextPanel->MarkForDeletion();
		m_pContextPanel = NULL;
	}

	if (m_pFaceContextPanel)
	{
		m_pFaceContextPanel->MarkForDeletion();
		m_pFaceContextPanel = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Show context panel for tool
//-----------------------------------------------------------------------------
void CContextPanelManager::ShowContextPanel(const char *toolName, bool show)
{
	if (!m_pContextPanel || !toolName)
		return;

	if (show)
	{
		m_pContextPanel->SetToolContext(toolName);
	}

	m_pContextPanel->ShowPanel(show);
}

//-----------------------------------------------------------------------------
// Purpose: Show face context panel
//-----------------------------------------------------------------------------
void CContextPanelManager::ShowFaceContextPanel(int entityIndex, int faceIndex, bool show)
{
	if (!m_pFaceContextPanel)
		return;

	if (show)
	{
		m_pFaceContextPanel->SetFaceData(entityIndex, faceIndex);
	}

	m_pFaceContextPanel->ShowPanel(show);
}

//-----------------------------------------------------------------------------
// Purpose: Hide all context panels
//-----------------------------------------------------------------------------
void CContextPanelManager::HideAllContextPanels()
{
	if (m_pContextPanel)
		m_pContextPanel->ShowPanel(false);

	if (m_pFaceContextPanel)
		m_pFaceContextPanel->ShowPanel(false);
}

//-----------------------------------------------------------------------------
// Purpose: Load context settings
//-----------------------------------------------------------------------------
void CContextPanelManager::LoadContextSettings()
{
	// Load global context settings
	DevMsg("Loading context panel settings...\n");
}

//-----------------------------------------------------------------------------
// Purpose: Save context settings
//-----------------------------------------------------------------------------
void CContextPanelManager::SaveContextSettings()
{
	// Save global context settings
	DevMsg("Saving context panel settings...\n");
}

//-----------------------------------------------------------------------------
// Purpose: Reload context settings
//-----------------------------------------------------------------------------
void CContextPanelManager::ReloadContextSettings()
{
	LoadContextSettings();

	if (m_pContextPanel)
		m_pContextPanel->ResetToDefaults();
}

//-----------------------------------------------------------------------------
// Purpose: Set context mode
//-----------------------------------------------------------------------------
void CContextPanelManager::SetContextMode(const char *mode)
{
	if (!mode)
		return;

	if (Q_stricmp(mode, "npc") == 0)
	{
		ShowContextPanel("npc", true);
	}
	else if (Q_stricmp(mode, "camera") == 0)
	{
		ShowContextPanel("camera", true);
	}
	else
	{
		HideAllContextPanels();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Toggle build mode
//-----------------------------------------------------------------------------
void CContextPanelManager::ToggleBuildMode()
{
	if (m_pContextPanel)
	{
		bool buildMode = !m_pContextPanel->IsBuildMode();
		m_pContextPanel->SetBuildMode(buildMode);
		DevMsg("Context panel build mode: %s\n", buildMode ? "ON" : "OFF");
	}
}

//=============================================================================
// CONSOLE COMMAND HANDLERS
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: gm_context console command
//-----------------------------------------------------------------------------
void ContextMode_f(void)
{
	if (!g_pContextPanelManager)
		return;

	// In 2003 engine, use alternative method to get command arguments
	if (engine->Cmd_Argc() >= 2)
	{
		g_pContextPanelManager->SetContextMode(engine->Cmd_Argv(1));
	}
	else
	{
		Msg("Usage: gm_context <mode>\n");
		Msg("Modes: npc, camera\n");
	}
}

//-----------------------------------------------------------------------------
// Purpose: context_build console command
//-----------------------------------------------------------------------------
void ContextBuild_f()
{
	if (!g_pContextPanelManager)
		return;

	g_pContextPanelManager->ToggleBuildMode();
}

//-----------------------------------------------------------------------------
// Purpose: context_reload console command
//-----------------------------------------------------------------------------
void ContextReload_f()
{
	if (!g_pContextPanelManager)
		return;

	g_pContextPanelManager->ReloadContextSettings();
}

//-----------------------------------------------------------------------------
// Purpose: Initialize context panel manager
//-----------------------------------------------------------------------------
void InitContextPanelManager()
{
	if (!g_pContextPanelManager)
	{
		g_pContextPanelManager = new CContextPanelManager();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown context panel manager
//-----------------------------------------------------------------------------
void ShutdownContextPanelManager()
{
	if (g_pContextPanelManager)
	{
		delete g_pContextPanelManager;
		g_pContextPanelManager = NULL;
	}
}