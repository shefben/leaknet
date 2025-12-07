//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Defines and structures for the BSP file format.
//
// $NoKeywords: $
//=============================================================================

#ifndef BSPFILE_H
#define BSPFILE_H
#pragma once

#ifndef MATHLIB_H
#include "mathlib.h"
#endif

#include "bumpvects.h"

#define IDBSPHEADER	(('P'<<24)+('S'<<16)+('B'<<8)+'V')
		// little-endian "VBSP"

//-----------------------------------------------------------------------------
// BSP Version Support
// This engine supports multiple BSP versions for compatibility:
// - v18: Original HL2 Beta 2003 (LeakNet native)
// - v19: HL2 Release 2004 (minor changes)
// - v20: Source 2006/2007 with HDR support (Orange Box)
// - v21: Left 4 Dead 2 / Portal 2 (extended format)
//-----------------------------------------------------------------------------

#define BSPVERSION_18	18	// HL2 Beta 2003 (LeakNet native)
#define BSPVERSION_19	19	// HL2 Release 2004
#define BSPVERSION_20	20	// Source 2006/2007 HDR (Orange Box)
#define BSPVERSION_21	21	// L4D2/Portal 2

// Default version for this engine (original LeakNet)
#define BSPVERSION		BSPVERSION_18

// Version range we support loading
#define BSPVERSION_MIN	BSPVERSION_18
#define BSPVERSION_MAX	BSPVERSION_21

// Version detection macros
#define BSP_VERSION_IS_VALID(v)		((v) >= BSPVERSION_MIN && (v) <= BSPVERSION_MAX)
#define BSP_VERSION_HAS_HDR(v)		((v) >= BSPVERSION_20)
#define BSP_VERSION_HAS_AVGCOLOR(v)	((v) <= BSPVERSION_18)	// dface_t has m_AvgLightColor only in v18
#define BSP_VERSION_HAS_AMBIENT_LUMP(v)	((v) >= BSPVERSION_20)	// dleaf_t ambient in separate lump
#define BSP_VERSION_HAS_THIN_BRUSHSIDE(v) ((v) >= BSPVERSION_21)	// dbrushside_t has thin field in v21+


// This needs to match the value in gl_lightmap.h
// Need to dynamically allocate the weights and light values in radial_t to make this variable.
#define MAX_BRUSH_LIGHTMAP_DIM_WITHOUT_BORDER 32
// This is one more than what vbsp cuts for to allow for rounding errors
#define MAX_BRUSH_LIGHTMAP_DIM_INCLUDING_BORDER	35

// We can have larger lightmaps on displacements
#define MAX_DISP_LIGHTMAP_DIM_WITHOUT_BORDER	128
#define MAX_DISP_LIGHTMAP_DIM_INCLUDING_BORDER	131


// This is the actual max.. (change if you change the brush lightmap dim or disp lightmap dim
#define MAX_LIGHTMAP_DIM_WITHOUT_BORDER		MAX_DISP_LIGHTMAP_DIM_WITHOUT_BORDER
#define MAX_LIGHTMAP_DIM_INCLUDING_BORDER	MAX_DISP_LIGHTMAP_DIM_INCLUDING_BORDER



// upper design bounds

// Common limits
// leaffaces, leafbrushes, planes, and verts are still bounded by
// 16 bit short limits
#define	MAX_MAP_MODELS		1024
#define	MAX_MAP_BRUSHES		8192
#define	MAX_MAP_ENTITIES	4096
#define	MAX_MAP_ENTSTRING	(256*1024)

#define	MAX_MAP_TEXINFO		12288
#define MAX_MAP_TEXDATA		2048

#define MIN_MAP_DISP_POWER		2	// Minimum and maximum power a displacement can be.
#define MAX_MAP_DISP_POWER		4	

// Max # of neighboring displacement touching a displacement's corner.
#define MAX_DISP_CORNER_NEIGHBORS	4

#define NUM_DISP_POWER_VERTS(power)	( ((1 << (power)) + 1) * ((1 << (power)) + 1) )
#define NUM_DISP_POWER_TRIS(power)	( (1 << (power)) * (1 << (power)) * 2 )

#define MAX_MAP_DISPINFO		2048
#define MAX_MAP_DISP_VERTS		( MAX_MAP_DISPINFO * ((1<<MAX_MAP_DISP_POWER)+1) * ((1<<MAX_MAP_DISP_POWER)+1) )
#define MAX_MAP_DISP_TRIS		( (1 << MAX_MAP_DISP_POWER) * (1 << MAX_MAP_DISP_POWER) * 2 )
#define MAX_DISPVERTS			NUM_DISP_POWER_VERTS( MAX_MAP_DISP_POWER )
#define MAX_DISPTRIS			NUM_DISP_POWER_TRIS( MAX_MAP_DISP_POWER )

#define	MAX_MAP_AREAS		256
#define MAX_MAP_AREA_BYTES	(MAX_MAP_AREAS/8)
#define	MAX_MAP_AREAPORTALS	1024
// Planes come in pairs, thus an even number.
#define	MAX_MAP_PLANES		65536
#define	MAX_MAP_NODES		65536
#define	MAX_MAP_BRUSHSIDES	65536
#define	MAX_MAP_LEAFS		65536
#define	MAX_MAP_VERTS		65536
#define MAX_MAP_VERTNORMALS			256000
#define MAX_MAP_VERTNORMALINDICES	256000
#define	MAX_MAP_FACES		65536
#define	MAX_MAP_LEAFFACES	65536
#define	MAX_MAP_LEAFBRUSHES 65536
#define	MAX_MAP_PORTALS		65536
#define MAX_MAP_CLUSTERS	65536
#define MAX_MAP_LEAFWATERDATA 32768
#define MAX_MAP_PORTALVERTS	128000
#define	MAX_MAP_EDGES		256000
#define	MAX_MAP_SURFEDGES	512000
#define	MAX_MAP_LIGHTING	0x1000000
#define	MAX_MAP_VISIBILITY	0x1000000			// increased BSPVERSION 7
#define	MAX_MAP_TEXTURES	1024
#define MAX_MAP_WORLDLIGHTS	8192
#define MAX_MAP_CUBEMAPSAMPLES 1024
#define MAX_MAP_OVERLAYS	    512
#define MAX_MAP_TEXDATA_STRING_DATA		256000
#define MAX_MAP_TEXDATA_STRING_TABLE	65536

// this is stuff for trilist/tristrips, etc.
#define MAX_MAP_PRIMITIVES	32768
#define MAX_MAP_PRIMVERTS	65536
#define MAX_MAP_PRIMINDICES 65536

// key / value pair sizes

#define	MAX_KEY		32
#define	MAX_VALUE	1024


// ------------------------------------------------------------------------------------------------ //
// Displacement neighbor rules
// ------------------------------------------------------------------------------------------------ //
//
// Each displacement is considered to be in its own space:
//
//               NEIGHBOREDGE_TOP
//
//                   1 --- 2
//                   |     |
// NEIGHBOREDGE_LEFT |     | NEIGHBOREDGE_RIGHT
//                   |     |
//                   0 --- 3
//
//   			NEIGHBOREDGE_BOTTOM
//
//
// Edge edge of a displacement can have up to two neighbors. If it only has one neighbor
// and the neighbor fills the edge, then SubNeighbor 0 uses CORNER_TO_CORNER (and SubNeighbor 1
// is undefined).
//
// CORNER_TO_MIDPOINT means that it spans [bottom edge,midpoint] or [left edge,midpoint] depending
// on which edge you're on.
//
// MIDPOINT_TO_CORNER means that it spans [midpoint,top edge] or [midpoint,right edge] depending
// on which edge you're on.
//
// Here's an illustration (where C2M=CORNER_TO_MIDPOINT and M2C=MIDPOINT_TO_CORNER
//
//
//				 C2M			  M2C
//
//       1 --------------> x --------------> 2
//
//       ^                                   ^
//       |                                   |
//       |                                   |
//  M2C  |                                   |	M2C
//       |                                   |
//       |                                   |
//
//       x                 x                 x 
//
//       ^                                   ^
//       |                                   |
//       |                                   |
//  C2M  |                                   |	C2M
//       |                                   |
//       |                                   |
// 
//       0 --------------> x --------------> 3
//
//               C2M			  M2C
//
//
// The CHILDNODE_ defines can be used to refer to a node's child nodes (this is for when you're
// recursing into the node tree inside a displacement):
//
// ---------
// |   |   |
// | 1 | 0 |
// |   |   |
// |---x---|
// |   |   |
// | 2 | 3 |
// |   |   |
// ---------
// 
// ------------------------------------------------------------------------------------------------ //

// These can be used to index g_ChildNodeIndexMul.
enum
{
	CHILDNODE_UPPER_RIGHT=0,
	CHILDNODE_UPPER_LEFT=1,
	CHILDNODE_LOWER_LEFT=2,
	CHILDNODE_LOWER_RIGHT=3
};


// Corner indices. Used to index m_CornerNeighbors.
enum
{
	CORNER_LOWER_LEFT=0,
	CORNER_UPPER_LEFT=1,
	CORNER_UPPER_RIGHT=2,
	CORNER_LOWER_RIGHT=3
};


// These edge indices must match the edge indices of the CCoreDispSurface.
enum
{
	NEIGHBOREDGE_LEFT=0,
	NEIGHBOREDGE_TOP=1,
	NEIGHBOREDGE_RIGHT=2,
	NEIGHBOREDGE_BOTTOM=3
};


// These denote where one dispinfo fits on another.
// Note: tables are generated based on these indices so make sure to update
//       them if these indices are changed.
typedef enum
{
	CORNER_TO_CORNER=0,
	CORNER_TO_MIDPOINT=1,
	MIDPOINT_TO_CORNER=2
} NeighborSpan;


// These define relative orientations of displacement neighbors.
typedef enum
{
	ORIENTATION_CCW_0=0,
	ORIENTATION_CCW_90=1,
	ORIENTATION_CCW_180=2,
	ORIENTATION_CCW_270=3
} NeighborOrientation;


//=============================================================================

enum
{
	LUMP_ENTITIES		= 0,		// *
	LUMP_PLANES			= 1,		// *
// JAY: This is texdata now, previously LUMP_TEXTURES
	LUMP_TEXDATA		= 2,		// 
	LUMP_VERTEXES		= 3,		// *
	LUMP_VISIBILITY		= 4,		// *
	LUMP_NODES			= 5,		// *
	LUMP_TEXINFO		= 6,		// *
	LUMP_FACES			= 7,		// *
	LUMP_LIGHTING		= 8,		// *
	LUMP_OCCLUSION		= 9,
	LUMP_LEAFS			= 10,		// *
	LUMP_FACEIDS		= 11,		// v48: Hammer face ID tracking

	LUMP_EDGES			= 12,		// *
	LUMP_SURFEDGES		= 13,		// *
	LUMP_MODELS			= 14,		// *
	LUMP_WORLDLIGHTS	= 15,		// 

	LUMP_LEAFFACES		= 16,		// *
	LUMP_LEAFBRUSHES	= 17,		// *
	LUMP_BRUSHES		= 18,		// *
	LUMP_BRUSHSIDES		= 19,		// *
	LUMP_AREAS			= 20,		// *
	LUMP_AREAPORTALS	= 21,		// *
	LUMP_PORTALS		= 22,
	LUMP_CLUSTERS		= 23,
	LUMP_PORTALVERTS	= 24,
	LUMP_CLUSTERPORTALS = 25,
	LUMP_DISPINFO		= 26,
	LUMP_ORIGINALFACES	= 27,
	LUMP_PHYSDISP		= 28,		// v48: Displacement physics collision
	LUMP_PHYSCOLLIDE	= 29,
	LUMP_VERTNORMALS	= 30,
	LUMP_VERTNORMALINDICES		= 31,

	LUMP_DISP_LIGHTMAP_ALPHAS	= 32,
	LUMP_DISP_VERTS = 33,						// CDispVerts
	
	LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS = 34,	// For each displacement
												//     For each lightmap sample
												//         byte for index
												//         if 255, then index = next byte + 255
												//         3 bytes for barycentric coordinates

	// The game lump is a method of adding game-specific lumps
	// FIXME: Eventually, all lumps could use the game lump system
	LUMP_GAME_LUMP = 35,

	LUMP_LEAFWATERDATA	= 36,

	LUMP_PRIMITIVES		= 37,
	LUMP_PRIMVERTS		= 38,
	LUMP_PRIMINDICES	= 39,

	// A pak file can be embedded in a .bsp now, and the file system will search the pak
	//  file first for any referenced names, before deferring to the game directory 
	//  file system/pak files and finally the base directory file system/pak files.
	LUMP_PAKFILE		= 40,
	LUMP_CLIPPORTALVERTS= 41,
	// A map can have a number of cubemap entities in it which cause cubemap renders
	// to be taken after running vrad.
	LUMP_CUBEMAPS		= 42,

	LUMP_TEXDATA_STRING_DATA	= 43,
	LUMP_TEXDATA_STRING_TABLE	= 44,
	LUMP_OVERLAYS				= 45,
	LUMP_LEAFMINDISTTOWATER		= 46,
	LUMP_FACE_MACRO_TEXTURE_INFO = 47,
	LUMP_DISP_TRIS = 48,

	//-----------------------------------------------------------------------------
	// New lumps added in BSP v19/v20 (Source 2006/2007)
	//-----------------------------------------------------------------------------
	LUMP_PHYSCOLLIDESURFACE = 49,		// v48: Win32 Havok physics compat (deprecated)
	LUMP_WATEROVERLAYS = 50,			// v20+
	LUMP_LEAF_AMBIENT_INDEX_HDR = 51,	// v20+ HDR ambient lighting indices
	LUMP_LEAF_AMBIENT_INDEX = 52,		// v20+ LDR ambient lighting indices

	// Xbox-specific alternate lightmap format (v20+)
	LUMP_LIGHTMAPPAGES = 53,			// v20+ alternate lightmap pages
	LUMP_LIGHTMAPPAGEINFOS = 54,		// v20+ lightmap page info

	// HDR lighting lumps (v20+)
	LUMP_LIGHTING_HDR = 55,				// v20+ HDR lightmap samples
	LUMP_WORLDLIGHTS_HDR = 56,			// v20+ HDR world lights
	LUMP_LEAF_AMBIENT_LIGHTING_HDR = 57,	// v20+ per-leaf HDR ambient
	LUMP_LEAF_AMBIENT_LIGHTING = 58,	// v20+ per-leaf LDR ambient (when HDR present)

	// v21+ lumps
	LUMP_XZIPPAKFILE = 59,				// v21+ Xbox compressed pakfile
	LUMP_FACES_HDR = 60,				// v21+ HDR face data
	LUMP_MAP_FLAGS = 61,				// v21+ Map compile flags

	// Extended lumps (v20+)
	LUMP_OVERLAY_FADES = 62,			// Overlay fade distances
	LUMP_OVERLAY_SYSTEM_LEVELS = 63,	// Overlay system level info
	LUMP_PHYSLEVEL = 64					// Physics level
};


// Lumps that have versions are listed here
enum
{
	LUMP_LIGHTING_VERSION = 1,
	LUMP_FACES_VERSION = 1,

	// Occlusion lump versions
	// v1: Original format (v18 BSP) - doccluderdataV1_t (no area field)
	// v2: v19+ format - doccluderdata_t (with area field)
	LUMP_OCCLUSION_VERSION_V1 = 1,	// v18 format
	LUMP_OCCLUSION_VERSION_V2 = 2,	// v19+ format
	LUMP_OCCLUSION_VERSION = 2,		// Current/default version for compatibility

	// Leaf lump versions (for dleaf_t structure differences)
	LUMP_LEAFS_VERSION_0 = 0,		// v19 and earlier: 56 bytes with ambient
	LUMP_LEAFS_VERSION_1 = 1,		// v20+: 32 bytes, ambient in separate lump
};


#define	HEADER_LUMPS		64

#pragma pack(1)
struct ZIP_EndOfCentralDirRecord
{
	unsigned int signature; // 4 bytes (0x06054b50)
	unsigned short numberOfThisDisk;  // 2 bytes
	unsigned short numberOfTheDiskWithStartOfCentralDirectory; // 2 bytes
	unsigned short nCentralDirectoryEntries_ThisDisk;	// 2 bytes
	unsigned short nCentralDirectoryEntries_Total;	// 2 bytes
	unsigned int centralDirectorySize; // 4 bytes
	unsigned int startOfCentralDirOffset; // 4 bytes
	unsigned short commentLength; // 2 bytes
	// zip file comment follows
};

struct ZIP_FileHeader
{
	unsigned int signature; //  4 bytes (0x02014b50) 
	unsigned short versionMadeBy; // version made by 2 bytes 
	unsigned short versionNeededToExtract; // version needed to extract 2 bytes 
	unsigned short flags; // general purpose bit flag 2 bytes 
	unsigned short compressionMethod; // compression method 2 bytes 
	unsigned short lastModifiedTime; // last mod file time 2 bytes 
	unsigned short lastModifiedDate; // last mod file date 2 bytes 
	unsigned int crc32; // crc-32 4 bytes 
	unsigned int compressedSize; // compressed size 4 bytes 
	unsigned int uncompressedSize; // uncompressed size 4 bytes 
	unsigned short fileNameLength; // file name length 2 bytes 
	unsigned short extraFieldLength; // extra field length 2 bytes 
	unsigned short fileCommentLength; // file comment length 2 bytes 
	unsigned short diskNumberStart; // disk number start 2 bytes 
	unsigned short internalFileAttribs; // internal file attributes 2 bytes 
	unsigned int externalFileAttribs; // external file attributes 4 bytes 
	unsigned int relativeOffsetOfLocalHeader; // relative offset of local header 4 bytes 
	// file name (variable size) 
	// extra field (variable size) 
	// file comment (variable size) 
};

struct ZIP_LocalFileHeader
{
	unsigned int signature; //local file header signature 4 bytes (0x04034b50) 
	unsigned short versionNeededToExtract; // version needed to extract 2 bytes 
	unsigned short flags; // general purpose bit flag 2 bytes 
	unsigned short compressionMethod; // compression method 2 bytes 
	unsigned short lastModifiedTime; // last mod file time 2 bytes 
	unsigned short lastModifiedDate; // last mod file date 2 bytes 
	unsigned int crc32; // crc-32 4 bytes 
	unsigned int compressedSize; // compressed size 4 bytes 
	unsigned int uncompressedSize; // uncompressed size 4 bytes 
	unsigned short fileNameLength; // file name length 2 bytes 
	unsigned short extraFieldLength; // extra field length 2 bytes 
};
#pragma pack()

struct lump_t
{
	int		fileofs, filelen;
	int		version;		// default to zero
	char	fourCC[4];		// default to ( char )0, ( char )0, ( char )0, ( char )0
};


struct dheader_t
{
	int			ident;
	int			version;	
	lump_t		lumps[HEADER_LUMPS];
	int			mapRevision;				// the map's revision (iteration, version) number (added BSPVERSION 6)
};

struct dgamelumpheader_t
{
	int lumpCount;

	// dclientlump_ts follow this
};

// This is expected to be a four-CC code ('lump')
typedef int GameLumpId_t;

struct dgamelump_t
{
	GameLumpId_t	id;
	unsigned short flags;		// currently unused, but you never know!
	unsigned short version;
	int	fileofs;
	int filelen;
};

extern int g_MapRevision;

struct dmodel_t
{
	Vector		mins, maxs;
	Vector		origin;			// for sounds or lights
	int			headnode;
	int			firstface, numfaces;	// submodels just draw faces
										// without walking the bsp tree
};

struct dphysmodel_t
{
	int			modelIndex;
	int			dataSize;
	int			keydataSize;
	int			solidCount;
};

struct dvertex_t
{
	Vector	point;
};

#ifndef PLANE_H
#include "plane.h"
#endif

// planes (x&~1) and (x&~1)+1 are always opposites
struct dplane_t
{
	Vector	normal;
	float	dist;
	int		type;		// PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
};

#ifndef BSPFLAGS_H
#include "bspflags.h"
#endif

struct dnode_t
{
	int			planenum;
	int			children[2];	// negative numbers are -(leafs+1), not nodes
	short		mins[3];		// for frustom culling
	short		maxs[3];
	unsigned short	firstface;
	unsigned short	numfaces;	// counting both sides
	short			area;		// If all leaves below this node are in the same area, then
								// this is the area index. If not, this is -1.
};

typedef struct texinfo_s
{
	float		textureVecsTexelsPerWorldUnits[2][4];			// [s/t][xyz offset]
	float		lightmapVecsLuxelsPerWorldUnits[2][4];			// [s/t][xyz offset] - length is in units of texels/area
	int			flags;				// miptex flags + overrides
	int			texdata;			// Pointer to texture name, size, etc.
} texinfo_t;

#define TEXTURE_NAME_LENGTH	 128			// changed from 64 BSPVERSION 8

struct dtexdata_t
{
	Vector		reflectivity;
	int			nameStringTableID;				// index into g_StringTable for the texture name
	int			width, height;					// source image
	int			view_width, view_height;		//
};


//-----------------------------------------------------------------------------
// Occluders are simply polygons
//-----------------------------------------------------------------------------
// Flags field of doccluderdata_t
enum
{
	OCCLUDER_FLAGS_INACTIVE = 0x1,
};

// v1 occluder structure (v18 BSP, LUMP_OCCLUSION_VERSION_V1)
// Used in original HL2 beta and earlier Source builds
struct doccluderdataV1_t
{
	int			flags;
	int			firstpoly;				// index into doccluderpolys
	int			polycount;
	Vector		mins;
	Vector		maxs;
};

// v2 occluder structure (v19+ BSP, LUMP_OCCLUSION_VERSION_V2)
// Used in HL2 retail and later Source builds - adds area field
struct doccluderdata_t
{
	int			flags;
	int			firstpoly;				// index into doccluderpolys
	int			polycount;
	Vector		mins;
	Vector		maxs;
	int			area;					// v19+: Area index (added in LUMP_OCCLUSION_VERSION_V2)
};

struct doccluderpolydata_t
{
	int			firstvertexindex;		// index into doccludervertindices
	int			vertexcount;
	int			planenum;
};


// NOTE: see the section above titled "displacement neighbor rules".
struct CDispSubNeighbor
{
public:

	unsigned short		GetNeighborIndex() const		{ return m_iNeighbor; }
	NeighborSpan		GetSpan() const					{ return (NeighborSpan)m_Span; }
	NeighborSpan		GetNeighborSpan() const			{ return (NeighborSpan)m_NeighborSpan; }
	NeighborOrientation	GetNeighborOrientation() const	{ return (NeighborOrientation)m_NeighborOrientation; }

	bool				IsValid() const				{ return m_iNeighbor != 0xFFFF; }
	void				SetInvalid()				{ m_iNeighbor = 0xFFFF; }


public:
	unsigned short		m_iNeighbor;		// This indexes into ddispinfos.
											// 0xFFFF if there is no neighbor here.

	unsigned char		m_NeighborOrientation;		// (CCW) rotation of the neighbor wrt this displacement.

	// These use the NeighborSpan type.
	unsigned char		m_Span;						// Where the neighbor fits onto this side of our displacement.
	unsigned char		m_NeighborSpan;				// Where we fit onto our neighbor.
};


// NOTE: see the section above titled "displacement neighbor rules".
class CDispNeighbor
{
public:
	void				SetInvalid()	{ m_SubNeighbors[0].SetInvalid(); m_SubNeighbors[1].SetInvalid(); }
	
	// Returns false if there isn't anything touching this edge.
	bool				IsValid()		{ return m_SubNeighbors[0].IsValid() || m_SubNeighbors[1].IsValid(); }


public:
	// Note: if there is a neighbor that fills the whole side (CORNER_TO_CORNER),
	//       then it will always be in CDispNeighbor::m_Neighbors[0]
	CDispSubNeighbor	m_SubNeighbors[2];
};


class CDispCornerNeighbors
{
public:

	void			SetInvalid()	{ m_nNeighbors = 0; }


public:
	unsigned short	m_Neighbors[MAX_DISP_CORNER_NEIGHBORS];	// indices of neighbors.
	unsigned char	m_nNeighbors;
};


class CDispVert
{
public:
	Vector		m_vVector;		// Vector field defining displacement volume.
	float		m_flDist;		// Displacement distances.
	float		m_flAlpha;		// "per vertex" alpha values.
};

#define DISPTRI_TAG_SURFACE			(1<<0)
#define DISPTRI_TAG_WALKABLE		(1<<1)
#define DISPTRI_TAG_BUILDABLE		(1<<2)
#define DISPTRI_FLAG_SURFPROP1		(1<<3)	// v48: Surface property flag 1
#define DISPTRI_FLAG_SURFPROP2		(1<<4)	// v48: Surface property flag 2

class CDispTri
{
public:
	unsigned short m_uiTags;		// Displacement triangle tags.
};

class ddispinfo_t
{
public:
	int			NumVerts() const		{ return NUM_DISP_POWER_VERTS(power); }
	int			NumTris() const			{ return NUM_DISP_POWER_TRIS(power); }

public:
	Vector		startPosition;						// start position used for orientation -- (added BSPVERSION 6)
	int			m_iDispVertStart;					// Index into LUMP_DISP_VERTS.
	int			m_iDispTriStart;					// Index into LUMP_DISP_TRIS.

    int         power;                              // power - indicates size of map (2^power + 1)
    int         minTess;                            // minimum tesselation allowed
    float       smoothingAngle;                     // lighting smoothing angle
    int         contents;                           // surface contents

	unsigned short	m_iMapFace;						// Which map face this displacement comes from.
	
	int			m_iLightmapAlphaStart;				// Index into ddisplightmapalpha.
													// The count is m_pParent->lightmapTextureSizeInLuxels[0]*m_pParent->lightmapTextureSizeInLuxels[1].

	int			m_iLightmapSamplePositionStart;		// Index into LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS.

	CDispNeighbor			m_EdgeNeighbors[4];		// Indexed by NEIGHBOREDGE_ defines.
	CDispCornerNeighbors	m_CornerNeighbors[4];	// Indexed by CORNER_ defines.

	enum { ALLOWEDVERTS_SIZE = PAD_NUMBER( MAX_DISPVERTS, 32 ) / 32 };
	unsigned long	m_AllowedVerts[ALLOWEDVERTS_SIZE];	// This is built based on the layout and sizes of our neighbors
														// and tells us which vertices are allowed to be active.
};



// note that edge 0 is never used, because negative edge nums are used for
// counterclockwise use of the edge in a face
struct dedge_t
{
	unsigned short	v[2];		// vertex numbers
};

#define	MAXLIGHTMAPS	4

enum dprimitive_type
{
	PRIM_TRILIST=0,
	PRIM_TRISTRIP=1,
};

struct dprimitive_t
{
	unsigned char type;
	unsigned short	firstIndex;
	unsigned short	indexCount;
	unsigned short	firstVert;
	unsigned short	vertCount;
};

struct dprimvert_t
{
	Vector		pos;
};

//-----------------------------------------------------------------------------
// dface_t structures - version specific
// v18 (LeakNet native): 72 bytes - includes m_AvgLightColor
// v19+ (Source 2004+): 56 bytes - no m_AvgLightColor
//-----------------------------------------------------------------------------

// v19+ face structure (Source 2004+, 56 bytes)
// This is the standard face format for modern Source maps
struct dface_v19_t
{
	unsigned short	planenum;
	byte		side;	// faces opposite to the node's plane direction
	byte		onNode; // 1 of on node, 0 if in leaf

	int			firstedge;		// we must support > 64k edges
	short		numedges;
	short		texinfo;
	short       dispinfo;
	short		surfaceFogVolumeID;

	// lighting info
	byte		styles[MAXLIGHTMAPS];
	int			lightofs;		// start of [numstyles*surfsize] samples
    float       area;

	int			m_LightmapTextureMinsInLuxels[2];
	int			m_LightmapTextureSizeInLuxels[2];

    int         origFace;       // reference the original face this face was derived from

	// non-polygon primitives (strips and lists)
	unsigned short	numPrims;
	unsigned short	firstPrimID;

	unsigned int	smoothingGroups;
};

// v18 face structure (HL2 Beta 2003, 72 bytes)
// Includes m_AvgLightColor for faster lighting computation
struct dface_t
{
	// v18 only: For computing lighting information (R_LightVec)
	// This field was removed in v19+
	colorRGBExp32	m_AvgLightColor[MAXLIGHTMAPS];

	unsigned short	planenum;
	byte		side;	// faces opposite to the node's plane direction
	byte		onNode; // 1 of on node, 0 if in leaf

	int			firstedge;		// we must support > 64k edges
	short		numedges;
	short		texinfo;
	// This is a union under the assumption that a fog volume boundary (ie. water surface)
	// isn't a displacement map.
	// FIXME: These should be made a union with a flags or type field for which one it is
	// if we can add more to this.
//	union
//	{
	    short       dispinfo;
		// This is only for surfaces that are the boundaries of fog volumes
		// (ie. water surfaces)
		// All of the rest of the surfaces can look at their leaf to find out
		// what fog volume they are in.
		short		surfaceFogVolumeID;
//	};

	// lighting info
	byte		styles[MAXLIGHTMAPS];
	int			lightofs;		// start of [numstyles*surfsize] samples
    float       area;

	// TODO: make these unsigned chars?
	int			m_LightmapTextureMinsInLuxels[2];
	int			m_LightmapTextureSizeInLuxels[2];

    int         origFace;       // reference the original face this face was derived from

	// non-polygon primitives (strips and lists)
	unsigned short	numPrims;
	unsigned short	firstPrimID;

	unsigned int	smoothingGroups;
};

// Alias for v18 format (native LeakNet)
typedef dface_t dface_v18_t;

// Size constants for version detection
#define DFACE_V18_SIZE	sizeof(dface_v18_t)	// 72 bytes
#define DFACE_V19_SIZE	sizeof(dface_v19_t)	// 56 bytes

//-----------------------------------------------------------------------------
// dleaf_t structures - version specific
// v18/v19 (Lump version 0): 56 bytes - includes ambient lighting
// v20+ (Lump version 1): 32 bytes - ambient lighting in separate lump
//-----------------------------------------------------------------------------

// Ambient lighting sample for v20+ (stored in separate lump)
struct dleafambientindex_t
{
	unsigned short	ambientSampleCount;
	unsigned short	firstAmbientSample;
};

// Per-leaf ambient lighting data for v20+
struct dleafambientlighting_t
{
	CompressedLightCube	cube;			// Ambient lighting at this sample point
	byte				x;				// Position of sample inside leaf (0-255 fraction)
	byte				y;
	byte				z;
	byte				pad;			// Alignment
};

// v20+ leaf structure (Lump version 1, 32 bytes)
// Ambient lighting moved to separate lump for HDR support
struct dleaf_v1_t
{
	int				contents;			// OR of all brushes

	short			cluster;
	short			area:9;				// Note: area uses 9 bits in v20+
	short			flags:7;			// Flags use remaining 7 bits

	short			mins[3];			// for frustum culling
	short			maxs[3];

	unsigned short	firstleafface;
	unsigned short	numleaffaces;

	unsigned short	firstleafbrush;
	unsigned short	numleafbrushes;
	short			leafWaterDataID;	// -1 for not in water

	// Note: no ambient lighting here - it's in LUMP_LEAF_AMBIENT_LIGHTING
};

// v18/v19 leaf structure (Lump version 0, 56 bytes for v19, 32 bytes for v18)
// v18 LeakNet uses a simpler 32-byte structure without ambient lighting fields
struct dleaf_t
{
	int				contents;			// OR of all brushes (not needed?)

	short			cluster;
	short			area;

	short			mins[3];			// for frustum culling
	short			maxs[3];

	unsigned short	firstleafface;
	unsigned short	numleaffaces;

	unsigned short	firstleafbrush;
	unsigned short	numleafbrushes;
	short			leafWaterDataID; // -1 for not in water
};

// v19 leaf structure with embedded ambient lighting (56 bytes)
// This was used before the ambient lump was introduced
// NOTE: area and flags are packed as bitfields in v19+ BSP format
struct dleaf_v0_t
{
	int				contents;			// OR of all brushes

	short			cluster;
	short			area:9;				// Area portal index (9 bits)
	short			flags:7;			// Per-leaf flags (7 bits)

	short			mins[3];			// for frustum culling
	short			maxs[3];

	unsigned short	firstleafface;
	unsigned short	numleaffaces;

	unsigned short	firstleafbrush;
	unsigned short	numleafbrushes;
	short			leafWaterDataID;	// -1 for not in water

	// v19 only: Embedded ambient lighting (removed in v20+)
	CompressedLightCube	m_AmbientLighting;
};

// Alias for v18 format (native LeakNet)
typedef dleaf_t dleaf_v18_t;

// Size constants for version detection
#define DLEAF_V18_SIZE	sizeof(dleaf_v18_t)	// 32 bytes (LeakNet native)
#define DLEAF_V0_SIZE	sizeof(dleaf_v0_t)	// 56 bytes (v19 with ambient)
#define DLEAF_V1_SIZE	sizeof(dleaf_v1_t)	// 32 bytes (v20+, ambient in separate lump)

//-----------------------------------------------------------------------------
// dbrushside_t structures - version specific
// v18-v20: Uses short for bevel field
// v21+: Splits bevel into bevel (byte) and thin (byte)
//-----------------------------------------------------------------------------

// v18-v20 brush side structure
struct dbrushside_t
{
	unsigned short	planenum;		// facing out of the leaf
	short	texinfo;
	short			dispinfo;		// displacement info (BSPVERSION 7)
	short			bevel;			// is the side a bevel plane? (BSPVERSION 7)
};

// v21+ brush side structure
// Split bevel into two bytes for 'thin' edge detection
struct dbrushside_v21_t
{
	unsigned short	planenum;		// facing out of the leaf
	short			texinfo;
	short			dispinfo;		// displacement info
	unsigned char	bevel;			// is the side a bevel plane?
	unsigned char	thin;			// is the side thin? (edges < 16 units)
};

struct dbrush_t
{
	int			firstside;
	int			numsides;
	int			contents;
};

#define	ANGLE_UP	-1
#define	ANGLE_DOWN	-2


// the visibility lump consists of a header with a count, then
// byte offsets for the PVS and PHS of each cluster, then the raw
// compressed bit vectors
#define	DVIS_PVS	0
#define	DVIS_PAS	1
struct dvis_t
{
	int			numclusters;
	int			bitofs[8][2];	// bitofs[numclusters][2]
};

// each area has a list of portals that lead into other areas
// when portals are closed, other areas may not be visible or
// hearable even if the vis info says that it should be
struct dareaportal_t
{
	unsigned short	m_PortalKey;		// Entities have a key called portalnumber (and in vbsp a variable
									// called areaportalnum) which is used
									// to bind them to the area portals by comparing with this value.
	
	unsigned short	otherarea;		// The area this portal looks into.
	
	unsigned short	m_FirstClipPortalVert;	// Portal geometry.
	unsigned short	m_nClipPortalVerts;

	int				planenum;
};


struct darea_t
{
	int		numareaportals;
	int		firstareaportal;
};

struct dportal_t
{
	int		firstportalvert;
	int		numportalverts;
	int		planenum;
	unsigned short	cluster[2];
};

struct dcluster_t
{
	int		firstportal;
	int		numportals;
};

struct dleafwaterdata_t
{
	float	surfaceZ;
	float	minZ;
	short	surfaceTexInfoID;
};

class CFaceMacroTextureInfo
{
public:
	// This looks up into g_TexDataStringTable, which looks up into g_TexDataStringData.
	// 0xFFFF if the face has no macro texture.
	unsigned short m_MacroTextureNameID;	
};

// lights that were used to illuminate the world
enum emittype_t
{
	emit_surface,		// 90 degree spotlight
	emit_point,			// simple point light source
	emit_spotlight,		// spotlight with penumbra
	emit_skylight,		// directional light with no falloff (surface must trace to SKY texture)
	emit_quakelight,	// linear falloff, non-lambertian
	emit_skyambient,	// spherical light source with no falloff (surface must trace to SKY texture)
};

// Flags for dworldlight_t::flags
#define DWL_FLAGS_INAMBIENTCUBE		0x0001	// v48: Light was put into the per-leaf ambient cubes.

struct dworldlight_t
{
	Vector		origin;
	Vector		intensity;
	Vector		normal;			// for surfaces and spotlights
	int			cluster;
	emittype_t	type;
    int			style;
	float		stopdot;		// start of penumbra for emit_spotlight
	float		stopdot2;		// end of penumbra for emit_spotlight
	float		exponent;		// 
	float		radius;			// cutoff distance
	// falloff for emit_spotlight + emit_point: 
	// 1 / (constant_attn + linear_attn * dist + quadratic_attn * dist^2)
	float		constant_attn;	
	float		linear_attn;
	float		quadratic_attn;
	int			flags;
	int			texinfo;		// 
	int			owner;			// entity that this light it relative to
};

struct dcubemapsample_t
{
	int			origin[3];			// position of light snapped to the nearest integer
									// the filename for the vtf file is derived from the position
	unsigned char size;				// 0 - default
									// otherwise, 1<<(size-1)
};

#define OVERLAY_BSP_FACE_COUNT	64

// Overlay render order support (v19+ BSP)
// In v19+ BSP files, the nFaceCount field encodes both face count and render order:
// - Upper 2 bits: render order (0-3)
// - Lower 14 bits: actual face count
#define OVERLAY_RENDER_ORDER_NUM_BITS	2
#define OVERLAY_NUM_RENDER_ORDERS		(1<<OVERLAY_RENDER_ORDER_NUM_BITS)
#define OVERLAY_RENDER_ORDER_MASK		0xC000	// top 2 bits set

// Original overlay format (v19+)
struct doverlay_t
{
	int			nId;
	short		nTexInfo;
	short		nFaceCount;		// In v19+: upper 2 bits = render order, lower 14 bits = face count
	int			aFaces[OVERLAY_BSP_FACE_COUNT];
	float		flU[2];
	float		flV[2];
	Vector		vecUVPoints[4];
	Vector		vecOrigin;
	Vector		vecBasisNormal;

	// Helper to get face count (works for all BSP versions)
	inline int GetFaceCount() const { return nFaceCount & ~OVERLAY_RENDER_ORDER_MASK; }
	// Helper to get render order (returns 0 for v18 BSP)
	inline int GetRenderOrder() const { return (nFaceCount >> (16 - OVERLAY_RENDER_ORDER_NUM_BITS)) & 0x3; }
};

// v48: 2007 Source Engine compatible overlay format with proper encapsulation
struct doverlay_v48_t
{
	int			nId;
	short		nTexInfo;

	// Accessor methods for proper encapsulation
	inline void SetFaceCount( unsigned short count )
	{
		Assert( count >= 0 && (count & OVERLAY_RENDER_ORDER_MASK) == 0 );
		m_nFaceCountAndRenderOrder &= OVERLAY_RENDER_ORDER_MASK;
		m_nFaceCountAndRenderOrder |= (count & ~OVERLAY_RENDER_ORDER_MASK);
	}
	inline unsigned short GetFaceCount() const
	{
		return m_nFaceCountAndRenderOrder & ~OVERLAY_RENDER_ORDER_MASK;
	}

	inline void SetRenderOrder( unsigned short order )
	{
		Assert( order >= 0 && order < OVERLAY_NUM_RENDER_ORDERS );
		m_nFaceCountAndRenderOrder &= ~OVERLAY_RENDER_ORDER_MASK;
		m_nFaceCountAndRenderOrder |= (order << (16 - OVERLAY_RENDER_ORDER_NUM_BITS));
	}
	inline unsigned short GetRenderOrder() const
	{
		return (m_nFaceCountAndRenderOrder & OVERLAY_RENDER_ORDER_MASK) >> (16 - OVERLAY_RENDER_ORDER_NUM_BITS);
	}

private:
	unsigned short	m_nFaceCountAndRenderOrder;

public:
	int			aFaces[OVERLAY_BSP_FACE_COUNT];
	float		flU[2];
	float		flV[2];
	Vector		vecUVPoints[4];
	Vector		vecOrigin;
	Vector		vecBasisNormal;
};

// Overlay fade distances (v19+ LUMP_OVERLAY_FADES)
struct doverlayfade_t
{
	float flFadeDistMinSq;
	float flFadeDistMaxSq;
};

// Water overlay structure (v19+ LUMP_WATEROVERLAYS)
#define WATEROVERLAY_BSP_FACE_COUNT				256
#define WATEROVERLAY_RENDER_ORDER_NUM_BITS		2
#define WATEROVERLAY_NUM_RENDER_ORDERS			(1<<WATEROVERLAY_RENDER_ORDER_NUM_BITS)
#define WATEROVERLAY_RENDER_ORDER_MASK			0xC000	// top 2 bits set

struct dwateroverlay_t
{
	int				nId;
	short			nTexInfo;
	short			nFaceCount;		// Upper 2 bits = render order, lower 14 bits = face count
	int				aFaces[WATEROVERLAY_BSP_FACE_COUNT];
	float			flU[2];
	float			flV[2];
	Vector			vecUVPoints[4];
	Vector			vecOrigin;
	Vector			vecBasisNormal;

	inline int GetFaceCount() const { return nFaceCount & ~WATEROVERLAY_RENDER_ORDER_MASK; }
	inline int GetRenderOrder() const { return (nFaceCount >> (16 - WATEROVERLAY_RENDER_ORDER_NUM_BITS)) & 0x3; }
};

// Finalized page of surface's lightmaps (Xbox format, v20+)
#define MAX_LIGHTMAPPAGE_WIDTH	256
#define MAX_LIGHTMAPPAGE_HEIGHT	128

struct dlightmappage_t
{
	byte	data[MAX_LIGHTMAPPAGE_WIDTH*MAX_LIGHTMAPPAGE_HEIGHT];
	byte	palette[256*4];
};

struct dlightmappageinfo_t
{
	byte			page;			// lightmap page [0..?]
	byte			offset[2];		// offset into page (s,t)
	byte			pad;			// unused
	colorRGBExp32	avgColor;		// average used for runtime lighting calcs
};

#ifndef _DEF_BYTE_
#define _DEF_BYTE_
typedef unsigned char	byte;
typedef unsigned short	word;
#endif


#define	ANGLE_UP	-1
#define	ANGLE_DOWN	-2


//===============

//-----------------------------------------------------------------------------
// v48: Map compilation flags (LUMP_MAP_FLAGS)
//-----------------------------------------------------------------------------
// Level feature flags
#define LVLFLAGS_BAKED_STATIC_PROP_LIGHTING_NONHDR 0x00000001	// v48: processed by vrad with -staticproplighting, no hdr data
#define LVLFLAGS_BAKED_STATIC_PROP_LIGHTING_HDR    0x00000002  // v48: processed by vrad with -staticproplighting, in hdr

struct dflagslump_t
{
	uint32 m_LevelFlags;						// LVLFLAGS_xxx
};


struct epair_t
{
	epair_t	*next;
	char	*key;
	char	*value;
};

#endif // BSPFILE_H
