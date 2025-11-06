cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VBSPINFO_SOURCE_FILES )
BEGIN_SRC( VBSPINFO_SOURCE_FILES "Source Files" )
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

			"vbspinfo.cpp"
		#}
	)
END_SRC( VBSPINFO_SOURCE_FILES "Source Files" )

set( VBSPINFO_HEADER_FILES )
BEGIN_SRC( VBSPINFO_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/checksum_crc.h"
			"${SRCDIR}/public/filesystem_helpers.h"
		#}
	)
END_SRC( VBSPINFO_HEADER_FILES "Header Files" )

add_object(
	TARGET vbspinfo
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VBSPINFO_SOURCE_FILES} ${VBSPINFO_HEADER_FILES}
)

target_compile_definitions(
	vbspinfo PRIVATE

	DONT_PROTECT_FILEIO_FUNCTIONS
)

target_include_directories(
	vbspinfo PRIVATE

	"${SRCDIR}/utils/common"
)