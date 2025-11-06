//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "tier0/platform.h"
#include "tier0/vcrmode.h"
#include "tier0/memalloc.h"
#include "tier0/dbg.h"

#include <sys/time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern VCRMode g_VCRMode;

double Plat_FloatTime()
{
        struct timeval  tp;
        static int      secbase = 0;

        gettimeofday( &tp, NULL );

        if ( !secbase )
        {
                secbase = tp.tv_sec;
                return ( tp.tv_usec / 1000000.0 );
        }

	if (g_VCRMode == VCR_Disabled)
		return (( tp.tv_sec - secbase ) + tp.tv_usec / 1000000.0 );

	return g_pVCR->Hook_Sys_FloatTime( ( tp.tv_sec - secbase ) + tp.tv_usec / 1000000.0 );
}

unsigned long Plat_MSTime()
{
        struct timeval  tp;
        static int      secbase = 0;

        gettimeofday( &tp, NULL );

        if ( !secbase )
        {
                secbase = tp.tv_sec;
                return ( tp.tv_usec / 1000000.0 );
        }

	return (unsigned long)(( tp.tv_sec - secbase ) + tp.tv_usec / 1000000.0 );

}

bool vtune( bool resume )
{
	Assert( "vtune not implemented" );
	return false;
}

// -------------------------------------------------------------------------------------------------- //
// Memory stuff.
// -------------------------------------------------------------------------------------------------- //

PLATFORM_INTERFACE void Plat_DefaultAllocErrorFn( unsigned long size )
{
}

Plat_AllocErrorFn g_AllocError = Plat_DefaultAllocErrorFn;

PLATFORM_INTERFACE void *Plat_Alloc( unsigned long size )
{
	void *pRet = g_pMemAlloc->Alloc( size );
	if ( pRet )
	{
		return pRet;
	}
	else
	{
		g_AllocError( size );
		return 0;
	}
}

PLATFORM_INTERFACE void* Plat_Realloc( void *ptr, unsigned long size )
{
	void *pRet = g_pMemAlloc->Realloc( ptr, size );
	if ( pRet )
	{
		return pRet;
	}
	else
	{
		g_AllocError( size );
		return 0;
	}
}

PLATFORM_INTERFACE void Plat_Free( void *ptr )
{
	g_pMemAlloc->Free( ptr );
}


PLATFORM_INTERFACE void Plat_SetAllocErrorFn( Plat_AllocErrorFn fn )
{
	g_AllocError = fn;
}

PLATFORM_INTERFACE void Plat_SetThreadName( unsigned long dwThreadID, const char *pName )
{
	Assert( "Plat_SetThreadName not implemented");
}

// Source2013
bool Plat_IsInDebugSession()
{
	// For linux: http://stackoverflow.com/questions/3596781/detect-if-gdb-is-running
	// Don't use "if (ptrace(PTRACE_TRACEME, 0, NULL, 0) == -1)" as it means debuggers can't attach.
	// 	Other solutions they mention involve forking. Ugh.
	//
	// Good solution from Pierre-Loup: Check TracerPid in /proc/self/status.
	// from "man proc"
	//     TracerPid: PID of process tracing this process (0 if not being traced).
	int tracerpid = -1;

	int fd = open( "/proc/self/status", O_RDONLY, S_IRUSR );
	if( fd >= 0 )
	{
		char buf[ 1024 ];
		static const char s_TracerPid[] = "TracerPid:";

		int len = read( fd, buf, sizeof( buf ) - 1 );
		if ( len > 0 )
		{
			buf[ len ] = 0;

			const char *str = strstr( buf, s_TracerPid );
			tracerpid = str ? atoi( str + sizeof( s_TracerPid ) ) : -1;
		}

		close( fd );
	}

	return ( tracerpid > 0 );
}

static char g_CmdLine[ 2048 ];
PLATFORM_INTERFACE void Plat_SetCommandLine( const char *cmdLine )
{
	strncpy( g_CmdLine, cmdLine, sizeof(g_CmdLine) );
	g_CmdLine[ sizeof(g_CmdLine) - 1 ] = 0;
}

PLATFORM_INTERFACE const char *Plat_GetCommandLine()
{
#ifdef LINUX
	if( !g_CmdLine[ 0 ] )
	{
		FILE *fp = fopen( "/proc/self/cmdline", "rb" );

		if( fp )
		{
			size_t nCharRead = 0;

			// -1 to leave room for the '\0'
			nCharRead = fread( g_CmdLine, sizeof( g_CmdLine[0] ), ARRAYSIZE( g_CmdLine ) - 1, fp );
			if ( feof( fp ) && !ferror( fp ) ) // Should have read the whole command line without error
			{
				Assert ( nCharRead < ARRAYSIZE( g_CmdLine ) );

				for( int i = 0; i < nCharRead; i++ )
				{
					if( g_CmdLine[ i ] == '\0' )
						g_CmdLine[ i ] = ' ';
				}

				g_CmdLine[ nCharRead ] = '\0';

			}
			fclose( fp );
		}

		Assert( g_CmdLine[ 0 ] );
	}
#endif // LINUX

	return g_CmdLine;
}
