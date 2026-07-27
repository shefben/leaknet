//======== (C) Copyright 1999, 2000 Valve, L.L.C. All rights reserved. ========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
#if !defined( ICVAR_H )
#define ICVAR_H
#ifdef _WIN32
#pragma once
#endif

#include "convar.h"
#include "interface.h"

class ICvar
{
public:
	// Try to register cvar
	virtual void			RegisterConCommandBase ( ConCommandBase *variable ) = 0;

	// If there is a +<varname> <value> on the command line, this returns the value.
	// Otherwise, it returns NULL.
	virtual char const*		GetCommandLineValue( char const *pVariableName ) = 0;

	// Try to find the cvar pointer by name
	virtual const ConVar	*FindVar ( const char *var_name ) = 0;

	// Get first ConCommandBase to allow iteration
	virtual ConCommandBase	*GetCommands( void ) = 0;

	// Remove a cvar/command from the engine's list. Must be called before the memory
	// backing 'variable' goes away, otherwise the engine list is left holding a
	// dangling pointer (which blows up the next time the list is walked, e.g. in
	// ConCommandBase::RemoveFlaggedCommands during shutdown).
	virtual void			UnregisterConCommandBase ( ConCommandBase *variable ) = 0;
};

#define VENGINE_CVAR_INTERFACE_VERSION "VEngineCvar001"

extern ICvar *cvar;

#endif // ICVAR_H