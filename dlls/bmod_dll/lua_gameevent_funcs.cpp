//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lua game event and misc functions for BarrysMod
// Based on reverse engineering of Garry's Mod 9.0.4b server.dll
// Adapted for HL2 beta leak API (uses KeyValues instead of IGameEvent)
//
//=============================================================================//

#include "cbase.h"
#include "lua_integration.h"
#include "filesystem.h"
#include "player.h"
#include "recipientfilter.h"
#include "usermessages.h"
#include "igameevents.h"
#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// GAME EVENT FUNCTIONS (GMod 9 compatibility)
// Note: HL2 beta leak uses KeyValues-based events, not IGameEvent
//=============================================================================

// Global game event storage (using KeyValues for HL2 beta leak)
static KeyValues *g_pCurrentGameEvent = NULL;
static char g_szCurrentEventName[128] = "";

// _gameevent_start - Start a game event
int Lua_GameEventStart(lua_State *L)
{
	const char *eventName = CLuaUtility::GetString(L, 1);
	if (!eventName || !*eventName)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	// Clean up any previous event
	if (g_pCurrentGameEvent)
	{
		g_pCurrentGameEvent->deleteThis();
		g_pCurrentGameEvent = NULL;
	}

	// Create new KeyValues event
	g_pCurrentGameEvent = new KeyValues(eventName);
	if (g_pCurrentGameEvent)
	{
		Q_strncpy(g_szCurrentEventName, eventName, sizeof(g_szCurrentEventName));
		lua_pushboolean(L, true);
	}
	else
	{
		lua_pushboolean(L, false);
	}
	return 1;
}

// _gameevent_SetString - Set string value in game event
int Lua_GameEventSetString(lua_State *L)
{
	const char *key = CLuaUtility::GetString(L, 1);
	const char *value = CLuaUtility::GetString(L, 2);

	if (!g_pCurrentGameEvent || !key || !value)
		return 0;

	g_pCurrentGameEvent->SetString(key, value);
	return 0;
}

// _gameevent_SetInt - Set int value in game event
int Lua_GameEventSetInt(lua_State *L)
{
	const char *key = CLuaUtility::GetString(L, 1);
	int value = CLuaUtility::GetInt(L, 2);

	if (!g_pCurrentGameEvent || !key)
		return 0;

	g_pCurrentGameEvent->SetInt(key, value);
	return 0;
}

// _gameevent_SetFloat - Set float value in game event
int Lua_GameEventSetFloat(lua_State *L)
{
	const char *key = CLuaUtility::GetString(L, 1);
	float value = CLuaUtility::GetFloat(L, 2);

	if (!g_pCurrentGameEvent || !key)
		return 0;

	g_pCurrentGameEvent->SetFloat(key, value);
	return 0;
}

// _gameevent_Fire - Fire the current game event
int Lua_GameEventFire(lua_State *L)
{
	if (!g_pCurrentGameEvent)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	// Fire event to all clients (note: typo in original HL2 beta code)
	CRelieableBroadcastRecipientFilter filter;
	bool result = gameeventmanager->FireEvent(g_pCurrentGameEvent, &filter);

	// Event is consumed by FireEvent, clear our reference
	g_pCurrentGameEvent = NULL;
	g_szCurrentEventName[0] = '\0';

	lua_pushboolean(L, result);
	return 1;
}

//=============================================================================
// ADDITIONAL CONVAR FUNCTIONS (GMod 9 compatibility)
//=============================================================================

// _GetConVar_Bool - Get convar as boolean
int Lua_GetConVar_Bool(lua_State *L)
{
	const char *name = CLuaUtility::GetString(L, 1);
	if (!name || !*name)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	const ConVar *pConVar = cvar->FindVar(name);
	if (!pConVar)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, pConVar->GetBool());
	return 1;
}

// _GetClientConVar_String - Get client convar (server-side query)
// Note: HL2 beta leak doesn't support direct client convar query
// This is a stub that returns nil - actual implementation would need
// client-side cooperation via usermessages
int Lua_GetClientConVar_String(lua_State *L)
{
	// Not supported in HL2 beta leak engine
	// Would require client-side implementation to query and send back
	lua_pushnil(L);
	return 1;
}

//=============================================================================
// TEAM FUNCTIONS - Already defined in lua_player_funcs.cpp
// Lua_TeamNumPlayers and Lua_TeamCount are in lua_player_funcs.cpp
//=============================================================================

//=============================================================================
// MISC MISSING FUNCTIONS (GMod 9 compatibility)
//=============================================================================

// _StartNextLevel - Start the next map in rotation
int Lua_StartNextLevel(lua_State *L)
{
	engine->ServerCommand("nextlevel\n");
	return 0;
}

// _SetDefaultRelationship - Set default NPC relationship
int Lua_SetDefaultRelationship(lua_State *L)
{
	int class1 = CLuaUtility::GetInt(L, 1);
	int class2 = CLuaUtility::GetInt(L, 2);
	int disposition = CLuaUtility::GetInt(L, 3);

	// This would require access to AI relationship manager
	// For now, this is a stub
	return 0;
}

// _GameSetTargetIDRules - Set target ID display rules
int Lua_GameSetTargetIDRules(lua_State *L)
{
	// Stub - would control player name display
	return 0;
}

// _PluginMsg - Send plugin message
int Lua_PluginMsg(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *msg = CLuaUtility::GetString(L, 2);

	if (!msg)
		return 0;

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return 0;

	// Use TextMsg for plugin messages
	ClientPrint(pPlayer, HUD_PRINTTALK, msg);
	return 0;
}

// _PluginText - Send plugin text to all
int Lua_PluginText(lua_State *L)
{
	const char *msg = CLuaUtility::GetString(L, 1);
	if (!msg)
		return 0;

	UTIL_ClientPrintAll(HUD_PRINTTALK, msg);
	return 0;
}

// _PlayerSetDrawTeamCircle - Set team circle drawing for player
int Lua_PlayerSetDrawTeamCircle(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	bool draw = CLuaUtility::GetBool(L, 2);

	// Stub - would require client-side implementation
	return 0;
}

// _PlayerGetRandomAllowedModel - Get random allowed player model
int Lua_PlayerGetRandomAllowedModel(lua_State *L)
{
	// Return a default model
	lua_pushstring(L, "models/player/combine_soldier.mdl");
	return 1;
}

//=============================================================================
// REGISTRATION
//=============================================================================

void RegisterLuaGameEventFunctions()
{
	// Game event functions (GMod 9 compatibility)
	CLuaIntegration::RegisterFunction("_gameevent_start", Lua_GameEventStart, "Start game event. Syntax: <eventname>");
	CLuaIntegration::RegisterFunction("_gameevent_SetString", Lua_GameEventSetString, "Set event string. Syntax: <key> <value>");
	CLuaIntegration::RegisterFunction("_gameevent_SetInt", Lua_GameEventSetInt, "Set event int. Syntax: <key> <value>");
	CLuaIntegration::RegisterFunction("_gameevent_SetFloat", Lua_GameEventSetFloat, "Set event float. Syntax: <key> <value>");
	CLuaIntegration::RegisterFunction("_gameevent_Fire", Lua_GameEventFire, "Fire game event.");

	// Additional ConVar functions (GMod 9 compatibility)
	CLuaIntegration::RegisterFunction("_GetConVar_Bool", Lua_GetConVar_Bool, "Get convar as bool. Syntax: <name>");
	CLuaIntegration::RegisterFunction("_GetClientConVar_String", Lua_GetClientConVar_String, "Get client convar. Syntax: <playerid> <name>");

	// Team functions - already registered in lua_player_funcs.cpp
	// _TeamNumPlayers and _TeamCount are registered there

	// Misc missing functions (GMod 9 compatibility)
	CLuaIntegration::RegisterFunction("_StartNextLevel", Lua_StartNextLevel, "Start next map in rotation.");
	CLuaIntegration::RegisterFunction("_SetDefaultRelationship", Lua_SetDefaultRelationship, "Set NPC relationship. Syntax: <class1> <class2> <disp>");
	CLuaIntegration::RegisterFunction("_GameSetTargetIDRules", Lua_GameSetTargetIDRules, "Set target ID rules.");
	CLuaIntegration::RegisterFunction("_PluginMsg", Lua_PluginMsg, "Send plugin message. Syntax: <playerid> <msg>");
	CLuaIntegration::RegisterFunction("_PluginText", Lua_PluginText, "Send plugin text to all. Syntax: <msg>");
	CLuaIntegration::RegisterFunction("_PlayerSetDrawTeamCircle", Lua_PlayerSetDrawTeamCircle, "Set team circle. Syntax: <playerid> <bool>");
	CLuaIntegration::RegisterFunction("_PlayerGetRandomAllowedModel", Lua_PlayerGetRandomAllowedModel, "Get random allowed model.");
}
