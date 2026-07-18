#include "BaseVSShader.h"

#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( WriteStencil, WriteStencil_DX8 )

BEGIN_VS_SHADER( WriteStencil_DX8, "Write only to the active stencil buffer" )
	BEGIN_SHADER_PARAMS
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
			pShaderShadow->EnableColorWrites( false );
			pShaderShadow->EnableAlphaWrites( false );
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->SetVertexShader( "writez" );
			pShaderShadow->SetPixelShader( "white" );
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 0, 0, 0, 0 );
		}
		DYNAMIC_STATE
		{
			pShaderAPI->FogMode( MATERIAL_FOG_NONE );
		}
		Draw();
	}
END_SHADER
