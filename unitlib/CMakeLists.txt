cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( UNITLIB_SOURCE_FILES )
BEGIN_SRC( UNITLIB_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"unitlib.cpp"
		#}
	)
END_SRC( UNITLIB_SOURCE_FILES "Source Files" )

set( UNITLIB_HEADER_FILES )
BEGIN_SRC( UNITLIB_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/unitlib/unitlib.h"
		#}
	)
END_SRC( UNITLIB_HEADER_FILES "Header Files" )

add_object(
	TARGET unitlib
	SHARED
	NO_LIB_PREFIX
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${UNITLIB_SOURCE_FILES} ${UNITLIB_HEADER_FILES}
)

target_compile_definitions(
	unitlib PRIVATE

	fopen=dont_use_fopen

	UNITLIB_DLL_EXPORT
)
