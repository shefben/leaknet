//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Tool System - Base weapon_tool implementation
//          Based on Garry's Mod tool system discovered via IDA analysis
//
//=============================================================================

#ifndef WEAPON_TOOL_H
#define WEAPON_TOOL_H
#ifdef _WIN32
#pragma once
#endif

#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundenvelope.h"
#include "engine/IEngineSound.h"
#include "physics.h"
#include "utlvector.h"

//-----------------------------------------------------------------------------
// Tool mode definitions.
//
// These values are NOT arbitrary - they are the authentic GMod 9.0.4b
// gm_toolmode IDs, byte-identical to what ships in settings/menu_main/*.txt
// and gmod_9_0_4b/settings/menu_main/*.txt (real extracted game assets), and
// to what every third-party GMod9 addon menu file out there also uses.
// DO NOT renumber these - content files reference these numbers directly by
// value, not by symbol, so the numbers themselves are the contract.
//
// TOOL_NONE is a sentinel for "nothing selected yet" and deliberately does
// NOT use 0, since 0 is a real tool (Rope) in the authentic numbering.
//-----------------------------------------------------------------------------
enum ToolMode_t
{
	TOOL_NONE		= -1,	// sentinel only - falls back to TOOL_WELD (the real game's default starting tool)

	TOOL_ROPE		= 0,
	TOOL_ELASTIC	= 1,	// spring constraint ("Elastic" in the menu)
	TOOL_WELD		= 2,
	TOOL_BALLSOCKET	= 3,
	TOOL_PULLEY		= 4,
	TOOL_EASYWELD	= 5,
	TOOL_EASYBALL	= 6,
	TOOL_AXIS		= 7,
	TOOL_SLIDER		= 8,
	TOOL_NAILGUN	= 9,
	TOOL_FACEPOSER	= 10,
	TOOL_EYESPOSER	= 11,
	TOOL_REMOVER	= 12,
	TOOL_IGNITE		= 13,
	TOOL_PAINT		= 14,
	TOOL_DUPLICATE	= 15,
	TOOL_COLOUR		= 16,
	TOOL_MAGNETISE	= 17,
	TOOL_NOCOLLIDE	= 18,
	TOOL_DYNAMITE	= 19,
	TOOL_MATERIAL	= 20,
	TOOL_RTCAMERA	= 21,
	// 22 is unused/reserved in the authentic numbering
	TOOL_THRUSTER	= 23,
	TOOL_PHYSPROPS	= 24,
	TOOL_STATUE		= 25,
	TOOL_BALLOON	= 26,
	TOOL_EMITTER	= 27,
	TOOL_SPRITE		= 28,
	TOOL_WHEEL		= 29,

	// Auxiliary tools that the real menu reaches via "gm_context <name>" with
	// no gm_toolmode number at all (Camera/NPC Spawn live outside the
	// Constraints/Construction/Visual numbered families), plus the resize
	// tool which has no authentic gm_toolmode slot at all. Given internal
	// slots here so the existing tool_camera.cpp/tool_npc.cpp/tool_gun.cpp/
	// tool_inflator.cpp implementations are reachable via direct console
	// command (gmod_toolmode 30-33) even though no menu button wires to them.
	TOOL_GUN		= 30,
	TOOL_CAMERA		= 31,
	TOOL_NPCSPAWN	= 32,
	TOOL_INFLATOR	= 33,

	TOOL_MAX		= 40	// array bound - must stay above the highest value above
};

//-----------------------------------------------------------------------------
// Tool flags and settings
//-----------------------------------------------------------------------------
#define TOOL_FLAG_NONE				0
#define TOOL_FLAG_ALLOW_UNDERWATER	(1 << 0)
#define TOOL_FLAG_DISABLE_IN_VEHICLE	(1 << 1)
#define TOOL_FLAG_PRIMARY_ONLY		(1 << 2)
#define TOOL_FLAG_SECONDARY_ONLY	(1 << 3)

//-----------------------------------------------------------------------------
// Tool information structure
//-----------------------------------------------------------------------------
struct ToolInfo_t
{
	const char	*pszName;			// Tool name for localization key
	const char	*pszDisplayName;	// Human-readable name shown to the player
	const char	*pszDescription;	// Tool description
	const char	*pszHelpText;		// Help text
	const char	*pszViewModel;		// View model path
	const char	*pszWorldModel;		// World model path
	const char	*pszSound;			// Tool sound
	int			nFlags;				// Tool flags
	float		flRange;			// Tool range
	float		flDelay;			// Tool use delay
};

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CBaseEntity;
class CBasePlayer;
class IPhysicsObject;

//-----------------------------------------------------------------------------
// Base tool weapon class - implements weapon_tool from Garry's Mod
//-----------------------------------------------------------------------------
class CWeaponTool : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponTool, CBaseHLCombatWeapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	CWeaponTool();
	virtual ~CWeaponTool();

	// Weapon interface
	virtual void	Precache();
	virtual void	Spawn();
	virtual bool	Deploy();
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	Drop( const Vector &vecVelocity );
	virtual void	ItemPreFrame();
	virtual void	ItemPostFrame();

	// Combat
	virtual void	PrimaryAttack();
	virtual void	SecondaryAttack();
	virtual void	WeaponIdle();

	// Tool system
	virtual void	SetToolMode( int nMode );
	virtual int		GetToolMode() const { return m_nToolMode; }
	virtual void	DoToolAction( trace_t &tr, bool bPrimary );
	virtual bool	CanUseOnEntity( CBaseEntity *pEntity );
	virtual void	StartToolSound();
	virtual void	StopToolSound();

	// Tool implementations - CWeaponTool is the only class ever instantiated
	// for the "weapon_tool" entity (mode is a runtime int, not a C++ type),
	// so these dispatch by m_nToolMode to the per-tool free functions declared
	// in tool_dispatch.h instead of being overridden by a subclass.
	virtual void	OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
	virtual void	OnToolTrace( trace_t &tr, bool bPrimary );
	virtual void	OnToolThink();

	// Shared "select first point, then select second point" state used by every
	// constraint-family tool (Rope/Elastic/Weld/Ballsocket/Pulley/EasyWeld/
	// EasyBall/Axis/Slider/NailGun/NoCollide/Wheel) so those tool_*.cpp files
	// don't each need their own per-instance storage. A pending selection with
	// a NULL entity + m_bPendingWorld set means the first click landed on the
	// world (only meaningful for tools that support world attachment).
	CBaseEntity		*GetPendingEntity() const { return m_hPendingEntity.Get(); }
	bool			IsPendingWorld() const { return m_bPendingWorld; }
	bool			HasPendingSelection() const { return m_bPendingWorld || m_hPendingEntity.Get() != NULL; }
	const Vector	&GetPendingPos() const { return m_vecPendingPos; }
	int				GetPendingPhysBone() const { return m_nPendingPhysBone; }
	float			GetPendingTime() const { return m_flPendingTime; }
	void			SetPendingSelection( CBaseEntity *pEntity, const Vector &vecPos, int nPhysBone = -1 )
	{
		m_hPendingEntity = pEntity;
		m_bPendingWorld = ( pEntity == NULL );
		m_vecPendingPos = vecPos;
		m_nPendingPhysBone = nPhysBone;	// trace physicsbone, so ragdolls constrain the clicked limb
		m_flPendingTime = gpGlobals->curtime;
	}
	void			ClearPendingSelection()
	{
		m_hPendingEntity = NULL;
		m_bPendingWorld = false;
		m_vecPendingPos = vec3_origin;
		m_nPendingPhysBone = -1;
		m_flPendingTime = 0.0f;
	}

	// Utility functions
	virtual float	GetRange() const;
	virtual float	GetDelay() const;
	virtual bool	IsValidTarget( CBaseEntity *pEntity, trace_t &tr );
	virtual void	GetToolTrace( trace_t &tr );
	virtual void	UpdateToolEffects();

	// Tool effects
	virtual void	CreateToolEffect( const Vector &vecStart, const Vector &vecEnd );
	virtual void	CreateSparkEffect( const Vector &vecPos );
	virtual void	PlayToolSound( const char *pszSound );

	// Network variables - matching Garry's Mod structure from IDA
	CNetworkVar( int, m_nToolMode );
	CNetworkVar( bool, m_bToolActive );
	CNetworkVar( float, m_flNextToolTime );

protected:
	// Tool state variables - based on IDA analysis of C_WeaponTool structure
	float			m_flLastUseTime;		// Last tool use time
	float			m_flToolDelay;			// Current tool delay
	bool			m_bInUse;				// Tool currently in use

	// Sound management
	CSoundPatch		*m_pToolSound;			// Tool sound patch
	bool			m_bSoundStarted;		// Sound started flag

	// Tool targeting
	EHANDLE			m_hLastTarget;			// Last targeted entity
	Vector			m_vecLastTargetPos;		// Last target position

	// Shared constraint-tool pending-selection state (see accessors above)
	EHANDLE			m_hPendingEntity;
	bool			m_bPendingWorld;
	Vector			m_vecPendingPos;
	int				m_nPendingPhysBone;
	float			m_flPendingTime;

	// Activity lookup table for different tool modes
	Activity		GetToolActivity( int nMode, bool bPrimary );

public:
	// Tool mode data - array of tool information. Public since the free
	// functions GetToolInfo()/GetToolName()/GetToolDescription() below (the
	// intended external API for this table) read it from outside the class.
	static ToolInfo_t s_ToolInfo[TOOL_MAX];

private:
	// Tool helpers
	bool			CheckToolConstraints();
	void			ApplyToolForce( CBaseEntity *pEntity, const Vector &vecForce );
	void			CreateToolConstraint( CBaseEntity *pEnt1, CBaseEntity *pEnt2, int nType );
};

extern ConVar gmod_toolsound;

//-----------------------------------------------------------------------------
// Global tool functions
//-----------------------------------------------------------------------------
const ToolInfo_t *GetToolInfo( int nMode );
const char *GetToolName( int nMode );
const char *GetToolDescription( int nMode );

#endif // WEAPON_TOOL_H