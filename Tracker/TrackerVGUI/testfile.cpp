#include "interface.h"
#include "..\..\tracker\common\winlite.h"
#include <VGUI/VGui.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Panel.h>
#include <VGUI/IScheme.h>
#include <VGUI/ISurface.h>
#include <VGUI/ILocalize.h>
#include <VGUI/IVGui.h>
#include "filesystem.h"

#include <stdio.h>

#include "TrackerDialog.h"
#include "Tracker.h"
#include "IRunGameEngine.h"
//#include "ServerSession.h"
#include "ServerBrowser/IServerBrowser.h"
#include <WinSock2.h>

extern IRunGameEngine* g_pRunGameEngine;
extern IServerBrowser* g_pIServerBrowser;

//-----------------------------------------------------------------------------
// Purpose: Entry point
//			loads interfaces and initializes dialog
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WSAData wsaData;
	int nReturnCode = ::WSAStartup(MAKEWORD(2, 0), &wsaData);

	// Load vgui
	CSysModule *vguiModule = Sys_LoadModule("bin/vgui2" DLL_EXT_STRING);
	if (!vguiModule)
	{
		vguiModule = Sys_LoadModule("vgui2" DLL_EXT_STRING);
	}

	CreateInterfaceFn vguiFactory = Sys_GetFactory(vguiModule);
	if (!vguiFactory)
	{
		printf("Fatal error: Could not load vgui2%s\n", DLL_EXT_STRING);
		return 2;
	}	  

	CSysModule *filesystemModule = Sys_LoadModule("bin/filesystem_stdio" DLL_EXT_STRING);
	if (!filesystemModule)
	{
		filesystemModule = Sys_LoadModule("filesystem_stdio" DLL_EXT_STRING);
	}

	CreateInterfaceFn filesystemFactory = Sys_GetFactory(filesystemModule);
	if (!filesystemFactory)
	{
		printf("Fatal error: Could not load bin/filesystem_stdio%s\n", DLL_EXT_STRING);
		return 2;
	}	  

	// Initialize interfaces
	CreateInterfaceFn factories[2];
	factories[0] = vguiFactory;
	factories[1] = filesystemFactory;

	if (!vgui::VGui_InitInterfacesList("TrackerVGUI", factories, 2))
	{
		printf("Fatal error: Could not initalize vgui2.dll\n");
		return 3;
	}
	
	// In order to load resource files the file must be in your vgui filesystem path.
	vgui::filesystem()->AddSearchPath("../hl2/", "GAME");
	vgui::filesystem()->AddSearchPath("../platform/", "PLATFORM");

	// From CGameUI::Start
	vgui::filesystem()->AddSearchPath("../platform/config", "CONFIG");
//	vgui::system()->SetUserConfigFile("InGameDialogConfig.vdf", "CONFIG");

	// Init the surface
	vgui::surface()->Init();

	// Load the scheme
	if (!vgui::scheme()->LoadSchemeFromFile("Resource/TrackerScheme.res", "TrackerScheme"))
		return 1;

	// localization
	vgui::localize()->AddFile(vgui::filesystem(), "Resource/platform_english.txt");
	vgui::localize()->AddFile(vgui::filesystem(), "Resource/vgui_english.txt");
	vgui::localize()->AddFile(vgui::filesystem(), "friends/trackerui_english.txt");





	if (!g_pRunGameEngine)
	{
		g_pRunGameEngine = (IRunGameEngine*)Sys_GetFactoryThis()(RUNGAMEENGINE_INTERFACE_VERSION, NULL);
	}

	if (!g_pIServerBrowser)
	{
		g_pIServerBrowser = (IServerBrowser*)Sys_GetFactoryThis()(SERVERBROWSER_INTERFACE_VERSION, NULL);
	}




	// Make a embedded panel
	vgui::Panel *panel = new vgui::Panel(NULL, "TopPanel");
	vgui::surface()->SetEmbeddedPanel( panel->GetVPanel() );

	// Start vgui
	vgui::ivgui()->Start();

	// Add our main window
//	CControlCatalog *panelZoo = new CControlCatalog();
//	panelZoo->Activate();

	Tracker_SetStandaloneMode(true);

	CTrackerDialog* trackerDialog = new CTrackerDialog;
	trackerDialog->SetTitle("#TrackerUI_Friends_Title", true);
	trackerDialog->MakePopup();

	if (trackerDialog->Start())
	{
	//	trackerDialog->SetVisible(false);
	}

	CTrackerDialog::GetInstance()->Activate();

	// Run app frame loop
	while (vgui::ivgui()->IsRunning())
	{
		vgui::ivgui()->RunFrame();
	}

	// Shutdown
	CTrackerDialog::GetInstance()->Shutdown();
	CTrackerDialog::GetInstance()->MarkForDeletion();
	vgui::surface()->Shutdown();

//	delete panelZoo;
//	delete trackerDialog;
//	delete panel;

	Sys_UnloadModule(vguiModule);

	::WSACleanup();

	return 1;
}






