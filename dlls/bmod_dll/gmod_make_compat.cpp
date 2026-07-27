#include "cbase.h"
#include "player.h"
#include "keyvalues.h"
#include "engine/ivmodelinfo.h"
#include "gmod_make.h"
#include "physics_prop_ragdoll.h"
#include "vcollide.h"
#include "vstdlib/strtools.h"

#include "tier0/memdbgon.h"

extern IVModelInfo *modelinfo;

//-----------------------------------------------------------------------------
// A model whose .phy holds more than one solid joined by ragdoll constraints is
// a ragdoll no matter which spawn-menu button the player clicked. Spawning one
// as prop_physics/prop_dynamic produces a rigid statue that floats upright,
// isn't solid to the physgun trace, and fails CWeaponGravityGun::AttachObject's
// MOVETYPE_VPHYSICS test - so route it to prop_ragdoll instead.
//
// NOTE: solidCount alone isn't enough. Plenty of ordinary props compile to
// several convex pieces; only ragdolls emit ragdollconstraint blocks.
//-----------------------------------------------------------------------------
bool ModelIsRagdoll( int modelindex )
{
	if ( modelindex <= 0 )
		return false;

	vcollide_t *pCollide = modelinfo->GetVCollide( modelindex );
	if ( !pCollide || pCollide->solidCount <= 1 || !pCollide->pKeyValues )
		return false;

	return Q_stristr( pCollide->pKeyValues, "ragdollconstraint" ) != NULL;
}

// CBaseProp::Spawn() (dlls/props.cpp) deletes a "prop_physics" outright if its
// model has no "prop_data" keyvalues section (real props require it), and
// separately deletes anything OTHER than prop_physics/physics_prop/
// prop_physics_override/prop_dynamic_override if the model DOES have one.
// Spawning everything as prop_physics unconditionally meant any model
// without prop_data (e.g. models/props_canal/boat001a.mdl) got silently
// deleted right after spawn. Pick the entity class the same way the real
// spawn menu does: only use prop_physics when the model actually has
// prop_data, otherwise fall back to prop_dynamic.
const char *PickPropClassForModel( const char *model, int modelindex )
{
	// A jointed ragdoll is never a usable prop_physics/prop_dynamic.
	if ( ModelIsRagdoll( modelindex ) )
		return "prop_ragdoll";

	const model_t *pModel = modelinfo->GetModel( modelindex );
	if ( !pModel )
		return "prop_dynamic";

	const char *pKeyValueText = modelinfo->GetModelKeyValueText( pModel );
	if ( !pKeyValueText || !*pKeyValueText )
		return "prop_dynamic";

	KeyValues *pModelKV = new KeyValues( "" );
	bool bHasPropData = false;
	if ( pModelKV->LoadFromBuffer( model, pKeyValueText ) )
	{
		bHasPropData = ( pModelKV->FindKey( "prop_data" ) != NULL );
	}
	pModelKV->deleteThis();

	return bHasPropData ? "prop_physics" : "prop_dynamic";
}

//-----------------------------------------------------------------------------
// Shared spawn path for gmod_makeragdoll / gmod_makeprop / gm_spawn. Handles
// the runtime precache, entity class selection, placement and dispatch.
// bForceRagdoll is for the menu's '#' entries, which should ragdoll even if the
// .phy doesn't look jointed. Returns NULL if the model or entity couldn't be
// created.
//-----------------------------------------------------------------------------
CBaseEntity *GModSpawnModelAtCrosshair( CBasePlayer *player, const char *model, bool bForceRagdoll )
{
	if ( !player || !model || !model[0] )
		return NULL;

	Vector forward;
	AngleVectors( player->EyeAngles(), &forward );

	trace_t trace;
	UTIL_TraceLine(
		player->EyePosition(),
		player->EyePosition() + forward * 200.0f,
		MASK_SOLID,
		player,
		COLLISION_GROUP_NONE,
		&trace );

	// This runs long after the map's own precache phase, so SetModel() (via
	// UTIL_SetModel -> modelinfo->GetModelIndex) would otherwise hit an
	// unprecached model and call Error("no precache: ...") - a fatal message
	// box + crash. Precache it first (lifting the same "too late" assert gate
	// used by npc_create, dlls/ai_concommands.cpp) before setting the model.
	bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );
	int modelindex = engine->PrecacheModel( model );
	CBaseEntity::SetAllowPrecache( bAllowPrecache );

	char szMsg[256];
	if ( modelindex <= 0 )
	{
		// Bad path, or the model precache table is full. Calling SetModel()
		// anyway would be a fatal Error(), so stop here.
		Q_snprintf( szMsg, sizeof(szMsg), "Couldn't precache '%s'\n", model );
		ClientPrint( player, HUD_PRINTTALK, szMsg );
		return NULL;
	}

	const char *classname = ( bForceRagdoll || ModelIsRagdoll( modelindex ) )
		? "prop_ragdoll"
		: PickPropClassForModel( model, modelindex );

	CBaseEntity *pEnt = CreateEntityByName( classname );
	if ( !pEnt )
	{
		Q_snprintf( szMsg, sizeof(szMsg), "Unable to create %s\n", classname );
		ClientPrint( player, HUD_PRINTTALK, szMsg );
		return NULL;
	}

	pEnt->SetModel( model );

	// CRagdollProp::Spawn() builds the ragdoll's bone transforms from whatever
	// sequence is current, so the death pose has to be selected before
	// DispatchSpawn() or the ragdoll is built in the bind pose.
	CRagdollProp *pRagdoll = dynamic_cast<CRagdollProp *>( pEnt );
	if ( pRagdoll )
	{
		pRagdoll->InitRagdollAnimation();
	}

	// trace.endpos sits exactly on the surface, which leaves the model halfway
	// inside it. Back off along the hit normal and let vphysics settle the rest.
	// A trace that hit nothing leaves plane.normal zeroed, which is what we
	// want - that spawn point is already out in open air.
	Vector spawnPos = trace.endpos + trace.plane.normal * ( pRagdoll ? 16.0f : 4.0f );

	pEnt->SetAbsOrigin( spawnPos );
	// Yaw only - pitch/roll from the player's view would spawn the model tipped.
	pEnt->SetAbsAngles( QAngle( 0, player->EyeAngles().y, 0 ) );

	if ( DispatchSpawn( pEnt ) < 0 )
	{
		// Spawn() rejected it (CBaseProp deletes itself on a prop_data mismatch).
		return NULL;
	}

	pEnt->Activate();
	return pEnt;
}

// The full gmod_make implementation was written against a newer Source API.
// This GMod 9-compatible command keeps the public spawn-menu contract while
// using only the entity and tracing interfaces available in this engine.
static void CC_GModMakeRagdoll()
{
	CBasePlayer *player = dynamic_cast<CBasePlayer *>( UTIL_GetCommandClient() );
	if ( !player )
		return;

	const char *model = engine->Cmd_Argc() > 1
		? engine->Cmd_Argv( 1 )
		: "models/humans/group01/male_01.mdl";

	GModSpawnModelAtCrosshair( player, model, true );
}

static ConCommand gmod_makeragdoll_cmd(
	"gmod_makeragdoll",
	CC_GModMakeRagdoll,
	"Create a ragdoll at the crosshair" );

//-----------------------------------------------------------------------------
// gmod_makeprop - the command the spawn menu's plain (unprefixed) prop
// buttons send (CPropGridPanel::GetSpawnCommand(PROPITEM_PROP)). The full
// gmod_make.cpp implementation isn't in the build (written against a newer
// Source API - see the file comment at the top of this file), so without
// this, clicking a normal prop button silently did nothing.
//-----------------------------------------------------------------------------
static void CC_GModMakeProp()
{
	CBasePlayer *player = dynamic_cast<CBasePlayer *>( UTIL_GetCommandClient() );
	if ( !player )
		return;

	const char *model = engine->Cmd_Argc() > 1
		? engine->Cmd_Argv( 1 )
		: "models/props_c17/oildrum001.mdl";

	// Not forced - GModSpawnModelAtCrosshair() still routes a jointed .phy to
	// prop_ragdoll, so a character model clicked from a prop list still works.
	GModSpawnModelAtCrosshair( player, model, false );
}

static ConCommand gmod_makeprop_cmd(
	"gmod_makeprop",
	CC_GModMakeProp,
	"Create a prop at the crosshair" );
