//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Version-aware vertex access helpers for v37 (HL2 Beta 2003) models
//          and v44+ models with external VVD vertex data.
//
// v37 models have embedded vertex data with different structure sizes:
// - mstudiovertex_v37_t is 64 bytes (vs 48 bytes for mstudiovertex_t)
// - mstudioboneweight_v37_t is 32 bytes (vs 16 bytes for mstudioboneweight_t)
//
// v44+ models use external VVD files with vertex data accessed via
// mstudiomodel_v44_t::vertexdata.pVertexData pointer.
//
// These helper functions provide version-aware access to vertex data.
//
//=============================================================================

#ifndef STUDIO_V37_COMPAT_H
#define STUDIO_V37_COMPAT_H

#include "studio.h"
#include "studiohdr_v44.h"

//-----------------------------------------------------------------------------
// Get vertex data with version awareness
// For v37 models, uses the 64-byte mstudiovertex_v37_t stride
// For v44+ models, uses the 48-byte mstudiovertex_t stride
//-----------------------------------------------------------------------------
inline void Studio_GetVertexData_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	Vector& outPosition,
	Vector& outNormal,
	Vector2D& outTexCoord )
{
	mstudiomodel_t* pModel = pMesh->pModel();
	int vertexOffset = pMesh->vertexoffset + idx;

	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		// v37 models have embedded vertex data with 64-byte vertices
		byte* pVertexBase = ((byte*)pModel) + pModel->vertexindex;
		mstudiovertex_v37_t* pVert37 = (mstudiovertex_v37_t*)pVertexBase + vertexOffset;

		outPosition = pVert37->m_vecPosition;
		outNormal = pVert37->m_vecNormal;
		outTexCoord = pVert37->m_vecTexCoord;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		// v44+ models use external VVD with vertex pointer set up by modelloader
		mstudiomodel_v44_t* pModel44 = (mstudiomodel_v44_t*)pModel;
		if ( pModel44->vertexdata.pVertexData )
		{
			mstudiovertex_t* pVert = (mstudiovertex_t*)pModel44->vertexdata.pVertexData + vertexOffset;
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
// For v44+ models, uses the 16-byte mstudioboneweight_t
//-----------------------------------------------------------------------------
inline void Studio_GetBoneWeightData_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	float outWeights[4],
	int outBones[4],
	int& outNumBones )
{
	mstudiomodel_t* pModel = pMesh->pModel();
	int vertexOffset = pMesh->vertexoffset + idx;

	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		// v37 models have embedded vertex data with 32-byte bone weights
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
		// v44+ models use external VVD
		mstudiomodel_v44_t* pModel44 = (mstudiomodel_v44_t*)pModel;
		if ( pModel44->vertexdata.pVertexData )
		{
			mstudiovertex_t* pVert = (mstudiovertex_t*)pModel44->vertexdata.pVertexData + vertexOffset;
			mstudioboneweight_t* pBoneWeight = &pVert->m_BoneWeights;
			outNumBones = pBoneWeight->numbones;
			for (int i = 0; i < MAX_NUM_BONES_PER_VERT; ++i)
			{
				outWeights[i] = pBoneWeight->weight[i];
				outBones[i] = pBoneWeight->bone[i];
			}
			if (MAX_NUM_BONES_PER_VERT < 4)
			{
				outWeights[3] = 0.0f;
				outBones[3] = 0;
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
			if (MAX_NUM_BONES_PER_VERT < 4)
			{
				outWeights[3] = 0.0f;
				outBones[3] = 0;
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
//-----------------------------------------------------------------------------
inline void Studio_GetPosition_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	Vector& outPosition )
{
	mstudiomodel_t* pModel = pMesh->pModel();
	int vertexOffset = pMesh->vertexoffset + idx;

	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		byte* pVertexBase = ((byte*)pModel) + pModel->vertexindex;
		mstudiovertex_v37_t* pVert37 = (mstudiovertex_v37_t*)pVertexBase + vertexOffset;
		outPosition = pVert37->m_vecPosition;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		mstudiomodel_v44_t* pModel44 = (mstudiomodel_v44_t*)pModel;
		if ( pModel44->vertexdata.pVertexData )
		{
			mstudiovertex_t* pVert = (mstudiovertex_t*)pModel44->vertexdata.pVertexData + vertexOffset;
			outPosition = pVert->m_vecPosition;
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
//-----------------------------------------------------------------------------
inline void Studio_GetNormal_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	Vector& outNormal )
{
	mstudiomodel_t* pModel = pMesh->pModel();
	int vertexOffset = pMesh->vertexoffset + idx;

	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		byte* pVertexBase = ((byte*)pModel) + pModel->vertexindex;
		mstudiovertex_v37_t* pVert37 = (mstudiovertex_v37_t*)pVertexBase + vertexOffset;
		outNormal = pVert37->m_vecNormal;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		mstudiomodel_v44_t* pModel44 = (mstudiomodel_v44_t*)pModel;
		if ( pModel44->vertexdata.pVertexData )
		{
			mstudiovertex_t* pVert = (mstudiovertex_t*)pModel44->vertexdata.pVertexData + vertexOffset;
			outNormal = pVert->m_vecNormal;
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
// Note: For v37, this copies data since the structure layout differs
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
	mstudiomodel_t* pModel = pMesh->pModel();
	int vertexOffset = pMesh->vertexoffset + idx;

	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
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
		mstudiomodel_v44_t* pModel44 = (mstudiomodel_v44_t*)pModel;
		if ( pModel44->vertexdata.pVertexData )
		{
			mstudiovertex_t* pVert = (mstudiovertex_t*)pModel44->vertexdata.pVertexData + vertexOffset;
			mstudioboneweight_t* pBoneWeight = &pVert->m_BoneWeights;
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
// This fills a temporary mstudioboneweight_t that can be passed to existing functions
// Note: For v37, bone indices are shorts but we truncate to char (valid for <128 bones)
//-----------------------------------------------------------------------------
inline void Studio_GetBoneWeight_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	mstudioboneweight_t& outBoneWeight )
{
	mstudiomodel_t* pModel = pMesh->pModel();
	int vertexOffset = pMesh->vertexoffset + idx;

	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
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
		mstudiomodel_v44_t* pModel44 = (mstudiomodel_v44_t*)pModel;
		if ( pModel44->vertexdata.pVertexData )
		{
			mstudiovertex_t* pVert = (mstudiovertex_t*)pModel44->vertexdata.pVertexData + vertexOffset;
			outBoneWeight = pVert->m_BoneWeights;
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
//-----------------------------------------------------------------------------
inline void Studio_GetVertexAndBoneWeight_V37Aware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx,
	Vector& outPosition,
	Vector& outNormal,
	mstudioboneweight_t& outBoneWeight )
{
	mstudiomodel_t* pModel = pMesh->pModel();
	int vertexOffset = pMesh->vertexoffset + idx;

	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
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
		mstudiomodel_v44_t* pModel44 = (mstudiomodel_v44_t*)pModel;
		if ( pModel44->vertexdata.pVertexData )
		{
			mstudiovertex_t* pVert = (mstudiovertex_t*)pModel44->vertexdata.pVertexData + vertexOffset;
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
// For v44+ models, uses external VVD with pVertexData pointer
//-----------------------------------------------------------------------------
inline mstudiovertex_t* Studio_GetVertex_VersionAware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx )
{
	mstudiomodel_t* pModel = pMesh->pModel();
	int vertexOffset = pMesh->vertexoffset + idx;

	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		// v37 uses different vertex structure (mstudiovertex_v37_t)
		// Cannot return mstudiovertex_t* for v37 models - caller should use
		// Studio_GetVertexData_V37Aware or handle v37 separately
		return NULL;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		// v44+ models use external VVD with vertex pointer set up by modelloader
		mstudiomodel_v44_t* pModel44 = (mstudiomodel_v44_t*)pModel;
		if ( pModel44->vertexdata.pVertexData )
		{
			return (mstudiovertex_t*)pModel44->vertexdata.pVertexData + vertexOffset;
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
// For v44+ models, uses external VVD with pTangentData pointer
//-----------------------------------------------------------------------------
inline Vector4D* Studio_GetTangentS_VersionAware(
	studiohdr_t* pStudioHdr,
	mstudiomesh_t* pMesh,
	int idx )
{
	mstudiomodel_t* pModel = pMesh->pModel();
	int vertexOffset = pMesh->vertexoffset + idx;

	if ( pStudioHdr && pStudioHdr->IsV37() )
	{
		// v37 models may not have tangent data, or it's embedded differently
		// Return NULL and let the caller compute tangents from normal
		return NULL;
	}
	else if ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 )
	{
		// v44+ models use external VVD with tangent pointer set up by modelloader
		mstudiomodel_v44_t* pModel44 = (mstudiomodel_v44_t*)pModel;
		if ( pModel44->vertexdata.pTangentData )
		{
			return (Vector4D*)pModel44->vertexdata.pTangentData + vertexOffset;
		}
		return NULL;
	}
	else
	{
		// Standard embedded tangent access for older models
		return pMesh->TangentS(idx);
	}
}

#endif // STUDIO_V37_COMPAT_H
