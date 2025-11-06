//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "materialsystem/IMaterialSystem.h"
#include "materialsystem/MaterialSystem_Config.h"
#include <cmdlib.h>
#include "tier0/dbg.h"
#include <windows.h>
#include "FileSystem.h"
#include "FileSystem_Tools.h"
#include "filesystem.h"
#include "engine/ISharedModelLoader.h"

IMaterialSystem *g_pMaterialSystem = NULL;
CreateInterfaceFn g_MatSysFactory = NULL;
CreateInterfaceFn g_ShaderAPIFactory = NULL;
IBaseFileSystem *filesystem = NULL;
ISharedModelLoader *sharedmodelloader = NULL;

static void LoadMaterialSystem( void )
{
	if( g_pMaterialSystem )
		return;
	
	const char *pDllName = "materialsystem" DLL_EXT_STRING;
	HINSTANCE materialSystemDLLHInst;
	materialSystemDLLHInst = LoadLibrary( pDllName );
	if( !materialSystemDLLHInst )
	{
		Error( "Can't load materialsystem%s\n", DLL_EXT_STRING );
	}

	g_MatSysFactory = Sys_GetFactory( "materialsystem" DLL_EXT_STRING );
	if ( g_MatSysFactory )
	{
		g_pMaterialSystem = (IMaterialSystem *)g_MatSysFactory( MATERIAL_SYSTEM_INTERFACE_VERSION, NULL );
		if ( !g_pMaterialSystem )
		{
			Error( "Could not get the material system interface from materialsystem%s", DLL_EXT_STRING );
		}
	}
	else
	{
		Error( "Could not find factory interface in library materialsystem%s", DLL_EXT_STRING );
	}

	FileSystem_Init();
	if (!( g_ShaderAPIFactory = g_pMaterialSystem->Init( "shaderapiempty" DLL_EXT_STRING, 0, FileSystem_GetFactory() )) )
	{
		Error( "Could not start the empty shader (shaderapiempty%s)!", DLL_EXT_STRING );
	}

	// VXP: Shared anim group routine
	HINSTANCE engineDLLHInst = LoadLibrary( "engine" DLL_EXT_STRING );
	if( !engineDLLHInst )
	{
		Error( "Can't load engine%s\n", DLL_EXT_STRING );
	}

	CreateInterfaceFn g_EngineFactory = Sys_GetFactory( "engine" DLL_EXT_STRING );
	if (!g_EngineFactory)
	{
		Error( "Could not find factory interface in library engine%s", DLL_EXT_STRING );
	}

	filesystem = ( IBaseFileSystem * )FileSystem_GetFactory()( BASEFILESYSTEM_INTERFACE_VERSION, NULL );
	if ( !filesystem )
	{
		Error( "Could not get filesystem interface in library filesystem_stdio%s", DLL_EXT_STRING );
	}
	sharedmodelloader = (ISharedModelLoader *)g_EngineFactory( ISHAREDMODELLOADER_INTERFACE_VERSION, NULL );
	if ( !sharedmodelloader )
	{
		Error( "Could not get the shared model loader interface from engine%s", DLL_EXT_STRING );
	}

	sharedmodelloader->InitFilesystem( filesystem );
}

void InitMaterialSystem( const char *materialBaseDirPath )
{
	MaterialSystem_Config_t config;

	memset( &config, 0, sizeof(config) );
	config.screenGamma = 2.2f;
	config.texGamma = 2.2f;
	config.overbright = 1.0f;
	config.bAllowCheats = false;
	config.bLinearFrameBuffer = false;
	config.polyOffset = 0.0f;
	config.skipMipLevels = 0;
	config.lightScale = 1.0f;
	config.bFilterLightmaps = false;
	config.bFilterTextures = false;
	config.bMipMapTextures = false;
	config.bBumpmap = true;
	config.bShowSpecular = true;
	config.bShowDiffuse = true;
	config.maxFrameLatency = 1;
	config.bLightingOnly = false;
	config.bCompressedTextures = false;
	config.bShowMipLevels = false;
	config.bEditMode = false;	// No, we're not in WorldCraft.
	config.m_bForceTrilinear = false;
	config.m_nForceAnisotropicLevel = 0;
	config.m_bForceBilinear = false;

	LoadMaterialSystem();
	g_pMaterialSystem->UpdateConfig( &config, false );
}
