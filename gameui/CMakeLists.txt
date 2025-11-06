cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( GAMEUI_SOURCE_FILES )
BEGIN_SRC( GAMEUI_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/GameUI/ObjectList.cpp"
			"${SRCDIR}/common/GameUI/Random.cpp"
			"${SRCDIR}/public/checksum_md5.cpp"
			"${SRCDIR}/public/convar.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"
			# SanyaSho: these files are only used with MasterServerVersionCheck code enabled
			#"${SRCDIR}/Tracker/common/msgbuffer.cpp"
			#"${SRCDIR}/Tracker/common/netapi.cpp"
			#"${SRCDIR}/Tracker/common/Socket.cpp"

			"BackgroundMenuButton.cpp"
			"BasePanel.cpp"
			"ChangeGameDialog.cpp"
			"CreateMultiplayerGameBotPage.cpp"
			"CreateMultiplayerGameDialog.cpp"
			"CreateMultiplayerGameGameplayPage.cpp"
			"CreateMultiplayerGameServerPage.cpp"
			"GameConsole.cpp"
			"GameUI_Interface.cpp"
			"LoadingDialog.cpp"
			"$<${IS_WINDOWS}:LogoFile.cpp>"
			"ModInfo.cpp"
			"RunGameEngine.cpp"
			"ScriptObject.cpp"
			"Sys_Utils.cpp"
			"Taskbar.cpp"
			"TaskButton.cpp"
			"VGuiSystemModuleLoader.cpp"
		#}
	)
END_SRC( GAMEUI_SOURCE_FILES "Source Files" )

BEGIN_SRC( GAMEUI_SOURCE_FILES "Controls" )
	SRC_GRP(
		SOURCES
		#{
			"BitmapImagePanel.cpp"
			"BitmapImagePanel.h"
			"CommandCheckButton.cpp"
			"CommandCheckButton.h"
			"CvarNegateCheckButton.cpp"
			"CvarNegateCheckButton.h"
			"CvarSlider.cpp"
			"CvarSlider.h"
			"CvarTextEntry.cpp"
			"CvarTextEntry.h"
			"CvarToggleCheckButton.cpp"
			"CvarToggleCheckButton.h"
			"KeyToggleCheckButton.cpp"
			"KeyToggleCheckButton.h"
			"LabeledCommandComboBox.cpp"
			"LabeledCommandComboBox.h"
			"PanelListPanel.cpp"
			"PanelListPanel.h"
			"vcontrolslistpanel.cpp"
			"vcontrolslistpanel.h"
		#}
	)
END_SRC( GAMEUI_SOURCE_FILES "Controls" )

BEGIN_SRC( GAMEUI_SOURCE_FILES "Dialogs" )
	SRC_GRP(
		SOURCES
		#{
			"ContentControlDialog.cpp"
			"ContentControlDialog.h"
			"GameConsoleDialog.cpp"
			"GameConsoleDialog.h"
			"HelpDialog.cpp"
			"HelpDialog.h"
			"LoadGameDialog.cpp"
			"LoadGameDialog.h"
			"MultiplayerAdvancedDialog.cpp"
			"MultiplayerAdvancedDialog.h"
			"NewGameDialog.cpp"
			"NewGameDialog.h"
			"PlayerListDialog.cpp"
			"PlayerListDialog.h"
			"SaveGameDialog.cpp"
			"SaveGameDialog.h"
			"SteamPasswordDialog.cpp"
			"SteamPasswordDialog.h"
		#}
	)
END_SRC( GAMEUI_SOURCE_FILES "Dialogs" )

BEGIN_SRC( GAMEUI_SOURCE_FILES "Options Dialog" )
	SRC_GRP(
		SOURCES
		#{
			"OptionsDialog.cpp"
			"OptionsDialog.h"
			"OptionsSubAdvanced.cpp"
			"OptionsSubAdvanced.h"
			"OptionsSubAudio.cpp"
			"OptionsSubAudio.h"
			"OptionsSubKeyboard.cpp"
			"OptionsSubKeyboard.h"
			"OptionsSubMouse.cpp"
			"OptionsSubMouse.h"
			"OptionsSubMultiplayer.cpp"
			"OptionsSubMultiplayer.h"
			"OptionsSubVideo.cpp"
			"OptionsSubVideo.h"
			"OptionsSubVoice.cpp"
			"OptionsSubVoice.h"
		#}
	)
END_SRC( GAMEUI_SOURCE_FILES "Options Dialog" )

set( GAMEUI_HEADER_FILES )
BEGIN_SRC( GAMEUI_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/GameUI/IGameUI.h"
			"${SRCDIR}/common/GameUI/ObjectList.h"
			"${SRCDIR}/common/GameUI/Random.h"
			"${SRCDIR}/common/IObjectContainer.h"
			"${SRCDIR}/common/IRunGameEngine.h"
			"${SRCDIR}/common/IVguiModule.h"
			"${SRCDIR}/common/IVGuiModuleLoader.h"
			"${SRCDIR}/public/BaseUI/IBaseUI.h"
			#"${SRCDIR}/public/BaseUI/IGameConsole.h"
			"${SRCDIR}/public/cl_dll/IGameClientExports.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/Tracker/common/TrackerMessageFlags.h"
			"${SRCDIR}/vgui2/src/Memorybitmap.h"

			"BackgroundMenuButton.h"
			"BasePanel.h"
			"ChangeGameDialog.h"
			"CreateMultiplayerGameDialog.h"
			"CreateMultiplayerGameGameplayPage.h"
			"CreateMultiplayerGameServerPage.h"
			"DemoEditDialog.h"
			"DemoEventsDialog.h"
			"DemoPlayerDialog.h"
			"DemoPlayerFileDialog.h"
			"GameConsole.h"
			"GameUI_Interface.h"
			"LoadingDialog.h"
			"ModInfo.h"
			"ScriptObject.h"
			"Sys_Utils.h"
			"Taskbar.h"
			"TaskButton.h"
			"taskframe.h"
			"VGuiSystemModuleLoader.h"
		#}
	)
END_SRC( GAMEUI_HEADER_FILES "Header Files" )

add_object(
	TARGET GameUI
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin" #"${GAMEDIR}/hl2/bin"
	SOURCES ${GAMEUI_SOURCE_FILES} ${GAMEUI_HEADER_FILES}
)

target_include_directories(
	GameUI PRIVATE

	"${SRCDIR}/Tracker/common"
)

target_compile_definitions(
	GameUI PRIVATE

	GAMEUI_EXPORTS
)

target_link_libraries(
	GameUI PRIVATE

	"$<${IS_WINDOWS}:Shlwapi>"
	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
	"$<${IS_WINDOWS}:wsock32>"

	vgui_controls
)

