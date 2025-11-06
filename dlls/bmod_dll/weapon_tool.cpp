//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Tool System - Implementation
//          Based on Garry's Mod tool system discovered via IDA analysis
//
//=============================================================================

#include "cbase.h"
#include "weapon_tool.h"
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
#include "bone_setup.h"
#include "studio.h"
#include "game_shared/effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "props.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Console variables - matching Garry's Mod
//-----------------------------------------------------------------------------
ConVar bm_toolmode("bm_toolmode", "2", FCVAR_USERINFO | FCVAR_ARCHIVE, "Current tool mode");
ConVar bm_toolsound("bm_toolsound", "1", FCVAR_ARCHIVE, "Enable tool sounds");

//-----------------------------------------------------------------------------
// Tool information table - based on IDA analysis of Garry's Mod tools
//-----------------------------------------------------------------------------
ToolInfo_t CWeaponTool::s_ToolInfo[TOOL_MAX] =
{
	// Tool Mode			Name			Description				HelpText						ViewModel				WorldModel				Sound					Flags				Range		Delay
	{ NULL,					NULL,			NULL,					NULL,							NULL,					NULL,					NULL,					0,					0.0f,		0.0f },	// TOOL_NONE
	{ "Gun",				"Gun Tool",		"Shoot at things",		"Left click to shoot",			"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "Weapon_Pistol.Single",		0,					2048.0f,	0.2f },	// TOOL_GUN
	{ "Physgun",			"Physics Gun",	"Move objects",			"Use weapon_physcannon instead", NULL,					NULL,					NULL,					0,					0.0f,		0.0f },	// TOOL_PHYSGUN (handled by separate weapon)
	{ "Camera",				"Camera Tool",	"Take screenshots",		"Left click to take photo",		"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "NPC_CScanner.TakePhoto",	0,					1024.0f,	1.0f },	// TOOL_CAMERA
	{ "NPC",				"NPC Tool",		"Spawn NPCs",			"Left click to spawn NPC",		"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "physics/wood/wood_crate_break5.wav", 0,		512.0f,		0.5f },	// TOOL_NPC
	{ "NextBot",			"NextBot Tool",	"Spawn NextBots",		"Left click to spawn NextBot",	"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "physics/wood/wood_crate_break5.wav", 0,		512.0f,		0.5f },	// TOOL_NEXTBOT
	{ "Material",			"Material Tool","Change materials",		"Left click to apply material",	"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/balloon_pop_cute.wav", 0,		256.0f,		0.3f },	// TOOL_MATERIAL
	{ "Color",				"Color Tool",	"Change colors",		"Left click to apply color",	"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/balloon_pop_cute.wav", 0,		256.0f,		0.3f },	// TOOL_COLOR
	{ "Paint",				"Paint Tool",	"Paint surfaces",		"Left click to paint",			"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/balloon_pop_cute.wav", 0,		256.0f,		0.1f },	// TOOL_PAINT
	{ "Inflator",			"Inflator Tool","Resize objects",		"Left click to resize",			"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/balloon_pop_cute.wav", 0,		256.0f,		0.3f },	// TOOL_INFLATOR
	{ "Duplicator",			"Duplicator",	"Duplicate objects",	"Left click to duplicate",		"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/save_sound1.wav", 0,			256.0f,		1.0f },	// TOOL_DUPLICATOR
	{ "Constrainer",		"Constrainer",	"Create constraints",	"Left click to constrain",		"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/constraint_sound1.wav", 0,	256.0f,		0.5f },	// TOOL_CONSTRAINER
	{ "Axis",				"Axis",			"Axis constraint",		"Left click to create axis",	"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/constraint_sound1.wav", 0,	256.0f,		0.5f },	// TOOL_AXIS
	{ "BallSocket",			"Ball Socket",	"Ball socket constraint","Left click to create ball socket","models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/constraint_sound1.wav", 0,	256.0f,		0.5f },	// TOOL_BALLSOCKET
	{ "Rope",				"Rope",			"Rope constraint",		"Left click to create rope",	"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/constraint_sound1.wav", 0,	256.0f,		0.5f },	// TOOL_ROPE
	{ "Pulley",				"Pulley",		"Pulley constraint",	"Left click to create pulley",	"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/constraint_sound1.wav", 0,	256.0f,		0.5f },	// TOOL_PULLEY
	{ "Motor",				"Motor",		"Motor constraint",		"Left click to create motor",	"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/constraint_sound1.wav", 0,	256.0f,		0.5f },	// TOOL_MOTOR
	{ "Hydraulic",			"Hydraulic",	"Hydraulic constraint",	"Left click to create hydraulic","models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/constraint_sound1.wav", 0,	256.0f,		0.5f },	// TOOL_HYDRAULIC
	{ "Pneumatic",			"Pneumatic",	"Pneumatic constraint",	"Left click to create pneumatic","models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/constraint_sound1.wav", 0,	256.0f,		0.5f },	// TOOL_PNEUMATIC
	{ "Spring",				"Spring",		"Spring constraint",	"Left click to create spring",	"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/constraint_sound1.wav", 0,	256.0f,		0.5f },	// TOOL_SPRING
	{ "KeepUpright",		"Keep Upright",	"Keep upright constraint","Left click to keep upright",	"models/weapons/v_pistol.mdl", "models/weapons/w_pistol.mdl", "garrysmod/constraint_sound1.wav", 0,	256.0f,		0.5f },	// TOOL_KEEPUPRIGHT
};

//-----------------------------------------------------------------------------
// Network table
//-----------------------------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST( CWeaponTool, DT_WeaponTool )
	SendPropInt( SENDINFO(m_nToolMode), 6, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO(m_bToolActive) ),
	SendPropFloat( SENDINFO(m_flNextToolTime) ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Data description
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CWeaponTool )
	DEFINE_FIELD( m_nToolMode, FIELD_INTEGER ),
	DEFINE_FIELD( m_bToolActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flNextToolTime, FIELD_TIME ),
	DEFINE_FIELD( m_flLastUseTime, FIELD_TIME ),
	DEFINE_FIELD( m_flToolDelay, FIELD_FLOAT ),
	DEFINE_FIELD( m_bInUse, FIELD_BOOLEAN ),
	DEFINE_SOUNDPATCH( m_pToolSound ),
	DEFINE_FIELD( m_bSoundStarted, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hLastTarget, FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecLastTargetPos, FIELD_POSITION_VECTOR ),
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
	m_nToolMode = TOOL_GUN;
	m_bToolActive = false;
	m_flNextToolTime = 0.0f;
	m_flLastUseTime = 0.0f;
	m_flToolDelay = 0.0f;
	m_bInUse = false;
	m_pToolSound = NULL;
	m_bSoundStarted = false;
	m_hLastTarget = NULL;
	m_vecLastTargetPos = vec3_origin;
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
	for ( int i = 1; i < TOOL_MAX; i++ )
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
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CWeaponTool::Spawn()
{
	BaseClass::Spawn();

	// Set default tool mode from ConVar
	SetToolMode( bm_toolmode.GetInt() );

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
	// Update tool mode from ConVar
	SetToolMode( bm_toolmode.GetInt() );

	// Set view/world models based on current tool
	const ToolInfo_t *pToolInfo = GetToolInfo( m_nToolMode );
	if ( pToolInfo && pToolInfo->pszViewModel )
	{
		SetViewModel( pToolInfo->pszViewModel );
	}
	if ( pToolInfo && pToolInfo->pszWorldModel )
	{
		SetWorldModel( pToolInfo->pszWorldModel );
	}

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

	// Check for tool mode changes
	HandleToolModeSwitch();

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
	BaseClass::ItemPostFrame();

	// Handle input
	CheckToolInput();
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
	if ( nMode < 0 || nMode >= TOOL_MAX )
		nMode = TOOL_GUN;

	if ( nMode == TOOL_PHYSGUN )
	{
		// Physgun is handled by weapon_physcannon
		DevMsg( "Tool mode %d (Physgun) should use weapon_physcannon instead\n", nMode );
		return;
	}

	m_nToolMode = nMode;

	// Update ConVar
	bm_toolmode.SetValue( nMode );

	// Update models
	const ToolInfo_t *pToolInfo = GetToolInfo( m_nToolMode );
	if ( pToolInfo )
	{
		if ( pToolInfo->pszViewModel )
			SetViewModel( pToolInfo->pszViewModel );
		if ( pToolInfo->pszWorldModel )
			SetWorldModel( pToolInfo->pszWorldModel );
	}

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
	if ( !bm_toolsound.GetBool() )
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
	if ( !bm_toolsound.GetBool() )
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
// Check tool input
//-----------------------------------------------------------------------------
void CWeaponTool::CheckToolInput()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	// Check for primary attack
	if ( pOwner->m_nButtons & IN_ATTACK )
	{
		if ( CanPrimaryAttack() )
		{
			PrimaryAttack();
		}
	}

	// Check for secondary attack
	if ( pOwner->m_nButtons & IN_ATTACK2 )
	{
		if ( CanSecondaryAttack() )
		{
			SecondaryAttack();
		}
	}
}

//-----------------------------------------------------------------------------
// Handle tool mode switching
//-----------------------------------------------------------------------------
void CWeaponTool::HandleToolModeSwitch()
{
	int nNewMode = bm_toolmode.GetInt();
	if ( nNewMode != m_nToolMode )
	{
		SetToolMode( nNewMode );
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
CON_COMMAND( bm_toolmode, "Sets the current tool mode" )
{
	if ( args.ArgC() < 2 )
	{
		Msg( "Current tool mode: %d (%s)\n", bm_toolmode.GetInt(), GetToolName( bm_toolmode.GetInt() ) );
		Msg( "Usage: bm_toolmode <mode>\n" );
		Msg( "Available modes:\n" );
		for ( int i = 1; i < TOOL_MAX; i++ )
		{
			const ToolInfo_t *pInfo = GetToolInfo( i );
			if ( pInfo && pInfo->pszName )
			{
				Msg( "  %d - %s\n", i, pInfo->pszName );
			}
		}
		return;
	}

	int nMode = atoi( args.Arg(1) );
	bm_toolmode.SetValue( nMode );
	Msg( "Tool mode set to: %d (%s)\n", nMode, GetToolName( nMode ) );
}

CON_COMMAND( bm_toolweapon, "Switches to tool weapon" )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;

	// Give player the tool weapon
	pPlayer->GiveNamedItem( "weapon_tool" );

	// Switch to it
	CBaseCombatWeapon *pWeapon = pPlayer->Weapon_OwnsThisType( "weapon_tool" );
	if ( pWeapon )
	{
		pPlayer->Weapon_Switch( pWeapon );
		Msg( "Switched to tool weapon\n" );
	}
}