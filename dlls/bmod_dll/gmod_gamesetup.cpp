//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GMod game setup entity
// Based on IDA reverse engineering of Garry's Mod 9.0.4b server.dll
//
// gmod_gamesetup is placed in maps to configure gamemode-specific settings
// such as round time, team scores, and game rules.
//
//=============================================================================//

#include "cbase.h"
#include "gmod_gamemode.h"
#include "lua_integration.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// CGModGameSetup - Entity for configuring gamemode settings
//-----------------------------------------------------------------------------
class CGModGameSetup : public CPointEntity
{
public:
	DECLARE_CLASS(CGModGameSetup, CPointEntity);
	DECLARE_DATADESC();

	CGModGameSetup();

	virtual void Spawn();
	virtual void Activate();

	// Input handlers
	void InputStartGame(inputdata_t &inputdata);
	void InputEndGame(inputdata_t &inputdata);
	void InputRestartRound(inputdata_t &inputdata);
	void InputSetTeamScore(inputdata_t &inputdata);
	void InputSetRoundTime(inputdata_t &inputdata);
	void InputRunLuaFunction(inputdata_t &inputdata);

private:
	// Map-specified settings
	string_t m_iszGamemode;		// Gamemode to use
	float m_flRoundTime;		// Round time limit (0 = no limit)
	int m_nTeam1Score;			// Starting score for team 1
	int m_nTeam2Score;			// Starting score for team 2
	bool m_bAutoStart;			// Automatically start game on map load

	// Outputs
	COutputEvent m_OnGameStart;		// Fired when game starts
	COutputEvent m_OnGameEnd;		// Fired when game ends
	COutputEvent m_OnRoundStart;	// Fired when round starts
	COutputEvent m_OnRoundEnd;		// Fired when round ends
};

//-----------------------------------------------------------------------------
// Entity linkage
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(gmod_gamesetup, CGModGameSetup);

//-----------------------------------------------------------------------------
// Data description
//-----------------------------------------------------------------------------
BEGIN_DATADESC(CGModGameSetup)
	// Key fields
	DEFINE_KEYFIELD(CGModGameSetup, m_iszGamemode, FIELD_STRING, "gamemode"),
	DEFINE_KEYFIELD(CGModGameSetup, m_flRoundTime, FIELD_FLOAT, "roundtime"),
	DEFINE_KEYFIELD(CGModGameSetup, m_nTeam1Score, FIELD_INTEGER, "team1score"),
	DEFINE_KEYFIELD(CGModGameSetup, m_nTeam2Score, FIELD_INTEGER, "team2score"),
	DEFINE_KEYFIELD(CGModGameSetup, m_bAutoStart, FIELD_BOOLEAN, "autostart"),

	// Input handlers
	DEFINE_INPUTFUNC(CGModGameSetup, FIELD_VOID, "StartGame", InputStartGame),
	DEFINE_INPUTFUNC(CGModGameSetup, FIELD_VOID, "EndGame", InputEndGame),
	DEFINE_INPUTFUNC(CGModGameSetup, FIELD_VOID, "RestartRound", InputRestartRound),
	DEFINE_INPUTFUNC(CGModGameSetup, FIELD_STRING, "SetTeamScore", InputSetTeamScore),
	DEFINE_INPUTFUNC(CGModGameSetup, FIELD_FLOAT, "SetRoundTime", InputSetRoundTime),
	DEFINE_INPUTFUNC(CGModGameSetup, FIELD_STRING, "RunLuaFunction", InputRunLuaFunction),

	// Outputs
	DEFINE_OUTPUT(CGModGameSetup, m_OnGameStart, "OnGameStart"),
	DEFINE_OUTPUT(CGModGameSetup, m_OnGameEnd, "OnGameEnd"),
	DEFINE_OUTPUT(CGModGameSetup, m_OnRoundStart, "OnRoundStart"),
	DEFINE_OUTPUT(CGModGameSetup, m_OnRoundEnd, "OnRoundEnd"),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGModGameSetup::CGModGameSetup()
{
	m_iszGamemode = NULL_STRING;
	m_flRoundTime = 0.0f;
	m_nTeam1Score = 0;
	m_nTeam2Score = 0;
	m_bAutoStart = true;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the entity
//-----------------------------------------------------------------------------
void CGModGameSetup::Spawn()
{
	BaseClass::Spawn();

	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
	m_fEffects |= EF_NODRAW;
}

//-----------------------------------------------------------------------------
// Purpose: Activate - apply settings and optionally start game
//-----------------------------------------------------------------------------
void CGModGameSetup::Activate()
{
	BaseClass::Activate();

	// Apply gamemode if specified
	if (m_iszGamemode != NULL_STRING)
	{
		const char *pszGamemode = STRING(m_iszGamemode);
		if (pszGamemode && pszGamemode[0])
		{
			CGModGamemodeSystem::SetActiveGamemode(pszGamemode);
			DevMsg("gmod_gamesetup: Set gamemode to '%s'\n", pszGamemode);
		}
	}

	// Apply initial team scores
	if (m_nTeam1Score != 0)
	{
		CGModGamemodeSystem::SetTeamScore(2, m_nTeam1Score);
	}
	if (m_nTeam2Score != 0)
	{
		CGModGamemodeSystem::SetTeamScore(3, m_nTeam2Score);
	}

	// Auto-start game if enabled
	if (m_bAutoStart)
	{
		DevMsg("gmod_gamesetup: Auto-starting game\n");
		m_OnGameStart.FireOutput(this, this);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Start the game
//-----------------------------------------------------------------------------
void CGModGameSetup::InputStartGame(inputdata_t &inputdata)
{
	DevMsg("gmod_gamesetup: Starting game\n");
	m_OnGameStart.FireOutput(inputdata.pActivator, this);
	m_OnRoundStart.FireOutput(inputdata.pActivator, this);
}

//-----------------------------------------------------------------------------
// Purpose: End the game
//-----------------------------------------------------------------------------
void CGModGameSetup::InputEndGame(inputdata_t &inputdata)
{
	DevMsg("gmod_gamesetup: Ending game\n");
	m_OnRoundEnd.FireOutput(inputdata.pActivator, this);
	m_OnGameEnd.FireOutput(inputdata.pActivator, this);
}

//-----------------------------------------------------------------------------
// Purpose: Restart the current round
//-----------------------------------------------------------------------------
void CGModGameSetup::InputRestartRound(inputdata_t &inputdata)
{
	DevMsg("gmod_gamesetup: Restarting round\n");
	m_OnRoundEnd.FireOutput(inputdata.pActivator, this);
	m_OnRoundStart.FireOutput(inputdata.pActivator, this);
}

//-----------------------------------------------------------------------------
// Purpose: Set team score via input (format: "team,score")
//-----------------------------------------------------------------------------
void CGModGameSetup::InputSetTeamScore(inputdata_t &inputdata)
{
	const char *pszValue = inputdata.value.String();
	if (!pszValue)
		return;

	int team, score;
	if (sscanf(pszValue, "%d,%d", &team, &score) == 2)
	{
		CGModGamemodeSystem::SetTeamScore(team, score);
		DevMsg("gmod_gamesetup: Set team %d score to %d\n", team, score);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set round time
//-----------------------------------------------------------------------------
void CGModGameSetup::InputSetRoundTime(inputdata_t &inputdata)
{
	m_flRoundTime = inputdata.value.Float();
	DevMsg("gmod_gamesetup: Set round time to %.1f\n", m_flRoundTime);
}

//-----------------------------------------------------------------------------
// Purpose: Run a Lua function
//-----------------------------------------------------------------------------
void CGModGameSetup::InputRunLuaFunction(inputdata_t &inputdata)
{
	const char *pszFunction = inputdata.value.String();
	if (!pszFunction || !pszFunction[0])
		return;

	if (CLuaIntegration::IsInitialized())
	{
		CLuaIntegration::CallFunction(pszFunction, 0);
	}
	else
	{
		Warning("gmod_gamesetup: Lua system not initialized, cannot run '%s'\n", pszFunction);
	}
}
