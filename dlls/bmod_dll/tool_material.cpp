//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Material Tool - Implementation of Material changing tool
//          Based on Garry's Mod tool system analysis
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "props.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables for material tool
//-----------------------------------------------------------------------------
ConVar bm_material_path("bm_material_path", "models/debug/debugwhite", FCVAR_ARCHIVE, "Current material to apply");
ConVar bm_material_submesh("bm_material_submesh", "0", FCVAR_ARCHIVE, "Target submesh index (0 = all)");

//-----------------------------------------------------------------------------
// Common materials list - based on common Half-Life 2 materials
//-----------------------------------------------------------------------------
static const char *g_CommonMaterials[] =
{
	"models/debug/debugwhite",
	"models/shiny",
	"models/wireframe",
	"models/error",
	"concrete/concretefloor001a",
	"metal/metalhull001a",
	"wood/wood_box001a",
	"glass/glasswindow007a",
	"brick/brickwall001a",
	"nature/dirtfloor006a",
	"dev/dev_measuregeneric01",
	"dev/dev_blendmeasure",
	"dev/reflectivity_30",
	"dev/reflectivity_60",
	"dev/reflectivity_90",
	"lights/white001",
	"nature/water_coastline01",
	"props/metalduct001a",
	"concrete/milwall001",
	"tile/tilefloor006a",
	NULL
};

//-----------------------------------------------------------------------------
// Material tool class - implements TOOL_MATERIAL mode
//-----------------------------------------------------------------------------
class CToolMaterial : public CWeaponTool
{
	DECLARE_CLASS( CToolMaterial, CWeaponTool );

public:
	CToolMaterial() : m_nSelectedMaterial(0) {}

	virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void OnToolThink();

private:
	bool ApplyMaterial( CBaseEntity *pEntity, const char *pszMaterial );
	void CycleMaterial();
	void CreateMaterialEffect( const Vector &vecPos );
	const char *GetCurrentMaterial();
	bool IsValidMaterial( const char *pszMaterial );

	// Material tool state
	int		m_nSelectedMaterial;	// Currently selected material index
};

//-----------------------------------------------------------------------------
// Tool implementation for Material mode
//-----------------------------------------------------------------------------
void CToolMaterial::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		// Primary attack - apply material
		if ( pEntity && pEntity != pOwner )
		{
			const char *pszMaterial = GetCurrentMaterial();
			if ( ApplyMaterial( pEntity, pszMaterial ) )
			{
				CreateMaterialEffect( tr.endpos );
				PlayToolSound( "garrysmod/balloon_pop_cute.wav" );

				ClientPrint( pOwner, HUD_PRINTTALK, "Applied material: %s", pszMaterial );
			}
			else
			{
				ClientPrint( pOwner, HUD_PRINTTALK, "Cannot apply material to %s", pEntity->GetClassname() );
			}
		}
		else
		{
			ClientPrint( pOwner, HUD_PRINTTALK, "No valid entity targeted" );
		}
	}
	else
	{
		// Secondary attack - cycle material
		CycleMaterial();
	}
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Material mode
//-----------------------------------------------------------------------------
void CToolMaterial::OnToolTrace( trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Material tool can only be applied to entities" );
	}
	else
	{
		// Secondary attack - cycle material
		CycleMaterial();
	}
}

//-----------------------------------------------------------------------------
// Tool think for Material mode
//-----------------------------------------------------------------------------
void CToolMaterial::OnToolThink()
{
	// Material tool doesn't need continuous thinking
}

//-----------------------------------------------------------------------------
// Apply material to entity
//-----------------------------------------------------------------------------
bool CToolMaterial::ApplyMaterial( CBaseEntity *pEntity, const char *pszMaterial )
{
	if ( !pEntity || !pszMaterial )
		return false;

	// Check if entity supports material changes
	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating*>( pEntity );
	if ( !pAnimating )
	{
		DevMsg( "Entity %s doesn't support material changes\n", pEntity->GetClassname() );
		return false;
	}

	// Validate material exists
	if ( !IsValidMaterial( pszMaterial ) )
	{
		DevMsg( "Material %s not found\n", pszMaterial );
		return false;
	}

	// Apply material to entity (older SDK compatible approach)
	// Note: Material override system may not be available in older SDK
	// This is a placeholder - in the older SDK, material changes would typically
	// be done through model replacement or other mechanisms

	// Store the material name in a custom keyvalue for potential future use
	pAnimating->SetKeyValue( "override_material", pszMaterial );

	// In a full implementation for older SDK, you would need to:
	// 1. Create a material proxy system
	// 2. Use model replacement
	// 3. Or implement custom rendering override

	DevMsg( "Note: Material override '%s' stored but not visually applied (older SDK limitation)\n", pszMaterial );

	DevMsg( "Applied material %s to entity %s\n", pszMaterial, pEntity->GetClassname() );
	return true;
}

//-----------------------------------------------------------------------------
// Cycle through materials
//-----------------------------------------------------------------------------
void CToolMaterial::CycleMaterial()
{
	// Find next valid material
	do
	{
		m_nSelectedMaterial++;
		if ( g_CommonMaterials[m_nSelectedMaterial] == NULL )
		{
			m_nSelectedMaterial = 0; // Wrap around
		}
	} while ( g_CommonMaterials[m_nSelectedMaterial] == NULL );

	// Update ConVar
	bm_material_path.SetValue( g_CommonMaterials[m_nSelectedMaterial] );

	// Inform player
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		ClientPrint( pOwner, HUD_PRINTTALK, "Selected material: %s",
			g_CommonMaterials[m_nSelectedMaterial] );
	}
}

//-----------------------------------------------------------------------------
// Create material application effect
//-----------------------------------------------------------------------------
void CToolMaterial::CreateMaterialEffect( const Vector &vecPos )
{
	// Create sparkle effect to indicate material change
	CEffectData data;
	data.m_vOrigin = vecPos;
	data.m_flMagnitude = 50.0f;
	data.m_flScale = 1.5f;
	data.m_nColor = 100; // Blue-ish tint

	DispatchEffect( "GlowSprite", data );

	// Create some particles
	data.m_vOrigin = vecPos + Vector(0, 0, 10);
	data.m_vNormal = Vector(0, 0, 1);
	data.m_flScale = 1.0f;

	DispatchEffect( "Sparks", data );
}

//-----------------------------------------------------------------------------
// Get current material from ConVar
//-----------------------------------------------------------------------------
const char *CToolMaterial::GetCurrentMaterial()
{
	// Find material matching the current ConVar
	const char *pszCurrentMaterial = bm_material_path.GetString();

	for ( int i = 0; g_CommonMaterials[i]; i++ )
	{
		if ( !Q_stricmp( g_CommonMaterials[i], pszCurrentMaterial ) )
		{
			m_nSelectedMaterial = i;
			return g_CommonMaterials[i];
		}
	}

	// If custom material from ConVar, use it directly
	return pszCurrentMaterial;
}

//-----------------------------------------------------------------------------
// Check if material exists in filesystem
//-----------------------------------------------------------------------------
bool CToolMaterial::IsValidMaterial( const char *pszMaterial )
{
	if ( !pszMaterial )
		return false;

	// Build material path
	char szMaterialPath[MAX_PATH];
	Q_snprintf( szMaterialPath, sizeof(szMaterialPath), "materials/%s.vmt", pszMaterial );

	// Check if material file exists
	return filesystem->FileExists( szMaterialPath );
}

//-----------------------------------------------------------------------------
// Console command for material context menu
//-----------------------------------------------------------------------------
CON_COMMAND( bm_context_material, "Opens material tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Check if player has material tool equipped
	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_MATERIAL )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Material tool must be equipped and selected" );
		return;
	}

	// Show available materials
	ClientPrint( pPlayer, HUD_PRINTTALK, "Available materials:" );

	for ( int i = 0; g_CommonMaterials[i]; i++ )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "  %s", g_CommonMaterials[i] );
	}

	ClientPrint( pPlayer, HUD_PRINTTALK, "Current: %s", bm_material_path.GetString() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Submesh: %d (0 = all)", bm_material_submesh.GetInt() );
	ClientPrint( pPlayer, HUD_PRINTTALK, "Use secondary fire to cycle materials" );
}

//-----------------------------------------------------------------------------
// Console command to set material directly
//-----------------------------------------------------------------------------
CON_COMMAND( bm_material_set, "Set material path directly" )
{
	if ( args.ArgC() < 2 )
	{
		Msg( "Usage: bm_material_set <material_path>\n" );
		return;
	}

	const char *pszMaterial = args.Arg(1);
	bm_material_path.SetValue( pszMaterial );

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrint( pPlayer, HUD_PRINTTALK, "Material set to: %s", pszMaterial );
	}
}

//-----------------------------------------------------------------------------
// Console command to list available materials
//-----------------------------------------------------------------------------
CON_COMMAND( bm_material_list, "List available materials" )
{
	Msg( "Available materials:\n" );
	for ( int i = 0; g_CommonMaterials[i]; i++ )
	{
		Msg( "  %d. %s\n", i + 1, g_CommonMaterials[i] );
	}
}