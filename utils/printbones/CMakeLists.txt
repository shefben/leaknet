cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( PRINTBONES_SOURCE_FILES )
BEGIN_SRC( PRINTBONES_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"printbones.cpp"
		#}
	)
END_SRC( PRINTBONES_SOURCE_FILES "Source Files" )

add_object(
	TARGET printbones
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${PRINTBONES_SOURCE_FILES}
)

target_include_directories(
	printbones PRIVATE

	"${SRCDIR}/utils/common"
)