//===== Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: LZMA compression/decompression support for BSP and model files
//          Infrastructure for 2007 Source Engine compatibility
//
// $NoKeywords: $
//
//===========================================================================//

#include "lzma_support.h"
#include "tier0/dbg.h"

//-----------------------------------------------------------------------------
// LZMA decompression implementation
//-----------------------------------------------------------------------------
#include "lzmalib.h"

bool LZMA_Decompress( const byte *pCompressed, uint32 compressedSize,
					  byte *pUncompressed, uint32 uncompressedSize )
{
	if ( !pCompressed || !pUncompressed || compressedSize == 0 || uncompressedSize == 0 )
	{
		Warning( "LZMA_Decompress: Invalid parameters\n" );
		return false;
	}

	if ( !IsLZMACompressed( pCompressed ) )
	{
		Warning( "LZMA_Decompress: Data is not LZMA compressed\n" );
		return false;
	}

	const lzmaheader_t *pHeader = (const lzmaheader_t *)pCompressed;

	// Validate header
	if ( pHeader->actualSize != uncompressedSize )
	{
		Warning( "LZMA_Decompress: Uncompressed size mismatch. Expected %u, header says %u\n",
				 uncompressedSize, pHeader->actualSize );
		return false;
	}

	if ( compressedSize < sizeof(lzmaheader_t) + pHeader->lzmaSize )
	{
		Warning( "LZMA_Decompress: Compressed data too small. Need %u, have %u\n",
				 (uint32)(sizeof(lzmaheader_t) + pHeader->lzmaSize), compressedSize );
		return false;
	}

	// Extract LZMA data and properties
	const byte *pLZMAData = pCompressed + sizeof(lzmaheader_t);
	size_t destLen = uncompressedSize;
	size_t srcLen = pHeader->lzmaSize;

	// Perform decompression
	int result = LzmaUncompress( pUncompressed, &destLen,
								 pLZMAData, &srcLen,
								 pHeader->properties, 5 );

	if ( result != SZ_OK )
	{
		const char *errorStr = "Unknown error";
		switch ( result )
		{
		case SZ_ERROR_DATA:        errorStr = "Data error"; break;
		case SZ_ERROR_MEM:         errorStr = "Memory allocation error"; break;
		case SZ_ERROR_UNSUPPORTED: errorStr = "Unsupported format"; break;
		case SZ_ERROR_PARAM:       errorStr = "Parameter error"; break;
		case SZ_ERROR_INPUT_EOF:   errorStr = "Unexpected end of input"; break;
		}

		Warning( "LZMA_Decompress: Decompression failed with error %d (%s)\n", result, errorStr );
		return false;
	}

	if ( destLen != uncompressedSize )
	{
		Warning( "LZMA_Decompress: Decompressed size mismatch. Expected %u, got %u\n",
				 uncompressedSize, (uint32)destLen );
		return false;
	}

	// Success
	return true;
}

//-----------------------------------------------------------------------------
// BSP lump decompression helper
//-----------------------------------------------------------------------------
byte* DecompressBSPLumpIfNeeded( const byte *pOriginalData, uint32 originalSize, uint32 *pUncompressedSize )
{
	if ( !pOriginalData || !pUncompressedSize )
		return NULL;

	// Check if data is LZMA compressed
	if ( !IsLZMACompressed( pOriginalData ) )
	{
		// Not compressed, return original data
		*pUncompressedSize = originalSize;
		return (byte*)pOriginalData;
	}

	// Get uncompressed size
	uint32 uncompressedSize = GetLZMAUncompressedSize( pOriginalData );
	if ( uncompressedSize == 0 )
	{
		Warning( "DecompressBSPLumpIfNeeded: Invalid LZMA header\n" );
		return NULL;
	}

	// Allocate buffer for uncompressed data
	byte *pUncompressed = new byte[uncompressedSize];
	if ( !pUncompressed )
	{
		Warning( "DecompressBSPLumpIfNeeded: Failed to allocate %u bytes for decompression\n", uncompressedSize );
		return NULL;
	}

	// Decompress the data
	if ( !LZMA_Decompress( pOriginalData, originalSize, pUncompressed, uncompressedSize ) )
	{
		delete[] pUncompressed;
		Warning( "DecompressBSPLumpIfNeeded: LZMA decompression failed\n" );
		return NULL;
	}

	*pUncompressedSize = uncompressedSize;
	return pUncompressed;
}

//-----------------------------------------------------------------------------
// Model data decompression helper
//-----------------------------------------------------------------------------
byte* DecompressModelDataIfNeeded( const byte *pOriginalData, uint32 originalSize, uint32 *pUncompressedSize )
{
	if ( !pOriginalData || !pUncompressedSize )
		return NULL;

	// Check if data is LZMA compressed
	if ( !IsLZMACompressed( pOriginalData ) )
	{
		// Not compressed, return original data
		*pUncompressedSize = originalSize;
		return (byte*)pOriginalData;
	}

	// Get uncompressed size
	uint32 uncompressedSize = GetLZMAUncompressedSize( pOriginalData );
	if ( uncompressedSize == 0 )
	{
		Warning( "DecompressModelDataIfNeeded: Invalid LZMA header\n" );
		return NULL;
	}

	// Allocate buffer for uncompressed data
	byte *pUncompressed = new byte[uncompressedSize];
	if ( !pUncompressed )
	{
		Warning( "DecompressModelDataIfNeeded: Failed to allocate %u bytes for decompression\n", uncompressedSize );
		return NULL;
	}

	// Decompress the data
	if ( !LZMA_Decompress( pOriginalData, originalSize, pUncompressed, uncompressedSize ) )
	{
		delete[] pUncompressed;
		Warning( "DecompressModelDataIfNeeded: LZMA decompression failed\n" );
		return NULL;
	}

	*pUncompressedSize = uncompressedSize;
	return pUncompressed;
}