#include "BaseVSShader.h"

#include "tier0/memdbgon.h"

// The retail Detail material shader is an authoring alias.  Rendering is
// intentionally delegated to the engine's existing UnlitGeneric shader.
BEGIN_VS_SHADER( Detail, "Help for Detail" )
	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS( MATERIAL_VAR_TRANSLUCENT );
	}

	SHADER_FALLBACK
	{
		return "UnlitGeneric";
	}

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
	}
END_SHADER
