
#include "tier0/dbg.h"
#include "mathlib.h"
#include "bone_setup.h"
#include <string.h>

#include "collisionutils.h"
#include "vstdlib/random.h"
#include "tier0/vprof.h"

// Include shared model loader for v37 animation groups
#include "engine/ISharedModelLoader.h"

// Version-aware bone accessors for v44+ MDL support
#include "studiohdr_v44.h"
#include "studio_helpers.h"  // Helper functions for studiohdr_t access

// studiohdr_t::pBone_v48 still uses studiohdr_t::boneindex. That offset is
// only valid for the legacy header, so v44+ models must reach the bone array
// through studiohdr_v44_t before using the otherwise compatible v48 fields.
static inline mstudiobone_v48_t *StudioBone_GetV48( const studiohdr_t *pStudioHdr, int iBone )
{
	if ( StudioHdr_IsV44Plus( pStudioHdr ) )
	{
		return reinterpret_cast<mstudiobone_v48_t *>(
			StudioHdr_AsV44( pStudioHdr )->pBone( iBone ) );
	}

	return pStudioHdr->pBone_v48( iBone );
}

void BuildBoneChain(
	const studiohdr_t *pStudioHdr,
	matrix3x4_t &rootxform,
	Vector pos[],
	Quaternion q[],
	int	iBone,
	matrix3x4_t *pBoneToWorld );

//-----------------------------------------------------------------------------
// GetAnimDescriptions - Version-aware animation descriptor retrieval
//
// v37: Uses animation groups and sequence groups for demand loading
// v48: Uses animation blocks for demand loading (simpler system)
//-----------------------------------------------------------------------------
mstudioanimdesc_t *GetAnimDescriptions( const studiohdr_t *pStudioHdr, mstudioseqdesc_t *pseqdesc, int x, int y )
{
	// Safety check: verify we have animations
	// CRITICAL: Use version-aware accessor - numanim is v37 field, v44+ uses numlocalanim
	int numAnims = pStudioHdr->GetNumLocalAnim();
	if ( numAnims <= 0 )
	{
		Warning( "GetAnimDescriptions: Model %s (v%d) has no animations!\n", pStudioHdr->name, pStudioHdr->version );
		return NULL;
	}

	//-----------------------------------------------------------------------------
	// v37 path: Animation groups and sequence groups
	//-----------------------------------------------------------------------------
	if (pStudioHdr->version <= STUDIO_VERSION_37)
	{
		// v37 models with no animation groups use direct indexing
		if ( pStudioHdr->animdata.v37.numanimgroups <= 0 )
		{
			// No animation groups - use direct animation index from sequence
			int iAnim = pseqdesc->anim( x, y );
			if ( iAnim < 0 || iAnim >= pStudioHdr->numanim )
				iAnim = 0;
			return pStudioHdr->pAnimdesc( iAnim );
		}

		int iAnimGroup = pseqdesc->GetAnimIndex( x, y, pStudioHdr->version );
		if ( iAnimGroup < 0 || iAnimGroup >= pStudioHdr->animdata.v37.numanimgroups )
			return pStudioHdr->pAnimdesc( 0 );

		mstudioanimgroup_t *pAnimGroup = pStudioHdr->pAnimgroup( iAnimGroup );
		if ( pAnimGroup == NULL )
			return pStudioHdr->pAnimdesc( 0 );

		int iSeqGroup = pAnimGroup->group;
		if ( iSeqGroup < 0 || iSeqGroup >= pStudioHdr->numseqgroups )
			return pStudioHdr->pAnimdesc( 0 );

		int iAnimIndex = pAnimGroup->index;

		// Validate iAnimIndex is in range
		if ( iAnimIndex < 0 || iAnimIndex >= pStudioHdr->numanim )
			iAnimIndex = 0;

		if ( iSeqGroup == 0 )
			return pStudioHdr->pAnimdesc( iAnimIndex );

		mstudioseqgroup_t *pSeqGroup = pStudioHdr->pSeqgroup( iSeqGroup );
		if ( pSeqGroup == NULL )
			return pStudioHdr->pAnimdesc( 0 );

		// VXP: Only Male_01.mdl has "shared_animation" label
		if ( pSeqGroup->szlabelindex != pSeqGroup->sznameindex )
		{
			if ( Q_strncmp( pSeqGroup->pszLabel(), "shared_animation", 17 ) != 0 )
			{
				return pStudioHdr->pAnimdesc( 0 );
			}
		}

		studioanimgrouphdr_t *pAnimGroupShared = (studioanimgrouphdr_t *)sharedmodelloader->LoadSharedModel( pSeqGroup->pszName() );
		if ( pAnimGroupShared == NULL )
			return pStudioHdr->pAnimdesc( 0 );

		return pAnimGroupShared->pAnimdesc( iAnimIndex );
	}
	//-----------------------------------------------------------------------------
	// v44+ path: Direct animation index (animation blocks handled separately)
	// v48 models use simpler indexing - animation blocks are demand-loaded
	//-----------------------------------------------------------------------------
	else
	{
		// For v48 models, use the v48 anim() accessor function to get correct field offsets
		const mstudioseqdesc_v48_t *pseqdesc_v48 = (const mstudioseqdesc_v48_t *)pseqdesc;
		int iAnim = pseqdesc_v48->anim( x, y );

		// CRITICAL: Use version-aware accessor for animation count!
		// pStudioHdr->numanim is the v37 field - for v44+ we need numlocalanim
		int numAnims = pStudioHdr->GetNumLocalAnim();
		if (iAnim < 0 || iAnim >= numAnims)
			return pStudioHdr->pAnimdesc( 0 );

		return pStudioHdr->pAnimdesc( iAnim );
	}
}

//-----------------------------------------------------------------------------
// Purpose: return a sub frame rotation for a single bone
//-----------------------------------------------------------------------------
void CalcBoneQuaternion( const studiohdr_t *pStudioHdr, int frame, float s, 
						const mstudiobone_t *pbone, const mstudioanim_t *panim, Quaternion &q )
{

	int					j, k;
	Quaternion			q1, q2;
	RadianEuler			angle1(0,0,0), angle2(0,0,0);
	mstudioanimvalue_t	*panimvalue;

	if (!(panim->flags & STUDIO_ROT_ANIMATED))
	{
		q.Init( panim->u.pose.q[0], panim->u.pose.q[1], panim->u.pose.q[2], panim->u.pose.q[3] );
		return;
	}

	for (j = 0; j < 3; j++)
	{
		if (panim->u.offset[j+3] == 0)
		{
			angle2[j] = angle1[j] = pbone->value[j+3]; // default;
		}
		else
		{
			panimvalue = panim->pAnimvalue( j+3 );
			k = frame;
			// DEBUG
			if (panimvalue->num.total < panimvalue->num.valid)
				k = 0;
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
				// DEBUG
				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;
			}
			// Bah, missing blend!
			if (panimvalue->num.valid > k)
			{
				angle1[j] = panimvalue[k+1].value;

				if (panimvalue->num.valid > k + 1)
				{
					angle2[j] = panimvalue[k+2].value;
				}
				else
				{
					if (panimvalue->num.total > k + 1)
						angle2[j] = angle1[j];
					else
						angle2[j] = panimvalue[panimvalue->num.valid+2].value;
				}
			}
			else
			{
				angle1[j] = panimvalue[panimvalue->num.valid].value;
				if (panimvalue->num.total > k + 1)
				{
					angle2[j] = angle1[j];
				}
				else
				{
					angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}
			angle1[j] = pbone->value[j+3] + angle1[j] * pbone->scale[j+3];
			angle2[j] = pbone->value[j+3] + angle2[j] * pbone->scale[j+3];
		}
	}

	Assert( angle1.IsValid() && angle2.IsValid() );
	if (angle1.x != angle2.x || angle1.y != angle2.y || angle1.z != angle2.z)
	{
		AngleQuaternion( angle1, q1 );
		AngleQuaternion( angle2, q2 );
		QuaternionBlend( q1, q2, s, q );
	}
	else
	{
		AngleQuaternion( angle1, q );
	}

	// align to unified bone
	if (!(panim->flags & STUDIO_DELTA) && (pbone->flags & BONE_FIXED_ALIGNMENT))
	{
		QuaternionAlign( pbone->qAlignment, q, q );
	}
}


//-----------------------------------------------------------------------------
// v48: Extract rotation from compressed animation data
// Supports Quaternion48, Quaternion64, and animated (RLE) rotation values
//-----------------------------------------------------------------------------
void CalcBoneQuaternion_v48( const studiohdr_t *pStudioHdr, int frame, float s,
						const mstudiobone_v48_t *pbone, const mstudioanim_v48_t *panim, Quaternion &q )
{
	if (panim->flags & STUDIO_ANIM_RAWROT)
	{
		// Compressed quaternion (48-bit)
		q = *(panim->pQuat48());
		Assert( q.IsValid() );
	}
	else if (panim->flags & STUDIO_ANIM_RAWROT2)
	{
		// High-precision compressed quaternion (64-bit)
		q = *(panim->pQuat64());
		Assert( q.IsValid() );
	}
	else if (panim->flags & STUDIO_ANIM_ANIMROT)
	{
		// Animated rotation - RLE compressed values
		int j, k;
		Quaternion q1, q2;
		RadianEuler angle1(0, 0, 0), angle2(0, 0, 0);

		mstudioanim_valueptr_t *pRotV = panim->pRotV();
		for (j = 0; j < 3; j++)
		{
			if (pRotV->offset[j] == 0)
			{
				// No animation data for this axis - use bone default
				angle1[j] = angle2[j] = pbone->rot[j];
			}
			else
			{
				mstudioanimvalue_t *panimvalue = pRotV->pAnimvalue(j);
				k = frame;

				// Safety check
				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;

				// Find the span containing our frame
				while (panimvalue->num.total <= k)
				{
					k -= panimvalue->num.total;
					panimvalue += panimvalue->num.valid + 1;
					if (panimvalue->num.total < panimvalue->num.valid)
						k = 0;
				}

				// Extract animation values for interpolation
				if (panimvalue->num.valid > k)
				{
					angle1[j] = panimvalue[k + 1].value;
					if (panimvalue->num.valid > k + 1)
						angle2[j] = panimvalue[k + 2].value;
					else if (panimvalue->num.total > k + 1)
						angle2[j] = angle1[j];
					else
						angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
				else
				{
					angle1[j] = panimvalue[panimvalue->num.valid].value;
					if (panimvalue->num.total > k + 1)
						angle2[j] = angle1[j];
					else
						angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}

				// Apply bone default + delta * scale
				angle1[j] = pbone->rot[j] + angle1[j] * pbone->rotscale[j];
				angle2[j] = pbone->rot[j] + angle2[j] * pbone->rotscale[j];
			}
		}

		Assert( angle1.IsValid() && angle2.IsValid() );

		// Interpolate if angles differ
		if (angle1.x != angle2.x || angle1.y != angle2.y || angle1.z != angle2.z)
		{
			AngleQuaternion( angle1, q1 );
			AngleQuaternion( angle2, q2 );
			QuaternionBlend( q1, q2, s, q );
		}
		else
		{
			AngleQuaternion( angle1, q );
		}
	}
	else
	{
		// No rotation animation - use bone default quaternion
		q = pbone->quat;
	}

	// Align to unified bone orientation
	if (!(panim->flags & STUDIO_ANIM_DELTA) && (pbone->flags & BONE_FIXED_ALIGNMENT))
	{
		QuaternionAlign( pbone->qAlignment, q, q );
	}

	Assert( q.IsValid() );
}


//-----------------------------------------------------------------------------
// v48: Extract position from compressed animation data
// Supports Vector48 and animated (RLE) position values
//-----------------------------------------------------------------------------
void CalcBonePosition_v48( const studiohdr_t *pStudioHdr, int frame, float s,
	const mstudiobone_v48_t *pbone, const mstudioanim_v48_t *panim, Vector &pos )
{
	if (panim->flags & STUDIO_ANIM_RAWPOS)
	{
		// Compressed position (48-bit using float16)
		pos = *(panim->pPos());
		Assert( pos.IsValid() );
	}
	else if (panim->flags & STUDIO_ANIM_ANIMPOS)
	{
		// Animated position - RLE compressed values
		int j, k;
		mstudioanim_valueptr_t *pPosV = panim->pPosV();

		for (j = 0; j < 3; j++)
		{
			pos[j] = pbone->pos[j]; // Start with bone default

			if (pPosV->offset[j] != 0)
			{
				mstudioanimvalue_t *panimvalue = pPosV->pAnimvalue(j);
				k = frame;

				// Safety check
				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;

				// Find the span containing our frame
				while (panimvalue->num.total <= k)
				{
					k -= panimvalue->num.total;
					panimvalue += panimvalue->num.valid + 1;
					if (panimvalue->num.total < panimvalue->num.valid)
						k = 0;
				}

				// Extract animation values for interpolation
				if (panimvalue->num.valid > k)
				{
					if (panimvalue->num.valid > k + 1)
					{
						pos[j] += (panimvalue[k + 1].value * (1.0f - s) + s * panimvalue[k + 2].value) * pbone->posscale[j];
					}
					else
					{
						pos[j] += panimvalue[k + 1].value * pbone->posscale[j];
					}
				}
				else
				{
					if (panimvalue->num.total <= k + 1)
					{
						pos[j] += (panimvalue[panimvalue->num.valid].value * (1.0f - s) + s * panimvalue[panimvalue->num.valid + 2].value) * pbone->posscale[j];
					}
					else
					{
						pos[j] += panimvalue[panimvalue->num.valid].value * pbone->posscale[j];
					}
				}
			}
		}
	}
	else
	{
		// No position animation - use bone default position
		pos = pbone->pos;
	}

	Assert( pos.IsValid() );
}


static mstudiobonecontroller_t* FindController( const studiohdr_t *pStudioHdr, int iController)
{
	// find first controller that matches the index
	for (int i = 0; i < StudioHdr_GetNumBoneControllers(pStudioHdr); i++)
	{
		mstudiobonecontroller_t *pbonecontroller = StudioHdr_GetBoneController(pStudioHdr, i);
		if (pbonecontroller && pbonecontroller->inputfield == iController)
			return pbonecontroller;
	}

	return NULL;
}


//-----------------------------------------------------------------------------
// Purpose: return a sub frame position for a single bone
//-----------------------------------------------------------------------------
void CalcBonePosition( const studiohdr_t *pStudioHdr, int frame, float s, 
	const mstudiobone_t *pbone, const mstudioanim_t *panim, Vector &pos	)
{
	int					j, k;
	mstudioanimvalue_t	*panimvalue;

	if (!(panim->flags & STUDIO_POS_ANIMATED))
	{
		pos.Init( panim->u.pose.pos[0], panim->u.pose.pos[1], panim->u.pose.pos[2] );
		return;
	}

	for (j = 0; j < 3; j++)
	{
		pos[j] = pbone->value[j]; // default;
		if (panim->u.offset[j] != 0)
		{
			panimvalue = panim->pAnimvalue( j );
			/*
			if (i == 0 && j == 0)
				Con_DPrintf("%d  %d:%d  %f\n", frame, panimvalue->num.valid, panimvalue->num.total, s );
			*/
			
			k = frame;
			// DEBUG
			if (panimvalue->num.total < panimvalue->num.valid)
				k = 0;
			// find span of values that includes the frame we want
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
  				// DEBUG
				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;
			}
			// if we're inside the span
			if (panimvalue->num.valid > k)
			{
				// and there's more data in the span
				if (panimvalue->num.valid > k + 1)
				{
					pos[j] += (panimvalue[k+1].value * (1.0 - s) + s * panimvalue[k+2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[k+1].value * pbone->scale[j];
				}
			}
			else
			{
				// are we at the end of the repeating values section and there's another section with data?
				if (panimvalue->num.total <= k + 1)
				{
					pos[j] += (panimvalue[panimvalue->num.valid].value * (1.0 - s) + s * panimvalue[panimvalue->num.valid + 2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[panimvalue->num.valid].value * pbone->scale[j];
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Helper function to extract animation values from mstudioanimvalue_t
// Used by CalcDecompressedAnimation for local hierarchy support
//-----------------------------------------------------------------------------
inline void ExtractAnimValue( int frame, mstudioanimvalue_t *panimvalue, float scale, float &v1, float &v2 )
{
	if ( !panimvalue )
	{
		v1 = v2 = 0;
		return;
	}

	int k = frame;

	while (panimvalue->num.total <= k)
	{
		k -= panimvalue->num.total;
		panimvalue += panimvalue->num.valid + 1;
		if ( panimvalue->num.total == 0 )
		{
			Assert( 0 ); // bad data
			v1 = v2 = 0;
			return;
		}
	}

	if (panimvalue->num.valid > k)
	{
		v1 = panimvalue[k+1].value * scale;

		if (panimvalue->num.valid > k + 1)
		{
			v2 = panimvalue[k+2].value * scale;
		}
		else
		{
			if (panimvalue->num.total > k + 1)
				v2 = v1;
			else
				v2 = panimvalue[panimvalue->num.valid+2].value * scale;
		}
	}
	else
	{
		v1 = panimvalue[panimvalue->num.valid].value * scale;
		if (panimvalue->num.total > k + 1)
		{
			v2 = v1;
		}
		else
		{
			v2 = panimvalue[panimvalue->num.valid + 2].value * scale;
		}
	}
}

inline void ExtractAnimValue( int frame, mstudioanimvalue_t *panimvalue, float scale, float &v1 )
{
	if ( !panimvalue )
	{
		v1 = 0;
		return;
	}

	int k = frame;

	while (panimvalue->num.total <= k)
	{
		k -= panimvalue->num.total;
		panimvalue += panimvalue->num.valid + 1;
		if ( panimvalue->num.total == 0 )
		{
			Assert( 0 ); // bad data
			v1 = 0;
			return;
		}
	}

	if (panimvalue->num.valid > k)
	{
		v1 = panimvalue[k+1].value * scale;
	}
	else
	{
		v1 = panimvalue[panimvalue->num.valid].value * scale;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Decompress animation data from mstudiocompressedikerror_t
// Used for local hierarchy bone animations (v48)
//-----------------------------------------------------------------------------
static void CalcDecompressedAnimation( const mstudiocompressedikerror_t *pCompressed, int iFrame, float fraq, Vector &pos, Quaternion &q )
{
	if (fraq > 0.0001f)
	{
		Vector p1, p2;
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 0 ), pCompressed->scale[0], p1.x, p2.x );
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 1 ), pCompressed->scale[1], p1.y, p2.y );
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 2 ), pCompressed->scale[2], p1.z, p2.z );
		pos = p1 * (1 - fraq) + p2 * fraq;

		Quaternion			q1, q2;
		RadianEuler			angle1, angle2;
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 3 ), pCompressed->scale[3], angle1.x, angle2.x );
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 4 ), pCompressed->scale[4], angle1.y, angle2.y );
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 5 ), pCompressed->scale[5], angle1.z, angle2.z );

		if (angle1.x != angle2.x || angle1.y != angle2.y || angle1.z != angle2.z)
		{
			AngleQuaternion( angle1, q1 );
			AngleQuaternion( angle2, q2 );
			QuaternionBlend( q1, q2, fraq, q );
		}
		else
		{
			AngleQuaternion( angle1, q );
		}
	}
	else
	{
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 0 ), pCompressed->scale[0], pos.x );
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 1 ), pCompressed->scale[1], pos.y );
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 2 ), pCompressed->scale[2], pos.z );

		RadianEuler			angle;
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 3 ), pCompressed->scale[3], angle.x );
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 4 ), pCompressed->scale[4], angle.y );
		ExtractAnimValue( iFrame, pCompressed->pAnimvalue( 5 ), pCompressed->scale[5], angle.z );

		AngleQuaternion( angle, q );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Calculate local hierarchy animation overrides (v48)
// Allows bones to be temporarily reparented during an animation
// NOTE: This function is v48-specific and uses pBone_v48() for correct struct offsets
//-----------------------------------------------------------------------------
static void CalcLocalHierarchyAnimation(
	const studiohdr_t *pStudioHdr,
	matrix3x4_t *boneToWorld,
	Vector *pos,
	Quaternion *q,
	const mstudiobone_t *pbone,  // Legacy parameter, use pStudioHdr->pBone_v48() instead
	mstudiolocalhierarchy_t *pHierarchy,
	int iBone,
	int iNewParent,
	float cycle,
	int iFrame,
	float flFraq,
	int boneMask
	)
{
	Vector localPos;
	Quaternion localQ;

	// make fake root transform
	static matrix3x4_t rootXform( 1.0f, 0, 0, 0,   0, 1.0f, 0, 0,  0, 0, 1.0f, 0 );

	float weight = 1.0f;

	// check to see if there's a ramp on the influence
	if ( pHierarchy->tail - pHierarchy->peak < 1.0f )
	{
		float index = cycle;

		if (pHierarchy->end > 1.0f && index < pHierarchy->start)
			index += 1.0f;

		if (index < pHierarchy->start)
			return;
		if (index >= pHierarchy->end)
			return;

		if (index < pHierarchy->peak && pHierarchy->start != pHierarchy->peak)
		{
			weight = (index - pHierarchy->start) / (pHierarchy->peak - pHierarchy->start);
		}
		else if (index > pHierarchy->tail && pHierarchy->end != pHierarchy->tail)
		{
			weight = (pHierarchy->end - index) / (pHierarchy->end - pHierarchy->tail);
		}

		weight = SimpleSpline( weight );
	}

	CalcDecompressedAnimation( pHierarchy->pLocalAnim(), iFrame - pHierarchy->iStart, flFraq, localPos, localQ );

	// Build bone chains for both bones
	BuildBoneChain( pStudioHdr, rootXform, pos, q, iBone, boneToWorld );
	BuildBoneChain( pStudioHdr, rootXform, pos, q, iNewParent, boneToWorld );

	matrix3x4_t localXform;
	QuaternionMatrix( localQ, localPos, localXform );

	ConcatTransforms( boneToWorld[iNewParent], localXform, boneToWorld[iBone] );

	// back solve
	Vector p1;
	Quaternion q1;
	// Use v48 bone accessor for correct struct size in array indexing
	int n = StudioBone_GetV48( pStudioHdr, iBone )->parent;
	if (n == -1)
	{
		if (weight == 1.0f)
		{
			MatrixAngles( boneToWorld[iBone], q[iBone], pos[iBone] );
		}
		else
		{
			MatrixAngles( boneToWorld[iBone], q1, p1 );
			QuaternionSlerp( q[iBone], q1, weight, q[iBone] );
			VectorLerp( pos[iBone], p1, weight, pos[iBone] );
		}
	}
	else
	{
		matrix3x4_t worldToBone;
		MatrixInvert( boneToWorld[n], worldToBone );

		matrix3x4_t local;
		ConcatTransforms( worldToBone, boneToWorld[iBone], local );
		if (weight == 1.0f)
		{
			MatrixAngles( local, q[iBone], pos[iBone] );
		}
		else
		{
			MatrixAngles( local, q1, p1 );
			QuaternionSlerp( q[iBone], q1, weight, q[iBone] );
			VectorLerp( pos[iBone], p1, weight, pos[iBone] );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Calculate bone rotations for v37 models (array-based animation)
//-----------------------------------------------------------------------------
static void CalcRotations_v37( const studiohdr_t *pStudioHdr, Vector *pos, Quaternion *q,
	const mstudioseqdesc_t *pseqdesc,
	const mstudioanimdesc_t *panimdesc, float cycle, int boneMask )
{
	int					i;
	mstudiobone_t *pbone = pStudioHdr->pBone( 0 );

	int					iFrame;
	float				s;

	float fFrame = cycle * (panimdesc->numframes - 1);

	iFrame = (int)fFrame;
	s = (fFrame - iFrame);

	mstudioanim_t *panim = panimdesc->pAnim( 0 );

	for (i = 0; i < pStudioHdr->numbones; i++, pbone++, panim++)
	{
		// CRITICAL: Always initialize to safe defaults first!
		// The pos/q arrays may come from uninitialized stack memory.
		// v37 stores default pose in animation's u.pose fields, but if weight <= 0
		// or boneMask doesn't match, we still need valid values.
		pos[i].Init( panim->u.pose.pos[0], panim->u.pose.pos[1], panim->u.pose.pos[2] );
		q[i].Init( panim->u.pose.q[0], panim->u.pose.q[1], panim->u.pose.q[2], panim->u.pose.q[3] );

		// Use version-safe weight accessor for v44+ seqdesc baseptr compatibility
		if (StudioSeqdesc_GetWeightFromPtr(pStudioHdr, pseqdesc, i) > 0 && (pbone->flags & boneMask))
		{
			CalcBoneQuaternion( pStudioHdr, iFrame, s, pbone, panim, q[i] );
			CalcBonePosition  ( pStudioHdr, iFrame, s, pbone, panim, pos[i] );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Calculate bone rotations for v48 models (linked-list animation)
// v48 uses a linked list of animated bones instead of array indexing.
// Only bones with animation data are in the list, others use bone defaults.
// Also handles local hierarchy overrides for runtime bone reparenting.
//-----------------------------------------------------------------------------
static void CalcRotations_v48_internal( const studiohdr_t *pStudioHdr, Vector *pos, Quaternion *q,
	const mstudioseqdesc_t *pseqdesc,
	const mstudioanimdesc_v48_t *panimdesc, float cycle, int boneMask )
{
	int iFrame;
	float s;

	float fFrame = cycle * (panimdesc->numframes - 1);
	iFrame = (int)fFrame;
	s = (fFrame - iFrame);

	// CRITICAL: Initialize ALL bones to their default pose first!
	// The pos/q arrays may come from uninitialized stack memory (e.g., AccumulatePose's local pos2/q2).
	// If we only initialize bones with weight > 0, the others remain as garbage/NaN, which causes
	// cascading NaN errors when SlerpBones tries to blend them.
	int numBones = StudioHdr_GetNumBones(pStudioHdr);
	for (int i = 0; i < numBones; i++)
	{
		mstudiobone_v48_t *pbone = StudioBone_GetV48( pStudioHdr, i );
		// Initialize all bones to safe default values
		pos[i] = pbone->pos;
		q[i] = pbone->quat;
	}

	// Now traverse the animation linked list and apply animated values
	// Use version-aware accessor that handles external animation blocks for v44+
	mstudioanim_v48_t *panim = StudioAnimDesc_GetAnim_v44_Frame(pStudioHdr, panimdesc, &iFrame);

	while (panim)
	{
		int iBone = panim->bone;

		// Make sure bone index is valid
		if (iBone >= 0 && iBone < numBones)
		{
			mstudiobone_v48_t *pbone = StudioBone_GetV48( pStudioHdr, iBone );

			// Use version-safe weight accessor for v44+ seqdesc baseptr compatibility
			if (StudioSeqdesc_GetWeightFromPtr(pStudioHdr, pseqdesc, iBone) > 0 && (pbone->flags & boneMask))
			{
				CalcBoneQuaternion_v48( pStudioHdr, iFrame, s, pbone, panim, q[iBone] );
				CalcBonePosition_v48( pStudioHdr, iFrame, s, pbone, panim, pos[iBone] );
			}
		}

		// Move to next bone in the linked list
		panim = panim->pNext();
	}

	//-----------------------------------------------------------------------------
	// v48 Local Hierarchy Support
	// Apply any local hierarchy overrides (runtime bone reparenting)
	//-----------------------------------------------------------------------------
	if (panimdesc->numlocalhierarchy > 0)
	{
		// Allocate temporary bone-to-world matrices for local hierarchy calculation
		matrix3x4_t *boneToWorld = (matrix3x4_t *)stackalloc( numBones * sizeof(matrix3x4_t) );

		// Initialize with identity matrices
		for (int i = 0; i < numBones; i++)
		{
			SetIdentityMatrix( boneToWorld[i] );
		}

		// NOTE: CalcLocalHierarchyAnimation keeps a legacy bone pointer parameter,
		// but the v48 path reads bones through version-aware accessors.
		mstudiobone_t *pbone = NULL;

		// Process each local hierarchy override
		for (int i = 0; i < panimdesc->numlocalhierarchy; i++)
		{
			mstudiolocalhierarchy_t *pHierarchy = panimdesc->pHierarchy( i );
			if ( !pHierarchy )
				break;

			int iBone = pHierarchy->iBone;
			if (iBone >= 0 && iBone < numBones &&
				(StudioBone_GetV48(pStudioHdr, iBone)->flags & boneMask))
			{
				int iNewParent = pHierarchy->iNewParent;
				if (iNewParent >= 0 && iNewParent < numBones &&
					(StudioBone_GetV48(pStudioHdr, iNewParent)->flags & boneMask))
				{
					CalcLocalHierarchyAnimation( pStudioHdr, boneToWorld, pos, q, pbone,
						pHierarchy, iBone, iNewParent, cycle, iFrame, s, boneMask );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Version-aware bone rotation calculation dispatcher
// Routes to v37 or v48 implementation based on model version
//-----------------------------------------------------------------------------
static void CalcRotations( const studiohdr_t *pStudioHdr, Vector *pos, Quaternion *q,
	const mstudioseqdesc_t *pseqdesc,
	const mstudioanimdesc_t *panimdesc, float cycle, int boneMask )
{
	if (pStudioHdr->version >= STUDIO_VERSION_44)
	{
		// v44+ models use v48-style animation with linked lists
		// Cast the animation description pointer to v48 type
		const mstudioanimdesc_v48_t *panimdesc_v48 = (const mstudioanimdesc_v48_t *)panimdesc;
		CalcRotations_v48_internal( pStudioHdr, pos, q, pseqdesc, panimdesc_v48, cycle, boneMask );
	}
	else
	{
		// v37 models use array-based animation
		CalcRotations_v37( pStudioHdr, pos, q, pseqdesc, panimdesc, cycle, boneMask );
	}
}

// qt = ( s * p ) * q
void QuaternionSM( float s, const Quaternion &p, const Quaternion &q, Quaternion &qt )
{
	Quaternion		p1, q1;

	QuaternionScale( p, s, p1 );
	QuaternionMult( p1, q, q1 );
	QuaternionNormalize( q1 );
	qt[0] = q1[0];
	qt[1] = q1[1];
	qt[2] = q1[2];
	qt[3] = q1[3];
}

// qt = p * ( s * q )
void QuaternionMA( const Quaternion &p, float s, const Quaternion &q, Quaternion &qt )
{
	Quaternion		p1, q1;

	QuaternionScale( q, s, q1 );
	QuaternionMult( p, q1, p1 );
	QuaternionNormalize( p1 );
	qt[0] = p1[0];
	qt[1] = p1[1];
	qt[2] = p1[2];
	qt[3] = p1[3];
}


// qt = p * ( s * q )
void QuaternionAccumulate( const Quaternion &p, float s, const Quaternion &q, Quaternion &qt )
{
	QuaternionAlign( p, q, qt );

	qt[0] = p[0] + s * qt[0];
	qt[1] = p[1] + s * qt[1];
	qt[2] = p[2] + s * qt[2];
	qt[3] = p[3] + s * qt[3];
}

//-----------------------------------------------------------------------------
// Purpose: blend together q1,pos1 with q2,pos2.  Return result in q1,pos1.  
//			0 returns q1, pos1.  1 returns q2, pos2
//-----------------------------------------------------------------------------
void SlerpBones( 
	const studiohdr_t *pStudioHdr,
	Quaternion q1[MAXSTUDIOBONES], 
	Vector pos1[MAXSTUDIOBONES], 
	const mstudioseqdesc_t *pseqdesc, 
	const Quaternion q2[MAXSTUDIOBONES], 
	const Vector pos2[MAXSTUDIOBONES], 
	float s,
	int boneMask )
{
	int			i;
	Quaternion		q3, q4;
	float		s1, s2;

	if (s <= 0.0f)
	{
		return;
	}
	else if (s > 1.0f)
	{
		s = 1.0f;
	}

	int seqFlags = StudioSeqdesc_GetFlagsFromPtr( pStudioHdr, pseqdesc );
	int numBones = StudioHdr_GetNumBones(pStudioHdr);

	if (seqFlags & STUDIO_DELTA)
	{
		for (i = 0; i < numBones; i++)
		{
			// skip unused bones - use version-aware flag accessor for v44+ support
			if (!(StudioBone_GetFlags(pStudioHdr, i) & boneMask))
			{
				continue;
			}

			// Use version-safe weight accessor for v44+ seqdesc baseptr compatibility
			s2 = s * StudioSeqdesc_GetWeightFromPtr(pStudioHdr, pseqdesc, i);	// blend in based on this bones weight
			if (s2 > 0.0)
			{
				if (seqFlags & STUDIO_POST)
				{
					QuaternionMA( q1[i], s2, q2[i], q1[i] );

					// FIXME: are these correct?
					pos1[i][0] = pos1[i][0] + pos2[i][0] * s2;
					pos1[i][1] = pos1[i][1] + pos2[i][1] * s2;
					pos1[i][2] = pos1[i][2] + pos2[i][2] * s2;
				}
				else
				{
					QuaternionSM( s2, q2[i], q1[i], q1[i] );

					// FIXME: are these correct?
					pos1[i][0] = pos1[i][0] + pos2[i][0] * s2;
					pos1[i][1] = pos1[i][1] + pos2[i][1] * s2;
					pos1[i][2] = pos1[i][2] + pos2[i][2] * s2;
				}
			}
		}
	}
	else
	{
		for (i = 0; i < numBones; i++)
		{
			// skip unused bones - use version-aware flag accessor for v44+ support
			if (!(StudioBone_GetFlags(pStudioHdr, i) & boneMask))
			{
				continue;
			}

			// Use version-safe weight accessor for v44+ seqdesc baseptr compatibility
			s2 = s * StudioSeqdesc_GetWeightFromPtr(pStudioHdr, pseqdesc, i);	// blend in based on this animations weights
			if (s2 > 0.0)
			{
				s1 = 1.0 - s2;

				if (StudioBone_GetFlags(pStudioHdr, i) & BONE_FIXED_ALIGNMENT)
				{
					QuaternionSlerpNoAlign( q2[i], q1[i], s1, q3 );
				}
				else
				{
					QuaternionSlerp( q2[i], q1[i], s1, q3 );
				}
				q1[i][0] = q3[0];
				q1[i][1] = q3[1];
				q1[i][2] = q3[2];
				q1[i][3] = q3[3];
				pos1[i][0] = pos1[i][0] * s1 + pos2[i][0] * s2;
				pos1[i][1] = pos1[i][1] * s1 + pos2[i][1] * s2;
				pos1[i][2] = pos1[i][2] * s1 + pos2[i][2] * s2;
			}
		}
	}
}



//-----------------------------------------------------------------------------
// Purpose: blend together q1,pos1 with q2,pos2.  Return result in q1,pos1.  
//			0 returns q1, pos1.  1 returns q2, pos2
//-----------------------------------------------------------------------------
void BlendBones( 
	const studiohdr_t *pStudioHdr,
	Quaternion q1[MAXSTUDIOBONES], 
	Vector pos1[MAXSTUDIOBONES], 
	mstudioseqdesc_t *pseqdesc,
	const Quaternion q2[MAXSTUDIOBONES], 
	const Vector pos2[MAXSTUDIOBONES], 
	float s,
	int boneMask )
{
	int			i;
	Quaternion		q3;
	int numBones = StudioHdr_GetNumBones(pStudioHdr);

	if (s <= 0)
	{
		return;
	}
	else if (s >= 1.0)
	{
		for (i = 0; i < numBones; i++)
		{
			// skip unused bones - use version-aware flag accessor for v44+ support
			if (!(StudioBone_GetFlags(pStudioHdr, i) & boneMask))
			{
				continue;
			}

			// Use version-safe weight accessor for v44+ seqdesc baseptr compatibility
			if (StudioSeqdesc_GetWeightFromPtr(pStudioHdr, pseqdesc, i) > 0.0)
			{
				q1[i] = q2[i];
				pos1[i] = pos2[i];
			}
		}
		return;
	}

	float s2 = s;
	float s1 = 1.0 - s2;

	for (i = 0; i < numBones; i++)
	{
		// skip unused bones - use version-aware flag accessor for v44+ support
		if (!(StudioBone_GetFlags(pStudioHdr, i) & boneMask))
		{
			continue;
		}

		// Use version-safe weight accessor for v44+ seqdesc baseptr compatibility
		if (StudioSeqdesc_GetWeightFromPtr(pStudioHdr, pseqdesc, i) > 0.0)
		{
			if (StudioBone_GetFlags(pStudioHdr, i) & BONE_FIXED_ALIGNMENT)
			{
				QuaternionBlendNoAlign( q2[i], q1[i], s1, q3 );
			}
			else
			{
				QuaternionBlend( q2[i], q1[i], s1, q3 );
			}
			q1[i][0] = q3[0];
			q1[i][1] = q3[1];
			q1[i][2] = q3[2];
			q1[i][3] = q3[3];
			pos1[i][0] = pos1[i][0] * s1 + pos2[i][0] * s2;
			pos1[i][1] = pos1[i][1] * s1 + pos2[i][1] * s2;
			pos1[i][2] = pos1[i][2] * s1 + pos2[i][2] * s2;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: resolve a global pose parameter to the specific setting for this sequence
//-----------------------------------------------------------------------------
void Studio_LocalPoseParameter( const studiohdr_t *pStudioHdr, const float poseParameter[], const mstudioseqdesc_t *pSeqDesc, int iLocalPose, float &flSetting, int &index )
{
	if (iLocalPose < 0 || iLocalPose >= 2)
	{
		flSetting = 0;
		index = 0;
		return;
	}

	int paramIdx = StudioSeqdesc_GetParamIndexFromPtr( pStudioHdr, pSeqDesc, iLocalPose );
	if (paramIdx == -1)
	{
		flSetting = 0;
		index = 0;
		return;
	}

	// Bounds check against number of pose parameters (use version-aware helper)
	int numPoseParams = StudioHdr_GetNumPoseParameters(pStudioHdr);
	if (paramIdx < 0 || paramIdx >= numPoseParams)
	{
		flSetting = 0;
		index = 0;
		return;
	}

	mstudioposeparamdesc_t *pPose = StudioHdr_GetPoseParameter(pStudioHdr, paramIdx);
	if (!pPose)
	{
		flSetting = 0;
		index = 0;
		return;
	}

	float flValue = poseParameter[paramIdx];

	// Sanitize input - NaN pose parameters can cause cascading NaN issues
	if (!IsFinite(flValue))
		flValue = 0.0f;

	if (pPose->loop)
	{
		float wrap = (pPose->start + pPose->end) / 2.0 + pPose->loop / 2.0;
		float shift = pPose->loop - wrap;

		// Guard against division by zero in loop calculation
		if (pPose->loop != 0.0f)
			flValue = flValue - pPose->loop * floor((flValue + shift) / pPose->loop);
	}

	int poseKeyIdx = StudioSeqdesc_GetPoseKeyIndexFromPtr( pStudioHdr, pSeqDesc );
	int groupSize = StudioSeqdesc_GetGroupSizeFromPtr( pStudioHdr, pSeqDesc, iLocalPose );

	if (poseKeyIdx == 0)
	{
		// Direct access to paramstart/paramend
		// Guard against division by zero which can cause NaN
		float poseRange = pPose->end - pPose->start;
		if (poseRange == 0.0f) poseRange = 1.0f;  // Prevent div by zero

		float flLocalStart	= (StudioSeqdesc_GetParamStartFromPtr( pStudioHdr, pSeqDesc, iLocalPose ) - pPose->start) / poseRange;
		float flLocalEnd	= (StudioSeqdesc_GetParamEndFromPtr( pStudioHdr, pSeqDesc, iLocalPose ) - pPose->start) / poseRange;

		// convert into local range
		// Guard against division by zero
		float localRange = flLocalEnd - flLocalStart;
		if (localRange == 0.0f) localRange = 1.0f;  // Prevent div by zero
		flSetting = (flValue - flLocalStart) / localRange;

		// clamp.  This shouldn't ever need to happen if it's looping.
		// Note: Must check IsFinite BEFORE clamp, as NaN comparisons are always false
		if (!IsFinite(flSetting))
			flSetting = 0.0f;
		if (flSetting < 0)
			flSetting = 0;
		if (flSetting > 1)
			flSetting = 1;

		index = 0;
		if (groupSize > 2 )
		{
			// estimate index
			index = (int)(flSetting * (groupSize - 1));
			if (index == groupSize - 1) index = groupSize - 2;
			flSetting = flSetting * (groupSize - 1) - index;
		}
	}
	else
	{
		flValue = flValue * (pPose->end - pPose->start) + pPose->start;
		index = 0;

		while (1)
		{
			float poseKeyThis = StudioSeqdesc_GetPoseKeyFromPtr( pStudioHdr, pSeqDesc, iLocalPose, index );
			float poseKeyNext = StudioSeqdesc_GetPoseKeyFromPtr( pStudioHdr, pSeqDesc, iLocalPose, index + 1 );
			// Guard against division by zero
			float keyRange = poseKeyNext - poseKeyThis;
			if (keyRange == 0.0f) keyRange = 1.0f;  // Prevent div by zero
			flSetting = (flValue - poseKeyThis) / keyRange;
			if (index > 0 && flSetting < 0.0)
			{
				index--;
				continue;
			}
			else if (index < groupSize - 2 && flSetting > 1.0)
			{
				index++;
				continue;
			}
			break;
		}

		// clamp.
		// Note: Must check IsFinite BEFORE clamp, as NaN comparisons are always false
		if (!IsFinite(flSetting))
			flSetting = 0.0f;
		if (flSetting < 0.0f)
			flSetting = 0.0f;
		if (flSetting > 1.0f)
			flSetting = 1.0f;
	}

	return;
}

void Studio_CalcBoneToBoneTransform( const studiohdr_t *pStudioHdr, int inputBoneIndex, int outputBoneIndex, matrix3x4_t& matrixOut )
{
	// Use version-aware poseToBone accessor for v44+ support
	const matrix3x4_t *pInputPoseToBone = StudioBone_GetPoseToBonePtr(pStudioHdr, inputBoneIndex);
	const matrix3x4_t *pOutputPoseToBone = StudioBone_GetPoseToBonePtr(pStudioHdr, outputBoneIndex);

	if (!pInputPoseToBone || !pOutputPoseToBone)
	{
		SetIdentityMatrix(matrixOut);
		return;
	}

	matrix3x4_t inputToPose;
	MatrixInvert( *pInputPoseToBone, inputToPose );
	ConcatTransforms( *pOutputPoseToBone, inputToPose, matrixOut );
}

//-----------------------------------------------------------------------------
// Purpose: calculate a pose for a single sequence
// Version-aware: v37 uses value[0-5], v48 uses pos/quat directly
//-----------------------------------------------------------------------------
void InitPose(
	const studiohdr_t *pStudioHdr,
	Vector pos[],
	Quaternion q[]
	)
{
	static int s_nNaNWarnings = 0;  // Limit NaN warning spam
	int numBones = StudioHdr_GetNumBones(pStudioHdr);

	if (pStudioHdr->version >= STUDIO_VERSION_44)
	{
		// v44+ models use pos/quat directly in bone structure
		for (int i = 0; i < numBones; i++)
		{
			mstudiobone_v48_t *pbone = StudioBone_GetV48( pStudioHdr, i );
			pos[i] = pbone->pos;
			q[i] = pbone->quat;

			// Validate bone data - NaN propagation causes cascading failures
			bool bPosValid = pos[i].IsValid();
			bool bQuatValid = q[i].IsValid();

			if (!bPosValid || !bQuatValid)
			{
				if (s_nNaNWarnings < 5)
				{
					s_nNaNWarnings++;
					DevWarning("InitPose v44+: NaN detected in bone %d ('%s') of model '%s' (v%d)\n",
						i, pbone->pszName(), pStudioHdr->name, pStudioHdr->version);
					DevWarning("  pos: (%.2f, %.2f, %.2f) valid=%d, quat: (%.2f, %.2f, %.2f, %.2f) valid=%d\n",
						pbone->pos.x, pbone->pos.y, pbone->pos.z, bPosValid,
						pbone->quat.x, pbone->quat.y, pbone->quat.z, pbone->quat.w, bQuatValid);
				}

				// Reset to safe defaults to prevent NaN propagation
				if (!bPosValid)
					pos[i].Init(0, 0, 0);
				if (!bQuatValid)
					q[i].Init(0, 0, 0, 1);
			}
		}
	}
	else
	{
		// v37 and earlier use value[0-5] array format
		for (int i = 0; i < numBones; i++)
		{
			mstudiobone_t *pbone = pStudioHdr->pBone( i );

			pos[i] = Vector( pbone->value[0], pbone->value[1], pbone->value[2] );

			// FIXME!!!
			if ( pbone->quat.w == 0.0)
			{
				AngleQuaternion( RadianEuler( pbone->value[3], pbone->value[4], pbone->value[5] ), q[i] );
				QuaternionAlign( pbone->qAlignment, q[i], q[i] );
			}
			else
			{
				q[i] = pbone->quat;
			}

			// Validate bone data for v37 models too
			bool bPosValid = pos[i].IsValid();
			bool bQuatValid = q[i].IsValid();

			if (!bPosValid || !bQuatValid)
			{
				if (s_nNaNWarnings < 5)
				{
					s_nNaNWarnings++;
					DevWarning("InitPose v37: NaN detected in bone %d of model '%s'\n",
						i, pStudioHdr->name);
				}

				// Reset to safe defaults
				if (!bPosValid)
					pos[i].Init(0, 0, 0);
				if (!bQuatValid)
					q[i].Init(0, 0, 0, 1);
			}
		}
	}
}
	
	
//-----------------------------------------------------------------------------
// Purpose: calculate a pose for a single sequence
//-----------------------------------------------------------------------------
bool CalcPoseSingle(
	const studiohdr_t *pStudioHdr,
	Vector pos[], 
	Quaternion q[], 
	int sequence, 
	float cycle,
	const float poseParameter[],
	int boneMask
	)
{
	ASSERT_NO_REENTRY();
	
	mstudioseqdesc_t	*pseqdesc;
	static Vector		pos2[MAXSTUDIOBONES];
	static Quaternion	q2[MAXSTUDIOBONES];
	static Vector		pos3[MAXSTUDIOBONES];
	static Quaternion	q3[MAXSTUDIOBONES];

	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	if (sequence >= pStudioHdr->GetNumLocalSeq())
	{
		sequence = 0;
	}

	pseqdesc = pStudioHdr->pSeqdesc( sequence );

	int i0 = 0, i1 = 0;
	float s0 = 0, s1 = 0;
	mstudioanimdesc_t		*panim0;
	mstudioanimdesc_t		*panim1;

	Studio_LocalPoseParameter( pStudioHdr, poseParameter, pseqdesc, 0, s0, i0 );
	Studio_LocalPoseParameter( pStudioHdr, poseParameter, pseqdesc, 1, s1, i1 );

	int seqFlags = StudioSeqdesc_GetFlagsFromPtr( pStudioHdr, pseqdesc );
	int groupSize0, groupSize1;
	if (pStudioHdr->version >= STUDIO_VERSION_44)
	{
		const mstudioseqdesc_v48_t *pseqdesc_v48 = (const mstudioseqdesc_v48_t *)pseqdesc;
		groupSize0 = pseqdesc_v48->groupsize[0];
		groupSize1 = pseqdesc_v48->groupsize[1];
	}
	else
	{
		groupSize0 = pseqdesc->groupsize[0];
		groupSize1 = pseqdesc->groupsize[1];
	}

	if (cycle < 0 || cycle >= 1)
	{
		if (seqFlags & STUDIO_LOOPING)
		{
			cycle = cycle - (int)cycle;
			if (cycle < 0) cycle += 1;
		}
		else
		{
			cycle = max( 0.0, min( cycle, 0.9999f ) );
		}
	}

	if (groupSize1 == 1)
	{
		if (groupSize0 == 1)
		{
			panim0 = GetAnimDescriptions( pStudioHdr, pseqdesc, 0, 0 );
			if ( !panim0 )
			{
				Warning( "CalcPoseSingle: No animation data for sequence %d in model %s\n", sequence, pStudioHdr->name );
				return false;
			}
			CalcRotations( pStudioHdr, pos, q, pseqdesc, panim0, cycle, boneMask);
		}
		else
		{
			panim0 = GetAnimDescriptions( pStudioHdr, pseqdesc, i0  , i1  );
			panim1 = GetAnimDescriptions( pStudioHdr, pseqdesc, i0+1, i1  );

			if ( !panim0 || !panim1 )
			{
				Warning( "CalcPoseSingle: Missing animation blend data for sequence %d in model %s\n", sequence, pStudioHdr->name );
				return false;
			}

			// remove "zero" positional blends
			if ((panim0->flags & STUDIO_ALLZEROS) && (s0 < 0.001))
			{
				return false;
			}
			if ((panim1->flags & STUDIO_ALLZEROS) && (s0 > 0.999))
			{
				return false;
			}

			CalcRotations( pStudioHdr, pos,  q,  pseqdesc, panim0, cycle, boneMask );
			CalcRotations( pStudioHdr, pos2, q2, pseqdesc, panim1, cycle, boneMask );

			BlendBones( pStudioHdr, q, pos, pseqdesc, q2, pos2, s0, boneMask );
		}
	}
	else
	{
		if (groupSize0 == 1)
		{
			panim0 = GetAnimDescriptions( pStudioHdr, pseqdesc, i0  , i1  );
			panim1 = GetAnimDescriptions( pStudioHdr, pseqdesc, i0  , i1+1);

			if ( !panim0 || !panim1 )
			{
				Warning( "CalcPoseSingle: Missing animation blend data for sequence %d in model %s\n", sequence, pStudioHdr->name );
				return false;
			}

			CalcRotations( pStudioHdr, pos,  q,  pseqdesc, panim0, cycle, boneMask );
			CalcRotations( pStudioHdr, pos2, q2, pseqdesc, panim1, cycle, boneMask );

			BlendBones( pStudioHdr, q, pos, pseqdesc, q2, pos2, s0, boneMask );
		}
		else
		{
			panim0 = GetAnimDescriptions( pStudioHdr, pseqdesc, i0  , i1);
			panim1 = GetAnimDescriptions( pStudioHdr, pseqdesc, i0+1, i1);

			if ( !panim0 || !panim1 )
			{
				Warning( "CalcPoseSingle: Missing animation blend data for sequence %d in model %s\n", sequence, pStudioHdr->name );
				return false;
			}

			CalcRotations( pStudioHdr, pos,  q,  pseqdesc, panim0, cycle, boneMask );
			CalcRotations( pStudioHdr, pos2, q2, pseqdesc, panim1, cycle, boneMask );

			BlendBones( pStudioHdr, q, pos, pseqdesc, q2, pos2, s0, boneMask );

			panim0 = GetAnimDescriptions( pStudioHdr, pseqdesc, i0  , i1+1);
			panim1 = GetAnimDescriptions( pStudioHdr, pseqdesc, i0+1, i1+1);

			if ( !panim0 || !panim1 )
			{
				Warning( "CalcPoseSingle: Missing animation blend data for sequence %d in model %s\n", sequence, pStudioHdr->name );
				return false;
			}

			CalcRotations( pStudioHdr, pos2, q2, pseqdesc, panim0, cycle, boneMask );
			CalcRotations( pStudioHdr, pos3, q3, pseqdesc, panim1, cycle, boneMask );

			BlendBones( pStudioHdr, q2, pos2, pseqdesc, q3, pos3, s0, boneMask );
			BlendBones( pStudioHdr, q, pos, pseqdesc, q2, pos2, s1, boneMask );
		}
	}

	return true;
}




//-----------------------------------------------------------------------------
// Purpose: calculate a pose for a single sequence
//			adds autolayers, runs local ik rukes
//-----------------------------------------------------------------------------
void AddSequenceLayers(
	const studiohdr_t *pStudioHdr,
	CIKContext *pIKContext,
	Vector pos[], 
	Quaternion q[], 
	int sequence, 
	float cycle,
	const float poseParameter[],
	int boneMask,
	float flWeight
	)
{
	mstudioseqdesc_t	*pseqdesc = pStudioHdr->pSeqdesc( sequence );

	int numAutoLayers = StudioSeqdesc_GetNumAutolayersFromPtr( pStudioHdr, pseqdesc );

	for (int i = 0; i < numAutoLayers; i++)
	{
		mstudioautolayer_t *pLayer = StudioSeqdesc_GetAutolayerFromPtr( pStudioHdr, pseqdesc, i );
		if (!pLayer)
			continue;

		float layerCycle = cycle;
		float layerWeight = flWeight;

		float layerStart = StudioAutolayer_GetStart( pStudioHdr, pLayer );
		float layerPeak = StudioAutolayer_GetPeak( pStudioHdr, pLayer );
		float layerTail = StudioAutolayer_GetTail( pStudioHdr, pLayer );
		float layerEnd = StudioAutolayer_GetEnd( pStudioHdr, pLayer );
		int layerFlags = StudioAutolayer_GetFlags( pStudioHdr, pLayer );

		if (layerStart != layerEnd)
		{
			float s = 1.0;

			if (cycle < layerStart)
				continue;
			if (cycle >= layerEnd)
				continue;

			if (layerFlags & STUDIO_WEIGHT)
			{
				if (cycle < layerPeak && layerStart != layerPeak)
				{
					s = (cycle - layerStart) / (layerPeak - layerStart);
				}
				else if (cycle > layerTail && layerEnd != layerTail)
				{
					s = (layerEnd - cycle) / (layerEnd - layerTail);
				}

				if (layerFlags & STUDIO_SPLINE)
				{
					s = 3 * s * s - 2 * s * s * s;
				}

				layerWeight = flWeight * s;
			}
			else
			{
				layerWeight = flWeight;
			}

			layerCycle = (cycle - layerStart) / (layerEnd - layerStart);
		}

		AccumulatePose( pStudioHdr, pIKContext, pos, q, StudioAutolayer_GetSequence( pStudioHdr, pLayer ), layerCycle, poseParameter, boneMask, layerWeight );
	}
}


//-----------------------------------------------------------------------------
// Purpose: calculate a pose for a single sequence
//			adds autolayers, runs local ik rukes
//-----------------------------------------------------------------------------
void CalcPose(
	const studiohdr_t *pStudioHdr,
	CIKContext *pIKContext,
	Vector pos[], 
	Quaternion q[], 
	int sequence, 
	float cycle,
	const float poseParameter[],
	int boneMask,
	float flWeight
	)
{
	mstudioseqdesc_t	*pseqdesc = pStudioHdr->pSeqdesc( sequence );

	Assert( flWeight >= 0.0f && flWeight <= 1.0f );
	// This shouldn't be necessary, but the Assert should help us catch whoever is screwing this up
	flWeight = clamp( flWeight, 0.0f, 1.0f );

	if ( pIKContext )
	{
		pIKContext->AddDependencies( sequence, cycle, poseParameter, flWeight );
	}
	
	CalcPoseSingle( pStudioHdr, pos, q, sequence, cycle, poseParameter, boneMask );

	// add any IK locks to prevent numautolayers from moving extremities
	CIKContext seq_ik;
	int numIKLocks = StudioSeqdesc_GetNumIKLocksFromPtr( pStudioHdr, pseqdesc );
	if (numIKLocks)
	{
		seq_ik.Init( pStudioHdr, QAngle( 0, 0, 0 ), Vector( 0, 0, 0 ), 0.0 ); // local space relative so absolute position doesn't mater
		seq_ik.AddSequenceLocks( pseqdesc, pos, q );
	}

	AddSequenceLayers( 	pStudioHdr, pIKContext, pos, q, sequence, cycle, poseParameter, boneMask, flWeight );

	if (numIKLocks)
	{
		seq_ik.SolveSequenceLocks( pseqdesc, pos, q );
	}
}


//-----------------------------------------------------------------------------
// Purpose: accumulate a pose for a single sequence on top of existing animation
//			adds autolayers, runs local ik rukes
//-----------------------------------------------------------------------------
void AccumulatePose(
	const studiohdr_t *pStudioHdr,
	CIKContext *pIKContext,
	Vector pos[], 
	Quaternion q[], 
	int sequence, 
	float cycle,
	const float poseParameter[],
	int boneMask,
	float flWeight
	)
{
	Vector		pos2[MAXSTUDIOBONES];
	Quaternion	q2[MAXSTUDIOBONES];

	Assert( flWeight >= 0.0f && flWeight <= 1.0f );
	// This shouldn't be necessary, but the Assert should help us catch whoever is screwing this up
	flWeight = clamp( flWeight, 0.0f, 1.0f );

	mstudioseqdesc_t	*pseqdesc = pStudioHdr->pSeqdesc( sequence );

	if (!CalcPoseSingle( pStudioHdr, pos2, q2, sequence, cycle, poseParameter, boneMask ))
	{
		return;
	}

	// add any IK locks to prevent extremities from moving
	CIKContext seq_ik;
	int numIKLocks = StudioSeqdesc_GetNumIKLocksFromPtr( pStudioHdr, pseqdesc );
	if (numIKLocks)
	{
		seq_ik.Init( pStudioHdr, QAngle( 0, 0, 0 ), Vector( 0, 0, 0 ), 0.0 );  // local space relative so absolute position doesn't mater
		seq_ik.AddSequenceLocks( pseqdesc, pos, q );
	}

	if ( pIKContext )
	{
		pIKContext->AddDependencies( sequence, cycle, poseParameter, flWeight );
	}

	SlerpBones( pStudioHdr, q, pos, pseqdesc, q2, pos2, flWeight, boneMask );

	AddSequenceLayers( 	pStudioHdr, pIKContext, pos, q, sequence, cycle, poseParameter, boneMask, flWeight );

	if (numIKLocks)
	{
		seq_ik.SolveSequenceLocks( pseqdesc, pos, q );
	}
}


//-----------------------------------------------------------------------------
// Purpose: blend together q1,pos1 with q2,pos2.  Return result in q1,pos1.  
//			0 returns q1, pos1.  1 returns q2, pos2
//-----------------------------------------------------------------------------
void CalcBoneAdj(
	const studiohdr_t *pStudioHdr,
	Vector pos[],
	Quaternion q[],
	const float controllers[],
	int boneMask
	)
{
	int					i, j, k;
	float				value;
	mstudiobonecontroller_t *pbonecontroller;
	RadianEuler a0;
	Quaternion q0;
	
	for (j = 0; j < StudioHdr_GetNumBoneControllers(pStudioHdr); j++)
	{
		pbonecontroller = StudioHdr_GetBoneController(pStudioHdr, j);
		if ( !pbonecontroller )
			continue;

		k = pbonecontroller->bone;

		if (StudioBone_GetFlags(pStudioHdr, k) & boneMask)
		{
			i = pbonecontroller->inputfield;
			value = controllers[i];
			if (value < 0) value = 0;
			if (value > 1.0) value = 1.0;
			value = (1.0 - value) * pbonecontroller->start + value * pbonecontroller->end;

			switch(pbonecontroller->type & STUDIO_TYPES)
			{
			case STUDIO_XR:
				a0.Init( value * (M_PI / 180.0), 0, 0 );
				AngleQuaternion( a0, q0 );
				QuaternionSM( 1.0, q0, q[k], q[k] );
				break;
			case STUDIO_YR:
				a0.Init( 0, value * (M_PI / 180.0), 0 );
				AngleQuaternion( a0, q0 );
				QuaternionSM( 1.0, q0, q[k], q[k] );
				break;
			case STUDIO_ZR:
				a0.Init( 0, 0, value * (M_PI / 180.0) );
				AngleQuaternion( a0, q0 );
				QuaternionSM( 1.0, q0, q[k], q[k] );
				break;
			case STUDIO_X:
				pos[k].x += value;
				break;
			case STUDIO_Y:
				pos[k].y += value;
				break;
			case STUDIO_Z:
				pos[k].z += value;
				break;
			}
		}
	}
}


void CalcBoneDerivatives( Vector &velocity, AngularImpulse &angVel, const matrix3x4_t &prev, const matrix3x4_t &current, float dt )
{
	float scale = 1.0;
	if ( dt > 0 )
	{
		scale = 1.0 / dt;
	}
	
	Vector endPosition, startPosition, deltaAxis;
	QAngle endAngles, startAngles;
	float deltaAngle;

	MatrixAngles( prev, startAngles, startPosition );
	MatrixAngles( current, endAngles, endPosition );

	velocity = (endPosition - startPosition) * scale;
	RotationDeltaAxisAngle( startAngles, endAngles, deltaAxis, deltaAngle );
	angVel = deltaAxis * (deltaAngle * scale);
}

void CalcBoneVelocityFromDerivative( const QAngle &vecAngles, Vector &velocity, AngularImpulse &angVel, const matrix3x4_t &current )
{
	Vector vecLocalVelocity;
	AngularImpulse LocalAngVel;
	Quaternion q;
	float angle;
	MatrixAngles( current, q, vecLocalVelocity );
	QuaternionAxisAngle( q, LocalAngVel, angle );
	LocalAngVel *= angle;

	matrix3x4_t matAngles;
	AngleMatrix( vecAngles, matAngles );
	VectorTransform( vecLocalVelocity, matAngles, velocity );
	VectorTransform( LocalAngVel, matAngles, angVel );
}

class ik
{
public:
//-------- SOLVE TWO LINK INVERSE KINEMATICS -------------
// Author: Ken Perlin
//
// Given a two link joint from [0,0,0] to end effector position P,
// let link lengths be a and b, and let norm |P| = c.  Clearly a+b >= c.
//
// Problem: find a "knee" position Q such that |Q| = a and |P-Q| = b.
//
// In the case of a point on the x axis R = [c,0,0], there is a
// closed form solution S = [d,e,0], where |S| = a and |R-S| = b:
//
//    d2+e2 = a2                  -- because |S| = a
//    (c-d)2+e2 = b2              -- because |R-S| = b
//
//    c2-2cd+d2+e2 = b2           -- combine the two equations
//    c2-2cd = b2 - a2
//    c-2d = (b2-a2)/c
//    d - c/2 = (a2-b2)/c / 2
//
//    d = (c + (a2-b2/c) / 2      -- to solve for d and e.
//    e = sqrt(a2-d2)

   static float findD(float a, float b, float c) {
      return max(0, min(a, (c + (a*a-b*b)/c) / 2));
   }
   static float findE(float a, float d) { return sqrt(a*a-d*d); } 

// This leads to a solution to the more general problem:
//
//   (1) R = Mfwd(P)         -- rotate P onto the x axis
//   (2) Solve for S
//   (3) Q = Minv(S)         -- rotate back again

   static float Mfwd[3][3];
   static float Minv[3][3];

   static bool solve(float A, float B, float const P[], float const D[], float Q[]) {
      float R[3];
      defineM(P,D);
      rot(Minv,P,R);
      float d = findD(A,B,length(R));
      float e = findE(A,d);
      float S[3] = {d,e,0};
      rot(Mfwd,S,Q);
      return d > 0 && d < A;
   }

// If "knee" position Q needs to be as close as possible to some point D,
// then choose M such that M(D) is in the y>0 half of the z=0 plane.
//
// Given that constraint, define the forward and inverse of M as follows:

   static void defineM(float const P[], float const D[]) {
      float *X = Minv[0], *Y = Minv[1], *Z = Minv[2];

// Minv defines a coordinate system whose x axis contains P, so X = unit(P).
	  int i;
      for (i = 0 ; i < 3 ; i++)
         X[i] = P[i];
      normalize(X);

// Its y axis is perpendicular to P, so Y = unit( E - X(E�X) ).

      float dDOTx = dot(D,X);
      for (i = 0 ; i < 3 ; i++)
         Y[i] = D[i] - dDOTx * X[i];
      normalize(Y);

// Its z axis is perpendicular to both X and Y, so Z = X�Y.

      cross(X,Y,Z);

// Mfwd = (Minv)T, since transposing inverts a rotation matrix.

      for (i = 0 ; i < 3 ; i++) {
         Mfwd[i][0] = Minv[0][i];
         Mfwd[i][1] = Minv[1][i];
         Mfwd[i][2] = Minv[2][i];
      }
   }

//------------ GENERAL VECTOR MATH SUPPORT -----------

   static float dot(float const a[], float const b[]) { return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]; }

   static float length(float const v[]) { return sqrt( dot(v,v) ); }

   static void normalize(float v[]) {
      float norm = length(v);
      for (int i = 0 ; i < 3 ; i++)
         v[i] /= norm;
   }

   static void cross(float const a[], float const b[], float c[]) {
      c[0] = a[1] * b[2] - a[2] * b[1];
      c[1] = a[2] * b[0] - a[0] * b[2];
      c[2] = a[0] * b[1] - a[1] * b[0];
   }

   static void rot(float const M[3][3], float const src[], float dst[]) {
      for (int i = 0 ; i < 3 ; i++)
         dst[i] = dot(M[i],src);
   }
};

float ik::Mfwd[3][3];
float ik::Minv[3][3];



//-----------------------------------------------------------------------------
// Purpose: ugly hacky hlmv debugging code
//-----------------------------------------------------------------------------
#if 1
void drawLine(const Vector& origin, const Vector& dest, int r, int g, int b, bool noDepthTest, float duration)
{
	 // drawLine( origin, dest, r, g, b, noDepthTest, duration );
}
#else
extern void drawLine( const Vector &p1, const Vector &p2, int r = 0, int g = 0, int b = 1 );
void drawLine(const Vector& origin, const Vector& dest, int r, int g, int b, bool noDepthTest, float duration)
{
	drawLine( origin, dest, r, g, b );
}
#endif


//-----------------------------------------------------------------------------
// Purpose: for a 2 bone chain, find the IK solution and reset the matrices
//-----------------------------------------------------------------------------
bool Studio_SolveIK( mstudioikchain_t *pikchain, Vector &targetFoot, matrix3x4_t *pBoneToWorld )
{
	return Studio_SolveIK( pikchain->pLink( 0 )->bone, pikchain->pLink( 1 )->bone, pikchain->pLink( 2 )->bone, targetFoot, pBoneToWorld );
}


bool Studio_SolveIK( int iThigh, int iKnee, int iFoot, Vector &targetFoot, matrix3x4_t *pBoneToWorld )
{
	Vector worldFoot, worldKnee, worldThigh;

	MatrixGetColumn( pBoneToWorld[ iThigh ], 3, worldThigh );
	MatrixGetColumn( pBoneToWorld[ iKnee ], 3, worldKnee );
	MatrixGetColumn( pBoneToWorld[ iFoot ], 3, worldFoot );

	drawLine( worldThigh, worldKnee, 0, 0, 255, true, 0 );
	drawLine( worldKnee, worldFoot, 0, 0, 255, true, 0 );

	Vector ikFoot, ikTargetKnee, ikKnee;

	ikFoot = targetFoot - worldThigh;
	ikKnee = worldKnee - worldThigh;

	float l1 = (worldKnee-worldThigh).Length();
	float l2 = (worldFoot-worldKnee).Length();
	float l3 = (worldFoot-worldThigh).Length();

	Vector ikHalf = (worldFoot-worldThigh) * (l1 / l3);

	// FIXME: what to do when the knee completely straight?
	Vector ikKneeDir = ikKnee - ikHalf;
	VectorNormalize( ikKneeDir );
	ikTargetKnee = ikKnee + ikKneeDir * l1;

	drawLine( worldKnee, worldThigh + ikTargetKnee, 0, 255, 0, true, 0);

	// leg too straight to figure out knee?
	if (l3 > (l1 + l2) * 0.9998)
	{
		return false;
	}

	// too far away? (0.9998 is about 1 degree)
	if (ikFoot.Length() > (l1 + l2) * 0.9998)
	{
		VectorNormalize( ikFoot );
		VectorScale( ikFoot, (l1 + l2) * 0.9998, ikFoot );
	}

	// too close?
	if (ikFoot.Length() < fabs(l1 - l2) * 1.01)
	{
		VectorNormalize( ikFoot );
		VectorScale( ikFoot, fabs(l1 - l2) * 1.01, ikFoot );
	}

	if (ik::solve( l1, l2, ikFoot.Base(), ikTargetKnee.Base(), ikKnee.Base() ))
	{
		matrix3x4_t& mWorldThigh = pBoneToWorld[ iThigh ];
		matrix3x4_t& mWorldKnee = pBoneToWorld[ iKnee ];
		matrix3x4_t& mWorldFoot = pBoneToWorld[ iFoot ];

		drawLine( worldThigh, ikKnee + worldThigh, 255, 0, 0, true, 0 );
		drawLine( ikKnee + worldThigh, ikFoot + worldThigh, 255, 0, 0, true,0 );

		Vector tmp1, tmp2, tmp3;

		// build transformation matrix for thigh
		tmp1 = ikKnee;
		VectorNormalize( tmp1 );
		MatrixSetColumn( tmp1, 0, mWorldThigh );

		MatrixGetColumn( mWorldThigh, 2, tmp3 );
		tmp2 = tmp3.Cross( tmp1 );
		VectorNormalize( tmp2 );
		MatrixSetColumn( tmp2, 1, mWorldThigh );

		tmp3 = tmp1.Cross( tmp2 );
		MatrixSetColumn( tmp3, 2, mWorldThigh );



		tmp1 = ikFoot - ikKnee;
		VectorNormalize(tmp1);
		MatrixSetColumn( tmp1, 0, mWorldKnee );

		MatrixGetColumn( mWorldKnee, 2, tmp3 );
		tmp2 = tmp3.Cross( tmp1 );
		VectorNormalize( tmp2 );
		MatrixSetColumn( tmp2, 1, mWorldKnee );

		tmp3 = tmp1.Cross( tmp2 );
		MatrixSetColumn( tmp3, 2, mWorldKnee );


		mWorldKnee[0][3] = ikKnee.x + worldThigh.x;
		mWorldKnee[1][3] = ikKnee.y + worldThigh.y;
		mWorldKnee[2][3] = ikKnee.z + worldThigh.z;

		mWorldFoot[0][3] = ikFoot.x + worldThigh.x;
		mWorldFoot[1][3] = ikFoot.y + worldThigh.y;
		mWorldFoot[2][3] = ikFoot.z + worldThigh.z;

		return true;
	}
	else
	{
		return false;
	}
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------


bool Studio_SolveIK( int iThigh, int iKnee, int iFoot, Vector &targetFoot, Vector &targetKnee, matrix3x4_t *pBoneToWorld )
{
	Vector worldFoot, worldKnee, worldThigh;

	MatrixGetColumn( pBoneToWorld[ iThigh ], 3, worldThigh );
	MatrixGetColumn( pBoneToWorld[ iKnee ], 3, worldKnee );
	MatrixGetColumn( pBoneToWorld[ iFoot ], 3, worldFoot );

	drawLine( worldThigh, worldKnee, 0, 0, 255, true, 0 );
	drawLine( worldKnee, worldFoot, 0, 0, 255, true, 0 );

	Vector ikFoot, ikTargetKnee, ikKnee;

	ikFoot = targetFoot - worldThigh;
	ikKnee = worldKnee - worldThigh;

	float l1 = (worldKnee-worldThigh).Length();
	float l2 = (worldFoot-worldKnee).Length();

	ikTargetKnee = targetKnee - worldThigh;

	// too far away? (0.9998 is about 1 degree)
	if (ikFoot.Length() > (l1 + l2) * 0.9998)
	{
		VectorNormalize( ikFoot );
		VectorScale( ikFoot, (l1 + l2) * 0.9998, ikFoot );
	}

	// too close?
	if (ikFoot.Length() < fabs(l1 - l2) * 1.01)
	{
		VectorNormalize( ikFoot );
		VectorScale( ikFoot, fabs(l1 - l2) * 1.01, ikFoot );
	}

	if (ik::solve( l1, l2, ikFoot.Base(), ikTargetKnee.Base(), ikKnee.Base() ))
	{
		matrix3x4_t& mWorldThigh = pBoneToWorld[ iThigh ];
		matrix3x4_t& mWorldKnee = pBoneToWorld[ iKnee ];
		matrix3x4_t& mWorldFoot = pBoneToWorld[ iFoot ];

		drawLine( worldThigh, ikKnee + worldThigh, 255, 0, 0, true, 0 );
		drawLine( ikKnee + worldThigh, ikFoot + worldThigh, 255, 0, 0, true,0 );

		Vector tmp1, tmp2, tmp3;

		// build transformation matrix for thigh
		tmp1 = ikKnee;
		VectorNormalize( tmp1 );
		MatrixSetColumn( tmp1, 0, mWorldThigh );

		MatrixGetColumn( mWorldThigh, 2, tmp3 );
		tmp2 = tmp3.Cross( tmp1 );
		VectorNormalize( tmp2 );
		MatrixSetColumn( tmp2, 1, mWorldThigh );

		tmp3 = tmp1.Cross( tmp2 );
		MatrixSetColumn( tmp3, 2, mWorldThigh );



		tmp1 = ikFoot - ikKnee;
		VectorNormalize(tmp1);
		MatrixSetColumn( tmp1, 0, mWorldKnee );

		MatrixGetColumn( mWorldKnee, 2, tmp3 );
		tmp2 = tmp3.Cross( tmp1 );
		VectorNormalize( tmp2 );
		MatrixSetColumn( tmp2, 1, mWorldKnee );

		tmp3 = tmp1.Cross( tmp2 );
		MatrixSetColumn( tmp3, 2, mWorldKnee );


		mWorldKnee[0][3] = ikKnee.x + worldThigh.x;
		mWorldKnee[1][3] = ikKnee.y + worldThigh.y;
		mWorldKnee[2][3] = ikKnee.z + worldThigh.z;

		mWorldFoot[0][3] = ikFoot.x + worldThigh.x;
		mWorldFoot[1][3] = ikFoot.y + worldThigh.y;
		mWorldFoot[2][3] = ikFoot.z + worldThigh.z;

		return true;
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

float Studio_IKRuleWeight( mstudioikrule_t &ikRule, float flCycle )
{
	if (ikRule.end > 1.0f && flCycle < ikRule.start)
	{
		flCycle = flCycle + 1.0f;
	}

	float value = 0.0f;
	if (flCycle < ikRule.start)
	{
		return 0.0f;
	}
	else if (flCycle < ikRule.peak )
	{
		value = (flCycle - ikRule.start) / (ikRule.peak - ikRule.start);
	}
	else if (flCycle < ikRule.tail )
	{
		return 1.0f;
	}
	else if (flCycle < ikRule.end )
	{
		value = 1.0f - ((flCycle - ikRule.tail) / (ikRule.end - ikRule.tail));
	}
	return 3.0f * value * value - 2.0f * value * value * value;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

bool Studio_IKShouldLatch( mstudioikrule_t &ikRule, float flCycle )
{
	if (ikRule.end > 1.0f && flCycle < ikRule.start)
	{
		flCycle = flCycle + 1.0f;
	}

	if (flCycle < ikRule.peak )
	{
		return false;
	}
	else if (flCycle < ikRule.end )
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
// Purpose: Get IK animation error for v37 models
// v37 models store IK error as direct array of mstudioikerror_t
//-----------------------------------------------------------------------------
void Studio_IKAnimationError_v37( mstudioikrule_t *pRule, mstudioanimdesc_t *panim, float flCycle, Vector &pos, Quaternion &q, float &flWeight )
{
	float fraq = (panim->numframes - 1) * flCycle;
	int iFrame = (int)fraq;
	fraq = fraq - iFrame;

	if (pRule->end > 1.0f && flCycle < pRule->start)
	{
		iFrame = iFrame + panim->numframes;
	}

	flWeight = Studio_IKRuleWeight( *pRule, flCycle );

	Assert( flWeight >= 0.0f && flWeight <= 1.0f );
	// This shouldn't be necessary, but the Assert should help us catch whoever is screwing this up
	flWeight = clamp( flWeight, 0.0f, 1.0f );

	mstudioikerror_t *pError = pRule->pError( iFrame );
	if (fraq < 0.001)
	{
		q = pError[0].q;
		pos = pError[0].pos;
	}
	else
	{
		QuaternionBlend( pError[0].q, pError[1].q, fraq, q );
		pos = pError[0].pos * (1.0f - fraq) + pError[1].pos * fraq;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get IK animation error for v48 models
// v48 models store IK error as compressed data using mstudiocompressedikerror_t
// NOTE: This is a simplified implementation - full v48 uses compressed IK data
//-----------------------------------------------------------------------------
void Studio_IKAnimationError_v48( mstudioikrule_v48_t *pRule, mstudioanimdesc_v48_t *panim, float flCycle, Vector &pos, Quaternion &q, float &flWeight )
{
	// v48 uses compressed IK error format and different influence system
	// For now, use the raw pose data from the IK rule
	flWeight = 1.0f;

	// TODO: Implement proper v48 compressed IK error decompression
	// v48 uses compressedikerrorindex to point to mstudiocompressedikerror_t data
	// which contains scale[6] and offset[6] into mstudioanimvalue_t arrays

	// Use the raw pos/q from the IK rule as fallback
	pos = pRule->pos;
	q = pRule->q;
}

//-----------------------------------------------------------------------------
// Purpose: Version-aware IK animation error dispatcher
//-----------------------------------------------------------------------------
void Studio_IKAnimationError( mstudioikrule_t *pRule, mstudioanimdesc_t *panim, float flCycle, Vector &pos, Quaternion &q, float &flWeight )
{
	// NOTE: For v48 models, this function should be called with mstudioikrule_v48_t*
	// but the signature expects mstudioikrule_t*. In practice, the caller should
	// check the model version and call the appropriate function directly.
	// This function is kept for v37 compatibility.
	Studio_IKAnimationError_v37( pRule, panim, flCycle, pos, q, flWeight );
}

//-----------------------------------------------------------------------------
// Purpose: Calculate IK sequence error for v37 models
// NOTE: v48 models use a different IK system with drop/top instead of start/peak/tail/end
// For v48 models, return false to skip v37-style IK processing
//-----------------------------------------------------------------------------

bool Studio_IKSequenceError( const studiohdr_t *pStudioHdr, int iSequence, float flCycle, int iTarget, const float poseParameter[], mstudioikrule_t &ikRule )
{
	// v48 models use different IK rule format - skip v37-style IK processing
	// v48 IK uses compressedikerrorindex and drop/top fields instead of
	// start/peak/tail/end and ikerrorindex fields
	if (pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44)
	{
		memset( &ikRule, 0, sizeof(ikRule) );
		return false;  // Signal that v37 IK processing is not applicable
	}

	mstudioanimdesc_t *panim[4];
	float	weight[4];

	int i, j;

	memset( &ikRule, 0, sizeof(ikRule) );
	Studio_SeqAnims( pStudioHdr, iSequence, poseParameter, panim, weight );

	ikRule.start = ikRule.peak = ikRule.tail = ikRule.end = 0;

	// find overall influence
	for (i = 0; i < 4; i++)
	{
		if (weight[i])
		{
			for (j = 0; j < panim[i]->numikrules; j++)
			{
				mstudioikrule_t *pRule = panim[i]->pIKRule( j );

				if (j /* pRule->index*/ == iTarget)
				{
					// BUGBUG: find the closest rule
					ikRule.start += pRule->start * weight[i];
					ikRule.peak += pRule->peak * weight[i];
					ikRule.tail += pRule->tail * weight[i];
					ikRule.end += pRule->end * weight[i];
					break;
				}
			}
		}
	}

	ikRule.flWeight = Studio_IKRuleWeight( ikRule, flCycle );
	if (ikRule.flWeight == 0.0f)
		return false;

	Assert( ikRule.flWeight > 0.0f );

	ikRule.pos.Init();
	ikRule.q.Init();

	// FIXME: add "latched" value
	ikRule.commit = Studio_IKShouldLatch( ikRule, flCycle );

	// find target error
	for (i = 0; i < 4; i++)
	{
		if (weight[i])
		{
			for (j = 0; j < panim[i]->numikrules; j++)
			{
				if (j /*panim[i]->pIKRule( j )->index*/ == iTarget)
				{
					Vector pos1;
					Quaternion q1;
					float w;
					Studio_IKAnimationError( panim[i]->pIKRule( j ), panim[i], flCycle, pos1, q1, w );

					// FIXME: move this!!
					ikRule.chain = panim[i]->pIKRule( j )->chain;
					ikRule.bone = panim[i]->pIKRule( j )->bone;
					ikRule.type = panim[i]->pIKRule( j )->type;
					ikRule.slot = panim[i]->pIKRule( j )->slot;

					ikRule.pos = ikRule.pos + pos1 *  weight[i];
					QuaternionAccumulate( ikRule.q, weight[i], q1, ikRule.q );
				}
			}
		}
	}

	QuaternionNormalize( ikRule.q );
	return true;
}



//-----------------------------------------------------------------------------
// Purpose: Estimate IK target position
// NOTE: v48 models use different IK rule format - skip v37-style IK processing
//-----------------------------------------------------------------------------

bool CIKContext::Estimate(
	int iSequence,
	float flCycle,
	int iTarget,
	const float poseParameter[],
	float flWeight
	)
{
	// v48 models use different IK rule format - skip v37-style IK estimation
	if (m_pStudioHdr && m_pStudioHdr->version >= STUDIO_VERSION_44)
	{
		return false;  // Signal that v37 IK processing is not applicable
	}

	mstudioanimdesc_t *panim[4];
	float	weight[4];

	int i, j;

	Assert( flWeight >= 0.0f && flWeight <= 1.0f );
	// This shouldn't be necessary, but the Assert should help us catch whoever is screwing this up
	flWeight = clamp( flWeight, 0.0f, 1.0f );

	Studio_SeqAnims( m_pStudioHdr, iSequence, poseParameter, panim, weight );

	int slot = 0;
	Vector pos;
	Quaternion q;

	pos.Init();
	q.Init();
	float height = 0;
	float floor = 0;
	float radius = 0;

	float latched = false;

	// find overall influence
	for (i = 0; i < 4; i++)
	{
		if (weight[i])
		{
			for (j = 0; j < panim[i]->numikrules; j++)
			{
				mstudioikrule_t *pRule = panim[i]->pIKRule( j );

				if (pRule->index == iTarget)
				{
					slot = pRule->slot;

					Vector deltaPos;
					QAngle deltaAngle;

					deltaPos.Init();

					// VXP: TODO: Test this
					AssertMsg( (STUDIO_VERSION == 35) || (STUDIO_VERSION == 36) || (STUDIO_VERSION == 37), "Remove the code after this line" );
					// hack in the contact point if the model hasn't been rebuilt
					float contact = pRule->contact;
					if (contact == 0.0 && pRule->peak != 0.0)
					{
						contact = pRule->peak;
					}
					AssertMsg( (STUDIO_VERSION == 35) || (STUDIO_VERSION == 36) || (STUDIO_VERSION == 37), "Remove the code before this line" );

					float flCheck = flCycle;
					if (flCheck < pRule->start)
							flCheck = flCheck + 1.0f;

					Studio_AnimMovement( panim[i], flCheck, contact, deltaPos, deltaAngle );
					pos = pos + (pRule->pos + deltaPos) * weight[i];
					QuaternionAccumulate( q, weight[i], pRule->q, q );
					height += pRule->height * weight[i];
					floor += pRule->floor * weight[i];
					radius += pRule->radius * weight[i];

					Assert( radius >= 0.0f );

					latched = Studio_IKShouldLatch( *pRule, flCycle );

					break;
				}
			}
		}
	}

	QuaternionNormalize( q );

	if (flWeight == 1.0f)
	{
		m_target[slot].local.q = q;
		m_target[slot].local.pos = pos;
		m_target[slot].est.height = height;
		m_target[slot].est.floor = floor;
		m_target[slot].est.radius = radius;
		m_target[slot].est.latched = latched;
	}
	else
	{
		QuaternionSlerp( m_target[slot].local.q, q, flWeight, m_target[slot].local.q );
		m_target[slot].local.pos = Lerp( flWeight, m_target[slot].local.pos, pos );
		m_target[slot].est.height = Lerp( flWeight, m_target[slot].est.height, height );
		m_target[slot].est.floor = Lerp( flWeight, m_target[slot].est.floor, floor );
		m_target[slot].est.radius = Lerp( flWeight, m_target[slot].est.radius, radius );
		m_target[slot].est.latched = Lerp( flWeight, m_target[slot].est.latched, latched );
	}

	m_target[slot].est.time = m_flTime;

	matrix3x4_t tmp1, tmp2;
	AngleMatrix( m_target[slot].local.q, m_target[slot].local.pos, tmp1 );
	ConcatTransforms( m_rootxform, tmp1, tmp2 );
	MatrixAngles( tmp2, m_target[slot].est.q, m_target[slot].est.pos );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------


CIKContext::CIKContext()
{
	m_target.EnsureCapacity( 12 ); // FIXME: this sucks, shouldn't it be grown?
	m_flPrevTime = -1.0f;
	m_pStudioHdr = NULL;
	m_flTime = -1.0f;
}


void CIKContext::Init( const studiohdr_t *pStudioHdr, const QAngle &angles, const Vector &pos, float flTime )
{
	m_pStudioHdr = pStudioHdr;
	m_ikRule.RemoveAll(); // m_numikrules = 0;
	AngleMatrix( angles, pos, m_rootxform );
	m_flPrevTime = m_flTime;
	m_flTime = flTime;
}

void CIKContext::AddDependencies( 
	int iSequence, 
	float flCycle, 
	const float poseParameters[],
	float flWeight
	)
{
	int i;
	mstudioseqdesc_t *pseqdesc = m_pStudioHdr->pSeqdesc( iSequence );
	mstudioikrule_t ikrule;

	Assert( flWeight >= 0.0f && flWeight <= 1.0f );
	// This shouldn't be necessary, but the Assert should help us catch whoever is screwing this up
	flWeight = clamp( flWeight, 0.0f, 1.0f );

	// Get numikrules - must use v48 for v44+ models
	int numIKRules = StudioSeqdesc_GetNumIKRulesFromPtr( m_pStudioHdr, pseqdesc );

	// FIXME: add proper number of rules!!!
	for (i = 0; i < numIKRules; i++)
	{
		if (Studio_IKSequenceError( m_pStudioHdr, iSequence, flCycle, i, poseParameters, ikrule ))
		{
			// FIXME: Brutal hackery to prevent a crash
			if (m_target.Count() == 0)
			{
				m_target.SetSize(12);
				memset( m_target.Base(), 0, sizeof(m_target[0])*m_target.Count() );
			}

			ikrule.flWeight *= flWeight;
		
			m_ikRule.AddToTail( ikrule );

			// m_target[ikrule.slot].est.flWeight = flWeight; // !!!

			Estimate( iSequence, flCycle, i, poseParameters, flWeight );
		}
	}
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

void CIKContext::AddAutoplayLocks( Vector pos[], Quaternion q[] )
{
	int i;
	static matrix3x4_t boneToWorld[MAXSTUDIOBONES];

	int numIKAutoplayLocks = StudioHdr_GetNumIKAutoplayLocks( m_pStudioHdr );
	for (i = 0; i < numIKAutoplayLocks; i++)
	{
		mstudioiklock_t *plock = StudioHdr_GetIKAutoplayLock( m_pStudioHdr, i );
		if ( !plock )
			continue;
		mstudioikchain_t *pchain = StudioHdr_GetIKChain( m_pStudioHdr, plock->chain );
		if ( !pchain )
			continue;
		int bone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 2 );
		if ( bone < 0 )
			continue;

		mstudioikrule_t ikrule;

		memset( &ikrule, 0, sizeof(ikrule) );
		ikrule.chain = plock->chain;
		ikrule.slot = i;
		ikrule.type = IK_WORLD;

		// eval current ik'd bone
		BuildBoneChain( m_pStudioHdr, m_rootxform, pos, q, bone, boneToWorld );

		MatrixAngles( boneToWorld[bone], ikrule.q, ikrule.pos );

		m_ikRule.AddToTail( ikrule );
	}
	return;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

void CIKContext::AddSequenceLocks( mstudioseqdesc_t *pSeqDesc, Vector pos[], Quaternion q[] )
{
	int i;
	static matrix3x4_t boneToWorld[MAXSTUDIOBONES];

	int numIKLocks = StudioSeqdesc_GetNumIKLocksFromPtr( m_pStudioHdr, pSeqDesc );
	for (i = 0; i < numIKLocks; i++)
	{
		mstudioiklock_t *plock = StudioSeqdesc_GetIKLockFromPtr( m_pStudioHdr, pSeqDesc, i );
		if ( !plock )
			continue;
		mstudioikchain_t *pchain = StudioHdr_GetIKChain( m_pStudioHdr, plock->chain );
		if ( !pchain )
			continue;
		int bone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 2 );
		if ( bone < 0 )
			continue;

		mstudioikrule_t ikrule;
		memset( &ikrule, 0, sizeof(ikrule) );

		ikrule.chain = i;
		ikrule.slot = i;
		ikrule.type = IK_WORLD;

		// eval current ik'd bone
		BuildBoneChain( m_pStudioHdr, m_rootxform, pos, q, bone, boneToWorld );

		MatrixAngles( boneToWorld[bone], ikrule.q, ikrule.pos );

		m_ikRule.AddToTail( ikrule );
	}

	return;
}

//-----------------------------------------------------------------------------
// Purpose: build boneToWorld transforms for a specific bone
//-----------------------------------------------------------------------------
void BuildBoneChain(
	const studiohdr_t *pStudioHdr,
	matrix3x4_t &rootxform,
	Vector pos[],
	Quaternion q[],
	int	iBone,
	matrix3x4_t *pBoneToWorld )
{
	matrix3x4_t bonematrix;

	QuaternionMatrix( q[iBone], pos[iBone], bonematrix );

	// Use version-aware parent accessor for v44+ MDL support
	int parent = StudioBone_GetParent(pStudioHdr, iBone);
	if (parent == -1)
	{
		ConcatTransforms( rootxform, bonematrix, pBoneToWorld[iBone] );
	}
	else
	{
		// evil recursive!!!
		BuildBoneChain( pStudioHdr, rootxform, pos, q, parent, pBoneToWorld );
		ConcatTransforms( pBoneToWorld[parent], bonematrix, pBoneToWorld[iBone]);
	}
}


//-----------------------------------------------------------------------------
// Purpose: turn a specific bones boneToWorld transform into a pos and q in parents bonespace
//-----------------------------------------------------------------------------
void SolveBone(
	const studiohdr_t *pStudioHdr,
	int	iBone,
	matrix3x4_t *pBoneToWorld,
	Vector pos[],
	Quaternion q[]
	)
{
	// Use version-aware parent accessor for v44+ MDL support
	int iParent = StudioBone_GetParent(pStudioHdr, iBone);

	matrix3x4_t worldToBone;
	MatrixInvert( pBoneToWorld[iParent], worldToBone );

	matrix3x4_t local;
	ConcatTransforms( worldToBone, pBoneToWorld[iBone], local );

	MatrixAngles( local, q[iBone], pos[iBone] );
}



void CIKContext::SolveDependencies(
	Vector pos[], 
	Quaternion q[]
	)
{
	ASSERT_NO_REENTRY();
	
	static matrix3x4_t boneToWorld[MAXSTUDIOBONES];
	matrix3x4_t worldTarget;
	int i;

	iktarget_t chainRule[8]; // allocate!!!
	int numIKChains = min( StudioHdr_GetNumIKChains( m_pStudioHdr ), 8 );

	// init chain rules
	for (i = 0; i < numIKChains; i++)
	{
		mstudioikchain_t *pchain = StudioHdr_GetIKChain( m_pStudioHdr, i );
		if ( !pchain )
			continue;
		iktarget_t *pChainRule = &chainRule[ i ];
		int bone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 2 );
		if ( bone < 0 )
			continue;

		pChainRule->est.flWeight = 0.0;

		// eval current ik'd bone
		BuildBoneChain( m_pStudioHdr, m_rootxform, pos, q, bone, boneToWorld );

		MatrixAngles( boneToWorld[bone], pChainRule->local.q, pChainRule->local.pos );
		pChainRule->est.pos = pChainRule->local.pos;
		pChainRule->est.q = pChainRule->local.q;
	}

	for (i = 0; i < m_ikRule.Count(); i++)
	{
		if (m_ikRule[i].chain < 0 || m_ikRule[i].chain >= numIKChains)
			continue;
		iktarget_t *pChainRule = &chainRule[ m_ikRule[i].chain ];

		switch( m_ikRule[i].type )
		{
		case IK_SELF:
			{
				// eval target bone space
				BuildBoneChain( m_pStudioHdr, m_rootxform, pos, q, m_ikRule[i].bone, boneToWorld );

				// xform IK target error into world space
				matrix3x4_t local;
				QuaternionMatrix( m_ikRule[i].q, m_ikRule[i].pos, local );
				ConcatTransforms( boneToWorld[m_ikRule[i].bone], local, worldTarget );
			}
			break;
		case IK_WORLD:
			{
				QuaternionMatrix( m_ikRule[i].q, m_ikRule[i].pos, worldTarget );
			}	
			break;
		case IK_GROUND:
			{
				matrix3x4_t footTarget;
				int slot = m_ikRule[i].slot;

				bool bIsLatched = (m_target[slot].est.latched == 1.0);
				bool bLatchAvail = (bIsLatched && m_target[slot].latched.time >= m_flPrevTime && m_flPrevTime >= m_flTime - 0.1);

				if (bIsLatched && bLatchAvail)
				{
					// use latched goal
					AngleMatrix( m_target[slot].latched.q, m_target[slot].latched.pos, footTarget );
				}
				else
				{
					// latch off current goal
					m_target[slot].latched.q = m_target[slot].est.q;
					m_target[slot].latched.pos = m_target[slot].est.pos;
					// use existing position
					AngleMatrix( m_target[slot].est.q, m_target[slot].est.pos, footTarget );
				}

				// xform IK target error into world space
				matrix3x4_t local;
				QuaternionMatrix( m_ikRule[i].q, m_ikRule[i].pos, local );
				ConcatTransforms( footTarget, local, worldTarget );

				if (bIsLatched)
				{
					m_target[slot].latched.time = m_flTime;
				}
			}
			break;
		case IK_RELEASE:
			{
				pChainRule->est.flWeight = pChainRule->est.flWeight * (1 - m_ikRule[i].flWeight) + m_ikRule[i].flWeight;

				pChainRule->est.pos = pChainRule->est.pos * (1.0 - m_ikRule[i].flWeight ) + pChainRule->local.pos * m_ikRule[i].flWeight;
				QuaternionSlerp( pChainRule->est.q, pChainRule->local.q, m_ikRule[i].flWeight, pChainRule->est.q );

				continue;
			}
			break;
		}

		pChainRule->est.flWeight = pChainRule->est.flWeight * (1 - m_ikRule[i].flWeight) + m_ikRule[i].flWeight;

		Vector p2;
		Quaternion q2;
		
		// target p and q
		MatrixAngles( worldTarget, q2, p2 );

		// blend in position and angles
		pChainRule->est.pos = pChainRule->est.pos * (1.0 - m_ikRule[i].flWeight ) + p2 * m_ikRule[i].flWeight;
		QuaternionSlerp( pChainRule->est.q, q2, m_ikRule[i].flWeight, pChainRule->est.q );
	}

	for (i = 0; i < numIKChains; i++)
	{
		iktarget_t *pChainRule = &chainRule[ i ];
		mstudioikchain_t *pchain = StudioHdr_GetIKChain( m_pStudioHdr, i );

		if (pChainRule->est.flWeight > 0.0)
		{
			int thighBone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 0 );
			int kneeBone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 1 );
			int footBone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 2 );
			if ( thighBone < 0 || kneeBone < 0 || footBone < 0 )
				continue;

			// do exact IK solution
			// FIXME: once per link!
			Studio_SolveIK( thighBone, kneeBone, footBone, pChainRule->est.pos, boneToWorld );

			Vector p3;
			MatrixGetColumn( boneToWorld[footBone], 3, p3 );
			QuaternionMatrix( pChainRule->est.q, p3, boneToWorld[footBone] );

			// rebuild chain
			SolveBone( m_pStudioHdr, footBone, boneToWorld, pos, q );
			SolveBone( m_pStudioHdr, kneeBone, boneToWorld, pos, q );
			SolveBone( m_pStudioHdr, thighBone, boneToWorld, pos, q );
		}
	}


#if 0
		Vector p1, p2, p3;
		Quaternion q1, q2, q3;

		// current p and q
		MatrixAngles( boneToWorld[bone], q1, p1 );

		
		// target p and q
		MatrixAngles( worldTarget, q2, p2 );

		// blend in position and angles
		p3 = p1 * (1.0 - m_ikRule[i].flWeight ) + p2 * m_ikRule[i].flWeight;

		// do exact IK solution
		// FIXME: once per link!
		Studio_SolveIK(pchain, p3, boneToWorld );

		// force angle (bad?)
		QuaternionSlerp( q1, q2, m_ikRule[i].flWeight, q3 );
		MatrixGetColumn( boneToWorld[bone], 3, p3 );
		QuaternionMatrix( q3, p3, boneToWorld[bone] );

		// rebuild chain
		SolveBone( m_pStudioHdr, pchain->pLink( 2 )->bone, boneToWorld, pos, q );
		SolveBone( m_pStudioHdr, pchain->pLink( 1 )->bone, boneToWorld, pos, q );
		SolveBone( m_pStudioHdr, pchain->pLink( 0 )->bone, boneToWorld, pos, q );
#endif
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

void CIKContext::SolveAutoplayLocks(
	Vector pos[],
	Quaternion q[]
	)
{
	ASSERT_NO_REENTRY();

	static matrix3x4_t boneToWorld[MAXSTUDIOBONES];
	int i;

	for (i = 0; i < m_ikRule.Count(); i++)
	{
		mstudioiklock_t *plock = StudioHdr_GetIKAutoplayLock( m_pStudioHdr, i );
		if ( !plock )
			continue;
		mstudioikchain_t *pchain = StudioHdr_GetIKChain( m_pStudioHdr, plock->chain );
		if ( !pchain )
			continue;
		int thighBone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 0 );
		int kneeBone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 1 );
		int bone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 2 );
		if ( thighBone < 0 || kneeBone < 0 || bone < 0 )
			continue;

		// eval current ik'd bone
		BuildBoneChain( m_pStudioHdr, m_rootxform, pos, q, bone, boneToWorld );

		Vector p1, p2, p3;
		Quaternion q1, q2, q3;

		// current p and q
		MatrixAngles( boneToWorld[bone], q1, p1 );

		// blend in position
		p3 = p1 * (1.0 - plock->flPosWeight ) + m_ikRule[i].pos * plock->flPosWeight;

		// do exact IK solution
		Studio_SolveIK( thighBone, kneeBone, bone, p3, boneToWorld );

		// slam orientation
		MatrixGetColumn( boneToWorld[bone], 3, p3 );
		QuaternionMatrix( m_ikRule[i].q, p3, boneToWorld[bone] );

		// rebuild chain
		q2 = q[ bone ];
		SolveBone( m_pStudioHdr, bone, boneToWorld, pos, q );
		QuaternionSlerp( q[bone], q2, plock->flLocalQWeight, q[bone] );

		SolveBone( m_pStudioHdr, kneeBone, boneToWorld, pos, q );
		SolveBone( m_pStudioHdr, thighBone, boneToWorld, pos, q );
	}
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

void CIKContext::SolveSequenceLocks(
	mstudioseqdesc_t *pSeqDesc,
	Vector pos[],
	Quaternion q[]
	)
{
	ASSERT_NO_REENTRY();

	static matrix3x4_t boneToWorld[MAXSTUDIOBONES];
	int i;

	for (i = 0; i < m_ikRule.Count(); i++)
	{
		mstudioiklock_t *plock = StudioSeqdesc_GetIKLockFromPtr( m_pStudioHdr, pSeqDesc, i );
		if ( !plock )
			continue;
		mstudioikchain_t *pchain = StudioHdr_GetIKChain( m_pStudioHdr, plock->chain );
		if ( !pchain )
			continue;
		int thighBone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 0 );
		int kneeBone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 1 );
		int bone = StudioIKChain_GetLinkBone( m_pStudioHdr, pchain, 2 );
		if ( thighBone < 0 || kneeBone < 0 || bone < 0 )
			continue;

		// eval current ik'd bone
		BuildBoneChain( m_pStudioHdr, m_rootxform, pos, q, bone, boneToWorld );

		Vector p1, p2, p3;
		Quaternion q1, q2, q3;

		// current p and q
		MatrixAngles( boneToWorld[bone], q1, p1 );

		// blend in position
		p3 = p1 * (1.0 - plock->flPosWeight ) + m_ikRule[i].pos * plock->flPosWeight;

		// do exact IK solution
		Studio_SolveIK( thighBone, kneeBone, bone, p3, boneToWorld );

		// slam orientation
		MatrixGetColumn( boneToWorld[bone], 3, p3 );
		QuaternionMatrix( m_ikRule[i].q, p3, boneToWorld[bone] );

		// rebuild chain
		q2 = q[ bone ];
		SolveBone( m_pStudioHdr, bone, boneToWorld, pos, q );
		QuaternionSlerp( q[bone], q2, plock->flLocalQWeight, q[bone] );

		SolveBone( m_pStudioHdr, kneeBone, boneToWorld, pos, q );
		SolveBone( m_pStudioHdr, thighBone, boneToWorld, pos, q );
	}
}

//-----------------------------------------------------------------------------
// Purpose: run all animations that automatically play and are driven off of poseParameters
//-----------------------------------------------------------------------------
void CalcAutoplaySequences(
	const studiohdr_t *pStudioHdr,
	CIKContext *pIKContext,
	Vector pos[], 
	Quaternion q[], 
	const float poseParameters[],
	int boneMask,
	float time
	)
{
	ASSERT_NO_REENTRY();
	
	int			i;

	if ( pIKContext )
	{
		pIKContext->AddAutoplayLocks( pos, q );
	}

	// CRITICAL: Use version-aware accessor for sequence count
	int numSeqs = pStudioHdr->GetNumLocalSeq();
	for (i = 0; i < numSeqs; i++)
	{
		mstudioseqdesc_t *pseqdesc = pStudioHdr->pSeqdesc( i );

		int seqFlags = StudioSeqdesc_GetFlags( pStudioHdr, i );

		if (seqFlags & STUDIO_AUTOPLAY)
		{
			float cycle = 0;
			float cps = Studio_CPS( pStudioHdr, i, poseParameters );
			cycle = time * cps;
			cycle = cycle - (int)cycle;

			AccumulatePose( pStudioHdr, NULL, pos, q, i, cycle, poseParameters, boneMask );
		}
	}

	if ( pIKContext )
	{
		pIKContext->SolveAutoplayLocks( pos, q );
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Studio_BuildMatrices(
	const studiohdr_t *pStudioHdr,
	const QAngle& angles,
	const Vector& origin,
	const Vector pos[],
	const Quaternion q[],
	int iBone,
	matrix3x4_t bonetoworld[MAXSTUDIOBONES],
	int boneMask
	)
{
	int i, j;

	int					chain[MAXSTUDIOBONES];
	int					chainlength = 0;

	// Use version-aware bone count for v44+ support
	int numBones = StudioHdr_GetNumBones(pStudioHdr);

	if (iBone < -1 || iBone >= numBones)
		iBone = 0;

	// build list of what bones to use
	if (iBone == -1)
	{
		// all bones
		chainlength = numBones;
		for (i = 0; i < numBones; i++)
		{
			chain[chainlength - i - 1] = i;
		}
	}
	else
	{
		// only the parent bones
		// Use version-aware parent accessor for correct bone stride
		i = iBone;
		while (i != -1)
		{
			chain[chainlength++] = i;
			i = StudioBone_GetParent(pStudioHdr, i);
		}
	}

	matrix3x4_t bonematrix;
	matrix3x4_t rotationmatrix; // model to world transformation
	AngleMatrix( angles, origin, rotationmatrix);

	for (j = chainlength - 1; j >= 0; j--)
	{
		i = chain[j];
		// Use version-aware flag accessor for correct bone stride and field offset
		if (StudioBone_GetFlags(pStudioHdr, i) & boneMask)
		{
			QuaternionMatrix( q[i], pos[i], bonematrix );

			int parentBone = StudioBone_GetParent(pStudioHdr, i);
			if (parentBone == -1)
			{
				ConcatTransforms (rotationmatrix, bonematrix, bonetoworld[i]);
			}
			else
			{
				ConcatTransforms (bonetoworld[parentBone], bonematrix, bonetoworld[i]);
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: look at single column vector of another bones local transformation
//			and generate a procedural transformation based on how that column
//			points down the 6 cardinal axis (all negative weights are clamped to 0).
// Version-aware: Works with both v37 and v44+ models
//-----------------------------------------------------------------------------

void DoAxisInterpBone(
	const studiohdr_t	*pStudioHdr,
	int	ibone,
	matrix3x4_t *bonetoworld
	)
{
	matrix3x4_t			bonematrix;
	Vector				control;

	// Use version-aware accessor for procedural bone data
	mstudioaxisinterpbone_t *pProc = (mstudioaxisinterpbone_t *)StudioBone_GetProcedure(pStudioHdr, ibone);
	if (!pProc)
		return;

	// Get control bone's parent using version-aware accessor
	int iControlParent = StudioBone_GetParent(pStudioHdr, pProc->control);
	if (iControlParent != -1)
	{
		Vector tmp;
		// pull out the control column
		tmp.x = bonetoworld[pProc->control][0][pProc->axis];
		tmp.y = bonetoworld[pProc->control][1][pProc->axis];
		tmp.z = bonetoworld[pProc->control][2][pProc->axis];

		// invert it back into parent's space.
		VectorIRotate( tmp, bonetoworld[iControlParent], control );
#if 0
		matrix3x4_t	tmpmatrix;
		matrix3x4_t	controlmatrix;
		MatrixInvert( bonetoworld[iControlParent], tmpmatrix );
		ConcatTransforms( tmpmatrix, bonetoworld[pProc->control], controlmatrix );

		// pull out the control column
		control.x = controlmatrix[0][pProc->axis];
		control.y = controlmatrix[1][pProc->axis];
		control.z = controlmatrix[2][pProc->axis];
#endif
	}
	else
	{
		// pull out the control column
		control.x = bonetoworld[pProc->control][0][pProc->axis];
		control.y = bonetoworld[pProc->control][1][pProc->axis];
		control.z = bonetoworld[pProc->control][2][pProc->axis];
	}

	Quaternion *q1, *q2, *q3;
	Vector *p1, *p2, *p3;

	// find axial control inputs
	float a1 = control.x;
	float a2 = control.y;
	float a3 = control.z;
	if (a1 >= 0) 
	{ 
		q1 = &pProc->quat[0];
		p1 = &pProc->pos[0];
	} 
	else 
	{ 
		a1 = -a1; 
		q1 = &pProc->quat[1];
		p1 = &pProc->pos[1];
	}

	if (a2 >= 0) 
	{ 
		q2 = &pProc->quat[2]; 
		p2 = &pProc->pos[2];
	} 
	else 
	{ 
		a2 = -a2; 
		q2 = &pProc->quat[3]; 
		p2 = &pProc->pos[3];
	}

	if (a3 >= 0) 
	{ 
		q3 = &pProc->quat[4]; 
		p3 = &pProc->pos[4];
	} 
	else 
	{ 
		a3 = -a3; 
		q3 = &pProc->quat[5]; 
		p3 = &pProc->pos[5];
	}

	// do a three-way blend
	Vector p;
	Quaternion v, tmp;
	if (a1 + a2 > 0)
	{
		float t = 1.0 / (a1 + a2 + a3);
		// FIXME: do a proper 3-way Quat blend!
		QuaternionSlerp( *q2, *q1, a1 / (a1 + a2), tmp );
		QuaternionSlerp( tmp, *q3, a3 * t, v );
		VectorScale( *p1, a1 * t, p );
		VectorMA( p, a2 * t, *p2, p );
		VectorMA( p, a3 * t, *p3, p );
	}
	else
	{
		QuaternionSlerp( *q3, *q3, 0, v ); // ??? no quat copy?
		p = *p3;
	}

	QuaternionMatrix( v, p, bonematrix );

	// Get parent bone index using version-aware accessor
	int iBoneParent = StudioBone_GetParent(pStudioHdr, ibone);
	ConcatTransforms (bonetoworld[iBoneParent], bonematrix, bonetoworld[ibone]);
}



//-----------------------------------------------------------------------------
// Purpose: Generate a procedural transformation based on how that another bones
//			local transformation matches a set of target orientations.
// Version-aware: Works with both v37 and v44+ models
//-----------------------------------------------------------------------------
void DoQuatInterpBone(
	const studiohdr_t	*pStudioHdr,
	int	ibone,
	matrix3x4_t *bonetoworld
	)
{
	matrix3x4_t			bonematrix;
	Vector				control;

	// Use version-aware accessor for procedural bone data
	mstudioquatinterpbone_t *pProc = (mstudioquatinterpbone_t *)StudioBone_GetProcedure(pStudioHdr, ibone);
	if (!pProc)
		return;

	// Get control bone's parent using version-aware accessor
	int iControlParent = StudioBone_GetParent(pStudioHdr, pProc->control);
	// Get this bone's parent using version-aware accessor
	int iBoneParent = StudioBone_GetParent(pStudioHdr, ibone);

	if (iControlParent != -1)
	{
		Quaternion	src;
		float		weight[32];
		float		scale = 0.0;
		Quaternion	quat;
		Vector		pos;

		matrix3x4_t	tmpmatrix;
		matrix3x4_t	controlmatrix;
		MatrixInvert( bonetoworld[iControlParent], tmpmatrix );
		ConcatTransforms( tmpmatrix, bonetoworld[pProc->control], controlmatrix );

		MatrixAngles( controlmatrix, src, pos ); // FIXME: make a version without pos

		int i;
		for (i = 0; i < pProc->numtriggers; i++)
		{
			float dot = fabs( QuaternionDotProduct( pProc->pTrigger( i )->trigger, src ) );
			// FIXME: a fast acos should be acceptable
			weight[i] = 1 - (2 * acos( dot ) * pProc->pTrigger( i )->inv_tolerance );
			weight[i] = max( 0.f, weight[i] );
			scale += weight[i];
		}

		if (scale <= 0.001)  // EPSILON?
		{
			AngleMatrix( pProc->pTrigger( 0 )->quat, pProc->pTrigger( 0 )->pos, bonematrix );
			ConcatTransforms (bonetoworld[iBoneParent], bonematrix, bonetoworld[ibone]);
			return;
		}

		scale = 1.0 / scale;

		quat.Init( 0, 0, 0, 0);
		pos.Init( );

		for (i = 0; i < pProc->numtriggers; i++)
		{
			if (weight[i])
			{
				float s = weight[i] * scale;
				mstudioquatinterpinfo_t *pTrigger = pProc->pTrigger( i );

				QuaternionAlign( pTrigger->quat, quat, quat );

				quat.x = quat.x + s * pTrigger->quat.x;
				quat.y = quat.y + s * pTrigger->quat.y;
				quat.z = quat.z + s * pTrigger->quat.z;
				quat.w = quat.w + s * pTrigger->quat.w;
				pos = pos + s * pTrigger->pos;
			}
		}
		Assert( QuaternionNormalize( quat ) != 0);
		QuaternionMatrix( quat, pos, bonematrix );
	}

	ConcatTransforms (bonetoworld[iBoneParent], bonematrix, bonetoworld[ibone]);
}


//-----------------------------------------------------------------------------
// v48: Aim-at bone procedural
// Makes a bone aim at another bone or attachment point
// Version-aware: Works with both v37 and v44+ models
//-----------------------------------------------------------------------------
void DoAimAtBone(
	const studiohdr_t	*pStudioHdr,
	int ibone,
	matrix3x4_t *bonetoworld,
	int proctype
	)
{
	// Use version-aware accessor for procedural bone data
	mstudioaimatbone_t *pProc = (mstudioaimatbone_t *)StudioBone_GetProcedure(pStudioHdr, ibone);
	if (!pProc)
		return;

	int iParent = pProc->parent;
	int iAim = pProc->aim;

	// Validate indices
	if (iParent < 0 || iAim < 0)
		return;

	// Get target position (either bone or attachment depending on proctype)
	Vector vTargetPos;
	if (proctype == STUDIO_PROC_AIMATATTACH)
	{
		// STUDIO_PROC_AIMATATTACH - aim at an attachment
		// For now, treat same as bone (attachment handling would need more context)
		vTargetPos.x = bonetoworld[iAim][0][3];
		vTargetPos.y = bonetoworld[iAim][1][3];
		vTargetPos.z = bonetoworld[iAim][2][3];
	}
	else
	{
		// STUDIO_PROC_AIMATBONE - aim at another bone
		vTargetPos.x = bonetoworld[iAim][0][3];
		vTargetPos.y = bonetoworld[iAim][1][3];
		vTargetPos.z = bonetoworld[iAim][2][3];
	}

	// Get current position from base
	Vector vCurrentPos = pProc->basepos;

	// Calculate aim direction
	Vector vAimDir;
	VectorSubtract(vTargetPos, vCurrentPos, vAimDir);
	VectorNormalize(vAimDir);

	// Build rotation matrix to aim at target
	// This is a simplified implementation - full Source engine uses more complex aim logic
	Vector vUp = pProc->upvector;
	Vector vRight;
	CrossProduct(vUp, vAimDir, vRight);
	VectorNormalize(vRight);
	CrossProduct(vAimDir, vRight, vUp);

	matrix3x4_t bonematrix;
	bonematrix[0][0] = vAimDir.x;  bonematrix[0][1] = vRight.x;  bonematrix[0][2] = vUp.x;  bonematrix[0][3] = vCurrentPos.x;
	bonematrix[1][0] = vAimDir.y;  bonematrix[1][1] = vRight.y;  bonematrix[1][2] = vUp.y;  bonematrix[1][3] = vCurrentPos.y;
	bonematrix[2][0] = vAimDir.z;  bonematrix[2][1] = vRight.z;  bonematrix[2][2] = vUp.z;  bonematrix[2][3] = vCurrentPos.z;

	// Get parent bone index using version-aware accessor
	int iBoneParent = StudioBone_GetParent(pStudioHdr, ibone);
	if (iBoneParent >= 0)
	{
		ConcatTransforms(bonetoworld[iBoneParent], bonematrix, bonetoworld[ibone]);
	}
	else
	{
		MatrixCopy(bonematrix, bonetoworld[ibone]);
	}
}

//-----------------------------------------------------------------------------
// v48: Jiggle bone procedural
// Simulates physics-based bone jiggling (simplified implementation)
// Full implementation would require per-bone state tracking between frames
// Version-aware: Works with both v37 and v44+ models
//-----------------------------------------------------------------------------
void DoJiggleBone(
	const studiohdr_t	*pStudioHdr,
	int ibone,
	matrix3x4_t *bonetoworld
	)
{
	// Use version-aware accessor for procedural bone data
	mstudiojigglebone_t *pProc = (mstudiojigglebone_t *)StudioBone_GetProcedure(pStudioHdr, ibone);
	if (!pProc)
		return;

	// Jiggle bones require frame-to-frame state that we don't have access to here
	// This is a simplified implementation that just uses the base pose
	// Full implementation would need:
	// - Previous frame bone positions
	// - Velocity/acceleration tracking
	// - Spring/damping physics simulation

	// Get parent bone index using version-aware accessor
	int iParent = StudioBone_GetParent(pStudioHdr, ibone);

	// For now, just set up the bone in its default position
	// The parent transform is already applied, so we just need the local offset
	if (iParent >= 0)
	{
		// Get bone's default local transform from poseToBone (version-aware)
		const matrix3x4_t *pPoseToBone = StudioBone_GetPoseToBonePtr(pStudioHdr, ibone);
		if (!pPoseToBone)
			return;

		matrix3x4_t bonematrix;
		MatrixCopy(*pPoseToBone, bonematrix);

		// Apply a small procedural offset based on jiggle length
		// This is a placeholder - real jiggle needs physics simulation
		float tipOffset = pProc->length;
		bonematrix[0][3] += tipOffset * 0.01f; // Tiny offset along bone

		ConcatTransforms(bonetoworld[iParent], bonematrix, bonetoworld[ibone]);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Calculate procedural bone transformations
// Supports both v37 bone types (AxisInterp, QuatInterp) and v48 types
// (AimAt, Jiggle). Version-aware for v37 and v44+ models.
//-----------------------------------------------------------------------------

bool CalcProceduralBone(
	const studiohdr_t *pStudioHdr,
	int iBone,
	matrix3x4_t *bonetoworld
	)
{
	// Use version-aware flag accessor for v44+ support
	if ( StudioBone_GetFlags(pStudioHdr, iBone) & BONE_ALWAYS_PROCEDURAL )
	{
		int proctype = StudioBone_GetProcType(pStudioHdr, iBone);

		switch( proctype )
		{
		case STUDIO_PROC_AXISINTERP:
			DoAxisInterpBone( pStudioHdr, iBone, bonetoworld );
			return true;

		case STUDIO_PROC_QUATINTERP:
			DoQuatInterpBone( pStudioHdr, iBone, bonetoworld );
			return true;

		//---------------------------------------------------------------------
		// v48 procedural bone types
		//---------------------------------------------------------------------
		case STUDIO_PROC_AIMATBONE:
			DoAimAtBone( pStudioHdr, iBone, bonetoworld, STUDIO_PROC_AIMATBONE );
			return true;

		case STUDIO_PROC_AIMATATTACH:
			DoAimAtBone( pStudioHdr, iBone, bonetoworld, STUDIO_PROC_AIMATATTACH );
			return true;

		case STUDIO_PROC_JIGGLE:
			DoJiggleBone( pStudioHdr, iBone, bonetoworld );
			return true;

		default:
			return false;
		}
	}
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: converts a ranged bone controller value into a 0..1 encoded value
// Output: 	ctlValue contains 0..1 encoding.
//			returns clamped ranged value
//-----------------------------------------------------------------------------

float Studio_SetController( const studiohdr_t *pStudioHdr, int iController, float flValue, float &ctlValue )
{
	if (! pStudioHdr)
		return flValue;

	mstudiobonecontroller_t *pbonecontroller = FindController(pStudioHdr, iController);
	if(!pbonecontroller)
	{
		ctlValue = 0;
		return flValue;
	}

	// wrap 0..360 if it's a rotational controller
	if (pbonecontroller->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pbonecontroller->end < pbonecontroller->start)
			flValue = -flValue;

		// does the controller not wrap?
		if (pbonecontroller->start + 359.0 >= pbonecontroller->end)
		{
			if (flValue > ((pbonecontroller->start + pbonecontroller->end) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pbonecontroller->start + pbonecontroller->end) / 2.0) - 180)
				flValue = flValue + 360;
		}
		else
		{
			if (flValue > 360)
				flValue = flValue - (int)(flValue / 360.0) * 360.0;
			else if (flValue < 0)
				flValue = flValue + (int)((flValue / -360.0) + 1) * 360.0;
		}
	}

	ctlValue = (flValue - pbonecontroller->start) / (pbonecontroller->end - pbonecontroller->start);
	if (ctlValue < 0) ctlValue = 0;
	if (ctlValue > 1) ctlValue = 1;

	float flReturnVal = ((1.0 - ctlValue)*pbonecontroller->start + ctlValue *pbonecontroller->end);

	// ugly hack, invert value if a rotational controller and end < start
	if (pbonecontroller->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR) &&
		pbonecontroller->end < pbonecontroller->start				)
	{
		flReturnVal *= -1;
	}
	
	return flReturnVal;
}


//-----------------------------------------------------------------------------
// Purpose: converts a 0..1 encoded bone controller value into a ranged value
// Output: 	returns ranged value
//-----------------------------------------------------------------------------

float Studio_GetController( const studiohdr_t *pStudioHdr, int iController, float ctlValue )
{
	if (!pStudioHdr)
		return 0.0;

	mstudiobonecontroller_t *pbonecontroller = FindController(pStudioHdr, iController);
	if(!pbonecontroller)
		return 0;

	return ctlValue * (pbonecontroller->end - pbonecontroller->start) + pbonecontroller->start;
}


//-----------------------------------------------------------------------------
// Purpose: converts a ranged pose parameter value into a 0..1 encoded value
// Output: 	ctlValue contains 0..1 encoding.
//			returns clamped ranged value
//-----------------------------------------------------------------------------

float Studio_SetPoseParameter( const studiohdr_t *pStudioHdr, int iParameter, float flValue, float &ctlValue )
{
	// Use version-aware helper for v37/v44+ compatibility
	int numPoseParams = StudioHdr_GetNumPoseParameters(pStudioHdr);
	if (iParameter < 0 || iParameter >= numPoseParams)
	{
		return 0;
	}

	mstudioposeparamdesc_t *pPoseParam = StudioHdr_GetPoseParameter(pStudioHdr, iParameter);

	if (pPoseParam->loop)
	{
		float wrap = (pPoseParam->start + pPoseParam->end) / 2.0 + pPoseParam->loop / 2.0;
		float shift = pPoseParam->loop - wrap;

		flValue = flValue - pPoseParam->loop * floor((flValue + shift) / pPoseParam->loop);
	}

	ctlValue = (flValue - pPoseParam->start) / (pPoseParam->end - pPoseParam->start);

	if (ctlValue < 0) ctlValue = 0;
	if (ctlValue > 1) ctlValue = 1;

	return ctlValue * (pPoseParam->end - pPoseParam->start) + pPoseParam->start;
}


//-----------------------------------------------------------------------------
// Purpose: converts a 0..1 encoded pose parameter value into a ranged value
// Output: 	returns ranged value
//-----------------------------------------------------------------------------

float Studio_GetPoseParameter( const studiohdr_t *pStudioHdr, int iParameter, float ctlValue )
{
	// Use version-aware helper for v37/v44+ compatibility
	int numPoseParams = StudioHdr_GetNumPoseParameters(pStudioHdr);
	if (iParameter < 0 || iParameter >= numPoseParams)
	{
		return 0;
	}

	mstudioposeparamdesc_t *pPoseParam = StudioHdr_GetPoseParameter(pStudioHdr, iParameter);

	return ctlValue * (pPoseParam->end - pPoseParam->start) + pPoseParam->start;
}

// Hit Testing code:
// 
//
struct bonecache_t
{
	int				bone;
	matrix3x4_t		matrix;
	bonecache_t		*pnext;
};

struct studiocache_t
{
	studiohdr_t		*pStudioHdr;
	int				sequence;
	float			animtime;
	QAngle			angles;
	Vector			origin;
	int				boneMask;
	bonecache_t		*bones;	// points to a linked list of matrix transforms for each bone
	
	// FIXME:  Cache controllers and poseparameters, too???
//	float			controllers[ MAXSTUDIOBONECTRLS ];
//	float			poseparam[ MAXSTUDIOPOSEPARAM ];
};

//-----------------------------------------------------------------------------
// Purpose: Singleton static object which sets up the studio bone cache
//-----------------------------------------------------------------------------
class CStudioBoneCache
{
public:
	// must be power of 2 or change the cache code
	enum
	{
		BONE_CACHE_SIZE = 512,
		MODEL_CACHE_SIZE = 16,
	};

	CStudioBoneCache()
	{
		studiomodelstart = 0;

		bonefreelist = studiobonecache;

		memset( studiobonecache, 0, sizeof(studiobonecache) );

		memset( studiomodelcache, 0, sizeof(studiomodelcache) );

		for ( int i = 0; i < BONE_CACHE_SIZE; i++ )
		{
			studiobonecache[i].pnext = &studiobonecache[i+1];
		}

		studiobonecache[BONE_CACHE_SIZE-1].pnext = NULL;
	}

	inline void BoneCacheFree( studiocache_t *pcache );
	inline void BoneCacheFreeLRU( void );
	matrix3x4_t *Studio_LookupCachedBone( studiocache_t *pCache, int iBone );
	void Studio_LinkHitboxCache( matrix3x4_t **bones, studiocache_t *pcache, studiohdr_t *pStudioHdr, mstudiohitboxset_t *set );
	studiocache_t *Studio_GetBoneCache( studiohdr_t *pStudioHdr, int sequence, float animtime, const QAngle& angles, const Vector& origin, int boneMask );
	studiocache_t *Studio_SetBoneCache( studiohdr_t *pStudioHdr, int sequence, float animtime, const QAngle& angles, const Vector& origin, int boneMask, matrix3x4_t *bonetoworld );

private:

	bonecache_t *bonefreelist;
	bonecache_t	studiobonecache[BONE_CACHE_SIZE];
	studiocache_t studiomodelcache[MODEL_CACHE_SIZE];
	int studiomodelstart;
};

// Construct a singleton
static CStudioBoneCache g_StudioBoneCache;

void CStudioBoneCache::BoneCacheFree( studiocache_t *pcache )
{
	bonecache_t *pbone = pcache->bones;
	bonecache_t *pnext;

	while ( pbone )
	{
		pnext = pbone->pnext;
		pbone->pnext = bonefreelist;
		bonefreelist = pbone;
		pbone = pnext;
	}
	pcache->pStudioHdr = NULL;
	pcache->bones = NULL;
}

void CStudioBoneCache::BoneCacheFreeLRU( void )
{
	for ( int i = studiomodelstart+1; i != studiomodelstart; i++ )
	{
		i &= (MODEL_CACHE_SIZE-1);
		BoneCacheFree( studiomodelcache + i );
		
		if ( bonefreelist )
			return;
	}
}

matrix3x4_t *CStudioBoneCache::Studio_LookupCachedBone( studiocache_t *pCache, int iBone )
{
	bonecache_t *pbone = pCache->bones;

	// FIXME: linear search.  This done much?
	while ( pbone )
	{
		if (pbone->bone == iBone)
			return &pbone->matrix;
		if (pbone->bone > iBone)
			return NULL;
		pbone = pbone->pnext;
	}
	return NULL;
}

void CStudioBoneCache::Studio_LinkHitboxCache( matrix3x4_t **bones, studiocache_t *pcache, studiohdr_t *pStudioHdr, mstudiohitboxset_t *set )
{
	int numHitboxes = StudioHitboxSet_GetNumHitboxesFromPtr( pStudioHdr, set );
	for ( int i = 0; i < numHitboxes; i++ )
	{
		int bone = StudioHitbox_GetBone( pStudioHdr, set, i );
		bones[i] = bone >= 0 ? Studio_LookupCachedBone( pcache, bone ) : NULL;
		Assert(bones[i]);
	}
}

studiocache_t *CStudioBoneCache::Studio_GetBoneCache( studiohdr_t *pStudioHdr, int sequence, float animtime, const QAngle& angles, const Vector& origin, int boneMask )
{
	// check for a cache hit
	int i = studiomodelstart;
	do
	{
		studiocache_t *pcache = studiomodelcache + i;
		if ( pcache->pStudioHdr == pStudioHdr &&
			pcache->sequence == sequence &&
			pcache->animtime == animtime &&
			pcache->angles == angles &&
			pcache->origin == origin && 
			(pcache->boneMask & boneMask) == boneMask )
		{
			return pcache;
		}
		i = (i-1)&(MODEL_CACHE_SIZE-1);
	} while ( i != studiomodelstart );

	return NULL;
}

studiocache_t *CStudioBoneCache::Studio_SetBoneCache( studiohdr_t *pStudioHdr, int sequence, float animtime, const QAngle& angles, const Vector& origin, int boneMask, matrix3x4_t *bonetoworld )
{
	// Get the LRU model entry and reuse it
	studiomodelstart = (studiomodelstart+1)&(MODEL_CACHE_SIZE-1);
	studiocache_t *pcache = studiomodelcache + studiomodelstart;
	BoneCacheFree( pcache );

	pcache->pStudioHdr = pStudioHdr;
	pcache->sequence = sequence;
	pcache->animtime = animtime;
	pcache->angles = angles;
	pcache->origin = origin;
	pcache->boneMask = boneMask;
	pcache->bones = NULL;

	bonecache_t *plast = NULL;
	int numBones = StudioHdr_GetNumBones(pStudioHdr);
	for ( int i = 0; i < numBones; i++ )
	{
		// 
		// Use version-aware flag accessor for v44+ support
		if (!(StudioBone_GetFlags(pStudioHdr, i) & boneMask))
		{
			// FIXME: temporary hack, if no flags set, assume it's a old version
			if (StudioBone_GetFlags(pStudioHdr, 0) & BONE_USED_MASK)
			{
				continue;
			}
		}

		// make sure there's a free bone for this box
		if ( !bonefreelist )
		{
			BoneCacheFreeLRU();
		}
		bonecache_t *pbone = bonefreelist;
		bonefreelist = bonefreelist->pnext;
		if ( pcache->bones == NULL )
		{
			pcache->bones = pbone;
		}
		else
		{
			plast->pnext = pbone;
		}

		pbone->bone = i;
		MatrixCopy( bonetoworld[i], pbone->matrix ); 
		plast = pbone;
	}
	plast->pnext = NULL;
	return pcache;
}


matrix3x4_t *Studio_LookupCachedBone( studiocache_t *pCache, int iBone )
{
	return g_StudioBoneCache.Studio_LookupCachedBone( pCache, iBone );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : **bones - 
//			*pcache - 
//			*pStudioHdr - 
//			*set - 
//-----------------------------------------------------------------------------
void Studio_LinkHitboxCache( matrix3x4_t **bones, studiocache_t *pcache, studiohdr_t *pStudioHdr, mstudiohitboxset_t *set )
{
	g_StudioBoneCache.Studio_LinkHitboxCache( bones, pcache, pStudioHdr, set );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pStudioHdr - 
//			sequence - 
//			cycle - 
//			angles - 
//			origin - 
//			boneMask - 
// Output : studiocache_t
//-----------------------------------------------------------------------------
studiocache_t *Studio_GetBoneCache( studiohdr_t *pStudioHdr, int sequence, float animtime, const QAngle& angles, const Vector& origin, int boneMask )
{
	return g_StudioBoneCache.Studio_GetBoneCache( pStudioHdr, sequence, animtime, angles, origin, boneMask );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
studiocache_t *Studio_SetBoneCache( studiohdr_t *pStudioHdr, int sequence, float animtime, const QAngle& angles, const Vector& origin, int boneMask, matrix3x4_t *bonetoworld )
{
	return g_StudioBoneCache.Studio_SetBoneCache( pStudioHdr, sequence, animtime, angles, origin, boneMask, bonetoworld );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Studio_InvalidateBoneCache( studiocache_t *pcache )
{
	g_StudioBoneCache.BoneCacheFree( pcache );
}


#pragma warning (disable : 4701)


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
static int ClipRayToHitbox( const Ray_t &ray, mstudiobbox_t *pbox, matrix3x4_t& matrix, trace_t &tr )
{
	// scale by current t so hits shorten the ray and increase the likelihood of early outs
	Vector delta2 = (0.5f * tr.fraction) * ray.m_Delta;

	// OPTIMIZE: Store this in the box instead of computing it here
	// compute center in local space
	Vector boxextents = (pbox->bbmin + pbox->bbmax) * 0.5; 
	Vector boxCenter;
	// transform to world space
	VectorTransform( boxextents, matrix, boxCenter );
	// calc extents from local center
	boxextents = pbox->bbmax - boxextents;
	// OPTIMIZE: This is optimized for world space.  If the transform is fast enough, it may make more
	// sense to just xform and call UTIL_ClipToBox() instead.  MEASURE THIS.

	// save the extents of the ray along 
	Vector extent, uextent;
	Vector segmentCenter = ray.m_Start + delta2 - boxCenter;

	extent.Init();

	// check box axes for separation
	for ( int j = 0; j < 3; j++ )
	{
		extent[j] = delta2.x * matrix[0][j] + delta2.y * matrix[1][j] +	delta2.z * matrix[2][j];
		uextent[j] = fabsf(extent[j]);
		float coord = segmentCenter.x * matrix[0][j] + segmentCenter.y * matrix[1][j] +	segmentCenter.z * matrix[2][j];
		coord = fabsf(coord);

		if ( coord > (boxextents[j] + uextent[j]) )
			return -1;
	}

	// now check cross axes for separation
	float tmp, cextent;
	Vector cross = delta2.Cross( segmentCenter );
	cextent = cross.x * matrix[0][0] + cross.y * matrix[1][0] + cross.z * matrix[2][0];
	cextent = fabsf(cextent);
	tmp = boxextents[1]*uextent[2] + boxextents[2]*uextent[1];
	if ( cextent > tmp )
		return -1;

	cextent = cross.x * matrix[0][1] + cross.y * matrix[1][1] + cross.z * matrix[2][1];
	cextent = fabsf(cextent);
	tmp = boxextents[0]*uextent[2] + boxextents[2]*uextent[0];
	if ( cextent > tmp )
		return -1;

	cextent = cross.x * matrix[0][2] + cross.y * matrix[1][2] + cross.z * matrix[2][2];
	cextent = fabsf(cextent);
	tmp = boxextents[0]*uextent[1] + boxextents[1]*uextent[0];
	if ( cextent > tmp )
		return -1;

	// !!! We hit this box !!! compute intersection point and return
	Vector start;
	// Compute ray start in bone space
	VectorITransform( ray.m_Start, matrix, start );
	// extent is delta2 in bone space, recompute delta in bone space
	VectorScale( extent, 2, extent );
	int hitside;
	// delta was prescaled by the current t, so no need to see if this intersection
	// is closer
	bool startsolid;
	float fraction;
	if ( !IntersectRayWithBox( start, extent, pbox->bbmin, pbox->bbmax, 0, fraction, hitside, startsolid ) )
		return -1;
	Assert( IsFinite(fraction) );
	tr.fraction *= fraction;
	tr.startsolid = startsolid;
	return hitside;
}

#pragma warning (default : 4701)


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool TraceToStudio( const Ray_t& ray, studiohdr_t *pStudioHdr, mstudiohitboxset_t *set, 
				   matrix3x4_t **hitboxbones, int fContentsMask, trace_t &tr )
{
	tr.fraction = 1.0;
	tr.startsolid = false;

	// no hit yet
	int hitbox = -1;
	int hitside = -1;

	// OPTIMIZE: Partition these?
	int numHitboxes = StudioHitboxSet_GetNumHitboxesFromPtr( pStudioHdr, set );
	for ( int i = 0; i < numHitboxes; i++ )
	{
		mstudiobbox_t *pbox = StudioHitboxSet_GetHitboxFromPtr( pStudioHdr, set, i );
		if ( !pbox )
			continue;

		// Filter based on contents mask - use version-aware accessor for v44+ support
		int hitBone = StudioHitbox_GetBone( pStudioHdr, set, i );
		int fBoneContents = StudioBone_GetContents(pStudioHdr, hitBone);
		if ( ( fBoneContents & fContentsMask ) == 0 )
			continue;
		if ( !hitboxbones[i] )
			continue;
		
		// columns are axes of the bones in world space, translation is in world space
		matrix3x4_t& matrix = *hitboxbones[i];

		int side = ClipRayToHitbox( ray, pbox, matrix, tr );
		if ( side >= 0 )
		{
			hitbox = i;
			hitside = side;
		}
	}

	if ( hitbox >= 0 )
	{
		tr.endpos = ray.m_Start + tr.fraction * ray.m_Delta;
		tr.hitgroup = StudioHitbox_GetGroup( pStudioHdr, set, hitbox );
		tr.hitbox = hitbox;
		// Use version-aware accessors for v44+ support
		int hitBone = StudioHitbox_GetBone( pStudioHdr, set, hitbox );
		tr.contents = StudioBone_GetContents(pStudioHdr, hitBone) | CONTENTS_HITBOX;
		tr.physicsbone = StudioBone_GetPhysicsBone(pStudioHdr, hitBone);
		Assert( tr.physicsbone >= 0 );

		matrix3x4_t& matrix = *hitboxbones[hitbox];
		if ( hitside >= 3 )
		{
			hitside -= 3;
			tr.plane.normal[0] = matrix[0][hitside];
			tr.plane.normal[1] = matrix[1][hitside];
			tr.plane.normal[2] = matrix[2][hitside];
			//tr.plane.dist = DotProduct( tr.plane.normal, Vector(matrix[0][3], matrix[1][3], matrix[2][3] ) ) + pbox->bbmax[hitside];
		}
		else
		{
			tr.plane.normal[0] = -matrix[0][hitside];
			tr.plane.normal[1] = -matrix[1][hitside];
			tr.plane.normal[2] = -matrix[2][hitside];
			//tr.plane.dist = DotProduct( tr.plane.normal, Vector(matrix[0][3], matrix[1][3], matrix[2][3] ) ) - pbox->bbmin[hitside];
		}
		// simpler plane constant equation
		tr.plane.dist = DotProduct( tr.endpos, tr.plane.normal );
		tr.plane.type = 3;
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: returns array of animations and weightings for a sequence based on current pose parameters
//-----------------------------------------------------------------------------

void Studio_SeqAnims( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[], mstudioanimdesc_t *panim[4], float *weight )
{
	// CRITICAL: Use version-aware accessor for sequence count
	if (!pStudioHdr || iSequence >= pStudioHdr->GetNumLocalSeq())
	{
		weight[0] = weight[1] = weight[2] = weight[3] = 0.0;
		return;
	}

	int i0 = 0, i1 = 0;
	float s0 = 0, s1 = 0;
	
	mstudioseqdesc_t *pseqdesc = pStudioHdr->pSeqdesc( iSequence );

	Studio_LocalPoseParameter( pStudioHdr, poseParameter, pseqdesc, 0, s0, i0 );
	Studio_LocalPoseParameter( pStudioHdr, poseParameter, pseqdesc, 1, s1, i1 );

	// Clamp s0 and s1 to [0, 1] range to prevent negative weights
	// This can happen with malformed or version 37 model data
	// Also handle NaN values which can occur from division by zero in pose parameter calculations
	s0 = clamp( s0, 0.0f, 1.0f );
	s1 = clamp( s1, 0.0f, 1.0f );

	// Additional safety check for NaN (clamp doesn't handle NaN properly)
	if ( !IsFinite(s0) ) s0 = 0.0f;
	if ( !IsFinite(s1) ) s1 = 0.0f;

	panim[0] = GetAnimDescriptions( pStudioHdr, pseqdesc, i0  , i1  );
	weight[0] = (1 - s0) * (1 - s1);

	panim[1] = GetAnimDescriptions( pStudioHdr, pseqdesc, i0+1, i1  );
	weight[1] = (s0) * (1 - s1);

	panim[2] = GetAnimDescriptions( pStudioHdr, pseqdesc, i0  , i1+1);
	weight[2] = (1 - s0) * (s1);

	panim[3] = GetAnimDescriptions( pStudioHdr, pseqdesc, i0+1, i1+1);
	weight[3] = (s0) * (s1);

	// Safety: ensure all weights are valid (handles any remaining edge cases)
	for ( int i = 0; i < 4; i++ )
	{
		if ( weight[i] < 0.0f || !IsFinite(weight[i]) )
			weight[i] = 0.0f;
	}

	Assert( weight[0] >= 0.0f && weight[1] >= 0.0f && weight[2] >= 0.0f && weight[3] >= 0.0f );
}

//-----------------------------------------------------------------------------
// Purpose: returns max frame number for a sequence
//-----------------------------------------------------------------------------

static const mstudioanimdesc_v48_t *StudioAnimDesc_AsV48( const studiohdr_t *pStudioHdr, const mstudioanimdesc_t *panim )
{
	return ( pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44 ) ? (const mstudioanimdesc_v48_t *)panim : NULL;
}

static int StudioAnimDesc_GetNumFrames( const studiohdr_t *pStudioHdr, const mstudioanimdesc_t *panim )
{
	const mstudioanimdesc_v48_t *panim48 = StudioAnimDesc_AsV48( pStudioHdr, panim );
	return panim48 ? panim48->numframes : panim->numframes;
}

static float StudioAnimDesc_GetFPS( const studiohdr_t *pStudioHdr, const mstudioanimdesc_t *panim )
{
	const mstudioanimdesc_v48_t *panim48 = StudioAnimDesc_AsV48( pStudioHdr, panim );
	return panim48 ? panim48->fps : panim->fps;
}

static int StudioAnimDesc_GetNumMovements( const studiohdr_t *pStudioHdr, const mstudioanimdesc_t *panim )
{
	const mstudioanimdesc_v48_t *panim48 = StudioAnimDesc_AsV48( pStudioHdr, panim );
	return panim48 ? panim48->nummovements : panim->nummovements;
}

static mstudiomovement_t *StudioAnimDesc_GetMovement( const studiohdr_t *pStudioHdr, mstudioanimdesc_t *panim, int i )
{
	const mstudioanimdesc_v48_t *panim48 = StudioAnimDesc_AsV48( pStudioHdr, panim );
	return panim48 ? (mstudiomovement_t *)const_cast<mstudioanimdesc_v48_t *>(panim48)->pMovement( i ) : panim->pMovement( i );
}

int Studio_MaxFrame( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[] )
{
	mstudioanimdesc_t *panim[4];
	float	weight[4];

	Studio_SeqAnims( pStudioHdr, iSequence, poseParameter, panim, weight );

	float maxFrame = 0;
	for (int i = 0; i < 4; i++)
	{
		if (weight[i] > 0)
		{
			maxFrame += StudioAnimDesc_GetNumFrames( pStudioHdr, panim[i] ) * weight[i];
		}
	}

	if ( maxFrame > 1 )
		maxFrame -= 1;
	
	return maxFrame;
}


//-----------------------------------------------------------------------------
// Purpose: returns frames per second of a sequence
//-----------------------------------------------------------------------------

float Studio_FPS( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[] )
{
	mstudioanimdesc_t *panim[4];
	float	weight[4];

	Studio_SeqAnims( pStudioHdr, iSequence, poseParameter, panim, weight );

	float t = 0;

	for (int i = 0; i < 4; i++)
	{
		if (weight[i] > 0)
		{
			t += StudioAnimDesc_GetFPS( pStudioHdr, panim[i] ) * weight[i];
		}
	}
	return t;
}


//-----------------------------------------------------------------------------
// Purpose: returns cycles per second of a sequence
//-----------------------------------------------------------------------------

float Studio_CPS( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[] )
{
	mstudioanimdesc_t *panim[4];
	float	weight[4];

	Studio_SeqAnims( pStudioHdr, iSequence, poseParameter, panim, weight );

	float t = 0;

	for (int i = 0; i < 4; i++)
	{
		int numFrames = StudioAnimDesc_GetNumFrames( pStudioHdr, panim[i] );
		if (weight[i] > 0 && numFrames > 1)
		{
			t += (StudioAnimDesc_GetFPS( pStudioHdr, panim[i] ) / (numFrames - 1)) * weight[i];
		}
	}
	return t;
}

//-----------------------------------------------------------------------------
// Purpose: returns length (in seconds) of a sequence
//-----------------------------------------------------------------------------

float Studio_Duration( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[] )
{
	mstudioanimdesc_t *panim[4];
	float	weight[4];

	Studio_SeqAnims( pStudioHdr, iSequence, poseParameter, panim, weight );

	float t = 0;

	for (int i = 0; i < 4; i++)
	{
		if (weight[i] > 0)
		{
			float fps = StudioAnimDesc_GetFPS( pStudioHdr, panim[i] );
			if ( fps != 0.0f )
			{
				t +=  ((StudioAnimDesc_GetNumFrames( pStudioHdr, panim[i] ) - 1) / fps) * weight[i];
			}
		}
	}

	return t;
}


//-----------------------------------------------------------------------------
// Purpose: calculate changes in position and angle relative to the start of an animations cycle
// Output:	updated position and angle, relative to the origin
//			returns false if animation is not a movement animation
//-----------------------------------------------------------------------------

static bool Studio_AnimPositionVersioned( const studiohdr_t *pStudioHdr, mstudioanimdesc_t *panim, float flCycle, Vector &vecPos, QAngle &vecAngle )
{
	float	prevframe = 0;
	vecPos.Init( );
	vecAngle.Init( );

	int numMovements = StudioAnimDesc_GetNumMovements( pStudioHdr, panim );
	if (numMovements == 0)
		return false;

	int iLoops = 0;
	if (flCycle > 1.0)
	{
		iLoops = (int)flCycle;
	}
	else if (flCycle < 0.0)
	{
		iLoops = (int)flCycle - 1;
	}
	flCycle = flCycle - iLoops;

	int numFrames = StudioAnimDesc_GetNumFrames( pStudioHdr, panim );
	float	flFrame = flCycle * (numFrames - 1);

	for (int i = 0; i < numMovements; i++)
	{
		mstudiomovement_t *pmove = StudioAnimDesc_GetMovement( pStudioHdr, panim, i );

		if (StudioMovement_GetEndFrame(pStudioHdr, pmove) >= flFrame)
		{
			float f = (flFrame - prevframe) / (StudioMovement_GetEndFrame(pStudioHdr, pmove) - prevframe);

			float d = StudioMovement_GetV0(pStudioHdr, pmove) * f + 0.5 * (StudioMovement_GetV1(pStudioHdr, pmove) - StudioMovement_GetV0(pStudioHdr, pmove)) * f * f;

			vecPos = vecPos + d * StudioMovement_GetVector(pStudioHdr, pmove);
			vecAngle.y = vecAngle.y * (1 - f) + StudioMovement_GetAngle(pStudioHdr, pmove) * f;
			if (iLoops != 0)
			{
				mstudiomovement_t *pmove = StudioAnimDesc_GetMovement( pStudioHdr, panim, numMovements - 1 );
				vecPos = vecPos + iLoops * StudioMovement_GetPosition(pStudioHdr, pmove); 
				vecAngle.y = vecAngle.y + iLoops * StudioMovement_GetAngle(pStudioHdr, pmove); 
			}
			return true;
		}
		else
		{
			prevframe = StudioMovement_GetEndFrame(pStudioHdr, pmove);
			vecPos = StudioMovement_GetPosition(pStudioHdr, pmove);
			vecAngle.y = StudioMovement_GetAngle(pStudioHdr, pmove);
		}
	}

	return false;
}

bool Studio_AnimPosition( mstudioanimdesc_t *panim, float flCycle, Vector &vecPos, QAngle &vecAngle )
{
	return Studio_AnimPositionVersioned( NULL, panim, flCycle, vecPos, vecAngle );
}

//-----------------------------------------------------------------------------
// Purpose: calculate instantaneous velocity at a given point in the animations cycle
// Output:	velocity vector, relative to identity orientation
//			returns false if animation is not a movement animation
//-----------------------------------------------------------------------------

static bool Studio_AnimVelocityVersioned( const studiohdr_t *pStudioHdr, mstudioanimdesc_t *panim, float flCycle, Vector &vecVelocity )
{
	float	prevframe = 0;

	int numFrames = StudioAnimDesc_GetNumFrames( pStudioHdr, panim );
	float	flFrame = flCycle * (numFrames - 1);
	flFrame = flFrame - (int)(flFrame / (numFrames - 1));

	for (int i = 0; i < StudioAnimDesc_GetNumMovements( pStudioHdr, panim ); i++)
	{
		mstudiomovement_t *pmove = StudioAnimDesc_GetMovement( pStudioHdr, panim, i );

		if (StudioMovement_GetEndFrame(pStudioHdr, pmove) >= flFrame)
		{
			float f = (flFrame - prevframe) / (StudioMovement_GetEndFrame(pStudioHdr, pmove) - prevframe);

			float vel = StudioMovement_GetV0(pStudioHdr, pmove) * (1 - f) + StudioMovement_GetV1(pStudioHdr, pmove) * f;
			// scale from per block to per cycle velocity
			vel = vel * (StudioMovement_GetEndFrame(pStudioHdr, pmove) - prevframe) / (numFrames - 1);

			vecVelocity = StudioMovement_GetVector(pStudioHdr, pmove) * vel;
			return true;
		}
	}
	return false;
}

bool Studio_AnimVelocity( mstudioanimdesc_t *panim, float flCycle, Vector &vecVelocity )
{
	return Studio_AnimVelocityVersioned( NULL, panim, flCycle, vecVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: calculate changes in position and angle between two points in an animation cycle
// Output:	updated position and angle, relative to CycleFrom being at the origin
//			returns false if animation is not a movement animation
//-----------------------------------------------------------------------------

static bool Studio_AnimMovementVersioned( const studiohdr_t *pStudioHdr, mstudioanimdesc_t *panim, float flCycleFrom, float flCycleTo, Vector &deltaPos, QAngle &deltaAngle )
{
	if (StudioAnimDesc_GetNumMovements( pStudioHdr, panim ) == 0)
		return false;

	Vector startPos;
	QAngle startA;
	Studio_AnimPositionVersioned( pStudioHdr, panim, flCycleFrom, startPos, startA );

	Vector endPos;
	QAngle endA;
	Studio_AnimPositionVersioned( pStudioHdr, panim, flCycleTo, endPos, endA );

	Vector tmp = endPos - startPos;
	deltaAngle.y = endA.y - startA.y;
	VectorYawRotate( tmp, -startA.y, deltaPos );

	return true;
}

bool Studio_AnimMovement( mstudioanimdesc_t *panim, float flCycleFrom, float flCycleTo, Vector &deltaPos, QAngle &deltaAngle )
{
	return Studio_AnimMovementVersioned( NULL, panim, flCycleFrom, flCycleTo, deltaPos, deltaAngle );
}

//-----------------------------------------------------------------------------
// Purpose: finds how much of an animation to play to move given linear distance
//-----------------------------------------------------------------------------

static float Studio_FindAnimDistanceVersioned( const studiohdr_t *pStudioHdr, mstudioanimdesc_t *panim, float flDist )
{
	float	prevframe = 0;

	if (flDist <= 0)
		return 0.0;

	for (int i = 0; i < StudioAnimDesc_GetNumMovements( pStudioHdr, panim ); i++)
	{
		mstudiomovement_t *pmove = StudioAnimDesc_GetMovement( pStudioHdr, panim, i );

		float flMove = (StudioMovement_GetV0(pStudioHdr, pmove) + StudioMovement_GetV1(pStudioHdr, pmove)) * 0.5;

		if (flMove >= flDist)
		{
			float root1, root2;

			// d = V0 * t + 1/2 (V1-V0) * t^2
			if (SolveQuadratic( 0.5 * (StudioMovement_GetV1(pStudioHdr, pmove) - StudioMovement_GetV0(pStudioHdr, pmove)), StudioMovement_GetV0(pStudioHdr, pmove), -flDist, root1, root2 ))
			{
				float cpf = 1.0 / (StudioAnimDesc_GetNumFrames( pStudioHdr, panim ) - 1);  // cycles per frame

				return (prevframe + root1 * (StudioMovement_GetEndFrame(pStudioHdr, pmove) - prevframe)) * cpf;
			}
			return 0.0;
		}
		else
		{
			flDist -= flMove;
			prevframe = StudioMovement_GetEndFrame(pStudioHdr, pmove);
		}
	}
	return 1.0;
}

float Studio_FindAnimDistance( mstudioanimdesc_t *panim, float flDist )
{
	return Studio_FindAnimDistanceVersioned( NULL, panim, flDist );
}

//-----------------------------------------------------------------------------
// Purpose: calculate changes in position and angle between two points in a sequences cycle
// Output:	updated position and angle, relative to CycleFrom being at the origin
//			returns false if sequence is not a movement sequence
//-----------------------------------------------------------------------------

bool Studio_SeqMovement( const studiohdr_t *pStudioHdr, int iSequence, float flCycleFrom, float flCycleTo, const float poseParameter[], Vector &deltaPos, QAngle &deltaAngles )
{
	mstudioanimdesc_t *panim[4];
	float	weight[4];

	Studio_SeqAnims( pStudioHdr, iSequence, poseParameter, panim, weight );
	
	deltaPos.Init( );
	deltaAngles.Init( );

	bool found = false;

	for (int i = 0; i < 4; i++)
	{
		if (weight[i])
		{
			Vector localPos;
			QAngle localAngles;

			localPos.Init();
			localAngles.Init();

			if (Studio_AnimMovementVersioned( pStudioHdr, panim[i], flCycleFrom, flCycleTo, localPos, localAngles ))
			{
				found = true;
				deltaPos = deltaPos + localPos * weight[i];
				// FIXME: this makes no sense
				deltaAngles = deltaAngles + localAngles * weight[i];
			}
		}
	}
	return found;
}


//-----------------------------------------------------------------------------
// Purpose: calculate instantaneous velocity at a given point in the sequence's cycle
// Output:	velocity vector, relative to identity orientation
//			returns false if sequence is not a movement sequence
//-----------------------------------------------------------------------------

bool Studio_SeqVelocity( const studiohdr_t *pStudioHdr, int iSequence, float flCycle, const float poseParameter[], Vector &vecVelocity )
{
	mstudioanimdesc_t *panim[4];
	float	weight[4];

	Studio_SeqAnims( pStudioHdr, iSequence, poseParameter, panim, weight );
	
	vecVelocity.Init( );

	bool found = false;

	for (int i = 0; i < 4; i++)
	{
		if (weight[i])
		{
			Vector vecLocalVelocity;

			if (Studio_AnimVelocityVersioned( pStudioHdr, panim[i], flCycle, vecLocalVelocity ))
			{
				float fps = StudioAnimDesc_GetFPS( pStudioHdr, panim[i] );
				if (fps != 0.0f)
					vecVelocity = vecVelocity + vecLocalVelocity * weight[i] * (StudioAnimDesc_GetNumFrames( pStudioHdr, panim[i] ) / fps);
				found = true;
			}
		}
	}
	return found;
}

//-----------------------------------------------------------------------------
// Purpose: finds how much of an sequence to play to move given linear distance
//-----------------------------------------------------------------------------

float Studio_FindSeqDistance( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[], float flDist )
{
	mstudioanimdesc_t *panim[4];
	float	weight[4];

	Studio_SeqAnims( pStudioHdr, iSequence, poseParameter, panim, weight );
	
	float flCycle = 0;

	for (int i = 0; i < 4; i++)
	{
		if (weight[i])
		{
			float flLocalCycle = Studio_FindAnimDistanceVersioned( pStudioHdr, panim[i], flDist );
			flCycle = flCycle + flLocalCycle * weight[i];
		}
	}
	return flCycle;
}

//-----------------------------------------------------------------------------
// Purpose: lookup attachment by name
//-----------------------------------------------------------------------------

int Studio_FindAttachment( const studiohdr_t *pStudioHdr, const char *pAttachmentName )
{
	if ( pStudioHdr )
	{
		// Extract the bone index from the name
		for (int i = 0; i < StudioHdr_GetNumAttachments(pStudioHdr); i++)
		{
			if (!Q_stricmp(pAttachmentName, StudioAttachment_GetName(pStudioHdr, i)))
			{
				return i;
			}
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: lookup attachments by substring. Randomly return one of the matching attachments.
//-----------------------------------------------------------------------------

int Studio_FindRandomAttachment( const studiohdr_t *pStudioHdr, const char *pAttachmentName )
{
	if ( pStudioHdr )
	{
		// First move them all matching attachments into a list
		CUtlVector<int> matchingAttachments;

		// Extract the bone index from the name
		for (int i = 0; i < StudioHdr_GetNumAttachments(pStudioHdr); i++)
		{
			if ( strstr( StudioAttachment_GetName(pStudioHdr, i), pAttachmentName ) ) 
			{
				matchingAttachments.AddToTail(i);
			}
		}

		// Then randomly return one of the attachments
		if ( matchingAttachments.Size() > 0 )
			return matchingAttachments[ RandomInt( 0, matchingAttachments.Size()-1 ) ];
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: lookup bone by name
//-----------------------------------------------------------------------------

int Studio_BoneIndexByName( const studiohdr_t *pStudioHdr, const char *pName )
{
	// Use version-aware bone name accessor for v44+ support
	int numBones = StudioHdr_GetNumBones(pStudioHdr);
	for ( int i = 0; i < numBones; i++ )
	{
		if (!Q_stricmp(pName, StudioBone_GetName(pStudioHdr, i)))
			return i;
	}

	return -1;
}

const char *Studio_GetDefaultSurfaceProps( studiohdr_t *pstudiohdr )
{
	return pstudiohdr->pszSurfaceProp();
}

float Studio_GetMass( studiohdr_t *pstudiohdr )
{
	return pstudiohdr ? pstudiohdr->mass : 0.f;
}

//-----------------------------------------------------------------------------
// Purpose: return pointer to sequence key value buffer
//-----------------------------------------------------------------------------

const char *Studio_GetKeyValueText( const studiohdr_t *pStudioHdr, int iSequence )
{
	if (pStudioHdr)
	{
		// CRITICAL: Use version-aware accessor for sequence count
		if (iSequence >= 0 && iSequence < pStudioHdr->GetNumLocalSeq())
		{
			return StudioSeqdesc_GetKeyValueText( pStudioHdr, iSequence );
		}
	}
	return NULL;
}

