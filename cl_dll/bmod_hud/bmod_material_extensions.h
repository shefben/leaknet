//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Material System Extensions - Advanced rendering functionality
//          Implements stencil buffer and color write control for 2003 engine
//
//=============================================================================

#ifndef BMOD_MATERIAL_EXTENSIONS_H
#define BMOD_MATERIAL_EXTENSIONS_H
#ifdef _WIN32
#pragma once
#endif

#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"

//-----------------------------------------------------------------------------
// BarrysMod Material System Extensions - Full implementations for gravity gun effects
//-----------------------------------------------------------------------------
namespace BMod
{
	//-----------------------------------------------------------------------------
	// Stencil buffer operation enums - matching later Source Engine definitions
	//-----------------------------------------------------------------------------
	enum StencilOperation_t
	{
		STENCILOP_KEEP = 0,
		STENCILOP_ZERO,
		STENCILOP_REPLACE,
		STENCILOP_INCR,
		STENCILOP_DECR,
		STENCILOP_INVERT,
		STENCILOP_INCR_WRAP,
		STENCILOP_DECR_WRAP
	};

	enum StencilFunc_t
	{
		STENCILFUNC_NEVER = 0,
		STENCILFUNC_LESS,
		STENCILFUNC_EQUAL,
		STENCILFUNC_LEQUAL,
		STENCILFUNC_GREATER,
		STENCILFUNC_NOTEQUAL,
		STENCILFUNC_GEQUAL,
		STENCILFUNC_ALWAYS
	};

	//-----------------------------------------------------------------------------
	// Material System State Management - stores advanced rendering state
	//-----------------------------------------------------------------------------
	struct MaterialExtensionState_t
	{
		bool bStencilEnabled;
		StencilFunc_t stencilFunc;
		StencilOperation_t stencilPassOp;
		StencilOperation_t stencilFailOp;
		StencilOperation_t stencilZFailOp;
		int stencilRef;
		bool bColorWritesEnabled;

		// Constructor
		MaterialExtensionState_t()
		{
			bStencilEnabled = false;
			stencilFunc = STENCILFUNC_ALWAYS;
			stencilPassOp = STENCILOP_KEEP;
			stencilFailOp = STENCILOP_KEEP;
			stencilZFailOp = STENCILOP_KEEP;
			stencilRef = 0;
			bColorWritesEnabled = true;
		}
	};

	// Global material extension state
	extern MaterialExtensionState_t g_MaterialExtensionState;

	//-----------------------------------------------------------------------------
	// Purpose: Enable/disable stencil testing - full implementation using 2003 engine
	//-----------------------------------------------------------------------------
	void SetStencilEnable( IMaterialSystem *pMaterialSystem, bool bEnable );

	//-----------------------------------------------------------------------------
	// Purpose: Set stencil comparison function - full implementation
	//-----------------------------------------------------------------------------
	void SetStencilFunc( IMaterialSystem *pMaterialSystem, StencilFunc_t func );

	//-----------------------------------------------------------------------------
	// Purpose: Set stencil operations - full implementation
	//-----------------------------------------------------------------------------
	void SetStencilPassOp( IMaterialSystem *pMaterialSystem, StencilOperation_t op );
	void SetStencilFailOp( IMaterialSystem *pMaterialSystem, StencilOperation_t op );
	void SetStencilZFailOp( IMaterialSystem *pMaterialSystem, StencilOperation_t op );

	//-----------------------------------------------------------------------------
	// Purpose: Set stencil reference value - full implementation
	//-----------------------------------------------------------------------------
	void SetStencilRef( IMaterialSystem *pMaterialSystem, int ref );

	//-----------------------------------------------------------------------------
	// Purpose: Enable/disable color writes - full implementation using 2003 engine
	//-----------------------------------------------------------------------------
	void SetColorWritesEnabled( IMaterialSystem *pMaterialSystem, bool bEnable );

	//-----------------------------------------------------------------------------
	// Purpose: Apply current material extension state to rendering pipeline
	//-----------------------------------------------------------------------------
	void ApplyMaterialExtensionState( IMaterialSystem *pMaterialSystem );

	//-----------------------------------------------------------------------------
	// Purpose: Reset material extension state to defaults
	//-----------------------------------------------------------------------------
	void ResetMaterialExtensionState( IMaterialSystem *pMaterialSystem );

	//-----------------------------------------------------------------------------
	// Purpose: Get current stencil/color write state for debugging
	//-----------------------------------------------------------------------------
	const MaterialExtensionState_t& GetMaterialExtensionState();
}

//-----------------------------------------------------------------------------
// Note: Global extension methods have been moved to .cpp file to avoid
// ambiguous function call errors with BMod:: namespaced functions.
// Gravity gun code should use BMod:: namespace directly for better clarity.
//-----------------------------------------------------------------------------

#endif // BMOD_MATERIAL_EXTENSIONS_H