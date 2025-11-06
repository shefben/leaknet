cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( BROWSER_SOURCE_FILES )
BEGIN_SRC( BROWSER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp"

			"browser.cpp"
			"vinternetdlg.cpp"
		#}
	)
END_SRC( BROWSER_SOURCE_FILES "Source Files" )

set( BROWSER_HEADER_FILES )
BEGIN_SRC( BROWSER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.h"

			"browser.h"
			"vinternetdlg.h"
		#}
	)
END_SRC( BROWSER_HEADER_FILES "Header Files" )

add_object(
	TARGET Browser
	EXECUTABLE WIN32
	PROPERTY_FOLDER "Tracker//Executables"
	INSTALL_DEST "${GAMEDIR}/platform/browser"
	SOURCES ${BROWSER_SOURCE_FILES} ${BROWSER_HEADER_FILES}
)

target_include_directories(
	Browser PRIVATE

	"${SRCDIR}/Tracker/Public"
	"${SRCDIR}/Tracker/common"
	"${SRCDIR}/vgui2/controls"
	"${SRCDIR}/vgui2/include"
	"${SRCDIR}/gameui"
)

target_compile_definitions(
	Browser PRIVATE

	BROWSER_EXPORTS
)

target_link_libraries(
	Browser PRIVATE

	"$<${IS_WINDOWS}:wsock32>"

	vgui_controls
)
