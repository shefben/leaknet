//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Lua physics and trace functions for BarrysMod
// Based on reverse engineering of Garry's Mod 9.0.4b server.dll
//
//=============================================================================//

#include "cbase.h"
#include "lua_integration.h"
#include "player.h"
#include "physics.h"
#include "vphysics_interface.h"
#include "physics_saverestore.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// PHYSICS FUNCTIONS
//=============================================================================

// Helper to get physics object from entity
static IPhysicsObject* GetPhysicsObject(CBaseEntity *pEntity)
{
	if (!pEntity)
		return NULL;
	return pEntity->VPhysicsGetObject();
}

// _phys_EnableMotion - Enable/disable physics motion
int Lua_PhysEnableMotion(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	bool enable = CLuaUtility::GetBool(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_EnableMotion: Entity has no physics");

	pPhys->EnableMotion(enable);
	return 0;
}

// _phys_EnableGravity - Enable/disable gravity on physics object
int Lua_PhysEnableGravity(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	bool enable = CLuaUtility::GetBool(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_EnableGravity: Entity has no physics");

	pPhys->EnableGravity(enable);
	return 0;
}

// _phys_EnableDrag - Enable/disable drag on physics object
int Lua_PhysEnableDrag(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	bool enable = CLuaUtility::GetBool(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_EnableDrag: Entity has no physics");

	pPhys->EnableDrag(enable);
	return 0;
}

// _phys_EnableCollisions - Enable/disable collisions
int Lua_PhysEnableCollisions(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	bool enable = CLuaUtility::GetBool(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_EnableCollisions: Entity has no physics");

	pPhys->EnableCollisions(enable);
	return 0;
}

// _phys_GetMass - Get mass of physics object
int Lua_PhysGetMass(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
	{
		lua_pushnumber(L, 0);
		return 1;
	}

	lua_pushnumber(L, pPhys->GetMass());
	return 1;
}

// _phys_SetMass - Set mass of physics object
int Lua_PhysSetMass(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	float mass = CLuaUtility::GetFloat(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_SetMass: Entity has no physics");

	pPhys->SetMass(mass);
	return 0;
}

// _phys_Sleep - Put physics object to sleep
int Lua_PhysSleep(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_Sleep: Entity has no physics");

	pPhys->Sleep();
	return 0;
}

// _phys_Wake - Wake physics object
int Lua_PhysWake(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_Wake: Entity has no physics");

	pPhys->Wake();
	return 0;
}

// _phys_IsAsleep - Check if physics object is asleep
int Lua_PhysIsAsleep(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
	{
		lua_pushboolean(L, true);
		return 1;
	}

	lua_pushboolean(L, pPhys->IsAsleep());
	return 1;
}

// _phys_HasPhysics - Check if entity has physics
int Lua_PhysHasPhysics(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	lua_pushboolean(L, GetPhysicsObject(pEntity) != NULL);
	return 1;
}

// _phys_ApplyForceCenter - Apply force at center
int Lua_PhysApplyForceCenter(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	float x = CLuaUtility::GetFloat(L, 2);
	float y = CLuaUtility::GetFloat(L, 3);
	float z = CLuaUtility::GetFloat(L, 4);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_ApplyForceCenter: Entity has no physics");

	pPhys->ApplyForceCenter(Vector(x, y, z));
	return 0;
}

// _phys_ApplyForceOffset - Apply force at offset
int Lua_PhysApplyForceOffset(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	float fx = CLuaUtility::GetFloat(L, 2);
	float fy = CLuaUtility::GetFloat(L, 3);
	float fz = CLuaUtility::GetFloat(L, 4);
	float ox = CLuaUtility::GetFloat(L, 5);
	float oy = CLuaUtility::GetFloat(L, 6);
	float oz = CLuaUtility::GetFloat(L, 7);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_ApplyForceOffset: Entity has no physics");

	pPhys->ApplyForceOffset(Vector(fx, fy, fz), Vector(ox, oy, oz));
	return 0;
}

// _phys_ApplyTorqueCenter - Apply torque at center
int Lua_PhysApplyTorqueCenter(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	float x = CLuaUtility::GetFloat(L, 2);
	float y = CLuaUtility::GetFloat(L, 3);
	float z = CLuaUtility::GetFloat(L, 4);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_ApplyTorqueCenter: Entity has no physics");

	pPhys->ApplyTorqueCenter(AngularImpulse(x, y, z));
	return 0;
}

//=============================================================================
// TRACE FUNCTIONS
//=============================================================================

// Global trace result storage for Lua trace functions
static trace_t g_LuaTrace;
static int g_LuaTraceCollisionGroup = COLLISION_GROUP_NONE;
static unsigned int g_LuaTraceMask = MASK_SOLID;

// _TraceSetCollisionGroup - Set trace collision group
int Lua_TraceSetCollisionGroup(lua_State *L)
{
	g_LuaTraceCollisionGroup = CLuaUtility::GetInt(L, 1);
	return 0;
}

// _TraceSetMask - Set trace mask
int Lua_TraceSetMask(lua_State *L)
{
	g_LuaTraceMask = CLuaUtility::GetInt(L, 1);
	return 0;
}

// _TraceLine - Perform a line trace
int Lua_TraceLine(lua_State *L)
{
	float x1 = CLuaUtility::GetFloat(L, 1);
	float y1 = CLuaUtility::GetFloat(L, 2);
	float z1 = CLuaUtility::GetFloat(L, 3);
	float x2 = CLuaUtility::GetFloat(L, 4);
	float y2 = CLuaUtility::GetFloat(L, 5);
	float z2 = CLuaUtility::GetFloat(L, 6);
	int ignoreEnt = CLuaUtility::GetInt(L, 7, -1);

	Vector start(x1, y1, z1);
	Vector end(x2, y2, z2);

	CBaseEntity *pIgnore = ignoreEnt > 0 ? UTIL_EntityByIndex(ignoreEnt) : NULL;

	CTraceFilterSimple filter(pIgnore, g_LuaTraceCollisionGroup);
	UTIL_TraceLine(start, end, g_LuaTraceMask, &filter, &g_LuaTrace);

	lua_pushboolean(L, g_LuaTrace.DidHit());
	return 1;
}

// _TraceEndPos - Get end position of last trace
int Lua_TraceEndPos(lua_State *L)
{
	lua_pushnumber(L, g_LuaTrace.endpos.x);
	lua_pushnumber(L, g_LuaTrace.endpos.y);
	lua_pushnumber(L, g_LuaTrace.endpos.z);
	return 3;
}

// _TraceFraction - Get fraction of last trace
int Lua_TraceFraction(lua_State *L)
{
	lua_pushnumber(L, g_LuaTrace.fraction);
	return 1;
}

// _TraceHitWorld - Check if last trace hit world
int Lua_TraceHitWorld(lua_State *L)
{
	lua_pushboolean(L, g_LuaTrace.DidHitWorld());
	return 1;
}

// _TraceHitNonWorld - Check if last trace hit non-world entity
int Lua_TraceHitNonWorld(lua_State *L)
{
	lua_pushboolean(L, g_LuaTrace.DidHitNonWorldEntity());
	return 1;
}

// _TraceHit - Check if last trace hit anything
int Lua_TraceHit(lua_State *L)
{
	lua_pushboolean(L, g_LuaTrace.DidHit());
	return 1;
}

// _TraceGetEnt - Get entity hit by last trace
int Lua_TraceGetEnt(lua_State *L)
{
	if (g_LuaTrace.m_pEnt)
		lua_pushinteger(L, g_LuaTrace.m_pEnt->entindex());
	else
		lua_pushnil(L);
	return 1;
}

// _TraceGetSurfaceNormal - Get surface normal from last trace
int Lua_TraceGetSurfaceNormal(lua_State *L)
{
	lua_pushnumber(L, g_LuaTrace.plane.normal.x);
	lua_pushnumber(L, g_LuaTrace.plane.normal.y);
	lua_pushnumber(L, g_LuaTrace.plane.normal.z);
	return 3;
}

// _TraceDidHitSky - Check if trace hit sky
int Lua_TraceDidHitSky(lua_State *L)
{
	lua_pushboolean(L, (g_LuaTrace.surface.flags & SURF_SKY) != 0);
	return 1;
}

// _TraceDidHitWater - Check if trace hit water
int Lua_TraceDidHitWater(lua_State *L)
{
	lua_pushboolean(L, (g_LuaTrace.contents & CONTENTS_WATER) != 0);
	return 1;
}

// _TraceGetTexture - Get texture name from trace
int Lua_TraceGetTexture(lua_State *L)
{
	lua_pushstring(L, g_LuaTrace.surface.name);
	return 1;
}

//=============================================================================
// REGISTRATION
//=============================================================================

void RegisterLuaPhysicsFunctions()
{
	// Physics control (original names)
	CLuaIntegration::RegisterFunction("_phys_EnableMotion", Lua_PhysEnableMotion, "Enable/disable motion. Syntax: <entid> <bool>");
	CLuaIntegration::RegisterFunction("_phys_EnableGravity", Lua_PhysEnableGravity, "Enable/disable gravity. Syntax: <entid> <bool>");
	CLuaIntegration::RegisterFunction("_phys_EnableDrag", Lua_PhysEnableDrag, "Enable/disable drag. Syntax: <entid> <bool>");
	CLuaIntegration::RegisterFunction("_phys_EnableCollisions", Lua_PhysEnableCollisions, "Enable/disable collisions. Syntax: <entid> <bool>");

	// Physics properties (original names)
	CLuaIntegration::RegisterFunction("_phys_GetMass", Lua_PhysGetMass, "Get mass. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_phys_SetMass", Lua_PhysSetMass, "Set mass. Syntax: <entid> <mass>");
	CLuaIntegration::RegisterFunction("_phys_Sleep", Lua_PhysSleep, "Put physics to sleep. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_phys_Wake", Lua_PhysWake, "Wake physics. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_phys_IsAsleep", Lua_PhysIsAsleep, "Check if asleep. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_phys_HasPhysics", Lua_PhysHasPhysics, "Check if has physics. Syntax: <entid>");

	// Physics forces (original names)
	CLuaIntegration::RegisterFunction("_phys_ApplyForceCenter", Lua_PhysApplyForceCenter, "Apply force at center. Syntax: <entid> <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_phys_ApplyForceOffset", Lua_PhysApplyForceOffset, "Apply force at offset. Syntax: <entid> <fx> <fy> <fz> <ox> <oy> <oz>");
	CLuaIntegration::RegisterFunction("_phys_ApplyTorqueCenter", Lua_PhysApplyTorqueCenter, "Apply torque. Syntax: <entid> <x> <y> <z>");

	// GMod-style physics aliases (for script compatibility)
	CLuaIntegration::RegisterFunction("_PhysEnableMotion", Lua_PhysEnableMotion, "Enable/disable motion. Syntax: <entid> <bool>");
	CLuaIntegration::RegisterFunction("_PhysEnableGravity", Lua_PhysEnableGravity, "Enable/disable gravity. Syntax: <entid> <bool>");
	CLuaIntegration::RegisterFunction("_PhysGetMass", Lua_PhysGetMass, "Get mass. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_PhysSetMass", Lua_PhysSetMass, "Set mass. Syntax: <entid> <mass>");
	CLuaIntegration::RegisterFunction("_PhysWake", Lua_PhysWake, "Wake physics. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_PhysSleep", Lua_PhysSleep, "Put physics to sleep. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_PhysApplyForce", Lua_PhysApplyForceCenter, "Apply force at center. Syntax: <entid> <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_PhysApplyForceCenter", Lua_PhysApplyForceCenter, "Apply force at center. Syntax: <entid> <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_PhysApplyForceOffset", Lua_PhysApplyForceOffset, "Apply force at offset. Syntax: <entid> <fx> <fy> <fz> <ox> <oy> <oz>");
	CLuaIntegration::RegisterFunction("_PhysApplyTorque", Lua_PhysApplyTorqueCenter, "Apply torque. Syntax: <entid> <x> <y> <z>");
	CLuaIntegration::RegisterFunction("_PhysHasPhysics", Lua_PhysHasPhysics, "Check if has physics. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_PhysIsAsleep", Lua_PhysIsAsleep, "Check if asleep. Syntax: <entid>");

	// Trace functions
	CLuaIntegration::RegisterFunction("_TraceSetCollisionGroup", Lua_TraceSetCollisionGroup, "Set trace collision group. Syntax: <group>");
	CLuaIntegration::RegisterFunction("_TraceSetMask", Lua_TraceSetMask, "Set trace mask. Syntax: <mask>");
	CLuaIntegration::RegisterFunction("_TraceLine", Lua_TraceLine, "Trace a line. Syntax: <x1> <y1> <z1> <x2> <y2> <z2> [ignoreent]");
	CLuaIntegration::RegisterFunction("_TraceEndPos", Lua_TraceEndPos, "Get trace end position.");
	CLuaIntegration::RegisterFunction("_TraceFraction", Lua_TraceFraction, "Get trace fraction (0-1).");
	CLuaIntegration::RegisterFunction("_TraceHitWorld", Lua_TraceHitWorld, "Check if trace hit world.");
	CLuaIntegration::RegisterFunction("_TraceHitNonWorld", Lua_TraceHitNonWorld, "Check if trace hit entity.");
	CLuaIntegration::RegisterFunction("_TraceHit", Lua_TraceHit, "Check if trace hit anything.");
	CLuaIntegration::RegisterFunction("_TraceGetEnt", Lua_TraceGetEnt, "Get entity hit by trace.");
	CLuaIntegration::RegisterFunction("_TraceGetSurfaceNormal", Lua_TraceGetSurfaceNormal, "Get surface normal from trace.");
	CLuaIntegration::RegisterFunction("_TraceDidHitSky", Lua_TraceDidHitSky, "Check if trace hit sky.");
	CLuaIntegration::RegisterFunction("_TraceDidHitWater", Lua_TraceDidHitWater, "Check if trace hit water.");
	CLuaIntegration::RegisterFunction("_TraceGetTexture", Lua_TraceGetTexture, "Get texture name from trace.");
}
