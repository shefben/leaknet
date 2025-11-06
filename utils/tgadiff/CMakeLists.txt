cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( TGADIFF_SOURCE_FILES )
BEGIN_SRC( TGADIFF_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/imageloader.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/tgaloader.cpp"
			"${SRCDIR}/public/tgawriter.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"

			"tgadiff.cpp"
		#}
	)
END_SRC( TGADIFF_SOURCE_FILES "Source Files" )

add_object(
	TARGET tgadiff
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${TGADIFF_SOURCE_FILES}
)

target_include_directories(
	tgadiff PRIVATE

	"${SRCDIR}/utils/common"
)

target_compile_definitions(
	tgadiff PRIVATE

	TGALOADER_USE_FOPEN
	TGAWRITER_USE_FOPEN
)

# @HACK(SanyaSho): TODO: Resolve SEH link error with s3tc.lib
target_link_options(
	tgadiff PRIVATE

	/SAFESEH:NO
)

target_link_libraries(
	tgadiff PRIVATE

	"$<${IS_WINDOWS}:${LIBCOMMON}/s3tc.lib>"
)