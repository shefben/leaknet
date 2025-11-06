cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( MAKEVMT_SOURCE_FILES )
BEGIN_SRC( MAKEVMT_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/filesystem_tools.cpp"
			"${SRCDIR}/public/interface.cpp"

			"${SRCDIR}/utils/common/cmdlib.cpp"

			"makevmt.cpp"
			"StdAfx.cpp" # Create precomp: StdAfx.h

			#"ReadMe.txt"
		#}
	)
END_SRC( MAKEVMT_SOURCE_FILES "Source Files" )

set( MAKEVMT_HEADER_FILES )
BEGIN_SRC( MAKEVMT_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/utils/common/cmdlib.h"

			"StdAfx.h"
		#}
	)
END_SRC( MAKEVMT_HEADER_FILES "Header Files" )

add_object(
	TARGET makevmt
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${MAKEVMT_SOURCE_FILES} ${MAKEVMT_HEADER_FILES}
)

target_precompile_headers(
	makevmt
	PRIVATE "StdAfx.h"
)

target_include_directories(
	makevmt PRIVATE

	"${SRCDIR}/utils/common"
)