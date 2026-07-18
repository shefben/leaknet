# V37/V44+ Model System Separation - Todo List

## Current Status: Phase 3 & 4 Complete - Animation system and model loading ready

---

## Phase 1: Analysis & Preparation

- [x] **Analyze current codebase** - Examined mixed v37/v44+ code, identified invalid casts in studio_v37_compat.h
- [x] **Analyze 2007 engine** - Reviewed 2007 Source Engine studiorender architecture and key structures
- [x] **Create architecture docs** - Created V44_SEPARATION_ARCHITECTURE.md and V44_IMPLEMENTATION_CHECKLIST.md

---

## Phase 2: StudioRender Separation - COMPLETE

- [x] **Backup mixed files** - Saved old mixed implementations
  - r_studiodraw_mixed_backup.cpp
  - r_studioflex_mixed_backup.cpp
  - r_studiodecal_mixed_backup.cpp
  - r_studio_mixed_backup.cpp

- [x] **Restore original v37 code** - Copied from clean LeakNet as main rendering files
  - r_studiodraw.cpp (original v37, now primary)
  - r_studioflex.cpp (original v37)
  - r_studiodecal.cpp (original v37)

- [x] **Copy 2007 engine v44+ files** - For future v44+ integration
  - r_studiodraw_v44.cpp (pure v44+)
  - r_studioflex_v44.cpp
  - r_studiodecal_v44.cpp
  - r_studio_v44.cpp

- [x] **Fix header compatibility** - Updated v37 code to use _v37 accessors
  - Added v37 overloads to cstudiorender.h: R_StudioTransform, R_StudioRotate
  - Added v37 overloads to r_studiodraw.cpp: ComputeSkinMatrix_v37, ComputeSkinMatrixSSE_v37
  - Added v37 overloads to r_studiodecal.cpp: IsFrontFacing, TransformToDecalSpace
  - Updated r_studiodraw.cpp: mstudiovertex_v37_t, Vertex_v37(), ComputeSkinMatrix_v37(), etc.
  - Updated r_studiodecal.cpp: mstudioboneweight_v37_t, BoneWeights_v37(), Position_v37(), Normal_v37()
  - Updated r_studioflex.cpp: mstudiovertex_v37_t, Vertex_v37()

- [x] **Build verification** - studiorender.dll builds successfully

- [x] **Keep r_studio.cpp with dispatch** - Using mixed version that handles both v37 and v44+ mesh building

---

## Phase 3: Animation System Separation - COMPLETE

- [x] **Analyze bone_setup.cpp** - Current code already uses version dispatch pattern
  - CalcRotations_v37() and CalcRotations_v48_internal() already exist
  - CalcBoneQuaternion() and CalcBoneQuaternion_v48() already exist
  - CalcBonePosition() and CalcBonePosition_v48() already exist
  - No need to split into separate files - dispatch pattern is correct

- [x] **Implement CStudioHdr wrapper** - Enhanced public/cstudiohdr.h
  - Added virtual model support (m_pVModel, m_pStudioHdrCache)
  - Added pSeqStudioHdr(), pAnimStudioHdr() for include models
  - Added RemapAnimBone(), RemapSeqBone() for bone remapping
  - Added GetNumSeq(), GetNumAnim() for total counts
  - Added pSeqdesc(), pAnimdesc() with virtual model handling
  - Added iRelativeAnim(), iRelativeSeq() for index remapping
  - Added InitVirtualModel(), GroupStudioHdr() helpers

---

## Phase 4: Model Loading - COMPLETE

- [x] **Complete VVD loading** - engine/modelloader_v44.cpp
  - LoadVvdFile_v44() fully implemented
  - Validates VVD header and version
  - Sets up vertex and tangent data pointers
  - Propagates vertex data to mstudiomodel structures

- [x] **Complete VTX loading** - engine/modelloader_v44.cpp
  - LoadVtxFile_v44() fully implemented
  - Tries .dx90.vtx, .dx80.vtx, .sw.vtx extensions
  - Validates VTX header, version, and checksum

- [x] **Implement animation blocks** - engine/modelloader_v44.cpp
  - LoadAnimBlocksFile_v44() fully implemented
  - Loads external .ani files for v44+ models
  - GetAnimBlock_v44() function for animation block access
  - Sets up animblockModel pointer in studiohdr

---

## Phase 5: Build System & Testing

- [x] **Update CMakeLists.txt** - Already uses existing source files

- [x] **Full engine build** - All core components build successfully:
  - studiorender.dll ✓
  - engine.dll ✓
  - materialsystem.dll ✓
  - vphysics.dll ✓
  - client_hl2.dll ✓
  - server_bmod.dll ✓

- [x] **Deploy to game directory** - Copied DLLs to C:\anon-hl2\bin and bmod\bin

- [ ] **Test v37 models** - Verify HL2 Beta assets render correctly

- [ ] **Test v44+ models** - Verify HL2 retail assets render correctly

- [ ] **Test mixed sessions** - Load both v37 and v44+ models simultaneously

---

## Summary of Changes Made

### cstudiorender.h
- Added v37 overloads for R_StudioTransform, R_StudioRotate (accepting mstudioboneweight_v37_t*)
- Added v37 overload declarations for IsFrontFacing, TransformToDecalSpace

### r_studiodraw.cpp
- Now uses restored v37 code from clean LeakNet
- Added ComputeSkinMatrix_v37 and ComputeSkinMatrixSSE_v37 functions
- Changed all mstudiovertex_t to mstudiovertex_v37_t
- Changed all Vertex() to Vertex_v37()
- Changed all ComputeSkinMatrix(vert.m_BoneWeights...) to ComputeSkinMatrix_v37(...)

### r_studiodecal.cpp
- Now uses restored v37 code from clean LeakNet
- Added v37 overloads for IsFrontFacing and TransformToDecalSpace
- Changed mstudiovertex_t to mstudiovertex_v37_t, Vertex() to Vertex_v37()
- Changed BoneWeights() to BoneWeights_v37(), Position() to Position_v37(), Normal() to Normal_v37()

### r_studioflex.cpp
- Now uses restored v37 code from clean LeakNet
- Changed mstudiovertex_t to mstudiovertex_v37_t, Vertex() to Vertex_v37()
- Fixed SetupComputation() call to include NULL studiohdr parameter

### r_studio.cpp
- Kept mixed version that handles both v37 and v44+ mesh building
- Has proper function signatures matching cstudiorender.h

### public/cstudiohdr.h (Phase 3)
- Enhanced with virtual model support for v44+ include models
- Added m_pVModel and m_pStudioHdrCache members
- Added sequence/animation descriptor access with virtual model handling
- Added bone remapping functions for include model support
- Added relative animation/sequence index mapping

### engine/modelloader_v44.cpp (Phase 4)
- Complete v44+ model loading system
- LoadVvdFile_v44(): Full VVD vertex data loading
- LoadVtxFile_v44(): Full VTX mesh data loading
- LoadAnimBlocksFile_v44(): External .ani animation block loading
- GetAnimBlock_v44(): Animation block data access function
- SetupVertexData_v44(): Vertex pointer propagation to models

### engine/modelloader_v44.h (Phase 4)
- model_v44_t structure for v44+ model data
- studiomeshdata_v44_t for mesh data storage
- CModelLoader_v44 class for model management
- Global functions for v44+ model access

### game_shared/bone_setup.cpp (Phase 3)
- Already has version dispatch pattern (no changes needed)
- CalcRotations dispatches to _v37 or _v48 based on model version
- CalcBoneQuaternion/CalcBonePosition have v37 and v48 variants

---

## Notes

- Clean LeakNet (v37 only): `F:\development\steam\emulator_bot\LeakNet`
- 2007 Engine reference: `F:\back-ups\betahl2_codebases\SourceEngine2007-master\src_main`
- Current rewrite: `F:\development\steam\emulator_bot\LeakNet-rewrite`
- v44+ files (r_studiodraw_v44.cpp, etc.) are ready for future integration
- studio_v37_compat.h is kept for r_studiogettriangles.cpp dispatch functions
