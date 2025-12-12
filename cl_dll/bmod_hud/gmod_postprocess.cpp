// Garry's Mod style post-process overlay + motion blur for the 2003 Source engine.
#include "cbase.h"

#ifdef BMOD_CLIENT_DLL

#include "gmod_postprocess.h"
#include "viewrender.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "vstdlib/strtools.h"
#include "iviewrender.h"
#include "view_shared.h"
#include <ctype.h>

// ConVars mirror the behavior seen in the original gmod 9 client.dll
static ConVar gm_motionblur_enable(
	"gm_motionblur_enable",
	"1",
	FCVAR_CLIENTDLL | FCVAR_ARCHIVE,
	"Enable Garry's Mod style motion blur overlay");

static ConVar gm_motionblur_strength(
	"gm_motionblur_strength",
	"0.85",
	FCVAR_CLIENTDLL | FCVAR_ARCHIVE,
	"Motion blur alpha strength",
	true, 0.0f, true, 1.0f);

static ConVar gm_overlay_enable(
	"gm_overlay_enable",
	"1",
	FCVAR_CLIENTDLL | FCVAR_ARCHIVE,
	"Enable Garry's Mod screen overlay from settings/gmod_overlay.txt");

static ConVar gm_shiny_overlay(
	"gm_shiny_overlay",
	"0",
	FCVAR_CLIENTDLL | FCVAR_ARCHIVE,
	"Force gmod/shiny as the screen overlay (overrides gm_overlay_enable when set)");

static IMaterial *s_pMotionBlurMat = NULL;
static IMaterial *s_pOverlayMat = NULL;
static IMaterial *s_pShinyMat = NULL;
static bool s_bOverlayLoaded = false;
static char s_OverlayMatName[MAX_PATH] = {0};

void GModPostProcess_Init()
{
	if (s_pMotionBlurMat)
		return;

	if (!materials)
		return;

	bool dummy;
	IMaterial *pMat = materials->FindMaterial("gmod/motionblur", &dummy, false);
	if (pMat)
	{
		s_pMotionBlurMat = pMat;
	}
}

void GModPostProcess_Shutdown()
{
	s_pMotionBlurMat = NULL;
	s_pOverlayMat = NULL;
	s_pShinyMat = NULL;
	s_bOverlayLoaded = false;
	s_OverlayMatName[0] = '\0';
}

static void ApplyMotionBlurSettings()
{
	if (!s_pMotionBlurMat)
		return;

	IMaterialVar *pAlpha = s_pMotionBlurMat->FindVar("$alpha", nullptr, false);
	if (pAlpha)
	{
		pAlpha->SetFloatValue(gm_motionblur_strength.GetFloat());
	}
}

static void LoadOverlayMaterialName()
{
	if (s_bOverlayLoaded)
		return;

	s_bOverlayLoaded = true; // only try once per session

	// Attempt to read settings/gmod_overlay.txt (KeyValues preferred, fallback to first non-empty line).
	KeyValues *pKV = new KeyValues("GMKVOverlay");
	if (pKV->LoadFromFile(filesystem, "settings/gmod_overlay.txt", "MOD"))
	{
		const char *matName = pKV->GetString("overlay", pKV->GetString("material", ""));
		if (!matName || !matName[0])
		{
			// Try first subkey if overlay/material not present.
			for (KeyValues *sub = pKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				if (sub->GetName() && sub->GetName()[0])
				{
					matName = sub->GetName();
					break;
				}
			}
		}

		if (matName && matName[0])
		{
			Q_strncpy(s_OverlayMatName, matName, sizeof(s_OverlayMatName));
		}
	}
	pKV->deleteThis();

	// Fallback: read first non-empty line.
	if (!s_OverlayMatName[0])
	{
		FileHandle_t f = filesystem->Open("settings/gmod_overlay.txt", "r", "MOD");
		if (f != FILESYSTEM_INVALID_HANDLE)
		{
			char line[256];
			while (filesystem->ReadLine(line, sizeof(line), f))
			{
				// Basic trim of leading spaces
				char *pLine = line;
				while (*pLine && isspace((unsigned char)*pLine)) { ++pLine; }
				if (pLine[0] && pLine[0] != '/' && pLine[0] != '#')
				{
					Q_strncpy(s_OverlayMatName, pLine, sizeof(s_OverlayMatName));
					break;
				}
			}
			filesystem->Close(f);
		}
	}
}

static void EnsureOverlayMaterial()
{
	if (s_pOverlayMat)
		return;

	LoadOverlayMaterialName();
	if (!s_OverlayMatName[0] || !materials)
		return;

	bool dummy;
	IMaterial *pMat = materials->FindMaterial(s_OverlayMatName, &dummy, false);
	if (pMat)
	{
		s_pOverlayMat = pMat;
	}
}

static void EnsureShinyMaterial()
{
	if (s_pShinyMat)
		return;

	if (!materials)
		return;

	bool dummy;
	IMaterial *pMat = materials->FindMaterial("gmod/shiny", &dummy, false);
	if (pMat)
	{
		s_pShinyMat = pMat;
	}
}

void GModPostProcess_Update(CViewRender &view)
{
	if (gm_shiny_overlay.GetBool())
	{
		EnsureShinyMaterial();
		// Overlay setting unavailable on this engine; skip if not found.
		if (s_pShinyMat)
			return;
	}

	// Overlay has priority when enabled; otherwise fallback to motion blur.
	if (gm_overlay_enable.GetBool())
	{
		EnsureOverlayMaterial();
		if (s_pOverlayMat)
			return;
	}

	if (!gm_motionblur_enable.GetBool())
	{
		return;
	}

	if (!s_pMotionBlurMat)
	{
		GModPostProcess_Init();
	}

	if (!s_pMotionBlurMat)
		return;

	ApplyMotionBlurSettings();
	// No public overlay setter; engine lacks this hook, so nothing further to do.
}

#endif // BMOD_CLIENT_DLL
