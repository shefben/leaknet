//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Virtual model implementation for v48 include models
//          Handles compositing multiple MDL files via $includemodel
//
// This implementation provides virtual model support for combining animations,
// sequences, attachments, and other data from multiple model files.
//
// Based on Source Engine 2007 implementation with adaptations for LeakNet.
//
//=============================================================================//

#include "cbase.h"
#include "studio.h"
#include "studiohdr_v44.h"  // Version-aware bone accessors for v44+
#include "studio_helpers.h"  // Helper functions for studiohdr_t access
#include "utlvector.h"
#include "utldict.h"
#include "vstdlib/strtools.h"
#include "engine/ivmodelinfo.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Model info interface - available to both client and server DLLs
extern IVModelInfo* modelinfo;

//-----------------------------------------------------------------------------
// Lookup table optimization for sequence/animation deduplication
// A string table to speed up searching for sequences in the current virtual model
//-----------------------------------------------------------------------------
struct modellookup_t
{
	CUtlDict<short,short> seqTable;
	CUtlDict<short,short> animTable;
};

static CUtlVector<modellookup_t> g_ModelLookup;
static int g_ModelLookupIndex = -1;

inline bool HasLookupTable()
{
	return g_ModelLookupIndex >= 0 ? true : false;
}

inline CUtlDict<short,short> *GetSeqTable()
{
	return &g_ModelLookup[g_ModelLookupIndex].seqTable;
}

inline CUtlDict<short,short> *GetAnimTable()
{
	return &g_ModelLookup[g_ModelLookupIndex].animTable;
}

//-----------------------------------------------------------------------------
// CModelLookupContext - RAII helper for managing lookup table lifetime
// Creates a lookup table for the root model with include models
//-----------------------------------------------------------------------------
class CModelLookupContext
{
public:
	CModelLookupContext(int group, const studiohdr_t *pStudioHdr);
	~CModelLookupContext();

private:
	int m_lookupIndex;
};

CModelLookupContext::CModelLookupContext(int group, const studiohdr_t *pStudioHdr)
{
	m_lookupIndex = -1;
	if (group == 0 && StudioHdr_GetNumIncludeModels(pStudioHdr))
	{
		m_lookupIndex = g_ModelLookup.AddToTail();
		g_ModelLookupIndex = g_ModelLookup.Count() - 1;
	}
}

CModelLookupContext::~CModelLookupContext()
{
	if (m_lookupIndex >= 0)
	{
		Assert(m_lookupIndex == (g_ModelLookup.Count() - 1));
		g_ModelLookup.FastRemove(m_lookupIndex);
		g_ModelLookupIndex = g_ModelLookup.Count() - 1;
	}
}

//-----------------------------------------------------------------------------
// virtualgroup_t::GetStudioHdr
// Retrieves the studiohdr_t for this virtual group from its cache handle
// The cache member is a model_t pointer set during AppendModels
//-----------------------------------------------------------------------------
const studiohdr_t *virtualgroup_t::GetStudioHdr( void ) const
{
	if (!cache)
		return NULL;

	// In the engine, cache is a model_t pointer
	// Use the model info interface to get the studiohdr_t
	if (!modelinfo)
		return NULL;

	return modelinfo->GetStudiomodel((const model_t *)cache);
}

//-----------------------------------------------------------------------------
// studiohdr_t::FindModel
// Loads an included model by name and returns its studiohdr_t
// This function is called recursively when building the virtual model
//-----------------------------------------------------------------------------
const studiohdr_t *studiohdr_t::FindModel( void **ppCache, const char *pModelName ) const
{
	if (!ppCache || !pModelName)
		return NULL;

	*ppCache = NULL;

	if (!modelinfo)
	{
		DevWarning("studiohdr_t::FindModel: modelinfo interface not available\n");
		return NULL;
	}

	// Get the model index - this will load/reference the model
	int modelIndex = modelinfo->GetModelIndex(pModelName);
	if (modelIndex == -1)
	{
		DevWarning("studiohdr_t::FindModel: Failed to find include model: %s\n", pModelName);
		return NULL;
	}

	// Get the model_t pointer
	const model_t *pModel = modelinfo->GetModel(modelIndex);
	if (!pModel)
	{
		DevWarning("studiohdr_t::FindModel: Failed to get model: %s\n", pModelName);
		return NULL;
	}

	// Store the model_t pointer as the cache handle
	*ppCache = (void *)pModel;

	// Get and return the studiohdr_t
	return modelinfo->GetStudiomodel(pModel);
}

//-----------------------------------------------------------------------------
// virtualmodel_t implementation
// These functions append data from included models to the virtual model
//-----------------------------------------------------------------------------

void virtualmodel_t::AppendModels( int group, const studiohdr_t *pStudioHdr )
{
	// Build a search table if necessary (only for root model with includes)
	CModelLookupContext ctx(group, pStudioHdr);

	AppendSequences( group, pStudioHdr );
	AppendAnimations( group, pStudioHdr );
	AppendBonemap( group, pStudioHdr );
	AppendAttachments( group, pStudioHdr );
	AppendPoseParameters( group, pStudioHdr );
	AppendNodes( group, pStudioHdr );
	AppendIKLocks( group, pStudioHdr );

	// Structure to temporarily cache found models
	// Prevents ref counting issues from calling FindModel repeatedly
	struct HandleAndHeader_t
	{
		void *handle;
		const studiohdr_t *pHdr;
	};
	HandleAndHeader_t list[64];

	// Determine quantity of valid include models in one pass
	int j;
	int nValidIncludes = 0;
	for (j = 0; j < StudioHdr_GetNumIncludeModels(pStudioHdr); j++)
	{
		// Find model (increases ref count)
		void *tmp = NULL;
		mstudiomodelgroup_t *pModelGroup = StudioHdr_GetModelGroup(pStudioHdr, j);
		if (!pModelGroup)
			continue;
		const studiohdr_t *pTmpHdr = pStudioHdr->FindModel(&tmp, pModelGroup->pszName());
		if (pTmpHdr)
		{
			if (nValidIncludes >= ARRAYSIZE(list))
			{
				// Would cause stack overflow
				Assert(0);
				Warning("virtualmodel_t::AppendModels: Too many include models (max 64)\n");
				break;
			}

			list[nValidIncludes].handle = tmp;
			list[nValidIncludes].pHdr = pTmpHdr;
			nValidIncludes++;
		}
	}

	// Now process all valid includes
	if (nValidIncludes)
	{
		m_group.EnsureCapacity(m_group.Count() + nValidIncludes);
		for (j = 0; j < nValidIncludes; j++)
		{
			int newGroup = m_group.AddToTail();
			m_group[newGroup].cache = list[j].handle;
			AppendModels(newGroup, list[j].pHdr);
		}
	}

	UpdateAutoplaySequences( pStudioHdr );
}

//-----------------------------------------------------------------------------
// AppendSequences - Add sequences from included model with deduplication
// Uses lookup table optimization for large models with many includes
// Supports STUDIO_OVERRIDE flag for forward-declared sequence replacement
//-----------------------------------------------------------------------------
void virtualmodel_t::AppendSequences( int group, const studiohdr_t *pStudioHdr )
{
	int numCheck = m_seq.Count();

	int j, k;

	CUtlVector< virtualsequence_t > seq;
	seq = m_seq;

	int numLocalSeq = StudioHdr_GetNumLocalSeq(pStudioHdr);
	m_group[group].masterSeq.SetCount(numLocalSeq);

	for (j = 0; j < numLocalSeq; j++)
	{
		const char *s1 = StudioSeqdesc_GetLabel(pStudioHdr, j);

		if (HasLookupTable())
		{
			k = numCheck;
			short index = GetSeqTable()->Find(s1);
			if (index != GetSeqTable()->InvalidIndex())
			{
				k = GetSeqTable()->Element(index);
			}
		}
		else
		{
			for (k = 0; k < numCheck; k++)
			{
				const studiohdr_t *hdr = m_group[seq[k].group].GetStudioHdr();
				if (hdr)
				{
					const char *s2 = StudioSeqdesc_GetLabel(hdr, seq[k].index);
					if (!Q_stricmp(s1, s2))
					{
						break;
					}
				}
			}
		}

		// No duplication
		if (k == numCheck)
		{
			virtualsequence_t tmp;
			tmp.group = group;
			tmp.index = j;
			tmp.flags = StudioSeqdesc_GetFlags(pStudioHdr, j);
			tmp.activity = StudioSeqdesc_GetActivity(pStudioHdr, j);
			k = seq.AddToTail(tmp);
		}
		else
		{
			// Check if the existing sequence is forward-declared (STUDIO_OVERRIDE)
			const studiohdr_t *existingHdr = m_group[seq[k].group].GetStudioHdr();
			if (existingHdr && (StudioSeqdesc_GetFlags(existingHdr, seq[k].index) & STUDIO_OVERRIDE))
			{
				// The one in memory is a forward declared sequence, override it
				virtualsequence_t tmp;
				tmp.group = group;
				tmp.index = j;
				tmp.flags = StudioSeqdesc_GetFlags(pStudioHdr, j);
				tmp.activity = StudioSeqdesc_GetActivity(pStudioHdr, j);
				seq[k] = tmp;
			}
		}

		m_group[group].masterSeq[j] = k;
	}

	// Add new sequences to lookup table
	if (HasLookupTable())
	{
		for (j = numCheck; j < seq.Count(); j++)
		{
			const studiohdr_t *hdr = m_group[seq[j].group].GetStudioHdr();
			if (hdr)
			{
				const char *s1 = StudioSeqdesc_GetLabel(hdr, seq[j].index);
				GetSeqTable()->Insert(s1, j);
			}
		}
	}

	m_seq = seq;
}

//-----------------------------------------------------------------------------
// AppendAnimations - Add animations from included model with deduplication
// Uses lookup table optimization for large models
//-----------------------------------------------------------------------------
void virtualmodel_t::AppendAnimations( int group, const studiohdr_t *pStudioHdr )
{
	int numCheck = m_anim.Count();

	CUtlVector< virtualgeneric_t > anim;
	anim = m_anim;

	int j, k;

	int numLocalAnim = StudioHdr_GetNumLocalAnims(pStudioHdr);
	m_group[group].masterAnim.SetCount(numLocalAnim);

	for (j = 0; j < numLocalAnim; j++)
	{
		const char *s1 = StudioAnimdesc_GetName(pStudioHdr, j);

		if (HasLookupTable())
		{
			k = numCheck;
			short index = GetAnimTable()->Find(s1);
			if (index != GetAnimTable()->InvalidIndex())
			{
				k = GetAnimTable()->Element(index);
			}
		}
		else
		{
			for (k = 0; k < numCheck; k++)
			{
				const studiohdr_t *hdr = m_group[anim[k].group].GetStudioHdr();
				if (hdr)
				{
					const char *s2 = StudioAnimdesc_GetName(hdr, anim[k].index);
					if (!Q_stricmp(s1, s2))
					{
						break;
					}
				}
			}
		}

		// No duplication
		if (k == numCheck)
		{
			virtualgeneric_t tmp;
			tmp.group = group;
			tmp.index = j;
			k = anim.AddToTail(tmp);
		}

		m_group[group].masterAnim[j] = k;
	}

	// Add new animations to lookup table
	if (HasLookupTable())
	{
		for (j = numCheck; j < anim.Count(); j++)
		{
			const studiohdr_t *hdr = m_group[anim[j].group].GetStudioHdr();
			if (hdr)
			{
				const char *s1 = StudioAnimdesc_GetName(hdr, anim[j].index);
				GetAnimTable()->Insert(s1, j);
			}
		}
	}

	m_anim = anim;
}

//-----------------------------------------------------------------------------
// AppendBonemap - Map bones between included model and base model
// Includes parent bone mismatch validation
// Version-aware: Works with both v37 and v44+ bone structures
//-----------------------------------------------------------------------------
void virtualmodel_t::AppendBonemap( int group, const studiohdr_t *pStudioHdr )
{
	const studiohdr_t *pBaseStudioHdr = m_group[0].GetStudioHdr();
	if (!pBaseStudioHdr)
	{
		// For the base group, use the same header
		pBaseStudioHdr = pStudioHdr;
	}

	// Get bone counts using version-aware accessor
	int numBones = StudioHdr_GetNumBones(pStudioHdr);
	int numBaseBones = StudioHdr_GetNumBones(pBaseStudioHdr);

	m_group[group].boneMap.SetCount(numBaseBones);
	m_group[group].masterBone.SetCount(numBones);

	int j, k;

	if (group == 0)
	{
		// Base model - bones map 1:1
		for (j = 0; j < numBones; j++)
		{
			m_group[group].boneMap[j] = j;
			m_group[group].masterBone[j] = j;
		}
	}
	else
	{
		// Included model - map bones by name
		for (j = 0; j < numBaseBones; j++)
		{
			m_group[group].boneMap[j] = -1;
		}

		for (j = 0; j < numBones; j++)
		{
			// Use version-aware bone name accessor
			const char *szBoneName = StudioBone_GetName(pStudioHdr, j);
			for (k = 0; k < numBaseBones; k++)
			{
				if (!Q_stricmp(szBoneName, StudioBone_GetName(pBaseStudioHdr, k)))
				{
					break;
				}
			}

			if (k < numBaseBones)
			{
				m_group[group].masterBone[j] = k;
				m_group[group].boneMap[k] = j;

				// Validate parent bone matching using version-aware accessor
				int parentJ = StudioBone_GetParent(pStudioHdr, j);
				int parentK = StudioBone_GetParent(pBaseStudioHdr, k);

				if ((parentJ == -1) || (parentK == -1))
				{
					if ((parentJ != -1) || (parentK != -1))
					{
						Warning("%s/%s : mismatched parent bones on \"%s\"\n",
							pBaseStudioHdr->name, pStudioHdr->name, szBoneName);
					}
				}
				else if (m_group[group].masterBone[parentJ] !=
				         m_group[0].masterBone[parentK])
				{
					Warning("%s/%s : mismatched parent bones on \"%s\"\n",
						pBaseStudioHdr->name, pStudioHdr->name, szBoneName);
				}
			}
			else
			{
				m_group[group].masterBone[j] = -1;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// AppendAttachments - Add attachments from included model
// Sets BONE_USED_BY_ATTACHMENT flags on bones used by new attachments
// Version-aware: Works with both v37 and v44+ models
//-----------------------------------------------------------------------------
void virtualmodel_t::AppendAttachments( int group, const studiohdr_t *pStudioHdr )
{
	int numCheck = m_attachment.Count();

	CUtlVector< virtualgeneric_t > attachment;
	attachment = m_attachment;

	int j, k, n;

	m_group[group].masterAttachment.SetCount(StudioHdr_GetNumAttachments(pStudioHdr));

	for (j = 0; j < StudioHdr_GetNumAttachments(pStudioHdr); j++)
	{
		// Get the mapped bone index in the base model
		int attachmentBone = StudioAttachment_GetBone(pStudioHdr, j);
		if (attachmentBone < 0 || attachmentBone >= m_group[group].masterBone.Count())
		{
			m_group[group].masterAttachment[j] = -1;
			continue;
		}

		n = m_group[group].masterBone[attachmentBone];

		// Skip if the attachment's bone doesn't exist in the base model
		if (n == -1)
		{
			m_group[group].masterAttachment[j] = -1;
			continue;
		}

		const char *s1 = StudioAttachment_GetName(pStudioHdr, j);

		// Check for duplicate attachments
		for (k = 0; k < numCheck; k++)
		{
			const studiohdr_t *hdr = m_group[attachment[k].group].GetStudioHdr();
			if (hdr)
			{
				const char *s2 = StudioAttachment_GetName(hdr, attachment[k].index);
				if (!Q_stricmp(s1, s2))
				{
					break;
				}
			}
		}

		// No duplication
		if (k == numCheck)
		{
			virtualgeneric_t tmp;
			tmp.group = group;
			tmp.index = j;
			k = attachment.AddToTail(tmp);

			// Make sure bone flags are set so attachment calculates
			// Use version-aware accessor to check flags
			const studiohdr_t *pBaseHdr = m_group[0].GetStudioHdr();
			if (pBaseHdr && (StudioBone_GetFlags(pBaseHdr, n) & BONE_USED_BY_ATTACHMENT) == 0)
			{
				// Walk up the bone hierarchy setting the flag
				int boneIndex = n;
				while (boneIndex != -1)
				{
					// Set bone flags using version-aware approach
					// We need to handle both v37 and v48 bone structures
					if (pBaseHdr->version >= STUDIO_VERSION_44)
					{
						// v44+ uses mstudiobone_v44_t
						mstudiobone_v44_t *pBone = const_cast<mstudiobone_v44_t*>(
							((const studiohdr_v44_t*)pBaseHdr)->pBone(boneIndex));
						pBone->flags |= BONE_USED_BY_ATTACHMENT;
					}
					else
					{
						// v37 uses mstudiobone_t
						mstudiobone_t *pBone = const_cast<mstudiobone_t*>(pBaseHdr->pBone(boneIndex));
						pBone->flags |= BONE_USED_BY_ATTACHMENT;
					}

					// Also update linear bone data if present
					if (pBaseHdr->pLinearBones())
					{
						int *pflags = const_cast<int*>(pBaseHdr->pLinearBones()->pflags(boneIndex));
						*pflags |= BONE_USED_BY_ATTACHMENT;
					}

					// Get parent using version-aware accessor
					boneIndex = StudioBone_GetParent(pBaseHdr, boneIndex);
				}
			}
		}

		m_group[group].masterAttachment[j] = k;
	}

	m_attachment = attachment;
}

//-----------------------------------------------------------------------------
// AppendPoseParameters - Add pose parameters with dynamic range merging
// When duplicates are found, expands the range to cover both
//-----------------------------------------------------------------------------
void virtualmodel_t::AppendPoseParameters( int group, const studiohdr_t *pStudioHdr )
{
	int numCheck = m_pose.Count();

	CUtlVector< virtualgeneric_t > pose;
	pose = m_pose;

	int j, k;

	int numLocalPoseParameters = StudioHdr_GetNumPoseParameters(pStudioHdr);
	m_group[group].masterPose.SetCount(numLocalPoseParameters);

	for (j = 0; j < numLocalPoseParameters; j++)
	{
		const char *s1 = StudioPoseParam_GetName(pStudioHdr, j);

		// Check for duplicate pose parameters
		for (k = 0; k < numCheck; k++)
		{
			const studiohdr_t *hdr = m_group[pose[k].group].GetStudioHdr();
			if (hdr)
			{
				const char *s2 = StudioPoseParam_GetName(hdr, pose[k].index);
				if (!Q_stricmp(s1, s2))
				{
					break;
				}
			}
		}

		if (k == numCheck)
		{
			// No duplication - add new pose parameter
			virtualgeneric_t tmp;
			tmp.group = group;
			tmp.index = j;
			k = pose.AddToTail(tmp);
		}
		else
		{
			// Duplicate found - merge the dynamic range
			// Reset start and end to fit full dynamic range of both
			const mstudioposeparamdesc_t *pPose1 = StudioHdr_GetPoseParameter(pStudioHdr, j);
			const studiohdr_t *existingHdr = m_group[pose[k].group].GetStudioHdr();
			if (existingHdr)
			{
				mstudioposeparamdesc_t *pPose2 = const_cast<mstudioposeparamdesc_t*>(
					StudioHdr_GetPoseParameter(existingHdr, pose[k].index));
				if (!pPose1 || !pPose2)
					continue;

				float start = min(pPose2->end, min(pPose1->end, min(pPose2->start, pPose1->start)));
				float end = max(pPose2->end, max(pPose1->end, max(pPose2->start, pPose1->start)));
				pPose2->start = start;
				pPose2->end = end;
			}
		}

		m_group[group].masterPose[j] = k;
	}

	m_pose = pose;
}

//-----------------------------------------------------------------------------
// AppendNodes - Add animation nodes for transition graph
// Note: v37 models don't have node names, so we use index-based matching
//-----------------------------------------------------------------------------
void virtualmodel_t::AppendNodes( int group, const studiohdr_t *pStudioHdr )
{
	int numlocalnodes = pStudioHdr->numlocalnodes();

	// If no nodes, set empty mapping
	if (numlocalnodes <= 0)
	{
		m_group[group].masterNode.SetCount(0);
		return;
	}

	int numCheck = m_node.Count();

	CUtlVector< virtualgeneric_t > node;
	node = m_node;

	int j, k;

	m_group[group].masterNode.SetCount(numlocalnodes);

	for (j = 0; j < numlocalnodes; j++)
	{
		// For v37 models without node names, we just use index-based identity
		// Nodes with the same index are considered the same
		// This is a simplification - proper v48 would use pszLocalNodeName

		// Check for existing node with same index
		// Since v37 doesn't have node names, we match by group 0's index
		if (group == 0)
		{
			// Base model - nodes map 1:1
			virtualgeneric_t tmp;
			tmp.group = group;
			tmp.index = j;
			k = node.AddToTail(tmp);
		}
		else
		{
			// For included models, try to match nodes
			// Without names, assume node indices should match
			k = j;
			if (k >= numCheck)
			{
				// Add new node
				virtualgeneric_t tmp;
				tmp.group = group;
				tmp.index = j;
				k = node.AddToTail(tmp);
			}
		}

		m_group[group].masterNode[j] = k;
	}

	m_node = node;
}

//-----------------------------------------------------------------------------
// AppendTransitions - Update transition table after nodes are appended
// Currently a stub - transitions use the base model's table
//-----------------------------------------------------------------------------
void virtualmodel_t::AppendTransitions( int group, const studiohdr_t *pStudioHdr )
{
	// Transitions are stored as a flat array indexed by (from * numNodes + to)
	// When merging models, the transition data from included models would need
	// to be remapped using the masterNode table
	// For now, we rely on the base model's transitions
}

//-----------------------------------------------------------------------------
// AppendIKLocks - Add IK autoplay locks with chain-based deduplication
//-----------------------------------------------------------------------------
void virtualmodel_t::AppendIKLocks( int group, const studiohdr_t *pStudioHdr )
{
	int numCheck = m_iklock.Count();

	CUtlVector< virtualgeneric_t > iklock;
	iklock = m_iklock;

	int j, k;

	int numLocalIKAutoplayLocks = StudioHdr_GetNumIKAutoplayLocks(pStudioHdr);
	for (j = 0; j < numLocalIKAutoplayLocks; j++)
	{
		const mstudioiklock_t *pLock = StudioHdr_GetIKAutoplayLock(pStudioHdr, j);
		if (!pLock)
			continue;
		int chain1 = pLock->chain;

		// Check for duplicate by chain index
		for (k = 0; k < numCheck; k++)
		{
			const studiohdr_t *hdr = m_group[iklock[k].group].GetStudioHdr();
			if (hdr)
			{
				const mstudioiklock_t *pExistingLock = StudioHdr_GetIKAutoplayLock(hdr, iklock[k].index);
				if (!pExistingLock)
					continue;
				int chain2 = pExistingLock->chain;
				if (chain1 == chain2)
				{
					break;
				}
			}
		}

		// No duplication
		if (k == numCheck)
		{
			virtualgeneric_t tmp;
			tmp.group = group;
			tmp.index = j;
			k = iklock.AddToTail(tmp);
		}
	}

	m_iklock = iklock;
}

//-----------------------------------------------------------------------------
// UpdateAutoplaySequences - Rebuild list of autoplay sequences
//-----------------------------------------------------------------------------
void virtualmodel_t::UpdateAutoplaySequences( const studiohdr_t *pStudioHdr )
{
	// Collect all sequences flagged for autoplay
	m_autoplaySequences.RemoveAll();

	for (int i = 0; i < m_seq.Count(); i++)
	{
		if (m_seq[i].flags & STUDIO_AUTOPLAY)
		{
			m_autoplaySequences.AddToTail((unsigned short)i);
		}
	}
}

//-----------------------------------------------------------------------------
// Studio_CreateVirtualModel
// Creates and initializes a virtual model for the given studiohdr_t
// This is called on-demand when GetVirtualModel() is first accessed
// Returns the created virtual model, or NULL if creation failed
//-----------------------------------------------------------------------------
virtualmodel_t *Studio_CreateVirtualModel( studiohdr_t *pStudioHdr )
{
	if (!pStudioHdr)
		return NULL;

	// Only v44+ models support virtual models
	if (pStudioHdr->version < STUDIO_VERSION_44)
		return NULL;

	// Cast to v44 header for proper field access
	// CRITICAL: For v44+ models, we MUST use studiohdr_v44_t to access fields
	// past the common header area, as their offsets differ from v37
	studiohdr_v44_t *pHdr44 = (studiohdr_v44_t *)pStudioHdr;

	// Check if already created (use v44 struct for correct offset)
	if (pHdr44->virtualModel)
		return (virtualmodel_t *)pHdr44->virtualModel;

	// If no include models, don't need a virtual model
	if (pHdr44->numincludemodels == 0)
		return NULL;

	DevMsg("Creating virtual model for %s with %d include models\n",
		pStudioHdr->name, pHdr44->numincludemodels);

	// Allocate the virtual model
	virtualmodel_t *pVModel = new virtualmodel_t;
	if (!pVModel)
	{
		Warning("Failed to allocate virtual model for %s\n", pStudioHdr->name);
		return NULL;
	}

	// Add the base model as group 0
	int nGroup = pVModel->m_group.AddToTail();
	Assert(nGroup == 0);

	// The cache for group 0 should be the model_t pointer
	// For now, store NULL - the engine will need to set this
	pVModel->m_group[nGroup].cache = NULL;

	// Build the virtual model by appending data from this model and all includes
	pVModel->AppendModels(0, pStudioHdr);

	// Store the virtual model pointer (use v44 struct for correct offset)
	pHdr44->virtualModel = pVModel;

	DevMsg("Virtual model created: %d sequences, %d animations, %d groups\n",
		pVModel->m_seq.Count(), pVModel->m_anim.Count(), pVModel->m_group.Count());

	return pVModel;
}

//-----------------------------------------------------------------------------
// Studio_DestroyVirtualModel
// Destroys a virtual model and clears the pointer
//-----------------------------------------------------------------------------
void Studio_DestroyVirtualModel( studiohdr_t *pStudioHdr )
{
	if (!pStudioHdr)
		return;

	// Only v44+ models have virtual models
	if (pStudioHdr->version < STUDIO_VERSION_44)
		return;

	// Cast to v44 header for proper field access
	studiohdr_v44_t *pHdr44 = (studiohdr_v44_t *)pStudioHdr;

	if (!pHdr44->virtualModel)
		return;

	delete (virtualmodel_t *)pHdr44->virtualModel;
	pHdr44->virtualModel = NULL;
}
