cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VTF2TGA_SOURCE_FILES )
BEGIN_SRC( VTF2TGA_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/imageloader.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/tgawriter.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"

			"vtf2tga.cpp"
		#}
	)
END_SRC( VTF2TGA_SOURCE_FILES "Source Files" )

set( VTF2TGA_HEADER_FILES )
BEGIN_SRC( VTF2TGA_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/imageloader.h"
			"${SRCDIR}/public/tgawriter.h"
		#}
	)
END_SRC( VTF2TGA_HEADER_FILES "Header Files" )

add_object(
	TARGET vtf2tga
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VTF2TGA_SOURCE_FILES} ${VTF2TGA_HEADER_FILES}
)

target_compile_definitions(
	vtf2tga PRIVATE

	TGAWRITER_USE_FOPEN
)

target_include_directories(
	vtf2tga PRIVATE

	"${SRCDIR}/utils/common"
)

# @HACK(SanyaSho): TODO: Resolve SEH link error with s3tc.lib
target_link_options(
	vtf2tga PRIVATE

	/SAFESEH:NO
)

target_link_libraries(
	vtf2tga PRIVATE

	vtf
)