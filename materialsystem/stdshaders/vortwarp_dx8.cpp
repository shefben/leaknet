#include "BaseVSShader.h"

#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( VortWarp, VortWarp_DX8 )

static float VortWarpLerp( float t, float a, float b )
{
	return a + t * ( b - a );
}

static float VortWarpQuadraticBezier( float t, float a, float b, float c )
{
	return VortWarpLerp( t, VortWarpLerp( t, a, b ), VortWarpLerp( t, b, c ) );
}

static float VortWarpCubicBezier( float t, float a, float b, float c, float d )
{
	return VortWarpQuadraticBezier( t, VortWarpLerp( t, a, b ),
		VortWarpLerp( t, b, c ), VortWarpLerp( t, c, d ) );
}

BEGIN_VS_SHADER( VortWarp_DX8, "Help for VortWarp_DX8" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "self-illumination tint" )
		SHADER_PARAM( ENTITYORIGIN, SHADER_PARAM_TYPE_VEC3, "[0 0 0]", "model center in world space" )
		SHADER_PARAM( WARPPARAM, SHADER_PARAM_TYPE_FLOAT, "0.0", "animation parameter between 0 and 1" )
		SHADER_PARAM( SELFILLUMMAP, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map" )
		SHADER_PARAM( UNLIT, SHADER_PARAM_TYPE_BOOL, "0", "disable vertex lighting" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

		if ( !params[SELFILLUMTINT]->IsDefined() )
			params[SELFILLUMTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );
		if ( !params[UNLIT]->IsDefined() )
			params[UNLIT]->SetIntValue( 0 );
	}

	SHADER_FALLBACK
	{
		return g_pHardwareConfig->GetDXSupportLevel() < 80 ? "VertexLitGeneric_DX7" : 0;
	}

	SHADER_INIT
	{
		if ( params[BASETEXTURE]->IsDefined() )
			LoadTexture( BASETEXTURE );
		if ( params[SELFILLUMMAP]->IsDefined() )
			LoadTexture( SELFILLUMMAP );
	}

	SHADER_DRAW
	{
		BlendType_t nBlendType = BT_BLEND;
		if ( params[BASETEXTURE]->GetType() == MATERIAL_VAR_TYPE_TEXTURE )
			nBlendType = EvaluateBlendRequirements( BASETEXTURE, true );

		const bool bUnlit = params[UNLIT]->GetIntValue() != 0;
		const bool bTranslucent = nBlendType == BT_BLEND;
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );

		if ( params[BASETEXTURE]->GetType() == MATERIAL_VAR_TYPE_TEXTURE )
				SetDefaultBlendingShadowState( BASETEXTURE, true );

			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION | VERTEX_NORMAL,
				1, 0, 0, 0 );

			pShaderShadow->SetVertexShader( bUnlit ? "vortwarp_unlit_vs11" : "vortwarp_vs11" );

			if ( bTranslucent )
			{
				EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
				pShaderShadow->EnableAlphaWrites( false );
			}
			else
			{
				pShaderShadow->EnableAlphaWrites( true );
			}

			const int nPixelIndex = ( bTranslucent ? 1 : 0 ) | ( bUnlit ? 2 : 0 );
			pShaderShadow->SetPixelShader( "vortwarp_ps11", nPixelIndex );
			DefaultFog();
		}
		DYNAMIC_STATE
		{
			if ( params[BASETEXTURE]->GetType() == MATERIAL_VAR_TYPE_TEXTURE )
			{
				BindTexture( SHADER_TEXTURE_STAGE0, BASETEXTURE, FRAME );
				SetVertexShaderTextureTransform( 90, BASETEXTURETRANSFORM );
			}
			if ( params[SELFILLUMMAP]->GetType() == MATERIAL_VAR_TYPE_TEXTURE )
				BindTexture( SHADER_TEXTURE_STAGE1, SELFILLUMMAP, -1 );

			const float flInputWarp = params[WARPPARAM]->GetFloatValue();
			float flWarp = VortWarpCubicBezier( flInputWarp, 0.0f, 1.0f, 0.0f, 0.0f );
			flWarp = VortWarpLerp( flInputWarp, flWarp, 1.0f );
			float vWarp[4] = { flWarp, flWarp, flWarp, flWarp };
			pShaderAPI->SetVertexShaderConstant( 92, vWarp, 1 );
			SetVertexShaderConstant( 93, ENTITYORIGIN );

			SetAmbientCubeDynamicStateVertexShader();
			SetModulationPixelShaderDynamicState( 3 );
			EnablePixelShaderOverbright( 0, true, true );
			SetPixelShaderConstant( 1, SELFILLUMTINT );

			float c4[4] = { 0.0f, 0.0f, 0.0f,
				( flWarp > 0.0f && flWarp < 1.0f ) ? 0.0f : 1.0f };
			pShaderAPI->SetPixelShaderConstant( 4, c4, 1 );
			float c5[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			pShaderAPI->SetPixelShaderConstant( 5, c5, 1 );

			const float flTime = (float)pShaderAPI->CurrentTime();
			float vScroll[4] = { 0.11f * flTime, 0.124f * flTime, 0.0f, 0.0f };
			pShaderAPI->SetVertexShaderConstant( 94, vScroll, 1 );

			const int nDynamicVertexIndex = ComputeVertexLitShaderIndex(
				!bUnlit, false, false, false, true );
			pShaderAPI->SetVertexShaderIndex( nDynamicVertexIndex );
		}
		Draw();
	}
END_SHADER
