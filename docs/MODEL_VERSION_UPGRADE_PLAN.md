# Model Version Upgrade Plan: v37 to v48 Support

## Executive Summary

This document outlines the implementation plan for adding Source Engine 2007 (v48) model support to the LeakNet 2003 engine while maintaining backward compatibility with the original v37 models.

## Version Comparison

| Component | 2003 LeakNet | 2007 Source | Notes |
|-----------|--------------|-------------|-------|
| MDL Version | 37 | 48 | Major structure changes |
| VTX Version | 6 | 7 | Minor changes |
| VVD Version | N/A | 4 | NEW: External vertex data |
| Min Supported | ~35 | 44 | |

## Architecture Changes Overview

### 1. File Format Changes

#### MDL Files (.mdl)
- **v37**: Vertex data embedded in MDL file within `mstudiomodel_t`
- **v48**: Vertex data external in VVD files, MDL contains only offsets

#### VTX Files (.vtx / .dx80.vtx / .dx90.vtx)
- **v6 (2003)**: Basic optimized mesh data
- **v7 (2007)**: Added topology data for hardware morphing

#### VVD Files (.vvd) - NEW in v48
- External vertex data storage
- Supports LOD vertex culling
- Contains: positions, normals, tangents, texcoords, bone weights

### 2. Major Structure Differences

#### studiohdr_t Changes

```cpp
// Fields RENAMED (v37 -> v48)
numanim          -> numlocalanim
animdescindex    -> localanimindex
numseq           -> numlocalseq
seqindex         -> localseqindex
numattachments   -> numlocalattachments
attachmentindex  -> localattachmentindex
numposeparameters -> numlocalposeparameters
poseparamindex   -> localposeparamindex
numikautoplaylocks -> numlocalikautoplaylocks
ikautoplaylockindex -> localikautoplaylockindex

// Fields REMOVED in v48
numanimgroups, animgroupindex     // v37-specific animation groups
numbonedescs, bonedescindex       // v37-specific bone descriptions
numseqgroups, seqgroupindex       // Replaced by include models
sequencesindexed                  // Replaced by activitylistversion

// Fields ADDED in v48
numincludemodels, includemodelindex  // External model includes
virtualModel                          // Virtual model pointer
szanimblocknameindex                  // Animation block name
numanimblocks, animblockindex         // Demand-loaded animations
animblockModel                        // Animation block model pointer
bonetablebynameindex                  // Sorted bone lookup
pVertexBase, pIndexBase               // Tool vertex/index pointers
constdirectionallightdot              // Static prop lighting
rootLOD, numAllowedRootLODs           // LOD configuration
numflexcontrollerui                   // Flex controller UI
studiohdr2index                       // Secondary header extension
```

#### mstudiobone_t Changes

```cpp
// v37 Structure
struct mstudiobone_t_v37 {
    int sznameindex;
    int parent;
    int bonecontroller[6];
    float value[6];           // Default DoF values
    float scale[6];           // Scale for delta DoF
    matrix3x4_t poseToBone;
    Quaternion qAlignment;
    int flags;
    int proctype;
    int procindex;
    int physicsbone;
    int surfacepropidx;
    Quaternion quat;
    int contents;
    int unused[3];
};

// v48 Structure
struct mstudiobone_t_v48 {
    int sznameindex;
    int parent;
    int bonecontroller[6];
    Vector pos;               // Direct position (was value[0-2])
    Quaternion quat;          // Moved up
    RadianEuler rot;          // NEW: Explicit rotation
    Vector posscale;          // Simplified from scale[0-2]
    Vector rotscale;          // NEW: Rotation scale
    matrix3x4_t poseToBone;
    Quaternion qAlignment;
    int flags;
    int proctype;
    int procindex;
    int physicsbone;
    int surfacepropidx;
    int contents;
    int unused[8];
};
```

### 3. New Structures in v48

```cpp
// Secondary header for extensibility
struct studiohdr2_t {
    int numsrcbonetransform;
    int srcbonetransformindex;
    int illumpositionattachmentindex;
    float flMaxEyeDeflection;
    int linearboneindex;
    int reserved[59];
};

// Optimized linear bone storage
struct mstudiolinearbone_t {
    int numbones;
    int flagsindex;
    int parentindex;
    int posindex;
    int quatindex;
    int rotindex;
    int posetoboneindex;
    int posscaleindex;
    int rotscaleindex;
    int qalignmentindex;
    int unused[6];
};

// Jiggle bone physics
struct mstudiojigglebone_t {
    int flags;
    float length, tipMass;
    // ... extensive physics parameters
};

// Aim-at bone procedural
struct mstudioaimatbone_t {
    int parent, aim;
    Vector aimvector, upvector, basepos;
};

// External vertex file header
struct vertexFileHeader_t {
    int id;                    // MODEL_VERTEX_FILE_ID
    int version;               // MODEL_VERTEX_FILE_VERSION (4)
    long checksum;
    int numLODs;
    int numLODVertexes[MAX_NUM_LODS];
    int numFixups;
    int fixupTableStart;
    int vertexDataStart;
    int tangentDataStart;
};

// Animation demand loading
struct mstudioanimblock_t {
    int datastart;
    int dataend;
};

// Virtual model compositing
struct virtualmodel_t {
    CUtlVector<virtualsequence_t> m_seq;
    CUtlVector<virtualgeneric_t> m_anim;
    CUtlVector<virtualgroup_t> m_group;
    // ... more
};
```

---

## Implementation Plan

### Phase 1: Foundation (Header Files)

#### Task 1.1: Create Version-Aware Headers
**Files to modify:**
- `public/studio.h`

**Actions:**
1. Add version constants:
```cpp
#define STUDIO_VERSION_37    37
#define STUDIO_VERSION_48    48
#define STUDIO_VERSION_MIN   37
#define STUDIO_VERSION_MAX   48

// Support loading both versions
#define STUDIO_VERSION       STUDIO_VERSION_48
```

2. Create dual structure definitions with version suffixes
3. Add `studiohdr2_t` structure
4. Add `vertexFileHeader_t` structure
5. Add `mstudiolinearbone_t`, `mstudiojigglebone_t`, `mstudioaimatbone_t`

#### Task 1.2: Create CStudioHdr Wrapper Class
**New file:** `public/cstudiohdr.h`

```cpp
class CStudioHdr {
public:
    CStudioHdr(const studiohdr_t *pStudioHdr);

    // Version detection
    int GetVersion() const { return m_nVersion; }
    bool IsV37() const { return m_nVersion == 37; }
    bool IsV48() const { return m_nVersion >= 44; }

    // Unified accessors (handle both versions)
    int GetNumBones() const;
    const mstudiobone_t* pBone(int i) const;
    int GetNumLocalAnims() const;
    int GetNumLocalSeqs() const;
    // ... etc

private:
    const studiohdr_t *m_pStudioHdr;
    int m_nVersion;
    virtualmodel_t *m_pVModel;
};
```

#### Task 1.3: Update VTX Format
**File:** `common/optimize.h`

```cpp
#define OPTIMIZED_MODEL_FILE_VERSION_V6  6
#define OPTIMIZED_MODEL_FILE_VERSION_V7  7
#define OPTIMIZED_MODEL_FILE_VERSION     OPTIMIZED_MODEL_FILE_VERSION_V7

// Add topology data structures for v7
struct TopologyDataHeader_t { ... };
```

---

### Phase 2: Model Loading

#### Task 2.1: Add VVD File Support
**Files to modify:**
- `engine/l_studio.cpp`
- `engine/modelloader.cpp`

**Actions:**
1. Add VVD file loading function:
```cpp
bool LoadVertexFile(const char* pModelName, vertexFileHeader_t** ppVvdHdr);
```

2. Modify `Mod_LoadStudioModel()` to:
   - Detect model version
   - For v48: Load external VVD file
   - For v37: Use embedded vertex data

#### Task 2.2: Version Conversion
**File:** `public/studio.h` (inline function)

```cpp
inline bool Studio_ConvertStudioHdrToNewVersion(studiohdr_t *pStudioHdr)
{
    int version = pStudioHdr->version;

    // v37 models need field remapping
    if (version == 37) {
        // Map v37 fields to v48 equivalents
        // This is complex due to structure size differences
        return Studio_ConvertV37ToV48(pStudioHdr);
    }

    // v44-47 compatible with v48
    if (version >= 44 && version < 48) {
        // Apply incremental fixups
        // ... (same as 2007 source)
    }

    if (version == 48) {
        return true;
    }

    return false;  // Unsupported version
}
```

#### Task 2.3: Dual Loading Path
**File:** `engine/l_studio.cpp`

```cpp
bool Mod_LoadStudioModel(model_t *mod, void *buffer, bool zerostructure)
{
    studiohdr_t *phdr = (studiohdr_t *)buffer;
    int version = phdr->version;

    // Version range check
    if (version < STUDIO_VERSION_MIN || version > STUDIO_VERSION_MAX) {
        Warning("%s has unsupported version %d\n", mod->name, version);
        return false;
    }

    // v48 path: Load external files
    if (version >= 44) {
        // Load VVD file
        vertexFileHeader_t *pVvdHdr = NULL;
        if (!LoadVertexFile(mod->name, &pVvdHdr)) {
            Warning("Failed to load VVD for %s\n", mod->name);
            return false;
        }

        // Load VTX file (v7)
        // ...
    }
    // v37 path: Embedded vertex data
    else {
        // Original loading code
        // ...
    }

    return true;
}
```

---

### Phase 3: StudioRender Updates

#### Task 3.1: Update IStudioRender Interface
**File:** `public/istudiorender.h`

```cpp
// Add method to handle external vertex data
virtual bool LoadModel_V48(
    studiohdr_t *pStudioHdr,
    vertexFileHeader_t *pVvdHdr,
    OptimizedModel::FileHeader_t *pVtxHdr,
    studiohwdata_t *pStudioHWData
) = 0;

// Version-aware initialization
virtual bool LoadModel(
    studiohdr_t *pStudioHdr,
    void *pVtxHdr,
    void *pVvdHdr,  // NULL for v37
    studiohwdata_t *pStudioHWData
) = 0;
```

#### Task 3.2: Update Mesh Building
**File:** `studiorender/r_studio.cpp`

Modify mesh building to:
1. Check for external vertex data (VVD)
2. Use `vertexFileHeader_t` for vertex/tangent data access
3. Fall back to embedded data for v37 models

#### Task 3.3: Update Material Loading
**File:** `studiorender/r_studio.cpp`

No major changes needed - material system is version-agnostic.

---

### Phase 4: Animation System Updates

#### Task 4.1: Add Animation Block Support
**Files:**
- `public/studio.h`
- `engine/l_studio.cpp`

```cpp
// Demand-loaded animation block support
struct AnimBlockLoader {
    bool LoadAnimBlock(studiohdr_t *pHdr, int blockIndex);
    byte* GetAnimBlock(studiohdr_t *pHdr, int blockIndex);
};
```

#### Task 4.2: Virtual Model Support
**New file:** `public/virtualmodel.h`

Implement `virtualmodel_t` for model compositing (include models).

---

### Phase 5: Bone System Updates

#### Task 5.1: Add New Procedural Bone Types
**File:** `dlls/bone_setup.cpp` and `cl_dll/c_baseanimating.cpp`

```cpp
// Add handlers for new procedural types
#define STUDIO_PROC_AIMATBONE   3
#define STUDIO_PROC_AIMATATTACH 4
#define STUDIO_PROC_JIGGLE      5

void CalcProceduralBone_AimAt(...);
void CalcProceduralBone_Jiggle(...);
```

#### Task 5.2: Linear Bone Optimization
Support `mstudiolinearbone_t` for optimized bone access in v48 models.

---

### Phase 6: Tools Updates

#### Task 6.1: Update studiomdl
**Files:** `utils/studiomdl/`

- Add v48 MDL output support
- Add VVD file generation
- Add VTX v7 generation

#### Task 6.2: Update hlmviewer
**Files:** `utils/hlmviewer/`

- Support loading both v37 and v48 models
- Display version information

---

## File Changes Summary

### New Files
| File | Purpose |
|------|---------|
| `public/cstudiohdr.h` | Unified studio header wrapper |
| `public/virtualmodel.h` | Virtual model compositing |
| `public/studio_v37.h` | Legacy v37 structure definitions |
| `engine/vvdloader.cpp` | VVD file loading |

### Modified Files
| File | Changes |
|------|---------|
| `public/studio.h` | Dual version support, new structures |
| `public/istudiorender.h` | New interface methods |
| `common/optimize.h` | VTX v7 support |
| `engine/l_studio.cpp` | Version-aware loading |
| `engine/modelloader.cpp` | VVD file handling |
| `studiorender/r_studio.cpp` | External vertex data |
| `studiorender/cstudiorender.cpp` | New interface implementation |
| `dlls/bone_setup.cpp` | New procedural types |

---

## Implementation Priority

### Critical (Must Have)
1. Version detection and routing
2. `studiohdr_t` dual support
3. VVD file loading for v48
4. Basic CStudioHdr wrapper

### Important (Should Have)
1. New procedural bone types
2. Animation block loading
3. Full VTX v7 support

### Optional (Nice to Have)
1. Virtual model compositing
2. Linear bone optimization
3. Flex controller UI

---

## Testing Strategy

### Unit Tests
1. Version detection accuracy
2. Structure size validation
3. Field offset verification

### Integration Tests
1. Load v37 model -> Render correctly
2. Load v48 model -> Render correctly
3. Mix v37 and v48 models in scene

### Asset Tests
1. All original HL2 beta models (v37)
2. HL2 retail models (v48)
3. EP1/EP2/TF2 models (v48)

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| Structure size mismatch | High | Extensive size validation |
| Animation playback issues | Medium | Fallback to v37 behavior |
| Vertex data corruption | High | Checksum validation |
| Performance regression | Low | Profile both paths |

---

## Timeline Estimate

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| Phase 1: Foundation | 1-2 weeks | None |
| Phase 2: Model Loading | 2-3 weeks | Phase 1 |
| Phase 3: StudioRender | 2-3 weeks | Phase 2 |
| Phase 4: Animation | 1-2 weeks | Phase 2 |
| Phase 5: Bones | 1 week | Phase 1 |
| Phase 6: Tools | 2 weeks | Phase 1-5 |
| Testing & Polish | 2 weeks | All |

**Total: 10-14 weeks**

---

## Appendix A: Structure Size Comparison

| Structure | v37 Size | v48 Size | Delta |
|-----------|----------|----------|-------|
| `studiohdr_t` | ~560 bytes | ~408 bytes* | -152 |
| `mstudiobone_t` | 216 bytes | 216 bytes | 0 |
| `mstudioseqdesc_t` | ~200 bytes | ~220 bytes | +20 |

*v48 uses studiohdr2_t extension for additional fields

---

## Appendix B: Version History Reference

| Version | Engine | Features |
|---------|--------|----------|
| 35 | HL2 Beta early | Basic features |
| 36 | HL2 Beta late | Hitbox improvements |
| 37 | HL2 Beta final | Animation groups |
| 44 | HL2 Retail | External vertex data |
| 45 | HL2 Update | Animation sections |
| 46 | EP1 | Animation blocks |
| 47 | EP2 | Zero frame caching |
| 48 | Orange Box/TF2 | Final format |

