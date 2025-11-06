cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VMTEDIT_SOURCE_FILES )
BEGIN_SRC( VMTEDIT_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/utlbuffer.cpp"

			"mateditpanel.cpp"
			"vmtedit.cpp"
			"VMTEditPanel.cpp"
		#}
	)
END_SRC( VMTEDIT_SOURCE_FILES "Source Files" )

set( VMTEDIT_HEADER_FILES )
BEGIN_SRC( VMTEDIT_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/interface.h"

			"${SRCDIR}/utils/common/cmdlib.h"

			"mateditpanel.h"
			"vmtedit.h"
		#}
	)
END_SRC( VMTEDIT_HEADER_FILES "Header Files" )

add_object(
	TARGET vmtedit
	EXECUTABLE WIN32
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${VMTEDIT_SOURCE_FILES} ${VMTEDIT_HEADER_FILES}
)

target_include_directories(
	vmtedit PRIVATE

	"${SRCDIR}/vgui2/include"
	"${SRCDIR}/vgui2/controls"
	"${SRCDIR}/common/MaterialSystem"
	"${SRCDIR}/utils/common"
)

# @HACK(SanyaSho): TODO: Resolve SEH link error with s3tc.lib
target_link_options(
	vmtedit PRIVATE

	/SAFESEH:NO
)

target_link_libraries(
	vmtedit PRIVATE

	"$<${IS_WINDOWS}:comctl32>"
	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"

	"${LIBCOMMON}/s3tc.lib"
	appframework
	vgui_controls
)