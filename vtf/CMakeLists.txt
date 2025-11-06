cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VTF_SOURCE_FILES )
BEGIN_SRC( VTF_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/imageloader.cpp"

			"vtf.cpp"
		#}
	)
END_SRC( VTF_SOURCE_FILES "Source Files" )

set( VTF_HEADER_FILES )
BEGIN_SRC( VTF_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/vtf/vtf.h"
		#}
	)
END_SRC( VTF_HEADER_FILES "Header Files" )

add_object(
	TARGET vtf
	STATIC
	SOURCES ${VTF_SOURCE_FILES} ${VTF_HEADER_FILES}
)

target_compile_definitions(
	vtf PRIVATE

	fopen=dont_use_fopen

	"$<${IS_LINUX}:IMAGE_LOADER_NO_DXTC>" # @TODO(SanyaSho): Linux support
)

target_link_libraries(
	vtf PRIVATE

	"$<${IS_WINDOWS}:${LIBCOMMON}/s3tc.lib>"
)
