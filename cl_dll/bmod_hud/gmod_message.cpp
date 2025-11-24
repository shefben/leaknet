//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GMod Message System - Text and UI messaging system for tools
// Based on reverse engineering of Garry's Mod client.dll
//
//=============================================================================//

#include "cbase.h"
#include "gmod_message.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/AnimationController.h>
#include "convar.h"
#include "c_baseentity.h"
#include "iclientmode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Console Variables
//-----------------------------------------------------------------------------
ConVar gmod_message_debug("gmod_message_debug", "0", FCVAR_CHEAT, "Debug GMod message system");

//-----------------------------------------------------------------------------
// Console Commands - Text System
//-----------------------------------------------------------------------------
ConCommand _GModText_Start("_GModText_Start", GModTextStart_f, "Initialize GMod text display");
ConCommand _GModText_SetPos("_GModText_SetPos", GModTextSetPos_f, "Set text position");
ConCommand _GModText_SetColor("_GModText_SetColor", GModTextSetColor_f, "Set text color");
ConCommand _GModText_SetTime("_GModText_SetTime", GModTextSetTime_f, "Set text display time");
ConCommand _GModText_SetDelay("_GModText_SetDelay", GModTextSetDelay_f, "Set text delay");
ConCommand _GModText_SetText("_GModText_SetText", GModTextSetText_f, "Set text content");
ConCommand _GModText_SetEntity("_GModText_SetEntity", GModTextSetEntity_f, "Attach text to entity");
ConCommand _GModText_Animate("_GModText_Animate", GModTextAnimate_f, "Animate text");
ConCommand _GModText_Hide("_GModText_Hide", GModTextHide_f, "Hide specific text");
ConCommand _GModText_HideAll("_GModText_HideAll", GModTextHideAll_f, "Hide all text");

//-----------------------------------------------------------------------------
// Console Commands - Rectangle System
//-----------------------------------------------------------------------------
ConCommand _GModRect_Start("_GModRect_Start", GModRectStart_f, "Initialize GMod rectangle");
ConCommand _GModRect_SetPos("_GModRect_SetPos", GModRectSetPos_f, "Set rectangle position");
ConCommand _GModRect_SetSize("_GModRect_SetSize", GModRectSetSize_f, "Set rectangle size");
ConCommand _GModRect_SetColor("_GModRect_SetColor", GModRectSetColor_f, "Set rectangle color");
ConCommand _GModRect_SetTime("_GModRect_SetTime", GModRectSetTime_f, "Set rectangle display time");
ConCommand _GModRect_SetDelay("_GModRect_SetDelay", GModRectSetDelay_f, "Set rectangle delay");
ConCommand _GModRect_Animate("_GModRect_Animate", GModRectAnimate_f, "Animate rectangle");
ConCommand _GModRect_Hide("_GModRect_Hide", GModRectHide_f, "Hide specific rectangle");
ConCommand _GModRect_HideAll("_GModRect_HideAll", GModRectHideAll_f, "Hide all rectangles");

//-----------------------------------------------------------------------------
// Global message manager
//-----------------------------------------------------------------------------
CGModMessageManager *g_pGModMessageManager = NULL;

// Current message/rect being configured
static CGModMessage *s_pCurrentMessage = NULL;
static CGModRect *s_pCurrentRect = NULL;

//=============================================================================
// GMOD MESSAGE CLASS
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGModMessage::CGModMessage(Panel *parent, const char *name)
	: BaseClass(parent, name)
{
	m_szText[0] = '\0';
	Q_strcpy(m_szFont, "Default");
	m_Color = Color(255, 255, 255, 255);
	m_Corner = GMOD_CORNER_TOPLEFT;
	m_iEntityIndex = -1;
	m_flDisplayTime = 5.0f;
	m_flStartTime = 0.0f;
	m_flDelay = 0.0f;
	m_bVisible = false;

	m_bAnimating = false;
	m_AnimationType = GMOD_ANIM_NONE;
	m_flAnimationDuration = 1.0f;
	m_flAnimationStart = 0.0f;
	m_vecAnimationStart.Init();
	m_vecAnimationEnd.Init();
	m_AnimationStartColor = Color(255, 255, 255, 0);
	m_AnimationEndColor = Color(255, 255, 255, 255);

	m_pLabel = new Label(this, "MessageLabel", "");
	m_hFont = INVALID_FONT;

	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CGModMessage::~CGModMessage()
{
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CGModMessage::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont(m_szFont, true);
	if (m_hFont == INVALID_FONT)
		m_hFont = pScheme->GetFont("Default", true);

	m_pLabel->SetFont(m_hFont);
	m_pLabel->SetFgColor(m_Color);
}

//-----------------------------------------------------------------------------
// Purpose: Layout the message
//-----------------------------------------------------------------------------
void CGModMessage::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize(wide, tall);
	m_pLabel->SetBounds(0, 0, wide, tall);

	UpdatePosition();
}

//-----------------------------------------------------------------------------
// Purpose: Think function
//-----------------------------------------------------------------------------
void CGModMessage::OnThink()
{
	BaseClass::OnThink();

	float currentTime = gpGlobals->curtime;

	// Handle delay
	if (m_flDelay > 0.0f && currentTime < m_flStartTime + m_flDelay)
	{
		SetVisible(false);
		return;
	}

	// Handle expiration
	if (IsExpired())
	{
		ShowMessage(false);
		return;
	}

	// Update animation
	if (m_bAnimating)
	{
		UpdateAnimation();
	}

	// Update entity attachment
	if (m_iEntityIndex != -1)
	{
		UpdateFromEntity();
	}

	// Show if not visible but should be
	if (!IsVisible() && m_bVisible)
	{
		SetVisible(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Paint the message
//-----------------------------------------------------------------------------
void CGModMessage::Paint()
{
	if (!m_bVisible)
		return;

	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: Set message text
//-----------------------------------------------------------------------------
void CGModMessage::SetText(const char *text)
{
	if (text)
	{
		Q_strncpy(m_szText, text, sizeof(m_szText));
		m_pLabel->SetText(text);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set message position
//-----------------------------------------------------------------------------
void CGModMessage::SetPosition(int x, int y)
{
	SetPos(x, y);
}

//-----------------------------------------------------------------------------
// Purpose: Set message size
//-----------------------------------------------------------------------------
void CGModMessage::SetSize(int wide, int tall)
{
	BaseClass::SetSize(wide, tall);
}

//-----------------------------------------------------------------------------
// Purpose: Set message color
//-----------------------------------------------------------------------------
void CGModMessage::SetColor(const Color &color)
{
	m_Color = color;
	if (m_pLabel)
		m_pLabel->SetFgColor(color);
}

//-----------------------------------------------------------------------------
// Purpose: Set message font
//-----------------------------------------------------------------------------
void CGModMessage::SetFont(const char *fontName)
{
	if (fontName)
	{
		Q_strncpy(m_szFont, fontName, sizeof(m_szFont));
		InvalidateLayout();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set positioning corner
//-----------------------------------------------------------------------------
void CGModMessage::SetCorner(GModCorner_t corner)
{
	m_Corner = corner;
	UpdatePosition();
}

//-----------------------------------------------------------------------------
// Purpose: Attach to entity
//-----------------------------------------------------------------------------
void CGModMessage::SetEntity(int entityIndex)
{
	m_iEntityIndex = entityIndex;
}

//-----------------------------------------------------------------------------
// Purpose: Set display time
//-----------------------------------------------------------------------------
void CGModMessage::SetTime(float displayTime)
{
	m_flDisplayTime = displayTime;
}

//-----------------------------------------------------------------------------
// Purpose: Set delay before showing
//-----------------------------------------------------------------------------
void CGModMessage::SetDelay(float delay)
{
	m_flDelay = delay;
}

//-----------------------------------------------------------------------------
// Purpose: Start animation
//-----------------------------------------------------------------------------
void CGModMessage::StartAnimation(GModAnimationType_t type, float duration)
{
	m_AnimationType = type;
	m_flAnimationDuration = duration;
	m_flAnimationStart = gpGlobals->curtime;
	m_bAnimating = true;

	int x, y;
	GetPos(x, y);
	m_vecAnimationStart.x = x;
	m_vecAnimationStart.y = y;

	switch (type)
	{
	case GMOD_ANIM_FADEIN:
		m_AnimationStartColor = Color(m_Color[0], m_Color[1], m_Color[2], 0);
		m_AnimationEndColor = m_Color;
		break;
	case GMOD_ANIM_FADEOUT:
		m_AnimationStartColor = m_Color;
		m_AnimationEndColor = Color(m_Color[0], m_Color[1], m_Color[2], 0);
		break;
	case GMOD_ANIM_SLIDE:
		m_vecAnimationEnd.x = x + 100;
		m_vecAnimationEnd.y = y;
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Stop animation
//-----------------------------------------------------------------------------
void CGModMessage::StopAnimation()
{
	m_bAnimating = false;
	m_AnimationType = GMOD_ANIM_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Show/hide message
//-----------------------------------------------------------------------------
void CGModMessage::ShowMessage(bool show)
{
	m_bVisible = show;
	if (show)
	{
		m_flStartTime = gpGlobals->curtime;
	}
	SetVisible(show);
}

//-----------------------------------------------------------------------------
// Purpose: Hide all messages
//-----------------------------------------------------------------------------
void CGModMessage::HideAll()
{
	ShowMessage(false);
}

//-----------------------------------------------------------------------------
// Purpose: Get remaining display time
//-----------------------------------------------------------------------------
float CGModMessage::GetRemainingTime() const
{
	if (!m_bVisible)
		return 0.0f;

	float elapsed = gpGlobals->curtime - (m_flStartTime + m_flDelay);
	return m_flDisplayTime - elapsed;
}

//-----------------------------------------------------------------------------
// Purpose: Check if message has expired
//-----------------------------------------------------------------------------
bool CGModMessage::IsExpired() const
{
	if (!m_bVisible || m_flDisplayTime <= 0.0f)
		return false;

	float elapsed = gpGlobals->curtime - (m_flStartTime + m_flDelay);
	return elapsed >= m_flDisplayTime;
}

//-----------------------------------------------------------------------------
// Purpose: Update animation
//-----------------------------------------------------------------------------
void CGModMessage::UpdateAnimation()
{
	if (!m_bAnimating)
		return;

	float elapsed = gpGlobals->curtime - m_flAnimationStart;
	float progress = elapsed / m_flAnimationDuration;

	if (progress >= 1.0f)
	{
		progress = 1.0f;
		OnAnimationComplete();
	}

	switch (m_AnimationType)
	{
	case GMOD_ANIM_FADEIN:
	case GMOD_ANIM_FADEOUT:
		{
			Color newColor;
			newColor.SetColor(
				Lerp(progress, m_AnimationStartColor[0], m_AnimationEndColor[0]),
				Lerp(progress, m_AnimationStartColor[1], m_AnimationEndColor[1]),
				Lerp(progress, m_AnimationStartColor[2], m_AnimationEndColor[2]),
				Lerp(progress, m_AnimationStartColor[3], m_AnimationEndColor[3])
			);
			SetColor(newColor);
		}
		break;

	case GMOD_ANIM_SLIDE:
		{
			int newX = Lerp(progress, m_vecAnimationStart.x, m_vecAnimationEnd.x);
			int newY = Lerp(progress, m_vecAnimationStart.y, m_vecAnimationEnd.y);
			SetPos(newX, newY);
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Animation complete
//-----------------------------------------------------------------------------
void CGModMessage::OnAnimationComplete()
{
	m_bAnimating = false;

	if (m_AnimationType == GMOD_ANIM_FADEOUT)
	{
		ShowMessage(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update position based on corner
//-----------------------------------------------------------------------------
void CGModMessage::UpdatePosition()
{
	if (m_Corner == GMOD_CORNER_TOPLEFT)
		return; // Already positioned

	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);

	int wide, tall;
	GetSize(wide, tall);

	int x, y;
	GetPos(x, y);

	switch (m_Corner)
	{
	case GMOD_CORNER_TOPRIGHT:
		SetPos(screenWide - wide - x, y);
		break;
	case GMOD_CORNER_BOTTOMLEFT:
		SetPos(x, screenTall - tall - y);
		break;
	case GMOD_CORNER_BOTTOMRIGHT:
		SetPos(screenWide - wide - x, screenTall - tall - y);
		break;
	case GMOD_CORNER_CENTER:
		SetPos((screenWide - wide) / 2 + x, (screenTall - tall) / 2 + y);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update position from attached entity
//-----------------------------------------------------------------------------
void CGModMessage::UpdateFromEntity()
{
	C_BaseEntity *pEntity = ClientEntityList().GetEnt(m_iEntityIndex);
	if (!pEntity)
		return;

	int screenX, screenY;
	if (GetVectorInScreenSpace(pEntity->GetAbsOrigin(), screenX, screenY))
	{
		SetPos(screenX, screenY);
	}
}

//=============================================================================
// GMOD RECTANGLE CLASS
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGModRect::CGModRect(Panel *parent, const char *name)
	: BaseClass(parent, name)
{
	m_Color = Color(255, 255, 255, 100);
	m_BorderColor = Color(255, 255, 255, 255);
	m_iBorderWidth = 1;
	m_Corner = GMOD_CORNER_TOPLEFT;
	m_flDisplayTime = 5.0f;
	m_flStartTime = 0.0f;
	m_flDelay = 0.0f;
	m_bVisible = false;

	m_bAnimating = false;
	m_AnimationType = GMOD_ANIM_NONE;
	m_flAnimationDuration = 1.0f;
	m_flAnimationStart = 0.0f;
	m_iAnimationStartWide = 0;
	m_iAnimationStartTall = 0;
	m_iAnimationEndWide = 0;
	m_iAnimationEndTall = 0;

	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CGModRect::~CGModRect()
{
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CGModRect::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetBgColor(m_Color);
}

//-----------------------------------------------------------------------------
// Purpose: Paint the rectangle
//-----------------------------------------------------------------------------
void CGModRect::Paint()
{
	if (!m_bVisible)
		return;

	int wide, tall;
	GetSize(wide, tall);

	// Draw filled rectangle
	surface()->DrawSetColor(m_Color);
	surface()->DrawFilledRect(0, 0, wide, tall);

	// Draw border
	if (m_iBorderWidth > 0)
	{
		surface()->DrawSetColor(m_BorderColor);
		for (int i = 0; i < m_iBorderWidth; i++)
		{
			surface()->DrawOutlinedRect(i, i, wide - i, tall - i);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Think function
//-----------------------------------------------------------------------------
void CGModRect::OnThink()
{
	BaseClass::OnThink();

	float currentTime = gpGlobals->curtime;

	// Handle delay
	if (m_flDelay > 0.0f && currentTime < m_flStartTime + m_flDelay)
	{
		SetVisible(false);
		return;
	}

	// Handle expiration
	if (IsExpired())
	{
		ShowRect(false);
		return;
	}

	// Update animation
	if (m_bAnimating)
	{
		UpdateAnimation();
	}

	// Show if not visible but should be
	if (!IsVisible() && m_bVisible)
	{
		SetVisible(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set rectangle position
//-----------------------------------------------------------------------------
void CGModRect::SetPosition(int x, int y)
{
	SetPos(x, y);
	UpdatePosition();
}

//-----------------------------------------------------------------------------
// Purpose: Set rectangle size
//-----------------------------------------------------------------------------
void CGModRect::SetSize(int wide, int tall)
{
	BaseClass::SetSize(wide, tall);
}

//-----------------------------------------------------------------------------
// Purpose: Set rectangle color
//-----------------------------------------------------------------------------
void CGModRect::SetColor(const Color &color)
{
	m_Color = color;
	SetBgColor(color);
}

//-----------------------------------------------------------------------------
// Purpose: Set border color
//-----------------------------------------------------------------------------
void CGModRect::SetBorderColor(const Color &color)
{
	m_BorderColor = color;
}

//-----------------------------------------------------------------------------
// Purpose: Set border width
//-----------------------------------------------------------------------------
void CGModRect::SetBorderWidth(int width)
{
	m_iBorderWidth = width;
}

//-----------------------------------------------------------------------------
// Purpose: Set positioning corner
//-----------------------------------------------------------------------------
void CGModRect::SetCorner(GModCorner_t corner)
{
	m_Corner = corner;
	UpdatePosition();
}

//-----------------------------------------------------------------------------
// Purpose: Set display time
//-----------------------------------------------------------------------------
void CGModRect::SetTime(float displayTime)
{
	m_flDisplayTime = displayTime;
}

//-----------------------------------------------------------------------------
// Purpose: Set delay before showing
//-----------------------------------------------------------------------------
void CGModRect::SetDelay(float delay)
{
	m_flDelay = delay;
}

//-----------------------------------------------------------------------------
// Purpose: Start animation
//-----------------------------------------------------------------------------
void CGModRect::StartAnimation(GModAnimationType_t type, float duration)
{
	m_AnimationType = type;
	m_flAnimationDuration = duration;
	m_flAnimationStart = gpGlobals->curtime;
	m_bAnimating = true;

	int wide, tall;
	GetSize(wide, tall);
	m_iAnimationStartWide = wide;
	m_iAnimationStartTall = tall;

	switch (type)
	{
	case GMOD_ANIM_SCALE:
		m_iAnimationEndWide = wide * 2;
		m_iAnimationEndTall = tall * 2;
		break;
	case GMOD_ANIM_FADEIN:
		m_AnimationStartColor = Color(m_Color[0], m_Color[1], m_Color[2], 0);
		m_AnimationEndColor = m_Color;
		break;
	case GMOD_ANIM_FADEOUT:
		m_AnimationStartColor = m_Color;
		m_AnimationEndColor = Color(m_Color[0], m_Color[1], m_Color[2], 0);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Stop animation
//-----------------------------------------------------------------------------
void CGModRect::StopAnimation()
{
	m_bAnimating = false;
	m_AnimationType = GMOD_ANIM_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Show/hide rectangle
//-----------------------------------------------------------------------------
void CGModRect::ShowRect(bool show)
{
	m_bVisible = show;
	if (show)
	{
		m_flStartTime = gpGlobals->curtime;
	}
	SetVisible(show);
}

//-----------------------------------------------------------------------------
// Purpose: Hide all rectangles
//-----------------------------------------------------------------------------
void CGModRect::HideAll()
{
	ShowRect(false);
}

//-----------------------------------------------------------------------------
// Purpose: Check if rectangle has expired
//-----------------------------------------------------------------------------
bool CGModRect::IsExpired() const
{
	if (!m_bVisible || m_flDisplayTime <= 0.0f)
		return false;

	float elapsed = gpGlobals->curtime - (m_flStartTime + m_flDelay);
	return elapsed >= m_flDisplayTime;
}

//-----------------------------------------------------------------------------
// Purpose: Update animation
//-----------------------------------------------------------------------------
void CGModRect::UpdateAnimation()
{
	if (!m_bAnimating)
		return;

	float elapsed = gpGlobals->curtime - m_flAnimationStart;
	float progress = elapsed / m_flAnimationDuration;

	if (progress >= 1.0f)
	{
		progress = 1.0f;
		m_bAnimating = false;
	}

	switch (m_AnimationType)
	{
	case GMOD_ANIM_SCALE:
		{
			int newWide = Lerp(progress, m_iAnimationStartWide, m_iAnimationEndWide);
			int newTall = Lerp(progress, m_iAnimationStartTall, m_iAnimationEndTall);
			SetSize(newWide, newTall);
		}
		break;

	case GMOD_ANIM_FADEIN:
	case GMOD_ANIM_FADEOUT:
		{
			Color newColor;
			newColor.SetColor(
				Lerp(progress, m_AnimationStartColor[0], m_AnimationEndColor[0]),
				Lerp(progress, m_AnimationStartColor[1], m_AnimationEndColor[1]),
				Lerp(progress, m_AnimationStartColor[2], m_AnimationEndColor[2]),
				Lerp(progress, m_AnimationStartColor[3], m_AnimationEndColor[3])
			);
			SetColor(newColor);
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update position based on corner
//-----------------------------------------------------------------------------
void CGModRect::UpdatePosition()
{
	if (m_Corner == GMOD_CORNER_TOPLEFT)
		return; // Already positioned

	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);

	int wide, tall;
	GetSize(wide, tall);

	int x, y;
	GetPos(x, y);

	switch (m_Corner)
	{
	case GMOD_CORNER_TOPRIGHT:
		SetPos(screenWide - wide - x, y);
		break;
	case GMOD_CORNER_BOTTOMLEFT:
		SetPos(x, screenTall - tall - y);
		break;
	case GMOD_CORNER_BOTTOMRIGHT:
		SetPos(screenWide - wide - x, screenTall - tall - y);
		break;
	case GMOD_CORNER_CENTER:
		SetPos((screenWide - wide) / 2 + x, (screenTall - tall) / 2 + y);
		break;
	}
}

//=============================================================================
// GMOD MESSAGE MANAGER
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGModMessageManager::CGModMessageManager()
{
	m_iNextMessageID = 1;
	m_iNextRectID = 1;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CGModMessageManager::~CGModMessageManager()
{
	RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: Create a new message
//-----------------------------------------------------------------------------
CGModMessage* CGModMessageManager::CreateMessage(const char *name)
{
	char autoName[64];
	if (!name)
	{
		Q_snprintf(autoName, sizeof(autoName), "message_%d", m_iNextMessageID++);
		name = autoName;
	}

	CGModMessage *message = new CGModMessage(g_pClientMode->GetViewport(), name);
	m_Messages.AddToTail(message);

	if (gmod_message_debug.GetBool())
		DevMsg("Created GMod message: %s\n", name);

	return message;
}

//-----------------------------------------------------------------------------
// Purpose: Create a new rectangle
//-----------------------------------------------------------------------------
CGModRect* CGModMessageManager::CreateRect(const char *name)
{
	char autoName[64];
	if (!name)
	{
		Q_snprintf(autoName, sizeof(autoName), "rect_%d", m_iNextRectID++);
		name = autoName;
	}

	CGModRect *rect = new CGModRect(g_pClientMode->GetViewport(), name);
	m_Rects.AddToTail(rect);

	if (gmod_message_debug.GetBool())
		DevMsg("Created GMod rectangle: %s\n", name);

	return rect;
}

//-----------------------------------------------------------------------------
// Purpose: Remove a message
//-----------------------------------------------------------------------------
void CGModMessageManager::RemoveMessage(CGModMessage *message)
{
	if (message)
	{
		m_Messages.FindAndRemove(message);
		message->MarkForDeletion();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove a rectangle
//-----------------------------------------------------------------------------
void CGModMessageManager::RemoveRect(CGModRect *rect)
{
	if (rect)
	{
		m_Rects.FindAndRemove(rect);
		rect->MarkForDeletion();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove all messages and rectangles
//-----------------------------------------------------------------------------
void CGModMessageManager::RemoveAll()
{
	for (int i = 0; i < m_Messages.Count(); i++)
	{
		m_Messages[i]->MarkForDeletion();
	}
	m_Messages.Purge();

	for (int i = 0; i < m_Rects.Count(); i++)
	{
		m_Rects[i]->MarkForDeletion();
	}
	m_Rects.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Hide all messages
//-----------------------------------------------------------------------------
void CGModMessageManager::HideAllMessages()
{
	for (int i = 0; i < m_Messages.Count(); i++)
	{
		m_Messages[i]->HideAll();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Hide all rectangles
//-----------------------------------------------------------------------------
void CGModMessageManager::HideAllRects()
{
	for (int i = 0; i < m_Rects.Count(); i++)
	{
		m_Rects[i]->HideAll();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Hide everything
//-----------------------------------------------------------------------------
void CGModMessageManager::HideAll()
{
	HideAllMessages();
	HideAllRects();
}

//-----------------------------------------------------------------------------
// Purpose: Animate message
//-----------------------------------------------------------------------------
void CGModMessageManager::AnimateMessage(const char *name, GModAnimationType_t type, float duration)
{
	CGModMessage *message = FindMessage(name);
	if (message)
	{
		message->StartAnimation(type, duration);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Animate rectangle
//-----------------------------------------------------------------------------
void CGModMessageManager::AnimateRect(const char *name, GModAnimationType_t type, float duration)
{
	CGModRect *rect = FindRect(name);
	if (rect)
	{
		rect->StartAnimation(type, duration);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Find message by name
//-----------------------------------------------------------------------------
CGModMessage* CGModMessageManager::FindMessage(const char *name)
{
	if (!name)
		return NULL;

	for (int i = 0; i < m_Messages.Count(); i++)
	{
		if (Q_stricmp(m_Messages[i]->GetName(), name) == 0)
		{
			return m_Messages[i];
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Find rectangle by name
//-----------------------------------------------------------------------------
CGModRect* CGModMessageManager::FindRect(const char *name)
{
	if (!name)
		return NULL;

	for (int i = 0; i < m_Rects.Count(); i++)
	{
		if (Q_stricmp(m_Rects[i]->GetName(), name) == 0)
		{
			return m_Rects[i];
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Update all messages and rectangles
//-----------------------------------------------------------------------------
void CGModMessageManager::Update()
{
	// Remove expired messages
	for (int i = m_Messages.Count() - 1; i >= 0; i--)
	{
		if (m_Messages[i]->IsExpired())
		{
			RemoveMessage(m_Messages[i]);
		}
	}

	// Remove expired rectangles
	for (int i = m_Rects.Count() - 1; i >= 0; i--)
	{
		if (m_Rects[i]->IsExpired())
		{
			RemoveRect(m_Rects[i]);
		}
	}
}

//=============================================================================
// CONSOLE COMMAND IMPLEMENTATIONS
//=============================================================================

//-----------------------------------------------------------------------------
// Text Commands
//-----------------------------------------------------------------------------
void GModTextStart_f(void)
{
	if (!g_pGModMessageManager)
		return;

	// 2003 engine doesn't have CCommand - use engine->Cmd_Argc/Argv
	const char *name = engine->Cmd_Argc() > 1 ? engine->Cmd_Argv(1) : NULL;
	s_pCurrentMessage = g_pGModMessageManager->CreateMessage(name);

	if (engine->Cmd_Argc() > 2)
	{
		s_pCurrentMessage->SetFont(engine->Cmd_Argv(2));
	}
}

void GModTextSetPos_f(void)
{
	if (!s_pCurrentMessage || engine->Cmd_Argc() < 3)
		return;

	int x = atoi(engine->Cmd_Argv(1));
	int y = atoi(engine->Cmd_Argv(2));
	s_pCurrentMessage->SetPosition(x, y);
}

void GModTextSetColor_f(void)
{
	if (!s_pCurrentMessage || engine->Cmd_Argc() < 4)
		return;

	int r = atoi(engine->Cmd_Argv(1));
	int g = atoi(engine->Cmd_Argv(2));
	int b = atoi(engine->Cmd_Argv(3));
	int a = engine->Cmd_Argc() > 4 ? atoi(engine->Cmd_Argv(4)) : 255;

	s_pCurrentMessage->SetColor(Color(r, g, b, a));
}

void GModTextSetTime_f(void)
{
	if (!s_pCurrentMessage || engine->Cmd_Argc() < 2)
		return;

	float time = atof(engine->Cmd_Argv(1));
	s_pCurrentMessage->SetTime(time);
}

void GModTextSetDelay_f(void)
{
	if (!s_pCurrentMessage || engine->Cmd_Argc() < 2)
		return;

	float delay = atof(engine->Cmd_Argv(1));
	s_pCurrentMessage->SetDelay(delay);
}

void GModTextSetText_f(void)
{
	if (!s_pCurrentMessage || engine->Cmd_Argc() < 2)
		return;

	s_pCurrentMessage->SetText(engine->Cmd_Argv(1));
	s_pCurrentMessage->ShowMessage(true);
}

void GModTextSetEntity_f(void)
{
	if (!s_pCurrentMessage || engine->Cmd_Argc() < 2)
		return;

	int entityIndex = atoi(engine->Cmd_Argv(1));
	s_pCurrentMessage->SetEntity(entityIndex);
}

void GModTextAnimate_f(void)
{
	if (engine->Cmd_Argc() < 3)
		return;

	const char *name = engine->Cmd_Argv(1);
	const char *animType = engine->Cmd_Argv(2);
	float duration = engine->Cmd_Argc() > 3 ? atof(engine->Cmd_Argv(3)) : 1.0f;

	GModAnimationType_t type = GMOD_ANIM_FADEIN;
	if (Q_stricmp(animType, "fadein") == 0)
		type = GMOD_ANIM_FADEIN;
	else if (Q_stricmp(animType, "fadeout") == 0)
		type = GMOD_ANIM_FADEOUT;
	else if (Q_stricmp(animType, "slide") == 0)
		type = GMOD_ANIM_SLIDE;

	if (g_pGModMessageManager)
		g_pGModMessageManager->AnimateMessage(name, type, duration);
}

void GModTextHide_f(void)
{
	if (engine->Cmd_Argc() < 2 || !g_pGModMessageManager)
		return;

	CGModMessage *message = g_pGModMessageManager->FindMessage(engine->Cmd_Argv(1));
	if (message)
		message->HideAll();
}

void GModTextHideAll_f(void)
{
	if (g_pGModMessageManager)
		g_pGModMessageManager->HideAllMessages();
}

//-----------------------------------------------------------------------------
// Rectangle Commands
//-----------------------------------------------------------------------------
void GModRectStart_f(void)
{
	if (!g_pGModMessageManager)
		return;

	const char *name = engine->Cmd_Argc() > 1 ? engine->Cmd_Argv(1) : NULL;
	s_pCurrentRect = g_pGModMessageManager->CreateRect(name);
}

void GModRectSetPos_f(void)
{
	if (!s_pCurrentRect || engine->Cmd_Argc() < 3)
		return;

	int x = atoi(engine->Cmd_Argv(1));
	int y = atoi(engine->Cmd_Argv(2));
	s_pCurrentRect->SetPosition(x, y);
}

void GModRectSetSize_f(void)
{
	if (!s_pCurrentRect || engine->Cmd_Argc() < 3)
		return;

	int wide = atoi(engine->Cmd_Argv(1));
	int tall = atoi(engine->Cmd_Argv(2));
	s_pCurrentRect->SetSize(wide, tall);
	s_pCurrentRect->ShowRect(true);
}

void GModRectSetColor_f(void)
{
	if (!s_pCurrentRect || engine->Cmd_Argc() < 4)
		return;

	int r = atoi(engine->Cmd_Argv(1));
	int g = atoi(engine->Cmd_Argv(2));
	int b = atoi(engine->Cmd_Argv(3));
	int a = engine->Cmd_Argc() > 4 ? atoi(engine->Cmd_Argv(4)) : 100;

	s_pCurrentRect->SetColor(Color(r, g, b, a));
}

void GModRectSetTime_f(void)
{
	if (!s_pCurrentRect || engine->Cmd_Argc() < 2)
		return;

	float time = atof(engine->Cmd_Argv(1));
	s_pCurrentRect->SetTime(time);
}

void GModRectSetDelay_f(void)
{
	if (!s_pCurrentRect || engine->Cmd_Argc() < 2)
		return;

	float delay = atof(engine->Cmd_Argv(1));
	s_pCurrentRect->SetDelay(delay);
}

void GModRectAnimate_f(void)
{
	if (engine->Cmd_Argc() < 3)
		return;

	const char *name = engine->Cmd_Argv(1);
	const char *animType = engine->Cmd_Argv(2);
	float duration = engine->Cmd_Argc() > 3 ? atof(engine->Cmd_Argv(3)) : 1.0f;

	GModAnimationType_t type = GMOD_ANIM_SCALE;
	if (Q_stricmp(animType, "scale") == 0)
		type = GMOD_ANIM_SCALE;
	else if (Q_stricmp(animType, "fadein") == 0)
		type = GMOD_ANIM_FADEIN;
	else if (Q_stricmp(animType, "fadeout") == 0)
		type = GMOD_ANIM_FADEOUT;

	if (g_pGModMessageManager)
		g_pGModMessageManager->AnimateRect(name, type, duration);
}

void GModRectHide_f(void)
{
	if (engine->Cmd_Argc() < 2 || !g_pGModMessageManager)
		return;

	CGModRect *rect = g_pGModMessageManager->FindRect(engine->Cmd_Argv(1));
	if (rect)
		rect->HideAll();
}

void GModRectHideAll_f(void)
{
	if (g_pGModMessageManager)
		g_pGModMessageManager->HideAllRects();
}

//=============================================================================
// SYSTEM INITIALIZATION
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Initialize GMod message system
//-----------------------------------------------------------------------------
void InitGModMessageSystem()
{
	if (!g_pGModMessageManager)
	{
		g_pGModMessageManager = new CGModMessageManager();
		DevMsg("GMod Message System initialized\n");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown GMod message system
//-----------------------------------------------------------------------------
void ShutdownGModMessageSystem()
{
	if (g_pGModMessageManager)
	{
		delete g_pGModMessageManager;
		g_pGModMessageManager = NULL;
		DevMsg("GMod Message System shutdown\n");
	}
}