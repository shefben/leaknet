set( STDSHADER_DX6_DIR ${CMAKE_CURRENT_LIST_DIR} )

set( STDSHADER_DX6_SOURCE_FILES )
BEGIN_SRC( STDSHADER_DX6_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			#"${SRCDIR}/public/tier0/memoverride.cpp"

			"${STDSHADER_DX6_DIR}/basetimesmod2xenvmap.cpp"
			"${STDSHADER_DX6_DIR}/cloud.cpp"
			"${STDSHADER_DX6_DIR}/decalbasetimeslightmapalphablendselfillum.cpp"
			"${STDSHADER_DX6_DIR}/decalmodulate.cpp"
			"${STDSHADER_DX6_DIR}/eyeball.cpp"
			"${STDSHADER_DX6_DIR}/fogsurface.cpp"
			"${STDSHADER_DX6_DIR}/lightmappedgeneric_dx6.cpp"
			"${STDSHADER_DX6_DIR}/lightmappedtwotexture.cpp"
			"${STDSHADER_DX6_DIR}/modulate_dx6.cpp"
			"${STDSHADER_DX6_DIR}/shadowbuild_dx6.cpp"
			"${STDSHADER_DX6_DIR}/shadow_dx6.cpp"
			"${STDSHADER_DX6_DIR}/skyfog.cpp"
			"${STDSHADER_DX6_DIR}/sky_dx6.cpp"
			"${STDSHADER_DX6_DIR}/sprite_dx6.cpp"
			"${STDSHADER_DX6_DIR}/unlitgeneric_dx6.cpp"
			"${STDSHADER_DX6_DIR}/unlittwotexture_dx6.cpp"
			"${STDSHADER_DX6_DIR}/vertexlitgeneric_dx6.cpp"
			"${STDSHADER_DX6_DIR}/viewalpha.cpp"
			"${STDSHADER_DX6_DIR}/volumetricfog.cpp"
			"${STDSHADER_DX6_DIR}/water_dx60.cpp"
			"${STDSHADER_DX6_DIR}/worldtwotextureblend.cpp"
			"${STDSHADER_DX6_DIR}/worldvertextransition_dx6.cpp"
			"${STDSHADER_DX6_DIR}/writez_dx6.cpp"
		#}
	)
END_SRC( STDSHADER_DX6_SOURCE_FILES "Source Files" )

add_object(
	TARGET stdshader_dx6
	MODULE
	INSTALL_DEST "${GAMEDIR}/bin"
	SOURCES ${STDSHADER_DX6_SOURCE_FILES}
)

target_compile_definitions(
	stdshader_dx6 PRIVATE

	fopen=dont_use_fopen

	STDSHADER_DX6_DLL_EXPORT
)

target_link_libraries(
	stdshader_dx6 PRIVATE

	shaderlib
)
