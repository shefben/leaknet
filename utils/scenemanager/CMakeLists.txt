cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( SCENEMANAGER_SOURCE_FILES )
BEGIN_SRC( SCENEMANAGER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/game_shared/choreoactor.cpp" # Noprecomp?
			"${SRCDIR}/game_shared/choreochannel.cpp" # Noprecomp?
			"${SRCDIR}/game_shared/choreoevent.cpp" # Noprecomp?
			"${SRCDIR}/game_shared/choreoscene.cpp" # Noprecomp?
			"${SRCDIR}/game_shared/interval.cpp"
			"${SRCDIR}/game_shared/SoundEmitterSystemBase.cpp"

			"${SRCDIR}/public/characterset.cpp" # Noprecomp?
			"${SRCDIR}/public/filesystem_helpers.cpp" # Noprecomp?
			"${SRCDIR}/public/filesystem_tools.cpp" # Noprecomp?
			"${SRCDIR}/public/interface.cpp" # Noprecomp?
			"${SRCDIR}/public/keyvalues.cpp" # Noprecomp?
			"${SRCDIR}/public/mathlib.cpp" # Noprecomp?
			"${SRCDIR}/public/mempool.cpp" # Noprecomp?
			"${SRCDIR}/public/sentence.cpp" # Noprecomp?
			#"${SRCDIR}/public/tier0/memoverride.cpp" # Noprecomp?
			"${SRCDIR}/public/utlbuffer.cpp" # Noprecomp?
			"${SRCDIR}/public/utlsymbol.cpp" # Noprecomp?

			"${SRCDIR}/utils/common/cmdlib.cpp" # Noprecomp?
			"${SRCDIR}/utils/common/scriplib.cpp" # Noprecomp?

			"basedialogparams.cpp"
			"cbase.cpp" # Create precomp: cbase.h
			"drawhelper.cpp"
			"fileloaderthread.cpp"
			"inputproperties.cpp"
			"itreeitem.cpp"
			"multiplerequest.cpp"
			"project.cpp"
			"riff.cpp" # Noprecomp?
			"scene.cpp"
			"scenemanager.cpp"
			"scenemanager_tools.cpp"
			"snd_audio_source.cpp"
			"snd_wave_mixer.cpp"
			"snd_wave_mixer_adpcm.cpp"
			"snd_wave_source.cpp"
			"sound.cpp"
			"soundbrowser.cpp"
			"soundentry.cpp"
			"soundproperties.cpp"
			"soundproperties_multiple.cpp"
			"statuswindow.cpp"
			"tabwindow.cpp"
			"vcdfile.cpp"
			"vssproperties.cpp"
			"wavebrowser.cpp"
			"wavefile.cpp"
			"waveproperties.cpp"
			"workspace.cpp"
			"workspacebrowser.cpp"
			"workspacemanager.cpp"

			"ico00001.ico"
			"ico00002.ico"
			"ico00003.ico"
			"ico00004.ico"
			"ico00005.ico"
			"ico00006.ico"
			"ico00007.ico"
			"icon1.ico"
			"project1.ico"
			"vcd1.ico"
			"wav1.ico"
			"workspac.ico"

			"scenemanager.rc"
		#}
	)
END_SRC( SCENEMANAGER_SOURCE_FILES "Source Files" )

set( SCENEMANAGER_HEADER_FILES )
BEGIN_SRC( SCENEMANAGER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/game_shared/choreoactor.h"
			"${SRCDIR}/game_shared/choreochannel.h"
			"${SRCDIR}/game_shared/choreoevent.h"
			"${SRCDIR}/game_shared/choreoscene.h"
			"${SRCDIR}/game_shared/expressionsample.h"
			"${SRCDIR}/game_shared/ichoreoeventcallback.h"
			"${SRCDIR}/game_shared/interval.h"
			"${SRCDIR}/game_shared/iscenetokenprocessor.h"
			"${SRCDIR}/game_shared/sharedInterface.h"
			"${SRCDIR}/game_shared/SoundEmitterSystemBase.h"
			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/Public/basetypes.h"
			"${SRCDIR}/Public/characterset.h"
			"${SRCDIR}/Public/commonmacros.h"
			"${SRCDIR}/Public/convar.h"
			"${SRCDIR}/Public/engine/IEngineSound.h"
			"${SRCDIR}/Public/filesystem.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/filesystem_tools.h"
			"${SRCDIR}/Public/interface.h"
			"${SRCDIR}/public/irecipientfilter.h"
			"${SRCDIR}/public/keyvalues.h"
			"${SRCDIR}/Public/mathlib.h"
			"${SRCDIR}/Public/mempool.h"
			"${SRCDIR}/public/networkvar.h"
			"${SRCDIR}/public/processor_detect.h"
			"${SRCDIR}/public/protected_things.h"
			"${SRCDIR}/Public/sentence.h"
			"${SRCDIR}/Public/soundflags.h"
			"${SRCDIR}/public/string_t.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/mem.h"
			"${SRCDIR}/public/tier0/memdbgoff.h"
			"${SRCDIR}/public/tier0/memdbgon.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/Public/utlbuffer.h"
			"${SRCDIR}/Public/utldict.h"
			"${SRCDIR}/public/utllinkedlist.h"
			"${SRCDIR}/Public/utlmemory.h"
			"${SRCDIR}/Public/utlrbtree.h"
			"${SRCDIR}/Public/utlsymbol.h"
			"${SRCDIR}/Public/utlvector.h"
			"${SRCDIR}/Public/vector.h"
			"${SRCDIR}/Public/vector2d.h"
			"${SRCDIR}/public/vstdlib/random.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"
			"${SRCDIR}/utils/common/cmdlib.h"
			"${SRCDIR}/utils/common/scriplib.h"
			"${SRCDIR}/utils/mxtk/include/mx/mx.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxButton.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxCheckBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxChoice.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxChooseColor.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxEvent.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxFileDialog.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxGlWindow.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxGroupBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxInit.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxLabel.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxLineEdit.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxLinkedList.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxListBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxlistview.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxMenu.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxMenuBar.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxMessageBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxpath.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxPopupMenu.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxProgressBar.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxRadioButton.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxScrollbar.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxSlider.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxstring.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxTab.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxToggleButton.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxToolTip.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxTreeView.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxWidget.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxWindow.h"
			"audiowaveoutput.h"
			"basedialogparams.h"
			"cbase.h"
			"drawhelper.h"
			"ifileloader.h"
			"inputproperties.h"
			"iscenemanagersound.h"
			"itreeitem.h"
			"multiplerequest.h"
			"project.h"
			"resource.h"
			"riff.h"
			"scene.h"
			"scenemanager_tools.h"
			"snd_audio_source.h"
			"snd_wave_mixer.h"
			"snd_wave_mixer_adpcm.h"
			"snd_wave_mixer_private.h"
			"snd_wave_source.h"
			"sound.h"
			"soundbrowser.h"
			"soundentry.h"
			"soundproperties.h"
			"soundproperties_multiple.h"
			"statuswindow.h"
			"tabwindow.h"
			"vcdfile.h"
			"vssproperties.h"
			"wavebrowser.h"
			"wavefile.h"
			"waveproperties.h"
			"workspace.h"
			"workspacebrowser.h"
			"workspacemanager.h"
		#}
	)
END_SRC( SCENEMANAGER_HEADER_FILES "Header Files" )

add_object(
	TARGET scenemanager
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${SCENEMANAGER_SOURCE_FILES} ${SCENEMANAGER_HEADER_FILES}
)

target_precompile_headers(
	scenemanager
	PRIVATE "cbase.h"
)

target_compile_definitions(
	scenemanager PRIVATE

	PROTECTED_THINGS_DISABLE
)

target_include_directories(
	scenemanager PRIVATE

	"." # cbase.h
	"${SRCDIR}/utils/common"
	"${SRCDIR}/utils/mxtk/include"
	"${SRCDIR}/game_shared"
)

target_link_libraries(
	scenemanager PRIVATE

	"$<${IS_WINDOWS}:winmm>"
	"$<${IS_WINDOWS}:msimg32>"
	"$<${IS_WINDOWS}:comctl32>"
	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"

	mxtk
)
