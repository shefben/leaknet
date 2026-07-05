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
#include "mathlib.h"  // LeakNet uses public/mathlib.h directly
#include "utlvector.h"
#include "compressed_vector.h"  // For Quaternion48, Quaternion64, Vector48, etc.
#include "vector4d.h"           // For Vector4D

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

// v44+ model file identifiers (same as v37, but defined here for isolation)
#define IDSTUDIOHEADER_V44			(('T'<<24)+('S'<<16)+('D'<<8)+'I')
#define IDSTUDIOANIMBLOCK_V44		(('V'<<24)+('I'<<16)+('N'<<8)+'A')

// v44+ vertex file identifiers
#define MODEL_VERTEX_FILE_ID_V44			(('V'<<24)+('S'<<16)+('D'<<8)+'I')
#define MODEL_VERTEX_FILE_VERSION_V44		4

// Compatibility aliases
#ifndef IDSTUDIOHEADER
#define IDSTUDIOHEADER				IDSTUDIOHEADER_V44
#endif

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

// Compatibility aliases (only if not already defined by studio.h)
#ifndef JIGGLE_IS_FLEXIBLE
#define JIGGLE_IS_FLEXIBLE				JIGGLE_IS_FLEXIBLE_V44
#define JIGGLE_IS_RIGID					JIGGLE_IS_RIGID_V44
#define JIGGLE_HAS_YAW_CONSTRAINT		JIGGLE_HAS_YAW_CONSTRAINT_V44
#define JIGGLE_HAS_PITCH_CONSTRAINT		JIGGLE_HAS_PITCH_CONSTRAINT_V44
#define JIGGLE_HAS_ANGLE_CONSTRAINT		JIGGLE_HAS_ANGLE_CONSTRAINT_V44
#define JIGGLE_HAS_LENGTH_CONSTRAINT	JIGGLE_HAS_LENGTH_CONSTRAINT_V44
#define JIGGLE_HAS_BASE_SPRING			JIGGLE_HAS_BASE_SPRING_V44
#endif

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

//-----------------------------------------------------------------------------
// v44+ Studio header flags
//-----------------------------------------------------------------------------
#define STUDIOHDR_FLAGS_AUTOGENERATED_HITBOX_V44		0x0001
#define STUDIOHDR_FLAGS_USES_ENV_CUBEMAP_V44			0x0002
#define STUDIOHDR_FLAGS_FORCE_OPAQUE_V44				0x0004
#define STUDIOHDR_FLAGS_TRANSLUCENT_TWOPASS_V44			0x0008
#define STUDIOHDR_FLAGS_STATIC_PROP_V44					0x0010
#define STUDIOHDR_FLAGS_USES_FB_TEXTURE_V44				0x0020
#define STUDIOHDR_FLAGS_HASSHADOWLOD_V44				0x0040
#define STUDIOHDR_FLAGS_USES_BUMPMAPPING_V44			0x0080
#define STUDIOHDR_FLAGS_USE_SHADOWLOD_MATERIALS_V44		0x0100
#define STUDIOHDR_FLAGS_OBSOLETE_V44					0x0200
#define STUDIOHDR_FLAGS_UNUSED_V44						0x0400
#define STUDIOHDR_FLAGS_NO_FORCED_FADE_V44				0x0800
#define STUDIOHDR_FLAGS_NO_ANIM_EVENTS_V44				0x1000
#define STUDIOHDR_FLAGS_CAST_TEXTURE_SHADOWS_V44		0x2000
#define STUDIOHDR_FLAGS_FLEXES_CONVERTED_V44			0x200000
#define STUDIO_EVENT_V44		0x2000
#define STUDIO_WORLD_V44		0x4000

// Compatibility alias
#ifndef STUDIOHDR_FLAGS_FLEXES_CONVERTED
#define STUDIOHDR_FLAGS_FLEXES_CONVERTED	STUDIOHDR_FLAGS_FLEXES_CONVERTED_V44
#endif

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

// mstudioanimblock_v44_t - Animation block descriptor for v44+ models
// Defined early because it's used in inline method pAnimBlock()
struct mstudioanimblock_v44_t
{
	int					datastart;
	int					dataend;
};

struct mstudioikchain_v44_t;
struct mstudioiklock_v44_t;
struct mstudioiklink_v44_t;
struct mstudioikrule_v44_t;
struct mstudioikerror_v44_t;
struct mstudiocompressedikerror_v44_t;
struct mstudiolocalhierarchy_v44_t;
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
struct mstudioaxisinterpbone_v44_t;
struct mstudioquatinterpinfo_v44_t;
struct mstudioquatinterpbone_v44_t;
struct mstudiomovement_v44_t;
struct mstudioautolayer_v44_t;
struct mstudioiface_v44_t;
struct mstudio_modelvertexdata_v44_t;
struct mstudio_meshvertexdata_v44_t;
struct mstudioboneweight_v44_t;
struct mstudiovertex_v44_t;
union mstudioanimvalue_v44_t;
struct mstudioanim_valueptr_v44_t;
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

// NOTE: No compatibility typedefs - v44+ code must use explicit _v44 suffix types
// Including both studio.h and studiohdr_v44.h requires using the _v44 types directly

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
// mstudioaxisinterpbone_v44_t - Axis interpolation procedural bone
//-----------------------------------------------------------------------------
struct mstudioaxisinterpbone_v44_t
{
	int				control;	// local transformation of this bone used to calc 3 point blend
	int				axis;		// axis to check
	Vector			pos[6];		// X+, X-, Y+, Y-, Z+, Z-
	Quaternion		quat[6];	// X+, X-, Y+, Y-, Z+, Z-
};

//-----------------------------------------------------------------------------
// mstudioquatinterpinfo_v44_t - Quaternion interpolation info
//-----------------------------------------------------------------------------
struct mstudioquatinterpinfo_v44_t
{
	float			inv_tolerance;	// 1 / radian angle of trigger influence
	Quaternion		trigger;		// angle to match
	Vector			pos;			// new position
	Quaternion		quat;			// new angle
};

//-----------------------------------------------------------------------------
// mstudioquatinterpbone_v44_t - Quaternion interpolation procedural bone
//-----------------------------------------------------------------------------
struct mstudioquatinterpbone_v44_t
{
	int				control;	// local transformation to check
	int				numtriggers;
	int				triggerindex;
	inline mstudioquatinterpinfo_v44_t *pTrigger( int i ) const { return (mstudioquatinterpinfo_v44_t *)(((byte *)this) + triggerindex) + i; };
};

//-----------------------------------------------------------------------------
// Animation value union for v44+ models
//-----------------------------------------------------------------------------
union mstudioanimvalue_v44_t
{
	struct
	{
		byte	valid;
		byte	total;
	} num;
	short		value;
};

//-----------------------------------------------------------------------------
// mstudioanim_valueptr_v44_t - Animation value pointers
//-----------------------------------------------------------------------------
struct mstudioanim_valueptr_v44_t
{
	short	offset[3];
	inline mstudioanimvalue_v44_t *pAnimvalue( int i ) const { if (offset[i] > 0) return (mstudioanimvalue_v44_t *)(((byte *)this) + offset[i]); else return NULL; };
};

// v44+ Animation flags
#define STUDIO_ANIM_RAWPOS_V44		0x01 // Vector48
#define STUDIO_ANIM_RAWROT_V44		0x02 // Quaternion48
#define STUDIO_ANIM_ANIMPOS_V44		0x04 // mstudioanim_valueptr_v44_t
#define STUDIO_ANIM_ANIMROT_V44		0x08 // mstudioanim_valueptr_v44_t
#define STUDIO_ANIM_DELTA_V44		0x10
#define STUDIO_ANIM_RAWROT2_V44		0x20 // Quaternion64

//-----------------------------------------------------------------------------
// mstudioanim_v44_t - Per bone animation data for v44+ models
//-----------------------------------------------------------------------------
struct mstudioanim_v44_t
{
	byte				bone;
	byte				flags;		// weighing options

	// valid for animating data only
	inline byte *pData( void ) const { return (((byte *)this) + sizeof( struct mstudioanim_v44_t )); };
	inline mstudioanim_valueptr_v44_t *pRotV( void ) const { return (mstudioanim_valueptr_v44_t *)(pData()); };
	inline mstudioanim_valueptr_v44_t *pPosV( void ) const { return (mstudioanim_valueptr_v44_t *)(pData()) + ((flags & STUDIO_ANIM_ANIMROT_V44) != 0); };

	// valid if animation unvarying over timeline
	inline Quaternion48 *pQuat48( void ) const { return (Quaternion48 *)(pData()); };
	inline Quaternion64 *pQuat64( void ) const { return (Quaternion64 *)(pData()); };
	inline Vector48 *pPos( void ) const { return (Vector48 *)(pData() + ((flags & STUDIO_ANIM_RAWROT_V44) != 0) * sizeof( *pQuat48() ) + ((flags & STUDIO_ANIM_RAWROT2_V44) != 0) * sizeof( *pQuat64() ) ); };

	short				nextoffset;
	inline mstudioanim_v44_t *pNext( void ) const { if (nextoffset != 0) return (mstudioanim_v44_t *)(((byte *)this) + nextoffset); else return NULL; };
};

//-----------------------------------------------------------------------------
// mstudiomovement_v44_t - Movement data for v44+ models
//-----------------------------------------------------------------------------
struct mstudiomovement_v44_t
{
	int					endframe;
	int					motionflags;
	float				v0;			// velocity at start of block
	float				v1;			// velocity at end of block
	float				angle;		// YAW rotation at end of this blocks movement
	Vector				vector;		// movement vector relative to this blocks initial angle
	Vector				position;	// relative to start of animation
};

//-----------------------------------------------------------------------------
// mstudioikerror_v44_t - IK error data
//-----------------------------------------------------------------------------
struct mstudioikerror_v44_t
{
	Vector		pos;
	Quaternion	q;
};

//-----------------------------------------------------------------------------
// mstudiocompressedikerror_v44_t - Compressed IK error data
//-----------------------------------------------------------------------------
struct mstudiocompressedikerror_v44_t
{
	float	scale[6];
	short	offset[6];
	inline mstudioanimvalue_v44_t *pAnimvalue( int i ) const { if (offset[i] > 0) return (mstudioanimvalue_v44_t *)(((byte *)this) + offset[i]); else return NULL; };
};

// IK rule types
#define IK_SELF_V44			1
#define IK_WORLD_V44		2
#define IK_GROUND_V44		3
#define IK_RELEASE_V44		4
#define IK_ATTACHMENT_V44	5
#define IK_UNLATCH_V44		6

//-----------------------------------------------------------------------------
// mstudioikrule_v44_t - IK rule structure
//-----------------------------------------------------------------------------
struct mstudioikrule_v44_t
{
	int			index;

	int			type;
	int			chain;

	int			bone;

	int			slot;		// iktarget slot. Usually same as chain.
	float		height;
	float		radius;
	float		floor;
	Vector		pos;
	Quaternion	q;

	int			compressedikerrorindex;
	inline mstudiocompressedikerror_v44_t *pCompressedError() const { return (mstudiocompressedikerror_v44_t *)(((byte *)this) + compressedikerrorindex); };
	int			unused2;

	int			iStart;
	int			ikerrorindex;
	inline mstudioikerror_v44_t *pError( int i ) const { return (ikerrorindex) ? (mstudioikerror_v44_t *)(((byte *)this) + ikerrorindex) + (i - iStart) : NULL; };

	float		start;		// beginning of influence
	float		peak;		// start of full influence
	float		tail;		// end of full influence
	float		end;		// end of all influence

	float		unused3;
	float		contact;	// frame footstep makes ground contact
	float		drop;		// how far down the foot should drop when reaching for IK
	float		top;		// top of the foot box

	int			unused6;
	int			unused7;
	int			unused8;

	int			szattachmentindex;		// name of world attachment
	inline char * const pszAttachment( void ) const { return ((char *)this) + szattachmentindex; }

	int			unused[7];
};

//-----------------------------------------------------------------------------
// mstudiolocalhierarchy_v44_t - Local hierarchy for v44+ models
//-----------------------------------------------------------------------------
struct mstudiolocalhierarchy_v44_t
{
	int			iBone;			// bone being adjusted
	int			iNewParent;		// the bones new parent

	float		start;			// beginning of influence
	float		peak;			// start of full influence
	float		tail;			// end of full influence
	float		end;			// end of all influence

	int			iStart;			// first frame

	int			localanimindex;
	inline mstudiocompressedikerror_v44_t *pLocalAnim() const { return (mstudiocompressedikerror_v44_t *)(((byte *)this) + localanimindex); };

	int			unused[4];
};

//-----------------------------------------------------------------------------
// mstudioautolayer_v44_t - Auto layer for v44+ sequences
//-----------------------------------------------------------------------------
struct mstudioautolayer_v44_t
{
	short				iSequence;
	short				iPose;
	int					flags;
	float				start;	// beginning of influence
	float				peak;	// start of full influence
	float				tail;	// end of full influence
	float				end;	// end of all influence
};

//-----------------------------------------------------------------------------
// mstudioiklink_v44_t - IK link for v44+ models
//-----------------------------------------------------------------------------
struct mstudioiklink_v44_t
{
	int		bone;
	Vector	kneeDir;	// ideal bending direction (per link, if applicable)
	Vector	unused0;	// unused
};

//-----------------------------------------------------------------------------
// mstudioiface_v44_t - Triangle face indices
//-----------------------------------------------------------------------------
struct mstudioiface_v44_t
{
	unsigned short a, b, c;		// Indices to vertices
};

//-----------------------------------------------------------------------------
// mstudio_modelvertexdata_v44_t - Model vertex data pointers
//-----------------------------------------------------------------------------
struct mstudiomodel_v44_t;

struct mstudio_modelvertexdata_v44_t
{
	Vector				*Position( int i ) const;
	Vector				*Normal( int i ) const;
	Vector4D			*TangentS( int i ) const;
	Vector2D			*Texcoord( int i ) const;
	mstudioboneweight_v44_t	*BoneWeights( int i ) const;
	mstudiovertex_v44_t	*Vertex( int i ) const;
	bool				HasTangentData( void ) const;
	int					GetGlobalVertexIndex( int i ) const;
	int					GetGlobalTangentIndex( int i ) const;

	// base of external vertex data stores
	const void			*pVertexData;
	const void			*pTangentData;
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
// mstudioeyeball_v44_t - Eyeball structure for v44+ models
//-----------------------------------------------------------------------------
struct mstudioeyeball_v44_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int					bone;
	Vector				org;
	float				zoffset;
	float				radius;
	Vector				up;
	Vector				forward;
	int					texture;

	int					unused1;
	float				iris_scale;
	int					unused2;

	int					upperflexdesc[3];	// index of raiser, neutral, and lowerer flexdesc that is set by the eyeball flex
	int					lowerflexdesc[3];
	float				uppertarget[3];		// angle (radians) of eye in default pose
	float				lowertarget[3];

	int					upperlidflexdesc;	// index of flex desc that drives the upper lid
	int					lowerlidflexdesc;	// index of flex desc that drives the lower lid
	int					unused[4];			// future expansion
	bool				m_bNonFACS;			// if true, eye is non-FACS and should be skipped in eyelid processing
	char				unused3[3];
	int					unused4[7];
};

//-----------------------------------------------------------------------------
// mstudiovertanim_v44_t - Vertex animation for flex v44+ models
//-----------------------------------------------------------------------------
union mstudioanimvalue_v44_t;

struct mstudiovertanim_v44_t
{
	unsigned short		index;
	byte				speed;	// 255 / max_speed
	byte				side;	// 255 / side_delta

protected:
	// JasonM changing this type a lot, to do distance-based compression
	short				flDelta[3];			// Fixed point 8.8
	short				flNDelta[3];		// Fixed point 8.8

public:
	inline void GetDelta( float *fDelta ) const
	{
		fDelta[0] = (float)flDelta[0] / 4096.0f;
		fDelta[1] = (float)flDelta[1] / 4096.0f;
		fDelta[2] = (float)flDelta[2] / 4096.0f;
	}

	inline void GetNDelta( float *fNDelta ) const
	{
		fNDelta[0] = (float)flNDelta[0] / 4096.0f;
		fNDelta[1] = (float)flNDelta[1] / 4096.0f;
		fNDelta[2] = (float)flNDelta[2] / 4096.0f;
	}

	inline Vector GetDeltaFixed() const
	{
		return Vector( (float)flDelta[0] / 4096.0f, (float)flDelta[1] / 4096.0f, (float)flDelta[2] / 4096.0f );
	}

	inline Vector GetNDeltaFixed() const
	{
		return Vector( (float)flNDelta[0] / 4096.0f, (float)flNDelta[1] / 4096.0f, (float)flNDelta[2] / 4096.0f );
	}
};

//-----------------------------------------------------------------------------
// mstudiovertanim_wrinkle_v44_t - Vertex animation with wrinkle for v44+ models
//-----------------------------------------------------------------------------
struct mstudiovertanim_wrinkle_v44_t : public mstudiovertanim_v44_t
{
	short	wrinkledelta;

	inline float GetWrinkleDelta() const
	{
		return (float)wrinkledelta / 4096.0f;
	}
};

//-----------------------------------------------------------------------------
// mstudioflex_v44_t - Flex structure for v44+ models
//-----------------------------------------------------------------------------
struct mstudioflex_v44_t
{
	int					flexdesc;	// input flex
	float				target0;	// zero
	float				target1;	// one
	float				target2;	// one
	float				target3;	// zero
	int					numverts;
	int					vertindex;

	inline byte *pBaseVertanim() const { return (byte *)(((byte *)this) + vertindex); }
	inline mstudiovertanim_v44_t *pVertanim( int i ) const { return (mstudiovertanim_v44_t *)(pBaseVertanim() + i * VertAnimSizeBytes()); }
	inline mstudiovertanim_wrinkle_v44_t *pVertanimWrinkle( int i ) const { return (mstudiovertanim_wrinkle_v44_t *)(pBaseVertanim() + i * VertAnimSizeBytes()); }

	int					flexpair;	// pair flex
	unsigned char		vertanimtype;	// Type of storage for vert anim (0 = standard, 1 = wrinkle)
	unsigned char		unusedchar[3];
	int					unused[6];

	inline int VertAnimSizeBytes() const
	{
		if (vertanimtype == 0)
			return sizeof(mstudiovertanim_v44_t);
		return sizeof(mstudiovertanim_wrinkle_v44_t);
	}
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
	inline mstudioiklink_v44_t *pLink( int i ) const { return (mstudioiklink_v44_t *)(((byte *)this) + linkindex) + i; };
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
// mstudio_meshvertexdata_v44_t - Mesh vertex data accessor for v44+ models
//-----------------------------------------------------------------------------
struct mstudio_meshvertexdata_v44_t
{
	// base of external vertex data
	const void			*pModelVertexData;
	const void			*pTangentData;		// tangent data pointer

	// indirection to model's vertex data arrays
	int					numLODVertexes[MAX_NUM_LODS_V44];

	// Accessor methods
	mstudiovertex_v44_t	*Vertex( int i ) const { return (mstudiovertex_v44_t *)pModelVertexData + i; }
	Vector4D			*TangentS( int i ) const { return pTangentData ? ((Vector4D *)pTangentData + i) : NULL; }
	bool				HasTangentData() const { return pTangentData != NULL; }
};

//-----------------------------------------------------------------------------
// mstudiomesh_v44_t - Mesh structure for v44+ models
//-----------------------------------------------------------------------------
struct mstudiomesh_v44_t
{
	int					material;

	int					modelindex;
	inline mstudiomodel_v44_t *pModel() const { return (mstudiomodel_v44_t *)(((byte *)this) + modelindex); }

	int					numvertices;		// number of unique vertices/normals/texcoords
	int					vertexoffset;		// vertex mstudiovertex_v44_t

	int					numflexes;			// vertex animation
	int					flexindex;
	inline mstudioflex_v44_t *pFlex( int i ) const { return (mstudioflex_v44_t *)(((byte *)this) + flexindex) + i; };

	// special codes for material operations
	int					materialtype;
	int					materialparam;

	// all unique mesh identifier
	int					meshid;

	Vector				center;

	// Embedded vertex data for v44+ models
	mstudio_meshvertexdata_v44_t vertexdata;

	int					unused[8]; // remove as appropriate

	// Vertex data access - defined out of line
	inline const mstudio_meshvertexdata_v44_t *GetVertexData( void *pModelHeader = NULL ) const
	{
		return &vertexdata;
	}
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
	inline mstudioeyeball_v44_t *pEyeball( int i ) const { return (mstudioeyeball_v44_t *)(((byte *)this) + eyeballindex) + i; };

	// Embedded vertex data pointers (set at load time)
	struct
	{
		const void		*pVertexData;
		const void		*pTangentData;
	} vertexdata;

	int					unused[8];		// remove as appropriate

	// Vertex data caching - returns true if data is valid
	inline bool CacheVertexData( void *pModelHeader ) const { return vertexdata.pVertexData != NULL; }
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
	inline mstudioautolayer_v44_t *pAutolayer( int i ) const { return (mstudioautolayer_v44_t *)(((byte *)this) + autolayerindex) + i; };

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
	inline mstudiomovement_v44_t *pMovement( int i ) const { return (mstudiomovement_v44_t *)(((byte *)this) + movementindex) + i; };

	int					unused1[6];

	int					animblock;
	int					animindex;
	inline mstudioanim_v44_t *pAnim( int *piFrame ) const;  // Implementation in bone_setup_v44.cpp

	int					numikrules;
	int					ikruleindex;
	int					animblockikruleindex;
	inline mstudioikrule_v44_t *pIKRule( int i ) const
	{
		if (ikruleindex)
			return (mstudioikrule_v44_t *)(((byte *)this) + ikruleindex) + i;
		return NULL;
	}

	int					numlocalhierarchy;
	int					localhierarchyindex;
	inline mstudiolocalhierarchy_v44_t *pLocalHierarchy( int i ) const
	{
		if (localhierarchyindex)
			return (mstudiolocalhierarchy_v44_t *)(((byte *)this) + localhierarchyindex) + i;
		return NULL;
	}

	int					sectionindex;
	int					sectionframes;

	short				zeroframespan;
	short				zeroframecount;
	int					zeroframeindex;
	float				zeroframestalltime;
};

//-----------------------------------------------------------------------------
// mstudiolinearbone_v44_t - Linear bone data for v47+ models
// Provides cache-friendly linear arrays of bone data
//-----------------------------------------------------------------------------
struct mstudiolinearbone_v44_t
{
	int numbones;

	int flagsindex;
	inline int flags( int i ) const { Assert( i >= 0 && i < numbones); return *((int *)(((byte *)this) + flagsindex) + i); };
	inline int *pflags( int i ) { Assert( i >= 0 && i < numbones); return ((int *)(((byte *)this) + flagsindex) + i); };

	int	parentindex;
	inline int parent( int i ) const { Assert( i >= 0 && i < numbones); return *((int *)(((byte *)this) + parentindex) + i); };

	int	posindex;
	inline const Vector &pos( int i ) const { Assert( i >= 0 && i < numbones); return *((Vector *)(((byte *)this) + posindex) + i); };

	int quatindex;
	inline const Quaternion &quat( int i ) const { Assert( i >= 0 && i < numbones); return *((Quaternion *)(((byte *)this) + quatindex) + i); };

	int rotindex;
	inline const RadianEuler &rot( int i ) const { Assert( i >= 0 && i < numbones); return *((RadianEuler *)(((byte *)this) + rotindex) + i); };

	int posetoboneindex;
	inline const matrix3x4_t &poseToBone( int i ) const { Assert( i >= 0 && i < numbones); return *((matrix3x4_t *)(((byte *)this) + posetoboneindex) + i); };

	int	posscaleindex;
	inline const Vector &posscale( int i ) const { Assert( i >= 0 && i < numbones); return *((Vector *)(((byte *)this) + posscaleindex) + i); };

	int	rotscaleindex;
	inline const Vector &rotscale( int i ) const { Assert( i >= 0 && i < numbones); return *((Vector *)(((byte *)this) + rotscaleindex) + i); };

	int	qalignmentindex;
	inline const Quaternion &qalignment( int i ) const { Assert( i >= 0 && i < numbones); return *((Quaternion *)(((byte *)this) + qalignmentindex) + i); };

	int unused[6];
};

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
	inline mstudiolinearbone_v44_t *pLinearBones() const { return (linearboneindex != 0) ? (mstudiolinearbone_v44_t *)(((byte *)this) + linearboneindex) : NULL; }

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
	inline mstudioanimblock_v44_t *pAnimBlock( int i ) const { return (mstudioanimblock_v44_t *)(((byte *)this) + animblockindex) + i; };
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

	// Access linear bones through studiohdr2
	inline mstudiolinearbone_v44_t *pLinearBones() const {
		studiohdr2_v44_t *pHdr2 = pStudioHdr2();
		if (pHdr2 && pHdr2->linearboneindex)
			return pHdr2->pLinearBones();
		return NULL;
	}
	inline bool IsV48() const { return version >= STUDIO_VERSION_V48; }
};

// NOTE: mstudioanimblock_v44_t is defined at the top of this file (in forward declarations section)

//-----------------------------------------------------------------------------
// vertexFileHeader_v44_t - Vertex file header for v44+ models (.vvd file)
//-----------------------------------------------------------------------------
struct vertexFileHeader_v44_t
{
	int					id;						// MODEL_VERTEX_FILE_ID_V44
	int					version;				// MODEL_VERTEX_FILE_VERSION_V44
	long				checksum;				// same as studiohdr_v44_t checksum
	int					numLODs;				// number of LODs
	int					numLODVertexes[MAX_NUM_LODS_V44];	// number of unique vertices for each LOD
	int					numFixups;				// number of vertex stream fixup records
	int					fixupTableStart;		// offset to fixup table
	int					vertexDataStart;		// offset to vertex data
	int					tangentDataStart;		// offset to tangent data
};

//-----------------------------------------------------------------------------
// Runtime rendering structures for v44+ models
// These are hardware rendering structures separate from v37
//-----------------------------------------------------------------------------

// Forward declarations for rendering interfaces
class IMesh;
class IMaterial;
namespace OptimizedModel { struct StripHeader_t; }

// Mesh group flags
enum studiomeshgroupflags_v44_t
{
	MESHGROUP_IS_HWSKINNED_V44		= 0x0001,
	MESHGROUP_IS_DELTA_FLEXED_V44	= 0x0002,
};

// Runtime mesh group for hardware rendering
struct studiomeshgroup_v44_t
{
	IMesh*		m_pMesh;
	IMesh*		m_pColorMesh;
	int			m_NumStrips;
	int			m_Flags;		// see studiomeshgroupflags_v44_t
	OptimizedModel::StripHeader_t*	m_pStripData;
	unsigned short*					m_pGroupIndexToMeshIndex;
	int			m_NumVertices;
	int*		m_pUniqueTris;	// for performance measurements
	unsigned short*	m_pIndices;
	bool		m_MeshNeedsRestore;
	short		m_ColorMeshID;

	inline unsigned short MeshIndex( int i ) const { return m_pGroupIndexToMeshIndex[m_pIndices[i]]; }
};

// Runtime mesh data for v44+ models
struct studiomeshdata_v44_t
{
	int						m_NumGroup;
	studiomeshgroup_v44_t*	m_pMeshGroup;
};

// Runtime LOD data for v44+ models
struct studioloddata_v44_t
{
	studiomeshdata_v44_t	*m_pMeshData;
	float					m_SwitchPoint;
	int						numMaterials;
	IMaterial				**ppMaterials;
	int						*pMaterialFlags;
};

// Runtime hardware data for v44+ models
struct studiohwdata_v44_t
{
	int						m_RootLOD;
	int						m_NumLODs;
	studioloddata_v44_t		*m_pLODs;
	int						m_NumStudioMeshes;
};

//-----------------------------------------------------------------------------
// v44+ Render state structures - isolated from v37 types
//-----------------------------------------------------------------------------

// v44+ eyeball rendering state
struct eyeballstate_v44_t
{
	const mstudioeyeball_v44_t *peyeball;

	matrix3x4_t	mat;

	Vector	org;		// world center of eyeball
	Vector	forward;
	Vector	right;
	Vector	up;

	Vector	cornea;		// world center of cornea
};

// v44+ cached position/normal/tangent for flex vertices
struct CachedPosNormTan_v44_t
{
	Vector		m_Position;
	Vector		m_Normal;
	Vector4D	m_TangentS;
};

#endif // STUDIOHDR_V44_H
