//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lua function stubs for BarrysMod compilation
// These are minimal stubs - in production, link against real Lua 5.0.2
//
//=============================================================================//

#include "cbase.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// LUA FUNCTION STUBS
// These are minimal implementations for compilation
// In production, use the real Lua 5.0.2 library
//=============================================================================

extern "C" {

lua_State* luaL_newstate(void)
{
	// Stub: return a dummy pointer
	static int dummy_state = 0;
	Warning("Lua Stub: luaL_newstate called - using stub implementation\n");
	return (lua_State*)&dummy_state;
}

void lua_close(lua_State *L)
{
	// Stub: do nothing
	if (L) {
		DevMsg("Lua Stub: lua_close called\n");
	}
}

int lua_atpanic(lua_State *L, lua_CFunction panicf)
{
	// Stub: do nothing, return 0
	return 0;
}

void luaL_openlibs(lua_State *L)
{
	// Stub: do nothing
	DevMsg("Lua Stub: luaL_openlibs called\n");
}

void lua_getglobal(lua_State *L, const char *name)
{
	// Stub: do nothing
	DevMsg("Lua Stub: lua_getglobal called with '%s'\n", name ? name : "NULL");
}

void lua_setglobal(lua_State *L, const char *name)
{
	// Stub: do nothing
	DevMsg("Lua Stub: lua_setglobal called with '%s'\n", name ? name : "NULL");
}

void lua_pushcfunction(lua_State *L, lua_CFunction f)
{
	// Stub: do nothing
	DevMsg("Lua Stub: lua_pushcfunction called\n");
}

void lua_pushstring(lua_State *L, const char *s)
{
	// Stub: do nothing
	DevMsg("Lua Stub: lua_pushstring called with '%s'\n", s ? s : "NULL");
}

void lua_pushnil(lua_State *L)
{
	// Stub: do nothing
	DevMsg("Lua Stub: lua_pushnil called\n");
}

void lua_pushnumber(lua_State *L, lua_Number n)
{
	// Stub: do nothing
	DevMsg("Lua Stub: lua_pushnumber called with %.2f\n", n);
}

void lua_pushboolean(lua_State *L, int b)
{
	// Stub: do nothing
	DevMsg("Lua Stub: lua_pushboolean called with %d\n", b);
}

int lua_isfunction(lua_State *L, int idx)
{
	// Stub: return false
	return 0;
}

int lua_isstring(lua_State *L, int idx)
{
	// Stub: return false
	return 0;
}

int lua_isnumber(lua_State *L, int idx)
{
	// Stub: return false
	return 0;
}

int lua_isboolean(lua_State *L, int idx)
{
	// Stub: return false
	return 0;
}

const char* lua_tostring(lua_State *L, int idx)
{
	// Stub: return empty string
	return "";
}

lua_Number lua_tonumber(lua_State *L, int idx)
{
	// Stub: return 0
	return 0.0;
}

int lua_toboolean(lua_State *L, int idx)
{
	// Stub: return false
	return 0;
}

int lua_gettop(lua_State *L)
{
	// Stub: return 0
	return 0;
}

void lua_pop(lua_State *L, int n)
{
	// Stub: do nothing
	DevMsg("Lua Stub: lua_pop called with %d\n", n);
}

int lua_pcall(lua_State *L, int nargs, int nresults, int errfunc)
{
	// Stub: return success (0)
	DevMsg("Lua Stub: lua_pcall called\n");
	return 0;
}

int lua_error(lua_State *L)
{
	// Stub: return 0
	Warning("Lua Stub: lua_error called\n");
	return 0;
}

int luaL_dofile(lua_State *L, const char *filename)
{
	// Stub: return success (0)
	Msg("Lua Stub: luaL_dofile called with '%s'\n", filename ? filename : "NULL");
	return 0;
}

} // extern "C"