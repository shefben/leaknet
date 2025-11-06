set( STDSHADER_DX7_DIR ${CMAKE_CURRENT_LIST_DIR} )

set( STDSHADER_DX7_SOURCE_FILES )
BEGIN_SRC( STDSHADER_DX7_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			#"${SRCDIR}/public/tier0/memoverride.cpp"

			"${STDSHADER_DX7_DIR}/shatteredglass_dx7.cpp"
			"${STDSHADER_DX7_DIR}/vertexlitgeneric_dx7.cpp"
		#}
	)
END_SRC( STDSHADER_DX7_SOURCE_FILES "Source Files" )

add_object(
	TARGET stdshader_dx7
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${STDSHADER_DX7_SOURCE_FILES}
)

target_compile_definitions(
	stdshader_dx7 PRIVATE

	fopen=dont_use_fopen

	STDSHADER_DX7_DLL_EXPORT
)

target_link_libraries(
	stdshader_dx7 PRIVATE

	shaderlib
)
