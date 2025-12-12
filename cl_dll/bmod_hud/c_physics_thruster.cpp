// Client-side stub for physics_thruster / gmod_thruster (GMod 9 parity)
#include "cbase.h"

#ifdef BMOD_CLIENT_DLL

#include "c_physics_thruster.h"
#include "SoundEmitterSystemBase.h"
#include "soundenvelope.h"

extern ConVar gm_thruster_sounds;

IMPLEMENT_CLIENTCLASS_DT( C_PhysicsThruster, DT_PhysicsThruster, CPhysThruster )
	RecvPropVector( RECVINFO_NAME(m_vecLocalOrigin, m_localOrigin) ),
END_RECV_TABLE()

C_PhysicsThruster::C_PhysicsThruster()
{
	m_pLoopSound = nullptr;
}

C_PhysicsThruster::~C_PhysicsThruster()
{
	StopLoopSound();
}

void C_PhysicsThruster::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		StartLoopSound();
	}
}

void C_PhysicsThruster::UpdateOnRemove()
{
	StopLoopSound();
	BaseClass::UpdateOnRemove();
}

void C_PhysicsThruster::StartLoopSound()
{
	if ( m_pLoopSound || !gm_thruster_sounds.GetBool() )
		return;

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	CLocalPlayerFilter filter;

	// Use a dependable HL2 loop sound present in the 2003 content set.
	const char *pszSound = "Weapon_PhysCannon.HoldLoop";
	m_pLoopSound = controller.SoundCreate( filter, entindex(), pszSound );
	if ( m_pLoopSound )
	{
		controller.Play( m_pLoopSound, 1.0f, 100 );
	}
}

void C_PhysicsThruster::StopLoopSound()
{
	if ( !m_pLoopSound )
		return;

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	controller.SoundDestroy( m_pLoopSound );
	m_pLoopSound = nullptr;
}

#endif // BMOD_CLIENT_DLL
