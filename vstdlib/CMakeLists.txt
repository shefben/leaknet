cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VSTDLIB_DIR ${CMAKE_CURRENT_LIST_DIR} )

set( VSTDLIB_SOURCE_FILES )
BEGIN_SRC( VSTDLIB_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/mempool.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"commandline.cpp"
			"KeyValuesSystem.cpp"
			"random.cpp"
			"strtools.cpp"
		#}
	)
END_SRC( VSTDLIB_SOURCE_FILES "Source Files" )

set( VSTDLIB_HEADER_FILES )
BEGIN_SRC( VSTDLIB_HEADER_FILES "Public Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/vstdlib/icommandline.h"
			"${SRCDIR}/public/vstdlib/IKeyValuesSystem.h"
			"${SRCDIR}/public/vstdlib/random.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"
		#}
	)
END_SRC( VSTDLIB_HEADER_FILES "Public Header Files" )

add_object(
	TARGET vstdlib
	SHARED
	EXCLUDE_LIB vstdlib
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VSTDLIB_SOURCE_FILES} ${VSTDLIB_HEADER_FILES}
)

target_compile_definitions(
	vstdlib PRIVATE

	VSTDLIB_DLL_EXPORT
)
