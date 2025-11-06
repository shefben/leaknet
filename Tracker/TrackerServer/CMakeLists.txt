cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( TRACKERSERVER_SOURCE_FILES )
BEGIN_SRC( TRACKERSERVER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp"

			"TrackerServer.cpp"
		#}
	)
END_SRC( TRACKERSERVER_SOURCE_FILES "Source Files" )

add_object(
	TARGET TrackerServer
	EXECUTABLE WIN32
	PROPERTY_FOLDER "Tracker//Executables"
	INSTALL_DEST "${GAMEDIR}/TrackerServer"
	SOURCES ${TRACKERSERVER_SOURCE_FILES}
)

target_link_libraries(
	TrackerServer PRIVATE

	"$<${IS_WINDOWS}:ws2_32>"
	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
)
