cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( PRINTDUMMY1_SOURCE_FILES )
BEGIN_SRC( PRINTDUMMY1_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"printdummy1.cpp"

			"eachmodel.bat"
		#}
	)
END_SRC( PRINTDUMMY1_SOURCE_FILES "Source Files" )

set( PRINTDUMMY1_HEADER_FILES )
BEGIN_SRC( PRINTDUMMY1_HEADER_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/studio.h"
		#}
	)
END_SRC( PRINTDUMMY1_HEADER_FILES "Source Files" )

add_object(
	TARGET printdummy1
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${PRINTDUMMY1_SOURCE_FILES} ${PRINTDUMMY1_HEADER_FILES}
)

target_include_directories(
	printdummy1 PRIVATE

	"${SRCDIR}/engine"
)