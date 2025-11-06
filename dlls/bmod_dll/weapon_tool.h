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
// Tool mode definitions - matching Garry's Mod values discovered in IDA
//-----------------------------------------------------------------------------
enum ToolMode_t
{
	TOOL_NONE = 0,
	TOOL_GUN = 1,			// Gun tool
	TOOL_PHYSGUN = 2,		// Physics gun (should use weapon_physcannon instead)
	TOOL_CAMERA = 3,		// Camera tool
	TOOL_NPC = 4,			// NPC spawning tool
	TOOL_NEXTBOT = 5,		// NextBot tool
	TOOL_MATERIAL = 6,		// Material tool
	TOOL_COLOR = 7,			// Color tool
	TOOL_PAINT = 8,			// Paint tool
	TOOL_INFLATOR = 9,		// Inflator/size tool
	TOOL_DUPLICATOR = 10,	// Duplicator tool
	TOOL_CONSTRAINER = 11,	// Constraint tool
	TOOL_AXIS = 12,			// Axis constraint
	TOOL_BALLSOCKET = 13,	// Ball socket constraint
	TOOL_ROPE = 14,			// Rope constraint
	TOOL_PULLEY = 15,		// Pulley constraint
	TOOL_MOTOR = 16,		// Motor constraint
	TOOL_HYDRAULIC = 17,	// Hydraulic constraint
	TOOL_PNEUMATIC = 18,	// Pneumatic constraint
	TOOL_SPRING = 19,		// Spring constraint
	TOOL_KEEPUPRIGHT = 20,	// Keep upright constraint

	TOOL_MAX = 32			// Maximum tool modes supported
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

	// Tool implementations - override these for specific tools
	virtual void	OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary ) {}
	virtual void	OnToolTrace( trace_t &tr, bool bPrimary ) {}
	virtual void	OnToolThink() {}

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

	// Tool mode data - array of tool information
	static ToolInfo_t s_ToolInfo[TOOL_MAX];

	// Activity lookup table for different tool modes
	Activity		GetToolActivity( int nMode, bool bPrimary );

private:
	// Input handling
	void			CheckToolInput();
	void			HandleToolModeSwitch();

	// Tool helpers
	bool			CheckToolConstraints();
	void			ApplyToolForce( CBaseEntity *pEntity, const Vector &vecForce );
	void			CreateToolConstraint( CBaseEntity *pEnt1, CBaseEntity *pEnt2, int nType );
};

//-----------------------------------------------------------------------------
// Tool mode console variable
//-----------------------------------------------------------------------------
extern ConVar bm_toolmode;
extern ConVar bm_toolsound;

//-----------------------------------------------------------------------------
// Global tool functions
//-----------------------------------------------------------------------------
void ToolMode_Think();
void ToolMode_Register();
const ToolInfo_t *GetToolInfo( int nMode );
const char *GetToolName( int nMode );
const char *GetToolDescription( int nMode );

//-----------------------------------------------------------------------------
// Tool registration macros
//-----------------------------------------------------------------------------
#define DECLARE_TOOL_MODE( name, mode ) \
	class CTool##name : public CWeaponTool \
	{ \
	public: \
		virtual void OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary ); \
		virtual void OnToolTrace( trace_t &tr, bool bPrimary ); \
		virtual void OnToolThink(); \
	}; \
	extern CTool##name g_Tool##name;

#define IMPLEMENT_TOOL_MODE( name, mode ) \
	CTool##name g_Tool##name; \
	void CTool##name::OnToolUse( CBaseEntity *pEntity, trace_t &tr, bool bPrimary )

//-----------------------------------------------------------------------------
// Console commands
//-----------------------------------------------------------------------------
void CC_ToolMode();
void CC_ToolWeapon();

#endif // WEAPON_TOOL_H