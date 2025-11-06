//====== Copyright © 1996-2003, Valve Corporation, All rights reserved. =======
//
// VXP: Old HL2 entities that has been used in WC mappack
//
//=============================================================================

#include "cbase.h"
#include "player.h"
#include "mathlib.h"
#include "ai_speech.h"
#include "stringregistry.h"
#include "gamerules.h"
#include "game.h"
#include <ctype.h>
#include "mem_fgets.h"
#include "entitylist.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "ndebugoverlay.h"
#include "soundscape.h"
#include "globalstate.h"

#define SPEAKER_START_SILENT			1	// wait for trigger 'on' to start announcements

// ===================================================================================
//
// Speaker class. Used for announcements per level.
//

class CSpeakerOld : public CPointEntity
{
	DECLARE_CLASS( CSpeakerOld, CPointEntity );
public:
	bool KeyValue( const char* szKeyName, const char* szValue );
	void Spawn( void );
	void Precache( void );
	void ToggleUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );
	void SpeakerThink( void );

	virtual int	ObjectCaps( void ) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	int	m_preset;			// preset number
	string_t m_iszMessage;

	float	m_delayMin;
	float	m_delayMax;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( speaker, CSpeakerOld );

BEGIN_DATADESC( CSpeakerOld )

	DEFINE_FIELD( CSpeakerOld, m_preset, FIELD_INTEGER ),
	DEFINE_KEYFIELD( CSpeakerOld, m_delayMin, FIELD_FLOAT, "delaymin" ),
	DEFINE_KEYFIELD( CSpeakerOld, m_delayMax, FIELD_FLOAT, "delaymax" ),
	DEFINE_KEYFIELD( CSpeakerOld, m_iszMessage, FIELD_STRING, "message" ),

	// Function Pointers
	DEFINE_THINKFUNC( CSpeakerOld, SpeakerThink ),

	DEFINE_USEFUNC( CSpeakerOld, ToggleUse ),

END_DATADESC()

//
// speaker - general-purpose user-defined static sound
//
void CSpeakerOld::Spawn( void )
{
	char* szSoundFile = (char*)STRING( m_iszMessage );

	if ( !m_preset && (m_iszMessage == NULL_STRING || strlen( szSoundFile ) < 1) )
	{
		Msg( "SPEAKER with no Level/Sentence! at: %f, %f, %f\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z );
		SetNextThink( gpGlobals->curtime + 0.1 );
		SetThink( &CSpeakerOld::SUB_Remove );
		return;
	}
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );


	SetThink( &CSpeakerOld::SpeakerThink );
	SetNextThink( TICK_NEVER_THINK );

	// allow on/off switching via 'use' function.
	SetUse( &CSpeakerOld::ToggleUse );

	Precache();
}

void CSpeakerOld::Precache( void )
{
	if ( !FBitSet( GetSpawnFlags(), SPEAKER_START_SILENT ) )
		// set first announcement time for random n second
		SetNextThink( gpGlobals->curtime + random->RandomFloat( 5.0, 15.0 ) );
}
void CSpeakerOld::SpeakerThink( void )
{
	const char* szSoundFile = NULL;
	float flvolume = m_iHealth * 0.1;
	int flags = 0;
	int pitch = 100;


	// Wait for the talking characters to finish first.
	if ( !g_AITalkSemaphore.IsAvailable() )
	{
		SetNextThink( gpGlobals->curtime + g_AITalkSemaphore.GetReleaseTime() + random->RandomFloat( 5, 10 ) );
		return;
	}

	if ( m_preset )
	{
		// go lookup preset text, assign szSoundFile
		switch ( m_preset )
		{
		case 1: szSoundFile = "CITA_HIT"; break;
		case 2: szSoundFile = "CITA_THEME"; break;
		case 3: szSoundFile = "CITA_TRANS"; break;
		case 4: szSoundFile = "CITA_MIX"; break;
		case 5: szSoundFile = "CITA_MIXP"; break;
		case 6: szSoundFile = "C17_WARa"; break;
		case 7: szSoundFile = "C17_WARb"; break;
		case 8: szSoundFile = "C17_WARc"; break;
		case 9: szSoundFile = "C17_SPEECHa"; break;
		case 10: szSoundFile = "C17_SPEECHb"; break;
		case 11: szSoundFile = "C17_CROWD"; break;
		case 12: szSoundFile = "C17_PA"; break;
		case 13: szSoundFile = "C17_CALM"; break;
		case 14: szSoundFile = "C17_WAR"; break;
		}
	}
	else
		szSoundFile = (char*)STRING( m_iszMessage );

	if ( szSoundFile[0] == '!' )
	{
		// play single sentence, one shot
		UTIL_EmitAmbientSound( this, GetAbsOrigin(), szSoundFile,
							   flvolume, SNDLVL_120dB, flags, pitch );

		// shut off and reset
		SetNextThink( TICK_NEVER_THINK );
	}
	else
	{
		// make random announcement from sentence group

		if ( SENTENCEG_PlayRndSz( edict(), szSoundFile, flvolume, SNDLVL_120dB, flags, pitch ) < 0 )
			Msg( "Level Design Error!\nSPEAKER has bad sentence group name: %s\n", szSoundFile );

		// set next announcement time for random 5 to 10 minute delay
		SetNextThink( gpGlobals->curtime +
					  random->RandomFloat( m_delayMin, m_delayMax ) );

		// time delay until it's ok to speak: used so that two NPCs don't talk at once
		g_AITalkSemaphore.Acquire( 5 );
	}

	return;
}


//
// ToggleUse - if an announcement is pending, cancel it.  If no announcement is pending, start one.
//
void CSpeakerOld::ToggleUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	int fActive = (GetNextThink() > 0.0);

	// fActive is TRUE only if an announcement is pending

	if ( useType != USE_TOGGLE )
	{
		// ignore if we're just turning something on that's already on, or
		// turning something off that's already off.
		if ( (fActive && useType == USE_ON) || (!fActive && useType == USE_OFF) )
			return;
	}

	if ( useType == USE_ON )
	{
		// turn on announcements
		SetNextThink( gpGlobals->curtime + 0.1 );
		return;
	}

	if ( useType == USE_OFF )
	{
		// turn off announcements
		SetNextThink( TICK_NEVER_THINK );
		return;

	}

	// Toggle announcements


	if ( fActive )
	{
		// turn off announcements
		SetNextThink( TICK_NEVER_THINK );
	}
	else
	{
		// turn on announcements
		SetNextThink( gpGlobals->curtime + 0.1 );
	}
}

// KeyValue - load keyvalue pairs into member data
// NOTE: called BEFORE spawn!

bool CSpeakerOld::KeyValue( const char* szKeyName, const char* szValue )
{
	// preset
	if ( FStrEq( szKeyName, "preset" ) )
	{
		m_preset = atoi( szValue );
		return true;
	}
	else
		return BaseClass::KeyValue( szKeyName, szValue );
}

//
// ===================================================================================


// ===================================================================================
//
// Render parameters trigger
//
// This entity will copy its render parameters (renderfx, rendermode, rendercolor, renderamt)
// to its targets when triggered.
//


// Flags to indicate masking off various render parameters that are normally copied to the targets
#define SF_RENDER_MASKFX	(1<<0)
#define SF_RENDER_MASKAMT	(1<<1)
#define SF_RENDER_MASKMODE	(1<<2)
#define SF_RENDER_MASKCOLOR	(1<<3)

class CRenderFxManager : public CBaseEntity
{
	DECLARE_CLASS( CRenderFxManager, CBaseEntity );
public:
	void	Spawn( void );
	void	Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );

	// Input handlers.
	void	InputActivate( inputdata_t& inputdata );

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CRenderFxManager )
DEFINE_INPUTFUNC( CRenderFxManager, FIELD_VOID, "Activate", InputActivate ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( env_render, CRenderFxManager );


void CRenderFxManager::Spawn( void )
{
	AddSolidFlags( FSOLID_NOT_SOLID );
	SetMoveType( MOVETYPE_NONE );
	m_fEffects |= EF_NODRAW;
}


void CRenderFxManager::Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	if ( m_target != NULL_STRING )
	{
		CBaseEntity* pEntity = NULL;
		while ( (pEntity = gEntList.FindEntityByName( pEntity, STRING( m_target ), NULL )) != NULL )
		{
			if ( !HasSpawnFlags( SF_RENDER_MASKFX ) )
				pEntity->m_nRenderFX = m_nRenderFX;
			if ( !HasSpawnFlags( SF_RENDER_MASKAMT ) )
				pEntity->SetRenderColorA( GetRenderColor().a );
			if ( !HasSpawnFlags( SF_RENDER_MASKMODE ) )
				pEntity->m_nRenderMode = m_nRenderMode;
			if ( !HasSpawnFlags( SF_RENDER_MASKCOLOR ) )
				pEntity->m_clrRender = m_clrRender;
		}
	}
}


void CRenderFxManager::InputActivate( inputdata_t& inputdata )
{
	Use( inputdata.pActivator, inputdata.pCaller, USE_ON, 0 );
}

//
// ===================================================================================
