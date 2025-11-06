//-----------------------------------------------------------------------------
// This is just a little redirection tool so I can get all the dlls in bin
//-----------------------------------------------------------------------------
#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <assert.h>

#elif _LINUX

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <limits.h>
#include <string.h>
#define MAX_PATH PATH_MAX

#else
#error
#endif

// SanyaSho: For DLL_EXT_STRING (tier0/basetypes.h in Source2013)
#include "tier0/platform.h"

#ifdef _WIN32
typedef int (*LauncherMain_t)( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
							  LPSTR lpCmdLine, int nCmdShow );
#elif _LINUX
typedef int (*LauncherMain_t)( int argc, char **argv );
#else
#error
#endif

//-----------------------------------------------------------------------------
// Purpose: Return the directory where this .exe is running from
// Output : char
//-----------------------------------------------------------------------------

static char *GetBaseDir( const char *pszBuffer )
{
	static char	basedir[ MAX_PATH ];
	char szBuffer[ MAX_PATH ];
	int j;
	char *pBuffer = NULL;

	strcpy( szBuffer, pszBuffer );

	pBuffer = strrchr( szBuffer,'\\' );
	if ( pBuffer )
	{
		*(pBuffer+1) = '\0';
	}

	strcpy( basedir, szBuffer );

	j = strlen( basedir );
	if (j > 0)
	{
		if ( ( basedir[ j-1 ] == '\\' ) || 
			 ( basedir[ j-1 ] == '/' ) )
		{
			basedir[ j-1 ] = 0;
		}
	}

	return basedir;
}

#ifdef _WIN32
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	// Must add 'bin' to the path....
	char* pPath = getenv("PATH");

	// Use the .EXE name to determine the root directory
	char moduleName[ MAX_PATH ];
	char szBuffer[ 4096 ];
	if ( !GetModuleFileName( hInstance, moduleName, MAX_PATH ) )
	{
		MessageBox( 0, "Failed calling GetModuleFileName", "Launcher Error", MB_OK );
		return 0;
	}

	// Get the root directory the .exe is in
	char* pRootDir = GetBaseDir( moduleName );

#ifdef _DEBUG
	int len = 
#endif
		sprintf( szBuffer, "PATH=%s\\bin\\;%s", pRootDir, pPath );
	assert( len < 4096 );
	_putenv( szBuffer );

	HINSTANCE launcher = LoadLibrary("launcher" DLL_EXT_STRING); // STEAM OK ... filesystem not mounted yet
	if (!launcher)
	{
		char *pszError;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pszError, 0, NULL);

		char szBuf[1024];
		sprintf(szBuf, "Failed to load the launcher DLL:\n\n%s", pszError);
		MessageBox( 0, szBuf, "Launcher Error", MB_OK );

		LocalFree(pszError);
		return 0;
	}

	LauncherMain_t main = (LauncherMain_t)GetProcAddress( launcher, "LauncherMain" );
	return main( hInstance, hPrevInstance, lpCmdLine, nCmdShow );
}
#elif _LINUX

int main( int argc, char *argv[] )
{
	void *launcher = dlopen("bin/launcher" DLL_EXT_STRING, RTLD_NOW );
	if ( !launcher )
	{
		fprintf( stderr, "Failed to load the launcher\n" );
		return 0;
	}

	LauncherMain_t lmain = (LauncherMain_t)dlsym( launcher, "LauncherMain" );
	if ( !lmain )
	{
		fprintf( stderr, "Failed to load the launcher entry proc\n" );
		return 0;
	}

	return lmain( argc, argv );
}

#else

#error 

#endif
