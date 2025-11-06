set( STDSHADER_DBG_DIR ${CMAKE_CURRENT_LIST_DIR} )

include( "${SRCDIR}/cmake/misc/parse_shaders.cmake" )

set( STDSHADER_DBG_SOURCE_FILES )
BEGIN_SRC( STDSHADER_DBG_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/mathlib.cpp"
			#"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/vmatrix.cpp"

			"${STDSHADER_DBG_DIR}/BaseVSShader.cpp"
			"${STDSHADER_DBG_DIR}/debugbumpedlightmap.cpp"
			"${STDSHADER_DBG_DIR}/debugbumpedvertexlit.cpp"
			"${STDSHADER_DBG_DIR}/debugfbtexture.cpp"
			"${STDSHADER_DBG_DIR}/debuglightingonly.cpp"
			"${STDSHADER_DBG_DIR}/debuglightmap.cpp"
			"${STDSHADER_DBG_DIR}/debugluxel.cpp"
			"${STDSHADER_DBG_DIR}/debugmodifyvertex.cpp"
			"${STDSHADER_DBG_DIR}/debugnormalmap.cpp"
			"${STDSHADER_DBG_DIR}/debugsoftwarevertexshader.cpp"
			"${STDSHADER_DBG_DIR}/debugtangentspace.cpp"
			"${STDSHADER_DBG_DIR}/debugunlit.cpp"
			"${STDSHADER_DBG_DIR}/debugvertexlit.cpp"
			"${STDSHADER_DBG_DIR}/fillrate.cpp"
			"${STDSHADER_DBG_DIR}/fillrate_dx6.cpp"
			"${STDSHADER_DBG_DIR}/stdshader_dbg.cpp"
		#}
	)
END_SRC( STDSHADER_DBG_SOURCE_FILES "Source Files" )

set( STDSHADER_DBG_HEADER_FILES )
BEGIN_SRC( STDSHADER_DBG_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${STDSHADER_DBG_DIR}/BaseVSShader.h"
		#}
	)
END_SRC( STDSHADER_DBG_HEADER_FILES "Header Files" )

add_object(
	TARGET stdshader_dbg
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${STDSHADER_DBG_SOURCE_FILES} ${STDSHADER_DBG_HEADER_FILES}
)

parse_shaders( TARGET stdshader_dbg WORKING_DIRECTORY "${STDSHADER_DBG_DIR}" FILE "stdshader_dbg.txt" )

target_compile_definitions(
	stdshader_dbg PRIVATE

	fopen=dont_use_fopen

	STDSHADER_DBG_DLL_EXPORT
)

target_link_libraries(
	stdshader_dbg PRIVATE

	shaderlib
)
