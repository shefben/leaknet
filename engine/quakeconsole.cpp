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

#ifdef _WIN32
#include <windows.h>
// Undefine Windows macros that conflict with method names
#ifdef DrawText
#undef DrawText
#endif
#else
// BMP file structures for non-Windows platforms
#pragma pack(push, 1)
typedef struct {
	unsigned short bfType;
	unsigned int   bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int   bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
	unsigned int   biSize;
	int            biWidth;
	int            biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int   biCompression;
	unsigned int   biSizeImage;
	int            biXPelsPerMeter;
	int            biYPelsPerMeter;
	unsigned int   biClrUsed;
	unsigned int   biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)
#endif

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
	m_bTextureLoadAttempted = false;
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

	// Note: Background texture loading is deferred to first paint
	// when VGUI surface is guaranteed to be ready

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

	// Safety check - VGUI surface must be available
	if (!vgui::surface())
		return;

	// Update screen dimensions in case resolution changed
	if (vid.width > 0 && vid.height > 0)
	{
		m_nConsoleWidth = vid.width;
		m_nConsoleHeight = vid.height / 2;
	}

	UpdateAnimation();

	// Get the embedded panel for drawing context
	vgui::VPANEL embeddedPanel = vgui::surface()->GetEmbeddedPanel();
	if (!embeddedPanel)
	{
		// No embedded panel available, can't draw
		return;
	}

	// Start VGUI drawing context if not already active
	// This ensures we can draw even if called outside normal VGUI paint cycle
	vgui::surface()->PushMakeCurrent( embeddedPanel, false );

	// Draw background
	DrawBackground();

	// Draw console text
	DrawText();

	// Draw input line
	DrawInputLine();

	// Finish the drawing context
	vgui::surface()->PopMakeCurrent( embeddedPanel );
}

//-----------------------------------------------------------------------------
// Load background texture
// Looks for console background in gfx/shell/console_background.bmp (mod path)
// Falls back to materials (VTF/VMT) if BMP not found
//-----------------------------------------------------------------------------
void CQuakeConsole::LoadBackgroundTexture()
{
	// Default to no texture
	m_nTextureId = -1;
	m_nTextureWidth = 0;
	m_nTextureHeight = 0;

	// Safety checks - VGUI surface must be available
	if (!vgui::surface())
		return;

	// Try to load BMP from gfx/shell/console_background.bmp first (GoldSrc/GMod style)
	const char *bmpPaths[] = {
		"gfx/shell/console_background.bmp",  // Primary mod-specific path
		"gfx/shell/conback.bmp",             // Alternative name
		NULL
	};

	for (int i = 0; bmpPaths[i] != NULL; i++)
	{
		FileHandle_t hFile = g_pFileSystem->Open(bmpPaths[i], "rb", "GAME");
		if (hFile)
		{
			// Read BMP header
			BITMAPFILEHEADER bmfh;
			BITMAPINFOHEADER bmih;

			g_pFileSystem->Read(&bmfh, sizeof(bmfh), hFile);
			g_pFileSystem->Read(&bmih, sizeof(bmih), hFile);

			// Verify it's a valid BMP
			if (bmfh.bfType == 0x4D42) // 'BM'
			{
				int width = bmih.biWidth;
				int height = abs(bmih.biHeight);  // Height can be negative for top-down BMP
				bool bottomUp = (bmih.biHeight > 0);
				int bpp = bmih.biBitCount;

				if ((bpp == 24 || bpp == 32) && width > 0 && height > 0)
				{
					// Allocate RGBA buffer
					unsigned char *rgba = new unsigned char[width * height * 4];
					if (rgba)
					{
						// Seek to pixel data
						g_pFileSystem->Seek(hFile, bmfh.bfOffBits, FILESYSTEM_SEEK_HEAD);

						// Calculate row size with padding (BMP rows are 4-byte aligned)
						int bytesPerPixel = bpp / 8;
						int rowSize = ((width * bytesPerPixel + 3) / 4) * 4;
						unsigned char *rowBuffer = new unsigned char[rowSize];

						if (rowBuffer)
						{
							for (int y = 0; y < height; y++)
							{
								// Determine which row to write to (flip if bottom-up)
								int destY = bottomUp ? (height - 1 - y) : y;

								g_pFileSystem->Read(rowBuffer, rowSize, hFile);

								for (int x = 0; x < width; x++)
								{
									int srcIdx = x * bytesPerPixel;
									int destIdx = (destY * width + x) * 4;

									// BMP stores BGR, convert to RGBA
									rgba[destIdx + 0] = rowBuffer[srcIdx + 2];  // R
									rgba[destIdx + 1] = rowBuffer[srcIdx + 1];  // G
									rgba[destIdx + 2] = rowBuffer[srcIdx + 0];  // B
									rgba[destIdx + 3] = (bpp == 32) ? rowBuffer[srcIdx + 3] : 255;  // A
								}
							}

							// Create texture from RGBA data
							m_nTextureId = vgui::surface()->CreateNewTextureID(true);  // Procedural texture
							vgui::surface()->DrawSetTextureRGBA(m_nTextureId, rgba, width, height, 0, false);

							m_nTextureWidth = width;
							m_nTextureHeight = height;

							delete[] rowBuffer;
							delete[] rgba;
							g_pFileSystem->Close(hFile);

							Msg("Console: Loaded background BMP '%s' (%dx%d)\n", bmpPaths[i], width, height);
							return;
						}

						delete[] rgba;
					}
				}
			}

			g_pFileSystem->Close(hFile);
		}
	}

	// BMP not found - try material system (VTF/VMT) as fallback
	const char *materialPaths[] = {
		"vgui/console/console_background",  // Mod-specific or game console background
		"console/background",               // Alternative path
		"vgui/splash",                      // Splash screen as fallback
		NULL
	};

	m_nTextureId = vgui::surface()->CreateNewTextureID(false);  // Non-procedural texture

	for (int i = 0; materialPaths[i] != NULL; i++)
	{
		// Try to load the material - DrawSetTextureFile handles VTF/VMT lookup
		vgui::surface()->DrawSetTextureFile(m_nTextureId, materialPaths[i], 0, false);

		// Check if the texture loaded successfully by getting its size
		vgui::surface()->DrawGetTextureSize(m_nTextureId, m_nTextureWidth, m_nTextureHeight);

		if (m_nTextureWidth > 0 && m_nTextureHeight > 0)
		{
			Msg("Console: Loaded background material '%s' (%dx%d)\n",
				materialPaths[i], m_nTextureWidth, m_nTextureHeight);
			return;
		}
	}

	// No material found - texture will stay invalid, DrawBackground will use solid color
	Msg("Console: No background material found, using solid background\n");
	m_nTextureId = -1;
	m_nTextureWidth = 0;
	m_nTextureHeight = 0;
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

	// Lazy load the background texture on first draw (VGUI is guaranteed ready here)
	if (!m_bTextureLoadAttempted)
	{
		m_bTextureLoadAttempted = true;
		LoadBackgroundTexture();
	}

	int y = GetConsoleY();

	// Draw textured background if available - no transparency, solid image
	if (m_nTextureId != -1 && m_nTextureWidth > 0 && m_nTextureHeight > 0)
	{
		// Draw the splash texture stretched to fit console area
		vgui::surface()->DrawSetTexture(m_nTextureId);
		vgui::surface()->DrawSetColor(255, 255, 255, 255);
		vgui::surface()->DrawTexturedRect(0, y, m_nConsoleWidth, y + m_nConsoleHeight);
	}
	else
	{
		// Fallback: solid black background (no transparency)
		vgui::surface()->DrawSetColor(0, 0, 0, 255);
		vgui::surface()->DrawFilledRect(0, y, m_nConsoleWidth, y + m_nConsoleHeight);
	}

	// Draw bottom border line (like WON console)
	vgui::surface()->DrawSetColor(128, 128, 128, 255);
	vgui::surface()->DrawFilledRect(0, y + m_nConsoleHeight - 2, m_nConsoleWidth, y + m_nConsoleHeight);
}

//-----------------------------------------------------------------------------
// Ensure font is loaded - called lazily since scheme may not be ready at init
//-----------------------------------------------------------------------------
void CQuakeConsole::EnsureFontLoaded()
{
	// If we already have a valid font, nothing to do
	if (m_hFont != 0)
		return;

	// Try to load the font from scheme
	if (vgui::scheme())
	{
		vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(vgui::scheme()->GetDefaultScheme());
		if (pScheme)
		{
			m_hFont = pScheme->GetFont("DefaultFixed", false);
			if (!m_hFont)
			{
				// Try alternate font names
				m_hFont = pScheme->GetFont("DefaultSmall", false);
			}
			if (!m_hFont)
			{
				m_hFont = pScheme->GetFont("Default", false);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Draw console text
//-----------------------------------------------------------------------------
void CQuakeConsole::DrawText()
{
	// Try to ensure font is loaded
	EnsureFontLoaded();

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
// Draw input line at the very bottom of the console
//-----------------------------------------------------------------------------
void CQuakeConsole::DrawInputLine()
{
	// Try to ensure font is loaded
	EnsureFontLoaded();

	if (!m_hFont || !vgui::surface() || !vgui::localize())
		return;

	int y = GetConsoleY();
	int fontTall = vgui::surface()->GetFontTall(m_hFont);
	if (fontTall <= 0)
		return;

	// Position input line at the very bottom of the console (above the border line)
	int inputY = y + m_nConsoleHeight - fontTall - 4;

	vgui::surface()->DrawSetTextFont(m_hFont);

	// Draw prompt character "]"
	vgui::surface()->DrawSetTextColor(255, 255, 255, 255);
	vgui::surface()->DrawSetTextPos(8, inputY);
	wchar_t prompt[4];
	vgui::localize()->ConvertANSIToUnicode("]", prompt, sizeof(prompt));
	vgui::surface()->DrawPrintText(prompt, 1);

	// Get prompt width to position input text after it
	int promptWide = 0, promptTall = 0;
	vgui::surface()->GetTextSize(m_hFont, prompt, promptWide, promptTall);
	int textStartX = 8 + promptWide;

	// Draw input text
	if (m_nInputLength > 0)
	{
		vgui::surface()->DrawSetTextPos(textStartX, inputY);
		wchar_t wideInput[INPUT_LINE_LENGTH];
		vgui::localize()->ConvertANSIToUnicode(m_szInputLine, wideInput, sizeof(wideInput));
		vgui::surface()->DrawPrintText(wideInput, m_nInputLength);
	}

	// Draw blinking cursor (block cursor like classic console)
	static float lastBlinkTime = 0;
	static bool bShowCursor = true;

	if (realtime - lastBlinkTime > 0.4f)
	{
		bShowCursor = !bShowCursor;
		lastBlinkTime = realtime;
	}

	if (bShowCursor)
	{
		int cursorX = textStartX;
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

		// Draw a block cursor (8 pixels wide, like classic console)
		vgui::surface()->DrawSetColor(255, 255, 255, 255);
		vgui::surface()->DrawFilledRect(cursorX, inputY, cursorX + 8, inputY + fontTall);
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
