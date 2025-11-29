//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Defines game-specific data
//
// $Revision: $
// $NoKeywords: $
//=============================================================================

#ifndef GAMEBSPFILE_H
#define GAMEBSPFILE_H

#ifdef _WIN32
#pragma once
#endif


#include "vector.h"
#include "basetypes.h"


//-----------------------------------------------------------------------------
// This enumerations defines all the four-CC codes for the client lump names
//-----------------------------------------------------------------------------
enum
{
	GAMELUMP_DETAIL_PROPS		= 'dprp',
	GAMELUMP_DETAIL_PROP_LIGHTING	= 'dplt',
	GAMELUMP_STATIC_PROPS		= 'sprp',
};

// Versions...
enum
{
	GAMELUMP_DETAIL_PROPS_VERSION	= 4,
	GAMELUMP_DETAIL_PROP_LIGHTING_VERSION	= 0,

	// Static props versions:
	// v4: Original HL2 Beta 2003 (LeakNet native)
	// v5: HL2 Release 2004 (added ForcedFadeScale)
	// v6: Source 2006/2007 (added MinDXLevel, MaxDXLevel)
	// v7: Added color modulation (diffuse)
	// v8: Same as v7 (unused version)
	// v9: L4D1 - same as v7 with different flag meanings
	// v10: L4D2/Portal 2 - 32-bit flags, added DisableX360
	// v11: CS:GO - added uniform scale
	GAMELUMP_STATIC_PROPS_VERSION_4		= 4,	// LeakNet native
	GAMELUMP_STATIC_PROPS_VERSION_5		= 5,	// HL2 Release
	GAMELUMP_STATIC_PROPS_VERSION_6		= 6,	// Source 2006/2007
	GAMELUMP_STATIC_PROPS_VERSION_7		= 7,	// Color modulation
	GAMELUMP_STATIC_PROPS_VERSION_8		= 8,	// Same as v7
	GAMELUMP_STATIC_PROPS_VERSION_9		= 9,	// L4D1
	GAMELUMP_STATIC_PROPS_VERSION_10	= 10,	// L4D2/Portal 2
	GAMELUMP_STATIC_PROPS_VERSION_11	= 11,	// CS:GO
	GAMELUMP_STATIC_PROPS_VERSION		= GAMELUMP_STATIC_PROPS_VERSION_4,	// Default (native)

	GAMELUMP_STATIC_PROP_LIGHTING_VERSION	= 0,
};


//-----------------------------------------------------------------------------
// This is the data associated with the GAMELUMP_DETAIL_PROPS lump
//-----------------------------------------------------------------------------
#define DETAIL_NAME_LENGTH 128

enum DetailPropOrientation_t
{
	DETAIL_PROP_ORIENT_NORMAL = 0,
	DETAIL_PROP_ORIENT_SCREEN_ALIGNED,
	DETAIL_PROP_ORIENT_SCREEN_ALIGNED_VERTICAL,
};

enum DetailPropType_t
{
	DETAIL_PROP_TYPE_MODEL = 0,
	DETAIL_PROP_TYPE_SPRITE,
};


//-----------------------------------------------------------------------------
// Model index when using studiomdls for detail props
//-----------------------------------------------------------------------------
struct DetailObjectDictLump_t
{
	char	m_Name[DETAIL_NAME_LENGTH];		// model name
};


//-----------------------------------------------------------------------------
// Information about the sprite to render
//-----------------------------------------------------------------------------
struct DetailSpriteDictLump_t
{
	// NOTE: All detail prop sprites must lie in the material detail/detailsprites
	Vector2D	m_UL;		// Coordinate of upper left 
	Vector2D	m_LR;		// Coordinate of lower right
	Vector2D	m_TexUL;	// Texcoords of upper left
	Vector2D	m_TexLR;	// Texcoords of lower left
};

struct DetailObjectLump_t
{
	Vector			m_Origin;
	QAngle			m_Angles;
	unsigned short	m_DetailModel;	// either index into DetailObjectDictLump_t or DetailPropSpriteLump_t
	unsigned short	m_Leaf;
	colorRGBExp32	m_Lighting;
	unsigned int	m_LightStyles; 
	unsigned char	m_LightStyleCount;
	unsigned char	m_Padding[3];	// FIXME: Remove when we rev the detail lump again..
	unsigned char	m_Orientation;	// See DetailPropOrientation_t
	unsigned char	m_Padding2[3];	// FIXME: Remove when we rev the detail lump again..
	unsigned char	m_Type;	// See DetailPropType_t
	unsigned char	m_Padding3[3];	// FIXME: Remove when we rev the detail lump again..
	float			m_flScale;	// For sprites only currently
};


//-----------------------------------------------------------------------------
// This is the data associated with the GAMELUMP_DETAIL_PROP_LIGHTING lump
//-----------------------------------------------------------------------------
struct DetailPropLightstylesLump_t
{
	colorRGBExp32	m_Lighting;
	unsigned char	m_Style;
};


//-----------------------------------------------------------------------------
// This is the data associated with the GAMELUMP_STATIC_PROPS lump
//-----------------------------------------------------------------------------
enum
{
	STATIC_PROP_NAME_LENGTH  = 128,

	// Flags field
	// These are automatically computed
	STATIC_PROP_FLAG_FADES	= 0x1,
	STATIC_PROP_USE_LIGHTING_ORIGIN	= 0x2,

	// These are set in WC
	STATIC_PROP_NO_SHADOW	= 0x10,

	// This mask includes all flags settable in WC
	STATIC_PROP_WC_MASK		= 0x10,
};

struct StaticPropDictLump_t
{
	char	m_Name[STATIC_PROP_NAME_LENGTH];		// model name
};

//-----------------------------------------------------------------------------
// StaticPropLump_t structures - version specific
// v4: Original HL2 Beta 2003 (LeakNet native)
// v5: HL2 Release 2004 (added ForcedFadeScale)
// v6: Source 2006/2007 (added MinDXLevel, MaxDXLevel)
// v7: Added color modulation
//-----------------------------------------------------------------------------

// v4 static prop structure (LeakNet native, 60 bytes)
struct StaticPropLump_t
{
	Vector		m_Origin;
	QAngle		m_Angles;
	unsigned short	m_PropType;
	unsigned short	m_FirstLeaf;
	unsigned short	m_LeafCount;
	unsigned char	m_Solid;
	unsigned char	m_Flags;
	int			m_Skin;
	float		m_FadeMinDist;
	float		m_FadeMaxDist;
	Vector		m_LightingOrigin;
//	int			m_Lighting;			// index into the GAMELUMP_STATIC_PROP_LIGHTING lump
};
typedef StaticPropLump_t StaticPropLump_v4_t;

// v5 static prop structure (HL2 Release 2004, 64 bytes)
// Added ForcedFadeScale for LOD control
struct StaticPropLump_v5_t
{
	Vector		m_Origin;
	QAngle		m_Angles;
	unsigned short	m_PropType;
	unsigned short	m_FirstLeaf;
	unsigned short	m_LeafCount;
	unsigned char	m_Solid;
	unsigned char	m_Flags;
	int			m_Skin;
	float		m_FadeMinDist;
	float		m_FadeMaxDist;
	Vector		m_LightingOrigin;
	float		m_flForcedFadeScale;	// v5+: Forced fade scale for LOD
};

// v6 static prop structure (Source 2006/2007, 68 bytes)
// Added DX level filtering
struct StaticPropLump_v6_t
{
	Vector		m_Origin;
	QAngle		m_Angles;
	unsigned short	m_PropType;
	unsigned short	m_FirstLeaf;
	unsigned short	m_LeafCount;
	unsigned char	m_Solid;
	unsigned char	m_Flags;
	int			m_Skin;
	float		m_FadeMinDist;
	float		m_FadeMaxDist;
	Vector		m_LightingOrigin;
	float		m_flForcedFadeScale;
	unsigned short	m_nMinDXLevel;		// v6+: Minimum DX level
	unsigned short	m_nMaxDXLevel;		// v6+: Maximum DX level
};

// v7 static prop structure (72 bytes)
// Added color modulation for tinting
struct StaticPropLump_v7_t
{
	Vector		m_Origin;
	QAngle		m_Angles;
	unsigned short	m_PropType;
	unsigned short	m_FirstLeaf;
	unsigned short	m_LeafCount;
	unsigned char	m_Solid;
	unsigned char	m_Flags;
	int			m_Skin;
	float		m_FadeMinDist;
	float		m_FadeMaxDist;
	Vector		m_LightingOrigin;
	float		m_flForcedFadeScale;
	unsigned short	m_nMinDXLevel;
	unsigned short	m_nMaxDXLevel;
	color32		m_DiffuseModulation;	// v7+: Per-prop color tint
};

// v8 static prop structure - same as v7 (unused transitional version)
typedef StaticPropLump_v7_t StaticPropLump_v8_t;

// v9 static prop structure (L4D1, 76 bytes)
// Same as v7 but with bDisableX360 flag
struct StaticPropLump_v9_t
{
	Vector		m_Origin;
	QAngle		m_Angles;
	unsigned short	m_PropType;
	unsigned short	m_FirstLeaf;
	unsigned short	m_LeafCount;
	unsigned char	m_Solid;
	unsigned char	m_Flags;
	int			m_Skin;
	float		m_FadeMinDist;
	float		m_FadeMaxDist;
	Vector		m_LightingOrigin;
	float		m_flForcedFadeScale;
	unsigned short	m_nMinDXLevel;
	unsigned short	m_nMaxDXLevel;
	color32		m_DiffuseModulation;
	bool		m_bDisableX360;			// v9+: Disable on Xbox 360
	unsigned char	m_Padding[3];		// Padding for alignment
};

// v10 static prop structure (L4D2/Portal 2, 76 bytes)
// Changed flags to 32-bit
struct StaticPropLump_v10_t
{
	Vector		m_Origin;
	QAngle		m_Angles;
	unsigned short	m_PropType;
	unsigned short	m_FirstLeaf;
	unsigned short	m_LeafCount;
	unsigned char	m_Solid;
	unsigned char	m_Flags;			// Keep 8-bit flags for backwards compat
	int			m_Skin;
	float		m_FadeMinDist;
	float		m_FadeMaxDist;
	Vector		m_LightingOrigin;
	float		m_flForcedFadeScale;
	unsigned short	m_nMinDXLevel;
	unsigned short	m_nMaxDXLevel;
	color32		m_DiffuseModulation;
	unsigned int	m_nFlagsEx;			// v10+: Extended 32-bit flags
};

// v11 static prop structure (CS:GO, 80 bytes)
// Added uniform scale factor
struct StaticPropLump_v11_t
{
	Vector		m_Origin;
	QAngle		m_Angles;
	unsigned short	m_PropType;
	unsigned short	m_FirstLeaf;
	unsigned short	m_LeafCount;
	unsigned char	m_Solid;
	unsigned char	m_Flags;
	int			m_Skin;
	float		m_FadeMinDist;
	float		m_FadeMaxDist;
	Vector		m_LightingOrigin;
	float		m_flForcedFadeScale;
	unsigned short	m_nMinDXLevel;
	unsigned short	m_nMaxDXLevel;
	color32		m_DiffuseModulation;
	unsigned int	m_nFlagsEx;
	float		m_flUniformScale;		// v11+: Uniform scale factor
};


struct StaticPropLeafLump_t
{
	unsigned short	m_Leaf;
};


//-----------------------------------------------------------------------------
// This is the data associated with the GAMELUMP_STATIC_PROP_LIGHTING lump
//-----------------------------------------------------------------------------
struct StaticPropLightstylesLump_t
{
	colorRGBExp32	m_Lighting;
};



#endif // GAMEBSPFILE_H