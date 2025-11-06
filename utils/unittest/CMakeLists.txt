cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( UNITTEST_SOURCE_FILES )
BEGIN_SRC( UNITTEST_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"unittest.cpp"
		#}
	)
END_SRC( UNITTEST_SOURCE_FILES "Source Files" )

add_object(
	TARGET unittest
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${UNITTEST_SOURCE_FILES}
)

target_include_directories(
	unittest PRIVATE

	"${SRCDIR}/utils/common"
)

target_compile_definitions(
	unittest PRIVATE

	PROTECTED_THINGS_DISABLE
)

target_link_libraries(
	unittest PRIVATE

	unitlib
)