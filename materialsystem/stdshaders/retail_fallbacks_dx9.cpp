#include "BaseVSShader.h"

#include "tier0/memdbgon.h"

// DX9 retail names backed by equivalent shaders already present in this tree.
DEFINE_FALLBACK_SHADER( Cable_DX9, Cable )
DEFINE_FALLBACK_SHADER( DecalModulate_DX9, DecalModulate )
DEFINE_FALLBACK_SHADER( Refract_DX90, Refract )
DEFINE_FALLBACK_SHADER( ShadowBuild_DX9, ShadowBuild )
DEFINE_FALLBACK_SHADER( ShadowModel_DX9, ShadowModel )
DEFINE_FALLBACK_SHADER( Sky_DX9, Sky )
DEFINE_FALLBACK_SHADER( Sprite_DX9, Sprite )
DEFINE_FALLBACK_SHADER( Teeth_DX9, Teeth )
DEFINE_FALLBACK_SHADER( UnlitTwoTexture_DX9, UnlitTwoTexture )
DEFINE_FALLBACK_SHADER( Water_DX90, Water )
DEFINE_FALLBACK_SHADER( WorldVertexTransition_DX9, WorldVertexTransition )
DEFINE_FALLBACK_SHADER( WriteZ_DX9, WriteZ )

// Additional retail dx9 spellings whose behavior is already covered by an
// existing implementation in this tree.  Names are kept disjoint from
// retail_fallbacks_dx8.cpp so no shader name is registered twice.
DEFINE_FALLBACK_SHADER( BufferClearObeyStencil_DX9, BufferClearObeyStencil )
DEFINE_FALLBACK_SHADER( Cloud_DX9, Cloud )
DEFINE_FALLBACK_SHADER( Eyes_DX9, Eyes )
DEFINE_FALLBACK_SHADER( Modulate_DX9, Modulate )
DEFINE_FALLBACK_SHADER( MonitorScreen_DX9, MonitorScreen )
DEFINE_FALLBACK_SHADER( ParticleSphere_DX9, ParticleSphere )
DEFINE_FALLBACK_SHADER( WriteStencil_DX9, WriteStencil )
DEFINE_FALLBACK_SHADER( VortWarp_DX9, VortWarp )
