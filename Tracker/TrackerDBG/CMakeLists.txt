cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( TRACKERDBG_SOURCE_FILES )
BEGIN_SRC( TRACKERDBG_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp"

			"TrackerDBG.cpp"
		#}
	)
END_SRC( TRACKERDBG_SOURCE_FILES "Source Files" )

set( TRACKERDBG_HEADER_FILES )
BEGIN_SRC( TRACKERDBG_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.h"

			"${SRCDIR}/Tracker/common/DebugConsole_Interface.h"
		#}
	)
END_SRC( TRACKERDBG_HEADER_FILES "Header Files" )

add_object(
	TARGET TrackerDBG
	MODULE
	PROPERTY_FOLDER "Tracker//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/TrackerServer"
	SOURCES ${TRACKERDBG_SOURCE_FILES} ${TRACKERDBG_HEADER_FILES}
)

target_include_directories(
	TrackerDBG PRIVATE

	"${SRCDIR}/Tracker/Public"
	"${SRCDIR}/Tracker/common"
)

target_compile_definitions(
	TrackerDBG PRIVATE

	TRACKERDBG_EXPORTS
)

target_link_libraries(
	TrackerDBG PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
)

