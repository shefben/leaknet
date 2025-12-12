//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Triangle extraction from studio models
//
// $NoKeywords: $
//=============================================================================

#include "cstudiorender.h"
#include "studio_v37_compat.h"
#include "optimize.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void CStudioRender::GetTriangles( const DrawModelInfo_t& info, matrix3x4_t *pBoneToWorld, GetTriangles_Output_t &out )
{
	VPROF( "CStudioRender::GetTriangles");

	out.m_MaterialBatches.RemoveAll(); // clear out data.

	if( !info.m_pStudioHdr || !info.m_pHardwareData ||
		!info.m_pHardwareData->m_NumLODs || !info.m_pHardwareData->m_pLODs )
	{
		return;
	}

	int lod = info.m_Lod;
	int lastlod = info.m_pHardwareData->m_NumLODs - 1;

	if ( lod == USESHADOWLOD )
	{
		lod = lastlod;
	}
	else
	{
		lod = clamp( lod, 0, lastlod );
	}

	// No root LOD clamping needed for this engine version

	int nSkin = info.m_Skin;
	if ( nSkin >= StudioHdr_GetNumSkinFamilies(info.m_pStudioHdr) )
	{
		nSkin = 0;
	}
	short *pSkinRef	= StudioHdr_GetSkinRef( info.m_pStudioHdr, nSkin * StudioHdr_GetNumSkinRef(info.m_pStudioHdr) );

	studiomeshdata_t *pStudioMeshes = info.m_pHardwareData->m_pLODs[lod].m_pMeshData;
	IMaterial **ppMaterials = info.m_pHardwareData->m_pLODs[lod].ppMaterials;

	// Bone to world must be set before calling this function; it uses it here
	ComputePoseToWorld( info.m_pStudioHdr );

	// Copy pose to world matrices to output
	for ( int i = 0; i < MAXSTUDIOBONES; i++ )
	{
		out.m_PoseToWorld[i] = m_PoseToWorld[i];
	}

	int i;
	for (i=0 ; i < StudioHdr_GetNumBodyparts(info.m_pStudioHdr) ; i++)
	{
		mstudiomodel_t *pModel = NULL;
		R_StudioSetupModel( i, info.m_Body, &pModel, info.m_pStudioHdr );

		// Iterate over all the meshes.... each mesh is a new material
		// Use version-safe accessors for v44+ model compatibility
		int numMeshes = StudioModel_GetNumMeshes(info.m_pStudioHdr, pModel);
		int k;
		for ( k = 0; k < numMeshes; ++k )
		{
			GetTriangles_MaterialBatch_t &materialBatch = out.m_MaterialBatches[out.m_MaterialBatches.AddToTail()];
			mstudiomesh_t *pMesh = StudioModel_GetMesh(info.m_pStudioHdr, pModel, k);

			// add the verts from this mesh to the materialBatch
			// Use version-safe accessor for numvertices
			int numVertices = StudioMesh_GetNumVertices(info.m_pStudioHdr, pMesh);
			materialBatch.m_Verts.SetCount( numVertices );
			for ( int vertID = 0; vertID < numVertices; vertID++ )
			{
				GetTriangles_Vertex_t& vert = materialBatch.m_Verts[vertID];

				// Use version-aware vertex access for v37 model compatibility
				Vector vecPosition, vecNormal;
				Vector2D vecTexCoord;
				Studio_GetVertexData_V37Aware( info.m_pStudioHdr, pMesh, vertID,
					vecPosition, vecNormal, vecTexCoord );

				vert.m_Position = vecPosition;
				vert.m_Normal   = vecNormal;
				vert.m_TexCoord = vecTexCoord;

				// Check if mesh has tangent data
				// Note: v37 models don't have tangent data, so skip for them
				// Also check tangentsindex > 0 since 0 means no tangent data (would point to struct start)
				// Use version-aware tangent access for v44+ models with external VVD data
				if ( !info.m_pStudioHdr->IsV37() && pModel->tangentsindex > 0 && numVertices > 0 )
				{
					Vector4D* pTangent = Studio_GetTangentS_VersionAware( info.m_pStudioHdr, pMesh, vertID );
					if ( pTangent )
					{
						Vector4DCopy( *pTangent, vert.m_TangentS );
					}
#if _DEBUG
					else
					{
						// ensure any unintended access faults
						vert.m_TangentS.Init( VEC_T_NAN, VEC_T_NAN, VEC_T_NAN, VEC_T_NAN );
					}
#endif
				}
#if _DEBUG
				else
				{
					// ensure any unintended access faults
					vert.m_TangentS.Init( VEC_T_NAN, VEC_T_NAN, VEC_T_NAN, VEC_T_NAN );
				}
#endif

				// Use version-aware bone weight access for v37 model compatibility
				mstudioboneweight_t boneWeight;
				Studio_GetBoneWeight_V37Aware( info.m_pStudioHdr, pMesh, vertID, boneWeight );
				vert.m_NumBones = boneWeight.numbones;
				int j;
				for ( j = 0; j < vert.m_NumBones; j++ )
				{
					vert.m_BoneWeight[j] = boneWeight.weight[j];
					vert.m_BoneIndex[j] = boneWeight.bone[j];
				}
			}

			IMaterial *pMaterial = ppMaterials[pSkinRef[pMesh->material]];
			Assert( pMaterial );
			materialBatch.m_pMaterial = pMaterial;
			studiomeshdata_t *pMeshData = &pStudioMeshes[StudioMesh_GetMeshId(info.m_pStudioHdr, pMesh)];
			if ( pMeshData->m_NumGroup == 0 )
				continue;

			// Clear out indices
			materialBatch.m_TriListIndices.SetCount( 0 );

			// Iterate over all stripgroups
			int stripGroupID;
			for ( stripGroupID = 0; stripGroupID < pMeshData->m_NumGroup; stripGroupID++ )
			{
				studiomeshgroup_t *pMeshGroup = &pMeshData->m_pMeshGroup[stripGroupID];
//				bool bIsFlexed = ( pMeshGroup->m_Flags & MESHGROUP_IS_FLEXED ) != 0;
//				bool bIsHWSkinned = ( pMeshGroup->m_Flags & MESHGROUP_IS_HWSKINNED ) != 0;

				// Iterate over all strips. . . each strip potentially changes bones states.
				int stripID;
				for ( stripID = 0; stripID < pMeshGroup->m_NumStrips; stripID++ )
				{
					OptimizedModel::StripHeader_t *pStripData = &pMeshGroup->m_pStripData[stripID];
//					int boneID;
//					for( boneID = 0; boneID < pStripData->numBoneStateChanges; boneID++ )
//					{
//						OptimizedModel::BoneStateChangeHeader_t *pBoneStateChange = pStripData->pBoneStateChange( boneID );
//						hardwareBoneToGlobalBone[pBoneStateChange->hardwareID] = pBoneStateChange->newBoneID;
//					}
					if ( pStripData->flags & OptimizedModel::STRIP_IS_TRILIST )
					{
						for ( int i = 0; i < pStripData->numIndices; i += 3 )
						{
							int idx = pStripData->indexOffset + i;
							materialBatch.m_TriListIndices.AddToTail( pMeshGroup->MeshIndex( idx ) );
							materialBatch.m_TriListIndices.AddToTail( pMeshGroup->MeshIndex( idx + 1 ) );
							materialBatch.m_TriListIndices.AddToTail( pMeshGroup->MeshIndex( idx + 2 ) );
						}
					}
					else
					{
						// triangle strip
						for ( int i = 0; i < pStripData->numIndices - 2; i++ )
						{
							int idx = pStripData->indexOffset + i;

							if ( i & 1 )
							{
								materialBatch.m_TriListIndices.AddToTail( pMeshGroup->MeshIndex( idx ) );
								materialBatch.m_TriListIndices.AddToTail( pMeshGroup->MeshIndex( idx + 2 ) );
								materialBatch.m_TriListIndices.AddToTail( pMeshGroup->MeshIndex( idx + 1 ) );
							}
							else
							{
								materialBatch.m_TriListIndices.AddToTail( pMeshGroup->MeshIndex( idx ) );
								materialBatch.m_TriListIndices.AddToTail( pMeshGroup->MeshIndex( idx + 1 ) );
								materialBatch.m_TriListIndices.AddToTail( pMeshGroup->MeshIndex( idx + 2 ) );
							}
						}
					}
				}
			}
		}
	}
}