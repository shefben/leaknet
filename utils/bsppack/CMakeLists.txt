cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( BSPPACK_SOURCE_FILES )
BEGIN_SRC( BSPPACK_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/checksum_crc.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/filesystem_tools.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"${SRCDIR}/utils/common/bsplib.cpp"
			"${SRCDIR}/utils/common/cmdlib.cpp"
			"${SRCDIR}/utils/common/scriplib.cpp"
			
			"bsppack.cpp"
		#}
	)
END_SRC( BSPPACK_SOURCE_FILES "Source Files" )

set( BSPPACK_HEADER_FILES )
BEGIN_SRC( BSPPACK_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/filesystem_tools.h"
			"${SRCDIR}/public/ibsppack.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlsymbol.h"
			"${SRCDIR}/public/utlvector.h"
		#}
	)
END_SRC( BSPPACK_HEADER_FILES "Header Files" )

add_object(
	TARGET bsppack
	MODULE
	PROPERTY_FOLDER "Utils//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${BSPPACK_SOURCE_FILES} ${BSPPACK_HEADER_FILES}
)

target_include_directories(
	bsppack PRIVATE

	"${SRCDIR}/utils/common"
)

target_compile_definitions(
	bsppack PRIVATE

	BSPPACK_EXPORTS
)
