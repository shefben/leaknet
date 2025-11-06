//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lua utility functions for parameter conversion and entity management
// Based on reverse engineering of Garry's Mod server.dll
//
//=============================================================================//

#include "cbase.h"
#include "lua_integration.h"
#include "player.h"
#include "baseentity.h"
#include "entitylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// PARAMETER HELPER FUNCTIONS
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Get entity from Lua stack by index
//-----------------------------------------------------------------------------
CBaseEntity* CLuaUtility::GetEntityFromIndex(lua_State *L, int index)
{
	if (!lua_isnumber(L, index))
		return NULL;

	int entIndex = (int)lua_tonumber(L, index);

	// Validate entity index range
	if (entIndex < 0 || entIndex >= MAX_EDICTS)
		return NULL;

	return CBaseEntity::Instance(entIndex);
}

//-----------------------------------------------------------------------------
// Purpose: Get player from Lua stack by player ID
//-----------------------------------------------------------------------------
CBasePlayer* CLuaUtility::GetPlayerFromID(lua_State *L, int index)
{
	if (!lua_isnumber(L, index))
		return NULL;

	int playerID = (int)lua_tonumber(L, index);

	// Validate player ID range (1-based)
	if (playerID < 1 || playerID > gpGlobals->maxClients)
		return NULL;

	CBaseEntity *pEntity = UTIL_PlayerByIndex(playerID);
	return ToBasePlayer(pEntity);
}

//-----------------------------------------------------------------------------
// Purpose: Get boolean from Lua stack
//-----------------------------------------------------------------------------
bool CLuaUtility::GetBool(lua_State *L, int index, bool defaultValue)
{
	if (lua_gettop(L) < index)
		return defaultValue;

	if (lua_isboolean(L, index))
		return lua_toboolean(L, index) != 0;

	if (lua_isnumber(L, index))
		return lua_tonumber(L, index) != 0;

	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get integer from Lua stack
//-----------------------------------------------------------------------------
int CLuaUtility::GetInt(lua_State *L, int index, int defaultValue)
{
	if (lua_gettop(L) < index)
		return defaultValue;

	if (lua_isnumber(L, index))
		return (int)lua_tonumber(L, index);

	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get float from Lua stack
//-----------------------------------------------------------------------------
float CLuaUtility::GetFloat(lua_State *L, int index, float defaultValue)
{
	if (lua_gettop(L) < index)
		return defaultValue;

	if (lua_isnumber(L, index))
		return (float)lua_tonumber(L, index);

	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get string from Lua stack
//-----------------------------------------------------------------------------
const char* CLuaUtility::GetString(lua_State *L, int index, const char *defaultValue)
{
	if (lua_gettop(L) < index)
		return defaultValue;

	if (lua_isstring(L, index))
		return lua_tostring(L, index);

	return defaultValue;
}

//=============================================================================
// RETURN VALUE HELPER FUNCTIONS
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Push entity to Lua stack
//-----------------------------------------------------------------------------
void CLuaUtility::PushEntity(lua_State *L, CBaseEntity *pEntity)
{
	if (!pEntity)
	{
		lua_pushnil(L);
		return;
	}

	// Push entity index
	lua_pushnumber(L, pEntity->entindex());
}

//-----------------------------------------------------------------------------
// Purpose: Push player to Lua stack
//-----------------------------------------------------------------------------
void CLuaUtility::PushPlayer(lua_State *L, CBasePlayer *pPlayer)
{
	if (!pPlayer)
	{
		lua_pushnil(L);
		return;
	}

	// Push player ID (1-based)
	lua_pushnumber(L, pPlayer->entindex());
}

//-----------------------------------------------------------------------------
// Purpose: Push boolean to Lua stack
//-----------------------------------------------------------------------------
void CLuaUtility::PushBool(lua_State *L, bool value)
{
	lua_pushboolean(L, value ? 1 : 0);
}

//-----------------------------------------------------------------------------
// Purpose: Push integer to Lua stack
//-----------------------------------------------------------------------------
void CLuaUtility::PushInt(lua_State *L, int value)
{
	lua_pushnumber(L, value);
}

//-----------------------------------------------------------------------------
// Purpose: Push float to Lua stack
//-----------------------------------------------------------------------------
void CLuaUtility::PushFloat(lua_State *L, float value)
{
	lua_pushnumber(L, value);
}

//-----------------------------------------------------------------------------
// Purpose: Push string to Lua stack
//-----------------------------------------------------------------------------
void CLuaUtility::PushString(lua_State *L, const char *value)
{
	if (!value)
	{
		lua_pushnil(L);
		return;
	}

	lua_pushstring(L, value);
}

//=============================================================================
// ERROR HANDLING
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Generate Lua error with formatted message
//-----------------------------------------------------------------------------
int CLuaUtility::LuaError(lua_State *L, const char *pszFormat, ...)
{
	char szBuffer[1024];
	va_list args;
	va_start(args, pszFormat);
	Q_vsnprintf(szBuffer, sizeof(szBuffer), pszFormat, args);
	va_end(args);

	// Log the error
	Warning("Lua Error: %s\n", szBuffer);

	// Push error to Lua stack and generate error
	lua_pushstring(L, szBuffer);
	return lua_error(L);
}