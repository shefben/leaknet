cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( HEIGHT2NORMAL_SOURCE_FILES )
BEGIN_SRC( HEIGHT2NORMAL_SOURCE_FILES "Source Files" )
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

			"height2normal.cpp"

			"height2normal.rc"
			"height2normal.ico"
		#}
	)
END_SRC( HEIGHT2NORMAL_SOURCE_FILES "Source Files" )

set( HEIGHT2NORMAL_HEADER_FILES )
BEGIN_SRC( HEIGHT2NORMAL_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/filesystem_tools.h"
			"${SRCDIR}/public/imageloader.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/tgaloader.h"
			"${SRCDIR}/public/tgawriter.h"
			"${SRCDIR}/public/utlbuffer.h"

			"resource.h"
		#}
	)
END_SRC( HEIGHT2NORMAL_HEADER_FILES "Header Files" )

add_object(
	TARGET height2normal
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${HEIGHT2NORMAL_SOURCE_FILES} ${HEIGHT2NORMAL_HEADER_FILES}
)

target_include_directories(
	height2normal PRIVATE

	"${SRCDIR}/utils/common"
)

target_compile_definitions(
	height2normal PRIVATE

	TGAWRITER_USE_FOPEN
	IMAGE_LOADER_NO_DXTC
	TGALOADER_USE_FOPEN
)

target_link_libraries(
	height2normal PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
)