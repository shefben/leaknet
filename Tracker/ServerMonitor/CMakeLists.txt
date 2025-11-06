cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( DEDICATED_SOURCE_FILES )
BEGIN_SRC( DEDICATED_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/Tracker/TrackerNET/NetAddress.cpp"

			"Main.cpp"
			"MainDialog.cpp"
			"ServerInfoPanel.cpp"
			"ServerList.cpp"
			"ServersPage.cpp"
			"UsersPage.cpp"
		#}
	)
END_SRC( DEDICATED_SOURCE_FILES "Source Files" )

set( DEDICATED_HEADER_FILES )
BEGIN_SRC( DEDICATED_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/Tracker/TrackerNET/NetAddress.h"

			"MainDialog.h"
			"ServerInfoPanel.h"
			"ServerList.h"
			"ServersPage.h"
			"UsersPage.h"
		#}
	)
END_SRC( DEDICATED_HEADER_FILES "Header Files" )

add_object(
	TARGET ServerMonitor
	EXECUTABLE WIN32
	PROPERTY_FOLDER "Tracker//Executables"
	INSTALL_DEST "${GAMEDIR}/platform/bin"
	SOURCES ${DEDICATED_SOURCE_FILES} ${DEDICATED_HEADER_FILES}
)

target_include_directories(
	ServerMonitor PRIVATE

	"${SRCDIR}/Tracker/common"
	"${SRCDIR}/vgui2/controls"
	"${SRCDIR}/vgui2/include"
)

target_link_libraries(
	ServerMonitor PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
	"$<${IS_WINDOWS}:ws2_32>"

	vgui_controls
)
