cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( HLDS_GUI_SOURCE_FILES )
BEGIN_SRC( HLDS_GUI_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp"

			"CreateMultiplayerGameServerPage.cpp"
			"gameserver.cpp"
			"hlds.cpp"
			"hlds_gui.cpp"
			"vinternetdlg.cpp"
		#}
	)
END_SRC( HLDS_GUI_SOURCE_FILES "Source Files" )

set( HLDS_GUI_HEADER_FILES )
BEGIN_SRC( HLDS_GUI_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.h"

			"CreateMultiplayerGameServerPage.h"
			"gameserver.h"
			"hlds.h"
			"hlds_gui.h"
			"vinternetdlg.h"
		#}
	)
END_SRC( HLDS_GUI_HEADER_FILES "Header Files" )

add_object(
	TARGET hlds_gui
	MODULE
	PROPERTY_FOLDER "Tracker//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/platform/servers"
	INSTALL_OUTNAME "Server"
	SOURCES ${HLDS_GUI_SOURCE_FILES} ${HLDS_GUI_HEADER_FILES}
)

target_include_directories(
	hlds_gui PRIVATE

	"${SRCDIR}/Tracker/Public"
	"${SRCDIR}/Tracker/common"
	"${SRCDIR}/vgui2/controls"
	"${SRCDIR}/vgui2/include"
)

target_compile_definitions(
	hlds_gui PRIVATE

	SERVERBROWSER_EXPORTS
)

target_link_libraries(
	hlds_gui PRIVATE

	"$<${IS_WINDOWS}:wsock32>"

	vgui_controls
)

