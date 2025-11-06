//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// $Header: $
// $NoKeywords: $
//
// Main class factory methods
//=============================================================================

#if !defined( DONT_PROTECT_FILEIO_FUNCTIONS )
	#define DONT_PROTECT_FILEIO_FUNCTIONS // for protected_things.h
#endif

#if defined( PROTECTED_THINGS_ENABLE )
#undef PROTECTED_THINGS_ENABLE // from protected_things.h
#endif


#include <stdio.h>
#include "interface.h"
#include "basetypes.h"
#include <string.h>
#include <stdlib.h>
#include "vstdlib/strtools.h"

// ------------------------------------------------------------------------------------ //
// InterfaceReg.
// ------------------------------------------------------------------------------------ //
InterfaceReg *InterfaceReg::s_pInterfaceRegs = NULL;


InterfaceReg::InterfaceReg( InstantiateInterfaceFn fn, const char *pName ) :
	m_pName(pName)
{
	m_CreateFn = fn;
	m_pNext = s_pInterfaceRegs;
	s_pInterfaceRegs = this;
}



// ------------------------------------------------------------------------------------ //
// CreateInterface.
// ------------------------------------------------------------------------------------ //

void* CreateInterface( const char *pName, int *pReturnCode )
{
	InterfaceReg *pCur;
	
	for(pCur=InterfaceReg::s_pInterfaceRegs; pCur; pCur=pCur->m_pNext)
	{
		if(strcmp(pCur->m_pName, pName) == 0)
		{
			if ( pReturnCode )
			{
				*pReturnCode = IFACE_OK;
			}
			return pCur->m_CreateFn();
		}
	}
	
	if ( pReturnCode )
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;	
}


#if defined( _WIN32 )
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#elif defined( _LINUX )
//
// Linux doesn't have this function so this emulates its functionality
//
void *GetModuleHandle( const char *name )
{
        void *handle;

        if( name == NULL )
        {
                // hmm, how can this be handled under linux....
                // is it even needed?
                return NULL;
        }

        char szCwd[1024] = { 0 };
        char szAbsoluteModuleName[1024] = { 0 };

        getcwd( szCwd, sizeof( szCwd ) );
        if ( szCwd[ strlen( szCwd ) - 1 ] == '/' )
		szCwd[ strlen( szCwd ) - 1 ] = 0;

        _snprintf( szAbsoluteModuleName, sizeof(szAbsoluteModuleName),"%s/%s", szCwd, name );

	handle = dlopen( szAbsoluteModuleName, RTLD_NOW ); // VXP: Try to load library with unedited name

        if( !handle )
        {
		const char *pError = dlerror();
		if ( pError )
		{
			printf( "dlerror(): %s\n", pError );
		}
	}

        // read "man dlopen" for details
        // in short dlopen() inc a ref count
        // so dec the ref count by performing the close
        dlclose(handle);
	return handle;
}
#endif // _LINUX

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a function, given a module
// Input  : pModuleName - module name
//			*pName - proc name
//-----------------------------------------------------------------------------
static void *Sys_GetProcAddress( const char *pModuleName, const char *pName )
{
	return GetProcAddress( GetModuleHandle(pModuleName), pName );
}

static void Sys_SpewLoadModuleError()
{
#if defined( _WIN32 )
	char* lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);

/*
	va_list	argptr;
	char text[1024];
	va_start(argptr, lpMsgBuf);
	snprintf(text, sizeof(text), "LoadLibrary: %s\n", argptr);
	va_end(argptr);
	OutputDebugString( text );
*/
	printf("LoadLibrary: %s\n", lpMsgBuf); // VXP: Not working in platform apps unfortunately

	LocalFree((HLOCAL)lpMsgBuf);
#elif defined( _LINUX )
	const char* pError = dlerror();
	if (pError)
	{
		printf("dlerror(): %s\n", pError);
	}
#endif // _LINUX
}

//-----------------------------------------------------------------------------
// Purpose: Loads a DLL/component from disk and returns a handle to it
// Input  : *pModuleName - filename of the component
// Output : opaque handle to the module (hides system dependency)
//-----------------------------------------------------------------------------
CSysModule *Sys_LoadModule( const char *pModuleName )
{
	// If using the Steam filesystem, either the DLL must be a minimum footprint
	// file in the depot (MFP) or a filesystem GetLocalCopy() call must be made
	// prior to the call to this routine.

	HMODULE hDLL;

#if defined( _WIN32 )
	hDLL = LoadLibrary( pModuleName );
#elif defined( _LINUX )
	char szCwd[1024] = { 0 };
	char szAbsoluteModuleName[1024] = { 0 };

	getcwd( szCwd, sizeof( szCwd ) );
	if ( szCwd[ strlen( szCwd ) - 1 ] == '/' )
		szCwd[ strlen( szCwd ) - 1 ] = 0;

	_snprintf( szAbsoluteModuleName, sizeof(szAbsoluteModuleName),"%s/%s", szCwd, pModuleName );

	hDLL = dlopen( szAbsoluteModuleName, RTLD_NOW );
#endif // _LINUX

	if( !hDLL )
	{
#ifdef _DEBUG
		// So you can see what the error is in the debugger...
		Sys_SpewLoadModuleError();
#endif // _DEBUG

		char str[512];
#if defined( _WIN32 )
		Q_snprintf( str, sizeof(str), "%s", pModuleName );
		hDLL = LoadLibrary( str );
#elif defined( _LINUX )
		Q_snprintf( str, sizeof(str), "%s", szAbsoluteModuleName );
		hDLL = dlopen(str, RTLD_NOW);
#endif // _LINUX

		if ( !hDLL )
		{
#ifdef _DEBUG
			Sys_SpewLoadModuleError();
#endif // _DEBUG
		}
	}

	return reinterpret_cast<CSysModule *>(hDLL);
}

//-----------------------------------------------------------------------------
// Purpose: Unloads a DLL/component from
// Input  : *pModuleName - filename of the component
// Output : opaque handle to the module (hides system dependency)
//-----------------------------------------------------------------------------
void Sys_UnloadModule( CSysModule *pModule )
{
	if ( !pModule )
		return;

	HMODULE	hDLL = reinterpret_cast<HMODULE>(pModule);

#if defined( _WIN32 )
	FreeLibrary( hDLL );
#elif defined( _LINUX )
	dlclose((void *)hDLL);
#endif // _LINUX
}

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a function, given a module
// Input  : module - windows HMODULE from Sys_LoadModule() 
//			*pName - proc name
// Output : factory for this module
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactory( CSysModule *pModule )
{
	if ( !pModule )
		return NULL;

	HMODULE	hDLL = reinterpret_cast<HMODULE>(pModule);
#if defined( _WIN32 )
	return reinterpret_cast<CreateInterfaceFn>(GetProcAddress( hDLL, CREATEINTERFACE_PROCNAME ));
#elif defined( _LINUX )
// Linux gives this error:
//../public/interface.cpp: In function `IBaseInterface *(*Sys_GetFactory
//(CSysModule *)) (const char *, int *)':
//../public/interface.cpp:154: ISO C++ forbids casting between
//pointer-to-function and pointer-to-object
//
// so lets get around it :)
        return (CreateInterfaceFn)(GetProcAddress( hDLL, CREATEINTERFACE_PROCNAME ));
#endif // _LINUX

}

//-----------------------------------------------------------------------------
// Purpose: returns the instance of this module
// Output : interface_instance_t
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactoryThis( void )
{
	return CreateInterface;
}

//-----------------------------------------------------------------------------
// Purpose: returns the instance of the named module
// Input  : *pModuleName - name of the module
// Output : interface_instance_t - instance of that module
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactory( const char *pModuleName )
{
#if defined( _WIN32 )
	return static_cast<CreateInterfaceFn>( Sys_GetProcAddress( pModuleName, CREATEINTERFACE_PROCNAME ) );
#elif defined( _LINUX )
	// see Sys_GetFactory( CSysModule *pModule ) for an explanation
	return (CreateInterfaceFn)( Sys_GetProcAddress( pModuleName, CREATEINTERFACE_PROCNAME ) );
#endif // _LINUX
}



