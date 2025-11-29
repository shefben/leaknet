//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CGmodRunFunction entity - executes Lua functions as entity think
// Based on reverse engineering of Garry's Mod server.dll
//
//=============================================================================//

#include "cbase.h"
#include "gmod_runfunction.h"
#include "lua_integration.h"
#include "entityoutput.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Entity Implementation
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(gmod_runfunction, CGmodRunFunction);

//-----------------------------------------------------------------------------
// Data Description
//-----------------------------------------------------------------------------
BEGIN_DATADESC(CGmodRunFunction)
	// Key Values
	DEFINE_KEYFIELD(CGmodRunFunction, m_iszFunction, FIELD_STRING, "function"),
	DEFINE_KEYFIELD(CGmodRunFunction, m_flDelay, FIELD_FLOAT, "delay"),

	// Inputs
	DEFINE_INPUTFUNC(CGmodRunFunction, FIELD_STRING, "RunFunction", InputRunFunction),

	// Outputs
	DEFINE_OUTPUT(CGmodRunFunction, m_OnFinished, "OnFinished"),

	// Functions
	DEFINE_THINKFUNC(CGmodRunFunction, Think),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Spawn the entity
//-----------------------------------------------------------------------------
void CGmodRunFunction::Spawn()
{
	BaseClass::Spawn();

	// Set invisible point entity
	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
	m_fEffects |= EF_NODRAW;

	// Set default values
	if (m_flDelay <= 0.0f)
		m_flDelay = 0.1f;

	// Start thinking if we have a function
	if (m_iszFunction != NULL_STRING)
	{
		SetThink(&CGmodRunFunction::Think);
		SetNextThink(gpGlobals->curtime + m_flDelay);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Think function - executes the Lua function
//-----------------------------------------------------------------------------
void CGmodRunFunction::Think()
{
	if (!CLuaIntegration::IsInitialized())
	{
		Warning("CGmodRunFunction: Lua system not initialized!\n");
		return;
	}

	if (m_iszFunction == NULL_STRING)
	{
		Warning("CGmodRunFunction: No function specified!\n");
		return;
	}

	const char *pszFunction = STRING(m_iszFunction);

	// Execute the Lua function
	bool success = CLuaIntegration::CallFunction(pszFunction, 0);

	if (success)
	{
		DevMsg("CGmodRunFunction: Successfully executed '%s'\n", pszFunction);
	}
	else
	{
		Warning("CGmodRunFunction: Failed to execute '%s'\n", pszFunction);
	}

	// Fire output
	m_OnFinished.FireOutput(this, this);

	// Set next think
	SetNextThink(gpGlobals->curtime + m_flDelay);
}

//-----------------------------------------------------------------------------
// Purpose: Input to run a specific function
//-----------------------------------------------------------------------------
void CGmodRunFunction::InputRunFunction(inputdata_t &inputdata)
{
	const char *pszFunction = inputdata.value.String();

	if (!pszFunction || pszFunction[0] == '\0')
	{
		Warning("CGmodRunFunction: InputRunFunction called with empty function name!\n");
		return;
	}

	// Store the function name
	m_iszFunction = AllocPooledString(pszFunction);

	// Execute immediately
	Think();
}

//-----------------------------------------------------------------------------
// Purpose: Set the Lua function to execute
//-----------------------------------------------------------------------------
void CGmodRunFunction::SetFunction(const char *pszFunction)
{
	if (!pszFunction)
		return;

	m_iszFunction = AllocPooledString(pszFunction);

	// Start thinking if not already
	if (GetNextThink() < gpGlobals->curtime)
	{
		SetThink(&CGmodRunFunction::Think);
		SetNextThink(gpGlobals->curtime + m_flDelay);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the execution delay
//-----------------------------------------------------------------------------
void CGmodRunFunction::SetDelay(float flDelay)
{
	m_flDelay = flDelay;

	if (m_flDelay <= 0.0f)
		m_flDelay = 0.1f;
}