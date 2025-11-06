#if !defined( VGUI_INTWRAP2_H )
#define VGUI_INTWRAP2_H

extern void VGuiWrap2_LoadingStarted( const char *resourceType, const char *resourceName );
extern void VGuiWrap2_LoadingFinished( const char *resourceType, const char *resourceName );
extern void StartLoadingProgressBar( const char *loadingType, int numProgressPoints );
extern void StopLoadingProgressBar();
extern void SetLoadingProgressBarStatusText( const char *statusText );
extern void ContinueLoadingProgressBar( const char *loadingType, int progressPoint, float progressFraction );
extern void SetSecondaryProgressBar( float progress );
extern void SetSecondaryProgressBarText( const char *statusText );

#endif
