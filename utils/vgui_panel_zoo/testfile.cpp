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

#include "CControlCatalog.h"

#include <stdio.h>

//-----------------------------------------------------------------------------
// Purpose: Entry point
//			loads interfaces and initializes dialog
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
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

	if (!vgui::VGui_InitInterfacesList("VGUIPanelZoo", factories, 2))
	{
		printf("Fatal error: Could not initalize vgui2%s\n", DLL_EXT_STRING);
		return 3;
	}
	
	// In order to load resource files the file must be in your vgui filesystem path.
	vgui::filesystem()->AddSearchPath("../hl2/", "GAME");
	vgui::filesystem()->AddSearchPath("../platform/", "PLATFORM");

	// Init the surface
	vgui::surface()->Init();

	// Load the scheme
	if (!vgui::scheme()->LoadSchemeFromFile("Resource/TrackerScheme.res", "TrackerScheme"))
		return 1;

	// localization
	vgui::localize()->AddFile(vgui::filesystem(), "Resource/platform_english.txt");
	vgui::localize()->AddFile(vgui::filesystem(), "Resource/vgui_english.txt");

	// Make a embedded panel
	vgui::Panel *panel = new vgui::Panel(NULL, "TopPanel");
	vgui::surface()->SetEmbeddedPanel( panel->GetVPanel() );

	// Start vgui
	vgui::ivgui()->Start();

	// Add our main window
	CControlCatalog *panelZoo = new CControlCatalog();
	panelZoo->Activate();

	// Run app frame loop
	while (vgui::ivgui()->IsRunning())
	{
		vgui::ivgui()->RunFrame();
	}

	// Shutdown
	vgui::surface()->Shutdown();

	delete panelZoo;
//	delete panel;

	Sys_UnloadModule(vguiModule);
	return 1;
}






