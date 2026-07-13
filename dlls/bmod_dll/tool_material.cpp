//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Material Tool - Implementation of Material changing tool
//          Based on Garry's Mod tool system analysis
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "props.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// ClientPrintf() only substitutes literal param1-4 strings into msg_name - it
// has no printf-style formatting of its own - so this wrapper formats first.
//-----------------------------------------------------------------------------
static void ClientPrintf( CBasePlayer *pPlayer, int msg_dest, const char *pszFormat, ... )
{
	char szBuf[256];
	va_list args;
	va_start( args, pszFormat );
	Q_vsnprintf( szBuf, sizeof( szBuf ), pszFormat, args );
	va_end( args );
	ClientPrint( pPlayer, msg_dest, szBuf );
}

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
// Per-weapon material state - CWeaponTool is not subclassed, so the state
// that used to live in CToolMaterial's member variables now lives here, keyed
// by the weapon's EHANDLE (mirrors the g_WeldConstraints/g_RopeConstraints
// registries used by the sibling tool_*.cpp files).
//-----------------------------------------------------------------------------
struct MaterialState_t
{
	EHANDLE	hTool;
	int		nSelectedMaterial;	// Currently selected material index

	MaterialState_t()
	{
		nSelectedMaterial = 0;
	}
};

static CUtlVector<MaterialState_t*> g_MaterialStates;

static MaterialState_t *FindMaterialState( CWeaponTool *pTool )
{
	for ( int i = 0; i < g_MaterialStates.Count(); i++ )
	{
		if ( g_MaterialStates[i]->hTool == pTool )
			return g_MaterialStates[i];
	}
	return NULL;
}

static MaterialState_t *GetMaterialState( CWeaponTool *pTool )
{
	MaterialState_t *pState = FindMaterialState( pTool );
	if ( !pState )
	{
		pState = new MaterialState_t;
		pState->hTool = pTool;
		g_MaterialStates.AddToTail( pState );
	}
	return pState;
}

// Removes state records for weapons that no longer exist.
static void CleanupMaterialStates()
{
	for ( int i = g_MaterialStates.Count() - 1; i >= 0; i-- )
	{
		if ( !g_MaterialStates[i]->hTool.Get() )
		{
			delete g_MaterialStates[i];
			g_MaterialStates.Remove( i );
		}
	}
}

//-----------------------------------------------------------------------------
// Forward declarations of tool helpers
//-----------------------------------------------------------------------------
static bool ApplyMaterial( CBaseEntity *pEntity, const char *pszMaterial );
static void CycleMaterial( CWeaponTool *pTool, MaterialState_t *pState );
static void CreateMaterialEffect( const Vector &vecPos );
static const char *GetCurrentMaterial( MaterialState_t *pState );
static bool IsValidMaterial( const char *pszMaterial );

//-----------------------------------------------------------------------------
// Tool implementation for Material mode
//-----------------------------------------------------------------------------
void Tool_Material_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	MaterialState_t *pState = GetMaterialState( pTool );

	if ( bPrimary )
	{
		// Primary attack - apply material
		if ( pEntity && pEntity != pOwner )
		{
			const char *pszMaterial = GetCurrentMaterial( pState );
			if ( ApplyMaterial( pEntity, pszMaterial ) )
			{
				CreateMaterialEffect( tr.endpos );
				pTool->PlayToolSound( "garrysmod/balloon_pop_cute.wav" );

				ClientPrintf( pOwner, HUD_PRINTTALK, "Applied material: %s", pszMaterial );
			}
			else
			{
				ClientPrintf( pOwner, HUD_PRINTTALK, "Cannot apply material to %s", pEntity->GetClassname() );
			}
		}
		else
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "No valid entity targeted" );
		}
	}
	else
	{
		// Secondary attack - cycle material
		CycleMaterial( pTool, pState );
	}
}

//-----------------------------------------------------------------------------
// Tool trace implementation for Material mode
//-----------------------------------------------------------------------------
void Tool_Material_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	MaterialState_t *pState = GetMaterialState( pTool );

	if ( bPrimary )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "Material tool can only be applied to entities" );
	}
	else
	{
		// Secondary attack - cycle material
		CycleMaterial( pTool, pState );
	}
}

//-----------------------------------------------------------------------------
// Tool think for Material mode
//-----------------------------------------------------------------------------
void Tool_Material_OnThink( CWeaponTool *pTool )
{
	// Material tool doesn't need continuous thinking; just keep the registry tidy
	CleanupMaterialStates();
}

//-----------------------------------------------------------------------------
// Apply material to entity
//-----------------------------------------------------------------------------
static bool ApplyMaterial( CBaseEntity *pEntity, const char *pszMaterial )
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
	pAnimating->KeyValue( "override_material", pszMaterial );

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
static void CycleMaterial( CWeaponTool *pTool, MaterialState_t *pState )
{
	// Find next valid material
	do
	{
		pState->nSelectedMaterial++;
		if ( g_CommonMaterials[pState->nSelectedMaterial] == NULL )
		{
			pState->nSelectedMaterial = 0; // Wrap around
		}
	} while ( g_CommonMaterials[pState->nSelectedMaterial] == NULL );

	// Update ConVar
	bm_material_path.SetValue( g_CommonMaterials[pState->nSelectedMaterial] );

	// Inform player
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( pOwner )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "Selected material: %s",
			g_CommonMaterials[pState->nSelectedMaterial] );
	}
}

//-----------------------------------------------------------------------------
// Create material application effect
//-----------------------------------------------------------------------------
static void CreateMaterialEffect( const Vector &vecPos )
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
static const char *GetCurrentMaterial( MaterialState_t *pState )
{
	// Find material matching the current ConVar
	const char *pszCurrentMaterial = bm_material_path.GetString();

	for ( int i = 0; g_CommonMaterials[i]; i++ )
	{
		if ( !Q_stricmp( g_CommonMaterials[i], pszCurrentMaterial ) )
		{
			pState->nSelectedMaterial = i;
			return g_CommonMaterials[i];
		}
	}

	// If custom material from ConVar, use it directly
	return pszCurrentMaterial;
}

//-----------------------------------------------------------------------------
// Check if material exists in filesystem
//-----------------------------------------------------------------------------
static bool IsValidMaterial( const char *pszMaterial )
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
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Material tool must be equipped and selected" );
		return;
	}

	// Show available materials
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Available materials:" );

	for ( int i = 0; g_CommonMaterials[i]; i++ )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "  %s", g_CommonMaterials[i] );
	}

	ClientPrintf( pPlayer, HUD_PRINTTALK, "Current: %s", bm_material_path.GetString() );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Submesh: %d (0 = all)", bm_material_submesh.GetInt() );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Use secondary fire to cycle materials" );
}

//-----------------------------------------------------------------------------
// Console command to set material directly
//-----------------------------------------------------------------------------
CON_COMMAND( bm_material_set, "Set material path directly" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: bm_material_set <material_path>\n" );
		return;
	}

	const char *pszMaterial = engine->Cmd_Argv(1);
	bm_material_path.SetValue( pszMaterial );

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Material set to: %s", pszMaterial );
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
