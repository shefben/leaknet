//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 2003 Source Engine compatible CCommand implementation
// Wraps legacy Cmd_Argc/Cmd_Argv functions to provide modern CCommand API
//
//=============================================================================

#include "convar.h"
#include "cdll_int.h"
#include "vstdlib/strtools.h"
#include <string.h>

// Global engine interface pointer - defined in engine client
extern IVEngineClient *engine;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCommand::CCommand()
{
	m_bArgSBuilt = false;
	m_pArgSBuffer[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: Get number of arguments
//-----------------------------------------------------------------------------
int CCommand::ArgC() const
{
	return engine ? engine->Cmd_Argc() : 0;
}

//-----------------------------------------------------------------------------
// Purpose: Get argument by index
//-----------------------------------------------------------------------------
const char *CCommand::Arg(int nIndex) const
{
	if (!engine)
		return "";

	return engine->Cmd_Argv(nIndex);
}

//-----------------------------------------------------------------------------
// Purpose: Get all arguments from index 1 onwards as a single string
//-----------------------------------------------------------------------------
const char *CCommand::ArgS() const
{
	if (!engine)
		return "";

	if (!m_bArgSBuilt)
	{
		BuildArgS();
	}

	return m_pArgSBuffer;
}

//-----------------------------------------------------------------------------
// Purpose: Build the ArgS string from individual arguments
//-----------------------------------------------------------------------------
void CCommand::BuildArgS() const
{
	m_pArgSBuffer[0] = '\0';

	if (!engine)
	{
		m_bArgSBuilt = true;
		return;
	}

	int argc = engine->Cmd_Argc();
	for (int i = 1; i < argc; i++)
	{
		const char *arg = engine->Cmd_Argv(i);
		if (arg && *arg)
		{
			if (i > 1)
			{
				Q_strncat(m_pArgSBuffer, " ", sizeof(m_pArgSBuffer));
			}
			Q_strncat(m_pArgSBuffer, arg, sizeof(m_pArgSBuffer));
		}
	}

	m_bArgSBuilt = true;
}