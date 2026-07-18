# V44+ MODEL SUPPORT IMPLEMENTATION GAPS IN LEAKNET-REWRITE

## EXECUTIVE SUMMARY

LeakNet-rewrite has established a solid architectural foundation for v44+ model support with:
- Version-aware bone/animation accessors
- Separate v37/v44+ code paths (no casting contamination)
- Basic VVD/VTX/ANI file loading framework
- studiohdr_v44.h structure definitions

However, **critical subsystems remain 40-60% complete**, causing v44+ models to fail rendering and physics:

| Subsystem | Status | Critical Impact |
|-----------|--------|-----------------|
| Hardware Rendering | ~40% | Models invisible/black |
| Model Loading | ~50% | Partial data only |
| Bone Setup/Animation | ~60% | Missing compressed formats |
| Client Animation | ~50% | No bone caching/layers |
| Physics Integration | ~30% | No ragdoll/collision |
| Flex System | ~40% | No stereo/delayed weights |

---

## 1. CRITICAL MODEL LOADING GAPS

### **MISSING: MDL Cache System**
- 2007 Engine: `g_pMDLCache` with `MDLHandle_t` reference counting
- LeakNet: Direct file loading with no unified cache
- **Impact:** Memory fragmentation, cache misses, potential double-loading

### **MISSING: PHY File Loading**
- 2007: Loads `.phy` files, initializes physics shapes, expands bounds
- LeakNet: No PHY loading code exists
- **Impact:** Physics system completely non-functional for v44+ models

### **MISSING: Hardware Vertex Buffers**
- 2007: Creates GPU VBOs/IBOs from VTX data via `BuildHardwareBuffers()`
- LeakNet: Only loads raw VTX/VVD buffers, no GPU upload
- **Impact:** No hardware acceleration, CPU skinning only (if at all)

### **PARTIAL: Material Reference Counting**
- 2007: Increments material refs during load/unload
- LeakNet: Materials loaded but no tracking
- **Impact:** Materials can be purged while models reference them

### **Location:** `engine/modelloader_v44.cpp` (incomplete implementation)

---

## 2. CRITICAL STUDIORENDER SYSTEM GAPS

### **MISSING: Hardware Morphing System**
2007 Engine functions NOT in LeakNet:
```cpp
void DetermineHWMorphing(...)
int CountDeltaFlexedStripGroups(...)
void R_StudioBuildMorph(...)
void ComputeHWMorphDecalBoneRemap(...)
```
- **Impact:** Flex animations cannot use hardware path

### **MISSING: Queue-Based Rendering**
- 2007: `StudioRenderContext_t` for deferred/batched rendering
- LeakNet: Immediate execution, no optimization
- **Impact:** Performance regression

### **MISSING: Eyelid FACS Animation**
```cpp
void R_StudioEyelidFACS(...)          // NOT in LeakNet
void ComputeEyelidStateFACS(...)      // NOT in LeakNet
```

### **MISSING: SSE Lighting**
- 2007: SIMD `R_ComputeLightAtPoints3()` with FourVectors
- LeakNet: Single-threaded scalar lighting
- **Impact:** Significantly slower lighting computation

### **PARTIAL: Flex Rendering**
- Basic vertex flexing implemented
- Missing: Stream offset optimization, stereo flex, delayed weights
- **Location:** `studiorender/r_studioflex.cpp`

### **LOCATIONS:**
- `studiorender/r_studiodraw.cpp` (59 FIXMEs noted)
- `studiorender/r_studio.cpp` (rendering setup)

---

## 3. BONE SETUP & ANIMATION GAPS

### **MISSING: CBoneCache Class**
- 2007: Thread-safe `CBoneCache` with allocation pools (`g_QuaternionPool`, `g_VectorPool`, `g_MatrixPool`)
- LeakNet: References exist (`CStudioBoneCache`) but incomplete
- **Impact:** No bone caching -> severe performance regression

### **MISSING: Compressed Animation Support**
Defined in LeakNet but NOT used in `CalcBoneQuaternion()`:
```cpp
#define STUDIO_ANIM_RAWPOS      0x01    // Vector48 compressed position
#define STUDIO_ANIM_RAWROT      0x02    // Quaternion48 (48-bit quat)
#define STUDIO_ANIM_RAWROT2     0x20    // Quaternion64 (64-bit quat, higher precision)
```

Current implementation only handles uncompressed animations.
- **Impact:** Animations appear wrong or don't play

### **MISSING: World-Space Blending**
```cpp
void WorldSpaceSlerp(...)     // NOT in LeakNet
void ScaleBones(...)          // NOT in LeakNet
```
For animations with `STUDIO_WORLD` flag.

### **PARTIAL: Procedural Bones**
- DoAxisInterpBone
- DoQuatInterpBone
- DoAimAtBone
- DoJiggleBone (stub only - no physics)

### **PARTIAL: IK System**
- Basic `Studio_SolveIK()` variants
- Missing: `Studio_AlignIKMatrix()`, full `CIKContext` class
- No inverse kinematics chain solving

### **LOCATION:** `game_shared/bone_setup.cpp` (lines 31-115 show gaps)

---

## 4. CLIENT ANIMATION SYSTEM GAPS

### **CRITICAL MISSING: Bone Accessor System**
- 2007: `m_BoneAccessor` with `GetBoneForWrite()`, `GetReadableBones()`, `GetWritableBones()`
- LeakNet: Direct bone array access with no protection
- **Impact:** No synchronization, potential data races

### **MISSING: Animation Layer Interpolation**
- 2007: `CInterpolatedVar<C_AnimationLayer>` with history tracking
- LeakNet: No interpolation
- **Impact:** Jerky overlay animations

### **MISSING: Sequence Transitions**
- 2007: `CSequenceTransitioner` for crossfade blending
- LeakNet: Hard cuts between sequences
- **Impact:** Animation pops/discontinuities

### **MISSING: Jiggle Bone Physics**
- 2007: `m_pJiggleBones` with IVP physics simulation
- LeakNet: Completely absent
- **Impact:** Hair/clothing static, no dynamic movement

### **CRITICAL MISSING: Hitbox Transforms**
```cpp
bool HitboxToWorldTransforms(matrix3x4_t *pHitboxToWorld[MAXSTUDIOBONES]);
bool ComputeHitboxSurroundingBox(...);
```
- **Impact:** Bullet tracing completely broken for v44+ models

### **MISSING: Threaded Bone Setup**
- 2007: `ThreadedBoneSetup()` for multi-core optimization
- LeakNet: Single-threaded only
- **Impact:** Performance bottleneck on multi-core systems

### **LOCATION:** `cl_dll/c_baseanimating.cpp`

---

## 5. PHYSICS INTEGRATION GAPS

### **CRITICAL MISSING: Ragdoll Systems**
Missing structures/functions:
```cpp
struct ragdollanimatedfriction_t  // NOT in LeakNet
void RagdollSetupAnimatedFriction(...)
class CRagdollCollisionRules      // NOT in LeakNet
void RagdollSetupCollisions(...)
void RagdollSolveSeparation(...)
```

### **MISSING: Ragdoll LRU/Culling System**
- 2007: ~300 lines of ragdoll fade/retire code (lines 700-1006)
- LeakNet: No ragdoll retirement
- **Impact:** Ragdolls never culled, memory leak

### **IDENTIFIED BUG: CreatePhysicsFollower Return Value**
File: `dlls/physics_bone_follower.cpp` line 70:
```cpp
return false;  // ALWAYS RETURNS FALSE
```
Should be `return true;` on success.

### **LOCATION:**
- `dlls/physics_bone_follower.cpp` (bug)
- `game_shared/ragdoll_shared.cpp` (missing systems)

---

## 6. FLEX/MORPH SYSTEM GAPS

### **MISSING: Stereo Flex Controllers**
```cpp
struct mstudioflexcontrollerui_t {
    bool stereo;
    FlexControllerRemapType_t remaptype;
    // FLEXCONTROLLER_REMAP_2WAY, FLEXCONTROLLER_REMAP_NWAY, FLEXCONTROLLER_REMAP_EYELID
};
```
- **Impact:** No split left/right flex controls

### **MISSING: Delayed Flex Weights**
- 2007: `m_pFlexDelayedWeights[]` for smooth transitions
- LeakNet: Single-frame only
- **Impact:** Jerky flex animations

### **MISSING: Flexpair Blending**
- 2007: Processes speed/side bytes with delayed weight blending
- LeakNet: `flexpair` field completely ignored
- **Impact:** Facial expressions incomplete

### **MISSING: Wrinkle Vertex Animation**
```cpp
struct mstudiovertanim_wrinkle_t {
    short wrinkledelta;  // NOT in LeakNet
};
```
- **Impact:** No wrinkle deformation on facial expressions

### **LOCATION:** `studiorender/r_studioflex.cpp` (incomplete)

---

## 7. VIRTUAL MODEL SYSTEM GAPS

### **MISSING: Thread Safety**
- 2007: `CThreadFastMutex m_Lock` with `AUTO_LOCK_` guards
- LeakNet: No synchronization
- **Impact:** Race conditions with include models

### **MISSING: Node Name Matching**
- 2007: Uses `pszLocalNodeName()` for proper name-based matching
- LeakNet: Simplified index matching (broken for real models)
- **Impact:** Include model bone remapping fails

### **BUG: Attachment Field Mismatch**
- 2007: `pAttach->localbone` field
- LeakNet: Uses `pAttach->bone` field
- **Suggests:** Struct layout inconsistency

### **PARTIAL: GetVirtualModel()**
- Status: Stubbed with comment "Defined after studiohdr_v44_t"
- **Location:** `public/cstudiohdr.h`

---

## 8. STRUCTURE DEFINITION ISSUES

### **mstudiobone_t Misalignment**
- v37: 192 bytes with wrong field order (`value[6]`, `scale[6]`)
- v48: 216 bytes with proper fields (`pos`, `quat`, `rot`, `posscale`, `rotscale`)
- **Offset Error:** v37 `quat` at 0xA4 vs correct 0x2C

### **mstudioanimdesc_t Size Difference**
- v37: 76 bytes
- v48: 100 bytes (24-byte difference)
- **Missing in v37:** `baseptr`, `animblock`, `animblockikruleindex`

### **mstudioseqdesc_t Field Names**
- v37: Has `entrynode/exitnode` (incorrect names)
- v48: Has `localentrynode/localexitnode` (correct)

### **mstudioanim_t Completely Different**
- v37: 28 bytes with inline animation data
- v48: 4 bytes with offset link
- **Architecture mismatch requires** runtime checking

### **mstudioeyeball_t Structure**
- v37: 140 bytes with iris/glint materials
- v48: 188 bytes with FACS animation support
- **48-byte difference** in layout

---

## 9. EXISTING IMPLEMENTATIONS TO BUILD ON

### **Already Working:**
1. Version-aware bone accessors in `cstudiohdr.h`
2. Separate `studiohdr_v44.h` with v44+ structures
3. `studio_v37_compat.h` dispatch functions
4. VVD/VTX/ANI file loading framework
5. Basic `modelloader_v44.cpp` skeleton
6. Compressed animation definitions (just not used)
7. Linear bone support (`pLinearBones()`)

### **Partially Working:**
1. Flex vertex processing
2. Procedural bones (except jiggle)
3. IK chains (basic only)
4. Animation groups (v37 only)
5. Bone caching framework (incomplete)

---

## 10. PRIORITY IMPLEMENTATION ROADMAP

### **Phase 1: CRITICAL - Rendering (Models Visible)**
1. **Hardware Vertex Buffer Creation** - VTX data -> GPU buffers
2. **Mesh Group Extraction** - Materials, LOD, strips
3. **Model Flags Computation** - Render flags from VTX
4. **PHY File Loading** - Physics collision setup

### **Phase 2: Animation Functional**
5. **Compressed Animation Support** - RAWROT/RAWPOS decompression
6. **CBoneCache** - Thread-safe bone caching
7. **Animation Layer Interpolation** - History tracking
8. **Sequence Transitions** - Crossfade blending
9. **Full IK System** - CIKContext implementation

### **Phase 3: Physics Working**
10. **Ragdoll Collision Rules** - Body part separation
11. **Ragdoll Separation Solver** - Interpenetration handling
12. **Physics Bone Follower Bug Fix** - Return value correction
13. **Ragdoll LRU System** - Culling/fading

### **Phase 4: Advanced Features**
14. **Hardware Morphing** - GPU flex vertex processing
15. **Stereo Flex Controllers** - Split left/right
16. **Delayed Flex Weights** - Smooth transitions
17. **Jiggle Bone Physics** - IVP integration
18. **Threaded Bone Setup** - Multi-core support
19. **MDL Cache System** - Unified caching layer

---

## 11. FILES REQUIRING MODIFICATION

### **HIGH PRIORITY:**
- `engine/modelloader_v44.cpp` - Complete hardware setup, PHY loading
- `studiorender/r_studiodraw.cpp` - Hardware vertex buffers, LOD
- `studiorender/r_studioflex.cpp` - Stereo/delayed flex
- `game_shared/bone_setup.cpp` - Compressed animation decompression
- `cl_dll/c_baseanimating.cpp` - Bone accessor, jiggle bones, hitboxes

### **MEDIUM PRIORITY:**
- `game_shared/ragdoll_shared.cpp` - Collision rules, separation solver
- `game_shared/studio_virtualmodel.cpp` - Thread safety, node matching
- `dlls/physics_bone_follower.cpp` - Return value bug fix (line 70)

### **STRUCTURE FILES:**
- `public/studio.h` - Fix mstudiobone_t, mstudioanimdesc_t alignment
- `public/studiohdr_v44.h` - Verify v48 structures

---

## CONCLUSION

LeakNet's v44+ implementation is **approximately 50% complete** with solid architectural foundations but missing critical subsystems. The most blocking issues preventing v44+ model visibility and functionality are:

1. **No GPU vertex buffers** - Models cannot render
2. **No PHY/collision loading** - Physics disabled
3. **No compressed animation decompression** - Animation broken
4. **No bone caching** - Performance severe regression
5. **No hitbox transforms** - Hit detection broken

The foundation is sound (separate code paths, version checking, structure definitions), but substantial implementation work remains in rendering, animation, physics, and flex systems.

---

## Quick Reference: Critical Missing Functions

### From 2007 Engine studiorender:
```cpp
// Hardware morphing
DetermineHWMorphing()
CountDeltaFlexedStripGroups()
R_StudioBuildMorph()
ComputeHWMorphDecalBoneRemap()

// FACS animation
R_StudioEyelidFACS()
ComputeEyelidStateFACS()

// SSE lighting
R_ComputeLightAtPoints3() // FourVectors version
```

### From 2007 Engine bone_setup:
```cpp
// Compressed animation
ExtractAnimValue() // For RAWPOS/RAWROT
Quaternion48/64 decompression

// World-space blending
WorldSpaceSlerp()
ScaleBones()

// Full IK
Studio_AlignIKMatrix()
CIKContext class
```

### From 2007 Engine client:
```cpp
// Bone accessor
GetBoneForWrite()
GetReadableBones()
GetWritableBones()

// Hitboxes
HitboxToWorldTransforms()
ComputeHitboxSurroundingBox()

// Threading
ThreadedBoneSetup()
```

### From 2007 Engine physics:
```cpp
// Ragdoll
RagdollSetupAnimatedFriction()
RagdollSetupCollisions()
RagdollSolveSeparation()
CRagdollCollisionRules
```
