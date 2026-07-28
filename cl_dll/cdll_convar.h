//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
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

#ifndef CDLL_CONVAR_H
#define CDLL_CONVAR_H
#pragma once


// This file implements IConVarAccessor to allow access to console variables.



#include "convar.h"
#include "cdll_util.h"
#include "icvar.h"

class CDLL_ConVarAccessor : public IConCommandBaseAccessor
{
public:
	virtual bool	RegisterConCommandBase( ConCommandBase *pCommand )
	{
		// Mark for easy removal
		pCommand->AddFlags( FCVAR_CLIENTDLL );

		// Unlink from client .dll only list (keeps the list's prev pointers intact)
		ConCommandBase::RemoveFromList( pCommand );

		// Link to engine's list instead
		cvar->RegisterConCommandBase( pCommand );
		return true;
	}

	virtual void	UnregisterConCommandBase( ConCommandBase *pCommand )
	{
		if ( cvar )
		{
			cvar->UnregisterConCommandBase( pCommand );
		}
	}
};



#endif // CDLL_CONVAR_H
