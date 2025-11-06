cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

# @TODO(SanyaSho): 
set( STEAM_MASTER 0 )

set( HLMASTER_SOURCE_FILES )
BEGIN_SRC( HLMASTER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"crc.cpp"
			"HLMaster.cpp"
			"HLMasterDlg.cpp"
			"info.cpp"
			"StdAfx.cpp"
			"Token.cpp"

			"HLMaster.rc"
			"res/HLMaster.ico"
			"res/HLMaster.rc2"
		#}
	)
END_SRC( HLMASTER_SOURCE_FILES "Source Files" )

set( HLMASTER_HEADER_FILES )
BEGIN_SRC( HLMASTER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/proto_oob.h"
			"${SRCDIR}/common/proto_version.h"
			"${SRCDIR}/public/archtypes.h"

			"crc.h"
			"HLMaster.h"
			"HLMasterDlg.h"
			"HLMasterProtocol.h"
			"info.h"
			"Resource.h"
			"StdAfx.h"
			"Token.h"
		#}
	)
END_SRC( HLMASTER_HEADER_FILES "Header Files" )

add_object(
	TARGET hlmaster
	EXECUTABLE WIN32
	PROPERTY_FOLDER "Master//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${HLMASTER_SOURCE_FILES} ${HLMASTER_HEADER_FILES}
)

target_include_directories(
	hlmaster PRIVATE

	#"${SRCDIR}/engine"
)

target_compile_definitions(
	hlmaster PRIVATE

	"$<${STEAM_MASTER}:_STEAM_HLMASTER>"
)

target_link_libraries(
	hlmaster PRIVATE

	"$<${IS_WINDOWS}:wsock32>"
)
