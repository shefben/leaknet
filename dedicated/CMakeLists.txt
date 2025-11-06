cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( DEDICATED_SOURCE_FILES )
BEGIN_SRC( DEDICATED_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/mathlib.cpp"

			"$<${IS_WINDOWS}:conproc.cpp>"
			"filesystem.cpp"
			"sys_common.cpp"
			"$<${IS_LINUX}:sys_linux.cpp>"
			"$<${IS_WINDOWS}:sys_windows.cpp>"

			"dedicated.rc"
			"netgame.ico"
		#}
	)
END_SRC( DEDICATED_SOURCE_FILES "Source Files" )

set( DEDICATED_HEADER_FILES )
BEGIN_SRC( DEDICATED_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/convar.h"
			"${SRCDIR}/public/dll_state.h"
			"${SRCDIR}/public/engine_launcher_api.h"
			"${SRCDIR}/public/exefuncs.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/icvar.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/tier0/vcrmode.h"

			"conproc.h"
			"dedicated.h"
			"ifilesystem.h"
			"isys.h"
			"resource.h"
		#}
	)
END_SRC( DEDICATED_HEADER_FILES "Header Files" )

add_object(
	TARGET dedicated
	EXECUTABLE WIN32
	INSTALL_DEST "${GAMEDIR}/bin"
	INSTALL_OUTNAME "hlds"
	SOURCES ${DEDICATED_SOURCE_FILES} ${DEDICATED_HEADER_FILES}
)

target_include_directories(
	dedicated PRIVATE

	"${SRCDIR}/engine"
)

target_compile_definitions(
	dedicated PRIVATE

	DEDICATED
	LAUNCHERONLY
)

target_link_libraries(
	dedicated PRIVATE

	"$<${IS_WINDOWS}:wsock32>"
	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
	"$<${IS_WINDOWS}:winmm>"

	"$<${IS_LINUX}:tier0>"
	"$<${IS_LINUX}:vstdlib>"
)
