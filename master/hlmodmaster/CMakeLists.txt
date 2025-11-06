cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( HLMODMASTER_SOURCE_FILES )
BEGIN_SRC( HLMODMASTER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"HLModMaster.cpp"
			"HLModMasterDlg.cpp"
			"StdAfx.cpp"
			"Token.cpp"

			"HLModMaster.rc"
		#}
	)
END_SRC( HLMODMASTER_SOURCE_FILES "Source Files" )

set( HLMODMASTER_HEADER_FILES )
BEGIN_SRC( HLMODMASTER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"HLModMaster.h"
			"HLModMasterDlg.h"
			"HLModMasterProtocol.h"

			"resource.h"
			"StdAfx.h"
			"Token.h"
		#}
	)
END_SRC( HLMODMASTER_HEADER_FILES "Header Files" )

add_object(
	TARGET hlmodmaster
	EXECUTABLE WIN32
	PROPERTY_FOLDER "Master//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${HLMODMASTER_SOURCE_FILES} ${HLMODMASTER_HEADER_FILES}
)

target_include_directories(
	hlmodmaster PRIVATE

	#"${SRCDIR}/engine"
)

target_link_libraries(
	hlmodmaster PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
	"$<${IS_WINDOWS}:wsock32>"
)
