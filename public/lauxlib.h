/*
** Lua 5.0.2 Auxiliary Library Header Stub for BarrysMod
** This is a minimal header stub for compilation
** In production, use the real Lua 5.0.2 headers
*/

#ifndef LAUXLIB_H
#define LAUXLIB_H

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

// Auxiliary library functions
void luaL_openlibs(lua_State *L);
int luaL_dofile(lua_State *L, const char *filename);

#ifdef __cplusplus
}
#endif

#endif