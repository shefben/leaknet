#include "BaseVSShader.h"

#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( MonitorScreen, MonitorScreen_DX8 )

BEGIN_VS_SHADER( MonitorScreen_DX8,
	"Contrast, saturation, and tint processing for monitor materials" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( CONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast: 0 is normal, 1 is color squared" )
		SHADER_PARAM( SATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation: 0 is greyscale, 1 is normal" )
		SHADER_PARAM( TINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "monitor tint" )
		SHADER_PARAM( TEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "", "optional second texture" )
		SHADER_PARAM( FRAME2, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $texture2" )
		SHADER_PARAM( TEXTURE2TRANSFORM, SHADER_PARAM_TYPE_MATRIX,
			"center .5 .5 scale 1 1 rotate 0 translate 0 0", "$texture2 texcoord transform" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		if ( !params[CONTRAST]->IsDefined() )
			params[CONTRAST]->SetFloatValue( 0.0f );
		if ( !params[SATURATION]->IsDefined() )
			params[SATURATION]->SetFloatValue( 1.0f );
		if ( !params[TINT]->IsDefined() )
			params[TINT]->SetVecValue( 1.0f, 1.0f, 1.0f );
	}

	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 80 ||
			 ( params && !params[BASETEXTURE]->IsDefined() ) )
		{
			return IS_FLAG_SET( MATERIAL_VAR_MODEL ) ?
				"VertexLitGeneric_DX6" : "LightmappedGeneric_DX6";
		}
		return 0;
	}

	SHADER_INIT
	{
		if ( params[BASETEXTURE]->IsDefined() )
			LoadTexture( BASETEXTURE );
		if ( params[TEXTURE2]->IsDefined() )
			LoadTexture( TEXTURE2 );
	}

	SHADER_DRAW
	{
		const bool bHasTexture2 = params[TEXTURE2]->GetType() == MATERIAL_VAR_TYPE_TEXTURE;
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, bHasTexture2 );

			bool bTranslucent = IsAlphaModulating() ||
				TextureIsTranslucent( BASETEXTURE, true );
			if ( bHasTexture2 )
				bTranslucent = bTranslucent || TextureIsTranslucent( TEXTURE2, true );

			if ( bTranslucent )
			{
				EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA,
					IS_FLAG_SET( MATERIAL_VAR_ADDITIVE ) ? SHADER_BLEND_ONE : SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			}
			else if ( IS_FLAG_SET( MATERIAL_VAR_ADDITIVE ) )
			{
				EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
			}
			else
			{
				DisableAlphaBlending();
			}

			int nVertexFormat = VERTEX_POSITION | VERTEX_NORMAL;
			const bool bDoSkin = IS_FLAG_SET( MATERIAL_VAR_MODEL );
			if ( bDoSkin )
				nVertexFormat |= VERTEX_BONE_INDEX;
			if ( IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR ) )
				nVertexFormat |= VERTEX_COLOR;

			pShaderShadow->VertexShaderVertexFormat( nVertexFormat, 1, 0, bDoSkin ? 3 : 0, 0 );
			pShaderShadow->SetVertexShader( "UnlitTwoTexture" );
			pShaderShadow->SetPixelShader( "MonitorScreen", bHasTexture2 ? 1 : 0 );
		}
		DYNAMIC_STATE
		{
			BindTexture( SHADER_TEXTURE_STAGE0, BASETEXTURE, FRAME );
			if ( bHasTexture2 )
			{
				BindTexture( SHADER_TEXTURE_STAGE1, TEXTURE2, FRAME2 );
				SetVertexShaderTextureTransform( 92, TEXTURE2TRANSFORM );
			}
			SetVertexShaderTextureTransform( 90, BASETEXTURETRANSFORM );
			SetPixelShaderConstant( 1, CONTRAST );
			SetPixelShaderConstant( 2, SATURATION );
			SetPixelShaderConstant( 3, TINT );
			SetModulationVertexShaderDynamicState();
			FogToFogColor();
		}
		Draw();
	}
END_SHADER
