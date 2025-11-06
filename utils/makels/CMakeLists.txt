cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( MAKELS_SOURCE_FILES )
BEGIN_SRC( MAKELS_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"makels.cpp"
		#}
	)
END_SRC( MAKELS_SOURCE_FILES "Source Files" )

add_object(
	TARGET makels
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${MAKELS_SOURCE_FILES}
)