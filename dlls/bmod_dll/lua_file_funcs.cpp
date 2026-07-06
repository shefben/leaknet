//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lua file I/O functions for BarrysMod
// Based on reverse engineering of Garry's Mod 9.0.4b server.dll
//
//=============================================================================//

#include "cbase.h"
#include "lua_integration.h"
#include "filesystem.h"
#include "player.h"
#include "recipientfilter.h"
#include "usermessages.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// FILE FUNCTIONS
//=============================================================================

static bool LuaSoundNameIsRawSample(const char *soundName)
{
	if (!soundName || !*soundName)
		return false;

	const char *extension = Q_strrchr(soundName, '.');
	if (!extension)
		return false;

	return Q_stricmp(extension, ".wav") == 0 || Q_stricmp(extension, ".mp3") == 0;
}

static void LuaPrecacheSoundName(const char *soundName)
{
	if (!soundName || !*soundName)
		return;

	if (LuaSoundNameIsRawSample(soundName))
		enginesound->PrecacheSound(soundName);
	else
		CBaseEntity::PrecacheScriptSound(soundName);
}

// Allowed paths for Lua file access (sandboxed)
static bool IsPathAllowed(const char *path)
{
	if (!path || !*path)
		return false;

	// Only allow paths within specific directories
	// Prevent directory traversal with ..
	if (Q_strstr(path, ".."))
		return false;

	// Allow paths starting with these directories
	if (Q_strnicmp(path, "data/", 5) == 0)
		return true;
	if (Q_strnicmp(path, "lua/", 4) == 0)
		return true;
	if (Q_strnicmp(path, "cfg/", 4) == 0)
		return true;

	return false;
}

// _FileExists - Check if file exists
int Lua_FileExists(lua_State *L)
{
	const char *path = CLuaUtility::GetString(L, 1);

	if (!IsPathAllowed(path))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, filesystem->FileExists(path, "GAME"));
	return 1;
}

// _FileRead - Read file contents
int Lua_FileRead(lua_State *L)
{
	const char *path = CLuaUtility::GetString(L, 1);

	if (!IsPathAllowed(path))
	{
		lua_pushnil(L);
		return 1;
	}

	if (!filesystem->FileExists(path, "GAME"))
	{
		lua_pushnil(L);
		return 1;
	}

	FileHandle_t file = filesystem->Open(path, "rb", "GAME");
	if (!file)
	{
		lua_pushnil(L);
		return 1;
	}

	int size = filesystem->Size(file);
	if (size <= 0 || size > 1024 * 1024) // 1MB limit
	{
		filesystem->Close(file);
		lua_pushnil(L);
		return 1;
	}

	char *buffer = new char[size + 1];
	filesystem->Read(buffer, size, file);
	buffer[size] = '\0';
	filesystem->Close(file);

	lua_pushstring(L, buffer);
	delete[] buffer;
	return 1;
}

// _FileWrite - Write file contents
int Lua_FileWrite(lua_State *L)
{
	const char *path = CLuaUtility::GetString(L, 1);
	const char *content = CLuaUtility::GetString(L, 2);

	if (!path || !*path)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	// Only allow writing to data/ directory
	if (Q_strnicmp(path, "data/", 5) != 0)
	{
		Warning("Lua _FileWrite: Can only write to data/ directory\n");
		lua_pushboolean(L, false);
		return 1;
	}

	if (Q_strstr(path, ".."))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	FileHandle_t file = filesystem->Open(path, "wb", "GAME");
	if (!file)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	int len = Q_strlen(content);
	filesystem->Write(content, len, file);
	filesystem->Close(file);

	lua_pushboolean(L, true);
	return 1;
}

// _FileAppend - Append to file
int Lua_FileAppend(lua_State *L)
{
	const char *path = CLuaUtility::GetString(L, 1);
	const char *content = CLuaUtility::GetString(L, 2);

	if (!path || !*path)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	// Only allow writing to data/ directory
	if (Q_strnicmp(path, "data/", 5) != 0)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	if (Q_strstr(path, ".."))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	FileHandle_t file = filesystem->Open(path, "ab", "GAME");
	if (!file)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	int len = Q_strlen(content);
	filesystem->Write(content, len, file);
	filesystem->Close(file);

	lua_pushboolean(L, true);
	return 1;
}

// _FileDelete - Delete file
int Lua_FileDelete(lua_State *L)
{
	const char *path = CLuaUtility::GetString(L, 1);

	if (!path || !*path)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	// Only allow deleting from data/ directory
	if (Q_strnicmp(path, "data/", 5) != 0)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	if (Q_strstr(path, ".."))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	filesystem->RemoveFile(path, "GAME");
	lua_pushboolean(L, true);
	return 1;
}

// _FileSize - Get file size
int Lua_FileSize(lua_State *L)
{
	const char *path = CLuaUtility::GetString(L, 1);

	if (!IsPathAllowed(path))
	{
		lua_pushinteger(L, -1);
		return 1;
	}

	if (!filesystem->FileExists(path, "GAME"))
	{
		lua_pushinteger(L, -1);
		return 1;
	}

	FileHandle_t file = filesystem->Open(path, "rb", "GAME");
	if (!file)
	{
		lua_pushinteger(L, -1);
		return 1;
	}

	int size = filesystem->Size(file);
	filesystem->Close(file);

	lua_pushinteger(L, size);
	return 1;
}

// _FileFind - Find files matching pattern
int Lua_FileFind(lua_State *L)
{
	const char *pattern = CLuaUtility::GetString(L, 1);
	const char *searchPath = CLuaUtility::GetString(L, 2, "GAME");

	lua_newtable(L);

	if (!pattern || !*pattern)
		return 1;

	// Only allow searching in allowed paths
	if (!IsPathAllowed(pattern))
		return 1;

	FileFindHandle_t findHandle;
	const char *filename = filesystem->FindFirst(pattern, &findHandle);

	int count = 0;
	while (filename)
	{
		count++;
		lua_pushinteger(L, count);
		lua_pushstring(L, filename);
		lua_settable(L, -3);

		filename = filesystem->FindNext(findHandle);
	}

	filesystem->FindClose(findHandle);
	return 1;
}

// _FileIsDir - Check if path is a directory
int Lua_FileIsDir(lua_State *L)
{
	const char *path = CLuaUtility::GetString(L, 1);

	if (!IsPathAllowed(path))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, filesystem->IsDirectory(path, "GAME"));
	return 1;
}

// _FileCreateDir - Create directory
int Lua_FileCreateDir(lua_State *L)
{
	const char *path = CLuaUtility::GetString(L, 1);

	if (!path || !*path)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	// Only allow creating directories in data/
	if (Q_strnicmp(path, "data/", 5) != 0)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	if (Q_strstr(path, ".."))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	filesystem->CreateDirHierarchy(path, "GAME");
	lua_pushboolean(L, true);
	return 1;
}

//=============================================================================
// CORE UTILITY FUNCTIONS
// Note: Lua_GetCurrentMap is already defined in lua_entity_funcs.cpp
//=============================================================================

static int Lua_PushConVarBoolRule(lua_State *L, const char *conVarName, bool defaultValue)
{
	const ConVar *pConVar = cvar->FindVar(conVarName);
	lua_pushboolean(L, pConVar ? (pConVar->GetInt() != 0) : defaultValue);
	return 1;
}

// _GetRule - Get GMod 9 gamerule values backed by server convars.
int Lua_GetRule(lua_State *L)
{
	const char *ruleName = CLuaUtility::GetString(L, 1);
	if (!ruleName || !*ruleName)
	{
		lua_pushnil(L);
		return 1;
	}

	// Common gamerule queries
	if (Q_stricmp(ruleName, "maxplayers") == 0)
	{
		lua_pushinteger(L, gpGlobals->maxClients);
		return 1;
	}
	else if (Q_stricmp(ruleName, "maptime") == 0)
	{
		lua_pushnumber(L, gpGlobals->curtime);
		return 1;
	}
	else if (Q_stricmp(ruleName, "timelimit") == 0)
	{
		const ConVar *pConVar = cvar->FindVar("mp_timelimit");
		lua_pushnumber(L, pConVar ? pConVar->GetFloat() : 0);
		return 1;
	}
	else if (Q_stricmp(ruleName, "fraglimit") == 0)
	{
		const ConVar *pConVar = cvar->FindVar("mp_fraglimit");
		lua_pushinteger(L, pConVar ? pConVar->GetInt() : 0);
		return 1;
	}
	else if (Q_stricmp(ruleName, "Teamplay") == 0)
	{
		return Lua_PushConVarBoolRule(L, "mp_teamplay", false);
	}
	else if (Q_stricmp(ruleName, "SpawnWithAllWeapons") == 0)
	{
		return Lua_PushConVarBoolRule(L, "gm_sv_allweapons", true);
	}
	else if (Q_stricmp(ruleName, "AllowPhysgun") == 0)
	{
		return Lua_PushConVarBoolRule(L, "gm_sv_allowphysgun", true);
	}
	else if (Q_stricmp(ruleName, "AllowMultigun") == 0)
	{
		return Lua_PushConVarBoolRule(L, "gm_sv_allowmultigun", true);
	}
	else if (Q_stricmp(ruleName, "AllowIgnite") == 0)
	{
		return Lua_PushConVarBoolRule(L, "gm_sv_allowignite", true);
	}
	else if (Q_stricmp(ruleName, "AllowNPC") == 0)
	{
		return Lua_PushConVarBoolRule(L, "gm_sv_allownpc", true);
	}
	else if (Q_stricmp(ruleName, "AllowSpawning") == 0)
	{
		return Lua_PushConVarBoolRule(L, "gm_sv_allowspawning", true);
	}
	else if (Q_stricmp(ruleName, "PlayerDamage") == 0)
	{
		return Lua_PushConVarBoolRule(L, "gm_sv_playerdamage", true);
	}
	else if (Q_stricmp(ruleName, "PvPDamage") == 0)
	{
		return Lua_PushConVarBoolRule(L, "gm_sv_pvpdamage", true);
	}
	else if (Q_stricmp(ruleName, "TeamDamage") == 0)
	{
		return Lua_PushConVarBoolRule(L, "gm_sv_teamdamage", false);
	}
	else
	{
		lua_pushnil(L);
	}
	return 1;
}

// _PrintMessage - Print message to specific player
int Lua_PrintMessage(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int msgType = CLuaUtility::GetInt(L, 2, HUD_PRINTTALK);
	const char *msg = CLuaUtility::GetString(L, 3);

	if (!msg)
		return 0;

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (pPlayer)
	{
		ClientPrint(pPlayer, msgType, msg);
	}
	return 0;
}

// _PrintMessageAll - Print message to all players
int Lua_PrintMessageAll(lua_State *L)
{
	int msgType = CLuaUtility::GetInt(L, 1, HUD_PRINTTALK);
	const char *msg = CLuaUtility::GetString(L, 2);

	if (!msg)
		return 0;

	UTIL_ClientPrintAll(msgType, msg);
	return 0;
}

// _PlaySound - Play sound globally at position
int Lua_PlaySound(lua_State *L)
{
	const char *soundName = CLuaUtility::GetString(L, 1);
	if (!soundName || !*soundName)
		return 0;

	// Get optional position (default to world origin)
	float x = CLuaUtility::GetFloat(L, 2, 0);
	float y = CLuaUtility::GetFloat(L, 3, 0);
	float z = CLuaUtility::GetFloat(L, 4, 0);
	float volume = CLuaUtility::GetFloat(L, 5, 1.0f);
	int pitch = CLuaUtility::GetInt(L, 6, 100);

	Vector pos(x, y, z);

	LuaPrecacheSoundName(soundName);

	CPASAttenuationFilter filter(pos);
	CBaseEntity::EmitSound(filter, 0, CHAN_AUTO, soundName, volume, SNDLVL_NORM, 0, pitch, &pos);
	return 0;
}

// _PlaySoundPlayer - Play sound to specific player
int Lua_PlaySoundPlayer(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *soundName = CLuaUtility::GetString(L, 2);
	float volume = CLuaUtility::GetFloat(L, 3, 1.0f);
	int pitch = CLuaUtility::GetInt(L, 4, 100);

	if (!soundName || !*soundName)
		return 0;

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return 0;

	CSingleUserRecipientFilter filter(pPlayer);
	Vector pos = pPlayer->GetAbsOrigin();
	LuaPrecacheSoundName(soundName);
	CBaseEntity::EmitSound(filter, pPlayer->entindex(), CHAN_AUTO, soundName, volume, SNDLVL_NORM, 0, pitch, &pos);
	return 0;
}

// _ScreenText - Simple screen text message (wrapper for HudMessage)
int Lua_ScreenText(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *text = CLuaUtility::GetString(L, 2);
	float x = CLuaUtility::GetFloat(L, 3, 0.5f);
	float y = CLuaUtility::GetFloat(L, 4, 0.5f);
	float holdTime = CLuaUtility::GetFloat(L, 5, 5.0f);

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

	// Use HudMsg user message for screen text
	UserMessageBegin(filter, "HudMsg");
		WRITE_BYTE(0); // channel
		WRITE_FLOAT(x);
		WRITE_FLOAT(y);
		WRITE_BYTE(255); // r
		WRITE_BYTE(255); // g
		WRITE_BYTE(255); // b
		WRITE_BYTE(0); // effect
		WRITE_FLOAT(0.1f); // fadeIn
		WRITE_FLOAT(0.1f); // fadeOut
		WRITE_FLOAT(holdTime);
		WRITE_STRING(text);
	MessageEnd();

	return 0;
}

// _ForceFileConsistency - Force client to have matching file (anti-cheat)
// Note: ForceExactFile not available in HL2 beta engine - stub implementation
int Lua_ForceFileConsistency(lua_State *L)
{
	const char *filename = CLuaUtility::GetString(L, 1);
	if (filename && *filename)
	{
		// ForceExactFile not available in HL2 beta engine
		// This would be used for anti-cheat file consistency checking
		DevMsg("_ForceFileConsistency: %s (not implemented in HL2 beta)\n", filename);
	}
	return 0;
}

//=============================================================================
// CONSOLE/PRINT FUNCTIONS
//=============================================================================

// _Msg - Print message to console
int Lua_Msg(lua_State *L)
{
	const char *msg = CLuaUtility::GetString(L, 1);
	if (msg)
		Msg("%s", msg);
	return 0;
}

// _MsgN - Print message with newline
int Lua_MsgN(lua_State *L)
{
	const char *msg = CLuaUtility::GetString(L, 1);
	if (msg)
		Msg("%s\n", msg);
	return 0;
}

// _Warning - Print warning to console
int Lua_Warning(lua_State *L)
{
	const char *msg = CLuaUtility::GetString(L, 1);
	if (msg)
		Warning("%s", msg);
	return 0;
}

// _DevMsg - Print developer message
int Lua_DevMsg(lua_State *L)
{
	const char *msg = CLuaUtility::GetString(L, 1);
	if (msg)
		DevMsg("%s", msg);
	return 0;
}

// _ServerCommand - Execute server command
int Lua_ServerCommand(lua_State *L)
{
	const char *cmd = CLuaUtility::GetString(L, 1);
	if (cmd && *cmd)
	{
		engine->ServerCommand(UTIL_VarArgs("%s\n", cmd));
	}
	return 0;
}

// _GetConVarValue - Get convar value
int Lua_GetConVarValue(lua_State *L)
{
	const char *name = CLuaUtility::GetString(L, 1);
	if (!name || !*name)
	{
		lua_pushnil(L);
		return 1;
	}

	const ConVar *pConVar = cvar->FindVar(name);
	if (!pConVar)
	{
		lua_pushnil(L);
		return 1;
	}

	lua_pushstring(L, pConVar->GetString());
	return 1;
}

// _SetConVarValue - Set convar value
int Lua_SetConVarValue(lua_State *L)
{
	const char *name = CLuaUtility::GetString(L, 1);
	const char *value = CLuaUtility::GetString(L, 2);

	if (!name || !*name || !value)
		return 0;

	// Use server command to set convar value (safer and works around const API)
	engine->ServerCommand(UTIL_VarArgs("%s \"%s\"\n", name, value));
	return 0;
}

//=============================================================================
// REGISTRATION
//=============================================================================

void RegisterLuaFileFunctions()
{
	// File operations

	// Core utility functions (_GetCurrentMap is registered in lua_entity_funcs.cpp)
	CLuaIntegration::RegisterFunction("_GetRule", Lua_GetRule, "Get gamerule value. Syntax: <rulename>");
	CLuaIntegration::RegisterFunction("_PrintMessage", Lua_PrintMessage, "Print message to player. Syntax: <playerid> <msgtype> <msg>");
	CLuaIntegration::RegisterFunction("_PrintMessageAll", Lua_PrintMessageAll, "Print message to all. Syntax: <msgtype> <msg>");
	CLuaIntegration::RegisterFunction("_PlaySound", Lua_PlaySound, "Play sound. Syntax: <sound> [x] [y] [z] [vol] [pitch]");
	CLuaIntegration::RegisterFunction("_PlaySoundPlayer", Lua_PlaySoundPlayer, "Play sound to player. Syntax: <playerid> <sound> [vol] [pitch]");
	CLuaIntegration::RegisterFunction("_ScreenText", Lua_ScreenText, "Show screen text. Syntax: <playerid> <text> [x] [y] [holdtime]");
	CLuaIntegration::RegisterFunction("_ForceFileConsistency", Lua_ForceFileConsistency, "Force file consistency. Syntax: <filename>");

	// Console/print
	CLuaIntegration::RegisterFunction("_Msg", Lua_Msg, "Print to console. Syntax: <message>");
	CLuaIntegration::RegisterFunction("_MsgN", Lua_MsgN, "Print to console with newline. Syntax: <message>");
	CLuaIntegration::RegisterFunction("_Warning", Lua_Warning, "Print warning. Syntax: <message>");
	CLuaIntegration::RegisterFunction("_DevMsg", Lua_DevMsg, "Print developer message. Syntax: <message>");
	CLuaIntegration::RegisterFunction("_ServerCommand", Lua_ServerCommand, "Execute server command. Syntax: <cmd>");
	CLuaIntegration::RegisterFunction("_GetConVarValue", Lua_GetConVarValue, "Get convar value. Syntax: <name>");
	CLuaIntegration::RegisterFunction("_SetConVarValue", Lua_SetConVarValue, "Set convar value. Syntax: <name> <value>");
}

//-----------------------------------------------------------------------------
// Registers the file-funcs globals into the ACTIVE CLuaIntegration state, which
// RegisterEngineBindings() points at the gamemode Lua state (s_pLuaState) before
// calling this. Original gmod exposes these as plain globals in the gamemode state,
// but bmod only registered them via RegisterLuaFileFunctions() into the separate
// CLuaIntegration::Initialize state -- RegisterEngineBindings never invoked it -- so
// they were NIL for gamemodes. That made e.g. MelonRacer's AddTimer(1,4,_PlaySound,...)
// store a nil func, which errors every tick at includes/timers.lua:37 and, past
// gmod_lua_maxerrors, disables the entire Lua system (and abandons the melon's physics
// mid-lifecycle). _PrintMessage and _Msg are intentionally OMITTED here: the core
// Register*Functions own those in the gamemode state and must keep winning (parity).
//-----------------------------------------------------------------------------
void RegisterLuaFileGlobalsForGamemode()
{
	CLuaIntegration::RegisterFunction("_GetRule", Lua_GetRule, "Get gamerule value. Syntax: <rulename>");
	CLuaIntegration::RegisterFunction("_PrintMessageAll", Lua_PrintMessageAll, "Print message to all. Syntax: <msgtype> <msg>");
	CLuaIntegration::RegisterFunction("_PlaySound", Lua_PlaySound, "Play sound. Syntax: <sound> [x] [y] [z] [vol] [pitch]");
	CLuaIntegration::RegisterFunction("_PlaySoundPlayer", Lua_PlaySoundPlayer, "Play sound to player. Syntax: <playerid> <sound> [vol] [pitch]");
	CLuaIntegration::RegisterFunction("_ScreenText", Lua_ScreenText, "Show screen text. Syntax: <playerid> <text> [x] [y] [holdtime]");
	CLuaIntegration::RegisterFunction("_ForceFileConsistency", Lua_ForceFileConsistency, "Force file consistency. Syntax: <filename>");
	CLuaIntegration::RegisterFunction("_MsgN", Lua_MsgN, "Print to console with newline. Syntax: <message>");
	CLuaIntegration::RegisterFunction("_Warning", Lua_Warning, "Print warning. Syntax: <message>");
	CLuaIntegration::RegisterFunction("_DevMsg", Lua_DevMsg, "Print developer message. Syntax: <message>");
	CLuaIntegration::RegisterFunction("_ServerCommand", Lua_ServerCommand, "Execute server command. Syntax: <cmd>");
	CLuaIntegration::RegisterFunction("_GetConVarValue", Lua_GetConVarValue, "Get convar value. Syntax: <name>");
	CLuaIntegration::RegisterFunction("_SetConVarValue", Lua_SetConVarValue, "Set convar value. Syntax: <name> <value>");
}

//-----------------------------------------------------------------------------
// The original gmod 9.0.4b registers file access as a "_file" table
// (_file.Read/Write/Exists/Find/IsDir/Delete/CreateDir/Rename), NOT as _File* globals.
// bmod's _File* globals only exist in the CLuaIntegration state; gamemode scripts run in
// the CGModLuaSystem state, so this builds the _file table there to match the original.
//-----------------------------------------------------------------------------
void RegisterLuaFileTable(lua_State* L)
{
	if (!L)
		return;

	lua_newtable(L);
	lua_pushcfunction(L, Lua_FileRead);      lua_setfield(L, -2, "Read");
	lua_pushcfunction(L, Lua_FileWrite);     lua_setfield(L, -2, "Write");
	lua_pushcfunction(L, Lua_FileAppend);    lua_setfield(L, -2, "Append");
	lua_pushcfunction(L, Lua_FileExists);    lua_setfield(L, -2, "Exists");
	lua_pushcfunction(L, Lua_FileFind);      lua_setfield(L, -2, "Find");
	lua_pushcfunction(L, Lua_FileIsDir);     lua_setfield(L, -2, "IsDir");
	lua_pushcfunction(L, Lua_FileDelete);    lua_setfield(L, -2, "Delete");
	lua_pushcfunction(L, Lua_FileCreateDir); lua_setfield(L, -2, "CreateDir");
	lua_pushcfunction(L, Lua_FileSize);      lua_setfield(L, -2, "Size");
	lua_setglobal(L, "_file");
}
