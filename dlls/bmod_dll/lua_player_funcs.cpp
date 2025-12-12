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
#include "recipientfilter.h"
#include "usermessages.h"
#include "gmod_gamemode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// PLAYER FUNCTIONS
//=============================================================================

// _PlayerFreeze - Freeze/unfreeze player
int Lua_PlayerFreeze(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	bool freeze = CLuaUtility::GetBool(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerFreeze: Invalid player ID");

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
	lua_pushnumber(L, pos.x);
	lua_pushnumber(L, pos.y);
	lua_pushnumber(L, pos.z);
	return 3;
}

// _PlayerGetShootAng - Get player eye angles
int Lua_PlayerGetShootAng(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerGetShootAng: Invalid player ID");

	QAngle ang = pPlayer->EyeAngles();
	lua_pushnumber(L, ang.x);
	lua_pushnumber(L, ang.y);
	lua_pushnumber(L, ang.z);
	return 3;
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

	pPlayer->SetModel(model);
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
		return CLuaUtility::LuaError(L, "_PlayerGiveAmmo: Invalid ammo type");

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
	int teamNum = CLuaUtility::GetInt(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerChangeTeam: Invalid player ID");

	pPlayer->ChangeTeam(teamNum);
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
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
	{
		lua_pushnil(L);
		return 1;
	}

	// Return a table with player info
	lua_newtable(L);

	// HL2 beta uses pl.netname (string_t) instead of GetPlayerName()
	const char *playerName = STRING(pPlayer->pl.netname);

	lua_pushstring(L, "name");
	lua_pushstring(L, playerName ? playerName : "");
	lua_settable(L, -3);

	lua_pushstring(L, "health");
	lua_pushnumber(L, (lua_Number)pPlayer->GetHealth());
	lua_settable(L, -3);

	lua_pushstring(L, "armor");
	lua_pushnumber(L, (lua_Number)pPlayer->ArmorValue());
	lua_settable(L, -3);

	lua_pushstring(L, "team");
	lua_pushnumber(L, (lua_Number)pPlayer->GetTeamNumber());
	lua_settable(L, -3);

	lua_pushstring(L, "alive");
	lua_pushboolean(L, pPlayer->IsAlive());
	lua_settable(L, -3);

	return 1;
}

//=============================================================================
// TEAM FUNCTIONS
//=============================================================================

// _TeamAddScore - Add to team score
int Lua_TeamAddScore(lua_State *L)
{
	int teamNum = CLuaUtility::GetInt(L, 1);
	int score = CLuaUtility::GetInt(L, 2);

	// Use CGModGamemodeSystem which has proper team support
	int currentScore = CGModGamemodeSystem::GetTeamScore(teamNum);
	CGModGamemodeSystem::SetTeamScore(teamNum, currentScore + score);
	return 0;
}

// _TeamSetScore - Set team score
int Lua_TeamSetScore(lua_State *L)
{
	int teamNum = CLuaUtility::GetInt(L, 1);
	int score = CLuaUtility::GetInt(L, 2);

	// Use CGModGamemodeSystem which has proper team support
	CGModGamemodeSystem::SetTeamScore(teamNum, score);
	return 0;
}

// _TeamNumPlayers - Get number of players on team
int Lua_TeamNumPlayers(lua_State *L)
{
	int teamNum = CLuaUtility::GetInt(L, 1);

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
	int teamNum = CLuaUtility::GetInt(L, 1);

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
int Lua_PlayerSpectatorStart(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSpectatorStart: Invalid player ID");

	// Put player into observer mode (HL2 beta uses position/angle based observer)
	pPlayer->StartObserverMode(pPlayer->GetAbsOrigin(), pPlayer->GetAbsAngles());
	return 0;
}

// _PlayerSpectatorTarget - Set spectator target
int Lua_PlayerSpectatorTarget(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int targetID = CLuaUtility::GetInt(L, 2);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	CBasePlayer *pTarget = UTIL_PlayerByIndex(targetID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_PlayerSpectatorTarget: Invalid player ID");

	if (pTarget)
	{
		pPlayer->SetObserverTarget(pTarget);
		// HL2 beta uses position/angle based observer
		pPlayer->StartObserverMode(pTarget->GetAbsOrigin(), pTarget->GetAbsAngles());
	}
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

	// Check if button is pressed
	bool isDown = (pPlayer->m_nButtons & key) != 0;
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
	int teamNum = CLuaUtility::GetInt(L, 1);
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

// _SWEPUpdateVariables - Update SWEP variables (stub)
int Lua_SWEPUpdateVariables(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	// SWEP variable update would be handled by SWEP system
	return 0;
}

// _SWEPUseAmmo - Use ammo from SWEP
int Lua_SWEPUseAmmo(lua_State *L)
{
	int playerID = CLuaUtility::GetInt(L, 1);
	int amount = CLuaUtility::GetInt(L, 2, 1);

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(playerID);
	if (!pPlayer)
		return CLuaUtility::LuaError(L, "_SWEPUseAmmo: Invalid player ID");

	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if (pWeapon)
	{
		// Remove ammo from current weapon
		int ammoType = pWeapon->GetPrimaryAmmoType();
		if (ammoType >= 0)
		{
			pPlayer->RemoveAmmo(amount, ammoType);
		}
	}
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

//=============================================================================
// REGISTRATION
//=============================================================================

void RegisterLuaPlayerFunctions()
{
	// Player control
	CLuaIntegration::RegisterFunction("_PlayerFreeze", Lua_PlayerFreeze, "Freeze/unfreeze player. Syntax: <playerid> <freeze bool>");
	CLuaIntegration::RegisterFunction("_PlayerSetSprint", Lua_PlayerSetSprint_New, "Enable/disable sprint. Syntax: <playerid> <bool>");
	CLuaIntegration::RegisterFunction("_PlayerKill", Lua_PlayerKill, "Kill player. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerRespawn", Lua_PlayerRespawn, "Respawn player. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerSilentKill", Lua_PlayerSilentKill, "Kill player silently. Syntax: <playerid>");

	// Player properties
	CLuaIntegration::RegisterFunction("_PlayerGetShootPos", Lua_PlayerGetShootPos, "Get player eye position. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerGetShootAng", Lua_PlayerGetShootAng, "Get player eye angles. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerGetActiveWeapon", Lua_PlayerGetActiveWeapon, "Get player's active weapon. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerSetHealth", Lua_PlayerSetHealth, "Set player health. Syntax: <playerid> <health>");
	CLuaIntegration::RegisterFunction("_PlayerSetArmor", Lua_PlayerSetArmor, "Set player armor. Syntax: <playerid> <armor>");
	CLuaIntegration::RegisterFunction("_PlayerSetMaxSpeed", Lua_PlayerSetMaxSpeed, "Set player max speed. Syntax: <playerid> <speed>");
	CLuaIntegration::RegisterFunction("_PlayerSetModel", Lua_PlayerSetModel_New, "Set player model. Syntax: <playerid> <modelpath>");
	CLuaIntegration::RegisterFunction("_PlayerSetFOV", Lua_PlayerSetFOV, "Set player FOV. Syntax: <playerid> <fov> [rate]");
	CLuaIntegration::RegisterFunction("_PlayerInfo", Lua_PlayerInfo, "Get player info table. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerIsKeyDown", Lua_PlayerIsKeyDown, "Check if key pressed. Syntax: <playerid> <key>");
	CLuaIntegration::RegisterFunction("_PlayerStopZooming", Lua_PlayerStopZooming, "Stop zoom effect. Syntax: <playerid>");

	// Weapons/Items
	CLuaIntegration::RegisterFunction("_PlayerGiveAmmo", Lua_PlayerGiveAmmo_New, "Give ammo to player. Syntax: <playerid> <amount> <ammotype> [playsounds]");
	CLuaIntegration::RegisterFunction("_PlayerGiveItem", Lua_PlayerGiveItem, "Give weapon to player. Syntax: <playerid> <weaponclass>");
	CLuaIntegration::RegisterFunction("_PlayerGiveSWEP", Lua_PlayerGiveSWEP, "Give SWEP to player. Syntax: <playerid> <swepclass>");
	CLuaIntegration::RegisterFunction("_PlayerRemoveAllWeapons", Lua_PlayerRemoveAllWeapons, "Remove all weapons. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerRemoveWeapon", Lua_PlayerRemoveWeapon, "Remove specific weapon. Syntax: <playerid> <weaponclass>");
	CLuaIntegration::RegisterFunction("_PlayerRemoveAllAmmo", Lua_PlayerRemoveAllAmmo, "Remove all ammo. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerSelectWeapon", Lua_PlayerSelectWeapon, "Select weapon. Syntax: <playerid> <weaponclass>");
	CLuaIntegration::RegisterFunction("_PlayerHolsterWeapon", Lua_PlayerHolsterWeapon, "Holster current weapon. Syntax: <playerid>");

	// Spectator
	CLuaIntegration::RegisterFunction("_PlayerSpectatorStart", Lua_PlayerSpectatorStart, "Start spectating. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerSpectatorTarget", Lua_PlayerSpectatorTarget, "Set spectator target. Syntax: <playerid> <targetid>");

	// UI
	CLuaIntegration::RegisterFunction("_PlayerShowScoreboard", Lua_PlayerShowScoreboard, "Show scoreboard. Syntax: <playerid> [show]");
	CLuaIntegration::RegisterFunction("_PlayerAllowDecalPaint", Lua_PlayerAllowDecalPaint, "Allow decal paint. Syntax: <playerid>");

	// Teams/Scoring
	CLuaIntegration::RegisterFunction("_PlayerChangeTeam", Lua_PlayerChangeTeam, "Change player team. Syntax: <playerid> <teamnum>");
	CLuaIntegration::RegisterFunction("_PlayerAddScore", Lua_PlayerAddScore, "Add to player score. Syntax: <playerid> <score>");
	CLuaIntegration::RegisterFunction("_PlayerSetScore", Lua_PlayerSetScore, "Set player score. Syntax: <playerid> <score>");
	CLuaIntegration::RegisterFunction("_PlayerAddDeath", Lua_PlayerAddDeath, "Add to death count. Syntax: <playerid> [deaths]");

	// Team functions
	CLuaIntegration::RegisterFunction("_TeamAddScore", Lua_TeamAddScore, "Add to team score. Syntax: <teamnum> <score>");
	CLuaIntegration::RegisterFunction("_TeamSetScore", Lua_TeamSetScore, "Set team score. Syntax: <teamnum> <score>");
	CLuaIntegration::RegisterFunction("_TeamNumPlayers", Lua_TeamNumPlayers, "Get team player count. Syntax: <teamnum>");
	CLuaIntegration::RegisterFunction("_TeamScore", Lua_TeamScore, "Get team score. Syntax: <teamnum>");
	CLuaIntegration::RegisterFunction("_TeamCount", Lua_TeamCount, "Get number of teams.");
	CLuaIntegration::RegisterFunction("_TeamSetName", Lua_TeamSetName, "Set team name. Syntax: <teamnum> <name>");

	// Game info
	CLuaIntegration::RegisterFunction("_MaxPlayers", Lua_MaxPlayers, "Get max players.");
	CLuaIntegration::RegisterFunction("_CurTime", Lua_CurTime, "Get current server time.");

	// SWEP functions
	CLuaIntegration::RegisterFunction("_SpawnSWEP", Lua_SpawnSWEP, "Spawn SWEP. Syntax: <swepclass> <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_SWEPUpdateVariables", Lua_SWEPUpdateVariables, "Update SWEP variables. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_SWEPUseAmmo", Lua_SWEPUseAmmo, "Use SWEP ammo. Syntax: <playerid> [amount]");

	// Additional GMod 9 compatibility functions
	CLuaIntegration::RegisterFunction("_PlayerGod", Lua_PlayerGod, "Toggle god mode. Syntax: <playerid> [enable]");
	CLuaIntegration::RegisterFunction("_PlayerHasWeapon", Lua_PlayerHasWeapon, "Check if player has weapon. Syntax: <playerid> <weaponclass>");
	CLuaIntegration::RegisterFunction("_PlayerIsCrouching", Lua_PlayerIsCrouching, "Check if crouching. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerSetDrawViewModel", Lua_PlayerSetDrawViewModel, "Show/hide viewmodel. Syntax: <playerid> [draw]");
	CLuaIntegration::RegisterFunction("_PlayerSetFlashlight", Lua_PlayerSetFlashlight, "Toggle flashlight. Syntax: <playerid> [enable]");
	CLuaIntegration::RegisterFunction("_PlayerGetFlashlight", Lua_PlayerGetFlashlight, "Get flashlight state. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerEnableSprint", Lua_PlayerEnableSprint, "Enable/disable sprint. Syntax: <playerid> [enable]");
	CLuaIntegration::RegisterFunction("_PlayerLockInPlace", Lua_PlayerLockInPlace, "Lock player in place. Syntax: <playerid> [lock]");
	CLuaIntegration::RegisterFunction("_PlayerDisableAttack", Lua_PlayerDisableAttack, "Disable attack. Syntax: <playerid> [disable]");
	CLuaIntegration::RegisterFunction("_PlayerSpectatorEnd", Lua_PlayerSpectatorEnd, "Stop spectating. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_PlayerSetAnimation", Lua_PlayerSetAnimation, "Set animation. Syntax: <playerid> <animation>");
	CLuaIntegration::RegisterFunction("_PlayerSetVecView", Lua_PlayerSetVecView, "Set view offset. Syntax: <playerid> <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_PlayerUseVehicle", Lua_PlayerUseVehicle, "Use vehicle. Syntax: <playerid> [vehicleid]");
	CLuaIntegration::RegisterFunction("_PlayerGetLimit", Lua_PlayerGetLimit, "Get spawn limit. Syntax: <playerid> <limittype>");
	CLuaIntegration::RegisterFunction("_PlayerShowPanel", Lua_PlayerShowPanel, "Show VGUI panel. Syntax: <playerid> <panel> [show]");
	CLuaIntegration::RegisterFunction("_PlayerLastHitGroup", Lua_PlayerLastHitGroup, "Get last hit group. Syntax: <playerid>");

	// Aliases for GMod script compatibility (underscore style)
	CLuaIntegration::RegisterFunction("_player_ShowPanel", Lua_PlayerShowPanel, "Show VGUI panel. Syntax: <playerid> <panel> [show]");
	CLuaIntegration::RegisterFunction("_player_GetFlashlight", Lua_PlayerGetFlashlight, "Get flashlight state. Syntax: <playerid>");
	CLuaIntegration::RegisterFunction("_player_SetFlashlight", Lua_PlayerSetFlashlight, "Toggle flashlight. Syntax: <playerid> [enable]");
	CLuaIntegration::RegisterFunction("_player_LastHitGroup", Lua_PlayerLastHitGroup, "Get last hit group. Syntax: <playerid>");
}
