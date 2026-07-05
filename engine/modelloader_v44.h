//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Completely independent v44+ model loading and management system
//          Separate from v37 models to avoid casting and compatibility issues
//
//=============================================================================

#ifndef MODELLOADER_V44_H
#define MODELLOADER_V44_H

// NO studio.h - v44+ code is completely isolated from v37
#include "studiohdr_v44.h"  // All v44+ types defined here
#include "utldict.h"
#include "utlmemory.h"

// Define MAX_QPATH if not already defined (typically 128)
#ifndef MAX_QPATH
#define MAX_QPATH 128
#endif

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
struct model_v44_t;
struct studiomeshdata_v44_t;
struct modelloadermeshdata_v44_t;

//-----------------------------------------------------------------------------
// v44+ Model structure - completely separate from model_t
//-----------------------------------------------------------------------------
struct model_v44_t
{
	char			name[MAX_QPATH];
	int				needload;		// bitmask of FMODELLOADER flags
	int				type;			// model type (should be mod_studio for v44+)

	// v44+ specific data
	studiohdr_v44_t	*pStudioHdr;	// v44+ studio header
	int				studiohdrsize;	// size of the v44+ header

	// Vertex data (external VVD)
	void			*pVvdData;		// Raw VVD file buffer (owned by this struct)
	void			*pVertexData;	// mstudiovertex_t array from .vvd file (points into pVvdData)
	void			*pTangentData;	// Vector4D array from .vvd file (points into pVvdData)
	int				vertexDataSize;

	// VTX mesh data (for hardware rendering)
	void			*pVtxData;		// Raw VTX file buffer (owned by this struct)
	int				vtxDataSize;

	// Animation block data (external .ani file)
	void			*pAnimBlockData;	// Raw .ani file buffer (owned by this struct)
	int				animBlockDataSize;
	bool			animBlocksLoaded;

	// Mesh data
	modelloadermeshdata_v44_t *pMeshData;
	int				numMeshes;

	// Bounding box
	Vector			mins, maxs;
	float			radius;

	// Load flags
	bool			verticesLoaded;
	bool			meshesLoaded;
};

//-----------------------------------------------------------------------------
// v44+ Local mesh data structure - extends the base studiomeshdata_v44_t
// with loader-specific fields
//-----------------------------------------------------------------------------
struct modelloadermeshdata_v44_t
{
	// Base mesh data (inherited concept)
	int							m_NumGroup;
	studiomeshgroup_v44_t		*m_pMeshGroup;

	// v44+ specific mesh info for model loading
	int							meshid;
	mstudiomesh_v44_t			*pMesh;
};

//-----------------------------------------------------------------------------
// v44+ Model Loader - completely independent system
//-----------------------------------------------------------------------------
class CModelLoader_v44
{
public:
	CModelLoader_v44();
	~CModelLoader_v44();

	// Initialize/shutdown the v44+ model system
	bool			Init();
	void			Shutdown();

	// Model loading
	model_v44_t		*LoadModel_v44(const char *name);
	void			UnloadModel_v44(model_v44_t *pModel);
	void			UnloadAllModels_v44();

	// Model lookup
	model_v44_t		*FindModel_v44(const char *name);
	int				GetCount_v44() const { return m_Models_v44.Count(); }

	// Model data loading
	bool			LoadStudioModel_v44(model_v44_t *pModel, void *buffer);
	bool			LoadVertexData_v44(model_v44_t *pModel);
	bool			LoadMeshData_v44(model_v44_t *pModel);

private:
	// Model storage - completely separate from v37 models
	CUtlDict<model_v44_t*, unsigned short> m_Models_v44;

	// Helper functions
	bool			LoadVvdFile_v44(model_v44_t *pModel);
	bool			LoadVtxFile_v44(model_v44_t *pModel);
	bool			LoadAnimBlocksFile_v44(model_v44_t *pModel);	// Load external .ani file
	bool			ValidateModel_v44(model_v44_t *pModel);
	void			SetupVertexData_v44(model_v44_t *pModel, void *pVvdData);
	void			FreeModel_v44(model_v44_t *pModel);
};

//-----------------------------------------------------------------------------
// Global v44+ model loader instance
//-----------------------------------------------------------------------------
extern CModelLoader_v44 *g_pModelLoader_v44;

//-----------------------------------------------------------------------------
// v44+ Model management functions
//-----------------------------------------------------------------------------
bool			Mod_LoadStudioModel_v44(model_v44_t *pModel, void *buffer);
bool			Mod_LoadVertexData_v44(model_v44_t *pModel);
model_v44_t		*Mod_ForName_v44(const char *name);
void			Mod_UnloadModel_v44(model_v44_t *pModel);

// System initialization
void			InitModelLoader_v44();
void			ShutdownModelLoader_v44();

//-----------------------------------------------------------------------------
// v44+ Animation block access function
// Returns pointer to animation data for a given animation block index
// Block 0 is always inline (returns NULL), blocks 1+ are in .ani file
//-----------------------------------------------------------------------------
void*			GetAnimBlock_v44(const studiohdr_v44_t* pStudioHdr, int nBlock);

#endif // MODELLOADER_V44_H