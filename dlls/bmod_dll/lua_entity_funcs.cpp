//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lua entity functions for BarrysMod
// Based on reverse engineering of Garry's Mod 9.0.4b server.dll
//
//=============================================================================//

#include "cbase.h"
#include "lua_integration.h"
#include "player.h"
#include "filesystem.h"
#include "soundent.h"
#include "entitylist.h"
#include "ai_basenpc.h"
#include "physics.h"
#include "physics_saverestore.h"
#include "vphysics_interface.h"
#include "eventqueue.h"
#include "recipientfilter.h"
#include "decals.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// ENTITY FUNCTIONS
//=============================================================================

// _EntGetPos - Get entity position
int Lua_EntGetPos(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetPos: Invalid entity");

	Vector pos = pEntity->GetAbsOrigin();
	lua_pushnumber(L, pos.x);
	lua_pushnumber(L, pos.y);
	lua_pushnumber(L, pos.z);
	return 3;
}

// _EntSetPos - Set entity position
int Lua_EntSetPos(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	float x = CLuaUtility::GetFloat(L, 2);
	float y = CLuaUtility::GetFloat(L, 3);
	float z = CLuaUtility::GetFloat(L, 4);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetPos: Invalid entity");

	pEntity->SetAbsOrigin(Vector(x, y, z));
	return 0;
}

// _EntGetAng - Get entity angles
int Lua_EntGetAng(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetAng: Invalid entity");

	QAngle ang = pEntity->GetAbsAngles();
	lua_pushnumber(L, ang.x);
	lua_pushnumber(L, ang.y);
	lua_pushnumber(L, ang.z);
	return 3;
}

// _EntSetAng - Set entity angles
int Lua_EntSetAng(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	float pitch = CLuaUtility::GetFloat(L, 2);
	float yaw = CLuaUtility::GetFloat(L, 3);
	float roll = CLuaUtility::GetFloat(L, 4);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetAng: Invalid entity");

	pEntity->SetAbsAngles(QAngle(pitch, yaw, roll));
	return 0;
}

// _EntGetVelocity - Get entity velocity
int Lua_EntGetVelocity(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetVelocity: Invalid entity");

	Vector vel = pEntity->GetAbsVelocity();
	lua_pushnumber(L, vel.x);
	lua_pushnumber(L, vel.y);
	lua_pushnumber(L, vel.z);
	return 3;
}

// _EntSetVelocity - Set entity velocity
int Lua_EntSetVelocity(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	float x = CLuaUtility::GetFloat(L, 2);
	float y = CLuaUtility::GetFloat(L, 3);
	float z = CLuaUtility::GetFloat(L, 4);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetVelocity: Invalid entity");

	pEntity->SetAbsVelocity(Vector(x, y, z));
	return 0;
}

// _EntGetForwardVector - Get entity forward vector
int Lua_EntGetForwardVector(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetForwardVector: Invalid entity");

	Vector forward;
	AngleVectors(pEntity->GetAbsAngles(), &forward);
	lua_pushnumber(L, forward.x);
	lua_pushnumber(L, forward.y);
	lua_pushnumber(L, forward.z);
	return 3;
}

// _EntGetRightVector - Get entity right vector
int Lua_EntGetRightVector(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetRightVector: Invalid entity");

	Vector forward, right;
	AngleVectors(pEntity->GetAbsAngles(), &forward, &right, NULL);
	lua_pushnumber(L, right.x);
	lua_pushnumber(L, right.y);
	lua_pushnumber(L, right.z);
	return 3;
}

// _EntGetUpVector - Get entity up vector
int Lua_EntGetUpVector(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetUpVector: Invalid entity");

	Vector forward, right, up;
	AngleVectors(pEntity->GetAbsAngles(), &forward, &right, &up);
	lua_pushnumber(L, up.x);
	lua_pushnumber(L, up.y);
	lua_pushnumber(L, up.z);
	return 3;
}

// _EntCreate - Create a new entity
int Lua_EntCreate(lua_State *L)
{
	const char *classname = CLuaUtility::GetString(L, 1);
	if (!classname || !*classname)
		return CLuaUtility::LuaError(L, "_EntCreate: Invalid classname");

	CBaseEntity *pEntity = CreateEntityByName(classname);
	if (!pEntity)
	{
		lua_pushnil(L);
		return 1;
	}

	lua_pushinteger(L, pEntity->entindex());
	return 1;
}

// _EntSpawn - Spawn an entity
int Lua_EntSpawn(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSpawn: Invalid entity");

	DispatchSpawn(pEntity);
	return 0;
}

// _EntActivate - Activate an entity
int Lua_EntActivate(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntActivate: Invalid entity");

	pEntity->Activate();
	return 0;
}

// _EntRemove - Remove an entity
int Lua_EntRemove_New(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntRemove: Invalid entity");

	UTIL_Remove(pEntity);
	return 0;
}

// _EntExists - Check if entity exists
int Lua_EntExists(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	lua_pushboolean(L, pEntity != NULL);
	return 1;
}

// _EntSetKeyValue - Set entity keyvalue
int Lua_EntSetKeyValue(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	const char *key = CLuaUtility::GetString(L, 2);
	const char *value = CLuaUtility::GetString(L, 3);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetKeyValue: Invalid entity");

	pEntity->KeyValue(key, value);
	return 0;
}

// _EntSetModel - Set entity model
int Lua_EntSetModel(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	const char *model = CLuaUtility::GetString(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetModel: Invalid entity");

	pEntity->SetModel(model);
	return 0;
}

// _EntGetModel - Get entity model
int Lua_EntGetModel(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetModel: Invalid entity");

	lua_pushstring(L, STRING(pEntity->GetModelName()));
	return 1;
}

// _EntPrecacheModel - Precache a model
int Lua_EntPrecacheModel(lua_State *L)
{
	const char *model = CLuaUtility::GetString(L, 1);
	if (!model || !*model)
		return CLuaUtility::LuaError(L, "_EntPrecacheModel: Invalid model path");

	int index = engine->PrecacheModel(model);
	lua_pushinteger(L, index);
	return 1;
}

// _EntSetOwner - Set entity owner
int Lua_EntSetOwner(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int ownerIndex = CLuaUtility::GetInt(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	CBaseEntity *pOwner = UTIL_EntityByIndex(ownerIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetOwner: Invalid entity");

	pEntity->SetOwnerEntity(pOwner);
	return 0;
}

// _EntGetOwner - Get entity owner
int Lua_EntGetOwner(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetOwner: Invalid entity");

	CBaseEntity *pOwner = pEntity->GetOwnerEntity();
	lua_pushinteger(L, pOwner ? pOwner->entindex() : -1);
	return 1;
}

// _EntGetType - Get entity class name
int Lua_EntGetType(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetType: Invalid entity");

	lua_pushstring(L, pEntity->GetClassname());
	return 1;
}

// _EntGetByName - Get entity by targetname
int Lua_EntGetByName(lua_State *L)
{
	const char *name = CLuaUtility::GetString(L, 1);
	if (!name || !*name)
		return CLuaUtility::LuaError(L, "_EntGetByName: Invalid name");

	CBaseEntity *pEntity = gEntList.FindEntityByName(NULL, name, NULL);
	if (pEntity)
		lua_pushinteger(L, pEntity->entindex());
	else
		lua_pushnil(L);
	return 1;
}

// _EntGetName - Get entity targetname
int Lua_EntGetName(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetName: Invalid entity");

	lua_pushstring(L, STRING(pEntity->GetEntityName()));
	return 1;
}

// _EntSetName - Set entity targetname
int Lua_EntSetName(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	const char *name = CLuaUtility::GetString(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetName: Invalid entity");

	pEntity->SetName(MAKE_STRING(name));
	return 0;
}

// _EntEmitSound - Emit sound from entity
int Lua_EntEmitSound(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	const char *soundname = CLuaUtility::GetString(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntEmitSound: Invalid entity");

	pEntity->EmitSound(soundname);
	return 0;
}

// _EntFire - Fire an input on an entity
int Lua_EntFire(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	const char *inputName = CLuaUtility::GetString(L, 2);
	const char *value = CLuaUtility::GetString(L, 3, "");
	float delay = CLuaUtility::GetFloat(L, 4, 0.0f);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntFire: Invalid entity");

	variant_t varValue;
	varValue.SetString(MAKE_STRING(value));

	g_EventQueue.AddEvent(pEntity, inputName, varValue, delay, NULL, NULL);
	return 0;
}

// _EntSetMoveType - Set entity move type
int Lua_EntSetMoveType(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int moveType = CLuaUtility::GetInt(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetMoveType: Invalid entity");

	pEntity->SetMoveType((MoveType_t)moveType);
	return 0;
}

// _EntGetMoveType - Get entity move type
int Lua_EntGetMoveType(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetMoveType: Invalid entity");

	lua_pushinteger(L, pEntity->GetMoveType());
	return 1;
}

// _EntSetSolid - Set entity solid type
int Lua_EntSetSolid(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int solidType = CLuaUtility::GetInt(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetSolid: Invalid entity");

	pEntity->SetSolid((SolidType_t)solidType);
	return 0;
}

// _EntGetSolid - Get entity solid type
int Lua_EntGetSolid(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetSolid: Invalid entity");

	lua_pushinteger(L, pEntity->GetSolid());
	return 1;
}

// _EntSetCollisionGroup - Set entity collision group
int Lua_EntSetCollisionGroup(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int group = CLuaUtility::GetInt(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetCollisionGroup: Invalid entity");

	pEntity->SetCollisionGroup(group);
	return 0;
}

// _EntGetCollisionGroup - Get entity collision group
int Lua_EntGetCollisionGroup(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetCollisionGroup: Invalid entity");

	lua_pushinteger(L, pEntity->GetCollisionGroup());
	return 1;
}

// _EntSetParent - Set entity parent
int Lua_EntSetParent(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int parentIndex = CLuaUtility::GetInt(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	CBaseEntity *pParent = UTIL_EntityByIndex(parentIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetParent: Invalid entity");

	pEntity->SetParent(pParent);
	return 0;
}

// _EntGetParent - Get entity parent
int Lua_EntGetParent(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetParent: Invalid entity");

	CBaseEntity *pParent = pEntity->GetMoveParent();
	lua_pushinteger(L, pParent ? pParent->entindex() : -1);
	return 1;
}

// _EntSetMaxHealth - Set entity max health
int Lua_EntSetMaxHealth(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int maxHealth = CLuaUtility::GetInt(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetMaxHealth: Invalid entity");

	pEntity->SetMaxHealth(maxHealth);
	return 0;
}

// _EntGetMaxHealth - Get entity max health
int Lua_EntGetMaxHealth(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetMaxHealth: Invalid entity");

	lua_pushinteger(L, pEntity->GetMaxHealth());
	return 1;
}

// _EntGetWaterLevel - Get entity water level
int Lua_EntGetWaterLevel(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetWaterLevel: Invalid entity");

	lua_pushinteger(L, pEntity->GetWaterLevel());
	return 1;
}

// _EntSetGravity - Set entity gravity scale
int Lua_EntSetGravity(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	float gravity = CLuaUtility::GetFloat(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetGravity: Invalid entity");

	pEntity->SetGravity(gravity);
	return 0;
}

// _EntSetMaterial - Set entity render material
int Lua_EntSetMaterial(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	const char *material = CLuaUtility::GetString(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetMaterial: Invalid entity");

	// This would set a custom material on the entity
	// Implementation depends on specific material override system
	return 0;
}

// _EntSetActivity - Set entity activity (for animating entities)
int Lua_EntSetActivity(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int activity = CLuaUtility::GetInt(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetActivity: Invalid entity");

	// Try as NPC first
	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>(pEntity);
	if (pNPC)
	{
		pNPC->SetActivity((Activity)activity);
	}
	return 0;
}

// _EntEmitSoundEx - Extended sound emission
int Lua_EntEmitSoundEx(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	const char *soundName = CLuaUtility::GetString(L, 2);
	float volume = CLuaUtility::GetFloat(L, 3, 1.0f);
	int pitch = CLuaUtility::GetInt(L, 4, 100);
	int channel = CLuaUtility::GetInt(L, 5, CHAN_AUTO);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntEmitSoundEx: Invalid entity");

	if (!soundName || !*soundName)
		return CLuaUtility::LuaError(L, "_EntEmitSoundEx: Invalid sound name");

	Vector pos = pEntity->GetAbsOrigin();
	CPASAttenuationFilter filter(pos);
	CBaseEntity::EmitSound(filter, pEntity->entindex(), channel, soundName, volume, SNDLVL_NORM, 0, pitch, &pos);
	return 0;
}

// _EntSetMoveCollide - Set entity move collide type
int Lua_EntSetMoveCollide(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int moveCollide = CLuaUtility::GetInt(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetMoveCollide: Invalid entity");

	pEntity->SetMoveCollide((MoveCollide_t)moveCollide);
	return 0;
}

// _EntGetHealth - Get entity health
int Lua_EntGetHealth(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntGetHealth: Invalid entity");

	lua_pushinteger(L, pEntity->GetHealth());
	return 1;
}

// _EntSetHealth - Set entity health
int Lua_EntSetHealth(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int health = CLuaUtility::GetInt(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_EntSetHealth: Invalid entity");

	pEntity->SetHealth(health);
	return 0;
}

// _MakeDecal - Create a decal at position
int Lua_MakeDecal(lua_State *L)
{
	const char *decalName = CLuaUtility::GetString(L, 1);
	float x = CLuaUtility::GetFloat(L, 2);
	float y = CLuaUtility::GetFloat(L, 3);
	float z = CLuaUtility::GetFloat(L, 4);

	if (!decalName || !*decalName)
		return CLuaUtility::LuaError(L, "_MakeDecal: Invalid decal name");

	// Create decal using trace to find surface
	Vector start(x, y, z + 10);
	Vector end(x, y, z - 10);

	trace_t tr;
	UTIL_TraceLine(start, end, MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr);

	if (tr.DidHit())
	{
		UTIL_DecalTrace(&tr, decalName);
	}
	return 0;
}

// _EntityGetPhysicsAttacker - Get the physics attacker of an entity
int Lua_EntityGetPhysicsAttacker(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
	{
		lua_pushnil(L);
		return 1;
	}

	// HasPhysicsAttacker not available in HL2 beta
	// Would need to track physics throwers manually
	lua_pushnil(L);
	return 1;
}

// _EntGetDisposition - Get entity disposition towards another
int Lua_EntGetDisposition(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int targetIndex = CLuaUtility::GetInt(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	CBaseEntity *pTarget = UTIL_EntityByIndex(targetIndex);

	if (!pEntity)
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	// Try as NPC
	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>(pEntity);
	if (pNPC && pTarget)
	{
		lua_pushinteger(L, pNPC->IRelationType(pTarget));
	}
	else
	{
		lua_pushinteger(L, 0); // D_NU (neutral)
	}
	return 1;
}

//=============================================================================
// NPC FUNCTIONS (GMod 9 compatibility)
//=============================================================================

// _npc_ExitScriptedSequence - Exit from scripted sequence
// Syntax: <ent>
int Lua_NPC_ExitScriptedSequence(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_npc_ExitScriptedSequence: Invalid entity");

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>(pEntity);
	if (!pNPC)
		return CLuaUtility::LuaError(L, "_npc_ExitScriptedSequence: Entity is not an NPC");

	// Exit from any scripted sequence
	pNPC->ExitScriptedSequence();
	return 0;
}

// _npc_SetSchedule - Set NPC schedule
// Syntax: <ent> <sched>
int Lua_NPC_SetSchedule(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int scheduleID = CLuaUtility::GetInt(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_npc_SetSchedule: Invalid entity");

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>(pEntity);
	if (!pNPC)
		return CLuaUtility::LuaError(L, "_npc_SetSchedule: Entity is not an NPC");

	// Set the schedule by ID
	pNPC->SetSchedule(scheduleID);
	return 0;
}

// _npc_SetLastPosition - Set NPC last known position
// Syntax: <ent> <x> <y> <z>
int Lua_NPC_SetLastPosition(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	float x = CLuaUtility::GetFloat(L, 2);
	float y = CLuaUtility::GetFloat(L, 3);
	float z = CLuaUtility::GetFloat(L, 4);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_npc_SetLastPosition: Invalid entity");

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>(pEntity);
	if (!pNPC)
		return CLuaUtility::LuaError(L, "_npc_SetLastPosition: Entity is not an NPC");

	// SetLastPosition not available in HL2 beta AI_BaseNPC
	// Would need to use enemy memory system
	DevMsg("_npc_SetLastPosition: Not implemented in HL2 beta\n");
	return 0;
}

// _npc_AddRelationship - Add relationship between NPCs
// Syntax: <ent> <targetent> <disposition> <priority>
// Disposition: D_HT(1)=Hate, D_FR(2)=Fear, D_LI(3)=Like, D_NU(4)=Neutral
int Lua_NPC_AddRelationship(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	int targetIndex = CLuaUtility::GetInt(L, 2);
	int disposition = CLuaUtility::GetInt(L, 3);
	int priority = CLuaUtility::GetInt(L, 4);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	CBaseEntity *pTarget = UTIL_EntityByIndex(targetIndex);

	if (!pEntity)
		return CLuaUtility::LuaError(L, "_npc_AddRelationship: Invalid entity");
	if (!pTarget)
		return CLuaUtility::LuaError(L, "_npc_AddRelationship: Invalid target entity");

	CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>(pEntity);
	if (!pNPC)
		return CLuaUtility::LuaError(L, "_npc_AddRelationship: Entity is not an NPC");

	// Add entity relationship
	pNPC->AddEntityRelationship(pTarget, (Disposition_t)disposition, priority);
	return 0;
}

//=============================================================================
// ENTITY FINDING FUNCTIONS
//=============================================================================

// _EntitiesFindByClass - Find entities by classname
int Lua_EntitiesFindByClass(lua_State *L)
{
	const char *classname = CLuaUtility::GetString(L, 1);
	int startIndex = CLuaUtility::GetInt(L, 2, 0);

	CBaseEntity *pStart = startIndex > 0 ? UTIL_EntityByIndex(startIndex) : NULL;
	CBaseEntity *pEntity = gEntList.FindEntityByClassname(pStart, classname);

	if (pEntity)
		lua_pushinteger(L, pEntity->entindex());
	else
		lua_pushnil(L);
	return 1;
}

// _EntitiesFindByName - Find entities by name
int Lua_EntitiesFindByName(lua_State *L)
{
	const char *name = CLuaUtility::GetString(L, 1);
	int startIndex = CLuaUtility::GetInt(L, 2, 0);

	CBaseEntity *pStart = startIndex > 0 ? UTIL_EntityByIndex(startIndex) : NULL;
	CBaseEntity *pEntity = gEntList.FindEntityByName(pStart, name, NULL);

	if (pEntity)
		lua_pushinteger(L, pEntity->entindex());
	else
		lua_pushnil(L);
	return 1;
}

// _EntitiesFindInSphere - Find entities in sphere
int Lua_EntitiesFindInSphere(lua_State *L)
{
	float x = CLuaUtility::GetFloat(L, 1);
	float y = CLuaUtility::GetFloat(L, 2);
	float z = CLuaUtility::GetFloat(L, 3);
	float radius = CLuaUtility::GetFloat(L, 4);
	int startIndex = CLuaUtility::GetInt(L, 5, 0);

	Vector center(x, y, z);
	CBaseEntity *pStart = startIndex > 0 ? UTIL_EntityByIndex(startIndex) : NULL;
	CBaseEntity *pEntity = gEntList.FindEntityInSphere(pStart, center, radius);

	if (pEntity)
		lua_pushinteger(L, pEntity->entindex());
	else
		lua_pushnil(L);
	return 1;
}

//=============================================================================
// UTILITY FUNCTIONS (GMod 9 compatibility)
//=============================================================================

// _util_PlayerByName - Find player by name
// Syntax: <name>
int Lua_Util_PlayerByName(lua_State *L)
{
	const char *playerName = CLuaUtility::GetString(L, 1);
	if (!playerName)
		return CLuaUtility::LuaError(L, "_util_PlayerByName: Missing player name");

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			const char *name = STRING(pPlayer->pl.netname);
			if (name && Q_stricmp(name, playerName) == 0)
			{
				lua_pushinteger(L, pPlayer->entindex());
				return 1;
			}
		}
	}

	lua_pushnil(L);
	return 1;
}

// _util_PlayerByUserId - Find player by user ID
// Note: GetUserID not available in HL2 beta - uses entindex instead
// Syntax: <userid>
int Lua_Util_PlayerByUserId(lua_State *L)
{
	int userId = CLuaUtility::GetInt(L, 1);

	// GetUserID not available in HL2 beta - use entindex as fallback
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(userId);
	if (pPlayer)
	{
		lua_pushinteger(L, pPlayer->entindex());
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

// _util_EntsInBox - Find entities in box
// Syntax: <minx> <miny> <minz> <maxx> <maxy> <maxz>
int Lua_Util_EntsInBox(lua_State *L)
{
	float minX = CLuaUtility::GetFloat(L, 1);
	float minY = CLuaUtility::GetFloat(L, 2);
	float minZ = CLuaUtility::GetFloat(L, 3);
	float maxX = CLuaUtility::GetFloat(L, 4);
	float maxY = CLuaUtility::GetFloat(L, 5);
	float maxZ = CLuaUtility::GetFloat(L, 6);

	Vector mins(minX, minY, minZ);
	Vector maxs(maxX, maxY, maxZ);

	// Return as table of entity indices
	lua_newtable(L);
	int tableIndex = 1;

	CBaseEntity *pEntity = NULL;
	while ((pEntity = gEntList.FindEntityInSphere(pEntity, (mins + maxs) * 0.5f, (maxs - mins).Length() * 0.5f)) != NULL)
	{
		Vector pos = pEntity->GetAbsOrigin();
		if (pos.x >= mins.x && pos.x <= maxs.x &&
			pos.y >= mins.y && pos.y <= maxs.y &&
			pos.z >= mins.z && pos.z <= maxs.z)
		{
			lua_pushinteger(L, pEntity->entindex());
			lua_rawseti(L, -2, tableIndex++);
		}
	}

	return 1;
}

// _util_DropToFloor - Drop entity to floor
// Syntax: <ent>
int Lua_Util_DropToFloor(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	if (!pEntity)
		return CLuaUtility::LuaError(L, "_util_DropToFloor: Invalid entity");

	UTIL_DropToFloor(pEntity, MASK_SOLID);
	return 0;
}

// _util_ScreenShake - Shake screen for all players
// Syntax: <x> <y> <z> <amplitude> <frequency> <duration> <radius>
int Lua_Util_ScreenShake(lua_State *L)
{
	float x = CLuaUtility::GetFloat(L, 1);
	float y = CLuaUtility::GetFloat(L, 2);
	float z = CLuaUtility::GetFloat(L, 3);
	float amplitude = CLuaUtility::GetFloat(L, 4);
	float frequency = CLuaUtility::GetFloat(L, 5);
	float duration = CLuaUtility::GetFloat(L, 6);
	float radius = CLuaUtility::GetFloat(L, 7, 0.0f);

	Vector center(x, y, z);
	UTIL_ScreenShake(center, amplitude, frequency, duration, radius, SHAKE_START, false);
	return 0;
}

// _util_PointAtEntity - Check if there's an entity at the point
// Syntax: <x> <y> <z>
int Lua_Util_PointAtEntity(lua_State *L)
{
	float x = CLuaUtility::GetFloat(L, 1);
	float y = CLuaUtility::GetFloat(L, 2);
	float z = CLuaUtility::GetFloat(L, 3);

	Vector point(x, y, z);
	CBaseEntity *pEntity = gEntList.FindEntityInSphere(NULL, point, 1.0f);
	if (pEntity)
		lua_pushinteger(L, pEntity->entindex());
	else
		lua_pushnil(L);
	return 1;
}

// _GetNextMap - Get next map name
// Note: nextlevel cvar not available in HL2 beta
int Lua_GetNextMap(lua_State *L)
{
	// nextlevel cvar not available in HL2 beta - return empty string
	lua_pushstring(L, "");
	return 1;
}

// _GetModPath - Get the path to the mod folder
// Note: GetSearchPath not available in HL2 beta IFileSystem
int Lua_GetModPath(lua_State *L)
{
	// GetSearchPath not available in HL2 beta
	// Return default "bmod" as the mod path
	lua_pushstring(L, "bmod");
	return 1;
}

// _IsLinux - Check if running on Linux
int Lua_IsLinux(lua_State *L)
{
#ifdef LINUX
	lua_pushboolean(L, true);
#else
	lua_pushboolean(L, false);
#endif
	return 1;
}

// _IsDedicatedServer - Check if this is a dedicated server
int Lua_IsDedicatedServer(lua_State *L)
{
	lua_pushboolean(L, engine->IsDedicatedServer());
	return 1;
}

// _GetCurrentMap - Get current map name
int Lua_GetCurrentMap(lua_State *L)
{
	lua_pushstring(L, STRING(gpGlobals->mapname));
	return 1;
}

//=============================================================================
// REGISTRATION
//=============================================================================

void RegisterLuaEntityFunctions()
{
	// Position/Angles
	CLuaIntegration::RegisterFunction("_EntGetPos", Lua_EntGetPos, "Returns entity position as x,y,z. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntSetPos", Lua_EntSetPos, "Set entity position. Syntax: <entindex> <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_EntGetAng", Lua_EntGetAng, "Returns entity angles as pitch,yaw,roll. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntSetAng", Lua_EntSetAng, "Set entity angles. Syntax: <entindex> <pitch> <yaw> <roll>");
	CLuaIntegration::RegisterFunction("_EntGetVelocity", Lua_EntGetVelocity, "Returns entity velocity as x,y,z. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntSetVelocity", Lua_EntSetVelocity, "Set entity velocity. Syntax: <entindex> <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_EntGetForwardVector", Lua_EntGetForwardVector, "Returns entity forward vector. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntGetRightVector", Lua_EntGetRightVector, "Returns entity right vector. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntGetUpVector", Lua_EntGetUpVector, "Returns entity up vector. Syntax: <entindex>");

	// Creation/Destruction
	CLuaIntegration::RegisterFunction("_EntCreate", Lua_EntCreate, "Create entity by classname. Syntax: <classname>");
	CLuaIntegration::RegisterFunction("_EntSpawn", Lua_EntSpawn, "Spawn entity. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntActivate", Lua_EntActivate, "Activate entity. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntRemove", Lua_EntRemove_New, "Remove entity. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntExists", Lua_EntExists, "Check if entity exists. Syntax: <entindex>");

	// Properties
	CLuaIntegration::RegisterFunction("_EntSetKeyValue", Lua_EntSetKeyValue, "Set entity keyvalue. Syntax: <entindex> <key> <value>");
	CLuaIntegration::RegisterFunction("_EntSetModel", Lua_EntSetModel, "Set entity model. Syntax: <entindex> <modelpath>");
	CLuaIntegration::RegisterFunction("_EntGetModel", Lua_EntGetModel, "Get entity model path. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntPrecacheModel", Lua_EntPrecacheModel, "Precache a model. Syntax: <modelpath>");
	CLuaIntegration::RegisterFunction("_EntSetOwner", Lua_EntSetOwner, "Set entity owner. Syntax: <entindex> <ownerindex>");
	CLuaIntegration::RegisterFunction("_EntGetOwner", Lua_EntGetOwner, "Get entity owner. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntGetType", Lua_EntGetType, "Get entity classname. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntGetByName", Lua_EntGetByName, "Get entity by targetname. Syntax: <name>");
	CLuaIntegration::RegisterFunction("_EntGetName", Lua_EntGetName, "Get entity targetname. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntSetName", Lua_EntSetName, "Set entity targetname. Syntax: <entindex> <name>");

	// Physics properties
	CLuaIntegration::RegisterFunction("_EntSetMoveType", Lua_EntSetMoveType, "Set entity move type. Syntax: <entindex> <movetype>");
	CLuaIntegration::RegisterFunction("_EntGetMoveType", Lua_EntGetMoveType, "Get entity move type. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntSetSolid", Lua_EntSetSolid, "Set entity solid type. Syntax: <entindex> <solidtype>");
	CLuaIntegration::RegisterFunction("_EntGetSolid", Lua_EntGetSolid, "Get entity solid type. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntSetCollisionGroup", Lua_EntSetCollisionGroup, "Set entity collision group. Syntax: <entindex> <group>");
	CLuaIntegration::RegisterFunction("_EntGetCollisionGroup", Lua_EntGetCollisionGroup, "Get entity collision group. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntSetGravity", Lua_EntSetGravity, "Set entity gravity scale. Syntax: <entindex> <scale>");
	CLuaIntegration::RegisterFunction("_EntSetMaterial", Lua_EntSetMaterial, "Set entity material. Syntax: <entindex> <materialpath>");

	// Hierarchy
	CLuaIntegration::RegisterFunction("_EntSetParent", Lua_EntSetParent, "Set entity parent. Syntax: <entindex> <parentindex>");
	CLuaIntegration::RegisterFunction("_EntGetParent", Lua_EntGetParent, "Get entity parent index. Syntax: <entindex>");

	// Health
	CLuaIntegration::RegisterFunction("_EntSetMaxHealth", Lua_EntSetMaxHealth, "Set entity max health. Syntax: <entindex> <maxhealth>");
	CLuaIntegration::RegisterFunction("_EntGetMaxHealth", Lua_EntGetMaxHealth, "Get entity max health. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntGetHealth", Lua_EntGetHealth, "Get entity health. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntSetHealth", Lua_EntSetHealth, "Set entity health. Syntax: <entindex> <health>");
	CLuaIntegration::RegisterFunction("_EntGetWaterLevel", Lua_EntGetWaterLevel, "Get entity water level. Syntax: <entindex>");

	// Sounds/Effects
	CLuaIntegration::RegisterFunction("_EntEmitSound", Lua_EntEmitSound, "Emit sound from entity. Syntax: <entindex> <soundname>");
	CLuaIntegration::RegisterFunction("_EntEmitSoundEx", Lua_EntEmitSoundEx, "Emit sound with params. Syntax: <entindex> <sound> [vol] [pitch] [channel]");
	CLuaIntegration::RegisterFunction("_EntFire", Lua_EntFire, "Fire input on entity. Syntax: <entindex> <input> [value] [delay]");

	// Activity/Animation
	CLuaIntegration::RegisterFunction("_EntSetActivity", Lua_EntSetActivity, "Set entity activity. Syntax: <entindex> <activity>");
	CLuaIntegration::RegisterFunction("_EntSetMoveCollide", Lua_EntSetMoveCollide, "Set move collide type. Syntax: <entindex> <type>");

	// Misc
	CLuaIntegration::RegisterFunction("_EntityGetPhysicsAttacker", Lua_EntityGetPhysicsAttacker, "Get physics attacker. Syntax: <entindex>");
	CLuaIntegration::RegisterFunction("_EntGetDisposition", Lua_EntGetDisposition, "Get NPC disposition. Syntax: <entindex> <targetindex>");
	CLuaIntegration::RegisterFunction("_MakeDecal", Lua_MakeDecal, "Create decal. Syntax: <decalname> <x> <y> <z>");

	// Finding
	CLuaIntegration::RegisterFunction("_EntitiesFindByClass", Lua_EntitiesFindByClass, "Find entity by classname. Syntax: <classname> [startindex]");
	CLuaIntegration::RegisterFunction("_EntitiesFindByName", Lua_EntitiesFindByName, "Find entity by name. Syntax: <name> [startindex]");
	CLuaIntegration::RegisterFunction("_EntitiesFindInSphere", Lua_EntitiesFindInSphere, "Find entity in sphere. Syntax: <x> <y> <z> <radius> [startindex]");

	// NPC Functions (GMod 9 compatibility)
	CLuaIntegration::RegisterFunction("_npc_ExitScriptedSequence", Lua_NPC_ExitScriptedSequence, "Exit from scripted sequence. Syntax: <ent>");
	CLuaIntegration::RegisterFunction("_npc_SetSchedule", Lua_NPC_SetSchedule, "Set NPC schedule. Syntax: <ent> <sched>");
	CLuaIntegration::RegisterFunction("_npc_SetLastPosition", Lua_NPC_SetLastPosition, "Set NPC last position. Syntax: <ent> <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_npc_AddRelationship", Lua_NPC_AddRelationship, "Add NPC relationship. Syntax: <ent> <targetent> <disposition> <priority>");

	// Utility Functions (GMod 9 compatibility)
	CLuaIntegration::RegisterFunction("_util_PlayerByName", Lua_Util_PlayerByName, "Find player by name. Syntax: <name>");
	CLuaIntegration::RegisterFunction("_util_PlayerByUserId", Lua_Util_PlayerByUserId, "Find player by userid. Syntax: <userid>");
	CLuaIntegration::RegisterFunction("_util_EntsInBox", Lua_Util_EntsInBox, "Find entities in box. Syntax: <minx> <miny> <minz> <maxx> <maxy> <maxz>");
	CLuaIntegration::RegisterFunction("_util_DropToFloor", Lua_Util_DropToFloor, "Drop entity to floor. Syntax: <ent>");
	CLuaIntegration::RegisterFunction("_util_ScreenShake", Lua_Util_ScreenShake, "Shake screen. Syntax: <x> <y> <z> <amp> <freq> <dur> [radius]");
	CLuaIntegration::RegisterFunction("_util_PointAtEntity", Lua_Util_PointAtEntity, "Find entity at point. Syntax: <x> <y> <z>");

	// System Utility Functions
	CLuaIntegration::RegisterFunction("_GetNextMap", Lua_GetNextMap, "Get next map name.");
	CLuaIntegration::RegisterFunction("_GetCurrentMap", Lua_GetCurrentMap, "Get current map name.");
	CLuaIntegration::RegisterFunction("_GetModPath", Lua_GetModPath, "Get mod folder path.");
	CLuaIntegration::RegisterFunction("_IsLinux", Lua_IsLinux, "Check if running on Linux.");
	CLuaIntegration::RegisterFunction("_IsDedicatedServer", Lua_IsDedicatedServer, "Check if dedicated server.");
}
