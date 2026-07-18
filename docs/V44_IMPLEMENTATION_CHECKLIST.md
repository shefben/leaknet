# V44+ Separation Implementation Checklist

## Phase 1: Header Separation

### public/studio_v37.h (NEW FILE)
- [ ] Copy original v37 structures from LeakNet
- [ ] `studiohdr_t` (v37 layout)
- [ ] `mstudiobone_t` (with value[6]/scale[6])
- [ ] `mstudioboneweight_v37_t` (32 bytes, short bone[4])
- [ ] `mstudiovertex_v37_t` (64 bytes)
- [ ] `mstudiomesh_t` (embedded vertices)
- [ ] `mstudiomodel_t` (no modelvertexdata)
- [ ] `mstudioseqdesc_t` (v37 layout)
- [ ] `mstudioanimdesc_t` (v37 layout)

### public/studio_v44.h (ENHANCE EXISTING studiohdr_v44.h)
- [ ] Copy from 2007 engine `public/studio.h`
- [ ] `studiohdr_v44_t` (v44+ layout) - already exists
- [ ] `mstudiobone_v48_t` (pos/quat/rot/posscale/rotscale) - already exists
- [ ] `mstudioboneweight_t` (16 bytes, char bone[3]) - already exists
- [ ] `mstudiovertex_t` (48 bytes) - already exists
- [ ] `mstudiomesh_v44_t` (external VVD) - already exists
- [ ] `mstudiomodel_v44_t` (with modelvertexdata) - already exists
- [ ] `mstudioanim_t` (v48 with compression flags)
- [ ] `mstudiolinearbone_t` (linear bone optimization)
- [ ] `CStudioHdr` wrapper class (CRITICAL - from 2007)
- [ ] `mstudio_meshvertexdata_t` accessor class
- [ ] `mstudio_modelvertexdata_t` accessor class
- [ ] Quaternion48/64, Vector48 compressed types

## Phase 2: StudioRender Separation

### studiorender/r_studiodraw_v37.cpp (NEW FILE)
- [ ] Copy original LeakNet `R_StudioDrawPoints()`
- [ ] Copy original `ComputeSkinMatrix()` (v37 version)
- [ ] Copy original `R_StudioRenderModel()` internals
- [ ] Use only v37 structures (`mstudiovertex_v37_t`, etc.)
- [ ] No version checks - pure v37 code

### studiorender/r_studiodraw_v44.cpp (NEW FILE - COPY FROM 2007)
Copy from `SourceEngine2007/src_main/studiorender/r_studiodraw.cpp`:
- [ ] `R_StudioDrawPoints()` (v44+ version)
- [ ] `ComputeSkinMatrix()` (v44+ version with char bone[3])
- [ ] `R_StudioRenderModel()` (v44+ internals)
- [ ] Hardware flex support
- [ ] Color mesh support
- [ ] Use only v44+ structures

### studiorender/r_studioflex_v37.cpp (NEW FILE)
- [ ] Copy original LeakNet flex code
- [ ] `R_StudioProcessFlexedMesh()` (v37 version)
- [ ] Software-only flex morphing
- [ ] Use `mstudiovertex_v37_t` for vertex access

### studiorender/r_studioflex_v44.cpp (NEW FILE - COPY FROM 2007)
Copy from `SourceEngine2007/src_main/studiorender/r_studioflex.cpp`:
- [ ] `R_StudioProcessFlexedMesh()` (v44+ version)
- [ ] Hardware morph accumulator support
- [ ] Use `mstudio_meshvertexdata_t` accessors

### studiorender/r_studiodecal_v37.cpp (NEW FILE)
- [ ] Copy original LeakNet decal code
- [ ] `R_StudioDecalProject()` (v37 version)
- [ ] Use embedded vertex data

### studiorender/r_studiodecal_v44.cpp (NEW FILE - COPY FROM 2007)
Copy from `SourceEngine2007/src_main/studiorender/r_studiodecal.cpp`:
- [ ] `R_StudioDecalProject()` (v44+ version)
- [ ] Use VVD vertex data

### studiorender/cstudiorender.cpp (MODIFY)
- [ ] Remove all inline version checks
- [ ] Add top-level dispatch functions:
  ```cpp
  void DrawModel_V37(...);
  void DrawModel_V44(...);
  void DrawModelResults_V37(...);
  void DrawModelResults_V44(...);
  ```
- [ ] Main `DrawModel()` dispatches based on version

### studiorender/studio_v37_compat.h (DELETE or DEPRECATE)
- [ ] Mark as deprecated
- [ ] Remove invalid `reinterpret_cast` functions
- [ ] Replace with simple dispatch macros

## Phase 3: Animation System Separation

### game_shared/bone_setup_v37.cpp (NEW FILE)
Extract from current bone_setup.cpp:
- [ ] `CalcRotations_v37()`
- [ ] `CalcBoneQuaternion()` (v37 with value[]/scale[])
- [ ] `CalcBonePosition()` (v37 with value[]/scale[])
- [ ] `ExtractAnimValue()` (v37 format)
- [ ] `CalcAnimation()` (v37 animation access)
- [ ] `CalcAutoplaySequences_v37()`
- [ ] `CalcBoneAdj_v37()`

### game_shared/bone_setup_v44.cpp (NEW FILE - COPY FROM 2007)
Copy from `SourceEngine2007/src_main/public/bone_setup.cpp`:
- [ ] `CalcRotations()` (v48 with compressed animations)
- [ ] `CalcBoneQuaternion_v48()` (Quaternion48/64)
- [ ] `CalcBonePosition_v48()` (Vector48)
- [ ] `ExtractAnimValue()` (v48 format)
- [ ] `CalcAnimation()` (v48 with animation blocks)
- [ ] `CalcPose()` / `AccumulatePose()` (2007 interface)
- [ ] `BuildBoneChain()` (with CStudioHdr)
- [ ] IK constraint solving

### game_shared/studiohdr_wrapper.cpp (NEW FILE - FROM 2007)
- [ ] `CStudioHdr` class implementation
- [ ] Virtual model composition
- [ ] Bone remapping functions
- [ ] Animation block cache

## Phase 4: Model Loading

### engine/modelloader_v44.cpp (ENHANCE)
- [ ] Complete `LoadVvdFile_v44()`:
  - [ ] Parse `vertexFileHeader_t`
  - [ ] Handle fixup tables for LOD sorting
  - [ ] Set up `mstudio_modelvertexdata_t` pointers
- [ ] Complete `LoadVtxFile_v44()`:
  - [ ] Parse `OptimizedModel::FileHeader_t`
  - [ ] Process body parts, models, LODs
  - [ ] Build strip groups and indices
- [ ] Implement `LoadAnimBlocksFile_v44()`:
  - [ ] Parse .ani file header
  - [ ] Demand-load animation blocks
  - [ ] Cache management
- [ ] Wire vertex data to studiorender

### engine/l_studio.cpp (MODIFY)
- [ ] Add dispatch for v44+ models to `CModelLoader_v44`
- [ ] Ensure `Mod_LoadStudioModel()` routes correctly
- [ ] Keep v37 path unchanged

## Phase 5: Client-Side Rendering Integration

### cl_dll/c_baseanimating.cpp
- [ ] Update `SetupBones()` to dispatch to correct bone_setup
- [ ] Use `CStudioHdr` for v44+ models
- [ ] Keep v37 path with direct `studiohdr_t` access

### cl_dll/c_baseplayer.cpp
- [ ] Update player model rendering dispatch
- [ ] Handle viewmodel v37/v44+ rendering

### cl_dll/c_baseviewmodel.cpp
- [ ] Dispatch viewmodel rendering by version

## Phase 6: Server-Side Animation

### dlls/animating.cpp
- [ ] Server-side bone setup dispatch
- [ ] v37/v44+ animation playback

### dlls/BaseAnimatingOverlay.cpp
- [ ] Animation overlay system dispatch
- [ ] Layer blending for both versions

## Testing Checklist

### V37 Models (HL2 Beta Assets)
- [ ] Character models render correctly
- [ ] Animations play correctly
- [ ] Bone transforms are accurate
- [ ] Flexes/morphs work
- [ ] Decals project correctly
- [ ] Hitboxes align properly

### V44+ Models (HL2 Retail Assets)
- [ ] Character models render correctly (no longer invisible)
- [ ] Animations play correctly
- [ ] Bone transforms match 2007 engine
- [ ] Flexes/morphs work (hardware path)
- [ ] Decals project correctly
- [ ] LODs work properly

### Mixed Sessions
- [ ] Load v37 model, then v44+ model
- [ ] Load v44+ model, then v37 model
- [ ] Render both simultaneously
- [ ] No memory corruption
- [ ] No performance degradation

## Files to Delete/Deprecate

- [ ] `studiorender/studio_v37_compat.h` - Remove after new dispatch layer complete
- [ ] Inline version checks in all files listed above

## Reference Files (2007 Engine)

```
F:\back-ups\betahl2_codebases\SourceEngine2007-master\src_main\
├── studiorender\
│   ├── r_studiodraw.cpp     # 2432 lines - main reference
│   ├── r_studioflex.cpp     # Flex/morph reference
│   ├── r_studiodecal.cpp    # Decal reference
│   └── r_studio.cpp         # Setup reference
├── public\
│   ├── studio.h             # v48 structures
│   ├── bone_setup.h         # CStudioHdr, CalcPose
│   ├── bone_setup.cpp       # Animation implementation
│   └── studio_virtualmodel.cpp  # Virtual models
└── engine\
    └── modelloader.cpp      # Model loading reference
```

## Priority Order

1. **HIGHEST**: Fix `studio_v37_compat.h` invalid casts (causing invisible models)
2. **HIGH**: Create separate `r_studiodraw_v37.cpp` / `r_studiodraw_v44.cpp`
3. **HIGH**: Complete `CModelLoader_v44` VVD/VTX loading
4. **MEDIUM**: Separate animation system
5. **MEDIUM**: Complete 2007 engine code copying
6. **LOW**: Optimize and clean up
