# LZMA Decompression Implementation TODO

This document outlines what's needed to complete LZMA decompression support for 2007 Source Engine compatibility.

## Status: Infrastructure Added ✅

The basic infrastructure for LZMA support has been added:

- `common/lzma_support.h` - Header with LZMA constants, structures, and function declarations
- `common/lzma_support.cpp` - Stub implementation with detailed TODOs
- Documentation for integration points

## What Still Needs Implementation

### 1. LZMA SDK Integration

**Download and Setup:**
- Download 7-Zip LZMA SDK from https://7-zip.org/sdk.html
- Extract to `external/lzma/` or similar directory
- Add include paths to build system

**Required Files:**
- `LzmaLib.h` - Main LZMA library header
- `LzmaLib.c` or `LzmaLib.lib` - Implementation
- Supporting LZMA SDK files

### 2. Build System Changes

**CMake Integration (CMakeLists.txt):**
```cmake
# Add LZMA SDK
set(LZMA_SDK_DIR "${CMAKE_SOURCE_DIR}/external/lzma")
include_directories("${LZMA_SDK_DIR}")

# Link LZMA library
target_link_libraries(engine lzma)
```

**Visual Studio Integration:**
- Add LZMA SDK include paths to project settings
- Link against LzmaLib.lib
- Ensure 32-bit compatibility (project is 32-bit)

### 3. Complete LZMA_Decompress() Implementation

Replace the stub in `lzma_support.cpp`:

```cpp
#include "LzmaLib.h"

bool LZMA_Decompress( const byte *pCompressed, uint32 compressedSize,
                      byte *pUncompressed, uint32 uncompressedSize )
{
    if ( !IsLZMACompressed( pCompressed ) )
        return false;

    const lzmaheader_t *pHeader = (const lzmaheader_t *)pCompressed;
    const byte *pLZMAData = pCompressed + sizeof(lzmaheader_t);

    size_t destLen = uncompressedSize;
    size_t srcLen = pHeader->lzmaSize;

    int result = LzmaUncompress( pUncompressed, &destLen,
                                 pLZMAData, &srcLen,
                                 pHeader->properties, 5 );

    return (result == SZ_OK && destLen == uncompressedSize);
}
```

### 4. Integration Points

**BSP File Loading:**
- Modify BSP lump loading functions to call `DecompressBSPLumpIfNeeded()`
- Check for compressed lumps before processing
- Handle memory management for decompressed data

**Model File Loading:**
- Modify VVD (vertex data) loading to call `DecompressModelDataIfNeeded()`
- Support compressed animation data
- Support compressed vertex data

**Key Files to Modify:**
- `engine/cmodel_bsp.cpp` - BSP loading
- `engine/modelloader.cpp` - Model loading
- `studiorender/` - Model rendering

### 5. Testing

**Test Data:**
- Need compressed BSP files from 2007+ Source games
- Need compressed model files (VVD, ANI)
- Verify decompression produces correct results

**Test Cases:**
- Uncompressed data (should pass through unchanged)
- LZMA compressed data (should decompress correctly)
- Invalid/corrupted compressed data (should fail gracefully)
- Memory leak testing

### 6. Error Handling

**Robust Error Handling:**
- Validate LZMA headers before decompression
- Handle decompression failures gracefully
- Provide meaningful error messages
- Memory cleanup on failure

### 7. Performance Considerations

**Optimization:**
- Cache decompressed data when appropriate
- Avoid redundant decompressions
- Consider async decompression for large files

## Implementation Priority

1. **High Priority:** LZMA SDK integration and basic decompression
2. **Medium Priority:** BSP lump decompression integration
3. **Medium Priority:** Model data decompression integration
4. **Low Priority:** Performance optimizations and caching

## Compatibility Notes

- Maintain backward compatibility with uncompressed data
- Support both v37 (uncompressed) and v48+ (potentially compressed) formats
- Graceful fallback when LZMA is not available or fails

## Resources

- [7-Zip LZMA SDK Documentation](https://7-zip.org/sdk.html)
- [LZMA File Format Specification](https://7-zip.org/recover.html)
- Source Engine 2007 reference implementation (if available)

## Current State

✅ LZMA infrastructure and stubs added
❌ LZMA SDK not integrated
❌ Actual decompression not implemented
❌ BSP/Model loading not updated
❌ Testing not performed

The groundwork is complete - actual LZMA decompression can now be implemented by following the steps above.