#=============================================================================
# parse_git_info.cmake
#
# Based on https://jonathanhamberg.com/post/cmake-embedding-git-hash/
#=============================================================================

include_guard( GLOBAL )

find_program( GIT_EXE git )

if( NOT GIT_EXE MATCHES "NOTFOUND" )
	# Store current git hash in GIT_COMMIT_HASH
	execute_process(
		COMMAND ${GIT_EXE} describe --dirty --always
		WORKING_DIRECTORY ${SRCDIR}
		OUTPUT_VARIABLE GIT_COMMIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	# Store current git branch in GIT_CURRENT_BRANCH
	execute_process(
		COMMAND ${GIT_EXE} rev-parse --abbrev-ref HEAD
			WORKING_DIRECTORY ${SRCDIR}
		OUTPUT_VARIABLE GIT_CURRENT_BRANCH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
else()
	set( GIT_COMMIT_HASH "00000000" )
	set( GIT_CURRENT_BRANCH "unknown" )

	message( STATUS "Failed to find \"git\" executable!" )
endif()

message( STATUS "Git info: (${GIT_CURRENT_BRANCH} @ ${GIT_COMMIT_HASH})" )

configure_file( "${SRCDIR}/common/git_info.h.in" "${SRCDIR}/common/git_info.h" @ONLY )

set_property( DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "common/git_info.h.in" )
