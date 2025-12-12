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
	int numbones() const { return m_pStudioHdr ? m_pStudioHdr->numbones : 0; }
	const mstudiobone_t *pBone( int i ) const;
	int RemapBone( int iBone ) const;
	int GetNumBoneControllers() const { return m_pStudioHdr ? m_pStudioHdr->numbonecontrollers : 0; }
	const mstudiobonecontroller_t *pBonecontroller( int i ) const;

	// v48: Linear bone data access
	const mstudiolinearbone_t *pLinearBones() const;

	//-----------------------------------------------------------------------------
	// Hitboxes
	//-----------------------------------------------------------------------------
	int numhitboxsets() const { return m_pStudioHdr ? m_pStudioHdr->numhitboxsets : 0; }
	const mstudiohitboxset_t *pHitboxSet( int i ) const;
	const mstudiobbox_t *pHitbox( int i, int set ) const;
	int iHitboxCount( int set ) const;

	//-----------------------------------------------------------------------------
	// Animations (unified access for both v37 and v48)
	//-----------------------------------------------------------------------------
	int GetNumLocalAnims() const { return m_pStudioHdr ? m_pStudioHdr->numanim : 0; }
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
	int GetNumLocalSeq() const { return m_pStudioHdr ? m_pStudioHdr->numseq : 0; }
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
	int GetNumAttachments() const { return m_pStudioHdr ? m_pStudioHdr->numattachments : 0; }
	const mstudioattachment_t *pAttachment( int i ) const;

	//-----------------------------------------------------------------------------
	// Body parts
	//-----------------------------------------------------------------------------
	int numbodyparts() const { return m_pStudioHdr ? m_pStudioHdr->numbodyparts : 0; }
	const mstudiobodyparts_t *pBodypart( int i ) const;

	//-----------------------------------------------------------------------------
	// Textures/Materials
	//-----------------------------------------------------------------------------
	int numtextures() const { return m_pStudioHdr ? m_pStudioHdr->numtextures : 0; }
	const mstudiotexture_t *pTexture( int i ) const;
	int numcdtextures() const { return m_pStudioHdr ? m_pStudioHdr->numcdtextures : 0; }
	const char *pCdtexture( int i ) const;
	int numskinref() const { return m_pStudioHdr ? m_pStudioHdr->numskinref : 0; }
	int numskinfamilies() const { return m_pStudioHdr ? m_pStudioHdr->numskinfamilies : 0; }
	const short *pSkinref( int i ) const;

	//-----------------------------------------------------------------------------
	// Flex
	//-----------------------------------------------------------------------------
	int numflexdesc() const { return m_pStudioHdr ? m_pStudioHdr->numflexdesc : 0; }
	const mstudioflexdesc_t *pFlexdesc( int i ) const;
	int numflexcontrollers() const { return m_pStudioHdr ? m_pStudioHdr->numflexcontrollers : 0; }
	const mstudioflexcontroller_t *pFlexcontroller( int i ) const;
	int numflexrules() const { return m_pStudioHdr ? m_pStudioHdr->numflexrules : 0; }
	const mstudioflexrule_t *pFlexRule( int i ) const;

	// v48: Flex controller UI
	int numflexcontrollerui() const;
	const mstudioflexcontrollerui_t *pFlexControllerUI( int i ) const;

	//-----------------------------------------------------------------------------
	// IK
	//-----------------------------------------------------------------------------
	int numikchains() const { return m_pStudioHdr ? m_pStudioHdr->numikchains : 0; }
	const mstudioikchain_t *pIKChain( int i ) const;
	int numikautoplaylocks() const { return m_pStudioHdr ? m_pStudioHdr->numikautoplaylocks : 0; }
	const mstudioiklock_t *pIKAutoplayLock( int i ) const;

	//-----------------------------------------------------------------------------
	// Mouths
	//-----------------------------------------------------------------------------
	int nummouths() const { return m_pStudioHdr ? m_pStudioHdr->nummouths : 0; }
	const mstudiomouth_t *pMouth( int i ) const;

	//-----------------------------------------------------------------------------
	// Pose parameters
	//-----------------------------------------------------------------------------
	int numposeparameters() const { return m_pStudioHdr ? m_pStudioHdr->numposeparameters : 0; }
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
	float mass() const { return m_pStudioHdr ? m_pStudioHdr->mass : 0.0f; }
	int contents() const { return m_pStudioHdr ? m_pStudioHdr->contents : 0; }

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

private:
	const studiohdr_t	*m_pStudioHdr;
	int					m_nVersion;
};

//-----------------------------------------------------------------------------
// Inline implementations
//-----------------------------------------------------------------------------

inline CStudioHdr::CStudioHdr()
{
	m_pStudioHdr = NULL;
	m_nVersion = 0;
}

inline CStudioHdr::CStudioHdr( const studiohdr_t *pStudioHdr )
{
	Init( pStudioHdr );
}

inline void CStudioHdr::Init( const studiohdr_t *pStudioHdr )
{
	m_pStudioHdr = pStudioHdr;
	m_nVersion = pStudioHdr ? pStudioHdr->version : 0;
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
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numbones)
		return NULL;
	return m_pStudioHdr->pBone( i );
}

inline int CStudioHdr::RemapBone( int iBone ) const
{
	// Default implementation - no remapping
	return iBone;
}

inline const mstudiobonecontroller_t *CStudioHdr::pBonecontroller( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numbonecontrollers)
		return NULL;
	return m_pStudioHdr->pBonecontroller( i );
}

inline const mstudiolinearbone_t *CStudioHdr::pLinearBones() const
{
	return m_pStudioHdr ? m_pStudioHdr->pLinearBones() : NULL;
}

inline const mstudiohitboxset_t *CStudioHdr::pHitboxSet( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numhitboxsets)
		return NULL;
	return m_pStudioHdr->pHitboxSet( i );
}

inline const mstudiobbox_t *CStudioHdr::pHitbox( int i, int set ) const
{
	return m_pStudioHdr ? m_pStudioHdr->pHitbox( i, set ) : NULL;
}

inline int CStudioHdr::iHitboxCount( int set ) const
{
	return m_pStudioHdr ? m_pStudioHdr->iHitboxCount( set ) : 0;
}

inline const mstudioanimdesc_t *CStudioHdr::pLocalAnimdesc( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numanim)
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
	return m_pStudioHdr->numanimblocks;
}

inline const mstudioanimblock_t *CStudioHdr::pAnimBlock( int i ) const
{
	return m_pStudioHdr ? m_pStudioHdr->pAnimBlock( i ) : NULL;
}

inline const mstudioseqdesc_t *CStudioHdr::pLocalSeqdesc( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numseq)
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
	return m_pStudioHdr->numincludemodels;
}

inline const mstudiomodelgroup_t *CStudioHdr::pModelGroup( int i ) const
{
	return m_pStudioHdr ? m_pStudioHdr->pModelGroup( i ) : NULL;
}

inline const mstudioattachment_t *CStudioHdr::pAttachment( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numattachments)
		return NULL;
	return m_pStudioHdr->pAttachment( i );
}

inline const mstudiobodyparts_t *CStudioHdr::pBodypart( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numbodyparts)
		return NULL;
	return m_pStudioHdr->pBodypart( i );
}

inline const mstudiotexture_t *CStudioHdr::pTexture( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numtextures)
		return NULL;
	return m_pStudioHdr->pTexture( i );
}

inline const char *CStudioHdr::pCdtexture( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numcdtextures)
		return "";
	return m_pStudioHdr->pCdtexture( i );
}

inline const short *CStudioHdr::pSkinref( int i ) const
{
	return m_pStudioHdr ? m_pStudioHdr->pSkinref( i ) : NULL;
}

inline const mstudioflexdesc_t *CStudioHdr::pFlexdesc( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numflexdesc)
		return NULL;
	return m_pStudioHdr->pFlexdesc( i );
}

inline const mstudioflexcontroller_t *CStudioHdr::pFlexcontroller( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numflexcontrollers)
		return NULL;
	return m_pStudioHdr->pFlexcontroller( i );
}

inline const mstudioflexrule_t *CStudioHdr::pFlexRule( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numflexrules)
		return NULL;
	return m_pStudioHdr->pFlexRule( i );
}

inline int CStudioHdr::numflexcontrollerui() const
{
	if (!m_pStudioHdr || !IsV44Plus())
		return 0;
	return m_pStudioHdr->numflexcontrollerui;
}

inline const mstudioflexcontrollerui_t *CStudioHdr::pFlexControllerUI( int i ) const
{
	return m_pStudioHdr ? m_pStudioHdr->pFlexControllerUI( i ) : NULL;
}

inline const mstudioikchain_t *CStudioHdr::pIKChain( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numikchains)
		return NULL;
	return m_pStudioHdr->pIKChain( i );
}

inline const mstudioiklock_t *CStudioHdr::pIKAutoplayLock( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numikautoplaylocks)
		return NULL;
	return m_pStudioHdr->pIKAutoplayLock( i );
}

inline const mstudiomouth_t *CStudioHdr::pMouth( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->nummouths)
		return NULL;
	return m_pStudioHdr->pMouth( i );
}

inline const mstudioposeparamdesc_t *CStudioHdr::pPoseParameter( int i ) const
{
	if (!m_pStudioHdr || i < 0 || i >= m_pStudioHdr->numposeparameters)
		return NULL;
	return m_pStudioHdr->pPoseParameter( i );
}

inline void *CStudioHdr::GetVirtualModel() const
{
	return m_pStudioHdr ? m_pStudioHdr->GetVirtualModel() : NULL;
}

inline void CStudioHdr::SetVirtualModel( void *pVirtualModel )
{
	if (m_pStudioHdr && IsV44Plus())
	{
		m_pStudioHdr->virtualModel = pVirtualModel;
	}
}

inline const studiohdr2_t *CStudioHdr::pStudioHdr2() const
{
	return m_pStudioHdr ? m_pStudioHdr->pStudioHdr2() : NULL;
}

inline int CStudioHdr::GetRootLOD() const
{
	if (!m_pStudioHdr || !IsV44Plus())
		return 0;
	return m_pStudioHdr->rootLOD;
}

inline int CStudioHdr::GetNumAllowedRootLODs() const
{
	if (!m_pStudioHdr || !IsV44Plus())
		return 0;
	return m_pStudioHdr->numAllowedRootLODs;
}

inline const byte *CStudioHdr::GetBoneTableSortedByName() const
{
	if (!m_pStudioHdr || !IsV44Plus())
		return NULL;
	return m_pStudioHdr->GetBoneTableSortedByName();
}

#endif // CSTUDIOHDR_H
