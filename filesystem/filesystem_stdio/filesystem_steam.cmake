set( FILESYSTEM_STEAM_DIR ${CMAKE_CURRENT_LIST_DIR} )

set( FILESYSTEM_STEAM_SOURCE_FILES )
BEGIN_SRC( FILESYSTEM_STEAM_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/Steam.c"
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/interface.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"${FILESYSTEM_STEAM_DIR}/BaseFileSystem.cpp"
			"${FILESYSTEM_STEAM_DIR}/filesystem_steam.cpp"

			"$<${IS_LINUX}:${FILESYSTEM_STDIO_DIR}/linux_support.cpp>"
		#}
	)
END_SRC( FILESYSTEM_STEAM_SOURCE_FILES "Source Files" )

set( FILESYSTEM_STEAM_HEADER_FILES )
BEGIN_SRC( FILESYSTEM_STEAM_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/Steam.h"
			"${SRCDIR}/common/SteamCommon.h"
			"${SRCDIR}/common/Steamlib.h"
			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/bspfile.h"
			"${SRCDIR}/public/bspflags.h"
			"${SRCDIR}/public/bumpvects.h"
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/mathlib.h"
			"${SRCDIR}/public/plane.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlsymbol.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vector.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vector4d.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"
			"${FILESYSTEM_STEAM_DIR}/BaseFileSystem.h"
		#}
	)
END_SRC( FILESYSTEM_STEAM_HEADER_FILES "Header Files" )

add_object(
	TARGET filesystem_steam
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${FILESYSTEM_STEAM_SOURCE_FILES} ${FILESYSTEM_STEAM_HEADER_FILES}
)

target_include_directories(
	filesystem_steam PRIVATE
)

target_compile_definitions(
	filesystem_steam PRIVATE

	FILESYSTEM_STEAM_EXPORTS
	DONT_PROTECT_FILEIO_FUNCTIONS
	PROTECTED_THINGS_ENABLE
)

target_link_libraries(
	filesystem_steam PRIVATE

	"${LIBCOMMON}/Steam.lib"
)

