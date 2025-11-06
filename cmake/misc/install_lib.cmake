#-----------------------------------------------------------------------------
# install_lib.cmake
#
# $
#-----------------------------------------------------------------------------

include_guard( GLOBAL )

set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )

# Set of helper functions to add defintions/options/libs for each target in a filtered way
function( add_compile_definitions_filtered target definitions exclude_list )
	foreach( additional_definition IN LISTS ${definitions} )
		set( SHOULD_EXCLUDE 0 )

		# Exclude the compile definition if target defines an exclude list
		foreach( exclude IN LISTS exclude_list )
			if( ${additional_definition} STREQUAL ${exclude} )
				set( SHOULD_EXCLUDE 1 )
				break()
			endif()
		endforeach()

		if( NOT ${SHOULD_EXCLUDE} )
			target_compile_definitions( ${target} PRIVATE ${additional_definition} )
		endif()
	endforeach()
endfunction()

function( add_compile_options_filtered target options exclude_list )
	foreach( additional_option IN LISTS ${options} )
		set( SHOULD_EXCLUDE 0 )

		# Exclude the compile options if target defines an exclude list
		foreach( exclude IN LISTS exclude_list )
			if( ${additional_option} STREQUAL ${exclude} )
				set( SHOULD_EXCLUDE 1 )
				break()
			endif()
		endforeach()

		if( NOT ${SHOULD_EXCLUDE} )
			target_compile_options( ${target} PRIVATE "$<$<COMPILE_LANGUAGE:C,CXX>:${additional_option}>" )
		endif()
	endforeach()
endfunction()

function( add_sources_filtered target sources exclude_list )
	foreach( additional_source IN LISTS ${sources} )
		set( SHOULD_EXCLUDE 0 )

		# Exclude the source if target defines an exclude list
		foreach( exclude IN LISTS exclude_list )
			if( ${additional_source} STREQUAL ${exclude} )
				set( SHOULD_EXCLUDE 1 )
				break()
			endif()
		endforeach()

		if( NOT ${SHOULD_EXCLUDE} )
			target_sources( ${target} PRIVATE ${additional_source} )
		endif()
	endforeach()
endfunction()

function( add_include_dirs_filtered target includes exclude_list )
	foreach( additional_include_dir IN LISTS ${includes} )
		set( SHOULD_EXCLUDE 0 )

		# Exclude the source if target defines an exclude list
		foreach( exclude IN LISTS exclude_list )
			if( ${additional_include_dir} STREQUAL ${exclude} )
				set( SHOULD_EXCLUDE 1 )
				break()
			endif()
		endforeach()

		if( NOT ${SHOULD_EXCLUDE} )
			target_include_directories( ${target} PRIVATE ${additional_include_dir} )
		endif()
	endforeach()
endfunction()

function( add_libraries_filtered target libraries exclude_list )
	foreach( additional_lib IN LISTS ${libraries} )
		set( SHOULD_EXCLUDE 0 )

		# Exclude the lib if target defines an exclude list
		foreach( exclude IN LISTS exclude_list )
			if( ${additional_lib} STREQUAL ${exclude} )
				set( SHOULD_EXCLUDE 1 )
				break()
			endif()
		endforeach()

		if( NOT ${SHOULD_EXCLUDE} )
			get_target_property( libraries ${target} LINK_LIBRARIES )
			# Don't bother adding it if the target already links it manually
			foreach( lib IN LISTS libraries )
				if( ${additional_lib} STREQUAL ${lib} )
					set( SHOULD_EXCLUDE 1 )
					break()
				endif()
			endforeach()
		endif()

		if( NOT ${SHOULD_EXCLUDE} )
			target_link_libraries( ${target} PRIVATE ${additional_lib} )
		endif()
	endforeach()
endfunction()

# WARNING! SOURCES SHOULD BE LAST
function( add_object )
	cmake_parse_arguments(
		SHARED_ARGS
		"DEBUGLOG;EXECUTABLE;SHARED;MODULE;STATIC;NO_LIB_PREFIX;NO_STRIP"
		"TARGET;PROPERTY_FOLDER;INSTALL_OUTNAME;INSTALL_DEST"
		"EXCLUDE_COMPILE_DEFINITIONS;EXCLUDE_COMPILE_OPTIONS;EXCLUDE_SOURCE;EXCLUDE_INCLUDE_DIRS;EXCLUDE_LIB;SOURCES"
		${ARGN}
	)

	if( SHARED_ARGS_DEBUGLOG )
		message( "SHARED_ARGS_DEBUGLOG: ${SHARED_ARGS_DEBUGLOG}" )
		message( "SHARED_ARGS_EXECUTABLE: ${SHARED_ARGS_EXECUTABLE}" )
		message( "SHARED_ARGS_SHARED: ${SHARED_ARGS_SHARED}" )
		message( "SHARED_ARGS_MODULE: ${SHARED_ARGS_MODULE}" )
		message( "SHARED_ARGS_STATIC: ${SHARED_ARGS_STATIC}" )
		message( "SHARED_ARGS_NO_LIB_PREFIX: ${SHARED_ARGS_NO_LIB_PREFIX}" )
		message( "SHARED_ARGS_TARGET: ${SHARED_ARGS_TARGET}" )
		message( "SHARED_ARGS_PROPERTY_FOLDER: ${SHARED_ARGS_PROPERTY_FOLDER}" )
		message( "SHARED_ARGS_INSTALL_OUTNAME: ${SHARED_ARGS_INSTALL_OUTNAME}" )
		message( "SHARED_ARGS_INSTALL_DEST: ${SHARED_ARGS_INSTALL_DEST}" )
		message( "SHARED_ARGS_SOURCES: ${SHARED_ARGS_SOURCES}" )

		message( "SHARED_ARGS_EXCLUDE_COMPILE_DEFINITIONS: ${SHARED_ARGS_EXCLUDE_COMPILE_DEFINITIONS}" )
		message( "SHARED_ARGS_EXCLUDE_COMPILE_OPTIONS: ${SHARED_ARGS_EXCLUDE_COMPILE_OPTIONS}" )
		message( "SHARED_ARGS_EXCLUDE_SOURCE: ${SHARED_ARGS_EXCLUDE_SOURCE}" )
		message( "SHARED_ARGS_EXCLUDE_INCLUDE_DIRS: ${SHARED_ARGS_EXCLUDE_INCLUDE_DIRS}" )
		message( "SHARED_ARGS_EXCLUDE_LIB: ${SHARED_ARGS_EXCLUDE_LIB}" )
	endif()

	if( NOT SHARED_ARGS_TARGET )
		message( FATAL_ERROR "Missing TARGET!" )
	endif()

	if( SHARED_ARGS_EXECUTABLE )
		# add_executable
		cmake_parse_arguments(
			ADD_EXECUTABLE_ARGS
			"WIN32"
			""
			""
			${ARGN}
		)

		if( SHARED_ARGS_DEBUGLOG )
			message( "ADD_EXECUTABLE_ARGS_WIN32: ${ADD_EXECUTABLE_ARGS_WIN32}" )
		endif()

		set( _INSTALL_WIN32 )
		if( ADD_EXECUTABLE_ARGS_WIN32 )
			set( _INSTALL_WIN32 WIN32 )
		endif()

		add_executable(
			${SHARED_ARGS_TARGET} ${_INSTALL_WIN32}
			${SHARED_ARGS_SOURCES}
		)

		set( NEW_PROPERTY_FOLDER "Executables" )

		if( NOT SHARED_ARGS_NO_STRIP )
			# Only applies to Linux and OSX
			target_strip_symbols( ${SHARED_ARGS_TARGET} )
		endif()

		add_compile_options_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_COMPILE_OPTIONS_EXE "${SHARED_ARGS_EXCLUDE_COMPILE_OPTIONS}" )
		add_libraries_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_LINK_LIBRARIES_EXE "${SHARED_ARGS_EXCLUDE_LIB}" )
		add_sources_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_SOURCES_EXE "${SHARED_ARGS_EXCLUDE_SOURCE}" )
		add_include_dirs_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_INCLUDES_EXE "${SHARED_ARGS_EXCLUDE_INCLUDE_DIRS}" )
		add_compile_definitions_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_COMPILE_DEFINITIONS_EXE "${SHARED_ARGS_EXCLUDE_COMPILE_DEFINITIONS}" )
		target_link_options( ${SHARED_ARGS_TARGET} PRIVATE ${ADDITIONAL_LINK_OPTIONS_EXE} )
		target_compile_definitions( ${SHARED_ARGS_TARGET} PRIVATE MEMOVERRIDE_MODULE=$<TARGET_NAME_IF_EXISTS:${SHARED_ARGS_TARGET}> )

	elseif( SHARED_ARGS_SHARED OR SHARED_ARGS_MODULE OR SHARED_ARGS_STATIC )
		# add_library
		set( _TYPE "" ) # this is shit
		if( SHARED_ARGS_SHARED )
			set( _TYPE SHARED )
		elseif( SHARED_ARGS_MODULE )
			set( _TYPE MODULE )
		elseif( SHARED_ARGS_STATIC )
			set( _TYPE STATIC )
		else()
			message( FATAL_ERROR "Missing library type for ${SHARED_ARGS_TARGET}" )
		endif()

		add_library(
			${SHARED_ARGS_TARGET} ${_TYPE}
			${SHARED_ARGS_SOURCES}
		)

		if( SHARED_ARGS_SHARED OR SHARED_ARGS_MODULE )
			add_compile_options_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_COMPILE_OPTIONS_DLL "${SHARED_ARGS_EXCLUDE_COMPILE_OPTIONS}" )
			add_libraries_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_LINK_LIBRARIES_DLL "${SHARED_ARGS_EXCLUDE_LIB}" )
			add_sources_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_SOURCES_DLL "${SHARED_ARGS_EXCLUDE_SOURCE}" )
			add_include_dirs_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_INCLUDES_DLL "${SHARED_ARGS_EXCLUDE_INCLUDE_DIRS}" )
			add_compile_definitions_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_COMPILE_DEFINITIONS_DLL "${SHARED_ARGS_EXCLUDE_COMPILE_DEFINITIONS}" )
			target_link_options( ${SHARED_ARGS_TARGET} PRIVATE ${ADDITIONAL_LINK_OPTIONS_DLL} )
			target_compile_definitions( ${SHARED_ARGS_TARGET} PRIVATE MEMOVERRIDE_MODULE=$<TARGET_NAME_IF_EXISTS:${SHARED_ARGS_TARGET}> DLLNAME=$<TARGET_NAME_IF_EXISTS:${SHARED_ARGS_TARGET}> )
		elseif( SHARED_ARGS_STATIC )
			add_compile_options_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_COMPILE_OPTIONS_LIB "${SHARED_ARGS_EXCLUDE_COMPILE_OPTIONS}" )
			add_sources_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_SOURCES_LIB "${SHARED_ARGS_EXCLUDE_SOURCE}" )
			add_include_dirs_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_INCLUDES_LIB "${SHARED_ARGS_EXCLUDE_INCLUDE_DIRS}" )
			add_compile_definitions_filtered( ${SHARED_ARGS_TARGET} ADDITIONAL_COMPILE_DEFINITIONS_LIB "${SHARED_ARGS_EXCLUDE_COMPILE_DEFINITIONS}" )
			target_link_options( ${SHARED_ARGS_TARGET} PRIVATE ${ADDITIONAL_LINK_OPTIONS_LIB} )
			target_compile_definitions( ${SHARED_ARGS_TARGET} PRIVATE LIBNAME=$<TARGET_NAME_IF_EXISTS:${SHARED_ARGS_TARGET}> )
		endif()

		if( SHARED_ARGS_MODULE OR SHARED_ARGS_NO_LIB_PREFIX ) # disable lib- prefix for all modules (from this time any new shared library should be MODULE)
			set_target_properties( ${SHARED_ARGS_TARGET} PROPERTIES PREFIX "" )
		endif()

		if( SHARED_ARGS_SHARED OR SHARED_ARGS_MODULE )
			set( NEW_PROPERTY_FOLDER "Shared Libs" )
		elseif( SHARED_ARGS_STATIC )
			set( NEW_PROPERTY_FOLDER "Static Libs" )
		endif()

		if( NOT SHARED_ARGS_NO_STRIP AND NOT SHARED_ARGS_STATIC )
			# Only applies to Linux and OSX
			target_strip_symbols( ${SHARED_ARGS_TARGET} )
		endif()
	else()
		message( FATAL_ERROR "${SHARED_ARGS_TARGET} has unknown object type!" )
	endif()

	if( SHARED_ARGS_PROPERTY_FOLDER )
		set( NEW_PROPERTY_FOLDER ${SHARED_ARGS_PROPERTY_FOLDER} )
	endif()

	if( NEW_PROPERTY_FOLDER )
		set_property(
			TARGET ${SHARED_ARGS_TARGET} PROPERTY FOLDER ${NEW_PROPERTY_FOLDER}
		)
	endif()


	## Merged set_install_properties into add_object
	# Don't install if DESTINATION is not set
	if( NOT "${SHARED_ARGS_INSTALL_DEST}" STREQUAL "" )
		# use SHARED_ARGS_TARGET if OUTNAME is not set
		if( NOT SHARED_ARGS_INSTALL_OUTNAME )
			#message( "WARNING! ${SHARED_ARGS_TARGET} is missing OUTNAME!" )
			set( SHARED_ARGS_INSTALL_OUTNAME "${SHARED_ARGS_TARGET}" )
		endif()

		# append executable subname for executables
		if( SHARED_ARGS_EXECUTABLE )
			set( SHARED_ARGS_INSTALL_OUTNAME "${SHARED_ARGS_INSTALL_OUTNAME}${EXECUTABLE_SUBNAME}" )
		endif()

		set_target_properties( ${SHARED_ARGS_TARGET} PROPERTIES OUTPUT_NAME "${SHARED_ARGS_INSTALL_OUTNAME}" )

		if( NOT MSVC )
			install(
				TARGETS ${SHARED_ARGS_TARGET}
				DESTINATION ${SHARED_ARGS_INSTALL_DEST}

				PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE	#
					    GROUP_READ GROUP_EXECUTE			# chmod 755
					    WORLD_READ WORLD_EXECUTE			#
			)

			if( NOT SHARED_ARGS_NO_STRIP AND NOT SHARED_ARGS_STATIC )
				# DBG
				install(
					FILES "$<TARGET_FILE:${SHARED_ARGS_TARGET}>.dbg"
					DESTINATION ${SHARED_ARGS_INSTALL_DEST}

					PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE	#
						    GROUP_READ GROUP_EXECUTE			# chmod 755
						    WORLD_READ WORLD_EXECUTE			#
				)
			endif()
		else()
			install(
				TARGETS ${SHARED_ARGS_TARGET}
				RUNTIME DESTINATION ${SHARED_ARGS_INSTALL_DEST}
				LIBRARY DESTINATION ${SHARED_ARGS_INSTALL_DEST}
			)

			# PDB
			install(
				FILES $<TARGET_PDB_FILE:${SHARED_ARGS_TARGET}>
				DESTINATION ${SHARED_ARGS_INSTALL_DEST}
			)
		endif()
	endif()
endfunction()
