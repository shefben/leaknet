#=============================================================================
# parse_shaders.cmake
#
# SanyaSho (2024)
#=============================================================================

include_guard( GLOBAL )

function( compile_shader )
	cmake_parse_arguments(
		ARGS
		"FXC;VSH;PSH"
		"TARGET;WORKING_DIRECTORY"
		"CUSTOM_OUTDIR;FILE_FULL;FILE;COMPILE_FLAGS"
		${ARGN}
	)

	#message( STATUS "ARGS_FXC: ${ARGS_FXC}" )
	#message( STATUS "ARGS_VSH: ${ARGS_VSH}" )
	#message( STATUS "ARGS_PSH: ${ARGS_PSH}" )
	#message( STATUS "ARGS_TARGET: ${ARGS_TARGET}" )
	#message( STATUS "ARGS_WORKING_DIRECTORY: ${ARGS_WORKING_DIRECTORY}" )
	#message( STATUS "ARGS_CUSTOM_OUTDIR: ${ARGS_CUSTOM_OUTDIR}" )
	#message( STATUS "ARGS_FILE_FULL: ${ARGS_FILE_FULL}" )
	#message( STATUS "ARGS_FILE: ${ARGS_FILE}" )
	#message( STATUS "ARGS_COMPILE_FLAGS: ${ARGS_COMPILE_FLAGS}" )

	if( NOT ARGS_FILE_FULL OR NOT ARGS_FILE )
		return()
	endif()

	if( NOT ARGS_COMPILE_FLAGS )
		set( ARGS_COMPILE_FLAGS "-dx9" ) # Fallback to -dx9
	endif()

	set( PERL_EXE "perl" )
	if( WIN32 )
		set( PERL_EXE "${SRCDIR}/devtools/bin/perl.exe" )
	endif()

	set( FILE_WLE )
	get_filename_component( FILE_WLE ${ARGS_FILE} NAME_WLE )

	if( ARGS_FXC )
		set( PREP_FILE "${SRCDIR}/devtools/bin/fxc_prep.pl" )
		set( OUTDIR "fxctmp9" )
		if( ARGS_CUSTOM_OUTDIR )
			set( OUTDIR "${ARGS_CUSTOM_OUTDIR}" )
			target_include_directories( ${ARGS_TARGET} PRIVATE "${ARGS_CUSTOM_OUTDIR}" )
		endif()
		set( OUT_FILE "${ARGS_WORKING_DIRECTORY}/${OUTDIR}/${FILE_WLE}.inc" )
	elseif( ARGS_VSH )
		set( PREP_FILE "${SRCDIR}/devtools/bin/vsh_prep.pl" )
		set( OUT_FILE "${ARGS_WORKING_DIRECTORY}/vshtmp9/${FILE_WLE}.inc" )
	elseif( ARGS_PSH )
		set( PREP_FILE "${SRCDIR}/devtools/bin/psh_prep.pl" )
		set( OUT_FILE "${ARGS_WORKING_DIRECTORY}/pshtmp9/${FILE_WLE}.inc" )
	endif()

	if( NOT ${IS_LINUX} )

	# HACK!
	string( REPLACE " " ";" ARGS_COMPILE_FLAGS "${ARGS_COMPILE_FLAGS}" )

	add_custom_command(
		OUTPUT ${OUT_FILE}

		COMMAND ${PERL_EXE} ${PREP_FILE}

		ARGS ${ARGS_COMPILE_FLAGS} ${ARGS_FILE}

		DEPENDS ${ARGS_FILE_FULL}

		WORKING_DIRECTORY ${ARGS_WORKING_DIRECTORY}

		COMMENT "Compiling shader file ${_FILENAME}..."

		VERBATIM
		USES_TERMINAL
	)

	endif()

	target_sources( ${ARGS_TARGET} PRIVATE ${OUT_FILE} )
	source_group( "Shaders//Compiled" FILES ${OUT_FILE} )
endfunction()

function( parse_shaders )
	cmake_parse_arguments(
		ARGS
		""
		"TARGET;WORKING_DIRECTORY;CUSTOM_OUTDIR"
		"FILE"
		${ARGN}
	)

	message( STATUS "[${ARGS_TARGET}] Parsing shader list..." )

	# Example line 1: hsv_ps20.fxc="-dx9 -nv3x -ps20a"
	# Example line 2: UnlitGeneric_LightingOnly.vsh
	file( STRINGS "${ARGS_FILE}" SHADERS REGEX "^([A-Za-z0-9_-]+.(fxc|vsh|psh))(=\"(.*)\")?$" ) # "^([A-Za-z0-9_-]+.(fxc|vsh|psh))=\"(.*)\"$ <--- valid only if additional args set
	foreach( LINE IN LISTS SHADERS )
		# Just to make a CMAKE_MATCH_ list
		if( ${LINE} MATCHES "^([A-Za-z0-9_-]+.(fxc|vsh|psh))(=\"(.*)\")?$" )
			# CMAKE_MATCH_1 (group 1) will be full file name (with extension)
			# CMAKE_MATCH_2 (group 2) will be a file extension (fxc|vsh|psh)
			# CMAKE_MATCH_3 (group 3) will be a additional command args (with ="") (empty if not set)
			# CMAKE_MATCH_4 (group 4) will be a additional command args (empty if not set)

			target_sources( ${ARGS_TARGET} PRIVATE "${ARGS_WORKING_DIRECTORY}/${CMAKE_MATCH_1}" )
			source_group( "Shaders" FILES "${ARGS_WORKING_DIRECTORY}/${CMAKE_MATCH_1}" )

			if( ${CMAKE_MATCH_2} STREQUAL "fxc" )
				compile_shader(
					TARGET ${ARGS_TARGET}
					WORKING_DIRECTORY "${ARGS_WORKING_DIRECTORY}"
					FXC
					CUSTOM_OUTDIR "${ARGS_CUSTOM_OUTDIR}" # shader_nvfx crap
					FILE_FULL "${ARGS_WORKING_DIRECTORY}/${CMAKE_MATCH_1}"
					FILE "${CMAKE_MATCH_1}"
					COMPILE_FLAGS "${CMAKE_MATCH_4}"
				)
			elseif( ${CMAKE_MATCH_2} STREQUAL "vsh" )
				compile_shader(
					TARGET ${ARGS_TARGET}
					WORKING_DIRECTORY "${ARGS_WORKING_DIRECTORY}"
					VSH
					FILE_FULL "${ARGS_WORKING_DIRECTORY}/${CMAKE_MATCH_1}"
					FILE "${CMAKE_MATCH_1}"
					COMPILE_FLAGS "${CMAKE_MATCH_4}"
				)
			elseif( ${CMAKE_MATCH_2} STREQUAL "psh" )
				compile_shader(
					TARGET ${ARGS_TARGET}
					WORKING_DIRECTORY "${ARGS_WORKING_DIRECTORY}"
					PSH
					FILE_FULL "${ARGS_WORKING_DIRECTORY}/${CMAKE_MATCH_1}"
					FILE "${CMAKE_MATCH_1}"
					COMPILE_FLAGS "${CMAKE_MATCH_4}"
				)
			endif()
		endif()
	endforeach()

	message( STATUS "[${ARGS_TARGET}] DONE!" )
endfunction()
