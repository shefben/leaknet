# V37/V44+ Model System Separation Architecture

## Overview

This document outlines the architectural plan for completely separating v37 (HL2 Beta 2003) model code from v44+ (Source Engine 2004+) model code. The goal is to ensure v44+ code matches the 2007 Source Engine exactly, allowing direct code copying from the reference implementation.

## Current State Analysis

### CRITICAL PROBLEM: Invalid Binary Casts

The current `studiorender/studio_v37_compat.h` contains a fundamentally broken approach:

```cpp
// studio_v37_compat.h:50 - THIS IS WRONG!
outMesh44 = reinterpret_cast<const mstudiomesh_v44_t*>(pMesh);
```

**Why this is wrong:**
- v37 `mstudiomesh_t` has a **completely different binary layout** than v44+ `mstudiomesh_v44_t`
- Field offsets don't match - accessing fields via the cast pointer reads garbage
- This causes invisible models, crashes, and corruption

### Binary Layout Differences

| Structure | v37 Size | v44+ Size | Key Differences |
|-----------|----------|-----------|-----------------|
| `mstudiovertex` | 64 bytes | 48 bytes | Different bone weight layout |
| `mstudioboneweight` | 32 bytes | 16 bytes | v37: `short bone[4]`, v44+: `char bone[3]` |
| `mstudiobone` | N/A | N/A | v37: `value[6]/scale[6]`, v44+: `pos/quat/rot/posscale/rotscale` |
| `mstudiomodel` | ~136 bytes | 148 bytes | v44+ has `mstudio_modelvertexdata_t` |
| `mstudiomesh` | Embedded | External | v44+ uses VVD vertex files |
| `mstudioseqdesc` | Different | Different | v44+ has additional fields (animblocks) |

### Animation Data Differences

| Feature | v37 | v44+ |
|---------|-----|------|
| Animation storage | Embedded in MDL | External .ani blocks |
| Rotation format | Raw Euler angles | Quaternion48/64 compressed |
| Position format | Raw vectors | Vector48 compressed |
| IK system | Basic | Sophisticated constraints |
| Flex/Morph | Software only | Hardware support |

## Files Requiring Separation

### 1. StudioRender (Critical - Most Problems)

**Current problematic files:**
- `studiorender/studio_v37_compat.h` - Dispatch layer with invalid casts
- `studiorender/r_studiodraw.cpp` - Mixed v37/v44+ inline code
- `studiorender/r_studioflex.cpp` - Mixed version checks
- `studiorender/r_studiodecal.cpp` - Mixed version checks
- `studiorender/r_studio.cpp` - Mixed version access

**Solution:** Create completely separate rendering paths:

```
studiorender/
├── r_studiodraw_v37.cpp      # v37-only rendering (copy from original LeakNet)
├── r_studiodraw_v44.cpp      # v44+-only rendering (copy from 2007 engine)
├── r_studioflex_v37.cpp      # v37-only flex
├── r_studioflex_v44.cpp      # v44+-only flex (copy from 2007 engine)
├── r_studiodecal_v37.cpp     # v37-only decals
├── r_studiodecal_v44.cpp     # v44+-only decals (copy from 2007 engine)
└── cstudiorender.cpp         # Top-level dispatch only
```

### 2. Animation System (bone_setup.cpp)

**Current state:** Already partially separated with:
- `CalcRotations_v37()` / `CalcRotations_v48_internal()`
- `CalcBoneQuaternion()` / `CalcBoneQuaternion_v48()`
- `CalcBonePosition()` / `CalcBonePosition_v48()`

**Remaining work:**
- Extract v44+ functions to `bone_setup_v44.cpp`
- Ensure v44+ code matches 2007 engine exactly
- Implement `CStudioHdr` wrapper class from 2007 engine

### 3. Engine Model Loading

**Current state:** Has separate `modelloader_v44.cpp/h` with:
- `model_v44_t` - Separate model structure
- `CModelLoader_v44` - Separate loader class
- `LoadVvdFile_v44()`, `LoadVtxFile_v44()` - External file loading

**Remaining work:**
- Complete VVD loading with proper vertex data setup
- Complete VTX loading with strip/stripgroup parsing
- Implement animation block demand-loading (.ani files)
- Wire into studiorender v44+ path

### 4. Public Headers

**Current headers:**
- `public/studio.h` - Mixed v37/v44+ structures (problematic)
- `public/studiohdr_v44.h` - Attempted v44+ isolation

**Solution:**
```
public/
├── studio_v37.h              # v37-only structures (original LeakNet types)
├── studio_v44.h              # v44+-only structures (2007 engine types)
├── studio.h                  # Version dispatch macros only
└── bone_setup.h              # Split into v37/v44+ versions
```

## Target Architecture

### Dispatch Strategy

High-level dispatch at entry points only - no inline version checks:

```cpp
// cstudiorender.cpp - TOP LEVEL DISPATCH ONLY
void CStudioRender::DrawModel(...)
{
    if (pStudioHdr->IsV37())
    {
        DrawModel_V37(...);  // Completely separate v37 code path
    }
    else // v44+
    {
        DrawModel_V44(...);  // Completely separate v44+ code path (2007 engine)
    }
}
```

### V44+ Code Path (Copy from 2007 Engine)

The v44+ code should be a near-copy of the 2007 Source Engine with minimal modifications:

**Key 2007 Engine components to copy:**
1. `CStudioHdr` wrapper class (multi-model composition)
2. `mstudio_meshvertexdata_t` / `mstudio_modelvertexdata_t` accessors
3. Animation extraction with Quaternion48/64, Vector48
4. Hardware flex/morph support
5. Linear bone optimization
6. External animation block loading

### V37 Code Path (Original LeakNet)

Keep original LeakNet code with minimal changes:
- Use original `studiohdr_t` (37) directly
- Use embedded vertex data with 64-byte stride
- Use 32-byte bone weights with `short bone[4]`
- Use `value[6]/scale[6]` bone defaults

## Implementation Plan

### Phase 1: Create Separate Headers
1. Create `public/studio_v37.h` with v37-only structures
2. Create `public/studio_v44.h` with v44+-only structures (from 2007 engine)
3. Update `public/studio.h` to include appropriate header based on runtime version

### Phase 2: Separate StudioRender
1. Create `r_studiodraw_v37.cpp` (copy from original LeakNet)
2. Create `r_studiodraw_v44.cpp` (copy from 2007 engine `r_studiodraw.cpp`)
3. Create dispatch layer in `cstudiorender.cpp`
4. Repeat for `r_studioflex`, `r_studiodecal`, `r_studio`

### Phase 3: Separate Animation System
1. Create `bone_setup_v37.cpp` with v37-only animation code
2. Create `bone_setup_v44.cpp` copying 2007 engine `bone_setup.cpp`
3. Implement `CStudioHdr` wrapper class
4. Update dispatch in shared animation functions

### Phase 4: Complete V44+ Model Loading
1. Complete `CModelLoader_v44::LoadVvdFile_v44()` with proper vertex setup
2. Complete `CModelLoader_v44::LoadVtxFile_v44()` with strip parsing
3. Implement animation block demand-loading
4. Wire v44+ vertex data into v44+ studiorender path

### Phase 5: Testing & Validation
1. Test v37 models (HL2 Beta assets) - should work as before
2. Test v44+ models (HL2 retail assets) - should render correctly
3. Test mixed sessions (v37 + v44+ models loaded simultaneously)

## 2007 Engine Reference Files

Files to copy/reference from `F:\back-ups\betahl2_codebases\SourceEngine2007-master\src_main`:

### StudioRender
- `studiorender/r_studiodraw.cpp` (2432 lines)
- `studiorender/r_studioflex.cpp`
- `studiorender/r_studiodecal.cpp`
- `studiorender/r_studio.cpp`
- `studiorender/cstudiorender.cpp`

### Animation
- `public/bone_setup.cpp` (full v48 implementation)
- `public/bone_setup.h` (CStudioHdr, CalcPose, AccumulatePose)
- `public/studio_virtualmodel.cpp` (virtual model composition)

### Headers
- `public/studio.h` (v48 structures)
- `public/optimize.h` (VTX file format)

## Key Structures from 2007 Engine

### CStudioHdr (CRITICAL for v44+)
```cpp
class CStudioHdr
{
    const studiohdr_t *m_pStudioHdr;
    virtualmodel_t *m_pVModel;
    CUtlVector<const studiohdr_t*> m_pStudioHdrCache;

public:
    inline int numbones() { return m_pStudioHdr->numbones; }
    inline mstudiobone_t *pBone(int i);
    int RemapAnimBone(int iAnim, int iLocalBone);
    int RemapSeqBone(int iSequence, int iLocalBone);
};
```

### Vertex Data Accessors (v44+)
```cpp
struct mstudio_meshvertexdata_t
{
    const mstudiovertex_t *Vertex(int i) const;
    const Vector *Position(int i) const;
    const Vector *Normal(int i) const;
    const Vector4D *TangentS(int i) const;
    const Vector2D *Texcoord(int i) const;
    const mstudioboneweight_t *BoneWeights(int i) const;
};
```

### Animation Compression (v44+)
```cpp
struct mstudioanim_t
{
    byte bone;
    byte flags;  // STUDIO_ANIM_RAWPOS, RAWROT, ANIMPOS, ANIMROT, DELTA, RAWROT2

    Quaternion48 *pQuat48();   // Compressed rotation
    Quaternion64 *pQuat64();   // Full precision rotation
    Vector48 *pPos();          // Compressed position
    mstudioanim_valueptr_t *pRotV();  // Animation stream
    mstudioanim_valueptr_t *pPosV();  // Animation stream
};
```

## Summary

The key principle is **complete isolation**:
- v37 code should not call v44+ functions
- v44+ code should not call v37 functions
- No `reinterpret_cast` between v37 and v44+ types
- Dispatch only at top level (model loading, render entry points)
- v44+ code should be copy-paste from 2007 engine where possible

This ensures:
1. v37 models work exactly as in original LeakNet
2. v44+ models work exactly as in 2007 Source Engine
3. Both can be loaded in the same game session
4. Future 2007 engine code updates can be applied directly to v44+ path
