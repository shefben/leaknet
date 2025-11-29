//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef OPTIMIZE_H
#define OPTIMIZE_H

#ifdef _WIN32
#pragma once
#endif

#include "studio.h"

// NOTE: MAX_NUM_BONES_PER_VERT is now defined in studio.h as 3 (v48 format)
// You can change this without affecting the vtx file format.
#define MAX_NUM_BONES_PER_TRI ( MAX_NUM_BONES_PER_VERT * 3 )
#define MAX_NUM_BONES_PER_STRIP 512

//-----------------------------------------------------------------------------
// VTX File Version Support
//-----------------------------------------------------------------------------
// v6: Original HL2 2003/2004 format
// v7: Source 2007+ format with topology data for hardware morphing
//-----------------------------------------------------------------------------
#define OPTIMIZED_MODEL_FILE_VERSION_V6  6
#define OPTIMIZED_MODEL_FILE_VERSION_V7  7
#define OPTIMIZED_MODEL_FILE_VERSION_MIN OPTIMIZED_MODEL_FILE_VERSION_V6
#define OPTIMIZED_MODEL_FILE_VERSION_MAX OPTIMIZED_MODEL_FILE_VERSION_V7

// Current target version (v7 for new files, but we read both)
#define OPTIMIZED_MODEL_FILE_VERSION     OPTIMIZED_MODEL_FILE_VERSION_V7

extern bool g_bDumpGLViewFiles;

struct s_bodypart_t;

namespace OptimizedModel
{

#pragma pack(1)

struct BoneStateChangeHeader_t
{
	int hardwareID;
	int newBoneID;
};

//-----------------------------------------------------------------------------
// v37: Vertex structure for HL2 Beta 2003 VTX files
// MAX_NUM_BONES_PER_VERT was 4 in v37, and boneID was shorts
//-----------------------------------------------------------------------------
struct Vertex_v37_t
{
	// These index into the mesh's vert[origMeshVertID]'s bones
	unsigned char boneWeightIndex[4];	// 4 bytes (v37 had 4 bone weights)

	// For sw skinned verts, these are indices into the global list of bones
	// For hw skinned verts, these are hardware bone indices
	short boneID[4];					// 8 bytes (v37 used shorts, not chars)

	short origMeshVertID;				// 2 bytes (v37 used short, not unsigned short)
	unsigned char numBones;				// 1 byte
};	// = 15 bytes total

//-----------------------------------------------------------------------------
// v48: Vertex structure for Source 2007+ VTX files
//-----------------------------------------------------------------------------
struct Vertex_t
{
	// These index into the mesh's vert[origMeshVertID]'s bones
	unsigned char boneWeightIndex[MAX_NUM_BONES_PER_VERT];
	unsigned char numBones;

	unsigned short origMeshVertID;

	// For sw skinned verts, these are indices into the global list of bones
	// For hw skinned verts, these are hardware bone indices
	char boneID[MAX_NUM_BONES_PER_VERT];
};
	
enum StripHeaderFlags_t {
	STRIP_IS_TRILIST	= 0x01,
	STRIP_IS_TRISTRIP	= 0x02
};

// a strip is a piece of a stripgroup that is divided by bones 
// (and potentially tristrips if we remove some degenerates.)
struct StripHeader_t
{
	// indexOffset offsets into the mesh's index array.
	int numIndices;
	int indexOffset;

	// vertexOffset offsets into the mesh's vert array.
	int numVerts;
	int vertOffset;

	// use this to enable/disable skinning.  
	// May decide (in optimize.cpp) to put all with 1 bone in a different strip 
	// than those that need skinning.
	short numBones;  
	
	unsigned char flags;
	
	int numBoneStateChanges;
	int boneStateChangeOffset;
	inline BoneStateChangeHeader_t *pBoneStateChange( int i ) const 
	{ 
		return (BoneStateChangeHeader_t *)(((byte *)this) + boneStateChangeOffset) + i; 
	};
};

enum StripGroupFlags_t
{
	STRIPGROUP_IS_FLEXED		 = 0x01,
	STRIPGROUP_IS_HWSKINNED		 = 0x02,
	STRIPGROUP_IS_DELTA_FLEXED	 = 0x04,
	STRIPGROUP_SUPPRESS_HW_MORPH = 0x08,	// NOTE: This is a temporary flag used at run time.
};

// a locking group
// a single vertex buffer
// a single index buffer
struct StripGroupHeader_t
{
	// These are the arrays of all verts and indices for this mesh.  strips index into this.
	int numVerts;
	int vertOffset;

	// v48 vertex accessor (9-byte Vertex_t)
	inline Vertex_t *pVertex( int i ) const
	{
		return (Vertex_t *)(((byte *)this) + vertOffset) + i;
	};

	// v37 vertex accessor (15-byte Vertex_v37_t)
	// Use this when loading v37 VTX files which have different vertex structure
	inline Vertex_v37_t *pVertex_V37( int i ) const
	{
		return (Vertex_v37_t *)(((byte *)this) + vertOffset) + i;
	};

	int numIndices;
	int indexOffset;
	inline unsigned short *pIndex( int i ) const 
	{ 
		return (unsigned short *)(((byte *)this) + indexOffset) + i; 
	};

	int numStrips;
	int stripOffset;
	inline StripHeader_t *pStrip( int i ) const 
	{ 
		return (StripHeader_t *)(((byte *)this) + stripOffset) + i; 
	};

	unsigned char flags;
};

enum MeshFlags_t { 
	// these are both material properties, and a mesh has a single material.
	MESH_IS_TEETH	= 0x01, 
	MESH_IS_EYES	= 0x02
};

// a collection of locking groups:
// up to 4:
// non-flexed, hardware skinned
// flexed, hardware skinned
// non-flexed, software skinned
// flexed, software skinned
//
// A mesh has a material associated with it.
struct MeshHeader_t
{
	int numStripGroups;
	int stripGroupHeaderOffset;
	inline StripGroupHeader_t *pStripGroup( int i ) const 
	{ 
		StripGroupHeader_t *pDebug = (StripGroupHeader_t *)(((byte *)this) + stripGroupHeaderOffset) + i; 
		return pDebug;
	};
	unsigned char flags;
};

struct ModelLODHeader_t
{
	int numMeshes;
	int meshOffset;
	float switchPoint;
	inline MeshHeader_t *pMesh( int i ) const 
	{ 
		MeshHeader_t *pDebug = (MeshHeader_t *)(((byte *)this) + meshOffset) + i; 
		return pDebug;
	};
};

// This maps one to one with models in the mdl file.
// There are a bunch of model LODs stored inside potentially due to the qc $lod command
struct ModelHeader_t
{
	int numLODs; // garymcthack - this is also specified in FileHeader_t
	int lodOffset;
	inline ModelLODHeader_t *pLOD( int i ) const 
	{ 
		ModelLODHeader_t *pDebug = ( ModelLODHeader_t *)(((byte *)this) + lodOffset) + i; 
		return pDebug;
	};
};

struct BodyPartHeader_t
{
	int numModels;
	int modelOffset;
	inline ModelHeader_t *pModel( int i ) const 
	{ 
		ModelHeader_t *pDebug = (ModelHeader_t *)(((byte *)this) + modelOffset) + i;
		return pDebug;
	};
};

struct MaterialReplacementHeader_t
{
	short materialID;
	int replacementMaterialNameOffset;
	inline const char *pMaterialReplacementName( void )
	{
		const char *pDebug = (const char *)(((byte *)this) + replacementMaterialNameOffset); 
		return pDebug;
	}
};

struct MaterialReplacementListHeader_t
{
	int numReplacements;
	int replacementOffset;
	inline MaterialReplacementHeader_t *pMaterialReplacement( int i ) const
	{
		MaterialReplacementHeader_t *pDebug = ( MaterialReplacementHeader_t *)(((byte *)this) + replacementOffset) + i; 
		return pDebug;
	}
};

struct FileHeader_t
{
	// file version as defined by OPTIMIZED_MODEL_FILE_VERSION
	int version;

	// hardware params that affect how the model is to be optimized.
	int vertCacheSize;
	unsigned short maxBonesPerStrip;
	unsigned short maxBonesPerTri;
	int maxBonesPerVert;

	// must match checkSum in the .mdl
	long checkSum;
	
	int numLODs; // garymcthack - this is also specified in ModelHeader_t and should match

	// one of these for each LOD
	int materialReplacementListOffset;
	MaterialReplacementListHeader_t *pMaterialReplacementList( int lodID ) const
	{ 
		MaterialReplacementListHeader_t *pDebug = 
			(MaterialReplacementListHeader_t *)(((byte *)this) + materialReplacementListOffset) + lodID;
		return pDebug;
	}

	int numBodyParts;
	int bodyPartOffset;
	inline BodyPartHeader_t *pBodyPart( int i ) const
	{
		BodyPartHeader_t *pDebug = (BodyPartHeader_t *)(((byte *)this) + bodyPartOffset) + i;
		return pDebug;
	};

	//-----------------------------------------------------------------------------
	// v7-specific helper functions
	//-----------------------------------------------------------------------------
	inline bool IsV7() const { return version >= OPTIMIZED_MODEL_FILE_VERSION_V7; }
	inline bool IsV6() const { return version == OPTIMIZED_MODEL_FILE_VERSION_V6; }
};

//-----------------------------------------------------------------------------
// v7: Topology data structures for hardware morphing
// These structures are appended after the standard VTX data in v7 files
//-----------------------------------------------------------------------------

// v7: Topology data header (appended after standard VTX data)
struct TopologyDataHeader_t
{
	int numStrips;
	int stripOffset;

	inline StripHeader_t *pStrip( int i ) const
	{
		return (StripHeader_t *)(((byte *)this) + stripOffset) + i;
	}
};

// v7: Strip topology data
struct StripTopology_t
{
	int numIndices;
	int indexOffset;

	inline unsigned short *pIndex( int i ) const
	{
		return (unsigned short *)(((byte *)this) + indexOffset) + i;
	}
};

// v7: Extended vertex data with morph target indices
struct Vertex_V7_t
{
	// Same base fields as Vertex_t
	unsigned char boneWeightIndex[MAX_NUM_BONES_PER_VERT];
	unsigned char numBones;

	unsigned short origMeshVertID;

	// For sw skinned verts, these are indices into the global list of bones
	// For hw skinned verts, these are hardware bone indices
	char boneID[MAX_NUM_BONES_PER_VERT];

	// v7: Additional morph target index
	short morphTargetIndex;
};

#pragma pack()

//-----------------------------------------------------------------------------
// Version checking helpers
//-----------------------------------------------------------------------------
inline bool IsValidVTXVersion( int version )
{
	return version >= OPTIMIZED_MODEL_FILE_VERSION_MIN &&
	       version <= OPTIMIZED_MODEL_FILE_VERSION_MAX;
}

void WriteOptimizedFiles( studiohdr_t *phdr, s_bodypart_t *pSrcBodyParts );

}; // namespace OptimizedModel

#endif // OPTIMIZE_H
