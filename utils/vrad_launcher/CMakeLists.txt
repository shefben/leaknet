cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VRAD_LAUNCHER_SOURCE_FILES )
BEGIN_SRC( VRAD_LAUNCHER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp" # Noprecomp?

			"StdAfx.cpp" # Create precomp: stdafx.h

			"vrad_launcher.cpp"
		#}
	)
END_SRC( VRAD_LAUNCHER_SOURCE_FILES "Source Files" )

set( VRAD_LAUNCHER_HEADER_FILES )
BEGIN_SRC( VRAD_LAUNCHER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/ivraddll.h"

			"${SRCDIR}/public/interface.h"

			"StdAfx.h"
		#}
	)
END_SRC( VRAD_LAUNCHER_HEADER_FILES "Header Files" )

add_object(
	TARGET vrad_launcher
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	INSTALL_OUTNAME "vrad"
	SOURCES ${VRAD_LAUNCHER_SOURCE_FILES} ${VRAD_LAUNCHER_HEADER_FILES}
)

target_precompile_headers(
	vrad_launcher
	PRIVATE "stdafx.h"
)

target_include_directories(
	vrad_launcher PRIVATE

	"${SRCDIR}/utils/common"
)

target_link_libraries(
	vrad_launcher PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
)