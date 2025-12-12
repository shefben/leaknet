//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GMod player spawn point entity
// Based on IDA reverse engineering of Garry's Mod 9.0.4b server.dll
//
// gmod_player_start is used by GMod gamemodes for team-based spawning.
// Uses spawnflags to determine which teams can spawn at this point:
//   Flag 1 (0x01): Team 1 can spawn here
//   Flag 2 (0x02): Team 2 can spawn here
//   Flag 4 (0x04): Team 3 can spawn here
//   Flag 8 (0x08): Team 4 can spawn here
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Spawn flag definitions for team spawning
//-----------------------------------------------------------------------------
#define SF_GMOD_SPAWN_TEAM1		0x01	// Team 1 can spawn
#define SF_GMOD_SPAWN_TEAM2		0x02	// Team 2 can spawn
#define SF_GMOD_SPAWN_TEAM3		0x04	// Team 3 can spawn
#define SF_GMOD_SPAWN_TEAM4		0x08	// Team 4 can spawn
#define SF_GMOD_SPAWN_ALLTEAMS	0x0F	// All teams can spawn

//-----------------------------------------------------------------------------
// CGModPlayerStart - Team-based spawn point for GMod gamemodes
//-----------------------------------------------------------------------------
class CGModPlayerStart : public CPointEntity
{
public:
	DECLARE_CLASS(CGModPlayerStart, CPointEntity);
	DECLARE_DATADESC();

	CGModPlayerStart();

	virtual void Spawn();
	virtual void Activate();

	// Check if a specific team can spawn here
	bool IsValidForTeam(int teamNumber);

	// Check if spawn is valid (not blocked by other players)
	bool IsValid(CBasePlayer *pPlayer = NULL);

	// Get spawn info
	int GetTeamMask() { return m_nSpawnFlags; }

	// Input handlers
	void InputEnable(inputdata_t &inputdata);
	void InputDisable(inputdata_t &inputdata);

private:
	bool m_bDisabled;		// Is this spawn point disabled?
	int m_nSpawnFlags;		// Team flags (copied from spawnflags)

	COutputEvent m_OnPlayerSpawn;	// Fired when player spawns here
};

//-----------------------------------------------------------------------------
// Entity linkage
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(gmod_player_start, CGModPlayerStart);

//-----------------------------------------------------------------------------
// Data description
//-----------------------------------------------------------------------------
BEGIN_DATADESC(CGModPlayerStart)
	DEFINE_KEYFIELD(CGModPlayerStart, m_bDisabled, FIELD_BOOLEAN, "StartDisabled"),

	// Input handlers
	DEFINE_INPUTFUNC(CGModPlayerStart, FIELD_VOID, "Enable", InputEnable),
	DEFINE_INPUTFUNC(CGModPlayerStart, FIELD_VOID, "Disable", InputDisable),

	// Outputs
	DEFINE_OUTPUT(CGModPlayerStart, m_OnPlayerSpawn, "OnPlayerSpawn"),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGModPlayerStart::CGModPlayerStart()
{
	m_bDisabled = false;
	m_nSpawnFlags = SF_GMOD_SPAWN_ALLTEAMS;	// Default: all teams can spawn
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the entity
//-----------------------------------------------------------------------------
void CGModPlayerStart::Spawn()
{
	BaseClass::Spawn();

	// Store spawnflags for team checking
	m_nSpawnFlags = GetSpawnFlags() & SF_GMOD_SPAWN_ALLTEAMS;

	// If no team flags set, allow all teams
	if (m_nSpawnFlags == 0)
		m_nSpawnFlags = SF_GMOD_SPAWN_ALLTEAMS;

	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
}

//-----------------------------------------------------------------------------
// Purpose: Activate - register with game rules if needed
//-----------------------------------------------------------------------------
void CGModPlayerStart::Activate()
{
	BaseClass::Activate();

	DevMsg("gmod_player_start activated at (%.1f, %.1f, %.1f) with team flags: 0x%X\n",
		GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z, m_nSpawnFlags);
}

//-----------------------------------------------------------------------------
// Purpose: Check if a specific team can spawn at this point
//-----------------------------------------------------------------------------
bool CGModPlayerStart::IsValidForTeam(int teamNumber)
{
	if (m_bDisabled)
		return false;

	// Team 0 (unassigned) and Team 1 (spectator) can't use team spawns
	if (teamNumber < 2)
		return false;

	// Map team number to flag bit
	// Team 2 = bit 0 (0x01), Team 3 = bit 1 (0x02), etc.
	int flagBit = 1 << (teamNumber - 2);

	return (m_nSpawnFlags & flagBit) != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Check if spawn point is valid (not blocked)
//-----------------------------------------------------------------------------
bool CGModPlayerStart::IsValid(CBasePlayer *pPlayer)
{
	if (m_bDisabled)
		return false;

	// Check if another player is blocking this spawn
	CBaseEntity *ent = NULL;
	for (CEntitySphereQuery sphere(GetAbsOrigin(), 64); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
	{
		if (ent->IsPlayer() && ent != pPlayer)
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Enable this spawn point
//-----------------------------------------------------------------------------
void CGModPlayerStart::InputEnable(inputdata_t &inputdata)
{
	m_bDisabled = false;
}

//-----------------------------------------------------------------------------
// Purpose: Disable this spawn point
//-----------------------------------------------------------------------------
void CGModPlayerStart::InputDisable(inputdata_t &inputdata)
{
	m_bDisabled = true;
}
