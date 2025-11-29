//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <stdio.h>

#include "BasePanel.h"
#include "Taskbar.h"
#include "EngineInterface.h"

#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>

#include <KeyValues.h>
#include <FileSystem.h>
#include <vgui_controls/Controls.h>

#include "GameConsole.h"
#include "GameUI_Interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBasePanel::CBasePanel() : Panel(NULL, "BasePanel")
{
	m_eBackgroundState = BACKGROUND_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Notifies the task bar that a new top-level panel has been created
//-----------------------------------------------------------------------------
void CBasePanel::OnChildAdded(VPANEL child)
{
	if (g_pTaskbar)
	{
		g_pTaskbar->AddTask(child);
	}
}

//-----------------------------------------------------------------------------
// Purpose: paints the main background image
//-----------------------------------------------------------------------------
void CBasePanel::PaintBackground()
{
	bool bIsInLevel = GameUI().IsInLevel();
	bool bIsInBackgroundLevel = GameUI().IsInBackgroundLevel();

	if ( bIsInLevel )
	{
		// render filled background in game
		int swide, stall;
		surface()->GetScreenSize(swide, stall);
		surface()->DrawSetColor(0, 0, 0, 128);
		surface()->DrawFilledRect(0, 0, swide, stall);		
		return;	
	}
	else if ( bIsInBackgroundLevel )
	{
		return;
	}

	switch (m_eBackgroundState)
	{
	case BACKGROUND_BLACK:
		{
			// if the loading dialog is visible, draw the background black
			int swide, stall;
			surface()->GetScreenSize(swide, stall);
			surface()->DrawSetColor(0, 0, 0, 255);
			surface()->DrawFilledRect(0, 0, swide, stall);		
		}
		break;

	case BACKGROUND_LOADING:
		DrawBackgroundImage();
		break;

	case BACKGROUND_DESKTOPIMAGE:
		DrawBackgroundImage();
		break;
		
	case BACKGROUND_LOADINGTRANSITION:
		{
		}
		break;

	case BACKGROUND_NONE:
	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Gets the mod name from the game directory path
//-----------------------------------------------------------------------------
static const char *GetModName()
{
	static char modName[64] = "";

	if (modName[0] == '\0')
	{
		const char *fullGamePath = engine->GetGameDirectory();
		if (fullGamePath && fullGamePath[0])
		{
			// Find the last path separator
			const char *pathSep = strrchr(fullGamePath, '/');
			if (!pathSep)
			{
				pathSep = strrchr(fullGamePath, '\\');
			}

			if (pathSep)
			{
				Q_strncpy(modName, pathSep + 1, sizeof(modName));
			}
			else
			{
				Q_strncpy(modName, fullGamePath, sizeof(modName));
			}
		}
	}

	return modName;
}

//-----------------------------------------------------------------------------
// Purpose: Gets the background name from scripts/ChapterBackgrounds.txt
// Similar to 2007 engine's CL_GetBackgroundLevelName
//-----------------------------------------------------------------------------
static void GetBackgroundName(char *pszBackgroundName, int bufSize)
{
	// Default background name
	Q_strncpy(pszBackgroundName, "background01", bufSize);

	// Try to load from ChapterBackgrounds.txt (like 2007 engine)
	KeyValues *pChapterFile = new KeyValues("ChapterBackgrounds");
	if (pChapterFile->LoadFromFile(vgui::filesystem(), "scripts/ChapterBackgrounds.txt", "GAME"))
	{
		// Get the first key which should be the default background
		KeyValues *pChapters = pChapterFile->GetFirstSubKey();
		if (pChapters)
		{
			// Just use the first background found for simplicity
			// The 2007 engine tracks game progress, but for LeakNet we'll just use the first one
			const char *bgName = pChapters->GetString();
			if (bgName && bgName[0])
			{
				Q_strncpy(pszBackgroundName, bgName, bufSize);
			}
		}
	}
	pChapterFile->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Tries to load a background texture
// Returns true if successful
//-----------------------------------------------------------------------------
static bool TryLoadBackgroundTexture(int imageID, const char *texturePath, bool hardwareFilter)
{
	// Check if the texture file exists
	char vmtPath[512];
	char vtfPath[512];
	Q_snprintf(vmtPath, sizeof(vmtPath), "materials/%s.vmt", texturePath);
	Q_snprintf(vtfPath, sizeof(vtfPath), "materials/%s.vtf", texturePath);

	if (vgui::filesystem()->FileExists(vmtPath, "GAME") ||
		vgui::filesystem()->FileExists(vtfPath, "GAME"))
	{
		surface()->DrawSetTextureFile(imageID, texturePath, hardwareFilter, false);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: loads background texture
// Tries to load mod-specific background first, then falls back to default
// Based on 2007 Source Engine background loading logic
//-----------------------------------------------------------------------------
void CBasePanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// Turn on hardware filtering if we're scaling the images
	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	bool hardwareFilter = false;

	// Calculate aspect ratio for widescreen detection
	float aspectRatio = (float)wide / (float)tall;
	bool bIsWidescreen = (aspectRatio >= 1.5999f);

	bimage_t &bimage = m_ImageID[0][0];
	bimage.imageID = surface()->CreateNewTextureID();

	char filename[512];
	bool bFoundBackground = false;

	// Get the mod name
	const char *modName = GetModName();

	// Get the background name from ChapterBackgrounds.txt
	char backgroundName[MAX_PATH];
	GetBackgroundName(backgroundName, sizeof(backgroundName));

	// Build list of paths to try, in order of preference
	// 1. ChapterBackgrounds.txt specified background (with widescreen variant)
	// 2. Mod-specific background: console/<modname>_background
	// 3. Generic background: console/background
	// 4. Default: console/background01 or console/console_background

	// Try 1: ChapterBackgrounds.txt background (widescreen first if applicable)
	if (!bFoundBackground && backgroundName[0])
	{
		if (bIsWidescreen)
		{
			Q_snprintf(filename, sizeof(filename), "console/%s_widescreen", backgroundName);
			bFoundBackground = TryLoadBackgroundTexture(bimage.imageID, filename, hardwareFilter);
		}

		if (!bFoundBackground)
		{
			Q_snprintf(filename, sizeof(filename), "console/%s", backgroundName);
			bFoundBackground = TryLoadBackgroundTexture(bimage.imageID, filename, hardwareFilter);
		}
	}

	// Try 2: Mod-specific background
	if (!bFoundBackground && modName && modName[0])
	{
		if (bIsWidescreen)
		{
			Q_snprintf(filename, sizeof(filename), "console/%s_background_widescreen", modName);
			bFoundBackground = TryLoadBackgroundTexture(bimage.imageID, filename, hardwareFilter);
		}

		if (!bFoundBackground)
		{
			Q_snprintf(filename, sizeof(filename), "console/%s_background", modName);
			bFoundBackground = TryLoadBackgroundTexture(bimage.imageID, filename, hardwareFilter);
		}
	}

	// Try 3: Generic console/background
	if (!bFoundBackground)
	{
		if (bIsWidescreen)
		{
			Q_snprintf(filename, sizeof(filename), "console/background_widescreen");
			bFoundBackground = TryLoadBackgroundTexture(bimage.imageID, filename, hardwareFilter);
		}

		if (!bFoundBackground)
		{
			Q_snprintf(filename, sizeof(filename), "console/background");
			bFoundBackground = TryLoadBackgroundTexture(bimage.imageID, filename, hardwareFilter);
		}
	}

	// Try 4a: Default background01
	if (!bFoundBackground)
	{
		if (bIsWidescreen)
		{
			Q_snprintf(filename, sizeof(filename), "console/background01_widescreen");
			bFoundBackground = TryLoadBackgroundTexture(bimage.imageID, filename, hardwareFilter);
		}

		if (!bFoundBackground)
		{
			Q_snprintf(filename, sizeof(filename), "console/background01");
			bFoundBackground = TryLoadBackgroundTexture(bimage.imageID, filename, hardwareFilter);
		}
	}

	// Try 4b: Legacy console_background
	if (!bFoundBackground)
	{
		Q_snprintf(filename, sizeof(filename), "console/console_background");
		bFoundBackground = TryLoadBackgroundTexture(bimage.imageID, filename, hardwareFilter);
	}

	// Final fallback - just load whatever we last tried
	if (!bFoundBackground)
	{
		surface()->DrawSetTextureFile(bimage.imageID, filename, hardwareFilter, false);
	}

	// Get the texture size
	surface()->DrawGetTextureSize(bimage.imageID, bimage.width, bimage.height);

	DevMsg("BasePanel: Loaded background '%s' (%dx%d) for mod '%s'\n",
		filename, bimage.width, bimage.height, modName ? modName : "unknown");
}

//-----------------------------------------------------------------------------
// Purpose: sets how the game background should render
//-----------------------------------------------------------------------------
void CBasePanel::SetBackgroundRenderState(EBackgroundState state)
{
	m_eBackgroundState = state;
}

//-----------------------------------------------------------------------------
// Purpose: draws the background desktop image
//-----------------------------------------------------------------------------
void CBasePanel::DrawBackgroundImage()
{
//	int xpos, ypos;
	int wide, tall;
	GetSize(wide, tall);

	// work out scaling factors
	int swide, stall;
	surface()->GetScreenSize(swide, stall);
	float xScale, yScale;
//	xScale = swide / 800.0f;
//	yScale = stall / 600.0f;
	xScale = 1.0f;
	yScale = 1.0f;
/*
	// iterate and draw all the background pieces
	ypos = 0;
	for (int y = 0; y < BACKGROUND_ROWS; y++)
	{
		xpos = 0;
		for (int x = 0; x < BACKGROUND_COLUMNS; x++)
		{
			bimage_t &bimage = m_ImageID[y][x];

			int dx = (int)ceil(xpos * xScale);
			int dy = (int)ceil(ypos * yScale);
			int dw = (int)ceil((xpos + bimage.width) * xScale);
			int dt = (int)ceil((ypos + bimage.height) * yScale);

			if (x == 0)
			{
				dx = 0;
			}
			if (y == 0)
			{
				dy = 0;
			}

  */
			bimage_t &bimage = m_ImageID[0][0];
			// draw the color image only if the mono image isn't yet fully opaque
			surface()->DrawSetColor(255, 255, 255, 255);
			surface()->DrawSetTexture(bimage.imageID);
			surface()->DrawTexturedRect(0, 0, wide, tall);

		/*	xpos += bimage.width;
		}
		ypos += m_ImageID[y][0].height;
	}
	*/
}
