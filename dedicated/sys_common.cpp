//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#ifdef _WIN32
#include <windows.h> 
#endif
#include <stdio.h>
#include <stdlib.h>
#include "isys.h"
#include "dedicated.h"
#include "dll_state.h"
#include "engine_hlds_api.h"
#include "filesystem.h"
#include "ifilesystem.h"
#include "vstdlib/strtools.h"
#include "vstdlib/icommandline.h"

static long		hDLLThirdParty	= 0L;


/*
==============
Load3rdParty

Load support for third party .dlls ( gamehost )
==============
*/
void Load3rdParty( void )
{
	// Only do this if the server operator wants the support.
	// ( In case of malicious code, too )
	if ( CommandLine()->CheckParm( "-usegh" ) )   
	{
		hDLLThirdParty = sys->LoadLibrary( "ghostinj" DLL_EXT_STRING );
	}
}

/*
==============
EF_VID_ForceUnlockedAndReturnState

Dummy funcion called by engine
==============
*/
int  EF_VID_ForceUnlockedAndReturnState(void)
{
	return 0;
}

/*
==============
EF_VID_ForceLockState

Dummy funcion called by engine
==============
*/
void EF_VID_ForceLockState(int)
{
}

/*
==============
InitInstance

==============
*/
bool InitInstance( )
{
	// Start up the file system
	if (!FileSystem_Init( ))
		return false;

	Load3rdParty();

	return true;
}

/*
==============
ProcessConsoleInput

==============
*/
void ProcessConsoleInput( void )
{
	char *s;

	if ( !engine )
		return;

	do
	{
		s = sys->ConsoleInput();
		if (s)
		{
			char szBuf[ 256 ];
			sprintf( szBuf, "%s\n", s );
			engine->AddConsoleText ( szBuf );
		}
	} while (s);
}
