//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GMod Message System - Text and UI messaging system for tools
// Based on reverse engineering of Garry's Mod client.dll
//
//=============================================================================//

#ifndef GMOD_MESSAGE_H
#define GMOD_MESSAGE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui/IScheme.h>
#include "Color.h"
#include "utlvector.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CGModRect;

//-----------------------------------------------------------------------------
// Message Types
//-----------------------------------------------------------------------------
enum GModMessageType_t
{
	GMOD_MSG_TEXT = 0,
	GMOD_MSG_RECT,
	GMOD_MSG_ANIMATION,
	GMOD_MSG_MAX
};

//-----------------------------------------------------------------------------
// Animation Types
//-----------------------------------------------------------------------------
enum GModAnimationType_t
{
	GMOD_ANIM_NONE = 0,
	GMOD_ANIM_FADEIN,
	GMOD_ANIM_FADEOUT,
	GMOD_ANIM_SLIDE,
	GMOD_ANIM_SCALE,
	GMOD_ANIM_MAX
};

//-----------------------------------------------------------------------------
// Message Corner Types (for positioning)
//-----------------------------------------------------------------------------
enum GModCorner_t
{
	GMOD_CORNER_TOPLEFT = 0,
	GMOD_CORNER_TOPRIGHT,
	GMOD_CORNER_BOTTOMLEFT,
	GMOD_CORNER_BOTTOMRIGHT,
	GMOD_CORNER_CENTER,
	GMOD_CORNER_MAX
};

//-----------------------------------------------------------------------------
// GMod Message Base Class
//-----------------------------------------------------------------------------
class CGModMessage : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CGModMessage, vgui::Panel);

public:
	CGModMessage(vgui::Panel *parent, const char *name);
	virtual ~CGModMessage();

	// Panel overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();
	virtual void OnThink();
	virtual void Paint();

	// Message interface
	virtual void SetText(const char *text);
	virtual void SetPosition(int x, int y);
	virtual void SetSize(int wide, int tall);
	virtual void SetColor(const Color &color);
	virtual void SetFont(const char *fontName);
	virtual void SetCorner(GModCorner_t corner);
	virtual void SetEntity(int entityIndex);
	virtual void SetTime(float displayTime);
	virtual void SetDelay(float delay);

	// Animation interface
	virtual void StartAnimation(GModAnimationType_t type, float duration);
	virtual void StopAnimation();
	virtual bool IsAnimating() const { return m_bAnimating; }

	// Visibility control
	virtual void ShowMessage(bool show);
	virtual void HideAll();

	// Properties
	virtual const char* GetText() const { return m_szText; }
	virtual Color GetMessageColor() const { return m_Color; }
	virtual float GetRemainingTime() const;
	virtual bool IsExpired() const;

protected:
	// Animation helpers
	virtual void UpdateAnimation();
	virtual void OnAnimationComplete();

	// Layout helpers
	virtual void UpdatePosition();
	virtual void UpdateFromEntity();

private:
	char m_szText[512];
	char m_szFont[64];
	Color m_Color;
	GModCorner_t m_Corner;
	int m_iEntityIndex;
	float m_flDisplayTime;
	float m_flStartTime;
	float m_flDelay;
	bool m_bVisible;

	// Animation data
	bool m_bAnimating;
	GModAnimationType_t m_AnimationType;
	float m_flAnimationDuration;
	float m_flAnimationStart;
	Vector2D m_vecAnimationStart;
	Vector2D m_vecAnimationEnd;
	Color m_AnimationStartColor;
	Color m_AnimationEndColor;

	// UI elements
	vgui::Label *m_pLabel;
	vgui::HFont m_hFont;
};

//-----------------------------------------------------------------------------
// GMod Rectangle Class
//-----------------------------------------------------------------------------
class CGModRect : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CGModRect, vgui::Panel);

public:
	CGModRect(vgui::Panel *parent, const char *name);
	virtual ~CGModRect();

	// Panel overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Paint();
	virtual void OnThink();

	// Rectangle interface
	virtual void SetPosition(int x, int y);
	virtual void SetSize(int wide, int tall);
	virtual void SetColor(const Color &color);
	virtual void SetBorderColor(const Color &color);
	virtual void SetBorderWidth(int width);
	virtual void SetCorner(GModCorner_t corner);
	virtual void SetTime(float displayTime);
	virtual void SetDelay(float delay);

	// Animation interface
	virtual void StartAnimation(GModAnimationType_t type, float duration);
	virtual void StopAnimation();

	// Visibility control
	virtual void ShowRect(bool show);
	virtual void HideAll();

	// Properties
	virtual Color GetRectColor() const { return m_Color; }
	virtual bool IsExpired() const;

protected:
	virtual void UpdateAnimation();
	virtual void UpdatePosition();

private:
	Color m_Color;
	Color m_BorderColor;
	int m_iBorderWidth;
	GModCorner_t m_Corner;
	float m_flDisplayTime;
	float m_flStartTime;
	float m_flDelay;
	bool m_bVisible;

	// Animation data
	bool m_bAnimating;
	GModAnimationType_t m_AnimationType;
	float m_flAnimationDuration;
	float m_flAnimationStart;
	int m_iAnimationStartWide, m_iAnimationStartTall;
	int m_iAnimationEndWide, m_iAnimationEndTall;
	Color m_AnimationStartColor;
	Color m_AnimationEndColor;
};

//-----------------------------------------------------------------------------
// GMod Message Manager
//-----------------------------------------------------------------------------
class CGModMessageManager
{
public:
	CGModMessageManager();
	~CGModMessageManager();

	// Message management
	CGModMessage* CreateMessage(const char *name = NULL);
	CGModRect* CreateRect(const char *name = NULL);
	void RemoveMessage(CGModMessage *message);
	void RemoveRect(CGModRect *rect);
	void RemoveAll();

	// Global commands
	void HideAllMessages();
	void HideAllRects();
	void HideAll();

	// Animation commands
	void AnimateMessage(const char *name, GModAnimationType_t type, float duration);
	void AnimateRect(const char *name, GModAnimationType_t type, float duration);

	// Find functions
	CGModMessage* FindMessage(const char *name);
	CGModRect* FindRect(const char *name);

	// Update function (called from client frame)
	void Update();

private:
	CUtlVector<CGModMessage*> m_Messages;
	CUtlVector<CGModRect*> m_Rects;
	int m_iNextMessageID;
	int m_iNextRectID;
};

// Global message manager
extern CGModMessageManager *g_pGModMessageManager;

//-----------------------------------------------------------------------------
// Console Command Functions (matching Garry's Mod pattern)
//-----------------------------------------------------------------------------

// Text functions
void GModTextStart_f(const CCommand &args);
void GModTextSetPos_f(const CCommand &args);
void GModTextSetColor_f(const CCommand &args);
void GModTextSetTime_f(const CCommand &args);
void GModTextSetDelay_f(const CCommand &args);
void GModTextSetText_f(const CCommand &args);
void GModTextSetEntity_f(const CCommand &args);
void GModTextAnimate_f(const CCommand &args);
void GModTextHide_f(const CCommand &args);
void GModTextHideAll_f(const CCommand &args);

// Rectangle functions
void GModRectStart_f(const CCommand &args);
void GModRectSetPos_f(const CCommand &args);
void GModRectSetSize_f(const CCommand &args);
void GModRectSetColor_f(const CCommand &args);
void GModRectSetTime_f(const CCommand &args);
void GModRectSetDelay_f(const CCommand &args);
void GModRectAnimate_f(const CCommand &args);
void GModRectHide_f(const CCommand &args);
void GModRectHideAll_f(const CCommand &args);

// Utility functions
void InitGModMessageSystem();
void ShutdownGModMessageSystem();

#endif // GMOD_MESSAGE_H