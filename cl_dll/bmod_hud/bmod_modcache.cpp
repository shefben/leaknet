//========= Copyright (c) 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Mod Cache Manager - Handles loading files from mods/-modcache
//          Based on Garry's Mod 9 mod system implementation
//
//=============================================================================

#include "cbase.h"
#include "bmod_modcache.h"
#include "filesystem.h"
#include "cdll_int.h"
#include "KeyValues.h"
#include "tier0/dbg.h"
#include "convar.h"
#include "iclientmode.h"

// Avoid ambiguous filesystem reference
#define g_pFullFilesystem (::filesystem)

// Helper to strip filename from path, leaving just the directory
static void StripFilenameFromPath( char *path )
{
	char *lastSlash = NULL;
	for ( char *p = path; *p; p++ )
	{
		if ( *p == '/' || *p == '\\' )
			lastSlash = p;
	}
	if ( lastSlash )
		*lastSlash = '\0';
}

// Global singleton instance
static CModCacheManager g_ModCacheManager;

CModCacheManager *GetModCacheManager()
{
	return &g_ModCacheManager;
}

static void AddModCacheSearchPaths()
{
	static bool s_bSearchPathsAdded = false;
	if ( s_bSearchPathsAdded || !g_pFullFilesystem || !engine )
		return;

	const char *pModPath = engine->GetGameDirectory();
	if ( !pModPath || !pModPath[0] )
		return;

	char modcacheDir[MAX_PATH];
	Q_snprintf( modcacheDir, sizeof( modcacheDir ), "%s/%s", pModPath, MODCACHE_PATH );

	if ( !g_pFullFilesystem->IsDirectory( modcacheDir, NULL ) )
		return;

	g_pFullFilesystem->AddSearchPath( modcacheDir, "GAME", PATH_ADD_TO_TAIL );
	g_pFullFilesystem->AddSearchPath( modcacheDir, "MOD", PATH_ADD_TO_TAIL );
	s_bSearchPathsAdded = true;

	Msg( "BarrysMod: Added modcache search path: %s\n", modcacheDir );
}

class CModCacheInit : public CAutoGameSystem
{
public:
	CModCacheInit() : CAutoGameSystem( "CModCacheInit" ) {}

	virtual bool Init()
	{
		GetModCacheManager()->Initialize();
		return true;
	}
};

static CModCacheInit g_ModCacheInit;

//-----------------------------------------------------------------------------
// Console commands
//-----------------------------------------------------------------------------
CON_COMMAND( bmod_modlist, "List all available mods" )
{
	GetModCacheManager()->ScanForMods();

	int count = GetModCacheManager()->GetModCount();
	Msg( "Available mods (%d):\n", count );

	for ( int i = 0; i < count; i++ )
	{
		const ModEntry_t *pMod = GetModCacheManager()->GetMod( i );
		if ( pMod )
		{
			Msg( "  [%s] %s (%s) - %s\n",
				pMod->bEnabled ? "ENABLED" : "DISABLED",
				pMod->szName,
				pMod->szFolderName,
				pMod->szVersion[0] ? pMod->szVersion : "n/a" );
		}
	}
}

CON_COMMAND( bmod_modenable, "Enable a mod: bmod_modenable <modname>" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: bmod_modenable <modname>\n" );
		return;
	}

	GetModCacheManager()->EnableMod( engine->Cmd_Argv( 1 ) );
}

CON_COMMAND( bmod_moddisable, "Disable a mod: bmod_moddisable <modname>" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: bmod_moddisable <modname>\n" );
		return;
	}

	GetModCacheManager()->DisableMod( engine->Cmd_Argv( 1 ) );
}

CON_COMMAND( bmod_modrefresh, "Refresh the mod cache (copy enabled mod files)" )
{
	GetModCacheManager()->RefreshModCache();
}

CON_COMMAND( bmod_modclear, "Clear the mod cache (forces a rebuild on next startup)" )
{
	filesystem->RemoveFile( MODCACHE_MANIFEST_FILE, "MOD" );
	Msg( "BarrysMod: Mod cache manifest cleared - cache will rebuild on next startup\n" );
}

//-----------------------------------------------------------------------------
// CModCacheManager implementation
//-----------------------------------------------------------------------------
CModCacheManager::CModCacheManager()
{
	m_bInitialized = false;
}

CModCacheManager::~CModCacheManager()
{
	m_Mods.Purge();
}

void CModCacheManager::Initialize()
{
	if ( m_bInitialized )
		return;

	// Ensure modcache directories exist
	g_pFullFilesystem->CreateDirHierarchy( "mods/-modcache/materials", "MOD" );
	g_pFullFilesystem->CreateDirHierarchy( "mods/-modcache/models", "MOD" );
	g_pFullFilesystem->CreateDirHierarchy( "mods/-modcache/maps", "MOD" );
	g_pFullFilesystem->CreateDirHierarchy( "mods/-modcache/sound", "MOD" );
	g_pFullFilesystem->CreateDirHierarchy( "mods/-modcache/settings", "MOD" );
	g_pFullFilesystem->CreateDirHierarchy( "mods/-modcache/lua", "MOD" );
	g_pFullFilesystem->CreateDirHierarchy( "mods/-modcache/scripts", "MOD" );
	g_pFullFilesystem->CreateDirHierarchy( "mods/-modcache/resource", "MOD" );
	g_pFullFilesystem->CreateDirHierarchy( "mods/-modcache/cfg", "MOD" );
	g_pFullFilesystem->CreateDirHierarchy( "mods/-modcache/particles", "MOD" );
	g_pFullFilesystem->CreateDirHierarchy( "mods/-modcache/spawnicons", "MOD" );

	AddModCacheSearchPaths();

	// Scan for available mods
	ScanForMods();

	LoadModStates();

	// Only rebuild the cache when the mod set actually changed (new/removed
	// mods, enable state flips, or modified files) or the cache was cleared.
	if ( IsCacheUpToDate() )
	{
		Msg( "BarrysMod: Mod cache is up to date, skipping rebuild\n" );
	}
	else
	{
		RefreshModCache();
	}

	m_bInitialized = true;

	Msg( "BarrysMod: Mod cache manager initialized\n" );
}

void CModCacheManager::ScanForMods()
{
	m_Mods.RemoveAll();

	// Find all directories in mods/
	FileFindHandle_t findHandle;
	const char *pFileName = g_pFullFilesystem->FindFirst( "mods/*", &findHandle );

	while ( pFileName )
	{
		// Skip . and ..
		if ( Q_strcmp( pFileName, "." ) == 0 || Q_strcmp( pFileName, ".." ) == 0 )
		{
			pFileName = g_pFullFilesystem->FindNext( findHandle );
			continue;
		}

		// Skip the modcache folder itself
		if ( Q_strcmp( pFileName, "-modcache" ) == 0 )
		{
			pFileName = g_pFullFilesystem->FindNext( findHandle );
			continue;
		}

		// Check if it's a directory
		char fullPath[256];
		Q_snprintf( fullPath, sizeof( fullPath ), "mods/%s", pFileName );

		if ( g_pFullFilesystem->IsDirectory( fullPath, "MOD" ) )
		{
			// Check if modinfo.txt exists
			char modInfoPath[256];
			Q_snprintf( modInfoPath, sizeof( modInfoPath ), "mods/%s/modinfo.txt", pFileName );

			ModEntry_t mod;
			Q_memset( &mod, 0, sizeof( mod ) );
			Q_strncpy( mod.szFolderName, pFileName, sizeof( mod.szFolderName ) );
			Q_snprintf( mod.szPath, sizeof( mod.szPath ), "mods/%s", pFileName );
			mod.bHasModInfo = g_pFullFilesystem->FileExists( modInfoPath, "MOD" );
			mod.bEnabled = false;

			if ( mod.bHasModInfo )
			{
				ParseModInfo( mod );
			}
			else
			{
				// Use folder name as display name if no modinfo.txt
				Q_strncpy( mod.szName, pFileName, sizeof( mod.szName ) );
			}

			// Check if DISABLED marker file exists
			char disabledPath[256];
			Q_snprintf( disabledPath, sizeof( disabledPath ), "mods/%s/DISABLED", pFileName );
			mod.bEnabled = !g_pFullFilesystem->FileExists( disabledPath, "MOD" );

			m_Mods.AddToTail( mod );
		}

		pFileName = g_pFullFilesystem->FindNext( findHandle );
	}

	g_pFullFilesystem->FindClose( findHandle );

	Msg( "BarrysMod: Found %d mods\n", m_Mods.Count() );
}

const ModEntry_t *CModCacheManager::GetMod( int index ) const
{
	if ( index < 0 || index >= m_Mods.Count() )
		return NULL;

	return &m_Mods[index];
}

ModEntry_t *CModCacheManager::GetModByName( const char *szFolderName )
{
	for ( int i = 0; i < m_Mods.Count(); i++ )
	{
		if ( Q_stricmp( m_Mods[i].szFolderName, szFolderName ) == 0 )
		{
			return &m_Mods[i];
		}
	}
	return NULL;
}

void CModCacheManager::ParseModInfo( ModEntry_t &mod )
{
	char modInfoPath[256];
	Q_snprintf( modInfoPath, sizeof( modInfoPath ), "mods/%s/modinfo.txt", mod.szFolderName );

	KeyValues *pKV = new KeyValues( "ModInfo" );
	if ( pKV->LoadFromFile( g_pFullFilesystem, modInfoPath, "MOD" ) )
	{
		Q_strncpy( mod.szName, pKV->GetString( "name", mod.szFolderName ), sizeof( mod.szName ) );
		Q_strncpy( mod.szVersion, pKV->GetString( "version", "" ), sizeof( mod.szVersion ) );
		Q_strncpy( mod.szAuthor, pKV->GetString( "author_name", "" ), sizeof( mod.szAuthor ) );
		Q_strncpy( mod.szAuthorEmail, pKV->GetString( "author_email", "" ), sizeof( mod.szAuthorEmail ) );
		Q_strncpy( mod.szAuthorWeb, pKV->GetString( "author_web", "" ), sizeof( mod.szAuthorWeb ) );
		Q_strncpy( mod.szIcon, pKV->GetString( "icon", "" ), sizeof( mod.szIcon ) );
	}
	else
	{
		// Fallback to folder name
		Q_strncpy( mod.szName, mod.szFolderName, sizeof( mod.szName ) );
	}

	pKV->deleteThis();
}

void CModCacheManager::EnableMod( const char *szFolderName )
{
	ModEntry_t *pMod = GetModByName( szFolderName );
	if ( !pMod )
	{
		Msg( "BarrysMod: Mod '%s' not found\n", szFolderName );
		return;
	}

	if ( pMod->bEnabled )
	{
		Msg( "BarrysMod: Mod '%s' is already enabled\n", szFolderName );
		return;
	}

	// Remove the DISABLED marker file
	char disabledPath[256];
	Q_snprintf( disabledPath, sizeof( disabledPath ), "mods/%s/DISABLED", szFolderName );
	g_pFullFilesystem->RemoveFile( disabledPath, "MOD" );

	pMod->bEnabled = true;

	// Copy mod files to cache
	CopyModToCache( szFolderName );

	SaveModStates();
	SaveCacheManifest();

	Msg( "BarrysMod: Enabled mod '%s'. Restart required for changes to take effect.\n", pMod->szName );
}

void CModCacheManager::DisableMod( const char *szFolderName )
{
	ModEntry_t *pMod = GetModByName( szFolderName );
	if ( !pMod )
	{
		Msg( "BarrysMod: Mod '%s' not found\n", szFolderName );
		return;
	}

	if ( !pMod->bEnabled )
	{
		Msg( "BarrysMod: Mod '%s' is already disabled\n", szFolderName );
		return;
	}

	// Create the DISABLED marker file
	char disabledPath[256];
	Q_snprintf( disabledPath, sizeof( disabledPath ), "mods/%s/DISABLED", szFolderName );

	FileHandle_t hFile = g_pFullFilesystem->Open( disabledPath, "w", "MOD" );
	if ( hFile )
	{
		g_pFullFilesystem->Close( hFile );
	}

	pMod->bEnabled = false;

	SaveModStates();

	// The disabled mod's files are still in the cache; drop the manifest so
	// the next startup rebuilds without them.
	g_pFullFilesystem->RemoveFile( MODCACHE_MANIFEST_FILE, "MOD" );

	Msg( "BarrysMod: Disabled mod '%s'. Restart required for changes to take effect.\n", pMod->szName );
}

void CModCacheManager::ToggleMod( const char *szFolderName )
{
	ModEntry_t *pMod = GetModByName( szFolderName );
	if ( !pMod )
	{
		Msg( "BarrysMod: Mod '%s' not found\n", szFolderName );
		return;
	}

	if ( pMod->bEnabled )
	{
		DisableMod( szFolderName );
	}
	else
	{
		EnableMod( szFolderName );
	}
}

void CModCacheManager::CopyModToCache( const char *szFolderName )
{
	ModEntry_t *pMod = GetModByName( szFolderName );
	if ( !pMod )
		return;

	Msg( "BarrysMod: Copying mod '%s' files to cache...\n", pMod->szName );

	// Copy materials
	CopyDirectoryToCache( szFolderName, "materials" );

	// Copy models
	CopyDirectoryToCache( szFolderName, "models" );

	// Copy maps
	CopyDirectoryToCache( szFolderName, "maps" );

	// Copy sound
	CopyDirectoryToCache( szFolderName, "sound" );

	// Copy GMod data/config folders. These feed menu_props, context panels,
	// level lists, NPC weapon presets, SWEPs, weapon scripts, and UI resources.
	CopyDirectoryToCache( szFolderName, "settings" );
	CopyDirectoryToCache( szFolderName, "lua" );
	CopyDirectoryToCache( szFolderName, "scripts" );
	CopyDirectoryToCache( szFolderName, "resource" );
	CopyDirectoryToCache( szFolderName, "cfg" );
	CopyDirectoryToCache( szFolderName, "particles" );
	CopyDirectoryToCache( szFolderName, "spawnicons" );

	// Copy the icon if it exists
	if ( pMod->szIcon[0] )
	{
		char iconVmt[256], iconVtf[256];
		Q_snprintf( iconVmt, sizeof( iconVmt ), "materials/%s.vmt", pMod->szIcon );
		Q_snprintf( iconVtf, sizeof( iconVtf ), "materials/%s.vtf", pMod->szIcon );

		CopyFileToCache( szFolderName, iconVmt );
		CopyFileToCache( szFolderName, iconVtf );
	}

	Msg( "BarrysMod: Finished copying mod '%s' files\n", pMod->szName );
}

void CModCacheManager::RemoveModFromCache( const char *szFolderName )
{
	// Note: This is a simplified version - a full implementation would
	// track which files came from which mod and remove only those
	Msg( "BarrysMod: RemoveModFromCache not fully implemented\n" );
}

void CModCacheManager::ClearDirectoryFiles( const char *szRelativeDir )
{
	char searchPath[256];
	Q_snprintf( searchPath, sizeof( searchPath ), "%s/*", szRelativeDir );

	FileFindHandle_t findHandle = FILESYSTEM_INVALID_FIND_HANDLE;
	const char *pFileName = g_pFullFilesystem->FindFirst( searchPath, &findHandle );
	while ( pFileName )
	{
		if ( Q_strcmp( pFileName, "." ) != 0 && Q_strcmp( pFileName, ".." ) != 0 )
		{
			char childPath[256];
			Q_snprintf( childPath, sizeof( childPath ), "%s/%s", szRelativeDir, pFileName );

			if ( g_pFullFilesystem->IsDirectory( childPath, "MOD" ) )
			{
				ClearDirectoryFiles( childPath );
			}
			else
			{
				g_pFullFilesystem->RemoveFile( childPath, "MOD" );
			}
		}

		pFileName = g_pFullFilesystem->FindNext( findHandle );
	}

	if ( findHandle != FILESYSTEM_INVALID_FIND_HANDLE )
	{
		g_pFullFilesystem->FindClose( findHandle );
	}
}

void CModCacheManager::ClearCachedContent()
{
	ClearDirectoryFiles( "mods/-modcache/materials" );
	ClearDirectoryFiles( "mods/-modcache/models" );
	ClearDirectoryFiles( "mods/-modcache/maps" );
	ClearDirectoryFiles( "mods/-modcache/sound" );
	ClearDirectoryFiles( "mods/-modcache/settings" );
	ClearDirectoryFiles( "mods/-modcache/lua" );
	ClearDirectoryFiles( "mods/-modcache/scripts" );
	ClearDirectoryFiles( "mods/-modcache/resource" );
	ClearDirectoryFiles( "mods/-modcache/cfg" );
	ClearDirectoryFiles( "mods/-modcache/particles" );
	ClearDirectoryFiles( "mods/-modcache/spawnicons" );
}

void CModCacheManager::RefreshModCache()
{
	Msg( "BarrysMod: Refreshing mod cache...\n" );

	// Rescan for mods
	ScanForMods();
	LoadModStates();
	ClearCachedContent();

	// Copy all enabled mods to cache
	for ( int i = 0; i < m_Mods.Count(); i++ )
	{
		if ( m_Mods[i].bEnabled )
		{
			CopyModToCache( m_Mods[i].szFolderName );
		}
	}

	SaveCacheManifest();

	Msg( "BarrysMod: Mod cache refresh complete\n" );
}

//-----------------------------------------------------------------------------
// Cache manifest - records the mod set the cache was built from so startup
// can skip the (slow) full rebuild when nothing changed.
//-----------------------------------------------------------------------------
void CModCacheManager::BuildDirectorySignature( const char *szRelativeDir, int &fileCount, long &newestTime )
{
	char searchPath[256];
	Q_snprintf( searchPath, sizeof( searchPath ), "%s/*", szRelativeDir );

	FileFindHandle_t findHandle = FILESYSTEM_INVALID_FIND_HANDLE;
	const char *pFileName = g_pFullFilesystem->FindFirst( searchPath, &findHandle );
	while ( pFileName )
	{
		if ( Q_strcmp( pFileName, "." ) != 0 && Q_strcmp( pFileName, ".." ) != 0 )
		{
			char childPath[256];
			Q_snprintf( childPath, sizeof( childPath ), "%s/%s", szRelativeDir, pFileName );

			if ( g_pFullFilesystem->IsDirectory( childPath, "MOD" ) )
			{
				BuildDirectorySignature( childPath, fileCount, newestTime );
			}
			else
			{
				fileCount++;
				long fileTime = g_pFullFilesystem->GetFileTime( childPath, "MOD" );
				if ( fileTime > newestTime )
					newestTime = fileTime;
			}
		}

		pFileName = g_pFullFilesystem->FindNext( findHandle );
	}

	if ( findHandle != FILESYSTEM_INVALID_FIND_HANDLE )
	{
		g_pFullFilesystem->FindClose( findHandle );
	}
}

void CModCacheManager::BuildModSignature( const char *szFolderName, int &fileCount, long &newestTime )
{
	fileCount = 0;
	newestTime = 0;

	char modDir[256];
	Q_snprintf( modDir, sizeof( modDir ), "mods/%s", szFolderName );
	BuildDirectorySignature( modDir, fileCount, newestTime );
}

void CModCacheManager::SaveCacheManifest()
{
	KeyValues *pKV = new KeyValues( "CacheManifest" );

	for ( int i = 0; i < m_Mods.Count(); i++ )
	{
		int fileCount;
		long newestTime;
		BuildModSignature( m_Mods[i].szFolderName, fileCount, newestTime );

		KeyValues *pModKV = pKV->CreateNewKey();
		pModKV->SetName( m_Mods[i].szFolderName );
		pModKV->SetInt( "enabled", m_Mods[i].bEnabled ? 1 : 0 );
		pModKV->SetInt( "filecount", fileCount );
		pModKV->SetInt( "newesttime", (int)newestTime );
	}

	pKV->SaveToFile( g_pFullFilesystem, MODCACHE_MANIFEST_FILE, "MOD" );
	pKV->deleteThis();
}

bool CModCacheManager::IsCacheUpToDate()
{
	// No manifest = cache was cleared or never built
	if ( !g_pFullFilesystem->FileExists( MODCACHE_MANIFEST_FILE, "MOD" ) )
		return false;

	KeyValues *pKV = new KeyValues( "CacheManifest" );
	if ( !pKV->LoadFromFile( g_pFullFilesystem, MODCACHE_MANIFEST_FILE, "MOD" ) )
	{
		pKV->deleteThis();
		return false;
	}

	bool bUpToDate = true;

	// Every current mod must match its recorded signature
	int manifestCount = 0;
	for ( KeyValues *pSubKey = pKV->GetFirstSubKey(); pSubKey; pSubKey = pSubKey->GetNextKey() )
		manifestCount++;

	if ( manifestCount != m_Mods.Count() )
	{
		bUpToDate = false;
	}
	else
	{
		for ( int i = 0; i < m_Mods.Count(); i++ )
		{
			KeyValues *pModKV = pKV->FindKey( m_Mods[i].szFolderName );
			if ( !pModKV )
			{
				bUpToDate = false;
				break;
			}

			int fileCount;
			long newestTime;
			BuildModSignature( m_Mods[i].szFolderName, fileCount, newestTime );

			if ( pModKV->GetInt( "enabled", -1 ) != ( m_Mods[i].bEnabled ? 1 : 0 ) ||
				 pModKV->GetInt( "filecount", -1 ) != fileCount ||
				 pModKV->GetInt( "newesttime", -1 ) != (int)newestTime )
			{
				bUpToDate = false;
				break;
			}
		}
	}

	pKV->deleteThis();
	return bUpToDate;
}

bool CModCacheManager::CopyFileToCache( const char *szModFolder, const char *szRelativePath )
{
	char srcPath[256], dstPath[256];
	Q_snprintf( srcPath, sizeof( srcPath ), "mods/%s/%s", szModFolder, szRelativePath );
	Q_snprintf( dstPath, sizeof( dstPath ), "mods/-modcache/%s", szRelativePath );

	// Check if source file exists
	if ( !g_pFullFilesystem->FileExists( srcPath, "MOD" ) )
		return false;

	// Read source file
	FileHandle_t hSrc = g_pFullFilesystem->Open( srcPath, "rb", "MOD" );
	if ( !hSrc )
		return false;

	int fileSize = g_pFullFilesystem->Size( hSrc );
	if ( fileSize <= 0 )
	{
		g_pFullFilesystem->Close( hSrc );
		return false;
	}

	// Allocate buffer and read
	byte *pBuffer = new byte[fileSize];
	g_pFullFilesystem->Read( pBuffer, fileSize, hSrc );
	g_pFullFilesystem->Close( hSrc );

	// Ensure destination directory exists
	char dstDir[256];
	Q_strncpy( dstDir, dstPath, sizeof( dstDir ) );
	StripFilenameFromPath( dstDir );
	g_pFullFilesystem->CreateDirHierarchy( dstDir, "MOD" );

	// Write destination file
	FileHandle_t hDst = g_pFullFilesystem->Open( dstPath, "wb", "MOD" );
	if ( !hDst )
	{
		delete[] pBuffer;
		return false;
	}

	g_pFullFilesystem->Write( pBuffer, fileSize, hDst );
	g_pFullFilesystem->Close( hDst );

	delete[] pBuffer;
	return true;
}

void CModCacheManager::CopyDirectoryToCache( const char *szModFolder, const char *szSubDir )
{
	char searchPath[256];
	Q_snprintf( searchPath, sizeof( searchPath ), "mods/%s/%s/*", szModFolder, szSubDir );

	FileFindHandle_t findHandle;
	const char *pFileName = g_pFullFilesystem->FindFirst( searchPath, &findHandle );

	while ( pFileName )
	{
		// Skip . and ..
		if ( Q_strcmp( pFileName, "." ) == 0 || Q_strcmp( pFileName, ".." ) == 0 )
		{
			pFileName = g_pFullFilesystem->FindNext( findHandle );
			continue;
		}

		char fullPath[256], relativePath[256];
		Q_snprintf( fullPath, sizeof( fullPath ), "mods/%s/%s/%s", szModFolder, szSubDir, pFileName );
		Q_snprintf( relativePath, sizeof( relativePath ), "%s/%s", szSubDir, pFileName );

		if ( g_pFullFilesystem->IsDirectory( fullPath, "MOD" ) )
		{
			// Recurse into subdirectory
			CopyDirectoryToCache( szModFolder, relativePath );
		}
		else
		{
			// Copy file
			CopyFileToCache( szModFolder, relativePath );
		}

		pFileName = g_pFullFilesystem->FindNext( findHandle );
	}

	g_pFullFilesystem->FindClose( findHandle );
}

void CModCacheManager::SaveModStates()
{
	KeyValues *pKV = new KeyValues( "ModCache" );

	for ( int i = 0; i < m_Mods.Count(); i++ )
	{
		KeyValues *pModKV = pKV->CreateNewKey();
		pModKV->SetName( m_Mods[i].szFolderName );
		pModKV->SetInt( "enabled", m_Mods[i].bEnabled ? 1 : 0 );
	}

	pKV->SaveToFile( g_pFullFilesystem, MODCACHE_STATE_FILE, "MOD" );
	pKV->deleteThis();
}

void CModCacheManager::LoadModStates()
{
	KeyValues *pKV = new KeyValues( "ModCache" );
	if ( pKV->LoadFromFile( g_pFullFilesystem, MODCACHE_STATE_FILE, "MOD" ) )
	{
		for ( KeyValues *pSubKey = pKV->GetFirstSubKey(); pSubKey; pSubKey = pSubKey->GetNextKey() )
		{
			ModEntry_t *pMod = GetModByName( pSubKey->GetName() );
			if ( pMod )
			{
				pMod->bEnabled = pSubKey->GetInt( "enabled", 0 ) != 0;
			}
		}
	}
	pKV->deleteThis();
}
