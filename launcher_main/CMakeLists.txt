cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( LAUNCHER_MAIN_SOURCE_FILES )
BEGIN_SRC( LAUNCHER_MAIN_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"launcher_main.rc"
			"main.cpp"
			"${SRCDIR}/launcher/res/launcher.ico"
		#}
	)
END_SRC( LAUNCHER_MAIN_SOURCE_FILES "Source Files" )

add_object(
	TARGET launcher_main
	EXECUTABLE WIN32
	INSTALL_DEST "${GAMEDIR}"
	INSTALL_OUTNAME "hl2"
	EXCLUDE_LIB tier0 vstdlib
	EXCLUDE_SOURCE "${SRCDIR}/public/tier0/memoverride.cpp"
	SOURCES ${LAUNCHER_MAIN_SOURCE_FILES}
)

target_compile_definitions(
	launcher_main PRIVATE

	fopen=dont_use_fopen
)

target_link_libraries(
	launcher_main PRIVATE

	"$<${IS_WINDOWS}:user32>"
)
