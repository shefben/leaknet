cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( GAME_CONTROLS_SOURCE_FILES )
BEGIN_SRC( GAME_CONTROLS_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"classmenu.cpp"
			"clientmotd.cpp"
			"ClientScoreBoardDialog.cpp"
			"commandmenu.cpp"
			"MapOverview.cpp"
			"SpectatorGUI.cpp"
			"teammenu.cpp"
			"vguitextwindow.cpp"
			"vgui_TeamFortressViewport.cpp"
			"vgui_TeamFortressViewport_msgs.cpp"
		#}
	)
END_SRC( GAME_CONTROLS_SOURCE_FILES "Source Files" )

set( GAME_CONTROLS_HEADER_FILES )
BEGIN_SRC( GAME_CONTROLS_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/cl_dll/IVGuiClientDll.h"
			"${SRCDIR}/public/game_controls/classmenu.h"
			"${SRCDIR}/public/game_controls/clientmotd.h"
			"${SRCDIR}/public/game_controls/ClientScoreBoardDialog.h"
			"${SRCDIR}/public/game_controls/iscoreboardinterface.h"
			"${SRCDIR}/public/game_controls/ispectatorinterface.h"
			"${SRCDIR}/public/game_controls/iviewport.h"
			"${SRCDIR}/public/game_controls/iviewportmsgs.h"
			"${SRCDIR}/public/game_controls/MapOverview.h"
			"${SRCDIR}/public/game_controls/mouseoverhtmlbutton.h"
			"${SRCDIR}/public/game_controls/mouseoverpanelbutton.h"
			"${SRCDIR}/public/game_controls/SpectatorGUI.h"
			"${SRCDIR}/public/game_controls/teammenu.h"
			"${SRCDIR}/public/game_controls/utlqueue.h"
			"${SRCDIR}/public/game_controls/vguitextwindow.h"
			"${SRCDIR}/public/game_controls/vgui_TeamFortressViewport.h"
			#"MiniMap.h"
		#}
	)
END_SRC( GAME_CONTROLS_HEADER_FILES "Header Files" )

add_object(
	TARGET game_controls
	STATIC
	SOURCES ${GAME_CONTROLS_SOURCE_FILES} ${GAME_CONTROLS_HEADER_FILES}
)

target_include_directories(
	game_controls PRIVATE

	"${SRCDIR}/cl_dll"
	"${SRCDIR}/game_shared"
)