set( FILESYSTEM_STDIO_DIR ${CMAKE_CURRENT_LIST_DIR} )

set( FILESYSTEM_STDIO_SOURCE_FILES )
BEGIN_SRC( FILESYSTEM_STDIO_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
    #<ClCompile Include="${SRCDIR}/common/Steam.c">
    #  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">true</ExcludedFromBuild>
    #  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">true</ExcludedFromBuild>
    #</ClCompile>
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/interface.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"${FILESYSTEM_STDIO_DIR}/BaseFileSystem.cpp"
			"${FILESYSTEM_STDIO_DIR}/filesystem_stdio.cpp" # !STEAM
    #<ClCompile Include="${FILESYSTEM_STDIO_DIR}/filesystem_steam.cpp">
    #  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">true</ExcludedFromBuild>
    #  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">true</ExcludedFromBuild>
    #</ClCompile>
			"$<${IS_LINUX}:${FILESYSTEM_STDIO_DIR}/linux_support.cpp>"
		#}
	)
END_SRC( FILESYSTEM_STDIO_SOURCE_FILES "Source Files" )

set( FILESYSTEM_STDIO_HEADER_FILES )
BEGIN_SRC( FILESYSTEM_STDIO_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
    #<ClInclude Include="${SRCDIR}/common/Steam.h">
    #  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">true</ExcludedFromBuild>
    #  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">true</ExcludedFromBuild>
    #</ClInclude>
    #<ClInclude Include="${SRCDIR}/common/SteamCommon.h">
    #  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">true</ExcludedFromBuild>
    #  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">true</ExcludedFromBuild>
    #</ClInclude>
    #<ClInclude Include="${SRCDIR}/common/Steamlib.h">
    #  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">true</ExcludedFromBuild>
    #  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">true</ExcludedFromBuild>
    #</ClInclude>
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

			"${FILESYSTEM_STDIO_DIR}/BaseFileSystem.h"
			"$<${IS_LINUX}:${FILESYSTEM_STDIO_DIR}/linux_support.h>"
		#}
	)
END_SRC( FILESYSTEM_STDIO_HEADER_FILES "Header Files" )

add_object(
	TARGET filesystem_stdio
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${FILESYSTEM_STDIO_SOURCE_FILES} ${FILESYSTEM_STDIO_HEADER_FILES}
)

target_include_directories(
	filesystem_stdio PRIVATE
)

target_compile_definitions(
	filesystem_stdio PRIVATE

	FILESYSTEM_STDIO_EXPORTS
	DONT_PROTECT_FILEIO_FUNCTIONS
	PROTECTED_THINGS_ENABLE
)

target_link_libraries(
	filesystem_stdio PRIVATE
)

