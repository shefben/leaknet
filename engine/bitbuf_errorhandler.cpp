//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#include "bitbuf.h"
#include "bitbuf_errorhandler.h"
#include "tier0/dbg.h"
#include "utlsymbol.h"


void EngineBitBufErrorHandler( BitBufErrorType errorType, const char *pDebugName )
{
	if ( !pDebugName )
	{
		pDebugName = "(unknown)";
	}

	// Track errors per unique debug name to avoid spam while still showing each unique error once
	static CUtlSymbolTable errorNames[ BITBUFERROR_NUM_ERRORS ];

	// Only print an error once per unique debug name
	CUtlSymbol sym = errorNames[ errorType ].Find( pDebugName );
	if ( UTL_INVAL_SYMBOL == sym )
	{
		errorNames[ errorType ].AddString( pDebugName );
		if ( errorType == BITBUFERROR_VALUE_OUT_OF_RANGE )
		{
			Warning( "Error in bitbuf [%s]: out of range value\n", pDebugName );
		}
		else if ( errorType == BITBUFERROR_BUFFER_OVERRUN )
		{
			Warning( "Error in bitbuf [%s]: buffer overrun\n", pDebugName );
		}
	}
}


void InstallBitBufErrorHandler()
{
	SetBitBufErrorHandler( EngineBitBufErrorHandler );
}






