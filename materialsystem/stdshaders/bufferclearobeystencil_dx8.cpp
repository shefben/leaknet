#include "BaseVSShader.h"

#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( BufferClearObeyStencil, BufferClearObeyStencil_DX8 )

BEGIN_VS_SHADER( BufferClearObeyStencil_DX8,
	"Clear selected buffers while preserving the current stencil test" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( CLEARCOLOR, SHADER_PARAM_TYPE_INTEGER, "1", "clear color and alpha" )
		SHADER_PARAM( CLEARDEPTH, SHADER_PARAM_TYPE_INTEGER, "1", "clear depth" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_FALLBACK
	{
		return g_pHardwareConfig->GetDXSupportLevel() < 80 ? "Wireframe" : 0;
	}

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_ALWAYS );
			pShaderShadow->EnableDepthWrites( params[CLEARDEPTH]->GetIntValue() != 0 );
			const bool bClearColor = params[CLEARCOLOR]->GetIntValue() != 0;
			pShaderShadow->EnableColorWrites( bClearColor );
			pShaderShadow->EnableAlphaWrites( bClearColor );
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION | VERTEX_COLOR,
				1, 0, 0, 0 );
			pShaderShadow->SetVertexShader( "BufferClearObeyStencil_vs11" );
			pShaderShadow->SetPixelShader( "BufferClearObeyStencil_ps11" );
		}
		DYNAMIC_STATE
		{
			pShaderAPI->SetVertexShaderIndex( 0 );
			pShaderAPI->SetPixelShaderIndex( 0 );
		}
		Draw();
	}
END_SHADER
