#include "cbase.h"
#include "player.h"
#include "keyvalues.h"
#include "engine/ivmodelinfo.h"

#include "tier0/memdbgon.h"

extern IVModelInfo *modelinfo;

// CBaseProp::Spawn() (dlls/props.cpp) deletes a "prop_physics" outright if its
// model has no "prop_data" keyvalues section (real props require it), and
// separately deletes anything OTHER than prop_physics/physics_prop/
// prop_physics_override/prop_dynamic_override if the model DOES have one.
// Spawning everything as prop_physics unconditionally meant any model
// without prop_data (e.g. models/props_canal/boat001a.mdl) got silently
// deleted right after spawn. Pick the entity class the same way the real
// spawn menu does: only use prop_physics when the model actually has
// prop_data, otherwise fall back to prop_dynamic.
static const char *PickPropClassForModel( const char *model, int modelindex )
{
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

	CBaseEntity *ragdoll = CreateEntityByName( "prop_ragdoll" );
	if ( !ragdoll )
	{
		ClientPrint( player, HUD_PRINTTALK, "Unable to create ragdoll" );
		return;
	}

	// This runs long after the map's own precache phase, so SetModel() (via
	// UTIL_SetModel -> modelinfo->GetModelIndex) would otherwise hit an
	// unprecached model and call Error("no precache: ...") - a fatal message
	// box + crash. Precache it first (lifting the same "too late" assert gate
	// used by npc_create, dlls/ai_concommands.cpp) before setting the model.
	bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );
	engine->PrecacheModel( model );
	CBaseEntity::SetAllowPrecache( bAllowPrecache );

	ragdoll->SetModel( model );
	ragdoll->SetAbsOrigin( trace.endpos );
	ragdoll->SetAbsAngles( player->EyeAngles() );
	DispatchSpawn( ragdoll );
	ragdoll->Activate();
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

	// See CC_GModMakeRagdoll() above - same "too late" precache gate needed.
	bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );
	int modelindex = engine->PrecacheModel( model );
	CBaseEntity::SetAllowPrecache( bAllowPrecache );

	CBaseEntity *prop = CreateEntityByName( PickPropClassForModel( model, modelindex ) );
	if ( !prop )
	{
		ClientPrint( player, HUD_PRINTTALK, "Unable to create prop" );
		return;
	}

	prop->SetModel( model );
	prop->SetAbsOrigin( trace.endpos );
	prop->SetAbsAngles( player->EyeAngles() );
	DispatchSpawn( prop );
	prop->Activate();
}

static ConCommand gmod_makeprop_cmd(
	"gmod_makeprop",
	CC_GModMakeProp,
	"Create a prop at the crosshair" );
