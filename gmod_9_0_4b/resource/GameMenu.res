"GameMenu"
{
	"1"
	{
		"label" "#GameUI_GameMenu_ResumeGame"
		"command" "ResumeGame"
		"OnlyInGame" "1"
	}
	
	"2"
	{
		"label" "#GameUI_GameMenu_Disconnect"
		"command" "Disconnect"
		"OnlyInGame" "1"
	}
	
	"3"
	{
		"label" "#GameUI_GameMenu_PlayerList"
		"command" "OpenPlayerListDialog"
		"OnlyInGame" "1"
	}
	
	"4"
	{
		"label" ""
		"command" ""
		"OnlyInGame" "1"
	}	
	
	"5"
	{
		"label" "START SINGLEPLAYER GAME"
		"command" "engine showspmenu"
		"notmulti" "1" 
	}
	
	"6"
	{
		"label" "#GameUI_GameMenu_LoadGame"
		"command" "OpenLoadGameDialog"
		"notmulti" "1"
	}
	"7"
	{
		"label" "#GameUI_GameMenu_SaveGame"
		"command" "OpenSaveGameDialog"
		"notmulti" "1"
		"OnlyInGame" "1"
	}
	
	"9"
	{
		"label" ""
		"command" ""
		"OnlyInGame" "1"
	}
	
	"10"
	{
		"label" "#GameUI_GameMenu_FindServers"
		"command" "OpenServerBrowser"
	}
	
	"11"
	{
		"label" "#GameUI_GameMenu_CreateServer"
		"command" "engine showmpmenu"
	}
	
	"12"
	{
		"label" "#GameUI_GameMenu_Friends"
		"command" "OpenFriendsDialog"
	}
	
	"13"
	{
		"label" "#GameUI_GameMenu_Options"
		"command" "OpenOptionsDialog"
	}
	
	"14"
	{
		"label" "#GameUI_GameMenu_Quit"
		"command" "Quit"
	}
	
	"15"
	{
		"label" " "
		"command" "spacer"
	}
	
	"16"
	{
		"label" "MOD MANAGER"
		"command" "engine showmodmenu"
	}
}

