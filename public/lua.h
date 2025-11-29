/*
** Lua 5.0.2 Basic Header Stub for BarrysMod
** This is a minimal header stub for compilation
** In production, use the real Lua 5.0.2 headers
*/

#ifndef LUA_H
#define LUA_H

#ifdef __cplusplus
extern "C" {
#endif

// Basic Lua types
typedef struct lua_State lua_State;
typedef int (*lua_CFunction) (lua_State *L);
typedef double lua_Number;

// Basic Lua constants
#define LUA_TNONE		(-1)
#define LUA_TNIL		0
#define LUA_TBOOLEAN	1
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TFUNCTION	6

// Core Lua functions - these would normally be implemented by the Lua library
lua_State* luaL_newstate(void);
void lua_close(lua_State *L);
int lua_atpanic(lua_State *L, lua_CFunction panicf);

void lua_getglobal(lua_State *L, const char *name);
void lua_setglobal(lua_State *L, const char *name);
void lua_register(lua_State *L, const char *name, lua_CFunction f);
void lua_pushcfunction(lua_State *L, lua_CFunction f);
void lua_pushstring(lua_State *L, const char *s);
void lua_pushnil(lua_State *L);
void lua_pushnumber(lua_State *L, lua_Number n);
void lua_pushboolean(lua_State *L, int b);
void lua_newtable(lua_State *L);
void lua_setfield(lua_State *L, int idx, const char *k);
void lua_getfield(lua_State *L, int idx, const char *k);

int lua_isfunction(lua_State *L, int idx);
int lua_isstring(lua_State *L, int idx);
int lua_isnumber(lua_State *L, int idx);
int lua_isboolean(lua_State *L, int idx);
int lua_istable(lua_State *L, int idx);

const char* lua_tostring(lua_State *L, int idx);
lua_Number lua_tonumber(lua_State *L, int idx);
int lua_toboolean(lua_State *L, int idx);

int lua_gettop(lua_State *L);
void lua_pop(lua_State *L, int n);
int lua_pcall(lua_State *L, int nargs, int nresults, int errfunc);
int lua_error(lua_State *L);

// Lua auxiliary library helpers
int luaL_loadstring(lua_State *L, const char *s);
int luaL_dostring(lua_State *L, const char *s);
void lua_settop(lua_State *L, int idx);

#ifdef __cplusplus
}
#endif

#endif
