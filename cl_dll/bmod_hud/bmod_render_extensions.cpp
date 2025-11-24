//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Render Extensions - Implementation
//          Full render functionality using native 2003 engine capabilities
//
//=============================================================================

#include "cbase.h"
#include "bmod_render_extensions.h"
#include "c_baseentity.h"

//-----------------------------------------------------------------------------
// Extension Method Implementations - Full functionality using 2003 engine
// Note: Global functions removed to avoid ambiguous calls with BMod:: namespace
// Code should use BMod::SetRenderMode, BMod::SetRenderFX, BMod::SetRenderColor directly
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// BarrysMod Entity Extension Class - Add methods to C_BaseEntity instances
// This provides the object-oriented interface that GMod code expects
//-----------------------------------------------------------------------------

// Define a namespace for entity method extensions
namespace BModEntityExtensions
{
	//-----------------------------------------------------------------------------
	// Purpose: Add SetRenderMode method to C_BaseEntity instances
	//-----------------------------------------------------------------------------
	void AddSetRenderModeMethod( C_BaseEntity *pEntity, RenderMode_t renderMode )
	{
		BMod::SetRenderMode( pEntity, renderMode );
	}

	//-----------------------------------------------------------------------------
	// Purpose: Add SetRenderFX method to C_BaseEntity instances
	//-----------------------------------------------------------------------------
	void AddSetRenderFXMethod( C_BaseEntity *pEntity, RenderFx_t renderFX )
	{
		BMod::SetRenderFX( pEntity, renderFX );
	}

	//-----------------------------------------------------------------------------
	// Purpose: Add SetRenderColor method to C_BaseEntity instances
	//-----------------------------------------------------------------------------
	void AddSetRenderColorMethod( C_BaseEntity *pEntity, int r, int g, int b, int a )
	{
		BMod::SetRenderColor( pEntity, r, g, b, a );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Initialize BarrysMod render extensions
//-----------------------------------------------------------------------------
void InitializeBModRenderExtensions()
{
	DevMsg( "BarrysMod: Render extensions initialized using native 2003 engine system\n" );
}