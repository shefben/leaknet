cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VVIS_LAUNCHER_SOURCE_FILES )
BEGIN_SRC( VVIS_LAUNCHER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp" # Noprecomp?
			"StdAfx.cpp" # Create precomp: stdafx.h
			"vvis_launcher.cpp"
		#}
	)
END_SRC( VVIS_LAUNCHER_SOURCE_FILES "Source Files" )

set( VVIS_LAUNCHER_HEADER_FILES )
BEGIN_SRC( VVIS_LAUNCHER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.h"
			"StdAfx.h"
		#}
	)
END_SRC( VVIS_LAUNCHER_HEADER_FILES "Header Files" )

add_object(
	TARGET vvis_launcher
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	INSTALL_OUTNAME "vvis"
	EXCLUDE_LIB tier0
	SOURCES ${VVIS_LAUNCHER_SOURCE_FILES} ${VVIS_LAUNCHER_HEADER_FILES}
)

target_precompile_headers(
	vvis_launcher
	PRIVATE "stdafx.h"
)

target_include_directories(
	vvis_launcher PRIVATE

	"${SRCDIR}/utils/common"
)

target_link_libraries(
	vvis_launcher PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
)