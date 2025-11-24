//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Context Panel System - Garry's Mod style context menus for tools
// Based on reverse engineering of Garry's Mod client.dll
//
//=============================================================================//

#ifndef CONTEXT_PANEL_H
#define CONTEXT_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/EditablePanel.h>
#include "vgui/IScheme.h"
#include "vgui/ILocalize.h"
#include "KeyValues.h"
#include "utlvector.h"
#include "convar.h"

namespace vgui
{
	class ImagePanel;
}

//-----------------------------------------------------------------------------
// Context Panel Control Types
//-----------------------------------------------------------------------------
enum ContextControlType_t
{
	CONTEXT_CONTROL_BUTTON = 0,
	CONTEXT_CONTROL_LABEL,
	CONTEXT_CONTROL_TEXTENTRY,
	CONTEXT_CONTROL_COMBOBOX,
	CONTEXT_CONTROL_CHECKBOX,
	CONTEXT_CONTROL_SLIDER,
	CONTEXT_CONTROL_COLORBROWSER,
	CONTEXT_CONTROL_MATERIALBROWSER,
	CONTEXT_CONTROL_PROPBROWSER,
	CONTEXT_CONTROL_MAX
};

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CContextPanel;
class CFaceContextPanel;

//-----------------------------------------------------------------------------
// Context Control Base Class
//-----------------------------------------------------------------------------
class CContextControl : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CContextControl, vgui::EditablePanel);

public:
	CContextControl(vgui::Panel *parent, const char *name, ContextControlType_t type);
	virtual ~CContextControl();

	// Panel overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();
	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	// Context control interface
	virtual void SetValue(const char *value) = 0;
	virtual const char *GetValue() = 0;
	virtual void SetCommandString(const char *command) { Q_strncpy(m_szCommand, command, sizeof(m_szCommand)); }
	virtual const char *GetCommandString() { return m_szCommand; }

	// Properties
	ContextControlType_t GetControlType() const { return m_ControlType; }
	bool IsSelected() const { return m_bSelected; }
	void SetSelected(bool selected) { m_bSelected = selected; }

protected:
	virtual void SendCommand();

	ContextControlType_t m_ControlType;
	char m_szCommand[256];
	bool m_bSelected;
	bool m_bMouseOver;
};

//-----------------------------------------------------------------------------
// Context Button Control
//-----------------------------------------------------------------------------
class CContextButton : public CContextControl
{
	DECLARE_CLASS_SIMPLE(CContextButton, CContextControl);

public:
	CContextButton(vgui::Panel *parent, const char *name);
	virtual ~CContextButton();

	// Control interface
	virtual void SetValue(const char *value);
	virtual const char *GetValue();

	// Button properties
	void SetText(const char *text);
	void SetImage(const char *imageName);

protected:
	virtual void OnMousePressed(vgui::MouseCode code);
	virtual void Paint();

private:
	vgui::Button *m_pButton;
	char m_szText[128];
	char m_szImage[128];
};

//-----------------------------------------------------------------------------
// Context TextEntry Control
//-----------------------------------------------------------------------------
class CContextTextEntry : public CContextControl
{
	DECLARE_CLASS_SIMPLE(CContextTextEntry, CContextControl);

public:
	CContextTextEntry(vgui::Panel *parent, const char *name);
	virtual ~CContextTextEntry();

	// Control interface
	virtual void SetValue(const char *value);
	virtual const char *GetValue();

protected:
	virtual void OnTextChanged();

private:
	vgui::TextEntry *m_pTextEntry;
	char m_szValue[256];
};

//-----------------------------------------------------------------------------
// Context ComboBox Control
//-----------------------------------------------------------------------------
class CContextComboBox : public CContextControl
{
	DECLARE_CLASS_SIMPLE(CContextComboBox, CContextControl);

public:
	CContextComboBox(vgui::Panel *parent, const char *name);
	virtual ~CContextComboBox();

	// Control interface
	virtual void SetValue(const char *value);
	virtual const char *GetValue();

	// ComboBox interface
	void AddItem(const char *itemText, const char *itemValue);
	void RemoveAll();

protected:
	virtual void OnTextChanged();

private:
	vgui::ComboBox *m_pComboBox;
	CUtlVector<char*> m_ItemValues;
};

//-----------------------------------------------------------------------------
// Main Context Panel
//-----------------------------------------------------------------------------
class CContextPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CContextPanel, vgui::EditablePanel);

public:
	CContextPanel(vgui::Panel *parent, const char *name);
	virtual ~CContextPanel();

	// Panel overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();
	virtual void OnSizeChanged(int newWide, int newTall);

	// Context panel interface
	virtual void ShowPanel(bool bShow);
	virtual void LoadFromFile(const char *filename);
	virtual void SaveToFile(const char *filename);
	virtual void SetToolContext(const char *toolName);
	virtual void ResetToDefaults();

	// Control management
	virtual CContextControl* CreateControl(ContextControlType_t type, const char *name);
	virtual void RemoveControl(CContextControl *control);
	virtual void RemoveAllControls();
	virtual CContextControl* FindControl(const char *name);

	// Build mode
	virtual void SetBuildMode(bool buildMode);
	virtual bool IsBuildMode() const { return m_bBuildMode; }

	// Event handling
	virtual void OnContextCommand(const char *command);

protected:
	virtual void OnMousePressed(vgui::MouseCode code);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void Paint();

	// Layout helpers
	virtual void LayoutControls();
	virtual void UpdateScrollBar();

	// File I/O helpers
	virtual KeyValues* SaveControlToKeyValues(CContextControl *control);
	virtual void LoadControlFromKeyValues(KeyValues *kv);

private:
	CUtlVector<CContextControl*> m_Controls;
	char m_szToolName[64];
	char m_szCurrentFile[MAX_PATH];
	bool m_bBuildMode;
	bool m_bVisible;

	// UI elements
	vgui::ScrollBar *m_pVerticalScrollBar;
	vgui::Panel *m_pBackground;
	int m_iScrollPosition;
};

//-----------------------------------------------------------------------------
// Face Context Panel - for face selection
//-----------------------------------------------------------------------------
class CFaceContextPanel : public CContextPanel
{
	DECLARE_CLASS_SIMPLE(CFaceContextPanel, CContextPanel);

public:
	CFaceContextPanel(vgui::Panel *parent, const char *name);
	virtual ~CFaceContextPanel();

	// Face context interface
	virtual void SetFaceData(int entityIndex, int faceIndex);
	virtual void ShowMaterialBrowser();
	virtual void ShowColorPicker();

protected:
	virtual void OnContextCommand(const char *command);

private:
	int m_iEntityIndex;
	int m_iFaceIndex;
};

//-----------------------------------------------------------------------------
// Context Panel Manager - global access
//-----------------------------------------------------------------------------
class CContextPanelManager
{
public:
	CContextPanelManager();
	~CContextPanelManager();

	// Panel management
	CContextPanel* GetContextPanel() { return m_pContextPanel; }
	CFaceContextPanel* GetFaceContextPanel() { return m_pFaceContextPanel; }

	// Global commands
	void ShowContextPanel(const char *toolName, bool show = true);
	void ShowFaceContextPanel(int entityIndex, int faceIndex, bool show = true);
	void HideAllContextPanels();

	// Settings management
	void LoadContextSettings();
	void SaveContextSettings();
	void ReloadContextSettings();

	// Console command handlers
	void SetContextMode(const char *mode);
	void ToggleBuildMode();

private:
	CContextPanel *m_pContextPanel;
	CFaceContextPanel *m_pFaceContextPanel;
	bool m_bInitialized;
};

// Global context panel manager
extern CContextPanelManager *g_pContextPanelManager;

// Console command functions
void ContextMode_f(void);
void ContextBuild_f();
void ContextReload_f();

#endif // CONTEXT_PANEL_H