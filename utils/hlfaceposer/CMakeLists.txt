cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( HLFACEPOSER_SOURCE_FILES )
BEGIN_SRC( HLFACEPOSER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/game_shared/bone_setup.cpp"
			"${SRCDIR}/game_shared/choreoactor.cpp"
			"${SRCDIR}/game_shared/choreochannel.cpp"
			"${SRCDIR}/game_shared/choreoevent.cpp"
			"${SRCDIR}/game_shared/choreoscene.cpp"
			"${SRCDIR}/game_shared/interval.cpp"
			"${SRCDIR}/game_shared/SoundEmitterSystemBase.cpp"

			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/checksum_crc.cpp"
			"${SRCDIR}/public/collisionutils.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/FileSystem_Tools.cpp"
			"${SRCDIR}/public/imageloader.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/keyvalues.cpp"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/mempool.cpp"
			"${SRCDIR}/public/physdll.cpp"
			"${SRCDIR}/public/sentence.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"${SRCDIR}/utils/common/cmdlib.cpp"
			"${SRCDIR}/utils/common/scriplib.cpp"

			"${SRCDIR}/utils/hlmviewer/src/physmesh.cpp"
			"${SRCDIR}/utils/hlmviewer/src/studio_flex.cpp"
			"${SRCDIR}/utils/hlmviewer/src/studio_render.cpp"
			"${SRCDIR}/utils/hlmviewer/src/studio_utils.cpp"
			"${SRCDIR}/utils/hlmviewer/src/ViewerSettings.cpp"

			"ActorProperties.cpp"
			"addsoundentry.cpp"
			"basedialogparams.cpp"
			"ChannelProperties.cpp"
			"choiceproperties.cpp"
			"Choreoview.cpp"
			"CloseCaptionManager.cpp"
			"ControlPanel.cpp"
			"EditPhrase.cpp"
			"EventProperties.cpp"
			"expclass.cpp"
			"expression.cpp"
			"ExpressionProperties.cpp"
			"expressions.cpp"
			"ExpressionTool.cpp"
			"faceposertoolwindow.cpp"
			"FacePoserWorkspace.cpp"
			"faceposer_models.cpp"
			"FlexPanel.cpp"
			"GestureTool.cpp"
			"GlobalEventProperties.cpp"
			"hlfaceposer.cpp"
			"InputProperties.cpp"
			"matsyswin.cpp"
			"mdlviewer.cpp"
			"mxbitmapbutton.cpp"
			"mxbitmaptools.cpp"
			"mxbitmapwindow.cpp"
			"mxexpressionslider.cpp"
			"mxExpressionTab.cpp"
			"mxexpressiontray.cpp"
			"mxstatuswindow.cpp"
			"PhonemeConverter.cpp"
			"PhonemeEditor.cpp"
			"PhonemeEditor_CloseCaption.cpp"
			"PhonemeProperties.cpp"
			"RampTool.cpp"
			"SceneRampTool.cpp"
			"soundlookup.cpp"
			"tabwindow.cpp"
			"TimelineItem.cpp"

			"hlfaceposer.rc"

			"ico00001.ico"
			"ico00002.ico"
			"icon1.ico"
			"icon2.ico"
		#}
	)

	SRC_GRP(
		SUBGROUP "Choreo Widgets"
		SOURCES
		#{
			"ChoreoActorWidget.cpp"
			"ChoreoActorWidget.h"
			"ChoreoChannelWidget.cpp"
			"ChoreoChannelWidget.h"
			"ChoreoEventWidget.cpp"
			"ChoreoEventWidget.h"
			"ChoreoGlobalEventWidget.cpp"
			"ChoreoGlobalEventWidget.h"
			"ChoreoWidget.cpp"
			"ChoreoWidget.h"
			"ChoreoWidgetDrawHelper.cpp"
			"ChoreoWidgetDrawHelper.h"
		#}
	)
END_SRC( HLFACEPOSER_SOURCE_FILES "Source Files" )

BEGIN_SRC( HLFACEPOSER_SOURCE_FILES "Audio Code" )
	SRC_GRP(
		SOURCES
		#{
			"riff.cpp"
			"riff.h"
			"snd_audio_source.cpp"
			"snd_audio_source.h"
			"snd_wave_mixer.cpp"
			"snd_wave_mixer.h"
			"snd_wave_mixer_adpcm.cpp"
			"snd_wave_mixer_adpcm.h"
			"snd_wave_source.cpp"
			"snd_wave_source.h"
			"sound.cpp"
			"sound.h"
		#}
	)
END_SRC( HLFACEPOSER_SOURCE_FILES "Audio Code" )

set( HLFACEPOSER_HEADER_FILES )
BEGIN_SRC( HLFACEPOSER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/game_shared/bone_setup.h"
			"${SRCDIR}/game_shared/choreoactor.h"
			"${SRCDIR}/game_shared/choreochannel.h"
			"${SRCDIR}/game_shared/choreoevent.h"
			"${SRCDIR}/game_shared/choreoscene.h"
			"${SRCDIR}/game_shared/expressionsample.h"
			"${SRCDIR}/game_shared/ichoreoeventcallback.h"
			"${SRCDIR}/game_shared/interval.h"
			"${SRCDIR}/game_shared/iscenetokenprocessor.h"
			"${SRCDIR}/game_shared/npcevent.h"
			"${SRCDIR}/game_shared/sharedInterface.h"
			"${SRCDIR}/game_shared/SoundEmitterSystemBase.h"

			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basehandle.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/bspfile.h"
			"${SRCDIR}/public/bspflags.h"
			"${SRCDIR}/public/bumpvects.h"
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/checksum_crc.h"
			"${SRCDIR}/public/cmodel.h"
			"${SRCDIR}/public/collisionutils.h"
			"${SRCDIR}/public/Color.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/const.h"
			"${SRCDIR}/public/convar.h"
			"${SRCDIR}/public/engine/IEngineSound.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/FileSystem_Tools.h"
			"${SRCDIR}/public/gametrace.h"
			"${SRCDIR}/public/ihandleentity.h"
			"${SRCDIR}/public/imageloader.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/irecipientfilter.h"
			"${SRCDIR}/public/istudiorender.h"
			"${SRCDIR}/public/keyvalues.h"
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
			"${SRCDIR}/public/networkvar.h"
			"${SRCDIR}/public/phyfile.h"
			"${SRCDIR}/public/physdll.h"
			"${SRCDIR}/public/plane.h"
			"${SRCDIR}/public/protected_things.h"
			"${SRCDIR}/public/s3_intrf.h"
			"${SRCDIR}/public/sentence.h"
			"${SRCDIR}/public/soundflags.h"
			"${SRCDIR}/public/string_t.h"
			"${SRCDIR}/public/studio.h"
			"${SRCDIR}/public/tgaloader.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/mem.h"
			"${SRCDIR}/public/tier0/memalloc.h"
			"${SRCDIR}/public/tier0/memdbgoff.h"
			"${SRCDIR}/public/tier0/memdbgon.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/tier0/vprof.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utldict.h"
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
			"${SRCDIR}/public/vstdlib/IKeyValuesSystem.h"
			"${SRCDIR}/public/vstdlib/random.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"

			"${SRCDIR}/utils/common/cmdlib.h"
			"${SRCDIR}/utils/common/scriplib.h"

			"${SRCDIR}/utils/hlmviewer/src/matsyswin.h"
			"${SRCDIR}/utils/hlmviewer/src/mdlviewer.h"
			"${SRCDIR}/utils/hlmviewer/src/physmesh.h"
			"${SRCDIR}/utils/hlmviewer/src/StudioModel.h"
			"${SRCDIR}/utils/hlmviewer/src/studio_render.h"
			"${SRCDIR}/utils/hlmviewer/src/ViewerSettings.h"

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

			"${SRCDIR}/utils/phonemeextractor/phonemeextractor.h"

			"${SRCDIR}/utils/sapi51/Include/sapi.h"
			"${SRCDIR}/utils/sapi51/Include/sapiddk.h"
			"${SRCDIR}/utils/sapi51/Include/Spddkhlp.h"
			"${SRCDIR}/utils/sapi51/Include/spdebug.h"
			"${SRCDIR}/utils/sapi51/Include/sperror.h"
			"${SRCDIR}/utils/sapi51/Include/sphelper.h"

			"ActorProperties.h"
			"addsoundentry.h"
			"AudioWaveOutput.h"
			"basedialogparams.h"
			"cbase.h"
			"ChannelProperties.h"
			"choiceproperties.h"
			"ChoreoView.h"
			"ChoreoViewColors.h"
			"ControlPanel.h"
			"EditPhrase.h"
			"EventProperties.h"
			"expclass.h"
			"expression.h"
			"ExpressionProperties.h"
			"expressions.h"
			"ExpressionTool.h"
			"faceposertoolwindow.h"
			"faceposer_models.h"
			"FlexPanel.h"
			"GestureTool.h"
			"GlobalEventProperties.h"
			"hlfaceposer.h"
			"ICloseCaptionManager.h"
			"ifaceposersound.h"
			"ifaceposerworkspace.h"
			"InputProperties.h"
			"mapentities.h"
			"matsyswin.h"
			"mdlviewer.h"
			"mxbitmapbutton.h"
			"mxbitmaptools.h"
			"mxbitmapwindow.h"
			"mxexpressionslider.h"
			"mxexpressiontab.h"
			"mxexpressiontray.h"
			"mxstatuswindow.h"
			"PhonemeConverter.h"
			"PhonemeEditor.h"
			"PhonemeEditorColors.h"
			"PhonemeProperties.h"
			"RampTool.h"
			"resource.h"
			"SceneRampTool.h"
			"snd_wave_mixer_private.h"
			"soundlookup.h"
			"tabwindow.h"
			"TimelineItem.h"
		#}
	)
END_SRC( HLFACEPOSER_HEADER_FILES "Header Files" )

add_object(
	TARGET hlfaceposer
	EXECUTABLE
	PROPERTY_FOLDER "Utils//Executables"
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${HLFACEPOSER_SOURCE_FILES} ${HLFACEPOSER_HEADER_FILES}
)

target_include_directories(
	hlfaceposer PRIVATE

	"." # cbase.h
	"${SRCDIR}/utils/common"
	"${SRCDIR}/utils/mxtk/include"
	"${SRCDIR}/utils/hlmviewer/src"
	"${SRCDIR}/game_shared"
	"${SRCDIR}/utils/sapi51/include"
	"lipsinc"
	"${SRCDIR}/utils/phonemeextractor"
)

target_compile_definitions(
	hlfaceposer PRIVATE

	PROTECTED_THINGS_DISABLE
)

# @HACK(SanyaSho): TODO: Resolve SEH link error with s3tc.lib
target_link_options(
	hlfaceposer PRIVATE

	/SAFESEH:NO
)

target_link_directories(
	hlfaceposer PRIVATE

	"${SRCDIR}/utils/sapi51/lib/i386"
	"lipsinc"
)

target_link_libraries(
	hlfaceposer PRIVATE

	"$<${IS_WINDOWS}:comctl32>"
	"$<${IS_WINDOWS}:winmm>"
	"$<${IS_WINDOWS}:Msimg32>"

	"$<${IS_WINDOWS}:${LIBCOMMON}/s3tc.lib>"
	mxtk
)
