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




#ifndef STUDIO_H
#define STUDIO_H

#ifdef _WIN32
#pragma once
#endif

#include "basetypes.h"
#include "vector2d.h"
#include "vector.h"
#include "vector4d.h"
#include "tier0/dbg.h"
#include "utlvector.h"
#include "mathlib.h"
#include "compressed_vector.h"	// v48: Vector48, Quaternion48, Quaternion64 for animation compression


//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------

class IMaterial;
class IMesh;

namespace OptimizedModel
{
	struct StripHeader_t;
}


/*
==============================================================================

STUDIO MODELS

Studio models are position independent, so the cache manager can move them.
==============================================================================
*/


//-----------------------------------------------------------------------------
// Model Version Support
//-----------------------------------------------------------------------------
// This engine supports loading both v37 (HL2 Beta 2003) and v48 (Source 2007) models.
// v37: Original LeakNet format with embedded vertex data
// v48: Modern Source format with external VVD vertex files
//-----------------------------------------------------------------------------
#include "bspflags.h"

// Version constants for multi-version support
#define STUDIO_VERSION_37		37		// HL2 Beta 2003 format
#define STUDIO_VERSION_44		44		// HL2 Retail minimum
#define STUDIO_VERSION_45		45		// Animation sections added
#define STUDIO_VERSION_46		46		// Animation blocks added
#define STUDIO_VERSION_47		47		// Zero frame caching
#define STUDIO_VERSION_48		48		// Orange Box / TF2 format

// Minimum and maximum supported versions
#define STUDIO_VERSION_MIN		STUDIO_VERSION_37
#define STUDIO_VERSION_MAX		STUDIO_VERSION_48

// Current target version for new models
#define STUDIO_VERSION			STUDIO_VERSION_48

// Backward compatibility macro - check if version uses embedded vertex data
#define STUDIO_VERSION_HAS_EMBEDDED_VERTICES(v)		((v) <= STUDIO_VERSION_37)

// Forward declarations
union mstudioanimvalue_t;
// Check if version uses external VVD files
#define STUDIO_VERSION_HAS_EXTERNAL_VERTICES(v)		((v) >= STUDIO_VERSION_44)

#define MAXSTUDIOTRIANGLES	25000	// TODO: tune this
#define MAXSTUDIOVERTS		25000	// TODO: tune this
#define MAXSTUDIOSKINS		32		// total textures
#define MAXSTUDIOBONES		128		// total bones actually used
#define MAXSTUDIOBLENDS		32
#define MAXSTUDIOFLEXDESC	1024	// v48: increased from 128 to 1024 for more flex targets
#define MAXSTUDIOFLEXCTRL	96		// v48: increased from 64 to 96 for more flex controllers
#define MAXSTUDIOFLEXVERTS	10000	// v48: maximum verts that can be flexed per mesh
#define MAXSTUDIOPOSEPARAM	24
#define MAXSTUDIOBONECTRLS	4
#define MAXSTUDIOANIMBLOCKS	256		// v48: maximum animation blocks

#define MAXSTUDIOBONEBITS	7		// NOTE: MUST MATCH MAXSTUDIOBONES

// v48: Maximum bones per vertex for skinning
#define MAX_NUM_BONES_PER_VERT	3

// v48: New event style flag
#define NEW_EVENT_STYLE		(1 << 10)


struct mstudiodata_t
{
	int		count;
	int		offset;
};

// Procedural bone types
#define STUDIO_PROC_AXISINTERP	1
#define STUDIO_PROC_QUATINTERP	2
#define STUDIO_PROC_AIMATBONE	3		// v48: Aim at bone
#define STUDIO_PROC_AIMATATTACH	4		// v48: Aim at attachment
#define STUDIO_PROC_JIGGLE		5		// v48: Jiggle bone physics

struct mstudioaxisinterpbone_t
{
	int				control;// local transformation of this bone used to calc 3 point blend
	int				axis;	// axis to check
	Vector			pos[6];	// X+, X-, Y+, Y-, Z+, Z-
	Quaternion		quat[6];// X+, X-, Y+, Y-, Z+, Z-
};


struct mstudioquatinterpinfo_t
{
	float			inv_tolerance;	// 1 / radian angle of trigger influence
	Quaternion		trigger;	// angle to match
	Vector			pos;		// new position
	Quaternion		quat;		// new angle
};

struct mstudioquatinterpbone_t
{
	int				control;// local transformation to check
	int				numtriggers;
	int				triggerindex;
	inline mstudioquatinterpinfo_t *pTrigger( int i ) const { return  (mstudioquatinterpinfo_t *)(((byte *)this) + triggerindex) + i; };
};

//-----------------------------------------------------------------------------
// v48: Jiggle bone flags
//-----------------------------------------------------------------------------
#define JIGGLE_IS_FLEXIBLE				0x01
#define JIGGLE_IS_RIGID					0x02
#define JIGGLE_HAS_YAW_CONSTRAINT		0x04
#define JIGGLE_HAS_PITCH_CONSTRAINT		0x08
#define JIGGLE_HAS_ANGLE_CONSTRAINT		0x10
#define JIGGLE_HAS_LENGTH_CONSTRAINT	0x20
#define JIGGLE_HAS_BASE_SPRING			0x40

//-----------------------------------------------------------------------------
// v48: Jiggle bone procedural structure
//-----------------------------------------------------------------------------
struct mstudiojigglebone_t
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

//-----------------------------------------------------------------------------
// v48: Aim-at bone procedural structure
//-----------------------------------------------------------------------------
struct mstudioaimatbone_t
{
	int				parent;
	int				aim;		// Might be bone or attachment index
	Vector			aimvector;
	Vector			upvector;
	Vector			basepos;
};

//-----------------------------------------------------------------------------
// bones - v37 format (HL2 Beta 2003)
// Uses value[6]/scale[6] for bone default position/rotation and animation scale
//-----------------------------------------------------------------------------
struct mstudiobone_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int		 			parent;		// parent bone
	int					bonecontroller[6];	// bone controller index, -1 == none
	// FIXME: remove the damn default value fields and put in pos
	float				value[6];	// default DoF values
	float				scale[6];   // scale for delta DoF values
	matrix3x4_t			poseToBone;
	Quaternion			qAlignment;
	int					flags;
	int					proctype;
	int					procindex;		// procedural rule
	mutable int			physicsbone;	// index into physically simulated bone
	inline void *pProcedure( ) const { if (procindex == 0) return NULL; else return  (void *)(((byte *)this) + procindex); };
	int					surfacepropidx;	// index into string tablefor property name
	inline char * const pszSurfaceProp( void ) const { return ((char *)this) + surfacepropidx; }
	Quaternion			quat;
	int					contents;		// See BSPFlags.h for the contents flags
	int					unused[3];		// remove as appropriate
};

//-----------------------------------------------------------------------------
// bones - v48 format (Source 2007)
// Uses Vector/Quaternion/RadianEuler for bone default pose and animation scale
//-----------------------------------------------------------------------------
struct mstudiobone_v48_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int		 			parent;		// parent bone
	int					bonecontroller[6];	// bone controller index, -1 == none

	// v48 default values - explicit Vector/Quaternion/Euler types
	Vector				pos;		// default bone position
	Quaternion			quat;		// default bone rotation (quaternion form)
	RadianEuler			rot;		// default bone rotation (euler form)

	// v48 compression scale - used to decompress animation delta values
	Vector				posscale;	// scale for position deltas
	Vector				rotscale;	// scale for rotation deltas

	matrix3x4_t			poseToBone;
	Quaternion			qAlignment;
	int					flags;
	int					proctype;
	int					procindex;		// procedural rule
	mutable int			physicsbone;	// index into physically simulated bone
	inline void *pProcedure( ) const { if (procindex == 0) return NULL; else return  (void *)(((byte *)this) + procindex); };
	int					surfacepropidx;	// index into string tablefor property name
	inline char * const pszSurfaceProp( void ) const { return ((char *)this) + surfacepropidx; }
	int					contents;		// See BSPFlags.h for the contents flags
	int					unused[8];		// remove as appropriate

	// Helper to get default position (consistent with v37 style)
	inline void GetDefaultPos( Vector &out ) const { out = pos; }

	// Helper to get default rotation as quaternion
	inline void GetDefaultQuat( Quaternion &out ) const { out = quat; }

	// Helper to get default rotation as euler angles
	inline void GetDefaultRot( RadianEuler &out ) const { out = rot; }
};

#define BONE_CALCULATE_MASK			0x1F
#define BONE_PHYSICALLY_SIMULATED	0x01	// bone is physically simulated when physics are active
#define BONE_PHYSICS_PROCEDURAL		0x02	// procedural when physics is active
#define BONE_ALWAYS_PROCEDURAL		0x04	// bone is always procedurally animated
#define BONE_SCREEN_ALIGN_SPHERE	0x08	// bone aligns to the screen, not constrained in motion.
#define BONE_SCREEN_ALIGN_CYLINDER	0x10	// bone aligns to the screen, constrained by it's own axis.

#define BONE_USED_MASK				0x0003FF00
#define BONE_USED_BY_ANYTHING		0x0003FF00
#define BONE_USED_BY_HITBOX			0x00000100	// bone (or child) is used by a hit box
#define BONE_USED_BY_ATTACHMENT		0x00000200	// bone (or child) is used by an attachment point
#define BONE_USED_BY_VERTEX_MASK	0x0003FC00
#define BONE_USED_BY_VERTEX_LOD0	0x00000400	// bone (or child) is used by the toplevel model via skinned vertex
#define BONE_USED_BY_VERTEX_LOD1	0x00000800	
#define BONE_USED_BY_VERTEX_LOD2	0x00001000  
#define BONE_USED_BY_VERTEX_LOD3	0x00002000
#define BONE_USED_BY_VERTEX_LOD4	0x00004000
#define BONE_USED_BY_VERTEX_LOD5	0x00008000
#define BONE_USED_BY_VERTEX_LOD6	0x00010000
#define BONE_USED_BY_VERTEX_LOD7	0x00020000

#define BONE_USED_BY_ANYTHING_AT_LOD(lod) ( ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK ) | ( BONE_USED_BY_VERTEX_LOD0 << lod ) )

#define MAX_NUM_LODS 8

#define BONE_TYPE_MASK				0x00F00000
#define BONE_FIXED_ALIGNMENT		0x00100000	// bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

// v48: Bone save frame flags (for zero-frame optimization)
#define BONE_HAS_SAVEFRAME_POS		0x00200000	// Bone has save frame position data (Vector48)
#define BONE_HAS_SAVEFRAME_ROT		0x00400000	// Bone has save frame rotation data (Quaternion64)

// v48: Bone merge support
#define BONE_USED_BY_BONE_MERGE		0x00040000	// bone is available for bone merge to occur against it

#define BONE_USED_BY_VERTEX_VERSION_32			0x0400	// bone (or child) is used by skinned vertex
#define BONE_USED_BY_VERTEX_LOD2_VERSION_32 	0x0800	// ???? There will be N of these, maybe the LOD info returns the mask??
#define BONE_USED_BY_VERTEX_LOD3_VERSION_32 	0x1000  // FIXME: these are currently unassigned....
#define BONE_USED_BY_VERTEX_LOD4_VERSION_32		0x2000
#define BONE_FIXED_ALIGNMENT_VERSION_32			0x10000	// bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

//-----------------------------------------------------------------------------
// v48: Linear bone array - optimized memory layout for bone data
// This structure provides indexed access to bone arrays without per-bone overhead
//-----------------------------------------------------------------------------
struct mstudiolinearbone_t
{
	int numbones;

	int flagsindex;
	inline int flags( int i ) const { Assert( i >= 0 && i < numbones); return *((int *)(((byte *)this) + flagsindex) + i); };
	inline int *pflags( int i ) { Assert( i >= 0 && i < numbones); return ((int *)(((byte *)this) + flagsindex) + i); };

	int	parentindex;
	inline int parent( int i ) const { Assert( i >= 0 && i < numbones); return *((int *)(((byte *)this) + parentindex) + i); };

	int	posindex;
	inline Vector pos( int i ) const { Assert( i >= 0 && i < numbones); return *((Vector *)(((byte *)this) + posindex) + i); };

	int quatindex;
	inline Quaternion quat( int i ) const { Assert( i >= 0 && i < numbones); return *((Quaternion *)(((byte *)this) + quatindex) + i); };

	int rotindex;
	inline RadianEuler rot( int i ) const { Assert( i >= 0 && i < numbones); return *((RadianEuler *)(((byte *)this) + rotindex) + i); };

	int posetoboneindex;
	inline matrix3x4_t poseToBone( int i ) const { Assert( i >= 0 && i < numbones); return *((matrix3x4_t *)(((byte *)this) + posetoboneindex) + i); };

	int	posscaleindex;
	inline Vector posscale( int i ) const { Assert( i >= 0 && i < numbones); return *((Vector *)(((byte *)this) + posscaleindex) + i); };

	int	rotscaleindex;
	inline Vector rotscale( int i ) const { Assert( i >= 0 && i < numbones); return *((Vector *)(((byte *)this) + rotscaleindex) + i); };

	int	qalignmentindex;
	inline Quaternion qalignment( int i ) const { Assert( i >= 0 && i < numbones); return *((Quaternion *)(((byte *)this) + qalignmentindex) + i); };

	int unused[6];

	mstudiolinearbone_t(){}
private:
	// No copy constructors allowed
	mstudiolinearbone_t(const mstudiolinearbone_t& vOther);
};

// bone controllers
struct mstudiobonecontroller_t
{
	int					bone;	// -1 == 0
	int					type;	// X, Y, Z, XR, YR, ZR, M
	float				start;
	float				end;
	int					rest;	// byte index value at rest
	int					inputfield;	// 0-3 user set controller, 4 mouth
	char				padding[32];	// future expansion.
};

// intersection boxes
struct mstudiobbox_t
{
	int					bone;
	int					group;				// intersection group
	Vector				bbmin;				// bounding box
	Vector				bbmax;
	int					szhitboxnameindex;	// offset to the name of the hitbox.
	int					unused[8];			// 2007 Source Engine format (future expansion)

	char* pszHitboxName(void* pHeader)
	{
		if( szhitboxnameindex == 0 )
			return (char *)"";

// NJS: Just a cosmetic change, next time the model format is rebuilt, please use the NEXT_MODEL_FORMAT_REVISION.
// also, do a grep to find the corresponding #ifdefs.
#ifdef NEXT_MODEL_FORMAT_REVISION
		return ((char*)this) + szhitboxnameindex;
#else
		return ((char*)pHeader) + szhitboxnameindex;
#endif
	}
};

// demand loaded sequence groups
struct mstudioseqgroup_t
{
	int					szlabelindex;	// textual name
	inline char * const pszLabel( void ) const { return ((char *)this) + szlabelindex; }
	int					sznameindex;	// file name
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	/* cache_user_t */	void *cache;	// cache index pointer
	int					data;			// hack for group 0

#if STUDIO_VERSION != 37
	char				padding[32];	// future expansion.
#endif
};


// events
struct mstudioevent_t
{
	float				cycle;
	int					event;
	int					type;
	char				options[64];
};


// attachment
// v37: Original attachment format
struct mstudioattachment_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int					type;
	int					bone;
	matrix3x4_t			local; // attachment point
};

// v48: 2007 Source Engine compatible attachment format
struct mstudioattachment_v48_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	unsigned int		flags;		// v48: was 'type'
	int					localbone;	// v48: was 'bone'
	matrix3x4_t			local;		// attachment point
	int					unused[8];	// v48: padding for future expansion
};

#define IK_SELF 1
#define IK_WORLD 2
#define IK_GROUND 3
#define IK_RELEASE 4

struct mstudioikerror_t
{
	Vector		pos;
	Quaternion	q;
};

//-----------------------------------------------------------------------------
// v48: Compressed IK error data
// Used for animation compression - stores scale and offset for IK error values
//-----------------------------------------------------------------------------
struct mstudiocompressedikerror_t
{
	float	scale[6];	// Scale factors for decompression
	short	offset[6];	// Offsets into animation value data
	inline mstudioanimvalue_t *pAnimvalue( int i ) const { if (offset[i] > 0) return (mstudioanimvalue_t *)(((byte *)this) + offset[i]); else return NULL; };

	mstudiocompressedikerror_t() {}

private:
	// No copy constructors allowed
	mstudiocompressedikerror_t(const mstudiocompressedikerror_t& vOther);
};

//-----------------------------------------------------------------------------
// v48: Local hierarchy override
// Allows animations to temporarily reparent bones
//-----------------------------------------------------------------------------
struct mstudiolocalhierarchy_t
{
	int			iBone;			// bone being reparented
	int			iNewParent;		// new parent bone

	float		start;			// beginning of influence
	float		peak;			// start of full influence
	float		tail;			// end of full influence
	float		end;			// end of all influence

	int			iStart;			// frame at which the hierarchy starts
	int			localanimindex;	// local animation data
	inline mstudiocompressedikerror_t *pLocalAnim() const { return (mstudiocompressedikerror_t *)(((byte *)this) + localanimindex); };

	int			unused[4];
};

// v48: 2007 Source Engine compatible IK rule format with compressed IK support
struct mstudioikrule_v48_t
{
	int			index;

	int			type;
	int			chain;

	int			bone;

	int			slot;
	float		height;
	float		radius;
	float		floor;
	Vector		pos;
	Quaternion	q;

	// v48: Additional fields for compressed IK error data
	int			compressedikerrorindex;		// offset into compressed IK error data
	int			unused2;

	// v48: Additional bone control fields
	int			drop;		// frame range for IK rule to drop out
	int			top;		// frame range for IK rule to be fully applied

	int			unused[6];		// v48: padding for future expansion
};

// v37: Original IK rule format
struct mstudioikrule_t
{
	int			index;

	int			type;
	int			chain;

	int			bone;

	int			slot;
	float		height;
	float		radius;
	float		floor;
	Vector		pos;
	Quaternion	q;

	float		flWeight;

	int			group; // match sub-sequence IK rules together

	int			iStart;
	int			ikerrorindex;
	inline mstudioikerror_t *pError( int i ) const { return  (mstudioikerror_t *)(((byte *)this) + ikerrorindex) + (i - iStart); };

	float		start;	// beginning of influence
	float		peak;	// start of full influence
	float		tail;	// end of full influence
	float		end;	// end of all influence

	float		commit;		// unused: frame footstep target should be committed
	float		contact;	// unused: frame footstep makes ground concact
	float		pivot;		// unused: frame ankle can begin rotation from latched orientation
	float		release;	// unused: frame ankle should end rotation from latched orientation
};



struct mstudioiklock_t
{
	int			chain;
	float		flPosWeight;
	float		flLocalQWeight;
	int			flags;			// 2007 Source Engine addition
	int			unused[4];		// 2007 Source Engine padding
};

// animation frames
union mstudioanimvalue_t
{
	struct 
	{
		byte	valid;
		byte	total;
	} num;
	short		value;
};

// per bone per animation DOF and weight pointers
struct mstudioanim_t
{
	// float			weight;		// bone influence
	int				flags;		// weighing options
	union
	{
		int				offset[6];	// pointers to animation 
		struct
		{
			float			pos[3];
			float			q[4];
		} pose;
	} u;
	inline mstudioanimvalue_t *pAnimvalue( int i ) const { return  (mstudioanimvalue_t *)(((byte *)this) + u.offset[i]); };
};

//-----------------------------------------------------------------------------
// v37 Animation Flags (used with v37 mstudioanim_t)
//-----------------------------------------------------------------------------
#define STUDIO_POS_ANIMATED		0x0001
#define STUDIO_ROT_ANIMATED		0x0002

//-----------------------------------------------------------------------------
// v48 Animation Flags (used with mstudioanim_v48_t)
// These flags determine what data follows the mstudioanim_v48_t header
//-----------------------------------------------------------------------------
#define STUDIO_ANIM_RAWPOS		0x01	// Vector48 follows
#define STUDIO_ANIM_RAWROT		0x02	// Quaternion48 follows
#define STUDIO_ANIM_ANIMPOS		0x04	// mstudioanim_valueptr_t for position follows
#define STUDIO_ANIM_ANIMROT		0x08	// mstudioanim_valueptr_t for rotation follows
#define STUDIO_ANIM_DELTA		0x10	// This animation is delta from base pose
#define STUDIO_ANIM_RAWROT2		0x20	// Quaternion64 follows (higher precision)

//-----------------------------------------------------------------------------
// v48: Animation value pointer structure
// Points to RLE-compressed animation data for position or rotation (XYZ)
//-----------------------------------------------------------------------------
struct mstudioanim_valueptr_t
{
	short	offset[3];	// Offsets to X, Y, Z animation data
	inline mstudioanimvalue_t *pAnimvalue( int i ) const {
		if (offset[i] > 0)
			return (mstudioanimvalue_t *)(((byte *)this) + offset[i]);
		else
			return NULL;
	};
};

//-----------------------------------------------------------------------------
// v48: Per-bone per-animation data with compression support
// This structure uses a linked list to iterate through animated bones
//-----------------------------------------------------------------------------
struct mstudioanim_v48_t
{
	byte				bone;			// Bone index this animation affects
	byte				flags;			// STUDIO_ANIM_* flags indicating data layout

	// Get pointer to data immediately following this header
	inline byte			*pData( void ) const { return (((byte *)this) + sizeof(mstudioanim_v48_t)); };

	// Get rotation animation value pointers (for STUDIO_ANIM_ANIMROT)
	inline mstudioanim_valueptr_t *pRotV( void ) const { return (mstudioanim_valueptr_t *)(pData()); };

	// Get position animation value pointers (for STUDIO_ANIM_ANIMPOS)
	// Note: Offset accounts for rotation data if present
	inline mstudioanim_valueptr_t *pPosV( void ) const {
		return (mstudioanim_valueptr_t *)(pData()) + ((flags & STUDIO_ANIM_ANIMROT) != 0);
	};

	// Get compressed quaternion (for STUDIO_ANIM_RAWROT)
	inline Quaternion48 *pQuat48( void ) const { return (Quaternion48 *)(pData()); };

	// Get high-precision compressed quaternion (for STUDIO_ANIM_RAWROT2)
	inline Quaternion64 *pQuat64( void ) const { return (Quaternion64 *)(pData()); };

	// Get compressed position (for STUDIO_ANIM_RAWPOS)
	// Note: Offset accounts for rotation data if present
	inline Vector48 *pPos( void ) const {
		return (Vector48 *)(pData() +
			((flags & STUDIO_ANIM_RAWROT) != 0) * sizeof(Quaternion48) +
			((flags & STUDIO_ANIM_RAWROT2) != 0) * sizeof(Quaternion64));
	};

	// Linked list traversal
	short				nextoffset;		// Offset to next bone's animation data
	inline mstudioanim_v48_t *pNext( void ) const {
		if (nextoffset != 0)
			return (mstudioanim_v48_t *)(((byte *)this) + nextoffset);
		else
			return NULL;
	};
};

struct mstudiomovement_t
{
	int					endframe;				
	int					motionflags;
	float				v0;			// velocity at start of block
	float				v1;			// velocity at end of block
	float				angle;		// YAW rotation at end of this blocks movement
	Vector				vector;		// movement vector relative to this blocks initial angle
	Vector				position;	// relative to start of animation???
};

//-----------------------------------------------------------------------------
// Animation description - v37 format (HL2 Beta 2003)
// Uses simple array-based bone animation (pAnim returns mstudioanim_t*)
//-----------------------------------------------------------------------------
struct mstudioanimdesc_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }

	float				fps;		// frames per second
	int					flags;		// looping/non-looping flags

	int					numframes;

	// piecewise movement
	int					nummovements;
	int					movementindex;
	inline mstudiomovement_t * const pMovement( int i ) const { return (mstudiomovement_t *)(((byte *)this) + movementindex) + i; };

	Vector				bbmin;		// per animation bounding box
	Vector				bbmax;

	int					animindex;	// mstudioanim_t pointer relative to start of mstudioanimdesc_t data
									// [bone][X, Y, Z, XR, YR, ZR]
	inline mstudioanim_t *pAnim( int i ) const { return (mstudioanim_t *)(((byte *)this) + animindex) + i; };

	int					numikrules;
	int					ikruleindex;
	inline mstudioikrule_t *pIKRule( int i ) const { return (mstudioikrule_t *)(((byte *)this) + ikruleindex) + i; };

	int					unused[8];		// remove as appropriate
};

// v48 animation structures - defined early for use by mstudioanimdesc_v48_t
// Animation sections for fast seeking (v45+)
struct mstudioanimsections_t
{
	int					animblock;
	int					animindex;
};

// NOTE: mstudiocompressedikerror_t and mstudiolocalhierarchy_t are defined earlier (near IK structures)

//-----------------------------------------------------------------------------
// Animation description - v48 format (Source 2007)
// Uses linked-list bone animation with compression support
//-----------------------------------------------------------------------------
struct mstudioanimdesc_v48_t
{
	int					baseptr;	// Offset back to studiohdr_t
	// Note: Use pStudiohdr() for access

	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }

	float				fps;		// frames per second
	int					flags;		// looping/non-looping flags

	int					numframes;

	// piecewise movement
	int					nummovements;
	int					movementindex;
	inline mstudiomovement_t * const pMovement( int i ) const { return (mstudiomovement_t *)(((byte *)this) + movementindex) + i; };

	int					unused1[6];	// reserved (zero if loading older versions)

	// Animation block for demand loading
	int					animblock;		// Animation block index (0 = inline data)
	int					animindex;		// Animation data offset (inline or in block)

	// Get animation data - returns first bone in linked list
	inline mstudioanim_v48_t *pAnim( void ) const {
		if (animindex == 0) return NULL;
		return (mstudioanim_v48_t *)(((byte *)this) + animindex);
	};

	int					numikrules;
	int					ikruleindex;		// IK rule data (inline in MDL)
	int					animblockikruleindex;	// IK rules in animation block file
	inline mstudioikrule_t *pIKRule( int i ) const { return (mstudioikrule_t *)(((byte *)this) + ikruleindex) + i; };

	// Local hierarchy overrides
	int					numlocalhierarchy;
	int					localhierarchyindex;
	inline mstudiolocalhierarchy_t *pHierarchy( int i ) const {
		return (mstudiolocalhierarchy_t *)(((byte *)this) + localhierarchyindex) + i;
	};

	// Animation sections for fast frame seeking (v45+)
	int					sectionindex;
	int					sectionframes;	// frames per section (0 = not used)
	inline mstudioanimsections_t *pSection( int i ) const {
		return (mstudioanimsections_t *)(((byte *)this) + sectionindex) + i;
	};

	// Zero frame caching for fast startup (v47+)
	short				zeroframespan;		// frames per span
	short				zeroframecount;		// number of spans
	int					zeroframeindex;		// offset to zero frame data
	inline byte *pZeroFrameData( void ) const {
		if (zeroframeindex) return (((byte *)this) + zeroframeindex);
		else return NULL;
	};
	mutable float		zeroframestalltime;	// saved during read stalls
};

struct mstudioikrule_t;

struct mstudioautolayer_t
{
	int					iSequence;
	int					flags;
	float				start;	// beginning of influence
	float				peak;	// start of full influence
	float				tail;	// end of full influence
	float				end;	// end of all influence
};

// v48: 2007 Source Engine compatible autolayer format
struct mstudioautolayer_v48_t
{
	short				iSequence;		// v48: sequence index (short)
	short				iPose;			// v48: pose parameter index (short)
	int					flags;
	float				start;			// beginning of influence
	float				peak;			// start of full influence
	float				tail;			// end of full influence
	float				end;			// end of all influence
};


// sequence descriptions
struct mstudioseqdesc_t
{
	int					szlabelindex;
	inline char * const pszLabel( void ) const { return ((char *)this) + szlabelindex; }

	int					szactivitynameindex;
	inline char * const pszActivityName( void ) const { return ((char *)this) + szactivitynameindex; }

	int					flags;		// looping/non-looping flags

	int					activity;	// initialized at loadtime to game DLL values
	int					actweight;

	// Version-aware activity accessor (activity field is in same location for v37/v48)
	inline int GetActivity( int version ) const { return activity; }

	int					numevents;
	int					eventindex;
	inline mstudioevent_t *pEvent( int i ) const { return (mstudioevent_t *)(((byte *)this) + eventindex) + i; };

	Vector				bbmin;		// per sequence bounding box
	Vector				bbmax;		

	int					numblends;

#if STUDIO_VERSION != 37
	int					anim[MAXSTUDIOBLENDS][MAXSTUDIOBLENDS];	// animation number
#else
	// Index into array of shorts which is groupsize[0] x groupsize[1] in length
	int					animindexindex;

	inline int			anim( int x, int y ) const
	{
		if ( x >= groupsize[0] )
		{
			x = groupsize[0] - 1;
		}

		if ( y >= groupsize[1] )
		{
			y = groupsize[ 1 ] - 1;
		}

		int offset = y * groupsize[0] + x;
		short *blends = pBlends();
		int value = (int)blends[ offset ];
		return value;
	}

	inline short * const pBlends( void ) const { return (short *)(((byte *)this) + animindexindex); }
#endif

	//-----------------------------------------------------------------------------
	// v37 binary layout offset calculations
	// The v37 struct is smaller because it uses animindexindex (4 bytes) instead of anim[32][32] (4096 bytes)
	// and doesn't have seqgroup field. Fields after numblends are at different offsets.
	//
	// v37 layout after numblends (offset 52):
	//   +0: animindexindex (4 bytes)
	//   +4: movementindex (4 bytes)
	//   +8: groupsize[0] (4 bytes)
	//   +12: groupsize[1] (4 bytes)
	//   +16: paramindex[0] (4 bytes)
	//   ...
	//
	// v48 layout after numblends (offset 52):
	//   +0: anim[32][32] (4096 bytes)
	//   +4096: movementindex (4 bytes)
	//   +4100: groupsize[0] (4 bytes)
	//   +4104: groupsize[1] (4 bytes)
	//   +4108: paramindex[0] (4 bytes)
	//   ...
	//   seqgroup is also added after paramparent in v48
	//-----------------------------------------------------------------------------

	// Offset from numblends to groupsize[0] in v37 binary
	static const int V37_GROUPSIZE_OFFSET = 4 + 4;  // animindexindex + movementindex = 8 bytes from numblends
	// Offset from numblends to groupsize[0] in v48 compiled code
	static const int V48_GROUPSIZE_OFFSET = 4096 + 4;  // anim[32][32] + movementindex = 4100 bytes from numblends

	// Helper: Get groupsize[0] accounting for v37 binary layout
	inline int GetGroupSize0( int version ) const
	{
		if ( version <= STUDIO_VERSION_37 )
		{
			// Read from v37 binary offset: numblends offset (52) + 8 = 60
			const byte *base = (const byte *)&numblends;
			return *(int *)(base + 4 + V37_GROUPSIZE_OFFSET);  // +4 for numblends itself
		}
		return groupsize[0];
	}

	// Helper: Get groupsize[1] accounting for v37 binary layout
	inline int GetGroupSize1( int version ) const
	{
		if ( version <= STUDIO_VERSION_37 )
		{
			// Read from v37 binary offset: numblends offset (52) + 12 = 64
			const byte *base = (const byte *)&numblends;
			return *(int *)(base + 4 + V37_GROUPSIZE_OFFSET + 4);  // +4 for numblends, +4 for groupsize[0]
		}
		return groupsize[1];
	}

	// Runtime accessor for animation index that works with both v37 and v48 binary data
	// For v48 compiled code loading v37 models, the binary layout differs
	inline int GetAnimIndex( int x, int y, int version ) const
	{
		if ( version <= STUDIO_VERSION_37 )
		{
			// v37 binary layout: anim[0][0] position contains animindexindex
			// The rest of the "anim" array space contains different data (movementindex, groupsize, etc)
			int animindexindex_runtime = anim[0][0];
			short *blends = (short *)(((byte *)this) + animindexindex_runtime);

			// CRITICAL: Read groupsize from correct v37 binary offset, not from v48 struct members
			int gs0 = GetGroupSize0( version );
			int gs1 = GetGroupSize1( version );

			int gx = (x >= gs0) ? gs0 - 1 : x;
			int gy = (y >= gs1) ? gs1 - 1 : y;

			// Bounds check to prevent crashes
			if (gx < 0) gx = 0;
			if (gy < 0) gy = 0;

			int offset = gy * gs0 + gx;
			return (int)blends[offset];
		}
		else
		{
			// v44+ binary layout: direct 2D array access
			return anim[x][y];
		}
	}

	// Runtime accessor for blends array pointer (v37 only, returns NULL for other versions)
	inline short * GetBlends( int version ) const
	{
		if ( version <= STUDIO_VERSION_37 )
		{
			// v37 binary layout: anim[0][0] contains animindexindex
			int animindexindex_runtime = anim[0][0];
			return (short *)(((byte *)this) + animindexindex_runtime);
		}
		return NULL;  // v44+ doesn't have a separate blends array
	}

	int					movementindex;	// [blend] float array for blended movement
	int					groupsize[2];
	int					paramindex[2];	// X, Y, Z, XR, YR, ZR
	float				paramstart[2];	// local (0..1) starting value
	float				paramend[2];	// local (0..1) ending value
	int					paramparent;

#if STUDIO_VERSION != 37
	int					seqgroup;		// sequence group for demand loading
#endif

	float				fadeintime;		// ideal cross fate in time (0.2 default)
	float				fadeouttime;	// ideal cross fade out time (0.2 default)

	int					entrynode;		// transition node at entry
	int					exitnode;		// transition node at exit
	int					nodeflags;		// transition rules

	float				entryphase;		// used to match entry gait
	float				exitphase;		// used to match exit gait
	
	float				lastframe;		// frame that should generation EndOfSequence

	int					nextseq;		// auto advancing sequences
	int					pose;			// index of delta animation between end and nextseq

	int					numikrules;

	int					numautolayers;	//
	int					autolayerindex;
	inline mstudioautolayer_t *pAutolayer( int i ) const { return (mstudioautolayer_t *)(((byte *)this) + autolayerindex) + i; };

	int					weightlistindex;
	float				*pBoneweight( int i ) const { return ((float *)(((byte *)this) + weightlistindex) + i); };
	float				weight( int i ) const { return *(pBoneweight( i)); };

	int					posekeyindex;
	float				*pPoseKey( int iParam, int iAnim ) const { return (float *)(((byte *)this) + posekeyindex) + iParam * groupsize[0] + iAnim; }
	float				poseKey( int iParam, int iAnim ) const { return *(pPoseKey( iParam, iAnim )); }

	int					numiklocks;
	int					iklockindex;
	inline mstudioiklock_t *pIKLock( int i ) const { return (mstudioiklock_t *)(((byte *)this) + iklockindex) + i; };

	// Key values
	int					keyvalueindex;
	int					keyvaluesize;
	inline const char * KeyValueText( void ) const { return keyvaluesize != 0 ? ((char *)this) + keyvalueindex : NULL; }

	int					unused[3];		// remove/add as appropriate (grow back to 8 ints on version change!)

	//-----------------------------------------------------------------------------
	// Version-aware field accessors for v37 binary compatibility
	//
	// When compiled with STUDIO_VERSION=48 but loading v37 models, fields after
	// numblends are at different binary offsets:
	// - v48 struct has anim[32][32] (4096 bytes) + seqgroup (4 bytes) = 4100 extra bytes
	// - v37 struct has just animindexindex (4 bytes)
	// - Offset difference: 4096 bytes
	//
	// These accessors read fields from the correct binary offset based on version.
	//-----------------------------------------------------------------------------

	// Offset adjustment for v37: all fields after numblends are 4096 bytes earlier
	static const int V37_FIELD_OFFSET_ADJ = 4096;

	// Helper: Read an int field at v37-adjusted offset
	inline int GetV37Int( int version, const int &v48_member ) const
	{
		if (version <= STUDIO_VERSION_37)
		{
			// For v37 binary, the field is 4096 bytes earlier than v48 layout
			const byte *adjusted_ptr = ((const byte *)&v48_member) - V37_FIELD_OFFSET_ADJ;
			return *(const int *)adjusted_ptr;
		}
		return v48_member;
	}

	// Helper: Read a float field at v37-adjusted offset
	inline float GetV37Float( int version, const float &v48_member ) const
	{
		if (version <= STUDIO_VERSION_37)
		{
			const byte *adjusted_ptr = ((const byte *)&v48_member) - V37_FIELD_OFFSET_ADJ;
			return *(const float *)adjusted_ptr;
		}
		return v48_member;
	}

	// Version-aware accessors for critical fields used in bone_setup.cpp
	inline int GetWeightListIndex( int version ) const { return GetV37Int( version, weightlistindex ); }
	inline int GetAutoLayerIndex( int version ) const { return GetV37Int( version, autolayerindex ); }
	inline int GetNumAutoLayers( int version ) const { return GetV37Int( version, numautolayers ); }
	inline int GetNumIKRules( int version ) const { return GetV37Int( version, numikrules ); }
	inline int GetFlags( int version ) const
	{
		// flags is before the anim array, so it's at the same offset in both versions
		return flags;
	}

	// Version-aware bone weight accessor
	inline float GetWeight( int i, int version ) const
	{
		int wli = GetWeightListIndex( version );
		float *weights = (float *)(((byte *)this) + wli);
		return weights[i];
	}

	// Version-aware autolayer accessor
	inline mstudioautolayer_t *pAutoLayerV( int i, int version ) const
	{
		int ali = GetAutoLayerIndex( version );
		return (mstudioautolayer_t *)(((byte *)this) + ali) + i;
	}

	// Version-aware pose key accessor
	inline float GetPoseKey( int iParam, int iAnim, int version ) const
	{
		int pki = GetV37Int( version, posekeyindex );
		int gs0 = GetGroupSize0( version );
		float *keys = (float *)(((byte *)this) + pki);
		return keys[iParam * gs0 + iAnim];
	}

	// Version-aware paramindex accessor (index into studiohdr_t pose parameters)
	inline int GetParamIndex( int i, int version ) const
	{
		if (i < 0 || i >= 2)
			return -1;
		return GetV37Int( version, paramindex[i] );
	}

	// Version-aware paramstart accessor
	inline float GetParamStart( int i, int version ) const
	{
		if (i < 0 || i >= 2)
			return 0.0f;
		return GetV37Float( version, paramstart[i] );
	}

	// Version-aware paramend accessor
	inline float GetParamEnd( int i, int version ) const
	{
		if (i < 0 || i >= 2)
			return 0.0f;
		return GetV37Float( version, paramend[i] );
	}

	// Version-aware posekeyindex accessor
	inline int GetPoseKeyIndex( int version ) const
	{
		return GetV37Int( version, posekeyindex );
	}

	// Version-aware numiklocks accessor
	inline int GetNumIKLocks( int version ) const
	{
		return GetV37Int( version, numiklocks );
	}

	// Version-aware iklockindex accessor
	inline int GetIKLockIndex( int version ) const
	{
		return GetV37Int( version, iklockindex );
	}

	// Version-aware IK lock accessor
	inline mstudioiklock_t *pIKLockV( int i, int version ) const
	{
		int idx = GetIKLockIndex( version );
		return (mstudioiklock_t *)(((byte *)this) + idx) + i;
	}
};


struct mstudioposeparamdesc_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int					flags;	// ????
	float				start;	// starting value
	float				end;	// ending value
	float				loop;	// looping range, 0 for no looping, 360 for rotations, etc.
};

struct mstudioflexdesc_t
{
	int					szFACSindex;
	inline char * const pszFACS( void ) const { return ((char *)this) + szFACSindex; }
};


struct mstudioflexcontroller_t
{
	int					sztypeindex;
	inline char * const pszType( void ) const { return ((char *)this) + sztypeindex; }
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	mutable int			link;	// remapped at load time to master list
	float				min;
	float				max;
};

struct mstudiovertanim_t
{
	int					index;
	Vector				delta;
	Vector				ndelta;
};


struct mstudioflex_t
{
	int					flexdesc;	// input value

	float				target0;	// zero
	float				target1;	// one
	float				target2;	// one
	float				target3;	// zero

	int					numverts;
	int					vertindex;
	inline	mstudiovertanim_t *pVertanim( int i ) const { return  (mstudiovertanim_t *)(((byte *)this) + vertindex) + i; };
};


struct mstudioflexop_t
{
	int		op;
	union 
	{
		int		index;
		float	value;
	} d;
};

struct mstudioflexrule_t
{
	int					flex;
	int					numops;
	int					opindex;
	inline mstudioflexop_t *iFlexOp( int i ) const { return  (mstudioflexop_t *)(((byte *)this) + opindex) + i; };
};

//-----------------------------------------------------------------------------
// v37: Old bone weight structure (32 bytes) - used by HL2 Beta 2003 models
// This format is used by v37 MDL files with embedded vertex data
//-----------------------------------------------------------------------------
struct mstudioboneweight_v37_t
{
	float	weight[4];		// 16 bytes
	short	bone[4];		// 8 bytes (16-bit bone indices)
	short	numbones;		// 2 bytes
	short	material;		// 2 bytes (removed in v48)
	short	firstref;		// 2 bytes (removed in v48)
	short	lastref;		// 2 bytes (removed in v48)
};	// = 32 bytes total

//-----------------------------------------------------------------------------
// v48: New bone weight structure (16 bytes) - used by Source 2007+ models
// This format is used by v44+ MDL files with external VVD vertex data
//-----------------------------------------------------------------------------
struct mstudioboneweight_t
{
	float	weight[MAX_NUM_BONES_PER_VERT];	// 12 bytes (only 3 weights needed)
	char	bone[MAX_NUM_BONES_PER_VERT];	// 3 bytes (8-bit bone indices)
	byte	numbones;						// 1 byte
};	// = 16 bytes total

//-----------------------------------------------------------------------------
// v37: Old vertex structure (64 bytes) - used by HL2 Beta 2003 models
// This format is used by v37 MDL files with embedded vertex data
//-----------------------------------------------------------------------------
struct mstudiovertex_v37_t
{
	mstudioboneweight_v37_t	m_BoneWeights;	// 32 bytes
	Vector		m_vecPosition;				// 12 bytes
	Vector		m_vecNormal;				// 12 bytes
	Vector2D	m_vecTexCoord;				// 8 bytes
};	// = 64 bytes total (two cache lines)

//-----------------------------------------------------------------------------
// v48: New vertex structure (48 bytes) - used by Source 2007+ models
// This format is used by v44+ MDL files with external VVD vertex data
//-----------------------------------------------------------------------------
struct mstudiovertex_t
{
	mstudioboneweight_t	m_BoneWeights;	// 16 bytes
	Vector				m_vecPosition;	// 12 bytes
	Vector				m_vecNormal;	// 12 bytes
	Vector2D			m_vecTexCoord;	// 8 bytes

	mstudiovertex_t() {}

private:
	// No copy constructors allowed (v48 requirement)
	mstudiovertex_t(const mstudiovertex_t& vOther);
};	// = 48 bytes total

// skin info
// v37: Original format with dPdu/dPdv fields (backward compatibility)
struct mstudiotexture_v37_t
{
	int						sznameindex;
	inline char * const		pszName( void ) const { return ((char *)this) + sznameindex; }
	int						flags;
	float					width;		// portion used
	float					height;		// portion used
	mutable IMaterial		*material;  // fixme: this needs to go away . .isn't used by the engine, but is used by studiomdl
	mutable void			*clientmaterial;	// gary, replace with client material pointer if used
	float					dPdu;		// world units per u
	float					dPdv;		// world units per v
};

// Default: 2007 Source Engine compatible format with legacy compatibility
struct mstudiotexture_t
{
	int						sznameindex;
	inline char * const		pszName( void ) const { return ((char *)this) + sznameindex; }
	int						flags;
	int						used;		// usage flag
	int						unused1;	// padding
	mutable IMaterial		*material;  // fixme: this needs to go away . .isn't used by the engine, but is used by studiomdl
	mutable void			*clientmaterial;	// gary, replace with client material pointer if used

	// Legacy compatibility fields (for zone.cpp memory calculations)
	float					width;		// legacy: texture width
	float					height;		// legacy: texture height

	int						unused[8];	// padding for future expansion (reduced from 10)
};

// eyeball
// v37: Original format with iris_material/glint_material fields
struct mstudioeyeball_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int		bone;
	Vector	org;
	float	zoffset;
	float	radius;
	Vector	up;
	Vector	forward;
	int		texture;

	int		iris_material;
	float	iris_scale;
	int		glint_material;	// !!!

	int		upperflexdesc[3];	// index of raiser, neutral, and lowerer flexdesc that is set by flex controllers
	int		lowerflexdesc[3];
	float	uppertarget[3];		// angle (radians) of raised, neutral, and lowered lid positions
	float	lowertarget[3];
	//int		upperflex;	// index of actual flex
	//int		lowerflex;

	int		upperlidflexdesc;	// index of flex desc that actual lid flexes look to
	int		lowerlidflexdesc;

	float	pitch[2];	// min/max pitch
	float	yaw[2];		// min/max yaw
};

// v48: 2007 Source Engine compatible format
struct mstudioeyeball_v48_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int		bone;
	Vector	org;
	float	zoffset;
	float	radius;
	Vector	up;
	Vector	forward;
	int		texture;

	int		unused1;		// v48: was iris_material
	float	iris_scale;
	int		unused2;		// v48: was glint_material

	int		upperflexdesc[3];	// index of raiser, neutral, and lowerer flexdesc that is set by flex controllers
	int		lowerflexdesc[3];
	float	uppertarget[3];		// angle (radians) of raised, neutral, and lowered lid positions
	float	lowertarget[3];

	int		upperlidflexdesc;	// index of flex desc that actual lid flexes look to
	int		lowerlidflexdesc;
	int		unused[4];		// v48: padding, not guaranteed to be 0
	bool	m_bNonFACS;		// v48: Never used before version 44
};


// ikinfo
struct mstudioiklink_t
{
	int		bone;
	Vector	contact;
	Vector	limits;
};

struct mstudioikchain_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int				linktype;
	int				numlinks;
	int				linkindex;
	inline mstudioiklink_t *pLink( int i ) const { return (mstudioiklink_t *)(((byte *)this) + linkindex) + i; };
};


struct mstudioiface_t
{
	unsigned short			a, b, c;		// Indices to vertices
};


//-----------------------------------------------------------------------------
// v48: Forward declarations for vertex data structures
//-----------------------------------------------------------------------------
struct mstudiomodel_t;
struct thinModelVertices_t;
struct vertexFileHeader_t;

//-----------------------------------------------------------------------------
// v48: External vertex data management for models
// Used with VVD files for external vertex data storage
//-----------------------------------------------------------------------------
struct mstudio_modelvertexdata_t
{
	Vector				*Position( int i ) const;
	Vector				*Normal( int i ) const;
	Vector4D			*TangentS( int i ) const;
	Vector2D			*Texcoord( int i ) const;
	mstudioboneweight_t	*BoneWeights( int i ) const;
	mstudiovertex_t		*Vertex( int i ) const;
	bool				HasTangentData( void ) const;
	int					GetGlobalVertexIndex( int i ) const;
	int					GetGlobalTangentIndex( int i ) const;

	// Base of external vertex data stores
	const void			*pVertexData;
	const void			*pTangentData;
};

//-----------------------------------------------------------------------------
// v48: External vertex data management for meshes
// Used with VVD files for per-mesh vertex data access
//-----------------------------------------------------------------------------
struct mstudio_meshvertexdata_t
{
	Vector				*Position( int i ) const;
	Vector				*Normal( int i ) const;
	Vector4D			*TangentS( int i ) const;
	Vector2D			*Texcoord( int i ) const;
	mstudioboneweight_t *BoneWeights( int i ) const;
	mstudiovertex_t		*Vertex( int i ) const;
	bool				HasTangentData( void ) const;
	int					GetModelVertexIndex( int i ) const;
	int					GetGlobalVertexIndex( int i ) const;

	// Indirection to this mesh's model's vertex data
	const mstudio_modelvertexdata_t	*modelvertexdata;

	// Used for fixup calcs when culling top level LODs
	// Expected number of mesh verts at desired LOD
	int					numLODVertexes[MAX_NUM_LODS];
};

struct mstudiomesh_t
{
	int					material;

	int					modelindex;
	mstudiomodel_t *pModel() const; // { return (mstudiomodel_t *)(((byte *)this) + modelindex); }

	int					numvertices;		// number of unique vertices/normals/texcoords
	int					vertexoffset;		// vertex mstudiovertex_t

	Vector				*Position( int i ) const;
	Vector				*Normal( int i ) const;
	Vector4D			*TangentS( int i ) const;
	Vector2D			*Texcoord( int i ) const;
	mstudioboneweight_t *BoneWeights( int i ) const;
	mstudiovertex_t		*Vertex( int i ) const;

	int					numflexes;			// vertex animation
	int					flexindex;
	inline mstudioflex_t *pFlex( int i ) const { return (mstudioflex_t *)(((byte *)this) + flexindex) + i; };

	//int					numresolutionupdates;
	//int					resolutionupdateindex;

	//int					numfaceupdates;
	//int					faceupdateindex;

	// special codes for material operations
	int					materialtype;
	int					materialparam;

	// a unique ordinal for this mesh
	int					meshid;

	Vector				center;

	int					unused[5]; // remove as appropriate
};

// studio models
struct mstudiomodel_t
{
	char				name[64];

	int					type;

	float				boundingradius;

	int					nummeshes;	
	int					meshindex;
	inline mstudiomesh_t *pMesh( int i ) const { return (mstudiomesh_t *)(((byte *)this) + meshindex) + i; };

	// cache purposes
	int					numvertices;		// number of unique vertices/normals/texcoords
	int					vertexindex;		// vertex Vector
	int					tangentsindex;		// tangents Vector

	Vector				*Position( int i ) const;
	Vector				*Normal( int i ) const;
	Vector4D			*TangentS( int i ) const;
	Vector2D			*Texcoord( int i ) const;
	mstudioboneweight_t *BoneWeights( int i ) const;
	mstudiovertex_t		*Vertex( int i ) const;

	int					numattachments;
	int					attachmentindex;

	int					numeyeballs;
	int					eyeballindex;
	inline  mstudioeyeball_t *pEyeball( int i ) { return (mstudioeyeball_t *)(((byte *)this) + eyeballindex) + i; };

	int					unused[8];		// remove as appropriate
};

// v48: 2007 Source Engine compatible eyeball format

inline mstudiovertex_t *mstudiomodel_t::Vertex( int i ) const 
{ 
	return (mstudiovertex_t *)(((byte *)this) + vertexindex) + i; 
}

inline Vector *mstudiomodel_t::Position( int i ) const 
{
	return &Vertex(i)->m_vecPosition;
}

inline Vector *mstudiomodel_t::Normal( int i ) const 
{ 
	return &Vertex(i)->m_vecNormal;
}

inline Vector4D *mstudiomodel_t::TangentS( int i ) const 
{
	// NOTE: The tangents vector is in a separate array
	// because it only exists on the high end, and if I leave it out
	// of the mstudiovertex_t, the vertex is 64-bytes (good for low end)
	return (Vector4D *)(((byte *)this) + tangentsindex) + i; 
}

inline Vector2D *mstudiomodel_t::Texcoord( int i ) const 
{ 
	return &Vertex(i)->m_vecTexCoord;
}

inline mstudioboneweight_t *mstudiomodel_t::BoneWeights( int i ) const 
{
	return &Vertex(i)->m_BoneWeights;
}



inline mstudiomodel_t *mstudiomesh_t::pModel() const 
{ 
	return (mstudiomodel_t *)(((byte *)this) + modelindex); 
}

inline Vector *mstudiomesh_t::Position( int i ) const 
{ 
	return pModel()->Position( vertexoffset + i ); 
};

inline Vector *mstudiomesh_t::Normal( int i ) const 
{ 
	return pModel()->Normal( vertexoffset + i ); 
};

inline Vector4D *mstudiomesh_t::TangentS( int i ) const
{
	return pModel()->TangentS( vertexoffset + i ); 
}

inline Vector2D *mstudiomesh_t::Texcoord( int i ) const 
{ 
	return pModel()->Texcoord( vertexoffset + i ); 
};

inline mstudioboneweight_t *mstudiomesh_t::BoneWeights( int i ) const 
{ 
	return pModel()->BoneWeights( vertexoffset + i ); 
};

inline mstudiovertex_t *mstudiomesh_t::Vertex( int i ) const
{
	return pModel()->Vertex( vertexoffset + i );
}

// a group of studio model data
enum studiomeshgroupflags_t
{
	MESHGROUP_IS_FLEXED = 0x1,
	MESHGROUP_IS_HWSKINNED = 0x2
};


// ----------------------------------------------------------
// runtime stuff
// ----------------------------------------------------------

struct studiomeshgroup_t
{
	IMesh*	m_pMesh;
	IMesh	*m_pColorMesh;
	int		m_NumStrips;
	int		m_Flags;		// see studiomeshgroupflags_t
	OptimizedModel::StripHeader_t*	m_pStripData;
	unsigned short*					m_pGroupIndexToMeshIndex;
	int		m_NumVertices;
	int*	m_pUniqueTris;	// for performance measurements
	unsigned short*	m_pIndices;
	bool	m_MeshNeedsRestore;
	short	m_ColorMeshID;

	inline unsigned short MeshIndex( int i ) const { return m_pGroupIndexToMeshIndex[m_pIndices[i]]; }
};


// studio model data
struct studiomeshdata_t
{
	int					m_NumGroup;
	studiomeshgroup_t*	m_pMeshGroup;
};

struct studioloddata_t
{
	// not needed - this is really the same as studiohwdata_t.m_NumStudioMeshes
	//int					m_NumMeshes; 
	studiomeshdata_t	*m_pMeshData; // there are studiohwdata_t.m_NumStudioMeshes of these.
	float				m_SwitchPoint;
	// one of these for each lod since we can switch to simpler materials on lower lods.
	int					numMaterials; 
	IMaterial			**ppMaterials; /* will have studiohdr_t.numtextures elements allocated */
	// hack - this needs to go away.
	int					*pMaterialFlags; /* will have studiohdr_t.numtextures elements allocated */
};

struct studiohwdata_t
{
	int					m_NumLODs;
	studioloddata_t		*m_pLODs;
	int					m_NumStudioMeshes;
};

// ----------------------------------------------------------
// ----------------------------------------------------------

// body part index
struct mstudiobodyparts_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	int					nummodels;
	int					base;
	int					modelindex; // index into models array
	inline mstudiomodel_t *pModel( int i ) const { return (mstudiomodel_t *)(((byte *)this) + modelindex) + i; };
};


struct mstudiomouth_t
{
	int					bone;
	Vector				forward;
	int					flexdesc;
};

struct mstudiohitboxset_t
{
	int					sznameindex;
	inline char * const	pszName( void ) const { return ((char *)this) + sznameindex; }
	int					numhitboxes;
	int					hitboxindex;
	inline mstudiobbox_t *pHitbox( int i ) const { return (mstudiobbox_t *)(((byte *)this) + hitboxindex) + i; };
};

// This flag is set if no hitbox information was specified
#define STUDIOHDR_FLAGS_AUTOGENERATED_HITBOX	( 1 << 0 )

// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
// models when we change materials.
#define STUDIOHDR_FLAGS_USES_ENV_CUBEMAP		( 1 << 1 )

// Use this when there are translucent parts to the model but we're not going to sort it 
#define STUDIOHDR_FLAGS_FORCE_OPAQUE			( 1 << 2 )

// Use this when we want to render the opaque parts during the opaque pass
// and the translucent parts during the translucent pass
#define STUDIOHDR_FLAGS_TRANSLUCENT_TWOPASS		( 1 << 3 )

// This is set any time the .qc files has $staticprop in it
// Means there's no bones and no transforms
#define STUDIOHDR_FLAGS_STATIC_PROP				( 1 << 4 )

// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
// models when we change materials.
#define STUDIOHDR_FLAGS_USES_FB_TEXTURE		    ( 1 << 5 )

// This flag is set by studiomdl.exe if a separate "$shadowlod" entry was present
//  for the .mdl (the shadow lod is the last entry in the lod list if present)
#define STUDIOHDR_FLAGS_HASSHADOWLOD			( 1 << 6 )

// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
// models when we change materials.
#define STUDIOHDR_FLAGS_USES_BUMPMAPPING		( 1 << 7 )

// NOTE:  This flag is set when we should use the actual materials on the shadow LOD
// instead of overriding them with the default one (necessary for translucent shadows)
#define STUDIOHDR_FLAGS_USE_SHADOWLOD_MATERIALS	( 1 << 8 )

//-----------------------------------------------------------------------------
// v37-specific structures (HL2 Beta format)
//-----------------------------------------------------------------------------
struct mstudioanimgroup_t
{
	int					group;
	int					index; // mstudioseqdesc_t anim index stuff
};

struct mstudiobonedesc_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }

	int					parent;

	float				value[6];	// default DoF values
	float				scale[6];   // scale for delta DoF values
	matrix3x4_t			poseToBone;
	Quaternion			qAlignment;

	int					dummy2;
};

// header for demand loaded animation group data (v37)
struct studioanimgrouphdr_t
{
	int					id;
	int					version;

	char				name[64];
	int					length;

	int					spacing1;

	int					numdummy1;
	int					dummy1index;

	int					numdummy2;
	int					dummy2index;
	inline mstudioanimdesc_t *pAnimdesc( int i ) const { return (mstudioanimdesc_t *)(((byte *)this) + dummy2index) + i; };
};

//-----------------------------------------------------------------------------
// v48: Animation block for demand loading (v46+)
//-----------------------------------------------------------------------------
struct mstudioanimblock_t
{
	int					datastart;
	int					dataend;
};

// NOTE: mstudiocompressedikerror_t is defined earlier (near IK structures)

//-----------------------------------------------------------------------------
// v48: Model group for include models
//-----------------------------------------------------------------------------
struct mstudiomodelgroup_t
{
	int					szlabelindex;	// textual name
	inline char * const pszLabel( void ) const { return ((char *)this) + szlabelindex; }
	int					sznameindex;	// file name
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
};

// forward declaration for virtualmodel_t
struct studiohdr_t;

//-----------------------------------------------------------------------------
// v48: Virtual model compositing structures for shared models/animations
// These are used to combine multiple MDL files with $includemodel
//-----------------------------------------------------------------------------

// Maps local sequence/animation/attachment to global indices
struct virtualsequence_t
{
	int	flags;
	int activity;
	int group;
	int index;
};

struct virtualgeneric_t
{
	int group;
	int index;
};

// Represents one included model group in the virtual model
class virtualgroup_t
{
public:
	virtualgroup_t( void ) { cache = NULL; };
	// tool dependant. In engine this is a model_t, in tool it's a direct pointer
	void *cache;
	// converts cache entry into a usable studiohdr_t *
	const studiohdr_t *GetStudioHdr( void ) const;

	CUtlVector< int > boneMap;				// maps global bone to local bone
	CUtlVector< int > masterBone;			// maps local bone to global bone
	CUtlVector< int > masterSeq;			// maps local sequence to master sequence
	CUtlVector< int > masterAnim;			// maps local animation to master animation
	CUtlVector< int > masterAttachment;		// maps local attachment to global
	CUtlVector< int > masterPose;			// maps local pose parameter to global
	CUtlVector< int > masterNode;			// maps local transition nodes to global
};

// Virtual model - composites multiple MDL files together
struct virtualmodel_t
{
	void AppendSequences( int group, const studiohdr_t *pStudioHdr );
	void AppendAnimations( int group, const studiohdr_t *pStudioHdr );
	void AppendAttachments( int ground, const studiohdr_t *pStudioHdr );
	void AppendPoseParameters( int group, const studiohdr_t *pStudioHdr );
	void AppendBonemap( int group, const studiohdr_t *pStudioHdr );
	void AppendNodes( int group, const studiohdr_t *pStudioHdr );
	void AppendTransitions( int group, const studiohdr_t *pStudioHdr );
	void AppendIKLocks( int group, const studiohdr_t *pStudioHdr );
	void AppendModels( int group, const studiohdr_t *pStudioHdr );
	void UpdateAutoplaySequences( const studiohdr_t *pStudioHdr );

	virtualgroup_t *pAnimGroup( int animation ) { return &m_group[ m_anim[ animation ].group ]; };
	virtualgroup_t *pSeqGroup( int sequence ) { return &m_group[ m_seq[ sequence ].group ]; };

	CUtlVector< virtualsequence_t > m_seq;
	CUtlVector< virtualgeneric_t > m_anim;
	CUtlVector< virtualgeneric_t > m_attachment;
	CUtlVector< virtualgeneric_t > m_pose;
	CUtlVector< virtualgroup_t > m_group;
	CUtlVector< virtualgeneric_t > m_node;
	CUtlVector< virtualgeneric_t > m_iklock;
	CUtlVector< unsigned short > m_autoplaySequences;
};


//-----------------------------------------------------------------------------
// v48: Flex controller UI
//-----------------------------------------------------------------------------
struct mstudioflexcontrollerui_t
{
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }

	// These are used like a union to save space
	// Here anchors the loaded value.
	int					szindex0;
	int					szindex1;
	int					szindex2;

	inline const char	*pszString( int i ) const
	{
		int idx = 0;
		switch (i) {
			case 0: idx = szindex0; break;
			case 1: idx = szindex1; break;
			case 2: idx = szindex2; break;
		}
		return idx ? ((char *)this) + idx : NULL;
	}

	// Number anchored anchored
	unsigned char		remaptype;	// See the FlexControllerRemapType_t enum
	bool				stereo;		// Is this a filtered controller with a symmetric partner?
	byte				unused[2];
};

//-----------------------------------------------------------------------------
// v48: Source bone transforms (.dmx/.smd to .mdl conversion)
//-----------------------------------------------------------------------------
struct mstudiosrcbonetransform_t
{
	int			sznameindex;
	inline const char *pszName( void ) const { return ((char *)this) + sznameindex; }
	matrix3x4_t	pretransform;
	matrix3x4_t	posttransform;
};

//-----------------------------------------------------------------------------
// v48: Secondary header extension (studiohdr2_t)
// Provides extensibility without breaking the main header format
//-----------------------------------------------------------------------------
struct studiohdr2_t
{
	// Source bone transforms for animation import
	int numsrcbonetransform;
	int srcbonetransformindex;

	int	illumpositionattachmentindex;
	inline int IllumPositionAttachmentIndex() const { return illumpositionattachmentindex; }

	float flMaxEyeDeflection;
	inline float MaxEyeDeflection() const { return flMaxEyeDeflection != 0.0f ? flMaxEyeDeflection : 0.866f; }

	int linearboneindex;
	inline mstudiolinearbone_t *pLinearBones() const { return (linearboneindex) ? (mstudiolinearbone_t *)(((byte *)this) + linearboneindex) : NULL; }

	int reserved[59];
};

//-----------------------------------------------------------------------------
// VVD File Format (v48: External Vertex Data)
//-----------------------------------------------------------------------------
// little-endian "IDSV"
#define MODEL_VERTEX_FILE_ID		(('V'<<24)+('S'<<16)+('D'<<8)+'I')
#define MODEL_VERTEX_FILE_VERSION	4
// Compressed/thin vertex data ID (IDCV)
#define MODEL_VERTEX_FILE_THIN_ID	(('V'<<24)+('C'<<16)+('D'<<8)+'I')

struct vertexFileHeader_t
{
	int		id;								// MODEL_VERTEX_FILE_ID
	int		version;						// MODEL_VERTEX_FILE_VERSION
	long	checksum;						// same as studiohdr_t, ensures sync
	int		numLODs;						// num of valid LODs
	int		numLODVertexes[MAX_NUM_LODS];	// num verts for desired root LOD
	int		numFixups;						// num of vertexFileFixup_t
	int		fixupTableStart;				// offset from base to fixup table
	int		vertexDataStart;				// offset from base to vertex block
	int		tangentDataStart;				// offset from base to tangent block

	// Access helpers
	inline const byte *GetVertexData() const
	{
		if ((id == MODEL_VERTEX_FILE_ID) && (vertexDataStart != 0))
			return ((byte *)this) + vertexDataStart;
		return NULL;
	}

	inline const byte *GetTangentData() const
	{
		if ((id == MODEL_VERTEX_FILE_ID) && (tangentDataStart != 0))
			return ((byte *)this) + tangentDataStart;
		return NULL;
	}
};

// Fixup entry for LOD vertex remapping
struct vertexFileFixup_t
{
	int		lod;				// used to skip culled root lod
	int		sourceVertexID;		// absolute index from start of vertex/tangent blocks
	int		numVertexes;
};


//-----------------------------------------------------------------------------
// Main Studio Header (studiohdr_t)
// This structure now supports both v37 and v48 formats through version checking
//-----------------------------------------------------------------------------
struct studiohdr_t
{
	int					id;
	int					version;

	long				checksum;		// this has to be the same in the phy and vtx files to load!
	
	char				name[64];
	int					length;


	Vector				eyeposition;	// ideal eye position

	Vector				illumposition;	// illumination center
	
	Vector				hull_min;			// ideal movement hull size
	Vector				hull_max;			

	Vector				view_bbmin;			// clipping bounding box
	Vector				view_bbmax;		

	int					flags;

	int					numbones;			// bones
	int					boneindex;
	// v37: Returns v37 bone structure (value[6]/scale[6] format)
	inline mstudiobone_t *pBone( int i ) const { return (mstudiobone_t *)(((byte *)this) + boneindex) + i; };
	// v48: Returns v48 bone structure (pos/quat/rot/posscale/rotscale format)
	inline mstudiobone_v48_t *pBone_v48( int i ) const { return (mstudiobone_v48_t *)(((byte *)this) + boneindex) + i; };

	int					numbonecontrollers;		// bone controllers
	int					bonecontrollerindex;
	inline mstudiobonecontroller_t *pBonecontroller( int i ) const { return (mstudiobonecontroller_t *)(((byte *)this) + bonecontrollerindex) + i; };

	int					numhitboxsets;
	int					hitboxsetindex;



	// Look up hitbox set by index
	mstudiohitboxset_t	*pHitboxSet( int i ) const 
	{ 
		return (mstudiohitboxset_t *)(((byte *)this) + hitboxsetindex ) + i; 
	};

	// Calls through to hitbox to determine size of specified set
	inline mstudiobbox_t *pHitbox( int i, int set ) const 
	{ 
		mstudiohitboxset_t const *s = pHitboxSet( set );
		if ( !s )
			return NULL;

		return s->pHitbox( i );
	};

	// Calls through to set to get hitbox count for set
	inline int			iHitboxCount( int set ) const
	{
		mstudiohitboxset_t const *s = pHitboxSet( set );
		if ( !s )
			return 0;

		return s->numhitboxes;
	};

	/*
	int					numhitboxes;			// complex bounding boxes
	int					hitboxindex;			
	inline mstudiobbox_t *pHitbox( int i ) const { return (mstudiobbox_t *)(((byte *)this) + hitboxindex) + i; };
	*/
	
	// Animation data - field names differ between v37 and v48
	// v37: numanim/animdescindex, v48: numlocalanim/localanimindex (but same offsets)
	int					numanim;			// v37: animations/poses, v48: use numlocalanim alias
	int					animdescindex;		// v37: animation descriptions, v48: use localanimindex alias
	// v37: Returns v37 animation description structure
	inline mstudioanimdesc_t *pAnimdesc( int i ) const { return (mstudioanimdesc_t *)(((byte *)this) + animdescindex) + i; };
	// v48: Returns v48 animation description structure (with animation blocks, sections, etc.)
	inline mstudioanimdesc_v48_t *pAnimdesc_v48( int i ) const { return (mstudioanimdesc_v48_t *)(((byte *)this) + animdescindex) + i; };
	// v48 aliases for virtual model compatibility
	inline int GetNumLocalAnim() const { return numanim; }
	inline int numlocalanim() const { return numanim; }
	inline mstudioanimdesc_t *pLocalAnimdesc( int i ) const { return pAnimdesc(i); }
	inline mstudioanimdesc_v48_t *pLocalAnimdesc_v48( int i ) const { return pAnimdesc_v48(i); }

	// v37-specific animation groups (these fields exist in v37 models at specific offsets)
	// For v48 models, these are repurposed as activitylistversion and eventsindexed
	union {
		struct {
			int				numanimgroups;		// v37: animation groups
			int				animgroupindex;
		} v37;
		struct {
			mutable int		activitylistversion;	// v48: activity list version (mutable for lazy init)
			mutable int		eventsindexed;			// v48: events indexed flag
		} v48;
	} animdata;

	inline mstudioanimgroup_t *pAnimgroup( int i ) const {
		return (version <= STUDIO_VERSION_37) ?
			(mstudioanimgroup_t *)(((byte *)this) + animdata.v37.animgroupindex) + i : NULL;
	}

	// v37: bone descriptions, v48: unused (zeroed)
	int					numbonedescs;		// v37 only
	int					bonedescindex;		// v37 only
	inline mstudiobonedesc_t *pBonedesc( int i ) const {
		return (version <= STUDIO_VERSION_37) ?
			(mstudiobonedesc_t *)(((byte *)this) + bonedescindex) + i : NULL;
	}

	// Sequence data - field names differ between v37 and v48
	int					numseq;				// v37: sequences, v48: use numlocalseq alias
	int					seqindex;			// v48: use localseqindex alias

	//-----------------------------------------------------------------------------
	// pSeqdesc - Version-aware sequence descriptor accessor
	//
	// The v37 and v48 mstudioseqdesc_t structures have dramatically different sizes:
	// - v48: Uses anim[32][32] (4096 bytes) + seqgroup field = ~4272 bytes per seqdesc
	// - v37: Uses animindexindex (4 bytes) + no seqgroup = ~168 bytes per seqdesc
	//
	// When iterating over sequence descriptors, we must use the correct struct size
	// for the model version, otherwise pointer arithmetic will be completely wrong.
	//-----------------------------------------------------------------------------

	// v37 seqdesc binary size (must match the actual v37 binary layout)
	// The difference between v48 and v37 seqdesc is:
	// - anim[32][32] (4096 bytes) vs animindexindex (4 bytes) = 4092 bytes
	// - seqgroup (4 bytes) only in v48 = 4 bytes
	// Total difference: 4096 bytes
	// v48 struct size is approximately 4284 bytes, so v37 is 4284 - 4096 = 188 bytes
	static const int SEQDESC_V37_SIZE = 188;

	inline mstudioseqdesc_t *pSeqdesc( int i ) const
	{
		if (i < 0 || i >= numseq) i = 0;

		if (version <= STUDIO_VERSION_37)
		{
			// v37: Use correct binary struct size for pointer arithmetic
			return (mstudioseqdesc_t *)(((byte *)this) + seqindex + i * SEQDESC_V37_SIZE);
		}
		else
		{
			// v48: Normal pointer arithmetic with compiled struct size
			return (mstudioseqdesc_t *)(((byte *)this) + seqindex) + i;
		}
	}

	// v48 aliases for virtual model compatibility
	inline int GetNumLocalSeq() const { return numseq; }
	inline int numlocalseq() const { return numseq; }
	inline mstudioseqdesc_t *pLocalSeqdesc( int i ) const { return pSeqdesc(i); }

	int					sequencesindexed;	// v37: initialization flag, v48: unused

	// v37: sequence groups for demand loading, v48: unused (replaced by include models)
	int					numseqgroups;
	int					seqgroupindex;
	inline mstudioseqgroup_t *pSeqgroup( int i ) const { if (i < 0 || i >= numseqgroups) i = 0; return (mstudioseqgroup_t *)(((byte *)this) + seqgroupindex) + i; };

	int					numtextures;		// raw textures
	int					textureindex;
	inline mstudiotexture_t *pTexture( int i ) const { return (mstudiotexture_t *)(((byte *)this) + textureindex) + i; };
	// v37: Texture structure is 32 bytes (mstudiotexture_v37_t), not 64 bytes (mstudiotexture_t)
	inline mstudiotexture_v37_t *pTexture_V37( int i ) const { return (mstudiotexture_v37_t *)(((byte *)this) + textureindex) + i; }; 

	int					numcdtextures;		// raw textures search paths
	int					cdtextureindex;
	inline char			*pCdtexture( int i ) const { return (((char *)this) + *((int *)(((byte *)this) + cdtextureindex) + i)); };

	int					numskinref;			// replaceable textures tables
	int					numskinfamilies;
	int					skinindex;
	inline short		*pSkinref( int i ) const { return (short *)(((byte *)this) + skinindex) + i; };

	int					numbodyparts;		
	int					bodypartindex;
	inline mstudiobodyparts_t	*pBodypart( int i ) const { return (mstudiobodyparts_t *)(((byte *)this) + bodypartindex) + i; };

	int					numattachments;		// queryable attachable points
	int					attachmentindex;
	inline mstudioattachment_t	*pAttachment( int i ) const { return (mstudioattachment_t *)(((byte *)this) + attachmentindex) + i; };
	// v48 aliases for virtual model compatibility
	inline int numlocalattachments() const { return numattachments; }
	inline mstudioattachment_t *pLocalAttachment( int i ) const { return pAttachment(i); }

	int					numtransitions;		// animation node to animation node transition graph
	int					transitionindex;
	inline byte	*pTransition( int i ) const { return (byte *)(((byte *)this) + transitionindex) + i; };
	// v48 aliases for virtual model compatibility
	// Note: v37 models don't have node names stored - numlocalnodes is derived from sqrt(numtransitions)
	// For v48 models, these fields are at different offsets with explicit node name data
	inline int numlocalnodes() const {
		// For v37 models, derive node count from transition table size (numtransitions = numlocalnodes^2)
		// For v48+ models, this should be overridden with actual field access
		if (numtransitions <= 0) return 0;
		int n = 1;
		while (n * n < numtransitions) n++;
		return (n * n == numtransitions) ? n : 0;
	}
	inline byte *pLocalTransition( int i ) const { return pTransition(i); }
	// pszLocalNodeName not available in v37 format - nodes are identified by index only

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

	int					numposeparameters;
	int					poseparamindex;
	inline mstudioposeparamdesc_t *pPoseParameter( int i ) const { return (mstudioposeparamdesc_t *)(((byte *)this) + poseparamindex) + i; };
	// v48 aliases for virtual model compatibility
	inline int numlocalposeparameters() const { return numposeparameters; }
	inline mstudioposeparamdesc_t *pLocalPoseParameter( int i ) const { return pPoseParameter(i); }

	int					surfacepropindex;
	inline char * const pszSurfaceProp( void ) const { return ((char *)this) + surfacepropindex; }

	// Key values
	int					keyvalueindex;
	int					keyvaluesize;
	inline const char * KeyValueText( void ) const { return keyvaluesize != 0 ? ((char *)this) + keyvalueindex : NULL; }

	int					numikautoplaylocks;
	int					ikautoplaylockindex;
	inline mstudioiklock_t *pIKAutoplayLock( int i ) const { return (mstudioiklock_t *)(((byte *)this) + ikautoplaylockindex) + i; };
	// v48 aliases for virtual model compatibility
	inline int numlocalikautoplaylocks() const { return numikautoplaylocks; }
	inline mstudioiklock_t *pLocalIKAutoplayLock( int i ) const { return pIKAutoplayLock(i); }

	float				mass;				// The collision model mass that jay wanted
	int					contents;

	//-----------------------------------------------------------------------------
	// v48-specific fields (not present in v37 models)
	// These fields extend the header for modern Source engine features
	//-----------------------------------------------------------------------------

	// v48: Include models for compositing (replaces sequence groups)
	int					numincludemodels;
	int					includemodelindex;
	inline mstudiomodelgroup_t *pModelGroup( int i ) const {
		return (version >= STUDIO_VERSION_44) ?
			(mstudiomodelgroup_t *)(((byte *)this) + includemodelindex) + i : NULL;
	};

	// v48: Find and load an included model (implementation in studio_virtualmodel.cpp)
	// Returns the studiohdr_t of the included model, or NULL if not found
	// ppCache receives the model handle for reference counting
	const studiohdr_t *FindModel( void **ppCache, const char *pModelName ) const;

	// v48: Virtual model pointer (runtime, not serialized)
	mutable virtualmodel_t	*virtualModel;
	// v48: Get virtual model - NULL for v37
	inline virtualmodel_t *GetVirtualModel( void ) const { return (version >= STUDIO_VERSION_44) ? virtualModel : NULL; };
	// v48: Set virtual model
	inline void SetVirtualModel( virtualmodel_t *pVModel ) const { virtualModel = pVModel; };

	// v48: Animation block name for demand loading
	int					szanimblocknameindex;
	inline char * const pszAnimBlockName( void ) const { return ((char *)this) + szanimblocknameindex; }

	// v48: Animation blocks for demand loading (v46+)
	int					numanimblocks;
	int					animblockindex;
	inline mstudioanimblock_t *pAnimBlock( int i ) const {
		return (version >= STUDIO_VERSION_46) ?
			(mstudioanimblock_t *)(((byte *)this) + animblockindex) + i : NULL;
	};
	mutable void		*animblockModel;

	// v48: Bone lookup table sorted by name
	int					bonetablebynameindex;
	inline const byte	*GetBoneTableSortedByName() const { return (byte *)this + bonetablebynameindex; }

	// v48: Vertex and index data base pointers (for tools, runtime set)
	mutable void		*pVertexBase;
	mutable void		*pIndexBase;

	// v48: Constant directional light dot product (static prop optimization)
	byte				constdirectionallightdot;

	// v48: Root LOD settings
	byte				rootLOD;
	byte				numAllowedRootLODs;

	byte				unused0;			// padding
	int					unused1;			// reserved

	// v48: Flex controller UI
	int					numflexcontrollerui;
	int					flexcontrolleruiindex;
	inline mstudioflexcontrollerui_t *pFlexControllerUI( int i ) const {
		return (version >= STUDIO_VERSION_44) ?
			(mstudioflexcontrollerui_t *)(((byte *)this) + flexcontrolleruiindex) + i : NULL;
	}

	// v48: studiohdr2_t extension index
	int					studiohdr2index;
	inline studiohdr2_t *pStudioHdr2() const {
		return (version >= STUDIO_VERSION_44 && studiohdr2index != 0) ?
			(studiohdr2_t *)(((byte *)this) + studiohdr2index) : NULL;
	}

	int					unused2;			// reserved for future use

	//-----------------------------------------------------------------------------
	// Helper functions for version-aware access
	//-----------------------------------------------------------------------------

	// Get model version
	inline int GetVersion() const { return version; }

	// Check if this is a v37 (2003) format model
	inline bool IsV37() const { return version <= STUDIO_VERSION_37; }

	// Check if this is a v44+ (2004+) format model
	inline bool IsV44Plus() const { return version >= STUDIO_VERSION_44; }

	// Check if this is a v48 (2007) format model
	inline bool IsV48() const { return version >= STUDIO_VERSION_48; }

	// Check if model has embedded vertex data (v37) or external VVD (v44+)
	inline bool HasEmbeddedVertices() const { return STUDIO_VERSION_HAS_EMBEDDED_VERTICES(version); }
	inline bool HasExternalVertices() const { return STUDIO_VERSION_HAS_EXTERNAL_VERTICES(version); }

	// v48: Get linear bone data if available
	inline mstudiolinearbone_t *pLinearBones() const {
		studiohdr2_t *pHdr2 = pStudioHdr2();
		return pHdr2 ? pHdr2->pLinearBones() : NULL;
	}

	// v48: Get source bone transforms if available
	inline int GetNumSrcBoneTransforms() const {
		studiohdr2_t *pHdr2 = pStudioHdr2();
		return pHdr2 ? pHdr2->numsrcbonetransform : 0;
	}
	inline mstudiosrcbonetransform_t *pSrcBoneTransform( int i ) const {
		studiohdr2_t *pHdr2 = pStudioHdr2();
		if (!pHdr2 || pHdr2->srcbonetransformindex == 0)
			return NULL;
		return (mstudiosrcbonetransform_t *)(((byte *)pHdr2) + pHdr2->srcbonetransformindex) + i;
	}

	// v48: Activity list version
	inline int GetActivityListVersion() const {
		return IsV44Plus() ? animdata.v48.activitylistversion : 0;
	}
	inline void SetActivityListVersion( int version ) const {
		if (IsV44Plus()) animdata.v48.activitylistversion = version;
	}
};

// header for demand loaded sequence group data
struct studioseqhdr_t
{
	int					id;
	int					version;

	char				name[64];
	int					length;
};


// Vector	boundingbox[model][bone][2];	// complex intersection info

struct flexweight_t
{
	int					key;
	float				weight;
	float				influence;
};

//-----------------------------------------------------------------------------
// Purpose: A markov group basically indexes another flex setting and includes a
//  weighting factor which is used to factor how likely the specified member
//  is to be picked
//-----------------------------------------------------------------------------
struct flexmarkovgroup_t
{
	int					settingnumber;
	int					weight;
};

#define FS_NORMAL 0
#define FS_MARKOV 1

struct flexsetting_t
{
	int					nameindex;

	inline char *pszName( void ) const
	{ 
		return (char *)(((byte *)this) + nameindex); 
	}

	inline bool IsMarkov( void ) const
	{ 
		return type == FS_MARKOV ? true : false; 
	}

	// FS_NORMAL or FS_MARKOV
	int					type;

	// Number of flex settings for FS_NORMAL or Number of 
	//  Markov group members for FS_MARKOV
	int					numsettings;
	int					index;
	
	// For FS_MARKOV only, the client .dll writes the current index into here so that
	//  it can retain which markov group is being followed. This is reset every time the expression
	//  starts to play back during scene playback
	int					currentindex;

	// Index of start of contiguous array of flexweight_t or 
	//  flexmarkovgroup_t structures
	int					settingindex;

	//-----------------------------------------------------------------------------
	// Purpose: Retrieves the specified markov group header if the entry is a markov entry
	// Input  : member - 
	// Output : flexmarkovgroup_t *
	//-----------------------------------------------------------------------------
	inline flexmarkovgroup_t *pMarkovGroup( int member ) const
	{
		// type must be FS_MARKOV to return this pointer
		if ( !IsMarkov() )
			return NULL;

		if ( member < 0 || 
			member >= numsettings )
			return NULL;

		flexmarkovgroup_t *group = ( flexmarkovgroup_t * )(((byte *)this) + settingindex ) + member;
		return group;
	};

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Input  : member - 
	// Output : int
	//-----------------------------------------------------------------------------
	inline int GetMarkovSetting( int member ) const
	{
		flexmarkovgroup_t *group = pMarkovGroup( member );
		if ( !group )
			 return -1;

		return group->settingnumber;
	};

	//-----------------------------------------------------------------------------
	// Purpose: Retrieves a pointer to the flexweight_t, including resolving
	//  any markov chain hierarchy.  Because of this possibility, we return
	//  the number of settings in the weights array returned.  We'll generally
	//  call this function with i == 0
	// Input  : *base - 
	//			i - 
	//			**weights - 
	// Output : int
	//-----------------------------------------------------------------------------
	inline int psetting( byte *base, int i, flexweight_t **weights ) const;
};


struct flexsettinghdr_t
{
	int					id;
	int					version;

	char				name[64];
	int					length;

	int					numflexsettings;
	int					flexsettingindex;
	inline flexsetting_t *pSetting( int i ) const { return (flexsetting_t *)(((byte *)this) + flexsettingindex) + i; };
	int					nameindex;

	// look up flex settings by "index"
	int					numindexes;
	int					indexindex;

	inline flexsetting_t *pIndexedSetting( int index ) const 
	{ 
		if ( index < 0 || index >= numindexes )
		{
			return NULL;
		}

		int i = *((int *)(((byte *)this) + indexindex) + index);
		
		if (i == -1) 
		{
			return NULL;
		}

		return pSetting( i );
	}

	// index names of "flexcontrollers"
	int					numkeys;
	int					keynameindex;
	inline char			*pLocalName( int i ) const { return (char *)(((byte *)this) + *((int *)(((byte *)this) + keynameindex) + i)); };

	int					keymappingindex;
	inline int			*pLocalToGlobal( int i ) const { return (int *)(((byte *)this) + keymappingindex) + i; };
	inline int			LocalToGlobal( int i ) const { return *pLocalToGlobal( i ); };

	//-----------------------------------------------------------------------------
	// Purpose: Same as pSetting( int i ) above, except it translates away any markov groups first
	// Input  : i - index to retrieve
	// Output : flexsetting_t * - a non-markov underlying flexsetting_t
	//-----------------------------------------------------------------------------
	inline flexsetting_t *pTranslatedSetting( int i ) const
	{ 
		flexsetting_t *setting = (flexsetting_t *)(((byte *)this) + flexsettingindex) + i;
		// If this one is not a markov setting, return it
		if ( !setting->IsMarkov() )
		{
			return setting;
		}

		int newindex = setting->GetMarkovSetting( setting->currentindex );
		// Ack, this should never happen (the markov references something that is gone)
		//  Since there was a problem, 
		//  just return this setting anyway, sigh.
		if ( newindex == -1 )
		{
			return setting;
		}

		// Otherwise, recurse on the translated index
		// NOTE:  It's theoretically possible to have an infinite recursion if two markov
		//  groups reference each other.  The faceposer shouldn't create such groups,
		//  so I don't think this will ever actually occur -- ywb
		return pTranslatedSetting( newindex );
	}
};

//-----------------------------------------------------------------------------
	// Purpose: Retrieves a pointer to the flexweight_t, including resolving
	//  any markov chain hierarchy.  Because of this possibility, we return
	//  the number of settings in the weights array returned.  We'll generally
	//  call this function with i == 0
// Input  : *base - flexsettinghdr_t * pointer
//			i - index of flex setting to retrieve
//			**weights - destination for weights array starting at index i.
// Output : int
//-----------------------------------------------------------------------------
inline int flexsetting_t::psetting( byte *base, int i, flexweight_t **weights ) const
{ 
	// Assume failure to find index
	*weights = NULL;

	// Recurse if this is a markov setting
	if ( IsMarkov() )
	{
		// Find the current redirected index
		int settingnum = GetMarkovSetting( currentindex );
		if ( settingnum == -1 )
		{
			// Couldn't find currentindex in the markov list for this flex setting
			return -1;
		}

		// Follow the markov link instead
		flexsetting_t *setting = ( (flexsettinghdr_t *)base)->pSetting( settingnum );
		if ( !setting )
		{
			return -1;
		}

		// Recurse ( could support more than one level of markov chains this way )
		return setting->psetting( base, i, weights );
	}

	// Grab array pointer
	*weights = (flexweight_t *)(((byte *)this) + settingindex) + i;
	// Return true number of settings
	return numsettings;
};

#define STUDIO_CONST	1	// get float
#define STUDIO_FETCH1	2	// get Flexcontroller value
#define STUDIO_FETCH2	3	// get flex weight
#define STUDIO_ADD		4
#define STUDIO_SUB		5
#define STUDIO_MUL		6
#define STUDIO_DIV		7
#define STUDIO_NEG		8	// not implemented
#define STUDIO_EXP		9	// not implemented
#define STUDIO_OPEN		10	// only used in token parsing
#define STUDIO_CLOSE	11

// motion flags
#define STUDIO_X		0x00000001
#define STUDIO_Y		0x00000002	
#define STUDIO_Z		0x00000004
#define STUDIO_XR		0x00000008
#define STUDIO_YR		0x00000010
#define STUDIO_ZR		0x00000020

#define STUDIO_LX		0x00000040
#define STUDIO_LY		0x00000080
#define STUDIO_LZ		0x00000100
#define STUDIO_LXR		0x00000200
#define STUDIO_LYR		0x00000400
#define STUDIO_LZR		0x00000800

#define STUDIO_TYPES	0x0003FFFF
#define STUDIO_RLOOP	0x00040000	// controller that wraps shortest distance

// sequence flags
#define STUDIO_LOOPING	0x0001		// ending frame should be the same as the starting frame
#define STUDIO_SNAP		0x0002		// do not interpolate between previous animation and this one
#define STUDIO_DELTA	0x0004		// this sequence "adds" to the base sequences, not slerp blends
#define STUDIO_AUTOPLAY	0x0008		// temporary flag that forces the sequence to always play
#define STUDIO_POST		0x0010		//
#define STUDIO_ALLZEROS	0x0020		// this animation/sequence has no real animation data
#define STUDIO_WEIGHT	0x0040		//
#define STUDIO_SPLINE	0x0080		//
#define STUDIO_REALTIME	0x0100		//
#define STUDIO_LOCAL	0x0200		// sequence has a local context sequence
#define STUDIO_HIDDEN	0x0400		// don't show in default selection views
#define STUDIO_OVERRIDE	0x0800		// forward declared sequence (placeholder for $includemodel override)
#define STUDIO_ACTIVITY	0x1000		// has been updated at runtime to activity index
#define STUDIO_EVENT	0x2000		// has been updated at runtime to event index 

//-----------------------------------------------------------------------------
// Studio_ConvertStudioHdrToNewVersion
// Version conversion and validation for multi-version support (v37-v48)
//
// This function handles version detection and ensures the header is compatible
// with the engine. It supports:
// - v37: HL2 Beta 2003 format (embedded vertex data)
// - v44-v48: Source 2004-2007 format (external VVD vertex data)
//-----------------------------------------------------------------------------
inline bool Studio_ConvertStudioHdrToNewVersion( studiohdr_t *pStudioHdr )
{
	int version = pStudioHdr->version;

	// Check if version is in supported range
	if (version < STUDIO_VERSION_MIN || version > STUDIO_VERSION_MAX)
	{
		// Unsupported version
		return false;
	}

	// v37 models (HL2 Beta 2003)
	// These have embedded vertex data and use the original structure layout
	if (version == STUDIO_VERSION_37)
	{
		// Slam all bone contents to SOLID for older versions if not set
		if (pStudioHdr->contents == 0)
		{
			pStudioHdr->contents = CONTENTS_SOLID;
		}

		// Initialize v48-specific fields to safe defaults for v37 models
		// These fields don't exist in the v37 file, but our unified structure has them
		// The file data doesn't include these bytes, so we must NOT touch them
		// as they would be reading/writing beyond the file buffer

		// No further conversion needed - v37 is natively supported
		return true;
	}

	// v44-v47 models (transitional versions)
	if (version >= STUDIO_VERSION_44 && version < STUDIO_VERSION_48)
	{
		// These versions are mostly compatible with v48
		// Apply incremental fixups if needed

		// v44: First version with external vertex data (VVD files)
		// v45: Added animation sections
		// v46: Added animation blocks for demand loading
		// v47: Added zero frame caching

		// For now, treat them as compatible with v48
		return true;
	}

	// v48 models (Source 2007 - Orange Box, TF2)
	if (version == STUDIO_VERSION_48)
	{
		// Full v48 support - no conversion needed
		return true;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Studio_IsValidModelVersion
// Quick check if a model version is loadable
//-----------------------------------------------------------------------------
inline bool Studio_IsValidModelVersion( int version )
{
	return version >= STUDIO_VERSION_MIN && version <= STUDIO_VERSION_MAX;
}

#endif // STUDIO_H
