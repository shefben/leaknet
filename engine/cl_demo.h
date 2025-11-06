//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef CL_DEMO_H
#define CL_DEMO_H
#ifdef _WIN32
#pragma once
#endif

#include "demo.h"

struct DemoCommandQueue
{
	DemoCommandQueue()
	{
		t = 0.0;
	}
	float			t;
	democmdinfo_t	info;
};

//-----------------------------------------------------------------------------
// Purpose: Implements IDemo and handles demo file i/o
// Demos are more or less driven off of network traffic, but there are a few
//  other kinds of data items that are also included in the demo file:  specifically
//  commands that the client .dll itself issued to the engine are recorded, though they
//  probably were not the result of network traffic.
// At the start of a connection to a map/server, all of the signon, etc. network packets
//  are buffered.  This allows us to actually decide to start recording the demo at a later
//  time.  Once we actually issue the recording command, we don't actually start recording 
//  network traffic, but instead we ask the server for an "uncompressed" packet (otherwise
//  we wouldn't be able to deal with the incoming packets during playback because we'd be missing the
//  data to delta from ) and go into a waiting state.  Once an uncompressed packet is received, 
//  we unset the waiting state and start recording network packets from that point forward.
// Demo's record the elapsed time based on the current client clock minus the time the demo was started
// During playback, the elapsed time for playback ( based on the host_time, which is subject to the
//  host_frametime cvar ) is compared with the elapsed time on the message from the demo file.  
// If it's not quite time for the message yet, the demo input stream is rewound
// The demo system sits at the point where the client is checking to see if any network messages
//  have arrived from the server.  If the message isn't ready for processing, the demo system
//  just responds that there are no messages waiting and the client continues on
// Once a true network message with entity data is read from the demo stream, a couple of other
//  actions occur.  First, the timestamp in the demo file and the view origin/angles corresponding
//  to the message are cached off.  Then, we search ahead (into the future) to find out the next true network message
//  we are going to read from the demo file.  We store of it's elapsed time and view origin/angles
// Every frame that the client is rendering, even if there is no data from the demo system,
//  the engine asks the demo system to compute an interpolated origin and view angles.  This
//  is done by taking the current time on the host and figuring out how far that puts us between
//  the last read origin from the demo file and the time when we'll actually read out and use the next origin
// We use Quaternions to avoid gimbel lock on interpolating the view angles
// To make a movie recorded at a fixed frame rate you would simply set the host_framerate to the
//  desired playback fps ( e.g., 0.02 == 50 fps ), then issue the startmovie command, and then
//  play the demo.  The demo system will think that the engine is running at 50 fps and will pull
//  messages accordingly, even though movie recording kills the actually framerate.
// It will also render frames with render origin/angles interpolated in-between the previous and next origins
//  even if the recording framerate was not 50 fps or greater.  The interpolation provides a smooth visual playback 
//  of the demo information to the client without actually adding latency to the view position (because we are
//  looking into the future for the position, not buffering the past data ).
//-----------------------------------------------------------------------------
class CDemo : public IDemo
{
public:
					CDemo( void );
					~CDemo( void );

	void			Init( void );
	void			Shutdown( void );

	void			StopPlayback( void );
	void			StopRecording( void );

	bool			IsPlayingBack( void ) const;
	bool			IsPlayingBack_TimeDemo( void );
	bool			IsTimeDemoTimerStarted( void );

	bool			IsRecording( void );

	void			RecordCommand( const char *cmdname );

	void			MarkFrame( float flCurTime, float flFPSVariability );

	void			StartupDemoHeader( void );

	bool			ReadMessage( void );

	void			WriteMessage( bool startup, int start, sizebuf_t *msg );

	void			SetWaitingForUpdate( bool waiting );
	bool			IsWaitingForUpdate( void );

	void			GetInterpolatedViewpoint( void );

	void			PausePlayback( float autoresumeafter = 0.0f );
	void			ResumePlayback();
	bool			IsPlaybackPaused() const;

	void			SkipToFrame( int whentoresume, bool pauseafterskip = false );
	void			SkipToTime( float whentoresume, bool pauseafterskip = false );

	float			GetProgress( void );
	void			GetProgressFrame( int &frame, int& totalframes );
	void			GetProgressTime( float& current, float& totaltime );

	void			SetAutoPauseAfterInit( bool pause );
	float			GetPlaybackTimeScale( void ) const;
	void			SetFastForwarding( bool ff );
	void			AdvanceOneFrame( void );

	void			WriteUserCmd( int cmdnumber );
	bool			IsFastForwarding( void );
	bool			IsSkippingAhead( void );

	void			QuitAfterTimeDemo()					{ m_bQuitAfterTimeDemo = true; }
	void			SnapshotAfterFrame( int nFrame )	{ m_nSnapshotFrame = nFrame; }
	void			SetSnapshotFilename( const char *pFileName );
	const char *		GetSnapshotFilename( void );

	virtual float			GetPlaybackRateModifier( void ) const;
	virtual void			SetPlaybackRateModifier( float rate );

	virtual void			DispatchEvents( void );

	virtual void			DrawDebuggingInfo( void );

	virtual void			LoadSmoothingInfo( const char *filename, CSmoothingContext& smoothing );
	virtual void			ClearSmoothingInfo( CSmoothingContext& smoothing );
	virtual void			SaveSmoothingInfo( char const *filename, CSmoothingContext& smoothing );

	virtual void			SetAutoRecord( char const *mapname, char const *demoname );
	virtual void			CheckAutoRecord( char const *mapname );

	virtual void			StartRendering( void );
 	virtual bool			IsTakingSnapshot();

public:

	bool			IsValidDemoHeader( void );


	void			Play( const char *name );
	void			Record( const char *name );
	int				GetNumFrames( const char *name );
	void			List( const char *name );

	void			Play_TimeDemo( const char *name );

private:
	bool			_ReadMessage( void );

	void			FinishTimeDemo( void );

	bool			MoveToNextSection( void );

	void			ReadSequenceInfo( FileHandle_t demofile, bool discard );
	void			ReadCmdInfo( FileHandle_t demofile, democmdinfo_t& info );
	void			ReadCmdHeader( FileHandle_t demofile, byte& cmd, float& dt, int& dframe );
	void			ReadUserCmd( FileHandle_t demofile, bool discard );

	void			WriteGetCmdInfo( democmdinfo_t& info );

	void			WriteSequenceInfo ( FileHandle_t fp );
	void			WriteCmdHeader( byte cmd, FileHandle_t fp );
	void			WriteCmdInfo( FileHandle_t fp, democmdinfo_t& info );

	void			WriteStringTables( FileHandle_t& demofile );
	void			ReadStringTables( int expected_length, FileHandle_t& demofile );

	float			GetPlaybackClock( void );
	float			GetRecordingClock( void );

	void			RewindMessageHeader( int oldposition );
	bool			ReadRawNetworkData( FileHandle_t demofile, int framecount, sizebuf_t& msg );

	bool			ParseAheadForInterval( float interval, CUtlVector< DemoCommandQueue >& list );
	void			InterpolateDemoCommand( CUtlVector< DemoCommandQueue >& list, float t, float& frac, democmdinfo_t& prev, democmdinfo_t& next );

	bool			ShouldSkip( float elapsed, int frame );

	bool			OverrideViewSetup( democmdinfo_t& info );
	void			ParseSmoothingInfo( FileHandle_t demofile, demodirectory_t& directory, CUtlVector< demosmoothing_t >& smooth );

	void			ClearAutoRecord( void );

	void			WriteTimeDemoResults( void );

private:
	struct SkipContext
	{
		bool		active;
		bool		useframe;
		bool		pauseafterskip;
		int			skiptoframe;
		float		skiptotime;

		float		skipstarttime;
		int			skipstartframe;

		int			skipframesremaining;
	};

	enum
	{
		GG_MAX_FRAMES = 300,
	};

	// Demo lump types
	enum
	{
		// This lump contains startup info needed to spawn into the server
		DEMO_STARTUP	= 0,    
		// This lump contains playback info of messages, etc., needed during playback.
		DEMO_NORMAL		= 1    
	};

	// Demo messages
	enum
	{
		// it's a startup message, process as fast as possible
		dem_norewind	= 1,
		// it's a normal network packet that we stored off
		dem_read,
		// move the demostart time value forward by this amount.
		dem_jumptime,
		// it's a command that the hud issued
		dem_clientdll,
		// usercmd
		dem_usercmd,
		// client string table state
		dem_stringtables,
		// end of time.
		dem_stop,

		// Last command
		dem_lastcmd		= dem_stop
	};

	// to meter out one message a frame
	int				m_nTimeDemoLastFrame;		
	// host_tickcount at start
	int				m_nTimeDemoStartFrame;		
	// realtime at second frame of timedemo
	float			m_flTimeDemoStartTime;		
	// Frame rate variability
	float			m_flTotalFPSVariability;
	// # of demo frames in this segment.
	int				m_nFrameCount;     

	// For synchronizing playback and recording.
	float			m_flStartTime;     
	// For synchronizing playback during timedemo.
	int				m_nStartTick;     
	// Name of demo file we are appending onto.
	char			m_szDemoFileName[ MAX_OSPATH ];  
	// For recording demos.
	FileHandle_t	m_pDemoFile;          
	// For saving startup data to start playing a demo midstream.
	FileHandle_t	m_pDemoFileHeader;  

	bool			m_bRecording;
	bool			m_bPlayingBack;
	bool			m_bTimeDemo;
	// I.e., demo is waiting for first nondeltacompressed message to arrive.
	//  We don't actually start to record until a non-delta message is received
	bool			m_bRecordWaiting;
	   
	// Game Gauge Stuff
	int				m_nGGMaxFPS;
	int				m_nGGMinFPS;
	int				m_nGGFrameCount;
	float			m_flGGTime;
	int				m_rgGGFPSFrames[ GG_MAX_FRAMES ];
	int				m_nGGSeconds;

	// Demo file i/o stuff
	demoentry_t*	m_pEntry;
	int				m_nEntryIndex;
	demodirectory_t	m_DemoDirectory;
	demoheader_t    m_DemoHeader;

	CUtlVector< DemoCommandQueue > m_DestCmdInfo;

	float			m_flLastFrameElapsed;
	democmdinfo_t	m_LastCmdInfo;

	bool			m_bInterpolateView;

	bool			m_bTimeDemoTimerStarted;
	int				m_nStartShowMessages;

	bool			m_bPlaybackPaused;
	float			m_flAutoResumeTime;
	SkipContext		m_Skip;
	bool			m_bAutoPause;
	bool			m_bFastForwarding;
	int				m_nAutoPauseFrame;

	float			m_flFastForwardStartTime;
	float			m_flPlaybackRateModifier;

	bool			m_bAutoRecord;
	char			m_szAutoRecordMap[ MAX_OSPATH ];
	char			m_szAutoRecordDemoName[ MAX_OSPATH ];

	bool			m_bQuitAfterTimeDemo;
	int				m_nSnapshotFrame;
	int				m_nFrameStartedRendering;
	char			m_pSnapshotFilename[MAX_OSPATH];
};

// Create object
static CDemo g_Demo;
IDemo *demo = ( IDemo * )&g_Demo;

#endif // CL_DEMO_H
