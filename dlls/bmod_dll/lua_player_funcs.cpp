//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lua player functions for BarrysMod
// Based on reverse engineering of Garry's Mod 9.0.4b server.dll
//
//=============================================================================//

#include "cbase.h"
#include "lua_integration.h"
#include "player.h"
#include "team.h"
#include "hl2_player.h"
#include "ammodef.h"
#include "basecombatweapon.h"
#include "weapon_parse.h"
#include "recipientfilter.h"
#include "usermessages.h"
#include "gmod_gamemode.h"
#include "gmod_lua.h"
#include "gmod_swep.h"
#include "shareddefs.h"  // OBS_MODE constants

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// PLAYER FUNCTIONS
//=============================================================================

static void LuaPushVector(lua_State *L, const Vector& vec)
{
	lua_newtable(L);
	lua_pushnumber(L, vec.x);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, vec.y);
	lua_setfield(L, -2, "y");
	lua_pushnumber(L, vec.z);
	lua_setfield(L, -2, "z");
}

static CBaseCombatWeapon *LuaGetWeapon(lua_State *L, int index)
{
	int entIndex = CLuaUtility::GetInt(L, index);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	return dynamic_cast<CBaseCombatWeapon *>(pEntity);
}

static FileWeaponInfo_t *LuaGetMutableWeaponInfo(CBaseCombatWeapon *pWeapon)
{
	if (!pWeapon)
		return NULL;

	return const_cast<FileWeaponInfo_t *>(&pWeapon->GetWpnData());
}

static SWEPData_t *LuaGetOrCreateSWEPData(CBaseCombatWeapon *pWeapon)
{
	if (!pWeapon)
		return NULL;

	const char *pszClassName = pWeapon->GetClassname();
	if (!pszClassName || !pszClassName[0])
		pszClassName = "weapon_scripted";

	SWEPData_t *pData = CGModSWEPSystem::GetSWEPData(pszClassName);
	if (pData)
		return pData;

	SWEPData_t data;
	Q_strncpy(data.className, pszClassName, sizeof(data.className));
	data.isRegistered = true;

	int index = CGModSWEPSystem::s_SWEPRegistry.AddToTail(data);
	return &CGModSWEPSystem::s_SWEPRegistry[index];
}

static bool LuaCallGlobalBool(lua_State *L, const char *pszFunction, bool &value)
{
	if (!L || !pszFunction || !pszFunction[0])
		return false;

	int stackTop = lua_gettop(L);
	lua_getglobal(L, pszFunction);
	if (!lua_isfunction(L, -1))
	{
		lua_settop(L, stackTop);
		return false;
	}

	if (lua_pcall(L, 0, 1, 0) != 0)
	{
		CGModLuaSystem::HandleLuaError(L, pszFunction);
		lua_settop(L, stackTop);
		return false;
	}

	bool ok = lua_isboolean(L, -1) || lua_isnumber(L, -1);
	if (ok)
		value = lua_toboolean(L, -1) != 0;

	lua_settop(L, stackTop);
	return ok;
}

static bool LuaCallGlobalNumber(lua_State *L, const char *pszFunction, float &value)
{
	if (!L || !pszFunction || !pszFunction[0])
		return false;

	int stackTop = lua_gettop(L);
	lua_getglobal(L, pszFunction);
	if (!lua_isfunction(L, -1))
	{
		lua_settop(L, stackTop);
		return false;
	}

	if (lua_pcall(L, 0, 1, 0) != 0)
	{
		CGModLuaSystem::HandleLuaError(L, pszFunction);
		lua_settop(L, stackTop);
		return false;
	}

	bool ok = lua_isnumber(L, -1);
	if (ok)
		value = (float)lua_tonumber(L, -1);

	lua_settop(L, stackTop);
	return ok;
}

static bool LuaCallGlobalString(lua_State *L, const char *pszFunction, char *pOut, int outSize)
{
	if (!L || !pszFunction || !pszFunction[0] || !pOut || outSize <= 0)
		return false;

	int stackTop = lua_gettop(L);
	lua_getglobal(L, pszFunction);
	if (!lua_isfunction(L, -1))
	{
		lua_settop(L, stackTop);
		return false;
	}

	if (lua_pcall(L, 0, 1, 0) != 0)
	{
		CGModLuaSystem::HandleLuaError(L, pszFunction);
		lua_settop(L, stackTop);
		return false;
	}

	bool ok = lua_isstring(L, -1);
	if (ok)
		Q_strncpy(pOut, lua_tostring(L, -1), outSize);

	lua_settop(L, stackTop);
	return ok;
}

static bool LuaCallGlobalVector(lua_State *L, const char *pszFunction, Vector &value)
{
	if (!L || !pszFunction || !pszFunction[0])
		return false;

	int stackTop = lua_gettop(L);
	lua_getglobal(L, pszFunction);
	if (!lua_isfunction(L, -1))
	{
		lua_settop(L, stackTop);
		return false;
	}

	if (lua_pcall(L, 0, 1, 0) != 0)
	{
		CGModLuaSystem::HandleLuaError(L, pszFunction);
		lua_settop(L, stackTop);
		return false;
	}

	bool ok = lua_istable(L, -1);
	if (ok)
	{
		lua_getfield(L, -1, "x");
		value.x = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "y");
		value.y = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "z");
		value.z = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
	}

	lua_settop(L, stackTop);
	return ok;
}

static void LuaApplyWeaponModels(CBaseCombatWeapon *pWeapon, FileWeaponInfo_t *pInfo)
{
	if (!pWeapon || !pInfo)
		return;

	if (pInfo->szViewModel[0])
		pWeapon->m_iViewModelIndex = pWeapon->PrecacheModel(pInfo->szViewModel);

	if (pInfo->szWorldModel[0])
	{
		pWeapon->m_iWorldModelIndex = pWeapon->PrecacheModel(pInfo->szWorldModel);
		pWeapon->SetModel(pInfo->szWorldModel);
	}
}

static void LuaApplyWeaponAmmo(CBaseCombatWeapon *pWeapon, FileWeaponInfo_t *pInfo)
{
	if (!pWeapon || !pInfo)
		return;

	if (pInfo->szAmmo1[0])
		pWeapon->m_iPrimaryAmmoType = GetAmmoDef()->Index(pInfo->szAmmo1);
	if (pInfo->szAmmo2[0])
		pWeapon->m_iSecondaryAmmoType = GetAmmoDef()->Index(pInfo->szAmmo2);

	if (pInfo->iMaxClip1 == WEAPON_NOCLIP)
	{
		pWeapon->m_iClip1 = WEAPON_NOCLIP;
	}
	else if (pWeapon->m_iClip1 == WEAPON_NOCLIP || pWeapon->m_iClip1 < 0)
	{
		pWeapon->m_iClip1 = min(pInfo->iDefaultClip1, pInfo->iMaxClip1);
	}
	else if (pWeapon->m_iClip1 > pInfo->iMaxClip1)
	{
		pWeapon->m_iClip1 = pInfo->iMaxClip1;
	}

	if (pInfo->iMaxClip2 == WEAPON_NOCLIP)
	{
		pWeapon->m_iClip2 = WEAPON_NOCLIP;
	}
	else if (pWeapon->m_iClip2 == WEAPON_NOCLIP || pWeapon->m_iClip2 < 0)
	{
		pWeapon->m_iClip2 = min(pInfo->iDefaultClip2, pInfo->iMaxClip2);
	}
	else if (pWeapon->m_iClip2 > pInfo->iMaxClip2)
	{
		pWeapon->m_iClip2 = pInfo->iMaxClip2;
	}
}

static int LuaWeaponSoundIndex(const char *pszEventName)
{
	if (!pszEventName)
		return SINGLE;

	if (!Q_stricmp(pszEventName, "empty")) return EMPTY;
	if (!Q_stricmp(pszEventName, "single") || !Q_stricmp(pszEventName, "single_shot")) return SINGLE;
	if (!Q_stricmp(pszEventName, "single_npc")) return SINGLE_NPC;
	if (!Q_stricmp(pszEventName, "double") || !Q_stricmp(pszEventName, "double_shot")) return WPN_DOUBLE;
	if (!Q_stricmp(pszEventName, "double_npc")) return DOUBLE_NPC;
	if (!Q_stricmp(pszEventName, "burst")) return BURST;
	if (!Q_stricmp(pszEventName, "reload")) return RELOAD;
	if (!Q_stricmp(pszEventName, "reload_npc")) return RELOAD_NPC;
	if (!Q_stricmp(pszEventName, "melee_miss")) return MELEE_MISS;
	if (!Q_stricmp(pszEventName, "melee_hit")) return MELEE_HIT;
	if (!Q_stricmp(pszEventName, "melee_hit_world")) return MELEE_HIT_WORLD;
	if (!Q_stricmp(pszEventName, "special1")) return SPECIAL1;
	if (!Q_stricmp(pszEventName, "special2")) return SPECIAL2;
	if (!Q_stricmp(pszEventName, "special3")) return SPECIAL3;

	return SINGLE;
}

static void LuaUpdateSWEPVariables(lua_State *L, CBaseCombatWeapon *pWeapon)
{
	if (!L || !pWeapon)
		return;

	SWEPData_t *pData = LuaGetOrCreateSWEPData(pWeapon);
	FileWeaponInfo_t *pInfo = LuaGetMutableWeaponInfo(pWeapon);
	if (!pData || !pInfo)
		return;

	float numberValue = 0.0f;
	bool boolValue = false;
	Vector vectorValue = vec3_origin;

	LuaCallGlobalString(L, "getClassName", pData->className, sizeof(pData->className));
	LuaCallGlobalString(L, "getPrintName", pData->printName, sizeof(pData->printName));
	LuaCallGlobalString(L, "getViewModel", pData->viewModel, sizeof(pData->viewModel));
	LuaCallGlobalString(L, "getWorldModel", pData->worldModel, sizeof(pData->worldModel));
	LuaCallGlobalString(L, "getHUDMaterial", pData->hudMaterial, sizeof(pData->hudMaterial));
	LuaCallGlobalString(L, "getDeathIcon", pData->deathIcon, sizeof(pData->deathIcon));
	LuaCallGlobalString(L, "getAnimPrefix", pData->animPrefix, sizeof(pData->animPrefix));
	LuaCallGlobalString(L, "getPrimaryAmmoType", pData->primaryAmmoType, sizeof(pData->primaryAmmoType));
	LuaCallGlobalString(L, "getSecondaryAmmoType", pData->secondaryAmmoType, sizeof(pData->secondaryAmmoType));

	if (LuaCallGlobalNumber(L, "getDamage", numberValue)) pData->damage = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getDamageSecondary", numberValue)) pData->damageSecondary = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getPrimaryShotDelay", numberValue)) pData->primaryShotDelay = numberValue;
	if (LuaCallGlobalNumber(L, "getSecondaryShotDelay", numberValue)) pData->secondaryShotDelay = numberValue;
	if (LuaCallGlobalNumber(L, "getNumShotsPrimary", numberValue)) pData->numShotsPrimary = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getNumShotsSecondary", numberValue)) pData->numShotsSecondary = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getMaxClipPrimary", numberValue)) pData->maxClipPrimary = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getMaxClipSecondary", numberValue)) pData->maxClipSecondary = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getDefClipPrimary", numberValue)) pData->defClipPrimary = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getDefClipSecondary", numberValue)) pData->defClipSecondary = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getTracerFreqPrimary", numberValue)) pData->tracerFreqPrimary = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getTracerFreqSecondary", numberValue)) pData->tracerFreqSecondary = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getPrimaryScriptOverride", numberValue)) pData->primaryScriptOverride = (SWEPScriptOverride_t)(int)numberValue;
	if (LuaCallGlobalNumber(L, "getSecondaryScriptOverride", numberValue)) pData->secondaryScriptOverride = (SWEPScriptOverride_t)(int)numberValue;
	if (LuaCallGlobalNumber(L, "getWeaponFOV", numberValue)) pData->weaponFOV = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getWeaponSlot", numberValue)) pData->weaponSlot = (int)numberValue;
	if (LuaCallGlobalNumber(L, "getWeaponSlotPos", numberValue)) pData->weaponSlotPos = (int)numberValue;

	if (LuaCallGlobalBool(L, "getPrimaryIsAutomatic", boolValue)) pData->primaryIsAutomatic = boolValue;
	if (LuaCallGlobalBool(L, "getSecondaryIsAutomatic", boolValue)) pData->secondaryIsAutomatic = boolValue;
	if (LuaCallGlobalBool(L, "getWeaponSwapHands", boolValue)) pData->weaponSwapHands = boolValue;
	if (LuaCallGlobalBool(L, "getFiresUnderwater", boolValue)) pData->firesUnderwater = boolValue;
	if (LuaCallGlobalBool(L, "getReloadsSingly", boolValue)) pData->reloadsSingly = boolValue;

	if (LuaCallGlobalVector(L, "getBulletSpread", vectorValue)) pData->bulletSpread = vectorValue;
	if (LuaCallGlobalVector(L, "getBulletSpreadSecondary", vectorValue)) pData->bulletSpreadSecondary = vectorValue;
	if (LuaCallGlobalVector(L, "getViewKick", vectorValue)) pData->viewKick = vectorValue;
	if (LuaCallGlobalVector(L, "getViewKickSecondary", vectorValue)) pData->viewKickSecondary = vectorValue;
	if (LuaCallGlobalVector(L, "getViewKickRandom", vectorValue)) pData->viewKickRandom = vectorValue;
	if (LuaCallGlobalVector(L, "getViewKickRandomSecondary", vectorValue)) pData->viewKickRandomSecondary = vectorValue;

	Q_strncpy(pInfo->szPrintName, pData->printName, sizeof(pInfo->szPrintName));
	Q_strncpy(pInfo->szViewModel, pData->viewModel, sizeof(pInfo->szViewModel));
	Q_strncpy(pInfo->szWorldModel, pData->worldModel, sizeof(pInfo->szWorldModel));
	Q_strncpy(pInfo->szAnimationPrefix, pData->animPrefix, sizeof(pInfo->szAnimationPrefix));
	Q_strncpy(pInfo->szAmmo1, pData->primaryAmmoType, sizeof(pInfo->szAmmo1));
	Q_strncpy(pInfo->szAmmo2, pData->secondaryAmmoType, sizeof(pInfo->szAmmo2));
	pInfo->iSlot = pData->weaponSlot;
	pInfo->iPosition = pData->weaponSlotPos;
	pInfo->iMaxClip1 = pData->maxClipPrimary;
	pInfo->iMaxClip2 = pData->maxClipSecondary;
	pInfo->iDefaultClip1 = pData->defClipPrimary;
	pInfo->iDefaultClip2 = pData->defClipSecondary;

	pWeapon->m_bFiresUnderwater = pData->firesUnderwater;
	pWeapon->m_bReloadsSingly = pData->reloadsSingly;

	LuaApplyWeaponModels(pWeapon, pInfo);
	LuaApplyWeaponAmmo(pWeapon, pInfo);
}

static int LuaGetGlobalInt(lua_State *L, const char *name, int defaultValue)
{
	int stackTop = lua_gettop(L);
	lua_getglobal(L, name);
	int value = lua_isnumber(L, -1) ? (int)lua_tonumber(L, -1) : defaultValue;
	lua_settop(L, stackTop);
	return value;
}

static int LuaObserverModeToSource(lua_State *L, int mode)
{
	if (mode == 0)
		return OBS_MODE_NONE;
	if (mode == 1)
		return OBS_MODE_FIXED;
	if (mode == 2)
		return OBS_MODE_FIXED;
	if (mode == 3)
		return OBS_MODE_IN_EYE;
	if (mode == 4)
		return OBS_MODE_CHASE;
	if (mode == 5)
		return OBS_MODE_ROAMING;

	if (mode == LuaGetGlobalInt(L, "OBS_MODE_FIXED", OBS_MODE_FIXED))
		return OBS_MODE_FIXED;
	if (mode == LuaGetGlobalInt(L, "OBS_MODE_IN_EYE", OBS_MODE_IN_EYE))
		return OBS_MODE_IN_EYE;
	if (mode == LuaGetGlobalInt(L, "OBS_MODE_CHASE", OBS_MODE_CHASE))
		return OBS_MODE_CHASE;
	if (mode == LuaGetGlobalInt(L, "OBS_MODE_ROAMING", OBS_MODE_ROAMING))
		return OBS_MODE_ROAMING;
	return mode;
}

static const char *LuaPlayerName(CBasePlayer *pPlayer)
{
	const char *name = pPlayer ? STRING(pPlayer->pl.netname) : "";
	return name ? name : "";
}

static int SourceTeamToGModTeam( int iTeam )
{
	switch ( iTeam )
	{
	case TEAM_UNASSIGNED:
		return 0;
	case TEAM_SPECTATOR:
		return 1;
	default:
		return ( iTeam > TEAM_SPECTATOR ) ? iTeam - 1 : iTeam;
	}
}

static int GModTeamToSourceTeam( int iTeam )
{
	switch ( iTeam )
	{
	case 0:
		return TEAM_UNASSIGNED;
	case 1:
		return TEAM_SPECTATOR;
	default:
		return iTeam + 1;
	}
}

static void LuaPushPlayerInfoField(lua_State *L, CBasePlayer *pPlayer, const char *field)
{
	if (!field)
	{
		lua_pushnil(L);
		return;
	}

	if (!pPlayer)
	{
		if (!Q_stricmp(field, "connected"))
			lua_pushboolean(L, false);
		else
			lua_pushnil(L);
		return;
	}

	if (!Q_stricmp(field, "connected"))
		lua_pushboolean(L, true);
	else if (!Q_stricmp(field, "name"))
		lua_pushstring(L, LuaPlayerName(pPlayer));
	else if (!Q_stricmp(field, "userid"))
		lua_pushnumber(L, engine->GetPlayerUserId(pPlayer->edict()));
	else if (!Q_stricmp(field, "ping"))
		lua_pushnumber(L, 0);
	else if (!Q_stricmp(field, "packetloss"))
		lua_pushnumber(L, 0);
	else if (!Q_stricmp(field, "kills"))
		lua_pushnumber(L, pPlayer->FragCount());
	else if (!Q_stricmp(field, "deaths"))
		lua_pushnumber(L, pPlayer->DeathCount());
	else if (!Q_stricmp(field, "team"))
		lua_pushnumber(L, SourceTeamToGModTeam( pPlayer->GetTeamNumber() ));
	else if (!Q_stricmp(field, "alive"))
		lua_pushboolean(L, pPlayer->IsAlive());
	else if (!Q_stricmp(field, "health"))
		lua_pushnumber(L, pPlayer->GetHealth());
	else if (!Q_stricmp(field, "armor"))
		lua_pushnumber(L, pPlayer->ArmorValue());
	else if (!Q_stricmp(field, "model"))
		lua_pushstring(L, STRING(pPlayer->GetModelName()));
	else if (!Q_stricmp(field, "networkid"))
		lua_pushstring(L, "UNKNOWN");
	else if (!Q_stricmp(field, "entindex"))
		lua_pushnumber(L, pPlayer->entindex());
	else if (!Q_stricmp(field, "weapon"))
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
		lua_pushstring(L, pWeapon ? pWeapon->GetClassname() : "none");
	}
	else
	{
		lua_pushstring(L, "<Not Found>");
	}
}

// _PlayerFreeze - Freeze/unfreeze player
int Lua_PlayerFreeze(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	bool freeze = CLuaUtility::GetBool(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return 0;

	if (freeze)
		pPlayer->AddFlag(FL_FROZEN);
	else
		pPlayer->RemoveFlag(FL_FROZEN);
	return 0;
}

// _PlayerSetSprint - Enable/disable sprint
int Lua_PlayerSetSprint_New(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	bool canSprint = CLuaUtility::GetBool(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetSprint: Invalid player ID");

	// HL2 specific sprint control - use StartSprinting/StopSprinting
	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);
	if (pHL2Player)
	{
		if (canSprint)
			pHL2Player->StartSprinting();
		else
			pHL2Player->StopSprinting();
	}
	return 0;
}

// _PlayerGetShootPos - Get player eye position
int Lua_PlayerGetShootPos(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerGetShootPos: Invalid player ID");

	Vector pos = pPlayer->EyePosition();
	LuaPushVector(L, pos);
	return 1;
}

// _PlayerGetShootAng - Get player shoot forward vector
int Lua_PlayerGetShootAng(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerGetShootAng: Invalid player ID");

	QAngle ang = pPlayer->EyeAngles();
	Vector forward;
	AngleVectors(ang, &forward);
	LuaPushVector(L, forward);
	return 1;
}

// _PlayerGetActiveWeapon - Get player's active weapon
int Lua_PlayerGetActiveWeapon(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerGetActiveWeapon: Invalid player ID");

	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if (pWeapon)
		lua_pushinteger(L, pWeapon->entindex());
	else
		lua_pushnil(L);
	return 1;
}

// _PlayerKill - Kill player
int Lua_PlayerKill(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerKill: Invalid player ID");

	pPlayer->CommitSuicide();
	return 0;
}

// _PlayerRespawn - Respawn player
int Lua_PlayerRespawn(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerRespawn: Invalid player ID");

	pPlayer->Spawn();
	return 0;
}

// _PlayerSetHealth - Set player health
int Lua_PlayerSetHealth(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int health = CLuaUtility::GetInt(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetHealth: Invalid player ID");

	pPlayer->SetHealth(health);
	return 0;
}

// _PlayerSetArmor - Set player armor
int Lua_PlayerSetArmor(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int armor = CLuaUtility::GetInt(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetArmor: Invalid player ID");

	pPlayer->SetArmorValue(armor);
	return 0;
}

// _PlayerSetMaxSpeed - Set player max speed
int Lua_PlayerSetMaxSpeed(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	float maxSpeed = CLuaUtility::GetFloat(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetMaxSpeed: Invalid player ID");

	pPlayer->SetMaxSpeed(maxSpeed);
	return 0;
}

// _PlayerSetModel - Set player model
int Lua_PlayerSetModel_New(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *model = CLuaUtility::GetString(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetModel: Invalid player ID");

	// GMod 9 scripts request models that may not be present in this content set.
	// Degrade to the known-good default rather than rendering an error/invisible
	// player.
	const char *pszModel = BMod_ResolvePlayerModel( model );

	// The model has to be in the precache list before SetModel can use it.
	pPlayer->PrecacheModel( pszModel );

	pPlayer->SetModel( pszModel );
	return 0;
}

// _PlayerGiveAmmo - Give ammo to player
int Lua_PlayerGiveAmmo_New(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int amount = CLuaUtility::GetInt(L, 2);
	const char *ammoType = CLuaUtility::GetString(L, 3);
	bool playSounds = CLuaUtility::GetBool(L, 4, true);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerGiveAmmo: Invalid player ID");

	int ammoIndex = GetAmmoDef()->Index(ammoType);
	if (ammoIndex == -1)
	{
		// Do NOT raise a Lua error here: GMod 9 loadout scripts give many ammo
		// types in a row, and an error aborts the whole GiveDefaultItems function
		// (players then never receive the physgun/tool gun that come after the
		// ammo lines). Unknown type = warn and give nothing, like retail.
		Warning("_PlayerGiveAmmo: unknown ammo type '%s' (player %d)\n", ammoType ? ammoType : "", playerID);
		lua_pushinteger(L, 0);
		return 1;
	}

	int given = pPlayer->GiveAmmo(amount, ammoIndex, true);
	lua_pushinteger(L, given);
	return 1;
}

// _PlayerGiveItem - Give weapon to player
int Lua_PlayerGiveItem(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *weaponClass = CLuaUtility::GetString(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerGiveItem: Invalid player ID");

	CBaseEntity *pWeapon = pPlayer->GiveNamedItem(weaponClass);
	if (pWeapon)
		lua_pushinteger(L, pWeapon->entindex());
	else
		lua_pushnil(L);
	return 1;
}

// _PlayerRemoveAllWeapons - Remove all weapons from player
int Lua_PlayerRemoveAllWeapons(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerRemoveAllWeapons: Invalid player ID");

	pPlayer->RemoveAllItems(false);
	return 0;
}

// _PlayerRemoveWeapon - Remove specific weapon from player
int Lua_PlayerRemoveWeapon(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *weaponClass = CLuaUtility::GetString(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerRemoveWeapon: Invalid player ID");

	CBaseCombatWeapon *pWeapon = pPlayer->Weapon_OwnsThisType(weaponClass);
	if (pWeapon)
	{
		pPlayer->Weapon_Drop(pWeapon, NULL, NULL);
		UTIL_Remove(pWeapon);
	}
	return 0;
}

// _PlayerChangeTeam - Change player team
int Lua_PlayerChangeTeam(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int teamNum = GModTeamToSourceTeam( CLuaUtility::GetInt(L, 2) );

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerChangeTeam: Invalid player ID");

	CGModGamemodeSystem::ChangePlayerTeam(pPlayer, teamNum);
	return 0;
}

// _PlayerAddScore - Add to player score
int Lua_PlayerAddScore(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int score = CLuaUtility::GetInt(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerAddScore: Invalid player ID");

	pPlayer->IncrementFragCount(score);
	return 0;
}

// _PlayerSetScore - Set player score
int Lua_PlayerSetScore(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int score = CLuaUtility::GetInt(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetScore: Invalid player ID");

	// HL2 beta doesn't have SetFragCount, use Reset + Increment
	pPlayer->ResetFragCount();
	pPlayer->IncrementFragCount(score);
	return 0;
}

// _PlayerAddDeath - Add to player death count
int Lua_PlayerAddDeath(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int deaths = CLuaUtility::GetInt(L, 2, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerAddDeath: Invalid player ID");

	pPlayer->IncrementDeathCount(deaths);
	return 0;
}

// _PlayerInfo - Get player info
int Lua_PlayerInfo(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *field = lua_gettop(L) >= 2 ? CLuaUtility::GetString(L, 2, NULL) : NULL;
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);

	if (field)
	{
		LuaPushPlayerInfoField(L, pPlayer, field);
		return 1;
	}

	if (!pPlayer)
	{
		lua_pushnil(L);
		return 1;
	}

	// Compatibility form for newer callers that ask for the whole table.
	lua_newtable(L);

	lua_pushstring(L, "name");
	lua_pushstring(L, LuaPlayerName(pPlayer));
	lua_settable(L, -3);

	lua_pushstring(L, "health");
	lua_pushnumber(L, (lua_Number)pPlayer->GetHealth());
	lua_settable(L, -3);

	lua_pushstring(L, "armor");
	lua_pushnumber(L, (lua_Number)pPlayer->ArmorValue());
	lua_settable(L, -3);

	lua_pushstring(L, "team");
	lua_pushnumber(L, (lua_Number)SourceTeamToGModTeam( pPlayer->GetTeamNumber() ));
	lua_settable(L, -3);

	lua_pushstring(L, "alive");
	lua_pushboolean(L, pPlayer->IsAlive());
	lua_settable(L, -3);

	lua_pushstring(L, "connected");
	lua_pushboolean(L, true);
	lua_settable(L, -3);

	lua_pushstring(L, "kills");
	lua_pushnumber(L, (lua_Number)pPlayer->FragCount());
	lua_settable(L, -3);

	lua_pushstring(L, "deaths");
	lua_pushnumber(L, (lua_Number)pPlayer->DeathCount());
	lua_settable(L, -3);

	return 1;
}

//=============================================================================
// TEAM FUNCTIONS
//=============================================================================

// _TeamAddScore - Add to team score
int Lua_TeamAddScore(lua_State *L)
{
	int teamNum = GModTeamToSourceTeam( CLuaUtility::GetInt(L, 1) );
	int score = CLuaUtility::GetInt(L, 2);

	// Use CGModGamemodeSystem which has proper team support
	int currentScore = CGModGamemodeSystem::GetTeamScore(teamNum);
	CGModGamemodeSystem::SetTeamScore(teamNum, currentScore + score);
	return 0;
}

// _TeamSetScore - Set team score
int Lua_TeamSetScore(lua_State *L)
{
	int teamNum = GModTeamToSourceTeam( CLuaUtility::GetInt(L, 1) );
	int score = CLuaUtility::GetInt(L, 2);

	// Use CGModGamemodeSystem which has proper team support
	CGModGamemodeSystem::SetTeamScore(teamNum, score);
	return 0;
}

// _TeamNumPlayers - Get number of players on team
int Lua_TeamNumPlayers(lua_State *L)
{
	int teamNum = GModTeamToSourceTeam( CLuaUtility::GetInt(L, 1) );

	CTeam *pTeam = GetGlobalTeam(teamNum);
	if (!pTeam)
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	lua_pushinteger(L, pTeam->GetNumPlayers());
	return 1;
}

// _TeamScore - Get team score
int Lua_TeamScore(lua_State *L)
{
	int teamNum = GModTeamToSourceTeam( CLuaUtility::GetInt(L, 1) );

	// Use CGModGamemodeSystem which has proper team support
	int score = CGModGamemodeSystem::GetTeamScore(teamNum);
	lua_pushinteger(L, score);
	return 1;
}

// _TeamCount - Get number of teams
int Lua_TeamCount(lua_State *L)
{
	lua_pushinteger(L, GetNumberOfTeams());
	return 1;
}

// _MaxPlayers - Get max players
int Lua_MaxPlayers(lua_State *L)
{
	lua_pushinteger(L, gpGlobals->maxClients);
	return 1;
}

// _CurTime - Get current server time
int Lua_CurTime(lua_State *L)
{
	lua_pushnumber(L, gpGlobals->curtime);
	return 1;
}

//=============================================================================
// ADDITIONAL PLAYER FUNCTIONS (GMod compatibility)
//=============================================================================

// _PlayerSetFOV - Set player field of view
int Lua_PlayerSetFOV(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int fov = CLuaUtility::GetInt(L, 2, 90);
	float rate = CLuaUtility::GetFloat(L, 3, 0.0f);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetFOV: Invalid player ID");

	pPlayer->SetFOV(fov, rate);
	return 0;
}

// _PlayerSelectWeapon - Select weapon by classname
int Lua_PlayerSelectWeapon(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *weaponClass = CLuaUtility::GetString(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSelectWeapon: Invalid player ID");

	CBaseCombatWeapon *pWeapon = pPlayer->Weapon_OwnsThisType(weaponClass);
	if (pWeapon)
	{
		pPlayer->Weapon_Switch(pWeapon);
		lua_pushboolean(L, true);
	}
	else
	{
		lua_pushboolean(L, false);
	}
	return 1;
}

// _PlayerHolsterWeapon - Holster current weapon
int Lua_PlayerHolsterWeapon(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerHolsterWeapon: Invalid player ID");

	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if (pWeapon)
	{
		pWeapon->Holster();
	}
	return 0;
}

// _PlayerRemoveAllAmmo - Remove all ammo from player
int Lua_PlayerRemoveAllAmmo(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerRemoveAllAmmo: Invalid player ID");

	// Remove all ammo types
	for (int i = 0; i < MAX_AMMO_SLOTS; i++)
	{
		pPlayer->SetAmmoCount(0, i);
	}
	return 0;
}

// _PlayerSilentKill - Kill player without announcements
int Lua_PlayerSilentKill(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSilentKill: Invalid player ID");

	// Kill without incrementing death count
	CTakeDamageInfo info;
	info.SetDamage(pPlayer->GetHealth() + 100);
	info.SetDamageType(DMG_GENERIC);
	info.SetAttacker(pPlayer);
	info.SetInflictor(pPlayer);

	pPlayer->TakeDamage(info);
	return 0;
}

// _PlayerSpectatorStart - Start spectating
// GMod syntax: _PlayerSpectatorStart(playerid, obs_mode)
// Accepts either current Source observer constants or beta Lua constants.
int Lua_PlayerSpectatorStart(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int obsMode = LuaObserverModeToSource(L, CLuaUtility::GetInt(L, 2, OBS_MODE_CHASE)); // Default to chase cam

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSpectatorStart: Invalid player ID");

	static int s_nLoggedSpectatorStart = 0;
	if (s_nLoggedSpectatorStart < 8)
	{
		DevMsg("GMod Lua: _PlayerSpectatorStart player %d mode %d\n", playerID, obsMode);
		++s_nLoggedSpectatorStart;
	}

	// Put player into observer mode. StartObserverMode resets non-player target
	// allowance, so do not call it again during MelonRacer round respawns while
	// the player is already chasing a prop.
	if ( !pPlayer->IsObserver() )
	{
		pPlayer->StartObserverMode(pPlayer->GetAbsOrigin(), pPlayer->GetAbsAngles());
	}

	// Set the specific observer mode
	pPlayer->SetObserverMode(obsMode);

	return 0;
}

// _PlayerSpectatorTarget - Set spectator target (can target ANY entity, not just players)
// GMod syntax: _PlayerSpectatorTarget(playerid, entityid)
// This is critical for MelonRacer where players spectate their watermelon prop
int Lua_PlayerSpectatorTarget(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int targetID = CLuaUtility::GetInt(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSpectatorTarget: Invalid player ID");

	if (targetID <= 0)
	{
		pPlayer->m_hObserverTarget.Set(NULL);
		pPlayer->m_bAllowNonPlayerObserverTarget = false;
		return 0;
	}

	// Get target as ANY entity (not just player) - required for MelonRacer watermelon spectating
	CBaseEntity *pTarget = UTIL_EntityByIndex(targetID);
	if (pTarget)
	{
		DevMsg("GMod DBG: Lua_PlayerSpectatorTarget EXT begin player %d target %d (%s) obs=%d allow_nonplayer=%d\n",
			playerID, targetID, pTarget->GetClassname(), pPlayer->m_iObserverMode,
			pPlayer->m_bAllowNonPlayerObserverTarget ? 1 : 0);

		// ORDER MATTERS. The camera only chases the melon in OBS_MODE_CHASE; OBS_MODE_ROAMING/FIXED
		// copy the player's OWN EyePosition (its buried, at-spawn origin) -- that is why the chase-cam
		// sat in the ground and never followed the prop. SetObserverMode(CHASE) -> CheckObserverSettings
		// validates m_hObserverTarget and, if it is not yet valid, FORCES the player to ROAMING. And
		// StartObserverMode() resets m_bAllowNonPlayerObserverTarget to false, which invalidates a melon
		// (non-player) target. So we must: (1) only StartObserverMode if not already observing (else it
		// wipes the allowance), (2) set the allowed target FIRST, (3) THEN switch to CHASE -- now the
		// target is valid so CHASE sticks instead of collapsing to ROAMING.
		if ( !pPlayer->IsObserver() )
		{
			DevMsg("GMod DBG: Lua_PlayerSpectatorTarget EXT StartObserverMode player %d\n", playerID);
			pPlayer->StartObserverMode(pPlayer->GetAbsOrigin(), pPlayer->GetAbsAngles());
			DevMsg("GMod DBG: Lua_PlayerSpectatorTarget EXT StartObserverMode done player %d obs=%d\n",
				playerID, pPlayer->m_iObserverMode);
		}

		pPlayer->m_bAllowNonPlayerObserverTarget = !pTarget->IsPlayer();
		DevMsg("GMod DBG: Lua_PlayerSpectatorTarget EXT SetObserverTarget player %d target %d\n",
			playerID, targetID);
		if (!pPlayer->SetObserverTarget(pTarget))
		{
			pPlayer->m_bAllowNonPlayerObserverTarget = !pTarget->IsPlayer();
			pPlayer->m_hObserverTarget.Set(pTarget);
		}
		DevMsg("GMod DBG: Lua_PlayerSpectatorTarget EXT SetObserverTarget done player %d target %d\n",
			playerID, targetID);

		if (pPlayer->m_iObserverMode != OBS_MODE_CHASE && pPlayer->m_iObserverMode != OBS_MODE_IN_EYE)
		{
			DevMsg("GMod DBG: Lua_PlayerSpectatorTarget EXT SetObserverMode CHASE player %d\n", playerID);
			pPlayer->SetObserverMode(OBS_MODE_CHASE);
			DevMsg("GMod DBG: Lua_PlayerSpectatorTarget EXT SetObserverMode done player %d obs=%d\n",
				playerID, pPlayer->m_iObserverMode);
		}

		static int s_nLoggedSpectatorTarget = 0;
		if (s_nLoggedSpectatorTarget < 8)
		{
			DevMsg("GMod Lua EXT: _PlayerSpectatorTarget player %d target %d (%s)\n",
				playerID, targetID, pTarget->GetClassname());
			++s_nLoggedSpectatorTarget;
		}
	}
	return 0;
}

// _PlayerSetChaseCamDistance - Set chase cam distance for spectator mode
int Lua_PlayerSetChaseCamDistance(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	float distance = CLuaUtility::GetFloat(L, 2, 96.0f);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetChaseCamDistance: Invalid player ID");

	// Clamp distance to reasonable values
	if (distance < 10.0f) distance = 10.0f;
	if (distance > 500.0f) distance = 500.0f;

	pPlayer->m_flChaseCamDistance = distance;
	return 0;
}

// _PlayerStopZooming - Stop any zoom effect
int Lua_PlayerStopZooming(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerStopZooming: Invalid player ID");

	// Reset FOV to default
	pPlayer->SetFOV(0, 0.1f);
	return 0;
}

// _PlayerAllowDecalPaint - Allow player to paint decals
int Lua_PlayerAllowDecalPaint(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerAllowDecalPaint: Invalid player ID");

	// Allow immediate decal painting (reset cooldown)
	// Implementation depends on paint system
	return 0;
}

// _PlayerIsKeyDown - Check if player has key pressed
int Lua_PlayerIsKeyDown(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int key = CLuaUtility::GetInt(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	// Source observer/movement code can clear m_nButtons after processing.
	// GMod Lua wants the raw command button state for this frame.
	int rawButtons = CGModLuaSystem::GetPlayerRawButtons(pPlayer);
	bool isDown = ((rawButtons | pPlayer->m_nButtons) & key) != 0;
	lua_pushboolean(L, isDown);
	return 1;
}

// _PlayerShowScoreboard - Show scoreboard to player
int Lua_PlayerShowScoreboard(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	bool show = CLuaUtility::GetBool(L, 2, true);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerShowScoreboard: Invalid player ID");

	// Send user message to show/hide scoreboard
	CSingleUserRecipientFilter filter(pPlayer);
	filter.MakeReliable();

	UserMessageBegin(filter, "GModScoreboard");
		WRITE_BYTE(show ? 1 : 0);
	MessageEnd();

	return 0;
}

// _TeamSetName - Set team name
int Lua_TeamSetName(lua_State *L)
{
	int teamNum = GModTeamToSourceTeam( CLuaUtility::GetInt(L, 1) );
	const char *name = CLuaUtility::GetString(L, 2);

	// Use CGModGamemodeSystem which has proper team name support
	CGModGamemodeSystem::SetTeamName(teamNum, name);
	return 0;
}

// _PlayerGiveSWEP - Give scripted weapon to player (stub - needs SWEP system)
int Lua_PlayerGiveSWEP(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *swepClass = CLuaUtility::GetString(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerGiveSWEP: Invalid player ID");

	if (!swepClass || !*swepClass)
		return CLuaUtility::LuaError(L, "_PlayerGiveSWEP: Invalid SWEP class");

	// Try to give as regular weapon first
	CBaseEntity *pWeapon = pPlayer->GiveNamedItem(swepClass);
	if (pWeapon)
		lua_pushinteger(L, pWeapon->entindex());
	else
		lua_pushnil(L);
	return 1;
}

// _SpawnSWEP - Spawn scripted weapon in world (stub - needs SWEP system)
int Lua_SpawnSWEP(lua_State *L)
{
	const char *swepClass = CLuaUtility::GetString(L, 1);
	float x = CLuaUtility::GetFloat(L, 2);
	float y = CLuaUtility::GetFloat(L, 3);
	float z = CLuaUtility::GetFloat(L, 4);

	if (!swepClass || !*swepClass)
		return CLuaUtility::LuaError(L, "_SpawnSWEP: Invalid SWEP class");

	// Try to create as regular entity
	CBaseEntity *pEntity = CreateEntityByName(swepClass);
	if (pEntity)
	{
		pEntity->SetAbsOrigin(Vector(x, y, z));
		DispatchSpawn(pEntity);
		lua_pushinteger(L, pEntity->entindex());
	}
	else
	{
		lua_pushnil(L);
	}
	return 1;
}

// _SWEPUpdateVariables - refresh runtime SWEP data from Lua getter functions
int Lua_SWEPUpdateVariables(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	if (!pWeapon)
		return CLuaUtility::LuaError(L, "_SWEPUpdateVariables: Invalid weapon ID");

	LuaUpdateSWEPVariables(L, pWeapon);
	return 0;
}

// _SWEPUseAmmo - Use ammo from SWEP
int Lua_SWEPUseAmmo(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	int ammoSlot = CLuaUtility::GetInt(L, 2, 0);
	int amount = CLuaUtility::GetInt(L, 3, 1);

	if (!pWeapon)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(CLuaUtility::GetInt(L, 1));
		if (pPlayer)
			pWeapon = pPlayer->GetActiveWeapon();
	}

	if (!pWeapon)
		return CLuaUtility::LuaError(L, "_SWEPUseAmmo: Invalid weapon ID");

	if (amount <= 0)
		return 0;

	CBaseCombatCharacter *pOwnerCharacter = pWeapon->GetOwner();
	CBasePlayer *pPlayer = pOwnerCharacter ? ToBasePlayer(pOwnerCharacter) : NULL;

	if (ammoSlot == 1)
	{
		if (pWeapon->m_iClip2 != WEAPON_NOCLIP)
			pWeapon->m_iClip2 = max(0, pWeapon->m_iClip2 - amount);
		else if (pPlayer && pWeapon->GetSecondaryAmmoType() >= 0)
			pPlayer->RemoveAmmo(amount, pWeapon->GetSecondaryAmmoType());
	}
	else
	{
		if (pWeapon->m_iClip1 != WEAPON_NOCLIP)
			pWeapon->m_iClip1 = max(0, pWeapon->m_iClip1 - amount);
		else if (pPlayer && pWeapon->GetPrimaryAmmoType() >= 0)
			pPlayer->RemoveAmmo(amount, pWeapon->GetPrimaryAmmoType());
	}

	return 0;
}

// _SWEPRunString - execute Lua in the current SWEP context
int Lua_SWEPRunString(lua_State *L)
{
	const char *pszCode = CLuaUtility::GetString(L, lua_gettop(L) >= 2 ? 2 : 1, NULL);
	if (!pszCode || !pszCode[0])
		return 0;

	CGModLuaSystem::ExecuteString(pszCode);
	return 0;
}

// _swep.GetClipAmmo / __swep_GetClipAmmo
int Lua_swep_GetClipAmmo(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	int ammoSlot = CLuaUtility::GetInt(L, 2, 0);

	if (!pWeapon)
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	int clip = (ammoSlot == 1) ? (int)pWeapon->m_iClip2 : (int)pWeapon->m_iClip1;
	lua_pushinteger(L, clip);
	return 1;
}

// _swep.SetClipAmmo / __swep_SetClipAmmo
int Lua_swep_SetClipAmmo(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	int ammoSlot = CLuaUtility::GetInt(L, 2, 0);
	int amount = CLuaUtility::GetInt(L, 3, 0);

	if (!pWeapon)
		return CLuaUtility::LuaError(L, "_swep.SetClipAmmo: Invalid weapon ID");

	if (ammoSlot == 1)
		pWeapon->m_iClip2 = amount;
	else
		pWeapon->m_iClip1 = amount;

	return 0;
}

// _swep.GetDeathIcon / __swep_GetDeathIcon
int Lua_swep_GetDeathIcon(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	if (!pWeapon)
	{
		lua_pushnil(L);
		return 1;
	}

	SWEPData_t *pData = LuaGetOrCreateSWEPData(pWeapon);
	if (pData && pData->deathIcon[0])
	{
		lua_pushstring(L, pData->deathIcon);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Lua_WeaponSetModel(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	const char *pszViewModel = CLuaUtility::GetString(L, 2, "");
	const char *pszWorldModel = CLuaUtility::GetString(L, 3, pszViewModel);

	if (!pWeapon)
		return CLuaUtility::LuaError(L, "_WeaponSetModel: Invalid weapon ID");

	FileWeaponInfo_t *pInfo = LuaGetMutableWeaponInfo(pWeapon);
	SWEPData_t *pData = LuaGetOrCreateSWEPData(pWeapon);
	if (!pInfo || !pData)
		return 0;

	if (pszViewModel && pszViewModel[0])
	{
		Q_strncpy(pInfo->szViewModel, pszViewModel, sizeof(pInfo->szViewModel));
		Q_strncpy(pData->viewModel, pszViewModel, sizeof(pData->viewModel));
	}
	if (pszWorldModel && pszWorldModel[0])
	{
		Q_strncpy(pInfo->szWorldModel, pszWorldModel, sizeof(pInfo->szWorldModel));
		Q_strncpy(pData->worldModel, pszWorldModel, sizeof(pData->worldModel));
	}

	LuaApplyWeaponModels(pWeapon, pInfo);
	return 0;
}

int Lua_WeaponSetSlot(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	int slot = CLuaUtility::GetInt(L, 2, 0);
	int slotPos = CLuaUtility::GetInt(L, 3, 0);

	if (!pWeapon)
		return CLuaUtility::LuaError(L, "_WeaponSetSlot: Invalid weapon ID");

	FileWeaponInfo_t *pInfo = LuaGetMutableWeaponInfo(pWeapon);
	SWEPData_t *pData = LuaGetOrCreateSWEPData(pWeapon);
	if (pInfo)
	{
		pInfo->iSlot = slot;
		pInfo->iPosition = slotPos;
	}
	if (pData)
	{
		pData->weaponSlot = slot;
		pData->weaponSlotPos = slotPos;
	}

	return 0;
}

int Lua_WeaponSetSound(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	const char *pszEventName = CLuaUtility::GetString(L, 2, "single_shot");
	const char *pszSoundName = CLuaUtility::GetString(L, 3, "");

	if (!pWeapon)
		return CLuaUtility::LuaError(L, "_WeaponSetSound: Invalid weapon ID");
	if (!pszSoundName || !pszSoundName[0])
		return 0;

	FileWeaponInfo_t *pInfo = LuaGetMutableWeaponInfo(pWeapon);
	if (!pInfo)
		return 0;

	int soundIndex = LuaWeaponSoundIndex(pszEventName);
	if (soundIndex >= 0 && soundIndex < NUM_SHOOT_SOUND_TYPES)
		Q_strncpy(pInfo->aShootSounds[soundIndex], pszSoundName, sizeof(pInfo->aShootSounds[soundIndex]));

	return 0;
}

int Lua_WeaponSetFOV(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	int fov = CLuaUtility::GetInt(L, 2, 70);

	if (!pWeapon)
		return CLuaUtility::LuaError(L, "_WeaponSetFOV: Invalid weapon ID");

	SWEPData_t *pData = LuaGetOrCreateSWEPData(pWeapon);
	if (pData)
		pData->weaponFOV = fov;

	return 0;
}

int Lua_WeaponFlipHands(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	bool flip = CLuaUtility::GetBool(L, 2, true);

	if (!pWeapon)
		return CLuaUtility::LuaError(L, "_WeaponFlipHands: Invalid weapon ID");

	SWEPData_t *pData = LuaGetOrCreateSWEPData(pWeapon);
	if (pData)
		pData->weaponSwapHands = flip;

	return 0;
}

int Lua_WeaponSetDamage(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	int damage = CLuaUtility::GetInt(L, 2, 0);
	int secondary = CLuaUtility::GetInt(L, 3, 0);

	if (!pWeapon)
		return CLuaUtility::LuaError(L, "_WeaponSetDamage: Invalid weapon ID");

	SWEPData_t *pData = LuaGetOrCreateSWEPData(pWeapon);
	if (pData)
	{
		if (secondary)
			pData->damageSecondary = damage;
		else
			pData->damage = damage;
	}

	return 0;
}

int Lua_WeaponSetup(lua_State *L)
{
	CBaseCombatWeapon *pWeapon = LuaGetWeapon(L, 1);
	if (!pWeapon)
		return CLuaUtility::LuaError(L, "_WeaponSetup: Invalid weapon ID");

	LuaUpdateSWEPVariables(L, pWeapon);
	return 0;
}

//=============================================================================
// ADDITIONAL PLAYER FUNCTIONS (GMod 9 complete compatibility)
//=============================================================================

// _PlayerGod - Toggle god mode
int Lua_PlayerGod(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	bool godMode = CLuaUtility::GetBool(L, 2, true);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerGod: Invalid player ID");

	if (godMode)
		pPlayer->AddFlag(FL_GODMODE);
	else
		pPlayer->RemoveFlag(FL_GODMODE);
	return 0;
}

// _PlayerHasWeapon - Check if player has weapon
int Lua_PlayerHasWeapon(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *weaponClass = CLuaUtility::GetString(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	CBaseCombatWeapon *pWeapon = pPlayer->Weapon_OwnsThisType(weaponClass);
	lua_pushboolean(L, pWeapon != NULL);
	return 1;
}

// _PlayerIsCrouching - Check if player is crouching
int Lua_PlayerIsCrouching(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, (pPlayer->GetFlags() & FL_DUCKING) != 0);
	return 1;
}

// _PlayerSetDrawViewModel - Show/hide view model
int Lua_PlayerSetDrawViewModel(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	bool draw = CLuaUtility::GetBool(L, 2, true);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetDrawViewModel: Invalid player ID");

	// Send user message to toggle viewmodel visibility
	CSingleUserRecipientFilter filter(pPlayer);
	filter.MakeReliable();

	UserMessageBegin(filter, "HideWeapon");
		WRITE_BYTE(draw ? 0 : 1);
	MessageEnd();

	return 0;
}

// _PlayerSetFlashlight - Toggle flashlight
int Lua_PlayerSetFlashlight(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	bool enabled = CLuaUtility::GetBool(L, 2, true);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetFlashlight: Invalid player ID");

	if (enabled)
		pPlayer->FlashlightTurnOn();
	else
		pPlayer->FlashlightTurnOff();
	return 0;
}

// _PlayerGetFlashlight - Get flashlight state
int Lua_PlayerGetFlashlight(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, pPlayer->FlashlightIsOn());
	return 1;
}

// _PlayerEnableSprint - Enable/disable sprint ability
int Lua_PlayerEnableSprint(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	bool enable = CLuaUtility::GetBool(L, 2, true);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerEnableSprint: Invalid player ID");

	// HL2 specific sprint control
	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);
	if (pHL2Player)
	{
		if (enable)
			pHL2Player->StartSprinting();
		else
			pHL2Player->StopSprinting();
	}
	return 0;
}

// _PlayerLockInPlace - Lock player in place
int Lua_PlayerLockInPlace(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	bool lock = CLuaUtility::GetBool(L, 2, true);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerLockInPlace: Invalid player ID");

	if (lock)
	{
		pPlayer->AddFlag(FL_FROZEN);
		pPlayer->SetAbsVelocity(vec3_origin);
	}
	else
	{
		pPlayer->RemoveFlag(FL_FROZEN);
	}
	return 0;
}

// _PlayerDisableAttack - Disable player attack
int Lua_PlayerDisableAttack(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	bool disable = CLuaUtility::GetBool(L, 2, true);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerDisableAttack: Invalid player ID");

	if (disable)
		pPlayer->AddFlag(FL_ATCONTROLS);  // Prevents attack inputs
	else
		pPlayer->RemoveFlag(FL_ATCONTROLS);
	return 0;
}

// _PlayerSpectatorEnd - Stop spectating
int Lua_PlayerSpectatorEnd(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSpectatorEnd: Invalid player ID");

	pPlayer->StopObserverMode();
	return 0;
}

// _PlayerSetAnimation - Set player animation
int Lua_PlayerSetAnimation(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int animation = CLuaUtility::GetInt(L, 2, PLAYER_IDLE);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetAnimation: Invalid player ID");

	pPlayer->SetAnimation((PLAYER_ANIM)animation);
	return 0;
}

// _PlayerSetVecView - Set player view offset
int Lua_PlayerSetVecView(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	float x = CLuaUtility::GetFloat(L, 2);
	float y = CLuaUtility::GetFloat(L, 3);
	float z = CLuaUtility::GetFloat(L, 4);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSetVecView: Invalid player ID");

	pPlayer->SetViewOffset(Vector(x, y, z));
	return 0;
}

// _PlayerUseVehicle - Put player in/out of vehicle
int Lua_PlayerUseVehicle(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int vehicleEntIndex = CLuaUtility::GetInt(L, 2, 0);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerUseVehicle: Invalid player ID");

	if (vehicleEntIndex == 0)
	{
		// Exit vehicle
		pPlayer->LeaveVehicle();
	}
	else
	{
		// Enter vehicle
		CBaseEntity *pVehicle = UTIL_EntityByIndex(vehicleEntIndex);
		if (pVehicle)
		{
			// Check if it's a vehicle
			IServerVehicle *pServerVehicle = pVehicle->GetServerVehicle();
			if (pServerVehicle)
			{
				pPlayer->GetInVehicle(pServerVehicle, VEHICLE_DRIVER);
			}
		}
	}
	return 0;
}

// _PlayerGetLimit - Get player spawn limit (for server limits)
int Lua_PlayerGetLimit(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *limitType = CLuaUtility::GetString(L, 2);

	// Default limits
	int limit = 100;

	if (Q_stricmp(limitType, "props") == 0)
		limit = 100;
	else if (Q_stricmp(limitType, "ragdolls") == 0)
		limit = 10;
	else if (Q_stricmp(limitType, "effects") == 0)
		limit = 50;
	else if (Q_stricmp(limitType, "vehicles") == 0)
		limit = 5;

	lua_pushinteger(L, limit);
	return 1;
}

// _PlayerShowPanel - Show a VGUI panel to player
int Lua_PlayerShowPanel(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *panelName = CLuaUtility::GetString(L, 2);
	bool show = CLuaUtility::GetBool(L, 3, true);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerShowPanel: Invalid player ID");

	if (panelName && *panelName)
	{
		// HL2 beta uses usermessages for VGUI panels instead of ShowViewPortPanel
		CSingleUserRecipientFilter filter(pPlayer);
		filter.MakeReliable();
		UserMessageBegin(filter, "VGUIMenu");
			WRITE_STRING(panelName);
			WRITE_BYTE(show ? 1 : 0);
		MessageEnd();
	}
	return 0;
}

// _PlayerLastHitGroup - Get last hit group for player
// Note: LastHitGroup is protected in HL2 beta - returns 0 (stub)
int Lua_PlayerLastHitGroup(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	// LastHitGroup is protected in CBaseCombatCharacter - return 0 as stub
	// Would need friend class declaration or public accessor to fix
	lua_pushinteger(L, 0);
	return 1;
}

// _PlayerOption - Trigger a named client option/menu for the player with a timeout.
// Original gmod nets an option name + expiry (curtime+value) to the client, which then
// opens the matching menu (e.g. "ChooseTeam"). bmod sends it as a reliable usermessage;
// the client (gmod_usermessages.cpp) dispatches the option name to the right menu.
// Syntax: _PlayerOption( playerid, optionName, timeout )
int Lua_PlayerOption(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *optionName = CLuaUtility::GetString(L, 2);
	float value = CLuaUtility::GetFloat(L, 3, 0.0f);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer || !optionName || !*optionName)
		return 0;

	CSingleUserRecipientFilter filter(pPlayer);
	filter.MakeReliable();
	UserMessageBegin(filter, "GModPlayerOption");
		WRITE_STRING(optionName);
		WRITE_FLOAT(value);
	MessageEnd();
	return 0;
}

// _PlayerPreferredModel - Returns the player's preferred model string.
// bmod has no separate "preferred model" field, so we return the current player model.
int Lua_PlayerPreferredModel(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	const char *model = (pPlayer && pPlayer->GetModelName() != NULL_STRING)
		? STRING(pPlayer->GetModelName()) : "";
	lua_pushstring(L, model);
	return 1;
}

// _PlayerViewModelSequence / _PlayerSetWeaponSequence - play an animation sequence on the
// player's active view/weapon model. Best-effort: resolve the sequence by name and set it.
int Lua_PlayerViewModelSequence(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *seqName = CLuaUtility::GetString(L, 2);
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer || !seqName || !*seqName)
		return 0;
	CBaseViewModel *pVM = pPlayer->GetViewModel();
	if (pVM)
	{
		int seq = pVM->LookupSequence(seqName);
		if (seq >= 0)
			pVM->SetSequence(seq);
	}
	return 0;
}

int Lua_PlayerSetWeaponSequence(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	const char *seqName = CLuaUtility::GetString(L, 2);
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer || !seqName || !*seqName)
		return 0;
	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if (pWeapon)
	{
		int seq = pWeapon->LookupSequence(seqName);
		if (seq >= 0)
			pWeapon->SetSequence(seq);
	}
	return 0;
}

// The following mirror original gmod globals that no shipped content exercises; they are
// implemented best-effort (correct where a direct beta-engine equivalent exists, otherwise
// they safely consume their args) so the registered surface matches and scripts never nil-call.

// _PlayerShowCrosshair( playerid, bShow ) - crosshair visibility is a client-side concern;
// registered so scripts don't nil-call. (Unused by shipped content.)
int Lua_PlayerShowCrosshair(lua_State *L)
{
	(void)UTIL_PlayerByIndex(CLuaUtility::GetInt(L, 1));
	return 0;
}

// _PlayerSetDrawWorldModel( playerid, bDraw ) - weapon world-model draw is a client-render
// concern on this engine; registered so scripts don't nil-call. (Unused by shipped content.)
int Lua_PlayerSetDrawWorldModel(lua_State *L)
{
	(void)UTIL_PlayerByIndex(CLuaUtility::GetInt(L, 1));
	return 0;
}

// _PlayerSetVecDuck( playerid, vector ) - duck view offset. Consumed (no beta netvar).
int Lua_PlayerSetVecDuck(lua_State *L)
{
	(void)UTIL_PlayerByIndex(CLuaUtility::GetInt(L, 1));
	return 0;
}

// _PlayerDetonateTripmines( playerid ) - detonate the player's placed SLAM tripmines.
int Lua_PlayerDetonateTripmines(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return 0;
	CBaseEntity *pEnt = NULL;
	while ((pEnt = gEntList.FindEntityByClassname(pEnt, "npc_satchel")) != NULL)
	{
		if (pEnt->GetOwnerEntity() == pPlayer)
			pEnt->Use(pPlayer, pPlayer, USE_ON, 0.0f);
	}
	return 0;
}

// _PlayerWeaponTranslateSequence( playerid, seq ) - returns the (identity) translated sequence.
int Lua_PlayerWeaponTranslateSequence(lua_State *L)
{
	lua_pushinteger(L, CLuaUtility::GetInt(L, 2, 0));
	return 1;
}

// _WeaponScriptedAssign - SWEP scripted-weapon assignment hook. Registered for surface parity;
// bmod's SWEP assignment happens through _SpawnSWEP/_PlayerGiveSWEP. (Unused by shipped content.)
int Lua_WeaponScriptedAssign(lua_State *L)
{
	return 0;
}

//=============================================================================
// REGISTRATION
//=============================================================================

void RegisterLuaPlayerFunctions()
{
	// Player control
	CLuaIntegration::RegisterFunction("_PlayerFreeze", Lua_PlayerFreeze, "Freeze the specified player. Syntax: <playerid> <freeze bool>");
	CLuaIntegration::RegisterFunction("_PlayerSetSprint", Lua_PlayerSetSprint_New, "Enable/Disable sprint for player. Syntax: <playerid> <freeze bool>");
	CLuaIntegration::RegisterFunction("_PlayerKill", Lua_PlayerKill, "Kill the specified player. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerRespawn", Lua_PlayerRespawn, "Force player to re-spawn. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerSilentKill", Lua_PlayerSilentKill, "Silently kill a player. Syntax: <playerid> <respawn time> <bool Dissolve>");

	// Player properties
	CLuaIntegration::RegisterFunction("_PlayerGetShootPos", Lua_PlayerGetShootPos, "Returns the vector of the player's shoot position. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerGetShootAng", Lua_PlayerGetShootAng, "Returns the forward vector of the player's shoot angle. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerGetActiveWeapon", Lua_PlayerGetActiveWeapon, "Returns the player's active weapon id. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerSetHealth", Lua_PlayerSetHealth, "Changes a player's health. Syntax: <playerid> <newhealth> ");
	CLuaIntegration::RegisterFunction("_PlayerSetArmor", Lua_PlayerSetArmor, "Changes a player's armour. Syntax: <playerid> <newarmor> ");
	CLuaIntegration::RegisterFunction("_PlayerSetMaxSpeed", Lua_PlayerSetMaxSpeed, "Changes a player's max speed. Syntax: <playerid> <speed> ");
	CLuaIntegration::RegisterFunction("_PlayerSetModel", Lua_PlayerSetModel_New, "Changes a player's model. Syntax: <playerid> <model> ");
	CLuaIntegration::RegisterFunction("_PlayerSetFOV", Lua_PlayerSetFOV, "Syntax: <playerid> <fov> <time>");
	CLuaIntegration::RegisterFunction("_PlayerInfo", Lua_PlayerInfo, "Returns info about a specific player (check 'connected' first!). Syntax: <playerid> <request> ");
	CLuaIntegration::RegisterFunction("_PlayerIsKeyDown", Lua_PlayerIsKeyDown, "Check whether specified key is pressed. Syntax: <playerid> <in_key>");
	CLuaIntegration::RegisterFunction("_PlayerStopZooming", Lua_PlayerStopZooming, "Syntax: <playerid>");

	// Weapons/Items
	CLuaIntegration::RegisterFunction("_PlayerGiveAmmo", Lua_PlayerGiveAmmo_New, "Give specified player ammo. Syntax: <playerid> <num amount> <string ammotype> <bool playsounds>");
	CLuaIntegration::RegisterFunction("_PlayerGiveItem", Lua_PlayerGiveItem, "Give player named item. Syntax: <playerid> <item> ");
	CLuaIntegration::RegisterFunction("_PlayerGiveSWEP", Lua_PlayerGiveSWEP, "Give player scripted weapon. Syntax: <playerid> <weapon script> ");
	CLuaIntegration::RegisterFunction("_PlayerRemoveAllWeapons", Lua_PlayerRemoveAllWeapons, "Strip all of a player's weapons. Syntax: <playerid> ");
	CLuaIntegration::RegisterFunction("_PlayerRemoveWeapon", Lua_PlayerRemoveWeapon, "Strip a specific weapon. Syntax: <playerid> <weaponname> ");
	CLuaIntegration::RegisterFunction("_PlayerRemoveAllAmmo", Lua_PlayerRemoveAllAmmo, "Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerSelectWeapon", Lua_PlayerSelectWeapon, "Select weapon. Syntax: <playerid> <weapon name>");
	CLuaIntegration::RegisterFunction("_PlayerHolsterWeapon", Lua_PlayerHolsterWeapon, "Syntax: <playerid>");

	// Spectator
	CLuaIntegration::RegisterFunction("_PlayerSpectatorStart", Lua_PlayerSpectatorStart, "Stop spectating mode. Syntax: <playerid> <mode>");
	CLuaIntegration::RegisterFunction("_PlayerSpectatorTarget", Lua_PlayerSpectatorTarget, "Set Spectator target for player. Syntax: <playerid> <target>");
	CLuaIntegration::RegisterFunction("_PlayerSetChaseCamDistance", Lua_PlayerSetChaseCamDistance, "Syntax: <playerid> <distance>");

	// UI
	CLuaIntegration::RegisterFunction("_PlayerShowScoreboard", Lua_PlayerShowScoreboard, "Shows the scoreboard on the specified players screen. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerAllowDecalPaint", Lua_PlayerAllowDecalPaint, "Allow the player to spraypaint now rather than waiting the 30 or so seconds. Syntax: <playerid> ");

	// Teams/Scoring
	CLuaIntegration::RegisterFunction("_PlayerChangeTeam", Lua_PlayerChangeTeam, "Changes a player's team. Syntax: <playerid> <teamid> ");
	CLuaIntegration::RegisterFunction("_PlayerAddScore", Lua_PlayerAddScore, "Add to players score (can be minus). Syntax: <playerid> <increment> ");
	CLuaIntegration::RegisterFunction("_PlayerSetScore", Lua_PlayerSetScore, "Sets to players score. Syntax: <playerid> <score> ");
	CLuaIntegration::RegisterFunction("_PlayerAddDeath", Lua_PlayerAddDeath, "Add to players deaths score (can be minus). Syntax: <playerid> <increment> ");

	// Team functions
	CLuaIntegration::RegisterFunction("_TeamAddScore", Lua_TeamAddScore, "Add score for a team. Syntax: <teamid> <increment> ");
	CLuaIntegration::RegisterFunction("_TeamSetScore", Lua_TeamSetScore, "Set score for a team. Syntax: <teamid> <score> ");
	CLuaIntegration::RegisterFunction("_TeamNumPlayers", Lua_TeamNumPlayers, "Get the number of players on a team.. Syntax: <teamid>");
	CLuaIntegration::RegisterFunction("_TeamScore", Lua_TeamScore, "Returns the total team score. Syntax: <teamid>");
	CLuaIntegration::RegisterFunction("_TeamCount", Lua_TeamCount, "Returns the number of teams");
	CLuaIntegration::RegisterFunction("_TeamSetName", Lua_TeamSetName, "Sets the name of the team. Syntax: <teamid> <new name>");

	// Game info
	CLuaIntegration::RegisterFunction("_MaxPlayers", Lua_MaxPlayers, "Returns the max players in the server.");
	CLuaIntegration::RegisterFunction("_CurTime", Lua_CurTime, "Returns the current time in seconds.");

	// SWEP functions
	CLuaIntegration::RegisterFunction("_SpawnSWEP", Lua_SpawnSWEP, "Spawn SWEP. Syntax: <swepclass> <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_SWEPUpdateVariables", Lua_SWEPUpdateVariables, "Re-reads all the 'fetch' variables. You should call this if you have changed the accuracy etc.. Syntax: <weapon index>");
	CLuaIntegration::RegisterFunction("_SWEPUseAmmo", Lua_SWEPUseAmmo, "Takes ammo from SWEP gun. Syntax: <weapon index> <clip# [0|1]> <amount>");
	CLuaIntegration::RegisterFunction("_SWEPRunString", Lua_SWEPRunString, "Runs a LUA string in the weapon's LUA instance Syntax: <weapon index> <string>");
	CLuaIntegration::RegisterFunction("_SWEPSetSound", Lua_WeaponSetSound, "Sets SWEP's sound. Syntax: <weapon index> <action> <sound>");
	CLuaIntegration::RegisterFunction("_WeaponSetModel", Lua_WeaponSetModel, "Sets weapon model. Syntax: <weapon name> <model type> <model name>");
	CLuaIntegration::RegisterFunction("_WeaponSetSlot", Lua_WeaponSetSlot, "Sets weapon slot. Syntax: <weapon name> <slot> <pos>");
	CLuaIntegration::RegisterFunction("_WeaponSetSound", Lua_WeaponSetSound, "Sets weapon sound. Syntax: <weapon name> <action> <sound>");
	CLuaIntegration::RegisterFunction("_WeaponSetFOV", Lua_WeaponSetFOV, "Sets a weapon's draw FOV. Syntax: <weapon name> <fov>");
	CLuaIntegration::RegisterFunction("_WeaponFlipHands", Lua_WeaponFlipHands, "Flips a weapon's hands. Syntax: <weapon name> <bool>");
	CLuaIntegration::RegisterFunction("_WeaponSetDamage", Lua_WeaponSetDamage, "Sets the damage that a bullet from this gun does. Syntax: <weapon name> <damage>");
	CLuaIntegration::RegisterFunction("_WeaponSetup", Lua_WeaponSetup, "Sets anim prefix . Syntax: <weapon name> <animprefix> <clipsize1> <clipsize2> <defaultammo1> <defaultammo2> <primaryammo> <secondaryammo>");

	CLuaIntegration::RegisterTableFunction("_swep", "GetClipAmmo", Lua_swep_GetClipAmmo, "Get amount of ammo in a clip. Syntax: <weapon index> <clip# [0|1]>");
	CLuaIntegration::RegisterTableFunction("_swep", "SetClipAmmo", Lua_swep_SetClipAmmo, "Get amount of ammo in a clip. Syntax: <weapon index> <clip# [0|1]> <clip>");
	CLuaIntegration::RegisterTableFunction("_swep", "GetDeathIcon", Lua_swep_GetDeathIcon, "Gets death icon name. Syntax: <weapon index>");

	// Additional GMod 9 compatibility functions
	CLuaIntegration::RegisterFunction("_PlayerGod", Lua_PlayerGod, "Give a player God Mode. Syntax: <player> <bool>");
	CLuaIntegration::RegisterFunction("_PlayerHasWeapon", Lua_PlayerHasWeapon, "Returns true if the player has this weapon. Syntax: <player> <weapon name>");
	CLuaIntegration::RegisterFunction("_PlayerIsCrouching", Lua_PlayerIsCrouching, "Returns true if a player is crouching. Syntax: <player>");
	CLuaIntegration::RegisterFunction("_PlayerSetDrawViewModel", Lua_PlayerSetDrawViewModel, "Sets whether to draw the player's view model. Syntax: <playerid> <bool>");
	CLuaIntegration::RegisterFunction("_PlayerSetFlashlight", Lua_PlayerSetFlashlight, "Syntax: <playerid> <bool (on/off)>");
	CLuaIntegration::RegisterFunction("_PlayerGetFlashlight", Lua_PlayerGetFlashlight, "Get flashlight state. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerEnableSprint", Lua_PlayerEnableSprint, "Syntax: <playerid> <bool>");
	CLuaIntegration::RegisterFunction("_PlayerLockInPlace", Lua_PlayerLockInPlace, "Syntax: <playerid> <bool>");
	CLuaIntegration::RegisterFunction("_PlayerDisableAttack", Lua_PlayerDisableAttack, "Syntax: <playerid> <bool>");
	CLuaIntegration::RegisterFunction("_PlayerSpectatorEnd", Lua_PlayerSpectatorEnd, "Stop spectating mode. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerSetAnimation", Lua_PlayerSetAnimation, "Sets a players animation. Syntax: <player> <int sequence>");
	CLuaIntegration::RegisterFunction("_PlayerSetVecView", Lua_PlayerSetVecView, "Sets the position of the view vector (0, 0, 64) default. Syntax: <playerid> <vector>");
	CLuaIntegration::RegisterFunction("_PlayerUseVehicle", Lua_PlayerUseVehicle, "Make a player attempt to enter a vehicle. Syntax: <player> <vehicle>");
	CLuaIntegration::RegisterFunction("_PlayerGetLimit", Lua_PlayerGetLimit, " Syntax: <playerid> <limit name>");
	CLuaIntegration::RegisterFunction("_PlayerShowPanel", Lua_PlayerShowPanel, "Show VGUI panel. Syntax: <playerid> <panel> [show]");
	CLuaIntegration::RegisterFunction("_PlayerLastHitGroup", Lua_PlayerLastHitGroup, "Get last hit group. Syntax: <playerid>");

	// Functions the original gmod registers that shipped content uses (were missing)
	CLuaIntegration::RegisterFunction("_PlayerOption", Lua_PlayerOption, "Player has an option to select. Syntax: <playerid> <callback> <timeout>");
	CLuaIntegration::RegisterFunction("_PlayerPreferredModel", Lua_PlayerPreferredModel, "Returns the player's preferred model. Returns a blank if the model isn't valid. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerViewModelSequence", Lua_PlayerViewModelSequence, "If successful returns the length of the sequence in seconds. Syntax: <playerid> <int sequence>");
	CLuaIntegration::RegisterFunction("_PlayerSetWeaponSequence", Lua_PlayerSetWeaponSequence, "Sets a players weapon activity. Syntax: <player> <int sequence>");
	CLuaIntegration::RegisterFunction("_PlayerShowCrosshair", Lua_PlayerShowCrosshair, "Syntax: <playerid> <bool>");
	CLuaIntegration::RegisterFunction("_PlayerSetDrawWorldModel", Lua_PlayerSetDrawWorldModel, "Sets whether to draw the player's world model. Syntax: <playerid> <bool>");
	CLuaIntegration::RegisterFunction("_PlayerSetVecDuck", Lua_PlayerSetVecDuck, "Sets the position of the duck view vector (0, 0, 24) default. Syntax: <playerid> <vector>");
	CLuaIntegration::RegisterFunction("_PlayerDetonateTripmines", Lua_PlayerDetonateTripmines, "Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerWeaponTranslateSequence", Lua_PlayerWeaponTranslateSequence, "Let the player's weapon decide how we should act out specific sequence. Syntax: <player> <int sequence>");
	CLuaIntegration::RegisterFunction("_WeaponScriptedAssign", Lua_WeaponScriptedAssign, "Assign a script to a weapon scripted. Syntax: <weapon> <script> ");

	// Aliases for GMod script compatibility (underscore style)
}
