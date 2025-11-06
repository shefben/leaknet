cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( TRACKERNET_SOURCE_FILES )
BEGIN_SRC( TRACKERNET_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"../../public/mempool.cpp"
			"../../common/GameUI/Random.cpp"
			"../../public/interface.cpp"
			"../common/CompletionEvent.cpp"
			"../common/IceKey.cpp"

			"BinaryBuffer.cpp"
			"NetAddress.cpp"
			"ReceiveMessage.cpp"
			"SendMessage.cpp"
			"Sockets.cpp"
			"ThreadedSocket.cpp"
			"threads.cpp"
			"TrackerNET.cpp"
		#}
	)
END_SRC( TRACKERNET_SOURCE_FILES "Source Files" )

set( TRACKERNET_HEADER_FILES )
BEGIN_SRC( TRACKERNET_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"../../public/mempool.h"
			"../../common/GameUI/Random.h"
			"../../public/interface.h"
			"../common/IceKey.H"
			"../common/winlite.h"

			"BinaryBuffer.h"
			"BufferAllocator.h"
			"NetAddress.h"
			"PacketHeader.h"
			"ReceiveMessage.h"
			"SendMessage.h"
			"Sockets.h"
			"ThreadedSocket.h"
			"threads.h"
			"TrackerNET_Interface.h"
		#}
	)
END_SRC( TRACKERNET_HEADER_FILES "Header Files" )

add_object(
	TARGET TrackerNET
	MODULE
	PROPERTY_FOLDER "Tracker//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/platform/friends"
	SOURCES ${TRACKERNET_SOURCE_FILES} ${TRACKERNET_HEADER_FILES}
)

target_include_directories(
	TrackerNET PRIVATE

	"${SRCDIR}/Tracker/Public"
	"${SRCDIR}/Tracker/common"
)

target_compile_definitions(
	TrackerNET PRIVATE

	TRACKERNET_EXPORTS
)

target_link_libraries(
	TrackerNET PRIVATE

	"$<${IS_WINDOWS}:ws2_32>"
)

