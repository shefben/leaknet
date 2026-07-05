//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lua effect and UI functions for BarrysMod
// Based on reverse engineering of Garry's Mod 9.0.4b server.dll
// Includes GModText, GModRect, GModQuad, and effect dispatch functions
//
//=============================================================================//

#include "cbase.h"
#include "lua_integration.h"
#include "player.h"
#include "te_effect_dispatch.h"
#include "recipientfilter.h"
#include "usermessages.h"
#include "soundent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// GMODTEXT FUNCTIONS
// These send user messages to clients for on-screen text display
//=============================================================================

// Current GModText state for building messages
static struct {
	char fontName[64];
	char text[256];
	float x, y;
	float r, g, b, a;
	float fadeIn, fadeOut, holdTime;
	int effect;
	int align;
	int entityID;
	float entityOffsetX, entityOffsetY, entityOffsetZ;
	int textID;
	float delay;          // original _GModText_SetDelay
	int allowOffscreen;   // original _GModText_AllowOffscreen
	int additive;         // original _GModText_SetAdditive
} g_GModText = {"Default", "", 0.5f, 0.5f, 255, 255, 255, 255, 0.1f, 0.1f, 5.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0, 0};

// _GModTextStart - Start building a text message
int Lua_GModTextStart(lua_State *L)
{
	const char *font = CLuaUtility::GetString(L, 1, "Default");
	Q_strncpy(g_GModText.fontName, font, sizeof(g_GModText.fontName));
	g_GModText.text[0] = '\0';
	return 0;
}

// _GModTextSetPos - Set text position (0-1 normalized)
int Lua_GModTextSetPos(lua_State *L)
{
	g_GModText.x = CLuaUtility::GetFloat(L, 1, 0.5f);
	g_GModText.y = CLuaUtility::GetFloat(L, 2, 0.5f);
	return 0;
}

// _GModTextSetColor - Set text color
int Lua_GModTextSetColor(lua_State *L)
{
	g_GModText.r = CLuaUtility::GetFloat(L, 1, 255);
	g_GModText.g = CLuaUtility::GetFloat(L, 2, 255);
	g_GModText.b = CLuaUtility::GetFloat(L, 3, 255);
	g_GModText.a = CLuaUtility::GetFloat(L, 4, 255);
	return 0;
}

// _GModTextSetFade - Set fade times
int Lua_GModTextSetFade(lua_State *L)
{
	g_GModText.fadeIn = CLuaUtility::GetFloat(L, 1, 0.1f);
	g_GModText.fadeOut = CLuaUtility::GetFloat(L, 2, 0.1f);
	g_GModText.holdTime = CLuaUtility::GetFloat(L, 3, 5.0f);
	return 0;
}

// _GModTextSetText - Set the text string
int Lua_GModTextSetText(lua_State *L)
{
	const char *text = CLuaUtility::GetString(L, 1, "");
	Q_strncpy(g_GModText.text, text, sizeof(g_GModText.text));
	return 0;
}

// _GModTextSetEffect - Set text effect
int Lua_GModTextSetEffect(lua_State *L)
{
	g_GModText.effect = CLuaUtility::GetInt(L, 1, 0);
	return 0;
}

// _GModTextSetAlign - Set text alignment
int Lua_GModTextSetAlign(lua_State *L)
{
	g_GModText.align = CLuaUtility::GetInt(L, 1, 0);
	return 0;
}

// _GModText_SetTime - Set timing (holdTime, fadeIn, fadeOut) - GMod style
int Lua_GModTextSetTime(lua_State *L)
{
	g_GModText.holdTime = CLuaUtility::GetFloat(L, 1, 5.0f);
	g_GModText.fadeIn = CLuaUtility::GetFloat(L, 2, 0.1f);
	g_GModText.fadeOut = CLuaUtility::GetFloat(L, 3, 0.1f);
	return 0;
}

// _GModText_SetEntity - Attach text to entity
int Lua_GModTextSetEntity(lua_State *L)
{
	g_GModText.entityID = CLuaUtility::GetInt(L, 1, 0);
	return 0;
}

// _GModText_SetEntityOffset - Set offset when attached to entity
int Lua_GModTextSetEntityOffset(lua_State *L)
{
	// Expect a vector3 table or 3 floats
	if (lua_istable(L, 1))
	{
		lua_getfield(L, 1, "x");
		g_GModText.entityOffsetX = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 1, "y");
		g_GModText.entityOffsetY = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 1, "z");
		g_GModText.entityOffsetZ = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	else
	{
		g_GModText.entityOffsetX = CLuaUtility::GetFloat(L, 1, 0);
		g_GModText.entityOffsetY = CLuaUtility::GetFloat(L, 2, 0);
		g_GModText.entityOffsetZ = CLuaUtility::GetFloat(L, 3, 0);
	}
	return 0;
}

// _GModText_SetDelay - delay (seconds) before the text is shown. Matches original gmod.
int Lua_GModTextSetDelay(lua_State *L)
{
	g_GModText.delay = CLuaUtility::GetFloat(L, 1, 0.0f);
	return 0;
}

// _GModText_AllowOffscreen - allow entity-attached text to draw when the entity is offscreen.
int Lua_GModTextAllowOffscreen(lua_State *L)
{
	g_GModText.allowOffscreen = CLuaUtility::GetInt(L, 1, 0);
	return 0;
}

// _GModText_SetAdditive - additive blend for the text.
int Lua_GModTextSetAdditive(lua_State *L)
{
	g_GModText.additive = CLuaUtility::GetInt(L, 1, 0);
	return 0;
}

// _GModTextSend - Send text to player(s)
// Params: playerID (0 for all), textID
int Lua_GModTextSend(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1, 0);
	int textID = CLuaUtility::GetInt(L, 2, 0);

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	// Send GModText user message
	UserMessageBegin(filter, "GModText");
		WRITE_SHORT(textID);
		WRITE_STRING(g_GModText.fontName);
		WRITE_STRING(g_GModText.text);
		WRITE_FLOAT(g_GModText.x);
		WRITE_FLOAT(g_GModText.y);
		WRITE_BYTE((int)g_GModText.r);
		WRITE_BYTE((int)g_GModText.g);
		WRITE_BYTE((int)g_GModText.b);
		WRITE_BYTE((int)g_GModText.a);
		WRITE_FLOAT(g_GModText.fadeIn);
		WRITE_FLOAT(g_GModText.fadeOut);
		WRITE_FLOAT(g_GModText.holdTime);
		WRITE_BYTE(g_GModText.effect);
		WRITE_SHORT(g_GModText.entityID);
		WRITE_FLOAT(g_GModText.entityOffsetX);
		WRITE_FLOAT(g_GModText.entityOffsetY);
		WRITE_FLOAT(g_GModText.entityOffsetZ);
	MessageEnd();

	return 0;
}

// _GModTextSendAnimate - Send animated text to player(s)
// Params: playerID (0 for all), textID, scale, duration
int Lua_GModTextSendAnimate(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1, 0);
	int textID = CLuaUtility::GetInt(L, 2, 0);
	float scale = CLuaUtility::GetFloat(L, 3, 1.0f);
	float duration = CLuaUtility::GetFloat(L, 4, 0.5f);

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	// Send GModTextAnimateate user message
	UserMessageBegin(filter, "GModTextAnimate");
		WRITE_SHORT(textID);
		WRITE_FLOAT(g_GModText.x);
		WRITE_FLOAT(g_GModText.y);
		WRITE_BYTE((int)g_GModText.r);
		WRITE_BYTE((int)g_GModText.g);
		WRITE_BYTE((int)g_GModText.b);
		WRITE_BYTE((int)g_GModText.a);
		WRITE_FLOAT(scale);
		WRITE_FLOAT(duration);
	MessageEnd();

	return 0;
}

// _GModText_Hide - Hide specific text for player(s)
// Params: playerID, textID, fadeTime, delay
int Lua_GModTextHide(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1, 0);
	int textID = CLuaUtility::GetInt(L, 2, 0);
	float fadeTime = CLuaUtility::GetFloat(L, 3, 0);
	float delay = CLuaUtility::GetFloat(L, 4, 0);

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	UserMessageBegin(filter, "GModTextHide");
		WRITE_SHORT(textID);
		WRITE_FLOAT(fadeTime);
		WRITE_FLOAT(delay);
	MessageEnd();

	return 0;
}

// _GModTextHideAll - Hide all text for player(s)
int Lua_GModTextHideAll(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1, 0);

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	UserMessageBegin(filter, "GModTextHideAll");
	MessageEnd();

	return 0;
}

//=============================================================================
// GMODRECT FUNCTIONS
// These send user messages for on-screen rectangle display
//=============================================================================

// Current GModRect state
static struct {
	char material[256];
	float x, y, w, h;
	float r, g, b, a;
	float holdTime, fadeIn, fadeOut;
	float delay;
	int id;
	float entityOffsetX, entityOffsetY, entityOffsetZ;  // original _GModRect_SetEntityOffset
} g_GModRect = {"", 0, 0, 100, 100, 255, 255, 255, 128, 5.0f, 0.1f, 0.1f, 0, 0, 0, 0, 0};

// _GModRect_Start - Initialize rect with material
int Lua_GModRectStart(lua_State *L)
{
	const char *material = CLuaUtility::GetString(L, 1, "");
	Q_strncpy(g_GModRect.material, material, sizeof(g_GModRect.material));
	// Reset other values
	g_GModRect.delay = 0;
	g_GModRect.holdTime = 5.0f;
	g_GModRect.fadeIn = 0.1f;
	g_GModRect.fadeOut = 0.1f;
	return 0;
}

// _GModRect_SetPos - Set rectangle position and size
// GMod style: x, y, w, h
int Lua_GModRectSetPos(lua_State *L)
{
	g_GModRect.x = CLuaUtility::GetFloat(L, 1, 0);
	g_GModRect.y = CLuaUtility::GetFloat(L, 2, 0);
	// If 4 params, also set w and h
	if (lua_gettop(L) >= 4)
	{
		g_GModRect.w = CLuaUtility::GetFloat(L, 3, 100);
		g_GModRect.h = CLuaUtility::GetFloat(L, 4, 100);
	}
	return 0;
}

// _GModRectSetSize - Set rectangle size
int Lua_GModRectSetSize(lua_State *L)
{
	g_GModRect.w = CLuaUtility::GetFloat(L, 1, 100);
	g_GModRect.h = CLuaUtility::GetFloat(L, 2, 100);
	return 0;
}

// _GModRectSetColor - Set rectangle color
int Lua_GModRectSetColor(lua_State *L)
{
	g_GModRect.r = CLuaUtility::GetFloat(L, 1, 255);
	g_GModRect.g = CLuaUtility::GetFloat(L, 2, 255);
	g_GModRect.b = CLuaUtility::GetFloat(L, 3, 255);
	g_GModRect.a = CLuaUtility::GetFloat(L, 4, 255);
	return 0;
}

// _GModRectSetID - Set rectangle ID
int Lua_GModRectSetID(lua_State *L)
{
	g_GModRect.id = CLuaUtility::GetInt(L, 1, 0);
	return 0;
}

// _GModRect_SetTime - Set timing (holdTime, fadeIn, fadeOut)
int Lua_GModRectSetTime(lua_State *L)
{
	g_GModRect.holdTime = CLuaUtility::GetFloat(L, 1, 5.0f);
	g_GModRect.fadeIn = CLuaUtility::GetFloat(L, 2, 0.1f);
	g_GModRect.fadeOut = CLuaUtility::GetFloat(L, 3, 0.1f);
	return 0;
}

// _GModRect_SetDelay - Set delay before showing
int Lua_GModRectSetDelay(lua_State *L)
{
	g_GModRect.delay = CLuaUtility::GetFloat(L, 1, 0);
	return 0;
}

// _GModRectSend - Send rectangle to player(s)
// Params: playerID (0 for all), rectID
int Lua_GModRectSend(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1, 0);
	int rectID = CLuaUtility::GetInt(L, 2, 0);

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	UserMessageBegin(filter, "GModRect");
		WRITE_SHORT(rectID);
		WRITE_STRING(g_GModRect.material);
		WRITE_FLOAT(g_GModRect.x);
		WRITE_FLOAT(g_GModRect.y);
		WRITE_FLOAT(g_GModRect.w);
		WRITE_FLOAT(g_GModRect.h);
		WRITE_BYTE((int)g_GModRect.r);
		WRITE_BYTE((int)g_GModRect.g);
		WRITE_BYTE((int)g_GModRect.b);
		WRITE_BYTE((int)g_GModRect.a);
		WRITE_FLOAT(g_GModRect.holdTime);
		WRITE_FLOAT(g_GModRect.fadeIn);
		WRITE_FLOAT(g_GModRect.fadeOut);
		WRITE_FLOAT(g_GModRect.delay);
	MessageEnd();

	return 0;
}

// _GModRect_SendAnimate - Send animated rect to player(s)
// Params: playerID (0 for all), rectID, targetX, targetY
int Lua_GModRectSendAnimate(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1, 0);
	int rectID = CLuaUtility::GetInt(L, 2, 0);
	float targetX = CLuaUtility::GetFloat(L, 3, g_GModRect.x);
	float targetY = CLuaUtility::GetFloat(L, 4, g_GModRect.y);

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	UserMessageBegin(filter, "GModRectAnimate");
		WRITE_SHORT(rectID);
		WRITE_FLOAT(g_GModRect.x);
		WRITE_FLOAT(g_GModRect.y);
		WRITE_FLOAT(g_GModRect.w);
		WRITE_FLOAT(g_GModRect.h);
		WRITE_BYTE((int)g_GModRect.r);
		WRITE_BYTE((int)g_GModRect.g);
		WRITE_BYTE((int)g_GModRect.b);
		WRITE_BYTE((int)g_GModRect.a);
		WRITE_FLOAT(targetX);
		WRITE_FLOAT(targetY);
		WRITE_FLOAT(g_GModRect.delay);
	MessageEnd();

	return 0;
}

// _GModRectHideAll - Hide all rectangles
int Lua_GModRectHideAll(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1, -1);

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	UserMessageBegin(filter, "GModRectHideAll");
	MessageEnd();

	return 0;
}

//=============================================================================
// GMODQUAD (WORLD QUAD) FUNCTIONS
// These send user messages for 3D world-space quad display
//=============================================================================

// Current GModQuad state
static struct {
	char material[512];
	Vector corners[4];      // 4 corner positions
	float delay;            // Delay before showing
	float fadeIn;           // Fade in time
	float life;             // How long to display
	float fadeOut;          // Fade out time
	int entityID;           // Entity to follow (0 for none)
} g_GModQuad = {"", {Vector(0,0,0), Vector(0,0,0), Vector(0,0,0), Vector(0,0,0)}, 0, 0, 5.0f, 0, 0};

// _GModQuad_Start - Initialize quad with material
int Lua_GModQuadStart(lua_State *L)
{
	const char *material = CLuaUtility::GetString(L, 1, "");

	// Reset quad state
	memset(&g_GModQuad, 0, sizeof(g_GModQuad));
	g_GModQuad.life = 5.0f;
	Q_strncpy(g_GModQuad.material, material, sizeof(g_GModQuad.material));

	return 0;
}

// _GModQuad_SetVector - Set corner position (0-3)
int Lua_GModQuadSetVector(lua_State *L)
{
	int corner = CLuaUtility::GetInt(L, 1, 0);

	if (corner < 0 || corner > 3)
	{
		Warning("GModQuad_SetVector: Corner index out of range (0-3)\n");
		return 0;
	}

	// Get vector from argument 2
	float x = CLuaUtility::GetFloat(L, 2);
	float y = CLuaUtility::GetFloat(L, 3);
	float z = CLuaUtility::GetFloat(L, 4);
	g_GModQuad.corners[corner] = Vector(x, y, z);

	return 0;
}

// _GModQuad_SetTimings - Set delay, fadeIn, life, fadeOut
int Lua_GModQuadSetTimings(lua_State *L)
{
	g_GModQuad.delay = CLuaUtility::GetFloat(L, 1, 0);
	g_GModQuad.fadeIn = CLuaUtility::GetFloat(L, 2, 0);
	g_GModQuad.life = CLuaUtility::GetFloat(L, 3, 5.0f);
	g_GModQuad.fadeOut = CLuaUtility::GetFloat(L, 4, 0);
	return 0;
}

// _GModQuad_SetEntity - Set entity to follow
int Lua_GModQuadSetEntity(lua_State *L)
{
	g_GModQuad.entityID = CLuaUtility::GetInt(L, 1, 0);
	return 0;
}

// _GModQuad_Send - Send quad to player(s)
int Lua_GModQuadSend(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1, 0);
	int quadID = CLuaUtility::GetInt(L, 2, 0);

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	UserMessageBegin(filter, "WQuad");
		WRITE_BYTE(quadID);
		WRITE_STRING(g_GModQuad.material);
		// Write 4 corner vectors
		for (int i = 0; i < 4; i++)
		{
			WRITE_FLOAT(g_GModQuad.corners[i].x);
			WRITE_FLOAT(g_GModQuad.corners[i].y);
			WRITE_FLOAT(g_GModQuad.corners[i].z);
		}
		// Write timing values
		WRITE_FLOAT(g_GModQuad.delay);
		WRITE_FLOAT(g_GModQuad.fadeIn);
		WRITE_FLOAT(g_GModQuad.life);
		WRITE_FLOAT(g_GModQuad.fadeOut);
		WRITE_LONG(g_GModQuad.entityID);
	MessageEnd();

	return 0;
}

// _GModQuad_SendAnimate - Send animated quad to player(s)
int Lua_GModQuadSendAnimate(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1, 0);
	int quadID = CLuaUtility::GetInt(L, 2, 0);
	float length = CLuaUtility::GetFloat(L, 3, 1.0f);
	float ease = CLuaUtility::GetFloat(L, 4, 0);

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	UserMessageBegin(filter, "WQuadAnimate");
		WRITE_BYTE(quadID);
		WRITE_FLOAT(0);  // Reserved/unused
		WRITE_FLOAT(length);
		WRITE_FLOAT(ease);
		// Write 4 corner vectors
		for (int i = 0; i < 4; i++)
		{
			WRITE_FLOAT(g_GModQuad.corners[i].x);
			WRITE_FLOAT(g_GModQuad.corners[i].y);
			WRITE_FLOAT(g_GModQuad.corners[i].z);
		}
	MessageEnd();

	return 0;
}

// _GModQuad_Hide - Hide specific quad
int Lua_GModQuadHide(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1, 0);
	int quadID = CLuaUtility::GetInt(L, 2, 0);
	float fadeTime = CLuaUtility::GetFloat(L, 3, 0);
	float delay = CLuaUtility::GetFloat(L, 4, 0);

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	UserMessageBegin(filter, "WQuadHide");
		WRITE_BYTE(quadID);
		WRITE_FLOAT(fadeTime);
		WRITE_FLOAT(delay);
	MessageEnd();

	return 0;
}

// _GModQuad_HideAll - Hide all quads
int Lua_GModQuadHideAll(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1, 0);

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	UserMessageBegin(filter, "WQuadHideAll");
	MessageEnd();

	return 0;
}

//=============================================================================
// EFFECT DISPATCH FUNCTIONS
//=============================================================================

// Current effect data
static CEffectData g_LuaEffectData;

// _EffectSetOrigin - Set effect origin
int Lua_EffectSetOrigin(lua_State *L)
{
	float x = CLuaUtility::GetFloat(L, 1);
	float y = CLuaUtility::GetFloat(L, 2);
	float z = CLuaUtility::GetFloat(L, 3);
	g_LuaEffectData.m_vOrigin = Vector(x, y, z);
	return 0;
}

// _EffectSetStart - Set effect start position
int Lua_EffectSetStart(lua_State *L)
{
	float x = CLuaUtility::GetFloat(L, 1);
	float y = CLuaUtility::GetFloat(L, 2);
	float z = CLuaUtility::GetFloat(L, 3);
	g_LuaEffectData.m_vStart = Vector(x, y, z);
	return 0;
}

// _EffectSetNormal - Set effect normal
int Lua_EffectSetNormal(lua_State *L)
{
	float x = CLuaUtility::GetFloat(L, 1);
	float y = CLuaUtility::GetFloat(L, 2);
	float z = CLuaUtility::GetFloat(L, 3);
	g_LuaEffectData.m_vNormal = Vector(x, y, z);
	return 0;
}

// _EffectSetAngles - Set effect angles
int Lua_EffectSetAngles(lua_State *L)
{
	float p = CLuaUtility::GetFloat(L, 1);
	float y = CLuaUtility::GetFloat(L, 2);
	float r = CLuaUtility::GetFloat(L, 3);
	g_LuaEffectData.m_vAngles = QAngle(p, y, r);
	return 0;
}

// _EffectSetEntity - Set effect entity
int Lua_EffectSetEntity(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (pEntity)
		g_LuaEffectData.m_nEntIndex = entIndex;
	return 0;
}

// _EffectSetScale - Set effect scale
int Lua_EffectSetScale(lua_State *L)
{
	g_LuaEffectData.m_flScale = CLuaUtility::GetFloat(L, 1);
	return 0;
}

// _EffectSetRadius - Set effect radius
int Lua_EffectSetRadius_New(lua_State *L)
{
	g_LuaEffectData.m_flRadius = CLuaUtility::GetFloat(L, 1);
	return 0;
}

// _EffectSetMagnitude - Set effect magnitude
int Lua_EffectSetMagnitude(lua_State *L)
{
	g_LuaEffectData.m_flMagnitude = CLuaUtility::GetFloat(L, 1);
	return 0;
}

// _EffectSetColor - Set effect color
int Lua_EffectSetColor(lua_State *L)
{
	int r = CLuaUtility::GetInt(L, 1);
	int g = CLuaUtility::GetInt(L, 2);
	int b = CLuaUtility::GetInt(L, 3);
	g_LuaEffectData.m_nColor = (r) | (g << 8) | (b << 16);
	return 0;
}

// _EffectSetFlags - Set effect flags
int Lua_EffectSetFlags(lua_State *L)
{
	g_LuaEffectData.m_fFlags = CLuaUtility::GetInt(L, 1);
	return 0;
}

// _EffectSetMaterialIndex - Set effect material
int Lua_EffectSetMaterialIndex(lua_State *L)
{
	g_LuaEffectData.m_nMaterial = CLuaUtility::GetInt(L, 1);
	return 0;
}

// _EffectDispatch - Dispatch the effect
int Lua_EffectDispatch(lua_State *L)
{
	const char *effectName = CLuaUtility::GetString(L, 1);
	if (!effectName || !*effectName)
		return 0;

	DispatchEffect(effectName, g_LuaEffectData);

	// Reset for next effect
	g_LuaEffectData = CEffectData();
	return 0;
}

//=============================================================================
// SOUND FUNCTIONS
//=============================================================================

// _EmitSound - Emit sound at position
int Lua_EmitSound(lua_State *L)
{
	float x = CLuaUtility::GetFloat(L, 1);
	float y = CLuaUtility::GetFloat(L, 2);
	float z = CLuaUtility::GetFloat(L, 3);
	const char *soundName = CLuaUtility::GetString(L, 4);
	float volume = CLuaUtility::GetFloat(L, 5, 1.0f);
	int pitch = CLuaUtility::GetInt(L, 6, 100);

	if (!soundName || !*soundName)
		return 0;

	Vector pos(x, y, z);

	// HL2 beta uses simpler EmitSound API without EmitSound_t struct
	CPASAttenuationFilter filter(pos);
	CBaseEntity::EmitSound(filter, 0, CHAN_AUTO, soundName, volume, SNDLVL_NORM, 0, pitch, &pos);
	return 0;
}

// _PrecacheSound - Precache a sound
int Lua_PrecacheSound(lua_State *L)
{
	const char *soundName = CLuaUtility::GetString(L, 1);
	if (soundName && *soundName)
		CBaseEntity::PrecacheScriptSound(soundName);
	return 0;
}

// _StopSound - Stop sound on entity
int Lua_StopSound(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	const char *soundName = CLuaUtility::GetString(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (pEntity && soundName)
	{
		pEntity->StopSound(soundName);
	}
	return 0;
}

//=============================================================================
// USERMESSAGE HELPER FUNCTIONS
//=============================================================================

// _SendHint - Send hint message to player
int Lua_SendHint(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *hint = CLuaUtility::GetString(L, 2);

	if (!hint || !*hint)
		return 0;

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	UserMessageBegin(filter, "GModHint");
		WRITE_STRING(hint);
	MessageEnd();

	return 0;
}

// _SendToolText - Send tool text to player
int Lua_SendToolText(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *text = CLuaUtility::GetString(L, 2);

	if (!text)
		return 0;

	CRecipientFilter filter;
	if (playerID > 0)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
		if (pPlayer)
			filter.AddRecipient(pPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}
	filter.MakeReliable();

	UserMessageBegin(filter, "GModToolText");
		WRITE_STRING(text);
	MessageEnd();

	return 0;
}

// _ClientPrint - Print message to client
int Lua_ClientPrint(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int dest = CLuaUtility::GetInt(L, 2, HUD_PRINTTALK);
	const char *msg = CLuaUtility::GetString(L, 3);

	if (!msg)
		return 0;

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (pPlayer)
	{
		ClientPrint(pPlayer, dest, msg);
	}
	return 0;
}

// _PrintAll - Print message to all clients
int Lua_PrintAll(lua_State *L)
{
	int dest = CLuaUtility::GetInt(L, 1, HUD_PRINTTALK);
	const char *msg = CLuaUtility::GetString(L, 2);

	if (!msg)
		return 0;

	UTIL_ClientPrintAll(dest, msg);
	return 0;
}

//=============================================================================
// SPAWN MENU FUNCTIONS (GMod 9 compatibility)
// These send user messages to clients to update their spawn menu
//=============================================================================

// _spawnmenu_AddItem - Add item to spawn menu
// Syntax: <playerid> <category> <name> <model/ent/etc>
int Lua_SpawnMenu_AddItem(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *category = CLuaUtility::GetString(L, 2);
	const char *displayName = CLuaUtility::GetString(L, 3);
	const char *modelPath = CLuaUtility::GetString(L, 4);

	if (!category || !displayName || !modelPath)
		return CLuaUtility::LuaError(L, "_spawnmenu_AddItem: Missing required parameters");

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_spawnmenu_AddItem: Invalid player ID");

	// Send GModAddSpawnItem user message
	CSingleUserRecipientFilter filter(pPlayer);
	filter.MakeReliable();

	UserMessageBegin(filter, "GModAddSpawnItem");
		WRITE_STRING(category);
		WRITE_STRING(displayName);
		WRITE_STRING(modelPath);
	MessageEnd();

	return 0;
}

// _spawnmenu_RemoveCategory - Remove spawn menu category
// Syntax: <playerid> <category>
int Lua_SpawnMenu_RemoveCategory(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *category = CLuaUtility::GetString(L, 2);

	if (!category)
		return CLuaUtility::LuaError(L, "_spawnmenu_RemoveCategory: Missing category");

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_spawnmenu_RemoveCategory: Invalid player ID");

	// Send GModRemoveSpawnCat user message
	CSingleUserRecipientFilter filter(pPlayer);
	filter.MakeReliable();

	UserMessageBegin(filter, "GModRemoveSpawnCat");
		WRITE_STRING(category);
	MessageEnd();

	return 0;
}

// _spawnmenu_RemoveAll - Remove all spawn menu items for player
// Syntax: <playerid>
int Lua_SpawnMenu_RemoveAll(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_spawnmenu_RemoveAll: Invalid player ID");

	// Send GModRemoveSpawnAll user message
	CSingleUserRecipientFilter filter(pPlayer);
	filter.MakeReliable();

	UserMessageBegin(filter, "GModRemoveSpawnAll");
	MessageEnd();

	return 0;
}

// _spawnmenu_SetCategory - Set current spawn menu category
// Syntax: <playerid> <category>
int Lua_SpawnMenu_SetCategory(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *category = CLuaUtility::GetString(L, 2);

	if (!category)
		return CLuaUtility::LuaError(L, "_spawnmenu_SetCategory: Missing category");

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_spawnmenu_SetCategory: Invalid player ID");

	// Send Spawn_SetCategory user message
	CSingleUserRecipientFilter filter(pPlayer);
	filter.MakeReliable();

	UserMessageBegin(filter, "Spawn_SetCategory");
		WRITE_STRING(category);
	MessageEnd();

	return 0;
}

//=============================================================================
// REGISTRATION
//=============================================================================

// _GModRect_SetEntityOffset - offset for an entity-attached rect. Matches original.
int Lua_GModRectSetEntityOffset(lua_State *L)
{
	if (lua_istable(L, 1))
	{
		lua_getfield(L, 1, "x"); g_GModRect.entityOffsetX = lua_tonumber(L, -1); lua_pop(L, 1);
		lua_getfield(L, 1, "y"); g_GModRect.entityOffsetY = lua_tonumber(L, -1); lua_pop(L, 1);
		lua_getfield(L, 1, "z"); g_GModRect.entityOffsetZ = lua_tonumber(L, -1); lua_pop(L, 1);
	}
	else
	{
		g_GModRect.entityOffsetX = CLuaUtility::GetFloat(L, 1, 0);
		g_GModRect.entityOffsetY = CLuaUtility::GetFloat(L, 2, 0);
		g_GModRect.entityOffsetZ = CLuaUtility::GetFloat(L, 3, 0);
	}
	return 0;
}

// _EffectSetAttachIndex - attachment point index for the next dispatched effect. Matches original.
static int g_EffectAttachIndex = 0;
int Lua_EffectSetAttachIndex(lua_State *L)
{
	g_EffectAttachIndex = CLuaUtility::GetInt(L, 1, 0);
	return 0;
}

void RegisterLuaEffectFunctions()
{
	// GModText functions (original names)

	// GModText aliases with underscore separator (for GMod script compatibility)
	CLuaIntegration::RegisterFunction("_GModText_Start", Lua_GModTextStart, "Start text message. Syntax: [fontname]");
	CLuaIntegration::RegisterFunction("_GModText_SetPos", Lua_GModTextSetPos, "Set text position. Syntax: <x> <y>");
	CLuaIntegration::RegisterFunction("_GModText_SetColor", Lua_GModTextSetColor, "Set text color. Syntax: <r> <g> <b> [a]");
	CLuaIntegration::RegisterFunction("_GModText_SetFade", Lua_GModTextSetFade, "Set fade times. Syntax: <fadeIn> <fadeOut> <holdTime>");
	CLuaIntegration::RegisterFunction("_GModText_SetText", Lua_GModTextSetText, "Set text string. Syntax: <text>");
	CLuaIntegration::RegisterFunction("_GModText_SetEffect", Lua_GModTextSetEffect, "Set text effect. Syntax: <effect>");
	CLuaIntegration::RegisterFunction("_GModText_SetAlign", Lua_GModTextSetAlign, "Set text alignment. Syntax: <align>");
	CLuaIntegration::RegisterFunction("_GModText_Send", Lua_GModTextSend, "Send text to player. Syntax: [playerid]");
	CLuaIntegration::RegisterFunction("_GModText_Hide", Lua_GModTextHide, "Hide text. Syntax: <playerid> <textid> [fadeTime] [delay]");
	CLuaIntegration::RegisterFunction("_GModText_SetTime", Lua_GModTextSetTime, "Set timing. Syntax: <holdTime> [fadeIn] [fadeOut]");
	CLuaIntegration::RegisterFunction("_GModText_SetEntity", Lua_GModTextSetEntity, "Attach to entity. Syntax: <entityID>");
	CLuaIntegration::RegisterFunction("_GModText_SetEntityOffset", Lua_GModTextSetEntityOffset, "Set entity offset. Syntax: <vector3>");
	CLuaIntegration::RegisterFunction("_GModText_SendAnimate", Lua_GModTextSendAnimate, "Send animated text. Syntax: <playerid> <textid> [scale] [duration]");
	// Functions the original gmod registers that shipped content uses (were missing -> nil-call errors)
	CLuaIntegration::RegisterFunction("_GModText_SetDelay", Lua_GModTextSetDelay, "Delay before showing text. Syntax: <seconds>");
	CLuaIntegration::RegisterFunction("_GModText_AllowOffscreen", Lua_GModTextAllowOffscreen, "Allow entity text offscreen. Syntax: <bool>");
	CLuaIntegration::RegisterFunction("_GModText_SetAdditive", Lua_GModTextSetAdditive, "Additive blend. Syntax: <bool>");
	CLuaIntegration::RegisterFunction("_GModText_HideAll", Lua_GModTextHideAll, "Hide all text. Syntax: [playerid]");

	// GModRect functions (original names)

	// GModRect aliases with underscore separator (for GMod script compatibility)
	CLuaIntegration::RegisterFunction("_GModRect_Start", Lua_GModRectStart, "Initialize rect with material. Syntax: [material]");
	CLuaIntegration::RegisterFunction("_GModRect_SetPos", Lua_GModRectSetPos, "Set rect position. Syntax: <x> <y> [w] [h]");
	CLuaIntegration::RegisterFunction("_GModRect_SetSize", Lua_GModRectSetSize, "Set rect size. Syntax: <w> <h>");
	CLuaIntegration::RegisterFunction("_GModRect_SetColor", Lua_GModRectSetColor, "Set rect color. Syntax: <r> <g> <b> [a]");
	CLuaIntegration::RegisterFunction("_GModRect_SetID", Lua_GModRectSetID, "Set rect ID. Syntax: <id>");
	CLuaIntegration::RegisterFunction("_GModRect_SetTime", Lua_GModRectSetTime, "Set timing. Syntax: <holdTime> [fadeIn] [fadeOut]");
	CLuaIntegration::RegisterFunction("_GModRect_SetDelay", Lua_GModRectSetDelay, "Set delay. Syntax: <delay>");
	CLuaIntegration::RegisterFunction("_GModRect_Send", Lua_GModRectSend, "Send rect to player. Syntax: <playerid> [rectid]");
	CLuaIntegration::RegisterFunction("_GModRect_SendAnimate", Lua_GModRectSendAnimate, "Send animated rect. Syntax: <playerid> <rectid> [targetX] [targetY]");
	CLuaIntegration::RegisterFunction("_GModRect_Hide", Lua_GModRectHideAll, "Hide all rects. Syntax: [playerid]");
	// Original gmod name (bmod previously only had _GModRect_Hide)
	CLuaIntegration::RegisterFunction("_GModRect_HideAll", Lua_GModRectHideAll, "Hide all rects. Syntax: [playerid]");

	// GModQuad (World Quad) functions
	CLuaIntegration::RegisterFunction("_GModQuad_Start", Lua_GModQuadStart, "Initialize quad. Syntax: <material>");
	CLuaIntegration::RegisterFunction("_GModQuad_SetVector", Lua_GModQuadSetVector, "Set corner vector. Syntax: <corner 0-3> <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_GModQuad_SetTimings", Lua_GModQuadSetTimings, "Set timings. Syntax: <delay> <fadeIn> <life> <fadeOut>");
	CLuaIntegration::RegisterFunction("_GModQuad_SetEntity", Lua_GModQuadSetEntity, "Set entity to follow. Syntax: <entityid>");
	CLuaIntegration::RegisterFunction("_GModQuad_Send", Lua_GModQuadSend, "Send quad. Syntax: <player> <index>");
	CLuaIntegration::RegisterFunction("_GModQuad_SendAnimate", Lua_GModQuadSendAnimate, "Send animated quad. Syntax: <player> <index> <length> <ease>");
	CLuaIntegration::RegisterFunction("_GModQuad_Hide", Lua_GModQuadHide, "Hide quad. Syntax: <player> <index> <fadeTime> <delay>");
	CLuaIntegration::RegisterFunction("_GModQuad_HideAll", Lua_GModQuadHideAll, "Hide all quads. Syntax: <player>");

	// Effect dispatch (original names)
	CLuaIntegration::RegisterFunction("_EffectSetOrigin", Lua_EffectSetOrigin, "Set effect origin. Syntax: <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_EffectSetStart", Lua_EffectSetStart, "Set effect start. Syntax: <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_EffectSetNormal", Lua_EffectSetNormal, "Set effect normal. Syntax: <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_EffectSetAngles", Lua_EffectSetAngles, "Set effect angles. Syntax: <p> <y> <r>");
	CLuaIntegration::RegisterFunction("_EffectSetEntity", Lua_EffectSetEntity, "Set effect entity. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EffectSetScale", Lua_EffectSetScale, "Set effect scale. Syntax: <scale>");
	CLuaIntegration::RegisterFunction("_EffectSetRadius", Lua_EffectSetRadius_New, "Set effect radius. Syntax: <radius>");
	CLuaIntegration::RegisterFunction("_EffectSetMagnitude", Lua_EffectSetMagnitude, "Set effect magnitude. Syntax: <magnitude>");
	CLuaIntegration::RegisterFunction("_EffectSetColor", Lua_EffectSetColor, "Set effect color. Syntax: <r> <g> <b>");
	CLuaIntegration::RegisterFunction("_EffectSetFlags", Lua_EffectSetFlags, "Set effect flags. Syntax: <flags>");
	CLuaIntegration::RegisterFunction("_EffectSetAttachIndex", Lua_EffectSetAttachIndex, "Set effect attachment index. Syntax: <index>");
	CLuaIntegration::RegisterFunction("_GModRect_SetEntityOffset", Lua_GModRectSetEntityOffset, "Set rect entity offset. Syntax: <vector3>");
	CLuaIntegration::RegisterFunction("_EffectSetMaterialIndex", Lua_EffectSetMaterialIndex, "Set effect material. Syntax: <index>");
	CLuaIntegration::RegisterFunction("_EffectDispatch", Lua_EffectDispatch, "Dispatch effect. Syntax: <effectname>");

	// Effect aliases for GMod compatibility
	CLuaIntegration::RegisterFunction("_EffectInit", Lua_EffectSetOrigin, "Initialize effect at origin. Syntax: <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_EffectSetEnt", Lua_EffectSetEntity, "Set effect entity. Syntax: <entindex>");

	// Sound functions
	CLuaIntegration::RegisterFunction("_EmitSound", Lua_EmitSound, "Emit sound at pos. Syntax: <x> <y> <z> <sound> [vol] [pitch]");
	CLuaIntegration::RegisterFunction("_PrecacheSound", Lua_PrecacheSound, "Precache sound. Syntax: <soundname>");
	CLuaIntegration::RegisterFunction("_StopSound", Lua_StopSound, "Stop sound on entity. Syntax: <entindex> <soundname>");

	// Usermessage helpers
	CLuaIntegration::RegisterFunction("_SendHint", Lua_SendHint, "Send hint message. Syntax: <playerid> <hint>");
	CLuaIntegration::RegisterFunction("_SendToolText", Lua_SendToolText, "Send tool text. Syntax: <playerid> <text>");
	CLuaIntegration::RegisterFunction("_ClientPrint", Lua_ClientPrint, "Print to client. Syntax: <playerid> <dest> <msg>");
	CLuaIntegration::RegisterFunction("_PrintAll", Lua_PrintAll, "Print to all clients. Syntax: <dest> <msg>");

	// Spawn Menu Functions (GMod 9 compatibility)
}
