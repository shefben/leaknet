//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Retail "SetZ" shader, ported to the beta shader API.
//
// Source: hl2_src/materialsystem/stdshaders/SetZ.cpp
//
// Adaptations for this branch:
//   - BEGIN_SHADER_FLAGS( ..., SHADER_NOT_EDITABLE ) does not exist in the
//     beta shaderlib (public/shaderlib/cshader.h only provides BEGIN_SHADER),
//     so the plain BEGIN_SHADER form is used.
//   - Everything else (EnableColorWrites / EnableAlphaWrites / DepthFunc /
//     DrawFlags) is present verbatim in public/materialsystem/ishaderapi.h,
//     so no state calls needed changing and no compiled shader assets are
//     required - this shader is pure fixed-function state.
//
//=============================================================================

#include "shaderlib/cshader.h"

#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( SetZ, SetZ_DX6 )

BEGIN_SHADER( SetZ_DX6, "Help for SetZ_DX6" )

	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableColorWrites( false );
			pShaderShadow->EnableAlphaWrites( false );
			pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_ALWAYS );

			pShaderShadow->DrawFlags( SHADER_DRAW_POSITION );
		}
		DYNAMIC_STATE
		{
		}
		Draw();
	}
END_SHADER
