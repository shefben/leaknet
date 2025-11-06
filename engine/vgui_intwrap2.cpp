#include "quakedef.h"
#include "vgui_intwrap2.h"
#include "sv_main.h"

#if !defined( SWDS )
#include "../common/GameUI/IGameUI.h"
#include "../public/BaseUI/IBaseUI.h"

extern IGameUI *staticGameUIFuncs;
extern IBaseUI *staticUIFuncs;

extern void SCR_UpdateScreen();
extern bool UserIsConnectedOnLoopback();
#endif

#if !defined( SWDS )

//-----------------------------------------------------------------------------
// Purpose: 
// TODO: CL_BatchResourceRequest
//-----------------------------------------------------------------------------
void VGuiWrap2_LoadingStarted( const char *resourceType, const char *resourceName )
{
	return;

	if( !staticGameUIFuncs )
		return;

	staticGameUIFuncs->LoadingStarted( resourceType, resourceName );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGuiWrap2_LoadingFinished( const char *resourceType, const char *resourceName )
{
	// @FIXME(SanyaSho): For some reason baseuifuncs->HideGameUI() makes impossible to open console in-game.
	return; // HACK!

	if( !staticGameUIFuncs )
		return;

	staticGameUIFuncs->LoadingFinished( resourceType, resourceName );
}

//-----------------------------------------------------------------------------
// Purpose: 
// TODO: CL_BatchResourceRequest
//-----------------------------------------------------------------------------
void StartLoadingProgressBar( const char *loadingType, int numProgressPoints )
{
	// NOTE: there's no way to check for gmodinfo.type != SINGLEPLAYER_ONLY
	if( !IsSinglePlayerGame() && ( !UserIsConnectedOnLoopback() || svs.maxclients != 1 /*|| (gmodinfo.type != SINGLEPLAYER_ONLY)*/) )
	{
		if( staticUIFuncs )
			staticUIFuncs->ActivateGameUI();

		if( staticGameUIFuncs )
		{
			staticGameUIFuncs->StartProgressBar( loadingType, numProgressPoints );

			SCR_UpdateScreen();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// TODO: DownloadManager::Reset; DownloadManager::CheckActiveDownload;
//	CL_HTTPUpdate; DownloadManager::StartNewDownload; CL_PrecacheResources;
//	CL_SendResourceListBlock; CL_PrecacheBSPModels
//-----------------------------------------------------------------------------
void ContinueLoadingProgressBar( const char *loadingType, int progressPoint, float progressFraction )
{
	if( !staticGameUIFuncs )
		return;

	if( staticGameUIFuncs->ContinueProgressBar( progressPoint, progressFraction ) != 0 )
	{
		SCR_UpdateScreen();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// TODO: DownloadManager::StartNewDownload; CL_BatchResourceRequest;
//	CL_ReadPackets; CL_PrecacheResources
//-----------------------------------------------------------------------------
void SetLoadingProgressBarStatusText( const char *statusText )
{
	if( !staticGameUIFuncs )
		return;

	if( staticGameUIFuncs->SetProgressBarStatusText( statusText ) != 0 )
	{
		SCR_UpdateScreen();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// TODO: CL_HTTPUpdate; DownloadManager::Reset; DownloadManager::CheckActiveDownload
//-----------------------------------------------------------------------------
void StopLoadingProgressBar()
{
	if( staticUIFuncs )
	{
		if( cls.state == ca_active )
		{
			staticUIFuncs->HideGameUI();
		}
		else
		{
#if 0
			if( staticClient )
			{
				staticClient->HideAllVGUIMenu();
			}
#endif
			staticUIFuncs->ActivateGameUI();
		}
	}

	if( staticGameUIFuncs )
		staticGameUIFuncs->StopProgressBar( gfExtendedError != false, gszDisconnectReason, /*gszExtendedDisconnectReason*/ "" );

	gfExtendedError = false;
	gszDisconnectReason[0] = 0;
	//gszExtendedDisconnectReason[0] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// TODO: DownloadManager::StartNewDownload; CL_ReadPackets
//-----------------------------------------------------------------------------
void SetSecondaryProgressBar( float progress )
{
	if( !staticGameUIFuncs )
		return;

	staticGameUIFuncs->SetSecondaryProgressBar( progress );
}

//-----------------------------------------------------------------------------
// Purpose: 
// TODO: DownloadManager::StartNewDownload; CL_ReadPackets
//-----------------------------------------------------------------------------
void SetSecondaryProgressBarText( const char *statusText )
{
	if( !staticGameUIFuncs )
		return;

	staticGameUIFuncs->SetSecondaryProgressBarText( statusText );
}

#else

void VGuiWrap2_LoadingStarted( const char *resourceType, const char *resourceName ) {}
void StartLoadingProgressBar( const char *loadingType, int numProgressPoints ) {}
void StopLoadingProgressBar() {}
void SetLoadingProgressBarStatusText( const char *statusText ) {}
void ContinueLoadingProgressBar( const char *loadingType, int progressPoint, float progressFraction ) {}
void SetSecondaryProgressBar( float progress ) {}
void SetSecondaryProgressBarText( const char *statusText ) {}

#endif
