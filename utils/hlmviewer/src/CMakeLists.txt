cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( HLMV_SOURCE_FILES )
BEGIN_SRC( HLMV_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/game_shared/bone_setup.cpp"
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/collisionutils.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/FileSystem_Tools.cpp"
			"${SRCDIR}/public/imageloader.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/physdll.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"${SRCDIR}/utils/common/cmdlib.cpp"

			"attachments_window.cpp"
			"ControlPanel.cpp"
			"FileAssociation.cpp"
			"matsyswin.cpp"
			"mdlviewer.cpp"
			"mxLineEdit2.cpp"
			"pakviewer.cpp"
			"physmesh.cpp"
			"studio_flex.cpp"
			"studio_render.cpp"
			"studio_utils.cpp"
			"sys_win.cpp"
			"ViewerSettings.cpp"

			"hlmviewer.rc"
			"icon1.ico"
		#}
	)
END_SRC( HLMV_SOURCE_FILES "Source Files" )

set( HLMV_HEADER_FILES )
BEGIN_SRC( HLMV_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/game_shared/bone_setup.h"
			"${SRCDIR}/game_shared/tf_shareddefs.h"

			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basehandle.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/bspflags.h"
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/cmodel.h"
			"${SRCDIR}/public/collisionutils.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/const.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/FileSystem_Tools.h"
			"${SRCDIR}/public/gametrace.h"
			"${SRCDIR}/public/ihandleentity.h"
			"${SRCDIR}/public/imageloader.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/istudiorender.h"
			"${SRCDIR}/public/materialsystem/imaterial.h"
			"${SRCDIR}/public/materialsystem/imaterialproxyfactory.h"
			"${SRCDIR}/public/materialsystem/imaterialsystem.h"
			"${SRCDIR}/public/materialsystem/imaterialsystemhardwareconfig.h"
			"${SRCDIR}/public/materialsystem/imaterialvar.h"
			"${SRCDIR}/public/materialsystem/imesh.h"
			"${SRCDIR}/public/materialsystem/itexture.h"
			"${SRCDIR}/public/materialsystem/materialsystem_config.h"
			"${SRCDIR}/public/mathlib.h"
			"${SRCDIR}/public/mempool.h"
			"${SRCDIR}/public/mouthinfo.h"
			"${SRCDIR}/public/phyfile.h"
			"${SRCDIR}/public/physdll.h"
			"${SRCDIR}/public/protected_things.h"
			"${SRCDIR}/public/s3_intrf.h"
			"${SRCDIR}/public/string_t.h"
			"${SRCDIR}/public/studio.h"
			"${SRCDIR}/public/tgaloader.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/memdbgoff.h"
			"${SRCDIR}/public/tier0/memdbgon.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/tier0/vprof.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utllinkedlist.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlsymbol.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vcollide.h"
			"${SRCDIR}/public/vcollide_parse.h"
			"${SRCDIR}/public/vector.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vector4d.h"
			"${SRCDIR}/public/vphysics/constraints.h"
			"${SRCDIR}/public/vphysics_interface.h"
			"${SRCDIR}/public/vstdlib/icommandline.h"
			"${SRCDIR}/public/vstdlib/random.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"

			"${SRCDIR}/utils/common/cmdlib.h"

			"${SRCDIR}/utils/mxtk/include/mx/mx.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxBmp.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxButton.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxCheckBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxChoice.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxChooseColor.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxEvent.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxFileDialog.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxGlWindow.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxGroupBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxImage.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxInit.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxLabel.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxLineEdit.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxLinkedList.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxListBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxMatSysWindow.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxMenu.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxMenuBar.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxMessageBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxpath.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxPcx.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxPopupMenu.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxProgressBar.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxRadioButton.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxScrollbar.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxSlider.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxstring.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxTab.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxTga.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxToggleButton.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxToolTip.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxTreeView.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxWidget.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxWindow.h"

			"attachments_window.h"
			"ControlPanel.h"
			"FileAssociation.h"
			"matsyswin.h"
			"mdlviewer.h"
			"mxLineEdit2.h"
			"pakviewer.h"
			"physmesh.h"
			"resource.h"
			"StudioModel.h"
			"studio_render.h"
			"sys.h"
			"ViewerSettings.h"
		#}
	)
END_SRC( HLMV_HEADER_FILES "Header Files" )

add_object(
	TARGET hlmv
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${HLMV_SOURCE_FILES} ${HLMV_HEADER_FILES}
)

target_include_directories(
	hlmv PRIVATE

	"${SRCDIR}/statemachine"
	"${SRCDIR}/utils/mxtk/include"
	"${SRCDIR}/utils/common"
	"${SRCDIR}/game_shared"
	"${SRCDIR}/game_shared/tf2"
)

target_compile_definitions(
	hlmv PRIVATE

	VECTOR
	PROTECTED_THINGS_DISABLE
)

# @HACK(SanyaSho): TODO: Resolve SEH link error with s3tc.lib
target_link_options(
	hlmv PRIVATE

	/SAFESEH:NO
)

target_link_libraries(
	hlmv PRIVATE

	"$<${IS_WINDOWS}:comctl32>"
	"$<${IS_WINDOWS}:winmm>"

	mxtk
	"$<${IS_WINDOWS}:${LIBCOMMON}/s3tc.lib>"
)
