//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Tool System - Implementation
//          Based on Garry's Mod tool system discovered via IDA analysis
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
#include "tool_dispatch.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundenvelope.h"
#include "engine/IEngineSound.h"
#include "util.h"
#include "physics.h"
#include "physics_saverestore.h"
#include "vphysics/constraints.h"
#include "vcollide_parse.h"
#include "te_effect_dispatch.h"
#include "props.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables - matching Garry's Mod
//-----------------------------------------------------------------------------
ConVar gmod_toolsound("gmod_toolsound", "1", FCVAR_ARCHIVE, "Enable tool sounds");

//-----------------------------------------------------------------------------
// Tool information table, indexed by the authentic ToolMode_t values.
// Every tool shares the same view/world model as the real GMod 9 tool gun -
// the crossbow (matches scripts/weapon_tool.txt, which is what actually
// drives the equipped model); only sound/range/delay differ meaningfully.
//-----------------------------------------------------------------------------
#define TOOLGUN_VM	"models/weapons/v_crossbow.mdl"
#define TOOLGUN_WM	"models/weapons/w_crossbow.mdl"
#define TOOLGUN_SND	"garrysmod/balloon_pop_cute.wav"
#define TOOLCON_SND	"garrysmod/constraint_sound1.wav"

ToolInfo_t CWeaponTool::s_ToolInfo[TOOL_MAX] =
{
	{ "Rope",		"Rope",			"Rope constraint",		"Click a prop, then a second prop (or world)",	TOOLGUN_VM, TOOLGUN_WM, TOOLCON_SND, 0, 256.0f, 0.5f },	// 0 TOOL_ROPE
	{ "Elastic",	"Elastic",		"Spring constraint",	"Click a prop, then a second prop",			TOOLGUN_VM, TOOLGUN_WM, TOOLCON_SND, 0, 256.0f, 0.5f },	// 1 TOOL_ELASTIC
	{ "Weld",		"Weld",			"Weld constraint",		"Click a prop, then a second prop",			TOOLGUN_VM, TOOLGUN_WM, TOOLCON_SND, 0, 256.0f, 0.5f },	// 2 TOOL_WELD
	{ "Ballsocket",	"Ball Socket",	"Ball socket constraint","Click a prop, then a second prop",			TOOLGUN_VM, TOOLGUN_WM, TOOLCON_SND, 0, 256.0f, 0.5f },	// 3 TOOL_BALLSOCKET
	{ "Pulley",		"Pulley",		"Pulley constraint",	"Click a prop, then a second prop",			TOOLGUN_VM, TOOLGUN_WM, TOOLCON_SND, 0, 256.0f, 0.5f },	// 4 TOOL_PULLEY
	{ "EasyWeld",	"EASY Weld",	"One-click weld",		"Click a prop, then a second prop (auto weld)",TOOLGUN_VM, TOOLGUN_WM, TOOLCON_SND, 0, 256.0f, 0.3f },	// 5 TOOL_EASYWELD
	{ "EasyBall",	"EASY Ball",	"One-click ballsocket",	"Click a prop, then a second prop (auto ball)",TOOLGUN_VM, TOOLGUN_WM, TOOLCON_SND, 0, 256.0f, 0.3f },	// 6 TOOL_EASYBALL
	{ "Axis",		"Axis",			"Axis constraint",		"Click a prop, then a second prop",			TOOLGUN_VM, TOOLGUN_WM, TOOLCON_SND, 0, 256.0f, 0.5f },	// 7 TOOL_AXIS
	{ "Slider",		"Slider",		"Slider constraint",	"Click a prop, then a second prop",			TOOLGUN_VM, TOOLGUN_WM, TOOLCON_SND, 0, 256.0f, 0.5f },	// 8 TOOL_SLIDER
	{ "NailGun",	"Nail gun",		"Nail constraint",		"Click a prop, then a second prop",			TOOLGUN_VM, TOOLGUN_WM, TOOLCON_SND, 0, 256.0f, 0.3f },	// 9 TOOL_NAILGUN
	{ "FacePoser",	"Face Poser",	"Pose facial flexes",	"Left click to select a target",				TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 256.0f, 0.3f },	// 10 TOOL_FACEPOSER
	{ "EyesPoser",	"Eyes Poser",	"Pose eye look target",	"Left click to select a target",				TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 256.0f, 0.3f },	// 11 TOOL_EYESPOSER
	{ "Remover",	"Remover",		"Remove props",			"Left click to remove",						TOOLGUN_VM, TOOLGUN_WM, "physics/wood/wood_crate_break1.wav", 0, 512.0f, 0.2f },	// 12 TOOL_REMOVER
	{ "Ignite",		"Ignite",		"Set props on fire",	"Left click to ignite",						TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 512.0f, 0.3f },	// 13 TOOL_IGNITE
	{ "Paint",		"Paint",		"Paint decals",			"Left click to paint",							TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 256.0f, 0.1f },	// 14 TOOL_PAINT
	{ "Duplicate",	"Duplicate",	"Copy/paste props",	"Left click to copy, right click to paste",	TOOLGUN_VM, TOOLGUN_WM, "garrysmod/save_sound1.wav", 0, 256.0f, 1.0f },	// 15 TOOL_DUPLICATE
	{ "Colour",		"Colour",		"Recolour props",		"Left click to apply colour",					TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 256.0f, 0.3f },	// 16 TOOL_COLOUR
	{ "Magnetise",	"Magnetise",	"Make props magnetic",	"Left click to toggle",						TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 256.0f, 0.3f },	// 17 TOOL_MAGNETISE
	{ "NoCollide",	"No Collide",	"Disable collision pair","Click a prop, then a second prop",			TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 256.0f, 0.3f },	// 18 TOOL_NOCOLLIDE
	{ "Dynamite",	"Dynamite",		"Attach an explosive",	"Left click to attach",						TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 512.0f, 0.5f },	// 19 TOOL_DYNAMITE
	{ "Material",	"Material",		"Change materials",	"Left click to apply material",				TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 256.0f, 0.3f },	// 20 TOOL_MATERIAL
	{ "RTCamera",	"RT Camera",	"Render target camera",	"Left click to place",						TOOLGUN_VM, TOOLGUN_WM, "NPC_CScanner.TakePhoto", 0, 1024.0f, 1.0f },	// 21 TOOL_RTCAMERA
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0.0f, 0.0f },	// 22 unused
	{ "Thruster",	"Thrusters",	"Attach a thruster",	"Left click to attach",						TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 512.0f, 0.5f },	// 23 TOOL_THRUSTER
	{ "PhysProps",	"Phys Props",	"Edit physics properties","Left click to apply",						TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 256.0f, 0.3f },	// 24 TOOL_PHYSPROPS
	{ "Statue",		"Statue",		"Freeze physics object","Left click to toggle",						TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 256.0f, 0.3f },	// 25 TOOL_STATUE
	{ "Balloon",	"Balloons",		"Attach a balloon",		"Left click to attach",						TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 512.0f, 0.5f },	// 26 TOOL_BALLOON
	{ "Emitter",	"Emitter",		"Attach a particle emitter","Left click to attach",					TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 512.0f, 0.5f },	// 27 TOOL_EMITTER
	{ "Sprite",		"Sprites",		"Attach a sprite",		"Left click to attach",						TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 512.0f, 0.5f },	// 28 TOOL_SPRITE
	{ "Wheel",		"Wheels",		"Attach a wheel axle",	"Click a prop, then a second prop",			TOOLGUN_VM, TOOLGUN_WM, TOOLCON_SND, 0, 256.0f, 0.5f },	// 29 TOOL_WHEEL
	{ "Gun",		"Gun Tool",		"Shoot at things",		"Left click to shoot",							TOOLGUN_VM, TOOLGUN_WM, "Weapon_Pistol.Single", 0, 2048.0f, 0.2f },	// 30 TOOL_GUN
	{ "Camera",		"Camera Tool",	"Take screenshots",	"Left click to take photo",						TOOLGUN_VM, TOOLGUN_WM, "NPC_CScanner.TakePhoto", 0, 1024.0f, 1.0f },	// 31 TOOL_CAMERA
	{ "NPCSpawn",	"NPC Tool",		"Spawn NPCs",			"Left click to spawn NPC",						TOOLGUN_VM, TOOLGUN_WM, "physics/wood/wood_crate_break5.wav", 0, 512.0f, 0.5f },	// 32 TOOL_NPCSPAWN
	{ "Inflator",	"Inflator",		"Resize props",			"Left click to grow, right click to shrink",	TOOLGUN_VM, TOOLGUN_WM, TOOLGUN_SND, 0, 256.0f, 0.3f },	// 33 TOOL_INFLATOR
};

//-----------------------------------------------------------------------------
// Network table
//-----------------------------------------------------------------------------
// Bisecting a map-load crash in the engine's SendTable precalc (dt.cpp
// SetupArrayProps_R/SetDataTableProxyIndices_R) that appeared when this
// ServerClass was wired into the build for the first time this session -
// temporarily emptied to isolate whether this table (vs. gmod_balloon.cpp's,
// also new this session) is the trigger. See runtime-spawn-precache-crash.md
// / MEMORY.md for context; restore custom props once root-caused.
IMPLEMENT_SERVERCLASS_ST( CWeaponTool, DT_WeaponTool )
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Data description
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CWeaponTool )
	DEFINE_FIELD( CWeaponTool, m_nToolMode, FIELD_INTEGER ),
	DEFINE_FIELD( CWeaponTool, m_bToolActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( CWeaponTool, m_flNextToolTime, FIELD_TIME ),
	DEFINE_FIELD( CWeaponTool, m_flLastUseTime, FIELD_TIME ),
	DEFINE_FIELD( CWeaponTool, m_flToolDelay, FIELD_FLOAT ),
	DEFINE_FIELD( CWeaponTool, m_bInUse, FIELD_BOOLEAN ),
	DEFINE_SOUNDPATCH( CWeaponTool, m_pToolSound ),
	DEFINE_FIELD( CWeaponTool, m_bSoundStarted, FIELD_BOOLEAN ),
	DEFINE_FIELD( CWeaponTool, m_hLastTarget, FIELD_EHANDLE ),
	DEFINE_FIELD( CWeaponTool, m_vecLastTargetPos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CWeaponTool, m_hPendingEntity, FIELD_EHANDLE ),
	DEFINE_FIELD( CWeaponTool, m_bPendingWorld, FIELD_BOOLEAN ),
	DEFINE_FIELD( CWeaponTool, m_vecPendingPos, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( CWeaponTool, m_nPendingPhysBone, FIELD_INTEGER ),
	DEFINE_FIELD( CWeaponTool, m_flPendingTime, FIELD_TIME ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Weapon registration
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS( weapon_tool, CWeaponTool );
PRECACHE_WEAPON_REGISTER( weapon_tool );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponTool::CWeaponTool()
{
	m_nToolMode = TOOL_NONE;
	m_bToolActive = false;
	m_flNextToolTime = 0.0f;
	m_flLastUseTime = 0.0f;
	m_flToolDelay = 0.0f;
	m_bInUse = false;
	m_pToolSound = NULL;
	m_bSoundStarted = false;
	m_hLastTarget = NULL;
	m_vecLastTargetPos = vec3_origin;
	m_hPendingEntity = NULL;
	m_bPendingWorld = false;
	m_vecPendingPos = vec3_origin;
	m_nPendingPhysBone = -1;
	m_flPendingTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CWeaponTool::~CWeaponTool()
{
	StopToolSound();
}

//-----------------------------------------------------------------------------
// Precache
//-----------------------------------------------------------------------------
void CWeaponTool::Precache()
{
	BaseClass::Precache();

	// Precache all tool sounds and models
	for ( int i = 0; i < TOOL_MAX; i++ )
	{
		if ( s_ToolInfo[i].pszSound )
		{
			PrecacheScriptSound( s_ToolInfo[i].pszSound );
		}
		if ( s_ToolInfo[i].pszViewModel )
		{
			PrecacheModel( s_ToolInfo[i].pszViewModel );
		}
		if ( s_ToolInfo[i].pszWorldModel )
		{
			PrecacheModel( s_ToolInfo[i].pszWorldModel );
		}
	}

	// Precache effects
	PrecacheScriptSound( "garrysmod/balloon_pop_cute.wav" );
	PrecacheScriptSound( "garrysmod/save_sound1.wav" );
	PrecacheScriptSound( "garrysmod/constraint_sound1.wav" );

	// Rope material used by the constraint tools' visual ropes
	// (tool_constraints.cpp) - CRopeKeyframe::SetMaterial does a late
	// PrecacheModel otherwise.
	PrecacheModel( "cable/cable.vmt" );
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CWeaponTool::Spawn()
{
	BaseClass::Spawn();

	// Real GMod9 default starting tool is Weld. gm_toolmode (gmod_tools.cpp)
	// sets the real mode directly on this instance once the player picks one
	// from the spawn menu; this is just the pre-selection default.
	if ( m_nToolMode == TOOL_NONE )
	{
		SetToolMode( TOOL_WELD );
	}

	// Set weapon properties
	m_fMinRange1 = 0.0f;
	m_fMaxRange1 = GetRange();
	m_fMinRange2 = 0.0f;
	m_fMaxRange2 = GetRange();
}

//-----------------------------------------------------------------------------
// Deploy
//-----------------------------------------------------------------------------
bool CWeaponTool::Deploy()
{
	// Every tool mode shares the same view/world model (see s_ToolInfo) - the
	// model comes from the weapon script, no per-mode model switch is needed.
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Holster
//-----------------------------------------------------------------------------
bool CWeaponTool::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	StopToolSound();
	m_bToolActive = false;
	m_bInUse = false;

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Drop
//-----------------------------------------------------------------------------
void CWeaponTool::Drop( const Vector &vecVelocity )
{
	StopToolSound();
	m_bToolActive = false;
	m_bInUse = false;

	BaseClass::Drop( vecVelocity );
}

//-----------------------------------------------------------------------------
// Item pre-frame
//-----------------------------------------------------------------------------
void CWeaponTool::ItemPreFrame()
{
	BaseClass::ItemPreFrame();

	// Update tool effects
	UpdateToolEffects();

	// Call tool-specific think
	OnToolThink();
}

//-----------------------------------------------------------------------------
// Item post-frame
//-----------------------------------------------------------------------------
void CWeaponTool::ItemPostFrame()
{
	// BaseClass::ItemPostFrame() already checks IN_ATTACK/IN_ATTACK2 and calls
	// PrimaryAttack()/SecondaryAttack() - no extra input handling needed here.
	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Primary attack
//-----------------------------------------------------------------------------
void CWeaponTool::PrimaryAttack()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	// Check tool delay
	if ( m_flNextToolTime > gpGlobals->curtime )
		return;

	// Get trace
	trace_t tr;
	GetToolTrace( tr );

	// Perform tool action
	DoToolAction( tr, true );

	// Set next use time
	m_flNextToolTime = gpGlobals->curtime + GetDelay();
	m_flLastUseTime = gpGlobals->curtime;

	// Play animation
	SendWeaponAnim( GetToolActivity( m_nToolMode, true ) );

	// Set next idle time
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
}

//-----------------------------------------------------------------------------
// Secondary attack
//-----------------------------------------------------------------------------
void CWeaponTool::SecondaryAttack()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	// Check tool delay
	if ( m_flNextToolTime > gpGlobals->curtime )
		return;

	// Get trace
	trace_t tr;
	GetToolTrace( tr );

	// Perform tool action
	DoToolAction( tr, false );

	// Set next use time
	m_flNextToolTime = gpGlobals->curtime + GetDelay();
	m_flLastUseTime = gpGlobals->curtime;

	// Play animation
	SendWeaponAnim( GetToolActivity( m_nToolMode, false ) );

	// Set next idle time
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
}

//-----------------------------------------------------------------------------
// Weapon idle
//-----------------------------------------------------------------------------
void CWeaponTool::WeaponIdle()
{
	// Stop tool sound if not in use
	if ( !m_bInUse )
	{
		StopToolSound();
	}

	BaseClass::WeaponIdle();
}

//-----------------------------------------------------------------------------
// Set tool mode
//-----------------------------------------------------------------------------
void CWeaponTool::SetToolMode( int nMode )
{
	if ( nMode < 0 || nMode >= TOOL_MAX || !s_ToolInfo[nMode].pszName )
		nMode = TOOL_WELD;

	// Changing mode clears any pending "first prop selected" state from
	// whatever the previous tool was - a half-finished rope shouldn't leak
	// into the newly selected tool.
	ClearPendingSelection();

	m_nToolMode = nMode;

	DevMsg( "Tool mode changed to: %d (%s)\n", nMode, GetToolName( nMode ) );
}

//-----------------------------------------------------------------------------
// Perform tool action
//-----------------------------------------------------------------------------
void CWeaponTool::DoToolAction( trace_t &tr, bool bPrimary )
{
	m_bToolActive = true;
	m_bInUse = true;

	// Start tool sound
	StartToolSound();

	// Create visual effects
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		Vector vecStart = pOwner->EyePosition();
		CreateToolEffect( vecStart, tr.endpos );
	}

	// Call tool-specific implementation
	if ( tr.m_pEnt && CanUseOnEntity( tr.m_pEnt ) )
	{
		OnToolUse( tr.m_pEnt, tr, bPrimary );
	}
	else
	{
		OnToolTrace( tr, bPrimary );
	}

	// Create spark effect on hit
	if ( tr.fraction < 1.0f )
	{
		CreateSparkEffect( tr.endpos );
	}
}

//-----------------------------------------------------------------------------
// Check if we can use tool on entity
//-----------------------------------------------------------------------------
bool CWeaponTool::CanUseOnEntity( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return false;

	// Don't use on players (unless specific tool allows it)
	if ( pEntity->IsPlayer() )
		return false;

	// Don't use on world
	if ( pEntity->entindex() == 0 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Start tool sound
//-----------------------------------------------------------------------------
void CWeaponTool::StartToolSound()
{
	if ( !gmod_toolsound.GetBool() )
		return;

	const ToolInfo_t *pToolInfo = GetToolInfo( m_nToolMode );
	if ( !pToolInfo || !pToolInfo->pszSound )
		return;

	if ( m_pToolSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pToolSound );
		m_pToolSound = NULL;
	}

	CPASAttenuationFilter filter( this );

	m_pToolSound = CSoundEnvelopeController::GetController().SoundCreate(
		filter, entindex(), pToolInfo->pszSound );

	if ( m_pToolSound )
	{
		CSoundEnvelopeController::GetController().Play( m_pToolSound, 1.0f, 100.0f );
		m_bSoundStarted = true;
	}
}

//-----------------------------------------------------------------------------
// Stop tool sound
//-----------------------------------------------------------------------------
void CWeaponTool::StopToolSound()
{
	if ( m_pToolSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pToolSound );
		m_pToolSound = NULL;
		m_bSoundStarted = false;
	}
}

//-----------------------------------------------------------------------------
// Get tool range
//-----------------------------------------------------------------------------
float CWeaponTool::GetRange() const
{
	const ToolInfo_t *pToolInfo = GetToolInfo( m_nToolMode );
	return pToolInfo ? pToolInfo->flRange : 256.0f;
}

//-----------------------------------------------------------------------------
// Get tool delay
//-----------------------------------------------------------------------------
float CWeaponTool::GetDelay() const
{
	const ToolInfo_t *pToolInfo = GetToolInfo( m_nToolMode );
	return pToolInfo ? pToolInfo->flDelay : 0.5f;
}

//-----------------------------------------------------------------------------
// Check if target is valid
//-----------------------------------------------------------------------------
bool CWeaponTool::IsValidTarget( CBaseEntity *pEntity, trace_t &tr )
{
	if ( !pEntity )
		return false;

	// Check distance
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		float flDist = pOwner->EyePosition().DistTo( tr.endpos );
		if ( flDist > GetRange() )
			return false;
	}

	return CanUseOnEntity( pEntity );
}

//-----------------------------------------------------------------------------
// Get tool trace
//-----------------------------------------------------------------------------
void CWeaponTool::GetToolTrace( trace_t &tr )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	Vector vecStart = pOwner->EyePosition();
	Vector vecForward;
	pOwner->EyeVectors( &vecForward );
	Vector vecEnd = vecStart + (vecForward * GetRange());

	UTIL_TraceLine( vecStart, vecEnd, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );
}

//-----------------------------------------------------------------------------
// Update tool effects
//-----------------------------------------------------------------------------
void CWeaponTool::UpdateToolEffects()
{
	// Stop sound if not in use
	if ( m_bSoundStarted && !m_bInUse && gpGlobals->curtime > m_flLastUseTime + 0.1f )
	{
		StopToolSound();
		m_bToolActive = false;
	}

	// Reset in-use flag
	m_bInUse = false;
}

//-----------------------------------------------------------------------------
// Create tool effect
//-----------------------------------------------------------------------------
void CWeaponTool::CreateToolEffect( const Vector &vecStart, const Vector &vecEnd )
{
	// Create beam effect (this would be implemented with proper effects in a full game)
	DevMsg( "Tool effect from (%f,%f,%f) to (%f,%f,%f)\n",
		vecStart.x, vecStart.y, vecStart.z,
		vecEnd.x, vecEnd.y, vecEnd.z );
}

//-----------------------------------------------------------------------------
// Create spark effect
//-----------------------------------------------------------------------------
void CWeaponTool::CreateSparkEffect( const Vector &vecPos )
{
	// Create spark effect (this would be implemented with proper effects in a full game)
	DevMsg( "Spark effect at (%f,%f,%f)\n", vecPos.x, vecPos.y, vecPos.z );
}

//-----------------------------------------------------------------------------
// Play tool sound
//-----------------------------------------------------------------------------
void CWeaponTool::PlayToolSound( const char *pszSound )
{
	if ( !gmod_toolsound.GetBool() )
		return;

	EmitSound( pszSound );
}

//-----------------------------------------------------------------------------
// Get tool activity
//-----------------------------------------------------------------------------
Activity CWeaponTool::GetToolActivity( int nMode, bool bPrimary )
{
	// Return appropriate activity for tool mode
	return bPrimary ? ACT_VM_PRIMARYATTACK : ACT_VM_SECONDARYATTACK;
}

//-----------------------------------------------------------------------------
// Per-tool dispatch. CWeaponTool is the only class ever instantiated for the
// "weapon_tool" entity - see tool_dispatch.h for why this is a switch instead
// of virtual overrides in a subclass.
//-----------------------------------------------------------------------------
void CWeaponTool::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )
{
	switch ( m_nToolMode )
	{
		case TOOL_ROPE:
		case TOOL_ELASTIC:
		case TOOL_WELD:
		case TOOL_BALLSOCKET:
		case TOOL_PULLEY:
		case TOOL_EASYWELD:
		case TOOL_EASYBALL:
		case TOOL_AXIS:
		case TOOL_SLIDER:
		case TOOL_NAILGUN:
			Tool_Constraint_OnUse( this, m_nToolMode, pEntity, tr, bPrimary );
			break;

		case TOOL_FACEPOSER:
		case TOOL_EYESPOSER:
			Tool_Poser_OnUse( this, m_nToolMode, pEntity, tr, bPrimary );
			break;

		case TOOL_REMOVER:
			Tool_Remover_OnUse( this, pEntity, tr, bPrimary );
			break;

		case TOOL_IGNITE:
		case TOOL_MAGNETISE:
		case TOOL_NOCOLLIDE:
		case TOOL_DYNAMITE:
		case TOOL_STATUE:
			Tool_Simple_OnUse( this, m_nToolMode, pEntity, tr, bPrimary );
			break;

		case TOOL_PAINT:
			Tool_Paint_OnUse( this, pEntity, tr, bPrimary );
			break;

		case TOOL_DUPLICATE:
			Tool_Duplicator_OnUse( this, pEntity, tr, bPrimary );
			break;

		case TOOL_COLOUR:
			Tool_Color_OnUse( this, pEntity, tr, bPrimary );
			break;

		case TOOL_MATERIAL:
			Tool_Material_OnUse( this, pEntity, tr, bPrimary );
			break;

		case TOOL_RTCAMERA:
		case TOOL_THRUSTER:
		case TOOL_PHYSPROPS:
		case TOOL_BALLOON:
		case TOOL_EMITTER:
		case TOOL_SPRITE:
		case TOOL_WHEEL:
			Tool_Attach_OnUse( this, m_nToolMode, pEntity, tr, bPrimary );
			break;

		case TOOL_GUN:
			Tool_Gun_OnUse( this, pEntity, tr, bPrimary );
			break;

		case TOOL_CAMERA:
			Tool_Camera_OnUse( this, pEntity, tr, bPrimary );
			break;

		case TOOL_NPCSPAWN:
			Tool_NPC_OnUse( this, pEntity, tr, bPrimary );
			break;

		case TOOL_INFLATOR:
			Tool_Inflator_OnUse( this, pEntity, tr, bPrimary );
			break;

		default:
			break;
	}
}

void CWeaponTool::OnToolTrace( trace_t &tr, bool bPrimary )
{
	switch ( m_nToolMode )
	{
		case TOOL_ROPE:
		case TOOL_ELASTIC:
		case TOOL_WELD:
		case TOOL_BALLSOCKET:
		case TOOL_PULLEY:
		case TOOL_EASYWELD:
		case TOOL_EASYBALL:
		case TOOL_AXIS:
		case TOOL_SLIDER:
		case TOOL_NAILGUN:
			Tool_Constraint_OnTrace( this, m_nToolMode, tr, bPrimary );
			break;

		case TOOL_FACEPOSER:
		case TOOL_EYESPOSER:
			Tool_Poser_OnTrace( this, m_nToolMode, tr, bPrimary );
			break;

		case TOOL_REMOVER:
			Tool_Remover_OnTrace( this, tr, bPrimary );
			break;

		case TOOL_IGNITE:
		case TOOL_MAGNETISE:
		case TOOL_NOCOLLIDE:
		case TOOL_DYNAMITE:
		case TOOL_STATUE:
			Tool_Simple_OnTrace( this, m_nToolMode, tr, bPrimary );
			break;

		case TOOL_PAINT:
			Tool_Paint_OnTrace( this, tr, bPrimary );
			break;

		case TOOL_DUPLICATE:
			Tool_Duplicator_OnTrace( this, tr, bPrimary );
			break;

		case TOOL_COLOUR:
			Tool_Color_OnTrace( this, tr, bPrimary );
			break;

		case TOOL_MATERIAL:
			Tool_Material_OnTrace( this, tr, bPrimary );
			break;

		case TOOL_RTCAMERA:
		case TOOL_THRUSTER:
		case TOOL_PHYSPROPS:
		case TOOL_BALLOON:
		case TOOL_EMITTER:
		case TOOL_SPRITE:
		case TOOL_WHEEL:
			Tool_Attach_OnTrace( this, m_nToolMode, tr, bPrimary );
			break;

		case TOOL_GUN:
			Tool_Gun_OnTrace( this, tr, bPrimary );
			break;

		case TOOL_CAMERA:
			Tool_Camera_OnTrace( this, tr, bPrimary );
			break;

		case TOOL_NPCSPAWN:
			Tool_NPC_OnTrace( this, tr, bPrimary );
			break;

		case TOOL_INFLATOR:
			Tool_Inflator_OnTrace( this, tr, bPrimary );
			break;

		default:
			break;
	}
}

void CWeaponTool::OnToolThink()
{
	switch ( m_nToolMode )
	{
		case TOOL_ROPE:
		case TOOL_ELASTIC:
		case TOOL_WELD:
		case TOOL_BALLSOCKET:
		case TOOL_PULLEY:
		case TOOL_EASYWELD:
		case TOOL_EASYBALL:
		case TOOL_AXIS:
		case TOOL_SLIDER:
		case TOOL_NAILGUN:
			Tool_Constraint_OnThink( this, m_nToolMode );
			break;

		case TOOL_FACEPOSER:
		case TOOL_EYESPOSER:
			Tool_Poser_OnThink( this, m_nToolMode );
			break;

		case TOOL_REMOVER:
			Tool_Remover_OnThink( this );
			break;

		case TOOL_IGNITE:
		case TOOL_MAGNETISE:
		case TOOL_NOCOLLIDE:
		case TOOL_DYNAMITE:
		case TOOL_STATUE:
			Tool_Simple_OnThink( this, m_nToolMode );
			break;

		case TOOL_PAINT:
			Tool_Paint_OnThink( this );
			break;

		case TOOL_DUPLICATE:
			Tool_Duplicator_OnThink( this );
			break;

		case TOOL_COLOUR:
			Tool_Color_OnThink( this );
			break;

		case TOOL_MATERIAL:
			Tool_Material_OnThink( this );
			break;

		case TOOL_RTCAMERA:
		case TOOL_THRUSTER:
		case TOOL_PHYSPROPS:
		case TOOL_BALLOON:
		case TOOL_EMITTER:
		case TOOL_SPRITE:
		case TOOL_WHEEL:
			Tool_Attach_OnThink( this, m_nToolMode );
			break;

		case TOOL_GUN:
			Tool_Gun_OnThink( this );
			break;

		case TOOL_CAMERA:
			Tool_Camera_OnThink( this );
			break;

		case TOOL_NPCSPAWN:
			Tool_NPC_OnThink( this );
			break;

		case TOOL_INFLATOR:
			Tool_Inflator_OnThink( this );
			break;

		default:
			break;
	}
}

//-----------------------------------------------------------------------------
// Check tool constraints
//-----------------------------------------------------------------------------
bool CWeaponTool::CheckToolConstraints()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return false;

	// Check if in vehicle
	const ToolInfo_t *pToolInfo = GetToolInfo( m_nToolMode );
	if ( pToolInfo && (pToolInfo->nFlags & TOOL_FLAG_DISABLE_IN_VEHICLE) )
	{
		if ( pOwner->IsInAVehicle() )
			return false;
	}

	// Check if underwater
	if ( pToolInfo && !(pToolInfo->nFlags & TOOL_FLAG_ALLOW_UNDERWATER) )
	{
		if ( pOwner->GetWaterLevel() >= 3 )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Apply tool force
//-----------------------------------------------------------------------------
void CWeaponTool::ApplyToolForce( CBaseEntity *pEntity, const Vector &vecForce )
{
	if ( !pEntity )
		return;

	IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
	if ( pPhysics )
	{
		pPhysics->ApplyForceCenter( vecForce );
	}
}

//-----------------------------------------------------------------------------
// Create tool constraint
//-----------------------------------------------------------------------------
void CWeaponTool::CreateToolConstraint( CBaseEntity *pEnt1, CBaseEntity *pEnt2, int nType )
{
	// This would implement constraint creation (complex physics system)
	DevMsg( "Creating constraint type %d between entities %d and %d\n",
		nType, pEnt1->entindex(), pEnt2 ? pEnt2->entindex() : 0 );
}

//-----------------------------------------------------------------------------
// Global tool functions
//-----------------------------------------------------------------------------
const ToolInfo_t *GetToolInfo( int nMode )
{
	if ( nMode < 0 || nMode >= TOOL_MAX )
		return NULL;

	return &CWeaponTool::s_ToolInfo[nMode];
}

const char *GetToolName( int nMode )
{
	const ToolInfo_t *pInfo = GetToolInfo( nMode );
	return pInfo ? pInfo->pszName : "Unknown";
}

const char *GetToolDescription( int nMode )
{
	const ToolInfo_t *pInfo = GetToolInfo( nMode );
	return pInfo ? pInfo->pszDescription : "Unknown tool";
}

//-----------------------------------------------------------------------------
// Console commands - matching Garry's Mod commands from IDA analysis
//-----------------------------------------------------------------------------
// NOTE: the primary entry point for changing tool mode is gm_toolmode
// (gmod_tools.cpp CMD_gm_toolmode), which is what the spawn menu UI sends and
// which also equips weapon_tool if the player doesn't have it out. This
// command is kept only as a convenient console alias that does the same
// thing directly on the invoking player's weapon_tool instance - there is
// deliberately no "gmod_toolmode" ConVar anymore (a global ConVar can't hold a
// different value per connected player, and a ConVar/ConCommand pair sharing
// one name is itself an engine registration collision).
CON_COMMAND( gmod_toolmode, "Sets the current tool mode on your equipped tool gun" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	if ( engine->Cmd_Argc() < 2 )
	{
		CBaseCombatWeapon *pWeapon = pPlayer->Weapon_OwnsThisType( "weapon_tool" );
		CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pWeapon );
		int nCurrent = pTool ? pTool->GetToolMode() : TOOL_NONE;

		Msg( "Current tool mode: %d (%s)\n", nCurrent, GetToolName( nCurrent ) );
		Msg( "Usage: gmod_toolmode <mode>\n" );
		Msg( "Available modes:\n" );
		for ( int i = 0; i < TOOL_MAX; i++ )
		{
			const ToolInfo_t *pInfo = GetToolInfo( i );
			if ( pInfo && pInfo->pszName )
			{
				Msg( "  %d - %s\n", i, pInfo->pszName );
			}
		}
		return;
	}

	int nMode = atoi( engine->Cmd_Argv(1) );

	CBaseCombatWeapon *pWeapon = pPlayer->Weapon_OwnsThisType( "weapon_tool" );
	if ( !pWeapon )
	{
		// Runtime weapon creation needs the precache window open, otherwise
		// Spawn()'s Precache() fails and the entity is discarded.
		const bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
		CBaseEntity::SetAllowPrecache( true );

		pPlayer->GiveNamedItem( "weapon_tool" );
		pWeapon = pPlayer->Weapon_OwnsThisType( "weapon_tool" );

		CBaseEntity::SetAllowPrecache( bAllowPrecache );
	}

	CWeaponTool *pTool = dynamic_cast<CWeaponTool*>( pWeapon );
	if ( !pTool )
	{
		Msg( "Could not equip weapon_tool\n" );
		return;
	}

	pTool->SetToolMode( nMode );
	if ( pPlayer->GetActiveWeapon() != pTool )
	{
		pPlayer->Weapon_Switch( pTool );
	}

	Msg( "Tool mode set to: %d (%s)\n", pTool->GetToolMode(), GetToolName( pTool->GetToolMode() ) );
}

CON_COMMAND( gmod_toolweapon, "Switches to tool weapon" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Give player the tool weapon (precache window has to be open for a runtime
	// weapon spawn, see gmod_toolmode above)
	const bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );
	pPlayer->GiveNamedItem( "weapon_tool" );
	CBaseEntity::SetAllowPrecache( bAllowPrecache );

	// Switch to it
	CBaseCombatWeapon *pWeapon = pPlayer->Weapon_OwnsThisType( "weapon_tool" );
	if ( pWeapon )
	{
		pPlayer->Weapon_Switch( pWeapon );
		Msg( "Switched to tool weapon\n" );
	}
}