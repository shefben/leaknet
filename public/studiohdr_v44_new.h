//-----------------------------------------------------------------------------
// Studio Header v44+ (studiohdr_v44_t)
// COMPLETELY SELF-CONTAINED - NO DEPENDENCIES ON V37 CODE
//
// This file defines ALL v44+ model structures from scratch.
// NEVER include studio.h or any v37 headers.
// All types use _v44 suffix to distinguish from v37 types.
//
// Based on Source Engine 2007 (v48) model format
//-----------------------------------------------------------------------------

#ifndef STUDIOHDR_V44_H
#define STUDIOHDR_V44_H

#ifdef _WIN32
#pragma once
#endif

// Foundation headers only - NO v37 model headers
#include "mathlib/mathlib.h"
#include "utlvector.h"

//-----------------------------------------------------------------------------
// v44+ Constants - completely separate from v37
//-----------------------------------------------------------------------------
#define MAXSTUDIOBONES_V44			128		// total bones actually used
#define MAXSTUDIOTRIANGLES_V44		65536
#define MAXSTUDIOVERTS_V44			65536
#define MAXSTUDIOFLEXVERTS_V44		10000
#define MAXSTUDIOFLEXDESC_V44		1024
#define MAXSTUDIOFLEXCTRL_V44		96
#define MAXSTUDIOPOSEPARAM_V44		24
#define MAXSTUDIOBONECTRLS_V44		4
#define MAXSTUDIOANIMBLOCKS_V44		256
#define MAXSTUDIOSKINS_V44			32
#define MAXSTUDIOBONEBITS_V44		7

#define MAX_NUM_BONES_PER_VERT_V44	3
#define MAX_NUM_LODS_V44			8

// v44+ model version constants
#define STUDIO_VERSION_V44			44
#define STUDIO_VERSION_V45			45
#define STUDIO_VERSION_V46			46
#define STUDIO_VERSION_V47			47
#define STUDIO_VERSION_V48			48

// Compatibility for code that uses MAXSTUDIOBONES without suffix
#ifndef MAXSTUDIOBONES
#define MAXSTUDIOBONES MAXSTUDIOBONES_V44
#endif

//-----------------------------------------------------------------------------
// v44+ Procedural bone types
//-----------------------------------------------------------------------------
#define STUDIO_PROC_AXISINTERP_V44	1
#define STUDIO_PROC_QUATINTERP_V44	2
#define STUDIO_PROC_AIMATBONE_V44	3
#define STUDIO_PROC_AIMATATTACH_V44	4
#define STUDIO_PROC_JIGGLE_V44		5

//-----------------------------------------------------------------------------
// v44+ Bone flags
//-----------------------------------------------------------------------------
#define BONE_CALCULATE_MASK_V44				0x1F
#define BONE_PHYSICALLY_SIMULATED_V44		0x01
#define BONE_PHYSICS_PROCEDURAL_V44			0x02
#define BONE_ALWAYS_PROCEDURAL_V44			0x04
#define BONE_SCREEN_ALIGN_SPHERE_V44		0x08
#define BONE_SCREEN_ALIGN_CYLINDER_V44		0x10

#define BONE_USED_MASK_V44					0x0007FF00
#define BONE_USED_BY_ANYTHING_V44			0x0007FF00
#define BONE_USED_BY_HITBOX_V44				0x00000100
#define BONE_USED_BY_ATTACHMENT_V44			0x00000200
#define BONE_USED_BY_VERTEX_MASK_V44		0x0003FC00
#define BONE_USED_BY_VERTEX_LOD0_V44		0x00000400
#define BONE_USED_BY_VERTEX_LOD1_V44		0x00000800
#define BONE_USED_BY_VERTEX_LOD2_V44		0x00001000
#define BONE_USED_BY_VERTEX_LOD3_V44		0x00002000
#define BONE_USED_BY_VERTEX_LOD4_V44		0x00004000
#define BONE_USED_BY_VERTEX_LOD5_V44		0x00008000
#define BONE_USED_BY_VERTEX_LOD6_V44		0x00010000
#define BONE_USED_BY_VERTEX_LOD7_V44		0x00020000
#define BONE_USED_BY_BONE_MERGE_V44			0x00040000

#define BONE_TYPE_MASK_V44					0x00F00000
#define BONE_FIXED_ALIGNMENT_V44			0x00100000
#define BONE_HAS_SAVEFRAME_POS_V44			0x00200000
#define BONE_HAS_SAVEFRAME_ROT_V44			0x00400000

//-----------------------------------------------------------------------------
// v44+ Jiggle bone flags
//-----------------------------------------------------------------------------
#define JIGGLE_IS_FLEXIBLE_V44				0x01
#define JIGGLE_IS_RIGID_V44					0x02
#define JIGGLE_HAS_YAW_CONSTRAINT_V44		0x04
#define JIGGLE_HAS_PITCH_CONSTRAINT_V44		0x08
#define JIGGLE_HAS_ANGLE_CONSTRAINT_V44		0x10
#define JIGGLE_HAS_LENGTH_CONSTRAINT_V44	0x20
#define JIGGLE_HAS_BASE_SPRING_V44			0x40

// Compatibility aliases (only for v44+ code files)
#define JIGGLE_IS_FLEXIBLE				JIGGLE_IS_FLEXIBLE_V44
#define JIGGLE_IS_RIGID					JIGGLE_IS_RIGID_V44
#define JIGGLE_HAS_YAW_CONSTRAINT		JIGGLE_HAS_YAW_CONSTRAINT_V44
#define JIGGLE_HAS_PITCH_CONSTRAINT		JIGGLE_HAS_PITCH_CONSTRAINT_V44
#define JIGGLE_HAS_ANGLE_CONSTRAINT		JIGGLE_HAS_ANGLE_CONSTRAINT_V44
#define JIGGLE_HAS_LENGTH_CONSTRAINT	JIGGLE_HAS_LENGTH_CONSTRAINT_V44
#define JIGGLE_HAS_BASE_SPRING			JIGGLE_HAS_BASE_SPRING_V44

//-----------------------------------------------------------------------------
// v44+ Sequence flags
//-----------------------------------------------------------------------------
#define STUDIO_LOOPING_V44		0x0001
#define STUDIO_SNAP_V44			0x0002
#define STUDIO_DELTA_V44		0x0004
#define STUDIO_AUTOPLAY_V44		0x0008
#define STUDIO_POST_V44			0x0010
#define STUDIO_ALLZEROS_V44		0x0020
#define STUDIO_CYCLEPOSE_V44	0x0080
#define STUDIO_REALTIME_V44		0x0100
#define STUDIO_LOCAL_V44		0x0200
#define STUDIO_HIDDEN_V44		0x0400
#define STUDIO_OVERRIDE_V44		0x0800
#define STUDIO_ACTIVITY_V44		0x1000
#define STUDIO_EVENT_V44		0x2000
#define STUDIO_WORLD_V44		0x4000

//-----------------------------------------------------------------------------
// Forward declarations - v44+ types only
//-----------------------------------------------------------------------------
struct studiohdr_v44_t;
struct mstudiobone_v44_t;
struct mstudiobonecontroller_v44_t;
struct mstudiohitboxset_v44_t;
struct mstudiobbox_v44_t;
struct mstudioseqdesc_v44_t;
struct mstudioanimdesc_v44_t;
struct mstudioanim_v44_t;
struct mstudioikchain_v44_t;
struct mstudioiklock_v44_t;
struct mstudiomesh_v44_t;
struct mstudiomodel_v44_t;
struct mstudiobodyparts_v44_t;
struct mstudiotexture_v44_t;
struct mstudioattachment_v44_t;
struct mstudioposeparamdesc_v44_t;
struct mstudioflexdesc_v44_t;
struct mstudioflexcontroller_v44_t;
struct mstudiomouth_v44_t;
struct mstudioevent_v44_t;
struct mstudiojigglebone_v44_t;
struct mstudioaimatbone_v44_t;
struct virtualmodel_v44_t;

//-----------------------------------------------------------------------------
// mstudiojigglebone_v44_t - Jiggle bone procedural structure
//-----------------------------------------------------------------------------
struct mstudiojigglebone_v44_t
{
	int				flags;

	// general params
	float			length;					// how far from bone base, along bone, is tip
	float			tipMass;

	// flexible params
	float			yawStiffness;
	float			yawDamping;
	float			pitchStiffness;
	float			pitchDamping;
	float			alongStiffness;
	float			alongDamping;

	// angle constraint
	float			angleLimit;				// maximum deflection of tip in radians

	// yaw constraint
	float			minYaw;					// in radians
	float			maxYaw;					// in radians
	float			yawFriction;
	float			yawBounce;

	// pitch constraint
	float			minPitch;				// in radians
	float			maxPitch;				// in radians
	float			pitchFriction;
	float			pitchBounce;

	// base spring
	float			baseMass;
	float			baseStiffness;
	float			baseDamping;
	float			baseMinLeft;
	float			baseMaxLeft;
	float			baseLeftFriction;
	float			baseMinUp;
	float			baseMaxUp;
	float			baseUpFriction;
	float			baseMinForward;
	float			baseMaxForward;
	float			baseForwardFriction;
};

// Compatibility typedef for jigglebones code
typedef mstudiojigglebone_v44_t mstudiojigglebone_t;

//-----------------------------------------------------------------------------
// mstudioaimatbone_v44_t - Aim-at bone procedural structure
//-----------------------------------------------------------------------------
struct mstudioaimatbone_v44_t
{
	int				parent;
	int				aim;		// Might be bone or attachment index
	Vector			aimvector;
	Vector			upvector;
	Vector			basepos;
};

//-----------------------------------------------------------------------------
// mstudiobone_v44_t - Bone structure for v44+ models
//-----------------------------------------------------------------------------
struct mstudiobone_v44_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int		 			parent;		// parent bone
	int					bonecontroller[6];	// bone controller index, -1 == none

	// default values
	Vector				pos;
	Quaternion			quat;
	RadianEuler			rot;
	// compression scale
	Vector				posscale;
	Vector				rotscale;

	matrix3x4_t			poseToBone;
	Quaternion			qAlignment;
	int					flags;
	int					proctype;
	int					procindex;		// procedural rule
	mutable int			physicsbone;	// index into physically simulated bone
	inline void *pProcedure( ) const { if (procindex == 0) return NULL; else return (void *)(((byte *)this) + procindex); };
	int					surfacepropidx;	// index into string table for property name
	inline char * const pszSurfaceProp( void ) const { return ((char *)this) + surfacepropidx; }
	int					contents;		// See BSPFlags.h for the contents flags

	int					unused[8];		// remove as appropriate
};

//-----------------------------------------------------------------------------
// mstudiobonecontroller_v44_t - Bone controller structure
//-----------------------------------------------------------------------------
struct mstudiobonecontroller_v44_t
{
	int					bone;	// -1 == 0
	int					type;	// X, Y, Z, XR, YR, ZR, M
	float				start;
	float				end;
	int					rest;	// byte index value at rest
	int					inputfield;	// 0-3 user set controller, 4 mouth
	int					unused[8];
};

//-----------------------------------------------------------------------------
// mstudiobbox_v44_t - Hitbox structure
//-----------------------------------------------------------------------------
struct mstudiobbox_v44_t
{
	int					bone;
	int					group;				// intersection group
	Vector				bbmin;				// bounding box
	Vector				bbmax;
	int					szhitboxnameindex;	// offset to the name of the hitbox
	int					unused[8];

	const char* pszHitboxName() const
	{
		if( szhitboxnameindex == 0 )
			return "";
		return ((const char*)this) + szhitboxnameindex;
	}
};

//-----------------------------------------------------------------------------
// mstudiohitboxset_v44_t - Hitbox set structure
//-----------------------------------------------------------------------------
struct mstudiohitboxset_v44_t
{
	int					sznameindex;
	inline char * const	pszName( void ) const { return ((char *)this) + sznameindex; }
	int					numhitboxes;
	int					hitboxindex;
	inline mstudiobbox_v44_t *pHitbox( int i ) const { return (mstudiobbox_v44_t *)(((byte *)this) + hitboxindex) + i; };
};

//-----------------------------------------------------------------------------
// mstudioevent_v44_t - Animation event structure
//-----------------------------------------------------------------------------
struct mstudioevent_v44_t
{
	float				cycle;
	int					event;
	int					type;
	inline const char * pszOptions( void ) const { return options; }
	char				options[64];

	int					szeventindex;
	inline char * const pszEventName( void ) const { return ((char *)this) + szeventindex; }
};

//-----------------------------------------------------------------------------
// mstudioattachment_v44_t - Attachment point structure
//-----------------------------------------------------------------------------
struct mstudioattachment_v44_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	unsigned int		flags;
	int					localbone;
	matrix3x4_t			local; // attachment point
	int					unused[8];
};

//-----------------------------------------------------------------------------
// mstudiotexture_v44_t - Texture/material reference
//-----------------------------------------------------------------------------
struct mstudiotexture_v44_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int					flags;
	int					used;
	int					unused1;
	mutable void		*material;  // IMaterial
	mutable void		*clientmaterial;	// void*
	int					unused[10];
};

//-----------------------------------------------------------------------------
// mstudioposeparamdesc_v44_t - Pose parameter descriptor
//-----------------------------------------------------------------------------
struct mstudioposeparamdesc_v44_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int					flags;	// ????
	float				start;	// starting value
	float				end;	// ending value
	float				loop;	// looping range, 0 for no looping, 360 for rotations, etc.
};

//-----------------------------------------------------------------------------
// mstudioflexdesc_v44_t - Flex descriptor
//-----------------------------------------------------------------------------
struct mstudioflexdesc_v44_t
{
	int					szFACSindex;
	inline char * const pszFACS( void ) const { return ((char *)this) + szFACSindex; }
};

//-----------------------------------------------------------------------------
// mstudioflexcontroller_v44_t - Flex controller
//-----------------------------------------------------------------------------
struct mstudioflexcontroller_v44_t
{
	int					sztypeindex;
	inline char * const pszType( void ) const { return ((char *)this) + sztypeindex; }
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	mutable int			localToGlobal;	// remapped at load time to master list
	float				min;
	float				max;
};

//-----------------------------------------------------------------------------
// mstudiomouth_v44_t - Mouth structure for lipsync
//-----------------------------------------------------------------------------
struct mstudiomouth_v44_t
{
	int					bone;
	Vector				forward;
	int					flexdesc;
};

//-----------------------------------------------------------------------------
// mstudioikchain_v44_t - IK chain structure
//-----------------------------------------------------------------------------
struct mstudioikchain_v44_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int					linktype;
	int					numlinks;
	int					linkindex;
	// inline mstudioiklink_v44_t *pLink( int i ) const;
};

//-----------------------------------------------------------------------------
// mstudioiklock_v44_t - IK lock structure
//-----------------------------------------------------------------------------
struct mstudioiklock_v44_t
{
	int					chain;
	float				flPosWeight;
	float				flLocalQWeight;
	int					flags;
	int					unused[4];
};

//-----------------------------------------------------------------------------
// mstudioboneweight_v44_t - Bone weight structure for v44+ models
//-----------------------------------------------------------------------------
struct mstudioboneweight_v44_t
{
	float	weight[MAX_NUM_BONES_PER_VERT_V44];
	char	bone[MAX_NUM_BONES_PER_VERT_V44];
	byte	numbones;
};

//-----------------------------------------------------------------------------
// mstudiovertex_v44_t - Vertex structure for v44+ models
//-----------------------------------------------------------------------------
struct mstudiovertex_v44_t
{
	mstudioboneweight_v44_t	m_BoneWeights;
	Vector				m_vecPosition;
	Vector				m_vecNormal;
	Vector2D			m_vecTexCoord;
};

//-----------------------------------------------------------------------------
// mstudiomesh_v44_t - Mesh structure for v44+ models
//-----------------------------------------------------------------------------
struct mstudiomesh_v44_t
{
	int					material;

	int					modelindex;
	mstudiomodel_v44_t *pModel() const;

	int					numvertices;		// number of unique vertices/normals/texcoords
	int					vertexoffset;		// vertex mstudiovertex_v44_t

	int					numflexes;			// vertex animation
	int					flexindex;

	// special codes for material operations
	int					materialtype;
	int					materialparam;

	// a]l unique mesh identifier
	int					meshid;

	Vector				center;

	// Embedded vertex data for v44+ models
	struct
	{
		int				numLODVertexes[MAX_NUM_LODS_V44];
	} vertexdata;

	int					unused[8]; // remove as appropriate
};

//-----------------------------------------------------------------------------
// mstudiomodel_v44_t - Model structure for v44+ models
//-----------------------------------------------------------------------------
struct mstudiomodel_v44_t
{
	inline const char * pszName( void ) const { return name; }
	char				name[64];

	int					type;

	float				boundingradius;

	int					nummeshes;
	int					meshindex;
	inline mstudiomesh_v44_t *pMesh( int i ) const { return (mstudiomesh_v44_t *)(((byte *)this) + meshindex) + i; };

	// cache purposes
	int					numvertices;		// number of unique vertices/normals/texcoords
	int					vertexindex;		// vertex Vector
	int					tangentsindex;		// tangents Vector

	int					numattachments;
	int					attachmentindex;

	int					numeyeballs;
	int					eyeballindex;

	// Embedded vertex data pointers (set at load time)
	struct
	{
		const void		*pVertexData;
		const void		*pTangentData;
	} vertexdata;

	int					unused[8];		// remove as appropriate
};

//-----------------------------------------------------------------------------
// mstudiobodyparts_v44_t - Bodypart structure
//-----------------------------------------------------------------------------
struct mstudiobodyparts_v44_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int					nummodels;
	int					base;
	int					modelindex; // index into models array
	inline mstudiomodel_v44_t *pModel( int i ) const { return (mstudiomodel_v44_t *)(((byte *)this) + modelindex) + i; };
};

//-----------------------------------------------------------------------------
// mstudioseqdesc_v44_t - Sequence descriptor for v44+ models
// Note: v44+ has baseptr field at offset 0 that v37 doesn't have
//-----------------------------------------------------------------------------
struct mstudioseqdesc_v44_t
{
	int					baseptr;		// v44+: Offset back to studiohdr_v44_t
	inline studiohdr_v44_t *pStudiohdr( void ) const { return (studiohdr_v44_t *)(((byte *)this) + baseptr); }

	int					szlabelindex;
	inline char * const pszLabel( void ) const { return ((char *)this) + szlabelindex; }

	int					szactivitynameindex;
	inline char * const pszActivityName( void ) const { return ((char *)this) + szactivitynameindex; }

	int					flags;
	int					activity;
	int					actweight;
	int					numevents;
	int					eventindex;
	inline mstudioevent_v44_t *pEvent( int i ) const { return (mstudioevent_v44_t *)(((byte *)this) + eventindex) + i; };

	Vector				bbmin;
	Vector				bbmax;

	int					numblends;
	int					animindexindex;

	int					movementindex;

	int					groupsize[2];
	int					paramindex[2];
	float				paramstart[2];
	float				paramend[2];
	int					paramparent;

	float				fadeintime;
	float				fadeouttime;

	int					localentrynode;
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

	int					weightlistindex;
	inline float		*pBoneweight( int i ) const { return ((float *)(((byte *)this) + weightlistindex) + i); };
	inline float		weight( int i ) const { return *(pBoneweight( i )); };

	int					posekeyindex;

	int					numiklocks;
	int					iklockindex;
	inline mstudioiklock_v44_t *pIKLock( int i ) const { return (mstudioiklock_v44_t *)(((byte *)this) + iklockindex) + i; };

	int					keyvalueindex;
	int					keyvaluesize;
	inline const char * KeyValueText( void ) const { return keyvaluesize != 0 ? ((char *)this) + keyvalueindex : NULL; }

	int					cycleposeindex;

	int					unused[7];
};

// Compatibility typedef
typedef mstudioseqdesc_v44_t mstudioseqdesc_v48_t;

//-----------------------------------------------------------------------------
// mstudioanimdesc_v44_t - Animation descriptor for v44+ models
//-----------------------------------------------------------------------------
struct mstudioanimdesc_v44_t
{
	int					baseptr;
	inline studiohdr_v44_t *pStudiohdr( void ) const { return (studiohdr_v44_t *)(((byte *)this) + baseptr); }

	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }

	float				fps;
	int					flags;

	int					numframes;

	int					nummovements;
	int					movementindex;

	int					unused1[6];

	int					animblock;
	int					animindex;

	int					numikrules;
	int					ikruleindex;
	int					animblockikruleindex;

	int					numlocalhierarchy;
	int					localhierarchyindex;

	int					sectionindex;
	int					sectionframes;

	short				zeroframespan;
	short				zeroframecount;
	int					zeroframeindex;
	float				zeroframestalltime;
};

// Compatibility typedef
typedef mstudioanimdesc_v44_t mstudioanimdesc_v48_t;

//-----------------------------------------------------------------------------
// studiohdr2_v44_t - Extended header for v47+ models
//-----------------------------------------------------------------------------
struct studiohdr2_v44_t
{
	int					numsrcbonetransform;
	int					srcbonetransformindex;

	int					illumpositionattachmentindex;
	inline int			IllumPositionAttachmentIndex() const { return illumpositionattachmentindex; }

	float				flMaxEyeDeflection;
	inline float		MaxEyeDeflection() const { return flMaxEyeDeflection != 0.0f ? flMaxEyeDeflection : 0.866f; }

	int					linearboneindex;
	// inline mstudiolinearbone_v44_t *pLinearBones() const { return (mstudiolinearbone_v44_t *)(((byte *)this) + linearboneindex); }

	int					unknown[64];
};

//-----------------------------------------------------------------------------
// studiohdr_v44_t - Main model header for v44+ models
// This is a COMPLETELY STANDALONE structure - no v37 dependencies
//-----------------------------------------------------------------------------
struct studiohdr_v44_t
{
	int					id;
	int					version;
	long				checksum;

	inline const char *	pszName( void ) const { return name; }
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
	inline mstudiobone_v44_t *pBone( int i ) const { return (mstudiobone_v44_t *)(((byte *)this) + boneindex) + i; };

	int					numbonecontrollers;
	int					bonecontrollerindex;
	inline mstudiobonecontroller_v44_t *pBonecontroller( int i ) const { return (mstudiobonecontroller_v44_t *)(((byte *)this) + bonecontrollerindex) + i; };

	int					numhitboxsets;
	int					hitboxsetindex;
	inline mstudiohitboxset_v44_t *pHitboxSet( int i ) const { return (mstudiohitboxset_v44_t *)(((byte *)this) + hitboxsetindex) + i; };
	inline mstudiobbox_v44_t *pHitbox( int i, int set ) const
	{
		mstudiohitboxset_v44_t const *s = pHitboxSet( set );
		if ( !s ) return NULL;
		return s->pHitbox( i );
	};

	int					numlocalanim;
	int					localanimindex;
	inline mstudioanimdesc_v44_t *pLocalAnimdesc( int i ) const { return (mstudioanimdesc_v44_t *)(((byte *)this) + localanimindex) + i; };

	int					numlocalseq;
	int					localseqindex;
	inline mstudioseqdesc_v44_t *pLocalSeqdesc( int i ) const { return (mstudioseqdesc_v44_t *)(((byte *)this) + localseqindex) + i; };

	mutable int			activitylistversion;
	mutable int			eventsindexed;

	int					numtextures;
	int					textureindex;
	inline mstudiotexture_v44_t *pTexture( int i ) const { return (mstudiotexture_v44_t *)(((byte *)this) + textureindex) + i; };

	int					numcdtextures;
	int					cdtextureindex;
	inline char *pCdtexture( int i ) const { return (((char *)this) + *((int *)(((byte *)this) + cdtextureindex) + i)); };

	int					numskinref;
	int					numskinfamilies;
	int					skinindex;
	inline short *pSkinref( int i ) const { return (short *)(((byte *)this) + skinindex) + i; };

	int					numbodyparts;
	int					bodypartindex;
	inline mstudiobodyparts_v44_t *pBodypart( int i ) const { return (mstudiobodyparts_v44_t *)(((byte *)this) + bodypartindex) + i; };

	int					numlocalattachments;
	int					localattachmentindex;
	inline mstudioattachment_v44_t *pLocalAttachment( int i ) const { return (mstudioattachment_v44_t *)(((byte *)this) + localattachmentindex) + i; };

	int					numlocalnodes;
	int					localnodeindex;
	int					localnodenameindex;

	int					numflexdesc;
	int					flexdescindex;
	inline mstudioflexdesc_v44_t *pFlexdesc( int i ) const { return (mstudioflexdesc_v44_t *)(((byte *)this) + flexdescindex) + i; };

	int					numflexcontrollers;
	int					flexcontrollerindex;
	inline mstudioflexcontroller_v44_t *pFlexcontroller( int i ) const { return (mstudioflexcontroller_v44_t *)(((byte *)this) + flexcontrollerindex) + i; };

	int					numflexrules;
	int					flexruleindex;

	int					numikchains;
	int					ikchainindex;
	inline mstudioikchain_v44_t *pIKChain( int i ) const { return (mstudioikchain_v44_t *)(((byte *)this) + ikchainindex) + i; };

	int					nummouths;
	int					mouthindex;
	inline mstudiomouth_v44_t *pMouth( int i ) const { return (mstudiomouth_v44_t *)(((byte *)this) + mouthindex) + i; };

	int					numlocalposeparameters;
	int					localposeparamindex;
	inline mstudioposeparamdesc_v44_t *pLocalPoseParameter( int i ) const { return (mstudioposeparamdesc_v44_t *)(((byte *)this) + localposeparamindex) + i; };

	int					surfacepropindex;
	inline char * const pszSurfaceProp( void ) const { return ((char *)this) + surfacepropindex; }

	int					keyvalueindex;
	int					keyvaluesize;
	inline const char * KeyValueText( void ) const { return keyvaluesize != 0 ? ((char *)this) + keyvalueindex : NULL; }

	int					numlocalikautoplaylocks;
	int					localikautoplaylockindex;
	inline mstudioiklock_v44_t *pLocalIKAutoplayLock( int i ) const { return (mstudioiklock_v44_t *)(((byte *)this) + localikautoplaylockindex) + i; };

	float				mass;
	int					contents;

	int					numincludemodels;
	int					includemodelindex;

	mutable void		*virtualModel;

	int					szanimblocknameindex;
	inline char * const pszAnimBlockName( void ) const { return ((char *)this) + szanimblocknameindex; }
	int					numanimblocks;
	int					animblockindex;
	mutable void		*animblockModel;

	int					bonetablebynameindex;
	inline const byte *GetBoneTableSortedByName() const { return (byte *)this + bonetablebynameindex; }

	void				*pVertexBase;
	void				*pIndexBase;

	byte				constdirectionallightdot;
	byte				rootLOD;
	byte				numAllowedRootLODs;
	byte				unused;
	int					unused4;

	int					numflexcontrollerui;
	int					flexcontrolleruiindex;

	int					unused3[2];

	int					studiohdr2index;
	inline studiohdr2_v44_t *pStudioHdr2() const { return (studiohdr2_v44_t *)(((byte *)this) + studiohdr2index); }

	int					unused2[1];

	// Helper methods
	inline bool IsV44() const { return version >= STUDIO_VERSION_V44; }
	inline bool IsV48() const { return version >= STUDIO_VERSION_V48; }
};

#endif // STUDIOHDR_V44_H
