cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( LAUNCHER_SOURCE_FILES )
BEGIN_SRC( LAUNCHER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/registry.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"

			"filesystem.cpp"
			"launcher.cpp"
		#}
	)
END_SRC( LAUNCHER_SOURCE_FILES "Source Files" )

set( LAUNCHER_HEADER_FILES )
BEGIN_SRC( LAUNCHER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/engine_launcher_api.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/iregistry.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/tier0/vcrmode.h"
			"${SRCDIR}/public/tier0/vcr_shared.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vgui/KeyCode.h"
			"${SRCDIR}/public/vgui/MouseCode.h"
			"${SRCDIR}/public/vgui/VGUI.h"
			"${SRCDIR}/public/vstdlib/icommandline.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"

			"ifilesystem.h"
		#}
	)
END_SRC( LAUNCHER_HEADER_FILES "Header Files" )

add_object(
	TARGET launcher
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${LAUNCHER_SOURCE_FILES} ${LAUNCHER_HEADER_FILES}
)

target_include_directories(
	launcher PRIVATE
)

target_compile_definitions(
	launcher PRIVATE

	LAUNCHERONLY
)

target_link_libraries(
	launcher PRIVATE

	"$<${IS_WINDOWS}:winmm.lib>"
	"$<${IS_WINDOWS}:wsock32.lib>"
	"$<${IS_WINDOWS}:odbc32.lib>"
	"$<${IS_WINDOWS}:odbccp32.lib>"

	#"$<${IS_WINDOWS}:${SRCDIR}/dx8sdk/lib/dinput8.lib>"
	#"$<${IS_WINDOWS}:${SRCDIR}/dx8sdk/lib/ddraw.lib>"
)

