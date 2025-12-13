//-----------------------------------------------------------------------------
// Studio Header v44+ (studiohdr_v44_t)
// This structure matches the exact binary layout of Source 2004+ (v44-v48) MDL files
// Use this struct when loading v44+ models, cast back to base for common operations
//
// IMPORTANT: This header must be included AFTER studio.h since it depends on
// forward declarations and other structures defined there.
//-----------------------------------------------------------------------------

#ifndef STUDIOHDR_V44_H
#define STUDIOHDR_V44_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// mstudioseqdesc_v44_t - Sequence descriptor for v44+ models
// This structure has a baseptr field at offset 0 that v37 doesn't have.
// When accessing v44 sequence data through mstudioseqdesc_t*, we skip baseptr
// so the field offsets align properly with our v37-based struct definition.
//-----------------------------------------------------------------------------
struct mstudioseqdesc_v44_t
{
	int					baseptr;		// v44+: Offset back to studiohdr_t
	inline studiohdr_t	*pStudiohdr( void ) const { return (studiohdr_t *)(((byte *)this) + baseptr); }

	// Remaining fields match mstudioseqdesc_t layout
	int					szlabelindex;
	inline char * const pszLabel( void ) const { return ((char *)this) + szlabelindex; }

	int					szactivitynameindex;
	inline char * const pszActivityName( void ) const { return ((char *)this) + szactivitynameindex; }

	int					flags;
	int					activity;
	int					actweight;
	int					numevents;
	int					eventindex;
	inline mstudioevent_t *pEvent( int i ) const { return (mstudioevent_t *)(((byte *)this) + eventindex) + i; };

	Vector				bbmin;
	Vector				bbmax;

	int					numblends;
	int					animindexindex;
	inline short * const pBlends( void ) const { return (short *)(((byte *)this) + animindexindex); }

	int					movementindex;
	int					groupsize[2];
	int					paramindex[2];
	float				paramstart[2];
	float				paramend[2];
	int					paramparent;

	float				fadeintime;
	float				fadeouttime;

	int					localentrynode;  // v44+ uses localentrynode/localexitnode
	int					localexitnode;
	int					nodeflags;

	float				entryphase;
	float				exitphase;
	float				lastframe;

	int					nextseq;
	int					pose;
	int					numikrules;
	int					numautolayers;
	int					autolayerindex;
	inline mstudioautolayer_t *pAutolayer( int i ) const { return (mstudioautolayer_t *)(((byte *)this) + autolayerindex) + i; };

	int					weightlistindex;
	inline float		*pBoneweight( int i ) const {
		if (weightlistindex == 0) return nullptr;
		return ((float *)(((byte *)this) + weightlistindex) + i);
	};
	inline float		weight( int i ) const {
		float *pWeight = pBoneweight( i );
		return pWeight ? *pWeight : 0.0f;
	};

	int					posekeyindex;
	float				*pPoseKey( int iParam, int iAnim, int groupsize0 ) const { return (float *)(((byte *)this) + posekeyindex) + iParam * groupsize0 + iAnim; }

	int					numiklocks;
	int					iklockindex;
	inline mstudioiklock_t *pIKLock( int i ) const { return (mstudioiklock_t *)(((byte *)this) + iklockindex) + i; };

	int					keyvalueindex;
	int					keyvaluesize;
	inline const char * KeyValueText( void ) const { return keyvaluesize != 0 ? ((char *)this) + keyvalueindex : NULL; }

	int					cycleposeindex;
	int					unused[7];
};

struct studiohdr_v44_t
{
	int					id;
	int					version;
	long				checksum;
	char				name[64];
	int					length;

	Vector				eyeposition;
	Vector				illumposition;
	Vector				hull_min;
	Vector				hull_max;
	Vector				view_bbmin;
	Vector				view_bbmax;

	int					flags;

	int					numbones;
	int					boneindex;
	inline mstudiobone_v48_t *pBone( int i ) const { return (mstudiobone_v48_t *)(((byte *)this) + boneindex) + i; };

	int					numbonecontrollers;
	int					bonecontrollerindex;
	inline mstudiobonecontroller_t *pBonecontroller( int i ) const { return (mstudiobonecontroller_t *)(((byte *)this) + bonecontrollerindex) + i; };

	int					numhitboxsets;
	int					hitboxsetindex;
	mstudiohitboxset_t	*pHitboxSet( int i ) const { return (mstudiohitboxset_t *)(((byte *)this) + hitboxsetindex ) + i; };
	inline mstudiobbox_t *pHitbox( int i, int set ) const {
		mstudiohitboxset_t const *s = pHitboxSet( set );
		return s ? s->pHitbox( i ) : NULL;
	};
	inline int iHitboxCount( int set ) const {
		mstudiohitboxset_t const *s = pHitboxSet( set );
		return s ? s->numhitboxes : 0;
	};

	// v44+: Local animations (numlocalseq follows directly after)
	int					numlocalanim;
	int					localanimindex;
	inline mstudioanimdesc_v48_t *pLocalAnimdesc( int i ) const { return (mstudioanimdesc_v48_t *)(((byte *)this) + localanimindex) + i; };
	inline mstudioanimdesc_v48_t *pAnimdesc( int i ) const { return pLocalAnimdesc(i); };

	// v44+: Local sequences (no animgroups, no bonedescs, no seqgroups between anim and seq!)
	// NOTE: v44 seqdesc has baseptr field at offset 0 that v37 doesn't have.
	// We skip past baseptr (+4 bytes) so mstudioseqdesc_t field offsets align correctly.
	int					numlocalseq;
	int					localseqindex;
	inline mstudioseqdesc_t *pLocalSeqdesc( int i ) const {
		if (i < 0 || i >= numlocalseq) i = 0;
		// v44 seqdesc stride = sizeof(mstudioseqdesc_t) + 4 (for baseptr)
		// Skip past baseptr (+4) so mstudioseqdesc_t fields align
		int v44_stride = sizeof(mstudioseqdesc_t) + 4;
		byte* pBase = ((byte *)this) + localseqindex;
		return (mstudioseqdesc_t *)(pBase + i * v44_stride + 4);
	};
	inline mstudioseqdesc_t *pSeqdesc( int i ) const { return pLocalSeqdesc(i); };

	// Access the full v44 seqdesc structure including baseptr
	inline mstudioseqdesc_v44_t *pLocalSeqdesc_V44( int i ) const {
		if (i < 0 || i >= numlocalseq) i = 0;
		// v44 seqdesc uses mstudioseqdesc_v44_t which has baseptr at offset 0
		return (mstudioseqdesc_v44_t *)(((byte *)this) + localseqindex) + i;
	};

	// v44+: Activity/event indexing (replaces v37's animgroups)
	mutable int			activitylistversion;
	mutable int			eventsindexed;

	// Textures (directly after activity/events in v44+)
	// NOTE: v44+ models use mstudiotexture_t (64 bytes) with unused[10] padding
	// v37 models use mstudiotexture_v37_t (32 bytes) with dPdu/dPdv fields
	int					numtextures;
	int					textureindex;
	inline mstudiotexture_t *pTexture( int i ) const { return (mstudiotexture_t *)(((byte *)this) + textureindex) + i; };

	int					numcdtextures;
	int					cdtextureindex;
	inline char *pCdtexture( int i ) const { return (((char *)this) + *((int *)(((byte *)this) + cdtextureindex) + i)); };

	int					numskinref;
	int					numskinfamilies;
	int					skinindex;
	inline short *pSkinref( int i ) const { return (short *)(((byte *)this) + skinindex) + i; };

	int					numbodyparts;
	int					bodypartindex;
	inline mstudiobodyparts_t *pBodypart( int i ) const { return (mstudiobodyparts_t *)(((byte *)this) + bodypartindex) + i; };

	int					numlocalattachments;
	int					localattachmentindex;
	inline mstudioattachment_v48_t *pLocalAttachment( int i ) const { return (mstudioattachment_v48_t *)(((byte *)this) + localattachmentindex) + i; };

	// v44+: Local nodes with name index
	int					numlocalnodes;
	int					localnodeindex;
	int					localnodenameindex;
	inline char *pszLocalNodeName( int iNode ) const { return (((char *)this) + *((int *)(((byte *)this) + localnodenameindex) + iNode)); }
	inline byte *pLocalTransition( int i ) const { return (byte *)(((byte *)this) + localnodeindex) + i; };

	int					numflexdesc;
	int					flexdescindex;
	inline mstudioflexdesc_t *pFlexdesc( int i ) const { return (mstudioflexdesc_t *)(((byte *)this) + flexdescindex) + i; };

	int					numflexcontrollers;
	int					flexcontrollerindex;
	inline mstudioflexcontroller_t *pFlexcontroller( int i ) const { return (mstudioflexcontroller_t *)(((byte *)this) + flexcontrollerindex) + i; };

	int					numflexrules;
	int					flexruleindex;
	inline mstudioflexrule_t *pFlexRule( int i ) const { return (mstudioflexrule_t *)(((byte *)this) + flexruleindex) + i; };

	int					numikchains;
	int					ikchainindex;
	inline mstudioikchain_t *pIKChain( int i ) const { return (mstudioikchain_t *)(((byte *)this) + ikchainindex) + i; };

	int					nummouths;
	int					mouthindex;
	inline mstudiomouth_t *pMouth( int i ) const { return (mstudiomouth_t *)(((byte *)this) + mouthindex) + i; };

	int					numlocalposeparameters;
	int					localposeparamindex;
	inline mstudioposeparamdesc_t *pLocalPoseParameter( int i ) const { return (mstudioposeparamdesc_t *)(((byte *)this) + localposeparamindex) + i; };

	int					surfacepropindex;
	inline char * const pszSurfaceProp( void ) const { return ((char *)this) + surfacepropindex; }

	int					keyvalueindex;
	int					keyvaluesize;
	inline const char * KeyValueText( void ) const { return keyvaluesize != 0 ? ((char *)this) + keyvalueindex : NULL; }

	int					numlocalikautoplaylocks;
	int					localikautoplaylockindex;
	inline mstudioiklock_t *pLocalIKAutoplayLock( int i ) const { return (mstudioiklock_t *)(((byte *)this) + localikautoplaylockindex) + i; };

	float				mass;
	int					contents;

	// v44+: Include models for compositing
	int					numincludemodels;
	int					includemodelindex;
	inline mstudiomodelgroup_t *pModelGroup( int i ) const { return (mstudiomodelgroup_t *)(((byte *)this) + includemodelindex) + i; };

	mutable void		*virtualModel;

	// v44+: Animation block name for demand loading
	int					szanimblocknameindex;
	inline char * const pszAnimBlockName( void ) const { return ((char *)this) + szanimblocknameindex; }

	int					numanimblocks;
	int					animblockindex;
	inline mstudioanimblock_t *pAnimBlock( int i ) const { return (mstudioanimblock_t *)(((byte *)this) + animblockindex) + i; };
	mutable void		*animblockModel;

	int					bonetablebynameindex;
	inline const byte *GetBoneTableSortedByName() const { return (byte *)this + bonetablebynameindex; }

	mutable void		*pVertexBase;
	mutable void		*pIndexBase;

	byte				constdirectionallightdot;
	byte				rootLOD;
	byte				numAllowedRootLODs;
	byte				unused0;
	int					unused1;

	int					numflexcontrollerui;
	int					flexcontrolleruiindex;

	int					unused3[2];

	int					studiohdr2index;
	inline studiohdr2_t *pStudioHdr2() const {
		return (studiohdr2index != 0) ? (studiohdr2_t *)(((byte *)this) + studiohdr2index) : NULL;
	}

	int					unused2;

	// Helper functions
	inline int GetVersion() const { return version; }
	inline bool IsV44Plus() const { return version >= STUDIO_VERSION_44; }
	inline bool IsV48() const { return version >= STUDIO_VERSION_48; }
	inline int numseq() const { return numlocalseq; }
	inline int numanim() const { return numlocalanim; }
	inline int numattachments() const { return numlocalattachments; }
	inline int numposeparameters() const { return numlocalposeparameters; }
	inline int numikautoplaylocks() const { return numlocalikautoplaylocks; }
};

//-----------------------------------------------------------------------------
// Helper to cast raw header to correct version struct
// Usage: studiohdr_v44_t* pHdr44 = StudioHdr_GetV44(pStudioHdr);
//-----------------------------------------------------------------------------
inline studiohdr_v44_t* StudioHdr_GetV44(void* pHdr)
{
	if (!pHdr) return NULL;
	int version = *(int*)((byte*)pHdr + 4); // version is at offset 4
	return (version >= STUDIO_VERSION_44) ? (studiohdr_v44_t*)pHdr : NULL;
}

inline const studiohdr_v44_t* StudioHdr_GetV44(const void* pHdr)
{
	if (!pHdr) return NULL;
	int version = *(int*)((byte*)pHdr + 4);
	return (version >= STUDIO_VERSION_44) ? (const studiohdr_v44_t*)pHdr : NULL;
}

//-----------------------------------------------------------------------------
// Version-safe accessor functions for studiohdr_t fields that are at different
// offsets between v37 and v44+
// These can be used throughout the codebase without explicit version checks
//-----------------------------------------------------------------------------

// Get numbodyparts - safe for both v37 and v44+
inline int StudioHdr_GetNumBodyparts(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numbodyparts;
	return pHdr->numbodyparts;
}

// Get bodypart - safe for both v37 and v44+
inline mstudiobodyparts_t* StudioHdr_GetBodypart(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((studiohdr_v44_t*)pHdr)->pBodypart(i);
	return pHdr->pBodypart(i);
}

// Get numtextures - safe for both v37 and v44+
inline int StudioHdr_GetNumTextures(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numtextures;
	return pHdr->numtextures;
}

// Get texture - safe for both v37 and v44+
// NOTE: v37 models use mstudiotexture_v37_t (32 bytes) with dPdu/dPdv fields
// v44+ models use mstudiotexture_t (64 bytes) with used/unused[10] fields
// The texture structure format changed at v44!
inline mstudiotexture_t* StudioHdr_GetTexture(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
	{
		// v44+ uses 64-byte mstudiotexture_t at v44 header offsets
		return ((studiohdr_v44_t*)pHdr)->pTexture(i);
	}
	// v37 uses 32-byte mstudiotexture_v37_t at v37 header offsets
	return (mstudiotexture_t*)pHdr->pTexture_V37(i);
}

// Get numskinref - safe for both v37 and v44+
inline int StudioHdr_GetNumSkinRef(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numskinref;
	return pHdr->numskinref;
}

// Get numskinfamilies - safe for both v37 and v44+
inline int StudioHdr_GetNumSkinFamilies(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numskinfamilies;
	return pHdr->numskinfamilies;
}

// Get skinref - safe for both v37 and v44+
inline short* StudioHdr_GetSkinRef(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((studiohdr_v44_t*)pHdr)->pSkinref(i);
	return pHdr->pSkinref(i);
}

// Get numattachments - safe for both v37 and v44+
inline int StudioHdr_GetNumAttachments(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numlocalattachments;
	return pHdr->numattachments;
}

// Get attachment - safe for both v37 and v44+
// Note: v44+ attachments use mstudioattachment_v48_t but we cast to mstudioattachment_t for compatibility
// The first 60 bytes are layout-compatible (sznameindex, type/flags, bone/localbone, local matrix)
inline mstudioattachment_t* StudioHdr_GetAttachment(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return (mstudioattachment_t*)((studiohdr_v44_t*)pHdr)->pLocalAttachment(i);
	return pHdr->pAttachment(i);
}

// Get numcdtextures - safe for both v37 and v44+
inline int StudioHdr_GetNumCdTextures(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numcdtextures;
	return pHdr->numcdtextures;
}

// Get cdtexture - safe for both v37 and v44+
inline char* StudioHdr_GetCdTexture(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((studiohdr_v44_t*)pHdr)->pCdtexture(i);
	return pHdr->pCdtexture(i);
}

// Get numincludemodels - safe for both v37 and v44+
// v37 models don't have include models, so always return 0 for them
inline int StudioHdr_GetNumIncludeModels(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numincludemodels;
	return 0;  // v37 doesn't support include models
}

// Get virtualModel pointer - safe for both v37 and v44+
// v37 models don't have virtual models, so always return NULL for them
inline virtualmodel_t* StudioHdr_GetVirtualModel(const studiohdr_t* pHdr)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return (virtualmodel_t*)((const studiohdr_v44_t*)pHdr)->virtualModel;
	return NULL;  // v37 doesn't support virtual models
}

// Set virtualModel pointer - safe for both v37 and v44+
inline void StudioHdr_SetVirtualModel(studiohdr_t* pHdr, virtualmodel_t* pVModel)
{
	if (!pHdr) return;
	if (pHdr->version >= STUDIO_VERSION_44)
		((studiohdr_v44_t*)pHdr)->virtualModel = pVModel;
	// v37 doesn't support virtual models - silently ignore
}

//-----------------------------------------------------------------------------
// Implementation of studiohdr_t::GetVirtualModel() and SetVirtualModel()
// These must be defined AFTER studiohdr_v44_t is available
//-----------------------------------------------------------------------------
inline virtualmodel_t *studiohdr_t::GetVirtualModel( void ) const
{
	// v37 models don't support virtual models
	if (version < STUDIO_VERSION_44)
		return NULL;

	// Cast to v44 structure to access correct field offsets
	const studiohdr_v44_t *pHdr44 = (const studiohdr_v44_t *)this;

	// Create virtual model on-demand if not yet created
	if (!pHdr44->virtualModel && pHdr44->numincludemodels > 0)
	{
		// Note: Studio_CreateVirtualModel casts away const to set virtualModel
		Studio_CreateVirtualModel(const_cast<studiohdr_t*>(this));
	}
	return (virtualmodel_t *)pHdr44->virtualModel;
}

inline void studiohdr_t::SetVirtualModel( virtualmodel_t *pVModel ) const
{
	// v37 models don't support virtual models
	if (version < STUDIO_VERSION_44)
		return;

	// Cast to v44 structure to access correct field offset
	studiohdr_v44_t *pHdr44 = (studiohdr_v44_t *)this;
	pHdr44->virtualModel = pVModel;
}

//-----------------------------------------------------------------------------
// Pose Parameter helpers - version-aware access
//-----------------------------------------------------------------------------

// Get number of pose parameters - safe for both v37 and v44+
inline int StudioHdr_GetNumPoseParameters(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numlocalposeparameters;
	return pHdr->numposeparameters;
}

// Get pose parameter descriptor - safe for both v37 and v44+
inline mstudioposeparamdesc_t* StudioHdr_GetPoseParameter(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((studiohdr_v44_t*)pHdr)->pLocalPoseParameter(i);
	return pHdr->pPoseParameter(i);
}

//-----------------------------------------------------------------------------
// Flex descriptor helpers - version-aware access
//-----------------------------------------------------------------------------

// Get number of flex descriptors - safe for both v37 and v44+
inline int StudioHdr_GetNumFlexDesc(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numflexdesc;
	return pHdr->numflexdesc;
}

// Get flex descriptor - safe for both v37 and v44+
inline mstudioflexdesc_t* StudioHdr_GetFlexDesc(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((studiohdr_v44_t*)pHdr)->pFlexdesc(i);
	return pHdr->pFlexdesc(i);
}

//-----------------------------------------------------------------------------
// Flex controller helpers - version-aware access
//-----------------------------------------------------------------------------

// Get number of flex controllers - safe for both v37 and v44+
inline int StudioHdr_GetNumFlexControllers(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numflexcontrollers;
	return pHdr->numflexcontrollers;
}

// Get flex controller - safe for both v37 and v44+
inline mstudioflexcontroller_t* StudioHdr_GetFlexController(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((studiohdr_v44_t*)pHdr)->pFlexcontroller(i);
	return pHdr->pFlexcontroller(i);
}

//-----------------------------------------------------------------------------
// Mouth helpers - version-aware access
//-----------------------------------------------------------------------------

// Get number of mouths - safe for both v37 and v44+
inline int StudioHdr_GetNumMouths(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->nummouths;
	return pHdr->nummouths;
}

// Get mouth - safe for both v37 and v44+
inline mstudiomouth_t* StudioHdr_GetMouth(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((studiohdr_v44_t*)pHdr)->pMouth(i);
	return pHdr->pMouth(i);
}

//-----------------------------------------------------------------------------
// IK Chain helpers - version-aware access
//-----------------------------------------------------------------------------

// Get number of IK chains - safe for both v37 and v44+
inline int StudioHdr_GetNumIKChains(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numikchains;
	return pHdr->numikchains;
}

// Get IK chain - safe for both v37 and v44+
inline mstudioikchain_t* StudioHdr_GetIKChain(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((studiohdr_v44_t*)pHdr)->pIKChain(i);
	return pHdr->pIKChain(i);
}

//-----------------------------------------------------------------------------
// IK Autoplay Lock helpers - version-aware access
//-----------------------------------------------------------------------------

// Get number of IK autoplay locks - safe for both v37 and v44+
inline int StudioHdr_GetNumIKAutoplayLocks(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numlocalikautoplaylocks;
	return pHdr->numikautoplaylocks;
}

// Get IK autoplay lock - safe for both v37 and v44+
inline mstudioiklock_t* StudioHdr_GetIKAutoplayLock(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((studiohdr_v44_t*)pHdr)->pLocalIKAutoplayLock(i);
	return pHdr->pIKAutoplayLock(i);
}

//-----------------------------------------------------------------------------
// Flex rule helpers - version-aware access
//-----------------------------------------------------------------------------

// Get number of flex rules - safe for both v37 and v44+
inline int StudioHdr_GetNumFlexRules(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numflexrules;
	return pHdr->numflexrules;
}

// Get flex rule - safe for both v37 and v44+
inline mstudioflexrule_t* StudioHdr_GetFlexRule(const studiohdr_t* pHdr, int i)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((studiohdr_v44_t*)pHdr)->pFlexRule(i);
	return pHdr->pFlexRule(i);
}

//-----------------------------------------------------------------------------
// Surface prop helpers - version-aware access
//-----------------------------------------------------------------------------

// Get surface prop string - safe for both v37 and v44+
inline const char* StudioHdr_GetSurfaceProp(const studiohdr_t* pHdr)
{
	if (!pHdr) return "";
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->pszSurfaceProp();
	return pHdr->pszSurfaceProp();
}

//-----------------------------------------------------------------------------
// KeyValue helpers - version-aware access
//-----------------------------------------------------------------------------

// Get keyvalue text - safe for both v37 and v44+
inline const char* StudioHdr_GetKeyValueText(const studiohdr_t* pHdr)
{
	if (!pHdr) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->KeyValueText();
	return pHdr->KeyValueText();
}

//-----------------------------------------------------------------------------
// Bone helpers - version-aware access
// CRITICAL: v37 bones are 192 bytes, v48 bones are 216 bytes!
// Using array indexing on the wrong type will read garbage memory.
//-----------------------------------------------------------------------------

// Get number of bones - safe for both v37 and v44+
inline int StudioHdr_GetNumBones(const studiohdr_t* pHdr)
{
	if (!pHdr) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const studiohdr_v44_t*)pHdr)->numbones;
	return pHdr->numbones;
}

// Get bone parent index - version-safe (uses correct bone stride)
inline int StudioBone_GetParent(const studiohdr_t* pHdr, int iBone)
{
	if (!pHdr || iBone < 0) return -1;
	if (pHdr->version >= STUDIO_VERSION_44)
	{
		const studiohdr_v44_t* pHdr44 = (const studiohdr_v44_t*)pHdr;
		if (iBone >= pHdr44->numbones) return -1;
		const mstudiobone_v48_t* pBone = pHdr44->pBone(iBone);
		return pBone ? pBone->parent : -1;
	}
	if (iBone >= pHdr->numbones) return -1;
	const mstudiobone_t* pBone = pHdr->pBone(iBone);
	return pBone ? pBone->parent : -1;
}

// Get bone flags - version-safe (uses correct bone stride and field offset)
inline int StudioBone_GetFlags(const studiohdr_t* pHdr, int iBone)
{
	if (!pHdr || iBone < 0) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
	{
		const studiohdr_v44_t* pHdr44 = (const studiohdr_v44_t*)pHdr;
		if (iBone >= pHdr44->numbones) return 0;
		const mstudiobone_v48_t* pBone = pHdr44->pBone(iBone);
		return pBone ? pBone->flags : 0;
	}
	if (iBone >= pHdr->numbones) return 0;
	const mstudiobone_t* pBone = pHdr->pBone(iBone);
	return pBone ? pBone->flags : 0;
}

// Get pointer to bone poseToBone matrix - version-safe (for direct read access)
inline const matrix3x4_t* StudioBone_GetPoseToBonePtr(const studiohdr_t* pHdr, int iBone)
{
	if (!pHdr || iBone < 0) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
	{
		const studiohdr_v44_t* pHdr44 = (const studiohdr_v44_t*)pHdr;
		if (iBone >= pHdr44->numbones) return NULL;
		const mstudiobone_v48_t* pBone = pHdr44->pBone(iBone);
		return pBone ? &pBone->poseToBone : NULL;
	}
	if (iBone >= pHdr->numbones) return NULL;
	const mstudiobone_t* pBone = pHdr->pBone(iBone);
	return pBone ? &pBone->poseToBone : NULL;
}

// Get bone contents - version-safe (uses correct bone stride)
inline int StudioBone_GetContents(const studiohdr_t* pHdr, int iBone)
{
	if (!pHdr || iBone < 0) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
	{
		const studiohdr_v44_t* pHdr44 = (const studiohdr_v44_t*)pHdr;
		if (iBone >= pHdr44->numbones) return 0;
		const mstudiobone_v48_t* pBone = pHdr44->pBone(iBone);
		return pBone ? pBone->contents : 0;
	}
	if (iBone >= pHdr->numbones) return 0;
	const mstudiobone_t* pBone = pHdr->pBone(iBone);
	return pBone ? pBone->contents : 0;
}

// Get bone physicsbone index - version-safe (uses correct bone stride)
inline int StudioBone_GetPhysicsBone(const studiohdr_t* pHdr, int iBone)
{
	if (!pHdr || iBone < 0) return -1;
	if (pHdr->version >= STUDIO_VERSION_44)
	{
		const studiohdr_v44_t* pHdr44 = (const studiohdr_v44_t*)pHdr;
		if (iBone >= pHdr44->numbones) return -1;
		const mstudiobone_v48_t* pBone = pHdr44->pBone(iBone);
		return pBone ? pBone->physicsbone : -1;
	}
	if (iBone >= pHdr->numbones) return -1;
	const mstudiobone_t* pBone = pHdr->pBone(iBone);
	return pBone ? pBone->physicsbone : -1;
}

// Get bone name - version-safe (uses correct bone stride)
inline const char* StudioBone_GetName(const studiohdr_t* pHdr, int iBone)
{
	if (!pHdr || iBone < 0) return "";
	if (pHdr->version >= STUDIO_VERSION_44)
	{
		const studiohdr_v44_t* pHdr44 = (const studiohdr_v44_t*)pHdr;
		if (iBone >= pHdr44->numbones) return "";
		const mstudiobone_v48_t* pBone = pHdr44->pBone(iBone);
		return pBone ? pBone->pszName() : "";
	}
	if (iBone >= pHdr->numbones) return "";
	const mstudiobone_t* pBone = pHdr->pBone(iBone);
	return pBone ? pBone->pszName() : "";
}

// Get bone proctype - version-safe (uses correct bone stride)
inline int StudioBone_GetProcType(const studiohdr_t* pHdr, int iBone)
{
	if (!pHdr || iBone < 0) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
	{
		const studiohdr_v44_t* pHdr44 = (const studiohdr_v44_t*)pHdr;
		if (iBone >= pHdr44->numbones) return 0;
		const mstudiobone_v48_t* pBone = pHdr44->pBone(iBone);
		return pBone ? pBone->proctype : 0;
	}
	if (iBone >= pHdr->numbones) return 0;
	const mstudiobone_t* pBone = pHdr->pBone(iBone);
	return pBone ? pBone->proctype : 0;
}

//-----------------------------------------------------------------------------
// Sequence descriptor weight accessor - version-safe (by index)
// CRITICAL: v37 and v44+ have different seqdesc layouts (v44+ has baseptr field)
// This function handles the offset differences properly.
//-----------------------------------------------------------------------------
inline float StudioSeqdesc_GetWeight(const studiohdr_t* pHdr, int iSeq, int iBone)
{
	if (!pHdr || iSeq < 0 || iBone < 0) return 0.0f;

	if (pHdr->version >= STUDIO_VERSION_44)
	{
		// Use v48 seqdesc struct which has correct baseptr field
		const mstudioseqdesc_v48_t* pSeq = pHdr->pSeqdesc_v48(iSeq);
		if (!pSeq) return 0.0f;
		return pSeq->weight(iBone);
	}

	// v37: Use standard seqdesc
	const mstudioseqdesc_t* pSeq = pHdr->pSeqdesc(iSeq);
	if (!pSeq) return 0.0f;
	return pSeq->weight(iBone);
}

//-----------------------------------------------------------------------------
// Sequence descriptor weight accessor - version-safe (by pointer)
// For use in bone_setup.cpp where we have pseqdesc pointer but need version-aware access.
// For v44+, pSeqdesc() returns a pointer that's +4 from actual struct start (skips baseptr).
// This function backs up 4 bytes to get the real v48 seqdesc and uses its weight().
//-----------------------------------------------------------------------------
inline float StudioSeqdesc_GetWeightFromPtr(const studiohdr_t* pHdr, const mstudioseqdesc_t* pseqdesc, int iBone)
{
	if (!pHdr || !pseqdesc || iBone < 0) return 0.0f;

	if (pHdr->version >= STUDIO_VERSION_44)
	{
		// pSeqdesc() for v44+ returns (actual_start + 4) to skip baseptr
		// Back up 4 bytes to get the real v48 seqdesc with baseptr at offset 0
		const mstudioseqdesc_v48_t* pSeq_v48 = (const mstudioseqdesc_v48_t*)(((const byte*)pseqdesc) - 4);
		return pSeq_v48->weight(iBone);
	}

	// v37: Use standard seqdesc directly
	return pseqdesc->weight(iBone);
}

// Get seqdesc flags - version-safe
inline int StudioSeqdesc_GetFlags(const studiohdr_t* pHdr, int iSeq)
{
	if (!pHdr || iSeq < 0) return 0;

	if (pHdr->version >= STUDIO_VERSION_44)
	{
		const mstudioseqdesc_v48_t* pSeq = pHdr->pSeqdesc_v48(iSeq);
		if (!pSeq) return 0;
		return pSeq->flags;
	}

	const mstudioseqdesc_t* pSeq = pHdr->pSeqdesc(iSeq);
	if (!pSeq) return 0;
	return pSeq->flags;
}

//-----------------------------------------------------------------------------
// Model/Mesh struct helpers - version-aware access
// These are critical because mesh struct sizes differ between v37 and v44+
// v37: mstudiomesh_t is 68 bytes
// v44+: mstudiomesh_v44_t is ~116 bytes (has embedded vertexdata)
//-----------------------------------------------------------------------------

// Get number of meshes from model - safe for both v37 and v44+
inline int StudioModel_GetNumMeshes(const studiohdr_t* pHdr, const mstudiomodel_t* pModel)
{
	if (!pHdr || !pModel) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const mstudiomodel_v44_t*)pModel)->nummeshes;
	return pModel->nummeshes;
}

// Get mesh from model - safe for both v37 and v44+
// Returns mstudiomesh_t* for compatibility, but it's actually mstudiomesh_v44_t* for v44+
inline mstudiomesh_t* StudioModel_GetMesh(const studiohdr_t* pHdr, const mstudiomodel_t* pModel, int i)
{
	if (!pHdr || !pModel) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return (mstudiomesh_t*)((const mstudiomodel_v44_t*)pModel)->pMesh(i);
	return pModel->pMesh(i);
}

// Get number of eyeballs from model - safe for both v37 and v44+
inline int StudioModel_GetNumEyeballs(const studiohdr_t* pHdr, const mstudiomodel_t* pModel)
{
	if (!pHdr || !pModel) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const mstudiomodel_v44_t*)pModel)->numeyeballs;
	return pModel->numeyeballs;
}

// Get eyeball from model - safe for both v37 and v44+
inline mstudioeyeball_t* StudioModel_GetEyeball(const studiohdr_t* pHdr, mstudiomodel_t* pModel, int i)
{
	if (!pHdr || !pModel) return NULL;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((mstudiomodel_v44_t*)pModel)->pEyeball(i);
	return pModel->pEyeball(i);
}

// Get mesh material index - safe for both v37 and v44+
inline int StudioMesh_GetMaterial(const studiohdr_t* pHdr, const mstudiomesh_t* pMesh)
{
	if (!pHdr || !pMesh) return 0;
	// material field is at same offset in both v37 and v44+ mesh structs
	return pMesh->material;
}

// Get mesh meshid - safe for both v37 and v44+
inline int StudioMesh_GetMeshId(const studiohdr_t* pHdr, const mstudiomesh_t* pMesh)
{
	if (!pHdr || !pMesh) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const mstudiomesh_v44_t*)pMesh)->meshid;
	return pMesh->meshid;
}

// Get mesh materialtype - safe for both v37 and v44+
inline int StudioMesh_GetMaterialType(const studiohdr_t* pHdr, const mstudiomesh_t* pMesh)
{
	if (!pHdr || !pMesh) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const mstudiomesh_v44_t*)pMesh)->materialtype;
	return pMesh->materialtype;
}

// Get mesh materialparam - safe for both v37 and v44+
inline int StudioMesh_GetMaterialParam(const studiohdr_t* pHdr, const mstudiomesh_t* pMesh)
{
	if (!pHdr || !pMesh) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const mstudiomesh_v44_t*)pMesh)->materialparam;
	return pMesh->materialparam;
}

// Get mesh numvertices - safe for both v37 and v44+
inline int StudioMesh_GetNumVertices(const studiohdr_t* pHdr, const mstudiomesh_t* pMesh)
{
	if (!pHdr || !pMesh) return 0;
	if (pHdr->version >= STUDIO_VERSION_44)
		return ((const mstudiomesh_v44_t*)pMesh)->numvertices;
	return pMesh->numvertices;
}

//-----------------------------------------------------------------------------
// 2007-style vertex data access - missing functions that 2007 engine uses
// These are needed for proper v44+ model rendering compatibility
//-----------------------------------------------------------------------------

// Forward declaration for vertex file header
struct vertexFileHeader_t;

// CacheVertexData function - needs to be implemented by application-specific code
// This should be defined in the engine's model loader
extern const vertexFileHeader_t* CacheVertexData(void* pModelData);

// Get vertex data for rendering - simplified approach that works with LeakNet's structure
// Returns true if vertex data is available, false otherwise
inline bool SetupVertexDataForMesh(mstudiomesh_t* pMesh, const studiohdr_t* pStudioHdr, mstudiovertex_t** ppVertices, Vector4D** ppTangentS)
{
	if (!pMesh || !pStudioHdr || !ppVertices) return false;

	*ppVertices = NULL;
	if (ppTangentS) *ppTangentS = NULL;

	// For v37 models, use the mesh's built-in accessor functions
	if (pStudioHdr->IsV37())
	{
		*ppVertices = pMesh->Vertex(0);
		if (ppTangentS) *ppTangentS = pMesh->TangentS(0);

		DevMsg("v37 vertex data: %s, vertices: %p\n", pStudioHdr->name, *ppVertices);
		return (*ppVertices != NULL);
	}

	// For v44+ models, we need to ensure the vertex data is properly set up
	if (pStudioHdr->version >= STUDIO_VERSION_44)
	{
		// Get the v44+ model
		const mstudiomodel_v44_t* pModel44 = (const mstudiomodel_v44_t*)pMesh->pModel();

		if (!pModel44)
		{
			DevWarning("SetupVertexDataForMesh: NULL pModel44 for %s\n", pStudioHdr->name);
			return false;
		}

		// Check if the model has VVD vertex data loaded
		if (!pModel44->vertexdata.pVertexData)
		{
			DevWarning("SetupVertexDataForMesh: v44+ model %s has no VVD vertex data loaded\n", pStudioHdr->name);
			return false;
		}

		// Calculate vertex pointers based on mesh offset into model's VVD data
		*ppVertices = (mstudiovertex_t*)pModel44->vertexdata.pVertexData + pMesh->vertexoffset;
		if (ppTangentS && pModel44->vertexdata.pTangentData)
		{
			*ppTangentS = (Vector4D*)pModel44->vertexdata.pTangentData + pMesh->vertexoffset;
		}

		DevMsg("v44+ vertex data: %s, vertices: %p, tangents: %p, offset: %d\n",
			pStudioHdr->name, *ppVertices, ppTangentS ? *ppTangentS : NULL, pMesh->vertexoffset);
		return (*ppVertices != NULL);
	}

	return false;
}

#endif // STUDIOHDR_V44_H
