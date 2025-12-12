// Client-side stub for physics_thruster / gmod_thruster to provide looped audio parity.
#pragma once

#ifdef BMOD_CLIENT_DLL
#include "c_baseentity.h"
#include "soundenvelope.h"

class C_PhysicsThruster : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_PhysicsThruster, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_PhysicsThruster();
	~C_PhysicsThruster();

	virtual void OnDataChanged( DataUpdateType_t type );
	virtual void UpdateOnRemove();

private:
	void StartLoopSound();
	void StopLoopSound();

	CSoundPatch *m_pLoopSound;
	Vector m_vecLocalOrigin;
};
#endif
