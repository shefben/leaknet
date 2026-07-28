//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Duplicator Tool - Implementation of entity copying tool
//          Based on Garry's Mod tool system analysis
//
//=============================================================================

#include "cbase.h"
#include "gmod_gamemode.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "gamerules.h"
#include "util.h"
#include "props.h"
#include "physics.h"

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
// Console variables for duplicator tool
//-----------------------------------------------------------------------------
ConVar gmod_duplicator_limit("gmod_duplicator_limit", "50", FCVAR_ARCHIVE, "Maximum entities per duplication");

//-----------------------------------------------------------------------------
// Snapshot of a single entity's state for later re-creation
//-----------------------------------------------------------------------------
struct DupeEntityInfo_t
{
	char			szClassName[64];
	char			szModel[256];
	Vector			vecPosition;
	QAngle			angAngles;
	Vector			vecVelocity;
	AngularImpulse	angVelocity;
	int				nHealth;
	int				nMaxHealth;
	float			flMass;
	int				r, g, b, a;
	int				nRenderMode;
	int				nRenderFX;
	int				nSkin;
	int				nBody;
	bool			bMoveable;

	DupeEntityInfo_t()
	{
		szClassName[0] = '\0';
		szModel[0] = '\0';
		vecPosition = vec3_origin;
		angAngles.Init();
		vecVelocity = vec3_origin;
		angVelocity.Init();
		nHealth = 0;
		nMaxHealth = 0;
		flMass = 0.0f;
		r = g = b = a = 255;
		nRenderMode = 0;
		nRenderFX = 0;
		nSkin = 0;
		nBody = 0;
		bMoveable = true;
	}
};

//-----------------------------------------------------------------------------
// Per-player duplication clipboard - duplication is conceptually per-player,
// so each player gets their own copy buffer rather than one per weapon_tool
// instance (matches GMod: your clipboard survives switching away and back).
//-----------------------------------------------------------------------------
struct DupeClipboard_t
{
	CHandle<CBasePlayer>			hOwner;
	CUtlVector<DupeEntityInfo_t*>	entities;
	Vector							vecCenterPoint;
	bool							bValid;
	float							flLastCopyTime;
	float							flLastPasteTime;

	DupeClipboard_t()
	{
		vecCenterPoint = vec3_origin;
		bValid = false;
		flLastCopyTime = 0.0f;
		flLastPasteTime = 0.0f;
	}

	~DupeClipboard_t()
	{
		Clear();
	}

	void Clear()
	{
		for ( int i = 0; i < entities.Count(); i++ )
		{
			delete entities[i];
		}
		entities.RemoveAll();
		bValid = false;
	}
};

//-----------------------------------------------------------------------------
// Global per-player clipboard registry
//-----------------------------------------------------------------------------
static CUtlVector<DupeClipboard_t*> g_DupeClipboards;

static DupeClipboard_t *FindDupeClipboard( CBasePlayer *pOwner )
{
	for ( int i = 0; i < g_DupeClipboards.Count(); i++ )
	{
		if ( g_DupeClipboards[i]->hOwner.Get() == pOwner )
			return g_DupeClipboards[i];
	}
	return NULL;
}

static DupeClipboard_t *GetOrCreateDupeClipboard( CBasePlayer *pOwner )
{
	DupeClipboard_t *pClip = FindDupeClipboard( pOwner );
	if ( pClip )
		return pClip;

	pClip = new DupeClipboard_t;
	pClip->hOwner = pOwner;
	g_DupeClipboards.AddToTail( pClip );
	return pClip;
}

//-----------------------------------------------------------------------------
// Save entity information for duplication
//-----------------------------------------------------------------------------
static void SaveEntityInfo( CBaseEntity *pEntity, DupeEntityInfo_t *pInfo )
{
	Q_strncpy( pInfo->szClassName, pEntity->GetClassname(), sizeof( pInfo->szClassName ) );

	if ( pEntity->GetModelName() != NULL_STRING )
	{
		Q_strncpy( pInfo->szModel, STRING( pEntity->GetModelName() ), sizeof( pInfo->szModel ) );
	}

	pInfo->vecPosition = pEntity->GetAbsOrigin();
	pInfo->angAngles = pEntity->GetAbsAngles();
	pInfo->nHealth = pEntity->GetHealth();
	pInfo->nMaxHealth = pEntity->GetMaxHealth();

	color32 renderColor = pEntity->GetRenderColor();
	pInfo->r = renderColor.r;
	pInfo->g = renderColor.g;
	pInfo->b = renderColor.b;
	pInfo->a = renderColor.a;
	pInfo->nRenderMode = pEntity->GetRenderMode();
	pInfo->nRenderFX = pEntity->GetRenderFX();

	IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
	if ( pPhysics )
	{
		pPhysics->GetVelocity( &pInfo->vecVelocity, &pInfo->angVelocity );
		pInfo->flMass = pPhysics->GetMass();
		pInfo->bMoveable = pPhysics->IsMoveable();
	}

	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating*>( pEntity );
	if ( pAnimating )
	{
		pInfo->nSkin = pAnimating->m_nSkin;
		pInfo->nBody = pAnimating->m_nBody;
	}
}

//-----------------------------------------------------------------------------
// Create entity from saved info
//-----------------------------------------------------------------------------
static CBaseEntity *CreateEntityFromInfo( CBasePlayer *pOwner, DupeEntityInfo_t *pInfo, const Vector &vecOffset )
{
	CBaseEntity *pEntity = CreateEntityByName( pInfo->szClassName );
	if ( !pEntity )
	{
		DevMsg( "Duplicator: failed to create entity '%s'\n", pInfo->szClassName );
		return NULL;
	}

	pEntity->SetAbsOrigin( pInfo->vecPosition + vecOffset );
	pEntity->SetAbsAngles( pInfo->angAngles );

	if ( pInfo->szModel[0] )
	{
		pEntity->SetModel( pInfo->szModel );
	}

	if ( pOwner )
	{
		pEntity->SetOwnerEntity( pOwner );
	}

	// GMod gamemode rules can veto duplicating certain ragdolls/props (e.g.
	// NPC corpses that aren't meant to be spammed).
	bool bIsRagdoll = !Q_stricmp( pInfo->szClassName, "prop_ragdoll" );
	bool bIsProp = !Q_stricmp( pInfo->szClassName, "prop_physics" ) || !Q_stricmp( pInfo->szClassName, "physics_prop" );

	if ( bIsRagdoll && !CGModGamemodeSystem::CanPlayerDuplicateRagdoll( pOwner, pEntity ) )
	{
		UTIL_Remove( pEntity );
		return NULL;
	}
	else if ( bIsProp && !CGModGamemodeSystem::CanPlayerDuplicateProp( pOwner, pEntity ) )
	{
		UTIL_Remove( pEntity );
		return NULL;
	}

	pEntity->Spawn();
	pEntity->Activate();

	pEntity->SetRenderColor( pInfo->r, pInfo->g, pInfo->b, pInfo->a );
	pEntity->SetRenderMode( pInfo->nRenderMode );
	pEntity->SetRenderFX( pInfo->nRenderFX );

	IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
	if ( pPhysics )
	{
		if ( pInfo->flMass > 0.0f )
		{
			pPhysics->SetMass( pInfo->flMass );
		}

		AngularImpulse angVelocity = pInfo->angVelocity;
		pPhysics->SetVelocity( &pInfo->vecVelocity, &angVelocity );
		pPhysics->EnableMotion( pInfo->bMoveable );
	}

	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating*>( pEntity );
	if ( pAnimating )
	{
		pAnimating->m_nSkin = pInfo->nSkin;
		pAnimating->m_nBody = pInfo->nBody;
	}

	return pEntity;
}

//-----------------------------------------------------------------------------
// Copy single entity into the clipboard
//-----------------------------------------------------------------------------
static bool CopyEntity( DupeClipboard_t *pClip, CBaseEntity *pEntity )
{
	if ( !pEntity )
		return false;

	pClip->Clear();

	DupeEntityInfo_t *pInfo = new DupeEntityInfo_t;
	SaveEntityInfo( pEntity, pInfo );

	pClip->entities.AddToTail( pInfo );
	pClip->vecCenterPoint = pEntity->GetAbsOrigin();
	pClip->bValid = true;

	return true;
}

//-----------------------------------------------------------------------------
// Copy every physics prop within a radius into the clipboard
//-----------------------------------------------------------------------------
static bool CopyArea( DupeClipboard_t *pClip, const Vector &vecCenter, float flRadius )
{
	pClip->Clear();

	CBaseEntity *pEntity = NULL;
	int nCount = 0;

	while ( ( pEntity = gEntList.FindEntityInSphere( pEntity, vecCenter, flRadius ) ) != NULL )
	{
		if ( pEntity->IsPlayer() || pEntity->entindex() == 0 )
			continue;

		if ( !pEntity->VPhysicsGetObject() )
			continue;

		if ( nCount >= gmod_duplicator_limit.GetInt() )
		{
			DevMsg( "Duplicator: hit entity limit (%d)\n", gmod_duplicator_limit.GetInt() );
			break;
		}

		DupeEntityInfo_t *pInfo = new DupeEntityInfo_t;
		SaveEntityInfo( pEntity, pInfo );
		pClip->entities.AddToTail( pInfo );

		nCount++;
	}

	if ( nCount == 0 )
		return false;

	pClip->vecCenterPoint = vecCenter;
	pClip->bValid = true;
	return true;
}

//-----------------------------------------------------------------------------
// Paste every stored entity, offset from the original copy location
//-----------------------------------------------------------------------------
static int PasteEntities( CBasePlayer *pOwner, DupeClipboard_t *pClip, const Vector &vecPastePos )
{
	if ( !pClip->bValid || pClip->entities.Count() == 0 )
		return 0;

	Vector vecOffset = vecPastePos - pClip->vecCenterPoint;
	int nSpawned = 0;

	for ( int i = 0; i < pClip->entities.Count(); i++ )
	{
		if ( CreateEntityFromInfo( pOwner, pClip->entities[i], vecOffset ) )
		{
			nSpawned++;
		}
	}

	return nSpawned;
}

//-----------------------------------------------------------------------------
// Tool dispatch - Duplicate(15)
//-----------------------------------------------------------------------------
void Tool_Duplicator_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	DupeClipboard_t *pClip = GetOrCreateDupeClipboard( pOwner );

	if ( bPrimary )
	{
		if ( pEntity && pEntity != pOwner && pEntity->VPhysicsGetObject() )
		{
			if ( ( gpGlobals->curtime - pClip->flLastCopyTime ) < 1.0f )
				return;

			bool bCopied;
			if ( pOwner->m_nButtons & IN_USE )
			{
				// Use + primary = copy every prop in a radius around the target
				bCopied = CopyArea( pClip, pEntity->GetAbsOrigin(), 256.0f );
				if ( bCopied )
				{
					pTool->PlayToolSound( "garrysmod/save_sound1.wav" );
					ClientPrintf( pOwner, HUD_PRINTTALK, "Copied area (%d entities)", pClip->entities.Count() );
				}
			}
			else
			{
				bCopied = CopyEntity( pClip, pEntity );
				if ( bCopied )
				{
					pTool->PlayToolSound( "garrysmod/save_sound1.wav" );
					ClientPrintf( pOwner, HUD_PRINTTALK, "Copied %s", pEntity->GetClassname() );
				}
			}

			pClip->flLastCopyTime = gpGlobals->curtime;
		}
		else
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "No valid entity to copy" );
		}
	}
	else
	{
		if ( !pClip->bValid )
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "No entities copied yet" );
			return;
		}

		if ( ( gpGlobals->curtime - pClip->flLastPasteTime ) < 2.0f )
			return;

		int nSpawned = PasteEntities( pOwner, pClip, tr.endpos );
		if ( nSpawned > 0 )
		{
			pTool->PlayToolSound( "garrysmod/save_sound2.wav" );
			ClientPrintf( pOwner, HUD_PRINTTALK, "Pasted %d entities", nSpawned );
		}
		else
		{
			ClientPrintf( pOwner, HUD_PRINTTALK, "Failed to paste entities" );
		}

		pClip->flLastPasteTime = gpGlobals->curtime;
	}
}

void Tool_Duplicator_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary )
{
	CBasePlayer *pOwner = ToBasePlayer( pTool->GetOwner() );
	if ( !pOwner )
		return;

	if ( bPrimary )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "No valid entity to copy" );
		return;
	}

	DupeClipboard_t *pClip = GetOrCreateDupeClipboard( pOwner );
	if ( !pClip->bValid )
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "No entities copied yet" );
		return;
	}

	if ( ( gpGlobals->curtime - pClip->flLastPasteTime ) < 2.0f )
		return;

	int nSpawned = PasteEntities( pOwner, pClip, tr.endpos );
	if ( nSpawned > 0 )
	{
		pTool->PlayToolSound( "garrysmod/save_sound2.wav" );
		ClientPrintf( pOwner, HUD_PRINTTALK, "Pasted %d entities", nSpawned );
	}
	else
	{
		ClientPrintf( pOwner, HUD_PRINTTALK, "Failed to paste entities" );
	}

	pClip->flLastPasteTime = gpGlobals->curtime;
}

void Tool_Duplicator_OnThink( CWeaponTool *pTool )
{
	// Prune clipboards belonging to players who have since disconnected.
	for ( int i = g_DupeClipboards.Count() - 1; i >= 0; i-- )
	{
		if ( !g_DupeClipboards[i]->hOwner.Get() )
		{
			delete g_DupeClipboards[i];
			g_DupeClipboards.Remove( i );
		}
	}
}

//-----------------------------------------------------------------------------
// Console command for duplicator context menu
//-----------------------------------------------------------------------------
CON_COMMAND( gmod_context_duplicator, "Opens duplicator tool context menu" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pPlayer->GetActiveWeapon() );
	if ( !pTool || pTool->GetToolMode() != TOOL_DUPLICATE )
	{
		ClientPrintf( pPlayer, HUD_PRINTTALK, "Duplicator tool must be equipped and selected" );
		return;
	}

	DupeClipboard_t *pClip = FindDupeClipboard( pPlayer );

	ClientPrintf( pPlayer, HUD_PRINTTALK, "Duplicator Tool:" );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Left click: Copy single entity" );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Use + Left click: Copy area" );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Right click: Paste entities" );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Entity limit: %d", gmod_duplicator_limit.GetInt() );
	ClientPrintf( pPlayer, HUD_PRINTTALK, "Clipboard: %s (%d entities)",
		( pClip && pClip->bValid ) ? "Ready" : "Empty",
		pClip ? pClip->entities.Count() : 0 );
}
