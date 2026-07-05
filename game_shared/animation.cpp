/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "cbase.h"
#include "studio.h"
#include "studiohdr_v44.h"
#include "studio_helpers.h"
#include "activitylist.h"
#include "engine/IEngineSound.h"
#include "ai_activity.h"
#include "animation.h"
#include "bone_setup.h"
#include "scriptevent.h"
#include "npcevent.h"

#if !defined( CLIENT_DLL )
#include "util.h"
#include "enginecallback.h"
#endif

#pragma warning( disable : 4244 )
#define iabs(i) (( (i) >= 0 ) ? (i) : -(i) )

int ExtractBbox( studiohdr_t *pstudiohdr, int sequence, Vector& mins, Vector& maxs )
{
	if (! pstudiohdr)
		return 0;

	mins = StudioSeqdesc_GetBBMin(pstudiohdr, sequence);
	maxs = StudioSeqdesc_GetBBMax(pstudiohdr, sequence);

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : *pstudiohdr - 
//			iSequence - 
//
// Output : mstudioseqdesc_t
//-----------------------------------------------------------------------------
mstudioseqdesc_t *GetSequenceDesc( studiohdr_t *pstudiohdr )
{
	mstudioseqdesc_t	*pseqdesc;

	if( !pstudiohdr )
	{
		return NULL;
	}

	pseqdesc = pstudiohdr->pSeqdesc( 0 );

	return pseqdesc;
}

//=========================================================
// IndexModelSequences - set activity indexes for all model
// sequences that have activities.
//=========================================================
void IndexModelSequences( studiohdr_t *pstudiohdr )
{
	int i;
	int iActivityIndex;
	const char *pszActivityName;

	if (! pstudiohdr)
		return;

	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	for ( i = 0 ; i < pstudiohdr->GetNumLocalSeq() ; i++ )
	{
		// look up the activity number, but only for sequences that are assigned activities.
		pszActivityName = GetSequenceActivityName( pstudiohdr, i );
		if ( pszActivityName && pszActivityName[0] != '\0' )
		{
			iActivityIndex = ActivityList_IndexForName( pszActivityName );

			int activityToSet;
			if ( iActivityIndex == -1 )
			{
				// Allow this now.  Animators can create custom activities that are referenced only on the client or by scripts, etc.
				activityToSet = ActivityList_RegisterPrivateActivity( pszActivityName );
			}
			else
			{
				activityToSet = iActivityIndex;
			}

			StudioSeqdesc_SetActivity(pstudiohdr, i, activityToSet);
		}
	}

	pstudiohdr->sequencesindexed = true;
}

//-----------------------------------------------------------------------------
// Purpose: Ensures that activity / index relationship is recalculated
// Input  :
// Output :
//-----------------------------------------------------------------------------
void ResetActivityIndexes( studiohdr_t *pstudiohdr )
{	
	if (! pstudiohdr)
		return;

	pstudiohdr->sequencesindexed = false;
}

void VerifySequenceIndex( studiohdr_t *pstudiohdr )
{
	if (! pstudiohdr)
		return;

	if( pstudiohdr->sequencesindexed == false )
	{
		// this model's sequences have not yet been indexed by activity
		IndexModelSequences( pstudiohdr );
	}
}

int SelectWeightedSequence( studiohdr_t *pstudiohdr, int activity, int curSequence )
{
	if (! pstudiohdr)
		return 0;

	VerifySequenceIndex( pstudiohdr );

	int weighttotal = 0;
	int seq = ACTIVITY_NOT_AVAILABLE;

	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	for (int i = 0; i < pstudiohdr->GetNumLocalSeq(); i++)
	{
		// Use version-safe helper functions - no inline casting needed
		int seqActivity = StudioSeqdesc_GetActivity(pstudiohdr, i);
		int seqActWeight = StudioSeqdesc_GetActweight(pstudiohdr, i);

		if (seqActivity == activity)
		{
			if ( curSequence == i && seqActWeight < 0 )
			{
				seq = i;
				break;
			}
			weighttotal += iabs(seqActWeight);
			if (!weighttotal || random->RandomInt(0,weighttotal-1) < iabs(seqActWeight))
				seq = i;
		}
	}

	return seq;
}


int SelectHeaviestSequence( studiohdr_t *pstudiohdr, int activity )
{
	if ( !pstudiohdr )
		return 0;

	VerifySequenceIndex( pstudiohdr );

	int weight = 0;
	int seq = ACTIVITY_NOT_AVAILABLE;

	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	for (int i = 0; i < pstudiohdr->GetNumLocalSeq(); i++)
	{
		// Use version-safe helper functions - no inline casting needed
		int seqActivity = StudioSeqdesc_GetActivity(pstudiohdr, i);
		int seqActWeight = StudioSeqdesc_GetActweight(pstudiohdr, i);

		if (seqActivity == activity)
		{
			if ( iabs(seqActWeight) > weight )
			{
				weight = iabs(seqActWeight);
				seq = i;
			}
		}
	}

	return seq;
}

void GetEyePosition ( studiohdr_t *pstudiohdr, Vector &vecEyePosition )
{
	if ( !pstudiohdr )
	{
		Warning( "GetEyePosition() Can't get pstudiohdr ptr!\n" );
		return;
	}

	vecEyePosition = pstudiohdr->eyeposition;
}


//-----------------------------------------------------------------------------
// Purpose: Looks up an activity by name.
// Input  : label - Name of the activity to look up, ie "ACT_IDLE"
// Output : Activity index or ACT_INVALID if not found.
//-----------------------------------------------------------------------------
int LookupActivity( studiohdr_t *pstudiohdr, const char *label )
{
	if ( !pstudiohdr )
	{
		return 0;
	}

	if (!label)
		return ACT_INVALID;

	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	for ( int i = 0; i < pstudiohdr->GetNumLocalSeq(); i++ )
	{
		// Use version-safe helper functions - no inline casting needed
		const char *pszActName = StudioSeqdesc_GetActivityName(pstudiohdr, i);
		int seqActivity = StudioSeqdesc_GetActivity(pstudiohdr, i);

		if ( _stricmp( pszActName, label ) == 0 )
		{
			return seqActivity;
		}
	}

	return ACT_INVALID;
}


//-----------------------------------------------------------------------------
// Purpose: Looks up a sequence by sequence name first, then by activity name.
// Input  : label - The sequence name or activity name to look up.
// Output : Returns the sequence index of the matching sequence, or ACT_INVALID.
//-----------------------------------------------------------------------------
int LookupSequence( studiohdr_t *pstudiohdr, const char *label )
{
	if (! pstudiohdr)
		return 0;

	if (!label)
		return ACT_INVALID;

	// Look up by sequence name.
	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	for (int i = 0; i < pstudiohdr->GetNumLocalSeq(); i++)
	{
		// Use version-safe helper function - no inline casting needed
		const char *pszLabel = StudioSeqdesc_GetLabel(pstudiohdr, i);

		if (_stricmp( pszLabel, label ) == 0)
			return i;
	}

	// Not found, look up by activity name.
	int nActivity = LookupActivity( pstudiohdr, label );
	if (nActivity != ACT_INVALID )
	{
		return SelectWeightedSequence( pstudiohdr, nActivity );
	}

	return ACT_INVALID;
}

void GetSequenceLinearMotion( studiohdr_t *pstudiohdr, int iSequence, const float poseParameter[], Vector *pVec )
{
	if (! pstudiohdr)
	{
		Msg( "Bad pstudiohdr in GetSequenceLinearMotion()!\n" );
		return;
	}

	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	int numSeqs = pstudiohdr->GetNumLocalSeq();
	if( iSequence < 0 || iSequence >= numSeqs )
	{
		// Don't spam on bogus model
		if ( numSeqs > 0 )
		{
			Msg( "Bad sequence (%i out of %i max) in GetSequenceLinearMotion() for model '%s'!\n", iSequence, numSeqs, pstudiohdr->name );
		}
		return;
	}

	QAngle vecAngles;
	Studio_SeqMovement( pstudiohdr, iSequence, 0, 1.0, poseParameter, (*pVec), vecAngles );
}


const char *GetSequenceName( studiohdr_t *pstudiohdr, int iSequence )
{
	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	if( !pstudiohdr || iSequence < 0 || iSequence >= pstudiohdr->GetNumLocalSeq() )
	{
		if ( pstudiohdr )
		{
			Msg( "Bad sequence in GetSequenceName() for model '%s'!\n", pstudiohdr->name );
		}
		return "Unknown";
	}

	// Use version-safe helper function - no inline casting needed
	return StudioSeqdesc_GetLabel(pstudiohdr, iSequence);
}

const char *GetSequenceActivityName( studiohdr_t *pstudiohdr, int iSequence )
{
	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	if( !pstudiohdr || iSequence < 0 || iSequence >= pstudiohdr->GetNumLocalSeq() )
	{
		if ( pstudiohdr )
		{
			Msg( "Bad sequence in GetSequenceActivityName() for model '%s'!\n", pstudiohdr->name );
		}
		return "Unknown";
	}

	// Use version-safe helper function - no inline casting needed
	return StudioSeqdesc_GetActivityName(pstudiohdr, iSequence);
}

int GetSequenceFlags( studiohdr_t *pstudiohdr, int sequence )
{
	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	if ( !pstudiohdr ||
		sequence < 0 ||
		sequence >= pstudiohdr->GetNumLocalSeq() )
	{
		return 0;
	}

	// Use version-safe helper function - no inline casting needed
	return StudioSeqdesc_GetFlags(pstudiohdr, sequence);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pstudiohdr -
//			sequence -
//			type -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool HasAnimationEventOfType( studiohdr_t *pstudiohdr, int sequence, int type )
{
	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	if ( !pstudiohdr || sequence >= pstudiohdr->GetNumLocalSeq() )
		return false;

	// Use version-safe helper functions - no inline casting needed
	int numEvents = StudioSeqdesc_GetNumEvents(pstudiohdr, sequence);
	if (numEvents == 0)
		return false;

	for (int index = 0; index < numEvents; index++)
	{
		mstudioevent_t *pevent = StudioSeqdesc_GetEvent(pstudiohdr, sequence, index);
		if (pevent && pevent->event == type)
		{
			return true;
		}
	}

	return false;
}

int GetAnimationEvent( studiohdr_t *pstudiohdr, int sequence, animevent_t *pNPCEvent, float flStart, float flEnd, int index )
{
	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	if ( !pstudiohdr || sequence >= pstudiohdr->GetNumLocalSeq() || !pNPCEvent )
		return 0;

	// Use version-safe helper functions - no inline casting needed
	int numEvents = StudioSeqdesc_GetNumEvents(pstudiohdr, sequence);
	int seqFlags = StudioSeqdesc_GetFlags(pstudiohdr, sequence);

	if (numEvents == 0 || index > numEvents )
		return 0;

	for (; index < numEvents; index++)
	{
		mstudioevent_t *pevent = StudioSeqdesc_GetEvent(pstudiohdr, sequence, index);
		if (!pevent)
			continue;

		// Don't send client-side events to the server AI
		if ( pevent->event >= EVENT_CLIENT )
			continue;

		bool bOverlapEvent = false;

		if (pevent->cycle >= flStart && pevent->cycle < flEnd)
		{
			bOverlapEvent = true;
		}
		// FIXME: doesn't work with animations being played in reverse
		else if ((seqFlags & STUDIO_LOOPING) && flEnd < flStart)
		{
			if (pevent->cycle >= flStart || pevent->cycle < flEnd)
			{
				bOverlapEvent = true;
			}
		}

		if (bOverlapEvent)
		{
			pNPCEvent->pSource = NULL;
			pNPCEvent->cycle = pevent->cycle;
			pNPCEvent->eventtime = gpGlobals->curtime;
			pNPCEvent->event = pevent->event;
			pNPCEvent->options = pevent->options;
			return index + 1;
		}
	}
	return 0;
}



int FindTransitionSequence( studiohdr_t *pstudiohdr, int iCurrentSequence, int iGoalSequence, int *piDir )
{
	if ( !pstudiohdr )
		return iGoalSequence;

	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	if ( ( iCurrentSequence < 0 ) || ( iCurrentSequence >= pstudiohdr->GetNumLocalSeq() ) )
		return iGoalSequence;

	// Use version-safe helper functions - no inline casting needed
	int currentEntryNode = StudioSeqdesc_GetEntryNode(pstudiohdr, iCurrentSequence);
	int currentExitNode = StudioSeqdesc_GetExitNode(pstudiohdr, iCurrentSequence);
	int goalEntryNode = StudioSeqdesc_GetEntryNode(pstudiohdr, iGoalSequence);

	// bail if we're going to or from a node 0
	if (currentEntryNode == 0 || goalEntryNode == 0)
	{
		*piDir = 1;
		return iGoalSequence;
	}

	int	iEndNode;

	// check to see if we should be going forward or backward through the graph
	if (*piDir > 0)
	{
		iEndNode = currentExitNode;
	}
	else
	{
		iEndNode = currentEntryNode;
	}

	// if both sequences are on the same node, just go there
	if (iEndNode == goalEntryNode)
	{
		*piDir = 1;
		return iGoalSequence;
	}

	byte *pTransition = pstudiohdr->pTransition( 0 );

	int iInternNode = pTransition[(iEndNode-1)*pstudiohdr->numtransitions + (goalEntryNode-1)];

	// if there is no transitionial node, just go to the goal sequence
	if (iInternNode == 0)
		return iGoalSequence;

	// look for someone going from the entry node to next node it should hit
	// this may be the goal sequences node or an intermediate node
	// CRITICAL: Use version-aware accessor - numseq is v37 field, v44+ uses numlocalseq
	for (int i = 0; i < pstudiohdr->GetNumLocalSeq(); i++)
	{
		// Use version-safe helper functions - no inline casting needed
		int entryNode = StudioSeqdesc_GetEntryNode(pstudiohdr, i);
		int exitNode = StudioSeqdesc_GetExitNode(pstudiohdr, i);
		int nodeFlags = StudioSeqdesc_GetNodeFlags(pstudiohdr, i);

		if (entryNode == iEndNode && exitNode == iInternNode)
		{
			*piDir = 1;
			return i;
		}
		if (nodeFlags)
		{
			if (exitNode == iEndNode && entryNode == iInternNode)
			{
				*piDir = -1;
				return i;
			}
		}
	}

	// this means that two parts of the node graph are not connected.
	Msg( "error in transition graph" );
	// Go ahead and jump to the goal sequence
	return iGoalSequence;
}

void SetBodygroup( studiohdr_t *pstudiohdr, int& body, int iGroup, int iValue )
{
	if (! pstudiohdr)
		return;

	if (iGroup >= StudioHdr_GetNumBodyparts(pstudiohdr))
		return;

	mstudiobodyparts_t *pbodypart = StudioHdr_GetBodypart( pstudiohdr, iGroup );

	if (iValue >= pbodypart->nummodels)
		return;

	int iCurrent = (body / pbodypart->base) % pbodypart->nummodels;

	body = (body - (iCurrent * pbodypart->base) + (iValue * pbodypart->base));
}


int GetBodygroup( studiohdr_t *pstudiohdr, int body, int iGroup )
{
	if (! pstudiohdr)
		return 0;

	if (iGroup >= StudioHdr_GetNumBodyparts(pstudiohdr))
		return 0;

	mstudiobodyparts_t *pbodypart = StudioHdr_GetBodypart( pstudiohdr, iGroup );

	if (pbodypart->nummodels <= 1)
		return 0;

	int iCurrent = (body / pbodypart->base) % pbodypart->nummodels;

	return iCurrent;
}

const char *GetBodygroupName( studiohdr_t *pstudiohdr, int iGroup )
{
	if ( !pstudiohdr)
		return "";

	if (iGroup >= StudioHdr_GetNumBodyparts(pstudiohdr))
		return "";

	mstudiobodyparts_t *pbodypart = StudioHdr_GetBodypart( pstudiohdr, iGroup );
	return pbodypart->pszName();
}

int FindBodygroupByName( studiohdr_t *pstudiohdr, const char *name )
{
	if ( !pstudiohdr )
		return -1;

	int group;
	for ( group = 0; group < StudioHdr_GetNumBodyparts(pstudiohdr); group++ )
	{
		mstudiobodyparts_t *pbodypart = StudioHdr_GetBodypart( pstudiohdr, group );
		if ( !Q_strcasecmp( name, pbodypart->pszName() ) )
		{
			return group;
		}
	}

	return -1;
}

int GetBodygroupCount( studiohdr_t *pstudiohdr, int iGroup )
{
	if ( !pstudiohdr )
		return 0;

	if (iGroup >= StudioHdr_GetNumBodyparts(pstudiohdr))
		return 0;

	mstudiobodyparts_t *pbodypart = StudioHdr_GetBodypart( pstudiohdr, iGroup );
	return pbodypart->nummodels;
}

int GetNumBodyGroups( studiohdr_t *pstudiohdr )
{
	if ( !pstudiohdr )
		return 0;

	return StudioHdr_GetNumBodyparts(pstudiohdr);
}

int GetSequenceActivity( studiohdr_t *pstudiohdr, int sequence )
{
	if (! pstudiohdr)
		return 0;

	// Use version-safe helper function - no inline casting needed
	return StudioSeqdesc_GetActivity(pstudiohdr, sequence);
}


void GetAttachmentLocalSpace( studiohdr_t *pstudiohdr, int attachIndex, matrix3x4_t &pLocalToWorld )
{
	if ( attachIndex >= 0 )
	{
		const matrix3x4_t *pLocal = StudioAttachment_GetLocal(pstudiohdr, attachIndex);
		if ( pLocal )
		{
			MatrixCopy( *pLocal, pLocalToWorld );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pstudiohdr - 
//			*name - 
// Output : int
//-----------------------------------------------------------------------------
int FindHitboxSetByName( studiohdr_t *pstudiohdr, const char *name )
{
	if ( !pstudiohdr )
		return -1;

	for ( int i = 0; i < StudioHdr_GetNumHitboxSets(pstudiohdr); i++ )
	{
		mstudiohitboxset_t *set = StudioHdr_GetHitboxSet( pstudiohdr, i );
		if ( !set )
			continue;

		if ( !_stricmp( StudioHitboxSet_GetName(pstudiohdr, i), name ) )
			return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pstudiohdr - 
//			setnumber - 
// Output : char const
//-----------------------------------------------------------------------------
const char *GetHitboxSetName( studiohdr_t *pstudiohdr, int setnumber )
{
	if ( !pstudiohdr )
		return "";

	mstudiohitboxset_t *set = StudioHdr_GetHitboxSet( pstudiohdr, setnumber );
	if ( !set )
		return "";

	return StudioHitboxSet_GetName(pstudiohdr, setnumber);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pstudiohdr - 
// Output : int
//-----------------------------------------------------------------------------
int GetHitboxSetCount( studiohdr_t *pstudiohdr )
{
	if ( !pstudiohdr )
		return 0;

	return StudioHdr_GetNumHitboxSets(pstudiohdr);
}
