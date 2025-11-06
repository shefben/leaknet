#=============================================================================
#	windows_base.cmake
#=============================================================================

add_compile_definitions(
	WIN32
	_WIN32
	_WINDOWS
)

if( MSVC )
	include( "${CMAKE_CURRENT_LIST_DIR}/msvc_base.cmake" )
endif()

list(
	APPEND ADDITIONAL_LINK_LIBRARIES_EXE
	tier0
	vstdlib
)

list(
	APPEND ADDITIONAL_LINK_LIBRARIES_DLL
	tier0
	vstdlib
)
