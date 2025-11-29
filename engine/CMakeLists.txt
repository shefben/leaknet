cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( ENGINE_SOURCE_FILES )
BEGIN_SRC( ENGINE_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/builddisp.cpp"
			"${SRCDIR}/common/dispcoll_common.cpp"
			"${SRCDIR}/common/disp_common.cpp"
			"${SRCDIR}/common/disp_powerinfo.cpp"
			"${SRCDIR}/common/lzma_support.cpp"
			"${SRCDIR}/common/lzmalib.cpp"
			"${SRCDIR}/common/vstring.cpp"
			"${SRCDIR}/public/bitbuf.cpp"
			"${SRCDIR}/public/bsptreedata.cpp"
			"${SRCDIR}/public/bumpvects.cpp"
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/checksum_crc.cpp"
			"${SRCDIR}/public/checksum_md5.cpp"
			"${SRCDIR}/public/collisionutils.cpp"
			"${SRCDIR}/public/convar.cpp"
			"${SRCDIR}/public/crtmemdebug.cpp"
			"${SRCDIR}/public/dt_recv.cpp"
			"${SRCDIR}/public/dt_send.cpp"
			"$<$<NOT:${IS_DEDICATED}>:${SRCDIR}/public/editor_sendcommand.cpp>" # !DEDICATED
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/imageloader.cpp"
			"${SRCDIR}/public/interface.cpp"
			"$<${IS_DEDICATED}:${SRCDIR}/public/keyvalues.cpp>"
			"${SRCDIR}/public/mathlib.cpp"
			"${SRCDIR}/public/measure_section.cpp"
			"${SRCDIR}/public/mempool.cpp"
			"${SRCDIR}/public/mem_fgets.cpp"
			"${SRCDIR}/public/networkvar.cpp"
			"$<$<NOT:${IS_DEDICATED}>:${SRCDIR}/public/registry.cpp>"
			"$<$<NOT:${IS_DEDICATED}>:${SRCDIR}/public/scratchpad3d.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:${SRCDIR}/public/sentence.cpp>"
			"${SRCDIR}/public/tgawriter.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"
			"${SRCDIR}/public/vallocator.cpp"
			"${SRCDIR}/public/vmatrix.cpp"
			"baseautocompletefilelist.cpp"
			"bitbuf_errorhandler.cpp"
			"$<$<NOT:${IS_DEDICATED}>:bugreporter.cpp>"
			"buildnum.cpp"
			"$<$<NOT:${IS_DEDICATED}>:cd.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cdll_engine_int.cpp>" # !DEDICATED # Precomp: glquake.h
			"$<$<NOT:${IS_DEDICATED}>:cdll_exp.cpp>" # !DEDICATED
			"changeframelist.cpp"
			"checksum_engine.cpp"
			"client.cpp"
			"$<$<NOT:${IS_DEDICATED}>:cl_demo.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demoaction.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demoactioneditors.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demoactionmanager.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demoaction_types.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demoeditorpanel.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demosmootherpanel.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demouipanel.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_entityreport.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_ents.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_ents_parse.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_localnetworkbackdoor.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_main.cpp>" # !DEDICATED
			"cl_null.cpp"
			"$<$<NOT:${IS_DEDICATED}>:cl_parse.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_parse_event.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_pred.cpp>" # !DEDICATED
			"cmd.cpp"
			"cmodel.cpp"
			"cmodel_bsp.cpp"
			"cmodel_disp.cpp"
			"common.cpp" # NoPrecomp?
			"$<$<NOT:${IS_DEDICATED}>:conproc.cpp>" # !DEDICATED # NoPrecomp?
			"console.cpp"
			"$<$<NOT:${IS_DEDICATED}>:quakeconsole.cpp>" # !DEDICATED
			"CSharedModelLoader.cpp"
			"cspatialpartition.cpp"
			"cvar.cpp"
			"$<$<NOT:${IS_DEDICATED}>:debugoverlay.cpp>" # !DEDICATED
			"debug_leafvis.cpp" # Precomp: glquake.h
			"decals.cpp"
			"decal_clip.cpp"
			"$<$<NOT:${IS_DEDICATED}>:dinput.cpp>" # !DEDICATED
			"disp.cpp" # Precomp: glquake.h
			"$<$<NOT:${IS_DEDICATED}>:dispbuild.cpp>" # !DEDICATED
			"dispchain.cpp" # Precomp: glquake.h
			"$<$<NOT:${IS_DEDICATED}>:displaystats.cpp>" # !DEDICATED # Precomp: glquake.h
			"disp_defs.cpp"
			"disp_helpers.cpp"
			"disp_interface.cpp" # Precomp: glquake.h
			"disp_leaflink.cpp"
			"disp_mapload.cpp" # Precomp: glquake.h
			"dt.cpp"
			"dt_common_eng.cpp"
			"dt_encode.cpp"
			"dt_instrumentation.cpp"
			"dt_instrumentation_server.cpp"
			"dt_localtransfer.cpp"
			"dt_recv_decoder.cpp"
			"dt_recv_eng.cpp"
			"dt_send_eng.cpp"
			"dt_stack.cpp"
			"dt_test.cpp"
			"enginesingleuserfilter.cpp"
			"enginestats.cpp"
			"enginetrace.cpp"
			"filesystem_engine.cpp"
			"GameEventManager.cpp"
			"gametrace_engine.cpp"
			"glquake.cpp" # Create precomp: glquake.h
			"gl_draw.cpp" # DEDICATED: Noprecomp? CLIENT: Precomp: glquake.h
			"$<$<NOT:${IS_DEDICATED}>:gl_drawlights.cpp>" # !DEDICATED # Precomp: glquake.h
			"$<$<NOT:${IS_DEDICATED}>:gl_lightmap.cpp>" # !DEDICATED # Precomp: glquake.h
			"$<$<NOT:${IS_DEDICATED}>:gl_matsysiface.cpp>" # !DEDICATED # NoPrecomp?
			"$<$<NOT:${IS_DEDICATED}>:gl_rlight.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:gl_rmain.cpp>" # !DEDICATED # Precomp: glquake.h
			"$<$<NOT:${IS_DEDICATED}>:gl_rmisc.cpp>" # !DEDICATED # NoPrecomp?
			"gl_rsurf.cpp" # DEDICATED: Noprecomp? CLIENT: Precomp: glquake.h
			"$<$<NOT:${IS_DEDICATED}>:gl_screen.cpp>" # !DEDICATED # Precomp: glquake.h
			"gl_shader.cpp" # Precomp: glquake.h
			"$<$<NOT:${IS_DEDICATED}>:gl_test.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:gl_vidnt.cpp>" # !DEDICATED # Precomp: glquake.h
			"$<$<NOT:${IS_DEDICATED}>:gl_warp.cpp>" # !DEDICATED # Precomp: glquake.h
			"host.cpp" # NoPrecomp?
			"host_cmd.cpp" # Precomp: glquake.h
			"host_listmaps.cpp"
			"$<$<NOT:${IS_DEDICATED}>:host_saverestore.cpp>" # !DEDICATED # Precomp: glquake.h
			"host_state.cpp"
			"info.cpp"
			"initmathlib.cpp"
			"$<$<NOT:${IS_DEDICATED}>:keys.cpp>" # !DEDICATED # NoPrecomp?
			"$<$<NOT:${IS_DEDICATED}>:lightcache.cpp>" # !DEDICATED
			"LocalNetworkBackdoor.cpp"
			"l_studio.cpp"
			"materialproxyfactory.cpp" # Precomp: glquake.h
			"matsys_interface.cpp"
			"ModelInfo.cpp"
			"modelloader.cpp" # Precomp: glquake.h
			"mod_vis.cpp"
			"$<$<NOT:${IS_DEDICATED}>:netgraph.cpp>" # !DEDICATED
			"networkstringtable.cpp"
			"$<$<NOT:${IS_DEDICATED}>:networkstringtableclient.cpp>" # !DEDICATED
			"NetworkStringTableItem.cpp"
			"networkstringtableserver.cpp"
			"net_chan.cpp"
			"net_synctags.cpp"
			"net_ws.cpp" # NoPrecomp?
			"OcclusionSystem.cpp"
			"$<$<NOT:${IS_DEDICATED}>:Overlay.cpp>" # !DEDICATED
			"packed_entity.cpp"
			"precache.cpp"
			"pr_edict.cpp"
			"quakedef.cpp"
			"randomstream.cpp"
			"recventlist.cpp"
			"$<$<NOT:${IS_DEDICATED}>:r_areaportal.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:r_efx.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:r_linefile.cpp>" # !DEDICATED # Precomp: glquake.h
			"$<$<NOT:${IS_DEDICATED}>:shadowmgr.cpp>" # !DEDICATED
			"staticpropmgr.cpp"
			"sv_ents_write.cpp"
			"sv_filter.cpp"
			"sv_framesnapshot.cpp"
			"sv_log.cpp"
			"sv_main.cpp"
			"sv_master.cpp"
			"sv_packedentities.cpp"
			"sv_phys.cpp"
			"sv_precache.cpp"
			"sv_rcom.cpp"
			"sv_redirect.cpp"
			"sv_user.cpp"
			"sys_dll.cpp" # NoPrecomp?
			"sys_dll2.cpp"
			"sys_engine.cpp"
			"$<$<NOT:${IS_DEDICATED}>:sys_getmodes.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:sys_mainwind.cpp>"
			"$<${IS_DEDICATED}:sys_linuxwind.cpp>"
			"terrainmod_functions.cpp"
			"testscriptmgr.cpp"
			"$<$<NOT:${IS_DEDICATED}>:thread.cpp>" # !DEDICATED
			"tmessage.cpp"
			"traceinit.cpp" # NoPrecomp?
			"vengineserver_impl.cpp"
			"$<$<NOT:${IS_DEDICATED}>:vgui_basepanel.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:vgui_baseui_interface.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:vgui_DebugSystemPanel.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:vgui_int.cpp>" # !DEDICATED # NoPrecomp?
			"vgui_intwrap2.cpp"
			"$<$<NOT:${IS_DEDICATED}>:view.cpp>" # !DEDICATED
			"voiceserver_impl.cpp"
			"vprof_engine.cpp"
			"world.cpp"
			"zone.cpp"

			"halflife.rc"
			"valve.ico"
		#}
	)

	SRC_GRP(
		SUBGROUP "Audio Code"
		SOURCES
		#{
			"$<$<NOT:${IS_DEDICATED}>:EngineSoundClient.cpp>" # !DEDICATED
			"EngineSoundServer.cpp"
			"$<$<NOT:${IS_DEDICATED}>:engsoundservice.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:${SRCDIR}/public/riff.cpp>" # !DEDICATED # Include: ${SRCDIR}/audio/private
			"$<$<NOT:${IS_DEDICATED}>:snd_io.cpp>" # !DEDICATED # Include: ${SRCDIR}/audio/private
		#}
	)
END_SRC( ENGINE_SOURCE_FILES "Source Files" )

BEGIN_SRC( ENGINE_SOURCE_FILES "Audio Private" )
	SRC_GRP(
		SUBGROUP "Audio Private: Source"
		SOURCES
		#{
			"$<$<NOT:${IS_DEDICATED}>:audio/private/circularbuffer.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_audio_source.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_dev_direct.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_dev_wave.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_dma.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_dsp.cpp>" # !DEDICATED
			"audio/private/snd_mem.cpp"
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_mix.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_mp3_source.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_sentence_mixer.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_wave_data.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_wave_mixer.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_wave_mixer_adpcm.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_wave_mixer_mp3.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_wave_source.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_wave_temp.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_win.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/voice.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/voice_gain.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/voice_mixer_controls.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/voice_record_dsound.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/voice_sound_engine_interface.cpp>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:audio/private/voice_wavefile.cpp>" # !DEDICATED
			"audio/private/vox.cpp"

			# @TODO(SanyaSho): id386 sound
			#"$<$<NOT:${IS_DEDICATED}>:audio/private/snd_mixa.s>"
		#}
	)

	SRC_GRP(
		SUBGROUP "Audio Private: Headers"
		SOURCES
		#{
			"audio/private/circularbuffer.h"
			"audio/private/eax.h"
			"audio/private/snd_channels.h"
			"audio/private/snd_convars.h"
			"audio/private/snd_dev_direct.h"
			"audio/private/snd_dev_wave.h"
			"audio/private/snd_dma.h"
			"audio/private/snd_env_fx.h"
			"audio/private/snd_fixedint.h"
			"audio/private/snd_mix_buf.h"
			"audio/private/snd_mp3_source.h"
			"audio/private/snd_sfx.h"
			"audio/private/snd_wave_data.h"
			"audio/private/snd_wave_mixer.h"
			"audio/private/snd_wave_mixer_adpcm.h"
			"audio/private/snd_wave_mixer_private.h"
			"audio/private/snd_wave_source.h"
			"audio/private/snd_wave_temp.h"
			"audio/private/sound_private.h"
			"audio/private/voice_gain.h"
			"audio/private/voice_mixer_controls.h"
			"audio/private/voice_sound_engine_interface.h"
			"audio/private/voice_wavefile.h"
			"audio/private/vox_private.h"
		#}
	)
END_SRC( ENGINE_SOURCE_FILES "Audio Private" )

BEGIN_SRC( ENGINE_SOURCE_FILES "Audio Public" )
	SRC_GRP(
		SOURCES
		#{
			"audio/public/ivoicecodec.h"
			"audio/public/ivoicerecord.h"
			"audio/public/snd_audio_source.h"
			"audio/public/snd_device.h"
			"audio/public/snd_io.h"
			"audio/public/sound.h"
			"audio/public/soundservice.h"
			"audio/public/voice.h"
			"audio/public/vox.h"
		#}
	)
END_SRC( ENGINE_SOURCE_FILES "Audio Public" )

set( ENGINE_HEADER_FILES )
BEGIN_SRC( ENGINE_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			#"${SRCDIR}/common/authchallenge.h"
			"${SRCDIR}/common/builddisp.h"
			"${SRCDIR}/common/common.h"
			"${SRCDIR}/common/con_nprint.h"
			"${SRCDIR}/common/dispcoll_common.h"
			"${SRCDIR}/common/disp_common.h"
			"${SRCDIR}/common/disp_powerinfo.h"
			"${SRCDIR}/common/event_system.h"
			"${SRCDIR}/common/GameUI/IGameConsole.h"
			"${SRCDIR}/common/optimize.h"
			"${SRCDIR}/common/protocol.h"
			"${SRCDIR}/common/proto_oob.h"
			"${SRCDIR}/common/proto_version.h"
			"${SRCDIR}/common/qlimits.h"
			"${SRCDIR}/common/quakedef.h"
			"${SRCDIR}/common/vmodes.h"
			"${SRCDIR}/common/vstring.h"
			"${SRCDIR}/gameui/GameUI_Interface.h"
			"${SRCDIR}/public/anorms.h"
			"${SRCDIR}/public/appframework/IAppSystem.h"
			"${SRCDIR}/public/basetypes.h"
			"${SRCDIR}/public/bitbuf.h"
			"${SRCDIR}/public/bitvec.h"
			"${SRCDIR}/public/bspfile.h"
			"${SRCDIR}/public/bspflags.h"
			"${SRCDIR}/public/bsptreedata.h"
			"${SRCDIR}/public/bumpvects.h"
			"${SRCDIR}/public/cache_user.h"
			"${SRCDIR}/public/cdll_int.h"
			"${SRCDIR}/public/characterset.h"
			"${SRCDIR}/public/checksum_crc.h"
			"${SRCDIR}/public/checksum_md5.h"
			"${SRCDIR}/public/clientstats.h"
			"${SRCDIR}/public/client_class.h"
			"${SRCDIR}/public/client_command.h"
			"${SRCDIR}/public/cmodel.h"
			"${SRCDIR}/public/collisionutils.h"
			"${SRCDIR}/public/commonmacros.h"
			"${SRCDIR}/public/const.h"
			"${SRCDIR}/public/convar.h"
			"${SRCDIR}/public/coordsize.h"
			"${SRCDIR}/public/crtmemdebug.h"
			#"${SRCDIR}/public/custom.h"
			"${SRCDIR}/public/datamap.h"
			"${SRCDIR}/public/tier0/dbg.h"
			"${SRCDIR}/public/dlight.h"
			"${SRCDIR}/public/dll_state.h"
			"${SRCDIR}/public/dt_common.h"
			"${SRCDIR}/public/dt_recv.h"
			"${SRCDIR}/public/dt_send.h"
			"${SRCDIR}/public/edict.h"
			"${SRCDIR}/public/editor_sendcommand.h"
			"${SRCDIR}/public/eiface.h"
			"${SRCDIR}/public/engine/icollideable.h"
			"${SRCDIR}/public/engine/IEngineSound.h"
			"${SRCDIR}/public/engine/IEngineTrace.h"
			#"${SRCDIR}/public/engine/iprops.h"
			"${SRCDIR}/public/engine/ishadowmgr.h"
			"${SRCDIR}/public/engine/ivdebugoverlay.h"
			"${SRCDIR}/public/engine/ivmodelrender.h"
			"${SRCDIR}/public/engine_hlds_api.h"
			"${SRCDIR}/public/engine_launcher_api.h"
			"${SRCDIR}/public/event_flags.h"
			"${SRCDIR}/public/exefuncs.h"
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/gamebspfile.h"
			"${SRCDIR}/public/ibsppack.h"
			#"${SRCDIR}/public/iclientcollideable.h"
			"${SRCDIR}/public/icliententity.h"
			"${SRCDIR}/public/icliententitylist.h"
			"${SRCDIR}/public/iclientnetworkable.h"
			"${SRCDIR}/public/iclientrenderable.h"
			"${SRCDIR}/public/iclientthinkable.h"
			"${SRCDIR}/public/iclientunknown.h"
			"${SRCDIR}/public/icreatevfont.h"
			"${SRCDIR}/public/icvar.h"
			"${SRCDIR}/public/idedicatedexports.h"
			"${SRCDIR}/public/iefx.h"
			"${SRCDIR}/public/ienginevgui.h"
			"${SRCDIR}/public/IGameUIFuncs.h"
			"${SRCDIR}/public/imageloader.h"
			"${SRCDIR}/public/inetgraph.h"
			"${SRCDIR}/public/inetworkstringtableclient.h"
			"${SRCDIR}/public/inetworkstringtableserver.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/iprediction.h"
			"${SRCDIR}/public/iregistry.h"
			"${SRCDIR}/public/iscratchpad3d.h"
			"${SRCDIR}/public/iserverentity.h"
			"${SRCDIR}/public/iservernetworkable.h"
			"${SRCDIR}/public/ispatialpartition.h"
			"${SRCDIR}/public/istudiorender.h"
			"${SRCDIR}/public/ivguicenterprint.h"
			"${SRCDIR}/public/ivoiceserver.h"
			"${SRCDIR}/public/ivoicetweak.h"
			"${SRCDIR}/public/ivrenderview.h"
			"${SRCDIR}/public/ivtex.h"
			"${SRCDIR}/public/kbutton.h"
			"${SRCDIR}/public/keydefs.h"
			"${SRCDIR}/public/materialsystem/imaterial.h"
			"${SRCDIR}/public/materialsystem/imaterialproxy.h"
			"${SRCDIR}/public/materialsystem/imaterialproxyfactory.h"
			"${SRCDIR}/public/materialsystem/imaterialsystem.h"
			"${SRCDIR}/public/materialsystem/imaterialsystemhardwareconfig.h"
			"${SRCDIR}/public/materialsystem/imaterialsystemstats.h"
			"${SRCDIR}/public/materialsystem/imaterialvar.h"
			"${SRCDIR}/public/materialsystem/imesh.h"
			"${SRCDIR}/public/materialsystem/itexture.h"
			"${SRCDIR}/public/materialsystem/materialsystem_config.h"
			"${SRCDIR}/public/mathlib.h"
			"${SRCDIR}/public/measure_section.h"
			"${SRCDIR}/public/mempool.h"
			"${SRCDIR}/public/mem_fgets.h"
			"${SRCDIR}/public/model_types.h"
			"${SRCDIR}/public/modes.h"
			"${SRCDIR}/public/mouthinfo.h"
			"${SRCDIR}/public/netadr.h"
			"${SRCDIR}/public/networkstringtabledefs.h"
			"${SRCDIR}/public/nmatrix.h"
			"${SRCDIR}/public/nvector.h"
			"${SRCDIR}/public/overlaytext.h"
			"${SRCDIR}/public/phyfile.h"
			"${SRCDIR}/public/plane.h"
			"${SRCDIR}/public/tier0/fasttimer.h"
			"${SRCDIR}/public/tier0/platform.h"
			"${SRCDIR}/public/tier0/vcrmode.h"
			"${SRCDIR}/public/tier0/vcr_shared.h"
			"${SRCDIR}/public/protected_things.h"
			"${SRCDIR}/public/riff.h"
			"${SRCDIR}/public/r_efx.h"
			"${SRCDIR}/public/s3_intrf.h"
			"${SRCDIR}/public/savegame_version.h"
			"${SRCDIR}/public/scratchpad3d.h"
			"${SRCDIR}/public/sentence.h"
			"${SRCDIR}/public/server_class.h"
			"${SRCDIR}/public/shake.h"
			"${SRCDIR}/public/soundflags.h"
			"${SRCDIR}/public/soundinfo.h"
			"${SRCDIR}/public/string_t.h"
			"${SRCDIR}/public/studio.h"
			"${SRCDIR}/public/surfinfo.h"
			"${SRCDIR}/public/terrainmod.h"
			"${SRCDIR}/public/tgaloader.h"
			"${SRCDIR}/public/tgawriter.h"
			"${SRCDIR}/public/trace.h"
			"${SRCDIR}/public/usercmd.h"
			"${SRCDIR}/public/utlbidirectionalset.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utldict.h"
			"${SRCDIR}/public/utlfixedlinkedlist.h"
			"${SRCDIR}/public/utlfixedmemory.h"
			"${SRCDIR}/public/utllinkedlist.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/UtlSortVector.h"
			"${SRCDIR}/public/utlsymbol.h"
			"${SRCDIR}/public/utlvector.h"
			"${SRCDIR}/public/vallocator.h"
			"${SRCDIR}/public/vcollide.h"
			"${SRCDIR}/public/vector.h"
			"${SRCDIR}/public/vector2d.h"
			"${SRCDIR}/public/vector4d.h"
			"${SRCDIR}/public/VGuiMatSurface/IMatSystemSurface.h"
			"${SRCDIR}/public/view_shared.h"
			"${SRCDIR}/public/vmatrix.h"
			"${SRCDIR}/public/vphysics_interface.h"
			"${SRCDIR}/public/vplane.h"
			"${SRCDIR}/public/vstdlib/icommandline.h"
			"${SRCDIR}/public/vstdlib/random.h"
			"${SRCDIR}/public/vstdlib/strtools.h"
			"${SRCDIR}/public/vstdlib/vstdlib.h"
			"${SRCDIR}/public/worldsize.h"

			#"${SRCDIR}/vgui2/controls/VGUI_Controls.h"
			#"${SRCDIR}/vgui2/controls/VGUI_EditablePanel.h"
			#"${SRCDIR}/vgui2/controls/VGUI_FocusNavGroup.h"
			#"${SRCDIR}/vgui2/controls/VGUI_Panel.h"
			#"${SRCDIR}/vgui2/controls/VGUI_PHandle.h"
			#"${SRCDIR}/vgui2/include/VGUI.h"
			#"${SRCDIR}/vgui2/include/VGUI_Color.h"
			#"${SRCDIR}/vgui2/include/VGUI_Cursor.h"
			#"${SRCDIR}/vgui2/include/VGUI_Dar.h"
			#"${SRCDIR}/vgui2/include/VGUI_IClientPanel.h"
			#"${SRCDIR}/vgui2/include/VGUI_IHTML.h"
			#"${SRCDIR}/vgui2/include/VGUI_IImage.h"
			#"${SRCDIR}/vgui2/include/VGUI_IInput.h"
			#"${SRCDIR}/vgui2/include/VGUI_ILocalize.h"
			#"${SRCDIR}/vgui2/include/VGUI_IPanel.h"
			#"${SRCDIR}/vgui2/include/VGUI_IScheme.h"
			#"${SRCDIR}/vgui2/include/VGUI_ISurface.h"
			#"${SRCDIR}/vgui2/include/VGUI_IVGui.h"
			#"${SRCDIR}/vgui2/include/VGUI_KeyCode.h"
			#"${SRCDIR}/vgui2/include/VGUI_keyvalues.h"
			#"${SRCDIR}/vgui2/include/VGUI_MessageMap.h"
			#"${SRCDIR}/vgui2/include/VGUI_MouseCode.h"

			#"ASM_I386.H"
			"baseautocompletefilelist.h"
			"bitbuf_errorhandler.h"
			#"bugreporter.h"
			"cache.h"
			"cd.h"
			"cdll_engine_int.h"
			"cd_internal.h"
			"changeframelist.h"
			"checksum_engine.h"
			"client.h"
			"$<$<NOT:${IS_DEDICATED}>:cl_demoaction.h>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demoactioneditors.h>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demoactionmanager.h>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demoaction_types.h>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demoeditorpanel.h>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demosmootherpanel.h>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demosmoothing.h>" # !DEDICATED
			"$<$<NOT:${IS_DEDICATED}>:cl_demouipanel.h>" # !DEDICATED
			"cl_entityreport.h"
			"cl_ents.h"
			"cl_ents_parse.h"
			"cl_localnetworkbackdoor.h"
			"cl_main.h"
			"cl_parse.h"
			"cl_parse_event.h"
			"cl_pred.h"
			"cl_servercache.h"
			"cmd.h"
			"cmodel_engine.h"
			"cmodel_private.h"
			"color.h"
			"colorquant.h"
			"conprint.h"
			"conproc.h"
			"console.h"
			"CSharedModelLoader.h"
			"cvar.h"
			"debugoverlay.h"
			"debug_leafvis.h"
			"decal.h"
			"decal_clip.h"
			"decal_private.h"
			"demo.h"
			"dinput.h"
			"disp.h"
			"dispbuild.h"
			"dispchain.h"
			"displaystats.h"
			"dispnode.h"
			"disp_defs.h"
			"disp_helpers.h"
			"disp_leaflink.h"
			"draw.h"
			"dt.h"
			"dt_common_eng.h"
			"dt_encode.h"
			"dt_instrumentation.h"
			"dt_instrumentation_server.h"
			"dt_localtransfer.h"
			"dt_recv_decoder.h"
			"dt_recv_eng.h"
			"dt_send_eng.h"
			"dt_stack.h"
			"dt_test.h"
			"enginebugreporter.h"
			"enginesingleuserfilter.h"
			"EngineSoundInternal.h"
			"enginestats.h"
			"enginetrace.h"
			"filesystem_engine.h"
			"filter.h"
			"frame.h"
			"framesnapshot.h"
			"gameeventmanager.h"
			"game_interface.h"
			"getintersectingsurfaces_struct.h"
			"glquake.h"
			"gl_cvars.h"
			"gl_drawlights.h"
			"gl_hw.h"
			"gl_lightmap.h"
			"gl_matsysiface.h"
			"gl_model.h"
			"gl_model_private.h"
			"gl_rmain.h"
			"gl_rsurf.h"
			"gl_shader.h"
			"gl_shaderutil.h"
			"gl_texture.h"
			"gl_warp_sin.h"
			"gl_water.h"
			"host.h"
			"host_cmd.h"
			"host_jmp.h"
			"host_saverestore.h"
			"host_state.h"
			"hud_handlers.h"
			"idispinfo.h"
			"iengine.h"
			"igame.h"
			"info.h"
			"IOcclusionSystem.h"
			"ispatialpartitioninternal.h"
			"ithread.h"
			"ivideomode.h"
			"keys.h"
			"lightcache.h"
			"llist.hpp"
			"LocalNetworkBackdoor.h"
			"lowpassstream.h"
			"l_studio.h"
			"master.h"
			"materialproxyfactory.h"
			"mem.h"
			"modelgen.h"
			"ModelInfo.h"
			"modelloader.h"
			"net.h"
			"networkstringtable.h"
			"networkstringtableclient.h"
			"networkstringtablecontainerclient.h"
			"networkstringtablecontainerserver.h"
			"networkstringtableitem.h"
			"networkstringtableserver.h"
			"net_synctags.h"
			"Overlay.h"
			"packedentityarray.h"
			"packed_entity.h"
			"precache.h"
			"procinfo.h"
			"profile.h"
			"profiling.h"
			"progs.h"
			"pr_edict.h"
			"recventlist.h"
			"render.h"
			"r_areaportal.h"
			"r_efxextern.h"
			"r_local.h"
			"sbar.h"
			"screen.h"
			"server.h"
			"shadowmgr.h"
			"spritegn.h"
			"staticpropmgr.h"
			"studio_internal.h"
			"sv_ents_write.h"
			"sv_filter.h"
			"sv_log.h"
			"sv_main.h"
			"sv_packedentities.h"
			"sv_precache.h"
			"sys.h"
			"sysexternal.h"
			"sys_dll.h"
			"sys_mainwind.h"
			"terrainmod_functions.h"
			"testscriptmgr.h"
			"textures.h"
			"tmessage.h"
			"traceinit.h"
			#"usermsg.h"
			"vengineserver_impl.h"
			"vgui_basepanel.h"
			"$<$<NOT:${IS_DEDICATED}>:vgui_baseui_interface.h>" # !DEDICATED
			"vgui_DebugSystemPanel.h"
			"vgui_int.h"
			"vgui_intwrap2.h"
			"vid.h"
			"view.h"
			"vprof_engine.h"
			"winquake.h"
			"world.h"
			"zone.h"
		#}
	)

	SRC_GRP(
		SUBGROUP "Audio Headers"
		SOURCES
		#{
			#"clientsound_impl.h"
		#}
	)
END_SRC( ENGINE_HEADER_FILES "Header Files" )

add_object(
	TARGET engine
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${ENGINE_SOURCE_FILES} ${ENGINE_HEADER_FILES}
)

target_precompile_headers(
	engine
	PRIVATE "glquake.h"
)

target_include_directories(
	engine PRIVATE

	"."
	"audio/public"
	"audio/private"
)

target_compile_definitions(
	engine PRIVATE

	"$<${IS_DEDICATED}:SWDS>"
	"$<${IS_DEDICATED}:IMAGE_LOADER_NO_DXTC>"

	#fopen=dont_use_fopen # @TODO(SanyaSho): see bugreporter.cpp
	USE_CONVARS
	VOICE_OVER_IP
	BUMPMAP
	__USEA3D
	_ADD_EAX_
	ENGINE_DLL
	#PROTECTED_THINGS_ENABLE # @TODO(SanyaSho): another weird conflict
)

target_link_libraries(
	engine PRIVATE

	"$<${IS_WINDOWS}:dinput8>"
	"$<${IS_WINDOWS}:winmm>"
	"$<${IS_WINDOWS}:wsock32>"

	"$<${IS_WINDOWS}:${SRCDIR}/dx8sdk/lib/dsound.lib>"
	"$<${IS_WINDOWS}:${SRCDIR}/dx8sdk/lib/dxguid.lib>"

	"$<${IS_WINDOWS}:${LIBCOMMON}/s3tc.lib>"

	"$<$<NOT:${IS_DEDICATED}>:vgui_controls>"
	vtf
)

