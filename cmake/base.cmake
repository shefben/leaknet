#=============================================================================
# base.cmake
#
# $
#=============================================================================

# NOTE: We use 0 or 1 so we can use these more easily in generator expressions
# Initialize them with default values that we then set later

set( IS_64BIT		0 )
set( IS_DEDICATED	0 )

# Platforms
set( IS_WINDOWS		0 )
set( IS_POSIX		0 )
set( IS_LINUX		0 )
set( IS_OSX		0 )
set( IS_XCODE 		0 )

if( WIN32 )
	set( IS_WINDOWS	1 )
endif()

if( UNIX )
	set( IS_POSIX 1 )

	if( NOT APPLE )
		set( IS_LINUX 1 )
	elseif( APPLE )
		set( IS_OSX 1 )
		if( ${CMAKE_GENERATOR} STREQUAL "Xcode" )
			set( IS_XCODE 1 )
		endif()
	else()
		message( FATAL_ERROR "Failed to determine UNIX platform!" )
	endif()
endif()

if( ${IS_WINDOWS} )
	set( _DLL_EXT ".dll" )
	set( STATIC_LIB_EXT ".lib" )
	set( IMPLIB_EXT ".lib" )
elseif( ${IS_LINUX} )
	set( _DLL_EXT ".so" )
	set( STATIC_LIB_EXT ".a" )
	set( IMPLIB_EXT ".so" )
elseif( ${IS_OSX} )
	set( _DLL_EXT ".dylib" )
	set( STATIC_LIB_EXT ".a" )
	set( IMPLIB_EXT ".dylib" )
endif()


message( "=============================================================================" )

message( STATUS "GAMEDIR: ${GAMEDIR}" )

# Set this to ON to supress all warnings (POSIX-only for now)
option( DISABLE_WARNINGS "Disable warnings" OFF )
message( STATUS "DISABLE_WARNINGS: ${DISABLE_WARNINGS}" )

option( DEDICATED "Build dedicated" OFF )
message( STATUS "DEDICATED: ${DEDICATED}" )
if( DEDICATED )
	set( IS_DEDICATED 1 )
	set( BUILD_GROUP "dedicated" )
endif()

option( BUILD_64BIT "64 bit build of engine" OFF )
message( STATUS "BUILD_64BIT: ${BUILD_64BIT}" )
if( BUILD_64BIT )
	set( IS_64BIT 1 )
endif()

if( NOT IS_64BIT AND CMAKE_SIZEOF_VOID_P EQUAL 8 )
	message( FATAL_ERROR "Forgot to add -DBUILD_64BIT=ON!" )
endif()

include( "${SRCDIR}/cmake/parse_git_info.cmake" )

set( GAMES_TO_BUILD "cstrike,hl2,hl1,tf2" CACHE STRING "Games which will be built" )
set_property( CACHE GAMES_TO_BUILD PROPERTY STRINGS "cstrike" "hl2" "hl1" "tf2" )
message( STATUS "GAMES_TO_BUILD: ${GAMES_TO_BUILD}" )
string( REPLACE "," ";" GAMES_TO_BUILD_LIST ${GAMES_TO_BUILD} )

message( "=============================================================================" )


list(
	APPEND ADDITIONAL_SOURCES_EXE
	"${SRCDIR}/public/tier0/memoverride.cpp"
)

list(
	APPEND ADDITIONAL_SOURCES_DLL
	"${SRCDIR}/public/tier0/memoverride.cpp"
)


# CMAKETODO(SanyaSho): windows support
if( ${IS_POSIX} )
	find_program( CCACHE ccache )

	if( NOT CCACHE MATCHES "NOTFOUND" )
		# https://github.com/llvm/llvm-project/blob/main/llvm/CMakeLists.txt#L239-L267
		set( CCACHE_COMMAND "CCACHE_DIR=\"${SRCDIR}/.ccache\" ${CCACHE}" )

		set_property(
			GLOBAL
			PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_COMMAND}"
		)

		# CMAKETODO(SanyaSho): 'Error running link command: No such file or directory'
		#set_property(
		#	GLOBAL
		#	PROPERTY RULE_LAUNCH_LINK "${CCACHE_COMMAND}"
		#)
	else()
		message( STATUS "Could not find ccache executable!" )
	endif()
endif()

list(
	APPEND ADDITIONAL_INCLUDES_EXE
	"${SRCDIR}/common"
	"${SRCDIR}/public"
	"${SRCDIR}/public/tier0"
)

list(
	APPEND ADDITIONAL_INCLUDES_DLL
	"${SRCDIR}/common"
	"${SRCDIR}/public"
	"${SRCDIR}/public/tier0"
)

list(
	APPEND ADDITIONAL_INCLUDES_LIB
	"${SRCDIR}/common"
	"${SRCDIR}/public"
	"${SRCDIR}/public/tier0"
)

add_compile_definitions(
	# Some shared values
	VPROF_LEVEL=4

	_DLL_EXT=${_DLL_EXT}
	FRAME_POINTER_OMISSION_DISABLED
)
