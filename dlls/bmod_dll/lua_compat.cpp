#include "lua_compat.h"
#include <string.h>

extern "C" {

void lua_getfield(lua_State *L, int idx, const char *k)
{
	// 5.1-style helper: pushes value of t[k] onto the stack.
	int absIndex = idx;
	if (idx < 0 && idx > LUA_REGISTRYINDEX)
	{
		absIndex = lua_gettop(L) + idx + 1;
	}

	lua_pushstring(L, k);
	lua_gettable(L, absIndex);
}

void lua_setfield(lua_State *L, int idx, const char *k)
{
	// 5.1-style helper: pops the value from the stack and sets t[k] = value.
	int absIndex = idx;
	if (idx < 0 && idx > LUA_REGISTRYINDEX)
	{
		absIndex = lua_gettop(L) + idx + 1;
	}

	lua_pushstring(L, k);
	// Move key below the value so lua_settable sees (table, key, value)
	lua_insert(L, -2);
	lua_settable(L, absIndex);
}

int luaL_loadstring(lua_State *L, const char *s)
{
	return luaL_loadbuffer(L, s, strlen(s), s);
}

int luaL_dostring(lua_State *L, const char *s)
{
	int status = luaL_loadstring(L, s);
	if (status == 0)
	{
		status = lua_pcall(L, 0, LUA_MULTRET, 0);
	}
	return status;
}

lua_State* luaL_newstate(void)
{
	return lua_open();
}

int luaL_dofile(lua_State *L, const char *filename)
{
	return lua_dofile(L, filename);
}

} // extern "C"
