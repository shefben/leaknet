//====== Copyright © 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: replaces the cl_*.cpp files with stubs
//
//=============================================================================

#ifdef SWDS

#include "quakedef.h"
#include "client.h"
#include "enginestats.h"
#include "convar.h"

#include "cl_demo.h"
#include "cl_main.h"
#include "gl_matsysiface.h"

client_static_t	cls;
CClientState	cl;
CEngineStats g_EngineStats;

bool Demo_IsPlayingBack()
{
	return false;
}

bool Demo_IsPlayingBack_TimeDemo()
{
	return false;
}

// VXP: Here begins the abomination...
CDemo::CDemo( void ) {}
CDemo::~CDemo( void ) {}

void CDemo::Init( void ) {}
void CDemo::Shutdown( void ) {}

void CDemo::StopPlayback( void ) {}
void CDemo::StopRecording( void ) {}

bool CDemo::IsPlayingBack( void ) const { return false; }
bool CDemo::IsPlayingBack_TimeDemo( void ) { return false; }
bool CDemo::IsTimeDemoTimerStarted( void ) { return false; }

bool CDemo::IsRecording( void ) { return false; }

void CDemo::RecordCommand( const char *cmdname ) {}

void CDemo::MarkFrame( float flCurTime, float flFPSVariability ) {}

void CDemo::StartupDemoHeader( void ) {}

bool CDemo::ReadMessage( void ) { return false; }

void CDemo::WriteMessage( bool startup, int start, sizebuf_t *msg ) {}

void CDemo::SetWaitingForUpdate( bool waiting ) {}
bool CDemo::IsWaitingForUpdate( void ) { return false; }

void CDemo::GetInterpolatedViewpoint( void ) {}

void CDemo::PausePlayback( float autoresumeafter ) {}
void CDemo::ResumePlayback() {}
bool CDemo::IsPlaybackPaused() const { return false; }

void CDemo::SkipToFrame( int whentoresume, bool pauseafterskip ) {}
void CDemo::SkipToTime( float whentoresume, bool pauseafterskip ) {}

float CDemo::GetProgress( void ) { return 0.0f; }
void CDemo::GetProgressFrame( int &frame, int& totalframes ) {}
void CDemo::GetProgressTime( float& current, float& totaltime ) {}

void CDemo::SetAutoPauseAfterInit( bool pause ) {}
float CDemo::GetPlaybackTimeScale( void ) const { return 0.0f; }
void CDemo::SetFastForwarding( bool ff ) {}
void CDemo::AdvanceOneFrame( void ) {}

void CDemo::WriteUserCmd( int cmdnumber ) {}
bool CDemo::IsFastForwarding( void ) { return false; }
bool CDemo::IsSkippingAhead( void ) { return false; }

//void CDemo::QuitAfterTimeDemo() 		{ m_bQuitAfterTimeDemo = true }
//void CDemo::SnapshotAfterFrame( int nFrame )	{ m_nSnapshotFrame = nFrame }
void CDemo::SetSnapshotFilename( const char *pFileName ) {}
const char *		CDemo::GetSnapshotFilename( void ) { return NULL; }

float CDemo::GetPlaybackRateModifier( void ) const { return 0.0f; }
void CDemo::SetPlaybackRateModifier( float rate ) {}

void CDemo::DispatchEvents( void ) {}

void CDemo::DrawDebuggingInfo( void ) {}

void CDemo::LoadSmoothingInfo( const char *filename, CSmoothingContext& smoothing ) {}
void CDemo::ClearSmoothingInfo( CSmoothingContext& smoothing ) {}
void CDemo::SaveSmoothingInfo( char const *filename, CSmoothingContext& smoothing ) {}

void CDemo::SetAutoRecord( char const *mapname, char const *demoname ) {}
void CDemo::CheckAutoRecord( char const *mapname ) {}

void CDemo::StartRendering( void ) {}
bool CDemo::IsTakingSnapshot() { return false; }





void CClientState::Clear( void ) {}

float CClientState::gettime() const { return 0.0f; }
float CClientState::getframetime() const { return 0.0f; }

void  CClientState::DumpPrecacheStats( TABLEID table ) {}

TABLEID  CClientState::GetModelPrecacheTable( void ) const { return 0; }
void  CClientState::SetModelPrecacheTable( TABLEID id ) {}

TABLEID  CClientState::GetGenericPrecacheTable( void ) const { return 0; }
void  CClientState::SetGenericPrecacheTable( TABLEID id ) {}

TABLEID  CClientState::GetSoundPrecacheTable( void ) const { return 0; }
void  CClientState::SetSoundPrecacheTable( TABLEID id ) {}

TABLEID  CClientState::GetDecalPrecacheTable( void ) const { return 0; }
void  CClientState::SetDecalPrecacheTable( TABLEID id ) {}

TABLEID  CClientState::GetInstanceBaselineTable() const { return 0; }
void  CClientState::SetInstanceBaselineTable( TABLEID id ) {}
void  CClientState::UpdateInstanceBaseline( int iStringID ) {}

// Public API to models
model_t *CClientState::GetModel( int index ) { return NULL; }
void CClientState::SetModel( int tableIndex ) {}
int CClientState::LookupModelIndex( char const *name ) { return 0; }

// Public API to generic
char const *CClientState::GetGeneric( int index ) { return NULL; }
void CClientState::SetGeneric( int tableIndex ) {}
int CClientState::LookupGenericIndex( char const *name ) { return 0; }

// Public API to sounds
CSfxTable *CClientState::GetSound( int index ) { return NULL; }
char const *CClientState::GetSoundName( int index ) { return NULL; }
void CClientState::SetSound( int tableIndex ) {}
int CClientState::LookupSoundIndex( char const *name ) { return 0; }

// Public API to decals
char const *CClientState::GetDecalName( int index ) { return NULL; }
void CClientState::SetDecal( int tableIndex ) {}



// matsys_interface.cpp
void SurfSetupSurfaceContext( SurfaceCtx_t& ctx, int surfID ) {}
void SurfComputeTextureCoordinate( SurfaceCtx_t const& ctx, int surfID, Vector const& vec, Vector2D& uv ) {}
void SurfComputeLightmapCoordinate( SurfaceCtx_t const& ctx, int surfID, Vector const& vec, Vector2D& uv ) {}

#endif
