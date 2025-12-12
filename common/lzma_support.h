//===== Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: LZMA compression/decompression support for BSP and model files
//          Infrastructure for 2007 Source Engine compatibility
//
// $NoKeywords: $
//
//===========================================================================//

#ifndef LZMA_SUPPORT_H
#define LZMA_SUPPORT_H

#ifdef _WIN32
#pragma once
#endif

#include "basetypes.h"

//-----------------------------------------------------------------------------
// LZMA compression constants
//-----------------------------------------------------------------------------
#define LZMA_ID			(('A'<<24)+('M'<<16)+('Z'<<8)+'L')	// 'LZMA' magic number
#define LZMA_VERSION	1											// LZMA format version

//-----------------------------------------------------------------------------
// LZMA header structure for compressed data
//-----------------------------------------------------------------------------
struct lzmaheader_t
{
	uint32		id;					// LZMA_ID
	uint32		actualSize;			// Size of uncompressed data
	uint32		lzmaSize;			// Size of compressed LZMA data
	byte		properties[5];		// LZMA encoder properties
};

//-----------------------------------------------------------------------------
// LZMA decompression functions
// TODO: Implement actual LZMA decompression using 7-Zip LZMA SDK
//-----------------------------------------------------------------------------

// Check if data is LZMA compressed
inline bool IsLZMACompressed( const byte *pData )
{
	if ( !pData )
		return false;

	const lzmaheader_t *pHeader = (const lzmaheader_t *)pData;
	return ( pHeader->id == LZMA_ID );
}

// Get uncompressed size of LZMA data
inline uint32 GetLZMAUncompressedSize( const byte *pData )
{
	if ( !IsLZMACompressed( pData ) )
		return 0;

	const lzmaheader_t *pHeader = (const lzmaheader_t *)pData;
	return pHeader->actualSize;
}

//-----------------------------------------------------------------------------
// LZMA decompression stub - needs actual implementation
//-----------------------------------------------------------------------------
// TODO: Implement using 7-Zip LZMA SDK
// Returns: true on success, false on failure
// pCompressed: Input compressed data (including LZMA header)
// compressedSize: Size of compressed data
// pUncompressed: Output buffer (must be at least GetLZMAUncompressedSize() bytes)
// uncompressedSize: Size of output buffer
bool LZMA_Decompress( const byte *pCompressed, uint32 compressedSize,
					  byte *pUncompressed, uint32 uncompressedSize );

//-----------------------------------------------------------------------------
// Integration points for BSP/model loading
//-----------------------------------------------------------------------------

// Check if a BSP lump is LZMA compressed and decompress if needed
// Returns: Pointer to uncompressed data (allocated if decompressed, original if not)
//          Caller must free returned data if it's different from pOriginalData
byte* DecompressBSPLumpIfNeeded( const byte *pOriginalData, uint32 originalSize, uint32 *pUncompressedSize );

// Check if model vertex data is LZMA compressed and decompress if needed
// Returns: Pointer to uncompressed data (allocated if decompressed, original if not)
//          Caller must free returned data if it's different from pOriginalData
byte* DecompressModelDataIfNeeded( const byte *pOriginalData, uint32 originalSize, uint32 *pUncompressedSize );

#endif // LZMA_SUPPORT_H