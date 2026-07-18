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
