#ifndef LUA_COMPAT_H
#define LUA_COMPAT_H

#pragma once

// Compatibility helpers that provide the Lua 5.1-style API functions
// expected by the legacy GMod code while running against Lua 5.0.3.

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#ifndef LUA_LOADLIBNAME
#define LUA_LOADLIBNAME "loadlib"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// These helpers are missing from Lua 5.0.x; we implement them ourselves.
void lua_getfield(lua_State *L, int idx, const char *k);
void lua_setfield(lua_State *L, int idx, const char *k);
int  luaL_loadstring(lua_State *L, const char *s);
int  luaL_dostring(lua_State *L, const char *s);
lua_State* luaL_newstate(void);
int  luaL_dofile(lua_State *L, const char *filename);

// lua_pushinteger doesn't exist in Lua 5.0.3, use lua_pushnumber
#define lua_pushinteger(L, n) lua_pushnumber((L), (lua_Number)(n))

#ifdef __cplusplus
}
#endif

#endif // LUA_COMPAT_H
