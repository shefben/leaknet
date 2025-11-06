cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( BUGREPORTER_SOURCE_FILES )
BEGIN_SRC( BUGREPORTER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/filesystem_tools.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/keyvalues.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"bugreporter.cpp"
		#}
	)
END_SRC( BUGREPORTER_SOURCE_FILES "Source Files" )

set( BUGREPORTER_HEADER_FILES )
BEGIN_SRC( BUGREPORTER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/bugreporter/bugreporter.h"
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/Color.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/filesystem_tools.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/keyvalues.h"
			"${SRCDIR}/public/protected_things.h"
			"${SRCDIR}/public/string_t.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/mem.h"
			"${SRCDIR}/public/tier0/memdbgoff.h"
			"${SRCDIR}/public/tier0/memdbgon.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utldict.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlsymbol.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vstdlib/IKeyValuesSystem.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"

			"trktool.h"
		#}
	)
END_SRC( BUGREPORTER_HEADER_FILES "Header Files" )

add_object(
	TARGET bugreporter
	MODULE
	PROPERTY_FOLDER "Utils//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${BUGREPORTER_SOURCE_FILES} ${BUGREPORTER_HEADER_FILES}
)

target_compile_definitions(
	bugreporter PRIVATE

	BUGREPORTER_EXPORTS
)

target_link_libraries(
	bugreporter PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"

	"$<${IS_WINDOWS}:${LIBPUBLIC}/expdlln.lib>"
	"$<${IS_WINDOWS}:${LIBPUBLIC}/trktooln.lib>"
)
