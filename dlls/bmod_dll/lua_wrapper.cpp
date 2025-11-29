#include "lua_wrapper.h"
#include "cbase.h"
#include "filesystem.h"
#include "gameinterface.h"
#include "recipientfilter.h"

// Static member definitions
lua_State* CLuaWrapper::s_pLuaState = NULL;
char CLuaWrapper::s_szLastError[1024] = "";
bool CLuaWrapper::s_bInitialized = false;
KeyValues *CLuaWrapper::s_pActiveEvent = NULL;

namespace
{
	void OpenStandardLibs(lua_State* L)
	{
		static const luaL_reg libs[] = {
			{"", luaopen_base},
			{LUA_TABLIBNAME, luaopen_table},
			{LUA_IOLIBNAME, luaopen_io},
			{LUA_STRLIBNAME, luaopen_string},
			{LUA_MATHLIBNAME, luaopen_math},
			{LUA_DBLIBNAME, luaopen_debug},
			{LUA_LOADLIBNAME, luaopen_loadlib},
			{NULL, NULL}
		};

		for (const luaL_reg* lib = libs; lib->func; ++lib)
		{
			lua_pushcfunction(L, lib->func);
			lua_pushstring(L, lib->name);
			lua_call(L, 1, 0);
		}
	}

	void SetError(lua_State* L, const char* defaultMessage, char (&buffer)[1024])
	{
		const char* err = defaultMessage;
		if (L && lua_gettop(L) > 0 && lua_isstring(L, -1))
		{
			err = lua_tostring(L, -1);
		}
		Q_strncpy(buffer, err ? err : "", sizeof(buffer));
	}
}

//-----------------------------------------------------------------------------
// Initialize the Lua state and load standard libraries
//-----------------------------------------------------------------------------
bool CLuaWrapper::InitializeLua()
{
	if (s_bInitialized)
		return true;

	s_pActiveEvent = NULL;

	s_pLuaState = lua_open();
	if (!s_pLuaState)
	{
		Q_strncpy(s_szLastError, "Failed to create Lua state", sizeof(s_szLastError));
		return false;
	}

	OpenStandardLibs(s_pLuaState);

	s_bInitialized = true;
	DevMsg("Lua wrapper initialized\n");
	return true;
}

//-----------------------------------------------------------------------------
// Shutdown Lua and cleanup
//-----------------------------------------------------------------------------
void CLuaWrapper::ShutdownLua()
{
	if (s_pLuaState)
	{
		lua_close(s_pLuaState);
		s_pLuaState = NULL;
	}
	if (s_pActiveEvent)
	{
		s_pActiveEvent->deleteThis();
		s_pActiveEvent = NULL;
	}
	s_bInitialized = false;
	DevMsg("Lua wrapper shutdown complete\n");
}

//-----------------------------------------------------------------------------
// Run Lua code from string
//-----------------------------------------------------------------------------
bool CLuaWrapper::RunLuaString(const char* pszLuaCode)
{
	if (!s_pLuaState || !pszLuaCode)
		return false;

	int status = luaL_loadstring(s_pLuaState, pszLuaCode);
	if (status == 0)
	{
		status = lua_pcall(s_pLuaState, 0, LUA_MULTRET, 0);
	}

	if (status != 0)
	{
		SetError(s_pLuaState, "Unknown Lua execution error", s_szLastError);
		lua_pop(s_pLuaState, 1); // remove error
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Run Lua file using Source Engine filesystem
//-----------------------------------------------------------------------------
bool CLuaWrapper::RunLuaFile(const char* pszFilename)
{
	if (!s_pLuaState || !pszFilename)
		return false;

	// Use Source Engine filesystem to read the file (searches GAME path)
	FileHandle_t hFile = filesystem->Open(pszFilename, "rb", "GAME");
	if (!hFile)
	{
		Q_snprintf(s_szLastError, sizeof(s_szLastError), "cannot read %s: No such file or directory", pszFilename);
		return false;
	}

	unsigned int fileSize = filesystem->Size(hFile);
	char* pBuffer = new char[fileSize + 1];
	filesystem->Read(pBuffer, fileSize, hFile);
	filesystem->Close(hFile);
	pBuffer[fileSize] = '\0';

	// Load and execute the Lua code from buffer
	int status = luaL_loadbuffer(s_pLuaState, pBuffer, fileSize, pszFilename);
	if (status == 0)
	{
		status = lua_pcall(s_pLuaState, 0, LUA_MULTRET, 0);
	}

	if (status != 0)
	{
		SetError(s_pLuaState, "Error executing Lua file", s_szLastError);
		lua_pop(s_pLuaState, 1); // remove error
		delete[] pBuffer;
		return false;
	}

	delete[] pBuffer;
	return true;
}

//-----------------------------------------------------------------------------
// Lua stack manipulation functions
//-----------------------------------------------------------------------------
void CLuaWrapper::PushString(const char* pszString)
{
	if (s_pLuaState)
		lua_pushstring(s_pLuaState, pszString ? pszString : "");
}

void CLuaWrapper::PushNumber(double dNumber)
{
	if (s_pLuaState)
		lua_pushnumber(s_pLuaState, dNumber);
}

void CLuaWrapper::PushBool(bool bValue)
{
	if (s_pLuaState)
		lua_pushboolean(s_pLuaState, bValue ? 1 : 0);
}

void CLuaWrapper::PushNil()
{
	if (s_pLuaState)
		lua_pushnil(s_pLuaState);
}

const char* CLuaWrapper::GetString(int iIndex)
{
	if (!s_pLuaState)
		return "";

	const char* str = lua_tostring(s_pLuaState, iIndex);
	return str ? str : "";
}

double CLuaWrapper::GetNumber(int iIndex)
{
	if (!s_pLuaState)
		return 0.0;

	return lua_tonumber(s_pLuaState, iIndex);
}

bool CLuaWrapper::GetBool(int iIndex)
{
	if (!s_pLuaState)
		return false;

	return lua_toboolean(s_pLuaState, iIndex) != 0;
}

CBaseEntity* CLuaWrapper::GetLuaEntity(lua_State* L, int index)
{
	if (!L)
		return NULL;

	// Lua userdata is expected to hold a pointer-sized value to a CBaseEntity*
	if (!lua_isuserdata(L, index))
		return NULL;

	CBaseEntity* pEnt = *(CBaseEntity**)lua_touserdata(L, index);
	return pEnt;
}

const char* CLuaWrapper::GetString(lua_State* L, int index, const char* defaultValue)
{
	if (!L)
		return defaultValue;
	const char* str = lua_tostring(L, index);
	return str ? str : defaultValue;
}

int CLuaWrapper::GetInt(lua_State* L, int index, int defaultValue)
{
	if (!L || !lua_isnumber(L, index))
		return defaultValue;
	return (int)lua_tonumber(L, index);
}

float CLuaWrapper::GetFloat(lua_State* L, int index, float defaultValue)
{
	if (!L || !lua_isnumber(L, index))
		return defaultValue;
	return (float)lua_tonumber(L, index);
}

bool CLuaWrapper::GetBool(lua_State* L, int index, bool defaultValue)
{
	if (!L || !lua_isboolean(L, index))
		return defaultValue;
	return lua_toboolean(L, index) != 0;
}

bool CLuaWrapper::GetVector(lua_State* L, int index, Vector& outVec)
{
	if (!L || !lua_istable(L, index))
		return false;

	lua_rawgeti(L, index, 1); outVec.x = (float)lua_tonumber(L, -1); lua_pop(L, 1);
	lua_rawgeti(L, index, 2); outVec.y = (float)lua_tonumber(L, -1); lua_pop(L, 1);
	lua_rawgeti(L, index, 3); outVec.z = (float)lua_tonumber(L, -1); lua_pop(L, 1);
	return true;
}

void CLuaWrapper::StartGameEvent(CBasePlayer* pPlayer)
{
	if ( !pPlayer || !gameeventmanager )
		return;

	if ( s_pActiveEvent )
	{
		s_pActiveEvent->deleteThis();
		s_pActiveEvent = NULL;
	}

	s_pActiveEvent = new KeyValues( "lua_event" );
	s_pActiveEvent->SetInt( "userid", pPlayer->entindex() );
}

void CLuaWrapper::GameEventSetString(const char* pszName)
{
	if ( s_pActiveEvent && pszName )
	{
		s_pActiveEvent->SetString( "string", pszName );
	}
}

void CLuaWrapper::GameEventSetPlayerInt(CBasePlayer* pSource, CBasePlayer* pTarget, int key, int value)
{
	if ( !s_pActiveEvent || !pSource || !pTarget )
		return;

	// Mirror gmod: store both source and target userids plus key/value.
	s_pActiveEvent->SetInt( "src_userid", pSource->entindex() );
	s_pActiveEvent->SetInt( "tgt_userid", pTarget->entindex() );
	s_pActiveEvent->SetInt( "key", key );
	s_pActiveEvent->SetInt( "value", value );
}

void CLuaWrapper::GameEventSetPlayerVector(CBasePlayer* pSource, const Vector& vec)
{
	if ( !s_pActiveEvent || !pSource )
		return;

	s_pActiveEvent->SetInt( "src_userid", pSource->entindex() );
	s_pActiveEvent->SetFloat( "vec_x", vec.x );
	s_pActiveEvent->SetFloat( "vec_y", vec.y );
	s_pActiveEvent->SetFloat( "vec_z", vec.z );
}

// Commit and fire the active event if one is staged.
void CLuaWrapper::CommitActiveGameEvent()
{
	if ( s_pActiveEvent && gameeventmanager )
	{
		CBroadcastRecipientFilter filter;
		gameeventmanager->FireEvent( s_pActiveEvent, &filter );
		s_pActiveEvent->deleteThis();
		s_pActiveEvent = NULL;
	}
}

int CLuaWrapper::GetTop()
{
	if (!s_pLuaState)
		return 0;

	return lua_gettop(s_pLuaState);
}

void CLuaWrapper::Pop(int iNum)
{
	if (s_pLuaState && iNum > 0)
		lua_pop(s_pLuaState, iNum);
}

//-----------------------------------------------------------------------------
// Call Lua function by name
//-----------------------------------------------------------------------------
bool CLuaWrapper::CallLuaFunction(const char* pszFunctionName, int iArgs, int iReturns)
{
	if (!s_pLuaState || !pszFunctionName)
		return false;

	lua_getglobal(s_pLuaState, pszFunctionName);
	if (!lua_isfunction(s_pLuaState, -1))
	{
		lua_pop(s_pLuaState, 1);
		Q_snprintf(s_szLastError, sizeof(s_szLastError), "Lua function '%s' not found", pszFunctionName);
		return false;
	}

	// Move the function before any arguments already on the stack
	int funcIndex = lua_gettop(s_pLuaState) - iArgs;
	lua_insert(s_pLuaState, funcIndex);

	int status = lua_pcall(s_pLuaState, iArgs, iReturns, 0);
	if (status != 0)
	{
		SetError(s_pLuaState, "Lua runtime error", s_szLastError);
		lua_pop(s_pLuaState, 1); // remove error
		return false;
	}

	return true;
}
