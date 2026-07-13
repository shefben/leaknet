//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Completely independent v44+ model loading and management system
//          Separate from v37 models to avoid casting and compatibility issues
//
//=============================================================================

#include "glquake.h"
#include "gl_model_private.h"
#include "modelloader_v44.h"
#include "modelgen.h"
#include "filesystem.h"
#include "filesystem_engine.h"
// NO studio.h - v44+ code is completely isolated from v37
#include "studiohdr_v44.h"  // All v44+ types defined here
#include "optimize.h"
#include "vstdlib/strtools.h"
#include "zone.h"

// Model loader flags (from modelloader.h to avoid circular include)
#define FMODELLOADER_NOTLOADEDORREFERENCED 0
#define FMODELLOADER_LOADED (1<<0)

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Global v44+ model loader instance
//-----------------------------------------------------------------------------
CModelLoader_v44 *g_pModelLoader_v44 = NULL;

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CModelLoader_v44::CModelLoader_v44()
{
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CModelLoader_v44::~CModelLoader_v44()
{
	Shutdown();
}

//-----------------------------------------------------------------------------
// Initialize the v44+ model system
//-----------------------------------------------------------------------------
bool CModelLoader_v44::Init()
{
	Con_DPrintf("Initializing v44+ model loader system...\n");
	return true;
}

//-----------------------------------------------------------------------------
// Shutdown the v44+ model system
//-----------------------------------------------------------------------------
void CModelLoader_v44::Shutdown()
{
	UnloadAllModels_v44();
	Con_DPrintf("v44+ model loader system shut down.\n");
}

//-----------------------------------------------------------------------------
// Load a v44+ model by name
//-----------------------------------------------------------------------------
model_v44_t *CModelLoader_v44::LoadModel_v44(const char *name)
{
	if (!name || !name[0])
		return NULL;

	// Check if already loaded
	model_v44_t *pExisting = FindModel_v44(name);
	if (pExisting)
	{
		return pExisting;
	}

	// Create new v44+ model
	model_v44_t *pModel = new model_v44_t;
	memset(pModel, 0, sizeof(model_v44_t));

	Q_strncpy(pModel->name, name, sizeof(pModel->name));
	pModel->type = mod_studio;
	pModel->needload = FMODELLOADER_NOTLOADEDORREFERENCED;

	// Load MDL file
	void *buffer = COM_LoadStackFile(name, NULL, 0);
	if (!buffer)
	{
		Warning("v44+ model loader: Failed to load MDL file: %s\n", name);
		delete pModel;
		return NULL;
	}

	// Load studio model data
	if (!LoadStudioModel_v44(pModel, buffer))
	{
		Warning("v44+ model loader: Failed to load studio model: %s\n", name);
		delete pModel;
		return NULL;
	}

	// Load external vertex data (VVD)
	if (!LoadVertexData_v44(pModel))
	{
		Warning("v44+ model loader: Failed to load vertex data: %s\n", name);
		UnloadModel_v44(pModel);
		return NULL;
	}

	// Load mesh data (VTX)
	if (!LoadMeshData_v44(pModel))
	{
		Warning("v44+ model loader: Failed to load mesh data: %s\n", name);
		UnloadModel_v44(pModel);
		return NULL;
	}

	// Load animation blocks (.ani file) if model has external animations
	// This is optional - not all v44+ models have external animation blocks
	LoadAnimBlocksFile_v44(pModel);

	// Add to model dictionary
	m_Models_v44.Insert(name, pModel);

	Con_DPrintf("v44+ model loaded successfully: %s (v%d)\n", name, pModel->pStudioHdr->version);
	return pModel;
}

//-----------------------------------------------------------------------------
// Unload a v44+ model
//-----------------------------------------------------------------------------
void CModelLoader_v44::UnloadModel_v44(model_v44_t *pModel)
{
	if (!pModel)
		return;

	// Find and remove from dictionary
	for (int i = m_Models_v44.First(); i != m_Models_v44.InvalidIndex(); i = m_Models_v44.Next(i))
	{
		if (m_Models_v44[i] == pModel)
		{
			m_Models_v44.RemoveAt(i);
			break;
		}
	}

	FreeModel_v44(pModel);
	delete pModel;
}

//-----------------------------------------------------------------------------
// Unload all v44+ models
//-----------------------------------------------------------------------------
void CModelLoader_v44::UnloadAllModels_v44()
{
	for (int i = m_Models_v44.First(); i != m_Models_v44.InvalidIndex(); i = m_Models_v44.Next(i))
	{
		FreeModel_v44(m_Models_v44[i]);
		delete m_Models_v44[i];
	}
	m_Models_v44.RemoveAll();
}

//-----------------------------------------------------------------------------
// Find a loaded v44+ model
//-----------------------------------------------------------------------------
model_v44_t *CModelLoader_v44::FindModel_v44(const char *name)
{
	int index = m_Models_v44.Find(name);
	if (index != m_Models_v44.InvalidIndex())
	{
		return m_Models_v44[index];
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Load studio model data from MDL buffer
//-----------------------------------------------------------------------------
bool CModelLoader_v44::LoadStudioModel_v44(model_v44_t *pModel, void *buffer)
{
	if (!pModel || !buffer)
		return false;

	studiohdr_v44_t *pHdr = (studiohdr_v44_t *)buffer;

	// Validate that this is a v44+ model
	if (pHdr->version < STUDIO_VERSION_V44)
	{
		Warning("v44+ model loader: Model %s is v%d, not v44+\n", pModel->name, pHdr->version);
		return false;
	}

	// Validate studio header
	if (pHdr->id != IDSTUDIOHEADER)
	{
		Warning("v44+ model loader: Invalid studio header ID for %s\n", pModel->name);
		return false;
	}

	// Allocate and copy studio header data
	pModel->studiohdrsize = pHdr->length;
	pModel->pStudioHdr = (studiohdr_v44_t *)malloc(pModel->studiohdrsize);
	if (!pModel->pStudioHdr)
	{
		Warning("v44+ model loader: Failed to allocate studio header memory for %s\n", pModel->name);
		return false;
	}

	memcpy(pModel->pStudioHdr, buffer, pModel->studiohdrsize);

	// Set up bounding box
	VectorCopy(pModel->pStudioHdr->hull_min, pModel->mins);
	VectorCopy(pModel->pStudioHdr->hull_max, pModel->maxs);

	pModel->radius = 0;
	for (int i = 0; i < 3; i++)
	{
		if (fabs(pModel->mins[i]) > pModel->radius)
			pModel->radius = fabs(pModel->mins[i]);
		if (fabs(pModel->maxs[i]) > pModel->radius)
			pModel->radius = fabs(pModel->maxs[i]);
	}

	Con_DPrintf("v44+ studio model loaded: %s (v%d, size=%d)\n",
		pModel->name, pModel->pStudioHdr->version, pModel->studiohdrsize);

	return true;
}

//-----------------------------------------------------------------------------
// Load external VVD vertex data
//-----------------------------------------------------------------------------
bool CModelLoader_v44::LoadVertexData_v44(model_v44_t *pModel)
{
	return LoadVvdFile_v44(pModel);
}

//-----------------------------------------------------------------------------
// Load VTX mesh data
//-----------------------------------------------------------------------------
bool CModelLoader_v44::LoadMeshData_v44(model_v44_t *pModel)
{
	return LoadVtxFile_v44(pModel);
}

//-----------------------------------------------------------------------------
// Load VVD file for vertex data
//-----------------------------------------------------------------------------
bool CModelLoader_v44::LoadVvdFile_v44(model_v44_t *pModel)
{
	if (!pModel || !pModel->pStudioHdr)
		return false;

	// Generate VVD filename
	char vvdFileName[256];
	Q_strncpy(vvdFileName, pModel->name, sizeof(vvdFileName));

	// Replace extension with .vvd
	char *pExt = Q_strrchr(vvdFileName, '.');
	if (pExt)
		*pExt = '\0';
	Q_strncat(vvdFileName, ".vvd", sizeof(vvdFileName));

	// Load VVD file
	FileHandle_t fileHandle = g_pFileSystem->Open(vvdFileName, "rb");
	if (!fileHandle)
	{
		Warning("v44+ model loader: VVD file not found: %s\n", vvdFileName);
		return false;
	}

	int fileSize = g_pFileSystem->Size(fileHandle);
	if (fileSize < sizeof(vertexFileHeader_t))
	{
		Warning("v44+ model loader: VVD file too small: %s\n", vvdFileName);
		g_pFileSystem->Close(fileHandle);
		return false;
	}

	// Read VVD data
	void *vvdBuffer = malloc(fileSize);
	if (!vvdBuffer)
	{
		Warning("v44+ model loader: Failed to allocate VVD buffer for %s\n", vvdFileName);
		g_pFileSystem->Close(fileHandle);
		return false;
	}

	g_pFileSystem->Read(vvdBuffer, fileSize, fileHandle);
	g_pFileSystem->Close(fileHandle);

	// Validate VVD header
	vertexFileHeader_t *pVvdHdr = (vertexFileHeader_t *)vvdBuffer;
	if (pVvdHdr->id != MODEL_VERTEX_FILE_ID)
	{
		Warning("v44+ model loader: Invalid VVD header ID for %s\n", vvdFileName);
		free(vvdBuffer);
		return false;
	}

	// Store the VVD buffer - this is now owned by the model
	pModel->pVvdData = vvdBuffer;
	pModel->vertexDataSize = fileSize;

	// Set up vertex data pointers (these point into pVvdData)
	SetupVertexData_v44(pModel, vvdBuffer);

	pModel->verticesLoaded = true;

	Con_DPrintf("v44+ VVD loaded: %s (size=%d, vertices=%d)\n",
		vvdFileName, fileSize, pVvdHdr->numLODVertexes[0]);

	return true;
}

//-----------------------------------------------------------------------------
// Load VTX file for mesh data
//-----------------------------------------------------------------------------
bool CModelLoader_v44::LoadVtxFile_v44(model_v44_t *pModel)
{
	if (!pModel || !pModel->pStudioHdr)
		return false;

	// Generate VTX filename - try multiple extensions
	char vtxFileName[256];
	Q_strncpy(vtxFileName, pModel->name, sizeof(vtxFileName));

	// Replace extension
	char *pExt = Q_strrchr(vtxFileName, '.');
	if (pExt)
		*pExt = '\0';

	// The static vertex format stores hardware palette indices in four bits.
	// Prefer the v44+ DX80 data, which is stripified to 16 palette slots. DX90
	// data may use up to 53 slots and cannot be represented by CMeshBuilder.
	const char *vtxExtensions[] = { ".dx80.vtx", ".sw.vtx" };
	FileHandle_t fileHandle = FILESYSTEM_INVALID_HANDLE;

	for (int i = 0; i < ARRAYSIZE(vtxExtensions); i++)
	{
		char tryName[256];
		Q_strncpy(tryName, vtxFileName, sizeof(tryName));
		Q_strncat(tryName, vtxExtensions[i], sizeof(tryName));

		fileHandle = g_pFileSystem->Open(tryName, "rb");
		if (fileHandle)
		{
			Q_strncpy(vtxFileName, tryName, sizeof(vtxFileName));
			break;
		}
	}

	if (!fileHandle)
	{
		Warning("v44+ model loader: VTX file not found for: %s\n", pModel->name);
		return false;
	}

	int fileSize = g_pFileSystem->Size(fileHandle);
	if (fileSize < sizeof(OptimizedModel::FileHeader_t))
	{
		Warning("v44+ model loader: VTX file too small: %s\n", vtxFileName);
		g_pFileSystem->Close(fileHandle);
		return false;
	}

	// Allocate and read VTX data
	void *vtxBuffer = malloc(fileSize);
	if (!vtxBuffer)
	{
		Warning("v44+ model loader: Failed to allocate VTX buffer for %s\n", vtxFileName);
		g_pFileSystem->Close(fileHandle);
		return false;
	}

	g_pFileSystem->Read(vtxBuffer, fileSize, fileHandle);
	g_pFileSystem->Close(fileHandle);

	// Validate VTX header
	OptimizedModel::FileHeader_t *pVtxHdr = (OptimizedModel::FileHeader_t *)vtxBuffer;

	if (!OptimizedModel::IsValidVTXVersion(pVtxHdr->version))
	{
		Warning("v44+ model loader: Invalid VTX version %d for %s\n", pVtxHdr->version, vtxFileName);
		free(vtxBuffer);
		return false;
	}

	// Validate checksum matches MDL
	if (pVtxHdr->checkSum != pModel->pStudioHdr->checksum)
	{
		Warning("v44+ model loader: VTX checksum mismatch for %s (VTX: %ld, MDL: %d)\n",
			vtxFileName, pVtxHdr->checkSum, pModel->pStudioHdr->checksum);
		free(vtxBuffer);
		return false;
	}

	// Store VTX data
	pModel->pVtxData = vtxBuffer;
	pModel->vtxDataSize = fileSize;
	pModel->meshesLoaded = true;

	Con_DPrintf("v44+ VTX loaded: %s (size=%d, version=%d, LODs=%d, bodyparts=%d)\n",
		vtxFileName, fileSize, pVtxHdr->version, pVtxHdr->numLODs, pVtxHdr->numBodyParts);

	return true;
}

//-----------------------------------------------------------------------------
// Set up vertex data pointers from VVD
//-----------------------------------------------------------------------------
void CModelLoader_v44::SetupVertexData_v44(model_v44_t *pModel, void *pVvdData)
{
	if (!pModel || !pVvdData)
		return;

	vertexFileHeader_t *pVvdHdr = (vertexFileHeader_t *)pVvdData;

	// Set up vertex data pointers
	pModel->pVertexData = (byte *)pVvdData + pVvdHdr->vertexDataStart;
	pModel->pTangentData = (byte *)pVvdData + pVvdHdr->tangentDataStart;

	// Set up vertex data in all models within the studio header
	if (pModel->pStudioHdr)
	{
		for (int bodyPartId = 0; bodyPartId < pModel->pStudioHdr->numbodyparts; bodyPartId++)
		{
			mstudiobodyparts_v44_t *pBodyPart = pModel->pStudioHdr->pBodypart(bodyPartId);
			if (!pBodyPart)
				continue;

			for (int modelId = 0; modelId < pBodyPart->nummodels; modelId++)
			{
				mstudiomodel_v44_t *pStudioModel = pBodyPart->pModel(modelId);
				if (!pStudioModel)
					continue;

				// Set up the vertex data pointers in the model's embedded structure
				pStudioModel->vertexdata.pVertexData = pModel->pVertexData;
				pStudioModel->vertexdata.pTangentData = pModel->pTangentData;
			}
		}
	}

	Con_DPrintf("v44+ vertex data set up for %s (vertices=%p, tangents=%p)\n",
		pModel->name, pModel->pVertexData, pModel->pTangentData);
}

//-----------------------------------------------------------------------------
// Load animation blocks (.ani file) for v44+ models
// v44+ models can have external animation blocks stored in .ani files
// Block 0 is always inline (unused), blocks 1+ are in the .ani file
//-----------------------------------------------------------------------------
bool CModelLoader_v44::LoadAnimBlocksFile_v44(model_v44_t *pModel)
{
	if (!pModel || !pModel->pStudioHdr)
		return false;

	// Check if model has animation blocks
	// numanimblocks <= 1 means no external blocks (block 0 is never used)
	if (pModel->pStudioHdr->numanimblocks <= 1)
	{
		pModel->animBlocksLoaded = true;  // No blocks to load, but mark as "done"
		return true;
	}

	// Get the animation block filename from the model header
	const char *pszAnimBlockName = pModel->pStudioHdr->pszAnimBlockName();
	if (!pszAnimBlockName || !pszAnimBlockName[0])
	{
		// No animation block name specified - blocks might be inline
		Con_DPrintf("v44+ model %s: No animation block name, %d blocks may be inline\n",
			pModel->name, pModel->pStudioHdr->numanimblocks);
		pModel->animBlocksLoaded = true;
		return true;
	}

	// Build the full path to the .ani file
	// The animblockname is typically just the filename, we need the path
	char aniFileName[256];
	Q_strncpy(aniFileName, pModel->name, sizeof(aniFileName));

	// Get the directory from the model path
	char *pLastSlash = Q_strrchr(aniFileName, '/');
	if (!pLastSlash)
		pLastSlash = Q_strrchr(aniFileName, '\\');

	if (pLastSlash)
	{
		// Keep the directory, append the animation block name
		pLastSlash[1] = '\0';
		Q_strncat(aniFileName, pszAnimBlockName, sizeof(aniFileName));
	}
	else
	{
		// No directory in model path, use animblockname directly
		Q_strncpy(aniFileName, pszAnimBlockName, sizeof(aniFileName));
	}

	// Open the .ani file
	FileHandle_t fileHandle = g_pFileSystem->Open(aniFileName, "rb");
	if (!fileHandle)
	{
		// Not finding the .ani file is not fatal - some models may have inlined blocks
		Con_DPrintf("v44+ model %s: Animation block file not found: %s\n",
			pModel->name, aniFileName);
		pModel->animBlocksLoaded = true;
		return true;
	}

	int fileSize = g_pFileSystem->Size(fileHandle);
	if (fileSize <= 0)
	{
		Warning("v44+ model %s: Animation block file is empty: %s\n",
			pModel->name, aniFileName);
		g_pFileSystem->Close(fileHandle);
		return false;
	}

	// Allocate buffer and read the file
	void *aniBuffer = malloc(fileSize);
	if (!aniBuffer)
	{
		Warning("v44+ model %s: Failed to allocate animation block buffer (%d bytes)\n",
			pModel->name, fileSize);
		g_pFileSystem->Close(fileHandle);
		return false;
	}

	g_pFileSystem->Read(aniBuffer, fileSize, fileHandle);
	g_pFileSystem->Close(fileHandle);

	// Store the animation block data
	pModel->pAnimBlockData = aniBuffer;
	pModel->animBlockDataSize = fileSize;
	pModel->animBlocksLoaded = true;

	// Set up the animblockModel pointer in the studio header
	// This is how the engine accesses animation block data
	pModel->pStudioHdr->animblockModel = aniBuffer;

	Con_DPrintf("v44+ animation blocks loaded: %s (file=%s, size=%d, blocks=%d)\n",
		pModel->name, aniFileName, fileSize, pModel->pStudioHdr->numanimblocks);

	return true;
}

//-----------------------------------------------------------------------------
// Free all model data
//-----------------------------------------------------------------------------
void CModelLoader_v44::FreeModel_v44(model_v44_t *pModel)
{
	if (!pModel)
		return;

	// Free studio header
	if (pModel->pStudioHdr)
	{
		free(pModel->pStudioHdr);
		pModel->pStudioHdr = NULL;
	}

	// Free VVD data (pVertexData and pTangentData point into this buffer)
	if (pModel->pVvdData)
	{
		free(pModel->pVvdData);
		pModel->pVvdData = NULL;
	}
	pModel->pVertexData = NULL;
	pModel->pTangentData = NULL;
	pModel->vertexDataSize = 0;

	// Free VTX data
	if (pModel->pVtxData)
	{
		free(pModel->pVtxData);
		pModel->pVtxData = NULL;
	}
	pModel->vtxDataSize = 0;

	// Free animation block data
	if (pModel->pAnimBlockData)
	{
		// Clear the animblockModel pointer in the header first
		if (pModel->pStudioHdr)
		{
			pModel->pStudioHdr->animblockModel = NULL;
		}
		free(pModel->pAnimBlockData);
		pModel->pAnimBlockData = NULL;
	}
	pModel->animBlockDataSize = 0;
	pModel->animBlocksLoaded = false;

	// Free mesh data
	if (pModel->pMeshData)
	{
		for (int i = 0; i < pModel->numMeshes; i++)
		{
			modelloadermeshdata_v44_t *pMesh = &pModel->pMeshData[i];
			if (pMesh->m_pMeshGroup)
			{
				delete[] pMesh->m_pMeshGroup;
				pMesh->m_pMeshGroup = NULL;
			}
		}
		delete[] pModel->pMeshData;
		pModel->pMeshData = NULL;
	}
	pModel->numMeshes = 0;

	pModel->verticesLoaded = false;
	pModel->meshesLoaded = false;
}

//-----------------------------------------------------------------------------
// Global functions for v44+ model management
//-----------------------------------------------------------------------------
model_v44_t *Mod_ForName_v44(const char *name)
{
	if (!g_pModelLoader_v44)
		return NULL;

	return g_pModelLoader_v44->LoadModel_v44(name);
}

void Mod_UnloadModel_v44(model_v44_t *pModel)
{
	if (!g_pModelLoader_v44)
		return;

	g_pModelLoader_v44->UnloadModel_v44(pModel);
}

//-----------------------------------------------------------------------------
// Initialize v44+ model system
//-----------------------------------------------------------------------------
static bool s_bInitialized_v44 = false;

void InitModelLoader_v44()
{
	if (s_bInitialized_v44)
		return;

	g_pModelLoader_v44 = new CModelLoader_v44;
	g_pModelLoader_v44->Init();
	s_bInitialized_v44 = true;

	Con_DPrintf("v44+ model loading system initialized.\n");
}

void ShutdownModelLoader_v44()
{
	if (!s_bInitialized_v44)
		return;

	if (g_pModelLoader_v44)
	{
		delete g_pModelLoader_v44;
		g_pModelLoader_v44 = NULL;
	}

	s_bInitialized_v44 = false;
	Con_DPrintf("v44+ model loading system shut down.\n");
}

//-----------------------------------------------------------------------------
// Get animation block data for v44+ models
// Returns pointer to the start of animation data for the specified block
// Block 0 is never used (inline data), blocks 1+ are in the .ani file
//-----------------------------------------------------------------------------
void* GetAnimBlock_v44(const studiohdr_v44_t* pStudioHdr, int nBlock)
{
	if (!pStudioHdr)
		return NULL;

	// Block 0 is never used for external data
	if (nBlock <= 0)
		return NULL;

	// Check if we have animation blocks
	if (nBlock >= pStudioHdr->numanimblocks)
		return NULL;

	// Get the animation block data pointer (loaded .ani file)
	byte* pAnimBlockData = (byte*)pStudioHdr->animblockModel;
	if (!pAnimBlockData)
	{
		// No animation block data loaded - fall back to inline data
		return NULL;
	}

	// Get the block info
	mstudioanimblock_v44_t* pBlock = pStudioHdr->pAnimBlock(nBlock);
	if (!pBlock)
		return NULL;

	// Return pointer to the block data within the .ani file
	// datastart is the offset from the beginning of the .ani file
	return pAnimBlockData + pBlock->datastart;
}
