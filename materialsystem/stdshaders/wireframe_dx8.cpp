#include "BaseVSShader.h"

#include "tier0/memdbgon.h"

// Retail HL2 exposes Wireframe as a material shader.  Keep it on the DX8
// implementation so it can use the shader programs already shipped by this
// engine without replacing any existing shader registration.
DEFINE_FALLBACK_SHADER( Wireframe, Wireframe_DX8 )
DEFINE_FALLBACK_SHADER( Wireframe_DX9, Wireframe_DX8 )

BEGIN_VS_SHADER( Wireframe_DX8, "Help for Wireframe_DX8" )
	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
		SET_FLAGS( MATERIAL_VAR_NOFOG );
	}

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->PolyMode( SHADER_POLYMODEFACE_FRONT_AND_BACK, SHADER_POLYMODE_LINE );
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, 0, 0, 0 );
			pShaderShadow->SetVertexShader( "UnlitGeneric" );
			pShaderShadow->SetPixelShader( "UnlitGeneric_NoTexture" );
		}
		DYNAMIC_STATE
		{
			SetModulationVertexShaderDynamicState();
			DefaultFog();
		}
		Draw();
	}
END_SHADER
