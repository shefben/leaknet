cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( HLMODMASTER_SOURCE_FILES )
BEGIN_SRC( HLMODMASTER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/info_key.cpp"

			"HLMasterAsyncSocket.cpp"
			"mastertest.cpp"
			"mastertestDlg.cpp"
			"MessageBuffer.cpp"
			"StdAfx.cpp"
			"UTIL.cpp"

			"mastertest.rc"
			"res/mastertest.ico"
			"res/mastertest.rc2"
			"ReadMe.txt"
		#}
	)
END_SRC( HLMODMASTER_SOURCE_FILES "Source Files" )

set( HLMODMASTER_HEADER_FILES )
BEGIN_SRC( HLMODMASTER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"../info.h"

			"HLMasterAsyncSocket.h"
			"mastertest.h"
			"mastertestDlg.h"
			"MessageBuffer.h"
			"Resource.h"
			"StdAfx.h"
		#}
	)
END_SRC( HLMODMASTER_HEADER_FILES "Header Files" )

add_object(
	TARGET mastertest
	EXECUTABLE WIN32
	PROPERTY_FOLDER "Master//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${HLMODMASTER_SOURCE_FILES} ${HLMODMASTER_HEADER_FILES}
)

target_precompile_headers(
	mastertest
	PRIVATE "StdAfx.h"
)

target_include_directories(
	mastertest PRIVATE

	#"${SRCDIR}/engine"
)

#target_link_libraries(
#	mastertest PRIVATE
#
#	"$<${IS_WINDOWS}:odbc32>"
#	"$<${IS_WINDOWS}:odbccp32>"
#	"$<${IS_WINDOWS}:wsock32>"
#)
