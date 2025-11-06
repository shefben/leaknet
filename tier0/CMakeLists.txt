cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( TIER0_SOURCE_FILES )
BEGIN_SRC( TIER0_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"assert_dialog.cpp"
			"assert_dialog.rc"
			"resource.h"
			"cpu.cpp"
			"$<${IS_LINUX}:cpu_linux.cpp>"
			"dbg.cpp"
			"fasttimer.cpp"
			"mem.cpp"
			"memdbg.cpp"
			"memstd.cpp"
			"memvalidate.cpp"
			"$<${IS_WINDOWS}:platform.cpp>"
			"$<${IS_LINUX}:platform_linux.cpp>"
			"$<${IS_WINDOWS}:security.cpp>"
			"$<${IS_LINUX}:security_linux.cpp>"
			"$<${IS_WINDOWS}:thread.cpp>"
			"$<${IS_WINDOWS}:vcrmode.cpp>"
			"$<${IS_LINUX}:vcrmode_linux.cpp>"
			"vprof.cpp"
		#}
	)
END_SRC( TIER0_SOURCE_FILES "Source Files" )

BEGIN_SRC( TIER0_SOURCE_FILES "DESKey" )
	SRC_GRP(
		SOURCES
		#{
			"DESKey/ALGO.H"
			"DESKey/DK2WIN32.H"
			"DESKey/ALGO32.LIB"
			"DESKey/DK2WIN32.LIB"
		#}
	)
END_SRC( TIER0_SOURCE_FILES "DESKey" )

set( TIER0_HEADER_FILES )
BEGIN_SRC( TIER0_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/mem.h"
			"${SRCDIR}/public/tier0/memalloc.h"
			"${SRCDIR}/public/tier0/memdbgoff.h"
			"${SRCDIR}/public/tier0/memdbgon.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/tier0/vcrmode.h"
			"${SRCDIR}/public/tier0/vcr_shared.h"
			"${SRCDIR}/public/tier0/vprof.h"
		#}
	)
END_SRC( TIER0_HEADER_FILES "Header Files" )

add_object(
	TARGET tier0
	SHARED
	INSTALL_DEST "${GAMEDIR}/bin"
	EXCLUDE_LIB tier0 vstdlib
	EXCLUDE_SOURCE "${SRCDIR}/public/tier0/memoverride.cpp"
	SOURCES ${TIER0_SOURCE_FILES} ${TIER0_HEADER_FILES}
)

target_include_directories(
	tier0 PRIVATE

	"${SRCDIR}/public/tier0"
)

target_compile_definitions(
	tier0 PRIVATE

	TIER0_DLL_EXPORT
)

target_link_libraries(
	tier0 PRIVATE

	"$<${IS_WINDOWS}:ws2_32>"
)

