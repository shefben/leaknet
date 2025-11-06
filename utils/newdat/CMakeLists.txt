cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( NEWDAT_SOURCE_FILES )
BEGIN_SRC( NEWDAT_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/checksum_md5.cpp" 
			"newdat.cpp"
		#}
	)
END_SRC( NEWDAT_SOURCE_FILES "Source Files" )

set( NEWDAT_HEADER_FILES )
BEGIN_SRC( NEWDAT_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/checksum_md5.h"
			"${SRCDIR}/public/commonmacros.h"
		#}
	)
END_SRC( NEWDAT_HEADER_FILES "Header Files" )

add_object(
	TARGET newdat
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${NEWDAT_SOURCE_FILES} ${NEWDAT_HEADER_FILES}
)

target_include_directories(
	newdat PRIVATE

	"${SRCDIR}/utils/common"
)

target_compile_definitions(
	newdat PRIVATE

	PROTECTED_THINGS_DISABLE
)