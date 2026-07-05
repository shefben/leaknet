//========= Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: v44+ Bone setup and IK system - COMPLETELY ISOLATED FROM V37
// This file is for v44-v48 model support only. Does not touch v37 code.
// From 2007 Source Engine
//
// $NoKeywords: $
//=============================================================================//

#ifndef BONE_SETUP_V44_H
#define BONE_SETUP_V44_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/mathlib.h"
#include "utlvector.h"
#include "utllinkedlist.h"
#include "studiohdr_v44.h"

// Forward declarations - v44+ specific classes
class CIKContext_v44;
class CIKTarget_v44;
class CJiggleBones;
class CBoneAccessor;

//-----------------------------------------------------------------------------
// IK chain result - accumulated offset from ideal footplant location (v44+)
//-----------------------------------------------------------------------------
struct ikchainresult_v44_t
{
	int			target;
	Vector		pos;
	Quaternion	q;
	float		flWeight;
};

//-----------------------------------------------------------------------------
// IK context rule - virtual IK rules, filtered and combined from each sequence (v44+)
//-----------------------------------------------------------------------------
struct ikcontextikrule_v44_t
{
	int			index;
	int			type;
	int			chain;
	int			bone;
	int			slot;			// iktarget slot. Usually same as chain.
	float		height;
	float		radius;
	float		floor;
	Vector		pos;
	Quaternion	q;

	float		start;			// beginning of influence
	float		peak;			// start of full influence
	float		tail;			// end of full influence
	float		end;			// end of all influence

	float		top;
	float		drop;
	float		commit;			// frame footstep target should be committed
	float		release;		// frame ankle should end rotation from latched orientation

	float		flWeight;		// processed version of start-end cycle
	float		flRuleWeight;	// blending weight
	float		latched;		// does the IK rule use a latched value?
	char		*szLabel;

	Vector		kneeDir;
	Vector		kneePos;

	ikcontextikrule_v44_t() { memset(this, 0, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// IK target structure (v44+)
//-----------------------------------------------------------------------------
struct iktarget_v44_t
{
	struct {
		float		time;
		Vector		pos;
		Quaternion	q;
	} latched;
	struct {
		Vector		pos;
		Quaternion	q;
	} local;
	struct {
		float		time;
		Vector		pos;
		Quaternion	q;
	} prev;
	struct {
		float		latched;
		float		height;
		float		floor;
		float		radius;
		float		flTime;
		Vector		pos;
		Quaternion	q;
		float		flWeight;
		bool		onWorld;
	} est;
};

//-----------------------------------------------------------------------------
// CIKTarget_v44 - Full IK target class from 2007 engine (v44+)
// Stores IK target state with latching, error tracking, and frame counter
//-----------------------------------------------------------------------------
class CIKTarget_v44
{
public:
	void SetOwner( int entindex, const Vector &pos, const QAngle &angles );
	void ClearOwner( void );
	int GetOwner( void );
	void UpdateOwner( int entindex, const Vector &pos, const QAngle &angles );
	void SetPos( const Vector &pos );
	void SetAngles( const QAngle &angles );
	void SetQuaternion( const Quaternion &q );
	void SetNormal( const Vector &normal );
	void SetPosWithNormalOffset( const Vector &pos, const Vector &normal );
	void SetOnWorld( bool bOnWorld = true );

	bool IsActive( void );
	void IKFailed( void );

	int chain;
	int type;

	void MoveReferenceFrame( Vector &deltaPos, QAngle &deltaAngles );

	// Accumulated offset from ideal footplant location
	struct {
		char		*pAttachmentName;
		Vector		pos;
		Quaternion	q;
	} offset;

private:
	struct {
		Vector		pos;
		Quaternion	q;
	} ideal;

public:
	struct {
		float		latched;
		float		release;
		float		height;
		float		floor;
		float		radius;
		float		flTime;
		float		flWeight;
		Vector		pos;
		Quaternion	q;
		bool		onWorld;
	} est;	// estimate contact position

	struct {
		float		hipToFoot;		// distance from hip
		float		hipToKnee;		// distance from hip to knee
		float		kneeToFoot;		// distance from knee to foot
		Vector		hip;			// location of hip
		Vector		closest;		// closest valid location from hip to foot
		Vector		knee;			// pre-ik location of knee
		Vector		farthest;		// farthest valid location from hip to foot
		Vector		lowest;			// lowest position directly below hip
	} trace;

private:
	// Internally latched footset, position
	struct {
		bool		bNeedsLatch;
		bool		bHasLatch;
		float		influence;
		int			iFramecounter;
		int			owner;
		Vector		absOrigin;
		QAngle		absAngles;
		Vector		pos;
		Quaternion	q;
		Vector		deltaPos;		// accumulated error
		Quaternion	deltaQ;
		Vector		debouncePos;
		Quaternion	debounceQ;
	} latched;

	struct {
		float		flTime;			// time last error was detected
		float		flErrorTime;
		float		ramp;
		bool		bInError;
	} error;

	friend class CIKContext_v44;
};

//-----------------------------------------------------------------------------
// CBoneBitList_v44 - Tracks which bones have been computed (v44+)
//-----------------------------------------------------------------------------
class CBoneBitList_v44
{
public:
	CBoneBitList_v44() { memset(m_bits, 0, sizeof(m_bits)); }

	inline void MarkBone( int iBone )
	{
		m_bits[iBone >> 5] |= (1 << (iBone & 31));
	}

	inline bool IsBoneMarked( int iBone ) const
	{
		return (m_bits[iBone >> 5] & (1 << (iBone & 31))) != 0;
	}

	inline void ClearAll()
	{
		memset(m_bits, 0, sizeof(m_bits));
	}

private:
	unsigned int m_bits[(MAXSTUDIOBONES + 31) / 32];
};

//-----------------------------------------------------------------------------
// CIKContext_v44 - Master IK controller for v44+ models
// Manages all IK chains, targets, locks, and dependencies
//-----------------------------------------------------------------------------
class CIKContext_v44
{
public:
	CIKContext_v44();
	void Init( const studiohdr_v44_t *pStudioHdr, const QAngle &angles, const Vector &pos, float flTime, int iFramecounter = 0, int boneMask = 0 );
	void AddDependencies( int iSequence, float flCycle, const float poseParameters[], float flWeight = 1.0f );

	void ClearTargets( void );
	void UpdateTargets( Vector pos[], Quaternion q[], matrix3x4_t boneToWorld[], CBoneBitList_v44 &boneComputed );
	void AutoIKRelease( void );
	void SolveDependencies( Vector pos[], Quaternion q[], matrix3x4_t boneToWorld[], CBoneBitList_v44 &boneComputed );

	// Legacy interface (no boneComputed tracking)
	void SolveDependencies( Vector pos[], Quaternion q[] );

	void AddAutoplayLocks( Vector pos[], Quaternion q[] );
	void SolveAutoplayLocks( Vector pos[], Quaternion q[] );

	void AddSequenceLocks( mstudioseqdesc_v48_t *pSeqDesc, Vector pos[], Quaternion q[] );
	void SolveSequenceLocks( mstudioseqdesc_v48_t *pSeqDesc, Vector pos[], Quaternion q[] );

	void AddAllLocks( Vector pos[], Quaternion q[] );
	void SolveAllLocks( Vector pos[], Quaternion q[] );

	void SolveLock( const mstudioiklock_v44_t *plock, int i, Vector pos[], Quaternion q[],
		matrix3x4_t boneToWorld[], CBoneBitList_v44 &boneComputed );

	// IK targets - fixed size array for 12 targets
	CIKTarget_v44 m_target[12];
	int m_targetCount;

private:
	studiohdr_v44_t const *m_pStudioHdr;

	bool Estimate( int iSequence, float flCycle, int iTarget, const float poseParameter[], float flWeight = 1.0f );
	void BuildBoneChain( const Vector pos[], const Quaternion q[], int iBone,
		matrix3x4_t *pBoneToWorld, CBoneBitList_v44 &boneComputed );

	// Virtual IK rules, filtered and combined from each sequence
	CUtlVector< CUtlVector< ikcontextikrule_v44_t > > m_ikChainRule;
	CUtlVector< ikcontextikrule_v44_t > m_ikLock;

	matrix3x4_t m_rootxform;

	int m_iFramecounter;
	float m_flPrevTime;
	float m_flTime;
	int m_boneMask;
};

//-----------------------------------------------------------------------------
// v44+ IK Solving functions
//-----------------------------------------------------------------------------
bool Studio_SolveIK_v44( int iThigh, int iKnee, int iFoot, Vector &targetFoot, matrix3x4_t* pBoneToWorld );
bool Studio_SolveIK_v44( int iThigh, int iKnee, int iFoot, Vector &targetFoot, Vector &targetKnee, matrix3x4_t* pBoneToWorld );
bool Studio_SolveIK_v44( int iThigh, int iKnee, int iFoot, Vector &targetFoot, Vector &targetKneePos, Vector &targetKneeDir, matrix3x4_t* pBoneToWorld );
bool Studio_SolveIK_v44( mstudioikchain_v44_t *pikchain, Vector &targetFoot, matrix3x4_t* pBoneToWorld );

// Realign matrix X axis to align with specified vector
void Studio_AlignIKMatrix_v44( matrix3x4_t &mMat, const Vector &vAlignTo );

// IK rule weight calculation
float Studio_IKRuleWeight_v44( ikcontextikrule_v44_t &ikRule, float flCycle );

// IK latching helpers
bool Studio_IKShouldLatch_v44( ikcontextikrule_v44_t &ikRule, float flCycle );
float Studio_IKTail_v44( ikcontextikrule_v44_t &ikRule, float flCycle );

//-----------------------------------------------------------------------------
// v44+ Bone setup functions
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
	);

void InitPose_v44(
	const studiohdr_v44_t *pStudioHdr,
	Vector pos[MAXSTUDIOBONES],
	Quaternion q[MAXSTUDIOBONES]
	);

void CalcPose_v44(
	const studiohdr_v44_t *pStudioHdr,
	CIKContext_v44 *pIKContext,
	Vector pos[MAXSTUDIOBONES],
	Quaternion q[MAXSTUDIOBONES],
	int sequence,
	float cycle,
	const float poseParameter[],
	int boneMask,
	float flWeight = 1.0f
	);

void AccumulatePose_v44(
	const studiohdr_v44_t *pStudioHdr,
	CIKContext_v44 *pIKContext,
	Vector pos[MAXSTUDIOBONES],
	Quaternion q[MAXSTUDIOBONES],
	int sequence,
	float cycle,
	const float poseParameter[],
	int boneMask,
	float flWeight = 1.0f
	);

void CalcBoneAdj_v44(
	const studiohdr_v44_t *pStudioHdr,
	Vector pos[],
	Quaternion q[],
	const float controllers[],
	int boneMask
	);

void CalcAutoplaySequences_v44(
	const studiohdr_v44_t *pStudioHdr,
	CIKContext_v44 *pIKContext,
	Vector pos[],
	Quaternion q[],
	const float poseParameters[],
	int boneMask,
	float time
	);

bool CalcProceduralBone_v44(
	const studiohdr_v44_t *pStudioHdr,
	int iBone,
	matrix3x4_t *bonetoworld
	);

void Studio_BuildMatrices_v44(
	const studiohdr_v44_t *pStudioHdr,
	const QAngle& angles,
	const Vector& origin,
	const Vector pos[],
	const Quaternion q[],
	int iBone,
	matrix3x4_t bonetoworld[MAXSTUDIOBONES],
	int boneMask
	);

// World-space blending functions (2007 engine)
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
	);

void ScaleBones_v44(
	const studiohdr_v44_t *pStudioHdr,
	Quaternion q[MAXSTUDIOBONES],
	Vector pos[MAXSTUDIOBONES],
	int sequence,
	float s,
	int boneMask
	);

#endif // BONE_SETUP_V44_H
