cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( TRACKERTEST_SOURCE_FILES )
BEGIN_SRC( TRACKERTEST_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			#"${SRCDIR}/common/mempool.cpp"
			#"${SRCDIR}/common/Random.cpp"
			"${SRCDIR}/public/interface.cpp"

			"DebugTimer.cpp"
			"keyvalues.cpp"
			"LoadTestApp.cpp"
			"main.cpp"
			"User.cpp"
			"UserDatabase.cpp"
		#}
	)
END_SRC( TRACKERTEST_SOURCE_FILES "Source Files" )

set( TRACKERTEST_HEADER_FILES )
BEGIN_SRC( TRACKERTEST_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			#"${SRCDIR}/common/mempool.h"
			#"${SRCDIR}/common/Random.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/Tracker/common/DebugTimer.h"

			"keyvalues.h"
			"LoadTestApp.h"
			"User.h"
			"UserDatabase.h"
		#}
	)
END_SRC( TRACKERTEST_HEADER_FILES "Header Files" )

add_object(
	TARGET TrackerTest
	EXECUTABLE WIN32
	PROPERTY_FOLDER "Tracker//Executables"
	INSTALL_DEST "${GAMEDIR}/TrackerServer"
	SOURCES ${TRACKERTEST_SOURCE_FILES} ${TRACKERTEST_HEADER_FILES}
)

target_link_libraries(
	TrackerTest PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
)
