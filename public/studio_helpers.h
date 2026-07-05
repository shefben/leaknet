//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Version-aware helper functions for accessing studiohdr_t data
//          These inline helpers work with v37 models only.
//          v44+ models are handled by the isolated v44+ system.
//
//=============================================================================

#ifndef STUDIO_HELPERS_H
#define STUDIO_HELPERS_H

#include "studio.h"

inline bool StudioHdr_IsV44Plus(const studiohdr_t* pStudioHdr)
{
	return pStudioHdr && pStudioHdr->version >= STUDIO_VERSION_44;
}

inline const studiohdr_v44_t* StudioHdr_AsV44(const studiohdr_t* pStudioHdr)
{
	return StudioHdr_IsV44Plus(pStudioHdr) ? (const studiohdr_v44_t*)pStudioHdr : NULL;
}

inline studiohdr_v44_t* StudioHdr_AsV44(studiohdr_t* pStudioHdr)
{
	return StudioHdr_IsV44Plus(pStudioHdr) ? (studiohdr_v44_t*)pStudioHdr : NULL;
}

//-----------------------------------------------------------------------------
// Studiohdr_t helpers
//-----------------------------------------------------------------------------

inline int StudioHdr_GetNumBodyparts(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numbodyparts : pStudioHdr->numbodyparts;
}

inline mstudiobodyparts_t* StudioHdr_GetBodypart(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudiobodyparts_t*)pHdr44->pBodypart(i) : pStudioHdr->pBodypart(i);
}

inline int StudioBodypart_GetNumModels(const studiohdr_t* pStudioHdr, const mstudiobodyparts_t* pBodypart)
{
	if (!pStudioHdr || !pBodypart)
		return 0;
	if (StudioHdr_IsV44Plus(pStudioHdr))
		return ((const mstudiobodyparts_v44_t*)pBodypart)->nummodels;
	return pBodypart->nummodels;
}

inline mstudiomodel_t* StudioBodypart_GetModel(const studiohdr_t* pStudioHdr, mstudiobodyparts_t* pBodypart, int i)
{
	if (!pStudioHdr || !pBodypart)
		return NULL;
	if (StudioHdr_IsV44Plus(pStudioHdr))
		return (mstudiomodel_t*)((mstudiobodyparts_v44_t*)pBodypart)->pModel(i);
	return pBodypart->pModel(i);
}

inline int StudioHdr_GetNumTextures(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numtextures : pStudioHdr->numtextures;
}

inline mstudiotexture_t* StudioHdr_GetTexture(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	if (pHdr44)
		return (mstudiotexture_t*)pHdr44->pTexture(i);
	if (pStudioHdr->version <= STUDIO_VERSION_37)
		return (mstudiotexture_t*)pStudioHdr->pTexture_V37(i);
	return pStudioHdr->pTexture(i);
}

inline int StudioHdr_GetNumCdTextures(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numcdtextures : pStudioHdr->numcdtextures;
}

inline char* StudioHdr_GetCdTexture(const studiohdr_t* pStudioHdr, int i)
{
	static char s_emptyCdTexture[] = "";
	if (!pStudioHdr)
		return s_emptyCdTexture;

	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	const int numCdTextures = pHdr44 ? pHdr44->numcdtextures : pStudioHdr->numcdtextures;
	const int cdTextureIndex = pHdr44 ? pHdr44->cdtextureindex : pStudioHdr->cdtextureindex;
	const int length = pHdr44 ? pHdr44->length : pStudioHdr->length;

	if (i < 0 || i >= numCdTextures || cdTextureIndex <= 0 || length <= 0)
		return s_emptyCdTexture;

	if (cdTextureIndex > length - (int)sizeof(int))
		return s_emptyCdTexture;

	const int maxTableIndex = (length - cdTextureIndex) / (int)sizeof(int) - 1;
	if (i > maxTableIndex)
		return s_emptyCdTexture;

	const int offset = *(int*)(((byte*)pStudioHdr) + cdTextureIndex + i * sizeof(int));
	if (offset <= 0 || offset >= length)
		return s_emptyCdTexture;

	return (char*)pStudioHdr + offset;
}

inline int StudioHdr_GetNumPoseParameters(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numlocalposeparameters : pStudioHdr->numlocalposeparameters();
}

inline int StudioHdr_GetNumLocalAnims(const studiohdr_t* pStudioHdr)
{
	return pStudioHdr ? pStudioHdr->GetNumLocalAnim() : 0;
}

inline int StudioHdr_GetNumLocalSeq(const studiohdr_t* pStudioHdr)
{
	return pStudioHdr ? pStudioHdr->GetNumLocalSeq() : 0;
}

inline int StudioHdr_GetNumAnimBlocks(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numanimblocks : pStudioHdr->numanimblocks;
}

inline mstudioanimblock_t* StudioHdr_GetAnimBlock(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudioanimblock_t*)pHdr44->pAnimBlock(i) : pStudioHdr->pAnimBlock(i);
}

inline mstudioattachment_t* StudioHdr_GetAttachment(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudioattachment_t*)pHdr44->pLocalAttachment(i) : pStudioHdr->pAttachment(i);
}

//-----------------------------------------------------------------------------
// Sequence descriptor helpers
//-----------------------------------------------------------------------------

inline const mstudioseqdesc_v48_t* StudioSeqdesc_AsV48(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pSeqdesc)
{
	return StudioHdr_IsV44Plus(pStudioHdr) ? (const mstudioseqdesc_v48_t*)pSeqdesc : NULL;
}

inline mstudioseqdesc_v48_t* StudioSeqdesc_AsV48(studiohdr_t* pStudioHdr, mstudioseqdesc_t* pSeqdesc)
{
	return StudioHdr_IsV44Plus(pStudioHdr) ? (mstudioseqdesc_v48_t*)pSeqdesc : NULL;
}

inline Vector StudioSeqdesc_GetBBMin(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return Vector(0, 0, 0);
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->bbmin : (pseq ? pseq->bbmin : Vector(0, 0, 0));
}

inline Vector StudioSeqdesc_GetBBMax(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return Vector(0, 0, 0);
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->bbmax : (pseq ? pseq->bbmax : Vector(0, 0, 0));
}

inline int StudioSeqdesc_GetActivity(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return 0;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->activity : (pseq ? pseq->activity : 0);
}

inline void StudioSeqdesc_SetActivity(studiohdr_t* pStudioHdr, int seq, int activity)
{
	if (!pStudioHdr)
		return;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	if (pseq48)
		pseq48->activity = activity;
	else if (pseq)
		pseq->activity = activity;
}

inline int StudioSeqdesc_GetActweight(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return 0;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->actweight : (pseq ? pseq->actweight : 0);
}

inline const char* StudioSeqdesc_GetActivityName(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return "";
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->pszActivityName() : (pseq ? pseq->pszActivityName() : "");
}

inline const char* StudioSeqdesc_GetLabel(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return "";
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->pszLabel() : (pseq ? pseq->pszLabel() : "");
}

inline int StudioSeqdesc_GetFlags(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return 0;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->flags : (pseq ? pseq->flags : 0);
}

inline int StudioSeqdesc_GetFlagsFromPtr(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pseqdesc)
{
	if (!pStudioHdr || !pseqdesc)
		return 0;
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseqdesc);
	return pseq48 ? pseq48->flags : pseqdesc->flags;
}

inline int StudioSeqdesc_GetNumEvents(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return 0;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->numevents : (pseq ? pseq->numevents : 0);
}

inline mstudioevent_t* StudioSeqdesc_GetEvent(const studiohdr_t* pStudioHdr, int seq, int eventIndex)
{
	if (!pStudioHdr)
		return NULL;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	if (!pseq)
		return NULL;
	if (StudioHdr_IsV44Plus(pStudioHdr))
		return (mstudioevent_t*)((const mstudioseqdesc_v44_t*)pseq)->pEvent(eventIndex);
	return pseq->pEvent(eventIndex);
}

inline int StudioSeqdesc_GetEntryNode(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return 0;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->localentrynode : (pseq ? pseq->entrynode : 0);
}

inline int StudioSeqdesc_GetExitNode(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return 0;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->localexitnode : (pseq ? pseq->exitnode : 0);
}

inline int StudioSeqdesc_GetNodeFlags(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return 0;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->nodeflags : (pseq ? pseq->nodeflags : 0);
}

inline float StudioSeqdesc_GetFadeOutTime(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return 0.2f;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->fadeouttime : (pseq ? pseq->fadeouttime : 0.2f);
}

inline float StudioSeqdesc_GetExitPhase(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return 0.0f;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->exitphase : (pseq ? pseq->exitphase : 0.0f);
}

inline int StudioSeqdesc_GetParamIndexFromPtr(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pseqdesc, int iLocalPose)
{
	if (!pStudioHdr || !pseqdesc || iLocalPose < 0 || iLocalPose >= 2)
		return -1;
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseqdesc);
	return pseq48 ? pseq48->paramindex[iLocalPose] : pseqdesc->paramindex[iLocalPose];
}

inline int StudioSeqdesc_GetGroupSizeFromPtr(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pseqdesc, int iLocalPose)
{
	if (!pStudioHdr || !pseqdesc || iLocalPose < 0 || iLocalPose >= 2)
		return 0;
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseqdesc);
	return pseq48 ? pseq48->groupsize[iLocalPose] : pseqdesc->groupsize[iLocalPose];
}

inline float StudioSeqdesc_GetParamStartFromPtr(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pseqdesc, int iLocalPose)
{
	if (!pStudioHdr || !pseqdesc || iLocalPose < 0 || iLocalPose >= 2)
		return 0.0f;
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseqdesc);
	return pseq48 ? pseq48->paramstart[iLocalPose] : pseqdesc->paramstart[iLocalPose];
}

inline float StudioSeqdesc_GetParamEndFromPtr(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pseqdesc, int iLocalPose)
{
	if (!pStudioHdr || !pseqdesc || iLocalPose < 0 || iLocalPose >= 2)
		return 0.0f;
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseqdesc);
	return pseq48 ? pseq48->paramend[iLocalPose] : pseqdesc->paramend[iLocalPose];
}

inline int StudioSeqdesc_GetPoseKeyIndexFromPtr(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pseqdesc)
{
	if (!pStudioHdr || !pseqdesc)
		return 0;
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseqdesc);
	return pseq48 ? pseq48->posekeyindex : pseqdesc->posekeyindex;
}

inline float StudioSeqdesc_GetPoseKeyFromPtr(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pseqdesc, int iParam, int iAnim)
{
	if (!pStudioHdr || !pseqdesc)
		return 0.0f;
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseqdesc);
	return pseq48 ? pseq48->poseKey(iParam, iAnim) : pseqdesc->poseKey(iParam, iAnim);
}

inline const char* StudioSeqdesc_GetKeyValueText(const studiohdr_t* pStudioHdr, int seq)
{
	if (!pStudioHdr)
		return NULL;
	mstudioseqdesc_t* pseq = pStudioHdr->pSeqdesc(seq);
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseq);
	return pseq48 ? pseq48->KeyValueText() : (pseq ? pseq->KeyValueText() : NULL);
}

// Helper that takes a pointer to the sequence descriptor directly (for optimized inner loops)
inline float StudioSeqdesc_GetWeightFromPtr(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pseqdesc, int boneIndex)
{
	if (!pStudioHdr || !pseqdesc)
		return 0.0f;
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseqdesc);
	if (pseq48)
		return pseq48->weightlistindex ? pseq48->weight(boneIndex) : 0.0f;
	return pseqdesc->weight(boneIndex);
}

inline int StudioSeqdesc_GetNumIKLocksFromPtr(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pseqdesc)
{
	if (!pStudioHdr || !pseqdesc)
		return 0;
	const mstudioseqdesc_v48_t* pseq48 = StudioSeqdesc_AsV48(pStudioHdr, pseqdesc);
	return pseq48 ? pseq48->numiklocks : pseqdesc->numiklocks;
}

inline mstudioiklock_t* StudioSeqdesc_GetIKLockFromPtr(const studiohdr_t* pStudioHdr, mstudioseqdesc_t* pseqdesc, int i)
{
	if (!pStudioHdr || !pseqdesc)
		return NULL;
	if (StudioHdr_IsV44Plus(pStudioHdr))
		return (mstudioiklock_t*)((mstudioseqdesc_v44_t*)pseqdesc)->pIKLock(i);
	return pseqdesc->pIKLock(i);
}

//-----------------------------------------------------------------------------
// Bone helpers
//-----------------------------------------------------------------------------

inline int StudioSeqdesc_GetNumAutolayersFromPtr(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pseqdesc)
{
	if (!pStudioHdr || !pseqdesc)
		return 0;
	if (StudioHdr_IsV44Plus(pStudioHdr))
		return ((const mstudioseqdesc_v44_t*)pseqdesc)->numautolayers;
	return pseqdesc->numautolayers;
}

inline mstudioautolayer_t* StudioSeqdesc_GetAutolayerFromPtr(const studiohdr_t* pStudioHdr, mstudioseqdesc_t* pseqdesc, int i)
{
	if (!pStudioHdr || !pseqdesc)
		return NULL;
	if (StudioHdr_IsV44Plus(pStudioHdr))
		return (mstudioautolayer_t*)((mstudioseqdesc_v44_t*)pseqdesc)->pAutolayer(i);
	return pseqdesc->pAutolayer(i);
}

inline int StudioAutolayer_GetSequence(const studiohdr_t* pStudioHdr, const mstudioautolayer_t* pLayer)
{
	if (!pLayer)
		return 0;
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudioautolayer_v44_t*)pLayer)->iSequence : pLayer->iSequence;
}

inline int StudioAutolayer_GetFlags(const studiohdr_t* pStudioHdr, const mstudioautolayer_t* pLayer)
{
	if (!pLayer)
		return 0;
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudioautolayer_v44_t*)pLayer)->flags : pLayer->flags;
}

inline float StudioAutolayer_GetStart(const studiohdr_t* pStudioHdr, const mstudioautolayer_t* pLayer)
{
	if (!pLayer)
		return 0.0f;
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudioautolayer_v44_t*)pLayer)->start : pLayer->start;
}

inline float StudioAutolayer_GetPeak(const studiohdr_t* pStudioHdr, const mstudioautolayer_t* pLayer)
{
	if (!pLayer)
		return 0.0f;
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudioautolayer_v44_t*)pLayer)->peak : pLayer->peak;
}

inline float StudioAutolayer_GetTail(const studiohdr_t* pStudioHdr, const mstudioautolayer_t* pLayer)
{
	if (!pLayer)
		return 0.0f;
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudioautolayer_v44_t*)pLayer)->tail : pLayer->tail;
}

inline float StudioAutolayer_GetEnd(const studiohdr_t* pStudioHdr, const mstudioautolayer_t* pLayer)
{
	if (!pLayer)
		return 0.0f;
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudioautolayer_v44_t*)pLayer)->end : pLayer->end;
}

inline int StudioSeqdesc_GetNumIKRulesFromPtr(const studiohdr_t* pStudioHdr, const mstudioseqdesc_t* pseqdesc)
{
	if (!pStudioHdr || !pseqdesc)
		return 0;
	if (StudioHdr_IsV44Plus(pStudioHdr))
		return ((const mstudioseqdesc_v44_t*)pseqdesc)->numikrules;
	return pseqdesc->numikrules;
}

inline int StudioMovement_GetEndFrame(const studiohdr_t* pStudioHdr, const mstudiomovement_t* pMove)
{
	if (!pMove)
		return 0;
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudiomovement_v44_t*)pMove)->endframe : pMove->endframe;
}

inline float StudioMovement_GetV0(const studiohdr_t* pStudioHdr, const mstudiomovement_t* pMove)
{
	if (!pMove)
		return 0.0f;
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudiomovement_v44_t*)pMove)->v0 : pMove->v0;
}

inline float StudioMovement_GetV1(const studiohdr_t* pStudioHdr, const mstudiomovement_t* pMove)
{
	if (!pMove)
		return 0.0f;
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudiomovement_v44_t*)pMove)->v1 : pMove->v1;
}

inline Vector StudioMovement_GetVector(const studiohdr_t* pStudioHdr, const mstudiomovement_t* pMove)
{
	if (!pMove)
		return Vector(0, 0, 0);
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudiomovement_v44_t*)pMove)->vector : pMove->vector;
}

inline float StudioMovement_GetAngle(const studiohdr_t* pStudioHdr, const mstudiomovement_t* pMove)
{
	if (!pMove)
		return 0.0f;
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudiomovement_v44_t*)pMove)->angle : pMove->angle;
}

inline Vector StudioMovement_GetPosition(const studiohdr_t* pStudioHdr, const mstudiomovement_t* pMove)
{
	if (!pMove)
		return Vector(0, 0, 0);
	return StudioHdr_IsV44Plus(pStudioHdr) ? ((const mstudiomovement_v44_t*)pMove)->position : pMove->position;
}

inline int StudioHdr_GetNumBones(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numbones : pStudioHdr->numbones;
}

inline mstudiobone_t* StudioHdr_GetBone(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudiobone_t*)pHdr44->pBone(i) : pStudioHdr->pBone(i);
}

inline int StudioHdr_GetNumBoneControllers(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numbonecontrollers : pStudioHdr->numbonecontrollers;
}

inline mstudiobonecontroller_t* StudioHdr_GetBoneController(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudiobonecontroller_t*)pHdr44->pBonecontroller(i) : pStudioHdr->pBonecontroller(i);
}

inline const char* StudioBone_GetName(const studiohdr_t* pStudioHdr, int boneIndex)
{
	if (!pStudioHdr)
		return "";
	return pStudioHdr->GetBoneName(boneIndex);
}

inline int StudioBone_GetParent(const studiohdr_t* pStudioHdr, int boneIndex)
{
	if (!pStudioHdr)
		return -1;
	return pStudioHdr->GetBoneParent(boneIndex);
}

inline int StudioBone_GetFlags(const studiohdr_t* pStudioHdr, int boneIndex)
{
	if (!pStudioHdr)
		return 0;
	return pStudioHdr->GetBoneFlags(boneIndex);
}

inline int StudioBone_GetContents(const studiohdr_t* pStudioHdr, int boneIndex)
{
	if (!pStudioHdr)
		return 0;
	return pStudioHdr->GetBoneContents(boneIndex);
}

inline int StudioBone_GetPhysicsBone(const studiohdr_t* pStudioHdr, int boneIndex)
{
	if (!pStudioHdr)
		return -1;
	return pStudioHdr->GetBonePhysicsbone(boneIndex);
}

inline int StudioBone_GetProcType(const studiohdr_t* pStudioHdr, int boneIndex)
{
	if (!pStudioHdr)
		return 0;
	return pStudioHdr->GetBoneProctype(boneIndex);
}

inline void* StudioBone_GetProcedure(const studiohdr_t* pStudioHdr, int boneIndex)
{
	if (!pStudioHdr)
		return NULL;
	return pStudioHdr->GetBoneProcedure(boneIndex);
}

inline const matrix3x4_t& StudioBone_GetPoseToBone(const studiohdr_t* pStudioHdr, int boneIndex)
{
	static matrix3x4_t identity;
	SetIdentityMatrix(identity);
	if (!pStudioHdr)
		return identity;
	return pStudioHdr->GetBonePoseToBone(boneIndex);
}

inline const matrix3x4_t* StudioBone_GetPoseToBonePtr(const studiohdr_t* pStudioHdr, int boneIndex)
{
	if (!pStudioHdr)
		return NULL;
	return &pStudioHdr->GetBonePoseToBone(boneIndex);
}

//-----------------------------------------------------------------------------
// Animation descriptor helpers
//-----------------------------------------------------------------------------

inline int StudioAnimdesc_GetNumFrames(const studiohdr_t* pStudioHdr, int animIndex)
{
	if (!pStudioHdr)
		return 0;
	mstudioanimdesc_t* pAnimDesc = pStudioHdr->pAnimdesc(animIndex);
	return pAnimDesc ? pAnimDesc->numframes : 0;
}

inline const char* StudioAnimdesc_GetName(const studiohdr_t* pStudioHdr, int animIndex)
{
	if (!pStudioHdr)
		return "";
	mstudioanimdesc_t* pAnimDesc = pStudioHdr->pAnimdesc(animIndex);
	if (!pAnimDesc)
		return "";
	return StudioHdr_IsV44Plus(pStudioHdr) ?
		((const mstudioanimdesc_v48_t*)pAnimDesc)->pszName() : pAnimDesc->pszName();
}

// Version-aware animation data accessor for v44+ models
// Returns mstudioanim_v48_t* for v44+ animation descriptor
// Note: For v37 models, use panimdesc->pAnim(bone) directly
inline mstudioanim_v48_t* StudioAnimDesc_GetAnimBlock_v44(const studiohdr_t* pStudioHdr,
	const mstudioanimdesc_v48_t* panimdesc, int animblock, int animindex)
{
	if (!pStudioHdr || !panimdesc)
		return NULL;

	if (animindex == 0 || animblock == -1)
		return NULL;

	if (animblock == 0)
		return (mstudioanim_v48_t*)((byte*)panimdesc + animindex);

	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	if (pHdr44 && pHdr44->animblockModel && animblock < pHdr44->numanimblocks)
	{
		mstudioanimblock_v44_t* pBlock = pHdr44->pAnimBlock(animblock);
		if (pBlock)
			return (mstudioanim_v48_t*)((byte*)pHdr44->animblockModel + pBlock->datastart + animindex);
	}

	return NULL;
}

inline mstudioanim_v48_t* StudioAnimDesc_GetAnim_v44_Frame(const studiohdr_t* pStudioHdr,
	const mstudioanimdesc_v48_t* panimdesc, int* pFrame)
{
	if (!pStudioHdr || !panimdesc || pStudioHdr->version < STUDIO_VERSION_44)
		return NULL;

	int animblock = panimdesc->animblock;
	int animindex = panimdesc->animindex;
	int section = 0;

	if (pFrame && panimdesc->sectionframes != 0)
	{
		if (panimdesc->numframes > panimdesc->sectionframes && *pFrame == panimdesc->numframes - 1)
		{
			*pFrame = 0;
			section = (panimdesc->numframes / panimdesc->sectionframes) + 1;
		}
		else
		{
			section = *pFrame / panimdesc->sectionframes;
			*pFrame -= section * panimdesc->sectionframes;
		}

		mstudioanimsections_t* pSection = panimdesc->pSection(section);
		if (!pSection)
			return NULL;
		animblock = pSection->animblock;
		animindex = pSection->animindex;
	}

	mstudioanim_v48_t* panim = StudioAnimDesc_GetAnimBlock_v44(pStudioHdr, panimdesc, animblock, animindex);
	if (panim || !pFrame || panimdesc->sectionframes == 0)
		return panim;

	while (--section >= 0)
	{
		mstudioanimsections_t* pSection = panimdesc->pSection(section);
		if (!pSection)
			continue;

		panim = StudioAnimDesc_GetAnimBlock_v44(pStudioHdr, panimdesc, pSection->animblock, pSection->animindex);
		if (panim)
		{
			*pFrame = panimdesc->sectionframes - 1;
			break;
		}
	}

	return panim;
}

inline mstudioanim_v48_t* StudioAnimDesc_GetAnim_v44(const studiohdr_t* pStudioHdr, const mstudioanimdesc_v48_t* panimdesc)
{
	return StudioAnimDesc_GetAnim_v44_Frame(pStudioHdr, panimdesc, NULL);
}

//-----------------------------------------------------------------------------
// Attachment helpers
//-----------------------------------------------------------------------------

inline int StudioHdr_GetNumAttachments(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numlocalattachments : pStudioHdr->numattachments;
}

inline const char* StudioAttachment_GetName(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return "";
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	if (pHdr44)
	{
		const mstudioattachment_v44_t* pAttachment = pHdr44->pLocalAttachment(i);
		return pAttachment ? pAttachment->pszName() : "";
	}

	mstudioattachment_t* pAttachment = pStudioHdr->pAttachment(i);
	return pAttachment ? pAttachment->pszName() : "";
}

inline int StudioAttachment_GetBone(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return -1;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	if (pHdr44)
	{
		const mstudioattachment_v44_t* pAttachment = pHdr44->pLocalAttachment(i);
		return pAttachment ? pAttachment->localbone : -1;
	}

	mstudioattachment_t* pAttachment = pStudioHdr->pAttachment(i);
	return pAttachment ? pAttachment->bone : -1;
}

inline const matrix3x4_t* StudioAttachment_GetLocal(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	if (pHdr44)
	{
		const mstudioattachment_v44_t* pAttachment = pHdr44->pLocalAttachment(i);
		return pAttachment ? &pAttachment->local : NULL;
	}

	mstudioattachment_t* pAttachment = pStudioHdr->pAttachment(i);
	return pAttachment ? &pAttachment->local : NULL;
}

//-----------------------------------------------------------------------------
// Hitbox helpers
//-----------------------------------------------------------------------------

inline int StudioHdr_GetNumHitboxSets(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numhitboxsets : pStudioHdr->numhitboxsets;
}

inline mstudiohitboxset_t* StudioHdr_GetHitboxSet(const studiohdr_t* pStudioHdr, int set)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudiohitboxset_t*)pHdr44->pHitboxSet(set) : pStudioHdr->pHitboxSet(set);
}

inline const mstudiohitboxset_v44_t* StudioHitboxSet_AsV44(const studiohdr_t* pStudioHdr, const mstudiohitboxset_t* pSet)
{
	return (StudioHdr_IsV44Plus(pStudioHdr) && pSet) ? (const mstudiohitboxset_v44_t*)pSet : NULL;
}

inline mstudiohitboxset_v44_t* StudioHitboxSet_AsV44(studiohdr_t* pStudioHdr, mstudiohitboxset_t* pSet)
{
	return (StudioHdr_IsV44Plus(pStudioHdr) && pSet) ? (mstudiohitboxset_v44_t*)pSet : NULL;
}

inline const char* StudioHitboxSet_GetName(const studiohdr_t* pStudioHdr, int set)
{
	mstudiohitboxset_t* pSet = StudioHdr_GetHitboxSet(pStudioHdr, set);
	const mstudiohitboxset_v44_t* pSet44 = StudioHitboxSet_AsV44(pStudioHdr, pSet);
	return pSet44 ? pSet44->pszName() : (pSet ? pSet->pszName() : "");
}

inline int StudioHitboxSet_GetNumHitboxes(const studiohdr_t* pStudioHdr, int set)
{
	mstudiohitboxset_t* pSet = StudioHdr_GetHitboxSet(pStudioHdr, set);
	const mstudiohitboxset_v44_t* pSet44 = StudioHitboxSet_AsV44(pStudioHdr, pSet);
	return pSet44 ? pSet44->numhitboxes : (pSet ? pSet->numhitboxes : 0);
}

inline int StudioHitboxSet_GetNumHitboxesFromPtr(const studiohdr_t* pStudioHdr, const mstudiohitboxset_t* pSet)
{
	const mstudiohitboxset_v44_t* pSet44 = StudioHitboxSet_AsV44(pStudioHdr, pSet);
	return pSet44 ? pSet44->numhitboxes : (pSet ? pSet->numhitboxes : 0);
}

inline mstudiobbox_t* StudioHitboxSet_GetHitbox(const studiohdr_t* pStudioHdr, int set, int hitbox)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudiobbox_t*)pHdr44->pHitbox(hitbox, set) : pStudioHdr->pHitbox(hitbox, set);
}

inline mstudiobbox_t* StudioHitboxSet_GetHitboxFromPtr(const studiohdr_t* pStudioHdr, const mstudiohitboxset_t* pSet, int hitbox)
{
	if (!pSet)
		return NULL;
	const mstudiohitboxset_v44_t* pSet44 = StudioHitboxSet_AsV44(pStudioHdr, pSet);
	return pSet44 ? (mstudiobbox_t*)pSet44->pHitbox(hitbox) : ((mstudiohitboxset_t*)pSet)->pHitbox(hitbox);
}

inline int StudioHitbox_GetBone(const studiohdr_t* pStudioHdr, const mstudiohitboxset_t* pSet, int hitbox)
{
	mstudiobbox_t* pBox = StudioHitboxSet_GetHitboxFromPtr(pStudioHdr, pSet, hitbox);
	return pBox ? pBox->bone : -1;
}

inline int StudioHitbox_GetGroup(const studiohdr_t* pStudioHdr, const mstudiohitboxset_t* pSet, int hitbox)
{
	mstudiobbox_t* pBox = StudioHitboxSet_GetHitboxFromPtr(pStudioHdr, pSet, hitbox);
	return pBox ? pBox->group : 0;
}

inline Vector StudioHitbox_GetBBMin(const studiohdr_t* pStudioHdr, const mstudiohitboxset_t* pSet, int hitbox)
{
	mstudiobbox_t* pBox = StudioHitboxSet_GetHitboxFromPtr(pStudioHdr, pSet, hitbox);
	return pBox ? pBox->bbmin : Vector(0, 0, 0);
}

inline Vector StudioHitbox_GetBBMax(const studiohdr_t* pStudioHdr, const mstudiohitboxset_t* pSet, int hitbox)
{
	mstudiobbox_t* pBox = StudioHitboxSet_GetHitboxFromPtr(pStudioHdr, pSet, hitbox);
	return pBox ? pBox->bbmax : Vector(0, 0, 0);
}

inline const char* StudioHitbox_GetName(const studiohdr_t* pStudioHdr, int set, int hitbox)
{
	if (!pStudioHdr)
		return "";
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	if (pHdr44)
	{
		mstudiohitboxset_v44_t* pSet = pHdr44->pHitboxSet(set);
		if (!pSet)
			return "";
		mstudiobbox_v44_t* pBox = pSet->pHitbox(hitbox);
		return pBox ? pBox->pszHitboxName() : "";
	}

	mstudiobbox_t* pBox = pStudioHdr->pHitbox(hitbox, set);
	return pBox ? pBox->pszHitboxName((void*)pStudioHdr) : "";
}

//-----------------------------------------------------------------------------
// Include model helpers
//-----------------------------------------------------------------------------

inline int StudioHdr_GetNumIncludeModels(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numincludemodels : pStudioHdr->numincludemodels;
}

inline mstudiomodelgroup_t* StudioHdr_GetModelGroup(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	if (pHdr44)
	{
		if (pHdr44->includemodelindex == 0)
			return NULL;
		return (mstudiomodelgroup_t*)(((byte*)pHdr44) + pHdr44->includemodelindex) + i;
	}
	return pStudioHdr->pModelGroup(i);
}

//-----------------------------------------------------------------------------
// Skin helpers
//-----------------------------------------------------------------------------

inline int StudioHdr_GetNumSkinFamilies(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numskinfamilies : pStudioHdr->numskinfamilies;
}

inline int StudioHdr_GetNumSkinRef(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numskinref : pStudioHdr->numskinref;
}

inline short* StudioHdr_GetSkinRef(const studiohdr_t* pStudioHdr, int offset)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	short* pSkinRef = pHdr44 ? pHdr44->pSkinref(0) : pStudioHdr->pSkinref(0);
	if (!pSkinRef)
		return NULL;
	return pSkinRef + offset;
}

//-----------------------------------------------------------------------------
// Pose parameter helpers
//-----------------------------------------------------------------------------

inline mstudioposeparamdesc_t* StudioHdr_GetPoseParameter(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudioposeparamdesc_t*)pHdr44->pLocalPoseParameter(i) : pStudioHdr->pPoseParameter(i);
}

inline const char* StudioPoseParam_GetName(const studiohdr_t* pStudioHdr, int i)
{
	mstudioposeparamdesc_t* pPose = StudioHdr_GetPoseParameter(pStudioHdr, i);
	return pPose ? pPose->pszName() : "";
}

inline int StudioHdr_GetNumFlexDescs(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numflexdesc : pStudioHdr->numflexdesc;
}

inline mstudioflexdesc_t* StudioHdr_GetFlexDesc(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudioflexdesc_t*)pHdr44->pFlexdesc(i) : pStudioHdr->pFlexdesc(i);
}

inline int StudioHdr_GetNumFlexControllers(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numflexcontrollers : pStudioHdr->numflexcontrollers;
}

inline mstudioflexcontroller_t* StudioHdr_GetFlexController(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudioflexcontroller_t*)pHdr44->pFlexcontroller(i) : pStudioHdr->pFlexcontroller(i);
}

inline int StudioHdr_GetNumFlexRules(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numflexrules : pStudioHdr->numflexrules;
}

inline mstudioflexrule_t* StudioHdr_GetFlexRule(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudioflexrule_t*)(((byte*)pHdr44) + pHdr44->flexruleindex) + i : pStudioHdr->pFlexRule(i);
}

inline int StudioHdr_GetNumFlexControllerUI(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr || !StudioHdr_IsV44Plus(pStudioHdr))
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numflexcontrollerui : 0;
}

inline mstudioflexcontrollerui_t* StudioHdr_GetFlexControllerUI(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr || !StudioHdr_IsV44Plus(pStudioHdr))
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return (pHdr44 && pHdr44->flexcontrolleruiindex) ?
		(mstudioflexcontrollerui_t*)(((byte*)pHdr44) + pHdr44->flexcontrolleruiindex) + i : NULL;
}

inline int StudioHdr_GetNumMouths(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->nummouths : pStudioHdr->nummouths;
}

inline mstudiomouth_t* StudioHdr_GetMouth(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudiomouth_t*)pHdr44->pMouth(i) : pStudioHdr->pMouth(i);
}

//-----------------------------------------------------------------------------
// IK chain helpers
//-----------------------------------------------------------------------------

inline int StudioHdr_GetNumIKChains(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numikchains : pStudioHdr->numikchains;
}

inline mstudioikchain_t* StudioHdr_GetIKChain(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudioikchain_t*)pHdr44->pIKChain(i) : pStudioHdr->pIKChain(i);
}

inline const mstudioikchain_v44_t* StudioIKChain_AsV44(const studiohdr_t* pStudioHdr, const mstudioikchain_t* pChain)
{
	return (StudioHdr_IsV44Plus(pStudioHdr) && pChain) ? (const mstudioikchain_v44_t*)pChain : NULL;
}

inline mstudioikchain_v44_t* StudioIKChain_AsV44(studiohdr_t* pStudioHdr, mstudioikchain_t* pChain)
{
	return (StudioHdr_IsV44Plus(pStudioHdr) && pChain) ? (mstudioikchain_v44_t*)pChain : NULL;
}

inline int StudioIKChain_GetNumLinks(const studiohdr_t* pStudioHdr, const mstudioikchain_t* pChain)
{
	const mstudioikchain_v44_t* pChain44 = StudioIKChain_AsV44(pStudioHdr, pChain);
	return pChain44 ? pChain44->numlinks : (pChain ? pChain->numlinks : 0);
}

inline int StudioIKChain_GetLinkBone(const studiohdr_t* pStudioHdr, const mstudioikchain_t* pChain, int i)
{
	if (!pChain || i < 0)
		return -1;
	const mstudioikchain_v44_t* pChain44 = StudioIKChain_AsV44(pStudioHdr, pChain);
	if (pChain44)
	{
		const mstudioiklink_v44_t* pLink = pChain44->pLink(i);
		return pLink ? pLink->bone : -1;
	}

	mstudioiklink_t* pLink = pChain->pLink(i);
	return pLink ? pLink->bone : -1;
}

inline int StudioHdr_GetNumIKAutoplayLocks(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numlocalikautoplaylocks : pStudioHdr->numikautoplaylocks;
}

inline mstudioiklock_t* StudioHdr_GetIKAutoplayLock(const studiohdr_t* pStudioHdr, int i)
{
	if (!pStudioHdr)
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (mstudioiklock_t*)pHdr44->pLocalIKAutoplayLock(i) : pStudioHdr->pIKAutoplayLock(i);
}

inline float StudioHdr_GetMass(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0.0f;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->mass : pStudioHdr->mass;
}

inline int StudioHdr_GetContents(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->contents : pStudioHdr->contents;
}

inline int StudioHdr_GetRootLOD(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr)
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->rootLOD : pStudioHdr->rootLOD;
}

inline int StudioHdr_GetNumAllowedRootLODs(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr || !StudioHdr_IsV44Plus(pStudioHdr))
		return 0;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->numAllowedRootLODs : 0;
}

inline const byte* StudioHdr_GetBoneTableSortedByName(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr || !StudioHdr_IsV44Plus(pStudioHdr))
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? pHdr44->GetBoneTableSortedByName() : NULL;
}

inline const studiohdr2_t* StudioHdr_GetStudioHdr2(const studiohdr_t* pStudioHdr)
{
	if (!pStudioHdr || !StudioHdr_IsV44Plus(pStudioHdr))
		return NULL;
	const studiohdr_v44_t* pHdr44 = StudioHdr_AsV44(pStudioHdr);
	return pHdr44 ? (const studiohdr2_t*)pHdr44->pStudioHdr2() : NULL;
}

#endif // STUDIO_HELPERS_H
