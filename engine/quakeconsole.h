//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// Purpose: Quake-style sliding console implementation
//
//=============================================================================

#ifndef QUAKECONSOLE_H
#define QUAKECONSOLE_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"
#include "conprint.h"
#include <vgui/ISurface.h>

class CQuakeConsole
{
public:
	CQuakeConsole();
	~CQuakeConsole();

	// Initialize the console
	void Initialize();
	void Shutdown();

	// Console visibility and animation
	void Show();
	void Hide();
	bool IsVisible() const { return m_bVisible || m_bAnimating; }

	// Text output
	void Print(const char *msg);
	void DPrint(const char *msg);
	void ColorPrint(Color& clr, const char *msg);
	void Clear();

	// Input handling
	void HandleKeyInput(int key, bool bDown);
	void HandleCharInput(int unichar);

	// Rendering
	void Paint();

private:
	enum
	{
		MAX_CONSOLE_LINES = 512,
		MAX_INPUT_HISTORY = 100,
		CONSOLE_LINE_LENGTH = 256,
		INPUT_LINE_LENGTH = 256,
	};

	// Seconds the slide animation takes to complete.
	static const float ANIMATION_TIME;

	struct ConsoleLine
	{
		Color color;
		char text[CONSOLE_LINE_LENGTH];
		ConsoleLine() { color = Color(255, 255, 255, 255); text[0] = '\0'; }
	};

	// Console state
	bool m_bInitialized;
	bool m_bVisible;
	bool m_bAnimating;
	float m_flAnimationStart;
	float m_flCurrentOffset;  // 0.0 = fully hidden, 1.0 = fully shown

	// Text storage
	CUtlVector<ConsoleLine> m_ConsoleLines;
	int m_nScrollOffset;

	// Input handling
	char m_szInputLine[INPUT_LINE_LENGTH];
	int m_nInputPos;
	int m_nInputLength;
	CUtlVector<char*> m_InputHistory;
	int m_nHistoryPos;

	// Rendering
	int m_nTextureId;  // splash.bmp texture
	vgui::HFont m_hFont;
	int m_nConsoleHeight;  // Half screen height
	int m_nConsoleWidth;   // Full screen width

	// Private methods
	void LoadBackgroundTexture();
	void AddLine(Color& clr, const char *text);
	void DrawBackground();
	void DrawText();
	void DrawInputLine();
	void ScrollUp();
	void ScrollDown();
	void ExecuteCommand();
	void AddToHistory(const char *cmd);
	void UpdateAnimation();
	float GetAnimationProgress() const;
	int GetConsoleY() const;  // Current Y position based on animation
};

// Global instance
extern CQuakeConsole *g_pQuakeConsole;

#endif // QUAKECONSOLE_H
