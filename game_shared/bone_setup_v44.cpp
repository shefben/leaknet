//========= Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: v44+ Bone setup and IK system implementation - COMPLETELY ISOLATED FROM V37
// This file is for v44-v48 model support only. Does not touch v37 code.
// From 2007 Source Engine
//
// $NoKeywords: $
//=============================================================================//

#include "tier0/dbg.h"
#include "mathlib/mathlib.h"
#include "bone_setup_v44.h"
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Ken Perlin's Two-Link IK Solver
// Copied here to maintain isolation from v37 bone_setup.cpp
//-----------------------------------------------------------------------------
class ik
{
public:
	// Given a two link joint from [0,0,0] to end effector position P,
	// let link lengths be a and b, and let norm |P| = c.  Clearly a+b >= c.
	// Problem: find a "knee" position Q such that |Q| = a and |P-Q| = b.

	static float findD(float a, float b, float c) {
		return max(0.f, min(a, (c + (a*a-b*b)/c) / 2));
	}
	static float findE(float a, float d) { return sqrt(a*a-d*d); }

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

	static void defineM(float const P[], float const D[]) {
		float *X = Minv[0], *Y = Minv[1], *Z = Minv[2];
		int i;
		for (i = 0 ; i < 3 ; i++)
			X[i] = P[i];
		normalize(X);

		float dDOTx = dot(D,X);
		for (i = 0 ; i < 3 ; i++)
			Y[i] = D[i] - dDOTx * X[i];
		normalize(Y);

		cross(X,Y,Z);

		for (i = 0 ; i < 3 ; i++) {
			Mfwd[i][0] = Minv[0][i];
			Mfwd[i][1] = Minv[1][i];
			Mfwd[i][2] = Minv[2][i];
		}
	}

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
// CIKContext_v44 Implementation
//-----------------------------------------------------------------------------

CIKContext_v44::CIKContext_v44()
{
	m_pStudioHdr = NULL;
	m_flPrevTime = -1.0f;
	m_flTime = -1.0f;
	m_iFramecounter = 0;
	m_boneMask = 0;
	m_targetCount = 0;
	memset(m_target, 0, sizeof(m_target));
}

void CIKContext_v44::Init( const studiohdr_v44_t *pStudioHdr, const QAngle &angles, const Vector &pos, float flTime, int iFramecounter, int boneMask )
{
	m_pStudioHdr = pStudioHdr;
	m_ikChainRule.RemoveAll();
	m_ikLock.RemoveAll();
	AngleMatrix( angles, pos, m_rootxform );
	m_flPrevTime = m_flTime;
	m_flTime = flTime;
	m_iFramecounter = iFramecounter;
	m_boneMask = boneMask;
}

void CIKContext_v44::ClearTargets( void )
{
	m_targetCount = 0;
	for ( int i = 0; i < 12; i++ )
	{
		m_target[i].latched.iFramecounter = -9999;
	}
}

void CIKContext_v44::AddDependencies( int iSequence, float flCycle, const float poseParameters[], float flWeight )
{
	if (!m_pStudioHdr)
		return;

	// Skip if no IK chains
	if (m_pStudioHdr->numikchains == 0)
		return;

	// Validate sequence
	if (iSequence < 0 || iSequence >= m_pStudioHdr->numlocalseq)
		return;

	mstudioseqdesc_v44_t *pseqdesc = m_pStudioHdr->pLocalSeqdesc( iSequence );
	if (!pseqdesc || pseqdesc->numikrules == 0)
		return;

	// Clamp weight
	flWeight = clamp( flWeight, 0.0f, 1.0f );

	// Normalize cycle
	if (pseqdesc->flags & STUDIO_LOOPING)
	{
		flCycle = flCycle - (int)flCycle;
		if (flCycle < 0) flCycle += 1;
	}
	else
	{
		flCycle = clamp( flCycle, 0.0f, 0.9999f );
	}

	// Ensure we have chain rules for each chain
	if (m_ikChainRule.Count() != m_pStudioHdr->numikchains)
	{
		m_ikChainRule.SetCount( m_pStudioHdr->numikchains );
	}

	// Note: Full IK rule processing requires Studio_SeqAnims and Studio_IKSequenceError
	// which are complex animation blending utilities. For now, this provides the basic
	// infrastructure - full implementation would require porting those utilities.
}

void CIKContext_v44::UpdateTargets( Vector pos[], Quaternion q[], matrix3x4_t boneToWorld[], CBoneBitList_v44 &boneComputed )
{
	if (!m_pStudioHdr)
		return;

	// Initialize all targets
	for (int i = 0; i < m_targetCount && i < 12; i++)
	{
		m_target[i].est.flWeight = 0.0f;
		m_target[i].est.latched = 1.0f;
		m_target[i].est.release = 1.0f;
		m_target[i].est.height = 0.0f;
		m_target[i].est.floor = 0.0f;
		m_target[i].est.radius = 0.0f;
		m_target[i].offset.pos.Init();
		m_target[i].offset.q.Init();
	}

	// Auto release any latched IK
	AutoIKRelease();

	// Process IK chain rules
	for (int j = 0; j < m_ikChainRule.Count(); j++)
	{
		for (int i = 0; i < m_ikChainRule.Element( j ).Count(); i++)
		{
			ikcontextikrule_v44_t *pRule = &m_ikChainRule.Element( j ).Element( i );

			// Validate slot
			if (pRule->slot < 0 || pRule->slot >= 12)
				continue;

			CIKTarget_v44 *pTarget = &m_target[pRule->slot];
			pTarget->chain = pRule->chain;
			pTarget->type = pRule->type;

			if (pRule->flRuleWeight == 1.0f || pTarget->est.flWeight == 0.0f)
			{
				pTarget->offset.q = pRule->q;
				pTarget->offset.pos = pRule->pos;
				pTarget->est.height = pRule->height;
				pTarget->est.floor = pRule->floor;
				pTarget->est.radius = pRule->radius;
				pTarget->est.latched = pRule->latched * pRule->flRuleWeight;
				pTarget->est.release = pRule->release;
				pTarget->est.flWeight = pRule->flWeight * pRule->flRuleWeight;
			}
			else
			{
				QuaternionSlerp( pTarget->offset.q, pRule->q, pRule->flRuleWeight, pTarget->offset.q );
				pTarget->offset.pos = Lerp( pRule->flRuleWeight, pTarget->offset.pos, pRule->pos );
				pTarget->est.height = Lerp( pRule->flRuleWeight, pTarget->est.height, pRule->height );
				pTarget->est.floor = Lerp( pRule->flRuleWeight, pTarget->est.floor, pRule->floor );
				pTarget->est.radius = Lerp( pRule->flRuleWeight, pTarget->est.radius, pRule->radius );
				pTarget->est.latched = min( pTarget->est.latched, pRule->latched );
				pTarget->est.release = Lerp( pRule->flRuleWeight, pTarget->est.release, pRule->release );
				pTarget->est.flWeight = Lerp( pRule->flRuleWeight, pTarget->est.flWeight, pRule->flWeight );
			}
		}
	}
}

void CIKContext_v44::AutoIKRelease( void )
{
	if (!m_pStudioHdr)
		return;

	// Check if we should auto-release any latched IK targets
	for (int i = 0; i < m_targetCount && i < 12; i++)
	{
		CIKTarget_v44 *pTarget = &m_target[i];

		// Skip inactive targets
		if (!pTarget->IsActive())
			continue;

		// Check frame counter for stale targets
		if (pTarget->latched.iFramecounter >= 0 &&
			pTarget->latched.iFramecounter < m_iFramecounter - 1)
		{
			// Target is stale, release it
			pTarget->latched.bNeedsLatch = false;
			pTarget->latched.bHasLatch = false;
			pTarget->latched.influence = 0.0f;
		}
	}
}

void CIKContext_v44::SolveDependencies( Vector pos[], Quaternion q[], matrix3x4_t boneToWorld[], CBoneBitList_v44 &boneComputed )
{
	if (!m_pStudioHdr)
		return;

	// Process each active IK target
	for (int i = 0; i < m_targetCount && i < 12; i++)
	{
		CIKTarget_v44 *pTarget = &m_target[i];

		// Skip inactive targets
		if (pTarget->est.flWeight <= 0.0f)
			continue;

		// Get IK chain for this target
		if (pTarget->chain < 0 || pTarget->chain >= m_pStudioHdr->numikchains)
			continue;

		mstudioikchain_v44_t *pchain = m_pStudioHdr->pIKChain( pTarget->chain );
		if (!pchain || pchain->numlinks < 3)
			continue;

		// Get foot bone
		int bone = pchain->pLink( 2 )->bone;
		if (bone < 0 || bone >= m_pStudioHdr->numbones)
			continue;

		// Build bone chain to current foot position
		BuildBoneChain( pos, q, bone, boneToWorld, boneComputed );

		// Get target position
		Vector targetPos = pTarget->est.pos;

		// Apply IK
		Studio_SolveIK_v44( pchain, targetPos, boneToWorld );

		// Back-solve to update pos/q
		SolveBone_v44( m_pStudioHdr, pchain->pLink( 2 )->bone, boneToWorld, pos, q );
		SolveBone_v44( m_pStudioHdr, pchain->pLink( 1 )->bone, boneToWorld, pos, q );
		SolveBone_v44( m_pStudioHdr, pchain->pLink( 0 )->bone, boneToWorld, pos, q );
	}
}

void CIKContext_v44::SolveDependencies( Vector pos[], Quaternion q[] )
{
	// Legacy interface - create temporary bone matrices
	static matrix3x4_t boneToWorld[MAXSTUDIOBONES];
	CBoneBitList_v44 boneComputed;

	SolveDependencies( pos, q, boneToWorld, boneComputed );
}

//-----------------------------------------------------------------------------
// SolveBone_v44 - Extract local pos/q from world transform (static helper)
//-----------------------------------------------------------------------------
static void SolveBone_v44(
	const studiohdr_v44_t *pStudioHdr,
	int iBone,
	matrix3x4_t *pBoneToWorld,
	Vector pos[],
	Quaternion q[] )
{
	if (!pStudioHdr || iBone < 0 || iBone >= pStudioHdr->numbones)
		return;

	mstudiobone_v44_t *pBone = pStudioHdr->pBone( iBone );
	if (!pBone)
		return;

	int iParent = pBone->parent;
	if (iParent == -1)
	{
		MatrixAngles( pBoneToWorld[iBone], q[iBone], pos[iBone] );
	}
	else
	{
		matrix3x4_t worldToBone;
		MatrixInvert( pBoneToWorld[iParent], worldToBone );

		matrix3x4_t local;
		ConcatTransforms( worldToBone, pBoneToWorld[iBone], local );

		MatrixAngles( local, q[iBone], pos[iBone] );
	}
}

void CIKContext_v44::AddAutoplayLocks( Vector pos[], Quaternion q[] )
{
	if (!m_pStudioHdr)
		return;

	// Skip if no autoplay locks
	if (m_pStudioHdr->numlocalikautoplaylocks == 0)
		return;

	// Use static matrices (no g_MatrixPool in LeakNet)
	static matrix3x4_t boneToWorld[MAXSTUDIOBONES];
	CBoneBitList_v44 boneComputed;

	int ikOffset = m_ikLock.AddMultipleToTail( m_pStudioHdr->numlocalikautoplaylocks );
	memset( &m_ikLock[ikOffset], 0, sizeof(ikcontextikrule_v44_t) * m_pStudioHdr->numlocalikautoplaylocks );

	for (int i = 0; i < m_pStudioHdr->numlocalikautoplaylocks; i++)
	{
		mstudioiklock_v44_t *plock = m_pStudioHdr->pLocalIKAutoplayLock( i );
		if (!plock)
			continue;

		mstudioikchain_v44_t *pchain = m_pStudioHdr->pIKChain( plock->chain );
		if (!pchain || pchain->numlinks < 3)
			continue;

		int bone = pchain->pLink( 2 )->bone;
		if (bone < 0 || bone >= m_pStudioHdr->numbones)
			continue;

		// Don't bother with iklock if the bone isn't going to be calculated
		mstudiobone_v44_t *pBone = m_pStudioHdr->pBone( bone );
		if (!pBone || !(pBone->flags & m_boneMask))
			continue;

		// Eval current ik'd bone
		BuildBoneChain( pos, q, bone, boneToWorld, boneComputed );

		ikcontextikrule_v44_t &ikrule = m_ikLock[ i + ikOffset ];

		ikrule.chain = plock->chain;
		ikrule.slot = i;
		ikrule.flWeight = 1.0f;
		ikrule.flRuleWeight = 1.0f;

		// Save pos and orientation
		MatrixAngles( boneToWorld[bone], ikrule.q, ikrule.pos );
	}
}

void CIKContext_v44::SolveAutoplayLocks( Vector pos[], Quaternion q[] )
{
	if (!m_pStudioHdr)
		return;

	// Use static matrices
	static matrix3x4_t boneToWorld[MAXSTUDIOBONES];
	CBoneBitList_v44 boneComputed;

	for (int i = 0; i < m_ikLock.Count(); i++)
	{
		if (i >= m_pStudioHdr->numlocalikautoplaylocks)
			break;

		mstudioiklock_v44_t *plock = m_pStudioHdr->pLocalIKAutoplayLock( i );
		if (plock)
		{
			SolveLock( plock, i, pos, q, boneToWorld, boneComputed );
		}
	}
}

void CIKContext_v44::AddSequenceLocks( mstudioseqdesc_v48_t *pSeqDesc, Vector pos[], Quaternion q[] )
{
	if (!m_pStudioHdr || !pSeqDesc)
		return;

	// Skip if no IK locks in sequence
	if (pSeqDesc->numiklocks == 0)
		return;

	// Use static matrices
	static matrix3x4_t boneToWorld[MAXSTUDIOBONES];
	CBoneBitList_v44 boneComputed;

	int ikOffset = m_ikLock.AddMultipleToTail( pSeqDesc->numiklocks );
	memset( &m_ikLock[ikOffset], 0, sizeof(ikcontextikrule_v44_t) * pSeqDesc->numiklocks );

	for (int i = 0; i < pSeqDesc->numiklocks; i++)
	{
		mstudioiklock_v44_t *plock = pSeqDesc->pIKLock( i );
		if (!plock)
			continue;

		mstudioikchain_v44_t *pchain = m_pStudioHdr->pIKChain( plock->chain );
		if (!pchain || pchain->numlinks < 3)
			continue;

		int bone = pchain->pLink( 2 )->bone;
		if (bone < 0 || bone >= m_pStudioHdr->numbones)
			continue;

		// Don't bother if bone won't be calculated
		mstudiobone_v44_t *pBone = m_pStudioHdr->pBone( bone );
		if (!pBone || !(pBone->flags & m_boneMask))
			continue;

		// Eval current bone
		BuildBoneChain( pos, q, bone, boneToWorld, boneComputed );

		ikcontextikrule_v44_t &ikrule = m_ikLock[ i + ikOffset ];

		ikrule.chain = plock->chain;
		ikrule.slot = i;
		ikrule.flWeight = 1.0f;
		ikrule.flRuleWeight = 1.0f;

		// Save pos and orientation
		MatrixAngles( boneToWorld[bone], ikrule.q, ikrule.pos );
	}
}

void CIKContext_v44::SolveSequenceLocks( mstudioseqdesc_v48_t *pSeqDesc, Vector pos[], Quaternion q[] )
{
	if (!m_pStudioHdr || !pSeqDesc)
		return;

	// Use static matrices
	static matrix3x4_t boneToWorld[MAXSTUDIOBONES];
	CBoneBitList_v44 boneComputed;

	for (int i = 0; i < pSeqDesc->numiklocks; i++)
	{
		mstudioiklock_v44_t *plock = pSeqDesc->pIKLock( i );
		if (plock)
		{
			SolveLock( plock, i, pos, q, boneToWorld, boneComputed );
		}
	}
}

void CIKContext_v44::AddAllLocks( Vector pos[], Quaternion q[] )
{
	// Add autoplay locks first
	AddAutoplayLocks( pos, q );
}

void CIKContext_v44::SolveAllLocks( Vector pos[], Quaternion q[] )
{
	// Solve autoplay locks
	SolveAutoplayLocks( pos, q );
}

void CIKContext_v44::SolveLock( const mstudioiklock_v44_t *plock, int i, Vector pos[], Quaternion q[],
	matrix3x4_t boneToWorld[], CBoneBitList_v44 &boneComputed )
{
	if (!m_pStudioHdr || !plock || i < 0 || i >= m_ikLock.Count())
		return;

	mstudioikchain_v44_t *pchain = m_pStudioHdr->pIKChain( plock->chain );
	if (!pchain || pchain->numlinks < 3)
		return;

	int bone = pchain->pLink( 2 )->bone;
	if (bone < 0 || bone >= m_pStudioHdr->numbones)
		return;

	// Don't bother if bone won't be calculated
	mstudiobone_v44_t *pBone = m_pStudioHdr->pBone( bone );
	if (!pBone || !(pBone->flags & m_boneMask))
		return;

	// Eval current ik'd bone
	BuildBoneChain( pos, q, bone, boneToWorld, boneComputed );

	Vector p1, p3;
	Quaternion q2;

	// Current position
	MatrixPosition( boneToWorld[bone], p1 );

	// Blend in position
	p3 = p1 * (1.0f - plock->flPosWeight) + m_ikLock[i].pos * plock->flPosWeight;

	// Do IK solution
	if (m_ikLock[i].kneeDir.LengthSqr() > 0)
	{
		Studio_SolveIK_v44(pchain->pLink( 0 )->bone, pchain->pLink( 1 )->bone, pchain->pLink( 2 )->bone,
						   p3, m_ikLock[i].kneePos, m_ikLock[i].kneeDir, boneToWorld );
	}
	else
	{
		Studio_SolveIK_v44(pchain, p3, boneToWorld );
	}

	// Slam orientation
	MatrixPosition( boneToWorld[bone], p3 );
	QuaternionMatrix( m_ikLock[i].q, p3, boneToWorld[bone] );

	// Rebuild chain
	q2 = q[bone];
	SolveBone_v44( m_pStudioHdr, pchain->pLink( 2 )->bone, boneToWorld, pos, q );
	QuaternionSlerp( q[bone], q2, plock->flLocalQWeight, q[bone] );

	SolveBone_v44( m_pStudioHdr, pchain->pLink( 1 )->bone, boneToWorld, pos, q );
	SolveBone_v44( m_pStudioHdr, pchain->pLink( 0 )->bone, boneToWorld, pos, q );
}

bool CIKContext_v44::Estimate( int iSequence, float flCycle, int iTarget, const float poseParameter[], float flWeight )
{
	if (!m_pStudioHdr)
		return false;

	// Validate target
	if (iTarget < 0 || iTarget >= 12)
		return false;

	// Validate sequence
	if (iSequence < 0 || iSequence >= m_pStudioHdr->numlocalseq)
		return false;

	mstudioseqdesc_v44_t *pseqdesc = m_pStudioHdr->pLocalSeqdesc( iSequence );
	if (!pseqdesc || pseqdesc->numikrules == 0)
		return false;

	// Clamp weight
	flWeight = clamp( flWeight, 0.0f, 1.0f );
	if (flWeight <= 0.0f)
		return false;

	// Normalize cycle
	if (pseqdesc->flags & STUDIO_LOOPING)
	{
		flCycle = flCycle - (int)flCycle;
		if (flCycle < 0) flCycle += 1;
	}
	else
	{
		flCycle = clamp( flCycle, 0.0f, 0.9999f );
	}

	// Get target
	CIKTarget_v44 *pTarget = &m_target[iTarget];

	// Note: Full estimation requires Studio_SeqAnims, Studio_IKSequenceError, and
	// related animation blending utilities. This is a simplified placeholder that
	// provides the basic infrastructure.

	// Mark target as valid with given weight
	pTarget->est.flWeight = flWeight;
	pTarget->est.flTime = m_flTime;

	return true;
}

void CIKContext_v44::BuildBoneChain( const Vector pos[], const Quaternion q[], int iBone,
	matrix3x4_t *pBoneToWorld, CBoneBitList_v44 &boneComputed )
{
	if (!m_pStudioHdr || iBone < 0 || iBone >= m_pStudioHdr->numbones)
		return;

	// Already computed?
	if (boneComputed.IsBoneMarked(iBone))
		return;

	matrix3x4_t bonematrix;
	QuaternionMatrix( q[iBone], pos[iBone], bonematrix );

	mstudiobone_v44_t *pBone = m_pStudioHdr->pBone( iBone );
	if (!pBone)
		return;

	int parent = pBone->parent;
	if (parent == -1)
	{
		ConcatTransforms( m_rootxform, bonematrix, pBoneToWorld[iBone] );
	}
	else
	{
		// Recursively build parent chain first
		BuildBoneChain( pos, q, parent, pBoneToWorld, boneComputed );
		ConcatTransforms( pBoneToWorld[parent], bonematrix, pBoneToWorld[iBone] );
	}
	boneComputed.MarkBone(iBone);
}

//-----------------------------------------------------------------------------
// CIKTarget_v44 Implementation
//-----------------------------------------------------------------------------

void CIKTarget_v44::SetOwner( int entindex, const Vector &pos, const QAngle &angles )
{
	latched.owner = entindex;
	latched.absOrigin = pos;
	latched.absAngles = angles;
}

void CIKTarget_v44::ClearOwner( void )
{
	latched.owner = -1;
}

int CIKTarget_v44::GetOwner( void )
{
	return latched.owner;
}

void CIKTarget_v44::UpdateOwner( int entindex, const Vector &pos, const QAngle &angles )
{
	latched.owner = entindex;
	latched.absOrigin = pos;
	latched.absAngles = angles;
}

void CIKTarget_v44::SetPos( const Vector &pos )
{
	est.pos = pos;
}

void CIKTarget_v44::SetAngles( const QAngle &angles )
{
	AngleQuaternion( angles, est.q );
}

void CIKTarget_v44::SetQuaternion( const Quaternion &q )
{
	est.q = q;
}

void CIKTarget_v44::SetNormal( const Vector &normal )
{
	// Recalculate foot angle based on slope of surface
	matrix3x4_t m1;
	Vector forward, right;
	QuaternionMatrix( est.q, m1 );

	MatrixGetColumn( m1, 1, right );
	forward = CrossProduct( right, normal );
	right = CrossProduct( normal, forward );
	MatrixSetColumn( forward, 0, m1 );
	MatrixSetColumn( right, 1, m1 );
	MatrixSetColumn( normal, 2, m1 );
	Vector p1;
	MatrixAngles( m1, est.q, p1 );
}

void CIKTarget_v44::SetPosWithNormalOffset( const Vector &pos, const Vector &normal )
{
	// Assume it's a disc edge intersecting with the floor, so try to estimate the z location of the center
	est.pos = pos;
	if (normal.z > 0.9999f)
	{
		return;
	}
	// Clamp at 45 degrees
	else if (normal.z > 0.707f)
	{
		// tan == sin / cos
		float tan = sqrt( 1 - normal.z * normal.z ) / normal.z;
		est.pos.z = est.pos.z - est.radius * tan;
	}
	else
	{
		est.pos.z = est.pos.z - est.radius;
	}
}

void CIKTarget_v44::SetOnWorld( bool bOnWorld )
{
	est.onWorld = bOnWorld;
}

bool CIKTarget_v44::IsActive( void )
{
	return est.flWeight > 0.0f;
}

void CIKTarget_v44::IKFailed( void )
{
	error.bInError = true;
	error.flTime = 0.0f;
}

void CIKTarget_v44::MoveReferenceFrame( Vector &deltaPos, QAngle &deltaAngles )
{
	// Update all position-based state by the reference frame delta
	est.pos -= deltaPos;
	latched.pos -= deltaPos;
	offset.pos -= deltaPos;
	ideal.pos -= deltaPos;
}

//-----------------------------------------------------------------------------
// v44+ IK Solving Functions
//-----------------------------------------------------------------------------

bool Studio_SolveIK_v44( int iThigh, int iKnee, int iFoot, Vector &targetFoot, matrix3x4_t* pBoneToWorld )
{
	Vector worldFoot, worldKnee, worldThigh;

	MatrixGetColumn( pBoneToWorld[ iThigh ], 3, worldThigh );
	MatrixGetColumn( pBoneToWorld[ iKnee ], 3, worldKnee );
	MatrixGetColumn( pBoneToWorld[ iFoot ], 3, worldFoot );

	Vector ikFoot, ikTargetKnee, ikKnee;

	ikFoot = targetFoot - worldThigh;
	ikKnee = worldKnee - worldThigh;

	float l1 = (worldKnee-worldThigh).Length();
	float l2 = (worldFoot-worldKnee).Length();
	float l3 = (worldFoot-worldThigh).Length();

	Vector ikHalf = (worldFoot-worldThigh) * (l1 / l3);

	// Figure out knee direction
	Vector ikKneeDir = ikKnee - ikHalf;
	VectorNormalize( ikKneeDir );
	ikTargetKnee = ikKnee + ikKneeDir * l1;

	// Leg too straight to figure out knee?
	if (l3 > (l1 + l2) * 0.9998f)
	{
		return false;
	}

	// Too far away?
	if (ikFoot.Length() > (l1 + l2) * 0.9998f)
	{
		VectorNormalize( ikFoot );
		VectorScale( ikFoot, (l1 + l2) * 0.9998f, ikFoot );
	}

	// Too close?
	if (ikFoot.Length() < fabs(l1 - l2) * 1.01f)
	{
		VectorNormalize( ikFoot );
		VectorScale( ikFoot, fabs(l1 - l2) * 1.01f, ikFoot );
	}

	if (ik::solve( l1, l2, ikFoot.Base(), ikTargetKnee.Base(), ikKnee.Base() ))
	{
		matrix3x4_t& mWorldThigh = pBoneToWorld[ iThigh ];
		matrix3x4_t& mWorldKnee = pBoneToWorld[ iKnee ];
		matrix3x4_t& mWorldFoot = pBoneToWorld[ iFoot ];

		Vector tmp1, tmp2, tmp3;

		// Build transformation matrix for thigh
		tmp1 = ikKnee;
		VectorNormalize( tmp1 );
		MatrixSetColumn( tmp1, 0, mWorldThigh );

		MatrixGetColumn( mWorldThigh, 2, tmp3 );
		tmp2 = tmp3.Cross( tmp1 );
		VectorNormalize( tmp2 );
		MatrixSetColumn( tmp2, 1, mWorldThigh );

		tmp3 = tmp1.Cross( tmp2 );
		MatrixSetColumn( tmp3, 2, mWorldThigh );

		// Build transformation matrix for knee
		tmp1 = ikFoot - ikKnee;
		VectorNormalize(tmp1);
		MatrixSetColumn( tmp1, 0, mWorldKnee );

		MatrixGetColumn( mWorldKnee, 2, tmp3 );
		tmp2 = tmp3.Cross( tmp1 );
		VectorNormalize( tmp2 );
		MatrixSetColumn( tmp2, 1, mWorldKnee );

		tmp3 = tmp1.Cross( tmp2 );
		MatrixSetColumn( tmp3, 2, mWorldKnee );

		// Set positions
		mWorldKnee[0][3] = ikKnee.x + worldThigh.x;
		mWorldKnee[1][3] = ikKnee.y + worldThigh.y;
		mWorldKnee[2][3] = ikKnee.z + worldThigh.z;

		mWorldFoot[0][3] = ikFoot.x + worldThigh.x;
		mWorldFoot[1][3] = ikFoot.y + worldThigh.y;
		mWorldFoot[2][3] = ikFoot.z + worldThigh.z;

		return true;
	}
	return false;
}

bool Studio_SolveIK_v44( int iThigh, int iKnee, int iFoot, Vector &targetFoot, Vector &targetKnee, matrix3x4_t* pBoneToWorld )
{
	Vector worldFoot, worldKnee, worldThigh;

	MatrixGetColumn( pBoneToWorld[ iThigh ], 3, worldThigh );
	MatrixGetColumn( pBoneToWorld[ iKnee ], 3, worldKnee );
	MatrixGetColumn( pBoneToWorld[ iFoot ], 3, worldFoot );

	Vector ikFoot, ikTargetKnee, ikKnee;

	ikFoot = targetFoot - worldThigh;
	ikTargetKnee = targetKnee - worldThigh;

	float l1 = (worldKnee-worldThigh).Length();
	float l2 = (worldFoot-worldKnee).Length();

	// Too far away?
	if (ikFoot.Length() > (l1 + l2) * 0.9998f)
	{
		VectorNormalize( ikFoot );
		VectorScale( ikFoot, (l1 + l2) * 0.9998f, ikFoot );
	}

	// Too close?
	if (ikFoot.Length() < fabs(l1 - l2) * 1.01f)
	{
		VectorNormalize( ikFoot );
		VectorScale( ikFoot, fabs(l1 - l2) * 1.01f, ikFoot );
	}

	if (ik::solve( l1, l2, ikFoot.Base(), ikTargetKnee.Base(), ikKnee.Base() ))
	{
		matrix3x4_t& mWorldThigh = pBoneToWorld[ iThigh ];
		matrix3x4_t& mWorldKnee = pBoneToWorld[ iKnee ];
		matrix3x4_t& mWorldFoot = pBoneToWorld[ iFoot ];

		Vector tmp1, tmp2, tmp3;

		// Build transformation matrix for thigh
		tmp1 = ikKnee;
		VectorNormalize( tmp1 );
		MatrixSetColumn( tmp1, 0, mWorldThigh );

		MatrixGetColumn( mWorldThigh, 2, tmp3 );
		tmp2 = tmp3.Cross( tmp1 );
		VectorNormalize( tmp2 );
		MatrixSetColumn( tmp2, 1, mWorldThigh );

		tmp3 = tmp1.Cross( tmp2 );
		MatrixSetColumn( tmp3, 2, mWorldThigh );

		// Build transformation matrix for knee
		tmp1 = ikFoot - ikKnee;
		VectorNormalize(tmp1);
		MatrixSetColumn( tmp1, 0, mWorldKnee );

		MatrixGetColumn( mWorldKnee, 2, tmp3 );
		tmp2 = tmp3.Cross( tmp1 );
		VectorNormalize( tmp2 );
		MatrixSetColumn( tmp2, 1, mWorldKnee );

		tmp3 = tmp1.Cross( tmp2 );
		MatrixSetColumn( tmp3, 2, mWorldKnee );

		// Set positions
		mWorldKnee[0][3] = ikKnee.x + worldThigh.x;
		mWorldKnee[1][3] = ikKnee.y + worldThigh.y;
		mWorldKnee[2][3] = ikKnee.z + worldThigh.z;

		mWorldFoot[0][3] = ikFoot.x + worldThigh.x;
		mWorldFoot[1][3] = ikFoot.y + worldThigh.y;
		mWorldFoot[2][3] = ikFoot.z + worldThigh.z;

		return true;
	}
	return false;
}

bool Studio_SolveIK_v44( int iThigh, int iKnee, int iFoot, Vector &targetFoot, Vector &targetKneePos, Vector &targetKneeDir, matrix3x4_t* pBoneToWorld )
{
	Vector worldFoot, worldKnee, worldThigh;

	MatrixGetColumn( pBoneToWorld[ iThigh ], 3, worldThigh );
	MatrixGetColumn( pBoneToWorld[ iKnee ], 3, worldKnee );
	MatrixGetColumn( pBoneToWorld[ iFoot ], 3, worldFoot );

	Vector ikFoot, ikTargetKnee, ikKnee;

	ikFoot = targetFoot - worldThigh;
	ikKnee = worldKnee - worldThigh;

	float l1 = (worldKnee-worldThigh).Length();
	float l2 = (worldFoot-worldKnee).Length();
	float l3 = (worldFoot-worldThigh).Length();

	// Compute target knee position using knee direction hint
	if (targetKneePos.Length() > 0)
	{
		ikTargetKnee = targetKneePos - worldThigh;
	}
	else if (targetKneeDir.Length() > 0)
	{
		Vector ikHalf = (worldFoot-worldThigh) * (l1 / l3);
		Vector ikKneeDir = ikKnee - ikHalf;
		if (ikKneeDir.Length() < 0.001f)
		{
			ikKneeDir = targetKneeDir;
		}
		VectorNormalize( ikKneeDir );
		ikTargetKnee = ikKnee + ikKneeDir * l1;
	}
	else
	{
		Vector ikHalf = (worldFoot-worldThigh) * (l1 / l3);
		Vector ikKneeDir = ikKnee - ikHalf;
		VectorNormalize( ikKneeDir );
		ikTargetKnee = ikKnee + ikKneeDir * l1;
	}

	// Too far away?
	if (ikFoot.Length() > (l1 + l2) * 0.9998f)
	{
		VectorNormalize( ikFoot );
		VectorScale( ikFoot, (l1 + l2) * 0.9998f, ikFoot );
	}

	// Too close?
	if (ikFoot.Length() < fabs(l1 - l2) * 1.01f)
	{
		VectorNormalize( ikFoot );
		VectorScale( ikFoot, fabs(l1 - l2) * 1.01f, ikFoot );
	}

	if (ik::solve( l1, l2, ikFoot.Base(), ikTargetKnee.Base(), ikKnee.Base() ))
	{
		matrix3x4_t& mWorldThigh = pBoneToWorld[ iThigh ];
		matrix3x4_t& mWorldKnee = pBoneToWorld[ iKnee ];
		matrix3x4_t& mWorldFoot = pBoneToWorld[ iFoot ];

		Vector tmp1, tmp2, tmp3;

		// Build transformation matrix for thigh
		tmp1 = ikKnee;
		VectorNormalize( tmp1 );
		MatrixSetColumn( tmp1, 0, mWorldThigh );

		MatrixGetColumn( mWorldThigh, 2, tmp3 );
		tmp2 = tmp3.Cross( tmp1 );
		VectorNormalize( tmp2 );
		MatrixSetColumn( tmp2, 1, mWorldThigh );

		tmp3 = tmp1.Cross( tmp2 );
		MatrixSetColumn( tmp3, 2, mWorldThigh );

		// Build transformation matrix for knee
		tmp1 = ikFoot - ikKnee;
		VectorNormalize(tmp1);
		MatrixSetColumn( tmp1, 0, mWorldKnee );

		MatrixGetColumn( mWorldKnee, 2, tmp3 );
		tmp2 = tmp3.Cross( tmp1 );
		VectorNormalize( tmp2 );
		MatrixSetColumn( tmp2, 1, mWorldKnee );

		tmp3 = tmp1.Cross( tmp2 );
		MatrixSetColumn( tmp3, 2, mWorldKnee );

		// Set positions
		mWorldKnee[0][3] = ikKnee.x + worldThigh.x;
		mWorldKnee[1][3] = ikKnee.y + worldThigh.y;
		mWorldKnee[2][3] = ikKnee.z + worldThigh.z;

		mWorldFoot[0][3] = ikFoot.x + worldThigh.x;
		mWorldFoot[1][3] = ikFoot.y + worldThigh.y;
		mWorldFoot[2][3] = ikFoot.z + worldThigh.z;

		return true;
	}
	return false;
}

bool Studio_SolveIK_v44( mstudioikchain_v44_t *pikchain, Vector &targetFoot, matrix3x4_t* pBoneToWorld )
{
	if (!pikchain)
		return false;

	// IK chains have exactly 3 links (thigh, knee, foot)
	return Studio_SolveIK_v44( pikchain->pLink(0)->bone, pikchain->pLink(1)->bone, pikchain->pLink(2)->bone, targetFoot, pBoneToWorld );
}

//-----------------------------------------------------------------------------
// Studio_AlignIKMatrix_v44 - Realign matrix X axis to align with specified vector
// From 2007 Source Engine
//-----------------------------------------------------------------------------
void Studio_AlignIKMatrix_v44( matrix3x4_t &mMat, const Vector &vAlignTo )
{
	Vector tmp1, tmp2, tmp3;

	// Column 2 (forward)
	MatrixGetColumn( mMat, 2, tmp3 );

	// Cross with align vector to get perpendicular
	tmp2 = CrossProduct( vAlignTo, tmp3 );
	VectorNormalize( tmp2 );

	// Cross to get final axis
	tmp1 = CrossProduct( tmp3, tmp2 );
	VectorNormalize( tmp1 );

	MatrixSetColumn( tmp1, 0, mMat );
	MatrixSetColumn( tmp2, 1, mMat );
}

float Studio_IKRuleWeight_v44( ikcontextikrule_v44_t &ikRule, float flCycle )
{
	if (flCycle < ikRule.start)
		return 0.0f;
	if (flCycle > ikRule.end)
		return 0.0f;

	float flWeight = 1.0f;

	if (flCycle < ikRule.peak)
	{
		flWeight = (flCycle - ikRule.start) / (ikRule.peak - ikRule.start);
	}
	else if (flCycle > ikRule.tail)
	{
		flWeight = (ikRule.end - flCycle) / (ikRule.end - ikRule.tail);
	}

	return clamp( flWeight, 0.0f, 1.0f );
}

bool Studio_IKShouldLatch_v44( ikcontextikrule_v44_t &ikRule, float flCycle )
{
	if (flCycle < ikRule.start)
		return false;
	if (flCycle >= ikRule.peak && flCycle <= ikRule.tail)
		return true;
	return false;
}

float Studio_IKTail_v44( ikcontextikrule_v44_t &ikRule, float flCycle )
{
	if (flCycle <= ikRule.tail)
		return 0.0f;
	if (flCycle >= ikRule.end)
		return 1.0f;
	return (flCycle - ikRule.tail) / (ikRule.end - ikRule.tail);
}

//-----------------------------------------------------------------------------
// v44+ Animation Extraction Functions
//-----------------------------------------------------------------------------

// Extract a single RLE-compressed animation value (single-precision version)
inline void ExtractAnimValue_v44( int frame, mstudioanimvalue_v44_t *panimvalue, float scale, float &v1 )
{
	if ( !panimvalue )
	{
		v1 = 0;
		return;
	}

	int k = frame;

	// Find the span containing our frame
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

// Extract RLE-compressed animation value with interpolation support
inline void ExtractAnimValue_v44( int frame, mstudioanimvalue_v44_t *panimvalue, float scale, float &v1, float &v2 )
{
	if ( !panimvalue )
	{
		v1 = v2 = 0;
		return;
	}

	int k = frame;

	// Find the span containing our frame
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
			{
				// Repeating the last value
				v2 = v1;
			}
			else
			{
				// Use next span's first value
				v2 = panimvalue[panimvalue->num.valid + 2].value * scale;
			}
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


// Calculate bone quaternion from v44+ animation data
void CalcBoneQuaternion_v44( const studiohdr_v44_t *pStudioHdr, int frame, float s,
	const mstudiobone_v44_t *pbone, const mstudioanim_v44_t *panim, Quaternion &q )
{
	if (panim->flags & STUDIO_ANIM_RAWROT_V44)
	{
		// Compressed quaternion (48-bit)
		q = *(panim->pQuat48());
		Assert( q.IsValid() );
	}
	else if (panim->flags & STUDIO_ANIM_RAWROT2_V44)
	{
		// High-precision compressed quaternion (64-bit)
		q = *(panim->pQuat64());
		Assert( q.IsValid() );
	}
	else if (panim->flags & STUDIO_ANIM_ANIMROT_V44)
	{
		// Animated rotation - RLE compressed values
		mstudioanim_valueptr_v44_t *pRotV = panim->pRotV();

		if (s > 0.001f)
		{
			// Interpolation needed
			Quaternion q1, q2;
			RadianEuler angle1(0, 0, 0), angle2(0, 0, 0);

			for (int j = 0; j < 3; j++)
			{
				if (pRotV->offset[j] == 0)
				{
					// No animation data - use bone default
					angle1[j] = angle2[j] = pbone->rot[j];
				}
				else
				{
					ExtractAnimValue_v44( frame, pRotV->pAnimvalue(j), pbone->rotscale[j], angle1[j], angle2[j] );
					angle1[j] += pbone->rot[j];
					angle2[j] += pbone->rot[j];
				}
			}

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
			// No interpolation needed
			RadianEuler angle;
			for (int j = 0; j < 3; j++)
			{
				if (pRotV->offset[j] == 0)
				{
					angle[j] = pbone->rot[j];
				}
				else
				{
					ExtractAnimValue_v44( frame, pRotV->pAnimvalue(j), pbone->rotscale[j], angle[j] );
					angle[j] += pbone->rot[j];
				}
			}
			AngleQuaternion( angle, q );
		}
	}
	else
	{
		// No animation - use bone's default orientation
		AngleQuaternion( pbone->rot, q );
	}
}


// Calculate bone position from v44+ animation data
void CalcBonePosition_v44( const studiohdr_v44_t *pStudioHdr, int frame, float s,
	const mstudiobone_v44_t *pbone, const mstudioanim_v44_t *panim, Vector &pos )
{
	if (panim->flags & STUDIO_ANIM_RAWPOS_V44)
	{
		// Compressed position (48-bit using float16)
		pos = *(panim->pPos());
		Assert( pos.IsValid() );
	}
	else if (panim->flags & STUDIO_ANIM_ANIMPOS_V44)
	{
		// Animated position - RLE compressed values
		mstudioanim_valueptr_v44_t *pPosV = panim->pPosV();

		if (s > 0.001f)
		{
			// Interpolation needed
			for (int j = 0; j < 3; j++)
			{
				pos[j] = pbone->pos[j];
				if (pPosV->offset[j] != 0)
				{
					float v1, v2;
					ExtractAnimValue_v44( frame, pPosV->pAnimvalue(j), pbone->posscale[j], v1, v2 );
					pos[j] += v1 * (1.0f - s) + v2 * s;
				}
			}
		}
		else
		{
			// No interpolation needed
			for (int j = 0; j < 3; j++)
			{
				pos[j] = pbone->pos[j];
				if (pPosV->offset[j] != 0)
				{
					float v1;
					ExtractAnimValue_v44( frame, pPosV->pAnimvalue(j), pbone->posscale[j], v1 );
					pos[j] += v1;
				}
			}
		}
	}
	else
	{
		// No animation - use bone's default position
		pos = pbone->pos;
	}
}


//-----------------------------------------------------------------------------
// v44+ Bone Setup Functions
//-----------------------------------------------------------------------------

void SlerpBones_v44(
	const studiohdr_v44_t *pStudioHdr,
	Quaternion q1[MAXSTUDIOBONES],
	Vector pos1[MAXSTUDIOBONES],
	const mstudioseqdesc_v44_t *pseqdesc,
	const Quaternion q2[MAXSTUDIOBONES],
	const Vector pos2[MAXSTUDIOBONES],
	float s,
	int boneMask
	)
{
	if (!pStudioHdr || !pseqdesc)
		return;

	int i;
	Quaternion q3;
	float s1, s2;

	if (s <= 0.0f)
		return;
	else if (s > 1.0f)
		s = 1.0f;

	if (pseqdesc->flags & STUDIO_DELTA)
	{
		// Delta animation - additive blending
		for (i = 0; i < pStudioHdr->numbones; i++)
		{
			mstudiobone_v44_t *pbone = pStudioHdr->pBone(i);
			if (!pbone)
				continue;

			// Skip bones not in mask
			if (!(pbone->flags & boneMask))
				continue;

			// Get per-bone weight from sequence
			s2 = s * pseqdesc->weight(i);
			if (s2 > 0.0f)
			{
				if (pseqdesc->flags & STUDIO_POST)
				{
					QuaternionMA( q1[i], s2, q2[i], q1[i] );
					pos1[i][0] = pos1[i][0] + pos2[i][0] * s2;
					pos1[i][1] = pos1[i][1] + pos2[i][1] * s2;
					pos1[i][2] = pos1[i][2] + pos2[i][2] * s2;
				}
				else
				{
					QuaternionSM( s2, q2[i], q1[i], q1[i] );
					pos1[i][0] = pos1[i][0] + pos2[i][0] * s2;
					pos1[i][1] = pos1[i][1] + pos2[i][1] * s2;
					pos1[i][2] = pos1[i][2] + pos2[i][2] * s2;
				}
			}
		}
	}
	else
	{
		// Normal animation - interpolated blending
		for (i = 0; i < pStudioHdr->numbones; i++)
		{
			mstudiobone_v44_t *pbone = pStudioHdr->pBone(i);
			if (!pbone)
				continue;

			// Skip bones not in mask
			if (!(pbone->flags & boneMask))
				continue;

			// Get per-bone weight from sequence
			s2 = s * pseqdesc->weight(i);
			if (s2 > 0.0f)
			{
				s1 = 1.0f - s2;

				if (pbone->flags & BONE_FIXED_ALIGNMENT)
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

void InitPose_v44(
	const studiohdr_v44_t *pStudioHdr,
	Vector pos[MAXSTUDIOBONES],
	Quaternion q[MAXSTUDIOBONES]
	)
{
	if (!pStudioHdr)
		return;

	// Initialize all bones to their default positions and orientations
	for (int i = 0; i < pStudioHdr->numbones; i++)
	{
		mstudiobone_v44_t *pBone = pStudioHdr->pBone(i);
		if (pBone)
		{
			pos[i] = pBone->pos;
			AngleQuaternion(pBone->rot, q[i]);
		}
		else
		{
			pos[i].Init();
			q[i].Init();
		}
	}
}

//-----------------------------------------------------------------------------
// v44+ CalcRotations - Extract bone positions and rotations from animation data
// This is the core animation extraction function for v44+ models
//-----------------------------------------------------------------------------
static void CalcRotations_v44(
	const studiohdr_v44_t *pStudioHdr,
	Vector pos[],
	Quaternion q[],
	const mstudioseqdesc_v44_t *pseqdesc,
	const mstudioanimdesc_v44_t *panimdesc,
	float cycle,
	int boneMask
	)
{
	if (!pStudioHdr || !pseqdesc || !panimdesc)
		return;

	// Calculate frame and interpolation value
	int iFrame;
	float s;
	float fFrame = cycle * (panimdesc->numframes - 1);
	iFrame = (int)fFrame;
	s = (fFrame - iFrame);

	// Initialize ALL bones to their default pose first
	for (int i = 0; i < pStudioHdr->numbones; i++)
	{
		mstudiobone_v44_t *pbone = pStudioHdr->pBone(i);
		if (pbone)
		{
			pos[i] = pbone->pos;
			AngleQuaternion(pbone->rot, q[i]);
		}
		else
		{
			pos[i].Init();
			q[i].Init();
		}
	}

	// Get animation data pointer
	mstudioanim_v44_t *panim = panimdesc->pAnim(&iFrame);
	if (!panim)
		return;

	// Traverse the animation linked list and apply animated values
	while (panim)
	{
		int iBone = panim->bone;

		// Make sure bone index is valid
		if (iBone >= 0 && iBone < pStudioHdr->numbones)
		{
			mstudiobone_v44_t *pbone = pStudioHdr->pBone(iBone);
			if (pbone && (pseqdesc->weight(iBone) > 0.0f) && (pbone->flags & boneMask))
			{
				CalcBoneQuaternion_v44(pStudioHdr, iFrame, s, pbone, panim, q[iBone]);
				CalcBonePosition_v44(pStudioHdr, iFrame, s, pbone, panim, pos[iBone]);
			}
		}

		// Move to next bone in the linked list
		panim = panim->pNext();
	}
}


//-----------------------------------------------------------------------------
// v44+ CalcPoseSingle - Calculate bone pose for a single sequence
//-----------------------------------------------------------------------------
static bool CalcPoseSingle_v44(
	const studiohdr_v44_t *pStudioHdr,
	Vector pos[],
	Quaternion q[],
	int sequence,
	float cycle,
	int boneMask
	)
{
	if (!pStudioHdr)
		return false;

	// Validate sequence
	if (sequence < 0 || sequence >= pStudioHdr->numlocalseq)
	{
		sequence = 0;
	}

	mstudioseqdesc_v44_t *pseqdesc = pStudioHdr->pLocalSeqdesc(sequence);
	if (!pseqdesc)
		return false;

	// Handle cycle wrapping
	if (cycle < 0 || cycle >= 1)
	{
		if (pseqdesc->flags & STUDIO_LOOPING)
		{
			cycle = cycle - (int)cycle;
			if (cycle < 0) cycle += 1;
		}
		else
		{
			cycle = clamp(cycle, 0.0f, 0.9999f);
		}
	}

	// For now, just use the first animation blend (index 0,0)
	// Full implementation would handle blend parameters
	mstudioanimdesc_v44_t *panim = pStudioHdr->pLocalAnimdesc(pseqdesc->anim(0, 0));
	if (!panim)
	{
		// No animation data - just return default pose
		InitPose_v44(pStudioHdr, pos, q);
		return false;
	}

	CalcRotations_v44(pStudioHdr, pos, q, pseqdesc, panim, cycle, boneMask);
	return true;
}


void CalcPose_v44(
	const studiohdr_v44_t *pStudioHdr,
	CIKContext_v44 *pIKContext,
	Vector pos[MAXSTUDIOBONES],
	Quaternion q[MAXSTUDIOBONES],
	int sequence,
	float cycle,
	const float poseParameter[],
	int boneMask,
	float flWeight
	)
{
	if (!pStudioHdr)
		return;

	// Clamp weight
	flWeight = clamp(flWeight, 0.0f, 1.0f);

	// Add IK dependencies if context is provided
	if (pIKContext)
	{
		pIKContext->AddDependencies(sequence, cycle, poseParameter, flWeight);
	}

	// Calculate the pose
	CalcPoseSingle_v44(pStudioHdr, pos, q, sequence, cycle, boneMask);
}

void AccumulatePose_v44(
	const studiohdr_v44_t *pStudioHdr,
	CIKContext_v44 *pIKContext,
	Vector pos[MAXSTUDIOBONES],
	Quaternion q[MAXSTUDIOBONES],
	int sequence,
	float cycle,
	const float poseParameter[],
	int boneMask,
	float flWeight
	)
{
	if (!pStudioHdr)
		return;

	// If weight is 0, don't bother calculating
	if (flWeight <= 0.0f)
		return;

	// Clamp weight
	flWeight = clamp(flWeight, 0.0f, 1.0f);

	// Add IK dependencies if context is provided
	if (pIKContext)
	{
		pIKContext->AddDependencies(sequence, cycle, poseParameter, flWeight);
	}

	// Validate sequence
	if (sequence < 0 || sequence >= pStudioHdr->numlocalseq)
	{
		return;
	}

	mstudioseqdesc_v44_t *pseqdesc = pStudioHdr->pLocalSeqdesc(sequence);
	if (!pseqdesc)
		return;

	// Calculate pose for this sequence
	Vector pos2[MAXSTUDIOBONES];
	Quaternion q2[MAXSTUDIOBONES];

	if (!CalcPoseSingle_v44(pStudioHdr, pos2, q2, sequence, cycle, boneMask))
	{
		return;
	}

	// Blend the calculated pose with the current pose using SlerpBones
	SlerpBones_v44(pStudioHdr, q, pos, pseqdesc, q2, pos2, flWeight, boneMask);
}

void CalcBoneAdj_v44(
	const studiohdr_v44_t *pStudioHdr,
	Vector pos[],
	Quaternion q[],
	const float controllers[],
	int boneMask
	)
{
	if (!pStudioHdr || !controllers)
		return;

	int j, k;
	float value;
	RadianEuler a0;
	Quaternion q0;

	for (j = 0; j < pStudioHdr->numbonecontrollers; j++)
	{
		mstudiobonecontroller_v44_t *pbonecontroller = pStudioHdr->pBonecontroller(j);
		if (!pbonecontroller)
			continue;

		k = pbonecontroller->bone;
		if (k < 0 || k >= pStudioHdr->numbones)
			continue;

		mstudiobone_v44_t *pbone = pStudioHdr->pBone(k);
		if (!pbone || !(pbone->flags & boneMask))
			continue;

		int i = pbonecontroller->inputfield;
		value = controllers[i];
		value = clamp(value, 0.0f, 1.0f);
		value = (1.0f - value) * pbonecontroller->start + value * pbonecontroller->end;

		switch(pbonecontroller->type & STUDIO_TYPES)
		{
		case STUDIO_XR:
			a0.Init( value * (M_PI / 180.0f), 0, 0 );
			AngleQuaternion( a0, q0 );
			QuaternionSM( 1.0f, q0, q[k], q[k] );
			break;
		case STUDIO_YR:
			a0.Init( 0, value * (M_PI / 180.0f), 0 );
			AngleQuaternion( a0, q0 );
			QuaternionSM( 1.0f, q0, q[k], q[k] );
			break;
		case STUDIO_ZR:
			a0.Init( 0, 0, value * (M_PI / 180.0f) );
			AngleQuaternion( a0, q0 );
			QuaternionSM( 1.0f, q0, q[k], q[k] );
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

void CalcAutoplaySequences_v44(
	const studiohdr_v44_t *pStudioHdr,
	CIKContext_v44 *pIKContext,
	Vector pos[],
	Quaternion q[],
	const float poseParameters[],
	int boneMask,
	float time
	)
{
	if (!pStudioHdr)
		return;

	// Add autoplay locks if IK context exists
	if (pIKContext)
	{
		pIKContext->AddAutoplayLocks(pos, q);
	}

	// Process all sequences looking for STUDIO_AUTOPLAY flag
	for (int i = 0; i < pStudioHdr->numlocalseq; i++)
	{
		mstudioseqdesc_v44_t *pseqdesc = pStudioHdr->pLocalSeqdesc(i);
		if (!pseqdesc)
			continue;

		if (pseqdesc->flags & STUDIO_AUTOPLAY)
		{
			// Calculate cycle from time and animation fps
			float cycle = 0;

			// Get first animation descriptor for this sequence
			mstudioanimdesc_v44_t *panim = pStudioHdr->pLocalAnimdesc(pseqdesc->anim(0, 0));
			if (panim && panim->numframes > 1)
			{
				// Calculate cycles per second
				float cps = panim->fps / (float)(panim->numframes - 1);
				cycle = time * cps;
				cycle = cycle - (int)cycle;  // Wrap to [0,1)
			}

			// Accumulate this autoplay sequence
			AccumulatePose_v44(pStudioHdr, NULL, pos, q, i, cycle, poseParameters, boneMask, 1.0f);
		}
	}

	// Solve autoplay locks if IK context exists
	if (pIKContext)
	{
		pIKContext->SolveAutoplayLocks(pos, q);
	}
}

//-----------------------------------------------------------------------------
// v44+ DoAxisInterpBone - Axis interpolation procedural bone
//-----------------------------------------------------------------------------
static void DoAxisInterpBone_v44(
	const studiohdr_v44_t *pStudioHdr,
	int iBone,
	matrix3x4_t *bonetoworld
	)
{
	mstudiobone_v44_t *pbone = pStudioHdr->pBone(iBone);
	if (!pbone)
		return;

	mstudioaxisinterpbone_v44_t *pProc = (mstudioaxisinterpbone_v44_t *)pbone->pProcedure();
	if (!pProc)
		return;

	// Get control bone's parent
	mstudiobone_v44_t *pControlBone = pStudioHdr->pBone(pProc->control);
	if (!pControlBone)
		return;

	Vector control;
	int iControlParent = pControlBone->parent;

	if (iControlParent != -1)
	{
		Vector tmp;
		// Pull out the control column
		tmp.x = bonetoworld[pProc->control][0][pProc->axis];
		tmp.y = bonetoworld[pProc->control][1][pProc->axis];
		tmp.z = bonetoworld[pProc->control][2][pProc->axis];

		// Invert it back into parent's space
		VectorIRotate(tmp, bonetoworld[iControlParent], control);
	}
	else
	{
		// Pull out the control column
		control.x = bonetoworld[pProc->control][0][pProc->axis];
		control.y = bonetoworld[pProc->control][1][pProc->axis];
		control.z = bonetoworld[pProc->control][2][pProc->axis];
	}

	// Find axial control inputs
	Quaternion *q1, *q2, *q3;
	Vector *p1, *p2, *p3;
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

	// Do a three-way blend
	Vector p;
	Quaternion v, tmp;
	float t = a1 + a2 + a3;
	if (t == 0.0f)
	{
		AngleQuaternion(pbone->rot, v);
		p = pbone->pos;
	}
	else
	{
		a1 = a1 / t;
		a2 = a2 / t;
		a3 = a3 / t;

		QuaternionSlerp(*q2, *q1, a1 / (a1 + a2), tmp);
		QuaternionSlerp(tmp, *q3, a3 * t, v);

		p = *p1 * a1 + *p2 * a2 + *p3 * a3;
	}

	matrix3x4_t bonematrix;
	QuaternionMatrix(v, p, bonematrix);

	// Get bone's parent
	if (pbone->parent == -1)
	{
		MatrixCopy(bonematrix, bonetoworld[iBone]);
	}
	else
	{
		ConcatTransforms(bonetoworld[pbone->parent], bonematrix, bonetoworld[iBone]);
	}
}


//-----------------------------------------------------------------------------
// v44+ DoQuatInterpBone - Quaternion interpolation procedural bone
//-----------------------------------------------------------------------------
static void DoQuatInterpBone_v44(
	const studiohdr_v44_t *pStudioHdr,
	int iBone,
	matrix3x4_t *bonetoworld
	)
{
	mstudiobone_v44_t *pbone = pStudioHdr->pBone(iBone);
	if (!pbone)
		return;

	mstudioquatinterpbone_v44_t *pProc = (mstudioquatinterpbone_v44_t *)pbone->pProcedure();
	if (!pProc)
		return;

	// Get control bone's parent
	mstudiobone_v44_t *pControlBone = pStudioHdr->pBone(pProc->control);
	if (!pControlBone)
		return;

	// Get control quaternion
	Quaternion controlQuat;
	int iControlParent = pControlBone->parent;

	if (iControlParent != -1)
	{
		matrix3x4_t tmpmatrix, controlmatrix;
		MatrixInvert(bonetoworld[iControlParent], tmpmatrix);
		ConcatTransforms(tmpmatrix, bonetoworld[pProc->control], controlmatrix);
		MatrixQuaternion(controlmatrix, controlQuat);
	}
	else
	{
		MatrixQuaternion(bonetoworld[pProc->control], controlQuat);
	}

	// Simple interpolation - just use first trigger
	// Full implementation would search through all triggers
	Quaternion q;
	Vector pos;

	if (pProc->numtriggers > 0)
	{
		mstudioquatinterpinfo_v44_t *pTrigger = pProc->pTrigger(0);
		if (pTrigger)
		{
			q = pTrigger->quat;
			pos = pTrigger->pos;
		}
		else
		{
			AngleQuaternion(pbone->rot, q);
			pos = pbone->pos;
		}
	}
	else
	{
		AngleQuaternion(pbone->rot, q);
		pos = pbone->pos;
	}

	matrix3x4_t bonematrix;
	QuaternionMatrix(q, pos, bonematrix);

	if (pbone->parent == -1)
	{
		MatrixCopy(bonematrix, bonetoworld[iBone]);
	}
	else
	{
		ConcatTransforms(bonetoworld[pbone->parent], bonematrix, bonetoworld[iBone]);
	}
}


bool CalcProceduralBone_v44(
	const studiohdr_v44_t *pStudioHdr,
	int iBone,
	matrix3x4_t *bonetoworld
	)
{
	if (!pStudioHdr || iBone < 0 || iBone >= pStudioHdr->numbones)
		return false;

	mstudiobone_v44_t *pbone = pStudioHdr->pBone(iBone);
	if (!pbone)
		return false;

	if (pbone->flags & BONE_ALWAYS_PROCEDURAL_V44)
	{
		switch (pbone->proctype)
		{
		case STUDIO_PROC_AXISINTERP:
			DoAxisInterpBone_v44(pStudioHdr, iBone, bonetoworld);
			return true;

		case STUDIO_PROC_QUATINTERP:
			DoQuatInterpBone_v44(pStudioHdr, iBone, bonetoworld);
			return true;

		// TODO: Implement these when needed
		case STUDIO_PROC_AIMATBONE:
		case STUDIO_PROC_AIMATATTACH:
		case STUDIO_PROC_JIGGLE:
			// Not yet implemented for v44+
			return false;

		default:
			return false;
		}
	}
	return false;
}

void Studio_BuildMatrices_v44(
	const studiohdr_v44_t *pStudioHdr,
	const QAngle& angles,
	const Vector& origin,
	const Vector pos[],
	const Quaternion q[],
	int iBone,
	matrix3x4_t bonetoworld[MAXSTUDIOBONES],
	int boneMask
	)
{
	if (!pStudioHdr)
		return;

	int i, j;
	int chain[MAXSTUDIOBONES];
	int chainlength = 0;

	// Build parent chain from target bone back to root
	if (iBone < 0)
	{
		// Build all bones
		chainlength = 0;
		for (i = 0; i < pStudioHdr->numbones; i++)
		{
			chain[chainlength++] = i;
		}
	}
	else if (iBone < pStudioHdr->numbones)
	{
		// Build bone chain from specified bone to root
		i = iBone;
		while (i != -1)
		{
			chain[chainlength++] = i;
			mstudiobone_v44_t *pBone = pStudioHdr->pBone(i);
			if (pBone)
				i = pBone->parent;
			else
				break;
		}
	}
	else
	{
		return;
	}

	// Create root transform from angles and origin
	matrix3x4_t rootXform;
	AngleMatrix(angles, origin, rootXform);

	// Build bone transforms from chain (root to leaf)
	for (j = chainlength - 1; j >= 0; j--)
	{
		i = chain[j];
		mstudiobone_v44_t *pBone = pStudioHdr->pBone(i);
		if (!pBone)
			continue;

		// Skip if bone mask doesn't match
		if (boneMask && !(pBone->flags & boneMask))
		{
			continue;
		}

		// Build local bone transform from position and quaternion
		matrix3x4_t bonematrix;
		QuaternionMatrix(q[i], pos[i], bonematrix);

		// Concatenate with parent transform
		if (pBone->parent == -1)
		{
			ConcatTransforms(rootXform, bonematrix, bonetoworld[i]);
		}
		else
		{
			ConcatTransforms(bonetoworld[pBone->parent], bonematrix, bonetoworld[i]);
		}
	}
}

//-----------------------------------------------------------------------------
// World-space blending functions (2007 engine) - v44+
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// BuildBoneChain_v44 - static version for WorldSpaceSlerp_v44
//-----------------------------------------------------------------------------
static void BuildBoneChain_Static_v44(
	const studiohdr_v44_t *pStudioHdr,
	const matrix3x4_t &rootxform,
	const Vector pos[],
	const Quaternion q[],
	int iBone,
	matrix3x4_t *pBoneToWorld,
	CBoneBitList_v44 &boneComputed )
{
	if (!pStudioHdr || iBone < 0 || iBone >= pStudioHdr->numbones)
		return;

	// Already computed?
	if (boneComputed.IsBoneMarked(iBone))
		return;

	matrix3x4_t bonematrix;
	QuaternionMatrix( q[iBone], pos[iBone], bonematrix );

	mstudiobone_v44_t *pBone = pStudioHdr->pBone( iBone );
	if (!pBone)
		return;

	int parent = pBone->parent;
	if (parent == -1)
	{
		ConcatTransforms( rootxform, bonematrix, pBoneToWorld[iBone] );
	}
	else
	{
		// Recursively build parent chain first
		BuildBoneChain_Static_v44( pStudioHdr, rootxform, pos, q, parent, pBoneToWorld, boneComputed );
		ConcatTransforms( pBoneToWorld[parent], bonematrix, pBoneToWorld[iBone] );
	}
	boneComputed.MarkBone(iBone);
}

//-----------------------------------------------------------------------------
// Purpose: blend together in world space q1,pos1 with q2,pos2 (v44+)
//			Return result in q1,pos1. 0 returns q1,pos1. 1 returns q2,pos2
//-----------------------------------------------------------------------------
void WorldSpaceSlerp_v44(
	const studiohdr_v44_t *pStudioHdr,
	Quaternion q1[MAXSTUDIOBONES],
	Vector pos1[MAXSTUDIOBONES],
	const mstudioseqdesc_v48_t *pseqdesc,
	int sequence,
	const Quaternion q2[MAXSTUDIOBONES],
	const Vector pos2[MAXSTUDIOBONES],
	float s,
	int boneMask
	)
{
	if (!pStudioHdr || !pseqdesc)
		return;

	if (s <= 0.0f)
		return;
	if (s > 1.0f)
		s = 1.0f;

	// Make fake root transform
	matrix3x4_t rootXform;
	SetIdentityMatrix( rootXform );

	// Use static arrays for matrices (no g_MatrixPool in LeakNet)
	static matrix3x4_t srcBoneToWorld[MAXSTUDIOBONES];
	static matrix3x4_t destBoneToWorld[MAXSTUDIOBONES];
	static matrix3x4_t targetBoneToWorld[MAXSTUDIOBONES];

	CBoneBitList_v44 srcBoneComputed;
	CBoneBitList_v44 destBoneComputed;

	for (int i = 0; i < pStudioHdr->numbones; i++)
	{
		mstudiobone_v44_t *pBone = pStudioHdr->pBone( i );
		if (!pBone)
			continue;

		// Skip unused bones
		if (!(pBone->flags & boneMask))
			continue;

		int parent = pBone->parent;
		float s1 = 0.0f;	// weight of parent for q2, pos2

		// Get bone weight from sequence
		float *pBoneWeight = pseqdesc->pBoneweight( i );
		float s2 = s * (pBoneWeight ? *pBoneWeight : 1.0f);

		if (parent != -1)
		{
			float *pParentWeight = pseqdesc->pBoneweight( parent );
			s1 = s * (pParentWeight ? *pParentWeight : 1.0f);
		}

		if (s1 == 1.0f && s2 == 1.0f)
		{
			pos1[i] = pos2[i];
			q1[i] = q2[i];
		}
		else if (s2 > 0.0f)
		{
			Quaternion srcQ, destQ;
			Vector srcPos, destPos;
			Quaternion targetQ;
			Vector tmp;

			BuildBoneChain_Static_v44( pStudioHdr, rootXform, pos1, q1, i, destBoneToWorld, destBoneComputed );
			BuildBoneChain_Static_v44( pStudioHdr, rootXform, pos2, q2, i, srcBoneToWorld, srcBoneComputed );

			MatrixAngles( destBoneToWorld[i], destQ, destPos );
			MatrixAngles( srcBoneToWorld[i], srcQ, srcPos );

			QuaternionSlerp( destQ, srcQ, s2, targetQ );
			AngleMatrix( targetQ, destPos, targetBoneToWorld[i] );

			// Back solve
			if (parent == -1)
			{
				MatrixAngles( targetBoneToWorld[i], q1[i], tmp );
			}
			else
			{
				matrix3x4_t worldToBone;
				MatrixInvert( targetBoneToWorld[parent], worldToBone );

				matrix3x4_t local;
				ConcatTransforms( worldToBone, targetBoneToWorld[i], local );
				MatrixAngles( local, q1[i], tmp );

				// Blend bone lengths (local space)
				pos1[i] = Lerp( s2, pos1[i], pos2[i] );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// QuaternionIdentityBlend_v44 - blend toward identity quaternion (local helper)
//-----------------------------------------------------------------------------
static void QuaternionIdentityBlend_v44( const Quaternion &p, float t, Quaternion &qt )
{
	float sclp = 1.0f - t;

	qt.x = p.x * sclp;
	qt.y = p.y * sclp;
	qt.z = p.z * sclp;
	if (qt.w < 0.0)
	{
		qt.w = p.w * sclp - t;
	}
	else
	{
		qt.w = p.w * sclp + t;
	}
	QuaternionNormalize( qt );
}

//-----------------------------------------------------------------------------
// Purpose: Scale a set of bones. Must be of type delta (v44+)
//-----------------------------------------------------------------------------
void ScaleBones_v44(
	const studiohdr_v44_t *pStudioHdr,
	Quaternion q[MAXSTUDIOBONES],
	Vector pos[MAXSTUDIOBONES],
	int sequence,
	float s,
	int boneMask
	)
{
	if (!pStudioHdr || sequence < 0 || sequence >= pStudioHdr->numlocalseq)
		return;

	mstudioseqdesc_v44_t *pseqdesc = pStudioHdr->pLocalSeqdesc( sequence );
	if (!pseqdesc)
		return;

	float s2 = s;
	float s1 = 1.0f - s2;

	for (int i = 0; i < pStudioHdr->numbones; i++)
	{
		// Skip unused bones
		mstudiobone_v44_t *pBone = pStudioHdr->pBone( i );
		if (!pBone)
			continue;

		if (!(pBone->flags & boneMask))
			continue;

		// Get bone weight from sequence
		float *pBoneWeight = pseqdesc->pBoneweight( i );
		float weight = pBoneWeight ? *pBoneWeight : 1.0f;

		if (weight > 0.0f)
		{
			QuaternionIdentityBlend_v44( q[i], s1, q[i] );
			VectorScale( pos[i], s2, pos[i] );
		}
	}
}
