//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Version-aware vertex access helpers for v37 (HL2 Beta 2003) models
//          and v44+ models with external VVD vertex data.
//
// v37 models have embedded vertex data with different structure sizes:
// - mstudiovertex_v37_t is 64 bytes (vs 48 bytes for mstudiovertex_t)
// - mstudioboneweight_v37_t is 32 bytes (vs 16 bytes for mstudioboneweight_t)
//
// v44+ models use external VVD files with vertex data accessed via the
// completely isolated v44+ API in studiohdr_v44.h. This file provides
// DISPATCH functions that route to the correct version-specific code.
//
// ARCHITECTURE:
// - v37 code: Inline implementation in this file (uses v37 types directly)
// - v44+ code: Delegates to isolated functions in studiohdr_v44.h (uses v44+ types directly)
// - NO casting between v37 and v44+ types occurs in either path
//
//=============================================================================

#ifndef STUDIO_V37_COMPAT_H
#define STUDIO_V37_COMPAT_H

#include "studio.h"
#include "studiohdr_v44.h"
#include "studio_helpers.h"  // Shared helper functions for studiohdr_t access

//=============================================================================
// V44+ VERTEX ACCESS HELPERS
// These functions provide isolated access to v44+ vertex data
// They accept v44+ types directly and output via reference parameters
//=============================================================================

inline void StudioMesh_SetDefaultVertexData(
	Vector& outPosition,
	Vector& outNormal,
	Vector2D& outTexCoord,
	mstudioboneweight_t& outBoneWeight)
{
	outPosition.Init();
	outNormal.Init(0, 0, 1);
	outTexCoord.Init();
	outBoneWeight.numbones = 0;
	for (int i = 0; i < MAX_NUM_BONES_PER_VERT; ++i)
	{
		outBoneWeight.weight[i] = 0.0f;
		outBoneWeight.bone[i] = 0;
	}
}

inline bool StudioMesh_V44_GetVertexIndex(
	const mstudiomesh_v44_t* pMesh44,
	const mstudiomodel_v44_t* pModel44,
	int idx,
	int& outVertexIndex)
{
	outVertexIndex = 0;

	if (!pMesh44 || !pModel44)
		return false;

	if (idx < 0 || idx >= pMesh44->numvertices)
		return false;

	if (pMesh44->vertexoffset < 0 || pModel44->numvertices < 0)
		return false;

	__int64 vertexIndex = (__int64)pMesh44->vertexoffset + idx;
	if (vertexIndex < 0 || vertexIndex >= pModel44->numvertices)
		return false;

	outVertexIndex = (int)vertexIndex;
	return true;
}

// Get full vertex data for a v44+ mesh vertex
inline void StudioMesh_V44_GetVertexData(
	const mstudiomesh_v44_t* pMesh44,
	const mstudiomodel_v44_t* pModel44,
	const void* pVertexBase,
	int idx,
	Vector& outPosition,
	Vector& outNormal,
	Vector2D& outTexCoord,
	mstudioboneweight_t& outBoneWeight)
{
	if (!pMesh44 || !pModel44 || !pVertexBase)
	{
		StudioMesh_SetDefaultVertexData(outPosition, outNormal, outTexCoord, outBoneWeight);
		return;
	}

	// Calculate vertex offset in external VVD data
	int vertexIndex;
	if (!StudioMesh_V44_GetVertexIndex(pMesh44, pModel44, idx, vertexIndex))
	{
		StudioMesh_SetDefaultVertexData(outPosition, outNormal, outTexCoord, outBoneWeight);
		return;
	}

	const mstudiovertex_v44_t* pVert =
		(const mstudiovertex_v44_t*)((const byte*)pVertexBase +
			pModel44->vertexindex + vertexIndex * sizeof(mstudiovertex_v44_t));

	outPosition = pVert->m_vecPosition;
	outNormal = pVert->m_vecNormal;
	outTexCoord = pVert->m_vecTexCoord;

	// Copy bone weights
	outBoneWeight.numbones = pVert->m_BoneWeights.numbones;
	for (int i = 0; i < MAX_NUM_BONES_PER_VERT; i++)
	{
		outBoneWeight.weight[i] = pVert->m_BoneWeights.weight[i];
		outBoneWeight.bone[i] = pVert->m_BoneWeights.bone[i];
	}
}

// Get just bone weights for a v44+ mesh vertex
inline bool StudioMesh_V44_GetBoneWeights(
	const mstudiomesh_v44_t* pMesh44,
	const mstudiomodel_v44_t* pModel44,
	const void* pVertexBase,
	int idx,
	mstudioboneweight_t& outBoneWeight)
{
	if (!pMesh44 || !pModel44 || !pVertexBase)
	{
		outBoneWeight.numbones = 0;
		return false;
	}

	// Calculate vertex offset in external VVD data
	int vertexIndex;
	if (!StudioMesh_V44_GetVertexIndex(pMesh44, pModel44, idx, vertexIndex))
	{
		outBoneWeight.numbones = 0;
		for (int i = 0; i < MAX_NUM_BONES_PER_VERT; i++)
		{
			outBoneWeight.weight[i] = 0.0f;
			outBoneWeight.bone[i] = 0;
		}
		return false;
	}

	const mstudiovertex_v44_t* pVert =
		(const mstudiovertex_v44_t*)((const byte*)pVertexBase +
			pModel44->vertexindex + vertexIndex * sizeof(mstudiovertex_v44_t));

	// Copy bone weights
	outBoneWeight.numbones = pVert->m_BoneWeights.numbones;
	for (int i = 0; i < MAX_NUM_BONES_PER_VERT; i++)
	{
		outBoneWeight.weight[i] = pVert->m_BoneWeights.weight[i];
		outBoneWeight.bone[i] = pVert->m_BoneWeights.bone[i];
	}
	return true;
}

// Get just position for a v44+ mesh vertex
inline void StudioMesh_V44_GetPosition(
	const mstudiomesh_v44_t* pMesh44,
	const mstudiomodel_v44_t* pModel44,
	const void* pVertexBase,
	int idx,
	Vector& outPosition)
{
	if (!pMesh44 || !pModel44 || !pVertexBase)
	{
		outPosition.Init();
		return;
	}

	// Calculate vertex offset in external VVD data
	int vertexIndex;
	if (!StudioMesh_V44_GetVertexIndex(pMesh44, pModel44, idx, vertexIndex))
	{
		outPosition.Init();
		return;
	}

	const mstudiovertex_v44_t* pVert =
		(const mstudiovertex_v44_t*)((const byte*)pVertexBase +
			pModel44->vertexindex + vertexIndex * sizeof(mstudiovertex_v44_t));

	outPosition = pVert->m_vecPosition;
}

// Get just normal for a v44+ mesh vertex
inline void StudioMesh_V44_GetNormal(
	const mstudiomesh_v44_t* pMesh44,
	const mstudiomodel_v44_t* pModel44,
	const void* pVertexBase,
	int idx,
	Vector& outNormal)
{
	if (!pMesh44 || !pModel44 || !pVertexBase)
	{
		outNormal.Init(0, 0, 1);
		return;
	}

	// Calculate vertex offset in external VVD data
	int vertexIndex;
	if (!StudioMesh_V44_GetVertexIndex(pMesh44, pModel44, idx, vertexIndex))
	{
		outNormal.Init(0, 0, 1);
		return;
	}

	const mstudiovertex_v44_t* pVert =
		(const mstudiovertex_v44_t*)((const byte*)pVertexBase +
			pModel44->vertexindex + vertexIndex * sizeof(mstudiovertex_v44_t));

	outNormal = pVert->m_vecNormal;
}

// Get raw vertex pointer for a v44+ mesh vertex (returns v44+ type)
inline const mstudiovertex_v44_t* StudioMesh_V44_GetVertex(
	const mstudiomesh_v44_t* pMesh44,
	const mstudiomodel_v44_t* pModel44,
	const void* pVertexBase,
	int idx)
{
	if (!pMesh44 || !pModel44 || !pVertexBase)
		return NULL;

	// Calculate vertex offset in external VVD data
	int vertexIndex;
	if (!StudioMesh_V44_GetVertexIndex(pMesh44, pModel44, idx, vertexIndex))
		return NULL;

	return (const mstudiovertex_v44_t*)((const byte*)pVertexBase +
		pModel44->vertexindex + vertexIndex * sizeof(mstudiovertex_v44_t));
}

inline Vector4D* StudioMesh_V44_GetTangentS(
	const mstudiomesh_v44_t* pMesh44,
	const mstudiomodel_v44_t* pModel44,
	int idx)
{
	if (!pMesh44 || !pModel44 || !pModel44->vertexdata.pTangentData)
		return NULL;

	int tangentIndex;
	if (!StudioMesh_V44_GetVertexIndex(pMesh44, pModel44, idx, tangentIndex))
		return NULL;

	return (Vector4D*)((byte*)pModel44->vertexdata.pTangentData +
		pModel44->tangentsindex + tangentIndex * sizeof(Vector4D));
}

//=============================================================================
// INTERNAL HELPER: Get v44+ mesh/model pointers from v37 mesh
// This is the ONLY place where we need to navigate from a generic mesh to
// its v44+ equivalents. Once we have v44+ pointers, all access goes through
// the isolated v44+ API.
//=============================================================================
inline bool Studio_GetV44MeshAndModel(
	const studiohdr_t* pStudioHdr,
	const mstudiomesh_t* pMesh,
	const mstudiomesh_v44_t*& outMesh44,
	const mstudiomodel_v44_t*& outModel44,
	const void*& outVertexBase)
{
	outMesh44 = NULL;
	outModel44 = NULL;
	outVertexBase = NULL;

	if (!pStudioHdr || !pMesh || pStudioHdr->version < STUDIO_VERSION_44)
		return false;

	// For v44+ models, the mesh pointer from the bodypart IS already
	// a mstudiomesh_v44_t* (different binary layout than v37)
	// We use reinterpret_cast here because we've verified the version
	outMesh44 = reinterpret_cast<const mstudiomesh_v44_t*>(pMesh);
	outModel44 = outMesh44->pModel();
	if (!outModel44)
		return false;
	// pVertexBase is a runtime field in the version-specific header. Reading it
	// through studiohdr_t uses a legacy offset and returns unrelated MDL data.
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	outVertexBase = pHdr44 && pHdr44->pVertexBase ? pHdr44->pVertexBase : outModel44->vertexdata.pVertexData;

	return (outMesh44 != NULL && outModel44 != NULL && outVertexBase != NULL);
}

//-----------------------------------------------------------------------------
// Get vertex data with version awareness
// For v37 models, uses the 64-byte mstudiovertex_v37_t stride
// For v44+ models, delegates to isolated v44+ API (no casting from v37 types)
//-----------------------------------------------------------------------------
inline void Studio_GetVertexData_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	Vector& outPosition,
	Vector& outNormal,
	Vector2D& outTexCoord )
{
	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		// v37 models have embedded vertex data with 64-byte vertices
		mstudiomodel_t* pModel = pMesh->pModel();
		int vertexOffset = pMesh->vertexoffset + idx;
		byte* pVertexBase = ((byte*)pModel) + pModel->vertexindex;
		mstudiovertex_v37_t* pVert37 = (mstudiovertex_v37_t*)pVertexBase + vertexOffset;

		outPosition = pVert37->m_vecPosition;
		outNormal = pVert37->m_vecNormal;
		outTexCoord = pVert37->m_vecTexCoord;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		// v44+ path: Use isolated v44+ API - NO casting from v37 types
		const mstudiomesh_v44_t* pMesh44;
		const mstudiomodel_v44_t* pModel44;
		const void* pVertexBase;

		if (Studio_GetV44MeshAndModel(pStudioHdr, pMesh, pMesh44, pModel44, pVertexBase))
		{
			mstudioboneweight_t unusedBoneWeight;
			StudioMesh_V44_GetVertexData(pMesh44, pModel44, pVertexBase, idx,
				outPosition, outNormal, outTexCoord, unusedBoneWeight);
		}
		else
		{
			outPosition.Init();
			outNormal.Init(0, 0, 1);
			outTexCoord.Init();
		}
	}
	else
	{
		mstudiovertex_t* pVert = pMesh->Vertex(idx);
		if (pVert)
		{
			outPosition = pVert->m_vecPosition;
			outNormal = pVert->m_vecNormal;
			outTexCoord = pVert->m_vecTexCoord;
		}
		else
		{
			outPosition.Init();
			outNormal.Init(0, 0, 1);
			outTexCoord.Init();
		}
	}
}

//-----------------------------------------------------------------------------
// Get bone weight data with version awareness
// For v37 models, uses the 32-byte mstudioboneweight_v37_t
// For v44+ models, delegates to isolated v44+ API (no casting from v37 types)
//-----------------------------------------------------------------------------
inline void Studio_GetBoneWeightData_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	float outWeights[4],
	int outBones[4],
	int& outNumBones )
{
	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		// v37 models have embedded vertex data with 32-byte bone weights
		mstudiomodel_t* pModel = pMesh->pModel();
		int vertexOffset = pMesh->vertexoffset + idx;
		byte* pVertexBase = ((byte*)pModel) + pModel->vertexindex;
		mstudiovertex_v37_t* pVert37 = (mstudiovertex_v37_t*)pVertexBase + vertexOffset;
		mstudioboneweight_v37_t* pBoneWeight37 = &pVert37->m_BoneWeights;

		outNumBones = pBoneWeight37->numbones;
		for (int i = 0; i < 4; ++i)
		{
			outWeights[i] = pBoneWeight37->weight[i];
			outBones[i] = pBoneWeight37->bone[i];  // v37 uses short, but we convert to int
		}
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		// v44+ path: Use isolated v44+ API - NO casting from v37 types
		const mstudiomesh_v44_t* pMesh44;
		const mstudiomodel_v44_t* pModel44;
		const void* pVertexBase;

		if (Studio_GetV44MeshAndModel(pStudioHdr, pMesh, pMesh44, pModel44, pVertexBase))
		{
			mstudioboneweight_t boneWeight;
			if (StudioMesh_V44_GetBoneWeights(pMesh44, pModel44, pVertexBase, idx, boneWeight))
			{
				outNumBones = boneWeight.numbones;
				for (int i = 0; i < MAX_NUM_BONES_PER_VERT; ++i)
				{
					outWeights[i] = boneWeight.weight[i];
					outBones[i] = boneWeight.bone[i];
				}
				for (int i = MAX_NUM_BONES_PER_VERT; i < 4; ++i)
				{
					outWeights[i] = 0.0f;
					outBones[i] = 0;
				}
				return;
			}
		}
		// Fallback
		outNumBones = 1;
		outWeights[0] = 1.0f;
		outBones[0] = 0;
		for (int i = 1; i < 4; ++i)
		{
			outWeights[i] = 0.0f;
			outBones[i] = 0;
		}
	}
	else
	{
		mstudioboneweight_t* pBoneWeight = pMesh->BoneWeights(idx);
		if (pBoneWeight)
		{
			outNumBones = pBoneWeight->numbones;
			for (int i = 0; i < MAX_NUM_BONES_PER_VERT; ++i)
			{
				outWeights[i] = pBoneWeight->weight[i];
				outBones[i] = pBoneWeight->bone[i];
			}
			for (int i = MAX_NUM_BONES_PER_VERT; i < 4; ++i)
			{
				outWeights[i] = 0.0f;
				outBones[i] = 0;
			}
		}
		else
		{
			outNumBones = 1;
			outWeights[0] = 1.0f;
			outBones[0] = 0;
			for (int i = 1; i < 4; ++i)
			{
				outWeights[i] = 0.0f;
				outBones[i] = 0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Get position with version awareness
// For v44+ models, delegates to isolated v44+ API (no casting from v37 types)
//-----------------------------------------------------------------------------
inline void Studio_GetPosition_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	Vector& outPosition )
{
	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		mstudiomodel_t* pModel = pMesh->pModel();
		int vertexOffset = pMesh->vertexoffset + idx;
		byte* pVertexBase = ((byte*)pModel) + pModel->vertexindex;
		mstudiovertex_v37_t* pVert37 = (mstudiovertex_v37_t*)pVertexBase + vertexOffset;
		outPosition = pVert37->m_vecPosition;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		// v44+ path: Use isolated v44+ API - NO casting from v37 types
		const mstudiomesh_v44_t* pMesh44;
		const mstudiomodel_v44_t* pModel44;
		const void* pVertexBase;

		if (Studio_GetV44MeshAndModel(pStudioHdr, pMesh, pMesh44, pModel44, pVertexBase))
		{
			StudioMesh_V44_GetPosition(pMesh44, pModel44, pVertexBase, idx, outPosition);
		}
		else
		{
			outPosition.Init();
		}
	}
	else
	{
		Vector* pPos = pMesh->Position(idx);
		if (pPos)
			outPosition = *pPos;
		else
			outPosition.Init();
	}
}

//-----------------------------------------------------------------------------
// Get normal with version awareness
// For v44+ models, delegates to isolated v44+ API (no casting from v37 types)
//-----------------------------------------------------------------------------
inline void Studio_GetNormal_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	Vector& outNormal )
{
	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		mstudiomodel_t* pModel = pMesh->pModel();
		int vertexOffset = pMesh->vertexoffset + idx;
		byte* pVertexBase = ((byte*)pModel) + pModel->vertexindex;
		mstudiovertex_v37_t* pVert37 = (mstudiovertex_v37_t*)pVertexBase + vertexOffset;
		outNormal = pVert37->m_vecNormal;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		// v44+ path: Use isolated v44+ API - NO casting from v37 types
		const mstudiomesh_v44_t* pMesh44;
		const mstudiomodel_v44_t* pModel44;
		const void* pVertexBase;

		if (Studio_GetV44MeshAndModel(pStudioHdr, pMesh, pMesh44, pModel44, pVertexBase))
		{
			StudioMesh_V44_GetNormal(pMesh44, pModel44, pVertexBase, idx, outNormal);
		}
		else
		{
			outNormal.Init(0, 0, 1);
		}
	}
	else
	{
		Vector* pNorm = pMesh->Normal(idx);
		if (pNorm)
			outNormal = *pNorm;
		else
			outNormal.Init(0, 0, 1);
	}
}

//-----------------------------------------------------------------------------
// Get bone weights pointer with version awareness
// Returns a temporary structure that can be used for bone transforms
// For v44+ models, delegates to isolated v44+ API (no casting from v37 types)
//-----------------------------------------------------------------------------
struct StudioBoneWeightInfo_t
{
	float weight[4];
	int bone[4];
	int numbones;
};

inline void Studio_GetBoneWeightInfo_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	StudioBoneWeightInfo_t& outInfo )
{
	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		mstudiomodel_t* pModel = pMesh->pModel();
		int vertexOffset = pMesh->vertexoffset + idx;
		byte* pVertexBase = ((byte*)pModel) + pModel->vertexindex;
		mstudiovertex_v37_t* pVert37 = (mstudiovertex_v37_t*)pVertexBase + vertexOffset;
		mstudioboneweight_v37_t* pBoneWeight37 = &pVert37->m_BoneWeights;

		outInfo.numbones = pBoneWeight37->numbones;
		for (int i = 0; i < 4; ++i)
		{
			outInfo.weight[i] = pBoneWeight37->weight[i];
			outInfo.bone[i] = pBoneWeight37->bone[i];
		}
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		// v44+ path: Use isolated v44+ API - NO casting from v37 types
		const mstudiomesh_v44_t* pMesh44;
		const mstudiomodel_v44_t* pModel44;
		const void* pVertexBase;

		if (Studio_GetV44MeshAndModel(pStudioHdr, pMesh, pMesh44, pModel44, pVertexBase))
		{
			mstudioboneweight_t boneWeight;
			if (StudioMesh_V44_GetBoneWeights(pMesh44, pModel44, pVertexBase, idx, boneWeight))
			{
				outInfo.numbones = boneWeight.numbones;
				for (int i = 0; i < MAX_NUM_BONES_PER_VERT; ++i)
				{
					outInfo.weight[i] = boneWeight.weight[i];
					outInfo.bone[i] = boneWeight.bone[i];
				}
				for (int i = MAX_NUM_BONES_PER_VERT; i < 4; ++i)
				{
					outInfo.weight[i] = 0.0f;
					outInfo.bone[i] = 0;
				}
				return;
			}
		}
		// Fallback
		outInfo.numbones = 1;
		outInfo.weight[0] = 1.0f;
		outInfo.bone[0] = 0;
		for (int i = 1; i < 4; ++i)
		{
			outInfo.weight[i] = 0.0f;
			outInfo.bone[i] = 0;
		}
	}
	else
	{
		mstudioboneweight_t* pBoneWeight = pMesh->BoneWeights(idx);
		if (pBoneWeight)
		{
			outInfo.numbones = pBoneWeight->numbones;
			for (int i = 0; i < MAX_NUM_BONES_PER_VERT; ++i)
			{
				outInfo.weight[i] = pBoneWeight->weight[i];
				outInfo.bone[i] = pBoneWeight->bone[i];
			}
			for (int i = MAX_NUM_BONES_PER_VERT; i < 4; ++i)
			{
				outInfo.weight[i] = 0.0f;
				outInfo.bone[i] = 0;
			}
		}
		else
		{
			outInfo.numbones = 1;
			outInfo.weight[0] = 1.0f;
			outInfo.bone[0] = 0;
			for (int i = 1; i < 4; ++i)
			{
				outInfo.weight[i] = 0.0f;
				outInfo.bone[i] = 0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Get mstudioboneweight_t-compatible data for v37 models
// For v44+ models, delegates to isolated v44+ API (no casting from v37 types)
//-----------------------------------------------------------------------------
inline void Studio_GetBoneWeight_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	mstudioboneweight_t& outBoneWeight )
{
	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		mstudiomodel_t* pModel = pMesh->pModel();
		int vertexOffset = pMesh->vertexoffset + idx;
		byte* pVertexBase = ((byte*)pModel) + pModel->vertexindex;
		mstudiovertex_v37_t* pVert37 = (mstudiovertex_v37_t*)pVertexBase + vertexOffset;
		mstudioboneweight_v37_t* pBoneWeight37 = &pVert37->m_BoneWeights;

		// Copy weights (v37 has 4 weights, v48 has 3 - use min)
		for (int i = 0; i < MAX_NUM_BONES_PER_VERT; ++i)
		{
			outBoneWeight.weight[i] = pBoneWeight37->weight[i];
			// v37 uses short bone indices, v48 uses char - truncate (safe for <128 bones)
			outBoneWeight.bone[i] = (char)pBoneWeight37->bone[i];
		}
		outBoneWeight.numbones = (byte)pBoneWeight37->numbones;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		// v44+ path: Use isolated v44+ API - NO casting from v37 types
		const mstudiomesh_v44_t* pMesh44;
		const mstudiomodel_v44_t* pModel44;
		const void* pVertexBase;

		if (Studio_GetV44MeshAndModel(pStudioHdr, pMesh, pMesh44, pModel44, pVertexBase))
		{
			StudioMesh_V44_GetBoneWeights(pMesh44, pModel44, pVertexBase, idx, outBoneWeight);
		}
		else
		{
			outBoneWeight.numbones = 1;
			outBoneWeight.weight[0] = 1.0f;
			outBoneWeight.bone[0] = 0;
			for (int i = 1; i < MAX_NUM_BONES_PER_VERT; ++i)
			{
				outBoneWeight.weight[i] = 0.0f;
				outBoneWeight.bone[i] = 0;
			}
		}
	}
	else
	{
		mstudioboneweight_t* pBoneWeight = pMesh->BoneWeights(idx);
		if (pBoneWeight)
		{
			outBoneWeight = *pBoneWeight;
		}
		else
		{
			outBoneWeight.numbones = 1;
			outBoneWeight.weight[0] = 1.0f;
			outBoneWeight.bone[0] = 0;
			for (int i = 1; i < MAX_NUM_BONES_PER_VERT; ++i)
			{
				outBoneWeight.weight[i] = 0.0f;
				outBoneWeight.bone[i] = 0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Combined vertex and bone weight retrieval for decal code
// For v44+ models, delegates to isolated v44+ API (no casting from v37 types)
//-----------------------------------------------------------------------------
inline void Studio_GetVertexAndBoneWeight_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	Vector& outPosition,
	Vector& outNormal,
	mstudioboneweight_t& outBoneWeight )
{
	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		mstudiomodel_t* pModel = pMesh->pModel();
		int vertexOffset = pMesh->vertexoffset + idx;
		byte* pVertexBase = ((byte*)pModel) + pModel->vertexindex;
		mstudiovertex_v37_t* pVert37 = (mstudiovertex_v37_t*)pVertexBase + vertexOffset;

		outPosition = pVert37->m_vecPosition;
		outNormal = pVert37->m_vecNormal;

		mstudioboneweight_v37_t* pBoneWeight37 = &pVert37->m_BoneWeights;
		for (int i = 0; i < MAX_NUM_BONES_PER_VERT; ++i)
		{
			outBoneWeight.weight[i] = pBoneWeight37->weight[i];
			outBoneWeight.bone[i] = (char)pBoneWeight37->bone[i];
		}
		outBoneWeight.numbones = (byte)pBoneWeight37->numbones;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		// v44+ path: Use isolated v44+ API - NO casting from v37 types
		const mstudiomesh_v44_t* pMesh44;
		const mstudiomodel_v44_t* pModel44;
		const void* pVertexBase;

		if (Studio_GetV44MeshAndModel(pStudioHdr, pMesh, pMesh44, pModel44, pVertexBase))
		{
			Vector2D unusedTexCoord;
			StudioMesh_V44_GetVertexData(pMesh44, pModel44, pVertexBase, idx,
				outPosition, outNormal, unusedTexCoord, outBoneWeight);
		}
		else
		{
			outPosition.Init();
			outNormal.Init(0, 0, 1);
			outBoneWeight.numbones = 1;
			outBoneWeight.weight[0] = 1.0f;
			outBoneWeight.bone[0] = 0;
			for (int i = 1; i < MAX_NUM_BONES_PER_VERT; ++i)
			{
				outBoneWeight.weight[i] = 0.0f;
				outBoneWeight.bone[i] = 0;
			}
		}
	}
	else
	{
		mstudiovertex_t* pVert = pMesh->Vertex(idx);
		if (pVert)
		{
			outPosition = pVert->m_vecPosition;
			outNormal = pVert->m_vecNormal;
			outBoneWeight = pVert->m_BoneWeights;
		}
		else
		{
			outPosition.Init();
			outNormal.Init(0, 0, 1);
			outBoneWeight.numbones = 1;
			outBoneWeight.weight[0] = 1.0f;
			outBoneWeight.bone[0] = 0;
			for (int i = 1; i < MAX_NUM_BONES_PER_VERT; ++i)
			{
				outBoneWeight.weight[i] = 0.0f;
				outBoneWeight.bone[i] = 0;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Get vertex pointer with version awareness
// Returns mstudiovertex_t* for direct access to vertex data
// For v37 models, returns NULL (use Studio_GetVertexData_V37Aware instead)
// For v44+ models, uses isolated v44+ API (no casting from v37 types)
//-----------------------------------------------------------------------------
inline mstudiovertex_t* Studio_GetVertex_VersionAware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx )
{
	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		// v37 uses different vertex structure (mstudiovertex_v37_t)
		// Cannot return mstudiovertex_t* for v37 models - caller should use
		// Studio_GetVertexData_V37Aware or handle v37 separately
		return NULL;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		// v44+ path: Use isolated v44+ API - NO casting from v37 types
		const mstudiomesh_v44_t* pMesh44;
		const mstudiomodel_v44_t* pModel44;
		const void* pVertexBase;

		if (Studio_GetV44MeshAndModel(pStudioHdr, pMesh, pMesh44, pModel44, pVertexBase))
		{
			// Cast is safe: mstudiovertex_v44_t and mstudiovertex_t have identical 48-byte layout
			return const_cast<mstudiovertex_t*>(reinterpret_cast<const mstudiovertex_t*>(
				StudioMesh_V44_GetVertex(pMesh44, pModel44, pVertexBase, idx)));
		}
		return NULL;
	}
	else
	{
		// Standard embedded vertex access for older models
		return pMesh->Vertex(idx);
	}
}

//-----------------------------------------------------------------------------
// Get tangent S with version awareness
// Returns Vector4D* for direct access to tangent data
// For v37 models, returns NULL (v37 may not have tangent data)
// For v44+ models, tangent data not fully implemented in LeakNet - returns NULL
//-----------------------------------------------------------------------------
inline Vector4D* Studio_GetTangentS_VersionAware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx )
{
	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		// v37 models may not have tangent data, or it's embedded differently
		// Return NULL and let the caller compute tangents from normal
		return NULL;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		const mstudiomesh_v44_t* pMesh44;
		const mstudiomodel_v44_t* pModel44;
		const void* pVertexBase;

		if (Studio_GetV44MeshAndModel(pStudioHdr, pMesh, pMesh44, pModel44, pVertexBase))
		{
			return StudioMesh_V44_GetTangentS(pMesh44, pModel44, idx);
		}
		return NULL;
	}
	else
	{
		// Standard embedded tangent access for older models
		return pMesh->TangentS(idx);
	}
}

//-----------------------------------------------------------------------------
// Version-aware helper functions for accessing models and meshes
// Common helpers (GetBodypart, GetNumBodyparts, etc.) are in studio_helpers.h
// These are studiorender-specific helpers with version dispatch logic
//-----------------------------------------------------------------------------

// Get mesh from model (version-aware)
inline mstudiomesh_t* StudioModel_GetMesh(const studiohdr_t* pStudioHdr, mstudiomodel_t* pModel, int i)
{
	if (!pStudioHdr || !pModel)
		return NULL;

	if (pStudioHdr->version >= STUDIO_VERSION_44)
	{
		// For v44+, use the v44+ mesh accessor
		mstudiomodel_v44_t* pModel44 = reinterpret_cast<mstudiomodel_v44_t*>(pModel);
		return reinterpret_cast<mstudiomesh_t*>(pModel44->pMesh(i));
	}

	return pModel->pMesh(i);
}

// Get number of vertices for a mesh (version-aware)
inline int StudioMesh_GetNumVertices(const studiohdr_t* pStudioHdr, mstudiomesh_t* pMesh)
{
	if (!pMesh)
		return 0;
	return pMesh->numvertices;
}

// Get number of meshes in a model (version-aware)
inline int StudioModel_GetNumMeshes(const studiohdr_t* pStudioHdr, mstudiomodel_t* pModel)
{
	if (!pStudioHdr || !pModel)
		return 0;

	if (pStudioHdr->version >= STUDIO_VERSION_44)
	{
		mstudiomodel_v44_t* pModel44 = reinterpret_cast<mstudiomodel_v44_t*>(pModel);
		return pModel44->nummeshes;
	}

	return pModel->nummeshes;
}

// Get mesh ID (material index) (version-aware)
inline int StudioMesh_GetMeshId(const studiohdr_t* pStudioHdr, mstudiomesh_t* pMesh)
{
	if (!pStudioHdr || !pMesh)
		return 0;

	if (pStudioHdr->version >= STUDIO_VERSION_44)
	{
		mstudiomesh_v44_t* pMesh44 = reinterpret_cast<mstudiomesh_v44_t*>(pMesh);
		return pMesh44->meshid;
	}

	return pMesh->meshid;
}

inline int StudioMesh_GetMaterial(const studiohdr_t* pStudioHdr, mstudiomesh_t* pMesh)
{
	if (!pStudioHdr || !pMesh)
		return 0;

	if (pStudioHdr->version >= STUDIO_VERSION_44)
	{
		mstudiomesh_v44_t* pMesh44 = reinterpret_cast<mstudiomesh_v44_t*>(pMesh);
		return pMesh44->material;
	}

	return pMesh->material;
}

inline int StudioMesh_GetMaterialType(const studiohdr_t* pStudioHdr, mstudiomesh_t* pMesh)
{
	if (!pStudioHdr || !pMesh)
		return 0;

	if (pStudioHdr->version >= STUDIO_VERSION_44)
	{
		mstudiomesh_v44_t* pMesh44 = reinterpret_cast<mstudiomesh_v44_t*>(pMesh);
		return pMesh44->materialtype;
	}

	return pMesh->materialtype;
}

inline int StudioMesh_GetMaterialParam(const studiohdr_t* pStudioHdr, mstudiomesh_t* pMesh)
{
	if (!pStudioHdr || !pMesh)
		return 0;

	if (pStudioHdr->version >= STUDIO_VERSION_44)
	{
		mstudiomesh_v44_t* pMesh44 = reinterpret_cast<mstudiomesh_v44_t*>(pMesh);
		return pMesh44->materialparam;
	}

	return pMesh->materialparam;
}

// Get number of textures (version-aware)
inline int StudioHdr_GetNumTextures_VersionAware(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;

	// CRITICAL FIX: Must use correct type based on model version
	if (pStudioHdr->version >= STUDIO_VERSION_44)
	{
		const studiohdr_v44_t* pHdr44 = (const studiohdr_v44_t*)pStudioHdr;
		return pHdr44->numtextures;
	}
	return pStudioHdr->numtextures;
}

#endif // STUDIO_V37_COMPAT_H
