//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Render Extensions - Client-side render functionality
//          Extends C_BaseEntity with render methods using native 2003 engine data
//
//=============================================================================

#ifndef BMOD_RENDER_EXTENSIONS_H
#define BMOD_RENDER_EXTENSIONS_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"
#include "shareddefs.h"

//-----------------------------------------------------------------------------
// BarrysMod Render Extensions - Full implementations using 2003 engine system
//-----------------------------------------------------------------------------
namespace BMod
{
	//-----------------------------------------------------------------------------
	// Purpose: Set entity render mode using native 2003 engine render system
	//-----------------------------------------------------------------------------
	inline void SetRenderMode( C_BaseEntity *pEntity, RenderMode_t renderMode )
	{
		if ( !pEntity )
			return;

		// Access the native 2003 engine render mode variable directly
		pEntity->m_nRenderMode = (int)renderMode;

		// Entity render state has been updated - no additional notification needed in 2003 engine
	}

	//-----------------------------------------------------------------------------
	// Purpose: Set entity render FX using native 2003 engine render system
	//-----------------------------------------------------------------------------
	inline void SetRenderFX( C_BaseEntity *pEntity, RenderFx_t renderFX )
	{
		if ( !pEntity )
			return;

		// Access the native 2003 engine render FX variable directly
		pEntity->m_nRenderFX = (int)renderFX;

		// Entity render state has been updated - no additional notification needed in 2003 engine
	}

	//-----------------------------------------------------------------------------
	// Purpose: Set entity render color using native 2003 engine render system
	//-----------------------------------------------------------------------------
	inline void SetRenderColor( C_BaseEntity *pEntity, int r, int g, int b, int a )
	{
		if ( !pEntity )
			return;

		// Create color32 structure for native 2003 engine format
		color32 color;
		color.r = (unsigned char)clamp( r, 0, 255 );
		color.g = (unsigned char)clamp( g, 0, 255 );
		color.b = (unsigned char)clamp( b, 0, 255 );
		color.a = (unsigned char)clamp( a, 0, 255 );

		// Access the native 2003 engine render color variable directly
		pEntity->m_clrRender = color;

		// Entity render state has been updated - no additional notification needed in 2003 engine
	}

	//-----------------------------------------------------------------------------
	// Purpose: Get entity render mode from native 2003 engine system
	//-----------------------------------------------------------------------------
	inline RenderMode_t GetRenderMode( C_BaseEntity *pEntity )
	{
		if ( !pEntity )
			return kRenderNormal;

		return (RenderMode_t)pEntity->m_nRenderMode;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Get entity render FX from native 2003 engine system
	//-----------------------------------------------------------------------------
	inline RenderFx_t GetRenderFX( C_BaseEntity *pEntity )
	{
		if ( !pEntity )
			return kRenderFxNone;

		return (RenderFx_t)pEntity->m_nRenderFX;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Get entity render color from native 2003 engine system
	//-----------------------------------------------------------------------------
	inline color32 GetRenderColor( C_BaseEntity *pEntity )
	{
		if ( !pEntity )
		{
			color32 defaultColor = { 255, 255, 255, 255 };
			return defaultColor;
		}

		return pEntity->m_clrRender;
	}
}

//-----------------------------------------------------------------------------
// Note: Global extension methods have been moved to .cpp file to avoid
// ambiguous function call errors with BMod:: namespaced functions.
// Entity manipulation code should use BMod:: namespace directly for better clarity.
//-----------------------------------------------------------------------------

#endif // BMOD_RENDER_EXTENSIONS_H