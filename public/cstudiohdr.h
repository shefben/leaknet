//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CStudioHdr wrapper class for version-agnostic access to model data
//
// This class provides a unified interface for accessing both v37 (HL2 Beta 2003)
// and v48 (Source 2007) model formats. It abstracts away the differences between
// the two formats and provides a consistent API for the engine and game code.
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSTUDIOHDR_H
#define CSTUDIOHDR_H
#pragma once

#include "studio.h"
#include "studio_helpers.h"
#include "utlvector.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
struct virtualmodel_t;
class IDataCache;

//-----------------------------------------------------------------------------
// CStudioHdr
// Wrapper class for studiohdr_t that provides version-agnostic access
//-----------------------------------------------------------------------------
class CStudioHdr
{
public:
	CStudioHdr();
	CStudioHdr( const studiohdr_t *pStudioHdr );
	~CStudioHdr() {}

	void Init( const studiohdr_t *pStudioHdr );

	// Is the header valid?
	bool IsValid() const { return m_pStudioHdr != NULL; }

	// Get the raw studiohdr_t pointer
	const studiohdr_t *GetRenderHdr() const { return m_pStudioHdr; }

	//-----------------------------------------------------------------------------
	// Version information
	//-----------------------------------------------------------------------------
	int GetVersion() const { return m_nVersion; }
	bool IsV37() const { return m_nVersion <= STUDIO_VERSION_37; }
	bool IsV44Plus() const { return m_nVersion >= STUDIO_VERSION_44; }
	bool IsV48() const { return m_nVersion >= STUDIO_VERSION_48; }

	// Check if model has embedded vertex data (v37) or external VVD (v44+)
	bool HasEmbeddedVertices() const { return STUDIO_VERSION_HAS_EMBEDDED_VERTICES(m_nVersion); }
	bool HasExternalVertices() const { return STUDIO_VERSION_HAS_EXTERNAL_VERTICES(m_nVersion); }

	//-----------------------------------------------------------------------------
	// Basic model info
	//-----------------------------------------------------------------------------
	const char *pszName() const { return m_pStudioHdr ? m_pStudioHdr->name : ""; }
	int GetLength() const { return m_pStudioHdr ? m_pStudioHdr->length : 0; }
	long GetChecksum() const { return m_pStudioHdr ? m_pStudioHdr->checksum : 0; }
	int GetFlags() const { return m_pStudioHdr ? m_pStudioHdr->flags : 0; }

	//-----------------------------------------------------------------------------
	// Bounding information
	//-----------------------------------------------------------------------------
	const Vector &eyeposition() const;
	const Vector &illumposition() const;
	const Vector &hull_min() const;
	const Vector &hull_max() const;
	const Vector &view_bbmin() const;
	const Vector &view_bbmax() const;

	//-----------------------------------------------------------------------------
	// Bones
	//-----------------------------------------------------------------------------
	int numbones() const { return StudioHdr_GetNumBones(m_pStudioHdr); }
	const mstudiobone_t *pBone( int i ) const;
	int RemapBone( int iBone ) const;
	int GetNumBoneControllers() const { return StudioHdr_GetNumBoneControllers(m_pStudioHdr); }
	const mstudiobonecontroller_t *pBonecontroller( int i ) const;

	// v48: Linear bone data access
	const mstudiolinearbone_t *pLinearBones() const;

	//-----------------------------------------------------------------------------
	// Hitboxes
	//-----------------------------------------------------------------------------
	int numhitboxsets() const { return StudioHdr_GetNumHitboxSets(m_pStudioHdr); }
	const mstudiohitboxset_t *pHitboxSet( int i ) const;
	const mstudiobbox_t *pHitbox( int i, int set ) const;
	int iHitboxCount( int set ) const;

	//-----------------------------------------------------------------------------
	// Animations (unified access for both v37 and v48)
	//-----------------------------------------------------------------------------
	int GetNumLocalAnims() const { return StudioHdr_GetNumLocalAnims(m_pStudioHdr); }
	const mstudioanimdesc_t *pLocalAnimdesc( int i ) const;

	// v37: Animation groups
	int GetNumAnimGroups() const;
	const mstudioanimgroup_t *pAnimGroup( int i ) const;

	// v48: Animation blocks (demand loading)
	int GetNumAnimBlocks() const;
	const mstudioanimblock_t *pAnimBlock( int i ) const;

	//-----------------------------------------------------------------------------
	// Sequences (unified access for both v37 and v48)
	//-----------------------------------------------------------------------------
	int GetNumLocalSeq() const { return StudioHdr_GetNumLocalSeq(m_pStudioHdr); }
	const mstudioseqdesc_t *pLocalSeqdesc( int i ) const;

	// v37: Sequence groups
	int GetNumSeqGroups() const { return m_pStudioHdr ? m_pStudioHdr->numseqgroups : 0; }
	const mstudioseqgroup_t *pSeqGroup( int i ) const;

	// v48: Include models
	int GetNumIncludeModels() const;
	const mstudiomodelgroup_t *pModelGroup( int i ) const;

	//-----------------------------------------------------------------------------
	// Attachments
	//-----------------------------------------------------------------------------
	int GetNumAttachments() const { return StudioHdr_GetNumAttachments(m_pStudioHdr); }
	const mstudioattachment_t *pAttachment( int i ) const;

	//-----------------------------------------------------------------------------
	// Body parts
	//-----------------------------------------------------------------------------
	int numbodyparts() const { return StudioHdr_GetNumBodyparts(m_pStudioHdr); }
	const mstudiobodyparts_t *pBodypart( int i ) const;

	//-----------------------------------------------------------------------------
	// Textures/Materials
	//-----------------------------------------------------------------------------
	int numtextures() const { return StudioHdr_GetNumTextures(m_pStudioHdr); }
	const mstudiotexture_t *pTexture( int i ) const;
	int numcdtextures() const { return StudioHdr_GetNumCdTextures(m_pStudioHdr); }
	const char *pCdtexture( int i ) const;
	int numskinref() const { return StudioHdr_GetNumSkinRef(m_pStudioHdr); }
	int numskinfamilies() const { return StudioHdr_GetNumSkinFamilies(m_pStudioHdr); }
	const short *pSkinref( int i ) const;

	//-----------------------------------------------------------------------------
	// Flex
	//-----------------------------------------------------------------------------
	int numflexdesc() const { return StudioHdr_GetNumFlexDescs(m_pStudioHdr); }
	const mstudioflexdesc_t *pFlexdesc( int i ) const;
	int numflexcontrollers() const { return StudioHdr_GetNumFlexControllers(m_pStudioHdr); }
	const mstudioflexcontroller_t *pFlexcontroller( int i ) const;
	int numflexrules() const { return StudioHdr_GetNumFlexRules(m_pStudioHdr); }
	const mstudioflexrule_t *pFlexRule( int i ) const;

	// v48: Flex controller UI
	int numflexcontrollerui() const;
	const mstudioflexcontrollerui_t *pFlexControllerUI( int i ) const;

	//-----------------------------------------------------------------------------
	// IK
	//-----------------------------------------------------------------------------
	int numikchains() const { return StudioHdr_GetNumIKChains(m_pStudioHdr); }
	const mstudioikchain_t *pIKChain( int i ) const;
	int numikautoplaylocks() const { return StudioHdr_GetNumIKAutoplayLocks(m_pStudioHdr); }
	const mstudioiklock_t *pIKAutoplayLock( int i ) const;

	//-----------------------------------------------------------------------------
	// Mouths
	//-----------------------------------------------------------------------------
	int nummouths() const { return StudioHdr_GetNumMouths(m_pStudioHdr); }
	const mstudiomouth_t *pMouth( int i ) const;

	//-----------------------------------------------------------------------------
	// Pose parameters
	//-----------------------------------------------------------------------------
	int numposeparameters() const { return StudioHdr_GetNumPoseParameters(m_pStudioHdr); }
	const mstudioposeparamdesc_t *pPoseParameter( int i ) const;

	//-----------------------------------------------------------------------------
	// Keyvalues
	//-----------------------------------------------------------------------------
	const char *KeyValueText() const { return m_pStudioHdr ? m_pStudioHdr->KeyValueText() : NULL; }

	//-----------------------------------------------------------------------------
	// Surface prop
	//-----------------------------------------------------------------------------
	const char *pszSurfaceProp() const { return m_pStudioHdr ? m_pStudioHdr->pszSurfaceProp() : ""; }

	//-----------------------------------------------------------------------------
	// Mass
	//-----------------------------------------------------------------------------
	float mass() const { return StudioHdr_GetMass(m_pStudioHdr); }
	int contents() const { return StudioHdr_GetContents(m_pStudioHdr); }

	//-----------------------------------------------------------------------------
	// Virtual model support (v48)
	//-----------------------------------------------------------------------------
	void *GetVirtualModel() const;
	void SetVirtualModel( void *pVirtualModel );

	//-----------------------------------------------------------------------------
	// v48: studiohdr2_t extension access
	//-----------------------------------------------------------------------------
	const studiohdr2_t *pStudioHdr2() const;

	//-----------------------------------------------------------------------------
	// v48: Root LOD
	//-----------------------------------------------------------------------------
	int GetRootLOD() const;
	int GetNumAllowedRootLODs() const;

	//-----------------------------------------------------------------------------
	// v48: Bone table by name
	//-----------------------------------------------------------------------------
	const byte *GetBoneTableSortedByName() const;

	//-----------------------------------------------------------------------------
	// Virtual model support (v44+ include models)
	//-----------------------------------------------------------------------------
	bool IsVirtual() const { return m_pVModel != NULL; }

	// Get the studiohdr for a specific sequence (may be from include model)
	const studiohdr_t *pSeqStudioHdr( int sequence ) const;

	// Get the studiohdr for a specific animation (may be from include model)
	const studiohdr_t *pAnimStudioHdr( int animation ) const;

	// Bone remapping for virtual models
	int RemapAnimBone( int iAnim, int iLocalBone ) const;
	int RemapSeqBone( int iSequence, int iLocalBone ) const;

	// Get total number of sequences (including from include models)
	int GetNumSeq() const;

	// Get total number of animations (including from include models)
	int GetNumAnim() const;

	// Get sequence/animation descriptor (handles include models)
	const mstudioseqdesc_t *pSeqdesc( int i ) const;
	const mstudioanimdesc_t *pAnimdesc( int i ) const;

	// Relative animation/sequence indices (for virtual models)
	int iRelativeAnim( int baseseq, int relanim ) const;
	int iRelativeSeq( int baseseq, int relseq ) const;

private:
	// Internal helpers for virtual model support
	void InitVirtualModel();
	const studiohdr_t *GroupStudioHdr( int group ) const;

private:
	const studiohdr_t	*m_pStudioHdr;
	int					m_nVersion;

	// Virtual model support (for v44+ include models)
	mutable virtualmodel_t	*m_pVModel;
	mutable CUtlVector< const studiohdr_t * > m_pStudioHdrCache;
};

//-----------------------------------------------------------------------------
// Inline implementations
//-----------------------------------------------------------------------------

inline CStudioHdr::CStudioHdr()
{
	m_pStudioHdr = NULL;
	m_nVersion = 0;
	m_pVModel = NULL;
}

inline CStudioHdr::CStudioHdr( const studiohdr_t *pStudioHdr )
{
	m_pStudioHdr = NULL;
	m_nVersion = 0;
	m_pVModel = NULL;
	Init( pStudioHdr );
}

inline void CStudioHdr::Init( const studiohdr_t *pStudioHdr )
{
	m_pStudioHdr = pStudioHdr;
	m_nVersion = pStudioHdr ? pStudioHdr->version : 0;

	// Clear virtual model cache
	m_pVModel = NULL;
	m_pStudioHdrCache.RemoveAll();

	// For v44+ models, initialize virtual model if present
	if (pStudioHdr && IsV44Plus())
	{
		InitVirtualModel();
	}
}

//-----------------------------------------------------------------------------
// InitVirtualModel - Set up virtual model for v44+ include models
//-----------------------------------------------------------------------------
inline void CStudioHdr::InitVirtualModel()
{
	if (!m_pStudioHdr || !IsV44Plus())
		return;

	// Get virtual model from the version-correct header field.
	m_pVModel = (virtualmodel_t*)GetVirtualModel();

	// If no include models, no virtual model needed
	if (!m_pVModel)
		return;

	// Initialize the studio header cache
	int nGroups = m_pVModel->m_group.Count();
	m_pStudioHdrCache.SetCount( nGroups );
	for (int i = 0; i < nGroups; i++)
	{
		m_pStudioHdrCache[i] = NULL;
	}

	// First group is always the main model
	m_pStudioHdrCache[0] = m_pStudioHdr;
}

//-----------------------------------------------------------------------------
// GroupStudioHdr - Get the studiohdr for a model group (include model)
//-----------------------------------------------------------------------------
inline const studiohdr_t *CStudioHdr::GroupStudioHdr( int group ) const
{
	if (!m_pVModel)
		return m_pStudioHdr;

	if (group < 0 || group >= m_pStudioHdrCache.Count())
		return m_pStudioHdr;

	// Check cache first
	if (m_pStudioHdrCache[group])
		return m_pStudioHdrCache[group];

	// Group 0 is always the main model
	if (group == 0)
	{
		m_pStudioHdrCache[0] = m_pStudioHdr;
		return m_pStudioHdr;
	}

	const studiohdr_t *pGroupHdr = m_pVModel->m_group[group].GetStudioHdr();
	if (!pGroupHdr)
		return m_pStudioHdr;

	m_pStudioHdrCache[group] = pGroupHdr;
	return pGroupHdr;
}

//-----------------------------------------------------------------------------
// GetNumSeq - Total sequences including from include models
//-----------------------------------------------------------------------------
inline int CStudioHdr::GetNumSeq() const
{
	if (!m_pStudioHdr)
		return 0;

	if (!m_pVModel)
		return StudioHdr_GetNumLocalSeq(m_pStudioHdr);

	return m_pVModel->m_seq.Count();
}

//-----------------------------------------------------------------------------
// GetNumAnim - Total animations including from include models
//-----------------------------------------------------------------------------
inline int CStudioHdr::GetNumAnim() const
{
	if (!m_pStudioHdr)
		return 0;

	if (!m_pVModel)
		return StudioHdr_GetNumLocalAnims(m_pStudioHdr);

	return m_pVModel->m_anim.Count();
}

//-----------------------------------------------------------------------------
// pSeqStudioHdr - Get studiohdr for a specific sequence
//-----------------------------------------------------------------------------
inline const studiohdr_t *CStudioHdr::pSeqStudioHdr( int sequence ) const
{
	if (!m_pVModel || sequence < 0 || sequence >= m_pVModel->m_seq.Count())
		return m_pStudioHdr;

	return GroupStudioHdr( m_pVModel->m_seq[sequence].group );
}

//-----------------------------------------------------------------------------
// pAnimStudioHdr - Get studiohdr for a specific animation
//-----------------------------------------------------------------------------
inline const studiohdr_t *CStudioHdr::pAnimStudioHdr( int animation ) const
{
	if (!m_pVModel || animation < 0 || animation >= m_pVModel->m_anim.Count())
		return m_pStudioHdr;

	return GroupStudioHdr( m_pVModel->m_anim[animation].group );
}

//-----------------------------------------------------------------------------
// pSeqdesc - Get sequence descriptor (handles virtual models)
//-----------------------------------------------------------------------------
inline const mstudioseqdesc_t *CStudioHdr::pSeqdesc( int i ) const
{
	if (!m_pStudioHdr)
		return NULL;

	if (!m_pVModel)
	{
		if (i < 0 || i >= StudioHdr_GetNumLocalSeq(m_pStudioHdr))
			return NULL;
		return m_pStudioHdr->pSeqdesc(i);
	}

	if (i < 0 || i >= m_pVModel->m_seq.Count())
		return NULL;

	const studiohdr_t *pHdr = GroupStudioHdr( m_pVModel->m_seq[i].group );
	int localIndex = m_pVModel->m_seq[i].index;

	if (localIndex < 0 || localIndex >= StudioHdr_GetNumLocalSeq(pHdr))
		return NULL;

	return pHdr->pSeqdesc(localIndex);
}

//-----------------------------------------------------------------------------
// pAnimdesc - Get animation descriptor (handles virtual models)
//-----------------------------------------------------------------------------
inline const mstudioanimdesc_t *CStudioHdr::pAnimdesc( int i ) const
{
	if (!m_pStudioHdr)
		return NULL;

	if (!m_pVModel)
	{
		if (i < 0 || i >= StudioHdr_GetNumLocalAnims(m_pStudioHdr))
			return NULL;
		return m_pStudioHdr->pAnimdesc(i);
	}

	if (i < 0 || i >= m_pVModel->m_anim.Count())
		return NULL;

	const studiohdr_t *pHdr = GroupStudioHdr( m_pVModel->m_anim[i].group );
	int localIndex = m_pVModel->m_anim[i].index;

	if (localIndex < 0 || localIndex >= StudioHdr_GetNumLocalAnims(pHdr))
		return NULL;

	return pHdr->pAnimdesc(localIndex);
}

//-----------------------------------------------------------------------------
// RemapAnimBone - Remap animation bone to global bone (for virtual models)
//-----------------------------------------------------------------------------
inline int CStudioHdr::RemapAnimBone( int iAnim, int iLocalBone ) const
{
	if (!m_pVModel || iAnim < 0 || iAnim >= m_pVModel->m_anim.Count())
		return iLocalBone;

	int group = m_pVModel->m_anim[iAnim].group;
	if (group < 0 || group >= m_pVModel->m_group.Count())
		return iLocalBone;

	const virtualgroup_t &virtualGroup = m_pVModel->m_group[group];
	if (iLocalBone < 0 || iLocalBone >= virtualGroup.masterBone.Count())
		return iLocalBone;

	return virtualGroup.masterBone[iLocalBone];
}

//-----------------------------------------------------------------------------
// RemapSeqBone - Remap sequence bone to global bone (for virtual models)
//-----------------------------------------------------------------------------
inline int CStudioHdr::RemapSeqBone( int iSequence, int iLocalBone ) const
{
	if (!m_pVModel || iSequence < 0 || iSequence >= m_pVModel->m_seq.Count())
		return iLocalBone;

	int group = m_pVModel->m_seq[iSequence].group;
	if (group < 0 || group >= m_pVModel->m_group.Count())
		return iLocalBone;

	const virtualgroup_t &virtualGroup = m_pVModel->m_group[group];
	if (iLocalBone < 0 || iLocalBone >= virtualGroup.masterBone.Count())
		return iLocalBone;

	return virtualGroup.masterBone[iLocalBone];
}

//-----------------------------------------------------------------------------
// iRelativeAnim - Map sequence-local anim reference to global anim index
//-----------------------------------------------------------------------------
inline int CStudioHdr::iRelativeAnim( int baseseq, int relanim ) const
{
	if (!m_pVModel)
		return relanim;

	if (baseseq < 0 || baseseq >= m_pVModel->m_seq.Count())
		return relanim;

	int group = m_pVModel->m_seq[baseseq].group;
	if (group < 0 || group >= m_pVModel->m_group.Count())
		return relanim;

	const virtualgroup_t &virtualGroup = m_pVModel->m_group[group];
	if (relanim < 0 || relanim >= virtualGroup.masterAnim.Count())
		return relanim;

	return virtualGroup.masterAnim[relanim];
}

//-----------------------------------------------------------------------------
// iRelativeSeq - Map sequence-local seq reference to global seq index
//-----------------------------------------------------------------------------
inline int CStudioHdr::iRelativeSeq( int baseseq, int relseq ) const
{
	if (!m_pVModel)
		return relseq;

	if (baseseq < 0 || baseseq >= m_pVModel->m_seq.Count())
		return relseq;

	int group = m_pVModel->m_seq[baseseq].group;
	if (group < 0 || group >= m_pVModel->m_group.Count())
		return relseq;

	const virtualgroup_t &virtualGroup = m_pVModel->m_group[group];
	if (relseq < 0 || relseq >= virtualGroup.masterSeq.Count())
		return relseq;

	return virtualGroup.masterSeq[relseq];
}

inline const Vector &CStudioHdr::eyeposition() const
{
	static Vector zero( 0, 0, 0 );
	return m_pStudioHdr ? m_pStudioHdr->eyeposition : zero;
}

inline const Vector &CStudioHdr::illumposition() const
{
	static Vector zero( 0, 0, 0 );
	return m_pStudioHdr ? m_pStudioHdr->illumposition : zero;
}

inline const Vector &CStudioHdr::hull_min() const
{
	static Vector zero( 0, 0, 0 );
	return m_pStudioHdr ? m_pStudioHdr->hull_min : zero;
}

inline const Vector &CStudioHdr::hull_max() const
{
	static Vector zero( 0, 0, 0 );
	return m_pStudioHdr ? m_pStudioHdr->hull_max : zero;
}

inline const Vector &CStudioHdr::view_bbmin() const
{
	static Vector zero( 0, 0, 0 );
	return m_pStudioHdr ? m_pStudioHdr->view_bbmin : zero;
}

inline const Vector &CStudioHdr::view_bbmax() const
{
	static Vector zero( 0, 0, 0 );
	return m_pStudioHdr ? m_pStudioHdr->view_bbmax : zero;
}

inline const mstudiobone_t *CStudioHdr::pBone( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumBones(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetBone( m_pStudioHdr, i );
}

inline int CStudioHdr::RemapBone( int iBone ) const
{
	// Default implementation - no remapping
	return iBone;
}

inline const mstudiobonecontroller_t *CStudioHdr::pBonecontroller( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumBoneControllers(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetBoneController( m_pStudioHdr, i );
}

inline const mstudiolinearbone_t *CStudioHdr::pLinearBones() const
{
	return m_pStudioHdr ? m_pStudioHdr->pLinearBones() : NULL;
}

inline const mstudiohitboxset_t *CStudioHdr::pHitboxSet( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumHitboxSets(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetHitboxSet( m_pStudioHdr, i );
}

inline const mstudiobbox_t *CStudioHdr::pHitbox( int i, int set ) const
{
	return StudioHitboxSet_GetHitbox( m_pStudioHdr, set, i );
}

inline int CStudioHdr::iHitboxCount( int set ) const
{
	return StudioHitboxSet_GetNumHitboxes( m_pStudioHdr, set );
}

inline const mstudioanimdesc_t *CStudioHdr::pLocalAnimdesc( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumLocalAnims(m_pStudioHdr))
		return NULL;
	return m_pStudioHdr->pAnimdesc( i );
}

inline int CStudioHdr::GetNumAnimGroups() const
{
	if (!m_pStudioHdr || !IsV37())
		return 0;
	return m_pStudioHdr->animdata.v37.numanimgroups;
}

inline const mstudioanimgroup_t *CStudioHdr::pAnimGroup( int i ) const
{
	return m_pStudioHdr ? m_pStudioHdr->pAnimgroup( i ) : NULL;
}

inline int CStudioHdr::GetNumAnimBlocks() const
{
	if (!m_pStudioHdr || !IsV44Plus())
		return 0;
	return StudioHdr_GetNumAnimBlocks(m_pStudioHdr);
}

inline const mstudioanimblock_t *CStudioHdr::pAnimBlock( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumAnimBlocks(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetAnimBlock( m_pStudioHdr, i );
}

inline const mstudioseqdesc_t *CStudioHdr::pLocalSeqdesc( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumLocalSeq(m_pStudioHdr))
		return NULL;
	return m_pStudioHdr->pSeqdesc( i );
}

inline const mstudioseqgroup_t *CStudioHdr::pSeqGroup( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numseqgroups)
		return NULL;
	return m_pStudioHdr->pSeqgroup( i );
}

inline int CStudioHdr::GetNumIncludeModels() const
{
	if (!m_pStudioHdr || !IsV44Plus())
		return 0;
	return StudioHdr_GetNumIncludeModels( m_pStudioHdr );
}

inline const mstudiomodelgroup_t *CStudioHdr::pModelGroup( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumIncludeModels(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetModelGroup( m_pStudioHdr, i );
}

inline const mstudioattachment_t *CStudioHdr::pAttachment( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumAttachments(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetAttachment( m_pStudioHdr, i );
}

inline const mstudiobodyparts_t *CStudioHdr::pBodypart( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumBodyparts(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetBodypart( m_pStudioHdr, i );
}

inline const mstudiotexture_t *CStudioHdr::pTexture( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumTextures(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetTexture( m_pStudioHdr, i );
}

inline const char *CStudioHdr::pCdtexture( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumCdTextures(m_pStudioHdr))
		return "";
	return StudioHdr_GetCdTexture( m_pStudioHdr, i );
}

inline const short *CStudioHdr::pSkinref( int i ) const
{
	return StudioHdr_GetSkinRef( m_pStudioHdr, i );
}

inline const mstudioflexdesc_t *CStudioHdr::pFlexdesc( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumFlexDescs(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetFlexDesc( m_pStudioHdr, i );
}

inline const mstudioflexcontroller_t *CStudioHdr::pFlexcontroller( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumFlexControllers(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetFlexController( m_pStudioHdr, i );
}

inline const mstudioflexrule_t *CStudioHdr::pFlexRule( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumFlexRules(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetFlexRule( m_pStudioHdr, i );
}

inline int CStudioHdr::numflexcontrollerui() const
{
	return StudioHdr_GetNumFlexControllerUI( m_pStudioHdr );
}

inline const mstudioflexcontrollerui_t *CStudioHdr::pFlexControllerUI( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumFlexControllerUI(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetFlexControllerUI( m_pStudioHdr, i );
}

inline const mstudioikchain_t *CStudioHdr::pIKChain( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumIKChains(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetIKChain( m_pStudioHdr, i );
}

inline const mstudioiklock_t *CStudioHdr::pIKAutoplayLock( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumIKAutoplayLocks(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetIKAutoplayLock( m_pStudioHdr, i );
}

inline const mstudiomouth_t *CStudioHdr::pMouth( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumMouths(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetMouth( m_pStudioHdr, i );
}

inline const mstudioposeparamdesc_t *CStudioHdr::pPoseParameter( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= StudioHdr_GetNumPoseParameters(m_pStudioHdr))
		return NULL;
	return StudioHdr_GetPoseParameter( m_pStudioHdr, i );
}

inline void *CStudioHdr::GetVirtualModel() const
{
	if (!m_pStudioHdr)
		return NULL;
	if (IsV44Plus())
	{
		const studiohdr_v44_t *pHdr44 = (const studiohdr_v44_t*)m_pStudioHdr;
		if (!pHdr44->virtualModel)
			return Studio_CreateVirtualModel((studiohdr_t*)m_pStudioHdr);
		return pHdr44->virtualModel;
	}
	return m_pStudioHdr->virtualModel;
}

inline void CStudioHdr::SetVirtualModel( void *pVirtualModel )
{
	if (!m_pStudioHdr)
		return;
	if (IsV44Plus())
	{
		((studiohdr_v44_t*)m_pStudioHdr)->virtualModel = pVirtualModel;
	}
	else
	{
		m_pStudioHdr->virtualModel = (virtualmodel_t*)pVirtualModel;
	}
}

inline const studiohdr2_t *CStudioHdr::pStudioHdr2() const
{
	return StudioHdr_GetStudioHdr2( m_pStudioHdr );
}

inline int CStudioHdr::GetRootLOD() const
{
	return StudioHdr_GetRootLOD( m_pStudioHdr );
}

inline int CStudioHdr::GetNumAllowedRootLODs() const
{
	return StudioHdr_GetNumAllowedRootLODs( m_pStudioHdr );
}

inline const byte *CStudioHdr::GetBoneTableSortedByName() const
{
	return StudioHdr_GetBoneTableSortedByName( m_pStudioHdr );
}

#endif // CSTUDIOHDR_H
