cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( TRACKERSRV_SOURCE_FILES )
BEGIN_SRC( TRACKERSRV_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/mempool.cpp" # Noprecomp?
			"${SRCDIR}/common/GameUI/Random.cpp" # Noprecomp?
			"${SRCDIR}/public/interface.cpp" # Noprecomp?
			"${SRCDIR}/Tracker/common/CompletionEvent.cpp" # Noprecomp?
			"${SRCDIR}/Tracker/TrackerNET/NetAddress.cpp" # Noprecomp?

			"DebugTimer.cpp"
			"LocalDebugconsole.cpp" # Noprecomp?
			"PrecompHeader.cpp" # Create precomp: PrecompHeader.h
			"SessionManager.cpp" # Noprecomp?
			"TopologyManager.cpp" # Noprecomp?
			"TrackerDatabaseManager.cpp"
			"TrackerSRV.cpp" # Noprecomp?
			"UserSession.cpp" # Noprecomp?
		#}
	)
END_SRC( TRACKERSRV_SOURCE_FILES "Source Files" )

set( TRACKERSRV_HEADER_FILES )
BEGIN_SRC( TRACKERSRV_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/GameUI/Random.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/Tracker/common/CompletionEvent.h"
			"${SRCDIR}/Tracker/common/DebugConsole_Interface.h"
			"${SRCDIR}/Tracker/common/TrackerProtocol.h"
			"${SRCDIR}/Tracker/common/UtlMsgBuffer.h"
			"${SRCDIR}/Tracker/Public/ITrackerDistroDatabase.h"
			"${SRCDIR}/Tracker/Public/ITrackerUserDatabase.h"
			"${SRCDIR}/Tracker/Public/TrackerCmdID.h"

			"DebugTimer.h"
			"LocalDebugConsole.h"
			"PrecompHeader.h"
			"SessionManager.h"
			"TopologyManager.h"
			"TrackerDatabaseManager.h"
			"TrackerSRV_Interface.h"
			"UserSession.h"
		#}
	)
END_SRC( TRACKERSRV_HEADER_FILES "Header Files" )

add_object(
	TARGET TrackerSRV
	MODULE
	PROPERTY_FOLDER "Tracker//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/TrackerServer"
	SOURCES ${TRACKERSRV_SOURCE_FILES} ${TRACKERSRV_HEADER_FILES}
)

target_precompile_headers(
	TrackerSRV
	PRIVATE "PrecompHeader.h"
)

target_include_directories(
	TrackerSRV PRIVATE

	"${SRCDIR}/Tracker/Public"
	"${SRCDIR}/Tracker/common"
	"${SRCDIR}/utils/mysql/include"
	"${SRCDIR}/utils/mysql/mysql/include"
)

target_compile_definitions(
	TrackerSRV PRIVATE

	TRACKERSRV_EXPORTS
)
