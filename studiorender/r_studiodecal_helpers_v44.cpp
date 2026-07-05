//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: v44+ studio decal helper functions - COMPLETELY ISOLATED from v37
//
// This file contains ONLY v44+ code. All types use _v44 suffix.
// NO v37 types are used anywhere in this file.
//
// These helper functions are called by CStudioRender when decaling v44+ models.
//
//=====================================================================================//

// NO studio.h - v44+ code is completely isolated from v37
#include "studiohdr_v44.h"  // All v44+ types defined here

#include "materialsystem/imesh.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "mathlib.h"
#include "vector.h"
#include "vmatrix.h"
#include "tier0/vprof.h"
#include "cmodel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// v44+ Decal front-facing check
// NOTE: This only works to rotate normals if there's no scale in the
// pose to world transforms. If we ever add scale, we'll need to
// multiply by the inverse transpose of the pose to decal
//-----------------------------------------------------------------------------
#define FRONTFACING_EPS_V44	0.1f

bool IsFrontFacing_v44( const Vector *pnorm,
						const mstudioboneweight_v44_t *pboneweight,
						const matrix3x4_t *pPoseToDecal )
{
	float z;
	if (pboneweight->numbones == 1)
	{
		z = DotProduct( pnorm->Base(), pPoseToDecal[pboneweight->bone[0]][2] );
	}
	else
	{
		float zbone;
		z = 0;
		for (int i = 0; i < pboneweight->numbones; i++)
		{
			zbone = DotProduct( pnorm->Base(), pPoseToDecal[pboneweight->bone[i]][2] );
			z += zbone * pboneweight->weight[i];
		}
	}

	return ( z >= FRONTFACING_EPS_V44 );
}


//-----------------------------------------------------------------------------
// v44+ Transform vertex to decal space
//-----------------------------------------------------------------------------
bool TransformToDecalSpace_v44( const Vector& pos,
								const mstudioboneweight_v44_t *pboneweight,
								const matrix3x4_t *pPoseToDecal,
								Vector2D& uv,
								bool noPokeThru,
								float radius )
{
	// NOTE: This only works to rotate normals if there's no scale in the
	// pose to world transforms. If we ever add scale, we'll need to
	// multiply by the inverse transpose of the pose to world

	if (pboneweight->numbones == 1)
	{
		uv.x = DotProduct( pos.Base(), pPoseToDecal[pboneweight->bone[0]][0] ) +
			pPoseToDecal[pboneweight->bone[0]][0][3];
		uv.y = DotProduct( pos.Base(), pPoseToDecal[pboneweight->bone[0]][1] ) +
			pPoseToDecal[pboneweight->bone[0]][1][3];
	}
	else
	{
		uv.x = uv.y = 0;
		float ubone, vbone;
		for (int i = 0; i < pboneweight->numbones; i++)
		{
			ubone = DotProduct( pos.Base(), pPoseToDecal[pboneweight->bone[i]][0] ) +
				pPoseToDecal[pboneweight->bone[i]][0][3];
			vbone = DotProduct( pos.Base(), pPoseToDecal[pboneweight->bone[i]][1] ) +
				pPoseToDecal[pboneweight->bone[i]][1][3];

			uv.x += ubone * pboneweight->weight[i];
			uv.y += vbone * pboneweight->weight[i];
		}
	}

	if (!noPokeThru)
		return true;

	// No poke thru? do culling....
	float z;
	if (pboneweight->numbones == 1)
	{
		z = DotProduct( pos.Base(), pPoseToDecal[pboneweight->bone[0]][2] ) +
			pPoseToDecal[pboneweight->bone[0]][2][3];
	}
	else
	{
		z = 0;
		float zbone;
		for (int i = 0; i < pboneweight->numbones; i++)
		{
			zbone = DotProduct( pos.Base(), pPoseToDecal[pboneweight->bone[i]][2] ) +
				pPoseToDecal[pboneweight->bone[i]][2][3];
			z += zbone * pboneweight->weight[i];
		}
	}

	return (fabs(z) < radius );
}


//-----------------------------------------------------------------------------
// v44+ Decal vertex info for building decals
//-----------------------------------------------------------------------------
struct DecalBuildVertexInfo_v44_t
{
	Vector2D	m_UV;
	unsigned short m_VertexIndex;	// index into the DecalVertex_t list
	unsigned char  m_UniqueID;
	unsigned char  m_Flags;

	enum
	{
		FRONT_FACING = 0x1,
		VALID_AREA = 0x2,
	};
};


//-----------------------------------------------------------------------------
// v44+ Project decal onto a single mesh
// Returns true if any vertices were processed
//-----------------------------------------------------------------------------
bool ProjectDecalOntoMesh_v44( const studiohdr_v44_t *pStudioHdr,
							   const mstudiomesh_v44_t *pMesh,
							   DecalBuildVertexInfo_v44_t *pVertexInfo,
							   const matrix3x4_t *pPoseToDecal,
							   float radius,
							   bool noPokeThru )
{
	if (!pStudioHdr || !pMesh || !pVertexInfo || !pPoseToDecal)
		return false;

	float invRadius = (radius != 0.0f) ? 1.0f / radius : 1.0f;

	const mstudio_meshvertexdata_v44_t *pVertData = pMesh->GetVertexData();
	if (!pVertData)
		return false;

	// For this to work, the plane and intercept must have been transformed
	// into pose space. Also, we'll not be bothering with flexes.
	for ( int j = 0; j < pMesh->numvertices; ++j )
	{
		const mstudiovertex_v44_t *pVertex = pVertData->Vertex( j );
		if (!pVertex)
			continue;

		Vector vecPosition = pVertex->m_vecPosition;
		Vector vecNormal = pVertex->m_vecNormal;
		const mstudioboneweight_v44_t *pBoneWeight = &pVertex->m_BoneWeights;

		// No decal vertex yet...
		pVertexInfo[j].m_VertexIndex = 0xFFFF;
		pVertexInfo[j].m_UniqueID = 0xFF;
		pVertexInfo[j].m_Flags = 0;

		// We need to know if the normal is pointing in the negative direction
		// if so, blow off all triangles connected to that vertex.
		if ( !IsFrontFacing_v44( &vecNormal, pBoneWeight, pPoseToDecal ) )
			continue;

		pVertexInfo[j].m_Flags |= DecalBuildVertexInfo_v44_t::FRONT_FACING;

		bool inValidArea = TransformToDecalSpace_v44( vecPosition, pBoneWeight,
													  pPoseToDecal, pVertexInfo[j].m_UV,
													  noPokeThru, radius );
		pVertexInfo[j].m_Flags |= ( inValidArea << 1 );

		pVertexInfo[j].m_UV *= invRadius * 0.5f;
		pVertexInfo[j].m_UV[0] += 0.5f;
		pVertexInfo[j].m_UV[1] += 0.5f;
	}
	return true;
}


//-----------------------------------------------------------------------------
// v44+ Compute total mesh count for decaling
//-----------------------------------------------------------------------------
int ComputeTotalMeshCount_v44( const studiohdr_v44_t *pStudioHdr,
							   int iRootLOD,
							   int iMaxLOD,
							   int body )
{
	if (!pStudioHdr)
		return 0;

	int nMeshCount = 0;
	for ( int k = 0; k < pStudioHdr->numbodyparts; k++)
	{
		mstudiobodyparts_v44_t *pBodypart = pStudioHdr->pBodypart( k );
		if (!pBodypart)
			continue;

		// Calculate model index from body value
		int index = body / pBodypart->base;
		index = index % pBodypart->nummodels;

		mstudiomodel_v44_t *pSubModel = pBodypart->pModel( index );
		if (!pSubModel)
			continue;

		nMeshCount += pSubModel->nummeshes;
	}

	nMeshCount *= iMaxLOD - iRootLOD + 1;

	return nMeshCount;
}


//-----------------------------------------------------------------------------
// v44+ Get model from bodypart and body value
//-----------------------------------------------------------------------------
mstudiomodel_v44_t* GetModelFromBodypart_v44( const studiohdr_v44_t *pStudioHdr,
											  int bodypart,
											  int body )
{
	if (!pStudioHdr || bodypart < 0 || bodypart >= pStudioHdr->numbodyparts)
		return NULL;

	mstudiobodyparts_v44_t *pBodypart = pStudioHdr->pBodypart( bodypart );
	if (!pBodypart)
		return NULL;

	int index = body / pBodypart->base;
	index = index % pBodypart->nummodels;

	return pBodypart->pModel( index );
}


//-----------------------------------------------------------------------------
// v44+ Get mesh from model
//-----------------------------------------------------------------------------
mstudiomesh_v44_t* GetMeshFromModel_v44( const mstudiomodel_v44_t *pModel, int meshID )
{
	if (!pModel || meshID < 0 || meshID >= pModel->nummeshes)
		return NULL;

	return pModel->pMesh( meshID );
}


//-----------------------------------------------------------------------------
// v44+ Get number of meshes in model
//-----------------------------------------------------------------------------
int GetNumMeshes_v44( const mstudiomodel_v44_t *pModel )
{
	return pModel ? pModel->nummeshes : 0;
}


//-----------------------------------------------------------------------------
// v44+ Get number of vertices in mesh
//-----------------------------------------------------------------------------
int GetNumVertices_v44( const mstudiomesh_v44_t *pMesh )
{
	return pMesh ? pMesh->numvertices : 0;
}


//-----------------------------------------------------------------------------
// v44+ Get mesh material index
//-----------------------------------------------------------------------------
int GetMeshMaterial_v44( const mstudiomesh_v44_t *pMesh )
{
	return pMesh ? pMesh->material : 0;
}


//-----------------------------------------------------------------------------
// v44+ Get vertex position from mesh
//-----------------------------------------------------------------------------
bool GetVertexPosition_v44( const mstudiomesh_v44_t *pMesh, int vertIndex, Vector& outPosition )
{
	if (!pMesh || vertIndex < 0 || vertIndex >= pMesh->numvertices)
		return false;

	const mstudio_meshvertexdata_v44_t *pVertData = pMesh->GetVertexData();
	if (!pVertData)
		return false;

	const mstudiovertex_v44_t *pVertex = pVertData->Vertex( vertIndex );
	if (!pVertex)
		return false;

	outPosition = pVertex->m_vecPosition;
	return true;
}


//-----------------------------------------------------------------------------
// v44+ Get vertex normal from mesh
//-----------------------------------------------------------------------------
bool GetVertexNormal_v44( const mstudiomesh_v44_t *pMesh, int vertIndex, Vector& outNormal )
{
	if (!pMesh || vertIndex < 0 || vertIndex >= pMesh->numvertices)
		return false;

	const mstudio_meshvertexdata_v44_t *pVertData = pMesh->GetVertexData();
	if (!pVertData)
		return false;

	const mstudiovertex_v44_t *pVertex = pVertData->Vertex( vertIndex );
	if (!pVertex)
		return false;

	outNormal = pVertex->m_vecNormal;
	return true;
}


//-----------------------------------------------------------------------------
// v44+ Get vertex texcoord from mesh
//-----------------------------------------------------------------------------
bool GetVertexTexCoord_v44( const mstudiomesh_v44_t *pMesh, int vertIndex, Vector2D& outTexCoord )
{
	if (!pMesh || vertIndex < 0 || vertIndex >= pMesh->numvertices)
		return false;

	const mstudio_meshvertexdata_v44_t *pVertData = pMesh->GetVertexData();
	if (!pVertData)
		return false;

	const mstudiovertex_v44_t *pVertex = pVertData->Vertex( vertIndex );
	if (!pVertex)
		return false;

	outTexCoord = pVertex->m_vecTexCoord;
	return true;
}


//-----------------------------------------------------------------------------
// v44+ Get vertex bone weights from mesh
//-----------------------------------------------------------------------------
bool GetVertexBoneWeights_v44( const mstudiomesh_v44_t *pMesh, int vertIndex,
							   mstudioboneweight_v44_t& outBoneWeight )
{
	if (!pMesh || vertIndex < 0 || vertIndex >= pMesh->numvertices)
		return false;

	const mstudio_meshvertexdata_v44_t *pVertData = pMesh->GetVertexData();
	if (!pVertData)
		return false;

	const mstudiovertex_v44_t *pVertex = pVertData->Vertex( vertIndex );
	if (!pVertex)
		return false;

	outBoneWeight = pVertex->m_BoneWeights;
	return true;
}


//-----------------------------------------------------------------------------
// v44+ Transform vertex position through bone weights
//-----------------------------------------------------------------------------
void TransformVertex_v44( const Vector& in,
						  const mstudioboneweight_v44_t *pboneweight,
						  const matrix3x4_t *pPoseToWorld,
						  Vector& out )
{
	if (!pboneweight || !pPoseToWorld)
	{
		out = in;
		return;
	}

	switch( pboneweight->numbones )
	{
	case 1:
		VectorTransform( in, pPoseToWorld[pboneweight->bone[0]], out );
		break;
	default:
		{
			Vector out2;
			VectorFill( out, 0 );
			for (int i = 0; i < pboneweight->numbones; i++)
			{
				VectorTransform( in, pPoseToWorld[pboneweight->bone[i]], out2 );
				VectorMA( out, pboneweight->weight[i], out2, out );
			}
		}
		break;
	}
}


//-----------------------------------------------------------------------------
// v44+ Transform normal through bone weights
//-----------------------------------------------------------------------------
void TransformNormal_v44( const Vector& in,
						  const mstudioboneweight_v44_t *pboneweight,
						  const matrix3x4_t *pPoseToWorld,
						  Vector& out )
{
	if (!pboneweight || !pPoseToWorld)
	{
		out = in;
		return;
	}

	if (pboneweight->numbones == 1)
	{
		VectorRotate( in, pPoseToWorld[pboneweight->bone[0]], out );
		VectorNormalize( out );
	}
	else
	{
		Vector out2;
		VectorFill( out, 0 );
		for (int i = 0; i < pboneweight->numbones; i++)
		{
			VectorRotate( in, pPoseToWorld[pboneweight->bone[i]], out2 );
			VectorMA( out, pboneweight->weight[i], out2, out );
		}
		VectorNormalize( out );
	}
}


//-----------------------------------------------------------------------------
// v44+ Check if studio header is v44+
//-----------------------------------------------------------------------------
bool IsV44Model( const studiohdr_v44_t *pStudioHdr )
{
	if (!pStudioHdr)
		return false;

	return pStudioHdr->version >= STUDIO_VERSION_V44;
}


//-----------------------------------------------------------------------------
// v44+ Get number of bodyparts
//-----------------------------------------------------------------------------
int GetNumBodyparts_v44_decal( const studiohdr_v44_t *pStudioHdr )
{
	return pStudioHdr ? pStudioHdr->numbodyparts : 0;
}


//-----------------------------------------------------------------------------
// v44+ Get number of bones
//-----------------------------------------------------------------------------
int GetNumBones_v44_decal( const studiohdr_v44_t *pStudioHdr )
{
	return pStudioHdr ? pStudioHdr->numbones : 0;
}


//-----------------------------------------------------------------------------
// v44+ Get studio header flags
//-----------------------------------------------------------------------------
int GetStudioFlags_v44( const studiohdr_v44_t *pStudioHdr )
{
	return pStudioHdr ? pStudioHdr->flags : 0;
}


//-----------------------------------------------------------------------------
// v44+ Get number of flex descs
//-----------------------------------------------------------------------------
int GetNumFlexDescs_v44( const studiohdr_v44_t *pStudioHdr )
{
	return pStudioHdr ? pStudioHdr->numflexdesc : 0;
}


//-----------------------------------------------------------------------------
// v44+ Get skin reference table
//-----------------------------------------------------------------------------
short* GetSkinRef_v44_decal( const studiohdr_v44_t *pStudioHdr, int offset )
{
	if (!pStudioHdr)
		return NULL;

	return pStudioHdr->pSkinref( offset );
}
