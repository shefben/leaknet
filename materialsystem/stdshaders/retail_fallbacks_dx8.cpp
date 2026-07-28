#include "BaseVSShader.h"

#include "tier0/memdbgon.h"

// Retail names whose rendering behavior is already provided by an existing
// shader in this branch.  These aliases add compatibility registrations only;
// the existing implementations remain untouched.
DEFINE_FALLBACK_SHADER( Cable_DX8, Cable )
DEFINE_FALLBACK_SHADER( Cloud_DX8, Cloud )
DEFINE_FALLBACK_SHADER( DecalModulate_DX8, DecalModulate )
DEFINE_FALLBACK_SHADER( Eyes_DX8, Eyes )
DEFINE_FALLBACK_SHADER( Modulate_DX8, Modulate )
DEFINE_FALLBACK_SHADER( ParticleSphere_DX8, ParticleSphere )
DEFINE_FALLBACK_SHADER( ShadowBuild_DX8, ShadowBuild )
DEFINE_FALLBACK_SHADER( ShadowModel_DX8, ShadowModel )
DEFINE_FALLBACK_SHADER( UnlitTwoTexture_DX8, UnlitTwoTexture )
DEFINE_FALLBACK_SHADER( WorldTwoTextureBlend_DX8, WorldTwoTextureBlend )
DEFINE_FALLBACK_SHADER( WriteZ_DX8, WriteZ )

// Retail dx-suffixed spellings of shaders this branch registers without the
// suffix.  Retail resolves these through its own per-DX-level implementations;
// here they simply chain to the single beta implementation.
DEFINE_FALLBACK_SHADER( Cable_DX6, Cable )
DEFINE_FALLBACK_SHADER( Eyes_DX6, Eyes )
DEFINE_FALLBACK_SHADER( Teeth_DX6, Teeth )
DEFINE_FALLBACK_SHADER( DecalModulate_DX6, DecalModulate )
DEFINE_FALLBACK_SHADER( WorldTwoTextureBlend_DX6, WorldTwoTextureBlend )
DEFINE_FALLBACK_SHADER( BufferClearObeyStencil_DX6, BufferClearObeyStencil )
DEFINE_FALLBACK_SHADER( ShatteredGlass_DX8, ShatteredGlass )
DEFINE_FALLBACK_SHADER( Predator_DX80, Predator )

// Retail shaders that have no beta equivalent at all.  These are the exact
// fallbacks retail itself installs on dx8-class hardware
// (hl2_src/materialsystem/stdshaders/dx8fallbacks.cpp), so a retail .vmt that
// names them resolves to something drawable instead of the error material.
DEFINE_FALLBACK_SHADER( DepthWrite, Wireframe )
DEFINE_FALLBACK_SHADER( EyeRefract, Eyes_DX8 )
DEFINE_FALLBACK_SHADER( EyeGlint, Wireframe )
DEFINE_FALLBACK_SHADER( AfterShock, Wireframe )
DEFINE_FALLBACK_SHADER( VolumeClouds, UnlitGeneric_DX8 )
DEFINE_FALLBACK_SHADER( DebugTextureView, UnlitGeneric_DX8 )
