#include "cbase.h"
#include "lua_integration.h"
#include "player.h"
#include "util.h"
#include <stdarg.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CBaseEntity* CLuaUtility::GetEntityFromIndex(lua_State *L, int idx)
{
	int entIndex = GetInt(L, idx, -1);
	if (entIndex <= 0)
		return NULL;

	return UTIL_EntityByIndex(entIndex);
}

CBasePlayer* CLuaUtility::GetPlayerFromID(lua_State *L, int idx)
{
	int playerIndex = GetInt(L, idx, -1);
	if (playerIndex < 1 || playerIndex > gpGlobals->maxClients)
		return NULL;

	return UTIL_PlayerByIndex(playerIndex);
}

bool CLuaUtility::GetBool(lua_State *L, int idx, bool defaultValue)
{
	if (!L || idx == 0)
		return defaultValue;

	if (lua_isboolean(L, idx))
		return lua_toboolean(L, idx) != 0;
	if (lua_isnumber(L, idx))
		return lua_tonumber(L, idx) != 0.0;
	if (lua_isstring(L, idx))
	{
		const char *str = lua_tostring(L, idx);
		if (!str || !str[0])
			return false;
		if (!Q_stricmp(str, "false") || !Q_stricmp(str, "0"))
			return false;
		return true;
	}
	return defaultValue;
}

int CLuaUtility::GetInt(lua_State *L, int idx, int defaultValue)
{
	if (!L || idx == 0)
		return defaultValue;

	if (lua_isnumber(L, idx))
		return (int)lua_tonumber(L, idx);
	if (lua_isboolean(L, idx))
		return lua_toboolean(L, idx) ? 1 : 0;
	return defaultValue;
}

float CLuaUtility::GetFloat(lua_State *L, int idx, float defaultValue)
{
	if (!L || idx == 0)
		return defaultValue;

	if (lua_isnumber(L, idx))
		return (float)lua_tonumber(L, idx);
	return defaultValue;
}

const char* CLuaUtility::GetString(lua_State *L, int idx, const char *defaultValue)
{
	if (!L || idx == 0)
		return defaultValue;

	if (lua_isstring(L, idx))
		return lua_tostring(L, idx);
	return defaultValue;
}

void CLuaUtility::PushInt(lua_State *L, int value)
{
	if (L)
		lua_pushnumber(L, (lua_Number)value);
}

int CLuaUtility::LuaError(lua_State *L, const char *fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	Q_vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	lua_pushstring(L, buffer);
	return lua_error(L);
}
