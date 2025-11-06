include_guard( GLOBAL )

set( VTEX_LAUNCHER_SOURCE_FILES )
BEGIN_SRC( VTEX_LAUNCHER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/filesystem_tools.cpp"
			"${SRCDIR}/public/imageloader.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/TGALoader.cpp"
			"${SRCDIR}/public/tgawriter.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"

			"${SRCDIR}/utils/common/cmdlib.cpp"

			"vtex.cpp"
		#}
	)
END_SRC( VTEX_LAUNCHER_SOURCE_FILES "Source Files" )

set( VTEX_LAUNCHER_HEADER_FILES )
BEGIN_SRC( VTEX_LAUNCHER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/filesystem_tools.h"
			"${SRCDIR}/public/interface.h"

			"resource.h"

			"vtex.rc"
			"vtex.ico"
		#}
	)
END_SRC( VTEX_LAUNCHER_HEADER_FILES "Header Files" )

add_object(
	TARGET vtex_launcher
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	INSTALL_OUTNAME "vtex"
	SOURCES ${VTEX_LAUNCHER_SOURCE_FILES} ${VTEX_LAUNCHER_HEADER_FILES}
)

target_include_directories(
	vtex_launcher PRIVATE

	"${SRCDIR}/common/MaterialSystem"
	"${SRCDIR}/utils/common"
)

target_compile_definitions(
	vtex_launcher PRIVATE

	TGAWRITER_USE_FOPEN
	TGALOADER_USE_FOPEN
	PROTECTED_THINGS_DISABLE
)

# @HACK(SanyaSho): TODO: Resolve SEH link error with s3tc.lib
target_link_options(
	vtex_launcher PRIVATE

	/SAFESEH:NO
)

target_link_libraries(
	vtex_launcher PRIVATE

	vtf
)