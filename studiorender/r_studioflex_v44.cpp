//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: v44+ flex/eyeball rendering - COMPLETELY ISOLATED from v37
//
// This file contains ONLY v44+ code. All types use _v44 suffix.
// NO v37 types are used anywhere in this file.
//
//===========================================================================//

// NO studio.h - v44+ code is completely isolated from v37
#include "studiohdr_v44.h"  // All v44+ types defined here

#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imesh.h"
#include "mathlib.h"
#include "pixelwriter.h"
#include "vtf/vtf.h"
#include "tier1/convar.h"
#include "tier1/KeyValues.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define sign( a ) (((a) < 0) ? -1 : (((a) > 0) ? 1 : 0 ))

//-----------------------------------------------------------------------------
// v44+ Eyeball position calculation
//-----------------------------------------------------------------------------
void R_StudioEyeballPosition_v44( const mstudioeyeball_v44_t *peyeball,
								  eyeballstate_v44_t *pstate,
								  const matrix3x4_t *pBoneToWorld,
								  const Vector &viewTarget,
								  const StudioRenderConfig_t &config )
{
	pstate->peyeball = peyeball;

	Vector tmp;
	// move eyeball into worldspace
	VectorCopy( peyeball->org, tmp );

	tmp[0] += config.fEyeShiftX * sign( tmp[0] );
	tmp[1] += config.fEyeShiftY * sign( tmp[1] );
	tmp[2] += config.fEyeShiftZ * sign( tmp[2] );

	VectorTransform( tmp, pBoneToWorld[peyeball->bone], pstate->org );
	VectorRotate( peyeball->up, pBoneToWorld[peyeball->bone], pstate->up );

	// look directly at target
	VectorSubtract( viewTarget, pstate->org, pstate->forward );
	VectorNormalize( pstate->forward );

	if ( !config.bEyeMove )
	{
		VectorRotate( peyeball->forward, pBoneToWorld[peyeball->bone], pstate->forward );
		VectorScale( pstate->forward, -1, pstate->forward );
	}

	CrossProduct( pstate->forward, pstate->up, pstate->right );
	VectorNormalize( pstate->right );

	// shift N degrees off of the target
	float dz = peyeball->zoffset;

	VectorMA( pstate->forward, peyeball->zoffset + dz, pstate->right, pstate->forward );

	VectorNormalize( pstate->forward );
	// re-aim eyes
	CrossProduct( pstate->forward, pstate->up, pstate->right );
	VectorNormalize( pstate->right );

	CrossProduct( pstate->right, pstate->forward, pstate->up );
	VectorNormalize( pstate->up );

	float scale = (1.0 / peyeball->iris_scale) + config.fEyeSize;

	if (scale > 0)
		scale = 1.0 / scale;

	VectorScale( &pstate->right[0], -scale, pstate->mat[0] );
	VectorScale( &pstate->up[0], -scale, pstate->mat[1] );

	pstate->mat[0][3] = -DotProduct( &pstate->org[0], pstate->mat[0] ) + 0.5f;
	pstate->mat[1][3] = -DotProduct( &pstate->org[0], pstate->mat[1] ) + 0.5f;
}


//-----------------------------------------------------------------------------
// v44+ FACS eyelid calculation
//-----------------------------------------------------------------------------
void R_StudioEyelidFACS_v44( const mstudioeyeball_v44_t *peyeball,
							 const eyeballstate_v44_t *pstate,
							 const matrix3x4_t *pBoneToWorld,
							 float *pFlexWeights )
{
	if ( peyeball->m_bNonFACS )
		return;

	Vector headup;
	Vector headforward;
	Vector pos;

	float upperlid = DEG2RAD( 9.5 );
	float lowerlid = DEG2RAD( -26.4 );

	// FIXME: Crash workaround
	Vector vecNormTarget;
	vecNormTarget.Init( peyeball->uppertarget[0], peyeball->uppertarget[1], peyeball->uppertarget[2] );
	vecNormTarget /= peyeball->radius;
	vecNormTarget.x = clamp( vecNormTarget.x, -1.0f, 1.0f );
	vecNormTarget.y = clamp( vecNormTarget.y, -1.0f, 1.0f );
	vecNormTarget.z = clamp( vecNormTarget.z, -1.0f, 1.0f );

	// get weighted position of eyeball angles based on the "raiser", "neutral", and "lowerer" controls
	upperlid = pFlexWeights[peyeball->upperflexdesc[0]] * asin( vecNormTarget.x );
	upperlid += pFlexWeights[peyeball->upperflexdesc[1]] * asin( vecNormTarget.y );
	upperlid += pFlexWeights[peyeball->upperflexdesc[2]] * asin( vecNormTarget.z );

	vecNormTarget.Init( peyeball->lowertarget[0], peyeball->lowertarget[1], peyeball->lowertarget[2] );
	vecNormTarget /= peyeball->radius;
	vecNormTarget.x = clamp( vecNormTarget.x, -1.0f, 1.0f );
	vecNormTarget.y = clamp( vecNormTarget.y, -1.0f, 1.0f );
	vecNormTarget.z = clamp( vecNormTarget.z, -1.0f, 1.0f );

	lowerlid = pFlexWeights[peyeball->lowerflexdesc[0]] * asin( vecNormTarget.x );
	lowerlid += pFlexWeights[peyeball->lowerflexdesc[1]] * asin( vecNormTarget.y );
	lowerlid += pFlexWeights[peyeball->lowerflexdesc[2]] * asin( vecNormTarget.z );

	float sinupper, cosupper, sinlower, coslower;
	SinCos( upperlid, &sinupper, &cosupper );
	SinCos( lowerlid, &sinlower, &coslower );

	// convert to head relative space
	VectorIRotate( pstate->up, pBoneToWorld[peyeball->bone], headup );
	VectorIRotate( pstate->forward, pBoneToWorld[peyeball->bone], headforward );

	// upper lid
	VectorScale( headup, sinupper * peyeball->radius, pos );
	VectorMA( pos, cosupper * peyeball->radius, headforward, pos );
	pFlexWeights[peyeball->upperlidflexdesc] = DotProduct( pos, peyeball->up );

	// lower lid
	VectorScale( headup, sinlower * peyeball->radius, pos );
	VectorMA( pos, coslower * peyeball->radius, headforward, pos );
	pFlexWeights[peyeball->lowerlidflexdesc] = DotProduct( pos, peyeball->up );
}


//-----------------------------------------------------------------------------
// v44+ Planar projection for materials
//-----------------------------------------------------------------------------
void MaterialPlanerProjection_v44( const matrix3x4_t& mat, int count,
								   const Vector *psrcverts, Vector2D *pdesttexcoords )
{
	for (int i = 0; i < count; i++)
	{
		pdesttexcoords[i][0] = DotProduct( &psrcverts[i].x, mat[0] ) + mat[0][3];
		pdesttexcoords[i][1] = DotProduct( &psrcverts[i].x, mat[1] ) + mat[1][3];
	}
}


//-----------------------------------------------------------------------------
// v44+ Ramp and clamp flex weight
//-----------------------------------------------------------------------------
float RampFlexWeight_v44( const mstudioflex_v44_t &flex, float w )
{
	if (w <= flex.target0 || w >= flex.target3)
	{
		// value outside of range
		w = 0.0;
	}
	else if (w < flex.target1)
	{
		// 0 to 1 ramp
		w = (w - flex.target0) / (flex.target1 - flex.target0);
	}
	else if (w > flex.target2)
	{
		// 1 to 0 ramp
		w = (flex.target3 - w) / (flex.target3 - flex.target2);
	}
	else
	{
		// plat
		w = 1.0;
	}
	return w;
}


//-----------------------------------------------------------------------------
// v44+ Setup flex verts
// Note: This is a standalone function, not a CStudioRender member
//-----------------------------------------------------------------------------
void R_StudioFlexVerts_v44( const studiohdr_v44_t *pStudioHdr,
						    mstudiomesh_v44_t *pmesh,
						    int lod,
						    float *pFlexWeights,
						    float *pFlexDelayedWeights,
						    CUtlVector<CachedPosNormTan_v44_t> &flexedVerts,
						    CUtlVector<int> &flexedIndices )
{
	VPROF_BUDGET( "R_StudioFlexVerts_v44", VPROF_BUDGETGROUP_MODEL_RENDERING );

	if (!pmesh || !pStudioHdr)
		return;

	// get pointers to geometry
	const mstudio_meshvertexdata_v44_t *vertData = pmesh->GetVertexData();
	if (!vertData || !vertData->pModelVertexData)
	{
		return;
	}

	// The flex data should have been converted to the new (fixed-point) format on load
	if ( (pStudioHdr->flags & STUDIOHDR_FLAGS_FLEXES_CONVERTED_V44) == 0 )
	{
		static unsigned int flexConversionTimesWarned = 0;
		if ( flexConversionTimesWarned++ < 6 )
			Warning( "ERROR: v44+ flex verts have not been converted - expect rendering issues" );
	}

	mstudiovertex_v44_t *pVertices = vertData->Vertex( 0 );
	Vector4D *pStudioTangentS = vertData->HasTangentData() ? vertData->TangentS( 0 ) : NULL;

	// Clear previous flexed data
	flexedVerts.RemoveAll();
	flexedIndices.RemoveAll();

	// Track which vertices have been flexed
	CUtlVector<int> vertexFlexIndex;
	int numVertices = pmesh->numvertices;
	vertexFlexIndex.SetCount( numVertices );
	for (int v = 0; v < numVertices; v++)
		vertexFlexIndex[v] = -1;

	// apply flex weights
	for (int i = 0; i < pmesh->numflexes; i++)
	{
		mstudioflex_v44_t *pflex = pmesh->pFlex( i );

		float w1 = RampFlexWeight_v44( *pflex, pFlexWeights[ pflex->flexdesc ] );
		float w2 = RampFlexWeight_v44( *pflex, pFlexDelayedWeights[ pflex->flexdesc ] );

		float w3, w4;
		if ( pflex->flexpair != 0)
		{
			w3 = RampFlexWeight_v44( *pflex, pFlexWeights[ pflex->flexpair ] );
			w4 = RampFlexWeight_v44( *pflex, pFlexDelayedWeights[ pflex->flexpair ] );
		}
		else
		{
			w3 = w1;
			w4 = w2;
		}

		if ( w1 > -0.001 && w1 < 0.001 && w2 > -0.001 && w2 < 0.001 )
		{
			if ( w3 > -0.001 && w3 < 0.001 && w4 > -0.001 && w4 < 0.001 )
			{
				continue;
			}
		}

		byte *pvanim = pflex->pBaseVertanim();
		int nVAnimSizeBytes = pflex->VertAnimSizeBytes();

		for (int j = 0; j < pflex->numverts; j++)
		{
			mstudiovertanim_v44_t *pAnim = (mstudiovertanim_v44_t*)( pvanim + j * nVAnimSizeBytes );
			int n = pAnim->index;

			// Only flex the indices that are part of this mesh at this LOD
			if (n < vertData->numLODVertexes[lod])
			{
				mstudiovertex_v44_t &vert = pVertices[n];

				CachedPosNormTan_v44_t* pFlexedVertex;
				int flexIndex = vertexFlexIndex[n];

				if (flexIndex == -1)
				{
					// Add a new flexed vert
					flexIndex = flexedVerts.AddToTail();
					vertexFlexIndex[n] = flexIndex;
					flexedIndices.AddToTail( n );

					pFlexedVertex = &flexedVerts[flexIndex];
					VectorCopy( vert.m_vecPosition, pFlexedVertex->m_Position );
					VectorCopy( vert.m_vecNormal, pFlexedVertex->m_Normal );

					if (pStudioTangentS)
					{
						Vector4DCopy( pStudioTangentS[n], pFlexedVertex->m_TangentS );
					}
				}
				else
				{
					pFlexedVertex = &flexedVerts[flexIndex];
				}

				float s = pAnim->speed * (1.0F/255.0F);
				float b = pAnim->side * (1.0F/255.0F);

				float w = (w1 * s + (1.0f - s) * w2) * (1.0f - b) + b * (w3 * s + (1.0f - s) * w4);

				// Accumulate weighted deltas
				pFlexedVertex->m_Position += pAnim->GetDeltaFixed() * w;
				pFlexedVertex->m_Normal += pAnim->GetNDeltaFixed() * w;

				if ( pStudioTangentS )
				{
					pFlexedVertex->m_TangentS.AsVector3D() += pAnim->GetNDeltaFixed() * w;
				}
			}
		}
	}

	// Renormalize the flexed normals and tangents
	for (int i = 0; i < flexedVerts.Count(); i++)
	{
		VectorNormalize( flexedVerts[i].m_Normal );
		if (pStudioTangentS)
		{
			float w = flexedVerts[i].m_TangentS.w;
			VectorNormalize( flexedVerts[i].m_TangentS.AsVector3D() );
			flexedVerts[i].m_TangentS.w = w;
		}
	}
}


//-----------------------------------------------------------------------------
// v44+ Glint gaussian coefficient
//-----------------------------------------------------------------------------
#define KERNEL_DIAMETER_V44 2
#define KERNEL_TEXELS_V44 (KERNEL_DIAMETER_V44)
#define KERNEL_TEXEL_RADIUS_V44 (KERNEL_TEXELS_V44 / 2)

inline float GlintGaussSpotCoefficient_v44( float dx, float dy )
{
	const float radius = KERNEL_DIAMETER_V44 / 2;
	const float rsq = 1.0f / (radius * radius);
	float r2 = (dx * dx + dy * dy) * rsq;
	if (r2 <= 1.0f)
	{
		return exp( -25.0 * r2 );
	}
	return 0;
}


//-----------------------------------------------------------------------------
// v44+ Add glint to texture
//-----------------------------------------------------------------------------
void AddGlint_v44( CPixelWriter &pixelWriter, float x, float y,
				   const Vector& color, int glintWidth, int glintHeight )
{
	x = (x + 0.5f) * glintWidth;
	y = (y + 0.5f) * glintHeight;
	const float texelRadius = KERNEL_DIAMETER_V44 / 2;

	int x0 = (int)x;
	int y0 = (int)y;
	int x1 = x0 + texelRadius;
	int y1 = y0 + texelRadius;
	x0 -= texelRadius;
	y0 -= texelRadius;

	// clip light to texture
	if ( (x0 >= glintWidth) || (x1 < 0) || (y0 >= glintHeight) || (y1 < 0) )
		return;

	// clamp coordinates
	if ( x0 < 0 ) x0 = 0;
	if ( y0 < 0 ) y0 = 0;
	if ( x1 >= glintWidth ) x1 = glintWidth - 1;
	if ( y1 >= glintHeight ) y1 = glintHeight - 1;

	for (int v = y0; v <= y1; ++v )
	{
		pixelWriter.Seek( x0, v );

		for (int u = x0; u <= x1; ++u )
		{
			float fu = ((float)u) - x;
			float fv = ((float)v) - y;
			const float offset = 0.25;
			float intensity = GlintGaussSpotCoefficient_v44( fu-offset, fv-offset ) +
							  GlintGaussSpotCoefficient_v44( fu+offset, fv-offset ) +
							  5 * GlintGaussSpotCoefficient_v44( fu, fv ) +
							  GlintGaussSpotCoefficient_v44( fu-offset, fv+offset ) +
							  GlintGaussSpotCoefficient_v44( fu+offset, fv+offset );

			// NOTE: Old filter code multiplies the signal by 8X, so we will too
			intensity *= (4.0f/9.0f);

			Vector outColor = intensity * color;
			int r, g, b, a;
			pixelWriter.ReadPixelNoAdvance( r, g, b, a );
			outColor.x += TextureToLinear(r);
			outColor.y += TextureToLinear(g);
			outColor.z += TextureToLinear(b);
			pixelWriter.WritePixel( LinearToTexture(outColor.x), LinearToTexture(outColor.y), LinearToTexture(outColor.z) );
		}
	}
}


//-----------------------------------------------------------------------------
// v44+ Glint render data structure
//-----------------------------------------------------------------------------
struct GlintRenderData_v44_t
{
	Vector2D	m_vecPosition;
	Vector		m_vecIntensity;
};


//-----------------------------------------------------------------------------
// v44+ Build glint render data
//-----------------------------------------------------------------------------
int BuildGlintRenderData_v44( GlintRenderData_v44_t *pData, int nMaxGlints,
							  const eyeballstate_v44_t *pState,
							  const Vector& vright, const Vector& vup,
							  const Vector& r_origin )
{
	Vector viewdelta;
	VectorSubtract( r_origin, pState->org, viewdelta );
	VectorNormalize( viewdelta );

	// hack cornea position
	float iris_radius = pState->peyeball->radius * (6.0 / 12.0);
	float cornea_radius = pState->peyeball->radius * (8.0 / 12.0);

	Vector cornea;
	// position on eyeball that matches iris radius
	float er = ( iris_radius / pState->peyeball->radius );
	er = FastSqrt( 1 - er * er );

	// position on cornea sphere that matches iris radius
	float cr = ( iris_radius / cornea_radius );
	cr = FastSqrt( 1 - cr * cr );

	float r = ( er * pState->peyeball->radius - cr * cornea_radius );
	VectorScale( pState->forward, r, cornea );

	// get offset for center of cornea
	float dx, dy;
	dx = DotProduct( vright, cornea );
	dy = DotProduct( vup, cornea );

	// move cornea to world space
	VectorAdd( cornea, pState->org, cornea );

	Vector delta, intensity;
	Vector reflection, coord;

	// TODO: Implement R_LightGlintPosition_v44 for light iteration
	int nGlintCount = 0;

	return nGlintCount;
}


//-----------------------------------------------------------------------------
// v44+ Compute glint texture projection
//-----------------------------------------------------------------------------
void ComputeGlintTextureProjection_v44( const eyeballstate_v44_t *pState,
										const Vector& vright, const Vector& vup,
										matrix3x4_t& mat )
{
	// project eyeball into screenspace texture
	float scale = 1.0 / (pState->peyeball->radius * 2);
	VectorScale( &vright.x, scale, mat[0] );
	VectorScale( &vup.x, scale, mat[1] );

	mat[0][3] = -DotProduct( pState->org.Base(), mat[0] ) + 0.5;
	mat[1][3] = -DotProduct( pState->org.Base(), mat[1] ) + 0.5;
}


//-----------------------------------------------------------------------------
// v44+ Mouth lighting computation
//-----------------------------------------------------------------------------
void R_MouthComputeLightingValues_v44( const studiohdr_v44_t *pStudioHdr,
									   const matrix3x4_t *pBoneToWorld,
									   float *pFlexWeights,
									   float& fIllum, Vector& forward )
{
	if (!pStudioHdr || pStudioHdr->nummouths < 1)
	{
		fIllum = 0;
		forward.Init(1, 0, 0);
		return;
	}

	mstudiomouth_v44_t *pMouth = pStudioHdr->pMouth( 0 );

	fIllum = pFlexWeights[pMouth->flexdesc];
	if (fIllum < 0) fIllum = 0;
	if (fIllum > 1) fIllum = 1;
	fIllum = LinearToTexture( fIllum ) / 255.0;

	VectorRotate( pMouth->forward, pBoneToWorld[ pMouth->bone ], forward );
}


//-----------------------------------------------------------------------------
// v44+ Mouth lighting application
//-----------------------------------------------------------------------------
void R_MouthLighting_v44( float fIllum, const Vector& normal,
						  const Vector& forward, Vector &light )
{
	float dot = -DotProduct( normal, forward );
	if (dot > 0)
	{
		VectorScale( light, dot * fIllum, light );
	}
	else
	{
		VectorFill( light, 0 );
	}
}


//-----------------------------------------------------------------------------
// v44+ Mouth setup for vertex shader
//-----------------------------------------------------------------------------
void R_MouthSetupVertexShader_v44( const studiohdr_v44_t *pStudioHdr,
								   const matrix3x4_t *pBoneToWorld,
								   float *pFlexWeights,
								   IMaterial* pMaterial )
{
	if (!pMaterial || !pStudioHdr || pStudioHdr->nummouths < 1)
		return;

	mstudiomouth_v44_t *pMouth = pStudioHdr->pMouth( 0 );

	// Don't deal with illum gamma, we apply it at a different point
	// for vertex shaders
	float fIllum = pFlexWeights[pMouth->flexdesc];
	if (fIllum < 0) fIllum = 0;
	if (fIllum > 1) fIllum = 1;

	Vector forward;
	VectorRotate( pMouth->forward, pBoneToWorld[ pMouth->bone ], forward );
	forward *= -1;

	static unsigned int illumVarCache_v44 = 0;
	static unsigned int forwardVarCache_v44 = 0;

	IMaterialVar* pIllumVar = pMaterial->FindVarFast( "$illumfactor", &illumVarCache_v44 );
	if (pIllumVar)
	{
		pIllumVar->SetFloatValue( fIllum );
	}

	IMaterialVar* pForwardVar = pMaterial->FindVarFast( "$forward", &forwardVarCache_v44 );
	if (pForwardVar)
	{
		pForwardVar->SetVecValue( forward.Base(), 3 );
	}
}


//-----------------------------------------------------------------------------
// v44+ Eyeball normal calculation
//-----------------------------------------------------------------------------
void R_StudioEyeballNormal_v44( const mstudioeyeball_v44_t *peyeball,
								const Vector& org, const Vector& pos,
								Vector& normal )
{
	// inside of a flattened torus
	VectorSubtract( pos, org, normal );
	float flUpAmount = DotProduct( normal, peyeball->up );
	VectorMA( normal, -0.5 * flUpAmount, peyeball->up, normal );
	VectorNormalize( normal );
}
