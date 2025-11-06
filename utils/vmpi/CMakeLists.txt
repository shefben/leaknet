cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VMPI_SOURCE_FILES )
BEGIN_SRC( VMPI_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/bitbuf.cpp"
			"${SRCDIR}/public/filesystem_tools.cpp"

			"iphelpers.cpp"
			"loopback_channel.cpp"
			"messbuf.cpp"
			"ThreadedTCPSocket.cpp"
			"ThreadedTCPSocketEmu.cpp"
			"threadhelpers.cpp"
			"vmpi.cpp"
			"vmpi_filesystem.cpp"
		#}
	)
END_SRC( VMPI_SOURCE_FILES "Source Files" )

set( VMPI_HEADER_FILES )
BEGIN_SRC( VMPI_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/bitbuf.h"

			"ichannel.h"
			"iphelpers.h"
			"IThreadedTCPSocket.h"
			"loopback_channel.h"
			"messbuf.h"
			"tcpsocket.h"
			"ThreadedTCPSocketEmu.h"
			"threadhelpers.h"
			"vmpi.h"
			"vmpi_defs.h"
			"vmpi_filesystem.h"
		#}
	)
END_SRC( VMPI_HEADER_FILES "Header Files" )

add_object(
	TARGET vmpi
	STATIC
	PROPERTY_FOLDER "Utils//Static Libs"
	SOURCES ${VMPI_SOURCE_FILES} ${VMPI_HEADER_FILES}
)

target_include_directories(
	vmpi PRIVATE

	"${SRCDIR}/utils/common"
	"ZLib"
)

target_link_libraries(
	vmpi PRIVATE

	"${SRCDIR}/utils/vmpi/ZLib.lib"
)