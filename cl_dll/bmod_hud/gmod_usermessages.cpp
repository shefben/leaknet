//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GMod-specific usermessage handlers for the 2003 client protocol.
// Handles GModText, GModRect, WQuad, and related display messages.
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "gmod_message.h"
#include "gmod_teammenu.h"
#include "usermessages.h"
#include "parsemsg.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Panel.h>
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/itexture.h"
#include "view.h"
#include "cdll_util.h"
#include "cliententitylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// GMod Rect with Material Support
//=============================================================================

static const char *ResolveGModHudFontName(const char *fontName)
{
	if (!fontName || !*fontName)
		return "Default";

	if (!Q_stricmp(fontName, "ImpactMassive") || !Q_stricmp(fontName, "TrebuchetMassive"))
		return "DefaultShadow";

	if (Q_strnicmp(fontName, "Impact", 6) == 0)
		return "DefaultShadow";

	return fontName;
}

class CGModMaterialRect : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CGModMaterialRect, vgui::Panel);

public:
	CGModMaterialRect(vgui::Panel *parent, const char *name, int rectID);
	virtual ~CGModMaterialRect();

	virtual void Paint();
	virtual void OnThink();

	void SetMaterial(const char *materialPath);
	void SetRectPosition(float x, float y, float w, float h);
	void SetColor(int r, int g, int b, int a);
	void SetTiming(float holdTime, float fadeIn, float fadeOut, float delay);
	void SetTargetPosition(float x, float y, float w, float h);
	void StartAnimation(float duration, float easeAmount);
	void Show();
	void Hide(float fadeTime = 0.0f, float delay = 0.0f);

	int GetRectID() const { return m_iRectID; }
	bool IsExpired() const;

private:
	int m_iRectID;
	char m_szMaterialPath[256];
	IMaterial *m_pMaterial;
	int m_iTextureID;

	// Position (screen-relative 0-1)
	float m_flX, m_flY, m_flW, m_flH;

	// Color
	Color m_Color;

	// Timing
	float m_flHoldTime;
	float m_flFadeIn;
	float m_flFadeOut;
	float m_flDelay;
	float m_flStartTime;
	bool m_bVisible;

	// Animation
	bool m_bAnimating;
	float m_flAnimStartTime;
	float m_flAnimDuration;
	float m_flAnimEase;
	float m_flStartX, m_flStartY, m_flStartW, m_flStartH;
	float m_flTargetX, m_flTargetY, m_flTargetW, m_flTargetH;

	// Fade out
	bool m_bFadingOut;
	float m_flFadeOutStart;
	float m_flFadeOutDuration;
};

//-----------------------------------------------------------------------------
CGModMaterialRect::CGModMaterialRect(vgui::Panel *parent, const char *name, int rectID)
	: BaseClass(parent, name)
{
	m_iRectID = rectID;
	m_szMaterialPath[0] = '\0';
	m_pMaterial = NULL;
	m_iTextureID = -1;
	m_flX = m_flY = 0;
	m_flW = m_flH = 100;
	m_Color = Color(255, 255, 255, 255);
	m_flHoldTime = 5.0f;
	m_flFadeIn = 0.1f;
	m_flFadeOut = 0.1f;
	m_flDelay = 0;
	m_flStartTime = 0;
	m_bVisible = false;
	m_bAnimating = false;
	m_bFadingOut = false;

	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetVisible(false);

	// Fill the full-screen parent so absolute-positioned rects aren't clipped away by
	// a 0x0 panel (see CGModTextDisplay ctor for why this matters).
	int screenWide, screenTall;
	vgui::surface()->GetScreenSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);
}

//-----------------------------------------------------------------------------
CGModMaterialRect::~CGModMaterialRect()
{
	// Note: 2003 ISurface doesn't have DestroyTextureID
	// Texture IDs are managed internally by the surface
}

//-----------------------------------------------------------------------------
void CGModMaterialRect::SetMaterial(const char *materialPath)
{
	if (!materialPath || !*materialPath)
	{
		m_szMaterialPath[0] = '\0';
		m_pMaterial = NULL;
		return;
	}

	char normalizedPath[256];
	Q_strncpy(normalizedPath, materialPath, sizeof(normalizedPath));
	for (char *p = normalizedPath; *p; ++p)
	{
		if (*p == '\\')
			*p = '/';
	}

	const char *lookupPath = normalizedPath;
	if (!Q_strnicmp(lookupPath, "materials/", 10))
		lookupPath += 10;

	char materialName[256];
	Q_strncpy(materialName, lookupPath, sizeof(materialName));
	char *pExt = Q_stristr(materialName, ".vmt");
	if (!pExt)
		pExt = Q_stristr(materialName, ".vtf");
	if (pExt)
		*pExt = '\0';

	Q_strncpy(m_szMaterialPath, materialName, sizeof(m_szMaterialPath));

	// Try to load the material (2003 engine uses NULL for texture group)
	m_pMaterial = materials->FindMaterial(m_szMaterialPath, NULL);

	if (m_szMaterialPath[0])
	{
		// Create a texture ID for VGUI rendering
		if (m_iTextureID < 0)
		{
			m_iTextureID = vgui::surface()->CreateNewTextureID();
		}

		// The VGUI surface hands this name straight to FindMaterial, which wants
		// the extensionless material name relative to materials/ - passing a
		// "materials/x.vtf" path fails the lookup and draws the checkerboard.
		vgui::surface()->DrawSetTextureFile(m_iTextureID, m_szMaterialPath, true, false);
	}
}

//-----------------------------------------------------------------------------
void CGModMaterialRect::SetRectPosition(float x, float y, float w, float h)
{
	m_flX = x;
	m_flY = y;
	m_flW = w;
	m_flH = h;
}

//-----------------------------------------------------------------------------
void CGModMaterialRect::SetColor(int r, int g, int b, int a)
{
	m_Color.SetColor(r, g, b, a);
}

//-----------------------------------------------------------------------------
void CGModMaterialRect::SetTiming(float holdTime, float fadeIn, float fadeOut, float delay)
{
	m_flHoldTime = holdTime;
	m_flFadeIn = fadeIn;
	m_flFadeOut = fadeOut;
	m_flDelay = delay;
}

//-----------------------------------------------------------------------------
void CGModMaterialRect::SetTargetPosition(float x, float y, float w, float h)
{
	m_flTargetX = x;
	m_flTargetY = y;
	m_flTargetW = w;
	m_flTargetH = h;
}

//-----------------------------------------------------------------------------
void CGModMaterialRect::StartAnimation(float duration, float easeAmount)
{
	m_bAnimating = true;
	m_flAnimStartTime = gpGlobals->curtime;
	m_flAnimDuration = duration;
	m_flAnimEase = easeAmount;
	m_flStartX = m_flX;
	m_flStartY = m_flY;
	m_flStartW = m_flW;
	m_flStartH = m_flH;
}

//-----------------------------------------------------------------------------
void CGModMaterialRect::Show()
{
	m_bVisible = true;
	m_flStartTime = gpGlobals->curtime;
	m_bFadingOut = false;
	SetVisible(true);
}

//-----------------------------------------------------------------------------
void CGModMaterialRect::Hide(float fadeTime, float delay)
{
	if (fadeTime > 0 || delay > 0)
	{
		m_bFadingOut = true;
		m_flFadeOutStart = gpGlobals->curtime + delay;
		m_flFadeOutDuration = fadeTime;
	}
	else
	{
		m_bVisible = false;
		SetVisible(false);
	}
}

//-----------------------------------------------------------------------------
bool CGModMaterialRect::IsExpired() const
{
	if (!m_bVisible)
		return true;

	if (m_flHoldTime <= 0)
		return false; // Infinite duration

	float elapsed = gpGlobals->curtime - (m_flStartTime + m_flDelay);
	return elapsed >= (m_flHoldTime + m_flFadeIn + m_flFadeOut);
}

//-----------------------------------------------------------------------------
void CGModMaterialRect::OnThink()
{
	BaseClass::OnThink();

	float currentTime = gpGlobals->curtime;

	// Handle delay
	if (m_bVisible && currentTime < m_flStartTime + m_flDelay)
	{
		SetVisible(false);
		return;
	}

	// Handle fade out
	if (m_bFadingOut && currentTime >= m_flFadeOutStart + m_flFadeOutDuration)
	{
		m_bVisible = false;
		SetVisible(false);
		return;
	}

	// Handle expiration
	if (IsExpired())
	{
		m_bVisible = false;
		SetVisible(false);
		return;
	}

	// Handle animation
	if (m_bAnimating)
	{
		float elapsed = currentTime - m_flAnimStartTime;
		float progress = (m_flAnimDuration > 0) ? (elapsed / m_flAnimDuration) : 1.0f;

		// Apply easing
		if (m_flAnimEase > 0)
		{
			progress = 1.0f - pow(1.0f - progress, m_flAnimEase);
		}

		if (progress >= 1.0f)
		{
			progress = 1.0f;
			m_bAnimating = false;
		}

		// Interpolate position
		m_flX = m_flStartX + (m_flTargetX - m_flStartX) * progress;
		m_flY = m_flStartY + (m_flTargetY - m_flStartY) * progress;
		m_flW = m_flStartW + (m_flTargetW - m_flStartW) * progress;
		m_flH = m_flStartH + (m_flTargetH - m_flStartH) * progress;
	}

	if (m_bVisible && !IsVisible())
		SetVisible(true);
}

//-----------------------------------------------------------------------------
void CGModMaterialRect::Paint()
{
	if (!m_bVisible)
		return;

	int screenWide, screenTall;
	vgui::surface()->GetScreenSize(screenWide, screenTall);

	// Convert normalized coordinates to screen pixels
	int x = (int)(m_flX * screenWide);
	int y = (int)(m_flY * screenTall);
	int w = (int)(m_flW * screenWide);
	int h = (int)(m_flH * screenTall);

	// Calculate alpha based on fade
	float alpha = 1.0f;
	float currentTime = gpGlobals->curtime;
	float elapsed = currentTime - (m_flStartTime + m_flDelay);

	// Fade in
	if (m_flFadeIn > 0 && elapsed < m_flFadeIn)
	{
		alpha = elapsed / m_flFadeIn;
	}
	// Fade out (from timing)
	else if (m_flHoldTime > 0 && m_flFadeOut > 0)
	{
		float fadeOutStart = m_flFadeIn + m_flHoldTime;
		if (elapsed > fadeOutStart)
		{
			float fadeOutProgress = (elapsed - fadeOutStart) / m_flFadeOut;
			alpha = 1.0f - min(fadeOutProgress, 1.0f);
		}
	}

	// Manual fade out
	if (m_bFadingOut && currentTime >= m_flFadeOutStart)
	{
		float fadeProgress = (currentTime - m_flFadeOutStart) / max(m_flFadeOutDuration, 0.001f);
		alpha = 1.0f - min(fadeProgress, 1.0f);
	}

	// Get color components (2003 Color class uses GetColor method)
	int cr, cg, cb, ca;
	m_Color.GetColor(cr, cg, cb, ca);
	int finalAlpha = (int)(ca * alpha);

	// Draw material or colored rect
	if (m_iTextureID >= 0 && m_szMaterialPath[0])
	{
		vgui::surface()->DrawSetColor(cr, cg, cb, finalAlpha);
		vgui::surface()->DrawSetTexture(m_iTextureID);
		vgui::surface()->DrawTexturedRect(x, y, x + w, y + h);
	}
	else
	{
		// Draw colored rectangle
		vgui::surface()->DrawSetColor(cr, cg, cb, finalAlpha);
		vgui::surface()->DrawFilledRect(x, y, x + w, y + h);
	}
}

//=============================================================================
// GMod Text Display
//=============================================================================
class CGModTextDisplay : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CGModTextDisplay, vgui::Panel);

public:
	CGModTextDisplay(vgui::Panel *parent, const char *name, int textID);
	virtual ~CGModTextDisplay();

	virtual void Paint();
	virtual void OnThink();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	void SetFont(const char *fontName);
	void SetText(const char *text);
	void SetTextPosition(float x, float y);
	void SetColor(int r, int g, int b, int a);
	void SetTiming(float holdTime, float fadeIn, float fadeOut);
	void SetEntity(int entityIndex, float offsetX, float offsetY, float offsetZ);
	void SetAlignment(int align);
	void Show();
	void Hide(float fadeTime = 0.0f, float delay = 0.0f);
	void Animate(float targetX, float targetY, float scale, float duration);

	int GetTextID() const { return m_iTextID; }
	bool IsExpired() const;

private:
	int m_iTextID;
	char m_szFontName[64];
	char m_szText[512];
	wchar_t m_wszText[512];
	vgui::HFont m_hFont;

	// Position (screen-relative 0-1)
	float m_flX, m_flY;

	// Color
	Color m_Color;

	// Timing
	float m_flHoldTime;
	float m_flFadeIn;
	float m_flFadeOut;
	float m_flStartTime;
	bool m_bVisible;

	// Entity attachment
	int m_iEntityIndex;
	Vector m_vecEntityOffset;

	// Alignment (0=left, 1=center, 2=right)
	int m_iAlignment;

	// Animation
	bool m_bAnimating;
	float m_flAnimStartTime;
	float m_flAnimDuration;
	float m_flStartX, m_flStartY;
	float m_flTargetX, m_flTargetY;
	float m_flAnimScale;

	// Fade out
	bool m_bFadingOut;
	float m_flFadeOutStart;
	float m_flFadeOutDuration;

	// Helper
};

//-----------------------------------------------------------------------------
CGModTextDisplay::CGModTextDisplay(vgui::Panel *parent, const char *name, int textID)
	: BaseClass(parent, name)
{
	m_iTextID = textID;
	Q_strcpy(m_szFontName, "Default");
	m_szText[0] = '\0';
	m_wszText[0] = L'\0';
	m_hFont = vgui::INVALID_FONT;
	m_flX = m_flY = 0.5f;
	m_Color = Color(255, 255, 255, 255);
	m_flHoldTime = 5.0f;
	m_flFadeIn = 0.1f;
	m_flFadeOut = 0.1f;
	m_flStartTime = 0;
	m_bVisible = false;
	m_iEntityIndex = 0;
	m_vecEntityOffset.Init();
	m_iAlignment = 0;
	m_bAnimating = false;
	m_bFadingOut = false;

	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetVisible(false);

	// Fill the full-screen parent. Paint() positions text in absolute screen coords,
	// so a default 0x0 panel would give VGUI a zero-area clip rect and PaintTraverse
	// would skip the panel entirely (nothing drawn). This is why HUD text was invisible.
	int screenWide, screenTall;
	vgui::surface()->GetScreenSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);
}

//-----------------------------------------------------------------------------
CGModTextDisplay::~CGModTextDisplay()
{
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont(ResolveGModHudFontName(m_szFontName), true);
	if (m_hFont == vgui::INVALID_FONT)
		m_hFont = pScheme->GetFont("Default", true);

	// Re-fill the screen in case the resolution changed.
	int screenWide, screenTall;
	vgui::surface()->GetScreenSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::SetFont(const char *fontName)
{
	Q_strncpy(m_szFontName, ResolveGModHudFontName(fontName), sizeof(m_szFontName));
	InvalidateLayout(false, true);
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::SetText(const char *text)
{
	Q_strncpy(m_szText, text ? text : "", sizeof(m_szText));
	vgui::localize()->ConvertANSIToUnicode(m_szText, m_wszText, sizeof(m_wszText));
	m_wszText[(sizeof(m_wszText) / sizeof(m_wszText[0])) - 1] = L'\0';
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::SetTextPosition(float x, float y)
{
	m_flX = x;
	m_flY = y;
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::SetColor(int r, int g, int b, int a)
{
	m_Color.SetColor(r, g, b, a);
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::SetTiming(float holdTime, float fadeIn, float fadeOut)
{
	m_flHoldTime = holdTime;
	m_flFadeIn = fadeIn;
	m_flFadeOut = fadeOut;
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::SetEntity(int entityIndex, float offsetX, float offsetY, float offsetZ)
{
	m_iEntityIndex = entityIndex;
	m_vecEntityOffset.Init(offsetX, offsetY, offsetZ);
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::SetAlignment(int align)
{
	m_iAlignment = align;
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::Show()
{
	m_bVisible = true;
	m_flStartTime = gpGlobals->curtime;
	m_bFadingOut = false;
	SetVisible(true);
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::Hide(float fadeTime, float delay)
{
	if (fadeTime > 0 || delay > 0)
	{
		m_bFadingOut = true;
		m_flFadeOutStart = gpGlobals->curtime + delay;
		m_flFadeOutDuration = fadeTime;
	}
	else
	{
		m_bVisible = false;
		SetVisible(false);
	}
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::Animate(float targetX, float targetY, float scale, float duration)
{
	m_bAnimating = true;
	m_flAnimStartTime = gpGlobals->curtime;
	m_flAnimDuration = duration;
	m_flStartX = m_flX;
	m_flStartY = m_flY;
	m_flTargetX = targetX;
	m_flTargetY = targetY;
	m_flAnimScale = scale;
}

//-----------------------------------------------------------------------------
bool CGModTextDisplay::IsExpired() const
{
	if (!m_bVisible)
		return true;

	if (m_flHoldTime <= 0)
		return false; // Infinite duration

	float elapsed = gpGlobals->curtime - m_flStartTime;
	return elapsed >= (m_flHoldTime + m_flFadeIn + m_flFadeOut);
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::OnThink()
{
	BaseClass::OnThink();

	float currentTime = gpGlobals->curtime;

	// Handle fade out
	if (m_bFadingOut && currentTime >= m_flFadeOutStart + m_flFadeOutDuration)
	{
		m_bVisible = false;
		SetVisible(false);
		return;
	}

	// Handle expiration
	if (IsExpired())
	{
		m_bVisible = false;
		SetVisible(false);
		return;
	}

	// Handle animation
	if (m_bAnimating)
	{
		float elapsed = currentTime - m_flAnimStartTime;
		float progress = (m_flAnimDuration > 0) ? (elapsed / m_flAnimDuration) : 1.0f;

		if (progress >= 1.0f)
		{
			progress = 1.0f;
			m_bAnimating = false;
		}

		// Ease out
		float eased = 1.0f - (1.0f - progress) * (1.0f - progress);

		m_flX = m_flStartX + (m_flTargetX - m_flStartX) * eased;
		m_flY = m_flStartY + (m_flTargetY - m_flStartY) * eased;
	}

	if (m_bVisible && !IsVisible())
		SetVisible(true);
}

//-----------------------------------------------------------------------------
void CGModTextDisplay::Paint()
{
	if (!m_bVisible || !m_wszText[0])
		return;

	if (m_hFont == vgui::INVALID_FONT)
		return;

	int screenWide, screenTall;
	vgui::surface()->GetScreenSize(screenWide, screenTall);

	float drawX = m_flX;
	float drawY = m_flY;

	// Entity attachment
	if (m_iEntityIndex > 0)
	{
		C_BaseEntity *pEntity = ClientEntityList().GetBaseEntity(m_iEntityIndex);
		if (!pEntity || pEntity->IsDormant())
			return;

		Vector worldPos = pEntity->GetAbsOrigin() + m_vecEntityOffset;
		int sx, sy;
		if (!GetVectorInScreenSpace(worldPos, sx, sy))
			return;

		drawX = (float)sx / (float)screenWide;
		drawY = (float)sy / (float)screenTall;
	}

	// Measure the text up front so we can honor GMod's centering sentinel and alignment.
	int textWide, textTall;
	vgui::surface()->GetTextSize(m_hFont, m_wszText, textWide, textTall);

	// Convert normalized position to screen coordinates.
	// GMod convention: an axis value of -1 means "center the text on that axis".
	// bmod previously passed -1 straight through, so x = -1 * screenWide put the
	// countdown ("3..2..1..GO!!") and the winner banner entirely off the left edge.
	int x, y;
	if (drawX == -1.0f)
		x = (screenWide - textWide) / 2;
	else
		x = (int)(drawX * screenWide);

	if (drawY == -1.0f)
		y = (screenTall - textTall) / 2;
	else
		y = (int)(drawY * screenTall);

	// Calculate alpha based on fade
	float alpha = 1.0f;
	float currentTime = gpGlobals->curtime;
	float elapsed = currentTime - m_flStartTime;

	// Fade in
	if (m_flFadeIn > 0 && elapsed < m_flFadeIn)
	{
		alpha = elapsed / m_flFadeIn;
	}
	// Fade out (from timing)
	else if (m_flHoldTime > 0 && m_flFadeOut > 0)
	{
		float fadeOutStart = m_flFadeIn + m_flHoldTime;
		if (elapsed > fadeOutStart)
		{
			float fadeOutProgress = (elapsed - fadeOutStart) / m_flFadeOut;
			alpha = 1.0f - min(fadeOutProgress, 1.0f);
		}
	}

	// Manual fade out
	if (m_bFadingOut && currentTime >= m_flFadeOutStart)
	{
		float fadeProgress = (currentTime - m_flFadeOutStart) / max(m_flFadeOutDuration, 0.001f);
		alpha = 1.0f - min(fadeProgress, 1.0f);
	}

	// Get color components (2003 Color class uses GetColor method)
	int cr, cg, cb, ca;
	m_Color.GetColor(cr, cg, cb, ca);
	int finalAlpha = (int)(ca * alpha);

	// Explicit alignment. Skipped on an axis that used the -1 centering sentinel
	// above (that path already positioned the whole text block).
	if (drawX != -1.0f)
	{
		if (m_iAlignment == 1) // Center
			x -= textWide / 2;
		else if (m_iAlignment == 2) // Right
			x -= textWide;
	}

	// Draw text
	vgui::surface()->DrawSetTextFont(m_hFont);
	vgui::surface()->DrawSetTextColor(cr, cg, cb, finalAlpha);
	vgui::surface()->DrawSetTextPos(x, y);

	vgui::surface()->DrawPrintText(m_wszText, wcslen(m_wszText));
}

//=============================================================================
// GMod HUD Display Manager
//=============================================================================
class CGModHudDisplay : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CGModHudDisplay, vgui::Panel);

public:
	CGModHudDisplay(const char *pName);
	virtual ~CGModHudDisplay();

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void LevelInit(void);
	virtual void LevelShutdown(void);

	// User message handlers
	void MsgFunc_GModHint(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_GModToolText(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_GModText(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_GModTextAnimate(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_GModTextHide(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_GModTextHideAll(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_GModPlayerOption(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_GModRect(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_GModRectAnimate(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_GModRectHideAll(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_WQuad(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_WQuadAnimate(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_WQuadHide(const char *pszName, int iSize, void *pbuf);
	void MsgFunc_WQuadHideAll(const char *pszName, int iSize, void *pbuf);

	// Element management
	CGModTextDisplay* FindOrCreateText(int textID);
	CGModMaterialRect* FindOrCreateRect(int rectID);
	void RemoveText(int textID);
	void RemoveRect(int rectID);
	void RemoveAllText();
	void RemoveAllRects();

private:
	CUtlVector<CGModTextDisplay*> m_TextElements;
	CUtlVector<CGModMaterialRect*> m_RectElements;
};

DECLARE_HUDELEMENT(CGModHudDisplay);
DECLARE_HUD_MESSAGE(CGModHudDisplay, GModHint);
DECLARE_HUD_MESSAGE(CGModHudDisplay, GModToolText);
DECLARE_HUD_MESSAGE(CGModHudDisplay, GModPlayerOption);
DECLARE_HUD_MESSAGE(CGModHudDisplay, GModText);
DECLARE_HUD_MESSAGE(CGModHudDisplay, GModTextAnimate);
DECLARE_HUD_MESSAGE(CGModHudDisplay, GModTextHide);
DECLARE_HUD_MESSAGE(CGModHudDisplay, GModTextHideAll);
DECLARE_HUD_MESSAGE(CGModHudDisplay, GModRect);
DECLARE_HUD_MESSAGE(CGModHudDisplay, GModRectAnimate);
DECLARE_HUD_MESSAGE(CGModHudDisplay, GModRectHideAll);
DECLARE_HUD_MESSAGE(CGModHudDisplay, WQuad);
DECLARE_HUD_MESSAGE(CGModHudDisplay, WQuadAnimate);
DECLARE_HUD_MESSAGE(CGModHudDisplay, WQuadHide);
DECLARE_HUD_MESSAGE(CGModHudDisplay, WQuadHideAll);

//-----------------------------------------------------------------------------
CGModHudDisplay::CGModHudDisplay(const char *pName)
	: CHudElement(pName), BaseClass(NULL, "GModHudDisplay")
{
	SetParent(g_pClientMode->GetViewport());
	SetVisible(true);
	SetPaintBackgroundEnabled(false);

	int screenWide, screenTall;
	vgui::surface()->GetScreenSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);
}

//-----------------------------------------------------------------------------
CGModHudDisplay::~CGModHudDisplay()
{
	RemoveAllText();
	RemoveAllRects();
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::Init(void)
{
	HOOK_MESSAGE(GModHint);
	HOOK_MESSAGE(GModToolText);
	HOOK_MESSAGE(GModPlayerOption);
	HOOK_MESSAGE(GModText);
	HOOK_MESSAGE(GModTextAnimate);
	HOOK_MESSAGE(GModTextHide);
	HOOK_MESSAGE(GModTextHideAll);
	HOOK_MESSAGE(GModRect);
	HOOK_MESSAGE(GModRectAnimate);
	HOOK_MESSAGE(GModRectHideAll);
	HOOK_MESSAGE(WQuad);
	HOOK_MESSAGE(WQuadAnimate);
	HOOK_MESSAGE(WQuadHide);
	HOOK_MESSAGE(WQuadHideAll);
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::VidInit(void)
{
	RemoveAllText();
	RemoveAllRects();
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::LevelInit(void)
{
	RemoveAllText();
	RemoveAllRects();
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::LevelShutdown(void)
{
	RemoveAllText();
	RemoveAllRects();
}

//-----------------------------------------------------------------------------
CGModTextDisplay* CGModHudDisplay::FindOrCreateText(int textID)
{
	// Find existing
	for (int i = 0; i < m_TextElements.Count(); i++)
	{
		if (m_TextElements[i]->GetTextID() == textID)
			return m_TextElements[i];
	}

	// Create new
	char name[64];
	Q_snprintf(name, sizeof(name), "GModText_%d", textID);
	CGModTextDisplay *pText = new CGModTextDisplay(this, name, textID);
	m_TextElements.AddToTail(pText);
	return pText;
}

//-----------------------------------------------------------------------------
CGModMaterialRect* CGModHudDisplay::FindOrCreateRect(int rectID)
{
	// Find existing
	for (int i = 0; i < m_RectElements.Count(); i++)
	{
		if (m_RectElements[i]->GetRectID() == rectID)
			return m_RectElements[i];
	}

	// Create new
	char name[64];
	Q_snprintf(name, sizeof(name), "GModRect_%d", rectID);
	CGModMaterialRect *pRect = new CGModMaterialRect(this, name, rectID);
	m_RectElements.AddToTail(pRect);
	return pRect;
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::RemoveText(int textID)
{
	for (int i = m_TextElements.Count() - 1; i >= 0; i--)
	{
		if (m_TextElements[i]->GetTextID() == textID)
		{
			m_TextElements[i]->MarkForDeletion();
			m_TextElements.Remove(i);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::RemoveRect(int rectID)
{
	for (int i = m_RectElements.Count() - 1; i >= 0; i--)
	{
		if (m_RectElements[i]->GetRectID() == rectID)
		{
			m_RectElements[i]->MarkForDeletion();
			m_RectElements.Remove(i);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::RemoveAllText()
{
	for (int i = 0; i < m_TextElements.Count(); i++)
	{
		m_TextElements[i]->MarkForDeletion();
	}
	m_TextElements.Purge();
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::RemoveAllRects()
{
	for (int i = 0; i < m_RectElements.Count(); i++)
	{
		m_RectElements[i]->MarkForDeletion();
	}
	m_RectElements.Purge();
}

//=============================================================================
// User Message Handlers
//=============================================================================

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_GModHint(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	char text[256];
	Q_strncpy(text, READ_STRING(), sizeof(text));

	// Display hint using text system
	CGModTextDisplay *pText = FindOrCreateText(999); // Special hint ID
	pText->SetFont("Default");
	pText->SetText(text);
	pText->SetTextPosition(0.02f, 0.9f);
	pText->SetColor(255, 255, 255, 255);
	pText->SetTiming(5.0f, 0.1f, 0.1f);
	pText->Show();
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_GModToolText(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	char text[256];
	Q_strncpy(text, READ_STRING(), sizeof(text));

	CGModTextDisplay *pText = FindOrCreateText(998); // Special tool text ID
	pText->SetFont("Default");
	pText->SetText(text);
	pText->SetTextPosition(0.02f, 0.02f);
	pText->SetColor(255, 255, 255, 255);
	pText->SetTiming(0, 0, 0); // Stays until replaced
	pText->Show();
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_GModText(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int textID = READ_SHORT();
	char fontName[64];
	Q_strncpy(fontName, READ_STRING(), sizeof(fontName));
	char text[256];
	Q_strncpy(text, READ_STRING(), sizeof(text));
	float x = READ_FLOAT();
	float y = READ_FLOAT();
	int r = READ_BYTE();
	int g = READ_BYTE();
	int b = READ_BYTE();
	int a = READ_BYTE();
	float fadeIn = READ_FLOAT();
	float fadeOut = READ_FLOAT();
	float holdTime = READ_FLOAT();
	int effect = READ_BYTE();
	int entityID = READ_SHORT();
	float offsetX = READ_FLOAT();
	float offsetY = READ_FLOAT();
	float offsetZ = READ_FLOAT();

	CGModTextDisplay *pText = FindOrCreateText(textID);
	pText->SetFont(fontName);
	pText->SetText(text);
	pText->SetTextPosition(x, y);
	pText->SetColor(r, g, b, a);
	pText->SetTiming(holdTime, fadeIn, fadeOut);
	pText->SetEntity(entityID, offsetX, offsetY, offsetZ);
	pText->Show();
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_GModTextAnimate(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int textID = READ_SHORT();
	float x = READ_FLOAT();
	float y = READ_FLOAT();
	int r = READ_BYTE();
	int g = READ_BYTE();
	int b = READ_BYTE();
	int a = READ_BYTE();
	float scale = READ_FLOAT();
	float duration = READ_FLOAT();

	CGModTextDisplay *pText = FindOrCreateText(textID);
	pText->SetColor(r, g, b, a);
	pText->Animate(x, y, scale, duration);
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_GModTextHide(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int textID = READ_SHORT();
	float fadeTime = READ_FLOAT();
	float delay = READ_FLOAT();

	for (int i = 0; i < m_TextElements.Count(); i++)
	{
		if (m_TextElements[i]->GetTextID() == textID)
		{
			m_TextElements[i]->Hide(fadeTime, delay);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_GModTextHideAll(const char *pszName, int iSize, void *pbuf)
{
	for (int i = 0; i < m_TextElements.Count(); i++)
	{
		m_TextElements[i]->Hide();
	}
}

//-----------------------------------------------------------------------------
// _PlayerOption: server asks the client to open a named menu/option (with a timeout).
// Dispatches the well-known built-in options to their menus; other (gamemode-specific)
// option names are consumed so the message does not desync.
//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_GModPlayerOption(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	char optionName[128];
	Q_strncpy(optionName, READ_STRING(), sizeof(optionName));
	float value = READ_FLOAT();  // timeout (curtime-relative); reserved for future auto-close
	(void)value;

	if (!Q_stricmp(optionName, "ChooseTeam") ||
		!Q_stricmp(optionName, "TeamSelect") ||
		!Q_stricmp(optionName, "onChooseTeam"))
	{
		CGModTeamMenu *pMenu = CGModTeamMenu::GetInstance();
		if (pMenu)
			pMenu->ShowMenu();
	}
	// Other options (BuyItem, BuyAmmo, ChooseBirdType, domenu, ChooseHelp, ...) are
	// gamemode-specific. They are accepted here (no error) and can be routed to
	// gamemode-specific client panels as those are implemented.
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_GModRect(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int rectID = READ_SHORT();
	char material[256];
	Q_strncpy(material, READ_STRING(), sizeof(material));
	float x = READ_FLOAT();
	float y = READ_FLOAT();
	float w = READ_FLOAT();
	float h = READ_FLOAT();
	int r = READ_BYTE();
	int g = READ_BYTE();
	int b = READ_BYTE();
	int a = READ_BYTE();
	float holdTime = READ_FLOAT();
	float fadeIn = READ_FLOAT();
	float fadeOut = READ_FLOAT();
	float delay = READ_FLOAT();

	CGModMaterialRect *pRect = FindOrCreateRect(rectID);
	pRect->SetMaterial(material);
	pRect->SetRectPosition(x, y, w, h);
	pRect->SetColor(r, g, b, a);
	pRect->SetTiming(holdTime, fadeIn, fadeOut, delay);
	pRect->Show();
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_GModRectAnimate(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int rectID = READ_SHORT();
	float x = READ_FLOAT();
	float y = READ_FLOAT();
	float w = READ_FLOAT();
	float h = READ_FLOAT();
	int r = READ_BYTE();
	int g = READ_BYTE();
	int b = READ_BYTE();
	int a = READ_BYTE();
	float targetX = READ_FLOAT();
	float targetY = READ_FLOAT();
	float delay = READ_FLOAT();

	CGModMaterialRect *pRect = FindOrCreateRect(rectID);
	pRect->SetRectPosition(x, y, w, h);
	pRect->SetColor(r, g, b, a);
	pRect->SetTargetPosition(targetX, targetY, w, h);
	pRect->StartAnimation(delay, 0.5f);
	pRect->Show();
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_GModRectHideAll(const char *pszName, int iSize, void *pbuf)
{
	for (int i = 0; i < m_RectElements.Count(); i++)
	{
		m_RectElements[i]->Hide();
	}
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_WQuad(const char *pszName, int iSize, void *pbuf)
{
	// World quads - stub for now
	// These are 3D quads in world space, more complex to implement
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_WQuadAnimate(const char *pszName, int iSize, void *pbuf)
{
	// World quad animation - stub
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_WQuadHide(const char *pszName, int iSize, void *pbuf)
{
	// World quad hide - stub
}

//-----------------------------------------------------------------------------
void CGModHudDisplay::MsgFunc_WQuadHideAll(const char *pszName, int iSize, void *pbuf)
{
	// World quad hide all - stub
}
