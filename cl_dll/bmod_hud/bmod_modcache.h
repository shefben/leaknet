//========= Copyright (c) 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Mod Cache Manager - Handles loading files from mods/-modcache
//          Based on Garry's Mod 9 mod system implementation
//
//=============================================================================

#ifndef BMOD_MODCACHE_H
#define BMOD_MODCACHE_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"
#include "filesystem.h"

//-----------------------------------------------------------------------------
// Mod entry structure - represents a single mod in the mods/ folder
//-----------------------------------------------------------------------------
struct ModEntry_t
{
	char szName[64];			// Mod display name (from modinfo.txt)
	char szFolderName[64];		// Folder name in mods/
	char szPath[256];			// Full path to mod folder
	char szVersion[32];			// Mod version
	char szAuthor[64];			// Author name
	char szAuthorEmail[128];	// Author email
	char szAuthorWeb[128];		// Author website
	char szIcon[64];			// Icon material name
	bool bEnabled;				// Whether mod is enabled
	bool bHasModInfo;			// Whether modinfo.txt exists
};

//-----------------------------------------------------------------------------
// CModCacheManager - Singleton class to manage mod loading
//-----------------------------------------------------------------------------
class CModCacheManager
{
public:
	CModCacheManager();
	~CModCacheManager();

	// Initialize the mod system
	void Initialize();

	// Scan for available mods in mods/ folder
	void ScanForMods();

	// Get list of mods
	int GetModCount() const { return m_Mods.Count(); }
	const ModEntry_t *GetMod( int index ) const;
	ModEntry_t *GetModByName( const char *szFolderName );

	// Enable/disable a mod
	void EnableMod( const char *szFolderName );
	void DisableMod( const char *szFolderName );
	void ToggleMod( const char *szFolderName );

	// Copy mod assets to modcache
	void CopyModToCache( const char *szFolderName );
	void RemoveModFromCache( const char *szFolderName );

	// Refresh the modcache (copy all enabled mod files)
	void RefreshModCache();

	// Save/Load mod enabled states
	void SaveModStates();
	void LoadModStates();

private:
	// Parse modinfo.txt for a mod
	void ParseModInfo( ModEntry_t &mod );

	// Copy a file from mod folder to modcache
	bool CopyFileToCache( const char *szModFolder, const char *szRelativePath );

	// Recursively copy directory contents
	void CopyDirectoryToCache( const char *szModFolder, const char *szSubDir );

	CUtlVector<ModEntry_t> m_Mods;
	bool m_bInitialized;
};

//-----------------------------------------------------------------------------
// Global mod cache manager instance
//-----------------------------------------------------------------------------
CModCacheManager *GetModCacheManager();

//-----------------------------------------------------------------------------
// Console commands
//-----------------------------------------------------------------------------
void CC_ModList();
void CC_ModEnable();
void CC_ModDisable();
void CC_ModRefresh();

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define MODCACHE_PATH		"mods/-modcache"
#define MODCACHE_STATE_FILE	"mods/-modcache/modcache.txt"
#define MODS_FOLDER			"mods"
#define MODINFO_FILE		"modinfo.txt"

#endif // BMOD_MODCACHE_H
