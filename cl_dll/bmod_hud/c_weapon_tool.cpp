//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Tool System - Client-side implementation
//          Based on Garry's Mod tool system discovered via IDA analysis
//
//=============================================================================

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"
#include "weapon_tool.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Client-side tool weapon
//-----------------------------------------------------------------------------
class C_WeaponTool : public C_BaseHLCombatWeapon
{
	DECLARE_CLASS( C_WeaponTool, C_BaseHLCombatWeapon );
	DECLARE_CLIENTCLASS();

public:
	C_WeaponTool();
	virtual ~C_WeaponTool();

	// Client-side overrides
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	UpdateOnRemove();
	virtual void	ClientThink();

	// Tool interface
	virtual int		GetToolMode() const { return m_nToolMode; }
	virtual bool	IsToolActive() const { return m_bToolActive; }

	// Tool effects
	virtual void	CreateToolEffect( const Vector &vecStart, const Vector &vecEnd );
	virtual void	UpdateToolEffects();
	virtual void	StopToolEffects();

private:
	// Network variables - must match server
	int		m_nToolMode;
	bool	m_bToolActive;
	float	m_flNextToolTime;

	// Client-side state
	int		m_nLastToolMode;
	bool	m_bLastToolActive;
	float	m_flEffectTime;
};

//-----------------------------------------------------------------------------
// Network table
//-----------------------------------------------------------------------------
IMPLEMENT_CLIENTCLASS_DT( C_WeaponTool, DT_WeaponTool, CWeaponTool )
	RecvPropInt( RECVINFO(m_nToolMode) ),
	RecvPropBool( RECVINFO(m_bToolActive) ),
	RecvPropFloat( RECVINFO(m_flNextToolTime) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Weapon stub
//-----------------------------------------------------------------------------
STUB_WEAPON_CLASS_IMPLEMENT( weapon_tool, C_WeaponTool );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
C_WeaponTool::C_WeaponTool()
{
	m_nToolMode = TOOL_GUN;
	m_bToolActive = false;
	m_flNextToolTime = 0.0f;
	m_nLastToolMode = -1;
	m_bLastToolActive = false;
	m_flEffectTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
C_WeaponTool::~C_WeaponTool()
{
	StopToolEffects();
}

//-----------------------------------------------------------------------------
// Data changed
//-----------------------------------------------------------------------------
void C_WeaponTool::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	// Check for tool mode changes
	if ( m_nToolMode != m_nLastToolMode )
	{
		DevMsg( "Client: Tool mode changed from %d to %d\n", m_nLastToolMode, m_nToolMode );
		m_nLastToolMode = m_nToolMode;
	}

	// Check for tool activation changes
	if ( m_bToolActive != m_bLastToolActive )
	{
		if ( m_bToolActive )
		{
			// Tool activated - start effects
			DevMsg( "Client: Tool activated\n" );
			m_flEffectTime = gpGlobals->curtime;
		}
		else
		{
			// Tool deactivated - stop effects
			DevMsg( "Client: Tool deactivated\n" );
			StopToolEffects();
		}
		m_bLastToolActive = m_bToolActive;
	}
}

//-----------------------------------------------------------------------------
// Update on remove
//-----------------------------------------------------------------------------
void C_WeaponTool::UpdateOnRemove()
{
	StopToolEffects();
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Client think
//-----------------------------------------------------------------------------
void C_WeaponTool::ClientThink()
{
	BaseClass::ClientThink();

	// Update tool effects
	UpdateToolEffects();
}

//-----------------------------------------------------------------------------
// Create tool effect
//-----------------------------------------------------------------------------
void C_WeaponTool::CreateToolEffect( const Vector &vecStart, const Vector &vecEnd )
{
	// This would create visual effects like beams, particles, etc.
	// For now, just debug output
	DevMsg( "Client: Tool effect from (%f,%f,%f) to (%f,%f,%f)\n",
		vecStart.x, vecStart.y, vecStart.z,
		vecEnd.x, vecEnd.y, vecEnd.z );
}

//-----------------------------------------------------------------------------
// Update tool effects
//-----------------------------------------------------------------------------
void C_WeaponTool::UpdateToolEffects()
{
	if ( !m_bToolActive )
		return;

	// Update any ongoing effects
	float flElapsed = gpGlobals->curtime - m_flEffectTime;
	if ( flElapsed > 0.1f ) // Stop effects after 0.1 seconds
	{
		StopToolEffects();
	}
}

//-----------------------------------------------------------------------------
// Stop tool effects
//-----------------------------------------------------------------------------
void C_WeaponTool::StopToolEffects()
{
	// Stop any ongoing effects
	m_flEffectTime = 0.0f;
}