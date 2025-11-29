//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// Purpose: Quake-style sliding console implementation
//
//=============================================================================

#include "quakedef.h"
#include "quakeconsole.h"
#include "filesystem.h"
#include "filesystem_engine.h"
#include "sys.h"
#include "keys.h"
#include "cmd.h"
#include "cvar.h"
#include "mathlib.h"
#include "vid.h"
#include "common.h"
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Controls.h>
#include "vstdlib/strtools.h"
#include "host.h"
#include <wchar.h>

// External declarations
extern double realtime;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Global instance
CQuakeConsole *g_pQuakeConsole = NULL;

const float CQuakeConsole::ANIMATION_TIME = 0.3f;

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CQuakeConsole::CQuakeConsole()
{
	m_bInitialized = false;
	m_bVisible = false;
	m_bAnimating = false;
	m_flAnimationStart = 0.0f;
	m_flCurrentOffset = 0.0f;
	m_nScrollOffset = 0;
	m_nInputPos = 0;
	m_nInputLength = 0;
	m_nHistoryPos = -1;
	m_nTextureId = -1;
	m_nTextureWidth = 0;
	m_nTextureHeight = 0;
	m_hFont = 0;
	m_nConsoleHeight = 0;
	m_nConsoleWidth = 0;

	m_szInputLine[0] = '\0';
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CQuakeConsole::~CQuakeConsole()
{
	Shutdown();
}

//-----------------------------------------------------------------------------
// Initialize the console
//-----------------------------------------------------------------------------
void CQuakeConsole::Initialize()
{
	if (m_bInitialized)
		return;

	// Get screen dimensions - use defaults if vid not ready
	m_nConsoleWidth = (vid.width > 0) ? vid.width : 640;
	m_nConsoleHeight = (vid.height > 0) ? (vid.height / 2) : 240;

	// Load background texture (has its own safety checks)
	LoadBackgroundTexture();

	// Get font - with safety checks
	m_hFont = 0;
	if (vgui::scheme())
	{
		vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(vgui::scheme()->GetDefaultScheme());
		if (pScheme)
		{
			m_hFont = pScheme->GetFont("DefaultFixed", false);
		}
	}

	// Add welcome message
	Color clr(255, 255, 100, 255);
	AddLine(clr, "Quake-style console initialized");
	AddLine(clr, "Type 'help' for commands");

	m_bInitialized = true;
}

//-----------------------------------------------------------------------------
// Shutdown the console
//-----------------------------------------------------------------------------
void CQuakeConsole::Shutdown()
{
	if (!m_bInitialized)
		return;

	// Clean up input history
	for (int i = 0; i < m_InputHistory.Count(); i++)
	{
		delete[] m_InputHistory[i];
	}
	m_InputHistory.RemoveAll();

	m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Show the console with sliding animation
//-----------------------------------------------------------------------------
void CQuakeConsole::Show()
{
	if (!m_bInitialized)
		Initialize();

	if (m_bVisible && !m_bAnimating)
		return;

	m_bVisible = true;
	m_bAnimating = true;
	m_flAnimationStart = realtime;
}

//-----------------------------------------------------------------------------
// Hide the console with sliding animation
//-----------------------------------------------------------------------------
void CQuakeConsole::Hide()
{
	if (!m_bVisible && !m_bAnimating)
		return;

	m_bVisible = false;
	m_bAnimating = true;
	m_flAnimationStart = realtime;
}

//-----------------------------------------------------------------------------
// Print message to console
//-----------------------------------------------------------------------------
void CQuakeConsole::Print(const char *msg)
{
	if (!msg || !msg[0])
		return;

	Color clr(255, 255, 255, 255);
	ColorPrint(clr, msg);
}

//-----------------------------------------------------------------------------
// Print debug message to console
//-----------------------------------------------------------------------------
void CQuakeConsole::DPrint(const char *msg)
{
	if (!msg || !msg[0])
		return;

	Color clr(196, 181, 80, 255);
	ColorPrint(clr, msg);
}

//-----------------------------------------------------------------------------
// Print colored message to console
//-----------------------------------------------------------------------------
void CQuakeConsole::ColorPrint(Color& clr, const char *msg)
{
	if (!msg || !msg[0])
		return;

	// Split message by newlines
	const char *p = msg;
	char line[CONSOLE_LINE_LENGTH];
	int linePos = 0;

	while (*p)
	{
		if (*p == '\n' || linePos >= CONSOLE_LINE_LENGTH - 1)
		{
			line[linePos] = '\0';
			if (linePos > 0)
			{
				AddLine(clr, line);
			}
			linePos = 0;
			if (*p == '\n')
				p++;
			continue;
		}

		line[linePos++] = *p++;
	}

	// Add remaining text
	if (linePos > 0)
	{
		line[linePos] = '\0';
		AddLine(clr, line);
	}
}

//-----------------------------------------------------------------------------
// Clear console
//-----------------------------------------------------------------------------
void CQuakeConsole::Clear()
{
	m_ConsoleLines.RemoveAll();
	m_nScrollOffset = 0;
}

//-----------------------------------------------------------------------------
// Handle key input
//-----------------------------------------------------------------------------
void CQuakeConsole::HandleKeyInput(int key, bool bDown)
{
	if (!bDown)
		return;

	switch (key)
	{
	case K_ENTER:
		ExecuteCommand();
		break;

	case K_BACKSPACE:
		if (m_nInputPos > 0)
		{
			// Move characters back
			for (int i = m_nInputPos - 1; i < m_nInputLength - 1; i++)
			{
				m_szInputLine[i] = m_szInputLine[i + 1];
			}
			m_nInputPos--;
			m_nInputLength--;
			m_szInputLine[m_nInputLength] = '\0';
		}
		break;

	case K_DEL:
		if (m_nInputPos < m_nInputLength)
		{
			// Move characters back
			for (int i = m_nInputPos; i < m_nInputLength - 1; i++)
			{
				m_szInputLine[i] = m_szInputLine[i + 1];
			}
			m_nInputLength--;
			m_szInputLine[m_nInputLength] = '\0';
		}
		break;

	case K_LEFTARROW:
		if (m_nInputPos > 0)
			m_nInputPos--;
		break;

	case K_RIGHTARROW:
		if (m_nInputPos < m_nInputLength)
			m_nInputPos++;
		break;

	case K_HOME:
		m_nInputPos = 0;
		break;

	case K_END:
		m_nInputPos = m_nInputLength;
		break;

	case K_UPARROW:
		// Previous history
		if (m_InputHistory.Count() > 0)
		{
			if (m_nHistoryPos == -1)
				m_nHistoryPos = m_InputHistory.Count() - 1;
			else if (m_nHistoryPos > 0)
				m_nHistoryPos--;

			if (m_nHistoryPos >= 0 && m_nHistoryPos < m_InputHistory.Count())
			{
				Q_strncpy(m_szInputLine, m_InputHistory[m_nHistoryPos], sizeof(m_szInputLine));
				m_nInputLength = Q_strlen(m_szInputLine);
				m_nInputPos = m_nInputLength;
			}
		}
		break;

	case K_DOWNARROW:
		// Next history
		if (m_InputHistory.Count() > 0 && m_nHistoryPos != -1)
		{
			m_nHistoryPos++;
			if (m_nHistoryPos >= m_InputHistory.Count())
			{
				m_nHistoryPos = -1;
				m_szInputLine[0] = '\0';
				m_nInputLength = 0;
				m_nInputPos = 0;
			}
			else
			{
				Q_strncpy(m_szInputLine, m_InputHistory[m_nHistoryPos], sizeof(m_szInputLine));
				m_nInputLength = Q_strlen(m_szInputLine);
				m_nInputPos = m_nInputLength;
			}
		}
		break;

	case K_PGUP:
		ScrollUp();
		break;

	case K_PGDN:
		ScrollDown();
		break;

	default:
		// Handle printable characters in HandleCharInput
		break;
	}
}

//-----------------------------------------------------------------------------
// Handle character input
//-----------------------------------------------------------------------------
void CQuakeConsole::HandleCharInput(int unichar)
{
	if (unichar < 32 || unichar > 126)
		return;

	if (m_nInputLength >= INPUT_LINE_LENGTH - 1)
		return;

	// Insert character at current position
	for (int i = m_nInputLength; i > m_nInputPos; i--)
	{
		m_szInputLine[i] = m_szInputLine[i - 1];
	}

	m_szInputLine[m_nInputPos] = (char)unichar;
	m_nInputPos++;
	m_nInputLength++;
	m_szInputLine[m_nInputLength] = '\0';
}

//-----------------------------------------------------------------------------
// Paint the console
//-----------------------------------------------------------------------------
void CQuakeConsole::Paint()
{
	if (!IsVisible())
		return;

	// Update screen dimensions in case resolution changed
	if (vid.width > 0 && vid.height > 0)
	{
		m_nConsoleWidth = vid.width;
		m_nConsoleHeight = vid.height / 2;
	}

	UpdateAnimation();

	// Draw background
	DrawBackground();

	// Draw console text
	DrawText();

	// Draw input line
	DrawInputLine();
}

//-----------------------------------------------------------------------------
// Load background texture
//-----------------------------------------------------------------------------
void CQuakeConsole::LoadBackgroundTexture()
{
	// Default to no texture
	m_nTextureId = -1;
	m_nTextureWidth = 0;
	m_nTextureHeight = 0;

	// Safety checks - VGUI surface and filesystem must be available
	if (!vgui::surface())
		return;

	if (!g_pFileSystem)
		return;

	char texturePath[MAX_PATH];
	bool bTextureFound = false;

	// First try mod-specific splash: <mod>/gfx/shell/splash.bmp
	// Note: DrawSetTextureFile expects path without extension and relative to game root
	if (com_gamedir[0])
	{
		Q_snprintf(texturePath, sizeof(texturePath), "%s/gfx/shell/splash", com_gamedir);

		// Check if the .bmp file exists
		char fullPath[MAX_PATH];
		Q_snprintf(fullPath, sizeof(fullPath), "%s.bmp", texturePath);
		if (g_pFileSystem->FileExists(fullPath))
		{
			bTextureFound = true;
		}
	}

	if (!bTextureFound)
	{
		// Fallback to hl2/gfx/shell/splash.bmp
		Q_strncpy(texturePath, "hl2/gfx/shell/splash", sizeof(texturePath));
		char fullPath[MAX_PATH];
		Q_snprintf(fullPath, sizeof(fullPath), "%s.bmp", texturePath);
		if (g_pFileSystem->FileExists(fullPath))
		{
			bTextureFound = true;
		}
	}

	if (bTextureFound)
	{
		// Create texture ID and load the texture
		m_nTextureId = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_nTextureId, texturePath, true, false);

		// Get texture dimensions
		int texWide, texTall;
		vgui::surface()->DrawGetTextureSize(m_nTextureId, texWide, texTall);
		m_nTextureWidth = texWide;
		m_nTextureHeight = texTall;
	}
}

//-----------------------------------------------------------------------------
// Add a line to console
//-----------------------------------------------------------------------------
void CQuakeConsole::AddLine(Color& clr, const char *text)
{
	ConsoleLine line;
	line.color = clr;
	Q_strncpy(line.text, text, sizeof(line.text));

	m_ConsoleLines.AddToTail(line);

	// Limit number of lines
	while (m_ConsoleLines.Count() > MAX_CONSOLE_LINES)
	{
		m_ConsoleLines.Remove(0);
	}

	// Auto-scroll to bottom
	m_nScrollOffset = 0;
}

//-----------------------------------------------------------------------------
// Draw background
//-----------------------------------------------------------------------------
void CQuakeConsole::DrawBackground()
{
	if (!vgui::surface())
		return;

	int y = GetConsoleY();

	// Draw textured background if available
	if (m_nTextureId != -1 && m_nTextureWidth > 0 && m_nTextureHeight > 0)
	{
		// Draw the splash texture, tiled or stretched to fit
		vgui::surface()->DrawSetTexture(m_nTextureId);
		vgui::surface()->DrawSetColor(255, 255, 255, 255);
		vgui::surface()->DrawTexturedRect(0, y, m_nConsoleWidth, y + m_nConsoleHeight);

		// Overlay a semi-transparent dark layer for text readability
		vgui::surface()->DrawSetColor(0, 0, 0, 140);
		vgui::surface()->DrawFilledRect(0, y, m_nConsoleWidth, y + m_nConsoleHeight);
	}
	else
	{
		// Fallback: solid semi-transparent background
		vgui::surface()->DrawSetColor(0, 0, 0, 200);
		vgui::surface()->DrawFilledRect(0, y, m_nConsoleWidth, y + m_nConsoleHeight);
	}

	// Draw border
	vgui::surface()->DrawSetColor(100, 100, 100, 255);
	vgui::surface()->DrawOutlinedRect(0, y, m_nConsoleWidth, y + m_nConsoleHeight);
}

//-----------------------------------------------------------------------------
// Draw console text
//-----------------------------------------------------------------------------
void CQuakeConsole::DrawText()
{
	if (!m_hFont || !vgui::surface() || !vgui::localize())
		return;

	int y = GetConsoleY();
	int fontTall = vgui::surface()->GetFontTall(m_hFont);
	if (fontTall <= 0)
		return;

	int textY = y + 10;
	int maxLines = (m_nConsoleHeight - 40) / fontTall;  // Leave space for input line

	vgui::surface()->DrawSetTextFont(m_hFont);

	int startLine = max(0, m_ConsoleLines.Count() - maxLines - m_nScrollOffset);
	int endLine = min(m_ConsoleLines.Count(), startLine + maxLines);

	for (int i = startLine; i < endLine; i++)
	{
		const ConsoleLine &line = m_ConsoleLines[i];
		vgui::surface()->DrawSetTextColor(line.color);
		vgui::surface()->DrawSetTextPos(10, textY);
		wchar_t wideText[CONSOLE_LINE_LENGTH];
		vgui::localize()->ConvertANSIToUnicode(line.text, wideText, sizeof(wideText));
		vgui::surface()->DrawPrintText(wideText, Q_strlen(line.text));
		textY += fontTall;
	}
}

//-----------------------------------------------------------------------------
// Draw input line
//-----------------------------------------------------------------------------
void CQuakeConsole::DrawInputLine()
{
	if (!m_hFont || !vgui::surface() || !vgui::localize())
		return;

	int y = GetConsoleY();
	int fontTall = vgui::surface()->GetFontTall(m_hFont);
	if (fontTall <= 0)
		return;
	int inputY = y + m_nConsoleHeight - fontTall - 10;

	vgui::surface()->DrawSetTextFont(m_hFont);

	// Draw prompt
	vgui::surface()->DrawSetTextColor(255, 255, 255, 255);
	vgui::surface()->DrawSetTextPos(10, inputY);
	wchar_t prompt[4];
	vgui::localize()->ConvertANSIToUnicode("]", prompt, sizeof(prompt));
	vgui::surface()->DrawPrintText(prompt, 1);

	// Draw input text
	if (m_nInputLength > 0)
	{
		vgui::surface()->DrawSetTextPos(20, inputY);
		wchar_t wideInput[INPUT_LINE_LENGTH];
		vgui::localize()->ConvertANSIToUnicode(m_szInputLine, wideInput, sizeof(wideInput));
		vgui::surface()->DrawPrintText(wideInput, m_nInputLength);
	}

	// Draw cursor
	static float lastBlinkTime = 0;
	static bool bShowCursor = true;

	if (realtime - lastBlinkTime > 0.5f)
	{
		bShowCursor = !bShowCursor;
		lastBlinkTime = realtime;
	}

	if (bShowCursor)
	{
		int cursorX = 20;
		if (m_nInputPos > 0)
		{
			char savedChar = m_szInputLine[m_nInputPos];
			m_szInputLine[m_nInputPos] = '\0';

			wchar_t wideCursorText[INPUT_LINE_LENGTH];
			vgui::localize()->ConvertANSIToUnicode(m_szInputLine, wideCursorText, sizeof(wideCursorText));
			int wide = 0, tall = 0;
			vgui::surface()->GetTextSize(m_hFont, wideCursorText, wide, tall);

			m_szInputLine[m_nInputPos] = savedChar;

			cursorX += wide;
		}

		vgui::surface()->DrawSetColor(255, 255, 255, 255);
		vgui::surface()->DrawFilledRect(cursorX, inputY, cursorX + 1, inputY + fontTall);
	}
}

//-----------------------------------------------------------------------------
// Scroll console up
//-----------------------------------------------------------------------------
void CQuakeConsole::ScrollUp()
{
	m_nScrollOffset++;
	int maxScroll = max(0, m_ConsoleLines.Count() - 1);
	if (m_nScrollOffset > maxScroll)
		m_nScrollOffset = maxScroll;
}

//-----------------------------------------------------------------------------
// Scroll console down
//-----------------------------------------------------------------------------
void CQuakeConsole::ScrollDown()
{
	m_nScrollOffset--;
	if (m_nScrollOffset < 0)
		m_nScrollOffset = 0;
}

//-----------------------------------------------------------------------------
// Execute current command
//-----------------------------------------------------------------------------
void CQuakeConsole::ExecuteCommand()
{
	if (m_nInputLength == 0)
		return;

	// Add to history
	AddToHistory(m_szInputLine);

	// Echo command
	Color cmdColor(100, 255, 100, 255);
	char echo[INPUT_LINE_LENGTH + 2];
	Q_snprintf(echo, sizeof(echo), "]%s", m_szInputLine);
	AddLine(cmdColor, echo);

	// Execute command through engine
	Cmd_ExecuteString(m_szInputLine, src_command);

	// Clear input line
	m_szInputLine[0] = '\0';
	m_nInputLength = 0;
	m_nInputPos = 0;
	m_nHistoryPos = -1;
}

//-----------------------------------------------------------------------------
// Add command to history
//-----------------------------------------------------------------------------
void CQuakeConsole::AddToHistory(const char *cmd)
{
	if (!cmd || !cmd[0])
		return;

	// Don't add duplicates
	if (m_InputHistory.Count() > 0)
	{
		int lastIdx = m_InputHistory.Count() - 1;
		if (Q_strcmp(m_InputHistory[lastIdx], cmd) == 0)
			return;
	}

	// Add to history
	char *cmdCopy = new char[Q_strlen(cmd) + 1];
	Q_strcpy(cmdCopy, cmd);
	m_InputHistory.AddToTail(cmdCopy);

	// Limit history size
	while (m_InputHistory.Count() > MAX_INPUT_HISTORY)
	{
		delete[] m_InputHistory[0];
		m_InputHistory.Remove(0);
	}
}

//-----------------------------------------------------------------------------
// Update animation
//-----------------------------------------------------------------------------
void CQuakeConsole::UpdateAnimation()
{
	if (!m_bAnimating)
		return;

	float elapsed = realtime - m_flAnimationStart;
	float progress = elapsed / ANIMATION_TIME;

	if (progress >= 1.0f)
	{
		// Animation complete
		progress = 1.0f;
		m_bAnimating = false;
	}

	// Smooth animation curve
	progress = 0.5f * (1.0f - cos(progress * M_PI));

	if (m_bVisible)
	{
		m_flCurrentOffset = progress;
	}
	else
	{
		m_flCurrentOffset = 1.0f - progress;
	}
}

//-----------------------------------------------------------------------------
// Get animation progress
//-----------------------------------------------------------------------------
float CQuakeConsole::GetAnimationProgress() const
{
	return m_flCurrentOffset;
}

//-----------------------------------------------------------------------------
// Get current console Y position
//-----------------------------------------------------------------------------
int CQuakeConsole::GetConsoleY() const
{
	return (int)(-m_nConsoleHeight + (m_flCurrentOffset * m_nConsoleHeight));
}
