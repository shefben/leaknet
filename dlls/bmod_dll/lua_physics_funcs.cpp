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

static bool LuaGetVector(lua_State *L, int index, Vector& vec)
{
	if (lua_istable(L, index))
	{
		lua_getfield(L, index, "x");
		lua_getfield(L, index, "y");
		lua_getfield(L, index, "z");

		vec.x = (float)lua_tonumber(L, -3);
		vec.y = (float)lua_tonumber(L, -2);
		vec.z = (float)lua_tonumber(L, -1);

		lua_pop(L, 3);
		return true;
	}

	if (lua_gettop(L) >= index + 2 && lua_isnumber(L, index))
	{
		vec.x = CLuaUtility::GetFloat(L, index);
		vec.y = CLuaUtility::GetFloat(L, index + 1);
		vec.z = CLuaUtility::GetFloat(L, index + 2);
		return true;
	}

	return false;
}

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

// _phys_EnableMotion - Enable/disable physics motion
int Lua_PhysEnableMotion(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	bool enable = CLuaUtility::GetBool(L, 2);

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_EnableMotion: Entity has no physics");

	// TEMP diagnostic: this is the RaceStart un-freeze path; log it so we can confirm it runs.
	DevMsg( "GMod Phys EnableMotion(_Phys): ent %d enable=%d (was moveable=%d)\n",
		entIndex, enable ? 1 : 0, pPhys->IsMoveable() ? 1 : 0 );
	pPhys->EnableMotion(enable);
	// Mirror the engine's CPhysicsProp::EnableMotion(): a frozen (pinned) object
	// falls asleep, and a sleeping object silently discards async forces. Wake it on
	// unfreeze so DoControls' ApplyForceCenter can actually roll the melon.
	if (enable)
		pPhys->Wake();
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
	Vector force;

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_ApplyForceCenter: Entity has no physics");

	if (!LuaGetVector(L, 2, force))
		return CLuaUtility::LuaError(L, "_phys_ApplyForceCenter: Expected vector3 or x,y,z");

	static int s_nLoggedApplyForceCenter = 0;
	if (s_nLoggedApplyForceCenter < 16)
	{
		DevMsg("GMod Lua: _phys.ApplyForceCenter ent %d force (%.1f %.1f %.1f)\n",
			entIndex, force.x, force.y, force.z);
		++s_nLoggedApplyForceCenter;
	}

	pPhys->ApplyForceCenter(force);
	return 0;
}

// _phys_ApplyForceOffset - Apply force at offset
int Lua_PhysApplyForceOffset(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	Vector force;
	Vector offset;

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_ApplyForceOffset: Entity has no physics");

	if (lua_istable(L, 2))
	{
		if (!LuaGetVector(L, 2, force) || !LuaGetVector(L, 3, offset))
			return CLuaUtility::LuaError(L, "_phys_ApplyForceOffset: Expected force and offset vectors");
	}
	else
	{
		if (!LuaGetVector(L, 2, force) || !LuaGetVector(L, 5, offset))
			return CLuaUtility::LuaError(L, "_phys_ApplyForceOffset: Expected force and offset vectors");
	}

	pPhys->ApplyForceOffset(force, offset);
	return 0;
}

// _phys_ApplyTorqueCenter - Apply torque at center
int Lua_PhysApplyTorqueCenter(lua_State *L)
{
	int entIndex = CLuaUtility::GetInt(L, 1);
	Vector torque;

	CBaseEntity *pEntity = UTIL_EntityByIndex(entIndex);
	IPhysicsObject *pPhys = GetPhysicsObject(pEntity);
	if (!pPhys)
		return CLuaUtility::LuaError(L, "_phys_ApplyTorqueCenter: Entity has no physics");

	if (!LuaGetVector(L, 2, torque))
		return CLuaUtility::LuaError(L, "_phys_ApplyTorqueCenter: Expected vector3 or x,y,z");

	pPhys->ApplyTorqueCenter(AngularImpulse(torque.x, torque.y, torque.z));
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
	Vector start;
	Vector end;
	int ignoreEnt = -1;

	if (lua_istable(L, 1))
	{
		Vector direction;
		if (!LuaGetVector(L, 1, start) || !LuaGetVector(L, 2, direction))
			return CLuaUtility::LuaError(L, "_TraceLine: Expected start and direction vectors");

		float distance = CLuaUtility::GetFloat(L, 3, 0.0f);
		end = start + (direction * distance);
		ignoreEnt = CLuaUtility::GetInt(L, 4, -1);
	}
	else
	{
		if (!LuaGetVector(L, 1, start) || !LuaGetVector(L, 4, end))
			return CLuaUtility::LuaError(L, "_TraceLine: Expected vector/distance or start/end coordinates");

		ignoreEnt = CLuaUtility::GetInt(L, 7, -1);
	}

	CBaseEntity *pIgnore = ignoreEnt > 0 ? UTIL_EntityByIndex(ignoreEnt) : NULL;

	CTraceFilterSimple filter(pIgnore, g_LuaTraceCollisionGroup);
	UTIL_TraceLine(start, end, g_LuaTraceMask, &filter, &g_LuaTrace);

	lua_pushboolean(L, g_LuaTrace.DidHit());
	return 1;
}

// _TraceEndPos - Get end position of last trace
int Lua_TraceEndPos(lua_State *L)
{
	LuaPushVector(L, g_LuaTrace.endpos);
	return 1;
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

// _TraceDidHitHitbox - whether the last trace hit a model hitbox (original checks the hitbox bit).
int Lua_TraceDidHitHitbox(lua_State *L)
{
	lua_pushboolean(L, g_LuaTrace.hitbox != 0);
	return 1;
}

// _TraceAttack - apply the last trace as an attack. bmod does not run the original's damage
// pipeline here; the function is registered so scripts do not nil-call. (Unused by shipped content.)
int Lua_TraceAttack(lua_State *L)
{
	return 0;
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
	LuaPushVector(L, g_LuaTrace.plane.normal);
	return 1;
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

	// Physics properties (original names)

	// Physics forces (original names)

	// GMod-style physics aliases (for script compatibility)
	CLuaIntegration::RegisterFunction("_PhysEnableMotion", Lua_PhysEnableMotion, "Enable/disable motion. Syntax: <entid> <bool>");
	CLuaIntegration::RegisterFunction("_PhysEnableGravity", Lua_PhysEnableGravity, "Enable/disable gravity. Syntax: <entid> <bool>");
	CLuaIntegration::RegisterFunction("_PhysGetMass", Lua_PhysGetMass, "Get mass. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_PhysSetMass", Lua_PhysSetMass, "Set mass. Syntax: <entid> <mass>");
	CLuaIntegration::RegisterFunction("_PhysWake", Lua_PhysWake, "Wake physics. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_PhysSleep", Lua_PhysSleep, "Put physics to sleep. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_PhysApplyForce", Lua_PhysApplyForceCenter, "Apply force at center. Syntax: <entid> <vector3|x y z>");
	CLuaIntegration::RegisterFunction("_PhysApplyForceCenter", Lua_PhysApplyForceCenter, "Apply force at center. Syntax: <entid> <vector3|x y z>");
	CLuaIntegration::RegisterFunction("_PhysApplyForceOffset", Lua_PhysApplyForceOffset, "Apply force at offset. Syntax: <entid> <force vector3|fx fy fz> <offset vector3|ox oy oz>");
	CLuaIntegration::RegisterFunction("_PhysApplyTorque", Lua_PhysApplyTorqueCenter, "Apply torque. Syntax: <entid> <vector3|x y z>");
	CLuaIntegration::RegisterFunction("_PhysHasPhysics", Lua_PhysHasPhysics, "Check if has physics. Syntax: <entid>");
	CLuaIntegration::RegisterFunction("_PhysIsAsleep", Lua_PhysIsAsleep, "Check if asleep. Syntax: <entid>");

	// Trace functions
	CLuaIntegration::RegisterFunction("_TraceSetCollisionGroup", Lua_TraceSetCollisionGroup, "Sets the collision group for the next trace to use. Syntax <group>");
	CLuaIntegration::RegisterFunction("_TraceSetMask", Lua_TraceSetMask, "Sets the MASK_ to use for the next trace. Syntax <group>");
	CLuaIntegration::RegisterFunction("_TraceLine", Lua_TraceLine, "Traces a line. Syntax <vector start> <vector direction> <length> <ignore (optional)>");
	CLuaIntegration::RegisterFunction("_TraceEndPos", Lua_TraceEndPos, "Return the endpos from the last trace");
	CLuaIntegration::RegisterFunction("_TraceFraction", Lua_TraceFraction, "Return the fraction of trace completed");
	CLuaIntegration::RegisterFunction("_TraceHitWorld", Lua_TraceHitWorld, "Return the true if the last trace hit the world");
	CLuaIntegration::RegisterFunction("_TraceDidHitHitbox", Lua_TraceDidHitHitbox, "Returns true if last hit was some kind of hitbox");
	CLuaIntegration::RegisterFunction("_TraceAttack", Lua_TraceAttack, "Attack using the last trace. Syntax: <victim> <inflictor ent> <attacker ent> <amount>");
	CLuaIntegration::RegisterFunction("_TraceHitNonWorld", Lua_TraceHitNonWorld, "Return the true if the last trace hit non world");
	CLuaIntegration::RegisterFunction("_TraceHit", Lua_TraceHit, "Return the true if the last trace hit something");
	CLuaIntegration::RegisterFunction("_TraceGetEnt", Lua_TraceGetEnt, "Return the entity that the last trace hit");
	CLuaIntegration::RegisterFunction("_TraceGetSurfaceNormal", Lua_TraceGetSurfaceNormal, "Return the normal of the surface that the trace hit");
	CLuaIntegration::RegisterFunction("_TraceDidHitSky", Lua_TraceDidHitSky, "Returns true if last hit was the sky");
	CLuaIntegration::RegisterFunction("_TraceDidHitWater", Lua_TraceDidHitWater, "Returns true if we hit water");
	CLuaIntegration::RegisterFunction("_TraceGetTexture", Lua_TraceGetTexture, "Returns the name of the texture that we hit");
}
