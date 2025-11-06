//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Complete Spawn List System - GMod 9.0.4b compatible
//
// $NoKeywords: $
//=============================================================================

#ifndef GMOD_SPAWNLIST_H
#define GMOD_SPAWNLIST_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "utlvector.h"
#include "utlstring.h"

//-----------------------------------------------------------------------------
// Spawn List Entry Structure
//-----------------------------------------------------------------------------
struct SpawnListEntry_t
{
    CUtlString modelPath;       // Full path to model file
    CUtlString displayName;     // Display name for UI
    CUtlString category;        // Category folder
    bool isRagdoll;            // Whether this is a ragdoll or prop
    bool isValid;              // Whether the model exists and is valid
};

//-----------------------------------------------------------------------------
// Complete Spawn List System
//-----------------------------------------------------------------------------
class CGModSpawnList
{
public:
    // System initialization
    static void Initialize();
    static void Shutdown();

    // Spawn list generation
    static void GenerateCompleteSpawnList(const char* pszFolder);
    static void SaveSpawnListToFile(const char* pszFilename);

    // Spawn list management
    static void LoadSpawnList(const char* pszFilename);
    static void ReloadSpawnMenu();

    // Entry management
    static int GetEntryCount();
    static SpawnListEntry_t* GetEntry(int index);
    static void AddEntry(const char* modelPath, const char* category = "");

    // Filtering and searching
    static void FilterByCategory(const char* category);
    static void SearchEntries(const char* searchTerm);
    static void ClearFilters();

    // Console commands
    static void CMD_gm_makecompletespawnlist(void);
    static void CMD_gm_reloadspawnmenu(void);

    // Utility functions
    static bool IsModelValid(const char* modelPath);
    static bool IsRagdollModel(const char* modelPath);
    static const char* ExtractModelName(const char* fullPath);
    static const char* ExtractCategory(const char* fullPath);

private:
    static CUtlVector<SpawnListEntry_t> m_SpawnEntries;
    static CUtlVector<SpawnListEntry_t> m_FilteredEntries;
    static bool m_bInitialized;
    static bool m_bGenerating;

    // Internal functions
    static void RecursiveScanDirectory(const char* directory, const char* baseDir);
    static void ProcessModelFile(const char* filePath, const char* baseDir);
    static void WriteFileHeader(FileHandle_t file);
    static void WriteFileEntry(FileHandle_t file, const SpawnListEntry_t& entry);
    static void WriteFileFooter(FileHandle_t file);

    // Progress tracking
    static int m_iFilesProcessed;
    static int m_iFilesTotal;
    static void UpdateProgress();
};

#endif // GMOD_SPAWNLIST_H