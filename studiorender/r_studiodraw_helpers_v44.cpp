//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: v44+ studio rendering helper functions - COMPLETELY ISOLATED from v37
//
// This file contains ONLY v44+ code. All types use _v44 suffix.
// NO v37 types are used anywhere in this file.
//
// These helper functions are called by CStudioRender when rendering v44+ models.
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
#include "optimize.h"
#include "mathlib.h"
#include "vector.h"
#include "vmatrix.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// v44+ Draw hitbox hulls
//-----------------------------------------------------------------------------
void R_StudioDrawHulls_v44( const studiohdr_v44_t *pStudioHdr,
						    int hitboxset,
						    bool translucent,
						    const matrix3x4_t *pBoneToWorld,
						    IMaterial *pTranslucentMaterial,
						    IMaterial *pSolidMaterial,
						    IMaterialSystem *pMaterialSystem )
{
	if (!pStudioHdr)
		return;

	static int boxpnt[6][4] =
	{
		{ 0, 4, 6, 2 }, // +X
		{ 0, 1, 5, 4 }, // +Y
		{ 0, 2, 3, 1 }, // +Z
		{ 7, 5, 1, 3 }, // -X
		{ 7, 3, 2, 6 }, // -Y
		{ 7, 6, 4, 5 }, // -Z
	};

	static Vector hullcolor[8] =
	{
		Vector( 1.0, 1.0, 1.0 ),
		Vector( 1.0, 0.5, 0.5 ),
		Vector( 0.5, 1.0, 0.5 ),
		Vector( 1.0, 1.0, 0.5 ),
		Vector( 0.5, 0.5, 1.0 ),
		Vector( 1.0, 0.5, 1.0 ),
		Vector( 0.5, 1.0, 1.0 ),
		Vector( 1.0, 1.0, 1.0 )
	};

	mstudiohitboxset_v44_t *s = pStudioHdr->pHitboxSet( hitboxset );
	if ( !s )
		return;

	mstudiobbox_v44_t *pbbox = s->pHitbox( 0 );
	if ( !pbbox )
		return;

	CMatRenderContextPtr pRenderContext( pMaterialSystem );
	IMaterial *pMaterial = translucent ? pTranslucentMaterial : pSolidMaterial;
	pRenderContext->Bind( pMaterial );

	static unsigned int s_nColorCache = 0;
	IMaterialVar *colorVar = pMaterial->FindVarFast( "$color", &s_nColorCache );

	Vector tmp;
	Vector p[8];

	for (int i = 0; i < s->numhitboxes; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			tmp[0] = (j & 1) ? pbbox[i].bbmin[0] : pbbox[i].bbmax[0];
			tmp[1] = (j & 2) ? pbbox[i].bbmin[1] : pbbox[i].bbmax[1];
			tmp[2] = (j & 4) ? pbbox[i].bbmin[2] : pbbox[i].bbmax[2];

			VectorTransform( tmp, pBoneToWorld[pbbox[i].bone], p[j] );
		}

		int colorIdx = (pbbox[i].group % 8);
		pMaterialSystem->Flush();
		if( colorVar )
		{
			if( translucent )
			{
				colorVar->SetVecValue( 0.2f * hullcolor[colorIdx].x,
									   0.2f * hullcolor[colorIdx].y,
									   0.2f * hullcolor[colorIdx].z );
			}
			else
			{
				colorVar->SetVecValue( hullcolor[colorIdx].x,
									   hullcolor[colorIdx].y,
									   hullcolor[colorIdx].z );
			}
		}

		for (int j = 0; j < 6; j++)
		{
			IMesh* pMesh = pRenderContext->GetDynamicMesh();
			CMeshBuilder meshBuilder;
			meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

			for (int k = 0; k < 4; ++k)
			{
				meshBuilder.Position3fv( p[boxpnt[j][k]].Base() );
				meshBuilder.AdvanceVertex();
			}

			meshBuilder.End();
			pMesh->Draw();
		}
	}
}


//-----------------------------------------------------------------------------
// v44+ Draw bone visualization
//-----------------------------------------------------------------------------
void R_StudioDrawBones_v44( const studiohdr_v44_t *pStudioHdr,
						    const matrix3x4_t *pBoneToWorld,
						    IMaterial *pBoneMaterial,
						    IMaterialSystem *pMaterialSystem )
{
	if (!pStudioHdr)
		return;

	CMatRenderContextPtr pRenderContext( pMaterialSystem );
	pRenderContext->Bind( pBoneMaterial );

	static int boxpnt[6][4] =
	{
		{ 0, 4, 6, 2 }, // +X
		{ 0, 1, 5, 4 }, // +Y
		{ 0, 2, 3, 1 }, // +Z
		{ 7, 5, 1, 3 }, // -X
		{ 7, 3, 2, 6 }, // -Y
		{ 7, 6, 4, 5 }, // -Z
	};

	Vector tmp, p[8], up, right, forward, a1;

	for (int i = 0; i < pStudioHdr->numbones; i++)
	{
		mstudiobone_v44_t *pBone = pStudioHdr->pBone( i );
		if (pBone->parent == -1)
			continue;

		int k = pBone->parent;

		a1[0] = a1[1] = a1[2] = 1.0;
		up[0] = pBoneToWorld[i][0][3] - pBoneToWorld[k][0][3];
		up[1] = pBoneToWorld[i][1][3] - pBoneToWorld[k][1][3];
		up[2] = pBoneToWorld[i][2][3] - pBoneToWorld[k][2][3];

		if (up[0] > up[1])
		{
			if (up[0] > up[2])
				a1[0] = 0.0;
			else
				a1[2] = 0.0;
		}
		else
		{
			if (up[1] > up[2])
				a1[1] = 0.0;
			else
				a1[2] = 0.0;
		}

		CrossProduct( up, a1, right );
		VectorNormalize( right );
		CrossProduct( up, right, forward );
		VectorNormalize( forward );
		VectorScale( right, 2.0, right );
		VectorScale( forward, 2.0, forward );

		for (int j = 0; j < 8; j++)
		{
			p[j][0] = pBoneToWorld[k][0][3];
			p[j][1] = pBoneToWorld[k][1][3];
			p[j][2] = pBoneToWorld[k][2][3];

			if (j & 1)
				VectorSubtract( p[j], right, p[j] );
			else
				VectorAdd( p[j], right, p[j] );

			if (j & 2)
				VectorSubtract( p[j], forward, p[j] );
			else
				VectorAdd( p[j], forward, p[j] );

			if (j & 4)
			{
				// Connect to child bone
				p[j][0] = pBoneToWorld[i][0][3];
				p[j][1] = pBoneToWorld[i][1][3];
				p[j][2] = pBoneToWorld[i][2][3];
			}
		}

		for (int j = 0; j < 6; j++)
		{
			IMesh* pMesh = pRenderContext->GetDynamicMesh();
			CMeshBuilder meshBuilder;
			meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

			for (int m = 0; m < 4; ++m)
			{
				meshBuilder.Position3fv( p[boxpnt[j][m]].Base() );
				meshBuilder.AdvanceVertex();
			}

			meshBuilder.End();
			pMesh->Draw();
		}
	}
}


//-----------------------------------------------------------------------------
// v44+ Setup model for rendering - returns submodel pointer
// This selects which model to render based on bodypart and body value
//-----------------------------------------------------------------------------
mstudiomodel_v44_t* R_StudioSetupModel_v44( const studiohdr_v44_t *pStudioHdr,
										     int bodypart,
										     int body )
{
	if (!pStudioHdr || bodypart < 0 || bodypart >= pStudioHdr->numbodyparts)
		return NULL;

	mstudiobodyparts_v44_t *pbodypart = pStudioHdr->pBodypart( bodypart );
	if (!pbodypart)
		return NULL;

	int index = body / pbodypart->base;
	index = index % pbodypart->nummodels;

	return pbodypart->pModel( index );
}


//-----------------------------------------------------------------------------
// v44+ Compute pose-to-world matrices from bone-to-world
//-----------------------------------------------------------------------------
void ComputePoseToWorld_v44( const studiohdr_v44_t *pStudioHdr,
						     const matrix3x4_t *pBoneToWorld,
						     matrix3x4_t *pPoseToWorld )
{
	if (!pStudioHdr || !pBoneToWorld || !pPoseToWorld)
		return;

	for (int i = 0; i < pStudioHdr->numbones; i++)
	{
		mstudiobone_v44_t *pBone = pStudioHdr->pBone( i );
		ConcatTransforms( pBoneToWorld[i], pBone->poseToBone, pPoseToWorld[i] );
	}
}


//-----------------------------------------------------------------------------
// v44+ Screen align billboard bones
//-----------------------------------------------------------------------------
void ScreenAlignBone_v44( const studiohdr_v44_t *pStudioHdr,
						  int boneIndex,
						  matrix3x4_t *pBoneToWorld,
						  const Vector &viewOrigin )
{
	if (!pStudioHdr || boneIndex < 0 || boneIndex >= pStudioHdr->numbones)
		return;

	mstudiobone_v44_t *pBone = pStudioHdr->pBone( boneIndex );

	int flags = pBone->flags;
	if (!(flags & (BONE_SCREEN_ALIGN_SPHERE_V44 | BONE_SCREEN_ALIGN_CYLINDER_V44)))
		return;

	Vector bonePos;
	bonePos[0] = pBoneToWorld[boneIndex][0][3];
	bonePos[1] = pBoneToWorld[boneIndex][1][3];
	bonePos[2] = pBoneToWorld[boneIndex][2][3];

	Vector forward;
	VectorSubtract( viewOrigin, bonePos, forward );
	VectorNormalize( forward );

	Vector up( 0, 0, 1 );
	Vector right;

	if (flags & BONE_SCREEN_ALIGN_CYLINDER_V44)
	{
		// Keep the up vector as the bone's original up
		up[0] = pBoneToWorld[boneIndex][0][2];
		up[1] = pBoneToWorld[boneIndex][1][2];
		up[2] = pBoneToWorld[boneIndex][2][2];
	}

	CrossProduct( up, forward, right );
	VectorNormalize( right );
	CrossProduct( forward, right, up );
	VectorNormalize( up );

	pBoneToWorld[boneIndex][0][0] = forward[0];
	pBoneToWorld[boneIndex][1][0] = forward[1];
	pBoneToWorld[boneIndex][2][0] = forward[2];

	pBoneToWorld[boneIndex][0][1] = right[0];
	pBoneToWorld[boneIndex][1][1] = right[1];
	pBoneToWorld[boneIndex][2][1] = right[2];

	pBoneToWorld[boneIndex][0][2] = up[0];
	pBoneToWorld[boneIndex][1][2] = up[1];
	pBoneToWorld[boneIndex][2][2] = up[2];
}


//-----------------------------------------------------------------------------
// v44+ Get mesh vertex data
//-----------------------------------------------------------------------------
const mstudio_meshvertexdata_v44_t* GetVertexData_v44( const mstudiomesh_v44_t *pMesh )
{
	if (!pMesh)
		return NULL;

	return pMesh->GetVertexData();
}


//-----------------------------------------------------------------------------
// v44+ Transform position through bone weights
//-----------------------------------------------------------------------------
void R_StudioTransform_v44( const Vector& in1,
						    const mstudioboneweight_v44_t *pboneweight,
						    const matrix3x4_t *pPoseToWorld,
						    Vector& out1 )
{
	Vector out2;
	switch( pboneweight->numbones )
	{
	case 1:
		VectorTransform( in1, pPoseToWorld[pboneweight->bone[0]], out1 );
		break;
	default:
		VectorFill( out1, 0 );
		for (int i = 0; i < pboneweight->numbones; i++)
		{
			VectorTransform( in1, pPoseToWorld[pboneweight->bone[i]], out2 );
			VectorMA( out1, pboneweight->weight[i], out2, out1 );
		}
		break;
	}
}


//-----------------------------------------------------------------------------
// v44+ Rotate normal through bone weights
//-----------------------------------------------------------------------------
void R_StudioRotate_v44( const Vector& in1,
					     const mstudioboneweight_v44_t *pboneweight,
					     const matrix3x4_t *pPoseToWorld,
					     Vector& out1 )
{
	if (pboneweight->numbones == 1)
	{
		VectorRotate( in1, pPoseToWorld[pboneweight->bone[0]], out1 );
		VectorNormalize( out1 );
	}
	else
	{
		Vector out2;
		VectorFill( out1, 0 );
		for (int i = 0; i < pboneweight->numbones; i++)
		{
			VectorRotate( in1, pPoseToWorld[pboneweight->bone[i]], out2 );
			VectorMA( out1, pboneweight->weight[i], out2, out1 );
		}
		VectorNormalize( out1 );
	}
}


//-----------------------------------------------------------------------------
// v44+ Rotate Vector4D (tangent) through bone weights
//-----------------------------------------------------------------------------
void R_StudioRotate4D_v44( const Vector4D& in1,
						   const mstudioboneweight_v44_t *pboneweight,
						   const matrix3x4_t *pPoseToWorld,
						   Vector4D& out1 )
{
	Vector in3( in1.x, in1.y, in1.z );
	Vector out3;

	if (pboneweight->numbones == 1)
	{
		VectorRotate( in3, pPoseToWorld[pboneweight->bone[0]], out3 );
		VectorNormalize( out3 );
	}
	else
	{
		Vector out2;
		VectorFill( out3, 0 );
		for (int i = 0; i < pboneweight->numbones; i++)
		{
			VectorRotate( in3, pPoseToWorld[pboneweight->bone[i]], out2 );
			VectorMA( out3, pboneweight->weight[i], out2, out3 );
		}
		VectorNormalize( out3 );
	}

	out1.Init( out3.x, out3.y, out3.z, in1.w );
}


//-----------------------------------------------------------------------------
// v44+ Get number of vertices for mesh at given LOD
//-----------------------------------------------------------------------------
int GetNumLODVertices_v44( const mstudiomesh_v44_t *pMesh, int lod )
{
	if (!pMesh)
		return 0;

	const mstudio_meshvertexdata_v44_t *pVertData = pMesh->GetVertexData();
	if (!pVertData)
		return 0;

	if (lod < 0 || lod >= MAX_NUM_LODS_V44)
		return pMesh->numvertices;

	return pVertData->numLODVertexes[lod];
}


//-----------------------------------------------------------------------------
// v44+ Check if model is v44+
//-----------------------------------------------------------------------------
bool IsModelV44( const studiohdr_v44_t *pStudioHdr )
{
	if (!pStudioHdr)
		return false;

	return pStudioHdr->version >= STUDIO_VERSION_V44;
}


//-----------------------------------------------------------------------------
// v44+ Get skin reference table
//-----------------------------------------------------------------------------
short* GetSkinRef_v44( const studiohdr_v44_t *pStudioHdr, int skin )
{
	if (!pStudioHdr)
		return NULL;

	if (skin < 0 || skin >= pStudioHdr->numskinfamilies)
		skin = 0;

	return pStudioHdr->pSkinref( skin * pStudioHdr->numskinref );
}


//-----------------------------------------------------------------------------
// v44+ Get number of skin families
//-----------------------------------------------------------------------------
int GetNumSkinFamilies_v44( const studiohdr_v44_t *pStudioHdr )
{
	return pStudioHdr ? pStudioHdr->numskinfamilies : 0;
}


//-----------------------------------------------------------------------------
// v44+ Get number of bodyparts
//-----------------------------------------------------------------------------
int GetNumBodyparts_v44( const studiohdr_v44_t *pStudioHdr )
{
	return pStudioHdr ? pStudioHdr->numbodyparts : 0;
}


//-----------------------------------------------------------------------------
// v44+ Get bodypart
//-----------------------------------------------------------------------------
mstudiobodyparts_v44_t* GetBodypart_v44( const studiohdr_v44_t *pStudioHdr, int index )
{
	if (!pStudioHdr || index < 0 || index >= pStudioHdr->numbodyparts)
		return NULL;

	return pStudioHdr->pBodypart( index );
}


//-----------------------------------------------------------------------------
// v44+ Get number of bones
//-----------------------------------------------------------------------------
int GetNumBones_v44( const studiohdr_v44_t *pStudioHdr )
{
	return pStudioHdr ? pStudioHdr->numbones : 0;
}


//-----------------------------------------------------------------------------
// v44+ Get bone
//-----------------------------------------------------------------------------
mstudiobone_v44_t* GetBone_v44( const studiohdr_v44_t *pStudioHdr, int index )
{
	if (!pStudioHdr || index < 0 || index >= pStudioHdr->numbones)
		return NULL;

	return pStudioHdr->pBone( index );
}
