include_guard( GLOBAL )

set( VTEX_DLL_SOURCE_FILES )
BEGIN_SRC( VTEX_DLL_SOURCE_FILES "Source Files" )
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
END_SRC( VTEX_DLL_SOURCE_FILES "Source Files" )

set( VTEX_DLL_HEADER_FILES )
BEGIN_SRC( VTEX_DLL_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/ivtex.h"
		#}
	)
END_SRC( VTEX_DLL_HEADER_FILES "Header Files" )

add_object(
	TARGET vtex_dll
	MODULE
	PROPERTY_FOLDER "Utils//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VTEX_DLL_SOURCE_FILES} ${VTEX_DLL_HEADER_FILES}
)

target_include_directories(
	vtex_dll PRIVATE

	"${SRCDIR}/common/MaterialSystem"
	"${SRCDIR}/utils/common"
)

target_compile_definitions(
	vtex_dll PRIVATE

	TGAWRITER_USE_FOPEN
	TGALOADER_USE_FOPEN
	VTEX_DLL
	VTEX_DLL_EXPORTS
	PROTECTED_THINGS_DISABLE
)

target_link_libraries(
	vtex_dll PRIVATE

	vtf
)