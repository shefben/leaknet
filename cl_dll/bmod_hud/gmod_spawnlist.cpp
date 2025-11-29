//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Complete Spawn List System Implementation
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "gmod_spawnlist.h"
#include "filesystem.h"
#include "sharedInterface.h"
#include "vstdlib/strtools.h"
#include "engine/IEngineSound.h"

// Initialize static members
CUtlVector<SpawnListEntry_t> CGModSpawnList::m_SpawnEntries;
CUtlVector<SpawnListEntry_t> CGModSpawnList::m_FilteredEntries;
bool CGModSpawnList::m_bInitialized = false;
bool CGModSpawnList::m_bGenerating = false;
int CGModSpawnList::m_iFilesProcessed = 0;
int CGModSpawnList::m_iFilesTotal = 0;

//-----------------------------------------------------------------------------
// Purpose: Basic accessors used by spawnmenu networking
//-----------------------------------------------------------------------------
int CGModSpawnList::GetEntryCount()
{
    return m_SpawnEntries.Count();
}

SpawnListEntry_t* CGModSpawnList::GetEntry(int index)
{
    if (index < 0 || index >= m_SpawnEntries.Count())
        return NULL;

    return &m_SpawnEntries[index];
}

void CGModSpawnList::AddEntry(const char* modelPath, const char* category /*= ""*/)
{
    if (!modelPath || !modelPath[0])
        return;

    SpawnListEntry_t entry;
    Q_strncpy(entry.modelPath, modelPath, sizeof(entry.modelPath));
    Q_strncpy(entry.displayName, ExtractModelName(modelPath), sizeof(entry.displayName));
    Q_strncpy(entry.category, category ? category : "", sizeof(entry.category));
    entry.isRagdoll = IsRagdollModel(modelPath);
    entry.isValid = IsModelValid(modelPath);

    m_SpawnEntries.AddToTail(entry);
}

//-----------------------------------------------------------------------------
// Purpose: Initialize the spawn list system
//-----------------------------------------------------------------------------
void CGModSpawnList::Initialize()
{
    if (m_bInitialized)
        return;

    m_SpawnEntries.Purge();
    m_FilteredEntries.Purge();
    m_bGenerating = false;
    m_iFilesProcessed = 0;
    m_iFilesTotal = 0;

    m_bInitialized = true;
    Msg("Complete Spawn List System initialized\n");
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown the spawn list system
//-----------------------------------------------------------------------------
void CGModSpawnList::Shutdown()
{
    if (!m_bInitialized)
        return;

    m_SpawnEntries.Purge();
    m_FilteredEntries.Purge();
    m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Generate complete spawn list from folder
//-----------------------------------------------------------------------------
void CGModSpawnList::GenerateCompleteSpawnList(const char* pszFolder)
{
    if (m_bGenerating)
    {
        Msg("Spawn list generation already in progress\n");
        return;
    }

    if (!pszFolder || !pszFolder[0])
    {
        Msg("Invalid folder specified\n");
        return;
    }

    Msg("Starting complete spawn list generation for folder: %s\n", pszFolder);
    Msg("WARNING: This WILL take a long time.\n");

    m_bGenerating = true;
    m_iFilesProcessed = 0;
    m_iFilesTotal = 0;
    m_SpawnEntries.Purge();

    // First pass - count files
    RecursiveScanDirectory(pszFolder, pszFolder);
    m_iFilesTotal = m_iFilesProcessed;
    m_iFilesProcessed = 0;

    Msg("Found %d potential model files to process\n", m_iFilesTotal);

    // Second pass - process files
    RecursiveScanDirectory(pszFolder, pszFolder);

    // Generate output filename
    char outputFile[MAX_PATH];
    Q_snprintf(outputFile, sizeof(outputFile), "settings/menu_props/complete_%s.txt",
               ExtractModelName(pszFolder));

    // Replace slashes in filename
    for (int i = 0; outputFile[i]; i++)
    {
        if (outputFile[i] == '/' || outputFile[i] == '\\')
            outputFile[i] = '_';
    }

    SaveSpawnListToFile(outputFile);

    m_bGenerating = false;
    Msg("Complete spawn list generation finished. Found %d valid models.\n", m_SpawnEntries.Count());
    Msg("Output saved to: %s\n", outputFile);
}

//-----------------------------------------------------------------------------
// Purpose: Recursively scan directory for model files
//-----------------------------------------------------------------------------
void CGModSpawnList::RecursiveScanDirectory(const char* directory, const char* baseDir)
{
    FileFindHandle_t findHandle;
    char searchPath[MAX_PATH];
    Q_snprintf(searchPath, sizeof(searchPath), "%s/*", directory);

    const char* pFileName = filesystem->FindFirst(searchPath, &findHandle);

    while (pFileName)
    {
        if (Q_strcmp(pFileName, ".") != 0 && Q_strcmp(pFileName, "..") != 0)
        {
            char fullPath[MAX_PATH];
            Q_snprintf(fullPath, sizeof(fullPath), "%s/%s", directory, pFileName);

            if (filesystem->FindIsDirectory(findHandle))
            {
                // Recurse into subdirectory
                RecursiveScanDirectory(fullPath, baseDir);
            }
            else
            {
                // Check if it's a model file
                if (Q_stristr(pFileName, ".mdl"))
                {
                    if (!m_bGenerating) // First pass - just count
                    {
                        m_iFilesProcessed++;
                    }
                    else // Second pass - process
                    {
                        ProcessModelFile(fullPath, baseDir);
                        UpdateProgress();
                    }
                }
            }
        }

        pFileName = filesystem->FindNext(findHandle);
    }

    filesystem->FindClose(findHandle);
}

//-----------------------------------------------------------------------------
// Purpose: Process a single model file
//-----------------------------------------------------------------------------
void CGModSpawnList::ProcessModelFile(const char* filePath, const char* baseDir)
{
    if (!IsModelValid(filePath))
        return;

    SpawnListEntry_t entry;
    Q_strncpy(entry.modelPath, filePath, sizeof(entry.modelPath));
    Q_strncpy(entry.displayName, ExtractModelName(filePath), sizeof(entry.displayName));
    Q_strncpy(entry.category, ExtractCategory(filePath), sizeof(entry.category));
    entry.isRagdoll = IsRagdollModel(filePath);
    entry.isValid = true;

    m_SpawnEntries.AddToTail(entry);
}

//-----------------------------------------------------------------------------
// Purpose: Update progress display
//-----------------------------------------------------------------------------
void CGModSpawnList::UpdateProgress()
{
    m_iFilesProcessed++;

    if (m_iFilesProcessed % 100 == 0 || m_iFilesProcessed >= m_iFilesTotal)
    {
        float percent = (float)m_iFilesProcessed / (float)m_iFilesTotal * 100.0f;
        Msg("Progress: %d/%d (%.1f%%)\n", m_iFilesProcessed, m_iFilesTotal, percent);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Save spawn list to file
//-----------------------------------------------------------------------------
void CGModSpawnList::SaveSpawnListToFile(const char* pszFilename)
{
    if (!pszFilename || m_SpawnEntries.Count() == 0)
        return;

    FileHandle_t file = filesystem->Open(pszFilename, "w", "MOD");
    if (file == FILESYSTEM_INVALID_HANDLE)
    {
        Msg("Failed to create spawn list file: %s\n", pszFilename);
        return;
    }

    WriteFileHeader(file);

    // Write all entries grouped by category - 2003 compatible version
    CUtlVector<const char*> categories;

    // Collect unique categories
    for (int i = 0; i < m_SpawnEntries.Count(); i++)
    {
        const SpawnListEntry_t& entry = m_SpawnEntries[i];

        // Check if category already exists
        bool bFound = false;
        for (int j = 0; j < categories.Count(); j++)
        {
            if (Q_strcmp(categories[j], entry.category) == 0)
            {
                bFound = true;
                break;
            }
        }

        if (!bFound)
        {
            categories.AddToTail(entry.category);
        }
    }

    // Write entries by category
    for (int c = 0; c < categories.Count(); c++)
    {
        const char* category = categories[c];

        // Write category header
        filesystem->FPrintf(file, "\n\t\"%s\"\n\t{\n", category);

        // Write all entries in this category
        for (int i = 0; i < m_SpawnEntries.Count(); i++)
        {
            const SpawnListEntry_t& entry = m_SpawnEntries[i];
            if (Q_strcmp(entry.category, category) == 0)
            {
                WriteFileEntry(file, entry);
            }
        }

        filesystem->FPrintf(file, "\t}\n");
    }

    WriteFileFooter(file);
    filesystem->Close(file);

    Msg("Spawn list saved to %s with %d entries\n", pszFilename, m_SpawnEntries.Count());
}

//-----------------------------------------------------------------------------
// Purpose: Write file header
//-----------------------------------------------------------------------------
void CGModSpawnList::WriteFileHeader(FileHandle_t file)
{
    filesystem->FPrintf(file, "\"spawnlist\"\n{\n");
    filesystem->FPrintf(file, "\t// Auto-generated complete spawn list\n");
    filesystem->FPrintf(file, "\t// Generated by gm_makecompletespawnlist\n\n");
}

//-----------------------------------------------------------------------------
// Purpose: Write file entry
//-----------------------------------------------------------------------------
void CGModSpawnList::WriteFileEntry(FileHandle_t file, const SpawnListEntry_t& entry)
{
    filesystem->FPrintf(file, "\t\t\"%s\"\n\t\t{\n", entry.displayName);
    filesystem->FPrintf(file, "\t\t\t\"model\"\t\t\"%s\"\n", entry.modelPath);

    if (entry.isRagdoll)
    {
        filesystem->FPrintf(file, "\t\t\t\"type\"\t\t\"ragdoll\"\n");
    }
    else
    {
        filesystem->FPrintf(file, "\t\t\t\"type\"\t\t\"prop\"\n");
    }

    filesystem->FPrintf(file, "\t\t}\n");
}

//-----------------------------------------------------------------------------
// Purpose: Write file footer
//-----------------------------------------------------------------------------
void CGModSpawnList::WriteFileFooter(FileHandle_t file)
{
    filesystem->FPrintf(file, "}\n");
}

//-----------------------------------------------------------------------------
// Purpose: Check if model is valid
//-----------------------------------------------------------------------------
bool CGModSpawnList::IsModelValid(const char* modelPath)
{
    if (!modelPath || !modelPath[0])
        return false;

    // Check if file exists
    return filesystem->FileExists(modelPath, "GAME");
}

//-----------------------------------------------------------------------------
// Purpose: Check if model is a ragdoll
//-----------------------------------------------------------------------------
bool CGModSpawnList::IsRagdollModel(const char* modelPath)
{
    if (!modelPath)
        return false;

    // Simple heuristic - check for common ragdoll indicators
    const char* lowerPath = Q_strlower((char*)modelPath);

    return (Q_stristr(lowerPath, "ragdoll") != NULL ||
            Q_stristr(lowerPath, "corpse") != NULL ||
            Q_stristr(lowerPath, "dead") != NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Extract model name from path
//-----------------------------------------------------------------------------
const char* CGModSpawnList::ExtractModelName(const char* fullPath)
{
    if (!fullPath)
        return "unknown";

    const char* lastSlash = max(strrchr(fullPath, '/'), strrchr(fullPath, '\\'));
    if (lastSlash)
    {
        const char* name = lastSlash + 1;

        // Remove .mdl extension
        static char nameBuffer[256];
        Q_strncpy(nameBuffer, name, sizeof(nameBuffer));

        char* dot = strrchr(nameBuffer, '.');
        if (dot)
            *dot = '\0';

        return nameBuffer;
    }

    return fullPath;
}

//-----------------------------------------------------------------------------
// Purpose: Extract category from path
//-----------------------------------------------------------------------------
const char* CGModSpawnList::ExtractCategory(const char* fullPath)
{
    if (!fullPath)
        return "Other";

    static char categoryBuffer[256];
    Q_strncpy(categoryBuffer, fullPath, sizeof(categoryBuffer));

    // Remove filename
    char* lastSlash = max(strrchr(categoryBuffer, '/'), strrchr(categoryBuffer, '\\'));
    if (lastSlash)
        *lastSlash = '\0';

    // Get last directory name as category
    lastSlash = max(strrchr(categoryBuffer, '/'), strrchr(categoryBuffer, '\\'));
    if (lastSlash)
        return lastSlash + 1;

    return "Other";
}

//-----------------------------------------------------------------------------
// Purpose: Reload spawn menu
//-----------------------------------------------------------------------------
void CGModSpawnList::ReloadSpawnMenu()
{
    // This would trigger a reload of the spawn menu UI
    // Implementation depends on spawn menu system
    Msg("Spawn menu reloaded\n");
}

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Generate complete spawn list command
//-----------------------------------------------------------------------------
void CGModSpawnList::CMD_gm_makecompletespawnlist(void)
{
    CCommand args;
    if (args.ArgC() < 2)
    {
        Msg("Usage: gm_makecompletespawnlist <folder>\n");
        Msg("Example: gm_makecompletespawnlist cstrike/models/\n");
        Msg("WARNING: this WILL take a long time.\n");
        return;
    }

    const char* folder = args.Arg(1);
    GenerateCompleteSpawnList(folder);
}

//-----------------------------------------------------------------------------
// Purpose: Reload spawn menu command
//-----------------------------------------------------------------------------
void CGModSpawnList::CMD_gm_reloadspawnmenu(void)
{
    ReloadSpawnMenu();
}

static ConCommand gm_makecompletespawnlist("gm_makecompletespawnlist",
    CGModSpawnList::CMD_gm_makecompletespawnlist,
    "gm_makecompletespawnlist <folder> - makes a complete spawn list from this folder. Outputs to .txt. Doesn't determine whether a ragdoll or not.");

static ConCommand gm_reloadspawnmenu("gm_reloadspawnmenu",
    CGModSpawnList::CMD_gm_reloadspawnmenu,
    "Reload the spawn menu");

//-----------------------------------------------------------------------------
// Client initialization hook
//-----------------------------------------------------------------------------
class CSpawnListInit : public CAutoGameSystem
{
public:
    CSpawnListInit() : CAutoGameSystem("SpawnListInit") {}

    virtual bool Init()
    {
        CGModSpawnList::Initialize();
        return true;
    }

    virtual void Shutdown()
    {
        CGModSpawnList::Shutdown();
    }
};

static CSpawnListInit g_SpawnListInit;
